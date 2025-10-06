//
// Created by zack on 9/25/25.
//
// Binary Search Tree memory management test

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "containers/binarysearchtree.h"
#include "TestAssert.h"
#include "TestHelpers.h"

// Test memory management with custom allocator
int test_bst_custom_allocator(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);
    ASSERT_NOT_NULL(bst);

    // Insert some data
    for (int i = 0; i < 10; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i * 10;
        ASSERT_EQ(anv_bst_insert(bst, data), 0);
    }

    ASSERT_EQ(anv_bst_size(bst), 10);

    // Clean up with data freeing
    anv_bst_destroy(bst, true);
    return TEST_SUCCESS;
}

// Test memory management without freeing data
int test_bst_no_free_data(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    int static_values[] = {50, 30, 70, 20, 40};

    // Insert static data (should not be freed)
    for (int i = 0; i < 5; i++)
    {
        ASSERT_EQ(anv_bst_insert(bst, &static_values[i]), 0);
    }

    ASSERT_EQ(anv_bst_size(bst), 5);

    // Clean up without freeing data
    anv_bst_destroy(bst, false);
    return TEST_SUCCESS;
}

// Test clear operation with and without freeing data
int test_bst_clear_memory(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    // Insert dynamically allocated data
    for (int i = 0; i < 5; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i;
        ASSERT_EQ(anv_bst_insert(bst, data), 0);
    }

    ASSERT_EQ(anv_bst_size(bst), 5);

    // Clear with freeing data
    anv_bst_clear(bst, true);
    ASSERT_EQ(anv_bst_size(bst), 0);
    ASSERT(anv_bst_is_empty(bst));

    // Insert more data
    for (int i = 0; i < 3; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i + 100;
        ASSERT_EQ(anv_bst_insert(bst, data), 0);
    }

    ASSERT_EQ(anv_bst_size(bst), 3);

    anv_bst_destroy(bst, true);
    return TEST_SUCCESS;
}

// Test removal with memory management
int test_bst_remove_memory(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    int* data[5];
    for (int i = 0; i < 5; i++)
    {
        data[i] = malloc(sizeof(int));
        *(data[i]) = (i + 1) * 10;
        ASSERT_EQ(anv_bst_insert(bst, data[i]), 0);
    }

    ASSERT_EQ(anv_bst_size(bst), 5);

    // Remove with freeing data
    int remove_val = 30;
    ASSERT_EQ(anv_bst_remove(bst, &remove_val, true), 0);
    ASSERT_EQ(anv_bst_size(bst), 4);
    ASSERT(!anv_bst_contains(bst, &remove_val));

    // Remove without freeing data (for static reference)
    int static_val = 20;
    ASSERT_EQ(anv_bst_remove(bst, &static_val, true), 0);
    ASSERT_EQ(anv_bst_size(bst), 3);

    anv_bst_destroy(bst, true); // Free remaining data
    return TEST_SUCCESS;
}

// Test large dataset for memory efficiency
int test_bst_large_dataset(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    const int NUM_ELEMENTS = 1000;

    // Insert large number of elements
    for (int i = 0; i < NUM_ELEMENTS; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i;
        ASSERT_EQ(anv_bst_insert(bst, data), 0);
    }

    ASSERT_EQ(anv_bst_size(bst), NUM_ELEMENTS);

    // Verify some elements exist
    for (int i = 0; i < NUM_ELEMENTS; i += 100)
    {
        ASSERT(anv_bst_contains(bst, &i));
    }

    // Remove half the elements
    for (int i = 0; i < NUM_ELEMENTS; i += 2)
    {
        ASSERT_EQ(anv_bst_remove(bst, &i, true), 0);
    }

    ASSERT_EQ(anv_bst_size(bst), NUM_ELEMENTS / 2);

    anv_bst_destroy(bst, true);
    return TEST_SUCCESS;
}

// Test with Person structures
int test_bst_person_memory(void)
{
    ANVAllocator alloc = create_person_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, person_cmp);

    Person* people[3];
    people[0] = create_person("Alice", 30);
    people[1] = create_person("Bob", 25);
    people[2] = create_person("Charlie", 35);

    for (int i = 0; i < 3; i++)
    {
        ASSERT_EQ(anv_bst_insert(bst, people[i]), 0);
    }

    ASSERT_EQ(anv_bst_size(bst), 3);

    // Verify people are in the tree
    ASSERT(anv_bst_contains(bst, people[0]));
    ASSERT(anv_bst_contains(bst, people[1]));
    ASSERT(anv_bst_contains(bst, people[2]));

    anv_bst_destroy(bst, true); // Free person data
    return TEST_SUCCESS;
}

