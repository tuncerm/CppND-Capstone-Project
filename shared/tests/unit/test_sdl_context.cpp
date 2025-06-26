/**
 * Unit Tests for SDL Context Component
 *
 * Tests SDL3 context management including initialization, cleanup,
 * window management, and rendering utilities.
 */

#include <gtest/gtest.h>
#include "../utils/test_helpers.h"
#include "sdl_framework/sdl_context.h"

class SDLContextTest : public ::testing::Test {
   protected:
    void SetUp() override {
        // SDL should already be initialized by global test environment
        ASSERT_TRUE(SDL_WasInit(SDL_INIT_VIDEO));
    }

    void TearDown() override {
        // Clean up any contexts created during tests
        if (sdl_context_is_ready(&test_context)) {
            sdl_cleanup_context(&test_context);
        }
    }

    SDLContext test_context = {};
};

// ===== Context Initialization Tests =====

TEST_F(SDLContextTest, SimpleInitialization) {
    EXPECT_TRUE(sdl_init_context_simple(&test_context, "Test Window", 640, 480));
    EXPECT_TRUE(sdl_context_is_ready(&test_context));
    EXPECT_TRUE(test_context.initialized);
    EXPECT_EQ(640, test_context.width);
    EXPECT_EQ(480, test_context.height);
    EXPECT_STREQ("Test Window", test_context.title);

    // Verify SDL objects are created
    EXPECT_NE(nullptr, sdl_get_window(&test_context));
    EXPECT_NE(nullptr, sdl_get_renderer(&test_context));
}

TEST_F(SDLContextTest, ConfigurationInitialization) {
    SDLContextConfig config = {.title = "Config Test",
                               .width = 800,
                               .height = 600,
                               .resizable = true,
                               .vsync = true,
                               .fullscreen = false};

    EXPECT_TRUE(sdl_init_context(&test_context, &config));
    EXPECT_TRUE(sdl_context_is_ready(&test_context));
    EXPECT_EQ(800, test_context.width);
    EXPECT_EQ(600, test_context.height);
    EXPECT_STREQ("Config Test", test_context.title);
    EXPECT_TRUE(test_context.vsync_enabled);
}

TEST_F(SDLContextTest, InitializationWithNullPointers) {
    EXPECT_FALSE(sdl_init_context_simple(nullptr, "Test", 640, 480));
    EXPECT_FALSE(sdl_init_context_simple(&test_context, nullptr, 640, 480));
    EXPECT_FALSE(sdl_init_context(nullptr, nullptr));
    EXPECT_FALSE(sdl_init_context(&test_context, nullptr));
}

TEST_F(SDLContextTest, InitializationWithInvalidDimensions) {
    EXPECT_FALSE(sdl_init_context_simple(&test_context, "Test", 0, 480));
    EXPECT_FALSE(sdl_init_context_simple(&test_context, "Test", 640, 0));
    EXPECT_FALSE(sdl_init_context_simple(&test_context, "Test", -100, 480));
    EXPECT_FALSE(sdl_init_context_simple(&test_context, "Test", 640, -100));
}

TEST_F(SDLContextTest, DoubleInitialization) {
    ASSERT_TRUE(sdl_init_context_simple(&test_context, "First", 640, 480));

    // Second initialization should fail or replace the first
    bool second_init = sdl_init_context_simple(&test_context, "Second", 800, 600);

    if (second_init) {
        // If second init succeeded, it should have replaced the first
        EXPECT_EQ(800, test_context.width);
        EXPECT_EQ(600, test_context.height);
    } else {
        // If second init failed, first should still be valid
        EXPECT_EQ(640, test_context.width);
        EXPECT_EQ(480, test_context.height);
    }

    EXPECT_TRUE(sdl_context_is_ready(&test_context));
}

// ===== Context Cleanup Tests =====

