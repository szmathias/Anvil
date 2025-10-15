//
// HashSet CRUD test
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "containers/hashset.h"
#include "TestAssert.h"
#include "TestHelpers.h"

// Test basic hash set creation and destruction
int test_hashset_create_destroy(void)
{
    ANVAllocator alloc = create_int_allocator();

    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);
    ASSERT_NOT_NULL(set);
    ASSERT_EQ(anv_hashset_size(set), 0);
    ASSERT(anv_hashset_is_empty(set));

    anv_hashset_destroy(set, false);
    return TEST_SUCCESS;
}

// Test basic add and contains operations
int test_hashset_add_contains(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    char* key1 = "hello";
    char* key2 = "world";

    // Test add operations
    ASSERT_EQ(anv_hashset_add(set, key1), 0);
    ASSERT_EQ(anv_hashset_size(set), 1);
    ASSERT(!anv_hashset_is_empty(set));

    ASSERT_EQ(anv_hashset_add(set, key2), 0);
    ASSERT_EQ(anv_hashset_size(set), 2);

    // Test contains operations
    ASSERT(anv_hashset_contains(set, key1));
    ASSERT(anv_hashset_contains(set, key2));

    // Test non-existent key
    ASSERT(!anv_hashset_contains(set, "nonexistent"));

    anv_hashset_destroy(set, false);
    return TEST_SUCCESS;
}

// Test adding duplicate keys
int test_hashset_duplicate_add(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    char* key = "test";

    // Insert initial value
    ASSERT_EQ(anv_hashset_add(set, key), 0);
    ASSERT_EQ(anv_hashset_size(set), 1);
    ASSERT(anv_hashset_contains(set, key));

    // Add duplicate - should be no-op
    ASSERT_EQ(anv_hashset_add(set, key), 0);
    ASSERT_EQ(anv_hashset_size(set), 1); // Size should remain the same
    ASSERT(anv_hashset_contains(set, key));

    anv_hashset_destroy(set, false);
    return TEST_SUCCESS;
}

// Test add_check function
int test_hashset_add_check(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    char* key = "test";
    bool was_added;

    // Add new key
    ASSERT_EQ(anv_hashset_add_check(set, key, &was_added), 0);
    ASSERT(was_added);
    ASSERT_EQ(anv_hashset_size(set), 1);

    // Add duplicate key
    ASSERT_EQ(anv_hashset_add_check(set, key, &was_added), 0);
    ASSERT(!was_added);
    ASSERT_EQ(anv_hashset_size(set), 1);

    anv_hashset_destroy(set, false);
    return TEST_SUCCESS;
}

// Test remove operations
int test_hashset_remove(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    char* key1 = "key1";
    char* key2 = "key2";

    // Add elements
    ASSERT_EQ(anv_hashset_add(set, key1), 0);
    ASSERT_EQ(anv_hashset_add(set, key2), 0);
    ASSERT_EQ(anv_hashset_size(set), 2);

    // Remove existing key
    ASSERT_EQ(anv_hashset_remove(set, key1, false), 0);
    ASSERT_EQ(anv_hashset_size(set), 1);
    ASSERT(!anv_hashset_contains(set, key1));
    ASSERT(anv_hashset_contains(set, key2));

    // Try to remove non-existent key
    ASSERT_EQ(anv_hashset_remove(set, "nonexistent", false), -1);
    ASSERT_EQ(anv_hashset_size(set), 1);

    // Remove remaining key
    ASSERT_EQ(anv_hashset_remove(set, key2, false), 0);
    ASSERT_EQ(anv_hashset_size(set), 0);
    ASSERT(anv_hashset_is_empty(set));

    anv_hashset_destroy(set, false);
    return TEST_SUCCESS;
}

