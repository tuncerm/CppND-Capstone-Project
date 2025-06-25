#include "text_renderer.h"
#include <stdio.h>
#include <string.h>
#include "font_data.h"

/**
 * 7-segment display digit patterns
 * Each digit maps to which segments should be lit
 */
static const uint8_t seven_segment_patterns[10] = {
    0b00111111,  // 0: A,B,C,D,E,F
    0b00000110,  // 1: B,C
    0b01011011,  // 2: A,B,G,E,D
    0b01001111,  // 3: A,B,G,C,D
    0b01100110,  // 4: F,G,B,C
    0b01101101,  // 5: A,F,G,C,D
    0b01111101,  // 6: A,F,G,E,D,C
    0b00000111,  // 7: A,B,C
    0b01111111,  // 8: All segments
    0b01101111   // 9: A,B,C,D,F,G
};

/**
 * Initialize text renderer
 */
bool text_renderer_init(TextRenderer* tr, SDL_Renderer* renderer) {
    if (!tr || !renderer) {
        return false;
    }

    tr->renderer = renderer;
    tr->initialized = true;
    tr->default_color = (SDL_Color){255, 255, 255, 255};  // White default

    // Validate font data in debug builds
    font_validate_data();

    return true;
}

/**
 * Cleanup text renderer
 */
void text_renderer_cleanup(TextRenderer* tr) {
    if (!tr) {
        return;
    }

    tr->renderer = NULL;
    tr->initialized = false;
}

/**
 * Set default text color
 */
void text_renderer_set_default_color(TextRenderer* tr, SDL_Color color) {
    if (!tr) {
        return;
    }
    tr->default_color = color;
}

/**
 * Render text string at specified position
 */
void text_render_string(TextRenderer* tr, const char* text, int x, int y, SDL_Color color) {
    if (!tr || !text || !tr->initialized) {
        return;
    }

    SDL_SetRenderDrawColor(tr->renderer, color.r, color.g, color.b, color.a);

    int len = (int)strlen(text);
    if (len > 32)
        len = 32;  // Limit to prevent overflow

    for (int i = 0; i < len; i++) {
        int char_index = font_get_char_index(text[i]);
        const uint8_t* pattern = font_get_glyph_pattern(char_index);

        // Draw character using bitmap pattern
        for (int row = 0; row < FONT_HEIGHT; row++) {
            for (int col = 0; col < FONT_WIDTH; col++) {
                if (pattern[row] & (1 << (FONT_WIDTH - 1 - col))) {
                    SDL_FRect pixel = {(float)(x + i * CHAR_SPACING + col), (float)(y + row), 1.0f,
                                       1.0f};
                    SDL_RenderFillRect(tr->renderer, &pixel);
                }
            }
        }
    }
}

/**
 * Render text string using default color
 */
void text_render_string_default(TextRenderer* tr, const char* text, int x, int y) {
    if (!tr) {
        return;
    }
    text_render_string(tr, text, x, y, tr->default_color);
}

/**
 * Get text dimensions
 */
void text_get_dimensions(const char* text, int* width, int* height) {
    font_get_text_dimensions(text, width, height);
}

/**
 * Render single character
 */
void text_render_char(TextRenderer* tr, char c, int x, int y, SDL_Color color) {
    if (!tr || !tr->initialized) {
        return;
    }

    char str[2] = {c, '\0'};
    text_render_string(tr, str, x, y, color);
}

/**
 * Check if text renderer is ready
 */
bool text_renderer_is_ready(const TextRenderer* tr) {
    return tr && tr->initialized && tr->renderer;
}

// ===== 7-Segment Display Implementation =====

/**
 * Draw a single segment of 7-segment display
 */
