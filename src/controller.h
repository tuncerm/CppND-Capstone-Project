// Copied From CppND-Capstone-Snake-Game
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "player.h"

class Controller {
   public:
    void HandleInput(bool& running, Player& player) const;

   private:
    static void ChangeDirection(Player& player, Player::Direction input);

    void FireProjectile(Player& player) const;

    void HandlePause() const;
};

#endif