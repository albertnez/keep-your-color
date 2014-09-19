#include "game.h"
#include "utils.h"
#include "player.h"
#include "wall.h"
#include <iostream>

const int Game::num_types = 2;
const float Game::game_over_speed = 200.0f;
const float Game::ready_speed = 1000.0f;
const float Game::start_speed = 300.0f;
const float Game::walls_width = 4.0f;
const float Game::walls_min_height = 180.0f; 
const int Game::num_positions = 8;
const float Game::init_walls_next_target_timeout = 2.0f;
const int Game::init_one_way_probability = 20;  // 30 percent of probability of only one way

Game::Game(int width, int height, std::string title, int style)
  : window(sf::VideoMode(width, height), title, style) {
  window.setMouseCursorVisible(false);
  window.setVerticalSyncEnabled(true);

  input = Input();
  gui = new Gui(*this);

  one_way_probability = init_one_way_probability;
  status = MENU;
  time_to_start = 0;
  score = 0;
  total_time = 0;
}

Game::~Game() {
  clear();
}

bool Game::init() {
  if (!gui->init()) return false;
  target_speed = start_speed;
  speed = start_speed;

  all_walls = std::vector<std::list<Wall*>>(num_types);

  walls_next_target_timeout = init_walls_next_target_timeout;
  walls_next_target_timer = walls_next_target_timeout;
  max_distance = num_positions/2;
  walls_last_target = walls_target = walls_next_target = std::vector<int>(num_types);
  for (int type = 0; type < num_types; ++type) {
    walls_target[type] = rand()%num_positions;
    walls_next_target[type] = rand()%num_positions;
    if (std::abs(walls_next_target[type] - walls_target[type]) > max_distance) {
      walls_next_target[type] = (walls_target[type] - walls_next_target[type])/2;
    }
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

  return true;
}

void Game::run() {
  sf::Clock clock;
  srand(time(0));
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
  // Common update
  input.update();
  // Update speed to target
  speed += (target_speed - speed)*delta_time*10.0f;

  total_time += delta_time;

  for (std::list<Wall*> & walls : all_walls) {
    for (Wall * wall : walls) {
      wall->set_speed(speed);
      wall->update(delta_time);
    }
  }

  // Walls
  erase_old_walls();
  if (status == PLAYING) generate_game_walls(delta_time);
  else if (status == MENU) generate_menu_walls();
  else if (status == READY) generate_ready_walls();
  else generate_walls();

  // Playing update
  if (status == PLAYING) { 
    score += delta_time*100;
    target_speed += delta_time*10.0f;
    gui->set_score(score);

    player->update(delta_time);
    if (!player_inside()) {
      status = GAME_OVER;
      gui->set_status(status);
      target_speed = game_over_speed;
    } 
  }
  else if (status == MENU) {
    gui->update();
    if (input.key_pressed(input.Key::PLAYER_ACTION)) {
        status = READY;
        gui->set_status(status);
        time_to_start = 3.0f;
    }
  }
  else if (status == READY) {
    if (player->get_type() != 0) player->set_type(0);
    target_speed = ready_speed;
    if (time_to_start < 2.5f) {
      target_speed = start_speed;
      one_way_probability = init_one_way_probability;
      walls_next_target_timeout = init_walls_next_target_timeout;
      for (int type = 0; type < num_types; ++type) {
        walls_target[type] = rand()%num_positions;
        walls_next_target[type] = rand()%num_positions;
        if (std::abs(walls_next_target[type] - walls_target[type]) > max_distance) {
          walls_next_target[type] = (walls_target[type] - walls_next_target[type])/2;
        }
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
      gui->set_status(status);
    }
  }
  else if (status == GAME_OVER) {
    score = 0;
    if (input.key_pressed(input.Key::PLAYER_ACTION)) {
      status = READY;
      gui->set_status(status);
      time_to_start = 3.0f;
    }
  }
}

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
  if (walls_next_target_timer < 0) {
    walls_next_target_timer = walls_next_target_timeout;
    walls_next_target_timeout = std::max(0.5f, walls_next_target_timeout*0.95f);
    one_way_probability = std::min(100, one_way_probability+2);
    bool one_path = false;
    for (int type = 0; type < num_types; ++type) {
      walls_last_target[type] = walls_target[type];
      walls_target[type] = walls_next_target[type];
      if (walls_target[type] == -1) one_path = true;
    }
    bool join = (rand()%100 < one_way_probability);
    // Only join if there is no one_path in process
    if (!one_path and join) {
      int pos = rand()%num_positions;
      for (int type = 0; type < num_types; ++type) {
        walls_target[type] = pos;
      }
      for (int type = 0; type < num_types; ++type) {
        walls_next_target[type] = -1;  // Eliminate
      }
      // Type to survive
      int survive = rand()%num_types;
      walls_next_target[survive] = rand()%num_positions;
    }
    else {
      for (int type = 0; type < num_types; ++type) {
        walls_next_target[type] = rand()%num_positions;
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
    while (last_x < SCREEN_WIDTH) {
      //int ind = walls_target[type];
      int target_ind = std::max(0, walls_target[type]);
      float new_y = last_y + (target_positions[target_ind] - last_y)*delta_time;
    //float new_y = last_y + (target_positions[target_ind] - target_positions[last_ind])*delta_time;
      float new_height = last_height + (walls_min_height - last_height)*delta_time;
      //float new_height = last_height + (walls_target[type] - walls_last_target[type])*delta_time;
      if (walls_target[type] == -1) new_height = last_height + (0.0f - last_height)*delta_time;
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
  float y_offset = 100.0f;
  float sin_diff = 30.0f;
  float size = 100.0f;
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
      float last_x = walls.back()->get_pos().x + walls_width;
      float last_y = walls.back()->get_pos().y;
      float last_height = walls.back()->get_size().y;
      while (last_x < SCREEN_WIDTH) {
        float new_y = last_y + (rand()%int(walls_width))*(rand()&1 ? -1:1);
        float new_height = last_height + (rand()%int(walls_width))*(rand()&1 ? -1:1);
        new_height = std::max(walls_min_height, std::min(SCREEN_HEIGHT - new_y, new_height));
        new_height = std::min(new_height, last_height + walls_width/2.0f);
        // limit new height
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
  //for (std::list<Wall*> walls : all_walls) {
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
