
#include "PPU.h"
#include <iostream>

using namespace std;

PPU::PPU() {
    ui = new UI();
}

PPU::PPU(bool ui_disabled) {
    ui = new UI(ui_disabled);
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
                uint8_t res = 0x0;
                res = ppustatus.serialize();
                ppustatus.vblank = false;
                return res;
            }
            case 3:
                throw std::runtime_error("Attempted to read from OAMADDR register");
                break;
            case 4:
                break;
            case 5:
                throw std::runtime_error("Attempted to read from PPUSCROLL register");
                break;
            case 6:
                throw std::runtime_error("Attempted to read from PPUADDR register");
                break;
            case 7: {
                uint8_t data = read_from_ppu(ppuaddr);

                if (ppuctrl.increment_mode == 0) {
                    ppuaddr++;
                } else {
                    ppuaddr += 32;
                }

                return data;
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
        return read_from_ppu(address & 0x2EFF);
    } else if (address >= 0x3F00 && address <= 0x3F1F) {
        // pallete table 
        return PALETTE_RAM[address & 0x3F];
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
    } else if (address >= 0x3F00 && address <= 0x3F1F) {
        // pallette table 
        PALETTE_RAM[address & 0x3F] = val;
    }

    return;
}

void PPU::load_cartridge(Cartridge* new_cartridge) {
    cartridge = new_cartridge;
} 

void PPU::load_OAMDMA(uint8_t high_byte, uint8_t data[]) {
    // Copy over a page of data starting at address

    for (int i = 0; i < 256; i++) {
        OAM[i] = data[(high_byte << 8) + i];
    }
}

void PPU::tick() {
    // TODO: Add scanline specific actions

    if (scanline == 261) {
        // pre render scanline
        // load first two tiles into buffer
        ppustatus.vblank = false;
    } else if (scanline >= 0 && scanline <= 239) {
        // visible scanlines

        if (cur_dot == 0) {
            // Idle cycle
        } else if (cur_dot >= 4) {
            // Due to rendering pipeline delays, first pixel is rendered on cycle 4
            uint16_t pixel_x = (cur_dot - 4) % 256;
            uint16_t pixel_y = scanline;

            // In the binary representation of a tile, the first pixel will be the MSB (bit 7)
            uint8_t tile_offset_x = 7 - (pixel_x % 8);
            uint8_t tile_offset_y = pixel_y % 8;

            // Fetch data from nametable

            // TODO: Add support for mirroring types
            // Who cares if we fetch the same data 8 times in a row, right???

            uint16_t nametable_index = 32 * (pixel_y / 8) + (pixel_x / 8);
            uint8_t cur_nametable_entry = read_from_ppu(0x2000 + nametable_index); // FIX THIS LATER

            if (ppuctrl.sprite_tile_select == 1) {
                cur_nametable_entry += 0x1000;
            }

            uint8_t pixel_layer_0 = read_from_ppu(16 * cur_nametable_entry + tile_offset_y);
            uint8_t pixel_layer_1 = read_from_ppu(16 * cur_nametable_entry + tile_offset_y + 8);

            uint16_t attribute_table_index = 8 * (pixel_y / 32) + (pixel_x / 32);
            uint8_t attribute_table_val = read_from_ppu(0x23C0 + attribute_table_index); // Attribute table is located at the end of the nametable

            bool is_left_tile = (pixel_x % 16) < 8;
            bool is_top_tile = (pixel_y % 16) < 8;

            // The value in the attribute table is constructed as follows:
            // attribute_table_val = (bottomright << 6) | (bottomleft << 4) | (topright << 2) | (topleft << 0)
            // Where bottomright, bottomleft, topright, and topleft are the palette numbers for each quadrant of this block in the nametable
            uint8_t color_palette_num;

            if (is_top_tile && is_left_tile) {
                // top left
                color_palette_num = attribute_table_val & 0x3;
            } else if (is_top_tile && !is_left_tile) {
                // top right
                color_palette_num = (attribute_table_val & 0xC) >> 2;
            } else if (!is_top_tile && is_left_tile) {
                // bottom left
                color_palette_num = (attribute_table_val & 0x30) >> 4;
            } else {
                // bottom right
                color_palette_num = (attribute_table_val & 0xC0) >> 6;
            }

            // For background rendering only.
            // Palette addresses for the background are from 0x3F00 - 0x3F0F.
            uint16_t palette_index = 0x3F00 + 4 * color_palette_num;
            
            uint8_t color0 = read_from_ppu(palette_index);
            uint8_t color1 = read_from_ppu(palette_index + 1);
            uint8_t color2 = read_from_ppu(palette_index + 2);
            uint8_t color3 = read_from_ppu(palette_index + 3);

            uint8_t pixel_color = (is_bit_set(tile_offset_x, pixel_layer_1) << 1) | is_bit_set(tile_offset_x, pixel_layer_0);

            if (pixel_color == 3) {
                int x = 2;
            }

            ui->set_palette(color0, color1, color2, color3);

            ui->set_pixel(pixel_y, pixel_x, pixel_color);

        }

    } else if (scanline == 240) {
        // post render scanline
        if (cur_dot == 340) {
            ui->tick();
        }
    } else if (scanline > 240) {
        // vertical blanking scanlines
        ppustatus.vblank = true;
    } else {
        throw std::runtime_error("Invalid scanline");
    }

    cur_dot++;

    if (cur_dot == 341) {
        cur_dot = 0;

        scanline++;

        if (scanline == 262) {
            scanline = 0;
        }
    }

}

void PPU::reset() {
    scanline = 261;
    cur_dot = 0;
}