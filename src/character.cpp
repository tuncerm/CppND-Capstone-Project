//Copied From CppND-Capstone-Snake-Game
#include "character.h"

void Character::Update() {
    switch (_direction) {
        case Direction::kUp:
            _pos_y -= _speed;
            break;

        case Direction::kDown:
            _pos_y += _speed;
            break;

        case Direction::kLeft:
            _pos_x -= _speed;
            break;

        case Direction::kRight:
            _pos_x += _speed;
            break;
    }
}
