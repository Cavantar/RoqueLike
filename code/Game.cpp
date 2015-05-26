#include <sstream>
#include "Game.h"

//PlayGameState Game::playGameState;

void
Game::updateGameState()
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

void
Game::processEvents()
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

void
Game::setWindowTitleToFps()
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

void
Game::start()
{
  Vector2i screenResolution(1280, 720);
  
  window.create(sf::VideoMode(screenResolution.x, screenResolution.y),
		"RoqueLike!");
  
  window.setVerticalSyncEnabled(true);
  //window.setFramerateLimit(120);
  
  //gameState = &playGameState;
  gameState = new PlayGameState();
  gameState->enter(this);
			   
  while (window.isOpen())
  {
    SfmlProfiler::get()->startFrame();
    {
      SfmlProfiler::get()->start("Update");
      {
	processEvents();
	updateGameState();
      }
      SfmlProfiler::get()->end("Update");
      
      SfmlProfiler::get()->start("Render");
      {
	window.clear();
	gameState->render(this);
	SfmlProfiler::get()->start("BufferFlip");
	window.display();
	SfmlProfiler::get()->end("BufferFlip");
      }      
      SfmlProfiler::get()->end("Render");
      // Time Handling
      lastDelta = clock.restart().asSeconds();
      setWindowTitleToFps();
    }
    SfmlProfiler::get()->endFrame();
    
    if(input.isKeyPressed(sf::Keyboard::P)) SfmlProfiler::get()->showData();
    input.clearKeyStates();
  }
  gameState->leave(this);
}

void
PlayGameState::render(Game* game)
{
  SfmlProfiler::get()->start("LevelRender");
  levelRenderer.renderLevel(level, cameraPosition);
  SfmlProfiler::get()->end("LevelRender");
  
  if(!levelGenerator->isGenerationFinished())
  {
    levelGenerator->renderAdditionalData(game->window, cameraPosition, worldScale * baseTileSizeInPixels);
  }

  Player* player = level->getPlayer();
  if(player)
  {
    playerHud.render(player);
  }

  sf::Sprite tempSprite = spriteManager.getSprite("playerBase");
  float tempScale = 8.0f;
  tempSprite.setScale(tempScale, tempScale);
  game->window.draw(tempSprite);
}

void
PlayGameState::enter(Game* game)
{
  levelGenerator = new SimpleLevelGenerator(150);
  
  level = levelGenerator->create();
  eventManager.registerListener(level.get());
  
  levelRenderer.setWindow(&game->window);
  levelRenderer.setTileSize(worldScale * baseTileSizeInPixels);
  
  playerHud.setWindow(&game->window);
  playerHud.setFont(levelRenderer.getFont());
  
  spriteManager.loadTexture("myTileset.png");
    
  spriteManager.loadSpriteSet("floor1_", IntRect(0, 0, 16, 16), 30);
  spriteManager.loadSpriteSet("floor1_", IntRect(0, 16, 16, 16), 30, 30);
  spriteManager.loadSpriteSet("floor1_", IntRect(0, 16 * 2, 16, 16), 9, 60);
  
  spriteManager.loadSpriteSet("wallTop1_", IntRect(0, 16 * 3, 16, 16), 30);
  spriteManager.loadSpriteSet("wallTop1_", IntRect(0, 16 * 4, 16, 16), 29, 30);
  
  spriteManager.loadSpriteSet("wall1_", IntRect(0, 16 * 5, 16, 32), 7);
  
  spriteManager.loadSprite("first", IntRect(16, 16 * 4, 16, 16));
  spriteManager.loadSprite("healthPotion", IntRect(0, 16 * 9, 16, 16));
  spriteManager.loadSprite("playerBase", IntRect(0, 16 * 10, 16, 32));
  spriteManager.loadSprite("followerBase", IntRect(16, 16 * 10, 16, 32));
  spriteManager.loadSprite("cannonBase", IntRect(0, 16 * 12, 16, 16));
  spriteManager.loadSprite("ratBase", IntRect(16, 16 * 12, 16, 16));
  spriteManager.loadSprite("snakeBase", IntRect(16 * 2, 16 * 12, 16, 16));
  
  spriteManager.loadSpriteSet("goblin_", IntRect(16 * 2, 16 * 13, 16, 16), 4);
    
  levelRenderer.setSpriteManager(&spriteManager);
}  

void
PlayGameState::leave(Game* game)
{
  delete levelGenerator;
}

GameState*
PlayGameState::update(Game* game)
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
    
    eventArgumentDataMap["text"] = std::string("Hello World");
    eventArgumentDataMap["position"] = WorldPosition();
    eventArgumentDataMap["number"] = 15.0f;
    
    std::cout << "From Soruce: " << eventArgumentDataMap["text"].asString() << std::endl;
    
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

void
PlayGameState::handleInput(Game* game)
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
  
  if(input.isKeyPressed(sf::Keyboard::R))
  {
    if(input.isKeyDown(sf::Keyboard::LShift) || input.isKeyDown(sf::Keyboard::LAlt))
    {
      int seed = 0;
      if(input.isKeyDown(sf::Keyboard::LShift)) seed = (int)time(NULL);
      eventManager.reset();
      
      level = levelGenerator->regenerate(seed);
      levelGenerator->generate();
      levelRenderer.setTileSize(worldScale * baseTileSizeInPixels);
      
      eventManager.registerListener(level.get());
    }
    else
    {
      eventManager.reset();

      int seed = time(NULL);
      if(input.isKeyDown(sf::Keyboard::LControl)) seed = 0;
      level = levelGenerator->regenerate(seed);
      levelRenderer.setTileSize(worldScale * baseTileSizeInPixels);
    
      eventManager.registerListener(level.get());
    }
    
  };
  
  Player* player = level->getPlayer();
  if(player)
  {
    PlayerInput playerInput = {};
    
    if(input.isKeyDown(sf::Keyboard::W)) playerInput.up = true;
    if(input.isKeyDown(sf::Keyboard::D)) playerInput.right = true;
    if(input.isKeyDown(sf::Keyboard::S)) playerInput.down = true;
    if(input.isKeyDown(sf::Keyboard::A)) playerInput.left = true;
    
    if(input.isKeyPressed(sf::Keyboard::Up)) playerInput.actionUp = true;
    if(input.isKeyPressed(sf::Keyboard::Right)) playerInput.actionRight = true;
    if(input.isKeyPressed(sf::Keyboard::Down)) playerInput.actionDown = true;
    if(input.isKeyPressed(sf::Keyboard::Left)) playerInput.actionLeft = true;
    
    if(input.isKeyPressed(sf::Keyboard::Num1)) playerInput.playerKey1 = true;
    if(input.isKeyPressed(sf::Keyboard::Num2)) playerInput.playerKey2 = true;
    if(input.isKeyPressed(sf::Keyboard::Num3)) playerInput.playerKey3 = true;
    if(input.isKeyPressed(sf::Keyboard::Num4)) playerInput.playerKey4 = true;
    if(input.isKeyPressed(sf::Keyboard::Num5)) playerInput.playerKey5 = true;
    if(input.isKeyPressed(sf::Keyboard::Num6)) playerInput.playerKey6 = true;
    
    player->handlePlayerInput(playerInput);
  }
}



