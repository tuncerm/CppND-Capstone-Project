#include "ui.h"
#include <stdio.h>
#include <string.h>
#include "palette_io.h"

/**
 * Initialize UI system
 */
bool ui_init(UIState* ui, SDL_Renderer* renderer) {
    if (!ui || !renderer) {
        return false;
    }

    // Initialize palette bar
    ui->palette_bar_rect.x = 10;
    ui->palette_bar_rect.y = WINDOW_HEIGHT - PALETTE_BAR_HEIGHT - 10;
    ui->palette_bar_rect.w = WINDOW_WIDTH - 20;
    ui->palette_bar_rect.h = PALETTE_BAR_HEIGHT;

    // Initialize palette swatches (16 colors in a row)
    int swatch_spacing = (ui->palette_bar_rect.w - 20) / 16;
    for (int i = 0; i < 16; i++) {
        ui->palette_swatches[i].x = ui->palette_bar_rect.x + 10 + i * swatch_spacing;
        ui->palette_swatches[i].y = ui->palette_bar_rect.y + 10;
        ui->palette_swatches[i].w = PALETTE_SWATCH_SIZE;
        ui->palette_swatches[i].h = PALETTE_SWATCH_SIZE;
    }

    ui->selected_palette_index = 1;  // Start with palette index 1 (not black)
    ui->hover_palette_index = -1;

    // Initialize buttons
    int button_y = 10;
    int button_spacing = BUTTON_WIDTH + 10;

    // Save button
    ui->save_button.rect.x = 10;
    ui->save_button.rect.y = button_y;
    ui->save_button.rect.w = BUTTON_WIDTH;
    ui->save_button.rect.h = BUTTON_HEIGHT;
    strcpy(ui->save_button.text, "Save (S)");
    ui->save_button.pressed = false;
    ui->save_button.hovered = false;

    // Load button
    ui->load_button.rect.x = 10 + button_spacing;
    ui->load_button.rect.y = button_y;
    ui->load_button.rect.w = BUTTON_WIDTH;
    ui->load_button.rect.h = BUTTON_HEIGHT;
    strcpy(ui->load_button.text, "Load (L)");
    ui->load_button.pressed = false;
    ui->load_button.hovered = false;

    // New button
    ui->new_button.rect.x = 10 + button_spacing * 2;
    ui->new_button.rect.y = button_y;
    ui->new_button.rect.w = BUTTON_WIDTH;
    ui->new_button.rect.h = BUTTON_HEIGHT;
    strcpy(ui->new_button.text, "New");
    ui->new_button.pressed = false;
    ui->new_button.hovered = false;

    // Quit button
    ui->quit_button.rect.x = 10 + button_spacing * 3;
    ui->quit_button.rect.y = button_y;
    ui->quit_button.rect.w = BUTTON_WIDTH;
    ui->quit_button.rect.h = BUTTON_HEIGHT;
    strcpy(ui->quit_button.text, "Quit (ESC)");
    ui->quit_button.pressed = false;
    ui->quit_button.hovered = false;

    // Initialize status
    strcpy(ui->status_text, "Tile Maker Ready");
    ui->dirty_indicator = false;

    // Initialize font texture (placeholder)
    ui->font_texture = NULL;

    // Initialize double-click tracking
    ui->last_click_time = 0;
    ui->last_clicked_tile = -1;

    printf("UI initialized\n");
    return true;
}

/**
 * Cleanup UI resources
 */
void ui_cleanup(UIState* ui) {
    if (!ui)
        return;

    if (ui->font_texture) {
        SDL_DestroyTexture(ui->font_texture);
        ui->font_texture = NULL;
    }

    printf("UI cleaned up\n");
}

/**
 * Update UI state
 */
void ui_update(UIState* ui, SDL_Renderer* renderer) {
    if (!ui || !renderer)
        return;

    // Reset button states
    ui->save_button.pressed = false;
    ui->load_button.pressed = false;
    ui->new_button.pressed = false;
    ui->quit_button.pressed = false;
}

/**
 * Render UI components
 */
void ui_render(UIState* ui, SDL_Renderer* renderer) {
    if (!ui || !renderer)
        return;

    // Render palette bar background
    SDL_SetRenderDrawColor(renderer, 48, 48, 48, 255);
    SDL_RenderFillRect(renderer, &ui->palette_bar_rect);

    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    SDL_RenderRect(renderer, &ui->palette_bar_rect);

    // Render palette swatches
    for (int i = 0; i < 16; i++) {
        SDL_Color color = palette_get_sdl_color(i);
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, &ui->palette_swatches[i]);

        // Draw swatch border
        if (i == ui->selected_palette_index) {
            // Selected swatch gets white border
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderRect(renderer, &ui->palette_swatches[i]);

            // Double border for emphasis
            SDL_FRect outer_rect = ui->palette_swatches[i];
            outer_rect.x -= 1;
            outer_rect.y -= 1;
            outer_rect.w += 2;
            outer_rect.h += 2;
            SDL_RenderRect(renderer, &outer_rect);
        } else if (i == ui->hover_palette_index) {
            // Hovered swatch gets light gray border
            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            SDL_RenderRect(renderer, &ui->palette_swatches[i]);
        } else {
            // Normal swatch gets dark gray border
            SDL_SetRenderDrawColor(renderer, 96, 96, 96, 255);
            SDL_RenderRect(renderer, &ui->palette_swatches[i]);
        }
    }

    // Render buttons
    render_button(renderer, &ui->save_button);
    render_button(renderer, &ui->load_button);
    render_button(renderer, &ui->new_button);
    render_button(renderer, &ui->quit_button);

    // Render status text
    SDL_Color text_color = {255, 255, 255, 255};
    render_text(renderer, ui->status_text, 10, WINDOW_HEIGHT - 80, text_color);

    // Render dirty indicator
    if (ui->dirty_indicator) {
        SDL_SetRenderDrawColor(renderer, 255, 200, 0, 255);
        SDL_FRect dirty_rect = {WINDOW_WIDTH - 30, 10, 20, 20};
        SDL_RenderFillRect(renderer, &dirty_rect);

        render_text(renderer, "*", WINDOW_WIDTH - 25, 15, text_color);
    }
}

