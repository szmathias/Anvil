#include <stdlib.h>

#include <anvil/testing.h>
#include "TestHelpers.h"
#include "containers/binarysearchtree.h"

//==============================================================================
// Static Helpers
//==============================================================================

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

void reset_traversal(void)
{
    traversal_index = 0;
    memset(traversal_results, 0, sizeof(traversal_results));
}

static int invariant_results[11];
static int invariant_index = 0;

static int desc_results[5];
static int desc_index = 0;

void collect_for_invariant(void* elem)
{
    invariant_results[invariant_index++] = *(int*)elem;
}

void collect_desc(void* elem)
{
    desc_results[desc_index++] = *(int*)elem;
}


//==============================================================================
// CRUD Tests
//==============================================================================

// Test basic BST creation and destruction
int test_bst_create_destroy(void)
{
    ANVAllocator alloc = create_int_allocator();

    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);
    ASSERT_NOT_NULL(bst);
    ASSERT_EQ(anv_bst_size(bst), 0);
    ASSERT(anv_bst_is_empty(bst));
    ASSERT_EQ(anv_bst_height(bst), 0);

    anv_bst_destroy(bst, false);
    return TEST_SUCCESS;
}

// Test NULL parameter handling
int test_bst_null_parameters(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Creating with NULL compare function should fail
    ASSERT_NULL(anv_bst_create(&alloc, NULL));

    // Creating with NULL allocator should fail
    ASSERT_NULL(anv_bst_create(NULL, int_cmp));

    // Operations on NULL tree should be safe
    ASSERT_EQ(anv_bst_size(NULL), 0);
    ASSERT(anv_bst_is_empty(NULL));
    ASSERT_EQ(anv_bst_height(NULL), 0);
    ASSERT(!anv_bst_contains(NULL, NULL));
    ASSERT_NULL(anv_bst_min(NULL));
    ASSERT_NULL(anv_bst_max(NULL));
    ASSERT_EQ(anv_bst_insert(NULL, NULL), -1);
    ASSERT_EQ(anv_bst_remove(NULL, NULL, false), -1);

    // Destruction should be safe
    anv_bst_destroy(NULL, false);
    anv_bst_clear(NULL, false);

    return TEST_SUCCESS;
}

// Test basic insertion operations
int test_bst_insert(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    MAKE_INT(data1, 50);
    MAKE_INT(data2, 30);
    MAKE_INT(data3, 70);

    // Test inserting root
    ASSERT_EQ(anv_bst_insert(bst, data1), 0);
    ASSERT_EQ(anv_bst_size(bst), 1);
    ASSERT(!anv_bst_is_empty(bst));
    ASSERT_EQ(anv_bst_height(bst), 1);

    // Test inserting left child
    ASSERT_EQ(anv_bst_insert(bst, data2), 0);
    ASSERT_EQ(anv_bst_size(bst), 2);
    ASSERT_EQ(anv_bst_height(bst), 2);

    // Test inserting right child
    ASSERT_EQ(anv_bst_insert(bst, data3), 0);
    ASSERT_EQ(anv_bst_size(bst), 3);
    ASSERT_EQ(anv_bst_height(bst), 2);

    // Test duplicate insertion
    MAKE_INT(duplicate, 50);
    ASSERT_EQ(anv_bst_insert(bst, duplicate), 1); // Should return 1 for duplicate
    ASSERT_EQ(anv_bst_size(bst), 3);              // Size should not change
    free(duplicate);

    // Test NULL data insertion
    ASSERT_EQ(anv_bst_insert(bst, NULL), -1);

    anv_bst_destroy(bst, true);
    return TEST_SUCCESS;
}

