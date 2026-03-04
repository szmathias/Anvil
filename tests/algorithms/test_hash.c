#include <stdio.h>
#include <string.h>

#include <anvil/testing.h>
#include "algorithms/hash.h"

//==============================================================================
// String Hash Tests
//==============================================================================

int test_hash_string_basic(void)
{
    const char* str = "hello";
    const size_t hash = anv_hash_string(str);
    ASSERT_NOT_EQ(hash, 0);

    // Same input should produce same hash
    const size_t hash2 = anv_hash_string(str);
    ASSERT_EQ(hash, hash2);

    return TEST_SUCCESS;
}

int test_hash_string_different_inputs(void)
{
    const size_t h1 = anv_hash_string("hello");
    const size_t h2 = anv_hash_string("world");

    // Different strings should (almost certainly) produce different hashes
    ASSERT_NOT_EQ(h1, h2);

    return TEST_SUCCESS;
}

int test_hash_string_empty(void)
{
    // Empty string should produce a consistent hash (the seed value 5381)
    const size_t h1 = anv_hash_string("");
    const size_t h2 = anv_hash_string("");
    ASSERT_EQ(h1, h2);

    return TEST_SUCCESS;
}

int test_hash_string_null(void)
{
    const size_t hash = anv_hash_string(NULL);
    ASSERT_EQ(hash, 0);

    return TEST_SUCCESS;
}

int test_hash_string_similar(void)
{
    // Similar strings should produce different hashes
    const size_t h1 = anv_hash_string("abc");
    const size_t h2 = anv_hash_string("abd");
    ASSERT_NOT_EQ(h1, h2);

    const size_t h3 = anv_hash_string("ab");
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

//==============================================================================
// Integer Hash Tests
//==============================================================================

int test_hash_int_basic(void)
{
    const int val = 42;
    const size_t hash = anv_hash_int(&val);
    ASSERT_NOT_EQ(hash, 0);

    const size_t hash2 = anv_hash_int(&val);
    ASSERT_EQ(hash, hash2);

    return TEST_SUCCESS;
}

int test_hash_int_different_values(void)
{
    const int a = 1;
    const int b = 2;
    const size_t h1 = anv_hash_int(&a);
    const size_t h2 = anv_hash_int(&b);
    ASSERT_NOT_EQ(h1, h2);

    return TEST_SUCCESS;
}

int test_hash_int_zero(void)
{
    const int zero = 0;
    const size_t hash = anv_hash_int(&zero);
    // Hash of 0 is allowed to be 0 - just check consistency
    const size_t hash2 = anv_hash_int(&zero);
    ASSERT_EQ(hash, hash2);

    return TEST_SUCCESS;
}

int test_hash_int_negative(void)
{
    const int neg = -42;
    const size_t hash = anv_hash_int(&neg);

    const int pos = 42;
    const size_t hash_pos = anv_hash_int(&pos);

    // Negative and positive should differ
    ASSERT_NOT_EQ(hash, hash_pos);

    return TEST_SUCCESS;
}

int test_hash_int_null(void)
{
    const size_t hash = anv_hash_int(NULL);
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

//==============================================================================
// Pointer Hash Tests
//==============================================================================

int test_hash_pointer_basic(void)
{
    const int a = 0;
    const int b = 0;
    const size_t h1 = anv_hash_pointer(&a);
    const size_t h2 = anv_hash_pointer(&b);

    // Different addresses should produce different hashes
    ASSERT_NOT_EQ(h1, h2);

    // Same address should produce same hash
    const size_t h1_again = anv_hash_pointer(&a);
    ASSERT_EQ(h1, h1_again);

    return TEST_SUCCESS;
}

int test_hash_pointer_null(void)
{
    // NULL pointer should produce a consistent hash
    const size_t h1 = anv_hash_pointer(NULL);
    const size_t h2 = anv_hash_pointer(NULL);
    ASSERT_EQ(h1, h2);

    return TEST_SUCCESS;
}

//==============================================================================
// Main
//==============================================================================

int main(void)
{
    const ANVTestCase tests[] = {
        // String Hash
        TEST_REGISTER(test_hash_string_basic),
        TEST_REGISTER(test_hash_string_different_inputs),
        TEST_REGISTER(test_hash_string_empty),
        TEST_REGISTER(test_hash_string_null),
        TEST_REGISTER(test_hash_string_similar),
        TEST_REGISTER(test_hash_string_distribution),

        // Integer Hash
        TEST_REGISTER(test_hash_int_basic),
        TEST_REGISTER(test_hash_int_different_values),
        TEST_REGISTER(test_hash_int_zero),
        TEST_REGISTER(test_hash_int_negative),
        TEST_REGISTER(test_hash_int_null),
        TEST_REGISTER(test_hash_int_distribution),

        // Pointer Hash
        TEST_REGISTER(test_hash_pointer_basic),
        TEST_REGISTER(test_hash_pointer_null),
    };

    return anv_run_tests("Hash Functions", tests, sizeof(tests) / sizeof(tests[0]));
}

