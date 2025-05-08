#include <string>
#include <stdexcept>
#include <iostream>
#include <iomanip>

#include "CPU.h"
#include "Helpers.h"

/*
BCC - Branch if Carry Clear
If the carry flag is clear then add the relative displacement to the program counter to cause a branch to a new location.
*/
void CPU::BCC(uint8_t displacement) {
    if (!get_flag(CARRY)) {
        clock_cycles_remaining += 1;
        if (crosses_page(RELATIVE, displacement)) {
            clock_cycles_remaining += 1;
        }
        program_counter += (int8_t) displacement;
    }
}

void CPU::INY()
{
    Y = Y+1;

    if( Y==0 )
    {
        set_flag(ZERO,1);
    } else {
        set_flag(ZERO, 0);
    }

    if(is_bit_set(7,Y))
    {
        set_flag(NEGATIVE, 1);
    } else {
        set_flag(NEGATIVE, 0);
    }

}

void CPU::RTS()
{
    uint8_t lowbyte = stack_pop();
    uint8_t highbyte = stack_pop();
    uint16_t stackaddress;
    stackaddress = form_address(lowbyte,highbyte);
    program_counter = stackaddress + 1;

}

void CPU::INX()
{
    X = X+1;

    if( X==0 )
    {
        set_flag(ZERO,1);
    } else {
        set_flag(ZERO, 0);
    }

    if(is_bit_set(7,X))
    {
        set_flag(NEGATIVE, 1);
    } else {
        set_flag(NEGATIVE, 0);
    }

}

void CPU::EOR(uint8_t memory_val)
{
    A = A^memory_val;

    if( A==0 )
    {
        set_flag(ZERO,1);
    } else {
        set_flag(ZERO, 0);
    }

    if(is_bit_set(7,A))
    {
        set_flag(NEGATIVE, 1);
    } else {
        set_flag(NEGATIVE, 0);
    }
    
}

void CPU::CLI()
{
    set_flag(INT_DISABLE,0);
}

void CPU::CLD()
{
    set_flag(DECIMAL,0);
}

void CPU::CLC()
{
    set_flag(CARRY,0);
}


void CPU::SEI()
{
    set_flag(INT_DISABLE, 1);
}

void CPU::SED()
{
    set_flag(DECIMAL, 1);
}

void CPU::SEC()
{
    set_flag(CARRY, 1);
}

void CPU::LDY(uint8_t memory_val) {
    Y = memory_val;

    if (Y == 0) {
        set_flag(ZERO, 1);
    } else {
        set_flag(ZERO, 0);
    }

    if (is_bit_set(7,Y)) {
        set_flag(NEGATIVE, 1);
    } else {
        set_flag(NEGATIVE, 0);
    }
}

void CPU::CLV()
{
    set_flag(OVER_FLOW, 0);
}
/*
ADC - Add with Carry
This instruction adds the contents of a memory location to the accumulator together with the carry bit.
If overflow occurs the carry bit is set, this enables multiple byte addition to be performed.
*/
void CPU::ADC(uint8_t memory_val) {
    uint8_t x = A;
    uint8_t y = memory_val;
    uint8_t carry = static_cast<uint8_t>(get_flag(CARRY));

    uint8_t sum = x + y + carry;

    // Check for overflow, set carry flag accordingly    
    if (x + y + carry > 0xFF) {
        set_flag(CARRY, 1);
    } else {
        set_flag(CARRY, 0);
    }
    
    // Zero flag is set if the sum is 0
    if (sum == 0) {
        set_flag(ZERO, 1);
    } else {
        set_flag(ZERO, 0);
    }

    if ((sum ^ x) & (sum ^ y) & 0x80) {
        set_flag(OVER_FLOW, 1);
    } else {
        set_flag(OVER_FLOW, 0);
    }

    // if ((y == 0x6F || y == 0x7F) && carry == 1) {
    //     set_flag(OVER_FLOW, 1);
    // } else {
    //     y = y + carry;

    //     bool same_signs = (x > 0 && y > 0) || (x < 0 && y < 0);
    //     bool opposite_result = (x > 0 && sum < 0) || (x < 0 && sum >= 0);

    //     // Overflow occurs when we are adding two numbers with the same sign, and their sum doesn't
    //     // match the signs of the input
    //     if (same_signs && opposite_result) {
    //         set_flag(OVER_FLOW, 1);
    //     } else {
    //         set_flag(OVER_FLOW, 0);
    //     }
    // }

    if (is_bit_set(7, sum)) {
        set_flag(NEGATIVE, 1);
    } else {
        set_flag(NEGATIVE, 0);
    }

    A = sum;
}

/*
PHA - Push Accumulator
Pushes a copy of the accumulator on to the stack.
*/
void CPU::PHA() {
    stack_push(A);
}

/*
PHP - Push Processor Status
Pushes a copy of the status flags on to the stack.
*/
void CPU::PHP() {
    // Break flag is always set to 1 on the pushed flags
    uint8_t new_flags = get_byte_from_flags() | (1 << 5) | (1 << 4); // Break flag is in the 6th position
    stack_push(new_flags);
}

/*
PLA - Pull Accumulator
Pulls an 8 bit value from the stack and into the accumulator. The zero and negative flags are set as appropriate.
*/
void CPU::PLA() {
    A = stack_pop();

    if (A == 0) {
        set_flag(ZERO, 1);
    } else {
        set_flag(ZERO, 0);
    }

    if (is_bit_set(7, A)) {
        set_flag(NEGATIVE, 1);
    } else {
        set_flag(NEGATIVE, 0);
    }
}

/*
PLP - Pull Processor Status
Pulls an 8 bit value from the stack and into the processor flags. The flags will take on new states as determined by the value pulled.
*/
void CPU::PLP() {
    uint8_t flag_byte = stack_pop();

    set_flag(NEGATIVE, flag_byte & 0x80);
    set_flag(OVER_FLOW, flag_byte & 0x40);
    set_flag(DECIMAL, flag_byte & 0x08);
    set_flag(INT_DISABLE, flag_byte & 0x04);
    set_flag(ZERO, flag_byte & 0x02);
    set_flag(CARRY, flag_byte & 0x01);

}


/*
ORA - Logical Inclusive OR
An inclusive OR is performed, bit by bit, on the accumulator contents using the contents of a byte of memory.
*/
void CPU::ORA(uint8_t memory_val) {
    A = A | memory_val;

    if (A == 0) {
        set_flag(ZERO, 1);
    } else {
        set_flag(ZERO, 0);
    }

    if (is_bit_set(7, A)) {
        set_flag(NEGATIVE, 1);
    } else {
        set_flag(NEGATIVE, 0);
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
    } else {
        set_flag(ZERO, 0);
    }

    if (is_bit_set(7, A)) {
        set_flag(NEGATIVE, 1);
    } else {
        set_flag(NEGATIVE, 0);
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
    } else {
        set_flag(ZERO, 0);
    }

    if (is_bit_set(7, A)) {
        set_flag(NEGATIVE, 1);
    } else {
        set_flag(NEGATIVE, 0);
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
    } else {
        set_flag(ZERO, 0);
    }

    if (is_bit_set(7, X)) {
        set_flag(NEGATIVE, 1);
    } else {
        set_flag(NEGATIVE, 0);
    }
}

void CPU::LAX(uint8_t mem_val)
{
    LDX(mem_val);

    LDA(mem_val);
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
    } else {
        set_flag(ZERO, 0);
    }

    if (is_bit_set(7, X)) {
        set_flag(NEGATIVE, 1);
    } else {
        set_flag(NEGATIVE, 0);
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
    } else {
        set_flag(ZERO, 0);
    }

    if (is_bit_set(7, A)) {
        set_flag(NEGATIVE, 1);
    } else {
        set_flag(NEGATIVE, 0);
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
    } else {
        set_flag(ZERO, 0);
    }

    if (is_bit_set(7, Y)) {
        set_flag(NEGATIVE, 1);
    } else {
        set_flag(NEGATIVE, 0);
    }
}

