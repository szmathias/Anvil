//
// Created by zack on 8/25/25.
//
// Implementation of doubly linked list functions.

#include "doublylinkedlist.h"

//==============================================================================
// Static helper functions (private to this file)
//==============================================================================

/**
 * Helper function to split the list into two halves (for merge sort).
 *
 * @param head The head of the list to split
 * @return Pointer to the head of the second half
 */
static ANVDoublyLinkedNode* anv_dll_split(ANVDoublyLinkedNode* head)
{
    if (!head || !head->next)
    {
        return NULL;
    }

    const ANVDoublyLinkedNode* fast = head;
    ANVDoublyLinkedNode* slow = head;
    ANVDoublyLinkedNode* prev = NULL;

    while (fast && fast->next)
    {
        fast = fast->next->next;
        prev = slow;
        slow = slow->next;
    }

    if (prev)
    {
        prev->next = NULL;
    }
    slow->prev = NULL;

    return slow;
}

/**
 * Helper function to merge two sorted lists (for merge sort).
 *
 * @param left The head of the first sorted list
 * @param right The head of the second sorted list
 * @param compare Comparison function
 * @return Pointer to the head of the merged sorted list
 */
static ANVDoublyLinkedNode* anv_dll_sort_helper_merge(ANVDoublyLinkedNode* left, ANVDoublyLinkedNode* right, const anv_compare_func compare)
{
    if (!left)
    {
        return right;
    }

    if (!right)
    {
        return left;
    }

    ANVDoublyLinkedNode* result;

    // Initialize result pointer
    if (compare(left->data, right->data) <= 0)
    {
        result = left;
        left = left->next;
    }
    else
    {
        result = right;
        right = right->next;
    }

    result->prev = NULL;
    ANVDoublyLinkedNode* current = result;

    // Merge the two lists
    while (left && right)
    {
        if (compare(left->data, right->data) <= 0)
        {
            current->next = left;
            left->prev = current;
            left = left->next;
        }
        else
        {
            current->next = right;
            right->prev = current;
            right = right->next;
        }
        current = current->next;
    }

    // Attach the remaining list
    if (left)
    {
        current->next = left;
        left->prev = current;
    }
    else if (right)
    {
        current->next = right;
        right->prev = current;
    }
    else
    {
        current->next = NULL;
    }

    return result;
}

/**
 * Helper function for merge sort implementation.
 *
 * @param head The head of the list to sort
 * @param compare Comparison function
 * @return Pointer to the head of the sorted list
 */
static ANVDoublyLinkedNode* anv_dll_merge_sort(ANVDoublyLinkedNode* head, const anv_compare_func compare)
{
    if (!head || !head->next)
    {
        return head;
    }

    // Split the list into two halves
    ANVDoublyLinkedNode* right = anv_dll_split(head);

    // Recursively sort both halves
    ANVDoublyLinkedNode* left_sorted = anv_dll_merge_sort(head, compare);
    ANVDoublyLinkedNode* right_sorted = anv_dll_merge_sort(right, compare);

    // Merge the sorted halves
    return anv_dll_sort_helper_merge(left_sorted, right_sorted, compare);
}

//==============================================================================
// Creation and destruction functions
//==============================================================================

ANV_API ANVDoublyLinkedList* anv_dll_create(ANVAllocator* alloc)
{
    if (!alloc)
    {
        return NULL;
    }

    ANVDoublyLinkedList* list = anv_alloc_allocate(alloc, sizeof(ANVDoublyLinkedList));
    if (!list)
    {
        return NULL;
    }

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    list->alloc = *alloc;

    return list;
}

ANV_API void anv_dll_destroy(ANVDoublyLinkedList* list, const bool should_free_data)
{
    if (list)
    {
        anv_dll_clear(list, should_free_data);
        anv_alloc_deallocate(&list->alloc, list);
    }
}

