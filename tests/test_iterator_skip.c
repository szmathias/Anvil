//
// Created by zack on 9/15/25.
//
// Comprehensive test suite for skip iterator functionality.
// Tests cover basic iteration, edge cases, skip handling,
// error handling, and composition with other iterators.

#include "containers/iterator.h"
#include "TestAssert.h"
#include "TestHelpers.h"

#include <stdio.h>
#include <stdlib.h>

//==============================================================================
// Helper Functions for Skip Iterator Tests
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
// Basic Skip Iterator Tests
//==============================================================================

int test_skip_basic_functionality(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator 1-10
    ANVIterator range_it = anv_iterator_range(1, 11, 1, &alloc);

    // Skip first 3 elements (should yield 4, 5, 6, 7, 8, 9, 10)
    ANVIterator skip_it = anv_iterator_skip(&range_it, &alloc, 3);
    ASSERT_TRUE(skip_it.is_valid(&skip_it));

    int values[10];
    const int count = collect_values(&skip_it, values, 10);

    ASSERT_EQ(count, 7);

    const int expected[] = {4, 5, 6, 7, 8, 9, 10};
    ASSERT_TRUE(verify_values(values, expected, 7, "skip_basic"));

    skip_it.destroy(&skip_it);
    return TEST_SUCCESS;
}

int test_skip_zero_count(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator 1-5
    ANVIterator range_it = anv_iterator_range(1, 6, 1, &alloc);

    // Skip 0 elements (should yield all elements)
    ANVIterator skip_it = anv_iterator_skip(&range_it, &alloc, 0);
    ASSERT_TRUE(skip_it.is_valid(&skip_it));

    int values[10];
    const int count = collect_values(&skip_it, values, 10);

    ASSERT_EQ(count, 5);

    const int expected[] = {1, 2, 3, 4, 5};
    ASSERT_TRUE(verify_values(values, expected, 5, "skip_zero"));

    skip_it.destroy(&skip_it);
    return TEST_SUCCESS;
}

int test_skip_more_than_available(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator with only 3 elements
    ANVIterator range_it = anv_iterator_range(1, 4, 1, &alloc);

    // Try to skip 10 elements (more than available)
    ANVIterator skip_it = anv_iterator_skip(&range_it, &alloc, 10);
    ASSERT_TRUE(skip_it.is_valid(&skip_it));

    // Should have no elements
    ASSERT_FALSE(skip_it.has_next(&skip_it));
    ASSERT_NULL(skip_it.get(&skip_it));

    skip_it.destroy(&skip_it);
    return TEST_SUCCESS;
}

int test_skip_all_elements(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator 1-5
    ANVIterator range_it = anv_iterator_range(1, 6, 1, &alloc);

    // Skip exactly all elements
    ANVIterator skip_it = anv_iterator_skip(&range_it, &alloc, 5);
    ASSERT_TRUE(skip_it.is_valid(&skip_it));

    // Should have no elements
    ASSERT_FALSE(skip_it.has_next(&skip_it));
    ASSERT_NULL(skip_it.get(&skip_it));

    skip_it.destroy(&skip_it);
    return TEST_SUCCESS;
}

int test_skip_single_element(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator 1-5
    ANVIterator range_it = anv_iterator_range(1, 6, 1, &alloc);

    // Skip only 1 element
    ANVIterator skip_it = anv_iterator_skip(&range_it, &alloc, 1);
    ASSERT_TRUE(skip_it.is_valid(&skip_it));

    int values[10];
    const int count = collect_values(&skip_it, values, 10);

    ASSERT_EQ(count, 4);

    const int expected[] = {2, 3, 4, 5};
    ASSERT_TRUE(verify_values(values, expected, 4, "skip_single"));

    skip_it.destroy(&skip_it);
    return TEST_SUCCESS;
}

//==============================================================================
// Edge Cases and Error Handling
//==============================================================================

