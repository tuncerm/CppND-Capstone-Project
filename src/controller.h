//Copied From CppND-Capstone-Snake-Game
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "character.h"

class Controller {
public:
    void HandleInput(bool &running, Character &character) const;

private:
    void ChangeDirection(Character &character, Character::Direction input,
                         Character::Direction opposite) const;
};

#endif