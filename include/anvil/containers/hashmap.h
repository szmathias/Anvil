//
// Created by zack on 9/9/25.
//

#ifndef ANVIL_HASHMAP_H
#define ANVIL_HASHMAP_H

#include "iterator.h"
#include "pair.h"
#include "anvil/common.h"
#include "anvil/algorithms/hash.h"

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Type definitions
//==============================================================================

/**
 * Key equality function type - compares two keys for equality.
 *
 * @param key1 Pointer to first key
 * @param key2 Pointer to second key
 * @return 1 if keys are equal, 0 otherwise
 */
typedef int (*key_equals_func)(const void* key1, const void* key2);

/**
 * Node in a hash map bucket (chaining for collision resolution).
 */
typedef struct ANVHashMapNode
{
        void* key;                   // Pointer to key data
        void* value;                 // Pointer to value data
        struct ANVHashMapNode* next; // Next node in chain
} ANVHashMapNode;

/**
 * Hash map structure.
 * Uses separate chaining for collision resolution.
 */
typedef struct ANVHashMap
{
        ANVHashMapNode** buckets;   // Array of bucket heads
        size_t bucket_count;        // Number of buckets
        size_t size;                // Number of key-value pairs
        double max_load_factor;     // Maximum load factor before resize
        anv_hash_func hash;         // Hash function for keys
        key_equals_func key_equals; // Key equality function
        ANVAllocator alloc;         // Custom allocator
} ANVHashMap;

//==============================================================================
// Creation and destruction functions
//==============================================================================

/**
 * Create a new hash map with custom allocator and functions.
 *
 * @param alloc Custom allocator (required)
 * @param hash Hash function for keys (required)
 * @param key_equals Key equality function (required)
 * @param initial_capacity Initial number of buckets (0 for default)
 * @return Pointer to new hash map, or NULL on failure
 */
ANV_API ANVHashMap* anv_hashmap_create(ANVAllocator* alloc, anv_hash_func hash,
                                       key_equals_func key_equals, size_t initial_capacity);

/**
 * Destroy the hash map and free all nodes.
 *
 * @param map The hash map to destroy
 * @param should_free_keys Whether to free key data
 * @param should_free_values Whether to free value data
 */
ANV_API void anv_hashmap_destroy(ANVHashMap* map, bool should_free_keys, bool should_free_values);

/**
 * Clear all elements from the hash map, but keep the structure intact.
 *
 * @param map The hash map to clear
 * @param should_free_keys Whether to free key data
 * @param should_free_values Whether to free value data
 */
ANV_API void anv_hashmap_clear(ANVHashMap* map, bool should_free_keys, bool should_free_values);

//==============================================================================
// Information functions
//==============================================================================

/**
 * Get the number of key-value pairs in the hash map.
 *
 * @param map The hash map to query
 * @return Number of pairs, or 0 if map is NULL
 */
ANV_API size_t anv_hashmap_size(const ANVHashMap* map);

/**
 * Check if the hash map is empty.
 *
 * @param map The hash map to check
 * @return 1 if empty or NULL, 0 if it contains elements
 */
ANV_API int anv_hashmap_is_empty(const ANVHashMap* map);

/**
 * Get the current load factor of the hash map.
 *
 * @param map The hash map to query
 * @return Load factor (size / bucket_count), or 0.0 if map is NULL
 */
ANV_API double anv_hashmap_load_factor(const ANVHashMap* map);

/**
 * Check if the hash map contains a key.
 *
 * @param map The hash map to search
 * @param key The key to search for
 * @return 1 if key exists, 0 if not found or on error
 */
ANV_API int anv_hashmap_contains_key(const ANVHashMap* map, const void* key);

//==============================================================================
// Hash map operations
//==============================================================================

/**
 * Insert or update a key-value pair in the hash map.
 *
 * @param map The hash map to modify
 * @param key Pointer to key data (ownership transferred to map)
 * @param value Pointer to value data (ownership transferred to map)
 * @return 0 on success, -1 on error
 */
ANV_API int anv_hashmap_put(ANVHashMap* map, void* key, void* value);

/**
 * Insert or update a key-value pair, returning the old value if key exists.
 * This allows the user to properly manage memory for replaced values.
 *
 * @param map The hash map to modify
 * @param key Pointer to key data (ownership transferred to map)
 * @param value Pointer to value data (ownership transferred to map)
 * @param old_value_out Pointer to store the old value (NULL if key didn't exist)
 * @return 0 on success, -1 on error
 */
ANV_API int anv_hashmap_put_replace(ANVHashMap* map, void* key, void* value, void** old_value_out);

/**
 * Insert or update a key-value pair, automatically freeing the old value if key exists.
 * Uses the map's allocator to free the old value.
 *
 * @param map The hash map to modify
 * @param key Pointer to key data (ownership transferred to map)
 * @param value Pointer to value data (ownership transferred to map)
 * @param should_free_old_value Whether to free the old value using the allocator
 * @return 0 on success, -1 on error
 */
ANV_API int anv_hashmap_put_with_free(ANVHashMap* map, void* key, void* value, bool should_free_old_value);

/**
 * Get the value associated with a key.
 *
 * @param map The hash map to search
 * @param key The key to look up
 * @return Pointer to associated value, or NULL if not found or on error
 */
