//
// Created by zack on 9/15/25.
//
// Comprehensive test suite for take iterator functionality.
// Tests cover basic iteration, edge cases, limit handling,
// error handling, and composition with other iterators.

#include "containers/iterator.h"
#include "TestAssert.h"
#include "TestHelpers.h"

#include <stdio.h>
#include <stdlib.h>

//==============================================================================
// Helper Functions for Take Iterator Tests
//==============================================================================

/**
 * Helper function to collect all values from an iterator into an array.
 * Returns the number of values collected.
 */
static int collect_values(const ANVIterator* it, int* values, const int max_count)
{
    int count = 0;
    while (it->has_next(it) && count < max_count)
    {
        const int* value = it->get(it);
        if (value)
        {
            values[count++] = *value;
        }
        it->next(it);
    }
    return count;
}

/**
 * Helper function to verify an array matches expected values.
 */
static int verify_values(const int* actual, const int* expected, const int count, const char* test_name)
{
    for (int i = 0; i < count; i++)
    {
        if (actual[i] != expected[i])
        {
            printf("FAIL: %s - Expected %d at index %d, got %d\n",
                   test_name, expected[i], i, actual[i]);
            return 0;
        }
    }
    return 1;
}

//==============================================================================
// Basic Take Iterator Tests
//==============================================================================

int test_take_basic_functionality(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator 1-10
    ANVIterator range_it = anv_iterator_range(1, 11, 1, &alloc);

    // Take first 5 elements
    ANVIterator take_it = anv_iterator_take(&range_it, &alloc, 5);
    ASSERT_TRUE(take_it.is_valid(&take_it));

    int values[10];
    const int count = collect_values(&take_it, values, 10);

    ASSERT_EQ(count, 5);

    const int expected[] = {1, 2, 3, 4, 5};
    ASSERT_TRUE(verify_values(values, expected, 5, "take_basic"));

    take_it.destroy(&take_it);
    return TEST_SUCCESS;
}

int test_take_zero_count(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator 1-10
    ANVIterator range_it = anv_iterator_range(1, 11, 1, &alloc);

    // Take 0 elements
    ANVIterator take_it = anv_iterator_take(&range_it, &alloc, 0);
    ASSERT_TRUE(take_it.is_valid(&take_it));

    // Should have no elements
    ASSERT_FALSE(take_it.has_next(&take_it));
    ASSERT_NULL(take_it.get(&take_it));

    take_it.destroy(&take_it);
    return TEST_SUCCESS;
}

int test_take_more_than_available(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator with only 3 elements
    ANVIterator range_it = anv_iterator_range(1, 4, 1, &alloc);

    // Try to take 10 elements (more than available)
    ANVIterator take_it = anv_iterator_take(&range_it, &alloc, 10);
    ASSERT_TRUE(take_it.is_valid(&take_it));

    int values[10];
    const int count = collect_values(&take_it, values, 10);

    ASSERT_EQ(count, 3);  // Should only get 3 elements

    const int expected[] = {1, 2, 3};
    ASSERT_TRUE(verify_values(values, expected, 3, "take_more_than_available"));

    take_it.destroy(&take_it);
    return TEST_SUCCESS;
}

int test_take_single_element(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator 1-10
    ANVIterator range_it = anv_iterator_range(1, 11, 1, &alloc);

    // Take only 1 element
    ANVIterator take_it = anv_iterator_take(&range_it, &alloc, 1);
    ASSERT_TRUE(take_it.is_valid(&take_it));

    ASSERT_TRUE(take_it.has_next(&take_it));
    const int* value = take_it.get(&take_it);
    ASSERT_NOT_NULL(value);
    ASSERT_EQ(*value, 1);

    ASSERT_EQ(take_it.next(&take_it), 0);

    // Should be no more elements
    ASSERT_FALSE(take_it.has_next(&take_it));
    ASSERT_NULL(take_it.get(&take_it));

    take_it.destroy(&take_it);
    return TEST_SUCCESS;
}

//==============================================================================
// Edge Cases and Error Handling
//==============================================================================

int test_take_empty_source(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create empty range iterator
    ANVIterator range_it = anv_iterator_range(1, 1, 1, &alloc);  // Empty range

    // Try to take 5 elements from empty iterator
    ANVIterator take_it = anv_iterator_take(&range_it, &alloc, 5);
    ASSERT_TRUE(take_it.is_valid(&take_it));

    // Should have no elements
    ASSERT_FALSE(take_it.has_next(&take_it));
    ASSERT_NULL(take_it.get(&take_it));

    take_it.destroy(&take_it);
    return TEST_SUCCESS;
}

int test_take_invalid_parameters(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Test with NULL iterator
    const ANVIterator take_it1 = anv_iterator_take(NULL, &alloc, 5);
    ASSERT_FALSE(take_it1.is_valid(&take_it1));

    // Test with NULL allocator
    ANVIterator range_it = anv_iterator_range(1, 11, 1, &alloc);

    const ANVIterator take_it2 = anv_iterator_take(&range_it, NULL, 5);
    ASSERT_FALSE(take_it2.is_valid(&take_it2));

    // Clean up the range iterator manually since take failed
    range_it.destroy(&range_it);

    return TEST_SUCCESS;
}

