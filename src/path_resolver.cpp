#include "path_resolver.h"
#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <vector>
#include "SDL3/SDL.h"

namespace {
bool file_exists(const std::filesystem::path& path) {
    if (path.empty() || !path.has_filename()) {
        return false;
    }

    std::error_code ec;
    return std::filesystem::exists(path, ec) && std::filesystem::is_regular_file(path, ec);
}

std::filesystem::path get_executable_base_dir() {
    const char* raw_base_path = SDL_GetBasePath();
    if (raw_base_path && raw_base_path[0] != '\0') {
        std::filesystem::path base(raw_base_path);
        SDL_free((void*)raw_base_path);
        return base;
    }

    if (raw_base_path) {
        SDL_free((void*)raw_base_path);
    }

    return std::filesystem::current_path();
}

std::vector<std::filesystem::path> get_search_roots() {
    std::vector<std::filesystem::path> roots;

    const std::filesystem::path base_dir = get_executable_base_dir();
    roots.push_back(base_dir);
    if (base_dir.has_parent_path()) {
        roots.push_back(base_dir.parent_path());
        if (base_dir.parent_path().has_parent_path()) {
            roots.push_back(base_dir.parent_path().parent_path());
        }
    }
    roots.push_back(std::filesystem::current_path());

    std::vector<std::filesystem::path> unique_roots;
    unique_roots.reserve(roots.size());
    for (const auto& root : roots) {
        const std::filesystem::path normalized = root.lexically_normal();
        if (std::find(unique_roots.begin(), unique_roots.end(), normalized) == unique_roots.end()) {
            unique_roots.push_back(normalized);
        }
    }

    return unique_roots;
}

std::string normalize_path_string(const std::filesystem::path& path) {
    return path.lexically_normal().string();
}

std::string resolve_first_existing(const std::vector<std::filesystem::path>& candidates,
                                   const std::filesystem::path& fallback) {
    for (const auto& path : candidates) {
        if (file_exists(path)) {
            return normalize_path_string(path);
        }
    }

    return normalize_path_string(fallback);
}
}  // namespace

std::string resolve_game_config_path() {
    const auto search_roots = get_search_roots();
    std::vector<std::filesystem::path> candidates;
    candidates.reserve(search_roots.size() * 2);

    for (const auto& root : search_roots) {
        candidates.push_back(root / "config" / "game_config.json");
        candidates.push_back(root / "game_config.json");
    }

    // Legacy fallbacks for compatibility when no preferred path exists.
    candidates.push_back("config/game_config.json");
    candidates.push_back("../config/game_config.json");
    candidates.push_back("game_config.json");

    return resolve_first_existing(candidates, "config/game_config.json");
}

std::string resolve_game_map_path(const char* configured_map_file) {
    const std::string map_file =
        (configured_map_file && configured_map_file[0] != '\0') ? configured_map_file : "game.map";
    const std::filesystem::path configured_path(map_file);

    if (configured_path.is_absolute() && file_exists(configured_path)) {
        return normalize_path_string(configured_path);
    }

    const auto search_roots = get_search_roots();
    std::vector<std::filesystem::path> candidates;
    candidates.reserve(search_roots.size() * 2 + 3);

    for (const auto& root : search_roots) {
        candidates.push_back(root / configured_path);
        candidates.push_back(root / "config" / configured_path);
    }

    // Legacy fallbacks for compatibility with old launch patterns.
    candidates.push_back(configured_path);
    candidates.push_back(std::filesystem::path("config") / configured_path);
    candidates.push_back("game.map");

    return resolve_first_existing(candidates, configured_path);
}
