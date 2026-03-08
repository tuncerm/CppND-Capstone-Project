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
#define PALETTE_SWATCH_COLUMNS 16
#define PALETTE_SWATCH_ROWS 1
#define PALETTE_PANEL_PADDING 8
#define PALETTE_SWATCH_GAP 6

static void render_button(UIState* ui, const UIButton* button);
static void render_text(UIState* ui, const char* text, int x, int y, SDL_Color color);
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

    // Initialize compact palette panel (single-row swatches).
    const int palette_panel_w = PALETTE_PANEL_PADDING * 2 +
                                PALETTE_SWATCH_COLUMNS * PALETTE_SWATCH_SIZE +
                                (PALETTE_SWATCH_COLUMNS - 1) * PALETTE_SWATCH_GAP;
    const int palette_panel_h =
        PALETTE_PANEL_PADDING * 2 + PALETTE_SWATCH_ROWS * PALETTE_SWATCH_SIZE +
        (PALETTE_SWATCH_ROWS - 1) * PALETTE_SWATCH_GAP;
    ui->palette_bar_rect.x = 10.0f;
    ui->palette_bar_rect.y = (float)(WINDOW_HEIGHT - palette_panel_h - 10);
    ui->palette_bar_rect.w = (float)palette_panel_w;
    ui->palette_bar_rect.h = (float)palette_panel_h;

    // Initialize palette swatches in a single row.
    for (int i = 0; i < 16; i++) {
        const int row = i / PALETTE_SWATCH_COLUMNS;
        const int col = i % PALETTE_SWATCH_COLUMNS;
        SDL_FRect swatch_bounds = {
            (float)(ui->palette_bar_rect.x + PALETTE_PANEL_PADDING +
                    col * (PALETTE_SWATCH_SIZE + PALETTE_SWATCH_GAP)),
            (float)(ui->palette_bar_rect.y + PALETTE_PANEL_PADDING +
                    row * (PALETTE_SWATCH_SIZE + PALETTE_SWATCH_GAP)),
            (float)PALETTE_SWATCH_SIZE,
            (float)PALETTE_SWATCH_SIZE};
        ui_input_init_clickable(&ui->palette_swatches[i], PALETTE_ID_BASE + i, swatch_bounds,
                                ui_on_palette_click, ui);
    }

    ui->selected_palette_index = 1;  // Start with palette index 1 (not black)
    ui_sync_palette_selection(ui);

    // Initialize action buttons
    int button_y = 10;
    int button_spacing = BUTTON_WIDTH + 10;

    ui_input_init_clickable(&ui->save_button.input, ACTION_ID_SAVE,
                            (SDL_FRect){10.0f, (float)button_y, (float)BUTTON_WIDTH,
                                        (float)BUTTON_HEIGHT},
                            ui_on_action_click, ui);
    strcpy(ui->save_button.text, "Save (S)");

    ui_input_init_clickable(&ui->load_button.input, ACTION_ID_LOAD,
                            (SDL_FRect){(float)(10 + button_spacing), (float)button_y,
                                        (float)BUTTON_WIDTH, (float)BUTTON_HEIGHT},
                            ui_on_action_click, ui);
    strcpy(ui->load_button.text, "Load (L)");

    ui_input_init_clickable(&ui->new_button.input, ACTION_ID_NEW,
                            (SDL_FRect){(float)(10 + button_spacing * 2), (float)button_y,
                                        (float)BUTTON_WIDTH, (float)BUTTON_HEIGHT},
                            ui_on_action_click, ui);
    strcpy(ui->new_button.text, "New");

    ui_input_init_clickable(&ui->quit_button.input, ACTION_ID_QUIT,
                            (SDL_FRect){(float)(10 + button_spacing * 3), (float)button_y,
                                        (float)BUTTON_WIDTH, (float)BUTTON_HEIGHT},
                            ui_on_action_click, ui);
    strcpy(ui->quit_button.text, "Quit (ESC)");

    // Initialize status
    strcpy(ui->status_text, "Tile Maker Ready");
    ui->dirty_indicator = false;
    if (!text_renderer_init(&ui->text_renderer, renderer)) {
        return false;
    }

    // Initialize double-click tracking
    ui->last_click_time = 0;
    ui->last_clicked_tile = -1;

    // Initialize quit confirmation dialog
    ui->show_quit_dialog = false;
    ui_input_init_clickable(&ui->quit_yes_button.input, DIALOG_ID_YES,
                            (SDL_FRect){(float)(WINDOW_WIDTH / 2 - 110),
                                        (float)(WINDOW_HEIGHT / 2), 100.0f, 40.0f},
                            ui_on_dialog_click, ui);
    strcpy(ui->quit_yes_button.text, "Yes");

    ui_input_init_clickable(&ui->quit_no_button.input, DIALOG_ID_NO,
                            (SDL_FRect){(float)(WINDOW_WIDTH / 2 + 10), (float)(WINDOW_HEIGHT / 2),
                                        100.0f, 40.0f},
                            ui_on_dialog_click, ui);
    strcpy(ui->quit_no_button.text, "No");

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

    text_renderer_cleanup(&ui->text_renderer);

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
    render_button(ui, &ui->save_button);
    render_button(ui, &ui->load_button);
    render_button(ui, &ui->new_button);
    render_button(ui, &ui->quit_button);

    // Render status text above the palette panel.
    SDL_Color text_color = {255, 255, 255, 255};
    int status_y = (int)ui->palette_bar_rect.y - 18;
    if (status_y < BUTTON_HEIGHT + 20) {
        status_y = BUTTON_HEIGHT + 20;
    }
    render_text(ui, ui->status_text, 10, status_y, text_color);

    // Render dirty indicator
    if (ui->dirty_indicator) {
        SDL_SetRenderDrawColor(renderer, 255, 200, 0, 255);
        SDL_FRect dirty_rect = {WINDOW_WIDTH - 30.0f, 10.0f, 20.0f, 20.0f};
        SDL_RenderFillRect(renderer, &dirty_rect);

        render_text(ui, "*", WINDOW_WIDTH - 25, 15, text_color);
    }

    // Render quit confirmation dialog
    if (ui->show_quit_dialog) {
        SDL_FRect dialog_rect = {(float)(WINDOW_WIDTH / 2 - 150), (float)(WINDOW_HEIGHT / 2 - 50),
                                 300.0f, 120.0f};
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 230);
        SDL_RenderFillRect(renderer, &dialog_rect);
        SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
        SDL_RenderRect(renderer, &dialog_rect);

        render_text(ui, "Unsaved changes! Quit?", (int)dialog_rect.x + 10, (int)dialog_rect.y + 10,
                    (SDL_Color){255, 255, 255, 255});
        render_button(ui, &ui->quit_yes_button);
        render_button(ui, &ui->quit_no_button);
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
    const UIMouseState passive_mouse = {(float)mouse_x, (float)mouse_y, false, false, false};
    const UIMouseState click_mouse = {(float)mouse_x, (float)mouse_y, left_click, left_click,
                                      left_click};
    ui->pending_action = 0;

    if (ui->show_quit_dialog) {
        ui_input_update_array(ui->palette_swatches, 16, false, dt_seconds, &passive_mouse);
        ui_input_update_array(&ui->save_button.input, 1, false, dt_seconds, &passive_mouse);
        ui_input_update_array(&ui->load_button.input, 1, false, dt_seconds, &passive_mouse);
        ui_input_update_array(&ui->new_button.input, 1, false, dt_seconds, &passive_mouse);
        ui_input_update_array(&ui->quit_button.input, 1, false, dt_seconds, &passive_mouse);
        ui_input_update_array(&ui->quit_yes_button.input, 1, true, dt_seconds, &click_mouse);
        ui_input_update_array(&ui->quit_no_button.input, 1, true, dt_seconds, &click_mouse);

        return ui->pending_action;
    }

    ui_input_update_array(ui->palette_swatches, 16, true, dt_seconds, &click_mouse);
    ui_input_update_array(&ui->save_button.input, 1, true, dt_seconds, &click_mouse);
    ui_input_update_array(&ui->load_button.input, 1, true, dt_seconds, &click_mouse);
    ui_input_update_array(&ui->new_button.input, 1, true, dt_seconds, &click_mouse);
    ui_input_update_array(&ui->quit_button.input, 1, true, dt_seconds, &click_mouse);
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
static void render_button(UIState* ui, const UIButton* button) {
    if (!ui || !ui->text_renderer.renderer || !button)
        return;

    SDL_Color base = {60, 60, 60, 255};
    SDL_Color hover = {82, 82, 82, 255};
    SDL_Color border = {128, 128, 128, 255};

    ui_input_widgets_render_button(ui->text_renderer.renderer, &button->input, base, hover, border,
                                   0.85f);

    SDL_Color text_color = {255, 255, 255, 255};
    int text_x =
        (int)(button->input.bounds.x + (button->input.bounds.w - (float)ui_text_width(button->text)) * 0.5f);
    int text_y = (int)(button->input.bounds.y + (button->input.bounds.h - 7.0f) * 0.5f);
    render_text(ui, button->text, text_x, text_y, text_color);
}

static void render_text(UIState* ui, const char* text, int x, int y, SDL_Color color) {
    if (!ui || !text) {
        return;
    }

    text_render_string(&ui->text_renderer, text, x, y, color);
}
