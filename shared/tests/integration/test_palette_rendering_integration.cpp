/**
 * Integration Tests for Palette Manager + Rendering Components
 *
 * Tests integration between palette management and rendering systems,
 * including color consistency and palette-based rendering.
 */

#include <gtest/gtest.h>
#include "../utils/test_helpers.h"
#include "palette_manager/palette_manager.h"
#include "text_renderer/text_renderer.h"
#include "ui_framework/ui_button.h"

class PaletteRenderingIntegrationTest : public TextRendererTestFixture {
   protected:
    void SetUp() override {
        TextRendererTestFixture::SetUp();
        ASSERT_TRUE(palette_manager_init(&palette_manager));
        CreateTestPalette(&palette_manager);
    }

    PaletteManager palette_manager;
};

// ===== Palette + Text Rendering Integration Tests =====

TEST_F(PaletteRenderingIntegrationTest, TextRenderingWithPaletteColors) {
    // Test rendering text using colors from palette
    for (int i = 0; i < PALETTE_COLOR_COUNT; ++i) {
        SDL_Color color = palette_get_sdl_color(&palette_manager, i);

        char text[32];
        snprintf(text, sizeof(text), "Color %d", i);

        EXPECT_NO_FATAL_FAILURE(text_render_string(&text_renderer, text, i * 60, 10, color));
    }
}

TEST_F(PaletteRenderingIntegrationTest, SevenSegmentWithPaletteColors) {
    // Test 7-segment display with palette colors
    for (int i = 0; i < 10 && i < PALETTE_COLOR_COUNT; ++i) {
        SDL_Color color = palette_get_sdl_color(&palette_manager, i);
        char digit = '0' + i;

        EXPECT_NO_FATAL_FAILURE(
            text_render_7segment_digit(&text_renderer, digit, i * 40, 50, color, 2));
    }
}

// ===== Palette + UI Integration Tests =====

TEST_F(PaletteRenderingIntegrationTest, ButtonsWithPaletteColors) {
    UIButtonArray array;
    ASSERT_TRUE(ui_button_array_init(&array, PALETTE_COLOR_COUNT));

    // Create buttons using palette colors
    for (int i = 0; i < PALETTE_COLOR_COUNT; ++i) {
        RGBA palette_color = palette_get_color(&palette_manager, i);
        SDL_Color bg_color = {palette_color.r, palette_color.g, palette_color.b, palette_color.a};

        char button_text[16];
        snprintf(button_text, sizeof(button_text), "P%d", i);

        UIButton button = CreateTestButton((i % 4) * 100,  // x (4 columns)
                                           (i / 4) * 40,   // y (4 rows)
                                           90, 35,         // size
                                           button_text);

        // Set button colors from palette
        ui_button_set_colors(&button, bg_color, bg_color, bg_color, bg_color);

        // Set contrasting text color
        SDL_Color text_color = (palette_color.r + palette_color.g + palette_color.b > 400)
                                   ? TEST_COLOR_BLACK
                                   : TEST_COLOR_WHITE;
        ui_button_set_text_color(&button, text_color);

        ui_button_array_add(&array, &button);
    }

    // Render all palette-colored buttons
    EXPECT_NO_FATAL_FAILURE(
        ui_button_array_render(&array, sdl_get_renderer(&sdl_context), &text_renderer));

    ui_button_array_cleanup(&array);
}

// ===== Color Consistency Tests =====

TEST_F(PaletteRenderingIntegrationTest, ColorConversionConsistency) {
    for (int i = 0; i < PALETTE_COLOR_COUNT; ++i) {
        // Get color in both formats
        RGBA rgba_color = palette_get_color(&palette_manager, i);
        SDL_Color sdl_color = palette_get_sdl_color(&palette_manager, i);

        // They should match
        EXPECT_EQ(rgba_color.r, sdl_color.r) << "Color " << i << " red mismatch";
        EXPECT_EQ(rgba_color.g, sdl_color.g) << "Color " << i << " green mismatch";
        EXPECT_EQ(rgba_color.b, sdl_color.b) << "Color " << i << " blue mismatch";
        EXPECT_EQ(rgba_color.a, sdl_color.a) << "Color " << i << " alpha mismatch";

        // Test rendering with both formats produces same visual result
        EXPECT_NO_FATAL_FAILURE(text_render_string(&text_renderer, "RGBA", i * 50, 100, sdl_color));

        // Convert back and forth
        RGBA converted = palette_from_sdl_color(sdl_color);
        EXPECT_RGBA_EQ(rgba_color, converted);
    }
}

