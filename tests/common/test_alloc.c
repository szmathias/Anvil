#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include <anvil/testing.h>
#include "TestHelpers.h"
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
// Debug Allocator Implementation
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

//==============================================================================
// Arena Allocator Implementation
//==============================================================================

typedef struct
{
    char* memory;
    size_t size;
    size_t used;
} Arena;

static Arena arena_global = {0};

static void* arena_alloc(const size_t size)
{
    if (!arena_global.memory)
    {
        return NULL;
    }

    // Align to 8-byte boundary
    const size_t aligned_size = (size + 7) & ~7;

    if (arena_global.used + aligned_size > arena_global.size)
    {
        return NULL; // Out of memory
    }

    void* ptr = arena_global.memory + arena_global.used;
    arena_global.used += aligned_size;
    return ptr;
}

static void arena_free(void* ptr)
{
    // Arena allocator doesn't free individual allocations
    (void)ptr;
}

static void arena_reset(void)
{
    arena_global.used = 0;
}

static int arena_init(size_t size)
{
    arena_global.memory = malloc(size);
    if (!arena_global.memory)
    {
        return -1;
    }
    arena_global.size = size;
    arena_global.used = 0;
    return 0;
}

static void arena_destroy(void)
{
    free(arena_global.memory);
    arena_global.memory = NULL;
    arena_global.size = 0;
    arena_global.used = 0;
}

//==============================================================================
// Static Stack Allocator Implementation
//==============================================================================

#define STACK_SIZE 4096
static char stack_memory[STACK_SIZE];
static size_t stack_top = 0;

static void* stack_alloc(const size_t size)
{
    // Align to 8-byte boundary
    const size_t aligned_size = (size + 7) & ~7;

    if (stack_top + aligned_size > STACK_SIZE)
    {
        return NULL; // Stack overflow
    }

    void* ptr = stack_memory + stack_top;
    stack_top += aligned_size;
    return ptr;
}

static void stack_free(void* ptr)
{
    // Simple stack allocator only supports LIFO deallocation
    if (!ptr)
        return;

    const char* char_ptr = ptr;
    if (char_ptr >= stack_memory && char_ptr < stack_memory + STACK_SIZE)
    {
        // Only free if it's the top allocation
        if (char_ptr < stack_memory + stack_top)
        {
            stack_top = char_ptr - stack_memory;
        }
    }
}

static void stack_reset(void)
{
    stack_top = 0;
}

//==============================================================================
// Counting Allocator Implementation
//==============================================================================

static size_t alloc_count = 0;
static size_t free_count = 0;

static void* counting_alloc(const size_t size)
{
    alloc_count++;
    return malloc(size);
}

static void counting_free(void* ptr)
{
    if (ptr)
    {
        free_count++;
        free(ptr);
    }
}

static void reset_counters(void)
{
    alloc_count = 0;
    free_count = 0;
}

static void* counting_int_copy(const void* data)
{
    const int* original = data;
    int* copy = counting_alloc(sizeof(int));
    if (copy)
    {
        *copy = *original;
    }
    return copy;
}

//==============================================================================
// Advanced Allocator Tests
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
        int* value = anv_alloc_allocate(&pool_data_alloc, sizeof(int));
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
        void* ptr = anv_alloc_allocate(&pool_data_alloc, 32);
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
        anv_alloc_deallocate(&pool_data_alloc, ptrs[i]);
    }

    // Clean up: manually free data with pool allocator, then destroy list
    for (size_t i = 0; i < anv_arraylist_size(list); i++)
    {
        int* value = anv_arraylist_get(list, i);
        anv_alloc_deallocate(&pool_data_alloc, value);
    }
    anv_arraylist_destroy(list, false); // Don't auto-free data since we freed it manually

    return TEST_SUCCESS;
}

int test_debug_allocator_tracking(void)
{
    debug_reset();

    const ANVAllocator alloc = anv_alloc_custom(debug_alloc, debug_free, debug_free, debug_int_copy);

    // Test allocation tracking
    void* ptr1 = anv_alloc_allocate(&alloc, 100);
    void* ptr2 = anv_alloc_allocate(&alloc, 200);
    void* ptr3 = anv_alloc_allocate(&alloc, 300);

    ASSERT(ptr1 != NULL);
    ASSERT(ptr2 != NULL);
    ASSERT(ptr3 != NULL);
    ASSERT_EQ(allocation_count, 3);
    ASSERT_EQ(total_allocated, 600);
    ASSERT_EQ(peak_allocated, 600);

    // Free one allocation
    anv_alloc_deallocate(&alloc, ptr2);
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
    anv_alloc_deallocate(&alloc, ptr1);
    anv_alloc_deallocate(&alloc, ptr3);
    anv_alloc_data_deallocate(&alloc, copied);

    ASSERT_EQ(allocation_count, 0);
    ASSERT_EQ(total_allocated, 0);

    return TEST_SUCCESS;
}

