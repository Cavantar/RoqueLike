#pragma once

#include "Vector.h"
#include "EntityPosition.h"
#include "ILevel.h"

#include <memory>

struct PlayerInput {
  bool up;
  bool right;
  bool down;
  bool left;

  bool actionUp;
  bool actionRight;
  bool actionDown;
  bool actionLeft;

  bool playerKey1;
  bool playerKey2;
  bool playerKey3;
  bool playerKey4;
  bool playerKey5;
};
  
enum MOB_DIRECTION{
  MOB_FACING_UP,
  MOB_FACING_RIGHT,
  MOB_FACING_DOWN,
  MOB_FACING_LEFT
};

enum ENTITY_RENDER_DATA_TYPE{
  ER_OVERLAYTEXT,
  ER_PRIMITIVE,
  ER_MOB,
  ER_BASICSPRITE
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
    
    // OverlayText
    struct{
      std::string text;
      float fontSize;
      Vector3f textColor;
      float textFadeValue;
    };

    struct{
      // Basic Sprite
      std::string basicSpriteName;
    };
  };
};

class Entity : public EventOperator {
public:
  Entity();
  Entity(EntityPosition position) : position(position) {};
  virtual ~Entity() {};

  // Important
  void setLevel(ILevel* level) { this->level = level; } 
  
  virtual void update(const float lastDelta) = 0;
  
  // Position / Movement
  const EntityPosition& getPosition() const { return position; }
  void setPosition(const EntityPosition& position) { this->position = position; }

  virtual Vector2f getVelocity() const  { return Vector2f(); }
  // Applies given velocity to an entity (Useful when pushing)
  virtual void addVelocity(Vector2f velocity) {}

  // For Ordering During Rendering 
  virtual Vector2f getDimensions() const { return Vector2f();}
  
  // Collision Stuff
  virtual FloatRect getCollisionRect() const { return FloatRect();}
  virtual EntityPosition getCollisionCenter() const { return position; }

  // Both are usually called by handleCollisionResults
  virtual void onWorldCollision(COLLISION_PLANE worldCollisionType) {}
  virtual void onEntityCollision(COLLISION_PLANE worldCollisionType, Entity* entity) {}

  // Is called when entity is removed from the map
  virtual void performDeathAction() {}
  
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
  // Hold pointer to the level it's on
  ILevel* level;
  EntityRenderData renderData;
  EntityPosition position;
  bool alive = true; 
};
typedef std::shared_ptr<Entity> EntityPtr;

struct OverlayTextData{
  std::string text;
  float fontSize;
  Vector3f color;
  float duration;
};

class OverlayText: public Entity {
public:
  OverlayText(const EntityPosition& position, const OverlayTextData& overlayTextData);
  void update(const float lastDelta);
  
private:
  OverlayTextData overlayTextData;
  float localTime;
};

class Moveable : public Entity {
 public:
  Vector2f getDimensions() const { return dimensions; }
  virtual FloatRect getCollisionRect() const ;
  
  Vector2f getVelocity() const { return velocity; }
  void addVelocity(Vector2f velocity) { this->velocity += velocity; }

  // Returns vector indicating center of collision region starting in reference to EntityPosition(top Left)  
  Vector2f getLocalCollisionCenter() const ;
  EntityPosition getCollisionCenter() const ;

  // Returns reflected velocity depending on collisionPlane
  Vector2f getReflectedVelocity(COLLISION_PLANE collisionPlane, float speedIncrease) const ;
  float getMovementSpeed() const { return metersPerSecondSquared; }

  float getBottomY() const { return dimensions.y; }
  
protected:
  Vector2f dimensions;
  
  Vector2f velocity;
  Vector2f acceleration;
  
  float metersPerSecondSquared = 25;

  // Calculates deltaVector based on the velocity acceleration and passed fakeFrictionValue
  Vector2f getPositionDeltaVector(const float lastDelta, const float fakeFrictionValue,
				  const float accelerationModifier = 1.0f);

  // Handles Collision Result calls onEntityCollision, onWorldCollision.
  void handleCollisionResult(EntityCollisionResult& collisionResult,
			     const Vector2f& positionDeltaVector);
};

