//
// Created by johan on 2025-10-02.
//

#include "cpu.h"
#include <stdio.h>
#include "bus.h"
#include "dbg.h"
#include "disassembler.h"

#define N_INSTRUCTIONS 256

#define FLAG_C (1 << 0)
#define FLAG_Z (1 << 1)
#define FLAG_I (1 << 2)
#define FLAG_D (1 << 3)
#define FLAG_B (1 << 4)
#define FLAG_U (1 << 5)
#define FLAG_V (1 << 6)
#define FLAG_N (1 << 7)

#define BIT_7 (1<<7)

// =========================================================
// Type definitions
// =========================================================
static CPU cpu;
static Instruction instructions[N_INSTRUCTIONS];

static void set_flag(const uint8_t flag, const bool b) {
    if (b) {
        cpu.status |= flag;
    } else {
        cpu.status &= ~flag;
    }
}

static bool get_flag(const uint8_t flag) {
    return cpu.status & flag;
}

void CPU_load_instructions() {
    // Set instructions (fill the ones we have not yet defined as NOP)
    for (int i = 0; i < N_INSTRUCTIONS; i++) {
        instructions[i] = (Instruction){.name = "NOP", .addressing = IMP, .opcode = NOP, .cycles = 2};
    }

    // Set our defined ones
    instructions[0xA9] = (Instruction){.name = "LDA", .addressing = IMM, .opcode = LDA, .cycles = 2};
    instructions[0xAD] = (Instruction){.name = "LDA", .addressing = ABS, .opcode = LDA, .cycles = 4};
    instructions[0xBD] = (Instruction){.name = "LDA", .addressing = ABX, .opcode = LDA, .cycles = 4};
    instructions[0xB9] = (Instruction){.name = "LDA", .addressing = ABY, .opcode = LDA, .cycles = 4};
    instructions[0xA5] = (Instruction){.name = "LDA", .addressing = ZP0, .opcode = LDA, .cycles = 3};
    instructions[0xB5] = (Instruction){.name = "LDA", .addressing = ZPX, .opcode = LDA, .cycles = 4};
    instructions[0xA2] = (Instruction){.name = "LDX", .addressing = IMM, .opcode = LDX, .cycles = 2};
    instructions[0xAE] = (Instruction){.name = "LDX", .addressing = ABS, .opcode = LDX, .cycles = 4};
    instructions[0xA6] = (Instruction){.name = "LDX", .addressing = ZP0, .opcode = LDX, .cycles = 3};
    instructions[0xA0] = (Instruction){.name = "LDY", .addressing = IMM, .opcode = LDY, .cycles = 2};
    instructions[0xAC] = (Instruction){.name = "LDY", .addressing = ABS, .opcode = LDY, .cycles = 4};
    instructions[0xA4] = (Instruction){.name = "LDY", .addressing = ZP0, .opcode = LDY, .cycles = 3};
    instructions[0x8A] = (Instruction){.name = "TXA", .addressing = IMP, .opcode = TXA, .cycles = 2};
    instructions[0x98] = (Instruction){.name = "TYA", .addressing = IMP, .opcode = TYA, .cycles = 2};
    instructions[0xAA] = (Instruction){.name = "TAX", .addressing = IMP, .opcode = TAX, .cycles = 2};
    instructions[0xA8] = (Instruction){.name = "TAY", .addressing = IMP, .opcode = TAY, .cycles = 2};
    instructions[0x8D] = (Instruction){.name = "STA", .addressing = ABS, .opcode = STA, .cycles = 4};
    instructions[0x9D] = (Instruction){.name = "STA", .addressing = ABX, .opcode = STA, .cycles = 5};
    instructions[0x99] = (Instruction){.name = "STA", .addressing = ABY, .opcode = STA, .cycles = 5};
    instructions[0x85] = (Instruction){.name = "STA", .addressing = ZP0, .opcode = STA, .cycles = 3};
    instructions[0x95] = (Instruction){.name = "STA", .addressing = ZPX, .opcode = STA, .cycles = 4};
    instructions[0x86] = (Instruction){.name = "STX", .addressing = ZP0, .opcode = STX, .cycles = 3};
    instructions[0x96] = (Instruction){.name = "STX", .addressing = ZPY, .opcode = STX, .cycles = 4};
    instructions[0x8E] = (Instruction){.name = "STX", .addressing = ABS, .opcode = STX, .cycles = 4};
    instructions[0x8C] = (Instruction){.name = "STY", .addressing = ABS, .opcode = STY, .cycles = 4};
    instructions[0x84] = (Instruction){.name = "STY", .addressing = ZP0, .opcode = STY, .cycles = 3};
    instructions[0x94] = (Instruction){.name = "STY", .addressing = ZPX, .opcode = STY, .cycles = 4};
    instructions[0xE6] = (Instruction){.name = "INC", .addressing = ZP0, .opcode = INC, .cycles = 5};
    instructions[0xF6] = (Instruction){.name = "INC", .addressing = ZPX, .opcode = INC, .cycles = 6};
    instructions[0xEE] = (Instruction){.name = "INC", .addressing = ABS, .opcode = INC, .cycles = 6};
    instructions[0xFE] = (Instruction){.name = "INC", .addressing = ABX, .opcode = INC, .cycles = 7};
    instructions[0xE8] = (Instruction){.name = "INX", .addressing = IMP, .opcode = INX, .cycles = 2};
    instructions[0xC8] = (Instruction){.name = "INY", .addressing = IMP, .opcode = INY, .cycles = 2};
    instructions[0x38] = (Instruction){.name = "SEC", .addressing = IMP, .opcode = SEC, .cycles = 2};
    instructions[0x18] = (Instruction){.name = "CLC", .addressing = IMP, .opcode = CLC, .cycles = 2};
    instructions[0xD0] = (Instruction){.name = "BNE", .addressing = REL, .opcode = BNE, .cycles = 2};
    instructions[0xB0] = (Instruction){.name = "BCS", .addressing = REL, .opcode = BCS, .cycles = 2};
    instructions[0x90] = (Instruction){.name = "BCC", .addressing = REL, .opcode = BCC, .cycles = 2};
    instructions[0xC5] = (Instruction){.name = "CMP", .addressing = ZP0, .opcode = CMP, .cycles = 3};
    instructions[0xD5] = (Instruction){.name = "CMP", .addressing = ZPX, .opcode = CMP, .cycles = 4};
    instructions[0xC9] = (Instruction){.name = "CMP", .addressing = IMM, .opcode = CMP, .cycles = 2};
    instructions[0xD9] = (Instruction){.name = "CMP", .addressing = ABY, .opcode = CMP, .cycles = 4};
    instructions[0xCD] = (Instruction){.name = "CMP", .addressing = ABS, .opcode = CMP, .cycles = 4};
    instructions[0xDD] = (Instruction){.name = "CMP", .addressing = ABX, .opcode = CMP, .cycles = 3};
    instructions[0xE0] = (Instruction){.name = "CPX", .addressing = IMM, .opcode = CPX, .cycles = 2};
    instructions[0xE4] = (Instruction){.name = "CPX", .addressing = ZP0, .opcode = CPX, .cycles = 3};
    instructions[0xEC] = (Instruction){.name = "CPX", .addressing = ABS, .opcode = CPX, .cycles = 4};
    instructions[0xC0] = (Instruction){.name = "CPY", .addressing = IMM, .opcode = CPY, .cycles = 2};
    instructions[0xC4] = (Instruction){.name = "CPY", .addressing = ZP0, .opcode = CPY, .cycles = 3};
    instructions[0xCC] = (Instruction){.name = "CPY", .addressing = ABS, .opcode = CPY, .cycles = 4};
    instructions[0x48] = (Instruction){.name = "PHA", .addressing = IMP, .opcode = PHA, .cycles = 3};
    instructions[0x68] = (Instruction){.name = "PLA", .addressing = IMP, .opcode = PLA, .cycles = 3};
    instructions[0x20] = (Instruction){.name = "JSR", .addressing = ABS, .opcode = JSR, .cycles = 6};
    instructions[0x60] = (Instruction){.name = "RTS", .addressing = IMP, .opcode = RTS, .cycles = 2};
    instructions[0x65] = (Instruction){.name = "ADC", .addressing = ZP0, .opcode = ADC, .cycles = 3};
    instructions[0x75] = (Instruction){.name = "ADC", .addressing = ZPX, .opcode = ADC, .cycles = 4};
    instructions[0x69] = (Instruction){.name = "ADC", .addressing = IMM, .opcode = ADC, .cycles = 2};
    instructions[0x79] = (Instruction){.name = "ADC", .addressing = ABY, .opcode = ADC, .cycles = 4};
    instructions[0x6D] = (Instruction){.name = "ADC", .addressing = ABS, .opcode = ADC, .cycles = 4};
    instructions[0x7D] = (Instruction){.name = "ADC", .addressing = ABX, .opcode = ADC, .cycles = 4};

    log_info("Instructions loaded");
}

