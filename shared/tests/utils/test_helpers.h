/**
 * Test Helpers for Shared Components Library Tests
 *
 * Common utilities, fixtures, and helper functions for testing
 * shared components functionality.
 */

#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <SDL3/SDL.h>
#include <gtest/gtest.h>
#include "shared_components.h"

// ===== Test Constants =====

constexpr int TEST_WINDOW_WIDTH = 800;
constexpr int TEST_WINDOW_HEIGHT = 600;
constexpr const char* TEST_WINDOW_TITLE = "Shared Components Test Window";

// Test colors
constexpr SDL_Color TEST_COLOR_WHITE = {255, 255, 255, 255};
constexpr SDL_Color TEST_COLOR_BLACK = {0, 0, 0, 255};
constexpr SDL_Color TEST_COLOR_RED = {255, 0, 0, 255};
constexpr SDL_Color TEST_COLOR_GREEN = {0, 255, 0, 255};
constexpr SDL_Color TEST_COLOR_BLUE = {0, 0, 255, 255};

// ===== Test Fixtures =====

/**
 * Base test fixture for tests that need SDL context
 */
class SDLTestFixture : public ::testing::Test {
   protected:
    void SetUp() override;
    void TearDown() override;

    SDLContext sdl_context;
    bool sdl_initialized = false;
};

/**
 * Test fixture for tests that need both SDL context and text renderer
 */
class TextRendererTestFixture : public SDLTestFixture {
   protected:
    void SetUp() override;
    void TearDown() override;

    TextRenderer text_renderer;
    bool text_renderer_initialized = false;
};

/**
 * Test fixture for palette manager tests
 */
class PaletteManagerTestFixture : public ::testing::Test {
   protected:
    void SetUp() override;
    void TearDown() override;

    PaletteManager palette_manager;
    bool palette_initialized = false;
};

// ===== Test Utilities =====

/**
 * Create a temporary test file with specified content
 * @param filename Filename for temporary file
 * @param content File content
 * @return Full path to created file
 */
std::string CreateTempTestFile(const char* filename, const char* content);

/**
 * Remove temporary test file
 * @param filepath Path to file to remove
 */
void RemoveTempTestFile(const std::string& filepath);

/**
 * Compare two RGBA colors for equality
 * @param a First color
 * @param b Second color
 * @return true if colors are equal
 */
bool ColorsEqual(const RGBA& a, const RGBA& b);

/**
 * Compare two SDL_Color structures for equality
 * @param a First color
 * @param b Second color
 * @return true if colors are equal
 */
bool ColorsEqual(const SDL_Color& a, const SDL_Color& b);

/**
 * Create test palette with known colors
 * @param pm Palette manager to initialize
 */
void CreateTestPalette(PaletteManager* pm);

/**
 * Validate that font data is accessible and valid
 * @return true if font data appears valid
 */
bool ValidateFontData();

/**
 * Test if point is within rectangle
 * @param x Point X coordinate
 * @param y Point Y coordinate
 * @param rect Rectangle to test against
 * @return true if point is inside rectangle
 */
bool PointInRect(int x, int y, const SDL_FRect& rect);

/**
 * Create test button with standard configuration
 * @param x Button X position
 * @param y Button Y position
 * @param w Button width
 * @param h Button height
 * @param text Button text
 * @return Configured test button
 */
UIButton CreateTestButton(int x, int y, int w, int h, const char* text);

/**
 * Verify SDL context is properly initialized
 * @param ctx SDL context to verify
 * @return true if context is valid and ready
 */
bool VerifySDLContext(const SDLContext* ctx);

// ===== Test Macros =====

#define EXPECT_RGBA_EQ(expected, actual)                               \
    EXPECT_EQ((expected).r, (actual).r) << "Red component mismatch";   \
    EXPECT_EQ((expected).g, (actual).g) << "Green component mismatch"; \
    EXPECT_EQ((expected).b, (actual).b) << "Blue component mismatch";  \
    EXPECT_EQ((expected).a, (actual).a) << "Alpha component mismatch"

#define EXPECT_SDL_COLOR_EQ(expected, actual)                          \
    EXPECT_EQ((expected).r, (actual).r) << "Red component mismatch";   \
    EXPECT_EQ((expected).g, (actual).g) << "Green component mismatch"; \
    EXPECT_EQ((expected).b, (actual).b) << "Blue component mismatch";  \
    EXPECT_EQ((expected).a, (actual).a) << "Alpha component mismatch"

#define ASSERT_SDL_SUCCESS(call) ASSERT_EQ(0, (call)) << "SDL Error: " << SDL_GetError()

#define EXPECT_SDL_SUCCESS(call) EXPECT_EQ(0, (call)) << "SDL Error: " << SDL_GetError()

// ===== Callback Test Helpers =====

/**
 * Simple callback counter for testing button callbacks
 */
struct CallbackCounter {
    int count = 0;
    void* last_userdata = nullptr;

    static void Callback(void* userdata) {
        CallbackCounter* counter = static_cast<CallbackCounter*>(userdata);
        counter->count++;
        counter->last_userdata = userdata;
    }

    void Reset() {
        count = 0;
        last_userdata = nullptr;
    }
};

#endif  // TEST_HELPERS_H
