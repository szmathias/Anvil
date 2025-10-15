//
// Created by zack on 8/25/25.
//

#ifndef ANVIL_DOUBLYLINKEDLIST_H
#define ANVIL_DOUBLYLINKEDLIST_H

#include "iterator.h"
#include "../common.h"

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Type definitions
//==============================================================================

// Node of doubly linked list
typedef struct ANVDoublyLinkedNode
{
        void* data;                       // Pointer to user data
        struct ANVDoublyLinkedNode* next; // Pointer to next node
        struct ANVDoublyLinkedNode* prev; // Pointer to previous node
} ANVDoublyLinkedNode;

// Doubly linked list structure
typedef struct ANVDoublyLinkedList
{
        ANVDoublyLinkedNode* head; // Pointer to first node
        ANVDoublyLinkedNode* tail; // Pointer to last node
        size_t size;               // Number of nodes in list

        ANVAllocator alloc;
} ANVDoublyLinkedList;

//==============================================================================
// Creation and destruction functions
//==============================================================================

/**
 * Create a new, empty doubly linked list.
 *
 * @param alloc Custom allocator
 * @return Pointer to new DoublyLinkedList, or NULL on failure.
 */
ANV_API ANVDoublyLinkedList* anv_dll_create(ANVAllocator* alloc);

/**
 * Destroy the list and free all nodes.
 *
 * @param list The list to destroy
 * @param should_free_data Whether to free the data stored in nodes
 */
ANV_API void anv_dll_destroy(ANVDoublyLinkedList* list, bool should_free_data);

/**
 * Clear all nodes from the list, but keep the list structure intact.
 *
 * @param list The list to clear
 * @param should_free_data Whether to free the data stored in nodes
 */
ANV_API void anv_dll_clear(ANVDoublyLinkedList* list, bool should_free_data);

//==============================================================================
// Information functions
//==============================================================================

/**
 * Get the number of elements in the list.
 *
 * @param list The list to query
 * @return Number of elements, or 0 if list is NULL
 */
ANV_API size_t anv_dll_size(const ANVDoublyLinkedList* list);

/**
 * Check if the list is empty.
 *
 * @param list The list to check
 * @return 1 if list is empty or NULL, 0 if list contains elements
 */
ANV_API int anv_dll_is_empty(const ANVDoublyLinkedList* list);

/**
 * Find the first node matching data using the comparison function.
 *
 * @param list The list to search
 * @param data The data to find
 * @param compare The comparison function to use
 * @return Pointer to matching node, or NULL if not found or on error
 */
ANV_API ANVDoublyLinkedNode* anv_dll_find(ANVDoublyLinkedList* list, const void* data, anv_compare_func compare);

/**
 * Compare two lists for equality using the given comparison function.
 *
 * @param list1 First list to compare
 * @param list2 Second list to compare
 * @param compare Function to compare elements
 * @return 1 if lists are equal, 0 if not equal, -1 on error
 */
ANV_API int anv_dll_equals(const ANVDoublyLinkedList* list1, const ANVDoublyLinkedList* list2, anv_compare_func compare);

//==============================================================================
// Insertion functions
//==============================================================================

/**
 * Add data to the front of the list.
 *
 * @param list The list to modify
 * @param data Pointer to the data to insert (ownership transferred to list)
 * @return 0 on success, -1 on error
 */
ANV_API int anv_dll_push_front(ANVDoublyLinkedList* list, void* data);

/**
 * Add data to the back of the list.
 *
 * @param list The list to modify
 * @param data Pointer to the data to insert (ownership transferred to list)
 * @return 0 on success, -1 on error
 */
ANV_API int anv_dll_push_back(ANVDoublyLinkedList* list, void* data);

/**
 * Insert data at a specific position in the list.
 *
 * @param list The list to modify
 * @param pos Zero-based index where to insert (0 = front, size = back)
 * @param data Pointer to the data to insert (ownership transferred to list)
 * @return 0 on success, -1 on error
 */
ANV_API int anv_dll_insert_at(ANVDoublyLinkedList* list, size_t pos, void* data);

//==============================================================================
// Removal functions
//==============================================================================

/**
 * Remove the first node matching data using the comparison function.
 *
 * @param list The list to modify
 * @param data The data to match for removal
 * @param compare Function to compare elements
 * @param should_free_data Whether to free the data
 * @return 0 on success, -1 if not found or on error
 */
ANV_API int anv_dll_remove(ANVDoublyLinkedList* list, const void* data, anv_compare_func compare, bool should_free_data);

/**
 * Remove node at a specific position.
 *
 * @param list The list to modify
 * @param pos Zero-based index of node to remove
 * @param should_free_data Whether to free the data
 * @return 0 on success, -1 on error
 */
ANV_API int anv_dll_remove_at(ANVDoublyLinkedList* list, size_t pos, bool should_free_data);

/**
 * Remove the first node in the list.
 *
 * @param list The list to modify
 * @param should_free_data Whether to free the data
 * @return 0 on success, -1 on error
 */
ANV_API int anv_dll_pop_front(ANVDoublyLinkedList* list, bool should_free_data);

/**
 * Remove the last node in the list.
 *
 * @param list The list to modify
 * @param should_free_data Whether to free the data
 * @return 0 on success, -1 on error
 */
