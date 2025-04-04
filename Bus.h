#pragma once

#include "CPU.h"
#include "PPU.h"
#include "Cartridge.h"
#include "IO.h"

#include <vector>

using std::vector;

struct PPU;

struct Bus {
    static const uint16_t RAM_MIRROR_START = 0x0000;
    static const uint16_t RAM_MIRROR_END = 0x1FFF;

    static const uint16_t PPU_REG_MIRROR_START = 0x2000;
    static const uint16_t PPU_REG_MIRROR_END = 0x3FFF;

    static const uint16_t APU_IO_REG_START = 0x4000;
    static const uint16_t APU_IO_REG_END = 0x4017;

    static const uint16_t RAM_SIZE = 0x2000;

    Bus();
    Bus(bool);

    CPU* cpu = nullptr;
    PPU* ppu = nullptr;
    Cartridge* cartridge;
    IO* io = nullptr;

    std::vector<uint8_t> cpu_RAM = vector<uint8_t>(RAM_SIZE);

    uint64_t num_ticks = 0;

    bool is_nmi_line_low = false;
    bool is_nmi_suppressed = false;

    uint8_t read_cpu(uint16_t);
    void write_cpu(uint16_t, uint8_t); 

    void insert_cartridge(Cartridge*);
    void reset();
    void tick();
    void halt();
    void set_nmi_line(bool);
    void set_nmi_suppression_status(bool);
    bool get_nmi_line_status() const;
    

};