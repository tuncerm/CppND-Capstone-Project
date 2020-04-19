//Copied From CppND-Capstone-Snake-Game
#include <iostream>
#include "controller.h"
#include "game.h"
#include "renderer.h"
#include "tempmap.cpp"
#include "gamemap.h"

int main() {
    constexpr int kGridSize{32};
    constexpr int kGridWidth{32};
    constexpr int kGridHeight{20};
    constexpr int kFramesPerSecond{60};
    constexpr int kMsPerFrame{1000 / kFramesPerSecond};

    std::shared_ptr<GameMap> map_ptr = std::make_shared<GameMap>(kGridHeight, kGridWidth, kGridSize, std::move(gamemap));
    Renderer renderer(kGridSize, kGridWidth, kGridHeight, map_ptr);
    Controller controller;
    Game game(kGridSize, kGridWidth, kGridHeight, map_ptr);
    game.Run(controller, renderer, kMsPerFrame);
    std::cout << "Game has terminated successfully!\n";
    std::cout << "Score: " << game.GetScore() << "\n";
    return 0;
}