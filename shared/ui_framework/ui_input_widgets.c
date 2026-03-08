#include "ui_input_widgets.h"
#include "ui_primitives.h"

static Uint8 ui_input_widgets_lerp_u8(Uint8 a, Uint8 b, float t) {
    if (t < 0.0f) {
        t = 0.0f;
    } else if (t > 1.0f) {
        t = 1.0f;
    }

    return (Uint8)((float)a + ((float)b - (float)a) * t);
}

SDL_Color ui_input_widgets_blend_color(SDL_Color a, SDL_Color b, float t) {
    SDL_Color out = {ui_input_widgets_lerp_u8(a.r, b.r, t), ui_input_widgets_lerp_u8(a.g, b.g, t),
                     ui_input_widgets_lerp_u8(a.b, b.b, t), ui_input_widgets_lerp_u8(a.a, b.a, t)};
    return out;
}

SDL_Color ui_input_widgets_darken(SDL_Color c, float factor) {
    if (factor < 0.0f) {
        factor = 0.0f;
    } else if (factor > 1.0f) {
        factor = 1.0f;
    }

    SDL_Color out = {(Uint8)((float)c.r * factor), (Uint8)((float)c.g * factor),
                     (Uint8)((float)c.b * factor), c.a};
    return out;
}

SDL_Color ui_input_widgets_button_color(const UIInputElement* element, SDL_Color base,
                                        SDL_Color hover, float pressed_factor) {
    SDL_Color color = base;
    if (!element) {
        return color;
    }

    color = ui_input_widgets_blend_color(base, hover, element->hover_anim_t);

    if (!element->enabled) {
        color = ui_input_widgets_darken(color, 0.55f);
    } else if (element->pressed) {
        color = ui_input_widgets_darken(color, pressed_factor);
    }

    return color;
}

void ui_input_widgets_render_button(SDL_Renderer* renderer, const UIInputElement* element,
                                    SDL_Color base, SDL_Color hover, SDL_Color border,
                                    float pressed_factor) {
    if (!renderer || !element) {
        return;
    }

    SDL_Color color = ui_input_widgets_button_color(element, base, hover, pressed_factor);
    ui_render_rect_f(renderer, &element->bounds, color);
    ui_render_rect_outline_f(renderer, &element->bounds, border);
}
