//Copied From CppND-Capstone-Snake-Game
#ifndef ENEMY_H
#define ENEMY_H

#include <vector>
#include "character.h"
#include "gamemap.h"

class Enemy : public Character {
public:
    Enemy(int grid_size, int startX, int startY, Direction direction, int speed, std::shared_ptr<GameMap> map_ptr);
    
    void Move() override;

private:
    bool mapping {false};
    int targetRow, targetCol;
    void DrawMap(int row, int col, int value);
    int ReadMap(int row, int col);
    int area_map[20][32];
};
#endif
