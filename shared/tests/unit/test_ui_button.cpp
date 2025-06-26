/**
 * Unit Tests for UI Button Component
 *
 * Tests button functionality including state management, input handling,
 * rendering, and callback system.
 */

#include <gtest/gtest.h>
#include "../utils/test_helpers.h"
#include "ui_framework/ui_button.h"

class UIButtonTest : public SDLTestFixture {
    // Test fixture provides initialized sdl_context
};

// ===== Button Initialization Tests =====

TEST_F(UIButtonTest, BasicButtonInitialization) {
    UIButton button;
    ui_button_init(&button, 10, 20, 100, 50, "Test Button");

    EXPECT_EQ(10, button.rect.x);
    EXPECT_EQ(20, button.rect.y);
    EXPECT_EQ(100, button.rect.w);
    EXPECT_EQ(50, button.rect.h);
    EXPECT_STREQ("Test Button", button.text);
    EXPECT_EQ(UI_BUTTON_NORMAL, button.state);
    EXPECT_TRUE(button.visible);
    EXPECT_EQ(nullptr, button.on_click);
    EXPECT_EQ(nullptr, button.userdata);
}

TEST_F(UIButtonTest, ButtonInitializationWithEmptyText) {
    UIButton button;
    ui_button_init(&button, 0, 0, 50, 30, "");

    EXPECT_STREQ("", button.text);
    EXPECT_EQ(UI_BUTTON_NORMAL, button.state);
}

TEST_F(UIButtonTest, ButtonInitializationWithNullText) {
    UIButton button;
    ui_button_init(&button, 0, 0, 50, 30, nullptr);

    // Should handle null text gracefully
    EXPECT_TRUE(strlen(button.text) == 0 || button.text[0] == '\0');
}

TEST_F(UIButtonTest, ButtonInitializationWithLongText) {
    UIButton button;
    const char* long_text =
        "This is a very long button text that exceeds normal button text length limits";
    ui_button_init(&button, 0, 0, 200, 50, long_text);

    // Text should be truncated to fit buffer
    EXPECT_TRUE(strlen(button.text) < strlen(long_text));
    EXPECT_LT(strlen(button.text), sizeof(button.text));
}

// ===== Button State Management Tests =====

TEST_F(UIButtonTest, StateFlags) {
    UIButton button = CreateTestButton(0, 0, 100, 50, "Test");

    // Test initial state
    EXPECT_TRUE(ui_button_has_state(&button, UI_BUTTON_NORMAL));
    EXPECT_FALSE(ui_button_has_state(&button, UI_BUTTON_HOVERED));
    EXPECT_FALSE(ui_button_has_state(&button, UI_BUTTON_PRESSED));
    EXPECT_FALSE(ui_button_has_state(&button, UI_BUTTON_DISABLED));

    // Test setting states
    ui_button_set_state(&button, UI_BUTTON_HOVERED, true);
    EXPECT_TRUE(ui_button_has_state(&button, UI_BUTTON_HOVERED));

    ui_button_set_state(&button, UI_BUTTON_PRESSED, true);
    EXPECT_TRUE(ui_button_has_state(&button, UI_BUTTON_PRESSED));

    ui_button_set_state(&button, UI_BUTTON_DISABLED, true);
    EXPECT_TRUE(ui_button_has_state(&button, UI_BUTTON_DISABLED));

    // Test clearing states
    ui_button_set_state(&button, UI_BUTTON_HOVERED, false);
    EXPECT_FALSE(ui_button_has_state(&button, UI_BUTTON_HOVERED));
}

TEST_F(UIButtonTest, MultipleStateFlags) {
    UIButton button = CreateTestButton(0, 0, 100, 50, "Test");

    // Set multiple states
    ui_button_set_state(&button, UI_BUTTON_HOVERED, true);
    ui_button_set_state(&button, UI_BUTTON_SELECTED, true);

    EXPECT_TRUE(ui_button_has_state(&button, UI_BUTTON_HOVERED));
    EXPECT_TRUE(ui_button_has_state(&button, UI_BUTTON_SELECTED));
    EXPECT_FALSE(ui_button_has_state(&button, UI_BUTTON_PRESSED));
}

