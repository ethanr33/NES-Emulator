
#pragma once

#include <stdexcept>
#include "Cartridge.h"
#include "UI.h"
#include "Helpers.h"

enum TILE_POSITION {TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT};

struct PPUCTRL {
    bool nmi_enable;
    bool ppu_ms;
    bool sprite_height;
    bool background_tile_select;
    bool sprite_tile_select;
    bool increment_mode;
    uint8_t nametable_select;

    PPUCTRL(uint8_t val) {
        nmi_enable = is_bit_set(7, val);
        ppu_ms = is_bit_set(6, val);
        sprite_height = is_bit_set(5, val);
        background_tile_select = is_bit_set(4, val);
        sprite_tile_select = is_bit_set(3, val);
        increment_mode = is_bit_set(2, val);
        nametable_select = val & 0x3;
    }

    // Return value of register in the form of a uint8_t, where each bit can be the value of a flag
    // Format specified here: https://www.nesdev.org/wiki/PPU_registers#PPUCTRL
    uint8_t serialize() {
        uint8_t res = nmi_enable << 7;
        res |= ppu_ms << 6;
        res |= sprite_height << 5;
        res |= background_tile_select << 4;
        res |= sprite_tile_select << 3;
        res |= increment_mode << 2;
        res |= nametable_select;

        return res;
    }
};

struct PPUMASK {
    bool emphasize_blue;
    bool emphasize_green;
    bool emphasize_red;
    bool sprite_enable;
    bool background_enable;
    bool sprite_left_column_enable;
    bool background_left_column_enable;
    bool greyscale;

    PPUMASK(uint8_t val) {
        emphasize_blue = is_bit_set(7, val);
        emphasize_green = is_bit_set(6, val);
        emphasize_red = is_bit_set(5, val);
        sprite_enable = is_bit_set(4, val);
        background_enable = is_bit_set(3, val);
        sprite_left_column_enable = is_bit_set(2, val);
        background_left_column_enable = is_bit_set(1, val);
        greyscale = is_bit_set(0, val);
    }

    // Return value of register in the form of a uint8_t, where each bit can be the value of a flag
    // Format specified here: https://www.nesdev.org/wiki/PPU_registers#PPUMASK
    uint8_t serialize() {
        uint8_t res = emphasize_blue << 7;
        res |= emphasize_green << 6;
        res |= emphasize_red << 5;
        res |= sprite_enable << 4;
        res |= background_enable << 3;
        res |= sprite_left_column_enable << 2;
        res |= background_left_column_enable << 1;
        res |= greyscale;

        return res;
    }
};

struct PPUSTATUS {
    bool vblank;
    bool sprite_hit;
    bool sprite_overflow;

    PPUSTATUS(uint8_t val) {
        vblank = is_bit_set(7, val);
        sprite_hit = is_bit_set(6, val);
        sprite_overflow = is_bit_set(5, val);
    }

    uint8_t serialize() {
        uint8_t res = vblank << 7;
        res = res | (sprite_hit << 6);
        res = res | (sprite_overflow << 5);

        return res;
    }
};

struct PPU {
    static const int VRAM_SIZE = 0x4000;
    static const int PALETTE_TABLE_SIZE = 32;
    static const int OAM_SIZE = 256;

    uint8_t VRAM[VRAM_SIZE];
    uint8_t PALETTE_RAM[PALETTE_TABLE_SIZE];
    uint8_t OAM[OAM_SIZE];

    Cartridge* cartridge;
    UI* ui = nullptr;

    uint16_t scanline = 241;
    uint16_t cur_dot = 0;

    // Internal registers
    uint16_t v;
    uint16_t t;
    uint8_t x;
    bool w;

    PPUCTRL ppuctrl = PPUCTRL(0);
    PPUMASK ppumask = PPUMASK(0);
    PPUSTATUS ppustatus = PPUSTATUS(0);
    uint8_t oamaddr = 0x0;
    uint8_t oamdata = 0x0;
    uint8_t ppuscroll = 0x0;
    uint16_t ppuaddr = 0x0;

    uint8_t nametable_entry = 0;
    uint8_t attribute_table_entry = 0;
    uint8_t pattern_low_byte = 0;
    uint8_t pattern_high_byte = 0;


    uint16_t pattern_shift_reg_low = 0x0000;
    uint16_t pattern_shift_reg_high = 0x0000;

    uint16_t map_to_nametable(uint16_t);

    // Communicate with CPU bus
    uint8_t read_from_cpu(uint16_t);
    void write_from_cpu(uint16_t, uint8_t); 

    // Communicate with PPU bus
    uint8_t read_from_ppu(uint16_t);
    void write_from_ppu(uint16_t, uint8_t);

    // Load "Cartridge" for PPU use
    void load_cartridge(Cartridge*);

    // For use with OAMDMA register
    void load_OAMDMA(uint8_t, uint8_t[]);

    void render_cycle();
    void tick();
    void reset();

    PPU();
    PPU(bool);
};