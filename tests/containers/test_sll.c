//
// Consolidated SLL tests
// Merged from: test_sll_crud.c, test_sll_algorithms.c, test_sll_iterator.c,
//              test_sll_memory.c, test_sll_performance.c, test_sll_properties.c
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "containers/singlylinkedlist.h"
#include "TestAssert.h"
#include "TestHelpers.h"
#include "TestRunner.h"

//==============================================================================
// CRUD Tests
//==============================================================================

int test_create_destroy(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);
    ASSERT_NOT_NULL(list);
    ASSERT_EQ(list->size, 0);
    anv_sll_destroy(list, false);
    return TEST_SUCCESS;
}

int test_insert_front_back_find(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);
    int* a = malloc(sizeof(int));
    *a = 1;
    int* b = malloc(sizeof(int));
    *b = 2;
    int* c = malloc(sizeof(int));
    *c = 3;
    ASSERT_EQ(anv_sll_push_front(list, a), 0);
    ASSERT_EQ(anv_sll_push_back(list, b), 0);
    ASSERT_EQ(anv_sll_push_back(list, c), 0);
    ASSERT_EQ(list->size, 3);

    const int key = 2;
    const ANVSinglyLinkedNode* found = anv_sll_find(list, &key, int_cmp);
    ASSERT_NOT_NULL(found);
    ASSERT_EQ(*(int*)found->data, 2);

    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

int test_remove(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);
    int* a = malloc(sizeof(int));
    *a = 1;
    int* b = malloc(sizeof(int));
    *b = 2;
    int* c = malloc(sizeof(int));
    *c = 3;
    anv_sll_push_back(list, a);
    anv_sll_push_back(list, b);
    anv_sll_push_back(list, c);

    const int key = 2;
    ASSERT_EQ(anv_sll_remove(list, &key, int_cmp, true), 0);
    ASSERT_EQ(list->size, 2);
    ANVSinglyLinkedNode* found = anv_sll_find(list, &key, int_cmp);
    ASSERT_NULL(found);

    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

int test_remove_not_found(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);
    int* a = malloc(sizeof(int));
    *a = 1;
    anv_sll_push_back(list, a);

    const int key = 99;
    ASSERT_EQ(anv_sll_remove(list, &key, int_cmp, true), -1);

    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

int test_NULL_handling(void)
{
    ASSERT_EQ(anv_sll_push_back(NULL, NULL), -1);
    ASSERT_EQ(anv_sll_push_front(NULL, NULL), -1);
    ASSERT_EQ_PTR(anv_sll_find(NULL, NULL, NULL), NULL);
    ASSERT_EQ(anv_sll_remove(NULL, NULL, NULL, false), -1);
    anv_sll_destroy(NULL, false); // Should not crash
    return TEST_SUCCESS;
}

int test_insert_at(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);
    int* a = malloc(sizeof(int));
    *a = 1;
    int* b = malloc(sizeof(int));
    *b = 2;
    int* c = malloc(sizeof(int));
    *c = 3;
    ASSERT_EQ(anv_sll_push_back(list, a), 0);  // [1]
    ASSERT_EQ(anv_sll_push_back(list, c), 0);  // [1,3]
    ASSERT_EQ(anv_sll_insert_at(list, 1, b), 0); // [1,2,3]
    ASSERT_EQ(list->size, 3);

    const int key = 2;
    const ANVSinglyLinkedNode* found = anv_sll_find(list, &key, int_cmp);
    ASSERT_NOT_NULL(found);
    ASSERT_EQ(*(int*)found->data, 2);

    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

int test_remove_at(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);
    int* a = malloc(sizeof(int));
    *a = 10;
    int* b = malloc(sizeof(int));
    *b = 20;
    int* c = malloc(sizeof(int));
    *c = 30;
    anv_sll_push_back(list, a); // [10]
    anv_sll_push_back(list, b); // [10,20]
    anv_sll_push_back(list, c); // [10,20,30]

    ASSERT_EQ(anv_sll_remove_at(list, 1, true), 0); // remove 20
    ASSERT_EQ(list->size, 2);

    const int key = 20;
    ANVSinglyLinkedNode* found = anv_sll_find(list, &key, int_cmp);
    ASSERT_NULL(found);

    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

int test_remove_at_head(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);
    int* a = malloc(sizeof(int));
    *a = 100;
    int* b = malloc(sizeof(int));
    *b = 200;
    anv_sll_push_back(list, a); // [100]
    anv_sll_push_back(list, b); // [100,200]

    ASSERT_EQ(anv_sll_remove_at(list, 0, true), 0); // remove head (100)
    ASSERT_EQ(list->size, 1);

    const int key = 100;
    ANVSinglyLinkedNode* found = anv_sll_find(list, &key, int_cmp);
    ASSERT_NULL(found);

    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

int test_remove_at_last(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);
    int* a = malloc(sizeof(int));
    *a = 1;
    int* b = malloc(sizeof(int));
    *b = 2;
    int* c = malloc(sizeof(int));
    *c = 3;
    anv_sll_push_back(list, a); // [1]
    anv_sll_push_back(list, b); // [1,2]
    anv_sll_push_back(list, c); // [1,2,3]

    ASSERT_EQ(anv_sll_remove_at(list, 2, true), 0); // remove last (3)
    ASSERT_EQ(list->size, 2);

    const int key = 3;
    ANVSinglyLinkedNode* found = anv_sll_find(list, &key, int_cmp);
    ASSERT_NULL(found);

    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

int test_remove_at_invalid(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);
    int* a = malloc(sizeof(int));
    *a = 1;
    anv_sll_push_back(list, a); // [1]

    ASSERT_EQ(anv_sll_remove_at(list, 5, true), -1);          // invalid position
    ASSERT_EQ(anv_sll_remove_at(list, (size_t)-1, true), -1); // negative as size_t (very large)

    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

int test_remove_at_empty(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);
    ASSERT_EQ(anv_sll_remove_at(list, 0, true), -1); // nothing to remove
    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

int test_remove_at_single_element(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);
    int* a = malloc(sizeof(int));
    *a = 123;
    anv_sll_push_back(list, a);                   // [123]
    ASSERT_EQ(anv_sll_remove_at(list, 0, true), 0); // remove only element
    ASSERT_EQ(list->size, 0);
    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

int test_remove_at_single_element_invalid_pos(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);
    int* a = malloc(sizeof(int));
    *a = 123;
    anv_sll_push_back(list, a);                    // [123]
    ASSERT_EQ(anv_sll_remove_at(list, 1, true), -1); // invalid position
    ASSERT_EQ(list->size, 1);
    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

int test_insert_at_out_of_bounds(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);
    int* a = malloc(sizeof(int));
    *a = 1;
    ASSERT_EQ(anv_sll_insert_at(list, 2, a), -1);          // out of bounds (list size is 0)
    ASSERT_EQ(anv_sll_insert_at(list, (size_t)-1, a), -1); // very large index
    anv_sll_destroy(list, true);
    free(a);
    return TEST_SUCCESS;
}

