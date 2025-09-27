//
// Created by zack on 9/26/25.
//

#ifndef ANVIL_SUITE_H
#define ANVIL_SUITE_H

#include "common/common.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct ANVTestCase ANVTestCase;
typedef struct ANVTestSuite ANVTestSuite;
typedef struct ANVTestResult ANVTestResult;
typedef struct ANVFixture ANVFixture;
typedef struct ANVTestEntry ANVTestEntry;

// Test function signature
typedef void (*ANVTestFunction)(ANVTestCase* test);

//==============================================================================
// Core Structures
//==============================================================================

// Individual test entry within a suite
struct ANVTestEntry
{
        const char* name;          // Test name
        ANVTestFunction test_func; // Test function to call
        ANVFixture* fixture;       // Per-test fixture (optional, overrides suite fixture)
        ANVAllocator* allocator;   // Per-test allocator (optional, overrides suite allocator)
};

// Test case - passed to each test function
struct ANVTestCase
{
        const char* name;        // Current test name
        ANVAllocator* allocator; // Allocator for this test
        ANVFixture* fixture;     // Fixture instance for this test (optional)
        ANVTestResult* result;   // For internal framework use
};

// Test result tracking
struct ANVTestResult
{
        int tests_run;
        int tests_passed;
        int tests_failed;
        int assertions_run;
        int assertions_passed;
        int assertions_failed;
        double execution_time_ms;
        bool verbose;
};

// Test suite - container for multiple tests
struct ANVTestSuite
{
        const char* name;     // Suite name
        ANVTestEntry* tests;  // Dynamic array of test entries
        size_t test_count;    // Number of tests
        size_t test_capacity; // Capacity of test array

        // Default configuration (can be overridden per test)
        ANVAllocator* default_allocator; // Default allocator for all tests
        ANVFixture* default_fixture;     // Default fixture template for all tests

        // Suite configuration
        ANVAllocator* suite_allocator; // Allocator for suite management
        ANVTestResult result;          // Test results
        bool verbose;                  // Verbose output
};

//==============================================================================
// Suite Management
//==============================================================================

// Create and destroy test suites
ANVTestSuite* anv_suite_create(const char* name);
void anv_suite_destroy(ANVTestSuite* suite);

// Configure test suite defaults (applied to all tests unless overridden)
void anv_suite_set_allocator(ANVTestSuite* suite, ANVAllocator* allocator);
void anv_suite_set_fixture(ANVTestSuite* suite, ANVFixture* fixture_template);
void anv_suite_set_verbose(ANVTestSuite* suite, bool verbose);
void anv_suite_set_all(ANVTestSuite* suite, ANVAllocator* allocator, ANVFixture* fixture_template, bool verbose);

//==============================================================================
// Test Management
//==============================================================================

// Add tests to suite - uses suite defaults
ANVResult anv_suite_add(ANVTestSuite* suite, const char* test_name, ANVTestFunction test_func);

// Add test with custom fixture (overrides suite default)
ANVResult anv_suite_add_with_fixture(ANVTestSuite* suite, const char* test_name,
                                     ANVTestFunction test_func, ANVFixture* fixture);

// Add test with custom allocator (overrides suite default)
ANVResult anv_suite_add_with_allocator(ANVTestSuite* suite, const char* test_name,
                                       ANVTestFunction test_func, ANVAllocator* allocator);

// Add test with both custom fixture and allocator
ANVResult anv_suite_add_with_config(ANVTestSuite* suite, const char* test_name,
                                    ANVTestFunction test_func, ANVFixture* fixture,
                                    ANVAllocator* allocator);

//==============================================================================
// Test Execution
//==============================================================================

// Run tests
int anv_suite_run(ANVTestSuite* suite);
ANVTestResult anv_suite_get_result(const ANVTestSuite* suite);

// Utility functions
void anv_test_print_summary(const ANVTestResult* result);

#ifdef __cplusplus
}
#endif

#endif // ANVIL_SUITE_H
