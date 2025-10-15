//
// Created by zack on 9/15/25.
//

#include "containers/pair.h"
#include "TestAssert.h"
#include "TestHelpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

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
    ANVPair* copy = anv_pair_copy_deep(original, true, int_anv_copy_func, int_anv_copy_func);

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

int main(void)
{
    printf("Running Pair properties tests...\n");

    int (*tests[])(void) = {
        test_pair_symmetry_property,
        test_pair_reflexivity_property,
        test_pair_transitivity_property,
        test_pair_swap_idempotency,
        test_pair_copy_independence,
        test_pair_shallow_copy_dependency,
        test_pair_lexicographic_ordering,
        test_pair_comparison_consistency,
        test_pair_different_allocators,
        test_pair_boundary_values,
    };

    const char* test_names[] = {
        "test_pair_symmetry_property",
        "test_pair_reflexivity_property",
        "test_pair_transitivity_property",
        "test_pair_swap_idempotency",
        "test_pair_copy_independence",
        "test_pair_shallow_copy_dependency",
        "test_pair_lexicographic_ordering",
        "test_pair_comparison_consistency",
        "test_pair_different_allocators",
        "test_pair_boundary_values",
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

    printf("Pair properties tests: %d/%d passed\n", passed, num_tests);
    return (passed == num_tests) ? 0 : 1;
}