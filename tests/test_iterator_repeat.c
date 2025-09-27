//
// Created by zack on 9/15/25.
//
// Comprehensive test suite for repeat iterator functionality.
// Tests cover basic iteration, edge cases, repeat handling,
// error handling, and composition with other iterators.

#include "containers/arraylist.h"
#include "containers/doublylinkedlist.h"
#include "containers/iterator.h"
#include "containers/pair.h"
#include "TestAssert.h"
#include "TestHelpers.h"

#include <stdio.h>
#include <stdlib.h>

//==============================================================================
// Helper Functions for Repeat Iterator Tests
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
 * Helper function to verify an array contains the same repeated value.
 */
static int verify_repeated_values(const int* actual, const int expected_value, const int count, const char* test_name)
{
    for (int i = 0; i < count; i++)
    {
        if (actual[i] != expected_value)
        {
            printf("FAIL: %s - Expected %d at index %d, got %d\n",
                   test_name, expected_value, i, actual[i]);
            return 0;
        }
    }
    return 1;
}

//==============================================================================
// Basic Repeat Iterator Tests
//==============================================================================

int test_repeat_basic_functionality(void)
{
    const ANVAllocator alloc = create_int_allocator();

    const int value = 42;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 5);
    ASSERT_TRUE(repeat_it.is_valid(&repeat_it));

    int values[10];
    const int count = collect_values(&repeat_it, values, 10);

    ASSERT_EQ(count, 5);
    ASSERT_TRUE(verify_repeated_values(values, 42, 5, "repeat_basic"));

    repeat_it.destroy(&repeat_it);
    return TEST_SUCCESS;
}

int test_repeat_single_count(void)
{
    const ANVAllocator alloc = create_int_allocator();

    const int value = 99;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 1);
    ASSERT_TRUE(repeat_it.is_valid(&repeat_it));

    ASSERT_TRUE(repeat_it.has_next(&repeat_it));
    const int* retrieved = repeat_it.get(&repeat_it);
    ASSERT_NOT_NULL(retrieved);
    ASSERT_EQ(*retrieved, 99);

    ASSERT_EQ(repeat_it.next(&repeat_it), 0);
    ASSERT_FALSE(repeat_it.has_next(&repeat_it));
    ASSERT_NULL(repeat_it.get(&repeat_it));

    repeat_it.destroy(&repeat_it);
    return TEST_SUCCESS;
}

int test_repeat_zero_count(void)
{
    const ANVAllocator alloc = create_int_allocator();

    const int value = 123;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 0);
    ASSERT_TRUE(repeat_it.is_valid(&repeat_it));

    // Should have no elements
    ASSERT_FALSE(repeat_it.has_next(&repeat_it));
    ASSERT_NULL(repeat_it.get(&repeat_it));

    repeat_it.destroy(&repeat_it);
    return TEST_SUCCESS;
}

int test_repeat_large_count(void)
{
    const ANVAllocator alloc = create_int_allocator();

    const int value = 777;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 1000);
    ASSERT_TRUE(repeat_it.is_valid(&repeat_it));

    // Test first few elements
    for (int i = 0; i < 10; i++)
    {
        ASSERT_TRUE(repeat_it.has_next(&repeat_it));
        const int* retrieved = repeat_it.get(&repeat_it);
        ASSERT_NOT_NULL(retrieved);
        ASSERT_EQ(*retrieved, 777);
        ASSERT_EQ(repeat_it.next(&repeat_it), 0);
    }

    // Skip to near the end
    for (int i = 10; i < 999; i++)
    {
        ASSERT_TRUE(repeat_it.has_next(&repeat_it));
        ASSERT_EQ(repeat_it.next(&repeat_it), 0);
    }

    // Should still have one element left
    ASSERT_TRUE(repeat_it.has_next(&repeat_it));
    const int* last = repeat_it.get(&repeat_it);
    ASSERT_NOT_NULL(last);
    ASSERT_EQ(*last, 777);

    ASSERT_EQ(repeat_it.next(&repeat_it), 0);
    ASSERT_FALSE(repeat_it.has_next(&repeat_it));

    repeat_it.destroy(&repeat_it);
    return TEST_SUCCESS;
}

