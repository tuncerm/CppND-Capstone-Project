#include "ui_viewport.h"

bool ui_viewport_window_to_logical(SDL_Renderer* renderer, float window_x, float window_y,
                                   float* logical_x, float* logical_y) {
    if (!logical_x || !logical_y) {
        return false;
    }

    *logical_x = window_x;
    *logical_y = window_y;

    if (!renderer) {
        return false;
    }

    return SDL_RenderCoordinatesFromWindow(renderer, window_x, window_y, logical_x, logical_y);
}

bool ui_viewport_logical_to_window(SDL_Renderer* renderer, float logical_x, float logical_y,
                                   float* window_x, float* window_y) {
    if (!window_x || !window_y) {
        return false;
    }

    *window_x = logical_x;
    *window_y = logical_y;

    if (!renderer) {
        return false;
    }

    return SDL_RenderCoordinatesToWindow(renderer, logical_x, logical_y, window_x, window_y);
}

bool ui_viewport_get_scale(SDL_Renderer* renderer, float* scale_x, float* scale_y) {
    if (!scale_x || !scale_y) {
        return false;
    }

    *scale_x = 1.0f;
    *scale_y = 1.0f;

    if (!renderer) {
        return false;
    }

    int logical_w = 0;
    int logical_h = 0;
    SDL_RendererLogicalPresentation mode = SDL_LOGICAL_PRESENTATION_DISABLED;
    if (!SDL_GetRenderLogicalPresentation(renderer, &logical_w, &logical_h, &mode)) {
        return false;
    }

    if (mode == SDL_LOGICAL_PRESENTATION_DISABLED || logical_w <= 0 || logical_h <= 0) {
        return false;
    }

    SDL_FRect logical_rect = {0};
    if (!SDL_GetRenderLogicalPresentationRect(renderer, &logical_rect)) {
        return false;
    }

    if (logical_rect.w <= 0.0f || logical_rect.h <= 0.0f) {
        return false;
    }

    *scale_x = logical_rect.w / (float)logical_w;
    *scale_y = logical_rect.h / (float)logical_h;
    return true;
}
