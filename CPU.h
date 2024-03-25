
#pragma once
#include <cstdint>

/*
    See:
    https://www.nesdev.org/obelisk-6502-guide/registers.html
    https://www.nesdev.org/obelisk-6502-guide/architecture.html
*/

enum flag_type {CARRY, ZERO, INT_DISABLE, DECIMAL, BREAK, OVER_FLOW, NEGATIVE};
enum addressing_mode {IMPLICIT, ACCUMULATOR, IMMEDIATE, ZERO_PAGE, ZERO_PAGE_X, ZERO_PAGE_Y, RELATIVE, ABSOLUTE, ABSOLUTE_X, ABSOLUTE_Y, INDIRECT, INDEXED_INDIRECT, INDIRECT_INDEXED};

class CPU {

    private:

    int stack_pointer = 0;
    int program_counter = 0;
    int accumulator = 0;
    uint8_t A = 0; // Accumulator register A
    uint8_t X = 0; // Index register X
    uint8_t Y = 0; // Index register Y
    // Array to store flags of processor
    // Index 0: Carry flag
    // Index 1: Zero flag
    // Index 2: Interrupt disable
    // Index 3: Decimal Mode
    // Index 4: Break command
    // Index 5: Overflow flag
    // Index 6: Negative flag
    bool flags[7] = {0, 0, 0, 0, 0, 0, 0};
    bool RAM[0x10000]; // Fixed size memory based on NES architecture specs

    void increment_program_counter();

    // Methods to control flags
    void set_flag(flag_type, bool);
    void toggle_flag(flag_type);

    public:

    // Given an addressing mode and parameters, return the value stored in memory
    uint8_t get_memory(addressing_mode, uint8_t) const;
    uint8_t get_memory(addressing_mode, uint8_t, uint8_t) const;

    void execute_opcode(uint8_t opcode);

    // Opcode implementations
    void ADC(uint8_t);
    void AND(uint8_t);
    void ASL(uint8_t);
    void BCC();
    void BCS();
    void BEQ();
    void BIT(uint8_t);
    void BMI();
    void BNE();
    void BPL();
    void BRK();
    void BVC();
    void BVS();
    void CLC();
    void CLD();
    void CLI();
    void CLV();
    void CMP(uint8_t);
    void CPX(uint8_t);
    void CPY(uint8_t);
    void DEC(uint8_t);
    void DEX();
    void DEY();
    void EOR(uint8_t);
    void INC(uint8_t);
    void INX();
    void INY();
    void JMP(uint16_t);
    void JSR(uint16_t);
    void LDA(uint8_t);
    void LDX(uint8_t);
    void LDY(uint8_t);
    void LSR();
    void LSR(uint8_t);
    void NOP();
    void ORA(uint8_t);
    void PHA();
    void PHP();
    void PLA();
    void PLP();
    void ROL();
    void ROL(uint8_t);
    void ROR();
    void ROR(uint8_t);
    void RTI();
    void RTS();
    void SBC(uint8_t);
    void SEC();
    void SED();
    void SEI();
    void STA(uint8_t);
    void STX(uint8_t);
    void STY(uint8_t);
    void TAX();
    void TAY();
    void TSX();
    void TXA();
    void TXS();
    void TYA();

    // Getters
    uint8_t get_a() const;
    uint8_t get_x() const;
    bool get_flag(flag_type);

    // Setters
    void set_a(uint8_t);
};