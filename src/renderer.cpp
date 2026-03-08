#include "renderer.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include "../shared/config/config_manager.h"
#include "../shared/error_handler/error_handler.h"
#include "constants.h"
#include "enemy.h"
#include "player.h"

Renderer::Renderer(const int grid_size, const int grid_width, const int grid_height,
                   std::shared_ptr<GameMap> map_ptr, SDLContext* context,
                   const std::string& config_path)
    : _context(context),
      _map_ptr(map_ptr),
      _screen_width(grid_size * grid_width),
      _screen_height(grid_size * grid_height),
      _grid_width(grid_width),
      _grid_height(grid_height),
      _grid_size(grid_size) {
    // Initialize configuration for colors and character rendering
    config_manager_init(&_config, "Character Game Renderer");

    // Register color configuration entries
    config_register_entry(&_config, "colors", "wall_color", CONFIG_TYPE_COLOR_RGBA,
                          config_make_rgba(255, 0, 0, 255), false);
    config_register_entry(&_config, "colors", "floor_color", CONFIG_TYPE_COLOR_RGBA,
                          config_make_rgba(0, 0, 255, 255), false);
    config_register_entry(&_config, "colors", "player_color", CONFIG_TYPE_COLOR_RGBA,
                          config_make_rgba(0, 0, 0, 255), false);
    config_register_entry(&_config, "colors", "player_eye_color", CONFIG_TYPE_COLOR_RGBA,
                          config_make_rgba(0, 0, 255, 255), false);
    config_register_entry(&_config, "colors", "enemy_color", CONFIG_TYPE_COLOR_RGBA,
                          config_make_rgba(170, 170, 0, 255), false);
    config_register_entry(&_config, "colors", "enemy_eye_color", CONFIG_TYPE_COLOR_RGBA,
                          config_make_rgba(0, 0, 255, 255), false);

    // Register character rendering configuration entries
    config_register_entry(&_config, "character", "eye_offset_x", CONFIG_TYPE_INT,
                          config_make_int(EYE_OFFSET_X), false);
    config_register_entry(&_config, "character", "eye_offset_y", CONFIG_TYPE_INT,
                          config_make_int(EYE_OFFSET_Y), false);
    config_register_entry(&_config, "character", "eye_width", CONFIG_TYPE_INT,
                          config_make_int(EYE_WIDTH), false);
    config_register_entry(&_config, "character", "eye_height", CONFIG_TYPE_INT,
                          config_make_int(EYE_HEIGHT), false);
    config_register_entry(&_config, "character", "eye_spacing", CONFIG_TYPE_INT,
                          config_make_int(EYE_SPACING), false);
    config_register_entry(&_config, "character", "mouth_offset_x", CONFIG_TYPE_INT,
                          config_make_int(MOUTH_OFFSET_X), false);
    config_register_entry(&_config, "character", "mouth_offset_y", CONFIG_TYPE_INT,
                          config_make_int(MOUTH_OFFSET_Y), false);
    config_register_entry(&_config, "character", "mouth_width", CONFIG_TYPE_INT,
                          config_make_int(MOUTH_WIDTH), false);
    config_register_entry(&_config, "character", "mouth_height", CONFIG_TYPE_INT,
                          config_make_int(MOUTH_HEIGHT), false);

    // Load configuration file
    if (!config_manager_load(&_config, config_path.c_str())) {
        std::cerr << "Warning: Renderer failed to load config from '" << config_path
                  << "', using defaults.\n";
        ErrorHandler_Log();
        ErrorHandler_Clear();
    }

    RefreshRenderConfig();
}

Renderer::~Renderer() {}

void Renderer::Render(Player& player, const Enemy& enemy) {
    std::vector<RenderObject> render_objects;

    // Add map objects to render list
    for (int row = 0; row < _map_ptr->RowCount(); ++row) {
        for (int col = 0; col < _map_ptr->ColCount(); ++col) {
            SDL_FRect block;
            block.w = static_cast<float>(_grid_size);
            block.h = static_cast<float>(_grid_size);
            block.x = static_cast<float>(col * _grid_size);
            block.y = static_cast<float>(row * _grid_size);

            if (_map_ptr->GetElement(row, col) == 1) {
                render_objects.push_back({block, _wall_color});
            } else {
                render_objects.push_back({block, _floor_color});
            }
        }
    }

    // Add player and enemy to render list
    if (player.IsMoving()) {
        player.Move();
    }
    AddCharacterObjects(render_objects, ObjectType::kPlayer, player.GetDirection(), player.GetX(),
                        player.GetY());
    AddCharacterObjects(render_objects, ObjectType::kEnemy, enemy.GetDirection(), enemy.GetX(),
                        enemy.GetY());

    // Sort objects by color
    std::sort(render_objects.begin(), render_objects.end(),
              [](const RenderObject& a, const RenderObject& b) { return a.color < b.color; });

    // Render sorted objects
    ConfigColorRGBA current_color = {0, 0, 0, 0};
    for (const auto& obj : render_objects) {
        if (!(obj.color == current_color)) {
            current_color = obj.color;
            SDL_SetRenderDrawColor(sdl_get_renderer(_context), current_color.r, current_color.g,
                                   current_color.b, current_color.a);
        }
        SDL_RenderFillRect(sdl_get_renderer(_context), &obj.rect);
    }

    SDL_RenderPresent(sdl_get_renderer(_context));
}

