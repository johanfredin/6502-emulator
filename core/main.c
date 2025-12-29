#include <stdio.h>
#include <stdlib.h>

#include "bus.h"
#include "cpu.h"
#include "disassembler.h"
#include "rom.h"


int main(void) {
    // ROM rom;
    // ROM_from_file(&rom, "kernel-rom.bin");
    BUS_init();
    CPU_load_instructions();
    // BUS_load_ROM(&rom);
    CPU_reset();

    // Dump code
    Disassembler_parse_section(0xFF00, 0xFFFF);
    while (CPU_get_pc() < 0xFFFF) {
        CPU_step();
    }

    return EXIT_SUCCESS;
}
