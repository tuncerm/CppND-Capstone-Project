//Copied From CppND-Capstone-Snake-Game
#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#include "SDL.h"
#include "player.h"

class Renderer {
public:
    Renderer(const std::size_t grid_size, const std::size_t grid_width, const std::size_t grid_height,
             std::vector<std::vector<int>> &map_vector);

    ~Renderer();

    void Render(Player const player);

    void UpdateWindowTitle(int score, int fps);

private:
    SDL_Window *sdl_window;
    SDL_Renderer *sdl_renderer;

    std::vector<std::vector<int>> &game_map;
    const std::size_t grid_size;
    const std::size_t screen_width;
    const std::size_t screen_height;
    const std::size_t grid_width;
    const std::size_t grid_height;
};

#endif