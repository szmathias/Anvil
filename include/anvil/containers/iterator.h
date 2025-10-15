//
// Created by zack on 8/26/25.
//

#ifndef ANVIL_ITERATOR_H
#define ANVIL_ITERATOR_H

#include "anvil/common.h"

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Iterator interface
//==============================================================================

/**
 * Generic iterator structure.
 * Provides a common interface for traversing different data structures.
 */
typedef struct ANVIterator ANVIterator;

/**
 * Indexed element structure for enumerate iterator.
 * Contains both the index/position and the element data.
 */
typedef struct ANVIndexedElement
{
        size_t index;        // Zero-based index/position
        void* element;       // Pointer to the element data
        ANVAllocator alloc;  // Allocator for memory management
} ANVIndexedElement;

/**
 * Iterator interface with function pointers for standard operations.
 */
struct ANVIterator
{
        void* data_state;                       // Implementation-specific state data
        ANVAllocator alloc;                     // Allocator for memory management

        // Element access
        void* (*get)(const ANVIterator* it);    // Get current element without advancing

        // Forward iteration
        int (*has_next)(const ANVIterator* it); // Check if more elements exist
        int (*next)(const ANVIterator* it);     // Advance to next position (returns 0 on success, -1 on failure)

        // Backward iteration
        int (*has_prev)(const ANVIterator* it); // Check if previous elements exist
        int (*prev)(const ANVIterator* it);     // Move back to previous position (returns 0 on success, -1 on failure)

        // Control operations
        void (*reset)(const ANVIterator* it);   // Reset to starting position
        int (*is_valid)(const ANVIterator* it); // Check if iterator is valid

        // Resource management
        void (*destroy)(ANVIterator* it);       // Free iterator resources
};

//==============================================================================
// Higher-order iterator functions
//==============================================================================

/**
 * Create a transforming iterator that applies a function to each element.
 * The new iterator produces transformed elements on-the-fly.
 *
 * Note: The returned iterator takes ownership of the source iterator
 * and will destroy it when the transform iterator is destroyed.
 *
 * Transform functions should return non-allocated memory (static buffers,
 * pointers to existing data, etc.) or handle their own memory management.
 * If you need to collect transformed values, use anv_iterator_copy().
 *
 * @param it Base iterator to transform (ownership transferred)
 * @param alloc The allocator to use for the new iterator's state
 * @param transform Function to apply to each element
 * @param transform_allocates Whether the transform function allocates memory
 * @return A new iterator producing transformed elements
 */
ANV_API ANVIterator anv_iterator_transform(ANVIterator* it, const ANVAllocator* alloc, anv_transform_func transform, bool transform_allocates);

/**
 * Create a filtering iterator that only yields elements matching a predicate.
 * The new iterator skips elements that don't satisfy the filter function.
 *
 * Note: The returned iterator takes ownership of the source iterator
 * and will destroy it when the filter iterator is destroyed.
 *
 * @param it Base iterator to filter (ownership transferred)
 * @param alloc The allocator to use for the new iterator's state.
 * @param filter Predicate function that determines which elements to include
 * @return A new iterator yielding only elements that satisfy the predicate
 */
ANV_API ANVIterator anv_iterator_filter(ANVIterator* it, const ANVAllocator* alloc, anv_filter_func filter);

/**
 * Create an iterator that yields integers in a specified range.
 *
 * @param alloc The allocator to use for the new iterator's state.
 * @param start Starting value (inclusive)
 * @param end Ending value (exclusive)
 * @param step Step value (non-zero)
 * @return A new iterator yielding integers in the specified range
 */
ANV_API ANVIterator anv_iterator_range(const ANVAllocator* alloc, int start, int end, int step);

/**
 * Create a copy iterator that returns deep copies of elements.
 *
 * The copy iterator creates deep copies that the user owns and must free.
 * This is different from other iterators which return cached pointers.
 *
 * @param it Base iterator to copy from (ownership transferred)
 * @param alloc The allocator to use for iterator state and element copies
 * @param copy Function to create deep copies of elements
 * @return A new iterator yielding user-owned copies
 */
ANV_API ANVIterator anv_iterator_copy(ANVIterator* it, const ANVAllocator* alloc, anv_copy_func copy);

