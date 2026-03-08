#include "gamemap.h"
#include <cstdlib>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "tempmap.h"

namespace {
constexpr int kSubtilesPerCell = GameMap::kSubtilesPerCell;
constexpr int kSubtilesPerAxis = GameMap::kSubtilesPerAxis;
constexpr std::uint8_t kHealthMask = 0x07u;
constexpr int kDestructionShift = 3;
constexpr std::uint8_t kDestructionMask = 0x07u;
constexpr int kMovementShift = 6;
constexpr std::uint8_t kMovementMask = 0x03u;

bool parse_int(const std::string& token, int* out_value) {
    if (!out_value || token.empty()) {
        return false;
    }

    char* end_ptr = nullptr;
    const long parsed = std::strtol(token.c_str(), &end_ptr, 10);
    if (end_ptr == token.c_str() || *end_ptr != '\0') {
        return false;
    }

    *out_value = static_cast<int>(parsed);
    return true;
}

bool parse_uint16_auto(const std::string& token, std::uint16_t* out_value) {
    if (!out_value || token.empty()) {
        return false;
    }

    char* end_ptr = nullptr;
    const unsigned long parsed = std::strtoul(token.c_str(), &end_ptr, 0);
    if (end_ptr == token.c_str() || *end_ptr != '\0') {
        return false;
    }
    if (parsed > 65535ul) {
        return false;
    }

    *out_value = static_cast<std::uint16_t>(parsed);
    return true;
}

std::vector<std::string> split(const std::string& input, char delimiter) {
    std::vector<std::string> parts;
    std::stringstream ss(input);
    std::string item;
    while (std::getline(ss, item, delimiter)) {
        parts.push_back(item);
    }
    return parts;
}

std::uint8_t subtile_spec(std::uint16_t entry) {
    return static_cast<std::uint8_t>((entry >> 8) & 0xFFu);
}

std::uint8_t subtile_tile_id(std::uint16_t entry) {
    return static_cast<std::uint8_t>(entry & 0xFFu);
}

std::uint8_t subtile_health_from_spec(std::uint8_t spec) {
    return static_cast<std::uint8_t>(spec & kHealthMask);
}

std::uint8_t subtile_destruction_mode_from_spec(std::uint8_t spec) {
    return static_cast<std::uint8_t>((spec >> kDestructionShift) & kDestructionMask);
}

std::uint8_t subtile_movement_from_spec(std::uint8_t spec) {
    return static_cast<std::uint8_t>((spec >> kMovementShift) & kMovementMask);
}

std::uint16_t make_subtile_entry(std::uint8_t tile_id, std::uint8_t health,
                                 std::uint8_t destruction_mode, std::uint8_t movement) {
    const std::uint8_t spec =
        static_cast<std::uint8_t>((health & kHealthMask) |
                                  ((destruction_mode & kDestructionMask) << kDestructionShift) |
                                  ((movement & kMovementMask) << kMovementShift));
    return static_cast<std::uint16_t>(tile_id | (static_cast<std::uint16_t>(spec) << 8));
}

std::uint16_t set_subtile_health(std::uint16_t entry, std::uint8_t health) {
    const std::uint8_t tile_id = subtile_tile_id(entry);
    const std::uint8_t spec = subtile_spec(entry);
    const std::uint8_t updated_spec =
        static_cast<std::uint8_t>((spec & static_cast<std::uint8_t>(~kHealthMask)) |
                                  (health & kHealthMask));
    return static_cast<std::uint16_t>(tile_id | (static_cast<std::uint16_t>(updated_spec) << 8));
}

bool parse_csv_uint8_16(const std::string& csv, std::array<std::uint8_t, kSubtilesPerCell>* out_ids) {
    if (!out_ids) {
        return false;
    }

    std::stringstream subtile_stream(csv);
    std::string subtile_token;
    int subtile_index = 0;
    while (std::getline(subtile_stream, subtile_token, ',')) {
        if (subtile_index >= kSubtilesPerCell) {
            return false;
        }

        int subtile_id = 0;
        if (!parse_int(subtile_token, &subtile_id)) {
            return false;
        }
        if (subtile_id < 0 || subtile_id > 255) {
            return false;
        }

        (*out_ids)[subtile_index++] = static_cast<std::uint8_t>(subtile_id);
    }

    return subtile_index == kSubtilesPerCell;
}

bool parse_csv_uint16_16(const std::string& csv,
                         std::array<std::uint16_t, kSubtilesPerCell>* out_entries) {
    if (!out_entries) {
        return false;
    }

    std::stringstream entry_stream(csv);
    std::string entry_token;
    int entry_index = 0;
    while (std::getline(entry_stream, entry_token, ',')) {
        if (entry_index >= kSubtilesPerCell) {
            return false;
        }

        std::uint16_t packed_entry = 0;
        if (!parse_uint16_auto(entry_token, &packed_entry)) {
            return false;
        }

        (*out_entries)[entry_index++] = packed_entry;
    }

    return entry_index == kSubtilesPerCell;
}

bool subtile_is_walkable(std::uint16_t entry) {
    const std::uint8_t spec = subtile_spec(entry);
    const std::uint8_t health = subtile_health_from_spec(spec);
    if (health == 0) {
        return true;
    }

    const std::uint8_t movement = subtile_movement_from_spec(spec);
    return movement == GameMap::kMovementPass;
}

bool can_apply_normal_damage(std::uint8_t destruction_mode) {
    return destruction_mode == GameMap::kDestructionNormal;
}

GameMap::MapCell make_legacy_cell(int value) {
    GameMap::MapCell cell;
    cell.material = (value == 0) ? 0 : 1;
    return cell;
}

void ensure_default_destructible_subtiles(GameMap::MapCell* cell) {
    if (!cell || cell->has_subtiles || cell->material == 0) {
        return;
    }

    const std::uint16_t default_entry = make_subtile_entry(
        0, 1, GameMap::kDestructionNormal, GameMap::kMovementNoPass);
    cell->subtiles.fill(default_entry);
    cell->has_subtiles = true;
}

bool parse_legacy_extended_cell_token(const std::string& token, GameMap::MapCell* out_cell) {
    if (!out_cell) {
        return false;
    }

    // Legacy extended token format:
    //   material|t0,t1,...,t15|destroyed_mask
    const std::vector<std::string> parts = split(token, '|');
    if (parts.size() != 3) {
        return false;
    }
    int material = 0;
    if (!parse_int(parts[0], &material)) {
        return false;
    }

    std::array<std::uint8_t, kSubtilesPerCell> tile_ids{};
    if (!parse_csv_uint8_16(parts[1], &tile_ids)) {
        return false;
    }

    int mask_value = 0;
    if (!parse_int(parts[2], &mask_value)) {
        return false;
    }
    if (mask_value < 0 || mask_value > 65535) {
        return false;
    }

    out_cell->material = (material == 0) ? 0 : 1;
    out_cell->has_subtiles = true;
    for (int i = 0; i < kSubtilesPerCell; ++i) {
        const bool destroyed = (mask_value & (1 << i)) != 0;
        const std::uint8_t health = destroyed ? 0 : 1;
        out_cell->subtiles[i] = make_subtile_entry(tile_ids[i], health, GameMap::kDestructionNormal,
                                                   GameMap::kMovementNoPass);
    }
    return true;
}

bool parse_packed_cell_token(const std::string& token, GameMap::MapCell* out_cell) {
    if (!out_cell) {
        return false;
    }

    // New packed token format:
    //   material|e0,e1,...,e15
    // each eN is uint16 (decimal or 0x-prefixed hex):
    //   low byte: tile_id, high byte: spec
    const std::vector<std::string> parts = split(token, '|');
    if (parts.size() != 2) {
        return false;
    }

    int material = 0;
    if (!parse_int(parts[0], &material)) {
        return false;
    }

    std::array<std::uint16_t, kSubtilesPerCell> entries{};
    if (!parse_csv_uint16_16(parts[1], &entries)) {
        return false;
    }

    out_cell->material = (material == 0) ? 0 : 1;
    out_cell->has_subtiles = true;
    out_cell->subtiles = entries;
    return true;
}
}  // namespace

