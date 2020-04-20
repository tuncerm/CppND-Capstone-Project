#include "enemy.h"

Enemy::Enemy(int grid_size, int startX, int startY, Direction direction, int speed, std::shared_ptr<GameMap> map_ptr) :
        Character(grid_size, startX, startY, direction, speed, map_ptr) {}