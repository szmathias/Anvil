//
// Created by zack on 9/19/25.
//
// Implementation of binary search tree functions.

#include "binarysearchtree.h"
#include "stack.h"

//==============================================================================
// Helpers
//==============================================================================

static ANVBinarySearchTreeNode* anv_bst_node_create(const ANVAllocator* alloc, void* data)
{
    ANVBinarySearchTreeNode* node = anv_alloc_allocate(alloc, sizeof(ANVBinarySearchTreeNode));

    if (!node)
    {
        return NULL;
    }

    node->data = data;
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;

    return node;
}

static void anv_bst_node_destroy_recursive(ANVBinarySearchTreeNode* node, ANVAllocator* alloc, const bool should_free_data)
{
    if (!node)
    {
        return;
    }

    anv_bst_node_destroy_recursive(node->left, alloc, should_free_data);
    anv_bst_node_destroy_recursive(node->right, alloc, should_free_data);

    if (should_free_data && node->data)
    {
        anv_alloc_data_deallocate(alloc, node->data);
    }

    anv_alloc_deallocate(alloc, node);
}

static size_t anv_bst_node_height(const ANVBinarySearchTreeNode* node)
{
    if (!node)
    {
        return 0;
    }

    const size_t left_height = anv_bst_node_height(node->left);
    const size_t right_height = anv_bst_node_height(node->right);

    return 1 + (left_height > right_height ? left_height : right_height);
}

static ANVBinarySearchTreeNode* anv_bst_node_min(ANVBinarySearchTreeNode* node)
{
    if (!node)
    {
        return NULL;
    }

    while (node->left)
    {
        node = node->left;
    }

    return node;
}

static ANVBinarySearchTreeNode* anv_bst_node_max(ANVBinarySearchTreeNode* node)
{
    if (!node)
    {
        return NULL;
    }

    while (node->right)
    {
        node = node->right;
    }

    return node;
}

/**
 * Replace one subtree as a child of its parent with another subtree.
 */
static void anv_bst_transplant(ANVBinarySearchTree* tree, const ANVBinarySearchTreeNode* u, ANVBinarySearchTreeNode* v)
{
    if (!u->parent)
    {
        tree->root = v;
    }
    else if (u == u->parent->left)
    {
        u->parent->left = v;
    }
    else
    {
        u->parent->right = v;
    }

    if (v)
    {
        v->parent = u->parent;
    }
}

static void anv_bst_node_remove_with_parent(ANVBinarySearchTree* tree, ANVBinarySearchTreeNode* node,
                                            const bool should_free_data)
{
    if (!node->left)
    {
        anv_bst_transplant(tree, node, node->right);
    }
    else if (!node->right)
    {
        anv_bst_transplant(tree, node, node->left);
    }
    else
    {
        ANVBinarySearchTreeNode* successor = anv_bst_node_min(node->right);

        if (successor->parent != node)
        {
            anv_bst_transplant(tree, successor, successor->right);
            successor->right = node->right;
            successor->right->parent = successor;
        }

        anv_bst_transplant(tree, node, successor);
        successor->left = node->left;
        successor->left->parent = successor;
    }

    if (should_free_data && node->data)
    {
        anv_alloc_data_deallocate(&tree->alloc, node->data);
    }

    anv_alloc_deallocate(&tree->alloc, node);
}

static void anv_bst_node_inorder(const ANVBinarySearchTreeNode* node, const anv_action_func action)
{
    if (!node)
    {
        return;
    }

    anv_bst_node_inorder(node->left, action);
    action(node->data);
    anv_bst_node_inorder(node->right, action);
}

static void anv_bst_node_preorder(const ANVBinarySearchTreeNode* node, const anv_action_func action)
{
    if (!node)
    {
        return;
    }

    action(node->data);
    anv_bst_node_preorder(node->left, action);
    anv_bst_node_preorder(node->right, action);
}

static void anv_bst_node_postorder(const ANVBinarySearchTreeNode* node, const anv_action_func action)
{
    if (!node)
    {
        return;
    }

    anv_bst_node_postorder(node->left, action);
    anv_bst_node_postorder(node->right, action);
    action(node->data);
}

