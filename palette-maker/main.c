#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include "palette.h"
#include "ui.h"

/**
 * Main application entry point
 * Initializes SDL, creates UI, and runs the main event loop
 */
int main(int argc, char* argv[]) {
    (void)argc;  // Suppress unused parameter warning
    (void)argv;

    // Initialize SDL3
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        printf("Error: Could not initialize SDL3: %s\n", SDL_GetError());
        return 1;
    }

    printf("SDL3 initialized successfully\n");
    printf("Palette Maker v1.0.0 - SDL3 Edition\n");
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
    palette_init(&palette);

    // Initialize UI system
    UIState ui;
    if (!ui_init(&ui)) {
        printf("Error: Failed to initialize UI system\n");
        SDL_Quit();
        return 1;
    }

    // Load palette from command line argument if provided
    if (argc > 1) {
        if (palette_load(&palette, argv[1])) {
            printf("Loaded palette from: %s\n", argv[1]);
        } else {
            printf("Warning: Could not load palette from: %s\n", argv[1]);
        }
    }

    // Main event loop
    bool running = true;
    SDL_Event event;

    printf("Starting main event loop...\n");

    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            running = ui_handle_event(&ui, &palette, &event);
            if (!running)
                break;
        }

        // Render frame
        ui_render(&ui, &palette);

        // Small delay to prevent excessive CPU usage
        SDL_Delay(16);  // ~60 FPS
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