/**
 * Handle mouse input for UI
 */
int ui_handle_mouse(UIState* ui, int mouse_x, int mouse_y, bool clicked, int button) {
    if (!ui)
        return 0;

    // Reset hover states
    ui->hover_palette_index = -1;
    ui->save_button.hovered = false;
    ui->load_button.hovered = false;
    ui->new_button.hovered = false;
    ui->quit_button.hovered = false;

    // Check palette swatches
    for (int i = 0; i < 16; i++) {
        if (point_in_rect(mouse_x, mouse_y, &ui->palette_swatches[i])) {
            ui->hover_palette_index = i;
            if (clicked && button == 1) {  // Left click
                ui->selected_palette_index = i;
                return 10 + i;  // Return palette selection code
            }
            break;
        }
    }

    // Check buttons
    if (point_in_rect(mouse_x, mouse_y, &ui->save_button.rect)) {
        ui->save_button.hovered = true;
        if (clicked && button == 1) {
            ui->save_button.pressed = true;
            return 1;  // Save action
        }
    }

    if (point_in_rect(mouse_x, mouse_y, &ui->load_button.rect)) {
        ui->load_button.hovered = true;
        if (clicked && button == 1) {
            ui->load_button.pressed = true;
            return 2;  // Load action
        }
    }

    if (point_in_rect(mouse_x, mouse_y, &ui->new_button.rect)) {
        ui->new_button.hovered = true;
        if (clicked && button == 1) {
            ui->new_button.pressed = true;
            return 3;  // New action
        }
    }

    if (point_in_rect(mouse_x, mouse_y, &ui->quit_button.rect)) {
        ui->quit_button.hovered = true;
        if (clicked && button == 1) {
            ui->quit_button.pressed = true;
            return 4;  // Quit action
        }
    }

    return 0;  // No action
}

/**
 * Set selected palette index
 */
void ui_set_palette_selection(UIState* ui, int index) {
    if (!ui)
        return;

    if (index >= 0 && index < 16) {
        ui->selected_palette_index = index;
    }
}

/**
 * Get selected palette index
 */
int ui_get_palette_selection(const UIState* ui) {
    if (!ui)
        return 0;
    return ui->selected_palette_index;
}

/**
 * Set status text
 */
void ui_set_status(UIState* ui, const char* text) {
    if (!ui || !text)
        return;

    strncpy(ui->status_text, text, sizeof(ui->status_text) - 1);
    ui->status_text[sizeof(ui->status_text) - 1] = '\0';
}

/**
 * Set dirty indicator
 */
void ui_set_dirty(UIState* ui, bool dirty) {
    if (!ui)
        return;
    ui->dirty_indicator = dirty;
}

/**
 * Check for double-click
 */
bool ui_check_double_click(UIState* ui, int tile_id) {
    if (!ui)
        return false;

    Uint64 current_time = SDL_GetTicks();
    bool is_double_click = false;

    if (ui->last_clicked_tile == tile_id &&
        (current_time - ui->last_click_time) < 500) {  // 500ms double-click threshold
        is_double_click = true;
    }

    ui->last_click_time = current_time;
    ui->last_clicked_tile = tile_id;

    return is_double_click;
}

/**
 * Render a simple filled rectangle button
 */
void render_button(SDL_Renderer* renderer, const UIButton* button) {
    if (!renderer || !button)
        return;

    // Button background
    if (button->pressed) {
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    } else if (button->hovered) {
        SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
    }
    SDL_RenderFillRect(renderer, &button->rect);

    // Button border
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    SDL_RenderRect(renderer, &button->rect);

    // Button text (simple)
    SDL_Color text_color = {255, 255, 255, 255};
    render_text(renderer, button->text, button->rect.x + 5, button->rect.y + 8, text_color);
}

/**
 * Render simple text (basic implementation)
 * This is a placeholder - in a full implementation you'd want proper font rendering
 */
void render_text(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color) {
    if (!renderer || !text)
        return;

    // This is a very basic text rendering placeholder
    // In a real implementation, you would use SDL_ttf or another font library

    // For now, just render a simple rectangle as a placeholder for text
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    int text_width = strlen(text) * 6;  // Approximate character width
    SDL_FRect text_rect = {x, y, text_width, 12};
    SDL_RenderRect(renderer, &text_rect);

    // Draw small rectangles to represent characters
    for (int i = 0; i < (int)strlen(text) && i < 20; i++) {
        SDL_FRect char_rect = {x + i * 6, y + 2, 4, 8};
        SDL_RenderFillRect(renderer, &char_rect);
    }
}

/**
 * Check if point is inside rectangle
 */
bool point_in_rect(int x, int y, const SDL_FRect* rect) {
    if (!rect)
        return false;

    return (x >= rect->x && x < rect->x + rect->w && y >= rect->y && y < rect->y + rect->h);
}
