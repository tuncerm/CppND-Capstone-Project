#include "ui.h"
#include <assert.h>
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

    // Initialize quit confirmation dialog
    ui->show_quit_dialog = false;
    ui->quit_yes_button.rect = (SDL_FRect){WINDOW_WIDTH / 2 - 110, WINDOW_HEIGHT / 2, 100, 40};
    strcpy(ui->quit_yes_button.text, "Yes");
    ui->quit_no_button.rect = (SDL_FRect){WINDOW_WIDTH / 2 + 10, WINDOW_HEIGHT / 2, 100, 40};
    strcpy(ui->quit_no_button.text, "No");

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

    // Render quit confirmation dialog
    if (ui->show_quit_dialog) {
        SDL_FRect dialog_rect = {WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 - 50, 300, 120};
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 230);
        SDL_RenderFillRect(renderer, &dialog_rect);
        SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
        SDL_RenderRect(renderer, &dialog_rect);

        render_text(renderer, "Unsaved changes! Quit?", dialog_rect.x + 10, dialog_rect.y + 10,
                    (SDL_Color){255, 255, 255, 255});
        render_button(renderer, &ui->quit_yes_button);
        render_button(renderer, &ui->quit_no_button);
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

    // Handle quit dialog input
    if (ui->show_quit_dialog) {
        if (point_in_rect(mouse_x, mouse_y, &ui->quit_yes_button.rect)) {
            if (clicked)
                return 5;  // Quit confirmed
        }
        if (point_in_rect(mouse_x, mouse_y, &ui->quit_no_button.rect)) {
            if (clicked)
                ui->show_quit_dialog = false;
        }
        return 0;  // Absorb input
    }

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

/* ----------------------------------------------------------------
 * Simple 5×7 bitmap font with extended special‑character support
 * ----------------------------------------------------------------
 *==============================
 * 1. Character → glyph index
 *==============================*/
static inline int get_char_index(char c) {
    if (c == ' ')
        return 0;
    if (c >= '0' && c <= '9')
        return 1 + (c - '0'); /* 1‑10 */
    if (c >= 'A' && c <= 'Z')
        return 11 + (c - 'A'); /* 11‑36 */
    if (c >= 'a' && c <= 'z')
        return 11 + (c - 'a'); /* map lowercase → uppercase */

    /* Ordered ASCII specials → 37‑59 */
    switch (c) {
        case '.':
            return 37; /* period */
        case ',':
            return 38; /* comma  */
        case ';':
            return 39; /* semicolon */
        case ':':
            return 40; /* colon */
        case '!':
            return 41;
        case '?':
            return 42;
        case '-':
            return 43;
        case '_':
            return 44;
        case '+':
            return 45;
        case '=':
            return 46;
        case '*':
            return 47;
        case '/':
            return 48;
        case '\\':
            return 49;
        case '<':
            return 50;
        case '>':
            return 51;
        case '(':
            return 52;
        case ')':
            return 53;
        case '&':
            return 54;
        case '%':
            return 55;
        case '\'':
            return 56; /* apostrophe */
        case '"':
            return 57; /* double quote */
        /* 58 → right‑arrow, 59 → left‑arrow (no ASCII) */
        default:
            return 0; /* unknown → space */
    }
}

/* Total number of glyphs in the table below */
#define GLYPH_COUNT 60 /* indices 0‑59 */

/*==============================
 * 2. 5×7 glyph bit‑patterns
 *==============================*/