// ===== Button Color Management Tests =====

TEST_F(UIButtonTest, ColorSettings) {
    UIButton button = CreateTestButton(0, 0, 100, 50, "Test");

    SDL_Color normal = {100, 100, 100, 255};
    SDL_Color hover = {120, 120, 120, 255};
    SDL_Color pressed = {80, 80, 80, 255};
    SDL_Color disabled = {50, 50, 50, 255};

    ui_button_set_colors(&button, normal, hover, pressed, disabled);

    EXPECT_SDL_COLOR_EQ(normal, button.bg_color_normal);
    EXPECT_SDL_COLOR_EQ(hover, button.bg_color_hover);
    EXPECT_SDL_COLOR_EQ(pressed, button.bg_color_pressed);
    EXPECT_SDL_COLOR_EQ(disabled, button.bg_color_disabled);
}

TEST_F(UIButtonTest, TextColorSetting) {
    UIButton button = CreateTestButton(0, 0, 100, 50, "Test");

    SDL_Color text_color = {255, 255, 0, 255};  // Yellow
    ui_button_set_text_color(&button, text_color);

    EXPECT_SDL_COLOR_EQ(text_color, button.text_color);
}

// ===== Button Callback Tests =====

TEST_F(UIButtonTest, CallbackSetting) {
    UIButton button = CreateTestButton(0, 0, 100, 50, "Test");
    CallbackCounter counter;

    ui_button_set_callback(&button, CallbackCounter::Callback, &counter);

    EXPECT_EQ(CallbackCounter::Callback, button.on_click);
    EXPECT_EQ(&counter, button.userdata);
}

TEST_F(UIButtonTest, CallbackExecution) {
    UIButton button = CreateTestButton(0, 0, 100, 50, "Test");
    CallbackCounter counter;
    ui_button_set_callback(&button, CallbackCounter::Callback, &counter);

    // Simulate button click by calling input handler with click inside button
    bool clicked = ui_button_handle_input(&button, 50, 25, true);

    EXPECT_TRUE(clicked);
    EXPECT_EQ(1, counter.count);
    EXPECT_EQ(&counter, counter.last_userdata);
}

TEST_F(UIButtonTest, NoCallbackSet) {
    UIButton button = CreateTestButton(0, 0, 100, 50, "Test");

    // Button with no callback should still handle input without crashing
    bool clicked = ui_button_handle_input(&button, 50, 25, true);
    EXPECT_TRUE(clicked);  // Click detected but no callback executed
}

// ===== Button Input Handling Tests =====

TEST_F(UIButtonTest, MouseHoverDetection) {
    UIButton button = CreateTestButton(10, 10, 100, 50, "Test");

    // Mouse outside button
    ui_button_handle_input(&button, 5, 5, false);
    EXPECT_FALSE(ui_button_has_state(&button, UI_BUTTON_HOVERED));

    // Mouse inside button
    ui_button_handle_input(&button, 50, 30, false);
    EXPECT_TRUE(ui_button_has_state(&button, UI_BUTTON_HOVERED));

    // Mouse outside again
    ui_button_handle_input(&button, 200, 200, false);
    EXPECT_FALSE(ui_button_has_state(&button, UI_BUTTON_HOVERED));
}

TEST_F(UIButtonTest, MouseClickDetection) {
    UIButton button = CreateTestButton(10, 10, 100, 50, "Test");
    CallbackCounter counter;
    ui_button_set_callback(&button, CallbackCounter::Callback, &counter);

    // Click inside button
    bool clicked = ui_button_handle_input(&button, 50, 30, true);
    EXPECT_TRUE(clicked);
    EXPECT_EQ(1, counter.count);

    // Click outside button
    counter.Reset();
    clicked = ui_button_handle_input(&button, 5, 5, true);
    EXPECT_FALSE(clicked);
    EXPECT_EQ(0, counter.count);
}

