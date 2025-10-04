//
// Created by zack on 9/26/25.
//

#ifndef ANVIL_FIXTURES_H
#define ANVIL_FIXTURES_H

#include "../common.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Example usage at bottom of file

// Forward declaration
typedef struct ANVFixture ANVFixture;
typedef struct ANVTestCase ANVTestCase;

/**
 * Fixture interface
 *
 * Just like ANVIterator, this has function pointers and a data_state
 * that points to implementation-specific state structures.
 */
typedef struct ANVFixture
{
        void* data_state;          // Implementation-specific state data
        const ANVAllocator alloc;

        int (*setup)(ANVFixture* fixture, ANVTestCase* test);     // Initialize test environment
        void (*teardown)(ANVFixture* fixture, ANVTestCase* test); // Clean up test environment

        void (*destroy)(ANVFixture* fixture); // Free fixture resources
} ANVFixture;

//==============================================================================
// Example Fixture Patterns
//==============================================================================

/*
// Example: Basic fixture state
typedef struct {
    ANVAllocator* allocator;
} BasicFixtureState;

// Example: ArrayList fixture
typedef struct {
    ANVAllocator* allocator;
    ANVArrayList* list;
    int* test_data;
    size_t data_count;
} ArrayListFixtureState;

static int arraylist_fixture_setup(ANVFixture* fixture, ANVTestCase* test) {
    ArrayListFixtureState* state = fixture->data_state;

    // Create fresh list for each test
    state->list = anv_arraylist_create(state->allocator);
    if (!state->list) return -1;

    // Initialize test data
    for (size_t i = 0; i < state->data_count; i++) {
        state->test_data[i] = (int)(i * 10);
    }

    test->allocator = state->allocator;
    return 0;
}

static void arraylist_fixture_teardown(ANVFixture* fixture, ANVTestCase* test) {
    ArrayListFixtureState* state = fixture->data_state;

    if (state->list) {
        anv_arraylist_destroy(state->list, false);
        state->list = NULL;
    }
}

static void arraylist_fixture_destroy(ANVFixture* fixture) {
    if (fixture && fixture->data_state) {
        ArrayListFixtureState* state = fixture->data_state;
        free(state->test_data);
        anv_alloc_free(fixture->alloc, fixture->data_state);
        fixture->data_state = NULL;
    }
}

ANVFixture create_arraylist_fixture(size_t data_count) {
    ANVFixture fixture = {0};

    fixture.setup = arraylist_fixture_setup;
    fixture.teardown = arraylist_fixture_teardown;
    fixture.destroy = arraylist_fixture_destroy;
    fixture.alloc = anv_alloc_default();

    ArrayListFixtureState* state = anv_alloc_malloc(fixture.alloc, sizeof(ArrayListFixtureState));
    state->allocator = anv_alloc_default();
    state->list = NULL;  // Created in setup
    state->data_count = data_count;
    state->test_data = malloc(data_count * sizeof(int));

    fixture.data_state = state;
    return fixture;
}

// Usage in tests:
void test_arraylist_push(ANVTestCase* test) {
    ArrayListFixtureState* state = test->fixture->data_state;

    anv_arraylist_push(state->list, &state->test_data[0]);
    ANV_ASSERT_EQUAL_INT(1, anv_arraylist_size(state->list), "Size should be 1");
}

// Example: Custom allocator fixture
typedef struct {
    ANVAllocator* system_allocator;
    ANVArena* arena;
    ANVAllocator* arena_allocator;
    size_t arena_size;
} ArenaFixtureState;

// Setup/teardown/destroy functions similar to above...

ANVFixture create_arena_fixture(size_t arena_size) {
    // Similar pattern to arraylist fixture
}

// Example: Multi-container comparison fixture
typedef struct {
    ANVAllocator* allocator;
    ANVArrayList* arraylist;
    ANVVector* vector;
    ANVLinkedList* linkedlist;
    int* shared_test_data;
    size_t data_count;
} ContainerComparisonState;

// This fixture sets up multiple containers for comparison testing/benchmarking
*/


#ifdef __cplusplus
}
#endif

#endif // ANVIL_FIXTURES_H
