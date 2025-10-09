//
// Created by GitHub Copilot on 9/11/25.
//

#include "anvil/common.h"
#include "TestAssert.h"
#include "TestHelpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//==============================================================================
// Arena Allocator Implementation
//==============================================================================

typedef struct
{
    char* memory;
    size_t size;
    size_t used;
} Arena;

static Arena arena_global = {0};

static void* arena_alloc(const size_t size)
{
    if (!arena_global.memory)
    {
        return NULL;
    }

    // Align to 8-byte boundary
    const size_t aligned_size = (size + 7) & ~7;

    if (arena_global.used + aligned_size > arena_global.size)
    {
        return NULL; // Out of memory
    }

    void* ptr = arena_global.memory + arena_global.used;
    arena_global.used += aligned_size;
    return ptr;
}

static void arena_free(void* ptr)
{
    // Arena allocator doesn't free individual allocations
    (void)ptr;
}

static void arena_reset(void)
{
    arena_global.used = 0;
}

static int arena_init(size_t size)
{
    arena_global.memory = malloc(size);
    if (!arena_global.memory)
    {
        return -1;
    }
    arena_global.size = size;
    arena_global.used = 0;
    return 0;
}

static void arena_destroy(void)
{
    free(arena_global.memory);
    arena_global.memory = NULL;
    arena_global.size = 0;
    arena_global.used = 0;
}

//==============================================================================
// Static Stack Allocator Implementation
//==============================================================================

#define STACK_SIZE 4096
static char stack_memory[STACK_SIZE];
static size_t stack_top = 0;

static void* stack_alloc(const size_t size)
{
    // Align to 8-byte boundary
    const size_t aligned_size = (size + 7) & ~7;

    if (stack_top + aligned_size > STACK_SIZE)
    {
        return NULL; // Stack overflow
    }

    void* ptr = stack_memory + stack_top;
    stack_top += aligned_size;
    return ptr;
}

static void stack_free(void* ptr)
{
    // Simple stack allocator only supports LIFO deallocation
    if (!ptr)
        return;

    const char* char_ptr = ptr;
    if (char_ptr >= stack_memory && char_ptr < stack_memory + STACK_SIZE)
    {
        // Only free if it's the top allocation
        if (char_ptr < stack_memory + stack_top)
        {
            stack_top = char_ptr - stack_memory;
        }
    }
}

static void stack_reset(void)
{
    stack_top = 0;
}

//==============================================================================
// Counting Allocator (for testing)
//==============================================================================

static size_t alloc_count = 0;
static size_t free_count = 0;

static void* counting_alloc(const size_t size)
{
    alloc_count++;
    return malloc(size);
}

static void counting_free(void* ptr)
{
    if (ptr)
    {
        free_count++;
        free(ptr);
    }
}

static void reset_counters(void)
{
    alloc_count = 0;
    free_count = 0;
}

static void* counting_int_copy(const void* data)
{
    const int* original = data;
    int* copy = counting_alloc(sizeof(int));
    if (copy)
    {
        *copy = *original;
    }
    return copy;
}

//==============================================================================
// Test Functions
//==============================================================================

int test_default_allocator(void)
{
    const ANVAllocator alloc = anv_alloc_default();

    // Test allocation
    void* ptr = anv_alloc_malloc(&alloc, 100);
    ASSERT(ptr != NULL);

    // Test copy (should return same pointer for default)
    const int value = 42;
    const void* copied = anv_alloc_copy(&alloc, &value);
    ASSERT(copied == &value);

    // Test free
    anv_alloc_free(&alloc, ptr);

    return TEST_SUCCESS;
}

int test_arena_allocator(void)
{
    // Initialize arena
    ASSERT_EQ(arena_init(1024), 0);

    const ANVAllocator alloc = anv_alloc_custom(arena_alloc, arena_free, NULL, NULL);

    // Test multiple allocations
    const void* ptr1 = anv_alloc_malloc(&alloc, 64);
    ASSERT(ptr1 != NULL);

    const void* ptr2 = anv_alloc_malloc(&alloc, 128);
    ASSERT(ptr2 != NULL);

    const void* ptr3 = anv_alloc_malloc(&alloc, 256);
    ASSERT(ptr3 != NULL);

    // Verify pointers are different and ordered
    ASSERT(ptr1 != ptr2);
    ASSERT(ptr2 != ptr3);
    ASSERT(ptr1 < ptr2);
    ASSERT(ptr2 < ptr3);

    // Test allocation failure when out of memory
    const void* big_ptr = anv_alloc_malloc(&alloc, 1024);
    ASSERT(big_ptr == NULL);

    // Test reset and reuse
    arena_reset();
    const void* ptr4 = anv_alloc_malloc(&alloc, 64);
    ASSERT(ptr4 == ptr1); // Should reuse same memory

    arena_destroy();
    return TEST_SUCCESS;
}