// ===== Palette Modification + Rendering Tests =====

TEST_F(PaletteRenderingIntegrationTest, DynamicPaletteChanges) {
    // Initial rendering with current palette
    SDL_Color initial_color = palette_get_sdl_color(&palette_manager, 0);
    EXPECT_NO_FATAL_FAILURE(
        text_render_string(&text_renderer, "Before Change", 10, 150, initial_color));

    // Modify palette color
    RGBA new_color = palette_make_color(200, 100, 50, 255);
    EXPECT_TRUE(palette_set_color(&palette_manager, 0, new_color));
    EXPECT_TRUE(palette_manager_is_modified(&palette_manager));

    // Render with modified color
    SDL_Color modified_color = palette_get_sdl_color(&palette_manager, 0);
    EXPECT_NO_FATAL_FAILURE(
        text_render_string(&text_renderer, "After Change", 10, 180, modified_color));

    // Verify the change took effect
    EXPECT_EQ(200, modified_color.r);
    EXPECT_EQ(100, modified_color.g);
    EXPECT_EQ(50, modified_color.b);
    EXPECT_EQ(255, modified_color.a);
}

// ===== Palette Save/Load + Rendering Consistency Tests =====

TEST_F(PaletteRenderingIntegrationTest, PalettePersistenceConsistency) {
    // Render with original palette
    std::vector<SDL_Color> original_colors;
    for (int i = 0; i < PALETTE_COLOR_COUNT; ++i) {
        SDL_Color color = palette_get_sdl_color(&palette_manager, i);
        original_colors.push_back(color);

        char text[16];
        snprintf(text, sizeof(text), "Orig%d", i);
        EXPECT_NO_FATAL_FAILURE(text_render_string(&text_renderer, text, i * 40, 200, color));
    }

    // Save and reload palette
    std::string temp_file = CreateTempTestFile("palette_test.pal", "");
    ASSERT_FALSE(temp_file.empty());

    EXPECT_TRUE(palette_manager_save(&palette_manager, temp_file.c_str()));

    PaletteManager loaded_palette;
    ASSERT_TRUE(palette_manager_init(&loaded_palette));
    EXPECT_TRUE(palette_manager_load(&loaded_palette, temp_file.c_str()));

    // Verify loaded colors match and render consistently
    for (int i = 0; i < PALETTE_COLOR_COUNT; ++i) {
        SDL_Color loaded_color = palette_get_sdl_color(&loaded_palette, i);

        EXPECT_SDL_COLOR_EQ(original_colors[i], loaded_color);

        char text[16];
        snprintf(text, sizeof(text), "Load%d", i);
        EXPECT_NO_FATAL_FAILURE(
            text_render_string(&text_renderer, text, i * 40, 230, loaded_color));
    }

    RemoveTempTestFile(temp_file);
}

// ===== Performance Integration Tests =====

