#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdio.h>

#include "palette_io.h"
#include "pixel_editor.h"
#include "tile_sheet.h"
#include "tiles_io.h"
#include "ui.h"

/**
 * Application state structure
 */
typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;

    TileSheet tile_sheet;
    PixelEditor pixel_editor;
    UIState ui;

    bool running;
    bool keys[SDL_SCANCODE_COUNT];

    int mouse_x, mouse_y;
    bool mouse_buttons[8];
    bool mouse_clicked[8];
} AppState;

/**
 * Initialize the application
 */
bool app_init(AppState* app) {
    if (!app)
        return false;

    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        printf("Error: Could not initialize SDL3: %s\n", SDL_GetError());
        return false;
    }

    // Create window
    app->window = SDL_CreateWindow("Tile Maker v1.0 - SDL3 Edition", WINDOW_WIDTH, WINDOW_HEIGHT,
                                   SDL_WINDOW_RESIZABLE);
    if (!app->window) {
        printf("Error: Could not create window: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }

    // Create renderer
    app->renderer = SDL_CreateRenderer(app->window, NULL);
    if (!app->renderer) {
        printf("Error: Could not create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(app->window);
        SDL_Quit();
        return false;
    }

    // Initialize input state
    memset(app->keys, 0, sizeof(app->keys));
    memset(app->mouse_buttons, 0, sizeof(app->mouse_buttons));
    memset(app->mouse_clicked, 0, sizeof(app->mouse_clicked));
    app->mouse_x = 0;
    app->mouse_y = 0;

    // Initialize subsystems
    if (!tile_sheet_init(&app->tile_sheet, app->renderer)) {
        printf("Error: Failed to initialize tile sheet\n");
        return false;
    }

    if (!pixel_editor_init(&app->pixel_editor, app->renderer)) {
        printf("Error: Failed to initialize pixel editor\n");
        return false;
    }

    if (!ui_init(&app->ui, app->renderer)) {
        printf("Error: Failed to initialize UI\n");
        return false;
    }

    app->running = true;

    printf("Tile Maker initialized successfully\n");
    return true;
}

/**
 * Cleanup application resources
 */
void app_cleanup(AppState* app) {
    if (!app)
        return;

    tile_sheet_cleanup(&app->tile_sheet);
    pixel_editor_cleanup(&app->pixel_editor);
    ui_cleanup(&app->ui);

    if (app->renderer) {
        SDL_DestroyRenderer(app->renderer);
        app->renderer = NULL;
    }

    if (app->window) {
        SDL_DestroyWindow(app->window);
        app->window = NULL;
    }

    SDL_Quit();
    printf("Tile Maker cleaned up\n");
}

/**
 * Handle SDL events
 */
void app_handle_events(AppState* app) {
    if (!app)
        return;

    SDL_Event event;

    // Reset click states
    memset(app->mouse_clicked, 0, sizeof(app->mouse_clicked));

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                app->running = false;
                break;

            case SDL_EVENT_KEY_DOWN:
                if (event.key.scancode < SDL_SCANCODE_COUNT) {
                    app->keys[event.key.scancode] = true;
                }
                break;

            case SDL_EVENT_KEY_UP:
                if (event.key.scancode < SDL_SCANCODE_COUNT) {
                    app->keys[event.key.scancode] = false;
                }
                break;

            case SDL_EVENT_MOUSE_MOTION:
                app->mouse_x = (int)event.motion.x;
                app->mouse_y = (int)event.motion.y;
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (event.button.button < 8) {
                    app->mouse_buttons[event.button.button] = true;
                    app->mouse_clicked[event.button.button] = true;
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (event.button.button < 8) {
                    app->mouse_buttons[event.button.button] = false;
                }
                break;

            default:
                break;
        }
    }
}

/**
 * Handle keyboard shortcuts
 */
void app_handle_keyboard(AppState* app) {
    if (!app)
        return;

    // ESC - Quit (with save prompt if dirty)
    if (app->keys[SDL_SCANCODE_ESCAPE]) {
        app->keys[SDL_SCANCODE_ESCAPE] = false;
        app->running = false;
    }

    // S - Save
    if (app->keys[SDL_SCANCODE_S]) {
        app->keys[SDL_SCANCODE_S] = false;
        if (tiles_save("tiles.dat")) {
            ui_set_status(&app->ui, "Tiles saved successfully");
            ui_set_dirty(&app->ui, false);
        } else {
            ui_set_status(&app->ui, "Failed to save tiles");
        }
    }

    // L - Load
    if (app->keys[SDL_SCANCODE_L]) {
        app->keys[SDL_SCANCODE_L] = false;
        if (tiles_load("tiles.dat")) {
            ui_set_status(&app->ui, "Tiles loaded successfully");
            ui_set_dirty(&app->ui, false);
            tile_sheet_set_selected(&app->tile_sheet, 0);
            pixel_editor_set_tile(&app->pixel_editor, 0);
        } else {
            ui_set_status(&app->ui, "Failed to load tiles");
        }
    }

    // G - Toggle grid
    if (app->keys[SDL_SCANCODE_G]) {
        app->keys[SDL_SCANCODE_G] = false;
        pixel_editor_toggle_grid(&app->pixel_editor);
        ui_set_status(&app->ui, pixel_editor_grid_visible(&app->pixel_editor) ? "Grid enabled"
                                                                              : "Grid disabled");
    }

    // Ctrl+N - New (clear all tiles)
    if (app->keys[SDL_SCANCODE_N] &&
        (app->keys[SDL_SCANCODE_LCTRL] || app->keys[SDL_SCANCODE_RCTRL])) {
        app->keys[SDL_SCANCODE_N] = false;
        clear_all_tiles(0);  // Clear to palette index 0 (black)
        ui_set_status(&app->ui, "All tiles cleared");
        ui_set_dirty(&app->ui, true);
    }

    // Arrow keys - Navigate tile selection
    if (app->keys[SDL_SCANCODE_LEFT]) {
        app->keys[SDL_SCANCODE_LEFT] = false;
        tile_sheet_navigate(&app->tile_sheet, -1, true);
        int selected = tile_sheet_get_selected(&app->tile_sheet);
        pixel_editor_set_tile(&app->pixel_editor, selected);
    }

    if (app->keys[SDL_SCANCODE_RIGHT]) {
        app->keys[SDL_SCANCODE_RIGHT] = false;
        tile_sheet_navigate(&app->tile_sheet, 1, true);
        int selected = tile_sheet_get_selected(&app->tile_sheet);
        pixel_editor_set_tile(&app->pixel_editor, selected);
    }

    if (app->keys[SDL_SCANCODE_UP]) {
        app->keys[SDL_SCANCODE_UP] = false;
        tile_sheet_navigate(&app->tile_sheet, -1, false);
        int selected = tile_sheet_get_selected(&app->tile_sheet);
        pixel_editor_set_tile(&app->pixel_editor, selected);
    }

    if (app->keys[SDL_SCANCODE_DOWN]) {
        app->keys[SDL_SCANCODE_DOWN] = false;
        tile_sheet_navigate(&app->tile_sheet, 1, false);
        int selected = tile_sheet_get_selected(&app->tile_sheet);
        pixel_editor_set_tile(&app->pixel_editor, selected);
    }
}

/**
 * Update application state
 */
void app_update(AppState* app) {
    if (!app)
        return;

    // Update subsystems
    tile_sheet_update(&app->tile_sheet, app->renderer);
    pixel_editor_update(&app->pixel_editor, app->renderer);
    ui_update(&app->ui, app->renderer);

    // Update UI dirty indicator
    ui_set_dirty(&app->ui, tiles_is_modified());

    // Handle UI mouse input
    int ui_action = ui_handle_mouse(&app->ui, app->mouse_x, app->mouse_y,
                                    app->mouse_clicked[SDL_BUTTON_LEFT], SDL_BUTTON_LEFT);

    if (ui_action >= 10) {
        // Palette selection
        int palette_index = ui_action - 10;
        pixel_editor_set_color(&app->pixel_editor, palette_index);
    } else if (ui_action == 1) {
        // Save
        if (tiles_save("tiles.dat")) {
            ui_set_status(&app->ui, "Tiles saved successfully");
        } else {
            ui_set_status(&app->ui, "Failed to save tiles");
        }
    } else if (ui_action == 2) {
        // Load
        if (tiles_load("tiles.dat")) {
            ui_set_status(&app->ui, "Tiles loaded successfully");
            tile_sheet_set_selected(&app->tile_sheet, 0);
            pixel_editor_set_tile(&app->pixel_editor, 0);
        } else {
            ui_set_status(&app->ui, "Failed to load tiles");
        }
    } else if (ui_action == 3) {
        // New
        clear_all_tiles(0);
        ui_set_status(&app->ui, "All tiles cleared");
    } else if (ui_action == 4) {
        // Quit
        app->running = false;
    }

    // Handle tile sheet input (10, 10 is the tile sheet position)
    int clicked_tile = tile_sheet_handle_input(&app->tile_sheet, 10, 50, app->mouse_x, app->mouse_y,
                                               app->mouse_clicked[SDL_BUTTON_LEFT],
                                               ui_check_double_click(&app->ui, -1));

    if (clicked_tile >= 0) {
        bool is_double_click = ui_check_double_click(&app->ui, clicked_tile);
        if (is_double_click) {
            // Double-click opens tile in pixel editor
            pixel_editor_set_tile(&app->pixel_editor, clicked_tile);
            ui_set_status(&app->ui, "Editing tile in pixel editor");
        }
    }

    // Handle pixel editor input (280, 50 is the pixel editor position)
    bool pixel_modified = pixel_editor_handle_input(
        &app->pixel_editor, 280, 50, app->mouse_x, app->mouse_y,
        app->mouse_buttons[SDL_BUTTON_LEFT], app->mouse_buttons[SDL_BUTTON_RIGHT],
        app->mouse_buttons[SDL_BUTTON_LEFT] || app->mouse_buttons[SDL_BUTTON_RIGHT]);

    if (pixel_modified) {
        // Sync current paint color with UI
        int current_color = pixel_editor_get_color(&app->pixel_editor);
        ui_set_palette_selection(&app->ui, current_color);
    }
}

/**
 * Render the application
 */
void app_render(AppState* app) {
    if (!app)
        return;

    // Clear screen
    SDL_SetRenderDrawColor(app->renderer, 32, 32, 32, 255);
    SDL_RenderClear(app->renderer);

    // Render tile sheet (left panel)
    tile_sheet_render(&app->tile_sheet, app->renderer, 10, 50);

    // Render pixel editor (center panel)
    pixel_editor_render(&app->pixel_editor, app->renderer, 280, 50);

    // Render UI (palette bar, buttons, status)
    ui_render(&app->ui, app->renderer);

    // Present frame
    SDL_RenderPresent(app->renderer);
}

/**
 * Main application entry point
 */
int main(int argc, char* argv[]) {
    (void)argc;  // Suppress unused parameter warning
    (void)argv;

    printf("Tile Maker v1.0.0 - SDL3 Edition\n");
    printf("Controls:\n");
    printf("  - Click tile sheet to select tiles\n");
    printf("  - Double-click tile to edit in pixel editor\n");
    printf("  - Left mouse: Paint with current color\n");
    printf("  - Right mouse: Pick color from pixel\n");
    printf("  - S: Save tiles\n");
    printf("  - L: Load tiles\n");
    printf("  - G: Toggle pixel grid\n");
    printf("  - Ctrl+N: Clear all tiles\n");
    printf("  - Arrow keys: Navigate tile selection\n");
    printf("  - ESC: Quit\n");
    printf("\n");

    AppState app = {0};

    // Initialize application
    if (!app_init(&app)) {
        return 1;
    }

    // Initialize palette and tiles
    palette_init();
    if (!palette_load("palette.dat")) {
        printf("Using default palette\n");
    }

    tiles_init();
    if (tiles_load("tiles.dat")) {
        ui_set_status(&app.ui, "Tiles loaded from tiles.dat");
        ui_set_dirty(&app.ui, false);
    } else {
        ui_set_status(&app.ui, "New tile set - no tiles.dat found");
        ui_set_dirty(&app.ui, true);
    }

    // Main application loop
    printf("Starting main application loop...\n");

    while (app.running) {
        // Handle events
        app_handle_events(&app);

        // Handle keyboard shortcuts
        app_handle_keyboard(&app);

        // Update application state
        app_update(&app);

        // Render frame
        app_render(&app);

        // Small delay to prevent excessive CPU usage
        SDL_Delay(16);  // ~60 FPS
    }

    // Check for unsaved changes before exiting
    if (tiles_is_modified()) {
        printf("\nWarning: You have unsaved changes!\n");
        printf("Your tiles have been modified but not saved.\n");
        printf("Consider saving your work with 'tiles.dat'.\n");
    }

    // Cleanup and exit
    printf("Shutting down...\n");
    app_cleanup(&app);

    printf("Tile Maker closed successfully\n");
    return 0;
}
