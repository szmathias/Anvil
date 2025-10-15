//
// Created by zack on 9/9/25.
//

#include "TestAssert.h"
#include "TestHelpers.h"
#include "containers/stack.h"
#include <stdio.h>
#include <stdlib.h>

// Test stack with failing allocator
int test_stack_failing_allocator(void)
{
    ANVAllocator failing_alloc = create_failing_int_allocator();

    // Set to fail immediately
    set_alloc_fail_countdown(0);

    // Stack creation should fail
    ANVStack* stack = anv_stack_create(&failing_alloc);
    ASSERT_NULL(stack);

    return TEST_SUCCESS;
}

// Test push with failing allocator
int test_stack_push_memory_failure(void)
{
    ANVAllocator failing_alloc = create_failing_int_allocator();

    // Allow stack creation but fail on first push
    set_alloc_fail_countdown(1);

    ANVStack* stack = anv_stack_create(&failing_alloc);
    ASSERT_NOT_NULL(stack);

    int* data = malloc(sizeof(int));
    *data = 42;

    // Push should fail due to node allocation failure
    ASSERT_EQ(anv_stack_push(stack, data), -1);
    ASSERT_EQ(anv_stack_size(stack), 0);

    free(data);
    anv_stack_destroy(stack, false);
    return TEST_SUCCESS;
}

// Test copy with failing allocator
int test_stack_copy_memory_failure(void)
{
    ANVAllocator std_alloc = create_int_allocator();
    ANVStack* original = anv_stack_create(&std_alloc);

    // Add some data
    for (int i = 0; i < 3; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i * 10;
        ASSERT_EQ(anv_stack_push(original, data), 0);
    }

    // Replace allocator with failing one
    ANVAllocator failing_alloc = create_failing_int_allocator();
    original->alloc = &failing_alloc;

    // Set to fail on copy creation
    set_alloc_fail_countdown(0);

    ANVStack* copy = anv_stack_copy(original);
    ASSERT_NULL(copy);

    // Restore original allocator for cleanup
    original->alloc = &std_alloc;
    anv_stack_destroy(original, true);
    return TEST_SUCCESS;
}

// Test deep copy with failing copy function
int test_stack_deep_copy_failure(void)
{
    set_alloc_fail_countdown(-1);
    ANVAllocator failing_alloc = create_failing_int_allocator();
    ANVStack* original = anv_stack_create(&failing_alloc);

    // Add some data
    for (int i = 0; i < 3; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i * 10;
        ASSERT_EQ(anv_stack_push(original, data), 0);
    }

    // Set to fail on copy function calls
    set_alloc_fail_countdown(2); // Allow stack creation, fail on first copy

    ANVStack* copy = anv_stack_copy_deep(original, true);
    ASSERT_NULL(copy);

    anv_stack_destroy(original, true);
    return TEST_SUCCESS;
}

// Test memory usage with large number of elements
int test_stack_large_memory_usage(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVStack* stack = anv_stack_create(&alloc);

    const int num_elements = 10000;

    // Push many elements
    for (int i = 0; i < num_elements; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i;
        ASSERT_EQ(anv_stack_push(stack, data), 0);
    }

    ASSERT_EQ(anv_stack_size(stack), (size_t)num_elements);

    // Pop all elements in LIFO order
    for (int i = num_elements - 1; i >= 0; i--)
    {
        void* data = anv_stack_pop_data(stack);
        ASSERT_NOT_NULL(data);
        ASSERT_EQ(*(int*)data, i);
        free(data);
    }

    ASSERT(anv_stack_is_empty(stack));

    anv_stack_destroy(stack, false);
    return TEST_SUCCESS;
}

// Test memory leaks with clear operations
int test_stack_clear_memory(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVStack* stack = anv_stack_create(&alloc);

    // Add elements multiple times and clear
    for (int cycle = 0; cycle < 5; cycle++)
    {
        // Add elements
        for (int i = 0; i < 100; i++)
        {
            int* data = malloc(sizeof(int));
            *data = i;
            ASSERT_EQ(anv_stack_push(stack, data), 0);
        }

        ASSERT_EQ(anv_stack_size(stack), 100);

        // Clear with memory cleanup
        anv_stack_clear(stack, true);
        ASSERT_EQ(anv_stack_size(stack), 0);
        ASSERT(anv_stack_is_empty(stack));
    }

    anv_stack_destroy(stack, false);
    return TEST_SUCCESS;
}

// Test iterator memory with failing allocator
int test_stack_iterator_memory_failure(void)
{
    set_alloc_fail_countdown(-1);
    ANVAllocator failing_alloc = create_failing_int_allocator();
    ANVStack* stack = anv_stack_create(&failing_alloc);

    // Add some data
    int* data = malloc(sizeof(int));
    *data = 42;
    ASSERT_EQ(anv_stack_push(stack, data), 0);

    // Set to fail on iterator state allocation
    set_alloc_fail_countdown(0);

    const ANVIterator it = anv_stack_iterator(stack);
    ASSERT(!it.is_valid(&it));

    anv_stack_destroy(stack, true);
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
        {test_stack_failing_allocator, "test_stack_failing_allocator"},
        {test_stack_push_memory_failure, "test_stack_push_memory_failure"},
        {test_stack_copy_memory_failure, "test_stack_copy_memory_failure"},
        {test_stack_deep_copy_failure, "test_stack_deep_copy_failure"},
        {test_stack_large_memory_usage, "test_stack_large_memory_usage"},
        {test_stack_clear_memory, "test_stack_clear_memory"},
        {test_stack_iterator_memory_failure, "test_stack_iterator_memory_failure"},
    };

    printf("Running Stack memory tests...\n");

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
        printf("All Stack memory tests passed!\n");
        return 0;
    }

    printf("%d Stack memory tests failed.\n", failed);
    return 1;
}