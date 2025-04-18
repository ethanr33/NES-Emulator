
#pragma once

#include <stdexcept>
#include "Cartridge.h"
#include "UI.h"
#include "Helpers.h"
#include "Bus.h"

enum TILE_POSITION {TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT};

struct Sprite {
    uint8_t y_position; // Y position of top of sprite
    uint8_t tile_index_number;
    uint8_t attributes;
    uint8_t x_position; // X position of left of sprite

    Sprite() {
        y_position = 0;
        tile_index_number = 0;
        attributes = 0;
        x_position = 0;
    }

    Sprite(uint8_t y_pos, uint8_t tile_index, uint8_t attr, uint8_t x_pos) {
        y_position = y_pos;
        tile_index_number = tile_index;
        attributes = attr;
        x_position = x_pos;
    }
};

// See https://www.nesdev.org/wiki/PPU_registers#PPUCTRL_-_Miscellaneous_settings_($2000_write)
struct PPUCTRL {
    bool nmi_enable;
    bool ppu_ms;
    bool sprite_height; // If false: 8 pixels high. If true: 16 pixels high.
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
    static const int PRIMARY_OAM_SIZE = 0x100;
    static const int SECONDARY_OAM_SIZE = 0x20;
    static const int MAX_SPRITES = 64;
    static const int VISIBLE_SCANLINES_PER_CYCLE = 240;
    static const int SPRITE_WIDTH = 8;

    uint8_t VRAM[VRAM_SIZE];
    uint8_t PALETTE_RAM[PALETTE_TABLE_SIZE];

    vector<uint8_t> primary_OAM = vector<uint8_t>(PRIMARY_OAM_SIZE);

    // Secondary OAM is a buffer for sprites being rendered on this scanline
    vector<uint8_t> secondary_OAM = vector<uint8_t>(SECONDARY_OAM_SIZE);

    Cartridge* cartridge;
    UI* ui = nullptr;
    Bus* bus = nullptr;

    uint16_t scanline = 0;
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
    
    // If PPUSTATUS is read the same time VBlank is set, PPUSTATUS will not show that VBlank has started
    // The VBlank flag in PPUSTATUS will not be set on the next frame either.
    // This variable is true if PPUSTATUS was read the dot when VBlank was going to be set.
    bool ppustatus_vblank_read_race_condition = false;

    // True if an NMI has been triggered since the VBlank NMI flag was set to true
    // False if VBlank NMI flag is false, or if NMI hasn't been triggered yet
    bool has_nmi_triggered = false;

    uint8_t ppudata_read_buffer;

    uint8_t nametable_entry = 0;
    uint8_t attribute_table_entry = 0;
    uint8_t pattern_low_byte = 0;
    uint8_t pattern_high_byte = 0;

    uint16_t frames_elapsed = 0;


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
    void load_OAMDMA(uint8_t);
    
    // Go through OAM_sprite_list and put only the sprites that will be rendered in OAM_renderable_sprites
    void filter_renderable_sprites();

    // Get current sprite height from PPUCTRL
    uint8_t get_sprite_height();

    // Run sprite evaluation for this scanline and dot
    bool run_sprite_evaluation();

    enum SPRITE_EVALUATION_STAGE {STAGE_1, STAGE_2_1, STAGE_2_1a, STAGE_2_2, STAGE_2_3, STAGE_2_3a, STAGE_2_3b, STAGE_2_4, STAGE_3, STAGE_4, IDLE};

    // The initial state of the PPU is scanline = 0 and cur_dot = 0
    // So we need to make sure we start on STAGE_1 and not IDLE
    uint8_t cur_sprite_evaluation_stage = STAGE_1;

    // This refers to the byte at offset 4*n + m within OAM.
    uint8_t n = 0;
    uint8_t m = 0;
    uint8_t num_sprites_found = 0;
    uint8_t start_m;

    bool secondary_oam_write_enabled = true;

    // In stage 2, reads from primary OAM only occur on odd cycles
    // Use this variable to store the value of that read so it can be used on the next cycle
    uint8_t primary_oam_buffer = 0;

    void attach_bus(Bus*);

    enum PPU_RENDERING_STAGE {PRE_RENDER, VISIBLE, POST_RENDER, VBLANK};
    uint8_t cur_ppu_rendering_stage = PRE_RENDER;

    void tick();


    void reset();

    PPU();
    PPU(bool);
};