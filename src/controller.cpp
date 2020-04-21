//Copied From CppND-Capstone-Snake-Game
#include "controller.h"
#include <iostream>
#include "SDL.h"
#include "player.h"

void Controller::ChangeDirection(Player &player, Player::Direction input) {
    if (player.GetY() % player.GetGridSize() || player.GetX() % player.GetGridSize()) {
        return player.Move();
    }
    if (player.GetDirection() == input) {
        player.Move();
    } else {
        player.SetDirection(input);
    }
    return;
}

void Controller::FireProjectile(Player &player) const {
    // ToDo Implement Fire Projectile
    std::cout << "Fire in the hole!" << std::endl;
}

void Controller::HandlePause() const {
    // ToDo Implement Pause
    std::cout << "Paused" << std::endl;
}

void Controller::HandleInput(bool &running, Player &player) const {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            running = false;
        } else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_UP:
                    ChangeDirection(player, Character::Direction::kUp);
                    break;

                case SDLK_DOWN:
                    ChangeDirection(player, Character::Direction::kDown);
                    break;

                case SDLK_LEFT:
                    ChangeDirection(player, Character::Direction::kLeft);
                    break;

                case SDLK_RIGHT:
                    ChangeDirection(player, Character::Direction::kRight);
                    break;

                case SDLK_f:
                    FireProjectile(player);
                    break;

                case SDLK_p:
                    HandlePause();
                    break;
            }
        }
    }
}