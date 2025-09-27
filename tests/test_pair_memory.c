//
// Created by zack on 9/15/25.
//

#include "containers/pair.h"
#include "TestAssert.h"
#include "TestHelpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper copy functions for testing
static void* int_anv_copy_func(const void* data)
{
    int* copy = malloc(sizeof(int));
    if (copy)
    {
        *copy = *(const int*)data;
    }
    return copy;
}

static void* string_anv_copy_func(const void* data)
{
    const char* str = (const char*)data;
    size_t len = strlen(str) + 1;
    char* copy = malloc(len);
    if (copy)
    {
        strcpy(copy, str);
    }
    return copy;
}

// Helper function that always fails for testing
static void* failing_anv_copy_func(const void* data)
{
    (void)data;
    return NULL; // Always fail
}

int test_pair_memory_allocation_failure(void)
{
    // Test with failing allocator
    set_alloc_fail_countdown(0); // Fail immediately on first allocation
    ANVAllocator failing_alloc = create_failing_int_allocator();

    ANVPair* pair = anv_pair_create(&failing_alloc, NULL, NULL);
    ASSERT_NULL(pair); // Should fail to allocate pair structure

    return TEST_SUCCESS;
}

int test_pair_copy_deep_allocation_failure(void)
{
    ANVAllocator normal_alloc = create_int_allocator();
    const ANVAllocator failing_alloc = create_failing_int_allocator();

    int* first = malloc(sizeof(int));
    int* second = malloc(sizeof(int));
    *first = 42;
    *second = 84;

    ANVPair* original = anv_pair_create(&normal_alloc, first, second);

    // Test failure during first element copy by using a failing copy function
    set_alloc_fail_countdown(1); // Allocate the Pair structure then fail on the first copy
    ANVPair* copy1 = anv_pair_copy_deep(original, true, failing_anv_copy_func, normal_alloc.copy);
    ASSERT_NULL(copy1);

    // Test failure during second element copy by using a failing copy function
    set_alloc_fail_countdown(1); // Allocate the Pair structure then fail on the second copy
    ANVPair* copy2 = anv_pair_copy_deep(original, true, normal_alloc.copy, failing_anv_copy_func);
    ASSERT_NULL(copy2);

    anv_pair_destroy(original, true, true);
    return TEST_SUCCESS;
}

int test_pair_destroy_null_safe(void)
{
    // Test that destroying NULL pair doesn't crash
    anv_pair_destroy(NULL, true, true);
    anv_pair_destroy(NULL, false, false);

    return TEST_SUCCESS;
}

int test_pair_memory_leak_prevention(void)
{
    ANVAllocator alloc = create_int_allocator();

    int* first = malloc(sizeof(int));
    int* second = malloc(sizeof(int));
    int* new_first = malloc(sizeof(int));
    int* new_second = malloc(sizeof(int));

    *first = 42;
    *second = 84;
    *new_first = 100;
    *new_second = 200;

    ANVPair* pair = anv_pair_create(&alloc, first, second);

    // Test that setting new values with should_free_old=true prevents leaks
    anv_pair_set_first(pair, new_first, true);   // Should free old first
    anv_pair_set_second(pair, new_second, true); // Should free old second

    // Clean up
    anv_pair_destroy(pair, true, true);

    return TEST_SUCCESS;
}

int test_pair_selective_memory_management(void)
{
    ANVAllocator alloc = create_int_allocator();

    int* first = malloc(sizeof(int));
    int* second = malloc(sizeof(int));
    *first = 42;
    *second = 84;

    ANVPair* pair = anv_pair_create(&alloc, first, second);

    // Test destroying with selective memory management
    // Free only first element
    anv_pair_destroy(pair, true, false);

    // Manual cleanup of second (since we didn't free it)
    free(second);

    return TEST_SUCCESS;
}

int test_pair_copy_deep_with_different_anv_copy_functions(void)
{
    ANVAllocator alloc = create_int_allocator();

    int* first = malloc(sizeof(int));
    char* second = malloc(20);
    *first = 42;
    strcpy(second, "hello world");

    ANVPair* original = anv_pair_create(&alloc, first, second);

    // Test deep copy with different copy functions for each element
    ANVPair* deep_copy = anv_pair_copy_deep(original, true, int_anv_copy_func, string_anv_copy_func);
    ASSERT_NOT_NULL(deep_copy);
    ASSERT_NOT_EQ_PTR(deep_copy->first, original->first);
    ASSERT_NOT_EQ_PTR(deep_copy->second, original->second);
    ASSERT_EQ(*(int*)deep_copy->first, 42);
    ASSERT_EQ(strcmp((char*)deep_copy->second, "hello world"), 0);

    anv_pair_destroy(original, true, true);
    anv_pair_destroy(deep_copy, true, true);
    return TEST_SUCCESS;
}

