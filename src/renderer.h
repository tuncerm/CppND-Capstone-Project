//Copied From CppND-Capstone-Snake-Game
#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#include "SDL.h"
#include "player.h"
#include "gamemap.h"

class Renderer {
public:
    Renderer(const int grid_size,
            const int grid_width,
            const int grid_height,
            std::shared_ptr<GameMap> map_ptr);

    ~Renderer();

    void Render(Player const player);

    void UpdateWindowTitle(int score, int fps);

private:
    SDL_Window *sdl_window;
    SDL_Renderer *sdl_renderer;

    std::shared_ptr<GameMap> _map_ptr;
    const int _grid_size;
    const int _screen_width;
    const int _screen_height;
    const int _grid_width;
    const int _grid_height;
};

#endif