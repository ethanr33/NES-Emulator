#include <string>
#include <stdexcept>

#include "CPU.h"
#include "Helpers.cpp"

/*
BCC - Branch if Carry Clear
If the carry flag is clear then add the relative displacement to the program counter to cause a branch to a new location.
*/
void CPU::BCC(uint8_t displacement) {
    if (!get_flag(CARRY)) {
        program_counter += displacement;
    }
}


/*
ADC - Add with Carry
This instruction adds the contents of a memory location to the accumulator together with the carry bit.
If overflow occurs the carry bit is set, this enables multiple byte addition to be performed.
*/
void CPU::ADC(uint8_t memory_val) {
    uint8_t sum = A + memory_val + static_cast<uint8_t>(get_flag(CARRY));

    // Check if overflow from bit 7
    if (sum > 0xFF) {
        set_flag(CARRY, 1);
    }

    if (sum == 0) {
        set_flag(ZERO, 1);
    }

    if (is_positive(A) && is_positive(memory_val) && sum < 0) {
        // Check if we add 2 positives and get a negative
        set_flag(OVER_FLOW, 1);
    } else if (!is_positive(A) && !is_positive(memory_val) && sum > 0) {
        // Check if we add 2 negatives and get a positive (or 0)
        set_flag(OVER_FLOW, 1);
    }

    if (is_bit_set(7, sum)) {
        set_flag(NEGATIVE, 1);
    }
}

/*
PHA - Push Accumulator
Pushes a copy of the accumulator on to the stack.
*/
void CPU::PHA() {
    RAM[stack_pointer] = A;
    stack_pointer--;
}

/*
PHP - Push Processor Status
Pushes a copy of the status flags on to the stack.
*/
void CPU::PHP() {
    RAM[stack_pointer] = get_byte_from_flags();
    stack_pointer--;
}

/*
PLA - Pull Accumulator
Pulls an 8 bit value from the stack and into the accumulator. The zero and negative flags are set as appropriate.
*/
void CPU::PLA() {
    stack_pointer++;
    A = RAM[stack_pointer];

    if (A == 0) {
        set_flag(ZERO, 1);
    }

    if (is_bit_set(7, A)) {
        set_flag(NEGATIVE, 1);
    }
}

/*
PLP - Pull Processor Status
Pulls an 8 bit value from the stack and into the processor flags. The flags will take on new states as determined by the value pulled.
*/
void CPU::PLP() {
    stack_pointer++;
    uint8_t flag_byte = RAM[stack_pointer];

    for (int i = 7; i >= 0; i--) {
        set_flag(static_cast<flag_type>(i), flag_byte & 1);
        flag_byte = flag_byte >> 1;
    }
}


/*
ORA - Logical Inclusive OR
An inclusive OR is performed, bit by bit, on the accumulator contents using the contents of a byte of memory.
*/
void CPU::ORA(uint8_t memory_val) {
    A = A | memory_val;

    if (A == 0) {
        set_flag(ZERO, 1);
    }

    if (is_bit_set(7, A)) {
        set_flag(NEGATIVE, 1);
    }
}


/*
AND - Logical AND
A logical AND is performed, bit by bit, on the accumulator contents using the contents of a byte of memory.
*/
void CPU::AND(uint8_t memory_val) {
    A = A & memory_val;

    if (A == 0) {
        set_flag(ZERO, 1);
    }

    if (is_bit_set(7, A)) {
        set_flag(NEGATIVE, 1);
    }
}


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
Transfer X to Stack Pointer
Copies the current contents of the X register into the stack register.
*/
void CPU::TXS() {

stack_pointer = X;
//No flags are affected.

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
TXA Transfer X contents to Accumulator
Copies the current contents of the X register into the accumulator and sets the zero and negative flags as appropriate.
*/

void CPU::TXA() {
    A = X;

    if (A == 0) {
        set_flag(ZERO, 1);
    }

    if (is_bit_set(7, A)) {
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

void CPU::TYA() {

    A=Y;

    if (A == 0) {
        set_flag(ZERO, 1);
    }

    if (is_bit_set(7, A)) {
        set_flag(NEGATIVE, 1);
    }

}

/*
    ASL - Arithmetic Shift Left (Accumulator version)
    This operation shifts all the bits of the accumulator one bit left. 
*/
void CPU::ASL() {
    
    set_flag(CARRY, is_bit_set(7, A));

    A = A << 1;

    if (A == 0) {
        set_flag(ZERO, 1);
    }

    if (is_bit_set(A, 7) == 1) {
        set_flag(NEGATIVE, 1);
    }
}

/*
    ASL - Arithmetic Shift Left (Memory version)
    This operation shifts all the bits of the memory contents one bit left. 
*/
void CPU::ASL(uint16_t memory_address) {
    
    set_flag(CARRY, is_bit_set(7, RAM[memory_address]));

    RAM[memory_address] = RAM[memory_address] << 1;

    if (RAM[memory_address] == 0) {
        set_flag(ZERO, 1);
    }

    if (is_bit_set(RAM[memory_address], 7) == 1) {
        set_flag(NEGATIVE, 1);
    }
}

/*
    BCS - Branch if Carry Set
    If the carry flag is set then add the relative displacement to the program counter to cause a branch to a new location.
*/
void CPU::BCS(uint8_t displacement) {
    if (get_flag(CARRY) == 1) {
        program_counter += displacement;
    }
}

/*
    BEQ - Branch if Equal
    If the zero flag is set then add the relative displacement to the program counter to cause a branch to a new location.
*/
void CPU::BEQ(uint8_t displacement) {
    if (get_flag(ZERO) == 1) {
        program_counter += displacement;
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

// Turns the flag array into a byte
// If the i-th flag is set in the flag array, the i-th bit will be 1 in the returned byte
// If the i-th flag is not set in the flag array, the i-th bit will be 0.
// The MSB of the returned byte will be position 0 in the flag array
uint8_t CPU::get_byte_from_flags() const {
    uint8_t flag_byte = 0;

    for (int i = 0; i < 8; i++) {
        flag_byte = flag_byte + static_cast<uint8_t>(flags[i]);
        flag_byte = flag_byte << 1;
    }

    return flag_byte;
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
        throw std::runtime_error("Memory addressing mode not implemented: " + std::to_string(mode));
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
            break;
        case 0xA5:
            // LDA, zero page
            LDA(get_memory(ZERO_PAGE, lsb));
            break;
        case 0xA9:
            // LDA, immediate
            LDA(get_memory(IMMEDIATE, lsb));
            break;
        case 0xAD:
            // LDA, absolute
            LDA(get_memory(ABSOLUTE, lsb, msb));
            break;
        case 0xB1:
            // LDA, indirect y
            LDA(get_memory(INDIRECT_INDEXED, lsb));
            break;
        case 0xB5:
            // LDA, zero page x
            LDA(get_memory(ZERO_PAGE_X, lsb));
            break;
        case 0xBD:
            // LDA, absolute x
            LDA(get_memory(ABSOLUTE_X, lsb, msb));
            break;
        case 0xB9:
            // LDA, absolute y
            LDA(get_memory(ABSOLUTE_Y, lsb, msb));
            break;
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
