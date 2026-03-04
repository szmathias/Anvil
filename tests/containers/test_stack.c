#include <stdio.h>
#include <stdlib.h>

#include <anvil/testing.h>
#include "TestHelpers.h"
#include "containers/arraylist.h"
#include "containers/stack.h"

//==============================================================================
// CRUD Tests
//==============================================================================

// Test basic stack creation and destruction
int test_stack_create_destroy(void)
{
    ANVAllocator alloc = create_int_allocator();

    ANVStack* stack = anv_stack_create(&alloc);
    ASSERT_NOT_NULL(stack);
    ASSERT_EQ(anv_stack_size(stack), 0);
    ASSERT(anv_stack_is_empty(stack));

    anv_stack_destroy(stack, false);
    return TEST_SUCCESS;
}

// Test NULL parameter handling
int test_stack_null_parameters(void)
{
    // Creating with NULL allocator should fail
    ASSERT_NULL(anv_stack_create(NULL));

    // Operations on NULL stack should be safe
    ASSERT_EQ(anv_stack_size(NULL), 0);
    ASSERT(anv_stack_is_empty(NULL));
    ASSERT_NULL(anv_stack_peek(NULL));
    ASSERT_NULL(anv_stack_top(NULL));
    ASSERT_EQ(anv_stack_push(NULL, NULL), -1);
    ASSERT_EQ(anv_stack_pop(NULL, false), -1);
    ASSERT_NULL(anv_stack_pop_data(NULL));

    // Destruction should be safe
    anv_stack_destroy(NULL, false);
    anv_stack_clear(NULL, false);

    return TEST_SUCCESS;
}

// Test basic push and pop operations
int test_stack_push_pop(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVStack* stack = anv_stack_create(&alloc);

    MAKE_INT(data1, 10);
    MAKE_INT(data2, 20);
    MAKE_INT(data3, 30);

    // Test pushing elements
    ASSERT_EQ(anv_stack_push(stack, data1), 0);
    ASSERT_EQ(anv_stack_size(stack), 1);
    ASSERT(!anv_stack_is_empty(stack));
    ASSERT_EQ(*(int*)anv_stack_peek(stack), 10);

    ASSERT_EQ(anv_stack_push(stack, data2), 0);
    ASSERT_EQ(anv_stack_size(stack), 2);
    ASSERT_EQ(*(int*)anv_stack_peek(stack), 20); // LIFO - should see data2

    ASSERT_EQ(anv_stack_push(stack, data3), 0);
    ASSERT_EQ(anv_stack_size(stack), 3);
    ASSERT_EQ(*(int*)anv_stack_peek(stack), 30); // LIFO - should see data3

    // Test popping elements
    ASSERT_EQ(anv_stack_pop(stack, true), 0); // Pops data3 and frees it
    ASSERT_EQ(anv_stack_size(stack), 2);
    ASSERT_EQ(*(int*)anv_stack_peek(stack), 20); // Should see data2

    ASSERT_EQ(anv_stack_pop(stack, true), 0); // Pops data2 and frees it
    ASSERT_EQ(anv_stack_size(stack), 1);
    ASSERT_EQ(*(int*)anv_stack_peek(stack), 10); // Should see data1

    ASSERT_EQ(anv_stack_pop(stack, true), 0); // Pops data1 and frees it
    ASSERT_EQ(anv_stack_size(stack), 0);
    ASSERT(anv_stack_is_empty(stack));
    ASSERT_NULL(anv_stack_peek(stack));

    // Test popping from empty stack
    ASSERT_EQ(anv_stack_pop(stack, false), -1);

    anv_stack_destroy(stack, false);
    return TEST_SUCCESS;
}

