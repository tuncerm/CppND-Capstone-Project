//Copied From CppND-Capstone-Snake-Game
#ifndef GAME_H
#define GAME_H

#include <random>
#include <memory>
#include "SDL.h"
#include "controller.h"
#include "renderer.h"
#include "character.h"
#include "enemy.h"
#include "projectile.h"

class Game {
public:
    Game(int grid_size, int grid_width, int grid_height, std::shared_ptr<GameMap> map_ptr);

    void Run(Controller const &controller, Renderer &renderer, std::size_t target_frame_duration);

    int GetScore() const;

private:
    Player player;
    Enemy enemy;
    // std::vector<Enemy> enemies;
    // std::vector<Projectile> projectiles;

    std::shared_ptr<GameMap> _map_ptr;
    int _grid_size;
    int score{0};
};

#endif