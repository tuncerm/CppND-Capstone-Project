//Copied From CppND-Capstone-Snake-Game
#include "game.h"
#include <iostream>
#include "SDL.h"

Game::Game(int grid_size, int grid_width, int grid_height, std::shared_ptr<GameMap> map_ptr) :
        player(grid_size * (grid_width / 2), grid_size * (grid_height - 2), Player::Direction::kUp, 4),
        _map_ptr(map_ptr),
        _grid_size(grid_size){
//    PlaceFood();
}

void Game::Run(Controller const &controller, Renderer &renderer, std::size_t target_frame_duration) {
    Uint32 title_timestamp = SDL_GetTicks();
    Uint32 frame_start;
    Uint32 frame_end;
    Uint32 frame_duration;
    int frame_count = 0;
    bool running = true;

    while (running) {
        frame_start = SDL_GetTicks();

        // Input, Update, Render - the main game loop.
        controller.HandleInput(running, player);
        Update();
        renderer.Render(player);

        frame_end = SDL_GetTicks();

        // Keep track of how long each loop through the input/update/render cycle takes.
        frame_count++;
        frame_duration = frame_end - frame_start;

        // After every second, update the window title.
        if (frame_end - title_timestamp >= 1000) {
            renderer.UpdateWindowTitle(score, frame_count);
            frame_count = 0;
            title_timestamp = frame_end;
        }

        // If the time for this frame is too small
        // (i.e. frame_duration is smaller than the target ms_per_frame),
        // delay the loop to achieve the correct frame rate.
        if (frame_duration < target_frame_duration) {
            SDL_Delay(target_frame_duration - frame_duration);
        }
    }
}

//void Game::PlaceFood() {
//    int x, y;
//    while (true) {
//        x = random_w(engine);
//        y = random_h(engine);
//        // Check that the location is not occupied by a character item before placing
//        // food.
//        if (!character.CharacterCell(x, y)) {
//            food.x = x;
//            food.y = y;
//            return;
//        }
//    }
//}

void Game::Update() {
    if (!player.IsAlive()) return;
//    player.Update();
//    int new_x = player.GetX();
//    int new_y = player.GetY();
}

int Game::GetScore() const { return score; }