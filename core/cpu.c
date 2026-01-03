
#include "cpu.h"

#include <stdbool.h>
#include <stdio.h>
#include "bus.h"
#include "dbg.h"
#include "disassembler.h"

#define N_INSTRUCTIONS 256

// =========================================================
// Type definitions
// =========================================================
static CPU cpu;
static Instruction instructions[N_INSTRUCTIONS];


// =========================================================
// Private functions
// =========================================================
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

static void compare_register(const uint8_t reg) {
    const uint16_t data = CPU_read(cpu.addr_abs);
    const uint16_t result = (uint16_t) reg - data;
    set_flag(FLAG_C, reg >= data);
    set_flag(FLAG_Z, (result & 0x00FF) == 0);
    set_flag(FLAG_N, result & 0x80);
}

static void branch_on_condition(bool condition) {
    if (condition) {
        cpu.cycles++;
        cpu.addr_abs = cpu.pc + cpu.addr_rel;

        // Add a cycle if we crossed a page
        if ((cpu.addr_abs & 0xFF00) != (cpu.pc & 0xFF00)) {
            cpu.cycles++;
        }

        cpu.pc = cpu.addr_abs;
    }
}

static void hardware_interrupt(const uint16_t pc_lo, const uint16_t pc_hi) {
    // Write hi and lo byte to stack (remember little-endian so reversed since we decrement sp)
    CPU_write(CPU_STACK_PAGE + cpu.sp--, cpu.pc >> 8);
    CPU_write(CPU_STACK_PAGE + cpu.sp--, cpu.pc & 0x00FF);

    // Set B and U flag before pushing to stack
    set_flag(FLAG_B, false);
    set_flag(FLAG_U, true);

    // Push status register to the stack
    CPU_write(CPU_STACK_PAGE + cpu.sp--, cpu.status);

    // Set I flag to true after copy (will be restored to 0 in RTI)
    set_flag(FLAG_I, true);

    // Set pc to irq address (irq or nmi)
    cpu.pc = (CPU_read(pc_hi) << 8) | (CPU_read(pc_lo) & 0x00FF);

    // Interrupts takes ~7 cycles
    cpu.cycles = 7;
}

/**
 * Used by both ADC and SBC because we can re-use all the logic except for the data in.
 * For ADC, data = the next byte read.
 * For SBC, we read the next byte but invert it to make it negative
 * @param data data from the bus (negated when used with SBC)
 */
static void set_add_or_sub_result(const uint16_t data) {
    const uint16_t a = cpu.a;
    const uint16_t carry = get_flag(FLAG_C);

    // Store result as 16 bit since we need the carry flag
    const uint16_t result = a + data + carry;

    // set carry, zero and negative flags
    set_flag(FLAG_C, result > 0xFF);
    set_flag(FLAG_Z, (result & 0x00FF) == 0);
    set_flag(FLAG_N, result & 0x80);

    // check if we overflowed
    const bool v = (~(a ^ data) & (a ^ result)) & 0x0080;
    set_flag(FLAG_V, v);

    // Convert back to 8-bit and add to accumulator
    cpu.a = result & 0x00FF;
}

static bool is_implied_addressing() {
    return CPU_get_instruction(cpu.curr_opcode)->addressing == IMP;
}

