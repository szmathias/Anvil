//
// Created by zack on 9/3/25.
//
// Comprehensive test suite for iterator chaining functionality.
// Tests cover all combinations of range, filter, and transform iterators,
// error handling, memory management, and complex multistep chains.

#include <stdio.h>
#include <stdlib.h>

#include "containers/doublylinkedlist.h"
#include "containers/iterator.h"
#include "TestAssert.h"
#include "TestHelpers.h"

//==============================================================================
// Helper Functions for Iterator Chaining Tests
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

//==============================================================================
// Range → Filter Chaining Tests
//==============================================================================

/**
 * Test range iterator chained with filter for even numbers.
 */
static int test_range_filter_even(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    ANVIterator range_it = anv_iterator_range(1, 11, 1, &alloc);
    ASSERT_TRUE(range_it.is_valid(&range_it));

    // Chain with even filter
    ANVIterator filter_it = anv_iterator_filter(&range_it, &alloc, is_even);
    ASSERT_TRUE(filter_it.is_valid(&filter_it));

    // Expected: [1,2,3,4,5,6,7,8,9,10] → [2,4,6,8,10]
    const int expected[] = {2, 4, 6, 8, 10};
    int actual[5];
    const int count = collect_values(&filter_it, actual, 5);

    ASSERT_EQ(count, 5);
    ASSERT_TRUE(verify_values(actual, expected, count, "range_filter_even"));

    filter_it.destroy(&filter_it);
    return TEST_SUCCESS;
}

/**
 * Test range iterator with step chained with divisible by 3 filter.
 */
static int test_range_step_filter_div3(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range [2, 5, 8, 11, 14, 17, 20]
    ANVIterator range_it = anv_iterator_range(2, 21, 3, &alloc);
    ASSERT_TRUE(range_it.is_valid(&range_it));

    // Chain with divisible by 3 filter
    ANVIterator filter_it = anv_iterator_filter(&range_it, &alloc, is_divisible_by_3);
    ASSERT_TRUE(filter_it.is_valid(&filter_it));

    // Expected: [2,5,8,11,14,17,20] → [] (none divisible by 3)
    ASSERT_FALSE(filter_it.has_next(&filter_it));
    ASSERT_NULL(filter_it.get(&filter_it));

    filter_it.destroy(&filter_it);
    return TEST_SUCCESS;
}

/**
 * Test range iterator chained with greater than 5 filter.
 */
static int test_range_filter_greater_than_5(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range [1, 2, 3, 4, 5, 6, 7, 8]
    ANVIterator range_it = anv_iterator_range(1, 9, 1, &alloc);

    // Chain with greater than 5 filter
    ANVIterator filter_it = anv_iterator_filter(&range_it, &alloc, is_greater_than_five);

    // Expected: [1,2,3,4,5,6,7,8] → [6,7,8]
    const int expected[] = {6, 7, 8};
    int actual[3];
    const int count = collect_values(&filter_it, actual, 3);

    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_values(actual, expected, count, "range_filter_gt5"));

    filter_it.destroy(&filter_it);
    return TEST_SUCCESS;
}

//==============================================================================
// Range → Transform Chaining Tests
//==============================================================================

/**
 * Test range iterator chained with double transform.
 */
static int test_range_transform_double(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range [1, 2, 3, 4, 5]
    ANVIterator range_it = anv_iterator_range(1, 6, 1, &alloc);

    // Chain with double transform
    ANVIterator transform_it = anv_iterator_transform(&range_it, &alloc, double_value, true);

    // Expected: [1,2,3,4,5] → [2,4,6,8,10]
    const int expected[] = {2, 4, 6, 8, 10};
    int actual[5];
    const int count = collect_values(&transform_it, actual, 5);

    ASSERT_EQ(count, 5);
    ASSERT_TRUE(verify_values(actual, expected, count, "range_transform_double"));

    transform_it.destroy(&transform_it);
    return TEST_SUCCESS;
}

/**
 * Test range iterator chained with square transform.
 */