int test_insert_remove_null_data(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);
    ASSERT_EQ(anv_sll_push_back(list, NULL), 0); // allow nullptr data
    ASSERT_EQ(list->size, 1);
    ASSERT_EQ(anv_sll_remove_at(list, 0, false), 0); // remove node with nullptr data, no free_func
    ASSERT_EQ(list->size, 0);
    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

int test_mixed_operations_integrity(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);
    int* a = malloc(sizeof(int));
    *a = 10;
    int* b = malloc(sizeof(int));
    *b = 20;
    int* c = malloc(sizeof(int));
    *c = 30;
    anv_sll_push_back(list, a);  // [10]
    anv_sll_push_front(list, b); // [20,10]
    anv_sll_insert_at(list, 1, c); // [20,30,10]
    ASSERT_EQ(list->size, 3);

    ASSERT_EQ(anv_sll_remove_at(list, 1, true), 0); // remove 30, [20,10]
    int key = 30;
    ANVSinglyLinkedNode* found = anv_sll_find(list, &key, int_cmp);
    ASSERT_NULL(found);

    key = 20;
    found = anv_sll_find(list, &key, int_cmp);
    ASSERT_NOT_NULL(found);

    key = 10;
    found = anv_sll_find(list, &key, int_cmp);
    ASSERT_NOT_NULL(found);

    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

int test_size(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);
    ASSERT_EQ(anv_sll_size(list), 0);

    int* a = malloc(sizeof(int));
    *a = 10;
    int* b = malloc(sizeof(int));
    *b = 20;
    anv_sll_push_back(list, a);
    ASSERT_EQ(anv_sll_size(list), 1);
    anv_sll_push_back(list, b);
    ASSERT_EQ(anv_sll_size(list), 2);

    anv_sll_remove_at(list, 0, true);
    ASSERT_EQ(anv_sll_size(list), 1);

    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

int test_is_empty(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);
    ASSERT_EQ(anv_sll_is_empty(list), 1); // Empty list

    int* a = malloc(sizeof(int));
    *a = 10;
    anv_sll_push_back(list, a);
    ASSERT_EQ(anv_sll_is_empty(list), 0); // Non-empty list

    anv_sll_remove_at(list, 0, true);
    ASSERT_EQ(anv_sll_is_empty(list), 1); // Empty again

    ASSERT_EQ(anv_sll_is_empty(NULL), 1); // NULL list should be considered empty

    anv_sll_destroy(list, false);
    return TEST_SUCCESS;
}

int test_complex_data_type(void)
{
    ANVAllocator alloc = create_person_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);

    Person* p1 = create_person("Alice", 30);
    Person* p2 = create_person("Bob", 25);
    Person* p3 = create_person("Charlie", 40);

    anv_sll_push_back(list, p1);
    anv_sll_push_back(list, p2);
    anv_sll_push_back(list, p3);
    ASSERT_EQ(list->size, 3);

    Person search_key;
    strcpy(search_key.name, "Bob");
    search_key.age = 0; // Age doesn't matter for comparison

    const ANVSinglyLinkedNode* found = anv_sll_find(list, &search_key, person_cmp);
    ASSERT_NOT_NULL(found);
    const Person* found_person = found->data;
    ASSERT_EQ(found_person->age, 25);

    // Clean up
    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

int test_remove_all(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);

    // Add 10 elements
    for (int i = 0; i < 10; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_sll_push_back(list, val);
    }
    ASSERT_EQ(list->size, 10);

    // Remove all elements one by one
    while (!anv_sll_is_empty(list))
    {
        anv_sll_remove_at(list, 0, true);
    }

    ASSERT_EQ(list->size, 0);
    ASSERT_NULL(list->head);

    anv_sll_destroy(list, false); // Already freed all data
    return TEST_SUCCESS;
}

int test_remove_front(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);

    // Test on empty list
    ASSERT_EQ(anv_sll_pop_front(list, true), -1);

    // Add elements
    int* a = malloc(sizeof(int));
    *a = 10;
    int* b = malloc(sizeof(int));
    *b = 20;
    int* c = malloc(sizeof(int));
    *c = 30;
    anv_sll_push_back(list, a);
    anv_sll_push_back(list, b);
    anv_sll_push_back(list, c);
    ASSERT_EQ(list->size, 3);

    // Remove front
    ASSERT_EQ(anv_sll_pop_front(list, true), 0);
    ASSERT_EQ(list->size, 2);

    // Check first element is now 20
    int key = 10;
    ASSERT_NULL(anv_sll_find(list, &key, int_cmp));
    key = 20;
    ASSERT_NOT_NULL(anv_sll_find(list, &key, int_cmp));

    // Remove until empty
    ASSERT_EQ(anv_sll_pop_front(list, true), 0);
    ASSERT_EQ(anv_sll_pop_front(list, true), 0);
    ASSERT_EQ(list->size, 0);
    ASSERT_NULL(list->head);

    anv_sll_destroy(list, false);
    return TEST_SUCCESS;
}

int test_remove_back(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);

    // Test on empty list
    ASSERT_EQ(anv_sll_pop_back(list, true), -1);

    // Test on single element list
    int* a = malloc(sizeof(int));
    *a = 10;
    anv_sll_push_back(list, a);
    ASSERT_EQ(anv_sll_pop_back(list, true), 0);
    ASSERT_EQ(list->size, 0);
    ASSERT_NULL(list->head);

    // Test with multiple elements
    int* b = malloc(sizeof(int));
    *b = 20;
    int* c = malloc(sizeof(int));
    *c = 30;
    int* d = malloc(sizeof(int));
    *d = 40;
    anv_sll_push_back(list, b);
    anv_sll_push_back(list, c);
    anv_sll_push_back(list, d);
    ASSERT_EQ(list->size, 3);

    // Remove back
    ASSERT_EQ(anv_sll_pop_back(list, true), 0);
    ASSERT_EQ(list->size, 2);

    // Check last element was removed
    int key = 40;
    ASSERT_NULL(anv_sll_find(list, &key, int_cmp));
    key = 30;
    ASSERT_NOT_NULL(anv_sll_find(list, &key, int_cmp));

    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Algorithm Tests
//==============================================================================

int test_sort_empty(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);

    ASSERT_EQ(anv_sll_sort(list, int_cmp), 0); // Empty list is already sorted
    ASSERT_EQ(list->size, 0);

    anv_sll_destroy(list, false);
    return TEST_SUCCESS;
}

int test_sort_already_sorted(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);
    for (int i = 0; i < 5; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_sll_push_back(list, val);
    }

    ASSERT_EQ(anv_sll_sort(list, int_cmp), 0);

    // Verify order
    const ANVSinglyLinkedNode* node = list->head;
    for (int i = 0; i < 5; i++)
    {
        ASSERT_EQ(*(int*)node->data, i);
        node = node->next;
    }

    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

