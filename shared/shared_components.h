#ifndef SHARED_COMPONENTS_H
#define SHARED_COMPONENTS_H

/**
 * Shared Components Library
 *
 * This library provides reusable components extracted from palette-maker and tile-maker
 * applications. It includes text rendering, UI framework, palette management, SDL3
 * integration, and common utilities.
 *
 * Usage:
 *   #include <shared_components.h>
 *
 * Components:
 *   - Text Renderer: 5x7 bitmap font with 7-segment display support
 *   - UI Framework: Buttons, primitives, and input handling
 *   - Palette Manager: Unified 16-color RGBA palette handling
 *   - SDL Framework: SDL3 context management and utilities
 *   - Utilities: Double-click detection, file operations
 */

// Core SDL3 dependency
#include <SDL3/SDL.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Text rendering system
#include "text_renderer/font_data.h"
#include "text_renderer/text_renderer.h"

// UI framework components
#include "ui_framework/ui_button.h"
#include "ui_framework/ui_primitives.h"

// Palette management
#include "palette_manager/palette_manager.h"

// SDL3 framework
#include "sdl_framework/sdl_context.h"

// Utilities
#include "utilities/double_click.h"
#include "utilities/file_utils.h"

/**
 * Shared Components Library Information
 */
#define SHARED_COMPONENTS_VERSION_MAJOR 1
#define SHARED_COMPONENTS_VERSION_MINOR 0
#define SHARED_COMPONENTS_VERSION_PATCH 0

/**
 * Get library version string
 * @return Version string in format "major.minor.patch"
 */
const char* shared_components_get_version(void);

/**
 * Initialize shared components library
 * Call this before using any shared components
 *
 * @return true if successful, false on error
 */
bool shared_components_init(void);

/**
 * Cleanup shared components library
 * Call this when shutting down application
 */
void shared_components_cleanup(void);

/**
 * Check if shared components library is initialized
 *
 * @return true if initialized and ready to use
 */
bool shared_components_is_initialized(void);

#ifdef __cplusplus
}
#endif

#endif  // SHARED_COMPONENTS_H
