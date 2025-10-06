//
// Created by zack on 9/3/25.
//

#include "containers/dynamicstring.h"
#include "TestAssert.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int test_find_and_compare(void)
{
    ANVString str = anv_str_create_from_cstring("abcdefgabc");
    ASSERT_EQ(anv_str_find_cstring(&str, "abc"), 0);
    ASSERT_EQ(anv_str_find_cstring(&str, "fg"), 5);
    ASSERT_EQ(anv_str_find_cstring(&str, "xyz"), STR_NPOS);
    ASSERT_EQ(anv_str_compare_cstring(&str, "abcdefgabc"), 0);
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_trim_and_case(void)
{
    ANVString str = anv_str_create_from_cstring("   Hello World!   ");
    anv_str_trim_front(&str);
    ASSERT_EQ_STR(anv_str_data(&str), "Hello World!   ");
    anv_str_trim_back(&str);
    ASSERT_EQ_STR(anv_str_data(&str), "Hello World!");
    anv_str_to_lower(&str);
    ASSERT_EQ_STR(anv_str_data(&str), "hello world!");
    anv_str_to_upper(&str);
    ASSERT_EQ_STR(anv_str_data(&str), "HELLO WORLD!");
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_substr(void)
{
    ANVString str = anv_str_create_from_cstring("abcdef");
    ANVString sub = anv_str_substr_create_string(&str, 2, 3);
    ASSERT_EQ_STR(anv_str_data(&sub), "cde");
    anv_str_destroy(&sub);
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_substr_out_of_bounds(void)
{
    ANVString str = anv_str_create_from_cstring("abc");
    ANVString sub1 = anv_str_substr_create_string(&str, 10, 2);
    ASSERT_EQ(anv_str_size(&sub1), 0);
    ASSERT_EQ_STR(anv_str_data(&sub1), "");
    ANVString sub2 = anv_str_substr_create_string(&str, 1, 10);
    ASSERT_EQ_STR(anv_str_data(&sub2), "bc");
    anv_str_destroy(&sub1);
    anv_str_destroy(&sub2);
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_embedded_null(void)
{
    ANVString str = anv_str_create_empty(16);
    const char data[] = {'a', 'b', '\0', 'c', 'd', '\0'};
    anv_str_assign_cstring(&str, data);
    ASSERT_EQ(anv_str_size(&str), 2); // Only up to first null
    ASSERT_EQ_STR(anv_str_data(&str), "ab");
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_trim_all_whitespace(void)
{
    ANVString str = anv_str_create_from_cstring("    \t\n  ");
    anv_str_trim_front(&str);
    anv_str_trim_back(&str);
    ASSERT_TRUE(anv_str_empty(&str) || anv_str_size(&str) == 0);
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_compare_different_lengths(void)
{
    ANVString str1 = anv_str_create_from_cstring("abc");
    ANVString str2 = anv_str_create_from_cstring("abcd");
    ASSERT_LT(anv_str_compare_string(&str1, &str2), 0);
    ASSERT_GT(anv_str_compare_string(&str2, &str1), 0);
    anv_str_destroy(&str1);
    anv_str_destroy(&str2);
    return TEST_SUCCESS;
}

int test_compare_different_contents(void)
{
    ANVString str1 = anv_str_create_from_cstring("abc");
    ANVString str2 = anv_str_create_from_cstring("abd");
    ASSERT_LT(anv_str_compare_string(&str1, &str2), 0);
    ASSERT_GT(anv_str_compare_string(&str2, &str1), 0);
    anv_str_destroy(&str1);
    anv_str_destroy(&str2);
    return TEST_SUCCESS;
}

int test_find_first_of_no_match(void)
{
    ANVString str = anv_str_create_from_cstring("abcdef");
    ASSERT_EQ(anv_str_find_first_of(&str, "xyz"), STR_NPOS);
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_find_first_of_multiple_matches(void)
{
    ANVString str = anv_str_create_from_cstring("abcdef");
    ASSERT_EQ(anv_str_find_first_of(&str, "fa"), 0); // 'a' at pos 0
    ASSERT_EQ(anv_str_find_first_of(&str, "f"), 5);  // 'f' at pos 5
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_remove_extra_ws(void)
{
    ANVString str = anv_str_create_from_cstring("  a   b\t\tc  ");
    anv_str_remove_extra_ws(&str);
    ASSERT_EQ_STR(anv_str_data(&str), "a b c");
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_to_lower_upper_already(void)
{
    ANVString str1 = anv_str_create_from_cstring("abc");
    anv_str_to_lower(&str1);
    ASSERT_EQ_STR(anv_str_data(&str1), "abc");
    anv_str_to_upper(&str1);
    ASSERT_EQ_STR(anv_str_data(&str1), "ABC");
    anv_str_destroy(&str1);

    ANVString str2 = anv_str_create_from_cstring("XYZ");
    anv_str_to_upper(&str2);
    ASSERT_EQ_STR(anv_str_data(&str2), "XYZ");
    anv_str_to_lower(&str2);
    ASSERT_EQ_STR(anv_str_data(&str2), "xyz");
    anv_str_destroy(&str2);
    return TEST_SUCCESS;
}

int test_substr_create_zero_count(void)
{
    ANVString str = anv_str_create_from_cstring("abcdef");
    ANVString sub = anv_str_substr_create_string(&str, 2, 0);
    ASSERT_EQ(anv_str_size(&sub), 0);
    ASSERT_EQ_STR(anv_str_data(&sub), "");
    anv_str_destroy(&sub);
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_compare_string_equality(void)
{
    ANVString str1 = anv_str_create_from_cstring("abc");
    ANVString str2 = anv_str_create_from_cstring("abc");
    ASSERT_EQ(anv_str_compare_string(&str1, &str2), 0);
    ASSERT_EQ(anv_str_compare_string(&str2, &str1), 0);
    anv_str_destroy(&str1);
    anv_str_destroy(&str2);
    return TEST_SUCCESS;
}

int test_substr_create_cstring_cases(void)
{
    const char* src = "abcdef";
    ANVString sub1 = anv_str_substr_create_cstring(src, 0, 2);
    ASSERT_EQ_STR(anv_str_data(&sub1), "ab");
    ANVString sub2 = anv_str_substr_create_cstring(src, 4, 10); // count > length
    ASSERT_EQ_STR(anv_str_data(&sub2), "ef");
    ANVString sub3 = anv_str_substr_create_cstring(src, 10, 2); // pos > length
    ASSERT_EQ(anv_str_size(&sub3), 0);
    anv_str_destroy(&sub1);
    anv_str_destroy(&sub2);
    anv_str_destroy(&sub3);
    return TEST_SUCCESS;
}

int test_compare_cstring_prefix_suffix(void)
{
    ANVString str = anv_str_create_from_cstring("abc");
    ASSERT_LT(anv_str_compare_cstring(&str, "abcd"), 0);
    ASSERT_GT(anv_str_compare_cstring(&str, "ab"), 0);
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_getline_ch_simulated(void)
{
    // Use a temporary file on disk for portability
    FILE* fp = fopen("test_tmpfile.txt", "w+");
    ASSERT_NOT_NULL(fp);
    fputs("hello\nworld", fp);
    fflush(fp);
    rewind(fp);
    ANVString line = anv_str_create_empty(16);
    const int status = anv_str_getline_ch(fp, &line, '\n');
    ASSERT_EQ_STR(anv_str_data(&line), "hello");
    ASSERT_EQ(status, 0);
    anv_str_destroy(&line);
    fclose(fp);
    remove("test_tmpfile.txt");
    return TEST_SUCCESS;
}

int test_getline_cstring_simulated(void)
{
    FILE* fp = fopen("test_tmpfile2.txt", "w+");
    ASSERT_NOT_NULL(fp);
    fputs("foo,bar,baz", fp);
    fflush(fp);
    rewind(fp);
    ANVString line = anv_str_create_empty(16);
    const int status = anv_str_getline_cstring(fp, &line, ",");
    ASSERT_EQ_STR(anv_str_data(&line), "foo");
    ASSERT_EQ(status, 0);
    anv_str_destroy(&line);
    fclose(fp);
    remove("test_tmpfile2.txt");
    return TEST_SUCCESS;
}

int test_find_cstring_empty_search(void)
{
    ANVString str = anv_str_create_from_cstring("abc");
    ASSERT_EQ(anv_str_find_cstring(&str, ""), STR_NPOS);
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_find_string_empty_search(void)
{
    ANVString str = anv_str_create_from_cstring("abc");
    ANVString empty = anv_str_create_empty(4);
    ASSERT_EQ(anv_str_find_string(&str, &empty), STR_NPOS);
    anv_str_destroy(&str);
    anv_str_destroy(&empty);
    return TEST_SUCCESS;
}

int test_substr_create_string_count_exceeds(void)
{
    ANVString str = anv_str_create_from_cstring("abc");
    ANVString sub = anv_str_substr_create_string(&str, 1, 10);
    ASSERT_EQ_STR(anv_str_data(&sub), "bc");
    anv_str_destroy(&sub);
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_substr_create_string_pos_at_size(void)
{
    ANVString str = anv_str_create_from_cstring("abc");
    ANVString sub = anv_str_substr_create_string(&str, anv_str_size(&str), 2);
    ASSERT_EQ(anv_str_size(&sub), 0);
    ASSERT_EQ_STR(anv_str_data(&sub), "");
    anv_str_destroy(&sub);
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_substr_create_string_pos_gt_size(void)
{
    ANVString str = anv_str_create_from_cstring("abc");
    ANVString sub = anv_str_substr_create_string(&str, anv_str_size(&str) + 1, 2);
    ASSERT_EQ(anv_str_size(&sub), 0);
    ASSERT_EQ_STR(anv_str_data(&sub), "");
    anv_str_destroy(&sub);
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_substr_cstring_pos_at_length(void)
{
    const char* src = "abc";
    char buf[8] = {0};
    anv_str_substr_cstring(src, strlen(src), 2, buf);
    ASSERT_EQ_STR(buf, "");
    return TEST_SUCCESS;
}

int test_substr_cstring_pos_gt_length(void)
{
    const char* src = "abc";
    char buf[8] = {0};
    anv_str_substr_cstring(src, strlen(src) + 1, 2, buf);
    ASSERT_EQ_STR(buf, "");
    return TEST_SUCCESS;
}

int test_compare_string_empty(void)
{
    ANVString str1 = anv_str_create_empty(4);
    ANVString str2 = anv_str_create_empty(4);
    ASSERT_EQ(anv_str_compare_string(&str1, &str2), 0);
    anv_str_destroy(&str1);
    anv_str_destroy(&str2);
    return TEST_SUCCESS;
}

int test_compare_cstring_empty(void)
{
    ANVString str = anv_str_create_empty(4);
    ASSERT_EQ(anv_str_compare_cstring(&str, ""), 0);
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_getline_ch_empty_file(void)
{
    FILE* fp = fopen("test_tmpfile_empty.txt", "w+");
    ASSERT_NOT_NULL(fp);
    ANVString line = anv_str_create_empty(8);
    const int status = anv_str_getline_ch(fp, &line, '\n');
    ASSERT_EQ(status, EOF);
    ASSERT_TRUE(anv_str_empty(&line));
    anv_str_destroy(&line);
    fclose(fp);
    remove("test_tmpfile_empty.txt");
    return TEST_SUCCESS;
}

int test_getline_cstring_empty_file(void)
{
    FILE* fp = fopen("test_tmpfile_empty2.txt", "w+");
    ASSERT_NOT_NULL(fp);
    ANVString line = anv_str_create_empty(8);
    const int status = anv_str_getline_cstring(fp, &line, ",");
    ASSERT_EQ(status, EOF);
    ASSERT_TRUE(anv_str_empty(&line));
    anv_str_destroy(&line);
    fclose(fp);
    remove("test_tmpfile_empty2.txt");
    return TEST_SUCCESS;
}

int test_trim_front_already_trimmed(void)
{
    ANVString str = anv_str_create_from_cstring("abc");
    anv_str_trim_front(&str);
    ASSERT_EQ_STR(anv_str_data(&str), "abc");
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_trim_back_already_trimmed(void)
{
    ANVString str = anv_str_create_from_cstring("abc");
    anv_str_trim_back(&str);
    ASSERT_EQ_STR(anv_str_data(&str), "abc");
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_remove_extra_ws_only_spaces(void)
{
    ANVString str = anv_str_create_from_cstring("     ");
    anv_str_remove_extra_ws(&str);
    ASSERT_TRUE(anv_str_empty(&str) || anv_str_size(&str) == 0);
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_to_lower_empty(void)
{
    ANVString str = anv_str_create_empty(8);
    anv_str_to_lower(&str);
    ASSERT_TRUE(anv_str_empty(&str));
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_to_upper_empty(void)
{
    ANVString str = anv_str_create_empty(8);
    anv_str_to_upper(&str);
    ASSERT_TRUE(anv_str_empty(&str));
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_substr_create_string_zero_count_zero_pos(void)
{
    ANVString str = anv_str_create_from_cstring("abc");
    ANVString sub = anv_str_substr_create_string(&str, 0, 0);
    ASSERT_EQ(anv_str_size(&sub), 0);
    ASSERT_EQ_STR(anv_str_data(&sub), "");
    anv_str_destroy(&sub);
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_substr_create_cstring_zero_count_zero_pos(void)
{
    const char* src = "abc";
    ANVString sub = anv_str_substr_create_cstring(src, 0, 0);
    ASSERT_EQ(anv_str_size(&sub), 0);
    ASSERT_EQ_STR(anv_str_data(&sub), "");
    anv_str_destroy(&sub);
    return TEST_SUCCESS;
}

int test_substr_string_zero_count_zero_pos(void)
{
    ANVString str = anv_str_create_from_cstring("abc");
    char buf[8] = {0};
    anv_str_substr_string(&str, 0, 0, buf);
    ASSERT_EQ_STR(buf, "");
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_substr_cstring_zero_count_zero_pos(void)
{
    const char* src = "abc";
    char buf[8] = {0};
    anv_str_substr_cstring(src, 0, 0, buf);
    ASSERT_EQ_STR(buf, "");
    return TEST_SUCCESS;
}

int test_compare_string_one_empty(void)
{
    ANVString str1 = anv_str_create_empty(4);
    ANVString str2 = anv_str_create_from_cstring("abc");
    ASSERT_LT(anv_str_compare_string(&str1, &str2), 0);
    ASSERT_GT(anv_str_compare_string(&str2, &str1), 0);
    anv_str_destroy(&str1);
    anv_str_destroy(&str2);
    return TEST_SUCCESS;
}

int test_compare_cstring_one_empty(void)
{
    ANVString str = anv_str_create_empty(4);
    ASSERT_LT(anv_str_compare_cstring(&str, "abc"), 0);
    ANVString str2 = anv_str_create_from_cstring("abc");
    ASSERT_GT(anv_str_compare_cstring(&str2, ""), 0);
    anv_str_destroy(&str);
    anv_str_destroy(&str2);
    return TEST_SUCCESS;
}

int test_getline_ch_delim_not_present(void)
{
    FILE* fp = fopen("test_tmpfile3.txt", "w+");
    ASSERT_NOT_NULL(fp);
    fputs("abcdef", fp);
    fflush(fp);
    rewind(fp);
    ANVString line = anv_str_create_empty(8);
    const int status = anv_str_getline_ch(fp, &line, ';'); // Delimiter not present
    ASSERT_EQ_STR(anv_str_data(&line), "abcdef");
    ASSERT_EQ(status, EOF);
    anv_str_destroy(&line);
    fclose(fp);
    remove("test_tmpfile3.txt");
    return TEST_SUCCESS;
}

int test_getline_cstring_delim_not_present(void)
{
    FILE* fp = fopen("test_tmpfile4.txt", "w+");
    ASSERT_NOT_NULL(fp);
    fputs("abcdef", fp);
    fflush(fp);
    rewind(fp);
    ANVString line = anv_str_create_empty(8);
    const int status = anv_str_getline_cstring(fp, &line, ";"); // Delimiter not present
    ASSERT_EQ_STR(anv_str_data(&line), "abcdef");
    ASSERT_EQ(status, EOF);
    anv_str_destroy(&line);
    fclose(fp);
    remove("test_tmpfile4.txt");
    return TEST_SUCCESS;
}

int test_trim_front_back_only_ws(void)
{
    ANVString str = anv_str_create_from_cstring("   \t\n  ");
    anv_str_trim_front(&str);
    ASSERT_TRUE(anv_str_empty(&str) || anv_str_size(&str) == 0);
    anv_str_assign_cstring(&str, "   \t\n  ");
    anv_str_trim_back(&str);
    ASSERT_TRUE(anv_str_empty(&str) || anv_str_size(&str) == 0);
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_remove_extra_ws_tabs_newlines(void)
{
    ANVString str = anv_str_create_from_cstring("\t\t\n\n\t");
    anv_str_remove_extra_ws(&str);
    ASSERT_TRUE(anv_str_empty(&str) || anv_str_size(&str) == 0);
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_to_lower_upper_mixed(void)
{
    ANVString str = anv_str_create_from_cstring("AbC123xYz");
    anv_str_to_lower(&str);
    ASSERT_EQ_STR(anv_str_data(&str), "abc123xyz");
    anv_str_to_upper(&str);
    ASSERT_EQ_STR(anv_str_data(&str), "ABC123XYZ");
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_find_first_of_empty_value(void)
{
    ANVString str = anv_str_create_from_cstring("abc");
    ASSERT_EQ(anv_str_find_first_of(&str, ""), STR_NPOS);
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_find_cstring_at_end(void)
{
    ANVString str = anv_str_create_from_cstring("abcdef");
    ASSERT_EQ(anv_str_find_cstring(&str, "ef"), 4);
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_find_string_at_end(void)
{
    ANVString str = anv_str_create_from_cstring("abcdef");
    ANVString find = anv_str_create_from_cstring("ef");
    ASSERT_EQ(anv_str_find_string(&str, &find), 4);
    anv_str_destroy(&str);
    anv_str_destroy(&find);
    return TEST_SUCCESS;
}

int test_substr_create_string_count_0_pos_end(void)
{
    ANVString str = anv_str_create_from_cstring("abc");
    ANVString sub = anv_str_substr_create_string(&str, anv_str_size(&str), 0);
    ASSERT_EQ(anv_str_size(&sub), 0);
    ASSERT_EQ_STR(anv_str_data(&sub), "");
    anv_str_destroy(&sub);
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_substr_create_cstring_count_0_pos_end(void)
{
    const char* src = "abc";
    ANVString sub = anv_str_substr_create_cstring(src, strlen(src), 0);
    ASSERT_EQ(anv_str_size(&sub), 0);
    ASSERT_EQ_STR(anv_str_data(&sub), "");
    anv_str_destroy(&sub);
    return TEST_SUCCESS;
}

int test_null_pointer_handling(void)
{
    // Should not crash, should return error or handle gracefully
    ASSERT_EQ(anv_str_empty(nullptr), true);
    ASSERT_EQ(anv_str_size(nullptr), 0);
    ASSERT_EQ(anv_str_capacity(nullptr), 0);
    ASSERT_EQ_PTR(anv_str_data(nullptr), NULL);
    // Add more NULL pointer checks for other API functions as needed
    return TEST_SUCCESS;
}

int test_invalid_values(void)
{
    ANVString str = anv_str_create_empty(8);
    anv_str_insert_char(&str, (size_t)-1, 'X'); // Out-of-bounds
    ASSERT_EQ_STR(anv_str_data(&str), "");
    anv_str_erase(&str, (size_t)-1); // Out-of-bounds
    ASSERT_TRUE(anv_str_empty(&str));
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_str_free_split_basic(void)
{
    const size_t count = 3;
    ANVString* arr = (ANVString*)malloc(sizeof(ANVString) * count);
    arr[0] = anv_str_create_from_cstring("one");
    arr[1] = anv_str_create_from_cstring("two");
    arr[2] = anv_str_create_from_cstring("three");
    anv_str_destroy_split(&arr, count);
    ASSERT_NULL(arr); // arr should be NULL after free
    return TEST_SUCCESS;
}

int test_str_free_split_nullptr(void)
{
    anv_str_destroy_split(nullptr, 3); // Should not crash
    return TEST_SUCCESS;
}

int test_str_free_split_zero_count(void)
{
    ANVString* arr = (ANVString*)malloc(sizeof(ANVString) * 2);
    arr[0] = anv_str_create_from_cstring("a");
    arr[1] = anv_str_create_from_cstring("b");
    anv_str_destroy_split(&arr, 0); // Should only free the array pointer
    ASSERT_NULL(arr);
    return TEST_SUCCESS;
}

int test_str_split_basic(void)
{
    ANVString str = anv_str_create_from_cstring("a,b,c");
    ANVString* out = nullptr;
    const size_t count = anv_str_split(&str, ",", &out);
    ASSERT_EQ(count, 3);
    ASSERT_EQ_STR(anv_str_data(&out[0]), "a");
    ASSERT_EQ_STR(anv_str_data(&out[1]), "b");
    ASSERT_EQ_STR(anv_str_data(&out[2]), "c");
    anv_str_destroy_split(&out, count);
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_str_split_no_delim(void)
{
    ANVString str = anv_str_create_from_cstring("abc");
    ANVString* out = nullptr;
    const size_t count = anv_str_split(&str, ";", &out);
    ASSERT_EQ(count, 1);
    ASSERT_EQ_STR(anv_str_data(&out[0]), "abc");
    anv_str_destroy_split(&out, count);
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_str_split_empty_string(void)
{
    ANVString str = anv_str_create_empty(8);
    ANVString* out = nullptr;
    const size_t count = anv_str_split(&str, ",", &out);
    ASSERT_EQ(count, 0);
    ASSERT_NULL(out);
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

int test_str_split_nullptr(void)
{
    const size_t count = anv_str_split(nullptr, ",", nullptr);
    ASSERT_EQ(count, 0);
    return TEST_SUCCESS;
}

int test_str_split_and_free_split(void)
{
    ANVString str = anv_str_create_from_cstring("alpha,beta,gamma,delta");
    ANVString* out = nullptr;
    const size_t count = anv_str_split(&str, ",", &out);
    ASSERT_EQ(count, 4);
    ASSERT_EQ_STR(anv_str_data(&out[0]), "alpha");
    ASSERT_EQ_STR(anv_str_data(&out[1]), "beta");
    ASSERT_EQ_STR(anv_str_data(&out[2]), "gamma");
    ASSERT_EQ_STR(anv_str_data(&out[3]), "delta");
    anv_str_destroy_split(&out, count);
    ASSERT_NULL(out);
    anv_str_destroy(&str);
    return TEST_SUCCESS;
}

typedef struct
{
    int (*func)(void);
    const char* name;
} TestCase;

TestCase tests[] = {
    {test_find_and_compare, "test_find_and_compare"},
    {test_trim_and_case, "test_trim_and_case"},
    {test_substr, "test_substr"},
    {test_substr_out_of_bounds, "test_substr_out_of_bounds"},
    {test_embedded_null, "test_embedded_null"},
    {test_trim_all_whitespace, "test_trim_all_whitespace"},
    {test_compare_different_lengths, "test_compare_different_lengths"},
    {test_compare_different_contents, "test_compare_different_contents"},
    {test_find_first_of_no_match, "test_find_first_of_no_match"},
    {test_find_first_of_multiple_matches, "test_find_first_of_multiple_matches"},
    {test_remove_extra_ws, "test_remove_extra_ws"},
    {test_to_lower_upper_already, "test_to_lower_upper_already"},
    {test_substr_create_zero_count, "test_substr_create_zero_count"},
    {test_compare_string_equality, "test_compare_string_equality"},
    {test_substr_create_cstring_cases, "test_substr_create_cstring_cases"},
    {test_compare_cstring_prefix_suffix, "test_compare_cstring_prefix_suffix"},
    {test_getline_ch_simulated, "test_getline_ch_simulated"},
    {test_getline_cstring_simulated, "test_getline_cstring_simulated"},
    {test_find_cstring_empty_search, "test_find_cstring_empty_search"},
    {test_find_string_empty_search, "test_find_string_empty_search"},
    {test_substr_create_string_count_exceeds, "test_substr_create_string_count_exceeds"},
    {test_substr_create_string_pos_at_size, "test_substr_create_string_pos_at_size"},
    {test_substr_create_string_pos_gt_size, "test_substr_create_string_pos_gt_size"},
    {test_substr_cstring_pos_at_length, "test_substr_cstring_pos_at_length"},
    {test_substr_cstring_pos_gt_length, "test_substr_cstring_pos_gt_length"},
    {test_compare_string_empty, "test_compare_string_empty"},
    {test_compare_cstring_empty, "test_compare_cstring_empty"},
    {test_getline_ch_empty_file, "test_getline_ch_empty_file"},
    {test_getline_cstring_empty_file, "test_getline_cstring_empty_file"},
    {test_trim_front_already_trimmed, "test_trim_front_already_trimmed"},
    {test_trim_back_already_trimmed, "test_trim_back_already_trimmed"},
    {test_remove_extra_ws_only_spaces, "test_remove_extra_ws_only_spaces"},
    {test_to_lower_empty, "test_to_lower_empty"},
    {test_to_upper_empty, "test_to_upper_empty"},
    {test_substr_create_string_zero_count_zero_pos, "test_substr_create_string_zero_count_zero_pos"},
    {test_substr_create_cstring_zero_count_zero_pos, "test_substr_create_cstring_zero_count_zero_pos"},
    {test_substr_string_zero_count_zero_pos, "test_substr_string_zero_count_zero_pos"},
    {test_substr_cstring_zero_count_zero_pos, "test_substr_cstring_zero_count_zero_pos"},
    {test_compare_string_one_empty, "test_compare_string_one_empty"},
    {test_compare_cstring_one_empty, "test_compare_cstring_one_empty"},
    {test_getline_ch_delim_not_present, "test_getline_ch_delim_not_present"},
    {test_getline_cstring_delim_not_present, "test_getline_cstring_delim_not_present"},
    {test_trim_front_back_only_ws, "test_trim_front_back_only_ws"},
    {test_remove_extra_ws_tabs_newlines, "test_remove_extra_ws_tabs_newlines"},
    {test_to_lower_upper_mixed, "test_to_lower_upper_mixed"},
    {test_find_first_of_empty_value, "test_find_first_of_empty_value"},
    {test_find_cstring_at_end, "test_find_cstring_at_end"},
    {test_find_string_at_end, "test_find_string_at_end"},
    {test_substr_create_string_count_0_pos_end, "test_substr_create_string_count_0_pos_end"},
    {test_substr_create_cstring_count_0_pos_end, "test_substr_create_cstring_count_0_pos_end"},
    {test_null_pointer_handling, "test_null_pointer_handling"},
    {test_invalid_values, "test_invalid_values"},
    {test_str_free_split_basic, "test_str_free_split_basic"},
    {test_str_free_split_nullptr, "test_str_free_split_nullptr"},
    {test_str_free_split_zero_count, "test_str_free_split_zero_count"},
    {test_str_split_basic, "test_str_split_basic"},
    {test_str_split_no_delim, "test_str_split_no_delim"},
    {test_str_split_empty_string, "test_str_split_empty_string"},
    {test_str_split_nullptr, "test_str_split_nullptr"},
    {test_str_split_and_free_split, "test_str_split_and_free_split"},
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
        printf("All DString Algorithm tests passed.\n");
        return 0;
    }

    printf("%d DString Algorithm tests failed.\n", failed);
    return 1;
}