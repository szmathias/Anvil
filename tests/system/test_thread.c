#include <stdlib.h>

#include <anvil/testing.h>
#include "system/thread.h"
#include "system/mutex.h"

//==============================================================================
// Static Helpers
//==============================================================================

static void* simple_return_thread(void* arg)
{
    return arg;
}

static void* add_one_thread(void* arg)
{
    int* val = arg;
    return (void*)(size_t)(*val + 1);
}

typedef struct
{
    ANVMutex* mutex;
    int* shared_counter;
    int increments;
} ThreadStressArg;

static void* stress_thread_func(void* arg)
{
    ThreadStressArg* a = arg;
    for (int i = 0; i < a->increments; i++)
    {
        anv_mutex_lock(a->mutex);
        (*a->shared_counter)++;
        anv_mutex_unlock(a->mutex);
    }
    return NULL;
}

//==============================================================================
// Thread Creation Tests
//==============================================================================

int test_thread_create_join_basic(void)
{
    ANVThread t;
    ASSERT_EQ(anv_thread_create(&t, simple_return_thread, NULL), 0);
    ASSERT_EQ(anv_thread_join(t, NULL), 0);
    return TEST_SUCCESS;
}

int test_thread_return_value(void)
{
    int input = 41;
    ANVThread t;
    ASSERT_EQ(anv_thread_create(&t, add_one_thread, &input), 0);

    void* retval = NULL;
    ASSERT_EQ(anv_thread_join(t, &retval), 0);
    ASSERT_EQ((size_t)retval, 42);
    return TEST_SUCCESS;
}

int test_thread_create_null_params(void)
{
    ANVThread t;
    // NULL thread pointer should fail
    ASSERT_NOT_EQ(anv_thread_create(NULL, simple_return_thread, NULL), 0);
    // NULL func should fail
    ASSERT_NOT_EQ(anv_thread_create(&t, NULL, NULL), 0);
    return TEST_SUCCESS;
}

int test_thread_multiple_create_join(void)
{
    const int NUM_THREADS = 16;
    ANVThread threads[16];

    for (int i = 0; i < NUM_THREADS; i++)
    {
        ASSERT_EQ(anv_thread_create(&threads[i], simple_return_thread, NULL), 0);
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        ASSERT_EQ(anv_thread_join(threads[i], NULL), 0);
    }
    return TEST_SUCCESS;
}

//==============================================================================
// Thread Detach Tests
//==============================================================================

static void* detach_worker(void* arg)
{
    (void)arg;
    // Quick work, then exit
    volatile int dummy = 0;
    for (int i = 0; i < 100; i++)
    {
        dummy += i;
    }
    return NULL;
}

int test_thread_detach_basic(void)
{
    ANVThread t;
    ASSERT_EQ(anv_thread_create(&t, detach_worker, NULL), 0);
    ASSERT_EQ(anv_thread_detach(t), 0);
    // After detach, we cannot join. Sleep briefly to let it complete.
    // (No portable sleep in C11, so just do some busy work)
    volatile int dummy = 0;
    for (int i = 0; i < 1000000; i++)
    {
        dummy += i;
    }
    return TEST_SUCCESS;
}

//==============================================================================
// Thread Stress Tests
//==============================================================================

int test_thread_stress_shared_counter(void)
{
    const int NUM_THREADS = 16;
    const int INCREMENTS = 100000;

    int counter = 0;
    ANVMutex m;
    ASSERT_EQ(anv_mutex_init(&m), 0);

    ANVThread threads[16];
    ThreadStressArg args[16];

    for (int i = 0; i < NUM_THREADS; i++)
    {
        args[i].mutex = &m;
        args[i].shared_counter = &counter;
        args[i].increments = INCREMENTS;
        ASSERT_EQ(anv_thread_create(&threads[i], stress_thread_func, &args[i]), 0);
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        ASSERT_EQ(anv_thread_join(threads[i], NULL), 0);
    }

    ASSERT_EQ(counter, NUM_THREADS * INCREMENTS);
    ASSERT_EQ(anv_mutex_destroy(&m), 0);
    return TEST_SUCCESS;
}

//==============================================================================
// Main
//==============================================================================

int main(void)
{
    const ANVTestCase tests[] = {
        // Creation
        TEST_REGISTER(test_thread_create_join_basic),
        TEST_REGISTER(test_thread_return_value),
        TEST_REGISTER(test_thread_create_null_params),
        TEST_REGISTER(test_thread_multiple_create_join),

        // Detach
        TEST_REGISTER(test_thread_detach_basic),

        // Stress
        TEST_REGISTER(test_thread_stress_shared_counter),
    };

    return anv_run_tests("Thread", tests, sizeof(tests) / sizeof(tests[0]));
}

