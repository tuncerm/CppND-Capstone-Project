#include "AICentral.h"

AICentral::AICentral(int rows, int cols) {
    if (rows <= 0 || cols <= 0) {
        return;
    }

    _map.assign(rows, std::vector<MapObject>(cols, MapObject::kDark));
}

bool AICentral::IsInBounds(int row, int col) const {
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

void AICentral::AddToMap(int row, int col, MapObject ob) {
    if (_map.empty() || !IsInBounds(row, col)) {
        return;
    }

    _map[row][col] = ob;
}

AICentral::MapObject AICentral::ReadFromMap(int row, int col) const {
    if (_map.empty() || !IsInBounds(row, col)) {
        // Treat out-of-bounds as walls to keep enemy pathing bounded.
        return MapObject::kWall;
    }

    return _map[row][col];
}
