//
// Created by zack on 9/27/25.
//

#include "suite.h"
#include "benchmarks.h"

#include "fixtures.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Color codes for output
#ifdef ANV_PLATFORM_WINDOWS
#define COLOR_RED     ""
#define COLOR_GREEN   ""
#define COLOR_YELLOW  ""
#define COLOR_BLUE    ""
#define COLOR_RESET   ""
#else
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_RESET   "\033[0m"
#endif

//==============================================================================
// Internal Structs
//==============================================================================

// Individual test entry within a suite
typedef struct ANVTestEntry
{
    const char* name;               // Test name
    ANVTestFunction test_func;      // Test function to call
    ANVFixture* fixture;            // Per-test fixture (optional, overrides suite fixture)
    ANVAllocator allocator;         // Per-test allocator (optional, overrides suite allocator)
    bool has_custom_allocator;      // Whether a custom allocator is set
    ANVBenchmarkRegistry* registry; // Per-test benchmark registry (optional, overrides suite registry)
    void* user_data;                // Internal data (e.g., benchmark wrapper)
    bool is_benchmark;              // Whether this is a benchmark test
} ANVTestEntry;

// Test suite - container for multiple tests
struct ANVTestSuite
{
    const char* name;     // Suite name
    ANVTestEntry* tests;  // Dynamic array of test entries
    size_t test_count;    // Number of tests
    size_t test_capacity; // Capacity of test array

    // Default configuration (can be overridden per test)
    ANVAllocator default_allocator;         // Default allocator for all tests
    ANVFixture* default_fixture;            // Default fixture template for all tests
    ANVBenchmarkRegistry* default_registry; // Default benchmark registry for all tests

    // Suite configuration
    ANVAllocator suite_allocator; // Allocator for suite management
    ANVTestResult result;         // Test results
    bool verbose;                 // Verbose output
};

//==============================================================================
// Test Suite Implementation
//==============================================================================

ANV_API ANVTestSuite* anv_suite_create(const char* name)
{
    const ANVAllocator default_alloc = anv_alloc_default();

    ANVTestSuite* suite = anv_alloc_malloc(&default_alloc, sizeof(ANVTestSuite));
    if (!suite)
    {
        return NULL;
    }

    memset(suite, 0, sizeof(ANVTestSuite));
    suite->name = name;
    suite->suite_allocator = default_alloc;
    suite->default_allocator = default_alloc;

    suite->test_capacity = ANV_DEFAULT_CAPACITY;
    suite->tests = anv_alloc_malloc(&default_alloc, suite->test_capacity * sizeof(ANVTestEntry));
    if (!suite->tests)
    {
        anv_alloc_free(&default_alloc, suite);
        return NULL;
    }

    return suite;
}

ANV_API void anv_suite_destroy(ANVTestSuite* suite)
{
    if (!suite)
    {
        return;
    }

    // Clean up any benchmark wrappers
    for (size_t i = 0; i < suite->test_count; i++)
    {
        if (suite->tests[i].user_data && suite->tests[i].is_benchmark)
        {
            anv_alloc_free(&suite->suite_allocator, suite->tests[i].user_data);
        }
    }

    anv_alloc_free(&suite->suite_allocator, suite->tests);
    anv_alloc_free(&suite->suite_allocator, suite);
}

//==============================================================================
// Suite Configuration
//==============================================================================

ANV_API void anv_suite_set_allocator(ANVTestSuite* suite, const ANVAllocator* allocator)
{
    if (suite && allocator)
    {
        suite->default_allocator = *allocator;
    }
}

ANV_API void anv_suite_set_fixture(ANVTestSuite* suite, ANVFixture* fixture_template)
{
    if (suite)
    {
        suite->default_fixture = fixture_template;
    }
}

ANV_API void anv_suite_set_verbose(ANVTestSuite* suite, const bool verbose)
{
    if (suite)
    {
        suite->verbose = verbose;
    }
}