int test_skip_empty_source(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create empty range iterator
    ANVIterator range_it = anv_iterator_range(1, 1, 1, &alloc);  // Empty range

    // Try to skip 5 elements from empty iterator
    ANVIterator skip_it = anv_iterator_skip(&range_it, &alloc, 5);
    ASSERT_TRUE(skip_it.is_valid(&skip_it));

    // Should have no elements
    ASSERT_FALSE(skip_it.has_next(&skip_it));
    ASSERT_NULL(skip_it.get(&skip_it));

    skip_it.destroy(&skip_it);
    return TEST_SUCCESS;
}

int test_skip_invalid_parameters(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Test with NULL iterator
    const ANVIterator skip_it1 = anv_iterator_skip(NULL, &alloc, 5);
    ASSERT_FALSE(skip_it1.is_valid(&skip_it1));

    // Test with NULL allocator
    ANVIterator range_it = anv_iterator_range(1, 11, 1, &alloc);

    const ANVIterator skip_it2 = anv_iterator_skip(&range_it, NULL, 5);
    ASSERT_FALSE(skip_it2.is_valid(&skip_it2));

    // Clean up the range iterator manually since skip failed
    range_it.destroy(&range_it);

    return TEST_SUCCESS;
}

int test_skip_large_count(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator 1-5
    ANVIterator range_it = anv_iterator_range(1, 6, 1, &alloc);

    // Skip very large number of elements
    ANVIterator skip_it = anv_iterator_skip(&range_it, &alloc, SIZE_MAX);
    ASSERT_TRUE(skip_it.is_valid(&skip_it));

    // Should have no elements
    ASSERT_FALSE(skip_it.has_next(&skip_it));
    ASSERT_NULL(skip_it.get(&skip_it));

    skip_it.destroy(&skip_it);
    return TEST_SUCCESS;
}

//==============================================================================
// Iterator Composition Tests
//==============================================================================

int test_skip_with_filter(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range 1-10, filter evens, then skip 1 (should skip first even: 2)
    ANVIterator range_it = anv_iterator_range(1, 11, 1, &alloc);

    ANVIterator filter_it = anv_iterator_filter(&range_it, &alloc, is_even);

    ANVIterator skip_it = anv_iterator_skip(&filter_it, &alloc, 1);
    ASSERT_TRUE(skip_it.is_valid(&skip_it));

    int values[10];
    const int count = collect_values(&skip_it, values, 10);

    ASSERT_EQ(count, 4);

    const int expected[] = {4, 6, 8, 10};  // Even numbers after skipping first even (2)
    ASSERT_TRUE(verify_values(values, expected, 4, "skip_with_filter"));

    skip_it.destroy(&skip_it);
    return TEST_SUCCESS;
}

