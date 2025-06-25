#include "palette_io.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * Global palette array - 16 RGBA colors
 * This is the master palette that all tiles reference via 4-bit indices
 */
RGBA gPalette[16];

/**
 * Initialize the global palette with default 16-color palette
 * Creates a standard set of colors suitable for tile graphics
 */
void palette_init(void) {
    // Initialize with a classic 16-color palette similar to EGA/VGA
    // Colors arranged for good visual variety and usefulness in tile graphics
    const RGBA default_colors[16] = {
        {0, 0, 0, 255},        // 0: Black
        {128, 0, 0, 255},      // 1: Dark Red
        {0, 128, 0, 255},      // 2: Dark Green
        {128, 128, 0, 255},    // 3: Dark Yellow/Brown
        {0, 0, 128, 255},      // 4: Dark Blue
        {128, 0, 128, 255},    // 5: Dark Magenta
        {0, 128, 128, 255},    // 6: Dark Cyan
        {192, 192, 192, 255},  // 7: Light Gray
        {128, 128, 128, 255},  // 8: Dark Gray
        {255, 0, 0, 255},      // 9: Bright Red
        {0, 255, 0, 255},      // 10: Bright Green
        {255, 255, 0, 255},    // 11: Bright Yellow
        {0, 0, 255, 255},      // 12: Bright Blue
        {255, 0, 255, 255},    // 13: Bright Magenta
        {0, 255, 255, 255},    // 14: Bright Cyan
        {255, 255, 255, 255}   // 15: White
    };

    // Copy default colors to global palette
    for (int i = 0; i < 16; i++) {
        gPalette[i] = default_colors[i];
    }
}

/**
 * Load palette from binary file
 * Expected format: 64 bytes (16 colors × 4 bytes RGBA each)
 */
bool palette_load(const char* path) {
    if (!path) {
        return false;
    }

    FILE* file = fopen(path, "rb");
    if (!file) {
        printf("Warning: Could not open palette file '%s', using default palette\n", path);
        return false;
    }

    // Read exactly 64 bytes (16 colors × 4 bytes)
    size_t bytes_read = fread(gPalette, sizeof(RGBA), 16, file);
    fclose(file);

    if (bytes_read != 16) {
        printf(
            "Warning: Invalid palette file format. Expected 64 bytes, got %zu colors. Using "
            "default palette.\n",
            bytes_read);
        palette_init();  // Fall back to default palette
        return false;
    }

    printf("Palette loaded successfully from '%s'\n", path);
    return true;
}

/**
 * Save palette to binary file
 * Saves exactly 64 bytes (16 colors × 4 bytes RGBA each)
 */
bool palette_save(const char* path) {
    if (!path) {
        return false;
    }

    FILE* file = fopen(path, "wb");
    if (!file) {
        printf("Error: Could not open file '%s' for writing\n", path);
        return false;
    }

    // Write exactly 64 bytes (16 colors × 4 bytes)
    size_t bytes_written = fwrite(gPalette, sizeof(RGBA), 16, file);
    fclose(file);

    if (bytes_written != 16) {
        printf("Error: Failed to write complete palette data\n");
        return false;
    }

    printf("Palette saved successfully to '%s'\n", path);
    return true;
}

/**
 * Get SDL_Color from palette index
 * Converts 4-bit palette index to SDL color for rendering
 * Clamps index to valid range (0-15)
 */
SDL_Color palette_get_sdl_color(int index) {
    // Clamp index to valid range
    if (index < 0)
        index = 0;
    if (index > 15)
        index = 15;

    RGBA rgba = gPalette[index];
    SDL_Color color = {rgba.r, rgba.g, rgba.b, rgba.a};
    return color;
}

/**
 * Convert RGBA to SDL_Color
 * Helper function for SDL rendering
 */
SDL_Color rgba_to_sdl_color(RGBA rgba) {
    SDL_Color color = {rgba.r, rgba.g, rgba.b, rgba.a};
    return color;
}
