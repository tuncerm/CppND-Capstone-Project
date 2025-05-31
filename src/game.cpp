// Copied From CppND-Capstone-Snake-Game
#include "game.h"
#include <SDL3/SDL.h>
#include <iostream>

#include <future>
#include <memory>
#include <thread>

namespace {
uint64_t SDL_GetTicksMS() {
    return SDL_GetTicksNS() / 1000000;
}
}  // namespace

Game::Game(int grid_size, int grid_width, int grid_height, std::shared_ptr<GameMap> map_ptr,
           std::shared_ptr<AICentral> aiCentral)
    : player(grid_size, grid_size * (grid_width / 2), grid_size * (grid_height - 2),
             Character::Direction::kUp, 4, map_ptr),
      enemy(grid_size, grid_size * (grid_width / 2), grid_size * (2), Character::Direction::kDown,
            2, map_ptr, aiCentral),
      _aiCentral(aiCentral),
      _map_ptr(map_ptr),
      _grid_size(grid_size) {}

void Game::Run(Controller const& controller, Renderer& renderer,
               std::size_t target_frame_duration) {
    uint64_t title_timestamp = SDL_GetTicksMS();
    uint64_t frame_start;
    uint64_t frame_end;
    uint64_t frame_duration;
    int frame_count = 0;
    bool running = true;

    while (running) {
        frame_start = SDL_GetTicksMS();

        // Input, Update, Render - the main game loop.
        controller.HandleInput(running, player);

        // Enemy movement.
        std::future<void> em = std::async(&Enemy::Move, &enemy);
        em.get();
        renderer.Render(player, enemy);

        frame_end = SDL_GetTicksMS();

        frame_count++;
        frame_duration = frame_end - frame_start;

        if (frame_end - title_timestamp >= 1000) {
            renderer.UpdateWindowTitle(score, frame_count);
            frame_count = 0;
            title_timestamp = frame_end;
        }

        if (frame_duration < target_frame_duration) {
            SDL_DelayNS((target_frame_duration - frame_duration) * 1000000);
        }
    }
}

int Game::GetScore() const {
    return score;
}
