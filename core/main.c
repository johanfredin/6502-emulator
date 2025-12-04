#include <stdio.h>
#include <stdlib.h>

#include "bus.h"
#include "cpu.h"
#include "disassembler.h"
#include "rom.h"


int main(void) {
    ROM rom;
    ROM_from_file(&rom, "Interrupts.bin");
    BUS_init();
    CPU_load_instructions();
    BUS_load_ROM(&rom);
    CPU_reset();

    // Dump code
    Disassembler_parse_rom(&rom);
    while (CPU_get_pc() < rom.end) {
        CPU_step();
    }

    return EXIT_SUCCESS;
}
