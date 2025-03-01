
#pragma once
#include <cstdint>


struct Mapper {
    Mapper(uint8_t, uint8_t);

    uint8_t num_prg_banks;
    uint8_t num_chr_banks;

    virtual bool cpu_mapper_read(uint16_t addr, uint32_t& mapped_addr) = 0;
    virtual bool cpu_mapper_write(uint16_t addr, uint32_t& mapped_addr, uint8_t data) = 0;
    virtual bool ppu_mapper_read(uint16_t addr, uint32_t& mapped_addr) = 0;
    virtual bool ppu_mapper_write(uint16_t addr, uint32_t& mapped_addr, uint8_t data) = 0;

    virtual void reset() = 0;

};