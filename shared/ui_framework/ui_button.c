#include "ui_button.h"
#include <stdlib.h>
#include <string.h>
#include "../constants.h"
#include "../text_renderer/text_renderer.h"

/**
 * Default button colors
 */
static const SDL_Color DEFAULT_BG_NORMAL = {60, 60, 60, 255};
static const SDL_Color DEFAULT_BG_HOVER = {80, 80, 80, 255};
static const SDL_Color DEFAULT_BG_PRESSED = {100, 100, 100, 255};
static const SDL_Color DEFAULT_BG_DISABLED = {40, 40, 40, 255};
static const SDL_Color DEFAULT_TEXT_COLOR = {255, 255, 255, 255};
static const SDL_Color DEFAULT_BORDER_COLOR = {128, 128, 128, 255};

/**
 * Initialize a button with default settings
 */
void ui_button_init(UIButton* button, int x, int y, int w, int h, const char* text) {
    if (!button) {
        return;
    }

    // Initialize basic properties
    button->rect = ui_make_rect(x, y, w, h);
    button->state = UI_BUTTON_NORMAL;
    button->on_click = NULL;
    button->userdata = NULL;
    button->visible = true;
    button->id = 0;

    // Set button text
    if (text) {
        strncpy(button->text, text, sizeof(button->text) - 1);
        button->text[sizeof(button->text) - 1] = '\0';
    } else {
        button->text[0] = '\0';
    }

    // Set default colors
    button->bg_color_normal = DEFAULT_BG_NORMAL;
    button->bg_color_hover = DEFAULT_BG_HOVER;
    button->bg_color_pressed = DEFAULT_BG_PRESSED;
    button->bg_color_disabled = DEFAULT_BG_DISABLED;
    button->text_color = DEFAULT_TEXT_COLOR;
    button->border_color = DEFAULT_BORDER_COLOR;
}

/**
 * Set button callback function
 */
void ui_button_set_callback(UIButton* button, UIButtonCallback callback, void* userdata) {
    if (!button) {
        return;
    }

    button->on_click = callback;
    button->userdata = userdata;
}

/**
 * Set button colors for different states
 */
void ui_button_set_colors(UIButton* button, SDL_Color normal, SDL_Color hover, SDL_Color pressed,
                          SDL_Color disabled) {
    if (!button) {
        return;
    }

    button->bg_color_normal = normal;
    button->bg_color_hover = hover;
    button->bg_color_pressed = pressed;
    button->bg_color_disabled = disabled;
}

/**
 * Set button text color
 */
void ui_button_set_text_color(UIButton* button, SDL_Color color) {
    if (!button) {
        return;
    }

    button->text_color = color;
}

/**
 * Handle mouse input for a single button
 */
bool ui_button_handle_input(UIButton* button, int mouse_x, int mouse_y, bool clicked) {
    if (!button || !button->visible) {
        return false;
    }

    // Check if disabled
    if (button->state & UI_BUTTON_DISABLED) {
        return false;
    }

    // Check if mouse is over button
    bool mouse_over = ui_point_in_rect(mouse_x, mouse_y, &button->rect);

    // Update hover state
    if (mouse_over) {
        button->state |= UI_BUTTON_HOVERED;
    } else {
        button->state &= ~UI_BUTTON_HOVERED;
    }

    // Handle click
    if (mouse_over && clicked) {
        button->state |= UI_BUTTON_PRESSED;

        // Execute callback if available
        if (button->on_click) {
            button->on_click(button->userdata);
        }

        return true;
    } else {
        button->state &= ~UI_BUTTON_PRESSED;
    }

    return false;
}

/**
 * Render a single button
 */
