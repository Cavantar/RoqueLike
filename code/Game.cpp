#include <sstream>
#include "Game.h"

void Game::updateGameState()
{
  
  GameState* newGameState = gameState->update(this);
  if(newGameState != NULL)
  {
    gameState->leave(this);
    
    delete gameState;
    gameState = newGameState;
    gameState->enter(this);
  }
  
}

void Game::processEvents()
{
  sf::Event event;
  while(window.pollEvent(event))
  {
    switch(event.type)
    {
    case sf::Event::Closed:
      {
	window.close();
	break;
      }
    case sf::Event::KeyPressed:
      input.handleKeyPress(event.key.code);
      break;
    case sf::Event::KeyReleased:
      input.handleKeyRelease(event.key.code);
      break;
    }
  }
}

void Game::setWindowTitleToFps()
{
  static float cumulativeTime = 0;
  const float titleChangeTimePeriod = 1.0f;

  cumulativeTime += lastDelta;
  
  if(cumulativeTime > titleChangeTimePeriod)
  {
    std::stringstream buffer;
    buffer << "RoqueLike! Fps: " << 1.0f/lastDelta;
    window.setTitle(buffer.str());

    cumulativeTime = fmodf(cumulativeTime, titleChangeTimePeriod);
  }
}

void Game::start()
{
  Vector2i screenResolution(1280, 720);
  
  window.create(sf::VideoMode(screenResolution.x, screenResolution.y),
		"RoqueLike!");
  
  //window.setVerticalSyncEnabled(true);
  window.setFramerateLimit(120);
  
  gameState = new PlayGameState();
  gameState->enter(this);
			   
  while (window.isOpen())
  {
    
    processEvents();
    updateGameState();
    input.clearKeyStates();
    
    window.clear();
    gameState->render(this);
    window.display();

    // Time Handling
    
    lastDelta = clock.restart().asSeconds();
    
    setWindowTitleToFps();
  }
  
}

void PlayGameState::render(Game* game)
{
  levelRenderer.renderLevel(level, cameraPosition);
  
  if(!levelGenerator->isGenerationFinished())
  {
    levelGenerator->renderAdditionalData(game->window, cameraPosition, worldScale * baseTileSizeInPixels);
  }

  Player* player = level->getPlayer();
  if(player)
  {
    playerHud.render(player);
  }
  
}

void PlayGameState::enter(Game* game)
{
  levelGenerator = new SimpleLevelGenerator(150);
  
  level = levelGenerator->create();
  eventManager.registerListener(level.get());
  
  levelRenderer.setWindow(&game->window);
  levelRenderer.setTileSize(worldScale * baseTileSizeInPixels);

  playerHud.setWindow(&game->window);
  playerHud.setFont(levelRenderer.getFont());
}  

void PlayGameState::leave(Game* game)
{
  delete levelGenerator;
}

GameState* PlayGameState::update(Game* game)
{
  if(!levelGenerator->isGenerationFinished())
  {
    static float cumulativeTime = 0;
    cumulativeTime += game->lastDelta;
    const float updatePeriod = 0.1f;

    while(cumulativeTime > updatePeriod)
    {
      cumulativeTime -= updatePeriod;
      levelGenerator->generateStep();
    }
  }

  if(game->input.isKeyPressed(sf::Keyboard::E))
  {
    
    EventArgumentDataMap eventArgumentDataMap;
    
    eventArgumentDataMap["text"] = EventArgumentData("Hello World");
    eventArgumentDataMap["position"] = EventArgumentData(WorldPosition());
    
    eventManager.queueEvent("HelloThere", eventArgumentDataMap);
  }
  
  handleInput(game);

  level->registerPendingEntities(eventManager);
  level->update(game->lastDelta);
  
  Player* player = level->getPlayer();
  if(player && cameraBoundToPlayer)
  {
    cameraPosition = player->getPosition() + Vector2f(0.5f, 0.5f);
  }
  
  eventManager.collectEvents();
  eventManager.dispatchEvents();

  level->removeDeadEntities();
  
  return NULL;
}

