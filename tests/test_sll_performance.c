//
// Created by zack on 9/5/25.
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "containers/singlylinkedlist.h"
#include "TestAssert.h"
#include "TestHelpers.h"

int test_stress(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);
    const size_t NUM_ELEMENTS = 10000;

    // Add many elements
    for (size_t i = 0; i < NUM_ELEMENTS; i++)
    {
        int* val = malloc(sizeof(int));
        *val = (int)i;
        ASSERT_EQ(anv_sll_push_back(list, val), 0);
    }
    ASSERT_EQ(list->size, NUM_ELEMENTS);

    // Find an element in the middle
    const size_t key = NUM_ELEMENTS / 2;
    const ANVSinglyLinkedNode* found = anv_sll_find(list, &key, int_cmp);
    ASSERT_NOT_NULL(found);
    ASSERT_EQ(*(int*)found->data, (int)key);

    // Remove elements from the front
    for (size_t i = 0; i < NUM_ELEMENTS / 2; i++)
    {
        ASSERT_EQ(anv_sll_remove_at(list, 0, true), 0);
    }
    ASSERT_EQ(list->size, NUM_ELEMENTS / 2);

    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

int test_performance(void)
{
    const int NUM_SIZES = 3;

    printf("\nSLL Performance tests:\n");
    for (int s = 0; s < NUM_SIZES; s++)
    {
        const int SIZES[] = {100, 1000, 10000};
        const int SIZE = SIZES[s];
        ANVAllocator alloc = create_int_allocator();
        ANVSinglyLinkedList* list = anv_sll_create(&alloc);

        // Measure insertion time
        clock_t start = clock();
        for (int i = 0; i < SIZE; i++)
        {
            int* val = malloc(sizeof(int));
            *val = i;
            anv_sll_push_back(list, val);
        }
        clock_t end = clock();
        printf("Insert %d elements: %.6f seconds\n", SIZE,
               (double)(end - start) / CLOCKS_PER_SEC);

        // Measure search time for last element
        start = clock();
        int key = SIZE - 1;
        const ANVSinglyLinkedNode* found = anv_sll_find(list, &key, int_cmp);
        end = clock();
        printf("Find last element in %d elements: %.6f seconds\n", SIZE,
               (double)(end - start) / CLOCKS_PER_SEC);
        ASSERT_NOT_NULL(found);

        // Cleanup
        anv_sll_destroy(list, true);
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
        printf("All SinglyLinkedList Performance tests passed.\n");
        return 0;
    }

    printf("%d SinglyLinkedList Performance tests failed.\n", failed);
    return 1;
}