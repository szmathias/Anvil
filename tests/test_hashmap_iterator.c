//
// HashMap iterator test - converted from DoublyLinkedList iterator test
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "containers/hashmap.h"
#include "containers/iterator.h"
#include "TestAssert.h"
#include "TestHelpers.h"

// Test basic iterator functionality
static int test_hashmap_iterator_basic(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    const char* keys[] = {"key1", "key2", "key3", "key4", "key5"};
    const char* values[] = {"val1", "val2", "val3", "val4", "val5"};
    const int num_items = 5;

    // Add test data
    for (int i = 0; i < num_items; i++)
    {
        ASSERT_EQ(anv_hashmap_put(map, (void*)keys[i], (void*)values[i]), 0);
    }

    // Test iterator
    ANVIterator it = anv_hashmap_iterator(map);
    ASSERT(it.is_valid(&it));

    int visited_count = 0;
    bool found[5] = {false, false, false, false, false};

    while (it.has_next(&it))
    {
        const ANVPair* pair = it.get(&it);
        ASSERT_NOT_NULL(pair);
        ASSERT_NOT_NULL(pair->first);
        ASSERT_NOT_NULL(pair->second);

        // Find which item this is
        for (int i = 0; i < num_items; i++)
        {
            if (strcmp(pair->first, keys[i]) == 0)
            {
                ASSERT_EQ_STR((char*)pair->second, values[i]);
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
    anv_hashmap_destroy(map, false, false);
    return TEST_SUCCESS;
}

// Test iterator with empty map
static int test_hashmap_iterator_empty(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    ANVIterator it = anv_hashmap_iterator(map);

    // Verify iterator for empty map
    ASSERT(!it.has_next(&it));
    ASSERT_NULL(it.get(&it));
    ASSERT_EQ(it.next(&it), -1); // Should return error code

    it.destroy(&it);
    anv_hashmap_destroy(map, false, false);
    return TEST_SUCCESS;
}

// Test iterator with modifications
static int test_hashmap_iterator_with_modifications(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    // Insert initial elements
    const char* keys[] = {"key1", "key2", "key3"};
    const char* values[] = {"val1", "val2", "val3"};

    for (int i = 0; i < 3; i++)
    {
        ASSERT_EQ(anv_hashmap_put(map, (void*)keys[i], (void*)values[i]), 0);
    }

    // Create iterator
    ANVIterator it = anv_hashmap_iterator(map);

    // Consume first element
    const ANVPair* pair = it.get(&it);
    ASSERT_NOT_NULL(pair);
    it.next(&it);

    // Modify map by adding new element
    ASSERT_EQ(anv_hashmap_put(map, "new_key", "new_val"), 0);

    // Continue iteration - new element should be visible
    int remaining_count = 0;
    while (it.has_next(&it))
    {
        pair = it.get(&it);
        ASSERT_NOT_NULL(pair);
        remaining_count++;
        it.next(&it);
    }

    // Should have seen at least 2 more elements (original + new)
    ASSERT_GTE(remaining_count, 2);

    it.destroy(&it);
    anv_hashmap_destroy(map, false, false);
    return TEST_SUCCESS;
}

// Test multiple iterators
static int test_hashmap_iterator_multiple(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    // Insert elements
    for (int i = 1; i <= 5; i++)
    {
        char* key = malloc(16);
        char* value = malloc(16);
        snprintf(key, 16, "key%d", i);
        snprintf(value, 16, "val%d", i);
        ASSERT_EQ(anv_hashmap_put(map, key, value), 0);
    }

    // Create two independent iterators
    ANVIterator it1 = anv_hashmap_iterator(map);
    ANVIterator it2 = anv_hashmap_iterator(map);

    // First iterator consumes two elements
    const ANVPair* pair1 = it1.get(&it1);
    ASSERT_NOT_NULL(pair1);
    it1.next(&it1);

    pair1 = it1.get(&it1);
    ASSERT_NOT_NULL(pair1);
    it1.next(&it1);

    // Second iterator should still be at the beginning
    const ANVPair* pair2 = it2.get(&it2);
    ASSERT_NOT_NULL(pair2);
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
    anv_hashmap_destroy(map, true, true);
    return TEST_SUCCESS;
}

// Test iterator get function
static int test_hashmap_iterator_get(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    char* key = "test_key";
    char* value = "test_value";

    ASSERT_EQ(anv_hashmap_put(map, key, value), 0);

    ANVIterator it = anv_hashmap_iterator(map);

    // Test get without advancing
    const ANVPair* pair = it.get(&it);
    ASSERT_NOT_NULL(pair);
    ASSERT_EQ_STR((char*)pair->first, key);
    ASSERT_EQ_STR((char*)pair->second, value);

    // Get again - should return same value
    const ANVPair* pair2 = it.get(&it);
    ASSERT_EQ(pair, pair2); // Same pointer

    // Now advance
    it.next(&it);
    ASSERT(!it.has_next(&it));
    ASSERT_NULL(it.get(&it));

    it.destroy(&it);
    anv_hashmap_destroy(map, false, false);
    return TEST_SUCCESS;
}

// Test iterator backward operations (should not be supported)
static int test_hashmap_iterator_backward(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    ASSERT_EQ(anv_hashmap_put(map, "key", "value"), 0);

    ANVIterator it = anv_hashmap_iterator(map);

    // HashMap iterator should not support backward iteration
    ASSERT(!it.has_prev(&it));
    ASSERT_EQ(it.prev(&it), -1);

    it.destroy(&it);
    anv_hashmap_destroy(map, false, false);
    return TEST_SUCCESS;
}

// Test creating hashmap from iterator
static int test_hashmap_from_iterator(void)
{
    ANVAllocator alloc = create_string_allocator();
    alloc.copy = anv_pair_copy_string_string;

    // Create original hashmap
    ANVHashMap* original = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    const char* keys[] = {"key1", "key2", "key3"};
    const char* values[] = {"val1", "val2", "val3"};

    for (int i = 0; i < 3; i++)
    {
        ASSERT_EQ(anv_hashmap_put(original, (void*)keys[i], (void*)values[i]), 0);
    }

    // Create iterator from original
    ANVIterator it = anv_hashmap_iterator(original);

    // Create new hashmap from iterator
    ANVHashMap* new_map = anv_hashmap_from_iterator(&it, &alloc, anv_hash_string, anv_key_equals_string, true);
    ASSERT_NOT_NULL(new_map);
    ASSERT_EQ(anv_hashmap_size(new_map), 3);

    // Verify all data was copied
    for (int i = 0; i < 3; i++)
    {
        void* value = anv_hashmap_get(new_map, keys[i]);
        ASSERT_NOT_NULL(value);
        ASSERT_EQ_STR((char*)value, values[i]);
    }

    it.destroy(&it);
    anv_hashmap_destroy(original, false, false);
    anv_hashmap_destroy(new_map, true, true);
    return TEST_SUCCESS;
}

// Test iterator with invalid hashmap
static int test_hashmap_iterator_invalid(void)
{
    const ANVIterator iter = anv_hashmap_iterator(NULL);
    ASSERT(!iter.is_valid(&iter));
    return TEST_SUCCESS;
}

// Test copy isolation - verify that copied elements are independent
static int test_hashmap_copy_isolation(void)
{
    ANVAllocator alloc = create_string_allocator();
    alloc.copy = anv_pair_copy_string_string;

    // Create source hashmap
    ANVHashMap* source_map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);
    ASSERT_NOT_NULL(source_map);

    const char* keys[] = {"key1", "key2", "key3"};
    const char* values[] = {"val1", "val2", "val3"};

    for (int i = 0; i < 3; i++)
    {
        ASSERT_EQ(anv_hashmap_put(source_map, (void*)keys[i], (void*)values[i]), 0);
    }

    ANVIterator map_it = anv_hashmap_iterator(source_map);
    ASSERT(map_it.is_valid(&map_it));

    // Create hashmap with copying enabled
    ANVHashMap* new_map = anv_hashmap_from_iterator(&map_it, &alloc, anv_hash_string, anv_key_equals_string, true);
    ASSERT_NOT_NULL(new_map);
    ASSERT_EQ(anv_hashmap_size(new_map), 3);

    // Verify all original values are preserved in new map
    for (int i = 0; i < 3; i++)
    {
        void* value = anv_hashmap_get(new_map, keys[i]);
        ASSERT_NOT_NULL(value);
        ASSERT_EQ_STR((char*)value, values[i]);
    }

    map_it.destroy(&map_it);
    anv_hashmap_destroy(new_map, true, true);
    anv_hashmap_destroy(source_map, false, false);
    return TEST_SUCCESS;
}

// Test that should_copy=true fails when allocator has no copy function
static int test_hashmap_anv_copy_function_required(void)
{
    ANVAllocator alloc = anv_alloc_default();
    alloc.copy = NULL;

    ANVHashMap* source = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);
    ASSERT_EQ(anv_hashmap_put(source, "key", "value"), 0);

    ANVIterator it = anv_hashmap_iterator(source);
    ASSERT(it.is_valid(&it));

    // Should return NULL because should_copy=true but no copy function available
    ANVHashMap* map = anv_hashmap_from_iterator(&it, &alloc, anv_hash_string, anv_key_equals_string, true);
    ASSERT_NULL(map);

    it.destroy(&it);
    anv_hashmap_destroy(source, false, false);
    return TEST_SUCCESS;
}

// Test that should_copy=false uses elements directly without copying
static int test_hashmap_from_iterator_no_copy(void)
{
    ANVAllocator alloc = create_string_allocator();

    // Create source map with allocated strings
    ANVHashMap* source = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    char* key = malloc(16);
    char* value = malloc(16);
    strcpy(key, "test_key");
    strcpy(value, "test_value");

    ASSERT_EQ(anv_hashmap_put(source, key, value), 0);

    ANVIterator it = anv_hashmap_iterator(source);
    ASSERT(it.is_valid(&it));

    // Create hashmap without copying (should_copy = false)
    ANVHashMap* map = anv_hashmap_from_iterator(&it, &alloc, anv_hash_string, anv_key_equals_string, false);
    ASSERT_NOT_NULL(map);
    ASSERT_EQ(anv_hashmap_size(map), 1);

    // Verify value is correct
    void* retrieved_value = anv_hashmap_get(map, "test_key");
    ASSERT_NOT_NULL(retrieved_value);
    ASSERT_EQ_STR((char*)retrieved_value, "test_value");

    it.destroy(&it);
    anv_hashmap_destroy(map, false, false);  // Don't free since we're sharing data
    anv_hashmap_destroy(source, true, true); // Free the original allocated data
    return TEST_SUCCESS;
}

// Test that iterator is exhausted after being consumed by anv_hashmap_from_iterator
static int test_hashmap_iterator_exhaustion_after_creation(void)
{
    ANVAllocator alloc = create_string_allocator();
    alloc.copy = anv_pair_copy_string_string;

    ANVHashMap* source = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    for (int i = 0; i < 5; i++)
    {
        char* key = malloc(16);
        char* value = malloc(16);
        snprintf(key, 16, "key%d", i);
        snprintf(value, 16, "val%d", i);
        ASSERT_EQ(anv_hashmap_put(source, key, value), 0);
    }

    ANVIterator it = anv_hashmap_iterator(source);
    ASSERT(it.is_valid(&it));

    // Verify iterator starts with elements
    ASSERT(it.has_next(&it));

    // Create hashmap from iterator (consumes all elements)
    ANVHashMap* map = anv_hashmap_from_iterator(&it, &alloc, anv_hash_string, anv_key_equals_string, true);
    ASSERT_NOT_NULL(map);
    ASSERT_EQ(anv_hashmap_size(map), 5);

    // Iterator should now be exhausted
    ASSERT(!it.has_next(&it));
    ASSERT_NULL(it.get(&it));
    ASSERT_EQ(it.next(&it), -1); // Should fail to advance

    // But iterator should still be valid
    ASSERT(it.is_valid(&it));

    it.destroy(&it);
    anv_hashmap_destroy(map, true, true);
    anv_hashmap_destroy(source, true, true);
    return TEST_SUCCESS;
}

// Test next() return values for proper error handling
static int test_hashmap_iterator_next_return_values(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    // Add single element
    ASSERT_EQ(anv_hashmap_put(map, "key", "value"), 0);

    ANVIterator iter = anv_hashmap_iterator(map);
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
    anv_hashmap_destroy(map, false, false);
    return TEST_SUCCESS;
}

// Test various combinations of get/next/has_next calls for consistency
static int test_hashmap_iterator_mixed_operations(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    // Add test data
    const char* keys[] = {"a", "b", "c"};
    const char* values[] = {"10", "20", "30"};
    for (int i = 0; i < 3; i++)
    {
        ASSERT_EQ(anv_hashmap_put(map, (void*)keys[i], (void*)values[i]), 0);
    }

    ANVIterator iter = anv_hashmap_iterator(map);
    ASSERT(iter.is_valid(&iter));

    // Multiple get() calls should return same pointer and same content
    const ANVPair* pair1 = iter.get(&iter);
    const ANVPair* pair2 = iter.get(&iter);
    ASSERT_NOT_NULL(pair1);
    ASSERT_NOT_NULL(pair2);
    ASSERT_EQ(pair1, pair2);                                 // Same pointer
    ASSERT_EQ_STR((char*)pair1->first, (char*)pair2->first);     // Same key
    ASSERT_EQ_STR((char*)pair1->second, (char*)pair2->second); // Same value

    // Capture first pair's data before advancing
    char first_key[10];
    char first_value[10];

    strcpy(first_key, pair1->first);
    strcpy(first_value, pair1->second);

    // has_next should be consistent
    ASSERT(iter.has_next(&iter));
    ASSERT(iter.has_next(&iter)); // Multiple calls should be safe

    // Advance and verify new position
    ASSERT_EQ(iter.next(&iter), 0);
    const ANVPair* pair3 = iter.get(&iter);
    ASSERT_NOT_NULL(pair3);

    // Same pointer (cached), but different content after advancing
    ASSERT_EQ(pair1, pair3); // Same cached pointer
    ASSERT(strcmp(first_key, (char*)pair3->first) != 0 ||
           strcmp(first_value, (char*)pair3->second) != 0); // Different content

    // Verify get() consistency at new position
    const ANVPair* pair4 = iter.get(&iter);
    ASSERT_EQ(pair3, pair4);                                 // Same pointer
    ASSERT_EQ_STR((char*)pair3->first, (char*)pair4->first);     // Same content
    ASSERT_EQ_STR((char*)pair3->second, (char*)pair4->second); // Same content

    // Test has_next behavior
    ASSERT_TRUE(iter.has_next(&iter));

    char second_key[10];
    char second_value[10];
    strcpy(second_key, pair3->first);
    strcpy(second_value, pair3->second);

    ASSERT_EQ(iter.next(&iter), 0);

    const ANVPair* pair5 = iter.get(&iter);
    ASSERT_NOT_NULL(pair5);

    // Should be different from both previous positions
    ASSERT(strcmp(first_key, pair5->first) != 0 ||
           strcmp(first_value, pair5->second) != 0);

    ASSERT(strcmp(second_key, pair5->first) != 0 ||
           strcmp(second_value, pair5->second) != 0);

    iter.destroy(&iter);
    anv_hashmap_destroy(map, false, false);
    return TEST_SUCCESS;
}

// Test reset functionality
static int test_hashmap_iterator_reset(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    for (int i = 0; i < 3; i++)
    {
        char* key = malloc(16);
        char* value = malloc(16);
        snprintf(key, 16, "key%d", i);
        snprintf(value, 16, "val%d", i);
        ASSERT_EQ(anv_hashmap_put(map, key, value), 0);
    }

    ANVIterator iter = anv_hashmap_iterator(map);

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
    anv_hashmap_destroy(map, true, true);
    return TEST_SUCCESS;
}

// Test single element iterator behavior
static int test_hashmap_iterator_single_element(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    ASSERT_EQ(anv_hashmap_put(map, "single", "element"), 0);

    ANVIterator iter = anv_hashmap_iterator(map);

    ASSERT(iter.has_next(&iter));
    ASSERT(!iter.has_prev(&iter)); // HashMap doesn't support backward iteration

    const ANVPair* pair = iter.get(&iter);
    ASSERT_NOT_NULL(pair);
    ASSERT_EQ_STR((char*)pair->first, "single");
    ASSERT_EQ_STR((char*)pair->second, "element");

    iter.next(&iter);
    ASSERT(!iter.has_next(&iter));
    ASSERT(!iter.has_prev(&iter)); // Still no backward support

    iter.destroy(&iter);
    anv_hashmap_destroy(map, false, false);
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
        {test_hashmap_iterator_basic, "test_hashmap_iterator_basic"},
        {test_hashmap_iterator_empty, "test_hashmap_iterator_empty"},
        {test_hashmap_iterator_with_modifications, "test_hashmap_iterator_with_modifications"},
        {test_hashmap_iterator_multiple, "test_hashmap_iterator_multiple"},
        {test_hashmap_iterator_get, "test_hashmap_iterator_get"},
        {test_hashmap_iterator_backward, "test_hashmap_iterator_backward"},
        {test_hashmap_from_iterator, "test_hashmap_from_iterator"},
        {test_hashmap_iterator_invalid, "test_hashmap_iterator_invalid"},
        {test_hashmap_copy_isolation, "test_hashmap_copy_isolation"},
        {test_hashmap_anv_copy_function_required, "test_hashmap_anv_copy_function_required"},
        {test_hashmap_from_iterator_no_copy, "test_hashmap_from_iterator_no_copy"},
        {test_hashmap_iterator_exhaustion_after_creation, "test_hashmap_iterator_exhaustion_after_creation"},
        {test_hashmap_iterator_next_return_values, "test_hashmap_iterator_next_return_values"},
        {test_hashmap_iterator_mixed_operations, "test_hashmap_iterator_mixed_operations"},
        {test_hashmap_iterator_reset, "test_hashmap_iterator_reset"},
        {test_hashmap_iterator_single_element, "test_hashmap_iterator_single_element"},
    };

    printf("Running HashMap iterator tests...\n");

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
        printf("All HashMap iterator tests passed!\n");
        return 0;
    }

    printf("%d HashMap iterator test(s) failed.\n", failed);
    return 1;
}