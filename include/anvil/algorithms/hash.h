//
// Created by zack on 9/26/25.
//

#ifndef ANVIL_HASH_H
#define ANVIL_HASH_H

#include "anvil/common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Generic hash function type
 *
 * @param data Data to hash
 * @return Hash value
 */
typedef size_t (*anv_hash_func)(const void* data);

/**
 * Hash function for string keys.
 *
 * @param key Pointer to null-terminated string
 * @return Hash value
 */
ANV_API size_t anv_hash_string(const void* key);

/**
 * Hash function for integer keys.
 *
 * @param key Pointer to int
 * @return Hash value
 */
ANV_API size_t anv_hash_int(const void* key);

/**
 * Hash function for pointer keys (uses memory address).
 *
 * @param key Pointer value
 * @return Hash value
 */
ANV_API size_t anv_hash_pointer(const void* key);

#ifdef __cplusplus
}
#endif

#endif // ANVIL_HASH_H
