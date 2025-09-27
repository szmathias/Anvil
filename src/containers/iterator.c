//
// Created by zack on 8/26/25.
//
// Implementation of iterator operations, including transform and filter iterators.

#include "iterator.h"
#include "pair.h"

static int invalid_has_prev(const ANVIterator* it)
{
    (void)it;
    return 0;
}

static int invalid_prev(const ANVIterator* it)
{
    (void)it;
    return -1;
}

static void invalid_reset(const ANVIterator* it)
{
    (void)it;
}

//==============================================================================
// Transform iterator implementation
//==============================================================================

/**
 * State structure for transform iterator.
 */
typedef struct TransformState
{
        ANVIterator* base_iterator;   // Source iterator
        anv_transform_func transform; // Transformation function
        void* cached_result;          // Cached transformed result
        int transform_allocates;      // Flag: does transform function allocate memory?
} TransformState;

/**
 * Get current element from transform iterator.
 *
 * Note: The returned pointer is valid until the next iterator operation.
 * Do not free the returned pointer.
 */
static void* transform_get(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return NULL;
    }

    TransformState* state = it->data_state;
    if (!state->base_iterator || !state->base_iterator->get || !state->transform)
    {
        return NULL;
    }

    // Only transform if we don't have a cached result yet
    if (!state->cached_result)
    {
        void* element = state->base_iterator->get(state->base_iterator);
        if (!element)
        {
            return NULL;
        }

        // Apply transformation and cache the result
        state->cached_result = state->transform(element);

        // // Free the base element if it was allocated
        // free_if_allocated(state->base_iterator, element);
    }

    return state->cached_result;
}

/**
 * Check if transform iterator has more elements.
 */
static int transform_has_next(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return 0;
    }

    const TransformState* state = it->data_state;
    if (!state->base_iterator || !state->base_iterator->has_next)
    {
        return 0;
    }

    return state->base_iterator->has_next(state->base_iterator);
}

/**
 * Advance transform iterator to next position.
 */
static int transform_next(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return -1;
    }

    TransformState* state = it->data_state;
    if (!state->base_iterator || !state->base_iterator->next || !state->transform)
    {
        return -1;
    }

    void* element = state->base_iterator->get(state->base_iterator);
    if (!element)
    {
        return -1;
    }

    // Free any previously cached result if it was allocated
    if (state->cached_result && state->transform_allocates)
    {
        anv_alloc_data_free(it->alloc, state->cached_result);
    }

    // Always clear cached result so next get() will recompute it
    state->cached_result = NULL;

    return state->base_iterator->next(state->base_iterator);
}

/**
 * Check if transform iterator is valid.
 */
static int transform_is_valid(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return 0;
    }

    const TransformState* state = it->data_state;
    if (!state->base_iterator || !state->base_iterator->is_valid)
    {
        return 0;
    }

    return state->base_iterator->is_valid(state->base_iterator);
}

/**
 * Free resources used by transform iterator.
 */
static void transform_destroy(ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return;
    }

    TransformState* state = it->data_state;

    // Free cached result if it exists
    if (state->cached_result && state->transform_allocates)
    {
        anv_alloc_free(it->alloc, state->cached_result);
    }

    // Destroy base iterator
    if (state->base_iterator && state->base_iterator->destroy)
    {
        state->base_iterator->destroy(state->base_iterator);
    }

    anv_alloc_free(it->alloc, state);
    it->data_state = NULL;
}

/**
 * Create a transforming iterator that applies a function to each element.
 */
ANV_API ANVIterator anv_iterator_transform(ANVIterator* it, const ANVAllocator* alloc, const anv_transform_func transform, const int transform_allocates)
{
    ANVIterator new_it = {0}; // Initialize all fields to NULL/0

    new_it.get = transform_get;
    new_it.has_next = transform_has_next;
    new_it.next = transform_next;
    new_it.has_prev = invalid_has_prev;
    new_it.prev = invalid_prev;
    new_it.reset = invalid_reset;
    new_it.is_valid = transform_is_valid;
    new_it.destroy = transform_destroy;

    if (!it || !transform || !alloc)
    {
        return new_it;
    }

    TransformState* state = anv_alloc_malloc(alloc, sizeof(TransformState));
    if (!state)
    {
        return new_it;
    }

    state->base_iterator = it;
    state->transform = transform;
    state->cached_result = NULL;
    state->transform_allocates = transform_allocates;

    new_it.alloc = alloc;
    new_it.data_state = state;

    return new_it;
}

//==============================================================================
// Filter iterator implementation
//==============================================================================

/**
 * State structure for filter iterator.
 */
typedef struct FilterState
{
        ANVIterator* base_iterator; // Source iterator
        anv_filter_func filter;     // Predicate function
        void* current_element;      // Cached current element
        int has_current;            // Flag indicating if we have a cached current element
        int current_matches;        // Flag indicating if current element matches filter
} FilterState;

/**
 * Helper function to position filter at next matching element.
 * Uses only get() and next() - no invalidation issues.
 * This is the key function that solves the composition problem.
 */
