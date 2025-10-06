//
// HashSet iterator test - converted from HashMap iterator test
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "containers/hashset.h"
#include "containers/iterator.h"
#include "TestAssert.h"
#include "TestHelpers.h"

// Test basic iterator functionality
static int test_hashset_iterator_basic(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    const char* keys[] = {"key1", "key2", "key3", "key4", "key5"};
    const int num_items = 5;

    // Add test data
    for (int i = 0; i < num_items; i++)
    {
        ASSERT_EQ(anv_hashset_add(set, (void*)keys[i]), 0);
    }

    // Test iterator
    ANVIterator it = anv_hashset_iterator(set);
    ASSERT(it.is_valid(&it));

    int visited_count = 0;
    bool found[5] = {false, false, false, false, false};

    while (it.has_next(&it))
    {
        const void* key = it.get(&it);
        ASSERT_NOT_NULL(key);

        // Find which item this is
        for (int i = 0; i < num_items; i++)
        {
            if (strcmp((char*)key, keys[i]) == 0)
            {
                ASSERT(!found[i]); // Should not have seen this before
                found[i] = true;
                break;
            }
        }
        visited_count++;
        it.next(&it);
    }

    // Verify we visited all items exactly once
    ASSERT_EQ(visited_count, num_items);
    for (int i = 0; i < num_items; i++)
    {
        ASSERT(found[i]);
    }

    // Verify the iterator is exhausted
    ASSERT(!it.has_next(&it));
    ASSERT_NULL(it.get(&it));
    ASSERT_EQ(it.next(&it), -1); // Should return error code

    it.destroy(&it);
    anv_hashset_destroy(set, false);
    return TEST_SUCCESS;
}

// Test iterator with empty set
static int test_hashset_iterator_empty(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    ANVIterator it = anv_hashset_iterator(set);

    // Verify iterator for empty set
    ASSERT(!it.has_next(&it));
    ASSERT_NULL(it.get(&it));
    ASSERT_EQ(it.next(&it), -1); // Should return error code

    it.destroy(&it);
    anv_hashset_destroy(set, false);
    return TEST_SUCCESS;
}

// Test iterator with modifications
static int test_hashset_iterator_with_modifications(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    // Insert initial elements
    const char* keys[] = {"key1", "key2", "key3"};

    for (int i = 0; i < 3; i++)
    {
        ASSERT_EQ(anv_hashset_add(set, (void*)keys[i]), 0);
    }

    // Create iterator
    ANVIterator it = anv_hashset_iterator(set);

    // Consume first element
    const void* key = it.get(&it);
    ASSERT_NOT_NULL(key);
    it.next(&it);

    // Modify set by adding new element
    ASSERT_EQ(anv_hashset_add(set, "new_key"), 0);

    // Continue iteration - new element should be visible
    int remaining_count = 0;
    while (it.has_next(&it))
    {
        key = it.get(&it);
        ASSERT_NOT_NULL(key);
        remaining_count++;
        it.next(&it);
    }

    // Should have seen at least 2 more elements (original + new)
    ASSERT_GTE(remaining_count, 2);

    it.destroy(&it);
    anv_hashset_destroy(set, false);
    return TEST_SUCCESS;
}

// Test multiple iterators
static int test_hashset_iterator_multiple(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    // Insert elements
    for (int i = 1; i <= 5; i++)
    {
        char* key = malloc(16);
        snprintf(key, 16, "key%d", i);
        ASSERT_EQ(anv_hashset_add(set, key), 0);
    }

    // Create two independent iterators
    ANVIterator it1 = anv_hashset_iterator(set);
    ANVIterator it2 = anv_hashset_iterator(set);

    // First iterator consumes two elements
    const void* key1 = it1.get(&it1);
    ASSERT_NOT_NULL(key1);
    it1.next(&it1);

    key1 = it1.get(&it1);
    ASSERT_NOT_NULL(key1);
    it1.next(&it1);

    // Second iterator should still be at the beginning
    const void* key2 = it2.get(&it2);
    ASSERT_NOT_NULL(key2);
    it2.next(&it2);

    // Continue with both iterators and verify they're independent
    int count1 = 0, count2 = 0;
    while (it1.has_next(&it1))
    {
        it1.get(&it1);
        it1.next(&it1);
        count1++;
    }

    while (it2.has_next(&it2))
    {
        it2.get(&it2);
        it2.next(&it2);
        count2++;
    }

    // Both should have seen all remaining elements
    ASSERT_EQ(count1, 3); // 5 total - 2 already consumed
    ASSERT_EQ(count2, 4); // 5 total - 1 already consumed

    it1.destroy(&it1);
    it2.destroy(&it2);
    anv_hashset_destroy(set, true);
    return TEST_SUCCESS;
}

