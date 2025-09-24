#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include "config.h"
#include "palette.h"
#include "ui.h"

/**
 * Main application entry point
 * Initializes SDL, creates UI, and runs the main event loop
 */
int main(int argc, char* argv[]) {
    (void)argv;  // Suppress unused parameter warning

    // Load configuration
    AppConfig config;
    if (!load_app_config(&config, "../config/palette_maker_config.json")) {
        fprintf(stderr, "FATAL: Could not load application configuration.\n");
        return 1;
    }

    // Initialize SDL3
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        printf("Error: Could not initialize SDL3: %s\n", SDL_GetError());
        return 1;
    }

    printf("SDL3 initialized successfully\n");
    printf("%s\n", config.window_title);
    printf("Controls:\n");
    printf("  - Click swatch to select\n");
    printf("  - Double-click swatch to open color picker\n");
    printf("  - S: Save palette\n");
    printf("  - Ctrl+S: Quick save\n");
    printf("  - L or Ctrl+L: Load palette\n");
    printf("  - ESC: Close dialogs or quit\n");
    printf("  - Enter: Confirm dialog actions\n");
    printf("\n");

    // Initialize palette with default colors
    Palette palette;
    palette_init(&palette, &config);

    palette_load(&palette, config.default_file);

    // Initialize UI system
    UIState ui;
    if (!ui_init(&ui, &config)) {
        printf("Error: Failed to initialize UI system\n");
        SDL_Quit();
        return 1;
    }

    // Main event loop
    bool running = true;
    SDL_Event event;

    printf("Starting main event loop...\n");

    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            running = ui_handle_event(&ui, &palette, &event, &config);
            if (!running)
                break;
        }

        // Render frame
        ui_render(&ui, &palette, &config);

        // Small delay to prevent excessive CPU usage
        SDL_Delay(config.frame_delay_ms);
    }

    // Check for unsaved changes before exiting
    if (palette_is_modified(&palette)) {
        printf("\nWarning: You have unsaved changes!\n");
        printf("Your palette has been modified but not saved.\n");

        // In a complete implementation, you might want to:
        // 1. Show a save dialog
        // 2. Auto-save to a temporary file
        // 3. Prompt the user via console input

        // For now, we'll just warn the user
        printf("Consider saving your work before closing.\n");
    }

    // Cleanup
    printf("Cleaning up resources...\n");
    ui_cleanup(&ui);
    SDL_Quit();

    printf("Palette Maker closed successfully\n");
    return 0;
}
