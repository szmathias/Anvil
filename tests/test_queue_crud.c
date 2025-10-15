#include "TestAssert.h"
#include "TestHelpers.h"
#include "containers/queue.h"
#include <stdio.h>
#include <stdlib.h>

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

typedef struct
{
    int (*func)(void);
    const char* name;
} TestCase;

int main(void)
{
    const TestCase tests[] = {
        {test_queue_create_destroy, "test_queue_create_destroy"},
        {test_queue_null_parameters, "test_queue_null_parameters"},
        {test_queue_enqueue_dequeue, "test_queue_enqueue_dequeue"},
        {test_queue_dequeue_data, "test_queue_dequeue_data"},
        {test_queue_clear, "test_queue_clear"},
        {test_queue_equals, "test_queue_equals"},
        {test_queue_fifo_behavior, "test_queue_fifo_behavior"},
    };

    printf("Running Queue CRUD tests...\n");

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
        printf("All Queue CRUD tests passed!\n");
        return 0;
    }

    printf("%d Queue CRUD tests failed.\n", failed);
    return 1;
}