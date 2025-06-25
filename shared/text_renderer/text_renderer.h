#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <SDL3/SDL.h>
#include <stdbool.h>
#include "font_data.h"

/**
 * Text Renderer for Shared Component Library
 *
 * Provides unified text rendering capabilities using 5x7 bitmap font
 * with support for regular text and 7-segment display styles.
 * Extracted from palette-maker for reuse across applications.
 */

/**
 * Text renderer context structure
 * Holds SDL renderer reference and rendering state
 */
typedef struct TextRenderer {
    SDL_Renderer* renderer;
    bool initialized;
    SDL_Color default_color;
} TextRenderer;

/**
 * 7-segment display digit patterns
 * Used for numeric displays with classic 7-segment appearance
 */
typedef enum {
    SEGMENT_A = 0x01,  // Top
    SEGMENT_B = 0x02,  // Top right
    SEGMENT_C = 0x04,  // Bottom right
    SEGMENT_D = 0x08,  // Bottom
    SEGMENT_E = 0x10,  // Bottom left
    SEGMENT_F = 0x20,  // Top left
    SEGMENT_G = 0x40   // Middle
} SevenSegmentFlags;

// ===== Core Text Rendering Functions =====

/**
 * Initialize text renderer
 *
 * @param tr Text renderer context
 * @param renderer SDL renderer to use for drawing
 * @return true if successful, false on error
 */
bool text_renderer_init(TextRenderer* tr, SDL_Renderer* renderer);

/**
 * Cleanup text renderer resources
 *
 * @param tr Text renderer context
 */
void text_renderer_cleanup(TextRenderer* tr);

/**
 * Set default text color
 *
 * @param tr Text renderer context
 * @param color Default text color
 */
void text_renderer_set_default_color(TextRenderer* tr, SDL_Color color);

/**
 * Render text string at specified position
 *
 * @param tr Text renderer context
 * @param text Text string to render (max 32 characters)
 * @param x X coordinate
 * @param y Y coordinate
 * @param color Text color
 */
void text_render_string(TextRenderer* tr, const char* text, int x, int y, SDL_Color color);

/**
 * Render text string using default color
 *
 * @param tr Text renderer context
 * @param text Text string to render
 * @param x X coordinate
 * @param y Y coordinate
 */
void text_render_string_default(TextRenderer* tr, const char* text, int x, int y);

/**
 * Get text dimensions for layout calculations
 *
 * @param text Text string to measure
 * @param width Output: calculated width in pixels
 * @param height Output: calculated height in pixels
 */
void text_get_dimensions(const char* text, int* width, int* height);

// ===== 7-Segment Display Functions =====

/**
 * Render single digit in 7-segment display style
 *
 * @param tr Text renderer context
 * @param digit Character digit '0'-'9' or space for blank
 * @param x X coordinate
 * @param y Y coordinate
 * @param color Segment color
 * @param scale Scale factor (1 = normal size, 2 = double size, etc.)
 */
void text_render_7segment_digit(TextRenderer* tr, char digit, int x, int y, SDL_Color color,
                                int scale);

/**
 * Render numeric string in 7-segment display style
 *
 * @param tr Text renderer context
 * @param numbers Numeric string (digits 0-9, spaces, decimal points)
 * @param x X coordinate
 * @param y Y coordinate
 * @param color Segment color
 * @param scale Scale factor for segment size
 */
void text_render_7segment_string(TextRenderer* tr, const char* numbers, int x, int y,
                                 SDL_Color color, int scale);

/**
 * Get 7-segment display dimensions
 *
 * @param text Text to measure
 * @param scale Scale factor
 * @param width Output: calculated width in pixels
 * @param height Output: calculated height in pixels
 */
void text_get_7segment_dimensions(const char* text, int scale, int* width, int* height);

// ===== Utility Functions =====

/**
 * Render single character (advanced usage)
 *
 * @param tr Text renderer context
 * @param c Character to render
 * @param x X coordinate
 * @param y Y coordinate
 * @param color Character color
 */
void text_render_char(TextRenderer* tr, char c, int x, int y, SDL_Color color);

/**
 * Check if text renderer is properly initialized
 *
 * @param tr Text renderer context
 * @return true if initialized and ready to use
 */
bool text_renderer_is_ready(const TextRenderer* tr);

#endif  // TEXT_RENDERER_H
