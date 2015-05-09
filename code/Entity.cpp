#include "Entity.h"
#include <iostream>
#include <sstream>

Entity::Entity() : position(EntityPosition())   
{
  position += Vector2f(2, 2);
}

void Entity::die()
{
  alive = false;
  
  EventArgumentDataMap eventArgumentDataMap;
  eventArgumentDataMap["pointer"] = (void*)this;
  queueEvent("EntityRemoved", eventArgumentDataMap);
}

Vector2f Moveable::getPositionDeltaVector(const float lastDelta, const float fakeFrictionValue,
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
  
  // If fakeFrictionValue Is 1.0f Than velocity will be reduced to 0 in 1 second  
  velocity -= velocity * fakeFrictionValue * lastDelta;
  acceleration = Vector2f();

  return positionDeltaVector;
}

FloatRect Moveable::getCollisionRect() const
{
  static const float bottomPart = 0.3f;
  return FloatRect(0, dimensions.y *(1.0f - bottomPart), dimensions.x, dimensions.y * bottomPart);
}

Vector2f Moveable::getLocalCollisionCenter() const
{
  FloatRect collisionRect = getCollisionRect();

  Vector2f localCenter = collisionRect[0];
  localCenter.x += collisionRect.width / 2.0f;
  localCenter.y += collisionRect.height / 2.0f;

  return localCenter;
}

EntityPosition Moveable::getCollisionCenter() const
{
  return position + getLocalCollisionCenter(); 
}

void Moveable::handleCollisionResult(EntityCollisionResult& collisionResult,
				     const Vector2f& positionDeltaVector)
{
  if(collisionResult.maxAllowedT != 1.0f)
  {
    if(collisionResult.collidedEntity != NULL)
    {
      onEntityCollision(collisionResult.collisionPlane, collisionResult.collidedEntity);
    }
    else
    {
      onWorldCollision(collisionResult.collisionPlane);
    }
  }
  position += positionDeltaVector * collisionResult.maxAllowedT;
}

XpOrb::XpOrb(const EntityPosition& position, const Vector2f& initialVelocity, float xpAmount)
{
  this->position = position;
  velocity = initialVelocity;
  this->xpAmount = xpAmount;
  
  float radius = 1.0f * (xpAmount / 50.0f);
  
  dimensions = Vector2f(radius, radius);
  
  renderData.type = ER_PRIMITIVE;
  
  renderData.primitiveType = PT_CIRCLE;
  renderData.dimensionsInTiles = Vector2f(radius, radius);
  renderData.color = Vector3f(102 + rand()%20, 255 - rand()%30, 0);
}

void XpOrb::update(Level& level, const float lastDelta)
{
  // Simulate Movement First
  
  Player* player = level.getPlayer();
  if(player)
  {

    // Getting Direction Accelerate Towards
    EntityPosition playerPosition = player->getCollisionCenter();
    Vector2f distanceVector = EntityPosition::calculateDistanceInTiles(position, playerPosition,
								       level.getTileMap()->getTileChunkSize());
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
								level.getTileMap()->getTileChunkSize());
      
    
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
  
  EntityCollisionResult collisionResult = level.checkCollisions(this, positionDeltaVector);
  handleCollisionResult(collisionResult, positionDeltaVector);
  
}

void XpOrb::onWorldCollision(COLLISION_PLANE worldCollisionType)
{
  static const float speedIncrease = 1.0f;

  switch(worldCollisionType) {
  case COLLISION_PLANE_VERTICAL:
    velocity.x *= -speedIncrease;
    break;
  case COLLISION_PLANE_HORIZONTAL:
    velocity.y *= -speedIncrease;
    break;
  }
}

FloatRect XpOrb::getCollisionRect() const
{
  return FloatRect(0, 0, dimensions.x, dimensions.y );
}

Bullet::Bullet(const EntityPosition& position, const Vector2f& initialVelocity,
	       const Vector2f& dimensions, float damageValue)
{
  
  this->dimensions = dimensions;
  this->position = position;
  velocity = initialVelocity;

  this->damageValue = damageValue; 
  
  renderData.type = ER_PRIMITIVE;
  
  renderData.primitiveType = PT_CIRCLE;
  renderData.dimensionsInTiles = dimensions;
  renderData.color = Vector3f(rand()%256, rand()%256, rand()%256);
}

void Bullet::update(Level& level, const float lastDelta)
{
  
  Vector2f positionDeltaVector = getPositionDeltaVector(lastDelta, 0.001f);
  
  if(velocity.getLength() < 1.0f) die();
  
  EntityCollisionResult collisionResult = level.checkCollisions(this, positionDeltaVector);
  handleCollisionResult(collisionResult, positionDeltaVector);
}

