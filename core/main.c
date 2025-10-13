#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bus.h"
#include "cpu.h"

static void load_program(const uint16_t org, char *program) {
    // Load the program ROM
    const char *token = strtok(program, " ");
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
}

int main(void) {
    Bus_init();
    CPU_init();

    /*
     * Example dummy program
     * LDA #$10
     * STA $0200
     */
    char program[] = "A9 10 8D 00 02";
    load_program(0x0600, program);
    CPU_reset();

    while (1) {

        CPU_tick();
    }

    return 0;
}