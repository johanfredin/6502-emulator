#include "bus.h"
#include "cpu.h"

int main(void) {
    Bus_init();
    CPU_load_instructions();

    /*
     * Example dummy program
     * LDA #$10
     * STA $0200
     */
    char program[] = "A9 10 8D 00 02";
    Bus_load_rom(0x0600, program);
    CPU_reset();

    CPU_step();
    CPU_step();

    return 0;
}