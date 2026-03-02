//
// Consolidated ArrayList tests
// Merged from: test_arraylist_algorithms.c, test_arraylist_boundary.c,
//              test_arraylist_crud.c, test_arraylist_iterator.c,
//              test_arraylist_memory.c, test_arraylist_properties.c
//

#include "containers/arraylist.h"
#include "TestAssert.h"
#include "TestHelpers.h"
#include "TestRunner.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//==============================================================================
// Static Helpers
//==============================================================================

// Helper for for_each test
static int for_each_sum = 0;

static void add_to_sum(void* data)
{
    for_each_sum += *(int*)data;
}

//==============================================================================
// CRUD Tests
//==============================================================================

int test_create_destroy(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);
    ASSERT_NOT_NULL(list);
    ASSERT_EQ(anv_arraylist_size(list), 0);
    ASSERT_EQ(anv_arraylist_capacity(list), 0);
    ASSERT(anv_arraylist_is_empty(list));
    anv_arraylist_destroy(list, false);
    return TEST_SUCCESS;
}

int test_create_with_capacity(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 10);
    ASSERT_NOT_NULL(list);
    ASSERT_EQ(anv_arraylist_size(list), 0);
    ASSERT_GT(anv_arraylist_capacity(list), 0);
    ASSERT(anv_arraylist_is_empty(list));
    anv_arraylist_destroy(list, false);
    return TEST_SUCCESS;
}

int test_push_back(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    int* a = malloc(sizeof(int));
    *a = 1;
    int* b = malloc(sizeof(int));
    *b = 2;
    int* c = malloc(sizeof(int));
    *c = 3;

    ASSERT_EQ(anv_arraylist_push_back(list, a), 0);
    ASSERT_EQ(anv_arraylist_size(list), 1);
    ASSERT(!anv_arraylist_is_empty(list));

    ASSERT_EQ(anv_arraylist_push_back(list, b), 0);
    ASSERT_EQ(anv_arraylist_push_back(list, c), 0);
    ASSERT_EQ(anv_arraylist_size(list), 3);

    // Verify elements
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 0), 1);
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 1), 2);
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 2), 3);

    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

int test_push_front(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    int* a = malloc(sizeof(int));
    *a = 1;
    int* b = malloc(sizeof(int));
    *b = 2;
    int* c = malloc(sizeof(int));
    *c = 3;

    ASSERT_EQ(anv_arraylist_push_front(list, a), 0);
    ASSERT_EQ(anv_arraylist_push_front(list, b), 0);
    ASSERT_EQ(anv_arraylist_push_front(list, c), 0);
    ASSERT_EQ(anv_arraylist_size(list), 3);

    // Verify elements (should be in reverse order)
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 0), 3);
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 1), 2);
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 2), 1);

    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

int test_insert_at(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    int* a = malloc(sizeof(int));
    *a = 1;
    int* b = malloc(sizeof(int));
    *b = 2;
    int* c = malloc(sizeof(int));
    *c = 3;
    int* d = malloc(sizeof(int));
    *d = 4;

    // Insert at beginning (empty list)
    ASSERT_EQ(anv_arraylist_insert(list, 0, a), 0);
    ASSERT_EQ(anv_arraylist_size(list), 1);
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 0), 1);

    // Insert at end
    ASSERT_EQ(anv_arraylist_insert(list, 1, c), 0);
    ASSERT_EQ(anv_arraylist_size(list), 2);
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 1), 3);

    // Insert in middle
    ASSERT_EQ(anv_arraylist_insert(list, 1, b), 0);
    ASSERT_EQ(anv_arraylist_size(list), 3);
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 0), 1);
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 1), 2);
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 2), 3);

    // Insert at beginning
    ASSERT_EQ(anv_arraylist_insert(list, 0, d), 0);
    ASSERT_EQ(anv_arraylist_size(list), 4);
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 0), 4);
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 1), 1);

    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

int test_get_set(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    int* a = malloc(sizeof(int));
    *a = 1;
    int* b = malloc(sizeof(int));
    *b = 2;
    int* c = malloc(sizeof(int));
    *c = 3;
    int* d = malloc(sizeof(int));
    *d = 42;

    anv_arraylist_push_back(list, a);
    anv_arraylist_push_back(list, b);
    anv_arraylist_push_back(list, c);

    // Test get
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 0), 1);
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 1), 2);
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 2), 3);
    ASSERT_NULL(anv_arraylist_get(list, 3)); // Out of bounds

    // Test set
    ASSERT_EQ(anv_arraylist_set(list, 1, d, true), 0);
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 1), 42);
    ASSERT_EQ(anv_arraylist_set(list, 5, d, false), -1); // Out of bounds

    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

int test_front_back(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Empty list
    ASSERT_NULL(anv_arraylist_front(list));
    ASSERT_NULL(anv_arraylist_back(list));

    int* a = malloc(sizeof(int));
    *a = 1;
    int* b = malloc(sizeof(int));
    *b = 2;
    int* c = malloc(sizeof(int));
    *c = 3;

    // Single element
    anv_arraylist_push_back(list, a);
    ASSERT_EQ(*(int*)anv_arraylist_front(list), 1);
    ASSERT_EQ(*(int*)anv_arraylist_back(list), 1);

    // Multiple elements
    anv_arraylist_push_back(list, b);
    anv_arraylist_push_back(list, c);
    ASSERT_EQ(*(int*)anv_arraylist_front(list), 1);
    ASSERT_EQ(*(int*)anv_arraylist_back(list), 3);

    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

int test_remove_at(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    int* a = malloc(sizeof(int));
    *a = 1;
    int* b = malloc(sizeof(int));
    *b = 2;
    int* c = malloc(sizeof(int));
    *c = 3;
    int* d = malloc(sizeof(int));
    *d = 4;

    anv_arraylist_push_back(list, a);
    anv_arraylist_push_back(list, b);
    anv_arraylist_push_back(list, c);
    anv_arraylist_push_back(list, d);

    // Remove from middle
    ASSERT_EQ(anv_arraylist_remove_at(list, 1, true), 0);
    ASSERT_EQ(anv_arraylist_size(list), 3);
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 0), 1);
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 1), 3);
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 2), 4);

    // Remove from beginning
    ASSERT_EQ(anv_arraylist_remove_at(list, 0, true), 0);
    ASSERT_EQ(anv_arraylist_size(list), 2);
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 0), 3);
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 1), 4);

    // Remove from end
    ASSERT_EQ(anv_arraylist_remove_at(list, 1, true), 0);
    ASSERT_EQ(anv_arraylist_size(list), 1);
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 0), 3);

    // Test invalid index
    ASSERT_EQ(anv_arraylist_remove_at(list, 5, false), -1);

    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

