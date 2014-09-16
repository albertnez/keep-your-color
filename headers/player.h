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
  const sf::Vector2f get_size();
  void set_pos(const sf::Vector2f & pos);
private:
  sf::Vector2f size;
  const static float acc;
  const static float dec;
  const static float first_move_speed;
  float act_speed;
};

#endif  // PLAYER_H