int test_repeat_different_data_types(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Test with negative number
    const int negative_value = -42;
    ANVIterator repeat_it1 = anv_iterator_repeat(&negative_value, &alloc, 3);
    ASSERT_TRUE(repeat_it1.is_valid(&repeat_it1));

    int values1[5];
    const int count1 = collect_values(&repeat_it1, values1, 5);
    ASSERT_EQ(count1, 3);
    ASSERT_TRUE(verify_repeated_values(values1, -42, 3, "repeat_negative"));

    repeat_it1.destroy(&repeat_it1);

    // Test with zero
    const int zero_value = 0;
    ANVIterator repeat_it2 = anv_iterator_repeat(&zero_value, &alloc, 4);
    ASSERT_TRUE(repeat_it2.is_valid(&repeat_it2));

    int values2[5];
    const int count2 = collect_values(&repeat_it2, values2, 5);
    ASSERT_EQ(count2, 4);
    ASSERT_TRUE(verify_repeated_values(values2, 0, 4, "repeat_zero"));

    repeat_it2.destroy(&repeat_it2);

    return TEST_SUCCESS;
}

//==============================================================================
// Edge Cases and Error Handling
//==============================================================================

int test_repeat_invalid_parameters(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Test with NULL value
    const ANVIterator repeat_it1 = anv_iterator_repeat(NULL, &alloc, 5);
    ASSERT_FALSE(repeat_it1.is_valid(&repeat_it1));

    // Test with NULL allocator
    const int value = 42;
    const ANVIterator repeat_it2 = anv_iterator_repeat(&value, NULL, 5);
    ASSERT_FALSE(repeat_it2.is_valid(&repeat_it2));

    return TEST_SUCCESS;
}

int test_repeat_pointer_consistency(void)
{
    const ANVAllocator alloc = create_int_allocator();

    int value = 888;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 3);

    ASSERT_TRUE(repeat_it.has_next(&repeat_it));

    // Multiple calls to get() should return the same pointer
    const int* ptr1 = repeat_it.get(&repeat_it);
    const int* ptr2 = repeat_it.get(&repeat_it);
    ASSERT_EQ(ptr1, ptr2);
    ASSERT_EQ(ptr1, &value); // Should point to original value

    // After next(), should still point to same original value
    repeat_it.next(&repeat_it);
    const int* ptr3 = repeat_it.get(&repeat_it);
    ASSERT_EQ(ptr3, &value);

    repeat_it.destroy(&repeat_it);
    return TEST_SUCCESS;
}

int test_repeat_exhausted_iterator(void)
{
    const ANVAllocator alloc = create_int_allocator();

    const int value = 555;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 2);

    // Exhaust the iterator
    ASSERT_TRUE(repeat_it.has_next(&repeat_it));
    repeat_it.next(&repeat_it);
    ASSERT_TRUE(repeat_it.has_next(&repeat_it));
    repeat_it.next(&repeat_it);

    // Should be exhausted now
    ASSERT_FALSE(repeat_it.has_next(&repeat_it));
    ASSERT_NULL(repeat_it.get(&repeat_it));
    ASSERT_EQ(repeat_it.next(&repeat_it), -1);

    // Multiple calls to next() on exhausted iterator should return -1
    ASSERT_EQ(repeat_it.next(&repeat_it), -1);
    ASSERT_EQ(repeat_it.next(&repeat_it), -1);

    repeat_it.destroy(&repeat_it);
    return TEST_SUCCESS;
}

//==============================================================================
// Reset Functionality Tests
//==============================================================================

int test_repeat_reset_functionality(void)
{
    const ANVAllocator alloc = create_int_allocator();

    const int value = 321;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 3);

    // Consume first element
    ASSERT_TRUE(repeat_it.has_next(&repeat_it));
    const int* val1 = repeat_it.get(&repeat_it);
    ASSERT_EQ(*val1, 321);
    repeat_it.next(&repeat_it);

    // Consume second element
    ASSERT_TRUE(repeat_it.has_next(&repeat_it));
    const int* val2 = repeat_it.get(&repeat_it);
    ASSERT_EQ(*val2, 321);
    repeat_it.next(&repeat_it);

    // Reset iterator
    repeat_it.reset(&repeat_it);

    // Should be back at beginning
    int values[5];
    const int count = collect_values(&repeat_it, values, 5);
    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_repeated_values(values, 321, 3, "repeat_reset"));

    repeat_it.destroy(&repeat_it);
    return TEST_SUCCESS;
}

int test_repeat_reset_exhausted(void)
{
    const ANVAllocator alloc = create_int_allocator();

    const int value = 654;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 2);

    // Exhaust the iterator
    repeat_it.next(&repeat_it);
    repeat_it.next(&repeat_it);
    ASSERT_FALSE(repeat_it.has_next(&repeat_it));

    // Reset should restore functionality
    repeat_it.reset(&repeat_it);

    int values[5];
    const int count = collect_values(&repeat_it, values, 5);
    ASSERT_EQ(count, 2);
    ASSERT_TRUE(verify_repeated_values(values, 654, 2, "repeat_reset_exhausted"));

    repeat_it.destroy(&repeat_it);
    return TEST_SUCCESS;
}

