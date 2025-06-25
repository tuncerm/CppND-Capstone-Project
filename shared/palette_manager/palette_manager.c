#include "palette_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Default 16-color palette
 * Standard palette used by both applications
 */
static const RGBA DEFAULT_PALETTE[PALETTE_COLOR_COUNT] = {
    {0, 0, 0, 255},        // 0: Black
    {255, 255, 255, 255},  // 1: White
    {255, 0, 0, 255},      // 2: Red
    {0, 255, 0, 255},      // 3: Green
    {0, 0, 255, 255},      // 4: Blue
    {255, 255, 0, 255},    // 5: Yellow
    {255, 0, 255, 255},    // 6: Magenta
    {0, 255, 255, 255},    // 7: Cyan
    {128, 128, 128, 255},  // 8: Gray
    {192, 192, 192, 255},  // 9: Light Gray
    {128, 0, 0, 255},      // 10: Dark Red
    {0, 128, 0, 255},      // 11: Dark Green
    {0, 0, 128, 255},      // 12: Dark Blue
    {128, 128, 0, 255},    // 13: Dark Yellow
    {128, 0, 128, 255},    // 14: Dark Magenta
    {0, 128, 128, 255}     // 15: Dark Cyan
};

/**
 * Initialize palette manager with default colors
 */
bool palette_manager_init(PaletteManager* pm) {
    if (!pm) {
        return false;
    }

    // Copy default palette
    memcpy(pm->colors, DEFAULT_PALETTE, sizeof(DEFAULT_PALETTE));

    // Initialize state
    pm->modified = false;
    pm->file_loaded = false;
    pm->current_file[0] = '\0';

    return true;
}

/**
 * Reset palette to default 16-color palette
 */
void palette_manager_reset_to_default(PaletteManager* pm) {
    if (!pm) {
        return;
    }

    memcpy(pm->colors, DEFAULT_PALETTE, sizeof(DEFAULT_PALETTE));
    palette_manager_mark_modified(pm);
}

/**
 * Get color at specified index
 */
RGBA palette_get_color(const PaletteManager* pm, int index) {
    if (!pm || index < 0 || index >= PALETTE_COLOR_COUNT) {
        RGBA black = {0, 0, 0, 255};
        return black;
    }

    return pm->colors[index];
}

/**
 * Set color at specified index
 */
bool palette_set_color(PaletteManager* pm, int index, RGBA color) {
    if (!pm || index < 0 || index >= PALETTE_COLOR_COUNT) {
        return false;
    }

    pm->colors[index] = color;
    palette_manager_mark_modified(pm);
    return true;
}

/**
 * Get SDL_Color for specified palette index
 */
SDL_Color palette_get_sdl_color(const PaletteManager* pm, int index) {
    RGBA rgba = palette_get_color(pm, index);
    SDL_Color color = {rgba.r, rgba.g, rgba.b, rgba.a};
    return color;
}

/**
 * Create RGBA color from individual components
 */
RGBA palette_make_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    RGBA color = {r, g, b, a};
    return color;
}

/**
 * Convert SDL_Color to RGBA
 */
RGBA palette_from_sdl_color(SDL_Color color) {
    RGBA rgba = {color.r, color.g, color.b, color.a};
    return rgba;
}

/**
 * Load palette from file
 */
bool palette_manager_load(PaletteManager* pm, const char* filepath) {
    if (!pm || !filepath) {
        return false;
    }

    FILE* file = fopen(filepath, "rb");
    if (!file) {
        printf("Error: Could not open palette file '%s' for reading\n", filepath);
        return false;
    }

    // Read palette data (64 bytes: 16 colors × 4 bytes RGBA)
    size_t bytes_read = fread(pm->colors, 1, sizeof(pm->colors), file);
    fclose(file);

    if (bytes_read != sizeof(pm->colors)) {
        printf("Error: Invalid palette file format. Expected %zu bytes, got %zu\n",
               sizeof(pm->colors), bytes_read);
        return false;
    }

    // Update state
    strncpy(pm->current_file, filepath, sizeof(pm->current_file) - 1);
    pm->current_file[sizeof(pm->current_file) - 1] = '\0';
    pm->file_loaded = true;
    pm->modified = false;

    printf("Palette loaded from '%s'\n", filepath);
    return true;
}

