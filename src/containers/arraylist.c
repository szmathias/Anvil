//
// Created by zack on 9/2/25.
//

#include <stdint.h>
#include <string.h>

#include "arraylist.h"

// Default initial capacity for new ArrayLists
#define DEFAULT_CAPACITY 16
// Growth factor for capacity expansion (1.5x)
#define GROWTH_FACTOR 1.5f

//==============================================================================
// Private helper functions
//==============================================================================

/**
 * Ensure the ArrayList has at least the specified capacity.
 * Grows the array if needed using the growth factor.
 */
static int ensure_capacity(ANVArrayList* list, const size_t min_capacity)
{
    if (!list)
    {
        return -1;
    }

    if (list->capacity >= min_capacity)
    {
        return 0; // Already has enough capacity
    }

    // Calculate new capacity using growth factor
    size_t new_capacity = list->capacity;
    if (new_capacity == 0)
    {
        new_capacity = DEFAULT_CAPACITY;
    }

    while (new_capacity < min_capacity)
    {
        // Use integer arithmetic to avoid floating point conversion warning
        size_t next_capacity = new_capacity + (new_capacity >> 1); // Equivalent to * 1.5
        // Check for overflow and ensure we always grow
        if (next_capacity <= new_capacity)
        {
            next_capacity = new_capacity + 1;
        }
        new_capacity = next_capacity;
    }

    // Reallocate the data array
    void** new_data = anv_alloc_allocate(&list->alloc, new_capacity * sizeof(void*));
    if (!new_data)
    {
        return -1;
    }

    // Copy existing data
    if (list->data && list->size > 0)
    {
        memcpy(new_data, list->data, list->size * sizeof(void*));
    }

    // Free old data array
    anv_alloc_deallocate(&list->alloc, list->data);

    list->data = new_data;
    list->capacity = new_capacity;
    return 0;
}

/**
 * Merge sort implementation for sorting the ArrayList.
 */
static void merge_sort_recursive(void** arr, void** temp, const size_t left, const size_t right, const anv_compare_func compare)
{
    if (left >= right)
    {
        return;
    }

    const size_t mid = left + (right - left) / 2;
    merge_sort_recursive(arr, temp, left, mid, compare);
    merge_sort_recursive(arr, temp, mid + 1, right, compare);

    // Merge the sorted halves
    size_t i = left, j = mid + 1, k = left;

    while (i <= mid && j <= right)
    {
        if (compare(arr[i], arr[j]) <= 0)
        {
            temp[k++] = arr[i++];
        }
        else
        {
            temp[k++] = arr[j++];
        }
    }

    while (i <= mid)
    {
        temp[k++] = arr[i++];
    }

    while (j <= right)
    {
        temp[k++] = arr[j++];
    }

    // Copy back to original array
    for (i = left; i <= right; i++)
    {
        arr[i] = temp[i];
    }
}

//==============================================================================
// Creation and destruction functions
//==============================================================================

ANV_API ANVArrayList* anv_arraylist_create(ANVAllocator* alloc, const size_t initial_capacity)
{
    if (!alloc)
    {
        return NULL;
    }

    ANVArrayList* list = anv_alloc_allocate(alloc, sizeof(ANVArrayList));
    if (!list)
    {
        return NULL;
    }

    list->alloc = *alloc;
    list->size = 0;
    list->capacity = 0;
    list->data = NULL;

    if (initial_capacity > 0)
    {
        if (ensure_capacity(list, initial_capacity) != 0)
        {
            anv_alloc_deallocate(alloc, list);
            return NULL;
        }
    }

    return list;
}

ANV_API void anv_arraylist_destroy(ANVArrayList* list, const bool should_free_data)
{
    if (!list)
    {
        return;
    }

    anv_arraylist_clear(list, should_free_data);

    anv_alloc_deallocate(&list->alloc, list->data);
    anv_alloc_deallocate(&list->alloc, list);
}

ANV_API void anv_arraylist_clear(ANVArrayList* list, const bool should_free_data)
{
    if (!list)
    {
        return;
    }

    if (should_free_data)
    {
        for (size_t i = 0; i < list->size; i++)
        {
            if (list->data[i])
            {
                anv_alloc_data_deallocate(&list->alloc, list->data[i]);
            }
        }
    }

    list->size = 0;
}

//==============================================================================
// Information functions
//==============================================================================

ANV_API size_t anv_arraylist_size(const ANVArrayList* list)
{
    return list ? list->size : 0;
}

