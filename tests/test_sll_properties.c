//
// Created by zack on 9/4/25.
//

#include <stdlib.h>

#include "containers/singlylinkedlist.h"
#include "TestAssert.h"
#include "TestHelpers.h"

// Property: The size of the list should be consistent after a series of insertions and removals.
int test_sll_size_after_insert_and_remove(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);
    ASSERT_EQ(anv_sll_size(list), 0);

    int* a = malloc(sizeof(int));
    *a = 1;
    anv_sll_push_back(list, a);
    ASSERT_EQ(anv_sll_size(list), 1);

    int* b = malloc(sizeof(int));
    *b = 2;
    anv_sll_push_back(list, b);
    ASSERT_EQ(anv_sll_size(list), 2);

    anv_sll_remove_at(list, 0, true);
    ASSERT_EQ(anv_sll_size(list), 1);

    anv_sll_remove_at(list, 0, true);
    ASSERT_EQ(anv_sll_size(list), 0);

    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

// Property: Sorting an already sorted list should not change it.
int test_sll_sort_is_idempotent(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);
    for (int i = 0; i < 10; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_sll_push_back(list, val);
    }

    anv_sll_sort(list, int_cmp); // First sort
    ANVSinglyLinkedList* copy = anv_sll_copy_deep(list, int_copy, true);

    anv_sll_sort(list, int_cmp); // Second sort

    ASSERT_EQ(anv_sll_equals(list, copy, int_cmp), 1);

    anv_sll_destroy(list, true);
    anv_sll_destroy(copy, true);
    return TEST_SUCCESS;
}

// Property: Reversing a list twice should return it to its original state.
int test_sll_reverse_is_involution(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);
    for (int i = 0; i < 10; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_sll_push_back(list, val);
    }

    ANVSinglyLinkedList* copy = anv_sll_copy_deep(list, int_copy, true);

    anv_sll_reverse(list);
    anv_sll_reverse(list);

    ASSERT_EQ(anv_sll_equals(list, copy, int_cmp), 1);

    anv_sll_destroy(list, true);
    anv_sll_destroy(copy, true);
    return TEST_SUCCESS;
}

typedef struct
{
    int (*func)(void);
    const char* name;
} TestCase;

TestCase tests[] = {
    {test_sll_size_after_insert_and_remove, "test_sll_size_after_insert_and_remove"},
    {test_sll_sort_is_idempotent, "test_sll_sort_is_idempotent"},
    {test_sll_reverse_is_involution, "test_sll_reverse_is_involution"},
};

int main(void)
{
    int failed = 0;
    const int num_tests = sizeof(tests) / sizeof(tests[0]);

    for (int i = 0; i < num_tests; i++)
    {
        if (tests[i].func() != TEST_SUCCESS)
        {
            printf("FAIL: %s\n", tests[i].name);
            failed++;
        }
    }

    if (failed == 0)
    {
        printf("All SinglyLinkedList property tests passed.\n");
        return 0;
    }

    printf("%d SinglyLinkedList property tests failed.\n", failed);
    return 1;
}