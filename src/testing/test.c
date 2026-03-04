//
// Anvil Test Framework — Runner Implementation
//

#include "anvil/testing/test.h"

#include <stdio.h>

//==============================================================================
// Optional between-test hook
//==============================================================================

// Test binaries can set this to a function that resets test-specific state
// (e.g. the failing allocator countdown) between test cases.
// The default is NULL (no-op).
static void (*anv_test_between_hook)(void) = NULL;

ANV_API void anv_test_set_between_hook(void (*hook)(void))
{
    anv_test_between_hook = hook;
}

//==============================================================================
// Runner
//==============================================================================

ANV_API int anv_run_tests(const char* suite_name, const ANVTestCase* tests, const int count)
{
    int passed = 0;
    int failed = 0;
    int skipped = 0;

    for (int i = 0; i < count; i++)
    {
        // Reset test-specific state between tests
        if (anv_test_between_hook)
        {
            anv_test_between_hook();
        }

        // --- Setup ---
        void* ctx = NULL;
        if (tests[i].setup)
        {
            ctx = tests[i].setup();
        }

        // --- Run test ---
        int result;
        if (tests[i].has_ctx)
        {
            result = tests[i].func_ctx(ctx);
        }
        else
        {
            result = tests[i].func();
        }

        // --- Teardown (always runs, even on failure/skip) ---
        if (tests[i].teardown)
        {
            tests[i].teardown(ctx);
        }

        // --- Record result ---
        if (result == TEST_SUCCESS)
        {
            passed++;
        }
        else if (result == TEST_SKIPPED)
        {
            printf("[SKIP] %s\n", tests[i].name);
            skipped++;
        }
        else
        {
            printf("[FAIL] %s\n", tests[i].name);
            failed++;
        }
    }

    printf("\n%s: %d passed, %d failed, %d skipped (of %d)\n",
           suite_name, passed, failed, skipped, count);

    return failed > 0 ? 1 : 0;
}