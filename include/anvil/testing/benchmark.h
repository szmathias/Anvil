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

#ifdef __cplusplus
}
#endif

#endif // ANVIL_BENCHMARKS_H
