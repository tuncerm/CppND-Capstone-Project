// Copied From CppND-Capstone-Snake-Game
#include <iostream>
#include <memory>
#include "../shared/config/config_manager.h"
#include "../shared/error_handler/error_handler.h"
#include "AICentral.h"
#include "constants.h"
#include "controller.h"
#include "game.h"
#include "gamemap.h"
#include "path_resolver.h"
#include "renderer.h"

int main() {
    // Initialize configuration system
    ConfigManager config;
    if (!config_manager_init(&config, "Character Game")) {
        ErrorHandler_Log();
        return 1;
    }

    // Register configuration entries with defaults
    config_register_entry(&config, "display", "grid_size", CONFIG_TYPE_INT,
                          config_make_int(GRID_SIZE), true);
    config_register_entry(&config, "display", "grid_width", CONFIG_TYPE_INT,
                          config_make_int(GRID_WIDTH), true);
    config_register_entry(&config, "display", "grid_height", CONFIG_TYPE_INT,
                          config_make_int(GRID_HEIGHT), true);
    config_register_entry(&config, "performance", "target_fps", CONFIG_TYPE_INT,
                          config_make_int(TARGET_FPS), true);
    config_register_entry(&config, "performance", "ms_per_frame", CONFIG_TYPE_INT,
                          config_make_int(MS_PER_FRAME), true);
    config_register_entry(&config, "files", "map_file", CONFIG_TYPE_STRING,
                          config_make_string("game.map"), false);

    // Load configuration file
    const std::string config_path = resolve_game_config_path();
    if (!config_manager_load(&config, config_path.c_str())) {
        std::cerr << "Warning: Failed to load configuration file, using defaults\n";
        ErrorHandler_Log();
        ErrorHandler_Clear();  // Clear error to allow execution with defaults
    }

    // Get configuration values
    const auto positive_or_default = [](int value, int fallback, const char* label) {
        if (value > 0) {
            return value;
        }

        std::cerr << "Warning: Invalid config value for " << label << " (" << value
                  << "), using default " << fallback << ".\n";
        return fallback;
    };

    const int kGridSize =
        positive_or_default(config_get_int(&config, "display", "grid_size", GRID_SIZE), GRID_SIZE,
                            "display.grid_size");
    const int kGridWidth = positive_or_default(
        config_get_int(&config, "display", "grid_width", GRID_WIDTH), GRID_WIDTH,
        "display.grid_width");
    const int kGridHeight = positive_or_default(
        config_get_int(&config, "display", "grid_height", GRID_HEIGHT), GRID_HEIGHT,
        "display.grid_height");
    const int kFramesPerSecond = positive_or_default(
        config_get_int(&config, "performance", "target_fps", TARGET_FPS), TARGET_FPS,
        "performance.target_fps");
    const int kMsPerFrame = positive_or_default(
        config_get_int(&config, "performance", "ms_per_frame", MS_PER_FRAME), MS_PER_FRAME,
        "performance.ms_per_frame");
    const char* configured_map_file = config_get_string(&config, "files", "map_file", "game.map");
    const std::string map_path = resolve_game_map_path(configured_map_file);

    std::cout << "Starting Character Game with configuration:\n";
    std::cout << "  Config path: " << config_path << "\n";
    std::cout << "  Grid: " << kGridWidth << "x" << kGridHeight << " (size: " << kGridSize << ")\n";
    std::cout << "  Target FPS: " << kFramesPerSecond << "\n";
    std::cout << "  Map path: " << map_path << "\n";

    std::shared_ptr<GameMap> map_ptr =
        std::make_shared<GameMap>(kGridHeight, kGridWidth, kGridSize, map_path);
    if (!map_ptr->MatchesDimensions(kGridHeight, kGridWidth)) {
        std::cerr << "Error: Map dimensions do not match configured grid dimensions.\n";
        std::cerr << "  Expected rows x cols: " << kGridHeight << "x" << kGridWidth << "\n";
        std::cerr << "  Loaded rows x cols:   " << map_ptr->RowCount() << "x"
                  << map_ptr->ColCount() << "\n";
        std::cerr << "Fix game.map or adjust config/game_config.json display.grid_width/"
                     "display.grid_height.\n";
        return 1;
    }
    std::shared_ptr<AICentral> aiCentral =
        std::make_shared<AICentral>(map_ptr->RowCount(), map_ptr->ColCount());

    // Initialize SDL context
    SDLContext context;
    if (!sdl_init_context_simple(&context, "Character Game", kGridWidth * kGridSize,
                                 kGridHeight * kGridSize)) {
        ErrorHandler_Log();
        return 1;
    }

    Renderer renderer(kGridSize, kGridWidth, kGridHeight, map_ptr, &context, config_path);
    Controller controller;
    Game game(kGridSize, kGridWidth, kGridHeight, map_ptr, aiCentral);
    game.Run(controller, renderer, kMsPerFrame);

    // Cleanup
    sdl_cleanup_context(&context);
    std::cout << "Game has terminated successfully!\n";
    std::cout << "Score: " << game.GetScore() << "\n";
    return 0;
}
