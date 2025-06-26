#include "config.h"
#include <stdio.h>
#include <string.h>
#include "config_manager.h"
#include "error_handler.h"

// Forward declaration of helper function
static void register_config_entries(ConfigManager* cm);
static void set_default_config(AppConfig* config);
static void populate_config_from_manager(AppConfig* config, const ConfigManager* cm);

bool load_app_config(AppConfig* config, const char* config_path) {
    if (!config || !config_path) {
        ErrorHandler_Set(ERR_INVALID_ARGUMENT, __FILE__, __LINE__,
                         "NULL pointer passed to load_app_config");
        return false;
    }

    ConfigManager cm;
    if (!config_manager_init(&cm, "PaletteMaker")) {
        ErrorHandler_Log();
        fprintf(stderr, "Failed to initialize config manager. Using default configuration.\n");
        set_default_config(config);
        return true;
    }

    register_config_entries(&cm);

    // Attempt to load from file, but proceed with defaults if it fails
    if (!config_manager_load(&cm, config_path)) {
        fprintf(stderr, "Warning: Could not load '%s'. Using default configuration.\n",
                config_path);
        ErrorHandler_Log();  // Log the specific error (e.g., file not found)
        set_default_config(config);
    } else {
        printf("Successfully loaded configuration from '%s'.\n", config_path);
        populate_config_from_manager(config, &cm);
    }

    // config_print_summary(&cm); // Uncomment for debugging

    return true;
}

static void set_default_config(AppConfig* config) {
    // [display]
    config->window_width = 800;
    config->window_height = 600;
    strcpy(config->window_title, "Palette Maker v1.0.0 - SDL3 Edition");

    // [ui]
    config->swatch_size = 45;
    config->swatch_border = 2;
    config->grid_cols = 4;
    config->grid_rows = 4;
    config->grid_start_x = 20;
    config->grid_start_y = 20;
    config->ui_panel_x = 220;
    config->ui_panel_y = 20;
    config->ui_panel_width = 320;
    config->ui_panel_height = 300;
    config->ui_panel_row_height = 30;
    config->button_width = 30;
    config->button_height = 20;
    config->value_display_width = 45;
    config->value_display_height = 20;
    config->action_button_width = 80;
    config->action_button_height = 25;

    // [colors]
    config->background_color = (ConfigColorRGBA){240, 240, 240, 255};
    config->border_color = (ConfigColorRGBA){128, 128, 128, 255};
    config->text_color = (ConfigColorRGBA){0, 0, 0, 255};
    config->button_color = (ConfigColorRGBA){224, 224, 224, 255};
    config->button_hover_color = (ConfigColorRGBA){208, 208, 208, 255};
    config->selected_color = (ConfigColorRGBA){0, 128, 255, 255};

    // [performance]
    config->target_fps = 60;
    config->frame_delay_ms = 16;

    // [palette]
    config->color_count = 16;
    strcpy(config->default_file, "palette.dat");
}

static void register_config_entries(ConfigManager* cm) {
    // Section "display"
    config_register_entry(cm, "display", "window_width", CONFIG_TYPE_INT, config_make_int(800),
                          false);
    config_register_entry(cm, "display", "window_height", CONFIG_TYPE_INT, config_make_int(600),
                          false);
    config_register_entry(cm, "display", "window_title", CONFIG_TYPE_STRING,
                          config_make_string("Palette Maker v1.0.0 - SDL3 Edition"), false);

    // Section "ui"
    config_register_entry(cm, "ui", "swatch_size", CONFIG_TYPE_INT, config_make_int(45), false);
    config_register_entry(cm, "ui", "swatch_border", CONFIG_TYPE_INT, config_make_int(2), false);
    config_register_entry(cm, "ui", "grid_cols", CONFIG_TYPE_INT, config_make_int(4), false);
    config_register_entry(cm, "ui", "grid_rows", CONFIG_TYPE_INT, config_make_int(4), false);
    config_register_entry(cm, "ui", "grid_start_x", CONFIG_TYPE_INT, config_make_int(20), false);
    config_register_entry(cm, "ui", "grid_start_y", CONFIG_TYPE_INT, config_make_int(20), false);
    config_register_entry(cm, "ui", "ui_panel_x", CONFIG_TYPE_INT, config_make_int(220), false);
    config_register_entry(cm, "ui", "ui_panel_y", CONFIG_TYPE_INT, config_make_int(20), false);
    config_register_entry(cm, "ui", "ui_panel_width", CONFIG_TYPE_INT, config_make_int(320), false);
    config_register_entry(cm, "ui", "ui_panel_height", CONFIG_TYPE_INT, config_make_int(300),
                          false);
    config_register_entry(cm, "ui", "ui_panel_row_height", CONFIG_TYPE_INT, config_make_int(30),
                          false);
    config_register_entry(cm, "ui", "button_width", CONFIG_TYPE_INT, config_make_int(30), false);
    config_register_entry(cm, "ui", "button_height", CONFIG_TYPE_INT, config_make_int(20), false);
    config_register_entry(cm, "ui", "value_display_width", CONFIG_TYPE_INT, config_make_int(45),
                          false);
    config_register_entry(cm, "ui", "value_display_height", CONFIG_TYPE_INT, config_make_int(20),
                          false);
    config_register_entry(cm, "ui", "action_button_width", CONFIG_TYPE_INT, config_make_int(80),
                          false);
    config_register_entry(cm, "ui", "action_button_height", CONFIG_TYPE_INT, config_make_int(25),
                          false);

    // Section "colors"
    config_register_entry(cm, "colors", "background_color", CONFIG_TYPE_COLOR_RGBA,
                          config_make_rgba(240, 240, 240, 255), false);
    config_register_entry(cm, "colors", "border_color", CONFIG_TYPE_COLOR_RGBA,
                          config_make_rgba(128, 128, 128, 255), false);
    config_register_entry(cm, "colors", "text_color", CONFIG_TYPE_COLOR_RGBA,
                          config_make_rgba(0, 0, 0, 255), false);
    config_register_entry(cm, "colors", "button_color", CONFIG_TYPE_COLOR_RGBA,
                          config_make_rgba(224, 224, 224, 255), false);
    config_register_entry(cm, "colors", "button_hover_color", CONFIG_TYPE_COLOR_RGBA,
                          config_make_rgba(208, 208, 208, 255), false);
    config_register_entry(cm, "colors", "selected_color", CONFIG_TYPE_COLOR_RGBA,
                          config_make_rgba(0, 128, 255, 255), false);

    // Section "performance"
    config_register_entry(cm, "performance", "target_fps", CONFIG_TYPE_INT, config_make_int(60),
                          false);
    config_register_entry(cm, "performance", "frame_delay_ms", CONFIG_TYPE_INT, config_make_int(16),
                          false);

    // Section "palette"
    config_register_entry(cm, "palette", "color_count", CONFIG_TYPE_INT, config_make_int(16),
                          false);
    config_register_entry(cm, "palette", "default_file", CONFIG_TYPE_STRING,
                          config_make_string("palette.dat"), false);
}