static int test_range_transform_square(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range [2, 4, 6]
    ANVIterator range_it = anv_iterator_range(2, 7, 2, &alloc);

    // Chain with square transform
    ANVIterator transform_it = anv_iterator_transform(&range_it, &alloc, square_func, true);

    // Expected: [2,4,6] → [4,16,36]
    const int expected[] = {4, 16, 36};
    int actual[3];
    const int count = collect_values(&transform_it, actual, 3);

    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_values(actual, expected, count, "range_transform_square"));

    transform_it.destroy(&transform_it);
    return TEST_SUCCESS;
}

/**
 * Test range iterator chained with add_ten transform.
 */
static int test_range_transform_add_ten(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range [1, 2, 3, 4]
    ANVIterator range_it = anv_iterator_range(1, 5, 1, &alloc);

    // Chain with add_ten transform
    ANVIterator transform_it = anv_iterator_transform(&range_it, &alloc, add_ten_func, true);

    // Expected: [1,2,3,4] → [11,12,13,14]
    const int expected[] = {11, 12, 13, 14};
    int actual[4];
    const int count = collect_values(&transform_it, actual, 4);

    ASSERT_EQ(count, 4);
    ASSERT_TRUE(verify_values(actual, expected, count, "range_transform_add_ten"));

    transform_it.destroy(&transform_it);
    return TEST_SUCCESS;
}

//==============================================================================
// Filter → Transform Chaining Tests
//==============================================================================

/**
 * Test filter iterator chained with transform (using list base).
 */