static void position_at_next_match(FilterState* state)
{
    // If we already have a matching current element, we're done
    if (state->has_current && state->current_matches)
    {
        return;
    }

    const ANVIterator* base_it = state->base_iterator;

    // Search for the next matching element
    while (base_it->has_next(base_it))
    {
        // Peek at the current element without advancing
        void* element = base_it->get(base_it);

        if (element && state->filter && state->filter(element))
        {
            // Found a match - cache it and stop
            state->current_element = element;
            state->has_current = 1;
            state->current_matches = 1;
            return;
        }

        // Element doesn't match, advance to next position
        if (base_it->next(base_it) != 0)
        {
            break; // No more elements
        }
    }

    // No matching element found
    state->has_current = 0;
    state->current_matches = 0;
    state->current_element = NULL;
}

/**
 * Get current element from filter iterator.
 *
 * Note: The returned pointer is valid until the next iterator operation.
 * Do not free the returned pointer.
 */
static void* filter_get(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return NULL;
    }

    FilterState* state = it->data_state;
    if (!state->base_iterator)
    {
        return NULL;
    }

    position_at_next_match(state);

    return state->current_matches ? state->current_element : NULL;
}

/**
 * Check if filter iterator has more elements.
 */
static int filter_has_next(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return 0;
    }

    FilterState* state = it->data_state;
    if (!state->base_iterator)
    {
        return 0;
    }

    // Position at next matching element if needed
    position_at_next_match(state);

    return state->current_matches;
}

/**
 * Advance filter iterator to next position.
 * Returns 0 on success, -1 if no more elements.
 */
static int filter_next(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return -1;
    }

    FilterState* state = it->data_state;
    const ANVIterator* base_it = state->base_iterator;
    if (!base_it)
    {
        return -1;
    }

    // Make sure we're positioned at a matching element
    if (!it->has_next(it))
    {
        return -1;
    }

    if (base_it->next(base_it) != 0)
    {
        return -1;
    }

    // No more elements in base iterator
    state->has_current = 0;
    state->current_matches = 0;
    state->current_element = NULL;

    return 0;
}

/**
 * Check if filter iterator is valid.
 */
static int filter_is_valid(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return 0;
    }

    const FilterState* state = it->data_state;
    if (!state->base_iterator || !state->base_iterator->is_valid)
    {
        return 0;
    }

    return state->base_iterator->is_valid(state->base_iterator);
}

/**
 * Free resources used by filter iterator.
 */
static void filter_destroy(ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return;
    }

    FilterState* state = it->data_state;

    // No need to free cached_element as it points to base iterator's memory
    // Base iterator cleanup will handle it

    // Destroy base iterator
    if (state->base_iterator && state->base_iterator->destroy)
    {
        state->base_iterator->destroy(state->base_iterator);
    }

    anv_alloc_free(it->alloc, state);
    it->data_state = NULL;
}

/**
 * Create a filtering iterator that only yields elements matching a predicate.
 */
ANV_API ANVIterator anv_iterator_filter(ANVIterator* it, const ANVAllocator* alloc, const anv_filter_func filter)
{
    ANVIterator new_it = {0}; // Initialize all fields to NULL/0

    new_it.get = filter_get;
    new_it.has_next = filter_has_next;
    new_it.next = filter_next;
    new_it.has_prev = invalid_has_prev;
    new_it.prev = invalid_prev;
    new_it.reset = invalid_reset;
    new_it.is_valid = filter_is_valid;
    new_it.destroy = filter_destroy;

    if (!it || !filter || !alloc)
    {
        return new_it;
    }

    FilterState* state = anv_alloc_malloc(alloc, sizeof(FilterState));
    if (!state)
    {
        return new_it;
    }

    state->base_iterator = it;
    state->filter = filter;
    state->current_element = NULL;
    state->has_current = 0;

    new_it.alloc = alloc;
    new_it.data_state = state;

    return new_it;
}

//==============================================================================
// Range iterator implementation
//==============================================================================

/**
 * State structure for range iterator.
 */
typedef struct RangeState
{
        int start;        // Starting value (stored for reset/has_prev)
        int current;      // Current value
        int end;          // End value (exclusive)
        int step;         // Increment value
        int cached_value; // Cached value to return pointers to
} RangeState;

/**
 * Get current element from range iterator.
 *
 * Note: The returned pointer is valid until the next iterator operation.
 * Do not free the returned pointer.
 */
static void* range_get(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return NULL;
    }

    RangeState* state = it->data_state;

    // For ongoing iteration, return the current value
    if ((state->step > 0 && state->current < state->end) ||
        (state->step < 0 && state->current > state->end))
    {
        state->cached_value = state->current;
        return &state->cached_value;
    }

    return NULL;
}

/**
 * Check if range iterator has more elements.
 */
