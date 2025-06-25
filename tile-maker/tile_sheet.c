#include "tile_sheet.h"
#include <stdio.h>
#include "palette_io.h"
#include "tiles_io.h"

/**
 * Initialize tile sheet system
 * Creates position rectangles for all tiles
 */
bool tile_sheet_init(TileSheet* sheet, SDL_Renderer* renderer) {
    if (!sheet || !renderer) {
        return false;
    }

    // Initialize all textures to NULL
    for (int i = 0; i < 64; i++) {
        sheet->textures[i] = NULL;
    }

    // Calculate tile positions in 8x8 grid
    for (int i = 0; i < 64; i++) {
        int row = i / TILE_SHEET_COLS;
        int col = i % TILE_SHEET_COLS;

        sheet->tile_rects[i].x = col * TILE_DISPLAY_SIZE;
        sheet->tile_rects[i].y = row * TILE_DISPLAY_SIZE;
        sheet->tile_rects[i].w = TILE_DISPLAY_SIZE;
        sheet->tile_rects[i].h = TILE_DISPLAY_SIZE;
    }

    // Initialize state
    sheet->selected_tile = 0;
    sheet->hover_tile = -1;
    sheet->needs_rebuild = true;

    printf("Tile sheet initialized\n");
    return true;
}

/**
 * Cleanup tile sheet resources
 */
void tile_sheet_cleanup(TileSheet* sheet) {
    if (!sheet)
        return;

    // Destroy all textures
    for (int i = 0; i < 64; i++) {
        if (sheet->textures[i]) {
            SDL_DestroyTexture(sheet->textures[i]);
            sheet->textures[i] = NULL;
        }
    }

    printf("Tile sheet cleaned up\n");
}

/**
 * Generate texture for a single tile
 * Creates a 32x32 texture from 8x8 tile data (4x magnification)
 */
SDL_Texture* generate_tile_texture(SDL_Renderer* renderer, int tile_id) {
    if (!renderer || tile_id < 0 || tile_id >= 64) {
        return NULL;
    }

    // Create 32x32 texture for magnified tile display
    SDL_Texture* texture =
        SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                          TILE_DISPLAY_SIZE, TILE_DISPLAY_SIZE);
    if (!texture) {
        printf("Error creating tile texture: %s\n", SDL_GetError());
        return NULL;
    }

    // Set texture as render target
    SDL_Texture* original_target = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, texture);

    // Clear texture to transparent
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    // Draw each pixel as a 4x4 block (8x8 tile -> 32x32 display)
    for (int y = 0; y < TILE_HEIGHT; y++) {
        for (int x = 0; x < TILE_WIDTH; x++) {
            uint8_t palette_index = get_px(tile_id, x, y);
            SDL_Color color = palette_get_sdl_color(palette_index);

            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

            // Draw 4x4 pixel block
            SDL_FRect pixel_rect = {x * 4.0f, y * 4.0f, 4.0f, 4.0f};
            SDL_RenderFillRect(renderer, &pixel_rect);
        }
    }

    // Restore original render target
    SDL_SetRenderTarget(renderer, original_target);

    return texture;
}

/**
 * Update tile sheet textures
 * Regenerates textures for tiles marked as dirty
 */
void tile_sheet_update(TileSheet* sheet, SDL_Renderer* renderer) {
    if (!sheet || !renderer)
        return;

    // Check if we need to rebuild all textures
    if (sheet->needs_rebuild) {
        mark_all_tiles_dirty();
        sheet->needs_rebuild = false;
    }

    // Update dirty tiles
    for (int i = 0; i < 64; i++) {
        if (is_tile_dirty(i)) {
            // Destroy old texture if it exists
            if (sheet->textures[i]) {
                SDL_DestroyTexture(sheet->textures[i]);
            }

            // Generate new texture
            sheet->textures[i] = generate_tile_texture(renderer, i);

            // Clear dirty flag
            clear_tile_dirty(i);
        }
    }
}

/**
 * Render tile sheet panel
 */
