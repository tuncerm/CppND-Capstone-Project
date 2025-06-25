#ifndef PALETTE_H
#define PALETTE_H

#include <stdbool.h>
#include <stdint.h>

/**
 * RGBA color structure for palette entries
 * Each component is 8-bit (0-255)
 */
typedef struct {
    uint8_t r;  // Red component
    uint8_t g;  // Green component
    uint8_t b;  // Blue component
    uint8_t a;  // Alpha component
} PaletteColor;

/**
 * Palette structure containing 16 RGBA colors
 * Total size: 16 * 4 = 64 bytes
 */
typedef struct {
    PaletteColor colors[16];  // 16 color entries
    bool modified;            // Track if palette has been modified
    char current_file[256];   // Current file path (for save prompt)
} Palette;

/**
 * Initialize a palette with default colors
 * Creates a basic 16-color palette with common colors
 *
 * @param palette Pointer to palette structure to initialize
 */
void palette_init(Palette* palette);

/**
 * Load palette from file
 * File format: 64 bytes (16 colors * 4 bytes RGBA)
 *
 * @param palette Pointer to palette structure to load into
 * @param path File path to load from
 * @return true if successful, false on error
 */
bool palette_load(Palette* palette, const char* path);

/**
 * Save palette to file
 * File format: 64 bytes (16 colors * 4 bytes RGBA)
 *
 * @param palette Pointer to palette structure to save
 * @param path File path to save to
 * @return true if successful, false on error
 */
bool palette_save(const Palette* palette, const char* path);

/**
 * Set a color in the palette
 * Marks palette as modified
 *
 * @param palette Pointer to palette structure
 * @param index Color index (0-15)
 * @param color New color value
 */
void palette_set_color(Palette* palette, int index, PaletteColor color);

/**
 * Get a color from the palette
 *
 * @param palette Pointer to palette structure
 * @param index Color index (0-15)
 * @return Color at specified index
 */
PaletteColor palette_get_color(const Palette* palette, int index);

/**
 * Check if palette has unsaved modifications
 *
 * @param palette Pointer to palette structure
 * @return true if modified, false otherwise
 */
bool palette_is_modified(const Palette* palette);

/**
 * Mark palette as saved (clear modified flag)
 *
 * @param palette Pointer to palette structure
 */
void palette_mark_saved(Palette* palette);

/**
 * Validate color component value (clamp to 0-255)
 *
 * @param value Input value
 * @return Clamped value between 0-255
 */
uint8_t palette_clamp_component(int value);

/**
 * Create a PaletteColor from RGBA components
 * Automatically clamps values to valid range
 *
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @param a Alpha component (0-255)
 * @return PaletteColor structure
 */
PaletteColor palette_make_color(int r, int g, int b, int a);

#endif  // PALETTE_H
