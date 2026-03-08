#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../shared/text_renderer/text_renderer.h"
#include "../shared/ui_framework/ui_viewport.h"
#include "../tile-maker/palette_io.h"
#include "../tile-maker/tile_specs_io.h"
#include "../tile-maker/tiles_io.h"

#define MAP_COLS 32
#define MAP_ROWS 20
#define CELL_SIZE 32

#define WINDOW_WIDTH 1400
#define WINDOW_HEIGHT 760

#define MAP_ORIGIN_X 10
#define MAP_ORIGIN_Y 52

#define TILE_PANEL_X 1060
#define TILE_PANEL_Y 52
#define TILE_PANEL_COLS 16
#define TILE_PANEL_ROWS 16
#define TILE_PANEL_TILE_SIZE 16
#define TILE_PANEL_TILE_GAP 2

#define STATUS_BAR_HEIGHT 24

typedef struct {
    int material;
    uint16_t entries[16];
} MapCell;

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    TextRenderer text_renderer;
    bool running;

    int mouse_x;
    int mouse_y;
    bool mouse_left_pressed;
    bool mouse_right_pressed;

    int hover_row;
    int hover_col;

    int selected_tile;
    uint8_t brush_health;
    uint8_t brush_destruction;
    uint8_t brush_movement;

    bool dirty;
    char status[160];

    char map_path[260];
    char palette_path[260];
    char tiles_path[260];
} AppState;

static MapCell g_map[MAP_ROWS][MAP_COLS];

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

static uint8_t pack_spec(uint8_t health, uint8_t destruction_mode, uint8_t movement) {
    return (uint8_t)((health & 0x07u) | ((destruction_mode & 0x07u) << 3) |
                     ((movement & 0x03u) << 6));
}

static uint16_t pack_entry(uint8_t tile_id, uint8_t spec) {
    return (uint16_t)(tile_id | ((uint16_t)spec << 8));
}

static void set_status(AppState* app, const char* message) {
    if (!app || !message) {
        return;
    }

    strncpy(app->status, message, sizeof(app->status) - 1);
    app->status[sizeof(app->status) - 1] = '\0';
}

static void fill_cell(int row, int col, uint8_t tile_id, uint8_t health, uint8_t destruction_mode,
                      uint8_t movement) {
    if (row < 0 || row >= MAP_ROWS || col < 0 || col >= MAP_COLS) {
        return;
    }

    MapCell* cell = &g_map[row][col];
    const uint8_t spec = pack_spec(health, destruction_mode, movement);
    const uint16_t entry = pack_entry(tile_id, spec);

    for (int i = 0; i < 16; ++i) {
        cell->entries[i] = entry;
    }

    cell->material = (movement == 1 || health == 0) ? 0 : 1;
}

static void init_default_map(void) {
    for (int row = 0; row < MAP_ROWS; ++row) {
        for (int col = 0; col < MAP_COLS; ++col) {
            const bool border = (row == 0 || col == 0 || row == MAP_ROWS - 1 || col == MAP_COLS - 1);
            if (border) {
                fill_cell(row, col, 1, 1, 1, 0);
            } else {
                fill_cell(row, col, 0, 1, 0, 1);
            }
        }
    }
}

static bool parse_int_token(const char* token, int* out_value) {
    if (!token || token[0] == '\0' || !out_value) {
        return false;
    }

    char* end_ptr = NULL;
    const long value = strtol(token, &end_ptr, 10);
    if (!end_ptr || *end_ptr != '\0') {
        return false;
    }

    *out_value = (int)value;
    return true;
}

static bool parse_u16_token(const char* token, uint16_t* out_value) {
    if (!token || token[0] == '\0' || !out_value) {
        return false;
    }

    char* end_ptr = NULL;
    const unsigned long value = strtoul(token, &end_ptr, 0);
    if (!end_ptr || *end_ptr != '\0' || value > 65535UL) {
        return false;
    }

    *out_value = (uint16_t)value;
    return true;
}

