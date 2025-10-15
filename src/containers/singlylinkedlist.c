//
// SinglyLinkedList.c
//

#include "singlylinkedlist.h"

//==============================================================================
// Helper functions
//==============================================================================

static ANVSinglyLinkedNode* sll_sort_helper_merge(ANVSinglyLinkedNode* left, ANVSinglyLinkedNode* right, const anv_compare_func compare)
{
    if (!left)
    {
        return right;
    }
    if (!right)
    {
        return left;
    }

    ANVSinglyLinkedNode* result;

    if (compare(left->data, right->data) <= 0)
    {
        result = left;
        result->next = sll_sort_helper_merge(left->next, right, compare);
    }
    else
    {
        result = right;
        result->next = sll_sort_helper_merge(left, right->next, compare);
    }

    return result;
}

//==============================================================================
// Creation and destruction functions
//==============================================================================

ANV_API ANVSinglyLinkedList* anv_sll_create(ANVAllocator* alloc)
{
    if (!alloc)
    {
        return NULL;
    }

    ANVSinglyLinkedList* list = anv_alloc_allocate(alloc, sizeof(ANVSinglyLinkedList));
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

ANV_API void anv_sll_destroy(ANVSinglyLinkedList* list, const bool should_free_data)
{
    if (list)
    {
        anv_sll_clear(list, should_free_data);
        anv_alloc_deallocate(&list->alloc, list);
    }
}

ANV_API void anv_sll_clear(ANVSinglyLinkedList* list, const bool should_free_data)
{
    if (!list)
    {
        return;
    }

    ANVSinglyLinkedNode* node = list->head;
    while (node)
    {
        ANVSinglyLinkedNode* next = node->next;
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

ANV_API size_t anv_sll_size(const ANVSinglyLinkedList* list)
{
    if (!list)
    {
        return 0;
    }
    return list->size;
}

ANV_API int anv_sll_is_empty(const ANVSinglyLinkedList* list)
{
    return !list || list->size == 0;
}

ANV_API ANVSinglyLinkedNode* anv_sll_find(const ANVSinglyLinkedList* list, const void* data, const anv_compare_func compare)
{
    if (!list || !compare)
    {
        return NULL;
    }

    ANVSinglyLinkedNode* curr = list->head;
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

ANV_API int anv_sll_equals(const ANVSinglyLinkedList* list1, const ANVSinglyLinkedList* list2, const anv_compare_func compare)
{
    if (!list1 || !list2 || !compare)
    {
        return -1;
    }

    if (list1->size != list2->size)
    {
        return 0;
    }

    if (list1->size == 0)
    {
        return 1;
    }

    const ANVSinglyLinkedNode* node1 = list1->head;
    const ANVSinglyLinkedNode* node2 = list2->head;

    while (node1 && node2)
    {
        if (compare(node1->data, node2->data) != 0)
        {
            return 0;
        }

        node1 = node1->next;
        node2 = node2->next;
    }

    return 1;
}

//==============================================================================
// Insertion functions
//==============================================================================

ANV_API int anv_sll_push_front(ANVSinglyLinkedList* list, void* data)
{
    if (!list)
    {
        return -1;
    }

    ANVSinglyLinkedNode* node = anv_alloc_allocate(&list->alloc, sizeof(ANVSinglyLinkedNode));
    if (!node)
    {
        return -1;
    }

    node->data = data;
    node->next = list->head;
    list->head = node;

    if (!list->tail)
    {
        list->tail = node;
    }

    list->size++;
    return 0;
}

ANV_API int anv_sll_push_back(ANVSinglyLinkedList* list, void* data)
{
    if (!list)
    {
        return -1;
    }

    ANVSinglyLinkedNode* node = anv_alloc_allocate(&list->alloc, sizeof(ANVSinglyLinkedNode));
    if (!node)
    {
        return -1;
    }

    node->data = data;
    node->next = NULL;

    if (!list->head)
    {
        list->head = node;
        list->tail = node;
    }
    else
    {
        list->tail->next = node;
        list->tail = node;
    }

    list->size++;
    return 0;
}

ANV_API int anv_sll_insert_at(ANVSinglyLinkedList* list, const size_t pos, void* data)
{
    if (!list || pos > list->size)
    {
        return -1;
    }

    if (pos == 0)
    {
        return anv_sll_push_front(list, data);
    }

    ANVSinglyLinkedNode* node = anv_alloc_allocate(&list->alloc, sizeof(ANVSinglyLinkedNode));
    if (!node)
    {
        return -1;
    }

    node->data = data;

    ANVSinglyLinkedNode* prev = list->head;
    for (size_t i = 1; i < pos; ++i)
    {
        prev = prev->next;
        if (!prev)
        {
            anv_alloc_deallocate(&list->alloc, node);
            return -1;
        }
    }

    node->next = prev->next;
    prev->next = node;
    list->size++;
    return 0;
}

//==============================================================================
// Removal functions
//==============================================================================

ANV_API int anv_sll_remove(ANVSinglyLinkedList* list, const void* data, const anv_compare_func compare, const bool should_free_data)
{
    if (!list || !compare)
    {
        return -1;
    }

    ANVSinglyLinkedNode* prev = NULL;
    ANVSinglyLinkedNode* curr = list->head;
    while (curr)
    {
        if (compare(curr->data, data) == 0)
        {
            if (prev)
            {
                prev->next = curr->next;
            }
            else
            {
                list->head = curr->next;
            }

            if (curr == list->tail)
            {
                list->tail = prev;
            }

            if (should_free_data && curr->data)
            {
                anv_alloc_data_deallocate(&list->alloc, curr->data);
            }

            anv_alloc_deallocate(&list->alloc, curr);
            list->size--;
            return 0;
        }

        prev = curr;
        curr = curr->next;
    }
    return -1;
}

ANV_API int anv_sll_remove_at(ANVSinglyLinkedList* list, const size_t pos, const bool should_free_data)
{
    if (!list || pos >= list->size)
    {
        return -1;
    }

    ANVSinglyLinkedNode* prev = NULL;
    ANVSinglyLinkedNode* curr = list->head;

    for (size_t i = 0; i < pos; ++i)
    {
        prev = curr;
        curr = curr->next;
    }

    if (prev)
    {
        prev->next = curr->next;
    }
    else
    {
        list->head = curr->next;
    }

    if (should_free_data && curr->data)
    {
        anv_alloc_data_deallocate(&list->alloc, curr->data);
    }

    anv_alloc_deallocate(&list->alloc, curr);
    list->size--;
    return 0;
}

ANV_API int anv_sll_pop_front(ANVSinglyLinkedList* list, const bool should_free_data)
{
    if (!list || !list->head)
    {
        return -1;
    }

    ANVSinglyLinkedNode* node_to_remove = list->head;
    list->head = node_to_remove->next;

    if (should_free_data && node_to_remove->data)
    {
        anv_alloc_data_deallocate(&list->alloc, node_to_remove->data);
    }

    anv_alloc_deallocate(&list->alloc, node_to_remove);
    list->size--;
    return 0;
}

ANV_API int anv_sll_pop_back(ANVSinglyLinkedList* list, const bool should_free_data)
{
    if (!list || !list->head)
    {
        return -1;
    }

    if (list->size == 1)
    {
        return anv_sll_pop_front(list, should_free_data);
    }

    ANVSinglyLinkedNode* prev = NULL;
    ANVSinglyLinkedNode* curr = list->head;
    while (curr->next)
    {
        prev = curr;
        curr = curr->next;
    }

    if (prev)
    {
        prev->next = NULL;
    }

    if (should_free_data && curr->data)
    {
        anv_alloc_data_deallocate(&list->alloc, curr->data);
    }

    anv_alloc_deallocate(&list->alloc, curr);
    list->size--;
    return 0;
}

//==============================================================================
// List manipulation functions
//==============================================================================

ANV_API int anv_sll_sort(ANVSinglyLinkedList* list, const anv_compare_func compare)
{
    if (!list || !compare)
    {
        return -1;
    }
    if (list->size <= 1)
    {
        return 0;
    }

    ANVSinglyLinkedNode* sub_lists[64] = {0};
    int num_sub_lists = 0;

    ANVSinglyLinkedNode* current = list->head;
    while (current)
    {
        ANVSinglyLinkedNode* next = current->next;
        current->next = NULL;

        int i = 0;
        while (i < num_sub_lists && sub_lists[i] != NULL)
        {
            current = sll_sort_helper_merge(sub_lists[i], current, compare);
            sub_lists[i] = NULL;
            i++;
        }

        if (i == num_sub_lists)
        {
            num_sub_lists++;
        }
        sub_lists[i] = current;

        current = next;
    }

    for (int i = 1; i < num_sub_lists; i++)
    {
        sub_lists[i] = sll_sort_helper_merge(sub_lists[i - 1], sub_lists[i], compare);
    }

    list->head = sub_lists[num_sub_lists - 1];

    return 0;
}

ANV_API int anv_sll_reverse(ANVSinglyLinkedList* list)
{
    if (!list)
    {
        return -1;
    }

    if (list->size <= 1)
    {
        return 0;
    }

    ANVSinglyLinkedNode* prev = NULL;
    ANVSinglyLinkedNode* current = list->head;

    while (current)
    {
        ANVSinglyLinkedNode* next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }

    list->head = prev;
    return 0;
}

ANV_API int anv_sll_merge(ANVSinglyLinkedList* dest, ANVSinglyLinkedList* src)
{
    if (!dest || !src)
    {
        return -1;
    }

    if (src->size == 0)
    {
        return 0;
    }

    if (dest->size == 0)
    {
        dest->head = src->head;
        dest->size = src->size;
    }
    else
    {
        ANVSinglyLinkedNode* last = dest->head;
        while (last->next)
        {
            last = last->next;
        }
        last->next = src->head;
        dest->size += src->size;
    }

    src->head = NULL;
    src->size = 0;

    return 0;
}

ANV_API int anv_sll_splice(ANVSinglyLinkedList* dest, ANVSinglyLinkedList* src, const size_t pos)
{
    if (!dest || !src || pos > dest->size)
    {
        return -1;
    }

    if (anv_sll_is_empty(src))
    {
        return 0;
    }

    if (pos == dest->size)
    {
        return anv_sll_merge(dest, src);
    }

    ANVSinglyLinkedNode* src_last = src->head;
    while (src_last->next)
    {
        src_last = src_last->next;
    }

    if (pos == 0)
    {
        src_last->next = dest->head;
        dest->head = src->head;
    }
    else
    {
        ANVSinglyLinkedNode* prev = dest->head;
        for (size_t i = 1; i < pos; ++i)
        {
            prev = prev->next;
        }
        src_last->next = prev->next;
        prev->next = src->head;
    }

    dest->size += src->size;

    src->head = NULL;
    src->size = 0;

    return 0;
}

//==============================================================================
// Higher-order functions
//==============================================================================

ANV_API ANVSinglyLinkedList* anv_sll_filter(ANVSinglyLinkedList* list, const anv_predicate_func pred)
{
    if (!list || !pred)
    {
        return NULL;
    }

    ANVSinglyLinkedList* filtered = anv_sll_create(&list->alloc);
    if (!filtered)
    {
        return NULL;
    }

    const ANVSinglyLinkedNode* curr = list->head;
    while (curr)
    {
        if (pred(curr->data))
        {
            if (anv_sll_push_back(filtered, curr->data) != 0)
            {
                anv_sll_destroy(filtered, false);
                return NULL;
            }
        }
        curr = curr->next;
    }

    return filtered;
}

ANV_API ANVSinglyLinkedList* anv_sll_filter_deep(ANVSinglyLinkedList* list, const anv_predicate_func pred)
{
    if (!list || !pred || !list->alloc.copy)
    {
        return NULL;
    }

    ANVSinglyLinkedList* filtered = anv_sll_create(&list->alloc);
    if (!filtered)
    {
        return NULL;
    }

    const ANVSinglyLinkedNode* curr = list->head;
    while (curr)
    {
        if (pred(curr->data))
        {
            void* filtered_data = anv_alloc_copy(&filtered->alloc, curr->data);
            if (anv_sll_push_back(filtered, filtered_data) != 0)
            {
                if (filtered_data)
                {
                    anv_alloc_data_deallocate(&filtered->alloc, filtered_data);
                }
                anv_sll_destroy(filtered, true);
                return NULL;
            }
        }
        curr = curr->next;
    }

    return filtered;
}

ANV_API ANVSinglyLinkedList* anv_sll_transform(ANVSinglyLinkedList* list, const anv_transform_func transform, const bool should_free_data)
{
    if (!list || !transform)
    {
        return NULL;
    }

    ANVSinglyLinkedList* transformed = anv_sll_create(&list->alloc);
    if (!transformed)
    {
        return NULL;
    }

    const ANVSinglyLinkedNode* curr = list->head;
    while (curr)
    {
        void* new_data = transform(curr->data);
        if (anv_sll_push_back(transformed, new_data) != 0)
        {
            if (should_free_data && new_data)
            {
                anv_alloc_data_deallocate(&transformed->alloc, new_data);
            }
            anv_sll_destroy(transformed, should_free_data);
            return NULL;
        }
        curr = curr->next;
    }

    return transformed;
}

ANV_API void anv_sll_for_each(const ANVSinglyLinkedList* list, const anv_action_func action)
{
    if (!list || !action)
    {
        return;
    }

    const ANVSinglyLinkedNode* curr = list->head;
    while (curr)
    {
        action(curr->data);
        curr = curr->next;
    }
}

//==============================================================================
// List copying functions
//==============================================================================

ANV_API ANVSinglyLinkedList* anv_sll_copy(ANVSinglyLinkedList* list)
{
    if (!list)
    {
        return NULL;
    }

    ANVSinglyLinkedList* clone = anv_sll_create(&list->alloc);
    if (!clone)
    {
        return NULL;
    }

    if (list->size == 0)
    {
        return clone;
    }

    const ANVSinglyLinkedNode* curr = list->head;
    while (curr)
    {
        if (anv_sll_push_back(clone, curr->data) != 0)
        {
            anv_sll_destroy(clone, false);
            return NULL;
        }
        curr = curr->next;
    }

    return clone;
}

ANV_API ANVSinglyLinkedList* anv_sll_copy_deep(ANVSinglyLinkedList* list, const anv_copy_func copy_data, const bool should_free_data)
{
    if (!list || !copy_data)
    {
        return NULL;
    }

    ANVSinglyLinkedList* clone = anv_sll_create(&list->alloc);
    if (!clone)
    {
        return NULL;
    }

    if (list->size == 0)
    {
        return clone;
    }

    const ANVSinglyLinkedNode* curr = list->head;
    while (curr)
    {
        void* data_copy = copy_data(curr->data);
        if (!data_copy)
        {
            anv_sll_destroy(clone, should_free_data);
            return NULL;
        }
        if (anv_sll_push_back(clone, data_copy) != 0)
        {
            if (should_free_data)
            {
                anv_alloc_data_deallocate(&list->alloc, data_copy);
            }
            anv_sll_destroy(clone, should_free_data);
            return NULL;
        }
        curr = curr->next;
    }

    return clone;
}

//==============================================================================
// Iterator state and helper functions
//==============================================================================

typedef struct SListIteratorState
{
    ANVSinglyLinkedNode* current; // Current node
    ANVSinglyLinkedList* list;    // The list being iterated
} SListIteratorState;

static int sll_iterator_has_next(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return 0;
    }

    const SListIteratorState* state = it->data_state;
    return state->current != NULL;
}

static void* sll_iterator_get(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return NULL;
    }

    const SListIteratorState* state = it->data_state;
    if (!state->current)
    {
        return NULL;
    }

    return state->current->data;
}

static int sll_iterator_next(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return -1;
    }

    SListIteratorState* state = it->data_state;
    if (!state->current)
    {
        return -1;
    }

    state->current = state->current->next;
    return 0;
}

static int sll_iterator_has_prev(const ANVIterator* it)
{
    (void)it;
    return 0;
}

static int sll_iterator_prev(const ANVIterator* it)
{
    (void)it;
    return -1;
}

static void sll_iterator_reset(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return;
    }

    SListIteratorState* state = it->data_state;
    state->current = state->list->head;
}

