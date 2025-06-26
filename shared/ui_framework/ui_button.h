#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include <SDL3/SDL.h>
#include <stdbool.h>
#include "ui_primitives.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * UI Button System for Shared Component Library
 *
 * Provides unified button functionality combining the best features from
 * palette-maker's coordinate-based system and tile-maker's structured approach.
 * Supports hover states, press feedback, and callback functions.
 */

// Forward declaration for text renderer
typedef struct TextRenderer TextRenderer;

/**
 * Button state flags
 */
typedef enum {
    UI_BUTTON_NORMAL = 0x00,
    UI_BUTTON_HOVERED = 0x01,
    UI_BUTTON_PRESSED = 0x02,
    UI_BUTTON_DISABLED = 0x04,
    UI_BUTTON_SELECTED = 0x08
} UIButtonState;

/**
 * Button callback function type
 * Called when button is clicked
 */
typedef void (*UIButtonCallback)(void* userdata);

/**
 * UI Button structure
 * Combines coordinate precision with structured state management
 */
typedef struct {
    SDL_FRect rect;             // Button bounds
    char text[32];              // Button text
    UIButtonState state;        // Current button state
    UIButtonCallback on_click;  // Click callback function
    void* userdata;             // User data for callback

    // Visual styling
    SDL_Color bg_color_normal;    // Normal background color
    SDL_Color bg_color_hover;     // Hover background color
    SDL_Color bg_color_pressed;   // Pressed background color
    SDL_Color bg_color_disabled;  // Disabled background color
    SDL_Color text_color;         // Text color
    SDL_Color border_color;       // Border color

    // Internal state
    bool visible;  // Whether button is visible
    int id;        // Optional button ID for identification
} UIButton;

/**
 * Button array manager for handling multiple buttons efficiently
 */
typedef struct {
    UIButton* buttons;   // Array of buttons
    int count;           // Number of buttons
    int capacity;        // Array capacity
    int hovered_button;  // Index of currently hovered button (-1 if none)
    int pressed_button;  // Index of currently pressed button (-1 if none)
} UIButtonArray;

// ===== Single Button Functions =====

/**
 * Initialize a button with default settings
 *
 * @param button Button to initialize
 * @param x X coordinate
 * @param y Y coordinate
 * @param w Width
 * @param h Height
 * @param text Button text
 */
void ui_button_init(UIButton* button, int x, int y, int w, int h, const char* text);

/**
 * Set button callback function
 *
 * @param button Button to modify
 * @param callback Callback function
 * @param userdata User data passed to callback
 */
void ui_button_set_callback(UIButton* button, UIButtonCallback callback, void* userdata);

/**
 * Set button colors for different states
 *
 * @param button Button to modify
 * @param normal Normal state color
 * @param hover Hover state color
 * @param pressed Pressed state color
 * @param disabled Disabled state color
 */
void ui_button_set_colors(UIButton* button, SDL_Color normal, SDL_Color hover, SDL_Color pressed,
                          SDL_Color disabled);

/**
 * Set button text color
 *
 * @param button Button to modify
 * @param color Text color
 */
void ui_button_set_text_color(UIButton* button, SDL_Color color);

/**
 * Handle mouse input for a single button
 *
 * @param button Button to process
 * @param mouse_x Mouse X coordinate
 * @param mouse_y Mouse Y coordinate
 * @param clicked Whether mouse was clicked this frame
 * @return true if button was clicked and callback executed
 */
bool ui_button_handle_input(UIButton* button, int mouse_x, int mouse_y, bool clicked);

/**
 * Render a single button
 *
 * @param button Button to render
 * @param renderer SDL renderer
 * @param text_renderer Text renderer for button text (can be NULL for simple rendering)
 */
void ui_button_render(UIButton* button, SDL_Renderer* renderer, TextRenderer* text_renderer);

/**
 * Check if button is in specified state
 *
 * @param button Button to check
 * @param state State to check for
 * @return true if button is in the specified state
 */
bool ui_button_has_state(const UIButton* button, UIButtonState state);

/**
 * Set button state
 *
 * @param button Button to modify
 * @param state State to set
 * @param enabled Whether to enable or disable the state
 */
void ui_button_set_state(UIButton* button, UIButtonState state, bool enabled);

// ===== Button Array Functions =====

/**
 * Initialize button array
 *
 * @param array Button array to initialize
 * @param initial_capacity Initial capacity for buttons
 * @return true if successful, false on memory allocation error
 */
bool ui_button_array_init(UIButtonArray* array, int initial_capacity);

/**
 * Cleanup button array and free memory
 *
 * @param array Button array to cleanup
 */
void ui_button_array_cleanup(UIButtonArray* array);

/**
 * Add button to array
 *
 * @param array Button array
 * @param button Button to add (will be copied)
 * @return Index of added button, -1 on error
 */
int ui_button_array_add(UIButtonArray* array, const UIButton* button);

/**
 * Get button by index
 *
 * @param array Button array
 * @param index Button index
 * @return Pointer to button, NULL if invalid index
 */
UIButton* ui_button_array_get(UIButtonArray* array, int index);

/**
 * Handle mouse input for all buttons in array
 *
 * @param array Button array
 * @param mouse_x Mouse X coordinate
 * @param mouse_y Mouse Y coordinate
 * @param clicked Whether mouse was clicked this frame
 * @return Index of clicked button, -1 if none clicked
 */
int ui_button_array_handle_input(UIButtonArray* array, int mouse_x, int mouse_y, bool clicked);

/**
 * Render all buttons in array
 *
 * @param array Button array
 * @param renderer SDL renderer
 * @param text_renderer Text renderer for button text
 */
void ui_button_array_render(UIButtonArray* array, SDL_Renderer* renderer,
                            TextRenderer* text_renderer);

/**
 * Find button by ID
 *
 * @param array Button array
 * @param id Button ID to search for
 * @return Index of button with matching ID, -1 if not found
 */
int ui_button_array_find_by_id(const UIButtonArray* array, int id);

#ifdef __cplusplus
}
#endif

#endif  // UI_BUTTON_H
