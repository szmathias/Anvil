//
// HashSet.h
//

#ifndef ANVIL_HASHSET_H
#define ANVIL_HASHSET_H

#include "hashmap.h"
#include "iterator.h"
#include "anvil/common.h"

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Type definitions
//==============================================================================

/**
 * Hash set structure.
 * Uses HashMap internally with a sentinel value for all elements.
 */
typedef struct ANVHashSet
{
        ANVHashMap* map; // Underlying hash map
} ANVHashSet;

//==============================================================================
// Creation and destruction functions
//==============================================================================

/**
 * Create a new hash set.
 *
 * @param alloc Custom allocator (required)
 * @param hash Hash function for keys (required)
 * @param key_equals Key equality function (required)
 * @param initial_capacity Initial number of buckets (0 for default)
 * @return Pointer to new hash set, or NULL on failure
 */
ANV_API ANVHashSet* anv_hashset_create(ANVAllocator* alloc, anv_hash_func hash,
                                       key_equals_func key_equals, size_t initial_capacity);

/**
 * Destroy the hash set and free all nodes.
 *
 * @param set The hash set to destroy
 * @param should_free_keys Whether to free key data
 */
ANV_API void anv_hashset_destroy(ANVHashSet* set, bool should_free_keys);

/**
 * Clear all elements from the hash set, but keep the structure intact.
 *
 * @param set The hash set to clear
 * @param should_free_keys Whether to free key data
 */
ANV_API void anv_hashset_clear(ANVHashSet* set, bool should_free_keys);

//==============================================================================
// Information functions
//==============================================================================

/**
 * Get the number of elements in the hash set.
 *
 * @param set The hash set to query
 * @return Number of elements, or 0 if set is NULL
 */
ANV_API size_t anv_hashset_size(const ANVHashSet* set);

/**
 * Check if the hash set is empty.
 *
 * @param set The hash set to check
 * @return 1 if empty or NULL, 0 if it contains elements
 */
ANV_API int anv_hashset_is_empty(const ANVHashSet* set);

/**
 * Get the current load factor of the hash set.
 *
 * @param set The hash set to query
 * @return Load factor (size / bucket_count), or 0.0 if set is NULL
 */
ANV_API double anv_hashset_load_factor(const ANVHashSet* set);

//==============================================================================
// Hash set operations
//==============================================================================

/**
 * Add an element to the hash set.
 * If the element already exists, this is a no-op.
 *
 * @param set The hash set to modify
 * @param key Pointer to key data (ownership transferred to set)
 * @return 0 on success, -1 on error
 */
ANV_API int anv_hashset_add(ANVHashSet* set, void* key);

/**
 * Add an element to the hash set, returning whether it was newly added.
 *
 * @param set The hash set to modify
 * @param key Pointer to key data (ownership transferred to set)
 * @param was_added_out Pointer to store whether key was newly added (not NULL if new)
 * @return 0 on success, -1 on error
 */
ANV_API int anv_hashset_add_check(ANVHashSet* set, void* key, bool* was_added_out);

/**
 * Check if the hash set contains an element.
 *
 * @param set The hash set to search
 * @param key The key to search for
 * @return 1 if element exists, 0 if not found or on error
 */
ANV_API int anv_hashset_contains(const ANVHashSet* set, const void* key);

/**
 * Remove an element from the hash set.
 *
 * @param set The hash set to modify
 * @param key The key to remove
 * @param should_free_key Whether to free the key data
 * @return 0 on success, -1 if key not found or on error
 */
ANV_API int anv_hashset_remove(ANVHashSet* set, const void* key, bool should_free_key);

/**
 * Remove an element and return it.
 *
 * @param set The hash set to modify
 * @param key The key to remove
 * @return Pointer to the removed key, or NULL if not found or on error
 */
ANV_API void* anv_hashset_remove_get(ANVHashSet* set, const void* key);

//==============================================================================
// Set operations
//==============================================================================

/**
 * Create the union of two hash sets (elements in either set).
 *
 * @param set1 First hash set
 * @param set2 Second hash set
 * @return New hash set containing union, or NULL on error
 */
