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
option(ANV_BUILD_TESTS "Build test suite" OFF)
option(ANV_BUILD_EXAMPLES "Build example programs" ON)
option(ANV_BUILD_DOCS "Build documentation" OFF)
option(ANV_ENABLE_SANITIZERS "Enable sanitizers in Debug builds" ON)

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
    set(ANV_LINK_FLAGS -fsanitize=address,undefined -lm)
endif()

# =============================================================================
# Source File Collection
# =============================================================================

# Collect source files from enabled modules
set(ANV_SOURCES
        src/common/allocator.c
        src/common/result.c
        src/common/version.c
        src/algorithms/hash.c
        src/containers/arraylist.c
        src/containers/binarysearchtree.c
        src/containers/doublylinkedlist.c
        src/containers/dynamicstring.c
        src/containers/hashmap.c
        src/containers/hashset.c
        src/containers/iterator.c
        src/containers/pair.c
        src/containers/queue.c
        src/containers/singlylinkedlist.c
        src/containers/stack.c
        src/system/mutex.c
        src/system/thread.c
        src/system/timing.c
#        src/testing/assertions.c
        src/testing/benchmark.c
#        src/testing/suite.c
)

set(ANV_INCLUDE_DIRS
        "include/anvil/algorithms"
        "include/anvil/common"
        "include/anvil/containers"
        "include/anvil/io"
        "include/anvil/math"
        "include/anvil/memory"
        "include/anvil/strings"
        "include/anvil/system"
        "include/anvil/testing"
)

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
    enable_testing()
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
message(STATUS "Build options:")
message(STATUS "  Tests:                ${ANV_BUILD_TESTS}")
message(STATUS "  Examples:             ${ANV_BUILD_EXAMPLES}")
message(STATUS "  Documentation:        ${ANV_BUILD_DOCS}")
message(STATUS "  Sanitizers:           ${ANV_ENABLE_SANITIZERS}")

list(LENGTH ANV_SOURCES CMAKE_CURRENT_LIST_LENGTH)

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