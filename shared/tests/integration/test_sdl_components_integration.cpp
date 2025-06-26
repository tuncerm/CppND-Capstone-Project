/**
 * Integration Tests for SDL Context + All Components
 *
 * Tests integration of SDL context with all other shared components,
 * ensuring proper initialization order and resource management.
 */

#include <gtest/gtest.h>
#include <atomic>
#include <thread>
#include "../utils/test_helpers.h"
#include "shared_components.h"

class SDLComponentsIntegrationTest : public ::testing::Test {
   protected:
    void SetUp() override {
        // Shared components should already be initialized by global environment
        ASSERT_TRUE(shared_components_is_initialized());
    }

    void TearDown() override {
        // Clean up any contexts created during tests
        if (sdl_context_is_ready(&test_context)) {
            sdl_cleanup_context(&test_context);
        }
    }

    SDLContext test_context = {};
};

// ===== Full System Integration Tests =====

TEST_F(SDLComponentsIntegrationTest, CompleteSystemInitialization) {
    // Test complete initialization sequence
    EXPECT_TRUE(sdl_init_context_simple(&test_context, "Integration Test", 800, 600));
    EXPECT_TRUE(VerifySDLContext(&test_context));

    // Initialize all major components with SDL context
    TextRenderer text_renderer;
    EXPECT_TRUE(text_renderer_init(&text_renderer, sdl_get_renderer(&test_context)));

    PaletteManager palette_manager;
    EXPECT_TRUE(palette_manager_init(&palette_manager));

    UIButtonArray button_array;
    EXPECT_TRUE(ui_button_array_init(&button_array, 10));

    DoubleClickDetector double_click;
    double_click_init(&double_click, 500);

    // All components should be ready
    EXPECT_TRUE(text_renderer_is_ready(&text_renderer));
    EXPECT_TRUE(sdl_context_is_ready(&test_context));
    EXPECT_FALSE(palette_manager_is_modified(&palette_manager));
    EXPECT_FALSE(double_click_has_previous(&double_click));

    // Cleanup
    text_renderer_cleanup(&text_renderer);
    ui_button_array_cleanup(&button_array);
}

TEST_F(SDLComponentsIntegrationTest, CrossComponentDataFlow) {
    // Test data flowing between components
    ASSERT_TRUE(sdl_init_context_simple(&test_context, "Data Flow Test", 640, 480));

    // Initialize components
    TextRenderer text_renderer;
    PaletteManager palette_manager;
    ASSERT_TRUE(text_renderer_init(&text_renderer, sdl_get_renderer(&test_context)));
    ASSERT_TRUE(palette_manager_init(&palette_manager));

    // Create test palette
    CreateTestPalette(&palette_manager);

    // Use palette colors in text rendering
    SDL_Color palette_color = palette_get_sdl_color(&palette_manager, 2);  // Red
    EXPECT_NO_FATAL_FAILURE(
        text_render_string(&text_renderer, "Palette Text", 10, 10, palette_color));

    // Create button using palette colors
    UIButton button = CreateTestButton(10, 50, 150, 40, "Palette Button");
    SDL_Color button_bg = palette_get_sdl_color(&palette_manager, 3);    // Green
    SDL_Color button_text = palette_get_sdl_color(&palette_manager, 1);  // White

    ui_button_set_colors(&button, button_bg, button_bg, button_bg, button_bg);
    ui_button_set_text_color(&button, button_text);

    EXPECT_NO_FATAL_FAILURE(
        ui_button_render(&button, sdl_get_renderer(&test_context), &text_renderer));

    // Present frame
    EXPECT_NO_FATAL_FAILURE(sdl_present(&test_context));

    text_renderer_cleanup(&text_renderer);
}

// ===== Resource Management Integration Tests =====

