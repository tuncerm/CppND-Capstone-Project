//Copied From CppND-Capstone-Snake-Game
#ifndef PLAYER_H
#define PLAYER_H

#include <vector>
#include "character.h"

class Player : public Character {
public:
    Player(int startX, int startY, Direction direction, int speed) : Character(startX, startY, direction, speed) {}
};

#endif
