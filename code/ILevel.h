#pragma once

#include "Event.h"
#include "EventManager.h"
#include "TileMap.h"
#include <memory>

class Entity;
typedef std::shared_ptr<Entity> EntityPtr;
typedef std::list<EntityPtr> EntityList;

enum COLLISION_PLANE{
  COLLISION_PLANE_VERTICAL,
  COLLISION_PLANE_HORIZONTAL,
  COLLISION_PLANE_BOTH,
  COLLISION_PLANE_NONE
};

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

class Player;

class ILevel : public EventOperator{
public:
  virtual ~ILevel() {};
  
  virtual void update(const float lastDelta) = 0;
  virtual void registerPendingEntities(EventManager& eventManager) = 0;
  
  virtual const TileMapPtr& getTileMap() const = 0;
  virtual const EntityList& getEntityList(int layerIndex = 0) const = 0;
  
  virtual bool addEntity(EntityPtr& entityPtr) = 0;
  virtual void addOverlayEntity(EntityPtr& entityPtr) = 0;
  virtual Player* getPlayer() const = 0;
  
  virtual void removeDeadEntities() = 0;
  virtual EntityCollisionResult checkCollisions(const Entity* entity, Vec2f deltaVec) const  = 0;
  
  virtual bool canSeeEachOther(const Entity* entity1, const Entity* entity2, float maxRange) const = 0;
  virtual Vec2f canSeeEachOtherCardinal(const Entity* entity1, const Entity* entity2, float maxRange) const = 0; 
  
  virtual float getFrictionValueAtPosition(EntityPosition& entityPosition) const = 0; 
  virtual float getAccelerationModifierAtPosition(EntityPosition& entityPosition) const = 0;
};

