#include "gui.h"
#include "utils.h"
#include "game.h"

Gui::Gui(Game & game) : game(game) {
  text.resize(Game::S_SIZE);
  text[Game::MENU].setString("Press SPACE to start");
  text[Game::PLAYING].setString("Score: 0");
  text[Game::GAME_OVER].setString("Game Over.\nPress SPACE to start again");
}

Gui::~Gui() {}

bool Gui::init() {
  status = Game::MENU;
  if (!font.loadFromFile("fonts/NovaMono.ttf")) return false;
  for (sf::Text & t : text) {
    t.setFont(font);
    t.setColor(sf::Color::Black);
  }
  return true;
}

void Gui::render() {
  game.get_window().draw(text[status]);
}

void Gui::set_score(int score) {
  score = score;
  std::stringstream ss;
  ss << "Score: " << score;
  text[Game::PLAYING].setString(ss.str()); 
}

void Gui::set_status(int status) {
  this->status = status;
  if (status == Game::PLAYING) set_score(0);
}

