#ifndef GAME_H
#define GAME_H

#include "utils.h"
#include "input.h"
#include "actor.h"
#include "player.h"
#include "wall.h"

class Game {
public:
  Game(int width, int height, std::string title, int style);
  ~Game();
  bool init();
  void run();
  sf::RenderWindow & get_window();
  const Input & get_input();
private:
  void update(float delta_time);
  void process_events();
  void render();
  void generate_walls();
  void erase_old_walls();
  sf::RenderWindow window;
  Input input;
  int num_types;
  float speed;
  float walls_width;
  int num_total_walls;
  std::vector<int> num_active_walls;
  // Matrix of walls. all_walls[type][offset]
  Player* player;
  std::vector<std::list<Wall*>> all_walls;
};

#endif  // GAME_H