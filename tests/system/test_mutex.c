/* ==========================================================================
 * test_mutex.c -- Consolidated mutex tests
 *
 * Merged from:
 *   - test_mutex_basic.c   (1 test)
 *   - test_mutex_unit.c    (3 tests)
 *
 * Total: 4 tests
 * ========================================================================== */

#include "system/mutex.h"
#include "system/thread.h"
#include "TestAssert.h"
#include "TestRunner.h"
#include <stdio.h>
#include <stdlib.h>

/* ==========================================================================
 * Helpers -- from test_mutex_basic.c
 * ========================================================================== */

#define BASIC_NUM_THREADS 8
#define BASIC_INCREMENTS  100000

typedef struct
{
    int* counter;
    ANVMutex* m;
} thread_arg_t;

static void* basic_inc_thread(void* arg)
{
    const thread_arg_t* t = (thread_arg_t*)arg;
    for (int i = 0; i < BASIC_INCREMENTS; ++i)
    {
        anv_mutex_lock(t->m);
        ++*(t->counter);
        anv_mutex_unlock(t->m);
    }
    return NULL;
}

/* ==========================================================================
 * Helpers -- from test_mutex_unit.c
 * ========================================================================== */

#define UNIT_NUM_THREADS 4
#define UNIT_INCREMENTS  50000

typedef struct
{
    int* counter;
    ANVMutex* m;
} inc_arg_t;

static void* unit_inc_thread(void* arg)
{
    const inc_arg_t* a = (inc_arg_t*)arg;
    for (int i = 0; i < UNIT_INCREMENTS; ++i)
    {
        anv_mutex_lock(a->m);
        ++*(a->counter);
        anv_mutex_unlock(a->m);
    }
    return NULL;
}

/* ==========================================================================
 * Basic tests (from test_mutex_basic.c)
 * ========================================================================== */

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
    thread_arg_t args[BASIC_NUM_THREADS];

    for (int i = 0; i < BASIC_NUM_THREADS; ++i)
    {
        args[i].counter = &counter;
        args[i].m = &m;
        if (anv_thread_create(&threads[i], basic_inc_thread, &args[i]) != 0)
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

/* ==========================================================================
 * Unit tests (from test_mutex_unit.c)
 * ========================================================================== */

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
    inc_arg_t args[UNIT_NUM_THREADS];

    for (int i = 0; i < UNIT_NUM_THREADS; ++i)
    {
        args[i].counter = &counter;
        args[i].m = &m;
        const int rc = anv_thread_create(&threads[i], unit_inc_thread, &args[i]);
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

/* ==========================================================================
 * Test runner
 * ========================================================================== */

int main(void)
{
    const ANVTestCase tests[] = {
        /* Basic tests (from test_mutex_basic.c) -- 1 test */
        {test_mutex_basic, "test_mutex_basic"},

        /* Unit tests (from test_mutex_unit.c) -- 3 tests */
        {test_mutex_init_destroy, "test_mutex_init_destroy"},
        {test_mutex_trylock_behavior, "test_mutex_trylock_behavior"},
        {test_mutex_threaded_increment, "test_mutex_threaded_increment"},
    };

    return anv_run_tests("mutex", tests, sizeof(tests) / sizeof(tests[0]));
}
