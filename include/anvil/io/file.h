//
// Created by zack on 10/9/25.
//

#ifndef ANVIL_FILE_H
#define ANVIL_FILE_H

#include <stdio.h>

#include "anvil/common.h"
#include "anvil/containers/dynamicstring.h"

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Type definitions
//==============================================================================

/**
 * File structure for managing file I/O operations.
 */
typedef struct ANVFile
{
    FILE *handle;           // Internal file handle (NULL when not actively reading/writing)
    ANVString path;         // Path to the file
    uint8_t *contents;      // Buffer containing file contents (populated by anv_file_read)
    size_t size;            // Size of file contents in bytes
    ANVAllocator allocator; // Custom allocator for memory management
} ANVFile;

//==============================================================================
// Creation and destruction functions
//==============================================================================

/**
 * Create a new file object.
 *
 * @param alloc The custom allocator to use for memory management (must not be NULL)
 * @param path Path to the file (must not be NULL)
 * @return Pointer to new ANVFile, or NULL on failure
 */
ANV_API ANVFile* anv_file_create(ANVAllocator *alloc, const char* path);

/**
 * Destroy the file object and free all associated resources.
 *
 * This function closes any open file handle, frees the contents buffer,
 * destroys the path string, and deallocates the file structure itself.
 *
 * @param file The file object to destroy (can be NULL)
 * @return ANV_RESULT_SUCCESS on success, ANV_RESULT_INVALID_ARGUMENT if file is NULL
 */
ANV_API ANVResult anv_file_destroy(ANVFile* file);

//==============================================================================
// File I/O operations
//==============================================================================

/**
 * Read the entire file contents into memory.
 *
 * This function opens the file in binary read mode, reads all contents into
 * a buffer, and null-terminates the buffer.
 * The file handle is closed after reading. The contents remain accessible via
 * file->contents until the file is destroyed or read again.
 *
 * @param file The file object to read from (must not be NULL)
 * @return ANV_RESULT_SUCCESS on success
 *         ANV_RESULT_INVALID_ARGUMENT if file is NULL
 *         ANV_RESULT_NOT_FOUND if file cannot be opened
 *         ANV_RESULT_OUT_OF_BOUNDS if file size cannot be determined
 *         ANV_RESULT_OUT_OF_MEMORY if buffer allocation fails
 *         ANV_RESULT_INSUFFICIENT_SPACE if read operation fails
 */
ANV_API ANVResult anv_file_read(ANVFile* file);

/**
 * Write data to the file, replacing any existing contents.
 *
 * This function opens the file in binary write mode (truncating existing content),
 * writes the provided data, and closes the file handle.
 *
 * @param file The file object to write to (must not be NULL)
 * @param data Pointer to data buffer to write (must not be NULL)
 * @param size Number of bytes to write (must be greater than 0)
 * @return ANV_RESULT_SUCCESS on success
 *         ANV_RESULT_INVALID_ARGUMENT if file, data is NULL or size is 0
 *         ANV_RESULT_NOT_FOUND if file cannot be opened
 *         ANV_RESULT_INSUFFICIENT_SPACE if write operation fails
 */
ANV_API ANVResult anv_file_write(ANVFile* file, const uint8_t* data, size_t size);

/**
 * Append data to the end of the file.
 *
 * This function opens the file in binary append mode, writes the provided data
 * to the end of the file, and closes the file handle. Existing file contents
 * are preserved.
 *
 * @param file The file object to write to (must not be NULL)
 * @param data Pointer to data buffer to append (must not be NULL)
 * @param size Number of bytes to append (must be greater than 0)
 * @return ANV_RESULT_SUCCESS on success
 *         ANV_RESULT_INVALID_ARGUMENT if file, data is NULL or size is 0
 *         ANV_RESULT_NOT_FOUND if file cannot be opened
 *         ANV_RESULT_INSUFFICIENT_SPACE if write operation fails
 */
ANV_API ANVResult anv_file_write_append(ANVFile* file, const uint8_t* data, size_t size);

#ifdef __cplusplus
}
#endif

#endif //ANVIL_FILE_H