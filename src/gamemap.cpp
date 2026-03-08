#include "gamemap.h"
#include <fstream>
#include <sstream>
#include <string>
#include "tempmap.cpp"

int GameMap::RowCount() const {
    return static_cast<int>(_map.size());
}

int GameMap::ColCount() const {
    if (_map.empty()) {
        return 0;
    }

    return static_cast<int>(_map.front().size());
}

bool GameMap::IsInBounds(int row, int col) const {
    if (row < 0 || col < 0) {
        return false;
    }
    if (row >= static_cast<int>(_map.size())) {
        return false;
    }
    if (col >= static_cast<int>(_map[row].size())) {
        return false;
    }

    return true;
}

bool GameMap::AreaIsAvailable(int row, int col) const {
    if (_map.empty()) {
        return false;
    }
    if (!IsInBounds(row, col)) {
        return false;
    }

    return _map[row][col] == 0;
}

int GameMap::GetElement(int row, int col) const {
    if (_map.empty() || !IsInBounds(row, col)) {
        // Treat out-of-bounds as blocked to prevent undefined access.
        return 1;
    }

    return _map[row][col];
}

bool GameMap::MatchesDimensions(int expected_rows, int expected_cols) const {
    if (expected_rows <= 0 || expected_cols <= 0) {
        return false;
    }
    if (RowCount() != expected_rows) {
        return false;
    }

    for (const auto& row : _map) {
        if (static_cast<int>(row.size()) != expected_cols) {
            return false;
        }
    }

    return true;
}

GameMap::GameMap(int grid_height, int grid_width, int grid_size, const std::string& map_path)
    : _height(grid_height), _width(grid_width), _size(grid_size) {
    std::ifstream filestream(map_path);
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
