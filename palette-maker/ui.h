#ifndef UI_H
#define UI_H

#include <SDL3/SDL.h>
#include "palette.h"

/**
 * UI layout constants
 */
#define WINDOW_WIDTH 320
#define WINDOW_HEIGHT 240
#define SWATCH_SIZE 40
#define SWATCH_BORDER 2
#define GRID_COLS 4
#define GRID_ROWS 4
#define GRID_START_X 10
#define GRID_START_Y 10
#define UI_PANEL_X 180
#define UI_PANEL_Y 10
#define INPUT_FIELD_WIDTH 60
#define INPUT_FIELD_HEIGHT 20
#define BUTTON_WIDTH 80
#define BUTTON_HEIGHT 25

/**
 * UI state structure
 */
typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* font_texture;  // For text rendering

    int selected_swatch;     // Currently selected swatch index (0-15)
    bool color_picker_open;  // Is color picker dialog open
    bool show_save_dialog;   // Show save file dialog
    bool show_load_dialog;   // Show load file dialog

    // Input field states for RGBA values
    bool editing_r, editing_g, editing_b, editing_a;
    char input_r[4], input_g[4], input_b[4], input_a[4];

    // File dialog state
    char file_input[256];
    bool editing_filename;

    // Mouse interaction
    int mouse_x, mouse_y;
    bool mouse_down;
    Uint32 last_click_time;  // For double-click detection
    int last_click_swatch;   // Last clicked swatch for double-click

} UIState;

/**
 * Initialize UI system
 * Creates window, renderer, and initializes UI state
 *
 * @param ui Pointer to UI state structure
 * @return true if successful, false on error
 */
bool ui_init(UIState* ui);

/**
 * Cleanup UI system
 * Destroys window, renderer, and cleans up resources
 *
 * @param ui Pointer to UI state structure
 */
void ui_cleanup(UIState* ui);

/**
 * Handle SDL events
 * Processes keyboard, mouse, and window events
 *
 * @param ui Pointer to UI state structure
 * @param palette Pointer to palette structure
 * @param event SDL event to process
 * @return true to continue main loop, false to quit
 */
bool ui_handle_event(UIState* ui, Palette* palette, SDL_Event* event);

/**
 * Render the complete UI
 * Draws swatch grid, input fields, buttons, and dialogs
 *
 * @param ui Pointer to UI state structure
 * @param palette Pointer to palette structure
 */
void ui_render(UIState* ui, const Palette* palette);

/**
 * Get swatch index from mouse coordinates
 * Returns -1 if mouse is not over any swatch
 *
 * @param x Mouse X coordinate
 * @param y Mouse Y coordinate
 * @return Swatch index (0-15) or -1 if not over swatch
 */
int ui_get_swatch_at_position(int x, int y);

/**
 * Open color picker for selected swatch
 * Sets up color picker state for editing
 *
 * @param ui Pointer to UI state structure
 * @param palette Pointer to palette structure
 */
void ui_open_color_picker(UIState* ui, Palette* palette);

/**
 * Close color picker
 * Cleans up color picker state
 *
 * @param ui Pointer to UI state structure
 */
void ui_close_color_picker(UIState* ui);

/**
 * Update RGBA input fields from selected color
 * Synchronizes input fields with currently selected swatch
 *
 * @param ui Pointer to UI state structure
 * @param palette Pointer to palette structure
 */
void ui_update_rgba_fields(UIState* ui, const Palette* palette);

/**
 * Apply RGBA input field values to selected color
 * Updates palette color from input field values
 *
 * @param ui Pointer to UI state structure
 * @param palette Pointer to palette structure
 */
void ui_apply_rgba_fields(UIState* ui, Palette* palette);

/**
 * Show save file dialog
 *
 * @param ui Pointer to UI state structure
 */
void ui_show_save_dialog(UIState* ui);

/**
 * Show load file dialog
 *
 * @param ui Pointer to UI state structure
 */
void ui_show_load_dialog(UIState* ui);

/**
 * Check for unsaved changes and prompt user
 * Returns true if it's safe to proceed (saved or user chose to discard)
 *
 * @param ui Pointer to UI state structure
 * @param palette Pointer to palette structure
 * @return true if safe to proceed, false if should cancel operation
 */
bool ui_check_unsaved_changes(UIState* ui, const Palette* palette);

/**
 * Render text at specified position
 * Simple text rendering function for UI labels
 *
 * @param ui Pointer to UI state structure
 * @param text Text string to render
 * @param x X coordinate
 * @param y Y coordinate
 * @param color Text color (SDL_Color)
 */
void ui_render_text(UIState* ui, const char* text, int x, int y, SDL_Color color);

/**
 * Render filled rectangle
 * Helper function for drawing UI elements
 *
 * @param ui Pointer to UI state structure
 * @param x X coordinate
 * @param y Y coordinate
 * @param w Width
 * @param h Height
 * @param color Fill color
 */
void ui_render_rect(UIState* ui, int x, int y, int w, int h, SDL_Color color);

/**
 * Render rectangle outline
 * Helper function for drawing borders
 *
 * @param ui Pointer to UI state structure
 * @param x X coordinate
 * @param y Y coordinate
 * @param w Width
 * @param h Height
 * @param color Border color
 */
void ui_render_rect_outline(UIState* ui, int x, int y, int w, int h, SDL_Color color);

#endif  // UI_H