int test_repeat_reset_empty(void)
{
    const ANVAllocator alloc = create_int_allocator();

    const int value = 111;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 0);

    // Should be empty
    ASSERT_FALSE(repeat_it.has_next(&repeat_it));

    // Reset on empty iterator
    repeat_it.reset(&repeat_it);

    // Should still be empty
    ASSERT_FALSE(repeat_it.has_next(&repeat_it));
    ASSERT_NULL(repeat_it.get(&repeat_it));

    repeat_it.destroy(&repeat_it);
    return TEST_SUCCESS;
}

//==============================================================================
// Iterator Composition Tests
//==============================================================================

int test_repeat_with_filter(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Repeat even number 6 times, then filter for evens (should pass all)
    const int value = 8;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 6);
    ANVIterator filter_it = anv_iterator_filter(&repeat_it, &alloc, is_even);
    ASSERT_TRUE(filter_it.is_valid(&filter_it));

    int values[10];
    const int count = collect_values(&filter_it, values, 10);

    ASSERT_EQ(count, 6);
    ASSERT_TRUE(verify_repeated_values(values, 8, 6, "repeat_with_filter_pass"));

    filter_it.destroy(&filter_it);
    return TEST_SUCCESS;
}

int test_repeat_with_filter_reject(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Repeat odd number 5 times, then filter for evens (should reject all)
    const int value = 7;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 5);
    ANVIterator filter_it = anv_iterator_filter(&repeat_it, &alloc, is_even);
    ASSERT_TRUE(filter_it.is_valid(&filter_it));

    // Should have no elements
    ASSERT_FALSE(filter_it.has_next(&filter_it));
    ASSERT_NULL(filter_it.get(&filter_it));

    filter_it.destroy(&filter_it);
    return TEST_SUCCESS;
}

int test_repeat_with_take(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Repeat value 10 times, then take first 3
    const int value = 456;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 10);
    ANVIterator take_it = anv_iterator_take(&repeat_it, &alloc, 3);
    ASSERT_TRUE(take_it.is_valid(&take_it));

    int values[10];
    const int count = collect_values(&take_it, values, 10);

    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_repeated_values(values, 456, 3, "repeat_with_take"));

    take_it.destroy(&take_it);
    return TEST_SUCCESS;
}

int test_repeat_with_take_more_than_available(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Repeat value 3 times, then try to take 5
    const int value = 789;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 3);
    ANVIterator take_it = anv_iterator_take(&repeat_it, &alloc, 5);
    ASSERT_TRUE(take_it.is_valid(&take_it));

    int values[10];
    const int count = collect_values(&take_it, values, 10);

    ASSERT_EQ(count, 3); // Limited by repeat count
    ASSERT_TRUE(verify_repeated_values(values, 789, 3, "repeat_with_take_limited"));

    take_it.destroy(&take_it);
    return TEST_SUCCESS;
}

int test_repeat_with_skip(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Repeat value 8 times, then skip first 3
    const int value = 202;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 8);
    ANVIterator skip_it = anv_iterator_skip(&repeat_it, &alloc, 3);
    ASSERT_TRUE(skip_it.is_valid(&skip_it));

    int values[10];
    const int count = collect_values(&skip_it, values, 10);

    ASSERT_EQ(count, 5); // 8 - 3 = 5
    ASSERT_TRUE(verify_repeated_values(values, 202, 5, "repeat_with_skip"));

    skip_it.destroy(&skip_it);
    return TEST_SUCCESS;
}

int test_repeat_with_skip_all(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Repeat value 4 times, then skip 4 (all)
    const int value = 303;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 4);
    ANVIterator skip_it = anv_iterator_skip(&repeat_it, &alloc, 4);
    ASSERT_TRUE(skip_it.is_valid(&skip_it));

    // Should have no elements
    ASSERT_FALSE(skip_it.has_next(&skip_it));
    ASSERT_NULL(skip_it.get(&skip_it));

    skip_it.destroy(&skip_it);
    return TEST_SUCCESS;
}

