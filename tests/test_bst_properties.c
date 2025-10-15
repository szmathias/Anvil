//
// Created by zack on 9/25/25.
//
// Binary Search Tree properties and edge cases test

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "containers/binarysearchtree.h"
#include "TestAssert.h"
#include "TestHelpers.h"

// Global variables for collecting traversal results
static int invariant_results[11];
static int invariant_index = 0;

static int desc_results[5];
static int desc_index = 0;

// Helper functions for collecting traversal results
void collect_for_invariant(void* elem) {
    invariant_results[invariant_index++] = *(int*)elem;
}

void collect_desc(void* elem) {
    desc_results[desc_index++] = *(int*)elem;
}

// Test BST property invariant - left < root < right
int test_bst_invariant_property(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    // Insert values in random order
    int values[] = {50, 25, 75, 10, 30, 60, 80, 5, 15, 27, 35};
    int* data[11];

    for (int i = 0; i < 11; i++)
    {
        data[i] = malloc(sizeof(int));
        *(data[i]) = values[i];
        ASSERT_EQ(anv_bst_insert(bst, data[i]), 0);
    }

    // Reset and collect traversal results
    invariant_index = 0;
    anv_bst_inorder(bst, collect_for_invariant);

    // Check that results are in ascending order
    for (int i = 1; i < 11; i++)
    {
        ASSERT(invariant_results[i-1] < invariant_results[i]);
    }

    anv_bst_destroy(bst, true);
    return TEST_SUCCESS;
}

// Test height calculation
int test_bst_height_calculation(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    // Empty tree height
    ASSERT_EQ(anv_bst_height(bst), 0);

    // Single node tree
    int* data1 = malloc(sizeof(int));
    *data1 = 50;
    anv_bst_insert(bst, data1);
    ASSERT_EQ(anv_bst_height(bst), 1);

    // Add left child - height should be 2
    int* data2 = malloc(sizeof(int));
    *data2 = 30;
    anv_bst_insert(bst, data2);
    ASSERT_EQ(anv_bst_height(bst), 2);

    // Add right child - height still 2
    int* data3 = malloc(sizeof(int));
    *data3 = 70;
    anv_bst_insert(bst, data3);
    ASSERT_EQ(anv_bst_height(bst), 2);

    // Add deeper left node - height becomes 3
    int* data4 = malloc(sizeof(int));
    *data4 = 20;
    anv_bst_insert(bst, data4);
    ASSERT_EQ(anv_bst_height(bst), 3);

    anv_bst_destroy(bst, true);
    return TEST_SUCCESS;
}

// Test degenerate tree (linear chain)
int test_bst_degenerate_tree(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    // Insert in ascending order to create right-skewed tree
    for (int i = 1; i <= 10; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i;
        ASSERT_EQ(anv_bst_insert(bst, data), 0);
    }

    // Height should be equal to number of nodes (worst case)
    ASSERT_EQ(anv_bst_height(bst), 10);
    ASSERT_EQ(anv_bst_size(bst), 10);

    // Min should be 1, max should be 10
    ASSERT_EQ(*(int*)anv_bst_min(bst), 1);
    ASSERT_EQ(*(int*)anv_bst_max(bst), 10);

    anv_bst_destroy(bst, true);
    return TEST_SUCCESS;
}

// Test perfect binary tree
int test_bst_perfect_tree(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    // Insert values to create perfect binary tree
    // Level 1: 50
    // Level 2: 25, 75
    // Level 3: 12, 37, 62, 87
    int values[] = {50, 25, 75, 12, 37, 62, 87};
    int* data[7];

    for (int i = 0; i < 7; i++)
    {
        data[i] = malloc(sizeof(int));
        *(data[i]) = values[i];
        ASSERT_EQ(anv_bst_insert(bst, data[i]), 0);
    }

    ASSERT_EQ(anv_bst_size(bst), 7);
    ASSERT_EQ(anv_bst_height(bst), 3); // Perfect tree with 7 nodes has height 3

    // Verify min and max
    ASSERT_EQ(*(int*)anv_bst_min(bst), 12);
    ASSERT_EQ(*(int*)anv_bst_max(bst), 87);

    anv_bst_destroy(bst, true);
    return TEST_SUCCESS;
}

