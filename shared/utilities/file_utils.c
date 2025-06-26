#include "file_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include "../constants.h"

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#define PATH_SEPARATOR '\\'
#define PATH_SEPARATOR_STR "\\"
#else
#include <unistd.h>
#define PATH_SEPARATOR '/'
#define PATH_SEPARATOR_STR "/"
#endif

/**
 * Check if file exists and is readable
 */
bool file_exists(const char* filepath) {
    if (!filepath) {
        return false;
    }

    FILE* file = fopen(filepath, "r");
    if (file) {
        fclose(file);
        return true;
    }
    return false;
}

/**
 * Check if directory exists
 */
bool dir_exists(const char* dirpath) {
    if (!dirpath) {
        return false;
    }

    struct stat st;
    return stat(dirpath, &st) == 0 && (st.st_mode & S_IFDIR);
}

/**
 * Get file size in bytes
 */
long file_get_size(const char* filepath) {
    if (!filepath) {
        return -1;
    }

    FILE* file = fopen(filepath, "rb");
    if (!file) {
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fclose(file);

    return size;
}

/**
 * Extract filename from full path
 */
bool file_get_filename(const char* filepath, char* filename, size_t filename_size) {
    if (!filepath || !filename || filename_size == 0) {
        return false;
    }

    const char* last_separator = strrchr(filepath, PATH_SEPARATOR);
    const char* name_start = last_separator ? last_separator + 1 : filepath;

    size_t name_len = strlen(name_start);
    if (name_len >= filename_size) {
        return false;
    }

    strcpy(filename, name_start);
    return true;
}

/**
 * Extract directory from full path
 */
bool file_get_directory(const char* filepath, char* directory, size_t directory_size) {
    if (!filepath || !directory || directory_size == 0) {
        return false;
    }

    const char* last_separator = strrchr(filepath, PATH_SEPARATOR);
    if (!last_separator) {
        // No directory separator, use current directory
        if (directory_size >= 2) {
            strcpy(directory, ".");
            return true;
        }
        return false;
    }

    size_t dir_len = last_separator - filepath;
    if (dir_len >= directory_size) {
        return false;
    }

    strncpy(directory, filepath, dir_len);
    directory[dir_len] = '\0';
    return true;
}

/**
 * Extract file extension from path
 */
bool file_get_extension(const char* filepath, char* extension, size_t extension_size) {
    if (!filepath || !extension || extension_size == 0) {
        return false;
    }

    const char* last_dot = strrchr(filepath, '.');
    const char* last_separator = strrchr(filepath, PATH_SEPARATOR);

    // Make sure the dot is after the last path separator (not in directory name)
    if (!last_dot || (last_separator && last_dot < last_separator)) {
        extension[0] = '\0';
        return false;
    }

    const char* ext_start = last_dot + 1;
    size_t ext_len = strlen(ext_start);
    if (ext_len >= extension_size) {
        return false;
    }

    strcpy(extension, ext_start);
    return true;
}

/**
 * Change file extension
 */
bool file_change_extension(const char* filepath, const char* new_extension, char* output,
                           size_t output_size) {
    if (!filepath || !new_extension || !output || output_size == 0) {
        return false;
    }

    const char* last_dot = strrchr(filepath, '.');
    const char* last_separator = strrchr(filepath, PATH_SEPARATOR);

    // Calculate base name length (without extension)
    size_t base_len;
    if (last_dot && (!last_separator || last_dot > last_separator)) {
        base_len = last_dot - filepath;
    } else {
        base_len = strlen(filepath);
    }

    // Check if result will fit
    size_t new_len = base_len + 1 + strlen(new_extension);  // +1 for dot
    if (new_len >= output_size) {
        return false;
    }

    // Copy base name
    strncpy(output, filepath, base_len);
    output[base_len] = '\0';

    // Add new extension
    strcat(output, ".");
    strcat(output, new_extension);

    return true;
}

/**
 * Join directory and filename into full path
 */
bool file_join_path(const char* directory, const char* filename, char* output, size_t output_size) {
    if (!directory || !filename || !output || output_size == 0) {
        return false;
    }

    size_t dir_len = strlen(directory);
    size_t file_len = strlen(filename);
    size_t total_len = dir_len + 1 + file_len;  // +1 for separator

    if (total_len >= output_size) {
        return false;
    }

    strcpy(output, directory);

    // Add separator if directory doesn't end with one
    if (dir_len > 0 && directory[dir_len - 1] != PATH_SEPARATOR) {
        strcat(output, PATH_SEPARATOR_STR);
    }

    strcat(output, filename);
    return true;
}

/**
 * Create backup of file by copying to .bak extension
 */
bool file_create_backup(const char* filepath) {
    if (!filepath || !file_exists(filepath)) {
        return false;
    }

    char backup_path[MAX_PATH_LENGTH];
    if (!file_change_extension(filepath, "bak", backup_path, sizeof(backup_path))) {
        return false;
    }

    return file_copy(filepath, backup_path);
}

/**
 * Copy file from source to destination
 */
bool file_copy(const char* src_path, const char* dest_path) {
    if (!src_path || !dest_path) {
        return false;
    }

    FILE* src = fopen(src_path, "rb");
    if (!src) {
        return false;
    }

    FILE* dest = fopen(dest_path, "wb");
    if (!dest) {
        fclose(src);
        return false;
    }

    char buffer[FILE_BUFFER_SIZE];
    size_t bytes_read;
    bool success = true;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        if (fwrite(buffer, 1, bytes_read, dest) != bytes_read) {
            success = false;
            break;
        }
    }

    fclose(src);
    fclose(dest);

    return success;
}

