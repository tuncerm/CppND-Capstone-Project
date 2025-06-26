#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enum defining various error codes.
 *
 * This enum categorizes errors that can occur within the application.
 * It helps in identifying the nature of the error without parsing strings.
 */
typedef enum {
    ERR_NONE = 0,
    ERR_UNKNOWN,
    ERR_SDL_INIT,
    ERR_SDL_WINDOW,
    ERR_SDL_RENDERER,
    ERR_SDL_IMAGE_INIT,
    ERR_SDL,
    ERR_FILE_OPEN,
    ERR_FILE_READ,
    ERR_FILE_WRITE,
    ERR_MEMORY_ALLOC,
    ERR_INVALID_ARGUMENT,
    ERR_INVALID_STATE,
    ERR_CONFIG_LOAD,
    ERR_CONFIG_PARSE,
    ERR_RESOURCE_NOT_FOUND
} ErrorCode_t;

/**
 * @brief Structure to hold detailed error information.
 */
typedef struct {
    ErrorCode_t code;
    char message[256];
    char file[256];
    int line;
} Error_t;

/**
 * @brief Sets the global error state.
 *
 * This function should be called when an error occurs. It records the error code,
 * a descriptive message, and the location of the error.
 *
 * @param code The error code from ErrorCode_t.
 * @param file The file where the error occurred (__FILE__).
 * @param line The line number where the error occurred (__LINE__).
 * @param format The formatted error message string.
 * @param ... Variable arguments for the format string.
 */
void ErrorHandler_Set(ErrorCode_t code, const char* file, int line, const char* format, ...);

/**
 * @brief Retrieves the last error that occurred.
 *
 * @return A constant pointer to the last recorded Error_t struct.
 *         Returns NULL if no error has been set.
 */
const Error_t* ErrorHandler_Get(void);

/**
 * @brief Checks if an error has been recorded.
 *
 * @return true if an error is currently set, false otherwise.
 */
bool ErrorHandler_HasError(void);

/**
 * @brief Clears the global error state.
 *
 * This should be called after an error has been handled to reset the state.
 */
void ErrorHandler_Clear(void);

/**
 * @brief Logs the current error to stderr.
 *
 * If an error is set, it prints a formatted message to the standard error stream.
 */
void ErrorHandler_Log(void);

#ifdef __cplusplus
}
#endif

#endif  // ERROR_HANDLER_H