//==============================================================================
// Public API implementation
//==============================================================================

ANV_API ANVBinarySearchTree* anv_bst_create(ANVAllocator* alloc, const anv_compare_func compare)
{
    if (!compare)
    {
        return NULL;
    }

    if (!alloc)
    {
        return NULL;
    }

    ANVBinarySearchTree* tree = anv_alloc_allocate(alloc, sizeof(ANVBinarySearchTree));
    if (!tree)
    {
        return NULL;
    }

    tree->root = NULL;
    tree->size = 0;
    tree->compare = compare;
    tree->alloc = *alloc;

    return tree;
}

ANV_API void anv_bst_destroy(ANVBinarySearchTree* tree, const bool should_free_data)
{
    if (!tree)
    {
        return;
    }

    anv_bst_node_destroy_recursive(tree->root, &tree->alloc, should_free_data);
    anv_alloc_deallocate(&tree->alloc, tree);
}

ANV_API void anv_bst_clear(ANVBinarySearchTree* tree, const bool should_free_data)
{
    if (!tree)
    {
        return;
    }

    anv_bst_node_destroy_recursive(tree->root, &tree->alloc, should_free_data);
    tree->root = NULL;
    tree->size = 0;
}

ANV_API size_t anv_bst_size(const ANVBinarySearchTree* tree)
{
    return tree ? tree->size : 0;
}

ANV_API int anv_bst_is_empty(const ANVBinarySearchTree* tree)
{
    return !tree || tree->size == 0;
}

ANV_API size_t anv_bst_height(const ANVBinarySearchTree* tree)
{
    return tree ? anv_bst_node_height(tree->root) : 0;
}

ANV_API int anv_bst_contains(const ANVBinarySearchTree* tree, const void* data)
{
    if (!tree || !data)
    {
        return 0;
    }

    const ANVBinarySearchTreeNode* current = tree->root;

    while (current)
    {
        const int cmp = tree->compare(data, current->data);

        if (cmp == 0)
        {
            return 1;
        }

        if (cmp < 0)
        {
            current = current->left;
        }
        else
        {
            current = current->right;
        }
    }

    return 0;
}

ANV_API void* anv_bst_min(const ANVBinarySearchTree* tree)
{
    if (!tree || !tree->root)
    {
        return NULL;
    }

    const ANVBinarySearchTreeNode* min_node = anv_bst_node_min(tree->root);
    return min_node ? min_node->data : NULL;
}

ANV_API void* anv_bst_max(const ANVBinarySearchTree* tree)
{
    if (!tree || !tree->root)
    {
        return NULL;
    }

    const ANVBinarySearchTreeNode* max_node = anv_bst_node_max(tree->root);
    return max_node ? max_node->data : NULL;
}

ANV_API int anv_bst_insert(ANVBinarySearchTree* tree, void* data)
{
    if (!tree || !data)
    {
        return -1;
    }

    if (!tree->root)
    {
        tree->root = anv_bst_node_create(&tree->alloc, data);
        if (!tree->root)
        {
            return -1;
        }
        tree->size++;
        return 0;
    }

    ANVBinarySearchTreeNode* current = tree->root;
    ANVBinarySearchTreeNode* parent = NULL;

    while (current)
    {
        parent = current;
        const int cmp = tree->compare(data, current->data);

        if (cmp == 0)
        {
            return 1; // Duplicate
        }

        if (cmp < 0)
        {
            current = current->left;
        }
        else
        {
            current = current->right;
        }
    }

    ANVBinarySearchTreeNode* new_node = anv_bst_node_create(&tree->alloc, data);
    if (!new_node || !parent)
    {
        return -1;
    }

    new_node->parent = parent;

    const int cmp = tree->compare(data, parent->data);
    if (cmp < 0)
    {
        parent->left = new_node;
    }
    else
    {
        parent->right = new_node;
    }

    tree->size++;
    return 0;
}