ANV_API size_t anv_arraylist_capacity(const ANVArrayList* list)
{
    return list ? list->capacity : 0;
}

ANV_API int anv_arraylist_is_empty(const ANVArrayList* list)
{
    return !list || list->size == 0;
}

ANV_API size_t anv_arraylist_find(const ANVArrayList* list, const void* data, const anv_compare_func compare)
{
    if (!list || !data || !compare)
    {
        return SIZE_MAX;
    }

    for (size_t i = 0; i < list->size; i++)
    {
        if (compare(list->data[i], data) == 0)
        {
            return i;
        }
    }

    return SIZE_MAX;
}

ANV_API bool anv_arraylist_equals(const ANVArrayList* list1, const ANVArrayList* list2, const anv_compare_func compare)
{
    if (!list1 || !list2 || !compare)
    {
        return false;
    }

    if (list1->size != list2->size)
    {
        return false;
    }

    for (size_t i = 0; i < list1->size; i++)
    {
        if (compare(list1->data[i], list2->data[i]) != 0)
        {
            return false;
        }
    }

    return true;
}

//==============================================================================
// Element access functions
//==============================================================================

ANV_API void* anv_arraylist_get(const ANVArrayList* list, const size_t index)
{
    if (!list || index >= list->size)
    {
        return NULL;
    }

    return list->data[index];
}

ANV_API int anv_arraylist_set(ANVArrayList* list, const size_t index, void* data, const bool should_free_old)
{
    if (!list || index >= list->size)
    {
        return -1;
    }

    if (should_free_old && list->data[index])
    {
        anv_alloc_data_deallocate(&list->alloc, list->data[index]);
    }

    list->data[index] = data;
    return 0;
}

ANV_API void* anv_arraylist_front(const ANVArrayList* list)
{
    return anv_arraylist_get(list, 0);
}

ANV_API void* anv_arraylist_back(const ANVArrayList* list)
{
    if (!list || list->size == 0)
    {
        return NULL;
    }
    return anv_arraylist_get(list, list->size - 1);
}

//==============================================================================
// Insertion functions
//==============================================================================

ANV_API int anv_arraylist_push_back(ANVArrayList* list, void* data)
{
    if (!list)
    {
        return -1;
    }

    if (ensure_capacity(list, list->size + 1) != 0)
    {
        return -1;
    }

    list->data[list->size] = data;
    list->size++;
    return 0;
}

ANV_API int anv_arraylist_push_front(ANVArrayList* list, void* data)
{
    return anv_arraylist_insert(list, 0, data);
}

ANV_API int anv_arraylist_insert(ANVArrayList* list, const size_t index, void* data)
{
    if (!list || index > list->size)
    {
        return -1;
    }

    if (ensure_capacity(list, list->size + 1) != 0)
    {
        return -1;
    }

    // Shift elements to the right
    for (size_t i = list->size; i > index; i--)
    {
        list->data[i] = list->data[i - 1];
    }

    list->data[index] = data;
    list->size++;
    return 0;
}

//==============================================================================
// Removal functions
//==============================================================================

ANV_API int anv_arraylist_pop_back(ANVArrayList* list, const bool should_free_data)
{
    if (!list || list->size == 0)
    {
        return -1;
    }

    return anv_arraylist_remove_at(list, list->size - 1, should_free_data);
}

ANV_API int anv_arraylist_pop_front(ANVArrayList* list, const bool should_free_data)
{
    return anv_arraylist_remove_at(list, 0, should_free_data);
}

ANV_API int anv_arraylist_remove_at(ANVArrayList* list, const size_t index, const bool should_free_data)
{
    if (!list || index >= list->size)
    {
        return -1;
    }

    if (should_free_data && list->data[index])
    {
        anv_alloc_data_deallocate(&list->alloc, list->data[index]);
    }

    // Shift elements to the left
    for (size_t i = index; i < list->size - 1; i++)
    {
        list->data[i] = list->data[i + 1];
    }

    list->size--;
    return 0;
}

ANV_API int anv_arraylist_remove(ANVArrayList* list, const void* data, const anv_compare_func compare, const bool should_free_data)
{
    const size_t index = anv_arraylist_find(list, data, compare);
    if (index == SIZE_MAX)
    {
        return -1;
    }

    return anv_arraylist_remove_at(list, index, should_free_data);
}

//==============================================================================
// Memory management functions
//==============================================================================

