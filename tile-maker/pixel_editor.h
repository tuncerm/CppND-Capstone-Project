#ifndef PIXEL_EDITOR_H
#define PIXEL_EDITOR_H

#include <SDL3/SDL.h>
#include <stdbool.h>

/**
 * Pixel editor panel constants
 */
#define PIXEL_EDITOR_WIDTH 256
#define PIXEL_EDITOR_HEIGHT 256
#define PIXEL_SIZE 32  // Each pixel displayed as 32x32 (8x8 tile -> 256x256 display)

/**
 * Pixel editor state structure
 */
typedef struct {
    SDL_Texture* tile_texture;  // Current tile texture (256x256)
    SDL_Texture* grid_texture;  // Grid overlay texture
    int current_tile;           // Currently edited tile ID (0-63)
    int current_color;          // Current paint color (palette index 0-15)
    bool show_grid;             // Grid overlay visibility
    bool needs_rebuild;         // Flag to rebuild tile texture
    bool dragging;              // Mouse drag state for painting
} PixelEditor;

/**
 * Initialize pixel editor system
 * Creates grid texture and sets up initial state
 *
 * @param editor Pointer to pixel editor structure
 * @param renderer SDL renderer for creating textures
 * @return true if successful, false on error
 */
bool pixel_editor_init(PixelEditor* editor, SDL_Renderer* renderer);

/**
 * Cleanup pixel editor resources
 * Destroys textures and cleans up resources
 *
 * @param editor Pointer to pixel editor structure
 */
void pixel_editor_cleanup(PixelEditor* editor);

/**
 * Set the tile to edit
 * Changes the current tile and rebuilds the texture
 *
 * @param editor Pointer to pixel editor structure
 * @param tile_id Tile ID to edit (0-63)
 */
void pixel_editor_set_tile(PixelEditor* editor, int tile_id);

/**
 * Set the current paint color
 *
 * @param editor Pointer to pixel editor structure
 * @param palette_index Palette index to use for painting (0-15)
 */
void pixel_editor_set_color(PixelEditor* editor, int palette_index);

/**
 * Toggle grid overlay visibility
 *
 * @param editor Pointer to pixel editor structure
 */
void pixel_editor_toggle_grid(PixelEditor* editor);

/**
 * Update pixel editor
 * Rebuilds tile texture if needed
 *
 * @param editor Pointer to pixel editor structure
 * @param renderer SDL renderer for updating textures
 */
void pixel_editor_update(PixelEditor* editor, SDL_Renderer* renderer);

/**
 * Render pixel editor panel
 * Draws the magnified tile with optional grid overlay
 *
 * @param editor Pointer to pixel editor structure
 * @param renderer SDL renderer
 * @param x Panel x position
 * @param y Panel y position
 */
void pixel_editor_render(PixelEditor* editor, SDL_Renderer* renderer, int x, int y);

/**
 * Handle mouse input for pixel editor
 * Processes painting and color picking
 *
 * @param editor Pointer to pixel editor structure
 * @param x Panel x position
 * @param y Panel y position
 * @param mouse_x Mouse x coordinate
 * @param mouse_y Mouse y coordinate
 * @param left_button Left mouse button state
 * @param right_button Right mouse button state
 * @param mouse_down Mouse button down state
 * @return true if pixel was modified, false otherwise
 */
bool pixel_editor_handle_input(PixelEditor* editor, int x, int y, int mouse_x, int mouse_y,
                               bool left_button, bool right_button, bool mouse_down);

/**
 * Get current tile being edited
 *
 * @param editor Pointer to pixel editor structure
 * @return Current tile ID (0-63)
 */
int pixel_editor_get_tile(const PixelEditor* editor);

/**
 * Get current paint color
 *
 * @param editor Pointer to pixel editor structure
 * @return Current palette index (0-15)
 */
int pixel_editor_get_color(const PixelEditor* editor);

/**
 * Check if grid is visible
 *
 * @param editor Pointer to pixel editor structure
 * @return true if grid is visible, false otherwise
 */
bool pixel_editor_grid_visible(const PixelEditor* editor);

/**
 * Generate pixel editor texture for current tile
 * Creates a 256x256 texture from 8x8 tile data (32x magnification)
 *
 * @param renderer SDL renderer
 * @param tile_id Tile ID (0-63)
 * @return New SDL_Texture or NULL on error
 */
SDL_Texture* generate_pixel_editor_texture(SDL_Renderer* renderer, int tile_id);

/**
 * Generate grid overlay texture
 * Creates a 256x256 texture with 8x8 grid lines
 *
 * @param renderer SDL renderer
 * @return New SDL_Texture or NULL on error
 */
SDL_Texture* generate_grid_texture(SDL_Renderer* renderer);

#endif  // PIXEL_EDITOR_H
