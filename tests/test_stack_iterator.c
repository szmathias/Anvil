//
// Created by zack on 9/9/25.
//

#include "TestAssert.h"
#include "TestHelpers.h"
#include "containers/stack.h"
#include "containers/arraylist.h"
#include <stdio.h>
#include <stdlib.h>

// Test stack with iterator
int test_stack_iterator(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVStack* stack = anv_stack_create(&alloc);

    // Add some test data
    for (int i = 0; i < 5; i++)
    {
        const int values[] = {10, 20, 30, 40, 50};
        int* data = malloc(sizeof(int));
        *data = values[i];
        ASSERT_EQ(anv_stack_push(stack, data), 0);
    }

    // Create iterator
    ANVIterator it = anv_stack_iterator(stack);
    ASSERT(it.is_valid(&it));

    // Iterate through stack (should be in LIFO order: 50, 40, 30, 20, 10)
    int index = 0;
    const int expected[] = {50, 40, 30, 20, 10};

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
    ASSERT_EQ(*(int*)first, 50); // Should be top element again

    // Test get without advancing
    it.reset(&it);
    void* peek_data = it.get(&it);
    ASSERT_EQ(*(int*)peek_data, 50);
    ASSERT(it.has_next(&it)); // Should still have next

    it.destroy(&it);
    anv_stack_destroy(stack, true);
    return TEST_SUCCESS;
}

// Test creating stack from iterator
int test_stack_from_iterator(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create a range iterator (0, 1, 2, 3, 4)
    ANVIterator range_it = anv_iterator_range(0, 5, 1, &alloc);

    // Create stack from iterator
    ANVStack* stack = anv_stack_from_iterator(&range_it, &alloc, true);
    ASSERT_NOT_NULL(stack);
    ASSERT_EQ(anv_stack_size(stack), 5);

    // Clean up the iterator immediately after use
    range_it.destroy(&range_it);

    // Verify stack has correct values in LIFO order
    // Iterator gives 0,1,2,3,4 but stack should have them as 4,3,2,1,0 (top to bottom)
    for (int expected = 4; expected >= 0; expected--)
    {
        void* data = anv_stack_pop_data(stack);
        ASSERT_NOT_NULL(data);
        ASSERT_EQ(*(int*)data, expected);
        free(data);
    }

    anv_stack_destroy(stack, false);
    return TEST_SUCCESS;
}

// Test iterator with empty stack
int test_stack_iterator_empty(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVStack* stack = anv_stack_create(&alloc);

    ANVIterator it = anv_stack_iterator(stack);
    ASSERT(it.is_valid(&it));
    ASSERT(!it.has_next(&it));
    ASSERT_NULL(it.get(&it));
    ASSERT_EQ(it.next(&it), -1); // Should return error code

    it.destroy(&it);
    anv_stack_destroy(stack, false);
    return TEST_SUCCESS;
}

// Test iterator validity with invalid stack
int test_stack_iterator_invalid(void)
{
    const ANVIterator it = anv_stack_iterator(NULL);
    ASSERT(!it.is_valid(&it));
    return TEST_SUCCESS;
}

// Test iterator state after stack modifications
int test_stack_iterator_modification(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVStack* stack = anv_stack_create(&alloc);

    // Add initial data
    for (int i = 0; i < 3; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i * 10;
        ASSERT_EQ(anv_stack_push(stack, data), 0);
    }

    ANVIterator it = anv_stack_iterator(stack);
    ASSERT(it.is_valid(&it));

    // Get first element
    void* first = it.get(&it);
    ASSERT_EQ(*(int*)first, 20); // Should be top element (2*10)
    it.next(&it);

    // Modify stack while iterator exists (implementation detail: iterator may become invalid)
    int* new_data = malloc(sizeof(int));
    *new_data = 999;
    ASSERT_EQ(anv_stack_push(stack, new_data), 0);

    // Iterator should still be valid but may not reflect new state
    ASSERT(it.is_valid(&it));

    it.destroy(&it);
    anv_stack_destroy(stack, true);
    return TEST_SUCCESS;
}

