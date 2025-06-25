#include "pixel_editor.h"
#include <stdio.h>
#include "palette_io.h"
#include "tiles_io.h"

/**
 * Initialize pixel editor system
 */
bool pixel_editor_init(PixelEditor* editor, SDL_Renderer* renderer) {
    if (!editor || !renderer) {
        return false;
    }

    // Initialize state
    editor->tile_texture = NULL;
    editor->grid_texture = NULL;
    editor->current_tile = 0;
    editor->current_color = 1;  // Start with palette index 1 (not black)
    editor->show_grid = true;
    editor->needs_rebuild = true;
    editor->dragging = false;

    // Generate grid texture
    editor->grid_texture = generate_grid_texture(renderer);
    if (!editor->grid_texture) {
        printf("Warning: Failed to create grid texture\n");
    }

    printf("Pixel editor initialized\n");
    return true;
}

/**
 * Cleanup pixel editor resources
 */
void pixel_editor_cleanup(PixelEditor* editor) {
    if (!editor)
        return;

    if (editor->tile_texture) {
        SDL_DestroyTexture(editor->tile_texture);
        editor->tile_texture = NULL;
    }

    if (editor->grid_texture) {
        SDL_DestroyTexture(editor->grid_texture);
        editor->grid_texture = NULL;
    }

    printf("Pixel editor cleaned up\n");
}

/**
 * Set the tile to edit
 */
void pixel_editor_set_tile(PixelEditor* editor, int tile_id) {
    if (!editor)
        return;

    // Clamp to valid range
    if (tile_id < 0)
        tile_id = 0;
    if (tile_id >= 64)
        tile_id = 63;

    if (editor->current_tile != tile_id) {
        editor->current_tile = tile_id;
        editor->needs_rebuild = true;
    }
}

/**
 * Set the current paint color
 */
void pixel_editor_set_color(PixelEditor* editor, int palette_index) {
    if (!editor)
        return;

    // Clamp to valid range
    if (palette_index < 0)
        palette_index = 0;
    if (palette_index > 15)
        palette_index = 15;

    editor->current_color = palette_index;
}

/**
 * Toggle grid overlay visibility
 */
void pixel_editor_toggle_grid(PixelEditor* editor) {
    if (!editor)
        return;
    editor->show_grid = !editor->show_grid;
}

/**
 * Generate pixel editor texture for current tile
 */
SDL_Texture* generate_pixel_editor_texture(SDL_Renderer* renderer, int tile_id) {
    if (!renderer || tile_id < 0 || tile_id >= 64) {
        return NULL;
    }

    // Create 256x256 texture for magnified tile display
    SDL_Texture* texture =
        SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                          PIXEL_EDITOR_WIDTH, PIXEL_EDITOR_HEIGHT);
    if (!texture) {
        printf("Error creating pixel editor texture: %s\n", SDL_GetError());
        return NULL;
    }

    // Set texture as render target
    SDL_Texture* original_target = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, texture);

    // Clear texture to transparent
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    // Draw each pixel as a 32x32 block (8x8 tile -> 256x256 display)
    for (int y = 0; y < TILE_HEIGHT; y++) {
        for (int x = 0; x < TILE_WIDTH; x++) {
            uint8_t palette_index = get_px(tile_id, x, y);
            SDL_Color color = palette_get_sdl_color(palette_index);

            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

            // Draw 32x32 pixel block
            SDL_FRect pixel_rect = {x * PIXEL_SIZE, y * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE};
            SDL_RenderFillRect(renderer, &pixel_rect);
        }
    }

    // Restore original render target
    SDL_SetRenderTarget(renderer, original_target);

    return texture;
}

/**
 * Generate grid overlay texture
 */
SDL_Texture* generate_grid_texture(SDL_Renderer* renderer) {
    if (!renderer) {
        return NULL;
    }

    // Create 256x256 texture for grid overlay
    SDL_Texture* texture =
        SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                          PIXEL_EDITOR_WIDTH, PIXEL_EDITOR_HEIGHT);
    if (!texture) {
        printf("Error creating grid texture: %s\n", SDL_GetError());
        return NULL;
    }

    // Enable alpha blending for the texture
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    // Set texture as render target
    SDL_Texture* original_target = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, texture);

    // Clear texture to transparent
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    // Draw grid lines
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 128);  // Semi-transparent white

    // Vertical lines
    for (int x = 0; x <= TILE_WIDTH; x++) {
        float line_x = x * PIXEL_SIZE;
        SDL_RenderLine(renderer, line_x, 0, line_x, PIXEL_EDITOR_HEIGHT);
    }

    // Horizontal lines
    for (int y = 0; y <= TILE_HEIGHT; y++) {
        float line_y = y * PIXEL_SIZE;
        SDL_RenderLine(renderer, 0, line_y, PIXEL_EDITOR_WIDTH, line_y);
    }

    // Restore original render target
    SDL_SetRenderTarget(renderer, original_target);

    return texture;
}