static int range_has_next(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return 0;
    }

    const RangeState* state = it->data_state;

    // For all cases, check if next step would be within bounds
    if (state->step > 0)
    {
        return state->current < state->end; // For positive step
    }

    if (state->step < 0)
    {
        return state->current > state->end; // For negative step
    }

    return 0; // Zero step, no more iterations
}

/**
 * Advance range iterator to next position.
 * Returns 0 on success, -1 if no more elements.
 */
static int range_next(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return -1;
    }

    RangeState* state = it->data_state;

    if (!range_has_next(it))
    {
        return -1;
    }
    state->current += state->step;

    return 0;
}

/**
 * Check if range iterator has previous elements.
 */
static int range_has_prev(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return 0;
    }

    const RangeState* state = it->data_state;

    // For all cases, check if previous step would be within bounds
    if (state->step > 0)
    {
        return state->current > state->start; // For positive step
    }

    if (state->step < 0)
    {
        return state->current < state->start; // For negative step
    }

    return 0; // Zero step, no more iterations
}

/**
 * Get previous element from range iterator and move back.
 *
 * Note: The returned pointer is valid until the next iterator operation.
 * Do not free the returned pointer.
 */
static int range_prev(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return -1;
    }

    RangeState* state = it->data_state;

    if (!range_has_prev(it))
    {
        return -1;
    }

    state->current -= state->step;
    return 0;
}

/**
 * Reset range iterator to starting position.
 */
static void range_reset(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return;
    }

    RangeState* state = it->data_state;

    // Reset to initial state using the stored start value
    state->current = state->start;
    // Note: cached_value doesn't need to be reset as it will be overwritten
}

/**
 * Check if range iterator is valid.
 */
static int range_is_valid(const ANVIterator* it)
{
    return it && it->data_state != NULL;
}

/**
 * Free resources used by range iterator.
 */
static void range_destroy(ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return;
    }

    anv_alloc_free(it->alloc, it->data_state);
    it->data_state = NULL;
}

/**
 * Create an iterator that yields integers in a specified range.
 *
 * @param start Starting value (inclusive)
 * @param end Ending value (exclusive)
 * @param step Step value (positive or negative, non-zero)
 * @param alloc The allocator to use for the new iterator's state.
 * @return A new iterator yielding integers in the specified range
 *
 * Note: Pointers returned by get() and next() are valid until the next
 * iterator operation. Do not free these pointers - they point to iterator-owned memory.
 */
ANV_API ANVIterator anv_iterator_range(const int start, const int end, const int step, const ANVAllocator* alloc)
{
    ANVIterator it = {0}; // Initialize all fields to NULL/0

    it.get = range_get;
    it.has_next = range_has_next;
    it.next = range_next;
    it.has_prev = range_has_prev;
    it.prev = range_prev;
    it.reset = range_reset;
    it.is_valid = range_is_valid;
    it.destroy = range_destroy;

    // Handle invalid step
    if (step == 0 || !alloc ||
        (start < end && step < 0) ||
        (start > end && step > 0))
    {
        return it; // Return invalid iterator with NULL data_state
    }

    RangeState* state = anv_alloc_malloc(alloc, sizeof(RangeState));
    if (!state)
    {
        return it;
    }

    state->start = start;
    state->current = start;
    state->end = end;
    state->step = step;
    state->cached_value = start; // Initialize cached value

    it.alloc = alloc;
    it.data_state = state;

    return it;
}

//==============================================================================
// Copy iterator implementation
//==============================================================================

/**
 * State structure for copy iterator.
 */
typedef struct CopyState
{
        ANVIterator* base_iterator; // Source iterator
        anv_copy_func copy;         // Copy function
        void* cached_copy;          // Cached copied element (user owns this)
} CopyState;

/**
 * Get current element from copy iterator.
 * Returns a deep copy that the USER OWNS and must free.
 */
static void* copy_get(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return NULL;
    }

    CopyState* state = it->data_state;
    if (!state->base_iterator || !state->copy)
    {
        return NULL;
    }

    // Only create copy if we don't have a cached one yet
    if (!state->cached_copy)
    {
        void* element = state->base_iterator->get(state->base_iterator);
        if (!element)
        {
            return NULL;
        }

        // Create copy using the provided copy function
        state->cached_copy = state->copy(element);
    }

    return state->cached_copy;
}

/**
 * Check if copy iterator has more elements.
 */
static int copy_has_next(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return 0;
    }

    const CopyState* state = it->data_state;
    if (!state->base_iterator || !state->base_iterator->has_next)
    {
        return 0;
    }

    return state->base_iterator->has_next(state->base_iterator);
}

/**
 * Advance copy iterator to next position.
 * Returns 0 on success, -1 if no more elements.
 */
static int copy_next(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return -1;
    }

    CopyState* state = it->data_state;
    if (!state->base_iterator)
    {
        return -1;
    }

    // Clear cached copy so next get() will create a new one
    // Note: We don't free cached_copy here because the user owns it
    state->cached_copy = NULL;

    return state->base_iterator->next(state->base_iterator);
}

/**
 * Check if copy iterator is valid.
 */
