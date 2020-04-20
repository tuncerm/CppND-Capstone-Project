//Copied From CppND-Capstone-Snake-Game
#ifndef CHARACTER_H
#define CHARACTER_H

#include <vector>
#include <memory>
#include "gamemap.h"

class Character {
public:
    enum class Direction {
        kUp, kDown, kLeft, kRight
    };

    Character(int grid_size, int startX, int startY, Direction direction, int speed, std::shared_ptr<GameMap> map_ptr)
            : _grid_size(grid_size),
              _direction(direction),
              _speed(speed),
              _pos_x(startX),
              _pos_y(startY),
              _map_ptr(map_ptr) {}

    void Update();

    int GetX() const { return _pos_x; }

    int GetY() const { return _pos_y; }

    void SetDirection(Direction direction) { _direction = direction; }

    Direction GetDirection() const { return _direction; }

    bool IsAlive() const { return _alive; }

    void IsAlive(bool alive) { alive = alive; }

private:
    int _grid_size;
    int _pos_x;
    int _pos_y;
    Direction _direction;
    bool _alive{true};
    int _speed;
    std::shared_ptr<GameMap> _map_ptr;
};

#endif