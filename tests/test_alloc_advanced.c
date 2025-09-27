//
// Created by GitHub Copilot on 9/11/25.
// Advanced allocator integration tests
//

#include <stdio.h>
#include <stdlib.h>

#include "TestAssert.h"
#include "TestHelpers.h"
#include "common/common.h"
#include "containers/arraylist.h"
#include "containers/singlylinkedlist.h"

//==============================================================================
// Pool Allocator Implementation
//==============================================================================

#define POOL_BLOCK_SIZE 64
#define POOL_NUM_BLOCKS 16

typedef struct PoolBlock
{
    struct PoolBlock* next;
    char data[POOL_BLOCK_SIZE];
} PoolBlock;

typedef struct
{
    PoolBlock blocks[POOL_NUM_BLOCKS];
    PoolBlock* free_list;
    int initialized;
} Pool;

static Pool pool_global = {0};

static void pool_init(void)
{
    if (pool_global.initialized)
        return;

    // Initialize free list
    pool_global.free_list = NULL;
    for (int i = 0; i < POOL_NUM_BLOCKS; i++)
    {
        pool_global.blocks[i].next = pool_global.free_list;
        pool_global.free_list = &pool_global.blocks[i];
    }
    pool_global.initialized = 1;
}

static void* pool_alloc(const size_t size)
{
    if (size > POOL_BLOCK_SIZE)
    {
        return NULL; // Size too large for pool
    }

    pool_init();

    if (!pool_global.free_list)
    {
        return NULL; // Pool exhausted
    }

    PoolBlock* block = pool_global.free_list;
    pool_global.free_list = block->next;
    return block->data;
}

static void pool_free(void* ptr)
{
    if (!ptr)
        return;

    // Find which block this pointer belongs to
    for (int i = 0; i < POOL_NUM_BLOCKS; i++)
    {
        if (ptr == pool_global.blocks[i].data)
        {
            pool_global.blocks[i].next = pool_global.free_list;
            pool_global.free_list = &pool_global.blocks[i];
            return;
        }
    }
}

static void pool_reset(void)
{
    pool_global.initialized = 0;
    pool_init();
}

//==============================================================================
// Debug Allocator Implementation (tracks all allocations)
//==============================================================================

#define MAX_ALLOCATIONS 100

typedef struct
{
    void* ptr;
    size_t size;
    const char* file;
    int line;
} AllocationInfo;

static AllocationInfo allocations[MAX_ALLOCATIONS];
static int allocation_count = 0;
static size_t total_allocated = 0;
static size_t peak_allocated = 0;

static void* debug_alloc(const size_t size)
{
    void* ptr = malloc(size);
    if (!ptr)
        return NULL;

    // Record allocation
    if (allocation_count < MAX_ALLOCATIONS)
    {
        allocations[allocation_count].ptr = ptr;
        allocations[allocation_count].size = size;
        allocations[allocation_count].file = __FILE__;
        allocations[allocation_count].line = __LINE__;
        allocation_count++;
    }

    total_allocated += size;
    if (total_allocated > peak_allocated)
    {
        peak_allocated = total_allocated;
    }

    return ptr;
}

static void debug_free(void* ptr)
{
    if (!ptr)
        return;

    // Find and remove allocation record
    for (int i = 0; i < allocation_count; i++)
    {
        if (allocations[i].ptr == ptr)
        {
            total_allocated -= allocations[i].size;

            // Remove by shifting remaining elements
            for (int j = i; j < allocation_count - 1; j++)
            {
                allocations[j] = allocations[j + 1];
            }
            allocation_count--;
            break;
        }
    }

    free(ptr);
}

static void debug_reset(void)
{
    allocation_count = 0;
    total_allocated = 0;
    peak_allocated = 0;
}

static void debug_report(void)
{
    printf("=== Debug Allocator Report ===\n");
    printf("Active allocations: %d\n", allocation_count);
    printf("Total allocated: %zu bytes\n", total_allocated);
    printf("Peak allocated: %zu bytes\n", peak_allocated);

    if (allocation_count > 0)
    {
        printf("Memory leaks detected:\n");
        for (int i = 0; i < allocation_count; i++)
        {
            printf("  %p: %zu bytes (from %s:%d)\n",
                   allocations[i].ptr, allocations[i].size,
                   allocations[i].file, allocations[i].line);
        }
    }
    printf("===============================\n");
}

