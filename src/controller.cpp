//Copied From CppND-Capstone-Snake-Game
#include "controller.h"
#include <iostream>
#include "SDL.h"
#include "player.h"

void Controller::ChangeDirection(Player &player, Player::Direction input) {
    if (input == Player::Direction::kNone)
    {
        return player.IsMoving(false);
    }

    if (player.GetDirection() != input)
    {
        player.IsMoving(false);
        return player.SetDirection(input);
    } else {
        return player.IsMoving(true);
    }
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
    const Uint8 *keystates = SDL_GetKeyboardState( NULL );

    SDL_Event e;
    while( SDL_PollEvent( &e ) != 0 ) {
        if( e.type == SDL_QUIT )
        {
            running = false;
        }
    }

    if ( keystates[ SDL_SCANCODE_F ] ) {
        FireProjectile(player);
    }

    if ( keystates[ SDL_SCANCODE_P ] ) {
        HandlePause();
    }

    if( keystates[ SDL_SCANCODE_UP ] ) {
        return ChangeDirection(player, Character::Direction::kUp);
    }

    if( keystates[ SDL_SCANCODE_DOWN ] ) {
        return ChangeDirection(player, Character::Direction::kDown);
    }

    if( keystates[ SDL_SCANCODE_LEFT ] ) {
        return ChangeDirection(player, Character::Direction::kLeft);
    }

    if( keystates[ SDL_SCANCODE_RIGHT ] ) {
        return ChangeDirection(player, Character::Direction::kRight);
    }

    return ChangeDirection(player, Character::Direction::kNone);
}