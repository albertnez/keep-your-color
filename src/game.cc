#include "game.h"
#include "utils.h"
#include "player.h"
#include "wall.h"
#include <iostream>

Game::Game(int width, int height, std::string title, int style)
  : window(sf::VideoMode(width, height), title, style) {
  window.setMouseCursorVisible(false);
  window.setVerticalSyncEnabled(true);
}

Game::~Game() {
  clear();
}

bool Game::init() {
  // Default color scheeme for actors
  input = Input();
  gui = new Gui(*this);
  if (!gui->init()) return false;
  num_types = 2;
  speed = 500.0f;
  walls_width = 4.0f;
  walls_min_height = 85.0f;
  num_total_walls = 0;

  num_active_walls = std::vector<int>(num_types, 0);

  Actor::colors = {sf::Color::Red, sf::Color::Blue}; 

  player = (new Player(*this, 0, speed));
  player->set_speed(500);
  // Create Walls test
  
  all_walls = std::vector<std::list<Wall*>>(num_types);
  float pos_x = 0.0f;
  while (pos_x + walls_width < SCREEN_WIDTH) {
    all_walls[0].push_back(new Wall(*this, 0, speed, sf::Vector2f(pos_x, 0.0f),
                           sf::Vector2f(walls_width, SCREEN_HEIGHT)));
    pos_x += walls_width;
    ++num_total_walls;
    ++num_active_walls[0];
  }
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
  for (std::list<Wall*> & walls : all_walls) {
    for (Wall * wall : walls) {
      wall->update(delta_time);
    }
  }
  erase_old_walls();
  generate_walls();
  // Playing update
  if (status == PLAYING) { 
    score += delta_time;
    gui->set_score(score);

    player->update(delta_time);
    if (!player_inside()) {
      status = GAME_OVER;
      gui->set_status(status);
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
    sf::Vector2f pos = player->get_pos();
    sf::Vector2f size = player->get_size();
    player->set_pos(sf::Vector2f(pos.x,
                                 pos.y + ((SCREEN_HEIGHT/2.0f-size.y/2.0) - pos.y)*delta_time*2));

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
    if (event.type == sf::Event::Closed) {
      window.close();
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
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
        // limit new height
        new_height = std::max(walls_min_height, std::min(SCREEN_HEIGHT - new_y, new_height));
        // limit new y
        new_y = std::max(0.0f, std::min(SCREEN_HEIGHT - new_height, new_height));
        walls.push_back(new Wall(*this, type, speed,
                        sf::Vector2f(last_x, new_y),
                        sf::Vector2f(walls_width, new_height)));

        last_x += walls_width;
        last_y = new_y;
        last_height = new_height;

        ++num_total_walls;
        ++num_active_walls[type];
      }
    }
    else {
      walls.push_back(new Wall(*this, type, speed,
                      sf::Vector2f(SCREEN_WIDTH + 20, 0),
                      sf::Vector2f(walls_width, 50)));
      ++num_total_walls;
      ++num_active_walls[type];
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
        --num_total_walls;
        --num_active_walls[type];
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
