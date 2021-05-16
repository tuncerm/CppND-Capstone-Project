#ifndef AICENTRAL_H
#define AICENTRAL_H

class AICentral {
public:
    enum class MapObject {
        kRoad,
        kWall,
        kDark
    };

    AICentral();

    AICentral(AICentral &source) = delete;

    AICentral(AICentral &&source) = delete;

    AICentral &operator=(AICentral &source) = delete;

    AICentral &operator=(AICentral &&source) = delete;

    ~AICentral() = default;

    // FOR FUTURE IMPLEMENTATIONS.
    // TODO : Find a way to calculate heuristic value from any point to any point...
    // TODO : Implement Last known position of player.
    // TODO : Implement Ask For direction

    void AddToMap(int row, int col, MapObject ob);

    MapObject ReadFromMap(int row, int col);

private:
    MapObject _map[20][32];
};


#endif
