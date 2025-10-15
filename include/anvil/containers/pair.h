//
// Created by zack on 9/15/25.
//

#ifndef ANVIL_PAIR_H
#define ANVIL_PAIR_H

#include "anvil/common.h"

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Type definitions
//==============================================================================

/**
 * Generic pair structure that holds two values of any type.
 * Provides a simple way to group two related values together.
 * The pair takes ownership of the pointers and can optionally free them.
 */
typedef struct ANVPair
{
        void* first;         // Pointer to first element data
        void* second;        // Pointer to second element data
        ANVAllocator alloc; // Custom allocator
} ANVPair;

/**
 * Comparison function for pair elements.
 * Should return:
 *   <0 if a < b,
 *    0 if a == b,
 *   >0 if a > b
 *
 * @param a Pointer to first element
 * @param b Pointer to second element
 * @return Comparison result
 */
typedef int (*pair_compare_func)(const void* a, const void* b);

//==============================================================================
// Creation and destruction functions
//==============================================================================

/**
 * Create a new pair with allocator.
 * The pair takes ownership of the provided pointers.
 *
 * @param alloc Custom allocator (required)
 * @param first Pointer to first element data (pair takes ownership)
 * @param second Pointer to second element data (pair takes ownership)
 * @return Pointer to new pair, or NULL on failure
 */
ANV_API ANVPair* anv_pair_create(ANVAllocator* alloc, void* first, void* second);

/**
 * Initialize a ANVPair structure with the given elements (no allocation).
 * The pair takes ownership of the provided pointers but is not itself allocated.
 * Use this when you have a stack-allocated ANVPair structure.
 *
 * @param pair Pointer to existing ANVPair structure
 * @param alloc Custom allocator (can be NULL)
 * @param first Pointer to first element data
 * @param second Pointer to second element data
 * @return 0 on success, -1 if pair is NULL
 */
ANV_API int anv_pair_init(ANVPair* pair, const ANVAllocator* alloc, void* first, void* second);

/**
 * Destroy the pair and optionally free the contained data.
 *
 * @param pair The pair to destroy
 * @param should_free_first Whether to free first element data
 * @param should_free_second Whether to free second element data
 */
ANV_API void anv_pair_destroy(ANVPair* pair, bool should_free_first, bool should_free_second);

//==============================================================================
// Access functions
//==============================================================================

/**
 * Get the first element of the pair.
 *
 * @param pair The pair
 * @return Pointer to first element data, or NULL if pair is NULL
 */
ANV_API void* anv_pair_first(const ANVPair* pair);

/**
 * Get the second element of the pair.
 *
 * @param pair The pair
 * @return Pointer to second element data, or NULL if pair is NULL
 */
ANV_API void* anv_pair_second(const ANVPair* pair);

/**
 * Set the first element of the pair.
 * The pair takes ownership of the new pointer.
 *
 * @param pair The pair
 * @param first New first element data (pair takes ownership)
 * @param should_free_old Whether to free the old first element
 */
ANV_API void anv_pair_set_first(ANVPair* pair, void* first, bool should_free_old);

/**
 * Set the second element of the pair.
 * The pair takes ownership of the new pointer.
 *
 * @param pair The pair
 * @param second New second element data (pair takes ownership)
 * @param should_free_old Whether to free the old second element
 */
ANV_API void anv_pair_set_second(ANVPair* pair, void* second, bool should_free_old);

//==============================================================================
// Utility functions
//==============================================================================

/**
 * Swap the first and second elements of the pair.
 *
 * @param pair The pair
 */
ANV_API void anv_pair_swap(ANVPair* pair);

/**
 * Compare two pairs lexicographically using provided comparison functions.
 * First compares the first elements, then the second elements if first are equal.
 *
 * @param pair1 First pair
 * @param pair2 Second pair
 * @param first_compare Comparison function for first elements
 * @param second_compare Comparison function for second elements
 * @return <0 if pair1 < pair2, 0 if equal, >0 if pair1 > pair2
 */
ANV_API int anv_pair_compare(const ANVPair* pair1, const ANVPair* pair2,
                             pair_compare_func first_compare, pair_compare_func second_compare);

/**
 * Check if two pairs are equal using provided comparison functions.
 *
 * @param pair1 First pair
 * @param pair2 Second pair
 * @param first_compare Comparison function for first elements
 * @param second_compare Comparison function for second elements
 * @return 1 if pairs are equal, 0 otherwise
 */
ANV_API int anv_pair_equals(const ANVPair* pair1, const ANVPair* pair2,
                            pair_compare_func first_compare, pair_compare_func second_compare);

/**
 * Create a shallow copy of a pair.
 *
 * @param pair The pair to copy
 * @return Pointer to new pair copy, or NULL on failure
 */
ANV_API ANVPair* anv_pair_copy(ANVPair* pair);

/**
 * Create a deep copy of a pair with copied data using separate copy functions.
 * This allows copying pairs with different data types for first and second elements.
 *
 * @param pair The pair to copy
 * @param first_copy Copy function for first element (NULL to skip copying, just reference)
 * @param second_copy Copy function for second element (NULL to skip copying, just reference)
 * @param should_free Whether to free first element if copy fails (for cleanup)
 * @return Pointer to new pair copy, or NULL on failure
 */
ANV_API ANVPair* anv_pair_copy_deep(ANVPair* pair, anv_copy_func first_copy, anv_copy_func second_copy, bool should_free);

//==============================================================================
// Common copy helper functions
//==============================================================================

/**
 * Copy function for pairs containing string first and int second elements.
 * Creates a deep copy of a ANVPair where first is a string and second is an int.
 *
 * @param pair_data Pointer to a ANVPair containing string first, int second
 * @return Pointer to newly allocated ANVPair copy, or NULL on failure
 */
ANV_API void* anv_pair_copy_string_int(const void* pair_data);

/**
 * Copy function for pairs containing int first and string second elements.
 * Creates a deep copy of a ANVPair where first is an int and second is a string.
 *
 * @param pair_data Pointer to a ANVPair containing int first, string second
 * @return Pointer to newly allocated ANVPair copy, or NULL on failure
 */
ANV_API void* anv_pair_copy_int_string(const void* pair_data);

/**
 * Copy function for pairs containing string first and string second elements.
 * Creates a deep copy of a ANVPair where both elements are strings.
 *
 * @param pair_data Pointer to a ANVPair containing string first, string second
 * @return Pointer to newly allocated ANVPair copy, or NULL on failure
 */
ANV_API void* anv_pair_copy_string_string(const void* pair_data);

/**
 * Copy function for pairs containing int first and int second elements.
 * Creates a deep copy of a ANVPair where both elements are integers.
 *
 * @param pair_data Pointer to a ANVPair containing int first, int second
 * @return Pointer to newly allocated ANVPair copy, or NULL on failure
 */
ANV_API void* anv_pair_copy_int_int(const void* pair_data);

#ifdef __cplusplus
}
#endif

#endif // ANVIL_PAIR_H