int test_pop_back_front(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    int* a = malloc(sizeof(int));
    *a = 1;
    int* b = malloc(sizeof(int));
    *b = 2;
    int* c = malloc(sizeof(int));
    *c = 3;

    anv_arraylist_push_back(list, a);
    anv_arraylist_push_back(list, b);
    anv_arraylist_push_back(list, c);

    // Pop back
    ASSERT_EQ(anv_arraylist_pop_back(list, true), 0);
    ASSERT_EQ(anv_arraylist_size(list), 2);
    ASSERT_EQ(*(int*)anv_arraylist_back(list), 2);

    // Pop front
    ASSERT_EQ(anv_arraylist_pop_front(list, true), 0);
    ASSERT_EQ(anv_arraylist_size(list), 1);
    ASSERT_EQ(*(int*)anv_arraylist_front(list), 2);

    // Pop remaining element
    ASSERT_EQ(anv_arraylist_pop_back(list, true), 0);
    ASSERT_EQ(anv_arraylist_size(list), 0);
    ASSERT(anv_arraylist_is_empty(list));

    // Test pop from empty list
    ASSERT_EQ(anv_arraylist_pop_back(list, false), -1);
    ASSERT_EQ(anv_arraylist_pop_front(list, false), -1);

    anv_arraylist_destroy(list, false);
    return TEST_SUCCESS;
}

int test_find(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    int* a = malloc(sizeof(int));
    *a = 1;
    int* b = malloc(sizeof(int));
    *b = 2;
    int* c = malloc(sizeof(int));
    *c = 3;

    anv_arraylist_push_back(list, a);
    anv_arraylist_push_back(list, b);
    anv_arraylist_push_back(list, c);

    const int key = 2;
    size_t index = anv_arraylist_find(list, &key, int_cmp);
    ASSERT_EQ(index, 1);

    const int not_found = 42;
    index = anv_arraylist_find(list, &not_found, int_cmp);
    ASSERT_EQ(index, SIZE_MAX);

    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

int test_remove(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    int* a = malloc(sizeof(int));
    *a = 1;
    int* b = malloc(sizeof(int));
    *b = 2;
    int* c = malloc(sizeof(int));
    *c = 3;

    anv_arraylist_push_back(list, a);
    anv_arraylist_push_back(list, b);
    anv_arraylist_push_back(list, c);

    const int key = 2;
    ASSERT_EQ(anv_arraylist_remove(list, &key, int_cmp, true), 0);
    ASSERT_EQ(anv_arraylist_size(list), 2);
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 0), 1);
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 1), 3);

    const int not_found = 42;
    ASSERT_EQ(anv_arraylist_remove(list, &not_found, int_cmp, false), -1);

    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

int test_clear(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    int* a = malloc(sizeof(int));
    *a = 1;
    int* b = malloc(sizeof(int));
    *b = 2;
    int* c = malloc(sizeof(int));
    *c = 3;

    anv_arraylist_push_back(list, a);
    anv_arraylist_push_back(list, b);
    anv_arraylist_push_back(list, c);

    anv_arraylist_clear(list, true);
    ASSERT_EQ(anv_arraylist_size(list), 0);
    ASSERT(anv_arraylist_is_empty(list));
    ASSERT_GT(anv_arraylist_capacity(list), 0); // Capacity should remain

    anv_arraylist_destroy(list, false);
    return TEST_SUCCESS;
}

//==============================================================================
// Boundary Tests
//==============================================================================

int test_single_element_lifecycle(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    int* val = malloc(sizeof(int));
    *val = 42;

    // Push, verify, remove
    ASSERT_EQ(anv_arraylist_push_back(list, val), 0);
    ASSERT_EQ(anv_arraylist_size(list), 1);
    ASSERT_FALSE(anv_arraylist_is_empty(list));
    ASSERT_EQ(*(int*)anv_arraylist_front(list), 42);
    ASSERT_EQ(*(int*)anv_arraylist_back(list), 42);
    ASSERT_EQ_PTR(anv_arraylist_front(list), anv_arraylist_back(list));

    ASSERT_EQ(anv_arraylist_pop_back(list, true), 0);
    ASSERT_EQ(anv_arraylist_size(list), 0);
    ASSERT_TRUE(anv_arraylist_is_empty(list));

    anv_arraylist_destroy(list, false);
    return TEST_SUCCESS;
}

int test_insert_at_boundaries(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    int* a = malloc(sizeof(int)); *a = 1;
    int* b = malloc(sizeof(int)); *b = 2;
    int* c = malloc(sizeof(int)); *c = 3;

    // Insert at index 0 (empty list)
    ASSERT_EQ(anv_arraylist_insert(list, 0, a), 0);

    // Insert at index = size (end)
    ASSERT_EQ(anv_arraylist_insert(list, anv_arraylist_size(list), c), 0);

    // Insert at index 1 (middle)
    ASSERT_EQ(anv_arraylist_insert(list, 1, b), 0);

    ASSERT_EQ(*(int*)anv_arraylist_get(list, 0), 1);
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 1), 2);
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 2), 3);

    // Insert at out-of-bounds index
    int* d = malloc(sizeof(int)); *d = 4;
    ASSERT_NOT_EQ(anv_arraylist_insert(list, 100, d), 0);
    free(d);

    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

