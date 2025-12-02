//
// Created by johan on 2025-10-02.
//

#include "bus.h"

#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include "dbg.h"
#include "rom.h"


static uint8_t ram[RAM_SIZE];

void BUS_init(void) {
    memset(ram, 0, RAM_SIZE);
}

void BUS_load_ROM_from_str(const uint16_t org, char *rom) {
    // Load the program ROM
    const char *token = strtok(rom, " ");
    int i = 0;
    while (token != NULL) {
        const uint8_t value = (uint8_t) strtoul(token, NULL, 16);
        BUS_write((org + i), value);
        token = strtok(NULL, " ");
        i++;
    }

    // Load reset vector (cheating for now)
    BUS_write(CPU_RESET_LO, org & 0xFF);
    BUS_write(CPU_RESET_HI, (org >> 8) & 0xFF);

    log_info("Rom loaded at 0x%04x", org);
}

void BUS_load_ROM(const ROM *const rom) {
    /*
     * This version does not manually load RESET vectors but assumes
     * that rom data has pc start at that address. We could, of course,
     * manually populate it from the hi and lo bytes of rom->org, but
     * that would be cheating :P
     */
    const uint16_t org = rom->org;
    const uint8_t *data = rom->data;
    const uint16_t size = rom->size;

    for (int i = 0; i < size; i++) {
        BUS_write((org + i), data[i]);
    }
    log_info("Rom loaded at 0x%04x", org);
}

uint8_t BUS_read(const uint16_t addr) {
    return ram[addr];
}

void BUS_write(const uint16_t addr, const uint8_t data) {
    ram[addr] = data;
}

uint8_t *BUS_get_page(const uint8_t page) {
    log_debug("Page retrieved at: %d", page);
    return &ram[page * 0x100];
}
