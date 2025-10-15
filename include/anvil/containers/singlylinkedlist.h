//
// Created by zack on 8/23/25.
//

#ifndef ANVIL_SINGLYLINKEDLIST_H
#define ANVIL_SINGLYLINKEDLIST_H

#include "iterator.h"
#include "anvil/common.h"

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Type definitions
//==============================================================================

/**
 * Node of a singly linked list.
 */
typedef struct ANVSinglyLinkedNode
{
        void* data;                       // Pointer to user data
        struct ANVSinglyLinkedNode* next; // Pointer to next node
} ANVSinglyLinkedNode;

/**
 * Singly linked list structure.
 */
typedef struct ANVSinglyLinkedList
{
        ANVSinglyLinkedNode* head;      // Pointer to first node
        ANVSinglyLinkedNode* tail;      // Pointer to last node
        size_t size;                    // Number of nodes in list
        ANVAllocator alloc;             // Custom allocator
} ANVSinglyLinkedList;

//==============================================================================
// Creation and destruction functions
//==============================================================================

/**
 * Create a new, empty singly linked list.
 *
 * @param alloc Allocator to use.
 * @return Pointer to new list, or NULL on failure
 */
ANV_API ANVSinglyLinkedList* anv_sll_create(ANVAllocator* alloc);

/**
 * Destroy the list and free all nodes.
 *
 * @param list The list to destroy
 * @param should_free_data Whether to free element data.
 */
ANV_API void anv_sll_destroy(ANVSinglyLinkedList* list, bool should_free_data);

/**
 * Clear all nodes but keep the list structure intact.
 *
 * @param list The list to clear
 * @param should_free_data Whether to free element data.
 */
ANV_API void anv_sll_clear(ANVSinglyLinkedList* list, bool should_free_data);

//==============================================================================
// Information functions
//==============================================================================

/**
 * Get number of elements in list.
 *
 * @param list The list to query
 * @return Number of elements, or 0 if list is NULL
 */
ANV_API size_t anv_sll_size(const ANVSinglyLinkedList* list);

/**
 * Check whether the list is empty.
 *
 * @param list The list to query
 * @return 1 if empty or NULL, 0 otherwise
 */
ANV_API int anv_sll_is_empty(const ANVSinglyLinkedList* list);

/**
 * Find first node matching data using compare function.
 *
 * @param list The list to search
 * @param data The value to match
 * @param compare Comparison function (returns 0 when equal)
 * @return Pointer to matching node, or NULL if not found or on error
 */
ANV_API ANVSinglyLinkedNode* anv_sll_find(const ANVSinglyLinkedList* list, const void* data, anv_compare_func compare);

/**
 * Compare two lists for equality element-wise using compare.
 *
 * @param list1 First list
 * @param list2 Second list
 * @param compare Comparison function used for elements
 * @return 1 if equal, 0 if not equal, -1 on error
 */
ANV_API int anv_sll_equals(const ANVSinglyLinkedList* list1, const ANVSinglyLinkedList* list2, anv_compare_func compare);

//==============================================================================
// Insertion functions
//==============================================================================

/**
 * Add element to the front of the list.
 *
 * @param list The list to modify
 * @param data Pointer to data (ownership transferred to list)
 * @return 0 on success, -1 on error
 */
ANV_API int anv_sll_push_front(ANVSinglyLinkedList* list, void* data);

/**
 * Add element to the back of the list.
 *
 * @param list The list to modify
 * @param data Pointer to data (ownership transferred to list)
 * @return 0 on success, -1 on error
 */
ANV_API int anv_sll_push_back(ANVSinglyLinkedList* list, void* data);

/**
 * Insert element at a specific position.
 *
 * @param list The list to modify
 * @param pos Zero-based insertion index (0 = front, size = back)
 * @param data Pointer to data (ownership transferred to list)
 * @return 0 on success, -1 on error
 */
ANV_API int anv_sll_insert_at(ANVSinglyLinkedList* list, size_t pos, void* data);

//==============================================================================
// Removal functions
//==============================================================================

/**
 * Remove first element matching data using compare.
 *
 * @param list The list to modify
 * @param data The value to match
 * @param compare Comparison function (returns 0 when equal)
 * @param should_free_data Whether to free element data.
 * @return 0 on success, -1 if not found or on error
 */
ANV_API int anv_sll_remove(ANVSinglyLinkedList* list, const void* data, anv_compare_func compare, bool should_free_data);

/**
 * Remove element at position pos.
 *
 * @param list The list to modify
 * @param pos Zero-based index of element to remove
 * @param should_free_data Whether to free element data.
 * @return 0 on success, -1 on error
 */
ANV_API int anv_sll_remove_at(ANVSinglyLinkedList* list, size_t pos, bool should_free_data);

/**
 * Remove the first element.
 *
 * @param list The list to modify
 * @param should_free_data Whether to free element data.
 * @return 0 on success, -1 on error
 */
ANV_API int anv_sll_pop_front(ANVSinglyLinkedList* list, bool should_free_data);

/**
 * Remove the last element.
 *
 * @param list The list to modify
 * @param should_free_data Whether to free element data.
 * @return 0 on success, -1 on error
 */
