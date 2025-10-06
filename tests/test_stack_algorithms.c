#include "TestAssert.h"
#include "TestHelpers.h"
#include "containers/stack.h"
#include <stdio.h>
#include <stdlib.h>

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
        int* data = malloc(sizeof(int));
        *data = original_values[i];
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
        int* data = malloc(sizeof(int));
        *data = i * 10;
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

typedef struct
{
    int (*func)(void);
    const char* name;
} TestCase;

int main(void)
{
    const TestCase tests[] = {
        {test_stack_copy_shallow, "test_stack_copy_shallow"},
        {test_stack_copy_deep, "test_stack_copy_deep"},
        {test_stack_for_each, "test_stack_for_each"},
        {test_stack_with_persons, "test_stack_with_persons"},
    };

    printf("Running Stack algorithm tests...\n");

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
        printf("All Stack algorithm tests passed!\n");
        return 0;
    }

    printf("%d Stack algorithm tests failed.\n", failed);
    return 1;
}