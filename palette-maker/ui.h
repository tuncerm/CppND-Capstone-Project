#ifndef UI_H
#define UI_H

#include <SDL3/SDL.h>
#include "palette.h"

/**
 * UI layout constants
 */
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define SWATCH_SIZE 45
#define SWATCH_BORDER 2
#define GRID_COLS 4
#define GRID_ROWS 4
#define GRID_START_X 20
#define GRID_START_Y 20
#define UI_PANEL_X 220
#define UI_PANEL_Y 20
#define UI_PANEL_W 320
#define UI_PANEL_H 300
#define UI_PANEL_ROW_H 30
#define BUTTON_WIDTH 30
#define BUTTON_HEIGHT 20
#define VALUE_DISPLAY_WIDTH 45
#define VALUE_DISPLAY_HEIGHT 20
#define ACTION_BUTTON_WIDTH 80
#define ACTION_BUTTON_HEIGHT 25

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
 * Show save file dialog
 *
 * @param ui Pointer to UI state structure
 */
void ui_show_save_dialog(UIState* ui);

/**
 * Reset palette to default colors
 *
 * @param ui Pointer to UI state structure
 * @param palette Pointer to palette structure
 */
void ui_reset_palette(UIState* ui, Palette* palette);

/**
 * Handle button clicks for RGBA value adjustment
 * Handles increment/decrement buttons for RGBA values
 *
 * @param ui Pointer to UI state structure
 * @param palette Pointer to palette structure
 * @param x Mouse X coordinate
 * @param y Mouse Y coordinate
 * @return true if a button was clicked, false otherwise
 */
bool ui_handle_rgba_button_click(UIState* ui, Palette* palette, int x, int y);

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

/**
 * Render RGBA control buttons
 * Renders increment/decrement buttons for RGBA values
 *
 * @param ui Pointer to UI state structure
 * @param palette Pointer to palette structure
 */
void ui_render_rgba_controls(UIState* ui, const Palette* palette);

#endif  // UI_H
