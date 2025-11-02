#include "bus.h"
#include "cpu.h"
#include "disassembler.h"

int main(void) {
    Bus_init();
    CPU_load_instructions();

    char program[] = "A9 0A 8D 00 02 A2 14 8E 01 02 A0 1E 8C 02 02 A9 00 8A A2 00 98 A0 00 AA A8";
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
