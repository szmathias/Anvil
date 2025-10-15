//
// Created by zack on 9/27/25.
//

#include "anvil/testing/benchmark.h"

#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

typedef struct ANVBenchmarkTimingState
{
    const char* name;
    uint64_t iterations;
    uint64_t start_time;
    uint64_t total_time;
    uint64_t min_time;
    uint64_t max_time;
    double mean;
    double m2;
    bool timer_running;

    ANVArrayList* samples;
    uint64_t* sample_array;
    uint64_t sample_count;
} ANVBenchmarkTimingState;

typedef struct ANVBenchmarkAggregateResult
{
    const char* name;             // Operation name
    uint64_t total_runs;          // Number of benchmark runs
    uint64_t total_iterations;    // Total iterations across all runs
    uint64_t min_time;            // Minimum time across all runs
    uint64_t max_time;            // Maximum time across all runs
    uint64_t total_time;          // Total time across all runs (in ns)
    double mean;                  // Overall mean
    double trimmed_mean;          // Mean with top/bottom 1% removed
    double m2;                    // For variance calculation

    double coefficient_of_variation;  // (std_dev / mean) * 100
    uint64_t p50;   // Median
    uint64_t p95;   // 95th percentile
    uint64_t p99;   // 99th percentile
    uint64_t p999;  // 99.9th percentile

    ANVArrayList* all_samples;
} ANVBenchmarkAggregateResult;

ANV_API ANVBenchmark* anv_benchmark_create(ANVAllocator* alloc, const char* name, const uint64_t iterations)
{
    if (!alloc || !name || iterations == 0)
    {
        return NULL;
    }

    ANVBenchmark *bench = anv_alloc_allocate(alloc, sizeof(ANVBenchmark));
    if (!bench)
    {
        return NULL;
    }

    ANVArrayList *timers = anv_arraylist_create(alloc, ANV_DEFAULT_CAPACITY);
    if (!timers)
    {
        anv_alloc_deallocate(alloc, bench);
        return NULL;
    }

    bench->timers = timers;
    bench->current_timer_index = 0;
    bench->warmup_iterations = 0;
    bench->aggregate_results = NULL;
    bench->verbose = false;

    bench->name = name;
    bench->target_iterations = iterations;
    bench->target_time_seconds = 0.0;
    bench->alloc = *alloc;
    bench->start_timer = anv_benchmark_start_timer;
    bench->stop_timer = anv_benchmark_stop_timer;

    return bench;
}

ANV_API ANVBenchmark* anv_benchmark_create_timed(ANVAllocator* alloc, const char* name, const double seconds)
{
    if (!alloc || !name || seconds <= 0.0)
    {
        return NULL;
    }

    ANVBenchmark *bench = anv_alloc_allocate(alloc, sizeof(ANVBenchmark));
    if (!bench)
    {
        return NULL;
    }

    ANVArrayList *timers = anv_arraylist_create(alloc, ANV_DEFAULT_CAPACITY);
    if (!timers)
    {
        anv_alloc_deallocate(alloc, bench);
        return NULL;
    }
    bench->timers = timers;
    bench->current_timer_index = 0;
    bench->warmup_iterations = 0;
    bench->aggregate_results = NULL;
    bench->verbose = false;

    bench->name = name;
    bench->target_iterations = 0;
    bench->target_time_seconds = seconds;
    bench->alloc = *alloc;
    bench->start_timer = anv_benchmark_start_timer;
    bench->stop_timer = anv_benchmark_stop_timer;

    return bench;
}