ANV_API void anv_suite_set_benchmark_registry(ANVTestSuite* suite, ANVBenchmarkRegistry* registry)
{
    if (suite)
    {
        suite->default_registry = registry;
    }
}

//==============================================================================
// Test Management
//==============================================================================

static ANVResult anv_suite_grow_tests(ANVTestSuite* suite)
{
    if (!suite)
    {
        return ANV_RESULT_INVALID_ARGUMENT;
    }

    const size_t new_capacity = suite->test_capacity * 2;
    ANVTestEntry* new_tests = anv_alloc_malloc(&suite->default_allocator, new_capacity * sizeof(ANVTestEntry));

    if (!new_tests)
    {
        return ANV_RESULT_OUT_OF_MEMORY;
    }

    memcpy(new_tests, suite->tests, suite->test_count * sizeof(ANVTestEntry));
    anv_alloc_free(&suite->default_allocator, new_tests);

    suite->tests = new_tests;
    suite->test_capacity = new_capacity;
    return ANV_RESULT_SUCCESS;
}

ANV_API ANVResult anv_suite_add(ANVTestSuite* suite, const char* test_name, const ANVTestFunction test_func)
{
    return anv_suite_add_with_config(suite, test_name, test_func, NULL, NULL, NULL);
}

ANV_API ANVResult anv_suite_add_with_fixture(ANVTestSuite* suite, const char* test_name,
                                             const ANVTestFunction test_func, ANVFixture* fixture)
{
    return anv_suite_add_with_config(suite, test_name, test_func, fixture, NULL, NULL);
}

ANV_API ANVResult anv_suite_add_with_allocator(ANVTestSuite* suite, const char* test_name,
                                               const ANVTestFunction test_func, const ANVAllocator* allocator)
{
    return anv_suite_add_with_config(suite, test_name, test_func, NULL, allocator, NULL);
}

ANV_API ANVResult anv_suite_add_with_registry(ANVTestSuite* suite, const char* test_name,
                                              const ANVTestFunction test_func, ANVBenchmarkRegistry* registry)
{
    return anv_suite_add_with_config(suite, test_name, test_func, NULL, NULL, registry);
}

ANV_API ANVResult anv_suite_add_with_config(ANVTestSuite* suite, const char* test_name,
                                            const ANVTestFunction test_func, ANVFixture* fixture,
                                            const ANVAllocator* allocator, ANVBenchmarkRegistry* registry)
{
    if (!suite || !test_name || !test_func)
    {
        return ANV_RESULT_INVALID_ARGUMENT;
    }

    if (suite->test_count >= suite->test_capacity)
    {
        const ANVResult result = anv_suite_grow_tests(suite);
        if (result != ANV_RESULT_SUCCESS)
        {
            return result;
        }
    }

    ANVTestEntry* entry = &suite->tests[suite->test_count];
    entry->name = test_name;
    entry->test_func = test_func;
    entry->fixture = fixture;

    if (allocator)
    {
        entry->allocator = *allocator;
        entry->has_custom_allocator = true;
    }
    else
    {
        memset(&entry->allocator, 0, sizeof(ANVAllocator)); // Clear it
        entry->has_custom_allocator = false;
    }

    entry->registry = registry;
    entry->user_data = NULL;
    entry->is_benchmark = false;

    suite->test_count++;
    return ANV_RESULT_SUCCESS;
}

//==============================================================================
// Benchmark Integration
//==============================================================================

// Internal benchmark wrapper that handles benchmark setup
typedef struct
{
    ANVBenchmarkFunction benchmark_func;
    uint64_t target_iterations;
    double target_time_seconds;
    const char* benchmark_name;
} BenchmarkWrapper;

