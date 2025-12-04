//
// Created by johan on 2025-10-02.
//

#ifndef INC_6502_EMULATOR_CPU_H
#define INC_6502_EMULATOR_CPU_H

#include <stdint.h>

#define FLAG_C (1 << 0)
#define FLAG_Z (1 << 1)
#define FLAG_I (1 << 2)
#define FLAG_D (1 << 3)
#define FLAG_B (1 << 4)
#define FLAG_U (1 << 5)
#define FLAG_V (1 << 6)
#define FLAG_N (1 << 7)

#define CPU_ZERO_PAGE 0x0000
#define CPU_STACK_PAGE 0x0100
#define CPU_STACK_PTR_START 0x00FD
#define CPU_NMI_LO 0xFFFA
#define CPU_NMI_HI 0xFFFB
#define CPU_RESET_LO 0xFFFC
#define CPU_RESET_HI 0xFFFD
#define CPU_IRQ_LO 0xFFFE
#define CPU_IRQ_HI 0xFFFF

// =========================================================
// Type declarations
// =========================================================
typedef uint8_t (*opcode_fn)(void);
typedef uint8_t (*addressing_fn)(void);

typedef struct mos6502 {
    uint8_t a;
    uint8_t x;
    uint8_t y;
    uint8_t sp;
    uint8_t status;
    uint16_t pc;
    uint16_t addr_abs;
    uint16_t addr_rel;
    uint8_t curr_opcode;
    uint8_t cycles;
} CPU;

typedef struct Instruction {
    char *name;
    opcode_fn opcode;
    addressing_fn addressing;
    uint8_t cycles;
} Instruction;

const CPU *CPU_get_state(void);
uint16_t CPU_get_pc(void);
void CPU_load_instructions(void);
void CPU_reset(void);
void CPU_irq(void);
void CPU_nmi(void);
uint8_t CPU_read(uint16_t addr);
void CPU_write(uint16_t addr, uint8_t data);
Instruction *CPU_get_instruction(uint8_t opcode);

// Tick one cycle
void CPU_tick(void);
// Tick to next instruction (e.g cycles==0)
void CPU_step(void);

// Opcodes
uint8_t LDA(void);
uint8_t LDX(void);
uint8_t LDY(void);
uint8_t STA(void);
uint8_t STX(void);
uint8_t STY(void);
uint8_t TAX(void);
uint8_t TAY(void);
uint8_t TXA(void);
uint8_t TYA(void);
uint8_t NOP(void);
uint8_t ILL(void);
uint8_t SEC(void);
uint8_t CLC(void);
uint8_t SEI(void);
uint8_t CLI(void);
uint8_t INC(void);
uint8_t INX(void);
uint8_t INY(void);
uint8_t BNE(void);
uint8_t BCS(void);
uint8_t BCC(void);
uint8_t BEQ(void);
uint8_t BMI(void);
uint8_t BPL(void);
uint8_t BIT(void);
uint8_t CMP(void);
uint8_t CPX(void);
uint8_t CPY(void);
uint8_t JMP(void);
uint8_t PHA(void);
uint8_t PLA(void);
uint8_t JSR(void);
uint8_t RTS(void);
uint8_t RTI(void);
uint8_t ADC(void);
uint8_t SBC(void);
uint8_t AND(void);
uint8_t ASL(void);
uint8_t BRK(void);


// Addressing modes
uint8_t IMM(void);
uint8_t ABS(void);
uint8_t ABX(void);
uint8_t ABY(void);
uint8_t IMP(void);
uint8_t ZP0(void);
uint8_t ZPX(void);
uint8_t ZPY(void);
uint8_t REL(void);


#endif //INC_6502_EMULATOR_CPU_H