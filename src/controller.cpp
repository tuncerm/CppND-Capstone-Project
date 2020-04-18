//Copied From CppND-Capstone-Snake-Game
#include "controller.h"
#include <iostream>
#include "SDL.h"
#include "character.h"

void Controller::ChangeDirection(Character &character, Character::Direction input) const {
    character.direction = input;
    return;
}

void Controller::FireProjectile(Character &character) const {
    // ToDo Implement Fire Projectile
    std::cout << "Fire in the hole!" << std::endl;
}   

void Controller::HandlePause() const {
    // ToDo Implement Pause
    std::cout << "Paused" << std::endl;
}   

void Controller::HandleInput(bool &running, Character &character) const {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            running = false;
        } else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_UP:
                    ChangeDirection(character, Character::Direction::kUp);
                    break;

                case SDLK_DOWN:
                    ChangeDirection(character, Character::Direction::kDown);
                    break;

                case SDLK_LEFT:
                    ChangeDirection(character, Character::Direction::kLeft);
                    break;

                case SDLK_RIGHT:
                    ChangeDirection(character, Character::Direction::kRight);
                    break;
                    
                case SDLK_f:
                    FireProjectile(character);
                    break;

                case SDLK_p:
                    HandlePause();
                    break;
            }
        }
    }
}