ANV_API int anv_arraylist_reserve(ANVArrayList* list, const size_t new_capacity)
{
    if (!list)
    {
        return -1;
    }

    return ensure_capacity(list, new_capacity);
}

ANV_API int anv_arraylist_shrink_to_fit(ANVArrayList* list)
{
    if (!list || !list->alloc.allocate)
    {
        return -1;
    }

    if (list->capacity == list->size)
    {
        return 0; // Already at optimal size
    }

    if (list->size == 0)
    {
        // Free the data array if empty
        if (list->data)
        {
            anv_alloc_deallocate(&list->alloc, list->data);
        }
        list->data = NULL;
        list->capacity = 0;
        return 0;
    }

    void** new_data = anv_alloc_allocate(&list->alloc, list->size * sizeof(void*));
    if (!new_data)
    {
        return -1;
    }

    memcpy(new_data, list->data, list->size * sizeof(void*));

    if (list->data)
    {
        anv_alloc_deallocate(&list->alloc, list->data);
    }

    list->data = new_data;
    list->capacity = list->size;
    return 0;
}

ANV_API int anv_arraylist_resize(ANVArrayList* list, const size_t new_size, void* default_value, const bool should_free_data)
{
    if (!list)
    {
        return -1;
    }

    if (new_size < list->size)
    {
        // Shrinking the list
        if (should_free_data)
        {
            for (size_t i = new_size; i < list->size; i++)
            {
                if (list->data[i])
                {
                    anv_alloc_data_deallocate(&list->alloc, list->data[i]);
                }
            }
        }
        list->size = new_size;
        return 0;
    }

    if (new_size > list->size)
    {
        // Growing the list
        if (ensure_capacity(list, new_size) != 0)
        {
            return -1;
        }

        for (size_t i = list->size; i < new_size; i++)
        {
            list->data[i] = default_value;
        }
        list->size = new_size;
    }

    return 0;
}

//==============================================================================
// ArrayList manipulation functions
//==============================================================================

ANV_API int anv_arraylist_sort(ANVArrayList* list, const anv_compare_func compare)
{
    if (!list || !compare || list->size <= 1)
    {
        return list ? 0 : -1;
    }

    // Allocate temporary array for merge sort
    void** temp = anv_alloc_allocate(&list->alloc, list->size * sizeof(void*));
    if (!temp)
    {
        return -1;
    }

    merge_sort_recursive(list->data, temp, 0, list->size - 1, compare);

    anv_alloc_deallocate(&list->alloc, temp);

    return 0;
}

ANV_API int anv_arraylist_reverse(ANVArrayList* list)
{
    if (!list || list->size <= 1)
    {
        return list ? 0 : -1;
    }

    size_t left = 0;
    size_t right = list->size - 1;

    while (left < right)
    {
        void* temp = list->data[left];
        list->data[left] = list->data[right];
        list->data[right] = temp;
        left++;
        right--;
    }

    return 0;
}

//==============================================================================
// Higher-order functions
//==============================================================================

ANV_API ANVArrayList* anv_arraylist_filter(ANVArrayList* list, const anv_filter_func filter)
{
    if (!list || !filter)
    {
        return NULL;
    }

    ANVArrayList* filtered = anv_arraylist_create(&list->alloc, 0);
    if (!filtered)
    {
        return NULL;
    }

    for (size_t i = 0; i < list->size; i++)
    {
        if (filter(list->data[i]))
        {
            if (anv_arraylist_push_back(filtered, list->data[i]) != 0)
            {
                anv_arraylist_destroy(filtered, false);
                return NULL;
            }
        }
    }

    return filtered;
}

ANV_API ANVArrayList* anv_arraylist_filter_deep(ANVArrayList* list, const anv_filter_func filter)
{
    if (!list || !filter || !list->alloc.copy)
    {
        return NULL;
    }

    ANVArrayList* filtered = anv_arraylist_create(&list->alloc, 0);
    if (!filtered)
    {
        return NULL;
    }

    for (size_t i = 0; i < list->size; i++)
    {
        if (filter(list->data[i]))
        {
            void* filtered_data = anv_alloc_copy(&filtered->alloc, list->data[i]);
            if (anv_arraylist_push_back(filtered, filtered_data) != 0)
            {
                if (filtered_data)
                {
                    anv_alloc_data_deallocate(&filtered->alloc, filtered_data);
                }
                anv_arraylist_destroy(filtered, true);
                return NULL;
            }
        }
    }

    return filtered;
}

