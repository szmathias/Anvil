//
// Created by zack on 9/26/25.
//

#ifndef ANVIL_BENCHMARKS_H
#define ANVIL_BENCHMARKS_H

#include "anvil/common.h"
#include "anvil/containers/arraylist.h"
#include "anvil/system/timing.h"

#ifdef __cplusplus
extern "C" {
#endif

// Example usage at bottom of file

/**
 * Benchmarking structure for performance measurement.
 * Supports both iteration-based and time-based benchmarks.
 */
typedef struct ANVBenchmark
{
    const char* name;
    uint64_t target_iterations;            // Target iterations (0 if using time-based)
    double target_time_seconds;            // Target time in seconds (0 if using iteration-based)
    ANVAllocator alloc;

    ANVArrayList* timers;
    size_t current_timer_index;
    size_t warmup_iterations;

    size_t runs;
    ANVArrayList* aggregate_results;
    bool verbose;

    // Timing function pointers
    void (*start_timer)(struct ANVBenchmark* bench);
    void (*stop_timer)(struct ANVBenchmark* bench);
} ANVBenchmark;


ANV_API ANVBenchmark* anv_benchmark_create(ANVAllocator* alloc, const char* name, uint64_t iterations);
ANV_API ANVBenchmark* anv_benchmark_create_timed(ANVAllocator* alloc, const char* name, double seconds);
ANV_API void anv_benchmark_destroy(ANVBenchmark* bench);

ANV_API void anv_benchmark_run(ANVBenchmark* bench, void (*test_func)(ANVBenchmark* bench));
ANV_API void anv_benchmark_run_warmup(ANVBenchmark* bench, void (*test_func)(ANVBenchmark* bench));
ANV_API void anv_benchmark_run_multiple(ANVBenchmark* bench, void (*test_func)(ANVBenchmark* bench), uint32_t runs);

ANV_API void anv_benchmark_set_warmup(ANVBenchmark* bench, uint64_t iterations);
ANV_API void anv_benchmark_set_verbose(ANVBenchmark* bench, bool verbose);

ANV_API void anv_benchmark_start_timer(ANVBenchmark* bench);
ANV_API void anv_benchmark_stop_timer(ANVBenchmark* bench);
ANV_API void anv_benchmark_submit_timing(ANVBenchmark* bench, const char *operation_name);

ANV_API void anv_benchmark_print_result(ANVBenchmark* bench);
ANV_API void anv_benchmark_print_result_units(ANVBenchmark* bench, ANVTime time_unit);
ANV_API void anv_benchmark_print_aggregate_results(ANVBenchmark* bench, ANVTime time_unit);

// Add to your header file
#if defined(__GNUC__) || defined(__clang__)
    #define ANV_COMPILER_BARRIER() __asm__ volatile("" ::: "memory")
#elif defined(_MSC_VER)
    #include <intrin.h>
    #define ANV_COMPILER_BARRIER() _ReadWriteBarrier()
#else
    #define ANV_COMPILER_BARRIER() do {} while(0)
#endif

#define ANV_BENCHMARK_START_TIMING(bench)       \
    do                                          \
    {                                           \
        ANV_COMPILER_BARRIER();                 \
        if ((bench) && (bench)->start_timer)    \
        {                                       \
            (bench)->start_timer((bench));      \
        }                                       \
    } while(0)

#define ANV_BENCHMARK_STOP_TIMING(bench)        \
    do                                          \
    {                                           \
        ANV_COMPILER_BARRIER();                 \
        if ((bench) && (bench)->stop_timer)     \
        {                                       \
            (bench)->stop_timer((bench));       \
        }                                       \
    } while(0)

#define ANV_BENCHMARK_SUBMIT_TIMING(bench, name)        \
    do                                                  \
    {                                                   \
        if (bench)                                      \
        {                                               \
            anv_benchmark_submit_timing(bench, name);   \
        }                                               \
    } while(0)

// Iteration helper
#define ANV_BENCHMARK_LOOP(bench, var) \
    for (uint64_t (var) = 0; (bench) && (var) < (bench)->target_iterations; (var)++)

// Time-based helper
#define ANV_BENCHMARK_LOOP_TIMED(bench, var) \
    for (uint64_t (var) = 0, m_start_time = anv_time_get_ns(); \
         (bench) && ((anv_time_get_ns() - m_start_time) < (uint64_t)((bench)->target_time_seconds * 1e9)); \
         (var)++)

//==============================================================================
// Example Usage
//==============================================================================

/*
// Example: Basic benchmark integrated with test suite

void benchmark_arraylist_push(ANVTestCase* test) {
    // Uses same fixture system as tests
    ANVArrayList* list = create_test_arraylist(test->allocator);

    // Benchmark loop with timing
    ANV_BENCHMARK_LOOP(test, i) {
        ANV_BENCHMARK_TIME_OPERATION(test, {
            anv_arraylist_push(list, &i);
        });
    }

    cleanup_test_arraylist(list);
}

void test_arraylist_correctness(ANVTestCase* test) {
    // Regular test - same fixture, same structure
    ANVArrayList* list = create_test_arraylist(test->allocator);

    anv_arraylist_push(list, &(int){42});
    ANV_ASSERT_EQUAL_INT(1, anv_arraylist_size(list), "Size should be 1");

    cleanup_test_arraylist(list);
}

int main(void) {
    ANVTestSuite* suite = anv_suite_create("ArrayListSuite");

    // Set up fixture for both tests and benchmarks
    ANVFixture fixture = create_arraylist_fixture();
    anv_suite_set_fixture(suite, &fixture);

    // Optional: Set up benchmark registry for collecting results
    ANVBenchmarkRegistry* registry = anv_benchmark_registry_create(anv_alloc_default());
    anv_suite_set_benchmark_registry(suite, registry);

    // Add regular tests
    anv_suite_add(suite, "correctness", test_arraylist_correctness);

    // Add benchmarks - same fixture system
    anv_suite_add_benchmark_with_iterations(suite, "push_performance",
                                           benchmark_arraylist_push, 10000);

    // Run everything together
    anv_suite_run(suite);

    // Get results
    ANVTestResult test_results = anv_suite_get_result(suite);
    printf("Tests: %d passed, %d failed\n", test_results.tests_passed, test_results.tests_failed);

    // Print benchmark summary
    anv_benchmark_registry_print_summary(registry);

    // Cleanup
    fixture.destroy(&fixture);
    anv_benchmark_registry_destroy(registry);
    anv_suite_destroy(suite);
    return 0;
}

// Example: Standalone benchmark usage (no test suite)

int main(void) {
    ANVBenchmarkRegistry* registry = anv_benchmark_registry_create(anv_alloc_default());

    // Create and run multiple benchmarks
    ANVBenchmark bench1 = anv_benchmark_create_with_registry("operation1", 10000, registry);
    ANV_BENCHMARK_LOOP(&bench1, i) {
        // benchmark code
    }
    anv_benchmark_submit_result(&bench1);

    ANVBenchmark bench2 = anv_benchmark_create_with_registry("operation2", 5000, registry);
    ANV_BENCHMARK_LOOP(&bench2, i) {
        // benchmark code
    }
    anv_benchmark_submit_result(&bench2);

    // Print summary
    anv_benchmark_registry_print_summary(registry);

    // Cleanup
    anv_benchmark_destroy(&bench1);
    anv_benchmark_destroy(&bench2);
    anv_benchmark_registry_destroy(registry);

    return 0;
}
*/

#ifdef __cplusplus
}
#endif

#endif // ANVIL_BENCHMARKS_H
