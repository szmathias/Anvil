#include <stdlib.h>
#include <time.h>

#include <anvil/testing.h>
#include "TestHelpers.h"
#include "common/allocator.h"
#include "containers/hashset.h"

//==============================================================================
// Constants
//==============================================================================

#define LARGE_SET_SIZE 10000
#define MEDIUM_SET_SIZE 1000
#define SMALL_SET_SIZE 100

//==============================================================================
// Static Helpers
//==============================================================================

static ANVHashSet* create_string_set(ANVAllocator* alloc, char** elements, const size_t count)
{
    ANVHashSet* set = anv_hashset_create(alloc, anv_hash_string, anv_key_equals_string, 0);
    if (!set)
        return NULL;

    for (size_t i = 0; i < count; i++)
    {
        if (anv_hashset_add(set, elements[i]) != 0)
        {
            anv_hashset_destroy(set, false);
            return NULL;
        }
    }
    return set;
}

// Global counter for testing
static int visit_count = 0;

// Action function that counts visits
static void count_action(void* key)
{
    (void)key; // Unused
    visit_count++;
}

//==============================================================================
// CRUD Tests
//==============================================================================

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

int test_hashset_add_contains(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    char* key1 = "apple";
    char* key2 = "banana";
    char* key3 = "cherry";

    // Initially empty
    ASSERT(!anv_hashset_contains(set, key1));
    ASSERT(!anv_hashset_contains(set, key2));
    ASSERT(!anv_hashset_contains(set, key3));

    // Add elements
    ASSERT_EQ(anv_hashset_add(set, key1), 0);
    ASSERT_EQ(anv_hashset_size(set), 1);
    ASSERT(!anv_hashset_is_empty(set));
    ASSERT(anv_hashset_contains(set, key1));

    ASSERT_EQ(anv_hashset_add(set, key2), 0);
    ASSERT_EQ(anv_hashset_size(set), 2);
    ASSERT(anv_hashset_contains(set, key2));

    ASSERT_EQ(anv_hashset_add(set, key3), 0);
    ASSERT_EQ(anv_hashset_size(set), 3);
    ASSERT(anv_hashset_contains(set, key3));

    // All should still be present
    ASSERT(anv_hashset_contains(set, key1));
    ASSERT(anv_hashset_contains(set, key2));
    ASSERT(anv_hashset_contains(set, key3));

    anv_hashset_destroy(set, false);
    return TEST_SUCCESS;
}

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

    char* key = "test_key";
    bool was_added;

    // First addition should be new
    ASSERT_EQ(anv_hashset_add_check(set, key, &was_added), 0);
    ASSERT(was_added);
    ASSERT_EQ(anv_hashset_size(set), 1);

    // Second addition should not be new
    ASSERT_EQ(anv_hashset_add_check(set, key, &was_added), 0);
    ASSERT(!was_added);
    ASSERT_EQ(anv_hashset_size(set), 1);

    anv_hashset_destroy(set, false);
    return TEST_SUCCESS;
}

int test_hashset_remove(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    char* key1 = "first";
    char* key2 = "second";
    char* key3 = "third";

    // Add elements
    anv_hashset_add(set, key1);
    anv_hashset_add(set, key2);
    anv_hashset_add(set, key3);
    ASSERT_EQ(anv_hashset_size(set), 3);

    // Remove middle element
    ASSERT_EQ(anv_hashset_remove(set, key2, false), 0);
    ASSERT_EQ(anv_hashset_size(set), 2);
    ASSERT(!anv_hashset_contains(set, key2));
    ASSERT(anv_hashset_contains(set, key1));
    ASSERT(anv_hashset_contains(set, key3));

    // Remove non-existent element
    ASSERT_EQ(anv_hashset_remove(set, "nonexistent", false), -1);
    ASSERT_EQ(anv_hashset_size(set), 2);

    // Remove remaining elements
    ASSERT_EQ(anv_hashset_remove(set, key1, false), 0);
    ASSERT_EQ(anv_hashset_remove(set, key3, false), 0);
    ASSERT_EQ(anv_hashset_size(set), 0);
    ASSERT(anv_hashset_is_empty(set));

    anv_hashset_destroy(set, false);
    return TEST_SUCCESS;
}

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

//==============================================================================
// Algorithm Tests
//==============================================================================

