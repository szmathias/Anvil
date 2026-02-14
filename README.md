# Anvil

A C11 data structures library built from scratch with an emphasis on memory safety, clean ownership semantics, and generic programming.

## Why Anvil?

Most C data structure libraries suffer from one or more common problems: unclear memory ownership, forced allocation strategies, or poor error handling. Anvil addresses these by cleanly separating **node allocation** (library-managed) from **data allocation** (user-managed), and by supporting pluggable custom allocators throughout the entire API.

## Features

**Data Structures**
- **Singly Linked List** — O(1) front insertion, with iterator support
- **Doubly Linked List** — O(1) insertion and removal at both ends
- **Dynamic String** — Growth-managed string with small string optimization *(in progress)*

**Core Systems**
- **Custom Allocator Interface** — Swap in memory pools, debug allocators, or arena allocators without changing application code. The allocator system uses distinct function pointers for allocation, copying, and deallocation, giving fine-grained control over the memory lifecycle.
- **Generic Iterator** — A unified iteration interface across all containers, supporting functional-style operations. Chain `filter` and `transform` calls to process data without writing manual loops.
- **Ownership Model** — Anvil manages internal node memory. You manage your data. This separation prevents double-frees and dangling pointers, which are common in C container libraries.

## Technical Details

| | |
|---|---|
| **Standard** | C11 |
| **Platforms** | Linux, macOS, Windows |
| **Sanitizers** | Clean under AddressSanitizer and UBSanitizer |
| **Thread Safety** | Thread-compatible with external synchronization |

## Building

```bash
# Clone the repository
git clone https://github.com/szmathias/Anvil.git
cd Anvil

# Build with CMake
mkdir build && cd build
cmake ..
make

# Run tests
./run_tests
```


## Usage Example
You can include everything with anvil.h, choose specific modules, or include only the features you want
```c
#include <anvil/anvil.h>
#include <anvil/system.h>
#include <anvil/containers/hashmap.h>
```
Here's a simple Doubly Linked List and Iterator example
```c
#include "anvil/containers/doublylinkedlist.h"
#include "anvil/containers/iterator.h"
#include <stdio.h>

// Create a list with default allocator
ANVAllocator alloc = anv_alloc_default();
ANVDoublyLinkedList* list = anv_dll_create(&alloc);

// Add elements
int a = 10, b = 20, c = 30;
anv_dll_push_back(list, &a);
anv_dll_push_back(list, &b);
anv_dll_push_back(list, &c);

// Iterate with functional operations
ANVIterator it = anv_dll_iterator(list);
while (it.has_next(&it))
{
	int* value = it.get(&it);
	if (value)
	{
		printf("Value: %d\n", *value);
	}
	it.next(&it);
}

// Cleanup
if (it.destroy)
{
	it.destroy(&it);
}
anv_dll_destroy(list, false); // Don't free the elements, they're on the stack
```


## Design Decisions

- **Function pointers for generics** — Rather than using `void*` everywhere with no type context, Anvil requires user-supplied function pointers for copying, comparing, and freeing data. This adds a small setup cost but eliminates entire categories of memory bugs.
- **No global state** — Every operation takes an explicit container reference. No hidden singletons, no thread-local storage, no surprises.
- **Fail-safe error handling** — All allocation-dependent operations return status codes. No silent failures.

## What I'd Improve

- Expand the container set (hash map, priority queue, ring buffer)
- Add benchmarks comparing against other C container libraries
- Formalize the iterator protocol to support reverse iteration
- Complete the dynamic string rewrite with full UTF-8 awareness
