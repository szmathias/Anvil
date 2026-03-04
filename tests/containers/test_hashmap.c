#include <stdlib.h>
#include <time.h>

#include <anvil/testing.h>
#include "TestHelpers.h"
#include "containers/hashmap.h"
#include "containers/pair.h"

//==============================================================================
// Static Helpers
//==============================================================================

static void increment_value(void* key, void* value)
{
    (void)key;
    int* val = value;
    (*val)++;
}

//==============================================================================
// Algorithms Tests
//==============================================================================

int test_hashmap_copy_shallow(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashMap* original = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    // Add some test data
    const char* keys[] = {"apple", "banana", "cherry", "date"};
    const char* values[] = {"red", "yellow", "red", "brown"};

    for (int i = 0; i < 4; i++)
    {
        ASSERT_EQ(anv_hashmap_put(original, (void*)keys[i], (void*)values[i]), 0);
    }

    // Create shallow copy
    ANVHashMap* copy = anv_hashmap_copy(original);
    ASSERT_NOT_NULL(copy);
    ASSERT_EQ(anv_hashmap_size(copy), 4);

    // Verify data is shared (same pointers)
    for (int i = 0; i < 4; i++)
    {
        void* orig_value = anv_hashmap_get(original, keys[i]);
        void* copy_value = anv_hashmap_get(copy, keys[i]);
        ASSERT_EQ_PTR(orig_value, copy_value); // Should be same pointer
        ASSERT_EQ_STR((char*)orig_value, values[i]);
    }

    anv_hashmap_destroy(original, false, false);
    anv_hashmap_destroy(copy, false, false);
    return TEST_SUCCESS;
}

int test_hashmap_copy_deep(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashMap* original = anv_hashmap_create(&alloc, anv_hash_int, anv_key_equals_int, 0);

    // Add some test data
    for (int i = 0; i < 3; i++)
    {
        int* key = malloc(sizeof(int));
        int* value = malloc(sizeof(int));
        *key = i;
        *value = i * 10;
        ASSERT_EQ(anv_hashmap_put(original, key, value), 0);
    }

    // Create deep copy
    ANVHashMap* copy = anv_hashmap_copy_deep(original, int_copy, int_copy);
    ASSERT_NOT_NULL(copy);
    ASSERT_EQ(anv_hashmap_size(copy), 3);

    // Verify data is different (different pointers, same values)
    for (int i = 0; i < 3; i++)
    {
        int* orig_value = anv_hashmap_get(original, &i);
        int* copy_value = anv_hashmap_get(copy, &i);
        ASSERT_NOT_EQ_PTR(orig_value, copy_value);
        ASSERT_EQ(*orig_value, *copy_value);
        ASSERT_EQ(*orig_value, i * 10);
    }

    anv_hashmap_destroy(original, true, true);
    anv_hashmap_destroy(copy, true, true);
    return TEST_SUCCESS;
}

// Test for_each functionality
int test_hashmap_for_each(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_int, anv_key_equals_int, 0);

    // Add some test data
    for (int i = 1; i <= 5; i++)
    {
        int* key = malloc(sizeof(int));
        int* value = malloc(sizeof(int));
        *key = i;
        *value = i * 10;
        ASSERT_EQ(anv_hashmap_put(map, key, value), 0);
    }

    // Test for_each with action that increments values
    anv_hashmap_for_each(map, increment_value);

    // Verify values were incremented
    for (int i = 1; i <= 5; i++)
    {
        const int* value = anv_hashmap_get(map, &i);
        ASSERT_EQ(*value, i * 10 + 1);
    }

    // Test with NULL parameters
    anv_hashmap_for_each(NULL, increment_value); // Should be safe
    anv_hashmap_for_each(map, NULL);             // Should be safe

    anv_hashmap_destroy(map, true, true);
    return TEST_SUCCESS;
}

// Test get_keys functionality
int test_hashmap_get_keys(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    const char* keys[] = {"alpha", "beta", "gamma"};
    const char* values[] = {"1", "2", "3"};

    // Add test data
    for (int i = 0; i < 3; i++)
    {
        ASSERT_EQ(anv_hashmap_put(map, (void*)keys[i], (void*)values[i]), 0);
    }

    // Get all keys
    void** retrieved_keys;
    size_t count;
    ASSERT_EQ(anv_hashmap_get_keys(map, &retrieved_keys, &count), 0);
    ASSERT_EQ(count, 3);
    ASSERT_NOT_NULL(retrieved_keys);

    // Verify all keys are present (order may vary)
    int found_count = 0;
    for (size_t i = 0; i < count; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (strcmp(retrieved_keys[i], keys[j]) == 0)
            {
                found_count++;
                break;
            }
        }
    }
    ASSERT_EQ(found_count, 3);

    free(retrieved_keys);
    anv_hashmap_destroy(map, false, false);
    return TEST_SUCCESS;
}

// Test get_values functionality
int test_hashmap_get_values(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    const char* keys[] = {"x", "y", "z"};
    const char* values[] = {"10", "20", "30"};

    // Add test data
    for (int i = 0; i < 3; i++)
    {
        ASSERT_EQ(anv_hashmap_put(map, (void*)keys[i], (void*)values[i]), 0);
    }

    // Get all values
    void** retrieved_values;
    size_t count;
    ASSERT_EQ(anv_hashmap_get_values(map, &retrieved_values, &count), 0);
    ASSERT_EQ(count, 3);
    ASSERT_NOT_NULL(retrieved_values);

    // Verify all values are present (order may vary)
    int found_count = 0;
    for (size_t i = 0; i < count; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (strcmp(retrieved_values[i], values[j]) == 0)
            {
                found_count++;
                break;
            }
        }
    }
    ASSERT_EQ(found_count, 3);

    free(retrieved_values);
    anv_hashmap_destroy(map, false, false);
    return TEST_SUCCESS;
}

// Test from_iterator functionality (algorithms variant)
int test_hashmap_from_iterator_algorithms(void)
{
    ANVAllocator alloc = create_string_allocator();
    alloc.copy = anv_pair_copy_string_string;
    ANVHashMap* original = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    const char* keys[] = {"key1", "key2", "key3"};
    const char* values[] = {"val1", "val2", "val3"};

    // Add test data to original
    for (int i = 0; i < 3; i++)
    {
        ASSERT_EQ(anv_hashmap_put(original, (void*)keys[i], (void*)values[i]), 0);
    }

    // Create iterator from original
    ANVIterator it = anv_hashmap_iterator(original);

    // Create new map from iterator
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

//==============================================================================
// CRUD tests
//==============================================================================

// Test basic hash map creation and destruction
int test_hashmap_create_destroy(void)
{
    ANVAllocator alloc = create_int_allocator();

    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);
    ASSERT_NOT_NULL(map);
    ASSERT_EQ(anv_hashmap_size(map), 0);
    ASSERT(anv_hashmap_is_empty(map));

    anv_hashmap_destroy(map, false, false);
    return TEST_SUCCESS;
}

