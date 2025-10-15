//
// Created by zack on 8/26/25.
//

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

typedef struct TransformState
{
    ANVIterator* base_iterator;   // Source iterator
    anv_transform_func transform; // Transformation function
    void* cached_result;          // Cached transformed result
    int transform_allocates;      // Flag: does transform function allocate memory?
} TransformState;

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

    if (!state->cached_result)
    {
        void* element = state->base_iterator->get(state->base_iterator);
        if (!element)
        {
            return NULL;
        }

        state->cached_result = state->transform(element);
    }

    return state->cached_result;
}

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

    if (state->cached_result && state->transform_allocates)
    {
        anv_alloc_data_deallocate(&it->alloc, state->cached_result);
    }

    state->cached_result = NULL;

    return state->base_iterator->next(state->base_iterator);
}

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

static void transform_destroy(ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return;
    }

    TransformState* state = it->data_state;

    if (state->cached_result && state->transform_allocates)
    {
        anv_alloc_deallocate(&it->alloc, state->cached_result);
    }

    if (state->base_iterator && state->base_iterator->destroy)
    {
        state->base_iterator->destroy(state->base_iterator);
    }

    anv_alloc_deallocate(&it->alloc, state);
    it->data_state = NULL;
}

ANV_API ANVIterator anv_iterator_transform(ANVIterator* it, const ANVAllocator* alloc, const anv_transform_func transform, const bool transform_allocates)
{
    ANVIterator new_it = {0};
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

    TransformState* state = anv_alloc_allocate(alloc, sizeof(TransformState));
    if (!state)
    {
        return new_it;
    }

    state->base_iterator = it;
    state->transform = transform;
    state->cached_result = NULL;
    state->transform_allocates = transform_allocates;

    new_it.alloc = *alloc;
    new_it.data_state = state;

    return new_it;
}

//==============================================================================
// Filter iterator implementation
//==============================================================================

typedef struct FilterState
{
    ANVIterator* base_iterator; // Source iterator
    anv_filter_func filter;     // Predicate function
    void* current_element;      // Cached current element
    int has_current;            // Flag indicating if we have a cached current element
    int current_matches;        // Flag indicating if current element matches filter
} FilterState;

static void position_at_next_match(FilterState* state)
{
    if (state->has_current && state->current_matches)
    {
        return;
    }

    const ANVIterator* base_it = state->base_iterator;

    while (base_it->has_next(base_it))
    {
        void* element = base_it->get(base_it);

        if (element && state->filter && state->filter(element))
        {
            state->current_element = element;
            state->has_current = 1;
            state->current_matches = 1;
            return;
        }

        if (base_it->next(base_it) != 0)
        {
            break;
        }
    }

    state->has_current = 0;
    state->current_matches = 0;
    state->current_element = NULL;
}

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

    position_at_next_match(state);

    return state->current_matches;
}

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

    if (!it->has_next(it))
    {
        return -1;
    }

    if (base_it->next(base_it) != 0)
    {
        return -1;
    }

    state->has_current = 0;
    state->current_matches = 0;
    state->current_element = NULL;

    return 0;
}

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

static void filter_destroy(ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return;
    }

    FilterState* state = it->data_state;

    if (state->base_iterator && state->base_iterator->destroy)
    {
        state->base_iterator->destroy(state->base_iterator);
    }

    anv_alloc_deallocate(&it->alloc, state);
    it->data_state = NULL;
}

ANV_API ANVIterator anv_iterator_filter(ANVIterator* it, const ANVAllocator* alloc, const anv_filter_func filter)
{
    ANVIterator new_it = {0};

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

    FilterState* state = anv_alloc_allocate(alloc, sizeof(FilterState));
    if (!state)
    {
        return new_it;
    }

    state->base_iterator = it;
    state->filter = filter;
    state->current_element = NULL;
    state->has_current = 0;

    new_it.alloc = *alloc;
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

static void* range_get(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return NULL;
    }

    RangeState* state = it->data_state;

    if ((state->step > 0 && state->current < state->end) ||
        (state->step < 0 && state->current > state->end))
    {
        state->cached_value = state->current;
        return &state->cached_value;
    }

    return NULL;
}

