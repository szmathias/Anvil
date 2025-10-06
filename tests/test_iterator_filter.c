//
// Created by zack on 9/3/25.
//
// Comprehensive test suite for filter iterator functionality.
// Tests cover basic filtering, filter chaining, error handling, memory management,
// and integration with different data structures.

#include <stdio.h>
#include <stdlib.h>

#include "containers/doublylinkedlist.h"
#include "containers/iterator.h"
#include "TestAssert.h"
#include "TestHelpers.h"

//==============================================================================
// Helper Functions for Filter Iterator Tests
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

/**
 * Helper function to create a test list with integers 1 through n.
 */
static ANVDoublyLinkedList* create_test_list(ANVAllocator* alloc, const int n)
{
    ANVDoublyLinkedList* list = anv_dll_create(alloc);
    if (!list)
        return NULL;

    for (int i = 1; i <= n; i++)
    {
        int* val = malloc(sizeof(int));
        if (!val)
        {
            anv_dll_destroy(list, true);
            return NULL;
        }
        *val = i;
        anv_dll_push_back(list, val);
    }
    return list;
}

/**
 * Helper function to create a test list with specific values.
 */
static ANVDoublyLinkedList* create_list_with_values(ANVAllocator* alloc, const int* values, const int count)
{
    ANVDoublyLinkedList* list = anv_dll_create(alloc);
    if (!list)
        return NULL;

    for (int i = 0; i < count; i++)
    {
        int* val = malloc(sizeof(int));
        if (!val)
        {
            anv_dll_destroy(list, true);
            return NULL;
        }
        *val = values[i];
        anv_dll_push_back(list, val);
    }
    return list;
}

//==============================================================================
// Basic Filter Iterator Tests
//==============================================================================

/**
 * Test basic filter iterator functionality with even numbers.
 */
static int test_filter_basic_even(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 10);
    ASSERT_NOT_NULL(list);

    // Create base iterator
    ANVIterator base_it = anv_dll_iterator(list);
    ASSERT_TRUE(base_it.is_valid(&base_it));

    // Create filter iterator for even numbers
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_even);
    ASSERT_TRUE(filter_it.is_valid(&filter_it));
    ASSERT_NOT_NULL(filter_it.data_state);

    // Expected: [1,2,3,4,5,6,7,8,9,10] -> [2,4,6,8,10]
    const int expected[] = {2, 4, 6, 8, 10};
    int actual[5];
    const int count = collect_values(&filter_it, actual, 5);

    ASSERT_EQ(count, 5);
    ASSERT_TRUE(verify_values(actual, expected, count, "basic_even"));

    // Iterator should be exhausted
    ASSERT_FALSE(filter_it.has_next(&filter_it));
    ASSERT_NULL(filter_it.get(&filter_it));

    // Cleanup
    filter_it.destroy(&filter_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test filter iterator with odd numbers.
 */
static int test_filter_odd(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 7);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_odd);

    ASSERT_TRUE(filter_it.is_valid(&filter_it));

    // Expected: [1,2,3,4,5,6,7] -> [1,3,5,7]
    const int expected[] = {1, 3, 5, 7};
    int actual[4];
    const int count = collect_values(&filter_it, actual, 4);

    ASSERT_EQ(count, 4);
    ASSERT_TRUE(verify_values(actual, expected, count, "filter_odd"));

    filter_it.destroy(&filter_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test filter iterator with greater than 5 predicate.
 */
static int test_filter_greater_than_five(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 8);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_greater_than_five);

    ASSERT_TRUE(filter_it.is_valid(&filter_it));

    // Expected: [1,2,3,4,5,6,7,8] -> [6,7,8]
    const int expected[] = {6, 7, 8};
    int actual[3];
    const int count = collect_values(&filter_it, actual, 3);

    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_values(actual, expected, count, "filter_greater_than_five"));

    filter_it.destroy(&filter_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test filter iterator with divisible by 3 predicate.
 */
static int test_filter_divisible_by_3(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 12);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_divisible_by_3);

    ASSERT_TRUE(filter_it.is_valid(&filter_it));

    // Expected: [1,2,3,4,5,6,7,8,9,10,11,12] -> [3,6,9,12]
    const int expected[] = {3, 6, 9, 12};
    int actual[4];
    const int count = collect_values(&filter_it, actual, 4);

    ASSERT_EQ(count, 4);
    ASSERT_TRUE(verify_values(actual, expected, count, "filter_divisible_by_3"));

    filter_it.destroy(&filter_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Edge Case Tests
//==============================================================================

/**
 * Test filter iterator with empty input.
 */
static int test_filter_empty_input(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    ASSERT_NOT_NULL(list);

    // Create iterator on empty list
    ANVIterator base_it = anv_dll_iterator(list);
    ASSERT_FALSE(base_it.has_next(&base_it));

    // Create filter iterator
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_even);
    ASSERT_TRUE(filter_it.is_valid(&filter_it));
    ASSERT_FALSE(filter_it.has_next(&filter_it));
    ASSERT_NULL(filter_it.get(&filter_it));

    // Test that next() fails appropriately
    ASSERT_EQ(filter_it.next(&filter_it), -1);

    filter_it.destroy(&filter_it);
    anv_dll_destroy(list, false);
    return TEST_SUCCESS;
}