int test_sort_reverse_order(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);
    for (int i = 4; i >= 0; i--)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_sll_push_back(list, val);
    }

    ASSERT_EQ(anv_sll_sort(list, int_cmp), 0);

    // Verify order
    const ANVSinglyLinkedNode* node = list->head;
    for (int i = 0; i < 5; i++)
    {
        ASSERT_EQ(*(int*)node->data, i);
        node = node->next;
    }

    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

int test_sort_random_order(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);
    const int values[] = {42, 17, 9, 39, 24, 5, 58};
    const int count = sizeof(values) / sizeof(values[0]);

    for (int i = 0; i < count; i++)
    {
        int* val = malloc(sizeof(int));
        *val = values[i];
        anv_sll_push_back(list, val);
    }

    ASSERT_EQ(anv_sll_sort(list, int_cmp), 0);

    // Verify order
    const ANVSinglyLinkedNode* node = list->head;
    for (int i = 0; i < count; i++)
    {
        const int sorted[] = {5, 9, 17, 24, 39, 42, 58};
        ASSERT_EQ(*(int*)node->data, sorted[i]);
        node = node->next;
    }

    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

int test_sort_with_duplicates(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);
    const int values[] = {5, 2, 9, 5, 7, 2, 9, 5};
    const int count = sizeof(values) / sizeof(values[0]);

    for (int i = 0; i < count; i++)
    {
        int* val = malloc(sizeof(int));
        *val = values[i];
        anv_sll_push_back(list, val);
    }

    ASSERT_EQ(anv_sll_sort(list, int_cmp), 0);

    // Verify order
    const ANVSinglyLinkedNode* node = list->head;
    for (int i = 0; i < count; i++)
    {
        const int sorted[] = {2, 2, 5, 5, 5, 7, 9, 9};
        ASSERT_EQ(*(int*)node->data, sorted[i]);
        node = node->next;
    }

    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

int test_sort_large_list(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);
    const int SIZE = 1000;

    // Insert in reverse order
    for (int i = SIZE - 1; i >= 0; i--)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_sll_push_back(list, val);
    }

    const clock_t start = clock();
    ASSERT_EQ(anv_sll_sort(list, int_cmp), 0);
    const clock_t end = clock();
    printf("SLL Sort %d elements: %.6f seconds\n", SIZE,
           (double)(end - start) / CLOCKS_PER_SEC);

    // Verify order (first few elements)
    const ANVSinglyLinkedNode* node = list->head;
    for (int i = 0; i < 10; i++)
    {
        ASSERT_EQ(*(int*)node->data, i);
        node = node->next;
    }

    // Verify list structure
    ASSERT_EQ(list->size, (size_t)SIZE);

    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

int test_sort_custom_compare(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);
    for (int i = 0; i < 5; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_sll_push_back(list, val);
    }

    // Sort in descending order
    ASSERT_EQ(anv_sll_sort(list, int_cmp_desc), 0);

    // Verify order
    const ANVSinglyLinkedNode* node = list->head;
    for (int i = 4; i >= 0; i--)
    {
        ASSERT_EQ(*(int*)node->data, i);
        node = node->next;
    }

    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

int test_sort_null_args(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);
    ASSERT_EQ(anv_sll_sort(NULL, int_cmp), -1); // NULL list
    ASSERT_EQ(anv_sll_sort(list, NULL), -1);    // NULL compare function
    anv_sll_destroy(list, false);
    return TEST_SUCCESS;
}

int test_sort_stability(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);

    // Person structs with same name (for comparison) but different ages
    Person* p1 = create_person("Alice", 30);
    Person* p2 = create_person("Alice", 25); // Same name, different age
    Person* p3 = create_person("Bob", 35);
    Person* p4 = create_person("Alice", 40); // Same name, different age

    anv_sll_push_back(list, p1);
    anv_sll_push_back(list, p2);
    anv_sll_push_back(list, p3);
    anv_sll_push_back(list, p4);

    // Sort by name only - ages should remain in insertion order for equal names
    ASSERT_EQ(anv_sll_sort(list, person_cmp), 0);

    // Verify order: All Alice's should come before Bob
    const ANVSinglyLinkedNode* node = list->head;
    const Person* person = node->data;
    ASSERT_EQ(strcmp(person->name, "Alice"), 0);
    ASSERT_EQ(person->age, 30); // First Alice

    node = node->next;
    person = node->data;
    ASSERT_EQ(strcmp(person->name, "Alice"), 0);
    ASSERT_EQ(person->age, 25); // Second Alice

    node = node->next;
    person = node->data;
    ASSERT_EQ(strcmp(person->name, "Alice"), 0);
    ASSERT_EQ(person->age, 40); // Third Alice

    node = node->next;
    person = node->data;
    ASSERT_EQ(strcmp(person->name, "Bob"), 0);
    ASSERT_EQ(person->age, 35);

    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

int test_reverse(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);

    // Test empty list
    ASSERT_EQ(anv_sll_reverse(list), 0);
    ASSERT_EQ(list->size, 0);

    // Test single element
    int* a = malloc(sizeof(int));
    *a = 10;
    anv_sll_push_back(list, a);
    ASSERT_EQ(anv_sll_reverse(list), 0);
    ASSERT_EQ(list->size, 1);
    ASSERT_EQ(*(int*)list->head->data, 10);

    // Test multiple elements
    int* b = malloc(sizeof(int));
    *b = 20;
    int* c = malloc(sizeof(int));
    *c = 30;
    anv_sll_push_back(list, b);
    anv_sll_push_back(list, c);
    // List is now [10,20,30]

    ASSERT_EQ(anv_sll_reverse(list), 0);
    // List should now be [30,20,10]

    const ANVSinglyLinkedNode* node = list->head;
    ASSERT_EQ(*(int*)node->data, 30);
    node = node->next;
    ASSERT_EQ(*(int*)node->data, 20);
    node = node->next;
    ASSERT_EQ(*(int*)node->data, 10);
    ASSERT_NULL(node->next);

    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

