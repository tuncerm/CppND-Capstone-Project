/**
 * Unit Tests for Palette Manager Component
 *
 * Tests palette management functionality including color management,
 * file I/O operations, and RGBA conversion utilities.
 */

#include <gtest/gtest.h>
#include "../utils/test_helpers.h"
#include "palette_manager/palette_manager.h"

class PaletteManagerTest : public PaletteManagerTestFixture {
    // Test fixture provides initialized palette_manager
};

// ===== Initialization Tests =====

TEST_F(PaletteManagerTest, InitializationSuccess) {
    EXPECT_TRUE(palette_initialized);
    EXPECT_FALSE(palette_manager_is_modified(&palette_manager));
    EXPECT_FALSE(palette_manager.file_loaded);
    EXPECT_EQ('\0', palette_manager.current_file[0]);
}

TEST_F(PaletteManagerTest, InitializationWithNullPointer) {
    EXPECT_FALSE(palette_manager_init(nullptr));
}

TEST_F(PaletteManagerTest, DefaultPaletteColors) {
    // After initialization, palette should have default colors
    for (int i = 0; i < PALETTE_COLOR_COUNT; ++i) {
        RGBA color = palette_get_color(&palette_manager, i);
        // All default colors should have full alpha
        EXPECT_EQ(255, color.a) << "Color " << i << " should have full alpha";
    }
}

// ===== Color Management Tests =====

TEST_F(PaletteManagerTest, GetValidColorIndex) {
    RGBA color = palette_get_color(&palette_manager, 0);
    // Should return a valid color (test depends on default palette)
    EXPECT_GE(color.r, 0);
    EXPECT_LE(color.r, 255);
    EXPECT_GE(color.g, 0);
    EXPECT_LE(color.g, 255);
    EXPECT_GE(color.b, 0);
    EXPECT_LE(color.b, 255);
    EXPECT_GE(color.a, 0);
    EXPECT_LE(color.a, 255);
}

TEST_F(PaletteManagerTest, GetInvalidColorIndex) {
    // Invalid indices should return black
    RGBA black_color = palette_get_color(&palette_manager, -1);
    EXPECT_EQ(0, black_color.r);
    EXPECT_EQ(0, black_color.g);
    EXPECT_EQ(0, black_color.b);
    EXPECT_EQ(255, black_color.a);  // Black with full alpha

    black_color = palette_get_color(&palette_manager, PALETTE_COLOR_COUNT);
    EXPECT_EQ(0, black_color.r);
    EXPECT_EQ(0, black_color.g);
    EXPECT_EQ(0, black_color.b);
    EXPECT_EQ(255, black_color.a);
}

TEST_F(PaletteManagerTest, SetValidColorIndex) {
    RGBA test_color = palette_make_color(128, 64, 192, 200);

    EXPECT_TRUE(palette_set_color(&palette_manager, 5, test_color));
    EXPECT_TRUE(palette_manager_is_modified(&palette_manager));

    RGBA retrieved = palette_get_color(&palette_manager, 5);
    EXPECT_RGBA_EQ(test_color, retrieved);
}

TEST_F(PaletteManagerTest, SetInvalidColorIndex) {
    RGBA test_color = palette_make_color(128, 64, 192, 200);

    EXPECT_FALSE(palette_set_color(&palette_manager, -1, test_color));
    EXPECT_FALSE(palette_set_color(&palette_manager, PALETTE_COLOR_COUNT, test_color));

    // Should not be marked as modified for invalid operations
    EXPECT_FALSE(palette_manager_is_modified(&palette_manager));
}