TEST_F(UIButtonTest, DisabledButtonInput) {
    UIButton button = CreateTestButton(10, 10, 100, 50, "Test");
    CallbackCounter counter;
    ui_button_set_callback(&button, CallbackCounter::Callback, &counter);

    ui_button_set_state(&button, UI_BUTTON_DISABLED, true);

    // Disabled button should not respond to clicks
    bool clicked = ui_button_handle_input(&button, 50, 30, true);
    EXPECT_FALSE(clicked);
    EXPECT_EQ(0, counter.count);
}

TEST_F(UIButtonTest, ButtonBoundaryTesting) {
    UIButton button = CreateTestButton(10, 10, 100, 50, "Test");

    // Test exact boundaries
    EXPECT_TRUE(PointInRect(10, 10, button.rect));    // Top-left corner
    EXPECT_TRUE(PointInRect(109, 59, button.rect));   // Bottom-right corner (exclusive)
    EXPECT_FALSE(PointInRect(9, 10, button.rect));    // Just outside left
    EXPECT_FALSE(PointInRect(10, 9, button.rect));    // Just outside top
    EXPECT_FALSE(PointInRect(110, 30, button.rect));  // Just outside right
    EXPECT_FALSE(PointInRect(50, 60, button.rect));   // Just outside bottom
}

// ===== Button Rendering Tests =====

TEST_F(UIButtonTest, BasicRendering) {
    UIButton button = CreateTestButton(10, 10, 100, 50, "Test");

    // Rendering should not crash
    EXPECT_NO_FATAL_FAILURE(ui_button_render(&button, sdl_get_renderer(&sdl_context), nullptr));
}

TEST_F(UIButtonTest, RenderingWithTextRenderer) {
    UIButton button = CreateTestButton(10, 10, 100, 50, "Test");

    TextRenderer text_renderer;
    ASSERT_TRUE(text_renderer_init(&text_renderer, sdl_get_renderer(&sdl_context)));

    EXPECT_NO_FATAL_FAILURE(
        ui_button_render(&button, sdl_get_renderer(&sdl_context), &text_renderer));

    text_renderer_cleanup(&text_renderer);
}

TEST_F(UIButtonTest, RenderingInvisibleButton) {
    UIButton button = CreateTestButton(10, 10, 100, 50, "Test");
    button.visible = false;

    // Invisible button rendering should be handled gracefully
    EXPECT_NO_FATAL_FAILURE(ui_button_render(&button, sdl_get_renderer(&sdl_context), nullptr));
}

// ===== Button Array Tests =====

TEST_F(UIButtonTest, ButtonArrayInitialization) {
    UIButtonArray array;
    EXPECT_TRUE(ui_button_array_init(&array, 10));

    EXPECT_NE(nullptr, array.buttons);
    EXPECT_EQ(0, array.count);
    EXPECT_EQ(10, array.capacity);
    EXPECT_EQ(-1, array.hovered_button);
    EXPECT_EQ(-1, array.pressed_button);

    ui_button_array_cleanup(&array);
}

TEST_F(UIButtonTest, ButtonArrayZeroCapacity) {
    UIButtonArray array;
    EXPECT_FALSE(ui_button_array_init(&array, 0));
}

TEST_F(UIButtonTest, ButtonArrayAddButtons) {
    UIButtonArray array;
    ASSERT_TRUE(ui_button_array_init(&array, 5));

    UIButton button1 = CreateTestButton(0, 0, 50, 30, "Button1");
    UIButton button2 = CreateTestButton(60, 0, 50, 30, "Button2");

    int index1 = ui_button_array_add(&array, &button1);
    int index2 = ui_button_array_add(&array, &button2);

    EXPECT_EQ(0, index1);
    EXPECT_EQ(1, index2);
    EXPECT_EQ(2, array.count);

    UIButton* retrieved1 = ui_button_array_get(&array, 0);
    UIButton* retrieved2 = ui_button_array_get(&array, 1);

    ASSERT_NE(nullptr, retrieved1);
    ASSERT_NE(nullptr, retrieved2);
    EXPECT_STREQ("Button1", retrieved1->text);
    EXPECT_STREQ("Button2", retrieved2->text);

    ui_button_array_cleanup(&array);
}

