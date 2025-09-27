//
// Created by zack on 9/3/25.
//

#include "containers/dynamicstring.h"
#include "TestAssert.h"
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#define NUM_FUZZ_OPERATIONS 100000

// Simple cross-platform PRNG using C standard library
static void fuzz_seed(void)
{
    srand((unsigned int)time(NULL));
}

static unsigned fuzz_rand_range(const unsigned bound)
{
    if (bound == 0)
        return 0u;
    return (unsigned)(rand() % bound);
}

static size_t fuzz_rand_size(size_t bound)
{
    if (bound == 0)
        return 0;
    return (size_t)(rand() % (bound));
}

static void ensure_seeded(void)
{
    static int seeded = 0;
    if (!seeded)
    {
        fuzz_seed();
        seeded = 1;
    }
}

static void perform_random_operation(ANVString* str)
{
    const int op = fuzz_rand_range(8);

    switch (op)
    {
        case 0:                                                       // push_back
            anv_str_push_back(str, (char)(fuzz_rand_range(95) + 32)); // Printable ASCII
            break;
        case 1: // pop_back
            if (!anv_str_empty(str))
            {
                anv_str_pop_back(str);
            }
            break;
        case 2: // insert
            if (anv_str_size(str) > 0)
            {
                anv_str_insert_char(str, fuzz_rand_size(anv_str_size(str)), 'X');
            }
            break;
        case 3: // erase
            if (anv_str_size(str) > 0)
            {
                anv_str_erase(str, fuzz_rand_size(anv_str_size(str)));
            }
            break;
        case 4: // assign
            anv_str_assign_cstring(str, "fuzz");
            break;
        case 5: // clear
            anv_str_clear(str);
            break;
        case 6: // trim
            anv_str_trim_front(str);
            anv_str_trim_back(str);
            break;
        case 7: // reserve
            anv_str_reserve(str, (size_t)fuzz_rand_range(256));
            break;

        default:
            break;
    }
}

int test_string_fuzz(void)
{
    ensure_seeded();
    ANVString str = anv_str_create_empty(0);

    for (int i = 0; i < NUM_FUZZ_OPERATIONS; ++i)
    {
        perform_random_operation(&str);
    }

    anv_str_destroy(&str);
    printf("DString fuzz test completed %d operations without crashing.\n", NUM_FUZZ_OPERATIONS);
    return TEST_SUCCESS;
}

int main(void)
{
    return test_string_fuzz() ? 0 : 1;
}