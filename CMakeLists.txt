cmake_minimum_required(VERSION 4.0)
project(6502_emulator C)

set(CMAKE_C_STANDARD 99)

add_executable(6502_emulator
        main.c
        cpu.c
        cpu.h
        bus.c
        bus.h
)
