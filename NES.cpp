
#include <fstream>
#include <iostream>
#include <vector>
#include "NES.h"
#include "Helpers.h"

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

    // We need to read in one byte at a time. However the ifstream.get method is not overloaded to work with uint8_t
    // types. Chars are also one byte big so these will work fine
    char cur_byte;

    std::vector<uint8_t> rom_data;


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
                std::cout << (int) cur_byte << std::endl;
                file_header.at(i) = cur_byte;
            }

            const int PRG_ROM_SIZE = file_header.at(4) * (1 << 14);
            const int CHR_ROM_SIZE = file_header.at(5) * (1 << 14);

            bool has_trainer = is_bit_set(2, file_header.at(6));

            if (has_trainer) {
                std::cerr << "Trainers are not supported yet" << std::endl;
            } else {
                // Read data from ROM

                for (int i = 0; i < PRG_ROM_SIZE; i++) {
                    rom_file.get(cur_byte);
                    rom_data.push_back(static_cast<uint8_t>(cur_byte));
                }
            }
        } else {
            std::cerr << "Unsupported ROM format" << std::endl;
            return false;
        }
    }

    std::cout << "rom data size: " << rom_data.size() << std::endl;

    // Once we read all of the data from the rom, we can store it in memory
    cpu->load_rom_into_memory(rom_data);

    return true;
}