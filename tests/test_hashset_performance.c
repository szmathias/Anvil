//
// HashSet Performance test
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "containers/hashset.h"
#include "TestAssert.h"
#include "TestHelpers.h"

#define LARGE_SET_SIZE 10000
#define MEDIUM_SET_SIZE 1000
#define SMALL_SET_SIZE 100

// Test performance of add operations
int test_hashset_add_performance(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    const clock_t start = clock();

    // Add a large number of elements
    for (int i = 0; i < LARGE_SET_SIZE; i++)
    {
        char* key = malloc(32);
        sprintf(key, "key_%d", i);
        ASSERT_EQ(anv_hashset_add(set, key), 0);
    }

    const clock_t end = clock();
    const double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;

    printf("Added %d elements in %f seconds\n", LARGE_SET_SIZE, time_taken);
    ASSERT_EQ(anv_hashset_size(set), LARGE_SET_SIZE);

    anv_hashset_destroy(set, true);
    return TEST_SUCCESS;
}

// Test performance of contains operations
int test_hashset_contains_performance(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    // Add elements
    char** keys = malloc(MEDIUM_SET_SIZE * sizeof(char*));
    for (int i = 0; i < MEDIUM_SET_SIZE; i++)
    {
        keys[i] = malloc(32);
        sprintf(keys[i], "key_%d", i);
        ASSERT_EQ(anv_hashset_add(set, keys[i]), 0);
    }

    const clock_t start = clock();

    // Search for all elements multiple times
    int found_count = 0;
    for (int iter = 0; iter < 10; iter++)
    {
        for (int i = 0; i < MEDIUM_SET_SIZE; i++)
        {
            if (anv_hashset_contains(set, keys[i]))
            {
                found_count++;
            }
        }
    }

    const clock_t end = clock();
    const double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;

    printf("Performed %d lookups in %f seconds\n", found_count, time_taken);
    ASSERT_EQ(found_count, MEDIUM_SET_SIZE * 10);

    anv_hashset_destroy(set, true);
    free(keys);
    return TEST_SUCCESS;
}

// Test performance of remove operations
int test_hashset_remove_performance(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    // Add elements
    char** keys = malloc(MEDIUM_SET_SIZE * sizeof(char*));
    for (int i = 0; i < MEDIUM_SET_SIZE; i++)
    {
        keys[i] = malloc(32);
        sprintf(keys[i], "key_%d", i);
        ASSERT_EQ(anv_hashset_add(set, keys[i]), 0);
    }

    const clock_t start = clock();

    // Remove all elements
    for (int i = 0; i < MEDIUM_SET_SIZE; i++)
    {
        ASSERT_EQ(anv_hashset_remove(set, keys[i], true), 0);
    }

    const clock_t end = clock();
    const double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;

    printf("Removed %d elements in %f seconds\n", MEDIUM_SET_SIZE, time_taken);
    ASSERT_EQ(anv_hashset_size(set), 0);

    free(keys);
    anv_hashset_destroy(set, false);
    return TEST_SUCCESS;
}

// Test performance of set operations
int test_hashset_set_operations_performance(void)
{
    ANVAllocator alloc = create_int_allocator();

    ANVHashSet* set1 = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);
    ANVHashSet* set2 = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    // Add elements to both sets with some overlap
    for (int i = 0; i < SMALL_SET_SIZE; i++)
    {
        char* key1 = malloc(32);
        char* key2 = malloc(32);
        sprintf(key1, "set1_key_%d", i);
        sprintf(key2, "set2_key_%d", i);

        ASSERT_EQ(anv_hashset_add(set1, key1), 0);
        ASSERT_EQ(anv_hashset_add(set2, key2), 0);

        // Add some overlapping elements
        if (i % 3 == 0)
        {
            char* common_key1 = malloc(32);
            char* common_key2 = malloc(32);
            sprintf(common_key1, "common_key_%d", i);
            sprintf(common_key2, "common_key_%d", i);
            ASSERT_EQ(anv_hashset_add(set1, common_key1), 0);
            ASSERT_EQ(anv_hashset_add(set2, common_key2), 0);
        }
    }

    const clock_t start = clock();

    // Perform set operations
    ANVHashSet* union_set = anv_hashset_union(set1, set2);
    ANVHashSet* intersection_set = anv_hashset_intersection(set1, set2);
    ANVHashSet* difference_set = anv_hashset_difference(set1, set2);

    const clock_t end = clock();
    const double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;

    printf("Performed set operations in %f seconds\n", time_taken);

    ASSERT_NOT_NULL(union_set);
    ASSERT_NOT_NULL(intersection_set);
    ASSERT_NOT_NULL(difference_set);

    anv_hashset_destroy(set1, true);
    anv_hashset_destroy(set2, true);
    anv_hashset_destroy(union_set, false);
    anv_hashset_destroy(intersection_set, false);
    anv_hashset_destroy(difference_set, false);
    return TEST_SUCCESS;
}

