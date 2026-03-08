#include "ui.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui_input_widgets.h"

// Double-click detection threshold (milliseconds)
#define DOUBLE_CLICK_TIME 300
#define SWATCH_ID_BASE 1000
#define ACTION_ID_SAVE 2001
#define ACTION_ID_RESET 2002
#define ACTION_ID_LOAD 2003
#define RGBA_ID_BASE 3000
#define RGBA_OP_MINUS_10 0
#define RGBA_OP_MINUS_1 1
#define RGBA_OP_PLUS_1 2
#define RGBA_OP_PLUS_10 3
#define DIALOG_ID_SAVE_YES 4001
#define DIALOG_ID_SAVE_NO 4002

#define RGBA_BUTTON_INDEX(channel, op) ((channel) * UI_RGBA_BUTTONS_PER_CHANNEL + (op))

static void ui_render_text(UIState* ui, const char* text, int x, int y, SDL_Color color);
static void ui_render_rect(UIState* ui, int x, int y, int w, int h, SDL_Color color);
static void ui_render_rect_outline(UIState* ui, int x, int y, int w, int h, SDL_Color color);

static void ui_sync_selected_swatch(UIState* ui) {
    if (!ui) {
        return;
    }

    for (int i = 0; i < UI_SWATCH_COUNT; i++) {
        ui_input_set_selected(&ui->swatch_inputs[i], i == ui->selected_swatch);
    }
}

static UIInputElement* ui_find_rgba_button(UIState* ui, int id) {
    if (!ui) {
        return NULL;
    }

    for (int i = 0; i < UI_RGBA_BUTTON_COUNT; i++) {
        if (ui->rgba_buttons[i].id == id) {
            return &ui->rgba_buttons[i];
        }
    }
    return NULL;
}

static void ui_on_rgba_value_change(int id, float value, void* userdata) {
    (void)id;
    (void)value;
    (void)userdata;
}

static void ui_on_rgba_click(int id, void* userdata) {
    UIState* ui = (UIState*)userdata;
    if (!ui || !ui->active_palette) {
        return;
    }

    const int local = id - RGBA_ID_BASE;
    if (local < 0) {
        return;
    }

    const int channel = local / 10;
    const int op = local % 10;
    if (channel < 0 || channel >= UI_RGBA_CHANNEL_COUNT) {
        return;
    }

    PaletteColor current = palette_get_color(ui->active_palette, ui->selected_swatch);
    Uint8 val = (channel == 0)   ? current.r
                : (channel == 1) ? current.g
                : (channel == 2) ? current.b
                                 : current.a;

    Uint8 new_val = val;
    switch (op) {
        case RGBA_OP_MINUS_10:
            new_val = (val >= 10) ? (Uint8)(val - 10) : 0;
            break;
        case RGBA_OP_MINUS_1:
            new_val = (val > 0) ? (Uint8)(val - 1) : 0;
            break;
        case RGBA_OP_PLUS_1:
            new_val = (val < 255) ? (Uint8)(val + 1) : 255;
            break;
        case RGBA_OP_PLUS_10:
            new_val = (val <= 245) ? (Uint8)(val + 10) : 255;
            break;
        default:
            return;
    }

    Uint8 r = current.r;
    Uint8 g = current.g;
    Uint8 b = current.b;
    Uint8 a = current.a;

    if (channel == 0) {
        r = new_val;
    } else if (channel == 1) {
        g = new_val;
    } else if (channel == 2) {
        b = new_val;
    } else {
        a = new_val;
    }

    palette_set_color(ui->active_palette, ui->selected_swatch, palette_make_color(r, g, b, a));

    UIInputElement* rgba_button = ui_find_rgba_button(ui, id);
    if (rgba_button) {
        ui_input_set_value(rgba_button, (float)new_val);
    }
}