ANV_API ANVArrayList* anv_arraylist_transform(ANVArrayList* list, const anv_transform_func transform, const bool should_free_data)
{
    if (!list || !transform)
    {
        return NULL;
    }

    ANVArrayList* transformed = anv_arraylist_create(&list->alloc, list->size);
    if (!transformed)
    {
        return NULL;
    }

    for (size_t i = 0; i < list->size; i++)
    {
        void* new_data = transform(list->data[i]);
        if (anv_arraylist_push_back(transformed, new_data) != 0)
        {
            if (new_data && should_free_data)
            {
                anv_alloc_data_deallocate(&transformed->alloc, new_data);
            }
            anv_arraylist_destroy(transformed, should_free_data);
            return NULL;
        }
    }

    return transformed;
}

ANV_API void anv_arraylist_for_each(const ANVArrayList* list, const anv_action_func action)
{
    if (!list || !action)
    {
        return;
    }

    for (size_t i = 0; i < list->size; i++)
    {
        action(list->data[i]);
    }
}

//==============================================================================
// ArrayList copying functions
//==============================================================================

ANV_API ANVArrayList* anv_arraylist_copy(ANVArrayList* list)
{
    if (!list)
    {
        return NULL;
    }

    ANVArrayList* copy = anv_arraylist_create(&list->alloc, list->capacity);
    if (!copy)
    {
        return NULL;
    }

    for (size_t i = 0; i < list->size; i++)
    {
        if (anv_arraylist_push_back(copy, list->data[i]) != 0)
        {
            anv_arraylist_destroy(copy, false);
            return NULL;
        }
    }

    return copy;
}

ANV_API ANVArrayList* anv_arraylist_copy_deep(ANVArrayList* list, const bool should_free_data)
{
    if (!list || !list->alloc.copy)
    {
        return NULL;
    }

    ANVArrayList* copy = anv_arraylist_create(&list->alloc, list->capacity);
    if (!copy)
    {
        return NULL;
    }

    for (size_t i = 0; i < list->size; i++)
    {
        void* copied_data = anv_alloc_copy(&list->alloc, list->data[i]);
        if (anv_arraylist_push_back(copy, copied_data) != 0)
        {
            anv_alloc_data_deallocate(&copy->alloc, copied_data);
            anv_arraylist_destroy(copy, should_free_data);
            return NULL;
        }
    }

    return copy;
}

//==============================================================================
// Iterator functions
//==============================================================================

// Forward iterator state
typedef struct ArrayListIterState
{
    const ANVArrayList* list;
    size_t current_index;
    bool reverse;
} ArrayListIterState;

static void* arraylist_iter_get(const ANVIterator* iter)
{
    if (!iter || !iter->data_state)
    {
        return NULL;
    }

    const ArrayListIterState* state = iter->data_state;

    if (!state->reverse)
    {
        if (state->current_index >= state->list->size)
        {
            return NULL;
        }
        return state->list->data[state->current_index];
    }

    if (state->current_index == SIZE_MAX || state->current_index >= state->list->size)
    {
        return NULL;
    }
    return state->list->data[state->current_index];
}

static int arraylist_iter_next(const ANVIterator* iter)
{
    if (!iter || !iter->data_state)
    {
        return -1;
    }

    ArrayListIterState* state = iter->data_state;

    if (!state->reverse)
    {
        if (state->current_index >= state->list->size)
        {
            return -1;
        }
        state->current_index++;
        return 0;
    }

    if (state->current_index == SIZE_MAX)
    {
        return -1;
    }

    state->current_index = (state->current_index == 0) ? SIZE_MAX : state->current_index - 1;
    return 0;
}

static int arraylist_iter_has_next(const ANVIterator* iter)
{
    if (!iter || !iter->data_state)
    {
        return 0;
    }

    const ArrayListIterState* state = iter->data_state;

    if (!state->reverse)
    {
        return state->current_index < state->list->size;
    }

    return state->current_index != SIZE_MAX && state->current_index < state->list->size;
}

static int arraylist_iter_prev(const ANVIterator* iter)
{
    if (!iter || !iter->data_state)
    {
        return -1;
    }

    ArrayListIterState* state = iter->data_state;

    if (!state->reverse)
    {
        if (state->current_index == 0)
        {
            return -1;
        }
        state->current_index--;
        return 0;
    }

    if (state->current_index >= state->list->size - 1)
    {
        return -1;
    }
    state->current_index++;
    return 0;
}

