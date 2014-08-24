#include "input.h"

Input::Input() {
  key_mapping[PLAYER_UP] = sf::Keyboard::Up;
  key_mapping[PLAYER_DOWN] = sf::Keyboard::Down;
  key_mapping[PLAYER_ACTION] = sf::Keyboard::Space;
  key_mapping[EXIT] = sf::Keyboard::Escape;
}

Input::~Input() {}

void Input::update() {
  for (int k = 0; k < K_SIZE; ++k) {
    old_key_status[k] = key_status[k];
    if (k == Key::PLAYER_ACTION) std::cerr << "Status: " << old_key_status[k] << std::endl;
  }
  for (int k = 0; k < K_SIZE; ++k) {
    key_status[k] = sf::Keyboard::isKeyPressed(key_mapping[k]);
  }
}

bool Input::key_down(int key) const {
  return key_status[key];
}

bool Input::key_pressed(int key) const {
  return key_status[key] and !old_key_status[key];
}

bool Input::key_released(int key) const {
  return !key_status[key] and old_key_status[key];
}
