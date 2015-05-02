#include "LevelGenerator.h"

bool Room::isColliding(TileMapPtr tileMap)
{
  bool result;
  
  result = !tileMap->isRectangleOfTileType(topLeftCorner + Vector2i(1, 1),
					  dimensions - Vector2i(2, 2),
					  TILE_TYPE_VOID);
  
  return result;
}

LevelPtr LevelGenerator::create(int seed)
{
  if(seed == 0)
  {
    seed = time(NULL);
  }

  this->seed = seed;
  
  level = LevelPtr(new Level());
  
  Player* player = new Player(EntityPosition(WorldPosition(), Vector2f(2.0f,2.0f)));
  EntityPtr playerPtr = EntityPtr(player);
  level->addEntity(playerPtr);
  level->player = player;
  
  return level;
}

void LevelGenerator::placeLine(WorldPosition startPosition, Vector2i deltaVector, TILE_TYPE tileType)
{
  // TODO Implement Other Octants

  Vector2i currentPosition;

  float slope = (float)deltaVector.y / (float)deltaVector.x;
  float slopeAcc = 0;

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
      level->tileMap->setTileType(startPosition + currentPosition, tileType);

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
      level->tileMap->setTileType(startPosition + currentPosition, tileType);

      ++currentPosition.y;
    }
  }

}

void LevelGenerator::fillRectangle(WorldPosition startPosition, Vector2i dimensions, TILE_TYPE tileType)
{

  for(int y = 0; y < dimensions.y; y++)
  {
    for(int x = 0; x < dimensions.x; x++)
    {
      level->tileMap->setTileType(startPosition + Vector2i(x, y), tileType);
    }
  }
}

void SimpleLevelGenerator::placeRoom(const Room& room)
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
		TILE_TYPE_FLOOR);
  
}

void SimpleLevelGenerator::placeRoomEntities(const Room& room)
{

  //std::list<WorldPostion> takenFields;  

  // Health Pack
  WorldPosition entityPosition;
  EntityPtr entity;
  
  if((rand()%11) < 3)
  {
    entityPosition = room.topLeftCorner + Vector2i(1, 1);
    entityPosition += Vector2i(rand()%(room.dimensions.x-2), rand()%(room.dimensions.y-2));
    entity = EntityPtr(new HealthItem(EntityPosition(entityPosition), (float)room.depth / 15));
    level->addEntity(entity);
  }

  static bool placedCannon = false;
  
  if(!placedCannon && (rand()%11) < 3)
  {
    entityPosition = room.topLeftCorner + Vector2i(1, 1);
    entityPosition += Vector2i(rand()%(room.dimensions.x-2), rand()%(room.dimensions.y-2));
    
    entity = EntityPtr(new Cannon(EntityPosition(entityPosition)));
    
    level->addEntity(entity);
    
    placedCannon = true;
  }
}

int SimpleLevelGenerator::getMaxRoomDepth() const
{
  int maxDepth = rooms.begin()->depth;
  for(auto roomIt = rooms.begin(); roomIt != rooms.end(); roomIt++)
  {
    if(roomIt->depth > maxDepth) maxDepth = roomIt->depth;
  }
  return maxDepth;
}

void SimpleLevelGenerator::placeCorridor(const Room& srcRoom, const Room& dstRoom, const DIRECTION direction)
{
  if(direction == DIRECTION_UP)
  {
    int possibleCorridorPlacements = std::min(srcRoom.dimensions.x, dstRoom.dimensions.x);
    
    // I subtract the borders
    possibleCorridorPlacements -= 2;
    int horizontalOffset = (rand()%possibleCorridorPlacements) + 1;
    
    WorldPosition corridorPosition = srcRoom.topLeftCorner + Vector2i(horizontalOffset, 0);
    
    level->tileMap->setTileType(corridorPosition, TILE_TYPE_FLOOR);
  }
  else if(direction == DIRECTION_RIGHT)
  {
    int possibleCorridorPlacements = std::min(srcRoom.dimensions.y, dstRoom.dimensions.y);

    // I subtract the borders
    possibleCorridorPlacements -= 2;
    int verticalOffset = (rand()%possibleCorridorPlacements) + 1;
    
    WorldPosition corridorPosition = dstRoom.topLeftCorner + Vector2i(0, verticalOffset);
    
    level->tileMap->setTileType(corridorPosition, TILE_TYPE_FLOOR);
  }
  else if(direction == DIRECTION_DOWN)
  {
    int possibleCorridorPlacements = std::min(srcRoom.dimensions.x, dstRoom.dimensions.x);

    // I subtract the borders
    possibleCorridorPlacements -= 2;
    int horizontalOffset = (rand()%possibleCorridorPlacements) + 1;
    
    WorldPosition corridorPosition = dstRoom.topLeftCorner + Vector2i(horizontalOffset, 0);
    
    level->tileMap->setTileType(corridorPosition, TILE_TYPE_FLOOR);
  }
  else if(direction == DIRECTION_LEFT)
  {
    int possibleCorridorPlacements = std::min(srcRoom.dimensions.y, dstRoom.dimensions.y);

    // I subtract the borders
    possibleCorridorPlacements -= 2;
    int verticalOffset = (rand()%possibleCorridorPlacements) + 1;
    
    WorldPosition corridorPosition = srcRoom.topLeftCorner + Vector2i(0, verticalOffset);
    
    level->tileMap->setTileType(corridorPosition, TILE_TYPE_FLOOR);
  }
    
}

