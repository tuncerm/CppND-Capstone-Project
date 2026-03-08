#include "controller.h"
#include <SDL3/SDL.h>
#include <iostream>
#include "player.h"

void Controller::ChangeDirection(Player& player, Player::Direction input) {
    if (input == Player::Direction::kNone) {
        return player.IsMoving(false);
    }

    if (player.GetDirection() != input) {
        player.IsMoving(false);
        return player.SetDirection(input);
    } else {
        return player.IsMoving(true);
    }
}

void Controller::FireProjectile(Player& player) const {
    if (player.DamageFrontSubtile()) {
        std::cout << "Subtile damaged." << std::endl;
    }
}

void Controller::HandlePause() const {
    std::cout << "Paused" << std::endl;
}

void Controller::HandleInput(bool& running, Player& player) const {
    const bool* keystates = SDL_GetKeyboardState(nullptr);  // SDL3 returns bool*
    bool fire_pressed = false;
    bool pause_pressed = false;

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT) {  // SDL_QUIT renamed
            running = false;
        } else if (e.type == SDL_EVENT_KEY_DOWN) {
            if (e.key.scancode == SDL_SCANCODE_F) {
                fire_pressed = true;
            } else if (e.key.scancode == SDL_SCANCODE_P) {
                pause_pressed = true;
            }
        }
    }

    if (fire_pressed) {
        FireProjectile(player);
    }

    if (pause_pressed || keystates[SDL_SCANCODE_P]) {
        HandlePause();
        return;
    }

    if (keystates[SDL_SCANCODE_UP]) {
        return ChangeDirection(player, Character::Direction::kUp);
    }

    if (keystates[SDL_SCANCODE_DOWN]) {
        return ChangeDirection(player, Character::Direction::kDown);
    }

    if (keystates[SDL_SCANCODE_LEFT]) {
        return ChangeDirection(player, Character::Direction::kLeft);
    }

    if (keystates[SDL_SCANCODE_RIGHT]) {
        return ChangeDirection(player, Character::Direction::kRight);
    }

    return ChangeDirection(player, Character::Direction::kNone);
}