TEST_F(SDLContextTest, CleanupAfterInit) {
    ASSERT_TRUE(sdl_init_context_simple(&test_context, "Test", 640, 480));
    EXPECT_TRUE(sdl_context_is_ready(&test_context));

    sdl_cleanup_context(&test_context);
    EXPECT_FALSE(sdl_context_is_ready(&test_context));
    EXPECT_FALSE(test_context.initialized);
    EXPECT_EQ(nullptr, sdl_get_window(&test_context));
    EXPECT_EQ(nullptr, sdl_get_renderer(&test_context));
}

TEST_F(SDLContextTest, CleanupWithoutInit) {
    EXPECT_FALSE(sdl_context_is_ready(&test_context));
    EXPECT_NO_FATAL_FAILURE(sdl_cleanup_context(&test_context));
    EXPECT_FALSE(sdl_context_is_ready(&test_context));
}

TEST_F(SDLContextTest, CleanupWithNullPointer) {
    EXPECT_NO_FATAL_FAILURE(sdl_cleanup_context(nullptr));
}

TEST_F(SDLContextTest, MultipleCleanups) {
    ASSERT_TRUE(sdl_init_context_simple(&test_context, "Test", 640, 480));

    sdl_cleanup_context(&test_context);
    EXPECT_FALSE(sdl_context_is_ready(&test_context));

    // Second cleanup should be safe
    EXPECT_NO_FATAL_FAILURE(sdl_cleanup_context(&test_context));
    EXPECT_FALSE(sdl_context_is_ready(&test_context));
}

// ===== Window Management Tests =====

TEST_F(SDLContextTest, WindowTitleOperations) {
    ASSERT_TRUE(sdl_init_context_simple(&test_context, "Initial Title", 640, 480));

    sdl_set_window_title(&test_context, "New Title");
    EXPECT_STREQ("New Title", test_context.title);

    // Verify SDL window title was actually changed
    const char* sdl_title = SDL_GetWindowTitle(sdl_get_window(&test_context));
    EXPECT_STREQ("New Title", sdl_title);
}

TEST_F(SDLContextTest, WindowSizeOperations) {
    ASSERT_TRUE(sdl_init_context_simple(&test_context, "Test", 640, 480));

    int width, height;
    sdl_get_window_size(&test_context, &width, &height);
    EXPECT_EQ(640, width);
    EXPECT_EQ(480, height);

    sdl_set_window_size(&test_context, 800, 600);
    EXPECT_EQ(800, test_context.width);
    EXPECT_EQ(600, test_context.height);

    sdl_get_window_size(&test_context, &width, &height);
    EXPECT_EQ(800, width);
    EXPECT_EQ(600, height);
}

TEST_F(SDLContextTest, FullscreenToggle) {
    ASSERT_TRUE(sdl_init_context_simple(&test_context, "Test", 640, 480));

    // Test enabling fullscreen
    EXPECT_TRUE(sdl_set_fullscreen(&test_context, true));

    // Test disabling fullscreen
    EXPECT_TRUE(sdl_set_fullscreen(&test_context, false));
}

// ===== Rendering Tests =====

TEST_F(SDLContextTest, ClearScreen) {
    ASSERT_TRUE(sdl_init_context_simple(&test_context, "Test", 640, 480));

    SDL_Color clear_colors[] = {
        {0, 0, 0, 255},        // Black
        {255, 255, 255, 255},  // White
        {255, 0, 0, 255},      // Red
        {0, 255, 0, 255},      // Green
        {0, 0, 255, 255}       // Blue
    };

    for (size_t i = 0; i < sizeof(clear_colors) / sizeof(clear_colors[0]); ++i) {
        EXPECT_NO_FATAL_FAILURE(sdl_clear_screen(&test_context, clear_colors[i]));
    }
}

TEST_F(SDLContextTest, PresentFrame) {
    ASSERT_TRUE(sdl_init_context_simple(&test_context, "Test", 640, 480));

    EXPECT_NO_FATAL_FAILURE(sdl_present(&test_context));
}

