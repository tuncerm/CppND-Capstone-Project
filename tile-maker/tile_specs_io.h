#ifndef TILE_SPECS_IO_H
#define TILE_SPECS_IO_H

#include <stdbool.h>
#include <stdint.h>

#include "tiles_io.h"

// Spec bit layout
#define TILE_SPEC_HEALTH_MASK 0x07
#define TILE_SPEC_DESTRUCT_SHIFT 3
#define TILE_SPEC_DESTRUCT_MASK 0x07
#define TILE_SPEC_MOVEMENT_SHIFT 6
#define TILE_SPEC_MOVEMENT_MASK 0x03

// Common mode values
#define TILE_DESTRUCT_MODE_INDESTRUCTIBLE 0
#define TILE_DESTRUCT_MODE_NORMAL 1
#define TILE_DESTRUCT_MODE_HEAVY 2
#define TILE_DESTRUCT_MODE_SPECIAL 3

#define TILE_MOVE_NO_PASS 0
#define TILE_MOVE_PASS 1
#define TILE_MOVE_REQUIRE 2
#define TILE_MOVE_SPECIAL 3

#define TILE_SPECS_FILE_SIZE TILE_COUNT

extern uint8_t tile_specs[TILE_COUNT];
extern bool tile_specs_modified;

void tile_specs_init(void);
void tile_specs_reset_defaults(void);
bool tile_specs_load(const char* path);
bool tile_specs_save(const char* path);
bool tile_specs_is_modified(void);
void tile_specs_mark_saved(void);

uint8_t tile_spec_get(int tile_id);
void tile_spec_set(int tile_id, uint8_t spec);

uint8_t tile_spec_get_health(int tile_id);
uint8_t tile_spec_get_destruction_mode(int tile_id);
uint8_t tile_spec_get_movement(int tile_id);

void tile_spec_set_health(int tile_id, uint8_t health);
void tile_spec_set_destruction_mode(int tile_id, uint8_t destruction_mode);
void tile_spec_set_movement(int tile_id, uint8_t movement);

#endif  // TILE_SPECS_IO_H
