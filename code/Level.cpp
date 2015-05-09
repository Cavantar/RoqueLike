#include "Level.h"

#include <iostream>
#include <algorithm>

Level::Level()
{
  tileMap = TileMapPtr(new TileMap(Vector2i(32, 32)));

  std::cout << "Level Created !\n";
}

void Level::update(const float lastDelta)
{
  killCollidingEntities();
  updateEntities(lastDelta);
}

void Level::registerPendingEntities(EventManager& eventManager)
{
  for(auto entityIt = pendingEntityList.begin(); entityIt != pendingEntityList.end(); entityIt++)
  {
    EntityPtr entityPtr= *entityIt;
    eventManager.registerListener(entityPtr.get());
    entityList.push_back(entityPtr);
  }
  
  pendingEntityList.clear();
}

void Level::updateEntities(const float lastDelta)
{
  for(auto entityPtr = entityList.begin(); entityPtr != entityList.end(); entityPtr++)
  {
    (*entityPtr)->update(*this, lastDelta);
  }
}

void Level::removeDeadEntities()
{
  auto entityPtrIt = entityList.begin();

  while(entityPtrIt != entityList.end())
  {
    if(!(*entityPtrIt)->isAlive())
    {
      (*entityPtrIt)->performDeathAction(*this);
      
      if((*entityPtrIt)->isPlayer()) player = NULL;
      entityPtrIt = entityList.erase(entityPtrIt);
    }
    else
    {
      entityPtrIt++;
    }
  }
}

EntityCollisionResult Level::checkCollisions(const Entity* entity, Vector2f deltaVector) const
{
  // EntityCollision Is initalized by default with values indicating no collision
  EntityCollisionResult collisionCheckResult; 
  
  // If There's no velocity collision couldn't occur - That Eliminates non Moving Entities
  if(deltaVector == Vector2f())
  {
    return collisionCheckResult;
  }
  
  EntityPosition entityPosition = entity->getPosition();
  FloatRect collisionRect = entity->getCollisionRect();
  
  WorldCollisionResult worldCollisionResult = checkWorldCollision(collisionRect,
								  entityPosition,
								  deltaVector);
  
  EntityCollisionResult entityCollisionResult = checkEntityCollision(entity,
								     collisionRect,
								     entityPosition,
								     deltaVector);
  
  float tileDistancePrecision;
  
  if(worldCollisionResult.maxAllowedT == entityCollisionResult.maxAllowedT == 1.0f)
  {
    // No Collision
    return collisionCheckResult;
  }
  else
  {
    if(worldCollisionResult.maxAllowedT < entityCollisionResult.maxAllowedT)
    {
      // Collision With Tiles
      tileDistancePrecision = 0.01f;
      
      collisionCheckResult.maxAllowedT = worldCollisionResult.maxAllowedT;
      collisionCheckResult.collisionPlane = worldCollisionResult.collisionPlane;
    }
    else
    {
      // Collision With Entity
      tileDistancePrecision = 0.1f;

      collisionCheckResult.maxAllowedT = entityCollisionResult.maxAllowedT;
      collisionCheckResult.collisionPlane = entityCollisionResult.collisionPlane;
      collisionCheckResult.collidedEntity = entityCollisionResult.collidedEntity;
    }
    	
    // Have To Decrease It Because Lack Of Precision Causes Clipping Errors
    float deltaVectorLength = deltaVector.getLength();
    
    // In Tiles
    float decreaseValue = tileDistancePrecision / deltaVectorLength;
	
    if(decreaseValue < 1.0f)
    {
      collisionCheckResult.maxAllowedT -= decreaseValue;
    }
    else collisionCheckResult.maxAllowedT = 0;

  }
  
  return collisionCheckResult;
}

