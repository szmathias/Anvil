//
// HashSet.c
// Implementation of hash set functions.
//
// This file implements a hash set as a wrapper around HashMap.
// Uses a sentinel value for all HashMap values to implement set semantics.
// Provides average O(1) add, remove, and contains operations.

#include <string.h>

#include "hashset.h"
#include "pair.h"

//==============================================================================
// Helper functions
//==============================================================================

// Global variable to store the action function for for_each
static void (*hashset_action)(void* key) = NULL;

// Wrapper function that ignores the value parameter for for_each operation
static void hashset_action_wrapper(void* key, void* value)
{
    (void)value; // Ignore the sentinel value
    if (hashset_action)
    {
        hashset_action(key);
    }
}

//==============================================================================
// Constants
//==============================================================================

// Sentinel value used for all HashMap values in the set
static void* const HASHSET_PRESENT = (void*)1;

//==============================================================================
// Creation and destruction functions
//==============================================================================

ANV_API ANVHashSet* anv_hashset_create(ANVAllocator* alloc, const anv_hash_func hash,
                                       const key_equals_func key_equals, const size_t initial_capacity)
{
    if (!alloc || !hash || !key_equals)
    {
        return NULL;
    }

    ANVHashSet* set = anv_alloc_allocate(alloc, sizeof(ANVHashSet));
    if (!set)
    {
        return NULL;
    }

    set->map = anv_hashmap_create(alloc, hash, key_equals, initial_capacity);
    if (!set->map)
    {
        anv_alloc_deallocate(alloc, set);
        return NULL;
    }

    return set;
}

ANV_API void anv_hashset_destroy(ANVHashSet* set, const bool should_free_keys)
{
    if (!set)
    {
        return;
    }

    if (set->map)
    {
        const ANVAllocator alloc = set->map->alloc;

        anv_hashmap_destroy(set->map, should_free_keys, false);

        anv_alloc_deallocate(&alloc, set);
    }
}

ANV_API void anv_hashset_clear(ANVHashSet* set, const bool should_free_keys)
{
    if (!set || !set->map)
    {
        return;
    }

    anv_hashmap_clear(set->map, should_free_keys, false);
}

//==============================================================================
// Information functions
//==============================================================================

ANV_API size_t anv_hashset_size(const ANVHashSet* set)
{
    return set && set->map ? anv_hashmap_size(set->map) : 0;
}

ANV_API int anv_hashset_is_empty(const ANVHashSet* set)
{
    return !set || !set->map || anv_hashmap_is_empty(set->map);
}

ANV_API double anv_hashset_load_factor(const ANVHashSet* set)
{
    return set && set->map ? anv_hashmap_load_factor(set->map) : 0.0;
}

//==============================================================================
// Hash set operations
//==============================================================================

ANV_API int anv_hashset_add(ANVHashSet* set, void* key)
{
    if (!set || !set->map || !key)
    {
        return -1;
    }

    return anv_hashmap_put(set->map, key, HASHSET_PRESENT);
}

ANV_API int anv_hashset_add_check(ANVHashSet* set, void* key, bool* was_added_out)
{
    if (!set || !set->map || !key || !was_added_out)
    {
        return -1;
    }

    *was_added_out = false;

    void* old_value = NULL;
    const int result = anv_hashmap_put_replace(set->map, key, HASHSET_PRESENT, &old_value);

    if (result == 0)
    {
        *was_added_out = (old_value == NULL);
    }

    return result;
}

ANV_API int anv_hashset_contains(const ANVHashSet* set, const void* key)
{
    if (!set || !set->map || !key)
    {
        return 0;
    }

    return anv_hashmap_contains_key(set->map, key);
}

ANV_API int anv_hashset_remove(ANVHashSet* set, const void* key, const bool should_free_key)
{
    if (!set || !set->map || !key)
    {
        return -1;
    }

    return anv_hashmap_remove(set->map, key, should_free_key, false);
}

ANV_API void* anv_hashset_remove_get(ANVHashSet* set, const void* key)
{
    if (!set || !set->map || !key)
    {
        return NULL;
    }

    void** keys;
    size_t count;
    if (anv_hashmap_get_keys(set->map, &keys, &count) != 0)
    {
        return NULL;
    }

    void* found_key = NULL;
    for (size_t i = 0; i < count; i++)
    {
        if (set->map->key_equals(keys[i], key))
        {
            found_key = keys[i];
            break;
        }
    }

    anv_alloc_deallocate(&set->map->alloc, keys);

    if (!found_key)
    {
        return NULL;
    }

    if (anv_hashmap_remove(set->map, key, false, false) == 0)
    {
        return found_key;
    }

    return NULL;
}

//==============================================================================
// Set operations
//==============================================================================