TEST_F(PaletteManagerTest, SetAllColors) {
    CreateTestPalette(&palette_manager);
    EXPECT_TRUE(palette_manager_is_modified(&palette_manager));

    // Verify specific test colors
    RGBA black = palette_get_color(&palette_manager, 0);
    EXPECT_EQ(0, black.r);
    EXPECT_EQ(0, black.g);
    EXPECT_EQ(0, black.b);
    EXPECT_EQ(255, black.a);

    RGBA white = palette_get_color(&palette_manager, 1);
    EXPECT_EQ(255, white.r);
    EXPECT_EQ(255, white.g);
    EXPECT_EQ(255, white.b);
    EXPECT_EQ(255, white.a);

    RGBA red = palette_get_color(&palette_manager, 2);
    EXPECT_EQ(255, red.r);
    EXPECT_EQ(0, red.g);
    EXPECT_EQ(0, red.b);
    EXPECT_EQ(255, red.a);
}

// ===== Color Conversion Tests =====

TEST_F(PaletteManagerTest, MakeColorFunction) {
    RGBA color = palette_make_color(100, 150, 200, 250);
    EXPECT_EQ(100, color.r);
    EXPECT_EQ(150, color.g);
    EXPECT_EQ(200, color.b);
    EXPECT_EQ(250, color.a);
}

TEST_F(PaletteManagerTest, SDLColorConversion) {
    palette_set_color(&palette_manager, 0, palette_make_color(64, 128, 192, 255));

    SDL_Color sdl_color = palette_get_sdl_color(&palette_manager, 0);
    EXPECT_EQ(64, sdl_color.r);
    EXPECT_EQ(128, sdl_color.g);
    EXPECT_EQ(192, sdl_color.b);
    EXPECT_EQ(255, sdl_color.a);
}

TEST_F(PaletteManagerTest, FromSDLColor) {
    SDL_Color sdl_color = {75, 100, 125, 150};
    RGBA rgba_color = palette_from_sdl_color(sdl_color);

    EXPECT_EQ(75, rgba_color.r);
    EXPECT_EQ(100, rgba_color.g);
    EXPECT_EQ(125, rgba_color.b);
    EXPECT_EQ(150, rgba_color.a);
}

// ===== Modification Tracking Tests =====

TEST_F(PaletteManagerTest, ModificationFlag) {
    EXPECT_FALSE(palette_manager_is_modified(&palette_manager));

    palette_set_color(&palette_manager, 0, palette_make_color(1, 2, 3, 4));
    EXPECT_TRUE(palette_manager_is_modified(&palette_manager));

    palette_manager_clear_modified(&palette_manager);
    EXPECT_FALSE(palette_manager_is_modified(&palette_manager));

    palette_manager_mark_modified(&palette_manager);
    EXPECT_TRUE(palette_manager_is_modified(&palette_manager));
}

// ===== Reset Functionality Tests =====

TEST_F(PaletteManagerTest, ResetToDefault) {
    // Modify the palette
    CreateTestPalette(&palette_manager);
    EXPECT_TRUE(palette_manager_is_modified(&palette_manager));

    // Reset to default
    palette_manager_reset_to_default(&palette_manager);
    EXPECT_TRUE(palette_manager_is_modified(&palette_manager));  // Reset marks as modified

    // Should have default colors again
    // (Exact verification depends on what the default palette contains)
    for (int i = 0; i < PALETTE_COLOR_COUNT; ++i) {
        RGBA color = palette_get_color(&palette_manager, i);
        EXPECT_EQ(255, color.a);  // All default colors should have full alpha
    }
}

// ===== File I/O Tests =====

TEST_F(PaletteManagerTest, SaveAndLoadPalette) {
    // Create a test palette
    CreateTestPalette(&palette_manager);

    // Save to temporary file
    std::string temp_file = CreateTempTestFile("test_palette.pal", "");
    ASSERT_FALSE(temp_file.empty());

    EXPECT_TRUE(palette_manager_save(&palette_manager, temp_file.c_str()));
    EXPECT_FALSE(palette_manager_is_modified(&palette_manager));  // Save clears modified flag
    EXPECT_STREQ(temp_file.c_str(), palette_manager_get_filename(&palette_manager));
    EXPECT_TRUE(palette_manager.file_loaded);

    // Create new palette and load
    PaletteManager loaded_palette;
    ASSERT_TRUE(palette_manager_init(&loaded_palette));

    EXPECT_TRUE(palette_manager_load(&loaded_palette, temp_file.c_str()));
    EXPECT_FALSE(palette_manager_is_modified(&loaded_palette));
    EXPECT_TRUE(loaded_palette.file_loaded);

    // Compare palettes
    EXPECT_TRUE(palette_manager_equals(&palette_manager, &loaded_palette));

    RemoveTempTestFile(temp_file);
}

