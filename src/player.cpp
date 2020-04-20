#include "character.h"
#include "player.h"
#include <iostream>

Player::Player(int grid_size, int startX, int startY, Direction direction, int speed, std::shared_ptr<GameMap> map_ptr) :
        Character(grid_size, startX, startY, direction, speed, map_ptr) {}


void Player::Move() {
    switch (_direction) {
        case Direction::kUp:
            if (_pos_x % _grid_size){
                if (_map_ptr->AreaIsAvailable( (_pos_y - _speed)/_grid_size, _pos_x/_grid_size)
                && _map_ptr->AreaIsAvailable( (_pos_y - _speed)/_grid_size, (_pos_x/_grid_size)+1)){
                    _pos_y -= _speed;
                }
            } else {
                if (_map_ptr->AreaIsAvailable( (_pos_y - _speed)/_grid_size, _pos_x/_grid_size)){
                    _pos_y -= _speed;
                }
            }
            break;

        case Direction::kDown:
            if (_pos_y % _grid_size){
                _pos_y += _speed;
            } else {
                if (_pos_x % _grid_size){
                    if (_map_ptr->AreaIsAvailable( ((_pos_y + _speed)/_grid_size)+1, _pos_x/_grid_size)
                        && _map_ptr->AreaIsAvailable( ((_pos_y + _speed)/_grid_size)+1, (_pos_x/_grid_size)+1)){
                        _pos_y += _speed;
                    }
                } else {
                    if (_map_ptr->AreaIsAvailable( ((_pos_y + _speed)/_grid_size)+1, _pos_x/_grid_size)){
                        _pos_y += _speed;
                    }
                }
            }
            break;

        case Direction::kLeft:
            if (_pos_y % _grid_size){
                if (_map_ptr->AreaIsAvailable( _pos_y/_grid_size, (_pos_x - _speed)/_grid_size)
                    && _map_ptr->AreaIsAvailable( (_pos_y/_grid_size)+1, (_pos_x - _speed)/_grid_size)){
                    _pos_x -= _speed;
                }
            } else {
                if (_map_ptr->AreaIsAvailable( _pos_y/_grid_size, (_pos_x - _speed)/_grid_size)){
                    _pos_x -= _speed;
                }
            }
            break;

        case Direction::kRight:
            if (_pos_x % _grid_size){
                _pos_x += _speed;
            } else {
                if (_pos_y % _grid_size){
                    if (_map_ptr->AreaIsAvailable( _pos_y/_grid_size, ((_pos_x + _speed)/_grid_size)+1)
                        && _map_ptr->AreaIsAvailable( (_pos_y/_grid_size)+1, ((_pos_x + _speed)/_grid_size)+1)){
                        _pos_x += _speed;
                    }
                } else {
                    if (_map_ptr->AreaIsAvailable( _pos_y/_grid_size, ((_pos_x + _speed)/_grid_size)+1)){
                        _pos_x += _speed;
                    }
                }
            }
            break;
    }
}