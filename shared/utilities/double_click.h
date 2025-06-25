#ifndef DOUBLE_CLICK_H
#define DOUBLE_CLICK_H

#include <SDL3/SDL.h>
#include <stdbool.h>

/**
 * Double-Click Detection Utility for Shared Component Library
 *
 * Provides unified double-click detection functionality extracted from
 * palette-maker and tile-maker with configurable timing thresholds.
 */

/**
 * Double-click detector structure
 * Tracks click timing and target identification
 */
typedef struct {
    Uint64 last_click_time;   // Time of last click in milliseconds
    int last_clicked_target;  // ID of last clicked target
    Uint32 threshold_ms;      // Double-click threshold in milliseconds
} DoubleClickDetector;

/**
 * Initialize double-click detector
 *
 * @param detector Double-click detector to initialize
 * @param threshold_ms Double-click threshold in milliseconds (0 for default)
 */
void double_click_init(DoubleClickDetector* detector, Uint32 threshold_ms);

/**
 * Check for double-click
 * Call this when a click occurs to check if it's a double-click
 *
 * @param detector Double-click detector
 * @param target_id ID of clicked target (e.g., button ID, swatch index)
 * @return true if this click is a double-click, false otherwise
 */
bool double_click_check(DoubleClickDetector* detector, int target_id);

/**
 * Reset double-click detector
 * Clears timing and target information
 *
 * @param detector Double-click detector to reset
 */
void double_click_reset(DoubleClickDetector* detector);

/**
 * Set double-click threshold
 *
 * @param detector Double-click detector
 * @param threshold_ms New threshold in milliseconds
 */
void double_click_set_threshold(DoubleClickDetector* detector, Uint32 threshold_ms);

/**
 * Get current double-click threshold
 *
 * @param detector Double-click detector
 * @return Current threshold in milliseconds
 */
Uint32 double_click_get_threshold(const DoubleClickDetector* detector);

/**
 * Get time since last click
 *
 * @param detector Double-click detector
 * @return Milliseconds since last click, 0 if no previous click
 */
Uint64 double_click_get_time_since_last(const DoubleClickDetector* detector);

/**
 * Check if detector has recorded a previous click
 *
 * @param detector Double-click detector
 * @return true if there was a previous click recorded
 */
bool double_click_has_previous(const DoubleClickDetector* detector);

// Default double-click thresholds (milliseconds)
#define DOUBLE_CLICK_THRESHOLD_FAST 200    // Fast double-click
#define DOUBLE_CLICK_THRESHOLD_NORMAL 300  // Normal double-click (palette-maker default)
#define DOUBLE_CLICK_THRESHOLD_SLOW 500    // Slow double-click (tile-maker default)

#endif  // DOUBLE_CLICK_H
