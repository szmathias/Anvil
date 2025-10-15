//
// Created by zack on 10/9/25.
//

#include "anvil/memory/stack_frame.h"

#include <string.h>

ANV_API void *anv_stackframe_allocate(ANVStackFrame* frame, const size_t size)
{
    if (!frame || size == 0)
    {
        return NULL;
    }

    const size_t aligned_size = (size + 7) & ~7;

    if (frame->top + aligned_size > ANV_STACK_FRAME_SIZE)
    {
        return NULL;
    }

    void* ptr = frame->memory + frame->top;
    frame->top += aligned_size;
    return ptr;
}

ANV_API void anv_stackframe_deallocate(ANVStackFrame* frame, void* ptr)
{
    if (!frame || !ptr)
    {
        return;
    }

    const uint8_t *byte_ptr = ptr;
    if (byte_ptr >= frame->memory && byte_ptr < frame->memory + ANV_STACK_FRAME_SIZE)
    {
        if (byte_ptr < frame->memory + frame->top)
        {
            frame->top = byte_ptr - frame->memory;
        }
    }
}

ANV_API void anv_stackframe_reset(ANVStackFrame* frame)
{
    if (!frame)
    {
        return;
    }
    memset(frame, 0, sizeof(ANVStackFrame));
}
