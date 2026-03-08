#ifndef UI_INPUT_WIDGETS_H
#define UI_INPUT_WIDGETS_H

#include <SDL3/SDL.h>
#include "ui_input.h"

#ifdef __cplusplus
extern "C" {
#endif

SDL_Color ui_input_widgets_blend_color(SDL_Color a, SDL_Color b, float t);
SDL_Color ui_input_widgets_darken(SDL_Color c, float factor);
SDL_Color ui_input_widgets_button_color(const UIInputElement* element, SDL_Color base,
                                        SDL_Color hover, float pressed_factor);

void ui_input_widgets_render_button(SDL_Renderer* renderer, const UIInputElement* element,
                                    SDL_Color base, SDL_Color hover, SDL_Color border,
                                    float pressed_factor);

#ifdef __cplusplus
}
#endif

#endif  // UI_INPUT_WIDGETS_H