int test_failing_allocator_error_handling(void)
{
    set_alloc_fail_countdown(2); // Allow 2 allocations, then fail

    const ANVAllocator alloc = anv_alloc_custom(failing_alloc, failing_free, failing_free, NULL);

    // First two allocations should succeed
    void* ptr1 = anv_alloc_allocate(&alloc, 100);
    void* ptr2 = anv_alloc_allocate(&alloc, 100);

    ASSERT(ptr1 != NULL);
    ASSERT(ptr2 != NULL);

    // Third allocation should fail
    const void* ptr3 = anv_alloc_allocate(&alloc, 100);
    ASSERT(ptr3 == NULL);

    // Fourth allocation should also fail
    const void* ptr4 = anv_alloc_allocate(&alloc, 100);
    ASSERT(ptr4 == NULL);

    // Clean up successful allocations
    anv_alloc_deallocate(&alloc, ptr1);
    anv_alloc_deallocate(&alloc, ptr2);

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
        int* value = anv_alloc_allocate(&data_alloc, sizeof(int));
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
        anv_alloc_deallocate(&data_alloc, value);
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
            ptrs[active_ptrs] = anv_alloc_allocate(&alloc, size);
            ASSERT(ptrs[active_ptrs] != NULL);
            active_ptrs++;
        }
        else
        {
            // Free a random pointer
            const int index = i % active_ptrs;
            anv_alloc_deallocate(&alloc, ptrs[index]);

            // Move last pointer to freed slot
            ptrs[index] = ptrs[active_ptrs - 1];
            active_ptrs--;
        }
    }

    // Free remaining allocations
    for (int i = 0; i < active_ptrs; i++)
    {
        anv_alloc_deallocate(&alloc, ptrs[i]);
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
    void* large_ptr = anv_alloc_allocate(&debug_alloc_struct, 1024);
    ASSERT(large_ptr != NULL);

    // Use pool allocator for small allocations
    void* small_ptr1 = anv_alloc_allocate(&pool_alloc_struct, 32);
    void* small_ptr2 = anv_alloc_allocate(&pool_alloc_struct, 16);

    ASSERT(small_ptr1 != NULL);
    ASSERT(small_ptr2 != NULL);

    // Verify tracking
    ASSERT_EQ(allocation_count, 1); // Only debug allocator tracked
    ASSERT_EQ(total_allocated, 1024);

    // Clean up
    anv_alloc_deallocate(&debug_alloc_struct, large_ptr);
    anv_alloc_deallocate(&pool_alloc_struct, small_ptr1);
    anv_alloc_deallocate(&pool_alloc_struct, small_ptr2);

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

    anv_alloc_data_deallocate(&deep_alloc, deep_copy);

    return TEST_SUCCESS;
}

//==============================================================================
// Custom Allocator Tests
//==============================================================================

int test_default_allocator(void)
{
    const ANVAllocator alloc = anv_alloc_default();

    // Test allocation
    void* ptr = anv_alloc_allocate(&alloc, 100);
    ASSERT(ptr != NULL);

    // Test copy (should return same pointer for default)
    const int value = 42;
    const void* copied = anv_alloc_copy(&alloc, &value);
    ASSERT(copied == &value);

    // Test free
    anv_alloc_deallocate(&alloc, ptr);

    return TEST_SUCCESS;
}