int test_merge(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list1 = anv_sll_create(&alloc);
    ANVSinglyLinkedList* list2 = anv_sll_create(&alloc);

    // Test merging empty lists
    ASSERT_EQ(anv_sll_merge(list1, list2), 0);
    ASSERT_EQ(list1->size, 0);
    ASSERT_EQ(list2->size, 0);

    // Test merging empty with non-empty
    int* a1 = malloc(sizeof(int));
    *a1 = 10;
    int* b1 = malloc(sizeof(int));
    *b1 = 20;
    anv_sll_push_back(list2, a1);
    anv_sll_push_back(list2, b1);

    ASSERT_EQ(anv_sll_merge(list1, list2), 0);
    ASSERT_EQ(list1->size, 2);
    ASSERT_EQ(list2->size, 0);
    ASSERT_NULL(list2->head);

    const ANVSinglyLinkedNode* node = list1->head;
    ASSERT_EQ(*(int*)node->data, 10);
    node = node->next;
    ASSERT_EQ(*(int*)node->data, 20);

    // Test merging two non-empty lists
    ANVSinglyLinkedList* list3 = anv_sll_create(&alloc);
    int* a2 = malloc(sizeof(int));
    *a2 = 30;
    int* b2 = malloc(sizeof(int));
    *b2 = 40;
    anv_sll_push_back(list3, a2);
    anv_sll_push_back(list3, b2);

    ASSERT_EQ(anv_sll_merge(list1, list3), 0);
    ASSERT_EQ(list1->size, 4);
    ASSERT_EQ(list3->size, 0);

    node = list1->head;
    ASSERT_EQ(*(int*)node->data, 10);
    node = node->next;
    ASSERT_EQ(*(int*)node->data, 20);
    node = node->next;
    ASSERT_EQ(*(int*)node->data, 30);
    node = node->next;
    ASSERT_EQ(*(int*)node->data, 40);

    anv_sll_destroy(list1, true);
    anv_sll_destroy(list2, false); // Already empty
    anv_sll_destroy(list3, false); // Already empty
    return TEST_SUCCESS;
}

int test_splice(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* dest = anv_sll_create(&alloc);
    ANVSinglyLinkedList* src = anv_sll_create(&alloc);

    // Test splicing empty lists
    ASSERT_EQ(anv_sll_splice(dest, src, 0), 0);
    ASSERT_EQ(dest->size, 0);
    ASSERT_EQ(src->size, 0);

    // Setup lists
    int* a = malloc(sizeof(int));
    *a = 10;
    int* b = malloc(sizeof(int));
    *b = 20;
    int* c = malloc(sizeof(int));
    *c = 30;
    anv_sll_push_back(dest, a);
    anv_sll_push_back(dest, b);
    anv_sll_push_back(dest, c);
    // dest = [10,20,30]

    int* d = malloc(sizeof(int));
    *d = 40;
    int* e = malloc(sizeof(int));
    *e = 50;
    anv_sll_push_back(src, d);
    anv_sll_push_back(src, e);
    // src = [40,50]

    // Test splicing at beginning
    ANVSinglyLinkedList* dest2 = anv_sll_create(&alloc);
    ANVSinglyLinkedList* src2 = anv_sll_create(&alloc);
    int* a2 = malloc(sizeof(int));
    *a2 = 10;
    int* b2 = malloc(sizeof(int));
    *b2 = 20;
    int* c2 = malloc(sizeof(int));
    *c2 = 30;
    int* d2 = malloc(sizeof(int));
    *d2 = 40;
    int* e2 = malloc(sizeof(int));
    *e2 = 50;
    anv_sll_push_back(dest2, a2);
    anv_sll_push_back(dest2, b2);
    anv_sll_push_back(dest2, c2);
    anv_sll_push_back(src2, d2);
    anv_sll_push_back(src2, e2);

    ASSERT_EQ(anv_sll_splice(dest2, src2, 0), 0);
    ASSERT_EQ(dest2->size, 5);
    ASSERT_EQ(src2->size, 0);

    const ANVSinglyLinkedNode* node = dest2->head;
    ASSERT_EQ(*(int*)node->data, 40);
    node = node->next;
    ASSERT_EQ(*(int*)node->data, 50);
    node = node->next;
    ASSERT_EQ(*(int*)node->data, 10);
    node = node->next;
    ASSERT_EQ(*(int*)node->data, 20);
    node = node->next;
    ASSERT_EQ(*(int*)node->data, 30);

    // Test splicing in the middle
    ASSERT_EQ(anv_sll_splice(dest, src, 1), 0);
    ASSERT_EQ(dest->size, 5);
    ASSERT_EQ(src->size, 0);

    node = dest->head;
    ASSERT_EQ(*(int*)node->data, 10);
    node = node->next;
    ASSERT_EQ(*(int*)node->data, 40);
    node = node->next;
    ASSERT_EQ(*(int*)node->data, 50);
    node = node->next;
    ASSERT_EQ(*(int*)node->data, 20);
    node = node->next;
    ASSERT_EQ(*(int*)node->data, 30);

    // Test splicing at the end
    ANVSinglyLinkedList* dest3 = anv_sll_create(&alloc);
    ANVSinglyLinkedList* src3 = anv_sll_create(&alloc);
    int* a3 = malloc(sizeof(int));
    *a3 = 10;
    int* b3 = malloc(sizeof(int));
    *b3 = 20;
    int* c3 = malloc(sizeof(int));
    *c3 = 30;
    int* d3 = malloc(sizeof(int));
    *d3 = 40;
    int* e3 = malloc(sizeof(int));
    *e3 = 50;
    anv_sll_push_back(dest3, a3);
    anv_sll_push_back(dest3, b3);
    anv_sll_push_back(dest3, c3);
    anv_sll_push_back(src3, d3);
    anv_sll_push_back(src3, e3);

    ASSERT_EQ(anv_sll_splice(dest3, src3, 3), 0);
    ASSERT_EQ(dest3->size, 5);
    ASSERT_EQ(src3->size, 0);

    node = dest3->head;
    ASSERT_EQ(*(int*)node->data, 10);
    node = node->next;
    ASSERT_EQ(*(int*)node->data, 20);
    node = node->next;
    ASSERT_EQ(*(int*)node->data, 30);
    node = node->next;
    ASSERT_EQ(*(int*)node->data, 40);
    node = node->next;
    ASSERT_EQ(*(int*)node->data, 50);

    anv_sll_destroy(dest, true);
    anv_sll_destroy(src, false);
    anv_sll_destroy(dest2, true);
    anv_sll_destroy(src2, false);
    anv_sll_destroy(dest3, true);
    anv_sll_destroy(src3, false);
    return TEST_SUCCESS;
}

