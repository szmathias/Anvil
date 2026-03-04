#include <stdio.h>
#include <stdlib.h>

#include <anvil/testing.h>
#include "TestHelpers.h"
#include "common/allocator.h"
#include "containers/arraylist.h"
#include "containers/doublylinkedlist.h"
#include "containers/hashset.h"
#include "containers/iterator.h"
#include "containers/queue.h"
#include "containers/singlylinkedlist.h"
#include "containers/stack.h"

//==============================================================================
// Static Helpers
//==============================================================================

static int collect_values(const ANVIterator* it, int* values, const int max_count)
{
    int count = 0;
    while (it->has_next(it) && count < max_count)
    {
        const int* value = it->get(it);
        if (value)
        {
            values[count++] = *value;
        }
        it->next(it);
    }
    return count;
}

static int verify_values(const int* actual, const int* expected, const int count, const char* test_name)
{
    for (int i = 0; i < count; i++)
    {
        if (actual[i] != expected[i])
        {
            printf("FAIL: %s - Expected %d at index %d, got %d\n",
                   test_name, expected[i], i, actual[i]);
            return 0;
        }
    }
    return 1;
}

static ANVDoublyLinkedList* create_test_list(ANVAllocator* alloc, const int n)
{
    ANVDoublyLinkedList* list = anv_dll_create(alloc);
    if (!list)
        return NULL;

    for (int i = 1; i <= n; i++)
    {
        int* val = malloc(sizeof(int));
        if (!val)
        {
            anv_dll_destroy(list, true);
            return NULL;
        }
        *val = i;
        anv_dll_push_back(list, val);
    }
    return list;
}

static ANVDoublyLinkedList* create_list_with_values(ANVAllocator* alloc, const int* values, const int count)
{
    ANVDoublyLinkedList* list = anv_dll_create(alloc);
    if (!list)
        return NULL;

    for (int i = 0; i < count; i++)
    {
        int* val = malloc(sizeof(int));
        if (!val)
        {
            anv_dll_destroy(list, true);
            return NULL;
        }
        *val = values[i];
        anv_dll_push_back(list, val);
    }
    return list;
}

static int collect_values_with_validation(const ANVIterator* it, int* values, int max_count)
{
    int count = 0;
    while (it->has_next(it) && count < max_count)
    {
        const int* value = it->get(it);
        if (!value)
        {
            // get() returned null but has_next was true - this is an error
            return -1;
        }

        values[count++] = *value;

        // Verify that next() succeeds when has_next() is true
        if (it->next(it) != 0)
        {
            // next() failed but has_next() was true - this is an error
            return -1;
        }
    }
    return count;
}

//==============================================================================
// Chains-Specific Helper
//==============================================================================

static int collect_values_with_validation_chains(const ANVIterator* it, int* values, int max_count)
{
    int count = 0;
    while (it->has_next(it) && count < max_count)
    {
        const int* value = it->get(it);
        if (!value)
        {
            // get() returned null but has_next was true - this is an error
            return -1;
        }

        values[count++] = *value;

        // Only call next() if we're not at the last element we want to collect
        // or if there are more elements after this one
        if (count < max_count && it->has_next(it))
        {
            if (it->next(it) != 0)
            {
                // next() failed but has_next() was true - this is an error
                return -1;
            }
        }
        else
        {
            // We've collected enough elements or this is the last element
            // Try to advance, but don't treat failure as an error since we might be at the end
            it->next(it);
            break;
        }
    }
    return count;
}

//==============================================================================
// Copy-Specific Helper
//==============================================================================

/**
 * Helper function to collect all values from a copy iterator into an array.
 * Returns the number of values collected.
 * Note: For copy iterator, the user owns the returned values and must free them.
 */
static int collect_values_copy(const ANVIterator* it, int** values, const int max_count)
{
    int count = 0;
    while (it->has_next(it) && count < max_count)
    {
        int* value = it->get(it);
        if (value)
        {
            values[count++] = value;
        }
        it->next(it);
    }
    return count;
}

/**
 * Helper function to verify an array matches expected values (copy variant).
 */
static int verify_values_copy(int** actual, const int* expected, const int count, const char* test_name)
{
    for (int i = 0; i < count; i++)
    {
        if (!actual[i] || *actual[i] != expected[i])
        {
            printf("FAIL: %s - Expected %d at index %d, got %s\n",
                   test_name, expected[i], i, actual[i] ? "different value" : "NULL");
            return 0;
        }
    }
    return 1;
}

/**
 * Helper function to free collected values from copy iterator.
 */
static void free_collected_values(int** values, const int count)
{
    for (int i = 0; i < count; i++)
    {
        free(values[i]);
    }
}

/**
 * Helper function with better validation for copy iterator.
 */
static int collect_values_with_validation_copy(const ANVIterator* it, int** values, int max_count)
{
    int count = 0;
    while (it->has_next(it) && count < max_count)
    {
        int* value = it->get(it);
        if (!value)
        {
            // get() returned null but has_next was true - this is an error
            return -1;
        }

        values[count++] = value;

        // Verify that next() succeeds when has_next() is true
        if (it->next(it) != 0)
        {
            // next() failed but has_next() was true - this is an error
            return -1;
        }
    }
    return count;
}

//==============================================================================
// Repeat-Specific Helper Functions
//==============================================================================

static int verify_repeated_values(const int* actual, const int expected_value, const int count, const char* test_name)
{
    for (int i = 0; i < count; i++)
    {
        if (actual[i] != expected_value)
        {
            printf("FAIL: %s - Expected %d at index %d, got %d\n",
                   test_name, expected_value, i, actual[i]);
            return 0;
        }
    }
    return 1;
}

//==============================================================================
// Enumerate-Specific Helper Functions
//==============================================================================

static int collect_indexed_elements(const ANVIterator* it, size_t* indices, int* values, const int max_count)
{
    int count = 0;
    while (it->has_next(it) && count < max_count)
    {
        const ANVIndexedElement* indexed = it->get(it);
        if (indexed && indexed->element)
        {
            indices[count] = indexed->index;
            values[count] = *(const int*)indexed->element;
            count++;
        }
        it->next(it);
    }
    return count;
}

/**
 * Helper function to verify indexed elements match expected values.
 */
static int verify_indexed_elements(const size_t* actual_indices, const int* actual_values,
                                   const size_t* expected_indices, const int* expected_values,
                                   const int count, const char* test_name)
{
    for (int i = 0; i < count; i++)
    {
        if (actual_indices[i] != expected_indices[i] || actual_values[i] != expected_values[i])
        {
            printf("FAIL: %s - Expected (index=%zu,value=%d) at position %d, got (index=%zu,value=%d)\n",
                   test_name, expected_indices[i], expected_values[i], i,
                   actual_indices[i], actual_values[i]);
            return 0;
        }
    }
    return 1;
}

//==============================================================================
// Zip-Specific Helper Functions
//==============================================================================

/**
 * Helper function to collect all pairs from a zip iterator into arrays.
 * Returns the number of pairs collected.
 */
static int collect_pairs(const ANVIterator* it, int* first_values, int* second_values, const int max_count)
{
    int count = 0;
    while (it->has_next(it) && count < max_count)
    {
        const ANVPair* pair = it->get(it);
        if (pair && pair->first && pair->second)
        {
            first_values[count] = *(const int*)pair->first;
            second_values[count] = *(const int*)pair->second;
            count++;
        }
        it->next(it);
    }
    return count;
}

/**
 * Helper function to verify pairs match expected values.
 */
static int verify_pairs(const int* actual_first, const int* actual_second,
                        const int* expected_first, const int* expected_second,
                        const int count, const char* test_name)
{
    for (int i = 0; i < count; i++)
    {
        if (actual_first[i] != expected_first[i] || actual_second[i] != expected_second[i])
        {
            printf("FAIL: %s - Expected (%d,%d) at index %d, got (%d,%d)\n",
                   test_name, expected_first[i], expected_second[i], i,
                   actual_first[i], actual_second[i]);
            return 0;
        }
    }
    return 1;
}

//==============================================================================
// Chain Iterator Tests
//==============================================================================

static int test_chain_basic_functionality(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create two range iterators to chain
    const ANVIterator range1 = anv_iterator_range(&alloc, 1, 4, 1);   // [1,2,3]
    const ANVIterator range2 = anv_iterator_range(&alloc, 10, 13, 1); // [10,11,12]

    // Create array of iterators to chain
    ANVIterator iterators[] = {range1, range2};
    ANVIterator chain_it = anv_iterator_chain(iterators, 2, &alloc);
    ASSERT_TRUE(chain_it.is_valid(&chain_it));

    int values[10];
    const int count = collect_values(&chain_it, values, 10);

    ASSERT_EQ(count, 6);

    // Should get all elements from first iterator, then all from second
    const int expected[] = {1, 2, 3, 10, 11, 12};
    ASSERT_TRUE(verify_values(values, expected, 6, "chain_basic"));

    chain_it.destroy(&chain_it);
    return TEST_SUCCESS;
}

static int test_chain_single_iterator(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Chain a single iterator
    const ANVIterator range1 = anv_iterator_range(&alloc, 5, 8, 1); // [5,6,7]

    ANVIterator iterators[] = {range1};
    ANVIterator chain_it = anv_iterator_chain(iterators, 1, &alloc);
    ASSERT_TRUE(chain_it.is_valid(&chain_it));

    int values[10];
    const int count = collect_values(&chain_it, values, 10);

    ASSERT_EQ(count, 3);

    const int expected[] = {5, 6, 7};
    ASSERT_TRUE(verify_values(values, expected, 3, "chain_single"));

    chain_it.destroy(&chain_it);
    return TEST_SUCCESS;
}

static int test_chain_empty_iterators(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create empty range iterators (start == end)
    const ANVIterator range1 = anv_iterator_range(&alloc, 5, 5, 1);   // empty
    const ANVIterator range2 = anv_iterator_range(&alloc, 10, 10, 1); // empty

    ANVIterator iterators[] = {range1, range2};
    ANVIterator chain_it = anv_iterator_chain(iterators, 2, &alloc);
    ASSERT_TRUE(chain_it.is_valid(&chain_it));

    ASSERT_FALSE(chain_it.has_next(&chain_it));
    ASSERT_NULL(chain_it.get(&chain_it));

    chain_it.destroy(&chain_it);
    return TEST_SUCCESS;
}

static int test_chain_mixed_empty_and_non_empty(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Mix empty and non-empty iterators
    const ANVIterator range1 = anv_iterator_range(&alloc, 1, 1, 1);   // empty
    const ANVIterator range2 = anv_iterator_range(&alloc, 5, 7, 1);   // [5,6]
    const ANVIterator range3 = anv_iterator_range(&alloc, 10, 10, 1); // empty
    const ANVIterator range4 = anv_iterator_range(&alloc, 20, 22, 1); // [20,21]

    ANVIterator iterators[] = {range1, range2, range3, range4};
    ANVIterator chain_it = anv_iterator_chain(iterators, 4, &alloc);
    ASSERT_TRUE(chain_it.is_valid(&chain_it));

    int values[10];
    const int count = collect_values(&chain_it, values, 10);

    ASSERT_EQ(count, 4);

    const int expected[] = {5, 6, 20, 21};
    ASSERT_TRUE(verify_values(values, expected, 4, "chain_mixed"));

    chain_it.destroy(&chain_it);
    return TEST_SUCCESS;
}

static int test_chain_multiple_iterators(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Chain multiple range iterators
    const ANVIterator range1 = anv_iterator_range(&alloc, 1, 3, 1);   // [1,2]
    const ANVIterator range2 = anv_iterator_range(&alloc, 10, 12, 1); // [10,11]
    const ANVIterator range3 = anv_iterator_range(&alloc, 20, 22, 1); // [20,21]
    const ANVIterator range4 = anv_iterator_range(&alloc, 30, 32, 1); // [30,31]

    ANVIterator iterators[] = {range1, range2, range3, range4};
    ANVIterator chain_it = anv_iterator_chain(iterators, 4, &alloc);
    ASSERT_TRUE(chain_it.is_valid(&chain_it));

    int values[20];
    const int count = collect_values(&chain_it, values, 20);

    ASSERT_EQ(count, 8);

    const int expected[] = {1, 2, 10, 11, 20, 21, 30, 31};
    ASSERT_TRUE(verify_values(values, expected, 8, "chain_multiple"));

    chain_it.destroy(&chain_it);
    return TEST_SUCCESS;
}

static int test_chain_with_repeat_iterators(void)
{
    const ANVAllocator alloc = create_int_allocator();

    const int value1 = 42;
    const int value2 = 99;

    // Chain repeat iterators with different values
    const ANVIterator repeat1 = anv_iterator_repeat(&value1, &alloc, 3); // [42,42,42]
    const ANVIterator repeat2 = anv_iterator_repeat(&value2, &alloc, 2); // [99,99]

    ANVIterator iterators[] = {repeat1, repeat2};
    ANVIterator chain_it = anv_iterator_chain(iterators, 2, &alloc);
    ASSERT_TRUE(chain_it.is_valid(&chain_it));

    int values[10];
    const int count = collect_values(&chain_it, values, 10);

    ASSERT_EQ(count, 5);

    const int expected[] = {42, 42, 42, 99, 99};
    ASSERT_TRUE(verify_values(values, expected, 5, "chain_repeat"));

    chain_it.destroy(&chain_it);
    return TEST_SUCCESS;
}

static int test_chain_with_take_skip_iterators(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create base range and apply take/skip to different copies
    ANVIterator range1 = anv_iterator_range(&alloc, 1, 10, 1); // [1,2,3,4,5,6,7,8,9]
    ANVIterator range2 = anv_iterator_range(&alloc, 1, 10, 1); // [1,2,3,4,5,6,7,8,9]

    const ANVIterator take_it = anv_iterator_take(&range1, &alloc, 3); // [1,2,3]
    const ANVIterator skip_it = anv_iterator_skip(&range2, &alloc, 6); // [7,8,9]

    ANVIterator iterators[] = {take_it, skip_it};
    ANVIterator chain_it = anv_iterator_chain(iterators, 2, &alloc);
    ASSERT_TRUE(chain_it.is_valid(&chain_it));

    int values[10];
    const int count = collect_values(&chain_it, values, 10);

    ASSERT_EQ(count, 6);

    const int expected[] = {1, 2, 3, 7, 8, 9};
    ASSERT_TRUE(verify_values(values, expected, 6, "chain_take_skip"));

    chain_it.destroy(&chain_it);
    return TEST_SUCCESS;
}

static int test_chain_invalid_parameters(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Test with NULL iterators array
    const ANVIterator chain_it1 = anv_iterator_chain(NULL, 2, &alloc);
    ASSERT_FALSE(chain_it1.is_valid(&chain_it1));

    // Test with zero count
    ANVIterator range = anv_iterator_range(&alloc, 1, 3, 1);
    ANVIterator iterators[] = {range};
    const ANVIterator chain_it2 = anv_iterator_chain(iterators, 0, &alloc);
    ASSERT_FALSE(chain_it2.is_valid(&chain_it2));

    // Test with NULL allocator
    const ANVIterator chain_it3 = anv_iterator_chain(iterators, 1, NULL);
    ASSERT_FALSE(chain_it3.is_valid(&chain_it3));

    // Clean up the range iterator that wasn't consumed
    range.destroy(&range);

    return TEST_SUCCESS;
}

static int test_chain_iterator_operations(void)
{
    const ANVAllocator alloc = create_int_allocator();

    const ANVIterator range1 = anv_iterator_range(&alloc, 1, 3, 1);   // [1,2]
    const ANVIterator range2 = anv_iterator_range(&alloc, 10, 12, 1); // [10,11]

    ANVIterator iterators[] = {range1, range2};
    ANVIterator chain_it = anv_iterator_chain(iterators, 2, &alloc);

    // Test initial state
    ASSERT_TRUE(chain_it.has_next(&chain_it));
    const int* value = chain_it.get(&chain_it);
    ASSERT_NOT_NULL(value);
    ASSERT_EQ(*value, 1);

    // Test advancement
    ASSERT_EQ(chain_it.next(&chain_it), 0);
    ASSERT_TRUE(chain_it.has_next(&chain_it));
    value = chain_it.get(&chain_it);
    ASSERT_NOT_NULL(value);
    ASSERT_EQ(*value, 2);

    // Test transition to second iterator
    ASSERT_EQ(chain_it.next(&chain_it), 0);
    ASSERT_TRUE(chain_it.has_next(&chain_it));
    value = chain_it.get(&chain_it);
    ASSERT_NOT_NULL(value);
    ASSERT_EQ(*value, 10);

    // Continue through second iterator
    ASSERT_EQ(chain_it.next(&chain_it), 0);
    ASSERT_TRUE(chain_it.has_next(&chain_it));
    value = chain_it.get(&chain_it);
    ASSERT_NOT_NULL(value);
    ASSERT_EQ(*value, 11);

    // Test end of iteration
    ASSERT_EQ(chain_it.next(&chain_it), 0);
    ASSERT_FALSE(chain_it.has_next(&chain_it));
    ASSERT_NULL(chain_it.get(&chain_it));

    // Test operations not supported
    ASSERT_FALSE(chain_it.has_prev(&chain_it));
    ASSERT_EQ(chain_it.prev(&chain_it), -1);

    chain_it.destroy(&chain_it);
    return TEST_SUCCESS;
}

static int test_chain_with_nested_chains(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create first chain: [1,2] + [10,11]
    const ANVIterator range1 = anv_iterator_range(&alloc, 1, 3, 1);
    const ANVIterator range2 = anv_iterator_range(&alloc, 10, 12, 1);
    ANVIterator iterators1[] = {range1, range2};
    const ANVIterator chain1 = anv_iterator_chain(iterators1, 2, &alloc);

    // Create second chain: [20,21] + [30,31]
    const ANVIterator range3 = anv_iterator_range(&alloc, 20, 22, 1);
    const ANVIterator range4 = anv_iterator_range(&alloc, 30, 32, 1);
    ANVIterator iterators2[] = {range3, range4};
    const ANVIterator chain2 = anv_iterator_chain(iterators2, 2, &alloc);

    // Chain the two chain iterators
    ANVIterator master_iterators[] = {chain1, chain2};
    ANVIterator master_chain = anv_iterator_chain(master_iterators, 2, &alloc);
    ASSERT_TRUE(master_chain.is_valid(&master_chain));

    int values[20];
    const int count = collect_values(&master_chain, values, 20);

    ASSERT_EQ(count, 8);

    // Should get: [1,2,10,11,20,21,30,31]
    const int expected[] = {1, 2, 10, 11, 20, 21, 30, 31};
    ASSERT_TRUE(verify_values(values, expected, 8, "chain_nested"));

    master_chain.destroy(&master_chain);
    return TEST_SUCCESS;
}

static int test_chain_with_arraylist_and_dll(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create ArrayList with values [1, 2, 3, 4, 5]
    ANVArrayList* arraylist = anv_arraylist_create(&alloc, 5);
    for (int i = 1; i <= 5; i++)
    {
        int* val = alloc.allocate(sizeof(int));
        *val = i;
        anv_arraylist_push_back(arraylist, val);
    }

    // Create DoublyLinkedList with values [10, 20, 30]
    ANVDoublyLinkedList* dll = anv_dll_create(&alloc);
    for (int i = 1; i <= 3; i++)
    {
        int* val = alloc.allocate(sizeof(int));
        *val = i * 10;
        anv_dll_push_back(dll, val);
    }

    // Get iterators from data structures
    ANVIterator arraylist_it = anv_arraylist_iterator(arraylist);
    ANVIterator dll_it = anv_dll_iterator(dll);

    // Apply transformations - filter even numbers from arraylist, take first 2 from dll
    const ANVIterator filtered_arraylist = anv_iterator_filter(&arraylist_it, &alloc, is_even);
    const ANVIterator taken_dll = anv_iterator_take(&dll_it, &alloc, 2);

    // Chain the transformed iterators
    ANVIterator iterators[] = {filtered_arraylist, taken_dll};
    ANVIterator chain_it = anv_iterator_chain(iterators, 2, &alloc);
    ASSERT_TRUE(chain_it.is_valid(&chain_it));

    int values[10];
    const int count = collect_values(&chain_it, values, 10);

    ASSERT_EQ(count, 4); // [2, 4] from filtered arraylist + [10, 20] from taken dll

    const int expected[] = {2, 4, 10, 20};
    ASSERT_TRUE(verify_values(values, expected, 4, "chain_arraylist_dll"));

    chain_it.destroy(&chain_it);
    anv_arraylist_destroy(arraylist, true);
    anv_dll_destroy(dll, true);
    return TEST_SUCCESS;
}

static int test_chain_with_stack_queue_and_hashset(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create Stack with values [100, 200, 300] (pushed in order, so top is 300)
    ANVStack* stack = anv_stack_create(&alloc);
    for (int i = 1; i <= 3; i++)
    {
        int* val = alloc.allocate(sizeof(int));
        *val = i * 100;
        anv_stack_push(stack, val);
    }

    // Create Queue with values [5, 10, 15, 20, 25]
    ANVQueue* queue = anv_queue_create(&alloc);
    for (int i = 1; i <= 5; i++)
    {
        int* val = alloc.allocate(sizeof(int));
        *val = i * 5;
        anv_queue_enqueue(queue, val);
    }

    // Create HashSet with values [7, 14, 21, 28, 35] (multiples of 7)
    ANVHashSet* hashset = anv_hashset_create(&alloc, anv_hash_int, int_cmp, 0);
    for (int i = 1; i <= 5; i++)
    {
        int* val = alloc.allocate(sizeof(int));
        *val = i * 7;
        anv_hashset_add(hashset, val);
    }

    // Get iterators from data structures
    ANVIterator stack_it = anv_stack_iterator(stack);
    ANVIterator queue_it = anv_queue_iterator(queue);
    ANVIterator hashset_it = anv_hashset_iterator(hashset);

    // Apply various transformations
    ANVIterator taken_stack = anv_iterator_take(&stack_it, &alloc, 2);                           // First 2 from stack
    ANVIterator skipped_queue = anv_iterator_skip(&queue_it, &alloc, 2);                         // Skip first 2 from queue
    ANVIterator filtered_hashset = anv_iterator_filter(&hashset_it, &alloc, is_greater_than_20); // > 20 from hashset

    // Chain all three transformed iterators
    ANVIterator iterators[] = {taken_stack, skipped_queue, filtered_hashset};
    ANVIterator chain_it = anv_iterator_chain(iterators, 3, &alloc);
    ASSERT_TRUE(chain_it.is_valid(&chain_it));

    int values[20];
    const int count = collect_values(&chain_it, values, 20);

    // Note: HashSet order is not guaranteed, so we'll just verify the count and that values are present
    ASSERT_TRUE(count >= 6); // At least 2 from stack + 3 from queue + 1+ from hashset

    // Verify stack values are first (300, 200 - stack iteration order)
    ASSERT_EQ(values[0], 300);
    ASSERT_EQ(values[1], 200);

    // Verify queue values come next (15, 20, 25 - after skipping 5, 10)
    ASSERT_EQ(values[2], 15);
    ASSERT_EQ(values[3], 20);
    ASSERT_EQ(values[4], 25);

    // Remaining values should be from hashset (> 20): could be 21, 28, 35 in any order
    for (int i = 5; i < count; i++)
    {
        ASSERT_TRUE(values[i] > 20 && values[i] % 7 == 0);
    }

    chain_it.destroy(&chain_it);
    anv_stack_destroy(stack, true);
    anv_queue_destroy(queue, true);
    anv_hashset_destroy(hashset, true);
    return TEST_SUCCESS;
}

static int test_chain_with_complex_transformations(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create ArrayList for source data
    ANVArrayList* list1 = anv_arraylist_create(&alloc, 10);
    ANVArrayList* list2 = anv_arraylist_create(&alloc, 10);

    // Fill first list with [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    for (int i = 1; i <= 10; i++)
    {
        int* val = alloc.allocate(sizeof(int));
        *val = i;
        anv_arraylist_push_back(list1, val);
    }

    // Fill second list with [11, 12, 13, 14, 15]
    for (int i = 11; i <= 15; i++)
    {
        int* val = alloc.allocate(sizeof(int));
        *val = i;
        anv_arraylist_push_back(list2, val);
    }

    // Get base iterators
    ANVIterator it1 = anv_arraylist_iterator(list1);
    ANVIterator it2 = anv_arraylist_iterator(list2);

    // Create complex transformation chain 1: skip(2) -> filter(even) -> take(3) -> double
    ANVIterator skipped1 = anv_iterator_skip(&it1, &alloc, 2);                       // [3,4,5,6,7,8,9,10]
    ANVIterator filtered1 = anv_iterator_filter(&skipped1, &alloc, is_even);         // [4,6,8,10]
    ANVIterator taken1 = anv_iterator_take(&filtered1, &alloc, 3);                   // [4,6,8]
    ANVIterator doubled1 = anv_iterator_transform(&taken1, &alloc, double_value, 1); // [8,12,16]

    // Create complex transformation chain 2: filter(odd) -> enumerate -> take(2)
    ANVIterator filtered2 = anv_iterator_filter(&it2, &alloc, is_odd);       // [11,13,15]
    ANVIterator enumerated2 = anv_iterator_enumerate(&filtered2, &alloc, 0); // [(0,11),(1,13),(2,15)]
    ANVIterator taken2 = anv_iterator_take(&enumerated2, &alloc, 2);         // [(0,11),(1,13)]

    // Chain the two complex transformation results
    ANVIterator iterators[] = {doubled1, taken2};
    ANVIterator chain_it = anv_iterator_chain(iterators, 2, &alloc);
    ASSERT_TRUE(chain_it.is_valid(&chain_it));

    // Collect and verify first part (doubled values)
    ASSERT_TRUE(chain_it.has_next(&chain_it));
    const int* val = chain_it.get(&chain_it);
    ASSERT_NOT_NULL(val);
    ASSERT_EQ(*val, 8);

    chain_it.next(&chain_it);
    ASSERT_TRUE(chain_it.has_next(&chain_it));
    val = chain_it.get(&chain_it);
    ASSERT_NOT_NULL(val);
    ASSERT_EQ(*val, 12);

    chain_it.next(&chain_it);
    ASSERT_TRUE(chain_it.has_next(&chain_it));
    val = chain_it.get(&chain_it);
    ASSERT_NOT_NULL(val);
    ASSERT_EQ(*val, 16);

    // Now we should get the enumerated elements
    chain_it.next(&chain_it);
    ASSERT_TRUE(chain_it.has_next(&chain_it));
    const ANVIndexedElement* indexed = chain_it.get(&chain_it);
    ASSERT_NOT_NULL(indexed);
    ASSERT_EQ(indexed->index, 0);
    ASSERT_EQ(*(int*)indexed->element, 11);

    chain_it.next(&chain_it);
    ASSERT_TRUE(chain_it.has_next(&chain_it));
    indexed = chain_it.get(&chain_it);
    ASSERT_NOT_NULL(indexed);
    ASSERT_EQ(indexed->index, 1);
    ASSERT_EQ(*(int*)indexed->element, 13);

    chain_it.next(&chain_it);
    ASSERT_FALSE(chain_it.has_next(&chain_it));

    chain_it.destroy(&chain_it);
    anv_arraylist_destroy(list1, true);
    anv_arraylist_destroy(list2, true);
    return TEST_SUCCESS;
}

