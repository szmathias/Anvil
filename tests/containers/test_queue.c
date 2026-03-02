#include "TestAssert.h"
#include "TestHelpers.h"
#include "TestRunner.h"
#include "containers/arraylist.h"
#include "containers/queue.h"
#include <stdio.h>
#include <stdlib.h>

//==============================================================================
// CRUD Tests
//==============================================================================

// Test basic queue creation and destruction
int test_queue_create_destroy(void)
{
    ANVAllocator alloc = create_int_allocator();

    ANVQueue* queue = anv_queue_create(&alloc);
    ASSERT_NOT_NULL(queue);
    ASSERT_EQ(anv_queue_size(queue), 0);
    ASSERT(anv_queue_is_empty(queue));

    anv_queue_destroy(queue, false);
    return TEST_SUCCESS;
}

// Test NULL parameter handling
int test_queue_null_parameters(void)
{
    // Creating with NULL allocator should fail
    ASSERT_NULL(anv_queue_create(NULL));

    // Operations on NULL queue should be safe
    ASSERT_EQ(anv_queue_size(NULL), 0);
    ASSERT(anv_queue_is_empty(NULL));
    ASSERT_NULL(anv_queue_front(NULL));
    ASSERT_NULL(anv_queue_back(NULL));
    ASSERT_EQ(anv_queue_enqueue(NULL, NULL), -1);
    ASSERT_EQ(anv_queue_dequeue(NULL, false), -1);
    ASSERT_NULL(anv_queue_dequeue_data(NULL));

    // Destruction should be safe
    anv_queue_destroy(NULL, false);
    anv_queue_clear(NULL, false);

    return TEST_SUCCESS;
}

// Test basic enqueue and dequeue operations
int test_queue_enqueue_dequeue(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVQueue* queue = anv_queue_create(&alloc);

    int* data1 = malloc(sizeof(int));
    int* data2 = malloc(sizeof(int));
    int* data3 = malloc(sizeof(int));
    *data1 = 10;
    *data2 = 20;
    *data3 = 30;

    // Test enqueuing elements
    ASSERT_EQ(anv_queue_enqueue(queue, data1), 0);
    ASSERT_EQ(anv_queue_size(queue), 1);
    ASSERT(!anv_queue_is_empty(queue));
    ASSERT_EQ(*(int*)anv_queue_front(queue), 10);
    ASSERT_EQ(*(int*)anv_queue_back(queue), 10); // Front and back same for single element

    ASSERT_EQ(anv_queue_enqueue(queue, data2), 0);
    ASSERT_EQ(anv_queue_size(queue), 2);
    ASSERT_EQ(*(int*)anv_queue_front(queue), 10); // FIFO - front should still be data1
    ASSERT_EQ(*(int*)anv_queue_back(queue), 20);  // Back should be data2

    ASSERT_EQ(anv_queue_enqueue(queue, data3), 0);
    ASSERT_EQ(anv_queue_size(queue), 3);
    ASSERT_EQ(*(int*)anv_queue_front(queue), 10); // FIFO - front should still be data1
    ASSERT_EQ(*(int*)anv_queue_back(queue), 30);  // Back should be data3

    // Test dequeuing elements
    ASSERT_EQ(anv_queue_dequeue(queue, true), 0); // Dequeues data1 and frees it
    ASSERT_EQ(anv_queue_size(queue), 2);
    ASSERT_EQ(*(int*)anv_queue_front(queue), 20); // Should see data2

    ASSERT_EQ(anv_queue_dequeue(queue, true), 0); // Dequeues data2 and frees it
    ASSERT_EQ(anv_queue_size(queue), 1);
    ASSERT_EQ(*(int*)anv_queue_front(queue), 30); // Should see data3
    ASSERT_EQ(*(int*)anv_queue_back(queue), 30);  // Front and back same again

    ASSERT_EQ(anv_queue_dequeue(queue, true), 0); // Dequeues data3 and frees it
    ASSERT_EQ(anv_queue_size(queue), 0);
    ASSERT(anv_queue_is_empty(queue));
    ASSERT_NULL(anv_queue_front(queue));
    ASSERT_NULL(anv_queue_back(queue));

    // Test dequeuing from empty queue
    ASSERT_EQ(anv_queue_dequeue(queue, false), -1);

    anv_queue_destroy(queue, false);
    return TEST_SUCCESS;
}

