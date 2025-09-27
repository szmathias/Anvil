//
// Created by zack on 9/5/25.
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "containers/doublylinkedlist.h"
#include "TestAssert.h"
#include "TestHelpers.h"

int test_stress(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    const int NUM_ELEMENTS = 10000;

    // Add many elements
    for (int i = 0; i < NUM_ELEMENTS; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        ASSERT_EQ(anv_dll_push_back(list, val), 0);
    }
    ASSERT_EQ(list->size, (size_t)NUM_ELEMENTS);

    // Find an element in the middle
    int key = NUM_ELEMENTS / 2;
    const ANVDoublyLinkedNode* found = anv_dll_find(list, &key, int_cmp);
    ASSERT_NOT_NULL(found);
    ASSERT_EQ(*(int*)found->data, key);

    // Remove elements from the front
    for (int i = 0; i < NUM_ELEMENTS / 2; i++)
    {
        ASSERT_EQ(anv_dll_pop_front(list, true), 0);
    }
    ASSERT_EQ(list->size, (size_t)NUM_ELEMENTS / 2);

    // The first element should now be NUM_ELEMENTS/2
    key = NUM_ELEMENTS / 2;
    ASSERT_EQ(*(int*)list->head->data, key);

    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

int test_performance(void)
{
    const int SIZES[] = {100, 1000, 10000};
    const size_t NUM_SIZES = sizeof(SIZES) / sizeof(SIZES[0]);

    printf("\nDLL Performance tests:\n");
    for (size_t s = 0; s < NUM_SIZES; s++)
    {
        const int SIZE = SIZES[s];
        ANVAllocator alloc = create_int_allocator();
        ANVDoublyLinkedList* list = anv_dll_create(&alloc);

        // Measure insertion time
        clock_t start = clock();
        for (int i = 0; i < SIZE; i++)
        {
            int* val = malloc(sizeof(int));
            *val = i;
            anv_dll_push_back(list, val);
        }
        clock_t end = clock();
        printf("Insert %d elements: %.6f seconds\n", SIZE,
               (double)(end - start) / CLOCKS_PER_SEC);

        // Measure search time for last element
        start = clock();
        int key = SIZE - 1;
        const ANVDoublyLinkedNode* found = anv_dll_find(list, &key, int_cmp);
        end = clock();
        printf("Find last element in %d elements: %.6f seconds\n", SIZE,
               (double)(end - start) / CLOCKS_PER_SEC);
        ASSERT_NOT_NULL(found);

        // Cleanup
        anv_dll_destroy(list, true);
    }

    return TEST_SUCCESS;
}

typedef struct
{
    int (*func)(void);
    const char* name;
} TestCase;

TestCase tests[] = {
    {test_stress, "test_stress"},
    {test_performance, "test_performance"},
};

int main(void)
{
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
        printf("All DoublyLinkedList Performance tests passed.\n");
        return 0;
    }

    printf("%d DoublyLinkedList Performance tests failed.\n", failed);
    return 1;
}