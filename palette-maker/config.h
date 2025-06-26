#ifndef PALETTE_MAKER_CONFIG_H
#define PALETTE_MAKER_CONFIG_H

#include <stdbool.h>
#include "config/config_manager.h"

/**
 * @brief Holds all configuration for the Palette Maker application.
 */
typedef struct {
    // Display settings
    int window_width;
    int window_height;
    char window_title[CONFIG_MAX_STRING_LENGTH];

    // UI layout and dimensions
    int swatch_size;
    int swatch_border;
    int grid_cols;
    int grid_rows;
    int grid_start_x;
    int grid_start_y;
    int ui_panel_x;
    int ui_panel_y;
    int ui_panel_width;
    int ui_panel_height;
    int ui_panel_row_height;
    int button_width;
    int button_height;
    int value_display_width;
    int value_display_height;
    int action_button_width;
    int action_button_height;

    // Colors
    ConfigColorRGBA background_color;
    ConfigColorRGBA border_color;
    ConfigColorRGBA text_color;
    ConfigColorRGBA button_color;
    ConfigColorRGBA button_hover_color;
    ConfigColorRGBA selected_color;

    // Performance
    int target_fps;
    int frame_delay_ms;

    // Palette settings
    int color_count;
    char default_file[CONFIG_MAX_PATH_LENGTH];

} AppConfig;

/**
 * @brief Loads the application configuration from the specified file.
 *
 * If the file cannot be loaded, it populates the config struct with
 * sensible default values.
 *
 * @param config Pointer to the AppConfig struct to populate.
 * @param config_path Path to the JSON configuration file.
 * @return true if the configuration was loaded successfully (even if from defaults),
 *         false on critical error.
 */
bool load_app_config(AppConfig* config, const char* config_path);

#endif  // PALETTE_MAKER_CONFIG_H