int test_stack_allocator(void)
{
    stack_reset();

    const ANVAllocator alloc = anv_alloc_custom(stack_alloc, stack_free, NULL, NULL);

    // Test allocation
    const void* ptr1 = anv_alloc_malloc(&alloc, 64);
    ASSERT(ptr1 != NULL);

    void* ptr2 = anv_alloc_malloc(&alloc, 128);
    ASSERT(ptr2 != NULL);

    // Test LIFO deallocation
    anv_alloc_free(&alloc, ptr2);
    const void* ptr3 = anv_alloc_malloc(&alloc, 100);
    ASSERT(ptr3 == ptr2); // Should reuse freed space

    // Test stack overflow
    const void* big_ptr = anv_alloc_malloc(&alloc, STACK_SIZE);
    ASSERT(big_ptr == NULL);

    stack_reset();
    return TEST_SUCCESS;
}

int test_counting_allocator(void)
{
    reset_counters();

    const ANVAllocator alloc = anv_alloc_custom(counting_alloc, counting_free, counting_free, counting_int_copy);

    // Test allocations are counted
    void* ptr1 = anv_alloc_malloc(&alloc, 64);
    ASSERT(ptr1 != NULL);
    ASSERT_EQ(alloc_count, 1);
    ASSERT_EQ(free_count, 0);

    void* ptr2 = anv_alloc_malloc(&alloc, 128);
    ASSERT(ptr2 != NULL);
    ASSERT_EQ(alloc_count, 2);
    ASSERT_EQ(free_count, 0);

    // Test copy function (should allocate new memory)
    const int value = 42;
    void* copied = anv_alloc_copy(&alloc, &value);
    ASSERT(copied != NULL);
    ASSERT(copied != &value);
    ASSERT_EQ(*(int*)copied, 42);
    ASSERT_EQ(alloc_count, 3); // Copy should trigger allocation

    // Test frees are counted
    anv_alloc_free(&alloc, ptr1);
    ASSERT_EQ(free_count, 1);

    anv_alloc_free(&alloc, ptr2);
    ASSERT_EQ(free_count, 2);

    anv_alloc_data_free(&alloc, copied);
    ASSERT_EQ(free_count, 3);

    return TEST_SUCCESS;
}

int test_custom_anv_copy_functions(void)
{
    const ANVAllocator int_alloc = anv_alloc_custom(malloc, free, free, int_copy);
    const ANVAllocator str_alloc = anv_alloc_custom(malloc, free, free, string_copy);

    // Test integer deep copy
    const int original_int = 123;
    int* copied_int = anv_alloc_copy(&int_alloc, &original_int);
    ASSERT(copied_int != NULL);
    ASSERT(copied_int != &original_int);
    ASSERT_EQ(*copied_int, 123);
    anv_alloc_data_free(&int_alloc, copied_int);

    // Test string deep copy
    const char* original_str = "Hello, World!";
    char* copied_str = anv_alloc_copy(&str_alloc, original_str);
    ASSERT(copied_str != NULL);
    ASSERT(copied_str != original_str);
    ASSERT_EQ_STR(copied_str, "Hello, World!");
    anv_alloc_data_free(&str_alloc, copied_str);

    return TEST_SUCCESS;
}

int test_allocator_edge_cases(void)
{
    const ANVAllocator alloc = anv_alloc_default();

    // Test NULL pointer handling
    anv_alloc_free(&alloc, NULL); // Should not crash

    const void* null_copy = anv_alloc_copy(&alloc, NULL);
    ASSERT(null_copy == NULL);

    // Test zero-size allocation
    void* zero_ptr = anv_alloc_malloc(&alloc, 0);
    // Behavior is implementation-defined, but shouldn't crash
    anv_alloc_free(&alloc, zero_ptr);

    // Test NULL allocator
    const void* null_alloc_ptr = anv_alloc_malloc(NULL, 100);
    ASSERT(null_alloc_ptr == NULL);

    return TEST_SUCCESS;
}