ANV_API int anv_dll_pop_back(ANVDoublyLinkedList* list, bool should_free_data);

//==============================================================================
// List manipulation functions
//==============================================================================

/**
 * Sort the list using merge sort algorithm.
 *
 * @param list The list to sort
 * @param compare Comparison function
 * @return 0 on success, -1 on error
 */
ANV_API int anv_dll_sort(ANVDoublyLinkedList* list, anv_compare_func compare);

/**
 * Reverse the order of nodes in the list.
 *
 * @param list The list to reverse
 * @return 0 on success, -1 on error
 */
ANV_API int anv_dll_reverse(ANVDoublyLinkedList* list);

/**
 * Merge src list into dest list.
 * After merging, src will be empty but not destroyed.
 *
 * @param dest Destination list (will contain all elements)
 * @param src Source list (will be emptied)
 * @return 0 on success, -1 on error
 */
ANV_API int anv_dll_merge(ANVDoublyLinkedList* dest, ANVDoublyLinkedList* src);

/**
 * Splice src list into dest list at the specified position.
 * After splicing, src will be empty but not destroyed.
 *
 * @param dest Destination list
 * @param src Source list (will be emptied)
 * @param pos Position in dest where to insert src
 * @return 0 on success, -1 on error
 */
ANV_API int anv_dll_splice(ANVDoublyLinkedList* dest, ANVDoublyLinkedList* src, size_t pos);

//==============================================================================
// Higher-order functions
//==============================================================================

/**
 * Create a new list containing only elements that satisfy the predicate function.
 * Performs a shallow copy: the new list shares data pointers with the source.
 *
 * @param list The source list
 * @param pred Function that returns non-zero for elements to include
 * @return A new list with matching elements, or NULL on error
 */
ANV_API ANVDoublyLinkedList* anv_dll_filter(ANVDoublyLinkedList* list, anv_predicate_func pred);

/**
 * Create a new list containing only elements that satisfy the predicate function.
 * Performs a deep copy: the new list contains cloned copies of the matching elements.
 *
 * @param list The source list
 * @param pred Function that returns non-zero for elements to include
 * @return A new list with matching elements, or NULL on error
 */
ANV_API ANVDoublyLinkedList* anv_dll_filter_deep(ANVDoublyLinkedList* list, anv_predicate_func pred);

/**
 * Create a new list by transforming each element with the given function.
 *
 * @param list The source list
 * @param transform Function that returns a new element based on the original
 * @param should_free_data Whether to free the data returned after
 * transformation if an error occurred
 * @return A new list with transformed elements, or NULL on error
 */
ANV_API ANVDoublyLinkedList* anv_dll_transform(ANVDoublyLinkedList* list, anv_transform_func transform, bool should_free_data);

/**
 * Apply an action function to each element in the list.
 *
 * @param list The list to process
 * @param action Function applied to each element
 */
ANV_API void anv_dll_for_each(const ANVDoublyLinkedList* list, anv_action_func action);

//==============================================================================
// List copying functions
//==============================================================================

/**
 * Create a shallow copy of the list.
 *
 * @param list The list to copy
 * @return A new list with same structure but sharing data, or NULL on error
 */
ANV_API ANVDoublyLinkedList* anv_dll_copy(ANVDoublyLinkedList* list);

/**
 * Create a deep copy of the list.
 *
 * @param list The list to copy
 * @param should_free_data Whether to free the data stored in nodes
 * @return A new list with copies of all data, or NULL on error
 */
ANV_API ANVDoublyLinkedList* anv_dll_copy_deep(ANVDoublyLinkedList* list, bool should_free_data);

//==============================================================================
// Iterator functions
//==============================================================================

/**
 * Create an iterator for the list.
 *
 * @param list The list to iterate over
 * @return An Iterator object for forward traversal
 */
ANV_API ANVIterator anv_dll_iterator(const ANVDoublyLinkedList* list);

/**
 * Create a reverse iterator for the list.
 *
 * @param list The list to iterate over
 * @return An Iterator object for backward traversal
 */
ANV_API ANVIterator anv_dll_iterator_reverse(const ANVDoublyLinkedList* list);

/**
 * Create a new DoublyLinkedList from an iterator.
 *
 * This function consumes all elements from the provided iterator and creates
 * a new DoublyLinkedList containing those elements. Elements are added to the DoublyLinkedList in the order
 * they are encountered from the iterator.
 *
 * @param it The source iterator (must be valid and support has_next/get/next)
 * @param alloc The custom allocator to use for the new DoublyLinkedList
 * @param should_copy If true, creates deep copies of all elements.
 *                    If false, uses elements directly from iterator.
 *                    When true, the allocators copy function must not be NULL.
 * @return A new DoublyLinkedList with elements from iterator, or NULL on error
 *
 * @note NULL elements from the iterator are always filtered out.
 * @note The iterator is consumed during this operation - it will be at the end
 *       position after the function completes.
 * @note If should_copy is true and copying fails for any element, the function
 *       cleans up and returns NULL.
 */
ANV_API ANVDoublyLinkedList* anv_dll_from_iterator(ANVIterator* it, ANVAllocator* alloc, bool should_copy);

#ifdef __cplusplus
}
#endif

#endif // ANVIL_DOUBLYLINKEDLIST_H
