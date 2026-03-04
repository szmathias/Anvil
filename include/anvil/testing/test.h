//
// Anvil Test Framework — Test Runner & Registration
//
// Public header. Include via <anvil/testing.h> or directly.
//
// Supports two test styles in the same suite:
//   - Plain tests:   int test_foo(void)               → TEST_REGISTER(test_foo)
//   - Fixture tests: int test_foo(void* ctx)           → TEST_REGISTER_FIXTURE(test_foo, my_setup, my_teardown)
//
// Fixture callbacks:
//   void* setup(void)       — called before the test, returns context pointer (or NULL)
//   void  teardown(void*)   — called after the test, ALWAYS runs (even on failure/skip)
//

#ifndef ANV_TESTING_TEST_H
#define ANV_TESTING_TEST_H

#include "anvil/testing/assert.h"

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Types
//==============================================================================

/**
 * A single test case with optional per-test setup/teardown.
 *
 * For plain tests (no fixture):
 *   func      — test function returning TEST_SUCCESS / TEST_FAILURE / TEST_SKIPPED
 *   setup     — NULL
 *   teardown  — NULL
 *   has_ctx   — false
 *
 * For fixture tests:
 *   func_ctx  — test function receiving the context pointer from setup()
 *   setup     — allocates and returns a context pointer
 *   teardown  — frees the context; always called, even on test failure
 *   has_ctx   — true
 */
typedef struct ANVTestCase
{
    int (*func)(void);
    int (*func_ctx)(void* ctx);
    const char* name;
    void* (*setup)(void);
    void  (*teardown)(void* ctx);
    bool has_ctx;
} ANVTestCase;

//==============================================================================
// Registration Macros
//==============================================================================

/**
 * Register a plain test function (no fixture).
 *
 * Usage:
 *   const ANVTestCase tests[] = {
 *       TEST_REGISTER(test_create_destroy),
 *       TEST_REGISTER(test_push_front),
 *   };
 */
#define TEST_REGISTER(fn) \
    { (fn), NULL, #fn, NULL, NULL, false }

/**
 * Register a fixture test function with per-test setup and teardown.
 *
 * The setup function returns a void* context that is passed to the test
 * and then to teardown. Teardown always runs, even if the test fails.
 *
 * Usage:
 *   static void* my_setup(void) { ... return ctx; }
 *   static void  my_teardown(void* ctx) { ... free(ctx); }
 *   int test_push(void* ctx) { MyFixture* f = ctx; ... }
 *
 *   const ANVTestCase tests[] = {
 *       TEST_REGISTER_FIXTURE(test_push, my_setup, my_teardown),
 *       TEST_REGISTER(test_null_handling),  // no fixture, same array
 *   };
 */
#define TEST_REGISTER_FIXTURE(fn, setup_fn, teardown_fn) \
    { NULL, (fn), #fn, (setup_fn), (teardown_fn), true }

//==============================================================================
// Test Runner
//==============================================================================

/**
 * Run a suite of test cases with proper result tracking.
 *
 * Handles TEST_SUCCESS, TEST_FAILURE, and TEST_SKIPPED.
 * For each test:
 *   1. Calls setup() if non-NULL to obtain a context pointer.
 *   2. Calls the test function (with or without context).
 *   3. Calls teardown(ctx) if non-NULL — ALWAYS, even on failure/skip.
 *   4. Resets internal test state between tests.
 *
 * @param suite_name  Name of the test suite (for output formatting)
 * @param tests       Array of test cases
 * @param count       Number of test cases in the array
 * @return 0 if all tests passed or were skipped, 1 if any test failed
 */
ANV_API int anv_run_tests(const char* suite_name, const ANVTestCase* tests, int count);

/**
 * Set a hook function that is called between test cases.
 *
 * Useful for resetting test-specific state (e.g. the failing allocator
 * countdown) between tests so that state doesn't leak across boundaries.
 * Set to NULL to disable.
 *
 * @param hook  Function to call between tests, or NULL
 */
ANV_API void anv_test_set_between_hook(void (*hook)(void));

#ifdef __cplusplus
}
#endif

#endif // ANV_TESTING_TEST_H

