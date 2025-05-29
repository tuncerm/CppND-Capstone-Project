#include "renderer.h"
#include <iostream>
#include <string>

Renderer::Renderer(const int grid_size,
                   const int grid_width,
                   const int grid_height,
                   std::shared_ptr<GameMap> map_ptr)
    : _map_ptr(map_ptr),
      _grid_size(grid_size),
      _screen_width(grid_size * grid_width),
      _screen_height(grid_size * grid_height),
      _grid_width(grid_width),
      _grid_height(grid_height)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cerr << "SDL could not initialize.\n";
        std::cerr << "SDL_Error: " << SDL_GetError() << "\n";
    }

    sdl_window = SDL_CreateWindow("Character Game", _screen_width, _screen_height, 0);
    if (!sdl_window)
    {
        std::cerr << "Window could not be created.\n";
        std::cerr << "SDL_Error: " << SDL_GetError() << "\n";
    }

    sdl_renderer = SDL_CreateRenderer(sdl_window, nullptr);
    if (!sdl_renderer)
    {
        std::cerr << "Renderer could not be created.\n";
        std::cerr << "SDL_Error: " << SDL_GetError() << "\n";
    }
}

Renderer::~Renderer()
{
    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
}

void Renderer::Render(Player &player, Enemy const enemy)
{
    SDL_FRect block;
    block.w = static_cast<float>(_grid_size);
    block.h = static_cast<float>(_grid_size);

    for (int row = 0; row < _map_ptr->RowCount(); ++row)
    {
        for (int col = 0; col < _map_ptr->ColCount(); ++col)
        {
            block.x = static_cast<float>(col * _grid_size);
            block.y = static_cast<float>(row * _grid_size);
            if (_map_ptr->GetElement(row, col) == 1)
            {
                SDL_SetRenderDrawColor(sdl_renderer, 0xFF, 0x00, 0x00, 0xFF);
            }
            else
            {
                SDL_SetRenderDrawColor(sdl_renderer, 0x00, 0x00, 0xFF, 0xFF);
            }
            SDL_RenderFillRect(sdl_renderer, &block);
        }
    }

    if (player.IsMoving())
    {
        player.Move();
    }

    RenderObject(ObjectType::kPlayer, player.GetDirection(), player.GetX(), player.GetY());
    RenderObject(ObjectType::kEnemy, enemy.GetDirection(), enemy.GetX(), enemy.GetY());

    SDL_RenderPresent(sdl_renderer);
}

void Renderer::UpdateWindowTitle(int score, int fps)
{
    std::string title{"PlayGame Score: " + std::to_string(score) + " FPS: " + std::to_string(fps)};
    SDL_SetWindowTitle(sdl_window, title.c_str());
}

void Renderer::RenderObject(Renderer::ObjectType ot, Character::Direction d, int posX, int posY)
{
    auto makeBlock = [&](float x, float y, float w, float h) -> SDL_FRect
    {
        return SDL_FRect{x, y, w, h};
    };

    if (ot == ObjectType::kPlayer || ot == ObjectType::kEnemy)
    {
        if (ot == ObjectType::kPlayer)
            SDL_SetRenderDrawColor(sdl_renderer, 0x00, 0x00, 0x00, 0xFF);
        else
            SDL_SetRenderDrawColor(sdl_renderer, 0xAA, 0xAA, 0x00, 0xFF);

        SDL_FRect base = makeBlock(posX, posY, _grid_size, _grid_size);
        SDL_RenderFillRect(sdl_renderer, &base);

        SDL_SetRenderDrawColor(sdl_renderer, 0x00, 0x00, 0xFF, 0xFF);

        if (d == Character::Direction::kUp)
        {
            SDL_FRect eye1 = makeBlock(posX + 8, posY, 6, 8);
            SDL_FRect eye2 = makeBlock(posX + 18, posY, 6, 8);
            SDL_FRect mouth = makeBlock(posX + 8, posY + 24, 16, 6);
            SDL_RenderFillRect(sdl_renderer, &eye1);
            SDL_RenderFillRect(sdl_renderer, &eye2);
            SDL_RenderFillRect(sdl_renderer, &mouth);
        }
        if (d == Character::Direction::kDown)
        {
            SDL_FRect eye1 = makeBlock(posX + 8, posY + 24, 6, 8);
            SDL_FRect eye2 = makeBlock(posX + 18, posY + 24, 6, 8);
            SDL_FRect mouth = makeBlock(posX + 8, posY, 16, 6);
            SDL_RenderFillRect(sdl_renderer, &eye1);
            SDL_RenderFillRect(sdl_renderer, &eye2);
            SDL_RenderFillRect(sdl_renderer, &mouth);
        }
        if (d == Character::Direction::kLeft)
        {
            SDL_FRect eye1 = makeBlock(posX, posY + 8, 8, 6);
            SDL_FRect eye2 = makeBlock(posX, posY + 18, 8, 6);
            SDL_FRect mouth = makeBlock(posX + 24, posY + 8, 6, 16);
            SDL_RenderFillRect(sdl_renderer, &eye1);
            SDL_RenderFillRect(sdl_renderer, &eye2);
            SDL_RenderFillRect(sdl_renderer, &mouth);
        }
        if (d == Character::Direction::kRight)
        {
            SDL_FRect eye1 = makeBlock(posX + 24, posY + 8, 8, 6);
            SDL_FRect eye2 = makeBlock(posX + 24, posY + 18, 8, 6);
            SDL_FRect mouth = makeBlock(posX, posY + 8, 6, 16);
            SDL_RenderFillRect(sdl_renderer, &eye1);
            SDL_RenderFillRect(sdl_renderer, &eye2);
            SDL_RenderFillRect(sdl_renderer, &mouth);
        }
    }
}