// Test iterator get function
static int test_hashset_iterator_get(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    char* key = "test_key";

    ASSERT_EQ(anv_hashset_add(set, key), 0);

    ANVIterator it = anv_hashset_iterator(set);

    // Test get without advancing
    const void* current_key = it.get(&it);
    ASSERT_NOT_NULL(current_key);
    ASSERT_EQ_STR((char*)current_key, key);

    // Get again - should return same value
    const void* same_key = it.get(&it);
    ASSERT_EQ(current_key, same_key); // Same pointer

    // Now advance
    it.next(&it);
    ASSERT(!it.has_next(&it));
    ASSERT_NULL(it.get(&it));

    it.destroy(&it);
    anv_hashset_destroy(set, false);
    return TEST_SUCCESS;
}

// Test iterator backward operations (should not be supported)
static int test_hashset_iterator_backward(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    ASSERT_EQ(anv_hashset_add(set, "key"), 0);

    ANVIterator it = anv_hashset_iterator(set);

    // HashSet iterator should not support backward iteration
    ASSERT(!it.has_prev(&it));
    ASSERT_EQ(it.prev(&it), -1);

    it.destroy(&it);
    anv_hashset_destroy(set, false);
    return TEST_SUCCESS;
}

// Test creating hashset from iterator
static int test_hashset_from_iterator(void)
{
    ANVAllocator alloc = create_string_allocator();

    // Create original hashset
    ANVHashSet* original = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    const char* keys[] = {"key1", "key2", "key3"};

    for (int i = 0; i < 3; i++)
    {
        ASSERT_EQ(anv_hashset_add(original, (void*)keys[i]), 0);
    }

    // Create iterator from original
    ANVIterator it = anv_hashset_iterator(original);

    // Create new hashset from iterator
    ANVHashSet* new_set = anv_hashset_from_iterator(&it, &alloc, anv_hash_string, anv_key_equals_string, true);
    ASSERT_NOT_NULL(new_set);
    ASSERT_EQ(anv_hashset_size(new_set), 3);

    // Verify all data was copied
    for (int i = 0; i < 3; i++)
    {
        ASSERT(anv_hashset_contains(new_set, keys[i]));
    }

    it.destroy(&it);
    anv_hashset_destroy(original, false);
    anv_hashset_destroy(new_set, true);
    return TEST_SUCCESS;
}

// Test iterator with invalid hashset
static int test_hashset_iterator_invalid(void)
{
    const ANVIterator iter = anv_hashset_iterator(NULL);
    ASSERT(!iter.is_valid(&iter));
    return TEST_SUCCESS;
}

// Test copy isolation - verify that copied elements are independent
static int test_hashset_copy_isolation(void)
{
    ANVAllocator alloc = create_string_allocator();

    // Create source hashset
    ANVHashSet* source_set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);
    ASSERT_NOT_NULL(source_set);

    const char* keys[] = {"key1", "key2", "key3"};

    for (int i = 0; i < 3; i++)
    {
        ASSERT_EQ(anv_hashset_add(source_set, (void*)keys[i]), 0);
    }

    ANVIterator set_it = anv_hashset_iterator(source_set);
    ASSERT(set_it.is_valid(&set_it));

    // Create hashset with copying enabled
    ANVHashSet* new_set = anv_hashset_from_iterator(&set_it, &alloc, anv_hash_string, anv_key_equals_string, true);
    ASSERT_NOT_NULL(new_set);
    ASSERT_EQ(anv_hashset_size(new_set), 3);

    // Verify all original values are preserved in new set
    for (int i = 0; i < 3; i++)
    {
        ASSERT(anv_hashset_contains(new_set, keys[i]));
    }

    set_it.destroy(&set_it);
    anv_hashset_destroy(new_set, true);
    anv_hashset_destroy(source_set, false);
    return TEST_SUCCESS;
}

// Test that should_copy=true fails when allocator has no copy function
static int test_hashset_anv_copy_function_required(void)
{
    ANVAllocator alloc = anv_alloc_default();
    alloc.copy = NULL;

    ANVHashSet* source = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);
    ASSERT_EQ(anv_hashset_add(source, "key"), 0);

    ANVIterator it = anv_hashset_iterator(source);
    ASSERT(it.is_valid(&it));

    // Should return NULL because should_copy=true but no copy function available
    ANVHashSet* set = anv_hashset_from_iterator(&it, &alloc, anv_hash_string, anv_key_equals_string, true);
    ASSERT_NULL(set);

    it.destroy(&it);
    anv_hashset_destroy(source, false);
    return TEST_SUCCESS;
}

