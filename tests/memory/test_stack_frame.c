//
// Tests for stack frame allocator (memory/stack_frame.h)
//

#include <string.h>
#include <stdio.h>
#include "memory/stack_frame.h"
#include "TestAssert.h"
#include "TestRunner.h"

int test_stackframe_allocate_basic(void)
{
    ANVStackFrame frame = {0};

    int* a = anv_stackframe_allocate(&frame, sizeof(int));
    ASSERT_NOT_NULL(a);
    *a = 42;
    ASSERT_EQ(*a, 42);
    ASSERT_GT(frame.top, 0);

    int* b = anv_stackframe_allocate(&frame, sizeof(int));
    ASSERT_NOT_NULL(b);
    *b = 99;
    ASSERT_EQ(*a, 42);
    ASSERT_EQ(*b, 99);

    return TEST_SUCCESS;
}

int test_stackframe_allocate_alignment(void)
{
    ANVStackFrame frame = {0};

    // 1-byte allocation should be padded to 8
    void* p1 = anv_stackframe_allocate(&frame, 1);
    ASSERT_NOT_NULL(p1);
    ASSERT_EQ(frame.top, 8);

    // 5-byte allocation should be padded to 8
    void* p2 = anv_stackframe_allocate(&frame, 5);
    ASSERT_NOT_NULL(p2);
    ASSERT_EQ(frame.top, 16);

    // Verify 8-byte alignment
    ASSERT_EQ((uintptr_t)p1 % 8, 0);
    ASSERT_EQ((uintptr_t)p2 % 8, 0);

    return TEST_SUCCESS;
}

int test_stackframe_allocate_exhaustion(void)
{
    ANVStackFrame frame = {0};

    // Fill the frame (4096 bytes total)
    size_t allocated = 0;
    while (allocated + 256 <= ANV_STACK_FRAME_SIZE)
    {
        void* p = anv_stackframe_allocate(&frame, 256);
        ASSERT_NOT_NULL(p);
        allocated += 256;
    }

    // Allocate remaining space
    size_t remaining = ANV_STACK_FRAME_SIZE - frame.top;
    if (remaining > 0)
    {
        void* p = anv_stackframe_allocate(&frame, remaining);
        ASSERT_NOT_NULL(p);
    }

    // Now the frame should be full
    void* fail = anv_stackframe_allocate(&frame, 1);
    ASSERT_NULL(fail);

    return TEST_SUCCESS;
}

int test_stackframe_allocate_zero_size(void)
{
    ANVStackFrame frame = {0};

    void* p = anv_stackframe_allocate(&frame, 0);
    ASSERT_NULL(p);
    ASSERT_EQ(frame.top, 0);

    return TEST_SUCCESS;
}

int test_stackframe_allocate_null(void)
{
    ASSERT_NULL(anv_stackframe_allocate(NULL, 16));
    return TEST_SUCCESS;
}

int test_stackframe_deallocate_lifo(void)
{
    ANVStackFrame frame = {0};

    void* p1 = anv_stackframe_allocate(&frame, 16);
    size_t top_after_p1 = frame.top;

    void* p2 = anv_stackframe_allocate(&frame, 32);
    ASSERT_NOT_NULL(p2);
    ASSERT_GT(frame.top, top_after_p1);

    // Deallocate p2 (most recent)
    anv_stackframe_deallocate(&frame, p2);
    ASSERT_EQ(frame.top, top_after_p1);

    // Deallocate p1
    anv_stackframe_deallocate(&frame, p1);
    ASSERT_EQ(frame.top, 0);

    return TEST_SUCCESS;
}

int test_stackframe_deallocate_null(void)
{
    ANVStackFrame frame = {0};

    // Should not crash
    anv_stackframe_deallocate(NULL, (void*)1);
    anv_stackframe_deallocate(&frame, NULL);

    return TEST_SUCCESS;
}

int test_stackframe_deallocate_out_of_range(void)
{
    ANVStackFrame frame = {0};
    int dummy = 0;

    void* p1 = anv_stackframe_allocate(&frame, 16);
    ASSERT_NOT_NULL(p1);
    size_t top_before = frame.top;

    // Try to deallocate a pointer outside the frame
    anv_stackframe_deallocate(&frame, &dummy);
    ASSERT_EQ(frame.top, top_before);

    return TEST_SUCCESS;
}

int test_stackframe_reset(void)
{
    ANVStackFrame frame = {0};

    int* a = anv_stackframe_allocate(&frame, sizeof(int));
    *a = 42;
    anv_stackframe_allocate(&frame, 100);
    ASSERT_GT(frame.top, 0);

    anv_stackframe_reset(&frame);
    ASSERT_EQ(frame.top, 0);

    // Should be reusable
    int* b = anv_stackframe_allocate(&frame, sizeof(int));
    ASSERT_NOT_NULL(b);
    *b = 99;
    ASSERT_EQ(*b, 99);

    return TEST_SUCCESS;
}

int test_stackframe_reset_null(void)
{
    // Should not crash
    anv_stackframe_reset(NULL);
    return TEST_SUCCESS;
}

int test_stackframe_reuse_after_dealloc(void)
{
    ANVStackFrame frame = {0};

    void* p1 = anv_stackframe_allocate(&frame, 64);
    ASSERT_NOT_NULL(p1);

    void* p2 = anv_stackframe_allocate(&frame, 64);
    ASSERT_NOT_NULL(p2);

    // Deallocate p2, then allocate same size - should reuse the space
    anv_stackframe_deallocate(&frame, p2);
    void* p3 = anv_stackframe_allocate(&frame, 64);
    ASSERT_NOT_NULL(p3);
    ASSERT_EQ_PTR(p2, p3); // Should get the same address

    return TEST_SUCCESS;
}

int test_stackframe_many_allocations(void)
{
    ANVStackFrame frame = {0};

    // Allocate many small chunks (8 bytes aligned = can fit 512 in 4096)
    const int count = 100;
    int* ptrs[100];
    for (int i = 0; i < count; i++)
    {
        ptrs[i] = anv_stackframe_allocate(&frame, sizeof(int));
        ASSERT_NOT_NULL(ptrs[i]);
        *ptrs[i] = i;
    }

    // Verify all values
    for (int i = 0; i < count; i++)
    {
        ASSERT_EQ(*ptrs[i], i);
    }

    return TEST_SUCCESS;
}

int main(void)
{
    const ANVTestCase tests[] = {
        {test_stackframe_allocate_basic, "test_stackframe_allocate_basic"},
        {test_stackframe_allocate_alignment, "test_stackframe_allocate_alignment"},
        {test_stackframe_allocate_exhaustion, "test_stackframe_allocate_exhaustion"},
        {test_stackframe_allocate_zero_size, "test_stackframe_allocate_zero_size"},
        {test_stackframe_allocate_null, "test_stackframe_allocate_null"},
        {test_stackframe_deallocate_lifo, "test_stackframe_deallocate_lifo"},
        {test_stackframe_deallocate_null, "test_stackframe_deallocate_null"},
        {test_stackframe_deallocate_out_of_range, "test_stackframe_deallocate_out_of_range"},
        {test_stackframe_reset, "test_stackframe_reset"},
        {test_stackframe_reset_null, "test_stackframe_reset_null"},
        {test_stackframe_reuse_after_dealloc, "test_stackframe_reuse_after_dealloc"},
        {test_stackframe_many_allocations, "test_stackframe_many_allocations"},
    };

    return anv_run_tests("StackFrame", tests, sizeof(tests) / sizeof(tests[0]));
}
