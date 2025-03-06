
#pragma once

#include "Mapper.h"


struct Mapper000 : Mapper {
    // PRG RAM is not supported on this mapper
    Mapper000(uint8_t prg_rom_banks, uint8_t prg_ram_banks, uint8_t chr_rom_banks) : Mapper(prg_rom_banks, 0, chr_rom_banks, 0) {};
    
    bool cpu_mapper_read(uint16_t addr, uint32_t& mapped_addr, uint8_t& data) override;
    bool cpu_mapper_write(uint16_t addr, uint32_t& mapped_addr, uint8_t data) override;
    bool ppu_mapper_read(uint16_t addr, uint32_t& mapped_addr) override;
    bool ppu_mapper_write(uint16_t addr, uint32_t& mapped_addr, uint8_t data) override;

    void reset() override;
    bool mapped_to_prg_ram(uint16_t addr);
    std::vector<uint8_t> get_prg_ram() override;

};