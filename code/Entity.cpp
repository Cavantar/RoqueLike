#include "Entity.h"

#include <iostream>
#include <sstream>
#include <iomanip>

#include "Mobs.h"

Entity::Entity() : position(EntityPosition())   
{
  position += Vector2f(2, 2);
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
    Entity* particle = new PrimitiveParticle(position, Vector2f::directionVector((rand()%16) * 22.5f) * speed,
					     Vector3f(102, 102, 102), 1.0f + ((rand()%10) * 0.1f));
    level->addEntity(EntityPtr(particle));
  }
}

void
Entity::spawnBloodParticles(const EntityPosition& position, int amount, float speed)
{
  for(int i = 0; i < amount; i++)
  {
    
    float realSpeed;
    Vector3f color;
    if(rand()%6)
    {
      color = Vector3f(138, 7, 7);
      realSpeed = speed / 2.0f; 
    }
    else
    {
      color =  Vector3f(102, 102, 102);
      realSpeed = speed; 
    }
    
    Entity* particle = new PrimitiveParticle(position, Vector2f::directionVector((rand()%16) * 22.5f) * realSpeed,
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
    static const Vector3f colorStart(102, 255, 0);
    static const Vector3f colorEnd(255, 222, 0);
    static const float numbOfColorCycles = 5.0f;
    
    float temp = fmodf((localTime / overlayTextData.duration) * numbOfColorCycles, 1.0f);
    if(temp > 0.5f) temp = 1.0f - temp;
    renderData.textColor = Vector3f::interpolate(colorStart, colorEnd, temp * 2.0f);
  }

  float halfTime = overlayTextData.duration / 2.0f;
  if(localTime > halfTime)
  {
    renderData.textFadeValue = ((localTime - halfTime) / halfTime) * 255;
  }
  
  static const Vector2f textVelocity(0, -8.0f);
  position += textVelocity * lastDelta;
}

Vector2f
Moveable::getPositionDeltaVector(const float lastDelta, const float fakeFrictionValue,
				 const float accelerationModifier)
{
  
  Vector2f positionDeltaVector;
  
  if(acceleration.x != 0 && acceleration.y != 0)
  {
    acceleration *= 0.70710678118f;
  }
  acceleration *= metersPerSecondSquared * accelerationModifier;
  
  positionDeltaVector = acceleration * 0.5f * (lastDelta * lastDelta) + velocity * lastDelta;
  velocity += acceleration * lastDelta;
  
  const float clampValue = 5000;
  if(velocity.x > clampValue ) velocity.x = clampValue;
  if(velocity.x < -clampValue ) velocity.x = -clampValue;
  if(velocity.y > clampValue ) velocity.y = clampValue;
  if(velocity.y < -clampValue ) velocity.y = -clampValue;
  
  // If fakeFrictionValue Is 1.0f Than velocity will be reduced to 0 in 1 second  
  velocity -= velocity * fakeFrictionValue * lastDelta;
  acceleration = Vector2f();
  acceleration = Vector2f();
  
  return positionDeltaVector;
}

FloatRect
Moveable::getCollisionRect() const
{
  static const float bottomPart = 0.3f;
  return FloatRect(0, dimensions.y *(1.0f - bottomPart), dimensions.x, dimensions.y * bottomPart);
}

Vector2f
Moveable::getLocalCollisionCenter() const
{
  FloatRect collisionRect = getCollisionRect();

  Vector2f localCenter = collisionRect[0];
  localCenter.x += collisionRect.width / 2.0f;
  localCenter.y += collisionRect.height / 2.0f;

  return localCenter;
}

EntityPosition
Moveable::getCollisionCenter() const
{
  return position + getLocalCollisionCenter(); 
}

Vector2f
Moveable::getReflectedVelocity(COLLISION_PLANE collisionPlane, float speedIncrease) const 
{
  Vector2f reflectedVector = velocity;
  
  switch(collisionPlane) {
  case COLLISION_PLANE_VERTICAL:
    reflectedVector.x *= -speedIncrease;
    break;
  case COLLISION_PLANE_HORIZONTAL:
    reflectedVector.y *= -speedIncrease;
    break;
  }
  
  return reflectedVector;
}

void
Moveable::handleCollisionResult(EntityCollisionResult& collisionResult,
				const Vector2f& positionDeltaVector)
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
  position += positionDeltaVector * collisionResult.maxAllowedT;
}