//==============================================================================
// Test Functions
//==============================================================================

int test_pool_allocator_integration(void)
{
    pool_reset();

    // Use separate allocators: default for ArrayList structure, pool for data
    const ANVAllocator pool_data_alloc = anv_alloc_custom(pool_alloc, pool_free, NULL, int_copy);
    ANVAllocator default_alloc = anv_alloc_default();

    // Test with ArrayList using default allocator for structure
    ANVArrayList* list = anv_arraylist_create(&default_alloc, 4);
    ASSERT(list != NULL);

    // Add elements using pool allocator for the data
    for (int i = 0; i < 8; i++)
    {
        int* value = anv_alloc_malloc(&pool_data_alloc, sizeof(int));
        ASSERT(value != NULL);
        *value = i * 10;
        anv_arraylist_push_back(list, value);
    }

    ASSERT_EQ(anv_arraylist_size(list), 8);

    // Verify values
    for (size_t i = 0; i < 8; i++)
    {
        const int* value = anv_arraylist_get(list, i);
        ASSERT_EQ(*value, (int)(i * 10));
    }

    // Test pool exhaustion with remaining blocks
    void* ptrs[POOL_NUM_BLOCKS];
    int allocated = 0;

    for (int i = 0; i < POOL_NUM_BLOCKS + 5; i++)
    {
        void* ptr = anv_alloc_malloc(&pool_data_alloc, 32);
        if (ptr)
        {
            ptrs[allocated++] = ptr;
        }
    }

    // Should have allocated some blocks but hit the limit
    ASSERT(allocated < POOL_NUM_BLOCKS);
    ASSERT(allocated > 0);

    // Free allocated blocks
    for (int i = 0; i < allocated; i++)
    {
        anv_alloc_free(&pool_data_alloc, ptrs[i]);
    }

    // Clean up: manually free data with pool allocator, then destroy list
    for (size_t i = 0; i < anv_arraylist_size(list); i++)
    {
        int* value = anv_arraylist_get(list, i);
        anv_alloc_free(&pool_data_alloc, value);
    }
    anv_arraylist_destroy(list, false); // Don't auto-free data since we freed it manually

    return TEST_SUCCESS;
}

static void* debug_int_copy(const void* data)
{
    const int* original = data;
    int* copy = debug_alloc(sizeof(int));
    if (copy)
    {
        *copy = *original;
    }
    return copy;
}

int test_debug_allocator_tracking(void)
{
    debug_reset();

    const ANVAllocator alloc = anv_alloc_custom(debug_alloc, debug_free, debug_free, debug_int_copy);

    // Test allocation tracking
    void* ptr1 = anv_alloc_malloc(&alloc, 100);
    void* ptr2 = anv_alloc_malloc(&alloc, 200);
    void* ptr3 = anv_alloc_malloc(&alloc, 300);

    ASSERT(ptr1 != NULL);
    ASSERT(ptr2 != NULL);
    ASSERT(ptr3 != NULL);
    ASSERT_EQ(allocation_count, 3);
    ASSERT_EQ(total_allocated, 600);
    ASSERT_EQ(peak_allocated, 600);

    // Free one allocation
    anv_alloc_free(&alloc, ptr2);
    ASSERT_EQ(allocation_count, 2);
    ASSERT_EQ(total_allocated, 400);
    ASSERT_EQ(peak_allocated, 600); // Peak should remain

    // Test copy function (should create new allocation)
    const int value = 42;
    int* copied = anv_alloc_copy(&alloc, &value);
    ASSERT(copied != NULL);
    ASSERT_EQ(*copied, 42);
    ASSERT_EQ(allocation_count, 3); // Should increase

    // Clean up
    anv_alloc_free(&alloc, ptr1);
    anv_alloc_free(&alloc, ptr3);
    anv_alloc_data_free(&alloc, copied);

    ASSERT_EQ(allocation_count, 0);
    ASSERT_EQ(total_allocated, 0);

    return TEST_SUCCESS;
}