const CPU *CPU_get_state(void) {
    log_info("CPU get state called!");
    return &cpu;
}

// Emulate cpu start/reset
void CPU_reset() {
    cpu.a = 0;
    cpu.x = 0;
    cpu.y = 0;

    /*
     * Set program counter start addr. This is acquired by reading the 2 bytes at reset vector hi|lo addresses
     */
    const uint16_t reset_lo = CPU_read(CPU_RESET_LO);
    const uint16_t reset_hi = CPU_read(CPU_RESET_HI);
    const uint16_t pc_start = (reset_hi << 8) | reset_lo;
    cpu.pc = pc_start;
    cpu.sp = CPU_STACK_PTR_START;

    // Set interrupt disabled and unused to 1
    cpu.status = 0x00 | FLAG_U;

    cpu.addr_abs = 0x0000;
    cpu.addr_rel = 0x0000;

    // A 6502 reset takes ~8 cycles
    cpu.cycles = 8;

    log_info("CPU started");
}

void CPU_tick() {
    if (cpu.cycles == 0) {
        cpu.curr_opcode = CPU_read(cpu.pc++);

        const Instruction *ins = &instructions[cpu.curr_opcode];
        cpu.cycles = ins->cycles;

        const uint8_t additional_cycle1 = ins->addressing();
        const uint8_t additional_cycle2 = ins->opcode();

        cpu.cycles += (additional_cycle1 & additional_cycle2);
    }
    cpu.cycles--;
}