int test_arena_allocator(void)
{
    // Initialize arena
    ASSERT_EQ(arena_init(1024), 0);

    const ANVAllocator alloc = anv_alloc_custom(arena_alloc, arena_free, NULL, NULL);

    // Test multiple allocations
    const void* ptr1 = anv_alloc_allocate(&alloc, 64);
    ASSERT(ptr1 != NULL);

    const void* ptr2 = anv_alloc_allocate(&alloc, 128);
    ASSERT(ptr2 != NULL);

    const void* ptr3 = anv_alloc_allocate(&alloc, 256);
    ASSERT(ptr3 != NULL);

    // Verify pointers are different and ordered
    ASSERT(ptr1 != ptr2);
    ASSERT(ptr2 != ptr3);
    ASSERT(ptr1 < ptr2);
    ASSERT(ptr2 < ptr3);

    // Test allocation failure when out of memory
    const void* big_ptr = anv_alloc_allocate(&alloc, 1024);
    ASSERT(big_ptr == NULL);

    // Test reset and reuse
    arena_reset();
    const void* ptr4 = anv_alloc_allocate(&alloc, 64);
    ASSERT(ptr4 == ptr1); // Should reuse same memory

    arena_destroy();
    return TEST_SUCCESS;
}

int test_stack_allocator(void)
{
    stack_reset();

    const ANVAllocator alloc = anv_alloc_custom(stack_alloc, stack_free, NULL, NULL);

    // Test allocation
    const void* ptr1 = anv_alloc_allocate(&alloc, 64);
    ASSERT(ptr1 != NULL);

    void* ptr2 = anv_alloc_allocate(&alloc, 128);
    ASSERT(ptr2 != NULL);

    // Test LIFO deallocation
    anv_alloc_deallocate(&alloc, ptr2);
    const void* ptr3 = anv_alloc_allocate(&alloc, 100);
    ASSERT(ptr3 == ptr2); // Should reuse freed space

    // Test stack overflow
    const void* big_ptr = anv_alloc_allocate(&alloc, STACK_SIZE);
    ASSERT(big_ptr == NULL);

    stack_reset();
    return TEST_SUCCESS;
}

int test_counting_allocator(void)
{
    reset_counters();

    const ANVAllocator alloc = anv_alloc_custom(counting_alloc, counting_free, counting_free, counting_int_copy);

    // Test allocations are counted
    void* ptr1 = anv_alloc_allocate(&alloc, 64);
    ASSERT(ptr1 != NULL);
    ASSERT_EQ(alloc_count, 1);
    ASSERT_EQ(free_count, 0);

    void* ptr2 = anv_alloc_allocate(&alloc, 128);
    ASSERT(ptr2 != NULL);
    ASSERT_EQ(alloc_count, 2);
    ASSERT_EQ(free_count, 0);

    // Test copy function (should allocate new memory)
    const int value = 42;
    void* copied = anv_alloc_copy(&alloc, &value);
    ASSERT(copied != NULL);
    ASSERT(copied != &value);
    ASSERT_EQ(*(int*)copied, 42);
    ASSERT_EQ(alloc_count, 3); // Copy should trigger allocation

    // Test frees are counted
    anv_alloc_deallocate(&alloc, ptr1);
    ASSERT_EQ(free_count, 1);

    anv_alloc_deallocate(&alloc, ptr2);
    ASSERT_EQ(free_count, 2);

    anv_alloc_data_deallocate(&alloc, copied);
    ASSERT_EQ(free_count, 3);

    return TEST_SUCCESS;
}

int test_custom_anv_copy_functions(void)
{
    const ANVAllocator int_alloc = anv_alloc_custom(malloc, free, free, int_copy);
    const ANVAllocator str_alloc = anv_alloc_custom(malloc, free, free, string_copy);

    // Test integer deep copy
    const int original_int = 123;
    int* copied_int = anv_alloc_copy(&int_alloc, &original_int);
    ASSERT(copied_int != NULL);
    ASSERT(copied_int != &original_int);
    ASSERT_EQ(*copied_int, 123);
    anv_alloc_data_deallocate(&int_alloc, copied_int);

    // Test string deep copy
    const char* original_str = "Hello, World!";
    char* copied_str = anv_alloc_copy(&str_alloc, original_str);
    ASSERT(copied_str != NULL);
    ASSERT(copied_str != original_str);
    ASSERT_EQ_STR(copied_str, "Hello, World!");
    anv_alloc_data_deallocate(&str_alloc, copied_str);

    return TEST_SUCCESS;
}

int test_allocator_edge_cases(void)
{
    const ANVAllocator alloc = anv_alloc_default();

    // Test NULL pointer handling
    anv_alloc_deallocate(&alloc, NULL); // Should not crash

    const void* null_copy = anv_alloc_copy(&alloc, NULL);
    ASSERT(null_copy == NULL);

    // Test zero-size allocation
    void* zero_ptr = anv_alloc_allocate(&alloc, 0);
    // Behavior is implementation-defined, but shouldn't crash
    anv_alloc_deallocate(&alloc, zero_ptr);

    // Test NULL allocator
    const void* null_alloc_ptr = anv_alloc_allocate(NULL, 100);
    ASSERT(null_alloc_ptr == NULL);

    return TEST_SUCCESS;
}

