#include "player.h"
#include "game.h"
#include <iostream>
#include "utils.h"
#include "input.h"

const float Player::acc = 700.0f;
const float Player::dec = 3000.0f;
const float Player::first_move_speed = 50.0f;

Player::Player(Game & game, int type, float speed) : Actor(game, type, speed) {
  size = sf::Vector2f(20, 20);
  act_speed = 0.0f;
  pos.x = 50;
  pos.y = SCREEN_HEIGHT/2.0f - size.y/2.0f;
}

Player::~Player() {}

void Player::update(float delta_time) {
  const Input & input = game.get_input();
  if (input.key_down(input.Key::PLAYER_DOWN) ^ input.key_down(input.Key::PLAYER_UP)) {
    float acceleration = acc;
    if (input.key_down(input.Key::PLAYER_UP)) {
      acceleration *= -1;
      if (std::abs(act_speed) < EPSILON) act_speed = -1*first_move_speed;
    }
    else {
      if (std::abs(act_speed) < EPSILON) act_speed = first_move_speed;
    }
    act_speed += ((acceleration < 0 ? -1 : 1)*speed*0.5f + acceleration) * delta_time;
    act_speed = std::min(speed, std::max(-speed, act_speed));
  }
  if ((input.key_down(input.Key::PLAYER_DOWN) and act_speed < -EPSILON) or 
      (input.key_down(input.Key::PLAYER_UP) and act_speed > EPSILON) or
      !(input.key_down(input.Key::PLAYER_DOWN) ^ input.key_down(input.Key::PLAYER_UP))) {
    if (std::abs(act_speed) > EPSILON) {
      float act_dec = dec;
      if (act_speed > EPSILON) act_dec *= -1;

      float new_speed = act_speed + act_dec * delta_time;
      if (act_speed * new_speed < -EPSILON) new_speed = 0.0f;
      act_speed = new_speed;
    }
  }
  pos.y = std::min(SCREEN_HEIGHT - size.y, std::max(0.0f, pos.y + act_speed * delta_time));

  if (input.key_pressed(input.Key::PLAYER_ACTION)) {
    type = 1-type;
  }
}

void Player::render() {
  sf::RectangleShape rectangle(size);
  rectangle.setPosition(pos);
  sf::Color color = colors[type];
  rectangle.setFillColor(color);
  game.get_window().draw(rectangle);
}

const sf::Vector2f Player::get_size() {
  return size;
}

void Player::set_pos(const sf::Vector2f & pos) {
  this->pos = pos;
}
