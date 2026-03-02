# Test suite for Anvil library
cmake_minimum_required(VERSION 3.16)

# Find all test source files across subdirectories
file(GLOB_RECURSE TEST_SOURCES
    "containers/test_*.c"
    "common/test_*.c"
    "memory/test_*.c"
    "algorithms/test_*.c"
    "system/test_*.c"
)

if(NOT TEST_SOURCES)
    message(WARNING "No test files found in subdirectories of ${CMAKE_CURRENT_SOURCE_DIR}")
    return()
endif()

list(LENGTH TEST_SOURCES TEST_COUNT)
message(STATUS "Found ${TEST_COUNT} test files")

# Internal include directories (tests use short paths like "containers/arraylist.h")
set(ANV_INTERNAL_INCLUDE_DIRS
    "${CMAKE_SOURCE_DIR}/include/anvil"
)

# Create an OBJECT library for test helpers
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/support/TestHelper.c")
    add_library(test_helpers OBJECT support/TestHelper.c)
    target_include_directories(test_helpers PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/support
        ${ANV_INTERNAL_INCLUDE_DIRS}
    )

    target_link_libraries(test_helpers PRIVATE Anvil::Anvil)

    # Apply same compiler settings as main library
    target_compile_options(test_helpers PRIVATE ${ANVIL_COMPILE_FLAGS})

    # Important: Apply sanitizer flags to test helpers too
    if(ANVIL_LINK_FLAGS)
        target_compile_options(test_helpers PUBLIC ${ANVIL_SANITIZER_FLAGS})
    endif()

    set(TEST_HELPER_OBJECTS $<TARGET_OBJECTS:test_helpers>)
else()
    set(TEST_HELPER_OBJECTS "")
endif()

# Loop through all found test sources and create an executable and test for each
foreach(TEST_SOURCE ${TEST_SOURCES})
    # Get the base name of the file to use as the test name
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)

    # Add the executable for the test
    add_executable(${TEST_NAME} ${TEST_SOURCE} ${TEST_HELPER_OBJECTS})

    # Link the test executable against the main library
    target_link_libraries(${TEST_NAME} PRIVATE Anvil::Anvil)

    # Add include directories: support/ for test framework, internal for library headers
    target_include_directories(${TEST_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/support
        ${ANV_INTERNAL_INCLUDE_DIRS}
    )

    # Apply sanitizer flags to test executables
    if(ANVIL_SANITIZER_FLAGS)
        target_compile_options(${TEST_NAME} PRIVATE ${ANVIL_SANITIZER_FLAGS})
        target_link_libraries(${TEST_NAME} PRIVATE ${ANVIL_SANITIZER_FLAGS})
    endif()

    # Set output directory for tests (flat, so CTest and DLL copy work)
    set_target_properties(${TEST_NAME} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
    )

    # Add to CTest with timeout to prevent hangs
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    set_tests_properties(${TEST_NAME} PROPERTIES TIMEOUT 30)

    # Set environment variables for tests
    if(ANVIL_ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
        set_tests_properties(${TEST_NAME} PROPERTIES
                ENVIRONMENT "ASAN_OPTIONS=halt_on_error=1:check_initialization_order=1:strict_init_order=1"
        )
    endif()
endforeach()

# Create a custom target to run all tests
add_custom_target(run_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --verbose
        COMMENT "Running all Anvil tests"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Summary message
message(STATUS "Configured ${TEST_COUNT} test executables")