void CPU_step() {
    while (cpu.cycles > 0) {
        CPU_tick();
    }
    puts(Disassembler_get_line_at(cpu.pc));
    CPU_tick();
}

Instruction *CPU_get_instruction(uint8_t opcode) {
    return &instructions[opcode];
}

/*
 ***********************************************************************
 *                               OPCODES                               *
 ***********************************************************************
 */

uint8_t LDA(void) {
    // Load into accumulator
    cpu.a = CPU_read(cpu.addr_abs);
    set_flag(FLAG_Z, cpu.a == 0);
    set_flag(FLAG_N, cpu.a & BIT_7);
    return 1;
}

uint8_t LDX(void) {
    // Load into x register
    cpu.x = CPU_read(cpu.addr_abs);
    set_flag(FLAG_Z, cpu.x == 0);
    set_flag(FLAG_N, cpu.x & BIT_7);
    return 1;
}

uint8_t LDY(void) {
    // Load into y register
    cpu.y = CPU_read(cpu.addr_abs);
    set_flag(FLAG_Z, cpu.y == 0);
    set_flag(FLAG_N, cpu.y & BIT_7);
    return 1;
}

uint8_t TAX(void) {
    cpu.x = cpu.a;
    set_flag(FLAG_Z, cpu.x == 0);
    set_flag(FLAG_N, cpu.x & BIT_7);
    return 0;
}

uint8_t TAY(void) {
    cpu.y = cpu.a;
    set_flag(FLAG_Z, cpu.y == 0);
    set_flag(FLAG_N, cpu.y & BIT_7);
    return 0;
}

uint8_t TXA(void) {
    cpu.a = cpu.x;
    set_flag(FLAG_Z, cpu.a == 0);
    set_flag(FLAG_N, cpu.a & BIT_7);
    return 0;
}