/**
 * Save palette to file
 */
bool palette_manager_save(PaletteManager* pm, const char* filepath) {
    if (!pm) {
        return false;
    }

    // Use provided filepath or current file
    const char* save_path = filepath;
    if (!save_path) {
        if (!pm->file_loaded || pm->current_file[0] == '\0') {
            save_path = "palette.dat";  // Default filename
        } else {
            save_path = pm->current_file;
        }
    }

    FILE* file = fopen(save_path, "wb");
    if (!file) {
        printf("Error: Could not open file '%s' for writing\n", save_path);
        return false;
    }

    // Write palette data (64 bytes: 16 colors × 4 bytes RGBA)
    size_t bytes_written = fwrite(pm->colors, 1, sizeof(pm->colors), file);
    fclose(file);

    if (bytes_written != sizeof(pm->colors)) {
        printf("Error: Failed to write complete palette data\n");
        return false;
    }

    // Update state
    if (filepath) {
        strncpy(pm->current_file, filepath, sizeof(pm->current_file) - 1);
        pm->current_file[sizeof(pm->current_file) - 1] = '\0';
    }
    pm->file_loaded = true;
    pm->modified = false;

    printf("Palette saved to '%s'\n", save_path);
    return true;
}

/**
 * Check if palette has been modified
 */
bool palette_manager_is_modified(const PaletteManager* pm) {
    if (!pm) {
        return false;
    }
    return pm->modified;
}

/**
 * Mark palette as modified
 */
void palette_manager_mark_modified(PaletteManager* pm) {
    if (!pm) {
        return;
    }
    pm->modified = true;
}

/**
 * Clear modification flag
 */
void palette_manager_clear_modified(PaletteManager* pm) {
    if (!pm) {
        return;
    }
    pm->modified = false;
}

/**
 * Get current palette filename
 */
const char* palette_manager_get_filename(const PaletteManager* pm) {
    if (!pm || !pm->file_loaded || pm->current_file[0] == '\0') {
        return NULL;
    }
    return pm->current_file;
}

/**
 * Copy palette colors from another palette manager
 */
void palette_manager_copy(PaletteManager* dest, const PaletteManager* src) {
    if (!dest || !src) {
        return;
    }

    memcpy(dest->colors, src->colors, sizeof(src->colors));
    palette_manager_mark_modified(dest);
}

/**
 * Compare two palettes for equality
 */
bool palette_manager_equals(const PaletteManager* pm1, const PaletteManager* pm2) {
    if (!pm1 || !pm2) {
        return false;
    }

    return memcmp(pm1->colors, pm2->colors, sizeof(pm1->colors)) == 0;
}

/**
 * Validate palette file format
 */
bool palette_manager_validate_file(const char* filepath) {
    if (!filepath) {
        return false;
    }

    FILE* file = fopen(filepath, "rb");
    if (!file) {
        return false;
    }

    // Check file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fclose(file);

    // Palette file should be exactly 64 bytes (16 colors × 4 bytes RGBA)
    return file_size == 64;
}

/**
 * Get palette as raw byte array
 */
int palette_manager_get_raw_data(const PaletteManager* pm, uint8_t* out_data) {
    if (!pm || !out_data) {
        return 0;
    }

    memcpy(out_data, pm->colors, sizeof(pm->colors));
    return sizeof(pm->colors);  // 64 bytes
}

/**
 * Set palette from raw byte array
 */
bool palette_manager_set_raw_data(PaletteManager* pm, const uint8_t* data, int size) {
    if (!pm || !data) {
        return false;
    }

    if (size == 64) {
        // RGBA format (16 colors × 4 bytes)
        memcpy(pm->colors, data, sizeof(pm->colors));
        palette_manager_mark_modified(pm);
        return true;
    } else if (size == 48) {
        // RGB format (16 colors × 3 bytes) - convert to RGBA
        const uint8_t* src = data;
        for (int i = 0; i < PALETTE_COLOR_COUNT; i++) {
            pm->colors[i].r = src[i * 3 + 0];
            pm->colors[i].g = src[i * 3 + 1];
            pm->colors[i].b = src[i * 3 + 2];
            pm->colors[i].a = 255;  // Full alpha
        }
        palette_manager_mark_modified(pm);
        return true;
    }

    return false;  // Invalid size
}
