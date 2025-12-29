//
// Created by johan on 2025-12-02.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rom.h"
#include "dbg.h"

void ROM_from_file(ROM *const rom, const char *filename) {
    // Open the file
    char *file_path = calloc(64, sizeof(char));
    strncat(file_path, ROMS_DIR, 64);
    strncat(file_path, filename, 64);
    FILE *file = fopen(file_path, "rb");
    check(file, "Failed to open file", return);

    fseek(file, 0, SEEK_END);
    const long file_size = ftell(file);
    rewind(file);

    uint8_t *bytes = calloc(file_size, sizeof(uint8_t));
    check_mem(bytes, {
        fclose(file);
        return;
    });

    // Read the binary file into the temporary buffer
    fread(bytes, 1, file_size, file);
    fclose(file);

    /*
     * The start of the program is located at 0xFFFC (almost at the very end).
     */
    const uint16_t index_org = file_size - 3;
    const uint16_t org_hi = bytes[index_org];
    const uint16_t org_lo = bytes[index_org - 1];

    rom->start = org_hi << 8 | org_lo;
    rom->data = bytes;
    rom->file = file_path;
    rom->end = rom->start + (file_size - 1);
}