TEST_F(SDLComponentsIntegrationTest, ResourceLifecycleManagement) {
    // Test proper resource cleanup order
    std::vector<std::unique_ptr<TextRenderer>> text_renderers;
    std::vector<std::unique_ptr<UIButtonArray>> button_arrays;

    ASSERT_TRUE(sdl_init_context_simple(&test_context, "Resource Test", 800, 600));

    // Create multiple instances of components
    for (int i = 0; i < 5; ++i) {
        auto tr = std::make_unique<TextRenderer>();
        EXPECT_TRUE(text_renderer_init(tr.get(), sdl_get_renderer(&test_context)));
        text_renderers.push_back(std::move(tr));

        auto ba = std::make_unique<UIButtonArray>();
        EXPECT_TRUE(ui_button_array_init(ba.get(), 5));
        button_arrays.push_back(std::move(ba));
    }

    // All should be functional
    for (const auto& tr : text_renderers) {
        EXPECT_TRUE(text_renderer_is_ready(tr.get()));
        EXPECT_NO_FATAL_FAILURE(text_render_string(tr.get(), "Test", 0, 0, TEST_COLOR_WHITE));
    }

    // Cleanup components before SDL context
    for (auto& tr : text_renderers) {
        text_renderer_cleanup(tr.get());
    }
    for (auto& ba : button_arrays) {
        ui_button_array_cleanup(ba.get());
    }

    text_renderers.clear();
    button_arrays.clear();

    // SDL context should still be valid
    EXPECT_TRUE(sdl_context_is_ready(&test_context));
}

// ===== Event Handling Integration Tests =====

TEST_F(SDLComponentsIntegrationTest, InputEventIntegration) {
    ASSERT_TRUE(sdl_init_context_simple(&test_context, "Input Test", 640, 480));

    // Create interactive components
    UIButtonArray button_array;
    DoubleClickDetector double_click;
    ASSERT_TRUE(ui_button_array_init(&button_array, 3));
    double_click_init(&double_click, 500);

    // Add buttons
    UIButton button1 = CreateTestButton(10, 10, 100, 40, "Button 1");
    UIButton button2 = CreateTestButton(120, 10, 100, 40, "Button 2");
    UIButton button3 = CreateTestButton(230, 10, 100, 40, "Button 3");

    CallbackCounter counter1, counter2, counter3;
    ui_button_set_callback(&button1, CallbackCounter::Callback, &counter1);
    ui_button_set_callback(&button2, CallbackCounter::Callback, &counter2);
    ui_button_set_callback(&button3, CallbackCounter::Callback, &counter3);

    ui_button_array_add(&button_array, &button1);
    ui_button_array_add(&button_array, &button2);
    ui_button_array_add(&button_array, &button3);

    // Simulate mouse interactions
    Uint32 current_time = SDL_GetTicks();

    // Click on button 1
    int clicked_button = ui_button_array_handle_input(&button_array, 60, 30, true);
    EXPECT_EQ(0, clicked_button);
    EXPECT_EQ(1, counter1.count);

    // Test double-click detection
    bool double_clicked = double_click_check(&double_click, 1);
    EXPECT_FALSE(double_clicked);  // First click

    current_time += 150;
    double_clicked = double_click_check(&double_click, 1);
    EXPECT_TRUE(double_clicked);  // Second click (double-click)

    // Click on button 2
    clicked_button = ui_button_array_handle_input(&button_array, 170, 30, true);
    EXPECT_EQ(1, clicked_button);
    EXPECT_EQ(1, counter2.count);

    ui_button_array_cleanup(&button_array);
}

// ===== File I/O Integration Tests =====