// Test basic put and get operations
int test_hashmap_put_get(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    char* key1 = "hello";
    char* value1 = "world";
    char* key2 = "foo";
    char* value2 = "bar";

    // Test put operations
    ASSERT_EQ(anv_hashmap_put(map, key1, value1), 0);
    ASSERT_EQ(anv_hashmap_size(map), 1);
    ASSERT(!anv_hashmap_is_empty(map));

    ASSERT_EQ(anv_hashmap_put(map, key2, value2), 0);
    ASSERT_EQ(anv_hashmap_size(map), 2);

    // Test get operations
    void* retrieved = anv_hashmap_get(map, key1);
    ASSERT_NOT_NULL(retrieved);
    ASSERT_EQ_STR((char*)retrieved, value1);

    retrieved = anv_hashmap_get(map, key2);
    ASSERT_NOT_NULL(retrieved);
    ASSERT_EQ_STR((char*)retrieved, value2);

    // Test non-existent key
    retrieved = anv_hashmap_get(map, "nonexistent");
    ASSERT_NULL(retrieved);

    anv_hashmap_destroy(map, false, false);
    return TEST_SUCCESS;
}

// Test updating existing keys
int test_hashmap_update(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    char* key = "test";
    char* value1 = "original";
    char* value2 = "updated";

    // Insert initial value
    ASSERT_EQ(anv_hashmap_put(map, key, value1), 0);
    ASSERT_EQ(anv_hashmap_size(map), 1);

    void* retrieved = anv_hashmap_get(map, key);
    ASSERT_EQ_STR((char*)retrieved, value1);

    // Update the value
    ASSERT_EQ(anv_hashmap_put(map, key, value2), 0);
    ASSERT_EQ(anv_hashmap_size(map), 1); // Size should remain the same

    retrieved = anv_hashmap_get(map, key);
    ASSERT_EQ_STR((char*)retrieved, value2);

    anv_hashmap_destroy(map, false, false);
    return TEST_SUCCESS;
}

// Test remove operations
int test_hashmap_remove(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    char* key1 = "key1";
    char* value1 = "value1";
    char* key2 = "key2";
    char* value2 = "value2";

    // Add some items
    ASSERT_EQ(anv_hashmap_put(map, key1, value1), 0);
    ASSERT_EQ(anv_hashmap_put(map, key2, value2), 0);
    ASSERT_EQ(anv_hashmap_size(map), 2);

    // Test remove
    ASSERT_EQ(anv_hashmap_remove(map, key1, false, false), 0);
    ASSERT_EQ(anv_hashmap_size(map), 1);
    ASSERT_NULL(anv_hashmap_get(map, key1));
    ASSERT_NOT_NULL(anv_hashmap_get(map, key2));

    // Test remove non-existent key
    ASSERT_EQ(anv_hashmap_remove(map, "nonexistent", false, false), -1);
    ASSERT_EQ(anv_hashmap_size(map), 1);

    anv_hashmap_destroy(map, false, false);
    return TEST_SUCCESS;
}

// Test remove_get operation
int test_hashmap_remove_get(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    char* key = "test";
    char* value = "value";

    ASSERT_EQ(anv_hashmap_put(map, key, value), 0);

    // Test remove_get
    void* removed_value = anv_hashmap_remove_get(map, key, false);
    ASSERT_NOT_NULL(removed_value);
    ASSERT_EQ_STR((char*)removed_value, value);
    ASSERT_EQ(anv_hashmap_size(map), 0);
    ASSERT_NULL(anv_hashmap_get(map, key));

    anv_hashmap_destroy(map, false, false);
    return TEST_SUCCESS;
}

// Test contains_key operation
int test_hashmap_contains(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    char* key = "test";
    char* value = "value";

    ASSERT(!anv_hashmap_contains_key(map, key));

    ASSERT_EQ(anv_hashmap_put(map, key, value), 0);
    ASSERT(anv_hashmap_contains_key(map, key));

    ASSERT_EQ(anv_hashmap_remove(map, key, false, false), 0);
    ASSERT(!anv_hashmap_contains_key(map, key));

    anv_hashmap_destroy(map, false, false);
    return TEST_SUCCESS;
}

// Test with integer keys
int test_hashmap_int_keys(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_int, anv_key_equals_int, 0);

    int* key1 = malloc(sizeof(int));
    int* key2 = malloc(sizeof(int));
    *key1 = 42;
    *key2 = 100;

    char* value1 = "forty-two";
    char* value2 = "one hundred";

    ASSERT_EQ(anv_hashmap_put(map, key1, value1), 0);
    ASSERT_EQ(anv_hashmap_put(map, key2, value2), 0);

    void* retrieved = anv_hashmap_get(map, key1);
    ASSERT_EQ_STR((char*)retrieved, value1);

    retrieved = anv_hashmap_get(map, key2);
    ASSERT_EQ_STR((char*)retrieved, value2);

    anv_hashmap_destroy(map, true, false); // Free the integer keys
    return TEST_SUCCESS;
}

// Test load factor and resizing
int test_hashmap_resize(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_int, anv_key_equals_int, 4); // Small initial size

    // Add enough items to trigger resize
    for (int i = 0; i < 10; i++)
    {
        int* key = malloc(sizeof(int));
        *key = i;
        char* value = malloc(20);
        sprintf(value, "value_%d", i);

        ASSERT_EQ(anv_hashmap_put(map, key, value), 0);
    }

    ASSERT_EQ(anv_hashmap_size(map), 10);

    // Verify all items are still accessible after resize
    for (int i = 0; i < 10; i++)
    {
        int key = i;
        const void* retrieved = anv_hashmap_get(map, &key);
        ASSERT_NOT_NULL(retrieved);
    }

    anv_hashmap_destroy(map, true, true); // Free keys and values
    return TEST_SUCCESS;
}

//==============================================================================
// Iterator tests
//==============================================================================

// Test basic iterator functionality
int test_hashmap_iterator_basic(void)
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
int test_hashmap_iterator_empty(void)
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
int test_hashmap_iterator_with_modifications(void)
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
int test_hashmap_iterator_multiple(void)
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
int test_hashmap_iterator_get(void)
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
int test_hashmap_iterator_backward(void)
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

// Test creating hashmap from iterator (iterator variant)
int test_hashmap_from_iterator(void)
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
int test_hashmap_iterator_invalid(void)
{
    const ANVIterator iter = anv_hashmap_iterator(NULL);
    ASSERT(!iter.is_valid(&iter));
    return TEST_SUCCESS;
}

// Test copy isolation - verify that copied elements are independent
int test_hashmap_copy_isolation(void)
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
int test_hashmap_anv_copy_function_required(void)
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
int test_hashmap_from_iterator_no_copy(void)
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
int test_hashmap_iterator_exhaustion_after_creation(void)
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
int test_hashmap_iterator_next_return_values(void)
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
int test_hashmap_iterator_mixed_operations(void)
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
    ASSERT_EQ(pair1, pair2);                                   // Same pointer
    ASSERT_EQ_STR((char*)pair1->first, (char*)pair2->first);   // Same key
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
    ASSERT(strcmp(first_key, pair3->first) != 0 ||
           strcmp(first_value, pair3->second) != 0); // Different content

    // Verify get() consistency at new position
    const ANVPair* pair4 = iter.get(&iter);
    ASSERT_EQ(pair3, pair4);                                   // Same pointer
    ASSERT_EQ_STR((char*)pair3->first, (char*)pair4->first);   // Same content
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
int test_hashmap_iterator_reset(void)
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
int test_hashmap_iterator_single_element(void)
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