int test_equals(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list1 = anv_sll_create(&alloc);
    ANVSinglyLinkedList* list2 = anv_sll_create(&alloc);

    // Empty lists should be equal
    ASSERT_EQ(anv_sll_equals(list1, list2, int_cmp), 1);

    // Lists with same elements should be equal
    int* a1 = malloc(sizeof(int));
    *a1 = 10;
    int* b1 = malloc(sizeof(int));
    *b1 = 20;
    int* a2 = malloc(sizeof(int));
    *a2 = 10;
    int* b2 = malloc(sizeof(int));
    *b2 = 20;
    anv_sll_push_back(list1, a1);
    anv_sll_push_back(list1, b1);
    anv_sll_push_back(list2, a2);
    anv_sll_push_back(list2, b2);

    ASSERT_EQ(anv_sll_equals(list1, list2, int_cmp), 1);

    // Lists with different elements should not be equal
    int* c2 = malloc(sizeof(int));
    *c2 = 30;
    anv_sll_push_back(list2, c2);

    ASSERT_EQ(anv_sll_equals(list1, list2, int_cmp), 0);

    // Lists with same size but different elements should not be equal
    ANVSinglyLinkedList* list3 = anv_sll_create(&alloc);
    int* a3 = malloc(sizeof(int));
    *a3 = 10;
    int* b3 = malloc(sizeof(int));
    *b3 = 30; // Different value
    anv_sll_push_back(list3, a3);
    anv_sll_push_back(list3, b3);

    ASSERT_EQ(anv_sll_equals(list1, list3, int_cmp), 0);

    // Error cases
    ASSERT_EQ(anv_sll_equals(NULL, list2, int_cmp), -1);
    ASSERT_EQ(anv_sll_equals(list1, NULL, int_cmp), -1);
    ASSERT_EQ(anv_sll_equals(list1, list2, NULL), -1);

    anv_sll_destroy(list1, true);
    anv_sll_destroy(list2, true);
    anv_sll_destroy(list3, true);
    return TEST_SUCCESS;
}

int test_filter(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);

    // Add numbers 0-9
    for (int i = 0; i < 10; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_sll_push_back(list, val);
    }

    // Filter for even numbers
    ANVSinglyLinkedList* filtered = anv_sll_filter(list, is_even);
    ASSERT_NOT_NULL(filtered);
    ASSERT_EQ(filtered->size, 5); // Should contain 0,2,4,6,8

    // Verify filtered list
    const ANVSinglyLinkedNode* node = filtered->head;
    for (int i = 0; i < 5; i++)
    {
        const int expected_values[] = {0, 2, 4, 6, 8};
        ASSERT_EQ(*(int*)node->data, expected_values[i]);
        node = node->next;
    }

    // Make sure original list is unchanged
    ASSERT_EQ(list->size, 10);

    // Test empty list
    ANVSinglyLinkedList* empty_list = anv_sll_create(&alloc);
    ANVSinglyLinkedList* filtered_empty = anv_sll_filter(empty_list, is_even);
    ASSERT_NOT_NULL(filtered_empty);
    ASSERT_EQ(filtered_empty->size, 0);

    // Test null cases
    ASSERT_NULL(anv_sll_filter(NULL, is_even));
    ASSERT_NULL(anv_sll_filter(list, NULL));

    anv_sll_destroy(list, true);
    anv_sll_destroy(filtered, false);
    anv_sll_destroy(empty_list, false);
    anv_sll_destroy(filtered_empty, false);
    return TEST_SUCCESS;
}

int test_filter_deep(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);

    // Add numbers 0-9
    for (int i = 0; i < 10; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_sll_push_back(list, val);
    }

    // Deep-filter for even numbers
    ANVSinglyLinkedList* filtered = anv_sll_filter_deep(list, is_even);
    ASSERT_NOT_NULL(filtered);
    ASSERT_EQ(filtered->size, 5); // Should contain 0,2,4,6,8

    // Verify filtered list values and that data pointers are different (deep copy)
    const ANVSinglyLinkedNode* orig = list->head;
    const ANVSinglyLinkedNode* node = filtered->head;
    int idx = 0;
    while (node && orig)
    {
        const int expected_values[] = {0, 2, 4, 6, 8};
        // advance orig until next even
        while (orig && (*(int*)orig->data % 2) != 0)
            orig = orig->next;
        ASSERT_NOT_NULL(orig);
        ASSERT_EQ(*(int*)node->data, expected_values[idx]);
        // pointers must be different for deep copy
        ASSERT_NOT_EQ(orig->data, node->data);
        orig = orig->next;
        node = node->next;
        idx++;
    }

    // Modify original data and ensure filtered copy is unaffected
    if (list->head && list->head->data)
    {
        *(int*)list->head->data = 99; // change 0 -> 99
        const ANVSinglyLinkedNode* fnode = filtered->head;
        ASSERT_EQ(*(int*)fnode->data, 0);
    }

    anv_sll_destroy(list, true);
    anv_sll_destroy(filtered, true);
    return TEST_SUCCESS;
}

int test_transform(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);

    // Add numbers 1-5
    for (int i = 1; i <= 5; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_sll_push_back(list, val);
    }

    // Map to double each value
    ANVSinglyLinkedList* mapped = anv_sll_transform(list, double_value, true);
    ASSERT_NOT_NULL(mapped);
    ASSERT_EQ(mapped->size, 5);

    // Verify mapped list (should be 2,4,6,8,10)
    const ANVSinglyLinkedNode* node = mapped->head;
    for (int i = 1; i <= 5; i++)
    {
        ASSERT_EQ(*(int*)node->data, i * 2);
        node = node->next;
    }

    // Make sure original list is unchanged
    node = list->head;
    for (int i = 1; i <= 5; i++)
    {
        ASSERT_EQ(*(int*)node->data, i);
        node = node->next;
    }

    // Test empty list
    ANVSinglyLinkedList* empty_list = anv_sll_create(&alloc);
    ANVSinglyLinkedList* mapped_empty = anv_sll_transform(empty_list, double_value, true);
    ASSERT_NOT_NULL(mapped_empty);
    ASSERT_EQ(mapped_empty->size, 0);

    // Test null cases
    ASSERT_NULL(anv_sll_transform(NULL, double_value, true));
    ASSERT_NULL(anv_sll_transform(list, NULL, false));

    anv_sll_destroy(list, true);
    anv_sll_destroy(mapped, true);
    anv_sll_destroy(empty_list, false);
    anv_sll_destroy(mapped_empty, false);
    return TEST_SUCCESS;
}

int test_for_each(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);

    // Add numbers 1-5
    for (int i = 1; i <= 5; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_sll_push_back(list, val);
    }

    // Increment each value
    anv_sll_for_each(list, increment);

    // Verify each value is incremented
    const ANVSinglyLinkedNode* node = list->head;
    for (int i = 1; i <= 5; i++)
    {
        ASSERT_EQ(*(int*)node->data, i + 1);
        node = node->next;
    }

    // Test empty list
    ANVSinglyLinkedList* empty_list = anv_sll_create(&alloc);
    anv_sll_for_each(empty_list, increment); // Should do nothing

    // Test null cases
    anv_sll_for_each(NULL, increment); // Should do nothing
    anv_sll_for_each(list, NULL);      // Should do nothing

    anv_sll_destroy(list, true);
    anv_sll_destroy(empty_list, false);
    return TEST_SUCCESS;
}

//==============================================================================
// Iterator Tests
//==============================================================================

