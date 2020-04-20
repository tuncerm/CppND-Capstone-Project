#include "enemy.h"

Enemy::Enemy(int grid_size, int startX, int startY, Direction direction, int speed, std::shared_ptr<GameMap> map_ptr) :
        Character(grid_size, startX, startY, direction, speed, map_ptr) {}


void Enemy::Move() {
    if(!(_pos_y % _grid_size)){

    }
    switch (_direction) {
        case Direction::kUp:
            if(_pos_y % _grid_size){
                _pos_y -= _speed;
            } else {
                if (_map_ptr->AreaIsAvailable( (_pos_y - _speed)/_grid_size, _pos_x/_grid_size)){
                    _pos_y -= _speed;
                } else {
                    _direction = Direction::kLeft;
                }
            }
            break;

        case Direction::kDown:
            if (_pos_y % _grid_size){
                _pos_y += _speed;
            } else {
                if (_map_ptr->AreaIsAvailable( ((_pos_y + _speed)/_grid_size)+1, _pos_x/_grid_size)){
                    _pos_y += _speed;
                } else {
                    _direction = Direction::kRight;
                }
            }
            break;

        case Direction::kLeft:
            if(_pos_x % _grid_size){
                _pos_x -= _speed;
            } else {
                if (_map_ptr->AreaIsAvailable( _pos_y/_grid_size, (_pos_x - _speed)/_grid_size)){
                    _pos_x -= _speed;
                } else {
                    _direction = Direction::kDown;
                }
            }
            break;

        case Direction::kRight:
            if (_pos_x % _grid_size){
                _pos_x += _speed;
            } else {
                if (_map_ptr->AreaIsAvailable( _pos_y/_grid_size, ((_pos_x + _speed)/_grid_size)+1)){
                    _pos_x += _speed;
                } else {
                    _direction = Direction::kUp;
                }
            }
            break;
    }    
}

void Enemy::DrawMap(int row, int col, int value){
    area_map[row][col] = value;    
};

int Enemy::ReadMap(int row, int col){
    return area_map[row][col];
}