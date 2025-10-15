cmake_minimum_required(VERSION 3.24)
project(Anvil
        LANGUAGES C
        VERSION 0.1.2
        DESCRIPTION "Personal Standard Library - Useful tools for building C applications"
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# =============================================================================
# Build Configuration
# =============================================================================

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

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(ANV_COMPILE_FLAGS -Wall -Wextra -Wpedantic -g)
elseif(MSVC)
    set(ANV_COMPILE_FLAGS /W4 /wd4996 /wd4100)
endif()

set(ANV_LINK_FLAGS)
if(NOT MSVC)
    list(APPEND ANV_LINK_FLAGS -lm)
endif()

# Sanitizers for Debug builds
if(NOT WIN32 AND ANV_ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug" AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    message(STATUS "Enabling sanitizers for Debug build")
    list(APPEND ANV_COMPILE_FLAGS -fsanitize=address,undefined -fno-omit-frame-pointer)
    list(APPEND ANV_LINK_FLAGS -fsanitize=address,undefined)
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
        src/testing/benchmark.c
        src/io/file.c
        src/memory/arena.c
        src/memory/stack_frame.c
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
else()
    add_library(Anvil STATIC ${ANV_SOURCES})
    message(STATUS "Building static library")
endif()

if(WIN32 AND ANV_BUILD_TESTS)
    add_custom_command(TARGET Anvil POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Anvil>
            ${CMAKE_BINARY_DIR}/tests/
            COMMENT "Copying Anvil to tests directory for Windows"
    )
endif()

if(WIN32 AND ANV_BUILD_EXAMPLES)
    add_custom_command(TARGET Anvil POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Anvil>
            ${CMAKE_BINARY_DIR}/examples/
            COMMENT "Copying Anvil to examples directory for Windows"
    )
endif()

add_library(Anvil::Anvil ALIAS Anvil)

# =============================================================================
# Target Configuration
# =============================================================================

target_include_directories(Anvil PRIVATE
        ${ANV_INCLUDE_DIRS}
)

target_compile_options(Anvil PRIVATE ${ANV_COMPILE_FLAGS})

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
#    enable_testing()
#    add_subdirectory(tests)
#    message(STATUS "Building test suite")
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

# Include directories
target_include_directories(Anvil PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)


# Install all headers preserving directory structure
install(DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h"
)

# Install library
install(TARGETS Anvil
        EXPORT AnvilTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install CMake config files for find_package() support
install(EXPORT AnvilTargets
        FILE AnvilTargets.cmake
        NAMESPACE Anvil::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Anvil
)

# Create package config files
include(CMakePackageConfigHelpers)

# Config file
configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/AnvilConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/AnvilConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Anvil
)

# Install config files
install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/AnvilConfig.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Anvil
)

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