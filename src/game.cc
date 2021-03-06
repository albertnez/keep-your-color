#include "game.h"
#include "wall.h"
#include "utils.h"
#include "player.h"
#include <iostream>

const int Game::num_types = 2;
const int Game::num_positions = 8;
const int Game::walls_max_dist = 3;
const float Game::walls_width = 4.0f;
const float Game::start_speed = 300.0f;
const float Game::ready_speed = 1000.0f;
const float Game::game_over_speed = 200.0f;
const float Game::walls_min_height = 180.0f; 
const int Game::init_one_way_probability = 20;
const float Game::min_walls_next_target_timeout = 0.5f;
const float Game::init_walls_next_target_timeout = 2.0f;

Game::Game(int width, int height, std::string title, int style)
  : window(sf::VideoMode(width, height), title, style) {
  
  window.setMouseCursorVisible(false);
  window.setVerticalSyncEnabled(true);

  sf::RenderStates(BlendMultiply);

  input = Input();
  gui = new Gui(*this);

  one_way_probability = init_one_way_probability;
  status = MENU;
  score = 0;
  total_time = 0;
  time_to_start = 0;
}

Game::~Game() {
  clear();
}

bool Game::init() {
  if (!gui->init()) return false;
  speed = target_speed = start_speed;

  all_walls = std::vector<std::list<Wall*>>(num_types);

  walls_next_target_timeout = init_walls_next_target_timeout;
  walls_next_target_timer = walls_next_target_timeout;
  walls_last_target = walls_target = walls_next_target = std::vector<int>(num_types);
  
  for (int type = 0; type < num_types; ++type) {
    walls_target[type] = walls_next_target[type] =  rand()%num_positions;
  }
  
  target_positions = std::vector<float> { 0.0f, 
                                          50.0f, 
                                          100.0f,
                                          150.0f,
                                          200.0f,
                                          250.0f,
                                          300.0f,
                                          350.0f };
                                          
  Actor::colors = {sf::Color::Red, sf::Color::Blue}; 
  player = (new Player(*this, 0, 1000.0f));

  // Assign initial status update, which is menu_update
  status_update = std::bind(&Game::menu_update, this, std::placeholders::_1);

  return true;
}

void Game::run() {
  srand(time(0));
  sf::Clock clock;
  while (window.isOpen()) {
    float delta_time = clock.restart().asSeconds();
    process_events();
    if (window.isOpen()) {
      update(delta_time);
      render();
    }
  }
}

sf::RenderWindow & Game::get_window() {
  return window;
}

const Input & Game::get_input() {
  return input;
}

void Game::update(float delta_time) {
  input.update();
  // Update speed to target
  speed += (target_speed - speed)*delta_time*10.0f;
  total_time += delta_time;

  // Update existing walls
  for (std::list<Wall*> & walls : all_walls) {
    for (Wall * wall : walls) {
      wall->set_speed(speed);
      wall->update(delta_time);
    }
  }

  // Delete old walls
  erase_old_walls();
  // Update specific for current status
  status_update(delta_time);
}

//** STATUS DEPENDENT UPDATE **
void Game::menu_update(float delta_time) {
  generate_menu_walls();

  gui->update();
  if (input.key_pressed(input.Key::PLAYER_ACTION)) {
      status = READY;
      status_update = std::bind(&Game::ready_update, this, std::placeholders::_1);
      gui->set_status(status);
      time_to_start = 3.0f;
  }
}

void Game::ready_update(float delta_time) {
  generate_ready_walls();

  if (player->get_type() != 0) player->set_type(0);
  target_speed = ready_speed;
  if (time_to_start < 2.5f) {
    target_speed = start_speed;
    one_way_probability = init_one_way_probability;
    walls_next_target_timeout = init_walls_next_target_timeout;
    for (int type = 0; type < num_types; ++type) {
      walls_target[type] = walls_next_target[type] = rand()%num_positions;
    }
  }

  // Move player to initial position
  sf::Vector2f pos = player->get_pos();
  sf::Vector2f size = player->get_size();
  player->set_pos(sf::Vector2f(pos.x,
                               pos.y + ((SCREEN_HEIGHT/2.0f-size.y/2.0) - pos.y)*delta_time*2));

  // Adjust speed
  time_to_start -= delta_time;
  gui->set_timeout(time_to_start+1);
  if (time_to_start < 0.0f) {
    status = PLAYING;
    status_update = std::bind(&Game::playing_update, this, std::placeholders::_1);
    gui->set_status(status);
  }
}

void Game::playing_update(float delta_time) {
  generate_game_walls(delta_time);

  score += delta_time*100;
  target_speed += delta_time*10.0f;
  gui->set_score(score);

  player->update(delta_time);
  if (!player_inside()) {
    status = GAME_OVER;
    gui->set_status(status);
    status_update = std::bind(&Game::game_over_update, this, std::placeholders::_1);
    target_speed = game_over_speed;
  } 
}

