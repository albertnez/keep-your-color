#include "actor.h"

Actor::Actor(Game & game, int type, float speed)
  : game(game), type(type), speed(speed) {
}

Actor::~Actor() {}

int Actor::get_type() {
  return type;
}

void Actor::set_type(int type) {
  this->type = type;
}

void Actor::set_speed(float speed) {
  this->speed = speed;
}

const sf::Vector2f & Actor::get_pos() {
  return pos;
}

std::vector<sf::Color> Actor::colors;
