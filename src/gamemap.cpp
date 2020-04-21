#include "gamemap.h"
#include <fstream>
#include <string>
#include <sstream>
#include "tempmap.cpp"

bool GameMap::AreaIsAvailable(int row, int col) const {
    return _map[row][col] == 0;
}

GameMap::GameMap(int grid_height, int grid_width, int grid_size) :
        _height(grid_height), _width(grid_width), _size(grid_size) {
    std::ifstream filestream("game.map");
    if (filestream.is_open()) {
        std::string line;
        while (std::getline(filestream, line)) {

            std::istringstream ls(line);
            std::vector<int> v;
            int x;
            while (ls >> x) {
                v.push_back(x);
            }
            _map.emplace_back(v);
        }
    } else {
        _map = std::move(tempgamemap);
    }
}