#include "tiles_io.h"

#include <stdio.h>
#include <string.h>

#include "tile_specs_io.h"

/**
 * Global tile storage - 256 tiles, each 32 bytes
 * Each tile is 8x8 pixels, 2 pixels per byte (4-bit palette indices)
 * Storage format: high nibble = first pixel, low nibble = second pixel
 */
uint8_t tiles[TILE_COUNT][BYTES_PER_TILE];

/**
 * Dirty flags for tiles - tracks which tiles need texture updates
 */
bool tile_dirty[TILE_COUNT];

/**
 * Global dirty flag - set when any tile is modified
 */
bool tiles_modified = false;

/**
 * Initialize tiles storage
 * Sets all tiles to palette index 0 (typically black)
 */
void tiles_init(void) {
    // Clear all tile data to 0 (palette index 0)
    memset(tiles, 0, sizeof(tiles));

    // Mark all tiles as dirty for initial rendering
    mark_all_tiles_dirty();

    // Set global modified flag
    tiles_modified = true;

    printf("Tiles initialized - all set to palette index 0\n");
}

/**
 * Load tiles from binary file.
 * Supported formats:
 * - 8448 bytes: [tile pixels][tile specs]
 * - 8192 bytes: legacy [tile pixels] only (specs defaulted)
 */
bool tiles_load(const char* path) {
    if (!path) {
        return false;
    }

    FILE* file = fopen(path, "rb");
    if (!file) {
        printf("Warning: Could not open tiles file '%s', initializing with empty tiles\n", path);
        tiles_init();
        tile_specs_reset_defaults();
        return false;
    }

    // Check file size.
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size != TILES_FILE_SIZE && file_size != TILES_LEGACY_FILE_SIZE) {
        printf("Warning: Invalid tiles file size. Expected %d (new) or %d (legacy), got %ld\n",
               TILES_FILE_SIZE, TILES_LEGACY_FILE_SIZE, file_size);
        fclose(file);
        tiles_init();
        tile_specs_reset_defaults();
        return false;
    }

    // Read tile pixel block.
    size_t pixels_read = fread(tiles, 1, TILES_PIXEL_DATA_SIZE, file);
    if (pixels_read != TILES_PIXEL_DATA_SIZE) {
        printf("Warning: Failed to read tile pixel data from '%s'\n", path);
        fclose(file);
        tiles_init();
        tile_specs_reset_defaults();
        return false;
    }

    if (file_size == TILES_FILE_SIZE) {
        // Read embedded tile specs block.
        size_t specs_read = fread(tile_specs, 1, TILE_SPECS_BLOCK_SIZE, file);
        if (specs_read != TILE_SPECS_BLOCK_SIZE) {
            printf("Warning: Failed to read tile specs block from '%s'\n", path);
            fclose(file);
            tiles_init();
            tile_specs_reset_defaults();
            return false;
        }
        tile_specs_mark_saved();
    } else {
        // Legacy format has no specs block.
        tile_specs_reset_defaults();
        tile_specs_mark_saved();
        printf("Warning: Loaded legacy tiles file '%s' without specs block; default specs applied.\n",
               path);
    }

    fclose(file);

    // Mark all tiles as clean (not modified since load)
    tiles_modified = false;

    // Mark all tiles as dirty for texture regeneration
    mark_all_tiles_dirty();

    printf("Tiles loaded successfully from '%s'\n", path);
    return true;
}

/**
 * Save tiles to binary file.
 * Format: [tile pixels][tile specs]
 */
bool tiles_save(const char* path) {
    if (!path) {
        return false;
    }

    FILE* file = fopen(path, "wb");
    if (!file) {
        printf("Error: Could not open file '%s' for writing\n", path);
        return false;
    }

    // Write tile pixel block.
    size_t pixels_written = fwrite(tiles, 1, TILES_PIXEL_DATA_SIZE, file);
    if (pixels_written != TILES_PIXEL_DATA_SIZE) {
        fclose(file);
        printf("Error: Failed to write tile pixel data. Wrote %zu bytes, expected %d\n",
               pixels_written, TILES_PIXEL_DATA_SIZE);
        return false;
    }

    // Write embedded tile specs block.
    size_t specs_written = fwrite(tile_specs, 1, TILE_SPECS_BLOCK_SIZE, file);
    fclose(file);
    if (specs_written != TILE_SPECS_BLOCK_SIZE) {
        printf("Error: Failed to write tile specs data. Wrote %zu bytes, expected %d\n",
               specs_written, TILE_SPECS_BLOCK_SIZE);
        return false;
    }

    // Clear modification flags
    tiles_mark_saved();
    tile_specs_mark_saved();

    printf("Tiles saved successfully to '%s'\n", path);
    return true;
}