void Bullet::onWorldCollision(COLLISION_PLANE worldCollisionType)
{
  if(numbOfBouncesLeft-- == 0) die();
  
  static const float speedIncrease = 1.0f;

  switch(worldCollisionType) {
  case COLLISION_PLANE_VERTICAL:
    velocity.x *= -speedIncrease;
    break;
  case COLLISION_PLANE_HORIZONTAL:
    velocity.y *= -speedIncrease;
    break;
  }
}

FloatRect Bullet::getCollisionRect() const
{
  static const float bottomPart = 1.0f;
  return FloatRect(0, dimensions.y *(1.0f - bottomPart), dimensions.x, dimensions.y * bottomPart);
}

void Bullet::onEntityCollision(COLLISION_PLANE worldCollisionType, Entity* entity)
{
  static const float speedIncrease = 1.0f;
  
  switch(worldCollisionType) {
  case COLLISION_PLANE_VERTICAL:
    velocity.x *= -speedIncrease;
    break;
  case COLLISION_PLANE_HORIZONTAL:
    velocity.y *= -speedIncrease;
    break;
  }
  
  entity->addLife(-damageValue);
  die();
}

Item::Item(const EntityPosition& position, const float value)
{
  this->position = position;
  itemValue = value;
  
  renderData.type = ER_PRIMITIVE;
  
  renderData.primitiveType = PT_RECTANGLE;
  renderData.dimensionsInTiles = Vector2f(1.0f, 1.0f);
  renderData.color = Vector3f(255.0f, 0, 0);
}

