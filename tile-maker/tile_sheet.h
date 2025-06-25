#ifndef TILE_SHEET_H
#define TILE_SHEET_H

#include <SDL3/SDL.h>
#include <stdbool.h>

/**
 * Tile sheet panel constants
 */
#define TILE_SHEET_WIDTH 256
#define TILE_SHEET_HEIGHT 256
#define TILE_SHEET_COLS 8
#define TILE_SHEET_ROWS 8
#define TILE_DISPLAY_SIZE 32  // Each tile displayed as 32x32 pixels (4x magnification)

/**
 * Tile sheet state structure
 */
typedef struct {
    SDL_Texture* textures[64];  // One texture per tile (32x32 pixels each)
    SDL_Rect tile_rects[64];    // Position rectangles for each tile
    int selected_tile;          // Currently selected tile (0-63)
    int hover_tile;             // Tile being hovered over (-1 if none)
    bool needs_rebuild;         // Flag to rebuild all textures
} TileSheet;

/**
 * Initialize tile sheet system
 * Creates textures and position rectangles for all tiles
 *
 * @param sheet Pointer to tile sheet structure
 * @param renderer SDL renderer for creating textures
 * @return true if successful, false on error
 */
bool tile_sheet_init(TileSheet* sheet, SDL_Renderer* renderer);

/**
 * Cleanup tile sheet resources
 * Destroys all tile textures
 *
 * @param sheet Pointer to tile sheet structure
 */
void tile_sheet_cleanup(TileSheet* sheet);

/**
 * Update tile sheet textures
 * Regenerates textures for tiles marked as dirty
 *
 * @param sheet Pointer to tile sheet structure
 * @param renderer SDL renderer for updating textures
 */
void tile_sheet_update(TileSheet* sheet, SDL_Renderer* renderer);

/**
 * Render tile sheet panel
 * Draws all tile thumbnails with selection and hover indicators
 *
 * @param sheet Pointer to tile sheet structure
 * @param renderer SDL renderer
 * @param x Panel x position
 * @param y Panel y position
 */
void tile_sheet_render(TileSheet* sheet, SDL_Renderer* renderer, int x, int y);

/**
 * Handle mouse input for tile sheet
 * Processes clicks and hover for tile selection
 *
 * @param sheet Pointer to tile sheet structure
 * @param x Panel x position
 * @param y Panel y position
 * @param mouse_x Mouse x coordinate
 * @param mouse_y Mouse y coordinate
 * @param clicked True if mouse was clicked
 * @param double_clicked True if mouse was double-clicked
 * @return Selected tile ID if tile was clicked, -1 otherwise
 */
int tile_sheet_handle_input(TileSheet* sheet, int x, int y, int mouse_x, int mouse_y, bool clicked,
                            bool double_clicked);

/**
 * Get currently selected tile
 *
 * @param sheet Pointer to tile sheet structure
 * @return Selected tile ID (0-63)
 */
int tile_sheet_get_selected(const TileSheet* sheet);

/**
 * Set selected tile
 *
 * @param sheet Pointer to tile sheet structure
 * @param tile_id Tile ID to select (0-63)
 */
void tile_sheet_set_selected(TileSheet* sheet, int tile_id);

/**
 * Navigate tile selection with keyboard
 *
 * @param sheet Pointer to tile sheet structure
 * @param direction Direction to move (-1 for left/up, +1 for right/down)
 * @param horizontal True for horizontal navigation, false for vertical
 */
void tile_sheet_navigate(TileSheet* sheet, int direction, bool horizontal);

/**
 * Generate texture for a single tile
 * Creates a 32x32 texture from 8x8 tile data (4x magnification)
 *
 * @param renderer SDL renderer
 * @param tile_id Tile ID (0-63)
 * @return New SDL_Texture or NULL on error
 */
SDL_Texture* generate_tile_texture(SDL_Renderer* renderer, int tile_id);

#endif  // TILE_SHEET_H