int test_failing_allocator_error_handling(void)
{
    set_alloc_fail_countdown(2); // Allow 2 allocations, then fail

    const ANVAllocator alloc = anv_alloc_custom(failing_alloc, failing_free, failing_free, NULL);

    // First two allocations should succeed
    void* ptr1 = anv_alloc_malloc(&alloc, 100);
    void* ptr2 = anv_alloc_malloc(&alloc, 100);

    ASSERT(ptr1 != NULL);
    ASSERT(ptr2 != NULL);

    // Third allocation should fail
    const void* ptr3 = anv_alloc_malloc(&alloc, 100);
    ASSERT(ptr3 == NULL);

    // Fourth allocation should also fail
    const void* ptr4 = anv_alloc_malloc(&alloc, 100);
    ASSERT(ptr4 == NULL);

    // Clean up successful allocations
    anv_alloc_free(&alloc, ptr1);
    anv_alloc_free(&alloc, ptr2);

    return TEST_SUCCESS;
}

int test_allocator_with_linked_list(void)
{
    debug_reset();

    // Use debug allocator only for data, regular allocator for structure
    const ANVAllocator data_alloc = anv_alloc_custom(debug_alloc, debug_free, debug_free, debug_int_copy);
    ANVAllocator regular_alloc = anv_alloc_default();

    // Create linked list with regular allocator for structure
    ANVSinglyLinkedList* list = anv_sll_create(&regular_alloc);
    ASSERT(list != NULL);

    // Add several elements using debug allocator for data
    for (int i = 0; i < 5; i++)
    {
        int* value = anv_alloc_malloc(&data_alloc, sizeof(int));
        ASSERT(value != NULL);
        *value = i + 1;
        anv_sll_push_back(list, value);
    }

    ASSERT_EQ(anv_sll_size(list), 5);

    // Verify elements using iterator
    ANVIterator iter = anv_sll_iterator(list);
    int expected = 1;
    while (iter.has_next(&iter))
    {
        const int* value = iter.get(&iter);
        ASSERT_EQ(*value, expected++);
        iter.next(&iter);
    }
    iter.destroy(&iter);

    // Test find functionality
    const int search_value = 3;
    const ANVSinglyLinkedNode* found_node = anv_sll_find(list, &search_value, int_cmp);
    ASSERT(found_node != NULL);
    ASSERT_EQ(*(int*)found_node->data, 3);

    // Clean up manually: iterate through list and free each data element
    ANVIterator cleanup_iter = anv_sll_iterator(list);
    while (cleanup_iter.has_next(&cleanup_iter))
    {
        int* value = cleanup_iter.get(&cleanup_iter);
        anv_alloc_free(&data_alloc, value);
        cleanup_iter.next(&cleanup_iter);
    }
    cleanup_iter.destroy(&cleanup_iter);

    // Destroy list structure without auto-freeing data since we freed it manually
    anv_sll_destroy(list, false);

    // Check for memory leaks in our debug allocator
    ASSERT_EQ(allocation_count, 0); // Should be no leaks
    ASSERT_EQ(total_allocated, 0);

    return TEST_SUCCESS;
}

int test_allocator_stress_test(void)
{
    debug_reset();

    const ANVAllocator alloc = anv_alloc_custom(debug_alloc, debug_free, debug_free, string_copy);

    const int num_operations = 100;
    void* ptrs[100]; // Fixed size array instead of VLA
    int active_ptrs = 0;

    // Perform mixed allocation/deallocation operations
    for (int i = 0; i < num_operations; i++)
    {
        if (active_ptrs == 0 || (i % 3 != 0 && active_ptrs < num_operations / 2))
        {
            // Allocate
            const size_t size = 16 + (i % 64); // Variable sizes
            ptrs[active_ptrs] = anv_alloc_malloc(&alloc, size);
            ASSERT(ptrs[active_ptrs] != NULL);
            active_ptrs++;
        }
        else
        {
            // Free a random pointer
            const int index = i % active_ptrs;
            anv_alloc_free(&alloc, ptrs[index]);

            // Move last pointer to freed slot
            ptrs[index] = ptrs[active_ptrs - 1];
            active_ptrs--;
        }
    }

    // Free remaining allocations
    for (int i = 0; i < active_ptrs; i++)
    {
        anv_alloc_free(&alloc, ptrs[i]);
    }

    // Verify no memory leaks
    ASSERT_EQ(allocation_count, 0);
    ASSERT_EQ(total_allocated, 0);
    ASSERT(peak_allocated > 0); // Should have allocated something

    return TEST_SUCCESS;
}

