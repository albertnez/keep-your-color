#ifndef WALL_H
#define WALL_H

#include "utils.h"
#include "actor.h"

class Wall : public Actor {
public:
  Wall(Game & game, int type, float speed,  const sf::Vector2f & pos, const sf::Vector2f & size);
  ~Wall();
  void update(float delta_time);
  void render();
  const sf::Vector2f & get_size();
  bool contains_point(const sf::Vector2f & p);
private:
  sf::Vector2f size;
};

#endif  // WALL_H
