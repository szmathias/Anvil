//
// Created by zack on 9/25/25.
//
// Binary Search Tree iterator operations test

#include <stdio.h>
#include <stdlib.h>

#include "containers/binarysearchtree.h"
#include "TestAssert.h"
#include "TestHelpers.h"

// Test basic in-order iterator functionality
int test_bst_iterator_inorder(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    // Create balanced tree: 50, 30, 70, 20, 40, 60, 80
    int values[] = {50, 30, 70, 20, 40, 60, 80};
    int* data[7];

    for (int i = 0; i < 7; i++)
    {
        data[i] = malloc(sizeof(int));
        *(data[i]) = values[i];
        ASSERT_EQ(anv_bst_insert(bst, data[i]), 0);
    }

    ANVIterator it = anv_bst_iterator(bst);
    ASSERT(it.is_valid(&it));

    // Expected order: 20, 30, 40, 50, 60, 70, 80
    int expected[] = {20, 30, 40, 50, 60, 70, 80};
    int index = 0;

    while (it.has_next(&it))
    {
        void* current = it.get(&it);
        ASSERT_NOT_NULL(current);
        ASSERT_EQ(*(int*)current, expected[index]);
        index++;

        if (it.next(&it) != 0)
            break;
    }

    ASSERT_EQ(index, 7);
    it.destroy(&it);
    anv_bst_destroy(bst, true);
    return TEST_SUCCESS;
}

// Test pre-order iterator functionality
int test_bst_iterator_preorder(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    // Same tree structure
    int values[] = {50, 30, 70, 20, 40, 60, 80};
    int* data[7];

    for (int i = 0; i < 7; i++)
    {
        data[i] = malloc(sizeof(int));
        *(data[i]) = values[i];
        ASSERT_EQ(anv_bst_insert(bst, data[i]), 0);
    }

    ANVIterator it = anv_bst_iterator_preorder(bst);
    ASSERT(it.is_valid(&it));

    // Expected order: 50, 30, 20, 40, 70, 60, 80
    int expected[] = {50, 30, 20, 40, 70, 60, 80};
    int index = 0;

    while (it.has_next(&it))
    {
        void* current = it.get(&it);
        ASSERT_NOT_NULL(current);
        ASSERT_EQ(*(int*)current, expected[index]);
        index++;

        if (it.next(&it) != 0)
            break;
    }

    ASSERT_EQ(index, 7);
    it.destroy(&it);
    anv_bst_destroy(bst, true);
    return TEST_SUCCESS;
}

// Test post-order iterator functionality
int test_bst_iterator_postorder(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    // Same tree structure
    int values[] = {50, 30, 70, 20, 40, 60, 80};
    int* data[7];

    for (int i = 0; i < 7; i++)
    {
        data[i] = malloc(sizeof(int));
        *(data[i]) = values[i];
        ASSERT_EQ(anv_bst_insert(bst, data[i]), 0);
    }

    ANVIterator it = anv_bst_iterator_postorder(bst);
    ASSERT(it.is_valid(&it));

    // Expected order: 20, 40, 30, 60, 80, 70, 50
    int expected[] = {20, 40, 30, 60, 80, 70, 50};
    int index = 0;

    while (it.has_next(&it))
    {
        void* current = it.get(&it);
        ASSERT_NOT_NULL(current);
        ASSERT_EQ(*(int*)current, expected[index]);
        index++;

        if (it.next(&it) != 0)
            break;
    }

    ASSERT_EQ(index, 7);
    it.destroy(&it);
    anv_bst_destroy(bst, true);
    return TEST_SUCCESS;
}

// Test iterator on empty tree
int test_bst_iterator_empty(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    ANVIterator it = anv_bst_iterator(bst);
    ASSERT(it.is_valid(&it));
    ASSERT(!it.has_next(&it));
    ASSERT_NULL(it.get(&it));

    it.destroy(&it);
    anv_bst_destroy(bst, false);
    return TEST_SUCCESS;
}

// Test iterator on single node tree
int test_bst_iterator_single_node(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    int* data = malloc(sizeof(int));
    *data = 42;
    ASSERT_EQ(anv_bst_insert(bst, data), 0);

    // Test all iterator types
    ANVIterator iterators[3] = {
        anv_bst_iterator(bst),
        anv_bst_iterator_preorder(bst),
        anv_bst_iterator_postorder(bst)
    };

    for (int i = 0; i < 3; i++)
    {
        ASSERT(iterators[i].is_valid(&iterators[i]));
        ASSERT(iterators[i].has_next(&iterators[i]));

        int* current = iterators[i].get(&iterators[i]);
        ASSERT_NOT_NULL(current);
        ASSERT_EQ(*(int*)current, 42);

        ASSERT_EQ(iterators[i].next(&iterators[i]), 0);
        ASSERT(!iterators[i].has_next(&iterators[i]));

        iterators[i].destroy(&iterators[i]);
    }

    anv_bst_destroy(bst, true);
    return TEST_SUCCESS;
}

// Test iterator reset functionality
int test_bst_iterator_reset(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    int values[] = {50, 30, 70};
    int* data[3];

    for (int i = 0; i < 3; i++)
    {
        data[i] = malloc(sizeof(int));
        *(data[i]) = values[i];
        ASSERT_EQ(anv_bst_insert(bst, data[i]), 0);
    }

    ANVIterator it = anv_bst_iterator(bst);

    // Iterate through once
    int count1 = 0;
    while (it.has_next(&it))
    {
        it.get(&it);
        count1++;
        if (it.next(&it) != 0)
            break;
    }
    ASSERT_EQ(count1, 3);

    // Reset and iterate again
    it.reset(&it);
    int count2 = 0;
    while (it.has_next(&it))
    {
        it.get(&it);
        count2++;
        if (it.next(&it) != 0)
            break;
    }
    ASSERT_EQ(count2, 3);

    it.destroy(&it);
    anv_bst_destroy(bst, true);
    return TEST_SUCCESS;
}