ANV_API void anv_benchmark_destroy(ANVBenchmark* bench)
{
    if (!bench || !bench->timers)
    {
        return;
    }

    if (bench->timers)
    {
        for (size_t i = 0; i < anv_arraylist_size(bench->timers); i++)
        {
            ANVBenchmarkTimingState* timing = anv_arraylist_get(bench->timers, i);
            if (timing)
            {
                if (timing->sample_array)
                {
                    anv_alloc_deallocate(&bench->alloc, timing->sample_array);
                    timing->sample_array = NULL;
                }
            }
        }
        anv_arraylist_destroy(bench->timers, true);
        bench->timers = NULL;
    }

    if (bench->aggregate_results)
    {
        for (size_t i = 0; i < anv_arraylist_size(bench->aggregate_results); i++)
        {
            ANVBenchmarkAggregateResult* agg = anv_arraylist_get(bench->aggregate_results, i);
            if (agg && agg->all_samples)
            {
                anv_arraylist_destroy(agg->all_samples, false);
                agg->all_samples = NULL;
            }
        }
        anv_arraylist_destroy(bench->aggregate_results, true);
        bench->aggregate_results = NULL;
    }

    anv_alloc_deallocate(&bench->alloc, bench);
}

ANV_API void anv_benchmark_run(ANVBenchmark* bench, void (*test_func)(ANVBenchmark* bench))
{
    if (bench && test_func)
    {
        test_func(bench);
    }
}

ANV_API void anv_benchmark_run_warmup(ANVBenchmark* bench, void (*test_func)(ANVBenchmark*))
{
    if (!bench || !test_func || bench->warmup_iterations == 0)
    {
        return;
    }

    const uint64_t original_iterations = bench->target_iterations;
    const double original_time = bench->target_time_seconds;
    const size_t original_timer_index = bench->current_timer_index;

    // Configure for warmup
    bench->target_iterations = bench->warmup_iterations;
    bench->target_time_seconds = 0;

    const size_t original_size = anv_arraylist_size(bench->timers);

    test_func(bench);

    bench->target_iterations = original_iterations;
    bench->target_time_seconds = original_time;
    bench->current_timer_index = original_timer_index;

    anv_arraylist_resize(bench->timers, original_size, NULL, true);
}

// Comparison function for qsort
static int compare_uint64(const void* a, const void* b)
{
    const uint64_t ua = *(const uint64_t*)a;
    const uint64_t ub = *(const uint64_t*)b;
    if (ua < ub) return -1;
    if (ua > ub) return 1;
    return 0;
}

static uint64_t calculate_percentile(const ANVArrayList* sorted_samples, const double percentile)
{
    const size_t n = anv_arraylist_size(sorted_samples);
    if (n == 0) return 0;

    const double index = (percentile / 100.0) * (double)(n - 1);
    const size_t lower = (size_t)floor(index);
    const size_t upper = (size_t)ceil(index);

    if (lower == upper)
    {
        const uint64_t* sample = anv_arraylist_get(sorted_samples, lower);
        return sample ? *sample : 0;
    }

    // Linear interpolation
    const uint64_t* lower_val = anv_arraylist_get(sorted_samples, lower);
    const uint64_t* upper_val = anv_arraylist_get(sorted_samples, upper);

    if (!lower_val || !upper_val) return 0;

    const double weight = index - (double)lower;
    return (uint64_t)((double)*lower_val + weight * (double)(*upper_val - *lower_val));
}

static double calculate_trimmed_mean(const ANVArrayList* sorted_samples, const double trim_percent)
{
    const size_t n = anv_arraylist_size(sorted_samples);
    if (n == 0) return 0.0;

    const size_t trim_count = (size_t)(trim_percent / 100.0 * (double)n);

    if (trim_count * 2 >= n) return 0.0;

    uint64_t sum = 0;
    size_t count = 0;

    for (size_t i = trim_count; i < n - trim_count; i++)
    {
        const uint64_t* sample = anv_arraylist_get(sorted_samples, i);
        if (sample)
        {
            sum += *sample;
            count++;
        }
    }

    return count > 0 ? (double)sum / (double)count : 0.0;
}