int test_stack_pop_data(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVStack* stack = anv_stack_create(&alloc);

    MAKE_INT(data1, 42);
    MAKE_INT(data2, 84);

    ASSERT_EQ(anv_stack_push(stack, data1), 0);
    ASSERT_EQ(anv_stack_push(stack, data2), 0);

    // Pop data2 and get its value
    void* popped = anv_stack_pop_data(stack);
    ASSERT_NOT_NULL(popped);
    ASSERT_EQ(*(int*)popped, 84);
    ASSERT_EQ(anv_stack_size(stack), 1);
    free(popped); // Caller's responsibility to free

    // Pop data1 and get its value
    popped = anv_stack_pop_data(stack);
    ASSERT_NOT_NULL(popped);
    ASSERT_EQ(*(int*)popped, 42);
    ASSERT_EQ(anv_stack_size(stack), 0);
    free(popped);

    // Pop from empty stack
    ASSERT_NULL(anv_stack_pop_data(stack));

    anv_stack_destroy(stack, false);
    return TEST_SUCCESS;
}

// Test stack clear operation
int test_stack_clear(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVStack* stack = anv_stack_create(&alloc);

    // Add some elements
    for (int i = 0; i < 5; i++)
    {
        MAKE_INT(data, i * 10);
        ASSERT_EQ(anv_stack_push(stack, data), 0);
    }

    ASSERT_EQ(anv_stack_size(stack), 5);

    // Clear with freeing data
    anv_stack_clear(stack, true);
    ASSERT_EQ(anv_stack_size(stack), 0);
    ASSERT(anv_stack_is_empty(stack));
    ASSERT_NULL(anv_stack_peek(stack));

    // Stack should still be usable after clear
    MAKE_INT(new_data, 999);
    ASSERT_EQ(anv_stack_push(stack, new_data), 0);
    ASSERT_EQ(anv_stack_size(stack), 1);
    ASSERT_EQ(*(int*)anv_stack_peek(stack), 999);

    anv_stack_destroy(stack, true);
    return TEST_SUCCESS;
}