static int copy_is_valid(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return 0;
    }

    const CopyState* state = it->data_state;
    if (!state->base_iterator || !state->base_iterator->is_valid || !state->copy)
    {
        return 0;
    }

    return state->base_iterator->is_valid(state->base_iterator);
}

/**
 * Free resources used by copy iterator.
 * Note: Does NOT free cached_copy as that is owned by the user.
 */
static void copy_destroy(ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return;
    }

    CopyState* state = it->data_state;

    // Do NOT free cached_copy - it's owned by the user

    // Destroy base iterator
    if (state->base_iterator && state->base_iterator->destroy)
    {
        state->base_iterator->destroy(state->base_iterator);
    }

    anv_alloc_free(it->alloc, state);
    it->data_state = NULL;
}

/**
 * Create a copy iterator that returns deep copies of elements.
 */
ANV_API ANVIterator anv_iterator_copy(ANVIterator* it, const ANVAllocator* alloc, const anv_copy_func copy)
{
    ANVIterator new_it = {0}; // Initialize all fields to NULL/0

    new_it.get = copy_get;
    new_it.has_next = copy_has_next;
    new_it.next = copy_next;
    new_it.has_prev = invalid_has_prev;
    new_it.prev = invalid_prev;
    new_it.reset = invalid_reset;
    new_it.is_valid = copy_is_valid;
    new_it.destroy = copy_destroy;

    if (!it || !copy || !alloc)
    {
        return new_it;
    }

    CopyState* state = anv_alloc_malloc(alloc, sizeof(CopyState));
    if (!state)
    {
        return new_it;
    }

    state->base_iterator = it;
    state->copy = copy;
    state->cached_copy = NULL;

    new_it.alloc = alloc;
    new_it.data_state = state;

    return new_it;
}

//==============================================================================
// Take iterator implementation
//==============================================================================

/**
 * State structure for take iterator.
 */
typedef struct TakeState
{
        ANVIterator* base_iterator; // Source iterator
        size_t max_count;           // Maximum number of elements to yield
        size_t current_count;       // Number of elements yielded so far
} TakeState;

/**
 * Get current element from take iterator.
 *
 * Note: The returned pointer is valid until the next iterator operation.
 * Do not free the returned pointer.
 */
static void* take_get(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return NULL;
    }

    const TakeState* state = it->data_state;
    if (!state->base_iterator || !state->base_iterator->get)
    {
        return NULL;
    }

    // Only return element if we haven't reached the limit
    if (state->current_count < state->max_count)
    {
        return state->base_iterator->get(state->base_iterator);
    }

    return NULL;
}

/**
 * Checks if the take iterator has more elements.
 */
static int take_has_next(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return 0;
    }

    const TakeState* state = it->data_state;
    if (!state->base_iterator || !state->base_iterator->has_next)
    {
        return 0;
    }

    // Check if we have reached the limit
    if (state->current_count >= state->max_count)
    {
        return 0;
    }

    return state->base_iterator->has_next(state->base_iterator);
}

/**
 * Advance take iterator to next position.
 * Returns 0 on success, -1 if no more elements.
 */
static int take_next(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return -1;
    }

    TakeState* state = it->data_state;
    if (!state->base_iterator || !state->base_iterator->next)
    {
        return -1;
    }

    // Check if we have reached the limit
    if (state->current_count >= state->max_count)
    {
        return -1;
    }

    const int result = state->base_iterator->next(state->base_iterator);
    if (result == 0)
    {
        state->current_count++;
    }

    return result;
}

/**
 * Check if the take iterator is valid.
 */
static int take_is_valid(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return 0;
    }

    const TakeState* state = it->data_state;
    if (!state->base_iterator || !state->base_iterator->is_valid)
    {
        return 0;
    }

    return state->base_iterator->is_valid(state->base_iterator);
}

/**
 * Free resources used by take iterator.
 */
static void take_destroy(ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return;
    }

    TakeState* state = it->data_state;

    // Destroy base iterator
    if (state->base_iterator && state->base_iterator->destroy)
    {
        state->base_iterator->destroy(state->base_iterator);
    }

    anv_alloc_free(it->alloc, state);
    it->data_state = NULL;
}

/**
 * Create a take iterator that limits iteration to first N elements.
 */
ANV_API ANVIterator anv_iterator_take(ANVIterator* it, const ANVAllocator* alloc, const size_t count)
{
    ANVIterator new_it = {0}; // Initialize all fields to NULL/0

    new_it.get = take_get;
    new_it.has_next = take_has_next;
    new_it.next = take_next;
    new_it.has_prev = invalid_has_prev;
    new_it.prev = invalid_prev;
    new_it.reset = invalid_reset;
    new_it.is_valid = take_is_valid;
    new_it.destroy = take_destroy;

    if (!it || !alloc)
    {
        return new_it;
    }

    TakeState* state = anv_alloc_malloc(alloc, sizeof(TakeState));
    if (!state)
    {
        return new_it;
    }

    state->base_iterator = it;
    state->max_count = count;
    state->current_count = 0;

    new_it.alloc = alloc;
    new_it.data_state = state;

    return new_it;
}

