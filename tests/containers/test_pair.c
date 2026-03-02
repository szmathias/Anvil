//
// Created by zack on 9/15/25.
//

#include "containers/pair.h"
#include "TestAssert.h"
#include "TestHelpers.h"
#include "TestRunner.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// Helper functions (deduplicated from all source files)
// ============================================================================

static void* int_anv_copy_func(const void* data)
{
    int* copy = malloc(sizeof(int));
    if (copy)
    {
        *copy = *(const int*)data;
    }
    return copy;
}

static void* string_anv_copy_func(const void* data)
{
    const char* str = (const char*)data;
    size_t len = strlen(str) + 1;
    char* copy = malloc(len);
    if (copy)
    {
        strcpy(copy, str);
    }
    return copy;
}

// Custom string comparison function (from test_pair_comparison.c)
static int string_cmp(const void* a, const void* b)
{
    return strcmp((const char*)a, (const char*)b);
}

// Helper function that always fails for testing (from test_pair_memory.c)
static void* failing_anv_copy_func(const void* data)
{
    (void)data;
    return NULL; // Always fail
}

// ============================================================================
// CRUD Tests (from test_pair_crud.c)
// ============================================================================

int test_pair_anv_copy_functions(void)
{
    ANVAllocator alloc = create_int_allocator();

    int* first = malloc(sizeof(int));
    int* second = malloc(sizeof(int));
    *first = 42;
    *second = 84;

    ANVPair* original = anv_pair_create(&alloc, first, second);

    // Test shallow copy
    ANVPair* shallow = anv_pair_copy(original);
    ASSERT_NOT_NULL(shallow);
    ASSERT_NOT_EQ_PTR(shallow, original);
    ASSERT_EQ_PTR(shallow->first, original->first);   // Same pointers
    ASSERT_EQ_PTR(shallow->second, original->second); // Same pointers
    ASSERT_EQ_PTR(shallow->alloc.allocate, original->alloc.allocate);

    // Test deep copy with both copy functions
    ANVPair* deep = anv_pair_copy_deep(original, int_anv_copy_func, int_anv_copy_func, true);
    ASSERT_NOT_NULL(deep);
    ASSERT_NOT_EQ_PTR(deep, original);
    ASSERT_NOT_EQ_PTR(deep->first, original->first);         // Different pointers
    ASSERT_NOT_EQ_PTR(deep->second, original->second);       // Different pointers
    ASSERT_EQ(*(int*)deep->first, *(int*)original->first);   // Same values
    ASSERT_EQ(*(int*)deep->second, *(int*)original->second); // Same values

    // Test deep copy with only first copy function
    ANVPair* partial = anv_pair_copy_deep(original, int_anv_copy_func, NULL, true);
    ASSERT_NOT_NULL(partial);
    ASSERT_NOT_EQ_PTR(partial->first, original->first); // Copied
    ASSERT_EQ_PTR(partial->second, original->second);   // Referenced
    ASSERT_EQ(*(int*)partial->first, *(int*)original->first);

    // Test copy with NULL
    ASSERT_NULL(anv_pair_copy(NULL));
    ASSERT_NULL(anv_pair_copy_deep(NULL, int_anv_copy_func, int_anv_copy_func, true));

    anv_pair_destroy(original, true, true);
    anv_pair_destroy(shallow, false, false); // Don't free data (shared with original)
    anv_pair_destroy(deep, true, true);
    anv_pair_destroy(partial, true, false); // First copied, second referenced
    return TEST_SUCCESS;
}

int test_pair_mixed_type_copy(void)
{
    ANVAllocator alloc = create_int_allocator();

    int* first = malloc(sizeof(int));
    char* second = malloc(10);
    *first = 42;
    strcpy(second, "hello");

    ANVPair* original = anv_pair_create(&alloc, first, second);

    // Test deep copy with different copy functions for each element
    ANVPair* mixed_copy = anv_pair_copy_deep(original, int_anv_copy_func, string_anv_copy_func, true);
    ASSERT_NOT_NULL(mixed_copy);
    ASSERT_NOT_EQ_PTR(mixed_copy->first, original->first);
    ASSERT_NOT_EQ_PTR(mixed_copy->second, original->second);
    ASSERT_EQ(*(int*)mixed_copy->first, 42);
    ASSERT_EQ(strcmp((char*)mixed_copy->second, "hello"), 0);

    anv_pair_destroy(original, true, true);
    anv_pair_destroy(mixed_copy, true, true);
    return TEST_SUCCESS;
}

int test_pair_create_destroy(void)
{
    ANVAllocator alloc = create_int_allocator();

    int* first = malloc(sizeof(int));
    int* second = malloc(sizeof(int));
    *first = 42;
    *second = 84;

    ANVPair* pair = anv_pair_create(&alloc, first, second);
    ASSERT_NOT_NULL(pair);
    ASSERT_EQ_PTR(pair->first, first);
    ASSERT_EQ_PTR(pair->second, second);
    ASSERT_EQ_PTR(pair->alloc.allocate, alloc.allocate);

    anv_pair_destroy(pair, true, true);
    return TEST_SUCCESS;
}

