#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Double-click detection threshold (milliseconds)
#define DOUBLE_CLICK_TIME 300

/**
 * Initialize UI system
 * Creates SDL window and renderer with proper settings
 */
bool ui_init(UIState* ui) {
    if (!ui)
        return false;

    // Initialize UI state
    memset(ui, 0, sizeof(UIState));
    ui->selected_swatch = 0;
    ui->color_picker_open = false;
    ui->show_save_dialog = false;
    ui->last_click_swatch = -1;

    // Create window
    ui->window =
        SDL_CreateWindow("Palette Maker - SDL3", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);

    if (!ui->window) {
        printf("Error: Could not create window: %s\n", SDL_GetError());
        return false;
    }

    // Create renderer with VSync
    ui->renderer = SDL_CreateRenderer(ui->window, NULL);
    if (!ui->renderer) {
        printf("Error: Could not create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(ui->window);
        return false;
    }

    // Set renderer logical size for consistent UI scaling
    SDL_SetRenderLogicalPresentation(ui->renderer, WINDOW_WIDTH, WINDOW_HEIGHT,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);

    printf("UI initialized successfully\n");
    return true;
}

/**
 * Cleanup UI system and free resources
 */
void ui_cleanup(UIState* ui) {
    if (!ui)
        return;

    if (ui->font_texture) {
        SDL_DestroyTexture(ui->font_texture);
    }
    if (ui->renderer) {
        SDL_DestroyRenderer(ui->renderer);
    }
    if (ui->window) {
        SDL_DestroyWindow(ui->window);
    }

    printf("UI cleaned up\n");
}

/**
 * Handle SDL events and update UI state
 */
bool ui_handle_event(UIState* ui, Palette* palette, SDL_Event* event) {
    if (!ui || !palette)
        return false;

    switch (event->type) {
        case SDL_EVENT_QUIT:
            // Check for unsaved changes before quitting
            return !ui_check_unsaved_changes(ui, palette);

        case SDL_EVENT_KEY_DOWN:
            switch (event->key.key) {
                case SDLK_ESCAPE:
                    if (ui->color_picker_open) {
                        ui_close_color_picker(ui);
                    } else if (ui->show_save_dialog) {
                        ui->show_save_dialog = false;
                    } else {
                        // Quit application with unsaved changes check
                        return !ui_check_unsaved_changes(ui, palette);
                    }
                    break;

                case SDLK_S:
                    ui_show_save_dialog(ui);
                    break;

                case SDLK_R:
                    ui_reset_palette(ui, palette);
                    break;

                default:
                    break;
            }
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if (event->button.button == SDL_BUTTON_LEFT) {
                ui->mouse_down = true;
                ui->mouse_x = (int)event->button.x;
                ui->mouse_y = (int)event->button.y;

                // Check if clicking on RGBA buttons first
                if (ui_handle_rgba_button_click(ui, palette, ui->mouse_x, ui->mouse_y)) {
                    // RGBA button was clicked, handled by function
                    break;
                }

                // Check if clicking on swatch
                int swatch = ui_get_swatch_at_position(ui->mouse_x, ui->mouse_y);
                if (swatch >= 0) {
                    // Check for double-click
                    Uint32 current_time = SDL_GetTicks();
                    if (swatch == ui->last_click_swatch &&
                        current_time - ui->last_click_time < DOUBLE_CLICK_TIME) {
                        // Double-click: open color picker
                        ui->selected_swatch = swatch;
                        ui_open_color_picker(ui, palette);
                        printf("Double-click detected on swatch %d\n", swatch);
                    } else {
                        // Single click: select swatch
                        ui->selected_swatch = swatch;
                        printf("Selected swatch %d\n", swatch);
                    }

                    ui->last_click_swatch = swatch;
                    ui->last_click_time = current_time;
                }
            }
            break;

        case SDL_EVENT_MOUSE_BUTTON_UP:
            if (event->button.button == SDL_BUTTON_LEFT) {
                ui->mouse_down = false;
            }
            break;

        case SDL_EVENT_MOUSE_MOTION:
            ui->mouse_x = (int)event->motion.x;
            ui->mouse_y = (int)event->motion.y;
            break;
    }

    return true;  // Continue main loop
}

/**
 * Render the complete UI interface
 */
void ui_render(UIState* ui, const Palette* palette) {
    if (!ui || !palette)
        return;

    // Clear background to dark gray
    SDL_SetRenderDrawColor(ui->renderer, 64, 64, 64, 255);
    SDL_RenderClear(ui->renderer);

    // Render swatch grid (4x4)
    for (int row = 0; row < GRID_ROWS; row++) {
        for (int col = 0; col < GRID_COLS; col++) {
            int index = row * GRID_COLS + col;
            int x = GRID_START_X + col * (SWATCH_SIZE + SWATCH_BORDER);
            int y = GRID_START_Y + row * (SWATCH_SIZE + SWATCH_BORDER);

            PaletteColor color = palette_get_color(palette, index);

            // Draw swatch
            SDL_Color swatch_color = {color.r, color.g, color.b, color.a};
            ui_render_rect(ui, x, y, SWATCH_SIZE, SWATCH_SIZE, swatch_color);

            // Draw selection border
            if (index == ui->selected_swatch) {
                SDL_Color white = {255, 255, 255, 255};
                ui_render_rect_outline(ui, x - 2, y - 2, SWATCH_SIZE + 4, SWATCH_SIZE + 4, white);
                ui_render_rect_outline(ui, x - 1, y - 1, SWATCH_SIZE + 2, SWATCH_SIZE + 2, white);
            }

            // Draw index number
            char index_str[3];
            snprintf(index_str, sizeof(index_str), "%d", index);
            SDL_Color text_color = {255, 255, 255, 255};
            ui_render_text(ui, index_str, x + 2, y + 2, text_color);
        }
    }

    // Render UI panel
    SDL_Color panel_bg = {32, 32, 32, 255};
    ui_render_rect(ui, UI_PANEL_X, UI_PANEL_Y, UI_PANEL_W, UI_PANEL_H, panel_bg);

    // Render RGBA button controls
    ui_render_rgba_controls(ui, palette);

    // Render action buttons
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color button_bg = {64, 64, 64, 255};
    int button_y = UI_PANEL_Y + 250;

    // Save button
    ui_render_rect(ui, UI_PANEL_X + 10, button_y, 100, BUTTON_HEIGHT, button_bg);
    ui_render_text(ui, "Save (S)", UI_PANEL_X + 15, button_y + 5, white);

    // Reset button
    ui_render_rect(ui, UI_PANEL_X + 120, button_y, 100, BUTTON_HEIGHT, button_bg);
    ui_render_text(ui, "Reset (R)", UI_PANEL_X + 125, button_y + 5, white);

    // Show modification indicator
    if (palette_is_modified(palette)) {
        SDL_Color red = {255, 0, 0, 255};
        ui_render_text(ui, "* Modified", UI_PANEL_X + 10, button_y + 35, red);
    }

    // Render color picker dialog
    if (ui->color_picker_open) {
        // Color picker background
        SDL_Color picker_bg = {40, 40, 40, 240};
        ui_render_rect(ui, 40, 60, 240, 120, picker_bg);

        // Title
        ui_render_text(ui, "Color Picker", 50, 70, white);
        char title_info[32];
        snprintf(title_info, sizeof(title_info), "Editing Swatch %d", ui->selected_swatch);
        ui_render_text(ui, title_info, 50, 85, white);

        // Show current color preview
        PaletteColor current = palette_get_color(palette, ui->selected_swatch);
        SDL_Color preview_color = {current.r, current.g, current.b, current.a};
        ui_render_rect(ui, 200, 70, 40, 40, preview_color);

        // Instructions
        ui_render_text(ui, "Click RGBA fields to edit", 50, 120, white);
        ui_render_text(ui, "Press ESC to close", 50, 135, white);
        ui_render_text(ui, "Tab to move between fields", 50, 150, white);
    }

    // Render save dialog (simplified - no text input)
    if (ui->show_save_dialog) {
        // Dialog background
        SDL_Color dialog_bg = {0, 0, 0, 200};
        ui_render_rect(ui, 50, 80, 220, 80, dialog_bg);

        ui_render_text(ui, "Save Palette", 60, 90, white);
        ui_render_text(ui, "Default: palette.dat", 60, 110, white);
        ui_render_text(ui, "Press Enter to save, Esc to cancel", 60, 130, white);
    }

    // Present the rendered frame
    SDL_RenderPresent(ui->renderer);
}

/**
 * Get swatch index from screen coordinates
 */
int ui_get_swatch_at_position(int x, int y) {
    // Check if within grid bounds
    if (x < GRID_START_X || y < GRID_START_Y)
        return -1;

    int rel_x = x - GRID_START_X;
    int rel_y = y - GRID_START_Y;

    int col = rel_x / (SWATCH_SIZE + SWATCH_BORDER);
    int row = rel_y / (SWATCH_SIZE + SWATCH_BORDER);

    if (col >= GRID_COLS || row >= GRID_ROWS)
        return -1;

    // Check if within actual swatch (not border)
    int swatch_x = rel_x % (SWATCH_SIZE + SWATCH_BORDER);
    int swatch_y = rel_y % (SWATCH_SIZE + SWATCH_BORDER);

    if (swatch_x >= SWATCH_SIZE || swatch_y >= SWATCH_SIZE)
        return -1;

    return row * GRID_COLS + col;
}

/**
 * Open color picker dialog
 */
void ui_open_color_picker(UIState* ui, Palette* palette) {
    if (!ui || !palette)
        return;

    ui->color_picker_open = true;
    printf("Color picker opened for swatch %d\n", ui->selected_swatch);
}

/**
 * Close color picker dialog
 */
void ui_close_color_picker(UIState* ui) {
    if (!ui)
        return;

    ui->color_picker_open = false;
    printf("Color picker closed\n");
}

/**
 * Reset palette to default colors
 */
void ui_reset_palette(UIState* ui, Palette* palette) {
    if (!ui || !palette)
        return;

    // Reset to default 16-color palette
    palette_reset_to_default(palette);
    printf("Palette reset to default colors\n");
}

/**
 * Show save file dialog
 */
void ui_show_save_dialog(UIState* ui) {
    if (!ui)
        return;

    ui->show_save_dialog = true;
    printf("Save dialog opened\n");
}

/**
 * Check for unsaved changes and prompt user
 */
bool ui_check_unsaved_changes(UIState* ui, const Palette* palette) {
    if (!ui || !palette)
        return true;

    if (palette_is_modified(palette)) {
        printf("Warning: You have unsaved changes. Save before quitting? (Y/N)\n");
        // In a full implementation, this would show a proper dialog
        // For now, we'll just print a warning
        return false;  // Prevent quit for now
    }

    return true;  // Safe to proceed
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
void ui_render_text(UIState* ui, const char* text, int x, int y, SDL_Color color) {
    if (!ui || !text) {
        return;
    }

    SDL_SetRenderDrawColor(ui->renderer, color.r, color.g, color.b, color.a);

    int len = (int)strlen(text);
    for (int i = 0; i < len && i < 32; i++) {
        int char_index = get_char_index(text[i]);
        const uint8_t* pattern = font_patterns[char_index < 52 ? char_index : 0];

        // Draw character using bitmap pattern
        for (int row = 0; row < 7; row++) {
            for (int col = 0; col < 5; col++) {
                if (pattern[row] & (1 << (4 - col))) {
                    SDL_FRect pixel = {(float)(x + i * 6 + col), (float)(y + row), 1.0f, 1.0f};
                    SDL_RenderFillRect(ui->renderer, &pixel);
                }
            }
        }
    }
}

/**
 * Render filled rectangle
 */
void ui_render_rect(UIState* ui, int x, int y, int w, int h, SDL_Color color) {
    if (!ui) {
        return;
    }

    SDL_SetRenderDrawColor(ui->renderer, color.r, color.g, color.b, color.a);
    SDL_FRect rect = {(float)x, (float)y, (float)w, (float)h};
    SDL_RenderFillRect(ui->renderer, &rect);
}

/**
 * Render rectangle outline
 */
void ui_render_rect_outline(UIState* ui, int x, int y, int w, int h, SDL_Color color) {
    if (!ui) {
        return;
    }

    SDL_SetRenderDrawColor(ui->renderer, color.r, color.g, color.b, color.a);
    SDL_FRect rect = {(float)x, (float)y, (float)w, (float)h};
    SDL_RenderRect(ui->renderer, &rect);
}

/**
 * Handle clicks on RGBA button controls
 * Returns true if a button was clicked and handled
 */
bool ui_handle_rgba_button_click(UIState* ui, Palette* palette, int x, int y) {
    if (!ui || !palette) {
        return false;
    }

    PaletteColor current = palette_get_color(palette, ui->selected_swatch);
    bool clicked = false;

    int control_y = UI_PANEL_Y + 20;
    int control_x = UI_PANEL_X + 10;

// Helper macro to check if click is within button bounds
#define IS_BUTTON_CLICKED(mx, my, bx, by, bw, bh) \
    ((mx) >= (bx) && (mx) <= (bx) + (bw) && (my) >= (by) && (my) <= (by) + (bh))

    // Red controls: "Red: (-10) (-1) [value] (+1) (+10) (0)"
    if (y >= control_y && y <= control_y + BUTTON_HEIGHT) {
        if (IS_BUTTON_CLICKED(x, y, control_x + 50, control_y, BUTTON_WIDTH, BUTTON_HEIGHT)) {
            // -10 button
            int new_r = current.r >= 10 ? current.r - 10 : 0;
            palette_set_color(palette, ui->selected_swatch,
                              palette_make_color(new_r, current.g, current.b, current.a));
            clicked = true;
        } else if (IS_BUTTON_CLICKED(x, y, control_x + 90, control_y, BUTTON_WIDTH,
                                     BUTTON_HEIGHT)) {
            // -1 button
            int new_r = current.r > 0 ? current.r - 1 : 0;
            palette_set_color(palette, ui->selected_swatch,
                              palette_make_color(new_r, current.g, current.b, current.a));
            clicked = true;
        } else if (IS_BUTTON_CLICKED(x, y, control_x + 180, control_y, BUTTON_WIDTH,
                                     BUTTON_HEIGHT)) {
            // +1 button
            int new_r = current.r < 255 ? current.r + 1 : 255;
            palette_set_color(palette, ui->selected_swatch,
                              palette_make_color(new_r, current.g, current.b, current.a));
            clicked = true;
        } else if (IS_BUTTON_CLICKED(x, y, control_x + 220, control_y, BUTTON_WIDTH,
                                     BUTTON_HEIGHT)) {
            // +10 button
            int new_r = current.r <= 245 ? current.r + 10 : 255;
            palette_set_color(palette, ui->selected_swatch,
                              palette_make_color(new_r, current.g, current.b, current.a));
            clicked = true;
        } else if (IS_BUTTON_CLICKED(x, y, control_x + 260, control_y, BUTTON_WIDTH,
                                     BUTTON_HEIGHT)) {
            // Reset to 0 button
            palette_set_color(palette, ui->selected_swatch,
                              palette_make_color(0, current.g, current.b, current.a));
            clicked = true;
        }
    }

    // Green controls
    control_y += 35;
    if (y >= control_y && y <= control_y + BUTTON_HEIGHT) {
        if (IS_BUTTON_CLICKED(x, y, control_x + 50, control_y, BUTTON_WIDTH, BUTTON_HEIGHT)) {
            int new_g = current.g >= 10 ? current.g - 10 : 0;
            palette_set_color(palette, ui->selected_swatch,
                              palette_make_color(current.r, new_g, current.b, current.a));
            clicked = true;
        } else if (IS_BUTTON_CLICKED(x, y, control_x + 90, control_y, BUTTON_WIDTH,
                                     BUTTON_HEIGHT)) {
            int new_g = current.g > 0 ? current.g - 1 : 0;
            palette_set_color(palette, ui->selected_swatch,
                              palette_make_color(current.r, new_g, current.b, current.a));
            clicked = true;
        } else if (IS_BUTTON_CLICKED(x, y, control_x + 180, control_y, BUTTON_WIDTH,
                                     BUTTON_HEIGHT)) {
            int new_g = current.g < 255 ? current.g + 1 : 255;
            palette_set_color(palette, ui->selected_swatch,
                              palette_make_color(current.r, new_g, current.b, current.a));
            clicked = true;
        } else if (IS_BUTTON_CLICKED(x, y, control_x + 220, control_y, BUTTON_WIDTH,
                                     BUTTON_HEIGHT)) {
            int new_g = current.g <= 245 ? current.g + 10 : 255;
            palette_set_color(palette, ui->selected_swatch,
                              palette_make_color(current.r, new_g, current.b, current.a));
            clicked = true;
        } else if (IS_BUTTON_CLICKED(x, y, control_x + 260, control_y, BUTTON_WIDTH,
                                     BUTTON_HEIGHT)) {
            palette_set_color(palette, ui->selected_swatch,
                              palette_make_color(current.r, 0, current.b, current.a));
            clicked = true;
        }
    }

    // Blue controls
    control_y += 35;
    if (y >= control_y && y <= control_y + BUTTON_HEIGHT) {
        if (IS_BUTTON_CLICKED(x, y, control_x + 50, control_y, BUTTON_WIDTH, BUTTON_HEIGHT)) {
            int new_b = current.b >= 10 ? current.b - 10 : 0;
            palette_set_color(palette, ui->selected_swatch,
                              palette_make_color(current.r, current.g, new_b, current.a));
            clicked = true;
        } else if (IS_BUTTON_CLICKED(x, y, control_x + 90, control_y, BUTTON_WIDTH,
                                     BUTTON_HEIGHT)) {
            int new_b = current.b > 0 ? current.b - 1 : 0;
            palette_set_color(palette, ui->selected_swatch,
                              palette_make_color(current.r, current.g, new_b, current.a));
            clicked = true;
        } else if (IS_BUTTON_CLICKED(x, y, control_x + 180, control_y, BUTTON_WIDTH,
                                     BUTTON_HEIGHT)) {
            int new_b = current.b < 255 ? current.b + 1 : 255;
            palette_set_color(palette, ui->selected_swatch,
                              palette_make_color(current.r, current.g, new_b, current.a));
            clicked = true;
        } else if (IS_BUTTON_CLICKED(x, y, control_x + 220, control_y, BUTTON_WIDTH,
                                     BUTTON_HEIGHT)) {
            int new_b = current.b <= 245 ? current.b + 10 : 255;
            palette_set_color(palette, ui->selected_swatch,
                              palette_make_color(current.r, current.g, new_b, current.a));
            clicked = true;
        } else if (IS_BUTTON_CLICKED(x, y, control_x + 260, control_y, BUTTON_WIDTH,
                                     BUTTON_HEIGHT)) {
            palette_set_color(palette, ui->selected_swatch,
                              palette_make_color(current.r, current.g, 0, current.a));
            clicked = true;
        }
    }

    // Alpha controls
    control_y += 35;
    if (y >= control_y && y <= control_y + BUTTON_HEIGHT) {
        if (IS_BUTTON_CLICKED(x, y, control_x + 50, control_y, BUTTON_WIDTH, BUTTON_HEIGHT)) {
            int new_a = current.a >= 10 ? current.a - 10 : 0;
            palette_set_color(palette, ui->selected_swatch,
                              palette_make_color(current.r, current.g, current.b, new_a));
            clicked = true;
        } else if (IS_BUTTON_CLICKED(x, y, control_x + 90, control_y, BUTTON_WIDTH,
                                     BUTTON_HEIGHT)) {
            int new_a = current.a > 0 ? current.a - 1 : 0;
            palette_set_color(palette, ui->selected_swatch,
                              palette_make_color(current.r, current.g, current.b, new_a));
            clicked = true;
        } else if (IS_BUTTON_CLICKED(x, y, control_x + 180, control_y, BUTTON_WIDTH,
                                     BUTTON_HEIGHT)) {
            int new_a = current.a < 255 ? current.a + 1 : 255;
            palette_set_color(palette, ui->selected_swatch,
                              palette_make_color(current.r, current.g, current.b, new_a));
            clicked = true;
        } else if (IS_BUTTON_CLICKED(x, y, control_x + 220, control_y, BUTTON_WIDTH,
                                     BUTTON_HEIGHT)) {
            int new_a = current.a <= 245 ? current.a + 10 : 255;
            palette_set_color(palette, ui->selected_swatch,
                              palette_make_color(current.r, current.g, current.b, new_a));
            clicked = true;
        } else if (IS_BUTTON_CLICKED(x, y, control_x + 260, control_y, BUTTON_WIDTH,
                                     BUTTON_HEIGHT)) {
            // Reset to 255 button (full alpha)
            palette_set_color(palette, ui->selected_swatch,
                              palette_make_color(current.r, current.g, current.b, 255));
            clicked = true;
        }
    }

#undef IS_BUTTON_CLICKED
    return clicked;
}

/**
 * Render RGBA control buttons in the format: "Red: (-10) (-1) [value] (+1) (+10) (0)"
 */
void ui_render_rgba_controls(UIState* ui, const Palette* palette) {
    if (!ui || !palette)
        return;

    PaletteColor current = palette_get_color(palette, ui->selected_swatch);
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color button_bg = {64, 64, 64, 255};
    SDL_Color value_bg = {48, 48, 48, 255};

    int control_y = UI_PANEL_Y + 20;
    int control_x = UI_PANEL_X + 10;

    // Red controls
    ui_render_text(ui, "Red:", control_x, control_y + 5, white);
    ui_render_rect(ui, control_x + 50, control_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_bg);
    ui_render_text(ui, "-10", control_x + 55, control_y + 5, white);
    ui_render_rect(ui, control_x + 90, control_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_bg);
    ui_render_text(ui, "-1", control_x + 98, control_y + 5, white);
    ui_render_rect(ui, control_x + 130, control_y, VALUE_DISPLAY_WIDTH, BUTTON_HEIGHT, value_bg);
    char r_str[4];
    snprintf(r_str, sizeof(r_str), "%d", current.r);
    ui_render_text(ui, r_str, control_x + 140, control_y + 5, white);
    ui_render_rect(ui, control_x + 185, control_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_bg);
    ui_render_text(ui, "+1", control_x + 193, control_y + 5, white);
    ui_render_rect(ui, control_x + 225, control_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_bg);
    ui_render_text(ui, "+10", control_x + 230, control_y + 5, white);
    ui_render_rect(ui, control_x + 265, control_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_bg);
    ui_render_text(ui, "0", control_x + 275, control_y + 5, white);

    // Green controls
    control_y += UI_PANEL_ROW_H;
    ui_render_text(ui, "Green:", control_x, control_y + 5, white);
    ui_render_rect(ui, control_x + 50, control_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_bg);
    ui_render_text(ui, "-10", control_x + 55, control_y + 5, white);
    ui_render_rect(ui, control_x + 90, control_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_bg);
    ui_render_text(ui, "-1", control_x + 98, control_y + 5, white);
    ui_render_rect(ui, control_x + 130, control_y, VALUE_DISPLAY_WIDTH, BUTTON_HEIGHT, value_bg);
    char g_str[4];
    snprintf(g_str, sizeof(g_str), "%d", current.g);
    ui_render_text(ui, g_str, control_x + 140, control_y + 5, white);
    ui_render_rect(ui, control_x + 185, control_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_bg);
    ui_render_text(ui, "+1", control_x + 193, control_y + 5, white);
    ui_render_rect(ui, control_x + 225, control_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_bg);
    ui_render_text(ui, "+10", control_x + 230, control_y + 5, white);
    ui_render_rect(ui, control_x + 265, control_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_bg);
    ui_render_text(ui, "0", control_x + 275, control_y + 5, white);

    // Blue controls
    control_y += UI_PANEL_ROW_H;
    ui_render_text(ui, "Blue:", control_x, control_y + 5, white);
    ui_render_rect(ui, control_x + 50, control_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_bg);
    ui_render_text(ui, "-10", control_x + 55, control_y + 5, white);
    ui_render_rect(ui, control_x + 90, control_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_bg);
    ui_render_text(ui, "-1", control_x + 98, control_y + 5, white);
    ui_render_rect(ui, control_x + 130, control_y, VALUE_DISPLAY_WIDTH, BUTTON_HEIGHT, value_bg);
    char b_str[4];
    snprintf(b_str, sizeof(b_str), "%d", current.b);
    ui_render_text(ui, b_str, control_x + 140, control_y + 5, white);
    ui_render_rect(ui, control_x + 185, control_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_bg);
    ui_render_text(ui, "+1", control_x + 193, control_y + 5, white);
    ui_render_rect(ui, control_x + 225, control_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_bg);
    ui_render_text(ui, "+10", control_x + 230, control_y + 5, white);
    ui_render_rect(ui, control_x + 265, control_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_bg);
    ui_render_text(ui, "0", control_x + 275, control_y + 5, white);

    // Alpha controls
    control_y += UI_PANEL_ROW_H;
    ui_render_text(ui, "Alpha:", control_x, control_y + 5, white);
    ui_render_rect(ui, control_x + 50, control_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_bg);
    ui_render_text(ui, "-10", control_x + 55, control_y + 5, white);
    ui_render_rect(ui, control_x + 90, control_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_bg);
    ui_render_text(ui, "-1", control_x + 98, control_y + 5, white);
    ui_render_rect(ui, control_x + 130, control_y, VALUE_DISPLAY_WIDTH, BUTTON_HEIGHT, value_bg);
    char a_str[4];
    snprintf(a_str, sizeof(a_str), "%d", current.a);
    ui_render_text(ui, a_str, control_x + 140, control_y + 5, white);
    ui_render_rect(ui, control_x + 185, control_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_bg);
    ui_render_text(ui, "+1", control_x + 193, control_y + 5, white);
    ui_render_rect(ui, control_x + 225, control_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_bg);
    ui_render_text(ui, "+10", control_x + 230, control_y + 5, white);
    ui_render_rect(ui, control_x + 265, control_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_bg);
    ui_render_text(ui, "255", control_x + 270, control_y + 5, white);
}
