//
// Legacy redirect — delegates to the public Anvil test framework.
//

#ifndef ANVIL_TESTASSERT_H
#define ANVIL_TESTASSERT_H

// All assertion macros now live in the public framework header.
// This file is kept as a redirect for existing test code.
#include "anvil/testing/assert.h"

// ASSERT_EQ_DSTRING is project-specific (depends on containers/dynamicstring.h),
// so it stays here rather than in the public framework.
#include <stdio.h>
#include "containers/dynamicstring.h"

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

#endif //ANVIL_TESTASSERT_H