class XpOrb : public Moveable {
public:
  XpOrb(const EntityPosition& position, const Vector2f& initialVelocity, float xpAmount);
  void update(const float lastDelta);
  
  bool isPlayerItem() const { return true; }
  void onWorldCollision(COLLISION_PLANE worldCollisionType);
  FloatRect getCollisionRect() const;
  bool canCollideWithEntities() const { return false; }
  
private:
  float xpAmount;
  float localTime;
};

class Bullet : public Moveable {
public:
  Bullet(const EntityPosition& position, const Vector2f& initialVelocity,
	 const Vector2f& dimensions, float damageValue);
  
  void update(const float lastDelta);
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
  void update(const float lastDelta);
  
  FloatRect getCollisionRect() const;
  
  bool isPlayerItem() const { return true; }
  bool canCollideWithEntities() const { return false; }
protected:
  float itemValue;
  
  virtual void performItemAction(Entity* entity) = 0;
};

class HealthItem : public Item{
public:
  HealthItem(const EntityPosition& position, const float value);  
  
private:
  void performItemAction(Entity* actionReceiver);
};

class Mob: public Moveable {
public:
  Mob(const EntityPosition& position, int level, int life = 1.0f);
  void addHealth(const float amount);
  const EntityRenderData& getRenderData();
  
  int getMobLevel() const { return mobLevel; }
  float getHealth() const { return health; }
  float getMaxHealth() const { return maxHealth; }
  
  float getDamageValue() const { return damageValue; }
  void spawnXp(int xpToSpawn) const;
  bool canGetHit() const { return true; }
  
protected:
  int mobLevel = 0;
  
  float health = 3.0f;
  float maxHealth = 10.0f;

  float damageValue = 0;
};

class Cannon : public Mob {
public:
  Cannon(const EntityPosition& position, int level);
  void update(const float lastDelta);
  void performDeathAction();
  
private:
  float localTime = 0;
};

class Follower : public Mob {
public:
  Follower(const EntityPosition& position, int level);
  void update(const float lastDelta);
  void performDeathAction();
  FloatRect getCollisionRect() const;
  
  void onWorldCollision(COLLISION_PLANE worldCollisionType);
  void onEntityCollision(COLLISION_PLANE worldCollisionType, Entity* entity);
};


enum PLAYER_UPGRADE {
  PU_HEALTH,
  PU_SHIELD,
  PU_MOVESPEED,
  PU_DAMAGE,
  PU_STAMINA,
  PU_BULLETSPEED
};

class Player : public Mob {
 public:
  Player(const EntityPosition& position);
  
  void update(const float lastDelta);
  void onWorldCollision(COLLISION_PLANE worldCollisionType);
  void onEntityCollision(COLLISION_PLANE worldCollisionType, Entity* entity);
  FloatRect getCollisionRect() const;
  
  // Reacts to the player Input state
  void handlePlayerInput(const PlayerInput& playerInput);
  void performDeathAction();
  
  bool canReceiveItems() const { return true; }
  bool isPlayer() const { return true; }
  
  void addXp(const float amount);
  float getShieldValue() const { return shieldValue; } 
  
  float getStamina() const { return stamina; }
  float getMaxStamina() const { return maxStamina; }
  
  float getXp() const { return xpAmount; }
  float getCurrentLevelXp() const { return (mobLevel - 1) * 100; } 
  float getNextLevelXp() const { return mobLevel * 100; }
  
  int getSkillPoints() const { return skillPointCount; } 
  
  // Events and Stuff
  EventNameList getEntityEvents();
  void onEvent(const std::string& eventName, EventArgumentDataMap eventDataMap);
  
private:
  float xpAmount = 0;
  float shieldValue = 0.5f;
  
  float bulletVelocity = 10.0f;
  
  float stamina = 100;
  float maxStamina = 100;

  int skillPointCount = 0;
  
  MOB_DIRECTION direction;

  void upgradeAbility(PLAYER_UPGRADE upgrade);
  void levelUp();
};
