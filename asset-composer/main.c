#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../shared/text_renderer/text_renderer.h"
#include "../shared/ui_framework/ui_viewport.h"
#include "asset_db.h"

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 760

#define LIST_X 16
#define LIST_Y 56
#define LIST_W 420
#define LIST_H 660
#define LIST_ROW_H 24

#define DETAILS_X 452
#define DETAILS_Y 56
#define DETAILS_W 732
#define DETAILS_H 660

#define STATUS_BAR_HEIGHT 24
#define MENU_BUTTON_X 16
#define MENU_BUTTON_Y 14
#define MENU_BUTTON_W 100
#define MENU_BUTTON_H 26
#define MENU_DIALOG_W 460
#define MENU_OPTION_W 360
#define MENU_OPTION_H 30
#define MENU_OPTION_GAP 10

typedef enum {
    MENU_NONE = 0,
    MENU_STARTUP = 1,
    MENU_MAIN = 2,
    MENU_CONFIRM_NEW = 3,
    MENU_CONFIRM_OPEN = 4,
    MENU_CONFIRM_QUIT = 5,
} MenuMode;

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    TextRenderer text_renderer;
    bool running;

    int mouse_x;
    int mouse_y;

    MenuMode menu_mode;
    bool dirty;

    AssetDb db;
    int selected_index;
    int list_scroll;

    char assets_path[260];
    char status[200];
} AppState;

static bool file_exists(const char* path) {
    if (!path || path[0] == '\0') {
        return false;
    }
    FILE* file = fopen(path, "rb");
    if (!file) {
        return false;
    }
    fclose(file);
    return true;
}

static void resolve_path(char* out, size_t out_size, const char* const* candidates, int count) {
    if (!out || out_size == 0 || !candidates || count <= 0) {
        return;
    }

    for (int i = 0; i < count; ++i) {
        if (file_exists(candidates[i])) {
            strncpy(out, candidates[i], out_size - 1);
            out[out_size - 1] = '\0';
            return;
        }
    }

    strncpy(out, candidates[0], out_size - 1);
    out[out_size - 1] = '\0';
}

static void set_status(AppState* app, const char* text) {
    if (!app || !text) {
        return;
    }
    strncpy(app->status, text, sizeof(app->status) - 1);
    app->status[sizeof(app->status) - 1] = '\0';
}

static void render_text_line(AppState* app, const char* text, int x, int y, SDL_Color color) {
    if (!app || !text) {
        return;
    }
    text_render_string(&app->text_renderer, text, x, y, color);
}

static int ui_text_width(const char* text) {
    if (!text) {
        return 0;
    }
    const int len = (int)strlen(text);
    if (len <= 0) {
        return 0;
    }
    return len * 6 - 1;
}

static bool point_in_frect(int x, int y, const SDL_FRect* rect) {
    if (!rect) {
        return false;
    }
    return (float)x >= rect->x && (float)x <= rect->x + rect->w && (float)y >= rect->y &&
           (float)y <= rect->y + rect->h;
}

static void render_button(AppState* app, const SDL_FRect* rect, const char* label, bool selected) {
    if (!app || !app->renderer || !rect || !label) {
        return;
    }

    SDL_Color fill = selected ? (SDL_Color){60, 102, 146, 255} : (SDL_Color){58, 58, 58, 255};
    SDL_Color border = selected ? (SDL_Color){218, 236, 255, 255} : (SDL_Color){132, 132, 132, 255};
    SDL_SetRenderDrawColor(app->renderer, fill.r, fill.g, fill.b, fill.a);
    SDL_RenderFillRect(app->renderer, rect);
    SDL_SetRenderDrawColor(app->renderer, border.r, border.g, border.b, border.a);
    SDL_RenderRect(app->renderer, rect);

    int tx = (int)(rect->x + (rect->w - (float)ui_text_width(label)) * 0.5f);
    int ty = (int)(rect->y + (rect->h - 7.0f) * 0.5f);
    render_text_line(app, label, tx, ty, (SDL_Color){255, 255, 255, 255});
}

static SDL_FRect menu_button_rect(void) {
    return (SDL_FRect){(float)MENU_BUTTON_X, (float)MENU_BUTTON_Y, (float)MENU_BUTTON_W,
                       (float)MENU_BUTTON_H};
}

