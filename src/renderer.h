#ifndef RENDERER_H
#define RENDERER_H

#include <memory>
#include <vector>
#include "../shared/config/config_manager.h"
#include "SDL3/SDL.h"
#include "character.h"
#include "gamemap.h"
class Player;
class Enemy;

struct RenderObject {
    SDL_FRect rect;
    ConfigColorRGBA color;

    bool operator<(const RenderObject& other) const {
        if (color.r != other.color.r)
            return color.r < other.color.r;
        if (color.g != other.color.g)
            return color.g < other.color.g;
        if (color.b != other.color.b)
            return color.b < other.color.b;
        return color.a < other.color.a;
    }

    bool operator==(const RenderObject& other) const {
        return color.r == other.color.r && color.g == other.color.g && color.b == other.color.b &&
               color.a == other.color.a;
    }
};

class Renderer {
   public:
    enum class ObjectType { kPlayer, kEnemy };
    Renderer(const int grid_size, const int grid_width, const int grid_height,
             std::shared_ptr<GameMap> map_ptr);
    ~Renderer();

    void Render(Player& player, Enemy const enemy);
    void UpdateWindowTitle(int score, int fps);

   private:
    void AddCharacterObjects(std::vector<RenderObject>& objects, ObjectType ot,
                             Character::Direction d, int posX, int posY);

    SDL_Window* sdl_window;
    SDL_Renderer* sdl_renderer;
    ConfigManager _config;

    std::shared_ptr<GameMap> _map_ptr;
    const int _screen_width;
    const int _screen_height;
    const int _grid_width;
    const int _grid_height;
    const int _grid_size;
};

#endif