TEST_F(PaletteManagerTest, LoadNonexistentFile) {
    EXPECT_FALSE(palette_manager_load(&palette_manager, "nonexistent_file.pal"));
    EXPECT_FALSE(palette_manager.file_loaded);
}

TEST_F(PaletteManagerTest, SaveToNullPath) {
    EXPECT_FALSE(palette_manager_save(&palette_manager, nullptr));
}

TEST_F(PaletteManagerTest, SaveWithoutCurrentFile) {
    // Save with no current file should fail when passing nullptr
    EXPECT_FALSE(palette_manager_save(&palette_manager, nullptr));
}

TEST_F(PaletteManagerTest, SaveWithCurrentFile) {
    std::string temp_file = CreateTempTestFile("current_palette.pal", "");
    ASSERT_FALSE(temp_file.empty());

    // First save to establish current file
    EXPECT_TRUE(palette_manager_save(&palette_manager, temp_file.c_str()));

    // Modify palette
    palette_set_color(&palette_manager, 0, palette_make_color(99, 88, 77, 255));
    EXPECT_TRUE(palette_manager_is_modified(&palette_manager));

    // Save with nullptr should use current file
    EXPECT_TRUE(palette_manager_save(&palette_manager, nullptr));
    EXPECT_FALSE(palette_manager_is_modified(&palette_manager));

    RemoveTempTestFile(temp_file);
}

// ===== File Validation Tests =====

TEST_F(PaletteManagerTest, ValidateValidFile) {
    CreateTestPalette(&palette_manager);
    std::string temp_file = CreateTempTestFile("valid_palette.pal", "");
    ASSERT_FALSE(temp_file.empty());

    ASSERT_TRUE(palette_manager_save(&palette_manager, temp_file.c_str()));
    EXPECT_TRUE(palette_manager_validate_file(temp_file.c_str()));

    RemoveTempTestFile(temp_file);
}

TEST_F(PaletteManagerTest, ValidateInvalidFile) {
    std::string temp_file = CreateTempTestFile("invalid_palette.pal", "not a palette file");
    ASSERT_FALSE(temp_file.empty());

    EXPECT_FALSE(palette_manager_validate_file(temp_file.c_str()));

    RemoveTempTestFile(temp_file);
}

TEST_F(PaletteManagerTest, ValidateNonexistentFile) {
    EXPECT_FALSE(palette_manager_validate_file("nonexistent.pal"));
}

// ===== Utility Function Tests =====

TEST_F(PaletteManagerTest, PaletteCopy) {
    CreateTestPalette(&palette_manager);

    PaletteManager copied_palette;
    ASSERT_TRUE(palette_manager_init(&copied_palette));

    palette_manager_copy(&copied_palette, &palette_manager);

    EXPECT_TRUE(palette_manager_equals(&palette_manager, &copied_palette));
    EXPECT_TRUE(palette_manager_is_modified(&copied_palette));  // Copy marks as modified
}

TEST_F(PaletteManagerTest, PaletteEquality) {
    PaletteManager palette1, palette2;
    ASSERT_TRUE(palette_manager_init(&palette1));
    ASSERT_TRUE(palette_manager_init(&palette2));

    // Initially equal (both have default palette)
    EXPECT_TRUE(palette_manager_equals(&palette1, &palette2));

    // Modify one palette
    palette_set_color(&palette1, 0, palette_make_color(123, 456, 789, 255));
    EXPECT_FALSE(palette_manager_equals(&palette1, &palette2));

    // Make them equal again
    palette_set_color(&palette2, 0, palette_make_color(123, 456, 789, 255));
    EXPECT_TRUE(palette_manager_equals(&palette1, &palette2));
}