int test_pair_create_with_null_elements(void)
{
    ANVAllocator alloc = create_int_allocator();

    int* first = malloc(sizeof(int));
    *first = 42;

    // Test with second element NULL
    ANVPair* pair1 = anv_pair_create(&alloc, first, NULL);
    ASSERT_NOT_NULL(pair1);
    ASSERT_NOT_NULL(pair1->first);
    ASSERT_NULL(pair1->second);
    ASSERT_EQ(*(int*)pair1->first, 42);

    // Test with first element NULL
    int* second = malloc(sizeof(int));
    *second = 84;
    ANVPair* pair2 = anv_pair_create(&alloc, NULL, second);
    ASSERT_NOT_NULL(pair2);
    ASSERT_NULL(pair2->first);
    ASSERT_NOT_NULL(pair2->second);
    ASSERT_EQ(*(int*)pair2->second, 84);

    // Test with both NULL
    ANVPair* pair3 = anv_pair_create(&alloc, NULL, NULL);
    ASSERT_NOT_NULL(pair3);
    ASSERT_NULL(pair3->first);
    ASSERT_NULL(pair3->second);

    anv_pair_destroy(pair1, true, false);
    anv_pair_destroy(pair2, false, true);
    anv_pair_destroy(pair3, false, false);
    return TEST_SUCCESS;
}

int test_pair_create_invalid_allocator(void)
{
    // Test with NULL allocator
    ANVPair* pair1 = anv_pair_create(NULL, NULL, NULL);
    ASSERT_NULL(pair1);

    // Test with incomplete allocator
    ANVAllocator incomplete_alloc = {0};
    ANVPair* pair2 = anv_pair_create(&incomplete_alloc, NULL, NULL);
    ASSERT_NULL(pair2);

    return TEST_SUCCESS;
}

int test_pair_accessors(void)
{
    ANVAllocator alloc = create_int_allocator();

    int* first = malloc(sizeof(int));
    int* second = malloc(sizeof(int));
    *first = 42;
    *second = 84;

    ANVPair* pair = anv_pair_create(&alloc, first, second);

    // Test getters
    void* get_first = anv_pair_first(pair);
    void* get_second = anv_pair_second(pair);
    ASSERT_EQ_PTR(get_first, first);
    ASSERT_EQ_PTR(get_second, second);
    ASSERT_EQ(*(int*)get_first, 42);
    ASSERT_EQ(*(int*)get_second, 84);

    // Test getters with NULL pair
    ASSERT_NULL(anv_pair_first(NULL));
    ASSERT_NULL(anv_pair_second(NULL));

    anv_pair_destroy(pair, true, true);
    return TEST_SUCCESS;
}

int test_pair_setters(void)
{
    ANVAllocator alloc = create_int_allocator();

    int* first = malloc(sizeof(int));
    int* second = malloc(sizeof(int));
    int* new_first = malloc(sizeof(int));
    int* new_second = malloc(sizeof(int));
    *first = 42;
    *second = 84;
    *new_first = 100;
    *new_second = 200;

    ANVPair* pair = anv_pair_create(&alloc, first, second);

    // Test set first without freeing old
    anv_pair_set_first(pair, new_first, false);
    ASSERT_EQ_PTR(pair->first, new_first);
    ASSERT_EQ(*(int*)pair->first, 100);

    // Test set second with freeing old
    anv_pair_set_second(pair, new_second, true);
    ASSERT_EQ_PTR(pair->second, new_second);
    ASSERT_EQ(*(int*)pair->second, 200);

    // Test setters with NULL pair
    anv_pair_set_first(NULL, NULL, false);
    anv_pair_set_second(NULL, NULL, false);

    // Clean up (first wasn't freed by set_first, so free manually)
    free(first);
    anv_pair_destroy(pair, true, true);
    return TEST_SUCCESS;
}

int test_pair_swap(void)
{
    ANVAllocator alloc = create_int_allocator();

    int* first = malloc(sizeof(int));
    int* second = malloc(sizeof(int));
    *first = 42;
    *second = 84;

    ANVPair* pair = anv_pair_create(&alloc, first, second);

    // Swap elements
    anv_pair_swap(pair);
    ASSERT_EQ_PTR(pair->first, second);
    ASSERT_EQ_PTR(pair->second, first);
    ASSERT_EQ(*(int*)pair->first, 84);
    ASSERT_EQ(*(int*)pair->second, 42);

    // Test swap with NULL pair
    anv_pair_swap(NULL);

    anv_pair_destroy(pair, true, true);
    return TEST_SUCCESS;
}

// ============================================================================
// Comparison Tests (from test_pair_comparison.c)
// ============================================================================