/**
 * Test filter iterator with no matching elements.
 */
static int test_filter_no_matches(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create list with only odd numbers: [1,3,5,7,9]
    const int odd_values[] = {1, 3, 5, 7, 9};
    ANVDoublyLinkedList* list = create_list_with_values(&alloc, odd_values, 5);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_even);

    ASSERT_TRUE(filter_it.is_valid(&filter_it));

    // Should find no even numbers
    ASSERT_FALSE(filter_it.has_next(&filter_it));
    ASSERT_NULL(filter_it.get(&filter_it));

    // Test that next() fails appropriately
    ASSERT_EQ(filter_it.next(&filter_it), -1);

    filter_it.destroy(&filter_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test filter iterator with all matching elements.
 */
static int test_filter_all_matches(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create list with only even numbers: [2,4,6,8,10]
    const int even_values[] = {2, 4, 6, 8, 10};
    ANVDoublyLinkedList* list = create_list_with_values(&alloc, even_values, 5);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_even);

    ASSERT_TRUE(filter_it.is_valid(&filter_it));

    // Should find all 5 even numbers
    const int expected[] = {2, 4, 6, 8, 10};
    int actual[5];
    const int count = collect_values(&filter_it, actual, 5);

    ASSERT_EQ(count, 5);
    ASSERT_TRUE(verify_values(actual, expected, count, "filter_all_matches"));

    filter_it.destroy(&filter_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test filter iterator with single element.
 */
static int test_filter_single_element(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 1);
    ASSERT_NOT_NULL(list);

    // Test with single odd element (should be filtered out)
    ANVIterator base_it1 = anv_dll_iterator(list);
    ANVIterator filter_it1 = anv_iterator_filter(&base_it1, &alloc, is_even);

    ASSERT_TRUE(filter_it1.is_valid(&filter_it1));
    ASSERT_FALSE(filter_it1.has_next(&filter_it1));
    ASSERT_NULL(filter_it1.get(&filter_it1));

    filter_it1.destroy(&filter_it1);

    // Test with single even element (should pass through)
    const int even_value[] = {4};
    ANVDoublyLinkedList* even_list = create_list_with_values(&alloc, even_value, 1);
    ASSERT_NOT_NULL(even_list);

    ANVIterator base_it2 = anv_dll_iterator(even_list);
    ANVIterator filter_it2 = anv_iterator_filter(&base_it2, &alloc, is_even);

    ASSERT_TRUE(filter_it2.is_valid(&filter_it2));
    ASSERT_TRUE(filter_it2.has_next(&filter_it2));

    const int* value = filter_it2.get(&filter_it2);
    ASSERT_NOT_NULL(value);
    ASSERT_EQ(*value, 4);

    filter_it2.next(&filter_it2);
    ASSERT_FALSE(filter_it2.has_next(&filter_it2));
    ASSERT_NULL(filter_it2.get(&filter_it2));

    filter_it2.destroy(&filter_it2);
    anv_dll_destroy(list, true);
    anv_dll_destroy(even_list, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Error Handling Tests
//==============================================================================

/**
 * Test filter iterator with invalid inputs.
 */
static int test_filter_invalid_inputs(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 1);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);

    // Test with NULL iterator
    ANVIterator invalid_it1 = anv_iterator_filter(NULL, &alloc, is_even);
    ASSERT_FALSE(invalid_it1.is_valid(&invalid_it1));
    ASSERT_FALSE(invalid_it1.has_next(&invalid_it1));
    ASSERT_NULL(invalid_it1.get(&invalid_it1));

    // Test with NULL filter function
    ANVIterator invalid_it2 = anv_iterator_filter(&base_it, &alloc, NULL);
    ASSERT_FALSE(invalid_it2.is_valid(&invalid_it2));

    // Test with NULL allocator
    ANVIterator base_it2 = anv_dll_iterator(list);
    ANVIterator invalid_it3 = anv_iterator_filter(&base_it2, NULL, is_even);
    ASSERT_FALSE(invalid_it3.is_valid(&invalid_it3));

    // Cleanup
    invalid_it1.destroy(&invalid_it1);
    invalid_it2.destroy(&invalid_it2);
    invalid_it3.destroy(&invalid_it3);
    base_it2.destroy(&base_it2);
    base_it.destroy(&base_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test filter iterator operations on invalid iterator.
 */
static int test_filter_operations_on_invalid(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create invalid filter iterator
    ANVIterator invalid_it = anv_iterator_filter(NULL, &alloc, is_even);
    ASSERT_FALSE(invalid_it.is_valid(&invalid_it));

    // All operations should fail gracefully
    ASSERT_EQ(invalid_it.next(&invalid_it), -1);
    ASSERT_EQ(invalid_it.prev(&invalid_it), -1); // Filter doesn't support prev
    ASSERT_FALSE(invalid_it.has_next(&invalid_it));
    ASSERT_FALSE(invalid_it.has_prev(&invalid_it)); // Filter doesn't support has_prev
    ASSERT_NULL(invalid_it.get(&invalid_it));

    // Reset should be safe to call but ineffective
    invalid_it.reset(&invalid_it); // Should not crash

    invalid_it.destroy(&invalid_it);
    return TEST_SUCCESS;
}

//==============================================================================
// API Behavior Tests
//==============================================================================

/**
 * Test that get() doesn't advance, next() does advance.
 */
static int test_filter_get_next_separation(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 6);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_even);

    ASSERT_TRUE(filter_it.is_valid(&filter_it));

    // Test get without advancing - should return first even number (2)
    const int* value1 = filter_it.get(&filter_it);
    ASSERT_NOT_NULL(value1);
    ASSERT_EQ(*value1, 2);

    // Get again - should return same value
    const int* value2 = filter_it.get(&filter_it);
    ASSERT_NOT_NULL(value2);
    ASSERT_EQ(*value2, 2);

    // Store value before advancing to avoid pointer reuse issues
    const int first_value = *value1;

    // Now advance
    ASSERT_EQ(filter_it.next(&filter_it), 0);
    const int* value3 = filter_it.get(&filter_it);
    ASSERT_NOT_NULL(value3);
    ASSERT_EQ(*value3, 4); // Next even number
    ASSERT_NOT_EQ(first_value, *value3);

    filter_it.destroy(&filter_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test next() return codes.
 */
static int test_filter_next_return_codes(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create list with only 2 even numbers: [2,4]
    const int even_values[] = {2, 4};
    ANVDoublyLinkedList* list = create_list_with_values(&alloc, even_values, 2);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_even);

    ASSERT_TRUE(filter_it.is_valid(&filter_it));

    // First next should succeed
    ASSERT_EQ(filter_it.next(&filter_it), 0);

    // Second next should succeed
    ASSERT_EQ(filter_it.next(&filter_it), 0);

    // Third next should fail (at end)
    ASSERT_EQ(filter_it.next(&filter_it), -1);

    filter_it.destroy(&filter_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test unsupported bidirectional operations.
 */
static int test_filter_unsupported_operations(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 3);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_even);

    ASSERT_TRUE(filter_it.is_valid(&filter_it));

    // Filter iterator should not support bidirectional operations
    ASSERT_FALSE(filter_it.has_prev(&filter_it));
    ASSERT_EQ(filter_it.prev(&filter_it), -1); // Returns -1 for unsupported

    // Reset should be safe but ineffective
    filter_it.reset(&filter_it);

    // Should still be valid after unsupported operations
    ASSERT_TRUE(filter_it.is_valid(&filter_it));

    filter_it.destroy(&filter_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Filter Iterator Chaining Tests
//==============================================================================

/**
 * Test chaining multiple filters: even AND divisible by 3.
 */
static int test_multiple_filter_chain_even_div3(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 20);
    ASSERT_NOT_NULL(list);

    // Create base iterator
    ANVIterator base_it = anv_dll_iterator(list);

    // Chain filters: even AND divisible by 3 (i.e., divisible by 6)
    ANVIterator filter_even = anv_iterator_filter(&base_it, &alloc, is_even);
    ANVIterator filter_div3 = anv_iterator_filter(&filter_even, &alloc, is_divisible_by_3);

    ASSERT_TRUE(filter_div3.is_valid(&filter_div3));

    // Expected: [1..20] -> [2,4,6,8,10,12,14,16,18,20] -> [6,12,18]
    const int expected[] = {6, 12, 18};
    int actual[3];
    const int count = collect_values(&filter_div3, actual, 3);

    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_values(actual, expected, count, "multiple_filter_chain_even_div3"));

    filter_div3.destroy(&filter_div3);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test chaining multiple filters: divisible by 4 AND greater than 10.
 */
static int test_multiple_filter_chain_div4_gt10(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 25);
    ASSERT_NOT_NULL(list);

    // Create base iterator
    ANVIterator base_it = anv_dll_iterator(list);

    // Chain filters: divisible by 4 AND greater than 10
    ANVIterator filter_div4 = anv_iterator_filter(&base_it, &alloc, is_divisible_by_4);
    ANVIterator filter_gt10 = anv_iterator_filter(&filter_div4, &alloc, is_greater_than_10);

    ASSERT_TRUE(filter_gt10.is_valid(&filter_gt10));

    // Expected: [1..25] -> [4,8,12,16,20,24] -> [12,16,20,24]
    const int expected[] = {12, 16, 20, 24};
    int actual[4];
    const int count = collect_values(&filter_gt10, actual, 4);

    ASSERT_EQ(count, 4);
    ASSERT_TRUE(verify_values(actual, expected, count, "multiple_filter_chain_div4_gt10"));

    filter_gt10.destroy(&filter_gt10);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test triple filter chaining: odd AND greater than 5 AND divisible by 3.
 */
static int test_triple_filter_chain(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 30);
    ASSERT_NOT_NULL(list);

    // Create base iterator
    ANVIterator base_it = anv_dll_iterator(list);

    // Chain three filters: odd AND greater than 5 AND divisible by 3
    ANVIterator filter_odd = anv_iterator_filter(&base_it, &alloc, is_odd);
    ANVIterator filter_gt5 = anv_iterator_filter(&filter_odd, &alloc, is_greater_than_five);
    ANVIterator filter_div3 = anv_iterator_filter(&filter_gt5, &alloc, is_divisible_by_3);

    ASSERT_TRUE(filter_div3.is_valid(&filter_div3));

    // Expected: [1..30] -> [1,3,5,7,9,11,13,15,17,19,21,23,25,27,29]
    //                   -> [7,9,11,13,15,17,19,21,23,25,27,29]
    //                   -> [9,15,21,27]
    const int expected[] = {9, 15, 21, 27};
    int actual[4];
    const int count = collect_values(&filter_div3, actual, 4);

    ASSERT_EQ(count, 4);
    ASSERT_TRUE(verify_values(actual, expected, count, "triple_filter_chain"));

    filter_div3.destroy(&filter_div3);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test filter chain with no final matches.
 */
static int test_filter_chain_no_matches(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 10);
    ASSERT_NOT_NULL(list);

    // Create base iterator
    ANVIterator base_it = anv_dll_iterator(list);

    // Chain filters: even AND greater than 20 (no matches in [1..10])
    ANVIterator filter_even = anv_iterator_filter(&base_it, &alloc, is_even);
    ANVIterator filter_gt20 = anv_iterator_filter(&filter_even, &alloc, is_greater_than_20);

    ASSERT_TRUE(filter_gt20.is_valid(&filter_gt20));

    // Should find no matching elements
    ASSERT_FALSE(filter_gt20.has_next(&filter_gt20));
    ASSERT_NULL(filter_gt20.get(&filter_gt20));
    ASSERT_EQ(filter_gt20.next(&filter_gt20), -1);

    filter_gt20.destroy(&filter_gt20);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test filter chain with single final match.
 */
static int test_filter_chain_single_match(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 15);
    ASSERT_NOT_NULL(list);

    // Create base iterator
    ANVIterator base_it = anv_dll_iterator(list);

    // Chain filters: divisible by 6 AND greater than 10 (only 12 matches in [1..15])
    ANVIterator filter_div6 = anv_iterator_filter(&base_it, &alloc, is_divisible_by_six);
    ANVIterator filter_gt10 = anv_iterator_filter(&filter_div6, &alloc, is_greater_than_10);

    ASSERT_TRUE(filter_gt10.is_valid(&filter_gt10));

    // Expected: [1..15] -> [6,12] -> [12]
    ASSERT_TRUE(filter_gt10.has_next(&filter_gt10));
    const int* value = filter_gt10.get(&filter_gt10);
    ASSERT_NOT_NULL(value);
    ASSERT_EQ(*value, 12);

    // Advance to end
    filter_gt10.next(&filter_gt10);
    ASSERT_FALSE(filter_gt10.has_next(&filter_gt10));
    ASSERT_NULL(filter_gt10.get(&filter_gt10));

    filter_gt10.destroy(&filter_gt10);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Memory Management Tests
//==============================================================================

/**
 * Test memory consistency across operations.
 */
static int test_filter_memory_consistency(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 4);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_even);

    // Get multiple references to the same value
    const int* ptr1 = filter_it.get(&filter_it);
    const int* ptr2 = filter_it.get(&filter_it);
    const int* ptr3 = filter_it.get(&filter_it);

    // All should return the same value (first even number: 2)
    ASSERT_NOT_NULL(ptr1);
    ASSERT_NOT_NULL(ptr2);
    ASSERT_NOT_NULL(ptr3);
    ASSERT_EQ(*ptr1, 2);
    ASSERT_EQ(*ptr2, 2);
    ASSERT_EQ(*ptr3, 2);

    // Store the value before moving to next
    const int first_value = *ptr1;

    // Move to next and verify new value
    filter_it.next(&filter_it);
    const int* ptr4 = filter_it.get(&filter_it);
    ASSERT_NOT_NULL(ptr4);
    ASSERT_EQ(*ptr4, 4); // Next even number

    // Values should be different
    ASSERT_NOT_EQ(first_value, *ptr4);

    filter_it.destroy(&filter_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test that filter iterator properly manages base iterator lifecycle.
 */
static int test_filter_iterator_ownership(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 2);
    ASSERT_NOT_NULL(list);

    // Create base iterator
    ANVIterator base_it = anv_dll_iterator(list);
    ASSERT_TRUE(base_it.is_valid(&base_it));

    // Create filter iterator (takes ownership of base_it)
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_even);
    ASSERT_TRUE(filter_it.is_valid(&filter_it));

    // Verify filter works
    const int* value = filter_it.get(&filter_it);
    ASSERT_NOT_NULL(value);
    ASSERT_EQ(*value, 2); // First even number

    // When we destroy filter iterator, it should clean up base iterator too
    filter_it.destroy(&filter_it);

    // Note: We should not access base_it after this point as it's been destroyed

    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test memory management in filter chains.
 */
static int test_filter_chain_memory_management(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 20);
    ASSERT_NOT_NULL(list);

    // Create base iterator
    ANVIterator base_it = anv_dll_iterator(list);

    // Create a chain of filters
    ANVIterator filter1 = anv_iterator_filter(&base_it, &alloc, is_even);
    ANVIterator filter2 = anv_iterator_filter(&filter1, &alloc, is_divisible_by_3);
    ANVIterator filter3 = anv_iterator_filter(&filter2, &alloc, is_greater_than_five);

    ASSERT_TRUE(filter3.is_valid(&filter3));

    // Use the chain
    int count = 0;
    while (filter3.has_next(&filter3))
    {
        const int* value = filter3.get(&filter3);
        ASSERT_NOT_NULL(value);
        count++;
        filter3.next(&filter3);
    }

    // Should have found some matching elements
    ASSERT_TRUE(count > 0);

    // Destroying the final filter should clean up the entire chain
    filter3.destroy(&filter3);

    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Performance and Stress Tests
//==============================================================================

/**
 * Test filter iterator with large dataset.
 */
static int test_filter_large_dataset(void)
{
    ANVAllocator alloc = create_int_allocator();
    const int SIZE = 1000;

    ANVDoublyLinkedList* list = create_test_list(&alloc, SIZE);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_even);

    ASSERT_TRUE(filter_it.is_valid(&filter_it));

    int count = 0;
    int expected = 2; // First even number

    while (filter_it.has_next(&filter_it))
    {
        const int* value = filter_it.get(&filter_it);
        ASSERT_NOT_NULL(value);
        ASSERT_EQ(*value, expected);

        count++;
        expected += 2; // Next expected even number
        filter_it.next(&filter_it);
    }

    ASSERT_EQ(count, SIZE / 2); // Should have SIZE/2 even numbers
    ASSERT_FALSE(filter_it.has_next(&filter_it));

    filter_it.destroy(&filter_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test complex filter chaining performance.
 */
static int test_filter_complex_chaining(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 100);
    ASSERT_NOT_NULL(list);

    // Create a complex chain: even -> divisible by 3 -> greater than 10 -> divisible by 4
    ANVIterator it1 = anv_dll_iterator(list);
    ANVIterator it2 = anv_iterator_filter(&it1, &alloc, is_even);
    ANVIterator it3 = anv_iterator_filter(&it2, &alloc, is_divisible_by_3);
    ANVIterator it4 = anv_iterator_filter(&it3, &alloc, is_greater_than_10);
    ANVIterator final_it = anv_iterator_filter(&it4, &alloc, is_divisible_by_4);

    ASSERT_TRUE(final_it.is_valid(&final_it));

    // Just verify it works end-to-end and count results
    int count = 0;
    while (final_it.has_next(&final_it))
    {
        const int* value = final_it.get(&final_it);
        ASSERT_NOT_NULL(value);

        // Verify all conditions are met
        ASSERT_TRUE(*value % 2 == 0); // even
        ASSERT_TRUE(*value % 3 == 0); // divisible by 3
        ASSERT_TRUE(*value > 10);     // greater than 10
        ASSERT_TRUE(*value % 4 == 0); // divisible by 4

        count++;
        final_it.next(&final_it);
    }

    // We should get some results (numbers divisible by 12 and > 10)
    ASSERT_TRUE(count > 0);

    final_it.destroy(&final_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Enhanced Helper Function Tests
//==============================================================================

/**
 * Enhanced helper function with better validation.
 */
static int collect_values_with_validation(const ANVIterator* it, int* values, int max_count)
{
    int count = 0;
    while (it->has_next(it) && count < max_count)
    {
        const int* value = it->get(it);
        if (!value)
        {
            // get() returned null but has_next was true - this is an error
            return -1;
        }

        values[count++] = *value;

        // Verify that next() succeeds when has_next() is true
        if (it->next(it) != 0)
        {
            // next() failed but has_next() was true - this is an error
            return -1;
        }
    }
    return count;
}

/**
 * Test helper function validation with filter iterator.
 */
static int test_filter_helper_validation(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 10);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_even);

    ASSERT_TRUE(filter_it.is_valid(&filter_it));

    int values[5];
    const int count = collect_values_with_validation(&filter_it, values, 5);

    // Should successfully collect all 5 even values
    ASSERT_EQ(count, 5);

    const int expected[] = {2, 4, 6, 8, 10};
    ASSERT_TRUE(verify_values(values, expected, count, "helper_validation"));

    filter_it.destroy(&filter_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Test Suite Definition
//==============================================================================

typedef struct
{
    int (*func)(void);
    const char* name;
} TestCase;

static const TestCase tests[] = {
    // Basic functionality tests
    {test_filter_basic_even, "test_filter_basic_even"},
    {test_filter_odd, "test_filter_odd"},
    {test_filter_greater_than_five, "test_filter_greater_than_five"},
    {test_filter_divisible_by_3, "test_filter_divisible_by_3"},

    // Edge case tests
    {test_filter_empty_input, "test_filter_empty_input"},
    {test_filter_no_matches, "test_filter_no_matches"},
    {test_filter_all_matches, "test_filter_all_matches"},
    {test_filter_single_element, "test_filter_single_element"},

    // Error handling tests
    {test_filter_invalid_inputs, "test_filter_invalid_inputs"},
    {test_filter_operations_on_invalid, "test_filter_operations_on_invalid"},

    // API behavior tests
    {test_filter_get_next_separation, "test_filter_get_next_separation"},
    {test_filter_next_return_codes, "test_filter_next_return_codes"},
    {test_filter_unsupported_operations, "test_filter_unsupported_operations"},

    // Filter chaining tests (pure filter chaining only)
    {test_multiple_filter_chain_even_div3, "test_multiple_filter_chain_even_div3"},
    {test_multiple_filter_chain_div4_gt10, "test_multiple_filter_chain_div4_gt10"},
    {test_triple_filter_chain, "test_triple_filter_chain"},
    {test_filter_chain_no_matches, "test_filter_chain_no_matches"},
    {test_filter_chain_single_match, "test_filter_chain_single_match"},

    // Memory management tests
    {test_filter_memory_consistency, "test_filter_memory_consistency"},
    {test_filter_iterator_ownership, "test_filter_iterator_ownership"},
    {test_filter_chain_memory_management, "test_filter_chain_memory_management"},

    // Performance and stress tests
    {test_filter_large_dataset, "test_filter_large_dataset"},
    {test_filter_complex_chaining, "test_filter_complex_chaining"},

    // Enhanced helper tests
    {test_filter_helper_validation, "test_filter_helper_validation"},
};

//==============================================================================
// Main Test Runner
//==============================================================================

int main(void)
{
    const int num_tests = sizeof(tests) / sizeof(tests[0]);
    int failed = 0;
    int passed = 0;

    printf("Running %d filter iterator tests...\n\n", num_tests);

    for (int i = 0; i < num_tests; i++)
    {
        printf("Running %s... ", tests[i].name);
        fflush(stdout);

        if (tests[i].func() == TEST_SUCCESS)
        {
            printf("PASSED\n");
            passed++;
        }
        else
        {
            printf("FAILED\n");
            failed++;
        }
    }

    printf("\n=== Filter Iterator Test Results ===\n");
    printf("Total tests: %d\n", num_tests);
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", failed);

    if (failed == 0)
    {
        printf("\n🎉 All filter iterator tests passed!\n");
        return 0;
    }

    printf("\n❌ %d test(s) failed.\n", failed);
    return 1;
}