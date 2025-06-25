#ifndef SDL_CONTEXT_H
#define SDL_CONTEXT_H

#include <SDL3/SDL.h>
#include <stdbool.h>

/**
 * SDL Framework for Shared Component Library
 *
 * Provides unified SDL3 initialization and context management
 * extracted from common patterns in palette-maker and tile-maker.
 * Handles window creation, renderer setup, and cleanup.
 */

/**
 * SDL context structure
 * Manages SDL window, renderer, and associated state
 */
typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    int width;
    int height;
    char title[128];
    bool initialized;
    bool vsync_enabled;
} SDLContext;

/**
 * SDL initialization configuration
 */
typedef struct {
    const char* title;
    int width;
    int height;
    bool resizable;
    bool vsync;
    bool fullscreen;
} SDLContextConfig;

// ===== Core SDL Functions =====

/**
 * Initialize SDL context with specified configuration
 *
 * @param ctx SDL context to initialize
 * @param config Configuration parameters
 * @return true if successful, false on error
 */
bool sdl_init_context(SDLContext* ctx, const SDLContextConfig* config);

/**
 * Initialize SDL context with default configuration
 *
 * @param ctx SDL context to initialize
 * @param title Window title
 * @param width Window width
 * @param height Window height
 * @return true if successful, false on error
 */
bool sdl_init_context_simple(SDLContext* ctx, const char* title, int width, int height);

/**
 * Cleanup SDL context and free resources
 *
 * @param ctx SDL context to cleanup
 */
void sdl_cleanup_context(SDLContext* ctx);

/**
 * Check if SDL context is properly initialized
 *
 * @param ctx SDL context to check
 * @return true if initialized and ready to use
 */
bool sdl_context_is_ready(const SDLContext* ctx);

/**
 * Get window from SDL context
 *
 * @param ctx SDL context
 * @return SDL window pointer, NULL if not initialized
 */
SDL_Window* sdl_get_window(const SDLContext* ctx);

/**
 * Get renderer from SDL context
 *
 * @param ctx SDL context
 * @return SDL renderer pointer, NULL if not initialized
 */
SDL_Renderer* sdl_get_renderer(const SDLContext* ctx);

// ===== Window Management =====

/**
 * Set window title
 *
 * @param ctx SDL context
 * @param title New window title
 */
void sdl_set_window_title(SDLContext* ctx, const char* title);

/**
 * Get current window size
 *
 * @param ctx SDL context
 * @param width Output: current width
 * @param height Output: current height
 */
void sdl_get_window_size(const SDLContext* ctx, int* width, int* height);

/**
 * Set window size
 *
 * @param ctx SDL context
 * @param width New width
 * @param height New height
 */
void sdl_set_window_size(SDLContext* ctx, int width, int height);

/**
 * Toggle fullscreen mode
 *
 * @param ctx SDL context
 * @param fullscreen true for fullscreen, false for windowed
 * @return true if successful, false on error
 */
bool sdl_set_fullscreen(SDLContext* ctx, bool fullscreen);

// ===== Rendering Utilities =====

/**
 * Clear screen with specified color
 *
 * @param ctx SDL context
 * @param color Clear color
 */
void sdl_clear_screen(SDLContext* ctx, SDL_Color color);

/**
 * Present rendered frame
 *
 * @param ctx SDL context
 */
void sdl_present(SDLContext* ctx);

/**
 * Set logical presentation for consistent UI scaling
 *
 * @param ctx SDL context
 * @param width Logical width
 * @param height Logical height
 * @return true if successful, false on error
 */
bool sdl_set_logical_presentation(SDLContext* ctx, int width, int height);

// ===== Utility Functions =====

/**
 * Get SDL error string
 *
 * @return Current SDL error message
 */
const char* sdl_get_error(void);

/**
 * Print SDL error with custom message
 *
 * @param message Custom error message prefix
 */
void sdl_print_error(const char* message);

/**
 * Check if SDL subsystem is initialized
 *
 * @param flags SDL subsystem flags to check
 * @return true if all specified subsystems are initialized
 */
bool sdl_is_subsystem_initialized(Uint32 flags);

/**
 * Get SDL version information
 *
 * @param major Output: major version
 * @param minor Output: minor version
 * @param patch Output: patch version
 */
void sdl_get_version(int* major, int* minor, int* patch);

#endif  // SDL_CONTEXT_H