/* Each glyph: 7 rows × 5 cols, MSB is left‑most pixel */
static const uint8_t font_patterns[GLYPH_COUNT][7] = {
    /* 0  : space */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},

    /* 1‑10 : '0'‑'9' */
    {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E}, /* 1 → '0' */
    {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E}, /* 2 → '1' */
    {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F}, /* 3 → '2' */
    {0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E}, /* 4 → '3' */
    {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02}, /* 5 → '4' */
    {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E}, /* 6 → '5' */
    {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E}, /* 7 → '6' */
    {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08}, /* 8 → '7' */
    {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E}, /* 9 → '8' */
    {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C}, /* 10 → '9' */

    /* 11‑36 : 'A'‑'Z' */
    {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}, /* 11 */
    {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E}, /* 12 */
    {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E}, /* 13 */
    {0x1C, 0x12, 0x11, 0x11, 0x11, 0x12, 0x1C}, /* 14 */
    {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F}, /* 15 */
    {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10}, /* 16 */
    {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F}, /* 17 */
    {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}, /* 18 */
    {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E}, /* 19 */
    {0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C}, /* 20 */
    {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11}, /* 21 */
    {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F}, /* 22 */
    {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11}, /* 23 */
    {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11}, /* 24 */
    {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}, /* 25 */
    {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10}, /* 26 */
    {0x0E, 0x11, 0x11, 0x15, 0x12, 0x0E, 0x01}, /* 27 */
    {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11}, /* 28 */
    {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E}, /* 29 */
    {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}, /* 30 */
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}, /* 31 */
    {0x11, 0x11, 0x11, 0x11, 0x0A, 0x0A, 0x04}, /* 32 */
    {0x11, 0x11, 0x11, 0x15, 0x15, 0x1B, 0x11}, /* 33 */
    {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11}, /* 34 */
    {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04}, /* 35 */
    {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F}, /* 36 */

    /* 37‑59 : ordered specials */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00}, /* 37 '.' */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x08}, /* 38 ',' */
    {0x00, 0x00, 0x04, 0x00, 0x00, 0x04, 0x08}, /* 39 ';' */
    {0x00, 0x00, 0x04, 0x00, 0x00, 0x04, 0x00}, /* 40 ':' */
    {0x04, 0x04, 0x04, 0x04, 0x00, 0x04, 0x00}, /* 41 '!' */
    {0x0E, 0x11, 0x02, 0x04, 0x00, 0x04, 0x00}, /* 42 '?' */
    {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00}, /* 43 '-' */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F}, /* 44 '_' */
    {0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00}, /* 45 '+' */
    {0x00, 0x0E, 0x00, 0x0E, 0x00, 0x0E, 0x00}, /* 46 '=' */
    {0x04, 0x0A, 0x11, 0x1F, 0x11, 0x11, 0x11}, /* 47 '*' */
    {0x01, 0x02, 0x04, 0x08, 0x10, 0x00, 0x00}, /* 48 '/' */
    {0x10, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00}, /* 49 '\' */
    {0x00, 0x04, 0x08, 0x10, 0x08, 0x04, 0x00}, /* 50 '<' */
    {0x00, 0x10, 0x08, 0x04, 0x08, 0x10, 0x00}, /* 51 '>' */
    {0x08, 0x0C, 0x0A, 0x09, 0x0A, 0x0C, 0x08}, /* 52 '(' */
    {0x02, 0x06, 0x0A, 0x12, 0x0A, 0x06, 0x02}, /* 53 ')' */
    {0x0A, 0x15, 0x15, 0x0E, 0x04, 0x04, 0x04}, /* 54 '&' */
    {0x06, 0x09, 0x06, 0x15, 0x09, 0x09, 0x16}, /* 55 '%' */
    {0x02, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00}, /* 56 '\'' */
    {0x0A, 0x0A, 0x0A, 0x00, 0x00, 0x00, 0x00}, /* 57 '"' */
    {0x00, 0x04, 0x02, 0x1F, 0x02, 0x04, 0x00}, /* 58 '→' */
    {0x00, 0x02, 0x04, 0x1F, 0x04, 0x02, 0x00}, /* 59 '←' */
};

/*==============================
 * 3. Sanity check (debug builds)
 *==============================*/
static inline void font_assert_init(void) {
#if !defined(NDEBUG)
    assert(GLYPH_COUNT == (int)(sizeof font_patterns / sizeof font_patterns[0]));
#endif
}

/*==============================
 * 4. Text‑render helper
 *==============================*/
void render_text(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color) {
    if (!renderer || !text) {
        return;
    }

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    int len = (int)strlen(text);
    for (int i = 0; i < len && i < 32; i++) {
        int char_index = get_char_index(text[i]);
        const uint8_t* pattern = font_patterns[char_index < 52 ? char_index : 0];

        // Draw character using bitmap pattern
        for (int row = 0; row < 7; row++) {
            for (int col = 0; col < 5; col++) {
                if (pattern[row] & (1 << (4 - col))) {
                    SDL_FRect pixel = {(float)(x + i * 6 + col), (float)(y + row), 1.0f, 1.0f};
                    SDL_RenderFillRect(renderer, &pixel);
                }
            }
        }
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