int test_pair_compare_equal_pairs(void)
{
    ANVAllocator alloc = create_int_allocator();

    int* first1 = malloc(sizeof(int));
    int* second1 = malloc(sizeof(int));
    int* first2 = malloc(sizeof(int));
    int* second2 = malloc(sizeof(int));
    *first1 = 42;
    *second1 = 84;
    *first2 = 42;
    *second2 = 84;

    ANVPair* pair1 = anv_pair_create(&alloc, first1, second1);
    ANVPair* pair2 = anv_pair_create(&alloc, first2, second2);

    // Test with comparison functions
    int result = anv_pair_compare(pair1, pair2, int_cmp, int_cmp);
    ASSERT_EQ(result, 0);

    // Test equality function
    ASSERT_TRUE(anv_pair_equals(pair1, pair2, int_cmp, int_cmp));

    anv_pair_destroy(pair1, true, true);
    anv_pair_destroy(pair2, true, true);
    return TEST_SUCCESS;
}

int test_pair_compare_first_different(void)
{
    ANVAllocator alloc = create_int_allocator();

    int* first1 = malloc(sizeof(int));
    int* second1 = malloc(sizeof(int));
    int* first2 = malloc(sizeof(int));
    int* second2 = malloc(sizeof(int));
    *first1 = 10;
    *second1 = 84;
    *first2 = 42;
    *second2 = 84;

    ANVPair* pair1 = anv_pair_create(&alloc, first1, second1);
    ANVPair* pair2 = anv_pair_create(&alloc, first2, second2);

    // pair1 < pair2 (first element is smaller)
    int result = anv_pair_compare(pair1, pair2, int_cmp, int_cmp);
    ASSERT_LT(result, 0);

    // Test equality function
    ASSERT_FALSE(anv_pair_equals(pair1, pair2, int_cmp, int_cmp));

    anv_pair_destroy(pair1, true, true);
    anv_pair_destroy(pair2, true, true);
    return TEST_SUCCESS;
}

int test_pair_compare_second_different(void)
{
    ANVAllocator alloc = create_int_allocator();

    int* first1 = malloc(sizeof(int));
    int* second1 = malloc(sizeof(int));
    int* first2 = malloc(sizeof(int));
    int* second2 = malloc(sizeof(int));
    *first1 = 42;
    *second1 = 10;
    *first2 = 42;
    *second2 = 84;

    ANVPair* pair1 = anv_pair_create(&alloc, first1, second1);
    ANVPair* pair2 = anv_pair_create(&alloc, first2, second2);

    // pair1 < pair2 (first elements equal, second element is smaller)
    int result = anv_pair_compare(pair1, pair2, int_cmp, int_cmp);
    ASSERT_LT(result, 0);

    // Test equality function
    ASSERT_FALSE(anv_pair_equals(pair1, pair2, int_cmp, int_cmp));

    anv_pair_destroy(pair1, true, true);
    anv_pair_destroy(pair2, true, true);
    return TEST_SUCCESS;
}

int test_pair_compare_null_pairs(void)
{
    ANVAllocator alloc = create_int_allocator();

    int* first = malloc(sizeof(int));
    int* second = malloc(sizeof(int));
    *first = 42;
    *second = 84;

    ANVPair* pair = anv_pair_create(&alloc, first, second);

    // Both NULL
    ASSERT_EQ(anv_pair_compare(NULL, NULL, int_cmp, int_cmp), 0);
    ASSERT_TRUE(anv_pair_equals(NULL, NULL, int_cmp, int_cmp));

    // First NULL
    ASSERT_LT(anv_pair_compare(NULL, pair, int_cmp, int_cmp), 0);
    ASSERT_FALSE(anv_pair_equals(NULL, pair, int_cmp, int_cmp));

    // Second NULL
    ASSERT_GT(anv_pair_compare(pair, NULL, int_cmp, int_cmp), 0);
    ASSERT_FALSE(anv_pair_equals(pair, NULL, int_cmp, int_cmp));

    anv_pair_destroy(pair, true, true);
    return TEST_SUCCESS;
}

int test_pair_compare_no_comparison_functions(void)
{
    ANVAllocator alloc = create_int_allocator();

    int* first1 = malloc(sizeof(int));
    int* second1 = malloc(sizeof(int));
    int* first2 = malloc(sizeof(int));
    int* second2 = malloc(sizeof(int));
    *first1 = 42;
    *second1 = 84;
    *first2 = 42;
    *second2 = 84;

    ANVPair* pair1 = anv_pair_create(&alloc, first1, second1);
    ANVPair* pair2 = anv_pair_create(&alloc, first2, second2);

    // Test without comparison functions (should use pointer comparison)
    int result = anv_pair_compare(pair1, pair2, NULL, NULL);
    // Result depends on memory layout, just ensure it doesn't crash
    (void)result; // Suppress unused variable warning

    // Test with only first comparison function
    result = anv_pair_compare(pair1, pair2, int_cmp, NULL);
    (void)result;

    // Test with only second comparison function
    result = anv_pair_compare(pair1, pair2, NULL, int_cmp);
    (void)result;

    anv_pair_destroy(pair1, true, true);
    anv_pair_destroy(pair2, true, true);
    return TEST_SUCCESS;
}