int test_forward_iterator(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);

    // Add numbers 1-5
    for (int i = 1; i <= 5; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_sll_push_back(list, val);
    }

    ANVIterator iter = anv_sll_iterator(list);
    ASSERT(iter.is_valid(&iter));

    // Test forward iteration
    int expected = 1;
    while (iter.has_next(&iter))
    {
        const int* val = (int*)iter.get(&iter);
        ASSERT_NOT_NULL(val);
        ASSERT_EQ(*val, expected);
        expected++;
        iter.next(&iter);
    }

    ASSERT_EQ(expected, 6); // Should have iterated through all 5 elements
    ASSERT(!iter.has_next(&iter));

    iter.destroy(&iter);
    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

int test_iterator_get(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);

    // Add numbers 1-3
    for (int i = 1; i <= 3; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_sll_push_back(list, val);
    }

    ANVIterator iter = anv_sll_iterator(list);

    // Test get without advancing
    const int* val = iter.get(&iter);
    ASSERT_NOT_NULL(val);
    ASSERT_EQ(*val, 1);

    // Get again - should return same value
    val = iter.get(&iter);
    ASSERT_EQ(*val, 1);

    // Now advance and test get
    iter.next(&iter);
    val = iter.get(&iter);
    ASSERT_EQ(*val, 2);

    iter.destroy(&iter);
    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

int test_iterator_reset(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);

    // Add numbers 1-3
    for (int i = 1; i <= 3; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_sll_push_back(list, val);
    }

    ANVIterator iter = anv_sll_iterator(list);

    // Advance iterator
    iter.next(&iter);
    iter.next(&iter);

    // Reset and verify back at beginning
    iter.reset(&iter);
    const int* val = iter.get(&iter);
    ASSERT_EQ(*val, 1);
    ASSERT(iter.has_next(&iter));

    iter.destroy(&iter);
    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

int test_iterator_empty_list(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);

    ANVIterator iter = anv_sll_iterator(list);
    ASSERT(iter.is_valid(&iter));
    ASSERT(!iter.has_next(&iter));
    ASSERT(!iter.has_prev(&iter));   // SLL doesn't support prev
    ASSERT_EQ(iter.next(&iter), -1); // Should return error code
    ASSERT_NULL(iter.get(&iter));

    iter.destroy(&iter);
    anv_sll_destroy(list, false);
    return TEST_SUCCESS;
}

int test_iterator_single_element(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);

    int* val = malloc(sizeof(int));
    *val = 42;
    anv_sll_push_back(list, val);

    ANVIterator iter = anv_sll_iterator(list);

    ASSERT(iter.has_next(&iter));
    ASSERT(!iter.has_prev(&iter)); // SLL doesn't support prev

    const int* retrieved = iter.get(&iter);
    ASSERT_EQ(*retrieved, 42);

    iter.next(&iter);
    ASSERT(!iter.has_next(&iter));
    ASSERT(!iter.has_prev(&iter)); // SLL doesn't support prev

    iter.destroy(&iter);
    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