void Game::game_over_update(float delta_time) {
  generate_walls();
  score = 0;
  if (input.key_pressed(input.Key::PLAYER_ACTION)) {
    status = READY;
    status_update = std::bind(&Game::ready_update, this, std::placeholders::_1);
    gui->set_status(status);
    time_to_start = 3.0f;
  }
}
//** END STATUS DEPENDENT UPDATE

void Game::process_events() {
  sf::Event event;
  while (window.pollEvent(event)) {
    if (event.type == sf::Event::Closed or sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
      gui->save_score();
      window.close();
    }
  }
}

void Game::render() {
  window.clear(sf::Color::White);
  for (std::list<Wall*> walls : all_walls) {
    for (Wall * wall : walls) {
      wall->render();
    }
  }
  player->render();
  gui->render();
  window.display();
}

void Game::clear() {
  delete player;
  for (std::list<Wall*> & walls : all_walls) {
    for (Wall * wall : walls) {
        delete wall;
        wall = NULL;
    }
  }
}

void Game::generate_game_walls(float delta_time) {
  walls_next_target_timer -= delta_time;
  // Timeout to change target
  if (walls_next_target_timer < 0) {
    // Increase difficulty
    walls_next_target_timer = walls_next_target_timeout;
    walls_next_target_timeout = std::max(min_walls_next_target_timeout, walls_next_target_timeout*0.95f);
    one_way_probability = std::min(100, one_way_probability+2);

    // Update target and check if now there is only one path
    bool one_path = false;
    for (int type = 0; type < num_types; ++type) {
      walls_last_target[type] = walls_target[type];
      walls_target[type] = walls_next_target[type];
      walls_next_target[type] = rand()%num_positions;
      if (walls_last_target[type] < 0 or walls_target[type] < 0) one_path = true;
    }

    // Join now to make next target only one path
    bool join = (rand()%100 < one_way_probability);
    if (!one_path and join) {
      int pos = 0;
      for (int type = 0; type < num_types; ++type) {
        pos += abs(walls_last_target[type]);
      }
      pos /= num_types;
      for (int type = 0; type < num_types; ++type) {
        walls_target[type] = pos;
      }
      for (int type = 0; type < num_types; ++type) {
        walls_next_target[type] = -(1 + (rand()%(num_positions-1)));  // Close path in random position
      }
      // Type that will survive, assign it a random position
      int survive = rand()%num_types;
      walls_next_target[survive] = rand()%num_positions;
    }
    
    // Limit next target by walls_max_dist
    for (int type = 0; type < num_types; ++type) {
      // Check the distance even if there is a negative target
      if (walls_next_target[type] >= 0) {
        walls_next_target[type] = rand()%num_positions;
        if (std::abs(std::abs(walls_target[type])-walls_next_target[type] > walls_max_dist)) {
          if (std::abs(walls_target[type]) > walls_next_target[type]) {
            walls_next_target[type] = std::abs(walls_target[type]) - walls_max_dist;
          }
          else {
            walls_next_target[type] = std::abs(walls_target[type]) + walls_max_dist;
          }
        }
      }
    }
  }
  
  for (int type = 0; type < num_types; ++type) {
    std::list<Wall*> & walls = all_walls[type];
    if (walls.empty()) {
      walls.push_back(new Wall(*this, type, speed,
                               sf::Vector2f(SCREEN_WIDTH, 150.0f*(type+1)),
                               sf::Vector2f(walls_width, 0.0f)));
    }
    float last_x = walls.back()->get_pos().x + walls_width;
    float last_y = walls.back()->get_pos().y;
    float last_height = walls.back()->get_size().y;

    //time left
    float time_left = walls_next_target_timer;
    int target_ind = std::abs(walls_target[type]);  // Abs to send closed paths to its position

    while (last_x < SCREEN_WIDTH) {
      float factor = std::max(1.0f, 3.0f*(1-time_left));
      float new_y = last_y + (target_positions[target_ind] - last_y)*delta_time*factor;
      float new_height = last_height + (walls_min_height - last_height)*delta_time;

      if (walls_target[type] < 0) new_height = last_height + (0.0f - last_height)*delta_time;
      walls.push_back(new Wall(*this, type, speed,
                               sf::Vector2f(last_x, new_y),
                               sf::Vector2f(walls_width, new_height)));
      last_height = new_height;
      last_x = last_x + walls_width;
      last_y = new_y;
    }
  }
}