int test_pair_compare_with_strings(void)
{
    ANVAllocator alloc = create_string_allocator();

    char* str1 = malloc(10);
    char* str2 = malloc(10);
    char* str3 = malloc(10);
    char* str4 = malloc(10);
    strcpy(str1, "apple");
    strcpy(str2, "banana");
    strcpy(str3, "apple");
    strcpy(str4, "cherry");

    ANVPair* pair1 = anv_pair_create(&alloc, str1, str2);
    ANVPair* pair2 = anv_pair_create(&alloc, str3, str4);

    // pair1 < pair2 (first elements equal, but "banana" < "cherry")
    int result = anv_pair_compare(pair1, pair2, string_cmp, string_cmp);
    ASSERT_LT(result, 0);

    anv_pair_destroy(pair1, true, true);
    anv_pair_destroy(pair2, true, true);
    return TEST_SUCCESS;
}

int test_pair_compare_with_persons(void)
{
    ANVAllocator alloc = create_person_allocator();

    Person* p1 = create_person("Alice", 25);
    Person* p2 = create_person("Bob", 30);
    Person* p3 = create_person("Alice", 25);
    Person* p4 = create_person("Charlie", 35);

    ANVPair* pair1 = anv_pair_create(&alloc, p1, p2);
    ANVPair* pair2 = anv_pair_create(&alloc, p3, p4);

    // pair1 < pair2 (Alice == Alice, but Bob < Charlie)
    int result = anv_pair_compare(pair1, pair2, person_cmp, person_cmp);
    ASSERT_LT(result, 0);

    anv_pair_destroy(pair1, true, true);
    anv_pair_destroy(pair2, true, true);
    return TEST_SUCCESS;
}

int test_pair_compare_mixed_types(void)
{
    ANVAllocator alloc = create_int_allocator();

    int* first1 = malloc(sizeof(int));
    int* first2 = malloc(sizeof(int));
    char* second1 = malloc(10);
    char* second2 = malloc(10);
    *first1 = 42;
    *first2 = 42;
    strcpy(second1, "apple");
    strcpy(second2, "banana");

    ANVPair* pair1 = anv_pair_create(&alloc, first1, second1);
    ANVPair* pair2 = anv_pair_create(&alloc, first2, second2);

    // First elements equal (42 == 42), second elements different ("apple" < "banana")
    int result = anv_pair_compare(pair1, pair2, int_cmp, string_cmp);
    ASSERT_LT(result, 0);

    anv_pair_destroy(pair1, true, true);
    anv_pair_destroy(pair2, true, true);
    return TEST_SUCCESS;
}

int test_pair_compare_mixed_types_with_copy(void)
{
    ANVAllocator alloc = create_int_allocator();

    int original_first1 = 42;
    int original_first2 = 42;
    const char* original_second1 = "apple";
    const char* original_second2 = "banana";

    // Create pairs with copied data using different copy functions
    int* first1 = int_anv_copy_func(&original_first1);
    int* first2 = int_anv_copy_func(&original_first2);
    char* second1 = string_anv_copy_func(original_second1);
    char* second2 = string_anv_copy_func(original_second2);

    ANVPair* pair1 = anv_pair_create(&alloc, first1, second1);
    ANVPair* pair2 = anv_pair_create(&alloc, first2, second2);

    ASSERT_NOT_NULL(pair1);
    ASSERT_NOT_NULL(pair2);

    // Verify the copies were made correctly
    ASSERT_EQ(*(int*)pair1->first, 42);
    ASSERT_EQ(strcmp((char*)pair1->second, "apple"), 0);
    ASSERT_EQ(*(int*)pair2->first, 42);
    ASSERT_EQ(strcmp((char*)pair2->second, "banana"), 0);

    // First elements equal (42 == 42), second elements different ("apple" < "banana")
    int result = anv_pair_compare(pair1, pair2, int_cmp, string_cmp);
    ASSERT_LT(result, 0);

    // Now we can safely free both elements since they were copied
    anv_pair_destroy(pair1, true, true);
    anv_pair_destroy(pair2, true, true);
    return TEST_SUCCESS;
}

// ============================================================================
// Memory Tests (from test_pair_memory.c)
// ============================================================================

int test_pair_memory_allocation_failure(void)
{
    // Test with failing allocator
    set_alloc_fail_countdown(0); // Fail immediately on first allocation
    ANVAllocator failing_alloc = create_failing_int_allocator();

    ANVPair* pair = anv_pair_create(&failing_alloc, NULL, NULL);
    ASSERT_NULL(pair); // Should fail to allocate pair structure

    return TEST_SUCCESS;
}

int test_pair_copy_deep_allocation_failure(void)
{
    ANVAllocator normal_alloc = create_int_allocator();
    const ANVAllocator failing_alloc = create_failing_int_allocator();

    int* first = malloc(sizeof(int));
    int* second = malloc(sizeof(int));
    *first = 42;
    *second = 84;

    ANVPair* original = anv_pair_create(&normal_alloc, first, second);

    // Test failure during first element copy by using a failing copy function
    set_alloc_fail_countdown(1); // Allocate the Pair structure then fail on the first copy
    ANVPair* copy1 = anv_pair_copy_deep(original, failing_anv_copy_func, normal_alloc.copy, true);
    ASSERT_NULL(copy1);

    // Test failure during second element copy by using a failing copy function
    set_alloc_fail_countdown(1); // Allocate the Pair structure then fail on the second copy
    ANVPair* copy2 = anv_pair_copy_deep(original, normal_alloc.copy, failing_anv_copy_func, true);
    ASSERT_NULL(copy2);

    anv_pair_destroy(original, true, true);
    return TEST_SUCCESS;
}

