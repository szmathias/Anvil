//
// Created by zack on 9/9/25.
//

#include "TestAssert.h"
#include "TestHelpers.h"
#include "containers/stack.h"
#include <stdio.h>
#include <stdlib.h>

// Test LIFO property extensively
int test_stack_lifo_property(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVStack* stack = anv_stack_create(&alloc);

    #define num_elements 100
    int* values[num_elements];

    // Push elements in order
    for (int i = 0; i < num_elements; i++)
    {
        values[i] = malloc(sizeof(int));
        *values[i] = i * 7; // Use non-sequential values
        ASSERT_EQ(anv_stack_push(stack, values[i]), 0);
    }

    // Pop elements - should come out in reverse order (LIFO)
    for (int i = num_elements - 1; i >= 0; i--)
    {
        void* data = anv_stack_pop_data(stack);
        ASSERT_NOT_NULL(data);
        ASSERT_EQ_PTR(data, values[i]); // Should be exact same pointer
        ASSERT_EQ(*(int*)data, i * 7);
        free(data);
    }

    ASSERT(anv_stack_is_empty(stack));

    anv_stack_destroy(stack, false);
    return TEST_SUCCESS;
}

// Test stack size consistency
int test_stack_size_consistency(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVStack* stack = anv_stack_create(&alloc);

    // Size should start at 0
    ASSERT_EQ(anv_stack_size(stack), 0);
    ASSERT(anv_stack_is_empty(stack));

    // Size should increase with each push
    for (int i = 1; i <= 50; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i;
        ASSERT_EQ(anv_stack_push(stack, data), 0);
        ASSERT_EQ(anv_stack_size(stack), (size_t)i);
        ASSERT(!anv_stack_is_empty(stack));
    }

    // Size should decrease with each pop
    for (int i = 49; i >= 0; i--)
    {
        ASSERT_EQ(anv_stack_pop(stack, true), 0);
        ASSERT_EQ(anv_stack_size(stack), (size_t)i);

        if (i == 0)
        {
            ASSERT(anv_stack_is_empty(stack));
        }
        else
        {
            ASSERT(!anv_stack_is_empty(stack));
        }
    }

    anv_stack_destroy(stack, false);
    return TEST_SUCCESS;
}

// Test peek invariant (peek doesn't modify stack)
int test_stack_peek_invariant(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVStack* stack = anv_stack_create(&alloc);

    int* data1 = malloc(sizeof(int));
    int* data2 = malloc(sizeof(int));
    int* data3 = malloc(sizeof(int));
    *data1 = 10;
    *data2 = 20;
    *data3 = 30;

    ASSERT_EQ(anv_stack_push(stack, data1), 0);
    ASSERT_EQ(anv_stack_push(stack, data2), 0);
    ASSERT_EQ(anv_stack_push(stack, data3), 0);

    size_t original_size = anv_stack_size(stack);

    // Multiple peeks should return same value and not change size
    for (int i = 0; i < 10; i++)
    {
        void* peeked = anv_stack_peek(stack);
        ASSERT_NOT_NULL(peeked);
        ASSERT_EQ(*(int*)peeked, 30);
        ASSERT_EQ(anv_stack_size(stack), original_size);

        // Also test top() alias
        void* top = anv_stack_top(stack);
        ASSERT_EQ_PTR(top, peeked);
    }

    anv_stack_destroy(stack, true);
    return TEST_SUCCESS;
}

