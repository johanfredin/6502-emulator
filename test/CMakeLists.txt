# Define the list of test files
set(TEST_FILES
        cpu_test
)

# Handle all test files in one loop
foreach(test ${TEST_FILES})
    add_executable(${test} ${test}.cpp)
    target_link_libraries(${test}
            PRIVATE
            6502_emulator_lib
            gtest_main
    )
    add_test(NAME ${test} COMMAND ${test})
endforeach()
