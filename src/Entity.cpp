#include "Entity.h"

#include <iostream>
#include <sstream>
#include <iomanip>

#include "Mobs.h"

Entity::Entity() : position(EntityPosition())
{
  position += Vec2f(2, 2);
}

void
Entity::die()
{
  alive = false;

  EventArgumentDataMap eventArgumentDataMap;
  eventArgumentDataMap["pointer"] = (void*)this;
  queueEvent("EntityRemoved", eventArgumentDataMap);
}

void
Entity::spawnDustParticles(const EntityPosition& position, int amount, float speed)
{
  for(int i = 0; i < amount; i++)
  {
    Entity* particle = new PrimitiveParticle(position, Vec2f::directionVector((rand()%16) * 22.5f) * speed,
					     Vec3f(102, 102, 102), 1.0f + ((rand()%10) * 0.1f));
    level->addEntity(EntityPtr(particle));
  }
}

void
Entity::spawnBloodParticles(const EntityPosition& position, int amount, float speed)
{
  for(int i = 0; i < amount; i++)
  {

    float realSpeed;
    Vec3f color;
    if(rand()%6)
    {
      color = Vec3f(138, 7, 7);
      realSpeed = speed / 2.0f;
    }
    else
    {
      color =  Vec3f(102, 102, 102);
      realSpeed = speed;
    }

    Entity* particle = new PrimitiveParticle(position, Vec2f::directionVector((rand()%16) * 22.5f) * realSpeed,
					     color, 1.0f + ((rand()%10) * 0.1f));
    level->addEntity(EntityPtr(particle));
  }
}

OverlayText::OverlayText(const EntityPosition& position, const OverlayTextData& overlayTextData) :
  Entity(position), overlayTextData(overlayTextData), localTime(0)
{
  renderData.type = ER_OVERLAYTEXT;

  renderData.text = overlayTextData.text;
  renderData.fontSize = overlayTextData.fontSize;
  renderData.textColor = overlayTextData.color;
  renderData.textFadeValue = 0;
}

void
OverlayText::update(const float lastDelta)
{
  localTime += lastDelta;
  if(localTime >= overlayTextData.duration)
  {
    die();
  }
  if(overlayTextData.text == "Leveled Up !")
  {
    static const Vec3f colorStart(102, 255, 0);
    static const Vec3f colorEnd(255, 222, 0);
    static const float numbOfColorCycles = 5.0f;

#if 1

    float temp = fmodf((localTime / overlayTextData.duration) * numbOfColorCycles, 1.0f);
    if(temp > 0.5f) temp = 1.0f - temp;
    renderData.textColor = Vec3f::lerp(colorStart, colorEnd, temp * 2.0f);

#else

    float temp = sin((localTime / overlayTextData.duration) * M_PI * numbOfColorCycles);
    temp = abs(temp);
    //std::cout << "temp: " << temp << std::endl;
    renderData.textColor = Vec3f::lerp(colorStart, colorEnd, temp);
#endif

  }

  float halfTime = overlayTextData.duration / 2.0f;
  if(localTime > halfTime)
  {
    renderData.textFadeValue = ((localTime - halfTime) / halfTime) * 255;
  }

  static const Vec2f textVelocity(0, -8.0f);
  position += textVelocity * lastDelta;
}

Vec2f
Moveable::getPositionDeltaVec(const float lastDelta, const float fakeFrictionValue,
			      const float accelerationModifier)
{

  Vec2f positionDeltaVec;

  if(acceleration.x != 0 && acceleration.y != 0)
  {
    acceleration *= 0.70710678118f;
  }
  acceleration *= metersPerSecondSquared * accelerationModifier;

  positionDeltaVec = acceleration * 0.5f * (lastDelta * lastDelta) + velocity * lastDelta;
  velocity += acceleration * lastDelta;

  const float clampValue = 5000;
  if(velocity.x > clampValue ) velocity.x = clampValue;
  if(velocity.x < -clampValue ) velocity.x = -clampValue;
  if(velocity.y > clampValue ) velocity.y = clampValue;
  if(velocity.y < -clampValue ) velocity.y = -clampValue;

  // If fakeFrictionValue Is 1.0f Than velocity will be reduced to 0 in 1 second
  velocity -= velocity * fakeFrictionValue * lastDelta;
  acceleration = Vec2f();
  acceleration = Vec2f();

  return positionDeltaVec;
}

FloatRect
Moveable::getCollisionRect() const
{
  static const float bottomPart = 0.3f;
  return FloatRect(0, dimensions.y *(1.0f - bottomPart), dimensions.x, dimensions.y * bottomPart);
}