TEST_F(SDLContextTest, LogicalPresentation) {
    ASSERT_TRUE(sdl_init_context_simple(&test_context, "Test", 640, 480));

    EXPECT_TRUE(sdl_set_logical_presentation(&test_context, 320, 240));
    EXPECT_TRUE(sdl_set_logical_presentation(&test_context, 1280, 720));

    // Test with invalid dimensions
    EXPECT_FALSE(sdl_set_logical_presentation(&test_context, 0, 240));
    EXPECT_FALSE(sdl_set_logical_presentation(&test_context, 320, 0));
}

// ===== Utility Function Tests =====

TEST_F(SDLContextTest, ErrorHandling) {
    const char* error = sdl_get_error();
    EXPECT_NE(nullptr, error);

    // Error string should be accessible
    size_t error_len = strlen(error);
    EXPECT_GE(error_len, 0);  // May be empty if no error
}

TEST_F(SDLContextTest, ErrorPrinting) {
    // Should not crash
    EXPECT_NO_FATAL_FAILURE(sdl_print_error("Test error message"));
}

TEST_F(SDLContextTest, SubsystemCheck) {
    EXPECT_TRUE(sdl_is_subsystem_initialized(SDL_INIT_VIDEO));

    // Audio might not be initialized
    bool audio_init = sdl_is_subsystem_initialized(SDL_INIT_AUDIO);
    EXPECT_TRUE(audio_init || !audio_init);  // Either state is valid
}

TEST_F(SDLContextTest, VersionInfo) {
    int major, minor, patch;
    sdl_get_version(&major, &minor, &patch);

    EXPECT_GE(major, 3);  // SDL3 or later
    EXPECT_GE(minor, 0);
    EXPECT_GE(patch, 0);
}

// ===== Accessor Function Tests =====

TEST_F(SDLContextTest, WindowAccessor) {
    EXPECT_EQ(nullptr, sdl_get_window(&test_context));  // Not initialized

    ASSERT_TRUE(sdl_init_context_simple(&test_context, "Test", 640, 480));

    SDL_Window* window = sdl_get_window(&test_context);
    EXPECT_NE(nullptr, window);
    EXPECT_EQ(window, test_context.window);
}

TEST_F(SDLContextTest, RendererAccessor) {
    EXPECT_EQ(nullptr, sdl_get_renderer(&test_context));  // Not initialized

    ASSERT_TRUE(sdl_init_context_simple(&test_context, "Test", 640, 480));

    SDL_Renderer* renderer = sdl_get_renderer(&test_context);
    EXPECT_NE(nullptr, renderer);
    EXPECT_EQ(renderer, test_context.renderer);
}

TEST_F(SDLContextTest, NullPointerAccessors) {
    EXPECT_EQ(nullptr, sdl_get_window(nullptr));
    EXPECT_EQ(nullptr, sdl_get_renderer(nullptr));
    EXPECT_FALSE(sdl_context_is_ready(nullptr));
}

// ===== Integration Tests =====

TEST_F(SDLContextTest, ContextReadinessAfterOperations) {
    ASSERT_TRUE(sdl_init_context_simple(&test_context, "Test", 640, 480));
    EXPECT_TRUE(VerifySDLContext(&test_context));

    // Context should remain ready after various operations
    sdl_set_window_title(&test_context, "New Title");
    EXPECT_TRUE(VerifySDLContext(&test_context));

    sdl_set_window_size(&test_context, 800, 600);
    EXPECT_TRUE(VerifySDLContext(&test_context));

    sdl_clear_screen(&test_context, {100, 100, 100, 255});
    EXPECT_TRUE(VerifySDLContext(&test_context));

    sdl_present(&test_context);
    EXPECT_TRUE(VerifySDLContext(&test_context));
}

