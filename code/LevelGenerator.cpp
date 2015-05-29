#include "LevelGenerator.h"
#include <iostream>

bool
Room::isColliding(TileMapPtr tileMap)
{
  bool result;
  
  result = !tileMap->isRectangleOfTileType(topLeftCorner + Vector2i(1, 1),
					  dimensions - Vector2i(2, 2),
					  TILE_TYPE_VOID);
  
  return result;
}

LevelPtr
LevelGenerator::create(int seed)
{
  if(seed == 0)
  {
    seed = time(NULL);
  }

  this->seed = seed;
  startSeed = seed;
  
  level = LevelPtr(new Level());
  return level;
}

void
LevelGenerator::placeLine(WorldPosition startPosition, Vector2i deltaVector, TILE_TYPE tileType)
{
  // TODO Implement Other Octants

  Vector2i currentPosition;

  float slope = (float)deltaVector.y / (float)deltaVector.x;
  float slopeAcc = 0;

  TileMapPtr tileMap = level->getTileMap();
  
  if(slope <= 1.0f)
  {
    while(currentPosition.x != deltaVector.x)
    {
      slopeAcc += slope;

      while(slopeAcc > 1.0f)
      {
	slopeAcc -= 1.0f;
	++currentPosition.y;
      }
      tileMap->setTileType(startPosition + currentPosition, tileType);

      ++currentPosition.x;
    }
  }
  else
  {
    slope = 1.0f/slope;
    while(currentPosition.y != deltaVector.y)
    {
      slopeAcc += slope;
      
      while(slopeAcc > 1.0f)
      {
	slopeAcc -= 1.0f;
	++currentPosition.x;
      }
      tileMap->setTileType(startPosition + currentPosition, tileType);

      ++currentPosition.y;
    }
  }

}

void
LevelGenerator::fillRectangle(WorldPosition startPosition, Vector2i dimensions, TILE_TYPE tileType)
{
  TileMapPtr tileMap = level->getTileMap();
  
  for(int y = 0; y < dimensions.y; y++)
  {
    for(int x = 0; x < dimensions.x; x++)
    {
      tileMap->setTileType(startPosition + Vector2i(x, y), tileType);
    }
  }
}

void
SimpleLevelGenerator::placeRoom(const Room& room)
{
  // Horizontal Walls
  placeLine(room.topLeftCorner, Vector2i(room.dimensions.x, 0), TILE_TYPE_WALL );
  placeLine(room.topLeftCorner + Vector2i(0, room.dimensions.y - 1), Vector2i(room.dimensions.x, 0), TILE_TYPE_WALL);
  
  // Vertical Walls
  placeLine(room.topLeftCorner, Vector2i(0, room.dimensions.y), TILE_TYPE_WALL);
  placeLine(room.topLeftCorner + Vector2i(room.dimensions.x - 1, 0), Vector2i(0, room.dimensions.y), TILE_TYPE_WALL);
  
  // Placing Floor
  fillRectangle(room.topLeftCorner + Vector2i(1, 1),
		Vector2i(room.dimensions.x - 2, room.dimensions.y - 2),
		room.floorType);
}

void
SimpleLevelGenerator::placeRoomEntities(const Room& room, bool immediateMode)
{

  if(room.depth == 0) return;
  
  WorldPosition entityPosition;
  if(immediateMode)
  {
    
    // Health Pack
    if((rand()%11) < 3)
    {
      entityPosition = room.topLeftCorner + Vector2i(1, 1);
      entityPosition += Vector2i(rand()%(room.dimensions.x-2), rand()%(room.dimensions.y-2));
      Entity* entity = new HealthItem(EntityPosition(entityPosition), (float)room.depth / 15);
      level->addEntity(EntityPtr(entity));
    }
    
  }
  else {
    
    float roomDifficulty = (float)room.depth / (float)maxRoomDepth;

    WorldPosition topLeftFloor = room.topLeftCorner + Vector2i(1, 1);
    Vector2i dimensions = room.dimensions - Vector2i(2, 2);
    
    
    for(int y = 0; y < dimensions.y; y++)
    {
      for(int x = 0; x < dimensions.x; x++)
      {
	Entity* entity = NULL;
	EntityPosition entityPosition = EntityPosition(room.topLeftCorner + Vector2i(x, y));
	int mobLevel = (roomDifficulty * 20.0f) + 1;
	
	if(roomDifficulty < 0.2f)
	{
	  // Spawn Any Entity
	  if(rand()%100 < 1)
	  {
	    if(rand()%10 || roomDifficulty < 0.1f)
	    {
	      
	      if(rand()%5)
		entity = new Rat(entityPosition, mobLevel);
	      else
		entity = new HealthItem(entityPosition, roomDifficulty * 20.0f + 0.01f);
	    }
	    else
	    {
	      entity = new Snake(entityPosition, mobLevel);
	    }
	  }
	  else if(rand()%1000 < 4)
	  {
	    entity = new MobSpawner(entityPosition, mobLevel, MT_RAT);
	  }
	}
	else if(roomDifficulty < 0.4f)
	{
	  if(rand()%100 < 1)
	  {
	    if(rand()%4)
	    {
	      if(rand()%3)
		entity = new Follower(entityPosition, mobLevel);
	      else
		entity = new HealthItem(entityPosition, roomDifficulty * 20.0f + 0.01f);
	    }
	    else
	    {
	      if(rand()%3)
		entity = new Cannon(entityPosition, mobLevel);
	      else
		entity = new Snake(entityPosition, mobLevel);
	    }
	    
	  }
	}
	
	if(entity) level->addEntity(EntityPtr(entity));
      }
    }
  }
}

