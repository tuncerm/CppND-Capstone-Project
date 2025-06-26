#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * File Utilities for Shared Component Library
 *
 * Provides common file operations and utilities used across applications.
 * Includes file existence checking, path manipulation, and safe file operations.
 */

#define MAX_PATH_LENGTH 512
#define MAX_FILENAME_LENGTH 256

/**
 * Check if file exists and is readable
 *
 * @param filepath Path to file to check
 * @return true if file exists and is readable
 */
bool file_exists(const char* filepath);

/**
 * Check if directory exists
 *
 * @param dirpath Path to directory to check
 * @return true if directory exists
 */
bool dir_exists(const char* dirpath);

/**
 * Get file size in bytes
 *
 * @param filepath Path to file
 * @return File size in bytes, -1 on error
 */
long file_get_size(const char* filepath);

/**
 * Extract filename from full path
 *
 * @param filepath Full file path
 * @param filename Output buffer for filename
 * @param filename_size Size of filename buffer
 * @return true if successful, false on error
 */
bool file_get_filename(const char* filepath, char* filename, size_t filename_size);

/**
 * Extract directory from full path
 *
 * @param filepath Full file path
 * @param directory Output buffer for directory
 * @param directory_size Size of directory buffer
 * @return true if successful, false on error
 */
bool file_get_directory(const char* filepath, char* directory, size_t directory_size);

/**
 * Extract file extension from path
 *
 * @param filepath File path
 * @param extension Output buffer for extension (without dot)
 * @param extension_size Size of extension buffer
 * @return true if successful, false if no extension found
 */
bool file_get_extension(const char* filepath, char* extension, size_t extension_size);

/**
 * Change file extension
 *
 * @param filepath Original file path
 * @param new_extension New extension (without dot)
 * @param output Output buffer for new path
 * @param output_size Size of output buffer
 * @return true if successful, false on error
 */
bool file_change_extension(const char* filepath, const char* new_extension, char* output,
                           size_t output_size);

/**
 * Join directory and filename into full path
 *
 * @param directory Directory path
 * @param filename File name
 * @param output Output buffer for full path
 * @param output_size Size of output buffer
 * @return true if successful, false on error
 */
bool file_join_path(const char* directory, const char* filename, char* output, size_t output_size);

/**
 * Create backup of file by copying to .bak extension
 *
 * @param filepath Path to file to backup
 * @return true if successful, false on error
 */
bool file_create_backup(const char* filepath);

/**
 * Copy file from source to destination
 *
 * @param src_path Source file path
 * @param dest_path Destination file path
 * @return true if successful, false on error
 */
bool file_copy(const char* src_path, const char* dest_path);

/**
 * Safely write data to file with atomic operation
 * Writes to temporary file first, then renames to target
 *
 * @param filepath Target file path
 * @param data Data to write
 * @param size Size of data in bytes
 * @return true if successful, false on error
 */
bool file_write_atomic(const char* filepath, const void* data, size_t size);

/**
 * Read entire file into buffer
 * Caller is responsible for freeing returned buffer
 *
 * @param filepath Path to file to read
 * @param size Output: size of data read
 * @return Pointer to allocated buffer containing file data, NULL on error
 */
void* file_read_all(const char* filepath, size_t* size);

/**
 * Check if filename has valid characters
 *
 * @param filename Filename to validate
 * @return true if filename is valid
 */
bool file_is_valid_filename(const char* filename);

/**
 * Sanitize filename by replacing invalid characters
 *
 * @param filename Input filename
 * @param output Output buffer for sanitized filename
 * @param output_size Size of output buffer
 * @return true if successful, false on error
 */
bool file_sanitize_filename(const char* filename, char* output, size_t output_size);

/**
 * Get temporary file path
 *
 * @param prefix Prefix for temporary filename
 * @param output Output buffer for temporary path
 * @param output_size Size of output buffer
 * @return true if successful, false on error
 */
bool file_get_temp_path(const char* prefix, char* output, size_t output_size);

#ifdef __cplusplus
}
#endif

#endif  // FILE_UTILS_H
