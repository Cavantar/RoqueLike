#pragma once

#include "Vector.h"
#include "EntityPosition.h"

#include <memory>

enum PLAYER_EVENT{
  PLAYER_MOVE_UP,
  PLAYER_MOVE_DOWN,
  PLAYER_MOVE_LEFT,
  PLAYER_MOVE_RIGHT,
  
  PLAYER_SHOOT_UP,
  PLAYER_SHOOT_DOWN,
  PLAYER_SHOOT_LEFT,
  PLAYER_SHOOT_RIGHT
  
};

enum MOB_DIRECTION{
  MOB_FACING_UP,
  MOB_FACING_RIGHT,
  MOB_FACING_DOWN,
  MOB_FACING_LEFT
};

enum COLLISION_PLANE{
  COLLISION_PLANE_VERTICAL,
  COLLISION_PLANE_HORIZONTAL,
  COLLISION_PLANE_BOTH,
  COLLISION_PLANE_NONE
};

enum ENTITY_RENDER_DATA_TYPE{
  ER_PRIMITIVE,
  ER_MOB
};

enum PRIMITIVE_TYPE{
  PT_RECTANGLE,
  PT_CIRCLE
};

struct EntityRenderData{
  ENTITY_RENDER_DATA_TYPE type;

  union{
    // Primitive
    struct{
      PRIMITIVE_TYPE primitiveType;
      Vector2f dimensionsInTiles;
      Vector3f color;
    };
    
    // Mob
    struct{
      std::string spriteName;
      std::string caption;
      float life;
    };
    
  };
};

enum ENTITY_REQUEST_TYPE{
  ERT_SPAWN_ENTITY
};

enum ENTITY_TYPE{
  ET_BULLET
};

struct EntityRequest{
  ENTITY_REQUEST_TYPE type;
  union{
    struct{
      ENTITY_TYPE entityType;
      EntityPosition position;
      Vector2f initialVelocity;
      Vector2f dimensions;
    };
  };
};

typedef std::list<EntityRequest> EntityRequestList;

class Entity : public EventOperator {
 public:
  Entity();
 Entity(EntityPosition position) : position(position) {};
  virtual ~Entity() {}
  
  virtual void update(const float lastDelta) = 0;
  
  // Position / Movement
  const EntityPosition& getPosition() const { return position; }
  void setPosition(const EntityPosition& position) { this->position = position;}
  
  // Delta vector should be zeroed after accessing it 
  virtual Vector2f getPositionDeltaVector() const  { return Vector2f(); }
  virtual Vector2f getVelocity() const  { return Vector2f(); }
  
  virtual void addVelocity(Vector2f velocity) {}
  
  // Collision Stuff
  virtual FloatRect getCollisionRect() const { return FloatRect();}
  virtual void onWorldCollision(COLLISION_PLANE worldCollisionType) {};
  virtual void onEntityCollision(COLLISION_PLANE worldCollisionType, Entity* entity) {};

  // PlayerStuff
  virtual void addXp(const float amount) {} 
  virtual void addLife(const float amount) {}

  // Flag Stuff
  virtual bool canReceiveItems() const { return false; }
  virtual bool isPlayerItem() const { return false; }

  virtual bool canCollide() const { return true; }
  
  virtual const EntityRenderData& getRenderData() { return renderData; }
  
  // RequestList should be zeroed after accessing it 
  EntityRequestList getRequestList(); 
  virtual bool isAlive() { return alive; }
  
 protected:
  EntityRenderData renderData;
  EntityRequestList entityRequestList;
  
  EntityPosition position;
  bool alive = true; 
};

typedef std::shared_ptr<Entity> EntityPtr;

class Moveable : public Entity {
 public:
  const Vector2f& getDimensions() const { return dimensions; }
  FloatRect getCollisionRect() const ;
  
  Vector2f getPositionDeltaVector() const { return positionDeltaVector; }
  
  Vector2f getVelocity() const { return velocity; }
  void addVelocity(Vector2f velocity) { this->velocity += velocity; }
  
 protected:
  Vector2f dimensions;
  
  Vector2f positionDeltaVector;
  Vector2f velocity;
  Vector2f acceleration;
  
  float metersPerSecondSquared = 50;
  
  void updateDeltaVectorPosition(const float lastDelta, const float fakeFrictionValue);
};

class Bullet : public Moveable {
 public:
  Bullet(const EntityPosition& position, const Vector2f& initialVelocity, const Vector2f& dimensions);
  
  void update(const float lastDelta);
  void onWorldCollision(COLLISION_PLANE worldCollisionType);
  void onEntityCollision(COLLISION_PLANE worldCollisionType, Entity* entity);
  
  FloatRect getCollisionRect() const;
  
  int numbOfBouncesLeft = 10;
};

class Item : public Entity {
 public:
  Item(const EntityPosition& position, const float value);
  void update(const float lastDelta) {}
  
  FloatRect getCollisionRect() const;
  void onEntityCollision(COLLISION_PLANE worldCollisionType, Entity* actionReceiver);
  
  bool isPlayerItem() const { return true; }
 protected:
  float itemValue;
  
  virtual void performItemAction(Entity* entity) = 0;
};

class HealthItem : public Item{
 public:
 HealthItem(const EntityPosition& position, const float value) : Item(position, value) {}
  
 private:
  void performItemAction(Entity* actionReceiver);
};

class Mob: public Moveable {
 public:
  Mob(const EntityPosition& position);
  void addLife(const float amount);
  const EntityRenderData& getRenderData();
  
 protected:
  int level = 0;
  float life = 0.1f;
};

class Cannon : public Mob {
 public:
  Cannon(const EntityPosition& position);
  void update(const float lastDelta);
  
 private:
  float localTime = 0;
};

class Player : public Mob {
 public:
  Player(const EntityPosition& position);
  
  void update(const float lastDelta);
  void onWorldCollision(COLLISION_PLANE worldCollisionType);
  void onEntityCollision(COLLISION_PLANE worldCollisionType, Entity* entity);
  
  void handlePlayerEvent(const PLAYER_EVENT playerEvent, const float lastDelta);
  
  bool canReceiveItems() const { return true; }

  // Events and Stuff
  
  EventNameList getEntityEvents();
  void onEvent(const std::string& eventName, EventArgumentDataMap eventDataMap);
  
 private:
  MOB_DIRECTION direction;
};