int test_pair_destroy_null_safe(void)
{
    // Test that destroying NULL pair doesn't crash
    anv_pair_destroy(NULL, true, true);
    anv_pair_destroy(NULL, false, false);

    return TEST_SUCCESS;
}

int test_pair_memory_leak_prevention(void)
{
    ANVAllocator alloc = create_int_allocator();

    int* first = malloc(sizeof(int));
    int* second = malloc(sizeof(int));
    int* new_first = malloc(sizeof(int));
    int* new_second = malloc(sizeof(int));

    *first = 42;
    *second = 84;
    *new_first = 100;
    *new_second = 200;

    ANVPair* pair = anv_pair_create(&alloc, first, second);

    // Test that setting new values with should_free_old=true prevents leaks
    anv_pair_set_first(pair, new_first, true);   // Should free old first
    anv_pair_set_second(pair, new_second, true); // Should free old second

    // Clean up
    anv_pair_destroy(pair, true, true);

    return TEST_SUCCESS;
}

int test_pair_selective_memory_management(void)
{
    ANVAllocator alloc = create_int_allocator();

    int* first = malloc(sizeof(int));
    int* second = malloc(sizeof(int));
    *first = 42;
    *second = 84;

    ANVPair* pair = anv_pair_create(&alloc, first, second);

    // Test destroying with selective memory management
    // Free only first element
    anv_pair_destroy(pair, true, false);

    // Manual cleanup of second (since we didn't free it)
    free(second);

    return TEST_SUCCESS;
}

int test_pair_copy_deep_with_different_anv_copy_functions(void)
{
    ANVAllocator alloc = create_int_allocator();

    int* first = malloc(sizeof(int));
    char* second = malloc(20);
    *first = 42;
    strcpy(second, "hello world");

    ANVPair* original = anv_pair_create(&alloc, first, second);

    // Test deep copy with different copy functions for each element
    ANVPair* deep_copy = anv_pair_copy_deep(original, int_anv_copy_func, string_anv_copy_func, true);
    ASSERT_NOT_NULL(deep_copy);
    ASSERT_NOT_EQ_PTR(deep_copy->first, original->first);
    ASSERT_NOT_EQ_PTR(deep_copy->second, original->second);
    ASSERT_EQ(*(int*)deep_copy->first, 42);
    ASSERT_EQ(strcmp((char*)deep_copy->second, "hello world"), 0);

    anv_pair_destroy(original, true, true);
    anv_pair_destroy(deep_copy, true, true);
    return TEST_SUCCESS;
}

int test_pair_large_data_handling(void)
{
    ANVAllocator alloc = create_string_allocator();

    // Create large strings
    size_t size = 10000;
    char* large_str1 = malloc(size);
    char* large_str2 = malloc(size);

    // Fill with data
    for (size_t i = 0; i < size - 1; i++)
    {
        large_str1[i] = 'A' + (i % 26);
        large_str2[i] = 'a' + (i % 26);
    }
    large_str1[size - 1] = '\0';
    large_str2[size - 1] = '\0';

    ANVPair* pair = anv_pair_create(&alloc, large_str1, large_str2);
    ASSERT_NOT_NULL(pair);

    // Verify data integrity
    ASSERT_EQ(strncmp((char*)pair->first, large_str1, 100), 0);  // Check first 100 chars
    ASSERT_EQ(strncmp((char*)pair->second, large_str2, 100), 0); // Check first 100 chars

    // Test deep copy
    ANVPair* deep_copy = anv_pair_copy_deep(pair, alloc.copy, alloc.copy, true);
    ASSERT_NOT_NULL(deep_copy);
    ASSERT_EQ(strncmp((char*)deep_copy->first, large_str1, 100), 0);  // Check first 100 chars
    ASSERT_EQ(strncmp((char*)deep_copy->second, large_str2, 100), 0); // Check first 100 chars

    // Clean up
    anv_pair_destroy(pair, true, true);
    anv_pair_destroy(deep_copy, true, true);

    return TEST_SUCCESS;
}

int test_pair_multiple_operations_memory_safety(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create multiple pairs and perform various operations
    ANVPair* pairs[10];

    for (int i = 0; i < 10; i++)
    {
        int* first = malloc(sizeof(int));
        int* second = malloc(sizeof(int));
        *first = i * 10;
        *second = i * 20;
        pairs[i] = anv_pair_create(&alloc, first, second);
        ASSERT_NOT_NULL(pairs[i]);
    }

    // Perform swaps on all pairs
    for (int i = 0; i < 10; i++)
    {
        anv_pair_swap(pairs[i]);
        ASSERT_EQ(*(int*)pairs[i]->first, i * 20);
        ASSERT_EQ(*(int*)pairs[i]->second, i * 10);
    }

    // Create copies
    ANVPair* copies[10];
    for (int i = 0; i < 10; i++)
    {
        copies[i] = anv_pair_copy_deep(pairs[i], alloc.copy, alloc.copy, true);
        ASSERT_NOT_NULL(copies[i]);
    }

    // Clean up all pairs
    for (int i = 0; i < 10; i++)
    {
        anv_pair_destroy(pairs[i], true, true);
        anv_pair_destroy(copies[i], true, true);
    }

    return TEST_SUCCESS;
}