int test_hashset_union(void)
{
    ANVAllocator alloc = create_int_allocator();

    char* set1_elements[] = {"a", "b", "c"};
    char* set2_elements[] = {"c", "d", "e"};

    ANVHashSet* set1 = create_string_set(&alloc, set1_elements, 3);
    ANVHashSet* set2 = create_string_set(&alloc, set2_elements, 3);

    ANVHashSet* union_set = anv_hashset_union(set1, set2);
    ASSERT_NOT_NULL(union_set);
    ASSERT_EQ(anv_hashset_size(union_set), 5); // a, b, c, d, e

    // Check all elements are present
    ASSERT(anv_hashset_contains(union_set, "a"));
    ASSERT(anv_hashset_contains(union_set, "b"));
    ASSERT(anv_hashset_contains(union_set, "c"));
    ASSERT(anv_hashset_contains(union_set, "d"));
    ASSERT(anv_hashset_contains(union_set, "e"));

    anv_hashset_destroy(set1, false);
    anv_hashset_destroy(set2, false);
    anv_hashset_destroy(union_set, false);
    return TEST_SUCCESS;
}

int test_hashset_intersection(void)
{
    ANVAllocator alloc = create_int_allocator();

    char* set1_elements[] = {"a", "b", "c", "d"};
    char* set2_elements[] = {"c", "d", "e", "f"};

    ANVHashSet* set1 = create_string_set(&alloc, set1_elements, 4);
    ANVHashSet* set2 = create_string_set(&alloc, set2_elements, 4);

    ANVHashSet* intersection_set = anv_hashset_intersection(set1, set2);
    ASSERT_NOT_NULL(intersection_set);
    ASSERT_EQ(anv_hashset_size(intersection_set), 2); // c, d

    // Check intersection elements
    ASSERT(anv_hashset_contains(intersection_set, "c"));
    ASSERT(anv_hashset_contains(intersection_set, "d"));
    ASSERT(!anv_hashset_contains(intersection_set, "a"));
    ASSERT(!anv_hashset_contains(intersection_set, "b"));
    ASSERT(!anv_hashset_contains(intersection_set, "e"));
    ASSERT(!anv_hashset_contains(intersection_set, "f"));

    anv_hashset_destroy(set1, false);
    anv_hashset_destroy(set2, false);
    anv_hashset_destroy(intersection_set, false);
    return TEST_SUCCESS;
}

int test_hashset_difference(void)
{
    ANVAllocator alloc = create_int_allocator();

    char* set1_elements[] = {"a", "b", "c", "d"};
    char* set2_elements[] = {"c", "d", "e", "f"};

    ANVHashSet* set1 = create_string_set(&alloc, set1_elements, 4);
    ANVHashSet* set2 = create_string_set(&alloc, set2_elements, 4);

    ANVHashSet* difference_set = anv_hashset_difference(set1, set2);
    ASSERT_NOT_NULL(difference_set);
    ASSERT_EQ(anv_hashset_size(difference_set), 2); // a, b

    // Check difference elements (in set1 but not set2)
    ASSERT(anv_hashset_contains(difference_set, "a"));
    ASSERT(anv_hashset_contains(difference_set, "b"));
    ASSERT(!anv_hashset_contains(difference_set, "c"));
    ASSERT(!anv_hashset_contains(difference_set, "d"));
    ASSERT(!anv_hashset_contains(difference_set, "e"));
    ASSERT(!anv_hashset_contains(difference_set, "f"));

    anv_hashset_destroy(set1, false);
    anv_hashset_destroy(set2, false);
    anv_hashset_destroy(difference_set, false);
    return TEST_SUCCESS;
}

int test_hashset_is_subset(void)
{
    ANVAllocator alloc = create_int_allocator();

    char* superset_elements[] = {"a", "b", "c", "d", "e"};
    char* subset_elements[] = {"b", "d"};
    char* non_subset_elements[] = {"b", "f"};

    ANVHashSet* superset = create_string_set(&alloc, superset_elements, 5);
    ANVHashSet* subset = create_string_set(&alloc, subset_elements, 2);
    ANVHashSet* non_subset = create_string_set(&alloc, non_subset_elements, 2);
    ANVHashSet* empty_set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    // Test valid subset
    ASSERT(anv_hashset_is_subset(subset, superset));

    // Test non-subset
    ASSERT(!anv_hashset_is_subset(non_subset, superset));

    // Test empty set is subset of any set
    ASSERT(anv_hashset_is_subset(empty_set, superset));
    ASSERT(anv_hashset_is_subset(empty_set, subset));

    // Test set is subset of itself
    ASSERT(anv_hashset_is_subset(superset, superset));

    anv_hashset_destroy(superset, false);
    anv_hashset_destroy(subset, false);
    anv_hashset_destroy(non_subset, false);
    anv_hashset_destroy(empty_set, false);
    return TEST_SUCCESS;
}

