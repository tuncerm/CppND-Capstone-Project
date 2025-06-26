/**
 * Unit Tests for Text Renderer Component
 *
 * Tests text rendering functionality including 5x7 bitmap font,
 * 7-segment display rendering, and text measurement functions.
 */

#include <gtest/gtest.h>
#include "../utils/test_helpers.h"
#include "text_renderer/text_renderer.h"

class TextRendererTest : public TextRendererTestFixture {
    // Test fixture provides initialized text_renderer and sdl_context
};

// ===== Initialization Tests =====

TEST_F(TextRendererTest, InitializationSuccess) {
    EXPECT_TRUE(text_renderer_initialized);
    EXPECT_TRUE(text_renderer_is_ready(&text_renderer));
    EXPECT_NE(nullptr, text_renderer.renderer);
    EXPECT_TRUE(text_renderer.initialized);
}

TEST_F(TextRendererTest, InitializationWithNullRenderer) {
    TextRenderer tr;
    EXPECT_FALSE(text_renderer_init(&tr, nullptr));
    EXPECT_FALSE(text_renderer_is_ready(&tr));
}

TEST_F(TextRendererTest, CleanupAfterInit) {
    TextRenderer tr;
    ASSERT_TRUE(text_renderer_init(&tr, sdl_get_renderer(&sdl_context)));
    EXPECT_TRUE(text_renderer_is_ready(&tr));

    text_renderer_cleanup(&tr);
    EXPECT_FALSE(text_renderer_is_ready(&tr));
}

TEST_F(TextRendererTest, CleanupWithoutInit) {
    TextRenderer tr = {};
    EXPECT_NO_FATAL_FAILURE(text_renderer_cleanup(&tr));
    EXPECT_FALSE(text_renderer_is_ready(&tr));
}

// ===== Color Management Tests =====

TEST_F(TextRendererTest, DefaultColorSetting) {
    SDL_Color test_color = TEST_COLOR_RED;
    text_renderer_set_default_color(&text_renderer, test_color);

    EXPECT_SDL_COLOR_EQ(test_color, text_renderer.default_color);
}

TEST_F(TextRendererTest, DefaultColorRendering) {
    SDL_Color blue = TEST_COLOR_BLUE;
    text_renderer_set_default_color(&text_renderer, blue);

    // Test that default rendering uses the set color
    // This is a smoke test - actual color verification would require pixel inspection
    EXPECT_NO_FATAL_FAILURE(text_render_string_default(&text_renderer, "Test", 10, 10));
}

// ===== Text Rendering Tests =====

TEST_F(TextRendererTest, BasicTextRendering) {
    EXPECT_NO_FATAL_FAILURE(text_render_string(&text_renderer, "Hello", 0, 0, TEST_COLOR_WHITE));
}

TEST_F(TextRendererTest, EmptyStringRendering) {
    EXPECT_NO_FATAL_FAILURE(text_render_string(&text_renderer, "", 0, 0, TEST_COLOR_WHITE));
}

TEST_F(TextRendererTest, NullStringHandling) {
    EXPECT_NO_FATAL_FAILURE(text_render_string(&text_renderer, nullptr, 0, 0, TEST_COLOR_WHITE));
}

TEST_F(TextRendererTest, LongStringRendering) {
    const char* long_text = "This is a very long text string that exceeds normal limits";
    EXPECT_NO_FATAL_FAILURE(text_render_string(&text_renderer, long_text, 0, 0, TEST_COLOR_WHITE));
}

TEST_F(TextRendererTest, SingleCharacterRendering) {
    for (char c = 'A'; c <= 'Z'; ++c) {
        EXPECT_NO_FATAL_FAILURE(text_render_char(&text_renderer, c, 0, 0, TEST_COLOR_WHITE))
            << "Failed to render character: " << c;
    }

    for (char c = '0'; c <= '9'; ++c) {
        EXPECT_NO_FATAL_FAILURE(text_render_char(&text_renderer, c, 0, 0, TEST_COLOR_WHITE))
            << "Failed to render digit: " << c;
    }
}

TEST_F(TextRendererTest, SpecialCharacterRendering) {
    const char special_chars[] = " !@#$%^&*()_+-=[]{}|;:,.<>?";
    for (size_t i = 0; i < strlen(special_chars); ++i) {
        EXPECT_NO_FATAL_FAILURE(
            text_render_char(&text_renderer, special_chars[i], i * 10, 0, TEST_COLOR_WHITE))
            << "Failed to render special character: " << special_chars[i];
    }
}

// ===== Text Measurement Tests =====

TEST_F(TextRendererTest, EmptyStringDimensions) {
    int width, height;
    text_get_dimensions("", &width, &height);
    EXPECT_EQ(0, width);
    EXPECT_GT(height, 0);  // Height should still be font height
}

TEST_F(TextRendererTest, SingleCharacterDimensions) {
    int width, height;
    text_get_dimensions("A", &width, &height);
    EXPECT_GT(width, 0);
    EXPECT_GT(height, 0);

    // All single characters should have same height
    int width2, height2;
    text_get_dimensions("B", &width2, &height2);
    EXPECT_EQ(height, height2);
}

TEST_F(TextRendererTest, MultipleCharacterDimensions) {
    int width1, height1;
    text_get_dimensions("A", &width1, &height1);

    int width2, height2;
    text_get_dimensions("AB", &width2, &height2);

    EXPECT_GT(width2, width1);    // Two characters should be wider
    EXPECT_EQ(height1, height2);  // Same height
}

