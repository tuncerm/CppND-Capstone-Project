#ifndef PATH_RESOLVER_H
#define PATH_RESOLVER_H

#include <string>

std::string resolve_game_config_path();
std::string resolve_game_map_path(const char* configured_map_file);

#endif  // PATH_RESOLVER_H
