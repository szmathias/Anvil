//
// Created by zack on 10/4/25.
//

#include "anvil/io.h"
#include "anvil/containers/dynamicstring.h"

int main(void)
{
    // ANVString str  = anv_str_create_from_cstring(":");
    //
    // ANVString *parts = NULL;
    // const size_t num_parts = anv_str_split(&str, ":", &parts);
    // for (size_t i = 0; i < num_parts; i++)
    // {
    //     printf("Part %zu: %s\n", i + 1, anv_str_data(&parts[i]));
    // }
    //
    // anv_str_destroy(&str);
    // anv_str_destroy_split(&parts, num_parts);


    anv_print_format("{}Hello, %s! {stack} {string:color=red:width=30:align=left}The {{{answer}}} is %d.\n", "World", 42);
    return 0;
}