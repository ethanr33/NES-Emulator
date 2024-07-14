
#include "PPU.h"

uint8_t PPU::read_cpu(uint16_t address) {
    return 0x0;
}

void PPU::write_cpu(uint16_t address, uint8_t val) {
    return;
} 

// Communicate with PPU bus

uint8_t PPU::read_ppu(uint16_t address) {
    uint8_t data;

    if (cartridge->read_ppu(address, data)) {
        // do something
    }

    return 0x0;
}

void PPU::write_ppu(uint16_t address, uint8_t val) {

    if (cartridge->write_ppu(address, val)) {
        // do something
    }

    return;
}

void PPU::load_cartridge(Cartridge* new_cartridge) {
    cartridge = new_cartridge;
} 

void PPU::tick() {
    return;
}

void PPU::reset() {
    scanline = -1;
}