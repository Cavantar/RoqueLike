#pragma once
#include "Entity.h"

class Mob: public Moveable {
public:
  Mob(const EntityPosition& position, int level, int life = 1.0f);
  void addHealth(float amount);
  
  int getMobLevel() const { return mobLevel; }
  float getHealth() const { return health; }
  float getMaxHealth() const { return maxHealth; }
  
  float getDamageValue() const { return damageValue; }
  float getShieldValue() const { return 0;} 
  void spawnXp(int xpToSpawn) const;
  bool canGetHit() const { return true; }

  const EntityRenderData* getRenderData();
  virtual void performDeathAction();
protected:
  int mobLevel = 0;
  
  float health = 3.0f;
  float maxHealth = 10.0f;

  float damageValue = 0;
  
  MobRenderData renderData;
};

class Cannon : public Mob {
public:
  Cannon(const EntityPosition& position, int level);
  void update(const float lastDelta);
  
private:
  float localTime = 0;
};

class Follower : public Mob {
public:
  Follower(const EntityPosition& position, int level);
  void update(const float lastDelta);
  FloatRect getCollisionRect() const;
  
  void onWorldCollision(COLLISION_PLANE worldCollisionType);
  void onEntityCollision(COLLISION_PLANE worldCollisionType, Entity* entity);
};

class Snake : public Mob {
public:
  Snake(const EntityPosition& position, int level);
  void update(const float lastDelta);
  FloatRect getCollisionRect() const;
  
  void onWorldCollision(COLLISION_PLANE worldCollisionType);
  void onEntityCollision(COLLISION_PLANE worldCollisionType, Entity* entity);
private:
  Vector2f currentDirection;
  const float attackSpeedValue = 50.0f;
  const float idleSpeedValue = 10.0f;
  float localAttackingTime;
};

enum RAT_STATE {
  RS_SNIFFING,
  RS_THINKING,
  RS_ATTACKING
};

class Rat : public Mob {
public:
  Rat(const EntityPosition& position, int level);
  void update(const float lastDelta);
  FloatRect getCollisionRect() const;
  
  void onWorldCollision(COLLISION_PLANE worldCollisionType);
  void onEntityCollision(COLLISION_PLANE worldCollisionType, Entity* entity);
private:
  RAT_STATE ratState; 
  Vector2f currentDirection;
  float localStateTime;
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
  float getBulletVelocity() const { return bulletVelocity; }
  
  int getSkillPoints() const { return skillPointCount; } 
  
  // Events and Stuff
  EventNameList getEntityEvents();
  void onEvent(const std::string& eventName, EventArgumentDataMap eventDataMap);
  
private:
  float xpAmount = 0;
  float shieldValue = 0.1f;
  
  float bulletVelocity = 10.0f;
  
  float stamina = 100;
  float maxStamina = 100;

  int skillPointCount = 0;
  
  MOB_DIRECTION direction;

  void upgradeAbility(PLAYER_UPGRADE upgrade);
  void levelUp();
};
