//
// Created by szmat on 8/19/2025.
//

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dynamicstring.h"

#define SAFE_FREE(ptr)    \
    do                    \
    {                     \
        if ((ptr))        \
        {                 \
            free((ptr));  \
            (ptr) = NULL; \
        }                 \
    } while (0)

#define GROW_CAPACITY(cap) ((cap) + ((cap) >> 1))

#define STR_DATA(str) ((str)->capacity == STR_MIN_INIT_CAP ? (str)->small_data : (str)->data)
#define ZERO_MEM(ptr, size) memset((ptr), 0, (size))

static char* mem_calloc(const size_t num_elements)
{
    return calloc(num_elements, sizeof(char));
}

static bool anv_str_realloc(ANVString* str, const size_t new_capacity)
{
    const char* old_data = STR_DATA(str);
    const size_t copy_size = str->size;

    if (new_capacity <= STR_MIN_INIT_CAP)
    {
        char* temp = calloc(copy_size + 1, sizeof(char));
        if (!temp && copy_size > 0)
        {
            return false;
        }

        memcpy(temp, old_data, copy_size);
        SAFE_FREE(str->data);

        ZERO_MEM(str->small_data, STR_MIN_INIT_CAP);
        memcpy(str->small_data, temp, copy_size);
        str->capacity = STR_MIN_INIT_CAP;
        free(temp);
    }
    else
    {
        char* new_data = mem_calloc(new_capacity);
        if (!new_data)
        {
            return false;
        }

        memcpy(new_data, old_data, copy_size);
        if (str->capacity > STR_MIN_INIT_CAP && str->data)
        {
            SAFE_FREE(str->data);
        }

        str->data = new_data;
        str->capacity = new_capacity;
    }
    return true;
}

static bool anv_str_ensure_capacity(ANVString* str, const size_t required_capacity)
{
    if (required_capacity <= str->capacity)
    {
        return true;
    }

    size_t new_capacity = GROW_CAPACITY(str->capacity);
    if (new_capacity < required_capacity)
    {
        new_capacity = required_capacity;
    }

    return anv_str_realloc(str, new_capacity);
}

ANV_API ANVString anv_str_create_empty(const size_t initial_capacity)
{
    ANVString result;

    const size_t capacity = (initial_capacity >= STR_MIN_INIT_CAP) ? initial_capacity : STR_MIN_INIT_CAP;

    result.capacity = capacity;
    result.size = 0;

    if (capacity > STR_MIN_INIT_CAP)
    {
        result.data = mem_calloc(capacity);
        if (!result.data)
        {
            result.capacity = STR_MIN_INIT_CAP;
            ZERO_MEM(result.small_data, STR_MIN_INIT_CAP);
        }
    }
    else
    {
        ZERO_MEM(result.small_data, STR_MIN_INIT_CAP);
        result.data = NULL;
    }

    return result;
}

ANV_API ANVString anv_str_create_from_cstring(const char* cstr)
{
    if (!cstr)
    {
        return anv_str_create_empty(0);
    }

    const size_t length = strlen(cstr);
    ANVString result = anv_str_create_empty(length + 1);
    char* data_to_use = STR_DATA(&result);
    memcpy(data_to_use, cstr, length);
    result.size = length;

    return result;
}

ANV_API ANVString anv_str_create_from_string(const ANVString* str)
{
    if (!str)
    {
        return anv_str_create_empty(0);
    }

    const char* data_to_use = STR_DATA(str);
    return anv_str_create_from_cstring(data_to_use);
}

ANV_API void anv_str_destroy(ANVString* str)
{
    if (!str)
    {
        return;
    }

    if (str->capacity != STR_MIN_INIT_CAP)
    {
        free(str->data);
    }
    str->size = 0;
    str->capacity = 0;
    str->data = NULL;
}

ANV_API void anv_str_destroy_split(ANVString** str, const size_t count)
{
    if (!str)
    {
        return;
    }

    if (count == 0)
    {
        free(*str);
        *str = NULL;
        return;
    }

    for (size_t i = 0; i < count; i++)
    {
        ANVString temp = (*str)[i];
        anv_str_destroy(&temp);
    }
    free(*str);
    *str = NULL;
}

ANV_API void anv_str_assign_char(ANVString* str, const char value)
{
    if (!str)
    {
        return;
    }

    anv_str_clear(str);
    char* data_to_use = STR_DATA(str);
    data_to_use[0] = value;
    str->size = 1;
}

ANV_API void anv_str_assign_cstring(ANVString* str, const char* cstr)
{
    if (!str || !cstr)
    {
        return;
    }

    anv_str_clear(str);
    const size_t length = strlen(cstr);
    anv_str_ensure_capacity(str, length + 1);
    char* data_to_use = STR_DATA(str);
    memcpy(data_to_use, cstr, length + 1);
    str->size = length;
}