// Test creating singly linked list from iterator
int test_from_iterator(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create a range iterator (0, 1, 2, 3, 4)
    ANVIterator range_it = anv_iterator_range(&alloc, 0, 5, 1);

    // Create singly linked list from iterator
    ANVSinglyLinkedList* list = anv_sll_from_iterator(&range_it, &alloc, true);
    ASSERT_NOT_NULL(list);
    ASSERT_EQ(anv_sll_size(list), 5);

    // Clean up the iterator immediately after use
    range_it.destroy(&range_it);

    // Verify singly linked list has correct values in sequential order
    // Iterator gives 0,1,2,3,4 and singly linked list should have them as 0,1,2,3,4 (head to tail)
    const ANVSinglyLinkedNode* node = list->head;
    for (int expected = 0; expected < 5; expected++)
    {
        ASSERT_NOT_NULL(node);
        ASSERT_EQ(*(int*)node->data, expected);
        node = node->next;
    }

    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

// Test iterator with invalid singly linked list
int test_iterator_invalid(void)
{
    const ANVIterator iter = anv_sll_iterator(NULL);
    ASSERT(!iter.is_valid(&iter));
    return TEST_SUCCESS;
}

// Test iterator state after singly linked list modifications
int test_iterator_modification(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);

    // Add initial data
    for (int i = 0; i < 3; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i * 10;
        ASSERT_EQ(anv_sll_push_back(list, data), 0);
    }

    ANVIterator iter = anv_sll_iterator(list);
    ASSERT(iter.is_valid(&iter));

    // Get first element
    void* first = iter.get(&iter);
    ASSERT_EQ(*(int*)first, 0); // Should be first element (0*10)
    iter.next(&iter);

    // Modify singly linked list while iterator exists (implementation detail: iterator may become invalid)
    int* new_data = malloc(sizeof(int));
    *new_data = 999;
    ASSERT_EQ(anv_sll_push_back(list, new_data), 0);

    // Iterator should still be valid but may not reflect new state
    ASSERT(iter.is_valid(&iter));

    iter.destroy(&iter);
    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

// Test copy isolation - verify that copied elements are independent
int test_sll_copy_isolation(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create original data that we can modify
    const int original_values[] = {10, 20, 30};
    int* data_ptrs[3];

    // Create a source singly linked list
    ANVSinglyLinkedList* source_list = anv_sll_create(&alloc);
    ASSERT_NOT_NULL(source_list);

    for (int i = 0; i < 3; i++)
    {
        data_ptrs[i] = malloc(sizeof(int));
        *data_ptrs[i] = original_values[i];
        ASSERT_EQ(anv_sll_push_back(source_list, data_ptrs[i]), 0);
    }

    ANVIterator list_it = anv_sll_iterator(source_list);
    ASSERT(list_it.is_valid(&list_it));

    // Create singly linked list with copying enabled
    ANVSinglyLinkedList* new_list = anv_sll_from_iterator(&list_it, &alloc, true);
    ASSERT_NOT_NULL(new_list);
    ASSERT_EQ(anv_sll_size(new_list), 3);

    // Modify original data
    *data_ptrs[0] = 999;
    *data_ptrs[1] = 888;
    *data_ptrs[2] = 777;

    // SinglyLinkedList should still have original values (proving data was copied)
    // Sequential order: 10, 20, 30
    const ANVSinglyLinkedNode* node = new_list->head;
    ASSERT_NOT_NULL(node);
    ASSERT_EQ(*(int*)node->data, 10); // Should be unchanged

    node = node->next;
    ASSERT_NOT_NULL(node);
    ASSERT_EQ(*(int*)node->data, 20); // Should be unchanged

    node = node->next;
    ASSERT_NOT_NULL(node);
    ASSERT_EQ(*(int*)node->data, 30); // Should be unchanged

    // Cleanup
    list_it.destroy(&list_it);
    anv_sll_destroy(new_list, true);
    anv_sll_destroy(source_list, true);

    return TEST_SUCCESS;
}

// Test that should_copy=true fails when allocator has no copy function
int test_sll_anv_copy_function_required(void)
{
    ANVAllocator alloc = anv_alloc_default();
    alloc.copy = NULL;

    ANVIterator range_it = anv_iterator_range(&alloc, 0, 3, 1);
    ASSERT(range_it.is_valid(&range_it));

    // Should return NULL because should_copy=true but no copy function available
    ANVSinglyLinkedList* list = anv_sll_from_iterator(&range_it, &alloc, true);
    ASSERT_NULL(list);

    range_it.destroy(&range_it);
    return TEST_SUCCESS;
}

// Test that should_copy=false uses elements directly without copying
int test_sll_from_iterator_no_copy(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create a range iterator and then a copy iterator to get actual owned data
    ANVIterator range_it = anv_iterator_range(&alloc, 0, 3, 1);
    ASSERT(range_it.is_valid(&range_it));

    // Use copy iterator to create actual data elements that we own
    ANVIterator copy_it = anv_iterator_copy(&range_it, &alloc, int_copy);
    ASSERT(copy_it.is_valid(&copy_it));

    // Create singly linked list without copying (should_copy = false)
    // This will use the copied elements directly from the copy iterator
    ANVSinglyLinkedList* list = anv_sll_from_iterator(&copy_it, &alloc, false);
    ASSERT_NOT_NULL(list);
    ASSERT_EQ(anv_sll_size(list), 3);

    // Verify values are correct (sequential order: 0, 1, 2)
    const ANVSinglyLinkedNode* node = list->head;
    ASSERT_NOT_NULL(node);
    ASSERT_EQ(*(int*)node->data, 0);

    node = node->next;
    ASSERT_NOT_NULL(node);
    ASSERT_EQ(*(int*)node->data, 1);

    node = node->next;
    ASSERT_NOT_NULL(node);
    ASSERT_EQ(*(int*)node->data, 2);

    range_it.destroy(&range_it);
    copy_it.destroy(&copy_it);
    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

// Test that iterator is exhausted after being consumed by anv_sll_from_iterator
int test_iterator_exhaustion_after_sll_creation(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVIterator range_it = anv_iterator_range(&alloc, 0, 5, 1);
    ASSERT(range_it.is_valid(&range_it));

    // Verify iterator starts with elements
    ASSERT(range_it.has_next(&range_it));

    // Create singly linked list from iterator (consumes all elements)
    ANVSinglyLinkedList* list = anv_sll_from_iterator(&range_it, &alloc, true);
    ASSERT_NOT_NULL(list);
    ASSERT_EQ(anv_sll_size(list), 5);

    // Iterator should now be exhausted
    ASSERT(!range_it.has_next(&range_it));
    ASSERT_NULL(range_it.get(&range_it));
    ASSERT_EQ(range_it.next(&range_it), -1); // Should fail to advance

    // But iterator should still be valid
    ASSERT(range_it.is_valid(&range_it));

    range_it.destroy(&range_it);
    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

// Test next() return values for proper error handling
int test_sll_iterator_next_return_values(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);
    ASSERT_NOT_NULL(list);

    // Add single element
    int* data = malloc(sizeof(int));
    *data = 42;
    ASSERT_EQ(anv_sll_push_back(list, data), 0);

    ANVIterator iter = anv_sll_iterator(list);
    ASSERT(iter.is_valid(&iter));

    // Should successfully advance once
    ASSERT(iter.has_next(&iter));
    ASSERT_EQ(iter.next(&iter), 0); // Success

    // Should fail to advance when exhausted
    ASSERT(!iter.has_next(&iter));
    ASSERT_EQ(iter.next(&iter), -1); // Failure

    // Additional calls should continue to fail
    ASSERT_EQ(iter.next(&iter), -1); // Still failure
    ASSERT(!iter.has_next(&iter));   // Still no elements

    iter.destroy(&iter);
    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

// Test various combinations of get/next/has_next calls for consistency
int test_sll_iterator_mixed_operations(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);
    ASSERT_NOT_NULL(list);

    // Add test data (will be in sequential order: 0, 10, 20)
    for (int i = 0; i < 3; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i * 10;
        ASSERT_EQ(anv_sll_push_back(list, data), 0);
    }

    ANVIterator iter = anv_sll_iterator(list);
    ASSERT(iter.is_valid(&iter));

    // Multiple get() calls should return same value
    void* data1 = iter.get(&iter);
    void* data2 = iter.get(&iter);
    ASSERT_NOT_NULL(data1);
    ASSERT_NOT_NULL(data2);
    ASSERT_EQ(data1, data2);               // Same pointer
    ASSERT_EQ(*(int*)data1, *(int*)data2); // Same value
    ASSERT_EQ(*(int*)data1, 0);            // First element should be 0

    // has_next should be consistent
    ASSERT(iter.has_next(&iter));
    ASSERT(iter.has_next(&iter)); // Multiple calls should be safe

    // Advance and verify new position
    ASSERT_EQ(iter.next(&iter), 0);
    void* data3 = iter.get(&iter);
    ASSERT_NOT_NULL(data3);
    // Note: data1 and data3 point to different singly linked list elements
    ASSERT_NOT_EQ(*(int*)data1, *(int*)data3); // Different values
    ASSERT_EQ(*(int*)data3, 10);               // Next element should be 10

    // Verify we can still advance
    ASSERT(iter.has_next(&iter));
    ASSERT_EQ(iter.next(&iter), 0);

    void* data4 = iter.get(&iter);
    ASSERT_NOT_NULL(data4);
    ASSERT_EQ(*(int*)data4, 20); // Last element should be 20

    // Now should be at end
    ASSERT_EQ(iter.next(&iter), 0); // Advance past last element
    ASSERT(!iter.has_next(&iter));
    ASSERT_NULL(iter.get(&iter));

    iter.destroy(&iter);
    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

// Test unsupported prev operations (SLL only supports forward iteration)
int test_sll_iterator_unsupported_operations(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);

    // Add test data
    for (int i = 1; i <= 3; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i;
        ASSERT_EQ(anv_sll_push_back(list, data), 0);
    }

    ANVIterator iter = anv_sll_iterator(list);
    ASSERT(iter.is_valid(&iter));

    // SLL iterator should not support bidirectional operations
    ASSERT(!iter.has_prev(&iter));
    ASSERT_EQ(iter.prev(&iter), -1); // Returns -1 for unsupported

    // Advance and test again
    iter.next(&iter);
    ASSERT(!iter.has_prev(&iter));
    ASSERT_EQ(iter.prev(&iter), -1); // Still unsupported

    // Reset should work
    iter.reset(&iter);

    // Should still be valid after unsupported operations
    ASSERT(iter.is_valid(&iter));

    iter.destroy(&iter);
    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

// Test iterator traversal order
int test_sll_iterator_order(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);

    // Add elements in specific order
    const int values[] = {100, 200, 300, 400, 500};
    for (int i = 0; i < 5; i++)
    {
        int* data = malloc(sizeof(int));
        *data = values[i];
        ASSERT_EQ(anv_sll_push_back(list, data), 0);
    }

    // Create iterator and verify order
    ANVIterator iter = anv_sll_iterator(list);

    for (int i = 0; i < 5; i++)
    {
        ASSERT(iter.has_next(&iter));
        void* data = iter.get(&iter);
        ASSERT_NOT_NULL(data);
        ASSERT_EQ(*(int*)data, values[i]);
        iter.next(&iter);
    }

    ASSERT(!iter.has_next(&iter));

    iter.destroy(&iter);
    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

// Test multiple iterators on the same list
int test_multiple_iterators(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVSinglyLinkedList* list = anv_sll_create(&alloc);

    for (int i = 0; i < 5; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_sll_push_back(list, val);
    }

    // Create two iterators on the same list
    ANVIterator iter1 = anv_sll_iterator(list);
    ANVIterator iter2 = anv_sll_iterator(list);

    // Advance first iterator by 2
    iter1.next(&iter1);
    iter1.next(&iter1);

    // Second iterator should still be at the beginning
    ASSERT_EQ(*(int*)iter2.get(&iter2), 0);

    // Both iterators should be independent
    ASSERT_EQ(*(int*)iter1.get(&iter1), 2);
    iter2.next(&iter2);
    ASSERT_EQ(*(int*)iter2.get(&iter2), 1);

    // Advance both and check
    iter1.next(&iter1);
    ASSERT_EQ(*(int*)iter1.get(&iter1), 3);
    iter2.next(&iter2);
    ASSERT_EQ(*(int*)iter2.get(&iter2), 2);

    iter1.destroy(&iter1);
    iter2.destroy(&iter2);
    anv_sll_destroy(list, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Memory Tests
//==============================================================================

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

//==============================================================================
// Performance Tests
//==============================================================================

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
        const double insert_time = (double)(end - start) / CLOCKS_PER_SEC;
        printf("Insert %d elements: %.6f seconds\n", SIZE, insert_time);
        ASSERT_LT(insert_time, 5.0);

        // Measure search time for last element
        start = clock();
        int key = SIZE - 1;
        const ANVSinglyLinkedNode* found = anv_sll_find(list, &key, int_cmp);
        end = clock();
        const double find_time = (double)(end - start) / CLOCKS_PER_SEC;
        printf("Find last element in %d elements: %.6f seconds\n", SIZE, find_time);
        ASSERT_LT(find_time, 5.0);
        ASSERT_NOT_NULL(found);

        // Cleanup
        anv_sll_destroy(list, true);
    }

    return TEST_SUCCESS;
}

//==============================================================================
// Property Tests
//==============================================================================

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

//==============================================================================
// Main - Combined Test Runner (73 tests total)
//==============================================================================

const ANVTestCase tests[] = {
    // CRUD Tests (22)
    {test_create_destroy, "test_create_destroy"},
    {test_insert_front_back_find, "test_insert_front_back_find"},
    {test_remove, "test_remove"},
    {test_remove_not_found, "test_remove_not_found"},
    {test_NULL_handling, "test_NULL_handling"},
    {test_insert_at, "test_insert_at"},
    {test_remove_at, "test_remove_at"},
    {test_remove_at_head, "test_remove_at_head"},
    {test_remove_at_last, "test_remove_at_last"},
    {test_remove_at_invalid, "test_remove_at_invalid"},
    {test_remove_at_empty, "test_remove_at_empty"},
    {test_remove_at_single_element, "test_remove_at_single_element"},
    {test_remove_at_single_element_invalid_pos, "test_remove_at_single_element_invalid_pos"},
    {test_insert_at_out_of_bounds, "test_insert_at_out_of_bounds"},
    {test_insert_remove_null_data, "test_insert_remove_null_data"},
    {test_mixed_operations_integrity, "test_mixed_operations_integrity"},
    {test_size, "test_size"},
    {test_is_empty, "test_is_empty"},
    {test_complex_data_type, "test_complex_data_type"},
    {test_remove_all, "test_remove_all"},
    {test_remove_front, "test_remove_front"},
    {test_remove_back, "test_remove_back"},

    // Algorithm Tests (17)
    {test_sort_empty, "test_sort_empty"},
    {test_sort_already_sorted, "test_sort_already_sorted"},
    {test_sort_reverse_order, "test_sort_reverse_order"},
    {test_sort_random_order, "test_sort_random_order"},
    {test_sort_with_duplicates, "test_sort_with_duplicates"},
    {test_sort_large_list, "test_sort_large_list"},
    {test_sort_custom_compare, "test_sort_custom_compare"},
    {test_sort_null_args, "test_sort_null_args"},
    {test_sort_stability, "test_sort_stability"},
    {test_reverse, "test_reverse"},
    {test_merge, "test_merge"},
    {test_splice, "test_splice"},
    {test_equals, "test_equals"},
    {test_filter, "test_filter"},
    {test_filter_deep, "test_filter_deep"},
    {test_transform, "test_transform"},
    {test_for_each, "test_for_each"},

    // Iterator Tests (17)
    {test_forward_iterator, "test_forward_iterator"},
    {test_iterator_get, "test_iterator_get"},
    {test_iterator_reset, "test_iterator_reset"},
    {test_iterator_empty_list, "test_iterator_empty_list"},
    {test_iterator_single_element, "test_iterator_single_element"},
    {test_from_iterator, "test_from_iterator"},
    {test_iterator_invalid, "test_iterator_invalid"},
    {test_iterator_modification, "test_iterator_modification"},
    {test_sll_copy_isolation, "test_sll_copy_isolation"},
    {test_sll_anv_copy_function_required, "test_sll_anv_copy_function_required"},
    {test_sll_from_iterator_no_copy, "test_sll_from_iterator_no_copy"},
    {test_iterator_exhaustion_after_sll_creation, "test_iterator_exhaustion_after_sll_creation"},
    {test_sll_iterator_next_return_values, "test_sll_iterator_next_return_values"},
    {test_sll_iterator_mixed_operations, "test_sll_iterator_mixed_operations"},
    {test_sll_iterator_unsupported_operations, "test_sll_iterator_unsupported_operations"},
    {test_sll_iterator_order, "test_sll_iterator_order"},
    {test_multiple_iterators, "test_multiple_iterators"},

    // Memory Tests (12)
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

    // Performance Tests (2)
    {test_stress, "test_stress"},
    {test_performance, "test_performance"},

    // Property Tests (3)
    {test_sll_size_after_insert_and_remove, "test_sll_size_after_insert_and_remove"},
    {test_sll_sort_is_idempotent, "test_sll_sort_is_idempotent"},
    {test_sll_reverse_is_involution, "test_sll_reverse_is_involution"},
};

int main(void)
{
    return anv_run_tests("SinglyLinkedList", tests, sizeof(tests) / sizeof(tests[0]));
}
