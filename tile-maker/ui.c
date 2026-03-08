#include "ui.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "ui_input_widgets.h"
#include "palette_io.h"
#include "pixel_editor.h"

#define DOUBLE_CLICK_THRESHOLD_MS 500
#define PALETTE_ID_BASE 1000
#define ACTION_ID_SAVE 2001
#define ACTION_ID_LOAD 2002
#define ACTION_ID_NEW 2003
#define ACTION_ID_QUIT 2004
#define ACTION_ID_SPEC_HEALTH_BASE 2100
#define ACTION_ID_SPEC_DESTRUCT_BASE 2200
#define ACTION_ID_SPEC_MOVE_BASE 2300
#define DIALOG_ID_YES 3001
#define DIALOG_ID_NO 3002
#define PALETTE_SWATCH_COLUMNS 8
#define PALETTE_SWATCH_ROWS 2
#define PALETTE_PANEL_PADDING 6
#define PALETTE_SWATCH_GAP 4

static void render_button(UIState* ui, const UIButton* button);
static void render_text(UIState* ui, const char* text, int x, int y, SDL_Color color);
static int ui_text_width(const char* text);
static void ui_sync_spec_selection(UIState* ui);

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
        ui->pending_action = UI_ACTION_SAVE;
    } else if (id == ACTION_ID_LOAD) {
        ui->pending_action = UI_ACTION_LOAD;
    } else if (id == ACTION_ID_NEW) {
        ui->pending_action = UI_ACTION_NEW;
    } else if (id == ACTION_ID_QUIT) {
        ui->pending_action = UI_ACTION_QUIT;
    } else if (id >= ACTION_ID_SPEC_HEALTH_BASE && id < ACTION_ID_SPEC_HEALTH_BASE + 8) {
        ui->pending_action = UI_ACTION_SPEC_HEALTH_SET_BASE + (id - ACTION_ID_SPEC_HEALTH_BASE);
    } else if (id >= ACTION_ID_SPEC_DESTRUCT_BASE && id < ACTION_ID_SPEC_DESTRUCT_BASE + 8) {
        ui->pending_action = UI_ACTION_SPEC_DESTRUCT_SET_BASE + (id - ACTION_ID_SPEC_DESTRUCT_BASE);
    } else if (id >= ACTION_ID_SPEC_MOVE_BASE && id < ACTION_ID_SPEC_MOVE_BASE + 4) {
        ui->pending_action = UI_ACTION_SPEC_MOVE_SET_BASE + (id - ACTION_ID_SPEC_MOVE_BASE);
    }
}