int GameMap::RowCount() const {
    return static_cast<int>(_cells.size());
}

int GameMap::ColCount() const {
    if (_cells.empty()) {
        return 0;
    }

    return static_cast<int>(_cells.front().size());
}

bool GameMap::IsInBounds(int row, int col) const {
    if (row < 0 || col < 0) {
        return false;
    }
    if (row >= static_cast<int>(_cells.size())) {
        return false;
    }
    if (col >= static_cast<int>(_cells[row].size())) {
        return false;
    }

    return true;
}

bool GameMap::AreaIsAvailable(int row, int col) const {
    if (_cells.empty()) {
        return false;
    }
    if (!IsInBounds(row, col)) {
        return false;
    }

    const MapCell& cell = _cells[row][col];
    if (!cell.has_subtiles) {
        return cell.material == 0;
    }

    for (std::uint16_t entry : cell.subtiles) {
        if (!subtile_is_walkable(entry)) {
            return false;
        }
    }

    return true;
}

int GameMap::GetElement(int row, int col) const {
    if (_cells.empty() || !IsInBounds(row, col)) {
        // Treat out-of-bounds as blocked to prevent undefined access.
        return 1;
    }

    return AreaIsAvailable(row, col) ? 0 : 1;
}

bool GameMap::MatchesDimensions(int expected_rows, int expected_cols) const {
    if (expected_rows <= 0 || expected_cols <= 0) {
        return false;
    }
    if (RowCount() != expected_rows) {
        return false;
    }

    for (const auto& row : _cells) {
        if (static_cast<int>(row.size()) != expected_cols) {
            return false;
        }
    }

    return true;
}

