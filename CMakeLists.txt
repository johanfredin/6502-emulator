cmake_minimum_required(VERSION 3.5)
project(6502_emulator C)

set(CMAKE_C_STANDARD 99)

add_library(6502_emulator_lib SHARED
        core/cpu.c
        core/cpu.h
        core/bus.c
        core/bus.h
        core/disassembler.c
        core/disassembler.h
        core/rom.c
        core/rom.h
)

add_executable(6502_emulator
        core/main.c
)

target_link_libraries(6502_emulator 6502_emulator_lib)