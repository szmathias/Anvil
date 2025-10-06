//
// HashMap CRUD test - converted from HashTable CRUD test
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "containers/hashmap.h"
#include "TestAssert.h"
#include "TestHelpers.h"

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

typedef struct
{
    int (*func)(void);
    const char* name;
} TestCase;

int main(void)
{
    const TestCase tests[] = {
        {test_hashmap_create_destroy, "test_hashmap_create_destroy"},
        {test_hashmap_put_get, "test_hashmap_put_get"},
        {test_hashmap_update, "test_hashmap_update"},
        {test_hashmap_remove, "test_hashmap_remove"},
        {test_hashmap_remove_get, "test_hashmap_remove_get"},
        {test_hashmap_contains, "test_hashmap_contains"},
        {test_hashmap_int_keys, "test_hashmap_int_keys"},
        {test_hashmap_resize, "test_hashmap_resize"},
    };

    printf("Running HashMap CRUD tests...\n");

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
        printf("All HashMap CRUD tests passed!\n");
        return 0;
    }

    printf("%d HashMap CRUD tests failed.\n", failed);
    return 1;
}