TEST_F(SDLContextTest, MultipleContexts) {
    SDLContext context1 = {}, context2 = {};

    EXPECT_TRUE(sdl_init_context_simple(&context1, "Context 1", 640, 480));
    EXPECT_TRUE(sdl_init_context_simple(&context2, "Context 2", 800, 600));

    EXPECT_TRUE(sdl_context_is_ready(&context1));
    EXPECT_TRUE(sdl_context_is_ready(&context2));

    EXPECT_NE(sdl_get_window(&context1), sdl_get_window(&context2));
    EXPECT_NE(sdl_get_renderer(&context1), sdl_get_renderer(&context2));

    sdl_cleanup_context(&context1);
    sdl_cleanup_context(&context2);
}

// ===== Performance Tests =====

TEST_F(SDLContextTest, InitializationPerformance) {
    auto start = std::chrono::high_resolution_clock::now();

    ASSERT_TRUE(sdl_init_context_simple(&test_context, "Performance Test", 640, 480));

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // SDL context initialization should be reasonably fast
    EXPECT_LT(duration.count(), 1000)
        << "SDL context initialization took " << duration.count() << "ms";
}

TEST_F(SDLContextTest, RenderingPerformance) {
    ASSERT_TRUE(sdl_init_context_simple(&test_context, "Test", 640, 480));

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 100; ++i) {
        sdl_clear_screen(&test_context, {i % 256, (i * 2) % 256, (i * 3) % 256, 255});
        sdl_present(&test_context);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // 100 clear/present cycles should complete reasonably quickly
    EXPECT_LT(duration.count(), 5000) << "100 render cycles took " << duration.count() << "ms";
}

// ===== Error Condition Tests =====

TEST_F(SDLContextTest, OperationsOnUninitializedContext) {
    // Operations on uninitialized context should be safe
    EXPECT_NO_FATAL_FAILURE(sdl_set_window_title(&test_context, "Test"));
    EXPECT_NO_FATAL_FAILURE(sdl_set_window_size(&test_context, 800, 600));
    EXPECT_NO_FATAL_FAILURE(sdl_clear_screen(&test_context, {0, 0, 0, 255}));
    EXPECT_NO_FATAL_FAILURE(sdl_present(&test_context));
}

TEST_F(SDLContextTest, OperationsAfterCleanup) {
    ASSERT_TRUE(sdl_init_context_simple(&test_context, "Test", 640, 480));
    sdl_cleanup_context(&test_context);

    // Operations after cleanup should be safe
    EXPECT_NO_FATAL_FAILURE(sdl_set_window_title(&test_context, "Test"));
    EXPECT_NO_FATAL_FAILURE(sdl_set_window_size(&test_context, 800, 600));
    EXPECT_NO_FATAL_FAILURE(sdl_clear_screen(&test_context, {0, 0, 0, 255}));
    EXPECT_NO_FATAL_FAILURE(sdl_present(&test_context));
}

// ===== Configuration Edge Cases =====

TEST_F(SDLContextTest, ExtremeWindowSizes) {
    // Test very small window
    EXPECT_TRUE(sdl_init_context_simple(&test_context, "Small", 1, 1));
    EXPECT_TRUE(sdl_context_is_ready(&test_context));
    sdl_cleanup_context(&test_context);

    // Test large window (may fail on some systems, but shouldn't crash)
    bool large_window = sdl_init_context_simple(&test_context, "Large", 3840, 2160);
    if (large_window) {
        EXPECT_TRUE(sdl_context_is_ready(&test_context));
    }
}

TEST_F(SDLContextTest, LongWindowTitle) {
    std::string long_title(1000, 'A');  // 1000 character title

    EXPECT_TRUE(sdl_init_context_simple(&test_context, long_title.c_str(), 640, 480));
    EXPECT_TRUE(sdl_context_is_ready(&test_context));

    // Title should be truncated to fit buffer
    EXPECT_LT(strlen(test_context.title), long_title.length());
    EXPECT_LT(strlen(test_context.title), sizeof(test_context.title));
}