//==============================================================================
// Memory tests
//==============================================================================

// Test hash map with failing allocator
int test_hashmap_failing_allocator(void)
{
    ANVAllocator alloc = create_failing_int_allocator();

    // Set to fail on first allocation
    set_alloc_fail_countdown(1);
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);
    ASSERT_NULL(map); // Should fail to create

    return TEST_SUCCESS;
}

// Test hash map node allocation failure
int test_hashmap_node_alloc_failure(void)
{
    ANVAllocator alloc = create_failing_int_allocator();

    // Allow map creation but fail on node allocation
    set_alloc_fail_countdown(2); // Let map and bucket array allocate, fail on node
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 4);
    ASSERT_NOT_NULL(map);

    // This should fail due to node allocation failure
    const int result = anv_hashmap_put(map, "test", "value");
    ASSERT_EQ(result, -1);
    ASSERT_EQ(anv_hashmap_size(map), 0);

    anv_hashmap_destroy(map, false, false);

    return TEST_SUCCESS;
}

// Test hash map resize allocation failure
int test_hashmap_resize_failure(void)
{
    ANVAllocator alloc = create_failing_int_allocator();

    set_alloc_fail_countdown(-1);
    // Create small map that will need to resize
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 2);
    ASSERT_NOT_NULL(map);

    // Add items to trigger resize, but fail the resize allocation
    ASSERT_EQ(anv_hashmap_put(map, "key1", "value1"), 0);
    ASSERT_EQ(anv_hashmap_put(map, "key2", "value2"), 0);

    // Set to fail on next allocation (which should be the resize)
    set_alloc_fail_countdown(1);
    const int result = anv_hashmap_put(map, "key3", "value3");
    // Should fail during resize, but original data should remain intact
    (void)result; // Suppress unused variable warning

    // Verify original data is still accessible
    ASSERT_NOT_NULL(anv_hashmap_get(map, "key1"));
    ASSERT_NOT_NULL(anv_hashmap_get(map, "key2"));

    anv_hashmap_destroy(map, false, false);

    return TEST_SUCCESS;
}

// Test memory management with data freeing
int test_hashmap_memory_freeing(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_int, anv_key_equals_int, 0);

    // Add dynamically allocated keys and values
    for (int i = 0; i < 5; i++)
    {
        int* key = malloc(sizeof(int));
        int* value = malloc(sizeof(int));
        *key = i;
        *value = i * 10;
        ASSERT_EQ(anv_hashmap_put(map, key, value), 0);
    }

    ASSERT_EQ(anv_hashmap_size(map), 5);

    // Remove one item with freeing
    const int key_to_remove = 2;
    ASSERT_EQ(anv_hashmap_remove(map, &key_to_remove, true, true), 0);
    ASSERT_EQ(anv_hashmap_size(map), 4);
    ASSERT_NULL(anv_hashmap_get(map, &key_to_remove));

    // Clear all remaining items with freeing
    anv_hashmap_clear(map, true, true);
    ASSERT_EQ(anv_hashmap_size(map), 0);
    ASSERT(anv_hashmap_is_empty(map));

    anv_hashmap_destroy(map, false, false);

    return TEST_SUCCESS;
}

// Test hash map copy with allocation failure
int test_hashmap_copy_failure(void)
{
    ANVAllocator good_alloc = create_int_allocator();
    ANVHashMap* original = anv_hashmap_create(&good_alloc, anv_hash_string, anv_key_equals_string, 0);

    // Add some data
    ASSERT_EQ(anv_hashmap_put(original, "key1", "value1"), 0);
    ASSERT_EQ(anv_hashmap_put(original, "key2", "value2"), 0);

    // Try to copy with failing allocator
    const ANVAllocator failing_alloc = create_failing_int_allocator();
    set_alloc_fail_countdown(1);

    // Temporarily replace allocator for copy test
    const ANVAllocator orig_alloc = original->alloc;
    original->alloc = failing_alloc;

    ANVHashMap* copy = anv_hashmap_copy(original);
    ASSERT_NULL(copy); // Should fail

    // Restore original allocator
    original->alloc = orig_alloc;

    anv_hashmap_destroy(original, false, false);

    return TEST_SUCCESS;
}

// Test deep copy with allocation failure
int test_hashmap_deep_copy_failure(void)
{
    ANVAllocator alloc = create_int_allocator();

    ANVHashMap* original = anv_hashmap_create(&alloc, anv_hash_int, anv_key_equals_int, 0);

    // Add some data
    int* key = malloc(sizeof(int));
    int* value = malloc(sizeof(int));
    *key = 42;
    *value = 100;
    ASSERT_EQ(anv_hashmap_put(original, key, value), 0);

    // Set failing copy to fail on first copy attempt
    set_alloc_fail_countdown(1);

    ANVHashMap* copy = anv_hashmap_copy_deep(original, failing_int_copy, failing_int_copy);
    ASSERT_NULL(copy); // Should fail due to copy function failure

    anv_hashmap_destroy(original, true, true);

    return TEST_SUCCESS;
}

// Test get_keys with allocation failure
int test_hashmap_get_keys_failure(void)
{
    set_alloc_fail_countdown(-1);
    ANVAllocator alloc = create_failing_int_allocator();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);
    ASSERT_NOT_NULL(map);

    // Add some data
    ASSERT_EQ(anv_hashmap_put(map, "key1", "value1"), 0);
    ASSERT_EQ(anv_hashmap_put(map, "key2", "value2"), 0);

    // Set to fail on keys array allocation
    set_alloc_fail_countdown(0);

    void** keys;
    size_t count;
    const int result = anv_hashmap_get_keys(map, &keys, &count);
    ASSERT_EQ(result, -1); // Should fail

    anv_hashmap_destroy(map, false, false);

    return TEST_SUCCESS;
}

// Test hash map with NULL key/value handling
int test_hashmap_null_handling(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    // Test NULL key
    ASSERT_EQ(anv_hashmap_put(map, NULL, "value"), -1);
    ASSERT_NULL(anv_hashmap_get(map, NULL));
    ASSERT_EQ(anv_hashmap_remove(map, NULL, false, false), -1);

    // Test NULL value (should be allowed)
    ASSERT_EQ(anv_hashmap_put(map, "key", NULL), 0);
    void* retrieved = anv_hashmap_get(map, "key");
    ASSERT_NULL(retrieved); // NULL value is valid

    anv_hashmap_destroy(map, false, false);

    return TEST_SUCCESS;
}