//==============================================================================
// Skip iterator implementation
//==============================================================================

/**
 * State structure for skip iterator.
 */
typedef struct SkipState
{
        ANVIterator* base_iterator; // Source iterator
        size_t skip_count;          // Number of elements to skip
        int has_skipped;            // Flag indicating if we have performed the skip
} SkipState;

/**
 * Helper function to perform the initial skip operation.
 */
static void perform_skip(SkipState* state)
{
    if (state->has_skipped)
    {
        return;
    }

    // Skip the specified number of elements
    for (size_t i = 0; i < state->skip_count && state->base_iterator->has_next(state->base_iterator); i++)
    {
        if (state->base_iterator->next(state->base_iterator) != 0)
        {
            break; // Error advancing iterator
        }
    }

    state->has_skipped = 1;
}

/**
 * Get current element from skip iterator.
 *
 * Note: The returned pointer is valid until the next iterator operation.
 * Do not free the returned pointer.
 */
static void* skip_get(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return NULL;
    }

    SkipState* state = it->data_state;
    if (!state->base_iterator || !state->base_iterator->get)
    {
        return NULL;
    }

    // Perform skip if not done yet
    perform_skip(state);

    return state->base_iterator->get(state->base_iterator);
}

/**
 * Check if skip iterator has more elements.
 */
static int skip_has_next(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return 0;
    }

    SkipState* state = it->data_state;
    if (!state->base_iterator || !state->base_iterator->has_next)
    {
        return 0;
    }

    // Perform skip if not done yet
    perform_skip(state);

    return state->base_iterator->has_next(state->base_iterator);
}

/**
 * Advance skip iterator to next position.
 * Returns 0 on success, -1 if no more elements.
 */
static int skip_next(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return -1;
    }

    SkipState* state = it->data_state;
    if (!state->base_iterator || !state->base_iterator->next)
    {
        return -1;
    }

    // Perform skip if not done yet
    perform_skip(state);

    return state->base_iterator->next(state->base_iterator);
}

/**
 * Check if skip iterator is valid.
 */
static int skip_is_valid(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return 0;
    }

    const SkipState* state = it->data_state;
    if (!state->base_iterator || !state->base_iterator->is_valid)
    {
        return 0;
    }

    return state->base_iterator->is_valid(state->base_iterator);
}

/**
 * Free resources used by skip iterator.
 */
static void skip_destroy(ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return;
    }

    SkipState* state = it->data_state;

    // Destroy base iterator
    if (state->base_iterator && state->base_iterator->destroy)
    {
        state->base_iterator->destroy(state->base_iterator);
    }

    anv_alloc_free(it->alloc, state);
    it->data_state = NULL;
}

/**
 * Create a skip iterator that skips first N elements, then yields the rest.
 */
ANV_API ANVIterator anv_iterator_skip(ANVIterator* it, const ANVAllocator* alloc, const size_t count)
{
    ANVIterator new_it = {0}; // Initialize all fields to NULL/0

    new_it.get = skip_get;
    new_it.has_next = skip_has_next;
    new_it.next = skip_next;
    new_it.has_prev = invalid_has_prev;
    new_it.prev = invalid_prev;
    new_it.reset = invalid_reset;
    new_it.is_valid = skip_is_valid;
    new_it.destroy = skip_destroy;

    if (!it || !alloc)
    {
        return new_it;
    }

    SkipState* state = anv_alloc_malloc(alloc, sizeof(SkipState));
    if (!state)
    {
        return new_it;
    }

    state->base_iterator = it;
    state->skip_count = count;
    state->has_skipped = 0;

    new_it.alloc = alloc;
    new_it.data_state = state;

    return new_it;
}

//==============================================================================
// Zip iterator implementation
//==============================================================================

/**
 * State structure for zip iterator.
 */
typedef struct ZipState
{
        ANVIterator* iter1;   // First source iterator
        ANVIterator* iter2;   // Second source iterator
        ANVPair* cached_pair; // Cached pair to return pointers to
        int has_cached_pair;  // Flag indicating if cached pair is valid
} ZipState;

/**
 * Get current element from zip iterator.
 * Returns a ANVPair containing elements from both iterators.
 *
 * Note: The returned pointer is valid until the next iterator operation.
 * Do not free the returned pointer.
 */
static void* zip_get(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return NULL;
    }

    ZipState* state = it->data_state;
    if (!state->iter1 || !state->iter2 ||
        !state->iter1->get || !state->iter2->get)
    {
        return NULL;
    }

    // Check if both iterators have elements
    if (!state->iter1->has_next(state->iter1) || !state->iter2->has_next(state->iter2))
    {
        return NULL;
    }

    if (!state->has_cached_pair)
    {
        // Get elements from both iterators
        void* elem1 = state->iter1->get(state->iter1);
        void* elem2 = state->iter2->get(state->iter2);

        // Update cached pair
        state->cached_pair->first = elem1;
        state->cached_pair->second = elem2;
        state->cached_pair->alloc = (ANVAllocator*)it->alloc;
        state->has_cached_pair = 1;
    }

    return state->cached_pair;
}

