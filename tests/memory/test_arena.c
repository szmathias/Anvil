#include <anvil/testing.h>
#include "memory/arena.h"

//==============================================================================
// CRUD Tests
//==============================================================================

int test_arena_create_destroy(void)
{
    ANVArena arena = anv_arena_create(1024);
    ASSERT_NOT_NULL(arena.memory);
    ASSERT_EQ(arena.size, 1024);
    ASSERT_EQ(arena.used, 0);

    const ANVResult result = anv_arena_destroy(&arena);
    ASSERT_EQ(result, ANV_RESULT_SUCCESS);
    ASSERT_NULL(arena.memory);
    ASSERT_EQ(arena.size, 0);
    ASSERT_EQ(arena.used, 0);

    return TEST_SUCCESS;
}

int test_arena_create_zero_size(void)
{
    // malloc(0) is implementation-defined; just verify no crash
    ANVArena arena = anv_arena_create(0);
    // Clean up if memory was allocated
    if (arena.memory)
    {
        anv_arena_destroy(&arena);
    }
    return TEST_SUCCESS;
}

int test_arena_destroy_null(void)
{
    ASSERT_EQ(anv_arena_destroy(NULL), ANV_RESULT_INVALID_ARGUMENT);

    ANVArena arena = {0};
    ASSERT_EQ(anv_arena_destroy(&arena), ANV_RESULT_INVALID_ARGUMENT);

    return TEST_SUCCESS;
}

int test_arena_allocate_basic(void)
{
    ANVArena arena = anv_arena_create(1024);

    int* a = anv_arena_allocate(&arena, sizeof(int));
    ASSERT_NOT_NULL(a);
    *a = 42;
    ASSERT_EQ(*a, 42);
    ASSERT_GT(arena.used, 0);

    int* b = anv_arena_allocate(&arena, sizeof(int));
    ASSERT_NOT_NULL(b);
    *b = 99;
    ASSERT_EQ(*b, 99);
    ASSERT_EQ(*a, 42); // First allocation still valid

    anv_arena_destroy(&arena);
    return TEST_SUCCESS;
}

int test_arena_allocate_alignment(void)
{
    ANVArena arena = anv_arena_create(1024);

    // Allocate 1 byte - should be padded to 8-byte alignment
    void* p1 = anv_arena_allocate(&arena, 1);
    ASSERT_NOT_NULL(p1);
    ASSERT_EQ(arena.used, 8); // Aligned to 8 bytes

    // Allocate 3 bytes - should be padded to 8
    void* p2 = anv_arena_allocate(&arena, 3);
    ASSERT_NOT_NULL(p2);
    ASSERT_EQ(arena.used, 16);

    // Allocate exactly 8 bytes - no padding needed
    void* p3 = anv_arena_allocate(&arena, 8);
    ASSERT_NOT_NULL(p3);
    ASSERT_EQ(arena.used, 24);

    // Verify pointers are 8-byte aligned
    ASSERT_EQ((uintptr_t)p1 % 8, 0);
    ASSERT_EQ((uintptr_t)p2 % 8, 0);
    ASSERT_EQ((uintptr_t)p3 % 8, 0);

    anv_arena_destroy(&arena);
    return TEST_SUCCESS;
}

int test_arena_allocate_exhaustion(void)
{
    ANVArena arena = anv_arena_create(32);

    // Fill the arena
    const void* p1 = anv_arena_allocate(&arena, 16);
    ASSERT_NOT_NULL(p1);

    const void* p2 = anv_arena_allocate(&arena, 16);
    ASSERT_NOT_NULL(p2);

    // Arena should be full - next allocation fails
    void* p3 = anv_arena_allocate(&arena, 1);
    ASSERT_NULL(p3);

    anv_arena_destroy(&arena);
    return TEST_SUCCESS;
}

int test_arena_allocate_zero_size(void)
{
    ANVArena arena = anv_arena_create(1024);

    void* p = anv_arena_allocate(&arena, 0);
    ASSERT_NULL(p);
    ASSERT_EQ(arena.used, 0);

    anv_arena_destroy(&arena);
    return TEST_SUCCESS;
}

int test_arena_allocate_null(void)
{
    ASSERT_NULL(anv_arena_allocate(NULL, 16));

    ANVArena arena = {0};
    ASSERT_NULL(anv_arena_allocate(&arena, 16));

    return TEST_SUCCESS;
}

//==============================================================================
// Deallocation Tests
//==============================================================================

int test_arena_deallocate_lifo(void)
{
    ANVArena arena = anv_arena_create(1024);

    const void* p1 = anv_arena_allocate(&arena, 16);
    const size_t used_after_p1 = arena.used;

    const void* p2 = anv_arena_allocate(&arena, 32);
    ASSERT_NOT_NULL(p2);
    ASSERT_GT(arena.used, used_after_p1);

    // Deallocate p2 (most recent) - should reset to p2's offset
    anv_arena_deallocate(&arena, p2);
    ASSERT_EQ(arena.used, used_after_p1);

    // Deallocate p1 - should reset to beginning
    anv_arena_deallocate(&arena, p1);
    ASSERT_EQ(arena.used, 0);

    anv_arena_destroy(&arena);
    return TEST_SUCCESS;
}

