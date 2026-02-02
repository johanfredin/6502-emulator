cmake_minimum_required(VERSION 3.31)
project(6502_emulator C CXX)

set(CMAKE_COMPILER clang)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

# Include FetchContent for downloading Unity
include(FetchContent)

# Fetch Unity testing framework
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

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

# Enable testing
enable_testing()

# Add the tests subdirectory
add_subdirectory(test)

# Add custom target that just runs the tests
add_custom_target(check
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running tests..."
)

add_executable(6502_emulator
        core/main.c
)

target_link_libraries(6502_emulator 6502_emulator_lib)