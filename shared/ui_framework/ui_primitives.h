#ifndef UI_PRIMITIVES_H
#define UI_PRIMITIVES_H

#include <SDL3/SDL.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * UI Primitives for Shared Component Library
 *
 * Provides basic rendering utilities for UI components including
 * rectangle rendering, coordinate utilities, and common drawing functions.
 * Extracted from palette-maker and tile-maker for shared use.
 */

/**
 * Render filled rectangle with specified color
 *
 * @param renderer SDL renderer
 * @param x X coordinate
 * @param y Y coordinate
 * @param w Width
 * @param h Height
 * @param color Fill color
 */
void ui_render_rect(SDL_Renderer* renderer, int x, int y, int w, int h, SDL_Color color);

/**
 * Render rectangle outline with specified color
 *
 * @param renderer SDL renderer
 * @param x X coordinate
 * @param y Y coordinate
 * @param w Width
 * @param h Height
 * @param color Border color
 */
void ui_render_rect_outline(SDL_Renderer* renderer, int x, int y, int w, int h, SDL_Color color);

/**
 * Render filled rectangle using floating point coordinates
 *
 * @param renderer SDL renderer
 * @param rect Rectangle structure with float coordinates
 * @param color Fill color
 */
void ui_render_rect_f(SDL_Renderer* renderer, const SDL_FRect* rect, SDL_Color color);

/**
 * Render rectangle outline using floating point coordinates
 *
 * @param renderer SDL renderer
 * @param rect Rectangle structure with float coordinates
 * @param color Border color
 */
void ui_render_rect_outline_f(SDL_Renderer* renderer, const SDL_FRect* rect, SDL_Color color);

/**
 * Check if a point is inside a rectangle
 *
 * @param x Point X coordinate
 * @param y Point Y coordinate
 * @param rect Rectangle to check against
 * @return true if point is inside rectangle
 */
bool ui_point_in_rect(int x, int y, const SDL_FRect* rect);

/**
 * Check if a point is inside an integer rectangle
 *
 * @param x Point X coordinate
 * @param y Point Y coordinate
 * @param rx Rectangle X coordinate
 * @param ry Rectangle Y coordinate
 * @param rw Rectangle width
 * @param rh Rectangle height
 * @return true if point is inside rectangle
 */
bool ui_point_in_rect_i(int x, int y, int rx, int ry, int rw, int rh);

/**
 * Create SDL_FRect structure from integer coordinates
 *
 * @param x X coordinate
 * @param y Y coordinate
 * @param w Width
 * @param h Height
 * @return SDL_FRect structure
 */
SDL_FRect ui_make_rect(int x, int y, int w, int h);

/**
 * Create SDL_FRect structure from float coordinates
 *
 * @param x X coordinate
 * @param y Y coordinate
 * @param w Width
 * @param h Height
 * @return SDL_FRect structure
 */
SDL_FRect ui_make_rect_f(float x, float y, float w, float h);

/**
 * Expand rectangle by specified amount in all directions
 *
 * @param rect Rectangle to expand
 * @param amount Amount to expand by (positive = larger, negative = smaller)
 * @return Expanded rectangle
 */
SDL_FRect ui_expand_rect(const SDL_FRect* rect, int amount);

/**
 * Check if two rectangles intersect
 *
 * @param rect1 First rectangle
 * @param rect2 Second rectangle
 * @return true if rectangles intersect
 */
bool ui_rects_intersect(const SDL_FRect* rect1, const SDL_FRect* rect2);

/**
 * Clamp integer value to specified range
 *
 * @param value Value to clamp
 * @param min Minimum value
 * @param max Maximum value
 * @return Clamped value
 */
int ui_clamp_int(int value, int min, int max);

/**
 * Clamp float value to specified range
 *
 * @param value Value to clamp
 * @param min Minimum value
 * @param max Maximum value
 * @return Clamped value
 */
float ui_clamp_float(float value, float min, float max);

#ifdef __cplusplus
}
#endif

#endif  // UI_PRIMITIVES_H
