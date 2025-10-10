//
// Created by johan on 2025-10-02.
//

#include "bus.h"
#include <string.h>


static uint8_t ram[RAM_SIZE];

void Bus_init(void) {
    memset(ram, 0, RAM_SIZE);
}

uint8_t Bus_read(const uint16_t addr) {
    return ram[addr];
}

void Bus_write(const uint16_t addr, const uint8_t data) {
    ram[addr] = data;
}