//
// Created by zack on 9/13/25.
//
// Comprehensive test suite for copy iterator functionality.
// Tests cover basic copying, error handling, memory management,
// and integration with different data structures and iterator types.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "containers/doublylinkedlist.h"
#include "containers/iterator.h"
#include "TestAssert.h"
#include "TestHelpers.h"

//==============================================================================
// Helper Functions for Copy Iterator Tests
//==============================================================================

/**
 * Helper function to collect all values from an iterator into an array.
 * Returns the number of values collected.
 * Note: For copy iterator, the user owns the returned values and must free them.
 */
static int collect_values(const ANVIterator* it, int** values, const int max_count)
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
 * Helper function to verify an array matches expected values.
 */
static int verify_values(int** actual, const int* expected, const int count, const char* test_name)
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
 * Helper function to create a test list with integers 1 through n.
 */
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

//==============================================================================
// Basic Copy Iterator Tests
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
    const int count = collect_values(&copy_it, actual, 5);

    ASSERT_EQ(count, 5);
    ASSERT_TRUE(verify_values(actual, expected, count, "basic_copy"));

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

//==============================================================================
// Edge Case Tests
//==============================================================================

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

//==============================================================================
// Error Handling Tests
//==============================================================================

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

//==============================================================================
// API Behavior Tests
//==============================================================================

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

//==============================================================================
// Integration Tests with Range Iterator
//==============================================================================

/**
 * Test copy iterator with range iterator as input.
 */
static int test_copy_with_range_iterator(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Create range iterator: [1, 2, 3, 4, 5]
    ANVIterator range_it = anv_iterator_range(1, 6, 1, &alloc);
    ASSERT_TRUE(range_it.is_valid(&range_it));

    // Apply copy
    ANVIterator copy_it = anv_iterator_copy(&range_it, &alloc, int_copy);
    ASSERT_TRUE(copy_it.is_valid(&copy_it));

    // Expected: [1,2,3,4,5] copied to new memory locations
    const int expected[] = {1, 2, 3, 4, 5};
    int* actual[5];
    const int count = collect_values(&copy_it, actual, 5);

    ASSERT_EQ(count, 5);
    ASSERT_TRUE(verify_values(actual, expected, count, "copy_range"));

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
    ANVIterator range_it = anv_iterator_range(2, 11, 2, &alloc);

    // Chain: range → copy
    ANVIterator copy_it = anv_iterator_copy(&range_it, &alloc, int_copy);

    // Expected: [2,4,6,8,10] copied
    const int expected[] = {2, 4, 6, 8, 10};
    int* actual[5];
    const int count = collect_values(&copy_it, actual, 5);

    ASSERT_EQ(count, 5);
    ASSERT_TRUE(verify_values(actual, expected, count, "range_copy_chain"));

    free_collected_values(actual, count);
    copy_it.destroy(&copy_it);
    return TEST_SUCCESS;
}

//==============================================================================
// Memory Management Tests
//==============================================================================

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

//==============================================================================
// Integration with Other Iterator Types Tests
//==============================================================================

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
    const int count = collect_values(&copy_it, actual, 3);

    ASSERT_EQ(count, 3);
    ASSERT_TRUE(verify_values(actual, expected, count, "filter_copy_chain"));

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
    const int count = collect_values(&copy_it, actual, 4);

    ASSERT_EQ(count, 4);
    ASSERT_TRUE(verify_values(actual, expected, count, "transform_copy_chain"));

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
    ANVIterator range_it = anv_iterator_range(1, 11, 1, &alloc);

    // Chain: range → filter even → transform square → copy
    ANVIterator filter_it = anv_iterator_filter(&range_it, &alloc, is_even);
    ANVIterator transform_it = anv_iterator_transform(&filter_it, &alloc, square_func, true);
    ANVIterator copy_it = anv_iterator_copy(&transform_it, &alloc, int_copy);

    // Expected: [1..10] → [2,4,6,8,10] → [4,16,36,64,100] → copied [4,16,36,64,100]
    const int expected[] = {4, 16, 36, 64, 100};
    int* actual[5];
    const int count = collect_values(&copy_it, actual, 5);

    ASSERT_EQ(count, 5);
    ASSERT_TRUE(verify_values(actual, expected, count, "complex_chain_copy"));

    free_collected_values(actual, count);
    copy_it.destroy(&copy_it);
    return TEST_SUCCESS;
}

