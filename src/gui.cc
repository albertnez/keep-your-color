#include "gui.h"
#include "utils.h"
#include "game.h"

Gui::Gui(Game & game) : game(game) {
  text.resize(Game::S_SIZE);
  text[Game::MENU].setString("Press SPACE to start");
  text[Game::READY].setString("Get ready...");
  text[Game::PLAYING].setString("Score: 0");
  text[Game::GAME_OVER].setString("Game Over.\nPress SPACE to start again");
}

Gui::~Gui() {}

bool Gui::init() {
  status = Game::MENU;
  index = 0;
  score = -1;
  timeout = 0;
  if (!font.loadFromFile("fonts/Audiowide-Regular.ttf")) {
    std::cerr << "Error loading font fonts/NovaMono.ttf" << std::endl;
    return false;
  }
  for (sf::Text & t : text) {
    t.setFont(font);
    t.setColor(sf::Color::Black);
  }
  return true;
}

void Gui::render() {
  game.get_window().draw(text[status]);
  std::string s = text[status].getString();
}

void Gui::update() {
  const Input & input = game.get_input();
}

void Gui::set_score(int score) {
  if (this->score != score) {
    this->score = score;
    std::stringstream ss;
    ss << "Score: " << score;
    text[Game::PLAYING].setString(ss.str()); 
  }
}

void Gui::set_timeout(int timeout) {
  if (timeout != this->timeout) {
    this->timeout = timeout;
    std::stringstream ss;
    ss << "Game starts in: " << timeout;
    text[Game::READY].setString(ss.str());
  }
}

void Gui::set_status(int status) {
  index = 0;
  this->status = status;
  if (status == Game::PLAYING) set_score(0);
  if (status == Game::READY) set_timeout(3);
}