// Test iterator memory management
int test_bst_iterator_memory(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    // Insert data
    for (int i = 0; i < 10; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i;
        ASSERT_EQ(anv_bst_insert(bst, data), 0);
    }

    // Create multiple iterators
    ANVIterator it1 = anv_bst_iterator(bst);
    ANVIterator it2 = anv_bst_iterator_preorder(bst);
    ANVIterator it3 = anv_bst_iterator_postorder(bst);

    ASSERT(it1.is_valid(&it1));
    ASSERT(it2.is_valid(&it2));
    ASSERT(it3.is_valid(&it3));

    // Use iterators briefly
    if (it1.has_next(&it1)) {
        it1.get(&it1);
        it1.next(&it1);
    }

    if (it2.has_next(&it2)) {
        it2.get(&it2);
        it2.next(&it2);
    }

    if (it3.has_next(&it3)) {
        it3.get(&it3);
        it3.next(&it3);
    }

    // Clean up iterators
    it1.destroy(&it1);
    it2.destroy(&it2);
    it3.destroy(&it3);

    anv_bst_destroy(bst, true);
    return TEST_SUCCESS;
}

void print_tree(void* data)
{
    if (data)
    {
        printf("%d ", *(int*)data);
    }
}

// Test from_iterator memory management
int test_bst_from_iterator_memory(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* source_bst = anv_bst_create(&alloc, int_cmp);

    // Create source data
    for (int i = 0; i < 5; i++)
    {
        int* data = malloc(sizeof(int));
        *data = i * 10;
        ASSERT_EQ(anv_bst_insert(source_bst, data), 0);
    }

    // Test with copying data
    ANVIterator it1 = anv_bst_iterator(source_bst);
    ANVBinarySearchTree* copy_bst = anv_bst_from_iterator(&it1, &alloc, int_cmp, true);
    ASSERT_NOT_NULL(copy_bst);
    ASSERT_EQ(anv_bst_size(copy_bst), 5);

    // Test without copying data
    it1.reset(&it1);
    ANVBinarySearchTree* ref_bst = anv_bst_from_iterator(&it1, &alloc, int_cmp, false);
    ASSERT_NOT_NULL(ref_bst);
    ASSERT_EQ(anv_bst_size(ref_bst), 5);

    it1.destroy(&it1);

    // Clean up - copy_bst should free its copied data, ref_bst should not
    anv_bst_destroy(ref_bst, false);
    anv_bst_destroy(copy_bst, true);
    anv_bst_destroy(source_bst, true);
    return TEST_SUCCESS;
}

// Test edge cases with memory
int test_bst_memory_edge_cases(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    // Test with single element
    int* single_data = malloc(sizeof(int));
    *single_data = 42;
    ASSERT_EQ(anv_bst_insert(bst, single_data), 0);

    // Clear and verify empty
    anv_bst_clear(bst, true);
    ASSERT(anv_bst_is_empty(bst));

    // Insert again after clear
    int* new_data = malloc(sizeof(int));
    *new_data = 100;
    ASSERT_EQ(anv_bst_insert(bst, new_data), 0);
    ASSERT_EQ(anv_bst_size(bst), 1);

    anv_bst_destroy(bst, true);
    return TEST_SUCCESS;
}

// Test tree destruction in various states
int test_bst_destruction_states(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Test destroying empty tree
    ANVBinarySearchTree* empty_bst = anv_bst_create(&alloc, int_cmp);
    anv_bst_destroy(empty_bst, true);

    // Test destroying single node tree
    ANVBinarySearchTree* single_bst = anv_bst_create(&alloc, int_cmp);
    int* data = malloc(sizeof(int));
    *data = 42;
    anv_bst_insert(single_bst, data);
    anv_bst_destroy(single_bst, true);

    // Test destroying after clear
    ANVBinarySearchTree* cleared_bst = anv_bst_create(&alloc, int_cmp);
    for (int i = 0; i < 5; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_bst_insert(cleared_bst, val);
    }
    anv_bst_clear(cleared_bst, true);
    anv_bst_destroy(cleared_bst, false); // Already cleared

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
        {test_bst_custom_allocator, "test_bst_custom_allocator"},
        {test_bst_no_free_data, "test_bst_no_free_data"},
        {test_bst_clear_memory, "test_bst_clear_memory"},
        {test_bst_remove_memory, "test_bst_remove_memory"},
        {test_bst_large_dataset, "test_bst_large_dataset"},
        {test_bst_person_memory, "test_bst_person_memory"},
        {test_bst_iterator_memory, "test_bst_iterator_memory"},
        {test_bst_from_iterator_memory, "test_bst_from_iterator_memory"},
        {test_bst_memory_edge_cases, "test_bst_memory_edge_cases"},
        {test_bst_destruction_states, "test_bst_destruction_states"}
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int passed = 0;

    printf("Running BST memory tests...\n");

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

    printf("\nBST Memory Tests: %d/%d passed\n", passed, num_tests);
    return (passed == num_tests) ? 0 : 1;
}
