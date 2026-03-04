#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <anvil/testing.h>
#include "TestHelpers.h"
#include "io/file.h"

//==============================================================================
// Static Helpers
//==============================================================================

#define TEST_FILE_PATH "anvil_test_io_tmp.bin"
#define TEST_FILE_PATH2 "anvil_test_io_tmp2.bin"

static void cleanup_test_file(const char* path)
{
    remove(path);
}

//==============================================================================
// Creation / Destruction Tests
//==============================================================================

int test_file_create_destroy(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVFile* file = anv_file_create(&alloc, TEST_FILE_PATH);
    ASSERT_NOT_NULL(file);
    ASSERT_NULL(file->handle);
    ASSERT_NULL(file->contents);
    ASSERT_EQ(file->size, 0);

    ANVResult res = anv_file_destroy(file);
    ASSERT_EQ(res, ANV_RESULT_SUCCESS);
    return TEST_SUCCESS;
}

int test_file_create_null_params(void)
{
    ANVAllocator alloc = create_int_allocator();
    ASSERT_NULL(anv_file_create(NULL, TEST_FILE_PATH));
    ASSERT_NULL(anv_file_create(&alloc, NULL));
    ASSERT_NULL(anv_file_create(NULL, NULL));
    return TEST_SUCCESS;
}

int test_file_destroy_null(void)
{
    ASSERT_EQ(anv_file_destroy(NULL), ANV_RESULT_INVALID_ARGUMENT);
    return TEST_SUCCESS;
}

//==============================================================================
// Write Tests
//==============================================================================

int test_file_write_basic(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVFile* file = anv_file_create(&alloc, TEST_FILE_PATH);
    ASSERT_NOT_NULL(file);

    const char* data = "Hello, Anvil!";
    ANVResult res = anv_file_write(file, (const uint8_t*)data, strlen(data));
    ASSERT_EQ(res, ANV_RESULT_SUCCESS);

    anv_file_destroy(file);
    cleanup_test_file(TEST_FILE_PATH);
    return TEST_SUCCESS;
}

int test_file_write_null_params(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVFile* file = anv_file_create(&alloc, TEST_FILE_PATH);
    ASSERT_NOT_NULL(file);

    const char* data = "test";
    ASSERT_EQ(anv_file_write(NULL, (const uint8_t*)data, 4), ANV_RESULT_INVALID_ARGUMENT);
    ASSERT_EQ(anv_file_write(file, NULL, 4), ANV_RESULT_INVALID_ARGUMENT);
    ASSERT_EQ(anv_file_write(file, (const uint8_t*)data, 0), ANV_RESULT_INVALID_ARGUMENT);

    anv_file_destroy(file);
    return TEST_SUCCESS;
}

//==============================================================================
// Read Tests
//==============================================================================

int test_file_read_basic(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Write some data first
    ANVFile* writer = anv_file_create(&alloc, TEST_FILE_PATH);
    ASSERT_NOT_NULL(writer);
    const char* data = "Hello, Anvil!";
    ASSERT_EQ(anv_file_write(writer, (const uint8_t*)data, strlen(data)), ANV_RESULT_SUCCESS);
    anv_file_destroy(writer);

    // Now read it back
    ANVFile* reader = anv_file_create(&alloc, TEST_FILE_PATH);
    ASSERT_NOT_NULL(reader);
    ANVResult res = anv_file_read(reader);
    ASSERT_EQ(res, ANV_RESULT_SUCCESS);
    ASSERT_NOT_NULL(reader->contents);
    ASSERT_EQ(reader->size, strlen(data));
    ASSERT_EQ(memcmp(reader->contents, data, strlen(data)), 0);
    // Should be null-terminated
    ASSERT_EQ(reader->contents[reader->size], '\0');

    anv_file_destroy(reader);
    cleanup_test_file(TEST_FILE_PATH);
    return TEST_SUCCESS;
}

int test_file_read_null(void)
{
    ASSERT_EQ(anv_file_read(NULL), ANV_RESULT_INVALID_ARGUMENT);
    return TEST_SUCCESS;
}

int test_file_read_nonexistent(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVFile* file = anv_file_create(&alloc, "nonexistent_anvil_test_file_xyz.bin");
    ASSERT_NOT_NULL(file);
    ASSERT_EQ(anv_file_read(file), ANV_RESULT_NOT_FOUND);
    anv_file_destroy(file);
    return TEST_SUCCESS;
}

//==============================================================================
// Append Tests
//==============================================================================

int test_file_write_append_basic(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Write initial data
    ANVFile* file = anv_file_create(&alloc, TEST_FILE_PATH);
    ASSERT_NOT_NULL(file);
    const char* part1 = "Hello";
    ASSERT_EQ(anv_file_write(file, (const uint8_t*)part1, strlen(part1)), ANV_RESULT_SUCCESS);
    anv_file_destroy(file);

    // Append more data
    file = anv_file_create(&alloc, TEST_FILE_PATH);
    ASSERT_NOT_NULL(file);
    const char* part2 = ", World!";
    ASSERT_EQ(anv_file_write_append(file, (const uint8_t*)part2, strlen(part2)), ANV_RESULT_SUCCESS);
    anv_file_destroy(file);

    // Read back and verify combined content
    file = anv_file_create(&alloc, TEST_FILE_PATH);
    ASSERT_NOT_NULL(file);
    ASSERT_EQ(anv_file_read(file), ANV_RESULT_SUCCESS);
    ASSERT_EQ(file->size, strlen("Hello, World!"));
    ASSERT_EQ(memcmp(file->contents, "Hello, World!", file->size), 0);

    anv_file_destroy(file);
    cleanup_test_file(TEST_FILE_PATH);
    return TEST_SUCCESS;
}

