//
// HashMap memory-safe test - converted from HashTable memory-safe test
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "containers/hashmap.h"
#include "TestAssert.h"
#include "TestHelpers.h"

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

typedef struct
{
    int (*func)(void);
    const char* name;
} TestCase;

int main(void)
{
    const TestCase tests[] = {
        {test_hashmap_put_replace, "test_hashmap_put_replace"},
        {test_hashmap_put_with_free, "test_hashmap_put_with_free"},
        {test_memory_leak_prevention, "test_memory_leak_prevention"},
        {test_multiple_updates_cleanup, "test_multiple_updates_cleanup"},
    };

    printf("Running memory-safe HashMap function tests...\n");

    int failed = 0;
    const int num_tests = sizeof(tests) / sizeof(tests[0]);
    for (int i = 0; i < num_tests; i++)
    {
        printf("Running %s... ", tests[i].name);
        if (tests[i].func() != TEST_SUCCESS)
        {
            printf("FAILED\n");
            failed++;
        }
        else
        {
            printf("PASSED\n");
        }
    }

    if (failed == 0)
    {
        printf("\nAll memory-safe HashMap function tests passed!\n");
        return 0;
    }

    printf("\n%d memory-safe HashMap function tests failed.\n", failed);
    return 1;
}