static void draw_segment(SDL_Renderer* renderer, int x, int y, int scale, char segment_type) {
    SDL_FRect rect;

    switch (segment_type) {
        case 'A':  // Top horizontal
            rect = (SDL_FRect){x + scale, y, scale * 3, scale};
            break;
        case 'B':  // Top right vertical
            rect = (SDL_FRect){x + scale * 4, y + scale, scale, scale * 2};
            break;
        case 'C':  // Bottom right vertical
            rect = (SDL_FRect){x + scale * 4, y + scale * 4, scale, scale * 2};
            break;
        case 'D':  // Bottom horizontal
            rect = (SDL_FRect){x + scale, y + scale * 6, scale * 3, scale};
            break;
        case 'E':  // Bottom left vertical
            rect = (SDL_FRect){x, y + scale * 4, scale, scale * 2};
            break;
        case 'F':  // Top left vertical
            rect = (SDL_FRect){x, y + scale, scale, scale * 2};
            break;
        case 'G':  // Middle horizontal
            rect = (SDL_FRect){x + scale, y + scale * 3, scale * 3, scale};
            break;
        default:
            return;
    }

    SDL_RenderFillRect(renderer, &rect);
}

/**
 * Render single digit in 7-segment display style
 */
void text_render_7segment_digit(TextRenderer* tr, char digit, int x, int y, SDL_Color color,
                                int scale) {
    if (!tr || !tr->initialized || scale < 1) {
        return;
    }

    // Handle space as blank display
    if (digit == ' ') {
        return;
    }

    // Only handle digits 0-9
    if (digit < '0' || digit > '9') {
        return;
    }

    SDL_SetRenderDrawColor(tr->renderer, color.r, color.g, color.b, color.a);

    int digit_index = digit - '0';
    uint8_t pattern = seven_segment_patterns[digit_index];

    // Draw each segment if it's enabled in the pattern
    if (pattern & SEGMENT_A)
        draw_segment(tr->renderer, x, y, scale, 'A');
    if (pattern & SEGMENT_B)
        draw_segment(tr->renderer, x, y, scale, 'B');
    if (pattern & SEGMENT_C)
        draw_segment(tr->renderer, x, y, scale, 'C');
    if (pattern & SEGMENT_D)
        draw_segment(tr->renderer, x, y, scale, 'D');
    if (pattern & SEGMENT_E)
        draw_segment(tr->renderer, x, y, scale, 'E');
    if (pattern & SEGMENT_F)
        draw_segment(tr->renderer, x, y, scale, 'F');
    if (pattern & SEGMENT_G)
        draw_segment(tr->renderer, x, y, scale, 'G');
}

/**
 * Render numeric string in 7-segment display style
 */
void text_render_7segment_string(TextRenderer* tr, const char* numbers, int x, int y,
                                 SDL_Color color, int scale) {
    if (!tr || !numbers || !tr->initialized || scale < 1) {
        return;
    }

    int len = (int)strlen(numbers);
    int digit_width = scale * 6;  // Each 7-segment digit is 6 units wide (5 + 1 spacing)

    for (int i = 0; i < len && i < 16; i++) {  // Limit to 16 digits
        char c = numbers[i];

        if (c == '.') {
            // Render decimal point
            SDL_SetRenderDrawColor(tr->renderer, color.r, color.g, color.b, color.a);
            SDL_FRect dot = {(float)(x + i * digit_width + scale * 5), (float)(y + scale * 6),
                             (float)scale, (float)scale};
            SDL_RenderFillRect(tr->renderer, &dot);
        } else {
            // Render digit or space
            text_render_7segment_digit(tr, c, x + i * digit_width, y, color, scale);
        }
    }
}

/**
 * Get 7-segment display dimensions
 */
void text_get_7segment_dimensions(const char* text, int scale, int* width, int* height) {
    if (!text || !width || !height || scale < 1) {
        if (width)
            *width = 0;
        if (height)
            *height = 0;
        return;
    }

    int len = (int)strlen(text);
    if (len > 16)
        len = 16;  // Limit

    *width = len * scale * 6;  // Each digit is 6 units wide
    *height = scale * 7;       // Each digit is 7 units tall
}