static bool menu_is_open(const AppState* app) {
    return app && app->menu_mode != MENU_NONE;
}

static int menu_option_count(MenuMode mode) {
    if (mode == MENU_STARTUP) {
        return 3;
    }
    if (mode == MENU_MAIN) {
        return 7;
    }
    if (mode == MENU_CONFIRM_NEW || mode == MENU_CONFIRM_OPEN) {
        return 2;
    }
    if (mode == MENU_CONFIRM_QUIT) {
        return 3;
    }
    return 0;
}

static const char* menu_option_label(MenuMode mode, int index) {
    static const char* startup[] = {"Open", "New", "Quit"};
    static const char* main[] = {"Open", "Save", "New", "Add Cell", "Delete", "Quit", "Resume"};
    static const char* confirm_new[] = {"Cancel", "Create New"};
    static const char* confirm_open[] = {"Cancel", "Open"};
    static const char* confirm_quit[] = {"Cancel", "Discard", "Save+Quit"};

    if (mode == MENU_STARTUP) {
        return (index >= 0 && index < 3) ? startup[index] : "";
    }
    if (mode == MENU_MAIN) {
        return (index >= 0 && index < 7) ? main[index] : "";
    }
    if (mode == MENU_CONFIRM_NEW) {
        return (index >= 0 && index < 2) ? confirm_new[index] : "";
    }
    if (mode == MENU_CONFIRM_OPEN) {
        return (index >= 0 && index < 2) ? confirm_open[index] : "";
    }
    if (mode == MENU_CONFIRM_QUIT) {
        return (index >= 0 && index < 3) ? confirm_quit[index] : "";
    }
    return "";
}

static SDL_FRect menu_dialog_rect(MenuMode mode) {
    const int option_count = menu_option_count(mode);
    const int content_h =
        option_count > 0 ? option_count * MENU_OPTION_H + (option_count - 1) * MENU_OPTION_GAP : 0;
    const int dialog_h = 70 + content_h + 20;
    const float x = (float)((WINDOW_WIDTH - MENU_DIALOG_W) / 2);
    const float y = (float)((WINDOW_HEIGHT - dialog_h) / 2);
    return (SDL_FRect){x, y, (float)MENU_DIALOG_W, (float)dialog_h};
}

static SDL_FRect menu_option_rect(MenuMode mode, int index) {
    SDL_FRect dialog = menu_dialog_rect(mode);
    const float x = dialog.x + (dialog.w - (float)MENU_OPTION_W) * 0.5f;
    const float y = dialog.y + 54.0f + (float)index * (float)(MENU_OPTION_H + MENU_OPTION_GAP);
    return (SDL_FRect){x, y, (float)MENU_OPTION_W, (float)MENU_OPTION_H};
}

static void select_clamp(AppState* app) {
    if (!app) {
        return;
    }
    if (app->db.count == 0) {
        app->selected_index = -1;
        app->list_scroll = 0;
        return;
    }
    if (app->selected_index < 0) {
        app->selected_index = 0;
    } else if ((size_t)app->selected_index >= app->db.count) {
        app->selected_index = (int)app->db.count - 1;
    }

    const int max_rows = LIST_H / LIST_ROW_H;
    if (max_rows > 0) {
        if (app->selected_index < app->list_scroll) {
            app->list_scroll = app->selected_index;
        } else if (app->selected_index >= app->list_scroll + max_rows) {
            app->list_scroll = app->selected_index - max_rows + 1;
        }
    }
    if (app->list_scroll < 0) {
        app->list_scroll = 0;
    }
}

static uint16_t find_next_asset_id(const AssetDb* db) {
    uint16_t max_id = 0;
    bool has_any = false;
    if (!db) {
        return 0;
    }
    for (size_t i = 0; i < db->count; ++i) {
        if (!has_any || db->records[i].asset_id > max_id) {
            max_id = db->records[i].asset_id;
            has_any = true;
        }
    }
    if (!has_any) {
        return 0;
    }
    if (max_id == 65535u) {
        return 65535u;
    }
    return (uint16_t)(max_id + 1u);
}