int GameMap::GetCellMaterial(int row, int col) const {
    if (!IsInBounds(row, col)) {
        return 1;
    }

    return _cells[row][col].material;
}

bool GameMap::HasDestructibleSubtiles(int row, int col) const {
    if (!IsInBounds(row, col)) {
        return false;
    }

    return _cells[row][col].has_subtiles;
}

std::uint8_t GameMap::GetSubtileId(int row, int col, int subtile_index) const {
    if (!IsInBounds(row, col)) {
        return 0;
    }
    if (subtile_index < 0 || subtile_index >= kSubtilesPerCell) {
        return 0;
    }
    if (!_cells[row][col].has_subtiles) {
        return 0;
    }

    return subtile_tile_id(_cells[row][col].subtiles[subtile_index]);
}

std::uint8_t GameMap::GetSubtileHealth(int row, int col, int subtile_index) const {
    if (!IsInBounds(row, col)) {
        return 0;
    }
    if (subtile_index < 0 || subtile_index >= kSubtilesPerCell) {
        return 0;
    }
    if (!_cells[row][col].has_subtiles) {
        return 0;
    }

    return subtile_health_from_spec(subtile_spec(_cells[row][col].subtiles[subtile_index]));
}

std::uint8_t GameMap::GetSubtileDestructionMode(int row, int col, int subtile_index) const {
    if (!IsInBounds(row, col)) {
        return 0;
    }
    if (subtile_index < 0 || subtile_index >= kSubtilesPerCell) {
        return 0;
    }
    if (!_cells[row][col].has_subtiles) {
        return 0;
    }

    return subtile_destruction_mode_from_spec(subtile_spec(_cells[row][col].subtiles[subtile_index]));
}

std::uint8_t GameMap::GetSubtileMovement(int row, int col, int subtile_index) const {
    if (!IsInBounds(row, col)) {
        return 0;
    }
    if (subtile_index < 0 || subtile_index >= kSubtilesPerCell) {
        return 0;
    }
    if (!_cells[row][col].has_subtiles) {
        return 0;
    }

    return subtile_movement_from_spec(subtile_spec(_cells[row][col].subtiles[subtile_index]));
}

bool GameMap::IsSubtileDestroyed(int row, int col, int subtile_index) const {
    if (!IsInBounds(row, col)) {
        return false;
    }
    if (subtile_index < 0 || subtile_index >= kSubtilesPerCell) {
        return false;
    }
    if (!_cells[row][col].has_subtiles) {
        return false;
    }

    return GetSubtileHealth(row, col, subtile_index) == 0;
}