// Test stack equality
int test_stack_equals(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVStack* stack1 = anv_stack_create(&alloc);
    ANVStack* stack2 = anv_stack_create(&alloc);

    // Empty stacks should be equal
    ASSERT_EQ(anv_stack_equals(stack1, stack2, int_cmp), 1);

    // Same stack should be equal to itself
    ASSERT_EQ(anv_stack_equals(stack1, stack1, int_cmp), 1);

    // Add same elements to both stacks
    for (int i = 0; i < 3; i++)
    {
        int* data1 = malloc(sizeof(int));
        int* data2 = malloc(sizeof(int));
        *data1 = *data2 = i * 10;
        ASSERT_EQ(anv_stack_push(stack1, data1), 0);
        ASSERT_EQ(anv_stack_push(stack2, data2), 0);
    }

    ASSERT_EQ(anv_stack_equals(stack1, stack2, int_cmp), 1);

    // Add different element to one stack
    MAKE_INT(diff_data, 999);
    ASSERT_EQ(anv_stack_push(stack1, diff_data), 0);

    ASSERT_EQ(anv_stack_equals(stack1, stack2, int_cmp), 0);

    // Test with NULL parameters
    ASSERT_EQ(anv_stack_equals(NULL, stack2, int_cmp), -1);
    ASSERT_EQ(anv_stack_equals(stack1, NULL, int_cmp), -1);
    ASSERT_EQ(anv_stack_equals(stack1, stack2, NULL), -1);

    anv_stack_destroy(stack1, true);
    anv_stack_destroy(stack2, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Iterator Tests
//==============================================================================

// Test stack with iterator
int test_stack_iterator(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVStack* stack = anv_stack_create(&alloc);

    // Add some test data
    for (int i = 0; i < 5; i++)
    {
        const int values[] = {10, 20, 30, 40, 50};
        MAKE_INT(data, values[i]);
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
    ANVIterator range_it = anv_iterator_range(&alloc, 0, 5, 1);

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
        MAKE_INT(data, i * 10);
        ASSERT_EQ(anv_stack_push(stack, data), 0);
    }

    ANVIterator it = anv_stack_iterator(stack);
    ASSERT(it.is_valid(&it));

    // Get first element
    void* first = it.get(&it);
    ASSERT_EQ(*(int*)first, 20); // Should be top element (2*10)
    it.next(&it);

    // Modify stack while iterator exists (implementation detail: iterator may become invalid)
    MAKE_INT(new_data, 999);
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

    ANVIterator range_it = anv_iterator_range(&alloc, 0, 3, 1);
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
    ANVIterator range_it = anv_iterator_range(&alloc, 0, 3, 1);
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
    ANVIterator range_it = anv_iterator_range(&alloc, 0, 5, 1);
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
    MAKE_INT(data, 42);
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
        MAKE_INT(data, i * 10);
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

//==============================================================================
// Algorithm Tests
//==============================================================================

// Test stack copying (shallow)
int test_stack_copy_shallow(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVStack* original = anv_stack_create(&alloc);

    // Add some test data
    const int original_values[] = {10, 20, 30, 40, 50};
    int* data_ptrs[5];

    for (int i = 0; i < 5; i++)
    {
        data_ptrs[i] = malloc(sizeof(int));
        *data_ptrs[i] = original_values[i];
        ASSERT_EQ(anv_stack_push(original, data_ptrs[i]), 0);
    }

    // Create shallow copy
    ANVStack* copy = anv_stack_copy(original);
    ASSERT_NOT_NULL(copy);
    ASSERT_EQ(anv_stack_size(copy), 5);
    ASSERT_EQ(anv_stack_equals(original, copy, int_cmp), 1);

    // Verify data is shared (same pointers)
    for (int i = 4; i >= 0; i--)
    {
        // LIFO order
        void* orig_data = anv_stack_pop_data(original);
        void* copy_data = anv_stack_pop_data(copy);
        ASSERT_EQ_PTR(orig_data, copy_data); // Should be same pointer
        ASSERT_EQ(*(int*)orig_data, original_values[i]);
        // Don't free - they're the same pointer
    }

    // Free the shared data once
    for (int i = 0; i < 5; i++)
    {
        free(data_ptrs[i]);
    }

    anv_stack_destroy(original, false);
    anv_stack_destroy(copy, false);
    return TEST_SUCCESS;
}

// Test stack copying (deep)
int test_stack_copy_deep(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVStack* original = anv_stack_create(&alloc);

    // Add some test data
    const int original_values[] = {10, 20, 30};

    for (int i = 0; i < 3; i++)
    {
        MAKE_INT(data, original_values[i]);
        ASSERT_EQ(anv_stack_push(original, data), 0);
    }

    // Create deep copy
    ANVStack* copy = anv_stack_copy_deep(original, false);
    ASSERT_NOT_NULL(copy);
    ASSERT_EQ(anv_stack_size(copy), 3);
    ASSERT_EQ(anv_stack_equals(original, copy, int_cmp), 1);

    // Verify data is different (different pointers, same values)
    for (int i = 2; i >= 0; i--)
    {
        // LIFO order
        void* orig_data = anv_stack_pop_data(original);
        void* copy_data = anv_stack_pop_data(copy);
        ASSERT_NOT_EQ_PTR(orig_data, copy_data);       // Should be different pointers
        ASSERT_EQ(*(int*)orig_data, *(int*)copy_data); // Same values
        ASSERT_EQ(*(int*)orig_data, original_values[i]);
        free(orig_data);
        free(copy_data);
    }

    anv_stack_destroy(original, false);
    anv_stack_destroy(copy, false);
    return TEST_SUCCESS;
}

// Test for_each functionality
int test_stack_for_each(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVStack* stack = anv_stack_create(&alloc);

    // Add some test data
    for (int i = 1; i <= 5; i++)
    {
        MAKE_INT(data, i * 10);
        ASSERT_EQ(anv_stack_push(stack, data), 0);
    }

    // Use increment action to modify all elements
    anv_stack_for_each(stack, increment);

    // Verify elements were incremented (should be 51, 41, 31, 21, 11 in LIFO order)
    for (int i = 0; i < 5; i++)
    {
        const int expected[] = {51, 41, 31, 21, 11};
        void* data = anv_stack_pop_data(stack);
        ASSERT_EQ(*(int*)data, expected[i]);
        free(data);
    }

    // Test with NULL parameters
    anv_stack_for_each(NULL, increment); // Should be safe
    anv_stack_for_each(stack, NULL);     // Should be safe

    anv_stack_destroy(stack, false);
    return TEST_SUCCESS;
}

// Test stack with Person objects
int test_stack_with_persons(void)
{
    ANVAllocator alloc = create_person_allocator();
    ANVStack* stack = anv_stack_create(&alloc);

    // Create and push some persons
    Person* alice = create_person("Alice", 25);
    Person* bob = create_person("Bob", 30);
    Person* charlie = create_person("Charlie", 35);

    ASSERT_EQ(anv_stack_push(stack, alice), 0);
    ASSERT_EQ(anv_stack_push(stack, bob), 0);
    ASSERT_EQ(anv_stack_push(stack, charlie), 0);

    // Peek at top (should be Charlie)
    Person* top = (Person*)anv_stack_peek(stack);
    ASSERT_NOT_NULL(top);
    ASSERT_EQ_STR(top->name, "Charlie");
    ASSERT_EQ(top->age, 35);

    // Test deep copy
    ANVStack* copy = anv_stack_copy_deep(stack, false);
    ASSERT_NOT_NULL(copy);
    ASSERT_EQ(anv_stack_equals(stack, copy, person_cmp), 1);

    // Verify persons are in correct LIFO order
    for (int i = 0; i < 3; i++)
    {
        const int expected_ages[] = {35, 30, 25};
        const char* expected_names[] = {"Charlie", "Bob", "Alice"};
        Person* person = (Person*)anv_stack_pop_data(copy);
        ASSERT_NOT_NULL(person);
        ASSERT_EQ_STR(person->name, expected_names[i]);
        ASSERT_EQ(person->age, expected_ages[i]);
        free(person);
    }

    anv_stack_destroy(stack, true);
    anv_stack_destroy(copy, false);
    return TEST_SUCCESS;
}

//==============================================================================
// Memory Tests
//==============================================================================

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

    MAKE_INT(data, 42);

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
        MAKE_INT(data, i * 10);
        ASSERT_EQ(anv_stack_push(original, data), 0);
    }

    // Replace allocator with failing one
    const ANVAllocator failing_alloc = create_failing_int_allocator();
    original->alloc = failing_alloc;

    // Set to fail on copy creation
    set_alloc_fail_countdown(0);

    ANVStack* copy = anv_stack_copy(original);
    ASSERT_NULL(copy);

    // Restore original allocator for cleanup
    original->alloc = std_alloc;
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
        MAKE_INT(data, i * 10);
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
        MAKE_INT(data, i);
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
            MAKE_INT(data, i);
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
    MAKE_INT(data, 42);
    ASSERT_EQ(anv_stack_push(stack, data), 0);

    // Set to fail on iterator state allocation
    set_alloc_fail_countdown(0);

    const ANVIterator it = anv_stack_iterator(stack);
    ASSERT(!it.is_valid(&it));

    anv_stack_destroy(stack, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Property Tests
//==============================================================================

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
        MAKE_INT(data, i);
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

    MAKE_INT(data1, 10);
    MAKE_INT(data2, 20);
    MAKE_INT(data3, 30);

    ASSERT_EQ(anv_stack_push(stack, data1), 0);
    ASSERT_EQ(anv_stack_push(stack, data2), 0);
    ASSERT_EQ(anv_stack_push(stack, data3), 0);

    const size_t original_size = anv_stack_size(stack);

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
        MAKE_INT(data, values[i]);
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
        MAKE_INT(data, i);
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
    MAKE_INT(new_data, 999);
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
        MAKE_INT(data, original_values[i]);
        ASSERT_EQ(anv_stack_push(stack, data), 0);
    }

    const size_t original_size = anv_stack_size(stack);

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

//==============================================================================
// Fuzz Tests
//==============================================================================

int test_stack_fuzz(void)
{
    srand((unsigned int)42);
    ANVAllocator alloc = create_int_allocator();
    ANVStack* stack = anv_stack_create(&alloc);
    ASSERT_NOT_NULL(stack);

    size_t expected_size = 0;

    for (int i = 0; i < 50000; i++)
    {
        const unsigned op = rand() % 3;

        switch (op)
        {
            case 0: // push
                {
                    MAKE_INT(val, rand());
                    if (anv_stack_push(stack, val) == 0)
                        expected_size++;
                    else
                        free(val);
                    break;
                }
            case 1: // pop
                {
                    if (expected_size > 0)
                    {
                        if (anv_stack_pop(stack, true) == 0)
                            expected_size--;
                    }
                    break;
                }
            case 2: // peek
                {
                    if (expected_size > 0)
                    {
                        void* top = anv_stack_peek(stack);
                        ASSERT_NOT_NULL(top);
                    }
                    else
                    {
                        ASSERT_NULL(anv_stack_peek(stack));
                    }
                    break;
                }
            default:
                break;
        }

        ASSERT_EQ(anv_stack_size(stack), expected_size);
        ASSERT_EQ(anv_stack_is_empty(stack), expected_size == 0 ? 1 : 0);
    }

    anv_stack_destroy(stack, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Main
//==============================================================================

int main(void)
{
    const ANVTestCase tests[] = {
        // CRUD Tests
        TEST_REGISTER(test_stack_create_destroy),
        TEST_REGISTER(test_stack_null_parameters),
        TEST_REGISTER(test_stack_push_pop),
        TEST_REGISTER(test_stack_pop_data),
        TEST_REGISTER(test_stack_clear),
        TEST_REGISTER(test_stack_equals),

        // Iterator Tests
        TEST_REGISTER(test_stack_iterator),
        TEST_REGISTER(test_stack_from_iterator),
        TEST_REGISTER(test_stack_iterator_empty),
        TEST_REGISTER(test_stack_iterator_invalid),
        TEST_REGISTER(test_stack_iterator_modification),
        TEST_REGISTER(test_stack_copy_isolation),
        TEST_REGISTER(test_stack_anv_copy_function_required),
        TEST_REGISTER(test_stack_from_iterator_no_copy),
        TEST_REGISTER(test_iterator_exhaustion_after_stack_creation),
        TEST_REGISTER(test_stack_iterator_next_return_values),
        TEST_REGISTER(test_stack_iterator_mixed_operations),

        // Algorithm Tests
        TEST_REGISTER(test_stack_copy_shallow),
        TEST_REGISTER(test_stack_copy_deep),
        TEST_REGISTER(test_stack_for_each),
        TEST_REGISTER(test_stack_with_persons),

        // Memory Tests
        TEST_REGISTER(test_stack_failing_allocator),
        TEST_REGISTER(test_stack_push_memory_failure),
        TEST_REGISTER(test_stack_copy_memory_failure),
        TEST_REGISTER(test_stack_deep_copy_failure),
        TEST_REGISTER(test_stack_large_memory_usage),
        TEST_REGISTER(test_stack_clear_memory),
        TEST_REGISTER(test_stack_iterator_memory_failure),

        // Property Tests
        TEST_REGISTER(test_stack_lifo_property),
        TEST_REGISTER(test_stack_size_consistency),
        TEST_REGISTER(test_stack_peek_invariant),
        TEST_REGISTER(test_stack_copy_preserves_order),
        TEST_REGISTER(test_stack_clear_preserves_structure),
        TEST_REGISTER(test_stack_for_each_preserves_contents),

        // Fuzz Tests
        TEST_REGISTER(test_stack_fuzz),
    };

    return anv_run_tests("Stack", tests, sizeof(tests) / sizeof(tests[0]));
}