float Level::getFrictionValueAtPosition(EntityPosition& entityPosition) const
{
  float frictionValue = 1.3;
  
  tileMap->recanonicalize(entityPosition);
  TILE_TYPE tileType = tileMap->getTileType(entityPosition.worldPosition);
  
  switch(tileType)
  {
  case TILE_TYPE_ICE_GROUND:
    frictionValue = 0.1f;
    break;
  }
  return frictionValue;
}

float Level::getAccelerationModifierAtPosition(EntityPosition& entityPosition) const
{
  float accelerationModifier = 1.0;
  
  tileMap->recanonicalize(entityPosition);
  TILE_TYPE tileType = tileMap->getTileType(entityPosition.worldPosition);
  
  switch(tileType)
  {
  case TILE_TYPE_SPEED_GROUND:
    accelerationModifier = 2.0f;
    break;
  }
  return accelerationModifier;
}

std::list<WorldPosition> Level::getAffectedTiles(const FloatRect& collisionRect,
						 const EntityPosition& basePosition,
						 const Vector2f positionDeltaVector) const
{
  std::list<WorldPosition> affectedTiles;
  
  const Vector2i& tileChunkSize = tileMap->getTileChunkSize();
  
  EntityPosition cornerPosition = basePosition + collisionRect[0];
  EntityPosition tempDeltaPos = cornerPosition + positionDeltaVector;

  // If i'm moving right then i'm adding width to "delta" position to include rect width
  if(positionDeltaVector.x >= 0) tempDeltaPos += Vector2f(collisionRect.width, 0);
  else cornerPosition += Vector2f(collisionRect.width, 0);

  // If i'm moving down then i'm adding height to "delta" position to include rect height
  if(positionDeltaVector.y >= 0) tempDeltaPos += Vector2f(0, collisionRect.height);
  else cornerPosition += Vector2f(0, collisionRect.height);
  
  tempDeltaPos.recanonicalize(tileChunkSize);
  cornerPosition.recanonicalize(tileChunkSize);

  // Getting How many Tiles Should I Check Off off the cornerPosition
  Vector2i tileDeltaVector = WorldPosition::calculateDistanceInTilesInclusive(cornerPosition.worldPosition,
									      tempDeltaPos.worldPosition,
									      tileChunkSize);
  
  int minY = std::min((float)tileDeltaVector.y, 0.0f);
  int maxY = std::max((float)tileDeltaVector.y, 0.0f);
  
  int minX = std::min((float)tileDeltaVector.x, 0.0f);
  int maxX = std::max((float)tileDeltaVector.x, 0.0f);

  for(int y = minY; y <= maxY; y++)
  {
    
    for(int x = minX; x <= maxX; x++)
    {
      affectedTiles.push_back(cornerPosition.worldPosition + Vector2i(x, y));
    }
    
  }
  
  return affectedTiles;
}
  