// Test performance of iterator operations
int test_hashset_iterator_performance(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    // Add elements
    for (int i = 0; i < MEDIUM_SET_SIZE; i++)
    {
        char* key = malloc(32);
        sprintf(key, "key_%d", i);
        ASSERT_EQ(anv_hashset_add(set, key), 0);
    }

    const clock_t start = clock();

    // Iterate through the set multiple times
    for (int iter = 0; iter < 10; iter++)
    {
        ANVIterator it = anv_hashset_iterator(set);
        int count = 0;
        while (it.has_next(&it))
        {
            const void* key = it.get(&it);
            it.next(&it);
            ASSERT_NOT_NULL(key);
            count++;
        }
        ASSERT_EQ(count, MEDIUM_SET_SIZE);
        it.destroy(&it);
    }

    const clock_t end = clock();
    const double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;

    printf("Performed 10 full iterations in %f seconds\n", time_taken);

    anv_hashset_destroy(set, true);
    return TEST_SUCCESS;
}

// Test performance of copy operations
int test_hashset_copy_performance(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashSet* original = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    // Add elements
    for (int i = 0; i < MEDIUM_SET_SIZE; i++)
    {
        char* key = malloc(32);
        sprintf(key, "key_%d", i);
        ASSERT_EQ(anv_hashset_add(original, key), 0);
    }

    clock_t start = clock();

    // Perform shallow copy
    ANVHashSet* shallow_copy = anv_hashset_copy(original);

    clock_t end = clock();
    const double shallow_time = ((double)(end - start)) / CLOCKS_PER_SEC;

    start = clock();

    // Perform deep copy with proper string copy function
    ANVHashSet* deep_copy = anv_hashset_copy_deep(original, string_copy);

    end = clock();
    const double deep_time = ((double)(end - start)) / CLOCKS_PER_SEC;

    printf("Shallow copy: %f seconds, Deep copy: %f seconds\n", shallow_time, deep_time);

    ASSERT_NOT_NULL(shallow_copy);
    ASSERT_NOT_NULL(deep_copy);
    ASSERT_EQ(anv_hashset_size(shallow_copy), MEDIUM_SET_SIZE);
    ASSERT_EQ(anv_hashset_size(deep_copy), MEDIUM_SET_SIZE);

    anv_hashset_destroy(original, true);
    anv_hashset_destroy(shallow_copy, false);
    anv_hashset_destroy(deep_copy, true);
    return TEST_SUCCESS;
}

// Test load factor impact on performance
int test_hashset_load_factor_performance(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Test with small initial capacity (high load factor)
    ANVHashSet* high_load_set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 4);

    clock_t start = clock();
    for (int i = 0; i < SMALL_SET_SIZE; i++)
    {
        char* key = malloc(32);
        sprintf(key, "high_load_key_%d", i);
        ASSERT_EQ(anv_hashset_add(high_load_set, key), 0);
    }
    clock_t end = clock();
    const double high_load_time = ((double)(end - start)) / CLOCKS_PER_SEC;

    // Test with large initial capacity (low load factor)
    ANVHashSet* low_load_set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 1024);

    start = clock();
    for (int i = 0; i < SMALL_SET_SIZE; i++)
    {
        char* key = malloc(32);
        sprintf(key, "low_load_key_%d", i);
        ASSERT_EQ(anv_hashset_add(low_load_set, key), 0);
    }
    end = clock();
    const double low_load_time = ((double)(end - start)) / CLOCKS_PER_SEC;

    printf("High load factor: %f seconds, Low load factor: %f seconds\n",
           high_load_time, low_load_time);

    printf("High load factor: %f, Low load factor: %f\n",
           anv_hashset_load_factor(high_load_set),
           anv_hashset_load_factor(low_load_set));

    anv_hashset_destroy(high_load_set, true);
    anv_hashset_destroy(low_load_set, true);
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
        {test_hashset_add_performance, "test_hashset_add_performance"},
        {test_hashset_contains_performance, "test_hashset_contains_performance"},
        {test_hashset_remove_performance, "test_hashset_remove_performance"},
        {test_hashset_set_operations_performance, "test_hashset_set_operations_performance"},
        {test_hashset_iterator_performance, "test_hashset_iterator_performance"},
        {test_hashset_copy_performance, "test_hashset_copy_performance"},
        {test_hashset_load_factor_performance, "test_hashset_load_factor_performance"},
    };

    printf("Running HashSet Performance tests...\n");

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
        printf("All HashSet Performance tests completed!\n");
        return 0;
    }
    printf("%d HashSet Performance tests failed!\n", failed);
    return 1;
}