void CPU::TYA() {

    A=Y;

    if (A == 0) {
        set_flag(ZERO, 1);
    } else {
        set_flag(ZERO, 0);
    }

    if (is_bit_set(7, A)) {
        set_flag(NEGATIVE, 1);
    } else {
        set_flag(NEGATIVE, 0);
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
    } else {
        set_flag(ZERO, 0);
    }

    if (is_bit_set(7, A) == 1) {
        set_flag(NEGATIVE, 1);
    } else {
        set_flag(NEGATIVE, 0);
    }
}

/*
    ASL - Arithmetic Shift Left (Memory version)
    This operation shifts all the bits of the memory contents one bit left. 
*/
void CPU::ASL(uint16_t memory_address) {

    uint8_t memory_val = bus->read_cpu(memory_address);

    set_flag(CARRY, is_bit_set(7, memory_val));
    
    memory_val <<= 1;

    bus->write_cpu(memory_address, memory_val);

    if (memory_val == 0) {
        set_flag(ZERO, 1);
    } else {
        set_flag(ZERO, 0);
    }

    if (is_bit_set(7, memory_val) == 1) {
        set_flag(NEGATIVE, 1);
    } else {
        set_flag(NEGATIVE, 0);
    }
}

/*
    BCS - Branch if Carry Set
    If the carry flag is set then add the relative displacement to the program counter to cause a branch to a new location.
*/
void CPU::BCS(uint8_t displacement) {
    if (get_flag(CARRY) == 1) {
        clock_cycles_remaining += 1;
        if (crosses_page(RELATIVE, displacement)) {
            clock_cycles_remaining += 1;
        }
        program_counter += (int8_t) displacement;
    }
}

/*
    BEQ - Branch if Equal
    If the zero flag is set then add the relative displacement to the program counter to cause a branch to a new location.
*/
void CPU::BEQ(uint8_t displacement) {
    if (get_flag(ZERO) == 1) {
        clock_cycles_remaining += 1;
        if (crosses_page(RELATIVE, displacement)) {
            clock_cycles_remaining += 1;
        }
        program_counter += (int8_t) displacement;
    }
}

/*
    BIT - Bit Test
*/
void CPU::BIT(uint8_t memory_val) {
    uint8_t result = A & memory_val;

    if (result == 0) {
        set_flag(ZERO, 1);
    } else {
        set_flag(ZERO, 0);
    }

    set_flag(OVER_FLOW, is_bit_set(6, memory_val));
    set_flag(NEGATIVE, is_bit_set(7, memory_val));
}

/*
    BMI - Branch if Minus
    If the negative flag is set then add the relative displacement to the program counter to cause a branch to a new location.
*/
void CPU::BMI(uint8_t displacement) {
    if (get_flag(NEGATIVE) == 1) {
        clock_cycles_remaining += 1;
        if (crosses_page(RELATIVE, displacement)) {
            clock_cycles_remaining += 1;
        }
        program_counter += (int8_t) displacement;
    }
}

/*
    BNE - Branch if Not Equal
    If the zero flag is clear then add the relative displacement to the program counter to cause a branch to a new location.
*/
void CPU::BNE(uint8_t displacement) {
    if (get_flag(ZERO) == 0) {
        clock_cycles_remaining += 1;
        if (crosses_page(RELATIVE, displacement)) {
            clock_cycles_remaining += 1;
        }
        program_counter += (int8_t) displacement;
    }
}

/*
    BPL - Branch if Positive
    If the negative flag is clear then add the relative displacement to the program counter to cause a branch to a new location.
*/
void CPU::BPL(uint8_t displacement) {
    if (get_flag(NEGATIVE) == 0) {
        clock_cycles_remaining += 1;
        if (crosses_page(RELATIVE, displacement)) {
            clock_cycles_remaining += 1;
        }
        program_counter += (int8_t) displacement;
    }
}

/*
    BVC - Branch if Overflow Clear
    If the overflow flag is clear then add the relative displacement to the program counter to cause a branch to a new location.
*/
void CPU::BVC(uint8_t displacement) {
    if (get_flag(OVER_FLOW) == 0) {
        clock_cycles_remaining += 1;
        if (crosses_page(RELATIVE, displacement)) {
            clock_cycles_remaining += 1;
        }
        program_counter += (int8_t) displacement;
    }
}

/*
    BVS - Branch if Overflow Set
    If the overflow flag is set then add the relative displacement to the program counter to cause a branch to a new location.
*/
void CPU::BVS(uint8_t displacement) {
    if (get_flag(OVER_FLOW) == 1) {
        clock_cycles_remaining += 1;
        program_counter += (int8_t) displacement;
    }
}

/*
    CMP - Compare
    This instruction compares the contents of the accumulator with another memory held value and sets the zero and carry flags as appropriate.
*/
void CPU::CMP(uint8_t memory_val) {
    uint8_t compare = A - memory_val;

    if (memory_val == A) {
        set_flag(ZERO, 1);
    } else {
        set_flag(ZERO, 0);
    }

    if (A >= memory_val) {
        set_flag(CARRY, 1);
    } else {
        set_flag(CARRY, 0);
    }

    if (is_bit_set(7, compare) == 1) {
        set_flag(NEGATIVE, 1);
    } else {
        set_flag(NEGATIVE, 0);
    }
}

/*
    CPX - Compare X Register
    This instruction compares the contents of the X register with another memory held value and sets the zero and carry flags as appropriate.
*/
void CPU::CPX(uint8_t memory_val) {
    uint8_t compare = X - memory_val;

    if (memory_val == X) {
        set_flag(ZERO, 1);
    } else {
        set_flag(ZERO, 0);
    }

    if (X >= memory_val) {
        set_flag(CARRY, 1);
    } else {
        set_flag(CARRY, 0);
    }

    if (is_bit_set(7, compare) == 1) {
        set_flag(NEGATIVE, 1);
    } else {
        set_flag(NEGATIVE, 0);
    }
}

/*
    CPY - Compare Y Register
    This instruction compares the contents of the Y register with another memory held value and sets the zero and carry flags as appropriate.
*/
void CPU::CPY(uint8_t memory_val) {
    uint8_t compare = Y - memory_val;

    if (memory_val == Y) {
        set_flag(ZERO, 1);
    } else {
        set_flag(ZERO, 0);
    }

    if (Y >= memory_val) {
        set_flag(CARRY, 1);
    } else {
        set_flag(CARRY, 0);
    }

    if (is_bit_set(7, compare) == 1) {
        set_flag(NEGATIVE, 1);
    } else {
        set_flag(NEGATIVE, 0);
    }
}

/*
    STA - Store Accumulator
    Stores the contents of the accumulator into memory.
*/
void CPU::STA(uint16_t location) {
    bus->write_cpu(location, A);
}

/*
    STX - Store X Register
    Stores the contents of the X register into memory.
*/
void CPU::STX(uint16_t location) {
    bus->write_cpu(location, X);
}

void CPU::JMP(uint16_t target) {
    /*
        TODO: In the original 6502 processor JMP does not properly fetch the target address if the indirect vector falls on a page boundary
              We need to account for this in the future
    */
    program_counter = target;
}

/*
    STY - Store Y Register
    Stores the contents of the Y register into memory.
*/
void CPU::STY(uint16_t location) {
    bus->write_cpu(location, Y);
}

/*
    ROL - Rotate Left (Accumulator)
    Move each of the bits in either A or M one place to the left.
*/
void CPU::ROL() {

    bool old_carry = get_flag(CARRY);

    set_flag(CARRY, is_bit_set(7, A));

    A = A << 1;

    if (old_carry) {
        A = A | 1;
    }

    if (A == 0) {
        set_flag(ZERO, 1);
    } else {
        set_flag(ZERO, 0);
    }

    if (is_bit_set(7, A)) {
        set_flag(NEGATIVE, 1);
    } else {
        set_flag(NEGATIVE, 0);
    }

}