/**
 * Check if zip iterator has more elements.
 */
static int zip_has_next(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return 0;
    }

    const ZipState* state = it->data_state;
    if (!state->iter1 || !state->iter2 ||
        !state->iter1->has_next || !state->iter2->has_next)
    {
        return 0;
    }

    // Both iterators must have elements
    return state->iter1->has_next(state->iter1) && state->iter2->has_next(state->iter2);
}

/**
 * Advance zip iterator to next position.
 * Returns 0 on success, -1 if no more elements.
 */
static int zip_next(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return -1;
    }

    ZipState* state = it->data_state;
    if (!state->iter1 || !state->iter2 ||
        !state->iter1->next || !state->iter2->next)
    {
        return -1;
    }

    // Check if we can advance both iterators
    if (!zip_has_next(it))
    {
        return -1;
    }

    // Advance both iterators
    const int result1 = state->iter1->next(state->iter1);
    const int result2 = state->iter2->next(state->iter2);

    // If either fails, the zip iterator fails
    if (result1 != 0 || result2 != 0)
    {
        return -1;
    }

    state->cached_pair->first = NULL;
    state->cached_pair->second = NULL;
    state->has_cached_pair = 0;

    return 0;
}

/**
 * Check if zip iterator is valid.
 */
static int zip_is_valid(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return 0;
    }

    const ZipState* state = it->data_state;
    if (!state->iter1 || !state->iter2 ||
        !state->iter1->is_valid || !state->iter2->is_valid)
    {
        return 0;
    }

    return state->iter1->is_valid(state->iter1) && state->iter2->is_valid(state->iter2);
}

/**
 * Free resources used by zip iterator.
 */
static void zip_destroy(ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return;
    }

    ZipState* state = it->data_state;

    // Destroy both source iterators
    if (state->iter1 && state->iter1->destroy)
    {
        state->iter1->destroy(state->iter1);
    }

    if (state->iter2 && state->iter2->destroy)
    {
        state->iter2->destroy(state->iter2);
    }

    anv_alloc_free(it->alloc, state->cached_pair);
    anv_alloc_free(it->alloc, state);
    it->data_state = NULL;
}

/**
 * Create a zip iterator that combines elements from two iterators pairwise.
 */
ANV_API ANVIterator anv_iterator_zip(ANVIterator* it1, ANVIterator* it2, const ANVAllocator* alloc)
{
    ANVIterator new_it = {0}; // Initialize all fields to NULL/0

    new_it.get = zip_get;
    new_it.has_next = zip_has_next;
    new_it.next = zip_next;
    new_it.has_prev = invalid_has_prev;
    new_it.prev = invalid_prev;
    new_it.reset = invalid_reset;
    new_it.is_valid = zip_is_valid;
    new_it.destroy = zip_destroy;

    if (!it1 || !it2 || !alloc)
    {
        return new_it;
    }

    ZipState* state = anv_alloc_malloc(alloc, sizeof(ZipState));
    if (!state)
    {
        return new_it;
    }

    state->cached_pair = anv_pair_create((ANVAllocator*)alloc, NULL, NULL);
    if (!state->cached_pair)
    {
        anv_alloc_free(alloc, state);
        return new_it;
    }

    state->iter1 = it1;
    state->iter2 = it2;
    state->has_cached_pair = 0;

    new_it.alloc = alloc;
    new_it.data_state = state;

    return new_it;
}

//==============================================================================
// Enumerate iterator implementation
//==============================================================================

/**
 * State structure for enumerate iterator.
 */
typedef struct EnumerateState
{
        ANVIterator* base_iterator;       // Source iterator
        size_t current_index;             // Current index counter
        ANVIndexedElement cached_element; // Cached indexed element to return pointers to
} EnumerateState;

/**
 * Get current element from enumerate iterator.
 * Returns a ANVIndexedElement containing both index and element data.
 *
 * Note: The returned pointer is valid until the next iterator operation.
 * Do not free the returned pointer.
 */
static void* enumerate_get(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return NULL;
    }

    EnumerateState* state = it->data_state;
    if (!state->base_iterator || !state->base_iterator->get)
    {
        return NULL;
    }

    // Check if base iterator has elements
    if (!state->base_iterator->has_next(state->base_iterator))
    {
        return NULL;
    }

    // Get element from base iterator
    void* element = state->base_iterator->get(state->base_iterator);

    // Update cached indexed element
    state->cached_element.index = state->current_index;
    state->cached_element.element = element;
    state->cached_element.alloc = (ANVAllocator*)it->alloc;

    return &state->cached_element;
}

/**
 * Check if enumerate iterator has more elements.
 */
static int enumerate_has_next(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return 0;
    }

    const EnumerateState* state = it->data_state;
    if (!state->base_iterator || !state->base_iterator->has_next)
    {
        return 0;
    }

    return state->base_iterator->has_next(state->base_iterator);
}

