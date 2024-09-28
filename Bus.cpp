
#include "Bus.h"

Bus::Bus() {
    cpu = new CPU();
    ppu = new PPU();
    cpu->attach_bus(this);
}

Bus::Bus(bool ui_disabled) {
    cpu = new CPU();
    ppu = new PPU(ui_disabled);
    cpu->attach_bus(this);
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
        return ppu->read_from_cpu(address);
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
        ppu->write_from_cpu(address, val);
    }

    // OAM DMA register
    if (address == 0x4014) {
        ppu->load_OAMDMA(val, cpu_RAM);
    }
}

void Bus::insert_cartridge(Cartridge* new_cartridge) {
    cartridge = new_cartridge;
    ppu->load_cartridge(new_cartridge);
}

void Bus::reset() {
    cpu->reset();
    ppu->reset();
}

void Bus::tick() {

    if (num_ticks % 3 == 0) {
        cpu->tick();
    }

    ppu->tick();
}

void Bus::halt() {
    throw std::runtime_error("Execution halted at " + std::to_string(cpu->program_counter));
}