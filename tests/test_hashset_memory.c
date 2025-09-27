//
// HashSet Memory test
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "containers/hashset.h"
#include "TestAssert.h"
#include "TestHelpers.h"

// Test memory allocation and deallocation
int test_hashset_memory_basic(void)
{
    ANVAllocator alloc = create_int_allocator();

    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);
    ASSERT_NOT_NULL(set);
    ASSERT_EQ(anv_hashset_size(set), 0);
    ASSERT(anv_hashset_is_empty(set));

    anv_hashset_destroy(set, false);
    return TEST_SUCCESS;
}

// Test memory with key freeing
int test_hashset_memory_with_key_freeing(void)
{
    ANVAllocator alloc = create_int_allocator();

    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    // Add some dynamically allocated keys
    for (int i = 0; i < 5; i++)
    {
        char* key = malloc(32);
        sprintf(key, "key_%d", i);
        ASSERT_EQ(anv_hashset_add(set, key), 0);
    }

    ASSERT_EQ(anv_hashset_size(set), 5);

    // Clear with key freeing
    anv_hashset_clear(set, true);
    ASSERT_EQ(anv_hashset_size(set), 0);
    ASSERT(anv_hashset_is_empty(set));

    anv_hashset_destroy(set, false);
    return TEST_SUCCESS;
}

// Test memory with copy operations
int test_hashset_memory_copy(void)
{
    ANVAllocator alloc = create_int_allocator();

    ANVHashSet* original = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    // Add some keys
    ASSERT_EQ(anv_hashset_add(original, "key1"), 0);
    ASSERT_EQ(anv_hashset_add(original, "key2"), 0);

    // Shallow copy
    ANVHashSet* copy = anv_hashset_copy(original);
    ASSERT_NOT_NULL(copy);

    // Verify copy contents
    ASSERT_EQ(anv_hashset_size(copy), 2);
    ASSERT(anv_hashset_contains(copy, "key1"));
    ASSERT(anv_hashset_contains(copy, "key2"));

    anv_hashset_destroy(original, false);
    anv_hashset_destroy(copy, false);
    return TEST_SUCCESS;
}

// Test memory with deep copy operations
int test_hashset_memory_deep_copy(void)
{
    ANVAllocator alloc = create_int_allocator();

    ANVHashSet* original = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    // Add some dynamically allocated keys
    char* key1 = malloc(32);
    char* key2 = malloc(32);
    strcpy(key1, "dynamic_key1");
    strcpy(key2, "dynamic_key2");

    ASSERT_EQ(anv_hashset_add(original, key1), 0);
    ASSERT_EQ(anv_hashset_add(original, key2), 0);

    // Deep copy with string copy function (use string_copy instead of int_copy)
    ANVHashSet* copy = anv_hashset_copy_deep(original, string_copy);
    ASSERT_NOT_NULL(copy);

    // Verify copy contents
    ASSERT_EQ(anv_hashset_size(copy), 2);

    // Destroy original (with key freeing) - copy should still work
    anv_hashset_destroy(original, true);

    // Copy should still have its elements
    ASSERT_EQ(anv_hashset_size(copy), 2);

    anv_hashset_destroy(copy, true);
    return TEST_SUCCESS;
}

// Test memory with get_elements operation
int test_hashset_memory_get_elements(void)
{
    ANVAllocator alloc = create_int_allocator();

    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    ASSERT_EQ(anv_hashset_add(set, "key1"), 0);
    ASSERT_EQ(anv_hashset_add(set, "key2"), 0);
    ASSERT_EQ(anv_hashset_add(set, "key3"), 0);

    void** keys;
    size_t count;
    ASSERT_EQ(anv_hashset_get_elements(set, &keys, &count), 0);

    ASSERT_EQ(count, 3);
    ASSERT_NOT_NULL(keys);

    // Free the allocated array
    free(keys);

    anv_hashset_destroy(set, false);
    return TEST_SUCCESS;
}

// Test memory with iterator operations
int test_hashset_memory_iterator(void)
{
    ANVAllocator alloc = create_int_allocator();

    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    ASSERT_EQ(anv_hashset_add(set, "key1"), 0);
    ASSERT_EQ(anv_hashset_add(set, "key2"), 0);

    // Create iterator
    ANVIterator it = anv_hashset_iterator(set);

    // Use iterator
    int count = 0;
    while (it.has_next(&it))
    {
        const void* key = it.get(&it);
        it.next(&it);
        ASSERT_NOT_NULL(key);
        count++;
    }
    ASSERT_EQ(count, 2);

    // Destroy iterator
    it.destroy(&it);

    anv_hashset_destroy(set, false);
    return TEST_SUCCESS;
}

