/**
 * Basic Example - Shared Components Library
 *
 * Minimal example demonstrating:
 * - Library initialization
 * - SDL context setup
 * - Text rendering
 * - Proper cleanup
 */

#include <shared_components.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    printf("Basic Example - Shared Components Library v%s\n", shared_components_get_version());

    // Initialize shared components library
    if (!shared_components_init()) {
        fprintf(stderr, "Failed to initialize shared components\n");
        return 1;
    }

    // Create SDL context
    SDLContext sdl_ctx;
    if (!sdl_init_context_simple(&sdl_ctx, "Basic Example", 400, 300)) {
        fprintf(stderr, "SDL initialization failed: %s\n", sdl_get_error());
        shared_components_cleanup();
        return 1;
    }

    // Initialize text renderer
    TextRenderer text_renderer;
    if (!text_renderer_init(&text_renderer, sdl_get_renderer(&sdl_ctx))) {
        fprintf(stderr, "Text renderer initialization failed\n");
        sdl_cleanup_context(&sdl_ctx);
        shared_components_cleanup();
        return 1;
    }

    // Set up colors
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color black = {0, 0, 0, 255};
    SDL_Color red = {255, 0, 0, 255};
    SDL_Color green = {0, 255, 0, 255};
    SDL_Color blue = {0, 0, 255, 255};

    // Main loop
    bool running = true;
    SDL_Event event;

    printf("Displaying text for 5 seconds...\n");
    Uint64 start_time = SDL_GetTicks();

    while (running && (SDL_GetTicks() - start_time) < 5000) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_KEY_DOWN) {
                running = false;
            }
        }

        // Clear screen
        sdl_clear_screen(&sdl_ctx, black);

        // Render text
        text_render_string(&text_renderer, "Hello, World!", 50, 50, white);
        text_render_string(&text_renderer, "Shared Components", 50, 70, green);
        text_render_string(&text_renderer, "Press any key to exit", 50, 90, blue);

        // Render version info
        text_render_string(&text_renderer, "Version: 1.0.0", 10, 10, red);

        // Calculate and display text dimensions
        int text_width, text_height;
        text_get_dimensions("Hello, World!", &text_width, &text_height);

        char size_info[64];
        snprintf(size_info, sizeof(size_info), "Text size: %dx%d", text_width, text_height);
        text_render_string(&text_renderer, size_info, 10, 250, white);

        // Present frame
        sdl_present(&sdl_ctx);

        // Small delay to avoid using too much CPU
        SDL_Delay(16);  // ~60 FPS
    }

    printf("Example completed successfully!\n");

    // Cleanup (in reverse order of initialization)
    text_renderer_cleanup(&text_renderer);
    sdl_cleanup_context(&sdl_ctx);
    shared_components_cleanup();

    return 0;
}
