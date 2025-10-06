//
// Created by zack on 9/9/25.
//

#include "TestAssert.h"
#include "TestHelpers.h"
#include "containers/queue.h"
#include <stdio.h>
#include <stdlib.h>

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

typedef struct
{
    int (*func)(void);
    const char* name;
} TestCase;

int main(void)
{
    const TestCase tests[] = {
        {test_queue_fifo_property, "test_queue_fifo_property"},
        {test_queue_size_consistency, "test_queue_size_consistency"},
        {test_queue_front_back_invariants, "test_queue_front_back_invariants"},
        {test_queue_copy_preserves_order, "test_queue_copy_preserves_order"},
        {test_queue_clear_preserves_structure, "test_queue_clear_preserves_structure"},
        {test_queue_for_each_preserves_contents, "test_queue_for_each_preserves_contents"},
        {test_queue_mixed_operations_fifo, "test_queue_mixed_operations_fifo"},
    };

    printf("Running Queue property tests...\n");

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

    if (failed == 0)
    {
        printf("All Queue property tests passed!\n");
        return 0;
    }

    printf("%d Queue property tests failed.\n", failed);
    return 1;
}