/*
    ROL - Rotate Left (Memory)
    Move each of the bits in either A or M one place to the left.
*/
void CPU::ROL(uint16_t address) {

    bool old_carry = get_flag(CARRY);

    uint8_t memory_val = bus->read_cpu(address);

    set_flag(CARRY, is_bit_set(7, memory_val));

    memory_val <<= 1;

    if (old_carry) {
        memory_val = memory_val | 1;
    }

    bus->write_cpu(address, memory_val);

    if (memory_val == 0) {
        set_flag(ZERO, 1);
    } else {
        set_flag(ZERO, 0);
    }

    if (is_bit_set(7, memory_val)) {
        set_flag(NEGATIVE, 1);
    } else {
        set_flag(NEGATIVE, 0);
    }

}


/*
    ROR - Rotate Right (Accumulator)
    Move each of the bits in either A or M one place to the right.
*/
void CPU::ROR() {

    bool old_carry = get_flag(CARRY);

    set_flag(CARRY, is_bit_set(0, A));

    A = A >> 1;

    if (old_carry) {
        A = A | 0x80;
    }

    if (A == 0) {
        set_flag(ZERO, 1);
    } else {
        set_flag(ZERO, 0);
    }

    if (is_bit_set(7, A)) {
        set_flag(NEGATIVE, 1);
    } else {
        set_flag(NEGATIVE, 0);
    }

}

/*
    ROR - Rotate Right (Memory)
    Move each of the bits in either A or M one place to the right.
*/
void CPU::ROR(uint16_t address) {

    bool old_carry = get_flag(CARRY);
    uint8_t memory_val = bus->read_cpu(address);

    set_flag(CARRY, is_bit_set(0, memory_val));

    memory_val >>= 1;

    if (old_carry) {
        memory_val |= 0x80;
    }

    bus->write_cpu(address, memory_val);

    if (memory_val == 0) {
        set_flag(ZERO, 1);
    } else {
        set_flag(ZERO, 0);
    }

    if (is_bit_set(7, memory_val)) {
        set_flag(NEGATIVE, 1);
    } else {
        set_flag(NEGATIVE, 0);
    }

}

void CPU::TSX() {
    X = stack_pointer;

    if (X == 0) {
        set_flag(ZERO, 1);
    } else {
        set_flag(ZERO, 0);
    }

    if (is_bit_set(7, X)) {
        set_flag(NEGATIVE, 1);
    } else {
        set_flag(NEGATIVE, 0);
    }
}

void CPU::DEC(uint16_t memory_address) {
    uint8_t memory_val = bus->read_cpu(memory_address);
    memory_val--;
    bus->write_cpu(memory_address, memory_val);

    if (memory_val == 0) {
        set_flag(ZERO, 1);
    } else {
        set_flag(ZERO, 0);
    }

    if (is_bit_set(7, memory_val)) {
        set_flag(NEGATIVE, 1);
    } else {
        set_flag(NEGATIVE, 0);
    }
}

void CPU::DEX() {
    X--;

    if (X == 0) {
        set_flag(ZERO, 1);
    } else {
        set_flag(ZERO, 0);
    }

    if (is_bit_set(7, X)) {
        set_flag(NEGATIVE, 1);
    } else {
        set_flag(NEGATIVE, 0);
    }
}

void CPU::DEY() {
    Y--;

    if (Y == 0) {
        set_flag(ZERO, 1);
    } else {
        set_flag(ZERO, 0);
    }

    if (is_bit_set(7, Y)) {
        set_flag(NEGATIVE, 1);
    } else {
        set_flag(NEGATIVE, 0);
    }
}

void CPU::LSR() {
    set_flag(CARRY, A & 0x1);

    A = A >> 1;

    if (A == 0) {
        set_flag(ZERO, 1);
    } else {
        set_flag(ZERO, 0);
    }

    if (is_bit_set(7, A)) {
        set_flag(NEGATIVE, 1);
    } else {
        set_flag(NEGATIVE, 0);
    }
}

void CPU::LSR(uint16_t address) {
    uint8_t val = bus->read_cpu(address);

    set_flag(CARRY, val & 0x1);

    val = val >> 1;
    bus->write_cpu(address, val);

    if (val == 0) {
        set_flag(ZERO, 1);
    } else {
        set_flag(ZERO, 0);
    }

    if (is_bit_set(7, val)) {
        set_flag(NEGATIVE, 1);
    } else {
        set_flag(NEGATIVE, 0);
    }
}


void CPU::NOP() {
    // no operation, do nothing
}

void CPU::INC(uint16_t address) {
    uint8_t val = bus->read_cpu(address);
    val++;
    bus->write_cpu(address, val);

    if (val == 0) {
        set_flag(ZERO, 1);
    } else {
        set_flag(ZERO, 0);
    }

    if (is_bit_set(7, val)) {
        set_flag(NEGATIVE, 1);
    } else {
        set_flag(NEGATIVE, 0);
    }
}

void CPU::JSR(uint16_t address) {
    uint16_t new_pc = program_counter - 1;
    stack_push(new_pc);
    program_counter = address;
}

void CPU::BRK() {
    // for this we push in the order of program_counter (little endian) then processor status (flags)
    stack_push((uint16_t) (program_counter + 2));

    // Break and reserve flags are set only for the flags that are pushed onto the stack
    uint8_t pushed_flags = get_byte_from_flags() | (1 << BREAK) | (1 << RESERVED);
    
    set_flag(INT_DISABLE, 1);
    stack_push(pushed_flags);

    program_counter = form_address(bus->read_cpu(0xFFFE), bus->read_cpu(0xFFFF));
}

void CPU::RTI() {
    uint8_t new_flags = stack_pop();
    uint8_t pc_lsb = stack_pop();
    uint8_t pc_msb = stack_pop();

    program_counter = form_address(pc_lsb, pc_msb) - 1;

    set_flag(NEGATIVE, new_flags & 0x80);
    set_flag(OVER_FLOW, new_flags & 0x40);
    set_flag(DECIMAL, new_flags & 0x08);
    set_flag(INT_DISABLE, new_flags & 0x04);
    set_flag(ZERO, new_flags & 0x02);
    set_flag(CARRY, new_flags & 0x01);
}

void CPU::SBC(uint8_t mem_val) {
    ADC(~mem_val);
}

void CPU::AXS(uint8_t value) {
    X = (A & X) - value;

    if (X & 0x80) {
        set_flag(NEGATIVE, 1);
    } else {
        set_flag(NEGATIVE, 0);
    }

    if (X == 0) {
        set_flag(ZERO, 1);
    } else {
        set_flag(ZERO, 0);
    }

    if ((A & X) >= value) {
        set_flag(CARRY, 1);
    } else {
        set_flag(CARRY, 0);
    }


}

void CPU::set_flag(flag_type flag_to_set, bool new_flag_val) {
    flags[flag_to_set] = new_flag_val;
}

void CPU::toggle_flag(flag_type flag_to_toggle) {
    flags[flag_to_toggle] = !flags[flag_to_toggle];
}

bool CPU::get_flag(flag_type flag_to_get) {
    return flags[flag_to_get];
}

void CPU::attach_bus(Bus* b) {
    bus = b;
}