/**
 * Advance enumerate iterator to next position.
 * Returns 0 on success, -1 if no more elements.
 */
static int enumerate_next(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return -1;
    }

    EnumerateState* state = it->data_state;
    if (!state->base_iterator || !state->base_iterator->next)
    {
        return -1;
    }

    // Check if we can advance
    if (!enumerate_has_next(it))
    {
        return -1;
    }

    // Advance base iterator and increment index
    const int result = state->base_iterator->next(state->base_iterator);
    if (result == 0)
    {
        state->current_index++;
    }

    return result;
}

/**
 * Check if enumerate iterator is valid.
 */
static int enumerate_is_valid(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return 0;
    }

    const EnumerateState* state = it->data_state;
    if (!state->base_iterator || !state->base_iterator->is_valid)
    {
        return 0;
    }

    return state->base_iterator->is_valid(state->base_iterator);
}

/**
 * Free resources used by enumerate iterator.
 */
static void enumerate_destroy(ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return;
    }

    EnumerateState* state = it->data_state;

    // Destroy base iterator
    if (state->base_iterator && state->base_iterator->destroy)
    {
        state->base_iterator->destroy(state->base_iterator);
    }

    anv_alloc_free(it->alloc, state);
    it->data_state = NULL;
}

/**
 * Create an enumerate iterator that adds index numbers to elements.
 */
ANV_API ANVIterator anv_iterator_enumerate(ANVIterator* it, const ANVAllocator* alloc, const size_t start_index)
{
    ANVIterator new_it = {0}; // Initialize all fields to NULL/0

    new_it.get = enumerate_get;
    new_it.has_next = enumerate_has_next;
    new_it.next = enumerate_next;
    new_it.has_prev = invalid_has_prev;
    new_it.prev = invalid_prev;
    new_it.reset = invalid_reset;
    new_it.is_valid = enumerate_is_valid;
    new_it.destroy = enumerate_destroy;

    if (!it || !alloc)
    {
        return new_it;
    }

    EnumerateState* state = anv_alloc_malloc(alloc, sizeof(EnumerateState));
    if (!state)
    {
        return new_it;
    }

    state->base_iterator = it;
    state->current_index = start_index;

    // Initialize cached indexed element
    state->cached_element.index = start_index;
    state->cached_element.element = NULL;
    state->cached_element.alloc = (ANVAllocator*)alloc;

    new_it.alloc = alloc;
    new_it.data_state = state;

    return new_it;
}

//==============================================================================
// Repeat iterator implementation
//==============================================================================

/**
 * State structure for repeat iterator.
 */
typedef struct RepeatState
{
        const void* value;    // Pointer to the value to repeat (not owned)
        size_t total_count;   // Total number of repetitions
        size_t current_count; // Current iteration count (0-based)
} RepeatState;

/**
 * Get current element from repeat iterator.
 * Returns the same value pointer for each iteration.
 *
 * Note: The returned pointer is valid until the iterator is destroyed.
 * Do not free the returned pointer.
 */
static void* repeat_get(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return NULL;
    }

    const RepeatState* state = it->data_state;

    // Check if we still have repetitions left
    if (state->current_count < state->total_count)
    {
        return (void*)state->value;
    }

    return NULL;
}

/**
 * Check if repeat iterator has more elements.
 */
static int repeat_has_next(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return 0;
    }

    const RepeatState* state = it->data_state;
    return state->current_count < state->total_count;
}

/**
 * Advance repeat iterator to next position.
 * Returns 0 on success, -1 if no more elements.
 */
static int repeat_next(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return -1;
    }

    RepeatState* state = it->data_state;

    // Check if we have more repetitions
    if (state->current_count >= state->total_count)
    {
        return -1;
    }

    state->current_count++;
    return 0;
}

static void repeat_reset(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return;
    }

    RepeatState* state = it->data_state;
    state->current_count = 0;
}

/**
 * Check if repeat iterator is valid.
 */
static int repeat_is_valid(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return 0;
    }

    const RepeatState* state = it->data_state;
    return state->value != NULL;
}

/**
 * Free resources used by repeat iterator.
 */
static void repeat_destroy(ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return;
    }

    // Note: We don't free the value pointer since we don't own it
    anv_alloc_free(it->alloc, it->data_state);
    it->data_state = NULL;
}

/**
 * Create a repeat iterator that yields the same value N times.
 */
ANV_API ANVIterator anv_iterator_repeat(const void* value, const ANVAllocator* alloc, const size_t count)
{
    ANVIterator new_it = {0}; // Initialize all fields to NULL/0

    new_it.get = repeat_get;
    new_it.has_next = repeat_has_next;
    new_it.next = repeat_next;
    new_it.has_prev = invalid_has_prev;
    new_it.prev = invalid_prev;
    new_it.reset = repeat_reset;
    new_it.is_valid = repeat_is_valid;
    new_it.destroy = repeat_destroy;

    if (!value || !alloc)
    {
        return new_it;
    }

    RepeatState* state = anv_alloc_malloc(alloc, sizeof(RepeatState));
    if (!state)
    {
        return new_it;
    }

    state->value = value;
    state->total_count = count;
    state->current_count = 0;

    new_it.alloc = alloc;
    new_it.data_state = state;

    return new_it;
}