ANV_API void anv_benchmark_run_multiple(ANVBenchmark* bench, void (*test_func)(ANVBenchmark* bench), const uint32_t runs)
{
    if (!bench || !test_func || runs == 0)
    {
        return;
    }

    const size_t starting_timer_count = bench->current_timer_index;
    bench->runs = runs;
    bench->aggregate_results = anv_arraylist_create(&bench->alloc, ANV_DEFAULT_CAPACITY);
    if (!bench->aggregate_results)
    {
        return;
    }

    if (bench->verbose)
    {
        printf("Starting benchmark: %s (%" PRIu32 " runs)\n", bench->name ? bench->name : "unnamed", runs);
    }

    for (uint32_t run = 0; run < runs; run++)
    {
        if (bench->verbose)
        {
            printf("  Run %u/%" PRIu32 "...\n", run + 1, runs);
        }
        test_func(bench);
    }

    if (bench->verbose)
    {
        printf("Finished benchmark: %s\n", bench->name ? bench->name : "unnamed");
        printf("Aggregating results...");
        fflush(stdout);
    }

    const size_t total_timers = bench->current_timer_index;
    const size_t timers_per_run = (total_timers - starting_timer_count) / runs;

    for (size_t timer_index = 0; timer_index < timers_per_run; timer_index++)
    {
        ANVBenchmarkAggregateResult* agg = anv_alloc_allocate(&bench->alloc,
                                                              sizeof(ANVBenchmarkAggregateResult));
        if (!agg)
        {
            continue;
        }

        const size_t first_timer_offset = starting_timer_count + timer_index;
        const ANVBenchmarkTimingState* first_timing = anv_arraylist_get(bench->timers, first_timer_offset);

        agg->name = first_timing->name;
        agg->total_runs = 1;
        agg->total_iterations = first_timing->iterations;
        agg->min_time = first_timing->min_time;
        agg->max_time = first_timing->max_time;
        agg->total_time = first_timing->total_time;
        agg->mean = first_timing->mean;
        agg->m2 = first_timing->m2;

        agg->all_samples = anv_arraylist_create(&bench->alloc, agg->total_iterations * runs);

        for (uint32_t run = 1; run < runs; run++)
        {
            const size_t timer_offset = starting_timer_count + (run * timers_per_run) + timer_index;
            const ANVBenchmarkTimingState* timing = anv_arraylist_get(bench->timers, timer_offset);

            if (!timing)
            {
                continue;
            }

            if (timing->sample_array)
            {
                for (size_t i = 0; i < timing->sample_count; i++)
                {
                    uint64_t* sample = &timing->sample_array[i];
                    anv_arraylist_push_back(agg->all_samples, sample);
                }
            }

            agg->total_runs++;
            agg->total_iterations += timing->iterations;

            if (timing->min_time < agg->min_time)
            {
                agg->min_time = timing->min_time;
            }
            if (timing->max_time > agg->max_time)
            {
                agg->max_time = timing->max_time;
            }

            agg->total_time += timing->total_time;

            // Welford's algorithm for combining batches
            const double delta = timing->mean - agg->mean;
            const uint64_t n1 = agg->total_iterations - timing->iterations;
            const uint64_t n2 = timing->iterations;

            agg->mean += delta * ((double)n2 / (double)agg->total_iterations);
            const double delta2 = timing->mean - agg->mean;
            agg->m2 += timing->m2 + delta * delta2 * (double)n1 * (double)n2 / (double)agg->total_iterations;
        }

        anv_arraylist_sort(agg->all_samples, compare_uint64);

        agg->p50 = calculate_percentile(agg->all_samples, 50.0);
        agg->p95 = calculate_percentile(agg->all_samples, 95.0);
        agg->p99 = calculate_percentile(agg->all_samples, 99.0);
        agg->p999 = calculate_percentile(agg->all_samples, 99.9);
        agg->trimmed_mean = calculate_trimmed_mean(agg->all_samples, 1.0);

        // Calculate coefficient of variation
        const double std_dev = sqrt(agg->m2 / (double)(agg->total_iterations - 1));
        agg->coefficient_of_variation = (agg->mean > 0) ? (std_dev / agg->mean) * 100.0 : 0.0;

        anv_arraylist_push_back(bench->aggregate_results, agg);
    }

    if (bench->verbose)
    {
        printf(" Done.\n");
    }
}