static void action_new(AppState* app) {
    if (!app) {
        return;
    }
    asset_db_init_default(&app->db);
    app->dirty = true;
    app->selected_index = 0;
    app->list_scroll = 0;
    set_status(app, "New asset database created");
}

static void action_open(AppState* app) {
    if (!app) {
        return;
    }
    char error[120] = {0};
    if (asset_db_load(&app->db, app->assets_path, error, sizeof(error))) {
        app->dirty = false;
        select_clamp(app);
        set_status(app, "assets.dat loaded");
    } else {
        char status[180];
        snprintf(status, sizeof(status), "Open failed: %s", error[0] ? error : "unknown");
        set_status(app, status);
    }
}

static bool action_save(AppState* app) {
    if (!app) {
        return false;
    }
    char error[120] = {0};
    if (asset_db_save(&app->db, app->assets_path, error, sizeof(error))) {
        app->dirty = false;
        set_status(app, "assets.dat saved");
        return true;
    }

    char status[180];
    snprintf(status, sizeof(status), "Save failed: %s", error[0] ? error : "unknown");
    set_status(app, status);
    return false;
}

static void action_add_cell(AppState* app) {
    if (!app) {
        return;
    }
    uint16_t next_id = find_next_asset_id(&app->db);
    if (next_id == 65535u && app->db.count > 0 && app->db.records[app->db.count - 1].asset_id == 65535u) {
        set_status(app, "Add failed: asset id limit reached");
        return;
    }

    if (!asset_db_add_default_cell(&app->db, next_id)) {
        set_status(app, "Add failed: database full");
        return;
    }

    app->selected_index = (int)app->db.count - 1;
    app->dirty = true;
    select_clamp(app);
    set_status(app, "CELL32 asset added");
}

static void action_delete_selected(AppState* app) {
    if (!app || app->selected_index < 0) {
        return;
    }
    if (!asset_db_remove_at(&app->db, (size_t)app->selected_index)) {
        set_status(app, "Delete failed");
        return;
    }
    app->dirty = true;
    select_clamp(app);
    set_status(app, "Asset removed");
}

static void begin_new_flow(AppState* app) {
    if (!app) {
        return;
    }
    if (app->menu_mode == MENU_STARTUP) {
        action_new(app);
        app->menu_mode = MENU_NONE;
        return;
    }
    app->menu_mode = app->dirty ? MENU_CONFIRM_NEW : MENU_MAIN;
    if (!app->dirty) {
        action_new(app);
        app->menu_mode = MENU_NONE;
    }
}

static void begin_open_flow(AppState* app) {
    if (!app) {
        return;
    }
    if (app->menu_mode == MENU_STARTUP) {
        action_open(app);
        app->menu_mode = MENU_NONE;
        return;
    }
    if (app->dirty) {
        app->menu_mode = MENU_CONFIRM_OPEN;
        return;
    }
    action_open(app);
    app->menu_mode = MENU_NONE;
}

static void begin_quit_flow(AppState* app) {
    if (!app) {
        return;
    }
    if (!app->dirty) {
        app->menu_mode = MENU_NONE;
        app->running = false;
        return;
    }
    app->menu_mode = MENU_CONFIRM_QUIT;
}

static void menu_cancel(AppState* app) {
    if (!app) {
        return;
    }
    if (app->menu_mode == MENU_STARTUP) {
        return;
    }
    app->menu_mode = MENU_NONE;
}

