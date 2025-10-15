//
// Created by zack on 9/15/25.
//

#include "containers/pair.h"
#include "TestAssert.h"
#include "TestHelpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper copy functions for testing
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

// Custom string comparison function
static int string_cmp(const void* a, const void* b)
{
    return strcmp((const char*)a, (const char*)b);
}

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

int main(void)
{
    printf("Running Pair comparison tests...\n");

    int (*tests[])(void) = {
        test_pair_compare_equal_pairs,
        test_pair_compare_first_different,
        test_pair_compare_second_different,
        test_pair_compare_null_pairs,
        test_pair_compare_no_comparison_functions,
        test_pair_compare_with_strings,
        test_pair_compare_with_persons,
        test_pair_compare_mixed_types,
        test_pair_compare_mixed_types_with_copy,
    };

    const char* test_names[] = {
        "test_pair_compare_equal_pairs",
        "test_pair_compare_first_different",
        "test_pair_compare_second_different",
        "test_pair_compare_null_pairs",
        "test_pair_compare_no_comparison_functions",
        "test_pair_compare_with_strings",
        "test_pair_compare_with_persons",
        "test_pair_compare_mixed_types",
        "test_pair_compare_mixed_types_with_copy",
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int passed = 0;

    for (int i = 0; i < num_tests; i++)
    {
        printf("Running %s... ", test_names[i]);
        if (tests[i]() == TEST_SUCCESS)
        {
            printf("PASSED\n");
            passed++;
        }
        else
        {
            printf("FAILED\n");
        }
    }

    printf("Pair comparison tests: %d/%d passed\n", passed, num_tests);
    return (passed == num_tests) ? 0 : 1;
}