
#include <stdexcept>
#include <iostream>
#include "Mapper001.h"

bool Mapper001::cpu_mapper_read(uint16_t addr, uint32_t& mapped_addr, uint8_t& data) {

    if (mapped_to_prg_ram(addr)) {
        // 8 KB PRG-RAM bank, (optional)
        data = PRG_RAM.at(addr - 0x6000);
        return false; // Not reading from cartridge, so return true
    }

    if ((prg_rom_bank_mode == 0 || prg_rom_bank_mode == 1) && addr >= 0x8000 && addr <= 0xFFFF) {
        mapped_addr = (addr - 0x8000) + prg_bank_32k * 0x8000;
        return true;
    }

    if (addr >= 0x8000 && addr <= 0xBFFF) {
        // 16 KB PRG-ROM bank, either switchable or fixed to the first bank
        mapped_addr = (addr - 0x8000) + prg_bank_low * 0x4000;
        return true;
    }

    if (addr >= 0xC000 && addr <= 0xFFFF) {
        // 16 KB PRG-ROM bank, either fixed to the last bank or switchable
        mapped_addr = (addr - 0xC000) + prg_bank_high * 0x4000;
        return true;
    }

    return false;
}

bool Mapper001::cpu_mapper_write(uint16_t addr, uint32_t& mapped_addr, uint8_t data) {

    if (addr < 0x6000) {
        // Not handled by mapper
        return false;
    }

    if (mapped_to_prg_ram(addr)) {
        PRG_RAM.at(addr - 0x6000) = data;
        return true; // We write to cartridge, so return true
    } else if (addr >= 0x6000 && addr <= 0x7FFF) {
        return false;
    }

    if (data & 0x80) {
        // Reset
        reset();
    } else {
        control_reg = (control_reg >> 1) | ((data & 0x1) << 4);
        control_reg_write_bit++;        

        if (control_reg_write_bit == 5) {
            if (addr >= 0x8000 && addr <= 0x9FFF) {
                prg_rom_bank_mode = (control_reg & 0xC) >> 2;
                chr_rom_bank_mode = control_reg >> 4;
            } else if (addr >= 0xA000 && addr <= 0xBFFF) {
                switch_banks_chr(control_reg, 0);
            } else if (addr >= 0xC000 && addr <= 0xDFFF) {
                switch_banks_chr(control_reg, 1);
            } else if (addr >= 0xE000 && addr <= 0xFFFF) {
                switch_banks_prg(control_reg & 0xF);
                prg_ram_enabled = (control_reg & 0x10) == 0;
            }
            
            control_reg_write_bit = 0;
            control_reg = 0;
        }
    }

    // We never edit PRG ROM
    return false;
}

bool Mapper001::ppu_mapper_read(uint16_t addr, uint32_t& mapped_addr) {

    if (addr >= 0x2000) {
        return false;
    }

    if (num_chr_banks == 0) {
        return true;
    }

    if (addr >= 0x0000 && addr <= 0x0FFF) {
        mapped_addr = addr + 0x1000 * chr_bank_low;
        return true;
    } else if (addr >= 0x1000 && addr <= 0x1FFF) {
        mapped_addr = (addr - 0x1000) + 0x1000 * chr_bank_high;
        return true;
    }

    return false;
}

bool Mapper001::ppu_mapper_write(uint16_t addr, uint32_t& mapped_addr, uint8_t data) {

    // Check if we are in range of CHR data
    if (addr >= 0x2000) {
        return false;
    }
    
    // If there are no CHR-ROM banks then we are using CHR-RAM
    if (num_chr_banks == 0) {
        return true;
    }

    return false;
}

void Mapper001::reset() {
    // A write with bit set will reset shift register and write Control with (Control OR $0C), 
    // locking PRG-ROM at $C000-$FFFF to the last bank.

    control_reg_write_bit = 0;
    control_reg = 0;

    prg_bank_high = num_prg_rom_banks - 1;
    prg_rom_bank_mode = 3;
}

bool Mapper001::mapped_to_prg_ram(uint16_t addr) {
    return addr >= 0x6000 && addr <= 0x7FFF && prg_ram_enabled;
}

void Mapper001::switch_banks_prg(uint8_t bank_num) {
    if (prg_rom_bank_mode == 0 || prg_rom_bank_mode == 1) {
        // 0, 1: switch 32 KB at $8000, ignoring low bit of bank number
        prg_bank_32k = bank_num >> 1;
    } else if (prg_rom_bank_mode == 2) {
        // 2: fix first bank at $8000 and switch 16 KB bank at $C000
        prg_bank_low = 0;
        prg_bank_high = bank_num;
    } else if (prg_rom_bank_mode == 3) {
        // 3: fix last bank at $C000 and switch 16 KB bank at $8000
        prg_bank_low = bank_num;
        prg_bank_high = num_prg_rom_banks - 1;
    } else {
        throw std::runtime_error("Mapper001: Unknown PRG rom bank mode");
    }
}

void Mapper001::switch_banks_chr(uint8_t new_bank_num, uint8_t which_bank) {
    // which_bank == 0: Switch low
    // which_bank == 1: Switch high

    if (which_bank == 0) {
        if (chr_rom_bank_mode == 0) {
            // 1 8 KiB bank
            chr_bank_low = new_bank_num;
        } else {
            // 2 4 KiB banks
            chr_bank_low = new_bank_num;
            chr_bank_high = new_bank_num + 1;
        }
    } else {
        if (chr_rom_bank_mode == 0) {
            chr_bank_high = new_bank_num;
        }
    }
}

vector<uint8_t> Mapper001::get_prg_ram() {
    return PRG_RAM;
}