static bool parse_packed_cell_token(const char* token, MapCell* out_cell) {
    if (!token || !out_cell) {
        return false;
    }

    const char* split = strchr(token, '|');
    if (!split) {
        return false;
    }

    const size_t material_len = (size_t)(split - token);
    if (material_len == 0 || material_len > 15) {
        return false;
    }

    char material_buf[16];
    memcpy(material_buf, token, material_len);
    material_buf[material_len] = '\0';

    int material = 0;
    if (!parse_int_token(material_buf, &material)) {
        return false;
    }

    const char* csv = split + 1;
    int entry_index = 0;
    const char* cursor = csv;

    while (*cursor != '\0' && entry_index < 16) {
        const char* comma = strchr(cursor, ',');
        size_t len = comma ? (size_t)(comma - cursor) : strlen(cursor);
        if (len == 0 || len > 15) {
            return false;
        }

        char entry_buf[16];
        memcpy(entry_buf, cursor, len);
        entry_buf[len] = '\0';

        uint16_t entry = 0;
        if (!parse_u16_token(entry_buf, &entry)) {
            return false;
        }

        out_cell->entries[entry_index++] = entry;
        if (!comma) {
            break;
        }
        cursor = comma + 1;
    }

    if (entry_index != 16) {
        return false;
    }

    out_cell->material = material == 0 ? 0 : 1;
    return true;
}

static bool parse_cell_token(const char* token, int row, int col) {
    if (!token || row < 0 || row >= MAP_ROWS || col < 0 || col >= MAP_COLS) {
        return false;
    }

    MapCell parsed = {0};
    if (strchr(token, '|')) {
        if (!parse_packed_cell_token(token, &parsed)) {
            return false;
        }
        g_map[row][col] = parsed;
        return true;
    }

    int legacy = 1;
    if (!parse_int_token(token, &legacy)) {
        legacy = 1;
    }

    if (legacy == 0) {
        fill_cell(row, col, 0, 1, 0, 1);
    } else {
        fill_cell(row, col, 1, 1, 1, 0);
    }
    g_map[row][col].material = legacy == 0 ? 0 : 1;
    return true;
}

static const char* scan_next_token(const char* input, char* out_token, size_t out_size) {
    if (!input || !out_token || out_size == 0) {
        return NULL;
    }

    const char* cursor = input;
    while (*cursor == ' ' || *cursor == '\t' || *cursor == '\r' || *cursor == '\n') {
        ++cursor;
    }

    if (*cursor == '\0') {
        return NULL;
    }

    size_t index = 0;
    while (*cursor != '\0' && *cursor != ' ' && *cursor != '\t' && *cursor != '\r' && *cursor != '\n') {
        if (index + 1 < out_size) {
            out_token[index++] = *cursor;
        }
        ++cursor;
    }
    out_token[index] = '\0';
    return cursor;
}

static bool load_map_file(const char* path) {
    if (!path) {
        return false;
    }

    FILE* file = fopen(path, "rb");
    if (!file) {
        return false;
    }

    init_default_map();

    char line[8192];
    int row = 0;
    while (row < MAP_ROWS && fgets(line, (int)sizeof(line), file)) {
        int col = 0;
        const char* cursor = line;
        while (col < MAP_COLS) {
            char token[4096];
            cursor = scan_next_token(cursor, token, sizeof(token));
            if (!cursor) {
                break;
            }

            if (!parse_cell_token(token, row, col)) {
                fill_cell(row, col, 1, 1, 1, 0);
                g_map[row][col].material = 1;
            }
            ++col;
        }

        while (col < MAP_COLS) {
            fill_cell(row, col, 1, 1, 1, 0);
            g_map[row][col].material = 1;
            ++col;
        }

        ++row;
    }

    while (row < MAP_ROWS) {
        for (int col = 0; col < MAP_COLS; ++col) {
            fill_cell(row, col, 1, 1, 1, 0);
            g_map[row][col].material = 1;
        }
        ++row;
    }

    fclose(file);
    return true;
}

static bool save_map_file(const char* path) {
    if (!path) {
        return false;
    }

    FILE* file = fopen(path, "wb");
    if (!file) {
        return false;
    }

    for (int row = 0; row < MAP_ROWS; ++row) {
        for (int col = 0; col < MAP_COLS; ++col) {
            const MapCell* cell = &g_map[row][col];
            fprintf(file, "%d|", cell->material == 0 ? 0 : 1);
            for (int i = 0; i < 16; ++i) {
                fprintf(file, "0x%04X", (unsigned)cell->entries[i]);
                if (i < 15) {
                    fputc(',', file);
                }
            }
            if (col < MAP_COLS - 1) {
                fputc(' ', file);
            }
        }
        fputc('\n', file);
    }

    fclose(file);
    return true;
}