int test_allocator_with_null_functions(void)
{
    // Test allocator with NULL copy and data_free functions
    const ANVAllocator alloc = anv_alloc_custom(malloc, free, NULL, NULL);

    void* ptr = anv_alloc_allocate(&alloc, 64);
    ASSERT(ptr != NULL);

    // data_free with NULL function should not crash
    anv_alloc_data_deallocate(&alloc, ptr);

    // copy with NULL function should use default copy (return same pointer)
    const int value = 42;
    const void* copied = anv_alloc_copy(&alloc, &value);
    ASSERT(copied == &value); // Default copy returns original pointer

    anv_alloc_deallocate(&alloc, ptr);

    return TEST_SUCCESS;
}

int test_arena_memory_alignment(void)
{
    ASSERT_EQ(arena_init(1024), 0);

    const ANVAllocator alloc = anv_alloc_custom(arena_alloc, arena_free, NULL, NULL);

    // Test that allocations are properly aligned
    void* ptr1 = anv_alloc_allocate(&alloc, 1);
    void* ptr2 = anv_alloc_allocate(&alloc, 1);

    ASSERT(ptr1 != NULL);
    ASSERT(ptr2 != NULL);

    // Check 8-byte alignment
    const uintptr_t addr1 = (uintptr_t)ptr1;
    const uintptr_t addr2 = (uintptr_t)ptr2;

    ASSERT_EQ(addr1 % 8, 0);
    ASSERT_EQ(addr2 % 8, 0);
    ASSERT_EQ(addr2 - addr1, 8); // Should be 8 bytes apart for 1-byte allocations

    arena_destroy();
    return TEST_SUCCESS;
}

int test_stack_allocator_lifo_behavior(void)
{
    stack_reset();

    const ANVAllocator alloc = anv_alloc_custom(stack_alloc, stack_free, NULL, NULL);

    // Allocate in order
    const void* ptr1 = anv_alloc_allocate(&alloc, 64);
    void* ptr2 = anv_alloc_allocate(&alloc, 64);
    void* ptr3 = anv_alloc_allocate(&alloc, 64);

    ASSERT(ptr1 != NULL);
    ASSERT(ptr2 != NULL);
    ASSERT(ptr3 != NULL);

    // Free in LIFO order (last allocated first)
    anv_alloc_deallocate(&alloc, ptr3);

    // Next allocation should reuse ptr3's space
    void* ptr4 = anv_alloc_allocate(&alloc, 64);
    ASSERT(ptr4 == ptr3);

    // Free ptr4 and ptr2 (not in LIFO order for ptr2)
    anv_alloc_deallocate(&alloc, ptr4);
    anv_alloc_deallocate(&alloc, ptr2);

    // New allocation should still work
    const void* ptr5 = anv_alloc_allocate(&alloc, 64);
    ASSERT(ptr5 == ptr2);

    stack_reset();
    return TEST_SUCCESS;
}

//==============================================================================
// Main
//==============================================================================

int main(void)
{
    const ANVTestCase tests[] = {
        // Allocator Tests
        TEST_REGISTER(test_pool_allocator_integration),
        TEST_REGISTER(test_debug_allocator_tracking),
        TEST_REGISTER(test_failing_allocator_error_handling),
        TEST_REGISTER(test_allocator_with_linked_list),
        TEST_REGISTER(test_allocator_stress_test),
        TEST_REGISTER(test_mixed_allocator_scenarios),
        TEST_REGISTER(test_allocator_anv_copy_function_variants),

        // Custom Allocator Tests
        TEST_REGISTER(test_default_allocator),
        TEST_REGISTER(test_arena_allocator),
        TEST_REGISTER(test_stack_allocator),
        TEST_REGISTER(test_counting_allocator),
        TEST_REGISTER(test_custom_anv_copy_functions),
        TEST_REGISTER(test_allocator_edge_cases),
        TEST_REGISTER(test_allocator_with_null_functions),
        TEST_REGISTER(test_arena_memory_alignment),
        TEST_REGISTER(test_stack_allocator_lifo_behavior),
    };

    return anv_run_tests("Alloc", tests, sizeof(tests) / sizeof(tests[0]));
}