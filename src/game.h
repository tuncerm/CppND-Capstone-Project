//Copied From CppND-Capstone-Snake-Game
#ifndef GAME_H
#define GAME_H

#include <random>
#include "SDL.h"
#include "controller.h"
#include "renderer.h"
#include "character.h"

class Game {
public:
    Game(int grid_size, int grid_width, int grid_height, std::vector<std::vector<int>> &map_vector);

    void Run(Controller const &controller, Renderer &renderer, std::size_t target_frame_duration);

    int GetScore() const;


private:
    Player player;
    // std::vector<Enemy> enemies;
    // std::vector<Projectile> projectiles;

    std::vector<std::vector<int>> &game_map;

    int score{0};

    void Update();
};

#endif