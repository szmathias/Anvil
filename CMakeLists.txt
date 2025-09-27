cmake_minimum_required(VERSION 3.16)
project(Anvil
        LANGUAGES C
        VERSION 0.1.0
        DESCRIPTION "Personal Standard Library - Useful tools for building C applications"
)

# =============================================================================
# Build Configuration
# =============================================================================

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Default to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

# =============================================================================
# Options
# =============================================================================

option(ANV_BUILD_SHARED "Build shared library instead of static" ON)
option(ANV_BUILD_TESTS "Build test suite" ON)
option(ANV_BUILD_EXAMPLES "Build example programs" OFF)
option(ANV_BUILD_DOCS "Build documentation" OFF)
option(ANV_ENABLE_SANITIZERS "Enable sanitizers in Debug builds" ON)

# Module options - let users choose what to include
option(ANV_WITH_ALGORITHMS "Include algorithms module" ON)
option(ANV_WITH_COMMON "Include common module (always required)" ON)
option(ANV_WITH_CONTAINERS "Include containers module" ON)
option(ANV_WITH_IO "Include I/O utilities module" ON)
option(ANV_WITH_MATH "Include math utilities module" ON)
option(ANV_WITH_MEMORY "Include memory utilities module" ON)
option(ANV_WITH_STRINGS "Include string utilities module" ON)
option(ANV_WITH_SYSTEM "Include system utilities module" ON)
option(ANV_WITH_TESTING "Include testing framework module" ON)

# Force common module on (required by everything else)
if(NOT ANV_WITH_COMMON)
    message(FATAL_ERROR "Common module cannot be disabled - it's required by all other modules")
endif()

# =============================================================================
# Compiler Settings
# =============================================================================

# Warning levels
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(ANV_COMPILE_FLAGS -Wall -Wextra -Wpedantic -g)
    # Additional warnings for high code quality
    list(APPEND ANV_COMPILE_FLAGS
            -Wcast-align -Wdisabled-optimization
            -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations
            -Wmissing-include-dirs -Wredundant-decls
            -Wstrict-overflow=5 -Wswitch-default -Wundef -Wno-unused-parameter
    )
elseif(MSVC)
    set(ANV_COMPILE_FLAGS /W4 /wd4996 /wd4100)
    # Disable specific warnings: deprecated functions, unused parameters
endif()

