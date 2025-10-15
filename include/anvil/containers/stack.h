//
// Created by zack on 9/9/25.
//

#ifndef ANVIL_STACK_H
#define ANVIL_STACK_H

#include "anvil/common.h"
#include "iterator.h"

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Type definitions
//==============================================================================

/**
 * Node of stack
 */
typedef struct ANVStackNode
{
        void* data;                // Pointer to user data
        struct ANVStackNode* next; // Pointer to next node
} ANVStackNode;

/**
 * Stack structure
 */
typedef struct ANVStack
{
        ANVStackNode* top;   // Pointer to top node
        size_t size;         // Number of elements in stack
        ANVAllocator alloc;  // Custom allocator
} ANVStack;

//==============================================================================
// Creation and destruction functions
//==============================================================================

/**
 * Create a new, empty stack with custom allocator.
 *
 * @param alloc Custom allocator (required)
 * @return Pointer to new Stack, or NULL on failure
 */
ANV_API ANVStack* anv_stack_create(ANVAllocator* alloc);

/**
 * Destroy the stack and free all nodes.
 *
 * @param stack The stack to destroy
 * @param should_free_data Whether to free the data elements
 */
ANV_API void anv_stack_destroy(ANVStack* stack, bool should_free_data);

/**
 * Clear all elements from the stack, but keep the structure intact.
 *
 * @param stack The stack to clear
 * @param should_free_data Whether to free the data elements
 */
ANV_API void anv_stack_clear(ANVStack* stack, bool should_free_data);

//==============================================================================
// Information functions
//==============================================================================

/**
 * Get the number of elements in the stack.
 *
 * @param stack The stack to query
 * @return Number of elements, or 0 if stack is NULL
 */
ANV_API size_t anv_stack_size(const ANVStack* stack);

/**
 * Check if the stack is empty.
 *
 * @param stack The stack to check
 * @return 1 if stack is empty or NULL, 0 if it contains elements
 */
ANV_API int anv_stack_is_empty(const ANVStack* stack);

/**
 * Compare two stacks for equality using the given comparison function.
 *
 * @param stack1 First stack to compare
 * @param stack2 Second stack to compare
 * @param compare Function to compare elements
 * @return 1 if stacks are equal, 0 if not equal, -1 on error
 */
ANV_API int anv_stack_equals(const ANVStack* stack1, const ANVStack* stack2, anv_compare_func compare);

//==============================================================================
// Element access functions
//==============================================================================

/**
 * Get the top element without removing it.
 *
 * @param stack The stack to access
 * @return Pointer to top element data, or NULL if empty or on error
 */
ANV_API void* anv_stack_peek(const ANVStack* stack);

/**
 * Get the top element without removing it.
 *
 * @param stack The stack to access
 * @return Pointer to top element data, or NULL if empty or on error
 *
 * @note Equivalent to anv_stack_peek()
 */
ANV_API void* anv_stack_top(const ANVStack* stack);

//==============================================================================
// Stack operations
//==============================================================================

/**
 * Push an element onto the top of the stack.
 *
 * @param stack The stack to modify
 * @param data Pointer to the data to push (ownership transferred to stack)
 * @return 0 on success, -1 on error
 */
ANV_API int anv_stack_push(ANVStack* stack, void* data);

/**
 * Pop the top element from the stack.
 *
 * @param stack The stack to modify
 * @param should_free_data Whether to free the popped data
 * @return 0 on success, -1 on error (e.g., empty stack)
 */
ANV_API int anv_stack_pop(ANVStack* stack, bool should_free_data);

/**
 * Pop the top element and return its data pointer.
 * The caller is responsible for freeing the returned data if needed.
 *
 * @param stack The stack to modify
 * @return Pointer to the popped data, or NULL if stack is empty or on error
 */
ANV_API void* anv_stack_pop_data(ANVStack* stack);

//==============================================================================
// Higher-order functions
//==============================================================================

/**
 * Apply an action function to each element in the stack.
 *
 * @param stack The stack to process
 * @param action Function applied to each element
 */
ANV_API void anv_stack_for_each(const ANVStack* stack, anv_action_func action);

//==============================================================================
// Stack copying functions
//==============================================================================

/**
 * Create a shallow copy of the stack.
 *
 * @param stack The stack to copy
 * @return A new stack with same structure but sharing data, or NULL on error
 */
ANV_API ANVStack* anv_stack_copy(ANVStack* stack);

/**
 * Create a deep copy of the stack.
 *
 * @param stack The stack to copy
 * @param should_free_data Whether the copy function should handle data freeing
 * @return A new stack with copies of all data, or NULL on error
 */
ANV_API ANVStack* anv_stack_copy_deep(ANVStack* stack, bool should_free_data);

//==============================================================================
// Iterator functions
//==============================================================================

/**
 * Create an iterator for the stack.
 *
 * @param stack The stack to iterate over
 * @return An Iterator object for forward traversal
 */
ANV_API ANVIterator anv_stack_iterator(const ANVStack* stack);

/**
 * Create a new stack from an iterator with custom allocator.
 *
 * This function consumes all elements from the provided iterator and creates
 * a new stack containing those elements.
 *
 * @param it The source iterator (must be valid and support has_next/get/next)
 * @param alloc The custom allocator to use for the new stack
 * @param should_copy If true, creates deep copies of all elements.
 *                    If false, uses elements directly from iterator.
 *                    When true, the allocators copy function must be valid.
 * @return A new stack with elements from iterator, or NULL on error
 *
 * @note NULL elements from the iterator are always filtered out as they indicate
 *       iterator issues rather than valid data.
 * @note The iterator is consumed during this operation - it will be at the end
 *       position after the function completes.
 * @note If should_copy is true and copying fails for any element, the function
 *       cleans up and returns NULL.
 */
ANV_API ANVStack* anv_stack_from_iterator(ANVIterator* it, ANVAllocator* alloc, bool should_copy);

#ifdef __cplusplus
}
#endif

#endif // ANVIL_STACK_H
