#include "TestAssert.h"
#include "TestHelpers.h"
#include "containers/queue.h"
#include <stdio.h>
#include <stdlib.h>

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

typedef struct
{
    int (*func)(void);
    const char* name;
} TestCase;

int main(void)
{
    const TestCase tests[] = {
        {test_queue_copy_shallow, "test_queue_copy_shallow"},
        {test_queue_copy_deep, "test_queue_copy_deep"},
        {test_queue_for_each, "test_queue_for_each"},
        {test_queue_with_persons, "test_queue_with_persons"},
        {test_queue_mixed_operations, "test_queue_mixed_operations"},
    };

    printf("Running Queue algorithm tests...\n");

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
        printf("All Queue algorithm tests passed!\n");
        return 0;
    }

    printf("%d Queue algorithm tests failed.\n", failed);
    return 1;
}