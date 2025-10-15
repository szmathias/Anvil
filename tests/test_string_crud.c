//
// Created by zack on 9/3/25.
//

#include "containers/dynamicstring.h"
#include "TestAssert.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int test_create_and_assign(void)
{
    ANVString str = anv_str_create_empty(32);
    ASSERT_EQ(anv_str_size(&str), 0);
    anv_str_assign_cstring(&str, "Hello");
    ASSERT_EQ_STR(anv_str_data(&str), "Hello");
    ASSERT_EQ(anv_str_size(&str), 5);
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_append_and_insert(void)
{
    ANVString str = anv_str_create_empty(16);
    anv_str_assign_cstring(&str, "abc");
    anv_str_append_cstring(&str, "def");
    ASSERT_EQ_STR(anv_str_data(&str), "abcdef");
    anv_str_insert_cstring(&str, 3, "XYZ");
    ASSERT_EQ_STR(anv_str_data(&str), "abcXYZdef");
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_push_pop_erase(void)
{
    ANVString str = anv_str_create_empty(8);
    anv_str_assign_cstring(&str, "hi");
    anv_str_push_back(&str, '!');
    ASSERT_EQ_STR(anv_str_data(&str), "hi!");
    anv_str_pop_back(&str);
    ASSERT_EQ_STR(anv_str_data(&str), "hi");
    anv_str_erase(&str, 0);
    ASSERT_EQ_STR(anv_str_data(&str), "i");
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_empty_string(void)
{
    ANVString str = anv_str_create_empty(0);
    ASSERT_EQ(anv_str_size(&str), 0);
    ASSERT_TRUE(anv_str_empty(&str));
    ASSERT_EQ_STR(anv_str_data(&str), "");
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_assign_empty_cstring(void)
{
    ANVString str = anv_str_create_empty(8);
    anv_str_assign_cstring(&str, "");
    ASSERT_EQ(anv_str_size(&str), 0);
    ASSERT_TRUE(anv_str_empty(&str));
    ASSERT_EQ_STR(anv_str_data(&str), "");
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_append_empty_cstring(void)
{
    ANVString str = anv_str_create_from_cstring("abc");
    anv_str_append_cstring(&str, "");
    ASSERT_EQ_STR(anv_str_data(&str), "abc");
    ASSERT_EQ(anv_str_size(&str), 3);
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_insert_at_bounds(void)
{
    ANVString str = anv_str_create_from_cstring("abc");
    anv_str_insert_cstring(&str, 0, "X");
    ASSERT_EQ_STR(anv_str_data(&str), "Xabc");
    anv_str_insert_cstring(&str, anv_str_size(&str), "Y");
    ASSERT_EQ_STR(anv_str_data(&str), "XabcY");
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_erase_out_of_bounds(void)
{
    ANVString str = anv_str_create_from_cstring("abc");
    anv_str_erase(&str, 10); // Should do nothing
    ASSERT_EQ_STR(anv_str_data(&str), "abc");
    anv_str_erase(&str, (size_t)-1); // Should do nothing (size_t -1 is large)
    ASSERT_EQ_STR(anv_str_data(&str), "abc");
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_self_assign_and_append(void)
{
    ANVString str = anv_str_create_from_cstring("abc");
    anv_str_assign_string(&str, &str);
    ASSERT_EQ_STR(anv_str_data(&str), "abc");
    anv_str_append_string(&str, &str);
    ASSERT_EQ_STR(anv_str_data(&str), "abcabc");
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_clear_non_empty(void)
{
    ANVString str = anv_str_create_from_cstring("not empty");
    anv_str_clear(&str);
    ASSERT_TRUE(anv_str_empty(&str));
    ASSERT_EQ(anv_str_size(&str), 0);
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_pop_back_empty(void)
{
    ANVString str = anv_str_create_empty(8);
    anv_str_pop_back(&str); // Should not crash
    ASSERT_TRUE(anv_str_empty(&str));
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_erase_empty(void)
{
    ANVString str = anv_str_create_empty(8);
    anv_str_erase(&str, 0); // Should not crash
    ASSERT_TRUE(anv_str_empty(&str));
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_assign_char(void)
{
    ANVString str = anv_str_create_empty(8);
    anv_str_assign_char(&str, 'A');
    ASSERT_EQ_STR(anv_str_data(&str), "A");
    ASSERT_EQ(anv_str_size(&str), 1);
    anv_str_assign_char(&str, '\0');
    ASSERT_EQ(anv_str_size(&str), 1);
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_insert_char_positions(void)
{
    ANVString str = anv_str_create_from_cstring("ac");
    anv_str_insert_char(&str, 1, 'b'); // Insert in middle
    ASSERT_EQ_STR(anv_str_data(&str), "abc");
    anv_str_insert_char(&str, 0, 'X'); // Insert at start
    ASSERT_EQ_STR(anv_str_data(&str), "Xabc");
    anv_str_insert_char(&str, anv_str_size(&str), 'Y'); // Insert at end
    ASSERT_EQ_STR(anv_str_data(&str), "XabcY");
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_append_char_multiple(void)
{
    ANVString str = anv_str_create_empty(4);
    anv_str_append_char(&str, 'a');
    anv_str_append_char(&str, 'b');
    anv_str_append_char(&str, 'c');
    ASSERT_EQ_STR(anv_str_data(&str), "abc");
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_clear_already_empty(void)
{
    ANVString str = anv_str_create_empty(8);
    anv_str_clear(&str);
    ASSERT_TRUE(anv_str_empty(&str));
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_assign_string_different(void)
{
    ANVString str1 = anv_str_create_from_cstring("foo");
    ANVString str2 = anv_str_create_from_cstring("bar");
    anv_str_assign_string(&str1, &str2);
    ASSERT_EQ_STR(anv_str_data(&str1), "bar");
    anv_str_destroy(&str1);
    anv_str_destroy(&str2);
    return TEST_SUCCESS;
}

int test_insert_cstring_empty(void)
{
    ANVString str = anv_str_create_from_cstring("abc");
    anv_str_insert_cstring(&str, 1, "");
    ASSERT_EQ_STR(anv_str_data(&str), "abc");
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_insert_string_empty(void)
{
    ANVString str = anv_str_create_from_cstring("abc");
    ANVString empty = anv_str_create_empty(4);
    anv_str_insert_string(&str, 1, &empty);
    ASSERT_EQ_STR(anv_str_data(&str), "abc");
    anv_str_destroy(&str);
    anv_str_destroy(&empty);
    return TEST_SUCCESS;
}

int test_append_string_empty(void)
{
    ANVString str = anv_str_create_from_cstring("abc");
    ANVString empty = anv_str_create_empty(4);
    anv_str_append_string(&str, &empty);
    ASSERT_EQ_STR(anv_str_data(&str), "abc");
    anv_str_destroy(&str);
    anv_str_destroy(&empty);
    return TEST_SUCCESS;
}

int test_push_back_null_char(void)
{
    ANVString str = anv_str_create_empty(8);
    anv_str_push_back(&str, '\0');
    ASSERT_EQ(anv_str_size(&str), 1);
    ASSERT_EQ(anv_str_data(&str)[0], '\0');
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_append_char_null_char(void)
{
    ANVString str = anv_str_create_empty(8);
    anv_str_append_char(&str, '\0');
    ASSERT_EQ(anv_str_size(&str), 1);
    ASSERT_EQ(anv_str_data(&str)[0], '\0');
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_insert_char_out_of_bounds(void)
{
    ANVString str = anv_str_create_from_cstring("abc");
    anv_str_insert_char(&str, 10, 'X'); // Should do nothing
    ASSERT_EQ_STR(anv_str_data(&str), "abc");
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_insert_cstring_out_of_bounds(void)
{
    ANVString str = anv_str_create_from_cstring("abc");
    anv_str_insert_cstring(&str, 10, "XYZ"); // Should do nothing
    ASSERT_EQ_STR(anv_str_data(&str), "abc");
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_insert_string_out_of_bounds(void)
{
    ANVString str = anv_str_create_from_cstring("abc");
    ANVString other = anv_str_create_from_cstring("XYZ");
    anv_str_insert_string(&str, 10, &other); // Should do nothing
    ASSERT_EQ_STR(anv_str_data(&str), "abc");
    anv_str_destroy(&str);
    anv_str_destroy(&other);
    return TEST_SUCCESS;
}

int test_erase_at_size(void)
{
    ANVString str = anv_str_create_from_cstring("abc");
    anv_str_erase(&str, anv_str_size(&str)); // Should do nothing
    ASSERT_EQ_STR(anv_str_data(&str), "abc");
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_assign_string_self(void)
{
    ANVString str = anv_str_create_from_cstring("self");
    anv_str_assign_string(&str, &str);
    ASSERT_EQ_STR(anv_str_data(&str), "self");
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_append_string_self(void)
{
    ANVString str = anv_str_create_from_cstring("dup");
    anv_str_append_string(&str, &str);
    ASSERT_EQ_STR(anv_str_data(&str), "dupdup");
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_insert_char_at_0_and_size(void)
{
    ANVString str = anv_str_create_from_cstring("bc");
    anv_str_insert_char(&str, 0, 'A');
    ASSERT_EQ_STR(anv_str_data(&str), "Abc");
    anv_str_insert_char(&str, anv_str_size(&str), 'Z');
    ASSERT_EQ_STR(anv_str_data(&str), "AbcZ");
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_insert_cstring_at_0_and_size(void)
{
    ANVString str = anv_str_create_from_cstring("bc");
    anv_str_insert_cstring(&str, 0, "A");
    ASSERT_EQ_STR(anv_str_data(&str), "Abc");
    anv_str_insert_cstring(&str, anv_str_size(&str), "Z");
    ASSERT_EQ_STR(anv_str_data(&str), "AbcZ");
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_insert_string_at_0_and_size(void)
{
    ANVString str = anv_str_create_from_cstring("bc");
    ANVString sA = anv_str_create_from_cstring("A");
    ANVString sZ = anv_str_create_from_cstring("Z");
    anv_str_insert_string(&str, 0, &sA);
    ASSERT_EQ_STR(anv_str_data(&str), "Abc");
    anv_str_insert_string(&str, anv_str_size(&str), &sZ);
    ASSERT_EQ_STR(anv_str_data(&str), "AbcZ");
    anv_str_destroy(&str);
    anv_str_destroy(&sA);
    anv_str_destroy(&sZ);
    return TEST_SUCCESS;
}

typedef struct
{
    int (*func)(void);
    const char* name;
} TestCase;

TestCase tests[] = {
    {test_create_and_assign, "test_create_and_assign"},
    {test_append_and_insert, "test_append_and_insert"},
    {test_push_pop_erase, "test_push_pop_erase"},
    {test_empty_string, "test_empty_string"},
    {test_assign_empty_cstring, "test_assign_empty_cstring"},
    {test_append_empty_cstring, "test_append_empty_cstring"},
    {test_insert_at_bounds, "test_insert_at_bounds"},
    {test_erase_out_of_bounds, "test_erase_out_of_bounds"},
    {test_self_assign_and_append, "test_self_assign_and_append"},
    {test_clear_non_empty, "test_clear_non_empty"},
    {test_pop_back_empty, "test_pop_back_empty"},
    {test_erase_empty, "test_erase_empty"},
    {test_assign_char, "test_assign_char"},
    {test_insert_char_positions, "test_insert_char_positions"},
    {test_append_char_multiple, "test_append_char_multiple"},
    {test_clear_already_empty, "test_clear_already_empty"},
    {test_assign_string_different, "test_assign_string_different"},
    {test_insert_cstring_empty, "test_insert_cstring_empty"},
    {test_insert_string_empty, "test_insert_string_empty"},
    {test_append_string_empty, "test_append_string_empty"},
    {test_push_back_null_char, "test_push_back_null_char"},
    {test_append_char_null_char, "test_append_char_null_char"},
    {test_insert_char_out_of_bounds, "test_insert_char_out_of_bounds"},
    {test_insert_cstring_out_of_bounds, "test_insert_cstring_out_of_bounds"},
    {test_insert_string_out_of_bounds, "test_insert_string_out_of_bounds"},
    {test_erase_at_size, "test_erase_at_size"},
    {test_assign_string_self, "test_assign_string_self"},
    {test_append_string_self, "test_append_string_self"},
    {test_insert_char_at_0_and_size, "test_insert_char_at_0_and_size"},
    {test_insert_cstring_at_0_and_size, "test_insert_cstring_at_0_and_size"},
    {test_insert_string_at_0_and_size, "test_insert_string_at_0_and_size"},
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
        printf("All DString CRUD tests passed.\n");
        return 0;
    }

    printf("%d DString CRUD tests failed.\n", failed);
    return 1;
}