// Test dequeue_data function
int test_queue_dequeue_data(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVQueue* queue = anv_queue_create(&alloc);

    int* data1 = malloc(sizeof(int));
    int* data2 = malloc(sizeof(int));
    *data1 = 42;
    *data2 = 84;

    ASSERT_EQ(anv_queue_enqueue(queue, data1), 0);
    ASSERT_EQ(anv_queue_enqueue(queue, data2), 0);

    // Dequeue data1 and get its value
    void* dequeued = anv_queue_dequeue_data(queue);
    ASSERT_NOT_NULL(dequeued);
    ASSERT_EQ(*(int*)dequeued, 42);
    ASSERT_EQ(anv_queue_size(queue), 1);
    free(dequeued); // Caller's responsibility to free

    // Dequeue data2 and get its value
    dequeued = anv_queue_dequeue_data(queue);
    ASSERT_NOT_NULL(dequeued);
    ASSERT_EQ(*(int*)dequeued, 84);
    ASSERT_EQ(anv_queue_size(queue), 0);
    free(dequeued);

    // Dequeue from empty queue
    ASSERT_NULL(anv_queue_dequeue_data(queue));

    anv_queue_destroy(queue, false);
    return TEST_SUCCESS;
}

// Test queue clear operation
int test_queue_clear(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVQueue* queue = anv_queue_create(&alloc);

    // Add some elements
    for (int i = 0; i < 5; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i * 10;
        ASSERT_EQ(anv_queue_enqueue(queue, data), 0);
    }

    ASSERT_EQ(anv_queue_size(queue), 5);

    // Clear with freeing data
    anv_queue_clear(queue, true);
    ASSERT_EQ(anv_queue_size(queue), 0);
    ASSERT(anv_queue_is_empty(queue));
    ASSERT_NULL(anv_queue_front(queue));
    ASSERT_NULL(anv_queue_back(queue));

    // Queue should still be usable after clear
    int* new_data = malloc(sizeof(int));
    *new_data = 999;
    ASSERT_EQ(anv_queue_enqueue(queue, new_data), 0);
    ASSERT_EQ(anv_queue_size(queue), 1);
    ASSERT_EQ(*(int*)anv_queue_front(queue), 999);

    anv_queue_destroy(queue, true);
    return TEST_SUCCESS;
}

// Test queue equality
int test_queue_equals(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVQueue* queue1 = anv_queue_create(&alloc);
    ANVQueue* queue2 = anv_queue_create(&alloc);

    // Empty queues should be equal
    ASSERT_EQ(anv_queue_equals(queue1, queue2, int_cmp), 1);

    // Same queue should be equal to itself
    ASSERT_EQ(anv_queue_equals(queue1, queue1, int_cmp), 1);

    // Add same elements to both queues
    for (int i = 0; i < 3; i++)
    {
        int* data1 = malloc(sizeof(int));
        int* data2 = malloc(sizeof(int));
        *data1 = *data2 = i * 10;
        ASSERT_EQ(anv_queue_enqueue(queue1, data1), 0);
        ASSERT_EQ(anv_queue_enqueue(queue2, data2), 0);
    }

    ASSERT_EQ(anv_queue_equals(queue1, queue2, int_cmp), 1);

    // Add different element to one queue
    int* diff_data = malloc(sizeof(int));
    *diff_data = 999;
    ASSERT_EQ(anv_queue_enqueue(queue1, diff_data), 0);

    ASSERT_EQ(anv_queue_equals(queue1, queue2, int_cmp), 0);

    // Test with NULL parameters
    ASSERT_EQ(anv_queue_equals(NULL, queue2, int_cmp), -1);
    ASSERT_EQ(anv_queue_equals(queue1, NULL, int_cmp), -1);
    ASSERT_EQ(anv_queue_equals(queue1, queue2, NULL), -1);

    anv_queue_destroy(queue1, true);
    anv_queue_destroy(queue2, true);
    return TEST_SUCCESS;
}

