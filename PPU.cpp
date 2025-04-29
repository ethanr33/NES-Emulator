
#include "PPU.h"
#include <iostream>

using namespace std;

PPU::PPU() {
    ui = new UI();
}

PPU::PPU(bool ui_disabled) {
    ui = new UI(ui_disabled);
}

uint8_t PPU::get_sprite_height() {
    if (ppuctrl.sprite_height) {
        return 16;
    } else {
        return 8;
    }
}

uint8_t PPU::read_from_cpu(uint16_t address) {
    if (address >= 0x2000 && address <= 0x3FFF) {
        int register_num = address & 0x7;
        switch (register_num) {
            case 0:
                throw std::runtime_error("Attempted to read from PPUCTRL register");
                break;
            case 1:
                throw std::runtime_error("Attempted to read from PPUMASK register");
                break;
            case 2: {
                // Reading one PPU clock before reads it as clear and never sets the flag or generates NMI for that frame
                if (scanline == 241 && cur_dot == 0) {
                    ppustatus.vblank = false;
                    has_nmi_triggered = true;
                    return ppustatus.serialize();
                } else if (scanline == 241 && (cur_dot == 1 || cur_dot == 2)) {
                    // Race condition case
                    uint8_t res = ppustatus.serialize();
                    bus->set_nmi_suppression_status(true);
                    ppustatus_vblank_read_race_condition = true;
                    ppustatus.vblank = false;
                    return res;
                } else {
                    // Normal case
                    uint8_t res = ppustatus.serialize();
                    ppustatus.vblank = false;

                    return res;
                }
                break;
            }
            case 3:
                throw std::runtime_error("Attempted to read from OAMADDR register");
                break;
            case 4:
                return primary_OAM.at(oamaddr);
                break;
            case 5:
                throw std::runtime_error("Attempted to read from PPUSCROLL register");
                break;
            case 6:
                throw std::runtime_error("Attempted to read from PPUADDR register");
                break;
            case 7: {

                // See https://www.nesdev.org/wiki/PPU_registers#The_PPUDATA_read_buffer
                // for detailed operation

                // When reading palette RAM data, return data immediately
                // Fill read buffer with mirrored nametable data\

                uint8_t read_data;

                if (ppuaddr >= 0x3F00 && ppuaddr <= 0x3FFF) {
                    ppudata_read_buffer = read_from_ppu(ppuaddr - 0x1000);
                    read_data = read_from_ppu(ppuaddr);
                } else {
                    read_data = ppudata_read_buffer;
                    ppudata_read_buffer = read_from_ppu(ppuaddr);
                }

                if (ppuctrl.increment_mode == 0) {
                    ppuaddr++;
                } else {
                    ppuaddr += 32;
                }

                return read_data;
            }
            default:
                break;
        }
    }

    return 0x0;
}

void PPU::write_from_cpu(uint16_t address, uint8_t val) {
    if (address >= 0x2000 && address <= 0x3FFF) {
        int register_num = address & 0x7;
        switch (register_num) {
            case 0:
                ppuctrl = PPUCTRL(val);
                break;
            case 1:
                ppumask = PPUMASK(val);
                break;
            case 2:
                break;
            case 3:
                oamaddr = val;
                break;
            case 4:
                primary_OAM.at(oamaddr) = val;
                oamaddr++;
                break;
            case 5:
                break;
            case 6:
                // w will be 0 on first write, 1 on second write
                if (w == 0) {
                    ppuaddr = val << 8;
                    w = 1;
                } else {
                    ppuaddr |= val;
                    w = 0;
                }
                break;
            case 7:
                write_from_ppu(ppuaddr, val);

                if (ppuctrl.increment_mode == 0) {
                    ppuaddr++;
                } else {
                    ppuaddr += 32;
                }

                break;
            default:
                break;
        }
    }
} 

// Communicate with PPU bus

