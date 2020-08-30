//Copied From CppND-Capstone-Snake-Game
#ifndef CHARACTER_H
#define CHARACTER_H

#include <vector>
#include <memory>
#include "gamemap.h"

class Character {
public:
    enum class Direction {
        kUp, kDown, kLeft, kRight, kNone
    };

    Character(int grid_size, int startX, int startY, Direction direction, int speed, std::shared_ptr<GameMap> map_ptr);

    virtual void Move() {};

    int GetX() const { return _pos_x; }

    int GetY() const { return _pos_y; }

    void SetDirection(Direction direction) { _direction = direction; }

    Direction GetDirection() const { return _direction; }

    bool IsAlive() const { return _alive; }

    void IsAlive(bool alive) { alive = alive; }

    bool IsMoving() const { return _moving; }

    void IsMoving(bool moving) { _moving = moving; }

protected:
    int _grid_size;
    int _pos_x;
    int _pos_y;
    Direction _direction;
    bool _alive{true};
    bool _moving{false};
    int _speed;
    std::shared_ptr<GameMap> _map_ptr;
};

#endif