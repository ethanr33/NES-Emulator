
#include "Bus.h"

Bus::Bus() {
    cpu.attach_bus(this);
}

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
    cartridge = new_cartridge;
    ppu.load_cartridge(new_cartridge);
}

void Bus::reset() {
    cpu.reset();
    ppu.reset();
}

void Bus::tick() {

    if (num_ticks % 3 == 0) {
        cpu.tick();
    }

    ppu.tick();
}