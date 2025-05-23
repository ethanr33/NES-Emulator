
#include "PPU.h"
#include <iostream>

using namespace std;

PPU::PPU() {
    ui = new UI();
}

PPU::PPU(bool ui_disabled) {
    ui = new UI(ui_disabled);
}

uint8_t PPU::get_sprite_height() const {
    if (ppuctrl.sprite_height) {
        return 16;
    } else {
        return 8;
    }
}

bool PPU::is_rendering_enabled() const {
    return ppumask.background_enable || ppumask.sprite_enable;
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
                // Reading PPUSTATUS clears w register
                // Useful for writes to PPUADDR or PPUSCROLL to ensure writes happen in correct order
                w = 0;

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
                // Fill read buffer with mirrored nametable data

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

                // If rendering is enabled, and the PPU is currently rendering,
                // the v register is updated in a weird way
                if (is_rendering_enabled() && (scanline < 240 || scanline == 261) ) {
                    // Both coarse x and coarse v are updated
                    increment_coarse_x(v);
                    increment_y(v);

                    // In case v overflows, wrap around by removing the 16th bit
                    v = v & 0x7FFF;
                } else {
                    // If the PPU is not rendering, update v according to ppuctrl.increment_mode

                    if (ppuctrl.increment_mode == 0) {
                        v++;
                    } else {
                        v += 32;
                    }
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
                t = (t & 0x73FF) | ((val & 0x3) << 10);
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
                // PPUSCROLL register
                // On the first write, update the x-position
                // On the second, update the y-position
                // Keep track of which write it is using the w register (shared with PPUADDR)

                if (w == 0) {

                    t = (t & 0x7FE0) | (val >> 3);
                    fine_x_offset = (val & 0x7);
                    w = 1;

                } else {

                    t = (t & 0x7C1F) | ((val >> 3) << 5);
                    t = (t & 0x0FFF) | ((val & 0x7) << 12);
                    w = 0;
                }
                break;
            case 6:
                // w will be 0 on first write, 1 on second write
                if (w == 0) {
                    ppuaddr = val << 8;

                    t = (t & 0x40FF) | ((val & 0x3F) << 8);
                    t = t & 0x3FFF;
                    w = 1;
                } else {
                    ppuaddr |= val;

                    t = (t & 0x7F00) | val;
                    v = t;
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

                // If rendering is enabled, and the PPU is currently rendering,
                // the v register is updated in a weird way
                if (is_rendering_enabled() && (scanline < 240 || scanline == 261) ) {
                    // Both coarse x and coarse v are updated
                    increment_coarse_x(v);
                    increment_y(v);

                    // In case v overflows, wrap around by removing the 16th bit
                    v = v & 0x7FFF;
                    return;
                } else {
                    // If the PPU is not rendering, update v according to ppuctrl.increment_mode

                    if (ppuctrl.increment_mode == 0) {
                        v++;
                    } else {
                        v += 32;
                    }
                }

                break;
            default:
                break;
        }
    }
} 

// Communicate with PPU bus

