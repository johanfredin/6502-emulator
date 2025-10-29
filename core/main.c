#include <stdio.h>

#include "bus.h"
#include "cpu.h"
#include "disassembler.h"

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

    // Dump code
    FILE *f = fopen("./code.txt", "w");
    SourceCode *code = Disassembler_get_code();
    for (int i = 0; i < code->n_lines; i++) {
        puts(code->lines[i].line);
        fprintf(f, "%s\n", code->lines[i].line);
    }


    CPU_step();
    CPU_step();
    CPU_step();
    CPU_step();
    CPU_step();
    CPU_step();



    return 0;
}