int test_pair_edge_case_null_elements(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Test pair with both elements NULL
    ANVPair* pair = anv_pair_create(&alloc, NULL, NULL);
    ASSERT_NOT_NULL(pair);
    ASSERT_NULL(pair->first);
    ASSERT_NULL(pair->second);

    // Test operations on NULL elements
    ASSERT_NULL(anv_pair_first(pair));
    ASSERT_NULL(anv_pair_second(pair));

    // Test swap with NULL elements
    anv_pair_swap(pair);
    ASSERT_NULL(pair->first);
    ASSERT_NULL(pair->second);

    // Test setting non-NULL values
    int* value1 = malloc(sizeof(int));
    int* value2 = malloc(sizeof(int));
    *value1 = 42;
    *value2 = 84;

    anv_pair_set_first(pair, value1, false); // No old value to free
    anv_pair_set_second(pair, value2, false);

    ASSERT_EQ(*(int*)pair->first, 42);
    ASSERT_EQ(*(int*)pair->second, 84);

    anv_pair_destroy(pair, true, true);
    return TEST_SUCCESS;
}

// ============================================================================
// Properties Tests (from test_pair_properties.c)
// ============================================================================

int test_pair_symmetry_property(void)
{
    ANVAllocator alloc = create_int_allocator();

    int* first1 = malloc(sizeof(int));
    int* second1 = malloc(sizeof(int));
    int* first2 = malloc(sizeof(int));
    int* second2 = malloc(sizeof(int));
    *first1 = 42;
    *second1 = 84;
    *first2 = 100;
    *second2 = 200;

    ANVPair* pair1 = anv_pair_create(&alloc, first1, second1);
    ANVPair* pair2 = anv_pair_create(&alloc, first2, second2);

    // Test symmetry: if pair1 < pair2, then pair2 > pair1
    int result1 = anv_pair_compare(pair1, pair2, int_cmp, int_cmp);
    int result2 = anv_pair_compare(pair2, pair1, int_cmp, int_cmp);

    ASSERT_LT(result1, 0);
    ASSERT_GT(result2, 0);
    ASSERT_EQ(result1, -result2);

    anv_pair_destroy(pair1, true, true);
    anv_pair_destroy(pair2, true, true);
    return TEST_SUCCESS;
}

int test_pair_reflexivity_property(void)
{
    ANVAllocator alloc = create_int_allocator();

    int* first = malloc(sizeof(int));
    int* second = malloc(sizeof(int));
    *first = 42;
    *second = 84;

    ANVPair* pair = anv_pair_create(&alloc, first, second);

    // Test reflexivity: pair should equal itself
    ASSERT_EQ(anv_pair_compare(pair, pair, int_cmp, int_cmp), 0);
    ASSERT_TRUE(anv_pair_equals(pair, pair, int_cmp, int_cmp));

    anv_pair_destroy(pair, true, true);
    return TEST_SUCCESS;
}

int test_pair_transitivity_property(void)
{
    ANVAllocator alloc = create_int_allocator();

    int* first1 = malloc(sizeof(int));
    int* second1 = malloc(sizeof(int));
    int* first2 = malloc(sizeof(int));
    int* second2 = malloc(sizeof(int));
    int* first3 = malloc(sizeof(int));
    int* second3 = malloc(sizeof(int));
    *first1 = 10;
    *second1 = 20;
    *first2 = 30;
    *second2 = 40;
    *first3 = 50;
    *second3 = 60;

    ANVPair* pair1 = anv_pair_create(&alloc, first1, second1);
    ANVPair* pair2 = anv_pair_create(&alloc, first2, second2);
    ANVPair* pair3 = anv_pair_create(&alloc, first3, second3);

    // Test transitivity: if pair1 < pair2 and pair2 < pair3, then pair1 < pair3
    int result12 = anv_pair_compare(pair1, pair2, int_cmp, int_cmp);
    int result23 = anv_pair_compare(pair2, pair3, int_cmp, int_cmp);
    int result13 = anv_pair_compare(pair1, pair3, int_cmp, int_cmp);

    ASSERT_LT(result12, 0);
    ASSERT_LT(result23, 0);
    ASSERT_LT(result13, 0);

    anv_pair_destroy(pair1, true, true);
    anv_pair_destroy(pair2, true, true);
    anv_pair_destroy(pair3, true, true);
    return TEST_SUCCESS;
}

