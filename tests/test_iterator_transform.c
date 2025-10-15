//
// Created by zack on 9/3/25.
//
// Comprehensive test suite for transform iterator functionality.
// Tests cover basic transformation, chaining, error handling, memory management,
// and integration with different data structures and iterator types.

#include <stdio.h>
#include <stdlib.h>

#include "containers/doublylinkedlist.h"
#include "containers/iterator.h"
#include "TestAssert.h"
#include "TestHelpers.h"

//==============================================================================
// Helper Functions for Transform Iterator Tests
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
// Basic Transform Iterator Tests
//==============================================================================

/**
 * Test basic transform iterator functionality with double_value.
 */
static int test_transform_basic_double(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 5);
    ASSERT_NOT_NULL(list);

    // Create base iterator
    ANVIterator base_it = anv_dll_iterator(list);
    ASSERT_TRUE(base_it.is_valid(&base_it));
    ASSERT_TRUE(base_it.has_next(&base_it));

    // Create transform iterator that doubles each value
    ANVIterator transform_it = anv_iterator_transform(&base_it, &alloc, double_value, true);
    ASSERT_TRUE(transform_it.is_valid(&transform_it));
    ASSERT_TRUE(transform_it.has_next(&transform_it));

    // Expected: [1,2,3,4,5] -> [2,4,6,8,10]
    const int expected[] = {2, 4, 6, 8, 10};
    int actual[5];
    const int count = collect_values(&transform_it, actual, 5);

    ASSERT_EQ(count, 5);
    ASSERT_TRUE(verify_values(actual, expected, count, "basic_double"));

    // Iterator should be exhausted
    ASSERT_FALSE(transform_it.has_next(&transform_it));
    ASSERT_NULL(transform_it.get(&transform_it));

    // Cleanup
    transform_it.destroy(&transform_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test transform iterator with square function.
 */
static int test_transform_square(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 4);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator transform_it = anv_iterator_transform(&base_it, &alloc, square_func, true);

    ASSERT_TRUE(transform_it.is_valid(&transform_it));

    // Expected: [1,2,3,4] -> [1,4,9,16]
    const int expected[] = {1, 4, 9, 16};
    int actual[4];
    const int count = collect_values(&transform_it, actual, 4);

    ASSERT_EQ(count, 4);
    ASSERT_TRUE(verify_values(actual, expected, count, "square_transform"));

    transform_it.destroy(&transform_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test transform iterator with add_one function.
 */
static int test_transform_add_one(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 3);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator transform_it = anv_iterator_transform(&base_it, &alloc, add_one, true);

    ASSERT_TRUE(transform_it.is_valid(&transform_it));

    // Expected: [1,2,3] -> [2,3,4]
    const int expected[] = {2, 3, 4};
    int actual[3];
    const int count = collect_values(&transform_it, actual, 3);

    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_values(actual, expected, count, "add_one_transform"));

    transform_it.destroy(&transform_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Transform Iterator Chaining Tests
//==============================================================================

/**
 * Test chaining transform iterators: double then add one.
 */
static int test_transform_chain_double_add_one(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 3);
    ASSERT_NOT_NULL(list);

    // Create base iterator
    ANVIterator base_it = anv_dll_iterator(list);

    // Chain transforms: first double, then add one
    ANVIterator double_it = anv_iterator_transform(&base_it, &alloc, double_value, true);
    ANVIterator add_one_it = anv_iterator_transform(&double_it, &alloc, add_one, true);

    ASSERT_TRUE(add_one_it.is_valid(&add_one_it));

    // Expected: [1,2,3] -> [2,4,6] -> [3,5,7]
    const int expected[] = {3, 5, 7};
    int actual[3];
    const int count = collect_values(&add_one_it, actual, 3);

    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_values(actual, expected, count, "chain_double_add_one"));

    add_one_it.destroy(&add_one_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test chaining transform iterators: square then multiply by 3.
 */
static int test_transform_chain_square_multiply(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 3);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator square_it = anv_iterator_transform(&base_it, &alloc, square_func, true);
    ANVIterator multiply_it = anv_iterator_transform(&square_it, &alloc, multiply_by_three, true);

    // Expected: [1,2,3] -> [1,4,9] -> [3,12,27]
    const int expected[] = {3, 12, 27};
    int actual[3];
    const int count = collect_values(&multiply_it, actual, 3);

    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_values(actual, expected, count, "chain_square_multiply"));

    multiply_it.destroy(&multiply_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test complex chaining: three transforms in sequence.
 */
static int test_transform_triple_chain(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 2);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator double_it = anv_iterator_transform(&base_it, &alloc, double_value, true);
    ANVIterator add_five_it = anv_iterator_transform(&double_it, &alloc, add_five, true);
    ANVIterator square_it = anv_iterator_transform(&add_five_it, &alloc, square_func, true);

    // Expected: [1,2] -> [2,4] -> [7,9] -> [49,81]
    const int expected[] = {49, 81};
    int actual[2];
    const int count = collect_values(&square_it, actual, 2);

    ASSERT_EQ(count, 2);
    ASSERT_TRUE(verify_values(actual, expected, count, "triple_chain"));

    square_it.destroy(&square_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Edge Case Tests
//==============================================================================

/**
 * Test transform iterator with empty input.
 */
static int test_transform_empty_input(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    ASSERT_NOT_NULL(list);

    // Create iterator on empty list
    ANVIterator base_it = anv_dll_iterator(list);
    ASSERT_FALSE(base_it.has_next(&base_it));

    // Create transform iterator
    ANVIterator transform_it = anv_iterator_transform(&base_it, &alloc, double_value, true);
    ASSERT_TRUE(transform_it.is_valid(&transform_it));
    ASSERT_FALSE(transform_it.has_next(&transform_it));
    ASSERT_NULL(transform_it.get(&transform_it));

    // Test that next() fails appropriately
    ASSERT_EQ(transform_it.next(&transform_it), -1);

    transform_it.destroy(&transform_it);
    anv_dll_destroy(list, false);
    return TEST_SUCCESS;
}

/**
 * Test transform iterator with single element.
 */
static int test_transform_single_element(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 1);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator transform_it = anv_iterator_transform(&base_it, &alloc, multiply_by_three, true);

    ASSERT_TRUE(transform_it.has_next(&transform_it));

    const int* value = transform_it.get(&transform_it);
    ASSERT_NOT_NULL(value);
    ASSERT_EQ(*value, 3); // 1 * 3 = 3

    transform_it.next(&transform_it);
    ASSERT_FALSE(transform_it.has_next(&transform_it));
    ASSERT_NULL(transform_it.get(&transform_it));

    transform_it.destroy(&transform_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Error Handling Tests
//==============================================================================

/**
 * Test transform iterator with invalid inputs.
 */
static int test_transform_invalid_inputs(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 1);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);

    // Test with NULL iterator
    ANVIterator invalid_it1 = anv_iterator_transform(NULL, &alloc, double_value, true);
    ASSERT_FALSE(invalid_it1.is_valid(&invalid_it1));
    ASSERT_FALSE(invalid_it1.has_next(&invalid_it1));
    ASSERT_NULL(invalid_it1.get(&invalid_it1));

    // Test with NULL transform function
    ANVIterator invalid_it2 = anv_iterator_transform(&base_it, &alloc, NULL, true);
    ASSERT_FALSE(invalid_it2.is_valid(&invalid_it2));

    // Test with NULL allocator
    ANVIterator base_it2 = anv_dll_iterator(list);
    ANVIterator invalid_it3 = anv_iterator_transform(&base_it2, NULL, double_value, true);
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
 * Test transform iterator operations on invalid iterator.
 */
static int test_transform_operations_on_invalid(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create invalid transform iterator
    ANVIterator invalid_it = anv_iterator_transform(NULL, &alloc, double_value, false);
    ASSERT_FALSE(invalid_it.is_valid(&invalid_it));

    // All operations should fail gracefully
    ASSERT_EQ(invalid_it.next(&invalid_it), -1);
    ASSERT_EQ(invalid_it.prev(&invalid_it), -1); // Transform doesn't support prev
    ASSERT_FALSE(invalid_it.has_next(&invalid_it));
    ASSERT_FALSE(invalid_it.has_prev(&invalid_it)); // Transform doesn't support has_prev
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
static int test_transform_get_next_separation(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 3);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator transform_it = anv_iterator_transform(&base_it, &alloc, add_ten_func, true);

    ASSERT_TRUE(transform_it.is_valid(&transform_it));

    // Test get without advancing
    const int* value1 = transform_it.get(&transform_it);
    ASSERT_NOT_NULL(value1);
    ASSERT_EQ(*value1, 11); // 1 + 10 = 11

    // Get again - should return same value
    const int* value2 = transform_it.get(&transform_it);
    ASSERT_NOT_NULL(value2);
    ASSERT_EQ(*value2, 11);

    // Store value before advancing
    const int first_value = *value1;

    // Now advance
    ASSERT_EQ(transform_it.next(&transform_it), 0);
    const int* value3 = transform_it.get(&transform_it);
    ASSERT_NOT_NULL(value3);
    ASSERT_EQ(*value3, 12); // 2 + 10 = 12
    ASSERT_NOT_EQ(first_value, *value3);

    transform_it.destroy(&transform_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test next() return codes.
 */
static int test_transform_next_return_codes(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 2);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator transform_it = anv_iterator_transform(&base_it, &alloc, double_value, true);

    ASSERT_TRUE(transform_it.is_valid(&transform_it));

    // First next should succeed
    ASSERT_EQ(transform_it.next(&transform_it), 0);

    // Second next should succeed
    ASSERT_EQ(transform_it.next(&transform_it), 0);

    // Third next should fail (at end)
    ASSERT_EQ(transform_it.next(&transform_it), -1);

    transform_it.destroy(&transform_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test unsupported bidirectional operations.
 */
static int test_transform_unsupported_operations(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 3);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator transform_it = anv_iterator_transform(&base_it, &alloc, double_value, true);

    ASSERT_TRUE(transform_it.is_valid(&transform_it));

    // Transform iterator should not support bidirectional operations
    ASSERT_FALSE(transform_it.has_prev(&transform_it));
    ASSERT_EQ(transform_it.prev(&transform_it), -1); // Returns -1 for unsupported

    // Reset should be safe but ineffective
    transform_it.reset(&transform_it);

    // Should still be valid after unsupported operations
    ASSERT_TRUE(transform_it.is_valid(&transform_it));

    transform_it.destroy(&transform_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Integration Tests with Range Iterator
//==============================================================================

/**
 * Test transform iterator with range iterator as input.
 */
static int test_transform_with_range_iterator(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator: [2, 4, 6, 8]
    ANVIterator range_it = anv_iterator_range(2, 10, 2, &alloc);
    ASSERT_TRUE(range_it.is_valid(&range_it));

    // Apply square transform
    ANVIterator transform_it = anv_iterator_transform(&range_it, &alloc, square_func, true);
    ASSERT_TRUE(transform_it.is_valid(&transform_it));

    // Expected: [2,4,6,8] -> [4,16,36,64]
    const int expected[] = {4, 16, 36, 64};
    int actual[4];
    const int count = collect_values(&transform_it, actual, 4);

    ASSERT_EQ(count, 4);
    ASSERT_TRUE(verify_values(actual, expected, count, "transform_range"));

    transform_it.destroy(&transform_it);
    return TEST_SUCCESS;
}

/**
 * Test chaining range -> transform -> transform.
 */
static int test_range_transform_chain(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator: [1, 2, 3]
    ANVIterator range_it = anv_iterator_range(1, 4, 1, &alloc);

    // Chain: range -> double -> add_five
    ANVIterator double_it = anv_iterator_transform(&range_it, &alloc, double_value, true);
    ANVIterator add_five_it = anv_iterator_transform(&double_it, &alloc, add_five, true);

    // Expected: [1,2,3] -> [2,4,6] -> [7,9,11]
    const int expected[] = {7, 9, 11};
    int actual[3];
    const int count = collect_values(&add_five_it, actual, 3);

    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_values(actual, expected, count, "range_transform_chain"));

    add_five_it.destroy(&add_five_it);
    return TEST_SUCCESS;
}

//==============================================================================
// Memory Management Tests
//==============================================================================

/**
 * Test memory consistency across operations.
 */
static int test_transform_memory_consistency(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 3);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator transform_it = anv_iterator_transform(&base_it, &alloc, multiply_by_three, true);

    // Get multiple references to the same value
    const int* ptr1 = transform_it.get(&transform_it);
    const int* ptr2 = transform_it.get(&transform_it);
    const int* ptr3 = transform_it.get(&transform_it);

    // All should point to the same memory and have the same value
    ASSERT_EQ(ptr1, ptr2);
    ASSERT_EQ(ptr2, ptr3);
    ASSERT_EQ(*ptr1, 3); // 1 * 3 = 3
    ASSERT_EQ(*ptr2, 3);
    ASSERT_EQ(*ptr3, 3);

    // Store the value before moving to next
    const int first_value = *ptr1;

    // Move to next and verify new value
    transform_it.next(&transform_it);
    const int* ptr4 = transform_it.get(&transform_it);
    ASSERT_EQ(*ptr4, 6); // 2 * 3 = 6

    // The transform iterator may use different memory for each value
    // This is normal behavior - what matters is the values are correct
    ASSERT_NOT_EQ(first_value, *ptr4); // Values should be different

    transform_it.destroy(&transform_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test that transform iterator properly manages base iterator lifecycle.
 */
static int test_transform_iterator_ownership(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 2);
    ASSERT_NOT_NULL(list);

    // Create base iterator
    ANVIterator base_it = anv_dll_iterator(list);
    ASSERT_TRUE(base_it.is_valid(&base_it));

    // Create transform iterator (takes ownership of base_it)
    ANVIterator transform_it = anv_iterator_transform(&base_it, &alloc, double_value, true);
    ASSERT_TRUE(transform_it.is_valid(&transform_it));

    // Verify transform works
    const int* value = transform_it.get(&transform_it);
    ASSERT_NOT_NULL(value);
    ASSERT_EQ(*value, 2); // 1 * 2 = 2

    // When we destroy transform iterator, it should clean up base iterator too
    transform_it.destroy(&transform_it);

    // Note: We should not access base_it after this point as it's been destroyed

    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Performance and Stress Tests
//==============================================================================

/**
 * Test transform iterator with large dataset.
 */
static int test_transform_large_dataset(void)
{
    ANVAllocator alloc = create_int_allocator();
    const int SIZE = 1000;

    ANVDoublyLinkedList* list = create_test_list(&alloc, SIZE);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator transform_it = anv_iterator_transform(&base_it, &alloc, double_value, true);

    ASSERT_TRUE(transform_it.is_valid(&transform_it));

    int count = 0;
    int expected = 2; // First element (1) doubled

    while (transform_it.has_next(&transform_it))
    {
        const int* value = transform_it.get(&transform_it);
        ASSERT_NOT_NULL(value);
        ASSERT_EQ(*value, expected);

        count++;
        expected += 2; // Next expected value (each input increments by 1, doubled)
        transform_it.next(&transform_it);
    }

    ASSERT_EQ(count, SIZE);
    ASSERT_FALSE(transform_it.has_next(&transform_it));

    transform_it.destroy(&transform_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test deep chaining performance.
 */
static int test_transform_deep_chaining(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 10);
    ASSERT_NOT_NULL(list);

    // Create a deep chain of 5 transforms
    ANVIterator it1 = anv_dll_iterator(list);
    ANVIterator it2 = anv_iterator_transform(&it1, &alloc, add_one, true);
    ANVIterator it3 = anv_iterator_transform(&it2, &alloc, double_value, true);
    ANVIterator it4 = anv_iterator_transform(&it3, &alloc, add_five, true);
    ANVIterator it5 = anv_iterator_transform(&it4, &alloc, multiply_by_three, true);
    ANVIterator final_it = anv_iterator_transform(&it5, &alloc, square_func, true);

    ASSERT_TRUE(final_it.is_valid(&final_it));

    // Just verify it works end-to-end for first element
    // Input: 1 -> +1 -> *2 -> +5 -> *3 -> ^2
    // 1 -> 2 -> 4 -> 9 -> 27 -> 729
    const int* first_value = final_it.get(&final_it);
    ASSERT_NOT_NULL(first_value);
    ASSERT_EQ(*first_value, 729);

    // Verify we can iterate through all elements
    int count = 0;
    while (final_it.has_next(&final_it))
    {
        const int* value = final_it.get(&final_it);
        ASSERT_NOT_NULL(value);
        count++;
        final_it.next(&final_it);
    }

    ASSERT_EQ(count, 10);

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
 * Test helper function validation with transform iterator.
 */
static int test_transform_helper_validation(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 5);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator transform_it = anv_iterator_transform(&base_it, &alloc, add_five, true);

    ASSERT_TRUE(transform_it.is_valid(&transform_it));

    int values[5];
    const int count = collect_values_with_validation(&transform_it, values, 5);

    // Should successfully collect all 5 values
    ASSERT_EQ(count, 5);

    const int expected[] = {6, 7, 8, 9, 10}; // [1,2,3,4,5] + 5 each
    ASSERT_TRUE(verify_values(values, expected, count, "helper_validation"));

    transform_it.destroy(&transform_it);
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
    {test_transform_basic_double, "test_transform_basic_double"},
    {test_transform_square, "test_transform_square"},
    {test_transform_add_one, "test_transform_add_one"},

    // Chaining tests
    {test_transform_chain_double_add_one, "test_transform_chain_double_add_one"},
    {test_transform_chain_square_multiply, "test_transform_chain_square_multiply"},
    {test_transform_triple_chain, "test_transform_triple_chain"},

    // Edge case tests
    {test_transform_empty_input, "test_transform_empty_input"},
    {test_transform_single_element, "test_transform_single_element"},

    // Error handling tests
    {test_transform_invalid_inputs, "test_transform_invalid_inputs"},
    {test_transform_operations_on_invalid, "test_transform_operations_on_invalid"},

    // API behavior tests
    {test_transform_get_next_separation, "test_transform_get_next_separation"},
    {test_transform_next_return_codes, "test_transform_next_return_codes"},
    {test_transform_unsupported_operations, "test_transform_unsupported_operations"},

    // Integration tests
    {test_transform_with_range_iterator, "test_transform_with_range_iterator"},
    {test_range_transform_chain, "test_range_transform_chain"},

    // Memory management tests
    {test_transform_memory_consistency, "test_transform_memory_consistency"},
    {test_transform_iterator_ownership, "test_transform_iterator_ownership"},

    // Performance and stress tests
    {test_transform_large_dataset, "test_transform_large_dataset"},
    {test_transform_deep_chaining, "test_transform_deep_chaining"},

    // Enhanced helper tests
    {test_transform_helper_validation, "test_transform_helper_validation"},
};

//==============================================================================
// Main Test Runner
//==============================================================================

int main(void)
{
    const int num_tests = sizeof(tests) / sizeof(tests[0]);
    int failed = 0;
    int passed = 0;

    printf("Running %d transform iterator tests...\n\n", num_tests);

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

    printf("\n=== Transform Iterator Test Results ===\n");
    printf("Total tests: %d\n", num_tests);
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", failed);

    if (failed == 0)
    {
        printf("\n🎉 All transform iterator tests passed!\n");
        return 0;
    }

    printf("\n❌ %d test(s) failed.\n", failed);
    return 1;
}