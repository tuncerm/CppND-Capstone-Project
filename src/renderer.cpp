//Copied From CppND-Capstone-Snake-Game
#include "renderer.h"
#include <iostream>
#include <string>

Renderer::Renderer(const int grid_size,
                   const int grid_width,
                   const int grid_height,
                   std::shared_ptr<GameMap> map_ptr)
        : _screen_width(grid_size * grid_width),
          _screen_height(grid_size * grid_height),
          _grid_width(grid_width),
          _grid_height(grid_height),
          _map_ptr(map_ptr),
          _grid_size(grid_size) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize.\n";
        std::cerr << "SDL_Error: " << SDL_GetError() << "\n";
    }

    // Create Window
    sdl_window = SDL_CreateWindow("Character Game", SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED, _screen_width,
                                  _screen_height, SDL_WINDOW_SHOWN);

    if (nullptr == sdl_window) {
        std::cerr << "Window could not be created.\n";
        std::cerr << " SDL_Error: " << SDL_GetError() << "\n";
    }

    // Create renderer
    sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED);
    if (nullptr == sdl_renderer) {
        std::cerr << "Renderer could not be created.\n";
        std::cerr << "SDL_Error: " << SDL_GetError() << "\n";
    }
}

Renderer::~Renderer() {
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
}

void Renderer::Render(Player const player) {
    SDL_Rect block;
    block.w = _grid_size;
    block.h = _grid_size;

    // Render Map   //32*20
    for (int row = 0; row < _map_ptr->RowCount(); ++row) {
        for (int col = 0; col < _map_ptr->ColCount(); ++col) {
            block.x = col * block.w;
            block.y = row * block.h;
            if (_map_ptr->GetElement(row, col) == 1) {
                SDL_SetRenderDrawColor(sdl_renderer, 0xFF, 0x00, 0x00, 0xFF);
            } else {
                SDL_SetRenderDrawColor(sdl_renderer, 0x00, 0x00, 0xFF, 0xFF);
            }
            SDL_RenderFillRect(sdl_renderer, &block);
        }
    }

    // Render Enemies
//     SDL_SetRenderDrawColor(sdl_renderer, 0xFF, 0xCC, 0x00, 0xFF);
//     block.x = food.x * block.w;
//     block.y = food.y * block.h;
//     SDL_RenderFillRect(sdl_renderer, &block);

    // Render Player
    SDL_SetRenderDrawColor(sdl_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    block.x = player.GetX();
    block.y = player.GetY();
    SDL_RenderFillRect(sdl_renderer, &block);

    // Render Projectiles
//     block.x = static_cast<int>(player.head_x) * block.w;
//     block.y = static_cast<int>(player.head_y) * block.h;
//     if (player.alive) {
//         SDL_SetRenderDrawColor(sdl_renderer, 0x00, 0x7A, 0xCC, 0xFF);
//     } else {
//         SDL_SetRenderDrawColor(sdl_renderer, 0xFF, 0x00, 0x00, 0xFF);
//     }
//     SDL_RenderFillRect(sdl_renderer, &block);

    // Update Screen
    SDL_RenderPresent(sdl_renderer);
}

void Renderer::UpdateWindowTitle(int score, int fps) {
    std::string title{"PlayGame Score: " + std::to_string(score) + " FPS: " + std::to_string(fps)};
    SDL_SetWindowTitle(sdl_window, title.c_str());
}