static void ui_on_action_click(int id, void* userdata) {
    UIState* ui = (UIState*)userdata;
    if (!ui || !ui->active_palette) {
        return;
    }

    switch (id) {
        case ACTION_ID_SAVE:
            ui_show_save_dialog(ui);
            break;
        case ACTION_ID_RESET:
            ui_reset_palette(ui, ui->active_palette);
            break;
        case ACTION_ID_LOAD:
            if (palette_load(ui->active_palette, "palette.dat")) {
                printf("Palette loaded from palette.dat\n");
            }
            break;
        default:
            break;
    }
}

static void ui_on_dialog_click(int id, void* userdata) {
    UIState* ui = (UIState*)userdata;
    if (!ui || !ui->active_palette) {
        return;
    }

    if (id == DIALOG_ID_SAVE_YES) {
        if (palette_save(ui->active_palette, "palette.dat")) {
            printf("Palette saved to palette.dat\n");
        }
    }

    if (id == DIALOG_ID_SAVE_YES || id == DIALOG_ID_SAVE_NO) {
        ui->show_save_dialog = false;
    }
}

static void ui_on_swatch_click(int id, void* userdata) {
    UIState* ui = (UIState*)userdata;
    if (!ui || !ui->active_palette) {
        return;
    }

    const int swatch = id - SWATCH_ID_BASE;
    if (swatch < 0 || swatch >= UI_SWATCH_COUNT) {
        return;
    }

    Uint32 current_time = SDL_GetTicks();
    if (swatch == ui->last_click_swatch && current_time - ui->last_click_time < DOUBLE_CLICK_TIME) {
        ui->selected_swatch = swatch;
        ui_sync_selected_swatch(ui);
        ui_open_color_picker(ui, ui->active_palette);
        printf("Double-click detected on swatch %d\n", swatch);
    } else {
        ui->selected_swatch = swatch;
        ui_sync_selected_swatch(ui);
        printf("Selected swatch %d\n", swatch);
    }
    ui->last_click_swatch = swatch;
    ui->last_click_time = current_time;
}