// Test FIFO behavior specifically
int test_queue_fifo_behavior(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVQueue* queue = anv_queue_create(&alloc);

    // Enqueue numbers 0-9
    for (int i = 0; i < 10; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i;
        ASSERT_EQ(anv_queue_enqueue(queue, data), 0);
    }

    // Dequeue should give us 0-9 in order (FIFO)
    for (int i = 0; i < 10; i++)
    {
        void* data = anv_queue_dequeue_data(queue);
        ASSERT_NOT_NULL(data);
        ASSERT_EQ(*(int*)data, i);
        free(data);
    }

    ASSERT(anv_queue_is_empty(queue));

    anv_queue_destroy(queue, false);
    return TEST_SUCCESS;
}

//==============================================================================
// Iterator Tests
//==============================================================================

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
    ANVIterator range_it = anv_iterator_range(&alloc, 0, 5, 1);

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

    ANVIterator range_it = anv_iterator_range(&alloc, 0, 3, 1);
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
    ANVIterator range_it = anv_iterator_range(&alloc, 0, 3, 1);
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
    ANVIterator range_it = anv_iterator_range(&alloc, 0, 5, 1);
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

//==============================================================================
// Algorithm Tests
//==============================================================================

// Test queue copying (shallow)
int test_queue_copy_shallow(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVQueue* original = anv_queue_create(&alloc);

    // Add some test data
    const int original_values[] = {10, 20, 30, 40, 50};
    int* data_ptrs[5];

    for (int i = 0; i < 5; i++)
    {
        data_ptrs[i] = malloc(sizeof(int));
        *data_ptrs[i] = original_values[i];
        ASSERT_EQ(anv_queue_enqueue(original, data_ptrs[i]), 0);
    }

    // Create shallow copy
    ANVQueue* copy = anv_queue_copy(original);
    ASSERT_NOT_NULL(copy);
    ASSERT_EQ(anv_queue_size(copy), 5);
    ASSERT_EQ(anv_queue_equals(original, copy, int_cmp), 1);

    // Verify data is shared (same pointers) and in FIFO order
    for (int i = 0; i < 5; i++)
    {
        // FIFO order
        void* orig_data = anv_queue_dequeue_data(original);
        void* copy_data = anv_queue_dequeue_data(copy);
        ASSERT_EQ_PTR(orig_data, copy_data); // Should be same pointer
        ASSERT_EQ(*(int*)orig_data, original_values[i]);
        // Don't free - they're the same pointer
    }

    // Free the shared data once
    for (int i = 0; i < 5; i++)
    {
        free(data_ptrs[i]);
    }

    anv_queue_destroy(original, false);
    anv_queue_destroy(copy, false);
    return TEST_SUCCESS;
}

// Test queue copying (deep)
int test_queue_copy_deep(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVQueue* original = anv_queue_create(&alloc);

    // Add some test data
    const int original_values[] = {10, 20, 30};

    for (int i = 0; i < 3; i++)
    {
        int* data = malloc(sizeof(int));
        *data = original_values[i];
        ASSERT_EQ(anv_queue_enqueue(original, data), 0);
    }

    // Create deep copy
    ANVQueue* copy = anv_queue_copy_deep(original, false);
    ASSERT_NOT_NULL(copy);
    ASSERT_EQ(anv_queue_size(copy), 3);
    ASSERT_EQ(anv_queue_equals(original, copy, int_cmp), 1);

    // Verify data is different (different pointers, same values) in FIFO order
    for (int i = 0; i < 3; i++)
    {
        // FIFO order
        void* orig_data = anv_queue_dequeue_data(original);
        void* copy_data = anv_queue_dequeue_data(copy);
        ASSERT_NOT_EQ_PTR(orig_data, copy_data);       // Should be different pointers
        ASSERT_EQ(*(int*)orig_data, *(int*)copy_data); // Same values
        ASSERT_EQ(*(int*)orig_data, original_values[i]);
        free(orig_data);
        free(copy_data);
    }

    anv_queue_destroy(original, false);
    anv_queue_destroy(copy, false);
    return TEST_SUCCESS;
}

// Test for_each functionality
int test_queue_for_each(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVQueue* queue = anv_queue_create(&alloc);

    // Add some test data
    for (int i = 1; i <= 5; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i * 10;
        ASSERT_EQ(anv_queue_enqueue(queue, data), 0);
    }

    // Use increment action to modify all elements
    anv_queue_for_each(queue, increment);

    // Verify elements were incremented (should be 11, 21, 31, 41, 51 in FIFO order)
    for (int i = 0; i < 5; i++)
    {
        const int expected[] = {11, 21, 31, 41, 51};
        void* data = anv_queue_dequeue_data(queue);
        ASSERT_EQ(*(int*)data, expected[i]);
        free(data);
    }

    // Test with NULL parameters
    anv_queue_for_each(NULL, increment); // Should be safe
    anv_queue_for_each(queue, NULL);     // Should be safe

    anv_queue_destroy(queue, false);
    return TEST_SUCCESS;
}