static int test_chain_with_zip_and_data_structures(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create data structures
    ANVArrayList* arraylist = anv_arraylist_create(&alloc, 5);
    ANVSinglyLinkedList* sll = anv_sll_create(&alloc);

    // Fill ArrayList with [1, 2, 3, 4, 5]
    for (int i = 1; i <= 5; i++)
    {
        int* val = alloc.allocate(sizeof(int));
        *val = i;
        anv_arraylist_push_back(arraylist, val);
    }

    // Fill SLL with [10, 20, 30]
    for (int i = 1; i <= 3; i++)
    {
        int* val = alloc.allocate(sizeof(int));
        *val = i * 10;
        anv_sll_push_back(sll, val);
    }

    // Get iterators and apply transformations
    ANVIterator arraylist_it = anv_arraylist_iterator(arraylist);
    ANVIterator sll_it = anv_sll_iterator(sll);

    ANVIterator filtered_arraylist = anv_iterator_filter(&arraylist_it, &alloc, is_odd); // [1,3,5]
    ANVIterator taken_sll = anv_iterator_take(&sll_it, &alloc, 2);                       // [10,20]

    // Zip the transformed iterators
    const ANVIterator zipped = anv_iterator_zip(&filtered_arraylist, &taken_sll, &alloc); // [(1,10),(3,20)]

    // Create a simple range for chaining
    const ANVIterator range_it = anv_iterator_range(&alloc, 100, 103, 1); // [100,101,102]

    // Chain zip result with range
    ANVIterator iterators[] = {zipped, range_it};
    ANVIterator chain_it = anv_iterator_chain(iterators, 2, &alloc);
    ASSERT_TRUE(chain_it.is_valid(&chain_it));

    // Verify zipped pairs first
    ASSERT_TRUE(chain_it.has_next(&chain_it));
    const ANVPair* pair = chain_it.get(&chain_it);
    ASSERT_NOT_NULL(pair);
    ASSERT_EQ(*(int*)pair->first, 1);
    ASSERT_EQ(*(int*)pair->second, 10);

    chain_it.next(&chain_it);
    ASSERT_TRUE(chain_it.has_next(&chain_it));
    pair = chain_it.get(&chain_it);
    ASSERT_NOT_NULL(pair);
    ASSERT_EQ(*(int*)pair->first, 3);
    ASSERT_EQ(*(int*)pair->second, 20);

    // Then verify range values
    chain_it.next(&chain_it);
    ASSERT_TRUE(chain_it.has_next(&chain_it));
    const int* val = chain_it.get(&chain_it);
    ASSERT_NOT_NULL(val);
    ASSERT_EQ(*val, 100);

    chain_it.next(&chain_it);
    ASSERT_TRUE(chain_it.has_next(&chain_it));
    val = chain_it.get(&chain_it);
    ASSERT_NOT_NULL(val);
    ASSERT_EQ(*val, 101);

    chain_it.next(&chain_it);
    ASSERT_TRUE(chain_it.has_next(&chain_it));
    val = chain_it.get(&chain_it);
    ASSERT_NOT_NULL(val);
    ASSERT_EQ(*val, 102);

    chain_it.next(&chain_it);
    ASSERT_FALSE(chain_it.has_next(&chain_it));

    chain_it.destroy(&chain_it);
    anv_arraylist_destroy(arraylist, true);
    anv_sll_destroy(sll, true);
    return TEST_SUCCESS;
}

static int test_chain_data_structure_round_trip(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create original ArrayList
    ANVArrayList* original_list = anv_arraylist_create(&alloc, 5);
    for (int i = 1; i <= 5; i++)
    {
        int* val = alloc.allocate(sizeof(int));
        *val = i * i; // [1, 4, 9, 16, 25]
        anv_arraylist_push_back(original_list, val);
    }

    // Create range iterator
    const ANVIterator range_it = anv_iterator_range(&alloc, 100, 103, 1); // [100, 101, 102]

    // Get iterator from ArrayList and apply transformation
    ANVIterator list_it = anv_arraylist_iterator(original_list);
    const ANVIterator filtered_list = anv_iterator_filter(&list_it, &alloc, is_greater_than_10); // [16, 25]

    // Chain filtered list with range
    ANVIterator iterators[] = {filtered_list, range_it};
    ANVIterator chained = anv_iterator_chain(iterators, 2, &alloc);

    // Create new data structures from the chained iterator
    ANVDoublyLinkedList* result_dll = anv_dll_from_iterator(&chained, &alloc, true);
    ASSERT_NOT_NULL(result_dll);
    ASSERT_EQ(anv_dll_size(result_dll), 5); // [16, 25, 100, 101, 102]

    // Verify the round-trip worked correctly
    ANVIterator result_it = anv_dll_iterator(result_dll);

    const int expected[] = {16, 25, 100, 101, 102};
    int i = 0;
    while (result_it.has_next(&result_it))
    {
        const int* val = result_it.get(&result_it);
        ASSERT_NOT_NULL(val);
        ASSERT_EQ(*val, expected[i]);
        i++;
        result_it.next(&result_it);
    }
    ASSERT_EQ(i, 5);

    chained.destroy(&chained);
    result_it.destroy(&result_it);
    anv_dll_destroy(result_dll, true);
    anv_arraylist_destroy(original_list, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Iterator Chaining Tests
//==============================================================================

/**
 * Test range iterator chained with filter for even numbers.
 */
static int test_range_filter_even(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 11, 1);
    ASSERT_TRUE(range_it.is_valid(&range_it));

    // Chain with even filter
    ANVIterator filter_it = anv_iterator_filter(&range_it, &alloc, is_even);
    ASSERT_TRUE(filter_it.is_valid(&filter_it));

    // Expected: [1,2,3,4,5,6,7,8,9,10] → [2,4,6,8,10]
    const int expected[] = {2, 4, 6, 8, 10};
    int actual[5];
    const int count = collect_values(&filter_it, actual, 5);

    ASSERT_EQ(count, 5);
    ASSERT_TRUE(verify_values(actual, expected, count, "range_filter_even"));

    filter_it.destroy(&filter_it);
    return TEST_SUCCESS;
}

/**
 * Test range iterator with step chained with divisible by 3 filter.
 */
static int test_range_step_filter_div3(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range [2, 5, 8, 11, 14, 17, 20]
    ANVIterator range_it = anv_iterator_range(&alloc, 2, 21, 3);
    ASSERT_TRUE(range_it.is_valid(&range_it));

    // Chain with divisible by 3 filter
    ANVIterator filter_it = anv_iterator_filter(&range_it, &alloc, is_divisible_by_3);
    ASSERT_TRUE(filter_it.is_valid(&filter_it));

    // Expected: [2,5,8,11,14,17,20] → [] (none divisible by 3)
    ASSERT_FALSE(filter_it.has_next(&filter_it));
    ASSERT_NULL(filter_it.get(&filter_it));

    filter_it.destroy(&filter_it);
    return TEST_SUCCESS;
}

/**
 * Test range iterator chained with greater than 5 filter.
 */
static int test_range_filter_greater_than_5(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range [1, 2, 3, 4, 5, 6, 7, 8]
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 9, 1);

    // Chain with greater than 5 filter
    ANVIterator filter_it = anv_iterator_filter(&range_it, &alloc, is_greater_than_five);

    // Expected: [1,2,3,4,5,6,7,8] → [6,7,8]
    const int expected[] = {6, 7, 8};
    int actual[3];
    const int count = collect_values(&filter_it, actual, 3);

    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_values(actual, expected, count, "range_filter_gt5"));

    filter_it.destroy(&filter_it);
    return TEST_SUCCESS;
}

/**
 * Test range iterator chained with double transform.
 */
static int test_range_transform_double(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range [1, 2, 3, 4, 5]
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 6, 1);

    // Chain with double transform
    ANVIterator transform_it = anv_iterator_transform(&range_it, &alloc, double_value, true);

    // Expected: [1,2,3,4,5] → [2,4,6,8,10]
    const int expected[] = {2, 4, 6, 8, 10};
    int actual[5];
    const int count = collect_values(&transform_it, actual, 5);

    ASSERT_EQ(count, 5);
    ASSERT_TRUE(verify_values(actual, expected, count, "range_transform_double"));

    transform_it.destroy(&transform_it);
    return TEST_SUCCESS;
}

/**
 * Test range iterator chained with square transform.
 */
static int test_range_transform_square(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range [2, 4, 6]
    ANVIterator range_it = anv_iterator_range(&alloc, 2, 7, 2);

    // Chain with square transform
    ANVIterator transform_it = anv_iterator_transform(&range_it, &alloc, square_func, true);

    // Expected: [2,4,6] → [4,16,36]
    const int expected[] = {4, 16, 36};
    int actual[3];
    const int count = collect_values(&transform_it, actual, 3);

    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_values(actual, expected, count, "range_transform_square"));

    transform_it.destroy(&transform_it);
    return TEST_SUCCESS;
}

/**
 * Test range iterator chained with add_ten transform.
 */
static int test_range_transform_add_ten(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range [1, 2, 3, 4]
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 5, 1);

    // Chain with add_ten transform
    ANVIterator transform_it = anv_iterator_transform(&range_it, &alloc, add_ten_func, true);

    // Expected: [1,2,3,4] → [11,12,13,14]
    const int expected[] = {11, 12, 13, 14};
    int actual[4];
    const int count = collect_values(&transform_it, actual, 4);

    ASSERT_EQ(count, 4);
    ASSERT_TRUE(verify_values(actual, expected, count, "range_transform_add_ten"));

    transform_it.destroy(&transform_it);
    return TEST_SUCCESS;
}

/**
 * Test filter iterator chained with transform (using list base).
 */
static int test_filter_transform_even_double(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 6);
    ASSERT_NOT_NULL(list);

    // Create base iterator
    ANVIterator base_it = anv_dll_iterator(list);

    // Chain: filter even → transform double
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_even);
    ANVIterator transform_it = anv_iterator_transform(&filter_it, &alloc, double_value, true);

    // Expected: [1,2,3,4,5,6] → [2,4,6] → [4,8,12]
    const int expected[] = {4, 8, 12};
    int actual[3];
    const int count = collect_values(&transform_it, actual, 3);

    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_values(actual, expected, count, "filter_transform_even_double"));

    transform_it.destroy(&transform_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test filter iterator chained with square transform.
 */
static int test_filter_transform_odd_square(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 5);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);

    // Chain: filter odd → transform square
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_odd);
    ANVIterator transform_it = anv_iterator_transform(&filter_it, &alloc, square_func, true);

    // Expected: [1,2,3,4,5] → [1,3,5] → [1,9,25]
    const int expected[] = {1, 9, 25};
    int actual[3];
    const int count = collect_values(&transform_it, actual, 3);

    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_values(actual, expected, count, "filter_transform_odd_square"));

    transform_it.destroy(&transform_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test filter with no matches chained with transform.
 */
static int test_filter_transform_no_matches(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create list with only odd numbers
    const int odd_values[] = {1, 3, 5, 7};
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    for (int i = 0; i < 4; i++)
    {
        int* val = malloc(sizeof(int));
        *val = odd_values[i];
        anv_dll_push_back(list, val);
    }

    ANVIterator base_it = anv_dll_iterator(list);

    // Chain: filter even (no matches) → transform double
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_even);
    ANVIterator transform_it = anv_iterator_transform(&filter_it, &alloc, double_value, true);

    // Expected: [1,3,5,7] → [] → []
    ASSERT_FALSE(transform_it.has_next(&transform_it));
    ASSERT_NULL(transform_it.get(&transform_it));

    transform_it.destroy(&transform_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test transform iterator chained with filter.
 */
static int test_transform_filter_add_one_even(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 5);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);

    // Chain: transform add_one → filter even
    ANVIterator transform_it = anv_iterator_transform(&base_it, &alloc, add_one, true);
    ANVIterator filter_it = anv_iterator_filter(&transform_it, &alloc, is_even);

    // Expected: [1,2,3,4,5] → [2,3,4,5,6] → [2,4,6]
    const int expected[] = {2, 4, 6};
    int actual[3];
    const int count = collect_values(&filter_it, actual, 3);

    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_values(actual, expected, count, "transform_filter_add_one_even"));

    filter_it.destroy(&filter_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test transform square chained with greater than 10 filter.
 */
static int test_transform_filter_square_gt10(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 5);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);

    // Chain: transform square → filter > 10
    ANVIterator transform_it = anv_iterator_transform(&base_it, &alloc, square_func, true);
    ANVIterator filter_it = anv_iterator_filter(&transform_it, &alloc, is_greater_than_10);

    // Expected: [1,2,3,4,5] → [1,4,9,16,25] → [16,25]
    const int expected[] = {16, 25};
    int actual[2];
    const int count = collect_values(&filter_it, actual, 2);

    ASSERT_EQ(count, 2);
    ASSERT_TRUE(verify_values(actual, expected, count, "transform_filter_square_gt10"));

    filter_it.destroy(&filter_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test transform multiply by 3 chained with divisible by 6 filter.
 */
static int test_transform_filter_multiply3_div6(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 4);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);

    // Chain: transform multiply by 3 → filter divisible by 6
    ANVIterator transform_it = anv_iterator_transform(&base_it, &alloc, multiply_by_three, true);
    ANVIterator filter_it = anv_iterator_filter(&transform_it, &alloc, is_divisible_by_six);

    // Expected: [1,2,3,4] → [3,6,9,12] → [6,12]
    const int expected[] = {6, 12};
    int actual[2];
    const int count = collect_values(&filter_it, actual, 2);

    ASSERT_EQ(count, 2);
    ASSERT_TRUE(verify_values(actual, expected, count, "transform_filter_multiply3_div6"));

    filter_it.destroy(&filter_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test range → filter → transform chain.
 */
static int test_range_filter_transform_chain(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 11, 1);

    // Chain: range → filter even → transform square
    ANVIterator filter_it = anv_iterator_filter(&range_it, &alloc, is_even);
    ANVIterator transform_it = anv_iterator_transform(&filter_it, &alloc, square_func, true);

    // Expected: [1,2,3,4,5,6,7,8,9,10] → [2,4,6,8,10] → [4,16,36,64,100]
    const int expected[] = {4, 16, 36, 64, 100};
    int actual[5];
    const int count = collect_values(&transform_it, actual, 5);

    ASSERT_EQ(count, 5);
    ASSERT_TRUE(verify_values(actual, expected, count, "range_filter_transform"));

    transform_it.destroy(&transform_it);
    return TEST_SUCCESS;
}

/**
 * Test range → transform → filter chain.
 */
static int test_range_transform_filter_chain(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range [1, 2, 3, 4, 5, 6, 7, 8]
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 9, 1);

    // Chain: range → transform add_ten → filter divisible by 3
    ANVIterator transform_it = anv_iterator_transform(&range_it, &alloc, add_ten_func, true);
    ANVIterator filter_it = anv_iterator_filter(&transform_it, &alloc, is_divisible_by_3);

    // Expected: [1,2,3,4,5,6,7,8] → [11,12,13,14,15,16,17,18] → [12,15,18]
    const int expected[] = {12, 15, 18};
    int actual[3];
    const int count = collect_values(&filter_it, actual, 3);

    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_values(actual, expected, count, "range_transform_filter"));

    filter_it.destroy(&filter_it);
    return TEST_SUCCESS;
}

/**
 * Test range → filter → transform → filter chain.
 */
static int test_range_filter_transform_filter_chain(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 11, 1);

    // Chain: range → filter odd → transform square → filter > 20
    ANVIterator filter_odd = anv_iterator_filter(&range_it, &alloc, is_odd);
    ANVIterator transform_it = anv_iterator_transform(&filter_odd, &alloc, square_func, true);
    ANVIterator filter_gt20 = anv_iterator_filter(&transform_it, &alloc, is_greater_than_20);

    // Expected: [1,2,3,4,5,6,7,8,9,10] → [1,3,5,7,9] → [1,9,25,49,81] → [25,49,81]
    const int expected[] = {25, 49, 81};
    int actual[3];
    const int count = collect_values(&filter_gt20, actual, 3);

    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_values(actual, expected, count, "range_filter_transform_filter"));

    filter_gt20.destroy(&filter_gt20);
    return TEST_SUCCESS;
}

/**
 * Test range → transform → transform → filter chain.
 */
static int test_range_transform_transform_filter_chain(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range [1, 2, 3, 4]
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 5, 1);

    // Chain: range → transform double → transform add_five → filter > 10
    ANVIterator transform_double = anv_iterator_transform(&range_it, &alloc, double_value, true);
    ANVIterator transform_add5 = anv_iterator_transform(&transform_double, &alloc, add_five, true);
    ANVIterator filter_gt10 = anv_iterator_filter(&transform_add5, &alloc, is_greater_than_10);

    // Expected: [1,2,3,4] → [2,4,6,8] → [7,9,11,13] → [11,13]
    const int expected[] = {11, 13};
    int actual[2];
    const int count = collect_values(&filter_gt10, actual, 2);

    ASSERT_EQ(count, 2);
    ASSERT_TRUE(verify_values(actual, expected, count, "range_transform_transform_filter"));

    filter_gt10.destroy(&filter_gt10);
    return TEST_SUCCESS;
}

/**
 * Test deeply nested chain with all iterator types.
 */
static int test_deep_nested_chain(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12]
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 13, 1);

    // Chain: range → filter even → transform add_one → filter divisible by 3 → transform square
    ANVIterator filter_even = anv_iterator_filter(&range_it, &alloc, is_even);
    ANVIterator transform_add1 = anv_iterator_transform(&filter_even, &alloc, add_one, true);
    ANVIterator filter_div3 = anv_iterator_filter(&transform_add1, &alloc, is_divisible_by_3);
    ANVIterator transform_square = anv_iterator_transform(&filter_div3, &alloc, square_func, true);

    // Expected: [1..12] → [2,4,6,8,10,12] → [3,5,7,9,11,13] → [3,9] → [9,81]
    const int expected[] = {9, 81};
    int actual[2];
    const int count = collect_values(&transform_square, actual, 2);

    ASSERT_EQ(count, 2);
    ASSERT_TRUE(verify_values(actual, expected, count, "deep_nested_chain"));

    transform_square.destroy(&transform_square);
    return TEST_SUCCESS;
}

/**
 * Test empty chain propagation.
 */
static int test_empty_chain_propagation(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range with only odd numbers, then filter for even
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 10, 2); // [1,3,5,7,9]

    // Chain: range (odd) → filter even → transform double
    ANVIterator filter_it = anv_iterator_filter(&range_it, &alloc, is_even);
    ANVIterator transform_it = anv_iterator_transform(&filter_it, &alloc, double_value, true);

    // Expected: [1,3,5,7,9] → [] → []
    ASSERT_FALSE(transform_it.has_next(&transform_it));
    ASSERT_NULL(transform_it.get(&transform_it));
    ASSERT_EQ(transform_it.next(&transform_it), -1);

    transform_it.destroy(&transform_it);
    return TEST_SUCCESS;
}

/**
 * Test single element chain.
 */
static int test_single_element_chain(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range with single element
    ANVIterator range_it = anv_iterator_range(&alloc, 4, 5, 1); // [4]

    // Chain: range → filter even → transform square
    ANVIterator filter_it = anv_iterator_filter(&range_it, &alloc, is_even);
    ANVIterator transform_it = anv_iterator_transform(&filter_it, &alloc, square_func, true);

    // Expected: [4] → [4] → [16]
    ASSERT_TRUE(transform_it.has_next(&transform_it));
    const int* value = transform_it.get(&transform_it);
    ASSERT_NOT_NULL(value);
    ASSERT_EQ(*value, 16);

    transform_it.next(&transform_it);
    ASSERT_FALSE(transform_it.has_next(&transform_it));
    ASSERT_NULL(transform_it.get(&transform_it));

    transform_it.destroy(&transform_it);
    return TEST_SUCCESS;
}

/**
 * Test chaining with invalid intermediate results.
 */
static int test_chain_invalid_intermediate(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range [100, 200, 300]
    ANVIterator range_it = anv_iterator_range(&alloc, 100, 301, 100);

    // Chain: range → filter (impossible condition) → transform
    ANVIterator filter_it = anv_iterator_filter(&range_it, &alloc, is_odd); // None match (all are even)
    ANVIterator transform_it = anv_iterator_transform(&filter_it, &alloc, double_value, true);

    // Should have no elements
    ASSERT_FALSE(transform_it.has_next(&transform_it));
    ASSERT_NULL(transform_it.get(&transform_it));

    transform_it.destroy(&transform_it);
    return TEST_SUCCESS;
}

/**
 * Test complex chain starting with list iterator.
 */
static int test_list_complex_chain(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 10);
    ASSERT_NOT_NULL(list);

    // Create base iterator from list
    ANVIterator base_it = anv_dll_iterator(list);

    // Chain: list → filter even → transform square → filter > 20
    ANVIterator filter_even = anv_iterator_filter(&base_it, &alloc, is_even);
    ANVIterator transform_square = anv_iterator_transform(&filter_even, &alloc, square_func, true);
    ANVIterator filter_gt20 = anv_iterator_filter(&transform_square, &alloc, is_greater_than_20);

    // Expected: [1,2,3,4,5,6,7,8,9,10] → [2,4,6,8,10] → [4,16,36,64,100] → [36,64,100]
    const int expected[] = {36, 64, 100};
    int actual[3];
    const int count = collect_values(&filter_gt20, actual, 3);

    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_values(actual, expected, count, "list_complex_chain"));

    filter_gt20.destroy(&filter_gt20);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test list iterator with multiple filters.
 */
static int test_list_multiple_filters(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 30);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);

    // Chain: list → filter even → filter divisible by 3 → filter > 10
    ANVIterator filter_even = anv_iterator_filter(&base_it, &alloc, is_even);
    ANVIterator filter_div3 = anv_iterator_filter(&filter_even, &alloc, is_divisible_by_3);
    ANVIterator filter_gt10 = anv_iterator_filter(&filter_div3, &alloc, is_greater_than_10);

    // Expected: numbers divisible by 6 and > 10: [12,18,24,30]
    const int expected[] = {12, 18, 24, 30};
    int actual[4];
    const int count = collect_values(&filter_gt10, actual, 4);

    ASSERT_EQ(count, 4);
    ASSERT_TRUE(verify_values(actual, expected, count, "list_multiple_filters"));

    filter_gt10.destroy(&filter_gt10);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test list iterator with multiple transforms.
 */
static int test_list_multiple_transforms(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 3);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);

    // Chain: list → transform double → transform add_one → transform multiply_by_three
    ANVIterator transform_double = anv_iterator_transform(&base_it, &alloc, double_value, true);
    ANVIterator transform_add1 = anv_iterator_transform(&transform_double, &alloc, add_one, true);
    ANVIterator transform_mult3 = anv_iterator_transform(&transform_add1, &alloc, multiply_by_three, true);

    // Expected: [1,2,3] → [2,4,6] → [3,5,7] → [9,15,21]
    const int expected[] = {9, 15, 21};
    int actual[3];
    const int count = collect_values(&transform_mult3, actual, 3);

    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_values(actual, expected, count, "list_multiple_transforms"));

    transform_mult3.destroy(&transform_mult3);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test memory consistency in long chains.
 */
static int test_chain_memory_consistency(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 5);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_even);
    ANVIterator transform_it = anv_iterator_transform(&filter_it, &alloc, multiply_by_three, true);

    // Get multiple references to the same value
    const int* ptr1 = transform_it.get(&transform_it);
    const int* ptr2 = transform_it.get(&transform_it);

    ASSERT_NOT_NULL(ptr1);
    ASSERT_NOT_NULL(ptr2);
    ASSERT_EQ(*ptr1, *ptr2);
    ASSERT_EQ(*ptr1, 6); // 2 * 3 = 6

    // Store value before advancing
    const int first_value = *ptr1;

    // Move to next
    transform_it.next(&transform_it);
    const int* ptr3 = transform_it.get(&transform_it);
    ASSERT_NOT_NULL(ptr3);
    ASSERT_EQ(*ptr3, 12); // 4 * 3 = 12
    ASSERT_NOT_EQ(first_value, *ptr3);

    transform_it.destroy(&transform_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test chain ownership and cleanup.
 */
static int test_chain_ownership_cleanup(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create a chain of iterators
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 5, 1);
    ANVIterator filter_it = anv_iterator_filter(&range_it, &alloc, is_even);
    ANVIterator transform_it = anv_iterator_transform(&filter_it, &alloc, double_value, true);

    // Verify chain works
    ASSERT_TRUE(transform_it.is_valid(&transform_it));
    const int* value = transform_it.get(&transform_it);
    ASSERT_NOT_NULL(value);
    ASSERT_EQ(*value, 4); // 2 * 2 = 4

    // Destroying final iterator should clean up entire chain
    transform_it.destroy(&transform_it);

    // Note: All intermediate iterators should be cleaned up automatically
    return TEST_SUCCESS;
}

/**
 * Test performance with long chains.
 */
static int test_chain_performance(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create a moderately large range
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 1001, 1);

    // Create a complex chain that actually filters significantly
    ANVIterator filter_even = anv_iterator_filter(&range_it, &alloc, is_even);
    ANVIterator transform_double = anv_iterator_transform(&filter_even, &alloc, double_value, true);
    ANVIterator filter_div6 = anv_iterator_filter(&transform_double, &alloc, is_divisible_by_six);
    ANVIterator transform_add5 = anv_iterator_transform(&filter_div6, &alloc, add_five, true);

    // Count all results to verify chain works efficiently
    int count = 0;
    while (transform_add5.has_next(&transform_add5))
    {
        const int* value = transform_add5.get(&transform_add5);
        ASSERT_NOT_NULL(value);

        // Verify conditions are met: (original was even * 2) should be divisible by 6, then +5
        // So (*value - 5) should be divisible by 6
        ASSERT_TRUE((*value - 5) % 6 == 0); // Should be divisible by 6 after subtracting 5

        count++;
        transform_add5.next(&transform_add5);
    }

    // Should have found some matching elements, but significantly fewer
    // Even numbers: 500, doubled numbers divisible by 6: every 3rd even number = ~167
    ASSERT_TRUE(count > 0);
    ASSERT_TRUE(count < 200); // Should be much less than 500 even numbers

    transform_add5.destroy(&transform_add5);
    return TEST_SUCCESS;
}

/**
 * Test helper function validation with iterator chains.
 */
static int test_chain_helper_validation(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create a simple chain
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 7, 1);
    ANVIterator filter_it = anv_iterator_filter(&range_it, &alloc, is_even);

    int values[3];
    const int count = collect_values_with_validation_chains(&filter_it, values, 3);

    // Should successfully collect all even values
    ASSERT_EQ(count, 3);

    const int expected[] = {2, 4, 6}; // Even numbers from [1,2,3,4,5,6]
    ASSERT_TRUE(verify_values(values, expected, count, "chain_helper_validation"));

    filter_it.destroy(&filter_it);
    return TEST_SUCCESS;
}

//==============================================================================
// Copy Iterator Tests
//==============================================================================

/**
 * Test basic copy iterator functionality with integer copying.
 */
