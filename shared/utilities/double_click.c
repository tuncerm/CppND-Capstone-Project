#include "double_click.h"

/**
 * Initialize double-click detector
 */
void double_click_init(DoubleClickDetector* detector, Uint32 threshold_ms) {
    if (!detector) {
        return;
    }

    detector->last_click_time = 0;
    detector->last_clicked_target = -1;

    // Use provided threshold or default
    if (threshold_ms == 0) {
        detector->threshold_ms = DOUBLE_CLICK_THRESHOLD_NORMAL;
    } else {
        detector->threshold_ms = threshold_ms;
    }
}

/**
 * Check for double-click
 */
bool double_click_check(DoubleClickDetector* detector, int target_id) {
    if (!detector) {
        return false;
    }

    Uint64 current_time = SDL_GetTicks();
    bool is_double_click = false;

    // Check if this is a double-click
    if (detector->last_clicked_target == target_id && detector->last_click_time > 0 &&
        (current_time - detector->last_click_time) < detector->threshold_ms) {
        is_double_click = true;
    }

    // Update state for next click
    detector->last_click_time = current_time;
    detector->last_clicked_target = target_id;

    return is_double_click;
}

/**
 * Reset double-click detector
 */
void double_click_reset(DoubleClickDetector* detector) {
    if (!detector) {
        return;
    }

    detector->last_click_time = 0;
    detector->last_clicked_target = -1;
}

/**
 * Set double-click threshold
 */
void double_click_set_threshold(DoubleClickDetector* detector, Uint32 threshold_ms) {
    if (!detector) {
        return;
    }

    detector->threshold_ms = threshold_ms;
}

/**
 * Get current double-click threshold
 */
Uint32 double_click_get_threshold(const DoubleClickDetector* detector) {
    if (!detector) {
        return 0;
    }

    return detector->threshold_ms;
}

/**
 * Get time since last click
 */
Uint64 double_click_get_time_since_last(const DoubleClickDetector* detector) {
    if (!detector || detector->last_click_time == 0) {
        return 0;
    }

    Uint64 current_time = SDL_GetTicks();
    return current_time - detector->last_click_time;
}

/**
 * Check if detector has recorded a previous click
 */
bool double_click_has_previous(const DoubleClickDetector* detector) {
    return detector && detector->last_click_time > 0;
}