Vec2f
Moveable::getLocalCollisionCenter() const
{
  FloatRect collisionRect = getCollisionRect();

  Vec2f localCenter = collisionRect[0];
  localCenter.x += collisionRect.width / 2.0f;
  localCenter.y += collisionRect.height / 2.0f;

  return localCenter;
}

EntityPosition
Moveable::getCollisionCenter() const
{
  return position + getLocalCollisionCenter();
}

Vec2f
Moveable::getReflectedVelocity(COLLISION_PLANE collisionPlane, float speedIncrease) const
{
  Vec2f reflectedVec = velocity;

  switch(collisionPlane) {
  case COLLISION_PLANE_VERTICAL:
    reflectedVec.x *= -speedIncrease;
    break;
  case COLLISION_PLANE_HORIZONTAL:
    reflectedVec.y *= -speedIncrease;
    break;
  }

  return reflectedVec;
}

void
Moveable::handleCollisionResult(EntityCollisionResult& collisionResult,
				const Vec2f& positionDeltaVec)
{
  if(collisionResult.maxAllowedT != 1.0f)
  {
    // Colliding With Entities
    if(collisionResult.collidedEntity != NULL)
    {
      collisionResult.collidedEntity->onEntityCollision(collisionResult.collisionPlane, this);
      onEntityCollision(collisionResult.collisionPlane, collisionResult.collidedEntity);
    }
    // Colliding With Tiles
    else
    {
      onWorldCollision(collisionResult.collisionPlane);
    }
  }
  // Moving Player By Allowed Value
  position += positionDeltaVec * collisionResult.maxAllowedT;
}

PrimitiveParticle::PrimitiveParticle(const EntityPosition& position, const Vec2f& initialVelocity,
				     const Vec3f& color, float lifeTime) :
  lifeTime(lifeTime)
{
  this->position = position;
  velocity = initialVelocity;

  float radius = 0.3f;
  dimensions = Vec2f(radius, radius);

  renderData.primitiveType = PT_CIRCLE;
  renderData.dimensionsInTiles = Vec2f(radius, radius);
  renderData.color = color;

  localTime = (float)(rand()%255) / 255.0f ;
}

FloatRect
PrimitiveParticle::getCollisionRect() const
{
  return FloatRect(0, 0, dimensions.x, dimensions.y);
}

void
PrimitiveParticle::update(const float lastDelta)
{
  localTime += lastDelta;
  if(localTime > lifeTime) die();

  Vec2f positionDeltaVec = getPositionDeltaVec(lastDelta, 0.5f);

  renderData.colorAlpha = 1.0f - (localTime / lifeTime);

  EntityCollisionResult collisionResult = level->checkCollisions(this, positionDeltaVec);
  handleCollisionResult(collisionResult, positionDeltaVec);
}

void
PrimitiveParticle::onWorldCollision(COLLISION_PLANE collisionPlane)
{
  velocity = getReflectedVelocity(collisionPlane, 1.0f);
}

XpOrb::XpOrb(const EntityPosition& position, const Vec2f& initialVelocity, float xpAmount)
{
  this->position = position;
  velocity = initialVelocity;
  this->xpAmount = xpAmount;

  float radius = 1.0f * (xpAmount / 70.0f);

  dimensions = Vec2f(radius, radius);

  renderData.type = ER_PRIMITIVE;

  renderData.primitiveType = PT_CIRCLE;
  renderData.dimensionsInTiles = Vec2f(radius, radius);
  renderData.color = Vec3f(102, 255, 0);
  renderData.colorAlpha = 1.0f;
  renderData.outlineThickness = 1.0f;

  localTime = (float)(rand()%255) / 255.0f ;
}