TEST_F(TextRendererTest, TextDimensionsConsistency) {
    const char* test_strings[] = {"A", "AB", "ABC", "ABCD", "Hello World"};
    int prev_width = 0;

    for (size_t i = 0; i < sizeof(test_strings) / sizeof(test_strings[0]); ++i) {
        int width, height;
        text_get_dimensions(test_strings[i], &width, &height);

        EXPECT_GT(width, 0) << "Text: " << test_strings[i];
        EXPECT_GT(height, 0) << "Text: " << test_strings[i];

        if (i > 0) {
            EXPECT_GE(width, prev_width) << "Width should increase or stay same for longer text";
        }
        prev_width = width;
    }
}

// ===== 7-Segment Display Tests =====

TEST_F(TextRendererTest, SevenSegmentDigitRendering) {
    for (char digit = '0'; digit <= '9'; ++digit) {
        EXPECT_NO_FATAL_FAILURE(
            text_render_7segment_digit(&text_renderer, digit, 0, 0, TEST_COLOR_GREEN, 1))
            << "Failed to render 7-segment digit: " << digit;
    }
}

TEST_F(TextRendererTest, SevenSegmentSpaceRendering) {
    EXPECT_NO_FATAL_FAILURE(
        text_render_7segment_digit(&text_renderer, ' ', 0, 0, TEST_COLOR_GREEN, 1));
}

TEST_F(TextRendererTest, SevenSegmentInvalidCharacter) {
    // Non-digit characters should be handled gracefully
    EXPECT_NO_FATAL_FAILURE(
        text_render_7segment_digit(&text_renderer, 'A', 0, 0, TEST_COLOR_GREEN, 1));
}

TEST_F(TextRendererTest, SevenSegmentStringRendering) {
    const char* test_numbers[] = {"0", "123", "456789", "0123456789"};

    for (size_t i = 0; i < sizeof(test_numbers) / sizeof(test_numbers[0]); ++i) {
        EXPECT_NO_FATAL_FAILURE(
            text_render_7segment_string(&text_renderer, test_numbers[i], 0, 0, TEST_COLOR_RED, 1))
            << "Failed to render 7-segment string: " << test_numbers[i];
    }
}

TEST_F(TextRendererTest, SevenSegmentScaling) {
    for (int scale = 1; scale <= 4; ++scale) {
        EXPECT_NO_FATAL_FAILURE(
            text_render_7segment_digit(&text_renderer, '5', 0, 0, TEST_COLOR_BLUE, scale))
            << "Failed to render 7-segment digit at scale: " << scale;
    }
}

TEST_F(TextRendererTest, SevenSegmentDimensions) {
    int width1, height1;
    text_get_7segment_dimensions("1", 1, &width1, &height1);
    EXPECT_GT(width1, 0);
    EXPECT_GT(height1, 0);

    // Test scaling
    int width2, height2;
    text_get_7segment_dimensions("1", 2, &width2, &height2);
    EXPECT_GT(width2, width1);
    EXPECT_GT(height2, height1);

    // Test multiple digits
    int width3, height3;
    text_get_7segment_dimensions("12", 1, &width3, &height3);
    EXPECT_GT(width3, width1);
    EXPECT_EQ(height3, height1);  // Same scale, same height
}

// ===== Position and Bounds Tests =====

TEST_F(TextRendererTest, NegativePositionRendering) {
    EXPECT_NO_FATAL_FAILURE(text_render_string(&text_renderer, "Test", -10, -5, TEST_COLOR_WHITE));
}

TEST_F(TextRendererTest, LargePositionRendering) {
    EXPECT_NO_FATAL_FAILURE(
        text_render_string(&text_renderer, "Test", 1000, 800, TEST_COLOR_WHITE));
}

// ===== Error Handling Tests =====

TEST_F(TextRendererTest, NullTextRendererHandling) {
    // Functions should handle null text renderer gracefully
    EXPECT_NO_FATAL_FAILURE(text_render_string(nullptr, "Test", 0, 0, TEST_COLOR_WHITE));
    EXPECT_NO_FATAL_FAILURE(text_render_char(nullptr, 'A', 0, 0, TEST_COLOR_WHITE));
    EXPECT_FALSE(text_renderer_is_ready(nullptr));
}

TEST_F(TextRendererTest, UninitializedTextRendererHandling) {
    TextRenderer uninitialized = {};
    EXPECT_FALSE(text_renderer_is_ready(&uninitialized));
    EXPECT_NO_FATAL_FAILURE(text_render_string(&uninitialized, "Test", 0, 0, TEST_COLOR_WHITE));
}

// ===== Performance Tests =====

TEST_F(TextRendererTest, RenderingPerformance) {
    // Test that rendering many characters doesn't take too long
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 100; ++i) {
        text_render_string(&text_renderer, "Performance Test String", i, 0, TEST_COLOR_WHITE);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // 100 string renders should complete reasonably quickly
    EXPECT_LT(duration.count(), 100) << "100 text renders took " << duration.count() << "ms";
}

TEST_F(TextRendererTest, DimensionCalculationPerformance) {
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 1000; ++i) {
        int width, height;
        text_get_dimensions("Test String For Performance", &width, &height);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // 1000 dimension calculations should be very fast
    EXPECT_LT(duration.count(), 10)
        << "1000 dimension calculations took " << duration.count() << "ms";
}