static int test_copy_basic_integers(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 5);
    ASSERT_NOT_NULL(list);

    // Create base iterator
    ANVIterator base_it = anv_dll_iterator(list);
    ASSERT_TRUE(base_it.is_valid(&base_it));
    ASSERT_TRUE(base_it.has_next(&base_it));

    // Create copy iterator that copies each integer
    ANVIterator copy_it = anv_iterator_copy(&base_it, &alloc, int_copy);
    ASSERT_TRUE(copy_it.is_valid(&copy_it));
    ASSERT_TRUE(copy_it.has_next(&copy_it));

    // Expected: [1,2,3,4,5] copied to new memory locations
    const int expected[] = {1, 2, 3, 4, 5};
    int* actual[5];
    const int count = collect_values_copy(&copy_it, actual, 5);

    ASSERT_EQ(count, 5);
    ASSERT_TRUE(verify_values_copy(actual, expected, count, "basic_copy"));

    base_it.destroy(&base_it);

    // Verify that returned pointers are different from original data
    base_it = anv_dll_iterator(list);
    for (int i = 0; i < count; i++)
    {
        const int* original = base_it.get(&base_it);
        ASSERT_NOT_EQ(actual[i], original); // Different memory addresses
        ASSERT_EQ(*actual[i], *original);   // Same values
        base_it.next(&base_it);
    }

    // Iterator should be exhausted
    ASSERT_FALSE(copy_it.has_next(&copy_it));
    ASSERT_NULL(copy_it.get(&copy_it));

    // Cleanup - user must free the copied values
    free_collected_values(actual, count);
    copy_it.destroy(&copy_it);
    base_it.destroy(&base_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test copy iterator with single element.
 */
static int test_copy_single_element(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 1);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator copy_it = anv_iterator_copy(&base_it, &alloc, int_copy);

    ASSERT_TRUE(copy_it.is_valid(&copy_it));
    ASSERT_TRUE(copy_it.has_next(&copy_it));

    const int* copied_value = copy_it.get(&copy_it);
    ASSERT_NOT_NULL(copied_value);
    ASSERT_EQ(*copied_value, 1);

    // Verify it's a different memory location
    const int* original_value = list->head->data;
    ASSERT_NOT_EQ(copied_value, original_value);
    ASSERT_EQ(*copied_value, *original_value);

    copy_it.next(&copy_it);
    ASSERT_FALSE(copy_it.has_next(&copy_it));
    ASSERT_NULL(copy_it.get(&copy_it));

    // User must free the copied value
    free((void*)copied_value);
    copy_it.destroy(&copy_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test copy iterator with custom data structure (Person).
 */
static int test_copy_custom_structure(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    ASSERT_NOT_NULL(list);

    // Create test persons
    Person* p1 = create_person("Alice", 30);
    Person* p2 = create_person("Bob", 25);
    Person* p3 = create_person("Charlie", 35);

    anv_dll_push_back(list, p1);
    anv_dll_push_back(list, p2);
    anv_dll_push_back(list, p3);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator copy_it = anv_iterator_copy(&base_it, &alloc, person_copy);

    ASSERT_TRUE(copy_it.is_valid(&copy_it));

    // Collect copied persons
    Person* copied_persons[3];
    int count = 0;
    while (copy_it.has_next(&copy_it) && count < 3)
    {
        Person* copied = copy_it.get(&copy_it);
        ASSERT_NOT_NULL(copied);
        copied_persons[count++] = copied;
        copy_it.next(&copy_it);
    }

    ASSERT_EQ(count, 3);

    // Verify copied data is correct but in different memory
    ASSERT_TRUE(strcmp(copied_persons[0]->name, "Alice") == 0);
    ASSERT_EQ(copied_persons[0]->age, 30);
    ASSERT_NOT_EQ(copied_persons[0], p1);

    ASSERT_TRUE(strcmp(copied_persons[1]->name, "Bob") == 0);
    ASSERT_EQ(copied_persons[1]->age, 25);
    ASSERT_NOT_EQ(copied_persons[1], p2);

    ASSERT_TRUE(strcmp(copied_persons[2]->name, "Charlie") == 0);
    ASSERT_EQ(copied_persons[2]->age, 35);
    ASSERT_NOT_EQ(copied_persons[2], p3);

    // User must free the copied persons
    for (int i = 0; i < count; i++)
    {
        person_free(copied_persons[i]);
    }

    copy_it.destroy(&copy_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test copy iterator with empty input.
 */
static int test_copy_empty_input(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    ASSERT_NOT_NULL(list);

    // Create iterator on empty list
    ANVIterator base_it = anv_dll_iterator(list);
    ASSERT_FALSE(base_it.has_next(&base_it));

    // Create copy iterator
    ANVIterator copy_it = anv_iterator_copy(&base_it, &alloc, int_copy);
    ASSERT_TRUE(copy_it.is_valid(&copy_it));
    ASSERT_FALSE(copy_it.has_next(&copy_it));
    ASSERT_NULL(copy_it.get(&copy_it));

    // Test that next() fails appropriately
    ASSERT_EQ(copy_it.next(&copy_it), -1);

    copy_it.destroy(&copy_it);
    anv_dll_destroy(list, false);
    return TEST_SUCCESS;
}

/**
 * Test copy iterator with large dataset.
 */
static int test_copy_large_dataset(void)
{
    ANVAllocator alloc = create_int_allocator();
    #define SIZE 100

    ANVDoublyLinkedList* list = create_test_list(&alloc, SIZE);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator copy_it = anv_iterator_copy(&base_it, &alloc, int_copy);

    ASSERT_TRUE(copy_it.is_valid(&copy_it));

    int count = 0;
    int expected = 1; // First element
    int* copied_values[SIZE];

    while (copy_it.has_next(&copy_it) && count < SIZE)
    {
        int* value = copy_it.get(&copy_it);
        ASSERT_NOT_NULL(value);
        ASSERT_EQ(*value, expected);

        copied_values[count] = value;
        count++;
        expected++;
        copy_it.next(&copy_it);
    }

    ASSERT_EQ(count, SIZE);
    ASSERT_FALSE(copy_it.has_next(&copy_it));

    // Cleanup all copied values
    for (int i = 0; i < count; i++)
    {
        free(copied_values[i]);
    }

    copy_it.destroy(&copy_it);
    anv_dll_destroy(list, true);

    #undef SIZE
    return TEST_SUCCESS;
}

/**
 * Test copy iterator with invalid inputs.
 */
static int test_copy_invalid_inputs(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 1);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);

    // Test with NULL iterator
    ANVIterator invalid_it1 = anv_iterator_copy(NULL, &alloc, int_copy);
    ASSERT_FALSE(invalid_it1.is_valid(&invalid_it1));
    ASSERT_FALSE(invalid_it1.has_next(&invalid_it1));
    ASSERT_NULL(invalid_it1.get(&invalid_it1));

    // Test with NULL copy function
    ANVIterator invalid_it2 = anv_iterator_copy(&base_it, &alloc, NULL);
    ASSERT_FALSE(invalid_it2.is_valid(&invalid_it2));

    // Test with NULL allocator
    ANVIterator base_it2 = anv_dll_iterator(list);
    ANVIterator invalid_it3 = anv_iterator_copy(&base_it2, NULL, int_copy);
    ASSERT_FALSE(invalid_it3.is_valid(&invalid_it3));

    // Cleanup
    invalid_it1.destroy(&invalid_it1);
    invalid_it2.destroy(&invalid_it2);
    invalid_it3.destroy(&invalid_it3);
    base_it2.destroy(&base_it2);
    base_it.destroy(&base_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test copy iterator operations on invalid iterator.
 */
static int test_copy_operations_on_invalid(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create invalid copy iterator
    ANVIterator invalid_it = anv_iterator_copy(NULL, &alloc, int_copy);
    ASSERT_FALSE(invalid_it.is_valid(&invalid_it));

    // All operations should fail gracefully
    ASSERT_EQ(invalid_it.next(&invalid_it), -1);
    ASSERT_EQ(invalid_it.prev(&invalid_it), -1); // Copy doesn't support prev
    ASSERT_FALSE(invalid_it.has_next(&invalid_it));
    ASSERT_FALSE(invalid_it.has_prev(&invalid_it)); // Copy doesn't support has_prev
    ASSERT_NULL(invalid_it.get(&invalid_it));

    // Reset should be safe to call but ineffective
    invalid_it.reset(&invalid_it); // Should not crash

    invalid_it.destroy(&invalid_it);
    return TEST_SUCCESS;
}

/**
 * Test that get() doesn't advance, next() does advance.
 */
static int test_copy_get_next_separation(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 3);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator copy_it = anv_iterator_copy(&base_it, &alloc, int_copy);

    ASSERT_TRUE(copy_it.is_valid(&copy_it));

    // Test get without advancing
    const int* value1 = copy_it.get(&copy_it);
    ASSERT_NOT_NULL(value1);
    ASSERT_EQ(*value1, 1);

    // Get again - should return same value (cached)
    const int* value2 = copy_it.get(&copy_it);
    ASSERT_NOT_NULL(value2);
    ASSERT_EQ(value1, value2); // Should be same pointer (cached)
    ASSERT_EQ(*value2, 1);

    // Now advance
    ASSERT_EQ(copy_it.next(&copy_it), 0);
    const int* value3 = copy_it.get(&copy_it);
    ASSERT_NOT_NULL(value3);
    ASSERT_EQ(*value3, 2);         // Next value
    ASSERT_NOT_EQ(value1, value3); // Different pointers

    // User must free the copied values
    free((void*)value1);
    free((void*)value3);

    copy_it.destroy(&copy_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test next() return codes.
 */
static int test_copy_next_return_codes(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 2);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator copy_it = anv_iterator_copy(&base_it, &alloc, int_copy);

    ASSERT_TRUE(copy_it.is_valid(&copy_it));

    // Collect values to free later
    const int* v1 = copy_it.get(&copy_it);
    ASSERT_NOT_NULL(v1);

    // First next should succeed
    ASSERT_EQ(copy_it.next(&copy_it), 0);

    const int* v2 = copy_it.get(&copy_it);
    ASSERT_NOT_NULL(v2);

    // Second next should succeed
    ASSERT_EQ(copy_it.next(&copy_it), 0);

    // Third next should fail (at end)
    ASSERT_EQ(copy_it.next(&copy_it), -1);

    // Free the copied values
    free((void*)v1);
    free((void*)v2);

    copy_it.destroy(&copy_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test unsupported bidirectional operations.
 */
static int test_copy_unsupported_operations(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 3);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator copy_it = anv_iterator_copy(&base_it, &alloc, int_copy);

    ASSERT_TRUE(copy_it.is_valid(&copy_it));

    // Copy iterator should not support bidirectional operations
    ASSERT_FALSE(copy_it.has_prev(&copy_it));
    ASSERT_EQ(copy_it.prev(&copy_it), -1); // Returns -1 for unsupported

    // Reset should be safe but ineffective
    copy_it.reset(&copy_it);

    // Should still be valid after unsupported operations
    ASSERT_TRUE(copy_it.is_valid(&copy_it));

    copy_it.destroy(&copy_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test copy iterator with range iterator as input.
 */
static int test_copy_with_range_iterator(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator: [1, 2, 3, 4, 5]
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 6, 1);
    ASSERT_TRUE(range_it.is_valid(&range_it));

    // Apply copy
    ANVIterator copy_it = anv_iterator_copy(&range_it, &alloc, int_copy);
    ASSERT_TRUE(copy_it.is_valid(&copy_it));

    // Expected: [1,2,3,4,5] copied to new memory locations
    const int expected[] = {1, 2, 3, 4, 5};
    int* actual[5];
    const int count = collect_values_copy(&copy_it, actual, 5);

    ASSERT_EQ(count, 5);
    ASSERT_TRUE(verify_values_copy(actual, expected, count, "copy_range"));

    // Cleanup copied values
    free_collected_values(actual, count);
    copy_it.destroy(&copy_it);
    return TEST_SUCCESS;
}

/**
 * Test chaining range → copy.
 */
static int test_range_copy_chain(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator: [2, 4, 6, 8, 10]
    ANVIterator range_it = anv_iterator_range(&alloc, 2, 11, 2);

    // Chain: range → copy
    ANVIterator copy_it = anv_iterator_copy(&range_it, &alloc, int_copy);

    // Expected: [2,4,6,8,10] copied
    const int expected[] = {2, 4, 6, 8, 10};
    int* actual[5];
    const int count = collect_values_copy(&copy_it, actual, 5);

    ASSERT_EQ(count, 5);
    ASSERT_TRUE(verify_values_copy(actual, expected, count, "range_copy_chain"));

    free_collected_values(actual, count);
    copy_it.destroy(&copy_it);
    return TEST_SUCCESS;
}

/**
 * Test memory ownership and lifecycle.
 */
static int test_copy_memory_ownership(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 3);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator copy_it = anv_iterator_copy(&base_it, &alloc, int_copy);

    // Get and verify ownership of copied values
    const int* copied1 = copy_it.get(&copy_it);
    ASSERT_NOT_NULL(copied1);
    ASSERT_EQ(*copied1, 1);

    copy_it.next(&copy_it);
    const int* copied2 = copy_it.get(&copy_it);
    ASSERT_NOT_NULL(copied2);
    ASSERT_EQ(*copied2, 2);

    copy_it.next(&copy_it);
    const int* copied3 = copy_it.get(&copy_it);
    ASSERT_NOT_NULL(copied3);
    ASSERT_EQ(*copied3, 3);

    // Verify all pointers are different
    ASSERT_NOT_EQ(copied1, copied2);
    ASSERT_NOT_EQ(copied2, copied3);
    ASSERT_NOT_EQ(copied1, copied3);

    // Verify they're different from original data
    ANVIterator check_it = anv_dll_iterator(list);
    const int* original1 = check_it.get(&check_it);
    ASSERT_NOT_EQ(copied1, original1);
    ASSERT_EQ(*copied1, *original1);

    // User must free all copied values
    free((void*)copied1);
    free((void*)copied2);
    free((void*)copied3);

    copy_it.destroy(&copy_it);
    check_it.destroy(&check_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test that copy iterator properly manages base iterator lifecycle.
 */
static int test_copy_iterator_ownership(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 2);
    ASSERT_NOT_NULL(list);

    // Create base iterator
    ANVIterator base_it = anv_dll_iterator(list);
    ASSERT_TRUE(base_it.is_valid(&base_it));

    // Create copy iterator (takes ownership of base_it)
    ANVIterator copy_it = anv_iterator_copy(&base_it, &alloc, int_copy);
    ASSERT_TRUE(copy_it.is_valid(&copy_it));

    // Verify copy works
    const int* value = copy_it.get(&copy_it);
    ASSERT_NOT_NULL(value);
    ASSERT_EQ(*value, 1);

    // Store the copied value to free later
    const int* copied_value = value;

    // When we destroy copy iterator, it should clean up base iterator too
    copy_it.destroy(&copy_it);

    // Free the copied value
    free((void*)copied_value);

    // Note: We should not access base_it after this point as it's been destroyed

    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test memory consistency across operations.
 */
static int test_copy_memory_consistency(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 3);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator copy_it = anv_iterator_copy(&base_it, &alloc, int_copy);

    // Get multiple references to the same cached copy
    const int* ptr1 = copy_it.get(&copy_it);
    const int* ptr2 = copy_it.get(&copy_it);
    const int* ptr3 = copy_it.get(&copy_it);

    // All should point to the same memory location (cached copy)
    ASSERT_EQ(ptr1, ptr2);
    ASSERT_EQ(ptr2, ptr3);
    ASSERT_EQ(*ptr1, 1);
    ASSERT_EQ(*ptr2, 1);
    ASSERT_EQ(*ptr3, 1);

    // Store pointer to free later
    const int* first_copy = ptr1;

    // Move to next and verify new copy
    copy_it.next(&copy_it);
    const int* ptr4 = copy_it.get(&copy_it);
    ASSERT_NOT_NULL(ptr4);
    ASSERT_EQ(*ptr4, 2);

    // New copy should be different memory
    ASSERT_NOT_EQ(first_copy, ptr4);

    // User must free the copied values
    free((void*)first_copy);
    free((void*)ptr4);

    copy_it.destroy(&copy_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test copy iterator chained with filter iterator.
 */
static int test_filter_copy_chain(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 6);
    ASSERT_NOT_NULL(list);

    // Chain: list → filter even → copy
    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_even);
    ANVIterator copy_it = anv_iterator_copy(&filter_it, &alloc, int_copy);

    // Expected: [1,2,3,4,5,6] → [2,4,6] → copied [2,4,6]
    const int expected[] = {2, 4, 6};
    int* actual[3];
    const int count = collect_values_copy(&copy_it, actual, 3);

    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_values_copy(actual, expected, count, "filter_copy_chain"));

    free_collected_values(actual, count);
    copy_it.destroy(&copy_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test copy iterator chained with transform iterator.
 */
static int test_transform_copy_chain(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 4);
    ASSERT_NOT_NULL(list);

    // Chain: list → transform double → copy
    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator transform_it = anv_iterator_transform(&base_it, &alloc, double_value, true);
    ANVIterator copy_it = anv_iterator_copy(&transform_it, &alloc, int_copy);

    // Expected: [1,2,3,4] → [2,4,6,8] → copied [2,4,6,8]
    const int expected[] = {2, 4, 6, 8};
    int* actual[4];
    const int count = collect_values_copy(&copy_it, actual, 4);

    ASSERT_EQ(count, 4);
    ASSERT_TRUE(verify_values_copy(actual, expected, count, "transform_copy_chain"));

    free_collected_values(actual, count);
    copy_it.destroy(&copy_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test complex chain: range → filter → transform → copy.
 */
static int test_complex_chain_with_copy(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range [1,2,3,4,5,6,7,8,9,10]
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 11, 1);

    // Chain: range → filter even → transform square → copy
    ANVIterator filter_it = anv_iterator_filter(&range_it, &alloc, is_even);
    ANVIterator transform_it = anv_iterator_transform(&filter_it, &alloc, square_func, true);
    ANVIterator copy_it = anv_iterator_copy(&transform_it, &alloc, int_copy);

    // Expected: [1..10] → [2,4,6,8,10] → [4,16,36,64,100] → copied [4,16,36,64,100]
    const int expected[] = {4, 16, 36, 64, 100};
    int* actual[5];
    const int count = collect_values_copy(&copy_it, actual, 5);

    ASSERT_EQ(count, 5);
    ASSERT_TRUE(verify_values_copy(actual, expected, count, "complex_chain_copy"));

    free_collected_values(actual, count);
    copy_it.destroy(&copy_it);
    return TEST_SUCCESS;
}

/**
 * Test copy iterator with string data.
 */
static int test_copy_strings(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    ASSERT_NOT_NULL(list);

    // Create test strings
    char* str1 = malloc(10);
    char* str2 = malloc(10);
    char* str3 = malloc(10);
    strcpy(str1, "Hello");
    strcpy(str2, "World");
    strcpy(str3, "Test");

    anv_dll_push_back(list, str1);
    anv_dll_push_back(list, str2);
    anv_dll_push_back(list, str3);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator copy_it = anv_iterator_copy(&base_it, &alloc, string_copy);

    ASSERT_TRUE(copy_it.is_valid(&copy_it));

    // Collect copied strings
    char* copied_strings[3];
    int count = 0;
    while (copy_it.has_next(&copy_it) && count < 3)
    {
        char* copied = copy_it.get(&copy_it);
        ASSERT_NOT_NULL(copied);
        copied_strings[count++] = copied;
        copy_it.next(&copy_it);
    }

    ASSERT_EQ(count, 3);

    // Verify copied strings are correct but in different memory
    ASSERT_TRUE(strcmp(copied_strings[0], "Hello") == 0);
    ASSERT_NOT_EQ(copied_strings[0], str1);

    ASSERT_TRUE(strcmp(copied_strings[1], "World") == 0);
    ASSERT_NOT_EQ(copied_strings[1], str2);

    ASSERT_TRUE(strcmp(copied_strings[2], "Test") == 0);
    ASSERT_NOT_EQ(copied_strings[2], str3);

    // User must free the copied strings
    for (int i = 0; i < count; i++)
    {
        free(copied_strings[i]);
    }

    copy_it.destroy(&copy_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test copy iterator performance with large dataset.
 */
static int test_copy_performance(void)
{
    ANVAllocator alloc = create_int_allocator();
    #define SIZE 1000

    ANVDoublyLinkedList* list = create_test_list(&alloc, SIZE);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator copy_it = anv_iterator_copy(&base_it, &alloc, int_copy);

    ASSERT_TRUE(copy_it.is_valid(&copy_it));

    // Just verify it works end-to-end and count results
    int count = 0;
    int* copied_values[SIZE];
    while (copy_it.has_next(&copy_it) && count < SIZE)
    {
        int* value = copy_it.get(&copy_it);
        ASSERT_NOT_NULL(value);
        copied_values[count] = value;
        count++;
        copy_it.next(&copy_it);
    }

    ASSERT_EQ(count, SIZE);

    // Cleanup all copied values
    for (int i = 0; i < count; i++)
    {
        free(copied_values[i]);
    }

    copy_it.destroy(&copy_it);
    anv_dll_destroy(list, true);

    #undef SIZE
    return TEST_SUCCESS;
}

/**
 * Test helper function validation with copy iterator.
 */
static int test_copy_helper_validation(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 5);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator copy_it = anv_iterator_copy(&base_it, &alloc, int_copy);

    ASSERT_TRUE(copy_it.is_valid(&copy_it));

    int* values[5];
    const int count = collect_values_with_validation_copy(&copy_it, values, 5);

    // Should successfully collect all 5 copied values
    ASSERT_EQ(count, 5);

    const int expected[] = {1, 2, 3, 4, 5};
    ASSERT_TRUE(verify_values_copy(values, expected, count, "copy_helper_validation"));

    // Free collected values
    free_collected_values(values, count);
    copy_it.destroy(&copy_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Enumerate Iterator Tests
//==============================================================================

static int test_enumerate_basic_functionality(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator [10, 11, 12, 13, 14]
    ANVIterator range_it = anv_iterator_range(&alloc, 10, 15, 1);

    // Enumerate starting from index 0
    ANVIterator enum_it = anv_iterator_enumerate(&range_it, &alloc, 0);
    ASSERT_TRUE(enum_it.is_valid(&enum_it));

    size_t indices[10];
    int values[10];
    const int count = collect_indexed_elements(&enum_it, indices, values, 10);

    ASSERT_EQ(count, 5);

    const size_t expected_indices[] = {0, 1, 2, 3, 4};
    const int expected_values[] = {10, 11, 12, 13, 14};
    ASSERT_TRUE(verify_indexed_elements(indices, values, expected_indices, expected_values, 5, "enumerate_basic"));

    enum_it.destroy(&enum_it);
    return TEST_SUCCESS;
}

static int test_enumerate_custom_start_index(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator [1, 2, 3]
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 4, 1);

    // Enumerate starting from index 100
    ANVIterator enum_it = anv_iterator_enumerate(&range_it, &alloc, 100);
    ASSERT_TRUE(enum_it.is_valid(&enum_it));

    size_t indices[10];
    int values[10];
    const int count = collect_indexed_elements(&enum_it, indices, values, 10);

    ASSERT_EQ(count, 3);

    const size_t expected_indices[] = {100, 101, 102};
    const int expected_values[] = {1, 2, 3};
    ASSERT_TRUE(verify_indexed_elements(indices, values, expected_indices, expected_values, 3, "enumerate_custom_start"));

    enum_it.destroy(&enum_it);
    return TEST_SUCCESS;
}

static int test_enumerate_single_element(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator with single element [42]
    ANVIterator range_it = anv_iterator_range(&alloc, 42, 43, 1);

    // Enumerate starting from index 5
    ANVIterator enum_it = anv_iterator_enumerate(&range_it, &alloc, 5);
    ASSERT_TRUE(enum_it.is_valid(&enum_it));

    ASSERT_TRUE(enum_it.has_next(&enum_it));
    const ANVIndexedElement* indexed = enum_it.get(&enum_it);
    ASSERT_NOT_NULL(indexed);
    ASSERT_EQ(indexed->index, 5);
    ASSERT_NOT_NULL(indexed->element);
    ASSERT_EQ(*(const int*)indexed->element, 42);

    ASSERT_EQ(enum_it.next(&enum_it), 0);
    ASSERT_FALSE(enum_it.has_next(&enum_it));
    ASSERT_NULL(enum_it.get(&enum_it));

    enum_it.destroy(&enum_it);
    return TEST_SUCCESS;
}

static int test_enumerate_large_start_index(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator [1, 2]
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 3, 1);

    // Enumerate starting from SIZE_MAX - 1
    ANVIterator enum_it = anv_iterator_enumerate(&range_it, &alloc, SIZE_MAX - 1);
    ASSERT_TRUE(enum_it.is_valid(&enum_it));

    size_t indices[10];
    int values[10];
    const int count = collect_indexed_elements(&enum_it, indices, values, 10);

    ASSERT_EQ(count, 2);

    const size_t expected_indices[] = {SIZE_MAX - 1, SIZE_MAX};
    const int expected_values[] = {1, 2};
    ASSERT_TRUE(verify_indexed_elements(indices, values, expected_indices, expected_values, 2, "enumerate_large_start"));

    enum_it.destroy(&enum_it);
    return TEST_SUCCESS;
}

static int test_enumerate_empty_source(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create empty range iterator
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 1, 1); // Empty

    // Enumerate empty iterator
    ANVIterator enum_it = anv_iterator_enumerate(&range_it, &alloc, 0);
    ASSERT_TRUE(enum_it.is_valid(&enum_it));

    // Should have no elements
    ASSERT_FALSE(enum_it.has_next(&enum_it));
    ASSERT_NULL(enum_it.get(&enum_it));

    enum_it.destroy(&enum_it);
    return TEST_SUCCESS;
}

static int test_enumerate_invalid_parameters(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Test with NULL iterator
    const ANVIterator enum_it1 = anv_iterator_enumerate(NULL, &alloc, 0);
    ASSERT_FALSE(enum_it1.is_valid(&enum_it1));

    // Test with NULL allocator
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 4, 1);
    const ANVIterator enum_it2 = anv_iterator_enumerate(&range_it, NULL, 0);
    ASSERT_FALSE(enum_it2.is_valid(&enum_it2));
    range_it.destroy(&range_it); // Clean up since enumerate failed

    return TEST_SUCCESS;
}

static int test_enumerate_with_filter(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range 1-10, filter evens, then enumerate
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 11, 1);             // [1,2,3,4,5,6,7,8,9,10]
    ANVIterator filter_it = anv_iterator_filter(&range_it, &alloc, is_even); // [2,4,6,8,10]
    ANVIterator enum_it = anv_iterator_enumerate(&filter_it, &alloc, 0);

    ASSERT_TRUE(enum_it.is_valid(&enum_it));

    size_t indices[10];
    int values[10];
    const int count = collect_indexed_elements(&enum_it, indices, values, 10);

    ASSERT_EQ(count, 5);

    const size_t expected_indices[] = {0, 1, 2, 3, 4};
    const int expected_values[] = {2, 4, 6, 8, 10};
    ASSERT_TRUE(verify_indexed_elements(indices, values, expected_indices, expected_values, 5, "enumerate_with_filter"));

    enum_it.destroy(&enum_it);
    return TEST_SUCCESS;
}

static int test_enumerate_with_take(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range 1-10, take first 3, then enumerate starting from 10
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 11, 1);   // [1,2,3,4,5,6,7,8,9,10]
    ANVIterator take_it = anv_iterator_take(&range_it, &alloc, 3); // [1,2,3]
    ANVIterator enum_it = anv_iterator_enumerate(&take_it, &alloc, 10);

    ASSERT_TRUE(enum_it.is_valid(&enum_it));

    size_t indices[10];
    int values[10];
    const int count = collect_indexed_elements(&enum_it, indices, values, 10);

    ASSERT_EQ(count, 3);

    const size_t expected_indices[] = {10, 11, 12};
    const int expected_values[] = {1, 2, 3};
    ASSERT_TRUE(verify_indexed_elements(indices, values, expected_indices, expected_values, 3, "enumerate_with_take"));

    enum_it.destroy(&enum_it);
    return TEST_SUCCESS;
}

static int test_enumerate_with_skip(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range 1-7, skip first 2, then enumerate starting from 0
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 8, 1);    // [1,2,3,4,5,6,7]
    ANVIterator skip_it = anv_iterator_skip(&range_it, &alloc, 2); // [3,4,5,6,7]
    ANVIterator enum_it = anv_iterator_enumerate(&skip_it, &alloc, 0);

    ASSERT_TRUE(enum_it.is_valid(&enum_it));

    size_t indices[10];
    int values[10];
    const int count = collect_indexed_elements(&enum_it, indices, values, 10);

    ASSERT_EQ(count, 5);

    const size_t expected_indices[] = {0, 1, 2, 3, 4};
    const int expected_values[] = {3, 4, 5, 6, 7};
    ASSERT_TRUE(verify_indexed_elements(indices, values, expected_indices, expected_values, 5, "enumerate_with_skip"));

    enum_it.destroy(&enum_it);
    return TEST_SUCCESS;
}

static int test_enumerate_chained(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range 1-10, filter odds, enumerate, then take first 2
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 11, 1);            // [1,2,3,4,5,6,7,8,9,10]
    ANVIterator filter_it = anv_iterator_filter(&range_it, &alloc, is_odd); // [1,3,5,7,9]
    ANVIterator enum_it = anv_iterator_enumerate(&filter_it, &alloc, 50);   // [(50,1),(51,3),(52,5),(53,7),(54,9)]
    ANVIterator take_it = anv_iterator_take(&enum_it, &alloc, 2);           // [(50,1),(51,3)]

    ASSERT_TRUE(take_it.is_valid(&take_it));

    // Since take operates on ANVIndexedElement objects, we need to handle this differently
    int count = 0;
    while (take_it.has_next(&take_it) && count < 2)
    {
        const ANVIndexedElement* indexed = take_it.get(&take_it);
        ASSERT_NOT_NULL(indexed);

        if (count == 0)
        {
            ASSERT_EQ(indexed->index, 50);
            ASSERT_EQ(*(const int*)indexed->element, 1);
        }
        else if (count == 1)
        {
            ASSERT_EQ(indexed->index, 51);
            ASSERT_EQ(*(const int*)indexed->element, 3);
        }

        count++;
        take_it.next(&take_it);
    }

    ASSERT_EQ(count, 2);
    ASSERT_FALSE(take_it.has_next(&take_it));

    take_it.destroy(&take_it);
    return TEST_SUCCESS;
}

static int test_enumerate_arraylist(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create ArrayList with some data
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);

    // Add values 100, 200, 300
    for (int i = 1; i <= 3; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i * 100;
        anv_arraylist_push_back(list, val);
    }

    // Create iterator and enumerate starting from index 5
    ANVIterator array_iter = anv_arraylist_iterator(list);
    ANVIterator enum_it = anv_iterator_enumerate(&array_iter, &alloc, 5);

    ASSERT_TRUE(enum_it.is_valid(&enum_it));

    size_t indices[10];
    int values[10];
    const int count = collect_indexed_elements(&enum_it, indices, values, 10);

    ASSERT_EQ(count, 3);

    const size_t expected_indices[] = {5, 6, 7};
    const int expected_values[] = {100, 200, 300};
    ASSERT_TRUE(verify_indexed_elements(indices, values, expected_indices, expected_values, 3, "enumerate_arraylist"));

    enum_it.destroy(&enum_it);
    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_enumerate_dll(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create DoublyLinkedList with some data
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);

    // Add values 5, 10, 15, 20
    for (int i = 1; i <= 4; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i * 5;
        anv_dll_push_back(list, val);
    }

    // Create iterator and enumerate starting from index 0
    ANVIterator dll_iter = anv_dll_iterator(list);
    ANVIterator enum_it = anv_iterator_enumerate(&dll_iter, &alloc, 0);

    ASSERT_TRUE(enum_it.is_valid(&enum_it));

    size_t indices[10];
    int values[10];
    const int count = collect_indexed_elements(&enum_it, indices, values, 10);

    ASSERT_EQ(count, 4);

    const size_t expected_indices[] = {0, 1, 2, 3};
    const int expected_values[] = {5, 10, 15, 20};
    ASSERT_TRUE(verify_indexed_elements(indices, values, expected_indices, expected_values, 4, "enumerate_dll"));

    enum_it.destroy(&enum_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_enumerate_queue_with_stack(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create Queue with FIFO behavior
    ANVQueue* queue = anv_queue_create(&alloc);

    // Add values 1, 2, 3 (will iterate as 1, 2, 3)
    for (int i = 1; i <= 3; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_queue_enqueue(queue, val);
    }

    // Enumerate queue starting from index 10
    ANVIterator queue_iter = anv_queue_iterator(queue);
    ANVIterator enum_it = anv_iterator_enumerate(&queue_iter, &alloc, 10);

    ASSERT_TRUE(enum_it.is_valid(&enum_it));

    size_t indices[10];
    int values[10];
    const int count = collect_indexed_elements(&enum_it, indices, values, 10);

    ASSERT_EQ(count, 3);

    const size_t expected_indices[] = {10, 11, 12};
    const int expected_values[] = {1, 2, 3}; // FIFO order
    ASSERT_TRUE(verify_indexed_elements(indices, values, expected_indices, expected_values, 3, "enumerate_queue"));

    enum_it.destroy(&enum_it);
    anv_queue_destroy(queue, true);
    return TEST_SUCCESS;
}

static int test_enumerate_complex_composition(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Complex pipeline: Range -> Filter -> Take -> Enumerate
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 20, 1);                       // [1..19]
    ANVIterator filter_it = anv_iterator_filter(&range_it, &alloc, is_divisible_by_3); // [3,6,9,12,15,18]
    ANVIterator take_it = anv_iterator_take(&filter_it, &alloc, 4);                    // [3,6,9,12]
    ANVIterator enum_it = anv_iterator_enumerate(&take_it, &alloc, 100);               // [(100,3),(101,6),(102,9),(103,12)]

    ASSERT_TRUE(enum_it.is_valid(&enum_it));

    size_t indices[10];
    int values[10];
    const int count = collect_indexed_elements(&enum_it, indices, values, 10);

    ASSERT_EQ(count, 4);

    const size_t expected_indices[] = {100, 101, 102, 103};
    const int expected_values[] = {3, 6, 9, 12};
    ASSERT_TRUE(verify_indexed_elements(indices, values, expected_indices, expected_values, 4, "enumerate_complex"));

    enum_it.destroy(&enum_it);
    return TEST_SUCCESS;
}

static int test_enumerate_iteration_state(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create enumerate iterator and test step-by-step iteration
    ANVIterator range_it = anv_iterator_range(&alloc, 50, 53, 1); // [50,51,52]
    ANVIterator enum_it = anv_iterator_enumerate(&range_it, &alloc, 20);

    // Test step-by-step iteration
    ASSERT_TRUE(enum_it.has_next(&enum_it));
    const ANVIndexedElement* indexed1 = enum_it.get(&enum_it);
    ASSERT_NOT_NULL(indexed1);
    ASSERT_EQ(indexed1->index, 20);
    ASSERT_EQ(*(const int*)indexed1->element, 50);

    ASSERT_EQ(enum_it.next(&enum_it), 0);
    ASSERT_TRUE(enum_it.has_next(&enum_it));

    const ANVIndexedElement* indexed2 = enum_it.get(&enum_it);
    ASSERT_NOT_NULL(indexed2);
    ASSERT_EQ(indexed2->index, 21);
    ASSERT_EQ(*(const int*)indexed2->element, 51);

    ASSERT_EQ(enum_it.next(&enum_it), 0);
    ASSERT_TRUE(enum_it.has_next(&enum_it));

    const ANVIndexedElement* indexed3 = enum_it.get(&enum_it);
    ASSERT_NOT_NULL(indexed3);
    ASSERT_EQ(indexed3->index, 22);
    ASSERT_EQ(*(const int*)indexed3->element, 52);

    ASSERT_EQ(enum_it.next(&enum_it), 0);
    ASSERT_FALSE(enum_it.has_next(&enum_it));
    ASSERT_NULL(enum_it.get(&enum_it));
    ASSERT_EQ(enum_it.next(&enum_it), -1);

    enum_it.destroy(&enum_it);
    return TEST_SUCCESS;
}

static int test_enumerate_unsupported_operations(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create enumerate iterator
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 4, 1);
    ANVIterator enum_it = anv_iterator_enumerate(&range_it, &alloc, 0);

    // Test unsupported operations
    ASSERT_FALSE(enum_it.has_prev(&enum_it));
    ASSERT_EQ(enum_it.prev(&enum_it), -1);

    // Reset should be no-op (doesn't crash)
    enum_it.reset(&enum_it);

    enum_it.destroy(&enum_it);
    return TEST_SUCCESS;
}

static int test_enumerate_element_consistency(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Test that the same ANVIndexedElement pointer is returned for multiple get() calls
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 3, 1); // [1,2]
    ANVIterator enum_it = anv_iterator_enumerate(&range_it, &alloc, 0);

    ASSERT_TRUE(enum_it.has_next(&enum_it));

    const ANVIndexedElement* indexed1 = enum_it.get(&enum_it);
    const ANVIndexedElement* indexed2 = enum_it.get(&enum_it);

    // Should return same pointer (cached indexed element)
    ASSERT_EQ(indexed1, indexed2);
    ASSERT_EQ(indexed1->index, 0);
    ASSERT_EQ(*(const int*)indexed1->element, 1);

    // After next(), should get different values but could be same pointer
    enum_it.next(&enum_it);
    const ANVIndexedElement* indexed3 = enum_it.get(&enum_it);
    ASSERT_EQ(indexed3->index, 1);
    ASSERT_EQ(*(const int*)indexed3->element, 2);

    enum_it.destroy(&enum_it);
    return TEST_SUCCESS;
}

