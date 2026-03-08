#include "tile_specs_io.h"

#include <stdio.h>
#include <string.h>

uint8_t tile_specs[TILE_COUNT];
bool tile_specs_modified = false;

static uint8_t make_spec(uint8_t health, uint8_t destruction_mode, uint8_t movement) {
    return (uint8_t)((health & TILE_SPEC_HEALTH_MASK) |
                     ((destruction_mode & TILE_SPEC_DESTRUCT_MASK) << TILE_SPEC_DESTRUCT_SHIFT) |
                     ((movement & TILE_SPEC_MOVEMENT_MASK) << TILE_SPEC_MOVEMENT_SHIFT));
}

static uint8_t default_spec(void) {
    return make_spec(1, TILE_DESTRUCT_MODE_NORMAL, TILE_MOVE_NO_PASS);
}

void tile_specs_reset_defaults(void) {
    const uint8_t spec = default_spec();
    for (int i = 0; i < TILE_COUNT; i++) {
        tile_specs[i] = spec;
    }
    tile_specs_modified = true;
}

void tile_specs_init(void) {
    tile_specs_reset_defaults();
}

bool tile_specs_load(const char* path) {
    if (!path) {
        return false;
    }

    FILE* file = fopen(path, "rb");
    if (!file) {
        printf("Warning: Could not open tile specs file '%s', using defaults\n", path);
        tile_specs_reset_defaults();
        return false;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (file_size != TILE_SPECS_FILE_SIZE) {
        printf("Warning: Invalid tile specs size. Expected %d bytes, got %ld. Using defaults.\n",
               TILE_SPECS_FILE_SIZE, file_size);
        fclose(file);
        tile_specs_reset_defaults();
        return false;
    }

    size_t bytes_read = fread(tile_specs, 1, TILE_SPECS_FILE_SIZE, file);
    fclose(file);

    if (bytes_read != TILE_SPECS_FILE_SIZE) {
        printf(
            "Warning: Failed to read complete tile specs. Read %zu bytes, expected %d. Using "
            "defaults.\n",
            bytes_read, TILE_SPECS_FILE_SIZE);
        tile_specs_reset_defaults();
        return false;
    }

    tile_specs_modified = false;
    printf("Tile specs loaded successfully from '%s'\n", path);
    return true;
}

bool tile_specs_save(const char* path) {
    if (!path) {
        return false;
    }

    FILE* file = fopen(path, "wb");
    if (!file) {
        printf("Error: Could not open tile specs file '%s' for writing\n", path);
        return false;
    }

    size_t bytes_written = fwrite(tile_specs, 1, TILE_SPECS_FILE_SIZE, file);
    fclose(file);
    if (bytes_written != TILE_SPECS_FILE_SIZE) {
        printf("Error: Failed to write tile specs. Wrote %zu bytes, expected %d\n", bytes_written,
               TILE_SPECS_FILE_SIZE);
        return false;
    }

    tile_specs_mark_saved();
    printf("Tile specs saved successfully to '%s'\n", path);
    return true;
}

bool tile_specs_is_modified(void) {
    return tile_specs_modified;
}

void tile_specs_mark_saved(void) {
    tile_specs_modified = false;
}

uint8_t tile_spec_get(int tile_id) {
    if (tile_id < 0 || tile_id >= TILE_COUNT) {
        return default_spec();
    }
    return tile_specs[tile_id];
}

void tile_spec_set(int tile_id, uint8_t spec) {
    if (tile_id < 0 || tile_id >= TILE_COUNT) {
        return;
    }

    if (tile_specs[tile_id] == spec) {
        return;
    }

    tile_specs[tile_id] = spec;
    tile_specs_modified = true;
}

uint8_t tile_spec_get_health(int tile_id) {
    return (uint8_t)(tile_spec_get(tile_id) & TILE_SPEC_HEALTH_MASK);
}

uint8_t tile_spec_get_destruction_mode(int tile_id) {
    return (uint8_t)((tile_spec_get(tile_id) >> TILE_SPEC_DESTRUCT_SHIFT) & TILE_SPEC_DESTRUCT_MASK);
}

uint8_t tile_spec_get_movement(int tile_id) {
    return (uint8_t)((tile_spec_get(tile_id) >> TILE_SPEC_MOVEMENT_SHIFT) & TILE_SPEC_MOVEMENT_MASK);
}

void tile_spec_set_health(int tile_id, uint8_t health) {
    const uint8_t spec = tile_spec_get(tile_id);
    const uint8_t updated =
        (uint8_t)((spec & (uint8_t)(~TILE_SPEC_HEALTH_MASK)) | (health & TILE_SPEC_HEALTH_MASK));
    tile_spec_set(tile_id, updated);
}

void tile_spec_set_destruction_mode(int tile_id, uint8_t destruction_mode) {
    const uint8_t spec = tile_spec_get(tile_id);
    const uint8_t updated =
        (uint8_t)((spec & (uint8_t)(~(TILE_SPEC_DESTRUCT_MASK << TILE_SPEC_DESTRUCT_SHIFT))) |
                  ((destruction_mode & TILE_SPEC_DESTRUCT_MASK) << TILE_SPEC_DESTRUCT_SHIFT));
    tile_spec_set(tile_id, updated);
}

void tile_spec_set_movement(int tile_id, uint8_t movement) {
    const uint8_t spec = tile_spec_get(tile_id);
    const uint8_t updated =
        (uint8_t)((spec & (uint8_t)(~(TILE_SPEC_MOVEMENT_MASK << TILE_SPEC_MOVEMENT_SHIFT))) |
                  ((movement & TILE_SPEC_MOVEMENT_MASK) << TILE_SPEC_MOVEMENT_SHIFT));
    tile_spec_set(tile_id, updated);
}
