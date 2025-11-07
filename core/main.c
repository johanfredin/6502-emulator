#include "bus.h"
#include "cpu.h"
#include "disassembler.h"

int main(void) {
    Bus_init();
    CPU_load_instructions();

    char program[] = "E8 E0 0A D0 FB 8A";
    Bus_load_rom(0x0600, program);
    CPU_reset();

    // Dump code
    Disassembler_parse_binary(0x0600, 0x06FF);
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



    return 0;
}