static int test_enumerate_index_overflow_behavior(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Test behavior near SIZE_MAX (this tests the edge case of index overflow)
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 3, 1); // [1,2]
    ANVIterator enum_it = anv_iterator_enumerate(&range_it, &alloc, SIZE_MAX);

    ASSERT_TRUE(enum_it.is_valid(&enum_it));

    // First element should have index SIZE_MAX
    ASSERT_TRUE(enum_it.has_next(&enum_it));
    const ANVIndexedElement* indexed1 = enum_it.get(&enum_it);
    ASSERT_NOT_NULL(indexed1);
    ASSERT_EQ(indexed1->index, SIZE_MAX);
    ASSERT_EQ(*(const int*)indexed1->element, 1);

    // Second element should wrap around to 0 (or whatever SIZE_MAX + 1 becomes)
    enum_it.next(&enum_it);
    ASSERT_TRUE(enum_it.has_next(&enum_it));
    const ANVIndexedElement* indexed2 = enum_it.get(&enum_it);
    ASSERT_NOT_NULL(indexed2);
    // Index should be SIZE_MAX + 1, which wraps to 0 for size_t
    ASSERT_EQ(indexed2->index, 0);
    ASSERT_EQ(*(const int*)indexed2->element, 2);

    enum_it.destroy(&enum_it);
    return TEST_SUCCESS;
}

//==============================================================================
// Filter Iterator Tests
//==============================================================================

/**
 * Test basic filter iterator functionality with even numbers.
 */
static int test_filter_basic_even(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 10);
    ASSERT_NOT_NULL(list);

    // Create base iterator
    ANVIterator base_it = anv_dll_iterator(list);
    ASSERT_TRUE(base_it.is_valid(&base_it));

    // Create filter iterator for even numbers
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_even);
    ASSERT_TRUE(filter_it.is_valid(&filter_it));
    ASSERT_NOT_NULL(filter_it.data_state);

    // Expected: [1,2,3,4,5,6,7,8,9,10] -> [2,4,6,8,10]
    const int expected[] = {2, 4, 6, 8, 10};
    int actual[5];
    const int count = collect_values(&filter_it, actual, 5);

    ASSERT_EQ(count, 5);
    ASSERT_TRUE(verify_values(actual, expected, count, "basic_even"));

    // Iterator should be exhausted
    ASSERT_FALSE(filter_it.has_next(&filter_it));
    ASSERT_NULL(filter_it.get(&filter_it));

    // Cleanup
    filter_it.destroy(&filter_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test filter iterator with odd numbers.
 */
static int test_filter_odd(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 7);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_odd);

    ASSERT_TRUE(filter_it.is_valid(&filter_it));

    // Expected: [1,2,3,4,5,6,7] -> [1,3,5,7]
    const int expected[] = {1, 3, 5, 7};
    int actual[4];
    const int count = collect_values(&filter_it, actual, 4);

    ASSERT_EQ(count, 4);
    ASSERT_TRUE(verify_values(actual, expected, count, "filter_odd"));

    filter_it.destroy(&filter_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test filter iterator with greater than 5 predicate.
 */
static int test_filter_greater_than_five(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 8);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_greater_than_five);

    ASSERT_TRUE(filter_it.is_valid(&filter_it));

    // Expected: [1,2,3,4,5,6,7,8] -> [6,7,8]
    const int expected[] = {6, 7, 8};
    int actual[3];
    const int count = collect_values(&filter_it, actual, 3);

    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_values(actual, expected, count, "filter_greater_than_five"));

    filter_it.destroy(&filter_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test filter iterator with divisible by 3 predicate.
 */
static int test_filter_divisible_by_3(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 12);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_divisible_by_3);

    ASSERT_TRUE(filter_it.is_valid(&filter_it));

    // Expected: [1,2,3,4,5,6,7,8,9,10,11,12] -> [3,6,9,12]
    const int expected[] = {3, 6, 9, 12};
    int actual[4];
    const int count = collect_values(&filter_it, actual, 4);

    ASSERT_EQ(count, 4);
    ASSERT_TRUE(verify_values(actual, expected, count, "filter_divisible_by_3"));

    filter_it.destroy(&filter_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test filter iterator with empty input.
 */
static int test_filter_empty_input(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    ASSERT_NOT_NULL(list);

    // Create iterator on empty list
    ANVIterator base_it = anv_dll_iterator(list);
    ASSERT_FALSE(base_it.has_next(&base_it));

    // Create filter iterator
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_even);
    ASSERT_TRUE(filter_it.is_valid(&filter_it));
    ASSERT_FALSE(filter_it.has_next(&filter_it));
    ASSERT_NULL(filter_it.get(&filter_it));

    // Test that next() fails appropriately
    ASSERT_EQ(filter_it.next(&filter_it), -1);

    filter_it.destroy(&filter_it);
    anv_dll_destroy(list, false);
    return TEST_SUCCESS;
}

/**
 * Test filter iterator with no matching elements.
 */
static int test_filter_no_matches(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create list with only odd numbers: [1,3,5,7,9]
    const int odd_values[] = {1, 3, 5, 7, 9};
    ANVDoublyLinkedList* list = create_list_with_values(&alloc, odd_values, 5);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_even);

    ASSERT_TRUE(filter_it.is_valid(&filter_it));

    // Should find no even numbers
    ASSERT_FALSE(filter_it.has_next(&filter_it));
    ASSERT_NULL(filter_it.get(&filter_it));

    // Test that next() fails appropriately
    ASSERT_EQ(filter_it.next(&filter_it), -1);

    filter_it.destroy(&filter_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test filter iterator with all matching elements.
 */
static int test_filter_all_matches(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create list with only even numbers: [2,4,6,8,10]
    const int even_values[] = {2, 4, 6, 8, 10};
    ANVDoublyLinkedList* list = create_list_with_values(&alloc, even_values, 5);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_even);

    ASSERT_TRUE(filter_it.is_valid(&filter_it));

    // Should find all 5 even numbers
    const int expected[] = {2, 4, 6, 8, 10};
    int actual[5];
    const int count = collect_values(&filter_it, actual, 5);

    ASSERT_EQ(count, 5);
    ASSERT_TRUE(verify_values(actual, expected, count, "filter_all_matches"));

    filter_it.destroy(&filter_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test filter iterator with single element.
 */
static int test_filter_single_element(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 1);
    ASSERT_NOT_NULL(list);

    // Test with single odd element (should be filtered out)
    ANVIterator base_it1 = anv_dll_iterator(list);
    ANVIterator filter_it1 = anv_iterator_filter(&base_it1, &alloc, is_even);

    ASSERT_TRUE(filter_it1.is_valid(&filter_it1));
    ASSERT_FALSE(filter_it1.has_next(&filter_it1));
    ASSERT_NULL(filter_it1.get(&filter_it1));

    filter_it1.destroy(&filter_it1);

    // Test with single even element (should pass through)
    const int even_value[] = {4};
    ANVDoublyLinkedList* even_list = create_list_with_values(&alloc, even_value, 1);
    ASSERT_NOT_NULL(even_list);

    ANVIterator base_it2 = anv_dll_iterator(even_list);
    ANVIterator filter_it2 = anv_iterator_filter(&base_it2, &alloc, is_even);

    ASSERT_TRUE(filter_it2.is_valid(&filter_it2));
    ASSERT_TRUE(filter_it2.has_next(&filter_it2));

    const int* value = filter_it2.get(&filter_it2);
    ASSERT_NOT_NULL(value);
    ASSERT_EQ(*value, 4);

    filter_it2.next(&filter_it2);
    ASSERT_FALSE(filter_it2.has_next(&filter_it2));
    ASSERT_NULL(filter_it2.get(&filter_it2));

    filter_it2.destroy(&filter_it2);
    anv_dll_destroy(list, true);
    anv_dll_destroy(even_list, true);
    return TEST_SUCCESS;
}

/**
 * Test filter iterator with invalid inputs.
 */
static int test_filter_invalid_inputs(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 1);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);

    // Test with NULL iterator
    ANVIterator invalid_it1 = anv_iterator_filter(NULL, &alloc, is_even);
    ASSERT_FALSE(invalid_it1.is_valid(&invalid_it1));
    ASSERT_FALSE(invalid_it1.has_next(&invalid_it1));
    ASSERT_NULL(invalid_it1.get(&invalid_it1));

    // Test with NULL filter function
    ANVIterator invalid_it2 = anv_iterator_filter(&base_it, &alloc, NULL);
    ASSERT_FALSE(invalid_it2.is_valid(&invalid_it2));

    // Test with NULL allocator
    ANVIterator base_it2 = anv_dll_iterator(list);
    ANVIterator invalid_it3 = anv_iterator_filter(&base_it2, NULL, is_even);
    ASSERT_FALSE(invalid_it3.is_valid(&invalid_it3));

    // Cleanup
    invalid_it1.destroy(&invalid_it1);
    invalid_it2.destroy(&invalid_it2);
    invalid_it3.destroy(&invalid_it3);
    base_it2.destroy(&base_it2);
    base_it.destroy(&base_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test filter iterator operations on invalid iterator.
 */
static int test_filter_operations_on_invalid(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create invalid filter iterator
    ANVIterator invalid_it = anv_iterator_filter(NULL, &alloc, is_even);
    ASSERT_FALSE(invalid_it.is_valid(&invalid_it));

    // All operations should fail gracefully
    ASSERT_EQ(invalid_it.next(&invalid_it), -1);
    ASSERT_EQ(invalid_it.prev(&invalid_it), -1); // Filter doesn't support prev
    ASSERT_FALSE(invalid_it.has_next(&invalid_it));
    ASSERT_FALSE(invalid_it.has_prev(&invalid_it)); // Filter doesn't support has_prev
    ASSERT_NULL(invalid_it.get(&invalid_it));

    // Reset should be safe to call but ineffective
    invalid_it.reset(&invalid_it); // Should not crash

    invalid_it.destroy(&invalid_it);
    return TEST_SUCCESS;
}

/**
 * Test that get() doesn't advance, next() does advance.
 */
static int test_filter_get_next_separation(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 6);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_even);

    ASSERT_TRUE(filter_it.is_valid(&filter_it));

    // Test get without advancing - should return first even number (2)
    const int* value1 = filter_it.get(&filter_it);
    ASSERT_NOT_NULL(value1);
    ASSERT_EQ(*value1, 2);

    // Get again - should return same value
    const int* value2 = filter_it.get(&filter_it);
    ASSERT_NOT_NULL(value2);
    ASSERT_EQ(*value2, 2);

    // Store value before advancing to avoid pointer reuse issues
    const int first_value = *value1;

    // Now advance
    ASSERT_EQ(filter_it.next(&filter_it), 0);
    const int* value3 = filter_it.get(&filter_it);
    ASSERT_NOT_NULL(value3);
    ASSERT_EQ(*value3, 4); // Next even number
    ASSERT_NOT_EQ(first_value, *value3);

    filter_it.destroy(&filter_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test next() return codes.
 */
static int test_filter_next_return_codes(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create list with only 2 even numbers: [2,4]
    const int even_values[] = {2, 4};
    ANVDoublyLinkedList* list = create_list_with_values(&alloc, even_values, 2);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_even);

    ASSERT_TRUE(filter_it.is_valid(&filter_it));

    // First next should succeed
    ASSERT_EQ(filter_it.next(&filter_it), 0);

    // Second next should succeed
    ASSERT_EQ(filter_it.next(&filter_it), 0);

    // Third next should fail (at end)
    ASSERT_EQ(filter_it.next(&filter_it), -1);

    filter_it.destroy(&filter_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test unsupported bidirectional operations.
 */
static int test_filter_unsupported_operations(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 3);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_even);

    ASSERT_TRUE(filter_it.is_valid(&filter_it));

    // Filter iterator should not support bidirectional operations
    ASSERT_FALSE(filter_it.has_prev(&filter_it));
    ASSERT_EQ(filter_it.prev(&filter_it), -1); // Returns -1 for unsupported

    // Reset should be safe but ineffective
    filter_it.reset(&filter_it);

    // Should still be valid after unsupported operations
    ASSERT_TRUE(filter_it.is_valid(&filter_it));

    filter_it.destroy(&filter_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test chaining multiple filters: even AND divisible by 3.
 */
static int test_multiple_filter_chain_even_div3(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 20);
    ASSERT_NOT_NULL(list);

    // Create base iterator
    ANVIterator base_it = anv_dll_iterator(list);

    // Chain filters: even AND divisible by 3 (i.e., divisible by 6)
    ANVIterator filter_even = anv_iterator_filter(&base_it, &alloc, is_even);
    ANVIterator filter_div3 = anv_iterator_filter(&filter_even, &alloc, is_divisible_by_3);

    ASSERT_TRUE(filter_div3.is_valid(&filter_div3));

    // Expected: [1..20] -> [2,4,6,8,10,12,14,16,18,20] -> [6,12,18]
    const int expected[] = {6, 12, 18};
    int actual[3];
    const int count = collect_values(&filter_div3, actual, 3);

    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_values(actual, expected, count, "multiple_filter_chain_even_div3"));

    filter_div3.destroy(&filter_div3);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test chaining multiple filters: divisible by 4 AND greater than 10.
 */
static int test_multiple_filter_chain_div4_gt10(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 25);
    ASSERT_NOT_NULL(list);

    // Create base iterator
    ANVIterator base_it = anv_dll_iterator(list);

    // Chain filters: divisible by 4 AND greater than 10
    ANVIterator filter_div4 = anv_iterator_filter(&base_it, &alloc, is_divisible_by_4);
    ANVIterator filter_gt10 = anv_iterator_filter(&filter_div4, &alloc, is_greater_than_10);

    ASSERT_TRUE(filter_gt10.is_valid(&filter_gt10));

    // Expected: [1..25] -> [4,8,12,16,20,24] -> [12,16,20,24]
    const int expected[] = {12, 16, 20, 24};
    int actual[4];
    const int count = collect_values(&filter_gt10, actual, 4);

    ASSERT_EQ(count, 4);
    ASSERT_TRUE(verify_values(actual, expected, count, "multiple_filter_chain_div4_gt10"));

    filter_gt10.destroy(&filter_gt10);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test triple filter chaining: odd AND greater than 5 AND divisible by 3.
 */
static int test_triple_filter_chain(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 30);
    ASSERT_NOT_NULL(list);

    // Create base iterator
    ANVIterator base_it = anv_dll_iterator(list);

    // Chain three filters: odd AND greater than 5 AND divisible by 3
    ANVIterator filter_odd = anv_iterator_filter(&base_it, &alloc, is_odd);
    ANVIterator filter_gt5 = anv_iterator_filter(&filter_odd, &alloc, is_greater_than_five);
    ANVIterator filter_div3 = anv_iterator_filter(&filter_gt5, &alloc, is_divisible_by_3);

    ASSERT_TRUE(filter_div3.is_valid(&filter_div3));

    // Expected: [1..30] -> [1,3,5,7,9,11,13,15,17,19,21,23,25,27,29]
    //                   -> [7,9,11,13,15,17,19,21,23,25,27,29]
    //                   -> [9,15,21,27]
    const int expected[] = {9, 15, 21, 27};
    int actual[4];
    const int count = collect_values(&filter_div3, actual, 4);

    ASSERT_EQ(count, 4);
    ASSERT_TRUE(verify_values(actual, expected, count, "triple_filter_chain"));

    filter_div3.destroy(&filter_div3);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test filter chain with no final matches.
 */
static int test_filter_chain_no_matches(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 10);
    ASSERT_NOT_NULL(list);

    // Create base iterator
    ANVIterator base_it = anv_dll_iterator(list);

    // Chain filters: even AND greater than 20 (no matches in [1..10])
    ANVIterator filter_even = anv_iterator_filter(&base_it, &alloc, is_even);
    ANVIterator filter_gt20 = anv_iterator_filter(&filter_even, &alloc, is_greater_than_20);

    ASSERT_TRUE(filter_gt20.is_valid(&filter_gt20));

    // Should find no matching elements
    ASSERT_FALSE(filter_gt20.has_next(&filter_gt20));
    ASSERT_NULL(filter_gt20.get(&filter_gt20));
    ASSERT_EQ(filter_gt20.next(&filter_gt20), -1);

    filter_gt20.destroy(&filter_gt20);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test filter chain with single final match.
 */
static int test_filter_chain_single_match(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 15);
    ASSERT_NOT_NULL(list);

    // Create base iterator
    ANVIterator base_it = anv_dll_iterator(list);

    // Chain filters: divisible by 6 AND greater than 10 (only 12 matches in [1..15])
    ANVIterator filter_div6 = anv_iterator_filter(&base_it, &alloc, is_divisible_by_six);
    ANVIterator filter_gt10 = anv_iterator_filter(&filter_div6, &alloc, is_greater_than_10);

    ASSERT_TRUE(filter_gt10.is_valid(&filter_gt10));

    // Expected: [1..15] -> [6,12] -> [12]
    ASSERT_TRUE(filter_gt10.has_next(&filter_gt10));
    const int* value = filter_gt10.get(&filter_gt10);
    ASSERT_NOT_NULL(value);
    ASSERT_EQ(*value, 12);

    // Advance to end
    filter_gt10.next(&filter_gt10);
    ASSERT_FALSE(filter_gt10.has_next(&filter_gt10));
    ASSERT_NULL(filter_gt10.get(&filter_gt10));

    filter_gt10.destroy(&filter_gt10);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test memory consistency across operations.
 */
static int test_filter_memory_consistency(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 4);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_even);

    // Get multiple references to the same value
    const int* ptr1 = filter_it.get(&filter_it);
    const int* ptr2 = filter_it.get(&filter_it);
    const int* ptr3 = filter_it.get(&filter_it);

    // All should return the same value (first even number: 2)
    ASSERT_NOT_NULL(ptr1);
    ASSERT_NOT_NULL(ptr2);
    ASSERT_NOT_NULL(ptr3);
    ASSERT_EQ(*ptr1, 2);
    ASSERT_EQ(*ptr2, 2);
    ASSERT_EQ(*ptr3, 2);

    // Store the value before moving to next
    const int first_value = *ptr1;

    // Move to next and verify new value
    filter_it.next(&filter_it);
    const int* ptr4 = filter_it.get(&filter_it);
    ASSERT_NOT_NULL(ptr4);
    ASSERT_EQ(*ptr4, 4); // Next even number

    // Values should be different
    ASSERT_NOT_EQ(first_value, *ptr4);

    filter_it.destroy(&filter_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test that filter iterator properly manages base iterator lifecycle.
 */
static int test_filter_iterator_ownership(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 2);
    ASSERT_NOT_NULL(list);

    // Create base iterator
    ANVIterator base_it = anv_dll_iterator(list);
    ASSERT_TRUE(base_it.is_valid(&base_it));

    // Create filter iterator (takes ownership of base_it)
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_even);
    ASSERT_TRUE(filter_it.is_valid(&filter_it));

    // Verify filter works
    const int* value = filter_it.get(&filter_it);
    ASSERT_NOT_NULL(value);
    ASSERT_EQ(*value, 2); // First even number

    // When we destroy filter iterator, it should clean up base iterator too
    filter_it.destroy(&filter_it);

    // Note: We should not access base_it after this point as it's been destroyed

    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test memory management in filter chains.
 */
static int test_filter_chain_memory_management(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 20);
    ASSERT_NOT_NULL(list);

    // Create base iterator
    ANVIterator base_it = anv_dll_iterator(list);

    // Create a chain of filters
    ANVIterator filter1 = anv_iterator_filter(&base_it, &alloc, is_even);
    ANVIterator filter2 = anv_iterator_filter(&filter1, &alloc, is_divisible_by_3);
    ANVIterator filter3 = anv_iterator_filter(&filter2, &alloc, is_greater_than_five);

    ASSERT_TRUE(filter3.is_valid(&filter3));

    // Use the chain
    int count = 0;
    while (filter3.has_next(&filter3))
    {
        const int* value = filter3.get(&filter3);
        ASSERT_NOT_NULL(value);
        count++;
        filter3.next(&filter3);
    }

    // Should have found some matching elements
    ASSERT_TRUE(count > 0);

    // Destroying the final filter should clean up the entire chain
    filter3.destroy(&filter3);

    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test filter iterator with large dataset.
 */
