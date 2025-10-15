//
// Created by zack on 8/19/2025.
//

#ifndef ANVIL_DYNAMICSTRING_H
#define ANVIL_DYNAMICSTRING_H

#include <stdio.h>

#include "anvil/common.h"

#ifdef __cplusplus
extern "C" {
#endif

// Minimum initial string capacity
#define STR_MIN_INIT_CAP ANV_DEFAULT_CAPACITY

// Represents position not found
#define STR_NPOS SIZE_MAX

typedef struct ANVString
{
        size_t capacity;
        size_t size;

        union
        {
                char* data;
                char small_data[STR_MIN_INIT_CAP];
        };
} ANVString;

// Convenience macros to hopefully save some typing
// The generic implementation of these macros are towards the bottom of the
// header file so they don't clog up the rest of the API
#define anv_str_create(value) anv_str_create_impl(value)
#define anv_str_assign(str, value) anv_str_assign_impl(str, value)
#define anv_str_insert(str, pos, value) anv_str_insert_impl(str, pos, value)
#define anv_str_append(str, value) anv_str_append_impl(str, value)
#define anv_str_find_str(str, find) anv_str_find_str_impl(str, find)
#define anv_str_substr_create(str, pos, count) anv_str_substr_create_impl(str, pos, count)
#define anv_str_substr(str, pos, count, buffer) anv_str_substr_impl(str, pos, count, buffer)
#define anv_str_compare(lhs, rhs) anv_str_compare_impl(lhs, rhs)
#define anv_str_getline(stream, line, delim) anv_str_getline_impl(stream, line, delim)

// Different creation functions
ANV_API ANVString anv_str_create_empty(size_t initial_capacity);

ANV_API ANVString anv_str_create_from_cstring(const char* cstr);

ANV_API ANVString anv_str_create_from_string(const ANVString* str);

// String free method
ANV_API void anv_str_destroy(ANVString* str);

ANV_API void anv_str_destroy_split(ANVString** str, size_t count);

// Clears the content of your string and assigns the value given
ANV_API void anv_str_assign_char(ANVString* str, char value);

ANV_API void anv_str_assign_cstring(ANVString* str, const char* cstr);

ANV_API void anv_str_assign_string(ANVString* str, const ANVString* from);

// Appends a character to the end of your string
ANV_API void anv_str_push_back(ANVString* str, char value);

// Appends a cstring or another String to the end of your string
ANV_API void anv_str_append_char(ANVString* str, char value);

ANV_API void anv_str_append_cstring(ANVString* str, const char* cstr);

ANV_API void anv_str_append_string(ANVString* str, const ANVString* from);

// Inserts the value into any valid position within the string
ANV_API void anv_str_insert_char(ANVString* str, size_t pos, char value);

ANV_API void anv_str_insert_cstring(ANVString* str, size_t pos, const char* cstr);

ANV_API void anv_str_insert_string(ANVString* str, size_t pos, const ANVString* from);

// Pop back gets rid of the last char in the string
// Erase gets rid of a character at the position given then shifts the strings
// content
ANV_API void anv_str_pop_back(ANVString* str);

ANV_API void anv_str_erase(ANVString* str, size_t pos);

// Empty returns whether the string is empty or not
// Clear resets the strings content and size to zero
ANV_API bool anv_str_empty(const ANVString* str);

ANV_API void anv_str_clear(ANVString* str);

ANV_API bool anv_str_reserve(ANVString* str, size_t new_capacity);

ANV_API bool anv_str_shrink_to_fit(ANVString* str);

// Helper functions for internal data
ANV_API char* anv_str_data(ANVString* str);

ANV_API size_t anv_str_capacity(const ANVString* str);

ANV_API size_t anv_str_size(const ANVString* str);

// Finds the first character that matches any character in value and
// returns it's position or STR_NPOS
ANV_API size_t anv_str_find_first_of(const ANVString* str, const char* value);

// Searches for the string given and returns the position of the beginning
// of that string or STR_NPOS //
ANV_API size_t anv_str_find_cstring(const ANVString* str, const char* find);

ANV_API size_t anv_str_find_string(const ANVString* str, const ANVString* find);

ANV_API void anv_str_trim_front(ANVString* str);

ANV_API void anv_str_trim_back(ANVString* str);

// Removes whitespace from the front and back of the string then gets rid
// of duplicate whitespace in the middle
ANV_API void anv_str_remove_extra_ws(ANVString* str);

// Sets the entire string to lowercase or uppercase
ANV_API void anv_str_to_lower(ANVString* str);

ANV_API void anv_str_to_upper(ANVString* str);

// Returns a new string that is a sub string of the original or an empty
// string if pos is invalid
ANV_API ANVString anv_str_substr_create_cstring(const char* cstr, size_t pos, size_t count);

ANV_API ANVString anv_str_substr_create_string(const ANVString* str, size_t pos, size_t count);

// Sets buffer to the substring at pos that is count long.
// Buffer must be count + 1 long to append the null terminator at the end.
ANV_API char* anv_str_substr_cstring(const char* cstr, size_t pos, size_t count, char* buffer);

ANV_API char* anv_str_substr_string(const ANVString* str, size_t pos, size_t count, char* buffer);

// Splits the string at each occurrence of delim and returns an array of
// Strings. The number of strings created is returned by the function.
ANV_API size_t anv_str_split_cstring(const char* str, const char* delim, ANVString** out);
ANV_API size_t anv_str_split(const ANVString* str, const char* delim, ANVString** out);

// Compares lhs to rhs. If they are equivalent then we evaluate the
// length of each string.
// Returns:
//  -1 if lhs  < rhs
//   0 if lhs == rhs
//   1 if lhs  > rhs
ANV_API int anv_str_compare_cstring(const ANVString* lhs, const char* rhs);

ANV_API int anv_str_compare_string(const ANVString* lhs, const ANVString* rhs);

// Reads characters from stream into line until a delimiter or EOF is read.
// Returns 0 if delim was succesfully reached otherwise returns EOF.
ANV_API int anv_str_getline_ch(FILE* stream, ANVString* line, int delim);

ANV_API int anv_str_getline_cstring(FILE* stream, ANVString* line, const char* delim);

ANV_API int anv_str_getline_string(FILE* stream, ANVString* line, const ANVString* delim);

// The generic macro implementations for convenience
#define anv_str_create_impl(value)                \
    _Generic(value,                               \
        char*: anv_str_create_from_cstring,       \
        const char*: anv_str_create_from_cstring, \
        String*: anv_str_create_from_string,      \
        default: anv_str_create_empty)(value)

#define anv_str_assign_impl(str, value)      \
    _Generic(value,                          \
        char*: anv_str_assign_cstring,       \
        const char*: anv_str_assign_cstring, \
        String*: anv_str_assign_string,      \
        default: anv_str_assign_char)(str, value)

#define anv_str_append_impl(str, value)      \
    _Generic(value,                          \
        char*: anv_str_append_cstring,       \
        const char*: anv_str_append_cstring, \
        String*: anv_str_append_string,      \
        default: anv_str_append_char)(str, value)

#define anv_str_insert_impl(str, pos, value) \
    _Generic(value,                          \
        char*: anv_str_insert_cstring,       \
        const char*: anv_str_insert_cstring, \
        String*: anv_str_insert_string,      \
        default: anv_str_insert_char)(str, pos, value)

#define anv_str_find_str_impl(str, find)   \
    _Generic(find,                         \
        char*: anv_str_find_cstring,       \
        const char*: anv_str_find_cstring, \
        default: anv_str_find_string)(str, find)

#define anv_str_substr_create_impl(str, pos, count) \
    _Generic(str,                                   \
        char*: anv_str_substr_create_cstring,       \
        const char*: anv_str_substr_create_cstring, \
        default: anv_str_substr_create_string)(str, pos, count)

#define anv_str_substr_impl(str, pos, count, buffer) \
    _Generic(str,                                    \
        char*: anv_str_substr_cstring,               \
        const char*: anv_str_substr_cstring,         \
        default: anv_str_substr_string)(str, pos, count, buffer)

#define anv_str_compare_impl(lhs, rhs)        \
    _Generic(rhs,                             \
        char*: anv_str_compare_cstring,       \
        const char*: anv_str_compare_cstring, \
        default: anv_str_compare_string)(lhs, rhs)

#define anv_str_getline_impl(stream, line, delim) \
    _Generic(delim,                               \
        char*: anv_str_getline_cstring,           \
        const char*: anv_str_getline_cstring,     \
        String*: anv_str_getline_string,          \
        default: anv_str_getline_ch)(stream, line, delim)

#ifdef __cplusplus
}
#endif

#endif // ANVIL_DYNAMICSTRING_H
