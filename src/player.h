#ifndef PLAYER_H
#define PLAYER_H

#include <vector>
#include "character.h"
#include "gamemap.h"

class Player : public Character {
   public:
    Player(int grid_size, int startX, int startY, Direction direction, int speed,
           std::shared_ptr<GameMap> map_ptr);

    void Move() override;

    int GetGridSize();
};

#endif
