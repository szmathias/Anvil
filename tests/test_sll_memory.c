//
// Created by zack on 9/2/25.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "containers/singlylinkedlist.h"
#include "TestAssert.h"
#include "TestHelpers.h"

// Tests from test_sll.c that fit into memory/resource category
int test_custom_allocator(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);
    ASSERT_NOT_NULL(list);
    int* a = malloc(sizeof(int));
    *a = 42;
    ASSERT_EQ(anv_sll_push_back(list, a), 0);
    ASSERT_EQ(list->size, 1);
    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

int test_clear(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);

    // Add some elements
    for (int i = 0; i < 5; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_sll_push_back(list, val);
    }
    ASSERT_EQ(list->size, 5);

    // Clear the list
    anv_sll_clear(list, true);

    // Verify list state
    ASSERT_NULL(list->head);
    ASSERT_EQ(list->size, 0);
    ASSERT_EQ(anv_sll_is_empty(list), 1);

    // Make sure we can still add elements after clearing
    int* val = malloc(sizeof(int));
    *val = 42;
    ASSERT_EQ(anv_sll_push_back(list, val), 0);
    ASSERT_EQ(list->size, 1);

    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

int test_clear_empty(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);

    // Clear an already empty list
    anv_sll_clear(list, true);
    ASSERT_NULL(list->head);
    ASSERT_EQ(list->size, 0);

    anv_sll_destroy(list, false);
    return TEST_SUCCESS;
}

int test_clear_null(void)
{
    // Calling clear on NULL shouldn't crash
    anv_sll_clear(NULL, true);
    return TEST_SUCCESS;
}

int test_copy_shallow(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);

    // Add some elements
    for (int i = 0; i < 5; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i * 10;
        anv_sll_push_back(list, val);
    }

    // Create shallow clone
    ANVSinglyLinkedList* clone = anv_sll_copy(list);
    ASSERT_NOT_NULL(clone);
    ASSERT_EQ(clone->size, list->size);

    // Verify structure
    ANVSinglyLinkedNode* orig_node = list->head;
    ANVSinglyLinkedNode* clone_node = clone->head;
    while (orig_node && clone_node)
    {
        // Data pointers should be identical in shallow clone
        ASSERT_EQ(orig_node->data, clone_node->data);
        // But nodes themselves should be different
        ASSERT_NOT_EQ(orig_node, clone_node);

        orig_node = orig_node->next;
        clone_node = clone_node->next;
    }

    // Modifying data should affect both lists (shared pointers)
    int* first_value = list->head->data;
    *first_value = 999;
    ASSERT_EQ(*(int*)clone->head->data, 999);

    // Cleanup - free each int only once since they're shared
    anv_sll_destroy(list, true);
    anv_sll_destroy(clone, false);
    return TEST_SUCCESS;
}

int test_copy_deep(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);

    // Add some elements
    for (int i = 0; i < 5; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i * 10;
        anv_sll_push_back(list, val);
    }

    // Create deep clone
    ANVSinglyLinkedList* clone = anv_sll_copy_deep(list, int_copy, true);
    ASSERT_NOT_NULL(clone);
    ASSERT_EQ(clone->size, list->size);

    // Verify structure and values
    const ANVSinglyLinkedNode* orig_node = list->head;
    const ANVSinglyLinkedNode* clone_node = clone->head;
    while (orig_node && clone_node)
    {
        // Data pointers should be different in deep clone
        ASSERT_NOT_EQ(orig_node->data, clone_node->data);
        // But values should be the same
        ASSERT_EQ(*(int*)orig_node->data, *(int*)clone_node->data);

        orig_node = orig_node->next;
        clone_node = clone_node->next;
    }

    // Modifying data should not affect the other list (independent copies)
    int* first_value = list->head->data;
    *first_value = 999;
    ASSERT_NOT_EQ(*(int*)clone->head->data, 999);

    // Cleanup - each list has its own data
    anv_sll_destroy(list, true);
    anv_sll_destroy(clone, true);
    return TEST_SUCCESS;
}

int test_copy_complex_data(void)
{
    ANVAllocator alloc = create_person_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);

    // Add some people
    Person* p1 = create_person("Alice", 30);
    Person* p2 = create_person("Bob", 25);
    Person* p3 = create_person("Charlie", 40);

    anv_sll_push_back(list, p1);
    anv_sll_push_back(list, p2);
    anv_sll_push_back(list, p3);

    // Create deep clone
    ANVSinglyLinkedList* clone = anv_sll_copy_deep(list, person_copy, true);
    ASSERT_NOT_NULL(clone);
    ASSERT_EQ(clone->size, list->size);

    // Verify structure and values
    const ANVSinglyLinkedNode* orig_node = list->head;
    const ANVSinglyLinkedNode* clone_node = clone->head;
    while (orig_node && clone_node)
    {
        Person* orig_person = orig_node->data;
        Person* clone_person = clone_node->data;

        // Data pointers should be different
        ASSERT_NOT_EQ(orig_person, clone_person);
        // But values should be the same
        ASSERT_EQ(strcmp(orig_person->name, clone_person->name), 0);
        ASSERT_EQ(orig_person->age, clone_person->age);

        orig_node = orig_node->next;
        clone_node = clone_node->next;
    }

    // Modifying should not affect the other list
    Person* first_person = list->head->data;
    first_person->age = 99;
    const Person* clone_first = clone->head->data;
    ASSERT_NOT_EQ(first_person->age, clone_first->age);

    // Cleanup
    anv_sll_destroy(list, true);
    anv_sll_destroy(clone, true);
    return TEST_SUCCESS;
}

