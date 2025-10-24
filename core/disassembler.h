//
// Created by johan on 2025-10-23.
//

#ifndef INC_6502_EMULATOR_DISASSEMBLER_H
#define INC_6502_EMULATOR_DISASSEMBLER_H
#include <stdint.h>

void Disassembler_get_source_code(uint16_t start, uint16_t end);
char *Disassembler_get_line_at(uint16_t address);


#endif //INC_6502_EMULATOR_DISASSEMBLER_H