#pragma once

#include <unordered_map>
#include <vector>
#include <memory>
#include <assert.h>
#include <iostream>

#include "ILevel.h"
#include "Entity.h"
#include "Mobs.h"
#include "TileMap.h"
#include "TileState.h"

typedef std::list<EntityPtr> EntityList;
typedef std::list<WorldPosition> TileList;

struct CollisionCheckData {
  EntityPosition basePosition;
  FloatRect collisionRect;
  Vec2f deltaVec;
};

const int numbOfEntityLayers = 2;

class Level : public ILevel{
public:
  Level(); 
  void update(const float lastDelta);

  // Registers entities in eventManager and puts them in list of ordinary entities
  void registerPendingEntities(EventManager& eventManager);

  const TileMapPtr& getTileMap() const { return tileMap; }
  const EntityList& getEntityList(int layerIndex = 0) const { return entityList[layerIndex];}
  
  bool addEntity(EntityPtr& entityPtr);
  
  // Overlay Entities Won't be registered with EventManager
  // Adds to the second layer of entities 
  void addOverlayEntity(EntityPtr& entityPtr);
  
  Player* getPlayer() const { return player; }
  void setPlayer(Player* player) { this->player = player; }
  
  void removeDeadEntities();

  // Checks collision between two entities and returns collision results 
  EntityCollisionResult checkCollisions(const Entity* entity, Vec2f deltaVec) const ;
  bool canSeeEachOther(const Entity* entity1, const Entity* entity2, float maxRange) const ;
  Vec2f canSeeEachOtherCardinal(const Entity* entity1, const Entity* entity2, float maxRange) const ; 

  float getFrictionValueAtPosition(EntityPosition& entityPosition) const; 
  float getAccelerationModifierAtPosition(EntityPosition& entityPosition) const;

  // Returns state of the tile value where 
  int getSurroundingTileData(const WorldPosition& worldPosition, TILE_TYPE tileType);
  
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
  
  // Returns the list of tiles that are affected depending on collisionCheckData
  TileList getAffectedTiles(const CollisionCheckData& collisionCheckData) const;
  
  WorldCollisionResult checkCollisionsWithTiles(TileList& tiles,
						const CollisionCheckData& collisionCheckData) const;
  
  WorldCollisionResult checkWorldCollision(const CollisionCheckData& collisionCheckData) const;
  
  // Checks Collisions with the whole level 
  EntityCollisionResult checkEntityCollision(const Entity* entity,
					     const CollisionCheckData& collisionCheckData) const;
  
  bool isCollidingWithLevel(Entity* entity) const;

  // For Debugging purposes - when testing collision checks 
  void killCollidingEntities();
};

typedef std::shared_ptr<Level> LevelPtr;
