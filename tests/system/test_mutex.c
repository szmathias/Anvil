#include <stdlib.h>

#include <anvil/testing.h>
#include "system/mutex.h"
#include "system/thread.h"

//==============================================================================
// Static Helpers
//==============================================================================

#define BASIC_NUM_THREADS 8
#define BASIC_INCREMENTS  100000
#define UNIT_NUM_THREADS 4
#define UNIT_INCREMENTS  50000

typedef struct
{
    int* counter;
    ANVMutex* m;
    int increments;
} mutex_thread_arg_t;

static void* mutex_inc_thread(void* arg)
{
    const mutex_thread_arg_t* t = (mutex_thread_arg_t*)arg;
    for (int i = 0; i < t->increments; ++i)
    {
        anv_mutex_lock(t->m);
        ++*t->counter;
        anv_mutex_unlock(t->m);
    }
    return NULL;
}

//==============================================================================
// Basic Tests
//==============================================================================

int test_mutex_basic(void)
{
    int counter = 0;
    ANVMutex m;
    if (anv_mutex_init(&m) != 0)
    {
        fprintf(stderr, "anv_mutex_init failed\n");
        return TEST_FAILURE;
    }

    ANVThread threads[BASIC_NUM_THREADS];
    mutex_thread_arg_t args[BASIC_NUM_THREADS];

    for (int i = 0; i < BASIC_NUM_THREADS; ++i)
    {
        args[i].counter = &counter;
        args[i].m = &m;
        args[i].increments = BASIC_INCREMENTS;
        if (anv_thread_create(&threads[i], mutex_inc_thread, &args[i]) != 0)
        {
            fprintf(stderr, "thread_create failed\n");
            anv_mutex_destroy(&m);
            return TEST_FAILURE;
        }
    }

    for (int i = 0; i < BASIC_NUM_THREADS; ++i)
    {
        anv_thread_join(threads[i], NULL);
    }

    if (counter != BASIC_NUM_THREADS * BASIC_INCREMENTS)
    {
        fprintf(stderr, "Counter mismatch: expected %d, got %d\n", BASIC_NUM_THREADS * BASIC_INCREMENTS, counter);
        anv_mutex_destroy(&m);
        return TEST_FAILURE;
    }

    anv_mutex_destroy(&m);
    return TEST_SUCCESS;
}

//==============================================================================
// Unit Tests
//==============================================================================

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

int test_mutex_threaded_increment(void)
{
    int counter = 0;
    ANVMutex m;
    ASSERT_EQ(anv_mutex_init(&m), 0);

    ANVThread threads[UNIT_NUM_THREADS];
    mutex_thread_arg_t args[UNIT_NUM_THREADS];

    for (int i = 0; i < UNIT_NUM_THREADS; ++i)
    {
        args[i].counter = &counter;
        args[i].m = &m;
        args[i].increments = UNIT_INCREMENTS;
        const int rc = anv_thread_create(&threads[i], mutex_inc_thread, &args[i]);
        ASSERT_EQ(rc, 0);
    }

    for (int i = 0; i < UNIT_NUM_THREADS; ++i)
    {
        ASSERT_EQ(anv_thread_join(threads[i], NULL), 0);
    }

    ASSERT_EQ(counter, UNIT_NUM_THREADS * UNIT_INCREMENTS);

    ASSERT_EQ(anv_mutex_destroy(&m), 0);
    return TEST_SUCCESS;
}

//==============================================================================
// Additional Unit Tests
//==============================================================================

int test_mutex_lock_unlock_cycle(void)
{
    ANVMutex m;
    ASSERT_EQ(anv_mutex_init(&m), 0);

    // Multiple lock/unlock cycles should work
    for (int i = 0; i < 100; i++)
    {
        ASSERT_EQ(anv_mutex_lock(&m), 0);
        ASSERT_EQ(anv_mutex_unlock(&m), 0);
    }

    ASSERT_EQ(anv_mutex_destroy(&m), 0);
    return TEST_SUCCESS;
}

int test_mutex_trylock_after_unlock(void)
{
    ANVMutex m;
    ASSERT_EQ(anv_mutex_init(&m), 0);

    // Lock, unlock, then trylock should succeed
    ASSERT_EQ(anv_mutex_lock(&m), 0);
    ASSERT_EQ(anv_mutex_unlock(&m), 0);

    ASSERT_EQ(anv_mutex_trylock(&m), 0); // Should succeed now
    ASSERT_EQ(anv_mutex_unlock(&m), 0);

    ASSERT_EQ(anv_mutex_destroy(&m), 0);
    return TEST_SUCCESS;
}

#define STRESS_NUM_THREADS 16
#define STRESS_INCREMENTS  100000

int test_mutex_stress_16_threads(void)
{
    int counter = 0;
    ANVMutex m;
    ASSERT_EQ(anv_mutex_init(&m), 0);

    ANVThread threads[STRESS_NUM_THREADS];
    mutex_thread_arg_t args[STRESS_NUM_THREADS];

    for (int i = 0; i < STRESS_NUM_THREADS; ++i)
    {
        args[i].counter = &counter;
        args[i].m = &m;
        args[i].increments = STRESS_INCREMENTS;
        ASSERT_EQ(anv_thread_create(&threads[i], mutex_inc_thread, &args[i]), 0);
    }

    for (int i = 0; i < STRESS_NUM_THREADS; ++i)
    {
        ASSERT_EQ(anv_thread_join(threads[i], NULL), 0);
    }

    ASSERT_EQ(counter, STRESS_NUM_THREADS * STRESS_INCREMENTS);
    ASSERT_EQ(anv_mutex_destroy(&m), 0);
    return TEST_SUCCESS;
}

//==============================================================================
// Main
//==============================================================================

int main(void)
{
    const ANVTestCase tests[] = {
        // Basic tests
        TEST_REGISTER(test_mutex_basic),

        // Unit tests
        TEST_REGISTER(test_mutex_init_destroy),
        TEST_REGISTER(test_mutex_trylock_behavior),
        TEST_REGISTER(test_mutex_threaded_increment),
        TEST_REGISTER(test_mutex_lock_unlock_cycle),
        TEST_REGISTER(test_mutex_trylock_after_unlock),

        // Stress tests
        TEST_REGISTER(test_mutex_stress_16_threads),
    };

    return anv_run_tests("mutex", tests, sizeof(tests) / sizeof(tests[0]));
}