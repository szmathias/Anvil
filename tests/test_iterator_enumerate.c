//
// Created by zack on 9/15/25.
//
// Comprehensive test suite for enumerate iterator functionality.
// Tests cover basic iteration, edge cases, index tracking,
// error handling, and composition with other iterators.

#include <stdio.h>
#include <stdlib.h>

#include "containers/arraylist.h"
#include "containers/doublylinkedlist.h"
#include "containers/iterator.h"
#include "containers/queue.h"
#include "TestAssert.h"
#include "TestHelpers.h"

//==============================================================================
// Helper Functions for Enumerate Iterator Tests
//==============================================================================

/**
 * Helper function to collect all indexed elements from an enumerate iterator.
 * Returns the number of indexed elements collected.
 */
static int collect_indexed_elements(const ANVIterator* it, size_t* indices, int* values, const int max_count)
{
    int count = 0;
    while (it->has_next(it) && count < max_count)
    {
        const ANVIndexedElement* indexed = it->get(it);
        if (indexed && indexed->element)
        {
            indices[count] = indexed->index;
            values[count] = *(const int*)indexed->element;
            count++;
        }
        it->next(it);
    }
    return count;
}

/**
 * Helper function to verify indexed elements match expected values.
 */
static int verify_indexed_elements(const size_t* actual_indices, const int* actual_values,
                                  const size_t* expected_indices, const int* expected_values,
                                  const int count, const char* test_name)
{
    for (int i = 0; i < count; i++)
    {
        if (actual_indices[i] != expected_indices[i] || actual_values[i] != expected_values[i])
        {
            printf("FAIL: %s - Expected (index=%zu,value=%d) at position %d, got (index=%zu,value=%d)\n",
                   test_name, expected_indices[i], expected_values[i], i,
                   actual_indices[i], actual_values[i]);
            return 0;
        }
    }
    return 1;
}

//==============================================================================
// Basic Enumerate Iterator Tests
//==============================================================================

int test_enumerate_basic_functionality(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator [10, 11, 12, 13, 14]
    ANVIterator range_it = anv_iterator_range(10, 15, 1, &alloc);

    // Enumerate starting from index 0
    ANVIterator enum_it = anv_iterator_enumerate(&range_it, &alloc, 0);
    ASSERT_TRUE(enum_it.is_valid(&enum_it));

    size_t indices[10];
    int values[10];
    const int count = collect_indexed_elements(&enum_it, indices, values, 10);

    ASSERT_EQ(count, 5);

    const size_t expected_indices[] = {0, 1, 2, 3, 4};
    const int expected_values[] = {10, 11, 12, 13, 14};
    ASSERT_TRUE(verify_indexed_elements(indices, values, expected_indices, expected_values, 5, "enumerate_basic"));

    enum_it.destroy(&enum_it);
    return TEST_SUCCESS;
}

int test_enumerate_custom_start_index(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator [1, 2, 3]
    ANVIterator range_it = anv_iterator_range(1, 4, 1, &alloc);

    // Enumerate starting from index 100
    ANVIterator enum_it = anv_iterator_enumerate(&range_it, &alloc, 100);
    ASSERT_TRUE(enum_it.is_valid(&enum_it));

    size_t indices[10];
    int values[10];
    const int count = collect_indexed_elements(&enum_it, indices, values, 10);

    ASSERT_EQ(count, 3);

    const size_t expected_indices[] = {100, 101, 102};
    const int expected_values[] = {1, 2, 3};
    ASSERT_TRUE(verify_indexed_elements(indices, values, expected_indices, expected_values, 3, "enumerate_custom_start"));

    enum_it.destroy(&enum_it);
    return TEST_SUCCESS;
}

int test_enumerate_single_element(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator with single element [42]
    ANVIterator range_it = anv_iterator_range(42, 43, 1, &alloc);

    // Enumerate starting from index 5
    ANVIterator enum_it = anv_iterator_enumerate(&range_it, &alloc, 5);
    ASSERT_TRUE(enum_it.is_valid(&enum_it));

    ASSERT_TRUE(enum_it.has_next(&enum_it));
    const ANVIndexedElement* indexed = enum_it.get(&enum_it);
    ASSERT_NOT_NULL(indexed);
    ASSERT_EQ(indexed->index, 5);
    ASSERT_NOT_NULL(indexed->element);
    ASSERT_EQ(*(const int*)indexed->element, 42);

    ASSERT_EQ(enum_it.next(&enum_it), 0);
    ASSERT_FALSE(enum_it.has_next(&enum_it));
    ASSERT_NULL(enum_it.get(&enum_it));

    enum_it.destroy(&enum_it);
    return TEST_SUCCESS;
}

