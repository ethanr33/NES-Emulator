
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

    // First, we need to get the ROM file format type. This is encoded in the file's extension.

    /*
        Supported formats:
        .nes: iNES or NES2.0 format
    */

    // We don't need to use size_t because string::npos is -1
    int extension_pos_start = rom_file_name.find('.');

    std::vector<uint8_t> file_header(16);

    if (extension_pos_start == std::string::npos) {
        std::cerr << "The file name is formatted incorrectly. Be sure to add an extension" << std::endl;
        return false;
    } else {
        std::string extension = rom_file_name.substr(extension_pos_start + 1);

        if (extension == "nes") {
            // Either iNES or NES2.0 format, header is 16 bytes

            char cur_byte;
            for (int i = 0; i < 16; i++) {
                rom_file.get(cur_byte);
                file_header.at(i) = cur_byte;
            }
        } else {
            std::cerr << "Unsupported ROM format" << std::endl;
            return false;
        }
    }

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