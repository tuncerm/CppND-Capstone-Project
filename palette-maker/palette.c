#include "palette.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Initialize a palette with default 16-color palette
 * Creates a standard set of colors suitable for retro games
 */
void palette_init(Palette* palette, const AppConfig* config) {
    if (!palette)
        return;

    // Initialize with a classic 16-color palette similar to EGA/VGA
    // Colors arranged for good visual variety and usefulness
    const PaletteColor default_colors[16] = {
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

    // Copy default colors to palette
    for (int i = 0; i < config->color_count && i < 16; i++) {
        palette->colors[i] = default_colors[i];
    }

    palette->modified = false;
    palette->current_file[0] = '\0';  // Empty string
}

/**
 * Reset palette to default colors
 * Resets to the same default colors as palette_init but marks as modified
 */
void palette_reset_to_default(Palette* palette) {
    if (!palette)
        return;

    // Use the same default colors as palette_init
    const PaletteColor default_colors[16] = {
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

    // Copy default colors to palette
    for (int i = 0; i < 16; i++) {
        palette->colors[i] = default_colors[i];
    }

    palette->modified = true;  // Mark as modified since this is a user action
}

/**
 * Load palette from binary file
 * Expected format: 64 bytes (16 colors × 4 bytes RGBA each)
 */
bool palette_load(Palette* palette, const char* path) {
    if (!palette || !path) {
        return false;
    }

    FILE* file = fopen(path, "rb");
    if (!file) {
        printf("Error: Could not open file '%s' for reading\n", path);
        return false;
    }

    // Read exactly 64 bytes (16 colors × 4 bytes)
    size_t bytes_read = fread(palette->colors, sizeof(PaletteColor), 16, file);
    fclose(file);

    if (bytes_read != 16) {
        printf("Error: Invalid palette file format. Expected 64 bytes, got %zu colors\n",
               bytes_read);
        return false;
    }

    // Store current file path and mark as unmodified
    strncpy(palette->current_file, path, sizeof(palette->current_file) - 1);
    palette->current_file[sizeof(palette->current_file) - 1] = '\0';
    palette->modified = false;

    printf("Palette loaded successfully from '%s'\n", path);
    return true;
}

/**
 * Save palette to binary file
 * Saves exactly 64 bytes (16 colors × 4 bytes RGBA each)
 */
bool palette_save(Palette* palette, const char* path) {
    if (!palette || !path) {
        return false;
    }

    FILE* file = fopen(path, "wb");
    if (!file) {
        printf("Error: Could not open file '%s' for writing\n", path);
        return false;
    }

    // Write exactly 64 bytes (16 colors × 4 bytes)
    size_t bytes_written = fwrite(palette->colors, sizeof(PaletteColor), 16, file);
    fclose(file);

    if (bytes_written != 16) {
        printf("Error: Failed to write complete palette data\n");
        return false;
    }

    printf("Palette saved successfully to '%s'\n", path);
    palette_mark_saved(palette);
    return true;
}

/**
 * Set a color in the palette at specified index
 * Validates index and marks palette as modified
 */
void palette_set_color(Palette* palette, int index, PaletteColor color) {
    if (!palette || index < 0 || index >= 16) {
        return;
    }

    palette->colors[index] = color;
    palette->modified = true;
}

/**
 * Get a color from the palette at specified index
 * Returns black color if index is invalid
 */
PaletteColor palette_get_color(const Palette* palette, int index) {
    if (!palette || index < 0 || index >= 16) {
        // Return black as default
        PaletteColor black = {0, 0, 0, 255};
        return black;
    }

    return palette->colors[index];
}

/**
 * Check if palette has been modified since last save/load
 */
bool palette_is_modified(const Palette* palette) {
    if (!palette)
        return false;
    return palette->modified;
}

/**
 * Mark palette as saved (clear modification flag)
 * Typically called after successful save operation
 */
void palette_mark_saved(Palette* palette) {
    if (!palette)
        return;
    palette->modified = false;
}

/**
 * Clamp a value to valid color component range (0-255)
 * Ensures input values are within valid uint8_t range
 */
uint8_t palette_clamp_component(int value) {
    if (value < 0)
        return 0;
    if (value > 255)
        return 255;
    return (uint8_t)value;
}

/**
 * Create a PaletteColor from individual RGBA components
 * Automatically clamps values to valid 0-255 range
 */
PaletteColor palette_make_color(int r, int g, int b, int a) {
    PaletteColor color;
    color.r = palette_clamp_component(r);
    color.g = palette_clamp_component(g);
    color.b = palette_clamp_component(b);
    color.a = palette_clamp_component(a);
    return color;
}
