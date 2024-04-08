
#pragma once
#include <cstdint>
#include <vector>

/*
    See:
    https://www.nesdev.org/obelisk-6502-guide/registers.html
    https://www.nesdev.org/obelisk-6502-guide/architecture.html
*/

enum flag_type {CARRY, ZERO, INT_DISABLE, DECIMAL, BREAK, OVER_FLOW, NEGATIVE};
enum addressing_mode {IMPLICIT, ACCUMULATOR, IMMEDIATE, ZERO_PAGE, ZERO_PAGE_X, ZERO_PAGE_Y, RELATIVE, ABSOLUTE, ABSOLUTE_X, ABSOLUTE_Y, INDIRECT, INDEXED_INDIRECT, INDIRECT_INDEXED};

class CPU {

    private:

    const uint16_t PROGRAM_MEMORY_START = 0x600;
    const uint16_t STACK_LOCATION = 0x1FF;
    
    // The stack pointer stores the low 8 bytes of the next memory location available on the stack
    uint8_t stack_pointer = STACK_LOCATION & 0xFF;
    uint16_t program_counter = PROGRAM_MEMORY_START;
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

    // Fixed size memory based on NES architecture specs
    // Store in units of kilobytes
    uint8_t RAM[0xFFFF];

    void increment_program_counter(int);

    // Methods to control flags
    void set_flag(flag_type, bool);
    void toggle_flag(flag_type);
    uint8_t get_byte_from_flags() const;

    public:

    // Given an addressing mode and parameters, return the value stored in memory
    // Some addressing modes only require one parameter, so there are 2 versions of the method:
    // one with one parameter and one with two
    uint8_t get_memory(addressing_mode, uint8_t);
    uint8_t get_memory(addressing_mode, uint8_t, uint8_t);

    // Given an address, execute the opcode at that address
    void execute_opcode(uint16_t);

    // Given an array, copy array into memory
    void load_rom_into_memory(const std::vector<uint8_t>&);

    // Execute one opcode
    void tick();

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
    void ROL(uint16_t);
    void ROR();
    void ROR(uint8_t);
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

    // Getters
    uint8_t get_a() const;
    uint8_t get_x() const;
    uint8_t get_y() const;
    uint16_t get_program_counter() const;
    bool get_flag(flag_type);

    // Setters
    void set_a(uint8_t);
};