static int arraylist_iter_has_prev(const ANVIterator* iter)
{
    if (!iter || !iter->data_state)
    {
        return 0;
    }

    const ArrayListIterState* state = iter->data_state;

    if (!state->reverse)
    {
        return state->current_index > 0;
    }

    return state->current_index != SIZE_MAX && state->current_index < state->list->size - 1;
}

static void arraylist_iter_reset(const ANVIterator* iter)
{
    if (!iter || !iter->data_state)
    {
        return;
    }

    ArrayListIterState* state = iter->data_state;

    if (!state->reverse)
    {
        state->current_index = 0;
    }
    else
    {
        state->current_index = (state->list->size > 0) ? state->list->size - 1 : SIZE_MAX;
    }
}

static int arraylist_iter_is_valid(const ANVIterator* iter)
{
    if (!iter || !iter->data_state)
    {
        return 0;
    }

    const ArrayListIterState* state = iter->data_state;
    return state->list != NULL;
}

static void arraylist_iter_destroy(ANVIterator* iter)
{
    if (!iter)
    {
        return;
    }

    if (iter->data_state)
    {
        const ArrayListIterState* state = iter->data_state;
        if (state->list)
        {
            anv_alloc_deallocate(&state->list->alloc, iter->data_state);
        }
    }
    iter->data_state = NULL;
}

ANV_API ANVIterator anv_arraylist_iterator(const ANVArrayList* list)
{
    ANVIterator iter = {0};

    iter.get = arraylist_iter_get;
    iter.next = arraylist_iter_next;
    iter.has_next = arraylist_iter_has_next;
    iter.prev = arraylist_iter_prev;
    iter.has_prev = arraylist_iter_has_prev;
    iter.reset = arraylist_iter_reset;
    iter.is_valid = arraylist_iter_is_valid;
    iter.destroy = arraylist_iter_destroy;

    if (!list || !list->alloc.allocate)
    {
        return iter;
    }

    ArrayListIterState* state = anv_alloc_allocate(&list->alloc, sizeof(ArrayListIterState));
    if (!state)
    {
        return iter;
    }

    state->list = list;
    state->current_index = 0;
    state->reverse = false;

    iter.alloc = list->alloc;
    iter.data_state = state;

    return iter;
}

ANV_API ANVIterator anv_arraylist_iterator_reverse(const ANVArrayList* list)
{
    ANVIterator it = {0};

    it.get = arraylist_iter_get;
    it.next = arraylist_iter_next;
    it.has_next = arraylist_iter_has_next;
    it.prev = arraylist_iter_prev;
    it.has_prev = arraylist_iter_has_prev;
    it.reset = arraylist_iter_reset;
    it.is_valid = arraylist_iter_is_valid;
    it.destroy = arraylist_iter_destroy;

    if (!list || !list->alloc.allocate)
    {
        return it;
    }

    ArrayListIterState* state = anv_alloc_allocate(&list->alloc, sizeof(ArrayListIterState));
    if (!state)
    {
        return it;
    }

    state->list = list;
    state->current_index = (list->size > 0) ? list->size - 1 : SIZE_MAX;
    state->reverse = true;

    it.alloc = list->alloc;
    it.data_state = state;

    return it;
}

ANV_API ANVArrayList* anv_arraylist_from_iterator(ANVIterator* it, ANVAllocator* alloc, const bool should_copy)
{
    if (!it || !alloc)
    {
        return NULL;
    }
    if (should_copy && !alloc->copy)
    {
        return NULL; // Can't copy without copy function
    }

    if (!it->is_valid || !it->is_valid(it))
    {
        return NULL;
    }

    ANVArrayList* list = anv_arraylist_create(alloc, 0);
    if (!list)
    {
        return NULL;
    }

    while (it->has_next(it))
    {
        void* element = it->get(it);

        // Skip NULL elements - they indicate iterator issues
        if (!element)
        {
            if (it->next(it) != 0)
            {
                break; // Iterator exhausted or failed
            }
            continue;
        }

        void* element_to_insert = element;
        if (should_copy)
        {
            element_to_insert = alloc->copy(element);
            if (!element_to_insert)
            {
                anv_arraylist_destroy(list, true);
                return NULL;
            }
        }

        if (anv_arraylist_push_back(list, element_to_insert) != 0)
        {
            if (should_copy)
            {
                anv_alloc_data_deallocate(alloc, element_to_insert);
            }
            anv_arraylist_destroy(list, should_copy);
            return NULL;
        }

        if (it->next(it) != 0)
        {
            break; // Iterator done or failed
        }
    }

    return list;
}