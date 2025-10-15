//
// Created by zack on 8/20/2025.
//

#ifndef ANVIL_TESTASSERT_H
#define ANVIL_TESTASSERT_H

#include <string.h>
#include <math.h>
#include <stdio.h>
#include "common/common.h"
#include "containers/dynamicstring.h"

#define TEST_SUCCESS 1
#define TEST_FAILURE 0
#define TEST_SKIPPED (-1)

#define ASSERT(expr) \
	do \
	{ \
		if (!(expr)) \
		{ \
			fprintf(stderr, "Assertion failed: %s, file %s, line %d\n", #expr, __FILE__, __LINE__); \
			return TEST_FAILURE; \
		} \
	} while (0)

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

/* NOTE: This macro is for DString types */
#define ASSERT_EQ_DSTRING(a, b) \
    do \
    { \
        if (anv_str_compare_string((a), (b)) != 0) \
        { \
            fprintf(stderr, "Assertion failed: %s == %s, file %s, line %d\n", #a, #b, __FILE__, __LINE__); \
            fprintf(stderr, "  Actual: '%s' != '%s'\n", anv_str_data(a), anv_str_data(b)); \
            return TEST_FAILURE; \
        } \
    } while (0)

#define ASSERT_EQ_FLOAT(a, b, eps) \
	do \
	{ \
		if (fabs((a) - (b)) > (eps)) \
		{ \
			fprintf(stderr, "Assertion failed: fabs(%s - %s) <= %s, file %s, line %d\n", #a, #b, #eps, __FILE__, __LINE__); \
			fprintf(stderr, "  Actual: %g != %g (eps=%g)\n", (double)(a), (double)(b), (double)(eps)); \
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

#endif //ANVIL_TESTASSERT_H