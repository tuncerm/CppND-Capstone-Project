#ifndef UI_H
#define UI_H

#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdint.h>

#include "../shared/config/config_manager.h"
#include "../shared/text_renderer/text_renderer.h"
#include "../shared/ui_framework/ui_input.h"

#include "constants.h"

/**
 * UI layout constants (fallback defaults)
 */

// UI action codes emitted by ui_handle_mouse
#define UI_ACTION_NONE 0
#define UI_ACTION_SAVE 1
#define UI_ACTION_LOAD 2
#define UI_ACTION_NEW 3
#define UI_ACTION_QUIT 4
#define UI_ACTION_QUIT_CONFIRM 5

#define UI_ACTION_SPEC_HEALTH_SET_BASE 30
#define UI_ACTION_SPEC_DESTRUCT_SET_BASE 40
#define UI_ACTION_SPEC_MOVE_SET_BASE 50

/**
 * UI button structure
 */
typedef struct {
    UIInputElement input;
    char text[32];
} UIButton;

/**
 * UI state structure
 */
typedef struct {
    // Palette bar
    SDL_FRect palette_bar_rect;
    UIInputElement palette_swatches[16];
    int selected_palette_index;

    // Tile specs panel
    SDL_FRect tile_spec_rect;
    UIButton spec_health_buttons[8];
    UIButton spec_destruction_buttons[8];
    UIButton spec_movement_buttons[4];
    int spec_tile_id;
    uint8_t spec_health;
    uint8_t spec_destruction_mode;
    uint8_t spec_movement_mode;

    // Buttons
    UIButton save_button;
    UIButton load_button;
    UIButton new_button;
    UIButton quit_button;

    // Status
    char status_text[256];
    bool dirty_indicator;

    // Shared text rendering
    TextRenderer text_renderer;

    // Double-click tracking
    Uint64 last_click_time;
    int last_clicked_tile;

    // Quit confirmation dialog
    bool show_quit_dialog;
    UIButton quit_yes_button;
    UIButton quit_no_button;

    // One-frame action emitted from UI callbacks
    int pending_action;
} UIState;

/**
 * Initialize UI system
 * Sets up button positions and UI layout
 *
 * @param ui Pointer to UI state structure
 * @param renderer SDL renderer for creating textures
 * @return true if successful, false on error
 */
bool ui_init(UIState* ui, SDL_Renderer* renderer);

/**
 * Cleanup UI resources
 *
 * @param ui Pointer to UI state structure
 */
void ui_cleanup(UIState* ui);

/**
 * Update UI state
 *
 * @param ui Pointer to UI state structure
 * @param renderer SDL renderer
 */
void ui_update(UIState* ui, SDL_Renderer* renderer);

/**
 * Render UI components
 * Draws palette bar, buttons, and status text
 *
 * @param ui Pointer to UI state structure
 * @param renderer SDL renderer
 */
void ui_render(UIState* ui, SDL_Renderer* renderer);

/**
 * Handle mouse input for UI
 * Processes clicks on palette and buttons
 *
 * @param ui Pointer to UI state structure
 * @param mouse_x Mouse x coordinate
 * @param mouse_y Mouse y coordinate
 * @param clicked True if mouse was clicked
 * @param button Mouse button (1=left, 3=right)
 * @return UI action code (0=none, 1=save, 2=load, 3=new, 4=quit, 10+palette_index for palette
 * selection)
 */
int ui_handle_mouse(UIState* ui, int mouse_x, int mouse_y, bool clicked, int button);

/**
 * Set selected palette index
 *
 * @param ui Pointer to UI state structure
 * @param index Palette index (0-15)
 */
void ui_set_palette_selection(UIState* ui, int index);

/**
 * Get selected palette index
 *
 * @param ui Pointer to UI state structure
 * @return Selected palette index (0-15)
 */
int ui_get_palette_selection(const UIState* ui);

/**
 * Set status text
 *
 * @param ui Pointer to UI state structure
 * @param text Status text to display
 */
void ui_set_status(UIState* ui, const char* text);

/**
 * Set dirty indicator
 *
 * @param ui Pointer to UI state structure
 * @param dirty True if data is modified, false otherwise
 */
void ui_set_dirty(UIState* ui, bool dirty);

/**
 * Set selected tile spec values displayed in the UI panel.
 */
void ui_set_tile_spec(UIState* ui, int tile_id, uint8_t health, uint8_t destruction_mode,
                      uint8_t movement_mode);

/**
 * Check for double-click
 * Tracks click timing to detect double-clicks
 *
 * @param ui Pointer to UI state structure
 * @param tile_id Tile that was clicked
 * @return true if this is a double-click, false otherwise
 */
bool ui_check_double_click(UIState* ui, int tile_id);

#endif  // UI_H
