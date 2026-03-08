#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "../shared/config/config_manager.h"
#include "../shared/error_handler/error_handler.h"
#include "../shared/ui_framework/ui_viewport.h"
#include "palette_io.h"
#include "pixel_editor.h"
#include "tile_specs_io.h"
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
    ConfigManager config;

    bool running;
    bool keys[SDL_SCANCODE_COUNT];

    int mouse_x, mouse_y;
    bool mouse_buttons[8];
    bool mouse_clicked[8];

    char tiles_file_path[CONFIG_MAX_PATH_LENGTH];
    char palette_file_path[CONFIG_MAX_PATH_LENGTH];
    int last_status_tile;
} AppState;

static bool file_exists(const char* path) {
    if (!path || path[0] == '\0') {
        return false;
    }

    FILE* f = fopen(path, "rb");
    if (!f) {
        return false;
    }
    fclose(f);
    return true;
}

static const char* resolve_tilemaker_config_path(void) {
    static const char* candidates[] = {"config/tile_maker_config.json",
                                       "../config/tile_maker_config.json",
                                       "tile_maker_config.json"};
    for (size_t i = 0; i < sizeof(candidates) / sizeof(candidates[0]); i++) {
        if (file_exists(candidates[i])) {
            return candidates[i];
        }
    }

    // Fall back to default search location; config manager will use defaults if missing.
    return candidates[0];
}

static void app_resolve_file_path(char* out, size_t out_size, const char* configured,
                                  const char* fallback) {
    if (!out || out_size == 0 || !fallback) {
        return;
    }

    const char* source = (configured && configured[0] != '\0') ? configured : fallback;
    strncpy(out, source, out_size - 1);
    out[out_size - 1] = '\0';
}

static bool app_has_unsaved_changes(const AppState* app) {
    (void)app;
    return tiles_is_modified() || tile_specs_is_modified();
}

static void app_sync_tile_spec_ui(AppState* app, int tile_id) {
    if (!app || tile_id < 0 || tile_id >= TILE_COUNT) {
        return;
    }

    const uint8_t health = tile_spec_get_health(tile_id);
    const uint8_t destruction = tile_spec_get_destruction_mode(tile_id);
    const uint8_t movement = tile_spec_get_movement(tile_id);

    ui_set_tile_spec(&app->ui, tile_id, health, destruction, movement);

    char spec_status[96];
    snprintf(spec_status, sizeof(spec_status), "Tile %03d  H:%u D:%u M:%u", tile_id,
             (unsigned)health, (unsigned)destruction, (unsigned)movement);
    ui_set_status(&app->ui, spec_status);
    app->last_status_tile = tile_id;
}

static bool app_save_all(AppState* app) {
    if (!app) {
        return false;
    }

    bool ok_tiles = tiles_save(app->tiles_file_path);
    if (ok_tiles) {
        ui_set_dirty(&app->ui, false);
        return true;
    }

    return false;
}

static bool app_load_all(AppState* app) {
    if (!app) {
        return false;
    }

    bool ok_tiles = tiles_load(app->tiles_file_path);
    ui_set_dirty(&app->ui, app_has_unsaved_changes(app));
    return ok_tiles;
}

/**
 * Initialize the application
 */
