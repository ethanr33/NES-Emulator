
#pragma once

#include <vector>

#include "Mapper.h"

using std::vector;


struct Mapper001 : Mapper {

    // Power up state: Low bank uninitialized, high bank is last bank
    uint8_t prg_bank_low;
    uint8_t prg_bank_high = num_prg_rom_banks - 1;

    uint8_t prg_bank_32k = 0;

    uint8_t chr_bank_low;
    uint8_t chr_bank_high;

    uint8_t control_reg_write_bit = 0;
    uint8_t control_reg = 0;

    // Power up state: 
    // PRG ROM bank mode: 3
    uint8_t prg_rom_bank_mode = 3;
    uint8_t chr_rom_bank_mode;

    bool prg_ram_enabled = true;

    vector<uint8_t> PRG_RAM;

    Mapper001(uint8_t prg_rom_banks, uint8_t prg_ram_banks, uint8_t chr_rom_banks) : Mapper(prg_rom_banks, prg_ram_banks, chr_rom_banks, 0x2000) {
        PRG_RAM = vector<uint8_t>(num_prg_ram_banks * prg_ram_bank_size);
    };
    
    bool cpu_mapper_read(uint16_t addr, uint32_t& mapped_addr, uint8_t& data) override;
    bool cpu_mapper_write(uint16_t addr, uint32_t& mapped_addr, uint8_t data) override;
    bool ppu_mapper_read(uint16_t addr, uint32_t& mapped_addr) override;
    bool ppu_mapper_write(uint16_t addr, uint32_t& mapped_addr, uint8_t data) override;

    void reset() override;
    bool mapped_to_prg_ram(uint16_t addr) override;
    vector<uint8_t> get_prg_ram() override;

    void switch_banks_prg(uint8_t);
    void switch_banks_chr(uint8_t, uint8_t);

};