#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include <anvil/testing.h>
#include "TestAssert.h"

#ifdef ANV_PLATFORM_WINDOWS
#include <io.h>
#include <process.h>
#else
#include <unistd.h>
#endif

//==============================================================================
// Fuzz Helpers
//==============================================================================

#define NUM_FUZZ_OPERATIONS 100000

static void fuzz_seed(void)
{
    srand((unsigned int)time(NULL));
}

static unsigned fuzz_rand_range(const unsigned bound)
{
    if (bound == 0)
        return 0u;
    return rand() % bound;
}

static size_t fuzz_rand_size(const size_t bound)
{
    if (bound == 0)
        return 0;
    return rand() % bound;
}

static void ensure_seeded(void)
{
    static int seeded = 0;
    if (!seeded)
    {
        fuzz_seed();
        seeded = 1;
    }
}

static void perform_random_operation(ANVString* str)
{
    const unsigned op = fuzz_rand_range(8);

    switch (op)
    {
        case 0:                                                       // push_back
            anv_str_push_back(str, (char)(fuzz_rand_range(95) + 32)); // Printable ASCII
            break;
        case 1: // pop_back
            if (!anv_str_empty(str))
            {
                anv_str_pop_back(str);
            }
            break;
        case 2: // insert
            if (anv_str_size(str) > 0)
            {
                anv_str_insert_char(str, fuzz_rand_size(anv_str_size(str)), 'X');
            }
            break;
        case 3: // erase
            if (anv_str_size(str) > 0)
            {
                anv_str_erase(str, fuzz_rand_size(anv_str_size(str)));
            }
            break;
        case 4: // assign
            anv_str_assign_cstring(str, "fuzz");
            break;
        case 5: // clear
            anv_str_clear(str);
            break;
        case 6: // trim
            anv_str_trim_front(str);
            anv_str_trim_back(str);
            break;
        case 7: // reserve
            anv_str_reserve(str, (size_t)fuzz_rand_range(256));
            break;

        default:
            break;
    }
}

//==============================================================================
// Algorithm Tests
//==============================================================================

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
    ANVString* arr = malloc(sizeof(ANVString) * count);
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
    ANVString* arr = malloc(sizeof(ANVString) * 2);
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

//==============================================================================
// CRUD Tests
//==============================================================================

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

//==============================================================================
// Fuzz Tests
//==============================================================================

int test_string_fuzz(void)
{
    ensure_seeded();
    ANVString str = anv_str_create_empty(0);

    for (int i = 0; i < NUM_FUZZ_OPERATIONS; ++i)
    {
        perform_random_operation(&str);
    }

    anv_str_destroy(&str);
    printf("DString fuzz test completed %d operations without crashing.\n", NUM_FUZZ_OPERATIONS);
    return TEST_SUCCESS;
}

//==============================================================================
// Memory Tests
//==============================================================================

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

//==============================================================================
// Properties Tests
//==============================================================================

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

//==============================================================================
// Main
//==============================================================================