int test_pair_large_data_handling(void)
{
    ANVAllocator alloc = create_string_allocator();

    // Create large strings
    size_t size = 10000;
    char* large_str1 = malloc(size);
    char* large_str2 = malloc(size);

    // Fill with data
    for (size_t i = 0; i < size - 1; i++)
    {
        large_str1[i] = 'A' + (i % 26);
        large_str2[i] = 'a' + (i % 26);
    }
    large_str1[size - 1] = '\0';
    large_str2[size - 1] = '\0';

    ANVPair* pair = anv_pair_create(&alloc, large_str1, large_str2);
    ASSERT_NOT_NULL(pair);

    // Verify data integrity
    ASSERT_EQ(strncmp((char*)pair->first, large_str1, 100), 0);  // Check first 100 chars
    ASSERT_EQ(strncmp((char*)pair->second, large_str2, 100), 0); // Check first 100 chars

    // Test deep copy
    ANVPair* deep_copy = anv_pair_copy_deep(pair, true, alloc.copy, alloc.copy);
    ASSERT_NOT_NULL(deep_copy);
    ASSERT_EQ(strncmp((char*)deep_copy->first, large_str1, 100), 0);  // Check first 100 chars
    ASSERT_EQ(strncmp((char*)deep_copy->second, large_str2, 100), 0); // Check first 100 chars

    // Clean up
    anv_pair_destroy(pair, true, true);
    anv_pair_destroy(deep_copy, true, true);

    return TEST_SUCCESS;
}

int test_pair_multiple_operations_memory_safety(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create multiple pairs and perform various operations
    ANVPair* pairs[10];

    for (int i = 0; i < 10; i++)
    {
        int* first = malloc(sizeof(int));
        int* second = malloc(sizeof(int));
        *first = i * 10;
        *second = i * 20;
        pairs[i] = anv_pair_create(&alloc, first, second);
        ASSERT_NOT_NULL(pairs[i]);
    }

    // Perform swaps on all pairs
    for (int i = 0; i < 10; i++)
    {
        anv_pair_swap(pairs[i]);
        ASSERT_EQ(*(int*)pairs[i]->first, i * 20);
        ASSERT_EQ(*(int*)pairs[i]->second, i * 10);
    }

    // Create copies
    ANVPair* copies[10];
    for (int i = 0; i < 10; i++)
    {
        copies[i] = anv_pair_copy_deep(pairs[i], true, alloc.copy, alloc.copy);
        ASSERT_NOT_NULL(copies[i]);
    }

    // Clean up all pairs
    for (int i = 0; i < 10; i++)
    {
        anv_pair_destroy(pairs[i], true, true);
        anv_pair_destroy(copies[i], true, true);
    }

    return TEST_SUCCESS;
}

int test_pair_edge_case_null_elements(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Test pair with both elements NULL
    ANVPair* pair = anv_pair_create(&alloc, NULL, NULL);
    ASSERT_NOT_NULL(pair);
    ASSERT_NULL(pair->first);
    ASSERT_NULL(pair->second);

    // Test operations on NULL elements
    ASSERT_NULL(anv_pair_first(pair));
    ASSERT_NULL(anv_pair_second(pair));

    // Test swap with NULL elements
    anv_pair_swap(pair);
    ASSERT_NULL(pair->first);
    ASSERT_NULL(pair->second);

    // Test setting non-NULL values
    int* value1 = malloc(sizeof(int));
    int* value2 = malloc(sizeof(int));
    *value1 = 42;
    *value2 = 84;

    anv_pair_set_first(pair, value1, false); // No old value to free
    anv_pair_set_second(pair, value2, false);

    ASSERT_EQ(*(int*)pair->first, 42);
    ASSERT_EQ(*(int*)pair->second, 84);

    anv_pair_destroy(pair, true, true);
    return TEST_SUCCESS;
}

int main(void)
{
    printf("Running Pair memory tests...\n");

    int (*tests[])(void) = {
        test_pair_memory_allocation_failure,
        test_pair_copy_deep_allocation_failure,
        test_pair_destroy_null_safe,
        test_pair_memory_leak_prevention,
        test_pair_selective_memory_management,
        test_pair_copy_deep_with_different_anv_copy_functions,
        test_pair_large_data_handling,
        test_pair_multiple_operations_memory_safety,
        test_pair_edge_case_null_elements,
    };

    const char* test_names[] = {
        "test_pair_memory_allocation_failure",
        "test_pair_copy_deep_allocation_failure",
        "test_pair_destroy_null_safe",
        "test_pair_memory_leak_prevention",
        "test_pair_selective_memory_management",
        "test_pair_copy_deep_with_different_anv_copy_functions",
        "test_pair_large_data_handling",
        "test_pair_multiple_operations_memory_safety",
        "test_pair_edge_case_null_elements",
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int passed = 0;

    for (int i = 0; i < num_tests; i++)
    {
        printf("Running %s... ", test_names[i]);
        if (tests[i]() == TEST_SUCCESS)
        {
            printf("PASSED\n");
            passed++;
        }
        else
        {
            printf("FAILED\n");
        }
    }

    printf("Pair memory tests: %d/%d passed\n", passed, num_tests);
    return (passed == num_tests) ? 0 : 1;
}