// Test that should_copy=false uses elements directly without copying
static int test_hashset_from_iterator_no_copy(void)
{
    ANVAllocator alloc = create_string_allocator();

    // Create source set with allocated strings
    ANVHashSet* source = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    char* key = malloc(16);
    strcpy(key, "test_key");

    ASSERT_EQ(anv_hashset_add(source, key), 0);

    ANVIterator it = anv_hashset_iterator(source);
    ASSERT(it.is_valid(&it));

    // Create hashset without copying (should_copy = false)
    ANVHashSet* set = anv_hashset_from_iterator(&it, &alloc, anv_hash_string, anv_key_equals_string, false);
    ASSERT_NOT_NULL(set);
    ASSERT_EQ(anv_hashset_size(set), 1);

    // Verify key is correct
    ASSERT(anv_hashset_contains(set, "test_key"));

    it.destroy(&it);
    anv_hashset_destroy(set, false);   // Don't free since we're sharing data
    anv_hashset_destroy(source, true); // Free the original allocated data
    return TEST_SUCCESS;
}

// Test that iterator is exhausted after being consumed by anv_hashset_from_iterator
static int test_hashset_iterator_exhaustion_after_creation(void)
{
    ANVAllocator alloc = create_string_allocator();

    ANVHashSet* source = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    for (int i = 0; i < 5; i++)
    {
        char* key = malloc(16);
        snprintf(key, 16, "key%d", i);
        ASSERT_EQ(anv_hashset_add(source, key), 0);
    }

    ANVIterator it = anv_hashset_iterator(source);
    ASSERT(it.is_valid(&it));

    // Verify iterator starts with elements
    ASSERT(it.has_next(&it));

    // Create hashset from iterator (consumes all elements)
    ANVHashSet* set = anv_hashset_from_iterator(&it, &alloc, anv_hash_string, anv_key_equals_string, true);
    ASSERT_NOT_NULL(set);
    ASSERT_EQ(anv_hashset_size(set), 5);

    // Iterator should now be exhausted
    ASSERT(!it.has_next(&it));
    ASSERT_NULL(it.get(&it));
    ASSERT_EQ(it.next(&it), -1); // Should fail to advance

    // But iterator should still be valid
    ASSERT(it.is_valid(&it));

    it.destroy(&it);
    anv_hashset_destroy(set, true);
    anv_hashset_destroy(source, true);
    return TEST_SUCCESS;
}

// Test next() return values for proper error handling
static int test_hashset_iterator_next_return_values(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    // Add single element
    ASSERT_EQ(anv_hashset_add(set, "key"), 0);

    ANVIterator iter = anv_hashset_iterator(set);
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
    anv_hashset_destroy(set, false);
    return TEST_SUCCESS;
}

// Test various combinations of get/next/has_next calls for consistency
static int test_hashset_iterator_mixed_operations(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    // Add test data
    const char* keys[] = {"a", "b", "c"};

    for (int i = 0; i < 3; i++)
    {
        ASSERT_EQ(anv_hashset_add(set, (void*)keys[i]), 0);
    }

    ANVIterator iter = anv_hashset_iterator(set);
    ASSERT(iter.is_valid(&iter));

    // Multiple get() calls should return same value
    const void* key1 = iter.get(&iter);
    const void* key2 = iter.get(&iter);
    ASSERT_NOT_NULL(key1);
    ASSERT_NOT_NULL(key2);
    ASSERT_EQ(key1, key2);                   // Same pointer
    ASSERT_EQ_STR((char*)key1, (char*)key2); // Same key

    // has_next should be consistent
    ASSERT(iter.has_next(&iter));
    ASSERT(iter.has_next(&iter)); // Multiple calls should be safe

    // Only advance if there are more elements
    if (iter.has_next(&iter))
    {
        ASSERT_EQ(iter.next(&iter), 0);

        // Check if we still have a valid element after advancing
        if (iter.has_next(&iter))
        {
            const void* key3 = iter.get(&iter);
            ASSERT_NOT_NULL(key3);
            // Different keys should have different values
            ASSERT(strcmp((char*)key1, (char*)key3) != 0);
        }
        else
        {
            // Iterator is exhausted after advancing
            ASSERT_NULL(iter.get(&iter));
        }
    }

    iter.destroy(&iter);
    anv_hashset_destroy(set, false);
    return TEST_SUCCESS;
}