uint8_t TYA(void) {
    cpu.a = cpu.y;
    set_flag(FLAG_Z, cpu.a == 0);
    set_flag(FLAG_N, cpu.a & BIT_7);
    return 0;
}

uint8_t STA(void) {
    // Store accumulator
    CPU_write(cpu.addr_abs, cpu.a);
    return 0;
}

uint8_t STX(void) {
    CPU_write(cpu.addr_abs, cpu.x);
    return 0;
}

uint8_t STY(void) {
    CPU_write(cpu.addr_abs, cpu.y);
    return 0;
}

uint8_t INC(void) {
    const uint8_t data = CPU_read(cpu.addr_abs);
    CPU_write(cpu.addr_abs, data + 1);
    set_flag(FLAG_Z, data == 0);
    set_flag(FLAG_N, data & BIT_7);
    return 0;
}

uint8_t INX(void) {
    cpu.x++;
    set_flag(FLAG_Z, cpu.x == 0);
    set_flag(FLAG_N, cpu.x & BIT_7);
    return 0;
}

uint8_t INY(void) {
    cpu.y++;
    set_flag(FLAG_Z, cpu.y == 0);
    set_flag(FLAG_N, cpu.y & BIT_7);
    return 0;
}

uint8_t SEC(void) {
    set_flag(FLAG_C, true);
    return 0;
}

uint8_t CLC(void) {
    set_flag(FLAG_C, false);
    return 0;
}

static inline void branch_on_condition(bool condition) {
    if (condition) {
        cpu.cycles++;
        cpu.addr_abs = cpu.pc + cpu.addr_rel;

        // Add additional cycle if we crossed page
        if ((cpu.addr_abs & 0xFF00) != (cpu.pc & 0xFF00)) {
            cpu.cycles++;
        }

        cpu.pc = cpu.addr_abs;
    }
}

uint8_t BNE(void) {
    branch_on_condition(get_flag(FLAG_Z) == 0);
    return 0;
}

uint8_t BCS(void) {
    branch_on_condition(get_flag(FLAG_C) == 1);
    return 0;
}

uint8_t BCC(void) {
    branch_on_condition(get_flag(FLAG_C) == 0);
    return 0;
}

uint8_t CMP(void) {
    const uint16_t data = CPU_read(cpu.addr_abs);
    const uint16_t result = (uint16_t) cpu.a - data;
    set_flag(FLAG_C, cpu.a >= data);
    set_flag(FLAG_Z, (result & 0x00FF) == 0);
    set_flag(FLAG_N, result & BIT_7);
    return 0;
}

uint8_t CPX(void) {
    const uint16_t data = CPU_read(cpu.addr_abs);
    const uint16_t result = (uint16_t) cpu.x - data;
    set_flag(FLAG_C, cpu.x >= data);
    set_flag(FLAG_Z, (result & 0x00FF) == 0);
    set_flag(FLAG_N, result & BIT_7);
    return 0;
}

uint8_t CPY(void) {
    const uint16_t data = CPU_read(cpu.addr_abs);
    const uint16_t result = (uint16_t) cpu.y - data;
    set_flag(FLAG_C, cpu.y >= data);
    set_flag(FLAG_Z, (result & 0x00FF) == 0);
    set_flag(FLAG_N, result & BIT_7);
    return 0;
}

uint8_t JMP(void) {
    cpu.pc = cpu.addr_abs;
    return 0;
}

uint8_t PHA(void) {
    CPU_write(CPU_STACK_PAGE + cpu.sp, cpu.a);
    cpu.sp--;
    return 0;
}

uint8_t PLA(void) {
    cpu.sp++;
    cpu.a = CPU_read(CPU_STACK_PAGE + cpu.sp);
    set_flag(FLAG_Z, cpu.a == 0);
    set_flag(FLAG_N, cpu.a & BIT_7);
    return 0;
}