// Test queue with Person objects
int test_queue_with_persons(void)
{
    ANVAllocator alloc = create_person_allocator();

    ANVQueue* queue = anv_queue_create(&alloc);

    // Create and enqueue some persons
    Person* alice = create_person("Alice", 25);
    Person* bob = create_person("Bob", 30);
    Person* charlie = create_person("Charlie", 35);

    ASSERT_EQ(anv_queue_enqueue(queue, alice), 0);
    ASSERT_EQ(anv_queue_enqueue(queue, bob), 0);
    ASSERT_EQ(anv_queue_enqueue(queue, charlie), 0);

    // Check front and back
    Person* front = (Person*)anv_queue_front(queue);
    ASSERT_NOT_NULL(front);
    ASSERT_EQ_STR(front->name, "Alice");
    ASSERT_EQ(front->age, 25);

    Person* back = (Person*)anv_queue_back(queue);
    ASSERT_NOT_NULL(back);
    ASSERT_EQ_STR(back->name, "Charlie");
    ASSERT_EQ(back->age, 35);

    // Test deep copy
    ANVQueue* copy = anv_queue_copy_deep(queue, false);
    ASSERT_NOT_NULL(copy);
    ASSERT_EQ(anv_queue_equals(queue, copy, person_cmp), 1);

    // Verify persons are in correct FIFO order
    const char* expected_names[] = {"Alice", "Bob", "Charlie"};
    int expected_ages[] = {25, 30, 35};

    for (int i = 0; i < 3; i++)
    {
        Person* person = (Person*)anv_queue_dequeue_data(copy);
        ASSERT_NOT_NULL(person);
        ASSERT_EQ_STR(person->name, expected_names[i]);
        ASSERT_EQ(person->age, expected_ages[i]);
        free(person);
    }

    anv_queue_destroy(queue, true);
    anv_queue_destroy(copy, false);
    return TEST_SUCCESS;
}

// Test mixed enqueue/dequeue operations
int test_queue_mixed_operations(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVQueue* queue = anv_queue_create(&alloc);

    // Mix enqueue and dequeue operations
    for (int i = 0; i < 3; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i;
        ASSERT_EQ(anv_queue_enqueue(queue, data), 0);
    }

    // Dequeue one element
    void* dequeued = anv_queue_dequeue_data(queue);
    ASSERT_EQ(*(int*)dequeued, 0);
    free(dequeued);
    ASSERT_EQ(anv_queue_size(queue), 2);

    // Add more elements
    for (int i = 3; i < 6; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i;
        ASSERT_EQ(anv_queue_enqueue(queue, data), 0);
    }

    ASSERT_EQ(anv_queue_size(queue), 5);

    // Dequeue remaining elements in FIFO order: 1, 2, 3, 4, 5
    for (int expected = 1; expected <= 5; expected++)
    {
        void* data = anv_queue_dequeue_data(queue);
        ASSERT_EQ(*(int*)data, expected);
        free(data);
    }

    ASSERT(anv_queue_is_empty(queue));

    anv_queue_destroy(queue, false);
    return TEST_SUCCESS;
}

//==============================================================================
// Memory Tests
//==============================================================================

// Test queue with failing allocator
int test_queue_failing_allocator(void)
{
    ANVAllocator failing_alloc = create_failing_int_allocator();

    // Set to fail immediately
    set_alloc_fail_countdown(0);

    // Queue creation should fail
    ANVQueue* queue = anv_queue_create(&failing_alloc);
    ASSERT_NULL(queue);

    return TEST_SUCCESS;
}