static int test_filter_transform_even_double(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 6);
    ASSERT_NOT_NULL(list);

    // Create base iterator
    ANVIterator base_it = anv_dll_iterator(list);

    // Chain: filter even → transform double
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_even);
    ANVIterator transform_it = anv_iterator_transform(&filter_it, &alloc, double_value, true);

    // Expected: [1,2,3,4,5,6] → [2,4,6] → [4,8,12]
    const int expected[] = {4, 8, 12};
    int actual[3];
    const int count = collect_values(&transform_it, actual, 3);

    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_values(actual, expected, count, "filter_transform_even_double"));

    transform_it.destroy(&transform_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test filter iterator chained with square transform.
 */
static int test_filter_transform_odd_square(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 5);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);

    // Chain: filter odd → transform square
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_odd);
    ANVIterator transform_it = anv_iterator_transform(&filter_it, &alloc, square_func, true);

    // Expected: [1,2,3,4,5] → [1,3,5] → [1,9,25]
    const int expected[] = {1, 9, 25};
    int actual[3];
    const int count = collect_values(&transform_it, actual, 3);

    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_values(actual, expected, count, "filter_transform_odd_square"));

    transform_it.destroy(&transform_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test filter with no matches chained with transform.
 */
static int test_filter_transform_no_matches(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create list with only odd numbers
    const int odd_values[] = {1, 3, 5, 7};
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    for (int i = 0; i < 4; i++)
    {
        int* val = malloc(sizeof(int));
        *val = odd_values[i];
        anv_dll_push_back(list, val);
    }

    ANVIterator base_it = anv_dll_iterator(list);

    // Chain: filter even (no matches) → transform double
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_even);
    ANVIterator transform_it = anv_iterator_transform(&filter_it, &alloc, double_value, true);

    // Expected: [1,3,5,7] → [] → []
    ASSERT_FALSE(transform_it.has_next(&transform_it));
    ASSERT_NULL(transform_it.get(&transform_it));

    transform_it.destroy(&transform_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Transform → Filter Chaining Tests
//==============================================================================

/**
 * Test transform iterator chained with filter.
 */
static int test_transform_filter_add_one_even(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 5);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);

    // Chain: transform add_one → filter even
    ANVIterator transform_it = anv_iterator_transform(&base_it, &alloc, add_one, true);
    ANVIterator filter_it = anv_iterator_filter(&transform_it, &alloc, is_even);

    // Expected: [1,2,3,4,5] → [2,3,4,5,6] → [2,4,6]
    const int expected[] = {2, 4, 6};
    int actual[3];
    const int count = collect_values(&filter_it, actual, 3);

    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_values(actual, expected, count, "transform_filter_add_one_even"));

    filter_it.destroy(&filter_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test transform square chained with greater than 10 filter.
 */
static int test_transform_filter_square_gt10(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 5);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);

    // Chain: transform square → filter > 10
    ANVIterator transform_it = anv_iterator_transform(&base_it, &alloc, square_func, true);
    ANVIterator filter_it = anv_iterator_filter(&transform_it, &alloc, is_greater_than_10);

    // Expected: [1,2,3,4,5] → [1,4,9,16,25] → [16,25]
    const int expected[] = {16, 25};
    int actual[2];
    const int count = collect_values(&filter_it, actual, 2);

    ASSERT_EQ(count, 2);
    ASSERT_TRUE(verify_values(actual, expected, count, "transform_filter_square_gt10"));

    filter_it.destroy(&filter_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test transform multiply by 3 chained with divisible by 6 filter.
 */
static int test_transform_filter_multiply3_div6(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 4);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);

    // Chain: transform multiply by 3 → filter divisible by 6
    ANVIterator transform_it = anv_iterator_transform(&base_it, &alloc, multiply_by_three, true);
    ANVIterator filter_it = anv_iterator_filter(&transform_it, &alloc, is_divisible_by_six);

    // Expected: [1,2,3,4] → [3,6,9,12] → [6,12]
    const int expected[] = {6, 12};
    int actual[2];
    const int count = collect_values(&filter_it, actual, 2);

    ASSERT_EQ(count, 2);
    ASSERT_TRUE(verify_values(actual, expected, count, "transform_filter_multiply3_div6"));

    filter_it.destroy(&filter_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Complex Multi-Step Chaining Tests
//==============================================================================

/**
 * Test range → filter → transform chain.
 */
static int test_range_filter_transform_chain(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    ANVIterator range_it = anv_iterator_range(1, 11, 1, &alloc);

    // Chain: range → filter even → transform square
    ANVIterator filter_it = anv_iterator_filter(&range_it, &alloc, is_even);
    ANVIterator transform_it = anv_iterator_transform(&filter_it, &alloc, square_func, true);

    // Expected: [1,2,3,4,5,6,7,8,9,10] → [2,4,6,8,10] → [4,16,36,64,100]
    const int expected[] = {4, 16, 36, 64, 100};
    int actual[5];
    const int count = collect_values(&transform_it, actual, 5);

    ASSERT_EQ(count, 5);
    ASSERT_TRUE(verify_values(actual, expected, count, "range_filter_transform"));

    transform_it.destroy(&transform_it);
    return TEST_SUCCESS;
}

/**
 * Test range → transform → filter chain.
 */
static int test_range_transform_filter_chain(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range [1, 2, 3, 4, 5, 6, 7, 8]
    ANVIterator range_it = anv_iterator_range(1, 9, 1, &alloc);

    // Chain: range → transform add_ten → filter divisible by 3
    ANVIterator transform_it = anv_iterator_transform(&range_it, &alloc, add_ten_func, true);
    ANVIterator filter_it = anv_iterator_filter(&transform_it, &alloc, is_divisible_by_3);

    // Expected: [1,2,3,4,5,6,7,8] → [11,12,13,14,15,16,17,18] → [12,15,18]
    const int expected[] = {12, 15, 18};
    int actual[3];
    const int count = collect_values(&filter_it, actual, 3);

    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_values(actual, expected, count, "range_transform_filter"));

    filter_it.destroy(&filter_it);
    return TEST_SUCCESS;
}

/**
 * Test range → filter → transform → filter chain.
 */