void PlayGameState::sendPlayerEvent(PLAYER_EVENT playerEvent)
{
  EventArgumentDataMap eventArgumentDataMap;
  eventArgumentDataMap["playerEventType"] = playerEvent;
  eventManager.queueEvent("Player", eventArgumentDataMap);
}

void PlayGameState::handleInput(Game* game)
{
  
  Input& input = game->input;
  
  if(input.isKeyPressed(sf::Keyboard::Escape) || input.isKeyPressed(sf::Keyboard::Q))
  {
    game->window.close();
  }
  
  float speed = 15.0f * (1.0f / worldScale);

  // Camera Controls 
  if(input.isKeyDown(sf::Keyboard::Left)) cameraPosition.tileOffset.x  -= game->lastDelta * speed;
  if(input.isKeyDown(sf::Keyboard::Right)) cameraPosition.tileOffset.x  += game->lastDelta * speed;
  
  if(input.isKeyDown(sf::Keyboard::Up)) cameraPosition.tileOffset.y  -= game->lastDelta * speed;
  if(input.isKeyDown(sf::Keyboard::Down)) cameraPosition.tileOffset.y  += game->lastDelta * speed;
  
  if(input.isKeyDown(sf::Keyboard::Add))
  {
    worldScale *= game->lastDelta + 1.0f;
    levelRenderer.setTileSize(worldScale * baseTileSizeInPixels);
  }
  
  if(input.isKeyDown(sf::Keyboard::Subtract))
  {
    worldScale *= 1.0f - game->lastDelta;
    levelRenderer.setTileSize(worldScale * baseTileSizeInPixels);
  }
  
  if(input.isKeyPressed(sf::Keyboard::C)) cameraBoundToPlayer = !cameraBoundToPlayer;
  
  if(input.isKeyPressed(sf::Keyboard::Q)) cameraPosition.worldPosition.tileChunkPosition.z = 1;
  if(input.isKeyPressed(sf::Keyboard::E)) cameraPosition.worldPosition.tileChunkPosition.z = 0;
  
  if(input.isKeyPressed(sf::Keyboard::R) && !input.isKeyDown(sf::Keyboard::LShift))
  {
    eventManager.reset();
    
    level = levelGenerator->regenerate(time(NULL));
    eventManager.registerListener(level.get());
    levelRenderer.setTileSize(worldScale * baseTileSizeInPixels);
    
  };
  
  if(input.isKeyPressed(sf::Keyboard::R) && input.isKeyDown(sf::Keyboard::LShift))
  {
    eventManager.reset();
    
    level = levelGenerator->regenerate(time(NULL));
    levelGenerator->generate();
    levelRenderer.setTileSize(worldScale * baseTileSizeInPixels);
    
    eventManager.registerListener(level.get());
  };

  Player* player = level->getPlayer();
  if(player)
  {
    
    if(input.isKeyDown(sf::Keyboard::A)) player->handlePlayerEvent(PLAYER_MOVE_LEFT, *level.get());
    if(input.isKeyDown(sf::Keyboard::D)) player->handlePlayerEvent(PLAYER_MOVE_RIGHT, *level.get());
    
    if(input.isKeyDown(sf::Keyboard::W)) player->handlePlayerEvent(PLAYER_MOVE_UP, *level.get());
    if(input.isKeyDown(sf::Keyboard::S)) player->handlePlayerEvent(PLAYER_MOVE_DOWN, *level.get());
    
    if(input.isKeyPressed(sf::Keyboard::Up)) player->handlePlayerEvent(PLAYER_SHOOT_UP, *level.get());
    if(input.isKeyPressed(sf::Keyboard::Right)) player->handlePlayerEvent(PLAYER_SHOOT_RIGHT, *level.get());
    if(input.isKeyPressed(sf::Keyboard::Down)) player->handlePlayerEvent(PLAYER_SHOOT_DOWN, *level.get());
    if(input.isKeyPressed(sf::Keyboard::Left)) player->handlePlayerEvent(PLAYER_SHOOT_LEFT, *level.get());
  }
}



