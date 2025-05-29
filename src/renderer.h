#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#include <memory>
#include <SDL3/SDL.h>

#include "player.h"
#include "enemy.h"
#include "gamemap.h"

class Renderer {
public:
    enum class ObjectType {
        kPlayer,
        kEnemy,
        kProjectile
    };

    Renderer(int grid_size,
             int grid_width,
             int grid_height,
             std::shared_ptr<GameMap> map_ptr);

    ~Renderer();

    void RenderObject(ObjectType ot, Character::Direction d, int posX, int posY);

    void Render(Player &player, const Enemy enemy);

    void UpdateWindowTitle(int score, int fps);

private:
    SDL_Window *sdl_window = nullptr;
    SDL_Renderer *sdl_renderer = nullptr;

    std::shared_ptr<GameMap> _map_ptr;
    const int _grid_size;
    const int _screen_width;
    const int _screen_height;
    const int _grid_width;
    const int _grid_height;
};

#endif
