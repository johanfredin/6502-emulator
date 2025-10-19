//
// Created by johan on 2025-10-02.
//

#ifndef INC_6502_EMULATOR_BUS_H
#define INC_6502_EMULATOR_BUS_H


#include <stdint.h>

#define RAM_SIZE (64 * 1024)
#define BUS_GET_ZERO_PAGE() (Bus_get_page(0))


void Bus_init(void);
uint8_t Bus_read(uint16_t addr);
void Bus_write(uint16_t addr, uint8_t data);
void Bus_load_rom(uint16_t org, char *rom);
const uint8_t *Bus_get_page(uint8_t page);

#endif //INC_6502_EMULATOR_BUS_H