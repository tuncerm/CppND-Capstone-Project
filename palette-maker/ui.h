#ifndef UI_H
#define UI_H

#include <SDL3/SDL.h>
#include "config.h"
#include "palette.h"

// --- UI Colors ---
#define COLOR_BACKGROUND {64, 64, 64, 255}
#define COLOR_PANEL_BG {32, 32, 32, 255}
#define COLOR_BUTTON_BG {64, 64, 64, 255}
#define COLOR_VALUE_BG {48, 48, 48, 255}
#define COLOR_WHITE {255, 255, 255, 255}
#define COLOR_RED {255, 0, 0, 255}
#define COLOR_SELECTION {255, 255, 255, 255}
#define COLOR_PICKER_BG {40, 40, 40, 240}
#define COLOR_DIALOG_BG {0, 0, 0, 200}

// --- UI Layout & Sizing ---

// Swatch Grid
#define GRID_COLS 4
#define GRID_ROWS 4
#define SWATCH_SIZE 45
#define SWATCH_BORDER 2
#define SWATCH_SPACING (SWATCH_SIZE + SWATCH_BORDER)
#define GRID_MARGIN_X 20
#define GRID_MARGIN_Y 20
#define SELECTION_BORDER_WIDTH 2
#define SWATCH_TEXT_OFFSET_X 2
#define SWATCH_TEXT_OFFSET_Y 2

// Main UI Panel
#define PANEL_MARGIN_X 20
#define PANEL_MARGIN_Y 20
#define PANEL_WIDTH 320
#define PANEL_HEIGHT 300
#define PANEL_X (GRID_MARGIN_X + (GRID_COLS * SWATCH_SPACING) + PANEL_MARGIN_X)
#define PANEL_Y GRID_MARGIN_Y

// RGBA Controls within Panel
#define RGBA_CONTROL_MARGIN_X 10
#define RGBA_CONTROL_MARGIN_Y 20
#define RGBA_CONTROL_START_X (PANEL_X + RGBA_CONTROL_MARGIN_X)
#define RGBA_CONTROL_START_Y (PANEL_Y + RGBA_CONTROL_MARGIN_Y)
#define RGBA_ROW_HEIGHT 35
#define RGBA_BUTTON_WIDTH 35
#define RGBA_BUTTON_HEIGHT 25
#define RGBA_BUTTON_SPACING 5
#define RGBA_VALUE_DISPLAY_WIDTH 45

// RGBA Control component X positions (relative to row start)
#define RGBA_X_LABEL 0
#define RGBA_X_BTN_MINUS_10 (RGBA_LABEL_WIDTH + 5)
#define RGBA_X_BTN_MINUS_1 (RGBA_X_BTN_MINUS_10 + RGBA_BUTTON_WIDTH + RGBA_BUTTON_SPACING)
#define RGBA_X_VALUE_BG (RGBA_X_BTN_MINUS_1 + RGBA_BUTTON_WIDTH + RGBA_BUTTON_SPACING)
#define RGBA_X_BTN_PLUS_1 (RGBA_X_VALUE_BG + RGBA_VALUE_DISPLAY_WIDTH + RGBA_BUTTON_SPACING)
#define RGBA_X_BTN_PLUS_10 (RGBA_X_BTN_PLUS_1 + RGBA_BUTTON_WIDTH + RGBA_BUTTON_SPACING)
#define RGBA_X_BTN_RESET (RGBA_X_BTN_PLUS_10 + RGBA_BUTTON_WIDTH + RGBA_BUTTON_SPACING)
#define RGBA_LABEL_WIDTH 45

// Action Buttons (Save, Reset)
#define ACTION_BUTTON_Y_OFFSET 250
#define ACTION_BUTTON_Y (PANEL_Y + ACTION_BUTTON_Y_OFFSET)
#define ACTION_BUTTON_WIDTH 100
#define ACTION_BUTTON_HEIGHT 30
#define ACTION_BUTTON_SPACING 10
#define SAVE_BUTTON_X (PANEL_X + RGBA_CONTROL_MARGIN_X)
#define RESET_BUTTON_X (SAVE_BUTTON_X + ACTION_BUTTON_WIDTH + ACTION_BUTTON_SPACING)
#define LOAD_BUTTON_X (RESET_BUTTON_X + ACTION_BUTTON_WIDTH + ACTION_BUTTON_SPACING)

