
## [Technical Write Up](https://docs.google.com/document/d/1fzqqa9KparlBGUkKNZd4IBr7n9K_vbH10DSQABEPsVY/edit?usp=sharing)



# About

This project is a work in progress emulator for the NES (Nintendo Entertainment System). All the main features of the NES's CPU and PPU are implemented, and a lot of popular games are playable, such as Donkey Kong, Super Mario Bros., and The Legend of Zelda.

# What has been done:

- **CPU emulation** - 
  The emulator can emulate the Ricoh 6502 with near perfect accuracy.
- **PPU emulation** -
  All of the main PPU operations such as sprite/background rendering, collisions, and sprite 0 hit have been implemented.
- **Controller Input** -
  The emulator is able to emulate the function of a single standard NES controller. 

# Requirements:

- C++17 or greater
- CMake 3.31 or greater
- SFML 2.6.x

# How to run the emulator:

1. Clone the repository
2. Run ```cmake .```, then ```make```.
3. Run ```./main \<rom file name\>```. The ROM file name should be the name of a file located in the ```roms``` folder.

# Supported ROMS:

All ROMs which are stored in the iNES file format and which use Mappers 0 and 1 will run on the emulator. If a ROM is run which does not satisfy these requirements the program will let you know.

# Input:

The following NES controller inputs are mapped to these keys on a standard keyboard:

- Button A: A
- Button B: B
- Select: O
- Start: P
- Up: Up arrow
- Down: Down arrow key
- Left: Left arrow key
- Right: Right arrow key

# Test ROMs passed:

## CPU tests:

- nestest.nes (Standard opcodes only)
- official_only.nes (Standard opcodes only)
- cpu_timing_test.nes

## [Instruction Misc.](https://github.com/christopherpow/nes-test-roms/tree/master/instr_misc)

- abs_x_wrap.nes
- branch_wrap.nes

## [Branch Timing Tests](https://github.com/christopherpow/nes-test-roms/tree/master/branch_timing_tests)

## [Blargg's PPU Tests](https://github.com/christopherpow/nes-test-roms/tree/master/blargg_ppu_tests_2005.09.15b)

- palette_ram.nes
- sprite_ram.nes
- vram_access.nes

## [PPU VBL/NMI tests](https://github.com/christopherpow/nes-test-roms/tree/master/ppu_vbl_nmi):

- vbl_basics.nes
- vbl_set_time.nes
- vbl_clear_time.nes
- nmi_control.nes

## [PPU VBL/NMI timing](https://github.com/christopherpow/nes-test-roms/tree/master/vbl_nmi_timing)

- frame_basics.nes
- vbl_timing.nes
- even_odd_frames.nes
- vbl_clear_timing.nes

## [Sprite hit tests](https://github.com/christopherpow/nes-test-roms/tree/master/sprite_hit_tests_2005.10.05)

- basics.nes
- alignment.nes
- corners.nes
- flip.nes
- left_clip.nes
- right_edge.nes

## [Sprite overflow tests](https://github.com/christopherpow/nes-test-roms/tree/master/sprite_overflow_tests)

- basics.nes
- details.nes

# Future work:

- Cycle-accurate implementation of the PPU
- Add functionality of second standard NES controller
- Add support for non-traditional controllers
- Add support for all common mappers (Mapper, 2, 3, etc.)
- Add support for the iNES2.0 file format
- Implement APU