// Test contains functionality
int test_bst_contains(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    const int values[] = {50, 30, 70, 20, 40, 60, 80};
    int* data[7];

    // Insert test data
    for (int i = 0; i < 7; i++)
    {
        data[i] = malloc(sizeof(int));
        *data[i] = values[i];
        ASSERT_EQ(anv_bst_insert(bst, data[i]), 0);
    }

    // Test existing values
    for (int i = 0; i < 7; i++)
    {
        ASSERT(anv_bst_contains(bst, &values[i]));
    }

    // Test non-existing values
    const int non_existing[] = {10, 25, 35, 55, 75, 90};
    for (int i = 0; i < 6; i++)
    {
        ASSERT(!anv_bst_contains(bst, &non_existing[i]));
    }

    // Test with NULL
    ASSERT(!anv_bst_contains(bst, NULL));

    anv_bst_destroy(bst, true);
    return TEST_SUCCESS;
}

// Test min and max functionality
int test_bst_min_max(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    // Empty tree
    ASSERT_NULL(anv_bst_min(bst));
    ASSERT_NULL(anv_bst_max(bst));

    int values[] = {50, 30, 70, 20, 40, 60, 80, 10, 90};
    int* data[9];

    // Insert test data
    for (int i = 0; i < 9; i++)
    {
        data[i] = malloc(sizeof(int));
        *data[i] = values[i];
        ASSERT_EQ(anv_bst_insert(bst, data[i]), 0);
    }

    // Test min and max
    const int* min_val = anv_bst_min(bst);
    const int* max_val = anv_bst_max(bst);

    ASSERT_NOT_NULL(min_val);
    ASSERT_NOT_NULL(max_val);
    ASSERT_EQ(*min_val, 10);
    ASSERT_EQ(*max_val, 90);

    anv_bst_destroy(bst, true);
    return TEST_SUCCESS;
}

// Test removal operations
int test_bst_remove(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    int values[] = {50, 30, 70, 20, 40, 60, 80};
    int* data[7];

    // Insert test data
    for (int i = 0; i < 7; i++)
    {
        data[i] = malloc(sizeof(int));
        *data[i] = values[i];
        ASSERT_EQ(anv_bst_insert(bst, data[i]), 0);
    }

    ASSERT_EQ(anv_bst_size(bst), 7);

    // Test removing leaf node
    const int leaf_val = 20;
    ASSERT_EQ(anv_bst_remove(bst, &leaf_val, true), 0);
    ASSERT_EQ(anv_bst_size(bst), 6);
    ASSERT(!anv_bst_contains(bst, &leaf_val));

    // Test removing node with one child
    const int one_child_val = 30;
    ASSERT_EQ(anv_bst_remove(bst, &one_child_val, true), 0);
    ASSERT_EQ(anv_bst_size(bst), 5);
    ASSERT(!anv_bst_contains(bst, &one_child_val));

    // Test removing node with two children (root)
    const int root_val = 50;
    ASSERT_EQ(anv_bst_remove(bst, &root_val, true), 0);
    ASSERT_EQ(anv_bst_size(bst), 4);
    ASSERT(!anv_bst_contains(bst, &root_val));

    // Test removing non-existent value
    const int non_existent = 99;
    ASSERT_EQ(anv_bst_remove(bst, &non_existent, false), -1);
    ASSERT_EQ(anv_bst_size(bst), 4);

    // Test with NULL
    ASSERT_EQ(anv_bst_remove(bst, NULL, false), -1);

    anv_bst_destroy(bst, true); // Remaining nodes were not freed individually
    return TEST_SUCCESS;
}

// Test clear operation
int test_bst_clear(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    // Add some elements
    for (int i = 0; i < 10; i++)
    {
        MAKE_INT(data, i * 10);
        ASSERT_EQ(anv_bst_insert(bst, data), 0);
    }

    ASSERT_EQ(anv_bst_size(bst), 10);
    ASSERT(!anv_bst_is_empty(bst));

    // Clear with freeing data
    anv_bst_clear(bst, true);
    ASSERT_EQ(anv_bst_size(bst), 0);
    ASSERT(anv_bst_is_empty(bst));
    ASSERT_EQ(anv_bst_height(bst), 0);
    ASSERT_NULL(anv_bst_min(bst));
    ASSERT_NULL(anv_bst_max(bst));

    // Tree should still be usable after clear
    MAKE_INT(new_data, 999);
    ASSERT_EQ(anv_bst_insert(bst, new_data), 0);
    ASSERT_EQ(anv_bst_size(bst), 1);
    ASSERT_EQ(*(int*)anv_bst_min(bst), 999);

    anv_bst_destroy(bst, true);
    return TEST_SUCCESS;
}

