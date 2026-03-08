#include "ui.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "ui_input_widgets.h"
#include "palette_io.h"

#define DOUBLE_CLICK_THRESHOLD_MS 500
#define PALETTE_ID_BASE 1000
#define ACTION_ID_SAVE 2001
#define ACTION_ID_LOAD 2002
#define ACTION_ID_NEW 2003
#define ACTION_ID_QUIT 2004
#define DIALOG_ID_YES 3001
#define DIALOG_ID_NO 3002

static void render_button(SDL_Renderer* renderer, const UIButton* button);
static int ui_text_width(const char* text);

static void ui_sync_palette_selection(UIState* ui) {
    if (!ui) {
        return;
    }

    for (int i = 0; i < 16; i++) {
        ui_input_set_selected(&ui->palette_swatches[i], i == ui->selected_palette_index);
    }
}

static void ui_on_palette_click(int id, void* userdata) {
    UIState* ui = (UIState*)userdata;
    if (!ui) {
        return;
    }

    const int index = id - PALETTE_ID_BASE;
    if (index < 0 || index >= 16) {
        return;
    }

    ui->selected_palette_index = index;
    ui_sync_palette_selection(ui);
    ui->pending_action = PALETTE_SELECTION_OFFSET + index;
}

static void ui_on_action_click(int id, void* userdata) {
    UIState* ui = (UIState*)userdata;
    if (!ui) {
        return;
    }

    if (id == ACTION_ID_SAVE) {
        ui->pending_action = 1;
    } else if (id == ACTION_ID_LOAD) {
        ui->pending_action = 2;
    } else if (id == ACTION_ID_NEW) {
        ui->pending_action = 3;
    } else if (id == ACTION_ID_QUIT) {
        ui->pending_action = 4;
    }
}

static void ui_on_dialog_click(int id, void* userdata) {
    UIState* ui = (UIState*)userdata;
    if (!ui) {
        return;
    }

    if (id == DIALOG_ID_YES) {
        ui->pending_action = 5;
    } else if (id == DIALOG_ID_NO) {
        ui->show_quit_dialog = false;
    }
}

/**
 * Initialize UI system
 */
