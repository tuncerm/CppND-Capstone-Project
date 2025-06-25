#ifndef FONT_DATA_H
#define FONT_DATA_H

#include <stdbool.h>
#include <stdint.h>

/**
 * Font Data for 5x7 Bitmap Font System
 *
 * This header provides the glyph patterns and character mapping for a complete
 * 5x7 bitmap font supporting 60 characters including digits, letters, punctuation,
 * and special symbols (arrows).
 */

// Total number of glyphs available
#define GLYPH_COUNT 60
#define FONT_WIDTH 5
#define FONT_HEIGHT 7
#define CHAR_SPACING 6  // 5 pixels + 1 pixel spacing

/**
 * Get glyph index for a character
 * Maps ASCII characters to font glyph indices
 *
 * @param c Character to map
 * @return Glyph index (0-59), 0 for unknown characters (space)
 */
int font_get_char_index(char c);

/**
 * Get font glyph pattern data
 * Returns pointer to 7-row bitmap pattern for specified glyph
 *
 * @param glyph_index Index of glyph (0-59)
 * @return Pointer to 7-byte glyph pattern, NULL for invalid index
 */
const uint8_t* font_get_glyph_pattern(int glyph_index);

/**
 * Calculate text dimensions
 *
 * @param text Text string to measure
 * @param width Output: calculated width in pixels
 * @param height Output: calculated height in pixels (always 7)
 */
void font_get_text_dimensions(const char* text, int* width, int* height);

/**
 * Validate font data integrity (debug builds only)
 * Verifies glyph count and pattern consistency
 */
void font_validate_data(void);

#endif  // FONT_DATA_H
