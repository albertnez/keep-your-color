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
  delete player;
  for (std::list<Wall*> & walls : all_walls) {
    for (Wall * wall : walls) {
        delete wall;
    }
  }
}

bool Game::init() {
  // Default color scheeme for actors
  std::cerr << "start init " << std::endl;
  input = Input();
  num_types = 2;
  speed = 500.0f;
  walls_width = 20.0f;
  num_total_walls = 0;

  num_active_walls = std::vector<int>(num_types, 0);

  Actor::colors = {sf::Color::Red, sf::Color::Blue}; 

  player = (new Player(*this, 0, speed));
  player->set_speed(500);
  // Create Walls test
  
  all_walls.resize(num_types);
  float pos_x = 0.0f;
  while (pos_x + walls_width < SCREEN_WIDTH) {
    all_walls[0].push_back(new Wall(*this, 0, speed, sf::Vector2f(pos_x, 0.0f),
                           sf::Vector2f(walls_width, SCREEN_HEIGHT)));
    pos_x += walls_width;
    ++num_total_walls;
    ++num_active_walls[0];
  }
  std::cerr << "endl init " << std::endl;
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
  input.update();
  player->update(delta_time);
  for (std::list<Wall*> & walls : all_walls) {
    for (Wall * wall : walls) {
      wall->update(delta_time);
    }
  }
  erase_old_walls();
  generate_walls();
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
  window.clear(sf::Color::Black);
  for (std::list<Wall*> walls : all_walls) {
    for (Wall * wall : walls) {
      wall->render();
    }
  }
  player->render();
  window.display();
}

void Game::generate_walls() {
  //std::cerr << "init gen" << std::endl;
  for (int type = 0; type < num_types; ++type) {
    std::list<Wall*> & walls = all_walls[type];
    if (!walls.empty()) {
      float last_x = walls.back()->get_pos().x + walls_width;
      float last_y = walls.back()->get_pos().y;
      float last_height = walls.back()->get_size().y;
      while (last_x < SCREEN_WIDTH) {
        float new_y = last_y + (rand()%20)*(rand()&1 ? -1:1);
        float new_height = last_height + (rand()%10)*(rand()&1 ? -1:1);
        // limit new height
        new_height = std::max(50.0f, std::min(float(SCREEN_HEIGHT), new_height));
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
                      sf::Vector2f(SCREEN_WIDTH, 0),
                      sf::Vector2f(walls_width, 50)));
      ++num_total_walls;
      ++num_active_walls[type];
    }
  }
  //std::cerr << "end gen" << std::endl;
}

void Game::erase_old_walls() {
  //for (std::list<Wall*> walls : all_walls) {
  for (std::list<Wall*> & walls : all_walls) {
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