bool ui_init(UIState* ui, SDL_Renderer* renderer) {
    if (!ui || !renderer) {
        return false;
    }

    memset(ui, 0, sizeof(*ui));

    // Initialize palette bar
    ui->palette_bar_rect.x = 10.0f;
    ui->palette_bar_rect.y = (float)(WINDOW_HEIGHT - PALETTE_BAR_HEIGHT - 10);
    ui->palette_bar_rect.w = (float)(WINDOW_WIDTH - 20);
    ui->palette_bar_rect.h = (float)PALETTE_BAR_HEIGHT;

    // Initialize palette swatches (16 colors in a row)
    int swatch_spacing = ((int)ui->palette_bar_rect.w - 20) / 16;
    for (int i = 0; i < 16; i++) {
        SDL_FRect swatch_bounds = {(float)(ui->palette_bar_rect.x + 10 + i * swatch_spacing),
                                   ui->palette_bar_rect.y + 10.0f, (float)PALETTE_SWATCH_SIZE,
                                   (float)PALETTE_SWATCH_SIZE};
        ui_input_init(&ui->palette_swatches[i], PALETTE_ID_BASE + i, swatch_bounds);
        ui_input_set_callbacks(&ui->palette_swatches[i], ui_on_palette_click, NULL, ui);
    }

    ui->selected_palette_index = 1;  // Start with palette index 1 (not black)
    ui_sync_palette_selection(ui);

    // Initialize action buttons
    int button_y = 10;
    int button_spacing = BUTTON_WIDTH + 10;

    ui_input_init(&ui->save_button.input, ACTION_ID_SAVE,
                  (SDL_FRect){10.0f, (float)button_y, (float)BUTTON_WIDTH, (float)BUTTON_HEIGHT});
    strcpy(ui->save_button.text, "Save (S)");
    ui_input_set_callbacks(&ui->save_button.input, ui_on_action_click, NULL, ui);

    ui_input_init(
        &ui->load_button.input, ACTION_ID_LOAD,
        (SDL_FRect){(float)(10 + button_spacing), (float)button_y, (float)BUTTON_WIDTH,
                    (float)BUTTON_HEIGHT});
    strcpy(ui->load_button.text, "Load (L)");
    ui_input_set_callbacks(&ui->load_button.input, ui_on_action_click, NULL, ui);

    ui_input_init(
        &ui->new_button.input, ACTION_ID_NEW,
        (SDL_FRect){(float)(10 + button_spacing * 2), (float)button_y, (float)BUTTON_WIDTH,
                    (float)BUTTON_HEIGHT});
    strcpy(ui->new_button.text, "New");
    ui_input_set_callbacks(&ui->new_button.input, ui_on_action_click, NULL, ui);

    ui_input_init(
        &ui->quit_button.input, ACTION_ID_QUIT,
        (SDL_FRect){(float)(10 + button_spacing * 3), (float)button_y, (float)BUTTON_WIDTH,
                    (float)BUTTON_HEIGHT});
    strcpy(ui->quit_button.text, "Quit (ESC)");
    ui_input_set_callbacks(&ui->quit_button.input, ui_on_action_click, NULL, ui);

    // Initialize status
    strcpy(ui->status_text, "Tile Maker Ready");
    ui->dirty_indicator = false;
    ui->font_texture = NULL;

    // Initialize double-click tracking
    ui->last_click_time = 0;
    ui->last_clicked_tile = -1;

    // Initialize quit confirmation dialog
    ui->show_quit_dialog = false;
    ui_input_init(&ui->quit_yes_button.input, DIALOG_ID_YES,
                  (SDL_FRect){(float)(WINDOW_WIDTH / 2 - 110), (float)(WINDOW_HEIGHT / 2), 100.0f,
                              40.0f});
    strcpy(ui->quit_yes_button.text, "Yes");
    ui_input_set_callbacks(&ui->quit_yes_button.input, ui_on_dialog_click, NULL, ui);

    ui_input_init(&ui->quit_no_button.input, DIALOG_ID_NO,
                  (SDL_FRect){(float)(WINDOW_WIDTH / 2 + 10), (float)(WINDOW_HEIGHT / 2), 100.0f,
                              40.0f});
    strcpy(ui->quit_no_button.text, "No");
    ui_input_set_callbacks(&ui->quit_no_button.input, ui_on_dialog_click, NULL, ui);

    ui->pending_action = 0;

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
    (void)renderer;
    if (!ui)
        return;
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
        UIInputElement* swatch = &ui->palette_swatches[i];
        SDL_Color color = palette_get_sdl_color(i);
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, &swatch->bounds);

        if (swatch->selected_anim_t > 0.01f) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255,
                                   (Uint8)(160 + 95 * swatch->selected_anim_t));
            SDL_RenderRect(renderer, &swatch->bounds);

            SDL_FRect outer_rect = swatch->bounds;
            int expand = 1 + (int)(swatch->selected_anim_t * 2.0f);
            outer_rect.x -= (float)expand;
            outer_rect.y -= (float)expand;
            outer_rect.w += (float)(expand * 2);
            outer_rect.h += (float)(expand * 2);
            SDL_RenderRect(renderer, &outer_rect);
        } else if (swatch->hover_anim_t > 0.01f) {
            SDL_SetRenderDrawColor(renderer, 200, 200, 200,
                                   (Uint8)(120 + 100 * swatch->hover_anim_t));
            SDL_RenderRect(renderer, &swatch->bounds);
        } else {
            SDL_SetRenderDrawColor(renderer, 96, 96, 96, 255);
            SDL_RenderRect(renderer, &swatch->bounds);
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
        SDL_FRect dirty_rect = {WINDOW_WIDTH - 30.0f, 10.0f, 20.0f, 20.0f};
        SDL_RenderFillRect(renderer, &dirty_rect);

        render_text(renderer, "*", WINDOW_WIDTH - 25, 15, text_color);
    }

    // Render quit confirmation dialog
    if (ui->show_quit_dialog) {
        SDL_FRect dialog_rect = {(float)(WINDOW_WIDTH / 2 - 150), (float)(WINDOW_HEIGHT / 2 - 50),
                                 300.0f, 120.0f};
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 230);
        SDL_RenderFillRect(renderer, &dialog_rect);
        SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
        SDL_RenderRect(renderer, &dialog_rect);

        render_text(renderer, "Unsaved changes! Quit?", (int)dialog_rect.x + 10,
                    (int)dialog_rect.y + 10, (SDL_Color){255, 255, 255, 255});
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

    const bool left_click = clicked && button == SDL_BUTTON_LEFT;
    const float dt_seconds = 1.0f / 60.0f;
    ui->pending_action = 0;

    if (ui->show_quit_dialog) {
        for (int i = 0; i < 16; i++) {
            ui_input_set_enabled(&ui->palette_swatches[i], false);
            ui_input_update(&ui->palette_swatches[i], dt_seconds, (float)mouse_x, (float)mouse_y,
                            false, false, false);
        }

        ui_input_set_enabled(&ui->save_button.input, false);
        ui_input_update(&ui->save_button.input, dt_seconds, (float)mouse_x, (float)mouse_y, false,
                        false, false);
        ui_input_set_enabled(&ui->load_button.input, false);
        ui_input_update(&ui->load_button.input, dt_seconds, (float)mouse_x, (float)mouse_y, false,
                        false, false);
        ui_input_set_enabled(&ui->new_button.input, false);
        ui_input_update(&ui->new_button.input, dt_seconds, (float)mouse_x, (float)mouse_y, false,
                        false, false);
        ui_input_set_enabled(&ui->quit_button.input, false);
        ui_input_update(&ui->quit_button.input, dt_seconds, (float)mouse_x, (float)mouse_y, false,
                        false, false);

        ui_input_set_enabled(&ui->quit_yes_button.input, true);
        ui_input_update(&ui->quit_yes_button.input, dt_seconds, (float)mouse_x, (float)mouse_y,
                        left_click, left_click, left_click);

        ui_input_set_enabled(&ui->quit_no_button.input, true);
        ui_input_update(&ui->quit_no_button.input, dt_seconds, (float)mouse_x, (float)mouse_y,
                        left_click, left_click, left_click);

        return ui->pending_action;
    }

    for (int i = 0; i < 16; i++) {
        ui_input_set_enabled(&ui->palette_swatches[i], true);
        ui_input_update(&ui->palette_swatches[i], dt_seconds, (float)mouse_x, (float)mouse_y,
                        left_click, left_click, left_click);
    }

    ui_input_set_enabled(&ui->save_button.input, true);
    ui_input_update(&ui->save_button.input, dt_seconds, (float)mouse_x, (float)mouse_y, left_click,
                    left_click, left_click);

    ui_input_set_enabled(&ui->load_button.input, true);
    ui_input_update(&ui->load_button.input, dt_seconds, (float)mouse_x, (float)mouse_y, left_click,
                    left_click, left_click);

    ui_input_set_enabled(&ui->new_button.input, true);
    ui_input_update(&ui->new_button.input, dt_seconds, (float)mouse_x, (float)mouse_y, left_click,
                    left_click, left_click);

    ui_input_set_enabled(&ui->quit_button.input, true);
    ui_input_update(&ui->quit_button.input, dt_seconds, (float)mouse_x, (float)mouse_y, left_click,
                    left_click, left_click);

    ui_input_set_enabled(&ui->quit_yes_button.input, false);
    ui_input_set_enabled(&ui->quit_no_button.input, false);

    return ui->pending_action;
}

