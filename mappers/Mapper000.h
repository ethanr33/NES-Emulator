
#pragma once
#include "Mapper.h"


struct Mapper000 : Mapper {
    Mapper000(uint8_t prg_rom, uint8_t chr_rom) : Mapper(prg_rom, chr_rom) {};
    
    bool cpu_mapper_read(uint16_t addr, uint32_t& mapped_addr) override;
    bool cpu_mapper_write(uint16_t addr, uint32_t& mapped_addr, uint8_t data) override;
    bool ppu_mapper_read(uint16_t addr, uint32_t& mapped_addr) override;
    bool ppu_mapper_write(uint16_t addr, uint32_t& mapped_addr, uint8_t data) override;

    void reset() override;

};