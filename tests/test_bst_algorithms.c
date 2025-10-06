//
// Created by zack on 9/25/25.
//
// Binary Search Tree traversal operations test

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "containers/binarysearchtree.h"
#include "TestAssert.h"
#include "TestHelpers.h"

// Global arrays to collect traversal results
static int traversal_results[20];
static int traversal_index;

// Helper function to collect traversal results
void collect_int(void* data)
{
    if (data && traversal_index < 20)
    {
        traversal_results[traversal_index++] = *(int*)data;
    }
}

// Reset traversal collection
void reset_traversal(void)
{
    traversal_index = 0;
    memset(traversal_results, 0, sizeof(traversal_results));
}

// Test in-order traversal
int test_bst_inorder_traversal(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    // Create a balanced tree:
    //      50
    //    /    \
    //   30     70
    //  /  \   /  \
    // 20  40 60  80
    int values[] = {50, 30, 70, 20, 40, 60, 80};
    int* data[7];

    for (int i = 0; i < 7; i++)
    {
        data[i] = malloc(sizeof(int));
        *(data[i]) = values[i];
        ASSERT_EQ(anv_bst_insert(bst, data[i]), 0);
    }

    // Test in-order traversal (should be sorted)
    reset_traversal();
    anv_bst_inorder(bst, collect_int);

    int expected[] = {20, 30, 40, 50, 60, 70, 80};
    ASSERT_EQ(traversal_index, 7);
    for (int i = 0; i < 7; i++)
    {
        ASSERT_EQ(traversal_results[i], expected[i]);
    }

    anv_bst_destroy(bst, true);
    return TEST_SUCCESS;
}

// Test pre-order traversal
int test_bst_preorder_traversal(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    // Same tree structure as above
    int values[] = {50, 30, 70, 20, 40, 60, 80};
    int* data[7];

    for (int i = 0; i < 7; i++)
    {
        data[i] = malloc(sizeof(int));
        *(data[i]) = values[i];
        ASSERT_EQ(anv_bst_insert(bst, data[i]), 0);
    }

    // Test pre-order traversal (root, left, right)
    reset_traversal();
    anv_bst_preorder(bst, collect_int);

    int expected[] = {50, 30, 20, 40, 70, 60, 80};
    ASSERT_EQ(traversal_index, 7);
    for (int i = 0; i < 7; i++)
    {
        ASSERT_EQ(traversal_results[i], expected[i]);
    }

    anv_bst_destroy(bst, true);
    return TEST_SUCCESS;
}

// Test post-order traversal
int test_bst_postorder_traversal(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    // Same tree structure as above
    int values[] = {50, 30, 70, 20, 40, 60, 80};
    int* data[7];

    for (int i = 0; i < 7; i++)
    {
        data[i] = malloc(sizeof(int));
        *(data[i]) = values[i];
        ASSERT_EQ(anv_bst_insert(bst, data[i]), 0);
    }

    // Test post-order traversal (left, right, root)
    reset_traversal();
    anv_bst_postorder(bst, collect_int);

    int expected[] = {20, 40, 30, 60, 80, 70, 50};
    ASSERT_EQ(traversal_index, 7);
    for (int i = 0; i < 7; i++)
    {
        ASSERT_EQ(traversal_results[i], expected[i]);
    }

    anv_bst_destroy(bst, true);
    return TEST_SUCCESS;
}

// Test traversal on empty tree
int test_bst_traversal_empty(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    // Test all traversals on empty tree
    reset_traversal();
    anv_bst_inorder(bst, collect_int);
    ASSERT_EQ(traversal_index, 0);

    reset_traversal();
    anv_bst_preorder(bst, collect_int);
    ASSERT_EQ(traversal_index, 0);

    reset_traversal();
    anv_bst_postorder(bst, collect_int);
    ASSERT_EQ(traversal_index, 0);

    anv_bst_destroy(bst, false);
    return TEST_SUCCESS;
}

// Test traversal on single node tree
int test_bst_traversal_single_node(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    int* data = malloc(sizeof(int));
    *data = 42;
    ASSERT_EQ(anv_bst_insert(bst, data), 0);

    // All traversals should visit the single node
    reset_traversal();
    anv_bst_inorder(bst, collect_int);
    ASSERT_EQ(traversal_index, 1);
    ASSERT_EQ(traversal_results[0], 42);

    reset_traversal();
    anv_bst_preorder(bst, collect_int);
    ASSERT_EQ(traversal_index, 1);
    ASSERT_EQ(traversal_results[0], 42);

    reset_traversal();
    anv_bst_postorder(bst, collect_int);
    ASSERT_EQ(traversal_index, 1);
    ASSERT_EQ(traversal_results[0], 42);

    anv_bst_destroy(bst, true);
    return TEST_SUCCESS;
}

