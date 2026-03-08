#ifndef GAMEMAP_H
#define GAMEMAP_H

#include <array>
#include <cstdint>
#include <string>
#include <vector>

class GameMap {
   public:
    static constexpr int kSubtilesPerAxis = 4;
    static constexpr int kSubtilesPerCell = kSubtilesPerAxis * kSubtilesPerAxis;
    static constexpr std::uint8_t kMovementNoPass = 0;
    static constexpr std::uint8_t kMovementPass = 1;
    static constexpr std::uint8_t kMovementRequire = 2;
    static constexpr std::uint8_t kMovementSpecial = 3;
    static constexpr std::uint8_t kDestructionIndestructible = 0;
    static constexpr std::uint8_t kDestructionNormal = 1;
    static constexpr std::uint8_t kDestructionHeavy = 2;
    static constexpr std::uint8_t kDestructionSpecial = 3;

    struct MapCell {
        int material{0};  // 0 = floor, 1 = blocked
        bool has_subtiles{false};
        std::array<std::uint16_t, kSubtilesPerCell> subtiles{};
    };

    GameMap(int grid_height, int grid_width, int grid_size, const std::string& map_path);

    bool AreaIsAvailable(int row, int col) const;

    int RowCount() const;

    int ColCount() const;

    int GetElement(int row, int col) const;

    bool IsInBounds(int row, int col) const;

    bool MatchesDimensions(int expected_rows, int expected_cols) const;

    int GetCellMaterial(int row, int col) const;

    bool HasDestructibleSubtiles(int row, int col) const;

    std::uint8_t GetSubtileId(int row, int col, int subtile_index) const;

    std::uint8_t GetSubtileHealth(int row, int col, int subtile_index) const;

    std::uint8_t GetSubtileDestructionMode(int row, int col, int subtile_index) const;

    std::uint8_t GetSubtileMovement(int row, int col, int subtile_index) const;

    bool IsSubtileDestroyed(int row, int col, int subtile_index) const;

    bool DamageSubtile(int row, int col, int subtile_index);

    bool WorldToSubtile(int world_x, int world_y, int* out_row, int* out_col,
                        int* out_subtile_index) const;

    bool DamageAtWorldPosition(int world_x, int world_y);

   private:
    int _height;
    int _width;
    int _size;
    std::vector<std::vector<MapCell>> _cells;
};

#endif  // GAMEMAP_H
