//
// Created by zack on 8/26/25.
//

#include <stdlib.h>

#include "containers/doublylinkedlist.h"
#include "containers/iterator.h"
#include "TestAssert.h"
#include "TestHelpers.h"

// Test basic iterator functionality
static int test_basic_iteration(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    ASSERT_NOT_NULL(list);

    // Insert elements
    for (int i = 1; i <= 5; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_dll_push_back(list, val);
    }

    // Create iterator
    ANVIterator it = anv_dll_iterator(list);
    ASSERT_NOT_NULL(it.data_state);
    ASSERT_TRUE(it.has_next(&it));

    // Iterate through list and verify values
    int expected = 1;
    while (it.has_next(&it))
    {
        const int* value = it.get(&it);
        ASSERT_NOT_NULL(value);
        ASSERT_EQ(*value, expected++);
        it.next(&it);
    }

    // Verify we processed all elements
    ASSERT_EQ(expected, 6);

    // Verify the iterator is exhausted
    ASSERT_FALSE(it.has_next(&it));
    ASSERT_NULL(it.get(&it));
    ASSERT_EQ(it.next(&it), -1); // Should return error code

    // Cleanup
    if (it.destroy)
        it.destroy(&it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

// Test iterator with empty list
static int test_empty_list_iterator(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    ASSERT_NOT_NULL(list);

    ANVIterator it = anv_dll_iterator(list);

    // Verify iterator for empty list
    ASSERT_FALSE(it.has_next(&it));
    ASSERT_NULL(it.get(&it));
    ASSERT_EQ(it.next(&it), -1); // Should return error code

    // Cleanup
    if (it.destroy)
        it.destroy(&it);
    anv_dll_destroy(list, false);
    return TEST_SUCCESS;
}

// Test iterator with modifications
static int test_iterator_with_modifications(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    ASSERT_NOT_NULL(list);

    // Insert initial elements
    for (int i = 1; i <= 3; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_dll_push_back(list, val);
    }

    // Create iterator
    ANVIterator it = anv_dll_iterator(list);

    // Consume first element
    const int* value = it.get(&it);
    ASSERT_NOT_NULL(value);
    ASSERT_EQ(*value, 1);
    it.next(&it);

    // Modify list by adding new elements
    int* new_val = malloc(sizeof(int));
    *new_val = 99;
    anv_dll_push_back(list, new_val);

    // Continue iteration
    value = it.get(&it);
    ASSERT_NOT_NULL(value);
    ASSERT_EQ(*value, 2);
    it.next(&it);

    value = it.get(&it);
    ASSERT_NOT_NULL(value);
    ASSERT_EQ(*value, 3);
    it.next(&it);

    // The newly added element should also be accessible
    value = it.get(&it);
    ASSERT_NOT_NULL(value);
    ASSERT_EQ(*value, 99);
    it.next(&it);

    ASSERT_FALSE(it.has_next(&it));

    // Cleanup
    if (it.destroy)
        it.destroy(&it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

// Test multiple iterators
static int test_multiple_iterators(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    ASSERT_NOT_NULL(list);

    // Insert elements
    for (int i = 1; i <= 5; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_dll_push_back(list, val);
    }

    // Create two independent iterators
    ANVIterator it1 = anv_dll_iterator(list);
    ANVIterator it2 = anv_dll_iterator(list);

    // First iterator consumes two elements
    const int* value1 = it1.get(&it1);
    ASSERT_EQ(*value1, 1);
    it1.next(&it1);

    value1 = it1.get(&it1);
    ASSERT_EQ(*value1, 2);
    it1.next(&it1);

    // Second iterator should still be at the beginning
    const int* value2 = it2.get(&it2);
    ASSERT_EQ(*value2, 1);
    it2.next(&it2);

    // Continue with first iterator
    value1 = it1.get(&it1);
    ASSERT_EQ(*value1, 3);
    it1.next(&it1);

    // Continue with second iterator
    value2 = it2.get(&it2);
    ASSERT_EQ(*value2, 2);
    it2.next(&it2);

    // Cleanup
    if (it1.destroy)
        it1.destroy(&it1);
    if (it2.destroy)
        it2.destroy(&it2);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

// Test reverse iterator
static int test_reverse_iteration(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    ASSERT_NOT_NULL(list);

    // Insert elements 1-5
    for (int i = 1; i <= 5; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_dll_push_back(list, val);
    }

    // Create reverse iterator
    ANVIterator it = anv_dll_iterator_reverse(list);
    ASSERT_NOT_NULL(it.data_state);
    ASSERT_TRUE(it.has_next(&it));

    // Iterate in reverse order (should get 5, 4, 3, 2, 1)
    int expected = 5;
    while (it.has_next(&it))
    {
        const int* value = it.get(&it);
        ASSERT_NOT_NULL(value);
        ASSERT_EQ(*value, expected--);
        it.next(&it);
    }

    // Verify we processed all elements
    ASSERT_EQ(expected, 0);

    // Cleanup
    if (it.destroy)
        it.destroy(&it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

// Test iterator get function
static int test_iterator_get(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    ASSERT_NOT_NULL(list);

    // Insert elements
    for (int i = 1; i <= 3; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_dll_push_back(list, val);
    }

    ANVIterator it = anv_dll_iterator(list);

    // Test get without advancing
    const int* val = it.get(&it);
    ASSERT_NOT_NULL(val);
    ASSERT_EQ(*val, 1);

    // Get again - should return same value
    val = it.get(&it);
    ASSERT_EQ(*val, 1);

    // Now advance and test get
    it.next(&it);
    val = it.get(&it);
    ASSERT_EQ(*val, 2);

    // Cleanup
    if (it.destroy)
        it.destroy(&it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

// Test bidirectional iteration
static int test_bidirectional_iteration(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    ASSERT_NOT_NULL(list);

    // Insert elements 1-5
    for (int i = 1; i <= 5; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_dll_push_back(list, val);
    }

    ANVIterator it = anv_dll_iterator(list);

    // Move forward to middle
    it.next(&it); // Move to 2
    it.next(&it); // Move to 3

    const int* val = it.get(&it);
    ASSERT_EQ(*val, 3);

    // Move back
    ASSERT_TRUE(it.has_prev(&it));
    it.prev(&it);
    val = it.get(&it);
    ASSERT_EQ(*val, 2);

    // Move forward again
    it.next(&it);
    val = it.get(&it);
    ASSERT_EQ(*val, 3);

    // Cleanup
    if (it.destroy)
        it.destroy(&it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

// Test creating doubly linked list from iterator
static int test_from_iterator(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create a range iterator (0, 1, 2, 3, 4)
    ANVIterator range_it = anv_iterator_range(0, 5, 1, &alloc);

    // Create doubly linked list from iterator
    ANVDoublyLinkedList* list = anv_dll_from_iterator(&range_it, &alloc, true);
    ASSERT_NOT_NULL(list);
    ASSERT_EQ(anv_dll_size(list), 5);

    // Clean up the iterator immediately after use
    range_it.destroy(&range_it);

    // Verify doubly linked list has correct values in sequential order
    // Iterator gives 0,1,2,3,4 and doubly linked list should have them as 0,1,2,3,4 (head to tail)
    const ANVDoublyLinkedNode* node = list->head;
    for (int expected = 0; expected < 5; expected++)
    {
        ASSERT_NOT_NULL(node);
        ASSERT_EQ(*(int*)node->data, expected);
        node = node->next;
    }

    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

// Test iterator with invalid doubly linked list
static int test_iterator_invalid(void)
{
    const ANVIterator iter = anv_dll_iterator(NULL);
    ASSERT(!iter.is_valid(&iter));
    return TEST_SUCCESS;
}

// Test copy isolation - verify that copied elements are independent
static int test_dll_copy_isolation(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create original data that we can modify
    const int original_values[] = {10, 20, 30};
    int* data_ptrs[3];

    // Create a source doubly linked list
    ANVDoublyLinkedList* source_list = anv_dll_create(&alloc);
    ASSERT_NOT_NULL(source_list);

    for (int i = 0; i < 3; i++)
    {
        data_ptrs[i] = malloc(sizeof(int));
        *data_ptrs[i] = original_values[i];
        ASSERT_EQ(anv_dll_push_back(source_list, data_ptrs[i]), 0);
    }

    ANVIterator list_it = anv_dll_iterator(source_list);
    ASSERT(list_it.is_valid(&list_it));

    // Create doubly linked list with copying enabled
    ANVDoublyLinkedList* new_list = anv_dll_from_iterator(&list_it, &alloc, true);
    ASSERT_NOT_NULL(new_list);
    ASSERT_EQ(anv_dll_size(new_list), 3);

    // Modify original data
    *data_ptrs[0] = 999;
    *data_ptrs[1] = 888;
    *data_ptrs[2] = 777;

    // DoublyLinkedList should still have original values (proving data was copied)
    // Sequential order: 10, 20, 30
    const ANVDoublyLinkedNode* node = new_list->head;
    ASSERT_NOT_NULL(node);
    ASSERT_EQ(*(int*)node->data, 10); // Should be unchanged

    node = node->next;
    ASSERT_NOT_NULL(node);
    ASSERT_EQ(*(int*)node->data, 20); // Should be unchanged

    node = node->next;
    ASSERT_NOT_NULL(node);
    ASSERT_EQ(*(int*)node->data, 30); // Should be unchanged

    // Cleanup
    list_it.destroy(&list_it);
    anv_dll_destroy(new_list, true);
    anv_dll_destroy(source_list, true);

    return TEST_SUCCESS;
}

// Test that should_copy=true fails when allocator has no copy function
static int test_dll_anv_copy_function_required(void)
{
    ANVAllocator alloc = anv_alloc_default();
    alloc.copy = NULL;

    ANVIterator range_it = anv_iterator_range(0, 3, 1, &alloc);
    ASSERT(range_it.is_valid(&range_it));

    // Should return NULL because should_copy=true but no copy function available
    ANVDoublyLinkedList* list = anv_dll_from_iterator(&range_it, &alloc, true);
    ASSERT_NULL(list);

    range_it.destroy(&range_it);
    return TEST_SUCCESS;
}

// Test that should_copy=false uses elements directly without copying
static int test_dll_from_iterator_no_copy(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create a range iterator and then a copy iterator to get actual owned data
    ANVIterator range_it = anv_iterator_range(0, 3, 1, &alloc);
    ASSERT(range_it.is_valid(&range_it));

    // Use copy iterator to create actual data elements that we own
    ANVIterator copy_it = anv_iterator_copy(&range_it, &alloc, int_copy);
    ASSERT(copy_it.is_valid(&copy_it));

    // Create doubly linked list without copying (should_copy = false)
    // This will use the copied elements directly from the copy iterator
    ANVDoublyLinkedList* list = anv_dll_from_iterator(&copy_it, &alloc, false);
    ASSERT_NOT_NULL(list);
    ASSERT_EQ(anv_dll_size(list), 3);

    // Verify values are correct (sequential order: 0, 1, 2)
    const ANVDoublyLinkedNode* node = list->head;
    ASSERT_NOT_NULL(node);
    ASSERT_EQ(*(int*)node->data, 0);

    node = node->next;
    ASSERT_NOT_NULL(node);
    ASSERT_EQ(*(int*)node->data, 1);

    node = node->next;
    ASSERT_NOT_NULL(node);
    ASSERT_EQ(*(int*)node->data, 2);

    range_it.destroy(&range_it);
    copy_it.destroy(&copy_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

// Test that iterator is exhausted after being consumed by anv_dll_from_iterator
static int test_iterator_exhaustion_after_dll_creation(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVIterator range_it = anv_iterator_range(0, 5, 1, &alloc);
    ASSERT(range_it.is_valid(&range_it));

    // Verify iterator starts with elements
    ASSERT(range_it.has_next(&range_it));

    // Create doubly linked list from iterator (consumes all elements)
    ANVDoublyLinkedList* list = anv_dll_from_iterator(&range_it, &alloc, true);
    ASSERT_NOT_NULL(list);
    ASSERT_EQ(anv_dll_size(list), 5);

    // Iterator should now be exhausted
    ASSERT(!range_it.has_next(&range_it));
    ASSERT_NULL(range_it.get(&range_it));
    ASSERT_EQ(range_it.next(&range_it), -1); // Should fail to advance

    // But iterator should still be valid
    ASSERT(range_it.is_valid(&range_it));

    range_it.destroy(&range_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

// Test next() return values for proper error handling
static int test_dll_iterator_next_return_values(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    ASSERT_NOT_NULL(list);

    // Add single element
    int* data = malloc(sizeof(int));
    *data = 42;
    ASSERT_EQ(anv_dll_push_back(list, data), 0);

    ANVIterator iter = anv_dll_iterator(list);
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
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

// Test various combinations of get/next/has_next calls for consistency
static int test_dll_iterator_mixed_operations(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    ASSERT_NOT_NULL(list);

    // Add test data (will be in sequential order: 0, 10, 20)
    for (int i = 0; i < 3; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i * 10;
        ASSERT_EQ(anv_dll_push_back(list, data), 0);
    }

    ANVIterator iter = anv_dll_iterator(list);
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
    // Note: data1 and data3 point to different doubly linked list elements
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
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

// Test iterator traversal order
static int test_dll_iterator_order(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);

    // Add elements in specific order
    const int values[] = {100, 200, 300, 400, 500};
    for (int i = 0; i < 5; i++)
    {
        int* data = malloc(sizeof(int));
        *data = values[i];
        ASSERT_EQ(anv_dll_push_back(list, data), 0);
    }

    // Create iterator and verify order
    ANVIterator iter = anv_dll_iterator(list);

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
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

// Test reset functionality
static int test_dll_iterator_reset(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);

    // Add numbers 1-3
    for (int i = 1; i <= 3; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_dll_push_back(list, val);
    }

    ANVIterator iter = anv_dll_iterator(list);

    // Advance iterator
    iter.next(&iter);
    iter.next(&iter);

    // Reset and verify back at beginning
    iter.reset(&iter);
    const int* val = iter.get(&iter);
    ASSERT_EQ(*val, 1);
    ASSERT(iter.has_next(&iter));

    iter.destroy(&iter);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

// Test single element iterator behavior
static int test_dll_iterator_single_element(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);

    int* val = malloc(sizeof(int));
    *val = 42;
    anv_dll_push_back(list, val);

    ANVIterator iter = anv_dll_iterator(list);

    ASSERT(iter.has_next(&iter));
    ASSERT(!iter.has_prev(&iter)); // At beginning, no previous

    const int* retrieved = iter.get(&iter);
    ASSERT_EQ(*retrieved, 42);

    iter.next(&iter);
    ASSERT(!iter.has_next(&iter));
    ASSERT(iter.has_prev(&iter)); // At end, has previous

    iter.destroy(&iter);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

typedef struct
{
    int (*func)(void);
    const char* name;
} TestCase;

int main(void)
{
    const TestCase tests[] = {
        {test_basic_iteration, "test_basic_iteration"},
        {test_empty_list_iterator, "test_empty_list_iterator"},
        {test_iterator_with_modifications, "test_iterator_with_modifications"},
        {test_multiple_iterators, "test_multiple_iterators"},
        {test_reverse_iteration, "test_reverse_iteration"},
        {test_iterator_get, "test_iterator_get"},
        {test_bidirectional_iteration, "test_bidirectional_iteration"},
        {test_from_iterator, "test_from_iterator"},
        {test_iterator_invalid, "test_iterator_invalid"},
        {test_dll_copy_isolation, "test_dll_copy_isolation"},
        {test_dll_anv_copy_function_required, "test_dll_anv_copy_function_required"},
        {test_dll_from_iterator_no_copy, "test_dll_from_iterator_no_copy"},
        {test_iterator_exhaustion_after_dll_creation, "test_iterator_exhaustion_after_dll_creation"},
        {test_dll_iterator_next_return_values, "test_dll_iterator_next_return_values"},
        {test_dll_iterator_mixed_operations, "test_dll_iterator_mixed_operations"},
        {test_dll_iterator_order, "test_dll_iterator_order"},
        {test_dll_iterator_reset, "test_dll_iterator_reset"},
        {test_dll_iterator_single_element, "test_dll_iterator_single_element"},
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

    printf("All doubly linked list iterator tests passed!\n");
    return 0;
}