// =========================================================
// Public functions
// =========================================================
void CPU_load_instructions(void) {
    // Start by filling the whole array with ILL opcodes then populate the legal ones
    for (int i = 0; i <= 0xFF; i++) {
        instructions[i] = (Instruction){.name = "???", .addressing = IMP, .opcode = ILL, .cycles = 2};
    }

    // Set our defined ones
    instructions[0x69] = (Instruction){.name = "ADC", .addressing = IMM, .opcode = ADC, .cycles = 2};
    instructions[0x65] = (Instruction){.name = "ADC", .addressing = ZP0, .opcode = ADC, .cycles = 3};
    instructions[0x75] = (Instruction){.name = "ADC", .addressing = ZPX, .opcode = ADC, .cycles = 4};
    instructions[0x6D] = (Instruction){.name = "ADC", .addressing = ABS, .opcode = ADC, .cycles = 4};
    instructions[0x7D] = (Instruction){.name = "ADC", .addressing = ABX, .opcode = ADC, .cycles = 4};
    instructions[0x79] = (Instruction){.name = "ADC", .addressing = ABY, .opcode = ADC, .cycles = 4};
    instructions[0x61] = (Instruction){.name = "ADC", .addressing = IZX, .opcode = ADC, .cycles = 6};
    instructions[0x71] = (Instruction){.name = "ADC", .addressing = IZY, .opcode = ADC, .cycles = 5};
    instructions[0x29] = (Instruction){.name = "AND", .addressing = IMM, .opcode = AND, .cycles = 2};
    instructions[0x25] = (Instruction){.name = "AND", .addressing = ZP0, .opcode = AND, .cycles = 3};
    instructions[0x35] = (Instruction){.name = "AND", .addressing = ZPX, .opcode = AND, .cycles = 4};
    instructions[0x2D] = (Instruction){.name = "AND", .addressing = ABS, .opcode = AND, .cycles = 4};
    instructions[0x3D] = (Instruction){.name = "AND", .addressing = ABX, .opcode = AND, .cycles = 4};
    instructions[0x39] = (Instruction){.name = "AND", .addressing = ABY, .opcode = AND, .cycles = 4};
    instructions[0x21] = (Instruction){.name = "AND", .addressing = IZX, .opcode = AND, .cycles = 6};
    instructions[0x31] = (Instruction){.name = "AND", .addressing = IZY, .opcode = AND, .cycles = 5};
    instructions[0x0A] = (Instruction){.name = "ASL", .addressing = IMP, .opcode = ASL, .cycles = 2};
    instructions[0x06] = (Instruction){.name = "ASL", .addressing = ZP0, .opcode = ASL, .cycles = 5};
    instructions[0x16] = (Instruction){.name = "ASL", .addressing = ZPX, .opcode = ASL, .cycles = 6};
    instructions[0x0E] = (Instruction){.name = "ASL", .addressing = ABS, .opcode = ASL, .cycles = 6};
    instructions[0x1E] = (Instruction){.name = "ASL", .addressing = ABX, .opcode = ASL, .cycles = 7};
    instructions[0x90] = (Instruction){.name = "BCC", .addressing = REL, .opcode = BCC, .cycles = 2};
    instructions[0xB0] = (Instruction){.name = "BCS", .addressing = REL, .opcode = BCS, .cycles = 2};
    instructions[0xF0] = (Instruction){.name = "BEQ", .addressing = REL, .opcode = BEQ, .cycles = 2};
    instructions[0x24] = (Instruction){.name = "BIT", .addressing = ZP0, .opcode = BIT, .cycles = 3};
    instructions[0x2C] = (Instruction){.name = "BIT", .addressing = ABS, .opcode = BIT, .cycles = 4};
    instructions[0x30] = (Instruction){.name = "BMI", .addressing = REL, .opcode = BMI, .cycles = 2};
    instructions[0xD0] = (Instruction){.name = "BNE", .addressing = REL, .opcode = BNE, .cycles = 2};
    instructions[0x10] = (Instruction){.name = "BPL", .addressing = REL, .opcode = BPL, .cycles = 2};
    instructions[0x00] = (Instruction){.name = "BRK", .addressing = IMP, .opcode = BRK, .cycles = 7};
    instructions[0x50] = (Instruction){.name = "BVC", .addressing = IMP, .opcode = BVC, .cycles = 2};
    instructions[0x70] = (Instruction){.name = "BVS", .addressing = IMP, .opcode = BVS, .cycles = 2};
    instructions[0x18] = (Instruction){.name = "CLC", .addressing = IMP, .opcode = CLC, .cycles = 2};
    instructions[0xD8] = (Instruction){.name = "CLD", .addressing = IMP, .opcode = CLD, .cycles = 2};
    instructions[0x58] = (Instruction){.name = "CLI", .addressing = IMP, .opcode = CLI, .cycles = 2};
    instructions[0xB8] = (Instruction){.name = "CLV", .addressing = IMP, .opcode = CLV, .cycles = 2};
    instructions[0xC9] = (Instruction){.name = "CMP", .addressing = IMM, .opcode = CMP, .cycles = 2};
    instructions[0xC5] = (Instruction){.name = "CMP", .addressing = ZP0, .opcode = CMP, .cycles = 3};
    instructions[0xD5] = (Instruction){.name = "CMP", .addressing = ZPX, .opcode = CMP, .cycles = 4};
    instructions[0xCD] = (Instruction){.name = "CMP", .addressing = ABS, .opcode = CMP, .cycles = 4};
    instructions[0xDD] = (Instruction){.name = "CMP", .addressing = ABX, .opcode = CMP, .cycles = 4};
    instructions[0xD9] = (Instruction){.name = "CMP", .addressing = ABY, .opcode = CMP, .cycles = 4};
    instructions[0xC1] = (Instruction){.name = "CMP", .addressing = IZX, .opcode = CMP, .cycles = 6};
    instructions[0xD1] = (Instruction){.name = "CMP", .addressing = IZY, .opcode = CMP, .cycles = 5};
    instructions[0xE0] = (Instruction){.name = "CPX", .addressing = IMM, .opcode = CPX, .cycles = 2};
    instructions[0xE4] = (Instruction){.name = "CPX", .addressing = ZP0, .opcode = CPX, .cycles = 3};
    instructions[0xEC] = (Instruction){.name = "CPX", .addressing = ABS, .opcode = CPX, .cycles = 4};
    instructions[0xC0] = (Instruction){.name = "CPY", .addressing = IMM, .opcode = CPY, .cycles = 2};
    instructions[0xC4] = (Instruction){.name = "CPY", .addressing = ZP0, .opcode = CPY, .cycles = 3};
    instructions[0xCC] = (Instruction){.name = "CPY", .addressing = ABS, .opcode = CPY, .cycles = 4};
    instructions[0xC6] = (Instruction){.name = "DEC", .addressing = ZP0, .opcode = DEC, .cycles = 5};
    instructions[0xD6] = (Instruction){.name = "DEC", .addressing = ZPX, .opcode = DEC, .cycles = 6};
    instructions[0xCE] = (Instruction){.name = "DEC", .addressing = ABS, .opcode = DEC, .cycles = 6};
    instructions[0xDE] = (Instruction){.name = "DEC", .addressing = ABX, .opcode = DEC, .cycles = 7};
    instructions[0xCA] = (Instruction){.name = "DEX", .addressing = IMP, .opcode = DEX, .cycles = 2};
    instructions[0x88] = (Instruction){.name = "DEY", .addressing = IMP, .opcode = DEY, .cycles = 2};
    instructions[0x49] = (Instruction){.name = "EOR", .addressing = IMM, .opcode = EOR, .cycles = 2};
    instructions[0x45] = (Instruction){.name = "EOR", .addressing = ZP0, .opcode = EOR, .cycles = 3};
    instructions[0x55] = (Instruction){.name = "EOR", .addressing = ZPX, .opcode = EOR, .cycles = 4};
    instructions[0x4D] = (Instruction){.name = "EOR", .addressing = ABS, .opcode = EOR, .cycles = 4};
    instructions[0x5D] = (Instruction){.name = "EOR", .addressing = ABX, .opcode = EOR, .cycles = 4};
    instructions[0x59] = (Instruction){.name = "EOR", .addressing = ABY, .opcode = EOR, .cycles = 4};
    instructions[0x41] = (Instruction){.name = "EOR", .addressing = IZX, .opcode = EOR, .cycles = 6};
    instructions[0x51] = (Instruction){.name = "EOR", .addressing = IZY, .opcode = EOR, .cycles = 5};
    instructions[0xE6] = (Instruction){.name = "INC", .addressing = ZP0, .opcode = INC, .cycles = 5};
    instructions[0xF6] = (Instruction){.name = "INC", .addressing = ZPX, .opcode = INC, .cycles = 6};
    instructions[0xEE] = (Instruction){.name = "INC", .addressing = ABS, .opcode = INC, .cycles = 6};
    instructions[0xFE] = (Instruction){.name = "INC", .addressing = ABX, .opcode = INC, .cycles = 7};
    instructions[0xE8] = (Instruction){.name = "INX", .addressing = IMP, .opcode = INX, .cycles = 2};
    instructions[0xC8] = (Instruction){.name = "INY", .addressing = IMP, .opcode = INY, .cycles = 2};
    instructions[0x4C] = (Instruction){.name = "JMP", .addressing = ABS, .opcode = JMP, .cycles = 3};
    instructions[0x6C] = (Instruction){.name = "JMP", .addressing = IND, .opcode = JMP, .cycles = 5};
    instructions[0x20] = (Instruction){.name = "JSR", .addressing = ABS, .opcode = JSR, .cycles = 6};
    instructions[0xA9] = (Instruction){.name = "LDA", .addressing = IMM, .opcode = LDA, .cycles = 2};
    instructions[0xA5] = (Instruction){.name = "LDA", .addressing = ZP0, .opcode = LDA, .cycles = 3};
    instructions[0xB5] = (Instruction){.name = "LDA", .addressing = ZPX, .opcode = LDA, .cycles = 4};
    instructions[0xAD] = (Instruction){.name = "LDA", .addressing = ABS, .opcode = LDA, .cycles = 4};
    instructions[0xBD] = (Instruction){.name = "LDA", .addressing = ABX, .opcode = LDA, .cycles = 4};
    instructions[0xB9] = (Instruction){.name = "LDA", .addressing = ABY, .opcode = LDA, .cycles = 4};
    instructions[0xA1] = (Instruction){.name = "LDA", .addressing = IZX, .opcode = LDA, .cycles = 6};
    instructions[0xB1] = (Instruction){.name = "LDA", .addressing = IZY, .opcode = LDA, .cycles = 5};
    instructions[0xA2] = (Instruction){.name = "LDX", .addressing = IMM, .opcode = LDX, .cycles = 2};
    instructions[0xA6] = (Instruction){.name = "LDX", .addressing = ZP0, .opcode = LDX, .cycles = 3};
    instructions[0xB6] = (Instruction){.name = "LDX", .addressing = ZPY, .opcode = LDX, .cycles = 4};
    instructions[0xAE] = (Instruction){.name = "LDX", .addressing = ABS, .opcode = LDX, .cycles = 4};
    instructions[0xBE] = (Instruction){.name = "LDX", .addressing = ABY, .opcode = LDX, .cycles = 4};
    instructions[0xA0] = (Instruction){.name = "LDY", .addressing = IMM, .opcode = LDY, .cycles = 2};
    instructions[0xA4] = (Instruction){.name = "LDY", .addressing = ZP0, .opcode = LDY, .cycles = 3};
    instructions[0xB4] = (Instruction){.name = "LDY", .addressing = ZPX, .opcode = LDY, .cycles = 4};
    instructions[0xAC] = (Instruction){.name = "LDY", .addressing = ABS, .opcode = LDY, .cycles = 4};
    instructions[0xBC] = (Instruction){.name = "LDY", .addressing = ABX, .opcode = LDY, .cycles = 4};
    instructions[0x4A] = (Instruction){.name = "LSR", .addressing = IMP, .opcode = LSR, .cycles = 2};
    instructions[0x46] = (Instruction){.name = "LSR", .addressing = ZP0, .opcode = LSR, .cycles = 5};
    instructions[0x56] = (Instruction){.name = "LSR", .addressing = ZPX, .opcode = LSR, .cycles = 6};
    instructions[0x4E] = (Instruction){.name = "LSR", .addressing = ABS, .opcode = LSR, .cycles = 6};
    instructions[0x5E] = (Instruction){.name = "LSR", .addressing = ABX, .opcode = LSR, .cycles = 7};
    instructions[0xEA] = (Instruction){.name = "NOP", .addressing = IMP, .opcode = NOP, .cycles = 2};
    instructions[0x09] = (Instruction){.name = "ORA", .addressing = IMM, .opcode = ORA, .cycles = 2};
    instructions[0x05] = (Instruction){.name = "ORA", .addressing = ZP0, .opcode = ORA, .cycles = 3};
    instructions[0x15] = (Instruction){.name = "ORA", .addressing = ZPX, .opcode = ORA, .cycles = 4};
    instructions[0x0D] = (Instruction){.name = "ORA", .addressing = ABS, .opcode = ORA, .cycles = 4};
    instructions[0x1D] = (Instruction){.name = "ORA", .addressing = ABX, .opcode = ORA, .cycles = 4};
    instructions[0x19] = (Instruction){.name = "ORA", .addressing = ABY, .opcode = ORA, .cycles = 4};
    instructions[0x01] = (Instruction){.name = "ORA", .addressing = IZX, .opcode = ORA, .cycles = 6};
    instructions[0x11] = (Instruction){.name = "ORA", .addressing = IZY, .opcode = ORA, .cycles = 5};
    instructions[0x48] = (Instruction){.name = "PHA", .addressing = IMP, .opcode = PHA, .cycles = 3};
    instructions[0x08] = (Instruction){.name = "PHP", .addressing = IMP, .opcode = PHP, .cycles = 3};
    instructions[0x68] = (Instruction){.name = "PLA", .addressing = IMP, .opcode = PLA, .cycles = 4};
    instructions[0x28] = (Instruction){.name = "PLP", .addressing = IMP, .opcode = PLP, .cycles = 4};
    instructions[0x2A] = (Instruction){.name = "ROL", .addressing = IMP, .opcode = ROL, .cycles = 2};
    instructions[0x26] = (Instruction){.name = "ROL", .addressing = ZP0, .opcode = ROL, .cycles = 5};
    instructions[0x36] = (Instruction){.name = "ROL", .addressing = ZPX, .opcode = ROL, .cycles = 6};
    instructions[0x2E] = (Instruction){.name = "ROL", .addressing = ABS, .opcode = ROL, .cycles = 6};
    instructions[0x3E] = (Instruction){.name = "ROL", .addressing = ABX, .opcode = ROL, .cycles = 7};
    instructions[0x6A] = (Instruction){.name = "ROR", .addressing = IMP, .opcode = ROR, .cycles = 2};
    instructions[0x66] = (Instruction){.name = "ROR", .addressing = ZP0, .opcode = ROR, .cycles = 5};
    instructions[0x76] = (Instruction){.name = "ROR", .addressing = ZPX, .opcode = ROR, .cycles = 6};
    instructions[0x6E] = (Instruction){.name = "ROR", .addressing = ABS, .opcode = ROR, .cycles = 6};
    instructions[0x7E] = (Instruction){.name = "ROR", .addressing = ABX, .opcode = ROR, .cycles = 7};
    instructions[0x40] = (Instruction){.name = "RTI", .addressing = IMP, .opcode = RTI, .cycles = 6};
    instructions[0x60] = (Instruction){.name = "RTS", .addressing = IMP, .opcode = RTS, .cycles = 6};
    instructions[0xE9] = (Instruction){.name = "SBC", .addressing = IMM, .opcode = SBC, .cycles = 2};
    instructions[0xE5] = (Instruction){.name = "SBC", .addressing = ZP0, .opcode = SBC, .cycles = 3};
    instructions[0xF5] = (Instruction){.name = "SBC", .addressing = ZPX, .opcode = SBC, .cycles = 4};
    instructions[0xED] = (Instruction){.name = "SBC", .addressing = ABS, .opcode = SBC, .cycles = 4};
    instructions[0xFD] = (Instruction){.name = "SBC", .addressing = ABX, .opcode = SBC, .cycles = 4};
    instructions[0xF9] = (Instruction){.name = "SBC", .addressing = ABY, .opcode = SBC, .cycles = 4};
    instructions[0xE1] = (Instruction){.name = "SBC", .addressing = IZX, .opcode = SBC, .cycles = 6};
    instructions[0xF1] = (Instruction){.name = "SBC", .addressing = IZY, .opcode = SBC, .cycles = 5};
    instructions[0x38] = (Instruction){.name = "SEC", .addressing = IMP, .opcode = SEC, .cycles = 2};
    instructions[0xF8] = (Instruction){.name = "SED", .addressing = IMP, .opcode = SED, .cycles = 2};
    instructions[0x78] = (Instruction){.name = "SEI", .addressing = IMP, .opcode = SEI, .cycles = 2};
    instructions[0x85] = (Instruction){.name = "STA", .addressing = ZP0, .opcode = STA, .cycles = 3};
    instructions[0x95] = (Instruction){.name = "STA", .addressing = ZPX, .opcode = STA, .cycles = 4};
    instructions[0x8D] = (Instruction){.name = "STA", .addressing = ABS, .opcode = STA, .cycles = 4};
    instructions[0x9D] = (Instruction){.name = "STA", .addressing = ABX, .opcode = STA, .cycles = 5};
    instructions[0x99] = (Instruction){.name = "STA", .addressing = ABY, .opcode = STA, .cycles = 5};
    instructions[0x81] = (Instruction){.name = "STA", .addressing = IZX, .opcode = STA, .cycles = 6};
    instructions[0x91] = (Instruction){.name = "STA", .addressing = IZY, .opcode = STA, .cycles = 6};
    instructions[0x96] = (Instruction){.name = "STX", .addressing = ZPY, .opcode = STX, .cycles = 4};
    instructions[0x8E] = (Instruction){.name = "STX", .addressing = ABS, .opcode = STX, .cycles = 4};
    instructions[0x84] = (Instruction){.name = "STY", .addressing = ZP0, .opcode = STY, .cycles = 3};
    instructions[0x94] = (Instruction){.name = "STY", .addressing = ZPX, .opcode = STY, .cycles = 4};
    instructions[0x8C] = (Instruction){.name = "STY", .addressing = ABS, .opcode = STY, .cycles = 4};
    instructions[0xAA] = (Instruction){.name = "TAX", .addressing = IMP, .opcode = TAX, .cycles = 2};
    instructions[0xA8] = (Instruction){.name = "TAY", .addressing = IMP, .opcode = TAY, .cycles = 2};
    instructions[0xBA] = (Instruction){.name = "TSX", .addressing = IMP, .opcode = TSX, .cycles = 2};
    instructions[0x8A] = (Instruction){.name = "TXA", .addressing = IMP, .opcode = TXA, .cycles = 2};
    instructions[0x9A] = (Instruction){.name = "TXS", .addressing = IMP, .opcode = TXS, .cycles = 2};
    instructions[0x98] = (Instruction){.name = "TYA", .addressing = IMP, .opcode = TYA, .cycles = 2};

    log_info("Instructions loaded");
}