static bool handle_menu_click(AppState* app, int x, int y) {
    if (!app) {
        return false;
    }

    SDL_FRect menu_button = menu_button_rect();
    if (!menu_is_open(app) && point_in_frect(x, y, &menu_button)) {
        app->menu_mode = MENU_MAIN;
        return true;
    }

    if (!menu_is_open(app)) {
        return false;
    }

    SDL_FRect dialog = menu_dialog_rect(app->menu_mode);
    if (!point_in_frect(x, y, &dialog)) {
        menu_cancel(app);
        return true;
    }

    const int option_count = menu_option_count(app->menu_mode);
    for (int i = 0; i < option_count; ++i) {
        SDL_FRect option = menu_option_rect(app->menu_mode, i);
        if (!point_in_frect(x, y, &option)) {
            continue;
        }

        switch (app->menu_mode) {
            case MENU_STARTUP:
                if (i == 0) {
                    begin_open_flow(app);
                } else if (i == 1) {
                    begin_new_flow(app);
                } else if (i == 2) {
                    begin_quit_flow(app);
                }
                return true;
            case MENU_MAIN:
                if (i == 0) {
                    begin_open_flow(app);
                } else if (i == 1) {
                    (void)action_save(app);
                    app->menu_mode = MENU_NONE;
                } else if (i == 2) {
                    begin_new_flow(app);
                } else if (i == 3) {
                    action_add_cell(app);
                    app->menu_mode = MENU_NONE;
                } else if (i == 4) {
                    action_delete_selected(app);
                    app->menu_mode = MENU_NONE;
                } else if (i == 5) {
                    begin_quit_flow(app);
                } else if (i == 6) {
                    app->menu_mode = MENU_NONE;
                }
                return true;
            case MENU_CONFIRM_NEW:
                if (i == 0) {
                    app->menu_mode = MENU_MAIN;
                } else if (i == 1) {
                    action_new(app);
                    app->menu_mode = MENU_NONE;
                }
                return true;
            case MENU_CONFIRM_OPEN:
                if (i == 0) {
                    app->menu_mode = MENU_MAIN;
                } else if (i == 1) {
                    action_open(app);
                    app->menu_mode = MENU_NONE;
                }
                return true;
            case MENU_CONFIRM_QUIT:
                if (i == 0) {
                    app->menu_mode = MENU_NONE;
                } else if (i == 1) {
                    app->running = false;
                } else if (i == 2) {
                    if (action_save(app)) {
                        app->running = false;
                    } else {
                        app->menu_mode = MENU_MAIN;
                    }
                }
                return true;
            case MENU_NONE:
            default:
                return true;
        }
    }

    return true;
}

static void render_list_panel(AppState* app) {
    if (!app || !app->renderer) {
        return;
    }

    SDL_FRect panel = {(float)LIST_X, (float)LIST_Y, (float)LIST_W, (float)LIST_H};
    SDL_SetRenderDrawColor(app->renderer, 35, 35, 35, 255);
    SDL_RenderFillRect(app->renderer, &panel);
    SDL_SetRenderDrawColor(app->renderer, 95, 95, 95, 255);
    SDL_RenderRect(app->renderer, &panel);

    render_text_line(app, "Assets", LIST_X + 8, LIST_Y + 6, (SDL_Color){225, 225, 225, 255});
    char count_line[64];
    snprintf(count_line, sizeof(count_line), "Count: %u", (unsigned)app->db.count);
    render_text_line(app, count_line, LIST_X + 260, LIST_Y + 6, (SDL_Color){175, 175, 175, 255});

    const int visible_rows = (LIST_H - 30) / LIST_ROW_H;
    const int first = app->list_scroll;
    for (int i = 0; i < visible_rows; ++i) {
        const int index = first + i;
        if (index < 0 || (size_t)index >= app->db.count) {
            break;
        }

        const int y = LIST_Y + 28 + i * LIST_ROW_H;
        SDL_FRect row = {(float)(LIST_X + 4), (float)y, (float)(LIST_W - 8), (float)(LIST_ROW_H - 2)};
        const bool selected = index == app->selected_index;
        SDL_SetRenderDrawColor(app->renderer, selected ? 62 : 48, selected ? 104 : 48,
                               selected ? 146 : 48, 255);
        SDL_RenderFillRect(app->renderer, &row);
        SDL_SetRenderDrawColor(app->renderer, selected ? 180 : 78, selected ? 210 : 78,
                               selected ? 240 : 78, 255);
        SDL_RenderRect(app->renderer, &row);

        const AssetRecord* rec = &app->db.records[index];
        char label[128];
        snprintf(label, sizeof(label), "#%04u  %-16.16s  type:%u", (unsigned)rec->asset_id, rec->name,
                 (unsigned)rec->type);
        render_text_line(app, label, LIST_X + 10, y + 8, (SDL_Color){242, 242, 242, 255});
    }
}