int test_allocator_with_null_functions(void)
{
    // Test allocator with NULL copy and data_free functions
    const ANVAllocator alloc = anv_alloc_custom(malloc, free, NULL, NULL);

    void* ptr = anv_alloc_malloc(&alloc, 64);
    ASSERT(ptr != NULL);

    // data_free with NULL function should not crash
    anv_alloc_data_free(&alloc, ptr);

    // copy with NULL function should use default copy (return same pointer)
    const int value = 42;
    const void* copied = anv_alloc_copy(&alloc, &value);
    ASSERT(copied == &value); // Default copy returns original pointer

    anv_alloc_free(&alloc, ptr);

    return TEST_SUCCESS;
}

int test_arena_memory_alignment(void)
{
    ASSERT_EQ(arena_init(1024), 0);

    const ANVAllocator alloc = anv_alloc_custom(arena_alloc, arena_free, NULL, NULL);

    // Test that allocations are properly aligned
    void* ptr1 = anv_alloc_malloc(&alloc, 1);
    void* ptr2 = anv_alloc_malloc(&alloc, 1);

    ASSERT(ptr1 != NULL);
    ASSERT(ptr2 != NULL);

    // Check 8-byte alignment
    const uintptr_t addr1 = (uintptr_t)ptr1;
    const uintptr_t addr2 = (uintptr_t)ptr2;

    ASSERT_EQ(addr1 % 8, 0);
    ASSERT_EQ(addr2 % 8, 0);
    ASSERT_EQ(addr2 - addr1, 8); // Should be 8 bytes apart for 1-byte allocations

    arena_destroy();
    return TEST_SUCCESS;
}

int test_stack_allocator_lifo_behavior(void)
{
    stack_reset();

    const ANVAllocator alloc = anv_alloc_custom(stack_alloc, stack_free, NULL, NULL);

    // Allocate in order
    const void* ptr1 = anv_alloc_malloc(&alloc, 64);
    void* ptr2 = anv_alloc_malloc(&alloc, 64);
    void* ptr3 = anv_alloc_malloc(&alloc, 64);

    ASSERT(ptr1 != NULL);
    ASSERT(ptr2 != NULL);
    ASSERT(ptr3 != NULL);

    // Free in LIFO order (last allocated first)
    anv_alloc_free(&alloc, ptr3);

    // Next allocation should reuse ptr3's space
    void* ptr4 = anv_alloc_malloc(&alloc, 64);
    ASSERT(ptr4 == ptr3);

    // Free ptr4 and ptr2 (not in LIFO order for ptr2)
    anv_alloc_free(&alloc, ptr4);
    anv_alloc_free(&alloc, ptr2);

    // New allocation should still work
    const void* ptr5 = anv_alloc_malloc(&alloc, 64);
    ASSERT(ptr5 == ptr2);

    stack_reset();
    return TEST_SUCCESS;
}

//==============================================================================
// Main test runner
//==============================================================================

int main(void)
{
    int tests_passed = 0;
    int tests_total = 0;

    printf("Running custom allocator tests...\n\n");

    // Run all tests
    const struct
    {
        const char* name;
        int (*test_func)(void);
    } tests[] = {
            {"Default Allocator", test_default_allocator},
            {"Arena Allocator", test_arena_allocator},
            {"Stack Allocator", test_stack_allocator},
            {"Counting Allocator", test_counting_allocator},
            {"Custom Copy Functions", test_custom_anv_copy_functions},
            {"Allocator Edge Cases", test_allocator_edge_cases},
            {"Allocator with NULL Functions", test_allocator_with_null_functions},
            {"Arena Memory Alignment", test_arena_memory_alignment},
            {"Stack LIFO Behavior", test_stack_allocator_lifo_behavior}
        };

    const int num_tests = sizeof(tests) / sizeof(tests[0]);

    for (int i = 0; i < num_tests; i++)
    {
        printf("Test %d: %s... ", i + 1, tests[i].name);
        fflush(stdout);

        const int result = tests[i].test_func();
        tests_total++;

        if (result == TEST_SUCCESS)
        {
            printf("PASSED\n");
            tests_passed++;
        }
        else if (result == TEST_FAILURE)
        {
            printf("FAILED\n");
        }
        else
        {
            printf("SKIPPED\n");
        }
    }

    printf("\n=== Test Results ===\n");
    printf("Tests passed: %d/%d\n", tests_passed, tests_total);
    printf("Success rate: %.1f%%\n",
           tests_total > 0 ? (100.0 * tests_passed / tests_total) : 0.0);

    return (tests_passed == tests_total) ? 0 : 1;
}