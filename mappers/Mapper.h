
#pragma once
#include <cstdint>
#include <vector>


struct Mapper {
    Mapper(uint8_t, uint8_t, uint8_t, uint16_t);

    uint8_t num_prg_rom_banks;
    uint8_t num_prg_ram_banks;
    uint8_t num_chr_banks;

    uint16_t prg_ram_bank_size;
    std::vector<uint8_t> PRG_RAM;

    virtual bool cpu_mapper_read(uint16_t addr, uint32_t& mapped_addr, uint8_t& data) = 0;
    virtual bool cpu_mapper_write(uint16_t addr, uint32_t& mapped_addr, uint8_t data) = 0;
    virtual bool ppu_mapper_read(uint16_t addr, uint32_t& mapped_addr) = 0;
    virtual bool ppu_mapper_write(uint16_t addr, uint32_t& mapped_addr, uint8_t data) = 0;

    virtual void reset() = 0;
    virtual bool mapped_to_prg_ram(uint16_t addr) = 0;
    virtual std::vector<uint8_t> get_prg_ram() = 0; // For debugging purposes only


};