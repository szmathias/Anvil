#include <string.h>

#include <anvil/testing.h>
#include "common/result.h"

//==============================================================================
// Basic Tests
//==============================================================================

int test_result_success_string(void)
{
    const char* str = anv_result_to_string(ANV_RESULT_SUCCESS);
    ASSERT_NOT_NULL(str);
    ASSERT_EQ_STR(str, "Success");
    return TEST_SUCCESS;
}

int test_result_all_codes_have_strings(void)
{
    // Every valid result code should return a non-NULL, non-empty string
    for (int i = 0; i < ANV_RESULT_COUNT; i++)
    {
        const char* str = anv_result_to_string((ANVResult)i);
        ASSERT_NOT_NULL(str);
        ASSERT_GT(strlen(str), 0);
    }
    return TEST_SUCCESS;
}

int test_result_known_codes(void)
{
    ASSERT_EQ_STR(anv_result_to_string(ANV_RESULT_SUCCESS), "Success");
    ASSERT_EQ_STR(anv_result_to_string(ANV_RESULT_NULL_POINTER), "Null pointer");
    ASSERT_EQ_STR(anv_result_to_string(ANV_RESULT_INVALID_ARGUMENT), "Invalid argument");
    ASSERT_EQ_STR(anv_result_to_string(ANV_RESULT_OUT_OF_BOUNDS), "Index out of bounds");
    ASSERT_EQ_STR(anv_result_to_string(ANV_RESULT_OUT_OF_MEMORY), "Out of memory");
    ASSERT_EQ_STR(anv_result_to_string(ANV_RESULT_INSUFFICIENT_SPACE), "Insufficient space");
    ASSERT_EQ_STR(anv_result_to_string(ANV_RESULT_NOT_FOUND), "Not found");
    ASSERT_EQ_STR(anv_result_to_string(ANV_RESULT_ALREADY_EXISTS), "Already exists");
    ASSERT_EQ_STR(anv_result_to_string(ANV_RESULT_INVALID_STATE), "Invalid state");
    ASSERT_EQ_STR(anv_result_to_string(ANV_RESULT_NOT_IMPLEMENTED), "Not implemented");
    return TEST_SUCCESS;
}

int test_result_out_of_range(void)
{
    // Values outside the valid range should return a fallback string
    const char* str = anv_result_to_string((ANVResult)ANV_RESULT_COUNT);
    ASSERT_NOT_NULL(str);
    ASSERT_EQ_STR(str, "Unknown result code");

    str = anv_result_to_string((ANVResult)(ANV_RESULT_COUNT + 100));
    ASSERT_NOT_NULL(str);
    ASSERT_EQ_STR(str, "Unknown result code");

    str = anv_result_to_string((ANVResult)(-1));
    ASSERT_NOT_NULL(str);
    ASSERT_EQ_STR(str, "Unknown result code");
    return TEST_SUCCESS;
}

int test_result_macros(void)
{
    ASSERT_TRUE(ANV_SUCCEEDED(ANV_RESULT_SUCCESS));
    ASSERT_FALSE(ANV_FAILED(ANV_RESULT_SUCCESS));

    ASSERT_TRUE(ANV_FAILED(ANV_RESULT_NULL_POINTER));
    ASSERT_FALSE(ANV_SUCCEEDED(ANV_RESULT_NULL_POINTER));

    ASSERT_TRUE(ANV_FAILED(ANV_RESULT_OUT_OF_MEMORY));
    ASSERT_TRUE(ANV_FAILED(ANV_RESULT_NOT_FOUND));
    ASSERT_TRUE(ANV_FAILED(ANV_RESULT_INVALID_ARGUMENT));
    return TEST_SUCCESS;
}

int test_result_consistency(void)
{
    // Calling the same code twice should return the same pointer
    const char* str1 = anv_result_to_string(ANV_RESULT_SUCCESS);
    const char* str2 = anv_result_to_string(ANV_RESULT_SUCCESS);
    ASSERT_EQ_PTR(str1, str2);
    return TEST_SUCCESS;
}

//==============================================================================
// Main
//==============================================================================

int main(void)
{
    const ANVTestCase tests[] = {
        TEST_REGISTER(test_result_success_string),
        TEST_REGISTER(test_result_all_codes_have_strings),
        TEST_REGISTER(test_result_known_codes),
        TEST_REGISTER(test_result_out_of_range),
        TEST_REGISTER(test_result_macros),
        TEST_REGISTER(test_result_consistency),
    };

    return anv_run_tests("Result", tests, sizeof(tests) / sizeof(tests[0]));
}