int test_enumerate_large_start_index(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator [1, 2]
    ANVIterator range_it = anv_iterator_range(1, 3, 1, &alloc);

    // Enumerate starting from SIZE_MAX - 1
    ANVIterator enum_it = anv_iterator_enumerate(&range_it, &alloc, SIZE_MAX - 1);
    ASSERT_TRUE(enum_it.is_valid(&enum_it));

    size_t indices[10];
    int values[10];
    const int count = collect_indexed_elements(&enum_it, indices, values, 10);

    ASSERT_EQ(count, 2);

    const size_t expected_indices[] = {SIZE_MAX - 1, SIZE_MAX};
    const int expected_values[] = {1, 2};
    ASSERT_TRUE(verify_indexed_elements(indices, values, expected_indices, expected_values, 2, "enumerate_large_start"));

    enum_it.destroy(&enum_it);
    return TEST_SUCCESS;
}

//==============================================================================
// Edge Cases and Error Handling
//==============================================================================

int test_enumerate_empty_source(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create empty range iterator
    ANVIterator range_it = anv_iterator_range(1, 1, 1, &alloc); // Empty

    // Enumerate empty iterator
    ANVIterator enum_it = anv_iterator_enumerate(&range_it, &alloc, 0);
    ASSERT_TRUE(enum_it.is_valid(&enum_it));

    // Should have no elements
    ASSERT_FALSE(enum_it.has_next(&enum_it));
    ASSERT_NULL(enum_it.get(&enum_it));

    enum_it.destroy(&enum_it);
    return TEST_SUCCESS;
}

int test_enumerate_invalid_parameters(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Test with NULL iterator
    const ANVIterator enum_it1 = anv_iterator_enumerate(NULL, &alloc, 0);
    ASSERT_FALSE(enum_it1.is_valid(&enum_it1));

    // Test with NULL allocator
    ANVIterator range_it = anv_iterator_range(1, 4, 1, &alloc);
    const ANVIterator enum_it2 = anv_iterator_enumerate(&range_it, NULL, 0);
    ASSERT_FALSE(enum_it2.is_valid(&enum_it2));
    range_it.destroy(&range_it); // Clean up since enumerate failed

    return TEST_SUCCESS;
}

//==============================================================================
// Iterator Composition Tests
//==============================================================================

int test_enumerate_with_filter(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range 1-10, filter evens, then enumerate
    ANVIterator range_it = anv_iterator_range(1, 11, 1, &alloc);   // [1,2,3,4,5,6,7,8,9,10]
    ANVIterator filter_it = anv_iterator_filter(&range_it, &alloc, is_even); // [2,4,6,8,10]
    ANVIterator enum_it = anv_iterator_enumerate(&filter_it, &alloc, 0);

    ASSERT_TRUE(enum_it.is_valid(&enum_it));

    size_t indices[10];
    int values[10];
    const int count = collect_indexed_elements(&enum_it, indices, values, 10);

    ASSERT_EQ(count, 5);

    const size_t expected_indices[] = {0, 1, 2, 3, 4};
    const int expected_values[] = {2, 4, 6, 8, 10};
    ASSERT_TRUE(verify_indexed_elements(indices, values, expected_indices, expected_values, 5, "enumerate_with_filter"));

    enum_it.destroy(&enum_it);
    return TEST_SUCCESS;
}

int test_enumerate_with_take(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range 1-10, take first 3, then enumerate starting from 10
    ANVIterator range_it = anv_iterator_range(1, 11, 1, &alloc);   // [1,2,3,4,5,6,7,8,9,10]
    ANVIterator take_it = anv_iterator_take(&range_it, &alloc, 3); // [1,2,3]
    ANVIterator enum_it = anv_iterator_enumerate(&take_it, &alloc, 10);

    ASSERT_TRUE(enum_it.is_valid(&enum_it));

    size_t indices[10];
    int values[10];
    const int count = collect_indexed_elements(&enum_it, indices, values, 10);

    ASSERT_EQ(count, 3);

    const size_t expected_indices[] = {10, 11, 12};
    const int expected_values[] = {1, 2, 3};
    ASSERT_TRUE(verify_indexed_elements(indices, values, expected_indices, expected_values, 3, "enumerate_with_take"));

    enum_it.destroy(&enum_it);
    return TEST_SUCCESS;
}

