//
// Created by johan on 2025-10-02.
//

#ifndef INC_6502_EMULATOR_CPU_H
#define INC_6502_EMULATOR_CPU_H

#include <stdint.h>

#define CPU_ZERO_PAGE 0x0000
#define CPU_STACK_PAGE 0x0100
#define CPU_STACK_START 0xFD
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

    uint16_t pc;
    uint8_t sp;
    uint8_t status;
    uint8_t data_fetched;
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

// Opcodes
uint8_t LDA(void);
uint8_t STA(void);
uint8_t ILL(void);

// Addressing modes
uint8_t IMM(void);
uint8_t ABS(void);
uint8_t IMP(void);

const CPU *CPU_get_state();
void CPU_init();
void CPU_reset();
uint8_t CPU_read(uint16_t addr);
void CPU_write(uint16_t addr, uint8_t data);
void CPU_tick();

#endif //INC_6502_EMULATOR_CPU_H