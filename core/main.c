#include <stdlib.h>

#include "bus.h"
#include "cpu.h"
#include "disassembler.h"

int main(void) {
    Bus_init();
    CPU_load_instructions();

    char program[] = "20 09 06 20 0C 06 20 12 06 A0 10 60 E8 E0 05 D0 FB 60 A9 FF";
    Bus_load_rom(0x0600, program);
    CPU_reset();

    // Dump code
    Disassembler_parse_binary(0x0600, 0x0700);
    CPU_step();
    CPU_step();
    CPU_step();
    CPU_step();
    CPU_step();
    CPU_step();
    CPU_step();
    CPU_step();
    CPU_step();
    CPU_step();
    CPU_step();
    CPU_step();
    CPU_step();
    CPU_step();
    CPU_step();
    CPU_step();
    CPU_step();
    CPU_step();
    CPU_step();
    CPU_step();
    CPU_step();
    CPU_step();

    return EXIT_SUCCESS;
}
