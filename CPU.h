
#pragma once
#include <cstdint>
#include <vector>

struct Bus;

/*
    See:
    https://www.nesdev.org/obelisk-6502-guide/registers.html
    https://www.nesdev.org/obelisk-6502-guide/architecture.html
*/

enum flag_type {CARRY, ZERO, INT_DISABLE, DECIMAL, BREAK, RESERVED, OVER_FLOW, NEGATIVE};
enum addressing_mode {IMPLICIT, ACCUMULATOR, IMMEDIATE, ZERO_PAGE, ZERO_PAGE_X, ZERO_PAGE_Y, RELATIVE, ABSOLUTE, ABSOLUTE_X, ABSOLUTE_Y, INDIRECT, INDEXED_INDIRECT, INDIRECT_INDEXED};

struct CPU {
    const uint16_t STACK_LOCATION_START = 0x1FD;
    const uint16_t STACK_LOCATION = 0x1FF;
    const uint16_t PAGE_SIZE = 0x100;

    // On a normal NES, when we load a game we need to call the interrupt handler for RESET. This takes 7 cycles
    int num_clock_cycles = 0;
    int clock_cycles_remaining = 0;
    
    // The stack pointer stores the low 8 bytes of the next memory location available on the stack
    // 
    uint8_t stack_pointer = STACK_LOCATION_START;
    uint16_t program_counter;
    uint8_t A = 0; // Accumulator register A
    uint8_t X = 0; // Index register X
    uint8_t Y = 0; // Index register Y
    // Array to store flags of processor
    // Index 0: Carry flag
    // Index 1: Zero flag
    // Index 2: Interrupt disable
    // Index 3: Decimal Mode
    // Index 4: Break command
    // Index 5: Reserved
    // Index 6: Overflow flag
    // Index 7: Negative flag
    bool const DEFAULT_FLAGS[8] = {0, 0, 0, 0, 0, 1, 0, 0};
    bool flags[8] = {0, 0, 0, 0, 0, 1, 0, 0};

    Bus* bus;

    void attach_bus(Bus*);

    void increment_program_counter(int);

    // Methods to control flags
    void set_flag(flag_type, bool);
    void toggle_flag(flag_type);
    uint8_t get_byte_from_flags() const;

    // Given an addressing mode and parameters, return the value stored in memory
    // Some addressing modes only require one parameter, so there are 2 versions of the method:
    // one with one parameter and one with two
    uint8_t get_memory(addressing_mode, uint8_t);
    uint8_t get_memory(addressing_mode, uint8_t, uint8_t);

    uint16_t make_address(addressing_mode, uint8_t);
    uint16_t make_address(addressing_mode, uint8_t, uint8_t);

    bool crosses_page(addressing_mode, uint8_t);
    bool crosses_page(addressing_mode, uint8_t, uint8_t);

    // Given an address, execute the opcode at that address
    void execute_opcode(uint16_t);

    // Given an array, copy array into memory
    void load_rom_into_memory(const std::vector<uint8_t>&);

    // Push and pop from stack
    void stack_push(uint8_t);
    void stack_push(uint16_t);
    uint8_t stack_pop();

    // Execute one opcode
    void execute_next_opcode();

    // Tick for a specified number of cycles
    void tick();

    // Reset CPU state
    void reset();

    // Opcode implementations
    void ADC(uint8_t);
    void AND(uint8_t);
    // ASL can operate on the accumulator or a value in memory, there are different versions for each use case
    void ASL(); 
    void ASL(uint16_t);
    void BCC(uint8_t);
    void BCS(uint8_t);
    void BEQ(uint8_t);
    void BIT(uint8_t);
    void BMI(uint8_t);
    void BNE(uint8_t);
    void BPL(uint8_t);
    void BRK();
    void BVC(uint8_t);
    void BVS(uint8_t);
    void CLC();
    void CLD();
    void CLI();
    void CLV();
    void CMP(uint8_t);
    void CPX(uint8_t);
    void CPY(uint8_t);
    void DEC(uint16_t);
    void DEX();
    void DEY();
    void EOR(uint8_t);
    void INC(uint16_t);
    void INX();
    void INY();
    void JMP(uint16_t);
    void JSR(uint16_t);
    void LDA(uint8_t);
    void LDX(uint8_t);
    void LDY(uint8_t);
    void LSR();
    void LSR(uint16_t);
    void NOP();
    void ORA(uint8_t);
    void PHA();
    void PHP();
    void PLA();
    void PLP();
    void ROL();
    void ROL(uint16_t);
    void ROR();
    void ROR(uint16_t);
    void RTI();
    void RTS();
    void SBC(uint8_t);
    void SEC();
    void SED();
    void SEI();
    void STA(uint16_t);
    void STX(uint16_t);
    void STY(uint16_t);
    void TAX();
    void TAY();
    void TSX();
    void TXA();
    void TXS();
    void TYA();


    bool get_flag(flag_type);
};

#include "Bus.h"