/**
 * Create a take iterator that limits iteration to first N elements.
 * The new iterator will yield at most N elements from the base iterator.
 *
 * Note: The returned iterator takes ownership of the source iterator
 * and will destroy it when the take iterator is destroyed.
 *
 * @param it Base iterator to take from (ownership transferred)
 * @param alloc The allocator to use for the new iterator's state
 * @param count Maximum number of elements to yield (0 means no elements)
 * @return A new iterator yielding at most count elements
 */
ANV_API ANVIterator anv_iterator_take(ANVIterator* it, const ANVAllocator* alloc, size_t count);

/**
 * Create a skip iterator that skips first N elements, then yields the rest.
 * The new iterator will skip the first N elements and then yield all remaining elements.
 *
 * Note: The returned iterator takes ownership of the source iterator
 * and will destroy it when the skip iterator is destroyed.
 *
 * @param it Base iterator to skip from (ownership transferred)
 * @param alloc The allocator to use for the new iterator's state
 * @param count Number of elements to skip (0 means skip none)
 * @return A new iterator yielding elements after skipping count elements
 */
ANV_API ANVIterator anv_iterator_skip(ANVIterator* it, const ANVAllocator* alloc, size_t count);

/**
 * Create a zip iterator that combines elements from two iterators pairwise.
 * The new iterator yields ANVPair objects containing corresponding elements from both iterators.
 * Iteration stops when either iterator is exhausted.
 *
 * Note: The returned iterator takes ownership of both source iterators
 * and will destroy them when the zip iterator is destroyed.
 *
 * @param it1 First iterator (ownership transferred)
 * @param it2 Second iterator (ownership transferred)
 * @param alloc The allocator to use for the new iterator's state and pair creation
 * @return A new iterator yielding ANVPair objects with elements from both iterators
 */
ANV_API ANVIterator anv_iterator_zip(ANVIterator* it1, ANVIterator* it2, const ANVAllocator* alloc);

/**
 * Create an enumerate iterator that adds index numbers to elements.
 * The new iterator yields ANVIndexedElement objects containing both the zero-based
 * index and the original element data.
 *
 * Note: The returned iterator takes ownership of the source iterator
 * and will destroy it when to enumerate iterator is destroyed.
 *
 * @param it Base iterator to enumerate (ownership transferred)
 * @param alloc The allocator to use for the new iterator's state
 * @param start_index Starting index value (typically 0)
 * @return A new iterator yielding ANVIndexedElement objects with index and data
 */
ANV_API ANVIterator anv_iterator_enumerate(ANVIterator* it, const ANVAllocator* alloc, size_t start_index);

/**
 * Create a repeat iterator that yields the same value N times.
 * The iterator returns a pointer to the original value for each iteration,
 * allowing the same data to be processed multiple times through iterator chains.
 *
 * Note: The returned iterator does not take ownership of the value pointer.
 * The caller must ensure the value remains valid for the lifetime of the iterator.
 * All calls to get() return the same pointer to the original value.
 * Chain with copy iterator if unique instances are needed
 *
 * @param value Pointer to the value to repeat (must remain valid during iteration)
 * @param alloc The allocator to use for the iterator's internal state
 * @param count Number of times to repeat the value (0 means no elements)
 * @return A new iterator yielding the same value count times
 */
ANV_API ANVIterator anv_iterator_repeat(const void* value, const ANVAllocator* alloc, size_t count);

/**
 * Create a chain iterator that combines multiple iterators sequentially.
 * The new iterator yields all elements from the first iterator, then all elements
 * from the second iterator, and so on. This allows appending datasets together.
 *
 * Note: The returned iterator takes ownership of all source iterators
 * and will destroy them when the chain iterator is destroyed.
 *
 * @param iterators Array of iterators to chain together (ownership transferred)
 * @param iterator_count Number of iterators in the array
 * @param alloc The allocator to use for the iterator's internal state
 * @return A new iterator yielding elements from all source iterators sequentially
 */
ANV_API ANVIterator anv_iterator_chain(ANVIterator* iterators, size_t iterator_count, const ANVAllocator* alloc);

#ifdef __cplusplus
}
#endif

#endif // ANVIL_ITERATOR_H