static void ui_init_input_elements(UIState* ui, const AppConfig* config) {
    if (!ui || !config) {
        return;
    }

    for (int row = 0; row < config->grid_rows; row++) {
        for (int col = 0; col < config->grid_cols; col++) {
            int index = row * config->grid_cols + col;
            if (index >= UI_SWATCH_COUNT) {
                break;
            }

            SDL_FRect bounds = {(float)(config->grid_start_x +
                                         col * (config->swatch_size + config->swatch_border)),
                                (float)(config->grid_start_y +
                                         row * (config->swatch_size + config->swatch_border)),
                                (float)config->swatch_size, (float)config->swatch_size};
            ui_input_init(&ui->swatch_inputs[index], SWATCH_ID_BASE + index, bounds);
            ui_input_set_callbacks(&ui->swatch_inputs[index], ui_on_swatch_click, NULL, ui);
        }
    }

    int action_button_y =
        config->ui_panel_y + config->ui_panel_height - config->action_button_height - 10;
    int save_button_x = config->ui_panel_x + 10;
    int reset_button_x = save_button_x + config->action_button_width + 10;
    int load_button_x = reset_button_x + config->action_button_width + 10;

    ui_input_init(&ui->action_buttons[0], ACTION_ID_SAVE,
                  (SDL_FRect){(float)save_button_x, (float)action_button_y,
                              (float)config->action_button_width, (float)config->action_button_height});
    ui_input_set_callbacks(&ui->action_buttons[0], ui_on_action_click, NULL, ui);

    ui_input_init(&ui->action_buttons[1], ACTION_ID_RESET,
                  (SDL_FRect){(float)reset_button_x, (float)action_button_y,
                              (float)config->action_button_width, (float)config->action_button_height});
    ui_input_set_callbacks(&ui->action_buttons[1], ui_on_action_click, NULL, ui);

    ui_input_init(&ui->action_buttons[2], ACTION_ID_LOAD,
                  (SDL_FRect){(float)load_button_x, (float)action_button_y,
                              (float)config->action_button_width, (float)config->action_button_height});
    ui_input_set_callbacks(&ui->action_buttons[2], ui_on_action_click, NULL, ui);

    for (int channel = 0; channel < UI_RGBA_CHANNEL_COUNT; channel++) {
        float control_y = (float)(config->ui_panel_y + 20 + channel * config->ui_panel_row_height);
        float control_x = (float)(config->ui_panel_x + 10);

        float val_x = control_x + 50.0f;
        float btn1_x = val_x + (float)config->value_display_width + 5.0f;
        float btn2_x = btn1_x + (float)config->button_width + 5.0f;
        float btn3_x = btn2_x + (float)config->button_width + 5.0f;
        float btn4_x = btn3_x + (float)config->button_width + 5.0f;

        const float w = (float)config->button_width;
        const float h = (float)config->button_height;
        const int base_id = RGBA_ID_BASE + channel * 10;

        ui_input_init(&ui->rgba_buttons[RGBA_BUTTON_INDEX(channel, RGBA_OP_MINUS_10)],
                      base_id + RGBA_OP_MINUS_10, (SDL_FRect){btn1_x, control_y, w, h});
        ui_input_set_callbacks(&ui->rgba_buttons[RGBA_BUTTON_INDEX(channel, RGBA_OP_MINUS_10)],
                               ui_on_rgba_click, ui_on_rgba_value_change, ui);

        ui_input_init(&ui->rgba_buttons[RGBA_BUTTON_INDEX(channel, RGBA_OP_MINUS_1)],
                      base_id + RGBA_OP_MINUS_1, (SDL_FRect){btn2_x, control_y, w, h});
        ui_input_set_callbacks(&ui->rgba_buttons[RGBA_BUTTON_INDEX(channel, RGBA_OP_MINUS_1)],
                               ui_on_rgba_click, ui_on_rgba_value_change, ui);

        ui_input_init(&ui->rgba_buttons[RGBA_BUTTON_INDEX(channel, RGBA_OP_PLUS_1)],
                      base_id + RGBA_OP_PLUS_1, (SDL_FRect){btn3_x, control_y, w, h});
        ui_input_set_callbacks(&ui->rgba_buttons[RGBA_BUTTON_INDEX(channel, RGBA_OP_PLUS_1)],
                               ui_on_rgba_click, ui_on_rgba_value_change, ui);

        ui_input_init(&ui->rgba_buttons[RGBA_BUTTON_INDEX(channel, RGBA_OP_PLUS_10)],
                      base_id + RGBA_OP_PLUS_10, (SDL_FRect){btn4_x, control_y, w, h});
        ui_input_set_callbacks(&ui->rgba_buttons[RGBA_BUTTON_INDEX(channel, RGBA_OP_PLUS_10)],
                               ui_on_rgba_click, ui_on_rgba_value_change, ui);
    }

    {
        int dialog_x = (config->window_width - SAVE_DIALOG_WIDTH) / 2;
        int dialog_y = (config->window_height - SAVE_DIALOG_HEIGHT) / 2;
        int button_y = dialog_y + 50;
        int yes_button_x = dialog_x + 10;
        int no_button_x = dialog_x + 120;

        ui_input_init(&ui->save_dialog_buttons[0], DIALOG_ID_SAVE_YES,
                      (SDL_FRect){(float)yes_button_x, (float)button_y, 80.0f, 20.0f});
        ui_input_set_callbacks(&ui->save_dialog_buttons[0], ui_on_dialog_click, NULL, ui);

        ui_input_init(&ui->save_dialog_buttons[1], DIALOG_ID_SAVE_NO,
                      (SDL_FRect){(float)no_button_x, (float)button_y, 80.0f, 20.0f});
        ui_input_set_callbacks(&ui->save_dialog_buttons[1], ui_on_dialog_click, NULL, ui);
    }

    ui_sync_selected_swatch(ui);
}

