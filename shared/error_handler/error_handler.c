#include "error_handler.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// Global error state, private to this module.
static Error_t g_last_error = {ERR_NONE, "", "", 0};
static bool g_error_set = false;

void ErrorHandler_Set(ErrorCode_t code, const char* file, int line, const char* format, ...) {
    g_last_error.code = code;
    strncpy(g_last_error.file, file, sizeof(g_last_error.file) - 1);
    g_last_error.file[sizeof(g_last_error.file) - 1] = '\0';
    g_last_error.line = line;

    va_list args;
    va_start(args, format);
    vsnprintf(g_last_error.message, sizeof(g_last_error.message), format, args);
    va_end(args);

    g_error_set = true;
}

const Error_t* ErrorHandler_Get(void) {
    return g_error_set ? &g_last_error : NULL;
}

bool ErrorHandler_HasError(void) {
    return g_error_set;
}

void ErrorHandler_Clear(void) {
    g_error_set = false;
    g_last_error.code = ERR_NONE;
    g_last_error.message[0] = '\0';
    g_last_error.file[0] = '\0';
    g_last_error.line = 0;
}

void ErrorHandler_Log(void) {
    if (g_error_set) {
        fprintf(stderr, "[ERROR %d] in %s:%d: %s\n", g_last_error.code, g_last_error.file,
                g_last_error.line, g_last_error.message);
    }
}