// Test memory with set operations
int test_hashset_memory_set_operations(void)
{
    ANVAllocator alloc = create_int_allocator();

    ANVHashSet* set1 = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);
    ANVHashSet* set2 = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    ASSERT_EQ(anv_hashset_add(set1, "a"), 0);
    ASSERT_EQ(anv_hashset_add(set1, "b"), 0);
    ASSERT_EQ(anv_hashset_add(set2, "b"), 0);
    ASSERT_EQ(anv_hashset_add(set2, "c"), 0);

    // Test union
    ANVHashSet* union_set = anv_hashset_union(set1, set2);
    ASSERT_NOT_NULL(union_set);
    ASSERT_EQ(anv_hashset_size(union_set), 3);

    // Test intersection
    ANVHashSet* intersection_set = anv_hashset_intersection(set1, set2);
    ASSERT_NOT_NULL(intersection_set);
    ASSERT_EQ(anv_hashset_size(intersection_set), 1);

    // Test difference
    ANVHashSet* difference_set = anv_hashset_difference(set1, set2);
    ASSERT_NOT_NULL(difference_set);
    ASSERT_EQ(anv_hashset_size(difference_set), 1);

    anv_hashset_destroy(set1, false);
    anv_hashset_destroy(set2, false);
    anv_hashset_destroy(union_set, false);
    anv_hashset_destroy(intersection_set, false);
    anv_hashset_destroy(difference_set, false);

    return TEST_SUCCESS;
}

// Test memory operations without leaks
int test_hashset_memory_no_leaks(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Perform various operations
    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 16);

    // Add and remove elements using dynamically allocated keys
    char** keys = malloc(10 * sizeof(char*));
    for (int i = 0; i < 10; i++)
    {
        keys[i] = malloc(32);
        sprintf(keys[i], "key_%d", i);
        ASSERT_EQ(anv_hashset_add(set, keys[i]), 0);
    }
    ASSERT_EQ(anv_hashset_size(set), 10);

    // Remove some elements
    ASSERT_EQ(anv_hashset_remove(set, keys[0], true), 0); // Free the key
    ASSERT_EQ(anv_hashset_remove(set, keys[5], true), 0); // Free the key
    ASSERT_EQ(anv_hashset_size(set), 8);

    // Clear and add again
    anv_hashset_clear(set, true); // Free remaining keys
    ASSERT_EQ(anv_hashset_size(set), 0);

    char* final_key = malloc(32);
    strcpy(final_key, "final_key");
    ASSERT_EQ(anv_hashset_add(set, final_key), 0);
    ASSERT_EQ(anv_hashset_size(set), 1);

    // Test iterator
    ANVIterator it = anv_hashset_iterator(set);
    int count = 0;
    while (it.has_next(&it))
    {
        it.next(&it);
        count++;
    }
    ASSERT_EQ(count, 1);
    it.destroy(&it);

    anv_hashset_destroy(set, true); // Free final key
    free(keys);
    return TEST_SUCCESS;
}

// Main test runner
typedef struct
{
    int (*func)(void);
    const char* name;
} TestCase;

int main(void)
{
    const TestCase tests[] = {
        {test_hashset_memory_basic, "test_hashset_memory_basic"},
        {test_hashset_memory_with_key_freeing, "test_hashset_memory_with_key_freeing"},
        {test_hashset_memory_copy, "test_hashset_memory_copy"},
        {test_hashset_memory_deep_copy, "test_hashset_memory_deep_copy"},
        {test_hashset_memory_get_elements, "test_hashset_memory_get_elements"},
        {test_hashset_memory_iterator, "test_hashset_memory_iterator"},
        {test_hashset_memory_set_operations, "test_hashset_memory_set_operations"},
        {test_hashset_memory_no_leaks, "test_hashset_memory_no_leaks"},
    };

    printf("Running HashSet Memory tests...\n");

    int failed = 0;
    const int num_tests = sizeof(tests) / sizeof(tests[0]);

    for (int i = 0; i < num_tests; i++)
    {
        printf("Running %s... ", tests[i].name);
        fflush(stdout);

        const int result = tests[i].func();
        if (result == TEST_SUCCESS)
        {
            printf("PASSED\n");
        }
        else
        {
            printf("FAILED\n");
            failed++;
        }
    }

    if (failed == 0)
    {
        printf("All HashSet Memory tests passed!\n");
        return 0;
    }
    printf("%d HashSet Memory tests failed!\n", failed);
    return 1;
}