ANV_API int anv_bst_remove(ANVBinarySearchTree* tree, const void* data, const bool should_free_data)
{
    if (!tree || !data)
    {
        return -1;
    }

    ANVBinarySearchTreeNode* current = tree->root;

    while (current)
    {
        const int cmp = tree->compare(data, current->data);

        if (cmp == 0)
        {
            anv_bst_node_remove_with_parent(tree, current, should_free_data);
            tree->size--;
            return 0;
        }

        if (cmp < 0)
        {
            current = current->left;
        }
        else
        {
            current = current->right;
        }
    }

    return -1;
}

ANV_API void anv_bst_inorder(const ANVBinarySearchTree* tree, const anv_action_func action)
{
    if (!tree || !action)
    {
        return;
    }

    anv_bst_node_inorder(tree->root, action);
}

ANV_API void anv_bst_preorder(const ANVBinarySearchTree* tree, const anv_action_func action)
{
    if (!tree || !action)
    {
        return;
    }

    anv_bst_node_preorder(tree->root, action);
}

ANV_API void anv_bst_postorder(const ANVBinarySearchTree* tree, const anv_action_func action)
{
    if (!tree || !action)
    {
        return;
    }

    anv_bst_node_postorder(tree->root, action);
}

//==============================================================================
// Iterator implementation
//==============================================================================

/**
 * Traversal types for BST iterators
 */
typedef enum
{
    BST_TRAVERSAL_INORDER,
    BST_TRAVERSAL_PREORDER,
    BST_TRAVERSAL_POSTORDER
} BSTTraversalType;

/**
 * State structure for BST iterator.
 */
typedef struct BSTIteratorState
{
    const ANVBinarySearchTree* tree;  // Source tree
    ANVStack* stack;                  // Stack for iterative traversal
    ANVBinarySearchTreeNode* current; // Current node
    BSTTraversalType traversal_type;  // Type of traversal
    bool finished;                    // Has iteration finished
} BSTIteratorState;

/**
 * Setup iterator for in-order traversal.
 */
static void bst_setup_inorder(BSTIteratorState* state)
{
    ANVBinarySearchTreeNode* node = state->tree->root;
    // Push all left nodes to stack
    while (node)
    {
        anv_stack_push(state->stack, node);
        node = node->left;
    }

    if (!anv_stack_is_empty(state->stack))
    {
        state->current = anv_stack_peek(state->stack);
        anv_stack_pop(state->stack, false); // Don't free the node data
    }
    else
    {
        state->current = NULL;
    }
}

/**
 * Setup iterator for pre-order traversal.
 */
static void bst_setup_preorder(BSTIteratorState* state)
{
    if (state->tree->root)
    {
        anv_stack_push(state->stack, state->tree->root);
        state->current = anv_stack_peek(state->stack);
        anv_stack_pop(state->stack, false); // Don't free the node data
    }
    else
    {
        state->current = NULL;
    }
}

/**
 * Setup iterator for post-order traversal.
 */
static void bst_setup_postorder(BSTIteratorState* state)
{
    ANVBinarySearchTreeNode* node = state->tree->root;

    // Find the first node in post-order (leftmost leaf)
    while (node)
    {
        anv_stack_push(state->stack, node);
        if (node->left)
        {
            node = node->left;
        }
        else if (node->right)
        {
            node = node->right;
        }
        else
        {
            // Leaf node - this is our first node
            state->current = anv_stack_peek(state->stack);
            anv_stack_pop(state->stack, false); // Don't free the node data
            break;
        }
    }

    if (!node)
    {
        state->current = NULL;
    }
}

/**
 * Get current element from BST iterator.
 */
static void* bst_iterator_get(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return NULL;
    }

    const BSTIteratorState* state = it->data_state;
    return state->current ? state->current->data : NULL;
}

/**
 * Check if BST iterator has next element.
 */
static int bst_iterator_has_next(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return 0;
    }

    const BSTIteratorState* state = it->data_state;
    return !state->finished && (state->current != NULL || !anv_stack_is_empty(state->stack));
}

