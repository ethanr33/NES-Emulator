
#include "Bus.h"

uint8_t Bus::read_cpu(uint16_t address) {
    uint8_t data;

    if (cartridge->read_cpu(address, data)) {
        return data;
    }

    if (address >= RAM_MIRROR_START && address <= RAM_MIRROR_END) {
        return cpu_RAM[address & 0x7FF];
    }

    if (address >= PPU_REG_MIRROR_START && address <= PPU_REG_MIRROR_END) {
        return cpu_RAM[address & 0x7];
    }

    return cpu_RAM[address];
}

void Bus::write_cpu(uint16_t address, uint8_t val) {
    if (cartridge->write_cpu(address, val)) {
        return;
    }

    if (address >= RAM_MIRROR_START && address <= RAM_MIRROR_END) {
        cpu_RAM[address & 0x7FF] = val;
    }

    if (address >= PPU_REG_MIRROR_START && address <= PPU_REG_MIRROR_END) {
        cpu_RAM[PPU_REG_MIRROR_START + address & 0x7] = val;
    }
}

void Bus::insert_cartridge(Cartridge* new_cartridge) {
    this->cartridge = new_cartridge;
    this->ppu.load_cartridge(new_cartridge);
}

void Bus::reset() {
    cpu.reset();
}