// Test enqueue with failing allocator
int test_queue_enqueue_memory_failure(void)
{
    ANVAllocator failing_alloc = create_failing_int_allocator();

    // Allow queue creation but fail on first enqueue
    set_alloc_fail_countdown(1);

    ANVQueue* queue = anv_queue_create(&failing_alloc);
    ASSERT_NOT_NULL(queue);

    int* data = malloc(sizeof(int));
    *data = 42;

    // Enqueue should fail due to node allocation failure
    ASSERT_EQ(anv_queue_enqueue(queue, data), -1);
    ASSERT_EQ(anv_queue_size(queue), 0);

    free(data);
    anv_queue_destroy(queue, false);
    return TEST_SUCCESS;
}

// Test copy with failing allocator
int test_queue_copy_memory_failure(void)
{
    ANVAllocator std_alloc = create_int_allocator();
    ANVQueue* original = anv_queue_create(&std_alloc);

    // Add some data
    for (int i = 0; i < 3; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i * 10;
        ASSERT_EQ(anv_queue_enqueue(original, data), 0);
    }

    // Replace allocator with failing one
    ANVAllocator failing_alloc = create_failing_int_allocator();
    original->alloc = failing_alloc;

    // Set to fail on copy creation
    set_alloc_fail_countdown(0);

    ANVQueue* copy = anv_queue_copy(original);
    ASSERT_NULL(copy);

    // Restore original allocator for cleanup
    original->alloc = std_alloc;
    anv_queue_destroy(original, true);

    return TEST_SUCCESS;
}

// Test deep copy with failing copy function
int test_queue_deep_copy_failure(void)
{
    set_alloc_fail_countdown(-1);
    ANVAllocator failing_alloc = create_failing_int_allocator();
    ANVQueue* original = anv_queue_create(&failing_alloc);

    // Add some data
    for (int i = 0; i < 3; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i * 10;
        ASSERT_EQ(anv_queue_enqueue(original, data), 0);
    }

    // Set to fail on copy function calls
    set_alloc_fail_countdown(2); // Allow queue creation, fail on first copy

    ANVQueue* copy = anv_queue_copy_deep(original, true);
    ASSERT_NULL(copy);

    anv_queue_destroy(original, true);

    return TEST_SUCCESS;
}

// Test memory usage with large number of elements
int test_queue_large_memory_usage(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVQueue* queue = anv_queue_create(&alloc);

    const int num_elements = 10000;

    // Enqueue many elements
    for (int i = 0; i < num_elements; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i;
        ASSERT_EQ(anv_queue_enqueue(queue, data), 0);
    }

    ASSERT_EQ(anv_queue_size(queue), (size_t)num_elements);

    // Dequeue all elements in FIFO order
    for (int i = 0; i < num_elements; i++)
    {
        void* data = anv_queue_dequeue_data(queue);
        ASSERT_NOT_NULL(data);
        ASSERT_EQ(*(int*)data, i);
        free(data);
    }

    ASSERT(anv_queue_is_empty(queue));

    anv_queue_destroy(queue, false);

    return TEST_SUCCESS;
}

// Test memory leaks with clear operations
int test_queue_clear_memory(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVQueue* queue = anv_queue_create(&alloc);

    // Add elements multiple times and clear
    for (int cycle = 0; cycle < 5; cycle++)
    {
        // Add elements
        for (int i = 0; i < 100; i++)
        {
            int* data = malloc(sizeof(int));
            *data = i;
            ASSERT_EQ(anv_queue_enqueue(queue, data), 0);
        }

        ASSERT_EQ(anv_queue_size(queue), 100);

        // Clear with memory cleanup
        anv_queue_clear(queue, true);
        ASSERT_EQ(anv_queue_size(queue), 0);
        ASSERT(anv_queue_is_empty(queue));
    }

    anv_queue_destroy(queue, false);

    return TEST_SUCCESS;
}

// Test iterator memory with failing allocator
int test_queue_iterator_memory_failure(void)
{
    set_alloc_fail_countdown(-1);
    ANVAllocator failing_alloc = create_failing_int_allocator();
    ANVQueue* queue = anv_queue_create(&failing_alloc);

    // Add some data
    int* data = malloc(sizeof(int));
    *data = 42;
    ASSERT_EQ(anv_queue_enqueue(queue, data), 0);

    // Set to fail on iterator state allocation
    set_alloc_fail_countdown(0);

    const ANVIterator it = anv_queue_iterator(queue);
    ASSERT(!it.is_valid(&it));

    anv_queue_destroy(queue, true);

    return TEST_SUCCESS;
}