uint8_t ADC(void) {
    const uint16_t data = CPU_read(cpu.addr_abs);
    const uint16_t a = cpu.a;
    const uint16_t carry = get_flag(FLAG_C);

    // Store result as 16 bit since we need the carry flag
    const uint16_t result = a + data + carry;

    // set carry, zero and negative flags
    set_flag(FLAG_C, result > 0xFF);
    set_flag(FLAG_Z, (result & 0x00FF) == 0);
    set_flag(FLAG_N, result & 0x80);

    // check if we overflowed
    const bool v = (~(a ^data) & (a ^ result)) & 0x0080;
    set_flag(FLAG_V, v);

    // Convert back to 8-bit and add to accumulator
    cpu.a = result & 0x00FF;

    // May require an additional cycle
    return 1;
}

/**
 * JSR pushes the address-1 of the next operation on to the stack before transferring program control to the
 * following address. Subroutines are normally terminated by a RTS op code.
 * @return 0
 */
uint8_t JSR(void) {
    cpu.pc--;

    // Write lo and hi byte of pc to stack
    CPU_write(CPU_STACK_PAGE + cpu.sp--, (cpu.pc >> 8) & 0x00FF);
    CPU_write(CPU_STACK_PAGE + cpu.sp--, cpu.pc & 0x00FF);

    // set pc to new address
    cpu.pc = cpu.addr_abs;
    return 0;
}

uint8_t RTS(void) {
    cpu.sp++;
    const uint16_t pc_lo = CPU_read(CPU_STACK_PAGE + cpu.sp);
    cpu.sp++;
    const uint16_t pc_hi = CPU_read(CPU_STACK_PAGE + cpu.sp);
    cpu.pc = (pc_hi << 8) | (pc_lo & 0x00FF);
    cpu.pc++;
    return 0;
}

uint8_t NOP(void) {
    return 0;
}

uint8_t ILL(void) {
    return 0;
}



// ==============================================
// Addressing modes
// ==============================================


uint8_t IMM(void) {
    // Immediate mode, set absolute address to pc and inc pc
    // Requires no additional cycles
    cpu.addr_abs = cpu.pc++;
    return 0;
}

uint8_t ABS(void) {
    // Absolute addressing mode, read the lo and hi byte from pc and or together to 16 bit word, no additional cycle
    const uint16_t lo = CPU_read(cpu.pc++);
    const uint16_t hi = CPU_read(cpu.pc++);
    cpu.addr_abs = (hi << 8) | lo;
    return 0;
}

uint8_t ABX(void) {
    const uint16_t lo = CPU_read(cpu.pc++);
    const uint16_t hi = CPU_read(cpu.pc++);
    cpu.addr_abs = ((hi << 8) | lo) + cpu.x;

    // If we crossed page boundary we need to return an additional clock cycle
    if ((cpu.addr_abs & 0xFF00) != (hi << 8)) {
        return 1;
    }
    return 0;
}

uint8_t ABY(void) {
    const uint16_t lo = CPU_read(cpu.pc++);
    const uint16_t hi = CPU_read(cpu.pc++);
    cpu.addr_abs = ((hi << 8) | lo) + cpu.y;

    // If we crossed page boundary we need to return an additional clock cycle
    if ((cpu.addr_abs & 0xFF00) != (hi << 8)) {
        return 1;
    }
    return 0;
}

uint8_t IMP(void) {
    // There could be stuff going on with the accumulator in implied mode
    cpu.curr_opcode = cpu.a;
    return 0;
}

uint8_t ZP0(void) {
    cpu.addr_abs = CPU_read(cpu.pc++) & 0x00FF;
    return 0;
}

uint8_t ZPX(void) {
    cpu.addr_abs = (CPU_read(cpu.pc++) + cpu.x) & 0x00FF;
    return 0;
}

uint8_t ZPY(void) {
    cpu.addr_abs = (CPU_read(cpu.pc++) + cpu.y) & 0x00FF;
    return 0;
}

uint8_t REL(void) {
    cpu.addr_rel = CPU_read(cpu.pc++);

    if (cpu.addr_rel & BIT_7) {
        cpu.addr_rel |= 0xFF00;
    }
    return 0;
}


uint8_t CPU_read(const uint16_t addr) {
    return Bus_read(addr);
}

void CPU_write(const uint16_t addr, const uint8_t data) {
    Bus_write(addr, data);
}



