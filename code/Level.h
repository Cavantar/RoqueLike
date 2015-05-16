#pragma once

#include <unordered_map>
#include <vector>
#include <memory>
#include <assert.h>

#include "ILevel.h"
#include "Entity.h"
#include "TileMap.h"

typedef std::list<EntityPtr> EntityList;
typedef std::list<WorldPosition> TileList;

struct CollisionCheckData{
  EntityPosition basePosition;
  FloatRect collisionRect;
  Vector2f deltaVector;
};

const int numbOfEntityLayers = 2;

enum SURROUNDING_TILE {
  ST_NORTH = 1,
  ST_NORTH_EAST = 2,
  ST_EAST = 4,
  ST_SOUTH_EAST = 8,
  ST_SOUTH = 16,
  ST_SOUTH_WEST = 32,
  ST_WEST = 64,
  ST_NORTH_WEST = 128
};

enum TILE_STATE {
  // Corners
  TS_CORNER_TOPLEFT = ST_SOUTH | ST_SOUTH_EAST | ST_EAST,
  TS_CORNER_TOPRIGHT = ST_SOUTH | ST_SOUTH_WEST | ST_WEST,
  
  TS_CORNER_BOTTOMLEFT = ST_NORTH | ST_NORTH_EAST | ST_EAST,
  TS_CORNER_BOTTOMRIGHT = ST_NORTH | ST_NORTH_WEST | ST_WEST,
  
  // Walls
  TS_WALL_LEFT = ST_NORTH | ST_NORTH_EAST | ST_EAST | ST_SOUTH_EAST | ST_SOUTH,
  TS_WALL_RIGHT = ST_NORTH | ST_NORTH_WEST | ST_WEST | ST_SOUTH_WEST | ST_SOUTH,
  
  TS_WALL_TOP = ST_WEST | ST_SOUTH_WEST | ST_SOUTH | ST_SOUTH_EAST | ST_EAST, 
  TS_WALL_BOTTOM = ST_WEST | ST_NORTH_WEST | ST_NORTH | ST_NORTH_EAST | ST_EAST,

  // Path
  
  TS_PATH_HORIZONTAL = ST_WEST | ST_EAST,
  TS_PATH_VERTICAL = ST_NORTH | ST_SOUTH
  
};

class Level : public ILevel{
public:
  Level(); 
  void update(const float lastDelta);
  void registerPendingEntities(EventManager& eventManager);
  
  const TileMapPtr& getTileMap() const { return tileMap; }
  const EntityList& getEntityList(int layerIndex = 0) const { return entityList[layerIndex];}
  
  bool addEntity(EntityPtr& entityPtr);
  // Overlay Entities Won't be registered with EventManager
  void addOverlayEntity(EntityPtr& entityPtr);
  
  Player* getPlayer() const { return player; }
  void setPlayer(Player* player) { this->player = player; }
  
  void removeDeadEntities();

  EntityCollisionResult checkCollisions(const Entity* entity, Vector2f deltaVector) const ;
  
  float getFrictionValueAtPosition(EntityPosition& entityPosition) const; 
  float getAccelerationModifierAtPosition(EntityPosition& entityPosition) const; 
  int getSurroundingTileData(const WorldPosition& worldPosition, TILE_TYPE tileType);
  
  bool isTileState(TILE_STATE tileState, int surroundTiles);
  
  // Event Operator
  EventNameList getEntityEvents();
  void onEvent(const std::string& eventName, EventArgumentDataMap eventDataMap);
  
private:
  TileMapPtr tileMap;
  EntityList entityList[numbOfEntityLayers];
  
  // Entities That Are Not Yet Registered By The Event Manager
  EntityList pendingEntityList;
  Player* player;
  
  void updateEntities(const float lastDelta);
  TileList getAffectedTiles(const CollisionCheckData& collisionCheckData) const;
  
  WorldCollisionResult checkCollisionsWithTiles(TileList& tiles,
						const CollisionCheckData& collisionCheckData) const;
  
  WorldCollisionResult checkWorldCollision(const CollisionCheckData& collisionCheckData) const;
  
  EntityCollisionResult checkEntityCollision(const Entity* entity,
					     const CollisionCheckData& collisionCheckData) const;
  
  bool isCollidingWithLevel(Entity* entity) const;
  void killCollidingEntities();
};

typedef std::shared_ptr<Level> LevelPtr;
