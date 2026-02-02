//
// Created by johan on 2026-01-27.
//
#include <gtest/gtest.h>

extern "C" {
#include "../core/bus.h"
#include "../core/cpu.h"
}


static CPU *cpu;

class CPUTest : public testing::Test {
protected:
    static void SetUpTestSuite() {
        CPU_load_instructions();
        cpu = CPU_get_state();
    }
    void SetUp() override {
        BUS_init();
    }
};

TEST_F(CPUTest, LoadAccumulator) {
    char rom[] = "A9 42";  // LDA immediate with 0x42
    BUS_load_ROM_from_str(0x600, rom);
    CPU_reset();
    CPU_step();
    EXPECT_EQ(cpu->a, 0x42);
}
