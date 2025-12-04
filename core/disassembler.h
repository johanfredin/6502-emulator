//
// Created by johan on 2025-10-23.
//

#ifndef INC_6502_EMULATOR_DISASSEMBLER_H
#define INC_6502_EMULATOR_DISASSEMBLER_H
#include <stdint.h>

#include "rom.h"

typedef struct SourceLine {
    char *line;
    uint16_t address;
} SourceLine;

typedef struct SourceCode {
    SourceLine *lines;
    uint16_t n_lines;
} SourceCode;

void Disassembler_parse_rom(const ROM *rom);
void Disassembler_parse_section(uint16_t start, uint16_t end);
char *Disassembler_get_line_at(uint16_t address);
SourceCode *Disassembler_get_code();

#endif //INC_6502_EMULATOR_DISASSEMBLER_H