// Test hash map with extreme sizes
int test_hashmap_extreme_sizes(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Test with size 1 (minimal)
    ANVHashMap* small_map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 1);
    ASSERT_NOT_NULL(small_map);

    // Add multiple items to force collisions in size-1 map
    ASSERT_EQ(anv_hashmap_put(small_map, "a", "1"), 0);
    ASSERT_EQ(anv_hashmap_put(small_map, "b", "2"), 0);
    ASSERT_EQ(anv_hashmap_put(small_map, "c", "3"), 0);

    // All should be accessible despite collisions
    ASSERT_EQ_STR((char*)anv_hashmap_get(small_map, "a"), "1");
    ASSERT_EQ_STR((char*)anv_hashmap_get(small_map, "b"), "2");
    ASSERT_EQ_STR((char*)anv_hashmap_get(small_map, "c"), "3");

    anv_hashmap_destroy(small_map, false, false);

    return TEST_SUCCESS;
}

//==============================================================================
// Memory-safe tests
//==============================================================================

// Test the new anv_hashmap_put_replace function
int test_hashmap_put_replace(void)
{
    ANVAllocator alloc = create_string_allocator();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    char* key = "test_key";

    // Allocate heap values to test memory management
    char* value1 = malloc(30);
    char* value2 = malloc(30);
    char* value3 = malloc(30);
    strcpy(value1, "first_heap_value");
    strcpy(value2, "second_heap_value");
    strcpy(value3, "third_heap_value");

    void* old_value = NULL;

    // Insert first value - should return NULL for old_value
    ASSERT_EQ(anv_hashmap_put_replace(map, key, value1, &old_value), 0);
    ASSERT_EQ(old_value, NULL);
    ASSERT_EQ(anv_hashmap_size(map), 1);
    ASSERT_EQ_STR((char*)anv_hashmap_get(map, key), "first_heap_value");

    // Replace with second value - should return first value
    ASSERT_EQ(anv_hashmap_put_replace(map, key, value2, &old_value), 0);
    ASSERT_EQ(old_value, value1);
    ASSERT_EQ(anv_hashmap_size(map), 1); // Size unchanged
    ASSERT_EQ_STR((char*)anv_hashmap_get(map, key), "second_heap_value");
    free(old_value); // Clean up first value - no leak!

    // Replace with third value - should return second value
    ASSERT_EQ(anv_hashmap_put_replace(map, key, value3, &old_value), 0);
    ASSERT_EQ(old_value, value2);
    ASSERT_EQ(anv_hashmap_size(map), 1); // Size unchanged
    ASSERT_EQ_STR((char*)anv_hashmap_get(map, key), "third_heap_value");
    free(old_value); // Clean up second value - no leak!

    // Test error conditions
    ASSERT_EQ(anv_hashmap_put_replace(NULL, key, value3, &old_value), -1);
    ASSERT_EQ(anv_hashmap_put_replace(map, NULL, value3, &old_value), -1);
    ASSERT_EQ(anv_hashmap_put_replace(map, key, value3, NULL), -1);

    // Clean up final value
    char* final_value = (char*)anv_hashmap_get(map, key);
    anv_hashmap_destroy(map, false, false);
    free(final_value);

    return TEST_SUCCESS;
}

// Test the new anv_hashmap_put_with_free function
int test_hashmap_put_with_free(void)
{
    ANVAllocator alloc = anv_alloc_default();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    char* key = "auto_free_key";

    // Test with string literals (should_free_old_value = false)
    ASSERT_EQ(anv_hashmap_put_with_free(map, key, "literal1", false), 0);
    ASSERT_EQ(anv_hashmap_size(map), 1);
    ASSERT_EQ_STR((char*)anv_hashmap_get(map, key), "literal1");

    ASSERT_EQ(anv_hashmap_put_with_free(map, key, "literal2", false), 0);
    ASSERT_EQ(anv_hashmap_size(map), 1); // Size unchanged
    ASSERT_EQ_STR((char*)anv_hashmap_get(map, key), "literal2");

    // Test error conditions
    ASSERT_EQ(anv_hashmap_put_with_free(NULL, key, "value", false), -1);
    ASSERT_EQ(anv_hashmap_put_with_free(map, NULL, "value", false), -1);

    // Test that function works with new keys
    ASSERT_EQ(anv_hashmap_put_with_free(map, "new_key", "new_value", false), 0);
    ASSERT_EQ(anv_hashmap_size(map), 2);
    ASSERT_EQ_STR((char*)anv_hashmap_get(map, "new_key"), "new_value");

    anv_hashmap_destroy(map, false, false);

    return TEST_SUCCESS;
}

// Test memory leak prevention comparison
int test_memory_leak_prevention(void)
{
    ANVAllocator alloc = anv_alloc_default();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    char* key = "leak_test_key";

    // Simulate the old problematic behavior (would leak with heap values)
    // We use string literals here for safety, but this demonstrates the pattern
    char* old_style_value1 = "old_first";
    char* old_style_value2 = "old_second";

    // Old way - would leak if these were malloc'd
    anv_hashmap_put(map, key, old_style_value1);
    ASSERT_EQ_STR((char*)anv_hashmap_get(map, key), "old_first");

    anv_hashmap_put(map, key, old_style_value2); // old_first would be lost!
    ASSERT_EQ_STR((char*)anv_hashmap_get(map, key), "old_second");

    anv_hashmap_clear(map, false, false);

    // New safe way with put_replace
    void* retrieved_old = NULL;
    anv_hashmap_put_replace(map, key, "safe_first", &retrieved_old);
    ASSERT_EQ(retrieved_old, NULL); // No old value
    ASSERT_EQ_STR((char*)anv_hashmap_get(map, key), "safe_first");

    anv_hashmap_put_replace(map, key, "safe_second", &retrieved_old);
    ASSERT_EQ_STR((char*)retrieved_old, "safe_first"); // Got old value back!
    ASSERT_EQ_STR((char*)anv_hashmap_get(map, key), "safe_second");
    // In real usage with malloc'd values, we would: free(retrieved_old);

    anv_hashmap_destroy(map, false, false);

    return TEST_SUCCESS;
}

// Test multiple key updates with proper cleanup
int test_multiple_updates_cleanup(void)
{
    ANVAllocator alloc = anv_alloc_default();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    char* key = "multi_update_key";

    // Allocate multiple heap values
    char* values[5];
    for (int i = 0; i < 5; i++)
    {
        values[i] = malloc(20);
        sprintf(values[i], "value_%d", i);
    }

    void* old_value = NULL;

    // Insert first value
    ASSERT_EQ(anv_hashmap_put_replace(map, key, values[0], &old_value), 0);
    ASSERT_EQ(old_value, NULL);

    // Update multiple times, cleaning up each old value
    for (int i = 1; i < 5; i++)
    {
        ASSERT_EQ(anv_hashmap_put_replace(map, key, values[i], &old_value), 0);
        ASSERT_NOT_NULL(old_value);
        ASSERT_EQ(old_value, values[i - 1]);
        free(old_value); // Clean up previous value

        char expected[20];
        sprintf(expected, "value_%d", i);
        ASSERT_EQ_STR((char*)anv_hashmap_get(map, key), expected);
        ASSERT_EQ(anv_hashmap_size(map), 1); // Size always 1
    }

    // Clean up final value
    char* final_value = (char*)anv_hashmap_get(map, key);
    anv_hashmap_destroy(map, false, false);
    free(final_value);

    return TEST_SUCCESS;
}

