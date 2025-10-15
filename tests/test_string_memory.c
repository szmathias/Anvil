//
// Created by zack on 9/3/25.
//

#include "containers/dynamicstring.h"
#include "TestAssert.h"

#include <stdio.h>
#include <stdlib.h>

int test_reserve_and_shrink(void)
{
    ANVString str = anv_str_create_empty(4);
    const size_t old_capacity = anv_str_capacity(&str);
    ASSERT_TRUE(anv_str_reserve(&str, 128));
    ASSERT_GT(anv_str_capacity(&str), old_capacity);
    anv_str_assign_cstring(&str, "abc");
    ASSERT_TRUE(anv_str_shrink_to_fit(&str));
    ASSERT_GTE(anv_str_capacity(&str), anv_str_size(&str) + 1);
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_buffer_growth(void)
{
    ANVString str = anv_str_create_empty(4);
    const size_t initial_capacity = anv_str_capacity(&str);
    // Append enough characters to force buffer growth
    for (int i = 0; i < 100; ++i)
    {
        anv_str_push_back(&str, 'x');
    }
    ASSERT_GT(anv_str_capacity(&str), initial_capacity);
    ASSERT_EQ(anv_str_size(&str), 100);
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_large_string(void)
{
    ANVString str = anv_str_create_empty(1024);
    for (int i = 0; i < 1000; ++i)
    {
        anv_str_push_back(&str, 'x');
    }
    ASSERT_EQ(anv_str_size(&str), 1000);
    ASSERT_GT(anv_str_capacity(&str), 1000);
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_reserve_and_shrink_optimal(void)
{
    ANVString str = anv_str_create_from_cstring("abc");
    const size_t cap = anv_str_capacity(&str);
    ASSERT_FALSE(anv_str_reserve(&str, cap));
    ASSERT_TRUE(anv_str_shrink_to_fit(&str));
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

typedef struct
{
    int (*func)(void);
    const char* name;
} TestCase;

TestCase tests[] = {
    {test_reserve_and_shrink, "test_reserve_and_shrink"},
    {test_buffer_growth, "test_buffer_growth"},
    {test_large_string, "test_large_string"},
    {test_reserve_and_shrink_optimal, "test_reserve_and_shrink_optimal"},
};

int main(void)
{
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
        printf("All DString Memory tests passed.\n");
        return 0;
    }

    printf("%d DString Memory tests failed.\n", failed);
    return 1;
}