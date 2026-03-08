#include "player.h"
#include <iostream>
#include "character.h"

Player::Player(int grid_size, int startX, int startY, Direction direction, int speed,
               std::shared_ptr<GameMap> map_ptr)
    : Character(grid_size, startX, startY, direction, speed, map_ptr) {}

void Player::Move() {
    switch (_direction) {
        case Direction::kUp:
            if (_map_ptr->AreaIsAvailable((_pos_y - _speed) / _grid_size, _pos_x / _grid_size)) {
                _pos_y -= _speed;
            }
            break;

        case Direction::kDown:
            if (_pos_y % _grid_size) {
                _pos_y += _speed;
            } else {
                if (_map_ptr->AreaIsAvailable(((_pos_y + _speed) / _grid_size) + 1,
                                              _pos_x / _grid_size)) {
                    _pos_y += _speed;
                }
            }
            break;

        case Direction::kLeft:
            if (_map_ptr->AreaIsAvailable(_pos_y / _grid_size, (_pos_x - _speed) / _grid_size)) {
                _pos_x -= _speed;
            }
            break;

        case Direction::kRight:
            if (_pos_x % _grid_size) {
                _pos_x += _speed;
            } else {
                if (_map_ptr->AreaIsAvailable(_pos_y / _grid_size,
                                              ((_pos_x + _speed) / _grid_size) + 1)) {
                    _pos_x += _speed;
                }
            }
            break;
        case Direction::kNone:
            break;
    }
}

int Player::GetGridSize() {
    return _grid_size;
}

bool Player::DamageFrontSubtile() {
    if (!_map_ptr) {
        return false;
    }

    int target_x = _pos_x + (_grid_size / 2);
    int target_y = _pos_y + (_grid_size / 2);

    switch (_direction) {
        case Direction::kUp:
            target_y = _pos_y - 1;
            break;
        case Direction::kDown:
            target_y = _pos_y + _grid_size;
            break;
        case Direction::kLeft:
            target_x = _pos_x - 1;
            break;
        case Direction::kRight:
            target_x = _pos_x + _grid_size;
            break;
        case Direction::kNone:
            return false;
    }

    return _map_ptr->DamageAtWorldPosition(target_x, target_y);
}