static int test_range_filter_transform_filter_chain(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    ANVIterator range_it = anv_iterator_range(1, 11, 1, &alloc);

    // Chain: range → filter odd → transform square → filter > 20
    ANVIterator filter_odd = anv_iterator_filter(&range_it, &alloc, is_odd);
    ANVIterator transform_it = anv_iterator_transform(&filter_odd, &alloc, square_func, true);
    ANVIterator filter_gt20 = anv_iterator_filter(&transform_it, &alloc, is_greater_than_20);

    // Expected: [1,2,3,4,5,6,7,8,9,10] → [1,3,5,7,9] → [1,9,25,49,81] → [25,49,81]
    const int expected[] = {25, 49, 81};
    int actual[3];
    const int count = collect_values(&filter_gt20, actual, 3);

    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_values(actual, expected, count, "range_filter_transform_filter"));

    filter_gt20.destroy(&filter_gt20);
    return TEST_SUCCESS;
}

/**
 * Test range → transform → transform → filter chain.
 */
static int test_range_transform_transform_filter_chain(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range [1, 2, 3, 4]
    ANVIterator range_it = anv_iterator_range(1, 5, 1, &alloc);

    // Chain: range → transform double → transform add_five → filter > 10
    ANVIterator transform_double = anv_iterator_transform(&range_it, &alloc, double_value, true);
    ANVIterator transform_add5 = anv_iterator_transform(&transform_double, &alloc, add_five, true);
    ANVIterator filter_gt10 = anv_iterator_filter(&transform_add5, &alloc, is_greater_than_10);

    // Expected: [1,2,3,4] → [2,4,6,8] → [7,9,11,13] → [11,13]
    const int expected[] = {11, 13};
    int actual[2];
    const int count = collect_values(&filter_gt10, actual, 2);

    ASSERT_EQ(count, 2);
    ASSERT_TRUE(verify_values(actual, expected, count, "range_transform_transform_filter"));

    filter_gt10.destroy(&filter_gt10);
    return TEST_SUCCESS;
}

/**
 * Test deeply nested chain with all iterator types.
 */
static int test_deep_nested_chain(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12]
    ANVIterator range_it = anv_iterator_range(1, 13, 1, &alloc);

    // Chain: range → filter even → transform add_one → filter divisible by 3 → transform square
    ANVIterator filter_even = anv_iterator_filter(&range_it, &alloc, is_even);
    ANVIterator transform_add1 = anv_iterator_transform(&filter_even, &alloc, add_one, true);
    ANVIterator filter_div3 = anv_iterator_filter(&transform_add1, &alloc, is_divisible_by_3);
    ANVIterator transform_square = anv_iterator_transform(&filter_div3, &alloc, square_func, true);

    // Expected: [1..12] → [2,4,6,8,10,12] → [3,5,7,9,11,13] → [3,9] → [9,81]
    const int expected[] = {9, 81};
    int actual[2];
    const int count = collect_values(&transform_square, actual, 2);

    ASSERT_EQ(count, 2);
    ASSERT_TRUE(verify_values(actual, expected, count, "deep_nested_chain"));

    transform_square.destroy(&transform_square);
    return TEST_SUCCESS;
}

//==============================================================================
// Edge Cases and Error Handling Tests
//==============================================================================

/**
 * Test empty chain propagation.
 */
static int test_empty_chain_propagation(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range with only odd numbers, then filter for even
    ANVIterator range_it = anv_iterator_range(1, 10, 2, &alloc); // [1,3,5,7,9]

    // Chain: range (odd) → filter even → transform double
    ANVIterator filter_it = anv_iterator_filter(&range_it, &alloc, is_even);
    ANVIterator transform_it = anv_iterator_transform(&filter_it, &alloc, double_value, true);

    // Expected: [1,3,5,7,9] → [] → []
    ASSERT_FALSE(transform_it.has_next(&transform_it));
    ASSERT_NULL(transform_it.get(&transform_it));
    ASSERT_EQ(transform_it.next(&transform_it), -1);

    transform_it.destroy(&transform_it);
    return TEST_SUCCESS;
}

/**
 * Test single element chain.
 */