static int range_has_next(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return 0;
    }

    const RangeState* state = it->data_state;

    if (state->step > 0)
    {
        return state->current < state->end;
    }

    if (state->step < 0)
    {
        return state->current > state->end;
    }

    return 0;
}

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

static int range_has_prev(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return 0;
    }

    const RangeState* state = it->data_state;

    if (state->step > 0)
    {
        return state->current > state->start;
    }

    if (state->step < 0)
    {
        return state->current < state->start;
    }

    return 0;
}

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

static void range_reset(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return;
    }

    RangeState* state = it->data_state;
    state->current = state->start;
}

static int range_is_valid(const ANVIterator* it)
{
    return it && it->data_state != NULL;
}

static void range_destroy(ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return;
    }

    anv_alloc_deallocate(&it->alloc, it->data_state);
    it->data_state = NULL;
}

ANV_API ANVIterator anv_iterator_range(const ANVAllocator* alloc, const int start, const int end, const int step)
{
    ANVIterator it = {0};

    it.get = range_get;
    it.has_next = range_has_next;
    it.next = range_next;
    it.has_prev = range_has_prev;
    it.prev = range_prev;
    it.reset = range_reset;
    it.is_valid = range_is_valid;
    it.destroy = range_destroy;

    if (step == 0 || !alloc ||
        (start < end && step < 0) ||
        (start > end && step > 0))
    {
        return it;
    }

    RangeState* state = anv_alloc_allocate(alloc, sizeof(RangeState));
    if (!state)
    {
        return it;
    }

    state->start = start;
    state->current = start;
    state->end = end;
    state->step = step;
    state->cached_value = start;

    it.alloc = *alloc;
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

    if (!state->cached_copy)
    {
        void* element = state->base_iterator->get(state->base_iterator);
        if (!element)
        {
            return NULL;
        }
        state->cached_copy = state->copy(element);
    }

    return state->cached_copy;
}

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

    state->cached_copy = NULL;

    return state->base_iterator->next(state->base_iterator);
}

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

static void copy_destroy(ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return;
    }

    CopyState* state = it->data_state;

    if (state->base_iterator && state->base_iterator->destroy)
    {
        state->base_iterator->destroy(state->base_iterator);
    }

    anv_alloc_deallocate(&it->alloc, state);
    it->data_state = NULL;
}

ANV_API ANVIterator anv_iterator_copy(ANVIterator* it, const ANVAllocator* alloc, const anv_copy_func copy)
{
    ANVIterator new_it = {0};

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

    CopyState* state = anv_alloc_allocate(alloc, sizeof(CopyState));
    if (!state)
    {
        return new_it;
    }

    state->base_iterator = it;
    state->copy = copy;
    state->cached_copy = NULL;

    new_it.alloc = *alloc;
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

    if (state->current_count < state->max_count)
    {
        return state->base_iterator->get(state->base_iterator);
    }

    return NULL;
}

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

    if (state->current_count >= state->max_count)
    {
        return 0;
    }

    return state->base_iterator->has_next(state->base_iterator);
}

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

static void take_destroy(ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return;
    }

    TakeState* state = it->data_state;

    if (state->base_iterator && state->base_iterator->destroy)
    {
        state->base_iterator->destroy(state->base_iterator);
    }

    anv_alloc_deallocate(&it->alloc, state);
    it->data_state = NULL;
}