bool GameMap::DamageSubtile(int row, int col, int subtile_index) {
    if (!IsInBounds(row, col)) {
        return false;
    }
    if (subtile_index < 0 || subtile_index >= kSubtilesPerCell) {
        return false;
    }

    MapCell& cell = _cells[row][col];
    if (cell.material == 0 && !cell.has_subtiles) {
        return false;
    }

    if (!cell.has_subtiles) {
        // Upgrade legacy blocked cells on demand so gameplay systems can still apply damage.
        ensure_default_destructible_subtiles(&cell);
        if (!cell.has_subtiles) {
            return false;
        }
    }

    const std::uint16_t entry = cell.subtiles[subtile_index];
    const std::uint8_t spec = subtile_spec(entry);
    const std::uint8_t health = subtile_health_from_spec(spec);
    if (health == 0) {
        return false;
    }

    const std::uint8_t destruction_mode = subtile_destruction_mode_from_spec(spec);
    if (!can_apply_normal_damage(destruction_mode)) {
        return false;
    }

    cell.subtiles[subtile_index] = set_subtile_health(entry, static_cast<std::uint8_t>(health - 1));
    return true;
}

bool GameMap::WorldToSubtile(int world_x, int world_y, int* out_row, int* out_col,
                             int* out_subtile_index) const {
    if (!out_row || !out_col || !out_subtile_index) {
        return false;
    }
    if (_size <= 0 || world_x < 0 || world_y < 0) {
        return false;
    }

    const int row = world_y / _size;
    const int col = world_x / _size;
    if (!IsInBounds(row, col)) {
        return false;
    }

    const int subtile_size = _size / kSubtilesPerAxis;
    if (subtile_size <= 0) {
        return false;
    }

    const int local_x = world_x % _size;
    const int local_y = world_y % _size;
    int subtile_col = local_x / subtile_size;
    int subtile_row = local_y / subtile_size;

    if (subtile_col >= kSubtilesPerAxis) {
        subtile_col = kSubtilesPerAxis - 1;
    }
    if (subtile_row >= kSubtilesPerAxis) {
        subtile_row = kSubtilesPerAxis - 1;
    }

    *out_row = row;
    *out_col = col;
    *out_subtile_index = subtile_row * kSubtilesPerAxis + subtile_col;
    return true;
}

bool GameMap::DamageAtWorldPosition(int world_x, int world_y) {
    int row = 0;
    int col = 0;
    int subtile_index = 0;
    if (!WorldToSubtile(world_x, world_y, &row, &col, &subtile_index)) {
        return false;
    }

    return DamageSubtile(row, col, subtile_index);
}

GameMap::GameMap(int grid_height, int grid_width, int grid_size, const std::string& map_path)
    : _height(grid_height), _width(grid_width), _size(grid_size) {
    std::ifstream filestream(map_path);
    if (filestream.is_open()) {
        std::string line;
        while (std::getline(filestream, line)) {
            std::istringstream ls(line);
            std::vector<MapCell> row;
            std::string token;
            while (ls >> token) {
                MapCell cell;
                if (token.find('|') != std::string::npos) {
                    if (!parse_packed_cell_token(token, &cell) &&
                        !parse_legacy_extended_cell_token(token, &cell)) {
                        // If parsing fails, preserve legacy behavior and treat the token as blocked.
                        cell = make_legacy_cell(1);
                    }
                } else {
                    int legacy_value = 0;
                    if (!parse_int(token, &legacy_value)) {
                        legacy_value = 1;
                    }
                    cell = make_legacy_cell(legacy_value);
                }
                row.push_back(cell);
            }

            if (!row.empty()) {
                _cells.emplace_back(std::move(row));
            }
        }
    } else {
        _cells.reserve(tempgamemap.size());
        for (const auto& legacy_row : tempgamemap) {
            std::vector<MapCell> row;
            row.reserve(legacy_row.size());
            for (int value : legacy_row) {
                row.push_back(make_legacy_cell(value));
            }
            _cells.emplace_back(std::move(row));
        }
    }
}