int test_take_large_count(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator 1-5
    ANVIterator range_it = anv_iterator_range(1, 6, 1, &alloc);

    // Take very large number of elements
    ANVIterator take_it = anv_iterator_take(&range_it, &alloc, SIZE_MAX);
    ASSERT_TRUE(take_it.is_valid(&take_it));

    int values[10];
    const int count = collect_values(&take_it, values, 10);

    ASSERT_EQ(count, 5);  // Should only get available elements

    const int expected[] = {1, 2, 3, 4, 5};
    ASSERT_TRUE(verify_values(values, expected, 5, "take_large_count"));

    take_it.destroy(&take_it);
    return TEST_SUCCESS;
}

//==============================================================================
// Iterator Composition Tests
//==============================================================================

int test_take_with_filter(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range 1-10, filter evens, then take 2
    ANVIterator range_it = anv_iterator_range(1, 11, 1, &alloc);

    ANVIterator filter_it = anv_iterator_filter(&range_it, &alloc, is_even);

    ANVIterator take_it = anv_iterator_take(&filter_it, &alloc, 2);
    ASSERT_TRUE(take_it.is_valid(&take_it));

    int values[10];
    const int count = collect_values(&take_it, values, 10);

    ASSERT_EQ(count, 2);

    const int expected[] = {2, 4};  // First 2 even numbers
    ASSERT_TRUE(verify_values(values, expected, 2, "take_with_filter"));

    take_it.destroy(&take_it);
    return TEST_SUCCESS;
}

int test_take_chained(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range 1-20, take 10, then take 3 from that
    ANVIterator range_it = anv_iterator_range(1, 21, 1, &alloc);

    ANVIterator take_it1 = anv_iterator_take(&range_it, &alloc, 10);

    ANVIterator take_it2 = anv_iterator_take(&take_it1, &alloc, 3);
    ASSERT_TRUE(take_it2.is_valid(&take_it2));

    int values[10];
    const int count = collect_values(&take_it2, values, 10);

    ASSERT_EQ(count, 3);

    const int expected[] = {1, 2, 3};
    ASSERT_TRUE(verify_values(values, expected, 3, "take_chained"));

    take_it2.destroy(&take_it2);
    return TEST_SUCCESS;
}

//==============================================================================
// Behavior and State Tests
//==============================================================================

int test_take_iteration_state(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator 1-10
    ANVIterator range_it = anv_iterator_range(1, 11, 1, &alloc);

    // Take first 3 elements
    ANVIterator take_it = anv_iterator_take(&range_it, &alloc, 3);

    // Test step-by-step iteration
    ASSERT_TRUE(take_it.has_next(&take_it));
    const int* val1 = take_it.get(&take_it);
    ASSERT_NOT_NULL(val1);
    ASSERT_EQ(*val1, 1);

    ASSERT_EQ(take_it.next(&take_it), 0);
    ASSERT_TRUE(take_it.has_next(&take_it));

    const int* val2 = take_it.get(&take_it);
    ASSERT_NOT_NULL(val2);
    ASSERT_EQ(*val2, 2);

    ASSERT_EQ(take_it.next(&take_it), 0);
    ASSERT_TRUE(take_it.has_next(&take_it));

    const int* val3 = take_it.get(&take_it);
    ASSERT_NOT_NULL(val3);
    ASSERT_EQ(*val3, 3);

    ASSERT_EQ(take_it.next(&take_it), 0);

    // Should be exhausted now
    ASSERT_FALSE(take_it.has_next(&take_it));
    ASSERT_NULL(take_it.get(&take_it));
    ASSERT_EQ(take_it.next(&take_it), -1);

    take_it.destroy(&take_it);
    return TEST_SUCCESS;
}

int test_take_unsupported_operations(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator 1-10
    ANVIterator range_it = anv_iterator_range(1, 11, 1, &alloc);

    // Take first 5 elements
    ANVIterator take_it = anv_iterator_take(&range_it, &alloc, 5);

    // Test unsupported operations
    ASSERT_FALSE(take_it.has_prev(&take_it));
    ASSERT_EQ(take_it.prev(&take_it), -1);

    // Reset should be no-op (doesn't crash)
    take_it.reset(&take_it);

    take_it.destroy(&take_it);
    return TEST_SUCCESS;
}

//==============================================================================
// Main Test Runner
//==============================================================================

int main(void)
{
    printf("Running take iterator tests...\n");

    int (*tests[])(void) = {
        test_take_basic_functionality,
        test_take_zero_count,
        test_take_more_than_available,
        test_take_single_element,
        test_take_empty_source,
        test_take_invalid_parameters,
        test_take_large_count,
        test_take_with_filter,
        test_take_chained,
        test_take_iteration_state,
        test_take_unsupported_operations,
    };

    const char* test_names[] = {
        "test_take_basic_functionality",
        "test_take_zero_count",
        "test_take_more_than_available",
        "test_take_single_element",
        "test_take_empty_source",
        "test_take_invalid_parameters",
        "test_take_large_count",
        "test_take_with_filter",
        "test_take_chained",
        "test_take_iteration_state",
        "test_take_unsupported_operations",
    };

    const int num_tests = sizeof(tests) / sizeof(tests[0]);
    int passed = 0;

    for (int i = 0; i < num_tests; i++) {
        printf("Running %s... ", test_names[i]);
        if (tests[i]() == TEST_SUCCESS) {
            printf("PASSED\n");
            passed++;
        } else {
            printf("FAILED\n");
        }
    }

    printf("Take iterator tests: %d/%d passed\n", passed, num_tests);
    return (passed == num_tests) ? 0 : 1;
}
