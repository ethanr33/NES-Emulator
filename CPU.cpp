
#include <stdexcept>

#include "CPU.h"
#include "Helpers.cpp"



/*
LDA - Load Accumulator
Loads a byte of memory into the accumulator setting the zero and negative flags as appropriate.
*/

void CPU::LDA(uint8_t new_a_val) {
    A = new_a_val;

    if (A == 0) {
        set_flag(ZERO, 1);
    }

    if (is_bit_set(7, A)) {
        set_flag(NEGATIVE, 1);
    }
}

/*
TAX - Transfer Accumulator to X
Copies the current contents of the accumulator into the X register and sets the zero and negative flags as appropriate.
*/
void CPU::TAX() {
    X = A;

    if (X == 0) {
        set_flag(ZERO, 1);
    }

    if (is_bit_set(7, X)) {
        set_flag(NEGATIVE, 1);
    }
}

/*
TAY - Transfer Accumulator to Y
Copies the current contents of the accumulator into the Y register and sets the zero and negative flags as appropriate.
*/
void CPU::TAY() {
    Y = A;

    if (Y == 0) {
        set_flag(ZERO, 1);
    }

    if (is_bit_set(7, Y)) {
        set_flag(NEGATIVE, 1);
    }
}


void CPU::set_flag(flag_type flag_to_set, bool new_flag_val) {
    flags[flag_to_set] = new_flag_val;
}

void CPU::toggle_flag(flag_type flag_to_toggle) {
    flags[flag_to_toggle] = !flags[flag_to_toggle];
}

uint8_t CPU::get_a() const {
    return A;
}

void CPU::set_a(uint8_t new_a) {
    A = new_a;
}

uint8_t CPU::get_x() const {
    return X;
}

bool CPU::get_flag(flag_type flag_to_get) {
    return flags[flag_to_get];
}

uint8_t CPU::get_memory(addressing_mode mode, uint8_t parameter_val) const {
    if (mode == ZERO_PAGE) {
        return RAM[parameter_val];
    } else if (mode == ZERO_PAGE_X) {
        return RAM[parameter_val + X];
    } else if (mode == ZERO_PAGE_Y) {
        return RAM[parameter_val + Y];
    } else if (mode == INDEXED_INDIRECT) {
        uint8_t least_significant_byte = RAM[parameter_val + X];
        uint8_t most_significant_byte = RAM[parameter_val + X + 1];

        return RAM[form_address(least_significant_byte, most_significant_byte)];
    } else if (mode == INDIRECT_INDEXED) {
        uint8_t least_significant_byte = RAM[parameter_val];
        uint8_t most_significant_byte = RAM[parameter_val + 1];

        return RAM[form_address(least_significant_byte, most_significant_byte) + Y];
    } else {
        throw std::runtime_error("Memory addressing mode not implemented: " + mode);
    }
}

uint8_t CPU::get_memory(addressing_mode mode, uint8_t parameter_lsb, uint8_t parameter_msb) const {
    uint16_t full_address = form_address(parameter_lsb, parameter_msb);

    if (mode == ABSOLUTE) {
        return RAM[full_address];
    } else if (mode == ABSOLUTE_X) {
        return RAM[full_address + X];
    } else if (mode == ABSOLUTE_Y) {
        return RAM[full_address + Y];
    } else {
        throw std::runtime_error("Memory addressing mode not implemented: " + mode);
    }
}