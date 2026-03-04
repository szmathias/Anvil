//
// Anvil Test Framework — Assertion Macros
//
// Public header. Include via <anvil/testing.h> or <anvil/testing/test.h>.
//

#ifndef ANV_TESTING_ASSERT_H
#define ANV_TESTING_ASSERT_H

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "anvil/common/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Test Result Codes
//==============================================================================

#define TEST_SUCCESS  1
#define TEST_FAILURE  0
#define TEST_SKIPPED (-1)

//==============================================================================
// Basic Assertions
//==============================================================================

#define ASSERT(expr) \
    do \
    { \
        if (!(expr)) \
        { \
            fprintf(stderr, "Assertion failed: %s, file %s, line %d\n", #expr, __FILE__, __LINE__); \
            return TEST_FAILURE; \
        } \
    } while (0)

#define ASSERT_TRUE(expr) \
    do \
    { \
        if (!(expr)) \
        { \
            fprintf(stderr, "Assertion failed: %s is true, file %s, line %d\n", #expr, __FILE__, __LINE__); \
            return TEST_FAILURE; \
        } \
    } while (0)

#define ASSERT_FALSE(expr) \
    do \
    { \
        if (expr) \
        { \
            fprintf(stderr, "Assertion failed: %s is false, file %s, line %d\n", #expr, __FILE__, __LINE__); \
            return TEST_FAILURE; \
        } \
    } while (0)

#define ASSERT_FAIL(msg) \
    do \
    { \
        fprintf(stderr, "Assertion failed: %s, file %s, line %d\n", msg, __FILE__, __LINE__); \
        return TEST_FAILURE; \
    } while (0)

//==============================================================================
// Equality Assertions
//==============================================================================

#define ASSERT_EQ(a, b) \
    do \
    { \
        if ((a) != (b)) \
        { \
            fprintf(stderr, "Assertion failed: %s == %s, file %s, line %d\n", #a, #b, __FILE__, __LINE__); \
            fprintf(stderr, "  Actual: %lld != %lld\n", (long long)(a), (long long)(b)); \
            return TEST_FAILURE; \
        } \
    } while (0)

#define ASSERT_EQ_SIZE(a, b) \
    do \
    { \
        size_t _val_a = (size_t)(a); \
        size_t _val_b = (size_t)(b); \
        if (_val_a != _val_b) \
        { \
            fprintf(stderr, "Assertion failed: %s == %s, file %s, line %d\n", #a, #b, __FILE__, __LINE__); \
            fprintf(stderr, "  Actual: %zu != %zu\n", _val_a, _val_b); \
            return TEST_FAILURE; \
        } \
    } while (0)

#define ASSERT_EQ_UINT(a, b) \
    do \
    { \
        unsigned long long _val_a = (unsigned long long)(a); \
        unsigned long long _val_b = (unsigned long long)(b); \
        if (_val_a != _val_b) \
        { \
            fprintf(stderr, "Assertion failed: %s == %s, file %s, line %d\n", #a, #b, __FILE__, __LINE__); \
            fprintf(stderr, "  Actual: %llu != %llu\n", _val_a, _val_b); \
            return TEST_FAILURE; \
        } \
    } while (0)

#define ASSERT_EQ_STR(a, b) \
    do \
    { \
        size_t len_a = strlen(a); \
        size_t len_b = strlen(b); \
        if (len_a != len_b || strncmp((a), (b), len_a) != 0) \
        { \
            fprintf(stderr, "Assertion failed: %s == %s, file %s, line %d\n", #a, #b, __FILE__, __LINE__); \
            fprintf(stderr, "  Actual: '%s' (len=%zu) != '%s' (len=%zu)\n", (a), len_a, (b), len_b); \
            return TEST_FAILURE; \
        } \
    } while (0)

#define ASSERT_EQ_FLOAT(a, b, eps) \
    do \
    { \
        double val_a = (double)(a); \
        double val_b = (double)(b); \
        double diff = val_a - val_b; \
        double val = diff < 0.0 ? -diff : diff; \
        if (val > (double)(eps)) \
        { \
            fprintf(stderr, "Assertion failed: fabs(%s - %s) <= %s, file %s, line %d\n", #a, #b, #eps, __FILE__, __LINE__); \
            fprintf(stderr, "  Actual: %g != %g (eps=%g)\n", val_a, val_b, (double)(eps)); \
            return TEST_FAILURE; \
        } \
    } while (0)

#define ASSERT_EQ_PTR(a, b) \
    do \
    { \
        if ((void*)(a) != (void*)(b)) \
        { \
            fprintf(stderr, "Assertion failed: %s == %s, file %s, line %d\n", #a, #b, __FILE__, __LINE__); \
            fprintf(stderr, "  Actual: %p != %p\n", (void*)(a), (void*)(b)); \
            return TEST_FAILURE; \
        } \
    } while (0)

//==============================================================================
// Inequality Assertions
//==============================================================================

#define ASSERT_NOT_EQ(a, b) \
    do \
    { \
        if ((a) == (b)) \
        { \
            fprintf(stderr, "Assertion failed: %s != %s, file %s, line %d\n", #a, #b, __FILE__, __LINE__); \
            fprintf(stderr, "  Actual: %lld == %lld\n", (long long)(a), (long long)(b)); \
            return TEST_FAILURE; \
        } \
    } while (0)

#define ASSERT_NOT_EQ_STR(a, b) \
    do \
    { \
        size_t _len_a = strlen(a); \
        size_t _len_b = strlen(b); \
        if (_len_a == _len_b && strncmp((a), (b), _len_a) == 0) \
        { \
            fprintf(stderr, "Assertion failed: %s != %s, file %s, line %d\n", #a, #b, __FILE__, __LINE__); \
            fprintf(stderr, "  Actual: '%s' == '%s'\n", (a), (b)); \
            return TEST_FAILURE; \
        } \
    } while (0)

#define ASSERT_NOT_EQ_PTR(a, b) \
    do \
    { \
        if ((void*)(a) == (void*)(b)) \
        { \
            fprintf(stderr, "Assertion failed: %s != %s, file %s, line %d\n", #a, #b, __FILE__, __LINE__); \
            fprintf(stderr, "  Actual: %p == %p\n", (void*)(a), (void*)(b)); \
            return TEST_FAILURE; \
        } \
    } while (0)

//==============================================================================
// NULL Assertions
//==============================================================================

#define ASSERT_NULL(ptr) \
    do \
    { \
        if ((ptr) != NULL) \
        { \
            fprintf(stderr, "Assertion failed: %s == NULL, file %s, line %d\n", #ptr, __FILE__, __LINE__); \
            fprintf(stderr, "  Actual: %p != NULL\n", (void*)(ptr)); \
            return TEST_FAILURE; \
        } \
    } while (0)

#define ASSERT_NOT_NULL(ptr) \
    do \
    { \
        if ((ptr) == NULL) \
        { \
            fprintf(stderr, "Assertion failed: %s != NULL, file %s, line %d\n", #ptr, __FILE__, __LINE__); \
            return TEST_FAILURE; \
        } \
    } while (0)

//==============================================================================
// Relational Assertions
//==============================================================================

#define ASSERT_GT(a, b) \
    do \
    { \
        if (!((a) > (b))) \
        { \
            fprintf(stderr, "Assertion failed: %s > %s, file %s, line %d\n", #a, #b, __FILE__, __LINE__); \
            fprintf(stderr, "  Actual: %lld <= %lld\n", (long long)(a), (long long)(b)); \
            return TEST_FAILURE; \
        } \
    } while (0)

#define ASSERT_LT(a, b) \
    do \
    { \
        if (!((a) < (b))) \
        { \
            fprintf(stderr, "Assertion failed: %s < %s, file %s, line %d\n", #a, #b, __FILE__, __LINE__); \
            fprintf(stderr, "  Actual: %lld >= %lld\n", (long long)(a), (long long)(b)); \
            return TEST_FAILURE; \
        } \
    } while (0)

#define ASSERT_GTE(a, b) \
    do \
    { \
        if (!((a) >= (b))) \
        { \
            fprintf(stderr, "Assertion failed: %s >= %s, file %s, line %d\n", #a, #b, __FILE__, __LINE__); \
            fprintf(stderr, "  Actual: %lld < %lld\n", (long long)(a), (long long)(b)); \
            return TEST_FAILURE; \
        } \
    } while (0)

#define ASSERT_LTE(a, b) \
    do \
    { \
        if (!((a) <= (b))) \
        { \
            fprintf(stderr, "Assertion failed: %s <= %s, file %s, line %d\n", #a, #b, __FILE__, __LINE__); \
            fprintf(stderr, "  Actual: %lld > %lld\n", (long long)(a), (long long)(b)); \
            return TEST_FAILURE; \
        } \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif // ANV_TESTING_ASSERT_H

