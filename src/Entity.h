#pragma once

#include <jpb/Vector.h>
#include <jpb/Rect.h>
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
  bool playerKey6;
};

enum MOB_DIRECTION{
  MOB_FACING_UP,
  MOB_FACING_RIGHT,
  MOB_FACING_DOWN,
  MOB_FACING_LEFT
};

enum ENTITY_RENDER_DATA_TYPE{
  ER_PRIMITIVE,
  ER_OVERLAYTEXT,
  ER_MOB,
  ER_BASICSPRITE
};

enum PRIMITIVE_TYPE{
  PT_RECTANGLE,
  PT_CIRCLE
};

class EntityRenderData {
public:
  EntityRenderData(ENTITY_RENDER_DATA_TYPE type) : type(type) {}
  virtual ~EntityRenderData() {};

  ENTITY_RENDER_DATA_TYPE type;
};

class PrimitiveRenderData : public EntityRenderData {
public:
  PrimitiveRenderData() : EntityRenderData(ER_PRIMITIVE), colorAlpha(1.0f), outlineThickness(0) {}

  PRIMITIVE_TYPE primitiveType;
  Vec2f dimensionsInTiles;
  Vec3f color;
  float colorAlpha;
  float outlineThickness;
};

class MobRenderData : public EntityRenderData {
public:
  MobRenderData() : EntityRenderData(ER_MOB), spriteColorAlpha(1.0f) {}
  std::string caption;
  float life;

  std::string spriteName;
  Vec3f spriteColor;
  float spriteColorAlpha;
};

class OverlayTextRenderData : public EntityRenderData {
public:
  OverlayTextRenderData() : EntityRenderData(ER_OVERLAYTEXT) {}

  std::string text;
  float fontSize;
  Vec3f textColor;
  float textFadeValue;
};

class BasicSpriteRenderData : public EntityRenderData {
public:

  BasicSpriteRenderData() : EntityRenderData(ER_BASICSPRITE) {}
  std::string basicSpriteName;
  // Vec3f color;
  // float colorAlpha;
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

  virtual Vec2f getVelocity() const  { return Vec2f(); }
  // Applies given velocity to an entity (Useful when pushing)
  virtual void addVelocity(Vec2f velocity) {}

  // For Ordering During Rendering
  virtual Vec2f getDimensions() const { return Vec2f(1.0f, 1.0f);}

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
  virtual void addHealth(float amount) {}

  // Flag Stuff
  virtual bool canReceiveItems() const { return false; }
  virtual bool isPlayerItem() const { return false; }
  virtual bool isPlayer() const { return false; }
  virtual bool canCollideWithEntities() const { return true; }

  virtual const EntityRenderData* getRenderData() = 0;
  virtual bool isAlive() { return alive; }
  void die();

  void spawnDustParticles(const EntityPosition& position, int amount, float speed);
  void spawnBloodParticles(const EntityPosition& position, int amount, float speed);

protected:
  // Hold pointer to the level it's on
  ILevel* level;
  EntityPosition position;
  bool alive = true;
};
typedef std::shared_ptr<Entity> EntityPtr;

struct OverlayTextData{
  std::string text;
  float fontSize;
  Vec3f color;
  float duration;
};

class OverlayText: public Entity {
public:
  OverlayText(const EntityPosition& position, const OverlayTextData& overlayTextData);
  void update(const float lastDelta);

  const EntityRenderData* getRenderData() { return &renderData; }
private:
  OverlayTextData overlayTextData;
  OverlayTextRenderData renderData;
  float localTime;
};

class Moveable : public Entity {
public:
  Vec2f getDimensions() const { return dimensions; }
  virtual FloatRect getCollisionRect() const ;

  Vec2f getVelocity() const { return velocity; }
  void addVelocity(Vec2f velocity) { this->velocity += velocity; }

  // Returns vector indicating center of collision region starting in reference to EntityPosition(top Left)
  Vec2f getLocalCollisionCenter() const ;
  EntityPosition getCollisionCenter() const ;

  // Returns reflected velocity depending on collisionPlane
  Vec2f getReflectedVelocity(COLLISION_PLANE collisionPlane, float speedIncrease) const ;
  float getMovementSpeed() const { return metersPerSecondSquared; }

  float getBottomY() const { return dimensions.y; }

protected:
  Vec2f dimensions;

  Vec2f velocity;
  Vec2f acceleration;

  float metersPerSecondSquared = 25;

  // Calculates deltaVec based on the velocity acceleration and passed fakeFrictionValue
  Vec2f getPositionDeltaVec(const float lastDelta, const float fakeFrictionValue,
				  const float accelerationModifier = 1.0f);

  // Handles Collision Result calls onEntityCollision, onWorldCollision.
  void handleCollisionResult(EntityCollisionResult& collisionResult,
			     const Vec2f& positionDeltaVec);
};

class PrimitiveParticle : public Moveable {
public:
  PrimitiveParticle(const EntityPosition& position, const Vec2f& initialVelocity,
		    const Vec3f& color, float lifeTime);

  void update(const float lastDelta);

  void onWorldCollision(COLLISION_PLANE worldCollisionType);
  FloatRect getCollisionRect() const;
  bool canCollideWithEntities() const { return false; }

  const EntityRenderData* getRenderData() { return &renderData;}
private:
  PrimitiveRenderData renderData;

  float localTime;
  const float lifeTime;
};

class XpOrb : public Moveable {
public:
  XpOrb(const EntityPosition& position, const Vec2f& initialVelocity, float xpAmount);
  void update(const float lastDelta);

  bool isPlayerItem() const { return true; }
  void onWorldCollision(COLLISION_PLANE worldCollisionType);
  FloatRect getCollisionRect() const;
  bool canCollideWithEntities() const { return false; }

  const EntityRenderData* getRenderData() { return &renderData;}
private:
  PrimitiveRenderData renderData;

  float xpAmount;
  float localTime;
};

class Bullet : public Moveable {
public:
  Bullet(const EntityPosition& position, const Vec2f& initialVelocity,
	 const Vec2f& dimensions, float damageValue);

  void update(const float lastDelta);
  void onWorldCollision(COLLISION_PLANE worldCollisionType);
  void onEntityCollision(COLLISION_PLANE worldCollisionType, Entity* entity);

  FloatRect getCollisionRect() const;

  const EntityRenderData* getRenderData() { return &renderData;}
private:
  PrimitiveRenderData renderData;

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
  const EntityRenderData* getRenderData() { return &renderData;}
protected:
  float itemValue;
  BasicSpriteRenderData renderData;
  virtual void performItemAction(Entity* entity) = 0;
};

class HealthItem : public Item{
public:
  HealthItem(const EntityPosition& position, const float value);

private:
  void performItemAction(Entity* actionReceiver);
};