//==============================================================================
// Performance tests
//==============================================================================

// Test insertion performance
int test_hashmap_performance_insertion(void)
{
    ANVAllocator alloc = anv_alloc_default();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    const int num_items = 1000;
    char keys[1000][20];
    char values[1000][20];

    // Prepare test data
    for (int i = 0; i < num_items; i++)
    {
        sprintf(keys[i], "key_%d", i);
        sprintf(values[i], "value_%d", i);
    }

    const clock_t start = clock();

    // Insert all items
    for (int i = 0; i < num_items; i++)
    {
        ASSERT_EQ(anv_hashmap_put(map, keys[i], values[i]), 0);
    }

    const clock_t end = clock();
    const double time_taken = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Inserted %d items in %f seconds\n", num_items, time_taken);
    ASSERT_LT(time_taken, 5.0);
    ASSERT_EQ(anv_hashmap_size(map), (size_t)num_items);

    anv_hashmap_destroy(map, false, false);

    return TEST_SUCCESS;
}

// Test lookup performance
int test_hashmap_performance_lookup(void)
{
    ANVAllocator alloc = anv_alloc_default();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    const int num_items = 1000;
    char keys[1000][20];
    char values[1000][20];

    // Prepare and insert test data
    for (int i = 0; i < num_items; i++)
    {
        sprintf(keys[i], "key_%d", i);
        sprintf(values[i], "value_%d", i);
        anv_hashmap_put(map, keys[i], values[i]);
    }

    const clock_t start = clock();

    // Perform lookups
    for (int i = 0; i < num_items; i++)
    {
        void* result = anv_hashmap_get(map, keys[i]);
        ASSERT_NOT_NULL(result);
        ASSERT_EQ_STR((char*)result, values[i]);
    }

    const clock_t end = clock();
    const double time_taken = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Performed %d lookups in %f seconds\n", num_items, time_taken);
    ASSERT_LT(time_taken, 5.0);

    anv_hashmap_destroy(map, false, false);

    return TEST_SUCCESS;
}

// Test removal performance
int test_hashmap_performance_removal(void)
{
    ANVAllocator alloc = anv_alloc_default();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    const int num_items = 1000;
    char keys[1000][20];
    char values[1000][20];

    // Prepare and insert test data
    for (int i = 0; i < num_items; i++)
    {
        sprintf(keys[i], "key_%d", i);
        sprintf(values[i], "value_%d", i);
        anv_hashmap_put(map, keys[i], values[i]);
    }

    const clock_t start = clock();

    // Remove all items
    for (int i = 0; i < num_items; i++)
    {
        ASSERT_EQ(anv_hashmap_remove(map, keys[i], false, false), 0);
    }

    const clock_t end = clock();
    const double time_taken = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Removed %d items in %f seconds\n", num_items, time_taken);
    ASSERT_LT(time_taken, 5.0);
    ASSERT_EQ(anv_hashmap_size(map), 0);

    anv_hashmap_destroy(map, false, false);

    return TEST_SUCCESS;
}

// Test copy performance
int test_hashmap_performance_copy(void)
{
    ANVAllocator alloc = anv_alloc_default();
    ANVHashMap* original = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    const int num_items = 1000;
    char keys[1000][20];
    char values[1000][20];

    // Prepare and insert test data
    for (int i = 0; i < num_items; i++)
    {
        sprintf(keys[i], "key_%d", i);
        sprintf(values[i], "value_%d", i);
        anv_hashmap_put(original, keys[i], values[i]);
    }

    const clock_t start = clock();

    ANVHashMap* copy = anv_hashmap_copy(original);

    const clock_t end = clock();
    const double time_taken = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Copied %d items in %f seconds\n", num_items, time_taken);
    ASSERT_LT(time_taken, 5.0);
    ASSERT_NOT_NULL(copy);
    ASSERT_EQ(anv_hashmap_size(copy), (size_t)num_items);

    anv_hashmap_destroy(original, false, false);
    anv_hashmap_destroy(copy, false, false);

    return TEST_SUCCESS;
}

// Test iteration performance
int test_hashmap_performance_iteration(void)
{
    ANVAllocator alloc = anv_alloc_default();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    const int num_items = 1000;
    char keys[1000][20];
    char values[1000][20];

    // Prepare and insert test data
    for (int i = 0; i < num_items; i++)
    {
        sprintf(keys[i], "key_%d", i);
        sprintf(values[i], "value_%d", i);
        anv_hashmap_put(map, keys[i], values[i]);
    }

    const clock_t start = clock();

    // Iterate through all items
    ANVIterator it = anv_hashmap_iterator(map);
    int visited_count = 0;
    while (it.has_next(&it))
    {
        const ANVPair* pair = it.get(&it);
        ASSERT_NOT_NULL(pair);
        visited_count++;
        it.next(&it);
    }

    const clock_t end = clock();
    const double time_taken = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Iterated through %d items in %f seconds\n", visited_count, time_taken);
    ASSERT_LT(time_taken, 5.0);
    ASSERT_EQ(visited_count, num_items);

    it.destroy(&it);
    anv_hashmap_destroy(map, false, false);

    return TEST_SUCCESS;
}

// Test resize performance under heavy load
int test_hashmap_performance_resize(void)
{
    ANVAllocator alloc = anv_alloc_default();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 4); // Small initial size

    const int num_items = 1000;
    char keys[1000][20];
    char values[1000][20];

    // Prepare test data
    for (int i = 0; i < num_items; i++)
    {
        sprintf(keys[i], "key_%d", i);
        sprintf(values[i], "value_%d", i);
    }

    const clock_t start = clock();

    // Insert items that will trigger multiple resizes
    for (int i = 0; i < num_items; i++)
    {
        ASSERT_EQ(anv_hashmap_put(map, keys[i], values[i]), 0);
    }

    const clock_t end = clock();
    const double time_taken = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Inserted %d items with resizing in %f seconds\n", num_items, time_taken);
    ASSERT_LT(time_taken, 5.0);
    ASSERT_EQ(anv_hashmap_size(map), (size_t)num_items);

    // Verify all items are still accessible
    for (int i = 0; i < num_items; i++)
    {
        const void* result = anv_hashmap_get(map, keys[i]);
        ASSERT_NOT_NULL(result);
    }

    anv_hashmap_destroy(map, false, false);

    return TEST_SUCCESS;
}

//==============================================================================
// Properties tests
//==============================================================================

// Test hash map size property
int test_hashmap_size_property(void)
{
    ANVAllocator alloc = create_string_allocator();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    // Declare arrays outside loops to avoid scope issues
    char keys[10][10], values[10][10];

    // Initially empty
    ASSERT_EQ(anv_hashmap_size(map), 0);
    ASSERT(anv_hashmap_is_empty(map));

    // Add items and verify size increases
    for (int i = 0; i < 10; i++)
    {
        sprintf(keys[i], "key%d", i);
        sprintf(values[i], "val%d", i);

        ASSERT_EQ(anv_hashmap_put(map, keys[i], values[i]), 0);
        ASSERT_EQ(anv_hashmap_size(map), (size_t)i + 1);
        ASSERT(!anv_hashmap_is_empty(map));
    }

    // Remove items and verify size decreases
    for (int i = 0; i < 5; i++)
    {
        ASSERT_EQ(anv_hashmap_remove(map, keys[i], false, false), 0);
        ASSERT_EQ(anv_hashmap_size(map), 10 - (size_t)i - 1);
    }

    // Clear and verify empty
    anv_hashmap_clear(map, false, false);
    ASSERT_EQ(anv_hashmap_size(map), 0);
    ASSERT(anv_hashmap_is_empty(map));

    anv_hashmap_destroy(map, false, false);
    return TEST_SUCCESS;
}