// Test front/back pointer consistency under memory pressure
int test_queue_front_back_consistency(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVQueue* queue = anv_queue_create(&alloc);

    // Test single element case
    int* single_data = malloc(sizeof(int));
    *single_data = 999;
    ASSERT_EQ(anv_queue_enqueue(queue, single_data), 0);

    // Front and back should point to same element
    ASSERT_EQ_PTR(anv_queue_front(queue), anv_queue_back(queue));
    ASSERT_EQ(*(int*)anv_queue_front(queue), 999);

    // Remove the element
    ASSERT_EQ(anv_queue_dequeue(queue, true), 0);
    ASSERT_NULL(anv_queue_front(queue));
    ASSERT_NULL(anv_queue_back(queue));

    // Add multiple elements and test front/back tracking
    for (int i = 0; i < 100; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i;
        ASSERT_EQ(anv_queue_enqueue(queue, data), 0);

        // Front should always be first element, back should be last
        ASSERT_EQ(*(int*)anv_queue_front(queue), 0);
        ASSERT_EQ(*(int*)anv_queue_back(queue), i);
    }

    anv_queue_destroy(queue, true);

    return TEST_SUCCESS;
}

//==============================================================================
// Property Tests
//==============================================================================

// Test FIFO property extensively
int test_queue_fifo_property(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVQueue* queue = anv_queue_create(&alloc);

    #define num_elements 100
    int* values[num_elements];

    // Enqueue elements in order
    for (int i = 0; i < num_elements; i++)
    {
        values[i] = malloc(sizeof(int));
        *values[i] = i * 7; // Use non-sequential values
        ASSERT_EQ(anv_queue_enqueue(queue, values[i]), 0);
    }

    // Dequeue elements - should come out in same order (FIFO)
    for (int i = 0; i < num_elements; i++)
    {
        void* data = anv_queue_dequeue_data(queue);
        ASSERT_NOT_NULL(data);
        ASSERT_EQ_PTR(data, values[i]); // Should be exact same pointer
        ASSERT_EQ(*(int*)data, i * 7);
        free(data);
    }

    ASSERT(anv_queue_is_empty(queue));

    anv_queue_destroy(queue, false);
    return TEST_SUCCESS;
}

// Test queue size consistency
int test_queue_size_consistency(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVQueue* queue = anv_queue_create(&alloc);

    // Size should start at 0
    ASSERT_EQ(anv_queue_size(queue), 0);
    ASSERT(anv_queue_is_empty(queue));

    // Size should increase with each enqueue
    for (int i = 1; i <= 50; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i;
        ASSERT_EQ(anv_queue_enqueue(queue, data), 0);
        ASSERT_EQ(anv_queue_size(queue), (size_t)i);
        ASSERT(!anv_queue_is_empty(queue));
    }

    // Size should decrease with each dequeue
    for (int i = 49; i >= 0; i--)
    {
        ASSERT_EQ(anv_queue_dequeue(queue, true), 0);
        ASSERT_EQ(anv_queue_size(queue), (size_t)i);

        if (i == 0)
        {
            ASSERT(anv_queue_is_empty(queue));
        }
        else
        {
            ASSERT(!anv_queue_is_empty(queue));
        }
    }

    anv_queue_destroy(queue, false);
    return TEST_SUCCESS;
}

// Test front/back access invariants
int test_queue_front_back_invariants(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVQueue* queue = anv_queue_create(&alloc);

    int* data1 = malloc(sizeof(int));
    int* data2 = malloc(sizeof(int));
    int* data3 = malloc(sizeof(int));
    *data1 = 10;
    *data2 = 20;
    *data3 = 30;

    ASSERT_EQ(anv_queue_enqueue(queue, data1), 0);
    ASSERT_EQ(anv_queue_enqueue(queue, data2), 0);
    ASSERT_EQ(anv_queue_enqueue(queue, data3), 0);

    size_t original_size = anv_queue_size(queue);

    // Multiple front/back accesses should return same values and not change size
    for (int i = 0; i < 10; i++)
    {
        void* front = anv_queue_front(queue);
        void* back = anv_queue_back(queue);

        ASSERT_NOT_NULL(front);
        ASSERT_NOT_NULL(back);
        ASSERT_EQ(*(int*)front, 10); // First enqueued
        ASSERT_EQ(*(int*)back, 30);  // Last enqueued
        ASSERT_EQ(anv_queue_size(queue), original_size);
    }

    anv_queue_destroy(queue, true);
    return TEST_SUCCESS;
}

