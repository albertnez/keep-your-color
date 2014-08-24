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
  void set_score(int score);
  void set_status(int status);
private:
  Game & game;
  int status;
  int score;
  sf::Font font;
  std::vector<sf::Text> text;
};

#endif  // UI_H
