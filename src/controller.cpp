//Copied From CppND-Capstone-Snake-Game
#include "controller.h"
#include <iostream>
#include "SDL.h"
#include "character.h"

void Controller::ChangeDirection(Character &character, Character::Direction input,
                                 Character::Direction opposite) const {
    if (character.direction != opposite || character.size == 1) character.direction = input;
    return;
}

void Controller::HandleInput(bool &running, Character &character) const {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            running = false;
        } else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_UP:
                    ChangeDirection(character, Character::Direction::kUp,
                                    Character::Direction::kDown);
                    break;

                case SDLK_DOWN:
                    ChangeDirection(character, Character::Direction::kDown,
                                    Character::Direction::kUp);
                    break;

                case SDLK_LEFT:
                    ChangeDirection(character, Character::Direction::kLeft,
                                    Character::Direction::kRight);
                    break;

                case SDLK_RIGHT:
                    ChangeDirection(character, Character::Direction::kRight,
                                    Character::Direction::kLeft);
                    break;
            }
        }
    }
}