/**
 * Get pixel value from a tile
 * Extracts 4-bit palette index from packed storage
 */
uint8_t get_px(int tile_id, int x, int y) {
    // Clamp parameters to valid ranges
    if (tile_id < 0 || tile_id >= TILE_COUNT)
        return 0;
    if (x < 0 || x >= TILE_WIDTH)
        return 0;
    if (y < 0 || y >= TILE_HEIGHT)
        return 0;

    // Calculate byte offset within tile
    int pixel_index = y * TILE_WIDTH + x;
    int byte_index = pixel_index / 2;              // 2 pixels per byte
    bool is_high_nibble = (pixel_index % 2) == 0;  // Even pixels use high nibble

    uint8_t byte_value = tiles[tile_id][byte_index];

    if (is_high_nibble) {
        return (byte_value >> 4) & 0x0F;  // Extract high nibble
    } else {
        return byte_value & 0x0F;  // Extract low nibble
    }
}

/**
 * Set pixel value in a tile
 * Stores 4-bit palette index in packed storage
 */
void set_px(int tile_id, int x, int y, uint8_t palette_index) {
    // Clamp parameters to valid ranges
    if (tile_id < 0 || tile_id >= TILE_COUNT)
        return;
    if (x < 0 || x >= TILE_WIDTH)
        return;
    if (y < 0 || y >= TILE_HEIGHT)
        return;

    // Clamp palette index to 4-bit range
    palette_index &= 0x0F;

    // Calculate byte offset within tile
    int pixel_index = y * TILE_WIDTH + x;
    int byte_index = pixel_index / 2;              // 2 pixels per byte
    bool is_high_nibble = (pixel_index % 2) == 0;  // Even pixels use high nibble

    uint8_t* byte_ptr = &tiles[tile_id][byte_index];

    if (is_high_nibble) {
        *byte_ptr = (*byte_ptr & 0x0F) | (palette_index << 4);  // Set high nibble
    } else {
        *byte_ptr = (*byte_ptr & 0xF0) | palette_index;  // Set low nibble
    }

    // Mark tile as dirty and set global modified flag
    mark_tile_dirty(tile_id);
    tiles_modified = true;
}

/**
 * Clear a tile to specified palette index
 */
void clear_tile(int tile_id, uint8_t palette_index) {
    if (tile_id < 0 || tile_id >= TILE_COUNT)
        return;

    // Clamp palette index to 4-bit range
    palette_index &= 0x0F;

    // Create byte value with both nibbles set to palette_index
    uint8_t fill_byte = (palette_index << 4) | palette_index;

    // Fill entire tile with the fill byte
    memset(tiles[tile_id], fill_byte, BYTES_PER_TILE);

    // Mark tile as dirty and set global modified flag
    mark_tile_dirty(tile_id);
    tiles_modified = true;
}

/**
 * Clear all tiles to specified palette index
 */
void clear_all_tiles(uint8_t palette_index) {
    // Clamp palette index to 4-bit range
    palette_index &= 0x0F;

    // Create byte value with both nibbles set to palette_index
    uint8_t fill_byte = (palette_index << 4) | palette_index;

    // Fill all tiles with the fill byte
    memset(tiles, fill_byte, sizeof(tiles));

    // Mark all tiles as dirty and set global modified flag
    mark_all_tiles_dirty();
    tiles_modified = true;
}

/**
 * Check if tiles have been modified since last save
 */
bool tiles_is_modified(void) {
    return tiles_modified;
}

/**
 * Mark tiles as saved (clear modification flag)
 */
void tiles_mark_saved(void) {
    tiles_modified = false;
}

/**
 * Mark a specific tile as dirty for texture regeneration
 */
void mark_tile_dirty(int tile_id) {
    if (tile_id >= 0 && tile_id < TILE_COUNT) {
        tile_dirty[tile_id] = true;
    }
}

/**
 * Mark all tiles as dirty for texture regeneration
 */
void mark_all_tiles_dirty(void) {
    for (int i = 0; i < TILE_COUNT; i++) {
        tile_dirty[i] = true;
    }
}

/**
 * Check if a tile is dirty and needs texture update
 */
bool is_tile_dirty(int tile_id) {
    if (tile_id < 0 || tile_id >= TILE_COUNT)
        return false;
    return tile_dirty[tile_id];
}

/**
 * Clear dirty flag for a tile
 */
void clear_tile_dirty(int tile_id) {
    if (tile_id >= 0 && tile_id < TILE_COUNT) {
        tile_dirty[tile_id] = false;
    }
}
