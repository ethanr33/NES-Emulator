
#include <fstream>
#include <iostream>
#include <vector>
#include "NES.h"

NES::NES() {
    cpu = new CPU();
}

CPU* NES::get_cpu() const {
    return cpu;
}

bool NES::load_program(const string& rom_file_name) {
    std::ifstream rom_file("roms/" + rom_file_name, std::ios::binary);

    if (!rom_file.is_open()) {
        std::cerr << "Failed to open file" << std::endl;
        return false;
    }

    // For now, let's not worry about the standard INES file format. Until we get the CPU and display up and running
    // let's assume that the program data can be read starting at the beginning of the file

    // We need to read in one byte at a time. However the ifstream.get method is not overloaded to work with uint8_t
    // types. Chars are also one byte big so these will work fine
    char cur_byte;

    std::vector<uint8_t> rom_data;

    // Read input from file one byte at a time, since that is the size of a opcode
    while (rom_file.get(cur_byte)) {
        // Conversion of char to uint8_t because that's the type of value we need to store in memory
        rom_data.push_back(static_cast<uint8_t>(cur_byte));
    }

    // Once we read all of the data from the rom, we can store it in memory
    cpu->load_rom_into_memory(rom_data);

    return true;
}