static int test_filter_large_dataset(void)
{
    ANVAllocator alloc = create_int_allocator();
    const int SIZE = 1000;

    ANVDoublyLinkedList* list = create_test_list(&alloc, SIZE);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_even);

    ASSERT_TRUE(filter_it.is_valid(&filter_it));

    int count = 0;
    int expected = 2; // First even number

    while (filter_it.has_next(&filter_it))
    {
        const int* value = filter_it.get(&filter_it);
        ASSERT_NOT_NULL(value);
        ASSERT_EQ(*value, expected);

        count++;
        expected += 2; // Next expected even number
        filter_it.next(&filter_it);
    }

    ASSERT_EQ(count, SIZE / 2); // Should have SIZE/2 even numbers
    ASSERT_FALSE(filter_it.has_next(&filter_it));

    filter_it.destroy(&filter_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test complex filter chaining performance.
 */
static int test_filter_complex_chaining(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 100);
    ASSERT_NOT_NULL(list);

    // Create a complex chain: even -> divisible by 3 -> greater than 10 -> divisible by 4
    ANVIterator it1 = anv_dll_iterator(list);
    ANVIterator it2 = anv_iterator_filter(&it1, &alloc, is_even);
    ANVIterator it3 = anv_iterator_filter(&it2, &alloc, is_divisible_by_3);
    ANVIterator it4 = anv_iterator_filter(&it3, &alloc, is_greater_than_10);
    ANVIterator final_it = anv_iterator_filter(&it4, &alloc, is_divisible_by_4);

    ASSERT_TRUE(final_it.is_valid(&final_it));

    // Just verify it works end-to-end and count results
    int count = 0;
    while (final_it.has_next(&final_it))
    {
        const int* value = final_it.get(&final_it);
        ASSERT_NOT_NULL(value);

        // Verify all conditions are met
        ASSERT_TRUE(*value % 2 == 0); // even
        ASSERT_TRUE(*value % 3 == 0); // divisible by 3
        ASSERT_TRUE(*value > 10);     // greater than 10
        ASSERT_TRUE(*value % 4 == 0); // divisible by 4

        count++;
        final_it.next(&final_it);
    }

    // We should get some results (numbers divisible by 12 and > 10)
    ASSERT_TRUE(count > 0);

    final_it.destroy(&final_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test helper function validation with filter iterator.
 */
static int test_filter_helper_validation(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 10);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator filter_it = anv_iterator_filter(&base_it, &alloc, is_even);

    ASSERT_TRUE(filter_it.is_valid(&filter_it));

    int values[5];
    const int count = collect_values_with_validation(&filter_it, values, 5);

    // Should successfully collect all 5 even values
    ASSERT_EQ(count, 5);

    const int expected[] = {2, 4, 6, 8, 10};
    ASSERT_TRUE(verify_values(values, expected, count, "helper_validation"));

    filter_it.destroy(&filter_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Range Iterator Tests
//==============================================================================

/**
 * Test basic forward iteration with step 1.
 */
static int test_range_positive_step(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(&alloc, 0, 5, 1);

    ASSERT_TRUE(it.is_valid(&it));

    const int expected[] = {0, 1, 2, 3, 4};
    int actual[5];
    const int count = collect_values(&it, actual, 5);

    ASSERT_EQ(count, 5);
    ASSERT_TRUE(verify_values(actual, expected, count, "basic_forward"));

    // Iterator should be exhausted
    ASSERT_FALSE(it.has_next(&it));
    ASSERT_NULL(it.get(&it));

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test basic backward iteration with negative step.
 */
static int test_range_negative_step(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(&alloc, 10, 5, -1);

    ASSERT_TRUE(it.is_valid(&it));

    const int expected[] = {10, 9, 8, 7, 6};
    int actual[5];
    const int count = collect_values(&it, actual, 5);

    ASSERT_EQ(count, 5);
    ASSERT_TRUE(verify_values(actual, expected, count, "basic_backward"));

    // Iterator should be exhausted
    ASSERT_FALSE(it.has_next(&it));
    ASSERT_NULL(it.get(&it));

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test iteration with larger positive step.
 */
static int test_range_larger_step(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(&alloc, 2, 15, 3);

    ASSERT_TRUE(it.is_valid(&it));

    const int expected[] = {2, 5, 8, 11, 14};
    int actual[5];
    const int count = collect_values(&it, actual, 5);

    ASSERT_EQ(count, 5);
    ASSERT_TRUE(verify_values(actual, expected, count, "positive_step"));

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test iteration with larger negative step.
 */
static int test_range_negative_step_size(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(&alloc, 20, 5, -4);

    ASSERT_TRUE(it.is_valid(&it));

    const int expected[] = {20, 16, 12, 8};
    int actual[4];
    const int count = collect_values(&it, actual, 4);

    ASSERT_EQ(count, 4);
    ASSERT_TRUE(verify_values(actual, expected, count, "negative_step"));

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test empty range where start equals end.
 */
static int test_range_empty(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(&alloc, 5, 5, 1);

    ASSERT_TRUE(it.is_valid(&it));
    ASSERT_FALSE(it.has_next(&it));
    ASSERT_NULL(it.get(&it));

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test single element range.
 */
static int test_single_element_range(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(&alloc, 7, 8, 1);

    ASSERT_TRUE(it.is_valid(&it));
    ASSERT_TRUE(it.has_next(&it));

    const int* value = it.get(&it);
    ASSERT_NOT_NULL(value);
    ASSERT_EQ(*value, 7);

    it.next(&it);
    ASSERT_FALSE(it.has_next(&it));
    ASSERT_NULL(it.get(&it));

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test range with extreme integer values.
 */
static int test_range_extreme_values(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Test near INT_MAX
    ANVIterator it1 = anv_iterator_range(&alloc, INT_MAX - 3, INT_MAX, 1);
    ASSERT_TRUE(it1.is_valid(&it1));

    const int expected_max[] = {INT_MAX - 3, INT_MAX - 2, INT_MAX - 1};
    int actual_max[3];
    const int count1 = collect_values(&it1, actual_max, 3);

    ASSERT_EQ(count1, 3);
    ASSERT_TRUE(verify_values(actual_max, expected_max, count1, "extreme_max"));
    it1.destroy(&it1);

    // Test near INT_MIN
    ANVIterator it2 = anv_iterator_range(&alloc, INT_MIN + 3, INT_MIN, -1);
    ASSERT_TRUE(it2.is_valid(&it2));

    const int expected_min[] = {INT_MIN + 3, INT_MIN + 2, INT_MIN + 1};
    int actual_min[3];
    const int count2 = collect_values(&it2, actual_min, 3);

    ASSERT_EQ(count2, 3);
    ASSERT_TRUE(verify_values(actual_min, expected_min, count2, "extreme_min"));
    it2.destroy(&it2);

    return TEST_SUCCESS;
}

/**
 * Test invalid step values.
 */
static int test_range_invalid_step(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Test zero step
    ANVIterator it1 = anv_iterator_range(&alloc, 0, 5, 0);
    ASSERT_FALSE(it1.is_valid(&it1));
    ASSERT_FALSE(it1.has_next(&it1));
    ASSERT_NULL(it1.get(&it1));
    it1.destroy(&it1);

    // Test conflicting direction: start < end with negative step
    ANVIterator it2 = anv_iterator_range(&alloc, 0, 10, -1);
    ASSERT_FALSE(it2.is_valid(&it2));
    ASSERT_FALSE(it2.has_next(&it2));
    ASSERT_NULL(it2.get(&it2));
    it2.destroy(&it2);

    // Test conflicting direction: start > end with positive step
    ANVIterator it3 = anv_iterator_range(&alloc, 10, 0, 1);
    ASSERT_FALSE(it3.is_valid(&it3));
    ASSERT_FALSE(it3.has_next(&it3));
    ASSERT_NULL(it3.get(&it3));
    it3.destroy(&it3);

    return TEST_SUCCESS;
}

/**
 * Test invalid allocator.
 */
static int test_invalid_allocator(void)
{
    ANVIterator it = anv_iterator_range(NULL, 0, 5, 1);
    ASSERT_FALSE(it.is_valid(&it));
    ASSERT_FALSE(it.has_next(&it));
    ASSERT_NULL(it.get(&it));
    it.destroy(&it);

    return TEST_SUCCESS;
}

/**
 * Test large range iteration performance.
 */
static int test_range_stress(void)
{
    const ANVAllocator alloc = create_int_allocator();
    const int SIZE = 10000;

    ANVIterator it = anv_iterator_range(&alloc, 0, SIZE, 1);
    ASSERT_TRUE(it.is_valid(&it));

    int count = 0;
    int expected = 0;

    while (it.has_next(&it))
    {
        const int* value = it.get(&it);
        ASSERT_NOT_NULL(value);
        ASSERT_EQ(*value, expected);

        count++;
        expected++;
        it.next(&it);
    }

    ASSERT_EQ(count, SIZE);
    ASSERT_FALSE(it.has_next(&it));

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test reset functionality.
 */
static int test_range_reset(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(&alloc, 0, 10, 2);

    ASSERT_TRUE(it.is_valid(&it));

    // Move forward a few steps
    const int* v1 = it.get(&it);
    ASSERT_EQ(*v1, 0); // Should be 0
    it.next(&it);      // Move to 2

    const int* v2 = it.get(&it);
    ASSERT_EQ(*v2, 2); // Should be 2
    it.next(&it);      // Move to 4

    const int* v3 = it.get(&it);
    ASSERT_EQ(*v3, 4); // Should be 4

    // Reset and verify we're back at start
    it.reset(&it);

    const int* reset_value = it.get(&it);
    ASSERT_EQ(*reset_value, 0); // Should be back to start

    // Verify normal iteration continues from start
    it.next(&it);
    const int* next_value = it.get(&it);
    ASSERT_EQ(*next_value, 2); // Should be first step from start

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test reset after bidirectional movement.
 */
static int test_reset_after_bidirectional(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(&alloc, 10, 20, 3);

    ASSERT_TRUE(it.is_valid(&it));

    // Forward and backward movement
    it.next(&it); // 10 -> 13
    it.next(&it); // 13 -> 16
    it.prev(&it); // 16 -> 13
    it.prev(&it); // 13 -> 10
    it.next(&it); // 10 -> 13

    const int* before_reset = it.get(&it);
    ASSERT_EQ(*before_reset, 13);

    // Reset should bring us back to start
    it.reset(&it);

    const int* after_reset = it.get(&it);
    ASSERT_EQ(*after_reset, 10); // This should always work - reset to start

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test the core zigzalogic.
 */
static int test_zigzag_compensation(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(&alloc, 0, 10, 1);

    ASSERT_TRUE(it.is_valid(&it));

    // Forward 3 steps: 0 → 1 → 2 → 3
    const int* v1 = it.get(&it);
    ASSERT_EQ(*v1, 0);
    it.next(&it);

    const int* v2 = it.get(&it);
    ASSERT_EQ(*v2, 1);
    it.next(&it);

    const int* v3 = it.get(&it);
    ASSERT_EQ(*v3, 2);
    it.next(&it);

    const int* v4 = it.get(&it);
    ASSERT_EQ(*v4, 3);

    // Move back 1: should go to 2
    ASSERT_TRUE(it.has_prev(&it));
    ASSERT_EQ(it.prev(&it), 0);

    const int* p1 = it.get(&it);
    ASSERT_EQ(*p1, 2); // Simple: 3 → 2

    // Move forward again: should go to 3
    it.next(&it);
    const int* f1 = it.get(&it);
    ASSERT_EQ(*f1, 3); // 2 → 3

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test bidirectional boundaries
 */
static int test_bidirectional_boundaries(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(&alloc, 0, 5, 1);

    ASSERT_TRUE(it.is_valid(&it));

    // Move forward to near boundary: 0 → 1 → 2 → 3 → 4
    it.next(&it); // 0 -> 1
    it.next(&it); // 1 -> 2
    it.next(&it); // 2 -> 3
    it.next(&it); // 3 -> 4

    const int* at_end = it.get(&it);
    ASSERT_EQ(*at_end, 4); // Should be at last valid element

    // Test backward from boundary: 4 → 3
    ASSERT_TRUE(it.has_prev(&it));
    ASSERT_EQ(it.prev(&it), 0);

    const int* back_one = it.get(&it);
    ASSERT_EQ(*back_one, 3); // Should be at previous element (not 2!)

    // Test forward again from this position: 3 → 4
    it.next(&it);
    const int* forward_again = it.get(&it);
    ASSERT_EQ(*forward_again, 4); // Should move forward properly

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test direction change
 */
static int test_direction_change_compensation(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(&alloc, 10, 20, 1);

    ASSERT_TRUE(it.is_valid(&it));

    // Start at 10, move forward to 12: 10 → 11 → 12
    const int* start = it.get(&it);
    ASSERT_EQ(*start, 10);
    it.next(&it); // -> 11

    const int* pos1 = it.get(&it);
    ASSERT_EQ(*pos1, 11);
    it.next(&it); // -> 12

    const int* pos2 = it.get(&it);
    ASSERT_EQ(*pos2, 12);

    // Change direction: 12 → 11
    it.prev(&it);
    const int* back_pos = it.get(&it);
    ASSERT_EQ(*back_pos, 11); // Should be 11, not 10

    // Change direction again: 11 → 12
    it.next(&it);
    const int* forward_pos = it.get(&it);
    ASSERT_EQ(*forward_pos, 12); // Should be 12, not 13

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test that we handle start boundary correctly.
 */
static int test_start_boundary_behavior(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(&alloc, 0, 5, 1);

    ASSERT_TRUE(it.is_valid(&it));

    // Move forward then back to start
    it.next(&it); // 0 -> 1
    it.next(&it); // 1 -> 2
    it.prev(&it); // 2 -> 1 (with compensation)
    it.prev(&it); // 1 -> 0 (with compensation)

    const int* at_start = it.get(&it);
    ASSERT_EQ(*at_start, 0);

    // Should be at start, no more backward movement
    ASSERT_FALSE(it.has_prev(&it));
    ASSERT_NOT_EQ(it.prev(&it), 0); // Should fail (return -1)

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test that get() doesn't advance, next() does advance.
 * Fixed: Store values before pointer changes.
 */
static int test_get_next_separation(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(&alloc, 5, 10, 1);

    ASSERT_TRUE(it.is_valid(&it));

    const int* v1 = it.get(&it);
    const int* v2 = it.get(&it);
    ASSERT_EQ(*v1, *v2); // get() shouldn't advance
    ASSERT_EQ(*v1, 5);   // Should be start value

    // Store value before advancing to avoid pointer reuse issues
    const int initial_value = *v1;

    it.next(&it); // This should advance
    const int* v3 = it.get(&it);
    const int new_value = *v3;

    ASSERT_NOT_EQ(initial_value, new_value); // Position should have changed
    ASSERT_EQ(new_value, 6);                 // Should be next value

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test next() return codes.
 */
static int test_next_return_codes(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(&alloc, 0, 2, 1);

    ASSERT_TRUE(it.is_valid(&it));

    ASSERT_EQ(it.next(&it), 0);  // Should succeed
    ASSERT_EQ(it.next(&it), 0);  // Should succeed
    ASSERT_EQ(it.next(&it), -1); // Should fail (at end)

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test prev() return codes.
 * Fixed: Based on actual boundary behavior from diagnostic.
 */
static int test_prev_return_codes(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(&alloc, 0, 3, 1);

    ASSERT_TRUE(it.is_valid(&it));

    // Move forward to position 2: 0 → 1 → 2
    it.next(&it); // 0 -> 1
    it.next(&it); // 1 -> 2

    // Now test backward movement: 2 → 1 → 0 → fail
    ASSERT_EQ(it.prev(&it), 0); // Should succeed: 2 -> 1
    ASSERT_EQ(it.prev(&it), 0); // Should succeed: 1 -> 0

    // At this point we're at start (0), so next prev should fail
    ASSERT_EQ(it.prev(&it), -1); // Should fail (at start)

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test memory consistency across operations.
 */
static int test_memory_consistency(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(&alloc, 100, 105, 1);

    ASSERT_TRUE(it.is_valid(&it));

    // Get multiple references to the same value
    const int* ptr1 = it.get(&it);
    const int* ptr2 = it.get(&it);
    const int* ptr3 = it.get(&it);

    // All should point to the same memory and have the same value
    ASSERT_EQ(ptr1, ptr2);
    ASSERT_EQ(ptr2, ptr3);
    ASSERT_EQ(*ptr1, 100);
    ASSERT_EQ(*ptr2, 100);
    ASSERT_EQ(*ptr3, 100);

    // Move to next and verify new value
    it.next(&it);
    const int* ptr4 = it.get(&it);
    ASSERT_EQ(*ptr4, 101);

    // Previous pointers should now point to updated value (same memory)
    ASSERT_EQ(ptr1, ptr4);
    ASSERT_EQ(*ptr1, 101);

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test has_prev() behavior at start position.
 */
static int test_has_prev_at_start(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(&alloc, 10, 15, 1);

    ASSERT_TRUE(it.is_valid(&it));

    // Should be false at start position
    ASSERT_FALSE(it.has_prev(&it));

    // Move forward once
    ASSERT_EQ(it.next(&it), 0);
    ASSERT_TRUE(it.has_prev(&it));

    // Move back to start
    ASSERT_EQ(it.prev(&it), 0);
    ASSERT_FALSE(it.has_prev(&it));

    // Verify we're actually at start
    const int* value = it.get(&it);
    ASSERT_EQ(*value, 10);

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test has_next() behavior at end position.
 */
static int test_has_next_at_end(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(&alloc, 0, 3, 1);

    ASSERT_TRUE(it.is_valid(&it));

    // Advance to end: 0->1->2->3 (but 3 is outside range [0,3))
    ASSERT_EQ(it.next(&it), 0); // 0->1
    ASSERT_TRUE(it.has_next(&it));

    ASSERT_EQ(it.next(&it), 0); // 1->2
    ASSERT_TRUE(it.has_next(&it));

    ASSERT_EQ(it.next(&it), 0); // 2->3 (3 is outside range)
    ASSERT_FALSE(it.has_next(&it));
    ASSERT_NULL(it.get(&it));

    // Further advance should fail
    ASSERT_EQ(it.next(&it), -1);

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test bidirectional movement with different step sizes.
 */
static int test_bidirectional_with_large_steps(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(&alloc, 0, 20, 5);

    ASSERT_TRUE(it.is_valid(&it));

    // Start at 0
    const int* v0 = it.get(&it);
    ASSERT_EQ(*v0, 0);

    // Forward: 0->5->10
    ASSERT_EQ(it.next(&it), 0); // 0->5
    const int* v1 = it.get(&it);
    ASSERT_EQ(*v1, 5);

    ASSERT_EQ(it.next(&it), 0); // 5->10
    const int* v2 = it.get(&it);
    ASSERT_EQ(*v2, 10);

    // Backward: 10->5->0
    ASSERT_EQ(it.prev(&it), 0); // 10->5
    const int* v3 = it.get(&it);
    ASSERT_EQ(*v3, 5);

    ASSERT_EQ(it.prev(&it), 0); // 5->0
    const int* v4 = it.get(&it);
    ASSERT_EQ(*v4, 0);

    // Should be back at start
    ASSERT_FALSE(it.has_prev(&it));

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test negative step iteration with proper boundary handling.
 */
static int test_negative_step_boundaries(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(&alloc, 10, 0, -2);

    ASSERT_TRUE(it.is_valid(&it));
    // Expected range: 10, 8, 6, 4, 2

    // Test forward iteration to boundary
    const int expected[] = {10, 8, 6, 4, 2};
    int actual[5];
    const int count = collect_values(&it, actual, 5);

    ASSERT_EQ(count, 5);
    ASSERT_TRUE(verify_values(actual, expected, count, "negative_step_boundaries"));

    // Should be at end, no more elements
    ASSERT_FALSE(it.has_next(&it));
    ASSERT_NULL(it.get(&it));

    // Test backward from end (but we're past the last valid element)
    // Reset to test backward from a valid position
    it.reset(&it);

    // Move to second position (8) then test backward
    it.next(&it); // 10->8
    const int* pos1 = it.get(&it);
    ASSERT_EQ(*pos1, 8);

    // Move backward to start
    ASSERT_TRUE(it.has_prev(&it));
    ASSERT_EQ(it.prev(&it), 0);
    const int* pos2 = it.get(&it);
    ASSERT_EQ(*pos2, 10);

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test operations on invalid iterator.
 */
static int test_operations_on_invalid_iterator(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(&alloc, 0, 5, 0); // Invalid: zero step

    ASSERT_FALSE(it.is_valid(&it));

    // All operations should fail gracefully
    ASSERT_EQ(it.next(&it), -1);
    ASSERT_EQ(it.prev(&it), -1);
    ASSERT_FALSE(it.has_next(&it));
    ASSERT_FALSE(it.has_prev(&it));
    ASSERT_NULL(it.get(&it));

    // Reset should be safe to call but ineffective
    it.reset(&it); // Should not crash
    ASSERT_FALSE(it.is_valid(&it));

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test reset functionality after boundary errors.
 */
static int test_reset_after_boundary_errors(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(&alloc, 0, 3, 1);

    ASSERT_TRUE(it.is_valid(&it));

    // Move to end and try to go past boundary
    ASSERT_EQ(it.next(&it), 0); // 0->1
    ASSERT_EQ(it.next(&it), 0); // 1->2
    ASSERT_EQ(it.next(&it), 0); // 2->3 (outside range)

    // Further advance should fail
    ASSERT_EQ(it.next(&it), -1);
    ASSERT_FALSE(it.has_next(&it));

    // Reset should still work and bring us back to start
    it.reset(&it);
    ASSERT_TRUE(it.is_valid(&it));

    const int* value = it.get(&it);
    ASSERT_NOT_NULL(value);
    ASSERT_EQ(*value, 0);

    // Normal iteration should work after reset
    ASSERT_TRUE(it.has_next(&it));
    ASSERT_EQ(it.next(&it), 0);
    const int* next_value = it.get(&it);
    ASSERT_EQ(*next_value, 1);

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test that get() calls during movement maintain pointer consistency.
 */
static int test_concurrent_get_calls_during_movement(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(&alloc, 100, 104, 1);

    ASSERT_TRUE(it.is_valid(&it));

    // Get initial pointer
    const int* ptr1 = it.get(&it);
    ASSERT_EQ(*ptr1, 100);

    // Move and get new pointer
    it.next(&it);
    const int* ptr2 = it.get(&it);
    ASSERT_EQ(*ptr2, 101);

    // Move back and get another pointer
    it.prev(&it);
    const int* ptr3 = it.get(&it);
    ASSERT_EQ(*ptr3, 100);

    // All pointers should point to same memory location
    ASSERT_EQ(ptr1, ptr2);
    ASSERT_EQ(ptr2, ptr3);

    // Current value should be back to original
    ASSERT_EQ(*ptr1, 100); // ptr1 should now show updated value
    ASSERT_EQ(*ptr2, 100); // ptr2 should show same updated value
    ASSERT_EQ(*ptr3, 100); // ptr3 should show same value

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test boundary conditions with single-step movements.
 */
static int test_single_step_boundaries(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(&alloc, 5, 8, 1);

    ASSERT_TRUE(it.is_valid(&it));
    // Range: 5, 6, 7

    // Test forward boundary
    const int* v1 = it.get(&it);
    ASSERT_EQ(*v1, 5);

    it.next(&it); // 5->6
    const int* v2 = it.get(&it);
    ASSERT_EQ(*v2, 6);

    it.next(&it); // 6->7
    const int* v3 = it.get(&it);
    ASSERT_EQ(*v3, 7);

    it.next(&it); // 7->8 (outside range)
    ASSERT_NULL(it.get(&it));
    ASSERT_FALSE(it.has_next(&it));

    // Test that prev still works from invalid position
    ASSERT_TRUE(it.has_prev(&it));
    ASSERT_EQ(it.prev(&it), 0); // Should go back to 7
    const int* v4 = it.get(&it);
    ASSERT_EQ(*v4, 7);

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test helper function validation.
 */
static int test_helper_function_validation(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(&alloc, 0, 5, 1);

    ASSERT_TRUE(it.is_valid(&it));

    int values[5];
    const int count = collect_values_with_validation(&it, values, 5);

    // Should successfully collect all 5 values
    ASSERT_EQ(count, 5);

    const int expected[] = {0, 1, 2, 3, 4};
    ASSERT_TRUE(verify_values(values, expected, count, "helper_validation"));

    it.destroy(&it);
    return TEST_SUCCESS;
}

//==============================================================================
// Repeat Iterator Tests
//==============================================================================

static int test_repeat_basic_functionality(void)
{
    const ANVAllocator alloc = create_int_allocator();

    const int value = 42;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 5);
    ASSERT_TRUE(repeat_it.is_valid(&repeat_it));

    int values[10];
    const int count = collect_values(&repeat_it, values, 10);

    ASSERT_EQ(count, 5);
    ASSERT_TRUE(verify_repeated_values(values, 42, 5, "repeat_basic"));

    repeat_it.destroy(&repeat_it);
    return TEST_SUCCESS;
}

static int test_repeat_single_count(void)
{
    const ANVAllocator alloc = create_int_allocator();

    const int value = 99;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 1);
    ASSERT_TRUE(repeat_it.is_valid(&repeat_it));

    ASSERT_TRUE(repeat_it.has_next(&repeat_it));
    const int* retrieved = repeat_it.get(&repeat_it);
    ASSERT_NOT_NULL(retrieved);
    ASSERT_EQ(*retrieved, 99);

    ASSERT_EQ(repeat_it.next(&repeat_it), 0);
    ASSERT_FALSE(repeat_it.has_next(&repeat_it));
    ASSERT_NULL(repeat_it.get(&repeat_it));

    repeat_it.destroy(&repeat_it);
    return TEST_SUCCESS;
}

static int test_repeat_zero_count(void)
{
    const ANVAllocator alloc = create_int_allocator();

    const int value = 123;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 0);
    ASSERT_TRUE(repeat_it.is_valid(&repeat_it));

    // Should have no elements
    ASSERT_FALSE(repeat_it.has_next(&repeat_it));
    ASSERT_NULL(repeat_it.get(&repeat_it));

    repeat_it.destroy(&repeat_it);
    return TEST_SUCCESS;
}

static int test_repeat_large_count(void)
{
    const ANVAllocator alloc = create_int_allocator();

    const int value = 777;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 1000);
    ASSERT_TRUE(repeat_it.is_valid(&repeat_it));

    // Test first few elements
    for (int i = 0; i < 10; i++)
    {
        ASSERT_TRUE(repeat_it.has_next(&repeat_it));
        const int* retrieved = repeat_it.get(&repeat_it);
        ASSERT_NOT_NULL(retrieved);
        ASSERT_EQ(*retrieved, 777);
        ASSERT_EQ(repeat_it.next(&repeat_it), 0);
    }

    // Skip to near the end
    for (int i = 10; i < 999; i++)
    {
        ASSERT_TRUE(repeat_it.has_next(&repeat_it));
        ASSERT_EQ(repeat_it.next(&repeat_it), 0);
    }

    // Should still have one element left
    ASSERT_TRUE(repeat_it.has_next(&repeat_it));
    const int* last = repeat_it.get(&repeat_it);
    ASSERT_NOT_NULL(last);
    ASSERT_EQ(*last, 777);

    ASSERT_EQ(repeat_it.next(&repeat_it), 0);
    ASSERT_FALSE(repeat_it.has_next(&repeat_it));

    repeat_it.destroy(&repeat_it);
    return TEST_SUCCESS;
}

static int test_repeat_different_data_types(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Test with negative number
    const int negative_value = -42;
    ANVIterator repeat_it1 = anv_iterator_repeat(&negative_value, &alloc, 3);
    ASSERT_TRUE(repeat_it1.is_valid(&repeat_it1));

    int values1[5];
    const int count1 = collect_values(&repeat_it1, values1, 5);
    ASSERT_EQ(count1, 3);
    ASSERT_TRUE(verify_repeated_values(values1, -42, 3, "repeat_negative"));

    repeat_it1.destroy(&repeat_it1);

    // Test with zero
    const int zero_value = 0;
    ANVIterator repeat_it2 = anv_iterator_repeat(&zero_value, &alloc, 4);
    ASSERT_TRUE(repeat_it2.is_valid(&repeat_it2));

    int values2[5];
    const int count2 = collect_values(&repeat_it2, values2, 5);
    ASSERT_EQ(count2, 4);
    ASSERT_TRUE(verify_repeated_values(values2, 0, 4, "repeat_zero"));

    repeat_it2.destroy(&repeat_it2);

    return TEST_SUCCESS;
}

static int test_repeat_invalid_parameters(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Test with NULL value
    const ANVIterator repeat_it1 = anv_iterator_repeat(NULL, &alloc, 5);
    ASSERT_FALSE(repeat_it1.is_valid(&repeat_it1));

    // Test with NULL allocator
    const int value = 42;
    const ANVIterator repeat_it2 = anv_iterator_repeat(&value, NULL, 5);
    ASSERT_FALSE(repeat_it2.is_valid(&repeat_it2));

    return TEST_SUCCESS;
}

static int test_repeat_pointer_consistency(void)
{
    const ANVAllocator alloc = create_int_allocator();

    int value = 888;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 3);

    ASSERT_TRUE(repeat_it.has_next(&repeat_it));

    // Multiple calls to get() should return the same pointer
    const int* ptr1 = repeat_it.get(&repeat_it);
    const int* ptr2 = repeat_it.get(&repeat_it);
    ASSERT_EQ(ptr1, ptr2);
    ASSERT_EQ(ptr1, &value); // Should point to original value

    // After next(), should still point to same original value
    repeat_it.next(&repeat_it);
    const int* ptr3 = repeat_it.get(&repeat_it);
    ASSERT_EQ(ptr3, &value);

    repeat_it.destroy(&repeat_it);
    return TEST_SUCCESS;
}

static int test_repeat_exhausted_iterator(void)
{
    const ANVAllocator alloc = create_int_allocator();

    const int value = 555;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 2);

    // Exhaust the iterator
    ASSERT_TRUE(repeat_it.has_next(&repeat_it));
    repeat_it.next(&repeat_it);
    ASSERT_TRUE(repeat_it.has_next(&repeat_it));
    repeat_it.next(&repeat_it);

    // Should be exhausted now
    ASSERT_FALSE(repeat_it.has_next(&repeat_it));
    ASSERT_NULL(repeat_it.get(&repeat_it));
    ASSERT_EQ(repeat_it.next(&repeat_it), -1);

    // Multiple calls to next() on exhausted iterator should return -1
    ASSERT_EQ(repeat_it.next(&repeat_it), -1);
    ASSERT_EQ(repeat_it.next(&repeat_it), -1);

    repeat_it.destroy(&repeat_it);
    return TEST_SUCCESS;
}

static int test_repeat_reset_functionality(void)
{
    const ANVAllocator alloc = create_int_allocator();

    const int value = 321;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 3);

    // Consume first element
    ASSERT_TRUE(repeat_it.has_next(&repeat_it));
    const int* val1 = repeat_it.get(&repeat_it);
    ASSERT_EQ(*val1, 321);
    repeat_it.next(&repeat_it);

    // Consume second element
    ASSERT_TRUE(repeat_it.has_next(&repeat_it));
    const int* val2 = repeat_it.get(&repeat_it);
    ASSERT_EQ(*val2, 321);
    repeat_it.next(&repeat_it);

    // Reset iterator
    repeat_it.reset(&repeat_it);

    // Should be back at beginning
    int values[5];
    const int count = collect_values(&repeat_it, values, 5);
    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_repeated_values(values, 321, 3, "repeat_reset"));

    repeat_it.destroy(&repeat_it);
    return TEST_SUCCESS;
}

static int test_repeat_reset_exhausted(void)
{
    const ANVAllocator alloc = create_int_allocator();

    const int value = 654;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 2);

    // Exhaust the iterator
    repeat_it.next(&repeat_it);
    repeat_it.next(&repeat_it);
    ASSERT_FALSE(repeat_it.has_next(&repeat_it));

    // Reset should restore functionality
    repeat_it.reset(&repeat_it);

    int values[5];
    const int count = collect_values(&repeat_it, values, 5);
    ASSERT_EQ(count, 2);
    ASSERT_TRUE(verify_repeated_values(values, 654, 2, "repeat_reset_exhausted"));

    repeat_it.destroy(&repeat_it);
    return TEST_SUCCESS;
}

static int test_repeat_reset_empty(void)
{
    const ANVAllocator alloc = create_int_allocator();

    const int value = 111;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 0);

    // Should be empty
    ASSERT_FALSE(repeat_it.has_next(&repeat_it));

    // Reset on empty iterator
    repeat_it.reset(&repeat_it);

    // Should still be empty
    ASSERT_FALSE(repeat_it.has_next(&repeat_it));
    ASSERT_NULL(repeat_it.get(&repeat_it));

    repeat_it.destroy(&repeat_it);
    return TEST_SUCCESS;
}

static int test_repeat_with_filter(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Repeat even number 6 times, then filter for evens (should pass all)
    const int value = 8;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 6);
    ANVIterator filter_it = anv_iterator_filter(&repeat_it, &alloc, is_even);
    ASSERT_TRUE(filter_it.is_valid(&filter_it));

    int values[10];
    const int count = collect_values(&filter_it, values, 10);

    ASSERT_EQ(count, 6);
    ASSERT_TRUE(verify_repeated_values(values, 8, 6, "repeat_with_filter_pass"));

    filter_it.destroy(&filter_it);
    return TEST_SUCCESS;
}

static int test_repeat_with_filter_reject(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Repeat odd number 5 times, then filter for evens (should reject all)
    const int value = 7;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 5);
    ANVIterator filter_it = anv_iterator_filter(&repeat_it, &alloc, is_even);
    ASSERT_TRUE(filter_it.is_valid(&filter_it));

    // Should have no elements
    ASSERT_FALSE(filter_it.has_next(&filter_it));
    ASSERT_NULL(filter_it.get(&filter_it));

    filter_it.destroy(&filter_it);
    return TEST_SUCCESS;
}

static int test_repeat_with_take(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Repeat value 10 times, then take first 3
    const int value = 456;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 10);
    ANVIterator take_it = anv_iterator_take(&repeat_it, &alloc, 3);
    ASSERT_TRUE(take_it.is_valid(&take_it));

    int values[10];
    const int count = collect_values(&take_it, values, 10);

    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_repeated_values(values, 456, 3, "repeat_with_take"));

    take_it.destroy(&take_it);
    return TEST_SUCCESS;
}

static int test_repeat_with_take_more_than_available(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Repeat value 3 times, then try to take 5
    const int value = 789;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 3);
    ANVIterator take_it = anv_iterator_take(&repeat_it, &alloc, 5);
    ASSERT_TRUE(take_it.is_valid(&take_it));

    int values[10];
    const int count = collect_values(&take_it, values, 10);

    ASSERT_EQ(count, 3); // Limited by repeat count
    ASSERT_TRUE(verify_repeated_values(values, 789, 3, "repeat_with_take_limited"));

    take_it.destroy(&take_it);
    return TEST_SUCCESS;
}

static int test_repeat_with_skip(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Repeat value 8 times, then skip first 3
    const int value = 202;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 8);
    ANVIterator skip_it = anv_iterator_skip(&repeat_it, &alloc, 3);
    ASSERT_TRUE(skip_it.is_valid(&skip_it));

    int values[10];
    const int count = collect_values(&skip_it, values, 10);

    ASSERT_EQ(count, 5); // 8 - 3 = 5
    ASSERT_TRUE(verify_repeated_values(values, 202, 5, "repeat_with_skip"));

    skip_it.destroy(&skip_it);
    return TEST_SUCCESS;
}

static int test_repeat_with_skip_all(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Repeat value 4 times, then skip 4 (all)
    const int value = 303;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 4);
    ANVIterator skip_it = anv_iterator_skip(&repeat_it, &alloc, 4);
    ASSERT_TRUE(skip_it.is_valid(&skip_it));

    // Should have no elements
    ASSERT_FALSE(skip_it.has_next(&skip_it));
    ASSERT_NULL(skip_it.get(&skip_it));

    skip_it.destroy(&skip_it);
    return TEST_SUCCESS;
}

static int test_repeat_with_zip(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create two repeat iterators and zip them
    const int value1 = 100;
    const int value2 = 200;
    ANVIterator repeat_it1 = anv_iterator_repeat(&value1, &alloc, 4);
    ANVIterator repeat_it2 = anv_iterator_repeat(&value2, &alloc, 4);

    ANVIterator zip_it = anv_iterator_zip(&repeat_it1, &repeat_it2, &alloc);
    ASSERT_TRUE(zip_it.is_valid(&zip_it));

    // Test all pairs
    for (int i = 0; i < 4; i++)
    {
        ASSERT_TRUE(zip_it.has_next(&zip_it));
        const ANVPair* pair = zip_it.get(&zip_it);
        ASSERT_NOT_NULL(pair);
        ASSERT_NOT_NULL(pair->first);
        ASSERT_NOT_NULL(pair->second);
        ASSERT_EQ(*(const int*)pair->first, 100);
        ASSERT_EQ(*(const int*)pair->second, 200);
        zip_it.next(&zip_it);
    }

    ASSERT_FALSE(zip_it.has_next(&zip_it));

    zip_it.destroy(&zip_it);
    return TEST_SUCCESS;
}

static int test_repeat_with_enumerate(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Repeat value 3 times, then enumerate
    const int value = 555;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 3);
    ANVIterator enum_it = anv_iterator_enumerate(&repeat_it, &alloc, 0);
    ASSERT_TRUE(enum_it.is_valid(&enum_it));

    // Test enumeration
    for (int i = 0; i < 3; i++)
    {
        ASSERT_TRUE(enum_it.has_next(&enum_it));
        const ANVIndexedElement* indexed = enum_it.get(&enum_it);
        ASSERT_NOT_NULL(indexed);
        ASSERT_EQ(indexed->index, (size_t)i);
        ASSERT_NOT_NULL(indexed->element);
        ASSERT_EQ(*(const int*)indexed->element, 555);
        enum_it.next(&enum_it);
    }

    ASSERT_FALSE(enum_it.has_next(&enum_it));

    enum_it.destroy(&enum_it);
    return TEST_SUCCESS;
}

static int test_repeat_chained_operations(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Complex chain: repeat 10 times -> skip 2 -> take 5 -> filter evens
    const int value = 4; // Even number
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 10);
    ANVIterator skip_it = anv_iterator_skip(&repeat_it, &alloc, 2);
    ANVIterator take_it = anv_iterator_take(&skip_it, &alloc, 5);
    ANVIterator filter_it = anv_iterator_filter(&take_it, &alloc, is_even);
    ASSERT_TRUE(filter_it.is_valid(&filter_it));

    int values[10];
    const int count = collect_values(&filter_it, values, 10);

    ASSERT_EQ(count, 5); // All 5 remaining elements should pass even filter
    ASSERT_TRUE(verify_repeated_values(values, 4, 5, "repeat_chained"));

    filter_it.destroy(&filter_it);
    return TEST_SUCCESS;
}

static int test_repeat_with_arraylist(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create ArrayList with some values
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);
    for (int i = 1; i <= 3; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i * 10; // [10, 20, 30]
        anv_arraylist_push_back(list, val);
    }

    // Create repeat iterator
    const int repeat_value = 99;
    ANVIterator repeat_it = anv_iterator_repeat(&repeat_value, &alloc, 3);

    // Zip ArrayList with repeat iterator
    ANVIterator array_iter = anv_arraylist_iterator(list);
    ANVIterator zip_it = anv_iterator_zip(&array_iter, &repeat_it, &alloc);
    ASSERT_TRUE(zip_it.is_valid(&zip_it));

    // Verify pairs
    const int expected_first[] = {10, 20, 30};
    const int expected_second[] = {99, 99, 99};

    for (int i = 0; i < 3; i++)
    {
        ASSERT_TRUE(zip_it.has_next(&zip_it));
        const ANVPair* pair = zip_it.get(&zip_it);
        ASSERT_NOT_NULL(pair);
        ASSERT_EQ(*(const int*)pair->first, expected_first[i]);
        ASSERT_EQ(*(const int*)pair->second, expected_second[i]);
        zip_it.next(&zip_it);
    }

    ASSERT_FALSE(zip_it.has_next(&zip_it));

    zip_it.destroy(&zip_it);
    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_repeat_with_range(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Zip range with repeat iterator
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 6, 1); // [1,2,3,4,5]

    const int repeat_value = -1;
    ANVIterator repeat_it = anv_iterator_repeat(&repeat_value, &alloc, 5);

    ANVIterator zip_it = anv_iterator_zip(&range_it, &repeat_it, &alloc);
    ASSERT_TRUE(zip_it.is_valid(&zip_it));

    // Verify all pairs
    for (int i = 1; i <= 5; i++)
    {
        ASSERT_TRUE(zip_it.has_next(&zip_it));
        const ANVPair* pair = zip_it.get(&zip_it);
        ASSERT_NOT_NULL(pair);
        ASSERT_EQ(*(const int*)pair->first, i);
        ASSERT_EQ(*(const int*)pair->second, -1);
        zip_it.next(&zip_it);
    }

    ASSERT_FALSE(zip_it.has_next(&zip_it));

    zip_it.destroy(&zip_it);
    return TEST_SUCCESS;
}

static int test_repeat_iteration_state(void)
{
    const ANVAllocator alloc = create_int_allocator();

    const int value = 888;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 3);

    // Test step-by-step iteration
    ASSERT_TRUE(repeat_it.has_next(&repeat_it));
    const int* val1 = repeat_it.get(&repeat_it);
    ASSERT_NOT_NULL(val1);
    ASSERT_EQ(*val1, 888);

    ASSERT_EQ(repeat_it.next(&repeat_it), 0);
    ASSERT_TRUE(repeat_it.has_next(&repeat_it));

    const int* val2 = repeat_it.get(&repeat_it);
    ASSERT_NOT_NULL(val2);
    ASSERT_EQ(*val2, 888);

    ASSERT_EQ(repeat_it.next(&repeat_it), 0);
    ASSERT_TRUE(repeat_it.has_next(&repeat_it));

    const int* val3 = repeat_it.get(&repeat_it);
    ASSERT_NOT_NULL(val3);
    ASSERT_EQ(*val3, 888);

    ASSERT_EQ(repeat_it.next(&repeat_it), 0);
    ASSERT_FALSE(repeat_it.has_next(&repeat_it));
    ASSERT_NULL(repeat_it.get(&repeat_it));
    ASSERT_EQ(repeat_it.next(&repeat_it), -1);

    repeat_it.destroy(&repeat_it);
    return TEST_SUCCESS;
}

static int test_repeat_unsupported_operations(void)
{
    const ANVAllocator alloc = create_int_allocator();

    const int value = 777;
    ANVIterator repeat_it = anv_iterator_repeat(&value, &alloc, 5);

    // Test unsupported operations
    ASSERT_FALSE(repeat_it.has_prev(&repeat_it));
    ASSERT_EQ(repeat_it.prev(&repeat_it), -1);

    // Advance iterator then test again
    repeat_it.next(&repeat_it);
    ASSERT_FALSE(repeat_it.has_prev(&repeat_it));
    ASSERT_EQ(repeat_it.prev(&repeat_it), -1);

    repeat_it.destroy(&repeat_it);
    return TEST_SUCCESS;
}

//==============================================================================
// Skip Iterator Tests
//==============================================================================

static int test_skip_basic_functionality(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator 1-10
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 11, 1);

    // Skip first 3 elements (should yield 4, 5, 6, 7, 8, 9, 10)
    ANVIterator skip_it = anv_iterator_skip(&range_it, &alloc, 3);
    ASSERT_TRUE(skip_it.is_valid(&skip_it));

    int values[10];
    const int count = collect_values(&skip_it, values, 10);

    ASSERT_EQ(count, 7);

    const int expected[] = {4, 5, 6, 7, 8, 9, 10};
    ASSERT_TRUE(verify_values(values, expected, 7, "skip_basic"));

    skip_it.destroy(&skip_it);
    return TEST_SUCCESS;
}

static int test_skip_zero_count(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator 1-5
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 6, 1);

    // Skip 0 elements (should yield all elements)
    ANVIterator skip_it = anv_iterator_skip(&range_it, &alloc, 0);
    ASSERT_TRUE(skip_it.is_valid(&skip_it));

    int values[10];
    const int count = collect_values(&skip_it, values, 10);

    ASSERT_EQ(count, 5);

    const int expected[] = {1, 2, 3, 4, 5};
    ASSERT_TRUE(verify_values(values, expected, 5, "skip_zero"));

    skip_it.destroy(&skip_it);
    return TEST_SUCCESS;
}

static int test_skip_more_than_available(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator with only 3 elements
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 4, 1);

    // Try to skip 10 elements (more than available)
    ANVIterator skip_it = anv_iterator_skip(&range_it, &alloc, 10);
    ASSERT_TRUE(skip_it.is_valid(&skip_it));

    // Should have no elements
    ASSERT_FALSE(skip_it.has_next(&skip_it));
    ASSERT_NULL(skip_it.get(&skip_it));

    skip_it.destroy(&skip_it);
    return TEST_SUCCESS;
}

static int test_skip_all_elements(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator 1-5
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 6, 1);

    // Skip exactly all elements
    ANVIterator skip_it = anv_iterator_skip(&range_it, &alloc, 5);
    ASSERT_TRUE(skip_it.is_valid(&skip_it));

    // Should have no elements
    ASSERT_FALSE(skip_it.has_next(&skip_it));
    ASSERT_NULL(skip_it.get(&skip_it));

    skip_it.destroy(&skip_it);
    return TEST_SUCCESS;
}

static int test_skip_single_element(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator 1-5
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 6, 1);

    // Skip only 1 element
    ANVIterator skip_it = anv_iterator_skip(&range_it, &alloc, 1);
    ASSERT_TRUE(skip_it.is_valid(&skip_it));

    int values[10];
    const int count = collect_values(&skip_it, values, 10);

    ASSERT_EQ(count, 4);

    const int expected[] = {2, 3, 4, 5};
    ASSERT_TRUE(verify_values(values, expected, 4, "skip_single"));

    skip_it.destroy(&skip_it);
    return TEST_SUCCESS;
}

static int test_skip_empty_source(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create empty range iterator
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 1, 1); // Empty range

    // Try to skip 5 elements from empty iterator
    ANVIterator skip_it = anv_iterator_skip(&range_it, &alloc, 5);
    ASSERT_TRUE(skip_it.is_valid(&skip_it));

    // Should have no elements
    ASSERT_FALSE(skip_it.has_next(&skip_it));
    ASSERT_NULL(skip_it.get(&skip_it));

    skip_it.destroy(&skip_it);
    return TEST_SUCCESS;
}

static int test_skip_invalid_parameters(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Test with NULL iterator
    const ANVIterator skip_it1 = anv_iterator_skip(NULL, &alloc, 5);
    ASSERT_FALSE(skip_it1.is_valid(&skip_it1));

    // Test with NULL allocator
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 11, 1);

    const ANVIterator skip_it2 = anv_iterator_skip(&range_it, NULL, 5);
    ASSERT_FALSE(skip_it2.is_valid(&skip_it2));

    // Clean up the range iterator manually since skip failed
    range_it.destroy(&range_it);

    return TEST_SUCCESS;
}

