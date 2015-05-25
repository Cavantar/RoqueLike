#include "Mobs.h"
#include <sstream>
#include <iomanip>
#include <iostream>

Mob::Mob(const EntityPosition& position, int mobLevel, int health) : mobLevel(mobLevel), health(health)
{
  this->position = position;
  renderData.type = ER_MOB;
}

void
Mob::addHealth(float amount)
{

  if(health < 0)
  {
    amount += getShieldValue();
    if(amount > 0) amount = 0;
  }
  
  health += amount;
  if(health > maxHealth) health = maxHealth;
  else if(health < 0) die();
      
  OverlayTextData overlayTextData = {"", 2.0f, Vector3f(), 1.1f};
  std::stringstream tempText;
  tempText << std::fixed << std::setw(11) << std::setprecision(2);
  
  if(amount != 0) tempText << amount << " Health";
  if(amount > 0)
  {
    overlayTextData.color = Vector3f(0, 200.0f, 0);
  }
  else if(amount < 0)
  {
    overlayTextData.color = Vector3f(200.0f, 0, 0);
  }
  else
  {
    tempText << "Blocked";
    overlayTextData.color = Vector3f(0, 0, 250);
  }
  
  overlayTextData.text = tempText.str();
  Entity* overlayText = new OverlayText(position, overlayTextData);
  level->addOverlayEntity(EntityPtr(overlayText));
}

void
Mob::spawnXp(int xpToSpawn) const
{
  assert(!(xpToSpawn%10));
  
  //xpToSpawn *= 20;
  while(xpToSpawn)
  {
    float value = -1;
    
    // Getting Randomly valued experience orb (in range)
    // I assume that the xp is divisible by 10
    value = (((rand() % (xpToSpawn/10)) + 1) * 10) % (xpToSpawn + 10);
    if(value > 50) value = 50;
    
    Entity* entity = new XpOrb(getCollisionCenter(), Vector2f::directionVector() * (1.0f + 0.25f * ((rand()%12) + 1)), value);
    level->addEntity(EntityPtr(entity));

    //std::cout << "Spawning: " << value << " xp \n";
    xpToSpawn -= value;
  }
}

void
Mob::performDeathAction()
{
  int xpToSpawn = mobLevel * 20;
  spawnXp(xpToSpawn);
}

const EntityRenderData*
Mob::getRenderData()
{
  renderData.life = health / maxHealth;
  return &renderData;
}