static int test_single_element_chain(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range with single element
    ANVIterator range_it = anv_iterator_range(4, 5, 1, &alloc); // [4]

    // Chain: range → filter even → transform square
    ANVIterator filter_it = anv_iterator_filter(&range_it, &alloc, is_even);
    ANVIterator transform_it = anv_iterator_transform(&filter_it, &alloc, square_func, true);

    // Expected: [4] → [4] → [16]
    ASSERT_TRUE(transform_it.has_next(&transform_it));
    const int* value = transform_it.get(&transform_it);
    ASSERT_NOT_NULL(value);
    ASSERT_EQ(*value, 16);

    transform_it.next(&transform_it);
    ASSERT_FALSE(transform_it.has_next(&transform_it));
    ASSERT_NULL(transform_it.get(&transform_it));

    transform_it.destroy(&transform_it);
    return TEST_SUCCESS;
}

/**
 * Test chaining with invalid intermediate results.
 */
static int test_chain_invalid_intermediate(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range [100, 200, 300]
    ANVIterator range_it = anv_iterator_range(100, 301, 100, &alloc);

    // Chain: range → filter (impossible condition) → transform
    ANVIterator filter_it = anv_iterator_filter(&range_it, &alloc, is_odd); // None match (all are even)
    ANVIterator transform_it = anv_iterator_transform(&filter_it, &alloc, double_value, true);

    // Should have no elements
    ASSERT_FALSE(transform_it.has_next(&transform_it));
    ASSERT_NULL(transform_it.get(&transform_it));

    transform_it.destroy(&transform_it);
    return TEST_SUCCESS;
}

//==============================================================================
// Integration Tests with List Iterator
//==============================================================================

/**
 * Test complex chain starting with list iterator.
 */
static int test_list_complex_chain(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 10);
    ASSERT_NOT_NULL(list);

    // Create base iterator from list
    ANVIterator base_it = anv_dll_iterator(list);

    // Chain: list → filter even → transform square → filter > 20
    ANVIterator filter_even = anv_iterator_filter(&base_it, &alloc, is_even);
    ANVIterator transform_square = anv_iterator_transform(&filter_even, &alloc, square_func, true);
    ANVIterator filter_gt20 = anv_iterator_filter(&transform_square, &alloc, is_greater_than_20);

    // Expected: [1,2,3,4,5,6,7,8,9,10] → [2,4,6,8,10] → [4,16,36,64,100] → [36,64,100]
    const int expected[] = {36, 64, 100};
    int actual[3];
    const int count = collect_values(&filter_gt20, actual, 3);

    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_values(actual, expected, count, "list_complex_chain"));

    filter_gt20.destroy(&filter_gt20);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test list iterator with multiple filters.
 */
static int test_list_multiple_filters(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 30);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);

    // Chain: list → filter even → filter divisible by 3 → filter > 10
    ANVIterator filter_even = anv_iterator_filter(&base_it, &alloc, is_even);
    ANVIterator filter_div3 = anv_iterator_filter(&filter_even, &alloc, is_divisible_by_3);
    ANVIterator filter_gt10 = anv_iterator_filter(&filter_div3, &alloc, is_greater_than_10);

    // Expected: numbers divisible by 6 and > 10: [12,18,24,30]
    const int expected[] = {12, 18, 24, 30};
    int actual[4];
    const int count = collect_values(&filter_gt10, actual, 4);

    ASSERT_EQ(count, 4);
    ASSERT_TRUE(verify_values(actual, expected, count, "list_multiple_filters"));

    filter_gt10.destroy(&filter_gt10);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test list iterator with multiple transforms.
 */
static int test_list_multiple_transforms(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 3);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);

    // Chain: list → transform double → transform add_one → transform multiply_by_three
    ANVIterator transform_double = anv_iterator_transform(&base_it, &alloc, double_value, true);
    ANVIterator transform_add1 = anv_iterator_transform(&transform_double, &alloc, add_one, true);
    ANVIterator transform_mult3 = anv_iterator_transform(&transform_add1, &alloc, multiply_by_three, true);

    // Expected: [1,2,3] → [2,4,6] → [3,5,7] → [9,15,21]
    const int expected[] = {9, 15, 21};
    int actual[3];
    const int count = collect_values(&transform_mult3, actual, 3);

    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_values(actual, expected, count, "list_multiple_transforms"));

    transform_mult3.destroy(&transform_mult3);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Memory Management and Performance Tests
