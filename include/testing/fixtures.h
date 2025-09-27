//
// Created by zack on 9/26/25.
//

#ifndef ANVIL_FIXTURES_H
#define ANVIL_FIXTURES_H

#include "common/common.h"

// Forward declarations
typedef struct ANVFixture ANVFixture;
typedef struct ANVTestCase ANVTestCase;

/**
 * Fixture interface - follows the same pattern as ANVIterator
 *
 * Just like ANVIterator, this has function pointers and a data_state
 * that points to implementation-specific state structures.
 */
struct ANVFixture
{
    void* data_state;              // Implementation-specific state data (just like iterator)
    const ANVAllocator* alloc;     // Allocator for memory management

    // Lifecycle operations
    int (*setup)(ANVFixture* fixture, ANVTestCase* test);     // Initialize test environment
    void (*teardown)(ANVFixture* fixture, ANVTestCase* test); // Clean up test environment

    // Resource management
    void (*destroy)(ANVFixture* fixture);          // Free fixture resources
};

#endif //ANVIL_FIXTURES_H