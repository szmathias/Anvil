//
// Created by zack on 9/15/25.
//

#include <string.h>

#include "pair.h"

//==============================================================================
// Creation and destruction functions
//==============================================================================

ANV_API ANVPair* anv_pair_create(ANVAllocator* alloc, void* first, void* second)
{
    if (!alloc || !alloc->allocate || !alloc->deallocate)
    {
        return NULL;
    }

    ANVPair* pair = anv_alloc_allocate(alloc, sizeof(ANVPair));
    if (!pair)
    {
        return NULL;
    }

    pair->first = first;
    pair->second = second;
    pair->alloc = *alloc;

    return pair;
}

ANV_API int anv_pair_init(ANVPair* pair, const ANVAllocator* alloc, void* first, void* second)
{
    if (!pair)
    {
        return -1;
    }

    pair->first = first;
    pair->second = second;

    if (alloc)
    {
        pair->alloc = *alloc;
    }
    else
    {
        pair->alloc = anv_alloc_default();
    }

    return 0;
}

ANV_API void anv_pair_destroy(ANVPair* pair, const bool should_free_first, const bool should_free_second)
{
    if (!pair)
    {
        return;
    }

    if (should_free_first)
    {
        anv_alloc_data_deallocate(&pair->alloc, pair->first);
    }

    if (should_free_second)
    {
        anv_alloc_data_deallocate(&pair->alloc, pair->second);
    }

    anv_alloc_deallocate(&pair->alloc, pair);
}

//==============================================================================
// Access functions
//==============================================================================

ANV_API void* anv_pair_first(const ANVPair* pair)
{
    return pair ? pair->first : NULL;
}

ANV_API void* anv_pair_second(const ANVPair* pair)
{
    return pair ? pair->second : NULL;
}

ANV_API void anv_pair_set_first(ANVPair* pair, void* first, const bool should_free_old)
{
    if (!pair)
    {
        return;
    }

    if (should_free_old)
    {
        anv_alloc_data_deallocate(&pair->alloc, pair->first);
    }

    pair->first = first;
}

ANV_API void anv_pair_set_second(ANVPair* pair, void* second, const bool should_free_old)
{
    if (!pair)
    {
        return;
    }

    if (should_free_old)
    {
        anv_alloc_data_deallocate(&pair->alloc, pair->second);
    }

    pair->second = second;
}

//==============================================================================
// Utility functions
//==============================================================================

ANV_API void anv_pair_swap(ANVPair* pair)
{
    if (!pair)
    {
        return;
    }

    void* temp = pair->first;
    pair->first = pair->second;
    pair->second = temp;
}

ANV_API int anv_pair_compare(const ANVPair* pair1, const ANVPair* pair2,
                             const pair_compare_func first_compare, const pair_compare_func second_compare)
{
    if (!pair1 && !pair2)
    {
        return 0;
    }

    if (!pair1)
    {
        return -1;
    }

    if (!pair2)
    {
        return 1;
    }

    if (first_compare)
    {
        const int first_result = first_compare(pair1->first, pair2->first);
        if (first_result != 0)
        {
            return first_result;
        }
    }

    if (second_compare)
    {
        return second_compare(pair1->second, pair2->second);
    }

    return 0;
}

ANV_API int anv_pair_equals(const ANVPair* pair1, const ANVPair* pair2,
                            const pair_compare_func first_compare, const pair_compare_func second_compare)
{
    return anv_pair_compare(pair1, pair2, first_compare, second_compare) == 0;
}

ANV_API ANVPair* anv_pair_copy(ANVPair* pair)
{
    if (!pair)
    {
        return NULL;
    }

    return anv_pair_create(&pair->alloc, pair->first, pair->second);
}

ANV_API ANVPair* anv_pair_copy_deep(ANVPair* pair, const anv_copy_func first_copy, const anv_copy_func second_copy, const bool should_free)
{
    if (!pair)
    {
        return NULL;
    }

    ANVPair* new_pair = anv_alloc_allocate(&pair->alloc, sizeof(ANVPair));
    if (!new_pair)
    {
        return NULL;
    }
    new_pair->alloc = pair->alloc;

    if (pair->first)
    {
        new_pair->first = first_copy ? first_copy(pair->first) : pair->first;
        if (first_copy && !new_pair->first)
        {
            anv_pair_destroy(new_pair, false, false);
            return NULL;
        }
    }
    else
    {
        new_pair->first = NULL;
    }

    if (pair->second)
    {
        new_pair->second = second_copy ? second_copy(pair->second) : pair->second;
        if (second_copy && !new_pair->second)
        {
            anv_pair_destroy(new_pair, should_free, false);
            return NULL;
        }
    }
    else
    {
        new_pair->second = NULL;
    }

    return new_pair;
}

//==============================================================================
// Common copy helper functions
//==============================================================================

