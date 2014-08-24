#ifndef ACTOR_H
#define ACTOR_H

#include "utils.h"

// Forward declaration
class Game;

class Actor {
 public:
  Actor(Game & game, int type, float speed);
  virtual ~Actor();
  virtual void update(float delta_time) = 0;
  virtual void render() = 0;
  int get_type();
  void set_type(int type);
  void set_speed(float speed);
  const sf::Vector2f & get_pos();
  static std::vector<sf::Color> colors;
 protected:
  Game & game;
  int type;
  float speed;
  sf::Vector2f pos;

};

#endif  // ACTOR_H
