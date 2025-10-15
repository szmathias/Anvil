//
// Created by zack on 8/30/25.
//

#ifndef ANVIL_ALLOC_H
#define ANVIL_ALLOC_H

#include "platform.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Struct containing Allocator function types for memory management.
 */
typedef struct ANVAllocator
{
        void* (*allocate)(size_t size);
        void (*deallocate)(void* ptr);
        void (*data_free)(void* ptr);
        void* (*copy)(const void* data);
} ANVAllocator;

//==============================================================================
// Utility functions for ANVAllocator
//==============================================================================

/**
 * Create a default allocator using standard library functions.
 * Uses malloc and free for allocation. The default copy function
 * just returns the pointer provided to it.
 *
 * @return ANVAllocator struct with default functions
 */
ANV_API ANVAllocator anv_alloc_default(void);

/**
 * Create a custom allocator with user-provided functions.
 *
 * @param alloc_func Memory allocation function (required)
 * @param dealloc_func Memory deallocation function (required)
 * @param data_free_func User data cleanup function (can be NULL)
 * @param anv_copy_func Data copying function (can be NULL)
 * @return ANVAllocator struct with custom functions
 */
ANV_API ANVAllocator anv_alloc_custom(anv_allocate_func alloc_func, anv_deallocate_func dealloc_func,
                                      anv_deallocate_func data_free_func, anv_copy_func anv_copy_func);

/**
 * Allocate memory using the allocator's allocation function.
 *
 * @param alloc Pointer to ANVAllocator struct
 * @param size Number of bytes to allocate
 * @return Pointer to allocated memory, or NULL on failure
 */
ANV_API void* anv_alloc_allocate(const ANVAllocator* alloc, size_t size);

/**
 * Free memory using the allocator's deallocation function.
 *
 * @param alloc Pointer to ANVAllocator struct
 * @param ptr Pointer to memory to free
 */
ANV_API void anv_alloc_deallocate(const ANVAllocator* alloc, void* ptr);

/**
 * Free user data using the allocator's data free function.
 * Does nothing if data_free is NULL.
 *
 * @param alloc Pointer to ANVAllocator struct
 * @param ptr Pointer to user data to free
 */
ANV_API void anv_alloc_data_deallocate(const ANVAllocator* alloc, void* ptr);

/**
 * Copy data using the allocator's copy function.
 * Returns NULL if copy is NULL.
 *
 * @param alloc Pointer to ANVAllocator struct
 * @param data Pointer to data to copy
 * @return Pointer to copied data, or NULL if anv_copy_func is NULL or on failure
 */
ANV_API void* anv_alloc_copy(const ANVAllocator* alloc, const void* data);

#ifdef __cplusplus
}
#endif

#endif // ANVIL_ALLOC_H