/**
 * Advance BST iterator to next element.
 */
static int bst_iterator_next(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return -1;
    }

    BSTIteratorState* state = it->data_state;

    if (state->finished)
    {
        return 0;
    }

    switch (state->traversal_type)
    {
        case BST_TRAVERSAL_INORDER:
            {
                if (!state->current)
                {
                    state->finished = true;
                    return 0;
                }

                // Push all left children of right subtree
                ANVBinarySearchTreeNode* node = state->current->right;
                while (node)
                {
                    anv_stack_push(state->stack, node);
                    node = node->left;
                }

                if (!anv_stack_is_empty(state->stack))
                {
                    state->current = anv_stack_peek(state->stack);
                    anv_stack_pop(state->stack, false);
                }
                else
                {
                    state->current = NULL;
                    state->finished = true;
                    return 0;
                }
                break;
            }

        case BST_TRAVERSAL_PREORDER:
            {
                if (!state->current)
                {
                    state->finished = true;
                    return 0;
                }

                // Push right child first, then left (so left is processed first)
                if (state->current->right)
                {
                    anv_stack_push(state->stack, state->current->right);
                }
                if (state->current->left)
                {
                    anv_stack_push(state->stack, state->current->left);
                }

                if (!anv_stack_is_empty(state->stack))
                {
                    state->current = anv_stack_peek(state->stack);
                    anv_stack_pop(state->stack, false);
                }
                else
                {
                    state->current = NULL;
                    state->finished = true;
                    return 0;
                }
                break;
            }

        case BST_TRAVERSAL_POSTORDER:
            {
                if (!state->current)
                {
                    state->finished = true;
                    return 0;
                }

                // Post-order traversal is complex - need to find next node
                if (anv_stack_is_empty(state->stack))
                {
                    state->current = NULL;
                    state->finished = true;
                    return 0;
                }

                const ANVBinarySearchTreeNode* top = anv_stack_peek(state->stack);

                // If current node is left child of top, process right subtree
                if (top->left == state->current && top->right)
                {
                    ANVBinarySearchTreeNode* node = top->right;
                    while (node)
                    {
                        anv_stack_push(state->stack, node);
                        if (node->left)
                        {
                            node = node->left;
                        }
                        else if (node->right)
                        {
                            node = node->right;
                        }
                        else
                        {
                            state->current = anv_stack_peek(state->stack);
                            anv_stack_pop(state->stack, false);
                            return 0;
                        }
                    }
                }

                // Otherwise, pop and return parent
                state->current = anv_stack_peek(state->stack);
                anv_stack_pop(state->stack, false);
                break;
            }
        default:
            return -1; // Unknown traversal type
    }

    return 0;
}

/**
 * BST iterator has_prev - not supported for tree traversals.
 */
static int bst_iterator_has_prev(const ANVIterator* it)
{
    (void)it;
    return 0; // Tree traversals don't support backward iteration
}

/**
 * BST iterator prev - not supported for tree traversals.
 */
static int bst_iterator_prev(const ANVIterator* it)
{
    (void)it;
    return -1; // Tree traversals don't support backward iteration
}

/**
 * Reset BST iterator to beginning.
 */
static void bst_iterator_reset(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return;
    }

    BSTIteratorState* state = it->data_state;
    anv_stack_clear(state->stack, false); // Clear stack without freeing node data
    state->finished = false;
    state->current = NULL;

    switch (state->traversal_type)
    {
        case BST_TRAVERSAL_INORDER:
            bst_setup_inorder(state);
            break;
        case BST_TRAVERSAL_PREORDER:
            bst_setup_preorder(state);
            break;
        case BST_TRAVERSAL_POSTORDER:
            bst_setup_postorder(state);
            break;

        default:
            break;
    }
}

/**
 * Check if BST iterator is valid.
 */
static int bst_iterator_is_valid(const ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return 0;
    }

    const BSTIteratorState* state = it->data_state;
    return state->tree != NULL && state->stack != NULL;
}

/**
 * Destroy BST iterator and free resources.
 */