// Test copy preserves order
int test_stack_copy_preserves_order(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVStack* original = anv_stack_create(&alloc);

    const int values[] = {1, 3, 5, 7, 9, 11, 13};
    const int num_values = sizeof(values) / sizeof(values[0]);

    // Build original stack
    for (int i = 0; i < num_values; i++)
    {
        int* data = malloc(sizeof(int));
        *data = values[i];
        ASSERT_EQ(anv_stack_push(original, data), 0);
    }

    // Create shallow copy
    ANVStack* shallow_copy = anv_stack_copy(original);
    ASSERT_NOT_NULL(shallow_copy);

    // Create deep copy
    ANVStack* deep_copy = anv_stack_copy_deep(original, false);
    ASSERT_NOT_NULL(deep_copy);

    // All three stacks should have same size and equal contents
    ASSERT_EQ(anv_stack_size(original), (size_t)num_values);
    ASSERT_EQ(anv_stack_size(shallow_copy), (size_t)num_values);
    ASSERT_EQ(anv_stack_size(deep_copy), (size_t)num_values);

    ASSERT_EQ(anv_stack_equals(original, shallow_copy, int_cmp), 1);
    ASSERT_EQ(anv_stack_equals(original, deep_copy, int_cmp), 1);

    // Pop from all three - should get same sequence
    for (int i = num_values - 1; i >= 0; i--)
    {
        void* orig_data = anv_stack_pop_data(original);
        void* shallow_data = anv_stack_pop_data(shallow_copy);
        void* deep_data = anv_stack_pop_data(deep_copy);

        ASSERT_EQ(*(int*)orig_data, values[i]);
        ASSERT_EQ(*(int*)shallow_data, values[i]);
        ASSERT_EQ(*(int*)deep_data, values[i]);

        // Shallow copy shares pointers, deep copy doesn't
        ASSERT_EQ_PTR(orig_data, shallow_data);
        ASSERT_NOT_EQ_PTR(orig_data, deep_data);

        free(orig_data); // Also frees shallow_data
        free(deep_data);
    }

    anv_stack_destroy(original, false);
    anv_stack_destroy(shallow_copy, false);
    anv_stack_destroy(deep_copy, false);
    return TEST_SUCCESS;
}

// Test clear preserves stack structure
int test_stack_clear_preserves_structure(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVStack* stack = anv_stack_create(&alloc);

    // Add elements
    for (int i = 0; i < 10; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i;
        ASSERT_EQ(anv_stack_push(stack, data), 0);
    }

    ASSERT_EQ(anv_stack_size(stack), 10);

    // Clear stack
    anv_stack_clear(stack, true);

    // Stack should be empty but still functional
    ASSERT_EQ(anv_stack_size(stack), 0);
    ASSERT(anv_stack_is_empty(stack));
    ASSERT_NULL(anv_stack_peek(stack));

    // Should be able to use stack normally after clear
    int* new_data = malloc(sizeof(int));
    *new_data = 999;
    ASSERT_EQ(anv_stack_push(stack, new_data), 0);
    ASSERT_EQ(anv_stack_size(stack), 1);
    ASSERT_EQ(*(int*)anv_stack_peek(stack), 999);

    anv_stack_destroy(stack, true);
    return TEST_SUCCESS;
}

// Test for_each preserves stack contents
int test_stack_for_each_preserves_contents(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVStack* stack = anv_stack_create(&alloc);

    const int original_values[] = {5, 10, 15, 20, 25};
    const int num_values = sizeof(original_values) / sizeof(original_values[0]);

    // Build stack
    for (int i = 0; i < num_values; i++)
    {
        int* data = malloc(sizeof(int));
        *data = original_values[i];
        ASSERT_EQ(anv_stack_push(stack, data), 0);
    }

    size_t original_size = anv_stack_size(stack);

    // Apply for_each (increment each element)
    anv_stack_for_each(stack, increment);

    // Stack size should be unchanged
    ASSERT_EQ(anv_stack_size(stack), original_size);

    // Elements should be modified but order preserved (LIFO)
    for (int i = num_values - 1; i >= 0; i--)
    {
        void* data = anv_stack_pop_data(stack);
        ASSERT_NOT_NULL(data);
        ASSERT_EQ(*(int*)data, original_values[i] + 1); // Should be incremented
        free(data);
    }

    anv_stack_destroy(stack, false);
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
        {test_stack_lifo_property, "test_stack_lifo_property"},
        {test_stack_size_consistency, "test_stack_size_consistency"},
        {test_stack_peek_invariant, "test_stack_peek_invariant"},
        {test_stack_copy_preserves_order, "test_stack_copy_preserves_order"},
        {test_stack_clear_preserves_structure, "test_stack_clear_preserves_structure"},
        {test_stack_for_each_preserves_contents, "test_stack_for_each_preserves_contents"},
    };

    printf("Running Stack property tests...\n");

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
        printf("All Stack property tests passed!\n");
        return 0;
    }

    printf("%d Stack property tests failed.\n", failed);
    return 1;
}