//
// Created by johan on 2026-01-27.
//
#include <unity.h>

#include "../core/bus.h"
#include "../core/cpu.h"

static CPU *cpu;

static void before_tests(void) {
    CPU_load_instructions();
    cpu = CPU_get_state();
}

void setUp(void) {
    BUS_init();
}

void tearDown(void) {
    puts("DOWN");
}

void test_adc_imm(void) {
    // Set up
    char rom[] = "69 10";
    BUS_load_ROM_from_str(0x600, rom);
    CPU_reset();

    // Act
    CPU_step();

    // Verify
    TEST_ASSERT_EQUAL_UINT8(16, cpu->a);
}

int main(void) {
    before_tests();


    UNITY_BEGIN();
    RUN_TEST(test_adc_imm);
    UNITY_END();
}