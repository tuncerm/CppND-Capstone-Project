#include "character.h"
#include <iostream>

Character::Character(int grid_size, int startX, int startY, Direction direction, int speed,
                     std::shared_ptr<GameMap> map_ptr)
        : _grid_size(grid_size), _direction(direction), _speed(speed), _pos_x(startX), _pos_y(startY),
          _map_ptr(map_ptr) {}