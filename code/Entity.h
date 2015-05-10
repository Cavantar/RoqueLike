#pragma once

#include "Vector.h"
#include "EntityPosition.h"
#include "ILevel.h"

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

class Entity : public EventOperator {
public:
  Entity();
  Entity(EntityPosition position) : position(position) {};
  virtual ~Entity() {}
  
  virtual void update(ILevel* level, const float lastDelta) = 0;
  
  // Position / Movement
  const EntityPosition& getPosition() const { return position; }
  void setPosition(const EntityPosition& position) { this->position = position;}
  
  // Delta vector should be zeroed after accessing it 
  virtual Vector2f getVelocity() const  { return Vector2f(); }
  virtual void addVelocity(Vector2f velocity) {}
  
  // Collision Stuff
  virtual FloatRect getCollisionRect() const { return FloatRect();}
  virtual void onWorldCollision(COLLISION_PLANE worldCollisionType) {}
  virtual void onEntityCollision(COLLISION_PLANE worldCollisionType, Entity* entity) {}
  
  virtual void performDeathAction(ILevel* level) {}
  
  // PlayerStuff
  virtual void addXp(const float amount) {} 
  virtual void addHealth(const float amount) {}
  
  // Flag Stuff
  virtual bool canReceiveItems() const { return false; }
  virtual bool isPlayerItem() const { return false; }
  virtual bool isPlayer() const { return false; }
  virtual bool canCollideWithEntities() const { return true; }
  
  virtual const EntityRenderData& getRenderData() { return renderData; }
  virtual bool isAlive() { return alive; }
  void die();
  
protected:
  EntityRenderData renderData;
  
  EntityPosition position;
  bool alive = true; 
};

typedef std::shared_ptr<Entity> EntityPtr;

class Moveable : public Entity {
 public:
  const Vector2f& getDimensions() const { return dimensions; }
  FloatRect getCollisionRect() const ;
  
  Vector2f getVelocity() const { return velocity; }
  void addVelocity(Vector2f velocity) { this->velocity += velocity; }

  Vector2f getLocalCollisionCenter() const ;
  EntityPosition getCollisionCenter() const ;
  
  Vector2f getReflectedVelocity(COLLISION_PLANE collisionPlane, float speedIncrease) const ;
  float getMovementSpeed() const { return metersPerSecondSquared; }
  
protected:
  Vector2f dimensions;
  
  Vector2f velocity;
  Vector2f acceleration;
  
  float metersPerSecondSquared = 25;
  
  Vector2f getPositionDeltaVector(const float lastDelta, const float fakeFrictionValue,
				  const float accelerationModifier = 1.0f);
    
  void handleCollisionResult(EntityCollisionResult& collisionResult,
			     const Vector2f& positionDeltaVector);
};

class XpOrb : public Moveable {
public:
  XpOrb(const EntityPosition& position, const Vector2f& initialVelocity, float xpAmount);
  void update(ILevel* level, const float lastDelta);
  
  bool isPlayerItem() const { return true; }
  
  void onWorldCollision(COLLISION_PLANE worldCollisionType);
  
  FloatRect getCollisionRect() const;
  
  bool canCollideWithEntities() const { return false; }
  
private:
  float xpAmount;
};

class Bullet : public Moveable {
public:
  Bullet(const EntityPosition& position, const Vector2f& initialVelocity,
	 const Vector2f& dimensions, float damageValue);
  
  void update(ILevel* level, const float lastDelta);
  void onWorldCollision(COLLISION_PLANE worldCollisionType);
  void onEntityCollision(COLLISION_PLANE worldCollisionType, Entity* entity);
  
  FloatRect getCollisionRect() const;

private:
  int numbOfBouncesLeft = 0;
  float damageValue; 
};

class Item : public Entity {
 public:
  Item(const EntityPosition& position, const float value);
  void update(ILevel* level, const float lastDelta);
  
  FloatRect getCollisionRect() const;
  
  bool isPlayerItem() const { return true; }
  bool canCollideWithEntities() const { return false; }
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
  Mob(const EntityPosition& position, int level, int life = 1.0f);
  void addHealth(const float amount);
  const EntityRenderData& getRenderData();
  
  int getLevel() const { return level; }
  float getHealth() const { return health; }
  float getMaxHealth() const { return maxHealth; }
  
  float getDamageValue() const { return damageValue; }
  void spawnXp(ILevel* level, float xpToSpawn) const;
  
protected:
  int level = 0;
  
  float health = 3.0f;
  float maxHealth = 10.0f;

  float damageValue = 0;
};

class Cannon : public Mob {
public:
  Cannon(const EntityPosition& position, int level);
  void update(ILevel* level, const float lastDelta);
  void performDeathAction(ILevel* level);
  
private:
  float localTime = 0;
};

class Player : public Mob {
 public:
  Player(const EntityPosition& position);
  
  void update(ILevel* level, const float lastDelta);
  void onWorldCollision(COLLISION_PLANE worldCollisionType);
  void onEntityCollision(COLLISION_PLANE worldCollisionType, Entity* entity);
  
  void handlePlayerEvent(const PLAYER_EVENT playerEvent, ILevel* level);
  void performDeathAction(ILevel* level);
  
  bool canReceiveItems() const { return true; }
  bool isPlayer() const { return true; }
  
  void addXp(const float amount);
  void levelUp();
  
  float getShieldValue() const { return shieldValue; } 
  
  float getStamina() const { return stamina; }
  float getMaxStamina() const { return maxStamina; }
  
  float getXp() const { return xpAmount; }
  float getCurrentLevelXp() const { return (level - 1) * 100; } 
  float getNextLevelXp() const { return level * 100; }
  
  int getSkillPoints() const { return skillPointCount; } 
  
  // Events and Stuff
  
  EventNameList getEntityEvents();
  void onEvent(const std::string& eventName, EventArgumentDataMap eventDataMap);
  
private:
  float xpAmount = 0;
  float shieldValue = 0;

  float stamina = 100;
  float maxStamina = 100;

  int skillPointCount = 0;
  
  MOB_DIRECTION direction;
};
