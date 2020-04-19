#include "gamemap.h"

bool GameMap::AreaIsAvailable(int row, int col) const {
    return _map[row][col] == 0;
}

GameMap::GameMap(int grid_height, int grid_width, int grid_size, std::vector<std::vector<int>> map_data) :
        _height(grid_height), _width(grid_width), _size(grid_size), _map(map_data) {}