int test_file_write_append_null_params(void)
{
    ANVAllocator alloc = create_int_allocator();
    ANVFile* file = anv_file_create(&alloc, TEST_FILE_PATH);
    ASSERT_NOT_NULL(file);

    const char* data = "test";
    ASSERT_EQ(anv_file_write_append(NULL, (const uint8_t*)data, 4), ANV_RESULT_INVALID_ARGUMENT);
    ASSERT_EQ(anv_file_write_append(file, NULL, 4), ANV_RESULT_INVALID_ARGUMENT);
    ASSERT_EQ(anv_file_write_append(file, (const uint8_t*)data, 0), ANV_RESULT_INVALID_ARGUMENT);

    anv_file_destroy(file);
    return TEST_SUCCESS;
}

//==============================================================================
// Write-then-Read roundtrip Tests
//==============================================================================

int test_file_write_overwrites(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Write initial data
    ANVFile* file = anv_file_create(&alloc, TEST_FILE_PATH);
    ASSERT_NOT_NULL(file);
    const char* initial = "initial content that is long";
    ASSERT_EQ(anv_file_write(file, (const uint8_t*)initial, strlen(initial)), ANV_RESULT_SUCCESS);
    anv_file_destroy(file);

    // Overwrite with shorter data
    file = anv_file_create(&alloc, TEST_FILE_PATH);
    ASSERT_NOT_NULL(file);
    const char* replacement = "short";
    ASSERT_EQ(anv_file_write(file, (const uint8_t*)replacement, strlen(replacement)), ANV_RESULT_SUCCESS);
    anv_file_destroy(file);

    // Read back and verify only the new data
    file = anv_file_create(&alloc, TEST_FILE_PATH);
    ASSERT_NOT_NULL(file);
    ASSERT_EQ(anv_file_read(file), ANV_RESULT_SUCCESS);
    ASSERT_EQ(file->size, strlen(replacement));
    ASSERT_EQ(memcmp(file->contents, replacement, file->size), 0);

    anv_file_destroy(file);
    cleanup_test_file(TEST_FILE_PATH);
    return TEST_SUCCESS;
}

int test_file_binary_data(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Write binary data with embedded nulls
    uint8_t binary_data[] = {0x00, 0x01, 0x02, 0xFF, 0xFE, 0x00, 0xAB};
    ANVFile* file = anv_file_create(&alloc, TEST_FILE_PATH);
    ASSERT_NOT_NULL(file);
    ASSERT_EQ(anv_file_write(file, binary_data, sizeof(binary_data)), ANV_RESULT_SUCCESS);
    anv_file_destroy(file);

    // Read back
    file = anv_file_create(&alloc, TEST_FILE_PATH);
    ASSERT_NOT_NULL(file);
    ASSERT_EQ(anv_file_read(file), ANV_RESULT_SUCCESS);
    ASSERT_EQ(file->size, sizeof(binary_data));
    ASSERT_EQ(memcmp(file->contents, binary_data, sizeof(binary_data)), 0);

    anv_file_destroy(file);
    cleanup_test_file(TEST_FILE_PATH);
    return TEST_SUCCESS;
}

int test_file_empty_read(void)
{
    ANVAllocator alloc = create_int_allocator();

    // Create an empty file via fopen/fclose
    FILE* fp = fopen(TEST_FILE_PATH, "wb");
    ASSERT_NOT_NULL(fp);
    fclose(fp);

    // Read empty file
    ANVFile* file = anv_file_create(&alloc, TEST_FILE_PATH);
    ASSERT_NOT_NULL(file);
    ASSERT_EQ(anv_file_read(file), ANV_RESULT_SUCCESS);
    ASSERT_EQ(file->size, 0);
    ASSERT_NOT_NULL(file->contents);
    ASSERT_EQ(file->contents[0], '\0');

    anv_file_destroy(file);
    cleanup_test_file(TEST_FILE_PATH);
    return TEST_SUCCESS;
}

//==============================================================================
// Main
//==============================================================================

int main(void)
{
    // Clean up any leftover test files
    cleanup_test_file(TEST_FILE_PATH);
    cleanup_test_file(TEST_FILE_PATH2);

    const ANVTestCase tests[] = {
        // Creation/destruction
        TEST_REGISTER(test_file_create_destroy),
        TEST_REGISTER(test_file_create_null_params),
        TEST_REGISTER(test_file_destroy_null),

        // Write
        TEST_REGISTER(test_file_write_basic),
        TEST_REGISTER(test_file_write_null_params),

        // Read
        TEST_REGISTER(test_file_read_basic),
        TEST_REGISTER(test_file_read_null),
        TEST_REGISTER(test_file_read_nonexistent),

        // Append
        TEST_REGISTER(test_file_write_append_basic),
        TEST_REGISTER(test_file_write_append_null_params),

        // Roundtrip
        TEST_REGISTER(test_file_write_overwrites),
        TEST_REGISTER(test_file_binary_data),
        TEST_REGISTER(test_file_empty_read),
    };

    const int result = anv_run_tests("File", tests, sizeof(tests) / sizeof(tests[0]));

    // Final cleanup
    cleanup_test_file(TEST_FILE_PATH);
    cleanup_test_file(TEST_FILE_PATH2);

    return result;
}