static void render_details_panel(AppState* app) {
    if (!app || !app->renderer) {
        return;
    }

    SDL_FRect panel = {(float)DETAILS_X, (float)DETAILS_Y, (float)DETAILS_W, (float)DETAILS_H};
    SDL_SetRenderDrawColor(app->renderer, 35, 35, 35, 255);
    SDL_RenderFillRect(app->renderer, &panel);
    SDL_SetRenderDrawColor(app->renderer, 95, 95, 95, 255);
    SDL_RenderRect(app->renderer, &panel);

    render_text_line(app, "Asset Details", DETAILS_X + 8, DETAILS_Y + 6, (SDL_Color){225, 225, 225, 255});
    render_text_line(app, "Schema: ASDB v1 (CELL32 only in skeleton)", DETAILS_X + 8, DETAILS_Y + 22,
                     (SDL_Color){180, 180, 180, 255});

    if (app->selected_index < 0 || (size_t)app->selected_index >= app->db.count) {
        render_text_line(app, "No asset selected", DETAILS_X + 8, DETAILS_Y + 58,
                         (SDL_Color){200, 200, 200, 255});
        return;
    }

    const AssetRecord* record = &app->db.records[app->selected_index];
    char line[128];
    snprintf(line, sizeof(line), "ID: %u", (unsigned)record->asset_id);
    render_text_line(app, line, DETAILS_X + 8, DETAILS_Y + 58, (SDL_Color){220, 220, 220, 255});
    snprintf(line, sizeof(line), "Name: %.16s", record->name);
    render_text_line(app, line, DETAILS_X + 8, DETAILS_Y + 74, (SDL_Color){220, 220, 220, 255});
    snprintf(line, sizeof(line), "Type: %u (CELL32=%u)", (unsigned)record->type, (unsigned)ASSET_TYPE_CELL32);
    render_text_line(app, line, DETAILS_X + 8, DETAILS_Y + 90, (SDL_Color){220, 220, 220, 255});
    snprintf(line, sizeof(line), "Flags: %u", (unsigned)record->flags);
    render_text_line(app, line, DETAILS_X + 8, DETAILS_Y + 106, (SDL_Color){220, 220, 220, 255});

    render_text_line(app, "Tile Refs (4x4):", DETAILS_X + 8, DETAILS_Y + 132,
                     (SDL_Color){205, 205, 205, 255});
    for (int row = 0; row < 4; ++row) {
        char row_text[96];
        snprintf(row_text, sizeof(row_text), "%03u %03u %03u %03u",
                 (unsigned)record->tile_refs[row * 4 + 0], (unsigned)record->tile_refs[row * 4 + 1],
                 (unsigned)record->tile_refs[row * 4 + 2], (unsigned)record->tile_refs[row * 4 + 3]);
        render_text_line(app, row_text, DETAILS_X + 12, DETAILS_Y + 150 + row * 16,
                         (SDL_Color){225, 225, 225, 255});
    }
}

static void render_menu_overlay(AppState* app) {
    if (!app || !app->renderer || !menu_is_open(app)) {
        return;
    }

    SDL_SetRenderDrawBlendMode(app->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 130);
    SDL_FRect fade = {0.0f, 0.0f, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT};
    SDL_RenderFillRect(app->renderer, &fade);

    SDL_FRect dialog = menu_dialog_rect(app->menu_mode);
    SDL_SetRenderDrawColor(app->renderer, 34, 34, 34, 238);
    SDL_RenderFillRect(app->renderer, &dialog);
    SDL_SetRenderDrawColor(app->renderer, 130, 130, 130, 255);
    SDL_RenderRect(app->renderer, &dialog);

    const char* title = "Menu";
    const char* subtitle = "";
    if (app->menu_mode == MENU_STARTUP) {
        title = "Asset Composer";
        subtitle = "Open existing assets.dat or create a new one";
    } else if (app->menu_mode == MENU_MAIN) {
        title = "Asset Menu";
        subtitle = "Open / Save / New / Add / Delete / Quit";
    } else if (app->menu_mode == MENU_CONFIRM_NEW) {
        title = "Confirm New";
        subtitle = "Current DB has unsaved changes";
    } else if (app->menu_mode == MENU_CONFIRM_OPEN) {
        title = "Confirm Open";
        subtitle = "Current DB has unsaved changes";
    } else if (app->menu_mode == MENU_CONFIRM_QUIT) {
        title = "Confirm Quit";
        subtitle = "Current DB has unsaved changes";
    }

    render_text_line(app, title, (int)dialog.x + 12, (int)dialog.y + 12, (SDL_Color){240, 240, 240, 255});
    render_text_line(app, subtitle, (int)dialog.x + 12, (int)dialog.y + 28,
                     (SDL_Color){190, 190, 190, 255});

    const int option_count = menu_option_count(app->menu_mode);
    for (int i = 0; i < option_count; ++i) {
        SDL_FRect option = menu_option_rect(app->menu_mode, i);
        render_button(app, &option, menu_option_label(app->menu_mode, i), false);
    }

    SDL_SetRenderDrawBlendMode(app->renderer, SDL_BLENDMODE_NONE);
}

