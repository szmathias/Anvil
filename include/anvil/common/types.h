//
// Created by zack on 9/26/25.
//

#ifndef ANVIL_TYPES_H
#define ANVIL_TYPES_H

#include <limits.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ANV_DEFAULT_CAPACITY 16

/**
 * Memory allocation function compatible with malloc.
 * Used for custom allocation.
 *
 * @param size Number of bytes to allocate
 * @return Pointer to allocated memory, or NULL on failure
 */
typedef void* (*anv_allocate_func)(size_t size);

/**
 * Generic free function type
 *
 * @param data Data to free
 */
typedef void (*anv_deallocate_func)(void* data);

/**
 * Copy function for deep copying element data.
 * Should return a pointer to a newly allocated copy of the data.
 *
 * @param src Pointer to original element data
 * @return Pointer to copied data (must be freed by caller)
 */
typedef void* (*anv_copy_func)(const void* src);

/**
 * Generic predicate function type.
 *
 * @param data Data to test
 * @return true if data matches predicate, false otherwise
 */
typedef bool (*anv_predicate_func)(const void* data);

/**
 * Generic transform function type.
 *
 * @param src Source data to transform
 * @return Transformed data, or NULL on failure
 */
typedef void* (*anv_transform_func)(const void* src);

/**
 * Action function for applying an operation to each element.
 * Used in for-each style traversal.
 *
 * @param data Pointer to element data
 */
typedef void (*anv_action_func)(void* data);

/**
 * Filter predicate function tests if elements should be included.
 *
 * @param element The element to test
 * @return Non-zero to include element, 0 to exclude
 */
typedef bool (*anv_filter_func)(const void* element);

/**
 * Generic equality function type
 *
 * @param a First element to compare
 * @param b Second element to compare
 * @return true if elements are equal, false otherwise
 */
typedef bool (*anv_equals_func)(const void* a, const void* b);

/**
 * Comparison function for list elements.
 * should return:
 *   < 0 if a < b,
 *    0 if a == b,
 *   > 0 if a > b.
 *
 * Used for searching, sorting, and equality checks.
 *
 * @param a Pointer to first element
 * @param b Pointer to second element
 * @return Integer indicating comparison result
 */
typedef int (*anv_compare_func)(const void* a, const void* b);

#ifdef __cplusplus
}
#endif

#endif // ANVIL_TYPES_H
