#ifndef GAME_H
#define GAME_H

#include "utils.h"
#include "input.h"
#include "actor.h"
#include "player.h"
#include "wall.h"
#include "gui.h"

class Game {
public:
  Game(int width, int height, std::string title, int style);
  ~Game();
  bool init();
  void run();
  sf::RenderWindow & get_window();
  const Input & get_input();
  enum Status { MENU, READY, PLAYING, GAME_OVER, S_SIZE };
private:
  void update(float delta_time);
  void process_events();
  void render();
  // Clear everything in game
  void clear();
  // Different kinds of generation
  void generate_game_walls(float delta_time);
  void generate_ready_walls();
  void generate_menu_walls();
  void generate_walls();
  void erase_old_walls();
  // Check if player is inside a wall of its type
  bool player_inside();
  sf::RenderWindow window;
  Input input;
  Gui * gui;
  int num_types;
  // speed
  float start_speed;
  float speed;
  float target_speed;
  float game_over_speed;
  float ready_speed;
  // Walls
  float walls_width;
  float walls_min_height;
  int num_total_walls;
  std::vector<int> num_active_walls;

  float walls_move_timeout;
  float walls_move_timer;
  int num_positions;
  std::vector<int> walls_target;
  std::vector<int> walls_next_target;
  std::vector<float> target_positions;

  Player* player;
  std::vector<std::list<Wall*>> all_walls;

  int status;
  float time_to_start;
  float score;
  float total_time;
};

#endif  // GAME_H