ANV_API ANVHashSet* anv_hashset_union(const ANVHashSet* set1, const ANVHashSet* set2)
{
    if (!set1 || !set1->map || !set2 || !set2->map)
    {
        return NULL;
    }

    ANVHashSet* result = anv_hashset_create(&set1->map->alloc, set1->map->hash,
                                            set1->map->key_equals, 0);
    if (!result)
    {
        return NULL;
    }

    ANVIterator it1 = anv_hashmap_iterator(set1->map);
    while (it1.has_next(&it1))
    {
        const ANVPair* pair = it1.get(&it1);

        if (pair && anv_hashset_add(result, pair->first) != 0)
        {
            it1.destroy(&it1);
            anv_hashset_destroy(result, false);
            return NULL;
        }
        it1.next(&it1);
    }
    it1.destroy(&it1);

    ANVIterator it2 = anv_hashmap_iterator(set2->map);
    while (it2.has_next(&it2))
    {
        const ANVPair* pair = it2.get(&it2);
        if (pair)
        {
            anv_hashset_add(result, pair->first);
        }
        it2.next(&it2);
    }
    it2.destroy(&it2);

    return result;
}

ANV_API ANVHashSet* anv_hashset_intersection(const ANVHashSet* set1, const ANVHashSet* set2)
{
    if (!set1 || !set1->map || !set2 || !set2->map)
    {
        return NULL;
    }

    ANVHashSet* result = anv_hashset_create(&set1->map->alloc, set1->map->hash,
                                            set1->map->key_equals, 0);
    if (!result)
    {
        return NULL;
    }

    const ANVHashSet* smaller = anv_hashset_size(set1) <= anv_hashset_size(set2) ? set1 : set2;
    const ANVHashSet* larger = (smaller == set1) ? set2 : set1;

    ANVIterator it = anv_hashmap_iterator(smaller->map);
    while (it.has_next(&it))
    {
        const ANVPair* pair = it.get(&it);

        if (pair && anv_hashset_contains(larger, pair->first))
        {
            if (anv_hashset_add(result, pair->first) != 0)
            {
                it.destroy(&it);
                anv_hashset_destroy(result, false);
                return NULL;
            }
        }
        it.next(&it);
    }
    it.destroy(&it);

    return result;
}

ANV_API ANVHashSet* anv_hashset_difference(const ANVHashSet* set1, const ANVHashSet* set2)
{
    if (!set1 || !set1->map)
    {
        return NULL;
    }

    ANVHashSet* result = anv_hashset_create(&set1->map->alloc, set1->map->hash,
                                            set1->map->key_equals, 0);
    if (!result)
    {
        return NULL;
    }

    ANVIterator it = anv_hashmap_iterator(set1->map);
    while (it.has_next(&it))
    {
        const ANVPair* pair = it.get(&it);
        if (pair && (!set2 || !set2->map || !anv_hashset_contains(set2, pair->first)))
        {
            if (anv_hashset_add(result, pair->first) != 0)
            {
                it.destroy(&it);
                anv_hashset_destroy(result, false);
                return NULL;
            }
        }
        it.next(&it);
    }
    it.destroy(&it);

    return result;
}

ANV_API int anv_hashset_is_subset(const ANVHashSet* subset, const ANVHashSet* superset)
{
    if (!subset || !subset->map || !superset || !superset->map)
    {
        return 0;
    }

    if (anv_hashset_is_empty(subset))
    {
        return 1;
    }

   ANVIterator it = anv_hashmap_iterator(subset->map);
    while (it.has_next(&it))
    {
        const ANVPair* pair = it.get(&it);

        if (pair && !anv_hashset_contains(superset, pair->first))
        {
            it.destroy(&it);
            return 0;
        }
        it.next(&it);
    }
    it.destroy(&it);

    return 1;
}

//==============================================================================
// Bulk operations
//==============================================================================

ANV_API int anv_hashset_get_elements(const ANVHashSet* set, void*** keys_out, size_t* count_out)
{
    if (!set || !set->map)
    {
        return -1;
    }

    return anv_hashmap_get_keys(set->map, keys_out, count_out);
}

ANV_API void anv_hashset_for_each(const ANVHashSet* set, void (*action)(void* key))
{
    if (!set || !set->map || !action)
    {
        return;
    }

    hashset_action = action;
    anv_hashmap_for_each(set->map, hashset_action_wrapper);
    hashset_action = NULL;
}

//==============================================================================
// Hash set copying functions
//==============================================================================

ANV_API ANVHashSet* anv_hashset_copy(const ANVHashSet* set)
{
    if (!set || !set->map)
    {
        return NULL;
    }

    ANVHashSet* copy = anv_hashset_create(&set->map->alloc, set->map->hash,
                                          set->map->key_equals, set->map->bucket_count);
    if (!copy)
    {
        return NULL;
    }

    ANVIterator it = anv_hashmap_iterator(set->map);
    while (it.has_next(&it))
    {
        const ANVPair* pair = it.get(&it);

        if (pair && anv_hashset_add(copy, pair->first) != 0)
        {
            it.destroy(&it);
            anv_hashset_destroy(copy, false);
            return NULL;
        }
        it.next(&it);
    }
    it.destroy(&it);

    return copy;
}