void
SimpleLevelGenerator::placeRemainingEntities()
{
  
  for(auto roomIt = rooms.begin(); roomIt != rooms.end(); roomIt++)
  {
    placeRoomEntities(*roomIt, false);
  }
  
}

int
SimpleLevelGenerator::getMaxRoomDepth() const
{
  int maxDepth = rooms.begin()->depth;
  for(auto roomIt = rooms.begin(); roomIt != rooms.end(); roomIt++)
  {
    if(roomIt->depth > maxDepth) maxDepth = roomIt->depth;
  }
  return maxDepth;
}

void
SimpleLevelGenerator::placeCorridor(const Room& srcRoom, const Room& dstRoom, const DIRECTION direction)
{
  TileMapPtr tileMap = level->getTileMap();
  
  if(direction == DIRECTION_UP)
  {
    int possibleCorridorPlacements = std::min(srcRoom.dimensions.x, dstRoom.dimensions.x);
    
    // I subtract the borders
    possibleCorridorPlacements -= 2;
    int horizontalOffset = (rand()%possibleCorridorPlacements) + 1;
    
    WorldPosition corridorPosition = srcRoom.topLeftCorner + Vector2i(horizontalOffset, 0);
    
    tileMap->setTileType(corridorPosition, srcRoom.floorType);
  }
  else if(direction == DIRECTION_RIGHT)
  {
    int possibleCorridorPlacements = std::min(srcRoom.dimensions.y, dstRoom.dimensions.y);

    // I subtract the borders
    possibleCorridorPlacements -= 2;
    int verticalOffset = (rand()%possibleCorridorPlacements) + 1;
    
    WorldPosition corridorPosition = dstRoom.topLeftCorner + Vector2i(0, verticalOffset);
    
    tileMap->setTileType(corridorPosition, srcRoom.floorType);
  }
  else if(direction == DIRECTION_DOWN)
  {
    int possibleCorridorPlacements = std::min(srcRoom.dimensions.x, dstRoom.dimensions.x);

    // I subtract the borders
    possibleCorridorPlacements -= 2;
    int horizontalOffset = (rand()%possibleCorridorPlacements) + 1;
    
    WorldPosition corridorPosition = dstRoom.topLeftCorner + Vector2i(horizontalOffset, 0);
    
    tileMap->setTileType(corridorPosition, srcRoom.floorType);
  }
  else if(direction == DIRECTION_LEFT)
  {
    int possibleCorridorPlacements = std::min(srcRoom.dimensions.y, dstRoom.dimensions.y);

    // I subtract the borders
    possibleCorridorPlacements -= 2;
    int verticalOffset = (rand()%possibleCorridorPlacements) + 1;
    
    WorldPosition corridorPosition = srcRoom.topLeftCorner + Vector2i(0, verticalOffset);
    
    tileMap->setTileType(corridorPosition, srcRoom.floorType);
  }
  
}

