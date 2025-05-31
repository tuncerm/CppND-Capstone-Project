// Copied From CppND-Capstone-Snake-Game
#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <vector>
#include "character.h"
#include "gamemap.h"

class Projectile : public Character {
   public:
    Projectile(int grid_size, int startX, int startY, Direction direction, int speed,
               std::shared_ptr<GameMap> map_ptr)
        : Character(grid_size, startX, startY, direction, speed, map_ptr) {}

    void Move() override {}
};

#endif