// Turns the flag array into a byte
// If the i-th flag is set in the flag array, the i-th bit will be 1 in the returned byte
// If the i-th flag is not set in the flag array, the i-th bit will be 0.
// The MSB of the returned byte will be position 7 in the flag array
uint8_t CPU::get_byte_from_flags() const {
    uint8_t flag_byte = 0;

    for (int i = 7; i >= 0; i--) {
        flag_byte = flag_byte | flags[i];

        if (i > 0) {
            flag_byte = flag_byte << 1;
        }
    }

    return flag_byte;
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

    if (mode == IMMEDIATE || mode == RELATIVE) {
        return parameter_val;
    } else if (mode == ZERO_PAGE) {
        return bus->read_cpu(parameter_val);
    } else if (mode == ZERO_PAGE_X) {
        // We always want to stay on zero page, and address may overflow here
        return bus->read_cpu((parameter_val + X) & 0xFF);
    } else if (mode == ZERO_PAGE_Y) {
        // We always want to stay on zero page, and address may overflow here
        return bus->read_cpu((parameter_val + Y) & 0xFF);
    } else if (mode == INDEXED_INDIRECT) {
        // Zero page wrap around may occur here, so we mod the address by the page size.
        // Luckily we can do this by only keeping the first 8 bits of the address and discarding the higher ones
        uint8_t lsb_address = parameter_val + X;
        uint8_t msb_address = lsb_address + 1;
        
        uint8_t least_significant_byte = bus->read_cpu(lsb_address);
        uint8_t most_significant_byte = bus->read_cpu(msb_address);
        return bus->read_cpu(form_address(least_significant_byte, most_significant_byte));
    } else if (mode == INDIRECT_INDEXED) {
        uint8_t least_significant_byte = bus->read_cpu(parameter_val);
        // If parameter_val = 0xFF, adding one may overflow.
        uint8_t most_significant_byte = bus->read_cpu((parameter_val + 1) & 0xFF);

        uint16_t new_address = form_address(least_significant_byte, most_significant_byte) + Y;

        return bus->read_cpu(new_address);
    } else {
        throw std::runtime_error("Memory addressing mode not implemented: " + std::to_string(mode));
    }
}

uint8_t CPU::get_memory(addressing_mode mode, uint8_t parameter_lsb, uint8_t parameter_msb) {
    uint16_t full_address = form_address(parameter_lsb, parameter_msb);

    increment_program_counter(3);

    if (mode == ABSOLUTE) {
        return bus->read_cpu(full_address);
    } else if (mode == ABSOLUTE_X) {
        // If address is too big, we may overflow
        return bus->read_cpu((full_address + X) & 0xFFFF);
    } else if (mode == ABSOLUTE_Y) {
        // If address is too big, we may overflow
        return bus->read_cpu((full_address + Y) & 0xFFFF);
    } else {
        throw std::runtime_error("Memory addressing mode not implemented: " + std::to_string(mode));
    }
}

uint16_t CPU::make_address(addressing_mode mode, uint8_t parameter_lsb) {

    increment_program_counter(2);

    if (mode == ZERO_PAGE) {
        return parameter_lsb;
    } else if (mode == ZERO_PAGE_X) {
        // We always want to stay on zero page, and address may overflow here
        return (parameter_lsb + X) & 0xFF;
    } else if (mode == ZERO_PAGE_Y) {
        // We always want to stay on zero page, and address may overflow here
        return (parameter_lsb + Y) & 0xFF;
    } else if (mode == INDEXED_INDIRECT) {
        // Zero page wrap around may occur here, so we mod the address by the page size.
        // Luckily we can do this by only keeping the first 8 bits of the address and discarding the higher ones
        uint8_t lsb_address = parameter_lsb + X;
        uint8_t msb_address = lsb_address + 1;

        uint8_t least_significant_byte = bus->read_cpu(lsb_address);
        uint8_t most_significant_byte = bus->read_cpu(msb_address);

        return form_address(least_significant_byte, most_significant_byte);
    } else if (mode == INDIRECT_INDEXED) {
        return form_address(bus->read_cpu(parameter_lsb), bus->read_cpu((parameter_lsb + 1) & 0xFF)) + Y;
    } else {
        throw std::runtime_error("Memory addressing mode not implemented: " + std::to_string(mode));
    }

}

uint16_t CPU::make_address(addressing_mode mode, uint8_t parameter_lsb, uint8_t parameter_msb) {
    increment_program_counter(3);

    if (mode == ABSOLUTE) {
        return form_address(parameter_lsb, parameter_msb);
    } else if (mode == ABSOLUTE_X) {
        return (form_address(parameter_lsb, parameter_msb) + X) & 0xFFFF;
    } else if (mode == ABSOLUTE_Y) {
        return (form_address(parameter_lsb, parameter_msb) + Y) & 0xFFFF;
    } else if (mode == INDIRECT) {
        /*
            An original 6502 has does not correctly fetch the target address if the indirect vector falls on a page boundary (e.g. $xxFF where xx is any value from $00 to $FF).
            In this case fetches the LSB from $xxFF as expected but takes the MSB from $xx00.
            This is fixed in some later chips like the 65SC02 so for compatibility always ensure the indirect vector is not at the end of the page.
        */

        uint16_t target_address = form_address(parameter_lsb, parameter_msb);
        
        if (parameter_lsb != 0xFF) {
            return form_address(bus->read_cpu(target_address), bus->read_cpu(target_address + 1));
        } else {
            // Buggy case
            return form_address(bus->read_cpu(target_address), bus->read_cpu(target_address & 0xFF00));
        }
    } else {
        throw std::runtime_error("Memory addressing mode not implemented: " + std::to_string(mode));
    }
}

bool CPU::crosses_page(addressing_mode mode, uint8_t lsb, uint8_t msb) {
    uint16_t address = form_address(lsb, msb);
    if (mode == ABSOLUTE_X) {
        return ((address + X) & 0xFF00) != (address & 0xFF00);
    } else if (mode == ABSOLUTE_Y) {
        return ((address + Y) & 0xFF00) != (address & 0xFF00); 
    } else {
        throw std::runtime_error("Tried to check crossing page with addressing mode " + std::to_string(mode));
    }
}

bool CPU::crosses_page(addressing_mode mode, uint8_t lsb) {
    if (mode == INDIRECT_INDEXED) {
        if (lsb == 0xFF) {
            return true;
        }

        uint8_t target_lsb = bus->read_cpu(lsb);
        uint8_t target_msb = bus->read_cpu(lsb + 1);

        uint8_t target_address = form_address(target_lsb, target_msb);

        return target_address + Y > 0xFF;
    } else if (mode == RELATIVE) {
        uint16_t target_address = program_counter + (int8_t) lsb;

        return (target_address & 0xFF00) != (program_counter & 0xFF00);
    } else {
        throw std::runtime_error("Tried to check crossing page with addressing mode " + std::to_string(mode));
    }
}