uint8_t PPU::read_from_ppu(uint16_t address) {
    uint8_t data;

    if (cartridge->read_ppu(address)) {
        // should handle the pattern table cases (addresses 0x0000 - 0x1FFF)

        if (cartridge->CHR_ROM.size() == 0) {
            return VRAM[address];
        }

        return cartridge->CHR_ROM.at(address);
    } else if (address >= 0x2000 && address <= 0x2FFF) {
        // handle nametables and mirroring
        address = map_to_nametable(address);
        return VRAM[address];
    } else if (address >= 0x3000 && address <= 0x3EFF) {
        // mirror of 0x2000 - 0x2EFF
        address = map_to_nametable(address - 0x1000);
        return VRAM[address];
    } else if (address >= 0x3F00 && address <= 0x3F1F) {
        // palette table
        // 0x3F00 - 0x3F1F is mirrored in the range 0x3F20 - 0x3FFF
        return PALETTE_RAM[address & 0x1F];
    } else {
        throw std::runtime_error("Edge case in read_from_ppu");
    }
}

uint16_t PPU::map_to_nametable(uint16_t address) {
    if (address < 0x2000 || address > 0x3000) {
        throw std::runtime_error("Attempted to access nametable out of bounds at address " + std::to_string(address));
    }

    switch (cartridge->mirroring_type) {
        case (HORIZONTAL):
            if (address >= 0x2400 && address <= 0x27FF) {
                address -= 0x400;
            } else if (address >= 0x2C00 && address <= 0x2FFF) {
                address -= 0x400;
            }
            break;
        case (VERTICAL):
            if (address >= 0x2800 && address <= 0x2BFF) {
                address -= 0x800;
            } else if (address >= 0x2C00 && address <= 0x2FFF) {
                address -= 0x800;
            }
            break;
        case (FOUR_SCREEN):
            throw std::runtime_error("Four screen mirroring is unsupported");
            break;
        default:
            throw std::runtime_error("Unknown mirroring type on cartridge");
            break;
    }

    return address;
}

void PPU::write_from_ppu(uint16_t address, uint8_t val) {

    if (cartridge->write_ppu(address, val)) {
        // In this case, we only write if we're working with CHR-RAM instead of CHR-ROM, because CHR-ROM is unwriteable
        VRAM[address] = val;
    } else if (address >= 0x2000 && address <= 0x2FFF) {
        // handle nametables and mirroring
        address = map_to_nametable(address);
        VRAM[address] = val;
    } else if (address >= 0x3000 && address <= 0x3EFF) {
        // mirror of 0x2000 - 0x2EFF
        write_from_ppu(address & 0x2EFF, val);
    } else if (address >= 0x3F00 && address <= 0x3FFF) {
        // palette table
        // 0x3F00 - 0x3F1F is mirrored in the range 0x3F20 - 0x3FFF
        if ((address & 0x3) == 0) {
            // Entry 0 of each palette is shared between the background and sprite palettes.
            // So if we write to entry 0 of any palette we need to make sure we also write to the 
            // other corresponding background or sprite palette.
            PALETTE_RAM[(address & 0x1F) | 0x10] = val;
            PALETTE_RAM[(address & 0x1F) & 0xEF] = val;
        } else {
            PALETTE_RAM[address & 0x1F] = val;
        }
    }

    return;
}

void PPU::load_cartridge(Cartridge* new_cartridge) {
    cartridge = new_cartridge;
} 

void PPU::load_OAMDMA(uint8_t high_byte) {
    // Copy over a page of data at address 0xXX00, where XX is the data written to this register
    // Copy over page of data to OAMDMA starting at oamaddr, wrapping around in case the address is greater than 255

    uint16_t starting_address = (high_byte) << 8;

    for (int i = 0; i < 256; i++) {
        uint8_t destination_address = oamaddr + i;
        uint16_t source_address = starting_address | i;

        primary_OAM.at(destination_address) = bus->read_cpu(source_address);
    }
}

// We represent the process for sprite evaluation as a state machine.
// For more details, see https://www.nesdev.org/wiki/PPU_sprite_evaluation