uint8_t CPU_read(const uint16_t addr) {
    return BUS_read(addr);
}

void CPU_write(const uint16_t addr, const uint8_t data) {
    BUS_write(addr, data);
}

const CPU *CPU_get_state(void) {
    return &cpu;
}

uint16_t CPU_get_pc(void) {
    return cpu.pc;
}

// Emulate cpu start/reset
void CPU_reset(void) {
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
    cpu.status = 0x00;
    set_flag(FLAG_U, true);

    cpu.addr_abs = 0x0000;
    cpu.addr_rel = 0x0000;

    // A 6502 reset takes ~8 cycles
    cpu.cycles = 8;

    log_info("CPU started");
}


// Emulate interrupt requests that are only allowed if allowed (I flag == 0)
void CPU_irq(void) {
    if (get_flag(FLAG_I) == 1) {
        // If disable interrupts are set, we are not allowed to run
        return;
    }
    hardware_interrupt(CPU_IRQ_LO, CPU_IRQ_HI);
    log_info("CPU IRQ requested, pc at: %04x", cpu.pc);
}

// Emulate non-maskable interrupts i.e., They will always run regardless of I flag
void CPU_nmi(void) {
    hardware_interrupt(CPU_NMI_LO, CPU_NMI_HI);
    log_info("CPU NMI requested, pc at: %04x", cpu.pc);
}