int test_hashset_empty_operations(void)
{
    ANVAllocator alloc = create_int_allocator();

    ANVHashSet* empty1 = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);
    ANVHashSet* empty2 = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    char* set_elements[] = {"a", "b", "c"};
    ANVHashSet* non_empty = create_string_set(&alloc, set_elements, 3);

    // Union with empty set
    ANVHashSet* union_result = anv_hashset_union(empty1, non_empty);
    ASSERT_NOT_NULL(union_result);
    ASSERT_EQ(anv_hashset_size(union_result), 3);

    // Intersection with empty set
    ANVHashSet* intersection_result = anv_hashset_intersection(empty1, non_empty);
    ASSERT_NOT_NULL(intersection_result);
    ASSERT_EQ(anv_hashset_size(intersection_result), 0);

    // Difference with empty set
    ANVHashSet* difference_result = anv_hashset_difference(non_empty, empty1);
    ASSERT_NOT_NULL(difference_result);
    ASSERT_EQ(anv_hashset_size(difference_result), 3);

    // Union of two empty sets
    ANVHashSet* empty_union = anv_hashset_union(empty1, empty2);
    ASSERT_NOT_NULL(empty_union);
    ASSERT_EQ(anv_hashset_size(empty_union), 0);

    anv_hashset_destroy(empty1, false);
    anv_hashset_destroy(empty2, false);
    anv_hashset_destroy(non_empty, false);
    anv_hashset_destroy(union_result, false);
    anv_hashset_destroy(intersection_result, false);
    anv_hashset_destroy(difference_result, false);
    anv_hashset_destroy(empty_union, false);
    return TEST_SUCCESS;
}

int test_hashset_operations_null_params(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    // Test NULL parameters
    ASSERT_NULL(anv_hashset_union(NULL, set));
    ASSERT_NULL(anv_hashset_union(set, NULL));
    ASSERT_NULL(anv_hashset_union(NULL, NULL));

    ASSERT_NULL(anv_hashset_intersection(NULL, set));
    ASSERT_NULL(anv_hashset_intersection(set, NULL));
    ASSERT_NULL(anv_hashset_intersection(NULL, NULL));

    ASSERT_NULL(anv_hashset_difference(NULL, set));
    // Note: anv_hashset_difference(set, NULL) should return a copy of set
    // since difference with empty set is the original set
    ANVHashSet* diff_result = anv_hashset_difference(set, NULL);
    ASSERT_NOT_NULL(diff_result);
    anv_hashset_destroy(diff_result, false);

    ASSERT(!anv_hashset_is_subset(NULL, set));
    ASSERT(!anv_hashset_is_subset(set, NULL));
    ASSERT(!anv_hashset_is_subset(NULL, NULL));

    anv_hashset_destroy(set, false);
    return TEST_SUCCESS;
}

int test_hashset_identical_operations(void)
{
    ANVAllocator alloc = create_int_allocator();

    char* elements[] = {"a", "b", "c"};
    ANVHashSet* set1 = create_string_set(&alloc, elements, 3);
    ANVHashSet* set2 = create_string_set(&alloc, elements, 3);

    // Union of identical sets
    ANVHashSet* union_result = anv_hashset_union(set1, set2);
    ASSERT_NOT_NULL(union_result);
    ASSERT_EQ(anv_hashset_size(union_result), 3);

    // Intersection of identical sets
    ANVHashSet* intersection_result = anv_hashset_intersection(set1, set2);
    ASSERT_NOT_NULL(intersection_result);
    ASSERT_EQ(anv_hashset_size(intersection_result), 3);

    // Difference of identical sets
    ANVHashSet* difference_result = anv_hashset_difference(set1, set2);
    ASSERT_NOT_NULL(difference_result);
    ASSERT_EQ(anv_hashset_size(difference_result), 0);

    // Subset check with identical sets
    ASSERT(anv_hashset_is_subset(set1, set2));
    ASSERT(anv_hashset_is_subset(set2, set1));

    anv_hashset_destroy(set1, false);
    anv_hashset_destroy(set2, false);
    anv_hashset_destroy(union_result, false);
    anv_hashset_destroy(intersection_result, false);
    anv_hashset_destroy(difference_result, false);
    return TEST_SUCCESS;
}

