#ifndef TILES_IO_H
#define TILES_IO_H

#include <stdbool.h>
#include <stdint.h>

/**
 * Tile dimensions and storage constants
 */
#define TILE_WIDTH 8
#define TILE_HEIGHT 8
#define TILE_COUNT 64
#define BYTES_PER_TILE 32                              // 8x8 pixels, 2 pixels per byte (4-bit each)
#define TILES_FILE_SIZE (TILE_COUNT * BYTES_PER_TILE)  // 2048 bytes

/**
 * Global tile storage - 64 tiles, each 32 bytes
 * Each tile is 8x8 pixels, 2 pixels per byte (4-bit palette indices)
 * High nibble = first pixel, low nibble = second pixel
 */
extern uint8_t tiles[TILE_COUNT][BYTES_PER_TILE];

/**
 * Dirty flags for tiles - tracks which tiles need texture updates
 */
extern bool tile_dirty[TILE_COUNT];

/**
 * Global dirty flag - set when any tile is modified
 */
extern bool tiles_modified;

/**
 * Initialize tiles storage
 * Sets all tiles to palette index 0 (typically black)
 * Marks all tiles as dirty and sets global modified flag
 */
void tiles_init(void);

/**
 * Load tiles from tiles.dat file
 * File format: 2048 bytes (64 tiles * 32 bytes each)
 *
 * @param path File path to load from (typically "tiles.dat")
 * @return true if successful, false on error
 */
bool tiles_load(const char* path);

/**
 * Save tiles to tiles.dat file
 * File format: 2048 bytes (64 tiles * 32 bytes each)
 * Clears the global modified flag on successful save
 *
 * @param path File path to save to (typically "tiles.dat")
 * @return true if successful, false on error
 */
bool tiles_save(const char* path);

/**
 * Get pixel value from a tile
 * Extracts 4-bit palette index from packed storage
 *
 * @param tile_id Tile index (0-63)
 * @param x Pixel x coordinate (0-7)
 * @param y Pixel y coordinate (0-7)
 * @return 4-bit palette index (0-15)
 */
uint8_t get_px(int tile_id, int x, int y);

/**
 * Set pixel value in a tile
 * Stores 4-bit palette index in packed storage
 * Marks tile as dirty and sets global modified flag
 *
 * @param tile_id Tile index (0-63)
 * @param x Pixel x coordinate (0-7)
 * @param y Pixel y coordinate (0-7)
 * @param palette_index 4-bit palette index (0-15)
 */
void set_px(int tile_id, int x, int y, uint8_t palette_index);

/**
 * Clear a tile to specified palette index
 * Sets all pixels in the tile to the given palette index
 *
 * @param tile_id Tile index (0-63)
 * @param palette_index 4-bit palette index (0-15)
 */
void clear_tile(int tile_id, uint8_t palette_index);

/**
 * Clear all tiles to specified palette index
 * Sets all pixels in all tiles to the given palette index
 *
 * @param palette_index 4-bit palette index (0-15)
 */
void clear_all_tiles(uint8_t palette_index);

/**
 * Check if tiles have been modified since last save
 *
 * @return true if tiles have been modified, false otherwise
 */
bool tiles_is_modified(void);

/**
 * Mark tiles as saved (clear modification flag)
 * Typically called after successful save operation
 */
void tiles_mark_saved(void);

/**
 * Mark a specific tile as dirty for texture regeneration
 *
 * @param tile_id Tile index (0-63)
 */
void mark_tile_dirty(int tile_id);

/**
 * Mark all tiles as dirty for texture regeneration
 */
void mark_all_tiles_dirty(void);

/**
 * Check if a tile is dirty and needs texture update
 *
 * @param tile_id Tile index (0-63)
 * @return true if tile is dirty, false otherwise
 */
bool is_tile_dirty(int tile_id);

/**
 * Clear dirty flag for a tile
 * Typically called after texture has been regenerated
 *
 * @param tile_id Tile index (0-63)
 */
void clear_tile_dirty(int tile_id);

#endif  // TILES_IO_H