void CPU::execute_opcode(uint16_t opcode_address) {
    uint8_t opcode = bus->read_cpu(opcode_address);
    uint8_t lsb = bus->read_cpu(opcode_address + 1);
    uint8_t msb = bus->read_cpu(opcode_address + 2);

    //std::cout << std::uppercase << std::hex << std::setfill('0') << std::setw(4) << program_counter << std::endl;

    // Figure out what command the opcode corresponds to
    // Get the values (possibly in memory) required to execure it
    // Then execute the command
    // Extra opcodes: http://www.ffd2.com/fridge/docs/6502-NMOS.extra.opcodes
    switch (opcode) {
        case 0xF4:
            increment_program_counter(2);
            num_clock_cycles += 4;
            break;
        case 0xD4:
            increment_program_counter(2);
            num_clock_cycles += 4;
            break;
        case 0x74:
            increment_program_counter(2);
            num_clock_cycles += 4;
            break;
        case 0x54:
            increment_program_counter(2);
            num_clock_cycles += 4;
            break;
        case 0x34:
            increment_program_counter(2);
            num_clock_cycles += 4;
            break;
        case 0x14:
            increment_program_counter(2);
            num_clock_cycles += 4;
            break;
        case 0x64:
            // NOP
            increment_program_counter(2);
            num_clock_cycles += 3;
            break;
        case 0x44:
            // NOP
            increment_program_counter(2);
            num_clock_cycles += 3;
            break;
        case 0xFC:
            // NOP
            increment_program_counter(3);
            num_clock_cycles += 4;

            if (crosses_page(ABSOLUTE_X, lsb, msb)) {
                num_clock_cycles++;
            }

            break;
        case 0xDC:
            // NOP
            increment_program_counter(3);
            num_clock_cycles += 4;

            if (crosses_page(ABSOLUTE_X, lsb, msb)) {
                num_clock_cycles++;
            }

            break;
        case 0x7C:
            // NOP
            increment_program_counter(3);
            num_clock_cycles += 4;

            if (crosses_page(ABSOLUTE_X, lsb, msb)) {
                num_clock_cycles++;
            }

            break;
        case 0x5C:
            // NOP
            increment_program_counter(3);
            num_clock_cycles += 4;

            if (crosses_page(ABSOLUTE_X, lsb, msb)) {
                num_clock_cycles++;
            }

            break;
        case 0x3C:
            // NOP
            increment_program_counter(3);
            num_clock_cycles += 4;

            if (crosses_page(ABSOLUTE_X, lsb, msb)) {
                num_clock_cycles++;
            }

            break;
        case 0x0C:
            // NOP
            increment_program_counter(3);
            num_clock_cycles += 4;
            break;
        case 0x1C:
            // NOP
            increment_program_counter(3);
            num_clock_cycles += 4;

            if (crosses_page(ABSOLUTE_X, lsb, msb)) {
                num_clock_cycles++;
            }

            break;
        case 0x80:
            // NOP
            increment_program_counter(2);
            num_clock_cycles += 2;
            break;
        case 0x82:
            // NOP
            increment_program_counter(2);
            num_clock_cycles += 2;
            break;
        case 0x89:
            // NOP
            increment_program_counter(2);
            num_clock_cycles += 2;
            break;
        case 0xC2:
            // NOP
            increment_program_counter(2);
            num_clock_cycles += 2;
            break;
        case 0xE2:
            // NOP
            increment_program_counter(2);
            num_clock_cycles += 2;
            break;
        case 0x1A:
            // NOP
            increment_program_counter(1);
            num_clock_cycles += 2;
            break;
        case 0x3A:
            // NOP
            increment_program_counter(1);
            num_clock_cycles += 2;
            break;
        case 0x5A:
            // NOP
            increment_program_counter(1);
            num_clock_cycles += 2;
            break;
        case 0x7A:
            // NOP
            increment_program_counter(1);
            num_clock_cycles += 2;
            break;
        case 0xDA:
            // NOP
            increment_program_counter(1);
            num_clock_cycles += 2;
            break;
        case 0xFA:
            // NOP
            increment_program_counter(1);
            num_clock_cycles += 2;
            break;
        case 0x02:
            // HLT
            bus->halt();
            break;
        case 0x12:
            // HLT
            bus->halt();
            break;
        case 0x22:
            // HLT
            bus->halt();
            break;
        case 0x32:
            // HLT
            bus->halt();
            break;
        case 0x42:
            // HLT
            bus->halt();
            break;
        case 0x52:
            // HLT
            bus->halt();
            break;
        case 0x62:
            // HLT
            bus->halt();
            break;
        case 0x72:
            // HLT
            bus->halt();
            break;
        case 0x92:
            // HLT
            bus->halt();
            break;
        case 0xB2:
            // HLT
            bus->halt();
            break;
        case 0xD2:
            // HLT
            bus->halt();
            break;
        case 0xF2:
            // HLT
            bus->halt();
            break;
        case 0xA1:
            // LDA, indirect x
            clock_cycles_remaining += 6;
            LDA(get_memory(INDEXED_INDIRECT, lsb));
            break;
        case 0xA5:
            // LDA, zero page
            clock_cycles_remaining += 3;
            LDA(get_memory(ZERO_PAGE, lsb));
            break;
        case 0xA9:
            // LDA, immediate
            clock_cycles_remaining += 2;
            LDA(get_memory(IMMEDIATE, lsb));
            break;
        case 0xAD:
            // LDA, absolute
            clock_cycles_remaining += 4;
            LDA(get_memory(ABSOLUTE, lsb, msb));
            break;
        case 0xB1:
            // LDA, indirect y
            if (crosses_page(INDIRECT_INDEXED, lsb)) {
                clock_cycles_remaining += 6;
            } else {
                clock_cycles_remaining += 5;
            }
            LDA(get_memory(INDIRECT_INDEXED, lsb));
            break;
        case 0xB5:
            // LDA, zero page x
            clock_cycles_remaining += 4;
            LDA(get_memory(ZERO_PAGE_X, lsb));
            break;
        case 0xBD:
            // LDA, absolute x
            if (crosses_page(ABSOLUTE_X, lsb, msb)) {
                clock_cycles_remaining += 5;
            } else {
                clock_cycles_remaining += 4;
            }
            LDA(get_memory(ABSOLUTE_X, lsb, msb));
            break;
        case 0xB9:
            // LDA, absolute y
            if (crosses_page(ABSOLUTE_Y, lsb, msb)) {
                clock_cycles_remaining += 5;
            } else {
                clock_cycles_remaining += 4;
            }
            LDA(get_memory(ABSOLUTE_Y, lsb, msb));
            break;
        case 0xC8:
            //INY
            clock_cycles_remaining += 2;
            INY();
            increment_program_counter(1);
            break;
        case 0xE8:
            //INX
            clock_cycles_remaining += 2;
            INX();
            increment_program_counter(1);
            break;
        case 0xEA:
            //NOP
            clock_cycles_remaining += 2;
            increment_program_counter(1);
            break;
        case 0x48:
            //PHA
            clock_cycles_remaining += 3;
            PHA();
            increment_program_counter(1);
            break;
        case 0x08:
            //PHP
            clock_cycles_remaining += 3;
            PHP();
            increment_program_counter(1);
            break;
        case 0x68:
            //PLA
            clock_cycles_remaining += 4;
            PLA();
            increment_program_counter(1);
            break;
        case 0x28:
            //PLP
            clock_cycles_remaining += 4;
            PLP();
            increment_program_counter(1);
            break;
        case 0xA2:
            clock_cycles_remaining += 2;
            LDX(get_memory(IMMEDIATE, lsb));
            break;
        case 0xAE:
            clock_cycles_remaining += 4;
            LDX(get_memory(ABSOLUTE, lsb, msb));
            break;
        case 0xA6:
            clock_cycles_remaining += 3;
            LDX(get_memory(ZERO_PAGE, lsb));
            break;
        case 0xB6:
            clock_cycles_remaining += 4;
            LDX(get_memory(ZERO_PAGE_Y,lsb));
            break;
        case 0xBE:
            if (crosses_page(ABSOLUTE_Y, lsb, msb)) {
                clock_cycles_remaining += 5;
            } else {
                clock_cycles_remaining += 4;
            }
            LDX(get_memory(ABSOLUTE_Y, lsb, msb));
            break;
        case 0xC9:
            clock_cycles_remaining += 2;
            CMP(get_memory(IMMEDIATE, lsb));
            break;
        case 0xC5:
            clock_cycles_remaining += 3;
            CMP(get_memory(ZERO_PAGE,lsb));
            break;
        case 0xD5:
            clock_cycles_remaining += 4;
            CMP(get_memory(ZERO_PAGE_X,lsb));
            break;
        case 0xCD:
            clock_cycles_remaining += 4;
            CMP(get_memory(ABSOLUTE, lsb, msb));
            break;
        case 0xDD:
            if (crosses_page(ABSOLUTE_X, lsb, msb)) {
                clock_cycles_remaining += 5;
            } else {
                clock_cycles_remaining += 4;
            }
            CMP(get_memory(ABSOLUTE_X, lsb, msb));
            break;
        case 0xD9:
            if (crosses_page(ABSOLUTE_Y, lsb, msb)) {
                clock_cycles_remaining += 5;
            } else {
                clock_cycles_remaining += 4;
            }
            CMP(get_memory(ABSOLUTE_Y, lsb, msb));
            break;
        case 0xC1:
            clock_cycles_remaining += 6;
            CMP(get_memory(INDEXED_INDIRECT, lsb));
            break;
        case 0xD1:
            if (crosses_page(INDIRECT_INDEXED, lsb)) {
                clock_cycles_remaining += 6;
            } else {
                clock_cycles_remaining += 5;
            }
            CMP(get_memory(INDIRECT_INDEXED, lsb));
            break;
        case 0x29:
            clock_cycles_remaining += 2;
            AND(get_memory(IMMEDIATE, lsb));
            break;
        case 0x25:
            clock_cycles_remaining += 3;
            AND(get_memory(ZERO_PAGE, lsb));
            break;
        case 0x35:
            clock_cycles_remaining += 4;
            AND(get_memory(ZERO_PAGE_X, lsb));
            break;
        case 0x2D:
            clock_cycles_remaining += 4;
            AND(get_memory(ABSOLUTE, lsb, msb));
            break;
        case 0x3D:
            if (crosses_page(ABSOLUTE_X, lsb, msb)) {
                clock_cycles_remaining += 5;
            } else {
                clock_cycles_remaining += 4;
            }
            AND(get_memory(ABSOLUTE_X, lsb, msb));
            break;
        case 0x39:
            if (crosses_page(ABSOLUTE_Y, lsb, msb)) {
                clock_cycles_remaining += 5;
            } else {
                clock_cycles_remaining += 4;
            }
            AND(get_memory(ABSOLUTE_Y, lsb, msb));
            break;
        case 0x21:
            clock_cycles_remaining += 6;
            AND(get_memory(INDEXED_INDIRECT, lsb));
            break;
        case 0x31:
            if (crosses_page(INDIRECT_INDEXED, lsb)) {
                clock_cycles_remaining += 6;
            } else {
                clock_cycles_remaining += 5;
            }
            AND(get_memory(INDIRECT_INDEXED, lsb));
            break;
        case 0xA0:
            clock_cycles_remaining += 2;
            LDY(get_memory(IMMEDIATE, lsb));
            break;
        case 0xA4:
            clock_cycles_remaining += 3;
            LDY(get_memory(ZERO_PAGE, lsb));
            break;
        case 0xB4:
            clock_cycles_remaining += 4;
            LDY(get_memory(ZERO_PAGE_X, lsb));
            break;
        case 0xAC:
            clock_cycles_remaining += 4;
            LDY(get_memory(ABSOLUTE, lsb, msb));
            break;
        case 0xBC:
            if (crosses_page(ABSOLUTE_X, lsb, msb)) {
                clock_cycles_remaining += 5;
            } else {
                clock_cycles_remaining += 4;
            }
            LDY(get_memory(ABSOLUTE_X, lsb, msb));
            break;
        case 0x85:
            clock_cycles_remaining += 3;
            STA(make_address(ZERO_PAGE, lsb));
            break;
        case 0x95:
            clock_cycles_remaining += 4;
            STA(make_address(ZERO_PAGE_X, lsb));
            break;
        case 0x8D:
            clock_cycles_remaining += 4;
            STA(make_address(ABSOLUTE, lsb, msb));
            break;
        case 0x9D:
            clock_cycles_remaining += 5;
            STA(make_address(ABSOLUTE_X, lsb, msb));
            break;
        case 0x99:
            clock_cycles_remaining += 5;
            STA(make_address(ABSOLUTE_Y, lsb, msb));
            break;
        case 0x81:
            clock_cycles_remaining += 6;
            STA(make_address(INDEXED_INDIRECT, lsb));
            break;
        case 0x91:
            clock_cycles_remaining += 6;
            STA(make_address(INDIRECT_INDEXED, lsb));
            break;
        case 0x86:
            clock_cycles_remaining += 3;
            STX(make_address(ZERO_PAGE, lsb));
            break;
        case 0x96:
            clock_cycles_remaining += 4;
            STX(make_address(ZERO_PAGE_Y, lsb));
            break;
        case 0x8E:
            clock_cycles_remaining += 4;
            STX(make_address(ABSOLUTE, lsb, msb));
            break;
        case 0x84:
            clock_cycles_remaining += 3;
            STY(make_address(ZERO_PAGE, lsb));
            break;
        case 0x94:
            clock_cycles_remaining += 4;
            STY(make_address(ZERO_PAGE_X, lsb));
            break;
        case 0x8C:
            clock_cycles_remaining += 4;
            STY(make_address(ABSOLUTE, lsb, msb));
            break;
        case 0xAA:
            clock_cycles_remaining += 2;
            TAX();
            increment_program_counter(1);
            break;
        case 0xA8:
            clock_cycles_remaining += 2;
            TAY();
            increment_program_counter(1);
            break;
        case 0xBA:
            clock_cycles_remaining += 2;
            TSX();
            increment_program_counter(1);
            break;
        case 0x8A:
            clock_cycles_remaining += 2;
            TXA();
            increment_program_counter(1);
            break;
        case 0xA3:
            clock_cycles_remaining +=6;
            LAX(get_memory(INDEXED_INDIRECT, lsb));
            break;
        case 0xA7:
            clock_cycles_remaining +=3;
            LAX(get_memory(ZERO_PAGE,  lsb));
            break;
        case 0xAF:
            clock_cycles_remaining +=4;
            LAX(get_memory(ABSOLUTE, lsb, msb));
            break;
        case 0xB3:
            if(crosses_page(INDIRECT_INDEXED, lsb))
            {
                clock_cycles_remaining +=6;
            }
            else
            {
                clock_cycles_remaining +=5;
            }

            LAX(get_memory(INDIRECT_INDEXED, lsb));
            break;
        case 0xB7:
            clock_cycles_remaining +=4;
            LAX(get_memory(ZERO_PAGE_Y, lsb));
            break;
        case 0xBF:
            if(crosses_page(ABSOLUTE_Y, lsb, msb))
            {
                clock_cycles_remaining +=4;
            }
            else
            {
                clock_cycles_remaining +=5;
            }
            LAX(get_memory(ABSOLUTE_Y, lsb, msb));
            break;
        case 0x9A:
            clock_cycles_remaining += 2;
            TXS();
            increment_program_counter(1);
            break;
        case 0x98:
            clock_cycles_remaining += 2;
            TYA();
            increment_program_counter(1);
            break;
        case 0xD8:
            clock_cycles_remaining += 2;
            CLD();
            increment_program_counter(1);
            break;
        case 0x58:
            clock_cycles_remaining += 2;
            CLI();
            increment_program_counter(1);
            break;
        case 0xB8:
            clock_cycles_remaining += 2;
            CLV();
            increment_program_counter(1);
            break;
        case 0x38:
            clock_cycles_remaining += 2;
            SEC();
            increment_program_counter(1);
            break;
        case 0xF8:
            clock_cycles_remaining += 2;
            SED();
            increment_program_counter(1);
            break;
        case 0x78:
            clock_cycles_remaining += 2;
            SEI();
            increment_program_counter(1);
            break;
        case 0x00:
            clock_cycles_remaining += 7;
            BRK();
            break;
        case 0x40:
            clock_cycles_remaining += 6;
            RTI();
            increment_program_counter(1);
            break;
        case 0x60:
            clock_cycles_remaining += 6;
            RTS();
            break;
        case 0xCA:
            clock_cycles_remaining += 2;
            DEX();
            increment_program_counter(1);
            break;
        case 0x88:
            clock_cycles_remaining += 2;
            DEY();
            increment_program_counter(1);
            break;
        case 0xC6:
            clock_cycles_remaining += 5;
            DEC(make_address(ZERO_PAGE, lsb));
            break;
        case 0xD6:
            clock_cycles_remaining += 6;
            DEC(make_address(ZERO_PAGE_X, lsb));
            break;
        case 0xCE:
            clock_cycles_remaining += 6;
            DEC(make_address(ABSOLUTE, lsb, msb));
            break;
        case 0xDE:
            clock_cycles_remaining += 7;
            DEC(make_address(ABSOLUTE_X, lsb, msb));
            break;
        case 0x49:
            clock_cycles_remaining += 2;
            EOR(get_memory(IMMEDIATE, lsb));
            break;
        case 0x45:
            clock_cycles_remaining += 3;
            EOR(get_memory(ZERO_PAGE, lsb));
            break;
        case 0x55:
            clock_cycles_remaining += 4;
            EOR(get_memory(ZERO_PAGE_X, lsb));
            break;
        case 0x4D:
            clock_cycles_remaining += 4;
            EOR(get_memory(ABSOLUTE, lsb, msb));
            break;
        case 0x5D:
            if (crosses_page(ABSOLUTE_X, lsb, msb)) {
                clock_cycles_remaining += 5;
            } else {
                clock_cycles_remaining += 4;
            }
            EOR(get_memory(ABSOLUTE_X, lsb, msb));
            break;
        case 0x59:
            if (crosses_page(ABSOLUTE_Y, lsb, msb)) {
                clock_cycles_remaining += 5;
            } else {
                clock_cycles_remaining += 4;
            }
            EOR(get_memory(ABSOLUTE_Y, lsb, msb));
            break;
        case 0x41:
            clock_cycles_remaining += 6;
            EOR(get_memory(INDEXED_INDIRECT, lsb));
            break;
        case 0x51:
            if (crosses_page(INDIRECT_INDEXED, lsb)) {
                clock_cycles_remaining += 6;
            } else {
                clock_cycles_remaining += 5;
            }
            EOR(get_memory(INDIRECT_INDEXED, lsb));
            break;
        case 0x09:
            clock_cycles_remaining += 2;
            ORA(get_memory(IMMEDIATE, lsb));
            break;
        case 0x05:
            clock_cycles_remaining += 3;
            ORA(get_memory(ZERO_PAGE, lsb));
            break;
        case 0x15:
            clock_cycles_remaining += 4;
            ORA(get_memory(ZERO_PAGE_X, lsb));
            break;
        case 0x0D:
            clock_cycles_remaining += 4;
            ORA(get_memory(ABSOLUTE, lsb, msb));
            break;
        case 0x1D:
            if (crosses_page(ABSOLUTE_X, lsb, msb)) {
                clock_cycles_remaining += 5;
            } else {
                clock_cycles_remaining += 4;
            }
            ORA(get_memory(ABSOLUTE_X, lsb, msb));
            break;
        case 0x19:
            if (crosses_page(ABSOLUTE_Y, lsb, msb)) {
                clock_cycles_remaining += 5;
            } else {
                clock_cycles_remaining += 4;
            }
            ORA(get_memory(ABSOLUTE_Y, lsb, msb));
            break;
        case 0x01:
            clock_cycles_remaining += 6;
            ORA(get_memory(INDEXED_INDIRECT, lsb));
            break;
        case 0x11:
            if (crosses_page(INDIRECT_INDEXED, lsb)) {
                clock_cycles_remaining += 6;
            } else {
                clock_cycles_remaining += 5;
            }
            ORA(get_memory(INDIRECT_INDEXED, lsb));
            break;
        case 0x24:
            clock_cycles_remaining += 3;
            BIT(get_memory(ZERO_PAGE, lsb));
            break;
        case 0x2C:
            clock_cycles_remaining += 4;
            BIT(get_memory(ABSOLUTE, lsb, msb));
            break;
        case 0x69:
            clock_cycles_remaining += 2;
            ADC(get_memory(IMMEDIATE, lsb));
            break;
        case 0x65:
            clock_cycles_remaining += 3;
            ADC(get_memory(ZERO_PAGE, lsb));
            break;
        case 0x75:
            clock_cycles_remaining += 4;
            ADC(get_memory(ZERO_PAGE_X, lsb));
            break;
        case 0x6D:
            clock_cycles_remaining += 4;
            ADC(get_memory(ABSOLUTE, lsb, msb));
            break;
        case 0x7D:
            if (crosses_page(ABSOLUTE_X, lsb, msb)) {
                clock_cycles_remaining += 5;
            } else {
                clock_cycles_remaining += 4;
            }
            ADC(get_memory(ABSOLUTE_X, lsb, msb));
            break;
        case 0x79:
            if (crosses_page(ABSOLUTE_Y, lsb, msb)) {
                clock_cycles_remaining += 5;
            } else {
                clock_cycles_remaining += 4;
            }
            ADC(get_memory(ABSOLUTE_Y, lsb, msb));
            break;
        case 0x61:
            clock_cycles_remaining += 6;
            ADC(get_memory(INDEXED_INDIRECT, lsb));
            break;
        case 0x71:
            if (crosses_page(INDIRECT_INDEXED, lsb)) {
                clock_cycles_remaining += 6;
            } else {
                clock_cycles_remaining += 5;
            }
            ADC(get_memory(INDIRECT_INDEXED, lsb));
            break;
        case 0xE9:
            clock_cycles_remaining += 2;
            SBC(get_memory(IMMEDIATE, lsb));
            break;
        case 0xE5:
            clock_cycles_remaining += 3;
            SBC(get_memory(ZERO_PAGE, lsb));
            break;
        case 0xF5:
            clock_cycles_remaining += 4;
            SBC(get_memory(ZERO_PAGE_X, lsb));
            break;
        case 0xED:
            clock_cycles_remaining += 4;
            SBC(get_memory(ABSOLUTE, lsb, msb));
            break;
        case 0xFD:
            if (crosses_page(ABSOLUTE_X, lsb, msb)) {
                clock_cycles_remaining += 5;
            } else {
                clock_cycles_remaining += 4;
            }
            SBC(get_memory(ABSOLUTE_X, lsb, msb));
            break;
        case 0xF9:
            if (crosses_page(ABSOLUTE_Y, lsb, msb)) {
                clock_cycles_remaining += 5;
            } else {
                clock_cycles_remaining += 4;
            }
            SBC(get_memory(ABSOLUTE_Y, lsb, msb));
            break;
        case 0xE1:
            clock_cycles_remaining += 6;
            SBC(get_memory(INDEXED_INDIRECT, lsb));
            break;
        case 0xF1:
            if (crosses_page(INDIRECT_INDEXED, lsb)) {
                clock_cycles_remaining += 6;
            } else {
                clock_cycles_remaining += 5;
            }
            SBC(get_memory(INDIRECT_INDEXED, lsb));
            break;
        case 0xE0:
            clock_cycles_remaining += 2;
            CPX(get_memory(IMMEDIATE, lsb));
            break;
        case 0xE4:
            clock_cycles_remaining += 3;
            CPX(get_memory(ZERO_PAGE, lsb));
            break;
        case 0xEC:
            clock_cycles_remaining += 4;
            CPX(get_memory(ABSOLUTE, lsb, msb));
            break;
        case 0xC0:
            clock_cycles_remaining += 2;
            CPY(get_memory(IMMEDIATE, lsb));
            break;
        case 0xC4:
            clock_cycles_remaining += 3;
            CPY(get_memory(ZERO_PAGE, lsb));
            break;
        case 0xCC:
            clock_cycles_remaining += 4;
            CPY(get_memory(ABSOLUTE, lsb, msb));
            break;
        case 0x0A:
            clock_cycles_remaining += 2;
            ASL();
            increment_program_counter(1);
            break;
        case 0x06:
            clock_cycles_remaining += 5;
            ASL(make_address(ZERO_PAGE, lsb));
            break;
        case 0x16:
            clock_cycles_remaining += 6;
            ASL(make_address(ZERO_PAGE_X, lsb));
            break;
        case 0x0E:
            clock_cycles_remaining += 6;
            ASL(make_address(ABSOLUTE, lsb, msb));
            break;
        case 0x1E:
            clock_cycles_remaining += 7;
            ASL(make_address(ABSOLUTE_X, lsb, msb));
            break;
        case 0x4A:
            clock_cycles_remaining += 2;
            LSR();
            increment_program_counter(1);
            break;
        case 0x46:
            clock_cycles_remaining += 5;
            LSR(make_address(ZERO_PAGE, lsb));
            break;
        case 0x56:
            clock_cycles_remaining += 6;
            LSR(make_address(ZERO_PAGE_X, lsb));
            break;
        case 0x4E:
            clock_cycles_remaining += 6;
            LSR(make_address(ABSOLUTE, lsb, msb));
            break;
        case 0x5E:
            clock_cycles_remaining += 7;
            LSR(make_address(ABSOLUTE_X, lsb, msb));
            break;
        case 0x2A:
            clock_cycles_remaining += 2;
            ROL();
            increment_program_counter(1);
            break;
        case 0x26:
            clock_cycles_remaining += 5;
            ROL(make_address(ZERO_PAGE, lsb));
            break;
        case 0x36:
            clock_cycles_remaining += 6;
            ROL(make_address(ZERO_PAGE_X, lsb));
            break;
        case 0x2E:
            clock_cycles_remaining += 6;
            ROL(make_address(ABSOLUTE, lsb, msb));
            break;
        case 0x3E:
            clock_cycles_remaining += 7;
            ROL(make_address(ABSOLUTE_X, lsb, msb));
            break;
        case 0x6A:
            clock_cycles_remaining += 2;
            ROR();
            increment_program_counter(1);
            break;
        case 0x66:
            clock_cycles_remaining += 5;
            ROR(make_address(ZERO_PAGE, lsb));
            break;
        case 0x76:
            clock_cycles_remaining += 6;
            ROR(make_address(ZERO_PAGE_X, lsb));
            break;
        case 0x6E:
            clock_cycles_remaining += 6;
            ROR(make_address(ABSOLUTE, lsb, msb));
            break;
        case 0x7E:
            clock_cycles_remaining += 7;
            ROR(make_address(ABSOLUTE_X, lsb, msb));
            break;
        case 0x4C:
            clock_cycles_remaining =+ 3;
            JMP(make_address(ABSOLUTE, lsb, msb));
            break;
        case 0x6C:
            clock_cycles_remaining += 5;
            JMP(make_address(INDIRECT, lsb, msb));
            break;
        case 0x20:
            clock_cycles_remaining += 6;
            JSR(make_address(ABSOLUTE, lsb, msb));
            break;
        case 0x90:
            clock_cycles_remaining += 2;
            BCC(get_memory(RELATIVE, lsb));
            break;
        case 0xB0:
            clock_cycles_remaining += 2;
            BCS(get_memory(RELATIVE, lsb));
            break;
        case 0xF0:
            clock_cycles_remaining += 2;
            BEQ(get_memory(RELATIVE, lsb));
            break;
        case 0x30:
            clock_cycles_remaining += 2;
            BMI(get_memory(RELATIVE, lsb));
            break;
        case 0xD0:
            clock_cycles_remaining += 2;
            BNE(get_memory(RELATIVE, lsb));
            break;
        case 0x10:
            clock_cycles_remaining += 2;
            BPL(get_memory(RELATIVE, lsb));
            break;
        case 0x50:
            clock_cycles_remaining += 2;
            BVC(get_memory(RELATIVE, lsb));
            break;
        case 0x70:
            clock_cycles_remaining += 2;
            if (crosses_page(RELATIVE, lsb)) {
                clock_cycles_remaining += 1;
            }
            BVS(get_memory(RELATIVE, lsb));
            break;
        case 0xE6:
            clock_cycles_remaining += 5;
            INC(make_address(ZERO_PAGE, lsb));
            break;
        case 0xF6:
            clock_cycles_remaining += 6;
            INC(make_address(ZERO_PAGE_X, lsb));
            break;
        case 0xEE:
            clock_cycles_remaining += 6;
            INC(make_address(ABSOLUTE, lsb, msb));
            break;
        case 0xFE:
            clock_cycles_remaining += 7;
            INC(make_address(ABSOLUTE_X, lsb, msb));
            break;
        case 0x18:
            clock_cycles_remaining += 2;
            CLC();
            increment_program_counter(1);
            break;
        case 0x04:
            clock_cycles_remaining += 3;
            increment_program_counter(2);
            break;
        default:
            std::cout << program_counter << std::endl;
            throw std::runtime_error("Unknown opcode " + std::to_string(opcode));
            break;
    };
}