static void benchmark_test_wrapper(ANVTestCase* test)
{
    BenchmarkWrapper* wrapper = test->user_data;
    if (!wrapper)
    {
        return;
    }

    // Create benchmark context
    ANVBenchmark* bench = anv_benchmark_create_with_registry(
        &test->allocator,
        wrapper->benchmark_name,
        wrapper->target_iterations,
        test->registry
    );

    // Store benchmark in test case for access by benchmark macros
    test->benchmark = bench;

    // Run the benchmark function
    wrapper->benchmark_func(test);

    // Submit results to registry if available
    if (test->registry)
    {
        anv_benchmark_submit_result(bench);
    }

    // Clean up
    anv_benchmark_destroy(bench);
    test->benchmark = NULL;
}

static ANVResult anv_suite_add_benchmark_with_config(ANVTestSuite* suite, const char* name,
                                                     const ANVBenchmarkFunction benchmark_func,
                                                     const uint64_t iterations, const double target_seconds,
                                                     ANVFixture* fixture, const ANVAllocator* allocator,
                                                     ANVBenchmarkRegistry* registry)
{
    if (!suite || !name || !benchmark_func)
    {
        return ANV_RESULT_INVALID_ARGUMENT;
    }

    // Create wrapper to store benchmark parameters
    BenchmarkWrapper* wrapper = anv_alloc_malloc(&suite->suite_allocator, sizeof(BenchmarkWrapper));
    if (!wrapper)
    {
        return ANV_RESULT_OUT_OF_MEMORY;
    }

    wrapper->benchmark_func = benchmark_func;
    wrapper->target_iterations = iterations > 0 ? iterations : 10000;
    wrapper->target_time_seconds = target_seconds;
    wrapper->benchmark_name = name;

    // Add as regular test but with benchmark wrapper
    const ANVResult result = anv_suite_add_with_config(suite, name, benchmark_test_wrapper,
                                                       fixture, allocator, registry);

    if (result != ANV_RESULT_SUCCESS)
    {
        anv_alloc_free(&suite->suite_allocator, wrapper);
        return result;
    }

    // Store wrapper in the test entry for cleanup later
    suite->tests[suite->test_count - 1].user_data = wrapper;
    suite->tests[suite->test_count - 1].is_benchmark = true;

    return ANV_RESULT_SUCCESS;
}

// ANV_API ANVResult anv_suite_add_benchmark(ANVTestSuite* suite, const char* name,
//                                           const ANVTestFunction benchmark_func)
// {
//     return anv_suite_add_benchmark_with_config(suite, name, benchmark_func, 10000, 0.0, NULL, NULL, NULL);
// }
//
// ANV_API ANVResult anv_suite_add_benchmark_with_iterations(ANVTestSuite* suite, const char* name,
//                                                           const ANVTestFunction benchmark_func,
//                                                           const uint64_t iterations)
// {
//     return anv_suite_add_benchmark_with_config(suite, name, benchmark_func, iterations, 0.0, NULL, NULL, NULL);
// }
//
// ANV_API ANVResult anv_suite_add_benchmark_with_time(ANVTestSuite* suite, const char* name,
//                                                     const ANVTestFunction benchmark_func,
//                                                     const double target_seconds)
// {
//     return anv_suite_add_benchmark_with_config(suite, name, benchmark_func, 0, target_seconds, NULL, NULL, NULL);
// }

//==============================================================================
// Test Execution
//==============================================================================