int main(void)
{
    const ANVTestCase tests[] = {
        // Algorithm tests
        TEST_REGISTER(test_find_and_compare),
        TEST_REGISTER(test_trim_and_case),
        TEST_REGISTER(test_substr),
        TEST_REGISTER(test_substr_out_of_bounds),
        TEST_REGISTER(test_embedded_null),
        TEST_REGISTER(test_trim_all_whitespace),
        TEST_REGISTER(test_compare_different_lengths),
        TEST_REGISTER(test_compare_different_contents),
        TEST_REGISTER(test_find_first_of_no_match),
        TEST_REGISTER(test_find_first_of_multiple_matches),
        TEST_REGISTER(test_remove_extra_ws),
        TEST_REGISTER(test_to_lower_upper_already),
        TEST_REGISTER(test_substr_create_zero_count),
        TEST_REGISTER(test_compare_string_equality),
        TEST_REGISTER(test_substr_create_cstring_cases),
        TEST_REGISTER(test_compare_cstring_prefix_suffix),
        TEST_REGISTER(test_getline_ch_simulated),
        TEST_REGISTER(test_getline_cstring_simulated),
        TEST_REGISTER(test_find_cstring_empty_search),
        TEST_REGISTER(test_find_string_empty_search),
        TEST_REGISTER(test_substr_create_string_count_exceeds),
        TEST_REGISTER(test_substr_create_string_pos_at_size),
        TEST_REGISTER(test_substr_create_string_pos_gt_size),
        TEST_REGISTER(test_substr_cstring_pos_at_length),
        TEST_REGISTER(test_substr_cstring_pos_gt_length),
        TEST_REGISTER(test_compare_string_empty),
        TEST_REGISTER(test_compare_cstring_empty),
        TEST_REGISTER(test_getline_ch_empty_file),
        TEST_REGISTER(test_getline_cstring_empty_file),
        TEST_REGISTER(test_trim_front_already_trimmed),
        TEST_REGISTER(test_trim_back_already_trimmed),
        TEST_REGISTER(test_remove_extra_ws_only_spaces),
        TEST_REGISTER(test_to_lower_empty),
        TEST_REGISTER(test_to_upper_empty),
        TEST_REGISTER(test_substr_create_string_zero_count_zero_pos),
        TEST_REGISTER(test_substr_create_cstring_zero_count_zero_pos),
        TEST_REGISTER(test_substr_string_zero_count_zero_pos),
        TEST_REGISTER(test_substr_cstring_zero_count_zero_pos),
        TEST_REGISTER(test_compare_string_one_empty),
        TEST_REGISTER(test_compare_cstring_one_empty),
        TEST_REGISTER(test_getline_ch_delim_not_present),
        TEST_REGISTER(test_getline_cstring_delim_not_present),
        TEST_REGISTER(test_trim_front_back_only_ws),
        TEST_REGISTER(test_remove_extra_ws_tabs_newlines),
        TEST_REGISTER(test_to_lower_upper_mixed),
        TEST_REGISTER(test_find_first_of_empty_value),
        TEST_REGISTER(test_find_cstring_at_end),
        TEST_REGISTER(test_find_string_at_end),
        TEST_REGISTER(test_substr_create_string_count_0_pos_end),
        TEST_REGISTER(test_substr_create_cstring_count_0_pos_end),
        TEST_REGISTER(test_null_pointer_handling),
        TEST_REGISTER(test_invalid_values),
        TEST_REGISTER(test_str_free_split_basic),
        TEST_REGISTER(test_str_free_split_nullptr),
        TEST_REGISTER(test_str_free_split_zero_count),
        TEST_REGISTER(test_str_split_basic),
        TEST_REGISTER(test_str_split_no_delim),
        TEST_REGISTER(test_str_split_empty_string),
        TEST_REGISTER(test_str_split_nullptr),
        TEST_REGISTER(test_str_split_and_free_split),

        // CRUD tests
        TEST_REGISTER(test_create_and_assign),
        TEST_REGISTER(test_append_and_insert),
        TEST_REGISTER(test_push_pop_erase),
        TEST_REGISTER(test_empty_string),
        TEST_REGISTER(test_assign_empty_cstring),
        TEST_REGISTER(test_append_empty_cstring),
        TEST_REGISTER(test_insert_at_bounds),
        TEST_REGISTER(test_erase_out_of_bounds),
        TEST_REGISTER(test_self_assign_and_append),
        TEST_REGISTER(test_clear_non_empty),
        TEST_REGISTER(test_pop_back_empty),
        TEST_REGISTER(test_erase_empty),
        TEST_REGISTER(test_assign_char),
        TEST_REGISTER(test_insert_char_positions),
        TEST_REGISTER(test_append_char_multiple),
        TEST_REGISTER(test_clear_already_empty),
        TEST_REGISTER(test_assign_string_different),
        TEST_REGISTER(test_insert_cstring_empty),
        TEST_REGISTER(test_insert_string_empty),
        TEST_REGISTER(test_append_string_empty),
        TEST_REGISTER(test_push_back_null_char),
        TEST_REGISTER(test_append_char_null_char),
        TEST_REGISTER(test_insert_char_out_of_bounds),
        TEST_REGISTER(test_insert_cstring_out_of_bounds),
        TEST_REGISTER(test_insert_string_out_of_bounds),
        TEST_REGISTER(test_erase_at_size),
        TEST_REGISTER(test_assign_string_self),
        TEST_REGISTER(test_append_string_self),
        TEST_REGISTER(test_insert_char_at_0_and_size),
        TEST_REGISTER(test_insert_cstring_at_0_and_size),
        TEST_REGISTER(test_insert_string_at_0_and_size),

        // Fuzz tests
        TEST_REGISTER(test_string_fuzz),

        // Memory tests
        TEST_REGISTER(test_reserve_and_shrink),
        TEST_REGISTER(test_buffer_growth),
        TEST_REGISTER(test_large_string),
        TEST_REGISTER(test_reserve_and_shrink_optimal),

        // Property tests
        TEST_REGISTER(test_string_size_and_capacity),
        TEST_REGISTER(test_string_idempotent_trim),
        TEST_REGISTER(test_string_case_conversion_reversibility),
    };
    return anv_run_tests("DString", tests, sizeof(tests) / sizeof(tests[0]));
}