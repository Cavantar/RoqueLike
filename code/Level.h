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

class Level : public ILevel{
  friend class LevelGenerator;
  friend class SimpleLevelGenerator;
 public:
Level(); 
//~Level(){}; 
void update(const float lastDelta);
void registerPendingEntities(EventManager& eventManager);
  
  const TileMapPtr& getTileMap() const { return tileMap; }
  const EntityList& getEntityList() const { return entityList;}
  
  bool addEntity(EntityPtr& entityPtr);
  Player* getPlayer() const { return player; }
  
  void removeDeadEntities();

  EntityCollisionResult checkCollisions(const Entity* entity, Vector2f deltaVector) const ;

  float getFrictionValueAtPosition(EntityPosition& entityPosition) const; 
  float getAccelerationModifierAtPosition(EntityPosition& entityPosition) const; 
  
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
