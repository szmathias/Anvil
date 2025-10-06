//
// Created by zack on 9/9/25.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "containers/hashmap.h"
#include "TestAssert.h"
#include "TestHelpers.h"

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
    ANVAllocator failing_alloc = create_failing_int_allocator();
    set_alloc_fail_countdown(1);

    // Temporarily replace allocator for copy test
    ANVAllocator* orig_alloc = original->alloc;
    original->alloc = &failing_alloc;

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

typedef struct
{
    int (*func)(void);
    const char* name;
} TestCase;

int main(void)
{
    const TestCase tests[] = {
        {test_hashmap_failing_allocator, "test_hashmap_failing_allocator"},
        {test_hashmap_node_alloc_failure, "test_hashmap_node_alloc_failure"},
        {test_hashmap_resize_failure, "test_hashmap_resize_failure"},
        {test_hashmap_memory_freeing, "test_hashmap_memory_freeing"},
        {test_hashmap_copy_failure, "test_hashmap_copy_failure"},
        {test_hashmap_deep_copy_failure, "test_hashmap_deep_copy_failure"},
        {test_hashmap_get_keys_failure, "test_hashmap_get_keys_failure"},
        {test_hashmap_null_handling, "test_hashmap_null_handling"},
        {test_hashmap_extreme_sizes, "test_hashmap_extreme_sizes"},
    };

    printf("Running HashMap memory tests...\n");

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
        printf("All HashMap memory tests passed!\n");
        return 0;
    }

    printf("%d HashMap memory tests failed.\n", failed);
    return 1;
}