//
// Created by zack on 9/9/25.
//

#include "TestAssert.h"
#include "TestHelpers.h"
#include "containers/queue.h"
#include "containers/arraylist.h"
#include <stdio.h>
#include <stdlib.h>

// Test queue with iterator
int test_queue_iterator(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVQueue* queue = anv_queue_create(&alloc);

    // Add some test data
    for (int i = 0; i < 5; i++)
    {
        const int values[] = {10, 20, 30, 40, 50};
        int* data = malloc(sizeof(int));
        *data = values[i];
        ASSERT_EQ(anv_queue_enqueue(queue, data), 0);
    }

    // Create iterator
    ANVIterator it = anv_queue_iterator(queue);
    ASSERT(it.is_valid(&it));

    // Iterate through queue (should be in FIFO order: 10, 20, 30, 40, 50)
    int index = 0;
    const int expected[] = {10, 20, 30, 40, 50};

    while (it.has_next(&it))
    {
        void* data = it.get(&it);
        ASSERT_NOT_NULL(data);
        ASSERT_EQ(*(int*)data, expected[index]);
        index++;
        it.next(&it);
    }
    ASSERT_EQ(index, 5);

    // Test reset functionality
    it.reset(&it);
    ASSERT(it.has_next(&it));
    void* first = it.get(&it);
    ASSERT_EQ(*(int*)first, 10); // Should be front element again

    // Test get without advancing
    it.reset(&it);
    void* peek_data = it.get(&it);
    ASSERT_EQ(*(int*)peek_data, 10);
    ASSERT(it.has_next(&it)); // Should still have next

    it.destroy(&it);
    anv_queue_destroy(queue, true);
    return TEST_SUCCESS;
}

// Test creating queue from iterator
int test_queue_from_iterator(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create a range iterator (0, 1, 2, 3, 4)
    ANVIterator range_it = anv_iterator_range(0, 5, 1, &alloc);

    // Create queue from iterator
    ANVQueue* queue = anv_queue_from_iterator(&range_it, &alloc, true);
    ASSERT_NOT_NULL(queue);
    ASSERT_EQ(anv_queue_size(queue), 5);

    // Clean up the iterator immediately after use
    range_it.destroy(&range_it);

    // Verify queue has correct values in FIFO order
    // Iterator gives 0,1,2,3,4 and queue should have them as 0,1,2,3,4 (front to back)
    for (int expected = 0; expected < 5; expected++)
    {
        void* data = anv_queue_dequeue_data(queue);
        ASSERT_NOT_NULL(data);
        ASSERT_EQ(*(int*)data, expected);
        free(data);
    }

    anv_queue_destroy(queue, false);
    return TEST_SUCCESS;
}

// Test iterator with empty queue
int test_queue_iterator_empty(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVQueue* queue = anv_queue_create(&alloc);

    ANVIterator it = anv_queue_iterator(queue);
    ASSERT(it.is_valid(&it));
    ASSERT(!it.has_next(&it));
    ASSERT_NULL(it.get(&it));
    ASSERT_EQ(it.next(&it), -1); // Should return error code

    it.destroy(&it);
    anv_queue_destroy(queue, false);
    return TEST_SUCCESS;
}

// Test iterator validity with invalid queue
int test_queue_iterator_invalid(void)
{
    const ANVIterator it = anv_queue_iterator(NULL);
    ASSERT(!it.is_valid(&it));
    return TEST_SUCCESS;
}

// Test iterator state after queue modifications
int test_queue_iterator_modification(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVQueue* queue = anv_queue_create(&alloc);

    // Add initial data
    for (int i = 0; i < 3; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i * 10;
        ASSERT_EQ(anv_queue_enqueue(queue, data), 0);
    }

    ANVIterator it = anv_queue_iterator(queue);
    ASSERT(it.is_valid(&it));

    // Get first element
    void* first = it.get(&it);
    ASSERT_EQ(*(int*)first, 0); // Should be front element (0*10)
    it.next(&it);

    // Modify queue while iterator exists (implementation detail: iterator may become invalid)
    int* new_data = malloc(sizeof(int));
    *new_data = 999;
    ASSERT_EQ(anv_queue_enqueue(queue, new_data), 0);

    // Iterator should still be valid but may not reflect new state
    ASSERT(it.is_valid(&it));

    it.destroy(&it);
    anv_queue_destroy(queue, true);
    return TEST_SUCCESS;
}

