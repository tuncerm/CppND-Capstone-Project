#include "sdl_context.h"
#include <stdio.h>
#include <string.h>

/**
 * Initialize SDL context with specified configuration
 */
bool sdl_init_context(SDLContext* ctx, const SDLContextConfig* config) {
    if (!ctx || !config) {
        return false;
    }

    // Initialize context structure
    memset(ctx, 0, sizeof(SDLContext));
    ctx->width = config->width;
    ctx->height = config->height;
    ctx->vsync_enabled = config->vsync;

    if (config->title) {
        strncpy(ctx->title, config->title, sizeof(ctx->title) - 1);
        ctx->title[sizeof(ctx->title) - 1] = '\0';
    } else {
        strcpy(ctx->title, "SDL Application");
    }

    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("Error: Could not initialize SDL: %s\n", SDL_GetError());
        return false;
    }

    // Create window
    Uint32 window_flags = 0;
    if (config->resizable) {
        window_flags |= SDL_WINDOW_RESIZABLE;
    }
    if (config->fullscreen) {
        window_flags |= SDL_WINDOW_FULLSCREEN;
    }

    ctx->window = SDL_CreateWindow(ctx->title, ctx->width, ctx->height, window_flags);
    if (!ctx->window) {
        printf("Error: Could not create window: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }

    // Create renderer
    const char* renderer_name = NULL;
    if (ctx->vsync_enabled) {
        renderer_name = NULL;  // Let SDL choose the best renderer with VSync
    }

    ctx->renderer = SDL_CreateRenderer(ctx->window, renderer_name);
    if (!ctx->renderer) {
        printf("Error: Could not create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(ctx->window);
        SDL_Quit();
        return false;
    }

    // Set logical presentation for consistent UI scaling
    if (!sdl_set_logical_presentation(ctx, ctx->width, ctx->height)) {
        printf("Warning: Could not set logical presentation\n");
        // Continue anyway - not critical
    }

    ctx->initialized = true;
    printf("SDL context initialized successfully: %dx%d '%s'\n", ctx->width, ctx->height,
           ctx->title);

    return true;
}

/**
 * Initialize SDL context with default configuration
 */
bool sdl_init_context_simple(SDLContext* ctx, const char* title, int width, int height) {
    SDLContextConfig config = {.title = title,
                               .width = width,
                               .height = height,
                               .resizable = true,
                               .vsync = true,
                               .fullscreen = false};

    return sdl_init_context(ctx, &config);
}

/**
 * Cleanup SDL context and free resources
 */
void sdl_cleanup_context(SDLContext* ctx) {
    if (!ctx) {
        return;
    }

    if (ctx->renderer) {
        SDL_DestroyRenderer(ctx->renderer);
        ctx->renderer = NULL;
    }

    if (ctx->window) {
        SDL_DestroyWindow(ctx->window);
        ctx->window = NULL;
    }

    if (ctx->initialized) {
        SDL_Quit();
        ctx->initialized = false;
    }

    printf("SDL context cleaned up\n");
}

/**
 * Check if SDL context is properly initialized
 */
bool sdl_context_is_ready(const SDLContext* ctx) {
    return ctx && ctx->initialized && ctx->window && ctx->renderer;
}

/**
 * Get window from SDL context
 */
SDL_Window* sdl_get_window(const SDLContext* ctx) {
    return ctx ? ctx->window : NULL;
}

/**
 * Get renderer from SDL context
 */
SDL_Renderer* sdl_get_renderer(const SDLContext* ctx) {
    return ctx ? ctx->renderer : NULL;
}

/**
 * Set window title
 */
void sdl_set_window_title(SDLContext* ctx, const char* title) {
    if (!ctx || !ctx->window || !title) {
        return;
    }

    SDL_SetWindowTitle(ctx->window, title);
    strncpy(ctx->title, title, sizeof(ctx->title) - 1);
    ctx->title[sizeof(ctx->title) - 1] = '\0';
}

/**
 * Get current window size
 */
void sdl_get_window_size(const SDLContext* ctx, int* width, int* height) {
    if (!ctx || !ctx->window) {
        if (width)
            *width = 0;
        if (height)
            *height = 0;
        return;
    }

    SDL_GetWindowSize(ctx->window, width, height);
}

/**
 * Set window size
 */
void sdl_set_window_size(SDLContext* ctx, int width, int height) {
    if (!ctx || !ctx->window) {
        return;
    }

    SDL_SetWindowSize(ctx->window, width, height);
    ctx->width = width;
    ctx->height = height;
}

/**
 * Toggle fullscreen mode
 */
bool sdl_set_fullscreen(SDLContext* ctx, bool fullscreen) {
    if (!ctx || !ctx->window) {
        return false;
    }

    Uint32 flags = fullscreen ? SDL_WINDOW_FULLSCREEN : 0;
    return SDL_SetWindowFullscreen(ctx->window, flags) == 0;
}

/**
 * Clear screen with specified color
 */
void sdl_clear_screen(SDLContext* ctx, SDL_Color color) {
    if (!ctx || !ctx->renderer) {
        return;
    }

    SDL_SetRenderDrawColor(ctx->renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(ctx->renderer);
}

/**
 * Present rendered frame
 */
void sdl_present(SDLContext* ctx) {
    if (!ctx || !ctx->renderer) {
        return;
    }

    SDL_RenderPresent(ctx->renderer);
}

/**
 * Set logical presentation for consistent UI scaling
 */
bool sdl_set_logical_presentation(SDLContext* ctx, int width, int height) {
    if (!ctx || !ctx->renderer) {
        return false;
    }

    return SDL_SetRenderLogicalPresentation(ctx->renderer, width, height,
                                            SDL_LOGICAL_PRESENTATION_LETTERBOX) == 0;
}

/**
 * Get SDL error string
 */
const char* sdl_get_error(void) {
    return SDL_GetError();
}

/**
 * Print SDL error with custom message
 */
void sdl_print_error(const char* message) {
    if (message) {
        printf("%s: %s\n", message, SDL_GetError());
    } else {
        printf("SDL Error: %s\n", SDL_GetError());
    }
}

/**
 * Check if SDL subsystem is initialized
 */
bool sdl_is_subsystem_initialized(Uint32 flags) {
    return (SDL_WasInit(flags) & flags) == flags;
}

/**
 * Get SDL version information
 */
void sdl_get_version(int* major, int* minor, int* patch) {
    int version = SDL_GetVersion();

    if (major)
        *major = SDL_VERSIONNUM_MAJOR(version);
    if (minor)
        *minor = SDL_VERSIONNUM_MINOR(version);
    if (patch)
        *patch = SDL_VERSIONNUM_MICRO(version);
}
