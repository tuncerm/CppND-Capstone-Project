/**
 * Integration Tests for Text Renderer + UI Framework
 *
 * Tests integration between text rendering and UI components,
 * specifically button text rendering and layout calculations.
 */

#include <gtest/gtest.h>
#include "../utils/test_helpers.h"
#include "text_renderer/text_renderer.h"
#include "ui_framework/ui_button.h"

class TextUIIntegrationTest : public TextRendererTestFixture {
    // Test fixture provides initialized text_renderer and sdl_context
};

// ===== Text Renderer + Button Integration Tests =====

TEST_F(TextUIIntegrationTest, ButtonTextRendering) {
    UIButton button = CreateTestButton(10, 10, 200, 60, "Integration Test");

    // Render button with text renderer
    EXPECT_NO_FATAL_FAILURE(
        ui_button_render(&button, sdl_get_renderer(&sdl_context), &text_renderer));

    // Test with different text lengths
    strcpy(button.text, "Short");
    EXPECT_NO_FATAL_FAILURE(
        ui_button_render(&button, sdl_get_renderer(&sdl_context), &text_renderer));

    strcpy(button.text, "Very Long Button Text That Might Not Fit");
    EXPECT_NO_FATAL_FAILURE(
        ui_button_render(&button, sdl_get_renderer(&sdl_context), &text_renderer));
}

TEST_F(TextUIIntegrationTest, ButtonTextCentering) {
    UIButton button = CreateTestButton(50, 50, 150, 40, "Centered");

    // Text should be centered within button bounds
    // This is primarily a visual test, but we can verify it doesn't crash
    EXPECT_NO_FATAL_FAILURE(
        ui_button_render(&button, sdl_get_renderer(&sdl_context), &text_renderer));
}

TEST_F(TextUIIntegrationTest, ButtonArrayTextRendering) {
    UIButtonArray array;
    ASSERT_TRUE(ui_button_array_init(&array, 5));

    // Add buttons with different text
    const char* button_texts[] = {"Button 1", "Btn 2", "Very Long Button Text", "B3", ""};
    for (int i = 0; i < 5; ++i) {
        UIButton button = CreateTestButton(i * 100, 10, 90, 30, button_texts[i]);
        ui_button_array_add(&array, &button);
    }

    // Render all buttons with text
    EXPECT_NO_FATAL_FAILURE(
        ui_button_array_render(&array, sdl_get_renderer(&sdl_context), &text_renderer));

    ui_button_array_cleanup(&array);
}

// ===== Text Measurement + UI Layout Tests =====

TEST_F(TextUIIntegrationTest, ButtonSizingBasedOnText) {
    const char* test_texts[] = {"A", "Hello", "Very Long Button Text"};

    for (size_t i = 0; i < sizeof(test_texts) / sizeof(test_texts[0]); ++i) {
        int text_width, text_height;
        text_get_dimensions(test_texts[i], &text_width, &text_height);

        EXPECT_GT(text_width, 0) << "Text: " << test_texts[i];
        EXPECT_GT(text_height, 0) << "Text: " << test_texts[i];

        // Create button with padding around text
        int padding = 20;
        UIButton button =
            CreateTestButton(0, 0, text_width + padding, text_height + padding, test_texts[i]);

        EXPECT_NO_FATAL_FAILURE(
            ui_button_render(&button, sdl_get_renderer(&sdl_context), &text_renderer));
    }
}

TEST_F(TextUIIntegrationTest, TextTruncationInSmallButtons) {
    // Create very small button with long text
    UIButton button = CreateTestButton(0, 0, 50, 20, "This text is way too long for the button");

    // Should handle gracefully without crashing
    EXPECT_NO_FATAL_FAILURE(
        ui_button_render(&button, sdl_get_renderer(&sdl_context), &text_renderer));
}

// ===== Different Font Styles Integration Tests =====

TEST_F(TextUIIntegrationTest, SevenSegmentButtonText) {
    UIButton button = CreateTestButton(10, 10, 150, 60, "12345");

    // While buttons don't typically use 7-segment display, test that
    // both rendering systems can coexist
    EXPECT_NO_FATAL_FAILURE(
        ui_button_render(&button, sdl_get_renderer(&sdl_context), &text_renderer));

    // Render 7-segment display nearby
    EXPECT_NO_FATAL_FAILURE(
        text_render_7segment_string(&text_renderer, "67890", 200, 20, TEST_COLOR_GREEN, 2));
}

// ===== Color Coordination Tests =====

TEST_F(TextUIIntegrationTest, TextColorConsistency) {
    UIButton button = CreateTestButton(10, 10, 100, 40, "Test");

    // Set button text color
    SDL_Color button_text_color = {255, 255, 0, 255};  // Yellow
    ui_button_set_text_color(&button, button_text_color);

    // Set text renderer default color to same
    text_renderer_set_default_color(&text_renderer, button_text_color);

    EXPECT_NO_FATAL_FAILURE(
        ui_button_render(&button, sdl_get_renderer(&sdl_context), &text_renderer));

    // Render text with same color using text renderer directly
    EXPECT_NO_FATAL_FAILURE(text_render_string_default(&text_renderer, "Matching Color", 120, 20));
}

// ===== Performance Integration Tests =====

