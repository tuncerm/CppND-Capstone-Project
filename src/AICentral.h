#ifndef AICENTRAL_H
#define AICENTRAL_H

#include <vector>

class AICentral {
   public:
    enum class MapObject { kRoad, kWall, kDark };

    AICentral(int rows, int cols);

    AICentral(AICentral& source) = delete;

    AICentral(AICentral&& source) = delete;

    AICentral& operator=(AICentral& source) = delete;

    AICentral& operator=(AICentral&& source) = delete;

    ~AICentral() = default;

    // FOR FUTURE IMPLEMENTATIONS.
    // TODO : Find a way to calculate heuristic value from any point to any point...
    // TODO : Implement Last known position of player.
    // TODO : Implement Ask For direction

    void AddToMap(int row, int col, MapObject ob);

    MapObject ReadFromMap(int row, int col) const;

   private:
    bool IsInBounds(int row, int col) const;

    std::vector<std::vector<MapObject>> _map;
};

#endif
