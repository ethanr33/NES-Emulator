#pragma once

#include "CPU.h"
#include "PPU.h"
#include "Cartridge.h"

struct Bus {
    static const uint16_t RAM_MIRROR_START = 0x0000;
    static const uint16_t RAM_MIRROR_END = 0x1FFF;

    static const uint16_t PPU_REG_MIRROR_START = 0x2000;
    static const uint16_t PPU_REG_MIRROR_END = 0x3FFF;

    static const uint16_t RAM_SIZE = 0x2000;

    Bus();
    Bus(bool);

    CPU* cpu = nullptr;
    PPU* ppu = nullptr;
    Cartridge* cartridge;

    uint8_t cpu_RAM[RAM_SIZE];

    uint64_t num_ticks = 0;

    uint8_t read_cpu(uint16_t);
    void write_cpu(uint16_t, uint8_t); 

    void insert_cartridge(Cartridge*);
    void reset();
    void tick();
    void halt();

};