int test_repeat_with_zip(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create two repeat iterators and zip them
    const int value1 = 100;
    const int value2 = 200;
    ANVIterator repeat_it1 = anv_iterator_repeat(&value1, &alloc, 4);
    ANVIterator repeat_it2 = anv_iterator_repeat(&value2, &alloc, 4);

    ANVIterator zip_it = anv_iterator_zip(&repeat_it1, &repeat_it2, &alloc);
    ASSERT_TRUE(zip_it.is_valid(&zip_it));

    // Test all pairs
    for (int i = 0; i < 4; i++)
    {
        ASSERT_TRUE(zip_it.has_next(&zip_it));
        const ANVPair* pair = zip_it.get(&zip_it);
        ASSERT_NOT_NULL(pair);
        ASSERT_NOT_NULL(pair->first);
        ASSERT_NOT_NULL(pair->second);
        ASSERT_EQ(*(const int*)pair->first, 100);
        ASSERT_EQ(*(const int*)pair->second, 200);
        zip_it.next(&zip_it);
    }

    ASSERT_FALSE(zip_it.has_next(&zip_it));

    zip_it.destroy(&zip_it);
    return TEST_SUCCESS;
}

int test_repeat_with_enumerate(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Repeat value 3 times, then enumerate
    const int value = 555;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 3);
    ANVIterator enum_it = anv_iterator_enumerate(&repeat_it, &alloc, 0);
    ASSERT_TRUE(enum_it.is_valid(&enum_it));

    // Test enumeration
    for (int i = 0; i < 3; i++)
    {
        ASSERT_TRUE(enum_it.has_next(&enum_it));
        const ANVIndexedElement* indexed = enum_it.get(&enum_it);
        ASSERT_NOT_NULL(indexed);
        ASSERT_EQ(indexed->index, (size_t)i);
        ASSERT_NOT_NULL(indexed->element);
        ASSERT_EQ(*(const int*)indexed->element, 555);
        enum_it.next(&enum_it);
    }

    ASSERT_FALSE(enum_it.has_next(&enum_it));

    enum_it.destroy(&enum_it);
    return TEST_SUCCESS;
}

int test_repeat_chained_operations(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Complex chain: repeat 10 times -> skip 2 -> take 5 -> filter evens
    const int value = 4; // Even number
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 10);
    ANVIterator skip_it = anv_iterator_skip(&repeat_it, &alloc, 2);
    ANVIterator take_it = anv_iterator_take(&skip_it, &alloc, 5);
    ANVIterator filter_it = anv_iterator_filter(&take_it, &alloc, is_even);
    ASSERT_TRUE(filter_it.is_valid(&filter_it));

    int values[10];
    const int count = collect_values(&filter_it, values, 10);

    ASSERT_EQ(count, 5); // All 5 remaining elements should pass even filter
    ASSERT_TRUE(verify_repeated_values(values, 4, 5, "repeat_chained"));

    filter_it.destroy(&filter_it);
    return TEST_SUCCESS;
}

//==============================================================================
// Data Structure Integration Tests
//==============================================================================

int test_repeat_with_arraylist(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create ArrayList with some values
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);
    for (int i = 1; i <= 3; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i * 10; // [10, 20, 30]
        anv_arraylist_push_back(list, val);
    }

    // Create repeat iterator
    const int repeat_value = 99;
    ANVIterator repeat_it = anv_iterator_repeat(&repeat_value, &alloc, 3);

    // Zip ArrayList with repeat iterator
    ANVIterator array_iter = anv_arraylist_iterator(list);
    ANVIterator zip_it = anv_iterator_zip(&array_iter, &repeat_it, &alloc);
    ASSERT_TRUE(zip_it.is_valid(&zip_it));

    // Verify pairs
    const int expected_first[] = {10, 20, 30};
    const int expected_second[] = {99, 99, 99};

    for (int i = 0; i < 3; i++)
    {
        ASSERT_TRUE(zip_it.has_next(&zip_it));
        const ANVPair* pair = zip_it.get(&zip_it);
        ASSERT_NOT_NULL(pair);
        ASSERT_EQ(*(const int*)pair->first, expected_first[i]);
        ASSERT_EQ(*(const int*)pair->second, expected_second[i]);
        zip_it.next(&zip_it);
    }

    ASSERT_FALSE(zip_it.has_next(&zip_it));

    zip_it.destroy(&zip_it);
    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

int test_repeat_with_range(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Zip range with repeat iterator
    ANVIterator range_it = anv_iterator_range(1, 6, 1, &alloc); // [1,2,3,4,5]

    const int repeat_value = -1;
    ANVIterator repeat_it = anv_iterator_repeat(&repeat_value, &alloc, 5);

    ANVIterator zip_it = anv_iterator_zip(&range_it, &repeat_it, &alloc);
    ASSERT_TRUE(zip_it.is_valid(&zip_it));

    // Verify all pairs
    for (int i = 1; i <= 5; i++)
    {
        ASSERT_TRUE(zip_it.has_next(&zip_it));
        const ANVPair* pair = zip_it.get(&zip_it);
        ASSERT_NOT_NULL(pair);
        ASSERT_EQ(*(const int*)pair->first, i);
        ASSERT_EQ(*(const int*)pair->second, -1);
        zip_it.next(&zip_it);
    }

    ASSERT_FALSE(zip_it.has_next(&zip_it));

    zip_it.destroy(&zip_it);
    return TEST_SUCCESS;
}

