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

    // Load configuration file
    if (!config_manager_load(&config, "config/game_config.json")) {
        std::cerr << "Warning: Failed to load configuration file, using defaults\n";
        ErrorHandler_Log();
        ErrorHandler_Clear();  // Clear error to allow execution with defaults
    }

    // Get configuration values
    const int kGridSize = config_get_int(&config, "display", "grid_size", GRID_SIZE);
    const int kGridWidth = config_get_int(&config, "display", "grid_width", GRID_WIDTH);
    const int kGridHeight = config_get_int(&config, "display", "grid_height", GRID_HEIGHT);
    const int kFramesPerSecond = config_get_int(&config, "performance", "target_fps", TARGET_FPS);
    const int kMsPerFrame = config_get_int(&config, "performance", "ms_per_frame", MS_PER_FRAME);

    std::cout << "Starting Character Game with configuration:\n";
    std::cout << "  Grid: " << kGridWidth << "x" << kGridHeight << " (size: " << kGridSize << ")\n";
    std::cout << "  Target FPS: " << kFramesPerSecond << "\n";

    std::shared_ptr<AICentral> aiCentral = std::make_shared<AICentral>();
    std::shared_ptr<GameMap> map_ptr =
        std::make_shared<GameMap>(kGridHeight, kGridWidth, kGridSize);
    Renderer renderer(kGridSize, kGridWidth, kGridHeight, map_ptr);
    Controller controller;
    Game game(kGridSize, kGridWidth, kGridHeight, map_ptr, aiCentral);
    game.Run(controller, renderer, kMsPerFrame);
    std::cout << "Game has terminated successfully!\n";
    std::cout << "Score: " << game.GetScore() << "\n";
    return 0;
}