ANV_API void anv_benchmark_set_warmup(ANVBenchmark* bench, const uint64_t iterations)
{
    if (bench)
    {
        bench->warmup_iterations = iterations;
    }
}

ANV_API void anv_benchmark_set_verbose(ANVBenchmark* bench, const bool verbose)
{
    if (bench)
    {
        bench->verbose = verbose;
    }
}

ANV_API void anv_benchmark_start_timer(ANVBenchmark* bench)
{
    if (!bench)
    {
        return;
    }

    ANVBenchmarkTimingState* timing = NULL;

    if (bench->current_timer_index < anv_arraylist_size(bench->timers))
    {
        timing = anv_arraylist_get(bench->timers, bench->current_timer_index);
    }
    else
    {
        timing = anv_alloc_allocate(&bench->alloc, sizeof(ANVBenchmarkTimingState));
        memset(timing, 0, sizeof(ANVBenchmarkTimingState));
        if (!timing)
        {
            return;
        }

        timing->sample_array = anv_alloc_allocate(&bench->alloc, sizeof(uint64_t) * bench->target_iterations);
        if (!timing->sample_array)
        {
            anv_alloc_deallocate(&bench->alloc, timing);
            return;
        }

        anv_arraylist_push_back(bench->timers, timing);
    }

    if (timing->timer_running)
    {
        return;
    }

    timing->timer_running = true;
    timing->start_time = anv_time_get_ns();
}

ANV_API void anv_benchmark_stop_timer(ANVBenchmark* bench)
{
    if (!bench)
    {
        return;
    }

    const uint64_t end_time = anv_time_get_ns();

    ANVBenchmarkTimingState* timing = anv_arraylist_get(bench->timers, bench->current_timer_index);
    if (!timing || !timing->timer_running)
    {
        return;
    }

    const uint64_t duration = end_time - timing->start_time;

    if (timing->sample_array)
    {
        timing->sample_array[timing->sample_count] = duration;
        timing->sample_count++;
    }

    timing->timer_running = false;
    timing->total_time += duration;
    timing->iterations++;

    if (timing->iterations == 1 || duration < timing->min_time)
    {
        timing->min_time = duration;
    }

    if (timing->iterations == 1 || duration > timing->max_time)
    {
        timing->max_time = duration;
    }

    // Welford's online algorithm for variance
    const double delta = (double)duration - timing->mean;
    timing->mean += delta / (double)timing->iterations;
    const double delta2 = (double)duration - timing->mean;
    timing->m2 += delta * delta2;
}

ANV_API void anv_benchmark_submit_timing(ANVBenchmark* bench, const char *operation_name)
{
    if (!bench)
    {
        return;
    }
    ANVBenchmarkTimingState* timing = anv_arraylist_get(bench->timers, bench->current_timer_index);
    timing->name = operation_name;

    bench->current_timer_index++;
}

ANV_API void anv_benchmark_print_result(ANVBenchmark* bench)
{
    anv_benchmark_print_result_units(bench, ANV_TIME_NANOSECONDS);
}

