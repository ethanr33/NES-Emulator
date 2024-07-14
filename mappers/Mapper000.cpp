
#include "Mapper000.h"

bool Mapper000::cpu_mapper_read(uint16_t addr, uint16_t& mapped_addr) {

    if (addr >= 0x6000 && addr <= 0x7FFF) {
        // Family switch, do something
    }

    if (addr >= 0x8000 && addr <= 0xBFFF) {
        // First 16KB of rom, directly mapped
        mapped_addr = addr - 0x8000;
        return true;
    }

    if (addr >= 0xC000 && addr <= 0xFFFF) {
        // If NROM-128, this will be a mirror of the first 16KB.
        // If NROM-256, this will be the second 16KB section.

        if (num_prg_banks == 1) {
            mapped_addr = addr - 0xC000;
        } else {
            mapped_addr = addr - 0x8000;
        }
        return true;
    }

    return false;
}

bool Mapper000::cpu_mapper_write(uint16_t addr, uint16_t& mapped_addr) {

    if (addr >= 0x6000 && addr <= 0x7FFF) {
        // Family switch, do something
    }

    if (addr >= 0x8000 && addr <= 0xBFFF) {
        // First 16KB of rom, directly mapped
        mapped_addr = addr - 0x8000;
        return true;
    }

    if (addr >= 0xC000 && addr <= 0xFFFF) {
        // If NROM-128, this will be a mirror of the first 16KB.
        // If NROM-256, this will be the second 16KB section.

        if (num_prg_banks == 1) {
            mapped_addr = addr - 0xC000;
        } else {
            mapped_addr = addr - 0x8000;
        }
        return true;
    }

    return false;
}

// The mapper does not affect PPU addresses

bool Mapper000::ppu_mapper_read(uint16_t addr, uint16_t& mapped_addr) {
    if (addr >= 0x0000 && addr <= 0x1FFF) {
        mapped_addr = addr;
        return true;
    }
    return false;
}

bool Mapper000::ppu_mapper_write(uint16_t addr, uint16_t& mapped_addr) {
    return false;    
}