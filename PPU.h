
#pragma once
#include "Cartridge.h"

struct PPU {
    static const int VRAM_SIZE = (1 << 11);
    static const int PALETTE_TABLE_SIZE = 32;

    uint8_t VRAM[VRAM_SIZE];
    uint8_t PALETTE_RAM[PALETTE_TABLE_SIZE];

    Cartridge* cartridge;

    uint16_t scanline = -1;

    // Communicate with CPU bus
    uint8_t read_cpu(uint16_t);
    void write_cpu(uint16_t, uint8_t); 

    // Communicate with PPU bus
    uint8_t read_ppu(uint16_t);
    void write_ppu(uint16_t, uint8_t);

    // Load "Cartridge" for PPU use
    void load_cartridge(Cartridge*);

    void tick();
    void reset();
};