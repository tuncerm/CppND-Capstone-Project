#include "ui_primitives.h"

/**
 * Render filled rectangle with specified color
 */
void ui_render_rect(SDL_Renderer* renderer, int x, int y, int w, int h, SDL_Color color) {
    if (!renderer) {
        return;
    }

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_FRect rect = {(float)x, (float)y, (float)w, (float)h};
    SDL_RenderFillRect(renderer, &rect);
}

/**
 * Render rectangle outline with specified color
 */
void ui_render_rect_outline(SDL_Renderer* renderer, int x, int y, int w, int h, SDL_Color color) {
    if (!renderer) {
        return;
    }

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_FRect rect = {(float)x, (float)y, (float)w, (float)h};
    SDL_RenderRect(renderer, &rect);
}

/**
 * Render filled rectangle using floating point coordinates
 */
void ui_render_rect_f(SDL_Renderer* renderer, const SDL_FRect* rect, SDL_Color color) {
    if (!renderer || !rect) {
        return;
    }

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, rect);
}

/**
 * Render rectangle outline using floating point coordinates
 */
void ui_render_rect_outline_f(SDL_Renderer* renderer, const SDL_FRect* rect, SDL_Color color) {
    if (!renderer || !rect) {
        return;
    }

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderRect(renderer, rect);
}

/**
 * Check if a point is inside a rectangle
 */
bool ui_point_in_rect(int x, int y, const SDL_FRect* rect) {
    if (!rect) {
        return false;
    }

    return (x >= rect->x && x < rect->x + rect->w && y >= rect->y && y < rect->y + rect->h);
}

/**
 * Check if a point is inside an integer rectangle
 */
bool ui_point_in_rect_i(int x, int y, int rx, int ry, int rw, int rh) {
    return (x >= rx && x < rx + rw && y >= ry && y < ry + rh);
}

/**
 * Create SDL_FRect structure from integer coordinates
 */
SDL_FRect ui_make_rect(int x, int y, int w, int h) {
    SDL_FRect rect = {(float)x, (float)y, (float)w, (float)h};
    return rect;
}

/**
 * Create SDL_FRect structure from float coordinates
 */
SDL_FRect ui_make_rect_f(float x, float y, float w, float h) {
    SDL_FRect rect = {x, y, w, h};
    return rect;
}

/**
 * Expand rectangle by specified amount in all directions
 */
SDL_FRect ui_expand_rect(const SDL_FRect* rect, int amount) {
    if (!rect) {
        SDL_FRect empty = {0, 0, 0, 0};
        return empty;
    }

    SDL_FRect expanded = {rect->x - amount, rect->y - amount, rect->w + 2 * amount,
                          rect->h + 2 * amount};
    return expanded;
}

/**
 * Check if two rectangles intersect
 */
bool ui_rects_intersect(const SDL_FRect* rect1, const SDL_FRect* rect2) {
    if (!rect1 || !rect2) {
        return false;
    }

    return !(rect1->x >= rect2->x + rect2->w || rect2->x >= rect1->x + rect1->w ||
             rect1->y >= rect2->y + rect2->h || rect2->y >= rect1->y + rect1->h);
}

/**
 * Clamp integer value to specified range
 */
int ui_clamp_int(int value, int min, int max) {
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}

/**
 * Clamp float value to specified range
 */
float ui_clamp_float(float value, float min, float max) {
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}