int test_pair_swap_idempotency(void)
{
    ANVAllocator alloc = create_int_allocator();

    int* first = malloc(sizeof(int));
    int* second = malloc(sizeof(int));
    *first = 42;
    *second = 84;

    ANVPair* pair = anv_pair_create(&alloc, first, second);

    void* original_first = pair->first;
    void* original_second = pair->second;

    // Swap twice should return to original state
    anv_pair_swap(pair);
    anv_pair_swap(pair);

    ASSERT_EQ_PTR(pair->first, original_first);
    ASSERT_EQ_PTR(pair->second, original_second);

    anv_pair_destroy(pair, true, true);
    return TEST_SUCCESS;
}

int test_pair_copy_independence(void)
{
    ANVAllocator alloc = create_int_allocator();

    int* first = malloc(sizeof(int));
    int* second = malloc(sizeof(int));
    *first = 42;
    *second = 84;

    ANVPair* original = anv_pair_create(&alloc, first, second);
    ANVPair* copy = anv_pair_copy_deep(original, int_anv_copy_func, int_anv_copy_func, true);

    // Modify original
    int* new_value = malloc(sizeof(int));
    *new_value = 999;
    anv_pair_set_first(original, new_value, true);

    // Copy should be unchanged
    ASSERT_EQ(*(int*)copy->first, 42);
    ASSERT_NOT_EQ(*(int*)original->first, *(int*)copy->first);

    anv_pair_destroy(original, true, true);
    anv_pair_destroy(copy, true, true);
    return TEST_SUCCESS;
}

int test_pair_shallow_copy_dependency(void)
{
    ANVAllocator alloc = create_int_allocator();

    int* first = malloc(sizeof(int));
    int* second = malloc(sizeof(int));
    *first = 42;
    *second = 84;

    ANVPair* original = anv_pair_create(&alloc, first, second);
    ANVPair* shallow = anv_pair_copy(original);

    // Shallow copy shares data
    ASSERT_EQ_PTR(shallow->first, original->first);
    ASSERT_EQ_PTR(shallow->second, original->second);

    // Modifying through one affects the other
    *(int*)original->first = 999;
    ASSERT_EQ(*(int*)shallow->first, 999);

    anv_pair_destroy(original, true, true);
    anv_pair_destroy(shallow, false, false); // Don't free shared data
    return TEST_SUCCESS;
}

int test_pair_lexicographic_ordering(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create pairs to test lexicographic ordering: (1,2) < (1,3) < (2,1) < (2,2)
    int vals[4][2] = {{1, 2}, {1, 3}, {2, 1}, {2, 2}};
    ANVPair* pairs[4];

    for (int i = 0; i < 4; i++)
    {
        int* first = malloc(sizeof(int));
        int* second = malloc(sizeof(int));
        *first = vals[i][0];
        *second = vals[i][1];
        pairs[i] = anv_pair_create(&alloc, first, second);
    }

    // Test ordering: (1,2) < (1,3) < (2,1) < (2,2)
    ASSERT_LT(anv_pair_compare(pairs[0], pairs[1], int_cmp, int_cmp), 0); // (1,2) < (1,3)
    ASSERT_LT(anv_pair_compare(pairs[1], pairs[2], int_cmp, int_cmp), 0); // (1,3) < (2,1)
    ASSERT_LT(anv_pair_compare(pairs[2], pairs[3], int_cmp, int_cmp), 0); // (2,1) < (2,2)

    for (int i = 0; i < 4; i++)
    {
        anv_pair_destroy(pairs[i], true, true);
    }
    return TEST_SUCCESS;
}

int test_pair_comparison_consistency(void)
{
    ANVAllocator alloc = create_int_allocator();

    int* first1 = malloc(sizeof(int));
    int* second1 = malloc(sizeof(int));
    int* first2 = malloc(sizeof(int));
    int* second2 = malloc(sizeof(int));
    *first1 = 42;
    *second1 = 84;
    *first2 = 42;
    *second2 = 84;

    ANVPair* pair1 = anv_pair_create(&alloc, first1, second1);
    ANVPair* pair2 = anv_pair_create(&alloc, first2, second2);

    // Multiple calls should return consistent results
    for (int i = 0; i < 5; i++)
    {
        ASSERT_EQ(anv_pair_compare(pair1, pair2, int_cmp, int_cmp), 0);
        ASSERT_TRUE(anv_pair_equals(pair1, pair2, int_cmp, int_cmp));
    }

    anv_pair_destroy(pair1, true, true);
    anv_pair_destroy(pair2, true, true);
    return TEST_SUCCESS;
}

int test_pair_different_allocators(void)
{
    ANVAllocator alloc1 = create_int_allocator();
    ANVAllocator alloc2 = create_int_allocator();

    int* first1 = malloc(sizeof(int));
    int* second1 = malloc(sizeof(int));
    int* first2 = malloc(sizeof(int));
    int* second2 = malloc(sizeof(int));
    *first1 = 42;
    *second1 = 84;
    *first2 = 42;
    *second2 = 84;

    ANVPair* pair1 = anv_pair_create(&alloc1, first1, second1);
    ANVPair* pair2 = anv_pair_create(&alloc2, first2, second2);

    // Pairs with different allocators should still be comparable
    ASSERT_EQ(anv_pair_compare(pair1, pair2, int_cmp, int_cmp), 0);
    ASSERT_TRUE(anv_pair_equals(pair1, pair2, int_cmp, int_cmp));

    anv_pair_destroy(pair1, true, true);
    anv_pair_destroy(pair2, true, true);
    return TEST_SUCCESS;
}