// Test hash map uniqueness property
int test_hashmap_uniqueness_property(void)
{
    ANVAllocator alloc = anv_alloc_default();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    char* key = "duplicate_key";
    char* value1 = "first_value";
    char* value2 = "second_value";
    char* value3 = "third_value";

    // Test 1: Basic uniqueness with string literals (no memory leak concern)
    ASSERT_EQ(anv_hashmap_put(map, key, value1), 0);
    ASSERT_EQ(anv_hashmap_size(map), 1);
    ASSERT_EQ_STR((char*)anv_hashmap_get(map, key), value1);

    ASSERT_EQ(anv_hashmap_put(map, key, value2), 0);
    ASSERT_EQ(anv_hashmap_size(map), 1); // Size should remain 1
    ASSERT_EQ_STR((char*)anv_hashmap_get(map, key), value2);

    ASSERT_EQ(anv_hashmap_put(map, key, value3), 0);
    ASSERT_EQ(anv_hashmap_size(map), 1); // Size should remain 1
    ASSERT_EQ_STR((char*)anv_hashmap_get(map, key), value3);

    anv_hashmap_clear(map, false, false);

    // Test 2: Memory-safe replacement with put_replace
    char* heap_value1 = malloc(20);
    char* heap_value2 = malloc(20);
    strcpy(heap_value1, "heap_first");
    strcpy(heap_value2, "heap_second");

    // Insert first value
    void* old_value = NULL;
    ASSERT_EQ(anv_hashmap_put_replace(map, key, heap_value1, &old_value), 0);
    ASSERT_EQ(anv_hashmap_size(map), 1);
    ASSERT_EQ(old_value, NULL); // No old value for new key
    ASSERT_EQ_STR((char*)anv_hashmap_get(map, key), "heap_first");

    // Replace with second value - get old value back for proper cleanup
    ASSERT_EQ(anv_hashmap_put_replace(map, key, heap_value2, &old_value), 0);
    ASSERT_EQ(anv_hashmap_size(map), 1); // Size should remain 1
    ASSERT_EQ(old_value, heap_value1);   // Should return the old value
    ASSERT_EQ_STR((char*)anv_hashmap_get(map, key), "heap_second");

    // Clean up the old value (no memory leak!)
    anv_alloc_deallocate(&alloc, old_value);

    // Clean up remaining value
    char* final_value = (char*)anv_hashmap_get(map, key);
    anv_hashmap_clear(map, false, false);
    free(final_value);

    // Test 3: Automatic cleanup with put_with_free
    // Note: This test uses string literals since we can't easily test with malloc'd values
    // in a unit test that uses the map's allocator for cleanup
    ASSERT_EQ(anv_hashmap_put_with_free(map, key, "auto_first", false), 0);
    ASSERT_EQ(anv_hashmap_size(map), 1);
    ASSERT_EQ_STR((char*)anv_hashmap_get(map, key), "auto_first");

    ASSERT_EQ(anv_hashmap_put_with_free(map, key, "auto_second", false), 0);
    ASSERT_EQ(anv_hashmap_size(map), 1); // Size should remain 1
    ASSERT_EQ_STR((char*)anv_hashmap_get(map, key), "auto_second");

    anv_hashmap_destroy(map, false, false);
    return TEST_SUCCESS;
}

// Test hash map load factor property
int test_hashmap_load_factor_property(void)
{
    ANVAllocator alloc = anv_alloc_default();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 8);

    // Declare arrays outside loop to avoid scope issues
    char keys[4][10], values[4][10];

    // Initially empty, load factor should be 0
    ASSERT_EQ(anv_hashmap_load_factor(map), 0.0);

    // Add items and check load factor increases
    for (int i = 0; i < 4; i++)
    {
        sprintf(keys[i], "key%d", i);
        sprintf(values[i], "val%d", i);

        ASSERT_EQ(anv_hashmap_put(map, keys[i], values[i]), 0);

        const double expected_lf = (double)(i + 1) / 8.0;
        const double actual_lf = anv_hashmap_load_factor(map);
        ASSERT(actual_lf >= expected_lf - 0.01 && actual_lf <= expected_lf + 0.01);
    }

    // Load factor should be 0.5 with 4 items in 8 buckets
    const double lf = anv_hashmap_load_factor(map);
    ASSERT(lf >= 0.49 && lf <= 0.51);

    anv_hashmap_destroy(map, false, false);
    return TEST_SUCCESS;
}

// Test hash map automatic resizing property
int test_hashmap_resize_property(void)
{
    ANVAllocator alloc = anv_alloc_default();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 4);

    // Declare arrays outside loops to avoid scope issues
    char keys[4][10], values[4][10];

    // Add items until resize triggers (load factor > 0.75)
    int items_added = 0;
    double initial_lf = 0.0;

    // Add 3 items (load factor = 0.75, at threshold)
    for (int i = 0; i < 3; i++)
    {
        sprintf(keys[i], "key%d", i);
        sprintf(values[i], "val%d", i);

        ASSERT_EQ(anv_hashmap_put(map, keys[i], values[i]), 0);
        items_added++;
    }

    initial_lf = anv_hashmap_load_factor(map);
    ASSERT(initial_lf >= 0.74 && initial_lf <= 0.76); // Should be ~0.75

    // Add one more item to trigger resize
    ASSERT_EQ(anv_hashmap_put(map, "trigger", "resize"), 0);
    items_added++;

    // After resize, load factor should be lower
    const double post_resize_lf = anv_hashmap_load_factor(map);
    ASSERT(post_resize_lf < initial_lf);
    ASSERT_EQ(anv_hashmap_size(map), (size_t)items_added);

    // All original items should still be accessible
    for (int i = 0; i < 3; i++)
    {
        ASSERT_NOT_NULL(anv_hashmap_get(map, keys[i]));
    }
    ASSERT_NOT_NULL(anv_hashmap_get(map, "trigger"));

    anv_hashmap_destroy(map, false, false);
    return TEST_SUCCESS;
}

// Test hash map key equality property
int test_hashmap_key_equality_property(void)
{
    ANVAllocator alloc = anv_alloc_default();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    // String literals might have different addresses but same content
    char* key1 = "test_key";
    char key2[] = "test_key";
    char* value1 = "value1";
    char* value2 = "value2";

    // Insert with first key
    ASSERT_EQ(anv_hashmap_put(map, key1, value1), 0);
    ASSERT_EQ(anv_hashmap_size(map), 1);

    // Update with equivalent key (different pointer, same content)
    ASSERT_EQ(anv_hashmap_put(map, key2, value2), 0);
    ASSERT_EQ(anv_hashmap_size(map), 1); // Should be updated, not new entry

    // Both keys should retrieve the updated value
    ASSERT_EQ_STR((char*)anv_hashmap_get(map, key1), value2);
    ASSERT_EQ_STR((char*)anv_hashmap_get(map, key2), value2);

    anv_hashmap_destroy(map, false, false);
    return TEST_SUCCESS;
}