// ===== Raw Data Tests =====

TEST_F(PaletteManagerTest, GetRawData) {
    CreateTestPalette(&palette_manager);

    uint8_t raw_data[64];  // RGBA format
    int bytes_written = palette_manager_get_raw_data(&palette_manager, raw_data);

    EXPECT_EQ(64, bytes_written);  // 16 colors * 4 bytes per color

    // Check first few colors
    EXPECT_EQ(0, raw_data[0]);    // Black R
    EXPECT_EQ(0, raw_data[1]);    // Black G
    EXPECT_EQ(0, raw_data[2]);    // Black B
    EXPECT_EQ(255, raw_data[3]);  // Black A

    EXPECT_EQ(255, raw_data[4]);  // White R
    EXPECT_EQ(255, raw_data[5]);  // White G
    EXPECT_EQ(255, raw_data[6]);  // White B
    EXPECT_EQ(255, raw_data[7]);  // White A
}

TEST_F(PaletteManagerTest, SetRawData) {
    uint8_t test_data[64];

    // Fill with test pattern
    for (int i = 0; i < 16; ++i) {
        test_data[i * 4 + 0] = i * 16;  // R
        test_data[i * 4 + 1] = i * 8;   // G
        test_data[i * 4 + 2] = i * 4;   // B
        test_data[i * 4 + 3] = 255;     // A
    }

    EXPECT_TRUE(palette_manager_set_raw_data(&palette_manager, test_data, 64));
    EXPECT_TRUE(palette_manager_is_modified(&palette_manager));

    // Verify colors were set correctly
    for (int i = 0; i < 16; ++i) {
        RGBA color = palette_get_color(&palette_manager, i);
        EXPECT_EQ(i * 16, color.r) << "Color " << i << " red component";
        EXPECT_EQ(i * 8, color.g) << "Color " << i << " green component";
        EXPECT_EQ(i * 4, color.b) << "Color " << i << " blue component";
        EXPECT_EQ(255, color.a) << "Color " << i << " alpha component";
    }
}

TEST_F(PaletteManagerTest, SetRawDataInvalidSize) {
    uint8_t test_data[32];  // Too small
    EXPECT_FALSE(palette_manager_set_raw_data(&palette_manager, test_data, 32));
    EXPECT_FALSE(palette_manager_is_modified(&palette_manager));
}

// ===== Error Handling Tests =====

TEST_F(PaletteManagerTest, NullPointerHandling) {
    RGBA test_color = palette_make_color(1, 2, 3, 4);

    // Functions should handle null palette gracefully
    EXPECT_FALSE(palette_manager_is_modified(nullptr));
    EXPECT_EQ(nullptr, palette_manager_get_filename(nullptr));
    EXPECT_FALSE(palette_set_color(nullptr, 0, test_color));

    RGBA result = palette_get_color(nullptr, 0);
    EXPECT_EQ(0, result.r);
    EXPECT_EQ(0, result.g);
    EXPECT_EQ(0, result.b);
    EXPECT_EQ(255, result.a);  // Should return black
}

// ===== Performance Tests =====

TEST_F(PaletteManagerTest, ColorAccessPerformance) {
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 10000; ++i) {
        RGBA color = palette_get_color(&palette_manager, i % PALETTE_COLOR_COUNT);
        (void)color;  // Suppress unused variable warning
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // 10000 color accesses should be very fast
    EXPECT_LT(duration.count(), 10) << "10000 color accesses took " << duration.count() << "ms";
}

TEST_F(PaletteManagerTest, ColorModificationPerformance) {
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 1000; ++i) {
        RGBA color = palette_make_color(i % 256, (i * 2) % 256, (i * 3) % 256, 255);
        palette_set_color(&palette_manager, i % PALETTE_COLOR_COUNT, color);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // 1000 color modifications should be reasonably fast
    EXPECT_LT(duration.count(), 50) << "1000 color modifications took " << duration.count() << "ms";
}