// Test duplicate handling
int test_bst_duplicate_handling(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    int* data1 = malloc(sizeof(int));
    int* data2 = malloc(sizeof(int));
    int* data3 = malloc(sizeof(int));
    *data1 = 50;
    *data2 = 50; // Duplicate
    *data3 = 50; // Another duplicate

    // First insert should succeed
    ASSERT_EQ(anv_bst_insert(bst, data1), 0);
    ASSERT_EQ(anv_bst_size(bst), 1);

    // Duplicate inserts should return 1 and not change size
    ASSERT_EQ(anv_bst_insert(bst, data2), 1);
    ASSERT_EQ(anv_bst_size(bst), 1);

    ASSERT_EQ(anv_bst_insert(bst, data3), 1);
    ASSERT_EQ(anv_bst_size(bst), 1);

    // Clean up duplicate data that wasn't inserted
    free(data2);
    free(data3);

    anv_bst_destroy(bst, true);
    return TEST_SUCCESS;
}

// Test with negative numbers
int test_bst_negative_numbers(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    int values[] = {0, -10, 10, -5, 5, -15, 15};
    int* data[7];

    for (int i = 0; i < 7; i++)
    {
        data[i] = malloc(sizeof(int));
        *(data[i]) = values[i];
        ASSERT_EQ(anv_bst_insert(bst, data[i]), 0);
    }

    ASSERT_EQ(anv_bst_size(bst), 7);
    ASSERT_EQ(*(int*)anv_bst_min(bst), -15);
    ASSERT_EQ(*(int*)anv_bst_max(bst), 15);

    // Verify all values are present
    for (int i = 0; i < 7; i++)
    {
        ASSERT(anv_bst_contains(bst, &values[i]));
    }

    anv_bst_destroy(bst, true);
    return TEST_SUCCESS;
}

// Test removal of root with various configurations
int test_bst_root_removal_cases(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Case 1: Root with no children
    ANVBinarySearchTree* bst1 = anv_bst_create(&alloc, int_cmp);
    int* root1 = malloc(sizeof(int));
    *root1 = 50;
    anv_bst_insert(bst1, root1);

    int remove_val = 50;
    ASSERT_EQ(anv_bst_remove(bst1, &remove_val, true), 0);
    ASSERT_EQ(anv_bst_size(bst1), 0);
    ASSERT(anv_bst_is_empty(bst1));
    anv_bst_destroy(bst1, false);

    // Case 2: Root with only left child
    ANVBinarySearchTree* bst2 = anv_bst_create(&alloc, int_cmp);
    int* root2 = malloc(sizeof(int));
    int* left2 = malloc(sizeof(int));
    *root2 = 50;
    *left2 = 30;
    anv_bst_insert(bst2, root2);
    anv_bst_insert(bst2, left2);

    remove_val = 50;
    ASSERT_EQ(anv_bst_remove(bst2, &remove_val, true), 0);
    ASSERT_EQ(anv_bst_size(bst2), 1);
    ASSERT_EQ(*(int*)anv_bst_min(bst2), 30);
    anv_bst_destroy(bst2, true);

    // Case 3: Root with only right child
    ANVBinarySearchTree* bst3 = anv_bst_create(&alloc, int_cmp);
    int* root3 = malloc(sizeof(int));
    int* right3 = malloc(sizeof(int));
    *root3 = 50;
    *right3 = 70;
    anv_bst_insert(bst3, root3);
    anv_bst_insert(bst3, right3);

    remove_val = 50;
    ASSERT_EQ(anv_bst_remove(bst3, &remove_val, true), 0);
    ASSERT_EQ(anv_bst_size(bst3), 1);
    ASSERT_EQ(*(int*)anv_bst_min(bst3), 70);
    anv_bst_destroy(bst3, true);

    // Case 4: Root with both children
    ANVBinarySearchTree* bst4 = anv_bst_create(&alloc, int_cmp);
    int values[] = {50, 30, 70, 20, 40, 60, 80};
    int* data[7];
    for (int i = 0; i < 7; i++)
    {
        data[i] = malloc(sizeof(int));
        *(data[i]) = values[i];
        anv_bst_insert(bst4, data[i]);
    }

    remove_val = 50;
    ASSERT_EQ(anv_bst_remove(bst4, &remove_val, true), 0);
    ASSERT_EQ(anv_bst_size(bst4), 6);
    ASSERT(!anv_bst_contains(bst4, &remove_val));

    // Tree should still maintain BST property
    ASSERT_EQ(*(int*)anv_bst_min(bst4), 20);
    ASSERT_EQ(*(int*)anv_bst_max(bst4), 80);
    anv_bst_destroy(bst4, true);

    return TEST_SUCCESS;
}

