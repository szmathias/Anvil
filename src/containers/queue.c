//
// Created by zack on 9/9/25.
//

#include "queue.h"

//==============================================================================
// Helper functions
//==============================================================================

static ANVQueueNode* create_node(const ANVQueue* queue, void* data)
{
    ANVQueueNode* node = anv_alloc_allocate(&queue->alloc, sizeof(ANVQueueNode));
    if (!node)
    {
        return NULL;
    }

    node->data = data;
    node->next = NULL;
    return node;
}

static void free_node(const ANVQueue* queue, ANVQueueNode* node, const bool should_free_data)
{
    if (!node)
    {
        return;
    }

    if (should_free_data && node->data)
    {
        anv_alloc_data_deallocate(&queue->alloc, node->data);
    }

    anv_alloc_deallocate(&queue->alloc, node);
}

//==============================================================================
// Creation and destruction functions
//==============================================================================

ANV_API ANVQueue* anv_queue_create(ANVAllocator* alloc)
{
    if (!alloc)
    {
        return NULL;
    }

    ANVQueue* queue = anv_alloc_allocate(alloc, sizeof(ANVQueue));
    if (!queue)
    {
        return NULL;
    }

    queue->front = NULL;
    queue->back = NULL;
    queue->size = 0;
    queue->alloc = *alloc;

    return queue;
}

ANV_API void anv_queue_destroy(ANVQueue* queue, const bool should_free_data)
{
    if (!queue)
    {
        return;
    }

    anv_queue_clear(queue, should_free_data);
    anv_alloc_deallocate(&queue->alloc, queue);
}

ANV_API void anv_queue_clear(ANVQueue* queue, const bool should_free_data)
{
    if (!queue)
    {
        return;
    }

    ANVQueueNode* current = queue->front;
    while (current)
    {
        ANVQueueNode* next = current->next;
        free_node(queue, current, should_free_data);
        current = next;
    }

    queue->front = NULL;
    queue->back = NULL;
    queue->size = 0;
}

//==============================================================================
// Information functions
//==============================================================================

ANV_API size_t anv_queue_size(const ANVQueue* queue)
{
    return queue ? queue->size : 0;
}

ANV_API int anv_queue_is_empty(const ANVQueue* queue)
{
    return !queue || queue->size == 0;
}

ANV_API int anv_queue_equals(const ANVQueue* queue1, const ANVQueue* queue2, const anv_compare_func compare)
{
    if (!queue1 || !queue2 || !compare)
    {
        return -1;
    }

    if (queue1 == queue2)
    {
        return 1;
    }

    if (queue1->size != queue2->size)
    {
        return 0;
    }

    const ANVQueueNode* node1 = queue1->front;
    const ANVQueueNode* node2 = queue2->front;

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
// Element access functions
//==============================================================================

ANV_API void* anv_queue_front(const ANVQueue* queue)
{
    if (!queue || !queue->front)
    {
        return NULL;
    }
    return queue->front->data;
}

ANV_API void* anv_queue_back(const ANVQueue* queue)
{
    if (!queue || !queue->back)
    {
        return NULL;
    }
    return queue->back->data;
}

//==============================================================================
// Queue operations
//==============================================================================

ANV_API int anv_queue_enqueue(ANVQueue* queue, void* data)
{
    if (!queue)
    {
        return -1;
    }

    ANVQueueNode* new_node = create_node(queue, data);
    if (!new_node)
    {
        return -1;
    }

    if (queue->size == 0)
    {
        queue->front = new_node;
        queue->back = new_node;
    }
    else
    {
        queue->back->next = new_node;
        queue->back = new_node;
    }

    queue->size++;
    return 0;
}

ANV_API int anv_queue_dequeue(ANVQueue* queue, const bool should_free_data)
{
    if (!queue || !queue->front)
    {
        return -1;
    }

    ANVQueueNode* old_front = queue->front;
    queue->front = old_front->next;

    if (!queue->front)
    {
        queue->back = NULL;
    }

    queue->size--;
    free_node(queue, old_front, should_free_data);
    return 0;
}

ANV_API void* anv_queue_dequeue_data(ANVQueue* queue)
{
    if (!queue || !queue->front)
    {
        return NULL;
    }

    ANVQueueNode* old_front = queue->front;
    void* data = old_front->data;

    queue->front = old_front->next;

    if (!queue->front)
    {
        queue->back = NULL;
    }

    queue->size--;

    free_node(queue, old_front, false);
    return data;
}

//==============================================================================
// Higher-order functions
//==============================================================================

ANV_API void anv_queue_for_each(const ANVQueue* queue, const anv_action_func action)
{
    if (!queue || !action)
    {
        return;
    }

    const ANVQueueNode* current = queue->front;
    while (current)
    {
        action(current->data);
        current = current->next;
    }
}

//==============================================================================
// Queue copying functions
//==============================================================================

ANV_API ANVQueue* anv_queue_copy(ANVQueue* queue)
{
    if (!queue)
    {
        return NULL;
    }

    ANVQueue* new_queue = anv_queue_create(&queue->alloc);
    if (!new_queue)
    {
        return NULL;
    }

    if (queue->size == 0)
    {
        return new_queue;
    }

    const ANVQueueNode* current = queue->front;
    while (current)
    {
        if (anv_queue_enqueue(new_queue, current->data) != 0)
        {
            anv_queue_destroy(new_queue, false);
            return NULL;
        }
        current = current->next;
    }

    return new_queue;
}

ANV_API ANVQueue* anv_queue_copy_deep(ANVQueue* queue, const bool should_free_data)
{
    if (!queue || !queue->alloc.copy)
    {
        return NULL;
    }

    ANVQueue* new_queue = anv_queue_create(&queue->alloc);
    if (!new_queue)
    {
        return NULL;
    }

    if (queue->size == 0)
    {
        return new_queue;
    }

    const ANVQueueNode* current = queue->front;
    while (current)
    {
        void* copied_data = queue->alloc.copy(current->data);
        if (!copied_data)
        {
            anv_queue_destroy(new_queue, should_free_data);
            return NULL;
        }

        if (anv_queue_enqueue(new_queue, copied_data) != 0)
        {
            if (should_free_data)
            {
                anv_alloc_data_deallocate(&queue->alloc, copied_data);
            }
            anv_queue_destroy(new_queue, should_free_data);
            return NULL;
        }
        current = current->next;
    }

    return new_queue;
}

//==============================================================================
// Iterator implementation
//==============================================================================

typedef struct QueueIteratorState
{
    const ANVQueue* queue;
    ANVQueueNode* current;
    ANVQueueNode* start;
} QueueIteratorState;

static void* queue_iterator_get(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return NULL;
    }

    const QueueIteratorState* state = it->data_state;
    return state->current ? state->current->data : NULL;
}