/**
 * Set selected palette index
 */
void ui_set_palette_selection(UIState* ui, int index) {
    if (!ui)
        return;

    if (index >= 0 && index < 16) {
        ui->selected_palette_index = index;
        ui_sync_palette_selection(ui);
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
        (current_time - ui->last_click_time) < DOUBLE_CLICK_THRESHOLD_MS) {
        is_double_click = true;
    }

    ui->last_click_time = current_time;
    ui->last_clicked_tile = tile_id;

    return is_double_click;
}

static int ui_text_width(const char* text) {
    if (!text) {
        return 0;
    }

    int len = (int)strlen(text);
    if (len <= 0) {
        return 0;
    }

    return len * 6 - 1;
}

/**
 * Render a simple filled rectangle button
 */
static void render_button(SDL_Renderer* renderer, const UIButton* button) {
    if (!renderer || !button)
        return;

    SDL_Color base = {60, 60, 60, 255};
    SDL_Color hover = {82, 82, 82, 255};
    SDL_Color border = {128, 128, 128, 255};

    ui_input_widgets_render_button(renderer, &button->input, base, hover, border, 0.85f);

    SDL_Color text_color = {255, 255, 255, 255};
    int text_x =
        (int)(button->input.bounds.x + (button->input.bounds.w - (float)ui_text_width(button->text)) * 0.5f);
    int text_y = (int)(button->input.bounds.y + (button->input.bounds.h - 7.0f) * 0.5f);
    render_text(renderer, button->text, text_x, text_y, text_color);
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
        const uint8_t* pattern =
            font_patterns[(char_index >= 0 && char_index < GLYPH_COUNT) ? char_index : 0];

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