ANV_API ANVIterator anv_iterator_take(ANVIterator* it, const ANVAllocator* alloc, const size_t count)
{
    ANVIterator new_it = {0};

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

    TakeState* state = anv_alloc_allocate(alloc, sizeof(TakeState));
    if (!state)
    {
        return new_it;
    }

    state->base_iterator = it;
    state->max_count = count;
    state->current_count = 0;

    new_it.alloc = *alloc;
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

static void perform_skip(SkipState* state)
{
    if (state->has_skipped)
    {
        return;
    }

    for (size_t i = 0; i < state->skip_count && state->base_iterator->has_next(state->base_iterator); i++)
    {
        if (state->base_iterator->next(state->base_iterator) != 0)
        {
            break;
        }
    }

    state->has_skipped = 1;
}

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
    perform_skip(state);

    return state->base_iterator->get(state->base_iterator);
}

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
    perform_skip(state);

    return state->base_iterator->has_next(state->base_iterator);
}

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
    perform_skip(state);

    return state->base_iterator->next(state->base_iterator);
}

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

static void skip_destroy(ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return;
    }

    SkipState* state = it->data_state;

    if (state->base_iterator && state->base_iterator->destroy)
    {
        state->base_iterator->destroy(state->base_iterator);
    }

    anv_alloc_deallocate(&it->alloc, state);
    it->data_state = NULL;
}

ANV_API ANVIterator anv_iterator_skip(ANVIterator* it, const ANVAllocator* alloc, const size_t count)
{
    ANVIterator new_it = {0};

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

    SkipState* state = anv_alloc_allocate(alloc, sizeof(SkipState));
    if (!state)
    {
        return new_it;
    }

    state->base_iterator = it;
    state->skip_count = count;
    state->has_skipped = 0;

    new_it.alloc = *alloc;
    new_it.data_state = state;

    return new_it;
}

//==============================================================================
// Zip iterator implementation
//==============================================================================

typedef struct ZipState
{
    ANVIterator* iter1;   // First source iterator
    ANVIterator* iter2;   // Second source iterator
    ANVPair* cached_pair; // Cached pair to return pointers to
    int has_cached_pair;  // Flag indicating if cached pair is valid
} ZipState;

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

    if (!state->iter1->has_next(state->iter1) || !state->iter2->has_next(state->iter2))
    {
        return NULL;
    }

    if (!state->has_cached_pair)
    {
        void* elem1 = state->iter1->get(state->iter1);
        void* elem2 = state->iter2->get(state->iter2);

        state->cached_pair->first = elem1;
        state->cached_pair->second = elem2;
        state->cached_pair->alloc = it->alloc;
        state->has_cached_pair = 1;
    }

    return state->cached_pair;
}

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

    return state->iter1->has_next(state->iter1) && state->iter2->has_next(state->iter2);
}

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

    if (!zip_has_next(it))
    {
        return -1;
    }

    const int result1 = state->iter1->next(state->iter1);
    const int result2 = state->iter2->next(state->iter2);

    if (result1 != 0 || result2 != 0)
    {
        return -1;
    }

    state->cached_pair->first = NULL;
    state->cached_pair->second = NULL;
    state->has_cached_pair = 0;

    return 0;
}

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

static void zip_destroy(ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return;
    }

    ZipState* state = it->data_state;

    if (state->iter1 && state->iter1->destroy)
    {
        state->iter1->destroy(state->iter1);
    }

    if (state->iter2 && state->iter2->destroy)
    {
        state->iter2->destroy(state->iter2);
    }

    anv_alloc_deallocate(&it->alloc, state->cached_pair);
    anv_alloc_deallocate(&it->alloc, state);
    it->data_state = NULL;
}

