#include "ui_input.h"

static float ui_clamp01(float value) {
    if (value < 0.0f) {
        return 0.0f;
    }
    if (value > 1.0f) {
        return 1.0f;
    }
    return value;
}

static float ui_approach(float current, float target, float step) {
    if (current < target) {
        current += step;
        if (current > target) {
            current = target;
        }
    } else if (current > target) {
        current -= step;
        if (current < target) {
            current = target;
        }
    }
    return current;
}

void ui_input_init(UIInputElement* element, int id, SDL_FRect bounds) {
    if (!element) {
        return;
    }

    element->id = id;
    element->bounds = bounds;
    element->enabled = true;

    element->hovered = false;
    element->pressed = false;
    element->selected = false;

    element->hover_anim_t = 0.0f;
    element->selected_anim_t = 0.0f;
    element->animation_speed = 8.0f;

    element->value = 0.0f;
    element->last_value = 0.0f;

    element->on_click = 0;
    element->on_change = 0;
    element->on_hover = 0;
    element->on_selected = 0;
    element->userdata = 0;
}

void ui_input_set_callbacks(UIInputElement* element, UIInputOnClick on_click,
                            UIInputOnChange on_change, void* userdata) {
    if (!element) {
        return;
    }

    element->on_click = on_click;
    element->on_change = on_change;
    element->userdata = userdata;
}

void ui_input_set_state_callbacks(UIInputElement* element, UIInputOnHover on_hover,
                                  UIInputOnSelected on_selected) {
    if (!element) {
        return;
    }

    element->on_hover = on_hover;
    element->on_selected = on_selected;
}

void ui_input_set_enabled(UIInputElement* element, bool enabled) {
    if (!element) {
        return;
    }

    element->enabled = enabled;
    if (!enabled) {
        if (element->hovered && element->on_hover) {
            element->on_hover(element->id, false, element->userdata);
        }
        element->hovered = false;
        element->pressed = false;
    }
}

void ui_input_set_selected(UIInputElement* element, bool selected) {
    if (!element) {
        return;
    }
    if (element->selected == selected) {
        return;
    }
    element->selected = selected;
    if (element->on_selected) {
        element->on_selected(element->id, selected, element->userdata);
    }
}

void ui_input_set_value(UIInputElement* element, float value) {
    if (!element) {
        return;
    }

    element->last_value = element->value;
    element->value = value;
    if (element->on_change && element->value != element->last_value) {
        element->on_change(element->id, element->value, element->userdata);
    }
}

bool ui_input_contains(const UIInputElement* element, float x, float y) {
    if (!element) {
        return false;
    }

    return x >= element->bounds.x && x <= element->bounds.x + element->bounds.w &&
           y >= element->bounds.y && y <= element->bounds.y + element->bounds.h;
}

bool ui_input_update(UIInputElement* element, float dt_seconds, float mouse_x, float mouse_y,
                     bool mouse_down, bool mouse_pressed, bool mouse_released) {
    if (!element || !element->enabled) {
        return false;
    }

    const bool contains_mouse = ui_input_contains(element, mouse_x, mouse_y);
    const bool was_hovered = element->hovered;
    bool clicked = false;

    element->hovered = contains_mouse;
    if (was_hovered != element->hovered && element->on_hover) {
        element->on_hover(element->id, element->hovered, element->userdata);
    }

    if (mouse_pressed && contains_mouse) {
        element->pressed = true;
    }

    if (mouse_released) {
        if (element->pressed && contains_mouse) {
            clicked = true;
            if (element->on_click) {
                element->on_click(element->id, element->userdata);
            }
        }
        element->pressed = false;
    } else if (!mouse_down && !contains_mouse) {
        element->pressed = false;
    }

    if (dt_seconds < 0.0f) {
        dt_seconds = 0.0f;
    }

    const float step = ui_clamp01(dt_seconds * element->animation_speed);
    const float hover_target = element->hovered ? 1.0f : 0.0f;
    const float selected_target = element->selected ? 1.0f : 0.0f;

    element->hover_anim_t = ui_approach(element->hover_anim_t, hover_target, step);
    element->selected_anim_t = ui_approach(element->selected_anim_t, selected_target, step);

    return clicked;
}