/**
 * Safely write data to file with atomic operation
 */
bool file_write_atomic(const char* filepath, const void* data, size_t size) {
    if (!filepath || !data) {
        return false;
    }

    char temp_path[MAX_PATH_LENGTH];
    if (!file_get_temp_path("tmp", temp_path, sizeof(temp_path))) {
        return false;
    }

    // Write to temporary file first
    FILE* temp_file = fopen(temp_path, "wb");
    if (!temp_file) {
        return false;
    }

    bool success = (fwrite(data, 1, size, temp_file) == size);
    fclose(temp_file);

    if (!success) {
        remove(temp_path);
        return false;
    }

    // Atomic rename
#ifdef _WIN32
    // On Windows, need to remove destination first
    remove(filepath);
    success = (rename(temp_path, filepath) == 0);
#else
    success = (rename(temp_path, filepath) == 0);
#endif

    if (!success) {
        remove(temp_path);
    }

    return success;
}

/**
 * Read entire file into buffer
 */
void* file_read_all(const char* filepath, size_t* size) {
    if (!filepath || !size) {
        return NULL;
    }

    FILE* file = fopen(filepath, "rb");
    if (!file) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size <= 0) {
        fclose(file);
        return NULL;
    }

    void* buffer = malloc(file_size);
    if (!buffer) {
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1, file_size, file);
    fclose(file);

    if (bytes_read != (size_t)file_size) {
        free(buffer);
        return NULL;
    }

    *size = bytes_read;
    return buffer;
}

/**
 * Check if filename has valid characters
 */
bool file_is_valid_filename(const char* filename) {
    if (!filename || strlen(filename) == 0) {
        return false;
    }

    // Check for invalid characters
    const char* invalid_chars = "<>:\"|?*";
    for (const char* p = filename; *p; p++) {
        if (strchr(invalid_chars, *p) || *p < MIN_ASCII) {
            return false;
        }
    }

    // Check for reserved names on Windows
#ifdef _WIN32
    const char* reserved[] = {"CON",  "PRN",  "AUX",  "NUL",  "COM1", "COM2", "COM3", "COM4",
                              "COM5", "COM6", "COM7", "COM8", "COM9", "LPT1", "LPT2", "LPT3",
                              "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"};

    for (size_t i = 0; i < sizeof(reserved) / sizeof(reserved[0]); i++) {
        if (strcmp(filename, reserved[i]) == 0) {
            return false;
        }
    }
#endif

    return true;
}

/**
 * Sanitize filename by replacing invalid characters
 */
bool file_sanitize_filename(const char* filename, char* output, size_t output_size) {
    if (!filename || !output || output_size == 0) {
        return false;
    }

    size_t len = strlen(filename);
    if (len >= output_size) {
        return false;
    }

    const char* invalid_chars = "<>:\"|?*";
    for (size_t i = 0; i <= len; i++) {
        char c = filename[i];
        if (c < MIN_ASCII || strchr(invalid_chars, c)) {
            output[i] = '_';
        } else {
            output[i] = c;
        }
    }

    return true;
}

/**
 * Get temporary file path
 */
bool file_get_temp_path(const char* prefix, char* output, size_t output_size) {
    if (!prefix || !output || output_size == 0) {
        return false;
    }

    time_t now = time(NULL);
    snprintf(output, output_size, "%s_%ld.tmp", prefix, (long)now);

    return true;
}
