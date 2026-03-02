//
// Tests for hash functions (algorithms/hash.h)
//

#include <stdio.h>
#include <string.h>
#include "algorithms/hash.h"
#include "TestAssert.h"
#include "TestRunner.h"

int test_hash_string_basic(void)
{
    const char* str = "hello";
    size_t hash = anv_hash_string(str);
    ASSERT_NOT_EQ(hash, 0);

    // Same input should produce same hash
    size_t hash2 = anv_hash_string(str);
    ASSERT_EQ(hash, hash2);

    return TEST_SUCCESS;
}

int test_hash_string_different_inputs(void)
{
    size_t h1 = anv_hash_string("hello");
    size_t h2 = anv_hash_string("world");

    // Different strings should (almost certainly) produce different hashes
    ASSERT_NOT_EQ(h1, h2);

    return TEST_SUCCESS;
}

int test_hash_string_empty(void)
{
    // Empty string should produce a consistent hash (the seed value 5381)
    size_t h1 = anv_hash_string("");
    size_t h2 = anv_hash_string("");
    ASSERT_EQ(h1, h2);

    return TEST_SUCCESS;
}

int test_hash_string_null(void)
{
    size_t hash = anv_hash_string(NULL);
    ASSERT_EQ(hash, 0);

    return TEST_SUCCESS;
}

int test_hash_string_similar(void)
{
    // Similar strings should produce different hashes
    size_t h1 = anv_hash_string("abc");
    size_t h2 = anv_hash_string("abd");
    ASSERT_NOT_EQ(h1, h2);

    size_t h3 = anv_hash_string("ab");
    ASSERT_NOT_EQ(h1, h3);

    return TEST_SUCCESS;
}

int test_hash_string_distribution(void)
{
    // Check that hashes are spread across different values
    const char* strings[] = {
        "apple", "banana", "cherry", "date", "elderberry",
        "fig", "grape", "honeydew", "kiwi", "lemon"
    };
    const int count = 10;

    size_t hashes[10];
    for (int i = 0; i < count; i++)
    {
        hashes[i] = anv_hash_string(strings[i]);
    }

    // No two hashes should be the same (extremely unlikely for different strings)
    for (int i = 0; i < count; i++)
    {
        for (int j = i + 1; j < count; j++)
        {
            ASSERT_NOT_EQ(hashes[i], hashes[j]);
        }
    }

    return TEST_SUCCESS;
}

int test_hash_int_basic(void)
{
    int val = 42;
    size_t hash = anv_hash_int(&val);
    ASSERT_NOT_EQ(hash, 0);

    // Same value should produce same hash
    size_t hash2 = anv_hash_int(&val);
    ASSERT_EQ(hash, hash2);

    return TEST_SUCCESS;
}

int test_hash_int_different_values(void)
{
    int a = 1, b = 2;
    size_t h1 = anv_hash_int(&a);
    size_t h2 = anv_hash_int(&b);
    ASSERT_NOT_EQ(h1, h2);

    return TEST_SUCCESS;
}

int test_hash_int_zero(void)
{
    int zero = 0;
    size_t hash = anv_hash_int(&zero);
    // Hash of 0 is allowed to be 0 - just check consistency
    size_t hash2 = anv_hash_int(&zero);
    ASSERT_EQ(hash, hash2);

    return TEST_SUCCESS;
}

int test_hash_int_negative(void)
{
    int neg = -42;
    size_t hash = anv_hash_int(&neg);

    int pos = 42;
    size_t hash_pos = anv_hash_int(&pos);

    // Negative and positive should differ
    ASSERT_NOT_EQ(hash, hash_pos);

    return TEST_SUCCESS;
}

int test_hash_int_null(void)
{
    size_t hash = anv_hash_int(NULL);
    ASSERT_EQ(hash, 0);

    return TEST_SUCCESS;
}

int test_hash_int_distribution(void)
{
    // Sequential integers should produce well-distributed hashes
    const int count = 10;
    size_t hashes[10];

    for (int i = 0; i < count; i++)
    {
        hashes[i] = anv_hash_int(&i);
    }

    // No two consecutive integers should hash to the same value
    for (int i = 0; i < count; i++)
    {
        for (int j = i + 1; j < count; j++)
        {
            ASSERT_NOT_EQ(hashes[i], hashes[j]);
        }
    }

    return TEST_SUCCESS;
}

int test_hash_pointer_basic(void)
{
    int a = 0, b = 0;
    size_t h1 = anv_hash_pointer(&a);
    size_t h2 = anv_hash_pointer(&b);

    // Different addresses should produce different hashes
    ASSERT_NOT_EQ(h1, h2);

    // Same address should produce same hash
    size_t h1_again = anv_hash_pointer(&a);
    ASSERT_EQ(h1, h1_again);

    return TEST_SUCCESS;
}

int test_hash_pointer_null(void)
{
    // NULL pointer should produce a consistent hash
    size_t h1 = anv_hash_pointer(NULL);
    size_t h2 = anv_hash_pointer(NULL);
    ASSERT_EQ(h1, h2);

    return TEST_SUCCESS;
}

int main(void)
{
    const ANVTestCase tests[] = {
        {test_hash_string_basic, "test_hash_string_basic"},
        {test_hash_string_different_inputs, "test_hash_string_different_inputs"},
        {test_hash_string_empty, "test_hash_string_empty"},
        {test_hash_string_null, "test_hash_string_null"},
        {test_hash_string_similar, "test_hash_string_similar"},
        {test_hash_string_distribution, "test_hash_string_distribution"},
        {test_hash_int_basic, "test_hash_int_basic"},
        {test_hash_int_different_values, "test_hash_int_different_values"},
        {test_hash_int_zero, "test_hash_int_zero"},
        {test_hash_int_negative, "test_hash_int_negative"},
        {test_hash_int_null, "test_hash_int_null"},
        {test_hash_int_distribution, "test_hash_int_distribution"},
        {test_hash_pointer_basic, "test_hash_pointer_basic"},
        {test_hash_pointer_null, "test_hash_pointer_null"},
    };

    return anv_run_tests("Hash Functions", tests, sizeof(tests) / sizeof(tests[0]));
}
