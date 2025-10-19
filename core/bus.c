//
// Created by johan on 2025-10-02.
//

#include "bus.h"

#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include "dbg.h"


static uint8_t ram[RAM_SIZE];

void Bus_init(void) {
    memset(ram, 0, RAM_SIZE);
}

void Bus_load_rom(const uint16_t org, char *rom) {
    // Load the program ROM
    const char *token = strtok(rom, " ");
    int i = 0;
    while (token != NULL) {
        const uint8_t value = (uint8_t) strtoul(token, NULL, 16);
        Bus_write((org + i), value);
        token = strtok(NULL, " ");
        i++;
    }

    // Load reset vector (cheating for now)
    Bus_write(CPU_RESET_LO, org & 0xFF);
    Bus_write(CPU_RESET_HI, (org >> 8) & 0xFF);

    log_info("Rom loaded at 0x%04x", org);
}

uint8_t Bus_read(const uint16_t addr) {
    return ram[addr];
}

void Bus_write(const uint16_t addr, const uint8_t data) {
    ram[addr] = data;
}

const uint8_t *Bus_get_page(const uint8_t page) {
    if (page > 0xFF) {
        return Bus_get_page(0);
    }

    log_info("Page retrieved at: %d", page);
    return &ram[page * 0x100];
}
