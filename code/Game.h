#pragma once
#include <SFML/Graphics.hpp>

#include "Input.h"
#include "EventManager.h"
#include "Level.h"
#include "PlayerHud.h"
#include "LevelRenderer.h" 
#include "LevelGenerator.h"
#include "SpriteManager.h"
#include "Profiler.h"

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
  SpriteManager spriteManager;
  LevelRenderer levelRenderer;
  LevelGenerator* levelGenerator;
  LevelPtr level;
  PlayerHud playerHud;
  
  // Camera Position - It's The Center Of The Viewport
  EntityPosition cameraPosition;
  bool cameraBoundToPlayer = true;
  
  float baseTileSizeInPixels = 20.0f;
  double worldScale = 2.0f;
  
  void handleInput(Game* game);
};
