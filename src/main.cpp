//Copied From CppND-Capstone-Snake-Game
#include <iostream>
#include "controller.h"
#include "game.h"
#include "renderer.h"
#include "tempmap.cpp"

int main() {
    constexpr std::size_t kGridSize{32};
    constexpr std::size_t kGridWidth{32};
    constexpr std::size_t kGridHeight{20};
    constexpr std::size_t kFramesPerSecond{25};
    constexpr std::size_t kMsPerFrame{1000 / kFramesPerSecond};
    constexpr std::size_t kScreenWidth{kGridSize * kGridWidth};
    constexpr std::size_t kScreenHeight{kGridSize * kGridHeight};

    Renderer renderer(kScreenWidth, kScreenHeight, kGridWidth, kGridHeight, gamemap);
    Controller controller;
    Game game(kGridWidth, kGridHeight, gamemap);
    game.Run(controller, renderer, kMsPerFrame);
    std::cout << "Game has terminated successfully!\n";
    std::cout << "Score: " << game.GetScore() << "\n";
    std::cout << "Size: " << game.GetSize() << "\n";
    return 0;
}