/**
 * Update pixel editor
 */
void pixel_editor_update(PixelEditor* editor, SDL_Renderer* renderer) {
    if (!editor || !renderer)
        return;

    // Rebuild tile texture if needed
    if (editor->needs_rebuild) {
        // Destroy old texture
        if (editor->tile_texture) {
            SDL_DestroyTexture(editor->tile_texture);
        }

        // Generate new texture
        editor->tile_texture = generate_pixel_editor_texture(renderer, editor->current_tile);
        editor->needs_rebuild = false;
    }
}

/**
 * Render pixel editor panel
 */
void pixel_editor_render(PixelEditor* editor, SDL_Renderer* renderer, int x, int y) {
    if (!editor || !renderer)
        return;

    // Draw background
    SDL_SetRenderDrawColor(renderer, 32, 32, 32, 255);
    SDL_FRect bg_rect = {x, y, PIXEL_EDITOR_WIDTH, PIXEL_EDITOR_HEIGHT};
    SDL_RenderFillRect(renderer, &bg_rect);

    // Draw tile texture
    if (editor->tile_texture) {
        SDL_FRect dest_rect = {x, y, PIXEL_EDITOR_WIDTH, PIXEL_EDITOR_HEIGHT};
        SDL_RenderTexture(renderer, editor->tile_texture, NULL, &dest_rect);
    }

    // Draw grid overlay if enabled
    if (editor->show_grid && editor->grid_texture) {
        SDL_FRect dest_rect = {x, y, PIXEL_EDITOR_WIDTH, PIXEL_EDITOR_HEIGHT};
        SDL_RenderTexture(renderer, editor->grid_texture, NULL, &dest_rect);
    }

    // Draw border
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    SDL_FRect border_rect = {x, y, PIXEL_EDITOR_WIDTH, PIXEL_EDITOR_HEIGHT};
    SDL_RenderRect(renderer, &border_rect);
}

/**
 * Handle mouse input for pixel editor
 */
bool pixel_editor_handle_input(PixelEditor* editor, int panel_x, int panel_y, int mouse_x,
                               int mouse_y, bool left_button, bool right_button, bool mouse_down) {
    if (!editor)
        return false;

    // Convert mouse coordinates to panel-relative coordinates
    int rel_x = mouse_x - panel_x;
    int rel_y = mouse_y - panel_y;

    // Check if mouse is within pixel editor bounds
    if (rel_x < 0 || rel_x >= PIXEL_EDITOR_WIDTH || rel_y < 0 || rel_y >= PIXEL_EDITOR_HEIGHT) {
        editor->dragging = false;
        return false;
    }

    // Calculate which pixel is under the mouse
    int pixel_x = rel_x / PIXEL_SIZE;
    int pixel_y = rel_y / PIXEL_SIZE;

    // Clamp to valid pixel coordinates
    if (pixel_x < 0)
        pixel_x = 0;
    if (pixel_x >= TILE_WIDTH)
        pixel_x = TILE_WIDTH - 1;
    if (pixel_y < 0)
        pixel_y = 0;
    if (pixel_y >= TILE_HEIGHT)
        pixel_y = TILE_HEIGHT - 1;

    bool pixel_modified = false;

    // Handle mouse input
    if (left_button) {
        // Left mouse button: Paint with current color
        if (mouse_down || editor->dragging) {
            uint8_t current_pixel = get_px(editor->current_tile, pixel_x, pixel_y);
            if (current_pixel != editor->current_color) {
                set_px(editor->current_tile, pixel_x, pixel_y, editor->current_color);
                editor->needs_rebuild = true;
                pixel_modified = true;
            }
            editor->dragging = mouse_down;
        }
    } else if (right_button && mouse_down) {
        // Right mouse button: Pick color
        uint8_t picked_color = get_px(editor->current_tile, pixel_x, pixel_y);
        editor->current_color = picked_color;
        editor->dragging = false;
    } else {
        editor->dragging = false;
    }

    return pixel_modified;
}

/**
 * Get current tile being edited
 */
int pixel_editor_get_tile(const PixelEditor* editor) {
    if (!editor)
        return 0;
    return editor->current_tile;
}

/**
 * Get current paint color
 */
int pixel_editor_get_color(const PixelEditor* editor) {
    if (!editor)
        return 0;
    return editor->current_color;
}

/**
 * Check if grid is visible
 */
bool pixel_editor_grid_visible(const PixelEditor* editor) {
    if (!editor)
        return false;
    return editor->show_grid;
}