//==============================================================================
// Chain iterator implementation
//==============================================================================

/**
 * State structure for chain iterator.
 */
typedef struct ChainState
{
        ANVIterator* iterators;        // Array of source iterators (owned)
        size_t iterator_count;         // Number of iterators in the array
        size_t current_iterator_index; // Index of currently active iterator
} ChainState;

/**
 * Get current element from chain iterator.
 * Returns element from the currently active iterator.
 *
 * Note: The returned pointer is valid until the next iterator operation.
 * Do not free the returned pointer.
 */
static void* chain_get(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return NULL;
    }

    const ChainState* state = it->data_state;
    if (!state->iterators || state->current_iterator_index >= state->iterator_count)
    {
        return NULL;
    }

    const ANVIterator* current_it = &state->iterators[state->current_iterator_index];
    if (!current_it->get)
    {
        return NULL;
    }

    return current_it->get(current_it);
}

/**
 * Check if chain iterator has more elements.
 */
static int chain_has_next(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return 0;
    }

    ChainState* state = it->data_state;
    if (!state->iterators)
    {
        return 0;
    }

    // Check from current iterator onwards
    for (size_t i = state->current_iterator_index; i < state->iterator_count; i++)
    {
        const ANVIterator* current_it = &state->iterators[i];
        if (current_it->has_next && current_it->has_next(current_it))
        {
            return 1;
        }
        state->current_iterator_index++;
    }

    return 0;
}

/**
 * Advance chain iterator to next position.
 * Returns 0 on success, -1 if no more elements.
 */
static int chain_next(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return -1;
    }

    ChainState* state = it->data_state;
    if (!state->iterators || state->current_iterator_index >= state->iterator_count)
    {
        return -1;
    }

    const ANVIterator* current_it = &state->iterators[state->current_iterator_index];

    // Try to advance current iterator
    if (current_it->next(current_it) == 0)
    {
        return 0;
    }

    // Current iterator exhausted, move to next one
    state->current_iterator_index++;

    // Check if we have more iterators
    if (state->current_iterator_index >= state->iterator_count)
    {
        return -1;
    }

    return 0;
}

/**
 * Check if chain iterator is valid.
 */
static int chain_is_valid(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return 0;
    }

    const ChainState* state = it->data_state;
    if (!state->iterators || state->iterator_count == 0)
    {
        return 0;
    }

    // Check if at least one iterator is valid
    for (size_t i = 0; i < state->iterator_count; i++)
    {
        const ANVIterator* current_it = &state->iterators[i];
        if (current_it->is_valid && current_it->is_valid(current_it))
        {
            return 1;
        }
    }

    return 0;
}

/**
 * Free resources used by chain iterator.
 */
static void chain_destroy(ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return;
    }

    ChainState* state = it->data_state;

    // Destroy all source iterators
    if (state->iterators)
    {
        for (size_t i = 0; i < state->iterator_count; i++)
        {
            ANVIterator* current_it = &state->iterators[i];
            if (current_it->destroy)
            {
                current_it->destroy(current_it);
            }
        }
        anv_alloc_free(it->alloc, state->iterators);
    }

    anv_alloc_free(it->alloc, state);
    it->data_state = NULL;
}

/**
 * Create a chain iterator that combines multiple iterators sequentially.
 */
ANV_API ANVIterator anv_iterator_chain(ANVIterator* iterators, const size_t iterator_count, const ANVAllocator* alloc)
{
    ANVIterator new_it = {0}; // Initialize all fields to NULL/0

    new_it.get = chain_get;
    new_it.has_next = chain_has_next;
    new_it.next = chain_next;
    new_it.has_prev = invalid_has_prev;
    new_it.prev = invalid_prev;
    new_it.reset = invalid_reset;
    new_it.is_valid = chain_is_valid;
    new_it.destroy = chain_destroy;

    if (!iterators || iterator_count == 0 || !alloc)
    {
        return new_it;
    }

    ChainState* state = anv_alloc_malloc(alloc, sizeof(ChainState));
    if (!state)
    {
        return new_it;
    }

    // Allocate array to store copies of the iterators
    state->iterators = anv_alloc_malloc(alloc, sizeof(ANVIterator) * iterator_count);
    if (!state->iterators)
    {
        anv_alloc_free(alloc, state);
        return new_it;
    }

    // Copy the iterators to our internal array
    for (size_t i = 0; i < iterator_count; i++)
    {
        state->iterators[i] = iterators[i];
    }

    state->iterator_count = iterator_count;
    state->current_iterator_index = 0;

    new_it.alloc = alloc;
    new_it.data_state = state;

    return new_it;
}
