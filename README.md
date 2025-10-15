# Anvil

My personal C library for reusable containers, functions, and utilities.

## What's Inside

- **Containers**: ArrayList, LinkedLists, Stack, Queue, HashMap, HashSet, and more
- **Strings**: Dynamic string manipulation
- **Memory**: Arena allocators, custom allocators, stack frames
- **Algorithms**: Simple hashing
- **Iterators**: Chain, filter, map, zip, and more
- **System**: Threads, mutexes, timing utilities
- **Testing**: Benchmark and testing helpers

## Usage

You can include everything with anvil.h, choose specific modules, or include only the features you want
```c
#include <anvil/anvil.h>
#include <anvil/system.h>
#include <anvil/containers/hashmap.h>
```

## Check out the TODO

Not everything is fully implemented and some things, like math, are placeholders for the future.