// Test copy preserves order
int test_queue_copy_preserves_order(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVQueue* original = anv_queue_create(&alloc);

    const int values[] = {1, 3, 5, 7, 9, 11, 13};
    const int num_values = sizeof(values) / sizeof(values[0]);

    // Build original queue
    for (int i = 0; i < num_values; i++)
    {
        int* data = malloc(sizeof(int));
        *data = values[i];
        ASSERT_EQ(anv_queue_enqueue(original, data), 0);
    }

    // Create shallow copy
    ANVQueue* shallow_copy = anv_queue_copy(original);
    ASSERT_NOT_NULL(shallow_copy);

    // Create deep copy
    ANVQueue* deep_copy = anv_queue_copy_deep(original, false);
    ASSERT_NOT_NULL(deep_copy);

    // All three queues should have same size and equal contents
    ASSERT_EQ(anv_queue_size(original), (size_t)num_values);
    ASSERT_EQ(anv_queue_size(shallow_copy), (size_t)num_values);
    ASSERT_EQ(anv_queue_size(deep_copy), (size_t)num_values);

    ASSERT_EQ(anv_queue_equals(original, shallow_copy, int_cmp), 1);
    ASSERT_EQ(anv_queue_equals(original, deep_copy, int_cmp), 1);

    // Dequeue from all three - should get same sequence (FIFO)
    for (int i = 0; i < num_values; i++)
    {
        void* orig_data = anv_queue_dequeue_data(original);
        void* shallow_data = anv_queue_dequeue_data(shallow_copy);
        void* deep_data = anv_queue_dequeue_data(deep_copy);

        ASSERT_EQ(*(int*)orig_data, values[i]);
        ASSERT_EQ(*(int*)shallow_data, values[i]);
        ASSERT_EQ(*(int*)deep_data, values[i]);

        // Shallow copy shares pointers, deep copy doesn't
        ASSERT_EQ_PTR(orig_data, shallow_data);
        ASSERT_NOT_EQ_PTR(orig_data, deep_data);

        free(orig_data); // Also frees shallow_data
        free(deep_data);
    }

    anv_queue_destroy(original, false);
    anv_queue_destroy(shallow_copy, false);
    anv_queue_destroy(deep_copy, false);
    return TEST_SUCCESS;
}

// Test clear preserves queue structure
int test_queue_clear_preserves_structure(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVQueue* queue = anv_queue_create(&alloc);

    // Add elements
    for (int i = 0; i < 10; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i;
        ASSERT_EQ(anv_queue_enqueue(queue, data), 0);
    }

    ASSERT_EQ(anv_queue_size(queue), 10);

    // Clear queue
    anv_queue_clear(queue, true);

    // Queue should be empty but still functional
    ASSERT_EQ(anv_queue_size(queue), 0);
    ASSERT(anv_queue_is_empty(queue));
    ASSERT_NULL(anv_queue_front(queue));
    ASSERT_NULL(anv_queue_back(queue));

    // Should be able to use queue normally after clear
    int* new_data = malloc(sizeof(int));
    *new_data = 999;
    ASSERT_EQ(anv_queue_enqueue(queue, new_data), 0);
    ASSERT_EQ(anv_queue_size(queue), 1);
    ASSERT_EQ(*(int*)anv_queue_front(queue), 999);
    ASSERT_EQ(*(int*)anv_queue_back(queue), 999);

    anv_queue_destroy(queue, true);
    return TEST_SUCCESS;
}

// Test for_each preserves queue contents
int test_queue_for_each_preserves_contents(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVQueue* queue = anv_queue_create(&alloc);

    const int original_values[] = {5, 10, 15, 20, 25};
    const int num_values = sizeof(original_values) / sizeof(original_values[0]);

    // Build queue
    for (int i = 0; i < num_values; i++)
    {
        int* data = malloc(sizeof(int));
        *data = original_values[i];
        ASSERT_EQ(anv_queue_enqueue(queue, data), 0);
    }

    size_t original_size = anv_queue_size(queue);

    // Apply for_each (increment each element)
    anv_queue_for_each(queue, increment);

    // Queue size should be unchanged
    ASSERT_EQ(anv_queue_size(queue), original_size);

    // Elements should be modified but order preserved (FIFO)
    for (int i = 0; i < num_values; i++)
    {
        void* data = anv_queue_dequeue_data(queue);
        ASSERT_NOT_NULL(data);
        ASSERT_EQ(*(int*)data, original_values[i] + 1); // Should be incremented
        free(data);
    }

    anv_queue_destroy(queue, false);
    return TEST_SUCCESS;
}