int test_arena_deallocate_null(void)
{
    ANVArena arena = anv_arena_create(1024);

    // Should not crash
    anv_arena_deallocate(NULL, (void*)1);
    anv_arena_deallocate(&arena, NULL);

    anv_arena_destroy(&arena);
    return TEST_SUCCESS;
}

int test_arena_deallocate_out_of_range(void)
{
    ANVArena arena = anv_arena_create(1024);
    const int dummy = 0;

    const void* p1 = anv_arena_allocate(&arena, 16);
    ASSERT_NOT_NULL(p1);
    const size_t used_before = arena.used;

    // Try to deallocate a pointer outside the arena - should be ignored
    anv_arena_deallocate(&arena, &dummy);
    ASSERT_EQ(arena.used, used_before);

    anv_arena_destroy(&arena);
    return TEST_SUCCESS;
}

//==============================================================================
// Reset Tests
//==============================================================================

int test_arena_reset(void)
{
    ANVArena arena = anv_arena_create(1024);

    // Allocate some memory
    int* a = anv_arena_allocate(&arena, sizeof(int));
    *a = 42;
    anv_arena_allocate(&arena, 100);
    ASSERT_GT(arena.used, 0);

    // Reset
    const ANVResult result = anv_arena_reset(&arena);
    ASSERT_EQ(result, ANV_RESULT_SUCCESS);
    ASSERT_EQ(arena.used, 0);
    ASSERT_EQ(arena.size, 1024); // Size should remain

    // Should be able to allocate again
    int* b = anv_arena_allocate(&arena, sizeof(int));
    ASSERT_NOT_NULL(b);
    *b = 99;
    ASSERT_EQ(*b, 99);

    anv_arena_destroy(&arena);
    return TEST_SUCCESS;
}

int test_arena_reset_null(void)
{
    ASSERT_EQ(anv_arena_reset(NULL), ANV_RESULT_INVALID_ARGUMENT);

    ANVArena arena = {0};
    ASSERT_EQ(anv_arena_reset(&arena), ANV_RESULT_INVALID_ARGUMENT);

    return TEST_SUCCESS;
}

//==============================================================================
// Boundary Tests
//==============================================================================

int test_arena_many_small_allocations(void)
{
    ANVArena arena = anv_arena_create(4096);

    // Allocate many small chunks
    const int count = 100;
    int* ptrs[100];
    for (int i = 0; i < count; i++)
    {
        ptrs[i] = anv_arena_allocate(&arena, sizeof(int));
        ASSERT_NOT_NULL(ptrs[i]);
        *ptrs[i] = i;
    }

    // Verify all values
    for (int i = 0; i < count; i++)
    {
        ASSERT_EQ(*ptrs[i], i);
    }

    anv_arena_destroy(&arena);
    return TEST_SUCCESS;
}

int test_arena_exact_fit(void)
{
    // Create an arena that fits exactly 2 aligned allocations
    ANVArena arena = anv_arena_create(16);

    const void* p1 = anv_arena_allocate(&arena, 8);
    ASSERT_NOT_NULL(p1);
    const void* p2 = anv_arena_allocate(&arena, 8);
    ASSERT_NOT_NULL(p2);

    // Should be exactly full
    void* p3 = anv_arena_allocate(&arena, 1);
    ASSERT_NULL(p3);

    anv_arena_destroy(&arena);
    return TEST_SUCCESS;
}

//==============================================================================
// Main
//==============================================================================

int main(void)
{
    const ANVTestCase tests[] = {
        // CRUD
        TEST_REGISTER(test_arena_create_destroy),
        TEST_REGISTER(test_arena_create_zero_size),
        TEST_REGISTER(test_arena_destroy_null),
        TEST_REGISTER(test_arena_allocate_basic),
        TEST_REGISTER(test_arena_allocate_alignment),
        TEST_REGISTER(test_arena_allocate_exhaustion),
        TEST_REGISTER(test_arena_allocate_zero_size),
        TEST_REGISTER(test_arena_allocate_null),

        // Deallocation
        TEST_REGISTER(test_arena_deallocate_lifo),
        TEST_REGISTER(test_arena_deallocate_null),
        TEST_REGISTER(test_arena_deallocate_out_of_range),

        // Reset
        TEST_REGISTER(test_arena_reset),
        TEST_REGISTER(test_arena_reset_null),

        // Boundary
        TEST_REGISTER(test_arena_many_small_allocations),
        TEST_REGISTER(test_arena_exact_fit),
    };

    return anv_run_tests("Arena", tests, sizeof(tests) / sizeof(tests[0]));
}