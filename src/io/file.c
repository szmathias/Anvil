//
// Created by zack on 10/9/25.
//

#include "anvil/io/file.h"

ANV_API ANVFile* anv_file_create(ANVAllocator *alloc, const char* path)
{
    if (!alloc || !path)
    {
        return NULL;
    }

    ANVFile* file = anv_alloc_allocate(alloc, sizeof(ANVFile));
    if (!file)
    {
        return NULL;
    }

    file->allocator = *alloc;
    file->handle = NULL;
    file->contents = NULL;
    file->size = 0;
    file->path = anv_str_create_from_cstring(path);

    return file;
}

ANV_API ANVResult anv_file_destroy(ANVFile* file)
{
    if (!file)
    {
        return ANV_RESULT_INVALID_ARGUMENT;
    }

    if (file->handle)
    {
        fclose(file->handle);
        file->handle = NULL;
    }

    if (file->contents)
    {
        anv_alloc_deallocate(&file->allocator, file->contents);
        file->contents = NULL;
    }

    anv_str_destroy(&file->path);
    anv_alloc_deallocate(&file->allocator, file);

    return ANV_RESULT_SUCCESS;
}

ANV_API ANVResult anv_file_read(ANVFile* file)
{
    if (!file)
    {
        return ANV_RESULT_INVALID_ARGUMENT;
    }

    file->handle = fopen(anv_str_data(&file->path), "rb");
    if (!file->handle)
    {
        return ANV_RESULT_NOT_FOUND;
    }

    fseek(file->handle, 0, SEEK_END);
    const long file_size = ftell(file->handle);
    fseek(file->handle, 0, SEEK_SET);

    if (file_size < 0)
    {
        fclose(file->handle);
        file->handle = NULL;
        return ANV_RESULT_OUT_OF_BOUNDS;
    }

    file->size = (size_t)file_size;
    file->contents = anv_alloc_allocate(&file->allocator, file->size + 1);
    if (!file->contents)
    {
        fclose(file->handle);
        file->handle = NULL;
        return ANV_RESULT_OUT_OF_MEMORY;
    }

    const size_t read_size = fread(file->contents, 1, file->size, file->handle);
    if (read_size != file->size)
    {
        anv_alloc_deallocate(&file->allocator, file->contents);
        file->contents = NULL;
        fclose(file->handle);
        file->handle = NULL;
        return ANV_RESULT_INSUFFICIENT_SPACE;
    }

    file->contents[file->size] = '\0';

    fclose(file->handle);
    file->handle = NULL;

    return ANV_RESULT_SUCCESS;
}

ANV_API ANVResult anv_file_write(ANVFile* file, const uint8_t* data, const size_t size)
{
    if (!file || !data || size == 0)
    {
        return ANV_RESULT_INVALID_ARGUMENT;
    }

    file->handle = fopen(anv_str_data(&file->path), "wb");
    if (!file->handle)
    {
        return ANV_RESULT_NOT_FOUND;
    }

    const size_t write_size = fwrite(data, 1, size, file->handle);
    if (write_size != size)
    {
        fclose(file->handle);
        file->handle = NULL;
        return ANV_RESULT_INSUFFICIENT_SPACE;
    }

    fclose(file->handle);
    file->handle = NULL;

    return ANV_RESULT_SUCCESS;
}

ANV_API ANVResult anv_file_write_append(ANVFile* file, const uint8_t* data, const size_t size)
{
    if (!file || !data || size == 0)
    {
        return ANV_RESULT_INVALID_ARGUMENT;
    }

    file->handle = fopen(anv_str_data(&file->path), "ab");
    if (!file->handle)
    {
        return ANV_RESULT_NOT_FOUND;
    }

    const size_t write_size = fwrite(data, 1, size, file->handle);
    if (write_size != size)
    {
        fclose(file->handle);
        file->handle = NULL;
        return ANV_RESULT_INSUFFICIENT_SPACE;
    }

    fclose(file->handle);
    file->handle = NULL;

    return ANV_RESULT_SUCCESS;
}
