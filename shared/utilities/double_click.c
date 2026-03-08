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
    bool has_previous = detector->last_clicked_target >= 0;

    if (!has_previous) {
        detector->last_click_time = current_time;
        detector->last_clicked_target = target_id;
        return false;
    }

    // SDL ticks are monotonic, but guard against unexpected backwards values.
    bool time_ordered = current_time >= detector->last_click_time;
    Uint64 elapsed = current_time - detector->last_click_time;
    bool same_target = detector->last_clicked_target == target_id;
    bool within_threshold = time_ordered && elapsed < detector->threshold_ms;

    if (same_target && within_threshold) {
        // A completed double-click consumes the pending state.
        detector->last_click_time = 0;
        detector->last_clicked_target = -1;
        return true;
    }

    if (!same_target) {
        // Different target starts a new sequence immediately.
        detector->last_click_time = current_time;
        detector->last_clicked_target = target_id;
        return false;
    }

    // Same target but outside threshold clears stale pending state.
    detector->last_click_time = 0;
    detector->last_clicked_target = -1;
    return false;
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
    return detector && detector->last_clicked_target >= 0;
}