ANV_API int anv_sll_pop_back(ANVSinglyLinkedList* list, bool should_free_data);

//==============================================================================
// List manipulation functions
//==============================================================================

/**
 * Sort the list using a merge-based algorithm.
 *
 * @param list The list to sort
 * @param compare Comparison function
 * @return 0 on success, -1 on error
 */
ANV_API int anv_sll_sort(ANVSinglyLinkedList* list, anv_compare_func compare);

/**
 * Reverse the order of nodes in the list.
 *
 * @param list The list to reverse
 * @return 0 on success, -1 on error
 */
ANV_API int anv_sll_reverse(ANVSinglyLinkedList* list);

/**
 * Merge src list into dest (append src onto dest). After the call src is
 * emptied but not destroyed.
 *
 * @param dest Destination list
 * @param src Source list (will be emptied)
 * @return 0 on success, -1 on error
 */
ANV_API int anv_sll_merge(ANVSinglyLinkedList* dest, ANVSinglyLinkedList* src);

/**
 * Splice src into dest at position pos. After the call src is emptied but
 * not destroyed.
 *
 * @param dest Destination list
 * @param src Source list (will be emptied)
 * @param pos Position in dest where to insert src
 * @return 0 on success, -1 on error
 */
ANV_API int anv_sll_splice(ANVSinglyLinkedList* dest, ANVSinglyLinkedList* src, size_t pos);

//==============================================================================
// Higher-order functions
//==============================================================================

/**
 * Create a new list containing only elements that satisfy the predicate.
 * Performs a shallow copy: elements in the new list reuse the original data pointers.
 *
 * @param list Source list
 * @param pred Predicate function
 * @return New list with matching elements, or NULL on error
 */
ANV_API ANVSinglyLinkedList* anv_sll_filter(ANVSinglyLinkedList* list, anv_predicate_func pred);

/**
 * Create a new list containing only elements that satisfy the predicate.
 * Performs a deep copy: if the allocator provides a copy function, elements are copied into the new list.
 *
 * @param list Source list
 * @param pred Predicate function
 * @return New list with matching elements, or NULL on error
 */
ANV_API ANVSinglyLinkedList* anv_sll_filter_deep(ANVSinglyLinkedList* list, anv_predicate_func pred);

/**
 * Create a new list by transforming each element using transform. The
 * transform function is responsible for allocating any new element data.
 *
 * @param list Source list
 * @param transform Transformation function
 * @param should_free_data Whether to free element data.
 * @return New transformed list, or NULL on error
 */
ANV_API ANVSinglyLinkedList* anv_sll_transform(ANVSinglyLinkedList* list, anv_transform_func transform, bool should_free_data);

/**
 * Apply an action to each element in the list.
 *
 * @param list The list to traverse
 * @param action Function applied to each element
 */
ANV_API void anv_sll_for_each(const ANVSinglyLinkedList* list, anv_action_func action);

//==============================================================================
// List copying functions
//==============================================================================

/**
 * Create a shallow copy of the list.
 *
 * @param list The list to copy
 * @return New list sharing the same data pointers, or NULL on error
 */
ANV_API ANVSinglyLinkedList* anv_sll_copy(ANVSinglyLinkedList* list);

/**
 * Create a deep copy of the list using copy_data to clone elements.
 *
 * @param list The list to copy
 * @param copy_data Function to copy element data
 * @param should_free_data Whether to free element data.
 * @return New list with deep-copied elements, or NULL on error
 */
ANV_API ANVSinglyLinkedList* anv_sll_copy_deep(ANVSinglyLinkedList* list, anv_copy_func copy_data, bool should_free_data);

//==============================================================================
// Iterator functions
//==============================================================================

/**
 * Create an iterator for the list.
 *
 * @param list The list to iterate
 * @return An iterator object configured for forward traversal
 */
ANV_API ANVIterator anv_sll_iterator(const ANVSinglyLinkedList* list);

/**
 * Create a new SinglyLinkedList from an iterator with custom allocator.
 *
 * This function consumes all elements from the provided iterator and creates
 * a new SinglyLinkedList containing those elements. Elements are added to the
 * SinglyLinkedList in the order they are encountered from the iterator.
 *
 * @param it The source iterator (must be valid and support has_next/get/next)
 * @param alloc The custom allocator to use for the new SinglyLinkedList
 * @param should_copy If true, creates deep copies of all elements.
 *                    If false, uses elements directly from iterator.
 *                    When true, the allocators copy function must be valid.
 * @return A new SinglyLinkedList with elements from iterator, or NULL on error
 *
 * @note NULL elements from the iterator are always filtered out as they indicate
 *       iterator issues rather than valid data.
 * @note The iterator is consumed during this operation - it will be at the end
 *       position after the function completes.
 * @note If should_copy is true and copying fails for any element, the function
 *       cleans up and returns NULL.
 */
ANV_API ANVSinglyLinkedList* anv_sll_from_iterator(ANVIterator* it, ANVAllocator* alloc, bool should_copy);

#ifdef __cplusplus
}
#endif

#endif // ANVIL_SINGLYLINKEDLIST_H