//==============================================================================

/**
 * Test memory consistency in long chains.
 */
static int test_chain_memory_consistency(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 5);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_even);
    ANVIterator transform_it = anv_iterator_transform(&filter_it, &alloc, multiply_by_three, true);

    // Get multiple references to the same value
    const int* ptr1 = transform_it.get(&transform_it);
    const int* ptr2 = transform_it.get(&transform_it);

    ASSERT_NOT_NULL(ptr1);
    ASSERT_NOT_NULL(ptr2);
    ASSERT_EQ(*ptr1, *ptr2);
    ASSERT_EQ(*ptr1, 6); // 2 * 3 = 6

    // Store value before advancing
    const int first_value = *ptr1;

    // Move to next
    transform_it.next(&transform_it);
    const int* ptr3 = transform_it.get(&transform_it);
    ASSERT_NOT_NULL(ptr3);
    ASSERT_EQ(*ptr3, 12); // 4 * 3 = 12
    ASSERT_NOT_EQ(first_value, *ptr3);

    transform_it.destroy(&transform_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test chain ownership and cleanup.
 */
static int test_chain_ownership_cleanup(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create a chain of iterators
    ANVIterator range_it = anv_iterator_range(1, 5, 1, &alloc);
    ANVIterator filter_it = anv_iterator_filter(&range_it, &alloc, is_even);
    ANVIterator transform_it = anv_iterator_transform(&filter_it, &alloc, double_value, true);

    // Verify chain works
    ASSERT_TRUE(transform_it.is_valid(&transform_it));
    const int* value = transform_it.get(&transform_it);
    ASSERT_NOT_NULL(value);
    ASSERT_EQ(*value, 4); // 2 * 2 = 4

    // Destroying final iterator should clean up entire chain
    transform_it.destroy(&transform_it);

    // Note: All intermediate iterators should be cleaned up automatically
    return TEST_SUCCESS;
}

/**
 * Test performance with long chains.
 */
static int test_chain_performance(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create a moderately large range
    ANVIterator range_it = anv_iterator_range(1, 1001, 1, &alloc);

    // Create a complex chain that actually filters significantly
    ANVIterator filter_even = anv_iterator_filter(&range_it, &alloc, is_even);
    ANVIterator transform_double = anv_iterator_transform(&filter_even, &alloc, double_value, true);
    ANVIterator filter_div6 = anv_iterator_filter(&transform_double, &alloc, is_divisible_by_six);
    ANVIterator transform_add5 = anv_iterator_transform(&filter_div6, &alloc, add_five, true);

    // Count all results to verify chain works efficiently
    int count = 0;
    while (transform_add5.has_next(&transform_add5))
    {
        const int* value = transform_add5.get(&transform_add5);
        ASSERT_NOT_NULL(value);

        // Verify conditions are met: (original was even * 2) should be divisible by 6, then +5
        // So (*value - 5) should be divisible by 6
        ASSERT_TRUE((*value - 5) % 6 == 0); // Should be divisible by 6 after subtracting 5

        count++;
        transform_add5.next(&transform_add5);
    }

    // Should have found some matching elements, but significantly fewer
    // Even numbers: 500, doubled numbers divisible by 6: every 3rd even number = ~167
    ASSERT_TRUE(count > 0);
    ASSERT_TRUE(count < 200); // Should be much less than 500 even numbers

    transform_add5.destroy(&transform_add5);
    return TEST_SUCCESS;
}

//==============================================================================
// Enhanced Helper Function Tests
//==============================================================================

/**
 * Enhanced helper function with better validation for chains.
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

        // Only call next() if we're not at the last element we want to collect
        // or if there are more elements after this one
        if (count < max_count && it->has_next(it))
        {
            if (it->next(it) != 0)
            {
                // next() failed but has_next() was true - this is an error
                return -1;
            }
        }
        else
        {
            // We've collected enough elements or this is the last element
            // Try to advance, but don't treat failure as an error since we might be at the end
            it->next(it);
            break;
        }
    }
    return count;
}

/**
 * Test helper function validation with iterator chains.
 */
static int test_chain_helper_validation(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create a simple chain
    ANVIterator range_it = anv_iterator_range(1, 7, 1, &alloc);
    ANVIterator filter_it = anv_iterator_filter(&range_it, &alloc, is_even);

    int values[3];
    const int count = collect_values_with_validation(&filter_it, values, 3);

    // Should successfully collect all even values
    ASSERT_EQ(count, 3);

    const int expected[] = {2, 4, 6}; // Even numbers from [1,2,3,4,5,6]
    ASSERT_TRUE(verify_values(values, expected, count, "chain_helper_validation"));

    filter_it.destroy(&filter_it);
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
    // Range → Filter chaining tests
    {test_range_filter_even, "test_range_filter_even"},
    {test_range_step_filter_div3, "test_range_step_filter_div3"},
    {test_range_filter_greater_than_5, "test_range_filter_greater_than_5"},

    // Range → Transform chaining tests
    {test_range_transform_double, "test_range_transform_double"},
    {test_range_transform_square, "test_range_transform_square"},
    {test_range_transform_add_ten, "test_range_transform_add_ten"},

    // Filter → Transform chaining tests
    {test_filter_transform_even_double, "test_filter_transform_even_double"},
    {test_filter_transform_odd_square, "test_filter_transform_odd_square"},
    {test_filter_transform_no_matches, "test_filter_transform_no_matches"},

    // Transform → Filter chaining tests
    {test_transform_filter_add_one_even, "test_transform_filter_add_one_even"},
    {test_transform_filter_square_gt10, "test_transform_filter_square_gt10"},
    {test_transform_filter_multiply3_div6, "test_transform_filter_multiply3_div6"},

    // Complex multi-step chaining tests
    {test_range_filter_transform_chain, "test_range_filter_transform_chain"},
    {test_range_transform_filter_chain, "test_range_transform_filter_chain"},
    {test_range_filter_transform_filter_chain, "test_range_filter_transform_filter_chain"},
    {test_range_transform_transform_filter_chain, "test_range_transform_transform_filter_chain"},
    {test_deep_nested_chain, "test_deep_nested_chain"},

    // Edge cases and error handling
    {test_empty_chain_propagation, "test_empty_chain_propagation"},
    {test_single_element_chain, "test_single_element_chain"},
    {test_chain_invalid_intermediate, "test_chain_invalid_intermediate"},

    // Integration tests with list iterator
    {test_list_complex_chain, "test_list_complex_chain"},
    {test_list_multiple_filters, "test_list_multiple_filters"},
    {test_list_multiple_transforms, "test_list_multiple_transforms"},

    // Memory management and performance tests
    {test_chain_memory_consistency, "test_chain_memory_consistency"},
    {test_chain_ownership_cleanup, "test_chain_ownership_cleanup"},
    {test_chain_performance, "test_chain_performance"},

    // Enhanced helper tests
    {test_chain_helper_validation, "test_chain_helper_validation"},
};

//==============================================================================
// Main Test Runner
//==============================================================================

int main(void)
{
    const int num_tests = sizeof(tests) / sizeof(tests[0]);
    int failed = 0;
    int passed = 0;

    printf("Running %d iterator chaining tests...\n\n", num_tests);

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

    printf("\n=== Iterator Chaining Test Results ===\n");
    printf("Total tests: %d\n", num_tests);
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", failed);

    if (failed == 0)
    {
        printf("\n🎉 All iterator chaining tests passed!\n");
        return 0;
    }

    printf("\n❌ %d test(s) failed.\n", failed);
    return 1;
}