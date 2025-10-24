//
// Created by johan on 2025-10-02.
//

#include "cpu.h"

#include <stdio.h>
#include <stdlib.h>

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

// =========================================================
// Type definitions
// =========================================================
static CPU cpu;
static Instruction instruction[N_INSTRUCTIONS];
static bool is_new_instruction = false;

void CPU_load_instructions() {
    // Set instructions (fill the ones we have not yet defined as illegal)
    for (int i = 0; i < N_INSTRUCTIONS; i++) {
        instruction[i] = (Instruction){.name = "NOP", .addressing = IMP, .opcode = NOP, .cycles = 2};
    }

    // Set our defined ones
    instruction[0xA9] = (Instruction){.name = "LDA", .addressing = IMM, .opcode = LDA, .cycles = 2};
    instruction[0x8D] = (Instruction){.name = "STA", .addressing = ABS, .opcode = STA, .cycles = 4};

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
    const uint16_t reset_lo = Bus_read(CPU_RESET_LO);
    const uint16_t reset_hi = Bus_read(CPU_RESET_HI);
    const uint16_t pc_start = (reset_hi << 8) | reset_lo;
    cpu.pc = pc_start;
    cpu.sp = CPU_STACK_START;

    // Set interrupt disabled and unused to 1
    cpu.status = 0x00 | FLAG_U;

    cpu.addr_abs = 0x0000;
    cpu.addr_rel = 0x0000;
    cpu.data_fetched = 0x00;

    // A 6502 reset takes ~8 cycles
    cpu.cycles = 8;

    log_info("CPU started");
}

// Opcodes
uint8_t LDA(void) {
    // Load into accumulator
    cpu.a = CPU_read(cpu.addr_abs);
    return 1;
}

uint8_t STA(void) {
    // Store accumulator
    CPU_write(cpu.addr_abs, cpu.a);
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

uint8_t IMP(void) {
    // cpu.pc++;
    return 0;
}

uint8_t CPU_read(const uint16_t addr) {
    return Bus_read(addr);
}

void CPU_write(const uint16_t addr, const uint8_t data) {
    Bus_write(addr, data);
}

void CPU_tick() {
    is_new_instruction = cpu.cycles == 0;
    if (is_new_instruction) {
        cpu.curr_opcode = CPU_read(cpu.pc);
        cpu.pc++;

        const Instruction *ins = &instruction[cpu.curr_opcode];
        cpu.cycles = ins->cycles;

        const uint8_t additional_cycle1 = ins->addressing();
        const uint8_t additional_cycle2 = ins->opcode();

        // log_debug("pc=%04x\tinstr=%s", cpu.pc, ins->name);

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
    return &instruction[opcode];
}

bool CPU_is_time_for_new_instruction() {
    return is_new_instruction;
}

