//
// Created by johan on 2025-12-02.
//

#ifndef INC_6502_EMULATOR_ROM_H
#define INC_6502_EMULATOR_ROM_H

#include <stdint.h>

#define ROMS_DIR "../resources/examples/"

typedef struct ROM {
    uint8_t *data;
    char *file;
    uint16_t end;
    uint16_t start;
} ROM;

/**
 * Parse a binary file into a ROM struct. This function will assume that
 * all interrupt handlers and pc start specified in the data. That means
 * that at the very least, the reset vector (0xFFFC) must be set. If the program
 * Uses an irq handler and/or depends on nmi handler, then those addresses (0xFFFE, 0xFFFA)
 * must also be specified.
 * @param rom the rom to populate (out parameter)
 * @param filename path to binary file
 */
void ROM_from_file(ROM *rom, const char *filename);

#endif //INC_6502_EMULATOR_ROM_H