// Modified Indicator
#define MODIFIED_INDICATOR_Y_OFFSET 35
#define MODIFIED_INDICATOR_Y (ACTION_BUTTON_Y + MODIFIED_INDICATOR_Y_OFFSET)
#define MODIFIED_INDICATOR_X (PANEL_X + RGBA_CONTROL_MARGIN_X)

// --- Color Picker Dialog ---
#define PICKER_WIDTH 240
#define PICKER_HEIGHT 120
#define PICKER_X ((config->window_width - PICKER_WIDTH) / 2)
#define PICKER_Y ((config->window_height - PICKER_HEIGHT) / 2)
#define PICKER_MARGIN_X 10
#define PICKER_MARGIN_Y 10
#define PICKER_TITLE_Y (PICKER_Y + PICKER_MARGIN_Y)
#define PICKER_SUBTITLE_Y (PICKER_TITLE_Y + 15)
#define PICKER_PREVIEW_SIZE 40
#define PICKER_PREVIEW_X (PICKER_X + PICKER_WIDTH - PICKER_PREVIEW_SIZE - PICKER_MARGIN_X)
#define PICKER_PREVIEW_Y (PICKER_Y + PICKER_MARGIN_Y)
#define PICKER_HELP_TEXT_Y1 (PICKER_SUBTITLE_Y + 35)
#define PICKER_HELP_TEXT_Y2 (PICKER_HELP_TEXT_Y1 + 15)
#define PICKER_HELP_TEXT_Y3 (PICKER_HELP_TEXT_Y2 + 15)

// --- Save Dialog ---
#define SAVE_DIALOG_WIDTH 220
#define SAVE_DIALOG_HEIGHT 80
#define SAVE_DIALOG_X ((config->window_width - SAVE_DIALOG_WIDTH) / 2)
#define SAVE_DIALOG_Y ((config->window_height - SAVE_DIALOG_HEIGHT) / 2)
#define SAVE_DIALOG_MARGIN_X 10
#define SAVE_DIALOG_MARGIN_Y 10
#define SAVE_DIALOG_TITLE_Y (SAVE_DIALOG_Y + SAVE_DIALOG_MARGIN_Y)
#define SAVE_DIALOG_TEXT_Y1 (SAVE_DIALOG_TITLE_Y + 20)
#define SAVE_DIALOG_TEXT_Y2 (SAVE_DIALOG_TEXT_Y1 + 20)

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
    float mouse_x, mouse_y;  // Use float for scaled coordinates
    bool mouse_down;
    Uint32 last_click_time;  // For double-click detection
    int last_click_swatch;   // Last clicked swatch for double-click

} UIState;

/**
 * Get UI scale factor for mouse coordinates
 *
 * @param ui Pointer to UI state structure
 * @param scale_x Output for X scale factor
 * @param scale_y Output for Y scale factor
 */
void ui_get_scale_factor(UIState* ui, float* scale_x, float* scale_y);

/**
 * Initialize UI system
 * Creates window, renderer, and initializes UI state
 *
 * @param ui Pointer to UI state structure
 * @return true if successful, false on error
 */
bool ui_init(UIState* ui, const AppConfig* config);

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
 * @param config The application configuration
 * @return true to continue main loop, false to quit
 */
bool ui_handle_event(UIState* ui, Palette* palette, SDL_Event* event, const AppConfig* config);

/**
 * Render the complete UI
 * Draws swatch grid, input fields, buttons, and dialogs
 *
 * @param ui Pointer to UI state structure
 * @param palette Pointer to palette structure
 * @param config The application configuration
 */
void ui_render(UIState* ui, const Palette* palette, const AppConfig* config);

/**
 * Get swatch index from mouse coordinates
 * Returns -1 if mouse is not over any swatch
 *
 * @param x Mouse X coordinate
 * @param y Mouse Y coordinate
 * @return Swatch index (0-15) or -1 if not over swatch
 */
int ui_get_swatch_at_position(float x, float y, const AppConfig* config);

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
bool ui_handle_rgba_button_click(UIState* ui, Palette* palette, float x, float y,
                                 const AppConfig* config);

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
void ui_render_rgba_controls(UIState* ui, const Palette* palette, const AppConfig* config);

#endif  // UI_H
