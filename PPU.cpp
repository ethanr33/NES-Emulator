
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

    if (cartridge->read_ppu(address, data)) {
        // do something
        // should handle the pattern table cases (addresses 0x0000 - 0x1FFF)
        return data;
    } else if (address >= 0x2000 && address <= 0x2FFF) {
        // handle nametables and mirroring
        address = map_to_nametable(address);
        return VRAM[address & 0x1FFF];
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
        // cannot write to CHR-ROM
    } else if (address >= 0x2000 && address <= 0x2FFF) {
        // handle nametables and mirroring
        address = map_to_nametable(address);
        VRAM[address & 0x1FFF] = val;
    } else if (address >= 0x3000 && address <= 0x3EFF) {
        // mirror of 0x2000 - 0x2EFF
        write_from_ppu(address & 0x2EFF, val);
    } else if (address >= 0x3F00 && address <= 0x3F1F) {
        // pallete table 
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

void PPU::render_cycle() {
    uint8_t step = cur_dot % 8;

    if (step == 1) {
        switch(ppuctrl.nametable_select) {
            case 0:
                nametable_entry = VRAM[map_to_nametable(0x2000 + (v & 0x3FF))];
                break;
            case 1:
                nametable_entry = VRAM[map_to_nametable(0x2400 + (v & 0x3FF))];
                break;
            case 2:
                nametable_entry = VRAM[map_to_nametable(0x2800 + (v & 0x3FF))];
                break;
            case 3:
                nametable_entry = VRAM[map_to_nametable(0x2C00 + (v & 0x3FF))];
                break;
            default:
                throw std::runtime_error("Unknown nametable selected " + std::to_string(ppuctrl.nametable_select) +  " when rendering");
                break;
        }
    } else if (step == 3) {
        attribute_table_entry = VRAM[v];
    } else if (step == 5) {
        if (ppuctrl.background_tile_select == 0) {
            pattern_low_byte = cartridge->CHR_ROM.at(nametable_entry);
        } else {
            pattern_low_byte = cartridge->CHR_ROM.at(nametable_entry + 0x1000);
        }
    } else if (step == 7) {
        if (ppuctrl.background_tile_select == 0) {
            pattern_low_byte = cartridge->CHR_ROM.at(nametable_entry + 8);
        } else {
            pattern_low_byte = cartridge->CHR_ROM.at(nametable_entry + 0x1000 + 8);
        }
    } else if (step == 0) {
        pattern_shift_reg_low = (pattern_shift_reg_low >> 8) | (pattern_low_byte << 8);
        pattern_shift_reg_high = (pattern_shift_reg_high >> 8) | (pattern_high_byte << 8);

        if (v & 0x001F == 31) {
            v &= 0x001F;
            v ^= 0x0400;
        } else {
            v += 1;
        }
    }

    if (cur_dot == 256) {
        if ((v & 0x7000) != 0x7000) {
            v += 0x1000;
        } else {
            v &= ~0x7000;
            int y = (v & 0x03E0) >> 5;
            if (y == 29) {
                y = 0;                          
                v ^= 0x0800;
            } else if (y == 31) {
                y = 0;
            } else {
                y += 1;
            }
            v = (v & ~0x03E0) | (y << 5);
        }
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
            // idle

            for (int i = 0x2000; i < 0x23C0; i++) {
                int tile_row = (i - 0x2000) / 0x20;
                int tile_col = i % 0x20;

                int attribute_table_index = (i - 0x2000) / 0xF;
                uint8_t attribute_table_val = read_from_ppu(0x23C0 + attribute_table_index);
                TILE_POSITION palette_area;
                uint8_t palette_number;

                if (tile_row % 4 == 0 || tile_row % 4 == 1) {
                    if (tile_col % 4 == 0 || tile_col % 4 == 1) {
                        palette_area = TOP_LEFT;
                    } else {
                        palette_area = TOP_RIGHT;
                    }
                } else {
                    if (tile_col % 4 == 0 || tile_col % 4 == 1) {
                        palette_area = BOTTOM_LEFT;
                    } else {
                        palette_area = BOTTOM_RIGHT;
                    }
                }

                switch (palette_area) {
                    case BOTTOM_LEFT:
                        palette_number = (attribute_table_val >> 4) & 0x3;
                        break;
                    case BOTTOM_RIGHT:
                        palette_number = (attribute_table_val >> 6) & 0x3;
                        break;
                    case TOP_LEFT:
                        palette_number = attribute_table_val & 0x3;
                        break;
                    case TOP_RIGHT:
                        palette_number = (attribute_table_val >> 2) & 0x3;
                        break;
                    default:
                        break;
                }

                uint16_t palette_base_address = 4 * palette_number;

                ui->set_palette(read_from_ppu(palette_base_address), read_from_ppu(palette_base_address + 1), read_from_ppu(palette_base_address + 2), read_from_ppu(palette_base_address + 3));

                uint16_t chr_index = read_from_ppu(i) << 4;

                if (ppuctrl.background_tile_select == 1) {
                    chr_index = chr_index | 0x1000;
                }

                for (int row = 0; row < 8; row++) {
                    uint8_t low_byte = cartridge->CHR_ROM[chr_index];
                    uint8_t high_byte = cartridge->CHR_ROM[chr_index + 8];

                    for (int col = 7; col >= 0; col--) {
                        bool low_bit = is_bit_set(col, low_byte);
                        bool high_bit = is_bit_set(col, high_byte);
                        uint8_t color_index = (high_bit << 1) | low_bit;
                        ui->set_pixel(8 * tile_row + row, 8 * tile_col + col, color_index);
                    }
                }
            }
            
        } else if (cur_dot >= 1 && cur_dot <= 256) {
           
        } else if (cur_dot >= 257 && cur_dot <= 320) {

        } else if (cur_dot >= 321 && cur_dot <= 336) {

        } else {

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