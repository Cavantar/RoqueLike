#pragma once

#include <unordered_map>
#include <vector>
#include <memory>
#include <assert.h>
#include "Entity.h"

typedef std::list<EntityPtr> EntityList;
typedef std::list<WorldPosition> TileList;

class Level : public EventOperator{
  friend class LevelGenerator;
  friend class SimpleLevelGenerator;
 public:
  Level(); 
  void update(const float lastDelta);
  void registerPendingEntities(EventManager& eventManager);
  
  //void resolveCollisions();
  
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
  
  TileList getAffectedTiles(const FloatRect& collisionRect,
			    const EntityPosition& basePosition,
			    const Vector2f positionDeltaVector) const;
  
  WorldCollisionResult checkWorldCollision(const FloatRect& collisionRect,
					   const EntityPosition& basePosition,
					   const Vector2f positionDeltaVector = Vector2f()) const;
  
  EntityCollisionResult checkEntityCollision(const Entity* entity,
					     const FloatRect& collisionRect,
					     const EntityPosition& basePosition,
					     const Vector2f positionDeltaVector = Vector2f()) const;
  
  bool isCollidingWithLevel(Entity* entity) const;
  void killCollidingEntities();
};

typedef std::shared_ptr<Level> LevelPtr;