void
SimpleLevelGenerator::openWall(const Room& srcRoom, const Room& dstRoom, const DIRECTION direction)
{
  
  TileMapPtr tileMap = level->getTileMap();

  if(direction == DIRECTION_UP)
  {
    int possibleCorridorPlacements = std::min(srcRoom.dimensions.x, dstRoom.dimensions.x);
    // I subtract the borders
    possibleCorridorPlacements -= 2;
    
    for(int offset = 0; offset < possibleCorridorPlacements; offset++)
    {
      WorldPosition corridorPosition = srcRoom.topLeftCorner + Vector2i(offset + 1, 0);
      tileMap->setTileType(corridorPosition, srcRoom.floorType);
    }
  }
  else if(direction == DIRECTION_RIGHT)
  {
    int possibleCorridorPlacements = std::min(srcRoom.dimensions.y, dstRoom.dimensions.y);

    // I subtract the borders
    possibleCorridorPlacements -= 2;
    for(int offset = 0; offset < possibleCorridorPlacements; offset++)
    {
      
      WorldPosition wallTilePosition = dstRoom.topLeftCorner + Vector2i(0, offset + 1);
      tileMap->setTileType(wallTilePosition, srcRoom.floorType);
    }
    
  }
  else if(direction == DIRECTION_DOWN)
  {
    int possibleCorridorPlacements = std::min(srcRoom.dimensions.x, dstRoom.dimensions.x);
    
    // I subtract the borders
    possibleCorridorPlacements -= 2;

    for(int offset = 0; offset < possibleCorridorPlacements; offset++)
    {

      WorldPosition corridorPosition = dstRoom.topLeftCorner + Vector2i(offset + 1, 0);
      tileMap->setTileType(corridorPosition, srcRoom.floorType);
    }
  }
  else if(direction == DIRECTION_LEFT )
  {
    int possibleCorridorPlacements = std::min(srcRoom.dimensions.y, dstRoom.dimensions.y);

    // I subtract the borders
    possibleCorridorPlacements -= 2;

    for(int offset = 0; offset < possibleCorridorPlacements; offset++)
    {
      WorldPosition corridorPosition = srcRoom.topLeftCorner + Vector2i(0, offset + 1);
      tileMap->setTileType(corridorPosition, srcRoom.floorType);
    }
  }
  
}

void
SimpleLevelGenerator::generateRoom()
{
  TileMapPtr tileMap = level->getTileMap();
  Room potentialRoom;
  
  static const Vector2i roomDeltaDimensions(20, 15);
  static const Vector2i roomMinDimensions(5, 5);
  
  if(placedRooms == 0)
  {
    
    potentialRoom.dimensions = Vector2i((rand() % roomDeltaDimensions.x) + roomMinDimensions.x,
					(rand() % roomDeltaDimensions.y) + roomMinDimensions.y);
    
    placeRoom(potentialRoom);
    
    Player* player = new Player(EntityPosition(WorldPosition(), Vector2f(1.0f,1.0f)));
    EntityPtr playerPtr = EntityPtr(player);
    level->addEntity(playerPtr);
    level->setPlayer(player);
    
    currentRoomPath.push_back(potentialRoom);
    rooms.push_back(potentialRoom);
    
    ++placedRooms;
  }
  else
  {
    while(1)
    {
      bool isRoomPlaced = false;
      
      // Getting The last room from the current branch
      Room currentRoom = currentRoomPath.back();
      
      int numbOfTries = 2;
      while(numbOfTries)
      {
	potentialRoom.topLeftCorner = currentRoom.topLeftCorner;
	potentialRoom.depth = currentRoom.depth + 1;

	// Trying To Put Adjacent Room in one of cardinal directions
	
	DIRECTION direction = DIRECTION(rand() % 4);
    
	potentialRoom.dimensions = Vector2i((rand() % roomDeltaDimensions.x) + roomMinDimensions.x,
					    (rand() % roomDeltaDimensions.y) + roomMinDimensions.y);
	  
	if(direction == DIRECTION_RIGHT)
	  potentialRoom.topLeftCorner += Vector2i(currentRoom.dimensions.x - 1, 0);
	if(direction == DIRECTION_DOWN)
	  potentialRoom.topLeftCorner += Vector2i(0, currentRoom.dimensions.y - 1);
	if(direction == DIRECTION_LEFT)
	  potentialRoom.topLeftCorner += Vector2i(-(potentialRoom.dimensions.x - 1), 0);
	if(direction == DIRECTION_UP)
	  potentialRoom.topLeftCorner += Vector2i(0, -(potentialRoom.dimensions.y - 1));

	// If it's not colliding place it on the map
	if(!potentialRoom.isColliding(tileMap))
	{
	  
	  if(potentialRoom.depth > 25) potentialRoom.floorType = TILE_TYPE_STONE_ICE_GROUND;
	  else if(potentialRoom.depth > 20) potentialRoom.floorType = TILE_TYPE_STONE_SPEED_GROUND;
	  else potentialRoom.floorType = TILE_TYPE_STONE_GROUND;
	  
	  placeRoom(potentialRoom);
	  placeRoomEntities(potentialRoom);
	  
	  // There's 90 chance that wall will be opened
	  if(rand()%100 < 10)
	  {
	    placeCorridor(currentRoom, potentialRoom, direction);
	  }
	  else
	  {
	    openWall(currentRoom, potentialRoom, direction);
	  }

	  currentRoomPath.push_back(potentialRoom);
	  rooms.push_back(potentialRoom);
	  isRoomPlaced = true;
	  break;
	}
	  
	--numbOfTries;
      }
      
      if(isRoomPlaced) {
	
	++placedRooms;

	// 20 % If The Room is last

	int chanceToGoBack = int((float)placedRooms / (float)numbOfRoomsToGenerate) * 60;
	
	if(rand() % 100 < chanceToGoBack) currentRoomPath.pop_back();
	break;
      }
      else
      {
	currentRoomPath.pop_back();

	// Adding Random Room If The CurrentPath Is Empty 
	if(currentRoomPath.size() == 0)
	{
	  auto roomIt = rooms.begin();
	  advance(roomIt, rand() % rooms.size());
	  
	  currentRoomPath.push_back(*roomIt);
	}
	
      }
    }
  }
}