void PPU::run_sprite_evaluation() {

    // Sprite evaluation only occurs on visible scanlines
    switch (cur_sprite_evaluation_stage) {
        case IDLE:
            if (scanline == 261 && cur_dot == 340) {
                cur_sprite_evaluation_stage = STAGE_1;
            }
            
            break;
        case STAGE_1:
            // Secondary OAM is initialized to $FF
            secondary_OAM.assign(SECONDARY_OAM_SIZE, 0xFF);

            if (cur_dot == 64) {
                cur_sprite_evaluation_stage = STAGE_2;
            }
            
            break;
        case STAGE_2:

            if (cur_dot == 65) {
                // Evaluate sprites here

                // n is the index of the sprite we're looking at
                int n = 0;
                for (; n < 64; n += 1) {
                    uint8_t cur_sprite_y = primary_OAM.at(4 * n);

                    uint8_t sprite_height = get_sprite_height();

                    // Check if the sprite will be rendered on the NEXT scanline
                    if (cur_sprite_y <= scanline + 1 && scanline + 1 < (uint8_t) (sprite_height + cur_sprite_y)) {
                        // If it will be rendered, copy it into secondary OAM
                        std::copy(primary_OAM.begin() + 4 * n, primary_OAM.begin() + 4 * n + 4, secondary_OAM.begin() + 4 * num_sprites_found);
                        OAM_indices.at(num_sprites_found) = n;
                        num_sprites_found++;
                    }

                    if (num_sprites_found == 8) {
                        // Our secondary OAM is full now
                        break;
                    }
                }

                // Search through the rest of the primary OAM to see if the sprite overflow flag is set
                if (num_sprites_found == 8) {
                    int m = 0;

                    while (n < 64) {
                        // This is the buggy case, and the y-coordinate may not be accurate
                        uint8_t cur_sprite_y = primary_OAM.at(4 * n + m);
                        uint8_t sprite_height = get_sprite_height();

                        // Check if our y value is in range
                        if (scanline + 1 >= cur_sprite_y && scanline + 1 < cur_sprite_y + sprite_height) {
                            // If it is, another sprite could have been rendered this scanline. 
                            // Set sprite overflow flag accordingly.
                            ppustatus.sprite_overflow = true;


                            m = m + 4;
                            if (m >= 4) {
                                n++;
                                m = m % 4;
                            }
                        } else {
                            // If sprite is not in range, increment n AND m. This is a hardware bug
                            m++;
                            n++;

                            // m can only be 0, 1, 2, or 3.
                            if (m == 4) {
                                m = 0;
                                n++;
                            }
                        }

                    }
                } else {
                    // If there aren't 8 sprites rendered this scanline, part of secondary OAM must be empty.
                    // Let's fill in the rest of secondary OAM properly.

                    // The first empty sprite slot will consist of sprite 63's Y-coordinate followed by 3 $FF bytes
                    // for subsequent empty sprite slots, this will be four $FF bytes

                    int i = 4 * num_sprites_found;

                    secondary_OAM.at(i) = primary_OAM.at(PRIMARY_OAM_SIZE - 4);
                    secondary_OAM.at(i + 1) = 0xFF;
                    secondary_OAM.at(i + 2) = 0xFF;
                    secondary_OAM.at(i + 3) = 0xFF;

                    i += 4;

                    while (i < SECONDARY_OAM_SIZE) {
                        secondary_OAM.at(i) = 0xFF;
                        secondary_OAM.at(i + 1) = 0xFF;
                        secondary_OAM.at(i + 2) = 0xFF;
                        secondary_OAM.at(i + 3) = 0xFF;
                        OAM_indices.at(i / 4) = 0xFF; // Need to fill in OAM index buffer with placeholder
                        i += 4;
                    }
                }
            }

            if (cur_dot == 256) {
                cur_sprite_evaluation_stage = STAGE_3;
            }
            break;
        case STAGE_3:
            // Sprite fetches (8 sprites total, 8 cycles per sprite)
            // Fill OAM buffer for rendering

            if (cur_dot == 257) {
                for (int i = 0; i < secondary_OAM.size(); i++) {
                    OAM_buffer.at(i) = secondary_OAM.at(i);
                }

                // for (int i = 0; i < OAM_buffer.size(); i++) {
                //     std::cout << std::hex << (int) OAM_buffer.at(i) << " ";
                // }

                // std::cout << std::endl;
            }


            if (cur_dot == 320) {
                num_sprites_found = 0;
                cur_sprite_evaluation_stage = STAGE_4;
            }
            
            break;
        case STAGE_4:
            // Cycles 321-340+0: Background render pipeline initialization

            if (cur_dot == 340) {
                if (scanline < 239) {
                    cur_sprite_evaluation_stage = STAGE_1;
                } else {
                    cur_sprite_evaluation_stage = IDLE;
                }
            }
            
            break;
        default:
            break;
    }
}

