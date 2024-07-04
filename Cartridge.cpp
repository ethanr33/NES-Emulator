
#pragma once

#include <fstream>
#include <iostream>
#include <stdexcept>

#include "Cartridge.h"
#include "Helpers.h"
#include "mappers/Mapper000.h"

Cartridge::Cartridge(const std::string& rom_file_name) {
    std::ifstream rom_file("roms/" + rom_file_name, std::ios::binary);

    if (!rom_file.is_open()) {
        std::cerr << "Failed to open file" << std::endl;
        return;
    }

    // We don't need to use size_t because string::npos is -1
    int extension_pos_start = rom_file_name.find('.');

    std::vector<uint8_t> file_header(16);

    // We need to read in one byte at a time. However the ifstream.get method is not overloaded to work with uint8_t
    // types. Chars are also one byte big so these will work fine
    char cur_byte;

    std::vector<uint8_t> rom_data;


    if (extension_pos_start == std::string::npos) {
        std::cerr << "The file name is formatted incorrectly. Be sure to add an extension" << std::endl;
        return;
    } else {
        std::string extension = rom_file_name.substr(extension_pos_start + 1);

        if (extension == "nes") {
            // iNES format, file header is 16 bytes

            char cur_byte;
            for (int i = 0; i < 16; i++) {
                rom_file.get(cur_byte);
                file_header.at(i) = cur_byte;
            }

            const int NUM_PRG_BANKS = file_header.at(4);
            const int NUM_CHR_BANKS = file_header.at(5);

            const int PRG_ROM_SIZE = file_header.at(4) * PRG_ROM_PAGE_SIZE;
            const int CHR_ROM_SIZE = file_header.at(5) * CHR_ROM_PAGE_SIZE;

            bool has_trainer = is_bit_set(2, file_header.at(6));

            if (has_trainer) {
                std::cerr << "Trainers are not supported yet" << std::endl;
            } else {
                for (int i = 0; i < PRG_ROM_SIZE; i++) {
                    rom_file.get(cur_byte);
                    PRG_ROM.push_back(static_cast<uint8_t>(cur_byte));
                }

                for (int i = 0; i < CHR_ROM_SIZE; i++) {
                    rom_file.get(cur_byte);
                    CHR_ROM.push_back(static_cast<uint8_t>(cur_byte));
                }
            }

            uint8_t mapper_number = (file_header.at(6) & 0xF0) | ((file_header.at(5) & 0xF0) >> 4);

            switch (mapper_number) {
                case 0:
                    this->mapper = new Mapper000(NUM_PRG_BANKS, NUM_CHR_BANKS);
                    break;
                default:
                    throw std::runtime_error("Mapper not supported: " + std::to_string(mapper_number));
                    break;
            }
        } else {
            std::cerr << "Unsupported ROM format" << std::endl;
            return;
        }
    }
}


    bool Cartridge::read_cpu(uint16_t address, uint8_t& data) {
        uint16_t mapped_address = address;

        if (mapper->cpu_mapper_read(address, mapped_address)) {
            data = PRG_ROM[mapped_address];
            return true;
        }

        return false;
    }

    bool Cartridge::write_cpu(uint16_t address, uint8_t data) {
        uint16_t mapped_address = address;

        if (mapper->cpu_mapper_write(address, mapped_address)) {
            PRG_ROM[mapped_address] = data;
            return true;
        }

        return false;
    }

    bool Cartridge::read_ppu(uint16_t address, uint8_t& data) {
        uint16_t mapped_address = address;

        if (mapper->ppu_mapper_read(address, mapped_address)) {
            data = CHR_ROM[mapped_address];
            return true;
        }

        return false;
    }

    bool Cartridge::write_ppu(uint16_t address, uint8_t data) {
        uint16_t mapped_address = address;

        if (mapper->ppu_mapper_write(address, mapped_address)) {
            CHR_ROM[mapped_address] = data;
            return true;
        }

        return false;
    }