//
// Created by johan on 2025-10-02.
//

#ifndef INC_6502_EMULATOR_BUS_H
#define INC_6502_EMULATOR_BUS_H

#include <stdint.h>
#include "rom.h"

#define RAM_SIZE (64 * 1024)
#define BUS_GET_ZERO_PAGE() (BUS_get_page(0))

void BUS_init(void);
void BUS_load_ROM_from_str(uint16_t org, char *rom);
void BUS_load_ROM(const ROM *rom);
void BUS_write(uint16_t addr, uint8_t data);

uint8_t BUS_read(uint16_t addr);
uint8_t *BUS_get_page(uint8_t page);

#endif //INC_6502_EMULATOR_BUS_H