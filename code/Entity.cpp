#include "Entity.h"
#include <iostream>
#include <sstream>
#include <iomanip>

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
  renderData.color = Vector3f(102 + rand()%20, 255 - rand()%30, 0);
}

void
XpOrb::update(const float lastDelta)
{
  // Simulate Movement First
  
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
  
  renderData.type = ER_PRIMITIVE;
  
  renderData.primitiveType = PT_CIRCLE;
  renderData.dimensionsInTiles = dimensions;
  renderData.color = Vector3f(rand()%256, rand()%256, rand()%256);
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
  if(numbOfBouncesLeft-- == 0) die();
  
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

  entity->addHealth(-damageValue);
  entity->addVelocity(velocity * 0.5f);
  
  velocity = getReflectedVelocity(collisionPlane, speedIncrease);
  
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

void
HealthItem::performItemAction(Entity* actionReceiver)
{
  actionReceiver->addHealth(itemValue);
}

Mob::Mob(const EntityPosition& position, int mobLevel, int health) : mobLevel(mobLevel), health(health)
{
  this->position = position;
  renderData.type = ER_MOB;
}

void
Mob::addHealth(const float amount)
{
  health += amount;
  if(health > maxHealth) health = maxHealth;
  else if(health < 0) die();
  
  OverlayTextData overlayTextData = {"", 2.0f, Vector3f(), 3.0f};
  std::stringstream tempText;
  tempText << std::fixed << std::setw(11) << std::setprecision(2);
    
  tempText << amount << " Health";
  if(amount > 0)
  {
    overlayTextData.color = Vector3f(0, 200.0f, 0);
  }
  else
  {
    overlayTextData.color = Vector3f(200.0f, 0, 0);
  }
  
  overlayTextData.text = tempText.str();
  Entity* overlayText = new OverlayText(position, overlayTextData);
  level->addOverlayEntity(EntityPtr(overlayText));
}

const EntityRenderData&
Mob::getRenderData()
{
  renderData.life = health / maxHealth;
  return renderData;
}

void
Mob::spawnXp(int xpToSpawn) const
{
  assert(!(xpToSpawn%10));

  //xpToSpawn *= 10;
  while(xpToSpawn)
  {
    float value = -1;
    
    // Getting Randomly valued experience orb (in range)
    // I assume that the xp is divisible by 10
    value = (((rand() % (xpToSpawn/10)) + 1) * 10) % (xpToSpawn + 10);
    if(value > 50) value = 50;
    
    Entity* entity = new XpOrb(position, Vector2f::directionVector() * (1.0f + 0.25f * ((rand()%12) + 1)), value);
    level->addEntity(EntityPtr(entity));

    //std::cout << "Spawning: " << value << " xp \n";
    xpToSpawn -= value;
  }
}

Cannon::Cannon(const EntityPosition& position, int level) : Mob(position, level)
{
  dimensions = Vector2f(1.0f, 1.0f);
  renderData.spriteName = "Cannon";
  renderData.color = Vector3f();
  
  std::stringstream caption;
  caption << "Cannon lvl: " << level;  
  renderData.caption = caption.str();

  maxHealth = 10 + (level - 1) * 5;
  health = maxHealth;
  damageValue = (level + 1.0f) / 5.0f;
}

void
Cannon::update(const float lastDelta)
{
  float shootPeriod = 5.0f / mobLevel;
  localTime += lastDelta;

  if(localTime >= shootPeriod)
  {
    localTime = fmodf(localTime, shootPeriod);
    Player* player = level->getPlayer();

    if(player)
    {
      EntityPosition playerPosition = player->getCollisionCenter();
      Vector2f distanceVector = EntityPosition::calculateDistanceInTiles(position, playerPosition,
									 level->getTileMap()->getTileChunkSize());
      // If There's Player in Radius of given length 
      if(distanceVector.getLength() < 15.0f)
      {
	distanceVector.normalize();
	Vector2f directionVector = distanceVector;
	
	float bulletRadius = 0.7f + mobLevel / 10;
	
	float bulletSpeedModifier = (mobLevel / 10.0f) + 1.0f; 
	
	Entity* bullet;
	bullet = new Bullet(position + directionVector * 2.0f,
			    directionVector * 10.0f * bulletSpeedModifier,
			    Vector2f(bulletRadius, bulletRadius),
			    damageValue);
	
	level->addEntity(EntityPtr(bullet));
      }
      
    }
  } 
}

void
Cannon::performDeathAction()
{
  int xpToSpawn = mobLevel * 20;
  spawnXp(xpToSpawn);
}

Player::Player(const EntityPosition& position) : Mob(position, 1, 1.0f)
{
  dimensions = Vector2f(0.8f, 2.0f);
  renderData.spriteName = "Player";
  renderData.caption = "Player";

  damageValue = 1.0f;
}

void
Player::update(const float lastDelta)
{
  EntityPosition collisionCenter = getCollisionCenter();
  float friction = level->getFrictionValueAtPosition(collisionCenter);
  float accelerationModifier  = level->getAccelerationModifierAtPosition(collisionCenter);
  
  Vector2f positionDeltaVector = getPositionDeltaVector(lastDelta, friction, accelerationModifier);
  
  EntityCollisionResult collisionResult = level->checkCollisions(this, positionDeltaVector);
  handleCollisionResult(collisionResult, positionDeltaVector);
  
  if(xpAmount >= getNextLevelXp()) levelUp();
  
  stamina += (lastDelta / 5.0f) * maxStamina;
  if(stamina >= maxStamina) stamina = maxStamina;

  //std::cout << velocity.x << " " << velocity.y << std::endl;
}

void
Player::onWorldCollision(COLLISION_PLANE collisionPlane)
{
  const float bounceFactor = 0.5f;
  velocity = getReflectedVelocity(collisionPlane, bounceFactor);
}

void
Player::onEntityCollision(COLLISION_PLANE collisionPlane, Entity* entity)
{
  static const float speedIncrease = 1.05f;
  entity->addVelocity(velocity * 0.01f);
  
  velocity = getReflectedVelocity(collisionPlane, speedIncrease);
}

void
Player::addXp(const float amount)
{
  xpAmount += amount;
  
  OverlayTextData overlayTextData = {"", 2.0f, Vector3f(), 3.0f};
  std::stringstream tempText;
  tempText << std::fixed << std::setw(11) << std::setprecision(2);
  tempText << amount << " Xp";
  overlayTextData.color = Vector3f(0, 200.0f, 0);
  overlayTextData.text = tempText.str();
  
  Entity* overlayText = new OverlayText(position, overlayTextData);
  level->addOverlayEntity(EntityPtr(overlayText));  
}

void
Player::levelUp()
{
  mobLevel++;
  skillPointCount++;
  
  maxHealth += 10.0f;
  maxStamina += 20.0f;

  OverlayTextData overlayTextData = {"", 4.0f, Vector3f(), 3.0f};
  overlayTextData.color = Vector3f(0, 200.0f, 0);
  overlayTextData.text = "Leveled Up !";
  Entity* overlayText = new OverlayText(position, overlayTextData);
  level->addOverlayEntity(EntityPtr(overlayText));
}

EventNameList
Player::getEntityEvents()
{
  EventNameList eventNameList;
  eventNameList.push_back("HelloThere");
  eventNameList.push_back("Player");
  return eventNameList;
}

void
Player::handlePlayerInput(const PlayerInput& playerInput)
{

  if(playerInput.up)
  {
    direction = MOB_FACING_UP;
    acceleration += Vector2f(0, -1.0f);
  }
  if(playerInput.right)
  {
    direction = MOB_FACING_RIGHT;
    acceleration += Vector2f(1.0f, 0);
  }
  if(playerInput.down)
  {
    direction = MOB_FACING_DOWN;
    acceleration += Vector2f(0, 1.0f);
  }
  if(playerInput.left)
  {
    direction = MOB_FACING_LEFT;
    acceleration += Vector2f(-1.0f, 0);
  }

  if(playerInput.actionUp || playerInput.actionRight ||
     playerInput.actionDown || playerInput.actionLeft)
  {
    float bulletRadius = 0.5f;
    
    static const float bulletDistance = 3.0f;
    Vector2f tempDirectionVector;
    
    if(playerInput.actionUp) tempDirectionVector = Vector2f(0, -1.0f);
    if(playerInput.actionRight) tempDirectionVector = Vector2f(1.0f, 0);
    if(playerInput.actionDown) tempDirectionVector = Vector2f(0, 1.0f);
    if(playerInput.actionLeft) tempDirectionVector = Vector2f(-1.0f, 0);
    
    EntityPosition bulletPosition = position + Vector2f(getLocalCollisionCenter().x, 0) +
      tempDirectionVector * bulletDistance;
      
    Entity* bullet = new Bullet(bulletPosition,
				velocity + tempDirectionVector * bulletVelocity,
				Vector2f(bulletRadius, bulletRadius),
				damageValue);
      
    if(stamina > 20 && level->addEntity(EntityPtr(bullet))) stamina -= 20;
  }

  if(skillPointCount > 0)
  {
    if(playerInput.playerKey1) upgradeAbility(PU_SHIELD);
    if(playerInput.playerKey2) upgradeAbility(PU_DAMAGE);
    if(playerInput.playerKey3) upgradeAbility(PU_MOVESPEED);
    if(playerInput.playerKey4) upgradeAbility(PU_HEALTH);
    if(playerInput.playerKey4) upgradeAbility(PU_STAMINA);
  }
}

void
Player::onEvent(const std::string& eventName, EventArgumentDataMap eventDataMap)
{
  if(eventName == "HelloThere")
  {
      
    std::cout << eventName << ": " << eventDataMap["text"].asString() << " " <<
      eventDataMap["number"].asInt() << std::endl;
  
    WorldPosition worldPos = eventDataMap["position"].asWorldPosition();
    std::cout << worldPos.tilePosition.x << std::endl;
  }
}

void
Player::performDeathAction()
{
  int xpToSpawn = mobLevel * 200 + xpAmount;
  spawnXp(xpToSpawn);
}

void
Player::upgradeAbility(PLAYER_UPGRADE upgrade)
{
  switch(upgrade)
  {
  case PU_HEALTH:
    maxHealth *= 1.5f;
    break;
  case PU_SHIELD:
    shieldValue *= 1.5f;
    break;
  case PU_MOVESPEED:
    metersPerSecondSquared += 5.0f;
    break;
  case PU_DAMAGE:
    damageValue *= 1.5f;
    break;
  case PU_STAMINA:
    maxStamina *= 1.5f;
    break;
  case PU_BULLETSPEED:
    bulletVelocity *= 1.2f;
    break;
  }

  --skillPointCount;
}