// Test copy isolation - verify that copied elements are independent
int test_stack_copy_isolation(void)
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

    // Create stack with copying enabled
    ANVStack* stack = anv_stack_from_iterator(&list_it, &alloc, true);
    ASSERT_NOT_NULL(stack);
    ASSERT_EQ(anv_stack_size(stack), 3);

    // Modify original data
    *data_ptrs[0] = 999;
    *data_ptrs[1] = 888;
    *data_ptrs[2] = 777;

    // Stack should still have original values (proving data was copied)
    void* stack_data = anv_stack_pop_data(stack);
    ASSERT_NOT_NULL(stack_data);
    ASSERT_EQ(*(int*)stack_data, 30); // Should be unchanged
    free(stack_data);

    stack_data = anv_stack_pop_data(stack);
    ASSERT_NOT_NULL(stack_data);
    ASSERT_EQ(*(int*)stack_data, 20); // Should be unchanged
    free(stack_data);

    stack_data = anv_stack_pop_data(stack);
    ASSERT_NOT_NULL(stack_data);
    ASSERT_EQ(*(int*)stack_data, 10); // Should be unchanged
    free(stack_data);

    // Cleanup
    list_it.destroy(&list_it);
    anv_stack_destroy(stack, false);
    anv_arraylist_destroy(list, true);

    return TEST_SUCCESS;
}

// Test that should_copy=true fails when allocator has no copy function
int test_stack_anv_copy_function_required(void)
{
    ANVAllocator alloc = anv_alloc_default();
    alloc.copy = NULL;

    ANVIterator range_it = anv_iterator_range(0, 3, 1, &alloc);
    ASSERT(range_it.is_valid(&range_it));

    // Should return NULL because should_copy=true but no copy function available
    ANVStack* stack = anv_stack_from_iterator(&range_it, &alloc, true);
    ASSERT_NULL(stack);

    range_it.destroy(&range_it);
    return TEST_SUCCESS;
}

// Test that should_copy=false uses elements directly without copying
int test_stack_from_iterator_no_copy(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create a range iterator and then a copy iterator to get actual owned data
    ANVIterator range_it = anv_iterator_range(0, 3, 1, &alloc);
    ASSERT(range_it.is_valid(&range_it));

    // Use copy iterator to create actual data elements that we own
    ANVIterator copy_it = anv_iterator_copy(&range_it, &alloc, int_copy);
    ASSERT(copy_it.is_valid(&copy_it));

    // Create stack without copying (should_copy = false)
    // This will use the copied elements directly from the copy iterator
    ANVStack* stack = anv_stack_from_iterator(&copy_it, &alloc, false);
    ASSERT_NOT_NULL(stack);
    ASSERT_EQ(anv_stack_size(stack), 3);

    // Verify values are correct (LIFO order: 2, 1, 0)
    void* data = anv_stack_pop_data(stack);
    ASSERT_NOT_NULL(data);
    ASSERT_EQ(*(int*)data, 2);
    free(data); // We own this data from the copy iterator

    data = anv_stack_pop_data(stack);
    ASSERT_NOT_NULL(data);
    ASSERT_EQ(*(int*)data, 1);
    free(data); // We own this data from the copy iterator

    data = anv_stack_pop_data(stack);
    ASSERT_NOT_NULL(data);
    ASSERT_EQ(*(int*)data, 0);
    free(data); // We own this data from the copy iterator

    range_it.destroy(&range_it);
    copy_it.destroy(&copy_it);
    anv_stack_destroy(stack, false); // Don't free elements since we already freed them
    return TEST_SUCCESS;
}