// Test reset functionality
static int test_hashset_iterator_reset(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    for (int i = 0; i < 3; i++)
    {
        char* key = malloc(16);
        snprintf(key, 16, "key%d", i);
        ASSERT_EQ(anv_hashset_add(set, key), 0);
    }

    ANVIterator iter = anv_hashset_iterator(set);

    // First iteration
    int first_count = 0;
    while (iter.has_next(&iter))
    {
        iter.get(&iter);
        iter.next(&iter);
        first_count++;
    }
    ASSERT_EQ(first_count, 3);

    // Reset and iterate again
    iter.reset(&iter);
    int second_count = 0;
    while (iter.has_next(&iter))
    {
        iter.get(&iter);
        iter.next(&iter);
        second_count++;
    }
    ASSERT_EQ(second_count, 3);

    iter.destroy(&iter);
    anv_hashset_destroy(set, true);
    return TEST_SUCCESS;
}

// Test single element iterator behavior
static int test_hashset_iterator_single_element(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    ASSERT_EQ(anv_hashset_add(set, "single"), 0);

    ANVIterator iter = anv_hashset_iterator(set);

    ASSERT(iter.has_next(&iter));
    ASSERT(!iter.has_prev(&iter)); // HashSet doesn't support backward iteration

    const void* key = iter.get(&iter);
    ASSERT_NOT_NULL(key);
    ASSERT_EQ_STR((char*)key, "single");

    iter.next(&iter);
    ASSERT(!iter.has_next(&iter));
    ASSERT(!iter.has_prev(&iter)); // Still no backward support

    iter.destroy(&iter);
    anv_hashset_destroy(set, false);
    return TEST_SUCCESS;
}

// Test from_iterator with NULL parameters
static int test_hashset_from_iterator_null_params(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);
    ANVIterator it = anv_hashset_iterator(set);

    // Test NULL iterator
    ASSERT_NULL(anv_hashset_from_iterator(NULL, &alloc, anv_hash_string, anv_key_equals_string, true));

    // Test NULL allocator
    ASSERT_NULL(anv_hashset_from_iterator(&it, NULL, anv_hash_string, anv_key_equals_string, true));

    // Test NULL hash function
    ASSERT_NULL(anv_hashset_from_iterator(&it, &alloc, NULL, anv_key_equals_string, true));

    // Test NULL key_equals function
    ASSERT_NULL(anv_hashset_from_iterator(&it, &alloc, anv_hash_string, NULL, true));

    it.destroy(&it);
    anv_hashset_destroy(set, false);
    return TEST_SUCCESS;
}

typedef struct
{
    int (*func)(void);
    const char* name;
} TestCase;

int main(void)
{
    const TestCase tests[] = {
        {test_hashset_iterator_basic, "test_hashset_iterator_basic"},
        {test_hashset_iterator_empty, "test_hashset_iterator_empty"},
        {test_hashset_iterator_with_modifications, "test_hashset_iterator_with_modifications"},
        {test_hashset_iterator_multiple, "test_hashset_iterator_multiple"},
        {test_hashset_iterator_get, "test_hashset_iterator_get"},
        {test_hashset_iterator_backward, "test_hashset_iterator_backward"},
        {test_hashset_from_iterator, "test_hashset_from_iterator"},
        {test_hashset_iterator_invalid, "test_hashset_iterator_invalid"},
        {test_hashset_copy_isolation, "test_hashset_copy_isolation"},
        {test_hashset_anv_copy_function_required, "test_hashset_anv_copy_function_required"},
        {test_hashset_from_iterator_no_copy, "test_hashset_from_iterator_no_copy"},
        {test_hashset_iterator_exhaustion_after_creation, "test_hashset_iterator_exhaustion_after_creation"},
        {test_hashset_iterator_next_return_values, "test_hashset_iterator_next_return_values"},
        {test_hashset_iterator_mixed_operations, "test_hashset_iterator_mixed_operations"},
        {test_hashset_iterator_reset, "test_hashset_iterator_reset"},
        {test_hashset_iterator_single_element, "test_hashset_iterator_single_element"},
        {test_hashset_from_iterator_null_params, "test_hashset_from_iterator_null_params"},
    };

    printf("Running HashSet iterator tests...\n");

    int failed = 0;
    const int num_tests = sizeof(tests) / sizeof(tests[0]);

    for (int i = 0; i < num_tests; i++)
    {
        if (tests[i].func() != TEST_SUCCESS)
        {
            printf("%s failed\n", tests[i].name);
            failed++;
        }
    }

    if (failed == 0)
    {
        printf("All HashSet iterator tests passed!\n");
        return 0;
    }

    printf("%d HashSet iterator test(s) failed.\n", failed);
    return 1;
}