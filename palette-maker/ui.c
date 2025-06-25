#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Double-click detection threshold (milliseconds)
#define DOUBLE_CLICK_TIME 500

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
    ui->show_load_dialog = false;
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
                    } else if (ui->show_save_dialog || ui->show_load_dialog) {
                        ui->show_save_dialog = false;
                        ui->show_load_dialog = false;
                        ui->editing_filename = false;
                    } else {
                        // Quit application with unsaved changes check
                        return !ui_check_unsaved_changes(ui, palette);
                    }
                    break;

                case SDLK_S:
                    if (event->key.mod & SDL_KMOD_CTRL) {
                        // Ctrl+S: Quick save
                        if (strlen(palette->current_file) > 0) {
                            palette_save(palette, palette->current_file);
                            palette_mark_saved(palette);
                        } else {
                            ui_show_save_dialog(ui);
                        }
                    } else {
                        // S: Show save dialog
                        ui_show_save_dialog(ui);
                    }
                    break;

                case SDLK_L:
                    if (event->key.mod & SDL_KMOD_CTRL) {
                        // Ctrl+L: Show load dialog
                        ui_show_load_dialog(ui);
                    }
                    break;

                case SDLK_RETURN:
                    if (ui->show_save_dialog) {
                        // Execute save
                        if (strlen(ui->file_input) > 0) {
                            if (palette_save(palette, ui->file_input)) {
                                palette_mark_saved(palette);
                                strncpy(palette->current_file, ui->file_input,
                                        sizeof(palette->current_file) - 1);
                            }
                            ui->show_save_dialog = false;
                            ui->editing_filename = false;
                        }
                    } else if (ui->show_load_dialog) {
                        // Execute load
                        if (strlen(ui->file_input) > 0) {
                            palette_load(palette, ui->file_input);
                            ui->show_load_dialog = false;
                            ui->editing_filename = false;
                            ui_update_rgba_fields(ui, palette);
                        }
                    }
                    break;

                default:
                    // Handle text input for RGBA fields and filename
                    if (ui->editing_filename || ui->editing_r || ui->editing_g || ui->editing_b ||
                        ui->editing_a) {
                        // Text input handled in SDL_EVENT_TEXT_INPUT
                    }
                    break;
            }
            break;

        case SDL_EVENT_TEXT_INPUT:
            // Handle text input for various fields
            if (ui->editing_filename && strlen(ui->file_input) < 255) {
                strcat(ui->file_input, event->text.text);
            } else if (ui->editing_r && strlen(ui->input_r) < 3) {
                strcat(ui->input_r, event->text.text);
                ui_apply_rgba_fields(ui, palette);
            } else if (ui->editing_g && strlen(ui->input_g) < 3) {
                strcat(ui->input_g, event->text.text);
                ui_apply_rgba_fields(ui, palette);
            } else if (ui->editing_b && strlen(ui->input_b) < 3) {
                strcat(ui->input_b, event->text.text);
                ui_apply_rgba_fields(ui, palette);
            } else if (ui->editing_a && strlen(ui->input_a) < 3) {
                strcat(ui->input_a, event->text.text);
                ui_apply_rgba_fields(ui, palette);
            }
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if (event->button.button == SDL_BUTTON_LEFT) {
                ui->mouse_down = true;
                ui->mouse_x = (int)event->button.x;
                ui->mouse_y = (int)event->button.y;

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
                    } else {
                        // Single click: select swatch
                        ui->selected_swatch = swatch;
                        ui_update_rgba_fields(ui, palette);
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
    ui_render_rect(ui, UI_PANEL_X, UI_PANEL_Y, 130, 200, panel_bg);

    // Render RGBA input fields
    PaletteColor selected_color = palette_get_color(palette, ui->selected_swatch);
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color input_bg = {48, 48, 48, 255};

    int field_y = UI_PANEL_Y + 20;

    // Red field
    ui_render_text(ui, "R:", UI_PANEL_X + 5, field_y, white);
    ui_render_rect(ui, UI_PANEL_X + 20, field_y, INPUT_FIELD_WIDTH, INPUT_FIELD_HEIGHT, input_bg);
    char r_str[4];
    snprintf(r_str, sizeof(r_str), "%d", selected_color.r);
    ui_render_text(ui, r_str, UI_PANEL_X + 25, field_y + 3, white);

    // Green field
    field_y += 25;
    ui_render_text(ui, "G:", UI_PANEL_X + 5, field_y, white);
    ui_render_rect(ui, UI_PANEL_X + 20, field_y, INPUT_FIELD_WIDTH, INPUT_FIELD_HEIGHT, input_bg);
    char g_str[4];
    snprintf(g_str, sizeof(g_str), "%d", selected_color.g);
    ui_render_text(ui, g_str, UI_PANEL_X + 25, field_y + 3, white);

    // Blue field
    field_y += 25;
    ui_render_text(ui, "B:", UI_PANEL_X + 5, field_y, white);
    ui_render_rect(ui, UI_PANEL_X + 20, field_y, INPUT_FIELD_WIDTH, INPUT_FIELD_HEIGHT, input_bg);
    char b_str[4];
    snprintf(b_str, sizeof(b_str), "%d", selected_color.b);
    ui_render_text(ui, b_str, UI_PANEL_X + 25, field_y + 3, white);

    // Alpha field
    field_y += 25;
    ui_render_text(ui, "A:", UI_PANEL_X + 5, field_y, white);
    ui_render_rect(ui, UI_PANEL_X + 20, field_y, INPUT_FIELD_WIDTH, INPUT_FIELD_HEIGHT, input_bg);
    char a_str[4];
    snprintf(a_str, sizeof(a_str), "%d", selected_color.a);
    ui_render_text(ui, a_str, UI_PANEL_X + 25, field_y + 3, white);

    // Render buttons
    field_y += 40;
    SDL_Color button_bg = {64, 64, 64, 255};

    // Save button
    ui_render_rect(ui, UI_PANEL_X + 5, field_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_bg);
    ui_render_text(ui, "Save (S)", UI_PANEL_X + 10, field_y + 5, white);

    // Load button
    field_y += 30;
    ui_render_rect(ui, UI_PANEL_X + 5, field_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_bg);
    ui_render_text(ui, "Load (L)", UI_PANEL_X + 10, field_y + 5, white);

    // Show modification indicator
    if (palette_is_modified(palette)) {
        SDL_Color red = {255, 0, 0, 255};
        ui_render_text(ui, "* Modified", UI_PANEL_X + 5, field_y + 35, red);
    }

    // Render dialogs
    if (ui->show_save_dialog || ui->show_load_dialog) {
        // Dialog background
        SDL_Color dialog_bg = {0, 0, 0, 200};
        ui_render_rect(ui, 50, 80, 220, 80, dialog_bg);

        const char* title = ui->show_save_dialog ? "Save Palette" : "Load Palette";
        ui_render_text(ui, title, 60, 90, white);

        // File input field
        SDL_Color input_field_bg = {32, 32, 32, 255};
        ui_render_rect(ui, 60, 110, 200, 20, input_field_bg);
        ui_render_text(ui, ui->file_input, 65, 113, white);

        ui_render_text(ui, "Press Enter to confirm, Esc to cancel", 60, 135, white);
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
    ui_update_rgba_fields(ui, palette);
    printf("Color picker opened for swatch %d\n", ui->selected_swatch);
}

/**
 * Close color picker dialog
 */
void ui_close_color_picker(UIState* ui) {
    if (!ui)
        return;

    ui->color_picker_open = false;
    ui->editing_r = ui->editing_g = ui->editing_b = ui->editing_a = false;
    printf("Color picker closed\n");
}

/**
 * Update RGBA input fields from selected color
 */
void ui_update_rgba_fields(UIState* ui, const Palette* palette) {
    if (!ui || !palette)
        return;

    PaletteColor color = palette_get_color(palette, ui->selected_swatch);
    snprintf(ui->input_r, sizeof(ui->input_r), "%d", color.r);
    snprintf(ui->input_g, sizeof(ui->input_g), "%d", color.g);
    snprintf(ui->input_b, sizeof(ui->input_b), "%d", color.b);
    snprintf(ui->input_a, sizeof(ui->input_a), "%d", color.a);
}

/**
 * Apply RGBA field values to selected color
 */
void ui_apply_rgba_fields(UIState* ui, Palette* palette) {
    if (!ui || !palette)
        return;

    int r = atoi(ui->input_r);
    int g = atoi(ui->input_g);
    int b = atoi(ui->input_b);
    int a = atoi(ui->input_a);

    PaletteColor new_color = palette_make_color(r, g, b, a);
    palette_set_color(palette, ui->selected_swatch, new_color);
}

/**
 * Show save file dialog
 */
void ui_show_save_dialog(UIState* ui) {
    if (!ui)
        return;

    ui->show_save_dialog = true;
    ui->show_load_dialog = false;
    ui->editing_filename = true;
    strcpy(ui->file_input, "palette.dat");
}

/**
 * Show load file dialog
 */
void ui_show_load_dialog(UIState* ui) {
    if (!ui)
        return;

    ui->show_load_dialog = true;
    ui->show_save_dialog = false;
    ui->editing_filename = true;
    strcpy(ui->file_input, "palette.dat");
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

/**
 * Simple text rendering (placeholder implementation)
 */
void ui_render_text(UIState* ui, const char* text, int x, int y, SDL_Color color) {
    if (!ui || !text)
        return;

    // This is a simplified text rendering
    // In a full implementation, you would use SDL_ttf or bitmap fonts
    // For now, we'll just draw small rectangles to represent text
    SDL_SetRenderDrawColor(ui->renderer, color.r, color.g, color.b, color.a);

    int len = (int)strlen(text);
    for (int i = 0; i < len && i < 20; i++) {
        SDL_FRect rect = {(float)(x + i * 8), (float)y, 6, 12};
        SDL_RenderFillRect(ui->renderer, &rect);
    }
}

/**
 * Render filled rectangle
 */
void ui_render_rect(UIState* ui, int x, int y, int w, int h, SDL_Color color) {
    if (!ui)
        return;

    SDL_SetRenderDrawColor(ui->renderer, color.r, color.g, color.b, color.a);
    SDL_FRect rect = {(float)x, (float)y, (float)w, (float)h};
    SDL_RenderFillRect(ui->renderer, &rect);
}

/**
 * Render rectangle outline
 */
void ui_render_rect_outline(UIState* ui, int x, int y, int w, int h, SDL_Color color) {
    if (!ui)
        return;

    SDL_SetRenderDrawColor(ui->renderer, color.r, color.g, color.b, color.a);
    SDL_FRect rect = {(float)x, (float)y, (float)w, (float)h};
    SDL_RenderRect(ui->renderer, &rect);
}
