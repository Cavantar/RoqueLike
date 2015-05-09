#pragma once
#include "EntityPosition.h"
#include "Level.h"

#include <SFML/Graphics.hpp>

class Room{
 public:
  WorldPosition topLeftCorner;
  Vector2i dimensions;
  int depth;
  TILE_TYPE floorType;
  
  bool isColliding(TileMapPtr tileMap);
  
  Room(const WorldPosition& topLeftCorner=WorldPosition(), const Vector2i& dimensions=Vector2i(),
       int32 depth=0, TILE_TYPE floorType = TILE_TYPE_STONE_GROUND) :
    topLeftCorner(topLeftCorner), dimensions(dimensions), depth(depth), floorType(floorType) {}
};

enum DIRECTION{
  DIRECTION_UP,
  DIRECTION_RIGHT,
  DIRECTION_DOWN,
  DIRECTION_LEFT
};

class LevelGenerator{
 public:
 LevelGenerator() : finishedGenerating(false){}
  
  virtual ~LevelGenerator() {}
  LevelPtr create(int seed = 0);
  
  virtual void generate() = 0;
  virtual LevelPtr regenerate(int seed = 0) = 0;
  
  virtual void generateStep() {};

  virtual void renderAdditionalData(sf::RenderWindow& window,
				    EntityPosition& cameraPosition,
				    float tileSizeInPixels) {}
  
  bool isGenerationFinished() { return finishedGenerating;}
  
protected:
  LevelPtr level;
  int seed;
  bool finishedGenerating;

  void addEntity(EntityPtr entity);
  
  void placeLine(WorldPosition startPosition, Vector2i deltaVector, TILE_TYPE tileType);
  void fillRectangle(WorldPosition startPosition, Vector2i dimensions, TILE_TYPE tileType);
  
};

class SimpleLevelGenerator : public LevelGenerator{
 public:
 SimpleLevelGenerator(const int numbOfRoomsToGenerate) :
  numbOfRoomsToGenerate(numbOfRoomsToGenerate) {}
  
  void generate();

  // If The seed is 0 we regenerate the same level
  
  LevelPtr regenerate(int seed = 0);
  void generateStep();
  
  void renderAdditionalData(sf::RenderWindow& window,
			    EntityPosition& cameraPosition,
			    float tileSizeInPixels);
  
 private:
  std::list<Room> rooms;
  std::list<Room> currentRoomPath;

  int placedRooms = 0;
  int numbOfRoomsToGenerate;

  // Set after level is generated completely
  int maxRoomDepth = -1;
  
  void placeRoom(const Room& room);
  void placeRoomEntities(const Room& room, bool immediateMode = true);
  
  void placeRemainingEntities();
  void generateRoom();
  
  int getMaxRoomDepth() const ;
  
  // Places Corridor When The Rooms Are Touching
  void placeCorridor(const Room& srcRoom, const Room& dstRoom, DIRECTION direction);
  
  // Removes The Wall Between Rooms 
  void openWall(const Room& srcRoom, const Room& dstRoom, DIRECTION direction);
};