//==============================================================================
// Iterator Tests
//==============================================================================

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

static int test_hashset_iterator_invalid(void)
{
    const ANVIterator iter = anv_hashset_iterator(NULL);
    ASSERT(!iter.is_valid(&iter));
    return TEST_SUCCESS;
}

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

//==============================================================================
// Memory Tests
//==============================================================================

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

//==============================================================================
// Properties Tests
//==============================================================================

int test_hashset_load_factor(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 4);

    // Empty set should have 0.0 load factor
    ASSERT_EQ(anv_hashset_load_factor(set), 0.0);

    // Add elements and check load factor increases
    ASSERT_EQ(anv_hashset_add(set, "key1"), 0);
    const double lf1 = anv_hashset_load_factor(set);
    ASSERT(lf1 > 0.0);

    ASSERT_EQ(anv_hashset_add(set, "key2"), 0);
    const double lf2 = anv_hashset_load_factor(set);
    ASSERT(lf2 > lf1);

    // Remove element and check load factor decreases
    ASSERT_EQ(anv_hashset_remove(set, "key1", false), 0);
    const double lf3 = anv_hashset_load_factor(set);
    ASSERT(lf3 < lf2);
    ASSERT(lf3 > 0.0);

    anv_hashset_destroy(set, false);
    return TEST_SUCCESS;
}

int test_hashset_size_consistency(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    // Initially empty
    ASSERT_EQ(anv_hashset_size(set), 0);
    ASSERT(anv_hashset_is_empty(set));

    // Add elements
    char* keys[] = {"apple", "banana", "cherry", "date"};
    for (int i = 0; i < 4; i++)
    {
        ASSERT_EQ(anv_hashset_add(set, keys[i]), 0);
        ASSERT_EQ(anv_hashset_size(set), (size_t)i + 1);
        ASSERT(!anv_hashset_is_empty(set));
    }

    // Add duplicate - size should not change
    ASSERT_EQ(anv_hashset_add(set, "apple"), 0);
    ASSERT_EQ(anv_hashset_size(set), 4);

    // Remove elements
    for (int i = 0; i < 4; i++)
    {
        ASSERT_EQ(anv_hashset_remove(set, keys[i], false), 0);
        ASSERT_EQ(anv_hashset_size(set), 4 - (size_t)i - 1);
    }

    ASSERT(anv_hashset_is_empty(set));

    anv_hashset_destroy(set, false);
    return TEST_SUCCESS;
}

int test_hashset_uniqueness(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    char* key = "unique_key";

    // Add same key multiple times
    for (int i = 0; i < 10; i++)
    {
        ASSERT_EQ(anv_hashset_add(set, key), 0);
        ASSERT_EQ(anv_hashset_size(set), 1);
    }

    // Should still contain only one element
    ASSERT(anv_hashset_contains(set, key));
    ASSERT_EQ(anv_hashset_size(set), 1);

    // Get all elements - should return only one
    void** elements;
    size_t count;
    ASSERT_EQ(anv_hashset_get_elements(set, &elements, &count), 0);
    ASSERT_EQ(count, 1);
    ASSERT_EQ_STR((char*)elements[0], key);

    free(elements);
    anv_hashset_destroy(set, false);
    return TEST_SUCCESS;
}

int test_hashset_for_each(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    for (int i = 0; i < 3; i++)
    {
        char* keys[] = {"a", "b", "c"};
        ASSERT_EQ(anv_hashset_add(set, keys[i]), 0);
    }

    // Reset counter
    visit_count = 0;

    anv_hashset_for_each(set, count_action);
    ASSERT_EQ(visit_count, 3);

    anv_hashset_destroy(set, false);
    return TEST_SUCCESS;
}

