#pragma once

#include <unordered_map>
#include <vector>
#include <memory>
#include <assert.h>

#include "Vector.h"
#include "TileMap.h"
#include "Entity.h"

#ifdef UNITY_BUILD

#include "Vector.cpp"
#include "TileMap.cpp"
#include "Entity.cpp"

#endif

typedef std::list<EntityPtr> EntityList;
typedef std::list<WorldPosition> TileList;

class WorldCollisionResult{
 public:
  float maxAllowedT;
  COLLISION_PLANE collisionPlane;
  
 WorldCollisionResult() : maxAllowedT(1.0f), collisionPlane(COLLISION_PLANE_NONE) {}
};

class EntityCollisionResult{
 public:
  float maxAllowedT;
  Entity* collidedEntity;
  COLLISION_PLANE collisionPlane;
  
 EntityCollisionResult() : maxAllowedT(1.0f), collidedEntity(NULL), collisionPlane(COLLISION_PLANE_NONE) {}
};

class Level : public EventOperator{
  friend class LevelGenerator;
  friend class SimpleLevelGenerator;
 public:
  Level(); 
  void update(const float lastDelta);
  void registerPendingEntities(EventManager& eventManager);
  
  void resolveCollisions();
  
  const TileMapPtr& getTileMap() const { return tileMap; }
  const EntityList& getEntityList() const { return entityList;}
  
  void addEntity(EntityPtr& entityPtr);
  Player* getPlayer() const { return player; }
  
  void removeDeadEntities();
  
  // Event Operator
  EventNameList getEntityEvents();
  void onEvent(const std::string& eventName, EventArgumentDataMap eventDataMap);
  
private:
  TileMapPtr tileMap;
  EntityList entityList;
  
  // Entities That Are Not Yet Registered By The Event Manager
  EntityList pendingEntityList;
  
  Player* player;
  
  void updateEntities(const float lastDelta);
  
  TileList getAffectedTiles(const FloatRect& collisionRect,
					    const EntityPosition& basePosition,
					    const Vector2f positionDeltaVector) const;
  
  WorldCollisionResult checkWorldCollision(const FloatRect& collisionRect,
					   const EntityPosition& basePosition,
					   const Vector2f positionDeltaVector = Vector2f()) const;
  
  EntityCollisionResult checkEntityCollision(const EntityPtr& entityPtr) const;
  
  bool isCollidingWithLevel(Entity* entity) const;

  
  void killCollidingEntities();

};

typedef std::shared_ptr<Level> LevelPtr;
