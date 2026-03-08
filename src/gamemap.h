#ifndef GAMEMAP_H
#define GAMEMAP_H

#include <string>
#include <vector>

class GameMap {
   public:
    GameMap(int grid_height, int grid_width, int grid_size, const std::string& map_path);

    bool AreaIsAvailable(int row, int col) const;

    int RowCount() const;

    int ColCount() const;

    int GetElement(int row, int col) const;

    bool IsInBounds(int row, int col) const;

    bool MatchesDimensions(int expected_rows, int expected_cols) const;

   private:
    int _height;
    int _width;
    int _size;
    std::vector<std::vector<int>> _map;
};

#endif  // GAMEMAP_H
