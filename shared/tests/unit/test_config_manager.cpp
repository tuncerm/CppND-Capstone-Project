#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include "config/config_manager.h"
#include "error_handler/error_handler.h"

class ConfigManagerTest : public ::testing::Test {
   protected:
    void SetUp() override {
        config_manager_init(&config, "Test Application");

        // Create a test configuration file
        std::ofstream test_file("test_config.json");
        test_file << R"({
            "_meta": {
                "application": "Test Application",
                "version": "1.0"
            },
            "display": {
                "width": 1024,
                "height": 768,
                "fullscreen": false,
                "title": "Test Window"
            },
            "colors": {
                "background": "#FF0000FF",
                "foreground": "#00FF00"
            },
            "performance": {
                "fps": 60,
                "vsync": true,
                "quality": 0.8
            }
        })";
        test_file.close();
    }

    void TearDown() override {
        // Clean up test file
        std::remove("test_config.json");
    }

    ConfigManager config;
};

TEST_F(ConfigManagerTest, InitializationTest) {
    EXPECT_STREQ(config.application_name, "Test Application");
    EXPECT_EQ(config.entry_count, 0);
    EXPECT_FALSE(config.is_loaded);
    EXPECT_FALSE(ErrorHandler_HasError());
}

TEST_F(ConfigManagerTest, RegisterEntryTest) {
    // Register various types of entries
    EXPECT_TRUE(config_register_entry(&config, "display", "width", CONFIG_TYPE_INT,
                                      config_make_int(800), true));
    EXPECT_TRUE(config_register_entry(&config, "display", "height", CONFIG_TYPE_INT,
                                      config_make_int(600), true));
    EXPECT_TRUE(config_register_entry(&config, "display", "fullscreen", CONFIG_TYPE_BOOL,
                                      config_make_bool(false), false));
    EXPECT_TRUE(config_register_entry(&config, "display", "title", CONFIG_TYPE_STRING,
                                      config_make_string("Default Title"), false));
    EXPECT_TRUE(config_register_entry(&config, "colors", "background", CONFIG_TYPE_COLOR_RGBA,
                                      config_make_rgba(255, 255, 255, 255), false));

    EXPECT_EQ(config.entry_count, 5);
}

TEST_F(ConfigManagerTest, LoadConfigurationTest) {
    // Register entries first
    config_register_entry(&config, "display", "width", CONFIG_TYPE_INT, config_make_int(800), true);
    config_register_entry(&config, "display", "height", CONFIG_TYPE_INT, config_make_int(600),
                          true);
    config_register_entry(&config, "display", "fullscreen", CONFIG_TYPE_BOOL,
                          config_make_bool(false), false);
    config_register_entry(&config, "display", "title", CONFIG_TYPE_STRING,
                          config_make_string("Default"), false);
    config_register_entry(&config, "colors", "background", CONFIG_TYPE_COLOR_RGBA,
                          config_make_rgba(255, 255, 255, 255), false);
    config_register_entry(&config, "performance", "fps", CONFIG_TYPE_INT, config_make_int(30),
                          false);
    config_register_entry(&config, "performance", "vsync", CONFIG_TYPE_BOOL,
                          config_make_bool(false), false);
    config_register_entry(&config, "performance", "quality", CONFIG_TYPE_FLOAT,
                          config_make_float(0.5), false);

    // Load configuration
    EXPECT_TRUE(config_manager_load(&config, "test_config.json"));
    EXPECT_TRUE(config.is_loaded);
    EXPECT_FALSE(ErrorHandler_HasError());
}

TEST_F(ConfigManagerTest, GetConfigurationValuesTest) {
    // Register and load configuration
    config_register_entry(&config, "display", "width", CONFIG_TYPE_INT, config_make_int(800), true);
    config_register_entry(&config, "display", "height", CONFIG_TYPE_INT, config_make_int(600),
                          true);
    config_register_entry(&config, "display", "fullscreen", CONFIG_TYPE_BOOL,
                          config_make_bool(false), false);
    config_register_entry(&config, "display", "title", CONFIG_TYPE_STRING,
                          config_make_string("Default"), false);
    config_register_entry(&config, "colors", "background", CONFIG_TYPE_COLOR_RGBA,
                          config_make_rgba(255, 255, 255, 255), false);
    config_register_entry(&config, "performance", "fps", CONFIG_TYPE_INT, config_make_int(30),
                          false);
    config_register_entry(&config, "performance", "quality", CONFIG_TYPE_FLOAT,
                          config_make_float(0.5), false);

    config_manager_load(&config, "test_config.json");

    // Test getting values
    EXPECT_EQ(config_get_int(&config, "display", "width", 800), 1024);
    EXPECT_EQ(config_get_int(&config, "display", "height", 600), 768);
    EXPECT_FALSE(config_get_bool(&config, "display", "fullscreen", true));
    EXPECT_STREQ(config_get_string(&config, "display", "title", "Default"), "Test Window");
    EXPECT_EQ(config_get_int(&config, "performance", "fps", 30), 60);
    EXPECT_FLOAT_EQ(config_get_float(&config, "performance", "quality", 0.5), 0.8f);

    // Test color parsing
    ConfigColorRGBA bg_color =
        config_get_rgba(&config, "colors", "background", {255, 255, 255, 255});
    EXPECT_EQ(bg_color.r, 255);
    EXPECT_EQ(bg_color.g, 0);
    EXPECT_EQ(bg_color.b, 0);
    EXPECT_EQ(bg_color.a, 255);
}

