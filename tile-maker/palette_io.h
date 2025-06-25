#ifndef PALETTE_IO_H
#define PALETTE_IO_H

#include <SDL3/SDL.h>
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
} RGBA;

/**
 * Global palette array - 16 RGBA colors loaded from palette.dat
 * This is the master palette that tiles reference via 4-bit indices
 */
extern RGBA gPalette[16];

/**
 * Initialize the global palette with default colors
 * Creates a basic 16-color palette suitable for tile graphics
 */
void palette_init(void);

/**
 * Load palette from palette.dat file
 * File format: 64 bytes (16 colors * 4 bytes RGBA)
 * Updates the global gPalette array
 *
 * @param path File path to load from (typically "palette.dat")
 * @return true if successful, false on error
 */
bool palette_load(const char* path);

/**
 * Save palette to palette.dat file
 * File format: 64 bytes (16 colors * 4 bytes RGBA)
 * Saves the current global gPalette array
 *
 * @param path File path to save to (typically "palette.dat")
 * @return true if successful, false on error
 */
bool palette_save(const char* path);

/**
 * Get SDL_Color from palette index
 * Converts 4-bit palette index to SDL color for rendering
 *
 * @param index Palette index (0-15)
 * @return SDL_Color structure for rendering
 */
SDL_Color palette_get_sdl_color(int index);

/**
 * Convert RGBA to SDL_Color
 * Helper function for SDL rendering
 *
 * @param rgba Source RGBA color
 * @return SDL_Color structure
 */
SDL_Color rgba_to_sdl_color(RGBA rgba);

#endif  // PALETTE_IO_H
