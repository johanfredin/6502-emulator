//
// Created by johan on 2025-10-23.
//

#include "disassembler.h"

#include <stdlib.h>

#include "cpu.h"
#include "dbg.h"

static SourceCode code;

void Disassembler_parse_binary(const uint16_t start, const uint16_t end) {
    const uint16_t max_instructions = end - start;
    SourceLine *lines = calloc(max_instructions, sizeof(SourceLine));
    check(lines, "Failed to allocate memory for disassembly lines", {
        exit(1);
    });

    uint16_t n_instructions = 0;
    for (uint16_t addr = start; addr < end;) {
        // Save the start of the current line to accurately print the memory address of instruction
        const uint16_t origin = addr;

        char buffer[64];
        const uint8_t opcode = CPU_read(addr++);
        const Instruction *ins = CPU_get_instruction(opcode);

        char operand_str[16] = "";
        const addressing_fn addr_fn = ins->addressing;
        if (addr_fn == IMP) {
            snprintf(operand_str, sizeof(operand_str), "{IMP}");
        } else if (addr_fn == IMM) {
            const uint8_t data = CPU_read(addr++);
            snprintf(operand_str, sizeof(operand_str), "#$%02x {IMM}", data);
        } else if (addr_fn == ABS) {
            const uint8_t lo = CPU_read(addr++);
            const uint8_t hi = CPU_read(addr++);
            const uint16_t abs = (hi << 8) | lo;
            snprintf(operand_str, sizeof(operand_str), "$%04x {ABS}", abs);
        }

        snprintf(buffer, 64,
            "%04x: %-4s %s",
            origin,
            ins->name,
            operand_str
        );

        lines[n_instructions].address = origin;
        lines[n_instructions].line = strdup(buffer);
        check(lines[n_instructions].line, "Failed to allocate memory for new line", {
              exit(1);
        });

        n_instructions++;
    }

    code = (SourceCode){lines, n_instructions};
    log_info("Binary disassembled");
}

SourceCode *Disassembler_get_code() {
    return &code;
}

char *Disassembler_get_line_at(const uint16_t address) {
    for (uint16_t i = 0; i < code.n_lines; i++) {
        if (code.lines[i].address == address) {
            return code.lines[i].line;
        }
    }
    return "NOT_FOUND";
}