ANV_API void anv_dll_clear(ANVDoublyLinkedList* list, const bool should_free_data)
{
    if (!list)
    {
        return;
    }

    ANVDoublyLinkedNode* node = list->head;
    while (node)
    {
        ANVDoublyLinkedNode* next = node->next;
        if (should_free_data && node->data)
        {
            anv_alloc_data_deallocate(&list->alloc, node->data);
        }

        anv_alloc_deallocate(&list->alloc, node);
        node = next;
    }

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

//==============================================================================
// Information functions
//==============================================================================

ANV_API size_t anv_dll_size(const ANVDoublyLinkedList* list)
{
    if (!list)
    {
        return 0;
    }

    return list->size;
}

ANV_API int anv_dll_is_empty(const ANVDoublyLinkedList* list)
{
    return !list || list->size == 0;
}

ANV_API ANVDoublyLinkedNode* anv_dll_find(ANVDoublyLinkedList* list, const void* data, const anv_compare_func compare)
{
    if (!list || !compare)
    {
        return NULL;
    }

    ANVDoublyLinkedNode* curr = list->head;
    while (curr)
    {
        if (compare(curr->data, data) == 0)
        {
            return curr;
        }

        curr = curr->next;
    }

    return NULL;
}

ANV_API int anv_dll_equals(const ANVDoublyLinkedList* list1, const ANVDoublyLinkedList* list2, const anv_compare_func compare)
{
    if (!list1 || !list2 || !compare)
    {
        return -1;
    }

    // Lists of different sizes cannot be equal
    if (list1->size != list2->size)
    {
        return 0;
    }

    // Empty lists are equal
    if (list1->size == 0)
    {
        return 1;
    }

    // Compare each node
    const ANVDoublyLinkedNode* node1 = list1->head;
    const ANVDoublyLinkedNode* node2 = list2->head;

    while (node1 && node2)
    {
        if (compare(node1->data, node2->data) != 0)
        {
            return 0; // Data not equal
        }

        node1 = node1->next;
        node2 = node2->next;
    }

    return 1; // Lists are equal
}

//==============================================================================
// Insertion functions
//==============================================================================

ANV_API int anv_dll_push_front(ANVDoublyLinkedList* list, void* data)
{
    if (!list)
    {
        return -1;
    }

    ANVDoublyLinkedNode* node = anv_alloc_allocate(&list->alloc, sizeof(ANVDoublyLinkedNode));
    if (!node)
    {
        return -1;
    }

    node->data = data;
    node->prev = NULL;
    node->next = list->head;

    if (list->head)
    {
        list->head->prev = node;
    }
    else
    {
        // Empty list, set tail
        list->tail = node;
    }

    list->head = node;
    list->size++;

    return 0;
}

ANV_API int anv_dll_push_back(ANVDoublyLinkedList* list, void* data)
{
    if (!list)
    {
        return -1;
    }

    ANVDoublyLinkedNode* node = anv_alloc_allocate(&list->alloc, sizeof(ANVDoublyLinkedNode));
    if (!node)
    {
        return -1;
    }

    node->data = data;
    node->next = NULL;
    node->prev = list->tail;

    if (list->tail)
    {
        list->tail->next = node;
    }
    else
    {
        // Empty list, set head
        list->head = node;
    }

    list->tail = node;
    list->size++;

    return 0;
}

ANV_API int anv_dll_insert_at(ANVDoublyLinkedList* list, const size_t pos, void* data)
{
    if (!list || pos > list->size)
    {
        return -1;
    }

    if (pos == 0)
    {
        return anv_dll_push_front(list, data);
    }

    if (pos == list->size)
    {
        return anv_dll_push_back(list, data);
    }

    ANVDoublyLinkedNode* node = anv_alloc_allocate(&list->alloc, sizeof(ANVDoublyLinkedNode));
    if (!node)
    {
        return -1;
    }

    node->data = data;

    // Determine if it's more efficient to start from head or tail
    if (pos <= list->size / 2)
    {
        // Start from head and move forward
        ANVDoublyLinkedNode* curr = list->head;
        for (size_t i = 0; i < pos - 1; ++i)
        {
            curr = curr->next;
        }

        node->next = curr->next;
        node->prev = curr;
        curr->next->prev = node;
        curr->next = node;
    }
    else
    {
        // Start from tail and move backward
        ANVDoublyLinkedNode* curr = list->tail;
        for (size_t i = list->size - 1; i > pos; --i)
        {
            curr = curr->prev;
        }

        node->prev = curr->prev;
        node->next = curr;
        curr->prev->next = node;
        curr->prev = node;
    }

    list->size++;

    return 0;
}

//==============================================================================
// Removal functions
//==============================================================================

ANV_API int anv_dll_remove(ANVDoublyLinkedList* list, const void* data, const anv_compare_func compare, const bool should_free_data)
{
    if (!list || !compare || list->size == 0)
    {
        return -1;
    }

    ANVDoublyLinkedNode* curr = list->head;
    while (curr)
    {
        if (compare(curr->data, data) == 0)
        {
            // Found the node to remove
            if (curr->prev)
            {
                curr->prev->next = curr->next;
            }
            else
            {
                // Removing head node
                list->head = curr->next;
            }

            if (curr->next)
            {
                curr->next->prev = curr->prev;
            }
            else
            {
                // Removing tail node
                list->tail = curr->prev;
            }

            if (should_free_data && curr->data)
            {
                anv_alloc_data_deallocate(&list->alloc, curr->data);
            }
            anv_alloc_deallocate(&list->alloc, curr);
            list->size--;

            return 0;
        }
        curr = curr->next;
    }

    return -1; // Node not found
}

ANV_API int anv_dll_remove_at(ANVDoublyLinkedList* list, const size_t pos, const bool should_free_data)
{
    if (!list || pos >= list->size)
    {
        return -1;
    }

    ANVDoublyLinkedNode* node_to_remove = NULL;

    if (pos == 0)
    {
        return anv_dll_pop_front(list, should_free_data);
    }

    if (pos == list->size - 1)
    {
        return anv_dll_pop_back(list, should_free_data);
    }

    if (pos <= list->size / 2)
    {
        // Find node from head (more efficient for first half)
        node_to_remove = list->head;
        for (size_t i = 0; i < pos; ++i)
        {
            node_to_remove = node_to_remove->next;
        }
        node_to_remove->prev->next = node_to_remove->next;
        node_to_remove->next->prev = node_to_remove->prev;
    }
    else
    {
        // Find node from tail (more efficient for second half)
        node_to_remove = list->tail;
        for (size_t i = list->size - 1; i > pos; --i)
        {
            node_to_remove = node_to_remove->prev;
        }
        node_to_remove->prev->next = node_to_remove->next;
        node_to_remove->next->prev = node_to_remove->prev;
    }

    if (should_free_data && node_to_remove->data)
    {
        anv_alloc_data_deallocate(&list->alloc, node_to_remove->data);
    }
    anv_alloc_deallocate(&list->alloc, node_to_remove);
    list->size--;

    return 0;
}

ANV_API int anv_dll_pop_front(ANVDoublyLinkedList* list, const bool should_free_data)
{
    if (!list || !list->head)
    {
        return -1;
    }

    ANVDoublyLinkedNode* node_to_remove = list->head;
    list->head = node_to_remove->next;

    if (list->head)
    {
        list->head->prev = NULL;
    }
    else
    {
        // Removed the only node
        list->tail = NULL;
    }

    if (should_free_data && node_to_remove->data)
    {
        anv_alloc_data_deallocate(&list->alloc, node_to_remove->data);
    }

    anv_alloc_deallocate(&list->alloc, node_to_remove);
    list->size--;

    return 0;
}

ANV_API int anv_dll_pop_back(ANVDoublyLinkedList* list, const bool should_free_data)
{
    if (!list || !list->tail)
    {
        return -1;
    }

    ANVDoublyLinkedNode* node_to_remove = list->tail;
    list->tail = node_to_remove->prev;

    if (list->tail)
    {
        list->tail->next = NULL;
    }
    else
    {
        // Removed the only node
        list->head = NULL;
    }

    if (should_free_data && node_to_remove->data)
    {
        anv_alloc_data_deallocate(&list->alloc, node_to_remove->data);
    }

    // Free the node memory using the allocator's dealloc function
    anv_alloc_deallocate(&list->alloc, node_to_remove);
    list->size--;
    return 0;
}

//==============================================================================
// List manipulation functions
//==============================================================================

ANV_API int anv_dll_sort(ANVDoublyLinkedList* list, const anv_compare_func compare)
{
    if (!list || !compare || list->size <= 1)
    {
        return !list || !compare ? -1 : 0;
    }

    // Perform the merge sort
    list->head = anv_dll_merge_sort(list->head, compare);

    // Update the tail pointer
    ANVDoublyLinkedNode* current = list->head;
    while (current && current->next)
    {
        current = current->next;
    }
    list->tail = current;

    return 0;
}

ANV_API int anv_dll_reverse(ANVDoublyLinkedList* list)
{
    if (!list)
    {
        return -1;
    }

    if (list->size <= 1)
    {
        return 0; // Empty or single-element list is already reversed
    }

    ANVDoublyLinkedNode* current = list->head;
    ANVDoublyLinkedNode* temp;

    // Swap next and prev pointers for all nodes
    while (current)
    {
        // Swap next and prev pointers
        temp = current->prev;
        current->prev = current->next;
        current->next = temp;

        // Move to the next node (which is now current->prev because we swapped)
        current = current->prev;
    }

    // Swap head and tail pointers
    temp = list->head;
    list->head = list->tail;
    list->tail = temp;

    return 0;
}

ANV_API int anv_dll_merge(ANVDoublyLinkedList* dest, ANVDoublyLinkedList* src)
{
    if (!dest || !src)
    {
        return -1;
    }

    // If src is empty, nothing to merge
    if (src->size == 0)
    {
        return 0;
    }

    // If dest is empty, make it equal to src
    if (dest->size == 0)
    {
        dest->head = src->head;
        dest->tail = src->tail;
        dest->size = src->size;
    }
    else
    {
        // Connect dest's tail to src's head
        dest->tail->next = src->head;
        src->head->prev = dest->tail;

        // Update dest's tail
        dest->tail = src->tail;

        // Update size
        dest->size += src->size;
    }

    // Clear src list without destroying nodes
    src->head = NULL;
    src->tail = NULL;
    src->size = 0;

    return 0;
}

ANV_API int anv_dll_splice(ANVDoublyLinkedList* dest, ANVDoublyLinkedList* src, const size_t pos)
{
    if (!dest || !src || pos > dest->size)
    {
        return -1;
    }

    // If src is empty, nothing to splice
    if (src->size == 0)
    {
        return 0;
    }

    // If inserting at the beginning
    if (pos == 0)
    {
        if (dest->size == 0)
        {
            dest->head = src->head;
            dest->tail = src->tail;
        }
        else
        {
            src->tail->next = dest->head;
            dest->head->prev = src->tail;
            dest->head = src->head;
        }
    }
    // If inserting at the end
    else if (pos == dest->size)
    {
        dest->tail->next = src->head;
        src->head->prev = dest->tail;
        dest->tail = src->tail;
    }
    // If inserting in the middle
    else
    {
        ANVDoublyLinkedNode* curr;

        // Find the node at position pos
        if (pos <= dest->size / 2)
        {
            // Search from head
            curr = dest->head;
            for (size_t i = 0; i < pos; ++i)
            {
                curr = curr->next;
            }
        }
        else
        {
            // Search from tail
            curr = dest->tail;
            for (size_t i = dest->size - 1; i > pos; --i)
            {
                curr = curr->prev;
            }
        }

        // Connect the node at position pos-1 to src's first node
        curr->prev->next = src->head;
        src->head->prev = curr->prev;

        // Connect src's last node to the node at position pos
        src->tail->next = curr;
        curr->prev = src->tail;
    }

    // Update size
    dest->size += src->size;

    // Clear src list without destroying nodes
    src->head = NULL;
    src->tail = NULL;
    src->size = 0;

    return 0;
}

//==============================================================================
// Higher-order functions
//==============================================================================

ANV_API ANVDoublyLinkedList* anv_dll_filter(ANVDoublyLinkedList* list, const anv_predicate_func pred)
{
    if (!list || !pred)
    {
        return NULL;
    }

    ANVDoublyLinkedList* filtered = anv_dll_create(&list->alloc);
    if (!filtered)
    {
        return NULL;
    }

    const ANVDoublyLinkedNode* curr = list->head;
    while (curr)
    {
        if (pred(curr->data))
        {
            if (anv_dll_push_back(filtered, curr->data) != 0)
            {
                anv_dll_destroy(filtered, false);
                return NULL;
            }
        }
        curr = curr->next;
    }

    return filtered;
}

ANV_API ANVDoublyLinkedList* anv_dll_filter_deep(ANVDoublyLinkedList* list, const anv_predicate_func pred)
{
    if (!list || !pred || !list->alloc.copy)
    {
        return NULL;
    }

    ANVDoublyLinkedList* filtered = anv_dll_create(&list->alloc);
    if (!filtered)
    {
        return NULL;
    }

    const ANVDoublyLinkedNode* curr = list->head;
    while (curr)
    {
        if (pred(curr->data))
        {
            void* filtered_data = anv_alloc_copy(&filtered->alloc, curr->data);

            if (anv_dll_push_back(filtered, filtered_data) != 0)
            {
                if (filtered_data)
                {
                    anv_alloc_data_deallocate(&filtered->alloc, filtered_data);
                }
                anv_dll_destroy(filtered, true);
                return NULL;
            }
        }
        curr = curr->next;
    }

    return filtered;
}

ANV_API ANVDoublyLinkedList* anv_dll_transform(ANVDoublyLinkedList* list, const anv_transform_func transform, const bool should_free_data)
{
    if (!list || !transform)
    {
        return NULL;
    }

    ANVDoublyLinkedList* transformed = anv_dll_create(&list->alloc);
    if (!transformed)
    {
        return NULL;
    }

    const ANVDoublyLinkedNode* curr = list->head;
    while (curr)
    {
        void* new_data = transform(curr->data);
        if (anv_dll_push_back(transformed, new_data) != 0)
        {
            if (should_free_data && new_data)
            {
                anv_alloc_data_deallocate(&transformed->alloc, new_data); // Free the transformed data if insertion fails
            }
            // Free any data already successfully inserted into the new list
            anv_dll_destroy(transformed, should_free_data);
            return NULL;
        }
        curr = curr->next;
    }

    return transformed;
}

ANV_API void anv_dll_for_each(const ANVDoublyLinkedList* list, const anv_action_func action)
{
    if (!list || !action)
    {
        return;
    }

    const ANVDoublyLinkedNode* curr = list->head;
    while (curr)
    {
        action(curr->data);
        curr = curr->next;
    }
}

//==============================================================================
// List copying functions
//==============================================================================

ANV_API ANVDoublyLinkedList* anv_dll_copy(ANVDoublyLinkedList* list)
{
    if (!list)
    {
        return NULL;
    }

    ANVDoublyLinkedList* copy = anv_dll_create(&list->alloc);
    if (!copy)
    {
        return NULL;
    }

    // Empty list case
    if (list->size == 0)
    {
        return copy;
    }

    // Copy nodes, sharing data pointers
    const ANVDoublyLinkedNode* curr = list->head;
    while (curr)
    {
        if (anv_dll_push_back(copy, curr->data) != 0)
        {
            anv_dll_destroy(copy, false); // Don't free data - they're shared
            return NULL;
        }
        curr = curr->next;
    }

    return copy;
}

ANV_API ANVDoublyLinkedList* anv_dll_copy_deep(ANVDoublyLinkedList* list, const bool should_free_data)
{
    if (!list || !list->alloc.copy)
    {
        return NULL;
    }

    ANVDoublyLinkedList* copy = anv_dll_create(&list->alloc);
    if (!copy)
    {
        return NULL;
    }

    // Empty list case
    if (list->size == 0)
    {
        return copy;
    }

    // Copy nodes and clone data
    const ANVDoublyLinkedNode* curr = list->head;
    while (curr)
    {
        void* data_copy = anv_alloc_copy(&copy->alloc, curr->data);
        if (!data_copy)
        {
            // On failure, destroy the partially built clone, freeing any data it contains
            anv_dll_destroy(copy, should_free_data);
            return NULL;
        }
        if (anv_dll_push_back(copy, data_copy) != 0)
        {
            // If insertion fails, free the orphaned copy and destroy the partial clone
            if (should_free_data)
            {
                anv_alloc_data_deallocate(&list->alloc, data_copy);
            }
            anv_dll_destroy(copy, should_free_data);
            return NULL;
        }
        curr = curr->next;
    }

    return copy;
}

// Create a list from an iterator using a provided allocator. This function
// matches the header declaration `anv_dll_from_iterator_custom(Iterator*, Alloc*)`.
ANV_API ANVDoublyLinkedList* anv_dll_from_iterator(ANVIterator* it, ANVAllocator* alloc, const bool should_copy)
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

    ANVDoublyLinkedList* list = anv_dll_create(alloc);
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
                anv_dll_destroy(list, true);
                return NULL;
            }
        }

        if (anv_dll_push_back(list, element_to_insert) != 0)
        {
            if (should_copy)
            {
                anv_alloc_data_deallocate(alloc, element_to_insert);
            }
            anv_dll_destroy(list, should_copy);
            return NULL;
        }

        if (it->next(it) != 0)
        {
            break; // Iterator done or failed
        }
    }

    return list;
}

