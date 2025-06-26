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
bool ui_init(UIState* ui, const AppConfig* config) {
    if (!ui)
        return false;

    // Initialize UI state
    memset(ui, 0, sizeof(UIState));
    ui->selected_swatch = 0;
    ui->color_picker_open = false;
    ui->show_save_dialog = false;
    ui->last_click_swatch = -1;

    // Create window
    ui->window = SDL_CreateWindow(config->window_title, config->window_width, config->window_height,
                                  SDL_WINDOW_RESIZABLE);

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
    SDL_SetRenderLogicalPresentation(ui->renderer, config->window_width, config->window_height,
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
bool ui_handle_event(UIState* ui, Palette* palette, SDL_Event* event, const AppConfig* config) {
    if (!ui || !palette)
        return false;
    (void)config;

    // Convert all mouse coordinates to the renderer's logical space
    if (event->type == SDL_EVENT_MOUSE_MOTION || event->type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
        event->type == SDL_EVENT_MOUSE_BUTTON_UP) {
        SDL_ConvertEventToRenderCoordinates(ui->renderer, event);
    }

    switch (event->type) {
        case SDL_EVENT_QUIT:
            return !ui_check_unsaved_changes(ui, palette);

        case SDL_EVENT_KEY_DOWN:
            switch (event->key.key) {
                case SDLK_ESCAPE:
                    if (ui->color_picker_open) {
                        ui_close_color_picker(ui);
                    } else if (ui->show_save_dialog) {
                        ui->show_save_dialog = false;
                    } else {
                        return !ui_check_unsaved_changes(ui, palette);
                    }
                    break;
                case SDLK_S:
                    if (event->key.mod & SDL_KMOD_CTRL) {
                        // Quick save
                        if (palette_save(palette, "palette.dat")) {
                            printf("Palette quick-saved to palette.dat\n");
                        }
                    } else {
                        ui_show_save_dialog(ui);
                    }
                    break;
                case SDLK_L:
                    if (event->key.mod & SDL_KMOD_CTRL) {
                        // Quick load
                        if (palette_load(palette, "palette.dat")) {
                            printf("Palette quick-loaded from palette.dat\n");
                        }
                    }
                    break;
                case SDLK_R:
                    ui_reset_palette(ui, palette);
                    break;
                case SDLK_RETURN:
                case SDLK_KP_ENTER:
                    if (ui->show_save_dialog) {
                        if (palette_save(palette, "palette.dat")) {
                            printf("Palette saved to palette.dat\n");
                        }
                        ui->show_save_dialog = false;
                    }
                    break;
            }
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if (event->button.button == SDL_BUTTON_LEFT) {
                ui->mouse_down = true;
                ui->mouse_x = event->button.x;
                ui->mouse_y = event->button.y;

                if (ui_handle_rgba_button_click(ui, palette, ui->mouse_x, ui->mouse_y, config)) {
                    break;
                }

                int swatch = ui_get_swatch_at_position(ui->mouse_x, ui->mouse_y, config);
                if (swatch >= 0) {
                    Uint32 current_time = SDL_GetTicks();
                    if (swatch == ui->last_click_swatch &&
                        current_time - ui->last_click_time < DOUBLE_CLICK_TIME) {
                        ui->selected_swatch = swatch;
                        ui_open_color_picker(ui, palette);
                        printf("Double-click detected on swatch %d\n", swatch);
                    } else {
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
            ui->mouse_x = event->motion.x;
            ui->mouse_y = event->motion.y;
            break;
    }

    return true;
}

/**
 * Render the complete UI interface
 */
void ui_render(UIState* ui, const Palette* palette, const AppConfig* config) {
    if (!ui || !palette)
        return;

    SDL_Color bg_color = {config->background_color.r, config->background_color.g,
                          config->background_color.b, config->background_color.a};
    SDL_SetRenderDrawColor(ui->renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
    SDL_RenderClear(ui->renderer);

    // Render swatch grid
    for (int row = 0; row < config->grid_rows; row++) {
        for (int col = 0; col < config->grid_cols; col++) {
            int index = row * config->grid_cols + col;
            int x = config->grid_start_x + col * (config->swatch_size + config->swatch_border);
            int y = config->grid_start_y + row * (config->swatch_size + config->swatch_border);

            PaletteColor p_color = palette_get_color(palette, index);
            SDL_Color swatch_color = {p_color.r, p_color.g, p_color.b, p_color.a};
            ui_render_rect(ui, x, y, config->swatch_size, config->swatch_size, swatch_color);

            if (index == ui->selected_swatch) {
                SDL_Color selection_color = {config->selected_color.r, config->selected_color.g,
                                             config->selected_color.b, config->selected_color.a};
                ui_render_rect_outline(ui, x - 2, y - 2, config->swatch_size + 4,
                                       config->swatch_size + 4, selection_color);
            }

            char index_str[3];
            snprintf(index_str, sizeof(index_str), "%d", index);
            SDL_Color text_color = {config->text_color.r, config->text_color.g,
                                    config->text_color.b, config->text_color.a};
            ui_render_text(ui, index_str, x + 2, y + 2, text_color);
        }
    }

    // Render UI panel
    SDL_Color panel_bg = {config->button_color.r, config->button_color.g, config->button_color.b,
                          128};
    ui_render_rect(ui, config->ui_panel_x, config->ui_panel_y, config->ui_panel_width,
                   config->ui_panel_height, panel_bg);

    ui_render_rgba_controls(ui, palette, config);

    // Render action buttons
    SDL_Color text_color = {config->text_color.r, config->text_color.g, config->text_color.b,
                            config->text_color.a};
    SDL_Color button_bg = {config->button_color.r, config->button_color.g, config->button_color.b,
                           config->button_color.a};

    int save_button_x = config->ui_panel_x + 10;
    int action_button_y =
        config->ui_panel_y + config->ui_panel_height - config->action_button_height - 10;

    ui_render_rect(ui, save_button_x, action_button_y, config->action_button_width,
                   config->action_button_height, button_bg);
    ui_render_text(ui, "Save (S)", save_button_x + 5, action_button_y + 5, text_color);

    int reset_button_x = save_button_x + config->action_button_width + 10;
    ui_render_rect(ui, reset_button_x, action_button_y, config->action_button_width,
                   config->action_button_height, button_bg);
    ui_render_text(ui, "Reset (R)", reset_button_x + 5, action_button_y + 5, text_color);

    if (palette_is_modified(palette)) {
        SDL_Color red = {255, 0, 0, 255};
        ui_render_text(ui, "* Modified", config->ui_panel_x + 10,
                       action_button_y + config->action_button_height + 5, red);
    }

    // Render color picker dialog
    if (ui->color_picker_open) {
        SDL_Color picker_bg = {40, 40, 40, 240};
        int picker_w = 240;
        int picker_h = 120;
        int picker_x = (config->window_width - picker_w) / 2;
        int picker_y = (config->window_height - picker_h) / 2;
        ui_render_rect(ui, picker_x, picker_y, picker_w, picker_h, picker_bg);

        ui_render_text(ui, "Color Picker", picker_x + 10, picker_y + 10, text_color);
        char title_info[32];
        snprintf(title_info, sizeof(title_info), "Editing Swatch %d", ui->selected_swatch);
        ui_render_text(ui, title_info, picker_x + 10, picker_y + 25, text_color);

        PaletteColor current = palette_get_color(palette, ui->selected_swatch);
        SDL_Color preview_color = {current.r, current.g, current.b, current.a};
        ui_render_rect(ui, picker_x + picker_w - 50, picker_y + 10, 40, 40, preview_color);

        ui_render_text(ui, "Click RGBA fields to edit", picker_x + 10, picker_y + 60, text_color);
        ui_render_text(ui, "Press ESC to close", picker_x + 10, picker_y + 75, text_color);
        ui_render_text(ui, "Tab to move between fields", picker_x + 10, picker_y + 90, text_color);
    }

    // Render save dialog
    if (ui->show_save_dialog) {
        SDL_Color dialog_bg = {0, 0, 0, 200};
        int dialog_w = 220;
        int dialog_h = 80;
        int dialog_x = (config->window_width - dialog_w) / 2;
        int dialog_y = (config->window_height - dialog_h) / 2;
        ui_render_rect(ui, dialog_x, dialog_y, dialog_w, dialog_h, dialog_bg);

        ui_render_text(ui, "Save Palette", dialog_x + 10, dialog_y + 10, text_color);
        ui_render_text(ui, "Default: palette.dat", dialog_x + 10, dialog_y + 30, text_color);
        ui_render_text(ui, "Press Enter to save, Esc to cancel", dialog_x + 10, dialog_y + 50,
                       text_color);
    }

    SDL_RenderPresent(ui->renderer);
}

/**
 * Get swatch index from screen coordinates
 */
int ui_get_swatch_at_position(float x, float y, const AppConfig* config) {
    if (x < config->grid_start_x || y < config->grid_start_y)
        return -1;

    float rel_x = x - config->grid_start_x;
    float rel_y = y - config->grid_start_y;

    int swatch_spacing = config->swatch_size + config->swatch_border;
    int col = (int)(rel_x / swatch_spacing);
    int row = (int)(rel_y / swatch_spacing);

    if (col >= config->grid_cols || row >= config->grid_rows)
        return -1;

    // Check if click is inside the swatch area (not the border)
    float swatch_x = rel_x - col * swatch_spacing;
    float swatch_y = rel_y - row * swatch_spacing;

    if (swatch_x >= config->swatch_size || swatch_y >= config->swatch_size)
        return -1;

    return row * config->grid_cols + col;
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
bool ui_handle_rgba_button_click(UIState* ui, Palette* palette, float x, float y,
                                 const AppConfig* config) {
    if (!ui || !palette)
        return false;

    PaletteColor current = palette_get_color(palette, ui->selected_swatch);
    bool clicked = false;

#define IS_BUTTON_CLICKED(mx, my, bx, by, bw, bh) \
    ((mx) >= (bx) && (mx) <= (bx) + (bw) && (my) >= (by) && (my) <= (by) + (bh))

    for (int i = 0; i < 4; ++i) {  // 0:R, 1:G, 2:B, 3:A
        float control_y = config->ui_panel_y + 20 + i * config->ui_panel_row_height;
        if (y >= control_y && y <= control_y + config->button_height) {
            uint8_t val = (i == 0)   ? current.r
                          : (i == 1) ? current.g
                          : (i == 2) ? current.b
                                     : current.a;
            uint8_t new_val = val;

            int start_x = config->ui_panel_x + 10;
            int val_x = start_x + 50;
            int btn1_x = val_x + config->value_display_width + 5;
            int btn2_x = btn1_x + config->button_width + 5;
            int btn3_x = btn2_x + config->button_width + 5;
            int btn4_x = btn3_x + config->button_width + 5;

            if (IS_BUTTON_CLICKED(x, y, btn1_x, control_y, config->button_width,
                                  config->button_height)) {
                new_val = (val >= 10) ? val - 10 : 0;
                clicked = true;
            } else if (IS_BUTTON_CLICKED(x, y, btn2_x, control_y, config->button_width,
                                         config->button_height)) {
                new_val = (val > 0) ? val - 1 : 0;
                clicked = true;
            } else if (IS_BUTTON_CLICKED(x, y, btn3_x, control_y, config->button_width,
                                         config->button_height)) {
                new_val = (val < 255) ? val + 1 : 255;
                clicked = true;
            } else if (IS_BUTTON_CLICKED(x, y, btn4_x, control_y, config->button_width,
                                         config->button_height)) {
                new_val = (val <= 245) ? val + 10 : 255;
                clicked = true;
            }

            if (clicked) {
                uint8_t r = current.r, g = current.g, b = current.b, a = current.a;
                if (i == 0)
                    r = new_val;
                else if (i == 1)
                    g = new_val;
                else if (i == 2)
                    b = new_val;
                else
                    a = new_val;
                palette_set_color(palette, ui->selected_swatch, palette_make_color(r, g, b, a));
                break;  // Exit loop once a button is handled
            }
        }
    }

#undef IS_BUTTON_CLICKED
    return clicked;
}

/**
 * Render RGBA control buttons in the format: "Red: (-10) (-1) [value] (+1) (+10) (0)"
 */
void ui_render_rgba_controls(UIState* ui, const Palette* palette, const AppConfig* config) {
    if (!ui || !palette)
        return;

    PaletteColor current = palette_get_color(palette, ui->selected_swatch);
    SDL_Color text_color = {config->text_color.r, config->text_color.g, config->text_color.b,
                            config->text_color.a};
    SDL_Color button_bg = {config->button_color.r, config->button_color.g, config->button_color.b,
                           config->button_color.a};
    SDL_Color value_bg = {config->background_color.r, config->background_color.g,
                          config->background_color.b, config->background_color.a};

    const char* labels[] = {"Red:", "Green:", "Blue:", "Alpha:"};
    uint8_t values[] = {current.r, current.g, current.b, current.a};

    for (int i = 0; i < 4; ++i) {
        float control_y = config->ui_panel_y + 20 + i * config->ui_panel_row_height;
        float control_x = config->ui_panel_x + 10;

        // Label
        ui_render_text(ui, labels[i], control_x, control_y + 5, text_color);

        int val_x = control_x + 50;
        int btn1_x = val_x + config->value_display_width + 5;
        int btn2_x = btn1_x + config->button_width + 5;
        int btn3_x = btn2_x + config->button_width + 5;
        int btn4_x = btn3_x + config->button_width + 5;

        // -10 Button
        ui_render_rect(ui, btn1_x, control_y, config->button_width, config->button_height,
                       button_bg);
        ui_render_text(ui, "-10", btn1_x + 5, control_y + 5, text_color);

        // -1 Button
        ui_render_rect(ui, btn2_x, control_y, config->button_width, config->button_height,
                       button_bg);
        ui_render_text(ui, "-1", btn2_x + 8, control_y + 5, text_color);

        // Value Display
        ui_render_rect(ui, val_x, control_y, config->value_display_width, config->button_height,
                       value_bg);
        char val_str[4];
        snprintf(val_str, sizeof(val_str), "%d", values[i]);
        ui_render_text(ui, val_str, val_x + 10, control_y + 5, text_color);

        // +1 Button
        ui_render_rect(ui, btn3_x, control_y, config->button_width, config->button_height,
                       button_bg);
        ui_render_text(ui, "+1", btn3_x + 8, control_y + 5, text_color);

        // +10 Button
        ui_render_rect(ui, btn4_x, control_y, config->button_width, config->button_height,
                       button_bg);
        ui_render_text(ui, "+10", btn4_x + 5, control_y + 5, text_color);
    }
}