ANV_API void anv_str_assign_string(ANVString* str, const ANVString* from)
{
    if (!str || !from)
    {
        return;
    }

    if (str == from)
    {
        return;
    }

    const char* data_to_use = STR_DATA(from);
    anv_str_assign_cstring(str, data_to_use);
}

ANV_API void anv_str_push_back(ANVString* str, const char value)
{
    if (!str)
    {
        return;
    }

    anv_str_ensure_capacity(str, str->size + 1);

    if (str->capacity == STR_MIN_INIT_CAP)
    {
        str->small_data[str->size] = value;
    }
    else
    {
        str->data[str->size] = value;
    }

    ++str->size;
}

ANV_API void anv_str_append_char(ANVString* str, const char value)
{
    if (!str)
    {
        return;
    }

    anv_str_push_back(str, value);
}

ANV_API void anv_str_append_cstring(ANVString* str, const char* cstr)
{
    if (!str || !cstr)
    {
        return;
    }

    const size_t length = strlen(cstr);
    anv_str_ensure_capacity(str, str->size + length + 1);

    char* data_to_use = STR_DATA(str);
    memcpy(data_to_use + str->size, cstr, length);

    str->size += length;
}

ANV_API void anv_str_append_string(ANVString* str, const ANVString* from)
{
    if (!str || !from)
    {
        return;
    }

    const char* data_to_use = STR_DATA(from);
    anv_str_append_cstring(str, data_to_use);
}

ANV_API void anv_str_insert_char(ANVString* str, const size_t pos, const char value)
{
    if (!str)
    {
        return;
    }

    if (pos > str->size)
    {
        return;
    }

    if (str->size + 1 > str->capacity - 1)
    {
        anv_str_realloc(str, GROW_CAPACITY(str->capacity));
    }

    char* data_to_use = STR_DATA(str);
    if (pos == str->size)
    {
        data_to_use[str->size] = value;
    }
    else
    {
        for (size_t i = str->size; i > pos; i--)
        {
            data_to_use[i] = data_to_use[i - 1];
        }
        data_to_use[pos] = value;
    }

    str->size++;
}

ANV_API void anv_str_insert_cstring(ANVString* str, const size_t pos, const char* cstr)
{
    if (!str || !cstr)
    {
        return;
    }

    if (pos > str->size)
    {
        return;
    }

    if (pos == str->size)
    {
        anv_str_append_cstring(str, cstr);
    }
    else
    {
        for (size_t i = 0; i < strlen(cstr); i++)
        {
            anv_str_insert_char(str, pos + i, cstr[i]);
        }
    }
}

ANV_API void anv_str_insert_string(ANVString* str, const size_t pos, const ANVString* from)
{
    if (!str || !from)
    {
        return;
    }

    const char* data_to_use = STR_DATA(from);
    anv_str_insert_cstring(str, pos, data_to_use);
}

ANV_API void anv_str_pop_back(ANVString* str)
{
    if (!str)
    {
        return;
    }

    if (str->size > 0)
    {
        char* data_to_use = STR_DATA(str);
        str->size--;
        data_to_use[str->size] = '\0';
    }
}

ANV_API void anv_str_erase(ANVString* str, const size_t pos)
{
    if (!str)
    {
        return;
    }

    if (str->size == 0 || pos >= str->size)
    {
        return;
    }

    if (pos == str->size - 1)
    {
        anv_str_pop_back(str);
    }
    else
    {
        char* data_to_use = STR_DATA(str);

        for (size_t i = pos; i < str->size - 1; i++)
        {
            data_to_use[i] = data_to_use[i + 1];
        }
        str->size--;
        data_to_use[str->size] = '\0';
    }
}

ANV_API bool anv_str_empty(const ANVString* str)
{
    if (!str)
    {
        return true;
    }

    return (str->size == 0);
}

ANV_API void anv_str_clear(ANVString* str)
{
    if (!str)
    {
        return;
    }

    char* data_to_use = STR_DATA(str);

    ZERO_MEM(data_to_use, str->size);
    str->size = 0;
}

ANV_API bool anv_str_reserve(ANVString* str, const size_t new_capacity)
{
    if (!str)
    {
        return false;
    }

    if (new_capacity <= str->capacity)
    {
        return false;
    }

    return anv_str_realloc(str, new_capacity);
}

ANV_API bool anv_str_shrink_to_fit(ANVString* str)
{
    if (!str)
    {
        return false;
    }

    size_t new_capacity = str->size + 1;
    if (new_capacity < STR_MIN_INIT_CAP)
    {
        new_capacity = STR_MIN_INIT_CAP;
    }

    if (new_capacity == str->capacity)
    {
        return true;
    }

    anv_str_realloc(str, new_capacity);
    return true;
}

