//
// Created by zack on 10/9/25.
//

#include "anvil/memory/arena.h"

#include <stdlib.h>
#include <string.h>

ANV_API ANVArena anv_arena_create(const size_t size)
{
    ANVArena arena = {0};
    arena.memory = malloc(size);
    if (arena.memory)
    {
        memset(arena.memory, 0, size);
        arena.size = size;
        arena.used = 0;
    }
    return arena;
}

ANV_API ANVResult anv_arena_destroy(ANVArena *arena)
{
    if (!arena || !arena->memory)
    {
        return ANV_RESULT_INVALID_ARGUMENT;
    }

    free(arena->memory);
    arena->memory = NULL;
    arena->size = 0;
    arena->used = 0;

    return ANV_RESULT_SUCCESS;
}

ANV_API void *anv_arena_allocate(ANVArena *arena, const size_t size)
{
    // Align to 8-byte boundary
    const size_t aligned_size = (size + 7) & ~7;

    if (!arena || !arena->memory || arena->used + aligned_size > arena->size)
    {
        return NULL;
    }

    void* ptr = arena->memory + arena->used;
    arena->used += aligned_size;
    return ptr;
}

ANV_API void anv_arena_deallocate(const ANVArena *arena, const void *ptr)
{
    // Arena allocator doesn't free individual allocations
    (void)arena;
    (void)ptr;
}

ANV_API ANVResult anv_arena_reset(ANVArena *arena)
{
    if (!arena || !arena->memory)
    {
        return ANV_RESULT_INVALID_ARGUMENT;
    }

    memset(arena->memory, 0, arena->size);
    arena->used = 0;
    return ANV_RESULT_SUCCESS;
}