static void populate_config_from_manager(AppConfig* config, const ConfigManager* cm) {
    // [display]
    config->window_width = config_get_int(cm, "display", "window_width", 800);
    config->window_height = config_get_int(cm, "display", "window_height", 600);
    strncpy(config->window_title,
            config_get_string(cm, "display", "window_title", "Palette Maker v1.0.0 - SDL3 Edition"),
            CONFIG_MAX_STRING_LENGTH - 1);

    // [ui]
    config->swatch_size = config_get_int(cm, "ui", "swatch_size", 45);
    config->swatch_border = config_get_int(cm, "ui", "swatch_border", 2);
    config->grid_cols = config_get_int(cm, "ui", "grid_cols", 4);
    config->grid_rows = config_get_int(cm, "ui", "grid_rows", 4);
    config->grid_start_x = config_get_int(cm, "ui", "grid_start_x", 20);
    config->grid_start_y = config_get_int(cm, "ui", "grid_start_y", 20);
    config->ui_panel_x = config_get_int(cm, "ui", "ui_panel_x", 220);
    config->ui_panel_y = config_get_int(cm, "ui", "ui_panel_y", 20);
    config->ui_panel_width = config_get_int(cm, "ui", "ui_panel_width", 320);
    config->ui_panel_height = config_get_int(cm, "ui", "ui_panel_height", 300);
    config->ui_panel_row_height = config_get_int(cm, "ui", "ui_panel_row_height", 30);
    config->button_width = config_get_int(cm, "ui", "button_width", 30);
    config->button_height = config_get_int(cm, "ui", "button_height", 20);
    config->value_display_width = config_get_int(cm, "ui", "value_display_width", 45);
    config->value_display_height = config_get_int(cm, "ui", "value_display_height", 20);
    config->action_button_width = config_get_int(cm, "ui", "action_button_width", 80);
    config->action_button_height = config_get_int(cm, "ui", "action_button_height", 25);

    // [colors]
    config->background_color =
        config_get_rgba(cm, "colors", "background_color", (ConfigColorRGBA){240, 240, 240, 255});
    config->border_color =
        config_get_rgba(cm, "colors", "border_color", (ConfigColorRGBA){128, 128, 128, 255});
    config->text_color =
        config_get_rgba(cm, "colors", "text_color", (ConfigColorRGBA){0, 0, 0, 255});
    config->button_color =
        config_get_rgba(cm, "colors", "button_color", (ConfigColorRGBA){224, 224, 224, 255});
    config->button_hover_color =
        config_get_rgba(cm, "colors", "button_hover_color", (ConfigColorRGBA){208, 208, 208, 255});
    config->selected_color =
        config_get_rgba(cm, "colors", "selected_color", (ConfigColorRGBA){0, 128, 255, 255});

    // [performance]
    config->target_fps = config_get_int(cm, "performance", "target_fps", 60);
    config->frame_delay_ms = config_get_int(cm, "performance", "frame_delay_ms", 16);

    // [palette]
    config->color_count = config_get_int(cm, "palette", "color_count", 16);
    strncpy(config->default_file, config_get_string(cm, "palette", "default_file", "palette.dat"),
            CONFIG_MAX_PATH_LENGTH - 1);
}