// Execute one cycle of CPU.
// This will typically run one opcode
void CPU::tick() {

    if (nmi_edge_detected) {
        nmi_latch_set = true;
    }

    // We may have some cycles left before we can execute the next opcode, but the PC will still be pointed to the next one
    if (clock_cycles_remaining == 0) {
        //std::cout << std::uppercase << std::hex << program_counter << "  ";
        //std::cout << std::setfill(' ') << std::dec << std::left << std::setw(3) << num_clock_cycles << " " << std::setw(3) << bus->ppu->cur_dot << " " << std::setw(3) << bus->ppu->scanline << std::endl;
        //std::cout << std::setfill(' ') << std::hex << std::left << std::setw(2) << (int) bus->read_cpu(program_counter) << "  ";
        //std::cout << std::setfill(' ') << std::hex << std::left << std::setw(2) << (int) stack_pointer << "  ";
        //std::cout << std::setfill(' ') << std::dec << std::left << std::setw(8) << num_opcodes_executed << std::endl;

        num_opcodes_executed++;
        execute_opcode(program_counter);

        if (nmi_flag) {

            nmi_flag = false;
            clock_cycles_remaining += 8;
                        
            stack_push(program_counter);
            set_flag(INT_DISABLE, 1);
            set_flag(BREAK, 0);
            stack_push(get_byte_from_flags());
            uint16_t nmi_interrput_address = form_address(bus->read_cpu(0xFFFA), bus->read_cpu(0xFFFB));

            nmi_latch_set = false;

            program_counter = nmi_interrput_address;
        }
    } else if (clock_cycles_remaining == 1) {
        // Check for pending NMI interrupt
        if (nmi_latch_set) {
            nmi_flag = true;
        } else {
            nmi_flag = false;
        }
    }

    clock_cycles_remaining -= 1;
    num_clock_cycles += 1;

    check_nmi_edge();

}