int test_skip_chained(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range 1-20, skip 5, then skip 2 more from that
    ANVIterator range_it = anv_iterator_range(1, 21, 1, &alloc);

    ANVIterator skip_it1 = anv_iterator_skip(&range_it, &alloc, 5);

    ANVIterator skip_it2 = anv_iterator_skip(&skip_it1, &alloc, 2);
    ASSERT_TRUE(skip_it2.is_valid(&skip_it2));

    int values[20];
    const int count = collect_values(&skip_it2, values, 20);

    ASSERT_EQ(count, 13);

    const int expected[] = {8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
    ASSERT_TRUE(verify_values(values, expected, 13, "skip_chained"));

    skip_it2.destroy(&skip_it2);
    return TEST_SUCCESS;
}

int test_skip_with_take(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range 1-20, skip 3, then take 5
    ANVIterator range_it = anv_iterator_range(1, 21, 1, &alloc);

    ANVIterator skip_it = anv_iterator_skip(&range_it, &alloc, 3);

    ANVIterator take_it = anv_iterator_take(&skip_it, &alloc, 5);
    ASSERT_TRUE(take_it.is_valid(&take_it));

    int values[10];
    const int count = collect_values(&take_it, values, 10);

    ASSERT_EQ(count, 5);

    const int expected[] = {4, 5, 6, 7, 8};  // Skip first 3, then take next 5
    ASSERT_TRUE(verify_values(values, expected, 5, "skip_with_take"));

    take_it.destroy(&take_it);
    return TEST_SUCCESS;
}

//==============================================================================
// Behavior and State Tests
//==============================================================================

int test_skip_iteration_state(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator 1-10
    ANVIterator range_it = anv_iterator_range(1, 11, 1, &alloc);

    // Skip first 2 elements
    ANVIterator skip_it = anv_iterator_skip(&range_it, &alloc, 2);

    // Test step-by-step iteration
    ASSERT_TRUE(skip_it.has_next(&skip_it));
    const int* val1 = skip_it.get(&skip_it);
    ASSERT_NOT_NULL(val1);
    ASSERT_EQ(*val1, 3);  // First non-skipped element

    ASSERT_EQ(skip_it.next(&skip_it), 0);
    ASSERT_TRUE(skip_it.has_next(&skip_it));

    const int* val2 = skip_it.get(&skip_it);
    ASSERT_NOT_NULL(val2);
    ASSERT_EQ(*val2, 4);

    ASSERT_EQ(skip_it.next(&skip_it), 0);
    ASSERT_TRUE(skip_it.has_next(&skip_it));

    const int* val3 = skip_it.get(&skip_it);
    ASSERT_NOT_NULL(val3);
    ASSERT_EQ(*val3, 5);

    skip_it.destroy(&skip_it);
    return TEST_SUCCESS;
}

int test_skip_lazy_evaluation(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator 1-10
    ANVIterator range_it = anv_iterator_range(1, 11, 1, &alloc);

    // Skip first 3 elements - should not perform skip until first access
    ANVIterator skip_it = anv_iterator_skip(&range_it, &alloc, 3);

    // The skip should happen on first has_next() call
    ASSERT_TRUE(skip_it.has_next(&skip_it));

    // Should be positioned at element 4
    const int* val = skip_it.get(&skip_it);
    ASSERT_NOT_NULL(val);
    ASSERT_EQ(*val, 4);

    skip_it.destroy(&skip_it);
    return TEST_SUCCESS;
}

int test_skip_unsupported_operations(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator 1-10
    ANVIterator range_it = anv_iterator_range(1, 11, 1, &alloc);

    // Skip first 5 elements
    ANVIterator skip_it = anv_iterator_skip(&range_it, &alloc, 5);

    // Test unsupported operations
    ASSERT_FALSE(skip_it.has_prev(&skip_it));
    ASSERT_EQ(skip_it.prev(&skip_it), -1);

    // Reset should be no-op (doesn't crash)
    skip_it.reset(&skip_it);

    skip_it.destroy(&skip_it);
    return TEST_SUCCESS;
}

//==============================================================================
// Main Test Runner
//==============================================================================

int main(void)
{
    printf("Running skip iterator tests...\n");

    int (*tests[])(void) = {
        test_skip_basic_functionality,
        test_skip_zero_count,
        test_skip_more_than_available,
        test_skip_all_elements,
        test_skip_single_element,
        test_skip_empty_source,
        test_skip_invalid_parameters,
        test_skip_large_count,
        test_skip_with_filter,
        test_skip_chained,
        test_skip_with_take,
        test_skip_iteration_state,
        test_skip_lazy_evaluation,
        test_skip_unsupported_operations,
    };

    const char* test_names[] = {
        "test_skip_basic_functionality",
        "test_skip_zero_count",
        "test_skip_more_than_available",
        "test_skip_all_elements",
        "test_skip_single_element",
        "test_skip_empty_source",
        "test_skip_invalid_parameters",
        "test_skip_large_count",
        "test_skip_with_filter",
        "test_skip_chained",
        "test_skip_with_take",
        "test_skip_iteration_state",
        "test_skip_lazy_evaluation",
        "test_skip_unsupported_operations",
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

    printf("Skip iterator tests: %d/%d passed\n", passed, num_tests);
    return (passed == num_tests) ? 0 : 1;
}