//==============================================================================
// Behavior and State Tests
//==============================================================================

int test_repeat_iteration_state(void)
{
    const ANVAllocator alloc = create_int_allocator();

    const int value = 888;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 3);

    // Test step-by-step iteration
    ASSERT_TRUE(repeat_it.has_next(&repeat_it));
    const int* val1 = repeat_it.get(&repeat_it);
    ASSERT_NOT_NULL(val1);
    ASSERT_EQ(*val1, 888);

    ASSERT_EQ(repeat_it.next(&repeat_it), 0);
    ASSERT_TRUE(repeat_it.has_next(&repeat_it));

    const int* val2 = repeat_it.get(&repeat_it);
    ASSERT_NOT_NULL(val2);
    ASSERT_EQ(*val2, 888);

    ASSERT_EQ(repeat_it.next(&repeat_it), 0);
    ASSERT_TRUE(repeat_it.has_next(&repeat_it));

    const int* val3 = repeat_it.get(&repeat_it);
    ASSERT_NOT_NULL(val3);
    ASSERT_EQ(*val3, 888);

    ASSERT_EQ(repeat_it.next(&repeat_it), 0);
    ASSERT_FALSE(repeat_it.has_next(&repeat_it));
    ASSERT_NULL(repeat_it.get(&repeat_it));
    ASSERT_EQ(repeat_it.next(&repeat_it), -1);

    repeat_it.destroy(&repeat_it);
    return TEST_SUCCESS;
}

int test_repeat_unsupported_operations(void)
{
    const ANVAllocator alloc = create_int_allocator();

    const int value = 777;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 5);

    // Test unsupported operations
    ASSERT_FALSE(repeat_it.has_prev(&repeat_it));
    ASSERT_EQ(repeat_it.prev(&repeat_it), -1);

    // Advance iterator then test again
    repeat_it.next(&repeat_it);
    ASSERT_FALSE(repeat_it.has_prev(&repeat_it));
    ASSERT_EQ(repeat_it.prev(&repeat_it), -1);

    repeat_it.destroy(&repeat_it);
    return TEST_SUCCESS;
}

//==============================================================================
// Main Test Runner
//==============================================================================

int main(void)
{
    printf("Running repeat iterator tests...\n");

    int (*tests[])(void) = {
        test_repeat_basic_functionality,
        test_repeat_single_count,
        test_repeat_zero_count,
        test_repeat_large_count,
        test_repeat_different_data_types,
        test_repeat_invalid_parameters,
        test_repeat_pointer_consistency,
        test_repeat_exhausted_iterator,
        test_repeat_reset_functionality,
        test_repeat_reset_exhausted,
        test_repeat_reset_empty,
        test_repeat_with_filter,
        test_repeat_with_filter_reject,
        test_repeat_with_take,
        test_repeat_with_take_more_than_available,
        test_repeat_with_skip,
        test_repeat_with_skip_all,
        test_repeat_with_zip,
        test_repeat_with_enumerate,
        test_repeat_chained_operations,
        test_repeat_with_arraylist,
        test_repeat_with_range,
        test_repeat_iteration_state,
        test_repeat_unsupported_operations,
    };

    const char* test_names[] = {
        "test_repeat_basic_functionality",
        "test_repeat_single_count",
        "test_repeat_zero_count",
        "test_repeat_large_count",
        "test_repeat_different_data_types",
        "test_repeat_invalid_parameters",
        "test_repeat_pointer_consistency",
        "test_repeat_exhausted_iterator",
        "test_repeat_reset_functionality",
        "test_repeat_reset_exhausted",
        "test_repeat_reset_empty",
        "test_repeat_with_filter",
        "test_repeat_with_filter_reject",
        "test_repeat_with_take",
        "test_repeat_with_take_more_than_available",
        "test_repeat_with_skip",
        "test_repeat_with_skip_all",
        "test_repeat_with_zip",
        "test_repeat_with_enumerate",
        "test_repeat_chained_operations",
        "test_repeat_with_arraylist",
        "test_repeat_with_range",
        "test_repeat_iteration_state",
        "test_repeat_unsupported_operations",
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

    printf("Repeat iterator tests: %d/%d passed\n", passed, num_tests);
    return (passed == num_tests) ? 0 : 1;
}