PrimitiveParticle::PrimitiveParticle(const EntityPosition& position, const Vector2f& initialVelocity,
				     const Vector3f& color, float lifeTime) :
  lifeTime(lifeTime)
{
  this->position = position;
  velocity = initialVelocity;
  
  float radius = 0.3f;  
  dimensions = Vector2f(radius, radius);
  
  renderData.primitiveType = PT_CIRCLE;
  renderData.dimensionsInTiles = Vector2f(radius, radius);
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
  
  Vector2f positionDeltaVector = getPositionDeltaVector(lastDelta, 0.5f);
  
  renderData.colorAlpha = 1.0f - (localTime / lifeTime);
  
  EntityCollisionResult collisionResult = level->checkCollisions(this, positionDeltaVector);
  handleCollisionResult(collisionResult, positionDeltaVector);
}

void
PrimitiveParticle::onWorldCollision(COLLISION_PLANE collisionPlane)
{
  velocity = getReflectedVelocity(collisionPlane, 1.0f);
}

XpOrb::XpOrb(const EntityPosition& position, const Vector2f& initialVelocity, float xpAmount)
{
  this->position = position;
  velocity = initialVelocity;
  this->xpAmount = xpAmount;
  
  float radius = 1.0f * (xpAmount / 70.0f);
  
  dimensions = Vector2f(radius, radius);
  
  renderData.type = ER_PRIMITIVE;
  
  renderData.primitiveType = PT_CIRCLE;
  renderData.dimensionsInTiles = Vector2f(radius, radius);
  renderData.color = Vector3f(102, 255, 0);
  renderData.colorAlpha = 1.0f;
  renderData.outlineThickness = 1.0f;

  localTime = (float)(rand()%255) / 255.0f ;
}

void
XpOrb::update(const float lastDelta)
{
  // Simulate Movement First
  localTime += lastDelta;

  static const Vector3f colorStart(102, 255, 0);
  static const Vector3f colorEnd(255, 222, 0);

  static const float period = 1.0f;
  float temp = fmodf(localTime / period, 1.0f);
  if(temp > 0.5f) temp = 1.0f - temp;
  renderData.color = Vector3f::interpolate(colorStart, colorEnd, temp * 2.0f);
  
  Player* player = level->getPlayer();
  if(player)
  {
    // Getting Direction To Accelerate Towards
    EntityPosition playerPosition = player->getCollisionCenter();
    Vector2f distanceVector = EntityPosition::calculateDistanceInTiles(position, playerPosition,
								       level->getTileMap()->getTileChunkSize());
    float distance = distanceVector.getLength();
    if(distance < 15)
    {
      distanceVector.normalize();
      acceleration = distanceVector;
    }

    // Checking Actual Collisions
    if(distance < 3)
    {
      distanceVector = EntityPosition::calculateDistanceInTiles(position, player->getPosition(),
								level->getTileMap()->getTileChunkSize());
      
    
      FloatRect playerCollisionRect = player->getCollisionRect();
      playerCollisionRect += distanceVector;
      
      FloatRect orbCollisionRect = getCollisionRect();
      if(orbCollisionRect.doesRectCollideWith(playerCollisionRect))
      {
	player->addXp(xpAmount);
	die();
      }
    }
  }
  
  Vector2f positionDeltaVector = getPositionDeltaVector(lastDelta, 0.5f);
  
  EntityCollisionResult collisionResult = level->checkCollisions(this, positionDeltaVector);
  handleCollisionResult(collisionResult, positionDeltaVector);
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

Bullet::Bullet(const EntityPosition& position, const Vector2f& initialVelocity,
	       const Vector2f& dimensions, float damageValue)
{
  
  this->dimensions = dimensions;
  this->position = position;
  velocity = initialVelocity;
  //numbOfBouncesLeft = 2;
  
  this->damageValue = damageValue; 
  
  renderData.primitiveType = PT_CIRCLE;
  renderData.dimensionsInTiles = dimensions;
  renderData.color = Vector3f(rand()%256, rand()%256, rand()%256);
  renderData.colorAlpha = 1.0f;
}

void
Bullet::update(const float lastDelta)
{
  
  Vector2f positionDeltaVector = getPositionDeltaVector(lastDelta, 0.001f);
  
  if(velocity.getLength() < 1.0f) die();
  
  EntityCollisionResult collisionResult = level->checkCollisions(this, positionDeltaVector);
  handleCollisionResult(collisionResult, positionDeltaVector);
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
    Vector2f distanceVector = EntityPosition::calculateDistanceInTiles(position, playerPosition,
								       level->getTileMap()->getTileChunkSize());
    
    FloatRect playerCollisionRect = player->getCollisionRect();
    playerCollisionRect += distanceVector;

    // If Player is close checking collisions
    float distance = distanceVector.getLength();
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