void PPU::tick() {

    run_sprite_evaluation();

    switch (cur_ppu_rendering_stage) {
        case PRE_RENDER:
            {
                // First dot skipped if the frame number is odd and rendering is enabled
                if (cur_dot == 0 && (frames_elapsed % 2 == 1) && (ppumask.background_enable || ppumask.sprite_enable)) {
                    cur_dot++;
                }

                // VBlank and other flags are always cleared on dot 1
                if (cur_dot == 1) {
                    // Doesn't really matter where we clear the race condition, as long as it's cleared before it can happen again
                    bus->set_nmi_line(false);
                    bus->set_nmi_suppression_status(false);
                    ppustatus_vblank_read_race_condition = false;
                    ppustatus.vblank = false;
                    ppustatus.sprite_hit = false;
                    ppustatus.sprite_overflow = false;

                    cur_sprite_evaluation_stage = IDLE;
                }

                // OAMADDR is set to 0 during ticks 257-320 of prerender scanlines
                if (cur_dot >= 257 && cur_dot <= 320) {
                    oamaddr = 0;
                }

                if (cur_dot == 340) {
                    cur_dot = 0;
                    scanline = 0;
                    cur_ppu_rendering_stage = VISIBLE;
                } else {
                    cur_dot++;
                }
            }
            break;
        case VISIBLE:
            {
                // OAMADDR is set to 0 during ticks 257-320 of visible scanlines
                if (cur_dot >= 257 && cur_dot <= 320) {
                    oamaddr = 0;
                }

                if (cur_dot >= 1 && cur_dot <= 256) {

                    // Start background rendering

                    uint16_t pixel_x = cur_dot % 256;
                    uint16_t pixel_y = scanline;
    
                    // In the binary representation of a tile, the first pixel will be the MSB (bit 7)
                    uint8_t tile_offset_x = 7 - (pixel_x % 8);
                    uint8_t tile_offset_y = pixel_y % 8;
    
                    // Fetch background data from nametable
    
                    // TODO: Add support for mirroring types
                    // Who cares if we fetch the same data 8 times in a row, right???
    
                    uint16_t background_nametable_index = 32 * (pixel_y / 8) + (pixel_x / 8);
                    uint8_t cur_nametable_entry = read_from_ppu(0x2000 + 0x400 * ppuctrl.nametable_select + background_nametable_index); // FIX THIS LATER
    
                    uint16_t pattern_table_offset = 0;
    
                    if (ppuctrl.background_tile_select == 1) {
                        pattern_table_offset = 0x1000;
                    }
    
                    uint8_t background_pixel_layer_0 = read_from_ppu(16 * cur_nametable_entry + tile_offset_y + pattern_table_offset);
                    uint8_t background_pixel_layer_1 = read_from_ppu(16 * cur_nametable_entry + tile_offset_y + pattern_table_offset + 8);
    
                    uint16_t attribute_table_index = 8 * (pixel_y / 32) + (pixel_x / 32);
                    uint8_t attribute_table_val = read_from_ppu(0x23C0 + 0x400 * ppuctrl.nametable_select + attribute_table_index); // Attribute table is located at the end of the nametable
    
                    bool is_left_tile = (pixel_x % 32) < 16;
                    bool is_top_tile = (pixel_y % 32) < 16;
    
                    // The value in the attribute table is constructed as follows:
                    // attribute_table_val = (bottomright << 6) | (bottomleft << 4) | (topright << 2) | (topleft << 0)
                    // Where bottomright, bottomleft, topright, and topleft are the palette numbers for each quadrant of this block in the nametable
                    uint8_t background_color_palette_num;
    
                    if (is_top_tile && is_left_tile) {
                        // top left
                        background_color_palette_num = attribute_table_val & 0x3;
                    } else if (is_top_tile && !is_left_tile) {
                        // top right
                        background_color_palette_num = (attribute_table_val & 0xC) >> 2;
                    } else if (!is_top_tile && is_left_tile) {
                        // bottom left
                        background_color_palette_num = (attribute_table_val & 0x30) >> 4;
                    } else {
                        // bottom right
                        background_color_palette_num = (attribute_table_val & 0xC0) >> 6;
                    }
    
                    // For background rendering only.
                    // Palette addresses for the background are from 0x3F00 - 0x3F0F.
                    uint16_t background_palette_index = 0x3F00 + 4 * background_color_palette_num;
                    
                    uint8_t background_color0 = read_from_ppu(background_palette_index);
                    uint8_t background_color1 = read_from_ppu(background_palette_index + 1);
                    uint8_t background_color2 = read_from_ppu(background_palette_index + 2);
                    uint8_t background_color3 = read_from_ppu(background_palette_index + 3);
    
                    uint8_t background_pixel_color = (is_bit_set(tile_offset_x, background_pixel_layer_1) << 1) | is_bit_set(tile_offset_x, background_pixel_layer_0);
    
                    ui->set_background_palette(background_color0, background_color1, background_color2, background_color3);

                    // Start sprite rendering from secondary OAM

                    // First, check if there is a sprite rendered here

                    Sprite sprite_to_render;
                    bool is_sprite_here = false;
                    bool is_sprite_0_rendered = false;

                    for (int i = 0; i < OAM_buffer.size(); i += 4) {
                        Sprite cur_sprite = Sprite(
                            OAM_buffer.at(i),
                            OAM_buffer.at(i + 1),
                            OAM_buffer.at(i + 2),
                            OAM_buffer.at(i + 3)
                        );

                        if (cur_sprite.x_position <= pixel_x && cur_sprite.x_position + 7 >= pixel_x) {
                            sprite_to_render = cur_sprite;
                            is_sprite_here = true;

                            if (OAM_indices.at(i / 4) == 0) {
                                is_sprite_0_rendered = true;
                            }

                            break;
                        }
                    }

                    // Get sprite offset from top, left
                    uint8_t sprite_offset_x = pixel_x - sprite_to_render.x_position;
                    uint8_t sprite_offset_y = pixel_y - sprite_to_render.y_position;
                        
                    // Check if flip sprite vertically flag is set
                    if (is_bit_set(7, sprite_to_render.attributes)) {
                        sprite_offset_y = get_sprite_height() - sprite_offset_y - 1;
                    }
    
                    // Check if flip sprite horizontally flag is set
                    if (is_bit_set(6, sprite_to_render.attributes)) {
                        sprite_offset_x = SPRITE_WIDTH - sprite_offset_x - 1;
                    }
    
                    // Check if the sprite is rendered in front of or behind the background
                    bool is_sprite_behind_background = is_bit_set(6, sprite_to_render.attributes);
    
                    // Fetch sprite palette
                    uint8_t sprite_palette_num = sprite_to_render.attributes & 0x03;
                    uint16_t sprite_palette_index = 0x3F10 + 4 * sprite_palette_num;
    
                    uint8_t sprite_color0 = read_from_ppu(sprite_palette_index);
                    uint8_t sprite_color1 = read_from_ppu(sprite_palette_index + 1);
                    uint8_t sprite_color2 = read_from_ppu(sprite_palette_index + 2);
                    uint8_t sprite_color3 = read_from_ppu(sprite_palette_index + 3);
    
                    ui->set_sprite_palette(sprite_color0, sprite_color1, sprite_color2, sprite_color3);

                    uint8_t sprite_pattern_table_address;

                    if (get_sprite_height() == 16) {
                        // For 8x16 sprites, the pattern table is taken from the first bit of tile index number
                        sprite_pattern_table_address = (sprite_to_render.tile_index_number & 1) * 0x1000;
                    } else {
                        sprite_pattern_table_address = ppuctrl.sprite_tile_select ? 0x1000 : 0;
                    }
    
    
                    uint8_t sprite_pixel_layer0 = read_from_ppu(sprite_offset_y + 16 * sprite_to_render.tile_index_number + sprite_pattern_table_address);
                    uint8_t sprite_pixel_layer1 = read_from_ppu(8 + sprite_offset_y + 16 * sprite_to_render.tile_index_number + sprite_pattern_table_address);
    
                    uint8_t sprite_pixel_offset = 7 - (sprite_offset_x % 8);
                    uint8_t sprite_pixel_color = is_bit_set(sprite_pixel_offset, sprite_pixel_layer0) | (is_bit_set(sprite_pixel_offset, sprite_pixel_layer1) << 1);

                    // Check for sprite 0 cases
                    bool sprite_0_hit_possible = 
                        is_sprite_0_rendered &&
                        ppumask.background_enable &&
                        ppumask.sprite_enable &&
                        (background_pixel_color != 0 && sprite_pixel_color != 0);
                    
                    if (is_sprite_0_rendered && sprite_0_hit_possible) {
                        ppustatus.sprite_hit = true;
                    }

                    // Sprite pixel replaces background pixel only if:
                    // 1. Sprite pixel is opaque and has front priority, AND
                    // 2. Background pixel is transparent
                    
                    // For now, let's assume that BG or sprite pixel is transparent iff the pixel color is 0
                    // Is this a valid assumption?
                    
                    bool is_background_rendered = ppumask.background_enable;
                    bool is_sprite_in_front = !is_bit_set(5, sprite_to_render.attributes);
                    bool is_sprite_rendered = ppumask.sprite_enable && scanline > 0 && is_sprite_here && ((sprite_pixel_color != 0 && is_sprite_in_front) || (background_pixel_color == 0));
                    
                    if (is_sprite_rendered) {
                        ui->set_pixel(pixel_y, pixel_x, sprite_pixel_color, false);
                    } else if (is_background_rendered) {
                        ui->set_pixel(pixel_y, pixel_x, background_pixel_color, true);
                    }


                }

                if (cur_dot == 340) {
                    scanline++;
                    cur_dot = 0;

                    if (scanline == 240) {
                        cur_ppu_rendering_stage = POST_RENDER;
                    }
                } else {
                    cur_dot++;
                }
                break;
            }
        case POST_RENDER:
            {
                if (cur_dot == 340) {
                    cur_dot = 0;
                    scanline++;
                    cur_ppu_rendering_stage = VBLANK;
                } else {
                    cur_dot++;
                }
                break;
            }
        case VBLANK:
            {
                if (!ppustatus_vblank_read_race_condition && scanline == 241 && cur_dot == 1) {
                    ppustatus.vblank = true;
                }

                // The PPU pulls /NMI low if and only if both vblank and nmi_enable are true
                if (ppustatus.vblank && ppuctrl.nmi_enable) {
                    bus->set_nmi_line(true);
                } else {
                    bus->set_nmi_line(false);
                }

                if (scanline == 260 && cur_dot == 340) {
                    scanline++;
                    cur_dot = 0;

                    cur_ppu_rendering_stage = PRE_RENDER;

                    ui->tick();
                    frames_elapsed++;
                } else {
                    cur_dot++;

                    if (cur_dot == 341) {
                        cur_dot = 0;
                        scanline++;
                    }
                }

            }
            break;
        default:
            break;
    }

    // std::cout << (int) cur_dot << " " << (int) scanline << " " << (int) cur_ppu_rendering_stage << std::endl;
}

void PPU::reset() {
    scanline = 261;
    cur_dot = 0;
    cur_ppu_rendering_stage = PRE_RENDER;
    cur_sprite_evaluation_stage = IDLE;
}

void PPU::attach_bus(Bus* b) {
    bus = b;
}