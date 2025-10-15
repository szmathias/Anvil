//
// Created by zack on 9/26/25.
//

#include "hash.h"

ANV_API size_t anv_hash_string(const void* key)
{
    if (!key)
    {
        return 0;
    }

    const char* str = key;
    size_t hash = 5381;
    int c;

    while ((c = (unsigned char)*str++))
    {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }

    return hash;
}

ANV_API size_t anv_hash_int(const void* key)
{
    if (!key)
    {
        return 0;
    }

    const int value = *(const int*)key;

    size_t hash = (size_t)value;
    hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
    hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
    hash = (hash >> 16) ^ hash;
    return hash;
}

ANV_API size_t anv_hash_pointer(const void* key)
{
    const uintptr_t addr = (uintptr_t)key;
    size_t hash = addr;
    hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
    hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
    hash = (hash >> 16) ^ hash;
    return hash;
}