// Test copy isolation - verify that copied elements are independent
int test_queue_copy_isolation(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create original data that we can modify
    int original_values[] = {10, 20, 30};
    int* data_ptrs[3];

    // Create a simple array-based iterator or use existing data structure
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);
    ASSERT_NOT_NULL(list);

    for (int i = 0; i < 3; i++)
    {
        data_ptrs[i] = malloc(sizeof(int));
        *data_ptrs[i] = original_values[i];
        ASSERT_EQ(anv_arraylist_push_back(list, data_ptrs[i]), 0);
    }

    ANVIterator list_it = anv_arraylist_iterator(list);
    ASSERT(list_it.is_valid(&list_it));

    // Create queue with copying enabled
    ANVQueue* queue = anv_queue_from_iterator(&list_it, &alloc, true);
    ASSERT_NOT_NULL(queue);
    ASSERT_EQ(anv_queue_size(queue), 3);

    // Modify original data
    *data_ptrs[0] = 999;
    *data_ptrs[1] = 888;
    *data_ptrs[2] = 777;

    // Queue should still have original values (proving data was copied)
    // FIFO order: 10, 20, 30
    void* queue_data = anv_queue_dequeue_data(queue);
    ASSERT_NOT_NULL(queue_data);
    ASSERT_EQ(*(int*)queue_data, 10); // Should be unchanged
    free(queue_data);

    queue_data = anv_queue_dequeue_data(queue);
    ASSERT_NOT_NULL(queue_data);
    ASSERT_EQ(*(int*)queue_data, 20); // Should be unchanged
    free(queue_data);

    queue_data = anv_queue_dequeue_data(queue);
    ASSERT_NOT_NULL(queue_data);
    ASSERT_EQ(*(int*)queue_data, 30); // Should be unchanged
    free(queue_data);

    // Cleanup
    list_it.destroy(&list_it);
    anv_queue_destroy(queue, false);
    anv_arraylist_destroy(list, true);

    return TEST_SUCCESS;
}

// Test that should_copy=true fails when allocator has no copy function
int test_queue_anv_copy_function_required(void)
{
    ANVAllocator alloc = anv_alloc_default();
    alloc.copy = NULL;

    ANVIterator range_it = anv_iterator_range(0, 3, 1, &alloc);
    ASSERT(range_it.is_valid(&range_it));

    // Should return NULL because should_copy=true but no copy function available
    ANVQueue* queue = anv_queue_from_iterator(&range_it, &alloc, true);
    ASSERT_NULL(queue);

    range_it.destroy(&range_it);
    return TEST_SUCCESS;
}

// Test that should_copy=false uses elements directly without copying
int test_queue_from_iterator_no_copy(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create a range iterator and then a copy iterator to get actual owned data
    ANVIterator range_it = anv_iterator_range(0, 3, 1, &alloc);
    ASSERT(range_it.is_valid(&range_it));

    // Use copy iterator to create actual data elements that we own
    ANVIterator copy_it = anv_iterator_copy(&range_it, &alloc, int_copy);
    ASSERT(copy_it.is_valid(&copy_it));

    // Create queue without copying (should_copy = false)
    // This will use the copied elements directly from the copy iterator
    ANVQueue* queue = anv_queue_from_iterator(&copy_it, &alloc, false);
    ASSERT_NOT_NULL(queue);
    ASSERT_EQ(anv_queue_size(queue), 3);

    // Verify values are correct (FIFO order: 0, 1, 2)
    void* data = anv_queue_dequeue_data(queue);
    ASSERT_NOT_NULL(data);
    ASSERT_EQ(*(int*)data, 0);
    free(data); // We own this data from the copy iterator

    data = anv_queue_dequeue_data(queue);
    ASSERT_NOT_NULL(data);
    ASSERT_EQ(*(int*)data, 1);
    free(data); // We own this data from the copy iterator

    data = anv_queue_dequeue_data(queue);
    ASSERT_NOT_NULL(data);
    ASSERT_EQ(*(int*)data, 2);
    free(data); // We own this data from the copy iterator

    range_it.destroy(&range_it);
    copy_it.destroy(&copy_it);
    anv_queue_destroy(queue, false); // Don't free elements since we already freed them
    return TEST_SUCCESS;
}

// Test that iterator is exhausted after being consumed by anv_queue_from_iterator
int test_iterator_exhaustion_after_queue_creation(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVIterator range_it = anv_iterator_range(0, 5, 1, &alloc);
    ASSERT(range_it.is_valid(&range_it));

    // Verify iterator starts with elements
    ASSERT(range_it.has_next(&range_it));

    // Create queue from iterator (consumes all elements)
    ANVQueue* queue = anv_queue_from_iterator(&range_it, &alloc, true);
    ASSERT_NOT_NULL(queue);
    ASSERT_EQ(anv_queue_size(queue), 5);

    // Iterator should now be exhausted
    ASSERT(!range_it.has_next(&range_it));
    ASSERT_NULL(range_it.get(&range_it));
    ASSERT_EQ(range_it.next(&range_it), -1); // Should fail to advance

    // But iterator should still be valid
    ASSERT(range_it.is_valid(&range_it));

    range_it.destroy(&range_it);
    anv_queue_destroy(queue, true);
    return TEST_SUCCESS;
}