// Test mixed operations maintain FIFO property
int test_queue_mixed_operations_fifo(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVQueue* queue = anv_queue_create(&alloc);

    // Pattern: enqueue some, dequeue some, enqueue more
    const int sequence[] = {100, 200, 300};

    // Enqueue initial elements
    for (int i = 0; i < 3; i++)
    {
        int* data = malloc(sizeof(int));
        *data = sequence[i];
        ASSERT_EQ(anv_queue_enqueue(queue, data), 0);
    }

    // Dequeue first element
    void* first = anv_queue_dequeue_data(queue);
    ASSERT_EQ(*(int*)first, 100);
    free(first);

    // Add more elements
    const int more_sequence[] = {400, 500};
    for (int i = 0; i < 2; i++)
    {
        int* data = malloc(sizeof(int));
        *data = more_sequence[i];
        ASSERT_EQ(anv_queue_enqueue(queue, data), 0);
    }

    // Dequeue remaining should be in FIFO order: 200, 300, 400, 500
    const int expected[] = {200, 300, 400, 500};
    for (int i = 0; i < 4; i++)
    {
        void* data = anv_queue_dequeue_data(queue);
        ASSERT_NOT_NULL(data);
        ASSERT_EQ(*(int*)data, expected[i]);
        free(data);
    }

    ASSERT(anv_queue_is_empty(queue));

    anv_queue_destroy(queue, false);
    return TEST_SUCCESS;
}

//==============================================================================
// Main
//==============================================================================

int main(void)
{
    const ANVTestCase tests[] = {
        // CRUD Tests (7)
        {test_queue_create_destroy, "test_queue_create_destroy"},
        {test_queue_null_parameters, "test_queue_null_parameters"},
        {test_queue_enqueue_dequeue, "test_queue_enqueue_dequeue"},
        {test_queue_dequeue_data, "test_queue_dequeue_data"},
        {test_queue_clear, "test_queue_clear"},
        {test_queue_equals, "test_queue_equals"},
        {test_queue_fifo_behavior, "test_queue_fifo_behavior"},

        // Iterator Tests (13)
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

        // Algorithm Tests (5)
        {test_queue_copy_shallow, "test_queue_copy_shallow"},
        {test_queue_copy_deep, "test_queue_copy_deep"},
        {test_queue_for_each, "test_queue_for_each"},
        {test_queue_with_persons, "test_queue_with_persons"},
        {test_queue_mixed_operations, "test_queue_mixed_operations"},

        // Memory Tests (8)
        {test_queue_failing_allocator, "test_queue_failing_allocator"},
        {test_queue_enqueue_memory_failure, "test_queue_enqueue_memory_failure"},
        {test_queue_copy_memory_failure, "test_queue_copy_memory_failure"},
        {test_queue_deep_copy_failure, "test_queue_deep_copy_failure"},
        {test_queue_large_memory_usage, "test_queue_large_memory_usage"},
        {test_queue_clear_memory, "test_queue_clear_memory"},
        {test_queue_iterator_memory_failure, "test_queue_iterator_memory_failure"},
        {test_queue_front_back_consistency, "test_queue_front_back_consistency"},

        // Property Tests (7)
        {test_queue_fifo_property, "test_queue_fifo_property"},
        {test_queue_size_consistency, "test_queue_size_consistency"},
        {test_queue_front_back_invariants, "test_queue_front_back_invariants"},
        {test_queue_copy_preserves_order, "test_queue_copy_preserves_order"},
        {test_queue_clear_preserves_structure, "test_queue_clear_preserves_structure"},
        {test_queue_for_each_preserves_contents, "test_queue_for_each_preserves_contents"},
        {test_queue_mixed_operations_fifo, "test_queue_mixed_operations_fifo"},
    };

    return anv_run_tests("Queue", tests, sizeof(tests) / sizeof(tests[0]));
}
