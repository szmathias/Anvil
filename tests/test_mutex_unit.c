#include "TestAssert.h"
#include "system/mutex.h"
#include "system/thread.h"
#include <stdio.h>
#include <stdlib.h>

int test_mutex_init_destroy(void)
{
    ANVMutex m;
    ASSERT_EQ(anv_mutex_init(&m), 0);
    ASSERT_EQ(anv_mutex_destroy(&m), 0);
    return TEST_SUCCESS;
}

int test_mutex_trylock_behavior(void)
{
    ANVMutex m;
    ASSERT_EQ(anv_mutex_init(&m), 0);

    // Lock the mutex
    ASSERT_EQ(anv_mutex_lock(&m), 0);

    // Try-lock should fail (non-zero) for non-recursive mutexes; accept any non-zero as "would block/error"
    const int rc = anv_mutex_trylock(&m);
    ASSERT(rc != 0);

    ASSERT_EQ(anv_mutex_unlock(&m), 0);
    ASSERT_EQ(anv_mutex_destroy(&m), 0);
    return TEST_SUCCESS;
}

#define NUM_THREADS 4
#define INCREMENTS 50000

typedef struct
{
    int* counter;
    ANVMutex* m;
} inc_arg_t;

static void* inc_thread(void* arg)
{
    const inc_arg_t* a = (inc_arg_t*)arg;
    for (int i = 0; i < INCREMENTS; ++i)
    {
        anv_mutex_lock(a->m);
        ++*(a->counter);
        anv_mutex_unlock(a->m);
    }
    return NULL;
}

int test_mutex_threaded_increment(void)
{
    int counter = 0;
    ANVMutex m;
    ASSERT_EQ(anv_mutex_init(&m), 0);

    ANVThread threads[NUM_THREADS];
    inc_arg_t args[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; ++i)
    {
        args[i].counter = &counter;
        args[i].m = &m;
        const int rc = anv_thread_create(&threads[i], inc_thread, &args[i]);
        ASSERT_EQ(rc, 0);
    }

    for (int i = 0; i < NUM_THREADS; ++i)
    {
        ASSERT_EQ(anv_thread_join(threads[i], NULL), 0);
    }

    ASSERT_EQ(counter, NUM_THREADS * INCREMENTS);

    ASSERT_EQ(anv_mutex_destroy(&m), 0);
    return TEST_SUCCESS;
}

typedef struct
{
    int (*func)(void);
    const char* name;
} TestCase;

int main(void)
{
    const TestCase tests[] = {
        {test_mutex_init_destroy, "test_mutex_init_destroy"},
        {test_mutex_trylock_behavior, "test_mutex_trylock_behavior"},
        {test_mutex_threaded_increment, "test_mutex_threaded_increment"},
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
        printf("All mutex unit tests passed.\n");
        return 0;
    }

    printf("%d mutex unit tests failed.\n", failed);
    return 1;
}