#include "system/thread.h"
#include "system/mutex.h"
#include "TestAssert.h"
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 8
#define INCREMENTS 100000

typedef struct
{
    int* counter;
    ANVMutex* m;
} thread_arg_t;

static void* inc_thread(void* arg)
{
    const thread_arg_t* t = (thread_arg_t*)arg;
    for (int i = 0; i < INCREMENTS; ++i)
    {
        anv_mutex_lock(t->m);
        ++*(t->counter);
        anv_mutex_unlock(t->m);
    }
    return NULL;
}

int test_mutex_basic(void)
{
    int counter = 0;
    ANVMutex m;
    if (anv_mutex_init(&m) != 0)
    {
        fprintf(stderr, "anv_mutex_init failed\n");
        return TEST_FAILURE;
    }

    ANVThread threads[NUM_THREADS];
    thread_arg_t args[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; ++i)
    {
        args[i].counter = &counter;
        args[i].m = &m;
        if (anv_thread_create(&threads[i], inc_thread, &args[i]) != 0)
        {
            fprintf(stderr, "thread_create failed\n");
            anv_mutex_destroy(&m);
            return TEST_FAILURE;
        }
    }

    for (int i = 0; i < NUM_THREADS; ++i)
    {
        anv_thread_join(threads[i], NULL);
    }

    if (counter != NUM_THREADS * INCREMENTS)
    {
        fprintf(stderr, "Counter mismatch: expected %d, got %d\n", NUM_THREADS * INCREMENTS, counter);
        anv_mutex_destroy(&m);
        return TEST_FAILURE;
    }

    anv_mutex_destroy(&m);
    return TEST_SUCCESS;
}

typedef struct
{
    int (*func)(void);
    const char* name;
} TestCase;

int main(void)
{
    TestCase tests[] = {
        {test_mutex_basic, "test_mutex_basic"},
    };

    int failed = 0;
    const int num_tests = sizeof(tests) / sizeof(tests[0]);
    for (int i = 0; i < num_tests; i++)
    {
        if (tests[i].func() != TEST_SUCCESS)
        {
            printf("%s failed\n", tests[i].name);
            failed++;
        }
    }

    if (failed == 0)
    {
        printf("All mutex basic tests passed.\n");
        return 0;
    }

    printf("%d mutex basic tests failed.\n", failed);
    return 1;
}