ANV_API void* anv_hashmap_get(const ANVHashMap* map, const void* key);

/**
 * Remove a key-value pair from the hash map.
 *
 * @param map The hash map to modify
 * @param key The key to remove
 * @param should_free_key Whether to free the key data
 * @param should_free_value Whether to free the value data
 * @return 0 on success, -1 if key not found or on error
 */
ANV_API int anv_hashmap_remove(ANVHashMap* map, const void* key,
                               bool should_free_key, bool should_free_value);

/**
 * Remove a key-value pair and return the value.
 *
 * @param map The hash map to modify
 * @param key The key to remove
 * @param should_free_key Whether to free the key data
 * @return Pointer to the removed value, or NULL if not found or on error
 */
ANV_API void* anv_hashmap_remove_get(ANVHashMap* map, const void* key, bool should_free_key);

//==============================================================================
// Bulk operations
//==============================================================================

/**
 * Get all keys in the hash map.
 *
 * @param map The hash map to query
 * @param keys_out Pointer to array that will be filled with keys
 * @param count_out Pointer to size_t that will receive the number of keys
 * @return 0 on success, -1 on error
 */
ANV_API int anv_hashmap_get_keys(const ANVHashMap* map, void*** keys_out, size_t* count_out);

/**
 * Get all values in the hash map.
 *
 * @param map The hash map to query
 * @param values_out Pointer to array that will be with values
 * @param count_out Pointer to size_t that will receive the number of values
 * @return 0 on success, -1 on error
 */
ANV_API int anv_hashmap_get_values(const ANVHashMap* map, void*** values_out, size_t* count_out);

/**
 * Apply an action function to each key-value pair in the hash map.
 *
 * @param map The hash map to process
 * @param action Function applied to each key-value pair
 */
ANV_API void anv_hashmap_for_each(const ANVHashMap* map, void (*action)(void* key, void* value));

//==============================================================================
// Hash map copying functions
//==============================================================================

/**
 * Create a shallow copy of the hash map.
 *
 * @param map The hash map to copy
 * @return A new hash map with same structure but sharing data, or NULL on error
 */
ANV_API ANVHashMap* anv_hashmap_copy(ANVHashMap* map);

/**
 * Create a deep copy of the hash map.
 *
 * @param map The hash map to copy
 * @param key_copy Function to copy key data (NULL for shallow copy of keys)
 * @param value_copy Function to copy value data (NULL for shallow copy of values)
 * @return A new hash map with copies of all data, or NULL on error
 */
ANV_API ANVHashMap* anv_hashmap_copy_deep(ANVHashMap* map,
                                          anv_copy_func key_copy, anv_copy_func value_copy);

//==============================================================================
// Iterator functions
//==============================================================================

/**
 * Create an iterator for the hash map.
 * Iterator yields ANVKeyValuePair structures.
 *
 * @param map The hash map to iterate over
 * @return An Iterator object for traversal
 */
ANV_API ANVIterator anv_hashmap_iterator(const ANVHashMap* map);

/**
 * Create a new hash map from an iterator of key-value pairs.
 *
 * This function consumes all elements from the provided iterator and creates
 * a new HashMap containing those elements.
 * Elements are added to the HashMap in the order they are encountered from the iterator.
 *
 * @param it The source iterator (must be valid and support has_next/get/next, yields ANVKeyValuePair*)
 * @param alloc The custom allocator to use
 * @param hash Hash function for keys
 * @param key_equals Key equality function
 * @param should_copy If true, creates deep copies of all keys and values using alloc->anv_copy_func.
 *                    If false, uses keys and values directly from iterator.
 *                    When true, the allocators copy function must be valid and must accept a ANVPair*
 *                    as input to handle copying both key and value appropriately.
 * @return A new hash map with elements from iterator, or NULL on error
 *
 * @note NULL elements from the iterator are always filtered out.
 * @note The iterator is consumed during this operation - it will be at the end
 *       position after the function completes.
 * @note If should_copy is true and copying fails for any element, the function
 *       cleans up and returns NULL.
 */
ANV_API ANVHashMap* anv_hashmap_from_iterator(ANVIterator* it, ANVAllocator* alloc,
                                              anv_hash_func hash, key_equals_func key_equals, bool should_copy);

//==============================================================================
// Utility equality functions
//==============================================================================

/**
 * Equality function for string keys.
 *
 * @param key1 Pointer to first null-terminated string
 * @param key2 Pointer to second null-terminated string
 * @return 1 if strings are equal, 0 otherwise
 */
ANV_API int anv_key_equals_string(const void* key1, const void* key2);

/**
 * Equality function for integer keys.
 *
 * @param key1 Pointer to first int
 * @param key2 Pointer to second int
 * @return 1 if integers are equal, 0 otherwise
 */
ANV_API int anv_key_equals_int(const void* key1, const void* key2);

/**
 * Equality function for pointer keys.
 *
 * @param key1 First pointer
 * @param key2 Second pointer
 * @return 1 if pointers are equal, 0 otherwise
 */
ANV_API int anv_key_equals_pointer(const void* key1, const void* key2);

#ifdef __cplusplus
}
#endif

#endif // ANVIL_HASHMAP_H
