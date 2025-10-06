//
// Created by zack on 10/4/25.
//

#include "anvil/io/print.h"

#include <stdarg.h>
#include <stdio.h>

#include "anvil/containers/dynamicstring.h"

#if defined(__GNUC__) || defined(__clang__)
    #define ANV_PRINTF_LIKE(fmt, args) __attribute__((format(printf, 1, 2)))
#else
    #define ANV_PRINTF_LIKE(fmt, args)
#endif

#define TEMP_BUFFER_SIZE 1024




ANV_PRINTF_LIKE(1, 2)
ANV_API void anv_print_format(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    ANVString output = anv_str_create_empty(0);
    const char *text = format;
    while (*text)
    {
        if (*text == '{' && *(text + 1) != '{')
        {
            uint32_t brace_pos = 1;
            while (text[brace_pos] && text[brace_pos] != '}')
            {
                brace_pos++;
            }

            if (text[brace_pos] == '}')
            {
                text++;
                char buffer[TEMP_BUFFER_SIZE] = {0};

                if (brace_pos - 1 > TEMP_BUFFER_SIZE - 1)
                {
                    brace_pos = TEMP_BUFFER_SIZE - 1;
                }
                anv_str_substr_cstring(text, 0, brace_pos - 1, buffer);

                ANVString *parts = NULL;
                const size_t num_parts = anv_str_split_cstring(buffer, ":", &parts);

                for (unsigned int i = 0; i < num_parts; i++)
                {
                    printf("Part[%d] = %s\n", i + 1, anv_str_data(&parts[i]));
                }



                anv_str_destroy_split(&parts, num_parts);

                text += brace_pos; // Move past the closing brace
            }
            else
            {
                // No closing brace found, treat as normal text
                anv_str_append_char(&output, *text);
                text++;
            }
        }
        else if (*text == '{' && *(text + 1) == '{')
        {
            anv_str_append_char(&output, '{');
            text += 2;
        }
        else if (*text == '}' && *(text + 1) == '}')
        {
            anv_str_append_char(&output, '}');
            text += 2;
        }
        else
        {
            anv_str_append_char(&output, *text);
            text++;
        }
    }
    format = anv_str_data(&output);



    vprintf(format, args);

    anv_str_destroy(&output);




    va_end(args);
}