// Test tree behavior after multiple operations
int test_bst_complex_operations(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    // Build initial tree
    int initial_values[] = {50, 30, 70, 20, 40, 60, 80, 10, 25, 35, 45};
    int* data[11];
    for (int i = 0; i < 11; i++)
    {
        data[i] = malloc(sizeof(int));
        *(data[i]) = initial_values[i];
        anv_bst_insert(bst, data[i]);
    }

    ASSERT_EQ(anv_bst_size(bst), 11);

    // Remove some leaf nodes
    int remove_values[] = {10, 45, 25};
    for (int i = 0; i < 3; i++)
    {
        ASSERT_EQ(anv_bst_remove(bst, &remove_values[i], true), 0);
    }

    ASSERT_EQ(anv_bst_size(bst), 8);

    // Add some new values
    int new_values[] = {15, 55, 75};
    int* new_data[3];
    for (int i = 0; i < 3; i++)
    {
        new_data[i] = malloc(sizeof(int));
        *(new_data[i]) = new_values[i];
        ASSERT_EQ(anv_bst_insert(bst, new_data[i]), 0);
    }

    ASSERT_EQ(anv_bst_size(bst), 11);

    // Verify BST property still holds
    ASSERT_EQ(*(int*)anv_bst_min(bst), 15);
    ASSERT_EQ(*(int*)anv_bst_max(bst), 80);

    anv_bst_destroy(bst, true);
    return TEST_SUCCESS;
}

// Test with custom comparison function (descending order)
int test_bst_custom_comparison(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp_desc);

    int values[] = {50, 30, 70, 20, 80};
    int* data[5];

    for (int i = 0; i < 5; i++)
    {
        data[i] = malloc(sizeof(int));
        *(data[i]) = values[i];
        ASSERT_EQ(anv_bst_insert(bst, data[i]), 0);
    }

    // With descending comparison, min and max are swapped
    ASSERT_EQ(*(int*)anv_bst_min(bst), 80); // "Minimum" in descending order
    ASSERT_EQ(*(int*)anv_bst_max(bst), 20); // "Maximum" in descending order

    // In-order traversal should be in descending order
    desc_index = 0;
    anv_bst_inorder(bst, collect_desc);

    // Should be in descending order: 80, 70, 50, 30, 20
    for (int i = 1; i < 5; i++)
    {
        ASSERT(desc_results[i-1] > desc_results[i]);
    }

    anv_bst_destroy(bst, true);
    return TEST_SUCCESS;
}

// Test boundary conditions
int test_bst_boundary_conditions(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    // Test with extreme values
    int* min_int = malloc(sizeof(int));
    int* max_int = malloc(sizeof(int));
    int* zero = malloc(sizeof(int));
    *min_int = INT_MIN;
    *max_int = INT_MAX;
    *zero = 0;

    ASSERT_EQ(anv_bst_insert(bst, zero), 0);
    ASSERT_EQ(anv_bst_insert(bst, min_int), 0);
    ASSERT_EQ(anv_bst_insert(bst, max_int), 0);

    ASSERT_EQ(anv_bst_size(bst), 3);
    ASSERT_EQ(*(int*)anv_bst_min(bst), INT_MIN);
    ASSERT_EQ(*(int*)anv_bst_max(bst), INT_MAX);

    // Test contains with extreme values
    ASSERT(anv_bst_contains(bst, min_int));
    ASSERT(anv_bst_contains(bst, max_int));
    ASSERT(anv_bst_contains(bst, zero));

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
        {test_bst_invariant_property, "test_bst_invariant_property"},
        {test_bst_height_calculation, "test_bst_height_calculation"},
        {test_bst_degenerate_tree, "test_bst_degenerate_tree"},
        {test_bst_perfect_tree, "test_bst_perfect_tree"},
        {test_bst_duplicate_handling, "test_bst_duplicate_handling"},
        {test_bst_negative_numbers, "test_bst_negative_numbers"},
        {test_bst_root_removal_cases, "test_bst_root_removal_cases"},
        {test_bst_complex_operations, "test_bst_complex_operations"},
        {test_bst_custom_comparison, "test_bst_custom_comparison"},
        {test_bst_boundary_conditions, "test_bst_boundary_conditions"}
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int passed = 0;

    printf("Running BST properties tests...\n");

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

    printf("\nBST Properties Tests: %d/%d passed\n", passed, num_tests);
    return (passed == num_tests) ? 0 : 1;
}