int test_hashset_get_elements_completeness(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    char* original_keys[] = {"first", "second", "third", "fourth"};
    for (int i = 0; i < 4; i++)
    {
        ASSERT_EQ(anv_hashset_add(set, original_keys[i]), 0);
    }

    void** retrieved_keys;
    size_t count;
    ASSERT_EQ(anv_hashset_get_elements(set, &retrieved_keys, &count), 0);
    ASSERT_EQ(count, 4);

    // Verify all original keys are present in retrieved keys
    for (int i = 0; i < 4; i++)
    {
        bool found = false;
        for (size_t j = 0; j < count; j++)
        {
            if (strcmp(original_keys[i], (char*)retrieved_keys[j]) == 0)
            {
                found = true;
                break;
            }
        }
        ASSERT(found);
    }

    free(retrieved_keys);
    anv_hashset_destroy(set, false);
    return TEST_SUCCESS;
}

int test_hashset_copy_properties(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashSet* original = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    // Add elements to original
    char* keys[] = {"alpha", "beta", "gamma"};
    for (int i = 0; i < 3; i++)
    {
        ASSERT_EQ(anv_hashset_add(original, keys[i]), 0);
    }

    // Create copy
    ANVHashSet* copy = anv_hashset_copy(original);
    ASSERT_NOT_NULL(copy);

    // Verify copy has same properties
    ASSERT_EQ(anv_hashset_size(copy), anv_hashset_size(original));
    ASSERT_EQ(anv_hashset_is_empty(copy), anv_hashset_is_empty(original));

    // Verify all elements are present in copy
    for (int i = 0; i < 3; i++)
    {
        ASSERT(anv_hashset_contains(copy, keys[i]));
    }

    // Modify original - copy should be unaffected
    ASSERT_EQ(anv_hashset_add(original, "delta"), 0);
    ASSERT_EQ(anv_hashset_size(original), 4);
    ASSERT_EQ(anv_hashset_size(copy), 3);
    ASSERT(!anv_hashset_contains(copy, "delta"));

    anv_hashset_destroy(original, false);
    anv_hashset_destroy(copy, false);
    return TEST_SUCCESS;
}

int test_hashset_operation_properties(void)
{
    ANVAllocator alloc = create_int_allocator();

    ANVHashSet* set1 = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);
    ANVHashSet* set2 = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    // Set1: {a, b, c}
    ASSERT_EQ(anv_hashset_add(set1, "a"), 0);
    ASSERT_EQ(anv_hashset_add(set1, "b"), 0);
    ASSERT_EQ(anv_hashset_add(set1, "c"), 0);

    // Set2: {b, c, d}
    ASSERT_EQ(anv_hashset_add(set2, "b"), 0);
    ASSERT_EQ(anv_hashset_add(set2, "c"), 0);
    ASSERT_EQ(anv_hashset_add(set2, "d"), 0);

    // Union should have size 4: {a, b, c, d}
    ANVHashSet* union_set = anv_hashset_union(set1, set2);
    ASSERT_NOT_NULL(union_set);
    ASSERT_EQ(anv_hashset_size(union_set), 4);

    // Intersection should have size 2: {b, c}
    ANVHashSet* intersection_set = anv_hashset_intersection(set1, set2);
    ASSERT_NOT_NULL(intersection_set);
    ASSERT_EQ(anv_hashset_size(intersection_set), 2);

    // Difference should have size 1: {a}
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

int test_hashset_iterator_consistency(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_string, anv_key_equals_string, 0);

    for (int i = 0; i < 5; i++)
    {
        char* keys[] = {"one", "two", "three", "four", "five"};
        ASSERT_EQ(anv_hashset_add(set, keys[i]), 0);
    }

    ANVIterator it = anv_hashset_iterator(set);
    int iter_count = 0;
    while (it.has_next(&it))
    {
        const void* key = it.get(&it);
        it.next(&it);

        ASSERT_NOT_NULL(key);

        // Verify the key exists in the set
        ASSERT(anv_hashset_contains(set, key));
        iter_count++;
    }

    ASSERT_EQ((size_t)iter_count, anv_hashset_size(set));

    it.destroy(&it);
    anv_hashset_destroy(set, false);
    return TEST_SUCCESS;
}