int test_get_out_of_bounds(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Empty list - all indices out of bounds
    ASSERT_NULL(anv_arraylist_get(list, 0));
    ASSERT_NULL(anv_arraylist_get(list, 1));
    ASSERT_NULL(anv_arraylist_get(list, SIZE_MAX));

    int* val = malloc(sizeof(int)); *val = 42;
    anv_arraylist_push_back(list, val);

    // Valid index
    ASSERT_NOT_NULL(anv_arraylist_get(list, 0));

    // Just past end
    ASSERT_NULL(anv_arraylist_get(list, 1));

    // Large index
    ASSERT_NULL(anv_arraylist_get(list, SIZE_MAX));

    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

int test_remove_at_boundaries(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Remove from empty list
    ASSERT_NOT_EQ(anv_arraylist_remove_at(list, 0, false), 0);

    int* a = malloc(sizeof(int)); *a = 1;
    int* b = malloc(sizeof(int)); *b = 2;
    int* c = malloc(sizeof(int)); *c = 3;

    anv_arraylist_push_back(list, a);
    anv_arraylist_push_back(list, b);
    anv_arraylist_push_back(list, c);

    // Remove at out-of-bounds
    ASSERT_NOT_EQ(anv_arraylist_remove_at(list, 10, false), 0);
    ASSERT_NOT_EQ(anv_arraylist_remove_at(list, SIZE_MAX, false), 0);
    ASSERT_EQ(anv_arraylist_size(list), 3); // Nothing changed

    // Remove last element
    ASSERT_EQ(anv_arraylist_remove_at(list, 2, true), 0);
    // Remove first element
    ASSERT_EQ(anv_arraylist_remove_at(list, 0, true), 0);
    // Remove remaining element
    ASSERT_EQ(anv_arraylist_remove_at(list, 0, true), 0);

    ASSERT_TRUE(anv_arraylist_is_empty(list));

    anv_arraylist_destroy(list, false);
    return TEST_SUCCESS;
}

int test_pop_empty_list(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    ASSERT_NOT_EQ(anv_arraylist_pop_back(list, false), 0);
    ASSERT_NOT_EQ(anv_arraylist_pop_front(list, false), 0);

    anv_arraylist_destroy(list, false);
    return TEST_SUCCESS;
}

int test_clear_empty_list(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Clear an already empty list should not crash
    anv_arraylist_clear(list, false);
    ASSERT_EQ(anv_arraylist_size(list), 0);
    ASSERT_TRUE(anv_arraylist_is_empty(list));

    anv_arraylist_destroy(list, false);
    return TEST_SUCCESS;
}

int test_front_back_empty(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    ASSERT_NULL(anv_arraylist_front(list));
    ASSERT_NULL(anv_arraylist_back(list));

    anv_arraylist_destroy(list, false);
    return TEST_SUCCESS;
}

int test_capacity_growth(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 1);

    size_t prev_capacity = anv_arraylist_capacity(list);

    // Push enough elements to force multiple capacity growths
    for (int i = 0; i < 100; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        ASSERT_EQ(anv_arraylist_push_back(list, val), 0);
    }

    // Capacity should have grown
    ASSERT_GT(anv_arraylist_capacity(list), prev_capacity);
    ASSERT_GTE(anv_arraylist_capacity(list), 100);

    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

int test_null_data(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Push NULL data
    ASSERT_EQ(anv_arraylist_push_back(list, NULL), 0);
    ASSERT_EQ(anv_arraylist_size(list), 1);
    ASSERT_NULL(anv_arraylist_get(list, 0));

    anv_arraylist_destroy(list, false);
    return TEST_SUCCESS;
}

int test_null_list_operations(void)
{
    // Operations on NULL list should not crash and return error codes
    ASSERT_NULL(anv_arraylist_get(NULL, 0));
    ASSERT_NULL(anv_arraylist_front(NULL));
    ASSERT_NULL(anv_arraylist_back(NULL));

    return TEST_SUCCESS;
}

//==============================================================================
// Iterator Tests
//==============================================================================

int test_forward_iterator(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Add numbers 1-5
    for (int i = 1; i <= 5; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_arraylist_push_back(list, val);
    }

    ANVIterator iter = anv_arraylist_iterator(list);
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
    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

int test_reverse_iterator(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Add numbers 1-5
    for (int i = 1; i <= 5; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_arraylist_push_back(list, val);
    }

    ANVIterator iter = anv_arraylist_iterator_reverse(list);
    ASSERT(iter.is_valid(&iter));

    // Test reverse iteration (should get 5, 4, 3, 2, 1)
    int expected = 5;
    while (iter.has_next(&iter))
    {
        const int* val = (int*)iter.get(&iter);
        ASSERT_NOT_NULL(val);
        ASSERT_EQ(*val, expected);
        expected--;
        iter.next(&iter);
    }

    ASSERT_EQ(expected, 0); // Should have iterated through all 5 elements
    ASSERT(!iter.has_next(&iter));

    iter.destroy(&iter);
    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

int test_iterator_get(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Add numbers 1-3
    for (int i = 1; i <= 3; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_arraylist_push_back(list, val);
    }

    ANVIterator iter = anv_arraylist_iterator(list);

    // Test get without advancing
    int* val = (int*)iter.get(&iter);
    ASSERT_NOT_NULL(val);
    ASSERT_EQ(*val, 1);

    // Get again - should return same value
    val = (int*)iter.get(&iter);
    ASSERT_EQ(*val, 1);

    // Now advance and test get
    iter.next(&iter);
    val = (int*)iter.get(&iter);
    ASSERT_EQ(*val, 2);

    iter.destroy(&iter);
    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

int test_iterator_prev(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Add numbers 1-3
    for (int i = 1; i <= 3; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_arraylist_push_back(list, val);
    }

    ANVIterator iter = anv_arraylist_iterator(list);
    iter.next(&iter); // Move to 2
    iter.next(&iter); // Move to 3

    // Test has_prev and prev
    ASSERT(iter.has_prev(&iter));
    iter.prev(&iter);
    const int* val = (int*)iter.get(&iter);
    ASSERT_EQ(*val, 2);

    ASSERT(iter.has_prev(&iter));
    iter.prev(&iter);
    val = (int*)iter.get(&iter);
    ASSERT_EQ(*val, 1);

    ASSERT(!iter.has_prev(&iter));

    iter.destroy(&iter);
    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

int test_iterator_reset(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Add numbers 1-3
    for (int i = 1; i <= 3; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_arraylist_push_back(list, val);
    }

    ANVIterator iter = anv_arraylist_iterator(list);

    // Advance iterator
    iter.next(&iter);
    iter.next(&iter);

    // Reset and verify back at beginning
    iter.reset(&iter);
    int* val = (int*)iter.get(&iter);
    ASSERT_EQ(*val, 1);
    ASSERT(iter.has_next(&iter));

    iter.destroy(&iter);
    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

int test_iterator_empty_list(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    ANVIterator iter = anv_arraylist_iterator(list);
    ASSERT(iter.is_valid(&iter));
    ASSERT(!iter.has_next(&iter));
    ASSERT(!iter.has_prev(&iter));
    ASSERT_EQ(iter.next(&iter), -1); // Should return error code
    ASSERT_NULL(iter.get(&iter));

    iter.destroy(&iter);
    anv_arraylist_destroy(list, false);
    return TEST_SUCCESS;
}

int test_iterator_single_element(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    int* val = malloc(sizeof(int));
    *val = 42;
    anv_arraylist_push_back(list, val);

    ANVIterator iter = anv_arraylist_iterator(list);

    ASSERT(iter.has_next(&iter));
    ASSERT(!iter.has_prev(&iter));

    const int* retrieved = iter.get(&iter);
    ASSERT_EQ(*retrieved, 42);

    iter.next(&iter);
    ASSERT(!iter.has_next(&iter));
    ASSERT(iter.has_prev(&iter));

    iter.destroy(&iter);
    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

// Test creating arraylist from iterator
int test_from_iterator(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create a range iterator (0, 1, 2, 3, 4)
    ANVIterator range_it = anv_iterator_range(&alloc, 0, 5, 1);

    // Create arraylist from iterator
    ANVArrayList* list = anv_arraylist_from_iterator(&range_it, &alloc, true);
    ASSERT_NOT_NULL(list);
    ASSERT_EQ(anv_arraylist_size(list), 5);

    // Clean up the iterator immediately after use
    range_it.destroy(&range_it);

    // Verify arraylist has correct values in sequential order
    // Iterator gives 0,1,2,3,4 and arraylist should have them as 0,1,2,3,4 (index order)
    for (int expected = 0; expected < 5; expected++)
    {
        void* data = anv_arraylist_get(list, expected);
        ASSERT_NOT_NULL(data);
        ASSERT_EQ(*(int*)data, expected);
    }

    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

// Test iterator with invalid arraylist
int test_iterator_invalid(void)
{
    const ANVIterator iter = anv_arraylist_iterator(NULL);
    ASSERT(!iter.is_valid(&iter));
    return TEST_SUCCESS;
}

// Test iterator state after arraylist modifications
int test_iterator_modification(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Add initial data
    for (int i = 0; i < 3; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i * 10;
        ASSERT_EQ(anv_arraylist_push_back(list, data), 0);
    }

    ANVIterator iter = anv_arraylist_iterator(list);
    ASSERT(iter.is_valid(&iter));

    // Get first element
    void* first = iter.get(&iter);
    ASSERT_EQ(*(int*)first, 0); // Should be first element (0*10)
    iter.next(&iter);

    // Modify arraylist while iterator exists (implementation detail: iterator may become invalid)
    int* new_data = malloc(sizeof(int));
    *new_data = 999;
    ASSERT_EQ(anv_arraylist_push_back(list, new_data), 0);

    // Iterator should still be valid but may not reflect new state
    ASSERT(iter.is_valid(&iter));

    iter.destroy(&iter);
    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

// Test copy isolation - verify that copied elements are independent
int test_arraylist_copy_isolation(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create original data that we can modify
    int original_values[] = {10, 20, 30};
    int* data_ptrs[3];

    // Create a source arraylist
    ANVArrayList* source_list = anv_arraylist_create(&alloc, 0);
    ASSERT_NOT_NULL(source_list);

    for (int i = 0; i < 3; i++)
    {
        data_ptrs[i] = malloc(sizeof(int));
        *data_ptrs[i] = original_values[i];
        ASSERT_EQ(anv_arraylist_push_back(source_list, data_ptrs[i]), 0);
    }

    ANVIterator list_it = anv_arraylist_iterator(source_list);
    ASSERT(list_it.is_valid(&list_it));

    // Create arraylist with copying enabled
    ANVArrayList* new_list = anv_arraylist_from_iterator(&list_it, &alloc, true);
    ASSERT_NOT_NULL(new_list);
    ASSERT_EQ(anv_arraylist_size(new_list), 3);

    // Modify original data
    *data_ptrs[0] = 999;
    *data_ptrs[1] = 888;
    *data_ptrs[2] = 777;

    // ArrayList should still have original values (proving data was copied)
    // Sequential order: 10, 20, 30
    void* arraylist_data = anv_arraylist_get(new_list, 0);
    ASSERT_NOT_NULL(arraylist_data);
    ASSERT_EQ(*(int*)arraylist_data, 10); // Should be unchanged

    arraylist_data = anv_arraylist_get(new_list, 1);
    ASSERT_NOT_NULL(arraylist_data);
    ASSERT_EQ(*(int*)arraylist_data, 20); // Should be unchanged

    arraylist_data = anv_arraylist_get(new_list, 2);
    ASSERT_NOT_NULL(arraylist_data);
    ASSERT_EQ(*(int*)arraylist_data, 30); // Should be unchanged

    // Cleanup
    list_it.destroy(&list_it);
    anv_arraylist_destroy(new_list, true);
    anv_arraylist_destroy(source_list, true);

    return TEST_SUCCESS;
}

// Test that should_copy=true fails when allocator has no copy function
int test_arraylist_anv_copy_function_required(void)
{
    ANVAllocator alloc = anv_alloc_default();
    alloc.copy = NULL;

    ANVIterator range_it = anv_iterator_range(&alloc, 0, 3, 1);
    ASSERT(range_it.is_valid(&range_it));

    // Should return NULL because should_copy=true but no copy function available
    ANVArrayList* list = anv_arraylist_from_iterator(&range_it, &alloc, true);
    ASSERT_NULL(list);

    range_it.destroy(&range_it);
    return TEST_SUCCESS;
}

// Test that should_copy=false uses elements directly without copying
int test_arraylist_from_iterator_no_copy(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create a range iterator and then a copy iterator to get actual owned data
    ANVIterator range_it = anv_iterator_range(&alloc, 0, 3, 1);
    ASSERT(range_it.is_valid(&range_it));

    // Use copy iterator to create actual data elements that we own
    ANVIterator copy_it = anv_iterator_copy(&range_it, &alloc, int_copy);
    ASSERT(copy_it.is_valid(&copy_it));

    // Create arraylist without copying (should_copy = false)
    // This will use the copied elements directly from the copy iterator
    ANVArrayList* list = anv_arraylist_from_iterator(&copy_it, &alloc, false);
    ASSERT_NOT_NULL(list);
    ASSERT_EQ(anv_arraylist_size(list), 3);

    // Verify values are correct (sequential order: 0, 1, 2)
    void* data = anv_arraylist_get(list, 0);
    ASSERT_NOT_NULL(data);
    ASSERT_EQ(*(int*)data, 0);

    data = anv_arraylist_get(list, 1);
    ASSERT_NOT_NULL(data);
    ASSERT_EQ(*(int*)data, 1);

    data = anv_arraylist_get(list, 2);
    ASSERT_NOT_NULL(data);
    ASSERT_EQ(*(int*)data, 2);

    range_it.destroy(&range_it);
    copy_it.destroy(&copy_it);
    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

// Test that iterator is exhausted after being consumed by anv_arraylist_from_iterator
int test_iterator_exhaustion_after_arraylist_creation(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVIterator range_it = anv_iterator_range(&alloc, 0, 5, 1);
    ASSERT(range_it.is_valid(&range_it));

    // Verify iterator starts with elements
    ASSERT(range_it.has_next(&range_it));

    // Create arraylist from iterator (consumes all elements)
    ANVArrayList* list = anv_arraylist_from_iterator(&range_it, &alloc, true);
    ASSERT_NOT_NULL(list);
    ASSERT_EQ(anv_arraylist_size(list), 5);

    // Iterator should now be exhausted
    ASSERT(!range_it.has_next(&range_it));
    ASSERT_NULL(range_it.get(&range_it));
    ASSERT_EQ(range_it.next(&range_it), -1); // Should fail to advance

    // But iterator should still be valid
    ASSERT(range_it.is_valid(&range_it));

    range_it.destroy(&range_it);
    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

// Test next() return values for proper error handling
int test_arraylist_iterator_next_return_values(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);
    ASSERT_NOT_NULL(list);

    // Add single element
    int* data = malloc(sizeof(int));
    *data = 42;
    ASSERT_EQ(anv_arraylist_push_back(list, data), 0);

    ANVIterator iter = anv_arraylist_iterator(list);
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
    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

// Test various combinations of get/next/has_next calls for consistency
int test_arraylist_iterator_mixed_operations(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);
    ASSERT_NOT_NULL(list);

    // Add test data (will be in sequential order: 0, 10, 20)
    for (int i = 0; i < 3; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i * 10;
        ASSERT_EQ(anv_arraylist_push_back(list, data), 0);
    }

    ANVIterator iter = anv_arraylist_iterator(list);
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
    // Note: data1 and data3 point to different arraylist elements
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
    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

int test_bidirectional_iteration(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Add numbers 1-5
    for (int i = 1; i <= 5; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_arraylist_push_back(list, val);
    }

    ANVIterator iter = anv_arraylist_iterator(list);

    // Move forward to middle
    iter.next(&iter); // Move to 2
    iter.next(&iter); // Move to 3

    const int* val = iter.get(&iter);
    ASSERT_EQ(*val, 3);

    // Move back
    iter.prev(&iter);
    val = iter.get(&iter);
    ASSERT_EQ(*val, 2);

    // Move forward again
    iter.next(&iter);
    val = iter.get(&iter);
    ASSERT_EQ(*val, 3);

    iter.destroy(&iter);
    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

// Test iterator traversal order
int test_arraylist_iterator_order(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Add elements in specific order
    const int values[] = {100, 200, 300, 400, 500};
    for (int i = 0; i < 5; i++)
    {
        int* data = malloc(sizeof(int));
        *data = values[i];
        ASSERT_EQ(anv_arraylist_push_back(list, data), 0);
    }

    // Create iterator and verify order
    ANVIterator iter = anv_arraylist_iterator(list);

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
    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Algorithm Tests
//==============================================================================

int test_sort(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Add unsorted elements
    for (int i = 0; i < 6; i++)
    {
        const int values[] = {5, 2, 8, 1, 9, 3};
        int* val = malloc(sizeof(int));
        *val = values[i];
        anv_arraylist_push_back(list, val);
    }

    ASSERT_EQ(anv_arraylist_sort(list, int_cmp), 0);

    // Verify sorted order
    for (int i = 0; i < 6; i++)
    {
        const int expected[] = {1, 2, 3, 5, 8, 9};
        ASSERT_EQ(*(int*)anv_arraylist_get(list, i), expected[i]);
    }

    anv_arraylist_destroy(list, true);

    return TEST_SUCCESS;
}

int test_sort_empty(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Sort empty list should succeed
    ASSERT_EQ(anv_arraylist_sort(list, int_cmp), 0);
    ASSERT_EQ(anv_arraylist_size(list), 0);

    anv_arraylist_destroy(list, false);

    return TEST_SUCCESS;
}

int test_sort_single_element(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    int* val = malloc(sizeof(int));
    *val = 42;
    anv_arraylist_push_back(list, val);

    ASSERT_EQ(anv_arraylist_sort(list, int_cmp), 0);
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 0), 42);

    anv_arraylist_destroy(list, true);

    return TEST_SUCCESS;
}

int test_sort_already_sorted(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Add already sorted elements
    for (int i = 1; i <= 10; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_arraylist_push_back(list, val);
    }

    ASSERT_EQ(anv_arraylist_sort(list, int_cmp), 0);

    // Verify still sorted
    for (int i = 0; i < 10; i++)
    {
        ASSERT_EQ(*(int*)anv_arraylist_get(list, i), i + 1);
    }

    anv_arraylist_destroy(list, true);

    return TEST_SUCCESS;
}

int test_sort_reverse_sorted(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Add reverse sorted elements
    for (int i = 10; i >= 1; i--)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_arraylist_push_back(list, val);
    }

    ASSERT_EQ(anv_arraylist_sort(list, int_cmp), 0);

    // Verify now sorted
    for (int i = 0; i < 10; i++)
    {
        ASSERT_EQ(*(int*)anv_arraylist_get(list, i), i + 1);
    }

    anv_arraylist_destroy(list, true);

    return TEST_SUCCESS;
}

int test_reverse(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Add elements 1, 2, 3, 4, 5
    for (int i = 1; i <= 5; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_arraylist_push_back(list, val);
    }

    ASSERT_EQ(anv_arraylist_reverse(list), 0);

    // Verify reversed order: 5, 4, 3, 2, 1
    for (int i = 0; i < 5; i++)
    {
        ASSERT_EQ(*(int*)anv_arraylist_get(list, i), 5 - i);
    }

    anv_arraylist_destroy(list, true);

    return TEST_SUCCESS;
}

int test_reverse_empty(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Reverse empty list should succeed
    ASSERT_EQ(anv_arraylist_reverse(list), 0);
    ASSERT_EQ(anv_arraylist_size(list), 0);

    anv_arraylist_destroy(list, false);

    return TEST_SUCCESS;
}

int test_reverse_single_element(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    int* val = malloc(sizeof(int));
    *val = 42;
    anv_arraylist_push_back(list, val);

    ASSERT_EQ(anv_arraylist_reverse(list), 0);
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 0), 42);

    anv_arraylist_destroy(list, true);

    return TEST_SUCCESS;
}

int test_filter(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Add numbers 1-10
    for (int i = 1; i <= 10; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_arraylist_push_back(list, val);
    }

    // Filter even numbers
    ANVArrayList* filtered = anv_arraylist_filter(list, is_even);
    ASSERT_NOT_NULL(filtered);
    ASSERT_EQ(anv_arraylist_size(filtered), 5);

    // Verify even numbers: 2, 4, 6, 8, 10
    for (int i = 0; i < 5; i++)
    {
        const int expected_evens[] = {2, 4, 6, 8, 10};
        ASSERT_EQ(*(int*)anv_arraylist_get(filtered, i), expected_evens[i]);
    }

    anv_arraylist_destroy(filtered, false);
    anv_arraylist_destroy(list, true);

    return TEST_SUCCESS;
}

// New test: deep filter should produce copies of matching elements
int test_filter_deep(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Add numbers 1-10
    for (int i = 1; i <= 10; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_arraylist_push_back(list, val);
    }

    ANVArrayList* filtered = anv_arraylist_filter_deep(list, is_even);
    ASSERT_NOT_NULL(filtered);
    ASSERT_EQ(anv_arraylist_size(filtered), 5);

    // Verify values are correct and that pointers are different (deep copied)
    for (size_t i = 0; i < anv_arraylist_size(filtered); i++)
    {
        int* filtered_val = anv_arraylist_get(filtered, i);
        int* original_val = anv_arraylist_get(list, i * 2 + 1); // original even positions: 1,3,5,...
        ASSERT_NOT_NULL(filtered_val);
        ASSERT_NOT_NULL(original_val);
        ASSERT_EQ(*(int*)filtered_val, *(int*)original_val);
        ASSERT_NOT_EQ_PTR(filtered_val, original_val);
    }

    // Free deep-copied data in filtered list
    anv_arraylist_destroy(filtered, true);
    anv_arraylist_destroy(list, true);

    return TEST_SUCCESS;
}

// New test: deep filter on empty list should return an empty list
int test_filter_deep_empty(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    ANVArrayList* filtered = anv_arraylist_filter_deep(list, is_even);
    ASSERT_NOT_NULL(filtered);
    ASSERT_EQ(anv_arraylist_size(filtered), 0);

    anv_arraylist_destroy(filtered, false);
    anv_arraylist_destroy(list, false);

    return TEST_SUCCESS;
}

int test_transform(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Add numbers 1-5
    for (int i = 1; i <= 5; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_arraylist_push_back(list, val);
    }

    // Transform by doubling
    ANVArrayList* transformed = anv_arraylist_transform(list, double_value, false);
    ASSERT_NOT_NULL(transformed);
    ASSERT_EQ(anv_arraylist_size(transformed), 5);

    // Verify doubled values: 2, 4, 6, 8, 10
    for (int i = 0; i < 5; i++)
    {
        ASSERT_EQ(*(int*)anv_arraylist_get(transformed, i), (i + 1) * 2);
    }

    anv_arraylist_destroy(transformed, true); // Free transformed data
    anv_arraylist_destroy(list, true);

    return TEST_SUCCESS;
}

int test_for_each(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    for_each_sum = 0;

    // Add numbers 1-5
    for (int i = 1; i <= 5; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_arraylist_push_back(list, val);
    }

    anv_arraylist_for_each(list, add_to_sum);

    // Sum should be 1+2+3+4+5 = 15
    ASSERT_EQ(for_each_sum, 15);

    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Memory Tests
//==============================================================================

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

//==============================================================================
// Properties Tests
//==============================================================================

int test_equals(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list1 = anv_arraylist_create(&alloc, 0);
    ANVArrayList* list2 = anv_arraylist_create(&alloc, 0);

    // Test empty lists
    ASSERT_EQ(anv_arraylist_equals(list1, list2, int_cmp), 1);

    // Add same elements to both
    for (int i = 1; i <= 3; i++)
    {
        int* val1 = malloc(sizeof(int));
        int* val2 = malloc(sizeof(int));
        *val1 = i;
        *val2 = i;
        anv_arraylist_push_back(list1, val1);
        anv_arraylist_push_back(list2, val2);
    }

    ASSERT_EQ(anv_arraylist_equals(list1, list2, int_cmp), 1);

    // Add different element to list2
    int* val = malloc(sizeof(int));
    *val = 99;
    anv_arraylist_push_back(list2, val);

    ASSERT_EQ(anv_arraylist_equals(list1, list2, int_cmp), 0);

    anv_arraylist_destroy(list1, true);
    anv_arraylist_destroy(list2, true);
    return TEST_SUCCESS;
}

int test_equals_different_sizes(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list1 = anv_arraylist_create(&alloc, 0);
    ANVArrayList* list2 = anv_arraylist_create(&alloc, 0);

    int* val1 = malloc(sizeof(int));
    *val1 = 1;
    int* val2 = malloc(sizeof(int));
    *val2 = 1;
    int* val3 = malloc(sizeof(int));
    *val3 = 2;

    anv_arraylist_push_back(list1, val1);
    anv_arraylist_push_back(list2, val2);
    anv_arraylist_push_back(list2, val3);

    ASSERT_EQ(anv_arraylist_equals(list1, list2, int_cmp), 0);

    anv_arraylist_destroy(list1, true);
    anv_arraylist_destroy(list2, true);
    return TEST_SUCCESS;
}

int test_copy_shallow(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* original = anv_arraylist_create(&alloc, 0);

    // Add numbers 1-3
    for (int i = 1; i <= 3; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_arraylist_push_back(original, val);
    }

    ANVArrayList* copy = anv_arraylist_copy(original);
    ASSERT_NOT_NULL(copy);
    ASSERT_EQ(anv_arraylist_size(copy), 3);
    ASSERT_EQ(anv_arraylist_equals(original, copy, int_cmp), 1);

    // Verify shallow copy (same data pointers)
    for (int i = 0; i < 3; i++)
    {
        ASSERT_EQ_PTR(anv_arraylist_get(original, i), anv_arraylist_get(copy, i));
    }

    anv_arraylist_destroy(copy, false); // Don't free shared data
    anv_arraylist_destroy(original, true);
    return TEST_SUCCESS;
}

int test_copy_deep(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* original = anv_arraylist_create(&alloc, 0);

    // Add numbers 1-3
    for (int i = 1; i <= 3; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_arraylist_push_back(original, val);
    }

    ANVArrayList* copy = anv_arraylist_copy_deep(original, false);
    ASSERT_NOT_NULL(copy);
    ASSERT_EQ(anv_arraylist_size(copy), 3);
    ASSERT_EQ(anv_arraylist_equals(original, copy, int_cmp), 1);

    // Verify deep copy (different data pointers, same values)
    for (int i = 0; i < 3; i++)
    {
        ASSERT_NOT_EQ_PTR(anv_arraylist_get(original, i), anv_arraylist_get(copy, i));
        ASSERT_EQ(*(int*)anv_arraylist_get(original, i), *(int*)anv_arraylist_get(copy, i));
    }

    anv_arraylist_destroy(copy, true); // Free copied data
    anv_arraylist_destroy(original, true);
    return TEST_SUCCESS;
}

int test_boundary_conditions(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Test operations on empty list
    ASSERT_NULL(anv_arraylist_get(list, 0));
    ASSERT_NULL(anv_arraylist_front(list));
    ASSERT_NULL(anv_arraylist_back(list));
    ASSERT_EQ(anv_arraylist_remove_at(list, 0, false), -1);
    ASSERT_EQ(anv_arraylist_pop_back(list, false), -1);
    ASSERT_EQ(anv_arraylist_pop_front(list, false), -1);

    // Test invalid indices
    int* val = malloc(sizeof(int));
    *val = 42;
    anv_arraylist_push_back(list, val);

    ASSERT_NULL(anv_arraylist_get(list, 1));
    ASSERT_EQ(anv_arraylist_set(list, 1, val, false), -1);
    ASSERT_EQ(anv_arraylist_remove_at(list, 1, false), -1);
    ASSERT_EQ(anv_arraylist_insert(list, 2, val), -1); // Can insert at size, but not size+1

    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

int test_null_parameters(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Test NULL list parameter
    ASSERT_EQ(anv_arraylist_size(NULL), 0);
    ASSERT_EQ(anv_arraylist_capacity(NULL), 0);
    ASSERT(anv_arraylist_is_empty(NULL));
    ASSERT_NULL(anv_arraylist_get(NULL, 0));
    ASSERT_EQ(anv_arraylist_push_back(NULL, &list), -1);

    // Test NULL allocator
    ANVArrayList* null_alloc_list = anv_arraylist_create(NULL, 0);
    ASSERT_NULL(null_alloc_list);

    anv_arraylist_destroy(list, false);
    return TEST_SUCCESS;
}

int test_size_consistency(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    ASSERT_EQ(anv_arraylist_size(list), 0);
    ASSERT(anv_arraylist_is_empty(list));

    // Add elements and verify size
    for (int i = 0; i < 10; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_arraylist_push_back(list, val);
        ASSERT_EQ(anv_arraylist_size(list), (size_t)i + 1);
        ASSERT(!anv_arraylist_is_empty(list));
    }

    // Remove elements and verify size
    for (int i = 9; i >= 0; i--)
    {
        anv_arraylist_pop_back(list, true);
        ASSERT_EQ(anv_arraylist_size(list), (size_t)i);
        if (i == 0)
        {
            ASSERT(anv_arraylist_is_empty(list));
        }
        else
        {
            ASSERT(!anv_arraylist_is_empty(list));
        }
    }

    anv_arraylist_destroy(list, false);
    return TEST_SUCCESS;
}

int test_data_integrity_after_operations(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Add initial data
    for (int i = 0; i < 10; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i * 10; // 0, 10, 20, ..., 90
        anv_arraylist_push_back(list, val);
    }

    // Insert in middle
    int* val = malloc(sizeof(int));
    *val = 99;
    anv_arraylist_insert(list, 5, val);

    // Verify data integrity
    for (int i = 0; i < 5; i++)
    {
        ASSERT_EQ(*(int*)anv_arraylist_get(list, i), i * 10);
    }
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 5), 99);
    for (int i = 6; i < 11; i++)
    {
        ASSERT_EQ(*(int*)anv_arraylist_get(list, i), (i - 1) * 10);
    }

    // Remove from middle
    anv_arraylist_remove_at(list, 5, true);

    // Verify data integrity restored
    for (int i = 0; i < 10; i++)
    {
        ASSERT_EQ(*(int*)anv_arraylist_get(list, i), i * 10);
    }

    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

int test_large_data_set(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    const int NUM_ELEMENTS = 10000;

    // Add large number of elements
    for (int i = 0; i < NUM_ELEMENTS; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        ASSERT_EQ(anv_arraylist_push_back(list, val), 0);
    }

    ASSERT_EQ(anv_arraylist_size(list), (size_t)NUM_ELEMENTS);

    // Spot check values
    ASSERT_EQ(*(int*)anv_arraylist_get(list, 0), 0);
    ASSERT_EQ(*(int*)anv_arraylist_get(list, NUM_ELEMENTS / 2), NUM_ELEMENTS / 2);
    ASSERT_EQ(*(int*)anv_arraylist_get(list, NUM_ELEMENTS - 1), NUM_ELEMENTS - 1);

    // Remove half the elements
    for (int i = 0; i < NUM_ELEMENTS / 2; i++)
    {
        anv_arraylist_pop_back(list, true);
    }

    ASSERT_EQ(anv_arraylist_size(list), (size_t)NUM_ELEMENTS / 2);
    ASSERT_EQ(*(int*)anv_arraylist_back(list), NUM_ELEMENTS / 2 - 1);

    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Main - Combined Test Runner (71 tests total)
//==============================================================================

int main(void)
{
    const ANVTestCase tests[] = {
        // CRUD Tests (12)
        {test_create_destroy, "test_create_destroy"},
        {test_create_with_capacity, "test_create_with_capacity"},
        {test_push_back, "test_push_back"},
        {test_push_front, "test_push_front"},
        {test_insert_at, "test_insert_at"},
        {test_get_set, "test_get_set"},
        {test_front_back, "test_front_back"},
        {test_remove_at, "test_remove_at"},
        {test_pop_back_front, "test_pop_back_front"},
        {test_find, "test_find"},
        {test_remove, "test_remove"},
        {test_clear, "test_clear"},

        // Boundary Tests (10)
        {test_single_element_lifecycle, "test_single_element_lifecycle"},
        {test_insert_at_boundaries, "test_insert_at_boundaries"},
        {test_get_out_of_bounds, "test_get_out_of_bounds"},
        {test_remove_at_boundaries, "test_remove_at_boundaries"},
        {test_pop_empty_list, "test_pop_empty_list"},
        {test_clear_empty_list, "test_clear_empty_list"},
        {test_front_back_empty, "test_front_back_empty"},
        {test_capacity_growth, "test_capacity_growth"},
        {test_null_data, "test_null_data"},
        {test_null_list_operations, "test_null_list_operations"},

        // Iterator Tests (18)
        {test_forward_iterator, "test_forward_iterator"},
        {test_reverse_iterator, "test_reverse_iterator"},
        {test_iterator_get, "test_iterator_get"},
        {test_iterator_prev, "test_iterator_prev"},
        {test_iterator_reset, "test_iterator_reset"},
        {test_iterator_empty_list, "test_iterator_empty_list"},
        {test_iterator_single_element, "test_iterator_single_element"},
        {test_from_iterator, "test_from_iterator"},
        {test_iterator_invalid, "test_iterator_invalid"},
        {test_iterator_modification, "test_iterator_modification"},
        {test_arraylist_copy_isolation, "test_arraylist_copy_isolation"},
        {test_arraylist_anv_copy_function_required, "test_arraylist_anv_copy_function_required"},
        {test_arraylist_from_iterator_no_copy, "test_arraylist_from_iterator_no_copy"},
        {test_iterator_exhaustion_after_arraylist_creation, "test_iterator_exhaustion_after_arraylist_creation"},
        {test_arraylist_iterator_next_return_values, "test_arraylist_iterator_next_return_values"},
        {test_arraylist_iterator_mixed_operations, "test_arraylist_iterator_mixed_operations"},
        {test_bidirectional_iteration, "test_bidirectional_iteration"},
        {test_arraylist_iterator_order, "test_arraylist_iterator_order"},

        // Algorithm Tests (13)
        {test_sort, "test_sort"},
        {test_sort_empty, "test_sort_empty"},
        {test_sort_single_element, "test_sort_single_element"},
        {test_sort_already_sorted, "test_sort_already_sorted"},
        {test_sort_reverse_sorted, "test_sort_reverse_sorted"},
        {test_reverse, "test_reverse"},
        {test_reverse_empty, "test_reverse_empty"},
        {test_reverse_single_element, "test_reverse_single_element"},
        {test_filter, "test_filter"},
        {test_filter_deep, "test_filter_deep"},
        {test_filter_deep_empty, "test_filter_deep_empty"},
        {test_transform, "test_transform"},
        {test_for_each, "test_for_each"},

        // Memory Tests (9)
        {test_reserve, "test_reserve"},
        {test_shrink_to_fit, "test_shrink_to_fit"},
        {test_shrink_empty_list, "test_shrink_empty_list"},
        {test_growth_pattern, "test_growth_pattern"},
        {test_memory_allocation_failure, "test_memory_allocation_failure"},
        {test_large_capacity, "test_large_capacity"},
        {test_memory_cleanup_on_destroy, "test_memory_cleanup_on_destroy"},
        {test_memory_cleanup_on_clear, "test_memory_cleanup_on_clear"},
        {test_capacity_consistency, "test_capacity_consistency"},

        // Properties Tests (9)
        {test_equals, "test_equals"},
        {test_equals_different_sizes, "test_equals_different_sizes"},
        {test_copy_shallow, "test_copy_shallow"},
        {test_copy_deep, "test_copy_deep"},
        {test_boundary_conditions, "test_boundary_conditions"},
        {test_null_parameters, "test_null_parameters"},
        {test_size_consistency, "test_size_consistency"},
        {test_data_integrity_after_operations, "test_data_integrity_after_operations"},
        {test_large_data_set, "test_large_data_set"},
    };

    return anv_run_tests("ArrayList", tests, sizeof(tests) / sizeof(tests[0]));
}