static void render_ui(AppState* app) {
    if (!app || !app->renderer) {
        return;
    }

    SDL_FRect menu_button = menu_button_rect();
    render_button(app, &menu_button, "Menu", menu_is_open(app));

    render_text_line(app, "AssetComposer v0.1", MENU_BUTTON_X + 118, MENU_BUTTON_Y + 8,
                     (SDL_Color){220, 220, 220, 255});
    render_text_line(app, "O open, S save, N new, A add, Del remove, Esc menu",
                     MENU_BUTTON_X + 260, MENU_BUTTON_Y + 8, (SDL_Color){160, 160, 160, 255});

    char path_line[300];
    snprintf(path_line, sizeof(path_line), "File: %s", app->assets_path);
    render_text_line(app, path_line, 16, WINDOW_HEIGHT - STATUS_BAR_HEIGHT - 16,
                     (SDL_Color){165, 165, 165, 255});

    SDL_FRect status_bar = {0.0f, (float)(WINDOW_HEIGHT - STATUS_BAR_HEIGHT), (float)WINDOW_WIDTH,
                            (float)STATUS_BAR_HEIGHT};
    SDL_SetRenderDrawColor(app->renderer, 22, 22, 22, 255);
    SDL_RenderFillRect(app->renderer, &status_bar);
    SDL_SetRenderDrawColor(app->renderer, 70, 70, 70, 255);
    SDL_RenderRect(app->renderer, &status_bar);

    char status_line[240];
    snprintf(status_line, sizeof(status_line), "[%s] %s", app->dirty ? "DIRTY" : "CLEAN", app->status);
    render_text_line(app, status_line, 8, WINDOW_HEIGHT - 18, (SDL_Color){245, 245, 245, 255});
}

static void handle_mouse_press(AppState* app, bool left_button) {
    if (!app || !left_button) {
        return;
    }

    if (handle_menu_click(app, app->mouse_x, app->mouse_y)) {
        return;
    }
    if (menu_is_open(app)) {
        return;
    }

    SDL_FRect list_body = {(float)LIST_X, (float)(LIST_Y + 28), (float)LIST_W, (float)(LIST_H - 28)};
    if (point_in_frect(app->mouse_x, app->mouse_y, &list_body)) {
        const int local_y = app->mouse_y - (LIST_Y + 28);
        const int row = local_y / LIST_ROW_H;
        const int index = app->list_scroll + row;
        if (index >= 0 && (size_t)index < app->db.count) {
            app->selected_index = index;
            select_clamp(app);
        }
    }
}