void SimpleLevelGenerator::openWall(const Room& srcRoom, const Room& dstRoom, const DIRECTION direction)
{
  if(direction == DIRECTION_UP)
  {
    int possibleCorridorPlacements = std::min(srcRoom.dimensions.x, dstRoom.dimensions.x);
    // I subtract the borders
    possibleCorridorPlacements -= 2;
    
    for(int offset = 0; offset < possibleCorridorPlacements; offset++)
    {
      WorldPosition corridorPosition = srcRoom.topLeftCorner + Vector2i(offset + 1, 0);
      level->tileMap->setTileType(corridorPosition, TILE_TYPE_FLOOR);
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
      level->tileMap->setTileType(wallTilePosition, TILE_TYPE_FLOOR);
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
      level->tileMap->setTileType(corridorPosition, TILE_TYPE_FLOOR);
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
      level->tileMap->setTileType(corridorPosition, TILE_TYPE_FLOOR);
    }
  }
  
}

void SimpleLevelGenerator::generateRoom()
{
    
  TileMapPtr tileMap = level->tileMap;
  Room potentialRoom;
  
  if(placedRooms == 0)
  {
    
    potentialRoom.dimensions = Vector2i(rand() % 15 + 4, rand() % 15 + 4);
    
    placeRoom(potentialRoom);
    
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
	
	potentialRoom.dimensions = Vector2i(rand() % 20 + 4, rand() % 20 + 4);
	  
	if(direction == DIRECTION_RIGHT)
	  potentialRoom.topLeftCorner += Vector2i(currentRoom.dimensions.x - 1, 0);
	if(direction == DIRECTION_DOWN)
	  potentialRoom.topLeftCorner += Vector2i(0, currentRoom.dimensions.y - 1);
	if(direction == DIRECTION_LEFT)
	  potentialRoom.topLeftCorner += Vector2i(-(potentialRoom.dimensions.x - 1), 0);
	if(direction == DIRECTION_UP)
	  potentialRoom.topLeftCorner += Vector2i(0, -(potentialRoom.dimensions.y - 1));

	// If it's not colliding we place it on the map
	if(!potentialRoom.isColliding(tileMap))
	{
	  placeRoom(potentialRoom);
	  placeRoomEntities(potentialRoom);
	  
	  // There's 80 chance that wall will be opened
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

	int chanceToGoBack = int((float)placedRooms / (float)numbOfRoomsToGenerate) * 40;
	
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

LevelPtr SimpleLevelGenerator::regenerate(int seed)
{
  if(seed != 0)
  {
    this->seed = seed;
  }

  finishedGenerating = false;

  rooms.clear();
  currentRoomPath.clear();
  
  placedRooms = 0;
    
  level = LevelPtr(new Level());
  
  Player* player = new Player(EntityPosition(WorldPosition(), Vector2f(2.0f,2.0f)));
  EntityPtr playerPtr = EntityPtr(player);
  level->addEntity(playerPtr);
  level->player = player;
  
  return level;
}

void SimpleLevelGenerator::generate()
{
  srand(seed);

  TileMapPtr tileMap = level->tileMap;
  
  while(placedRooms != numbOfRoomsToGenerate)
  {
    generateRoom();
  }
  
  finishedGenerating = true;
}

void SimpleLevelGenerator::generateStep()
{
  srand(seed++);

  generateRoom();
  
  if(placedRooms == numbOfRoomsToGenerate ||
     (placedRooms != 1 && currentRoomPath.size() == 0))
  {
    finishedGenerating = true;
  }
}

void SimpleLevelGenerator::renderAdditionalData(sf::RenderWindow& window,
						EntityPosition& cameraPosition,
						float tileSizeInPixels)
{
  TileMapPtr tileMap = level->tileMap;
  
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