//==============================================================================
// Performance Tests
//==============================================================================

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
    ASSERT_LT(time_taken, 5.0);
    ASSERT_EQ(anv_hashset_size(set), LARGE_SET_SIZE);

    anv_hashset_destroy(set, true);
    return TEST_SUCCESS;
}

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
    ASSERT_LT(time_taken, 5.0);
    ASSERT_EQ(found_count, MEDIUM_SET_SIZE * 10);

    anv_hashset_destroy(set, true);
    free(keys);
    return TEST_SUCCESS;
}

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
    ASSERT_LT(time_taken, 5.0);
    ASSERT_EQ(anv_hashset_size(set), 0);

    free(keys);
    anv_hashset_destroy(set, false);
    return TEST_SUCCESS;
}

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
    ASSERT_LT(time_taken, 5.0);

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
    ASSERT_LT(time_taken, 5.0);

    anv_hashset_destroy(set, true);
    return TEST_SUCCESS;
}

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

//==============================================================================
// Stress & Collision Tests
//==============================================================================

// Custom hash that always returns the same bucket — forces worst-case chaining
static size_t collision_hash_set(const void* key)
{
    (void)key;
    return 7; // Every key hashes to the same bucket
}

int test_hashset_collision_stress(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVHashSet* set = anv_hashset_create(&alloc, collision_hash_set, anv_key_equals_int, 4);
    ASSERT_NOT_NULL(set);

    const int N = 200;
    int* keys[200];

    // Insert N items that all collide
    for (int i = 0; i < N; i++)
    {
        keys[i] = malloc(sizeof(int));
        *keys[i] = i;
        ASSERT_EQ(anv_hashset_add(set, keys[i]), 0);
    }
    ASSERT_EQ(anv_hashset_size(set), (size_t)N);

    // All items should be found despite collisions
    for (int i = 0; i < N; i++)
    {
        ASSERT(anv_hashset_contains(set, keys[i]));
    }

    // Remove half
    for (int i = 0; i < N / 2; i++)
    {
        ASSERT_EQ(anv_hashset_remove(set, keys[i], true), 0);
    }
    ASSERT_EQ(anv_hashset_size(set), (size_t)(N / 2));

    // Remaining half still present
    for (int i = N / 2; i < N; i++)
    {
        ASSERT(anv_hashset_contains(set, keys[i]));
    }

    // Removed half should not be found
    for (int i = 0; i < N / 2; i++)
    {
        ASSERT(!anv_hashset_contains(set, &i));
    }

    anv_hashset_destroy(set, true);
    return TEST_SUCCESS;
}

