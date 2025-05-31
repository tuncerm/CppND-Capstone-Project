// Copied From CppND-Capstone-Snake-Game
#ifndef GAME_H
#define GAME_H

#include <SDL3/SDL.h>
#include <memory>
#include <random>

#include "AICentral.h"
#include "character.h"
#include "controller.h"
#include "enemy.h"
#include "projectile.h"
#include "renderer.h"

class Game {
   public:
    Game(int grid_size, int grid_width, int grid_height, std::shared_ptr<GameMap> map_ptr,
         std::shared_ptr<AICentral> aiCentral);

    void Run(Controller const& controller, Renderer& renderer, std::size_t target_frame_duration);

    int GetScore() const;

   private:
    Player player;
    Enemy enemy;
    std::shared_ptr<AICentral> _aiCentral;
    // std::vector<Enemy> enemies;
    // std::vector<Projectile> projectiles;

    std::shared_ptr<GameMap> _map_ptr;
    int _grid_size;
    int score{0};
};

#endif