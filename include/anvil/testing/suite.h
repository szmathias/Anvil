//
// Created by zack on 9/26/25.
//

#ifndef ANVIL_SUITE_H
#define ANVIL_SUITE_H

#include "types.h"
#include "common/common.h"

#ifdef __cplusplus
extern "C" {
#endif

// Test function signatures
typedef void (*ANVTestFunction)(ANVTestCase* test);

typedef struct ANVTestSuite ANVTestSuite;

//==============================================================================
// Suite Management
//==============================================================================

ANV_API ANVTestSuite* anv_suite_create(const char* name);
ANV_API void anv_suite_destroy(ANVTestSuite* suite);

ANV_API void anv_suite_set_allocator(ANVTestSuite* suite, const ANVAllocator* allocator);
ANV_API void anv_suite_set_fixture(ANVTestSuite* suite, ANVFixture* fixture_template);
ANV_API void anv_suite_set_verbose(ANVTestSuite* suite, bool verbose);
ANV_API void anv_suite_set_benchmark_registry(ANVTestSuite* suite, ANVBenchmarkRegistry* registry);

//==============================================================================
// Test Management
//==============================================================================

ANV_API ANVResult anv_suite_add(ANVTestSuite* suite, const char* test_name, ANVTestFunction test_func);

ANV_API ANVResult anv_suite_add_with_fixture(ANVTestSuite* suite, const char* test_name,
                                            ANVTestFunction test_func, ANVFixture* fixture);

ANV_API ANVResult anv_suite_add_with_allocator(ANVTestSuite* suite, const char* test_name,
                                              ANVTestFunction test_func, const ANVAllocator* allocator);

ANV_API ANVResult anv_suite_add_with_registry(ANVTestSuite* suite, const char* test_name,
                                             ANVTestFunction test_func, ANVBenchmarkRegistry* registry);

ANV_API ANVResult anv_suite_add_with_config(ANVTestSuite* suite, const char* test_name,
                                           ANVTestFunction test_func, ANVFixture* fixture,
                                           const ANVAllocator* allocator, ANVBenchmarkRegistry* registry);

//==============================================================================
// Benchmark Integration
//==============================================================================

// ANV_API ANVResult anv_suite_add_benchmark(ANVTestSuite* suite, const char* name,
//                                          ANVTestFunction benchmark_func);
//
// ANV_API ANVResult anv_suite_add_benchmark_with_iterations(ANVTestSuite* suite, const char* name,
//                                                          ANVTestFunction benchmark_func,
//                                                          uint64_t iterations);
//
// ANV_API ANVResult anv_suite_add_benchmark_with_time(ANVTestSuite* suite, const char* name,
//                                                     ANVTestFunction benchmark_func,
//                                                     double target_seconds);

//==============================================================================
// Test Execution
//==============================================================================

ANV_API int anv_suite_run(ANVTestSuite* suite);
ANV_API ANVTestResult anv_suite_get_result(const ANVTestSuite* suite);

ANV_API ANVBenchmarkResult* anv_suite_get_benchmark_results(const ANVTestSuite* suite);
ANV_API size_t anv_suite_get_benchmark_count(const ANVTestSuite* suite);

ANV_API void anv_test_print_summary(const ANVTestResult* result);
ANV_API ANVBenchmark* anv_test_get_benchmark(const ANVTestCase* test);

#ifdef __cplusplus
}
#endif

#endif // ANVIL_SUITE_H
