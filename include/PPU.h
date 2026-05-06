
#pragma once

#include <stdexcept>
#include "Cartridge.h"
#include "UI.h"
#include "Helpers.h"
#include "Bus.h"

enum TILE_POSITION {TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT};

struct Sprite {
    uint8_t y_position; // Y position of top of sprite

    // Tile number of the sprite within the pattern table selected by PPUCTRL
    // Ignored for 8x16 sprites
    uint8_t tile_index_number; 

    // Byte which stores various attributes which can be applied to the sprite (palette index, priority, flipping sprite horizontally/vertically)
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
    bool nmi_enable; // True if the PPU should generate an NMI when VBlank starts
    bool ppu_ms;
    bool sprite_height; // If false: 8 pixels high. If true: 16 pixels high.

    // Chooses the address of the pattern table which background tiles use
    bool background_tile_select;
    
    // Chooses the address of the pattern table which sprites use
    bool sprite_tile_select;

    // Chooses whether to increment the v register by 1 (move horizontally) or 32 (move vertically)
    bool increment_mode;

    // Chooses what nametable is being rendered at the top left corner of the screen
    // Value ranges from 0 (nametable at address 0x2000) to 3 (nametable at address 0x2C00)
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

    // Outputs the status of each variable in the PPUCTRL register
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

    // If false: sprites are not rendered. True: sprites are rendered
    bool sprite_enable;

    // If false: background is not rendered. True: background is rendered
    bool background_enable;

    // If false: sprites are not rendered in the left 8 pixels of the screen. If true: sprites are rendered in the left 8 pixels
    bool sprite_left_column_enable;

    // If false: background is not rendered in the left 8 pixels of the screen. If true: background is rendered in the left 8 pixels
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

    // Outputs the status of each variable in the PPUMASK register
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
    // if true: PPU is in VBlank currently
    bool vblank;


    bool sprite_hit;

    // if true: more than 8 sprites could have been rendered on a scanline which has been processed this frame
    bool sprite_overflow;

    PPUSTATUS(uint8_t val) {
        vblank = is_bit_set(7, val);
        sprite_hit = is_bit_set(6, val);
        sprite_overflow = is_bit_set(5, val);
    }

    // Outputs the status of each variable in the PPUSTATUS register
    uint8_t serialize() {
        uint8_t res = vblank << 7;
        res = res | (sprite_hit << 6);
        res = res | (sprite_overflow << 5);

        return res;
    }
};

struct PPU {
    static const int VRAM_SIZE = 0x3000;
    static const int PALETTE_TABLE_SIZE = 32;
    static const int PRIMARY_OAM_SIZE = 0x100;
    static const int SECONDARY_OAM_SIZE = 0x20;
    static const int MAX_SPRITES = 64;
    static const int VISIBLE_SCANLINES_PER_CYCLE = 240;
    static const int SPRITE_WIDTH = 8;

    // Stores CHR data and nametable data
    vector<uint8_t> VRAM = vector<uint8_t>(VRAM_SIZE);

    // Stores indices into the palette table 
    vector<uint8_t> PALETTE_RAM = vector<uint8_t>(PALETTE_TABLE_SIZE);

    // Primary OAM stores 64 sprites, and is searched through to determine what sprites will be rendered on this scanline
    vector<uint8_t> primary_OAM = vector<uint8_t>(PRIMARY_OAM_SIZE);

    // Secondary OAM is a buffer for sprites being rendered on this scanline
    vector<uint8_t> secondary_OAM = vector<uint8_t>(SECONDARY_OAM_SIZE);

    // Array which stores the indices of selected sprites in primary OAM
    vector<uint8_t> OAM_indices = vector<uint8_t>(SECONDARY_OAM_SIZE / 4, 0xFF);

    // Secondary OAM is cleared before those sprites are rendered on the screen.
    // This is the place to store sprites after secondary OAM is filled
    vector<uint8_t> OAM_buffer = vector<uint8_t>(SECONDARY_OAM_SIZE, 0xFF);

    Cartridge* cartridge;
    UI* ui = nullptr;
    Bus* bus = nullptr;

    uint16_t scanline = 0;
    uint16_t cur_dot = 0;

    // Internal registers
    uint16_t v = 0;
    uint16_t t = 0;
    uint8_t fine_x_offset = 0;

    // Register which keeps track of which write PPUSCROLL and PPUADDR are on
    // 0: first write, 1: second write
    bool w = 0;

    PPUCTRL ppuctrl = PPUCTRL(0);
    PPUMASK ppumask = PPUMASK(0);
    PPUSTATUS ppustatus = PPUSTATUS(0);
    uint8_t oamaddr = 0x0;
    uint8_t oamdata = 0x0;
    uint16_t ppuaddr = 0x0;
    
    // If PPUSTATUS is read the same time VBlank is set, PPUSTATUS will not show that VBlank has started
    // The VBlank flag in PPUSTATUS will not be set on the next frame either.
    // This variable is true if PPUSTATUS was read the dot when VBlank was going to be set.
    bool ppustatus_vblank_read_race_condition = false;

    // True if an NMI has been triggered since the VBlank NMI flag was set to true
    // False if VBlank NMI flag is false, or if NMI hasn't been triggered yet
    bool has_nmi_triggered = false;

    uint8_t ppudata_read_buffer;

    // How many frames has the PPU rendered so far?
    uint16_t frames_elapsed = 0;

    // Given an address which indexes a nametable, map and return the address to another nametable address which is not a mirror
    uint16_t map_to_nametable(uint16_t);

    // Perform a read/write from CPU address space
    uint8_t read_from_cpu(uint16_t);
    void write_from_cpu(uint16_t, uint8_t); 

    // Perform a read/write from PPU address space
    // Should only be used by PPU, since only the PPU can access its full address space
    uint8_t read_from_ppu(uint16_t);
    void write_from_ppu(uint16_t, uint8_t);

    // Load "Cartridge" for PPU use
    void load_cartridge(Cartridge*);

    // Load a page from CPU memory into the primary OAM starting at the address specified by oamaddr
    void load_OAMDMA(uint8_t);

    // Get current sprite height from PPUCTRL
    uint8_t get_sprite_height() const;

    // Check if rendering is enabled
    bool is_rendering_enabled() const;

    // Run sprite evaluation for this scanline and dot
    void run_sprite_evaluation();

    // Functions for incrementing scroll registers

    // Increment the coarse X value in v by one. Accounts for nametable wrapping
    void increment_coarse_x(uint16_t&) const;

    // Increment the fine Y value in v by one. Wraps around to next coarse y / nametable when necessary
    void increment_y(uint16_t&) const;

    // Functions for copying data from register t to register v

    // Copy coarse x and horizontal nametable data from reg. t to reg. v
    void copy_x_pos_data();

    // Copy fine y, coarse y and vertical nametable data from reg. t to reg. v
    void copy_y_pos_data();

    enum SPRITE_EVALUATION_STAGE {STAGE_1, STAGE_2, STAGE_3, STAGE_4, IDLE};

    // Stores the current sprite evaluation stage the PPU is on
    uint8_t cur_sprite_evaluation_stage;

    // This refers to the byte at offset 4*n + m within OAM.
    uint8_t num_sprites_found = 0;

    void attach_bus(Bus*);

    enum PPU_RENDERING_STAGE {PRE_RENDER, VISIBLE, POST_RENDER, VBLANK};

    // Stores the current rendering stage the PPU is on
    uint8_t cur_ppu_rendering_stage;

    // Runs one cycle of the PPU
    void tick();

    // Resets PPU to startup state
    void reset();

    PPU();
    PPU(bool);
};