//
// Created by zack on 9/2/25.
//

#include "containers/arraylist.h"
#include "TestAssert.h"
#include "TestHelpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_reserve(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Test reserve on empty list
    ASSERT_EQ(anv_arraylist_reserve(list, 100), 0);
    ASSERT_GTE(anv_arraylist_capacity(list), 100);
    ASSERT_EQ(anv_arraylist_size(list), 0);

    // Add some elements
    int* a = malloc(sizeof(int));
    *a = 1;
    int* b = malloc(sizeof(int));
    *b = 2;
    anv_arraylist_push_back(list, a);
    anv_arraylist_push_back(list, b);

    const size_t old_capacity = anv_arraylist_capacity(list);

    // Reserve smaller capacity (should not shrink)
    ASSERT_EQ(anv_arraylist_reserve(list, 5), 0);
    ASSERT_EQ(anv_arraylist_capacity(list), old_capacity);
    ASSERT_EQ(anv_arraylist_size(list), 2);

    // Reserve larger capacity
    ASSERT_EQ(anv_arraylist_reserve(list, 200), 0);
    ASSERT_GTE(anv_arraylist_capacity(list), 200);
    ASSERT_EQ(anv_arraylist_size(list), 2);

    // Verify data integrity
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 0), 1);
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 1), 2);

    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

int test_shrink_to_fit(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 100);

    // Add some elements (less than capacity)
    for (int i = 0; i < 10; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_arraylist_push_back(list, val);
    }

    ASSERT_GT(anv_arraylist_capacity(list), anv_arraylist_size(list));

    // Shrink to fit
    ASSERT_EQ(anv_arraylist_shrink_to_fit(list), 0);
    ASSERT_EQ(anv_arraylist_capacity(list), anv_arraylist_size(list));
    ASSERT_EQ(anv_arraylist_size(list), 10);

    // Verify data integrity
    for (int i = 0; i < 10; i++)
    {
        ASSERT_EQ(*(int*)anv_arraylist_get(list, i), i);
    }

    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

int test_shrink_empty_list(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 100);

    // Shrink empty list
    ASSERT_EQ(anv_arraylist_shrink_to_fit(list), 0);
    ASSERT_EQ(anv_arraylist_capacity(list), 0);
    ASSERT_EQ(anv_arraylist_size(list), 0);

    anv_arraylist_destroy(list, false);
    return TEST_SUCCESS;
}

int test_growth_pattern(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    size_t last_capacity = 0;

    // Test automatic growth
    for (int i = 0; i < 100; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_arraylist_push_back(list, val);

        const size_t current_capacity = anv_arraylist_capacity(list);
        if (current_capacity != last_capacity)
        {
            // Capacity should grow by at least 1.5x (our growth factor)
            if (last_capacity > 0)
            {
                ASSERT_GTE(current_capacity, last_capacity + (last_capacity >> 1));
            }
            last_capacity = current_capacity;
        }
    }

    ASSERT_EQ(anv_arraylist_size(list), 100);

    // Verify all data
    for (int i = 0; i < 100; i++)
    {
        ASSERT_EQ(*(int*)anv_arraylist_get(list, i), i);
    }

    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

int test_memory_allocation_failure(void)
{
    // This test would require a custom allocator that can simulate failures
    // For now, just test with NULL allocator
    ANVArrayList* list = anv_arraylist_create(NULL, 0);
    ASSERT_NULL(list);
    return TEST_SUCCESS;
}

int test_large_capacity(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 1000);

    ASSERT_NOT_NULL(list);
    ASSERT_GTE(anv_arraylist_capacity(list), 1000);
    ASSERT_EQ(anv_arraylist_size(list), 0);

    // Fill it up
    for (int i = 0; i < 1000; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_arraylist_push_back(list, val);
    }

    ASSERT_EQ(anv_arraylist_size(list), 1000);

    // Spot check some values
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 0), 0);
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 500), 500);
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 999), 999);

    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

int test_memory_cleanup_on_destroy(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 10);

    // Add elements
    for (int i = 0; i < 5; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_arraylist_push_back(list, val);
    }

    // Destroy with data cleanup
    anv_arraylist_destroy(list, true);

    // If we get here without crash, memory was properly cleaned up
    return TEST_SUCCESS;
}

int test_memory_cleanup_on_clear(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 10);

    // Add elements
    for (int i = 0; i < 5; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_arraylist_push_back(list, val);
    }

    const size_t capacity_before = anv_arraylist_capacity(list);

    // Clear with data cleanup
    anv_arraylist_clear(list, true);

    ASSERT_EQ(anv_arraylist_size(list), 0);
    ASSERT_EQ(anv_arraylist_capacity(list), capacity_before); // Capacity preserved

    anv_arraylist_destroy(list, false);
    return TEST_SUCCESS;
}

int test_capacity_consistency(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Capacity should always be >= size
    for (int i = 0; i < 50; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_arraylist_push_back(list, val);

        ASSERT_GTE(anv_arraylist_capacity(list), anv_arraylist_size(list));
    }

    // Remove elements and check consistency
    for (int i = 0; i < 25; i++)
    {
        anv_arraylist_pop_back(list, true);
        ASSERT_GTE(anv_arraylist_capacity(list), anv_arraylist_size(list));
    }

    anv_arraylist_destroy(list, true);
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
        {test_reserve, "test_reserve"},
        {test_shrink_to_fit, "test_shrink_to_fit"},
        {test_shrink_empty_list, "test_shrink_empty_list"},
        {test_growth_pattern, "test_growth_pattern"},
        {test_memory_allocation_failure, "test_memory_allocation_failure"},
        {test_large_capacity, "test_large_capacity"},
        {test_memory_cleanup_on_destroy, "test_memory_cleanup_on_destroy"},
        {test_memory_cleanup_on_clear, "test_memory_cleanup_on_clear"},
        {test_capacity_consistency, "test_capacity_consistency"},
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

    if (failed == 0)
    {
        printf("All ArrayList memory tests passed.\n");
        return 0;
    }

    printf("%d ArrayList memory tests failed.\n", failed);
    return 1;
}