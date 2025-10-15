//
// Created by zack on 9/9/25.
//

#ifndef ANVIL_QUEUE_H
#define ANVIL_QUEUE_H

#include "iterator.h"
#include "anvil/common.h"

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Type definitions
//==============================================================================

/**
 * Node of queue
 */
typedef struct ANVQueueNode
{
        void* data;                // Pointer to user data
        struct ANVQueueNode* next; // Pointer to next node
} ANVQueueNode;

/**
 * Queue structure
 */
typedef struct ANVQueue
{
        ANVQueueNode* front; // Pointer to front node
        ANVQueueNode* back;  // Pointer to back node
        size_t size;         // Number of elements
        ANVAllocator alloc;  // Custom allocator
} ANVQueue;

/**
 * Action function for applying an operation to each element.
 * Used in for-each style traversal.
 *
 * @param data Pointer to element data
 */
typedef void (*anv_action_func)(void* data);

//==============================================================================
// Creation and destruction functions
//==============================================================================

/**
 * Create a new, empty queue with custom allocator.
 *
 * @param alloc Custom allocator (required)
 * @return Pointer to new Queue, or NULL on failure
 */
ANV_API ANVQueue* anv_queue_create(ANVAllocator* alloc);

/**
 * Destroy the queue and free all nodes.
 *
 * @param queue The queue to destroy
 * @param should_free_data Whether to free the data elements
 */
ANV_API void anv_queue_destroy(ANVQueue* queue, bool should_free_data);

/**
 * Clear all elements from the queue, but keep the structure intact.
 *
 * @param queue The queue to clear
 * @param should_free_data Whether to free the data elements
 */
ANV_API void anv_queue_clear(ANVQueue* queue, bool should_free_data);

//==============================================================================
// Information functions
//==============================================================================

/**
 * Get the number of elements in the queue.
 *
 * @param queue The queue to query
 * @return Number of elements, or 0 if queue is NULL
 */
ANV_API size_t anv_queue_size(const ANVQueue* queue);

/**
 * Check if the queue is empty.
 *
 * @param queue The queue to check
 * @return 1 if queue is empty or NULL, 0 if it contains elements
 */
ANV_API int anv_queue_is_empty(const ANVQueue* queue);

/**
 * Compare two queues for equality using the given comparison function.
 *
 * @param queue1 First queue to compare
 * @param queue2 Second queue to compare
 * @param compare Function to compare elements
 * @return 1 if queues are equal, 0 if not equal, -1 on error
 */
ANV_API int anv_queue_equals(const ANVQueue* queue1, const ANVQueue* queue2, anv_compare_func compare);

//==============================================================================
// Element access functions
//==============================================================================

/**
 * Get the front element without removing it.
 *
 * @param queue The queue to access
 * @return Pointer to front element data, or NULL if empty or on error
 */
ANV_API void* anv_queue_front(const ANVQueue* queue);

/**
 * Get the back element without removing it.
 *
 * @param queue The queue to access
 * @return Pointer to back element data, or NULL if empty or on error
 */
ANV_API void* anv_queue_back(const ANVQueue* queue);

//==============================================================================
// Queue operations
//==============================================================================

/**
 * Add an element to the back of the queue.
 *
 * @param queue The queue to modify
 * @param data Pointer to the data to enqueue (ownership transferred to queue)
 * @return 0 on success, -1 on error
 */
ANV_API int anv_queue_enqueue(ANVQueue* queue, void* data);

/**
 * Remove the front element from the queue.
 *
 * @param queue The queue to modify
 * @param should_free_data Whether to free the dequeued data
 * @return 0 on success, -1 on error
 */
ANV_API int anv_queue_dequeue(ANVQueue* queue, bool should_free_data);

/**
 * Remove the front element and return its data pointer.
 * The caller is responsible for freeing the returned data if needed.
 *
 * @param queue The queue to modify
 * @return Pointer to the dequeued data, or NULL if queue is empty or on error
 */
ANV_API void* anv_queue_dequeue_data(ANVQueue* queue);

//==============================================================================
// Higher-order functions
//==============================================================================

/**
 * Apply an action function to each element in the queue.
 *
 * @param queue The queue to process
 * @param action Function applied to each element
 */
ANV_API void anv_queue_for_each(const ANVQueue* queue, anv_action_func action);

//==============================================================================
// Queue copying functions
//==============================================================================

/**
 * Create a shallow copy of the queue.
 *
 * @param queue The queue to copy
 * @return A new queue with same structure but sharing data, or NULL on error
 */
ANV_API ANVQueue* anv_queue_copy(ANVQueue* queue);

/**
 * Create a deep copy of the queue.
 *
 * @param queue The queue to copy
 * @param should_free_data Whether the copy function should handle data freeing
 * @return A new queue with copies of all data, or NULL on error
 */
ANV_API ANVQueue* anv_queue_copy_deep(ANVQueue* queue, bool should_free_data);

//==============================================================================
// Iterator functions
//==============================================================================

/**
 * Create an iterator for the queue.
 *
 * @param queue The queue to iterate over
 * @return An Iterator object for forward traversal
 */
ANV_API ANVIterator anv_queue_iterator(const ANVQueue* queue);

/**
 * Create a new queue from an iterator with custom allocator.
 *
 * This function consumes all elements from the provided iterator and creates
 * a new queue containing those elements. Elements are enqueued in FIFO order as
 * they are encountered from the iterator.
 *
 * @param it The source iterator (must be valid and support has_next/get/next)
 * @param alloc The custom allocator to use for the new queue
 * @param should_copy If true, creates deep copies of all elements.
 *                    If false, uses elements directly from iterator.
 *                    When true, the allocators copy function must be valid.
 * @return A new queue with elements from iterator, or NULL on error
 *
 * @note NULL elements from the iterator are always filtered out as they indicate
 *       iterator issues rather than valid data.
 * @note The iterator is consumed during this operation - it will be at the end
 *       position after the function completes.
 * @note If should_copy is true and copying fails for any element, the function
 *       cleans up and returns NULL.
 */
ANV_API ANVQueue* anv_queue_from_iterator(ANVIterator* it, ANVAllocator* alloc, bool should_copy);

#ifdef __cplusplus
}
#endif

#endif // ANVIL_QUEUE_H
