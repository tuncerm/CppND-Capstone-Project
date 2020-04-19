//Copied From CppND-Capstone-Snake-Game
#ifndef PLAYER_H
#define PLAYER_H

#include <vector>
#include "character.h"
#include "gamemap.h"

class Player : public Character {
public:
    Player(int grid_size, int startX, int startY, Direction direction, int speed, std::shared_ptr<GameMap> map_ptr) :
        Character(grid_size, startX, startY, direction, speed, map_ptr) {}
};

#endif