void ui_button_render(UIButton* button, SDL_Renderer* renderer, TextRenderer* text_renderer) {
    if (!button || !renderer || !button->visible) {
        return;
    }

    // Determine background color based on state
    SDL_Color bg_color;
    if (button->state & UI_BUTTON_DISABLED) {
        bg_color = button->bg_color_disabled;
    } else if (button->state & UI_BUTTON_PRESSED) {
        bg_color = button->bg_color_pressed;
    } else if (button->state & UI_BUTTON_HOVERED) {
        bg_color = button->bg_color_hover;
    } else {
        bg_color = button->bg_color_normal;
    }

    // Render button background
    ui_render_rect_f(renderer, &button->rect, bg_color);

    // Render button border
    ui_render_rect_outline_f(renderer, &button->rect, button->border_color);

    // Render button text
    if (button->text[0] != '\0') {
        // Calculate text position (centered)
        int text_width, text_height;
        text_get_dimensions(button->text, &text_width, &text_height);

        int text_x = (int)(button->rect.x + (button->rect.w - text_width) / 2);
        int text_y = (int)(button->rect.y + (button->rect.h - text_height) / 2);

        if (text_renderer) {
            // Use shared text renderer
            text_render_string(text_renderer, button->text, text_x, text_y, button->text_color);
        } else {
            // Fallback: simple text rendering using SDL primitives
            // This is a basic implementation for when text renderer is not available
            SDL_SetRenderDrawColor(renderer, button->text_color.r, button->text_color.g,
                                   button->text_color.b, button->text_color.a);

            // Draw simple rectangles to represent text characters
            int char_width = CHAR_WIDTH;
            for (int i = 0; i < (int)strlen(button->text) && i < MAX_CHARS_PER_LINE; i++) {
                SDL_FRect char_rect = {(float)(text_x + i * char_width), (float)(text_y + 2), 4.0f,
                                       8.0f};
                SDL_RenderFillRect(renderer, &char_rect);
            }
        }
    }
}

/**
 * Check if button is in specified state
 */
bool ui_button_has_state(const UIButton* button, UIButtonState state) {
    if (!button) {
        return false;
    }

    return (button->state & state) != 0;
}

/**
 * Set button state
 */
void ui_button_set_state(UIButton* button, UIButtonState state, bool enabled) {
    if (!button) {
        return;
    }

    if (enabled) {
        button->state |= state;
    } else {
        button->state &= ~state;
    }
}

// ===== Button Array Functions =====

/**
 * Initialize button array
 */
bool ui_button_array_init(UIButtonArray* array, int initial_capacity) {
    if (!array || initial_capacity <= 0) {
        return false;
    }

    array->buttons = malloc(sizeof(UIButton) * initial_capacity);
    if (!array->buttons) {
        return false;
    }

    array->count = 0;
    array->capacity = initial_capacity;
    array->hovered_button = -1;
    array->pressed_button = -1;

    return true;
}

/**
 * Cleanup button array and free memory
 */
void ui_button_array_cleanup(UIButtonArray* array) {
    if (!array) {
        return;
    }

    if (array->buttons) {
        free(array->buttons);
        array->buttons = NULL;
    }

    array->count = 0;
    array->capacity = 0;
    array->hovered_button = -1;
    array->pressed_button = -1;
}

/**
 * Add button to array
 */
int ui_button_array_add(UIButtonArray* array, const UIButton* button) {
    if (!array || !button) {
        return -1;
    }

    // Check if we need to resize
    if (array->count >= array->capacity) {
        int new_capacity = array->capacity * 2;
        UIButton* new_buttons = realloc(array->buttons, sizeof(UIButton) * new_capacity);
        if (!new_buttons) {
            return -1;  // Memory allocation failed
        }
        array->buttons = new_buttons;
        array->capacity = new_capacity;
    }

    // Copy button to array
    array->buttons[array->count] = *button;

    return array->count++;
}

/**
 * Get button by index
 */
UIButton* ui_button_array_get(UIButtonArray* array, int index) {
    if (!array || index < 0 || index >= array->count) {
        return NULL;
    }

    return &array->buttons[index];
}

/**
 * Handle mouse input for all buttons in array
 */
int ui_button_array_handle_input(UIButtonArray* array, int mouse_x, int mouse_y, bool clicked) {
    if (!array) {
        return -1;
    }

    array->hovered_button = -1;
    array->pressed_button = -1;

    for (int i = 0; i < array->count; i++) {
        UIButton* button = &array->buttons[i];

        if (ui_point_in_rect(mouse_x, mouse_y, &button->rect)) {
            array->hovered_button = i;
        }

        if (ui_button_handle_input(button, mouse_x, mouse_y, clicked)) {
            array->pressed_button = i;
            return i;  // Return index of clicked button
        }
    }

    return -1;  // No button clicked
}

/**
 * Render all buttons in array
 */
void ui_button_array_render(UIButtonArray* array, SDL_Renderer* renderer,
                            TextRenderer* text_renderer) {
    if (!array || !renderer) {
        return;
    }

    for (int i = 0; i < array->count; i++) {
        ui_button_render(&array->buttons[i], renderer, text_renderer);
    }
}

/**
 * Find button by ID
 */
int ui_button_array_find_by_id(const UIButtonArray* array, int id) {
    if (!array) {
        return -1;
    }

    for (int i = 0; i < array->count; i++) {
        if (array->buttons[i].id == id) {
            return i;
        }
    }

    return -1;
}