// Test hash map contains property
int test_hashmap_contains_property(void)
{
    ANVAllocator alloc = anv_alloc_default();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    char* keys[] = {"apple", "banana", "cherry"};
    char* values[] = {"red", "yellow", "red"};

    // Initially no keys present
    for (int i = 0; i < 3; i++)
    {
        ASSERT(!anv_hashmap_contains_key(map, keys[i]));
    }

    // Add keys one by one and verify contains property
    for (int i = 0; i < 3; i++)
    {
        ASSERT_EQ(anv_hashmap_put(map, keys[i], values[i]), 0);

        // Current key should be contained
        ASSERT(anv_hashmap_contains_key(map, keys[i]));

        // All previously added keys should still be contained
        for (int j = 0; j <= i; j++)
        {
            ASSERT(anv_hashmap_contains_key(map, keys[j]));
        }

        // Keys not yet added should not be contained
        for (int j = i + 1; j < 3; j++)
        {
            ASSERT(!anv_hashmap_contains_key(map, keys[j]));
        }
    }

    // Remove keys and verify contains property
    for (int i = 0; i < 3; i++)
    {
        ASSERT_EQ(anv_hashmap_remove(map, keys[i], false, false), 0);

        // Removed key should not be contained
        ASSERT(!anv_hashmap_contains_key(map, keys[i]));

        // Remaining keys should still be contained
        for (int j = i + 1; j < 3; j++)
        {
            ASSERT(anv_hashmap_contains_key(map, keys[j]));
        }
    }

    anv_hashmap_destroy(map, false, false);
    return TEST_SUCCESS;
}

// Test hash map iterator completeness property - verifies that iteration
// visits every element exactly once (mathematical property, not API behavior)
int test_hashmap_iterator_completeness(void)
{
    ANVAllocator alloc = anv_alloc_default();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    const int num_items = 20;
    char keys[20][10];
    char values[20][10];

    // Add test data
    for (int i = 0; i < num_items; i++)
    {
        sprintf(keys[i], "key%d", i);
        sprintf(values[i], "val%d", i);
        ASSERT_EQ(anv_hashmap_put(map, keys[i], values[i]), 0);
    }

    // Use iterator to visit all items
    ANVIterator it = anv_hashmap_iterator(map);
    int visited_count = 0;
    int found_flags[20] = {0}; // Track which items we've seen

    while (it.has_next(&it))
    {
        const ANVPair* pair = it.get(&it);
        ASSERT_NOT_NULL(pair);

        // Find which item this is
        int found_index = -1;
        for (int i = 0; i < num_items; i++)
        {
            if (strcmp(pair->first, keys[i]) == 0)
            {
                found_index = i;
                break;
            }
        }

        ASSERT(found_index >= 0);          // Should find the key
        ASSERT(!found_flags[found_index]); // Should not have seen it before
        found_flags[found_index] = 1;

        // Verify value matches
        ASSERT_EQ_STR((char*)pair->second, values[found_index]);
        visited_count++;
        it.next(&it);
    }

    // Should have visited exactly all items
    ASSERT_EQ(visited_count, num_items);

    // Should have seen each item exactly once
    for (int i = 0; i < num_items; i++)
    {
        ASSERT(found_flags[i]);
    }

    it.destroy(&it);
    anv_hashmap_destroy(map, false, false);
    return TEST_SUCCESS;
}

// Test hash map with different hash functions
int test_hashmap_anv_hash_function_property(void)
{
    ANVAllocator alloc = anv_alloc_default();

    // Test with string hash
    ANVHashMap* str_map = anv_hashmap_create(&alloc, anv_hash_string, anv_key_equals_string, 0);
    ASSERT_EQ(anv_hashmap_put(str_map, "test", "value"), 0);
    ASSERT_EQ_STR((char*)anv_hashmap_get(str_map, "test"), "value");

    // Test with integer hash
    ANVHashMap* int_map = anv_hashmap_create(&alloc, anv_hash_int, anv_key_equals_int, 0);
    int key = 42;
    ASSERT_EQ(anv_hashmap_put(int_map, &key, "forty-two"), 0);
    ASSERT_EQ_STR((char*)anv_hashmap_get(int_map, &key), "forty-two");

    // Test with pointer hash
    ANVHashMap* ptr_map = anv_hashmap_create(&alloc, anv_hash_pointer, anv_key_equals_pointer, 0);
    void* ptr_key = (void*)0x12345678;
    ASSERT_EQ(anv_hashmap_put(ptr_map, ptr_key, "pointer_value"), 0);
    ASSERT_EQ_STR((char*)anv_hashmap_get(ptr_map, ptr_key), "pointer_value");

    anv_hashmap_destroy(str_map, false, false);
    anv_hashmap_destroy(int_map, false, false);
    anv_hashmap_destroy(ptr_map, false, false);
    return TEST_SUCCESS;
}

//==============================================================================
// Stress & Collision Tests
//==============================================================================

// Custom hash that always returns the same bucket — forces worst-case chaining
static size_t collision_hash(const void* key)
{
    (void)key;
    return 42; // Every key hashes to the same bucket
}

int test_hashmap_collision_stress(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashMap* map = anv_hashmap_create(&alloc, collision_hash, anv_key_equals_int, 4);
    ASSERT_NOT_NULL(map);

    const int N = 200;
    int* keys[200];
    int* values[200];

    // Insert N items that all collide
    for (int i = 0; i < N; i++)
    {
        keys[i] = malloc(sizeof(int));
        values[i] = malloc(sizeof(int));
        *keys[i] = i;
        *values[i] = i * 100;
        ASSERT_EQ(anv_hashmap_put(map, keys[i], values[i]), 0);
    }
    ASSERT_EQ(anv_hashmap_size(map), (size_t)N);

    // All items should be retrievable despite collisions
    for (int i = 0; i < N; i++)
    {
        int* val = anv_hashmap_get(map, keys[i]);
        ASSERT_NOT_NULL(val);
        ASSERT_EQ(*val, i * 100);
    }

    // Remove half of them
    for (int i = 0; i < N / 2; i++)
    {
        ASSERT_EQ(anv_hashmap_remove(map, keys[i], true, true), 0);
    }
    ASSERT_EQ(anv_hashmap_size(map), (size_t)(N / 2));

    // Remaining half still accessible
    for (int i = N / 2; i < N; i++)
    {
        int* val = anv_hashmap_get(map, keys[i]);
        ASSERT_NOT_NULL(val);
        ASSERT_EQ(*val, i * 100);
    }

    anv_hashmap_destroy(map, true, true);
    return TEST_SUCCESS;
}