void
XpOrb::update(const float lastDelta)
{
  // Simulate Movement First
  localTime += lastDelta;

  static const Vec3f colorStart(102, 255, 0);
  static const Vec3f colorEnd(255, 222, 0);

  static const float period = 1.0f;
  float temp = fmodf(localTime / period, 1.0f);
  if(temp > 0.5f) temp = 1.0f - temp;
  renderData.color = Vec3f::lerp(colorStart, colorEnd, temp * 2.0f);

  Player* player = level->getPlayer();
  if(player)
  {
    // Getting Direction To Accelerate Towards
    EntityPosition playerPosition = player->getCollisionCenter();
    Vec2f distanceVec = EntityPosition::calculateDistanceInTiles(position, playerPosition,
								       level->getTileMap()->getTileChunkSize());
    float distance = distanceVec.getLength();
    if(distance < 15)
    {
      distanceVec.normalize();
      acceleration = distanceVec;
    }

    // Checking Actual Collisions
    if(distance < 3)
    {
      distanceVec = EntityPosition::calculateDistanceInTiles(position, player->getPosition(),
								level->getTileMap()->getTileChunkSize());


      FloatRect playerCollisionRect = player->getCollisionRect();
      playerCollisionRect += distanceVec;

      FloatRect orbCollisionRect = getCollisionRect();
      if(orbCollisionRect.doesRectCollideWith(playerCollisionRect))
      {
	player->addXp(xpAmount);
	die();
      }
    }
  }

  Vec2f positionDeltaVec = getPositionDeltaVec(lastDelta, 0.5f);

  EntityCollisionResult collisionResult = level->checkCollisions(this, positionDeltaVec);
  handleCollisionResult(collisionResult, positionDeltaVec);
}

void
XpOrb::onWorldCollision(COLLISION_PLANE collisionPlane)
{
  static const float speedIncrease = 0.5f;

  velocity = getReflectedVelocity(collisionPlane, speedIncrease);
}

FloatRect
XpOrb::getCollisionRect() const
{
  return FloatRect(0, 0, dimensions.x, dimensions.y );
}

Bullet::Bullet(const EntityPosition& position, const Vec2f& initialVelocity,
	       const Vec2f& dimensions, float damageValue)
{

  this->dimensions = dimensions;
  this->position = position;
  velocity = initialVelocity;
  numbOfBouncesLeft = 2;

  this->damageValue = damageValue;

  renderData.primitiveType = PT_CIRCLE;
  renderData.dimensionsInTiles = dimensions;
  renderData.color = Vec3f(rand()%256, rand()%256, rand()%256);
  renderData.colorAlpha = 1.0f;
}

void
Bullet::update(const float lastDelta)
{

  Vec2f positionDeltaVec = getPositionDeltaVec(lastDelta, 0.001f);

  if(velocity.getLength() < 1.0f) die();

  EntityCollisionResult collisionResult = level->checkCollisions(this, positionDeltaVec);
  handleCollisionResult(collisionResult, positionDeltaVec);
}

void
Bullet::onWorldCollision(COLLISION_PLANE collisionPlane)
{
  if(numbOfBouncesLeft-- == 0)
  {
    spawnDustParticles(getCollisionCenter(), 10, velocity.getLength() / 4.0f);
    die();
  }

  static const float speedIncrease = 1.0f;
  velocity = getReflectedVelocity(collisionPlane, speedIncrease);
}

FloatRect
Bullet::getCollisionRect() const
{

  static const float bottomPart = 1.0f;
  return FloatRect(0, dimensions.y *(1.0f - bottomPart), dimensions.x, dimensions.y * bottomPart);
}

void
Bullet::onEntityCollision(COLLISION_PLANE collisionPlane, Entity* entity)
{
  static const float speedIncrease = 1.0f;

  //std::cout << "Died with the entity\n";

  entity->addHealth(-damageValue);
  entity->addVelocity(velocity * 0.5f);

  velocity = getReflectedVelocity(collisionPlane, speedIncrease);

  spawnBloodParticles(entity->getCollisionCenter(), 10, velocity.getLength() / 4.0f);
  die();
}

Item::Item(const EntityPosition& position, const float value)
{
  this->position = position;
  itemValue = value;

  renderData.type = ER_BASICSPRITE;
}

void
Item::update(const float lastDelta)
{
  Player* player = level->getPlayer();
  if(player)
  {
    EntityPosition playerPosition = player->getPosition();
    Vec2f distanceVec = EntityPosition::calculateDistanceInTiles(position, playerPosition,
								 level->getTileMap()->getTileChunkSize());

    FloatRect playerCollisionRect = player->getCollisionRect();
    playerCollisionRect += distanceVec;

    // If Player is close checking collisions
    float distance = distanceVec.getLength();
    if(distance < 3)
    {
      FloatRect orbCollisionRect = getCollisionRect();
      if(orbCollisionRect.doesRectCollideWith(playerCollisionRect))
      {
	performItemAction(player);
	die();
      }
    }
  }
}

FloatRect
Item::getCollisionRect() const
{
  return FloatRect(0, 0, 1.0f, 1.0f);
}

HealthItem::HealthItem(const EntityPosition& position, const float value) : Item(position, value)
{
  renderData.basicSpriteName = "healthPotion";
}

void
HealthItem::performItemAction(Entity* actionReceiver)
{
  actionReceiver->addHealth(itemValue);
}