ANV_API ANVHashSet* anv_hashset_copy_deep(const ANVHashSet* set, const anv_copy_func key_copy)
{
    if (!set || !set->map)
    {
        return NULL;
    }

    ANVHashSet* copy = anv_hashset_create(&set->map->alloc, set->map->hash,
                                          set->map->key_equals, set->map->bucket_count);
    if (!copy)
    {
        return NULL;
    }

    ANVIterator it = anv_hashmap_iterator(set->map);
    while (it.has_next(&it))
    {
        const ANVPair* pair = it.get(&it);

        if (pair)
        {
            void* copied_key = key_copy ? key_copy(pair->first) : pair->first;
            if ((key_copy && !copied_key) || anv_hashset_add(copy, copied_key) != 0)
            {
                if (key_copy && copied_key)
                {
                    anv_alloc_data_deallocate(&set->map->alloc, copied_key);
                }
                it.destroy(&it);
                anv_hashset_destroy(copy, key_copy != NULL);
                return NULL;
            }
        }
        it.next(&it);
    }
    it.destroy(&it);

    return copy;
}

//==============================================================================
// Iterator implementation
//==============================================================================

// HashSet iterator state - wraps HashMap iterator but only returns keys
typedef struct HashSetIteratorState
{
    ANVIterator map_iterator; // Underlying HashMap iterator
    void* current_key;        // Current key to return
    ANVAllocator alloc;      // Allocator for freeing this state
} HashSetIteratorState;

static void* hashset_iterator_get(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return NULL;
    }

    HashSetIteratorState* state = it->data_state;

    if (!state->current_key && state->map_iterator.has_next(&state->map_iterator))
    {
        const ANVPair* pair = state->map_iterator.get(&state->map_iterator);
        if (pair)
        {
            state->current_key = pair->first;
        }
    }

    return state->current_key;
}

static int hashset_iterator_has_next(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return 0;
    }

    const HashSetIteratorState* state = it->data_state;
    return state->map_iterator.has_next(&state->map_iterator);
}

static int hashset_iterator_next(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return -1;
    }

    HashSetIteratorState* state = it->data_state;

    const int result = state->map_iterator.next(&state->map_iterator);

    state->current_key = NULL;

    return result;
}

static int hashset_iterator_has_prev(const ANVIterator* it)
{
    (void)it;
    return 0;
}

static int hashset_iterator_prev(const ANVIterator* it)
{
    (void)it;
    return -1;
}

static void hashset_iterator_reset(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return;
    }

    HashSetIteratorState* state = it->data_state;
    state->map_iterator.reset(&state->map_iterator);
    state->current_key = NULL;
}

static int hashset_iterator_is_valid(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return 0;
    }

    const HashSetIteratorState* state = it->data_state;
    return state->map_iterator.is_valid(&state->map_iterator);
}

static void hashset_iterator_destroy(ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return;
    }

    HashSetIteratorState* state = it->data_state;
    state->map_iterator.destroy(&state->map_iterator);

    anv_alloc_deallocate(&state->alloc, state);

    it->data_state = NULL;
}

ANV_API ANVIterator anv_hashset_iterator(const ANVHashSet* set)
{
    ANVIterator it = {0};

    it.get = hashset_iterator_get;
    it.has_next = hashset_iterator_has_next;
    it.next = hashset_iterator_next;
    it.has_prev = hashset_iterator_has_prev;
    it.prev = hashset_iterator_prev;
    it.reset = hashset_iterator_reset;
    it.is_valid = hashset_iterator_is_valid;
    it.destroy = hashset_iterator_destroy;

    HashSetIteratorState* state = anv_alloc_allocate(&set->map->alloc, sizeof(HashSetIteratorState));
    if (!state)
    {
        return it;
    }

    state->map_iterator = anv_hashmap_iterator(set->map);
    state->current_key = NULL;
    state->alloc = set->map->alloc;

    it.alloc = set->map->alloc;
    it.data_state = state;
    return it;
}

ANV_API ANVHashSet* anv_hashset_from_iterator(ANVIterator* it, ANVAllocator* alloc,
                                              const anv_hash_func hash, const key_equals_func key_equals, const bool should_copy)
{
    if (!it || !alloc || !hash || !key_equals)
    {
        return NULL;
    }
    if (should_copy && !alloc->copy)
    {
        return NULL;
    }

    if (!it->is_valid || !it->is_valid(it))
    {
        return NULL;
    }

    ANVHashSet* set = anv_hashset_create(alloc, hash, key_equals, 0);
    if (!set)
    {
        return NULL;
    }

    while (it->has_next(it))
    {
        void* key = it->get(it);

        if (!key)
        {
            if (it->next(it) != 0)
            {
                break;
            }
            continue;
        }

        void* key_to_insert = key;

        if (should_copy)
        {
            key_to_insert = alloc->copy(key);
            if (!key_to_insert)
            {
                anv_hashset_destroy(set, true);
                return NULL;
            }
        }

        if (anv_hashset_add(set, key_to_insert) != 0)
        {
            if (should_copy)
            {
                anv_alloc_data_deallocate(alloc, key_to_insert);
            }
            anv_hashset_destroy(set, should_copy);
            return NULL;
        }

        if (it->next(it) != 0)
        {
            break;
        }
    }

    return set;
}