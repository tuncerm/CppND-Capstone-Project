//Copied From CppND-Capstone-Snake-Game
#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#include <memory>
#include "SDL.h"
#include "player.h"
#include "enemy.h"
#include "gamemap.h"

class Renderer {
public:
    enum class ObjectType{
        kPlayer,
        kEnemy,
        kProjectile
    };
    Renderer(const int grid_size,
            const int grid_width,
            const int grid_height,
            std::shared_ptr<GameMap> map_ptr);

    ~Renderer();

    void RenderObject(ObjectType ot, Character::Direction d, int posX, int posY);

    void Render(Player const player, Enemy const enemy);

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