# Sanitizers for Debug builds
if(NOT WIN32 AND ANV_ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug" AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    message(STATUS "Enabling sanitizers for Debug build")
    list(APPEND ANV_COMPILE_FLAGS -fsanitize=address,undefined -fno-omit-frame-pointer)
    set(ANV_LINK_FLAGS -fsanitize=address,undefined)
endif()

# =============================================================================
# Source File Collection
# =============================================================================

# Collect source files from enabled modules
set(ANV_SOURCES "")
set(ANV_HEADERS "")
set(ANV_INCLUDE_DIRS "")

# Always include common module (required foundation)
file(GLOB_RECURSE COMMON_SOURCES "src/common/*.c")
file(GLOB_RECURSE COMMON_HEADERS "include/common/*.h")
list(APPEND ANV_SOURCES ${COMMON_SOURCES})
list(APPEND ANV_HEADERS ${COMMON_HEADERS})
list(APPEND ANV_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include/common")
message(STATUS "Including common module (foundation)")

# Algorithms module
if(ANV_WITH_ALGORITHMS)
    file(GLOB_RECURSE ALGORITHM_SOURCES "src/algorithms/*.c")
    file(GLOB_RECURSE ALGORITHM_HEADERS "include/algorithms/*.h")
    list(APPEND ANV_SOURCES ${ALGORITHM_SOURCES})
    list(APPEND ANV_HEADERS ${ALGORITHM_HEADERS})
    list(APPEND ANV_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms")
    message(STATUS "Including algorithms module")
endif()

# Containers module (core data structures)
if(ANV_WITH_CONTAINERS)
    file(GLOB_RECURSE CONTAINER_SOURCES "src/containers/*.c")
    file(GLOB_RECURSE CONTAINER_HEADERS "include/containers/*.h")
    list(APPEND ANV_SOURCES ${CONTAINER_SOURCES})
    list(APPEND ANV_HEADERS ${CONTAINER_HEADERS})
    list(APPEND ANV_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include/containers")
    message(STATUS "Including containers module")
endif()

# I/O utilities module
if(ANV_WITH_IO)
    file(GLOB_RECURSE IO_SOURCES "src/io/*.c")
    file(GLOB_RECURSE IO_HEADERS "include/io/*.h")
    list(APPEND ANV_SOURCES ${IO_SOURCES})
    list(APPEND ANV_HEADERS ${IO_HEADERS})
    list(APPEND ANV_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include/io")
    message(STATUS "Including I/O module")
endif()

# Math utilities module
if(ANV_WITH_MATH)
    file(GLOB_RECURSE MATH_SOURCES "src/math/*.c")
    file(GLOB_RECURSE MATH_HEADERS "include/math/*.h")
    list(APPEND ANV_SOURCES ${MATH_SOURCES})
    list(APPEND ANV_HEADERS ${MATH_HEADERS})
    list(APPEND ANV_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include/math")
    message(STATUS "Including math module")
endif()

# Memory utilities module
if(ANV_WITH_MEMORY)
    file(GLOB_RECURSE MEMORY_SOURCES "src/memory/*.c")
    file(GLOB_RECURSE MEMORY_HEADERS "include/memory/*.h")
    list(APPEND ANV_SOURCES ${MEMORY_SOURCES})
    list(APPEND ANV_HEADERS ${MEMORY_HEADERS})
    list(APPEND ANV_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include/memory")
    message(STATUS "Including memory module")
endif()

# String utilities module
if(ANV_WITH_STRINGS)
    file(GLOB_RECURSE STRING_SOURCES "src/strings/*.c")
    file(GLOB_RECURSE STRING_HEADERS "include/strings/*.h")
    list(APPEND ANV_SOURCES ${STRING_SOURCES})
    list(APPEND ANV_HEADERS ${STRING_HEADERS})
    list(APPEND ANV_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include/strings")
    message(STATUS "Including strings module")
endif()

# System utilities module
if(ANV_WITH_SYSTEM)
    file(GLOB_RECURSE SYSTEM_SOURCES "src/system/*.c")
    file(GLOB_RECURSE SYSTEM_HEADERS "include/system/*.h")
    list(APPEND ANV_SOURCES ${SYSTEM_SOURCES})
    list(APPEND ANV_HEADERS ${SYSTEM_HEADERS})
    list(APPEND ANV_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include/system")
    message(STATUS "Including system module")
endif()

# Testing framework module
if(ANV_WITH_TESTING)
    file(GLOB_RECURSE TESTING_SOURCES "src/testing/*.c")
    file(GLOB_RECURSE TESTING_HEADERS "include/testing/*.h")
    list(APPEND ANV_SOURCES ${TESTING_SOURCES})
    list(APPEND ANV_HEADERS ${TESTING_HEADERS})
    list(APPEND ANV_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include/testing")
    message(STATUS "Including testing module")
endif()

# Verify we have source files
if(NOT ANV_SOURCES)
    message(FATAL_ERROR "No source files found! Check that source directories exist and contain .c files")
endif()

# =============================================================================
# Library Target Creation
# =============================================================================

# Create the main library target
if(ANV_BUILD_SHARED)
    add_library(Anvil SHARED ${ANV_SOURCES})
    message(STATUS "Building shared library")

    # Set shared library properties
    set_target_properties(Anvil PROPERTIES
            VERSION ${PROJECT_VERSION}
            SOVERSION ${PROJECT_VERSION_MAJOR}
    )

    if(WIN32)
        add_custom_command(TARGET Anvil POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:Anvil>
                ${CMAKE_BINARY_DIR}/tests/
                COMMENT "Copying Anvil.dll to tests directory for Windows"
        )
    endif()
else()
    add_library(Anvil STATIC ${ANV_SOURCES})
    message(STATUS "Building static library")
endif()

# Create alias for consistent naming
add_library(Anvil::Anvil ALIAS Anvil)

# =============================================================================
# Target Configuration
# =============================================================================

# Include directories
target_include_directories(Anvil PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Private include directories for internal use
target_include_directories(Anvil PRIVATE
        ${ANV_INCLUDE_DIRS}
)

# Compiler options
target_compile_options(Anvil PRIVATE ${ANV_COMPILE_FLAGS})

# Link options (for sanitizers)
if(ANV_LINK_FLAGS)
    target_link_libraries(Anvil PUBLIC ${ANV_LINK_FLAGS})
endif()

# Platform-specific definitions and libraries
if(WIN32)
    target_compile_definitions(Anvil PUBLIC ANV_PLATFORM_WINDOWS)
    if(ANV_BUILD_SHARED)
        target_compile_definitions(Anvil PRIVATE ANV_BUILDING_DLL)
        target_compile_definitions(Anvil INTERFACE ANV_USING_DLL)
    endif()
elseif(APPLE)
    target_compile_definitions(Anvil PUBLIC ANV_PLATFORM_MACOS)
elseif(UNIX)
    target_compile_definitions(Anvil PUBLIC ANV_PLATFORM_LINUX)
    # Link threading library on Unix-like systems
    find_package(Threads REQUIRED)
    target_link_libraries(Anvil PRIVATE Threads::Threads)
endif()

# Version information
target_compile_definitions(Anvil PUBLIC
        ANV_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        ANV_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        ANV_VERSION_PATCH=${PROJECT_VERSION_PATCH}
        ANV_VERSION_STRING="${PROJECT_VERSION}"
)

# Debug/Release definitions
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(Anvil PUBLIC ANV_DEBUG)
else()
    target_compile_definitions(Anvil PUBLIC ANV_RELEASE)
endif()

# =============================================================================
# Testing
# =============================================================================

if(ANV_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
    message(STATUS "Building test suite")
endif()

# =============================================================================
# Examples
# =============================================================================

if(ANV_BUILD_EXAMPLES)
    add_subdirectory(examples)
    message(STATUS "Building examples")
endif()

# =============================================================================
# Documentation
# =============================================================================

if(ANV_BUILD_DOCS)
    find_package(Doxygen QUIET)
    if(DOXYGEN_FOUND)
        add_subdirectory(docs)
        message(STATUS "Building documentation")
    else()
        message(WARNING "Doxygen not found, cannot build documentation")
    endif()
endif()

# =============================================================================
# Installation
# =============================================================================

include(GNUInstallDirs)

# Install all headers preserving directory structure
install(DIRECTORY include/
        DESTINATION include
        FILES_MATCHING PATTERN "*.h"
)

# Install library
install(TARGETS Anvil
        EXPORT AnvilTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
)

# Install CMake config files for find_package() support
install(EXPORT AnvilTargets
        FILE AnvilTargets.cmake
        NAMESPACE Anvil::
        DESTINATION lib/cmake/Anvil
)

# Create package config files
include(CMakePackageConfigHelpers)

# Version file
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/AnvilConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)

# Config file
configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/AnvilConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/AnvilConfig.cmake"
        INSTALL_DESTINATION lib/cmake/Anvil
        PATH_VARS CMAKE_INSTALL_INCLUDEDIR
)

# Install config files
install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/AnvilConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/AnvilConfigVersion.cmake"
        DESTINATION lib/cmake/Anvil
)

# Create pkg-config file for traditional Unix builds
if(UNIX)
    configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/anvil.pc.in"
            "${CMAKE_CURRENT_BINARY_DIR}/anvil.pc"
            @ONLY
    )
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/anvil.pc"
            DESTINATION lib/pkgconfig
    )
endif()

# =============================================================================
# Development Tools
# =============================================================================

# Generate compile_commands.json for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Custom targets for development
add_custom_target(format
        COMMAND find ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/include
        -name "*.c" -o -name "*.h" | xargs clang-format -i
        COMMENT "Formatting source code with clang-format"
        VERBATIM
)

add_custom_target(analyze
        COMMAND clang-tidy ${ANV_SOURCES} -- -I${CMAKE_CURRENT_SOURCE_DIR}/include
        COMMENT "Running static analysis with clang-tidy"
        VERBATIM
)

# =============================================================================
# Status Summary
# =============================================================================

message(STATUS "")
message(STATUS "==============================================================================")
message(STATUS "🔨 Anvil Library Configuration Summary")
message(STATUS "==============================================================================")
message(STATUS "Version:              ${PROJECT_VERSION}")
message(STATUS "Build type:           ${CMAKE_BUILD_TYPE}")
if(ANV_BUILD_SHARED)
    message(STATUS "Library type:         Shared (.so/.dll/.dylib)")
else()
    message(STATUS "Library type:         Static (.a/.lib)")
endif()
message(STATUS "C Standard:           C${CMAKE_C_STANDARD}")
message(STATUS "Compiler:             ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")

message(STATUS "")
message(STATUS "Enabled modules:")
message(STATUS "  Common (required):    ${ANV_WITH_COMMON}")
message(STATUS "  Containers:           ${ANV_WITH_CONTAINERS}")
message(STATUS "  Algorithms:           ${ANV_WITH_ALGORITHMS}")
message(STATUS "  Strings:              ${ANV_WITH_STRINGS}")
message(STATUS "  I/O:                  ${ANV_WITH_IO}")
message(STATUS "  System:               ${ANV_WITH_SYSTEM}")
message(STATUS "  Math:                 ${ANV_WITH_MATH}")

message(STATUS "")
message(STATUS "Build options:")
message(STATUS "  Tests:                ${ANV_BUILD_TESTS}")
message(STATUS "  Examples:             ${ANV_BUILD_EXAMPLES}")
message(STATUS "  Documentation:        ${ANV_BUILD_DOCS}")
message(STATUS "  Sanitizers:           ${ANV_ENABLE_SANITIZERS}")

message(STATUS "")
message(STATUS "Source files found:   ${CMAKE_CURRENT_LIST_LENGTH} files")
message(STATUS "Install prefix:       ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
message(STATUS "To build:")
message(STATUS "  make -j$(nproc)")
message(STATUS "")
message(STATUS "To test:")
message(STATUS "  make test")
message(STATUS "==============================================================================")
message(STATUS "")