// Test remove_get operations
int test_hashset_remove_get(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    char* key1 = "key1";
    char* key2 = "key2";

    // Add elements
    ASSERT_EQ(anv_hashset_add(set, key1), 0);
    ASSERT_EQ(anv_hashset_add(set, key2), 0);

    // Remove and get existing key
    void* removed = anv_hashset_remove_get(set, key1);
    ASSERT_NOT_NULL(removed);
    ASSERT_EQ_STR((char*)removed, key1);
    ASSERT_EQ(anv_hashset_size(set), 1);
    ASSERT(!anv_hashset_contains(set, key1));

    // Try to remove non-existent key
    removed = anv_hashset_remove_get(set, "nonexistent");
    ASSERT_NULL(removed);

    anv_hashset_destroy(set, false);
    return TEST_SUCCESS;
}

// Test clear operations
int test_hashset_clear(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    // Add some elements
    ASSERT_EQ(anv_hashset_add(set, "key1"), 0);
    ASSERT_EQ(anv_hashset_add(set, "key2"), 0);
    ASSERT_EQ(anv_hashset_add(set, "key3"), 0);
    ASSERT_EQ(anv_hashset_size(set), 3);

    // Clear the set
    anv_hashset_clear(set, false);
    ASSERT_EQ(anv_hashset_size(set), 0);
    ASSERT(anv_hashset_is_empty(set));

    // Verify elements are gone
    ASSERT(!anv_hashset_contains(set, "key1"));
    ASSERT(!anv_hashset_contains(set, "key2"));
    ASSERT(!anv_hashset_contains(set, "key3"));

    anv_hashset_destroy(set, false);
    return TEST_SUCCESS;
}

// Test NULL parameter handling
int test_hashset_null_params(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    // Test NULL set parameters
    ASSERT_EQ(anv_hashset_size(NULL), 0);
    ASSERT(anv_hashset_is_empty(NULL));
    ASSERT_EQ(anv_hashset_load_factor(NULL), 0.0);
    ASSERT_EQ(anv_hashset_add(NULL, "key"), -1);
    ASSERT_EQ(anv_hashset_add(set, NULL), -1);
    ASSERT(!anv_hashset_contains(NULL, "key"));
    ASSERT(!anv_hashset_contains(set, NULL));
    ASSERT_EQ(anv_hashset_remove(NULL, "key", false), -1);
    ASSERT_EQ(anv_hashset_remove(set, NULL, false), -1);
    ASSERT_NULL(anv_hashset_remove_get(NULL, "key"));
    ASSERT_NULL(anv_hashset_remove_get(set, NULL));

    bool was_added;
    ASSERT_EQ(anv_hashset_add_check(NULL, "key", &was_added), -1);
    ASSERT_EQ(anv_hashset_add_check(set, NULL, &was_added), -1);
    ASSERT_EQ(anv_hashset_add_check(set, "key", NULL), -1);

    anv_hashset_destroy(set, false);
    anv_hashset_destroy(NULL, false); // Should not crash
    return TEST_SUCCESS;
}

// Test invalid creation parameters
int test_hashset_invalid_creation(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Test NULL allocator
    ASSERT_NULL(anv_hashset_create(NULL, anv_hash_string, anv_key_equals_string, 0));

    // Test NULL hash function
    ASSERT_NULL(anv_hashset_create(&alloc, NULL, anv_key_equals_string, 0));

    // Test NULL key_equals function
    ASSERT_NULL(anv_hashset_create(&alloc, anv_hash_string, NULL, 0));

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
        {test_hashset_create_destroy, "test_hashset_create_destroy"},
        {test_hashset_add_contains, "test_hashset_add_contains"},
        {test_hashset_duplicate_add, "test_hashset_duplicate_add"},
        {test_hashset_add_check, "test_hashset_add_check"},
        {test_hashset_remove, "test_hashset_remove"},
        {test_hashset_remove_get, "test_hashset_remove_get"},
        {test_hashset_clear, "test_hashset_clear"},
        {test_hashset_null_params, "test_hashset_null_params"},
        {test_hashset_invalid_creation, "test_hashset_invalid_creation"},
    };

    printf("Running HashSet CRUD tests...\n");

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
        printf("All HashSet CRUD tests passed!\n");
        return 0;
    }
    printf("%d HashSet CRUD tests failed!\n", failed);
    return 1;
}