ANV_API void* anv_pair_copy_string_int(const void* pair_data)
{
    if (!pair_data)
    {
        return NULL;
    }

    const ANVPair* original = pair_data;

    ANVPair* new_pair = anv_alloc_allocate(&original->alloc, sizeof(ANVPair));
    if (!new_pair)
    {
        return NULL;
    }

    new_pair->alloc = original->alloc;
    new_pair->first = NULL;
    new_pair->second = NULL;

    if (original->first)
    {
        const char* str = original->first;
        const size_t len = strlen(str) + 1;
        char* str_copy = anv_alloc_allocate(&original->alloc, len);
        if (!str_copy)
        {
            anv_alloc_deallocate(&original->alloc, new_pair);
            return NULL;
        }
        strncpy(str_copy, str, len);
        new_pair->first = str_copy;
    }

    if (original->second)
    {
        int* int_copy = anv_alloc_allocate(&original->alloc, sizeof(int));
        if (!int_copy)
        {
            if (new_pair->first)
            {
                anv_alloc_deallocate(&original->alloc, new_pair->first);
            }
            anv_alloc_deallocate(&original->alloc, new_pair);
            return NULL;
        }
        *int_copy = *(const int*)original->second;
        new_pair->second = int_copy;
    }

    return new_pair;
}

ANV_API void* anv_pair_copy_int_string(const void* pair_data)
{
    if (!pair_data)
    {
        return NULL;
    }

    const ANVPair* original = pair_data;

    ANVPair* new_pair = anv_alloc_allocate(&original->alloc, sizeof(ANVPair));
    if (!new_pair)
    {
        return NULL;
    }

    new_pair->alloc = original->alloc;
    new_pair->first = NULL;
    new_pair->second = NULL;

    if (original->first)
    {
        int* int_copy = anv_alloc_allocate(&original->alloc, sizeof(int));
        if (!int_copy)
        {
            anv_alloc_deallocate(&original->alloc, new_pair);
            return NULL;
        }
        *int_copy = *(const int*)original->first;
        new_pair->first = int_copy;
    }

    // Copy string second element
    if (original->second)
    {
        const char* str = (const char*)original->second;
        const size_t len = strlen(str) + 1;
        char* str_copy = anv_alloc_allocate(&original->alloc, len);
        if (!str_copy)
        {
            if (new_pair->first)
            {
                anv_alloc_deallocate(&original->alloc, new_pair->first);
            }
            anv_alloc_deallocate(&original->alloc, new_pair);
            return NULL;
        }
        strncpy(str_copy, str, len);
        new_pair->second = str_copy;
    }

    return new_pair;
}

ANV_API void* anv_pair_copy_string_string(const void* pair_data)
{
    if (!pair_data)
    {
        return NULL;
    }

    const ANVPair* original = pair_data;

    ANVPair* new_pair = anv_alloc_allocate(&original->alloc, sizeof(ANVPair));
    if (!new_pair)
    {
        return NULL;
    }

    new_pair->alloc = original->alloc;
    new_pair->first = NULL;
    new_pair->second = NULL;

    if (original->first)
    {
        const char* str1 =original->first;
        const size_t len1 = strlen(str1) + 1;
        char* str1_copy = anv_alloc_allocate(&original->alloc, len1);
        if (!str1_copy)
        {
            anv_alloc_deallocate(&original->alloc, new_pair);
            return NULL;
        }
        strncpy(str1_copy, str1, len1);
        new_pair->first = str1_copy;
    }

    if (original->second)
    {
        const char* str2 = original->second;
        const size_t len2 = strlen(str2) + 1;
        char* str2_copy = anv_alloc_allocate(&original->alloc, len2);
        if (!str2_copy)
        {
            if (new_pair->first)
            {
                anv_alloc_deallocate(&original->alloc, new_pair->first);
            }
            anv_alloc_deallocate(&original->alloc, new_pair);
            return NULL;
        }
        strncpy(str2_copy, str2, len2);
        new_pair->second = str2_copy;
    }

    return new_pair;
}

ANV_API void* anv_pair_copy_int_int(const void* pair_data)
{
    if (!pair_data)
    {
        return NULL;
    }

    const ANVPair* original = pair_data;

    ANVPair* new_pair = anv_alloc_allocate(&original->alloc, sizeof(ANVPair));
    if (!new_pair)
    {
        return NULL;
    }

    new_pair->alloc = original->alloc;
    new_pair->first = NULL;
    new_pair->second = NULL;

    if (original->first)
    {
        int* int1_copy = anv_alloc_allocate(&original->alloc, sizeof(int));
        if (!int1_copy)
        {
            anv_alloc_deallocate(&original->alloc, new_pair);
            return NULL;
        }
        *int1_copy = *(const int*)original->first;
        new_pair->first = int1_copy;
    }

    if (original->second)
    {
        int* int2_copy = anv_alloc_allocate(&original->alloc, sizeof(int));
        if (!int2_copy)
        {
            if (new_pair->first)
            {
                anv_alloc_deallocate(&original->alloc, new_pair->first);
            }
            anv_alloc_deallocate(&original->alloc, new_pair);
            return NULL;
        }
        *int2_copy = *(const int*)original->second;
        new_pair->second = int2_copy;
    }

    return new_pair;
}