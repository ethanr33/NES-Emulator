#pragma once
#include <cstdint>
#include <vector>

#include "Mapper.h"


struct Mapper003 : Mapper {
    // Fixed sized 32K PRG ROM, no PRG RAM
    Mapper003(uint8_t num_prg_banks, uint8_t num_chr_banks) : Mapper(1, 0, num_chr_banks, 0) {}

    uint8_t cur_chr_bank = 0;

    bool cpu_mapper_read(uint16_t addr, uint32_t& mapped_addr, uint8_t& data) override;
    bool cpu_mapper_write(uint16_t addr, uint32_t& mapped_addr, uint8_t data) override;
    bool ppu_mapper_read(uint16_t addr, uint32_t& mapped_addr) override;
    bool ppu_mapper_write(uint16_t addr, uint32_t& mapped_addr, uint8_t data) override;

    void reset() override;
    bool mapped_to_prg_ram(uint16_t addr) override;
    std::vector<uint8_t> get_prg_ram() override; // For debugging purposes only
};