int test_mixed_allocator_scenarios(void)
{
    // Test using different allocators for different purposes
    const ANVAllocator debug_alloc_struct = anv_alloc_custom(debug_alloc, debug_free, debug_free, NULL);
    const ANVAllocator pool_alloc_struct = anv_alloc_custom(pool_alloc, pool_free, NULL, NULL);

    debug_reset();
    pool_reset();

    // Use debug allocator for large allocations
    void* large_ptr = anv_alloc_malloc(&debug_alloc_struct, 1024);
    ASSERT(large_ptr != NULL);

    // Use pool allocator for small allocations
    void* small_ptr1 = anv_alloc_malloc(&pool_alloc_struct, 32);
    void* small_ptr2 = anv_alloc_malloc(&pool_alloc_struct, 16);

    ASSERT(small_ptr1 != NULL);
    ASSERT(small_ptr2 != NULL);

    // Verify tracking
    ASSERT_EQ(allocation_count, 1); // Only debug allocator tracked
    ASSERT_EQ(total_allocated, 1024);

    // Clean up
    anv_alloc_free(&debug_alloc_struct, large_ptr);
    anv_alloc_free(&pool_alloc_struct, small_ptr1);
    anv_alloc_free(&pool_alloc_struct, small_ptr2);

    ASSERT_EQ(allocation_count, 0);
    ASSERT_EQ(total_allocated, 0);

    return TEST_SUCCESS;
}

int test_allocator_anv_copy_function_variants(void)
{
    // Test different copy function behaviors
    const ANVAllocator shallow_alloc = anv_alloc_custom(malloc, free, free, NULL);
    const ANVAllocator deep_alloc = anv_alloc_custom(malloc, free, free, int_copy);

    int original = 42;

    // Shallow copy should use default copy (return same pointer)
    const void* shallow_copy = anv_alloc_copy(&shallow_alloc, &original);
    ASSERT(shallow_copy == &original); // Default copy returns original pointer

    // Deep copy should create new allocation
    int* deep_copy = anv_alloc_copy(&deep_alloc, &original);
    ASSERT(deep_copy != NULL);
    ASSERT(deep_copy != &original);
    ASSERT_EQ(*deep_copy, 42);

    // Modify original - deep copy should be unaffected
    original = 100;
    ASSERT_EQ(*deep_copy, 42);

    anv_alloc_data_free(&deep_alloc, deep_copy);

    return TEST_SUCCESS;
}

//==============================================================================
// Main test runner
//==============================================================================

int main(void)
{
    int tests_passed = 0;
    int tests_total = 0;

    printf("Running advanced allocator integration tests...\n\n");

    const struct
    {
        const char* name;
        int (*test_func)(void);
    } tests[] = {
            {"Pool Allocator Integration", test_pool_allocator_integration},
            {"Debug Allocator Tracking", test_debug_allocator_tracking},
            {"Failing Allocator Error Handling", test_failing_allocator_error_handling},
            {"Allocator with Linked List", test_allocator_with_linked_list},
            {"Allocator Stress Test", test_allocator_stress_test},
            {"Mixed Allocator Scenarios", test_mixed_allocator_scenarios},
            {"Copy Function Variants", test_allocator_anv_copy_function_variants}
        };

    const int num_tests = sizeof(tests) / sizeof(tests[0]);

    for (int i = 0; i < num_tests; i++)
    {
        printf("Test %d: %s... ", i + 1, tests[i].name);
        fflush(stdout);

        const int result = tests[i].test_func();
        tests_total++;

        if (result == TEST_SUCCESS)
        {
            printf("PASSED\n");
            tests_passed++;
        }
        else if (result == TEST_FAILURE)
        {
            printf("FAILED\n");
        }
        else
        {
            printf("SKIPPED\n");
        }
    }

    printf("\n=== Advanced Test Results ===\n");
    printf("Tests passed: %d/%d\n", tests_passed, tests_total);
    printf("Success rate: %.1f%%\n",
           tests_total > 0 ? (100.0 * tests_passed / tests_total) : 0.0);

    // Print final debug report
    debug_report();

    return (tests_passed == tests_total) ? 0 : 1;
}