ANV_API ANVHashSet* anv_hashset_union(const ANVHashSet* set1, const ANVHashSet* set2);

/**
 * Create the intersection of two hash sets (elements in both sets).
 *
 * @param set1 First hash set
 * @param set2 Second hash set
 * @return New hash set containing intersection, or NULL on error
 */
ANV_API ANVHashSet* anv_hashset_intersection(const ANVHashSet* set1, const ANVHashSet* set2);

/**
 * Create the difference of two hash sets (elements in first but not second).
 *
 * @param set1 First hash set
 * @param set2 Second hash set
 * @return New hash set containing difference, or NULL on error
 */
ANV_API ANVHashSet* anv_hashset_difference(const ANVHashSet* set1, const ANVHashSet* set2);

/**
 * Check if one set is a subset of another.
 *
 * @param subset The potential subset
 * @param superset The potential superset
 * @return 1 if subset is contained in superset, 0 otherwise
 */
ANV_API int anv_hashset_is_subset(const ANVHashSet* subset, const ANVHashSet* superset);

//==============================================================================
// Bulk operations
//==============================================================================

/**
 * Get all elements in the hash set.
 *
 * @param set The hash set to query
 * @param keys_out Pointer to array that will be allocated and filled with keys
 * @param count_out Pointer to size_t that will receive the number of keys
 * @return 0 on success, -1 on error
 */
ANV_API int anv_hashset_get_elements(const ANVHashSet* set, void*** keys_out, size_t* count_out);

/**
 * Apply an action function to each element in the hash set.
 *
 * @param set The hash set to process
 * @param action Function applied to each key
 */
ANV_API void anv_hashset_for_each(const ANVHashSet* set, void (*action)(void* key));

//==============================================================================
// Hash set copying functions
//==============================================================================

/**
 * Create a shallow copy of the hash set.
 *
 * @param set The hash set to copy
 * @return A new hash set with same structure but sharing data, or NULL on error
 */
ANV_API ANVHashSet* anv_hashset_copy(const ANVHashSet* set);

/**
 * Create a deep copy of the hash set.
 *
 * @param set The hash set to copy
 * @param key_copy Function to copy key data (NULL for shallow copy of keys)
 * @return A new hash set with copies of all data, or NULL on error
 */
ANV_API ANVHashSet* anv_hashset_copy_deep(const ANVHashSet* set, anv_copy_func key_copy);

//==============================================================================
// Iterator functions
//==============================================================================

/**
 * Create an iterator for the hash set (unordered traversal).
 * Iterator yields key pointers directly.
 *
 * @param set The hash set to iterate over
 * @return An Iterator object for traversal
 */
ANV_API ANVIterator anv_hashset_iterator(const ANVHashSet* set);

/**
 * Create a new hash set from an iterator of keys.
 *
 * This function consumes all elements from the provided iterator and creates
 * a new HashSet containing those elements. Elements are added to the HashSet
 * in the order they are encountered from the iterator.
 *
 * @param it The source iterator (must be valid and support has_next/get/next, yields key pointers)
 * @param alloc The custom allocator to use for the new HashSet
 * @param hash Hash function for keys
 * @param key_equals Key equality function
 * @param should_copy If true, creates deep copies of all keys using alloc->anv_copy_func.
 *                    If false, uses keys directly from iterator.
 *                    When true, the allocators copy function must be valid.
 * @return A new hash set with elements from iterator, or NULL on error
 *
 * @note NULL elements from the iterator are always filtered out as they indicate
 *       iterator issues rather than valid data.
 * @note The iterator is consumed during this operation - it will be at the end
 *       position after the function completes.
 * @note If should_copy is true and copying fails for any element, the function
 *       cleans up and returns NULL.
 */
ANV_API ANVHashSet* anv_hashset_from_iterator(ANVIterator* it, ANVAllocator* alloc,
                                              anv_hash_func hash, key_equals_func key_equals, bool should_copy);

#ifdef __cplusplus
}
#endif

#endif // ANVIL_HASHSET_H
