#include "game.h"
#include "utils.h"
#include <iostream>

int main() {
  srand(time(NULL));
  Game game(SCREEN_WIDTH, SCREEN_HEIGHT, "ld30", sf::Style::Default);
  if (game.init()) {
    game.run();
  }
  return 0;
}
