#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "bus.h"
#include "cpu.h"

static void load_binary(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        printf("Failed to open file %s\n", filename);
        exit(1);
    }

    fseek(f, 0, SEEK_END);
    const size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);

    uint8_t *data = malloc(size);
    fread(data, 1, size, f);
    fclose(f);

    // Load data into our "ram"
    for (size_t i = 0; i < size; i++) {
        Bus_write(i, data[i]);
    }

    free(data);
}

void print_cpu(const CPU *cpu) {
    puts("Registers:");
    printf("A: %02x\n", cpu->a);
    printf("X: %02x\n", cpu->x);
    printf("Y: %02x\n", cpu->y);
    puts("");
    for (uint8_t i = Bus_read(cpu->pc); i < 16; i++) {
        if (i % 8 == 0) {
            puts("");
        }
        printf("%02x\t", Bus_read(i));
    }
    system("clear");
}

int main(void) {
#ifdef _WIN32
    system("cls");
#else
    setenv("TERM", "xterm", 0);  // Set only if not already set
    system("clear");
#endif

    Bus_init();
    CPU_init();

    load_binary("/home/johan/CLionProjects/6502-emulator/hello-world.bin");

    CPU_reset();

    while (1) {
        print_cpu(CPU_get_state());
        CPU_tick();
    }

    return 0;
}