LevelPtr
SimpleLevelGenerator::regenerate(int seed)
{
  
  if(seed != 0)
  {
    startSeed = seed;
    this->seed = seed;
  }
  else this->seed = startSeed;
  
  finishedGenerating = false;

  rooms.clear();
  currentRoomPath.clear();
  
  placedRooms = 0;
    
  level = LevelPtr(new Level());
  
  Player* player = new Player(EntityPosition(WorldPosition(), Vector2f(2.0f,2.0f)));
  EntityPtr playerPtr = EntityPtr(player);
  level->addEntity(playerPtr);
  level->setPlayer(player);
  
  return level;
}

void
SimpleLevelGenerator::generate()
{
  srand(seed);
  
  TileMapPtr tileMap = level->getTileMap();

  int i = 0;
  while(placedRooms != numbOfRoomsToGenerate)
  {
    generateRoom();
  }
  
  maxRoomDepth = getMaxRoomDepth();
  placeRemainingEntities();
  finishedGenerating = true;
}

void
SimpleLevelGenerator::generateStep()
{
  srand(seed++);

  generateRoom();
  
  if(placedRooms == numbOfRoomsToGenerate ||
     (placedRooms != 1 && currentRoomPath.size() == 0))
  {
    
    maxRoomDepth = getMaxRoomDepth();
    placeRemainingEntities();
    finishedGenerating = true;
  }
}

void
SimpleLevelGenerator::renderAdditionalData(sf::RenderWindow& window,
						EntityPosition& cameraPosition,
						float tileSizeInPixels)
{
  TileMapPtr tileMap = level->getTileMap();
  
  const sf::Vector2u windowDimensions = window.getSize();

  Vector2f tileChunkSizeInPixels(tileMap->tileChunkSize.x, tileMap->tileChunkSize.y);
  tileChunkSizeInPixels *= tileSizeInPixels;
  
  cameraPosition.recanonicalize(tileMap->tileChunkSize);
  
  // cameraPosition identifies center of the viewport so we have to translate it
  
  float tilesPerScreenWidth = (float)windowDimensions.x/tileSizeInPixels;
  float tilesPerScreenHeight = (float)windowDimensions.y/tileSizeInPixels;
  
  EntityPosition topLeftViewport = cameraPosition;
  topLeftViewport.tileOffset.x -= tilesPerScreenWidth / 2.0f;
  topLeftViewport.tileOffset.y -= tilesPerScreenHeight / 2.0f;
  
  topLeftViewport.recanonicalize(tileMap->tileChunkSize);
  
  // CameraPosition in Pixels Inside The Chunk
  Vector2f cameraOffset((float) topLeftViewport.worldPosition.tilePosition.x * tileSizeInPixels,
			(float) topLeftViewport.worldPosition.tilePosition.y * tileSizeInPixels);
  
  sf::RectangleShape rectangleShape;
  rectangleShape.setOutlineThickness(0);
  
  int maxRoomDepth = getMaxRoomDepth();
  for(auto roomIt = rooms.begin(); roomIt != rooms.end(); roomIt++)
  {
    
    // TopLeftCorner Of the Room In Tiles 
    Vector2f topLeftCornerOnScreen = EntityPosition::calculateDistanceInTiles(topLeftViewport,
								       EntityPosition(roomIt->topLeftCorner),
								       tileMap->tileChunkSize);
    topLeftCornerOnScreen += Vector2f(1, 1);
    
    topLeftCornerOnScreen *= tileSizeInPixels;
    rectangleShape.setPosition(topLeftCornerOnScreen.x, topLeftCornerOnScreen.y);
    
    float roomDifficulty = (float)roomIt->depth/(float)maxRoomDepth;

    if(roomIt->depth != 0)
    {
      rectangleShape.setFillColor(sf::Color(255, 0, 0, sf::Uint8(roomDifficulty * 255.0f)));
    }
    else
    {
      rectangleShape.setFillColor(sf::Color(0, 128, 0, 128));
    }

    // Subtracting Borders
    sf::Vector2f roomSize(roomIt->dimensions.x - 2, roomIt->dimensions.y - 2);
    roomSize *= tileSizeInPixels;
    
    rectangleShape.setSize(roomSize);
    
    // TODO Check If It's On The Screen !
    window.draw(rectangleShape);
  }
  
}