static void sync_brush_from_selected_tile(AppState* app) {
    if (!app) {
        return;
    }

    const uint8_t spec = tile_spec_get(app->selected_tile);
    app->brush_health = (uint8_t)(spec & 0x07u);
    app->brush_destruction = (uint8_t)((spec >> 3) & 0x07u);
    app->brush_movement = (uint8_t)((spec >> 6) & 0x03u);
}

static bool point_in_rect(int x, int y, int rx, int ry, int rw, int rh) {
    return x >= rx && x < rx + rw && y >= ry && y < ry + rh;
}

static bool hit_test_map(int x, int y, int* out_row, int* out_col) {
    if (!point_in_rect(x, y, MAP_ORIGIN_X, MAP_ORIGIN_Y, MAP_COLS * CELL_SIZE, MAP_ROWS * CELL_SIZE)) {
        return false;
    }

    int col = (x - MAP_ORIGIN_X) / CELL_SIZE;
    int row = (y - MAP_ORIGIN_Y) / CELL_SIZE;
    if (row < 0 || row >= MAP_ROWS || col < 0 || col >= MAP_COLS) {
        return false;
    }

    if (out_row) {
        *out_row = row;
    }
    if (out_col) {
        *out_col = col;
    }
    return true;
}

static bool hit_test_tile_panel(int x, int y, int* out_tile_id) {
    const int panel_w = TILE_PANEL_COLS * TILE_PANEL_TILE_SIZE +
                        (TILE_PANEL_COLS - 1) * TILE_PANEL_TILE_GAP;
    const int panel_h = TILE_PANEL_ROWS * TILE_PANEL_TILE_SIZE +
                        (TILE_PANEL_ROWS - 1) * TILE_PANEL_TILE_GAP;

    if (!point_in_rect(x, y, TILE_PANEL_X, TILE_PANEL_Y, panel_w, panel_h)) {
        return false;
    }

    const int cell_w = TILE_PANEL_TILE_SIZE + TILE_PANEL_TILE_GAP;
    const int cell_h = TILE_PANEL_TILE_SIZE + TILE_PANEL_TILE_GAP;
    const int local_x = x - TILE_PANEL_X;
    const int local_y = y - TILE_PANEL_Y;
    const int col = local_x / cell_w;
    const int row = local_y / cell_h;

    if (col < 0 || col >= TILE_PANEL_COLS || row < 0 || row >= TILE_PANEL_ROWS) {
        return false;
    }

    const int in_cell_x = local_x % cell_w;
    const int in_cell_y = local_y % cell_h;
    if (in_cell_x >= TILE_PANEL_TILE_SIZE || in_cell_y >= TILE_PANEL_TILE_SIZE) {
        return false;
    }

    const int tile_id = row * TILE_PANEL_COLS + col;
    if (tile_id < 0 || tile_id >= TILE_COUNT) {
        return false;
    }

    if (out_tile_id) {
        *out_tile_id = tile_id;
    }
    return true;
}

static void paint_at(AppState* app, int row, int col) {
    if (!app) {
        return;
    }

    fill_cell(row, col, (uint8_t)app->selected_tile, app->brush_health, app->brush_destruction,
              app->brush_movement);
    app->dirty = true;
}

static void pick_from(AppState* app, int row, int col) {
    if (!app || row < 0 || row >= MAP_ROWS || col < 0 || col >= MAP_COLS) {
        return;
    }

    const uint16_t entry = g_map[row][col].entries[0];
    const uint8_t tile_id = (uint8_t)(entry & 0xFFu);
    const uint8_t spec = (uint8_t)((entry >> 8) & 0xFFu);
    app->selected_tile = tile_id;
    app->brush_health = (uint8_t)(spec & 0x07u);
    app->brush_destruction = (uint8_t)((spec >> 3) & 0x07u);
    app->brush_movement = (uint8_t)((spec >> 6) & 0x03u);
}