// Test iterator backward operations (should not be supported)
int test_bst_iterator_backward(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    int* data = malloc(sizeof(int));
    *data = 42;
    ASSERT_EQ(anv_bst_insert(bst, data), 0);

    ANVIterator it = anv_bst_iterator(bst);

    // BST iterators should not support backward iteration
    ASSERT(!it.has_prev(&it));
    ASSERT_EQ(it.prev(&it), -1);

    it.destroy(&it);
    anv_bst_destroy(bst, true);
    return TEST_SUCCESS;
}

// Test creating BST from iterator
int test_bst_from_iterator(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* source_bst = anv_bst_create(&alloc, int_cmp);

    // Create source tree with known values
    int values[] = {50, 30, 70, 20, 40};
    int* data[5];

    for (int i = 0; i < 5; i++)
    {
        data[i] = malloc(sizeof(int));
        *(data[i]) = values[i];
        ASSERT_EQ(anv_bst_insert(source_bst, data[i]), 0);
    }

    // Create iterator and new BST from it
    ANVIterator it = anv_bst_iterator(source_bst);
    ANVBinarySearchTree* new_bst = anv_bst_from_iterator(&it, &alloc, int_cmp, true);

    ASSERT_NOT_NULL(new_bst);
    ASSERT_EQ(anv_bst_size(new_bst), 5);

    // Verify all values are present in new BST
    for (int i = 0; i < 5; i++)
    {
        ASSERT(anv_bst_contains(new_bst, &values[i]));
    }

    it.destroy(&it);
    anv_bst_destroy(source_bst, true);
    anv_bst_destroy(new_bst, true);
    return TEST_SUCCESS;
}

// Test iterator with NULL parameters
int test_bst_iterator_null_params(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Test creating iterator with NULL tree
    ANVIterator it = anv_bst_iterator(NULL);
    ASSERT(!it.is_valid(&it));

    // Test anv_bst_from_iterator with NULL parameters
    ASSERT_NULL(anv_bst_from_iterator(NULL, &alloc, int_cmp, false));

    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);
    ANVIterator valid_it = anv_bst_iterator(bst);

    ASSERT_NULL(anv_bst_from_iterator(&valid_it, NULL, int_cmp, false));
    ASSERT_NULL(anv_bst_from_iterator(&valid_it, &alloc, NULL, false));

    valid_it.destroy(&valid_it);
    anv_bst_destroy(bst, false);
    return TEST_SUCCESS;
}

// Test iterator on complex tree structure
int test_bst_iterator_complex(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    // Insert values to create a more complex tree
    int values[] = {50, 25, 75, 12, 37, 62, 87, 6, 18, 31, 43};
    int* data[11];

    for (int i = 0; i < 11; i++)
    {
        data[i] = malloc(sizeof(int));
        *(data[i]) = values[i];
        ASSERT_EQ(anv_bst_insert(bst, data[i]), 0);
    }

    // Test in-order iterator (should be sorted)
    ANVIterator it = anv_bst_iterator(bst);
    int prev_value = -1;
    int count = 0;

    while (it.has_next(&it))
    {
        void* current = it.get(&it);
        ASSERT_NOT_NULL(current);
        int curr_value = *(int*)current;

        // Verify sorted order
        ASSERT(curr_value > prev_value);
        prev_value = curr_value;
        count++;

        if (it.next(&it) != 0)
            break;
    }

    ASSERT_EQ(count, 11);
    it.destroy(&it);
    anv_bst_destroy(bst, true);
    return TEST_SUCCESS;
}

typedef struct
{
    int (*func)(void);
    const char* name;
} TestCase;

int main(void)
{
    TestCase tests[] = {
        {test_bst_iterator_inorder, "test_bst_iterator_inorder"},
        {test_bst_iterator_preorder, "test_bst_iterator_preorder"},
        {test_bst_iterator_postorder, "test_bst_iterator_postorder"},
        {test_bst_iterator_empty, "test_bst_iterator_empty"},
        {test_bst_iterator_single_node, "test_bst_iterator_single_node"},
        {test_bst_iterator_reset, "test_bst_iterator_reset"},
        {test_bst_iterator_backward, "test_bst_iterator_backward"},
        {test_bst_from_iterator, "test_bst_from_iterator"},
        {test_bst_iterator_null_params, "test_bst_iterator_null_params"},
        {test_bst_iterator_complex, "test_bst_iterator_complex"}
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int passed = 0;

    printf("Running BST iterator tests...\n");

    for (int i = 0; i < num_tests; i++)
    {
        printf("Running %s... ", tests[i].name);
        fflush(stdout);

        int result = tests[i].func();
        if (result == TEST_SUCCESS)
        {
            printf("PASSED\n");
            passed++;
        }
        else if (result == TEST_SKIPPED)
        {
            printf("SKIPPED\n");
        }
        else
        {
            printf("FAILED\n");
        }
    }

    printf("\nBST Iterator Tests: %d/%d passed\n", passed, num_tests);
    return (passed == num_tests) ? 0 : 1;
}