static bool app_init(AppState* app) {
    if (!app) {
        return false;
    }

    memset(app, 0, sizeof(*app));
    app->running = true;
    app->selected_index = -1;
    app->menu_mode = MENU_STARTUP;
    set_status(app, "Startup menu: choose Open or New");

    const char* path_candidates[] = {"assets.dat", "../assets.dat", "../../assets.dat"};
    resolve_path(app->assets_path, sizeof(app->assets_path), path_candidates, 3);
    asset_db_init_default(&app->db);
    app->selected_index = 0;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        printf("SDL init failed: %s\n", SDL_GetError());
        return false;
    }

    app->window = SDL_CreateWindow("Asset Composer v0.1", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (!app->window) {
        printf("Create window failed: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }

    app->renderer = SDL_CreateRenderer(app->window, NULL);
    if (!app->renderer) {
        printf("Create renderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(app->window);
        SDL_Quit();
        return false;
    }

    if (!SDL_SetRenderLogicalPresentation(app->renderer, WINDOW_WIDTH, WINDOW_HEIGHT,
                                          SDL_LOGICAL_PRESENTATION_LETTERBOX)) {
        printf("Warning: failed to set logical presentation: %s\n", SDL_GetError());
    }

    if (!text_renderer_init(&app->text_renderer, app->renderer)) {
        printf("Text renderer init failed\n");
        SDL_DestroyRenderer(app->renderer);
        SDL_DestroyWindow(app->window);
        SDL_Quit();
        return false;
    }

    return true;
}

static void app_cleanup(AppState* app) {
    if (!app) {
        return;
    }
    text_renderer_cleanup(&app->text_renderer);
    if (app->renderer) {
        SDL_DestroyRenderer(app->renderer);
        app->renderer = NULL;
    }
    if (app->window) {
        SDL_DestroyWindow(app->window);
        app->window = NULL;
    }
    SDL_Quit();
}

static void app_handle_events(AppState* app) {
    if (!app) {
        return;
    }

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                begin_quit_flow(app);
                break;
            case SDL_EVENT_MOUSE_MOTION: {
                float logical_x = event.motion.x;
                float logical_y = event.motion.y;
                ui_viewport_window_to_logical(app->renderer, event.motion.x, event.motion.y, &logical_x,
                                              &logical_y);
                app->mouse_x = (int)logical_x;
                app->mouse_y = (int)logical_y;
                break;
            }
            case SDL_EVENT_MOUSE_BUTTON_DOWN: {
                float logical_x = event.button.x;
                float logical_y = event.button.y;
                ui_viewport_window_to_logical(app->renderer, event.button.x, event.button.y, &logical_x,
                                              &logical_y);
                app->mouse_x = (int)logical_x;
                app->mouse_y = (int)logical_y;
                if (event.button.button == SDL_BUTTON_LEFT) {
                    handle_mouse_press(app, true);
                }
                break;
            }
            case SDL_EVENT_KEY_DOWN:
                if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
                    if (app->menu_mode == MENU_NONE) {
                        app->menu_mode = MENU_MAIN;
                    } else {
                        menu_cancel(app);
                    }
                    break;
                }

                if (menu_is_open(app)) {
                    break;
                }

                switch (event.key.scancode) {
                    case SDL_SCANCODE_O:
                        begin_open_flow(app);
                        break;
                    case SDL_SCANCODE_S:
                        (void)action_save(app);
                        break;
                    case SDL_SCANCODE_N:
                        begin_new_flow(app);
                        break;
                    case SDL_SCANCODE_A:
                        action_add_cell(app);
                        break;
                    case SDL_SCANCODE_DELETE:
                        action_delete_selected(app);
                        break;
                    case SDL_SCANCODE_UP:
                        app->selected_index--;
                        select_clamp(app);
                        break;
                    case SDL_SCANCODE_DOWN:
                        app->selected_index++;
                        select_clamp(app);
                        break;
                    case SDL_SCANCODE_Q:
                        begin_quit_flow(app);
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
}

static void app_render(AppState* app) {
    if (!app || !app->renderer) {
        return;
    }

    SDL_SetRenderDrawColor(app->renderer, 28, 28, 28, 255);
    SDL_RenderClear(app->renderer);

    render_list_panel(app);
    render_details_panel(app);
    render_ui(app);
    render_menu_overlay(app);

    SDL_RenderPresent(app->renderer);
}

int main(void) {
    AppState app;
    if (!app_init(&app)) {
        return 1;
    }

    printf("Asset Composer started\n");
    printf("DB: %s\n", app.assets_path);

    while (app.running) {
        app_handle_events(&app);
        app_render(&app);
        SDL_Delay(16);
    }

    if (app.dirty) {
        printf("Warning: unsaved asset database changes\n");
    }

    app_cleanup(&app);
    return 0;
}
