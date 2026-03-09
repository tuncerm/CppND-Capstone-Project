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

#define MAP_ORIGIN_X 360
#define MAP_ORIGIN_Y 52

#define TILE_PANEL_X 16
#define TILE_PANEL_Y 52
#define TILE_PANEL_COLS 16
#define TILE_PANEL_ROWS 16
#define TILE_PANEL_TILE_SIZE 16
#define TILE_PANEL_TILE_GAP 2

#define STATUS_BAR_HEIGHT 24
#define MAP_META_MAGIC "MMD1"
#define MAP_META_VERSION 1
#define MAP_META_HEADER_SIZE 64
#define SOURCE_TILE_GRID_SIZE 16
#define MAP_SIZE_BUTTON_Y (TILE_PANEL_Y + 418)
#define MAP_SIZE_BUTTON_W 30
#define MAP_SIZE_BUTTON_H 22
#define MAP_SIZE_BUTTON_GAP 4
#define SPAWN_BUTTON_Y (TILE_PANEL_Y + 452)
#define SPAWN_BUTTON_W 42
#define SPAWN_BUTTON_H 22
#define SPAWN_BUTTON_GAP 6
#define SPAWN_MOVE_Y (SPAWN_BUTTON_Y + SPAWN_BUTTON_H + 10)
#define SPAWN_MOVE_W 30
#define SPAWN_MOVE_H 22
#define MENU_BUTTON_X 10
#define MENU_BUTTON_Y 14
#define MENU_BUTTON_W 90
#define MENU_BUTTON_H 26
#define MENU_DIALOG_W 320
#define MENU_OPTION_W 220
#define MENU_OPTION_H 30
#define MENU_OPTION_GAP 10

typedef struct {
    int material;
    uint16_t entries[16];
} MapCell;

typedef struct {
    uint8_t x;
    uint8_t y;
} GridPoint;

typedef struct {
    bool exists;
    uint8_t x;
    uint8_t y;
    uint8_t w;
    uint8_t h;
    uint8_t tile_x;
    uint8_t tile_y;
} StructureRect;

typedef struct {
    uint8_t map_cols;
    uint8_t map_rows;
    GridPoint player_spawns[2];
    GridPoint enemy_spawns[3];
    StructureRect player_base;
    StructureRect enemy_base;
    uint8_t enemy_count;
    bool enemy_base_produces_extra;
} MapMetadata;

typedef enum {
    BASE_EDIT_PLAYER = 0,
    BASE_EDIT_ENEMY = 1,
} BaseEditTarget;

typedef enum {
    APP_MENU_NONE = 0,
    APP_MENU_STARTUP = 1,
    APP_MENU_MAIN = 2,
} AppMenuMode;

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

    MapMetadata meta;
    BaseEditTarget active_base;
    int active_spawn_slot;  // 0..4 => P1,P2,E1,E2,E3
    int pending_spawn_slot; // -1 none, 0..4 waiting for map click
    AppMenuMode menu_mode;
} AppState;

static MapCell g_map[MAP_ROWS][MAP_COLS];
static void render_text_line(AppState* app, const char* text, int x, int y, SDL_Color color);
static bool point_in_frect(int x, int y, const SDL_FRect* rect);

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

static uint8_t clamp_u8(uint8_t value, uint8_t min_v, uint8_t max_v) {
    if (value < min_v) {
        return min_v;
    }
    if (value > max_v) {
        return max_v;
    }
    return value;
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
            fill_cell(row, col, 0, 1, 0, 1);
        }
    }
}

static void metadata_init_defaults(MapMetadata* meta) {
    if (!meta) {
        return;
    }

    memset(meta, 0, sizeof(*meta));
    meta->map_cols = MAP_COLS;
    meta->map_rows = MAP_ROWS;
    meta->player_spawns[0].x = 2;
    meta->player_spawns[0].y = MAP_ROWS - 2;
    meta->player_spawns[1].x = 4;
    meta->player_spawns[1].y = MAP_ROWS - 2;

    meta->enemy_spawns[0].x = MAP_COLS / 2;
    meta->enemy_spawns[0].y = 1;
    meta->enemy_spawns[1].x = (MAP_COLS / 2) - 4;
    meta->enemy_spawns[1].y = 1;
    meta->enemy_spawns[2].x = (MAP_COLS / 2) + 4;
    meta->enemy_spawns[2].y = 1;

    meta->player_base.exists = false;
    meta->player_base.x = 1;
    meta->player_base.y = MAP_ROWS - 4;
    meta->player_base.w = 3;
    meta->player_base.h = 2;
    meta->player_base.tile_x = 0;
    meta->player_base.tile_y = 0;

    meta->enemy_base.exists = false;
    meta->enemy_base.x = MAP_COLS - 4;
    meta->enemy_base.y = 1;
    meta->enemy_base.w = 3;
    meta->enemy_base.h = 2;
    meta->enemy_base.tile_x = 0;
    meta->enemy_base.tile_y = 0;

    meta->enemy_count = 8;
    meta->enemy_base_produces_extra = false;
}

static void structure_clamp_to_map(StructureRect* structure, int max_cols, int max_rows) {
    if (!structure) {
        return;
    }
    if (max_cols < 1) {
        max_cols = 1;
    }
    if (max_rows < 1) {
        max_rows = 1;
    }

    structure->w = clamp_u8(structure->w, 1, (uint8_t)max_cols);
    structure->h = clamp_u8(structure->h, 1, (uint8_t)max_rows);
    structure->x = clamp_u8(structure->x, 0, (uint8_t)(max_cols - 1));
    structure->y = clamp_u8(structure->y, 0, (uint8_t)(max_rows - 1));

    if ((int)structure->x + (int)structure->w > max_cols) {
        structure->x = (uint8_t)(max_cols - structure->w);
    }
    if ((int)structure->y + (int)structure->h > max_rows) {
        structure->y = (uint8_t)(max_rows - structure->h);
    }

    structure->tile_x = clamp_u8(structure->tile_x, 0, SOURCE_TILE_GRID_SIZE - 1);
    structure->tile_y = clamp_u8(structure->tile_y, 0, SOURCE_TILE_GRID_SIZE - 1);
}

