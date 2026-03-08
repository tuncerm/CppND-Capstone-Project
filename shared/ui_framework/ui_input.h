#ifndef UI_INPUT_H
#define UI_INPUT_H

#include <SDL3/SDL.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*UIInputOnClick)(int id, void* userdata);
typedef void (*UIInputOnChange)(int id, float value, void* userdata);

typedef struct {
    int id;
    SDL_FRect bounds;
    bool enabled;

    bool hovered;
    bool pressed;
    bool selected;

    float hover_anim_t;
    float selected_anim_t;
    float animation_speed;

    float value;
    float last_value;

    UIInputOnClick on_click;
    UIInputOnChange on_change;
    void* userdata;
} UIInputElement;

void ui_input_init(UIInputElement* element, int id, SDL_FRect bounds);
void ui_input_set_callbacks(UIInputElement* element, UIInputOnClick on_click,
                            UIInputOnChange on_change, void* userdata);
void ui_input_set_enabled(UIInputElement* element, bool enabled);
void ui_input_set_selected(UIInputElement* element, bool selected);
void ui_input_set_value(UIInputElement* element, float value);

bool ui_input_contains(const UIInputElement* element, float x, float y);
bool ui_input_update(UIInputElement* element, float dt_seconds, float mouse_x, float mouse_y,
                     bool mouse_down, bool mouse_pressed, bool mouse_released);

#ifdef __cplusplus
}
#endif

#endif  // UI_INPUT_H