bool app_init(AppState* app) {
    if (!app)
        return false;

    // Initialize configuration system
    if (!config_manager_init(&app->config, "Tile Maker")) {
        printf("Error: Failed to initialize configuration manager\n");
        return false;
    }

    // Register configuration entries
    config_register_entry(&app->config, "display", "window_width", CONFIG_TYPE_INT,
                          config_make_int(WINDOW_WIDTH), false);
    config_register_entry(&app->config, "display", "window_height", CONFIG_TYPE_INT,
                          config_make_int(WINDOW_HEIGHT), false);
    config_register_entry(&app->config, "display", "window_title", CONFIG_TYPE_STRING,
                          config_make_string("Tile Maker v1.0 - SDL3 Edition"), false);
    config_register_entry(&app->config, "ui", "palette_bar_height", CONFIG_TYPE_INT,
                          config_make_int(PALETTE_BAR_HEIGHT), false);
    config_register_entry(&app->config, "ui", "button_width", CONFIG_TYPE_INT,
                          config_make_int(BUTTON_WIDTH), false);
    config_register_entry(&app->config, "ui", "button_height", CONFIG_TYPE_INT,
                          config_make_int(BUTTON_HEIGHT), false);
    config_register_entry(&app->config, "ui", "palette_swatch_size", CONFIG_TYPE_INT,
                          config_make_int(PALETTE_SWATCH_SIZE), false);
    config_register_entry(&app->config, "performance", "target_fps", CONFIG_TYPE_INT,
                          config_make_int(TARGET_FPS), false);
    config_register_entry(&app->config, "performance", "frame_delay_ms", CONFIG_TYPE_INT,
                          config_make_int(FRAME_DELAY_MS), false);
    config_register_entry(&app->config, "files", "default_tiles_file", CONFIG_TYPE_STRING,
                          config_make_string("tiles.dat"), false);
    config_register_entry(&app->config, "files", "default_palette_file", CONFIG_TYPE_STRING,
                          config_make_string("palette.dat"), false);

    // Load configuration file
    const char* config_path = resolve_tilemaker_config_path();
    if (!config_manager_load(&app->config, config_path)) {
        printf("Warning: Failed to load configuration file, using defaults\n");
        if (ErrorHandler_HasError()) {
            const Error_t* err = ErrorHandler_Get();
            if (err) {
                printf("Error: %s\n", err->message);
            }
            ErrorHandler_Clear();
        }
    }

    // Get configuration values
    int window_width = config_get_int(&app->config, "display", "window_width", WINDOW_WIDTH);
    int window_height = config_get_int(&app->config, "display", "window_height", WINDOW_HEIGHT);
    const char* window_title = config_get_string(&app->config, "display", "window_title",
                                                 "Tile Maker v1.0 - SDL3 Edition");
    const char* configured_tiles_file =
        config_get_string(&app->config, "files", "default_tiles_file", "tiles.dat");
    const char* configured_palette_file =
        config_get_string(&app->config, "files", "default_palette_file", "palette.dat");

    app_resolve_file_path(app->tiles_file_path, sizeof(app->tiles_file_path), configured_tiles_file,
                          "tiles.dat");
    app_resolve_file_path(app->palette_file_path, sizeof(app->palette_file_path),
                          configured_palette_file, "palette.dat");

    printf("Starting Tile Maker with configuration:\n");
    printf("  Config file: %s\n", config_path);
    printf("  Window: %dx%d\n", window_width, window_height);
    printf("  Title: %s\n", window_title);
    printf("  Tiles file: %s\n", app->tiles_file_path);
    printf("  Palette file: %s\n", app->palette_file_path);

    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        printf("Error: Could not initialize SDL3: %s\n", SDL_GetError());
        return false;
    }

    // Create window
    app->window = SDL_CreateWindow(window_title, window_width, window_height, SDL_WINDOW_RESIZABLE);
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

    if (!SDL_SetRenderLogicalPresentation(app->renderer, window_width, window_height,
                                          SDL_LOGICAL_PRESENTATION_LETTERBOX)) {
        printf("Warning: Failed to set logical presentation: %s\n", SDL_GetError());
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
    app->last_status_tile = -1;

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
                if (app_has_unsaved_changes(app)) {
                    app->ui.show_quit_dialog = true;
                } else {
                    app->running = false;
                }
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
                {
                    float logical_x = event.motion.x;
                    float logical_y = event.motion.y;
                    ui_viewport_window_to_logical(app->renderer, event.motion.x, event.motion.y,
                                                  &logical_x, &logical_y);
                    app->mouse_x = (int)logical_x;
                    app->mouse_y = (int)logical_y;
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                {
                    float logical_x = event.button.x;
                    float logical_y = event.button.y;
                    ui_viewport_window_to_logical(app->renderer, event.button.x, event.button.y,
                                                  &logical_x, &logical_y);
                    app->mouse_x = (int)logical_x;
                    app->mouse_y = (int)logical_y;
                }
                if (event.button.button < MOUSE_BUTTON_LIMIT) {
                    app->mouse_buttons[event.button.button] = true;
                    app->mouse_clicked[event.button.button] = true;
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
                {
                    float logical_x = event.button.x;
                    float logical_y = event.button.y;
                    ui_viewport_window_to_logical(app->renderer, event.button.x, event.button.y,
                                                  &logical_x, &logical_y);
                    app->mouse_x = (int)logical_x;
                    app->mouse_y = (int)logical_y;
                }
                if (event.button.button < MOUSE_BUTTON_LIMIT) {
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
        if (app_has_unsaved_changes(app)) {
            app->ui.show_quit_dialog = true;
        } else {
            app->running = false;
        }
    }

    // S - Save
    if (app->keys[SDL_SCANCODE_S]) {
        app->keys[SDL_SCANCODE_S] = false;
        if (app_save_all(app)) {
            ui_set_status(&app->ui, "Tiles (with specs) saved successfully");
        } else {
            ui_set_status(&app->ui, "Failed to save tiles file");
        }
    }

    // L - Load
    if (app->keys[SDL_SCANCODE_L]) {
        app->keys[SDL_SCANCODE_L] = false;
        if (app_load_all(app)) {
            ui_set_status(&app->ui, "Tiles (with specs) loaded");
            tile_sheet_set_selected(&app->tile_sheet, 0);
            pixel_editor_set_tile(&app->pixel_editor, 0);
            app_sync_tile_spec_ui(app, 0);
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
        tile_specs_reset_defaults();
        ui_set_status(&app->ui, "All tiles/specs reset");
        ui_set_dirty(&app->ui, true);
    }

    // Tile spec editing shortcuts for selected tile:
    // [ ] health down/up, , . destruction mode down/up, ; ' movement down/up
    {
        const int selected_tile = tile_sheet_get_selected(&app->tile_sheet);
        bool spec_changed = false;

        if (app->keys[SDL_SCANCODE_LEFTBRACKET]) {
            app->keys[SDL_SCANCODE_LEFTBRACKET] = false;
            uint8_t health = tile_spec_get_health(selected_tile);
            if (health > 0) {
                tile_spec_set_health(selected_tile, (uint8_t)(health - 1));
                spec_changed = true;
            }
        }
        if (app->keys[SDL_SCANCODE_RIGHTBRACKET]) {
            app->keys[SDL_SCANCODE_RIGHTBRACKET] = false;
            uint8_t health = tile_spec_get_health(selected_tile);
            if (health < TILE_SPEC_HEALTH_MASK) {
                tile_spec_set_health(selected_tile, (uint8_t)(health + 1));
                spec_changed = true;
            }
        }
        if (app->keys[SDL_SCANCODE_COMMA]) {
            app->keys[SDL_SCANCODE_COMMA] = false;
            uint8_t mode = tile_spec_get_destruction_mode(selected_tile);
            if (mode > 0) {
                tile_spec_set_destruction_mode(selected_tile, (uint8_t)(mode - 1));
                spec_changed = true;
            }
        }
        if (app->keys[SDL_SCANCODE_PERIOD]) {
            app->keys[SDL_SCANCODE_PERIOD] = false;
            uint8_t mode = tile_spec_get_destruction_mode(selected_tile);
            if (mode < TILE_SPEC_DESTRUCT_MASK) {
                tile_spec_set_destruction_mode(selected_tile, (uint8_t)(mode + 1));
                spec_changed = true;
            }
        }
        if (app->keys[SDL_SCANCODE_SEMICOLON]) {
            app->keys[SDL_SCANCODE_SEMICOLON] = false;
            uint8_t movement = tile_spec_get_movement(selected_tile);
            if (movement > 0) {
                tile_spec_set_movement(selected_tile, (uint8_t)(movement - 1));
                spec_changed = true;
            }
        }
        if (app->keys[SDL_SCANCODE_APOSTROPHE]) {
            app->keys[SDL_SCANCODE_APOSTROPHE] = false;
            uint8_t movement = tile_spec_get_movement(selected_tile);
            if (movement < TILE_SPEC_MOVEMENT_MASK) {
                tile_spec_set_movement(selected_tile, (uint8_t)(movement + 1));
                spec_changed = true;
            }
        }

        if (spec_changed) {
            ui_set_dirty(&app->ui, true);
            app_sync_tile_spec_ui(app, selected_tile);
        }
    }

    // Arrow keys - Navigate tile selection
    if (app->keys[SDL_SCANCODE_LEFT]) {
        app->keys[SDL_SCANCODE_LEFT] = false;
        tile_sheet_navigate(&app->tile_sheet, -1, true);
        int selected = tile_sheet_get_selected(&app->tile_sheet);
        pixel_editor_set_tile(&app->pixel_editor, selected);
        app_sync_tile_spec_ui(app, selected);
    }

    if (app->keys[SDL_SCANCODE_RIGHT]) {
        app->keys[SDL_SCANCODE_RIGHT] = false;
        tile_sheet_navigate(&app->tile_sheet, 1, true);
        int selected = tile_sheet_get_selected(&app->tile_sheet);
        pixel_editor_set_tile(&app->pixel_editor, selected);
        app_sync_tile_spec_ui(app, selected);
    }

    if (app->keys[SDL_SCANCODE_UP]) {
        app->keys[SDL_SCANCODE_UP] = false;
        tile_sheet_navigate(&app->tile_sheet, -1, false);
        int selected = tile_sheet_get_selected(&app->tile_sheet);
        pixel_editor_set_tile(&app->pixel_editor, selected);
        app_sync_tile_spec_ui(app, selected);
    }

    if (app->keys[SDL_SCANCODE_DOWN]) {
        app->keys[SDL_SCANCODE_DOWN] = false;
        tile_sheet_navigate(&app->tile_sheet, 1, false);
        int selected = tile_sheet_get_selected(&app->tile_sheet);
        pixel_editor_set_tile(&app->pixel_editor, selected);
        app_sync_tile_spec_ui(app, selected);
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
    ui_set_dirty(&app->ui, app_has_unsaved_changes(app));

    // Handle UI mouse input
    int ui_action = ui_handle_mouse(&app->ui, app->mouse_x, app->mouse_y,
                                    app->mouse_clicked[SDL_BUTTON_LEFT], SDL_BUTTON_LEFT);

    if (ui_action >= PALETTE_SELECTION_OFFSET &&
        ui_action < PALETTE_SELECTION_OFFSET + 16) {
        // Palette selection
        int palette_index = ui_action - PALETTE_SELECTION_OFFSET;
        pixel_editor_set_color(&app->pixel_editor, palette_index);
    } else if (ui_action == UI_ACTION_SAVE) {
        // Save
        if (app_save_all(app)) {
            ui_set_status(&app->ui, "Tiles (with specs) saved successfully");
        } else {
            ui_set_status(&app->ui, "Failed to save tiles file");
        }
    } else if (ui_action == UI_ACTION_LOAD) {
        // Load
        if (app_load_all(app)) {
            ui_set_status(&app->ui, "Tiles (with specs) loaded");
            tile_sheet_set_selected(&app->tile_sheet, 0);
            pixel_editor_set_tile(&app->pixel_editor, 0);
            app_sync_tile_spec_ui(app, 0);
        } else {
            ui_set_status(&app->ui, "Failed to load tiles");
        }
    } else if (ui_action == UI_ACTION_NEW) {
        // New
        clear_all_tiles(0);
        tile_specs_reset_defaults();
        ui_set_status(&app->ui, "All tiles/specs reset");
        app_sync_tile_spec_ui(app, tile_sheet_get_selected(&app->tile_sheet));
    } else if (ui_action == UI_ACTION_QUIT) {
        // Quit
        if (app_has_unsaved_changes(app)) {
            app->ui.show_quit_dialog = true;
        } else {
            app->running = false;
        }
    } else if (ui_action == UI_ACTION_QUIT_CONFIRM) {
        // Quit confirmed from dialog
        app->running = false;
    } else if (ui_action >= UI_ACTION_SPEC_HEALTH_SET_BASE &&
               ui_action < UI_ACTION_SPEC_HEALTH_SET_BASE + 8) {
        const int selected_tile = tile_sheet_get_selected(&app->tile_sheet);
        uint8_t value = (uint8_t)(ui_action - UI_ACTION_SPEC_HEALTH_SET_BASE);
        tile_spec_set_health(selected_tile, value);
        ui_set_dirty(&app->ui, true);
        app_sync_tile_spec_ui(app, selected_tile);
    } else if (ui_action >= UI_ACTION_SPEC_DESTRUCT_SET_BASE &&
               ui_action < UI_ACTION_SPEC_DESTRUCT_SET_BASE + 8) {
        const int selected_tile = tile_sheet_get_selected(&app->tile_sheet);
        uint8_t value = (uint8_t)(ui_action - UI_ACTION_SPEC_DESTRUCT_SET_BASE);
        tile_spec_set_destruction_mode(selected_tile, value);
        ui_set_dirty(&app->ui, true);
        app_sync_tile_spec_ui(app, selected_tile);
    } else if (ui_action >= UI_ACTION_SPEC_MOVE_SET_BASE &&
               ui_action < UI_ACTION_SPEC_MOVE_SET_BASE + 4) {
        const int selected_tile = tile_sheet_get_selected(&app->tile_sheet);
        uint8_t value = (uint8_t)(ui_action - UI_ACTION_SPEC_MOVE_SET_BASE);
        tile_spec_set_movement(selected_tile, value);
        ui_set_dirty(&app->ui, true);
        app_sync_tile_spec_ui(app, selected_tile);
    }

    // Handle tile sheet input (tile sheet panel position)
    int hovered_tile = tile_sheet_handle_input(
        &app->tile_sheet, TILE_SHEET_POS_X, TILE_SHEET_POS_Y, app->mouse_x, app->mouse_y,
        app->mouse_clicked[SDL_BUTTON_LEFT], ui_check_double_click(&app->ui, -1));

    if (hovered_tile != -1) {
        pixel_editor_set_tile(&app->pixel_editor, hovered_tile);
    } else {
        pixel_editor_set_tile(&app->pixel_editor, app->tile_sheet.selected_tile);
    }

    if (app->tile_sheet.selected_tile != app->last_status_tile) {
        app_sync_tile_spec_ui(app, app->tile_sheet.selected_tile);
    }

    // Handle pixel editor input (pixel editor panel position)
    bool pixel_modified = pixel_editor_handle_input(
        &app->pixel_editor, PIXEL_EDITOR_POS_X, PIXEL_EDITOR_POS_Y, app->mouse_x, app->mouse_y,
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
    SDL_SetRenderDrawColor(app->renderer, CLEAR_COLOR_R, CLEAR_COLOR_G, CLEAR_COLOR_B,
                           CLEAR_COLOR_A);
    SDL_RenderClear(app->renderer);

    // Render tile sheet (left panel)
    tile_sheet_render(&app->tile_sheet, app->renderer, TILE_SHEET_POS_X, TILE_SHEET_POS_Y);

    // Render pixel editor (center panel)
    pixel_editor_render(&app->pixel_editor, app->renderer, PIXEL_EDITOR_POS_X, PIXEL_EDITOR_POS_Y);

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
    printf("  - [ / ]: Decrease/increase selected tile health\n");
    printf("  - , / .: Decrease/increase selected tile destruction mode\n");
    printf("  - ; / ': Decrease/increase selected tile movement mode\n");
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
    if (!palette_load(app.palette_file_path)) {
        printf("Using default palette\n");
    }

    tiles_init();
    tile_specs_init();

    if (app_load_all(&app)) {
        ui_set_status(&app.ui, "Tiles (with specs) loaded");
    } else {
        ui_set_status(&app.ui, "New tile set - no tiles file found");
    }
    app_sync_tile_spec_ui(&app, 0);

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
        SDL_Delay(FRAME_DELAY_MS);  // ~60 FPS
    }

    // Check for unsaved changes before exiting
    if (app_has_unsaved_changes(&app)) {
        printf("\nWarning: You have unsaved changes!\n");
        printf("Your tiles/specs have been modified but not saved.\n");
        printf("Consider saving your work with '%s'.\n", app.tiles_file_path);
    }

    // Cleanup and exit
    printf("Shutting down...\n");
    app_cleanup(&app);

    printf("Tile Maker closed successfully\n");
    return 0;
}