static int test_skip_large_count(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator 1-5
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 6, 1);

    // Skip very large number of elements
    ANVIterator skip_it = anv_iterator_skip(&range_it, &alloc, SIZE_MAX);
    ASSERT_TRUE(skip_it.is_valid(&skip_it));

    // Should have no elements
    ASSERT_FALSE(skip_it.has_next(&skip_it));
    ASSERT_NULL(skip_it.get(&skip_it));

    skip_it.destroy(&skip_it);
    return TEST_SUCCESS;
}

static int test_skip_with_filter(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range 1-10, filter evens, then skip 1 (should skip first even: 2)
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 11, 1);

    ANVIterator filter_it = anv_iterator_filter(&range_it, &alloc, is_even);

    ANVIterator skip_it = anv_iterator_skip(&filter_it, &alloc, 1);
    ASSERT_TRUE(skip_it.is_valid(&skip_it));

    int values[10];
    const int count = collect_values(&skip_it, values, 10);

    ASSERT_EQ(count, 4);

    const int expected[] = {4, 6, 8, 10}; // Even numbers after skipping first even (2)
    ASSERT_TRUE(verify_values(values, expected, 4, "skip_with_filter"));

    skip_it.destroy(&skip_it);
    return TEST_SUCCESS;
}

static int test_skip_chained(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range 1-20, skip 5, then skip 2 more from that
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 21, 1);

    ANVIterator skip_it1 = anv_iterator_skip(&range_it, &alloc, 5);

    ANVIterator skip_it2 = anv_iterator_skip(&skip_it1, &alloc, 2);
    ASSERT_TRUE(skip_it2.is_valid(&skip_it2));

    int values[20];
    const int count = collect_values(&skip_it2, values, 20);

    ASSERT_EQ(count, 13);

    const int expected[] = {8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
    ASSERT_TRUE(verify_values(values, expected, 13, "skip_chained"));

    skip_it2.destroy(&skip_it2);
    return TEST_SUCCESS;
}

static int test_skip_with_take(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range 1-20, skip 3, then take 5
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 21, 1);

    ANVIterator skip_it = anv_iterator_skip(&range_it, &alloc, 3);

    ANVIterator take_it = anv_iterator_take(&skip_it, &alloc, 5);
    ASSERT_TRUE(take_it.is_valid(&take_it));

    int values[10];
    const int count = collect_values(&take_it, values, 10);

    ASSERT_EQ(count, 5);

    const int expected[] = {4, 5, 6, 7, 8}; // Skip first 3, then take next 5
    ASSERT_TRUE(verify_values(values, expected, 5, "skip_with_take"));

    take_it.destroy(&take_it);
    return TEST_SUCCESS;
}

static int test_skip_iteration_state(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator 1-10
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 11, 1);

    // Skip first 2 elements
    ANVIterator skip_it = anv_iterator_skip(&range_it, &alloc, 2);

    // Test step-by-step iteration
    ASSERT_TRUE(skip_it.has_next(&skip_it));
    const int* val1 = skip_it.get(&skip_it);
    ASSERT_NOT_NULL(val1);
    ASSERT_EQ(*val1, 3); // First non-skipped element

    ASSERT_EQ(skip_it.next(&skip_it), 0);
    ASSERT_TRUE(skip_it.has_next(&skip_it));

    const int* val2 = skip_it.get(&skip_it);
    ASSERT_NOT_NULL(val2);
    ASSERT_EQ(*val2, 4);

    ASSERT_EQ(skip_it.next(&skip_it), 0);
    ASSERT_TRUE(skip_it.has_next(&skip_it));

    const int* val3 = skip_it.get(&skip_it);
    ASSERT_NOT_NULL(val3);
    ASSERT_EQ(*val3, 5);

    skip_it.destroy(&skip_it);
    return TEST_SUCCESS;
}

static int test_skip_lazy_evaluation(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator 1-10
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 11, 1);

    // Skip first 3 elements - should not perform skip until first access
    ANVIterator skip_it = anv_iterator_skip(&range_it, &alloc, 3);

    // The skip should happen on first has_next() call
    ASSERT_TRUE(skip_it.has_next(&skip_it));

    // Should be positioned at element 4
    const int* val = skip_it.get(&skip_it);
    ASSERT_NOT_NULL(val);
    ASSERT_EQ(*val, 4);

    skip_it.destroy(&skip_it);
    return TEST_SUCCESS;
}

static int test_skip_unsupported_operations(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator 1-10
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 11, 1);

    // Skip first 5 elements
    ANVIterator skip_it = anv_iterator_skip(&range_it, &alloc, 5);

    // Test unsupported operations
    ASSERT_FALSE(skip_it.has_prev(&skip_it));
    ASSERT_EQ(skip_it.prev(&skip_it), -1);

    // Reset should be no-op (doesn't crash)
    skip_it.reset(&skip_it);

    skip_it.destroy(&skip_it);
    return TEST_SUCCESS;
}

//==============================================================================
// Take Iterator Tests
//==============================================================================

static int test_take_basic_functionality(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator 1-10
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 11, 1);

    // Take first 5 elements
    ANVIterator take_it = anv_iterator_take(&range_it, &alloc, 5);
    ASSERT_TRUE(take_it.is_valid(&take_it));

    int values[10];
    const int count = collect_values(&take_it, values, 10);

    ASSERT_EQ(count, 5);

    const int expected[] = {1, 2, 3, 4, 5};
    ASSERT_TRUE(verify_values(values, expected, 5, "take_basic"));

    take_it.destroy(&take_it);
    return TEST_SUCCESS;
}

static int test_take_zero_count(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator 1-10
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 11, 1);

    // Take 0 elements
    ANVIterator take_it = anv_iterator_take(&range_it, &alloc, 0);
    ASSERT_TRUE(take_it.is_valid(&take_it));

    // Should have no elements
    ASSERT_FALSE(take_it.has_next(&take_it));
    ASSERT_NULL(take_it.get(&take_it));

    take_it.destroy(&take_it);
    return TEST_SUCCESS;
}

static int test_take_more_than_available(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator with only 3 elements
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 4, 1);

    // Try to take 10 elements (more than available)
    ANVIterator take_it = anv_iterator_take(&range_it, &alloc, 10);
    ASSERT_TRUE(take_it.is_valid(&take_it));

    int values[10];
    const int count = collect_values(&take_it, values, 10);

    ASSERT_EQ(count, 3); // Should only get 3 elements

    const int expected[] = {1, 2, 3};
    ASSERT_TRUE(verify_values(values, expected, 3, "take_more_than_available"));

    take_it.destroy(&take_it);
    return TEST_SUCCESS;
}

static int test_take_single_element(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator 1-10
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 11, 1);

    // Take only 1 element
    ANVIterator take_it = anv_iterator_take(&range_it, &alloc, 1);
    ASSERT_TRUE(take_it.is_valid(&take_it));

    ASSERT_TRUE(take_it.has_next(&take_it));
    const int* value = take_it.get(&take_it);
    ASSERT_NOT_NULL(value);
    ASSERT_EQ(*value, 1);

    ASSERT_EQ(take_it.next(&take_it), 0);

    // Should be no more elements
    ASSERT_FALSE(take_it.has_next(&take_it));
    ASSERT_NULL(take_it.get(&take_it));

    take_it.destroy(&take_it);
    return TEST_SUCCESS;
}

static int test_take_empty_source(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create empty range iterator
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 1, 1); // Empty range

    // Try to take 5 elements from empty iterator
    ANVIterator take_it = anv_iterator_take(&range_it, &alloc, 5);
    ASSERT_TRUE(take_it.is_valid(&take_it));

    // Should have no elements
    ASSERT_FALSE(take_it.has_next(&take_it));
    ASSERT_NULL(take_it.get(&take_it));

    take_it.destroy(&take_it);
    return TEST_SUCCESS;
}

static int test_take_invalid_parameters(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Test with NULL iterator
    const ANVIterator take_it1 = anv_iterator_take(NULL, &alloc, 5);
    ASSERT_FALSE(take_it1.is_valid(&take_it1));

    // Test with NULL allocator
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 11, 1);

    const ANVIterator take_it2 = anv_iterator_take(&range_it, NULL, 5);
    ASSERT_FALSE(take_it2.is_valid(&take_it2));

    // Clean up the range iterator manually since take failed
    range_it.destroy(&range_it);

    return TEST_SUCCESS;
}

static int test_take_large_count(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator 1-5
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 6, 1);

    // Take very large number of elements
    ANVIterator take_it = anv_iterator_take(&range_it, &alloc, SIZE_MAX);
    ASSERT_TRUE(take_it.is_valid(&take_it));

    int values[10];
    const int count = collect_values(&take_it, values, 10);

    ASSERT_EQ(count, 5); // Should only get available elements

    const int expected[] = {1, 2, 3, 4, 5};
    ASSERT_TRUE(verify_values(values, expected, 5, "take_large_count"));

    take_it.destroy(&take_it);
    return TEST_SUCCESS;
}

static int test_take_with_filter(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range 1-10, filter evens, then take 2
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 11, 1);

    ANVIterator filter_it = anv_iterator_filter(&range_it, &alloc, is_even);

    ANVIterator take_it = anv_iterator_take(&filter_it, &alloc, 2);
    ASSERT_TRUE(take_it.is_valid(&take_it));

    int values[10];
    const int count = collect_values(&take_it, values, 10);

    ASSERT_EQ(count, 2);

    const int expected[] = {2, 4}; // First 2 even numbers
    ASSERT_TRUE(verify_values(values, expected, 2, "take_with_filter"));

    take_it.destroy(&take_it);
    return TEST_SUCCESS;
}

static int test_take_chained(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range 1-20, take 10, then take 3 from that
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 21, 1);

    ANVIterator take_it1 = anv_iterator_take(&range_it, &alloc, 10);

    ANVIterator take_it2 = anv_iterator_take(&take_it1, &alloc, 3);
    ASSERT_TRUE(take_it2.is_valid(&take_it2));

    int values[10];
    const int count = collect_values(&take_it2, values, 10);

    ASSERT_EQ(count, 3);

    const int expected[] = {1, 2, 3};
    ASSERT_TRUE(verify_values(values, expected, 3, "take_chained"));

    take_it2.destroy(&take_it2);
    return TEST_SUCCESS;
}

static int test_take_iteration_state(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator 1-10
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 11, 1);

    // Take first 3 elements
    ANVIterator take_it = anv_iterator_take(&range_it, &alloc, 3);

    // Test step-by-step iteration
    ASSERT_TRUE(take_it.has_next(&take_it));
    const int* val1 = take_it.get(&take_it);
    ASSERT_NOT_NULL(val1);
    ASSERT_EQ(*val1, 1);

    ASSERT_EQ(take_it.next(&take_it), 0);
    ASSERT_TRUE(take_it.has_next(&take_it));

    const int* val2 = take_it.get(&take_it);
    ASSERT_NOT_NULL(val2);
    ASSERT_EQ(*val2, 2);

    ASSERT_EQ(take_it.next(&take_it), 0);
    ASSERT_TRUE(take_it.has_next(&take_it));

    const int* val3 = take_it.get(&take_it);
    ASSERT_NOT_NULL(val3);
    ASSERT_EQ(*val3, 3);

    ASSERT_EQ(take_it.next(&take_it), 0);

    // Should be exhausted now
    ASSERT_FALSE(take_it.has_next(&take_it));
    ASSERT_NULL(take_it.get(&take_it));
    ASSERT_EQ(take_it.next(&take_it), -1);

    take_it.destroy(&take_it);
    return TEST_SUCCESS;
}

static int test_take_unsupported_operations(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator 1-10
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 11, 1);

    // Take first 5 elements
    ANVIterator take_it = anv_iterator_take(&range_it, &alloc, 5);

    // Test unsupported operations
    ASSERT_FALSE(take_it.has_prev(&take_it));
    ASSERT_EQ(take_it.prev(&take_it), -1);

    // Reset should be no-op (doesn't crash)
    take_it.reset(&take_it);

    take_it.destroy(&take_it);
    return TEST_SUCCESS;
}

