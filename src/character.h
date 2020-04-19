//Copied From CppND-Capstone-Snake-Game
#ifndef CHARACTER_H
#define CHARACTER_H

#include <vector>

class Character {
public:
    enum class Direction {
        kUp, kDown, kLeft, kRight
    };

    Character(int startX, int startY, Direction direction, int speed)
            : _direction(direction),
              _speed(speed),
              _pos_x(startX),
              _pos_y(startY) {}

    void Update();

    int GetX() const { return _pos_x; }

    int GetY() const { return _pos_y; }

    void SetDirection(Direction direction) { _direction = direction; }

    Direction GetDirection() { return _direction; }

    bool IsAlive() const { return _alive; }

    void IsAlive(bool alive) { alive = alive; }

private:
    Direction _direction;
    int _pos_x;
    int _pos_y;
    bool _alive{true};
    int _speed;
};

#endif