ANV_API char* anv_str_data(ANVString* str)
{
    if (!str)
    {
        return NULL;
    }

    return STR_DATA(str);
}

ANV_API size_t anv_str_capacity(const ANVString* str)
{
    if (!str)
    {
        return 0;
    }

    return str->capacity;
}

ANV_API size_t anv_str_size(const ANVString* str)
{
    if (!str)
    {
        return 0;
    }

    return str->size;
}

ANV_API size_t anv_str_find_first_of(const ANVString* str, const char* value)
{
    if (!str || !value)
    {
        return STR_NPOS;
    }

    const char* data_to_use = STR_DATA(str);

    const char* result = strpbrk(data_to_use, value);
    if (result != NULL)
    {
        return (size_t)(result - data_to_use);
    }
    return STR_NPOS;
}

ANV_API size_t anv_str_find_cstring(const ANVString* str, const char* find)
{
    if (!str || !find)
    {
        return STR_NPOS;
    }

    const size_t length = strlen(find);
    if (length == 0)
    {
        return STR_NPOS;
    }

    const char* data_to_use = STR_DATA(str);

    size_t pos = STR_NPOS;
    for (size_t i = 0; i < str->size; i++)
    {
        if (data_to_use[i] == find[0])
        {
            if (strncmp(data_to_use + i, find, length) == 0)
            {
                pos = i;
                break;
            }
        }
    }
    return pos;
}

ANV_API size_t anv_str_find_string(const ANVString* str, const ANVString* find)
{
    if (!str || !find)
    {
        return STR_NPOS;
    }

    const char* data_to_use = STR_DATA(find);
    return anv_str_find_cstring(str, data_to_use);
}

ANV_API void anv_str_trim_front(ANVString* str)
{
    if (!str)
    {
        return;
    }

    char* data = STR_DATA(str);
    size_t i = 0;

    while (i < str->size && isspace((unsigned char)data[i]))
    {
        i++;
    }

    if (i == 0)
    {
        return;
    }

    const size_t count = str->size - i;
    if (count == 0)
    {
        anv_str_clear(str);
    }
    else
    {
        memmove(data, data + i, count);
        for (size_t j = 0; j < i; ++j)
        {
            anv_str_pop_back(str);
        }
    }
}

ANV_API void anv_str_trim_back(ANVString* str)
{
    if (!str)
    {
        return;
    }

    if (str->size == 0)
    {
        return;
    }

    const char* data = STR_DATA(str);
    while (isspace(data[str->size - 1]))
    {
        anv_str_pop_back(str);
    }
}

ANV_API void anv_str_remove_extra_ws(ANVString* str)
{
    if (!str)
    {
        return;
    }

    char* data_to_use = STR_DATA(str);

    anv_str_trim_back(str);
    anv_str_trim_front(str);
    for (size_t i = 0; i < str->size; ++i)
    {
        if (isspace((unsigned char)data_to_use[i]) != 0 && isspace((unsigned char)data_to_use[i + 1]) != 0)
        {
            if (data_to_use[i] != ' ')
            {
                data_to_use[i] = ' ';
            }
            anv_str_erase(str, i);
            --i;
        }
        else if (isspace((unsigned char)data_to_use[i]) != 0 && data_to_use[i] != ' ')
        {
            data_to_use[i] = ' ';
        }
    }
}

ANV_API void anv_str_to_lower(ANVString* str)
{
    if (!str)
    {
        return;
    }

    char* data_to_use = STR_DATA(str);
    for (size_t i = 0; i < str->size; i++)
    {
        data_to_use[i] = (char)tolower((unsigned char)data_to_use[i]);
    }
}

ANV_API void anv_str_to_upper(ANVString* str)
{
    if (!str)
    {
        return;
    }

    char* data_to_use = STR_DATA(str);
    for (size_t i = 0; i < str->size; i++)
    {
        data_to_use[i] = (char)toupper((unsigned char)data_to_use[i]);
    }
}

ANV_API ANVString anv_str_substr_create_cstring(const char* cstr, const size_t pos, size_t count)
{
    if (!cstr)
    {
        return anv_str_create_empty(0);
    }

    const size_t size = strlen(cstr);

    if (count + pos >= size)
    {
        count = size - pos;
    }

    if (size - count > size)
    {
        return anv_str_create_empty(0);
    }

    if (pos >= size)
    {
        return anv_str_create_empty(0);
    }

    ANVString result = anv_str_create_empty(count);
    for (size_t i = 0; i < count; i++)
    {
        anv_str_push_back(&result, cstr[i + pos]);
    }

    return result;
}

ANV_API ANVString anv_str_substr_create_string(const ANVString* str, const size_t pos, const size_t count)
{
    if (!str)
    {
        return anv_str_create_empty(0);
    }

    const char* data_to_use = STR_DATA(str);
    return anv_str_substr_create_cstring(data_to_use, pos, count);
}