void Game::generate_ready_walls() {
  for (int type = 0; type < num_types; ++type) {
    std::list<Wall*> & walls = all_walls[type];
    float diff = walls_width;
    if (type > 0) diff *= -1;
    if (walls.empty() and type > 0) {
      continue;
    }
    if (walls.empty() and type == 0) {
      walls.push_back(new Wall(*this, type, speed,
                               sf::Vector2f(SCREEN_WIDTH, SCREEN_HEIGHT/2.0f),
                               sf::Vector2f(walls_width, 5.0f)));
    }
    float last_x = walls.back()->get_pos().x + walls_width;
    float last_y = walls.back()->get_pos().y;
    float last_height = walls.back()->get_size().y;
    while (last_x < SCREEN_WIDTH) {
      float new_y = std::max(0.0f, std::min(SCREEN_HEIGHT - walls_width, last_y-diff/2.0f));
      float new_height = last_height + diff;
      new_height = std::max(0.0f, std::min(new_height, SCREEN_HEIGHT - new_y));
      walls.push_back(new Wall(*this, type, speed,
                               sf::Vector2f(last_x, new_y),
                               sf::Vector2f(walls_width, new_height)));
      last_height = new_height;
      last_x = last_x + walls_width;
      last_y = new_y;
    }
  }
}

void Game::generate_menu_walls() {
  float size = 100.0f;
  float sin_diff = 30.0f;
  float y_offset = 100.0f;
  for (int type = 0; type < num_types; ++type) {
    std::list<Wall*> & walls = all_walls[type];
    float diff = sin_diff * sin(total_time * (1.337f * (type+1)));
    float pos_y = y_offset*(type+1) + diff;
    if (walls.empty()) {
      walls.push_back(new Wall(*this, type, speed,
                               sf::Vector2f(SCREEN_WIDTH, pos_y),
                               sf::Vector2f(walls_width, 5.0f)));
    }
    float last_x = walls.back()->get_pos().x + walls_width;
    float last_height = walls.back()->get_size().y;
    while (last_x < SCREEN_WIDTH) {
      float new_height = std::min(last_height + walls_width/2.0f, size);
      walls.push_back(new Wall(*this, type, speed,
                               sf::Vector2f(last_x, pos_y),
                               sf::Vector2f(walls_width, new_height)));
      last_height = new_height;
      last_x = last_x + walls_width;
    }
  }
}

void Game::generate_walls() {
  for (int type = 0; type < num_types; ++type) {
    std::list<Wall*> & walls = all_walls[type];
    if (!walls.empty()) {
      float last_y = walls.back()->get_pos().y;
      float last_height = walls.back()->get_size().y;
      float last_x = walls.back()->get_pos().x + walls_width;
      while (last_x < SCREEN_WIDTH) {
        float new_y = last_y + (rand()%int(walls_width))*(rand()&1 ? -1:1);
        float new_height = last_height + (rand()%int(walls_width))*(rand()&1 ? -1:1);
        new_height = std::max(walls_min_height, std::min(SCREEN_HEIGHT - new_y, new_height));
        // limit new height
        new_height = std::min(new_height, last_height + walls_width/2.0f);
        // limit new y
        new_y = std::max(0.0f, std::min(SCREEN_HEIGHT - new_height, new_y));
        walls.push_back(new Wall(*this, type, speed,
                        sf::Vector2f(last_x, new_y),
                        sf::Vector2f(walls_width, new_height)));

        last_x += walls_width;
        last_y = new_y;
        last_height = new_height;
      }
    }
    else {
      walls.push_back(new Wall(*this, type, speed,
                      sf::Vector2f(SCREEN_WIDTH, 0),
                      sf::Vector2f(walls_width, 50)));
    }
  }
}

void Game::erase_old_walls() {
  for (int type = 0; type < num_types; ++type) {
    std::list<Wall*> & walls = all_walls[type];
    bool move_next = true;
    while (!walls.empty() and move_next) { 
      float last_x = walls.front()->get_pos().x + walls_width;
      move_next = (last_x <= 0);
      if (move_next) {
        delete walls.front();
        walls.pop_front();
      }
    }
  }
}

bool Game::player_inside() {
  sf::Vector2f pos = player->get_pos(), size = player->get_size();
  std::vector<sf::Vector2f> points { pos,
                                     pos+size,
                                     sf::Vector2f(pos.x+size.x, pos.y),
                                     sf::Vector2f(pos.x, pos.y+size.y) };
  int inside_points = 0;  // Bitmask indicating wich points are inside a wall of same type
  
  std::list<Wall*> & walls = all_walls[player->get_type()];
  std::list<Wall*>::iterator wall_it = walls.begin();
  while (wall_it != walls.end() and (*wall_it)->get_pos().x <= points[1].x) {
    for (int i = 0; i < 4; ++i) {
      if (!(inside_points&(1<<i))) {
        if ((*wall_it)->contains_point(points[i])) {
          inside_points |= (1<<i);
        }
      }
    }
    ++wall_it;
  }
  return inside_points == (1<<4)-1;  // All points visited;
}
