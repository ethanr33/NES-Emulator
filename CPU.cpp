
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
LDX - Load X
Loads a byte of memory into the X register setting the zero and negative flags as appropriate.
*/

void CPU::LDX(uint8_t new_x_val) {
    X = new_x_val;

    if (X == 0) {
        set_flag(ZERO, 1);
    }

    if (is_bit_set(7, X)) {
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

uint8_t CPU::get_y() const {
    return Y;
}

bool CPU::get_flag(flag_type flag_to_get) {
    return flags[flag_to_get];
}

uint16_t CPU::get_program_counter() const {
    return program_counter;
}

void CPU::increment_program_counter(int increment_amount) {
    program_counter += increment_amount;
}

/*

addressing_mode mode: The way we want to address memory
uint8_t parameter_val: The address of the parameter

return value: The content in memory we want to use

*/
uint8_t CPU::get_memory(addressing_mode mode, uint8_t parameter_val) {
    // The opcode will be 2 bytes long if we get here, so we should increment the program counter by 2
    // to reflect this

    increment_program_counter(2);

    if (mode == IMMEDIATE) {
        return parameter_val;
    } else if (mode == ZERO_PAGE) {
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

uint8_t CPU::get_memory(addressing_mode mode, uint8_t parameter_lsb, uint8_t parameter_msb) {
    uint16_t full_address = form_address(parameter_lsb, parameter_msb);

    increment_program_counter(3);

    if (mode == ABSOLUTE) {
        return RAM[full_address];
    } else if (mode == ABSOLUTE_X) {
        return RAM[full_address + X];
    } else if (mode == ABSOLUTE_Y) {
        return RAM[full_address + Y];
    } else {
        throw std::runtime_error("Memory addressing mode not implemented: " + std::to_string(mode));
    }
}

void CPU::execute_opcode(uint16_t opcode_address) {
    uint8_t opcode = RAM[opcode_address];
    uint8_t lsb = RAM[opcode_address + 1];
    uint8_t msb = RAM[opcode_address + 2];


    // Figure out what command the opcode corresponds to
    // Get the values (possibly in memory) required to execure it
    // Then execute the command
    switch (opcode) {
        case 0xA1:
            // LDA, indirect x
            LDA(get_memory(INDEXED_INDIRECT, lsb));
        case 0xA5:
            // LDA, zero page
            LDA(get_memory(ZERO_PAGE, lsb));
        case 0xA9:
            // LDA, immediate
            LDA(get_memory(IMMEDIATE, lsb));
        case 0xAD:
            // LDA, absolute
            LDA(get_memory(ABSOLUTE, lsb, msb));
        case 0xB1:
            // LDA, indirect y
            LDA(get_memory(INDIRECT_INDEXED, lsb));
        case 0xB5:
            // LDA, zero page x
            LDA(get_memory(ZERO_PAGE_X, lsb));
        case 0xBD:
            // LDA, absolute x
            LDA(get_memory(ABSOLUTE_X, lsb, msb));
        case 0xB9:
            // LDA, absolute y
            LDA(get_memory(ABSOLUTE_Y, lsb, msb));
        default:
            throw std::runtime_error("Unknown opcode " + std::to_string(opcode));
            break;
    };
}

void CPU::load_rom_into_memory(const std::vector<uint8_t>& rom_data) {
    for (int i = 0; i < rom_data.size(); i++) {
        RAM[PROGRAM_MEMORY_START + i] = rom_data.at(i);
    }
} 

// Execute one cycle of CPU.
// This will typically run one opcode
void CPU::tick() {
    // TODO: check for special end condition
    execute_opcode(program_counter);
}