int test_hashset_high_load_factor_stress(void)
{
    ANVAllocator alloc = create_int_allocator();
    // Very small initial capacity to force many resizes
    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_int, anv_key_equals_int, 2);
    ASSERT_NOT_NULL(set);

    const int N = 10000;

    for (int i = 0; i < N; i++)
    {
        int* key = malloc(sizeof(int));
        *key = i;
        ASSERT_EQ(anv_hashset_add(set, key), 0);
    }
    ASSERT_EQ(anv_hashset_size(set), (size_t)N);

    // Verify all items after many resizes
    for (int i = 0; i < N; i++)
    {
        ASSERT(anv_hashset_contains(set, &i));
    }

    // Load factor should be reasonable
    ASSERT_LT(anv_hashset_load_factor(set), 1.0);

    anv_hashset_destroy(set, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Fuzz Tests
//==============================================================================

int test_hashset_fuzz(void)
{
    srand((unsigned int)42);
    ANVAllocator alloc = create_int_allocator();
    ANVHashSet* set = anv_hashset_create(&alloc, anv_hash_int, anv_key_equals_int, 16);
    ASSERT_NOT_NULL(set);

    size_t expected_size = 0;

    for (int i = 0; i < 50000; i++)
    {
        const unsigned op = rand() % 3;

        switch (op)
        {
            case 0: // add
                {
                    MAKE_INT(val, rand() % 500);
                    const int already = anv_hashset_contains(set, val);
                    const int rc = anv_hashset_add(set, val);
                    if (rc == 0 && !already)
                        expected_size++;
                    else
                        free(val); // duplicate or failure — free the unused value
                    break;
                }
            case 1: // remove
                {
                    int key = rand() % 500;
                    const int had = anv_hashset_contains(set, &key);
                    const int rc = anv_hashset_remove(set, &key, true);
                    if (rc == 0 && had)
                        expected_size--;
                    break;
                }
            case 2: // contains
                {
                    int key = rand() % 500;
                    anv_hashset_contains(set, &key); // exercise, ignore result
                    break;
                }
            default:
                break;
        }

        ASSERT_EQ(anv_hashset_size(set), expected_size);
    }

    anv_hashset_destroy(set, true);
    return TEST_SUCCESS;
}

//==============================================================================
// Main
//==============================================================================

int main(void)
{
    const ANVTestCase tests[] = {
        // CRUD tests
        TEST_REGISTER(test_hashset_create_destroy),
        TEST_REGISTER(test_hashset_add_contains),
        TEST_REGISTER(test_hashset_duplicate_add),
        TEST_REGISTER(test_hashset_add_check),
        TEST_REGISTER(test_hashset_remove),
        TEST_REGISTER(test_hashset_remove_get),
        TEST_REGISTER(test_hashset_clear),
        TEST_REGISTER(test_hashset_null_params),
        TEST_REGISTER(test_hashset_invalid_creation),

        // Algorithm tests
        TEST_REGISTER(test_hashset_union),
        TEST_REGISTER(test_hashset_intersection),
        TEST_REGISTER(test_hashset_difference),
        TEST_REGISTER(test_hashset_is_subset),
        TEST_REGISTER(test_hashset_empty_operations),
        TEST_REGISTER(test_hashset_operations_null_params),
        TEST_REGISTER(test_hashset_identical_operations),

        // Iterator tests
        TEST_REGISTER(test_hashset_iterator_basic),
        TEST_REGISTER(test_hashset_iterator_empty),
        TEST_REGISTER(test_hashset_iterator_with_modifications),
        TEST_REGISTER(test_hashset_iterator_multiple),
        TEST_REGISTER(test_hashset_iterator_get),
        TEST_REGISTER(test_hashset_iterator_backward),
        TEST_REGISTER(test_hashset_from_iterator),
        TEST_REGISTER(test_hashset_iterator_invalid),
        TEST_REGISTER(test_hashset_copy_isolation),
        TEST_REGISTER(test_hashset_anv_copy_function_required),
        TEST_REGISTER(test_hashset_from_iterator_no_copy),
        TEST_REGISTER(test_hashset_iterator_exhaustion_after_creation),
        TEST_REGISTER(test_hashset_iterator_next_return_values),
        TEST_REGISTER(test_hashset_iterator_mixed_operations),
        TEST_REGISTER(test_hashset_iterator_reset),
        TEST_REGISTER(test_hashset_iterator_single_element),
        TEST_REGISTER(test_hashset_from_iterator_null_params),

        // Memory tests
        TEST_REGISTER(test_hashset_memory_basic),
        TEST_REGISTER(test_hashset_memory_with_key_freeing),
        TEST_REGISTER(test_hashset_memory_copy),
        TEST_REGISTER(test_hashset_memory_deep_copy),
        TEST_REGISTER(test_hashset_memory_get_elements),
        TEST_REGISTER(test_hashset_memory_iterator),
        TEST_REGISTER(test_hashset_memory_set_operations),
        TEST_REGISTER(test_hashset_memory_no_leaks),

        // Properties tests
        TEST_REGISTER(test_hashset_load_factor),
        TEST_REGISTER(test_hashset_size_consistency),
        TEST_REGISTER(test_hashset_uniqueness),
        TEST_REGISTER(test_hashset_for_each),
        TEST_REGISTER(test_hashset_get_elements_completeness),
        TEST_REGISTER(test_hashset_copy_properties),
        TEST_REGISTER(test_hashset_operation_properties),
        TEST_REGISTER(test_hashset_iterator_consistency),

        // Performance tests
        TEST_REGISTER(test_hashset_add_performance),
        TEST_REGISTER(test_hashset_contains_performance),
        TEST_REGISTER(test_hashset_remove_performance),
        TEST_REGISTER(test_hashset_set_operations_performance),
        TEST_REGISTER(test_hashset_iterator_performance),
        TEST_REGISTER(test_hashset_copy_performance),
        TEST_REGISTER(test_hashset_load_factor_performance),

        // Stress & Collision
        TEST_REGISTER(test_hashset_collision_stress),
        TEST_REGISTER(test_hashset_high_load_factor_stress),

        // Fuzz Tests
        TEST_REGISTER(test_hashset_fuzz),
    };

    return anv_run_tests("HashSet", tests, sizeof(tests) / sizeof(tests[0]));
}