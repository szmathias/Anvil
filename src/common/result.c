//
// Created by zack on 9/26/25.
//

#include "result.h"

static const char* anv_result_strings[] = {
    [ANV_RESULT_SUCCESS] = "Success",
    [ANV_RESULT_NULL_POINTER] = "Null pointer",
    [ANV_RESULT_OUT_OF_MEMORY] = "Out of memory",
    [ANV_RESULT_INVALID_ARGUMENT] = "Invalid argument",
    [ANV_RESULT_OUT_OF_BOUNDS] = "Index out of bounds",
    [ANV_RESULT_NOT_FOUND] = "Not found",
    [ANV_RESULT_ALREADY_EXISTS] = "Already exists",
    [ANV_RESULT_INSUFFICIENT_SPACE] = "Insufficient space",
    [ANV_RESULT_INVALID_STATE] = "Invalid state",
    [ANV_RESULT_NOT_IMPLEMENTED] = "Not implemented"
};

ANV_API const char* anv_result_to_string(const ANVResult result)
{
    if (result < 0 || result >= ANV_RESULT_COUNT)
    {
        return "Unknown result code";
    }
    return anv_result_strings[result];
}