TEST_F(UIButtonTest, ButtonArrayInvalidIndex) {
    UIButtonArray array;
    ASSERT_TRUE(ui_button_array_init(&array, 5));

    EXPECT_EQ(nullptr, ui_button_array_get(&array, 0));   // Empty array
    EXPECT_EQ(nullptr, ui_button_array_get(&array, -1));  // Negative index
    EXPECT_EQ(nullptr, ui_button_array_get(&array, 10));  // Out of bounds

    ui_button_array_cleanup(&array);
}

TEST_F(UIButtonTest, ButtonArrayInputHandling) {
    UIButtonArray array;
    ASSERT_TRUE(ui_button_array_init(&array, 5));

    UIButton button1 = CreateTestButton(0, 0, 50, 30, "Button1");
    UIButton button2 = CreateTestButton(60, 0, 50, 30, "Button2");
    button1.id = 1;
    button2.id = 2;

    ui_button_array_add(&array, &button1);
    ui_button_array_add(&array, &button2);

    // Click on first button
    int clicked_index = ui_button_array_handle_input(&array, 25, 15, true);
    EXPECT_EQ(0, clicked_index);

    // Click on second button
    clicked_index = ui_button_array_handle_input(&array, 85, 15, true);
    EXPECT_EQ(1, clicked_index);

    // Click outside buttons
    clicked_index = ui_button_array_handle_input(&array, 200, 200, true);
    EXPECT_EQ(-1, clicked_index);

    ui_button_array_cleanup(&array);
}

TEST_F(UIButtonTest, ButtonArrayFindById) {
    UIButtonArray array;
    ASSERT_TRUE(ui_button_array_init(&array, 5));

    UIButton button1 = CreateTestButton(0, 0, 50, 30, "Button1");
    UIButton button2 = CreateTestButton(60, 0, 50, 30, "Button2");
    button1.id = 100;
    button2.id = 200;

    ui_button_array_add(&array, &button1);
    ui_button_array_add(&array, &button2);

    EXPECT_EQ(0, ui_button_array_find_by_id(&array, 100));
    EXPECT_EQ(1, ui_button_array_find_by_id(&array, 200));
    EXPECT_EQ(-1, ui_button_array_find_by_id(&array, 999));  // Not found

    ui_button_array_cleanup(&array);
}

TEST_F(UIButtonTest, ButtonArrayRendering) {
    UIButtonArray array;
    ASSERT_TRUE(ui_button_array_init(&array, 5));

    UIButton button1 = CreateTestButton(0, 0, 50, 30, "Button1");
    UIButton button2 = CreateTestButton(60, 0, 50, 30, "Button2");

    ui_button_array_add(&array, &button1);
    ui_button_array_add(&array, &button2);

    EXPECT_NO_FATAL_FAILURE(
        ui_button_array_render(&array, sdl_get_renderer(&sdl_context), nullptr));

    ui_button_array_cleanup(&array);
}

// ===== Error Handling Tests =====

TEST_F(UIButtonTest, NullPointerHandling) {
    // Functions should handle null pointers gracefully
    EXPECT_NO_FATAL_FAILURE(ui_button_init(nullptr, 0, 0, 50, 30, "Test"));
    EXPECT_FALSE(ui_button_handle_input(nullptr, 0, 0, false));
    EXPECT_NO_FATAL_FAILURE(ui_button_render(nullptr, sdl_get_renderer(&sdl_context), nullptr));
    EXPECT_FALSE(ui_button_has_state(nullptr, UI_BUTTON_NORMAL));
}

TEST_F(UIButtonTest, ButtonArrayNullHandling) {
    EXPECT_FALSE(ui_button_array_init(nullptr, 10));
    EXPECT_NO_FATAL_FAILURE(ui_button_array_cleanup(nullptr));
    EXPECT_EQ(-1, ui_button_array_add(nullptr, nullptr));
    EXPECT_EQ(nullptr, ui_button_array_get(nullptr, 0));
}