// Test BST property maintenance
int test_bst_property(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);

    // Insert values in various orders to test BST property
    const int values[] = {50, 30, 70, 20, 40, 60, 80, 10, 25, 35, 45};
    int* data[11];

    for (int i = 0; i < 11; i++)
    {
        data[i] = malloc(sizeof(int));
        *data[i] = values[i];
        ASSERT_EQ(anv_bst_insert(bst, data[i]), 0);
    }

    // Verify all values are present
    for (int i = 0; i < 11; i++)
    {
        ASSERT(anv_bst_contains(bst, &values[i]));
    }

    // Verify min and max are correct
    ASSERT_EQ(*(int*)anv_bst_min(bst), 10);
    ASSERT_EQ(*(int*)anv_bst_max(bst), 80);

    anv_bst_destroy(bst, true);
    return TEST_SUCCESS;
}

// Test with string data
int test_bst_string_data(void)
{
    ANVAllocator alloc = create_string_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, (anv_compare_func)strcmp);

    char* strings[] = {"apple", "banana", "cherry", "date", "elderberry"};
    const int num_strings = 5;

    // Insert strings
    for (int i = 0; i < num_strings; i++)
    {
        char* str_copy = malloc(strlen(strings[i]) + 1);
        strcpy(str_copy, strings[i]);
        ASSERT_EQ(anv_bst_insert(bst, str_copy), 0);
    }

    ASSERT_EQ(anv_bst_size(bst), num_strings);

    // Test contains
    for (int i = 0; i < num_strings; i++)
    {
        ASSERT(anv_bst_contains(bst, strings[i]));
    }

    // Test min and max (lexicographic order)
    ASSERT_EQ_STR((char*)anv_bst_min(bst), "apple");
    ASSERT_EQ_STR((char*)anv_bst_max(bst), "elderberry");

    anv_bst_destroy(bst, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Algorithms Tests
//==============================================================================

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
        *data[i] = values[i];
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
        *data[i] = values[i];
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
        *data[i] = values[i];
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

    MAKE_INT(data, 42);
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

    MAKE_INT(data, 42);
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
        *data[i] = values[i];
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
        *data[i] = values[i];
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

//==============================================================================
// Iterator Tests
//==============================================================================

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
        *data[i] = values[i];
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
        *data[i] = values[i];
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
        *data[i] = values[i];
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

    MAKE_INT(data, 42);
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
        *data[i] = values[i];
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

    MAKE_INT(data, 42);
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
    const int values[] = {50, 30, 70, 20, 40};
    int* data[5];

    for (int i = 0; i < 5; i++)
    {
        data[i] = malloc(sizeof(int));
        *data[i] = values[i];
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
    const ANVIterator it = anv_bst_iterator(NULL);
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
        *data[i] = values[i];
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
        const int curr_value = *(int*)current;

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

//==============================================================================
// Memory Tests
//==============================================================================

// Test memory management with custom allocator
int test_bst_custom_allocator(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);
    ASSERT_NOT_NULL(bst);

    // Insert some data
    for (int i = 0; i < 10; i++)
    {
        MAKE_INT(data, i * 10);
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
        MAKE_INT(data, i);
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
        MAKE_INT(data, i + 100);
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
        *data[i] = (i + 1) * 10;
        ASSERT_EQ(anv_bst_insert(bst, data[i]), 0);
    }

    ASSERT_EQ(anv_bst_size(bst), 5);

    // Remove with freeing data
    const int remove_val = 30;
    ASSERT_EQ(anv_bst_remove(bst, &remove_val, true), 0);
    ASSERT_EQ(anv_bst_size(bst), 4);
    ASSERT(!anv_bst_contains(bst, &remove_val));

    // Remove without freeing data (for static reference)
    const int static_val = 20;
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
        MAKE_INT(data, i);
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
        MAKE_INT(data, i);
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
    if (it1.has_next(&it1))
    {
        it1.get(&it1);
        it1.next(&it1);
    }

    if (it2.has_next(&it2))
    {
        it2.get(&it2);
        it2.next(&it2);
    }

    if (it3.has_next(&it3))
    {
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

// Test from_iterator memory management
int test_bst_from_iterator_memory(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* source_bst = anv_bst_create(&alloc, int_cmp);

    // Create source data
    for (int i = 0; i < 5; i++)
    {
        MAKE_INT(data, i * 10);
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
    MAKE_INT(single_data, 42);
    ASSERT_EQ(anv_bst_insert(bst, single_data), 0);

    // Clear and verify empty
    anv_bst_clear(bst, true);
    ASSERT(anv_bst_is_empty(bst));

    // Insert again after clear
    MAKE_INT(new_data, 100);
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
    MAKE_INT(data, 42);
    anv_bst_insert(single_bst, data);
    anv_bst_destroy(single_bst, true);

    // Test destroying after clear
    ANVBinarySearchTree* cleared_bst = anv_bst_create(&alloc, int_cmp);
    for (int i = 0; i < 5; i++)
    {
        MAKE_INT(val, i);
        anv_bst_insert(cleared_bst, val);
    }
    anv_bst_clear(cleared_bst, true);
    anv_bst_destroy(cleared_bst, false); // Already cleared

    return TEST_SUCCESS;
}

//==============================================================================
// Properties Tests
//==============================================================================

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
        *data[i] = values[i];
        ASSERT_EQ(anv_bst_insert(bst, data[i]), 0);
    }

    // Reset and collect traversal results
    invariant_index = 0;
    anv_bst_inorder(bst, collect_for_invariant);

    // Check that results are in ascending order
    for (int i = 1; i < 11; i++)
    {
        ASSERT(invariant_results[i - 1] < invariant_results[i]);
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
    MAKE_INT(data1, 50);
    anv_bst_insert(bst, data1);
    ASSERT_EQ(anv_bst_height(bst), 1);

    // Add left child - height should be 2
    MAKE_INT(data2, 30);
    anv_bst_insert(bst, data2);
    ASSERT_EQ(anv_bst_height(bst), 2);

    // Add right child - height still 2
    MAKE_INT(data3, 70);
    anv_bst_insert(bst, data3);
    ASSERT_EQ(anv_bst_height(bst), 2);

    // Add deeper left node - height becomes 3
    MAKE_INT(data4, 20);
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
        MAKE_INT(data, i);
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
        *data[i] = values[i];
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

    MAKE_INT(data1, 50);
    MAKE_INT(data2, 50); // Duplicate
    MAKE_INT(data3, 50); // Another duplicate

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

    const int values[] = {0, -10, 10, -5, 5, -15, 15};
    int* data[7];

    for (int i = 0; i < 7; i++)
    {
        data[i] = malloc(sizeof(int));
        *data[i] = values[i];
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
    MAKE_INT(root1, 50);
    anv_bst_insert(bst1, root1);

    int remove_val = 50;
    ASSERT_EQ(anv_bst_remove(bst1, &remove_val, true), 0);
    ASSERT_EQ(anv_bst_size(bst1), 0);
    ASSERT(anv_bst_is_empty(bst1));
    anv_bst_destroy(bst1, false);

    // Case 2: Root with only left child
    ANVBinarySearchTree* bst2 = anv_bst_create(&alloc, int_cmp);
    MAKE_INT(root2, 50);
    MAKE_INT(left2, 30);
    anv_bst_insert(bst2, root2);
    anv_bst_insert(bst2, left2);

    remove_val = 50;
    ASSERT_EQ(anv_bst_remove(bst2, &remove_val, true), 0);
    ASSERT_EQ(anv_bst_size(bst2), 1);
    ASSERT_EQ(*(int*)anv_bst_min(bst2), 30);
    anv_bst_destroy(bst2, true);

    // Case 3: Root with only right child
    ANVBinarySearchTree* bst3 = anv_bst_create(&alloc, int_cmp);
    MAKE_INT(root3, 50);
    MAKE_INT(right3, 70);
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
        *data[i] = initial_values[i];
        anv_bst_insert(bst, data[i]);
    }

    ASSERT_EQ(anv_bst_size(bst), 11);

    // Remove some leaf nodes
    const int remove_values[] = {10, 45, 25};
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
        *new_data[i] = new_values[i];
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
        *data[i] = values[i];
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
        ASSERT(desc_results[i - 1] > desc_results[i]);
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
    MAKE_INT(min_int, INT_MIN);
    MAKE_INT(max_int, INT_MAX);
    MAKE_INT(zero, 0);

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

//==============================================================================
// Fuzz Tests
//==============================================================================

// Helper: verify BST ordering invariant via in-order traversal
static int fuzz_inorder_results[2000];
static int fuzz_inorder_index;

static void fuzz_collect_inorder(void* data)
{
    if (data && fuzz_inorder_index < 2000)
    {
        fuzz_inorder_results[fuzz_inorder_index++] = *(int*)data;
    }
}

static int verify_bst_sorted(const ANVBinarySearchTree* bst)
{
    fuzz_inorder_index = 0;
    memset(fuzz_inorder_results, 0, sizeof(fuzz_inorder_results));
    anv_bst_inorder(bst, fuzz_collect_inorder);
    for (int i = 1; i < fuzz_inorder_index; i++)
    {
        if (fuzz_inorder_results[i] < fuzz_inorder_results[i - 1])
            return 0; // Not sorted
    }
    return 1;
}

int test_bst_fuzz(void)
{
    srand((unsigned int)42);
    ANVAllocator alloc = create_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&alloc, int_cmp);
    ASSERT_NOT_NULL(bst);

    size_t expected_size = 0;

    for (int i = 0; i < 50000; i++)
    {
        const unsigned op = rand() % 3;
        const int val = rand() % 1000;

        switch (op)
        {
            case 0: // insert
            {
                MAKE_INT(data, val);
                const int rc = anv_bst_insert(bst, data);
                if (rc == 0)
                    expected_size++;
                else
                    free(data); // duplicate or error
                break;
            }
            case 1: // remove
            {
                if (anv_bst_contains(bst, &val))
                {
                    ASSERT_EQ(anv_bst_remove(bst, &val, true), 0);
                    expected_size--;
                }
                break;
            }
            case 2: // contains
            {
                anv_bst_contains(bst, &val);
                break;
            }
            default:
                break;
        }

        ASSERT_EQ(anv_bst_size(bst), expected_size);

        // Verify BST ordering every 5000 operations
        if (i % 5000 == 0)
        {
            ASSERT_TRUE(verify_bst_sorted(bst));
            // min <= max when non-empty
            if (expected_size > 0)
            {
                const int* min_v = anv_bst_min(bst);
                const int* max_v = anv_bst_max(bst);
                ASSERT_NOT_NULL(min_v);
                ASSERT_NOT_NULL(max_v);
                ASSERT_LTE(*min_v, *max_v);
            }
        }
    }

    // Final ordering check
    ASSERT_TRUE(verify_bst_sorted(bst));

    anv_bst_destroy(bst, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Memory Failure Tests (Step 4 - BST)
//==============================================================================

int test_bst_failing_allocator_create(void)
{
    // Allocation fails immediately - create should fail
    set_alloc_fail_countdown(0);
    ANVAllocator failing_alloc = create_failing_int_allocator();
    ANVBinarySearchTree* bst = anv_bst_create(&failing_alloc, int_cmp);
    ASSERT_NULL(bst);
    return TEST_SUCCESS;
}

int test_bst_failing_allocator_insert(void)
{
    ANVAllocator failing_alloc = create_failing_int_allocator();

    // Allow create to succeed, then fail on insert
    set_alloc_fail_countdown(1);
    ANVBinarySearchTree* bst = anv_bst_create(&failing_alloc, int_cmp);
    if (!bst)
        return TEST_SKIPPED; // Create itself may need >1 alloc

    MAKE_INT(data, 42);
    const int rc = anv_bst_insert(bst, data);
    if (rc != 0)
    {
        free(data);
    }
    // Should not crash regardless of outcome

    anv_bst_destroy(bst, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Main
//==============================================================================

int main(void)
{
    const ANVTestCase tests[] = {
        // CRUD Tests
        TEST_REGISTER(test_bst_create_destroy),
        TEST_REGISTER(test_bst_null_parameters),
        TEST_REGISTER(test_bst_insert),
        TEST_REGISTER(test_bst_contains),
        TEST_REGISTER(test_bst_min_max),
        TEST_REGISTER(test_bst_remove),
        TEST_REGISTER(test_bst_clear),
        TEST_REGISTER(test_bst_property),
        TEST_REGISTER(test_bst_string_data),

        // Algorithms Tests
        TEST_REGISTER(test_bst_inorder_traversal),
        TEST_REGISTER(test_bst_preorder_traversal),
        TEST_REGISTER(test_bst_postorder_traversal),
        TEST_REGISTER(test_bst_traversal_empty),
        TEST_REGISTER(test_bst_traversal_single_node),
        TEST_REGISTER(test_bst_traversal_null_params),
        TEST_REGISTER(test_bst_traversal_linear),
        TEST_REGISTER(test_bst_traversal_after_removal),

        // Iterator Tests
        TEST_REGISTER(test_bst_iterator_inorder),
        TEST_REGISTER(test_bst_iterator_preorder),
        TEST_REGISTER(test_bst_iterator_postorder),
        TEST_REGISTER(test_bst_iterator_empty),
        TEST_REGISTER(test_bst_iterator_single_node),
        TEST_REGISTER(test_bst_iterator_reset),
        TEST_REGISTER(test_bst_iterator_backward),
        TEST_REGISTER(test_bst_from_iterator),
        TEST_REGISTER(test_bst_iterator_null_params),
        TEST_REGISTER(test_bst_iterator_complex),

        // Memory Tests
        TEST_REGISTER(test_bst_custom_allocator),
        TEST_REGISTER(test_bst_no_free_data),
        TEST_REGISTER(test_bst_clear_memory),
        TEST_REGISTER(test_bst_remove_memory),
        TEST_REGISTER(test_bst_large_dataset),
        TEST_REGISTER(test_bst_person_memory),
        TEST_REGISTER(test_bst_iterator_memory),
        TEST_REGISTER(test_bst_from_iterator_memory),
        TEST_REGISTER(test_bst_memory_edge_cases),
        TEST_REGISTER(test_bst_destruction_states),

        // Properties Tests
        TEST_REGISTER(test_bst_invariant_property),
        TEST_REGISTER(test_bst_height_calculation),
        TEST_REGISTER(test_bst_degenerate_tree),
        TEST_REGISTER(test_bst_perfect_tree),
        TEST_REGISTER(test_bst_duplicate_handling),
        TEST_REGISTER(test_bst_negative_numbers),
        TEST_REGISTER(test_bst_root_removal_cases),
        TEST_REGISTER(test_bst_complex_operations),
        TEST_REGISTER(test_bst_custom_comparison),
        TEST_REGISTER(test_bst_boundary_conditions),

        // Fuzz Tests
        TEST_REGISTER(test_bst_fuzz),

        // Memory Failure Tests
        TEST_REGISTER(test_bst_failing_allocator_create),
        TEST_REGISTER(test_bst_failing_allocator_insert),
    };

    return anv_run_tests("BST", tests, sizeof(tests) / sizeof(tests[0]));
}