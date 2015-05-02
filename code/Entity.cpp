#include "Entity.h"
#include <iostream>

Entity::Entity() : position(EntityPosition())   
{
  position += Vector2f(2, 2);
}

void Entity::update(const float lastDelta)
{
  
}

EntityRequestList Entity::getRequestList()
{
  EntityRequestList entityRequestListToReturn = entityRequestList;
  entityRequestList.clear();
  return entityRequestListToReturn;
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

  if(velocity.getLength() < 1.0f) alive = false;
}

void Bullet::onWorldCollision(COLLISION_PLANE worldCollisionType)
{
  if(numbOfBouncesLeft-- == 0) alive = false;
  
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

  //std::cout << " Bullet Colliding " << (int)entity % 10 << std::endl;
  //entity->addVelocity(velocity * 0.1f);
  
  switch(worldCollisionType) {
  case COLLISION_PLANE_VERTICAL:
    velocity.x *= -speedIncrease;
    positionDeltaVector.x *= -1.0f;

    //std::cout << "Changing Direction To: " <<  ((velocity.x > 0) ? "positive" : "negative") << std::endl;
    break;
  case COLLISION_PLANE_HORIZONTAL:
    velocity.y *= -speedIncrease;
    positionDeltaVector.y *= -1.0f;

    break;
  }
  //entity->addLife(-0.1f);
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
    alive = false;
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
  if(life < 0) alive = false;
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
    
    EntityRequest entityRequest;
    entityRequest.type = ERT_SPAWN_ENTITY;
    entityRequest.entityType = ET_BULLET;
    entityRequest.position = position + dimensions/2.0f + directionVector * 2.0f;
    entityRequest.initialVelocity = directionVector * 10.0f;
    entityRequest.dimensions = Vector2f(bulletRadius, bulletRadius);
    entityRequestList.push_back(entityRequest);
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

/*
 */

void Player::handlePlayerEvent(const PLAYER_EVENT playerEvent, const float lastDelta)
{
  static const float bulletVelocity = 10.0f;
  float bulletRadius = 0.5f + 0.1f * (rand()%15); 
  
  switch(playerEvent)
  {
  case PLAYER_MOVE_LEFT:
    {
      direction = MOB_FACING_LEFT;
      acceleration += Vector2f(-1.0f, 0);
      
      /*
      EventArgumentDataMap eventArgumentDataMap;
      
      eventArgumentDataMap["text"] = EventArgumentData("Hello World");
      eventArgumentDataMap["number"] = EventArgumentData(54);
      eventArgumentDataMap["position"] = EventArgumentData(WorldPosition());
      
      queueEvent("HelloThere", eventArgumentDataMap);
      */
      
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
    {
      EntityRequest entityRequest;
      entityRequest.type = ERT_SPAWN_ENTITY;
      entityRequest.entityType = ET_BULLET;
      entityRequest.position = position + Vector2f(0, -1.5f);
      entityRequest.initialVelocity = velocity + Vector2f(0, -bulletVelocity);
      entityRequest.dimensions = Vector2f(bulletRadius, bulletRadius);
      entityRequestList.push_back(entityRequest);
    }break;
  case PLAYER_SHOOT_RIGHT:
    {
      EntityRequest entityRequest;
      entityRequest.type = ERT_SPAWN_ENTITY;
      entityRequest.entityType = ET_BULLET;
      entityRequest.position = position + Vector2f(1.5f, 0);
      entityRequest.initialVelocity = velocity + Vector2f(bulletVelocity, 0);
      entityRequest.dimensions = Vector2f(bulletRadius, bulletRadius);
      entityRequestList.push_back(entityRequest);
    }break;
  case PLAYER_SHOOT_DOWN:
    {
      EntityRequest entityRequest;
      entityRequest.type = ERT_SPAWN_ENTITY;
      entityRequest.entityType = ET_BULLET;
      entityRequest.position = position + Vector2f(0, 2.5f);
      entityRequest.initialVelocity = velocity + Vector2f(0, bulletVelocity);
      entityRequest.dimensions = Vector2f(bulletRadius, bulletRadius);
      entityRequestList.push_back(entityRequest);
    }break;
  case PLAYER_SHOOT_LEFT:
    {
      EntityRequest entityRequest;
      entityRequest.type = ERT_SPAWN_ENTITY;
      entityRequest.entityType = ET_BULLET;
      entityRequest.position = position + Vector2f(-0.5f, 0);
      entityRequest.initialVelocity = velocity + Vector2f(-bulletVelocity, 0);
      entityRequest.dimensions = Vector2f(bulletRadius, bulletRadius);
      entityRequestList.push_back(entityRequest);
    }break;
  }

}
EventNameList Player::getEntityEvents()
{
  EventNameList eventNameList;
  eventNameList.push_back("HelloThere");
  return eventNameList;
}

void Player::onEvent(const std::string& eventName, EventArgumentDataMap eventDataMap)
{
  
  std::cout << eventName << ": " << eventDataMap["text"].asString() << " " <<
    eventDataMap["number"].asInt() << std::endl;
  
  WorldPosition worldPos = eventDataMap["position"].asWorldPosition();
  
  std::cout << worldPos.tilePosition.x << std::endl;
  
  /*  
  */
  
}
