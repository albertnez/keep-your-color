#ifndef UI_H
#define UI_H

#include "utils.h"

class Game;

class Gui {
public:
  Gui(Game & game);
  ~Gui();
  bool init();
  void render();
  void update();
  void set_score(int score);
  void set_timeout(int timeout);
  void set_status(int status);
private:
  Game & game;
  int status;
  int index;
  int score;
  int timeout;
  sf::Font font;
  std::vector<sf::Text> text;
};

#endif  // UI_H
