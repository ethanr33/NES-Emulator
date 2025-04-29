
# About

This project is a currently work in progress emulator for the NES (Nintendo Entertainment System). Please note that the emulator is still very much in development, so there may be some major features of the NES that are still unsupported.

# What has been done:

- **CPU emulation** - 
  The emulator can emulate the Ricoh 6502 with good accuracy.
- **PPU emulation** -
  Basic PPU operation and pipeline have been implemented. However, advanced operations such as sprite rendering and scrolling have not.
- **Controller Input** -
  The emulator is able to emulate the function of a single standard NES controller (so far). Functionality with more than one controller has not been tested yet.

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

## [Sprite overflow tests](https://github.com/christopherpow/nes-test-roms/tree/master/sprite_overflow_tests)

- basics.nes

# Future work:

- Full, cycle-accurate implementation of the PPU
- Verify cycle-accurate implementation of the CPU
- Verify functionality of second standard NES controller
- Add support for non-traditional controllers
- Add support for all common mappers
- Add support for the iNES2.0 file format
- Implement APU