void CPU_tick(void) {
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

void CPU_step(void) {
    while (cpu.cycles > 0) {
        CPU_tick();
    }
    printf("PC=%04X, DATA=%s\n", cpu.pc, Disassembler_get_line_at(cpu.pc));
    CPU_tick();
}

Instruction *CPU_get_instruction(const uint8_t opcode) {
    return &instructions[opcode];
}

/*
 ***********************************************************************
 *                               OPCODES                               *
 ***********************************************************************
 */

uint8_t ADC(void) {
    const uint16_t data = CPU_read(cpu.addr_abs);
    set_add_or_sub_result(data);
    // May require an additional cycle
    return 1;
}

uint8_t AND(void) {
    cpu.a &= CPU_read(cpu.addr_abs);
    set_flag(FLAG_Z, cpu.a == 0);
    set_flag(FLAG_N, cpu.a & 0x80);
    return 0;
}

uint8_t ASL(void) {
    uint16_t data = 0;
    uint16_t res = 0;

    /*
     * We need to separate the logic here.
     * If addressing mode = IMP then we do NOT want to read another
     * byte from memory, but modify the accumulator directly.
     * If not then we DO want to read the next byte and write it back into memory.
     */
    if (is_implied_addressing()) {
        data = cpu.a;
        res = data << 1;
        cpu.a = res & 0x00FF;
    } else {
        data = CPU_read(cpu.addr_abs);
        res = data << 1;
        CPU_write(cpu.addr_abs, res & 0x00FF);
    }

    set_flag(FLAG_C, res > 0x00FF);
    set_flag(FLAG_Z, (res & 0x00FF) == 0);
    set_flag(FLAG_N, res & 0x80);
    return 0;
}

uint8_t BCC(void) {
    branch_on_condition(get_flag(FLAG_C) == 0);
    return 0;
}

uint8_t BCS(void) {
    branch_on_condition(get_flag(FLAG_C) == 1);
    return 0;
}

uint8_t BEQ(void) {
    branch_on_condition(get_flag(FLAG_Z) == 1);
    return 0;
}

uint8_t BIT(void) {
    /*
     * Used for setting N and V bits to whatever is in the memory location read.
     * If bit6 set, then set V flag.
     * If bit7 set, then set N flag.
     * Kind of a poor mans version of SEC/CLC for bit 6 and/or 7 (that's how my brain thinks of them).
     * Extra confusion caused since flag Z is ACTUALLY set based on a real calculation with A
     */
    const uint8_t data = CPU_read(cpu.addr_abs);
    const uint8_t res = cpu.a & data;
    set_flag(FLAG_Z, res == 0);
    set_flag(FLAG_V, data & FLAG_V);
    set_flag(FLAG_N, data & FLAG_N);
    return 0;
}

uint8_t BMI(void) {
    branch_on_condition(get_flag(FLAG_N) == 1);
    return 0;
}

uint8_t BNE(void) {
    branch_on_condition(get_flag(FLAG_Z) == 0);
    return 0;
}

uint8_t BPL(void) {
    branch_on_condition(get_flag(FLAG_N) == 0);
    return 0;
}

uint8_t BRK(void) {
    set_flag(FLAG_I, true);

    // Write hi and lo byte to stack (little-endian, reversed due to SP decrement)
    CPU_write(CPU_STACK_PAGE + cpu.sp--, (cpu.pc >> 8) & 0x00FF);
    CPU_write(CPU_STACK_PAGE + cpu.sp--, cpu.pc & 0x00FF);

    // Set B flag to 1 before pushing status (to indicate BRK vs IRQ)
    set_flag(FLAG_B, true);
    CPU_write(CPU_STACK_PAGE + cpu.sp--, cpu.status);

    // B flag is cleared immediately after (it's only used for identification on the stack)
    set_flag(FLAG_B, false);

    // Jump to IRQ vector
    const uint16_t irq_lo = CPU_read(CPU_IRQ_LO) & 0x00FF;
    const uint16_t irq_hi = CPU_read(CPU_IRQ_HI) & 0x00FF;
    cpu.pc = (irq_hi << 8) | irq_lo;

    return 0;
}

uint8_t BVC(void) {
    // Branch if overflow clear
    branch_on_condition(get_flag(FLAG_V) == 0);
    return 0;
}

uint8_t BVS(void) {
    // Branch on overflow set
    branch_on_condition(get_flag(FLAG_V) == 1);
    return 0;
}

uint8_t CLC(void) {
    set_flag(FLAG_C, false);
    return 0;
}

uint8_t CLD(void) {
    // Clear decimal flag
    set_flag(FLAG_D, false);
    return 0;
}

uint8_t CLI(void) {
    // Clear interrupt disable bit
    set_flag(FLAG_I, false);
    return 0;
}

uint8_t CLV(void) {
    // Clear overflow flag
    set_flag(FLAG_V, false);
    return 0;
}

uint8_t CMP(void) {
    compare_register(cpu.a);
    return 0;
}

uint8_t CPX(void) {
    compare_register(cpu.x);
    return 0;
}

uint8_t CPY(void) {
    compare_register(cpu.y);
    return 0;
}

uint8_t DEC(void) {
    // Decrement memory by one
    uint8_t data = CPU_read(cpu.addr_abs);
    data--;
    CPU_write(cpu.addr_abs, data);
    set_flag(FLAG_Z, data == 0);
    set_flag(FLAG_N, data & 0x80);
    return 0;
}

uint8_t DEX(void) {
    // Decrement X register by one
    cpu.x--;
    set_flag(FLAG_Z, cpu.x == 0);
    set_flag(FLAG_N, cpu.x & 0x80);
    return 0;
}

uint8_t DEY(void) {
    // Decrement Y register by one
    cpu.y--;
    set_flag(FLAG_Z, cpu.y == 0);
    set_flag(FLAG_N, cpu.y & 0x80);
    return 0;
}

uint8_t EOR(void) {
    // Exclusive or memory with accumulator
    const uint8_t data = CPU_read(cpu.addr_abs);
    cpu.a ^= data;
    set_flag(FLAG_Z, cpu.a == 0);
    set_flag(FLAG_N, cpu.a & 0x80);
    return 0;
}

uint8_t ILL(void) {
    return 0;
}

uint8_t INC(void) {
    uint8_t data = CPU_read(cpu.addr_abs);
    data++;
    CPU_write(cpu.addr_abs, data);
    set_flag(FLAG_Z, data == 0);
    set_flag(FLAG_N, data & 0x80);
    return 0;
}

uint8_t INX(void) {
    cpu.x++;
    set_flag(FLAG_Z, cpu.x == 0);
    set_flag(FLAG_N, cpu.x & 0x80);
    return 0;
}

uint8_t INY(void) {
    cpu.y++;
    set_flag(FLAG_Z, cpu.y == 0);
    set_flag(FLAG_N, cpu.y & 0x80);
    return 0;
}

uint8_t JMP(void) {
    cpu.pc = cpu.addr_abs;
    return 0;
}

uint8_t JSR(void) {
    cpu.pc--;

    // Push PC hi and lo byte of pc to stack
    CPU_write(CPU_STACK_PAGE + cpu.sp--, (cpu.pc >> 8) & 0x00FF);
    CPU_write(CPU_STACK_PAGE + cpu.sp--, cpu.pc & 0x00FF);

    // set pc to new address
    cpu.pc = cpu.addr_abs;
    return 0;
}

uint8_t LDA(void) {
    // Load into accumulator
    cpu.a = CPU_read(cpu.addr_abs);
    set_flag(FLAG_Z, cpu.a == 0);
    set_flag(FLAG_N, cpu.a & 0x80);
    return 1;
}

uint8_t LDX(void) {
    // Load into x register
    cpu.x = CPU_read(cpu.addr_abs);
    set_flag(FLAG_Z, cpu.x == 0);
    set_flag(FLAG_N, cpu.x & 0x80);
    return 1;
}

uint8_t LDY(void) {
    // Load into y register
    cpu.y = CPU_read(cpu.addr_abs);
    set_flag(FLAG_Z, cpu.y == 0);
    set_flag(FLAG_N, cpu.y & 0x80);
    return 1;
}

uint8_t LSR(void) {
    uint16_t data = 0;
    uint16_t res = 0;

    /*
     * We need to separate the logic here.
     * If addressing mode = IMP then we do NOT want to read another
     * byte from memory, but modify the accumulator directly.
     * If not then we DO want to read the next byte and write it back into memory.
     */
    if (is_implied_addressing()) {
        data = cpu.a;
        set_flag(FLAG_C, data & 0x01);
        res = data >> 1;
        cpu.a = res & 0x00FF;
    } else {
        data = CPU_read(cpu.addr_abs);
        set_flag(FLAG_C, data & 0x01);
        res = data >> 1;
        CPU_write(cpu.addr_abs, res & 0x00FF);
    }

    set_flag(FLAG_Z, (res & 0x00FF) == 0);
    set_flag(FLAG_N, false); // MSB is always 0 after LSR
    return 0;
}

uint8_t NOP(void) {
    return 0;
}

uint8_t ORA(void) {
    // OR memory with accumulator
    const uint8_t data = CPU_read(cpu.addr_abs);
    cpu.a |= data;
    set_flag(FLAG_Z, cpu.a == 0);
    set_flag(FLAG_N, cpu.a & 0x80);

    // Candidate for additional cycle
    return 1;
}

uint8_t PHA(void) {
    // Push accumulator to stack
    CPU_write(CPU_STACK_PAGE + cpu.sp, cpu.a);
    cpu.sp--;
    return 0;
}

uint8_t PHP(void) {
    /*
     * Push status register to stack. Before pushing, B and U need to be set
     * and then toggled off.
     * */
    CPU_write(CPU_STACK_PAGE + cpu.sp, cpu.status | FLAG_B | FLAG_U);
    set_flag(FLAG_B, false);
    set_flag(FLAG_U, false);
    cpu.sp--;
    return 0;
}

uint8_t PLA(void) {
    // Pop accumulator from stack
    cpu.sp++;
    cpu.a = CPU_read(CPU_STACK_PAGE + cpu.sp);
    set_flag(FLAG_Z, cpu.a == 0);
    set_flag(FLAG_N, cpu.a & 0x80);
    return 0;
}

uint8_t PLP(void) {
    // Pop status register from stack
    cpu.sp++;
    cpu.status = CPU_read(CPU_STACK_PAGE + cpu.sp);
    set_flag(FLAG_B, false);
    set_flag(FLAG_U, true);
    return 0;
}

uint8_t ROL(void) {
    uint16_t data = 0;
    uint16_t res = 0;

    // Rotate one bit left, LSB = whatever is in carry
    const bool carry = get_flag(FLAG_C);

    if (is_implied_addressing()) {
        data = cpu.a;
        res = data << 1 | carry;
        cpu.a = res & 0x00FF;
    } else {
        data = CPU_read(cpu.addr_abs);
        res = data << 1 | carry;
        CPU_write(cpu.addr_abs, res & 0x00FF);
    }

    set_flag(FLAG_C, res > 0x00FF);
    set_flag(FLAG_Z, (res & 0x00FF) == 0);
    set_flag(FLAG_N, res & 0x0080);

    return 0;
}

uint8_t ROR(void) {
    // Rotate one bit right, MSB = whatever is in carry
    uint16_t data = 0;
    uint16_t res = 0;
    const bool carry = get_flag(FLAG_C);

    if (is_implied_addressing()) {
        data = cpu.a;
        set_flag(FLAG_C, data & 0x01);  // Bit 0 goes to carry
        res = (data >> 1) | (carry << 7);
        cpu.a = res & 0x00FF;
    } else {
        data = CPU_read(cpu.addr_abs);
        set_flag(FLAG_C, data & 0x01);  // Bit 0 goes to carry
        res = (data >> 1) | (carry << 7);
        CPU_write(cpu.addr_abs, res & 0x00FF);
    }

    set_flag(FLAG_Z, (res & 0x00FF) == 0);
    set_flag(FLAG_N, res & 0x0080);

    return 0;
}

uint8_t RTI(void) {
    // Retrieve status register from stack (should be the last thing that was pushed)
    cpu.status = CPU_read(CPU_STACK_PAGE + (++cpu.sp));

    // Set I and B to 0
    set_flag(FLAG_I, false);
    set_flag(FLAG_B, false);

    // Retrieve where pc was before calling an interrupt.
    const uint16_t pc_lo = CPU_read(CPU_STACK_PAGE + (++cpu.sp));
    const uint16_t pc_hi = CPU_read(CPU_STACK_PAGE + (++cpu.sp));

    // Set pc to where it was before entering the subroutine
    cpu.pc = (pc_hi << 8) | (pc_lo & 0x00FF);
    return 0;
}

uint8_t RTS(void) {
    // Pop PC lo and hi byte from the stack
    const uint16_t pc_lo = CPU_read(CPU_STACK_PAGE + (++cpu.sp));
    const uint16_t pc_hi = CPU_read(CPU_STACK_PAGE + (++cpu.sp));

    // Set pc to where it was before entering the subroutine +1 (since we decrement by one on JSR)
    cpu.pc = ((pc_hi << 8) | (pc_lo & 0x00FF)) + 1;
    return 0;
}

uint8_t SBC(void) {
    const uint16_t data = CPU_read(cpu.addr_abs);
    set_add_or_sub_result(data ^ 0x00FF);
    // May required additional cycle
    return 1;
}

uint8_t SEC(void) {
    set_flag(FLAG_C, true);
    return 0;
}

uint8_t SED(void) {
    // Set decimal mode
    set_flag(FLAG_D, true);
    return 0;
}

uint8_t SEI(void) {
    // Set interrupt disable bit (0 = irq and brk are free to go, nmi will always fire)
    set_flag(FLAG_I, true);
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

uint8_t TAX(void) {
    cpu.x = cpu.a;
    set_flag(FLAG_Z, cpu.x == 0);
    set_flag(FLAG_N, cpu.x & 0x80);
    return 0;
}

uint8_t TAY(void) {
    cpu.y = cpu.a;
    set_flag(FLAG_Z, cpu.y == 0);
    set_flag(FLAG_N, cpu.y & 0x80);
    return 0;
}

uint8_t TSX(void) {
    cpu.x = cpu.sp;
    set_flag(FLAG_Z, cpu.x == 0);
    set_flag(FLAG_N, cpu.x & 0x80);
    return 0;
}

uint8_t TXA(void) {
    cpu.a = cpu.x;
    set_flag(FLAG_Z, cpu.a == 0);
    set_flag(FLAG_N, cpu.a & 0x80);
    return 0;
}

uint8_t TXS(void) {
    cpu.sp = cpu.x;
    return 0;
}

uint8_t TYA(void) {
    cpu.a = cpu.y;
    set_flag(FLAG_Z, cpu.a == 0);
    set_flag(FLAG_N, cpu.a & 0x80);
    return 0;
}


// ==============================================
// Addressing modes
// ==============================================

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

uint8_t IMM(void) {
    // Immediate mode, set absolute address to pc and inc pc
    // Requires no additional cycles
    cpu.addr_abs = cpu.pc++;
    return 0;
}

uint8_t IMP(void) {
    // There could be stuff going on with the accumulator in implied mode
    cpu.curr_opcode = cpu.a;
    return 0;
}

uint8_t IND(void) {
    /*
     * Indirect addressing mode meaning the location we are reading is a 16-bit pointer to the actual
     * address to set addr_abs to.
     */
    const uint16_t ptr_lo = CPU_read(cpu.pc++) & 0x00FF;
    const uint16_t ptr_hi = CPU_read(cpu.pc++) & 0x00FF;

    const uint16_t data = (ptr_hi << 8) | ptr_lo;

    /*
     * There is a bug in the 6502 where if the low byte of the address is 0xFF it does
     * not jump to the next page in memory but wraps around in the same page
     */
    const uint8_t new_addr_lo = CPU_read(data);
    uint16_t new_addr_hi = 0;
    if (ptr_lo == 0x00FF) {
        new_addr_hi = CPU_read(data & 0xFF00) << 8;
    } else {
        new_addr_hi = CPU_read(data + 1);
    }
    cpu.addr_abs = new_addr_hi << 8 | new_addr_lo;

    return 0;
}

uint8_t IZX(void) {
    /*
     * Pre-indexed indirect addressing mode in the zero page.
     * Pc is set to hi-byte+x+1 | lo-byte+x to data at location read
     */
    const uint16_t ptr = CPU_read(cpu.pc++) & 0x00FF;
    const uint16_t x = cpu.x;

    const uint8_t new_addr_lo = CPU_read(ptr + x);
    const uint8_t new_addr_hi = CPU_read(ptr + x + 1);

    cpu.addr_abs = new_addr_hi << 8 | new_addr_lo;
    return 0;
}

uint8_t IZY(void) {
    /*
     * Address Mode: Indirect Y
     * The supplied 8-bit address indexes a location in page 0x00. From
     * here the actual 16-bit address is read, and the contents of
     * Y Register are added to it to offset it. If the offset causes a
     * change in the page, then an additional clock cycle is required.
     */
    const uint16_t ptr = CPU_read(cpu.pc++) & 0x00FF;
    const uint16_t lo = CPU_read(ptr);
    const uint16_t hi = CPU_read(ptr + 1);

    cpu.addr_abs = ((hi << 8) | lo) + cpu.y;
    if ((cpu.addr_abs & 0xFF00) != (hi << 8)) {
        return 1;
    }
    return 0;
}

uint8_t REL(void) {
    cpu.addr_rel = CPU_read(cpu.pc++);

    if (cpu.addr_rel & 0x80) {
        cpu.addr_rel |= 0xFF00;
    }
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

