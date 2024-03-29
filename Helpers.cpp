
#pragma once
#include <cstdint>

bool is_bit_set(int bit_index, uint8_t value) {
    return (value >> bit_index) & 1;
}

uint16_t form_address(uint8_t least_significant_byte, uint8_t most_significant_byte) {
    return (most_significant_byte << 4) + least_significant_byte;
}

// We'll consider 0 to be a positive number, because the last bit isn't set
bool is_positive(uint8_t value) {
    if (value == 0) {
        return true;
    }

    return value & 0x80;
}