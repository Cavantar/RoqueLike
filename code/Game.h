#pragma once
#include <SFML/Graphics.hpp>

#include "Input.h"
#include "EventManager.h"

#include "Level.h"
#include "LevelRenderer.h" 
#include "LevelGenerator.h"

#ifdef UNITY_BUILD

#include "Input.cpp"
#include "EventManager.cpp"
#include "Level.cpp"
#include "LevelRenderer.cpp"
#include "LevelGenerator.cpp"

#endif

class Game;
class GameState{
 public:
  virtual ~GameState() {}
  virtual GameState* update(Game* game) = 0;
  virtual void render(Game* game) = 0;
  
  // Called When State is Entered 
  virtual void enter(Game* game) {};

  // Called When State is Left 
  virtual void leave(Game* game) {};
};

class Game {
  friend class PlayGameState;
 public:
  void start();
  
 private:
  sf::RenderWindow window;
  Input input;
  GameState* gameState;
  
  sf::Clock clock;

  // lastDelta In Seconds
  float lastDelta;

  void setWindowTitleToFps();

  void processEvents();
  void updateGameState();
  
  void render();
};

class PlayGameState : public GameState {
 public:
  GameState* update(Game* game);
  void render(Game* game);
  
  void enter(Game* game);
  void leave(Game* game);

private:
  EventManager eventManager;
  
  LevelRenderer levelRenderer;
  LevelGenerator* levelGenerator;
  LevelPtr level;
  
  // Camera Position - It's The Center Of The Viewport
  EntityPosition cameraPosition;
  bool cameraBoundToPlayer = true;
  
  float baseTileSizeInPixels = 20.0f;
  double worldScale = 1.0f;

  void sendPlayerEvent(PLAYER_EVENT playerEvent);
  void handleInput(Game* game);
};
