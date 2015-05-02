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

class Level{
  friend class LevelGenerator;
  friend class SimpleLevelGenerator;
 public:
  Level(); 
  void update(const float lastDelta);
  void resolveCollisions();
  
  const TileMapPtr& getTileMap() const { return tileMap; }
  const EntityList& getEntityList() const { return entityList;}

  void addEntity(EntityPtr& entityPtr);

  Player* getPlayer() const { return player; }
 private:
  // TODO ImplementHashMap
  TileMapPtr tileMap;
  EntityList entityList;
  Player* player;

  void processEntityRequest(const EntityRequest& entityRequest);
  void processEntityRequests(const EntityRequestList& entityRequestList);
  
  void updateEntities(const float lastDelta);
  void removeDeadEntities();

  TileList getAffectedTiles(const FloatRect& collisionRect,
					    const EntityPosition& basePosition,
					    const Vector2f positionDeltaVector) const;
  
  WorldCollisionResult checkWorldCollision(const FloatRect& collisionRect,
					   const EntityPosition& basePosition,
					   const Vector2f positionDeltaVector = Vector2f()) const;
  
  EntityCollisionResult checkEntityCollision(const EntityPtr& entityPtr) const;
  
  bool isCollidingWithLevel(const Entity* entity) const;
};

typedef std::shared_ptr<Level> LevelPtr;