//==============================================================================
// Iterator functions
//==============================================================================

typedef struct ListIteratorState
{
    ANVDoublyLinkedNode* current;    // Current node
    ANVDoublyLinkedNode* start;      // Starting position (head or tail)
    const ANVDoublyLinkedList* list; // The list being iterated (const for safety)
} ListIteratorState;

/**
 * Check if iterator has more elements.
 */
static int anv_dll_iterator_has_next(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return 0;
    }

    const ListIteratorState* state = it->data_state;
    return state->current != NULL;
}

/**
 * Get current element without advancing iterator.
 */
static void* anv_dll_iterator_get(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return NULL;
    }

    const ListIteratorState* state = it->data_state;
    if (!state->current)
    {
        return NULL;
    }

    return state->current->data;
}

/**
 * Advance iterator to next position.
 */
static int anv_dll_iterator_next(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return -1;
    }

    ListIteratorState* state = it->data_state;
    if (!state->current)
    {
        return -1;
    }

    if (state->start == state->list->head)
    {
        state->current = state->current->next;
    }
    else
    {
        state->current = state->current->prev;
    }

    return 0;
}

/**
 * Check if iterator has previous elements.
 */
static int anv_dll_iterator_has_prev(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return 0;
    }

    const ListIteratorState* state = it->data_state;

    if (!state->list)
    {
        return 0;
    }

    // If current is NULL, we can only go back if we moved past the end
    if (!state->current)
    {
        return state->start != NULL; // Can return to valid position
    }

    return state->current != state->start;
}