TEST_F(SDLComponentsIntegrationTest, FileOperationsIntegration) {
    ASSERT_TRUE(sdl_init_context_simple(&test_context, "File I/O Test", 640, 480));

    // Create and save palette
    PaletteManager palette_manager;
    ASSERT_TRUE(palette_manager_init(&palette_manager));
    CreateTestPalette(&palette_manager);

    // Create temporary file
    std::string temp_file = CreateTempTestFile("integration_palette.pal", "");
    ASSERT_FALSE(temp_file.empty());

    // Save palette
    EXPECT_TRUE(palette_manager_save(&palette_manager, temp_file.c_str()));

    // Verify file operations
    EXPECT_TRUE(file_exists(temp_file.c_str()));
    EXPECT_GT(file_get_size(temp_file.c_str()), 0);

    // Load palette in new instance
    PaletteManager loaded_palette;
    ASSERT_TRUE(palette_manager_init(&loaded_palette));
    EXPECT_TRUE(palette_manager_load(&loaded_palette, temp_file.c_str()));

    // Use loaded palette for rendering
    TextRenderer text_renderer;
    ASSERT_TRUE(text_renderer_init(&text_renderer, sdl_get_renderer(&test_context)));

    SDL_Color loaded_color = palette_get_sdl_color(&loaded_palette, 2);
    EXPECT_NO_FATAL_FAILURE(
        text_render_string(&text_renderer, "Loaded Palette", 10, 10, loaded_color));

    text_renderer_cleanup(&text_renderer);
    RemoveTempTestFile(temp_file);
}

// ===== Performance Integration Tests =====

