#include "enemy.h"
#include <random>

int RandomNum(int size){
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> uni(0,size);

    return uni(rng);
}

Enemy::Enemy(int grid_size, int startX, int startY, Direction direction, int speed, std::shared_ptr<GameMap> map_ptr,
             std::shared_ptr<AICentral> ai) :
        Character(grid_size, startX, startY, direction, speed, map_ptr), _ai(ai) {}

void Enemy::Move() {
    if (!(_pos_y % _grid_size) && !(_pos_x % _grid_size)) {
        if(!_moving){
            // check options
            std::vector<Enemy::Direction> options;
            std::vector<Enemy::Direction> nonVisited;
            int row = _pos_y / _grid_size;
            int col = _pos_x / _grid_size;
            bool b = _map_ptr->AreaIsAvailable(row, col-1);
            if (b){
                options.push_back(Enemy::Direction::kLeft);
            }
            if (ReadMap(row, col - 1) == AICentral::MapObject::kDark){
                DrawMap(row, col - 1, b  ? AICentral::MapObject::kRoad : AICentral::MapObject::kWall);
                if(b){
                    nonVisited.push_back(Enemy::Direction::kLeft);
                }
            }
            b = _map_ptr->AreaIsAvailable(row, col + 1);
            if (b){
                options.push_back(Enemy::Direction::kRight);
            }
            if (ReadMap(row, col + 1) == AICentral::MapObject::kDark){
                DrawMap(row, col + 1, b  ? AICentral::MapObject::kRoad : AICentral::MapObject::kWall);
                if(b){
                    nonVisited.push_back(Enemy::Direction::kRight);
                }
            }
            b = _map_ptr->AreaIsAvailable(row -1, col);
            if (b){
                options.push_back(Enemy::Direction::kUp);
            }
            if (ReadMap(row - 1, col) == AICentral::MapObject::kDark){
                DrawMap(row - 1, col, b  ? AICentral::MapObject::kRoad : AICentral::MapObject::kWall);
                if(b){
                    nonVisited.push_back(Enemy::Direction::kUp);
                }
            }
            b = _map_ptr->AreaIsAvailable(row + 1, col);
            if (b){
                options.push_back(Enemy::Direction::kDown);
            }
            if (ReadMap(row + 1, col) == AICentral::MapObject::kDark){
                DrawMap(row + 1, col, b  ? AICentral::MapObject::kRoad : AICentral::MapObject::kWall);
                if(b){
                    nonVisited.push_back(Enemy::Direction::kDown);
                }
            }

            Enemy::Direction temp = _direction;

            if (nonVisited.empty()){
                if (!options.empty())
                    _direction = options[RandomNum(options.size())];
            } else {
                _direction = nonVisited[RandomNum(nonVisited.size())];
            }

            if (temp != _direction){
                return;
            }

            _moving = true;
        }
    }
    _moving = false;
    switch (_direction) {
        case Direction::kUp:
            if (_pos_y % _grid_size) {
                _pos_y -= _speed;
            } else {
                if (_map_ptr->AreaIsAvailable((_pos_y - _speed) / _grid_size, _pos_x / _grid_size)) {
                    _pos_y -= _speed;
                } else {
                    _direction = Direction::kLeft;
                }
            }
            break;

        case Direction::kDown:
            if (_pos_y % _grid_size) {
                _pos_y += _speed;
            } else {
                if (_map_ptr->AreaIsAvailable(((_pos_y + _speed) / _grid_size) + 1, _pos_x / _grid_size)) {
                    _pos_y += _speed;
                } else {
                    _direction = Direction::kRight;
                }
            }
            break;

        case Direction::kLeft:
            if (_pos_x % _grid_size) {
                _pos_x -= _speed;
            } else {
                if (_map_ptr->AreaIsAvailable(_pos_y / _grid_size, (_pos_x - _speed) / _grid_size)) {
                    _pos_x -= _speed;
                } else {
                    _direction = Direction::kDown;
                }
            }
            break;

        case Direction::kRight:
            if (_pos_x % _grid_size) {
                _pos_x += _speed;
            } else {
                if (_map_ptr->AreaIsAvailable(_pos_y / _grid_size, ((_pos_x + _speed) / _grid_size) + 1)) {
                    _pos_x += _speed;
                } else {
                    _direction = Direction::kUp;
                }
            }
            break;
    }
}

void Enemy::DrawMap(int row, int col, AICentral::MapObject value) {
    _ai->AddToMap(row, col, value);
};

AICentral::MapObject Enemy::ReadMap(int row, int col) {
    return _ai->ReadFromMap(row, col);
}