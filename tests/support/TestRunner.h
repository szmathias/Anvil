//
// Standardized test runner for Anvil test suite.
// Replaces per-file TestCase structs and main() boilerplate.
//

#ifndef ANVIL_TESTRUNNER_H
#define ANVIL_TESTRUNNER_H

#include <stdio.h>
#include "TestAssert.h"

//==============================================================================
// Types
//==============================================================================

typedef struct
{
    int (*func)(void);
    const char* name;
} ANVTestCase;

//==============================================================================
// Test runner
//==============================================================================

/**
 * Run a suite of test cases with proper result tracking.
 *
 * Handles TEST_SUCCESS, TEST_FAILURE, and TEST_SKIPPED correctly.
 * Resets the failing allocator countdown between tests to prevent
 * state leaking across test boundaries.
 *
 * @param suite_name Name of the test suite (for output formatting)
 * @param tests      Array of test cases
 * @param count      Number of test cases in the array
 * @return 0 if all tests passed or were skipped, 1 if any test failed
 */
int anv_run_tests(const char* suite_name, const ANVTestCase* tests, int count);

#endif //ANVIL_TESTRUNNER_H