//==============================================================================
// String Copy Tests
//==============================================================================

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

//==============================================================================
// Performance and Stress Tests
//==============================================================================

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

//==============================================================================
// Enhanced Helper Function Tests
//==============================================================================

/**
 * Enhanced helper function with better validation for copy iterator.
 */
static int collect_values_with_validation(const ANVIterator* it, int** values, int max_count)
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
    const int count = collect_values_with_validation(&copy_it, values, 5);

    // Should successfully collect all 5 copied values
    ASSERT_EQ(count, 5);

    const int expected[] = {1, 2, 3, 4, 5};
    ASSERT_TRUE(verify_values(values, expected, count, "copy_helper_validation"));

    // Free collected values
    free_collected_values(values, count);
    copy_it.destroy(&copy_it);
    anv_dll_destroy(list, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Test Suite Definition
//==============================================================================

typedef struct
{
    int (*func)(void);
    const char* name;
} TestCase;

static const TestCase tests[] = {
    // Basic functionality tests
    {test_copy_basic_integers, "test_copy_basic_integers"},
    {test_copy_single_element, "test_copy_single_element"},
    {test_copy_custom_structure, "test_copy_custom_structure"},

    // Edge case tests
    {test_copy_empty_input, "test_copy_empty_input"},
    {test_copy_large_dataset, "test_copy_large_dataset"},

    // Error handling tests
    {test_copy_invalid_inputs, "test_copy_invalid_inputs"},
    {test_copy_operations_on_invalid, "test_copy_operations_on_invalid"},

    // API behavior tests
    {test_copy_get_next_separation, "test_copy_get_next_separation"},
    {test_copy_next_return_codes, "test_copy_next_return_codes"},
    {test_copy_unsupported_operations, "test_copy_unsupported_operations"},

    // Integration tests with range iterator
    {test_copy_with_range_iterator, "test_copy_with_range_iterator"},
    {test_range_copy_chain, "test_range_copy_chain"},

    // Memory management tests
    {test_copy_memory_ownership, "test_copy_memory_ownership"},
    {test_copy_iterator_ownership, "test_copy_iterator_ownership"},
    {test_copy_memory_consistency, "test_copy_memory_consistency"},

    // Integration with other iterator types
    {test_filter_copy_chain, "test_filter_copy_chain"},
    {test_transform_copy_chain, "test_transform_copy_chain"},
    {test_complex_chain_with_copy, "test_complex_chain_with_copy"},

    // String copy tests
    {test_copy_strings, "test_copy_strings"},

    // Performance and stress tests
    {test_copy_performance, "test_copy_performance"},

    // Enhanced helper tests
    {test_copy_helper_validation, "test_copy_helper_validation"},
};

//==============================================================================
// Main Test Runner
//==============================================================================

int main(void)
{
    const int num_tests = sizeof(tests) / sizeof(tests[0]);
    int failed = 0;
    int passed = 0;

    printf("Running %d copy iterator tests...\n\n", num_tests);

    for (int i = 0; i < num_tests; i++)
    {
        printf("Running %s... ", tests[i].name);
        fflush(stdout);

        if (tests[i].func() == TEST_SUCCESS)
        {
            printf("PASSED\n");
            passed++;
        }
        else
        {
            printf("FAILED\n");
            failed++;
        }
    }

    printf("\n=== Copy Iterator Test Results ===\n");
    printf("Total tests: %d\n", num_tests);
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", failed);

    if (failed == 0)
    {
        printf("\n🎉 All copy iterator tests passed!\n");
        return 0;
    }

    printf("\n❌ %d test(s) failed.\n", failed);
    return 1;
}