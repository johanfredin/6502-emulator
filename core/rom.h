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
    uint16_t size;
    uint16_t org;
} ROM;

void ROM_from_file(ROM *rom, const char *filename);

#endif //INC_6502_EMULATOR_ROM_H