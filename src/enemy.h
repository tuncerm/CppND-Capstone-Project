// Copied From CppND-Capstone-Snake-Game
#ifndef ENEMY_H
#define ENEMY_H

#include <memory>
#include <vector>
#include "AICentral.h"
#include "character.h"
#include "gamemap.h"

class Enemy : public Character {
   public:
    Enemy(int grid_size, int startX, int startY, Direction direction, int speed,
          std::shared_ptr<GameMap> map_ptr, std::shared_ptr<AICentral> ai);

    void Move() override;

   private:
    bool _moving{false};
    std::shared_ptr<AICentral> _ai;
    bool mapping{true};
    void DrawMap(int row, int col, AICentral::MapObject value);
    AICentral::MapObject ReadMap(int row, int col);
};

#endif