static void bst_iterator_destroy(ANVIterator* it)
{
    if (!it || !it->data_state)
    {
        return;
    }

    BSTIteratorState* state = it->data_state;
    if (state->stack)
    {
        anv_stack_destroy(state->stack, false); // Don't free node data
    }
    anv_alloc_deallocate(&it->alloc, state);
    it->data_state = NULL;
}

/**
 * Create a BST iterator with specified traversal type.
 */
static ANVIterator bst_create_iterator(ANVBinarySearchTree* tree, const BSTTraversalType traversal_type)
{
    ANVIterator it = {0};

    it.get = bst_iterator_get;
    it.has_next = bst_iterator_has_next;
    it.next = bst_iterator_next;
    it.has_prev = bst_iterator_has_prev;
    it.prev = bst_iterator_prev;
    it.reset = bst_iterator_reset;
    it.is_valid = bst_iterator_is_valid;
    it.destroy = bst_iterator_destroy;

    if (!tree)
    {
        return it;
    }

    BSTIteratorState* state = anv_alloc_allocate(&tree->alloc, sizeof(BSTIteratorState));
    if (!state)
    {
        return it;
    }

    state->tree = tree;
    state->traversal_type = traversal_type;
    state->current = NULL;
    state->finished = false;

    // Create stack for traversal
    state->stack = anv_stack_create(&tree->alloc);
    if (!state->stack)
    {
        anv_alloc_deallocate(&tree->alloc, state);
        return it;
    }

    // Setup initial state based on traversal type
    switch (traversal_type)
    {
        case BST_TRAVERSAL_INORDER:
            bst_setup_inorder(state);
            break;
        case BST_TRAVERSAL_PREORDER:
            bst_setup_preorder(state);
            break;
        case BST_TRAVERSAL_POSTORDER:
            bst_setup_postorder(state);
            // Fallthrough
        default:
            break;
    }

    if (state->tree->size == 0)
    {
        state->finished = true;
    }

    it.alloc = tree->alloc;
    it.data_state = state;

    return it;
}

ANV_API ANVIterator anv_bst_iterator(ANVBinarySearchTree* tree)
{
    return bst_create_iterator(tree, BST_TRAVERSAL_INORDER);
}

ANV_API ANVIterator anv_bst_iterator_preorder(ANVBinarySearchTree* tree)
{
    return bst_create_iterator(tree, BST_TRAVERSAL_PREORDER);
}

ANV_API ANVIterator anv_bst_iterator_postorder(ANVBinarySearchTree* tree)
{
    return bst_create_iterator(tree, BST_TRAVERSAL_POSTORDER);
}

ANV_API ANVBinarySearchTree* anv_bst_from_iterator(ANVIterator* it, ANVAllocator* alloc, const anv_compare_func compare, const bool should_copy)
{
    if (!it || !compare || !alloc)
    {
        return NULL;
    }

    ANVBinarySearchTree* tree = anv_bst_create(alloc, compare);
    if (!tree)
    {
        return NULL;
    }

    // Iterate through all elements and insert them into the tree
    while (it->has_next && it->has_next(it))
    {
        void* data = it->get ? it->get(it) : NULL;
        if (data)
        {
            void* insert_data = data;
            if (should_copy && alloc->copy)
            {
                insert_data = anv_alloc_copy(alloc, data);
                if (!insert_data)
                {
                    anv_bst_destroy(tree, should_copy);
                    return NULL;
                }
            }

            const int result = anv_bst_insert(tree, insert_data);
            // On error, clean up and return NULL
            if (result < 0)
            {
                if (should_copy && insert_data != data)
                {
                    anv_alloc_data_deallocate(alloc, insert_data);
                }
                anv_bst_destroy(tree, should_copy);
                return NULL;
            }

            // Indicates a duplicate was found
            if (result == 1 && should_copy && insert_data != data)
            {
                anv_alloc_data_deallocate(alloc, insert_data);
            }
        }

        if (it->next && it->next(it) != 0)
        {
            break; // Iterator exhausted or error
        }
    }

    return tree;
}