void Item::update(Level& level, const float lastDelta)
{
  Player* player = level.getPlayer();
  if(player)
  {
    EntityPosition playerPosition = player->getPosition();
    Vector2f distanceVector = EntityPosition::calculateDistanceInTiles(position, playerPosition,
								       level.getTileMap()->getTileChunkSize());
    
    FloatRect playerCollisionRect = player->getCollisionRect();
    playerCollisionRect += distanceVector;
    
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

FloatRect Item::getCollisionRect() const
{
  return FloatRect(0, 0, 1.0f, 1.0f);
}
void HealthItem::performItemAction(Entity* actionReceiver)
{
  actionReceiver->addLife(itemValue);
}

Mob::Mob(const EntityPosition& position, int level, int health) : level(level), health(health)
{
  this->position = position;
  renderData.type = ER_MOB;
}

void Mob::addLife(const float amount)
{
  health += amount;
  if(health > 1.0f) health = 1.0f;
  else if(health < 0) die();
}

const EntityRenderData& Mob::getRenderData()
{
  renderData.life = health / maxHealth;
  return renderData;
}

Cannon::Cannon(const EntityPosition& position, int level) : Mob(position, level)
{
  dimensions = Vector2f(1.0f, 1.0f);
  renderData.spriteName = "Cannon";
  
  std::stringstream caption;
  caption << "Cannon lvl: " << level;  
  renderData.caption = caption.str();
}

void Cannon::update(Level& level, const float lastDelta)
{
  float shootPeriod = 5.0f / this->level;
  localTime += lastDelta;

  if(localTime >= shootPeriod)
  {
    localTime = fmodf(localTime, shootPeriod);
    Player* player = level.getPlayer();

    if(player)
    {
      EntityPosition playerPosition = player->getPosition();
      Vector2f distanceVector = EntityPosition::calculateDistanceInTiles(position, playerPosition,
									 level.getTileMap()->getTileChunkSize());
      
      Vector2f directionVector;
      float bulletRadius = 0.7f + (this->level) / 10;
      
      if(distanceVector.getLength() < 15.0f)
      {
	distanceVector.normalize();
	directionVector = distanceVector;
	
	Entity* bullet;
	bullet = new Bullet(position + dimensions/2.0f + directionVector * 2.0f,
			    directionVector * 10.0f,
			    Vector2f(bulletRadius, bulletRadius),
			    (this->level + 1.0f) / 5.0f);
	
	level.addEntity(EntityPtr(bullet));
      }
      else
      {
	float randomDirection = ((3.14f) / 180.0f) * (rand() % 360);
	directionVector = Vector2f(cos(randomDirection), -sin(randomDirection));
      }
    }
  } 
}

void Cannon::performDeathAction(Level& level)
{
  float xpToSpawn = (this->level) * 20;

  while(xpToSpawn)
  {
    float value = -1;
    while(value > xpToSpawn || value < 0) value = ((rand() % 10) + 1) * 10;
    
    Entity* entity = new XpOrb(position, Vector2f::directionVector(), value);
    level.addEntity(EntityPtr(entity));
    
    xpToSpawn -= value;
  }
}

Player::Player(const EntityPosition& position) : Mob(position, 0.1f)
{
  dimensions = Vector2f(0.8f, 2.0f);
  renderData.spriteName = "Player";
  renderData.caption = "Player";
}

void Player::update(Level& level, const float lastDelta)
{
  EntityPosition collisionCenter = getCollisionCenter();
  float friction = level.getFrictionValueAtPosition(collisionCenter);
  float accelerationModifier  = level.getAccelerationModifierAtPosition(collisionCenter);
  
  Vector2f positionDeltaVector = getPositionDeltaVector(lastDelta, friction, accelerationModifier);
  
  EntityCollisionResult collisionResult = level.checkCollisions(this, positionDeltaVector);
  handleCollisionResult(collisionResult, positionDeltaVector);
}

void Player::onWorldCollision(COLLISION_PLANE worldCollisionType)
{
  const float bounceFactor = 0.5f;
  
  switch(worldCollisionType) {
  case COLLISION_PLANE_VERTICAL:
    velocity.x *= -0.5f;
    break;
  case COLLISION_PLANE_HORIZONTAL:
    velocity.y *= -0.5f;
    break;
  case COLLISION_PLANE_BOTH:
    velocity = Vector2f();
    break;
  }
}

void Player::onEntityCollision(COLLISION_PLANE worldCollisionType, Entity* entity)
{
  static const float speedIncrease = 1.05f;
  entity->addVelocity(velocity * 0.01f);
  
  
  switch(worldCollisionType) {
  case COLLISION_PLANE_VERTICAL:
    velocity.x *= -speedIncrease;
    break;
  case COLLISION_PLANE_HORIZONTAL:
    velocity.y *= -speedIncrease;
    break;
  }    
  
}

void Player::addXp(const float amount)
{
  xpAmount += amount;
  std::cout << "Added: " << amount << " xp\n";
  std::cout << "CurrentXp: " << xpAmount << " xp\n";
}

EventNameList Player::getEntityEvents()
{
  EventNameList eventNameList;
  eventNameList.push_back("HelloThere");
  eventNameList.push_back("Player");
  return eventNameList;
}

void Player::handlePlayerEvent(const PLAYER_EVENT playerEvent, Level& level)
{
  switch(playerEvent)
  {
  case PLAYER_MOVE_LEFT:
    {
      direction = MOB_FACING_LEFT;
      acceleration += Vector2f(-1.0f, 0);
    }break;
  case PLAYER_MOVE_RIGHT:
    {
      direction = MOB_FACING_RIGHT;
      acceleration += Vector2f(1.0f, 0);
    }break;
  case PLAYER_MOVE_UP:
    {
      direction = MOB_FACING_UP;
      acceleration += Vector2f(0, -1.0f);
    }break;
  case PLAYER_MOVE_DOWN:
    {
      direction = MOB_FACING_DOWN;
      acceleration += Vector2f(0, 1.0f);
    }break;
    
  case PLAYER_SHOOT_UP: 
  case PLAYER_SHOOT_RIGHT:
  case PLAYER_SHOOT_DOWN: 
  case PLAYER_SHOOT_LEFT: 
    {
      static const float bulletVelocity = 10.0f;
      float bulletRadius = 0.5f; 
    
      static const float bulletDistance = 3.0f;
      
      Vector2f tempDirectionVector;
      switch(playerEvent)
      {
      case PLAYER_SHOOT_UP: tempDirectionVector = Vector2f(0, -1.0f);
	break;
      case PLAYER_SHOOT_RIGHT: tempDirectionVector = Vector2f(1.0f, 0);
	break;
      case PLAYER_SHOOT_DOWN: tempDirectionVector = Vector2f(0, 1.0f);
	break;
      case PLAYER_SHOOT_LEFT: tempDirectionVector = Vector2f(-1.0f, 0);
	break;
      }

      Entity* bullet;
      
      bullet = new Bullet(position + Vector2f(getLocalCollisionCenter().x, 0) +
			  tempDirectionVector * bulletDistance,
			  velocity + tempDirectionVector * bulletVelocity,
			  Vector2f(bulletRadius, bulletRadius),
			  damageValue);
      
      level.addEntity(EntityPtr(bullet));
      
    }break;
  }
}

void Player::onEvent(const std::string& eventName, EventArgumentDataMap eventDataMap)
{
  if(eventName == "HelloThere")
  {
      
    std::cout << eventName << ": " << eventDataMap["text"].asString() << " " <<
      eventDataMap["number"].asInt() << std::endl;
  
    WorldPosition worldPos = eventDataMap["position"].asWorldPosition();
    std::cout << worldPos.tilePosition.x << std::endl;
  }
}