// Test traversal with NULL parameters
int test_bst_traversal_null_params(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    int* data = malloc(sizeof(int));
    *data = 42;
    ASSERT_EQ(anv_bst_insert(bst, data), 0);

    // Test with NULL tree
    reset_traversal();
    anv_bst_inorder(NULL, collect_int);
    ASSERT_EQ(traversal_index, 0);

    anv_bst_preorder(NULL, collect_int);
    ASSERT_EQ(traversal_index, 0);

    anv_bst_postorder(NULL, collect_int);
    ASSERT_EQ(traversal_index, 0);

    // Test with NULL action function
    anv_bst_inorder(bst, NULL);
    anv_bst_preorder(bst, NULL);
    anv_bst_postorder(bst, NULL);

    anv_bst_destroy(bst, true);
    return TEST_SUCCESS;
}

// Test traversal on linear tree (worst case)
int test_bst_traversal_linear(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    // Create a linear tree (all right children): 1->2->3->4->5
    int values[] = {1, 2, 3, 4, 5};
    int* data[5];

    for (int i = 0; i < 5; i++)
    {
        data[i] = malloc(sizeof(int));
        *(data[i]) = values[i];
        ASSERT_EQ(anv_bst_insert(bst, data[i]), 0);
    }

    // In-order should still be sorted
    reset_traversal();
    anv_bst_inorder(bst, collect_int);
    ASSERT_EQ(traversal_index, 5);
    for (int i = 0; i < 5; i++)
    {
        ASSERT_EQ(traversal_results[i], i + 1);
    }

    // Pre-order should be same as insertion order for this case
    reset_traversal();
    anv_bst_preorder(bst, collect_int);
    ASSERT_EQ(traversal_index, 5);
    for (int i = 0; i < 5; i++)
    {
        ASSERT_EQ(traversal_results[i], i + 1);
    }

    // Post-order should be reverse
    reset_traversal();
    anv_bst_postorder(bst, collect_int);
    ASSERT_EQ(traversal_index, 5);
    for (int i = 0; i < 5; i++)
    {
        ASSERT_EQ(traversal_results[i], 5 - i);
    }

    anv_bst_destroy(bst, true);
    return TEST_SUCCESS;
}

// Test traversal consistency after removals
int test_bst_traversal_after_removal(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    // Insert values
    int values[] = {50, 30, 70, 20, 40, 60, 80, 10, 90};
    int* data[9];

    for (int i = 0; i < 9; i++)
    {
        data[i] = malloc(sizeof(int));
        *(data[i]) = values[i];
        ASSERT_EQ(anv_bst_insert(bst, data[i]), 0);
    }

    // Remove some values
    int remove_val = 30;
    ASSERT_EQ(anv_bst_remove(bst, &remove_val, true), 0);

    remove_val = 80;
    ASSERT_EQ(anv_bst_remove(bst, &remove_val, true), 0);

    // In-order traversal should still be sorted
    reset_traversal();
    anv_bst_inorder(bst, collect_int);
    ASSERT_EQ(traversal_index, 7);

    // Verify sorted order
    for (int i = 0; i < traversal_index - 1; i++)
    {
        ASSERT(traversal_results[i] < traversal_results[i + 1]);
    }

    // Verify removed elements are not present
    for (int i = 0; i < traversal_index; i++)
    {
        ASSERT(traversal_results[i] != 30);
        ASSERT(traversal_results[i] != 80);
    }

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
        {test_bst_inorder_traversal, "test_bst_inorder_traversal"},
        {test_bst_preorder_traversal, "test_bst_preorder_traversal"},
        {test_bst_postorder_traversal, "test_bst_postorder_traversal"},
        {test_bst_traversal_empty, "test_bst_traversal_empty"},
        {test_bst_traversal_single_node, "test_bst_traversal_single_node"},
        {test_bst_traversal_null_params, "test_bst_traversal_null_params"},
        {test_bst_traversal_linear, "test_bst_traversal_linear"},
        {test_bst_traversal_after_removal, "test_bst_traversal_after_removal"}
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int passed = 0;

    printf("Running BST traversal tests...\n");

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

    printf("\nBST Traversal Tests: %d/%d passed\n", passed, num_tests);
    return (passed == num_tests) ? 0 : 1;
}