/**
 * Move iterator backwards.
 */
static int anv_dll_iterator_prev(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return -1;
    }

    ListIteratorState* state = it->data_state;
    if (!state->current)
    {
        return -1;
    }

    if (state->start == state->list->head)
    {
        state->current = state->current->prev;
    }
    else
    {
        state->current = state->current->next;
    }

    return 0;
}

/**
 * Reset iterator to starting position.
 */
static void anv_dll_iterator_reset(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return;
    }

    ListIteratorState* state = it->data_state;
    state->current = state->start;
}

/**
 * Check if iterator is valid.
 */
static int anv_dll_iterator_is_valid(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return 0;
    }

    const ListIteratorState* state = it->data_state;
    return state->list != NULL;
}

/**
 * Free resources used by iterator.
 */
static void anv_dll_iterator_destroy(ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return;
    }


    anv_alloc_deallocate(&it->alloc, it->data_state);

    it->data_state = NULL;
}

ANV_API ANVIterator anv_dll_iterator(const ANVDoublyLinkedList* list)
{
    ANVIterator it = {0}; // Initialize all fields to NULL/0

    it.get = anv_dll_iterator_get;
    it.next = anv_dll_iterator_next;
    it.has_next = anv_dll_iterator_has_next;
    it.prev = anv_dll_iterator_prev;
    it.has_prev = anv_dll_iterator_has_prev;
    it.reset = anv_dll_iterator_reset;
    it.is_valid = anv_dll_iterator_is_valid;
    it.destroy = anv_dll_iterator_destroy;

    if (!list)
    {
        return it;
    }

    ListIteratorState* state = anv_alloc_allocate(&list->alloc, sizeof(ListIteratorState));
    if (!state)
    {
        return it;
    }

    state->current = list->head;
    state->start = list->head;
    state->list = list;

    it.alloc = list->alloc;
    it.data_state = state;

    return it;
}

ANV_API ANVIterator anv_dll_iterator_reverse(const ANVDoublyLinkedList* list)
{
    ANVIterator it = {0}; // Initialize all fields to NULL/0

    it.get = anv_dll_iterator_get;
    it.next = anv_dll_iterator_next;
    it.has_next = anv_dll_iterator_has_next;
    it.prev = anv_dll_iterator_prev;
    it.has_prev = anv_dll_iterator_has_prev;
    it.reset = anv_dll_iterator_reset;
    it.is_valid = anv_dll_iterator_is_valid;
    it.destroy = anv_dll_iterator_destroy;

    if (!list)
    {
        return it;
    }

    ListIteratorState* state = anv_alloc_allocate(&list->alloc, sizeof(ListIteratorState));
    if (!state)
    {
        return it;
    }

    state->current = list->tail;
    state->start = list->tail;
    state->list = list;

    it.alloc = list->alloc;
    it.data_state = state;

    return it;
}