// Test next() return values for proper error handling
int test_queue_iterator_next_return_values(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVQueue* queue = anv_queue_create(&alloc);
    ASSERT_NOT_NULL(queue);

    // Add single element
    int* data = malloc(sizeof(int));
    *data = 42;
    ASSERT_EQ(anv_queue_enqueue(queue, data), 0);

    ANVIterator it = anv_queue_iterator(queue);
    ASSERT(it.is_valid(&it));

    // Should successfully advance once
    ASSERT(it.has_next(&it));
    ASSERT_EQ(it.next(&it), 0); // Success

    // Should fail to advance when exhausted
    ASSERT(!it.has_next(&it));
    ASSERT_EQ(it.next(&it), -1); // Failure

    // Additional calls should continue to fail
    ASSERT_EQ(it.next(&it), -1); // Still failure
    ASSERT(!it.has_next(&it));   // Still no elements

    it.destroy(&it);
    anv_queue_destroy(queue, true);
    return TEST_SUCCESS;
}

// Test various combinations of get/next/has_next calls for consistency
int test_queue_iterator_mixed_operations(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVQueue* queue = anv_queue_create(&alloc);
    ASSERT_NOT_NULL(queue);

    // Add test data (will be in FIFO order: 0, 10, 20)
    for (int i = 0; i < 3; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i * 10;
        ASSERT_EQ(anv_queue_enqueue(queue, data), 0);
    }

    ANVIterator it = anv_queue_iterator(queue);
    ASSERT(it.is_valid(&it));

    // Multiple get() calls should return same value
    void* data1 = it.get(&it);
    void* data2 = it.get(&it);
    ASSERT_NOT_NULL(data1);
    ASSERT_NOT_NULL(data2);
    ASSERT_EQ(data1, data2);               // Same pointer
    ASSERT_EQ(*(int*)data1, *(int*)data2); // Same value
    ASSERT_EQ(*(int*)data1, 0);            // Front element should be 0

    // has_next should be consistent
    ASSERT(it.has_next(&it));
    ASSERT(it.has_next(&it)); // Multiple calls should be safe

    // Advance and verify new position
    ASSERT_EQ(it.next(&it), 0);
    void* data3 = it.get(&it);
    ASSERT_NOT_NULL(data3);
    // Note: data1 and data3 point to different queue elements
    ASSERT_NOT_EQ(*(int*)data1, *(int*)data3); // Different values
    ASSERT_EQ(*(int*)data3, 10);               // Next element should be 10

    // Verify we can still advance
    ASSERT(it.has_next(&it));
    ASSERT_EQ(it.next(&it), 0);

    void* data4 = it.get(&it);
    ASSERT_NOT_NULL(data4);
    ASSERT_EQ(*(int*)data4, 20); // Last element should be 20

    // Now should be at end
    ASSERT_EQ(it.next(&it), 0); // Advance past last element
    ASSERT(!it.has_next(&it));
    ASSERT_NULL(it.get(&it));

    it.destroy(&it);
    anv_queue_destroy(queue, true);
    return TEST_SUCCESS;
}

// Test iterator traversal order
int test_queue_iterator_order(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVQueue* queue = anv_queue_create(&alloc);

    // Add elements in specific order
    const int values[] = {100, 200, 300, 400, 500};
    for (int i = 0; i < 5; i++)
    {
        int* data = malloc(sizeof(int));
        *data = values[i];
        ASSERT_EQ(anv_queue_enqueue(queue, data), 0);
    }

    // Create iterator and verify order
    ANVIterator it = anv_queue_iterator(queue);

    for (int i = 0; i < 5; i++)
    {
        ASSERT(it.has_next(&it));
        void* data = it.get(&it);
        ASSERT_NOT_NULL(data);
        ASSERT_EQ(*(int*)data, values[i]);
        it.next(&it);
    }

    ASSERT(!it.has_next(&it));

    it.destroy(&it);
    anv_queue_destroy(queue, true);
    return TEST_SUCCESS;
}

// Test iterator get function
int test_queue_iterator_get(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVQueue* queue = anv_queue_create(&alloc);

    // Add test data
    for (int i = 1; i <= 3; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i;
        ASSERT_EQ(anv_queue_enqueue(queue, data), 0);
    }

    ANVIterator it = anv_queue_iterator(queue);

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

    it.destroy(&it);
    anv_queue_destroy(queue, true);
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
        {test_queue_iterator, "test_queue_iterator"},
        {test_queue_from_iterator, "test_queue_from_iterator"},
        {test_queue_iterator_empty, "test_queue_iterator_empty"},
        {test_queue_iterator_invalid, "test_queue_iterator_invalid"},
        {test_queue_iterator_modification, "test_queue_iterator_modification"},
        {test_queue_copy_isolation, "test_queue_copy_isolation"},
        {test_queue_anv_copy_function_required, "test_queue_anv_copy_function_required"},
        {test_queue_from_iterator_no_copy, "test_queue_from_iterator_no_copy"},
        {test_iterator_exhaustion_after_queue_creation, "test_iterator_exhaustion_after_queue_creation"},
        {test_queue_iterator_next_return_values, "test_queue_iterator_next_return_values"},
        {test_queue_iterator_mixed_operations, "test_queue_iterator_mixed_operations"},
        {test_queue_iterator_order, "test_queue_iterator_order"},
        {test_queue_iterator_get, "test_queue_iterator_get"},
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

    printf("All queue iterator tests passed!\n");
    return 0;
}