#include "bus.h"
#include "cpu.h"
#include "disassembler.h"

int main(void) {
    Bus_init();
    CPU_load_instructions();

    char program[] = "a2 00 a0 00 8a 99 00 02 48 e8 c8 c0 10 d0 f5 68 99 00 02 c8 c0 20 d0 f7";
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



    return 0;
}
