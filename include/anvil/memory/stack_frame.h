//
// Created by zack on 10/9/25.
//

#ifndef ANVIL_STACK_FRAME_H
#define ANVIL_STACK_FRAME_H

#include "anvil/common.h"

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Constants
//==============================================================================

/**
 * Size of the stack frame buffer in bytes.
 */
#define ANV_STACK_FRAME_SIZE 4096

//==============================================================================
// Type definitions
//==============================================================================

/**
 * Stack frame allocator structure for LIFO memory allocation.
 *
 * This structure manages a fixed-size stack-based memory buffer for fast
 * temporary allocations. Memory is allocated by advancing the 'top' pointer.
 * Deallocations must follow LIFO order - only the most recent allocation
 * can be freed. All allocations are 8-byte aligned.
 */
typedef struct ANVStackFrame
{
    uint8_t memory[ANV_STACK_FRAME_SIZE];  // Fixed-size memory buffer
    size_t top;                            // Current stack top position (bytes used)
} ANVStackFrame;

//==============================================================================
// Memory allocation operations
//==============================================================================

/**
 * Allocate memory from the stack frame.
 *
 * Allocates the requested size (rounded up to 8-byte alignment) from the
 * stack frame's memory buffer by advancing the 'top' pointer. Returns NULL
 * if there is insufficient space remaining.
 *
 * @param frame The stack frame to allocate from (must not be NULL)
 * @param size Number of bytes to allocate (must be greater than 0)
 * @return Pointer to allocated memory, or NULL if allocation fails
 */
ANV_API void *anv_stackframe_allocate(ANVStackFrame* frame, size_t size);

/**
 * Deallocate memory from the stack frame (LIFO only).
 *
 * Deallocates memory by moving the 'top' pointer back to the given pointer's
 * position. This only works correctly if the pointer being freed is the most
 * recent allocation (LIFO order). Freeing out of order will corrupt the stack
 * and leak memory. Does nothing if ptr is NULL or outside the frame's memory.
 *
 * @param frame The stack frame to deallocate from (must not be NULL)
 * @param ptr Pointer to memory to deallocate (should be the most recent allocation)
 */
ANV_API void anv_stackframe_deallocate(ANVStackFrame* frame, void* ptr);

/**
 * Reset the stack frame to its initial empty state.
 *
 * Resets the entire frame to zero, effectively freeing all allocations.
 * After reset, the frame can be reused for new allocations.
 *
 * @param frame The stack frame to reset (must not be NULL)
 */
ANV_API void anv_stackframe_reset(ANVStackFrame* frame);

#ifdef __cplusplus
}
#endif

#endif //ANVIL_STACK_FRAME_H