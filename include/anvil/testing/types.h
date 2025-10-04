//
// Created by zack on 9/28/25.
//

#ifndef ANVIL_TESTING_TYPES_H
#define ANVIL_TESTING_TYPES_H

#include "benchmark.h"
#include "fixtures.h"
#include "common/common.h"

typedef struct ANVBenchmark ANVBenchmark;
typedef struct ANVBenchmarkRegistry ANVBenchmarkRegistry;
typedef ANVTestCase ANVTestCase;

// Test result summary
typedef struct ANVTestResult
{
    int tests_run;
    int tests_passed;
    int tests_failed;
    int assertions_run;
    int assertions_passed;
    int assertions_failed;
    double execution_time_ms;
    bool verbose;
} ANVTestResult;

typedef struct ANVTestCase
{
    const char* name;                    // Current test name
    ANVAllocator allocator;             // Allocator for this test
    ANVFixture* fixture;                 // Fixture instance for this test (optional)
    ANVBenchmarkRegistry* registry;      // Benchmark registry for this test (optional)
    ANVBenchmark* benchmark;             // Current benchmark context (for benchmark tests)
    ANVTestResult* result;               // For internal framework use
    void* user_data;                     // Internal data
} ANVTestCase;

// Benchmark result summary


typedef struct ANVBenchmarkRegistry
{
    ANVBenchmarkResult* results;
    size_t result_count;
    size_t result_capacity;
    ANVAllocator alloc;
} ANVBenchmarkRegistry;

typedef struct ANVBenchmark
{
    const char* name;               // Benchmark name
    uint64_t target_iterations;     // Target number of iterations
    double target_time_seconds;     // Target time to run
    ANVBenchmarkRegistry* registry; // Registry for result submission
    ANVAllocator alloc;             // Allocator for benchmark management

    // Timing functions
    void (*start_timer)(ANVBenchmark* bench);
    void (*stop_timer)(ANVBenchmark* bench);

    // Internal timing state
    void* timing_state;
} ANVBenchmark;

#endif //ANVIL_TESTING_TYPES_H