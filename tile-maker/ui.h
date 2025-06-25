#ifndef UI_H
#define UI_H

#include <SDL3/SDL.h>
#include <stdbool.h>

/**
 * UI layout constants
 */
#define WINDOW_WIDTH 900
#define WINDOW_HEIGHT 600
#define PALETTE_BAR_HEIGHT 50
#define BUTTON_WIDTH 80
#define BUTTON_HEIGHT 30
#define PALETTE_SWATCH_SIZE 30

/**
 * UI button structure
 */
typedef struct {
    SDL_FRect rect;
    char text[32];
    bool pressed;
    bool hovered;
} UIButton;

/**
 * UI state structure
 */
typedef struct {
    // Palette bar
    SDL_FRect palette_bar_rect;
    SDL_FRect palette_swatches[16];
    int selected_palette_index;
    int hover_palette_index;

    // Buttons
    UIButton save_button;
    UIButton load_button;
    UIButton new_button;
    UIButton quit_button;

    // Status
    char status_text[256];
    bool dirty_indicator;

    // Font rendering (simple text)
    SDL_Texture* font_texture;

    // Double-click tracking
    Uint64 last_click_time;
    int last_clicked_tile;
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
 * Check for double-click
 * Tracks click timing to detect double-clicks
 *
 * @param ui Pointer to UI state structure
 * @param tile_id Tile that was clicked
 * @return true if this is a double-click, false otherwise
 */
bool ui_check_double_click(UIState* ui, int tile_id);

/**
 * Render a simple filled rectangle button
 *
 * @param renderer SDL renderer
 * @param button Pointer to button structure
 */
void render_button(SDL_Renderer* renderer, const UIButton* button);

/**
 * Render simple text (basic implementation)
 *
 * @param renderer SDL renderer
 * @param text Text to render
 * @param x X position
 * @param y Y position
 * @param color Text color
 */
void render_text(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color);

/**
 * Check if point is inside rectangle
 *
 * @param x Point x coordinate
 * @param y Point y coordinate
 * @param rect Rectangle to test
 * @return true if point is inside rectangle, false otherwise
 */
bool point_in_rect(int x, int y, const SDL_FRect* rect);

#endif  // UI_H
