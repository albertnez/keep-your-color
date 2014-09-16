#include "gui.h"
#include "utils.h"
#include "game.h"

Gui::Gui(Game & game) : game(game) {
  text.resize(Game::S_SIZE);
  text[Game::MENU].setString("Keep your color" 
                             "\n\nStay in the zone that shares your color."
                             "\nYou can change your color in a mixed color zone."
                             "\nTo your change color, press SPACE."
                             "\nTo move, use the arrow keys UP and DOWN."
                             "\nPress SPACE to start, ESCAPE to exit");
  text[Game::READY].setString("Get ready...");
  text[Game::PLAYING].setString("Score: 0");
  text[Game::GAME_OVER].setString("Game Over.\nPress SPACE to start again");
}

Gui::~Gui() {}

bool Gui::init() {
  status = Game::MENU;
  index = 0;
  score = 0;
  std::ifstream file("best_score.txt");
  best_score = 0;
  if (file.is_open()) {
    file >> best_score;
  }
  file.close();
  timeout = 0;
  if (!font.loadFromFile("fonts/Audiowide-Regular.ttf")) {
    std::cerr << "Error loading font fonts/NovaMono.ttf" << std::endl;
    return false;
  }
  for (sf::Text & t : text) {
    t.setFont(font);
    t.setColor(sf::Color::Black);
    t.setCharacterSize(24);
  }
  return true;
}

void Gui::render() {
  game.get_window().draw(text[status]);
  std::string s = text[status].getString();
}

void Gui::update() {
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
  if (status == Game::GAME_OVER) {
    bool new_best_score = (score > best_score);
    best_score = std::max(score, best_score);
    std::stringstream ss;
    ss << "Game Over.";
    if (new_best_score) {
      ss << "\nNew best score: " << score;
    }
    else {
      ss << "\nScore: " << score;
    }
    ss << "\nBest score: " << best_score << 
          "\nPress Space to start again";
    text[Game::GAME_OVER].setString(ss.str());
  }
}

void Gui::save_score() {
  std::ofstream file("best_score.txt");
  if (file.is_open()) {
    file << best_score;
  }
  file.close();
}