Cannon::Cannon(const EntityPosition& position, int level) : Mob(position, level)
{
  dimensions = Vector2f(1.0f, 1.0f);
  renderData.spriteName = "cannonBase";
  
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
    
    if(player && level->canSeeEachOther(this, player, 15.0f))
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

Follower::Follower(const EntityPosition& position, int level) : Mob(position, level)
{
  dimensions = Vector2f(1.0f, 2.0f);
  renderData.spriteName = "followerBase";
  
  std::stringstream caption;
  caption << "Follower lvl: " << level;  
  renderData.caption = caption.str();
  
  maxHealth = 5 + (level - 1) * 5;
  health = maxHealth;
  damageValue = (level + 1.0f) / 5.0f;
}

void
Follower::update(const float lastDelta)
{

  Player* player = level->getPlayer();
  if(player && level->canSeeEachOther(this, player, 15.0f))
  {
    EntityPosition playerPosition = player->getCollisionCenter();
    EntityPosition followerPosition = getCollisionCenter();
    
    Vector2f distanceVector = EntityPosition::calculateDistanceInTiles(followerPosition, playerPosition,
								       level->getTileMap()->getTileChunkSize());
    // If There's Player in Radius of given length 
    if(distanceVector.getLength() < 15.0f)
    {
      distanceVector.normalize();
      Vector2f directionVector = distanceVector;
      acceleration = directionVector;
    }
  }

  EntityPosition collisionCenter = getCollisionCenter();
  float friction = level->getFrictionValueAtPosition(collisionCenter);
  float accelerationModifier  = level->getAccelerationModifierAtPosition(collisionCenter);
  
  Vector2f positionDeltaVector = getPositionDeltaVector(lastDelta, friction, accelerationModifier);

  EntityCollisionResult collisionResult = level->checkCollisions(this, positionDeltaVector);
  handleCollisionResult(collisionResult, positionDeltaVector);
}

FloatRect
Follower::getCollisionRect() const
{
  const float width = 0.6f;
  const float height = 0.3f;
  return FloatRect(0.5f - (width / 2.0f), 2.0f - height, width, height);
}

void
Follower::onWorldCollision(COLLISION_PLANE collisionPlane)
{
  velocity = getReflectedVelocity(collisionPlane, 0.5f);
}

void
Follower::onEntityCollision(COLLISION_PLANE collisionPlane, Entity* entity)
{
  if(entity->isPlayer())
  {
    entity->addHealth(-damageValue);
    entity->addVelocity(velocity * 2.5f);
  }
  else entity->addVelocity(velocity * 0.5f);

  velocity = getReflectedVelocity(collisionPlane, 0.5f);
}

Snake::Snake(const EntityPosition& position, int level) : Mob(position, level)
{
  dimensions = Vector2f(1.0f, 1.0f);
  renderData.spriteName = "snakeBase";
  
  std::stringstream caption;
  caption << "Snake lvl: " << level;  
  renderData.caption = caption.str();
  
  maxHealth = 5 + (level - 1) * 5;
  health = maxHealth;
  damageValue = (level + 1.0f) / 5.0f;
  
  currentDirection = Vector2f::cardinalDirection((CARDINAL_DIRECTION)(rand()%4));
  metersPerSecondSquared = idleSpeedValue;
  localAttackingTime = 0;
}

void
Snake::update(const float lastDelta)
{

  Player* player = level->getPlayer();
  if(player && metersPerSecondSquared != attackSpeedValue)
  {
    Vector2f checkResult = level->canSeeEachOtherCardinal(this, player, 15.0f);
    if(checkResult != Vector2f())
    {
      velocity = 0;
      currentDirection = checkResult;
      metersPerSecondSquared = attackSpeedValue;
    }
  }

  if(metersPerSecondSquared == attackSpeedValue)
  {
    localAttackingTime += lastDelta;
  }

  static const float maxAttackingTime = 2.0f;
  if(localAttackingTime > maxAttackingTime)
  {
    localAttackingTime = 0;
    metersPerSecondSquared = idleSpeedValue;
  }
  
  acceleration = currentDirection;
  
  EntityPosition collisionCenter = getCollisionCenter();
  float friction = level->getFrictionValueAtPosition(collisionCenter);
  float accelerationModifier  = level->getAccelerationModifierAtPosition(collisionCenter);
  
  Vector2f positionDeltaVector = getPositionDeltaVector(lastDelta, friction, accelerationModifier);

  EntityCollisionResult collisionResult = level->checkCollisions(this, positionDeltaVector);
  handleCollisionResult(collisionResult, positionDeltaVector);
}

FloatRect
Snake::getCollisionRect() const
{
  const float width = 0.8f;
  const float height = 0.3f;
  return FloatRect(0.5f - (width / 2.0f), 1.0f - height, width, height);
}

void
Snake::onWorldCollision(COLLISION_PLANE collisionPlane)
{
  velocity = 0;
  if(currentDirection.x != 0)
    currentDirection = Vector2f::cardinalDirection(rand()%2 ? CD_UP : CD_DOWN);
  else
    currentDirection = Vector2f::cardinalDirection(rand()%2 ? CD_LEFT : CD_RIGHT);
  
  metersPerSecondSquared = idleSpeedValue;
}

void
Snake::onEntityCollision(COLLISION_PLANE collisionPlane, Entity* entity)
{
  
  entity->addHealth(-damageValue);
  entity->addVelocity(velocity * 2.5f);
  
  //velocity = getReflectedVelocity(collisionPlane, 0.5f);
  velocity = 0;
  
  if(currentDirection.x != 0)
    currentDirection = Vector2f::cardinalDirection(rand()%2 ? CD_UP : CD_DOWN);
  else
    currentDirection = Vector2f::cardinalDirection(rand()%2 ? CD_LEFT : CD_RIGHT);

  metersPerSecondSquared = idleSpeedValue;
}


Rat::Rat(const EntityPosition& position, int level) : Mob(position, level)
{
  dimensions = Vector2f(1.0f, 1.0f);
  renderData.spriteName = "ratBase";
  
  std::stringstream caption;
  caption << "Rat lvl: " << level;  
  renderData.caption = caption.str();
  
  maxHealth = 5 + (level - 1) * 5;
  health = maxHealth;
  damageValue = (level + 1.0f) / 5.0f;
  
  currentDirection = Vector2f::directionVector();
  
  metersPerSecondSquared = 10.0f;
  ratState = RS_SNIFFING;
  localStateTime = 2.0f + (rand()%5) * 0.5f;
  
}

void
Rat::update(const float lastDelta)
{
  bool dying = (health / maxHealth)  < 0.2f;
  Player* player = level->getPlayer();
  if(player && level->canSeeEachOther(this, player, 15.0f))
  {
    EntityPosition playerPosition = player->getCollisionCenter();
    EntityPosition followerPosition = getCollisionCenter();
    
    Vector2f distanceVector = EntityPosition::calculateDistanceInTiles(followerPosition, playerPosition,
								       level->getTileMap()->getTileChunkSize());
    // If There's Player in given Radius 
    if(distanceVector.getLength() < 4.0f + (mobLevel * 0.5) && !dying)
    {
      distanceVector.normalize();
      Vector2f directionVector = distanceVector;
      acceleration = directionVector;
    }
    // If he's dying 
    else if(dying)
    {
      distanceVector.normalize();
      acceleration = distanceVector * -1.0f;
    }
    else acceleration = currentDirection;
  }
  else acceleration = currentDirection;
  
  localStateTime -= lastDelta;
  if(localStateTime < 0 && !dying)
  {
    switch(ratState)
    {
    case RS_SNIFFING:
      {
	// Going into thinking state
	if(rand()%3 == 0)
	{
	  localStateTime = 0.5f + (rand()%5) * 0.2f;
	  ratState = RS_THINKING;
	  currentDirection = Vector2f();
	}
	// Still Sniffing
	else
	{
	  localStateTime = 2.0f + (rand()%5) * 0.5f;
	  currentDirection = Vector2f::directionVector();
	}
      } break;
    case RS_THINKING:
      {
	localStateTime = 2.0f + (rand()%5) * 0.5f;
	ratState = RS_SNIFFING;
      } break;
    }
  }
    
  
  EntityPosition collisionCenter = getCollisionCenter();
  float friction = level->getFrictionValueAtPosition(collisionCenter);
  float accelerationModifier  = level->getAccelerationModifierAtPosition(collisionCenter);
  
  Vector2f positionDeltaVector = getPositionDeltaVector(lastDelta, friction, accelerationModifier);

  EntityCollisionResult collisionResult = level->checkCollisions(this, positionDeltaVector);
  handleCollisionResult(collisionResult, positionDeltaVector);
}

FloatRect
Rat::getCollisionRect() const
{
  const float width = 0.8f;
  const float height = 0.3f;
  return FloatRect(0.5f - (width / 2.0f), 1.0f - height, width, height);
}

void
Rat::onWorldCollision(COLLISION_PLANE collisionPlane)
{
  velocity = 0;
  currentDirection = Vector2f::directionVector();
}

void
Rat::onEntityCollision(COLLISION_PLANE collisionPlane, Entity* entity)
{
  
  entity->addHealth(-damageValue);
  entity->addVelocity(velocity * 2.5f);
  
  //velocity = getReflectedVelocity(collisionPlane, 0.5f);
  velocity = 0;
}


Player::Player(const EntityPosition& position) : Mob(position, 1, 1.0f)
{
  dimensions = Vector2f(1.0f, 2.0f);
  renderData.spriteName = "playerBase";
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
}

void
Player::onWorldCollision(COLLISION_PLANE collisionPlane)
{
  const float bounceFactor = 0.5f;
  velocity = getReflectedVelocity(collisionPlane, bounceFactor);
  
  spawnDustParticles(getCollisionCenter(), 10, 2);
}

void
Player::onEntityCollision(COLLISION_PLANE collisionPlane, Entity* entity)
{
  static const float speedIncrease = 1.05f;
  entity->addVelocity(velocity * 0.01f);
  
  velocity = getReflectedVelocity(collisionPlane, speedIncrease);
}

FloatRect
Player::getCollisionRect() const
{
  const float width = 0.5f;
  const float height = 0.3f;
  return FloatRect(width / 2.0f, 2.0f - height, width, height);
}

void
Player::addXp(const float amount)
{
  xpAmount += amount;
  
  OverlayTextData overlayTextData = {"", 2.0f, Vector3f(), 1.1f};
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

  OverlayTextData overlayTextData = {"", 4.0f, Vector3f(), 1.5f};
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
    
    static const float bulletDistance = 1.5f;
    Vector2f tempDirectionVector;
    
    if(playerInput.actionUp) tempDirectionVector = Vector2f(0, -1.0f);
    if(playerInput.actionRight) tempDirectionVector = Vector2f(1.0f, 0);
    if(playerInput.actionDown) tempDirectionVector = Vector2f(0, 1.0f);
    if(playerInput.actionLeft) tempDirectionVector = Vector2f(-1.0f, 0);
    
    EntityPosition bulletPosition = position + getLocalCollisionCenter() +
      tempDirectionVector * bulletDistance;
    
    bulletPosition -=Vector2f(0, bulletRadius);
    
    if(playerInput.actionRight || playerInput.actionLeft)
      bulletPosition -= Vector2f(0, 1.0f);
    
    Entity* bullet = new Bullet(bulletPosition,
				velocity + tempDirectionVector * bulletVelocity,
				Vector2f(bulletRadius, bulletRadius),
				damageValue);
      
    if(stamina > 20 && level->addEntity(EntityPtr(bullet))) stamina -= 20;

    //spawnDustParticles(getCollisionCenter(), 10, 10);
  }

  if(skillPointCount > 0)
  {
    if(playerInput.playerKey1) upgradeAbility(PU_SHIELD);
    if(playerInput.playerKey2) upgradeAbility(PU_DAMAGE);
    if(playerInput.playerKey3) upgradeAbility(PU_MOVESPEED);
    if(playerInput.playerKey4) upgradeAbility(PU_BULLETSPEED);
    if(playerInput.playerKey5) upgradeAbility(PU_HEALTH);
    if(playerInput.playerKey6) upgradeAbility(PU_STAMINA);

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
  case PU_SHIELD:
    shieldValue *= 1.5f;
    break;
  case PU_DAMAGE:
    damageValue *= 1.5f;
    break;
  case PU_MOVESPEED:
    metersPerSecondSquared += 5.0f;
    break;
  case PU_BULLETSPEED:
    bulletVelocity *= 1.2f;
    break;
  case PU_HEALTH:
    maxHealth *= 1.5f;
    break;
  case PU_STAMINA:
    maxStamina *= 1.5f;
    break;
  }

  --skillPointCount;
}