TEST_F(SDLComponentsIntegrationTest, SystemPerformance) {
    ASSERT_TRUE(sdl_init_context_simple(&test_context, "Performance Test", 800, 600));

    // Initialize all components
    TextRenderer text_renderer;
    PaletteManager palette_manager;
    UIButtonArray button_array;
    DoubleClickDetector double_click;

    ASSERT_TRUE(text_renderer_init(&text_renderer, sdl_get_renderer(&test_context)));
    ASSERT_TRUE(palette_manager_init(&palette_manager));
    ASSERT_TRUE(ui_button_array_init(&button_array, 20));
    double_click_init(&double_click, 500);

    CreateTestPalette(&palette_manager);

    // Create multiple buttons
    for (int i = 0; i < 20; ++i) {
        char text[16];
        snprintf(text, sizeof(text), "Btn%d", i);
        UIButton button = CreateTestButton((i % 5) * 150, (i / 5) * 50, 140, 40, text);
        ui_button_array_add(&button_array, &button);
    }

    auto start = std::chrono::high_resolution_clock::now();

    // Simulate multiple frames of rendering
    for (int frame = 0; frame < 50; ++frame) {
        // Clear screen
        SDL_Color bg_color = palette_get_sdl_color(&palette_manager, 0);
        sdl_clear_screen(&test_context, bg_color);

        // Render text
        SDL_Color text_color = palette_get_sdl_color(&palette_manager, 1);
        char frame_text[32];
        snprintf(frame_text, sizeof(frame_text), "Frame %d", frame);
        text_render_string(&text_renderer, frame_text, 10, 10, text_color);

        // Render buttons
        ui_button_array_render(&button_array, sdl_get_renderer(&test_context), &text_renderer);

        // Present frame
        sdl_present(&test_context);

        // Simulate input processing
        ui_button_array_handle_input(&button_array, frame % 800, frame % 600, frame % 10 == 0);
        if (frame % 10 == 0) {
            double_click_check(&double_click, 1);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // 50 frames should complete reasonably quickly
    EXPECT_LT(duration.count(), 2000)
        << "50 frame integration test took " << duration.count() << "ms";

    text_renderer_cleanup(&text_renderer);
    ui_button_array_cleanup(&button_array);
}

// ===== Error Recovery Integration Tests =====

TEST_F(SDLComponentsIntegrationTest, ErrorRecoveryBehavior) {
    // Test system behavior when components fail or are used incorrectly

    // Initialize SDL context
    ASSERT_TRUE(sdl_init_context_simple(&test_context, "Error Recovery", 640, 480));

    // Test component operations with missing dependencies
    TextRenderer text_renderer;
    EXPECT_TRUE(text_renderer_init(&text_renderer, sdl_get_renderer(&test_context)));

    // Destroy SDL context while text renderer still exists
    sdl_cleanup_context(&test_context);

    // Text renderer operations should handle gracefully
    EXPECT_NO_FATAL_FAILURE(
        text_render_string(&text_renderer, "Error Test", 0, 0, TEST_COLOR_WHITE));

    text_renderer_cleanup(&text_renderer);

    // Reinitialize SDL context
    EXPECT_TRUE(sdl_init_context_simple(&test_context, "Recovery", 640, 480));
    EXPECT_TRUE(sdl_context_is_ready(&test_context));
}

// ===== Configuration Integration Tests =====

TEST_F(SDLComponentsIntegrationTest, SystemConfiguration) {
    // Test different SDL configurations with components
    SDLContextConfig configs[] = {{"Small Window", 320, 240, false, false, false},
                                  {"Large Window", 1280, 720, true, true, false},
                                  {"Square Window", 500, 500, false, true, false}};

    for (size_t i = 0; i < sizeof(configs) / sizeof(configs[0]); ++i) {
        SDLContext config_context = {};
        EXPECT_TRUE(sdl_init_context(&config_context, &configs[i]));

        // Test text rendering in different window sizes
        TextRenderer text_renderer;
        EXPECT_TRUE(text_renderer_init(&text_renderer, sdl_get_renderer(&config_context)));

        EXPECT_NO_FATAL_FAILURE(
            text_render_string(&text_renderer, "Config Test", 10, 10, TEST_COLOR_WHITE));

        text_renderer_cleanup(&text_renderer);
        sdl_cleanup_context(&config_context);
    }
}

// ===== Multi-threaded Safety Tests =====

TEST_F(SDLComponentsIntegrationTest, ThreadSafetyBasics) {
    // Basic thread safety test (SDL is generally not thread-safe)
    ASSERT_TRUE(sdl_init_context_simple(&test_context, "Thread Test", 640, 480));

    PaletteManager palette_manager;
    ASSERT_TRUE(palette_manager_init(&palette_manager));

    std::atomic<int> operation_count{0};
    std::vector<std::thread> threads;

    // Test palette operations from multiple threads
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([&palette_manager, &operation_count, i]() {
            for (int j = 0; j < 100; ++j) {
                RGBA color = palette_make_color(i * 50, j % 256, (i + j) % 256, 255);
                palette_set_color(&palette_manager, j % PALETTE_COLOR_COUNT, color);
                palette_get_color(&palette_manager, j % PALETTE_COLOR_COUNT);
                operation_count++;
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(400, operation_count.load());
    EXPECT_TRUE(palette_manager_is_modified(&palette_manager));
}

// ===== Version Compatibility Tests =====

TEST_F(SDLComponentsIntegrationTest, VersionCompatibility) {
    // Test that all components report consistent version information
    const char* lib_version = shared_components_get_version();
    EXPECT_NE(nullptr, lib_version);

    int sdl_major, sdl_minor, sdl_patch;
    sdl_get_version(&sdl_major, &sdl_minor, &sdl_patch);

    EXPECT_GE(sdl_major, 3);  // Should be SDL3 or later

    // All components should work together regardless of version
    ASSERT_TRUE(sdl_init_context_simple(&test_context, "Version Test", 640, 480));

    TextRenderer text_renderer;
    PaletteManager palette_manager;
    ASSERT_TRUE(text_renderer_init(&text_renderer, sdl_get_renderer(&test_context)));
    ASSERT_TRUE(palette_manager_init(&palette_manager));

    // Render version information
    char version_text[128];
    snprintf(version_text, sizeof(version_text), "Shared Components: %s", lib_version);
    EXPECT_NO_FATAL_FAILURE(
        text_render_string(&text_renderer, version_text, 10, 10, TEST_COLOR_WHITE));

    snprintf(version_text, sizeof(version_text), "SDL: %d.%d.%d", sdl_major, sdl_minor, sdl_patch);
    EXPECT_NO_FATAL_FAILURE(
        text_render_string(&text_renderer, version_text, 10, 30, TEST_COLOR_GREEN));

    text_renderer_cleanup(&text_renderer);
}
