#ifndef INPUT_H
#define INPUT_H

#include "utils.h"

class Input {
public:
  enum Key {
    PLAYER_UP, PLAYER_DOWN, PLAYER_LEFT, PLAYER_RIGHT,
    PLAYER_ACTION,
    EXIT,
    K_SIZE
  };
  Input();
  ~Input();
  void update();
  bool key_down(int key) const;
  bool key_pressed(int key) const;
  bool key_released(int key) const;
private:
  bool key_status[K_SIZE];
  bool old_key_status[K_SIZE];
  sf::Keyboard::Key key_mapping[K_SIZE];
};

#endif  // INPUT_H