TEST_F(PaletteRenderingIntegrationTest, PaletteAccessPerformance) {
    auto start = std::chrono::high_resolution_clock::now();

    // Rapidly access palette colors and render
    for (int frame = 0; frame < 100; ++frame) {
        for (int i = 0; i < PALETTE_COLOR_COUNT; ++i) {
            SDL_Color color = palette_get_sdl_color(&palette_manager, i);
            char text[8];
            snprintf(text, sizeof(text), "%d", frame % 10);
            text_render_string(&text_renderer, text, i * 20, frame % 300, color);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // 100 frames * 16 colors should be reasonably fast
    EXPECT_LT(duration.count(), 500)
        << "Palette rendering performance test took " << duration.count() << "ms";
}

// ===== Multi-Component Integration Tests =====

TEST_F(PaletteRenderingIntegrationTest, ComplexSceneRendering) {
    // Create a complex scene using palette colors

    // Background color (first palette color)
    SDL_Color bg_color = palette_get_sdl_color(&palette_manager, 0);
    sdl_clear_screen(&sdl_context, bg_color);

    // Title using second palette color
    SDL_Color title_color = palette_get_sdl_color(&palette_manager, 1);
    EXPECT_NO_FATAL_FAILURE(
        text_render_string(&text_renderer, "Palette Integration Test", 10, 10, title_color));

    // Create UI elements using various palette colors
    UIButton button1 = CreateTestButton(10, 40, 100, 30, "Button A");
    SDL_Color btn1_color = palette_get_sdl_color(&palette_manager, 2);
    ui_button_set_colors(&button1, btn1_color, btn1_color, btn1_color, btn1_color);
    ui_button_set_text_color(&button1, palette_get_sdl_color(&palette_manager, 15));

    UIButton button2 = CreateTestButton(120, 40, 100, 30, "Button B");
    SDL_Color btn2_color = palette_get_sdl_color(&palette_manager, 3);
    ui_button_set_colors(&button2, btn2_color, btn2_color, btn2_color, btn2_color);
    ui_button_set_text_color(&button2, palette_get_sdl_color(&palette_manager, 0));

    EXPECT_NO_FATAL_FAILURE(
        ui_button_render(&button1, sdl_get_renderer(&sdl_context), &text_renderer));
    EXPECT_NO_FATAL_FAILURE(
        ui_button_render(&button2, sdl_get_renderer(&sdl_context), &text_renderer));

    // 7-segment display using palette colors
    for (int i = 0; i < 5; ++i) {
        SDL_Color segment_color = palette_get_sdl_color(&palette_manager, 4 + i);
        char digit = '0' + i;
        EXPECT_NO_FATAL_FAILURE(
            text_render_7segment_digit(&text_renderer, digit, 10 + i * 50, 80, segment_color, 1));
    }

    // Status text using another palette color
    SDL_Color status_color = palette_get_sdl_color(&palette_manager, 10);
    EXPECT_NO_FATAL_FAILURE(text_render_string(&text_renderer, "Status: All systems operational",
                                               10, 120, status_color));

    // Present the scene
    EXPECT_NO_FATAL_FAILURE(sdl_present(&sdl_context));
}

// ===== Error Handling Integration Tests =====

TEST_F(PaletteRenderingIntegrationTest, InvalidPaletteHandling) {
    // Test rendering with invalid palette indices
    SDL_Color invalid_color = palette_get_sdl_color(&palette_manager, -1);
    EXPECT_NO_FATAL_FAILURE(
        text_render_string(&text_renderer, "Invalid Index", 10, 260, invalid_color));

    invalid_color = palette_get_sdl_color(&palette_manager, PALETTE_COLOR_COUNT + 10);
    EXPECT_NO_FATAL_FAILURE(
        text_render_string(&text_renderer, "Out of Range", 10, 290, invalid_color));
}

// ===== Color Space and Format Tests =====

TEST_F(PaletteRenderingIntegrationTest, ColorFormatEdgeCases) {
    // Test edge case colors
    RGBA edge_colors[] = {
        {0, 0, 0, 0},          // Transparent black
        {255, 255, 255, 0},    // Transparent white
        {128, 128, 128, 128},  // Semi-transparent gray
        {255, 0, 0, 255},      // Opaque red
        {0, 255, 0, 255},      // Opaque green
        {0, 0, 255, 255}       // Opaque blue
    };

    for (size_t i = 0; i < sizeof(edge_colors) / sizeof(edge_colors[0]) && i < PALETTE_COLOR_COUNT;
         ++i) {
        palette_set_color(&palette_manager, i, edge_colors[i]);
        SDL_Color sdl_color = palette_get_sdl_color(&palette_manager, i);

        char text[32];
        snprintf(text, sizeof(text), "Edge %zu", i);
        EXPECT_NO_FATAL_FAILURE(text_render_string(&text_renderer, text, i * 80, 320, sdl_color));
    }
}

// ===== Palette Animation Integration Tests =====

TEST_F(PaletteRenderingIntegrationTest, AnimatedPaletteEffects) {
    // Simulate palette animation by cycling colors
    for (int frame = 0; frame < 10; ++frame) {
        // Rotate palette colors
        RGBA first_color = palette_get_color(&palette_manager, 0);

        for (int i = 0; i < PALETTE_COLOR_COUNT - 1; ++i) {
            RGBA next_color = palette_get_color(&palette_manager, i + 1);
            palette_set_color(&palette_manager, i, next_color);
        }
        palette_set_color(&palette_manager, PALETTE_COLOR_COUNT - 1, first_color);

        // Render frame with rotated palette
        SDL_Color frame_color = palette_get_sdl_color(&palette_manager, 0);
        char frame_text[16];
        snprintf(frame_text, sizeof(frame_text), "Frame %d", frame);

        EXPECT_NO_FATAL_FAILURE(
            text_render_string(&text_renderer, frame_text, 10, 350 + frame * 15, frame_color));
    }
}
