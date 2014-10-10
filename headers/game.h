#ifndef GAME_H
#define GAME_H

#include <functional>
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
  std::function<void(float)> status_update;
  void menu_update(float delta_time);
  void ready_update(float delta_time);
  void playing_update(float delta_time);
  void game_over_update(float delta_time);
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

  // Static members
  const static int num_types;
  const static float game_over_speed;
  const static float ready_speed;
  const static float start_speed;
  const static float walls_width;
  const static float walls_min_height;
  const static int walls_max_dist;
  const static int num_positions;
  const static float init_walls_next_target_timeout;
  const static float min_walls_next_target_timeout;
  const static int init_one_way_probability;

  // speed
  float speed;
  float target_speed;
  // Walls
  float walls_next_target_timeout;
  float walls_next_target_timer;
  int max_distance;
  int one_way_probability;
  std::vector<int> walls_target;
  std::vector<int> walls_next_target;
  std::vector<int> walls_last_target;
  std::vector<float> target_positions;

  Player* player;
  std::vector<std::list<Wall*>> all_walls;

  int status;
  float time_to_start;
  float score;
  float total_time;
};

#endif  // GAME_H
