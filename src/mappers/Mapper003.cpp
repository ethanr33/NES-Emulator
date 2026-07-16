
#include "mappers/Mapper003.h"

bool Mapper003::cpu_mapper_read(uint16_t addr, uint32_t& mapped_addr, uint8_t& data) {
    if (addr >= 0x8000 && addr <= 0xBFFF) {
        mapped_addr = mapped_addr - 0x8000;
        return true;
    } else if (addr >= 0xC000) {
        if (num_prg_rom_banks == 1) {
            mapped_addr = mapped_addr - 0xC000;
        } else {
            mapped_addr = mapped_addr - 0x8000;
        }
        return true;
    }

    return false;

}

bool Mapper003::cpu_mapper_write(uint16_t addr, uint32_t& mapped_addr, uint8_t data) {
    if (addr >= 0x8000 && addr <= 0xFFFF) {
        this->cur_chr_bank = data & 0x3;
        return true;
    }

    return false;
}

bool Mapper003::ppu_mapper_read(uint16_t addr, uint32_t& mapped_addr) {
    if (addr <= 0x1FFF) {
        mapped_addr = addr + this->cur_chr_bank * 0x2000;
        return true;
    }
    return false;
}

bool Mapper003::ppu_mapper_write(uint16_t addr, uint32_t& mapped_addr, uint8_t data) {
    if (addr <= 0x1FFF) {
        mapped_addr = addr + this->cur_chr_bank * 0x2000;
        return true;
    }
    return false;
}

void Mapper003::reset() {

}

bool Mapper003::mapped_to_prg_ram(uint16_t addr) {
    // No PRG RAM
    return false;
}

std::vector<uint8_t> Mapper003::get_prg_ram() {
    return std::vector<uint8_t>();
}