ANV_API int anv_suite_run(ANVTestSuite* suite)
{
    if (!suite)
    {
        return -1;
    }

    memset(&suite->result, 0, sizeof(ANVTestResult));
    suite->result.verbose = suite->verbose;

    printf("=== Running Test Suite: %s ===\n", suite->name);
    printf("Tests to run: %zu\n\n", suite->test_count);

    for (size_t i = 0; i < suite->test_count; i++)
    {
        const ANVTestEntry* entry = &suite->tests[i];

        ANVTestCase test_case = {0};
        test_case.name = entry->name;

        // Use custom allocator if provided, otherwise use suite default
        // Point to the stored allocators (safe because they're stored by value)
        if (entry->has_custom_allocator)
        {
            test_case.allocator = entry->allocator;
        }
        else
        {
            test_case.allocator = suite->default_allocator;
        }

        test_case.fixture = entry->fixture ? entry->fixture : suite->default_fixture;
        test_case.registry = entry->registry ? entry->registry : suite->default_registry;
        test_case.result = &suite->result;
        test_case.user_data = entry->user_data;
        test_case.benchmark = NULL;

        if (suite->verbose)
        {
            printf("Running %s: %s\n",
                   entry->is_benchmark ? "BENCHMARK" : "TEST", entry->name);
        }

        suite->result.tests_run++;

        // Setup fixture if present
        bool fixture_setup_ok = true;
        if (test_case.fixture && test_case.fixture->setup)
        {
            if (test_case.fixture->setup(test_case.fixture, &test_case) != 0)
            {
                printf("[ERROR] Failed to setup fixture for %s\n", entry->name);
                suite->result.tests_failed++;
                fixture_setup_ok = false;
            }
        }

        // Run test if fixture setup succeeded
        if (fixture_setup_ok)
        {
            const int initial_assertion_failures = suite->result.assertions_failed;

            entry->test_func(&test_case);

            if (suite->result.assertions_failed == initial_assertion_failures)
            {
                suite->result.tests_passed++;
                if (suite->verbose)
                {
                    printf("  [PASS] %s\n", entry->name);
                }
            }
            else
            {
                suite->result.tests_failed++;
                printf("  [FAIL] %s\n", entry->name);
            }
        }

        // Teardown fixture
        if (test_case.fixture && test_case.fixture->teardown)
        {
            test_case.fixture->teardown(test_case.fixture, &test_case);
        }
    }

    anv_test_print_summary(&suite->result);

    return suite->result.tests_failed > 0 ? 1 : 0;
}

ANV_API ANVTestResult anv_suite_get_result(const ANVTestSuite* suite)
{
    if (suite)
    {
        return suite->result;
    }

    const ANVTestResult empty = {0};
    return empty;
}

ANV_API void anv_test_print_summary(const ANVTestResult* result)
{
    if (!result)
    {
        return;
    }

    printf("\n%s=== Test Summary ===%s\n", COLOR_BLUE, COLOR_RESET);
    printf("Tests run:    %d\n", result->tests_run);
    printf("Tests passed: %s%d%s\n", COLOR_GREEN, result->tests_passed, COLOR_RESET);
    printf("Tests failed: %s%d%s\n",
           result->tests_failed > 0 ? COLOR_RED : COLOR_GREEN,
           result->tests_failed, COLOR_RESET);

    printf("\nAssertions run:    %d\n", result->assertions_run);
    printf("Assertions passed: %s%d%s\n", COLOR_GREEN, result->assertions_passed, COLOR_RESET);
    printf("Assertions failed: %s%d%s\n",
           result->assertions_failed > 0 ? COLOR_RED : COLOR_GREEN,
           result->assertions_failed, COLOR_RESET);

    printf("\nExecution time: %.3f ms\n", result->execution_time_ms);

    if (result->tests_failed == 0)
    {
        printf("\n%s🎉 All tests passed! 🎉%s\n", COLOR_GREEN, COLOR_RESET);
    }
    else
    {
        printf("\n%s❌ %d test(s) failed%s\n", COLOR_RED, result->tests_failed, COLOR_RESET);
    }
}

//==============================================================================
// Benchmark Integration Helpers
//==============================================================================

ANV_API ANVBenchmarkResult* anv_suite_get_benchmark_results(const ANVTestSuite* suite)
{
    if (!suite || !suite->default_registry)
    {
        return NULL;
    }

    return anv_benchmark_registry_get_results(suite->default_registry);
}

ANV_API size_t anv_suite_get_benchmark_count(const ANVTestSuite* suite)
{
    if (!suite || !suite->default_registry)
    {
        return 0;
    }

    return anv_benchmark_registry_get_count(suite->default_registry);
}

ANV_API ANVBenchmark* anv_test_get_benchmark(const ANVTestCase* test)
{
    return test ? test->benchmark : NULL;
}