#include <string.h>

#include <anvil/testing.h>
#include "common/version.h"

//==============================================================================
// Basic Tests
//==============================================================================

int test_version_get(void)
{
    ANVVersionInfo info = anv_version_get();
    ASSERT_GTE(info.major, 0);
    ASSERT_GTE(info.minor, 0);
    ASSERT_GTE(info.patch, 0);
    ASSERT_NOT_NULL(info.string);
    ASSERT_NOT_NULL(info.build_date);
    ASSERT_NOT_NULL(info.build_time);
    return TEST_SUCCESS;
}

int test_version_string(void)
{
    const char* str = anv_version_string();
    ASSERT_NOT_NULL(str);
    ASSERT_GT(strlen(str), 0);

    // Should contain at least two dots (major.minor.patch)
    int dots = 0;
    for (size_t i = 0; i < strlen(str); i++)
    {
        if (str[i] == '.')
            dots++;
    }
    ASSERT_EQ(dots, 2);
    return TEST_SUCCESS;
}

int test_version_build_date(void)
{
    const char* date = anv_version_build_date();
    ASSERT_NOT_NULL(date);
    ASSERT_GT(strlen(date), 0);
    // __DATE__ format is "MMM DD YYYY" (11 characters)
    ASSERT_EQ(strlen(date), 11);
    return TEST_SUCCESS;
}

int test_version_build_time(void)
{
    const char* time_str = anv_version_build_time();
    ASSERT_NOT_NULL(time_str);
    ASSERT_GT(strlen(time_str), 0);
    // __TIME__ format is "HH:MM:SS" (8 characters)
    ASSERT_EQ(strlen(time_str), 8);
    return TEST_SUCCESS;
}

int test_version_compatible_current(void)
{
    ANVVersionInfo info = anv_version_get();
    // Current version should be compatible with itself
    ASSERT_TRUE(anv_version_compatible(info.major, info.minor));
    return TEST_SUCCESS;
}

int test_version_compatible_older(void)
{
    ANVVersionInfo info = anv_version_get();
    // Should be compatible with older minor versions
    if (info.minor > 0)
    {
        ASSERT_TRUE(anv_version_compatible(info.major, info.minor - 1));
    }
    // Should be compatible with older major versions
    if (info.major > 0)
    {
        ASSERT_TRUE(anv_version_compatible(info.major - 1, 0));
    }
    return TEST_SUCCESS;
}

int test_version_compatible_newer_minor(void)
{
    ANVVersionInfo info = anv_version_get();
    // Should NOT be compatible with a newer minor version (same major)
    ASSERT_FALSE(anv_version_compatible(info.major, info.minor + 1));
    return TEST_SUCCESS;
}

int test_version_compatible_newer_major(void)
{
    ANVVersionInfo info = anv_version_get();
    // Should NOT be compatible with a newer major version
    ASSERT_FALSE(anv_version_compatible(info.major + 1, 0));
    return TEST_SUCCESS;
}

int test_version_info_matches_string(void)
{
    ANVVersionInfo info = anv_version_get();
    const char* str = anv_version_string();
    ASSERT_EQ_STR(info.string, str);
    return TEST_SUCCESS;
}

int test_version_compatible_zero(void)
{
    // Should always be compatible with version 0.0
    ASSERT_TRUE(anv_version_compatible(0, 0));
    return TEST_SUCCESS;
}

//==============================================================================
// Main
//==============================================================================

int main(void)
{
    const ANVTestCase tests[] = {
        TEST_REGISTER(test_version_get),
        TEST_REGISTER(test_version_string),
        TEST_REGISTER(test_version_build_date),
        TEST_REGISTER(test_version_build_time),
        TEST_REGISTER(test_version_compatible_current),
        TEST_REGISTER(test_version_compatible_older),
        TEST_REGISTER(test_version_compatible_newer_minor),
        TEST_REGISTER(test_version_compatible_newer_major),
        TEST_REGISTER(test_version_info_matches_string),
        TEST_REGISTER(test_version_compatible_zero),
    };

    return anv_run_tests("Version", tests, sizeof(tests) / sizeof(tests[0]));
}

