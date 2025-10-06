//
// Created by zack on 9/4/25.
//

#include "containers/dynamicstring.h"
#include "TestAssert.h"
#include <stdlib.h>
#include <stdio.h>

#ifdef ANV_PLATFORM_WINDOWS
#include <io.h>
#include <process.h>
#else
#include <unistd.h>
#endif

// Property: The size of a string should never exceed its capacity.
int test_string_size_and_capacity(void)
{
    ANVString str = anv_str_create_empty(0);
    ASSERT_GTE(anv_str_capacity(&str), anv_str_size(&str));

    anv_str_assign_cstring(&str, "hello");
    ASSERT_GTE(anv_str_capacity(&str), anv_str_size(&str));

    for (int i = 0; i < 100; ++i)
    {
        anv_str_push_back(&str, 'a');
        ASSERT_GTE(anv_str_capacity(&str), anv_str_size(&str));
    }

    anv_str_shrink_to_fit(&str);
    ASSERT_GTE(anv_str_capacity(&str), anv_str_size(&str));

    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

// Property: Trimming an already-trimmed string should not change it.
int test_string_idempotent_trim(void)
{
    ANVString str = anv_str_create_from_cstring("no whitespace");
    ANVString copy = anv_str_create_from_string(&str);

    anv_str_trim_front(&str);
    anv_str_trim_back(&str);

    ASSERT_EQ_DSTRING(&str, &copy);

    anv_str_destroy(&str);
    anv_str_destroy(&copy);
    return TEST_SUCCESS;
}

// Property: Converting to lower than upper case should be the same as just converting to upper case.
int test_string_case_conversion_reversibility(void)
{
    ANVString str1 = anv_str_create_from_cstring("MiXeD cAsE 123!");
    ANVString str2 = anv_str_create_from_string(&str1);

    anv_str_to_lower(&str1);
    anv_str_to_upper(&str1);

    anv_str_to_upper(&str2);

    ASSERT_EQ_DSTRING(&str1, &str2);

    anv_str_destroy(&str1);
    anv_str_destroy(&str2);
    return TEST_SUCCESS;
}

typedef struct
{
    int (*func)(void);
    const char* name;
} TestCase;

TestCase tests[] = {
    {test_string_size_and_capacity, "test_string_size_and_capacity"},
    {test_string_idempotent_trim, "test_string_idempotent_trim"},
    {test_string_case_conversion_reversibility, "test_string_case_conversion_reversibility"},
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
        printf("All DString Property tests passed.\n");
        return 0;
    }

    printf("%d DString Property tests failed.\n", failed);
    return 1;
}