int test_hashmap_high_load_factor(void)
{
    ANVAllocator alloc = create_int_allocator();
    // Very small initial capacity to force many resizes
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_int, anv_key_equals_int, 2);
    ASSERT_NOT_NULL(map);

    const int N = 10000;
    int* keys[10000];
    int* values[10000];

    for (int i = 0; i < N; i++)
    {
        keys[i] = malloc(sizeof(int));
        values[i] = malloc(sizeof(int));
        *keys[i] = i;
        *values[i] = i;
        ASSERT_EQ(anv_hashmap_put(map, keys[i], values[i]), 0);
    }
    ASSERT_EQ(anv_hashmap_size(map), (size_t)N);

    // Verify all items after many resizes
    for (int i = 0; i < N; i++)
    {
        int* val = anv_hashmap_get(map, keys[i]);
        ASSERT_NOT_NULL(val);
        ASSERT_EQ(*val, i);
    }

    // Check load factor is reasonable (should have resized to keep it <1)
    ASSERT_LT(anv_hashmap_load_factor(map), 1.0);

    anv_hashmap_destroy(map, true, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Fuzz Tests
//==============================================================================

int test_hashmap_fuzz(void)
{
    srand((unsigned int)42);
    ANVAllocator alloc = create_int_allocator();
    ANVHashMap* map = anv_hashmap_create(&alloc, anv_hash_int, anv_key_equals_int, 4);
    ASSERT_NOT_NULL(map);

    // Track which keys are live so we can verify size
    size_t expected_size = 0;

    for (int i = 0; i < 50000; i++)
    {
        const unsigned op = rand() % 4;
        const int key_val = rand() % 500; // Limited key space to force collisions & updates

        switch (op)
        {
            case 0: // put (insert or update)
                {
                    int* key = malloc(sizeof(int));
                    int* value = malloc(sizeof(int));
                    *key = key_val;
                    *value = rand();

                    const int already_exists = anv_hashmap_contains_key(map, key);
                    const int rc = anv_hashmap_put_with_free(map, key, value, true);
                    if (rc == 0 && !already_exists)
                        expected_size++;

                    if (rc != 0)
                        free(key);
                    break;
                }
            case 1: // get
                {
                    const void* val = anv_hashmap_get(map, &key_val);
                    if (anv_hashmap_contains_key(map, &key_val))
                    {
                        ASSERT_NOT_NULL(val);
                    }
                    break;
                }
            case 2: // remove
                {
                    if (anv_hashmap_contains_key(map, &key_val))
                    {
                        ASSERT_EQ(anv_hashmap_remove(map, &key_val, true, true), 0);
                        expected_size--;
                    }
                    break;
                }
            case 3: // contains_key
                {
                    anv_hashmap_contains_key(map, &key_val);
                    break;
                }
            default:
                break;
        }

        // Invariant: size must always match
        ASSERT_EQ(anv_hashmap_size(map), expected_size);
    }

    anv_hashmap_destroy(map, true, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Main
//==============================================================================

int main(void)
{
    const ANVTestCase tests[] = {
        // Algorithms
        TEST_REGISTER(test_hashmap_copy_shallow),
        TEST_REGISTER(test_hashmap_copy_deep),
        TEST_REGISTER(test_hashmap_for_each),
        TEST_REGISTER(test_hashmap_get_keys),
        TEST_REGISTER(test_hashmap_get_values),
        TEST_REGISTER(test_hashmap_from_iterator_algorithms),

        // CRUD
        TEST_REGISTER(test_hashmap_create_destroy),
        TEST_REGISTER(test_hashmap_put_get),
        TEST_REGISTER(test_hashmap_update),
        TEST_REGISTER(test_hashmap_remove),
        TEST_REGISTER(test_hashmap_remove_get),
        TEST_REGISTER(test_hashmap_contains),
        TEST_REGISTER(test_hashmap_int_keys),
        TEST_REGISTER(test_hashmap_resize),

        // Iterator
        TEST_REGISTER(test_hashmap_iterator_basic),
        TEST_REGISTER(test_hashmap_iterator_empty),
        TEST_REGISTER(test_hashmap_iterator_with_modifications),
        TEST_REGISTER(test_hashmap_iterator_multiple),
        TEST_REGISTER(test_hashmap_iterator_get),
        TEST_REGISTER(test_hashmap_iterator_backward),
        TEST_REGISTER(test_hashmap_from_iterator),
        TEST_REGISTER(test_hashmap_iterator_invalid),
        TEST_REGISTER(test_hashmap_copy_isolation),
        TEST_REGISTER(test_hashmap_anv_copy_function_required),
        TEST_REGISTER(test_hashmap_from_iterator_no_copy),
        TEST_REGISTER(test_hashmap_iterator_exhaustion_after_creation),
        TEST_REGISTER(test_hashmap_iterator_next_return_values),
        TEST_REGISTER(test_hashmap_iterator_mixed_operations),
        TEST_REGISTER(test_hashmap_iterator_reset),
        TEST_REGISTER(test_hashmap_iterator_single_element),

        // Memory
        TEST_REGISTER(test_hashmap_failing_allocator),
        TEST_REGISTER(test_hashmap_node_alloc_failure),
        TEST_REGISTER(test_hashmap_resize_failure),
        TEST_REGISTER(test_hashmap_memory_freeing),
        TEST_REGISTER(test_hashmap_copy_failure),
        TEST_REGISTER(test_hashmap_deep_copy_failure),
        TEST_REGISTER(test_hashmap_get_keys_failure),
        TEST_REGISTER(test_hashmap_null_handling),
        TEST_REGISTER(test_hashmap_extreme_sizes),

        // Memory-safe
        TEST_REGISTER(test_hashmap_put_replace),
        TEST_REGISTER(test_hashmap_put_with_free),
        TEST_REGISTER(test_memory_leak_prevention),
        TEST_REGISTER(test_multiple_updates_cleanup),

        // Performance
        TEST_REGISTER(test_hashmap_performance_insertion),
        TEST_REGISTER(test_hashmap_performance_lookup),
        TEST_REGISTER(test_hashmap_performance_removal),
        TEST_REGISTER(test_hashmap_performance_copy),
        TEST_REGISTER(test_hashmap_performance_iteration),
        TEST_REGISTER(test_hashmap_performance_resize),

        // Properties
        TEST_REGISTER(test_hashmap_size_property),
        TEST_REGISTER(test_hashmap_uniqueness_property),
        TEST_REGISTER(test_hashmap_load_factor_property),
        TEST_REGISTER(test_hashmap_resize_property),
        TEST_REGISTER(test_hashmap_key_equality_property),
        TEST_REGISTER(test_hashmap_contains_property),
        TEST_REGISTER(test_hashmap_iterator_completeness),
        TEST_REGISTER(test_hashmap_anv_hash_function_property),

        // Stress & Collision
        TEST_REGISTER(test_hashmap_collision_stress),
        TEST_REGISTER(test_hashmap_high_load_factor),

        // Fuzz
        TEST_REGISTER(test_hashmap_fuzz),
    };

    return anv_run_tests("HashMap", tests, sizeof(tests) / sizeof(tests[0]));
}