int test_pair_boundary_values(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Test with extreme values
    int* min_val = malloc(sizeof(int));
    int* max_val = malloc(sizeof(int));
    int* zero = malloc(sizeof(int));
    int* min_val2 = malloc(sizeof(int));
    int* zero2 = malloc(sizeof(int));
    int* max_val2 = malloc(sizeof(int));
    *min_val = INT_MIN;
    *max_val = INT_MAX;
    *zero = 0;
    *min_val2 = INT_MIN;
    *zero2 = 0;
    *max_val2 = INT_MAX;

    ANVPair* pair1 = anv_pair_create(&alloc, min_val, max_val);
    ANVPair* pair2 = anv_pair_create(&alloc, zero, zero2);
    ANVPair* pair3 = anv_pair_create(&alloc, max_val2, min_val2);

    // Test comparisons with boundary values
    ASSERT_LT(anv_pair_compare(pair1, pair2, int_cmp, int_cmp), 0); // (INT_MIN, INT_MAX) < (0, 0)
    ASSERT_LT(anv_pair_compare(pair2, pair3, int_cmp, int_cmp), 0); // (0, 0) < (INT_MAX, INT_MIN)

    anv_pair_destroy(pair1, true, true);
    anv_pair_destroy(pair2, true, true);
    anv_pair_destroy(pair3, true, true);
    return TEST_SUCCESS;
}

// ============================================================================
// Main - Combined test runner (36 tests total)
// ============================================================================

int main(void)
{
    const ANVTestCase tests[] = {
        // CRUD Tests (8)
        {test_pair_create_destroy, "test_pair_create_destroy"},
        {test_pair_create_with_null_elements, "test_pair_create_with_null_elements"},
        {test_pair_create_invalid_allocator, "test_pair_create_invalid_allocator"},
        {test_pair_accessors, "test_pair_accessors"},
        {test_pair_setters, "test_pair_setters"},
        {test_pair_swap, "test_pair_swap"},
        {test_pair_anv_copy_functions, "test_pair_anv_copy_functions"},
        {test_pair_mixed_type_copy, "test_pair_mixed_type_copy"},

        // Comparison Tests (9)
        {test_pair_compare_equal_pairs, "test_pair_compare_equal_pairs"},
        {test_pair_compare_first_different, "test_pair_compare_first_different"},
        {test_pair_compare_second_different, "test_pair_compare_second_different"},
        {test_pair_compare_null_pairs, "test_pair_compare_null_pairs"},
        {test_pair_compare_no_comparison_functions, "test_pair_compare_no_comparison_functions"},
        {test_pair_compare_with_strings, "test_pair_compare_with_strings"},
        {test_pair_compare_with_persons, "test_pair_compare_with_persons"},
        {test_pair_compare_mixed_types, "test_pair_compare_mixed_types"},
        {test_pair_compare_mixed_types_with_copy, "test_pair_compare_mixed_types_with_copy"},

        // Memory Tests (9)
        {test_pair_memory_allocation_failure, "test_pair_memory_allocation_failure"},
        {test_pair_copy_deep_allocation_failure, "test_pair_copy_deep_allocation_failure"},
        {test_pair_destroy_null_safe, "test_pair_destroy_null_safe"},
        {test_pair_memory_leak_prevention, "test_pair_memory_leak_prevention"},
        {test_pair_selective_memory_management, "test_pair_selective_memory_management"},
        {test_pair_copy_deep_with_different_anv_copy_functions, "test_pair_copy_deep_with_different_anv_copy_functions"},
        {test_pair_large_data_handling, "test_pair_large_data_handling"},
        {test_pair_multiple_operations_memory_safety, "test_pair_multiple_operations_memory_safety"},
        {test_pair_edge_case_null_elements, "test_pair_edge_case_null_elements"},

        // Properties Tests (10)
        {test_pair_symmetry_property, "test_pair_symmetry_property"},
        {test_pair_reflexivity_property, "test_pair_reflexivity_property"},
        {test_pair_transitivity_property, "test_pair_transitivity_property"},
        {test_pair_swap_idempotency, "test_pair_swap_idempotency"},
        {test_pair_copy_independence, "test_pair_copy_independence"},
        {test_pair_shallow_copy_dependency, "test_pair_shallow_copy_dependency"},
        {test_pair_lexicographic_ordering, "test_pair_lexicographic_ordering"},
        {test_pair_comparison_consistency, "test_pair_comparison_consistency"},
        {test_pair_different_allocators, "test_pair_different_allocators"},
        {test_pair_boundary_values, "test_pair_boundary_values"},
    };

    return anv_run_tests("Pair", tests, sizeof(tests) / sizeof(tests[0]));
}
