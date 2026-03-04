#include <stdlib.h>
#include <time.h>

#include <anvil/testing.h>
#include "TestHelpers.h"
#include "containers/doublylinkedlist.h"

//==============================================================================
// CRUD Tests
//==============================================================================

static int test_create_destroy(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    ASSERT_NOT_NULL(list);
    ASSERT_EQ(list->size, 0);
    ASSERT_NULL(list->head);
    ASSERT_NULL(list->tail);
    anv_dll_destroy(list, false);
    return TEST_SUCCESS;
}

static int test_push_front(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    MAKE_INT(a, 1);
    MAKE_INT(b, 2);
    ASSERT_EQ(anv_dll_push_front(list, b), 0);
    ASSERT_EQ(anv_dll_push_front(list, a), 0);
    ASSERT_EQ(list->size, 2);
    ASSERT_EQ(*(int*)list->head->data, 1);
    ASSERT_EQ(*(int*)list->tail->data, 2);
    ASSERT_NULL(list->head->prev);
    ASSERT_NULL(list->tail->next);

    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_push_back(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    MAKE_INT(a, 1);
    MAKE_INT(b, 2);
    ASSERT_EQ(anv_dll_push_back(list, a), 0);
    ASSERT_EQ(anv_dll_push_back(list, b), 0);
    ASSERT_EQ(list->size, 2);
    ASSERT_EQ(*(int*)list->head->data, 1);
    ASSERT_EQ(*(int*)list->tail->data, 2);
    ASSERT_NOT_NULL(list->head->next);
    ASSERT_NOT_NULL(list->tail->prev);

    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_find(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    MAKE_INT(a, 1);
    MAKE_INT(b, 2);
    MAKE_INT(c, 3);
    anv_dll_push_back(list, a);
    anv_dll_push_back(list, b);
    anv_dll_push_back(list, c);

    const int key = 2;
    const ANVDoublyLinkedNode* found = anv_dll_find(list, &key, int_cmp);
    ASSERT_NOT_NULL(found);
    ASSERT_EQ(*(int*)found->data, 2);

    const int missing_key = 99;
    ASSERT_NULL(anv_dll_find(list, &missing_key, int_cmp));

    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_remove(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    MAKE_INT(a, 1);
    MAKE_INT(b, 2);
    MAKE_INT(c, 3);
    anv_dll_push_back(list, a);
    anv_dll_push_back(list, b);
    anv_dll_push_back(list, c);

    const int key = 2;
    ASSERT_EQ(anv_dll_remove(list, &key, int_cmp, true), 0);
    ASSERT_EQ(list->size, 2);
    ANVDoublyLinkedNode* found = anv_dll_find(list, &key, int_cmp);
    ASSERT_NULL(found);

    // Verify links are properly maintained
    ASSERT_EQ(*(int*)list->head->data, 1);
    ASSERT_EQ(*(int*)list->head->next->data, 3);
    ASSERT_EQ(*(int*)list->tail->data, 3);
    ASSERT_EQ(list->tail->prev, list->head);

    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_remove_not_found(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    MAKE_INT(a, 1);
    anv_dll_push_back(list, a);

    const int key = 99;
    ASSERT_EQ(anv_dll_remove(list, &key, int_cmp, true), -1);

    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_nullptr_handling(void)
{
    ASSERT_EQ(anv_dll_push_back(NULL, NULL), -1);
    ASSERT_EQ(anv_dll_push_front(NULL, NULL), -1);
    ASSERT_EQ_PTR(anv_dll_find(NULL, NULL, NULL), NULL);
    ASSERT_EQ(anv_dll_remove(NULL, NULL, NULL, false), -1);
    anv_dll_destroy(NULL, false); // Should not crash
    return TEST_SUCCESS;
}

static int test_insert_at(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    MAKE_INT(a, 1);
    MAKE_INT(b, 2);
    MAKE_INT(c, 3);
    ASSERT_EQ(anv_dll_push_back(list, a), 0);    // [1]
    ASSERT_EQ(anv_dll_push_back(list, c), 0);    // [1,3]
    ASSERT_EQ(anv_dll_insert_at(list, 1, b), 0); // [1,2,3]
    ASSERT_EQ(list->size, 3);

    const int key = 2;
    const ANVDoublyLinkedNode* found = anv_dll_find(list, &key, int_cmp);
    ASSERT_NOT_NULL(found);
    ASSERT_EQ(*(int*)found->data, 2);

    // Verify prev/next links
    ASSERT_EQ(found->prev, list->head);
    ASSERT_EQ(found->next, list->tail);
    ASSERT_EQ(list->head->next, found);
    ASSERT_EQ(list->tail->prev, found);

    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_remove_at(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    MAKE_INT(a, 10);
    MAKE_INT(b, 20);
    MAKE_INT(c, 30);
    anv_dll_push_back(list, a); // [10]
    anv_dll_push_back(list, b); // [10,20]
    anv_dll_push_back(list, c); // [10,20,30]

    ASSERT_EQ(anv_dll_remove_at(list, 1, true), 0); // remove 20 (free data)
    ASSERT_EQ(list->size, 2);

    const int key = 20;
    ANVDoublyLinkedNode* found = anv_dll_find(list, &key, int_cmp);
    ASSERT_NULL(found);

    // Check links
    ASSERT_EQ(list->head->next, list->tail);
    ASSERT_EQ(list->tail->prev, list->head);

    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_remove_front(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);

    // Test on empty list
    ASSERT_EQ(anv_dll_pop_front(list, true), -1);

    // Add elements
    MAKE_INT(a, 10);
    MAKE_INT(b, 20);
    MAKE_INT(c, 30);
    anv_dll_push_back(list, a);
    anv_dll_push_back(list, b);
    anv_dll_push_back(list, c);
    ASSERT_EQ(list->size, 3);

    // Remove front
    ASSERT_EQ(anv_dll_pop_front(list, true), 0);
    ASSERT_EQ(list->size, 2);

    // Check first element is now 20
    int key = 10;
    ASSERT_NULL(anv_dll_find(list, &key, int_cmp));
    key = 20;
    ASSERT_NOT_NULL(anv_dll_find(list, &key, int_cmp));
    ASSERT_EQ(list->head->data, anv_dll_find(list, &key, int_cmp)->data);
    ASSERT_NULL(list->head->prev);

    // Remove until empty
    ASSERT_EQ(anv_dll_pop_front(list, true), 0);
    ASSERT_EQ(anv_dll_pop_front(list, true), 0);
    ASSERT_EQ(list->size, 0);
    ASSERT_NULL(list->head);
    ASSERT_NULL(list->tail);

    anv_dll_destroy(list, false);
    return TEST_SUCCESS;
}

static int test_remove_back(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);

    // Test on empty list
    ASSERT_EQ(anv_dll_pop_back(list, true), -1);

    // Test on single element list
    MAKE_INT(a, 10);
    anv_dll_push_back(list, a);
    ASSERT_EQ(anv_dll_pop_back(list, true), 0);
    ASSERT_EQ(list->size, 0);
    ASSERT_NULL(list->head);
    ASSERT_NULL(list->tail);

    // Test with multiple elements
    MAKE_INT(b, 20);
    MAKE_INT(c, 30);
    MAKE_INT(d, 40);
    anv_dll_push_back(list, b);
    anv_dll_push_back(list, c);
    anv_dll_push_back(list, d);
    ASSERT_EQ(list->size, 3);

    // Remove back
    ASSERT_EQ(anv_dll_pop_back(list, true), 0);
    ASSERT_EQ(list->size, 2);

    // Check last element was removed
    int key = 40;
    ASSERT_NULL(anv_dll_find(list, &key, int_cmp));
    key = 30;
    ASSERT_NOT_NULL(anv_dll_find(list, &key, int_cmp));
    ASSERT_EQ(list->tail->data, anv_dll_find(list, &key, int_cmp)->data);
    ASSERT_NULL(list->tail->next);

    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_remove_at_head(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    MAKE_INT(a, 100);
    MAKE_INT(b, 200);
    anv_dll_push_back(list, a); // [100]
    anv_dll_push_back(list, b); // [100,200]

    ASSERT_EQ(anv_dll_remove_at(list, 0, true), 0); // remove head (100)
    ASSERT_EQ(list->size, 1);
    ASSERT_EQ(*(int*)list->head->data, 200);
    ASSERT_EQ(list->head, list->tail);

    const int key = 100;
    ANVDoublyLinkedNode* found = anv_dll_find(list, &key, int_cmp);
    ASSERT_NULL(found);

    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_remove_at_last(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    MAKE_INT(a, 1);
    MAKE_INT(b, 2);
    MAKE_INT(c, 3);
    anv_dll_push_back(list, a); // [1]
    anv_dll_push_back(list, b); // [1,2]
    anv_dll_push_back(list, c); // [1,2,3]

    ASSERT_EQ(anv_dll_remove_at(list, 2, true), 0); // remove last (3)
    ASSERT_EQ(list->size, 2);
    ASSERT_EQ(*(int*)list->tail->data, 2);

    const int key = 3;
    ANVDoublyLinkedNode* found = anv_dll_find(list, &key, int_cmp);
    ASSERT_NULL(found);

    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_remove_at_invalid(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    MAKE_INT(a, 1);
    anv_dll_push_back(list, a); // [1]

    ASSERT_EQ(anv_dll_remove_at(list, 5, true), -1);          // invalid position
    ASSERT_EQ(anv_dll_remove_at(list, (size_t)-1, true), -1); // negative as size_t (very large)

    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_remove_at_empty(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    ASSERT_EQ(anv_dll_remove_at(list, 0, true), -1); // nothing to remove
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_remove_at_single_element(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    MAKE_INT(a, 123);
    anv_dll_push_back(list, a);                     // [123]
    ASSERT_EQ(anv_dll_remove_at(list, 0, true), 0); // remove only element
    ASSERT_EQ(list->size, 0);
    ASSERT_NULL(list->head);
    ASSERT_NULL(list->tail);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_remove_at_single_element_invalid_pos(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    MAKE_INT(a, 123);
    anv_dll_push_back(list, a);                      // [123]
    ASSERT_EQ(anv_dll_remove_at(list, 1, true), -1); // invalid position
    ASSERT_EQ(list->size, 1);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_insert_at_out_of_bounds(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    MAKE_INT(a, 1);
    ASSERT_EQ(anv_dll_insert_at(list, 2, a), -1);          // out of bounds (list size is 0)
    ASSERT_EQ(anv_dll_insert_at(list, (size_t)-1, a), -1); // very large index
    anv_dll_destroy(list, true);
    free(a);
    return TEST_SUCCESS;
}

static int test_insert_remove_null_data(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    ASSERT_EQ(anv_dll_push_back(list, NULL), 0); // allow NULL data
    ASSERT_EQ(list->size, 1);
    ASSERT_EQ(anv_dll_remove_at(list, 0, false), 0); // remove node with NULL data, no free
    ASSERT_EQ(list->size, 0);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_mixed_operations_integrity(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    MAKE_INT(a, 10);
    MAKE_INT(b, 20);
    MAKE_INT(c, 30);
    anv_dll_push_back(list, a);    // [10]
    anv_dll_push_front(list, b);   // [20,10]
    anv_dll_insert_at(list, 1, c); // [20,30,10]
    ASSERT_EQ(list->size, 3);

    // Verify the bidirectional links
    ASSERT_EQ(*(int*)list->head->data, 20);
    ASSERT_EQ(*(int*)list->head->next->data, 30);
    ASSERT_EQ(*(int*)list->head->next->next->data, 10);
    ASSERT_EQ(*(int*)list->tail->data, 10);
    ASSERT_EQ(*(int*)list->tail->prev->data, 30);
    ASSERT_EQ(*(int*)list->tail->prev->prev->data, 20);

    ASSERT_EQ(anv_dll_remove_at(list, 1, true), 0); // remove 30, [20,10]
    const int key = 30;
    ANVDoublyLinkedNode* found = anv_dll_find(list, &key, int_cmp);
    ASSERT_NULL(found);

    // Verify new links
    ASSERT_EQ(*(int*)list->head->data, 20);
    ASSERT_EQ(*(int*)list->head->next->data, 10);
    ASSERT_EQ(*(int*)list->tail->data, 10);
    ASSERT_EQ(*(int*)list->tail->prev->data, 20);
    ASSERT_NULL(list->head->prev);
    ASSERT_NULL(list->tail->next);

    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_size(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    ASSERT_EQ(anv_dll_size(list), 0);

    MAKE_INT(a, 10);
    MAKE_INT(b, 20);
    anv_dll_push_back(list, a);
    ASSERT_EQ(anv_dll_size(list), 1);
    anv_dll_push_back(list, b);
    ASSERT_EQ(anv_dll_size(list), 2);

    anv_dll_remove_at(list, 0, true);
    ASSERT_EQ(anv_dll_size(list), 1);

    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_is_empty(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    ASSERT_EQ(anv_dll_is_empty(list), 1); // Empty list

    MAKE_INT(a, 10);
    anv_dll_push_back(list, a);
    ASSERT_EQ(anv_dll_is_empty(list), 0); // Non-empty list

    anv_dll_remove_at(list, 0, true);
    ASSERT_EQ(anv_dll_is_empty(list), 1); // Empty again

    ASSERT_EQ(anv_dll_is_empty(NULL), 1); // NULL list should be considered empty

    anv_dll_destroy(list, false);
    return TEST_SUCCESS;
}

static int test_complex_data_type(void)
{
    ANVAllocator alloc = create_person_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);

    Person* p1 = create_person("Alice", 30);
    Person* p2 = create_person("Bob", 25);
    Person* p3 = create_person("Charlie", 40);

    anv_dll_push_back(list, p1);
    anv_dll_push_back(list, p2);
    anv_dll_push_back(list, p3);
    ASSERT_EQ(list->size, 3);

    Person search_key;
    strcpy(search_key.name, "Bob");
    search_key.age = 0; // Age doesn't matter for comparison

    const ANVDoublyLinkedNode* found = anv_dll_find(list, &search_key, person_cmp);
    ASSERT_NOT_NULL(found);
    const Person* found_person = found->data;
    ASSERT_EQ(found_person->age, 25);

    // Clean up
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_remove_all(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);

    // Add 10 elements
    for (int i = 0; i < 10; i++)
    {
        MAKE_INT(val, i);
        anv_dll_push_back(list, val);
    }
    ASSERT_EQ(list->size, 10);

    // Remove all elements one by one
    while (!anv_dll_is_empty(list))
    {
        anv_dll_pop_front(list, true);
    }

    ASSERT_EQ(list->size, 0);
    ASSERT_NULL(list->head);
    ASSERT_NULL(list->tail);

    anv_dll_destroy(list, false); // Already freed all data
    return TEST_SUCCESS;
}

//==============================================================================
// Algorithm Tests
//==============================================================================

static int test_sort_empty(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    ASSERT_EQ(anv_dll_sort(list, int_cmp), 0); // Empty list is already sorted
    ASSERT_EQ(list->size, 0);
    anv_dll_destroy(list, false);
    return TEST_SUCCESS;
}

static int test_sort_already_sorted(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    for (int i = 0; i < 5; i++)
    {
        MAKE_INT(val, i);
        anv_dll_push_back(list, val);
    }

    ASSERT_EQ(anv_dll_sort(list, int_cmp), 0);

    // Verify order
    const ANVDoublyLinkedNode* node = list->head;
    for (int i = 0; i < 5; i++)
    {
        ASSERT_EQ(*(int*)node->data, i);
        node = node->next;
    }

    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_sort_reverse_order(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    for (int i = 4; i >= 0; i--)
    {
        MAKE_INT(val, i);
        anv_dll_push_back(list, val);
    }

    ASSERT_EQ(anv_dll_sort(list, int_cmp), 0);

    // Verify order
    const ANVDoublyLinkedNode* node = list->head;
    for (int i = 0; i < 5; i++)
    {
        ASSERT_EQ(*(int*)node->data, i);
        node = node->next;
    }

    // Check bidirectional links
    const ANVDoublyLinkedNode* tail = list->tail;
    for (int i = 4; i >= 0; i--)
    {
        ASSERT_EQ(*(int*)tail->data, i);
        tail = tail->prev;
    }

    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_sort_with_duplicates(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    const int values[] = {5, 2, 9, 5, 7, 2, 9, 5};
    const size_t count = sizeof(values) / sizeof(values[0]);

    for (size_t i = 0; i < count; i++)
    {
        MAKE_INT(val, values[i]);
        anv_dll_push_back(list, val);
    }

    ASSERT_EQ(anv_dll_sort(list, int_cmp), 0);

    // Verify order
    const ANVDoublyLinkedNode* node = list->head;
    for (size_t i = 0; i < count; i++)
    {
        const int sorted[] = {2, 2, 5, 5, 5, 7, 9, 9};
        ASSERT_EQ(*(int*)node->data, sorted[i]);
        node = node->next;
    }

    // Check bidirectional links
    const ANVDoublyLinkedNode* tail = list->tail;
    for (size_t i = 0; i < count; i++)
    {
        const int sorted_reverse[] = {9, 9, 7, 5, 5, 5, 2, 2};
        ASSERT_EQ(*(int*)tail->data, sorted_reverse[i]);
        tail = tail->prev;
    }

    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_sort_large_list(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    const int SIZE = 1000;

    // Insert in reverse order
    for (int i = SIZE - 1; i >= 0; i--)
    {
        MAKE_INT(val, i);
        anv_dll_push_back(list, val);
    }

    const clock_t start = clock();
    ASSERT_EQ(anv_dll_sort(list, int_cmp), 0);
    const clock_t end = clock();
    printf("DLL Sort %d elements: %.6f seconds\n", SIZE,
           (double)(end - start) / CLOCKS_PER_SEC);

    // Verify order (first few and last few)
    const ANVDoublyLinkedNode* node = list->head;
    for (int i = 0; i < 10; i++)
    {
        ASSERT_EQ(*(int*)node->data, i);
        node = node->next;
    }

    node = list->tail;
    for (int i = SIZE - 1; i >= SIZE - 10; i--)
    {
        ASSERT_EQ(*(int*)node->data, i);
        node = node->prev;
    }

    // Verify list structure
    ASSERT_EQ(list->size, (size_t)SIZE);
    ASSERT_EQ(*(int*)list->head->data, 0);
    ASSERT_EQ(*(int*)list->tail->data, SIZE - 1);

    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_sort_custom_compare(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    for (int i = 0; i < 5; i++)
    {
        MAKE_INT(val, i);
        anv_dll_push_back(list, val);
    }

    // Sort in descending order
    ASSERT_EQ(anv_dll_sort(list, int_cmp_desc), 0);

    // Verify order
    const ANVDoublyLinkedNode* node = list->head;
    for (int i = 4; i >= 0; i--)
    {
        ASSERT_EQ(*(int*)node->data, i);
        node = node->next;
    }

    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_sort_null_args(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    ASSERT_EQ(anv_dll_sort(NULL, int_cmp), -1); // NULL list
    ASSERT_EQ(anv_dll_sort(list, NULL), -1);    // NULL compare function
    anv_dll_destroy(list, false);
    return TEST_SUCCESS;
}

static int test_sort_stability(void)
{
    ANVAllocator alloc = create_person_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);

    // Person structs with same name (for comparison) but different ages
    Person* p1 = create_person("Alice", 30);
    Person* p2 = create_person("Alice", 25); // Same name, different age
    Person* p3 = create_person("Bob", 35);
    Person* p4 = create_person("Alice", 40); // Same name, different age

    anv_dll_push_back(list, p1);
    anv_dll_push_back(list, p2);
    anv_dll_push_back(list, p3);
    anv_dll_push_back(list, p4);

    // Sort by name only - ages should remain in insertion order for equal names
    ASSERT_EQ(anv_dll_sort(list, person_cmp), 0);

    // Verify order: All Alice's should come before Bob
    const ANVDoublyLinkedNode* node = list->head;
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

    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_reverse(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);

    // Test empty list
    ASSERT_EQ(anv_dll_reverse(list), 0);
    ASSERT_EQ(list->size, 0);

    // Test single element
    MAKE_INT(a, 10);
    anv_dll_push_back(list, a);
    ASSERT_EQ(anv_dll_reverse(list), 0);
    ASSERT_EQ(list->size, 1);
    ASSERT_EQ(*(int*)list->head->data, 10);
    ASSERT_EQ(list->head, list->tail);

    // Test multiple elements
    MAKE_INT(b, 20);
    MAKE_INT(c, 30);
    anv_dll_push_back(list, b);
    anv_dll_push_back(list, c);
    // List is now [10,20,30]

    ASSERT_EQ(anv_dll_reverse(list), 0);
    // List should now be [30,20,10]

    // Verify head-to-tail traversal
    const ANVDoublyLinkedNode* node = list->head;
    ASSERT_EQ(*(int*)node->data, 30);
    ASSERT_NULL(node->prev);

    node = node->next;
    ASSERT_EQ(*(int*)node->data, 20);
    ASSERT_EQ(*(int*)node->prev->data, 30);

    node = node->next;
    ASSERT_EQ(*(int*)node->data, 10);
    ASSERT_EQ(*(int*)node->prev->data, 20);
    ASSERT_NULL(node->next);
    ASSERT_EQ(node, list->tail);

    // Verify tail-to-head traversal
    node = list->tail;
    ASSERT_EQ(*(int*)node->data, 10);
    ASSERT_NULL(node->next);

    node = node->prev;
    ASSERT_EQ(*(int*)node->data, 20);
    ASSERT_EQ(*(int*)node->next->data, 10);

    node = node->prev;
    ASSERT_EQ(*(int*)node->data, 30);
    ASSERT_EQ(*(int*)node->next->data, 20);
    ASSERT_NULL(node->prev);
    ASSERT_EQ(node, list->head);

    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_merge(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list1 = anv_dll_create(&alloc);
    ANVDoublyLinkedList* list2 = anv_dll_create(&alloc);

    // Test merging empty lists
    ASSERT_EQ(anv_dll_merge(list1, list2), 0);
    ASSERT_EQ(list1->size, 0);
    ASSERT_EQ(list2->size, 0);

    // Test merging empty with non-empty
    MAKE_INT(a1, 10);
    MAKE_INT(b1, 20);
    anv_dll_push_back(list2, a1);
    anv_dll_push_back(list2, b1);

    ASSERT_EQ(anv_dll_merge(list1, list2), 0);
    ASSERT_EQ(list1->size, 2);
    ASSERT_EQ(list2->size, 0);
    ASSERT_NULL(list2->head);
    ASSERT_NULL(list2->tail);

    // Verify the merged list
    ASSERT_EQ(*(int*)list1->head->data, 10);
    ASSERT_EQ(*(int*)list1->tail->data, 20);
    ASSERT_NULL(list1->head->prev);
    ASSERT_NULL(list1->tail->next);
    ASSERT_EQ(list1->head->next, list1->tail);
    ASSERT_EQ(list1->tail->prev, list1->head);

    // Test merging two non-empty lists
    ANVDoublyLinkedList* list3 = anv_dll_create(&alloc);
    MAKE_INT(a2, 30);
    MAKE_INT(b2, 40);
    anv_dll_push_back(list3, a2);
    anv_dll_push_back(list3, b2);

    ASSERT_EQ(anv_dll_merge(list1, list3), 0);
    ASSERT_EQ(list1->size, 4);
    ASSERT_EQ(list3->size, 0);

    // Verify the final merged list
    ASSERT_EQ(*(int*)list1->head->data, 10);
    ASSERT_EQ(*(int*)list1->tail->data, 40);

    const ANVDoublyLinkedNode* node = list1->head;
    ASSERT_EQ(*(int*)node->data, 10);
    node = node->next;
    ASSERT_EQ(*(int*)node->data, 20);
    ASSERT_EQ(*(int*)node->prev->data, 10);
    node = node->next;
    ASSERT_EQ(*(int*)node->data, 30);
    ASSERT_EQ(*(int*)node->prev->data, 20);
    node = node->next;
    ASSERT_EQ(*(int*)node->data, 40);
    ASSERT_EQ(*(int*)node->prev->data, 30);
    ASSERT_EQ(node, list1->tail);
    ASSERT_NULL(node->next);

    anv_dll_destroy(list1, true);
    anv_dll_destroy(list2, false); // Already empty
    anv_dll_destroy(list3, false); // Already empty
    return TEST_SUCCESS;
}

static int test_splice(void)
{
    ANVAllocator alloc = create_int_allocator();
    // Test splicing at the beginning
    ANVDoublyLinkedList* dest1 = anv_dll_create(&alloc);
    ANVDoublyLinkedList* src1 = anv_dll_create(&alloc);

    MAKE_INT(a1, 10);
    MAKE_INT(b1, 20);
    MAKE_INT(c1, 30);
    MAKE_INT(d1, 40);
    MAKE_INT(e1, 50);

    anv_dll_push_back(dest1, a1);
    anv_dll_push_back(dest1, b1);
    anv_dll_push_back(dest1, c1);
    anv_dll_push_back(src1, d1);
    anv_dll_push_back(src1, e1);

    ASSERT_EQ(anv_dll_splice(dest1, src1, 0), 0);
    ASSERT_EQ(dest1->size, 5);
    ASSERT_EQ(src1->size, 0);

    // Verify the spliced list
    ASSERT_EQ(*(int*)dest1->head->data, 40);
    ASSERT_EQ(*(int*)dest1->head->next->data, 50);
    ASSERT_EQ(*(int*)dest1->head->next->next->data, 10);
    ASSERT_EQ(*(int*)dest1->head->next->next->next->data, 20);
    ASSERT_EQ(*(int*)dest1->tail->data, 30);

    // Test bidirectional links
    ASSERT_NULL(dest1->head->prev);
    ASSERT_EQ(*(int*)dest1->head->next->prev->data, 40);
    ASSERT_EQ(*(int*)dest1->tail->prev->data, 20);
    ASSERT_NULL(dest1->tail->next);

    // Test splicing in the middle
    ANVDoublyLinkedList* dest2 = anv_dll_create(&alloc);
    ANVDoublyLinkedList* src2 = anv_dll_create(&alloc);

    MAKE_INT(a2, 10);
    MAKE_INT(b2, 20);
    MAKE_INT(c2, 30);
    MAKE_INT(d2, 40);
    MAKE_INT(e2, 50);

    anv_dll_push_back(dest2, a2);
    anv_dll_push_back(dest2, b2);
    anv_dll_push_back(dest2, c2);
    anv_dll_push_back(src2, d2);
    anv_dll_push_back(src2, e2);

    ASSERT_EQ(anv_dll_splice(dest2, src2, 1), 0);
    ASSERT_EQ(dest2->size, 5);
    ASSERT_EQ(src2->size, 0);

    // Verify the spliced list
    ASSERT_EQ(*(int*)dest2->head->data, 10);
    ASSERT_EQ(*(int*)dest2->head->next->data, 40);
    ASSERT_EQ(*(int*)dest2->head->next->next->data, 50);
    ASSERT_EQ(*(int*)dest2->head->next->next->next->data, 20);
    ASSERT_EQ(*(int*)dest2->tail->data, 30);

    // Test bidirectional links
    ASSERT_NULL(dest2->head->prev);
    ASSERT_EQ(*(int*)dest2->head->next->prev->data, 10);
    ASSERT_EQ(*(int*)dest2->tail->prev->data, 20);
    ASSERT_NULL(dest2->tail->next);

    // Test splicing at the end
    ANVDoublyLinkedList* dest3 = anv_dll_create(&alloc);
    ANVDoublyLinkedList* src3 = anv_dll_create(&alloc);

    MAKE_INT(a3, 10);
    MAKE_INT(b3, 20);
    MAKE_INT(c3, 30);
    MAKE_INT(d3, 40);
    MAKE_INT(e3, 50);

    anv_dll_push_back(dest3, a3);
    anv_dll_push_back(dest3, b3);
    anv_dll_push_back(dest3, c3);
    anv_dll_push_back(src3, d3);
    anv_dll_push_back(src3, e3);

    ASSERT_EQ(anv_dll_splice(dest3, src3, 3), 0);
    ASSERT_EQ(dest3->size, 5);
    ASSERT_EQ(src3->size, 0);

    // Verify the spliced list
    ASSERT_EQ(*(int*)dest3->head->data, 10);
    ASSERT_EQ(*(int*)dest3->head->next->data, 20);
    ASSERT_EQ(*(int*)dest3->head->next->next->data, 30);
    ASSERT_EQ(*(int*)dest3->head->next->next->next->data, 40);
    ASSERT_EQ(*(int*)dest3->tail->data, 50);

    // Test bidirectional links
    ASSERT_NULL(dest3->head->prev);
    ASSERT_EQ(*(int*)dest3->head->next->prev->data, 10);
    ASSERT_EQ(*(int*)dest3->tail->prev->data, 40);
    ASSERT_NULL(dest3->tail->next);

    // Test splicing with empty source
    ANVDoublyLinkedList* empty = anv_dll_create(&alloc);
    ASSERT_EQ(anv_dll_splice(dest1, empty, 2), 0);
    ASSERT_EQ(dest1->size, 5); // Should be unchanged

    // Test splicing with invalid position
    ASSERT_EQ(anv_dll_splice(dest1, src1, 99), -1);

    anv_dll_destroy(dest1, true);
    anv_dll_destroy(src1, false);
    anv_dll_destroy(dest2, true);
    anv_dll_destroy(src2, false);
    anv_dll_destroy(dest3, true);
    anv_dll_destroy(src3, false);
    anv_dll_destroy(empty, false);

    return TEST_SUCCESS;
}

static int test_equals(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list1 = anv_dll_create(&alloc);
    ANVDoublyLinkedList* list2 = anv_dll_create(&alloc);

    // Empty lists should be equal
    ASSERT_EQ(anv_dll_equals(list1, list2, int_cmp), 1);

    // Lists with same elements should be equal
    MAKE_INT(a1, 10);
    MAKE_INT(b1, 20);
    MAKE_INT(a2, 10);
    MAKE_INT(b2, 20);
    anv_dll_push_back(list1, a1);
    anv_dll_push_back(list1, b1);
    anv_dll_push_back(list2, a2);
    anv_dll_push_back(list2, b2);

    ASSERT_EQ(anv_dll_equals(list1, list2, int_cmp), 1);

    // Lists with different elements should not be equal
    MAKE_INT(c2, 30);
    anv_dll_push_back(list2, c2);

    ASSERT_EQ(anv_dll_equals(list1, list2, int_cmp), 0);

    // Lists with same size but different elements should not be equal
    ANVDoublyLinkedList* list3 = anv_dll_create(&alloc);
    MAKE_INT(a3, 10);
    MAKE_INT(b3, 30); // Different value
    anv_dll_push_back(list3, a3);
    anv_dll_push_back(list3, b3);

    ASSERT_EQ(anv_dll_equals(list1, list3, int_cmp), 0);

    // Error cases
    ASSERT_EQ(anv_dll_equals(NULL, list2, int_cmp), -1);
    ASSERT_EQ(anv_dll_equals(list1, NULL, int_cmp), -1);
    ASSERT_EQ(anv_dll_equals(list1, list2, NULL), -1);

    anv_dll_destroy(list1, true);
    anv_dll_destroy(list2, true);
    anv_dll_destroy(list3, true);
    return TEST_SUCCESS;
}

static int test_filter(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);

    // Add numbers 0-9
    for (int i = 0; i < 10; i++)
    {
        MAKE_INT(val, i);
        anv_dll_push_back(list, val);
    }

    // Filter for even numbers
    ANVDoublyLinkedList* filtered = anv_dll_filter(list, is_even);
    ASSERT_NOT_NULL(filtered);
    ASSERT_EQ(filtered->size, 5); // Should contain 0,2,4,6,8

    // Verify filtered list
    const ANVDoublyLinkedNode* node = filtered->head;
    for (int i = 0; i < 5; i++)
    {
        const int expected_values[] = {0, 2, 4, 6, 8};
        ASSERT_EQ(*(int*)node->data, expected_values[i]);
        node = node->next;
    }

    // Make sure original list is unchanged
    ASSERT_EQ(list->size, 10);

    // Test empty list
    ANVDoublyLinkedList* empty_list = anv_dll_create(&alloc);
    ANVDoublyLinkedList* filtered_empty = anv_dll_filter(empty_list, is_even);
    ASSERT_NOT_NULL(filtered_empty);
    ASSERT_EQ(filtered_empty->size, 0);

    // Test null cases
    ASSERT_NULL(anv_dll_filter(NULL, is_even));
    ASSERT_NULL(anv_dll_filter(list, NULL));

    anv_dll_destroy(list, true);
    anv_dll_destroy(filtered, false);
    anv_dll_destroy(empty_list, false);
    anv_dll_destroy(filtered_empty, false);
    return TEST_SUCCESS;
}

static int test_filter_deep(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);

    // Add numbers 0-9
    for (int i = 0; i < 10; i++)
    {
        MAKE_INT(val, i);
        anv_dll_push_back(list, val);
    }

    // Deep-filter for even numbers
    ANVDoublyLinkedList* filtered = anv_dll_filter_deep(list, is_even);
    ASSERT_NOT_NULL(filtered);
    ASSERT_EQ(filtered->size, 5); // 0,2,4,6,8

    // Verify values and deep-copy semantics (pointers differ)
    const ANVDoublyLinkedNode* orig = list->head;
    const ANVDoublyLinkedNode* node = filtered->head;
    int idx = 0;
    while (node && orig)
    {
        const int expected_values[] = {0, 2, 4, 6, 8};
        // advance orig to next matching even
        while (orig && (*(int*)orig->data % 2) != 0)
            orig = orig->next;
        ASSERT_NOT_NULL(orig);
        ASSERT_EQ(*(int*)node->data, expected_values[idx]);
        // deep copy -> different pointers
        ASSERT_NOT_EQ(orig->data, node->data);
        orig = orig->next;
        node = node->next;
        idx++;
    }

    // Mutate original and ensure filtered copy unchanged
    if (list->head && list->head->data)
    {
        *(int*)list->head->data = 99; // change 0 -> 99
        const ANVDoublyLinkedNode* fnode = filtered->head;
        ASSERT_EQ(*(int*)fnode->data, 0);
    }

    anv_dll_destroy(list, true);
    anv_dll_destroy(filtered, true);
    return TEST_SUCCESS;
}

static int test_transform(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);

    // Add numbers 1-5
    for (int i = 1; i <= 5; i++)
    {
        MAKE_INT(val, i);
        anv_dll_push_back(list, val);
    }

    // Map to double each value
    ANVDoublyLinkedList* transformed = anv_dll_transform(list, double_value, true);
    ASSERT_NOT_NULL(transformed);
    ASSERT_EQ(transformed->size, 5);

    // Verify mapped list (should be 2,4,6,8,10)
    const ANVDoublyLinkedNode* node = transformed->head;
    for (int i = 1; i <= 5; i++)
    {
        ASSERT_EQ(*(int*)node->data, i * 2);
        node = node->next;
    }

    // Make sure bidirectional links are correct
    node = transformed->head;
    ASSERT_NULL(node->prev);

    node = node->next;
    ASSERT_EQ(*(int*)node->prev->data, 2);

    node = transformed->tail;
    ASSERT_NULL(node->next);
    ASSERT_EQ(*(int*)node->data, 10);

    // Make sure original list is unchanged
    node = list->head;
    for (int i = 1; i <= 5; i++)
    {
        ASSERT_EQ(*(int*)node->data, i);
        node = node->next;
    }

    // Test empty list
    ANVDoublyLinkedList* empty_list = anv_dll_create(&alloc);
    ANVDoublyLinkedList* transformed_empty = anv_dll_transform(empty_list, double_value, true);
    ASSERT_NOT_NULL(transformed_empty);
    ASSERT_EQ(transformed_empty->size, 0);

    // Test null cases
    ASSERT_NULL(anv_dll_transform(NULL, double_value, true));
    ASSERT_NULL(anv_dll_transform(list, NULL, true));

    anv_dll_destroy(list, true);
    anv_dll_destroy(transformed, true);
    anv_dll_destroy(empty_list, false);
    anv_dll_destroy(transformed_empty, false);
    return TEST_SUCCESS;
}

static int test_for_each(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);

    // Add numbers 1-5
    for (int i = 1; i <= 5; i++)
    {
        MAKE_INT(val, i);
        anv_dll_push_back(list, val);
    }

    // Increment each value
    anv_dll_for_each(list, increment);

    // Verify each value is incremented
    const ANVDoublyLinkedNode* node = list->head;
    for (int i = 1; i <= 5; i++)
    {
        ASSERT_EQ(*(int*)node->data, i + 1);
        node = node->next;
    }

    // Test empty list
    ANVDoublyLinkedList* empty_list = anv_dll_create(&alloc);
    anv_dll_for_each(empty_list, increment); // Should do nothing

    // Test null cases
    anv_dll_for_each(NULL, increment); // Should do nothing
    anv_dll_for_each(list, NULL);      // Should do nothing

    anv_dll_destroy(list, true);
    anv_dll_destroy(empty_list, false);
    return TEST_SUCCESS;
}

//==============================================================================
// Iterator Tests
//==============================================================================

static int test_basic_iteration(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    ASSERT_NOT_NULL(list);

    // Insert elements
    for (int i = 1; i <= 5; i++)
    {
        MAKE_INT(val, i);
        anv_dll_push_back(list, val);
    }

    // Create iterator
    ANVIterator it = anv_dll_iterator(list);
    ASSERT_NOT_NULL(it.data_state);
    ASSERT_TRUE(it.has_next(&it));

    // Iterate through list and verify values
    int expected = 1;
    while (it.has_next(&it))
    {
        const int* value = it.get(&it);
        ASSERT_NOT_NULL(value);
        ASSERT_EQ(*value, expected++);
        it.next(&it);
    }

    // Verify we processed all elements
    ASSERT_EQ(expected, 6);

    // Verify the iterator is exhausted
    ASSERT_FALSE(it.has_next(&it));
    ASSERT_NULL(it.get(&it));
    ASSERT_EQ(it.next(&it), -1); // Should return error code

    // Cleanup
    if (it.destroy)
        it.destroy(&it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_empty_list_iterator(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    ASSERT_NOT_NULL(list);

    ANVIterator it = anv_dll_iterator(list);

    // Verify iterator for empty list
    ASSERT_FALSE(it.has_next(&it));
    ASSERT_NULL(it.get(&it));
    ASSERT_EQ(it.next(&it), -1); // Should return error code

    // Cleanup
    if (it.destroy)
        it.destroy(&it);
    anv_dll_destroy(list, false);
    return TEST_SUCCESS;
}

static int test_iterator_with_modifications(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    ASSERT_NOT_NULL(list);

    // Insert initial elements
    for (int i = 1; i <= 3; i++)
    {
        MAKE_INT(val, i);
        anv_dll_push_back(list, val);
    }

    // Create iterator
    ANVIterator it = anv_dll_iterator(list);

    // Consume first element
    const int* value = it.get(&it);
    ASSERT_NOT_NULL(value);
    ASSERT_EQ(*value, 1);
    it.next(&it);

    // Modify list by adding new elements
    MAKE_INT(new_val, 99);
    anv_dll_push_back(list, new_val);

    // Continue iteration
    value = it.get(&it);
    ASSERT_NOT_NULL(value);
    ASSERT_EQ(*value, 2);
    it.next(&it);

    value = it.get(&it);
    ASSERT_NOT_NULL(value);
    ASSERT_EQ(*value, 3);
    it.next(&it);

    // The newly added element should also be accessible
    value = it.get(&it);
    ASSERT_NOT_NULL(value);
    ASSERT_EQ(*value, 99);
    it.next(&it);

    ASSERT_FALSE(it.has_next(&it));

    // Cleanup
    if (it.destroy)
        it.destroy(&it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_multiple_iterators(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    ASSERT_NOT_NULL(list);

    // Insert elements
    for (int i = 1; i <= 5; i++)
    {
        MAKE_INT(val, i);
        anv_dll_push_back(list, val);
    }

    // Create two independent iterators
    ANVIterator it1 = anv_dll_iterator(list);
    ANVIterator it2 = anv_dll_iterator(list);

    // First iterator consumes two elements
    const int* value1 = it1.get(&it1);
    ASSERT_EQ(*value1, 1);
    it1.next(&it1);

    value1 = it1.get(&it1);
    ASSERT_EQ(*value1, 2);
    it1.next(&it1);

    // Second iterator should still be at the beginning
    const int* value2 = it2.get(&it2);
    ASSERT_EQ(*value2, 1);
    it2.next(&it2);

    // Continue with first iterator
    value1 = it1.get(&it1);
    ASSERT_EQ(*value1, 3);
    it1.next(&it1);

    // Continue with second iterator
    value2 = it2.get(&it2);
    ASSERT_EQ(*value2, 2);
    it2.next(&it2);

    // Cleanup
    if (it1.destroy)
        it1.destroy(&it1);
    if (it2.destroy)
        it2.destroy(&it2);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_reverse_iteration(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    ASSERT_NOT_NULL(list);

    // Insert elements 1-5
    for (int i = 1; i <= 5; i++)
    {
        MAKE_INT(val, i);
        anv_dll_push_back(list, val);
    }

    // Create reverse iterator
    ANVIterator it = anv_dll_iterator_reverse(list);
    ASSERT_NOT_NULL(it.data_state);
    ASSERT_TRUE(it.has_next(&it));

    // Iterate in reverse order (should get 5, 4, 3, 2, 1)
    int expected = 5;
    while (it.has_next(&it))
    {
        const int* value = it.get(&it);
        ASSERT_NOT_NULL(value);
        ASSERT_EQ(*value, expected--);
        it.next(&it);
    }

    // Verify we processed all elements
    ASSERT_EQ(expected, 0);

    // Cleanup
    if (it.destroy)
        it.destroy(&it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_iterator_get(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    ASSERT_NOT_NULL(list);

    // Insert elements
    for (int i = 1; i <= 3; i++)
    {
        MAKE_INT(val, i);
        anv_dll_push_back(list, val);
    }

    ANVIterator it = anv_dll_iterator(list);

    // Test get without advancing
    const int* val = it.get(&it);
    ASSERT_NOT_NULL(val);
    ASSERT_EQ(*val, 1);

    // Get again - should return same value
    val = it.get(&it);
    ASSERT_EQ(*val, 1);

    // Now advance and test get
    it.next(&it);
    val = it.get(&it);
    ASSERT_EQ(*val, 2);

    // Cleanup
    if (it.destroy)
        it.destroy(&it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_bidirectional_iteration(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    ASSERT_NOT_NULL(list);

    // Insert elements 1-5
    for (int i = 1; i <= 5; i++)
    {
        MAKE_INT(val, i);
        anv_dll_push_back(list, val);
    }

    ANVIterator it = anv_dll_iterator(list);

    // Move forward to middle
    it.next(&it); // Move to 2
    it.next(&it); // Move to 3

    const int* val = it.get(&it);
    ASSERT_EQ(*val, 3);

    // Move back
    ASSERT_TRUE(it.has_prev(&it));
    it.prev(&it);
    val = it.get(&it);
    ASSERT_EQ(*val, 2);

    // Move forward again
    it.next(&it);
    val = it.get(&it);
    ASSERT_EQ(*val, 3);

    // Cleanup
    if (it.destroy)
        it.destroy(&it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_from_iterator(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create a range iterator (0, 1, 2, 3, 4)
    ANVIterator range_it = anv_iterator_range(&alloc, 0, 5, 1);

    // Create doubly linked list from iterator
    ANVDoublyLinkedList* list = anv_dll_from_iterator(&range_it, &alloc, true);
    ASSERT_NOT_NULL(list);
    ASSERT_EQ(anv_dll_size(list), 5);

    // Clean up the iterator immediately after use
    range_it.destroy(&range_it);

    // Verify doubly linked list has correct values in sequential order
    // Iterator gives 0,1,2,3,4 and doubly linked list should have them as 0,1,2,3,4 (head to tail)
    const ANVDoublyLinkedNode* node = list->head;
    for (int expected = 0; expected < 5; expected++)
    {
        ASSERT_NOT_NULL(node);
        ASSERT_EQ(*(int*)node->data, expected);
        node = node->next;
    }

    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_iterator_invalid(void)
{
    const ANVIterator iter = anv_dll_iterator(NULL);
    ASSERT(!iter.is_valid(&iter));
    return TEST_SUCCESS;
}

static int test_dll_copy_isolation(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create original data that we can modify
    const int original_values[] = {10, 20, 30};
    int* data_ptrs[3];

    // Create a source doubly linked list
    ANVDoublyLinkedList* source_list = anv_dll_create(&alloc);
    ASSERT_NOT_NULL(source_list);

    for (int i = 0; i < 3; i++)
    {
        data_ptrs[i] = malloc(sizeof(int));
        *data_ptrs[i] = original_values[i];
        ASSERT_EQ(anv_dll_push_back(source_list, data_ptrs[i]), 0);
    }

    ANVIterator list_it = anv_dll_iterator(source_list);
    ASSERT(list_it.is_valid(&list_it));

    // Create doubly linked list with copying enabled
    ANVDoublyLinkedList* new_list = anv_dll_from_iterator(&list_it, &alloc, true);
    ASSERT_NOT_NULL(new_list);
    ASSERT_EQ(anv_dll_size(new_list), 3);

    // Modify original data
    *data_ptrs[0] = 999;
    *data_ptrs[1] = 888;
    *data_ptrs[2] = 777;

    // DoublyLinkedList should still have original values (proving data was copied)
    // Sequential order: 10, 20, 30
    const ANVDoublyLinkedNode* node = new_list->head;
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
    anv_dll_destroy(new_list, true);
    anv_dll_destroy(source_list, true);

    return TEST_SUCCESS;
}

static int test_dll_anv_copy_function_required(void)
{
    ANVAllocator alloc = anv_alloc_default();
    alloc.copy = NULL;

    ANVIterator range_it = anv_iterator_range(&alloc, 0, 3, 1);
    ASSERT(range_it.is_valid(&range_it));

    // Should return NULL because should_copy=true but no copy function available
    ANVDoublyLinkedList* list = anv_dll_from_iterator(&range_it, &alloc, true);
    ASSERT_NULL(list);

    range_it.destroy(&range_it);
    return TEST_SUCCESS;
}

static int test_dll_from_iterator_no_copy(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create a range iterator and then a copy iterator to get actual owned data
    ANVIterator range_it = anv_iterator_range(&alloc, 0, 3, 1);
    ASSERT(range_it.is_valid(&range_it));

    // Use copy iterator to create actual data elements that we own
    ANVIterator copy_it = anv_iterator_copy(&range_it, &alloc, int_copy);
    ASSERT(copy_it.is_valid(&copy_it));

    // Create doubly linked list without copying (should_copy = false)
    // This will use the copied elements directly from the copy iterator
    ANVDoublyLinkedList* list = anv_dll_from_iterator(&copy_it, &alloc, false);
    ASSERT_NOT_NULL(list);
    ASSERT_EQ(anv_dll_size(list), 3);

    // Verify values are correct (sequential order: 0, 1, 2)
    const ANVDoublyLinkedNode* node = list->head;
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
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_iterator_exhaustion_after_dll_creation(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVIterator range_it = anv_iterator_range(&alloc, 0, 5, 1);
    ASSERT(range_it.is_valid(&range_it));

    // Verify iterator starts with elements
    ASSERT(range_it.has_next(&range_it));

    // Create doubly linked list from iterator (consumes all elements)
    ANVDoublyLinkedList* list = anv_dll_from_iterator(&range_it, &alloc, true);
    ASSERT_NOT_NULL(list);
    ASSERT_EQ(anv_dll_size(list), 5);

    // Iterator should now be exhausted
    ASSERT(!range_it.has_next(&range_it));
    ASSERT_NULL(range_it.get(&range_it));
    ASSERT_EQ(range_it.next(&range_it), -1); // Should fail to advance

    // But iterator should still be valid
    ASSERT(range_it.is_valid(&range_it));

    range_it.destroy(&range_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_dll_iterator_next_return_values(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    ASSERT_NOT_NULL(list);

    // Add single element
    MAKE_INT(data, 42);
    ASSERT_EQ(anv_dll_push_back(list, data), 0);

    ANVIterator iter = anv_dll_iterator(list);
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
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_dll_iterator_mixed_operations(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    ASSERT_NOT_NULL(list);

    // Add test data (will be in sequential order: 0, 10, 20)
    for (int i = 0; i < 3; i++)
    {
        MAKE_INT(data, i * 10);
        ASSERT_EQ(anv_dll_push_back(list, data), 0);
    }

    ANVIterator iter = anv_dll_iterator(list);
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
    // Note: data1 and data3 point to different doubly linked list elements
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
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_dll_iterator_order(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);

    // Add elements in specific order
    const int values[] = {100, 200, 300, 400, 500};
    for (int i = 0; i < 5; i++)
    {
        MAKE_INT(data, values[i]);
        ASSERT_EQ(anv_dll_push_back(list, data), 0);
    }

    // Create iterator and verify order
    ANVIterator iter = anv_dll_iterator(list);

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
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_dll_iterator_reset(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);

    // Add numbers 1-3
    for (int i = 1; i <= 3; i++)
    {
        MAKE_INT(val, i);
        anv_dll_push_back(list, val);
    }

    ANVIterator iter = anv_dll_iterator(list);

    // Advance iterator
    iter.next(&iter);
    iter.next(&iter);

    // Reset and verify back at beginning
    iter.reset(&iter);
    const int* val = iter.get(&iter);
    ASSERT_EQ(*val, 1);
    ASSERT(iter.has_next(&iter));

    iter.destroy(&iter);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_dll_iterator_single_element(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);

    MAKE_INT(val, 42);
    anv_dll_push_back(list, val);

    ANVIterator iter = anv_dll_iterator(list);

    ASSERT(iter.has_next(&iter));
    ASSERT(!iter.has_prev(&iter)); // At beginning, no previous

    const int* retrieved = iter.get(&iter);
    ASSERT_EQ(*retrieved, 42);

    iter.next(&iter);
    ASSERT(!iter.has_next(&iter));
    ASSERT(iter.has_prev(&iter)); // At end, has previous

    iter.destroy(&iter);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Memory Tests
//==============================================================================

static int test_custom_allocator(void)
{
    ANVAllocator alloc = create_failing_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    ASSERT_NOT_NULL(list);
    MAKE_INT(a, 42);
    ASSERT_EQ(anv_dll_push_back(list, a), 0);
    ASSERT_EQ(list->size, 1);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_clear(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);

    // Add some elements
    for (int i = 0; i < 5; i++)
    {
        MAKE_INT(val, i);
        anv_dll_push_back(list, val);
    }
    ASSERT_EQ(list->size, 5);

    // Clear the list
    anv_dll_clear(list, true);

    // Verify list state
    ASSERT_NULL(list->head);
    ASSERT_NULL(list->tail);
    ASSERT_EQ(list->size, 0);
    ASSERT_EQ(anv_dll_is_empty(list), 1);

    // Make sure we can still add elements after clearing
    MAKE_INT(val, 42);
    ASSERT_EQ(anv_dll_push_back(list, val), 0);
    ASSERT_EQ(list->size, 1);

    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_clear_empty(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);

    // Clear an already empty list
    anv_dll_clear(list, true);
    ASSERT_NULL(list->head);
    ASSERT_NULL(list->tail);
    ASSERT_EQ(list->size, 0);

    anv_dll_destroy(list, false);
    return TEST_SUCCESS;
}

static int test_clear_null(void)
{
    // Calling clear on NULL shouldn't crash
    anv_dll_clear(NULL, true);
    return TEST_SUCCESS;
}

static int test_copy_shallow(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);

    // Add some elements
    for (int i = 0; i < 5; i++)
    {
        MAKE_INT(val, i * 10);
        anv_dll_push_back(list, val);
    }

    // Create shallow clone
    ANVDoublyLinkedList* copy = anv_dll_copy(list);
    ASSERT_NOT_NULL(copy);
    ASSERT_EQ(copy->size, list->size);

    // Verify structure
    ANVDoublyLinkedNode* orig_node = list->head;
    ANVDoublyLinkedNode* copy_node = copy->head;
    while (orig_node && copy_node)
    {
        // Data pointers should be identical in shallow clone
        ASSERT_EQ(orig_node->data, copy_node->data);
        // But nodes themselves should be different
        ASSERT_NOT_EQ(orig_node, copy_node);

        // Verify next pointers
        if (orig_node->next)
        {
            ASSERT_NOT_EQ(orig_node->next, copy_node->next);
        }
        else
        {
            ASSERT_NULL(copy_node->next);
        }

        // Verify prev pointers
        if (orig_node->prev)
        {
            ASSERT_NOT_EQ(orig_node->prev, copy_node->prev);
        }
        else
        {
            ASSERT_NULL(copy_node->prev);
        }

        orig_node = orig_node->next;
        copy_node = copy_node->next;
    }

    // Verify bidirectional links in the clone
    ASSERT_NULL(copy->head->prev);
    ASSERT_NULL(copy->tail->next);

    // Modifying data should affect both lists (shared pointers)
    int* first_value = list->head->data;
    *first_value = 999;
    ASSERT_EQ(*(int*)copy->head->data, 999);

    // Cleanup - free each int only once since they're shared
    anv_dll_destroy(list, true);
    anv_dll_destroy(copy, false);

    return TEST_SUCCESS;
}

static int test_copy_deep(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);

    // Add some elements
    for (int i = 0; i < 5; i++)
    {
        MAKE_INT(val, i * 10);
        anv_dll_push_back(list, val);
    }

    // Create deep clone
    ANVDoublyLinkedList* copy = anv_dll_copy_deep(list, true);
    ASSERT_NOT_NULL(copy);
    ASSERT_EQ(copy->size, list->size);

    // Verify structure and values
    const ANVDoublyLinkedNode* orig_node = list->head;
    const ANVDoublyLinkedNode* copy_node = copy->head;
    while (orig_node && copy_node)
    {
        // Data pointers should be different in deep clone
        ASSERT_NOT_EQ(orig_node->data, copy_node->data);
        // But values should be the same
        ASSERT_EQ(*(int*)orig_node->data, *(int*)copy_node->data);

        orig_node = orig_node->next;
        copy_node = copy_node->next;
    }

    // Verify bidirectional links in the clone
    ASSERT_NULL(copy->head->prev);
    ASSERT_NULL(copy->tail->next);

    // Check traversal in reverse
    orig_node = list->tail;
    copy_node = copy->tail;
    while (orig_node && copy_node)
    {
        ASSERT_NOT_EQ(orig_node->data, copy_node->data);
        ASSERT_EQ(*(int*)orig_node->data, *(int*)copy_node->data);

        orig_node = orig_node->prev;
        copy_node = copy_node->prev;
    }

    // Modifying data should not affect the other list (independent copies)
    int* first_value = list->head->data;
    *first_value = 999;
    ASSERT_NOT_EQ(*(int*)copy->head->data, 999);

    // Cleanup - each list has its own data
    anv_dll_destroy(list, true);
    anv_dll_destroy(copy, true);

    return TEST_SUCCESS;
}

static int test_copy_complex_data(void)
{
    ANVAllocator alloc = create_person_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);

    // Add some people
    Person* p1 = create_person("Alice", 30);
    Person* p2 = create_person("Bob", 25);
    Person* p3 = create_person("Charlie", 40);

    anv_dll_push_back(list, p1);
    anv_dll_push_back(list, p2);
    anv_dll_push_back(list, p3);
    ASSERT_EQ(list->size, 3);

    // Create deep clone
    ANVDoublyLinkedList* copy = anv_dll_copy_deep(list, true);
    ASSERT_NOT_NULL(copy);
    ASSERT_EQ(copy->size, list->size);

    // Verify structure and values
    const ANVDoublyLinkedNode* orig_node = list->head;
    const ANVDoublyLinkedNode* copy_node = copy->head;
    while (orig_node && copy_node)
    {
        Person* orig_person = orig_node->data;
        Person* clone_person = copy_node->data;

        // Data pointers should be different
        ASSERT_NOT_EQ(orig_person, clone_person);
        // But values should be the same
        ASSERT_EQ(strcmp(orig_person->name, clone_person->name), 0);
        ASSERT_EQ(orig_person->age, clone_person->age);

        orig_node = orig_node->next;
        copy_node = copy_node->next;
    }

    // Modifying should not affect the other list
    Person* first_person = list->head->data;
    first_person->age = 99;
    const Person* copy_first = copy->head->data;
    ASSERT_NOT_EQ(first_person->age, copy_first->age);

    // Cleanup
    anv_dll_destroy(list, true);
    anv_dll_destroy(copy, true);

    return TEST_SUCCESS;
}

static int test_copy_empty(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);

    // Clone empty list
    ANVDoublyLinkedList* shallow_copy = anv_dll_copy(list);
    ASSERT_NOT_NULL(shallow_copy);
    ASSERT_EQ(shallow_copy->size, 0);
    ASSERT_NULL(shallow_copy->head);
    ASSERT_NULL(shallow_copy->tail);

    ANVDoublyLinkedList* deep_copy = anv_dll_copy_deep(list, true);
    ASSERT_NOT_NULL(deep_copy);
    ASSERT_EQ(deep_copy->size, 0);
    ASSERT_NULL(deep_copy->head);
    ASSERT_NULL(deep_copy->tail);

    // Cleanup
    anv_dll_destroy(list, false);
    anv_dll_destroy(shallow_copy, false);
    anv_dll_destroy(deep_copy, false);

    return TEST_SUCCESS;
}

static int test_copy_null(void)
{
    // Should handle NULL gracefully
    ASSERT_NULL(anv_dll_copy(NULL));
    ASSERT_NULL(anv_dll_copy_deep(NULL, true));

    return TEST_SUCCESS;
}

static int test_insert_allocation_failure(void)
{
    set_alloc_fail_countdown(-1);
    ANVAllocator alloc = create_failing_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    MAKE_INT(a, 1);
    anv_dll_push_back(list, a);
    ASSERT_EQ(list->size, 1);

    // Set allocator to fail on the next allocation (for the node)
    set_alloc_fail_countdown(0);
    MAKE_INT(b, 2);
    ASSERT_EQ(anv_dll_push_back(list, b), -1);

    // Verify list is unchanged
    ASSERT_EQ(list->size, 1);
    ASSERT_NOT_NULL(list->head);
    ASSERT_EQ(list->head, list->tail);
    ASSERT_NULL(list->head->next);

    set_alloc_fail_countdown(-1);
    anv_dll_destroy(list, true);
    free(b); // 'b' was never added to the list, so we must free it manually
    return TEST_SUCCESS;
}

static int test_copy_deep_allocation_failure(void)
{
    set_alloc_fail_countdown(-1);
    ANVAllocator alloc = create_failing_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    for (int i = 0; i < 5; i++)
    {
        MAKE_INT(val, i);
        anv_dll_push_back(list, val);
    }

    // Case 1: Fail allocating the new list struct itself
    set_alloc_fail_countdown(0);
    ANVDoublyLinkedList* clone1 = anv_dll_copy_deep(list, true);
    ASSERT_NULL(clone1);

    // Case 2: Fail allocating the data partway through
    set_alloc_fail_countdown(3); // 1=clone list, 2=data0, 3=node0, FAIL on data1
    ANVDoublyLinkedList* clone2 = anv_dll_copy_deep(list, true);
    ASSERT_NULL(clone2);

    // Case 3: Fail allocating a node partway through
    set_alloc_fail_countdown(2); // 1=clone list, 2=data0, FAIL on node0
    ANVDoublyLinkedList* clone3 = anv_dll_copy_deep(list, true);
    ASSERT_NULL(clone3);

    set_alloc_fail_countdown(-1); // Reset for cleanup
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_transform_allocation_failure(void)
{
    set_alloc_fail_countdown(-1);
    ANVAllocator alloc = create_failing_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    for (int i = 0; i < 5; i++)
    {
        MAKE_INT(val, i);
        anv_dll_push_back(list, val);
    }

    // Case 1: Fail on creation of the result list
    set_alloc_fail_countdown(0);
    ANVDoublyLinkedList* mapped1 = anv_dll_transform(list, double_value_failing, true);
    ASSERT_NULL(mapped1);

    // Case 2: Fail on data allocation inside the transform function
    set_alloc_fail_countdown(1); // 1=result list, FAIL on data for first element
    ANVDoublyLinkedList* mapped2 = anv_dll_transform(list, double_value_failing, true);
    ASSERT_NULL(mapped2);

    // Case 3: Fail on node allocation inside anv_dll_push_back
    set_alloc_fail_countdown(2); // 1=result list, 2=data for first element, FAIL on 3=node
    ANVDoublyLinkedList* mapped3 = anv_dll_transform(list, double_value_failing, true);
    ASSERT_NULL(mapped3);

    set_alloc_fail_countdown(-1);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_from_iterator_custom_alloc_failure(void)
{
    set_alloc_fail_countdown(-1);
    ANVAllocator src_alloc = create_failing_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&src_alloc);
    for (int i = 0; i < 5; i++)
    {
        MAKE_INT(val, i);
        anv_dll_push_back(list, val);
    }
    ANVIterator it = anv_dll_iterator(list);

    // Case 1: Fail on list creation
    set_alloc_fail_countdown(0);
    ANVAllocator alloc_for_new = create_failing_int_allocator();
    ANVDoublyLinkedList* new_list1 = anv_dll_from_iterator(&it, &alloc_for_new, true);
    ASSERT_NULL(new_list1);
    it.reset(&it);

    // Case 2: Fail on data copy
    set_alloc_fail_countdown(1); // 1=new list, FAIL on data copy
    ANVDoublyLinkedList* new_list2 = anv_dll_from_iterator(&it, &alloc_for_new, true);
    ASSERT_NULL(new_list2);
    it.reset(&it);

    // Case 3: Fail on node insertion
    set_alloc_fail_countdown(2); // 1=new list, 2=data copy, FAIL on node insert
    ANVDoublyLinkedList* new_list3 = anv_dll_from_iterator(&it, &alloc_for_new, true);
    ASSERT_NULL(new_list3);

    it.destroy(&it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Performance Tests
//==============================================================================

static int test_stress(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    const int NUM_ELEMENTS = 10000;

    // Add many elements
    for (int i = 0; i < NUM_ELEMENTS; i++)
    {
        MAKE_INT(val, i);
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

static int test_performance(void)
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
            MAKE_INT(val, i);
            anv_dll_push_back(list, val);
        }
        clock_t end = clock();
        const double insert_time = (double)(end - start) / CLOCKS_PER_SEC;
        printf("Insert %d elements: %.6f seconds\n", SIZE, insert_time);
        ASSERT_LT(insert_time, 5.0);

        // Measure search time for last element
        start = clock();
        int key = SIZE - 1;
        const ANVDoublyLinkedNode* found = anv_dll_find(list, &key, int_cmp);
        end = clock();
        const double find_time = (double)(end - start) / CLOCKS_PER_SEC;
        printf("Find last element in %d elements: %.6f seconds\n", SIZE, find_time);
        ASSERT_LT(find_time, 5.0);
        ASSERT_NOT_NULL(found);

        // Cleanup
        anv_dll_destroy(list, true);
    }

    return TEST_SUCCESS;
}

//==============================================================================
// Properties Tests
//==============================================================================

static int test_anv_dll_size_after_insert_and_remove(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    ASSERT_EQ(anv_dll_size(list), 0);

    MAKE_INT(a, 1);
    anv_dll_push_back(list, a);
    ASSERT_EQ(anv_dll_size(list), 1);

    MAKE_INT(b, 2);
    anv_dll_push_front(list, b);
    ASSERT_EQ(anv_dll_size(list), 2);

    anv_dll_remove_at(list, 0, true);
    ASSERT_EQ(anv_dll_size(list), 1);

    anv_dll_pop_back(list, true);
    ASSERT_EQ(anv_dll_size(list), 0);

    anv_dll_destroy(list, false);
    return TEST_SUCCESS;
}

static int test_anv_dll_sort_is_idempotent(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    for (int i = 0; i < 10; i++)
    {
        MAKE_INT(val, i);
        anv_dll_push_back(list, val);
    }

    anv_dll_sort(list, int_cmp); // First sort
    ANVDoublyLinkedList* copy = anv_dll_copy_deep(list, true);

    anv_dll_sort(list, int_cmp); // Second sort

    ASSERT_EQ(anv_dll_equals(list, copy, int_cmp), 1);

    anv_dll_destroy(list, true);
    anv_dll_destroy(copy, true);
    return TEST_SUCCESS;
}

static int test_anv_dll_reverse_is_involution(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    for (int i = 0; i < 10; i++)
    {
        MAKE_INT(val, i);
        anv_dll_push_back(list, val);
    }

    ANVDoublyLinkedList* copy = anv_dll_copy_deep(list, true);

    anv_dll_reverse(list);
    anv_dll_reverse(list);

    ASSERT_EQ(anv_dll_equals(list, copy, int_cmp), 1);

    anv_dll_destroy(list, true);
    anv_dll_destroy(copy, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Fuzz Tests
//==============================================================================

static int test_dll_fuzz(void)
{
    srand((unsigned int)42);
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    ASSERT_NOT_NULL(list);

    size_t expected_size = 0;

    for (int i = 0; i < 50000; i++)
    {
        const unsigned op = rand() % 5;

        switch (op)
        {
            case 0: // push_front
            {
                MAKE_INT(val, rand());
                if (anv_dll_push_front(list, val) == 0)
                    expected_size++;
                else
                    free(val);
                break;
            }
            case 1: // push_back
            {
                MAKE_INT(val, rand());
                if (anv_dll_push_back(list, val) == 0)
                    expected_size++;
                else
                    free(val);
                break;
            }
            case 2: // remove front
            {
                if (expected_size > 0)
                {
                    void* data = list->head->data;
                    if (anv_dll_remove(list, data, int_cmp, true) == 0)
                        expected_size--;
                }
                break;
            }
            case 3: // find random
            {
                if (expected_size > 0)
                {
                    int key = rand() % 1000;
                    anv_dll_find(list, &key, int_cmp);
                }
                break;
            }
            case 4: // insert_at random position
            {
                if (expected_size > 0)
                {
                    const size_t pos = rand() % expected_size;
                    MAKE_INT(val, rand());
                    if (anv_dll_insert_at(list, pos, val) == 0)
                        expected_size++;
                    else
                        free(val);
                }
                break;
            }
            default:
                break;
        }

        ASSERT_EQ(list->size, expected_size);
    }

    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Main
//==============================================================================

int main(void)
{
    const ANVTestCase tests[] = {
        // CRUD
        TEST_REGISTER(test_create_destroy),
        TEST_REGISTER(test_push_front),
        TEST_REGISTER(test_push_back),
        TEST_REGISTER(test_find),
        TEST_REGISTER(test_remove),
        TEST_REGISTER(test_remove_not_found),
        TEST_REGISTER(test_nullptr_handling),
        TEST_REGISTER(test_insert_at),
        TEST_REGISTER(test_remove_at),
        TEST_REGISTER(test_remove_at_head),
        TEST_REGISTER(test_remove_at_last),
        TEST_REGISTER(test_remove_at_invalid),
        TEST_REGISTER(test_remove_at_empty),
        TEST_REGISTER(test_remove_at_single_element),
        TEST_REGISTER(test_remove_at_single_element_invalid_pos),
        TEST_REGISTER(test_insert_at_out_of_bounds),
        TEST_REGISTER(test_insert_remove_null_data),
        TEST_REGISTER(test_mixed_operations_integrity),
        TEST_REGISTER(test_size),
        TEST_REGISTER(test_is_empty),
        TEST_REGISTER(test_complex_data_type),
        TEST_REGISTER(test_remove_all),
        TEST_REGISTER(test_remove_front),
        TEST_REGISTER(test_remove_back),

        // Algorithms
        TEST_REGISTER(test_sort_empty),
        TEST_REGISTER(test_sort_already_sorted),
        TEST_REGISTER(test_sort_reverse_order),
        TEST_REGISTER(test_sort_with_duplicates),
        TEST_REGISTER(test_sort_large_list),
        TEST_REGISTER(test_sort_custom_compare),
        TEST_REGISTER(test_sort_null_args),
        TEST_REGISTER(test_sort_stability),
        TEST_REGISTER(test_reverse),
        TEST_REGISTER(test_merge),
        TEST_REGISTER(test_splice),
        TEST_REGISTER(test_equals),
        TEST_REGISTER(test_filter),
        TEST_REGISTER(test_filter_deep),
        TEST_REGISTER(test_transform),
        TEST_REGISTER(test_for_each),

        // Iterator
        TEST_REGISTER(test_basic_iteration),
        TEST_REGISTER(test_empty_list_iterator),
        TEST_REGISTER(test_iterator_with_modifications),
        TEST_REGISTER(test_multiple_iterators),
        TEST_REGISTER(test_reverse_iteration),
        TEST_REGISTER(test_iterator_get),
        TEST_REGISTER(test_bidirectional_iteration),
        TEST_REGISTER(test_from_iterator),
        TEST_REGISTER(test_iterator_invalid),
        TEST_REGISTER(test_dll_copy_isolation),
        TEST_REGISTER(test_dll_anv_copy_function_required),
        TEST_REGISTER(test_dll_from_iterator_no_copy),
        TEST_REGISTER(test_iterator_exhaustion_after_dll_creation),
        TEST_REGISTER(test_dll_iterator_next_return_values),
        TEST_REGISTER(test_dll_iterator_mixed_operations),
        TEST_REGISTER(test_dll_iterator_order),
        TEST_REGISTER(test_dll_iterator_reset),
        TEST_REGISTER(test_dll_iterator_single_element),

        // Memory
        TEST_REGISTER(test_custom_allocator),
        TEST_REGISTER(test_clear),
        TEST_REGISTER(test_clear_empty),
        TEST_REGISTER(test_clear_null),
        TEST_REGISTER(test_copy_shallow),
        TEST_REGISTER(test_copy_deep),
        TEST_REGISTER(test_copy_complex_data),
        TEST_REGISTER(test_copy_empty),
        TEST_REGISTER(test_copy_null),
        TEST_REGISTER(test_insert_allocation_failure),
        TEST_REGISTER(test_copy_deep_allocation_failure),
        TEST_REGISTER(test_transform_allocation_failure),
        TEST_REGISTER(test_from_iterator_custom_alloc_failure),

        // Performance
        TEST_REGISTER(test_stress),
        TEST_REGISTER(test_performance),

        // Properties
        TEST_REGISTER(test_anv_dll_size_after_insert_and_remove),
        TEST_REGISTER(test_anv_dll_sort_is_idempotent),
        TEST_REGISTER(test_anv_dll_reverse_is_involution),

        // Fuzz Tests
        TEST_REGISTER(test_dll_fuzz),
    };
    return anv_run_tests("DoublyLinkedList", tests, sizeof(tests) / sizeof(tests[0]));
}