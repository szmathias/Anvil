//
// Created by zack on 8/28/25.
//
// Comprehensive test suite for range iterator functionality.
// Tests cover basic iteration, edge cases, bidirectional movement with zigzag compensation,
// error handling, and API behavior separation.

#include "containers/iterator.h"
#include "TestAssert.h"
#include "TestHelpers.h"

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

//==============================================================================
// Helper Functions for Range Iterator Tests
//==============================================================================

/**
 * Helper function to collect all values from an iterator into an array.
 * Returns the number of values collected.
 */
static int collect_values(const ANVIterator* it, int* values, const int max_count)
{
    int count = 0;
    while (it->has_next(it) && count < max_count)
    {
        const int* value = it->get(it);
        if (value)
        {
            values[count++] = *value;
        }
        it->next(it);
    }
    return count;
}

/**
 * Helper function to verify an array matches expected values.
 */
static int verify_values(const int* actual, const int* expected, const int count, const char* test_name)
{
    for (int i = 0; i < count; i++)
    {
        if (actual[i] != expected[i])
        {
            printf("FAIL: %s - Expected %d at index %d, got %d\n",
                   test_name, expected[i], i, actual[i]);
            return 0;
        }
    }
    return 1;
}

//==============================================================================
// Basic Range Iterator Tests (Should Pass Without Changes)
//==============================================================================

/**
 * Test basic forward iteration with step 1.
 */
