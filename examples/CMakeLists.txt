# Test suite for Anvil library
cmake_minimum_required(VERSION 3.16)
project(Anvil-Examples LANGUAGES C)

set (TESTING_SOURCES
        testing/benchmark.c
)

set (TESTING_HEADERS
        testing/testing_setup.h
)

foreach(TEST_SOURCE ${TESTING_SOURCES})
    # Get the base name of the file to use as the test name
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)

    # Add the executable for the test
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    target_include_directories(${TEST_NAME} PUBLIC "testing")

    # Link the test executable against the main library
    target_link_libraries(${TEST_NAME} PRIVATE Anvil::Anvil)

    target_compile_options(${TEST_NAME} PRIVATE ${ANV_COMPILE_FLAGS})

    if(ANV_ENABLE_SANITIZERS)
        target_link_libraries(${TEST_NAME} PRIVATE ${ANV_LINK_FLAGS})
    endif()

    set_target_properties(${TEST_NAME} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples"
    )

    # Add to CTest
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

    # Set environment variables for tests
    if(ANV_ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
        set_tests_properties(${TEST_NAME} PROPERTIES
                ENVIRONMENT "ASAN_OPTIONS=halt_on_error=1:check_initialization_order=1:strict_init_order=1"
        )
    endif()
endforeach()