static void ui_on_dialog_click(int id, void* userdata) {
    UIState* ui = (UIState*)userdata;
    if (!ui) {
        return;
    }

    if (id == DIALOG_ID_YES) {
        ui->pending_action = UI_ACTION_QUIT_CONFIRM;
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

    // Initialize palette panel above the pixel editor with matched width.
    const int palette_panel_w = PIXEL_EDITOR_WIDTH;
    const float swatch_size = (float)(palette_panel_w - PALETTE_PANEL_PADDING * 2 -
                                      (PALETTE_SWATCH_COLUMNS - 1) * PALETTE_SWATCH_GAP) /
                              (float)PALETTE_SWATCH_COLUMNS;
    const int palette_panel_h =
        (int)(PALETTE_PANEL_PADDING * 2 + PALETTE_SWATCH_ROWS * swatch_size +
              (PALETTE_SWATCH_ROWS - 1) * PALETTE_SWATCH_GAP);
    ui->palette_bar_rect.x = (float)PIXEL_EDITOR_POS_X;
    ui->palette_bar_rect.y = (float)(PIXEL_EDITOR_POS_Y - palette_panel_h - 12);
    ui->palette_bar_rect.w = (float)palette_panel_w;
    ui->palette_bar_rect.h = (float)palette_panel_h;

    // Initialize palette swatches in a single row.
    for (int i = 0; i < 16; i++) {
        const int row = i / PALETTE_SWATCH_COLUMNS;
        const int col = i % PALETTE_SWATCH_COLUMNS;
        SDL_FRect swatch_bounds = {
            (float)(ui->palette_bar_rect.x + PALETTE_PANEL_PADDING +
                    col * (swatch_size + PALETTE_SWATCH_GAP)),
            (float)(ui->palette_bar_rect.y + PALETTE_PANEL_PADDING +
                    row * (swatch_size + PALETTE_SWATCH_GAP)),
            swatch_size,
            swatch_size};
        ui_input_init_clickable(&ui->palette_swatches[i], PALETTE_ID_BASE + i, swatch_bounds,
                                ui_on_palette_click, ui);
    }

    ui->selected_palette_index = 1;  // Start with palette index 1 (not black)
    ui_sync_palette_selection(ui);

    // Initialize tile spec panel under pixel editor.
    ui->tile_spec_rect.x = (float)PIXEL_EDITOR_POS_X;
    ui->tile_spec_rect.y = (float)(PIXEL_EDITOR_POS_Y + PIXEL_EDITOR_HEIGHT + 12);
    ui->tile_spec_rect.w = (float)PIXEL_EDITOR_WIDTH;
    ui->tile_spec_rect.h = 126.0f;

    const float row_start_y = ui->tile_spec_rect.y + 28.0f;
    const float row_step_y = 26.0f;
    const float spec_button_start_x = ui->tile_spec_rect.x + 40.0f;
    const float spec_button_w = 20.0f;
    const float spec_button_h = 20.0f;
    const float spec_button_gap = 3.0f;

    for (int i = 0; i < 8; i++) {
        SDL_FRect health_bounds = {spec_button_start_x + i * (spec_button_w + spec_button_gap),
                                   row_start_y, spec_button_w, spec_button_h};
        ui_input_init_clickable(&ui->spec_health_buttons[i].input, ACTION_ID_SPEC_HEALTH_BASE + i,
                                health_bounds, ui_on_action_click, ui);
        snprintf(ui->spec_health_buttons[i].text, sizeof(ui->spec_health_buttons[i].text), "%d", i);

        SDL_FRect destruct_bounds = {spec_button_start_x + i * (spec_button_w + spec_button_gap),
                                     row_start_y + row_step_y, spec_button_w, spec_button_h};
        ui_input_init_clickable(&ui->spec_destruction_buttons[i].input,
                                ACTION_ID_SPEC_DESTRUCT_BASE + i, destruct_bounds, ui_on_action_click,
                                ui);
        snprintf(ui->spec_destruction_buttons[i].text, sizeof(ui->spec_destruction_buttons[i].text),
                 "%d", i);
    }

    for (int i = 0; i < 4; i++) {
        SDL_FRect movement_bounds = {spec_button_start_x + i * (spec_button_w + spec_button_gap),
                                     row_start_y + row_step_y * 2.0f, spec_button_w, spec_button_h};
        ui_input_init_clickable(&ui->spec_movement_buttons[i].input, ACTION_ID_SPEC_MOVE_BASE + i,
                                movement_bounds, ui_on_action_click, ui);
        snprintf(ui->spec_movement_buttons[i].text, sizeof(ui->spec_movement_buttons[i].text), "%d",
                 i);
    }

    ui->spec_tile_id = 0;
    ui->spec_health = 1;
    ui->spec_destruction_mode = 1;
    ui->spec_movement_mode = 0;
    ui_sync_spec_selection(ui);

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

    // Render tile spec panel.
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
    SDL_RenderFillRect(renderer, &ui->tile_spec_rect);
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    SDL_RenderRect(renderer, &ui->tile_spec_rect);

    render_text(ui, "Tile Spec Defaults", (int)ui->tile_spec_rect.x + 8, (int)ui->tile_spec_rect.y + 8,
                (SDL_Color){255, 255, 255, 255});

    char tile_id_text[32];
    snprintf(tile_id_text, sizeof(tile_id_text), "Tile %03d", ui->spec_tile_id);
    render_text(ui, tile_id_text, (int)ui->tile_spec_rect.x + 150, (int)ui->tile_spec_rect.y + 8,
                (SDL_Color){220, 220, 220, 255});

    ui_sync_spec_selection(ui);
    const int row0_y = (int)ui->tile_spec_rect.y + 28;
    const int row_step = 26;

    render_text(ui, "H", (int)ui->tile_spec_rect.x + 10, row0_y + 6, (SDL_Color){220, 220, 220, 255});
    for (int i = 0; i < 8; i++) {
        render_button(ui, &ui->spec_health_buttons[i]);
    }

    render_text(ui, "D", (int)ui->tile_spec_rect.x + 10, row0_y + row_step + 6,
                (SDL_Color){220, 220, 220, 255});
    for (int i = 0; i < 8; i++) {
        render_button(ui, &ui->spec_destruction_buttons[i]);
    }

    render_text(ui, "M", (int)ui->tile_spec_rect.x + 10, row0_y + row_step * 2 + 6,
                (SDL_Color){220, 220, 220, 255});
    for (int i = 0; i < 4; i++) {
        render_button(ui, &ui->spec_movement_buttons[i]);
    }

    render_text(ui, "D:0 ind 1 norm 2 heavy 3 spec", (int)ui->tile_spec_rect.x + 8,
                (int)ui->tile_spec_rect.y + 104, (SDL_Color){170, 170, 170, 255});

    // Render a fixed bottom status bar so text never overlaps the tile grid.
    SDL_Color text_color = {255, 255, 255, 255};
    SDL_FRect status_bar = {0.0f, (float)(WINDOW_HEIGHT - 24), (float)WINDOW_WIDTH, 24.0f};
    SDL_SetRenderDrawColor(renderer, 24, 24, 24, 255);
    SDL_RenderFillRect(renderer, &status_bar);
    SDL_SetRenderDrawColor(renderer, 72, 72, 72, 255);
    SDL_RenderRect(renderer, &status_bar);
    render_text(ui, ui->status_text, 10, WINDOW_HEIGHT - 18, text_color);

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
    ui->pending_action = UI_ACTION_NONE;

    if (ui->show_quit_dialog) {
        ui_input_update_array(ui->palette_swatches, 16, false, dt_seconds, &passive_mouse);
        ui_input_update_array(&ui->save_button.input, 1, false, dt_seconds, &passive_mouse);
        ui_input_update_array(&ui->load_button.input, 1, false, dt_seconds, &passive_mouse);
        ui_input_update_array(&ui->new_button.input, 1, false, dt_seconds, &passive_mouse);
        ui_input_update_array(&ui->quit_button.input, 1, false, dt_seconds, &passive_mouse);
        for (int i = 0; i < 8; i++) {
            ui_input_update_array(&ui->spec_health_buttons[i].input, 1, false, dt_seconds,
                                  &passive_mouse);
            ui_input_update_array(&ui->spec_destruction_buttons[i].input, 1, false, dt_seconds,
                                  &passive_mouse);
            if (i < 4) {
                ui_input_update_array(&ui->spec_movement_buttons[i].input, 1, false, dt_seconds,
                                      &passive_mouse);
            }
        }
        ui_input_update_array(&ui->quit_yes_button.input, 1, true, dt_seconds, &click_mouse);
        ui_input_update_array(&ui->quit_no_button.input, 1, true, dt_seconds, &click_mouse);

        return ui->pending_action;
    }

    ui_input_update_array(ui->palette_swatches, 16, true, dt_seconds, &click_mouse);
    ui_input_update_array(&ui->save_button.input, 1, true, dt_seconds, &click_mouse);
    ui_input_update_array(&ui->load_button.input, 1, true, dt_seconds, &click_mouse);
    ui_input_update_array(&ui->new_button.input, 1, true, dt_seconds, &click_mouse);
    ui_input_update_array(&ui->quit_button.input, 1, true, dt_seconds, &click_mouse);
    for (int i = 0; i < 8; i++) {
        ui_input_update_array(&ui->spec_health_buttons[i].input, 1, true, dt_seconds, &click_mouse);
        ui_input_update_array(&ui->spec_destruction_buttons[i].input, 1, true, dt_seconds,
                              &click_mouse);
        if (i < 4) {
            ui_input_update_array(&ui->spec_movement_buttons[i].input, 1, true, dt_seconds,
                                  &click_mouse);
        }
    }
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

void ui_set_tile_spec(UIState* ui, int tile_id, uint8_t health, uint8_t destruction_mode,
                      uint8_t movement_mode) {
    if (!ui) {
        return;
    }

    if (tile_id < 0) {
        tile_id = 0;
    }
    if (tile_id > 999) {
        tile_id = 999;
    }

    ui->spec_tile_id = tile_id;
    ui->spec_health = (uint8_t)(health & 0x07u);
    ui->spec_destruction_mode = (uint8_t)(destruction_mode & 0x07u);
    ui->spec_movement_mode = (uint8_t)(movement_mode & 0x03u);
    ui_sync_spec_selection(ui);
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
    const float selected_t = button->input.selected_anim_t;

    if (selected_t > 0.01f) {
        const SDL_Color selected_base = {45, 108, 156, 255};
        const SDL_Color selected_hover = {60, 126, 178, 255};
        const SDL_Color selected_border = {220, 240, 255, 255};
        base = ui_input_widgets_blend_color(base, selected_base, selected_t);
        hover = ui_input_widgets_blend_color(hover, selected_hover, selected_t);
        border = ui_input_widgets_blend_color(border, selected_border, selected_t);
    }

    ui_input_widgets_render_button(ui->text_renderer.renderer, &button->input, base, hover, border,
                                   0.85f);

    SDL_Color text_color = {255, 255, 255, 255};
    if (selected_t > 0.01f) {
        text_color = ui_input_widgets_blend_color(text_color, (SDL_Color){245, 255, 255, 255},
                                                  selected_t);

        SDL_FRect outer_rect = button->input.bounds;
        const float expand = 1.0f + selected_t;
        outer_rect.x -= expand;
        outer_rect.y -= expand;
        outer_rect.w += expand * 2.0f;
        outer_rect.h += expand * 2.0f;
        SDL_SetRenderDrawColor(ui->text_renderer.renderer, 235, 250, 255,
                               (Uint8)(140 + 95 * selected_t));
        SDL_RenderRect(ui->text_renderer.renderer, &outer_rect);
    }

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

static void ui_sync_spec_selection(UIState* ui) {
    if (!ui) {
        return;
    }

    for (int i = 0; i < 8; i++) {
        ui_input_set_selected(&ui->spec_health_buttons[i].input, i == ui->spec_health);
        ui_input_set_selected(&ui->spec_destruction_buttons[i].input,
                              i == ui->spec_destruction_mode);
    }
    for (int i = 0; i < 4; i++) {
        ui_input_set_selected(&ui->spec_movement_buttons[i].input, i == ui->spec_movement_mode);
    }
}