static void ui_update_inputs(UIState* ui, float dt_seconds, bool mouse_pressed, bool mouse_released) {
    if (!ui) {
        return;
    }

    const bool interaction_enabled = !ui->show_save_dialog && !ui->color_picker_open;
    const bool rgba_enabled = !ui->show_save_dialog;
    const bool dialog_enabled = ui->show_save_dialog;

    for (int i = 0; i < UI_ACTION_BUTTON_COUNT; i++) {
        ui_input_set_enabled(&ui->action_buttons[i], interaction_enabled);
        ui_input_update(&ui->action_buttons[i], dt_seconds, ui->mouse_x, ui->mouse_y, ui->mouse_down,
                        mouse_pressed, mouse_released);
    }

    for (int i = 0; i < UI_SWATCH_COUNT; i++) {
        ui_input_set_enabled(&ui->swatch_inputs[i], interaction_enabled);
        ui_input_update(&ui->swatch_inputs[i], dt_seconds, ui->mouse_x, ui->mouse_y, ui->mouse_down,
                        mouse_pressed, mouse_released);
    }

    for (int i = 0; i < UI_RGBA_BUTTON_COUNT; i++) {
        ui_input_set_enabled(&ui->rgba_buttons[i], rgba_enabled);
        ui_input_update(&ui->rgba_buttons[i], dt_seconds, ui->mouse_x, ui->mouse_y, ui->mouse_down,
                        mouse_pressed, mouse_released);
    }

    for (int i = 0; i < UI_DIALOG_BUTTON_COUNT; i++) {
        ui_input_set_enabled(&ui->save_dialog_buttons[i], dialog_enabled);
        ui_input_update(&ui->save_dialog_buttons[i], dt_seconds, ui->mouse_x, ui->mouse_y,
                        ui->mouse_down, mouse_pressed, mouse_released);
    }
}

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
    ui->last_frame_ticks = SDL_GetTicks();
    ui->active_palette = NULL;
    ui->active_config = config;

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

    ui_init_input_elements(ui, config);

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
 * Get UI scale factor for mouse coordinates
 */
void ui_get_scale_factor(UIState* ui, float* scale_x, float* scale_y) {
    if (!ui || !scale_x || !scale_y)
        return;

    int win_w, win_h;
    SDL_GetWindowSize(ui->window, &win_w, &win_h);

    int rend_w = 0;
    int rend_h = 0;
    SDL_GetCurrentRenderOutputSize(ui->renderer, &rend_w, &rend_h);

    if (win_w <= 0 || win_h <= 0 || rend_w <= 0 || rend_h <= 0) {
        *scale_x = 1.0f;
        *scale_y = 1.0f;
        return;
    }

    /* Keep scaling uniform to match LETTERBOX logical presentation. */
    const float sx = (float)rend_w / (float)win_w;
    const float sy = (float)rend_h / (float)win_h;
    const float uniform = (sx < sy) ? sx : sy;

    *scale_x = uniform;
    *scale_y = uniform;
}

/**
 * Convert window coordinates to logical UI coordinates used for rendering.
 * This accounts for uniform scaling and letterbox offsets.
 */
static void ui_window_to_logical_coords(UIState* ui, const AppConfig* config, float window_x,
                                        float window_y, float* logical_x, float* logical_y) {
    if (!ui || !config || !logical_x || !logical_y) {
        return;
    }

    int win_w = 0;
    int win_h = 0;
    SDL_GetWindowSize(ui->window, &win_w, &win_h);

    if (win_w <= 0 || win_h <= 0 || config->window_width <= 0 || config->window_height <= 0) {
        *logical_x = window_x;
        *logical_y = window_y;
        return;
    }

    const float sx = (float)win_w / (float)config->window_width;
    const float sy = (float)win_h / (float)config->window_height;
    const float uniform_scale = (sx < sy) ? sx : sy;

    if (uniform_scale <= 0.0f) {
        *logical_x = window_x;
        *logical_y = window_y;
        return;
    }

    const float viewport_w = (float)config->window_width * uniform_scale;
    const float viewport_h = (float)config->window_height * uniform_scale;
    const float viewport_x = ((float)win_w - viewport_w) * 0.5f;
    const float viewport_y = ((float)win_h - viewport_h) * 0.5f;

    *logical_x = (window_x - viewport_x) / uniform_scale;
    *logical_y = (window_y - viewport_y) / uniform_scale;
}