ANV_API void anv_benchmark_print_result_units(ANVBenchmark* bench, ANVTime time_unit)
{
    static const char* time_unit_strings[ANV_TIME_COUNT] = {
        "ns", "μs", "ms", "s"
    };

    if (time_unit >= ANV_TIME_COUNT)
    {
        time_unit = ANV_TIME_NANOSECONDS;
    }

    if (!bench || !bench->timers)
    {
        return;
    }

    const char* time_unit_string = time_unit_strings[time_unit];

    printf("================================================================================\n");
    printf("Benchmark: %s\n", bench->name ? bench->name : "unnamed");
    printf("================================================================================\n\n");

    for (uint64_t i = 0; i < bench->current_timer_index; i++)
    {
        const ANVBenchmarkTimingState* timing = anv_arraylist_get(bench->timers, i);
        if (timing->iterations == 0)
        {
            continue;
        }
        const double total_time = anv_time_convert(timing->total_time, time_unit);
        const double avg_time = total_time / (double)timing->iterations;
        const double min_time = anv_time_convert(timing->min_time, time_unit);
        const double max_time = anv_time_convert(timing->max_time, time_unit);
        const double ops_per_second = avg_time > 0 ? 1000000000.0 / ((double)timing->total_time / (double)timing->iterations) : 0.0;

        double std_dev = 0.0;
        if (timing->iterations > 1) {
            std_dev = sqrt(timing->m2 / (double)(timing->iterations - 1));
            std_dev = anv_time_convert((uint64_t)std_dev, time_unit);
        }

        printf("\n%s\n", timing->name ? timing->name : "unnamed");
        printf("  Iterations:    %-12" PRIu64 "\n", timing->iterations);
        printf("  Avg time:          %10.3f %s/op\n", avg_time, time_unit_string);
        printf("  Min time:          %10.3f %s/op\n", min_time, time_unit_string);
        printf("  Max time:          %10.3f %s/op\n", max_time, time_unit_string);
        printf("  Std dev:           %10.3f %s/op\n", std_dev, time_unit_string);
        printf("  Throughput:     %12.2f ops/sec\n", ops_per_second);

        if (i + 1 < bench->current_timer_index)
        {
            printf("\n");
        }
    }
    printf("\n");
}

