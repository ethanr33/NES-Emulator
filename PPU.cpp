
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

// void PPU::filter_renderable_sprites() {

//     // Clear list to get rid of previously rendered sprites
//     OAM_renderable_sprites.clear();

//     // Maps a scanline number to how many sprites we've seen so far that will be rendered on that scanline number
//     vector<uint8_t> num_sprites_on_scanline(256);
//     uint8_t sprite_height = ppuctrl.sprite_height ? 16 : 8;

//     for (int i = 0; i < MAX_SPRITES; i++) {
//         Sprite cur_sprite = OAM_sprite_list.at(i);
//         uint8_t cur_sprite_scanline = cur_sprite.y_position;

//         // If this sprite is out of range of drawing on the screen, then don't render the sprite
//         if (cur_sprite_scanline >= VISIBLE_SCANLINES_PER_CYCLE) {
//             continue;
//         }

//         // Check how many sprites are being rendered on this scanline.
//         // If there are eight or more, then don't render the sprite.
//         // Continue evaluation onto the next sprite.
//         if (num_sprites_on_scanline.at(cur_sprite_scanline) >= 8) {
//             continue;
//         }

//         // The sprite must be rendered from scanline i to i + sprite_height - 1.
//         // Update map accordingly.
//         for (int j = 0; j < sprite_height; j++) {
//             num_sprites_on_scanline.at(cur_sprite_scanline + j)++;
//             OAM_renderable_sprites.push_back(cur_sprite);
//         }

//     }
// }

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

bool PPU::run_sprite_evaluation() {
    // // Sprite evaluation only occurs on visible scanlines
    // switch (cur_sprite_evaluation_stage) {
    //     case IDLE:
    //         if (scanline == 261 && cur_dot == 340) {
    //             cur_sprite_evaluation_stage = STAGE_1;
    //         }
            
    //         return true;
    //         break;
    //     case STAGE_1:
    //         // Secondary OAM is initialized to $FF
    //         secondary_OAM.assign(SECONDARY_OAM_SIZE, 0xFF);
    //         if (cur_dot == 64) {
    //             cur_sprite_evaluation_stage = STAGE_2_1;
    //         }
            
    //         return true;
    //         break;
    //     case STAGE_2_1:
    //         {
    //             bool is_done_here = true;

    //             if (cur_dot % 2 == 1) {
    //                 primary_oam_buffer = primary_OAM.at(4 * n);
    //                 return true;
    //             } else {
    
    //                 if (secondary_oam_write_enabled) {
    //                     secondary_OAM.at(4 * n) = primary_oam_buffer;
    //                 }
    
    //                 uint8_t sprite_height = get_sprite_height();
    //                 uint8_t cur_sprite_y = primary_oam_buffer;
    
    //                 // Check if sprite y-coordinate is in range to be rendered here
    //                 // Note that we check if this sprite is rendered on the NEXT scanline, hence scanline + 1.
    //                 if (scanline + 1 <= cur_sprite_y + sprite_height && scanline + 1 >= cur_sprite_y) {
    //                     cur_sprite_evaluation_stage = STAGE_2_1a;
    //                     num_sprites_found++;
    //                     m++;
    //                 } else {
    //                     // This sprite is not on the next scanline, so look at the next one
    //                     cur_sprite_evaluation_stage = STAGE_2_2;
    //                     is_done_here = false;
    //                 }
    
    
    //             }
    
    //             if (cur_dot == 256) {
    //                 cur_sprite_evaluation_stage = STAGE_3;
    //             }
    
    //             return is_done_here;
    //             break;
    //         }
    //     case STAGE_2_1a:
    //         if (cur_dot % 2 == 1) {
    //             primary_oam_buffer = primary_OAM.at(4 * n + m);
    //         } else {
    //             if (secondary_oam_write_enabled) {
    //                 secondary_OAM.at(4 * n + m) = primary_oam_buffer;
    //             }
    //             m++;
    //         }

    //         if (m == 4) {
    //             m = 0;
    //             cur_sprite_evaluation_stage = STAGE_2_2;
    //             return false;
    //         } else {
    //             return true;
    //         }
    //         break;
    //     case STAGE_2_2:
    //         n++;

    //         if (n == 64) {
    //             // Case: If all 64 sprites have been evaluated
    //             cur_sprite_evaluation_stage = STAGE_2_4;
    //         } else if (num_sprites_found < 8) {
    //             cur_sprite_evaluation_stage = STAGE_2_1;
    //             return true;
    //         } else if (num_sprites_found == 8) {
    //             secondary_oam_write_enabled = false;
    //             cur_sprite_evaluation_stage = STAGE_2_3;
    //             m = 0;
    //         } else {
    //             cur_sprite_evaluation_stage = STAGE_2_3;
    //             m = 0;
    //         }
    //         break;
    //     case STAGE_2_3:
    //         // Starting at m = 0, evaluate OAM[n][m] as a Y-coordinate

    //         if (cur_dot % 2 == 1) {
    //             primary_oam_buffer = primary_OAM.at(4 * n + m);
    //             return true;
    //         } else {
    //             uint8_t sprite_height = get_sprite_height();
    //             uint8_t cur_sprite_y = primary_oam_buffer;

    //             //If the value is in range, set the sprite overflow flag in $2002 and read the next 3 entries of OAM 
    //             if (scanline + 1 <= cur_sprite_y + sprite_height && scanline + 1 >= cur_sprite_y) {
    //                 cur_sprite_evaluation_stage = STAGE_2_3a;
    //                 start_m = m;
    //                 return false;
    //             } else {
    //                 cur_sprite_evaluation_stage = STAGE_2_3b;
    //                 return false;
    //             }
    //         }

    //         break;
    //     case STAGE_2_3a:
    //         ppustatus.sprite_overflow = true;

    //         if (cur_dot % 2 == 1) {
    //             primary_oam_buffer = primary_OAM.at(4 * n + m);
    //             m++;

    //             if (m == 4) {
    //                 m = 0;
    //                 n++;
    //             }
    //         }

    //         // We are done after 3 reads
    //         if (m == (start_m + 3) % 4) {
    //             cur_sprite_evaluation_stage = STAGE_2_3;
    //             return false;
    //         } else {
    //             return true;
    //         }

    //         break;
    //     case STAGE_2_3b:
    //         n++;
    //         m++;

    //         if (m == 4) {
    //             m = 0;
    //         }

    //         if (n == 64) {
    //             cur_sprite_evaluation_stage = STAGE_2_4;
    //         } else {
    //             cur_sprite_evaluation_stage = STAGE_2_3;
    //         }

    //         return true;
    //         break;
    //     case STAGE_2_4:
    //         // Increment n here until HBLANK
    //         if (cur_dot == 256) {
    //             cur_sprite_evaluation_stage = STAGE_3;
    //         }

    //         return true;
    //         break;
    //     case STAGE_3:
    //         // Sprite fetches (8 sprites total, 8 cycles per sprite)

    //         if (cur_dot == 320) {
    //             cur_sprite_evaluation_stage = STAGE_4;
    //         }
            
    //         return true;
    //         break;
    //     case STAGE_4:
    //         // Cycles 321-340+0: Background render pipeline initialization

    //         if (cur_dot == 340) {
    //             if (scanline < 239) {
    //                 cur_sprite_evaluation_stage = STAGE_1;
    //             } else {
    //                 cur_sprite_evaluation_stage = IDLE;
    //             }
    //         }
            
    //         return true;
    //         break;
    //     default:
    //         return true;
    //         break;
    // }
}