/**
 * Handle mouse click events
 */
static bool ui_handle_mouse_click(UIState* ui, Palette* palette, float mouse_x, float mouse_y,
                                  const AppConfig* config) {
    (void)ui;
    (void)palette;
    (void)mouse_x;
    (void)mouse_y;
    (void)config;

    return false;
}

/**
 * Handle SDL events and update UI state
 */
bool ui_handle_event(UIState* ui, Palette* palette, SDL_Event* event, const AppConfig* config) {
    if (!ui || !palette)
        return false;

    ui->active_palette = palette;
    ui->active_config = config;

    switch (event->type) {
        case SDL_EVENT_QUIT:
            return !ui_check_unsaved_changes(ui, palette);

        case SDL_EVENT_KEY_DOWN:
            if (ui->show_save_dialog) {
                if (event->key.key == SDLK_RETURN || event->key.key == SDLK_KP_ENTER) {
                    if (palette_save(palette, "palette.dat")) {
                        printf("Palette saved to palette.dat\n");
                    }
                    ui->show_save_dialog = false;
                } else if (event->key.key == SDLK_ESCAPE) {
                    ui->show_save_dialog = false;
                }
            } else if (ui->color_picker_open) {
                if (event->key.key == SDLK_ESCAPE) {
                    ui_close_color_picker(ui);
                }
            } else {
                switch (event->key.key) {
                    case SDLK_S:
                        if (event->key.mod & SDL_KMOD_CTRL) {
                            if (palette_save(palette, "palette.dat")) {
                                printf("Palette quick-saved to palette.dat\n");
                            }
                        } else {
                            ui_show_save_dialog(ui);
                        }
                        break;
                    case SDLK_L:
                        if (event->key.mod & SDL_KMOD_CTRL) {
                            if (palette_load(palette, "palette.dat")) {
                                printf("Palette quick-loaded from palette.dat\n");
                            }
                        } else {
                            // In a real application, this would open a file dialog.
                            // For now, we'll just load the default palette.
                            if (palette_load(palette, "palette.dat")) {
                                printf("Palette loaded from palette.dat\n");
                            }
                        }
                        break;
                    case SDLK_R:
                        ui_reset_palette(ui, palette);
                        break;
                }
            }
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if (event->button.button == SDL_BUTTON_LEFT) {
                ui->mouse_down = true;
                ui_window_to_logical_coords(ui, config, event->button.x, event->button.y,
                                            &ui->mouse_x, &ui->mouse_y);

                if (!ui_handle_mouse_click(ui, palette, ui->mouse_x, ui->mouse_y, config)) {
                    ui_update_inputs(ui, 1.0f / 60.0f, true, false);
                } else {
                    ui_update_inputs(ui, 1.0f / 60.0f, false, false);
                }
            }
            break;

        case SDL_EVENT_MOUSE_BUTTON_UP:
            if (event->button.button == SDL_BUTTON_LEFT) {
                ui->mouse_down = false;
                ui_window_to_logical_coords(ui, config, event->button.x, event->button.y,
                                            &ui->mouse_x, &ui->mouse_y);
                ui_update_inputs(ui, 1.0f / 60.0f, false, true);
            }
            break;

        case SDL_EVENT_MOUSE_MOTION: {
            ui_window_to_logical_coords(ui, config, event->motion.x, event->motion.y,
                                        &ui->mouse_x, &ui->mouse_y);
            ui_update_inputs(ui, 1.0f / 60.0f, false, false);
            break;
        }
    }

    return true;
}

/**
 * Render the complete UI interface
 */
