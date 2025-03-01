
#pragma once
#include "Mapper.h"


struct Mapper001 : Mapper {

    // Power up state: Low bank uninitialized, high bank is last bank
    uint8_t prg_bank_low;
    uint8_t prg_bank_high = num_prg_banks - 1;

    uint8_t chr_bank_low;
    uint8_t chr_bank_high;

    uint8_t control_reg_write_bit = 0;
    uint8_t control_reg = 0;

    // Power up state: 
    // PRG ROM bank mode: 3
    uint8_t prg_rom_bank_mode = 3;
    uint8_t chr_rom_bank_mode;

    Mapper001(uint8_t prg_rom, uint8_t chr_rom) : Mapper(prg_rom, chr_rom) {};
    
    bool cpu_mapper_read(uint16_t addr, uint32_t& mapped_addr) override;
    bool cpu_mapper_write(uint16_t addr, uint32_t& mapped_addr, uint8_t data) override;
    bool ppu_mapper_read(uint16_t addr, uint32_t& mapped_addr) override;
    bool ppu_mapper_write(uint16_t addr, uint32_t& mapped_addr, uint8_t data) override;

    void reset() override;
    void switch_banks_prg(uint8_t);
    void switch_banks_chr(uint8_t, uint8_t);

};