int test_enumerate_with_skip(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range 1-7, skip first 2, then enumerate starting from 0
    ANVIterator range_it = anv_iterator_range(1, 8, 1, &alloc);   // [1,2,3,4,5,6,7]
    ANVIterator skip_it = anv_iterator_skip(&range_it, &alloc, 2); // [3,4,5,6,7]
    ANVIterator enum_it = anv_iterator_enumerate(&skip_it, &alloc, 0);

    ASSERT_TRUE(enum_it.is_valid(&enum_it));

    size_t indices[10];
    int values[10];
    const int count = collect_indexed_elements(&enum_it, indices, values, 10);

    ASSERT_EQ(count, 5);

    const size_t expected_indices[] = {0, 1, 2, 3, 4};
    const int expected_values[] = {3, 4, 5, 6, 7};
    ASSERT_TRUE(verify_indexed_elements(indices, values, expected_indices, expected_values, 5, "enumerate_with_skip"));

    enum_it.destroy(&enum_it);
    return TEST_SUCCESS;
}

int test_enumerate_chained(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range 1-10, filter odds, enumerate, then take first 2
    ANVIterator range_it = anv_iterator_range(1, 11, 1, &alloc);   // [1,2,3,4,5,6,7,8,9,10]
    ANVIterator filter_it = anv_iterator_filter(&range_it, &alloc, is_odd); // [1,3,5,7,9]
    ANVIterator enum_it = anv_iterator_enumerate(&filter_it, &alloc, 50); // [(50,1),(51,3),(52,5),(53,7),(54,9)]
    ANVIterator take_it = anv_iterator_take(&enum_it, &alloc, 2); // [(50,1),(51,3)]

    ASSERT_TRUE(take_it.is_valid(&take_it));

    // Since take operates on ANVIndexedElement objects, we need to handle this differently
    int count = 0;
    while (take_it.has_next(&take_it) && count < 2)
    {
        const ANVIndexedElement* indexed = take_it.get(&take_it);
        ASSERT_NOT_NULL(indexed);

        if (count == 0)
        {
            ASSERT_EQ(indexed->index, 50);
            ASSERT_EQ(*(const int*)indexed->element, 1);
        }
        else if (count == 1)
        {
            ASSERT_EQ(indexed->index, 51);
            ASSERT_EQ(*(const int*)indexed->element, 3);
        }

        count++;
        take_it.next(&take_it);
    }

    ASSERT_EQ(count, 2);
    ASSERT_FALSE(take_it.has_next(&take_it));

    take_it.destroy(&take_it);
    return TEST_SUCCESS;
}

//==============================================================================
// Data Structure Iterator Composition Tests
//==============================================================================

