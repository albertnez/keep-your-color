#ifndef PLAYER_H
#define PLAYER_H

#include "actor.h"
#include "utils.h"


class Player : public Actor {
public:
  Player(Game & game, int type, float speed);
  ~Player();
  void update(float delta_time);
  void render(); 
private:
  float width, height;
};

#endif  // PLAYER_H
