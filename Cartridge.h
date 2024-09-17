
#pragma once
#include <vector>
#include <string>

#include "mappers/Mapper.h"

enum MIRRORING_TYPE {HORIZONTAL, VERTICAL, FOUR_SCREEN};

struct Cartridge {

    static const uint16_t PRG_ROM_PAGE_SIZE = (1 << 14);
    static const uint16_t CHR_ROM_PAGE_SIZE = (1 << 13);
    
    std::vector<int> PRG_ROM;
    std::vector<int> CHR_ROM;
    std::vector<int> trainer;

    MIRRORING_TYPE mirroring_type;

    Mapper* mapper;

    Cartridge(const std::string&);

    bool read_cpu(uint16_t, uint8_t&);
    bool write_cpu(uint16_t, uint8_t);

    bool read_ppu(uint16_t, uint8_t&);
    bool write_ppu(uint16_t, uint8_t);

    void dump_CHR();
};