uint8_t PPU::read_from_ppu(uint16_t address) {
    if (cartridge->read_ppu(address)) {
        // should handle the pattern table cases (addresses 0x0000 - 0x1FFF)

        if (cartridge->CHR_ROM.size() == 0) {
            return VRAM.at(address);
        }

        return cartridge->CHR_ROM.at(address);
    } else if (address >= 0x2000 && address <= 0x2FFF) {
        // handle nametables and mirroring
        address = map_to_nametable(address);
        return VRAM.at(address);
    } else if (address >= 0x3000 && address <= 0x3EFF) {
        // mirror of 0x2000 - 0x2EFF
        address = map_to_nametable(address - 0x1000);
        return VRAM.at(address);
    } else if (address >= 0x3F00 && address <= 0x3F1F) {
        // palette table
        // 0x3F00 - 0x3F1F is mirrored in the range 0x3F20 - 0x3FFF
        return PALETTE_RAM.at(address & 0x1F);
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
        VRAM.at(address) = val;
    } else if (address >= 0x2000 && address <= 0x2FFF) {
        // handle nametables and mirroring
        address = map_to_nametable(address);
        VRAM.at(address) = val;
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
            PALETTE_RAM.at((address & 0x1F) | 0x10) = val;
            PALETTE_RAM.at((address & 0x1F) & 0xEF) = val;
        } else {
            PALETTE_RAM.at(address & 0x1F) = val;
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

// See https://www.nesdev.org/wiki/PPU_scrolling#Wrapping_around for explanation
void PPU::increment_coarse_x(uint16_t& reg) const {
    if ((reg & 0x001F) == 31) {
        reg = reg & ~0x001F;
        reg = reg ^ 0x0400;
    } else {
        reg++;
    }
    return;
}

// See https://www.nesdev.org/wiki/PPU_scrolling#Wrapping_around for explanation
void PPU::increment_y(uint16_t& reg) const {
    if ((reg & 0x7000) != 0x7000) {
        reg += 0x1000;
    } else {
        reg &= ~0x7000;
        int y = (reg & 0x03E0) >> 5;

        if (y == 29) {
            y = 0;
            reg ^= 0x0800;
        } else if (y == 31) {
            y = 0;
        } else {
            y += 1;
        }

        reg = (reg & ~0x03E0) | (y << 5);
    }
}


void PPU::copy_x_pos_data() {
    uint8_t A = (t & 0x0400) >> 10;
    uint8_t BCDEF = t & 0x001F;

    v = (v & 0xFBFF) | (A << 10);
    v = (v & 0xFFE0) | BCDEF;

    return;
}

void PPU::copy_y_pos_data() {
    v = (v & 0x041F) | (t & 0x7BE0);
    return;
}

// We represent the process for sprite evaluation as a state machine.
// For more details, see https://www.nesdev.org/wiki/PPU_sprite_evaluation

// Sprite evaluation here is not exactly cycle accurate. Stages are transitioned through at the correct times,
// however sprite evaluation

void PPU::run_sprite_evaluation() {

    // Sprite evaluation only occurs on visible scanlines. If the PPU is not on a visible scanline, then the state machine should be idle.
    switch (cur_sprite_evaluation_stage) {
        case IDLE:
            if (scanline == 0 && cur_dot == 0) {
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
                    if (cur_sprite_y <= scanline && scanline < sprite_height + cur_sprite_y) {
                        // If it will be rendered, copy it into secondary OAM
                        std::copy(primary_OAM.begin() + 4 * n, primary_OAM.begin() + 4 * n + 4, secondary_OAM.begin() + 4 * num_sprites_found);
                        OAM_indices.at(num_sprites_found) = n;
                        num_sprites_found++;
                    }

                    if (num_sprites_found == 8) {
                        // Our secondary OAM is full now
                        n++;
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
                        if (scanline >= cur_sprite_y && scanline < cur_sprite_y + sprite_height) {
                            // If it is, another sprite could have been rendered this scanline. 
                            // Set sprite overflow flag accordingly.
                            bool sprite_overflow_conditions = 
                                 (ppumask.background_enable || ppumask.sprite_enable) &&
                                 cur_sprite_y < 240;

                            if (sprite_overflow_conditions) {
                                ppustatus.sprite_overflow = true;
                            }


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
                for (unsigned int i = 0; i < secondary_OAM.size(); i++) {
                    OAM_buffer.at(i) = secondary_OAM.at(i);
                }
            }


            if (cur_dot == 320) {
                num_sprites_found = 0;
                cur_sprite_evaluation_stage = STAGE_4;
            }
            
            break;
        case STAGE_4:
            // Cycles 321-340: Background render pipeline initialization

            // We already did this in previous stages though, so no need to do anything here

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
                // First dot skipped if the frame number is odd and rendering is enabled on an NTSC NES
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

                if (is_rendering_enabled()) {
                    if (cur_dot % 8 == 0 && ((cur_dot > 0 && cur_dot <= 256))) {
                        increment_coarse_x(v);
                    }

                    // At dot 256 of each scanline, the vertical component of v is incremented
                    if (cur_dot == 256) {
                        increment_y(v);
                    } else if (cur_dot == 257) {
                        copy_x_pos_data();
                    }

                    if (cur_dot >= 280 && cur_dot <= 304) {
                        copy_y_pos_data();
                    }
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
                    
                    // Pixels start rendering at dot 1
                    // Subtract 1 from cur_dot so that the first pixel is rendered correctly
                    uint16_t pixel_x = ((cur_dot - 1) % 256);
                    uint16_t pixel_y = scanline;

                    // In the binary representation of a tile, the first pixel on the left will be the MSB (bit 7)
                    uint8_t tile_offset_x = (7 - (pixel_x % 8));

                    // The last 3 bits of v contain the fine y offset of the screen
                    // AKA how many pixels down from the top of the tile are we shifted
                    uint8_t tile_offset_y = v >> 12;

                    // In the case where tile_offset_x < fine_x_offset, we should be rendering from the next horizontal tile.
                    // The v register is used for other things, so it's best not to change it.

                    // In the case where we are rendering from the next horizontal tile, offset_v stores the location for the next horizontal tile.
                    uint16_t offset_v = v;

                    // 
                    if (tile_offset_x < fine_x_offset) {
                        // Temporarily increment the coarse x of v to access the next tile
                        if ((offset_v & 0x001F) == 31) {
                            offset_v = offset_v & ~0x001F;
                            offset_v = offset_v ^ 0x0400;
                        } else {
                            offset_v++;
                        }
                        tile_offset_x += 8 - fine_x_offset;
                    } else {
                        tile_offset_x -= fine_x_offset;
                    }

                    // Fetch background data from nametable
                    uint8_t cur_nametable_entry = read_from_ppu(0x2000 | (offset_v & 0x0FFF));

                    uint16_t pattern_table_offset = 0;
    
                    if (ppuctrl.background_tile_select == 1) {
                        pattern_table_offset = 0x1000;
                    }
    
                    uint8_t background_pixel_layer_0 = read_from_ppu((cur_nametable_entry << 4) + tile_offset_y + pattern_table_offset);
                    uint8_t background_pixel_layer_1 = read_from_ppu((cur_nametable_entry << 4) + tile_offset_y + pattern_table_offset + 8);
    
                    uint8_t attribute_table_val = read_from_ppu(0x23C0 | (offset_v & 0x0C00) | ((offset_v >> 4) & 0x38) | ((offset_v >> 2) & 0x07));

                    uint8_t coarse_x = offset_v & 0x1F;
                    uint8_t coarse_y = (offset_v >> 5) & 0x1F;


                    bool is_left_tile = (coarse_x & 0x2) == 0;
                    bool is_top_tile = (coarse_y & 0x2) == 0;
    
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
    
                    if (((coarse_x & 0x3) == 0) || ((coarse_x & 0x2) == 0x2)) {
                        ui->set_background_palette(background_color0, background_color1, background_color2, background_color3);
                    }


                    // Start sprite rendering from secondary OAM

                    // First, check if there is a sprite rendered here

                    bool is_sprite_here = false;
                    bool is_sprite_0_rendered = false;

                    vector<Sprite> sprite_priority_order;

                    for (unsigned int i = 0; i < OAM_buffer.size(); i += 4) {
                        Sprite cur_sprite = Sprite(
                            OAM_buffer.at(i),
                            OAM_buffer.at(i + 1),
                            OAM_buffer.at(i + 2),
                            OAM_buffer.at(i + 3)
                        );

                        if (cur_sprite.x_position <= pixel_x && cur_sprite.x_position + 7 >= pixel_x) {
                            sprite_priority_order.push_back(cur_sprite);
                            is_sprite_here = true;

                            if (OAM_indices.at(i / 4) == 0) {
                                is_sprite_0_rendered = true;
                            }
                        }
                    }

                    unsigned int cur_sprite_index = 0;

                    uint8_t sprite_pixel_color;

                    while (cur_sprite_index < sprite_priority_order.size()) {
                        Sprite sprite_to_render = sprite_priority_order.at(cur_sprite_index);

                        // Get sprite offset from top, left
                        uint8_t sprite_offset_x = pixel_x - sprite_to_render.x_position;
                        uint8_t sprite_offset_y = pixel_y - sprite_to_render.y_position - 1;
                            
                        // Check if flip sprite vertically flag is set
                        if (is_bit_set(7, sprite_to_render.attributes)) {
                            sprite_offset_y = get_sprite_height() - sprite_offset_y - 1;
                        }
        
                        // Check if flip sprite horizontally flag is set
                        if (is_bit_set(6, sprite_to_render.attributes)) {
                            sprite_offset_x = SPRITE_WIDTH - sprite_offset_x - 1;
                        }
        
                        // Fetch sprite palette
                        uint8_t sprite_palette_num = sprite_to_render.attributes & 0x03;
                        uint16_t sprite_palette_index = 0x3F10 + 4 * sprite_palette_num;
        
                        uint8_t sprite_color0 = read_from_ppu(sprite_palette_index);
                        uint8_t sprite_color1 = read_from_ppu(sprite_palette_index + 1);
                        uint8_t sprite_color2 = read_from_ppu(sprite_palette_index + 2);
                        uint8_t sprite_color3 = read_from_ppu(sprite_palette_index + 3);
        
                        ui->set_sprite_palette(sprite_color0, sprite_color1, sprite_color2, sprite_color3);
    
                        uint16_t sprite_pattern_table_address;
    
                        if (get_sprite_height() == 16) {
                            // For 8x16 sprites, the pattern table is taken from the first bit of tile index number
                            sprite_pattern_table_address = (sprite_to_render.tile_index_number & 1) * 0x1000;
                        } else {
                            sprite_pattern_table_address = ppuctrl.sprite_tile_select ? 0x1000 : 0;
                        }

                        uint16_t sprite_pixel_layer0_address = sprite_offset_y + sprite_pattern_table_address;
                        uint16_t sprite_pixel_layer1_address = 8 + sprite_offset_y + sprite_pattern_table_address;

                        if (get_sprite_height() == 16) {
                            // Low bit of tile index number is 0 when fetching data for 8x16 sprites
                            sprite_pixel_layer0_address += 16 * (sprite_to_render.tile_index_number & 0xFE);
                            sprite_pixel_layer1_address += 16 * (sprite_to_render.tile_index_number & 0xFE);
                        } else {
                            sprite_pixel_layer0_address += 16 * sprite_to_render.tile_index_number;
                            sprite_pixel_layer1_address += 16 * sprite_to_render.tile_index_number;
                        }
        
        
                        uint8_t sprite_pixel_layer0 = read_from_ppu(sprite_pixel_layer0_address);
                        uint8_t sprite_pixel_layer1 = read_from_ppu(sprite_pixel_layer1_address);
        
                        uint8_t sprite_pixel_offset = 7 - (sprite_offset_x % 8);
                        sprite_pixel_color = is_bit_set(sprite_pixel_offset, sprite_pixel_layer0) | (is_bit_set(sprite_pixel_offset, sprite_pixel_layer1) << 1);


                        // If the sprite pixel is transparent, check if the next sprite in line has an opaque pixel to render
                        if (sprite_pixel_color == 0) {
                            cur_sprite_index++;
                            continue;
                        }

                        bool left_clipping_enabled = !ppumask.background_left_column_enable || !ppumask.sprite_left_column_enable;

                        // Check for sprite 0 cases
                        bool sprite_0_hit_possible = 
                            is_sprite_0_rendered &&
                            cur_sprite_index == 0 &&
                            ppumask.background_enable &&
                            ppumask.sprite_enable &&
                            pixel_x != 255 &&
                            pixel_y < 239 &&
                            !(left_clipping_enabled && pixel_x <= 7) &&
                            background_pixel_color != 0;
                        
                        if (sprite_0_hit_possible) {
                            ppustatus.sprite_hit = true;
                        }

                        break;
                    }

                    Sprite sprite_to_render;

                    if (cur_sprite_index < sprite_priority_order.size()) {
                        sprite_to_render = sprite_priority_order.at(cur_sprite_index);
                    } else {
                        is_sprite_here = false;
                    }


                    // Sprite pixel replaces background pixel only if:
                    // 1. Sprite pixel is opaque and has front priority, AND
                    // 2. Background pixel is transparent
                    
                    // For now, let's assume that BG or sprite pixel is transparent iff the pixel color is 0
                    // Is this a valid assumption?

                    bool is_background_left_clipped = !ppumask.background_left_column_enable && (pixel_x <= 7);
                    bool is_sprite_left_clipped = !ppumask.sprite_left_column_enable && (pixel_x <= 7);
                    
                    bool is_background_rendered = !is_background_left_clipped && ppumask.background_enable && background_pixel_color != 0;
                    bool is_sprite_in_front = !is_bit_set(5, sprite_to_render.attributes);
                    bool is_sprite_rendered = !is_sprite_left_clipped && ppumask.sprite_enable && scanline > 0 && is_sprite_here && ((sprite_pixel_color != 0 && is_sprite_in_front) || (background_pixel_color == 0));
                    
                    if (is_sprite_rendered) {
                        ui->set_pixel(pixel_y, pixel_x, sprite_pixel_color, false);
                    } else if (is_background_rendered) {
                        ui->set_pixel(pixel_y, pixel_x, background_pixel_color, true);
                    } else {
                        // If both background and sprite are transparent, set the pixel to the background color
                        // This is at mem. address 0x3F00 or 0x3F10 in PPU memory
                        // Which is index 0 or 16 in PALETTE_RAM
                        ui->set_pixel_color(pixel_y, pixel_x, PALETTE_RAM.at(0));
                    }

                }

                if (is_rendering_enabled()) {
                    if (cur_dot % 8 == 0 && ((cur_dot > 0 && cur_dot <= 256))) {
                        increment_coarse_x(v);
                    }

                    // At dot 256 of each scanline, the vertical component of v is incremented
                    if (cur_dot == 256) {
                        increment_y(v);
                    } else if (cur_dot == 257) {
                        copy_x_pos_data();
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

                if (is_rendering_enabled()) {
                    if (cur_dot % 8 == 0 && ((cur_dot > 0 && cur_dot <= 256))) {
                        increment_coarse_x(v);
                    }
    
                    // At dot 256 of each scanline, the vertical component of v is incremented
                    if (cur_dot == 256) {
                        increment_y(v);
                    } else if (cur_dot == 257) {
                        copy_x_pos_data();
                    }
                }

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

                if (is_rendering_enabled()) {
                    if (cur_dot % 8 == 0 && ((cur_dot > 0 && cur_dot <= 256))) {
                        increment_coarse_x(v);
                    }
    
                    // At dot 256 of each scanline, the vertical component of v is incremented
                    if (cur_dot == 256) {
                        increment_y(v);
                    } else if (cur_dot == 257) {
                        copy_x_pos_data();
                    }
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

}

void PPU::reset() {
    scanline = 0;
    cur_dot = 0;
    v = 0;
    t = 0;
    cur_ppu_rendering_stage = VISIBLE;
    cur_sprite_evaluation_stage = IDLE;
}

void PPU::attach_bus(Bus* b) {
    bus = b;
}