ANV_API char* anv_str_substr_cstring(const char* cstr, const size_t pos, size_t count, char* buffer)
{
    if (!cstr || !buffer)
    {
        return buffer;
    }

    const size_t size = strlen(cstr);
    if (count + pos >= size)
    {
        count = size - pos;
    }

    if (size - count > size)
    {
        return buffer;
    }

    if (pos >= size)
    {
        return buffer;
    }

    for (size_t i = 0; i < count; i++)
    {
        buffer[i] = cstr[i + pos];
    }
    buffer[count] = '\0';
    return buffer;
}

ANV_API char* anv_str_substr_string(const ANVString* str, const size_t pos, const size_t count, char* buffer)
{
    if (!str || !buffer)
    {
        return buffer;
    }

    const char* data_to_use = STR_DATA(str);
    return anv_str_substr_cstring(data_to_use, pos, count, buffer);
}

ANV_API size_t anv_str_split_cstring(const char* str, const char* delim, ANVString** out)
{
    if (!str || !delim)
    {
        return 0;
    }

    const size_t str_size = strlen(str);
    if (str_size == 0)
    {
        return 0;
    }

    ANVString *parts = malloc(sizeof(ANVString));
    if (!parts)
    {
        return 0;
    }

    char *buffer = mem_calloc(str_size + 1);
    if (!buffer)
    {
        free(parts);
        return 0;
    }

    memcpy(buffer, str, str_size);
    size_t num_strings = 0;
    const char* token = strtok(buffer, delim);
    while (token != NULL)
    {
        ANVString* temp = realloc(parts, sizeof(ANVString) * (num_strings + 1));
        if (!temp)
        {
            free(parts);
            return 0;
        }

        parts = temp;
        parts[num_strings] = anv_str_create_from_cstring(token);
        num_strings++;
        token = strtok(NULL, delim);
    }
    *out = parts;
    free(buffer);

    return num_strings;
}

ANV_API size_t anv_str_split(const ANVString* str, const char* delim, ANVString** out)
{
    if (!str || !delim)
    {
        return 0;
    }

    const char* data_to_use = STR_DATA(str);
    return anv_str_split_cstring(data_to_use, delim, out);
}

ANV_API int anv_str_compare_cstring(const ANVString* lhs, const char* rhs)
{
    if (!lhs || !rhs)
    {
        return -1;
    }

    const char* data_to_use = STR_DATA(lhs);

    const size_t rhs_size = strlen(rhs);
    const size_t min = (lhs->size < strlen(rhs)) ? lhs->size : rhs_size;

    int result = strncmp(data_to_use, rhs, min);
    if (result != 0)
    {
        return result;
    }
    if (lhs->size < rhs_size)
    {
        result = -1;
    }
    else if (lhs->size > rhs_size)
    {
        result = 1;
    }
    return result;
}

ANV_API int anv_str_compare_string(const ANVString* lhs, const ANVString* rhs)
{
    if (!lhs || !rhs)
    {
        return -1;
    }

    const char* rhs_data_to_use = STR_DATA(rhs);
    return anv_str_compare_cstring(lhs, rhs_data_to_use);
}

ANV_API int anv_str_getline_ch(FILE* stream, ANVString* line, int delim)
{
    if (line == NULL || stream == NULL)
    {
        return EOF;
    }

    if (delim < 0 || delim > 255)
    {
        return EOF;
    }

    anv_str_clear(line);
    int ch = 0;
    int status = 0;

    if (delim == '\0')
    {
        delim = '\n';
    }

    while ((ch = fgetc(stream)) != EOF && ch != delim)
    {
        anv_str_push_back(line, (char)ch);
    }

    if (feof(stream))
    {
        status = EOF;
    }
    return status;
}

ANV_API int anv_str_getline_cstring(FILE* stream, ANVString* line, const char* delim)
{
    if (line == NULL || stream == NULL)
    {
        return EOF;
    }

    if (delim == NULL || strlen(delim) == 0)
    {
        delim = "\n";
    }

    anv_str_clear(line);

    int ch = 0;
    int status = 0;

    while (((ch = fgetc(stream))) && ch != EOF)
    {
        if (strchr(delim, ch) != NULL)
        {
            break;
        }
        anv_str_push_back(line, (char)ch);
    }

    if (ch == EOF)
    {
        status = EOF;
    }
    return status;
}

ANV_API int anv_str_getline_string(FILE* stream, ANVString* line, const ANVString* delim)
{
    if (!line || !stream || !delim)
    {
        return EOF;
    }

    const char* delim_data = STR_DATA(delim);
    return anv_str_getline_cstring(stream, line, delim_data);
}