void CPU::execute_next_opcode() {
    // There may be some time remaining before we start the next opcode, we need to account for that
    // We are just skipping forward

    execute_opcode(program_counter);

    num_clock_cycles += clock_cycles_remaining;
    clock_cycles_remaining = 0;
}

void CPU::stack_push(uint8_t new_val) {
    bus->write_cpu(0x100 | stack_pointer, new_val);
    stack_pointer--;
}

void CPU::stack_push(uint16_t new_val) {
    uint8_t lsb = new_val & 0xFF;
    uint8_t msb = new_val >> 8;

    stack_push(msb);
    stack_push(lsb);
} 

uint8_t CPU::stack_pop() {
    stack_pointer++;
    uint8_t temp = bus->read_cpu(0x100 | stack_pointer);
    return temp;
}

void CPU::reset() {
    clock_cycles_remaining = 8;
    num_clock_cycles = 0;
    program_counter = (bus->read_cpu(0xFFFD) << 8) | bus->read_cpu(0xFFFC);
    stack_pointer = STACK_LOCATION_START & 0xFF;
    
    for (int i = 0; i < 8; i++) {
        flags[i] = DEFAULT_FLAGS[i];    
    }
}

void CPU::check_nmi_edge() {
    // NMI input is connected to a edge detector
    // Polled on the second half of every CPU cycle
    // If the line goes from being high to low, then an internal signal is set

    bool cur_nmi_line_status = bus->get_nmi_line_status();

    if (cur_nmi_line_status && !is_nmi_line_low) {
        nmi_edge_detected = true;
    } else {
        nmi_edge_detected = false;
    }

    is_nmi_line_low = cur_nmi_line_status;
}