//
// Created by zack on 9/26/25.
//

#ifndef ANVIL_RESULT_H
#define ANVIL_RESULT_H

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ANV_SUCCEEDED(result) ((result) == ANV_RESULT_SUCCESS)
#define ANV_FAILED(result) ((result) != ANV_RESULT_SUCCESS)

/**
 * ANVResult - Result codes for Anvil functions.
 */
typedef enum ANVResult
{
    ANV_RESULT_SUCCESS = 0,

    // Input validation errors
    ANV_RESULT_NULL_POINTER,
    ANV_RESULT_INVALID_ARGUMENT,
    ANV_RESULT_OUT_OF_BOUNDS,

    // Resource errors
    ANV_RESULT_OUT_OF_MEMORY,
    ANV_RESULT_INSUFFICIENT_SPACE,

    // State errors
    ANV_RESULT_NOT_FOUND,
    ANV_RESULT_ALREADY_EXISTS,
    ANV_RESULT_INVALID_STATE,

    // Implementation status
    ANV_RESULT_NOT_IMPLEMENTED,
    ANV_RESULT_COUNT // Not a result code. Keep this last to get the number of result codes.
} ANVResult;

/**
 * Convert an ANVResult code to a human-readable string.
 *
 * @param result The ANVResult code to convert.
 * @return A string representation of the result code.
 */
ANV_API const char* anv_result_to_string(ANVResult result);

#ifdef __cplusplus
}
#endif

#endif // ANVIL_RESULT_H
