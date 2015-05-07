#include "Entity.h"
#include <iostream>

Entity::Entity() : position(EntityPosition())   
{
  position += Vector2f(2, 2);
}

void Entity::update(const float lastDelta)
{
  
}

void Entity::die()
{
  alive = false;
  
  EventArgumentDataMap eventArgumentDataMap;
  eventArgumentDataMap["pointer"] = (void*)this;
  queueEvent("EntityRemoved", eventArgumentDataMap);
}

void Moveable::updateDeltaVectorPosition(const float lastDelta, const float fakeFrictionValue)
{
  if(acceleration.x != 0 && acceleration.y != 0)
  {
    acceleration *= 0.70710678118f;
  }
  acceleration *= metersPerSecondSquared;
  
  positionDeltaVector = acceleration * 0.5f * (lastDelta * lastDelta) + velocity * lastDelta;
  velocity += acceleration * lastDelta;


  // If fakeFrictionValue Is 1.0f Than velocity will be reduced to 0 in 1 second  
  velocity -= velocity * fakeFrictionValue * lastDelta;
  acceleration = Vector2f();
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

Bullet::Bullet(const EntityPosition& position, const Vector2f& initialVelocity, const Vector2f& dimensions)
{
  
  this->dimensions = dimensions;
  this->position = position;
  velocity = initialVelocity;
  
  renderData.type = ER_PRIMITIVE;
  
  renderData.primitiveType = PT_CIRCLE;
  renderData.dimensionsInTiles = dimensions;
  renderData.color = Vector3f(rand()%256, rand()%256, rand()%256);

  //std::cout << "Created Bullet \n";
}

void Bullet::update(const float lastDelta)
{
  positionDeltaVector = Vector2f();
  updateDeltaVectorPosition(lastDelta, 0.001f);

  if(velocity.getLength() < 1.0f) die();
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
    positionDeltaVector.x *= -1.0f;
    break;
  case COLLISION_PLANE_HORIZONTAL:
    velocity.y *= -speedIncrease;
    positionDeltaVector.y *= -1.0f;

    break;
  }

  entity->addLife(-0.05f);
  
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

FloatRect Item::getCollisionRect() const
{
  return FloatRect(0, 0, 1.0f, 1.0f);
}

void Item::onEntityCollision(COLLISION_PLANE worldCollisionType, Entity* entity)
{
  if(entity->canReceiveItems())
  {
    performItemAction(entity);
    die();
  }
}

void HealthItem::performItemAction(Entity* actionReceiver)
{
  actionReceiver->addLife(itemValue);
}

Mob::Mob(const EntityPosition& position)
{
  this->position = position;
  renderData.type = ER_MOB;
}

void Mob::addLife(const float amount)
{
  life += amount;
  if(life > 1.0f) life = 1.0f;
  if(life < 0) die();
}

const EntityRenderData& Mob::getRenderData()
{
  renderData.life = life;
  return renderData;
}

Cannon::Cannon(const EntityPosition& position) : Mob(position)
{
  dimensions = Vector2f(1.0f, 1.0f);
  renderData.spriteName = "Cannon";
  renderData.caption = "Cannon";
}

void Cannon::update(const float lastDelta)
{
  static const float shootPeriod = 5.0f;
  localTime += lastDelta;

  if(localTime >= shootPeriod)
  {
    localTime = fmodf(localTime, shootPeriod);

    float randomDirection = ((3.14f) / 180.0f) * (rand() % 360);
    
    Vector2f directionVector(cos(randomDirection), -sin(randomDirection));
    float bulletRadius = 1.0f;
    
    EventArgumentDataMap eventArgumentDataMap;
    
    eventArgumentDataMap["entityType"] = ET_BULLET;
    eventArgumentDataMap["dimensions"] = Vector2f(bulletRadius, bulletRadius);
    eventArgumentDataMap["position"] = position + dimensions/2.0f + directionVector * 2.0f;
    eventArgumentDataMap["initialVelocity"] = directionVector * 10.0f;
      
    queueEvent("SpawnEntity", eventArgumentDataMap);
  }
}

Player::Player(const EntityPosition& position) : Mob(position)
{
  dimensions = Vector2f(0.8f, 2.0f);
  renderData.spriteName = "Player";
  renderData.caption = "Player";
}

void Player::update(const float lastDelta)
{
  positionDeltaVector = Vector2f();
  updateDeltaVectorPosition(lastDelta, 0.0f);
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

  if(!entity->isPlayerItem())
  {
    switch(worldCollisionType) {
    case COLLISION_PLANE_VERTICAL:
      velocity.x *= -speedIncrease;
      break;
    case COLLISION_PLANE_HORIZONTAL:
      velocity.y *= -speedIncrease;
      break;
    }    
  }
  
}

EventNameList Player::getEntityEvents()
{
  EventNameList eventNameList;
  eventNameList.push_back("HelloThere");
  eventNameList.push_back("Player");
  return eventNameList;
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
  else if(eventName == "Player")
  {
    
    static const float bulletVelocity = 10.0f;
    float bulletRadius = 0.5f + 0.1f * (rand()%15); 
    
    static const float bulletDistance = 3.0f;
    
    PLAYER_EVENT playerEvent = (PLAYER_EVENT)eventDataMap["playerEventType"].asInt();
    
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
	EventArgumentDataMap eventArgumentDataMap;

	//std::cout << "Here Mutha\n";
      
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
      
	eventArgumentDataMap["entityType"] = ET_BULLET;
	eventArgumentDataMap["dimensions"] = Vector2f(bulletRadius, bulletRadius);
	eventArgumentDataMap["position"] = position + Vector2f(getLocalCollisionCenter().x, 0) +
	  tempDirectionVector * bulletDistance;
	eventArgumentDataMap["initialVelocity"] = velocity + tempDirectionVector * bulletVelocity;
      
	queueEvent("SpawnEntity", eventArgumentDataMap);
      
      }break;
    }
  }
  
}
