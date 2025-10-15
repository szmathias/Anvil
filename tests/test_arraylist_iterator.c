//
// Created by zack on 9/2/25.
//

#include "containers/arraylist.h"
#include "TestAssert.h"
#include "TestHelpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_forward_iterator(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Add numbers 1-5
    for (int i = 1; i <= 5; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_arraylist_push_back(list, val);
    }

    ANVIterator iter = anv_arraylist_iterator(list);
    ASSERT(iter.is_valid(&iter));

    // Test forward iteration
    int expected = 1;
    while (iter.has_next(&iter))
    {
        const int* val = (int*)iter.get(&iter);
        ASSERT_NOT_NULL(val);
        ASSERT_EQ(*val, expected);
        expected++;
        iter.next(&iter);
    }

    ASSERT_EQ(expected, 6); // Should have iterated through all 5 elements
    ASSERT(!iter.has_next(&iter));

    iter.destroy(&iter);
    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

int test_reverse_iterator(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Add numbers 1-5
    for (int i = 1; i <= 5; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_arraylist_push_back(list, val);
    }

    ANVIterator iter = anv_arraylist_iterator_reverse(list);
    ASSERT(iter.is_valid(&iter));

    // Test reverse iteration (should get 5, 4, 3, 2, 1)
    int expected = 5;
    while (iter.has_next(&iter))
    {
        const int* val = (int*)iter.get(&iter);
        ASSERT_NOT_NULL(val);
        ASSERT_EQ(*val, expected);
        expected--;
        iter.next(&iter);
    }

    ASSERT_EQ(expected, 0); // Should have iterated through all 5 elements
    ASSERT(!iter.has_next(&iter));

    iter.destroy(&iter);
    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

int test_iterator_get(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Add numbers 1-3
    for (int i = 1; i <= 3; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_arraylist_push_back(list, val);
    }

    ANVIterator iter = anv_arraylist_iterator(list);

    // Test get without advancing
    int* val = (int*)iter.get(&iter);
    ASSERT_NOT_NULL(val);
    ASSERT_EQ(*val, 1);

    // Get again - should return same value
    val = (int*)iter.get(&iter);
    ASSERT_EQ(*val, 1);

    // Now advance and test get
    iter.next(&iter);
    val = (int*)iter.get(&iter);
    ASSERT_EQ(*val, 2);

    iter.destroy(&iter);
    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

int test_iterator_prev(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Add numbers 1-3
    for (int i = 1; i <= 3; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_arraylist_push_back(list, val);
    }

    ANVIterator iter = anv_arraylist_iterator(list);
    iter.next(&iter); // Move to 2
    iter.next(&iter); // Move to 3

    // Test has_prev and prev
    ASSERT(iter.has_prev(&iter));
    iter.prev(&iter);
    const int* val = (int*)iter.get(&iter);
    ASSERT_EQ(*val, 2);

    ASSERT(iter.has_prev(&iter));
    iter.prev(&iter);
    val = (int*)iter.get(&iter);
    ASSERT_EQ(*val, 1);

    ASSERT(!iter.has_prev(&iter));

    iter.destroy(&iter);
    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

int test_iterator_reset(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Add numbers 1-3
    for (int i = 1; i <= 3; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_arraylist_push_back(list, val);
    }

    ANVIterator iter = anv_arraylist_iterator(list);

    // Advance iterator
    iter.next(&iter);
    iter.next(&iter);

    // Reset and verify back at beginning
    iter.reset(&iter);
    int* val = (int*)iter.get(&iter);
    ASSERT_EQ(*val, 1);
    ASSERT(iter.has_next(&iter));

    iter.destroy(&iter);
    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

int test_iterator_empty_list(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    ANVIterator iter = anv_arraylist_iterator(list);
    ASSERT(iter.is_valid(&iter));
    ASSERT(!iter.has_next(&iter));
    ASSERT(!iter.has_prev(&iter));
    ASSERT_EQ(iter.next(&iter), -1); // Should return error code
    ASSERT_NULL(iter.get(&iter));

    iter.destroy(&iter);
    anv_arraylist_destroy(list, false);
    return TEST_SUCCESS;
}

int test_iterator_single_element(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    int* val = malloc(sizeof(int));
    *val = 42;
    anv_arraylist_push_back(list, val);

    ANVIterator iter = anv_arraylist_iterator(list);

    ASSERT(iter.has_next(&iter));
    ASSERT(!iter.has_prev(&iter));

    const int* retrieved = iter.get(&iter);
    ASSERT_EQ(*retrieved, 42);

    iter.next(&iter);
    ASSERT(!iter.has_next(&iter));
    ASSERT(iter.has_prev(&iter));

    iter.destroy(&iter);
    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

// Test creating arraylist from iterator
int test_from_iterator(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create a range iterator (0, 1, 2, 3, 4)
    ANVIterator range_it = anv_iterator_range(0, 5, 1, &alloc);

    // Create arraylist from iterator
    ANVArrayList* list = anv_arraylist_from_iterator(&range_it, &alloc, true);
    ASSERT_NOT_NULL(list);
    ASSERT_EQ(anv_arraylist_size(list), 5);

    // Clean up the iterator immediately after use
    range_it.destroy(&range_it);

    // Verify arraylist has correct values in sequential order
    // Iterator gives 0,1,2,3,4 and arraylist should have them as 0,1,2,3,4 (index order)
    for (int expected = 0; expected < 5; expected++)
    {
        void* data = anv_arraylist_get(list, expected);
        ASSERT_NOT_NULL(data);
        ASSERT_EQ(*(int*)data, expected);
    }

    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

// Test iterator with invalid arraylist
int test_iterator_invalid(void)
{
    const ANVIterator iter = anv_arraylist_iterator(NULL);
    ASSERT(!iter.is_valid(&iter));
    return TEST_SUCCESS;
}

// Test iterator state after arraylist modifications
int test_iterator_modification(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Add initial data
    for (int i = 0; i < 3; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i * 10;
        ASSERT_EQ(anv_arraylist_push_back(list, data), 0);
    }

    ANVIterator iter = anv_arraylist_iterator(list);
    ASSERT(iter.is_valid(&iter));

    // Get first element
    void* first = iter.get(&iter);
    ASSERT_EQ(*(int*)first, 0); // Should be first element (0*10)
    iter.next(&iter);

    // Modify arraylist while iterator exists (implementation detail: iterator may become invalid)
    int* new_data = malloc(sizeof(int));
    *new_data = 999;
    ASSERT_EQ(anv_arraylist_push_back(list, new_data), 0);

    // Iterator should still be valid but may not reflect new state
    ASSERT(iter.is_valid(&iter));

    iter.destroy(&iter);
    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

// Test copy isolation - verify that copied elements are independent
int test_arraylist_copy_isolation(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create original data that we can modify
    int original_values[] = {10, 20, 30};
    int* data_ptrs[3];

    // Create a source arraylist
    ANVArrayList* source_list = anv_arraylist_create(&alloc, 0);
    ASSERT_NOT_NULL(source_list);

    for (int i = 0; i < 3; i++)
    {
        data_ptrs[i] = malloc(sizeof(int));
        *data_ptrs[i] = original_values[i];
        ASSERT_EQ(anv_arraylist_push_back(source_list, data_ptrs[i]), 0);
    }

    ANVIterator list_it = anv_arraylist_iterator(source_list);
    ASSERT(list_it.is_valid(&list_it));

    // Create arraylist with copying enabled
    ANVArrayList* new_list = anv_arraylist_from_iterator(&list_it, &alloc, true);
    ASSERT_NOT_NULL(new_list);
    ASSERT_EQ(anv_arraylist_size(new_list), 3);

    // Modify original data
    *data_ptrs[0] = 999;
    *data_ptrs[1] = 888;
    *data_ptrs[2] = 777;

    // ArrayList should still have original values (proving data was copied)
    // Sequential order: 10, 20, 30
    void* arraylist_data = anv_arraylist_get(new_list, 0);
    ASSERT_NOT_NULL(arraylist_data);
    ASSERT_EQ(*(int*)arraylist_data, 10); // Should be unchanged

    arraylist_data = anv_arraylist_get(new_list, 1);
    ASSERT_NOT_NULL(arraylist_data);
    ASSERT_EQ(*(int*)arraylist_data, 20); // Should be unchanged

    arraylist_data = anv_arraylist_get(new_list, 2);
    ASSERT_NOT_NULL(arraylist_data);
    ASSERT_EQ(*(int*)arraylist_data, 30); // Should be unchanged

    // Cleanup
    list_it.destroy(&list_it);
    anv_arraylist_destroy(new_list, true);
    anv_arraylist_destroy(source_list, true);

    return TEST_SUCCESS;
}

// Test that should_copy=true fails when allocator has no copy function
int test_arraylist_anv_copy_function_required(void)
{
    ANVAllocator alloc = anv_alloc_default();
    alloc.copy = NULL;

    ANVIterator range_it = anv_iterator_range(0, 3, 1, &alloc);
    ASSERT(range_it.is_valid(&range_it));

    // Should return NULL because should_copy=true but no copy function available
    ANVArrayList* list = anv_arraylist_from_iterator(&range_it, &alloc, true);
    ASSERT_NULL(list);

    range_it.destroy(&range_it);
    return TEST_SUCCESS;
}

// Test that should_copy=false uses elements directly without copying
int test_arraylist_from_iterator_no_copy(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create a range iterator and then a copy iterator to get actual owned data
    ANVIterator range_it = anv_iterator_range(0, 3, 1, &alloc);
    ASSERT(range_it.is_valid(&range_it));

    // Use copy iterator to create actual data elements that we own
    ANVIterator copy_it = anv_iterator_copy(&range_it, &alloc, int_copy);
    ASSERT(copy_it.is_valid(&copy_it));

    // Create arraylist without copying (should_copy = false)
    // This will use the copied elements directly from the copy iterator
    ANVArrayList* list = anv_arraylist_from_iterator(&copy_it, &alloc, false);
    ASSERT_NOT_NULL(list);
    ASSERT_EQ(anv_arraylist_size(list), 3);

    // Verify values are correct (sequential order: 0, 1, 2)
    void* data = anv_arraylist_get(list, 0);
    ASSERT_NOT_NULL(data);
    ASSERT_EQ(*(int*)data, 0);

    data = anv_arraylist_get(list, 1);
    ASSERT_NOT_NULL(data);
    ASSERT_EQ(*(int*)data, 1);

    data = anv_arraylist_get(list, 2);
    ASSERT_NOT_NULL(data);
    ASSERT_EQ(*(int*)data, 2);

    range_it.destroy(&range_it);
    copy_it.destroy(&copy_it);
    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

// Test that iterator is exhausted after being consumed by anv_arraylist_from_iterator
int test_iterator_exhaustion_after_arraylist_creation(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVIterator range_it = anv_iterator_range(0, 5, 1, &alloc);
    ASSERT(range_it.is_valid(&range_it));

    // Verify iterator starts with elements
    ASSERT(range_it.has_next(&range_it));

    // Create arraylist from iterator (consumes all elements)
    ANVArrayList* list = anv_arraylist_from_iterator(&range_it, &alloc, true);
    ASSERT_NOT_NULL(list);
    ASSERT_EQ(anv_arraylist_size(list), 5);

    // Iterator should now be exhausted
    ASSERT(!range_it.has_next(&range_it));
    ASSERT_NULL(range_it.get(&range_it));
    ASSERT_EQ(range_it.next(&range_it), -1); // Should fail to advance

    // But iterator should still be valid
    ASSERT(range_it.is_valid(&range_it));

    range_it.destroy(&range_it);
    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

// Test next() return values for proper error handling
int test_arraylist_iterator_next_return_values(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);
    ASSERT_NOT_NULL(list);

    // Add single element
    int* data = malloc(sizeof(int));
    *data = 42;
    ASSERT_EQ(anv_arraylist_push_back(list, data), 0);

    ANVIterator iter = anv_arraylist_iterator(list);
    ASSERT(iter.is_valid(&iter));

    // Should successfully advance once
    ASSERT(iter.has_next(&iter));
    ASSERT_EQ(iter.next(&iter), 0); // Success

    // Should fail to advance when exhausted
    ASSERT(!iter.has_next(&iter));
    ASSERT_EQ(iter.next(&iter), -1); // Failure

    // Additional calls should continue to fail
    ASSERT_EQ(iter.next(&iter), -1); // Still failure
    ASSERT(!iter.has_next(&iter));   // Still no elements

    iter.destroy(&iter);
    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

// Test various combinations of get/next/has_next calls for consistency
int test_arraylist_iterator_mixed_operations(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);
    ASSERT_NOT_NULL(list);

    // Add test data (will be in sequential order: 0, 10, 20)
    for (int i = 0; i < 3; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i * 10;
        ASSERT_EQ(anv_arraylist_push_back(list, data), 0);
    }

    ANVIterator iter = anv_arraylist_iterator(list);
    ASSERT(iter.is_valid(&iter));

    // Multiple get() calls should return same value
    void* data1 = iter.get(&iter);
    void* data2 = iter.get(&iter);
    ASSERT_NOT_NULL(data1);
    ASSERT_NOT_NULL(data2);
    ASSERT_EQ(data1, data2);               // Same pointer
    ASSERT_EQ(*(int*)data1, *(int*)data2); // Same value
    ASSERT_EQ(*(int*)data1, 0);            // First element should be 0

    // has_next should be consistent
    ASSERT(iter.has_next(&iter));
    ASSERT(iter.has_next(&iter)); // Multiple calls should be safe

    // Advance and verify new position
    ASSERT_EQ(iter.next(&iter), 0);
    void* data3 = iter.get(&iter);
    ASSERT_NOT_NULL(data3);
    // Note: data1 and data3 point to different arraylist elements
    ASSERT_NOT_EQ(*(int*)data1, *(int*)data3); // Different values
    ASSERT_EQ(*(int*)data3, 10);               // Next element should be 10

    // Verify we can still advance
    ASSERT(iter.has_next(&iter));
    ASSERT_EQ(iter.next(&iter), 0);

    void* data4 = iter.get(&iter);
    ASSERT_NOT_NULL(data4);
    ASSERT_EQ(*(int*)data4, 20); // Last element should be 20

    // Now should be at end
    ASSERT_EQ(iter.next(&iter), 0); // Advance past last element
    ASSERT(!iter.has_next(&iter));
    ASSERT_NULL(iter.get(&iter));

    iter.destroy(&iter);
    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

int test_bidirectional_iteration(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Add numbers 1-5
    for (int i = 1; i <= 5; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_arraylist_push_back(list, val);
    }

    ANVIterator iter = anv_arraylist_iterator(list);

    // Move forward to middle
    iter.next(&iter); // Move to 2
    iter.next(&iter); // Move to 3

    const int* val = iter.get(&iter);
    ASSERT_EQ(*val, 3);

    // Move back
    iter.prev(&iter);
    val = iter.get(&iter);
    ASSERT_EQ(*val, 2);

    // Move forward again
    iter.next(&iter);
    val = iter.get(&iter);
    ASSERT_EQ(*val, 3);

    iter.destroy(&iter);
    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

// Test iterator traversal order
int test_arraylist_iterator_order(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Add elements in specific order
    const int values[] = {100, 200, 300, 400, 500};
    for (int i = 0; i < 5; i++)
    {
        int* data = malloc(sizeof(int));
        *data = values[i];
        ASSERT_EQ(anv_arraylist_push_back(list, data), 0);
    }

    // Create iterator and verify order
    ANVIterator iter = anv_arraylist_iterator(list);

    for (int i = 0; i < 5; i++)
    {
        ASSERT(iter.has_next(&iter));
        void* data = iter.get(&iter);
        ASSERT_NOT_NULL(data);
        ASSERT_EQ(*(int*)data, values[i]);
        iter.next(&iter);
    }

    ASSERT(!iter.has_next(&iter));

    iter.destroy(&iter);
    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

typedef struct
{
    int (*func)(void);
    const char* name;
} TestCase;

int main(void)
{
    TestCase tests[] = {
        {test_forward_iterator, "test_forward_iterator"},
        {test_reverse_iterator, "test_reverse_iterator"},
        {test_iterator_get, "test_iterator_get"},
        {test_iterator_prev, "test_iterator_prev"},
        {test_iterator_reset, "test_iterator_reset"},
        {test_iterator_empty_list, "test_iterator_empty_list"},
        {test_iterator_single_element, "test_iterator_single_element"},
        {test_from_iterator, "test_from_iterator"},
        {test_iterator_invalid, "test_iterator_invalid"},
        {test_iterator_modification, "test_iterator_modification"},
        {test_arraylist_copy_isolation, "test_arraylist_copy_isolation"},
        {test_arraylist_anv_copy_function_required, "test_arraylist_anv_copy_function_required"},
        {test_arraylist_from_iterator_no_copy, "test_arraylist_from_iterator_no_copy"},
        {test_iterator_exhaustion_after_arraylist_creation, "test_iterator_exhaustion_after_arraylist_creation"},
        {test_arraylist_iterator_next_return_values, "test_arraylist_iterator_next_return_values"},
        {test_arraylist_iterator_mixed_operations, "test_arraylist_iterator_mixed_operations"},
        {test_bidirectional_iteration, "test_bidirectional_iteration"},
        {test_arraylist_iterator_order, "test_arraylist_iterator_order"},
    };

    int failed = 0;
    const int num_tests = sizeof(tests) / sizeof(tests[0]);

    for (int i = 0; i < num_tests; i++)
    {
        if (tests[i].func() != TEST_SUCCESS)
        {
            printf("%s failed\n", tests[i].name);
            failed++;
        }
    }

    if (failed)
    {
        printf("%d test(s) failed.\n", failed);
        return 1;
    }
    else
    {
        printf("All arraylist iterator tests passed!\n");
        return 0;
    }
}