ANV_API void anv_benchmark_print_aggregate_results(ANVBenchmark* bench, const ANVTime time_unit)
{
    if (!bench || !bench->aggregate_results)
    {
        return;
    }

    static const char* time_unit_strings[ANV_TIME_COUNT] = {
        "ns", "μs", "ms", "s"
    };

    const char* time_unit_string = time_unit_strings[time_unit];
    const size_t timers_per_run = anv_arraylist_size(bench->aggregate_results);

    if (timers_per_run == 0 || bench->runs == 0)
    {
        return;
    }

    printf("================================================================================\n");
    printf("Benchmark: %s\n", bench->name ? bench->name : "unnamed");
    printf("================================================================================\n\n");

    if (!bench->verbose)
    {
        printf("Run Summary:\n");
        for (size_t run = 0; run < bench->runs; run++)
        {
            printf("  Run %zu/%zu:\n", run + 1, bench->runs);

            for (size_t timer_index = 0; timer_index < timers_per_run; timer_index++)
            {
                const size_t timer_offset = (run * timers_per_run) + timer_index;
                const ANVBenchmarkTimingState* timing = anv_arraylist_get(bench->timers, timer_offset);

                if (!timing || timing->iterations == 0)
                {
                    continue;
                }

                const double avg_time = anv_time_convert(timing->total_time / timing->iterations, time_unit);

                printf("  \t%s: %.3f %s\n",
                       timing->name ? timing->name : "unnamed",
                       avg_time,
                       time_unit_string);
            }
            printf("\n");
        }
        printf("\n");
    }
    else
    {
        for (size_t run = 0; run < bench->runs; run++)
        {
            printf("================================== Run %zu/%zu ==================================\n",
                   run + 1, bench->runs);

            for (size_t timer_index = 0; timer_index < timers_per_run; timer_index++)
            {
                const size_t timer_offset = run * timers_per_run + timer_index;
                const ANVBenchmarkTimingState* timing = anv_arraylist_get(bench->timers, timer_offset);

                if (!timing || timing->iterations == 0)
                {
                    continue;
                }

                const double total_time = anv_time_convert(timing->total_time, time_unit);
                const double avg_time = total_time / (double)timing->iterations;
                const double min_time = anv_time_convert(timing->min_time, time_unit);
                const double max_time = anv_time_convert(timing->max_time, time_unit);
                const double ops_per_second = avg_time > 0 ?
                    1000000000.0 / ((double)timing->total_time / (double)timing->iterations) : 0.0;

                double std_dev = 0.0;
                if (timing->iterations > 1)
                {
                    std_dev = sqrt(timing->m2 / (double)(timing->iterations - 1));
                    std_dev = anv_time_convert((uint64_t)std_dev, time_unit);
                }

                printf("\n%s\n", timing->name ? timing->name : "unnamed");
                printf("  Iterations:    %-12" PRIu64 "\n", timing->iterations);
                printf("  Avg time:          %10.3f %s/op\n", avg_time, time_unit_string);
                printf("  Min time:          %10.3f %s/op\n", min_time, time_unit_string);
                printf("  Max time:          %10.3f %s/op\n", max_time, time_unit_string);
                printf("  Std dev:           %10.3f %s/op\n", std_dev, time_unit_string);
                printf("  Throughput:     %12.2f ops/sec\n", ops_per_second);
            }

            printf("\n");
        }
    }

    printf("================================================================================\n");
    printf("AGGREGATE RESULTS (%zu runs)\n", bench->runs);
    printf("================================================================================\n\n");

    for (size_t i = 0; i < anv_arraylist_size(bench->aggregate_results); i++)
    {
        ANVBenchmarkAggregateResult* agg = anv_arraylist_get(bench->aggregate_results, i);
        if (!agg)
        {
            continue;
        }

        const double avg_time = anv_time_convert(agg->total_time / agg->total_iterations, time_unit);
        const double trimmed_avg = anv_time_convert((uint64_t)agg->trimmed_mean, time_unit);
        const double min_time = anv_time_convert(agg->min_time, time_unit);
        const double max_time = anv_time_convert(agg->max_time, time_unit);
        const double ops_per_second = 1000000000.0 / ((double)agg->total_time / (double)agg->total_iterations);

        double std_dev = 0.0;
        if (agg->total_iterations > 1)
        {
            std_dev = sqrt(agg->m2 / ((double)agg->total_iterations - 1));
            std_dev = anv_time_convert((uint64_t)std_dev, time_unit);
        }

        const double p50 = anv_time_convert(agg->p50, time_unit);
        const double p95 = anv_time_convert(agg->p95, time_unit);
        const double p99 = anv_time_convert(agg->p99, time_unit);
        const double p999 = anv_time_convert(agg->p999, time_unit);

        const char* name = agg->name ? agg->name : "unnamed";
        const size_t name_len = strlen(name);
        const size_t box_width = 65;

        printf("┌─ %s ", name);
        for (size_t j = 0; j < (box_width > name_len ? box_width - name_len - 5 : 0); j++)
        {
            printf("─");
        }
        printf("┐\n");
        printf("  Total iterations: %-12" PRIu64 "\n", agg->total_iterations);
        printf("\n");
        printf("  Central Tendency:\n");
        printf("    Mean             %10.3f %s/op\n", avg_time, time_unit_string);
        printf("    Trimmed mean     %10.3f %s/op  (1%% trim)\n", trimmed_avg, time_unit_string);
        printf("    Median (P50)     %10.3f %s/op\n", p50, time_unit_string);
        printf("\n");
        printf("  Spread\n");
        printf("    Min              %10.3f %s/op\n", min_time, time_unit_string);
        printf("    Max              %10.3f %s/op\n", max_time, time_unit_string);
        printf("    Std dev          %10.3f %s/op\n", std_dev, time_unit_string);
        printf("    CV              %10.2f%%\n", agg->coefficient_of_variation);
        printf("\n");
        printf("  Percentiles:\n");
        printf("    P95              %10.3f %s/op\n", p95, time_unit_string);
        printf("    P99              %10.3f %s/op\n", p99, time_unit_string);
        printf("    P99.9            %10.3f %s/op\n", p999, time_unit_string);
        printf("\n");
        printf("  Throughput:     %12.2f ops/sec\n", ops_per_second);
        printf("└───────────────────────────────────────────────────────────────┘\n");

        if (i + 1 < anv_arraylist_size(bench->aggregate_results))
        {
            printf("\n");
        }
    }
}