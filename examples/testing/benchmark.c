//
// Created by zack on 9/27/25.
//

#include "anvil/testing/benchmark.h"

static void test_benchmark_warmup_arraylist(ANVBenchmark* bench)
{
    ANVArrayList* warmup_list = anv_arraylist_create(&bench->alloc, ANV_DEFAULT_CAPACITY);
    for (uint64_t i = 0; i < bench->warmup_iterations; i++) {
        uint64_t* temp = anv_alloc_allocate(&bench->alloc, sizeof(uint64_t));
        if (temp) *temp = i;
        anv_arraylist_push_back(warmup_list, temp);
    }
    anv_arraylist_destroy(warmup_list, true);
}

static void test_benchmark_functions(ANVBenchmark* bench)
{
    if (!bench)
    {
        return;
    }

    ANVArrayList* list = NULL;
    {
        ANVAllocator allocator = anv_alloc_default();
        list = anv_arraylist_create(&allocator, ANV_DEFAULT_CAPACITY);
    }

    anv_arraylist_reserve(list, bench->target_iterations);
    anv_benchmark_run_warmup(bench, test_benchmark_warmup_arraylist);
    for (uint64_t i = 0; i < bench->target_iterations; i++)
    {
        uint64_t* temp = anv_alloc_allocate(&bench->alloc, sizeof(uint64_t));
        if (temp)
        {
            *temp = i;
        }

        bench->start_timer(bench);
        anv_arraylist_push_back(list, temp);
        bench->stop_timer(bench);
    }
    anv_benchmark_submit_timing(bench, "Arraylist push back new elements");

    anv_benchmark_run_warmup(bench, test_benchmark_warmup_arraylist);
    volatile uint64_t count = 0;
    ANV_BENCHMARK_LOOP(bench, i)
    {
        ANV_BENCHMARK_START_TIMING(bench);
        const uint64_t* val = anv_arraylist_get(list, i);
        ANV_BENCHMARK_STOP_TIMING(bench);
        if (val && *val == i)
        {
            count++;
            anv_alloc_deallocate(&bench->alloc, (void*)val);
            val = NULL;
        }
    }
    ANV_BENCHMARK_SUBMIT_TIMING(bench, "Arraylist get elements by index");

    ANV_BENCHMARK_START_TIMING(bench);
    anv_arraylist_destroy(list, false);
    ANV_BENCHMARK_STOP_TIMING(bench);
    ANV_BENCHMARK_SUBMIT_TIMING(bench, "Arraylist destroy");
}

int main(void)
{
    ANVAllocator alloc = anv_alloc_default();
    ANVBenchmark* bench = anv_benchmark_create(&alloc, "Individual tests", 1000000);
    if (!bench)
    {
        return -1;
    }
    anv_benchmark_set_warmup(bench, 1000);
    bench->verbose = true;

    anv_benchmark_run_multiple(bench, test_benchmark_functions, 5);

    anv_benchmark_print_aggregate_results(bench, ANV_TIME_MICROSECONDS);

    anv_benchmark_destroy(bench);
    return 0;
}