static int queue_iterator_has_next(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return 0;
    }

    const QueueIteratorState* state = it->data_state;
    return state->current != NULL;
}

static int queue_iterator_next(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return -1;
    }

    QueueIteratorState* state = it->data_state;
    if (!state->current)
    {
        return -1;
    }

    state->current = state->current->next;
    return 0;
}

static int queue_iterator_has_prev(const ANVIterator* it)
{
    (void)it;
    return 0;
}

static int queue_iterator_prev(const ANVIterator* it)
{
    (void)it;
    return -1;
}

static void queue_iterator_reset(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return;
    }

    QueueIteratorState* state = it->data_state;
    state->current = state->start;
}

static int queue_iterator_is_valid(const ANVIterator* it)
{
    return it && it->data_state != NULL;
}

static void queue_iterator_destroy(ANVIterator* it)
{
    if (!it)
    {
        return;
    }

    if (it->data_state)
    {
        QueueIteratorState* state = it->data_state;
        anv_alloc_deallocate(&state->queue->alloc, state);
    }
    it->data_state = NULL;
}

ANV_API ANVIterator anv_queue_iterator(const ANVQueue* queue)
{
    ANVIterator it = {0};

    it.get = queue_iterator_get;
    it.has_next = queue_iterator_has_next;
    it.next = queue_iterator_next;
    it.has_prev = queue_iterator_has_prev;
    it.prev = queue_iterator_prev;
    it.reset = queue_iterator_reset;
    it.is_valid = queue_iterator_is_valid;
    it.destroy = queue_iterator_destroy;

    if (!queue)
    {
        return it;
    }

    QueueIteratorState* state = anv_alloc_allocate(&queue->alloc, sizeof(QueueIteratorState));
    if (!state)
    {
        return it;
    }

    state->queue = queue;
    state->current = queue->front;
    state->start = queue->front;

    it.alloc = queue->alloc;
    it.data_state = state;

    return it;
}

ANV_API ANVQueue* anv_queue_from_iterator(ANVIterator* it, ANVAllocator* alloc, const bool should_copy)
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

    ANVQueue* queue = anv_queue_create(alloc);
    if (!queue)
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
                anv_queue_destroy(queue, true);
                return NULL;
            }
        }

        if (anv_queue_enqueue(queue, element_to_insert) != 0)
        {
            if (should_copy)
            {
                anv_alloc_data_deallocate(alloc, element_to_insert);
            }
            anv_queue_destroy(queue, should_copy);
            return NULL;
        }

        if (it->next(it) != 0)
        {
            break;
        }
    }

    return queue;
}