#ifndef UI_VIEWPORT_H
#define UI_VIEWPORT_H

#include <SDL3/SDL.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Convert window-space coordinates to renderer logical coordinates.
 *
 * Returns true when SDL performed the conversion, false when fallback was used.
 * On fallback, output coordinates are set to the input coordinates.
 */
bool ui_viewport_window_to_logical(SDL_Renderer* renderer, float window_x, float window_y,
                                   float* logical_x, float* logical_y);

/**
 * Convert renderer logical coordinates to window-space coordinates.
 *
 * Returns true when SDL performed the conversion, false when fallback was used.
 * On fallback, output coordinates are set to the input coordinates.
 */
bool ui_viewport_logical_to_window(SDL_Renderer* renderer, float logical_x, float logical_y,
                                   float* window_x, float* window_y);

/**
 * Get effective logical presentation scale.
 *
 * Returns true when scale could be derived from renderer logical presentation settings.
 * On failure, outputs are set to 1.0f.
 */
bool ui_viewport_get_scale(SDL_Renderer* renderer, float* scale_x, float* scale_y);

#ifdef __cplusplus
}
#endif

#endif  // UI_VIEWPORT_H