TEST_F(ConfigManagerTest, DefaultValuesTest) {
    // Register entries but don't load config
    config_register_entry(&config, "display", "width", CONFIG_TYPE_INT, config_make_int(800),
                          false);
    config_register_entry(&config, "display", "height", CONFIG_TYPE_INT, config_make_int(600),
                          false);

    // Should return default values
    EXPECT_EQ(config_get_int(&config, "display", "width", 999), 800);
    EXPECT_EQ(config_get_int(&config, "display", "height", 999), 600);

    // Unknown entries should return fallback defaults
    EXPECT_EQ(config_get_int(&config, "unknown", "value", 123), 123);
}

TEST_F(ConfigManagerTest, ValidationTest) {
    // Register required entry
    config_register_entry(&config, "display", "width", CONFIG_TYPE_INT, config_make_int(800), true);
    config_register_entry(&config, "display", "height", CONFIG_TYPE_INT, config_make_int(600),
                          true);

    // Load valid configuration
    EXPECT_TRUE(config_manager_load(&config, "test_config.json"));
    EXPECT_TRUE(config_manager_validate(&config));
    EXPECT_FALSE(ErrorHandler_HasError());
}

TEST_F(ConfigManagerTest, ErrorHandlingTest) {
    // Try to load non-existent file
    EXPECT_FALSE(config_manager_load(&config, "nonexistent.json"));
    EXPECT_TRUE(ErrorHandler_HasError());
    EXPECT_TRUE(strlen(ErrorHandler_Get()->message) > 0);
    ErrorHandler_Clear();
}

TEST_F(ConfigManagerTest, UtilityFunctionsTest) {
    // Test config value creation functions
    ConfigValue int_val = config_make_int(42);
    EXPECT_EQ(int_val.int_val, 42);

    ConfigValue float_val = config_make_float(3.14f);
    EXPECT_FLOAT_EQ(float_val.float_val, 3.14f);

    ConfigValue bool_val = config_make_bool(true);
    EXPECT_TRUE(bool_val.bool_val);

    ConfigValue string_val = config_make_string("test");
    EXPECT_STREQ(string_val.string_val, "test");

    ConfigValue rgba_val = config_make_rgba(255, 128, 64, 192);
    EXPECT_EQ(rgba_val.rgba_val.r, 255);
    EXPECT_EQ(rgba_val.rgba_val.g, 128);
    EXPECT_EQ(rgba_val.rgba_val.b, 64);
    EXPECT_EQ(rgba_val.rgba_val.a, 192);
}

TEST_F(ConfigManagerTest, HasEntryTest) {
    config_register_entry(&config, "display", "width", CONFIG_TYPE_INT, config_make_int(800),
                          false);

    EXPECT_TRUE(config_has_entry(&config, "display", "width"));
    EXPECT_FALSE(config_has_entry(&config, "display", "height"));
    EXPECT_FALSE(config_has_entry(&config, "unknown", "value"));
}

// Test configuration save functionality
TEST_F(ConfigManagerTest, SaveConfigurationTest) {
    // Register and set some values
    config_register_entry(&config, "display", "width", CONFIG_TYPE_INT, config_make_int(800),
                          false);
    config_register_entry(&config, "display", "height", CONFIG_TYPE_INT, config_make_int(600),
                          false);
    config_register_entry(&config, "display", "title", CONFIG_TYPE_STRING,
                          config_make_string("Test"), false);

    // Save configuration
    EXPECT_TRUE(config_manager_save(&config, "test_save.json"));

    // Verify file was created and has content
    std::ifstream saved_file("test_save.json");
    EXPECT_TRUE(saved_file.good());

    std::string content((std::istreambuf_iterator<char>(saved_file)),
                        std::istreambuf_iterator<char>());
    EXPECT_TRUE(content.find("Test Application") != std::string::npos);
    EXPECT_TRUE(content.find("\"width\": 800") != std::string::npos);

    saved_file.close();
    std::remove("test_save.json");
}