WorldCollisionResult Level::checkWorldCollision(const FloatRect& collisionRect,
					 const EntityPosition& basePosition,
					 const Vector2f positionDeltaVector) const
{
  WorldCollisionResult collisionResult;
  
  const Vector2i& tileChunkSize = tileMap->getTileChunkSize();

  // 2 Collision Iterations Per Tile In Delta Vector 
  int numbOfCollisionIterations = positionDeltaVector.getLength() * 2.0f;
  // But Minimum is One
  numbOfCollisionIterations = std::max(numbOfCollisionIterations, 1);

  // DeltaPosition Increment per collision iteration
  Vector2f deltaPart = positionDeltaVector / numbOfCollisionIterations;
  
  EntityPosition lastMovedPosition = basePosition;
  for(int i=1; i<= numbOfCollisionIterations; i++)
  {
    // Getting Tiles That May Collide With Player
    std::list<WorldPosition> affectedTiles = getAffectedTiles(collisionRect, lastMovedPosition, deltaPart);
    
    float maxAllowedT = 1.0f;
    int wallIndex = -1;

    // Checking Each Tile 
    for(auto affectedTileIt = affectedTiles.begin(); affectedTileIt != affectedTiles.end(); affectedTileIt++)
    {

      // Only Collide With Wall Tiles
      if(tileMap->getTileType(*affectedTileIt) == TILE_TYPE_WALL)
      {
	
	// Distance Of The Tile From The Position 
	Vector2f localTilePosition = EntityPosition::calculateDistanceInTiles(lastMovedPosition,
									      EntityPosition(*affectedTileIt),
									      tileChunkSize);
	// This Part Is Preparing Objects for Minkowsy's Collision Checking 
	float halfWidth = collisionRect.width / 2.0f;
	float halfHeight = collisionRect.height / 2.0f;
	
	FloatRect localTileRect(localTilePosition.x - halfWidth, localTilePosition.y - halfHeight,
				1.0f + collisionRect.width, 1.0f + collisionRect.height);
	
	// Positioning Player Point In The Center of Previous Collision Rect
	Vector2f playerPoint = collisionRect[0] + Vector2f(halfWidth, halfHeight);

	// Checking Each of 4 walls and Getting The One That Collides First
	for(int wall=0; wall < 4; wall++)
	{
	  float tempT = localTileRect.getMaxTime(playerPoint, deltaPart, wall);
	  
	  if(tempT < maxAllowedT)
	  {
	    maxAllowedT = tempT;
	    wallIndex = wall;
	  }
	}
      }
    }

    // Checking If Collision Occured During TileCheck
    if(maxAllowedT < 1.0f && maxAllowedT >= 0.0f)
    {
      // Adding Current Checked Part Of Delta Vector 
      collisionResult.maxAllowedT = (float(i-1)/(float)numbOfCollisionIterations);
      
      // Adding The Relative Point Of Collision To DeltaVector
      collisionResult.maxAllowedT += (maxAllowedT / (float)numbOfCollisionIterations);
      
      if(wallIndex%2 == 0)
      {
	// Collision Horizontal
	collisionResult.collisionPlane = COLLISION_PLANE_HORIZONTAL;
      }
      else{
	// Collision Vertical
	collisionResult.collisionPlane = COLLISION_PLANE_VERTICAL;
      }
      
      return collisionResult;
    }

    // Advancing Position by DeltaPart because Collision hasn't occured
    lastMovedPosition += deltaPart;
  }
  
  return collisionResult;
}

  
EntityCollisionResult Level::checkEntityCollision(const Entity* entity,
						  const FloatRect& collisionRect,
						  const EntityPosition& basePosition,
						  const Vector2f positionDeltaVector ) const
{
  
  float maxAllowedT = 1.0f;
  int wallIndex = -1;
  
  float halfWidth = collisionRect.width / 2.0f;
  float halfHeight = collisionRect.height / 2.0f;
  
  // Centering EntityPoint For Minkowsky
  Vector2f entityPosition = collisionRect[0] + Vector2f(halfWidth, halfHeight);
  
  EntityCollisionResult collisionResult;
  if(!entity->canCollideWithEntities()) return collisionResult;
  
  for(auto entity2 = entityList.begin(); entity2 != entityList.end(); entity2++)
  {
    // If The Entities are The Same we don't check Collisions(Comparing Pointers)
    if(entity == (*entity2).get()) continue;
    if(!(*entity2)->isAlive() || !(*entity2)->canCollideWithEntities()) continue;
    
    // Check Collisions
    // Convert To Local Space By Subtracting deltaVector
    
    EntityPtr entityPtr2 = *entity2;
    
    // Collision Rect For Second Object
    FloatRect collisionRect2 = entityPtr2->getCollisionRect();
    
    Vector2f localRectPosition = EntityPosition::calculateDistanceInTiles(basePosition,
									  entityPtr2->getPosition(),
									  tileMap->getTileChunkSize());
    
    // Minkowsky Addition
    FloatRect collidingRect(localRectPosition.x + collisionRect2.left - halfWidth,
			    localRectPosition.y + collisionRect2.top - halfHeight,
			    collisionRect2.width + collisionRect.width,
			    collisionRect2.height + collisionRect.height);
    
    // Checking Each of 4 walls and Getting The One That Collides First
    for(int wall=0; wall < 4; wall++)
    {
      float tempT = collidingRect.getMaxTime(entityPosition, positionDeltaVector, wall);
      
      if(tempT < maxAllowedT && tempT >= 0.0f) 
      {
	maxAllowedT = tempT;
	wallIndex = wall;
	collisionResult.collidedEntity = (*entity2).get();
      }
    }
  }

  if(maxAllowedT < 1.0f && maxAllowedT >= 0.0f)
  {
    // Adding Current Checked Part Of Delta Vector 
    collisionResult.maxAllowedT = maxAllowedT;
    
    if(wallIndex%2 == 0)
    {
      collisionResult.collisionPlane = COLLISION_PLANE_HORIZONTAL;
    }
    else{
      collisionResult.collisionPlane = COLLISION_PLANE_VERTICAL;
    }
  }
  
  return collisionResult;
}

