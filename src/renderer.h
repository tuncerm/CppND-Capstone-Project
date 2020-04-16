//Copied From CppND-Capstone-Snake-Game
#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#include "SDL.h"
#include "character.h"

class Renderer {
public:
    Renderer(const std::size_t screen_width, const std::size_t screen_height,
             const std::size_t grid_width, const std::size_t grid_height, const int grid_map[]);

    ~Renderer();

    void Render(Character const character, SDL_Point const &food);

    void UpdateWindowTitle(int score, int fps);

private:
    SDL_Window *sdl_window;
    SDL_Renderer *sdl_renderer;

    const int *game_map;
    const std::size_t screen_width;
    const std::size_t screen_height;
    const std::size_t grid_width;
    const std::size_t grid_height;
};

#endif