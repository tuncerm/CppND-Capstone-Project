/**
 * Test Helpers Implementation for Shared Components Library Tests
 */

#include "test_helpers.h"
#include <cstdio>
#include <filesystem>
#include <fstream>

// ===== Test Fixture Implementations =====

void SDLTestFixture::SetUp() {
    // SDL should already be initialized by the global test environment
    ASSERT_TRUE(SDL_WasInit(SDL_INIT_VIDEO)) << "SDL video subsystem not initialized";

    // Initialize SDL context for this test
    sdl_initialized = sdl_init_context_simple(&sdl_context, TEST_WINDOW_TITLE, TEST_WINDOW_WIDTH,
                                              TEST_WINDOW_HEIGHT);
    ASSERT_TRUE(sdl_initialized) << "Failed to initialize SDL context for test";

    // Hide the test window to avoid desktop clutter during automated testing
    SDL_HideWindow(sdl_get_window(&sdl_context));
}

void SDLTestFixture::TearDown() {
    if (sdl_initialized) {
        sdl_cleanup_context(&sdl_context);
        sdl_initialized = false;
    }
}

void TextRendererTestFixture::SetUp() {
    // Set up SDL context first
    SDLTestFixture::SetUp();

    // Initialize text renderer
    text_renderer_initialized = text_renderer_init(&text_renderer, sdl_get_renderer(&sdl_context));
    ASSERT_TRUE(text_renderer_initialized) << "Failed to initialize text renderer for test";
}

void TextRendererTestFixture::TearDown() {
    if (text_renderer_initialized) {
        text_renderer_cleanup(&text_renderer);
        text_renderer_initialized = false;
    }

    // Clean up SDL context
    SDLTestFixture::TearDown();
}

void PaletteManagerTestFixture::SetUp() {
    palette_initialized = palette_manager_init(&palette_manager);
    ASSERT_TRUE(palette_initialized) << "Failed to initialize palette manager for test";
}

void PaletteManagerTestFixture::TearDown() {
    // Palette manager doesn't require explicit cleanup currently
    palette_initialized = false;
}

// ===== Test Utility Implementations =====

std::string CreateTempTestFile(const char* filename, const char* content) {
    std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
    std::filesystem::path file_path = temp_dir / filename;

    std::ofstream file(file_path);
    if (file.is_open()) {
        file << content;
        file.close();
        return file_path.string();
    }

    return "";
}

void RemoveTempTestFile(const std::string& filepath) {
    if (!filepath.empty()) {
        std::filesystem::remove(filepath);
    }
}

bool ColorsEqual(const RGBA& a, const RGBA& b) {
    return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}

bool ColorsEqual(const SDL_Color& a, const SDL_Color& b) {
    return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}

void CreateTestPalette(PaletteManager* pm) {
    // Create a known test palette with specific colors
    const RGBA test_colors[PALETTE_COLOR_COUNT] = {
        {0, 0, 0, 255},        // 0: Black
        {255, 255, 255, 255},  // 1: White
        {255, 0, 0, 255},      // 2: Red
        {0, 255, 0, 255},      // 3: Green
        {0, 0, 255, 255},      // 4: Blue
        {255, 255, 0, 255},    // 5: Yellow
        {255, 0, 255, 255},    // 6: Magenta
        {0, 255, 255, 255},    // 7: Cyan
        {128, 128, 128, 255},  // 8: Gray
        {192, 192, 192, 255},  // 9: Light Gray
        {128, 0, 0, 255},      // 10: Dark Red
        {0, 128, 0, 255},      // 11: Dark Green
        {0, 0, 128, 255},      // 12: Dark Blue
        {128, 128, 0, 255},    // 13: Dark Yellow
        {128, 0, 128, 255},    // 14: Dark Magenta
        {0, 128, 128, 255}     // 15: Dark Cyan
    };

    for (int i = 0; i < PALETTE_COLOR_COUNT; i++) {
        palette_set_color(pm, i, test_colors[i]);
    }
}

bool ValidateFontData() {
    // Test basic font data access
    try {
        // This should not crash if font data is properly initialized
        font_validate_data();
        return true;
    } catch (...) {
        return false;
    }
}

bool PointInRect(int x, int y, const SDL_FRect& rect) {
    return x >= rect.x && x < rect.x + rect.w && y >= rect.y && y < rect.y + rect.h;
}

UIButton CreateTestButton(int x, int y, int w, int h, const char* text) {
    UIButton button;
    ui_button_init(&button, x, y, w, h, text);

    // Set up standard test colors
    ui_button_set_colors(&button, {200, 200, 200, 255},  // normal
                         {220, 220, 220, 255},           // hover
                         {180, 180, 180, 255},           // pressed
                         {100, 100, 100, 255}            // disabled
    );

    ui_button_set_text_color(&button, {0, 0, 0, 255});  // black text

    return button;
}

bool VerifySDLContext(const SDLContext* ctx) {
    if (!ctx || !sdl_context_is_ready(ctx)) {
        return false;
    }

    SDL_Window* window = sdl_get_window(ctx);
    SDL_Renderer* renderer = sdl_get_renderer(ctx);

    if (!window || !renderer) {
        return false;
    }

    // Verify window properties
    int w, h;
    SDL_GetWindowSize(window, &w, &h);

    return w > 0 && h > 0;
}
