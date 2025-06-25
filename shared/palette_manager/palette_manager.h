#ifndef PALETTE_MANAGER_H
#define PALETTE_MANAGER_H

#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * Palette Manager for Shared Component Library
 *
 * Unified palette management system that combines the best features from
 * palette-maker's struct-based approach and tile-maker's global array system.
 * Supports 16-color RGBA palettes with file I/O and modification tracking.
 */

#define PALETTE_COLOR_COUNT 16
#define PALETTE_FILENAME_MAX 256

/**
 * RGBA color structure
 * Standard 8-bit per channel color representation
 */
typedef struct {
    uint8_t r, g, b, a;
} RGBA;

/**
 * Palette manager structure
 * Manages palette state, file operations, and modification tracking
 */
typedef struct {
    RGBA colors[PALETTE_COLOR_COUNT];         // 16 colors
    bool modified;                            // Has palette been modified since last save/load
    char current_file[PALETTE_FILENAME_MAX];  // Current palette file path
    bool file_loaded;                         // Whether a file has been loaded
} PaletteManager;

// ===== Core Palette Functions =====

/**
 * Initialize palette manager with default colors
 *
 * @param pm Palette manager to initialize
 * @return true if successful, false on error
 */
bool palette_manager_init(PaletteManager* pm);

/**
 * Reset palette to default 16-color palette
 *
 * @param pm Palette manager
 */
void palette_manager_reset_to_default(PaletteManager* pm);

/**
 * Get color at specified index
 *
 * @param pm Palette manager
 * @param index Color index (0-15)
 * @return RGBA color, returns black if invalid index
 */
RGBA palette_get_color(const PaletteManager* pm, int index);

/**
 * Set color at specified index
 *
 * @param pm Palette manager
 * @param index Color index (0-15)
 * @param color New color value
 * @return true if successful, false if invalid index
 */
bool palette_set_color(PaletteManager* pm, int index, RGBA color);

/**
 * Get SDL_Color for specified palette index
 * Convenience function for SDL rendering
 *
 * @param pm Palette manager
 * @param index Color index (0-15)
 * @return SDL_Color structure
 */
SDL_Color palette_get_sdl_color(const PaletteManager* pm, int index);

/**
 * Create RGBA color from individual components
 *
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @param a Alpha component (0-255)
 * @return RGBA color structure
 */
RGBA palette_make_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

/**
 * Convert SDL_Color to RGBA
 *
 * @param color SDL_Color to convert
 * @return RGBA color structure
 */
RGBA palette_from_sdl_color(SDL_Color color);

// ===== File I/O Functions =====

/**
 * Load palette from file
 *
 * @param pm Palette manager
 * @param filepath Path to palette file
 * @return true if successful, false on error
 */
bool palette_manager_load(PaletteManager* pm, const char* filepath);

/**
 * Save palette to file
 *
 * @param pm Palette manager
 * @param filepath Path to save palette file (NULL to use current file)
 * @return true if successful, false on error
 */
bool palette_manager_save(PaletteManager* pm, const char* filepath);

/**
 * Check if palette has been modified since last save/load
 *
 * @param pm Palette manager
 * @return true if modified, false otherwise
 */
bool palette_manager_is_modified(const PaletteManager* pm);

/**
 * Mark palette as modified
 * Called automatically when colors are changed
 *
 * @param pm Palette manager
 */
void palette_manager_mark_modified(PaletteManager* pm);

/**
 * Clear modification flag
 * Usually called after successful save operation
 *
 * @param pm Palette manager
 */
void palette_manager_clear_modified(PaletteManager* pm);

/**
 * Get current palette filename
 *
 * @param pm Palette manager
 * @return Current filename, or NULL if no file loaded
 */
const char* palette_manager_get_filename(const PaletteManager* pm);

// ===== Utility Functions =====

/**
 * Copy palette colors from another palette manager
 *
 * @param dest Destination palette manager
 * @param src Source palette manager
 */
void palette_manager_copy(PaletteManager* dest, const PaletteManager* src);

/**
 * Compare two palettes for equality
 *
 * @param pm1 First palette manager
 * @param pm2 Second palette manager
 * @return true if palettes have identical colors
 */
bool palette_manager_equals(const PaletteManager* pm1, const PaletteManager* pm2);

/**
 * Validate palette file format
 *
 * @param filepath Path to palette file
 * @return true if file appears to be valid palette format
 */
bool palette_manager_validate_file(const char* filepath);

/**
 * Get palette as raw byte array
 * Useful for interfacing with external systems
 *
 * @param pm Palette manager
 * @param out_data Output buffer (must be at least 64 bytes)
 * @return Number of bytes written (64 for RGBA, 48 for RGB)
 */
int palette_manager_get_raw_data(const PaletteManager* pm, uint8_t* out_data);

/**
 * Set palette from raw byte array
 *
 * @param pm Palette manager
 * @param data Raw palette data
 * @param size Size of data in bytes (64 for RGBA, 48 for RGB)
 * @return true if successful, false on error
 */
bool palette_manager_set_raw_data(PaletteManager* pm, const uint8_t* data, int size);

#endif  // PALETTE_MANAGER_H