static int test_range_positive_step(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(0, 5, 1, &alloc);

    ASSERT_TRUE(it.is_valid(&it));

    const int expected[] = {0, 1, 2, 3, 4};
    int actual[5];
    const int count = collect_values(&it, actual, 5);

    ASSERT_EQ(count, 5);
    ASSERT_TRUE(verify_values(actual, expected, count, "basic_forward"));

    // Iterator should be exhausted
    ASSERT_FALSE(it.has_next(&it));
    ASSERT_NULL(it.get(&it));

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test basic backward iteration with negative step.
 */
static int test_range_negative_step(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(10, 5, -1, &alloc);

    ASSERT_TRUE(it.is_valid(&it));

    const int expected[] = {10, 9, 8, 7, 6};
    int actual[5];
    const int count = collect_values(&it, actual, 5);

    ASSERT_EQ(count, 5);
    ASSERT_TRUE(verify_values(actual, expected, count, "basic_backward"));

    // Iterator should be exhausted
    ASSERT_FALSE(it.has_next(&it));
    ASSERT_NULL(it.get(&it));

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test iteration with larger positive step.
 */
static int test_range_larger_step(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(2, 15, 3, &alloc);

    ASSERT_TRUE(it.is_valid(&it));

    const int expected[] = {2, 5, 8, 11, 14};
    int actual[5];
    const int count = collect_values(&it, actual, 5);

    ASSERT_EQ(count, 5);
    ASSERT_TRUE(verify_values(actual, expected, count, "positive_step"));

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test iteration with larger negative step.
 */
static int test_range_negative_step_size(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(20, 5, -4, &alloc);

    ASSERT_TRUE(it.is_valid(&it));

    const int expected[] = {20, 16, 12, 8};
    int actual[4];
    const int count = collect_values(&it, actual, 4);

    ASSERT_EQ(count, 4);
    ASSERT_TRUE(verify_values(actual, expected, count, "negative_step"));

    it.destroy(&it);
    return TEST_SUCCESS;
}

//==============================================================================
// Edge Case Tests
//==============================================================================

/**
 * Test empty range where start equals end.
 */
static int test_range_empty(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(5, 5, 1, &alloc);

    ASSERT_TRUE(it.is_valid(&it));
    ASSERT_FALSE(it.has_next(&it));
    ASSERT_NULL(it.get(&it));

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test single element range.
 */
static int test_single_element_range(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(7, 8, 1, &alloc);

    ASSERT_TRUE(it.is_valid(&it));
    ASSERT_TRUE(it.has_next(&it));

    const int* value = it.get(&it);
    ASSERT_NOT_NULL(value);
    ASSERT_EQ(*value, 7);

    it.next(&it);
    ASSERT_FALSE(it.has_next(&it));
    ASSERT_NULL(it.get(&it));

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test range with extreme integer values.
 */
static int test_range_extreme_values(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Test near INT_MAX
    ANVIterator it1 = anv_iterator_range(INT_MAX - 3, INT_MAX, 1, &alloc);
    ASSERT_TRUE(it1.is_valid(&it1));

    const int expected_max[] = {INT_MAX - 3, INT_MAX - 2, INT_MAX - 1};
    int actual_max[3];
    const int count1 = collect_values(&it1, actual_max, 3);

    ASSERT_EQ(count1, 3);
    ASSERT_TRUE(verify_values(actual_max, expected_max, count1, "extreme_max"));
    it1.destroy(&it1);

    // Test near INT_MIN
    ANVIterator it2 = anv_iterator_range(INT_MIN + 3, INT_MIN, -1, &alloc);
    ASSERT_TRUE(it2.is_valid(&it2));

    const int expected_min[] = {INT_MIN + 3, INT_MIN + 2, INT_MIN + 1};
    int actual_min[3];
    const int count2 = collect_values(&it2, actual_min, 3);

    ASSERT_EQ(count2, 3);
    ASSERT_TRUE(verify_values(actual_min, expected_min, count2, "extreme_min"));
    it2.destroy(&it2);

    return TEST_SUCCESS;
}

//==============================================================================
// Error Handling Tests
//==============================================================================

/**
 * Test invalid step values.
 */
static int test_range_invalid_step(void)
{
    const ANVAllocator alloc = create_int_allocator();

    // Test zero step
    ANVIterator it1 = anv_iterator_range(0, 5, 0, &alloc);
    ASSERT_FALSE(it1.is_valid(&it1));
    ASSERT_FALSE(it1.has_next(&it1));
    ASSERT_NULL(it1.get(&it1));
    it1.destroy(&it1);

    // Test conflicting direction: start < end with negative step
    ANVIterator it2 = anv_iterator_range(0, 10, -1, &alloc);
    ASSERT_FALSE(it2.is_valid(&it2));
    ASSERT_FALSE(it2.has_next(&it2));
    ASSERT_NULL(it2.get(&it2));
    it2.destroy(&it2);

    // Test conflicting direction: start > end with positive step
    ANVIterator it3 = anv_iterator_range(10, 0, 1, &alloc);
    ASSERT_FALSE(it3.is_valid(&it3));
    ASSERT_FALSE(it3.has_next(&it3));
    ASSERT_NULL(it3.get(&it3));
    it3.destroy(&it3);

    return TEST_SUCCESS;
}

/**
 * Test invalid allocator.
 */
static int test_invalid_allocator(void)
{
    ANVIterator it = anv_iterator_range(0, 5, 1, NULL);
    ASSERT_FALSE(it.is_valid(&it));
    ASSERT_FALSE(it.has_next(&it));
    ASSERT_NULL(it.get(&it));
    it.destroy(&it);

    return TEST_SUCCESS;
}

//==============================================================================
// Performance and Stress Tests
//==============================================================================

/**
 * Test large range iteration performance.
 */
static int test_range_stress(void)
{
    const ANVAllocator alloc = create_int_allocator();
    const int SIZE = 10000;

    ANVIterator it = anv_iterator_range(0, SIZE, 1, &alloc);
    ASSERT_TRUE(it.is_valid(&it));

    int count = 0;
    int expected = 0;

    while (it.has_next(&it))
    {
        const int* value = it.get(&it);
        ASSERT_NOT_NULL(value);
        ASSERT_EQ(*value, expected);

        count++;
        expected++;
        it.next(&it);
    }

    ASSERT_EQ(count, SIZE);
    ASSERT_FALSE(it.has_next(&it));

    it.destroy(&it);
    return TEST_SUCCESS;
}

//==============================================================================
// Reset Functionality Tests (Rewritten for Correct Behavior)
//==============================================================================

/**
 * Test reset functionality.
 */
static int test_range_reset(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(0, 10, 2, &alloc);

    ASSERT_TRUE(it.is_valid(&it));

    // Move forward a few steps
    const int* v1 = it.get(&it);
    ASSERT_EQ(*v1, 0); // Should be 0
    it.next(&it);      // Move to 2

    const int* v2 = it.get(&it);
    ASSERT_EQ(*v2, 2); // Should be 2
    it.next(&it);      // Move to 4

    const int* v3 = it.get(&it);
    ASSERT_EQ(*v3, 4); // Should be 4

    // Reset and verify we're back at start
    it.reset(&it);

    const int* reset_value = it.get(&it);
    ASSERT_EQ(*reset_value, 0); // Should be back to start

    // Verify normal iteration continues from start
    it.next(&it);
    const int* next_value = it.get(&it);
    ASSERT_EQ(*next_value, 2); // Should be first step from start

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test reset after bidirectional movement.
 */
static int test_reset_after_bidirectional(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(10, 20, 3, &alloc);

    ASSERT_TRUE(it.is_valid(&it));

    // Forward and backward movement
    it.next(&it); // 10 -> 13
    it.next(&it); // 13 -> 16
    it.prev(&it); // 16 -> 13
    it.prev(&it); // 13 -> 10
    it.next(&it); // 10 -> 13

    const int* before_reset = it.get(&it);
    ASSERT_EQ(*before_reset, 13);

    // Reset should bring us back to start
    it.reset(&it);

    const int* after_reset = it.get(&it);
    ASSERT_EQ(*after_reset, 10); // This should always work - reset to start

    it.destroy(&it);
    return TEST_SUCCESS;
}

//==============================================================================
// Zigzag Tests
//==============================================================================

/**
 * Test the core zigzalogic.
 */
static int test_zigzag_compensation(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(0, 10, 1, &alloc);

    ASSERT_TRUE(it.is_valid(&it));

    // Forward 3 steps: 0 → 1 → 2 → 3
    const int* v1 = it.get(&it);
    ASSERT_EQ(*v1, 0);
    it.next(&it);

    const int* v2 = it.get(&it);
    ASSERT_EQ(*v2, 1);
    it.next(&it);

    const int* v3 = it.get(&it);
    ASSERT_EQ(*v3, 2);
    it.next(&it);

    const int* v4 = it.get(&it);
    ASSERT_EQ(*v4, 3);

    // Move back 1: should go to 2
    ASSERT_TRUE(it.has_prev(&it));
    ASSERT_EQ(it.prev(&it), 0);

    const int* p1 = it.get(&it);
    ASSERT_EQ(*p1, 2); // Simple: 3 → 2

    // Move forward again: should go to 3
    it.next(&it);
    const int* f1 = it.get(&it);
    ASSERT_EQ(*f1, 3); // 2 → 3

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test bidirectional boundaries
 */
static int test_bidirectional_boundaries(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(0, 5, 1, &alloc);

    ASSERT_TRUE(it.is_valid(&it));

    // Move forward to near boundary: 0 → 1 → 2 → 3 → 4
    it.next(&it); // 0 -> 1
    it.next(&it); // 1 -> 2
    it.next(&it); // 2 -> 3
    it.next(&it); // 3 -> 4

    const int* at_end = it.get(&it);
    ASSERT_EQ(*at_end, 4); // Should be at last valid element

    // Test backward from boundary: 4 → 3
    ASSERT_TRUE(it.has_prev(&it));
    ASSERT_EQ(it.prev(&it), 0);

    const int* back_one = it.get(&it);
    ASSERT_EQ(*back_one, 3); // Should be at previous element (not 2!)

    // Test forward again from this position: 3 → 4
    it.next(&it);
    const int* forward_again = it.get(&it);
    ASSERT_EQ(*forward_again, 4); // Should move forward properly

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test direction change
 */
static int test_direction_change_compensation(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(10, 20, 1, &alloc);

    ASSERT_TRUE(it.is_valid(&it));

    // Start at 10, move forward to 12: 10 → 11 → 12
    const int* start = it.get(&it);
    ASSERT_EQ(*start, 10);
    it.next(&it); // -> 11

    const int* pos1 = it.get(&it);
    ASSERT_EQ(*pos1, 11);
    it.next(&it); // -> 12

    const int* pos2 = it.get(&it);
    ASSERT_EQ(*pos2, 12);

    // Change direction: 12 → 11
    it.prev(&it);
    const int* back_pos = it.get(&it);
    ASSERT_EQ(*back_pos, 11); // Should be 11, not 10

    // Change direction again: 11 → 12
    it.next(&it);
    const int* forward_pos = it.get(&it);
    ASSERT_EQ(*forward_pos, 12); // Should be 12, not 13

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test that we handle start boundary correctly.
 */
static int test_start_boundary_behavior(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(0, 5, 1, &alloc);

    ASSERT_TRUE(it.is_valid(&it));

    // Move forward then back to start
    it.next(&it); // 0 -> 1
    it.next(&it); // 1 -> 2
    it.prev(&it); // 2 -> 1 (with compensation)
    it.prev(&it); // 1 -> 0 (with compensation)

    const int* at_start = it.get(&it);
    ASSERT_EQ(*at_start, 0);

    // Should be at start, no more backward movement
    ASSERT_FALSE(it.has_prev(&it));
    ASSERT_NOT_EQ(it.prev(&it), 0); // Should fail (return -1)

    it.destroy(&it);
    return TEST_SUCCESS;
}

//==============================================================================
// New API-Specific Tests
//==============================================================================

/**
 * Test that get() doesn't advance, next() does advance.
 * Fixed: Store values before pointer changes.
 */
static int test_get_next_separation(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(5, 10, 1, &alloc);

    ASSERT_TRUE(it.is_valid(&it));

    const int* v1 = it.get(&it);
    const int* v2 = it.get(&it);
    ASSERT_EQ(*v1, *v2); // get() shouldn't advance
    ASSERT_EQ(*v1, 5);   // Should be start value

    // Store value before advancing to avoid pointer reuse issues
    const int initial_value = *v1;

    it.next(&it); // This should advance
    const int* v3 = it.get(&it);
    const int new_value = *v3;

    ASSERT_NOT_EQ(initial_value, new_value); // Position should have changed
    ASSERT_EQ(new_value, 6);                 // Should be next value

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test next() return codes.
 */
static int test_next_return_codes(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(0, 2, 1, &alloc);

    ASSERT_TRUE(it.is_valid(&it));

    ASSERT_EQ(it.next(&it), 0);  // Should succeed
    ASSERT_EQ(it.next(&it), 0);  // Should succeed
    ASSERT_EQ(it.next(&it), -1); // Should fail (at end)

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test prev() return codes.
 * Fixed: Based on actual boundary behavior from diagnostic.
 */
static int test_prev_return_codes(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(0, 3, 1, &alloc);

    ASSERT_TRUE(it.is_valid(&it));

    // Move forward to position 2: 0 → 1 → 2
    it.next(&it); // 0 -> 1
    it.next(&it); // 1 -> 2

    // Now test backward movement: 2 → 1 → 0 → fail
    ASSERT_EQ(it.prev(&it), 0); // Should succeed: 2 -> 1
    ASSERT_EQ(it.prev(&it), 0); // Should succeed: 1 -> 0

    // At this point we're at start (0), so next prev should fail
    ASSERT_EQ(it.prev(&it), -1); // Should fail (at start)

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test memory consistency across operations.
 */
static int test_memory_consistency(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(100, 105, 1, &alloc);

    ASSERT_TRUE(it.is_valid(&it));

    // Get multiple references to the same value
    const int* ptr1 = it.get(&it);
    const int* ptr2 = it.get(&it);
    const int* ptr3 = it.get(&it);

    // All should point to the same memory and have the same value
    ASSERT_EQ(ptr1, ptr2);
    ASSERT_EQ(ptr2, ptr3);
    ASSERT_EQ(*ptr1, 100);
    ASSERT_EQ(*ptr2, 100);
    ASSERT_EQ(*ptr3, 100);

    // Move to next and verify new value
    it.next(&it);
    const int* ptr4 = it.get(&it);
    ASSERT_EQ(*ptr4, 101);

    // Previous pointers should now point to updated value (same memory)
    ASSERT_EQ(ptr1, ptr4);
    ASSERT_EQ(*ptr1, 101);

    it.destroy(&it);
    return TEST_SUCCESS;
}

//==============================================================================
// Additional Test Cases for Range Iterator
// These tests address potential flaws and missing edge cases
//==============================================================================

/**
 * Test has_prev() behavior at start position.
 */
static int test_has_prev_at_start(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(10, 15, 1, &alloc);

    ASSERT_TRUE(it.is_valid(&it));

    // Should be false at start position
    ASSERT_FALSE(it.has_prev(&it));

    // Move forward once
    ASSERT_EQ(it.next(&it), 0);
    ASSERT_TRUE(it.has_prev(&it));

    // Move back to start
    ASSERT_EQ(it.prev(&it), 0);
    ASSERT_FALSE(it.has_prev(&it));

    // Verify we're actually at start
    const int* value = it.get(&it);
    ASSERT_EQ(*value, 10);

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test has_next() behavior at end position.
 */
static int test_has_next_at_end(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(0, 3, 1, &alloc);

    ASSERT_TRUE(it.is_valid(&it));

    // Advance to end: 0->1->2->3 (but 3 is outside range [0,3))
    ASSERT_EQ(it.next(&it), 0); // 0->1
    ASSERT_TRUE(it.has_next(&it));

    ASSERT_EQ(it.next(&it), 0); // 1->2
    ASSERT_TRUE(it.has_next(&it));

    ASSERT_EQ(it.next(&it), 0); // 2->3 (3 is outside range)
    ASSERT_FALSE(it.has_next(&it));
    ASSERT_NULL(it.get(&it));

    // Further advance should fail
    ASSERT_EQ(it.next(&it), -1);

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test bidirectional movement with different step sizes.
 */
static int test_bidirectional_with_large_steps(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(0, 20, 5, &alloc);

    ASSERT_TRUE(it.is_valid(&it));

    // Start at 0
    const int* v0 = it.get(&it);
    ASSERT_EQ(*v0, 0);

    // Forward: 0->5->10
    ASSERT_EQ(it.next(&it), 0); // 0->5
    const int* v1 = it.get(&it);
    ASSERT_EQ(*v1, 5);

    ASSERT_EQ(it.next(&it), 0); // 5->10
    const int* v2 = it.get(&it);
    ASSERT_EQ(*v2, 10);

    // Backward: 10->5->0
    ASSERT_EQ(it.prev(&it), 0); // 10->5
    const int* v3 = it.get(&it);
    ASSERT_EQ(*v3, 5);

    ASSERT_EQ(it.prev(&it), 0); // 5->0
    const int* v4 = it.get(&it);
    ASSERT_EQ(*v4, 0);

    // Should be back at start
    ASSERT_FALSE(it.has_prev(&it));

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test negative step iteration with proper boundary handling.
 */
static int test_negative_step_boundaries(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(10, 0, -2, &alloc);

    ASSERT_TRUE(it.is_valid(&it));
    // Expected range: 10, 8, 6, 4, 2

    // Test forward iteration to boundary
    const int expected[] = {10, 8, 6, 4, 2};
    int actual[5];
    const int count = collect_values(&it, actual, 5);

    ASSERT_EQ(count, 5);
    ASSERT_TRUE(verify_values(actual, expected, count, "negative_step_boundaries"));

    // Should be at end, no more elements
    ASSERT_FALSE(it.has_next(&it));
    ASSERT_NULL(it.get(&it));

    // Test backward from end (but we're past the last valid element)
    // Reset to test backward from a valid position
    it.reset(&it);

    // Move to second position (8) then test backward
    it.next(&it); // 10->8
    const int* pos1 = it.get(&it);
    ASSERT_EQ(*pos1, 8);

    // Move backward to start
    ASSERT_TRUE(it.has_prev(&it));
    ASSERT_EQ(it.prev(&it), 0);
    const int* pos2 = it.get(&it);
    ASSERT_EQ(*pos2, 10);

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test operations on invalid iterator.
 */
static int test_operations_on_invalid_iterator(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(0, 5, 0, &alloc); // Invalid: zero step

    ASSERT_FALSE(it.is_valid(&it));

    // All operations should fail gracefully
    ASSERT_EQ(it.next(&it), -1);
    ASSERT_EQ(it.prev(&it), -1);
    ASSERT_FALSE(it.has_next(&it));
    ASSERT_FALSE(it.has_prev(&it));
    ASSERT_NULL(it.get(&it));

    // Reset should be safe to call but ineffective
    it.reset(&it); // Should not crash
    ASSERT_FALSE(it.is_valid(&it));

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test reset functionality after boundary errors.
 */
static int test_reset_after_boundary_errors(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(0, 3, 1, &alloc);

    ASSERT_TRUE(it.is_valid(&it));

    // Move to end and try to go past boundary
    ASSERT_EQ(it.next(&it), 0); // 0->1
    ASSERT_EQ(it.next(&it), 0); // 1->2
    ASSERT_EQ(it.next(&it), 0); // 2->3 (outside range)

    // Further advance should fail
    ASSERT_EQ(it.next(&it), -1);
    ASSERT_FALSE(it.has_next(&it));

    // Reset should still work and bring us back to start
    it.reset(&it);
    ASSERT_TRUE(it.is_valid(&it));

    const int* value = it.get(&it);
    ASSERT_NOT_NULL(value);
    ASSERT_EQ(*value, 0);

    // Normal iteration should work after reset
    ASSERT_TRUE(it.has_next(&it));
    ASSERT_EQ(it.next(&it), 0);
    const int* next_value = it.get(&it);
    ASSERT_EQ(*next_value, 1);

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test that get() calls during movement maintain pointer consistency.
 */
static int test_concurrent_get_calls_during_movement(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(100, 104, 1, &alloc);

    ASSERT_TRUE(it.is_valid(&it));

    // Get initial pointer
    const int* ptr1 = it.get(&it);
    ASSERT_EQ(*ptr1, 100);

    // Move and get new pointer
    it.next(&it);
    const int* ptr2 = it.get(&it);
    ASSERT_EQ(*ptr2, 101);

    // Move back and get another pointer
    it.prev(&it);
    const int* ptr3 = it.get(&it);
    ASSERT_EQ(*ptr3, 100);

    // All pointers should point to same memory location
    ASSERT_EQ(ptr1, ptr2);
    ASSERT_EQ(ptr2, ptr3);

    // Current value should be back to original
    ASSERT_EQ(*ptr1, 100); // ptr1 should now show updated value
    ASSERT_EQ(*ptr2, 100); // ptr2 should show same updated value
    ASSERT_EQ(*ptr3, 100); // ptr3 should show same value

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Test boundary conditions with single-step movements.
 */
static int test_single_step_boundaries(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(5, 8, 1, &alloc);

    ASSERT_TRUE(it.is_valid(&it));
    // Range: 5, 6, 7

    // Test forward boundary
    const int* v1 = it.get(&it);
    ASSERT_EQ(*v1, 5);

    it.next(&it); // 5->6
    const int* v2 = it.get(&it);
    ASSERT_EQ(*v2, 6);

    it.next(&it); // 6->7
    const int* v3 = it.get(&it);
    ASSERT_EQ(*v3, 7);

    it.next(&it); // 7->8 (outside range)
    ASSERT_NULL(it.get(&it));
    ASSERT_FALSE(it.has_next(&it));

    // Test that prev still works from invalid position
    ASSERT_TRUE(it.has_prev(&it));
    ASSERT_EQ(it.prev(&it), 0); // Should go back to 7
    const int* v4 = it.get(&it);
    ASSERT_EQ(*v4, 7);

    it.destroy(&it);
    return TEST_SUCCESS;
}

/**
 * Enhanced helper function with better validation.
 */
static int collect_values_with_validation(const ANVIterator* it, int* values, int max_count)
{
    int count = 0;
    while (it->has_next(it) && count < max_count)
    {
        const int* value = it->get(it);
        if (!value)
        {
            // get() returned null but has_next was true - this is an error
            return -1;
        }

        values[count++] = *value;

        // Verify that next() succeeds when has_next() is true
        if (it->next(it) != 0)
        {
            // next() failed but has_next() was true - this is an error
            return -1;
        }
    }
    return count;
}

/**
 * Test helper function validation.
 */
static int test_helper_function_validation(void)
{
    const ANVAllocator alloc = create_int_allocator();
    ANVIterator it = anv_iterator_range(0, 5, 1, &alloc);

    ASSERT_TRUE(it.is_valid(&it));

    int values[5];
    const int count = collect_values_with_validation(&it, values, 5);

    // Should successfully collect all 5 values
    ASSERT_EQ(count, 5);

    const int expected[] = {0, 1, 2, 3, 4};
    ASSERT_TRUE(verify_values(values, expected, count, "helper_validation"));

    it.destroy(&it);
    return TEST_SUCCESS;
}

//==============================================================================
// Test Suite Definition
//==============================================================================

typedef struct
{
    int (*func)(void);
    const char* name;
} TestCase;

static const TestCase tests[] = {
    // Basic functionality tests (should pass without changes)
    {test_range_positive_step, "test_range_positive_step"},
    {test_range_negative_step, "test_range_negative_step"},
    {test_range_larger_step, "test_range_larger_step"},
    {test_range_negative_step_size, "test_range_negative_step_size"},

    // Edge case tests
    {test_range_empty, "test_range_empty"},
    {test_single_element_range, "test_single_element_range"},
    {test_range_extreme_values, "test_range_extreme_values"},

    // Error handling tests
    {test_range_invalid_step, "test_range_invalid_step"},
    {test_invalid_allocator, "test_invalid_allocator"},

    // Performance and stress tests
    {test_range_stress, "test_range_stress"},

    // Reset functionality tests (rewritten)
    {test_range_reset, "test_range_reset"},
    {test_reset_after_bidirectional, "test_reset_after_bidirectional"},

    // Zigzag compensation tests (completely rewritten)
    {test_zigzag_compensation, "test_zigzag_compensation"},
    {test_bidirectional_boundaries, "test_bidirectional_boundaries"},
    {test_direction_change_compensation, "test_direction_change_compensation"},
    {test_start_boundary_behavior, "test_start_boundary_behavior"},

    // New API-specific tests
    {test_get_next_separation, "test_get_next_separation"},
    {test_next_return_codes, "test_next_return_codes"},
    {test_prev_return_codes, "test_prev_return_codes"},
    {test_memory_consistency, "test_memory_consistency"},

    // Additional comprehensive test cases
    {test_has_prev_at_start, "test_has_prev_at_start"},
    {test_has_next_at_end, "test_has_next_at_end"},
    {test_bidirectional_with_large_steps, "test_bidirectional_with_large_steps"},
    {test_negative_step_boundaries, "test_negative_step_boundaries"},
    {test_operations_on_invalid_iterator, "test_operations_on_invalid_iterator"},
    {test_reset_after_boundary_errors, "test_reset_after_boundary_errors"},
    {test_concurrent_get_calls_during_movement, "test_concurrent_get_calls_during_movement"},
    {test_single_step_boundaries, "test_single_step_boundaries"},
    {test_helper_function_validation, "test_helper_function_validation"},
};

//==============================================================================
// Main Test Runner
//==============================================================================

int main(void)
{
    const int num_tests = sizeof(tests) / sizeof(tests[0]);
    int failed = 0;
    int passed = 0;

    printf("Running %d range iterator tests...\n\n", num_tests);

    for (int i = 0; i < num_tests; i++)
    {
        printf("Running %s... ", tests[i].name);
        fflush(stdout);

        if (tests[i].func() == TEST_SUCCESS)
        {
            printf("PASSED\n");
            passed++;
        }
        else
        {
            printf("FAILED\n");
            failed++;
        }
    }

    printf("\n=== Range Iterator Test Results ===\n");
    printf("Total tests: %d\n", num_tests);
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", failed);

    if (failed == 0)
    {
        printf("\n🎉 All range iterator tests passed!\n");
        return 0;
    }

    printf("\n❌ %d test(s) failed.\n", failed);
    return 1;
}