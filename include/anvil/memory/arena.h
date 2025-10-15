//
// Created by zack on 10/9/25.
//

#ifndef ANVIL_ARENA_H
#define ANVIL_ARENA_H

#include "anvil/common.h"

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Type definitions
//==============================================================================

/**
 * Arena allocator structure for bump-pointer allocation.
 *
 * This structure manages a contiguous block of memory for fast allocation.
 * Allocations are made by bumping a pointer forward (increasing 'used').
 * Memory is reclaimed only when the arena is reset or destroyed. All
 * allocations are 8-byte aligned.
 */
typedef struct ANVArena
{
    uint8_t *memory;        // Pointer to the arena's memory block
    size_t size;            // Total size of the arena in bytes
    size_t used;            // Number of bytes currently allocated
} ANVArena;

//==============================================================================
// Creation and destruction functions
//==============================================================================

/**
 * Create a new arena allocator with the specified size.
 *
 * Allocates a contiguous block of memory of the given size. The arena
 * starts with zero bytes used. If allocation fails, returns an arena
 * with NULL memory pointer.
 *
 * @param size Size of the arena in bytes (must be greater than 0)
 * @return New ANVArena structure (check memory field for NULL to detect failure)
 */
ANV_API ANVArena anv_arena_create(size_t size);

/**
 * Destroy the arena and free all associated memory.
 *
 * Frees the entire arena memory block and resets all fields to zero.
 * After destruction, the arena cannot be used for allocation.
 *
 * @param arena The arena to destroy (must not be NULL, arena->memory must not be NULL)
 * @return ANV_RESULT_SUCCESS on success, ANV_RESULT_INVALID_ARGUMENT if arena or arena->memory is NULL
 */
ANV_API ANVResult anv_arena_destroy(ANVArena *arena);

//==============================================================================
// Memory allocation operations
//==============================================================================

/**
 * Allocate memory from the arena.
 *
 * Allocates the requested size (rounded up to 8-byte alignment) from the arena's
 * memory pool by advancing the 'used' pointer. Returns NULL if there is insufficient
 * space remaining in the arena.
 *
 * @param arena The arena to allocate from (must not be NULL, arena->memory must not be NULL)
 * @param size Number of bytes to allocate (must be greater than 0)
 * @return Pointer to allocated memory, or NULL if allocation fails
 */
ANV_API void *anv_arena_allocate(ANVArena *arena, size_t size);

/**
 * Deallocate memory from the arena.
 *
 * This function implements stack-based (LIFO) deallocation. When a pointer
 * is freed, the arena's 'used' counter is reset to that pointer's offset,
 * effectively freeing that allocation and all allocations made after it.
 * This allows freeing the most recent allocations in reverse order.
 *
 * Note: Only pointers obtained from anv_arena_allocate should be passed to
 * this function. Freeing a pointer will also free any allocations made after it.
 *
 * @param arena The arena to deallocate from (must not be NULL, arena->memory must not be NULL)
 * @param ptr Pointer to memory previously allocated from this arena
 */
ANV_API void anv_arena_deallocate(ANVArena *arena, const void *ptr);

/**
 * Reset the arena to its initial empty state.
 *
 * Resets the 'used' counter to zero, effectively freeing all allocations
 * made from the arena. The memory block remains allocated and is zeroed out.
 * After reset, the arena can be reused for new allocations.
 *
 * @param arena The arena to reset (must not be NULL, arena->memory must not be NULL)
 * @return ANV_RESULT_SUCCESS on success, ANV_RESULT_INVALID_ARGUMENT if arena or arena->memory is NULL
 */
ANV_API ANVResult anv_arena_reset(ANVArena *arena);

#ifdef __cplusplus
}
#endif

#endif //ANVIL_ARENA_H