void ui_render(UIState* ui, const Palette* palette, const AppConfig* config) {
    if (!ui || !palette)
        return;

    ui->active_palette = (Palette*)palette;
    ui->active_config = config;

    Uint64 now_ticks = SDL_GetTicks();
    float dt_seconds = 1.0f / 60.0f;
    if (ui->last_frame_ticks > 0 && now_ticks >= ui->last_frame_ticks) {
        dt_seconds = (float)(now_ticks - ui->last_frame_ticks) / 1000.0f;
        if (dt_seconds > 0.05f) {
            dt_seconds = 0.05f;
        }
    }
    ui->last_frame_ticks = now_ticks;
    ui_update_inputs(ui, dt_seconds, false, false);

    SDL_Color bg_color = {config->background_color.r, config->background_color.g,
                          config->background_color.b, config->background_color.a};
    SDL_SetRenderDrawColor(ui->renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
    SDL_RenderClear(ui->renderer);

    SDL_Color text_color = {config->text_color.r, config->text_color.g, config->text_color.b,
                            config->text_color.a};
    SDL_Color button_base = {config->button_color.r, config->button_color.g, config->button_color.b,
                             config->button_color.a};
    SDL_Color button_hover = {config->button_hover_color.r, config->button_hover_color.g,
                              config->button_hover_color.b, config->button_hover_color.a};

    // Render swatch grid
    for (int row = 0; row < config->grid_rows; row++) {
        for (int col = 0; col < config->grid_cols; col++) {
            int index = row * config->grid_cols + col;
            if (index >= UI_SWATCH_COUNT) {
                continue;
            }

            UIInputElement* swatch_input = &ui->swatch_inputs[index];
            int x = (int)swatch_input->bounds.x;
            int y = (int)swatch_input->bounds.y;

            PaletteColor p_color = palette_get_color(palette, index);
            SDL_Color swatch_color = {p_color.r, p_color.g, p_color.b, p_color.a};
            ui_render_rect(ui, x, y, config->swatch_size, config->swatch_size, swatch_color);

            if (swatch_input->hover_anim_t > 0.01f) {
                SDL_Color hover_outline = {255, 255, 255, (Uint8)(100 + 120 * swatch_input->hover_anim_t)};
                ui_render_rect_outline(ui, x - 1, y - 1, config->swatch_size + 2,
                                       config->swatch_size + 2, hover_outline);
            }

            if (swatch_input->selected_anim_t > 0.01f) {
                int expand = 2 + (int)(2.0f * swatch_input->selected_anim_t);
                SDL_Color selection_color = {config->selected_color.r, config->selected_color.g,
                                             config->selected_color.b,
                                             (Uint8)(140 + 115 * swatch_input->selected_anim_t)};
                ui_render_rect_outline(ui, x - expand, y - expand, config->swatch_size + expand * 2,
                                       config->swatch_size + expand * 2, selection_color);
            }

            char index_str[4];
            snprintf(index_str, sizeof(index_str), "%d", index);
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
    const char* action_labels[UI_ACTION_BUTTON_COUNT] = {"Save (S)", "Reset (R)", "Load (L)"};
    for (int i = 0; i < UI_ACTION_BUTTON_COUNT; i++) {
        UIInputElement* btn = &ui->action_buttons[i];
        ui_input_widgets_render_button(ui->renderer, btn, button_base, button_hover,
                                       (SDL_Color){96, 96, 96, 255}, 0.85f);
        ui_render_text(ui, action_labels[i], (int)btn->bounds.x + 5, (int)btn->bounds.y + 5,
                       text_color);
    }

    int action_button_y =
        config->ui_panel_y + config->ui_panel_height - config->action_button_height - 10;

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

        SDL_Color white = {255, 255, 255, 255};
        ui_render_text(ui, "Save Palette", dialog_x + 10, dialog_y + 10, white);
        ui_render_text(ui, "Save changes to palette.dat?", dialog_x + 10, dialog_y + 30, white);

        for (int i = 0; i < UI_DIALOG_BUTTON_COUNT; i++) {
            UIInputElement* btn = &ui->save_dialog_buttons[i];
            ui_input_widgets_render_button(ui->renderer, btn, button_base, button_hover,
                                           (SDL_Color){96, 96, 96, 255}, 0.85f);
        }

        ui_render_text(ui, "Yes", (int)ui->save_dialog_buttons[0].bounds.x + 25,
                       (int)ui->save_dialog_buttons[0].bounds.y + 5, text_color);
        ui_render_text(ui, "No", (int)ui->save_dialog_buttons[1].bounds.x + 30,
                       (int)ui->save_dialog_buttons[1].bounds.y + 5, text_color);
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
static void ui_render_text(UIState* ui, const char* text, int x, int y, SDL_Color color) {
    if (!ui || !text) {
        return;
    }

    SDL_SetRenderDrawColor(ui->renderer, color.r, color.g, color.b, color.a);

    int len = (int)strlen(text);
    for (int i = 0; i < len && i < 32; i++) {
        int char_index = get_char_index(text[i]);
        const uint8_t* pattern = font_patterns[(char_index >= 0 && char_index < GLYPH_COUNT)
                                                   ? char_index
                                                   : 0];

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
static void ui_render_rect(UIState* ui, int x, int y, int w, int h, SDL_Color color) {
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
static void ui_render_rect_outline(UIState* ui, int x, int y, int w, int h, SDL_Color color) {
    if (!ui) {
        return;
    }

    SDL_SetRenderDrawColor(ui->renderer, color.r, color.g, color.b, color.a);
    SDL_FRect rect = {(float)x, (float)y, (float)w, (float)h};
    SDL_RenderRect(ui->renderer, &rect);
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
    SDL_Color button_base = {config->button_color.r, config->button_color.g, config->button_color.b,
                             config->button_color.a};
    SDL_Color button_hover = {config->button_hover_color.r, config->button_hover_color.g,
                              config->button_hover_color.b, config->button_hover_color.a};
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

        UIInputElement* b1 = &ui->rgba_buttons[RGBA_BUTTON_INDEX(i, RGBA_OP_MINUS_10)];
        UIInputElement* b2 = &ui->rgba_buttons[RGBA_BUTTON_INDEX(i, RGBA_OP_MINUS_1)];
        UIInputElement* b3 = &ui->rgba_buttons[RGBA_BUTTON_INDEX(i, RGBA_OP_PLUS_1)];
        UIInputElement* b4 = &ui->rgba_buttons[RGBA_BUTTON_INDEX(i, RGBA_OP_PLUS_10)];

        // -10 Button
        ui_input_widgets_render_button(ui->renderer, b1, button_base, button_hover,
                                       (SDL_Color){96, 96, 96, 255}, 0.85f);
        ui_render_text(ui, "-10", btn1_x + 5, control_y + 5, text_color);

        // -1 Button
        ui_input_widgets_render_button(ui->renderer, b2, button_base, button_hover,
                                       (SDL_Color){96, 96, 96, 255}, 0.85f);
        ui_render_text(ui, "-1", btn2_x + 8, control_y + 5, text_color);

        // Value Display
        ui_render_rect(ui, val_x, control_y, config->value_display_width, config->button_height,
                       value_bg);
        char val_str[4];
        snprintf(val_str, sizeof(val_str), "%d", values[i]);
        ui_render_text(ui, val_str, val_x + 10, control_y + 5, text_color);

        // +1 Button
        ui_input_widgets_render_button(ui->renderer, b3, button_base, button_hover,
                                       (SDL_Color){96, 96, 96, 255}, 0.85f);
        ui_render_text(ui, "+1", btn3_x + 8, control_y + 5, text_color);

        // +10 Button
        ui_input_widgets_render_button(ui->renderer, b4, button_base, button_hover,
                                       (SDL_Color){96, 96, 96, 255}, 0.85f);
        ui_render_text(ui, "+10", btn4_x + 5, control_y + 5, text_color);
    }
}