void PPU::tick() {
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
                }

                // OAMADDR is set to 0 during ticks 257-320 of prerender scanlines
                if (cur_dot >= 257 && cur_dot <= 320) {
                    oamaddr = 0;
                }

                if (cur_dot == 341) {
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
                    ui->set_pixel(pixel_y, pixel_x, background_pixel_color, true);   
                }

                if (cur_dot == 341) {
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
                if (cur_dot == 341) {
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
                    ui->tick();
                    frames_elapsed++;
                }

                if (scanline == 260 && cur_dot == 340) {
                    scanline++;
                    cur_dot = 0;
                    cur_ppu_rendering_stage = PRE_RENDER;
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

// void PPU::tick() {
//     // TODO: Add scanline specific actions

//     if (scanline == 261) {
//         // pre render scanline

//         // First dot skipped if the frame number is odd and rendering is enabled
//         if (cur_dot == 0 && (frames_elapsed % 2 == 1) && (ppumask.background_enable || ppumask.sprite_enable)) {
//             cur_dot++;
//         }

//         // VBlank and other flags are always cleared on dot 1
//         if (cur_dot == 1) {
//             // Doesn't really matter where we clear the race condition, as long as it's cleared before it can happen again
//             bus->set_nmi_line(false);
//             bus->set_nmi_suppression_status(false);
//             ppustatus_vblank_read_race_condition = false;
//             ppustatus.vblank = false;
//             ppustatus.sprite_hit = false;
//             ppustatus.sprite_overflow = false;
//         }

//         // OAMADDR is set to 0 during ticks 257-320 of prerender scanlines
//         if (cur_dot >= 257 && cur_dot <= 320) {
//             oamaddr = 0;
//         }
        
//     } else if (scanline >= 0 && scanline <= 239) {
//         // visible scanlines

//         if (cur_dot == 0) {
//             // Idle cycle
//         } else if (cur_dot >= 4) {

//             // OAMADDR is set to 0 during ticks 257-320 of visible scanlines
//             if (cur_dot >= 257 && cur_dot <= 320) {
//                 oamaddr = 0;
//             }

//             // Due to rendering pipeline delays, first pixel is rendered on cycle 4
//             uint16_t pixel_x = (cur_dot - 4) % 256;
//             uint16_t pixel_y = scanline;

//             // In the binary representation of a tile, the first pixel will be the MSB (bit 7)
//             uint8_t tile_offset_x = 7 - (pixel_x % 8);
//             uint8_t tile_offset_y = pixel_y % 8;

//             // Fetch data from nametable

//             // TODO: Add support for mirroring types
//             // Who cares if we fetch the same data 8 times in a row, right???

//             uint16_t background_nametable_index = 32 * (pixel_y / 8) + (pixel_x / 8);
//             uint8_t cur_nametable_entry = read_from_ppu(0x2000 + 0x400 * ppuctrl.nametable_select + background_nametable_index); // FIX THIS LATER

//             uint16_t pattern_table_offset = 0;

//             if (ppuctrl.background_tile_select == 1) {
//                 pattern_table_offset = 0x1000;
//             }

//             uint8_t background_pixel_layer_0 = read_from_ppu(16 * cur_nametable_entry + tile_offset_y + pattern_table_offset);
//             uint8_t background_pixel_layer_1 = read_from_ppu(16 * cur_nametable_entry + tile_offset_y + pattern_table_offset + 8);

//             uint16_t attribute_table_index = 8 * (pixel_y / 32) + (pixel_x / 32);
//             uint8_t attribute_table_val = read_from_ppu(0x23C0 + 0x400 * ppuctrl.nametable_select + attribute_table_index); // Attribute table is located at the end of the nametable

//             bool is_left_tile = (pixel_x % 32) < 16;
//             bool is_top_tile = (pixel_y % 32) < 16;

//             // The value in the attribute table is constructed as follows:
//             // attribute_table_val = (bottomright << 6) | (bottomleft << 4) | (topright << 2) | (topleft << 0)
//             // Where bottomright, bottomleft, topright, and topleft are the palette numbers for each quadrant of this block in the nametable
//             uint8_t background_color_palette_num;

//             if (is_top_tile && is_left_tile) {
//                 // top left
//                 background_color_palette_num = attribute_table_val & 0x3;
//             } else if (is_top_tile && !is_left_tile) {
//                 // top right
//                 background_color_palette_num = (attribute_table_val & 0xC) >> 2;
//             } else if (!is_top_tile && is_left_tile) {
//                 // bottom left
//                 background_color_palette_num = (attribute_table_val & 0x30) >> 4;
//             } else {
//                 // bottom right
//                 background_color_palette_num = (attribute_table_val & 0xC0) >> 6;
//             }

//             // For background rendering only.
//             // Palette addresses for the background are from 0x3F00 - 0x3F0F.
//             uint16_t background_palette_index = 0x3F00 + 4 * background_color_palette_num;
            
//             uint8_t background_color0 = read_from_ppu(background_palette_index);
//             uint8_t background_color1 = read_from_ppu(background_palette_index + 1);
//             uint8_t background_color2 = read_from_ppu(background_palette_index + 2);
//             uint8_t background_color3 = read_from_ppu(background_palette_index + 3);

//             uint8_t background_pixel_color = (is_bit_set(tile_offset_x, background_pixel_layer_1) << 1) | is_bit_set(tile_offset_x, background_pixel_layer_0);

//             ui->set_background_palette(background_color0, background_color1, background_color2, background_color3);
//             ui->set_pixel(pixel_y, pixel_x, background_pixel_color, true);    


//             // Sprite rendering logic

//             // Run sprite evaluation state machine
//             // while (!run_sprite_evaluation());

//             // Sprite rendering does not occur on the first scanline
//             // if (scanline > 0) {
//             //     bool is_sprite_rendered = false;
//             //     Sprite sprite_to_render;
    
//             //     for (int i = 0; i < secondary_OAM.size(); i += 4) {
//             //         Sprite cur_sprite = Sprite(secondary_OAM.at(i), secondary_OAM.at(i + 1), secondary_OAM.at(i + 2), secondary_OAM.at(i + 3));
                    
//             //         // Check if the pixel is within the x bounds of the sprite
//             //         if (cur_sprite.x_position <= pixel_x && cur_sprite.x_position + SPRITE_WIDTH > pixel_x) {
//             //             // Check if the pixel is within the y bounds of the sprite
//             //             if (cur_sprite.y_position <= pixel_y && cur_sprite.y_position + get_sprite_height() > pixel_y) {
//             //                 is_sprite_rendered = true;
//             //                 sprite_to_render = cur_sprite;
//             //                 break; // Only one sprite to render at a time
//             //                 // TODO: Implement sprite priority
//             //             }
//             //         }
//             //     }


//             //     if (is_sprite_rendered) {
//             //         // Get pixel offset from top, left of sprite
//             //         uint8_t sprite_offset_x = pixel_x - sprite_to_render.x_position;
//             //         uint8_t sprite_offset_y = pixel_y - sprite_to_render.y_position;
    
//             //         // Check if flip sprite vertically flag is set
//             //         if (is_bit_set(7, sprite_to_render.attributes)) {
//             //             sprite_offset_y = get_sprite_height() - sprite_offset_y - 1;
//             //         }
    
//             //         // Check if flip sprite horizontally flag is set
//             //         if (is_bit_set(6, sprite_to_render.attributes)) {
//             //             sprite_offset_x = SPRITE_WIDTH - sprite_offset_x - 1;
//             //         }
    
//             //         // Check if the sprite is rendered in front of or behind the background
//             //         bool is_sprite_behind_background = is_bit_set(6, sprite_to_render.attributes);
    
//             //         // Fetch sprite palette
//             //         uint8_t sprite_palette_num = sprite_to_render.attributes & 0x03;
//             //         uint16_t sprite_palette_index = 0x3F10 + 4 * sprite_palette_num;
    
//             //         uint8_t sprite_color0 = read_from_ppu(sprite_palette_index);
//             //         uint8_t sprite_color1 = read_from_ppu(sprite_palette_index + 1);
//             //         uint8_t sprite_color2 = read_from_ppu(sprite_palette_index + 2);
//             //         uint8_t sprite_color3 = read_from_ppu(sprite_palette_index + 3);
    
//             //         ui->set_sprite_palette(sprite_color0, sprite_color1, sprite_color2, sprite_color3);
    
//             //         if (get_sprite_height() == 16) {
//             //             throw std::runtime_error("Rendering for 8x16 sprites has not been implemented yet");
//             //         }
    
//             //         uint8_t sprite_pattern_table_address = ppuctrl.sprite_tile_select ? 0x1000 : 0;
    
//             //         uint8_t sprite_pixel_layer0 = read_from_ppu(sprite_offset_y + 16 * sprite_to_render.tile_index_number + sprite_pattern_table_address);
//             //         uint8_t sprite_pixel_layer1 = read_from_ppu(8 + sprite_offset_y + 16 * sprite_to_render.tile_index_number + sprite_pattern_table_address);
    
//             //         uint8_t sprite_pixel_offset = 7 - (sprite_offset_x % 8);
//             //         uint8_t sprite_pixel_color = is_bit_set(sprite_pixel_offset, sprite_pixel_layer0) | (is_bit_set(sprite_pixel_offset, sprite_pixel_layer1) << 1);
    
//             //         // If the sprite's color is transparent, do not render it
//             //         if (sprite_pixel_color == 0) {
//             //             ui->set_pixel(pixel_y, pixel_x, background_pixel_color, true);    
//             //         } else {
//             //             ui->set_pixel(pixel_y, pixel_x, sprite_pixel_color, false);    
//             //         }
    
//             //     } else {
//             //         // If there is no sprite here, render background as normal
//             //         ui->set_pixel(pixel_y, pixel_x, background_pixel_color, true);    
//             //     }

//             // }

//         }

//     } else if (scanline == 240) {
//         // post render scanline
//     } else if (scanline > 240) {
//         // vertical blanking scanlines

//         if (!ppustatus_vblank_read_race_condition && scanline == 241 && cur_dot == 1) {
//             ppustatus.vblank = true;
//         }

//         // The PPU pulls /NMI low if and only if both vblank and nmi_enable are true
//         if (ppustatus.vblank && ppuctrl.nmi_enable) {
//             bus->set_nmi_line(true);
//         } else {
//             bus->set_nmi_line(false);
//         }


//         if (scanline == 260 && cur_dot == 340) {
//             ui->tick();

//             // for (int i = 0; i < 30; i++) {
//             //     for (int j = 0; j < 32; j++) {
//             //         std::cout << std::hex << (int) read_from_ppu(0x2000 + 32 * i + j) << " ";
//             //     }
//             //     std::cout << std::endl;
//             // }


//             frames_elapsed++;
//         }
//     } else {
//         throw std::runtime_error("Invalid scanline");
//     }

//     cur_dot++;

//     if (cur_dot == 341) {
//         cur_dot = 0;

//         scanline++;

//         if (scanline == 262) {
//             scanline = 0;
//         }
//     }

// }

void PPU::reset() {
    scanline = 261;
    cur_dot = 0;
    cur_ppu_rendering_stage = PRE_RENDER;
}

void PPU::attach_bus(Bus* b) {
    bus = b;
}