void tile_sheet_render(TileSheet* sheet, SDL_Renderer* renderer, int x, int y) {
    if (!sheet || !renderer)
        return;

    // Draw background
    SDL_SetRenderDrawColor(renderer, 64, 64, 64, 255);
    SDL_FRect bg_rect = {x, y, TILE_SHEET_WIDTH, TILE_SHEET_HEIGHT};
    SDL_RenderFillRect(renderer, &bg_rect);

    // Draw tile textures
    for (int i = 0; i < 64; i++) {
        if (sheet->textures[i]) {
            SDL_FRect dest_rect = {x + sheet->tile_rects[i].x, y + sheet->tile_rects[i].y,
                                   TILE_DISPLAY_SIZE, TILE_DISPLAY_SIZE};
            SDL_RenderTexture(renderer, sheet->textures[i], NULL, &dest_rect);
        }
    }

    // Draw hover outline
    if (sheet->hover_tile >= 0) {
        SDL_SetRenderDrawColor(renderer, 100, 150, 255, 255);
        SDL_FRect hover_rect = {x + sheet->tile_rects[sheet->hover_tile].x,
                                y + sheet->tile_rects[sheet->hover_tile].y, TILE_DISPLAY_SIZE,
                                TILE_DISPLAY_SIZE};
        SDL_RenderRect(renderer, &hover_rect);
    }

    // Draw selection outline
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_FRect select_rect = {x + sheet->tile_rects[sheet->selected_tile].x,
                             y + sheet->tile_rects[sheet->selected_tile].y, TILE_DISPLAY_SIZE,
                             TILE_DISPLAY_SIZE};
    SDL_RenderRect(renderer, &select_rect);

    // Draw thick selection border
    select_rect.x -= 1;
    select_rect.y -= 1;
    select_rect.w += 2;
    select_rect.h += 2;
    SDL_RenderRect(renderer, &select_rect);
}

/**
 * Handle mouse input for tile sheet
 */
int tile_sheet_handle_input(TileSheet* sheet, int panel_x, int panel_y, int mouse_x, int mouse_y,
                            bool clicked, bool double_clicked) {
    if (!sheet)
        return -1;

    // Convert mouse coordinates to panel-relative coordinates
    int rel_x = mouse_x - panel_x;
    int rel_y = mouse_y - panel_y;

    // Check if mouse is within tile sheet bounds
    if (rel_x < 0 || rel_x >= TILE_SHEET_WIDTH || rel_y < 0 || rel_y >= TILE_SHEET_HEIGHT) {
        sheet->hover_tile = -1;
        return -1;
    }

    // Calculate which tile is under the mouse
    int tile_col = rel_x / TILE_DISPLAY_SIZE;
    int tile_row = rel_y / TILE_DISPLAY_SIZE;
    int tile_id = tile_row * TILE_SHEET_COLS + tile_col;

    // Clamp to valid range
    if (tile_id < 0 || tile_id >= 64) {
        sheet->hover_tile = -1;
        return -1;
    }

    // Update hover
    sheet->hover_tile = tile_id;

    // Handle clicks
    if (clicked || double_clicked) {
        sheet->selected_tile = tile_id;
        return tile_id;
    }

    return -1;
}

/**
 * Get currently selected tile
 */
int tile_sheet_get_selected(const TileSheet* sheet) {
    if (!sheet)
        return 0;
    return sheet->selected_tile;
}

/**
 * Set selected tile
 */
void tile_sheet_set_selected(TileSheet* sheet, int tile_id) {
    if (!sheet)
        return;

    // Clamp to valid range
    if (tile_id < 0)
        tile_id = 0;
    if (tile_id >= 64)
        tile_id = 63;

    sheet->selected_tile = tile_id;
}

/**
 * Navigate tile selection with keyboard
 */
void tile_sheet_navigate(TileSheet* sheet, int direction, bool horizontal) {
    if (!sheet)
        return;

    int new_tile = sheet->selected_tile;

    if (horizontal) {
        // Left/Right navigation
        new_tile += direction;

        // Wrap around at row boundaries
        int current_row = sheet->selected_tile / TILE_SHEET_COLS;
        int new_row = new_tile / TILE_SHEET_COLS;

        if (new_row != current_row) {
            if (direction > 0) {
                // Moving right, wrap to next row or first tile
                if (current_row < TILE_SHEET_ROWS - 1) {
                    new_tile = (current_row + 1) * TILE_SHEET_COLS;
                } else {
                    new_tile = 0;  // Wrap to first tile
                }
            } else {
                // Moving left, wrap to previous row or last tile
                if (current_row > 0) {
                    new_tile = current_row * TILE_SHEET_COLS - 1;
                } else {
                    new_tile = 63;  // Wrap to last tile
                }
            }
        }
    } else {
        // Up/Down navigation
        new_tile += direction * TILE_SHEET_COLS;

        // Wrap around at column boundaries
        if (new_tile < 0) {
            new_tile += 64;  // Wrap to bottom
        } else if (new_tile >= 64) {
            new_tile -= 64;  // Wrap to top
        }
    }

    // Clamp to valid range
    if (new_tile < 0)
        new_tile = 0;
    if (new_tile >= 64)
        new_tile = 63;

    sheet->selected_tile = new_tile;
}
