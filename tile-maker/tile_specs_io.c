#include "tile_specs_io.h"

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
