#ifndef GAMEMAP_H
#define GAMEMAP_H
#include <vector>

class GameMap {
public:
    GameMap(int grid_height, int grid_width, int grid_size, std::vector<std::vector<int>> map_data);
    bool AreaIsAvailable(int x, int y) const;
    int RowCount() { return _height;}
    int ColCount() { return _width; }
    int GetElement(int row, int col){ return _map[row][col]; }
private:
    int _height;
    int _width;
    int _size;
    std::vector<std::vector<int>> _map;
};


#endif //GAMEMAP_H
