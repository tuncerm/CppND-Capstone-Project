# Error Handler Module

This module provides a centralized, C-compatible error handling mechanism for the project. It is designed to be used across both C and C++ codebases to ensure consistent error reporting and management.

## Features

- **Unified Error Codes**: A single `ErrorCode_t` enum for categorizing all application errors.
- **Detailed Error Information**: The `Error_t` struct captures the error code, a descriptive message, and the source file/line number.
- **Global Error State**: A simple, global error state that can be set, checked, and cleared.
- **C/C++ Compatibility**: `extern "C"` linkage allows seamless use from C++ source files.
- **Simple Logging**: A utility function to print the current error to `stderr`.

## How to Use

### Setting an Error

When a function encounters an error, it should use `ErrorHandler_Set` to record it.

```c
#include "error_handler.h"
#include <stdio.h>

bool load_resource(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) {
        // Set the error with a code, file, line, and message
        ErrorHandler_Set(ERR_FILE_OPEN, __FILE__, __LINE__, "Failed to open resource at '%s'", path);
        return false;
    }
    // ... proceed with file ...
    fclose(f);
    return true;
}
```

### Checking for an Error

The caller of a function can check if an error occurred.

```c
#include "error_handler.h"

void setup_game() {
    if (!load_resource("assets/player.dat")) {
        // An error occurred, log it and handle it
        ErrorHandler_Log();
        // Potentially exit or try to recover
        exit(1);
    }
}
```

### Error Propagation

Functions should propagate errors up the call stack by returning a status (e.g., `bool` or a pointer) and letting the caller check `ErrorHandler_HasError()`.

### Clearing the Error

Once an error has been handled, the error state should be cleared.

```c
if (ErrorHandler_HasError()) {
    ErrorHandler_Log();
    // ... recovery logic ...
    ErrorHandler_Clear(); // Reset for the next operation
}
```

## C++ Integration

The header is C++-aware. You can include it directly in `.cpp` files.

```cpp
#include "shared/error_handler/error_handler.h"

void Game::initialize() {
    // ... C++ code ...
    if (!some_c_function_that_can_fail()) {
        const Error_t* err = ErrorHandler_Get();
        if (err) {
            // Log or throw an exception
            throw std::runtime_error(err->message);
        }
    }
}
```

This system provides a bridge between C-style error codes and C++ exceptions, allowing for a consistent strategy across the entire application.