// Test that iterator is exhausted after being consumed by anv_stack_from_iterator
int test_iterator_exhaustion_after_stack_creation(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVIterator range_it = anv_iterator_range(0, 5, 1, &alloc);
    ASSERT(range_it.is_valid(&range_it));

    // Verify iterator starts with elements
    ASSERT(range_it.has_next(&range_it));

    // Create stack from iterator (consumes all elements)
    ANVStack* stack = anv_stack_from_iterator(&range_it, &alloc, true);
    ASSERT_NOT_NULL(stack);
    ASSERT_EQ(anv_stack_size(stack), 5);

    // Iterator should now be exhausted
    ASSERT(!range_it.has_next(&range_it));
    ASSERT_NULL(range_it.get(&range_it));
    ASSERT_EQ(range_it.next(&range_it), -1); // Should fail to advance

    // But iterator should still be valid
    ASSERT(range_it.is_valid(&range_it));

    range_it.destroy(&range_it);
    anv_stack_destroy(stack, true);
    return TEST_SUCCESS;
}

// Test next() return values for proper error handling
int test_stack_iterator_next_return_values(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVStack* stack = anv_stack_create(&alloc);
    ASSERT_NOT_NULL(stack);

    // Add single element
    int* data = malloc(sizeof(int));
    *data = 42;
    ASSERT_EQ(anv_stack_push(stack, data), 0);

    ANVIterator it = anv_stack_iterator(stack);
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
    anv_stack_destroy(stack, true);
    return TEST_SUCCESS;
}

// Test various combinations of get/next/has_next calls for consistency
int test_stack_iterator_mixed_operations(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVStack* stack = anv_stack_create(&alloc);
    ASSERT_NOT_NULL(stack);

    // Add test data (will be in LIFO order: 20, 10, 0)
    for (int i = 0; i < 3; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i * 10;
        ASSERT_EQ(anv_stack_push(stack, data), 0);
    }

    ANVIterator it = anv_stack_iterator(stack);
    ASSERT(it.is_valid(&it));

    // Multiple get() calls should return same value
    void* data1 = it.get(&it);
    void* data2 = it.get(&it);
    ASSERT_NOT_NULL(data1);
    ASSERT_NOT_NULL(data2);
    ASSERT_EQ(data1, data2);               // Same pointer
    ASSERT_EQ(*(int*)data1, *(int*)data2); // Same value
    ASSERT_EQ(*(int*)data1, 20);           // Top element should be 20

    // has_next should be consistent
    ASSERT(it.has_next(&it));
    ASSERT(it.has_next(&it)); // Multiple calls should be safe

    // Advance and verify new position
    ASSERT_EQ(it.next(&it), 0);
    void* data3 = it.get(&it);
    ASSERT_NOT_NULL(data3);
    // Note: data1 and data3 point to different stack elements
    ASSERT_NOT_EQ(*(int*)data1, *(int*)data3); // Different values
    ASSERT_EQ(*(int*)data3, 10);               // Next element should be 10

    // Verify we can still advance
    ASSERT(it.has_next(&it));
    ASSERT_EQ(it.next(&it), 0);

    void* data4 = it.get(&it);
    ASSERT_NOT_NULL(data4);
    ASSERT_EQ(*(int*)data4, 0); // Last element should be 0

    // Now should be at end
    ASSERT_EQ(it.next(&it), 0); // Advance past last element
    ASSERT(!it.has_next(&it));
    ASSERT_NULL(it.get(&it));

    it.destroy(&it);
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
        {test_stack_iterator, "test_stack_iterator"},
        {test_stack_from_iterator, "test_stack_from_iterator"},
        {test_stack_iterator_empty, "test_stack_iterator_empty"},
        {test_stack_iterator_invalid, "test_stack_iterator_invalid"},
        {test_stack_iterator_modification, "test_stack_iterator_modification"},
        {test_stack_copy_isolation, "test_stack_copy_isolation"},
        {test_stack_anv_copy_function_required, "test_stack_anv_copy_function_required"},
        {test_stack_from_iterator_no_copy, "test_stack_from_iterator_no_copy"},
        {test_iterator_exhaustion_after_stack_creation, "test_iterator_exhaustion_after_stack_creation"},
        {test_stack_iterator_next_return_values, "test_stack_iterator_next_return_values"},
        {test_stack_iterator_mixed_operations, "test_stack_iterator_mixed_operations"},
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

    printf("All stack iterator tests passed!\n");
    return 0;
}