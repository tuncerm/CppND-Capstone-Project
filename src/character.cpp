//Copied From CppND-Capstone-Snake-Game
#include "character.h"

void Character::Update() {
    switch (_direction) {
        case Direction::kUp:
            if (_map_ptr->AreaIsAvailable(_pos_x, _pos_y - _speed, 0, 0))
                _pos_y -= _speed;
            break;

        case Direction::kDown:
            if (_map_ptr->AreaIsAvailable( _pos_x, _pos_y + _speed, 0, 1))
                _pos_y += _speed;
            break;

        case Direction::kLeft:
            if (_map_ptr->AreaIsAvailable(_pos_x - _speed, _pos_y, 0, 0))
                _pos_x -= _speed;
            break;

        case Direction::kRight:
            if (_map_ptr->AreaIsAvailable(_pos_x + _speed, _pos_y, 1, 0))
            _pos_x += _speed;
            break;
    }
}