void Renderer::UpdateWindowTitle(int score, int fps) {
    std::string title{"PlayGame Score: " + std::to_string(score) + " FPS: " + std::to_string(fps)};
    sdl_set_window_title(_context, title.c_str());
}

void Renderer::AddCharacterObjects(std::vector<RenderObject>& objects, Renderer::ObjectType ot,
                                   Character::Direction d, int posX, int posY) {
    auto makeBlock = [&](float x, float y, float w, float h) -> SDL_FRect {
        return SDL_FRect{x, y, w, h};
    };

    if (ot == ObjectType::kPlayer || ot == ObjectType::kEnemy) {
        ConfigColorRGBA body_color;
        ConfigColorRGBA eye_color;
        if (ot == ObjectType::kPlayer) {
            body_color = _player_color;
            eye_color = _player_eye_color;
        } else {
            body_color = _enemy_color;
            eye_color = _enemy_eye_color;
        }

        // Add character body
        SDL_FRect base = makeBlock(posX, posY, _grid_size, _grid_size);
        objects.push_back({base, body_color});

        if (d == Character::Direction::kUp) {
            SDL_FRect eye1 = makeBlock(posX + _eye_offset_x, posY, _eye_width, _eye_height);
            SDL_FRect eye2 =
                makeBlock(posX + _eye_offset_x + _eye_spacing, posY, _eye_width, _eye_height);
            SDL_FRect mouth =
                makeBlock(posX + _mouth_offset_x, posY + _mouth_offset_y, _mouth_width, _mouth_height);
            objects.push_back({eye1, eye_color});
            objects.push_back({eye2, eye_color});
            objects.push_back({mouth, eye_color});
        }
        if (d == Character::Direction::kDown) {
            SDL_FRect eye1 =
                makeBlock(posX + _eye_offset_x, posY + _mouth_offset_y, _eye_width, _eye_height);
            SDL_FRect eye2 = makeBlock(posX + _eye_offset_x + _eye_spacing, posY + _mouth_offset_y,
                                       _eye_width, _eye_height);
            SDL_FRect mouth = makeBlock(posX + _mouth_offset_x, posY, _mouth_width, _mouth_height);
            objects.push_back({eye1, eye_color});
            objects.push_back({eye2, eye_color});
            objects.push_back({mouth, eye_color});
        }
        if (d == Character::Direction::kLeft) {
            SDL_FRect eye1 = makeBlock(posX, posY + _eye_offset_y, _eye_height, _eye_width);
            SDL_FRect eye2 =
                makeBlock(posX, posY + _eye_offset_y + _eye_spacing, _eye_height, _eye_width);
            SDL_FRect mouth =
                makeBlock(posX + _mouth_offset_y, posY + _eye_offset_y, _mouth_height, _mouth_width);
            objects.push_back({eye1, eye_color});
            objects.push_back({eye2, eye_color});
            objects.push_back({mouth, eye_color});
        }
        if (d == Character::Direction::kRight) {
            SDL_FRect eye1 =
                makeBlock(posX + _mouth_offset_y, posY + _eye_offset_y, _eye_height, _eye_width);
            SDL_FRect eye2 = makeBlock(posX + _mouth_offset_y, posY + _eye_offset_y + _eye_spacing,
                                       _eye_height, _eye_width);
            SDL_FRect mouth = makeBlock(posX, posY + _eye_offset_y, _mouth_height, _mouth_width);
            objects.push_back({eye1, eye_color});
            objects.push_back({eye2, eye_color});
            objects.push_back({mouth, eye_color});
        }
    }
}

void Renderer::RefreshRenderConfig() {
    _wall_color = config_get_rgba(&_config, "colors", "wall_color", {255, 0, 0, 255});
    _floor_color = config_get_rgba(&_config, "colors", "floor_color", {0, 0, 255, 255});
    _player_color = config_get_rgba(&_config, "colors", "player_color", {0, 0, 0, 255});
    _player_eye_color = config_get_rgba(&_config, "colors", "player_eye_color", {0, 0, 255, 255});
    _enemy_color = config_get_rgba(&_config, "colors", "enemy_color", {170, 170, 0, 255});
    _enemy_eye_color = config_get_rgba(&_config, "colors", "enemy_eye_color", {0, 0, 255, 255});

    _eye_offset_x = config_get_int(&_config, "character", "eye_offset_x", EYE_OFFSET_X);
    _eye_offset_y = config_get_int(&_config, "character", "eye_offset_y", EYE_OFFSET_Y);
    _eye_width = config_get_int(&_config, "character", "eye_width", EYE_WIDTH);
    _eye_height = config_get_int(&_config, "character", "eye_height", EYE_HEIGHT);
    _eye_spacing = config_get_int(&_config, "character", "eye_spacing", EYE_SPACING);
    _mouth_offset_x = config_get_int(&_config, "character", "mouth_offset_x", MOUTH_OFFSET_X);
    _mouth_offset_y = config_get_int(&_config, "character", "mouth_offset_y", MOUTH_OFFSET_Y);
    _mouth_width = config_get_int(&_config, "character", "mouth_width", MOUTH_WIDTH);
    _mouth_height = config_get_int(&_config, "character", "mouth_height", MOUTH_HEIGHT);
}
