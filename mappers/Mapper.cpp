
#include "Mapper.h"

Mapper::Mapper(uint8_t prg_rom_banks, uint8_t prg_ram_banks, uint8_t chr_banks, uint16_t ram_bank_size) {
    num_prg_rom_banks = prg_rom_banks;
    num_prg_ram_banks = prg_ram_banks;
    num_chr_banks = chr_banks;
    prg_ram_bank_size = ram_bank_size;
}