bool Level::addEntity(EntityPtr& entityPtr)
{

  if(!isCollidingWithLevel(entityPtr.get()))
  {
    pendingEntityList.push_back(entityPtr);
    return true;
  }
  else
  {
    return false;
  }
  
}

EventNameList Level::getEntityEvents()
{
  EventNameList eventNameList;
  eventNameList.push_back("SpawnEntity");
  return eventNameList;
}

void Level::onEvent(const std::string& eventName, EventArgumentDataMap eventDataMap)
{
  //std::cout << "Here\n";
  
  if(eventName == "SpawnEntity")
  {
    //std::cout << "Spawning Entity \n";
    
    ENTITY_TYPE entityType = (ENTITY_TYPE) eventDataMap["entityType"].asInt();
    Entity* entity = NULL;
    
    switch(entityType)
    {
    case ET_BULLET:
      {
	entity = new Bullet(eventDataMap["position"].asEntityPosition(),
			    eventDataMap["initialVelocity"].asVector2f(),
			    eventDataMap["dimensions"].asVector2f(),
			    0.1f);
	
	if(isCollidingWithLevel(entity))
	{
	  delete entity;
	  entity = NULL;
	}
	
      } break;
    }
    
    if(entity) addEntity(EntityPtr(entity));
  }
}

bool Level::isCollidingWithLevel(Entity* entity) const
{
  const EntityPosition& entityPosition = entity->getPosition();
  const FloatRect& collisionRect = entity->getCollisionRect();
  
  // Checking Collisions with Tiles - Cause It's Faster
  TileList affectedTiles = getAffectedTiles(collisionRect, entityPosition, Vector2f());
  for(auto tileIt = affectedTiles.begin(); tileIt != affectedTiles.end(); tileIt++)
  {
    if(tileMap->getTileType(*tileIt) == TILE_TYPE_WALL)
    {
      return true;
    }
  }

  if(entity->canCollideWithEntities())
  {
      
    for(auto entityIt = entityList.begin(); entityIt != entityList.end(); entityIt++)
    {
      Entity* entity2 = (*entityIt).get();
      if(entity == entity2 || !entity2->isAlive() || !entity2->canCollideWithEntities()) continue;
      
      EntityPosition entityPosition2 = entity2->getPosition();
      FloatRect collisionRect2 = entity2->getCollisionRect();

      Vector2f relativeDistance = EntityPosition::calculateDistanceInTiles(entityPosition,
									   entityPosition2,
									   tileMap->getTileChunkSize());
      collisionRect2 += relativeDistance;
    
      if(collisionRect.doesRectCollideWith(collisionRect2))
      {
	return true;
      }
    }
    
  }
  
  return false;
}

void Level::killCollidingEntities()
{
  for(auto entityPtr = entityList.begin(); entityPtr != entityList.end(); entityPtr++)
  {
    Entity* entity = (*entityPtr).get();
    if(isCollidingWithLevel(entity)) entity->die();
  }
}
