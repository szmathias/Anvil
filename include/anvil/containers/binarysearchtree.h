//
// Created by zack on 9/19/25.
//

#ifndef ANVIL_BINARYSEARCHTREE_H
#define ANVIL_BINARYSEARCHTREE_H

#include "iterator.h"
#include "anvil/common.h"

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Type definitions
//==============================================================================

// Node of binary search tree
typedef struct ANVBinarySearchTreeNode
{
        void* data;                             // Pointer to user data
        struct ANVBinarySearchTreeNode* left;   // Pointer to left child
        struct ANVBinarySearchTreeNode* right;  // Pointer to right child
        struct ANVBinarySearchTreeNode* parent; // Pointer to parent node
} ANVBinarySearchTreeNode;

// Binary search tree structure
typedef struct ANVBinarySearchTree
{
        ANVBinarySearchTreeNode* root; // Pointer to root node
        size_t size;                   // Number of nodes in tree
        anv_compare_func compare;      // Comparison function for ordering
        ANVAllocator alloc;            // Custom allocator
} ANVBinarySearchTree;

//==============================================================================
// Creation and destruction functions
//==============================================================================

/**
 * Create a new, empty binary search tree.
 *
 * @param alloc Custom allocator
 * @param compare Comparison function for ordering elements
 * @return Pointer to new BinarySearchTree, or NULL on failure
 */
ANV_API ANVBinarySearchTree* anv_bst_create(ANVAllocator* alloc, anv_compare_func compare);

/**
 * Destroy the tree and free all nodes.
 *
 * @param tree The tree to destroy
 * @param should_free_data Whether to free the data stored in nodes
 */
ANV_API void anv_bst_destroy(ANVBinarySearchTree* tree, bool should_free_data);

/**
 * Clear all nodes from the tree, but keep the tree structure intact.
 *
 * @param tree The tree to clear
 * @param should_free_data Whether to free the data stored in nodes
 */
ANV_API void anv_bst_clear(ANVBinarySearchTree* tree, bool should_free_data);

//==============================================================================
// Information functions
//==============================================================================

/**
 * Get the number of elements in the tree.
 *
 * @param tree The tree to query
 * @return Number of elements, or 0 if tree is NULL
 */
ANV_API size_t anv_bst_size(const ANVBinarySearchTree* tree);

/**
 * Check if the tree is empty.
 *
 * @param tree The tree to check
 * @return 1 if tree is empty or NULL, 0 if tree contains elements
 */
ANV_API int anv_bst_is_empty(const ANVBinarySearchTree* tree);

/**
 * Get the height of the tree.
 *
 * @param tree The tree to query
 * @return Height of the tree (0 for empty tree, 1 for root only)
 */
ANV_API size_t anv_bst_height(const ANVBinarySearchTree* tree);

/**
 * Check if a value exists in the tree.
 *
 * @param tree The tree to search
 * @param data The data to search for
 * @return 1 if found, 0 if not found or on error
 */
ANV_API int anv_bst_contains(const ANVBinarySearchTree* tree, const void* data);

/**
 * Find the minimum element in the tree.
 *
 * @param tree The tree to search
 * @return Pointer to minimum data, or NULL if tree is empty
 */
ANV_API void* anv_bst_min(const ANVBinarySearchTree* tree);

/**
 * Find the maximum element in the tree.
 *
 * @param tree The tree to search
 * @return Pointer to maximum data, or NULL if tree is empty
 */
ANV_API void* anv_bst_max(const ANVBinarySearchTree* tree);

//==============================================================================
// Insertion and removal functions
//==============================================================================

/**
 * Insert data into the tree.
 *
 * @param tree The tree to modify
 * @param data Pointer to the data to insert (ownership transferred to tree)
 * @return 0 on success, -1 on error, 1 if duplicate exists
 */
ANV_API int anv_bst_insert(ANVBinarySearchTree* tree, void* data);

/**
 * Remove the first occurrence of data from the tree.
 *
 * @param tree The tree to modify
 * @param data The data to remove
 * @param should_free_data Whether to free the data
 * @return 0 on success, -1 if not found or on error
 */
ANV_API int anv_bst_remove(ANVBinarySearchTree* tree, const void* data, bool should_free_data);

//==============================================================================
// Traversal functions
//==============================================================================

/**
 * Perform in-order traversal of the tree.
 * Visits nodes in sorted order.
 *
 * @param tree The tree to traverse
 * @param action Function to call for each element
 */
ANV_API void anv_bst_inorder(const ANVBinarySearchTree* tree, anv_action_func action);

/**
 * Perform pre-order traversal of the tree.
 * Visits root before children.
 *
 * @param tree The tree to traverse
 * @param action Function to call for each element
 */
ANV_API void anv_bst_preorder(const ANVBinarySearchTree* tree, anv_action_func action);

/**
 * Perform post-order traversal of the tree.
 * Visits children before root.
 *
 * @param tree The tree to traverse
 * @param action Function to call for each element
 */
ANV_API void anv_bst_postorder(const ANVBinarySearchTree* tree, anv_action_func action);

//==============================================================================
// Iterator support
//==============================================================================

/**
 * Create an iterator for in-order traversal of the tree.
 * Visits nodes in sorted order.
 *
 * @param tree The tree to iterate over
 * @return Iterator for in-order traversal
 */
ANV_API ANVIterator anv_bst_iterator(ANVBinarySearchTree* tree);

/**
 * Create an iterator for pre-order traversal of the tree.
 * Visits root before children.
 *
 * @param tree The tree to iterate over
 * @return Iterator for pre-order traversal
 */
ANV_API ANVIterator anv_bst_iterator_preorder(ANVBinarySearchTree* tree);

/**
 * Create an iterator for post-order traversal of the tree.
 * Visits children before root.
 *
 * @param tree The tree to iterate over
 * @return Iterator for post-order traversal
 */
ANV_API ANVIterator anv_bst_iterator_postorder(ANVBinarySearchTree* tree);

/**
 * Create a new binary search tree from an iterator.
 * Elements from the iterator will be inserted in the order they appear.
 *
 * @param it Iterator to read elements from
 * @param alloc Allocator for the new tree
 * @param compare Comparison function for the new tree
 * @param should_copy Whether to copy the data from iterator elements
 * @return New tree containing elements from iterator, or NULL on failure
 */
ANV_API ANVBinarySearchTree* anv_bst_from_iterator(ANVIterator* it, ANVAllocator* alloc, anv_compare_func compare, bool should_copy);

#ifdef __cplusplus
}
#endif

#endif // ANVIL_BINARYSEARCHTREE_H
