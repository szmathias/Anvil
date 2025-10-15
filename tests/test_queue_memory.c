//
// Created by zack on 9/9/25.
//

#include "TestAssert.h"
#include "TestHelpers.h"
#include "containers/queue.h"
#include <stdio.h>
#include <stdlib.h>

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
    original->alloc = &failing_alloc;

    // Set to fail on copy creation
    set_alloc_fail_countdown(0);

    ANVQueue* copy = anv_queue_copy(original);
    ASSERT_NULL(copy);

    // Restore original allocator for cleanup
    original->alloc = &std_alloc;
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

typedef struct
{
    int (*func)(void);
    const char* name;
} TestCase;

int main(void)
{
    const TestCase tests[] = {
        {test_queue_failing_allocator, "test_queue_failing_allocator"},
        {test_queue_enqueue_memory_failure, "test_queue_enqueue_memory_failure"},
        {test_queue_copy_memory_failure, "test_queue_copy_memory_failure"},
        {test_queue_deep_copy_failure, "test_queue_deep_copy_failure"},
        {test_queue_large_memory_usage, "test_queue_large_memory_usage"},
        {test_queue_clear_memory, "test_queue_clear_memory"},
        {test_queue_iterator_memory_failure, "test_queue_iterator_memory_failure"},
        {test_queue_front_back_consistency, "test_queue_front_back_consistency"},
    };

    printf("Running Queue memory tests...\n");

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
        printf("All Queue memory tests passed!\n");
        return 0;
    }

    printf("%d Queue memory tests failed.\n", failed);
    return 1;
}