ANV_API ANVIterator anv_iterator_zip(ANVIterator* it1, ANVIterator* it2, const ANVAllocator* alloc)
{
    ANVIterator new_it = {0};

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

    ZipState* state = anv_alloc_allocate(alloc, sizeof(ZipState));
    if (!state)
    {
        return new_it;
    }

    state->cached_pair = anv_pair_create((ANVAllocator*)alloc, NULL, NULL);
    if (!state->cached_pair)
    {
        anv_alloc_deallocate(alloc, state);
        return new_it;
    }

    state->iter1 = it1;
    state->iter2 = it2;
    state->has_cached_pair = 0;

    new_it.alloc = *alloc;
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

    if (!state->base_iterator->has_next(state->base_iterator))
    {
        return NULL;
    }

    void* element = state->base_iterator->get(state->base_iterator);

    state->cached_element.index = state->current_index;
    state->cached_element.element = element;
    state->cached_element.alloc = it->alloc;

    return &state->cached_element;
}

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

    if (!enumerate_has_next(it))
    {
        return -1;
    }

    const int result = state->base_iterator->next(state->base_iterator);
    if (result == 0)
    {
        state->current_index++;
    }

    return result;
}

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

static void enumerate_destroy(ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return;
    }

    EnumerateState* state = it->data_state;

    if (state->base_iterator && state->base_iterator->destroy)
    {
        state->base_iterator->destroy(state->base_iterator);
    }

    anv_alloc_deallocate(&it->alloc, state);
    it->data_state = NULL;
}

ANV_API ANVIterator anv_iterator_enumerate(ANVIterator* it, const ANVAllocator* alloc, const size_t start_index)
{
    ANVIterator new_it = {0};

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

    EnumerateState* state = anv_alloc_allocate(alloc, sizeof(EnumerateState));
    if (!state)
    {
        return new_it;
    }

    state->base_iterator = it;
    state->current_index = start_index;

    state->cached_element.index = start_index;
    state->cached_element.element = NULL;
    state->cached_element.alloc = *alloc;

    new_it.alloc = *alloc;
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

static int repeat_has_next(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return 0;
    }

    const RepeatState* state = it->data_state;
    return state->current_count < state->total_count;
}

static int repeat_next(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return -1;
    }

    RepeatState* state = it->data_state;

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

static int repeat_is_valid(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return 0;
    }

    const RepeatState* state = it->data_state;
    return state->value != NULL;
}

static void repeat_destroy(ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return;
    }

    anv_alloc_deallocate(&it->alloc, it->data_state);
    it->data_state = NULL;
}

ANV_API ANVIterator anv_iterator_repeat(const void* value, const ANVAllocator* alloc, const size_t count)
{
    ANVIterator new_it = {0};

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

    RepeatState* state = anv_alloc_allocate(alloc, sizeof(RepeatState));
    if (!state)
    {
        return new_it;
    }

    state->value = value;
    state->total_count = count;
    state->current_count = 0;

    new_it.alloc = *alloc;
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

    if (current_it->next(current_it) == 0)
    {
        return 0;
    }

    state->current_iterator_index++;
    if (state->current_iterator_index >= state->iterator_count)
    {
        return -1;
    }

    return 0;
}

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

static void chain_destroy(ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return;
    }

    ChainState* state = it->data_state;

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
        anv_alloc_deallocate(&it->alloc, state->iterators);
    }

    anv_alloc_deallocate(&it->alloc, state);
    it->data_state = NULL;
}

ANV_API ANVIterator anv_iterator_chain(ANVIterator* iterators, const size_t iterator_count, const ANVAllocator* alloc)
{
    ANVIterator new_it = {0};

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

    ChainState* state = anv_alloc_allocate(alloc, sizeof(ChainState));
    if (!state)
    {
        return new_it;
    }

    state->iterators = anv_alloc_allocate(alloc, sizeof(ANVIterator) * iterator_count);
    if (!state->iterators)
    {
        anv_alloc_deallocate(alloc, state);
        return new_it;
    }

    for (size_t i = 0; i < iterator_count; i++)
    {
        state->iterators[i] = iterators[i];
    }

    state->iterator_count = iterator_count;
    state->current_iterator_index = 0;

    new_it.alloc = *alloc;
    new_it.data_state = state;

    return new_it;
}