TEST_F(TextUIIntegrationTest, ManyButtonsWithText) {
    const int button_count = 20;
    UIButtonArray array;
    ASSERT_TRUE(ui_button_array_init(&array, button_count));

    // Create many buttons
    for (int i = 0; i < button_count; ++i) {
        char text[32];
        snprintf(text, sizeof(text), "Button %d", i);

        UIButton button = CreateTestButton((i % 5) * 80,  // x position (5 columns)
                                           (i / 5) * 40,  // y position (4 rows)
                                           75, 35,        // size
                                           text);
        ui_button_array_add(&array, &button);
    }

    // Measure rendering performance
    auto start = std::chrono::high_resolution_clock::now();

    ui_button_array_render(&array, sdl_get_renderer(&sdl_context), &text_renderer);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Rendering 20 buttons with text should be reasonably fast
    EXPECT_LT(duration.count(), 100)
        << "Rendering " << button_count << " buttons took " << duration.count() << "ms";

    ui_button_array_cleanup(&array);
}

// ===== State Synchronization Tests =====

TEST_F(TextUIIntegrationTest, ButtonStateVisualFeedback) {
    UIButton button = CreateTestButton(10, 10, 120, 40, "State Test");

    // Test different button states with text rendering
    ui_button_set_state(&button, UI_BUTTON_NORMAL, true);
    EXPECT_NO_FATAL_FAILURE(
        ui_button_render(&button, sdl_get_renderer(&sdl_context), &text_renderer));

    ui_button_set_state(&button, UI_BUTTON_HOVERED, true);
    EXPECT_NO_FATAL_FAILURE(
        ui_button_render(&button, sdl_get_renderer(&sdl_context), &text_renderer));

    ui_button_set_state(&button, UI_BUTTON_PRESSED, true);
    EXPECT_NO_FATAL_FAILURE(
        ui_button_render(&button, sdl_get_renderer(&sdl_context), &text_renderer));

    ui_button_set_state(&button, UI_BUTTON_DISABLED, true);
    EXPECT_NO_FATAL_FAILURE(
        ui_button_render(&button, sdl_get_renderer(&sdl_context), &text_renderer));
}

// ===== Error Handling Integration Tests =====

TEST_F(TextUIIntegrationTest, ErrorConditionsHandling) {
    UIButton button = CreateTestButton(10, 10, 100, 40, "Error Test");

    // Render with null text renderer (should fallback gracefully)
    EXPECT_NO_FATAL_FAILURE(ui_button_render(&button, sdl_get_renderer(&sdl_context), nullptr));

    // Render with null renderer (both components should handle)
    EXPECT_NO_FATAL_FAILURE(ui_button_render(&button, nullptr, &text_renderer));
}

// ===== Interactive Integration Tests =====

TEST_F(TextUIIntegrationTest, ButtonInteractionWithTextFeedback) {
    UIButton button = CreateTestButton(50, 50, 100, 40, "Click Me");
    CallbackCounter counter;
    ui_button_set_callback(&button, CallbackCounter::Callback, &counter);

    // Simulate hover and render
    ui_button_handle_input(&button, 75, 70, false);  // Inside button, no click
    EXPECT_TRUE(ui_button_has_state(&button, UI_BUTTON_HOVERED));

    EXPECT_NO_FATAL_FAILURE(
        ui_button_render(&button, sdl_get_renderer(&sdl_context), &text_renderer));

    // Simulate click and render
    ui_button_handle_input(&button, 75, 70, true);  // Inside button, click
    EXPECT_EQ(1, counter.count);

    EXPECT_NO_FATAL_FAILURE(
        ui_button_render(&button, sdl_get_renderer(&sdl_context), &text_renderer));
}

// ===== Layout Integration Tests =====

TEST_F(TextUIIntegrationTest, TextBasedButtonLayout) {
    // Create buttons that size themselves based on text content
    const char* button_texts[] = {"OK", "Cancel", "Apply Changes", "A"};
    const int num_buttons = sizeof(button_texts) / sizeof(button_texts[0]);

    int y_pos = 10;
    for (int i = 0; i < num_buttons; ++i) {
        int text_width, text_height;
        text_get_dimensions(button_texts[i], &text_width, &text_height);

        // Create button sized to text with padding
        UIButton button = CreateTestButton(10,                // x
                                           y_pos,             // y
                                           text_width + 40,   // width with padding
                                           text_height + 20,  // height with padding
                                           button_texts[i]);

        EXPECT_NO_FATAL_FAILURE(
            ui_button_render(&button, sdl_get_renderer(&sdl_context), &text_renderer));

        y_pos += text_height + 30;  // Next button position
    }
}

// ===== Multi-Component Rendering Tests =====

TEST_F(TextUIIntegrationTest, MixedTextAndUIRendering) {
    // Render a mix of UI components and standalone text

    // Title text
    EXPECT_NO_FATAL_FAILURE(
        text_render_string(&text_renderer, "UI Integration Test", 10, 10, TEST_COLOR_WHITE));

    // Regular button
    UIButton button1 = CreateTestButton(10, 40, 100, 30, "Button 1");
    EXPECT_NO_FATAL_FAILURE(
        ui_button_render(&button1, sdl_get_renderer(&sdl_context), &text_renderer));

    // Label text
    EXPECT_NO_FATAL_FAILURE(
        text_render_string(&text_renderer, "Status:", 120, 45, TEST_COLOR_GREEN));

    // 7-segment display
    EXPECT_NO_FATAL_FAILURE(
        text_render_7segment_string(&text_renderer, "12345", 10, 80, TEST_COLOR_RED, 1));

    // Another button
    UIButton button2 = CreateTestButton(120, 80, 80, 25, "OK");
    EXPECT_NO_FATAL_FAILURE(
        ui_button_render(&button2, sdl_get_renderer(&sdl_context), &text_renderer));
}