static void metadata_clamp(MapMetadata* meta) {
    if (!meta) {
        return;
    }

    meta->map_cols = clamp_u8(meta->map_cols, 1, MAP_COLS);
    meta->map_rows = clamp_u8(meta->map_rows, 1, MAP_ROWS);
    const int max_cols = (int)meta->map_cols;
    const int max_rows = (int)meta->map_rows;

    for (int i = 0; i < 2; ++i) {
        meta->player_spawns[i].x = clamp_u8(meta->player_spawns[i].x, 0, (uint8_t)(max_cols - 1));
        meta->player_spawns[i].y = clamp_u8(meta->player_spawns[i].y, 0, (uint8_t)(max_rows - 1));
    }
    for (int i = 0; i < 3; ++i) {
        meta->enemy_spawns[i].x = clamp_u8(meta->enemy_spawns[i].x, 0, (uint8_t)(max_cols - 1));
        meta->enemy_spawns[i].y = clamp_u8(meta->enemy_spawns[i].y, 0, (uint8_t)(max_rows - 1));
    }

    structure_clamp_to_map(&meta->player_base, max_cols, max_rows);
    structure_clamp_to_map(&meta->enemy_base, max_cols, max_rows);
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

static bool load_map_stream(FILE* file) {
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

    return true;
}

static bool save_map_stream(FILE* file) {
    if (!file) {
        return false;
    }

    for (int row = 0; row < MAP_ROWS; ++row) {
        for (int col = 0; col < MAP_COLS; ++col) {
            const MapCell* cell = &g_map[row][col];
            if (fprintf(file, "%d|", cell->material == 0 ? 0 : 1) < 0) {
                return false;
            }
            for (int i = 0; i < 16; ++i) {
                if (fprintf(file, "0x%04X", (unsigned)cell->entries[i]) < 0) {
                    return false;
                }
                if (i < 15 && fputc(',', file) == EOF) {
                    return false;
                }
            }
            if (col < MAP_COLS - 1 && fputc(' ', file) == EOF) {
                return false;
            }
        }
        if (fputc('\n', file) == EOF) {
            return false;
        }
    }

    return true;
}

static bool metadata_write_header(FILE* file, const MapMetadata* meta) {
    if (!file || !meta) {
        return false;
    }

    uint8_t header[MAP_META_HEADER_SIZE] = {0};
    header[0] = 'M';
    header[1] = 'M';
    header[2] = 'D';
    header[3] = '1';
    header[4] = (uint8_t)(MAP_META_VERSION & 0xFFu);
    header[5] = (uint8_t)((MAP_META_VERSION >> 8) & 0xFFu);
    header[6] = meta->map_cols;
    header[7] = meta->map_rows;
    header[8] = meta->enemy_count;
    header[9] = meta->enemy_base_produces_extra ? 1u : 0u;

    header[10] = meta->player_spawns[0].x;
    header[11] = meta->player_spawns[0].y;
    header[12] = meta->player_spawns[1].x;
    header[13] = meta->player_spawns[1].y;

    header[14] = meta->enemy_spawns[0].x;
    header[15] = meta->enemy_spawns[0].y;
    header[16] = meta->enemy_spawns[1].x;
    header[17] = meta->enemy_spawns[1].y;
    header[18] = meta->enemy_spawns[2].x;
    header[19] = meta->enemy_spawns[2].y;

    header[20] = meta->player_base.exists ? 1u : 0u;
    header[21] = meta->player_base.x;
    header[22] = meta->player_base.y;
    header[23] = meta->player_base.w;
    header[24] = meta->player_base.h;
    header[25] = meta->player_base.tile_x;
    header[26] = meta->player_base.tile_y;

    header[27] = meta->enemy_base.exists ? 1u : 0u;
    header[28] = meta->enemy_base.x;
    header[29] = meta->enemy_base.y;
    header[30] = meta->enemy_base.w;
    header[31] = meta->enemy_base.h;
    header[32] = meta->enemy_base.tile_x;
    header[33] = meta->enemy_base.tile_y;

    return fwrite(header, 1, sizeof(header), file) == sizeof(header);
}

static bool metadata_read_header(FILE* file, MapMetadata* out_meta) {
    if (!file || !out_meta) {
        return false;
    }

    uint8_t header[MAP_META_HEADER_SIZE] = {0};
    if (fread(header, 1, sizeof(header), file) != sizeof(header)) {
        return false;
    }

    if (!(header[0] == 'M' && header[1] == 'M' && header[2] == 'D' && header[3] == '1')) {
        return false;
    }

    const uint16_t version = (uint16_t)(header[4] | ((uint16_t)header[5] << 8));
    if (version != MAP_META_VERSION) {
        return false;
    }
    if (header[6] == 0 || header[6] > MAP_COLS || header[7] == 0 || header[7] > MAP_ROWS) {
        return false;
    }

    MapMetadata loaded;
    metadata_init_defaults(&loaded);
    loaded.map_cols = header[6];
    loaded.map_rows = header[7];

    loaded.enemy_count = header[8];
    loaded.enemy_base_produces_extra = header[9] != 0;

    loaded.player_spawns[0].x = header[10];
    loaded.player_spawns[0].y = header[11];
    loaded.player_spawns[1].x = header[12];
    loaded.player_spawns[1].y = header[13];

    loaded.enemy_spawns[0].x = header[14];
    loaded.enemy_spawns[0].y = header[15];
    loaded.enemy_spawns[1].x = header[16];
    loaded.enemy_spawns[1].y = header[17];
    loaded.enemy_spawns[2].x = header[18];
    loaded.enemy_spawns[2].y = header[19];

    loaded.player_base.exists = header[20] != 0;
    loaded.player_base.x = header[21];
    loaded.player_base.y = header[22];
    loaded.player_base.w = header[23];
    loaded.player_base.h = header[24];
    loaded.player_base.tile_x = header[25];
    loaded.player_base.tile_y = header[26];

    loaded.enemy_base.exists = header[27] != 0;
    loaded.enemy_base.x = header[28];
    loaded.enemy_base.y = header[29];
    loaded.enemy_base.w = header[30];
    loaded.enemy_base.h = header[31];
    loaded.enemy_base.tile_x = header[32];
    loaded.enemy_base.tile_y = header[33];

    metadata_clamp(&loaded);
    *out_meta = loaded;
    return true;
}

static bool load_map_file(const char* path, MapMetadata* out_meta) {
    if (!path) {
        return false;
    }

    FILE* file = fopen(path, "rb");
    if (!file) {
        return false;
    }

    uint8_t magic[4] = {0};
    size_t magic_read = fread(magic, 1, sizeof(magic), file);
    if (magic_read != sizeof(magic)) {
        fclose(file);
        return false;
    }

    bool has_header = magic[0] == 'M' && magic[1] == 'M' && magic[2] == 'D' && magic[3] == '1';
    bool ok = false;
    if (has_header) {
        if (fseek(file, 0, SEEK_SET) != 0 || !metadata_read_header(file, out_meta) ||
            fseek(file, MAP_META_HEADER_SIZE, SEEK_SET) != 0) {
            fclose(file);
            return false;
        }
        ok = load_map_stream(file);
    } else {
        if (out_meta) {
            metadata_init_defaults(out_meta);
        }
        if (fseek(file, 0, SEEK_SET) != 0) {
            fclose(file);
            return false;
        }
        ok = load_map_stream(file);
    }

    fclose(file);
    return ok;
}

static bool save_map_file(const char* path, const MapMetadata* meta) {
    if (!path || !meta) {
        return false;
    }

    FILE* file = fopen(path, "wb");
    if (!file) {
        return false;
    }

    bool ok = metadata_write_header(file, meta) &&
              fseek(file, MAP_META_HEADER_SIZE, SEEK_SET) == 0 && save_map_stream(file);
    fclose(file);
    return ok;
}

static void app_new_map_action(AppState* app) {
    if (!app) {
        return;
    }

    init_default_map();
    metadata_init_defaults(&app->meta);
    metadata_clamp(&app->meta);
    app->pending_spawn_slot = -1;
    app->active_spawn_slot = -1;
    app->dirty = true;
    set_status(app, "New map created (unsaved)");
}

static void app_load_map_action(AppState* app) {
    if (!app) {
        return;
    }

    bool map_loaded = load_map_file(app->map_path, &app->meta);
    if (!map_loaded) {
        init_default_map();
        metadata_init_defaults(&app->meta);
    }
    metadata_clamp(&app->meta);
    app->pending_spawn_slot = -1;
    app->active_spawn_slot = -1;
    app->dirty = false;
    if (map_loaded) {
        set_status(app, "Map loaded (header+payload)");
    } else {
        set_status(app, "Open failed, defaults loaded");
    }
}

static void app_save_map_action(AppState* app) {
    if (!app) {
        return;
    }

    metadata_clamp(&app->meta);
    bool map_saved = save_map_file(app->map_path, &app->meta);
    if (map_saved) {
        app->dirty = false;
        set_status(app, "Map saved (64-byte header + payload)");
    } else {
        set_status(app, "Save failed");
    }
}

static bool app_menu_is_open(const AppState* app) {
    return app && app->menu_mode != APP_MENU_NONE;
}

static int app_menu_option_count(AppMenuMode mode) {
    if (mode == APP_MENU_STARTUP) {
        return 2;
    }
    if (mode == APP_MENU_MAIN) {
        return 4;
    }
    return 0;
}

static const char* app_menu_option_label(AppMenuMode mode, int index) {
    static const char* startup_options[] = {"Open", "New"};
    static const char* main_options[] = {"Load", "Save", "New", "Resume"};
    if (mode == APP_MENU_STARTUP) {
        return (index >= 0 && index < 2) ? startup_options[index] : "";
    }
    if (mode == APP_MENU_MAIN) {
        return (index >= 0 && index < 4) ? main_options[index] : "";
    }
    return "";
}

static SDL_FRect menu_button_rect(void) {
    return (SDL_FRect){(float)MENU_BUTTON_X, (float)MENU_BUTTON_Y, (float)MENU_BUTTON_W,
                       (float)MENU_BUTTON_H};
}

static SDL_FRect menu_dialog_rect(AppMenuMode mode) {
    const int option_count = app_menu_option_count(mode);
    const int content_h = option_count > 0 ? option_count * MENU_OPTION_H + (option_count - 1) * MENU_OPTION_GAP : 0;
    const int dialog_h = 64 + content_h + 20;
    const float x = (float)((WINDOW_WIDTH - MENU_DIALOG_W) / 2);
    const float y = (float)((WINDOW_HEIGHT - dialog_h) / 2);
    return (SDL_FRect){x, y, (float)MENU_DIALOG_W, (float)dialog_h};
}

static SDL_FRect menu_option_rect(AppMenuMode mode, int index) {
    SDL_FRect dialog = menu_dialog_rect(mode);
    const float x = dialog.x + (dialog.w - (float)MENU_OPTION_W) * 0.5f;
    const float y = dialog.y + 50.0f + (float)index * (float)(MENU_OPTION_H + MENU_OPTION_GAP);
    return (SDL_FRect){x, y, (float)MENU_OPTION_W, (float)MENU_OPTION_H};
}

static bool handle_menu_click(AppState* app, int x, int y) {
    if (!app) {
        return false;
    }

    SDL_FRect menu_button = menu_button_rect();
    if (!app_menu_is_open(app) && point_in_frect(x, y, &menu_button)) {
        app->menu_mode = APP_MENU_MAIN;
        return true;
    }

    if (!app_menu_is_open(app)) {
        return false;
    }

    SDL_FRect dialog = menu_dialog_rect(app->menu_mode);
    if (!point_in_frect(x, y, &dialog)) {
        if (app->menu_mode == APP_MENU_MAIN) {
            app->menu_mode = APP_MENU_NONE;
        }
        return true;
    }

    const int option_count = app_menu_option_count(app->menu_mode);
    for (int i = 0; i < option_count; ++i) {
        SDL_FRect option = menu_option_rect(app->menu_mode, i);
        if (!point_in_frect(x, y, &option)) {
            continue;
        }

        if (app->menu_mode == APP_MENU_STARTUP) {
            if (i == 0) {
                app_load_map_action(app);
            } else if (i == 1) {
                app_new_map_action(app);
            }
            app->menu_mode = APP_MENU_NONE;
            return true;
        }

        if (app->menu_mode == APP_MENU_MAIN) {
            if (i == 0) {
                app_load_map_action(app);
            } else if (i == 1) {
                app_save_map_action(app);
            } else if (i == 2) {
                app_new_map_action(app);
            }
            app->menu_mode = APP_MENU_NONE;
            return true;
        }
    }

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

static StructureRect* app_get_active_structure(AppState* app) {
    if (!app) {
        return NULL;
    }
    return app->active_base == BASE_EDIT_ENEMY ? &app->meta.enemy_base : &app->meta.player_base;
}

static bool app_hover_cell(const AppState* app, int* out_row, int* out_col) {
    if (!app || app->hover_row < 0 || app->hover_col < 0) {
        return false;
    }
    if (out_row) {
        *out_row = app->hover_row;
    }
    if (out_col) {
        *out_col = app->hover_col;
    }
    return true;
}

static bool app_cell_enabled(const AppState* app, int row, int col) {
    if (!app || row < 0 || col < 0 || row >= MAP_ROWS || col >= MAP_COLS) {
        return false;
    }
    return row < (int)app->meta.map_rows && col < (int)app->meta.map_cols;
}

static void app_set_spawn_at_cell(AppState* app, int spawn_slot, int row, int col) {
    if (!app || !app_cell_enabled(app, row, col)) {
        return;
    }

    if (spawn_slot >= 0 && spawn_slot < 2) {
        app->meta.player_spawns[spawn_slot].x = (uint8_t)col;
        app->meta.player_spawns[spawn_slot].y = (uint8_t)row;
    } else if (spawn_slot >= 2 && spawn_slot < 5) {
        int enemy_slot = spawn_slot - 2;
        app->meta.enemy_spawns[enemy_slot].x = (uint8_t)col;
        app->meta.enemy_spawns[enemy_slot].y = (uint8_t)row;
    } else {
        return;
    }

    app->dirty = true;
}

static void app_set_spawn_at_hover(AppState* app, int spawn_slot) {
    if (!app) {
        return;
    }

    int row = 0;
    int col = 0;
    if (!app_hover_cell(app, &row, &col)) {
        return;
    }

    app_set_spawn_at_cell(app, spawn_slot, row, col);
}

static GridPoint* app_get_spawn_slot(AppState* app, int spawn_slot) {
    if (!app || spawn_slot < 0 || spawn_slot > 4) {
        return NULL;
    }

    if (spawn_slot < 2) {
        return &app->meta.player_spawns[spawn_slot];
    }

    return &app->meta.enemy_spawns[spawn_slot - 2];
}

static void app_move_active_spawn(AppState* app, int dx, int dy) {
    if (!app) {
        return;
    }

    GridPoint* spawn = app_get_spawn_slot(app, app->active_spawn_slot);
    if (!spawn) {
        return;
    }

    int next_x = (int)spawn->x + dx;
    int next_y = (int)spawn->y + dy;
    const int max_cols = (int)app->meta.map_cols;
    const int max_rows = (int)app->meta.map_rows;
    if (next_x < 0) {
        next_x = 0;
    } else if (next_x >= max_cols) {
        next_x = max_cols - 1;
    }
    if (next_y < 0) {
        next_y = 0;
    } else if (next_y >= max_rows) {
        next_y = max_rows - 1;
    }

    if ((int)spawn->x != next_x || (int)spawn->y != next_y) {
        spawn->x = (uint8_t)next_x;
        spawn->y = (uint8_t)next_y;
        app->dirty = true;
    }
}

static SDL_FRect map_cols_minus_rect(void) {
    return (SDL_FRect){(float)TILE_PANEL_X, (float)MAP_SIZE_BUTTON_Y, (float)MAP_SIZE_BUTTON_W,
                       (float)MAP_SIZE_BUTTON_H};
}

static SDL_FRect map_cols_plus_rect(void) {
    return (SDL_FRect){(float)(TILE_PANEL_X + MAP_SIZE_BUTTON_W + MAP_SIZE_BUTTON_GAP),
                       (float)MAP_SIZE_BUTTON_Y, (float)MAP_SIZE_BUTTON_W, (float)MAP_SIZE_BUTTON_H};
}

static SDL_FRect map_rows_minus_rect(void) {
    return (SDL_FRect){(float)(TILE_PANEL_X + 126), (float)MAP_SIZE_BUTTON_Y,
                       (float)MAP_SIZE_BUTTON_W, (float)MAP_SIZE_BUTTON_H};
}

static SDL_FRect map_rows_plus_rect(void) {
    return (SDL_FRect){(float)(TILE_PANEL_X + 126 + MAP_SIZE_BUTTON_W + MAP_SIZE_BUTTON_GAP),
                       (float)MAP_SIZE_BUTTON_Y, (float)MAP_SIZE_BUTTON_W, (float)MAP_SIZE_BUTTON_H};
}

static bool handle_map_size_ui_click(AppState* app, int x, int y) {
    if (!app) {
        return false;
    }

    SDL_FRect w_minus = map_cols_minus_rect();
    SDL_FRect w_plus = map_cols_plus_rect();
    SDL_FRect h_minus = map_rows_minus_rect();
    SDL_FRect h_plus = map_rows_plus_rect();

    bool changed = false;
    if (point_in_frect(x, y, &w_minus) && app->meta.map_cols > 1) {
        app->meta.map_cols--;
        changed = true;
    } else if (point_in_frect(x, y, &w_plus) && app->meta.map_cols < MAP_COLS) {
        app->meta.map_cols++;
        changed = true;
    } else if (point_in_frect(x, y, &h_minus) && app->meta.map_rows > 1) {
        app->meta.map_rows--;
        changed = true;
    } else if (point_in_frect(x, y, &h_plus) && app->meta.map_rows < MAP_ROWS) {
        app->meta.map_rows++;
        changed = true;
    }

    if (changed) {
        metadata_clamp(&app->meta);
        app->dirty = true;
        set_status(app, "Map size updated");
    }
    return changed;
}

static SDL_FRect spawn_slot_rect(int slot) {
    return (SDL_FRect){(float)(TILE_PANEL_X + slot * (SPAWN_BUTTON_W + SPAWN_BUTTON_GAP)),
                       (float)SPAWN_BUTTON_Y, (float)SPAWN_BUTTON_W, (float)SPAWN_BUTTON_H};
}

static SDL_FRect spawn_move_up_rect(void) {
    return (SDL_FRect){(float)(TILE_PANEL_X + SPAWN_MOVE_W + 4), (float)SPAWN_MOVE_Y,
                       (float)SPAWN_MOVE_W, (float)SPAWN_MOVE_H};
}

static SDL_FRect spawn_move_left_rect(void) {
    return (SDL_FRect){(float)TILE_PANEL_X, (float)(SPAWN_MOVE_Y + SPAWN_MOVE_H + 2),
                       (float)SPAWN_MOVE_W, (float)SPAWN_MOVE_H};
}

static SDL_FRect spawn_move_down_rect(void) {
    return (SDL_FRect){(float)(TILE_PANEL_X + SPAWN_MOVE_W + 4),
                       (float)(SPAWN_MOVE_Y + SPAWN_MOVE_H + 2), (float)SPAWN_MOVE_W,
                       (float)SPAWN_MOVE_H};
}

static SDL_FRect spawn_move_right_rect(void) {
    return (SDL_FRect){(float)(TILE_PANEL_X + (SPAWN_MOVE_W + 4) * 2),
                       (float)(SPAWN_MOVE_Y + SPAWN_MOVE_H + 2), (float)SPAWN_MOVE_W,
                       (float)SPAWN_MOVE_H};
}

static bool handle_spawn_ui_click(AppState* app, int x, int y) {
    if (!app) {
        return false;
    }

    for (int slot = 0; slot < 5; ++slot) {
        SDL_FRect rect = spawn_slot_rect(slot);
        if (point_in_frect(x, y, &rect)) {
            app->active_spawn_slot = slot;
            app->pending_spawn_slot = slot;
            set_status(app, "Spawn armed: click map cell to place");
            return true;
        }
    }

    SDL_FRect up = spawn_move_up_rect();
    if (point_in_frect(x, y, &up)) {
        app_move_active_spawn(app, 0, -1);
        return true;
    }
    SDL_FRect left = spawn_move_left_rect();
    if (point_in_frect(x, y, &left)) {
        app_move_active_spawn(app, -1, 0);
        return true;
    }
    SDL_FRect down = spawn_move_down_rect();
    if (point_in_frect(x, y, &down)) {
        app_move_active_spawn(app, 0, 1);
        return true;
    }
    SDL_FRect right = spawn_move_right_rect();
    if (point_in_frect(x, y, &right)) {
        app_move_active_spawn(app, 1, 0);
        return true;
    }

    return false;
}

static void app_place_base_at_hover(AppState* app, bool enemy_base) {
    if (!app) {
        return;
    }

    int row = 0;
    int col = 0;
    if (!app_hover_cell(app, &row, &col)) {
        return;
    }

    StructureRect* structure = enemy_base ? &app->meta.enemy_base : &app->meta.player_base;
    structure->exists = true;
    structure->x = (uint8_t)col;
    structure->y = (uint8_t)row;
    if (structure->w == 0) {
        structure->w = 3;
    }
    if (structure->h == 0) {
        structure->h = 2;
    }
    structure->tile_x = (uint8_t)(app->selected_tile % SOURCE_TILE_GRID_SIZE);
    structure->tile_y = (uint8_t)(app->selected_tile / SOURCE_TILE_GRID_SIZE);
    structure_clamp_to_map(structure, (int)app->meta.map_cols, (int)app->meta.map_rows);
    app->dirty = true;
}

static void app_adjust_active_structure(AppState* app, SDL_Scancode code) {
    if (!app) {
        return;
    }

    StructureRect* structure = app_get_active_structure(app);
    if (!structure) {
        return;
    }
    const int max_cols = (int)app->meta.map_cols;
    const int max_rows = (int)app->meta.map_rows;

    switch (code) {
        case SDL_SCANCODE_UP:
            if (structure->y > 0) {
                structure->y--;
                app->dirty = true;
            }
            break;
        case SDL_SCANCODE_DOWN:
            if (structure->y < max_rows - 1) {
                structure->y++;
                app->dirty = true;
            }
            break;
        case SDL_SCANCODE_LEFT:
            if (structure->x > 0) {
                structure->x--;
                app->dirty = true;
            }
            break;
        case SDL_SCANCODE_RIGHT:
            if (structure->x < max_cols - 1) {
                structure->x++;
                app->dirty = true;
            }
            break;
        case SDL_SCANCODE_U:
            if (structure->w > 1) {
                structure->w--;
                app->dirty = true;
            }
            break;
        case SDL_SCANCODE_O:
            if (structure->w < max_cols) {
                structure->w++;
                app->dirty = true;
            }
            break;
        case SDL_SCANCODE_J:
            if (structure->h > 1) {
                structure->h--;
                app->dirty = true;
            }
            break;
        case SDL_SCANCODE_K:
            if (structure->h < max_rows) {
                structure->h++;
                app->dirty = true;
            }
            break;
        case SDL_SCANCODE_T:
            if (structure->tile_x > 0) {
                structure->tile_x--;
                app->dirty = true;
            }
            break;
        case SDL_SCANCODE_G:
            if (structure->tile_x < SOURCE_TILE_GRID_SIZE - 1) {
                structure->tile_x++;
                app->dirty = true;
            }
            break;
        case SDL_SCANCODE_Y:
            if (structure->tile_y > 0) {
                structure->tile_y--;
                app->dirty = true;
            }
            break;
        case SDL_SCANCODE_H:
            if (structure->tile_y < SOURCE_TILE_GRID_SIZE - 1) {
                structure->tile_y++;
                app->dirty = true;
            }
            break;
        default:
            break;
    }

    structure_clamp_to_map(structure, (int)app->meta.map_cols, (int)app->meta.map_rows);
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

static void render_spawn_marker(AppState* app, GridPoint point, SDL_Color color, const char* label,
                                bool active) {
    if (!app || !app->renderer || !label) {
        return;
    }

    const int cx = MAP_ORIGIN_X + point.x * CELL_SIZE + CELL_SIZE / 2;
    const int cy = MAP_ORIGIN_Y + point.y * CELL_SIZE + CELL_SIZE / 2;
    SDL_SetRenderDrawColor(app->renderer, color.r, color.g, color.b, color.a);
    SDL_FRect marker = {(float)(cx - 3), (float)(cy - 3), 7.0f, 7.0f};
    SDL_RenderFillRect(app->renderer, &marker);
    SDL_FRect border = {(float)(cx - 5), (float)(cy - 5), 11.0f, 11.0f};
    SDL_RenderRect(app->renderer, &border);
    if (active) {
        SDL_SetRenderDrawColor(app->renderer, 255, 255, 255, 255);
        SDL_FRect outer = {(float)(cx - 7), (float)(cy - 7), 15.0f, 15.0f};
        SDL_RenderRect(app->renderer, &outer);
    }
    render_text_line(app, label, cx + 6, cy - 3, color);
}

static void render_structure(AppState* app, const StructureRect* structure, SDL_Color color,
                             const char* label) {
    if (!app || !app->renderer || !structure || !structure->exists || !label) {
        return;
    }

    const int x = MAP_ORIGIN_X + structure->x * CELL_SIZE;
    const int y = MAP_ORIGIN_Y + structure->y * CELL_SIZE;
    const int w = structure->w * CELL_SIZE;
    const int h = structure->h * CELL_SIZE;

    SDL_SetRenderDrawColor(app->renderer, color.r, color.g, color.b, color.a);
    SDL_FRect outer = {(float)x, (float)y, (float)w, (float)h};
    SDL_RenderRect(app->renderer, &outer);
    SDL_FRect inner = {(float)(x + 1), (float)(y + 1), (float)(w - 2), (float)(h - 2)};
    SDL_RenderRect(app->renderer, &inner);

    char text[80];
    snprintf(text, sizeof(text), "%s %ux%u t(%u,%u)", label, (unsigned)structure->w,
             (unsigned)structure->h, (unsigned)structure->tile_x, (unsigned)structure->tile_y);
    render_text_line(app, text, x + 2, y + 2, color);
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
            const bool enabled = app_cell_enabled(app, row, col);

            if (!enabled) {
                SDL_SetRenderDrawColor(app->renderer, 22, 22, 22, 255);
                SDL_FRect disabled = {(float)x, (float)y, (float)CELL_SIZE, (float)CELL_SIZE};
                SDL_RenderFillRect(app->renderer, &disabled);
                SDL_SetRenderDrawColor(app->renderer, 58, 58, 58, 255);
                SDL_RenderRect(app->renderer, &disabled);
                continue;
            }

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

    render_spawn_marker(app, app->meta.player_spawns[0], (SDL_Color){80, 220, 255, 255}, "P1",
                        app->active_spawn_slot == 0);
    render_spawn_marker(app, app->meta.player_spawns[1], (SDL_Color){140, 255, 170, 255}, "P2",
                        app->active_spawn_slot == 1);
    render_spawn_marker(app, app->meta.enemy_spawns[0], (SDL_Color){255, 140, 140, 255}, "E1",
                        app->active_spawn_slot == 2);
    render_spawn_marker(app, app->meta.enemy_spawns[1], (SDL_Color){255, 170, 120, 255}, "E2",
                        app->active_spawn_slot == 3);
    render_spawn_marker(app, app->meta.enemy_spawns[2], (SDL_Color){255, 210, 100, 255}, "E3",
                        app->active_spawn_slot == 4);
    render_structure(app, &app->meta.player_base, (SDL_Color){80, 210, 255, 255}, "HOME");
    render_structure(app, &app->meta.enemy_base, (SDL_Color){255, 120, 120, 255}, "ENEMY");
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

static void render_button(AppState* app, const SDL_FRect* rect, const char* label, bool selected) {
    if (!app || !app->renderer || !rect || !label) {
        return;
    }

    SDL_Color fill = selected ? (SDL_Color){62, 112, 154, 255} : (SDL_Color){60, 60, 60, 255};
    SDL_Color border = selected ? (SDL_Color){220, 240, 255, 255} : (SDL_Color){128, 128, 128, 255};
    SDL_SetRenderDrawColor(app->renderer, fill.r, fill.g, fill.b, fill.a);
    SDL_RenderFillRect(app->renderer, rect);
    SDL_SetRenderDrawColor(app->renderer, border.r, border.g, border.b, border.a);
    SDL_RenderRect(app->renderer, rect);

    int tx = (int)(rect->x + (rect->w - (float)ui_text_width(label)) * 0.5f);
    int ty = (int)(rect->y + (rect->h - 7.0f) * 0.5f);
    render_text_line(app, label, tx, ty, (SDL_Color){255, 255, 255, 255});
}

static bool point_in_frect(int x, int y, const SDL_FRect* rect) {
    if (!rect) {
        return false;
    }
    return (float)x >= rect->x && (float)x <= (rect->x + rect->w) && (float)y >= rect->y &&
           (float)y <= (rect->y + rect->h);
}

static void render_menu_overlay(AppState* app) {
    if (!app || !app->renderer || !app_menu_is_open(app)) {
        return;
    }

    SDL_SetRenderDrawBlendMode(app->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 130);
    SDL_FRect fade = {0.0f, 0.0f, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT};
    SDL_RenderFillRect(app->renderer, &fade);

    SDL_FRect dialog = menu_dialog_rect(app->menu_mode);
    SDL_SetRenderDrawColor(app->renderer, 34, 34, 34, 238);
    SDL_RenderFillRect(app->renderer, &dialog);
    SDL_SetRenderDrawColor(app->renderer, 128, 128, 128, 255);
    SDL_RenderRect(app->renderer, &dialog);

    if (app->menu_mode == APP_MENU_STARTUP) {
        render_text_line(app, "Map Maker", (int)dialog.x + 12, (int)dialog.y + 12,
                         (SDL_Color){240, 240, 240, 255});
        render_text_line(app, "Select Open or New to begin", (int)dialog.x + 12, (int)dialog.y + 26,
                         (SDL_Color){190, 190, 190, 255});
    } else {
        render_text_line(app, "Menu", (int)dialog.x + 12, (int)dialog.y + 12,
                         (SDL_Color){240, 240, 240, 255});
        render_text_line(app, "Load / Save / New", (int)dialog.x + 12, (int)dialog.y + 26,
                         (SDL_Color){190, 190, 190, 255});
    }

    const int option_count = app_menu_option_count(app->menu_mode);
    for (int i = 0; i < option_count; ++i) {
        SDL_FRect option = menu_option_rect(app->menu_mode, i);
        render_button(app, &option, app_menu_option_label(app->menu_mode, i), false);
    }

    SDL_SetRenderDrawBlendMode(app->renderer, SDL_BLENDMODE_NONE);
}

static void render_ui(AppState* app) {
    if (!app || !app->renderer) {
        return;
    }

    char line1[180];
    snprintf(line1, sizeof(line1), "Tile:%03d H:%u D:%u M:%u | Enemies:%u Factory:%s",
             app->selected_tile, (unsigned)app->brush_health, (unsigned)app->brush_destruction,
             (unsigned)app->brush_movement, (unsigned)app->meta.enemy_count,
             app->meta.enemy_base_produces_extra ? "ON" : "OFF");
    render_text_line(app, line1, TILE_PANEL_X, TILE_PANEL_Y + 300, (SDL_Color){235, 235, 235, 255});

    const char* active_label = app->active_base == BASE_EDIT_ENEMY ? "enemy" : "player";
    const char* spawn_labels[] = {"P1", "P2", "E1", "E2", "E3"};
    const bool spawn_pending = app->pending_spawn_slot >= 0 && app->pending_spawn_slot <= 4;
    const int pending_spawn = spawn_pending ? app->pending_spawn_slot : 0;
    char line2[220];
    if (spawn_pending) {
        snprintf(line2, sizeof(line2), "Spawn %s armed: click map cell to place, Tab base:%s, ESC menu",
                 spawn_labels[pending_spawn], active_label);
    } else {
        snprintf(line2, sizeof(line2),
                 "Click P1/P2/E1/E2/E3 then click map to place, Tab base:%s, ESC menu",
                 active_label);
    }
    render_text_line(app, line2, TILE_PANEL_X, TILE_PANEL_Y + 316, (SDL_Color){180, 180, 180, 255});
    render_text_line(app,
                     "Arrows move base, U/O width, J/K height, T/G tileX, Y/H tileY, F factory, -/= count",
                     TILE_PANEL_X, TILE_PANEL_Y + 332, (SDL_Color){180, 180, 180, 255});
    render_text_line(app,
                     "LMB paint, RMB pick, S save, L load, C reset map, [ ] health, , . destruct, ; ' movement",
                     TILE_PANEL_X, TILE_PANEL_Y + 348, (SDL_Color){180, 180, 180, 255});

    SDL_FRect menu_button = menu_button_rect();
    render_button(app, &menu_button, "Menu", app_menu_is_open(app));

    char size_line[64];
    snprintf(size_line, sizeof(size_line), "Size W:%u H:%u", (unsigned)app->meta.map_cols,
             (unsigned)app->meta.map_rows);
    render_text_line(app, size_line, TILE_PANEL_X, TILE_PANEL_Y + 400, (SDL_Color){220, 220, 220, 255});

    SDL_FRect w_minus = map_cols_minus_rect();
    SDL_FRect w_plus = map_cols_plus_rect();
    SDL_FRect h_minus = map_rows_minus_rect();
    SDL_FRect h_plus = map_rows_plus_rect();
    render_button(app, &w_minus, "W-", false);
    render_button(app, &w_plus, "W+", false);
    render_button(app, &h_minus, "H-", false);
    render_button(app, &h_plus, "H+", false);

    const char* slot_labels[5] = {"P1", "P2", "E1", "E2", "E3"};
    for (int slot = 0; slot < 5; ++slot) {
        SDL_FRect rect = spawn_slot_rect(slot);
        render_button(app, &rect, slot_labels[slot], app->pending_spawn_slot == slot);
    }

    SDL_FRect up = spawn_move_up_rect();
    SDL_FRect left = spawn_move_left_rect();
    SDL_FRect down = spawn_move_down_rect();
    SDL_FRect right = spawn_move_right_rect();
    render_button(app, &up, "U", false);
    render_button(app, &left, "L", false);
    render_button(app, &down, "D", false);
    render_button(app, &right, "R", false);

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

    if (left_button && handle_menu_click(app, app->mouse_x, app->mouse_y)) {
        return;
    }
    if (app_menu_is_open(app)) {
        return;
    }

    if (left_button && handle_map_size_ui_click(app, app->mouse_x, app->mouse_y)) {
        return;
    }

    if (left_button && handle_spawn_ui_click(app, app->mouse_x, app->mouse_y)) {
        return;
    }

    if (left_button && app->pending_spawn_slot >= 0 && app->pending_spawn_slot <= 4) {
        int row = -1;
        int col = -1;
        if (hit_test_map(app->mouse_x, app->mouse_y, &row, &col) && app_cell_enabled(app, row, col)) {
            app_set_spawn_at_cell(app, app->pending_spawn_slot, row, col);
            app->active_spawn_slot = -1;
            app->pending_spawn_slot = -1;
            set_status(app, "Spawn placed");
        }
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
        if (!app_cell_enabled(app, row, col)) {
            return;
        }
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
    app->active_base = BASE_EDIT_PLAYER;
    app->active_spawn_slot = -1;
    app->pending_spawn_slot = -1;
    app->menu_mode = APP_MENU_STARTUP;
    metadata_init_defaults(&app->meta);
    init_default_map();
    set_status(app, "Startup menu: choose Open or New");

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

    app->window = SDL_CreateWindow("Map Maker v0.2", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
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
    metadata_clamp(&app->meta);
    app->dirty = false;

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
                if (app->mouse_left_pressed && app->pending_spawn_slot < 0 && !app_menu_is_open(app)) {
                    int row = -1;
                    int col = -1;
                    if (hit_test_map(app->mouse_x, app->mouse_y, &row, &col) &&
                        app_cell_enabled(app, row, col)) {
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
            case SDL_EVENT_KEY_DOWN: {
                if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
                    if (app->menu_mode == APP_MENU_NONE) {
                        app->menu_mode = APP_MENU_MAIN;
                    } else if (app->menu_mode == APP_MENU_MAIN) {
                        app->menu_mode = APP_MENU_NONE;
                    }
                    break;
                }

                if (app_menu_is_open(app)) {
                    break;
                }

                const bool shift_held = (event.key.mod & SDL_KMOD_SHIFT) != 0;
                switch (event.key.scancode) {
                    case SDL_SCANCODE_S:
                        app_save_map_action(app);
                        break;
                    case SDL_SCANCODE_L:
                        app_load_map_action(app);
                        break;
                    case SDL_SCANCODE_C:
                        app_new_map_action(app);
                        break;
                    case SDL_SCANCODE_1:
                        if (shift_held) {
                            app->active_spawn_slot = 0;
                        } else {
                            app_set_spawn_at_hover(app, 0);
                        }
                        break;
                    case SDL_SCANCODE_2:
                        if (shift_held) {
                            app->active_spawn_slot = 1;
                        } else {
                            app_set_spawn_at_hover(app, 1);
                        }
                        break;
                    case SDL_SCANCODE_3:
                        if (shift_held) {
                            app->active_spawn_slot = 2;
                        } else {
                            app_set_spawn_at_hover(app, 2);
                        }
                        break;
                    case SDL_SCANCODE_4:
                        if (shift_held) {
                            app->active_spawn_slot = 3;
                        } else {
                            app_set_spawn_at_hover(app, 3);
                        }
                        break;
                    case SDL_SCANCODE_5:
                        if (shift_held) {
                            app->active_spawn_slot = 4;
                        } else {
                            app_set_spawn_at_hover(app, 4);
                        }
                        break;
                    case SDL_SCANCODE_B:
                        if (shift_held) {
                            app->meta.player_base.exists = !app->meta.player_base.exists;
                            app->dirty = true;
                        } else {
                            app_place_base_at_hover(app, false);
                            app->active_base = BASE_EDIT_PLAYER;
                        }
                        break;
                    case SDL_SCANCODE_N:
                        if (shift_held) {
                            app->meta.enemy_base.exists = !app->meta.enemy_base.exists;
                            app->dirty = true;
                        } else {
                            app_place_base_at_hover(app, true);
                            app->active_base = BASE_EDIT_ENEMY;
                        }
                        break;
                    case SDL_SCANCODE_TAB:
                        app->active_base = app->active_base == BASE_EDIT_PLAYER ? BASE_EDIT_ENEMY
                                                                                 : BASE_EDIT_PLAYER;
                        break;
                    case SDL_SCANCODE_MINUS:
                        if (app->meta.enemy_count > 0) {
                            app->meta.enemy_count--;
                            app->dirty = true;
                        }
                        break;
                    case SDL_SCANCODE_EQUALS:
                        if (app->meta.enemy_count < 255) {
                            app->meta.enemy_count++;
                            app->dirty = true;
                        }
                        break;
                    case SDL_SCANCODE_F:
                        app->meta.enemy_base_produces_extra = !app->meta.enemy_base_produces_extra;
                        app->dirty = true;
                        break;
                    case SDL_SCANCODE_UP:
                        if (shift_held) {
                            app_move_active_spawn(app, 0, -1);
                        } else {
                            app_adjust_active_structure(app, event.key.scancode);
                        }
                        break;
                    case SDL_SCANCODE_DOWN:
                        if (shift_held) {
                            app_move_active_spawn(app, 0, 1);
                        } else {
                            app_adjust_active_structure(app, event.key.scancode);
                        }
                        break;
                    case SDL_SCANCODE_LEFT:
                        if (shift_held) {
                            app_move_active_spawn(app, -1, 0);
                        } else {
                            app_adjust_active_structure(app, event.key.scancode);
                        }
                        break;
                    case SDL_SCANCODE_RIGHT:
                        if (shift_held) {
                            app_move_active_spawn(app, 1, 0);
                        } else {
                            app_adjust_active_structure(app, event.key.scancode);
                        }
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
                        app_adjust_active_structure(app, event.key.scancode);
                        break;
                }
                break;
            }
            default:
                break;
        }
    }
}

static void app_update_hover(AppState* app) {
    if (!app) {
        return;
    }

    if (app_menu_is_open(app)) {
        app->hover_row = -1;
        app->hover_col = -1;
        return;
    }

    int row = -1;
    int col = -1;
    if (hit_test_map(app->mouse_x, app->mouse_y, &row, &col) && app_cell_enabled(app, row, col)) {
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
    render_menu_overlay(app);

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