static void render_tile(SDL_Renderer* renderer, int tile_id, int x, int y, int size) {
    if (!renderer || tile_id < 0 || tile_id >= TILE_COUNT || size <= 0) {
        return;
    }

    int scale = size / 8;
    if (scale <= 0) {
        scale = 1;
    }

    for (int py = 0; py < 8; ++py) {
        for (int px = 0; px < 8; ++px) {
            const uint8_t palette_index = get_px(tile_id, px, py);
            const SDL_Color color = palette_get_sdl_color((int)palette_index);
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            SDL_FRect rect = {(float)(x + px * scale), (float)(y + py * scale), (float)scale, (float)scale};
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

static void render_map(AppState* app) {
    if (!app || !app->renderer) {
        return;
    }

    SDL_SetRenderDrawColor(app->renderer, 36, 36, 36, 255);
    SDL_FRect bg = {(float)MAP_ORIGIN_X, (float)MAP_ORIGIN_Y, (float)(MAP_COLS * CELL_SIZE),
                    (float)(MAP_ROWS * CELL_SIZE)};
    SDL_RenderFillRect(app->renderer, &bg);

    for (int row = 0; row < MAP_ROWS; ++row) {
        for (int col = 0; col < MAP_COLS; ++col) {
            const int x = MAP_ORIGIN_X + col * CELL_SIZE;
            const int y = MAP_ORIGIN_Y + row * CELL_SIZE;
            const uint16_t entry = g_map[row][col].entries[0];
            const int tile_id = (int)(entry & 0xFFu);
            const uint8_t spec = (uint8_t)((entry >> 8) & 0xFFu);
            const uint8_t health = (uint8_t)(spec & 0x07u);

            render_tile(app->renderer, tile_id, x, y, CELL_SIZE);

            if (health == 0) {
                SDL_SetRenderDrawColor(app->renderer, 12, 12, 12, 150);
                SDL_FRect fade = {(float)x, (float)y, (float)CELL_SIZE, (float)CELL_SIZE};
                SDL_RenderFillRect(app->renderer, &fade);
            }

            SDL_SetRenderDrawColor(app->renderer, 80, 80, 80, 255);
            SDL_FRect border = {(float)x, (float)y, (float)CELL_SIZE, (float)CELL_SIZE};
            SDL_RenderRect(app->renderer, &border);
        }
    }

    if (app->hover_row >= 0 && app->hover_col >= 0) {
        SDL_SetRenderDrawColor(app->renderer, 255, 230, 70, 255);
        SDL_FRect hover = {(float)(MAP_ORIGIN_X + app->hover_col * CELL_SIZE),
                           (float)(MAP_ORIGIN_Y + app->hover_row * CELL_SIZE), (float)CELL_SIZE,
                           (float)CELL_SIZE};
        SDL_RenderRect(app->renderer, &hover);
    }
}

static void render_tile_panel(AppState* app) {
    if (!app || !app->renderer) {
        return;
    }

    const int panel_w = TILE_PANEL_COLS * TILE_PANEL_TILE_SIZE +
                        (TILE_PANEL_COLS - 1) * TILE_PANEL_TILE_GAP;
    const int panel_h = TILE_PANEL_ROWS * TILE_PANEL_TILE_SIZE +
                        (TILE_PANEL_ROWS - 1) * TILE_PANEL_TILE_GAP;

    SDL_FRect panel = {(float)(TILE_PANEL_X - 6), (float)(TILE_PANEL_Y - 6), (float)(panel_w + 12),
                       (float)(panel_h + 12)};
    SDL_SetRenderDrawColor(app->renderer, 42, 42, 42, 255);
    SDL_RenderFillRect(app->renderer, &panel);
    SDL_SetRenderDrawColor(app->renderer, 108, 108, 108, 255);
    SDL_RenderRect(app->renderer, &panel);

    for (int tile_id = 0; tile_id < TILE_COUNT; ++tile_id) {
        const int row = tile_id / TILE_PANEL_COLS;
        const int col = tile_id % TILE_PANEL_COLS;
        const int x = TILE_PANEL_X + col * (TILE_PANEL_TILE_SIZE + TILE_PANEL_TILE_GAP);
        const int y = TILE_PANEL_Y + row * (TILE_PANEL_TILE_SIZE + TILE_PANEL_TILE_GAP);
        render_tile(app->renderer, tile_id, x, y, TILE_PANEL_TILE_SIZE);

        if (tile_id == app->selected_tile) {
            SDL_SetRenderDrawColor(app->renderer, 90, 210, 255, 255);
            SDL_FRect highlight = {(float)x, (float)y, (float)TILE_PANEL_TILE_SIZE,
                                   (float)TILE_PANEL_TILE_SIZE};
            SDL_RenderRect(app->renderer, &highlight);
        }
    }
}

static void render_text_line(AppState* app, const char* text, int x, int y, SDL_Color color) {
    if (!app || !text) {
        return;
    }
    text_render_string(&app->text_renderer, text, x, y, color);
}

static void render_ui(AppState* app) {
    if (!app || !app->renderer) {
        return;
    }

    char line1[128];
    snprintf(line1, sizeof(line1), "Tile:%03d  H:%u D:%u M:%u", app->selected_tile,
             (unsigned)app->brush_health, (unsigned)app->brush_destruction,
             (unsigned)app->brush_movement);
    render_text_line(app, line1, TILE_PANEL_X, TILE_PANEL_Y + 300, (SDL_Color){235, 235, 235, 255});

    render_text_line(
        app, "LMB paint, RMB pick, S save, L load, C reset", TILE_PANEL_X, TILE_PANEL_Y + 316,
        (SDL_Color){180, 180, 180, 255});
    render_text_line(
        app, "[ ] health, , . destruct, ; ' movement", TILE_PANEL_X, TILE_PANEL_Y + 332,
        (SDL_Color){180, 180, 180, 255});

    SDL_FRect status_bar = {0.0f, (float)(WINDOW_HEIGHT - STATUS_BAR_HEIGHT), (float)WINDOW_WIDTH,
                            (float)STATUS_BAR_HEIGHT};
    SDL_SetRenderDrawColor(app->renderer, 22, 22, 22, 255);
    SDL_RenderFillRect(app->renderer, &status_bar);
    SDL_SetRenderDrawColor(app->renderer, 70, 70, 70, 255);
    SDL_RenderRect(app->renderer, &status_bar);
    render_text_line(app, app->status, 8, WINDOW_HEIGHT - 18, (SDL_Color){245, 245, 245, 255});
}

static void handle_mouse_press(AppState* app, bool left_button) {
    if (!app) {
        return;
    }

    int tile_id = -1;
    if (hit_test_tile_panel(app->mouse_x, app->mouse_y, &tile_id)) {
        if (left_button) {
            app->selected_tile = tile_id;
            sync_brush_from_selected_tile(app);
        }
        return;
    }

    int row = -1;
    int col = -1;
    if (hit_test_map(app->mouse_x, app->mouse_y, &row, &col)) {
        if (left_button) {
            paint_at(app, row, col);
        } else {
            pick_from(app, row, col);
        }
    }
}

static bool app_init(AppState* app) {
    if (!app) {
        return false;
    }

    memset(app, 0, sizeof(*app));
    app->hover_row = -1;
    app->hover_col = -1;
    app->selected_tile = 0;
    app->brush_health = 1;
    app->brush_destruction = 1;
    app->brush_movement = 0;
    app->running = true;
    set_status(app, "Map Maker ready");

    const char* map_candidates[] = {"src/game.map", "../src/game.map", "../../src/game.map"};
    const char* palette_candidates[] = {"palette.dat", "../palette.dat", "../../palette.dat"};
    const char* tiles_candidates[] = {"tiles.dat", "../tiles.dat", "../../tiles.dat"};
    resolve_path(app->map_path, sizeof(app->map_path), map_candidates, 3);
    resolve_path(app->palette_path, sizeof(app->palette_path), palette_candidates, 3);
    resolve_path(app->tiles_path, sizeof(app->tiles_path), tiles_candidates, 3);

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        printf("SDL init failed: %s\n", SDL_GetError());
        return false;
    }

    app->window = SDL_CreateWindow("Map Maker v0.1", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
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

    palette_init();
    if (!palette_load(app->palette_path)) {
        printf("Palette load failed: %s (using defaults)\n", app->palette_path);
    }

    tiles_init();
    tile_specs_init();
    if (!tiles_load(app->tiles_path)) {
        printf("Tiles load failed: %s (using defaults)\n", app->tiles_path);
    }

    sync_brush_from_selected_tile(app);
    if (load_map_file(app->map_path)) {
        set_status(app, "Map loaded");
    } else {
        init_default_map();
        set_status(app, "Map file not found, using defaults");
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
                app->running = false;
                break;
            case SDL_EVENT_MOUSE_MOTION: {
                float logical_x = event.motion.x;
                float logical_y = event.motion.y;
                ui_viewport_window_to_logical(app->renderer, event.motion.x, event.motion.y, &logical_x,
                                              &logical_y);
                app->mouse_x = (int)logical_x;
                app->mouse_y = (int)logical_y;
                if (app->mouse_left_pressed) {
                    int row = -1;
                    int col = -1;
                    if (hit_test_map(app->mouse_x, app->mouse_y, &row, &col)) {
                        paint_at(app, row, col);
                    }
                }
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
                    app->mouse_left_pressed = true;
                    handle_mouse_press(app, true);
                } else if (event.button.button == SDL_BUTTON_RIGHT) {
                    app->mouse_right_pressed = true;
                    handle_mouse_press(app, false);
                }
                break;
            }
            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    app->mouse_left_pressed = false;
                } else if (event.button.button == SDL_BUTTON_RIGHT) {
                    app->mouse_right_pressed = false;
                }
                break;
            case SDL_EVENT_KEY_DOWN:
                switch (event.key.scancode) {
                    case SDL_SCANCODE_ESCAPE:
                        app->running = false;
                        break;
                    case SDL_SCANCODE_S:
                        if (save_map_file(app->map_path)) {
                            app->dirty = false;
                            set_status(app, "Map saved");
                        } else {
                            set_status(app, "Failed to save map");
                        }
                        break;
                    case SDL_SCANCODE_L:
                        if (load_map_file(app->map_path)) {
                            app->dirty = false;
                            set_status(app, "Map loaded");
                        } else {
                            set_status(app, "Failed to load map");
                        }
                        break;
                    case SDL_SCANCODE_C:
                        init_default_map();
                        app->dirty = true;
                        set_status(app, "Map reset to defaults");
                        break;
                    case SDL_SCANCODE_LEFTBRACKET:
                        if (app->brush_health > 0) {
                            app->brush_health--;
                        }
                        break;
                    case SDL_SCANCODE_RIGHTBRACKET:
                        if (app->brush_health < 7) {
                            app->brush_health++;
                        }
                        break;
                    case SDL_SCANCODE_COMMA:
                        if (app->brush_destruction > 0) {
                            app->brush_destruction--;
                        }
                        break;
                    case SDL_SCANCODE_PERIOD:
                        if (app->brush_destruction < 7) {
                            app->brush_destruction++;
                        }
                        break;
                    case SDL_SCANCODE_SEMICOLON:
                        if (app->brush_movement > 0) {
                            app->brush_movement--;
                        }
                        break;
                    case SDL_SCANCODE_APOSTROPHE:
                        if (app->brush_movement < 3) {
                            app->brush_movement++;
                        }
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

static void app_update_hover(AppState* app) {
    if (!app) {
        return;
    }

    int row = -1;
    int col = -1;
    if (hit_test_map(app->mouse_x, app->mouse_y, &row, &col)) {
        app->hover_row = row;
        app->hover_col = col;
    } else {
        app->hover_row = -1;
        app->hover_col = -1;
    }
}

static void app_render(AppState* app) {
    if (!app || !app->renderer) {
        return;
    }

    SDL_SetRenderDrawColor(app->renderer, 28, 28, 28, 255);
    SDL_RenderClear(app->renderer);

    render_map(app);
    render_tile_panel(app);
    render_ui(app);

    SDL_RenderPresent(app->renderer);
}

int main(void) {
    AppState app;
    if (!app_init(&app)) {
        return 1;
    }

    printf("Map Maker started\n");
    printf("Map: %s\n", app.map_path);
    printf("Tiles: %s\n", app.tiles_path);
    printf("Palette: %s\n", app.palette_path);

    while (app.running) {
        app_handle_events(&app);
        app_update_hover(&app);
        app_render(&app);
        SDL_Delay(16);
    }

    if (app.dirty) {
        printf("Warning: unsaved map changes\n");
    }

    app_cleanup(&app);
    return 0;
}
