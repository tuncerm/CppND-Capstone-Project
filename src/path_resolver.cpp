#include "path_resolver.h"
#include <cstdio>
#include <vector>

namespace {
bool file_exists(const std::string& path) {
    if (path.empty()) {
        return false;
    }

    FILE* handle = std::fopen(path.c_str(), "rb");
    if (!handle) {
        return false;
    }
    std::fclose(handle);
    return true;
}

std::string resolve_first_existing(const std::vector<std::string>& candidates,
                                   const std::string& fallback) {
    for (const auto& path : candidates) {
        if (file_exists(path)) {
            return path;
        }
    }

    return fallback;
}
}  // namespace

std::string resolve_game_config_path() {
    const std::vector<std::string> candidates = {"config/game_config.json",
                                                 "../config/game_config.json",
                                                 "game_config.json"};
    return resolve_first_existing(candidates, candidates.front());
}

std::string resolve_game_map_path(const char* configured_map_file) {
    const std::string map_file =
        (configured_map_file && configured_map_file[0] != '\0') ? configured_map_file : "game.map";

    const std::vector<std::string> candidates = {
        map_file, "config/" + map_file, "../" + map_file, "../config/" + map_file, "game.map"};
    return resolve_first_existing(candidates, map_file);
}