int test_enumerate_arraylist(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create ArrayList with some data
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Add values 100, 200, 300
    for (int i = 1; i <= 3; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i * 100;
        anv_arraylist_push_back(list, val);
    }

    // Create iterator and enumerate starting from index 5
    ANVIterator array_iter = anv_arraylist_iterator(list);
    ANVIterator enum_it = anv_iterator_enumerate(&array_iter, &alloc, 5);

    ASSERT_TRUE(enum_it.is_valid(&enum_it));

    size_t indices[10];
    int values[10];
    const int count = collect_indexed_elements(&enum_it, indices, values, 10);

    ASSERT_EQ(count, 3);

    const size_t expected_indices[] = {5, 6, 7};
    const int expected_values[] = {100, 200, 300};
    ASSERT_TRUE(verify_indexed_elements(indices, values, expected_indices, expected_values, 3, "enumerate_arraylist"));

    enum_it.destroy(&enum_it);
    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

int test_enumerate_dll(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create DoublyLinkedList with some data
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);

    // Add values 5, 10, 15, 20
    for (int i = 1; i <= 4; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i * 5;
        anv_dll_push_back(list, val);
    }

    // Create iterator and enumerate starting from index 0
    ANVIterator dll_iter = anv_dll_iterator(list);
    ANVIterator enum_it = anv_iterator_enumerate(&dll_iter, &alloc, 0);

    ASSERT_TRUE(enum_it.is_valid(&enum_it));

    size_t indices[10];
    int values[10];
    const int count = collect_indexed_elements(&enum_it, indices, values, 10);

    ASSERT_EQ(count, 4);

    const size_t expected_indices[] = {0, 1, 2, 3};
    const int expected_values[] = {5, 10, 15, 20};
    ASSERT_TRUE(verify_indexed_elements(indices, values, expected_indices, expected_values, 4, "enumerate_dll"));

    enum_it.destroy(&enum_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

int test_enumerate_queue_with_stack(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create Queue with FIFO behavior
    ANVQueue* queue = anv_queue_create(&alloc);

    // Add values 1, 2, 3 (will iterate as 1, 2, 3)
    for (int i = 1; i <= 3; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_queue_enqueue(queue, val);
    }

    // Enumerate queue starting from index 10
    ANVIterator queue_iter = anv_queue_iterator(queue);
    ANVIterator enum_it = anv_iterator_enumerate(&queue_iter, &alloc, 10);

    ASSERT_TRUE(enum_it.is_valid(&enum_it));

    size_t indices[10];
    int values[10];
    const int count = collect_indexed_elements(&enum_it, indices, values, 10);

    ASSERT_EQ(count, 3);

    const size_t expected_indices[] = {10, 11, 12};
    const int expected_values[] = {1, 2, 3}; // FIFO order
    ASSERT_TRUE(verify_indexed_elements(indices, values, expected_indices, expected_values, 3, "enumerate_queue"));

    enum_it.destroy(&enum_it);
    anv_queue_destroy(queue, true);
    return TEST_SUCCESS;
}

int test_enumerate_complex_composition(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Complex pipeline: Range -> Filter -> Take -> Enumerate
    ANVIterator range_it = anv_iterator_range(1, 20, 1, &alloc); // [1..19]
    ANVIterator filter_it = anv_iterator_filter(&range_it, &alloc, is_divisible_by_3); // [3,6,9,12,15,18]
    ANVIterator take_it = anv_iterator_take(&filter_it, &alloc, 4); // [3,6,9,12]
    ANVIterator enum_it = anv_iterator_enumerate(&take_it, &alloc, 100); // [(100,3),(101,6),(102,9),(103,12)]

    ASSERT_TRUE(enum_it.is_valid(&enum_it));

    size_t indices[10];
    int values[10];
    const int count = collect_indexed_elements(&enum_it, indices, values, 10);

    ASSERT_EQ(count, 4);

    const size_t expected_indices[] = {100, 101, 102, 103};
    const int expected_values[] = {3, 6, 9, 12};
    ASSERT_TRUE(verify_indexed_elements(indices, values, expected_indices, expected_values, 4, "enumerate_complex"));

    enum_it.destroy(&enum_it);
    return TEST_SUCCESS;
}

//==============================================================================
// Behavior and State Tests
//==============================================================================

int test_enumerate_iteration_state(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create enumerate iterator and test step-by-step iteration
    ANVIterator range_it = anv_iterator_range(50, 53, 1, &alloc); // [50,51,52]
    ANVIterator enum_it = anv_iterator_enumerate(&range_it, &alloc, 20);

    // Test step-by-step iteration
    ASSERT_TRUE(enum_it.has_next(&enum_it));
    const ANVIndexedElement* indexed1 = enum_it.get(&enum_it);
    ASSERT_NOT_NULL(indexed1);
    ASSERT_EQ(indexed1->index, 20);
    ASSERT_EQ(*(const int*)indexed1->element, 50);

    ASSERT_EQ(enum_it.next(&enum_it), 0);
    ASSERT_TRUE(enum_it.has_next(&enum_it));

    const ANVIndexedElement* indexed2 = enum_it.get(&enum_it);
    ASSERT_NOT_NULL(indexed2);
    ASSERT_EQ(indexed2->index, 21);
    ASSERT_EQ(*(const int*)indexed2->element, 51);

    ASSERT_EQ(enum_it.next(&enum_it), 0);
    ASSERT_TRUE(enum_it.has_next(&enum_it));

    const ANVIndexedElement* indexed3 = enum_it.get(&enum_it);
    ASSERT_NOT_NULL(indexed3);
    ASSERT_EQ(indexed3->index, 22);
    ASSERT_EQ(*(const int*)indexed3->element, 52);

    ASSERT_EQ(enum_it.next(&enum_it), 0);
    ASSERT_FALSE(enum_it.has_next(&enum_it));
    ASSERT_NULL(enum_it.get(&enum_it));
    ASSERT_EQ(enum_it.next(&enum_it), -1);

    enum_it.destroy(&enum_it);
    return TEST_SUCCESS;
}

int test_enumerate_unsupported_operations(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create enumerate iterator
    ANVIterator range_it = anv_iterator_range(1, 4, 1, &alloc);
    ANVIterator enum_it = anv_iterator_enumerate(&range_it, &alloc, 0);

    // Test unsupported operations
    ASSERT_FALSE(enum_it.has_prev(&enum_it));
    ASSERT_EQ(enum_it.prev(&enum_it), -1);

    // Reset should be no-op (doesn't crash)
    enum_it.reset(&enum_it);

    enum_it.destroy(&enum_it);
    return TEST_SUCCESS;
}

int test_enumerate_element_consistency(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Test that the same ANVIndexedElement pointer is returned for multiple get() calls
    ANVIterator range_it = anv_iterator_range(1, 3, 1, &alloc); // [1,2]
    ANVIterator enum_it = anv_iterator_enumerate(&range_it, &alloc, 0);

    ASSERT_TRUE(enum_it.has_next(&enum_it));

    const ANVIndexedElement* indexed1 = enum_it.get(&enum_it);
    const ANVIndexedElement* indexed2 = enum_it.get(&enum_it);

    // Should return same pointer (cached indexed element)
    ASSERT_EQ(indexed1, indexed2);
    ASSERT_EQ(indexed1->index, 0);
    ASSERT_EQ(*(const int*)indexed1->element, 1);

    // After next(), should get different values but could be same pointer
    enum_it.next(&enum_it);
    const ANVIndexedElement* indexed3 = enum_it.get(&enum_it);
    ASSERT_EQ(indexed3->index, 1);
    ASSERT_EQ(*(const int*)indexed3->element, 2);

    enum_it.destroy(&enum_it);
    return TEST_SUCCESS;
}

int test_enumerate_index_overflow_behavior(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Test behavior near SIZE_MAX (this tests the edge case of index overflow)
    ANVIterator range_it = anv_iterator_range(1, 3, 1, &alloc); // [1,2]
    ANVIterator enum_it = anv_iterator_enumerate(&range_it, &alloc, SIZE_MAX);

    ASSERT_TRUE(enum_it.is_valid(&enum_it));

    // First element should have index SIZE_MAX
    ASSERT_TRUE(enum_it.has_next(&enum_it));
    const ANVIndexedElement* indexed1 = enum_it.get(&enum_it);
    ASSERT_NOT_NULL(indexed1);
    ASSERT_EQ(indexed1->index, SIZE_MAX);
    ASSERT_EQ(*(const int*)indexed1->element, 1);

    // Second element should wrap around to 0 (or whatever SIZE_MAX + 1 becomes)
    enum_it.next(&enum_it);
    ASSERT_TRUE(enum_it.has_next(&enum_it));
    const ANVIndexedElement* indexed2 = enum_it.get(&enum_it);
    ASSERT_NOT_NULL(indexed2);
    // Index should be SIZE_MAX + 1, which wraps to 0 for size_t
    ASSERT_EQ(indexed2->index, 0);
    ASSERT_EQ(*(const int*)indexed2->element, 2);

    enum_it.destroy(&enum_it);
    return TEST_SUCCESS;
}

//==============================================================================
// Main Test Runner
//==============================================================================

int main(void)
{
    printf("Running enumerate iterator tests...\n");

    int (*tests[])(void) = {
        test_enumerate_basic_functionality,
        test_enumerate_custom_start_index,
        test_enumerate_single_element,
        test_enumerate_large_start_index,
        test_enumerate_empty_source,
        test_enumerate_invalid_parameters,
        test_enumerate_with_filter,
        test_enumerate_with_take,
        test_enumerate_with_skip,
        test_enumerate_chained,
        test_enumerate_arraylist,
        test_enumerate_dll,
        test_enumerate_queue_with_stack,
        test_enumerate_complex_composition,
        test_enumerate_iteration_state,
        test_enumerate_unsupported_operations,
        test_enumerate_element_consistency,
        test_enumerate_index_overflow_behavior,
    };

    const char* test_names[] = {
        "test_enumerate_basic_functionality",
        "test_enumerate_custom_start_index",
        "test_enumerate_single_element",
        "test_enumerate_large_start_index",
        "test_enumerate_empty_source",
        "test_enumerate_invalid_parameters",
        "test_enumerate_with_filter",
        "test_enumerate_with_take",
        "test_enumerate_with_skip",
        "test_enumerate_chained",
        "test_enumerate_arraylist",
        "test_enumerate_dll",
        "test_enumerate_queue_with_stack",
        "test_enumerate_complex_composition",
        "test_enumerate_iteration_state",
        "test_enumerate_unsupported_operations",
        "test_enumerate_element_consistency",
        "test_enumerate_index_overflow_behavior",
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

    printf("Enumerate iterator tests: %d/%d passed\n", passed, num_tests);
    return (passed == num_tests) ? 0 : 1;
}