//==============================================================================
// Transform Iterator Tests
//==============================================================================

/**
 * Test basic transform iterator functionality with double_value.
 */
static int test_transform_basic_double(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 5);
    ASSERT_NOT_NULL(list);

    // Create base iterator
    ANVIterator base_it = anv_dll_iterator(list);
    ASSERT_TRUE(base_it.is_valid(&base_it));
    ASSERT_TRUE(base_it.has_next(&base_it));

    // Create transform iterator that doubles each value
    ANVIterator transform_it = anv_iterator_transform(&base_it, &alloc, double_value, true);
    ASSERT_TRUE(transform_it.is_valid(&transform_it));
    ASSERT_TRUE(transform_it.has_next(&transform_it));

    // Expected: [1,2,3,4,5] -> [2,4,6,8,10]
    const int expected[] = {2, 4, 6, 8, 10};
    int actual[5];
    const int count = collect_values(&transform_it, actual, 5);

    ASSERT_EQ(count, 5);
    ASSERT_TRUE(verify_values(actual, expected, count, "basic_double"));

    // Iterator should be exhausted
    ASSERT_FALSE(transform_it.has_next(&transform_it));
    ASSERT_NULL(transform_it.get(&transform_it));

    // Cleanup
    transform_it.destroy(&transform_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test transform iterator with square function.
 */
static int test_transform_square(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 4);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator transform_it = anv_iterator_transform(&base_it, &alloc, square_func, true);

    ASSERT_TRUE(transform_it.is_valid(&transform_it));

    // Expected: [1,2,3,4] -> [1,4,9,16]
    const int expected[] = {1, 4, 9, 16};
    int actual[4];
    const int count = collect_values(&transform_it, actual, 4);

    ASSERT_EQ(count, 4);
    ASSERT_TRUE(verify_values(actual, expected, count, "square_transform"));

    transform_it.destroy(&transform_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test transform iterator with add_one function.
 */
static int test_transform_add_one(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 3);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator transform_it = anv_iterator_transform(&base_it, &alloc, add_one, true);

    ASSERT_TRUE(transform_it.is_valid(&transform_it));

    // Expected: [1,2,3] -> [2,3,4]
    const int expected[] = {2, 3, 4};
    int actual[3];
    const int count = collect_values(&transform_it, actual, 3);

    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_values(actual, expected, count, "add_one_transform"));

    transform_it.destroy(&transform_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test chaining transform iterators: double then add one.
 */
static int test_transform_chain_double_add_one(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 3);
    ASSERT_NOT_NULL(list);

    // Create base iterator
    ANVIterator base_it = anv_dll_iterator(list);

    // Chain transforms: first double, then add one
    ANVIterator double_it = anv_iterator_transform(&base_it, &alloc, double_value, true);
    ANVIterator add_one_it = anv_iterator_transform(&double_it, &alloc, add_one, true);

    ASSERT_TRUE(add_one_it.is_valid(&add_one_it));

    // Expected: [1,2,3] -> [2,4,6] -> [3,5,7]
    const int expected[] = {3, 5, 7};
    int actual[3];
    const int count = collect_values(&add_one_it, actual, 3);

    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_values(actual, expected, count, "chain_double_add_one"));

    add_one_it.destroy(&add_one_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test chaining transform iterators: square then multiply by 3.
 */
static int test_transform_chain_square_multiply(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 3);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator square_it = anv_iterator_transform(&base_it, &alloc, square_func, true);
    ANVIterator multiply_it = anv_iterator_transform(&square_it, &alloc, multiply_by_three, true);

    // Expected: [1,2,3] -> [1,4,9] -> [3,12,27]
    const int expected[] = {3, 12, 27};
    int actual[3];
    const int count = collect_values(&multiply_it, actual, 3);

    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_values(actual, expected, count, "chain_square_multiply"));

    multiply_it.destroy(&multiply_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test complex chaining: three transforms in sequence.
 */
static int test_transform_triple_chain(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 2);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator double_it = anv_iterator_transform(&base_it, &alloc, double_value, true);
    ANVIterator add_five_it = anv_iterator_transform(&double_it, &alloc, add_five, true);
    ANVIterator square_it = anv_iterator_transform(&add_five_it, &alloc, square_func, true);

    // Expected: [1,2] -> [2,4] -> [7,9] -> [49,81]
    const int expected[] = {49, 81};
    int actual[2];
    const int count = collect_values(&square_it, actual, 2);

    ASSERT_EQ(count, 2);
    ASSERT_TRUE(verify_values(actual, expected, count, "triple_chain"));

    square_it.destroy(&square_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test transform iterator with empty input.
 */
static int test_transform_empty_input(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = anv_dll_create(&alloc);
    ASSERT_NOT_NULL(list);

    // Create iterator on empty list
    ANVIterator base_it = anv_dll_iterator(list);
    ASSERT_FALSE(base_it.has_next(&base_it));

    // Create transform iterator
    ANVIterator transform_it = anv_iterator_transform(&base_it, &alloc, double_value, true);
    ASSERT_TRUE(transform_it.is_valid(&transform_it));
    ASSERT_FALSE(transform_it.has_next(&transform_it));
    ASSERT_NULL(transform_it.get(&transform_it));

    // Test that next() fails appropriately
    ASSERT_EQ(transform_it.next(&transform_it), -1);

    transform_it.destroy(&transform_it);
    anv_dll_destroy(list, false);
    return TEST_SUCCESS;
}

/**
 * Test transform iterator with single element.
 */
static int test_transform_single_element(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 1);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator transform_it = anv_iterator_transform(&base_it, &alloc, multiply_by_three, true);

    ASSERT_TRUE(transform_it.has_next(&transform_it));

    const int* value = transform_it.get(&transform_it);
    ASSERT_NOT_NULL(value);
    ASSERT_EQ(*value, 3); // 1 * 3 = 3

    transform_it.next(&transform_it);
    ASSERT_FALSE(transform_it.has_next(&transform_it));
    ASSERT_NULL(transform_it.get(&transform_it));

    transform_it.destroy(&transform_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test transform iterator with invalid inputs.
 */
static int test_transform_invalid_inputs(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 1);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);

    // Test with NULL iterator
    ANVIterator invalid_it1 = anv_iterator_transform(NULL, &alloc, double_value, true);
    ASSERT_FALSE(invalid_it1.is_valid(&invalid_it1));
    ASSERT_FALSE(invalid_it1.has_next(&invalid_it1));
    ASSERT_NULL(invalid_it1.get(&invalid_it1));

    // Test with NULL transform function
    ANVIterator invalid_it2 = anv_iterator_transform(&base_it, &alloc, NULL, true);
    ASSERT_FALSE(invalid_it2.is_valid(&invalid_it2));

    // Test with NULL allocator
    ANVIterator base_it2 = anv_dll_iterator(list);
    ANVIterator invalid_it3 = anv_iterator_transform(&base_it2, NULL, double_value, true);
    ASSERT_FALSE(invalid_it3.is_valid(&invalid_it3));

    // Cleanup
    invalid_it1.destroy(&invalid_it1);
    invalid_it2.destroy(&invalid_it2);
    invalid_it3.destroy(&invalid_it3);
    base_it2.destroy(&base_it2);
    base_it.destroy(&base_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test transform iterator operations on invalid iterator.
 */
static int test_transform_operations_on_invalid(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create invalid transform iterator
    ANVIterator invalid_it = anv_iterator_transform(NULL, &alloc, double_value, false);
    ASSERT_FALSE(invalid_it.is_valid(&invalid_it));

    // All operations should fail gracefully
    ASSERT_EQ(invalid_it.next(&invalid_it), -1);
    ASSERT_EQ(invalid_it.prev(&invalid_it), -1); // Transform doesn't support prev
    ASSERT_FALSE(invalid_it.has_next(&invalid_it));
    ASSERT_FALSE(invalid_it.has_prev(&invalid_it)); // Transform doesn't support has_prev
    ASSERT_NULL(invalid_it.get(&invalid_it));

    // Reset should be safe to call but ineffective
    invalid_it.reset(&invalid_it); // Should not crash

    invalid_it.destroy(&invalid_it);
    return TEST_SUCCESS;
}

/**
 * Test that get() doesn't advance, next() does advance.
 */
static int test_transform_get_next_separation(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 3);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator transform_it = anv_iterator_transform(&base_it, &alloc, add_ten_func, true);

    ASSERT_TRUE(transform_it.is_valid(&transform_it));

    // Test get without advancing
    const int* value1 = transform_it.get(&transform_it);
    ASSERT_NOT_NULL(value1);
    ASSERT_EQ(*value1, 11); // 1 + 10 = 11

    // Get again - should return same value
    const int* value2 = transform_it.get(&transform_it);
    ASSERT_NOT_NULL(value2);
    ASSERT_EQ(*value2, 11);

    // Store value before advancing
    const int first_value = *value1;

    // Now advance
    ASSERT_EQ(transform_it.next(&transform_it), 0);
    const int* value3 = transform_it.get(&transform_it);
    ASSERT_NOT_NULL(value3);
    ASSERT_EQ(*value3, 12); // 2 + 10 = 12
    ASSERT_NOT_EQ(first_value, *value3);

    transform_it.destroy(&transform_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test next() return codes.
 */
static int test_transform_next_return_codes(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 2);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator transform_it = anv_iterator_transform(&base_it, &alloc, double_value, true);

    ASSERT_TRUE(transform_it.is_valid(&transform_it));

    // First next should succeed
    ASSERT_EQ(transform_it.next(&transform_it), 0);

    // Second next should succeed
    ASSERT_EQ(transform_it.next(&transform_it), 0);

    // Third next should fail (at end)
    ASSERT_EQ(transform_it.next(&transform_it), -1);

    transform_it.destroy(&transform_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test unsupported bidirectional operations.
 */
static int test_transform_unsupported_operations(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 3);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator transform_it = anv_iterator_transform(&base_it, &alloc, double_value, true);

    ASSERT_TRUE(transform_it.is_valid(&transform_it));

    // Transform iterator should not support bidirectional operations
    ASSERT_FALSE(transform_it.has_prev(&transform_it));
    ASSERT_EQ(transform_it.prev(&transform_it), -1); // Returns -1 for unsupported

    // Reset should be safe but ineffective
    transform_it.reset(&transform_it);

    // Should still be valid after unsupported operations
    ASSERT_TRUE(transform_it.is_valid(&transform_it));

    transform_it.destroy(&transform_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test transform iterator with range iterator as input.
 */
static int test_transform_with_range_iterator(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator: [2, 4, 6, 8]
    ANVIterator range_it = anv_iterator_range(&alloc, 2, 10, 2);
    ASSERT_TRUE(range_it.is_valid(&range_it));

    // Apply square transform
    ANVIterator transform_it = anv_iterator_transform(&range_it, &alloc, square_func, true);
    ASSERT_TRUE(transform_it.is_valid(&transform_it));

    // Expected: [2,4,6,8] -> [4,16,36,64]
    const int expected[] = {4, 16, 36, 64};
    int actual[4];
    const int count = collect_values(&transform_it, actual, 4);

    ASSERT_EQ(count, 4);
    ASSERT_TRUE(verify_values(actual, expected, count, "transform_range"));

    transform_it.destroy(&transform_it);
    return TEST_SUCCESS;
}

/**
 * Test chaining range -> transform -> transform.
 */
static int test_range_transform_chain(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator: [1, 2, 3]
    ANVIterator range_it = anv_iterator_range(&alloc, 1, 4, 1);

    // Chain: range -> double -> add_five
    ANVIterator double_it = anv_iterator_transform(&range_it, &alloc, double_value, true);
    ANVIterator add_five_it = anv_iterator_transform(&double_it, &alloc, add_five, true);

    // Expected: [1,2,3] -> [2,4,6] -> [7,9,11]
    const int expected[] = {7, 9, 11};
    int actual[3];
    const int count = collect_values(&add_five_it, actual, 3);

    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_values(actual, expected, count, "range_transform_chain"));

    add_five_it.destroy(&add_five_it);
    return TEST_SUCCESS;
}

/**
 * Test memory consistency across operations.
 */
static int test_transform_memory_consistency(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 3);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator transform_it = anv_iterator_transform(&base_it, &alloc, multiply_by_three, true);

    // Get multiple references to the same value
    const int* ptr1 = transform_it.get(&transform_it);
    const int* ptr2 = transform_it.get(&transform_it);
    const int* ptr3 = transform_it.get(&transform_it);

    // All should point to the same memory and have the same value
    ASSERT_EQ(ptr1, ptr2);
    ASSERT_EQ(ptr2, ptr3);
    ASSERT_EQ(*ptr1, 3); // 1 * 3 = 3
    ASSERT_EQ(*ptr2, 3);
    ASSERT_EQ(*ptr3, 3);

    // Store the value before moving to next
    const int first_value = *ptr1;

    // Move to next and verify new value
    transform_it.next(&transform_it);
    const int* ptr4 = transform_it.get(&transform_it);
    ASSERT_EQ(*ptr4, 6); // 2 * 3 = 6

    // The transform iterator may use different memory for each value
    // This is normal behavior - what matters is the values are correct
    ASSERT_NOT_EQ(first_value, *ptr4); // Values should be different

    transform_it.destroy(&transform_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test that transform iterator properly manages base iterator lifecycle.
 */
static int test_transform_iterator_ownership(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 2);
    ASSERT_NOT_NULL(list);

    // Create base iterator
    ANVIterator base_it = anv_dll_iterator(list);
    ASSERT_TRUE(base_it.is_valid(&base_it));

    // Create transform iterator (takes ownership of base_it)
    ANVIterator transform_it = anv_iterator_transform(&base_it, &alloc, double_value, true);
    ASSERT_TRUE(transform_it.is_valid(&transform_it));

    // Verify transform works
    const int* value = transform_it.get(&transform_it);
    ASSERT_NOT_NULL(value);
    ASSERT_EQ(*value, 2); // 1 * 2 = 2

    // When we destroy transform iterator, it should clean up base iterator too
    transform_it.destroy(&transform_it);

    // Note: We should not access base_it after this point as it's been destroyed

    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test transform iterator with large dataset.
 */
static int test_transform_large_dataset(void)
{
    ANVAllocator alloc = create_int_allocator();
    const int SIZE = 1000;

    ANVDoublyLinkedList* list = create_test_list(&alloc, SIZE);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator transform_it = anv_iterator_transform(&base_it, &alloc, double_value, true);

    ASSERT_TRUE(transform_it.is_valid(&transform_it));

    int count = 0;
    int expected = 2; // First element (1) doubled

    while (transform_it.has_next(&transform_it))
    {
        const int* value = transform_it.get(&transform_it);
        ASSERT_NOT_NULL(value);
        ASSERT_EQ(*value, expected);

        count++;
        expected += 2; // Next expected value (each input increments by 1, doubled)
        transform_it.next(&transform_it);
    }

    ASSERT_EQ(count, SIZE);
    ASSERT_FALSE(transform_it.has_next(&transform_it));

    transform_it.destroy(&transform_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test deep chaining performance.
 */
static int test_transform_deep_chaining(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 10);
    ASSERT_NOT_NULL(list);

    // Create a deep chain of 5 transforms
    ANVIterator it1 = anv_dll_iterator(list);
    ANVIterator it2 = anv_iterator_transform(&it1, &alloc, add_one, true);
    ANVIterator it3 = anv_iterator_transform(&it2, &alloc, double_value, true);
    ANVIterator it4 = anv_iterator_transform(&it3, &alloc, add_five, true);
    ANVIterator it5 = anv_iterator_transform(&it4, &alloc, multiply_by_three, true);
    ANVIterator final_it = anv_iterator_transform(&it5, &alloc, square_func, true);

    ASSERT_TRUE(final_it.is_valid(&final_it));

    // Just verify it works end-to-end for first element
    // Input: 1 -> +1 -> *2 -> +5 -> *3 -> ^2
    // 1 -> 2 -> 4 -> 9 -> 27 -> 729
    const int* first_value = final_it.get(&final_it);
    ASSERT_NOT_NULL(first_value);
    ASSERT_EQ(*first_value, 729);

    // Verify we can iterate through all elements
    int count = 0;
    while (final_it.has_next(&final_it))
    {
        const int* value = final_it.get(&final_it);
        ASSERT_NOT_NULL(value);
        count++;
        final_it.next(&final_it);
    }

    ASSERT_EQ(count, 10);

    final_it.destroy(&final_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

/**
 * Test helper function validation with transform iterator.
 */
static int test_transform_helper_validation(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVDoublyLinkedList* list = create_test_list(&alloc, 5);
    ASSERT_NOT_NULL(list);

    ANVIterator base_it = anv_dll_iterator(list);
    ANVIterator transform_it = anv_iterator_transform(&base_it, &alloc, add_five, true);

    ASSERT_TRUE(transform_it.is_valid(&transform_it));

    int values[5];
    const int count = collect_values_with_validation(&transform_it, values, 5);

    // Should successfully collect all 5 values
    ASSERT_EQ(count, 5);

    const int expected[] = {6, 7, 8, 9, 10}; // [1,2,3,4,5] + 5 each
    ASSERT_TRUE(verify_values(values, expected, count, "helper_validation"));

    transform_it.destroy(&transform_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Zip Iterator Tests
//==============================================================================

static int test_zip_basic_functionality(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create two range iterators
    ANVIterator range1 = anv_iterator_range(&alloc, 1, 4, 1);   // [1,2,3]
    ANVIterator range2 = anv_iterator_range(&alloc, 10, 13, 1); // [10,11,12]

    // Zip them together
    ANVIterator zip_it = anv_iterator_zip(&range1, &range2, &alloc);
    ASSERT_TRUE(zip_it.is_valid(&zip_it));

    int first_values[10], second_values[10];
    const int count = collect_pairs(&zip_it, first_values, second_values, 10);

    ASSERT_EQ(count, 3);

    const int expected_first[] = {1, 2, 3};
    const int expected_second[] = {10, 11, 12};
    ASSERT_TRUE(verify_pairs(first_values, second_values, expected_first, expected_second, 3, "zip_basic"));

    zip_it.destroy(&zip_it);
    return TEST_SUCCESS;
}

static int test_zip_different_lengths_first_shorter(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // First iterator shorter: [1,2] vs [10,11,12,13]
    ANVIterator range1 = anv_iterator_range(&alloc, 1, 3, 1);   // [1,2]
    ANVIterator range2 = anv_iterator_range(&alloc, 10, 14, 1); // [10,11,12,13]

    ANVIterator zip_it = anv_iterator_zip(&range1, &range2, &alloc);
    ASSERT_TRUE(zip_it.is_valid(&zip_it));

    int first_values[10], second_values[10];
    const int count = collect_pairs(&zip_it, first_values, second_values, 10);

    ASSERT_EQ(count, 2); // Should stop when first iterator exhausted

    const int expected_first[] = {1, 2};
    const int expected_second[] = {10, 11};
    ASSERT_TRUE(verify_pairs(first_values, second_values, expected_first, expected_second, 2, "zip_first_shorter"));

    zip_it.destroy(&zip_it);
    return TEST_SUCCESS;
}

static int test_zip_different_lengths_second_shorter(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Second iterator shorter: [1,2,3,4] vs [10,11]
    ANVIterator range1 = anv_iterator_range(&alloc, 1, 5, 1);   // [1,2,3,4]
    ANVIterator range2 = anv_iterator_range(&alloc, 10, 12, 1); // [10,11]

    ANVIterator zip_it = anv_iterator_zip(&range1, &range2, &alloc);
    ASSERT_TRUE(zip_it.is_valid(&zip_it));

    int first_values[10], second_values[10];
    const int count = collect_pairs(&zip_it, first_values, second_values, 10);

    ASSERT_EQ(count, 2); // Should stop when second iterator exhausted

    const int expected_first[] = {1, 2};
    const int expected_second[] = {10, 11};
    ASSERT_TRUE(verify_pairs(first_values, second_values, expected_first, expected_second, 2, "zip_second_shorter"));

    zip_it.destroy(&zip_it);
    return TEST_SUCCESS;
}

static int test_zip_equal_length_single_elements(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Both iterators have single element
    ANVIterator range1 = anv_iterator_range(&alloc, 42, 43, 1);  // [42]
    ANVIterator range2 = anv_iterator_range(&alloc, 99, 100, 1); // [99]

    ANVIterator zip_it = anv_iterator_zip(&range1, &range2, &alloc);
    ASSERT_TRUE(zip_it.is_valid(&zip_it));

    ASSERT_TRUE(zip_it.has_next(&zip_it));
    const ANVPair* pair = zip_it.get(&zip_it);
    ASSERT_NOT_NULL(pair);
    ASSERT_NOT_NULL(pair->first);
    ASSERT_NOT_NULL(pair->second);
    ASSERT_EQ(*(const int*)pair->first, 42);
    ASSERT_EQ(*(const int*)pair->second, 99);

    ASSERT_EQ(zip_it.next(&zip_it), 0);
    ASSERT_FALSE(zip_it.has_next(&zip_it));
    ASSERT_NULL(zip_it.get(&zip_it));

    zip_it.destroy(&zip_it);
    return TEST_SUCCESS;
}

static int test_zip_both_empty(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Both iterators empty
    ANVIterator range1 = anv_iterator_range(&alloc, 1, 1, 1); // Empty
    ANVIterator range2 = anv_iterator_range(&alloc, 1, 1, 1); // Empty

    ANVIterator zip_it = anv_iterator_zip(&range1, &range2, &alloc);
    ASSERT_TRUE(zip_it.is_valid(&zip_it));

    // Should have no elements
    ASSERT_FALSE(zip_it.has_next(&zip_it));
    ASSERT_NULL(zip_it.get(&zip_it));

    zip_it.destroy(&zip_it);
    return TEST_SUCCESS;
}

static int test_zip_one_empty_first(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // First iterator empty, second has elements
    ANVIterator range1 = anv_iterator_range(&alloc, 1, 1, 1);   // Empty
    ANVIterator range2 = anv_iterator_range(&alloc, 10, 13, 1); // [10,11,12]

    ANVIterator zip_it = anv_iterator_zip(&range1, &range2, &alloc);
    ASSERT_TRUE(zip_it.is_valid(&zip_it));

    // Should have no elements
    ASSERT_FALSE(zip_it.has_next(&zip_it));
    ASSERT_NULL(zip_it.get(&zip_it));

    zip_it.destroy(&zip_it);
    return TEST_SUCCESS;
}

static int test_zip_one_empty_second(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // First iterator has elements, second empty
    ANVIterator range1 = anv_iterator_range(&alloc, 1, 4, 1); // [1,2,3]
    ANVIterator range2 = anv_iterator_range(&alloc, 1, 1, 1); // Empty

    ANVIterator zip_it = anv_iterator_zip(&range1, &range2, &alloc);
    ASSERT_TRUE(zip_it.is_valid(&zip_it));

    // Should have no elements
    ASSERT_FALSE(zip_it.has_next(&zip_it));
    ASSERT_NULL(zip_it.get(&zip_it));

    zip_it.destroy(&zip_it);
    return TEST_SUCCESS;
}

static int test_zip_invalid_parameters(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Test with NULL first iterator
    ANVIterator range2 = anv_iterator_range(&alloc, 1, 4, 1);
    const ANVIterator zip_it1 = anv_iterator_zip(NULL, &range2, &alloc);
    ASSERT_FALSE(zip_it1.is_valid(&zip_it1));
    range2.destroy(&range2); // Clean up since zip failed

    // Test with NULL second iterator
    ANVIterator range1 = anv_iterator_range(&alloc, 1, 4, 1);
    const ANVIterator zip_it2 = anv_iterator_zip(&range1, NULL, &alloc);
    ASSERT_FALSE(zip_it2.is_valid(&zip_it2));
    range1.destroy(&range1); // Clean up since zip failed

    // Test with NULL allocator
    ANVIterator range3 = anv_iterator_range(&alloc, 1, 4, 1);
    ANVIterator range4 = anv_iterator_range(&alloc, 1, 4, 1);
    const ANVIterator zip_it3 = anv_iterator_zip(&range3, &range4, NULL);
    ASSERT_FALSE(zip_it3.is_valid(&zip_it3));
    range3.destroy(&range3); // Clean up since zip failed
    range4.destroy(&range4); // Clean up since zip failed

    return TEST_SUCCESS;
}

static int test_zip_with_filter(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range 1-6, filter evens from first, zip with 10-15
    ANVIterator range1 = anv_iterator_range(&alloc, 1, 7, 1);              // [1,2,3,4,5,6]
    ANVIterator filter_it = anv_iterator_filter(&range1, &alloc, is_even); // [2,4,6]
    ANVIterator range2 = anv_iterator_range(&alloc, 10, 16, 1);            // [10,11,12,13,14,15]

    ANVIterator zip_it = anv_iterator_zip(&filter_it, &range2, &alloc);
    ASSERT_TRUE(zip_it.is_valid(&zip_it));

    int first_values[10], second_values[10];
    const int count = collect_pairs(&zip_it, first_values, second_values, 10);

    ASSERT_EQ(count, 3);

    const int expected_first[] = {2, 4, 6};
    const int expected_second[] = {10, 11, 12};
    ASSERT_TRUE(verify_pairs(first_values, second_values, expected_first, expected_second, 3, "zip_with_filter"));

    zip_it.destroy(&zip_it);
    return TEST_SUCCESS;
}

static int test_zip_with_take(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create ranges, take first 2 from each, then zip
    ANVIterator range1 = anv_iterator_range(&alloc, 1, 10, 1);    // [1,2,3,4,5,6,7,8,9]
    ANVIterator take_it1 = anv_iterator_take(&range1, &alloc, 2); // [1,2]

    ANVIterator range2 = anv_iterator_range(&alloc, 20, 30, 1);   // [20,21,22,23,24,25,26,27,28,29]
    ANVIterator take_it2 = anv_iterator_take(&range2, &alloc, 2); // [20,21]

    ANVIterator zip_it = anv_iterator_zip(&take_it1, &take_it2, &alloc);
    ASSERT_TRUE(zip_it.is_valid(&zip_it));

    int first_values[10], second_values[10];
    const int count = collect_pairs(&zip_it, first_values, second_values, 10);

    ASSERT_EQ(count, 2);

    const int expected_first[] = {1, 2};
    const int expected_second[] = {20, 21};
    ASSERT_TRUE(verify_pairs(first_values, second_values, expected_first, expected_second, 2, "zip_with_take"));

    zip_it.destroy(&zip_it);
    return TEST_SUCCESS;
}

static int test_zip_with_skip(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create ranges, skip first 2 from each, then zip
    ANVIterator range1 = anv_iterator_range(&alloc, 1, 6, 1);     // [1,2,3,4,5]
    ANVIterator skip_it1 = anv_iterator_skip(&range1, &alloc, 2); // [3,4,5]

    ANVIterator range2 = anv_iterator_range(&alloc, 10, 15, 1);   // [10,11,12,13,14]
    ANVIterator skip_it2 = anv_iterator_skip(&range2, &alloc, 2); // [12,13,14]

    ANVIterator zip_it = anv_iterator_zip(&skip_it1, &skip_it2, &alloc);
    ASSERT_TRUE(zip_it.is_valid(&zip_it));

    int first_values[10], second_values[10];
    const int count = collect_pairs(&zip_it, first_values, second_values, 10);

    ASSERT_EQ(count, 3);

    const int expected_first[] = {3, 4, 5};
    const int expected_second[] = {12, 13, 14};
    ASSERT_TRUE(verify_pairs(first_values, second_values, expected_first, expected_second, 3, "zip_with_skip"));

    zip_it.destroy(&zip_it);
    return TEST_SUCCESS;
}

static int test_zip_nested(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create nested zip: zip(1-3, 10-12) then zip that with 100-102
    ANVIterator range1 = anv_iterator_range(&alloc, 1, 4, 1);         // [1,2,3]
    ANVIterator range2 = anv_iterator_range(&alloc, 10, 13, 1);       // [10,11,12]
    ANVIterator zip_it1 = anv_iterator_zip(&range1, &range2, &alloc); // [(1,10),(2,11),(3,12)]

    ANVIterator range3 = anv_iterator_range(&alloc, 100, 103, 1); // [100,101,102]
    ANVIterator zip_it2 = anv_iterator_zip(&zip_it1, &range3, &alloc);

    ASSERT_TRUE(zip_it2.is_valid(&zip_it2));

    // Iterate and verify structure
    int count = 0;
    while (zip_it2.has_next(&zip_it2) && count < 3)
    {
        const ANVPair* outer_pair = zip_it2.get(&zip_it2);
        ASSERT_NOT_NULL(outer_pair);
        ASSERT_NOT_NULL(outer_pair->first);  // Should be a ANVPair*
        ASSERT_NOT_NULL(outer_pair->second); // Should be an int*

        const ANVPair* inner_pair = outer_pair->first;
        const int* third_value = outer_pair->second;

        ASSERT_NOT_NULL(inner_pair->first);
        ASSERT_NOT_NULL(inner_pair->second);

        const int first_val = *(const int*)inner_pair->first;
        const int second_val = *(const int*)inner_pair->second;
        const int third_val = *third_value;

        // Verify expected values
        ASSERT_EQ(first_val, 1 + count);
        ASSERT_EQ(second_val, 10 + count);
        ASSERT_EQ(third_val, 100 + count);

        zip_it2.next(&zip_it2);
        count++;
    }

    ASSERT_EQ(count, 3);
    ASSERT_FALSE(zip_it2.has_next(&zip_it2));

    zip_it2.destroy(&zip_it2);
    return TEST_SUCCESS;
}

static int test_zip_iteration_state(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create zip iterator and test step-by-step iteration
    ANVIterator range1 = anv_iterator_range(&alloc, 100, 103, 1); // [100,101,102]
    ANVIterator range2 = anv_iterator_range(&alloc, 200, 203, 1); // [200,201,202]

    ANVIterator zip_it = anv_iterator_zip(&range1, &range2, &alloc);

    // Test step-by-step iteration
    ASSERT_TRUE(zip_it.has_next(&zip_it));
    const ANVPair* pair1 = zip_it.get(&zip_it);
    ASSERT_NOT_NULL(pair1);
    ASSERT_EQ(*(const int*)pair1->first, 100);
    ASSERT_EQ(*(const int*)pair1->second, 200);

    ASSERT_EQ(zip_it.next(&zip_it), 0);
    ASSERT_TRUE(zip_it.has_next(&zip_it));

    const ANVPair* pair2 = zip_it.get(&zip_it);
    ASSERT_NOT_NULL(pair2);
    ASSERT_EQ(*(const int*)pair2->first, 101);
    ASSERT_EQ(*(const int*)pair2->second, 201);

    ASSERT_EQ(zip_it.next(&zip_it), 0);
    ASSERT_TRUE(zip_it.has_next(&zip_it));

    const ANVPair* pair3 = zip_it.get(&zip_it);
    ASSERT_NOT_NULL(pair3);
    ASSERT_EQ(*(const int*)pair3->first, 102);
    ASSERT_EQ(*(const int*)pair3->second, 202);

    ASSERT_EQ(zip_it.next(&zip_it), 0);
    ASSERT_FALSE(zip_it.has_next(&zip_it));
    ASSERT_NULL(zip_it.get(&zip_it));
    ASSERT_EQ(zip_it.next(&zip_it), -1);

    zip_it.destroy(&zip_it);
    return TEST_SUCCESS;
}

static int test_zip_unsupported_operations(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create zip iterator
    ANVIterator range1 = anv_iterator_range(&alloc, 1, 4, 1);
    ANVIterator range2 = anv_iterator_range(&alloc, 10, 13, 1);
    ANVIterator zip_it = anv_iterator_zip(&range1, &range2, &alloc);

    // Test unsupported operations
    ASSERT_FALSE(zip_it.has_prev(&zip_it));
    ASSERT_EQ(zip_it.prev(&zip_it), -1);

    // Reset should be no-op (doesn't crash)
    zip_it.reset(&zip_it);

    zip_it.destroy(&zip_it);
    return TEST_SUCCESS;
}

static int test_zip_pair_consistency(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Test that the same pair pointer is returned for multiple get() calls
    ANVIterator range1 = anv_iterator_range(&alloc, 1, 3, 1);   // [1,2]
    ANVIterator range2 = anv_iterator_range(&alloc, 10, 12, 1); // [10,11]

    ANVIterator zip_it = anv_iterator_zip(&range1, &range2, &alloc);

    ASSERT_TRUE(zip_it.has_next(&zip_it));

    const ANVPair* pair1 = zip_it.get(&zip_it);
    const ANVPair* pair2 = zip_it.get(&zip_it);

    // Should return same pointer (cached pair)
    ASSERT_EQ(pair1, pair2);
    ASSERT_EQ(*(const int*)pair1->first, 1);
    ASSERT_EQ(*(const int*)pair1->second, 10);

    // After next(), should get different values but could be same pointer
    zip_it.next(&zip_it);
    const ANVPair* pair3 = zip_it.get(&zip_it);
    ASSERT_EQ(*(const int*)pair3->first, 2);
    ASSERT_EQ(*(const int*)pair3->second, 11);

    zip_it.destroy(&zip_it);
    return TEST_SUCCESS;
}

static int test_zip_arraylist_iterators(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create two ArrayLists with different data
    ANVArrayList* list1 = anv_arraylist_create(&alloc, 0);
    ANVArrayList* list2 = anv_arraylist_create(&alloc, 0);

    // Populate list1 with 1, 2, 3
    for (int i = 1; i <= 3; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_arraylist_push_back(list1, val);
    }

    // Populate list2 with 10, 20, 30
    for (int i = 1; i <= 3; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i * 10;
        anv_arraylist_push_back(list2, val);
    }

    // Create iterators
    ANVIterator iter1 = anv_arraylist_iterator(list1);
    ANVIterator iter2 = anv_arraylist_iterator(list2);

    // Zip the iterators
    ANVIterator zip_it = anv_iterator_zip(&iter1, &iter2, &alloc);
    ASSERT_TRUE(zip_it.is_valid(&zip_it));

    int first_values[10], second_values[10];
    const int count = collect_pairs(&zip_it, first_values, second_values, 10);

    ASSERT_EQ(count, 3);

    const int expected_first[] = {1, 2, 3};
    const int expected_second[] = {10, 20, 30};
    ASSERT_TRUE(verify_pairs(first_values, second_values, expected_first, expected_second, 3, "zip_arraylist"));

    zip_it.destroy(&zip_it);
    anv_arraylist_destroy(list1, true);
    anv_arraylist_destroy(list2, true);
    return TEST_SUCCESS;
}

static int test_zip_dll_iterators(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create two DoublyLinkedLists
    ANVDoublyLinkedList* list1 = anv_dll_create(&alloc);
    ANVDoublyLinkedList* list2 = anv_dll_create(&alloc);

    // Populate list1 with 5, 6, 7, 8
    for (int i = 5; i <= 8; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_dll_push_back(list1, val);
    }

    // Populate list2 with 50, 60 (shorter list)
    for (int i = 5; i <= 6; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i * 10;
        anv_dll_push_back(list2, val);
    }

    // Create iterators
    ANVIterator iter1 = anv_dll_iterator(list1);
    ANVIterator iter2 = anv_dll_iterator(list2);

    // Zip the iterators
    ANVIterator zip_it = anv_iterator_zip(&iter1, &iter2, &alloc);
    ASSERT_TRUE(zip_it.is_valid(&zip_it));

    int first_values[10], second_values[10];
    const int count = collect_pairs(&zip_it, first_values, second_values, 10);

    ASSERT_EQ(count, 2); // Limited by shorter list

    const int expected_first[] = {5, 6};
    const int expected_second[] = {50, 60};
    ASSERT_TRUE(verify_pairs(first_values, second_values, expected_first, expected_second, 2, "zip_dll"));

    zip_it.destroy(&zip_it);
    anv_dll_destroy(list1, true);
    anv_dll_destroy(list2, true);
    return TEST_SUCCESS;
}

static int test_zip_arraylist_with_dll(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create ArrayList and DoublyLinkedList
    ANVArrayList* arraylist = anv_arraylist_create(&alloc, 0);
    ANVDoublyLinkedList* dll = anv_dll_create(&alloc);

    // Populate ArrayList with 100, 200, 300, 400
    for (int i = 1; i <= 4; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i * 100;
        anv_arraylist_push_back(arraylist, val);
    }

    // Populate DLL with 1, 2, 3
    for (int i = 1; i <= 3; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_dll_push_back(dll, val);
    }

    // Create iterators
    ANVIterator array_iter = anv_arraylist_iterator(arraylist);
    ANVIterator dll_iter = anv_dll_iterator(dll);

    // Zip different data structure iterators
    ANVIterator zip_it = anv_iterator_zip(&array_iter, &dll_iter, &alloc);
    ASSERT_TRUE(zip_it.is_valid(&zip_it));

    int first_values[10], second_values[10];
    const int count = collect_pairs(&zip_it, first_values, second_values, 10);

    ASSERT_EQ(count, 3); // Limited by DLL

    const int expected_first[] = {100, 200, 300};
    const int expected_second[] = {1, 2, 3};
    ASSERT_TRUE(verify_pairs(first_values, second_values, expected_first, expected_second, 3, "zip_arraylist_dll"));

    zip_it.destroy(&zip_it);
    anv_arraylist_destroy(arraylist, true);
    anv_dll_destroy(dll, true);
    return TEST_SUCCESS;
}

static int test_zip_queue_with_stack(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create Queue and Stack
    ANVQueue* queue = anv_queue_create(&alloc);
    ANVStack* stack = anv_stack_create(&alloc);

    // Populate queue with 1, 2, 3, 4 (FIFO: will iterate as 1, 2, 3, 4)
    for (int i = 1; i <= 4; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_queue_enqueue(queue, val);
    }

    // Populate stack with 10, 20, 30, 40 (LIFO: will iterate as 40, 30, 20, 10)
    for (int i = 1; i <= 4; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i * 10;
        anv_stack_push(stack, val);
    }

    // Create iterators
    ANVIterator queue_iter = anv_queue_iterator(queue);
    ANVIterator stack_iter = anv_stack_iterator(stack);

    // Zip queue and stack iterators
    ANVIterator zip_it = anv_iterator_zip(&queue_iter, &stack_iter, &alloc);
    ASSERT_TRUE(zip_it.is_valid(&zip_it));

    int first_values[10], second_values[10];
    const int count = collect_pairs(&zip_it, first_values, second_values, 10);

    ASSERT_EQ(count, 4);

    // Queue: FIFO order (1, 2, 3, 4), Stack: LIFO order (40, 30, 20, 10)
    const int expected_first[] = {1, 2, 3, 4};
    const int expected_second[] = {40, 30, 20, 10};
    ASSERT_TRUE(verify_pairs(first_values, second_values, expected_first, expected_second, 4, "zip_queue_stack"));

    zip_it.destroy(&zip_it);
    anv_queue_destroy(queue, true);
    anv_stack_destroy(stack, true);
    return TEST_SUCCESS;
}

static int test_zip_range_with_arraylist(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create range iterator and ArrayList
    ANVIterator range_iter = anv_iterator_range(&alloc, 100, 105, 1); // [100, 101, 102, 103, 104]

    ANVArrayList* list = anv_arraylist_create(&alloc, 0);
    // Populate with characters as integers: 'a', 'b', 'c'
    for (int i = 0; i < 3; i++)
    {
        int* val = malloc(sizeof(int));
        *val = 'a' + i;
        anv_arraylist_push_back(list, val);
    }

    ANVIterator array_iter = anv_arraylist_iterator(list);

    // Zip range with ArrayList
    ANVIterator zip_it = anv_iterator_zip(&range_iter, &array_iter, &alloc);
    ASSERT_TRUE(zip_it.is_valid(&zip_it));

    int first_values[10], second_values[10];
    const int count = collect_pairs(&zip_it, first_values, second_values, 10);

    ASSERT_EQ(count, 3); // Limited by ArrayList

    const int expected_first[] = {100, 101, 102};
    const int expected_second[] = {'a', 'b', 'c'};
    ASSERT_TRUE(verify_pairs(first_values, second_values, expected_first, expected_second, 3, "zip_range_arraylist"));

    zip_it.destroy(&zip_it);
    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

static int test_zip_filtered_data_structures(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create ArrayList with numbers 1-10
    ANVArrayList* list1 = anv_arraylist_create(&alloc, 0);
    for (int i = 1; i <= 10; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_arraylist_push_back(list1, val);
    }

    // Create DLL with numbers 11-20
    ANVDoublyLinkedList* list2 = anv_dll_create(&alloc);
    for (int i = 11; i <= 20; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i;
        anv_dll_push_back(list2, val);
    }

    // Create iterators and filter them
    ANVIterator array_iter = anv_arraylist_iterator(list1);
    ANVIterator filtered_array = anv_iterator_filter(&array_iter, &alloc, is_even); // [2, 4, 6, 8, 10]

    ANVIterator dll_iter = anv_dll_iterator(list2);
    ANVIterator filtered_dll = anv_iterator_filter(&dll_iter, &alloc, is_odd); // [11, 13, 15, 17, 19]

    // Zip the filtered iterators
    ANVIterator zip_it = anv_iterator_zip(&filtered_array, &filtered_dll, &alloc);
    ASSERT_TRUE(zip_it.is_valid(&zip_it));

    int first_values[10], second_values[10];
    const int count = collect_pairs(&zip_it, first_values, second_values, 10);

    ASSERT_EQ(count, 5);

    const int expected_first[] = {2, 4, 6, 8, 10};
    const int expected_second[] = {11, 13, 15, 17, 19};
    ASSERT_TRUE(verify_pairs(first_values, second_values, expected_first, expected_second, 5, "zip_filtered_structures"));

    zip_it.destroy(&zip_it);
    anv_arraylist_destroy(list1, true);
    anv_dll_destroy(list2, true);
    return TEST_SUCCESS;
}

static int test_zip_complex_composition(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create a complex composition: Range -> Skip -> Take vs ArrayList -> Filter
    ANVIterator range_iter = anv_iterator_range(&alloc, 1, 20, 1);     // [1..19]
    ANVIterator skip_iter = anv_iterator_skip(&range_iter, &alloc, 5); // [6..19]
    ANVIterator take_iter = anv_iterator_take(&skip_iter, &alloc, 3);  // [6, 7, 8]

    // Create ArrayList with multiples of 2
    ANVArrayList* list = anv_arraylist_create(&alloc, 0);
    for (int i = 1; i <= 10; i++)
    {
        int* val = malloc(sizeof(int));
        *val = i * 2; // [2, 4, 6, 8, 10, 12, 14, 16, 18, 20]
        anv_arraylist_push_back(list, val);
    }

    ANVIterator array_iter = anv_arraylist_iterator(list);
    ANVIterator filtered_array = anv_iterator_filter(&array_iter, &alloc, is_greater_than_10); // [12, 14, 16, 18, 20]

    // Zip the complex compositions
    ANVIterator zip_it = anv_iterator_zip(&take_iter, &filtered_array, &alloc);
    ASSERT_TRUE(zip_it.is_valid(&zip_it));

    int first_values[10], second_values[10];
    const int count = collect_pairs(&zip_it, first_values, second_values, 10);

    ASSERT_EQ(count, 3); // Limited by take_iter

    const int expected_first[] = {6, 7, 8};
    const int expected_second[] = {12, 14, 16};
    ASSERT_TRUE(verify_pairs(first_values, second_values, expected_first, expected_second, 3, "zip_complex_composition"));

    zip_it.destroy(&zip_it);
    anv_arraylist_destroy(list, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Main
//==============================================================================

int main(void)
{
    const ANVTestCase tests[] = {
        // Chain Iterator Tests
        TEST_REGISTER(test_chain_basic_functionality),
        TEST_REGISTER(test_chain_single_iterator),
        TEST_REGISTER(test_chain_empty_iterators),
        TEST_REGISTER(test_chain_mixed_empty_and_non_empty),
        TEST_REGISTER(test_chain_multiple_iterators),
        TEST_REGISTER(test_chain_with_repeat_iterators),
        TEST_REGISTER(test_chain_with_take_skip_iterators),
        TEST_REGISTER(test_chain_invalid_parameters),
        TEST_REGISTER(test_chain_iterator_operations),
        TEST_REGISTER(test_chain_with_nested_chains),
        TEST_REGISTER(test_chain_with_arraylist_and_dll),
        TEST_REGISTER(test_chain_with_stack_queue_and_hashset),
        TEST_REGISTER(test_chain_with_complex_transformations),
        TEST_REGISTER(test_chain_with_zip_and_data_structures),
        TEST_REGISTER(test_chain_data_structure_round_trip),

        // Iterator Chaining Tests
        TEST_REGISTER(test_range_filter_even),
        TEST_REGISTER(test_range_step_filter_div3),
        TEST_REGISTER(test_range_filter_greater_than_5),
        TEST_REGISTER(test_range_transform_double),
        TEST_REGISTER(test_range_transform_square),
        TEST_REGISTER(test_range_transform_add_ten),
        TEST_REGISTER(test_filter_transform_even_double),
        TEST_REGISTER(test_filter_transform_odd_square),
        TEST_REGISTER(test_filter_transform_no_matches),
        TEST_REGISTER(test_transform_filter_add_one_even),
        TEST_REGISTER(test_transform_filter_square_gt10),
        TEST_REGISTER(test_transform_filter_multiply3_div6),
        TEST_REGISTER(test_range_filter_transform_chain),
        TEST_REGISTER(test_range_transform_filter_chain),
        TEST_REGISTER(test_range_filter_transform_filter_chain),
        TEST_REGISTER(test_range_transform_transform_filter_chain),
        TEST_REGISTER(test_deep_nested_chain),
        TEST_REGISTER(test_empty_chain_propagation),
        TEST_REGISTER(test_single_element_chain),
        TEST_REGISTER(test_chain_invalid_intermediate),
        TEST_REGISTER(test_list_complex_chain),
        TEST_REGISTER(test_list_multiple_filters),
        TEST_REGISTER(test_list_multiple_transforms),
        TEST_REGISTER(test_chain_memory_consistency),
        TEST_REGISTER(test_chain_ownership_cleanup),
        TEST_REGISTER(test_chain_performance),
        TEST_REGISTER(test_chain_helper_validation),

        // Copy Iterator Tests
        TEST_REGISTER(test_copy_basic_integers),
        TEST_REGISTER(test_copy_single_element),
        TEST_REGISTER(test_copy_custom_structure),
        TEST_REGISTER(test_copy_empty_input),
        TEST_REGISTER(test_copy_large_dataset),
        TEST_REGISTER(test_copy_invalid_inputs),
        TEST_REGISTER(test_copy_operations_on_invalid),
        TEST_REGISTER(test_copy_get_next_separation),
        TEST_REGISTER(test_copy_next_return_codes),
        TEST_REGISTER(test_copy_unsupported_operations),
        TEST_REGISTER(test_copy_with_range_iterator),
        TEST_REGISTER(test_range_copy_chain),
        TEST_REGISTER(test_copy_memory_ownership),
        TEST_REGISTER(test_copy_iterator_ownership),
        TEST_REGISTER(test_copy_memory_consistency),
        TEST_REGISTER(test_filter_copy_chain),
        TEST_REGISTER(test_transform_copy_chain),
        TEST_REGISTER(test_complex_chain_with_copy),
        TEST_REGISTER(test_copy_strings),
        TEST_REGISTER(test_copy_performance),
        TEST_REGISTER(test_copy_helper_validation),

        // Enumerate Iterator Tests
        TEST_REGISTER(test_enumerate_basic_functionality),
        TEST_REGISTER(test_enumerate_custom_start_index),
        TEST_REGISTER(test_enumerate_single_element),
        TEST_REGISTER(test_enumerate_large_start_index),
        TEST_REGISTER(test_enumerate_empty_source),
        TEST_REGISTER(test_enumerate_invalid_parameters),
        TEST_REGISTER(test_enumerate_with_filter),
        TEST_REGISTER(test_enumerate_with_take),
        TEST_REGISTER(test_enumerate_with_skip),
        TEST_REGISTER(test_enumerate_chained),
        TEST_REGISTER(test_enumerate_arraylist),
        TEST_REGISTER(test_enumerate_dll),
        TEST_REGISTER(test_enumerate_queue_with_stack),
        TEST_REGISTER(test_enumerate_complex_composition),
        TEST_REGISTER(test_enumerate_iteration_state),
        TEST_REGISTER(test_enumerate_unsupported_operations),
        TEST_REGISTER(test_enumerate_element_consistency),
        TEST_REGISTER(test_enumerate_index_overflow_behavior),

        // Filter Iterator Tests
        TEST_REGISTER(test_filter_basic_even),
        TEST_REGISTER(test_filter_odd),
        TEST_REGISTER(test_filter_greater_than_five),
        TEST_REGISTER(test_filter_divisible_by_3),
        TEST_REGISTER(test_filter_empty_input),
        TEST_REGISTER(test_filter_no_matches),
        TEST_REGISTER(test_filter_all_matches),
        TEST_REGISTER(test_filter_single_element),
        TEST_REGISTER(test_filter_invalid_inputs),
        TEST_REGISTER(test_filter_operations_on_invalid),
        TEST_REGISTER(test_filter_get_next_separation),
        TEST_REGISTER(test_filter_next_return_codes),
        TEST_REGISTER(test_filter_unsupported_operations),
        TEST_REGISTER(test_multiple_filter_chain_even_div3),
        TEST_REGISTER(test_multiple_filter_chain_div4_gt10),
        TEST_REGISTER(test_triple_filter_chain),
        TEST_REGISTER(test_filter_chain_no_matches),
        TEST_REGISTER(test_filter_chain_single_match),
        TEST_REGISTER(test_filter_memory_consistency),
        TEST_REGISTER(test_filter_iterator_ownership),
        TEST_REGISTER(test_filter_chain_memory_management),
        TEST_REGISTER(test_filter_large_dataset),
        TEST_REGISTER(test_filter_complex_chaining),
        TEST_REGISTER(test_filter_helper_validation),

        // Range Iterator Tests
        TEST_REGISTER(test_range_positive_step),
        TEST_REGISTER(test_range_negative_step),
        TEST_REGISTER(test_range_larger_step),
        TEST_REGISTER(test_range_negative_step_size),
        TEST_REGISTER(test_range_empty),
        TEST_REGISTER(test_single_element_range),
        TEST_REGISTER(test_range_extreme_values),
        TEST_REGISTER(test_range_invalid_step),
        TEST_REGISTER(test_invalid_allocator),
        TEST_REGISTER(test_range_stress),
        TEST_REGISTER(test_range_reset),
        TEST_REGISTER(test_reset_after_bidirectional),
        TEST_REGISTER(test_zigzag_compensation),
        TEST_REGISTER(test_bidirectional_boundaries),
        TEST_REGISTER(test_direction_change_compensation),
        TEST_REGISTER(test_start_boundary_behavior),
        TEST_REGISTER(test_get_next_separation),
        TEST_REGISTER(test_next_return_codes),
        TEST_REGISTER(test_prev_return_codes),
        TEST_REGISTER(test_memory_consistency),
        TEST_REGISTER(test_has_prev_at_start),
        TEST_REGISTER(test_has_next_at_end),
        TEST_REGISTER(test_bidirectional_with_large_steps),
        TEST_REGISTER(test_negative_step_boundaries),
        TEST_REGISTER(test_operations_on_invalid_iterator),
        TEST_REGISTER(test_reset_after_boundary_errors),
        TEST_REGISTER(test_concurrent_get_calls_during_movement),
        TEST_REGISTER(test_single_step_boundaries),
        TEST_REGISTER(test_helper_function_validation),

        // Repeat Iterator Tests
        TEST_REGISTER(test_repeat_basic_functionality),
        TEST_REGISTER(test_repeat_single_count),
        TEST_REGISTER(test_repeat_zero_count),
        TEST_REGISTER(test_repeat_large_count),
        TEST_REGISTER(test_repeat_different_data_types),
        TEST_REGISTER(test_repeat_invalid_parameters),
        TEST_REGISTER(test_repeat_pointer_consistency),
        TEST_REGISTER(test_repeat_exhausted_iterator),
        TEST_REGISTER(test_repeat_reset_functionality),
        TEST_REGISTER(test_repeat_reset_exhausted),
        TEST_REGISTER(test_repeat_reset_empty),
        TEST_REGISTER(test_repeat_with_filter),
        TEST_REGISTER(test_repeat_with_filter_reject),
        TEST_REGISTER(test_repeat_with_take),
        TEST_REGISTER(test_repeat_with_take_more_than_available),
        TEST_REGISTER(test_repeat_with_skip),
        TEST_REGISTER(test_repeat_with_skip_all),
        TEST_REGISTER(test_repeat_with_zip),
        TEST_REGISTER(test_repeat_with_enumerate),
        TEST_REGISTER(test_repeat_chained_operations),
        TEST_REGISTER(test_repeat_with_arraylist),
        TEST_REGISTER(test_repeat_with_range),
        TEST_REGISTER(test_repeat_iteration_state),
        TEST_REGISTER(test_repeat_unsupported_operations),

        // Skip Iterator Tests
        TEST_REGISTER(test_skip_basic_functionality),
        TEST_REGISTER(test_skip_zero_count),
        TEST_REGISTER(test_skip_more_than_available),
        TEST_REGISTER(test_skip_all_elements),
        TEST_REGISTER(test_skip_single_element),
        TEST_REGISTER(test_skip_empty_source),
        TEST_REGISTER(test_skip_invalid_parameters),
        TEST_REGISTER(test_skip_large_count),
        TEST_REGISTER(test_skip_with_filter),
        TEST_REGISTER(test_skip_chained),
        TEST_REGISTER(test_skip_with_take),
        TEST_REGISTER(test_skip_iteration_state),
        TEST_REGISTER(test_skip_lazy_evaluation),
        TEST_REGISTER(test_skip_unsupported_operations),

        // Take Iterator Tests
        TEST_REGISTER(test_take_basic_functionality),
        TEST_REGISTER(test_take_zero_count),
        TEST_REGISTER(test_take_more_than_available),
        TEST_REGISTER(test_take_single_element),
        TEST_REGISTER(test_take_empty_source),
        TEST_REGISTER(test_take_invalid_parameters),
        TEST_REGISTER(test_take_large_count),
        TEST_REGISTER(test_take_with_filter),
        TEST_REGISTER(test_take_chained),
        TEST_REGISTER(test_take_iteration_state),
        TEST_REGISTER(test_take_unsupported_operations),

        // Transform Iterator Tests
        TEST_REGISTER(test_transform_basic_double),
        TEST_REGISTER(test_transform_square),
        TEST_REGISTER(test_transform_add_one),
        TEST_REGISTER(test_transform_chain_double_add_one),
        TEST_REGISTER(test_transform_chain_square_multiply),
        TEST_REGISTER(test_transform_triple_chain),
        TEST_REGISTER(test_transform_empty_input),
        TEST_REGISTER(test_transform_single_element),
        TEST_REGISTER(test_transform_invalid_inputs),
        TEST_REGISTER(test_transform_operations_on_invalid),
        TEST_REGISTER(test_transform_get_next_separation),
        TEST_REGISTER(test_transform_next_return_codes),
        TEST_REGISTER(test_transform_unsupported_operations),
        TEST_REGISTER(test_transform_with_range_iterator),
        TEST_REGISTER(test_range_transform_chain),
        TEST_REGISTER(test_transform_memory_consistency),
        TEST_REGISTER(test_transform_iterator_ownership),
        TEST_REGISTER(test_transform_large_dataset),
        TEST_REGISTER(test_transform_deep_chaining),
        TEST_REGISTER(test_transform_helper_validation),

        // Zip Iterator Tests
        TEST_REGISTER(test_zip_basic_functionality),
        TEST_REGISTER(test_zip_different_lengths_first_shorter),
        TEST_REGISTER(test_zip_different_lengths_second_shorter),
        TEST_REGISTER(test_zip_equal_length_single_elements),
        TEST_REGISTER(test_zip_both_empty),
        TEST_REGISTER(test_zip_one_empty_first),
        TEST_REGISTER(test_zip_one_empty_second),
        TEST_REGISTER(test_zip_invalid_parameters),
        TEST_REGISTER(test_zip_with_filter),
        TEST_REGISTER(test_zip_with_take),
        TEST_REGISTER(test_zip_with_skip),
        TEST_REGISTER(test_zip_nested),
        TEST_REGISTER(test_zip_iteration_state),
        TEST_REGISTER(test_zip_unsupported_operations),
        TEST_REGISTER(test_zip_pair_consistency),
        TEST_REGISTER(test_zip_arraylist_iterators),
        TEST_REGISTER(test_zip_dll_iterators),
        TEST_REGISTER(test_zip_arraylist_with_dll),
        TEST_REGISTER(test_zip_queue_with_stack),
        TEST_REGISTER(test_zip_range_with_arraylist),
        TEST_REGISTER(test_zip_filtered_data_structures),
        TEST_REGISTER(test_zip_complex_composition),

    };

    return anv_run_tests("Iterator", tests, sizeof(tests) / sizeof(tests[0]));
}