int test_copy_empty(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);

    // Clone empty list
    ANVSinglyLinkedList* shallow_clone = anv_sll_copy(list);
    ASSERT_NOT_NULL(shallow_clone);
    ASSERT_EQ(shallow_clone->size, 0);
    ASSERT_NULL(shallow_clone->head);

    ANVSinglyLinkedList* deep_clone = anv_sll_copy_deep(list, int_copy, true);
    ASSERT_NOT_NULL(deep_clone);
    ASSERT_EQ(deep_clone->size, 0);
    ASSERT_NULL(deep_clone->head);

    // Cleanup
    anv_sll_destroy(list, false);
    anv_sll_destroy(shallow_clone, false);
    anv_sll_destroy(deep_clone, false);
    return TEST_SUCCESS;
}

int test_copy_null(void)
{
    // Should handle NULL gracefully
    ASSERT_NULL(anv_sll_copy(NULL));
    ASSERT_NULL(anv_sll_copy_deep(NULL, int_copy, true));

    // Should require a valid copy function
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);
    ASSERT_NULL(anv_sll_copy_deep(list, NULL, false));
    anv_sll_destroy(list, false);
    return TEST_SUCCESS;
}

int test_transform_allocation_failure(void)
{
    set_alloc_fail_countdown(-1); // Ensure normal allocation for setup
    ANVAllocator alloc = create_failing_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);
    ASSERT_NOT_NULL(list);
    for (int i = 0; i < 5; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_sll_push_back(list, val);
    }

    // Case 1: Fail on creation of the result list
    set_alloc_fail_countdown(0);
    ANVSinglyLinkedList* mapped1 = anv_sll_transform(list, double_value_failing, true);
    ASSERT_NULL(mapped1);

    // Case 2: Fail on data allocation inside the transform function
    // Allocations: 1=result list, FAIL on 2=data for first element
    set_alloc_fail_countdown(1);
    ANVSinglyLinkedList* mapped2 = anv_sll_transform(list, double_value_failing, true);
    ASSERT_NULL(mapped2); // sll_transform should handle this and clean up

    // Case 3: Fail on node allocation inside sll_insert_back
    // Allocations: 1=result list, 2=data for first element, FAIL on 3=node for first element
    set_alloc_fail_countdown(2);
    ANVSinglyLinkedList* mapped3 = anv_sll_transform(list, double_value_failing, true);
    ASSERT_NULL(mapped3);

    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

int test_copy_deep_allocation_failure(void)
{
    set_alloc_fail_countdown(-1);
    ANVAllocator alloc = create_failing_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);
    for (int i = 0; i < 5; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_sll_push_back(list, val);
    }

    // Case 1: Fail allocating the new list struct itself
    set_alloc_fail_countdown(0);
    ANVSinglyLinkedList* clone1 = anv_sll_copy_deep(list, failing_int_copy, true);
    ASSERT_NULL(clone1);

    // Case 2: Fail allocating a node partway through
    set_alloc_fail_countdown(3); // 1=clone list, 2=data0, 3=node0, FAIL on data1
    ANVSinglyLinkedList* clone2 = anv_sll_copy_deep(list, failing_int_copy, true);
    ASSERT_NULL(clone2);

    // Case 3: Fail allocating the *data* partway through
    set_alloc_fail_countdown(2); // 1=clone list, 2=data0, FAIL on node0
    ANVSinglyLinkedList* clone3 = anv_sll_copy_deep(list, failing_int_copy, true);
    ASSERT_NULL(clone3);

    set_alloc_fail_countdown(-1); // Reset for cleanup
    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

int test_insert_allocation_failure(void)
{
    set_alloc_fail_countdown(-1);
    ANVAllocator alloc = create_failing_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);
    int* a = malloc(sizeof(int));
    *a = 1;
    anv_sll_push_back(list, a);
    ASSERT_EQ(list->size, 1);

    // Set allocator to fail on the next allocation
    set_alloc_fail_countdown(0);
    int* b = malloc(sizeof(int));
    *b = 2;
    ASSERT_EQ(anv_sll_push_back(list, b), -1);

    // Verify list is unchanged
    ASSERT_EQ(list->size, 1);
    ASSERT_NOT_NULL(list->head);
    ASSERT_NULL(list->head->next);

    set_alloc_fail_countdown(-1);
    anv_sll_destroy(list, true);
    free(b); // 'b' was never added to the list, so we must free it manually
    return TEST_SUCCESS;
}

typedef struct
{
    int (*func)(void);
    const char* name;
} TestCase;

TestCase tests[] = {
    {test_custom_allocator, "test_custom_allocator"},
    {test_clear, "test_clear"},
    {test_clear_empty, "test_clear_empty"},
    {test_clear_null, "test_clear_null"},
    {test_copy_shallow, "test_copy_shallow"},
    {test_copy_deep, "test_copy_deep"},
    {test_copy_complex_data, "test_copy_complex_data"},
    {test_copy_empty, "test_copy_empty"},
    {test_copy_null, "test_copy_null"},
    {test_transform_allocation_failure, "test_transform_allocation_failure"},
    {test_copy_deep_allocation_failure, "test_copy_deep_allocation_failure"},
    {test_insert_allocation_failure, "test_insert_allocation_failure"},
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
        printf("All SinglyLinkedList Memory tests passed.\n");
        return 0;
    }

    printf("%d SinglyLinkedList Memory tests failed.\n", failed);
    return 1;
}