static int sll_iterator_is_valid(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return 0;
    }

    const SListIteratorState* state = it->data_state;
    return state->list != NULL;
}

static void sll_iterator_destroy(ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return;
    }

    const SListIteratorState* state = it->data_state;
    anv_alloc_deallocate(&state->list->alloc, it->data_state);
    it->data_state = NULL;
}

ANV_API ANVIterator anv_sll_iterator(const ANVSinglyLinkedList* list)
{
    ANVIterator it = {0};

    it.get = sll_iterator_get;
    it.next = sll_iterator_next;
    it.has_next = sll_iterator_has_next;
    it.prev = sll_iterator_prev;
    it.has_prev = sll_iterator_has_prev;
    it.reset = sll_iterator_reset;
    it.is_valid = sll_iterator_is_valid;
    it.destroy = sll_iterator_destroy;

    if (!list)
    {
        return it;
    }

    SListIteratorState* state = anv_alloc_allocate(&list->alloc, sizeof(SListIteratorState));
    if (!state)
    {
        return it;
    }

    state->current = list->head;
    state->list = (ANVSinglyLinkedList*)list;

    it.alloc = list->alloc;
    it.data_state = state;

    return it;
}

ANV_API ANVSinglyLinkedList* anv_sll_from_iterator(ANVIterator* it, ANVAllocator* alloc, const bool should_copy)
{
    if (!it || !alloc)
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

    ANVSinglyLinkedList* list = anv_sll_create(alloc);
    if (!list)
    {
        return NULL;
    }

    while (it->has_next(it))
    {
        void* element = it->get(it);

        if (!element)
        {
            if (it->next(it) != 0)
            {
                break;
            }
            continue;
        }

        void* element_to_insert = element;
        if (should_copy)
        {
            element_to_insert = alloc->copy(element);
            if (!element_to_insert)
            {
                anv_sll_destroy(list, true);
                return NULL;
            }
        }

        if (anv_sll_push_back(list, element_to_insert) != 0)
        {
            if (should_copy)
            {
                anv_alloc_data_deallocate(alloc, element_to_insert);
            }
            anv_sll_destroy(list, should_copy);
            return NULL;
        }

        if (it->next(it) != 0)
        {
            break;
        }
    }

    return list;
}