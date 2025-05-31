// Copied From CppND-Capstone-Snake-Game
#include <iostream>
#include <memory>
#include "AICentral.h"
#include "controller.h"
#include "game.h"
#include "gamemap.h"
#include "renderer.h"

int main() {
    constexpr int kGridSize{32};
    constexpr int kGridWidth{32};
    constexpr int kGridHeight{20};
    constexpr int kFramesPerSecond{60};
    constexpr int kMsPerFrame{1000 / kFramesPerSecond};
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