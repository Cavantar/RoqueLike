#include "Level.h"

#include <iostream>
#include <algorithm>

Level::Level()
{
  tileMap = TileMapPtr(new TileMap(Vector2i(16, 16)));
  std::cout << "Level Created !\n";
}

void
Level::update(const float lastDelta)
{
  killCollidingEntities();
  updateEntities(lastDelta);
}

void
Level::registerPendingEntities(EventManager& eventManager)
{
  for(auto entityIt = pendingEntityList.begin(); entityIt != pendingEntityList.end(); entityIt++)
  {
    EntityPtr entityPtr= *entityIt;
    eventManager.registerListener(entityPtr.get());
    entityList[0].push_back(entityPtr);
  }
  
  pendingEntityList.clear();
}

void
Level::updateEntities(const float lastDelta)
{

  for(int entityLayer = 0; entityLayer < numbOfEntityLayers; entityLayer++)
  {
    for(auto entityPtr = entityList[entityLayer].begin(); entityPtr != entityList[entityLayer].end(); entityPtr++)
    {
      (*entityPtr)->update(lastDelta);
    }
  }
}

void
Level::removeDeadEntities()
{
  for(int entityLayer = 0; entityLayer < numbOfEntityLayers; entityLayer++)
  {
    auto entityPtrIt = entityList[entityLayer].begin();
    
    while(entityPtrIt != entityList[entityLayer].end())
    {
      if(!(*entityPtrIt)->isAlive())
      {
	(*entityPtrIt)->performDeathAction();
	
	if((*entityPtrIt)->isPlayer()) player = NULL;
	entityPtrIt = entityList[entityLayer].erase(entityPtrIt);
      }
      else
      {
	entityPtrIt++;
      }
    }
  }
}

EntityCollisionResult
Level::checkCollisions(const Entity* entity, Vector2f deltaVector) const
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
  
  CollisionCheckData collisionCheckData = {entityPosition, collisionRect, deltaVector};
  WorldCollisionResult worldCollisionResult = checkWorldCollision(collisionCheckData);
  
  EntityCollisionResult entityCollisionResult;
  
  if(entity->canCollideWithEntities())
  {
    entityCollisionResult = checkEntityCollision(entity, collisionCheckData);
  }
  
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
      collisionCheckResult = entityCollisionResult;
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

bool
Level::canSeeEachOther(const Entity* entity1, const Entity* entity2, float maxRange) const
{
  EntityPosition pos1 = entity1->getCollisionCenter();
  EntityPosition pos2 = entity2->getCollisionCenter();

  Vector2f deltaVector =  EntityPosition::calculateDistanceInTiles(pos1, pos2, tileMap->getTileChunkSize());
  
  float deltaLength = deltaVector.getLength();
  
  if(deltaLength < maxRange)
  {
    Vector2f directionVector = deltaVector / deltaLength;
    
    // Distance Between Checked Points
    // 0.5f because of diagonal
    
    float iterationRange = 0.3f;
    int numbOfIterations = deltaLength / iterationRange;
    
    for(int i = 1; i <= numbOfIterations; i++)
    {
      pos1 += directionVector * iterationRange;
      tileMap->recanonicalize(pos1);
      TILE_TYPE tileType = tileMap->getTileType(pos1.worldPosition);
      if(tileType == TILE_TYPE_WALL) return false;
    }
  }
  
  return true;
}

Vector2f 
Level::canSeeEachOtherCardinal(const Entity* entity1, const Entity* entity2, float maxRange) const
{

  EntityPosition pos1 = entity1->getCollisionCenter();
  EntityPosition pos2 = entity2->getCollisionCenter();

  Vector2f deltaVector =  EntityPosition::calculateDistanceInTiles(pos1, pos2, tileMap->getTileChunkSize());
  Vector2f directionVector;
  float iterationRange = 0.1f;
  
  float deltaLength = 0;
  
  // Y Axis Delta
  Vector2f yAxisDelta(0, deltaVector.y);
  deltaLength = yAxisDelta.getLength();
  
  directionVector = yAxisDelta;
  directionVector.normalize();
  
  EntityPosition checkPosition = pos1;
  if(deltaLength < maxRange)
  {
    int numbOfIterations = deltaLength / iterationRange;
    
    for(int i = 1; i <= numbOfIterations; i++)
    {
      checkPosition += directionVector * iterationRange;
      tileMap->recanonicalize(checkPosition);
      TILE_TYPE tileType = tileMap->getTileType(checkPosition.worldPosition);
      if(tileType == TILE_TYPE_WALL) break;
      
      // If checkPosition is inside collision rect of entity 2 then return directionVector
      FloatRect collisionRect = entity2->getCollisionRect();

      Vector2f relativeDistance = EntityPosition::calculateDistanceInTiles(checkPosition,
									   pos2,
									   tileMap->getTileChunkSize());

      // Because Relative Distance is offst from the collision Center
      relativeDistance += Vector2f(collisionRect.width / 2.0f, collisionRect.height / 2.0f);

      // To normalize collision rect
      collisionRect.left = 0;
      collisionRect.top = 0;
      
      if(collisionRect.doesContain(relativeDistance))
      {
	return directionVector;
      }
    }
  }

  // X Axis Delta
  
  Vector2f xAxisDelta(deltaVector.x, 0);
  deltaLength = xAxisDelta.getLength();
  
  directionVector = xAxisDelta;
  directionVector.normalize();
  
  checkPosition = pos1;
  if(deltaLength < maxRange)
  {
    int numbOfIterations = deltaLength / iterationRange;
    
    for(int i = 1; i <= numbOfIterations; i++)
    {
      checkPosition += directionVector * iterationRange;
      tileMap->recanonicalize(checkPosition);
      TILE_TYPE tileType = tileMap->getTileType(checkPosition.worldPosition);
      if(tileType == TILE_TYPE_WALL) break;
            
      // If checkPosition is inside collision rect of entity 2 then return directionVector
      FloatRect collisionRect = entity2->getCollisionRect();
      Vector2f relativeDistance = EntityPosition::calculateDistanceInTiles(checkPosition,
									   pos2,
									   tileMap->getTileChunkSize());
      
      // Because Relative Distance is offst from the collision Center
      relativeDistance += Vector2f(collisionRect.width / 2.0f, collisionRect.height / 2.0f);

      // To normalize collision rect
      collisionRect.left = 0;
      collisionRect.top = 0;
      
      if(collisionRect.doesContain(relativeDistance))
      {
	return directionVector;
      }
      
    }
  }

  return Vector2f();
}

float
Level::getFrictionValueAtPosition(EntityPosition& entityPosition) const
{
  float frictionValue = 2.0;
  
  tileMap->recanonicalize(entityPosition);
  TILE_TYPE tileType = tileMap->getTileType(entityPosition.worldPosition);
  
  switch(tileType)
  {
  case TILE_TYPE_STONE_ICE_GROUND:
    frictionValue = 0.1f;
    break;
  }
  return frictionValue;
}

float
Level::getAccelerationModifierAtPosition(EntityPosition& entityPosition) const
{
  float accelerationModifier = 1.0;
  
  tileMap->recanonicalize(entityPosition);
  TILE_TYPE tileType = tileMap->getTileType(entityPosition.worldPosition);
  
  switch(tileType)
  {
  case TILE_TYPE_STONE_SPEED_GROUND:
    accelerationModifier = 2.0f;
    break;
  }
  return accelerationModifier;
}

int
Level::getSurroundingTileData(const WorldPosition& worldPosition, TILE_TYPE tileType)
{
  int result = 0;
  
  if(tileMap->getTileType(worldPosition + Vector2i(0.0f, -1.0f)) == tileType)
    result |= ST_NORTH;
  if(tileMap->getTileType(worldPosition + Vector2i(1.0f, -1.0f)) == tileType)
    result |= ST_NORTH_EAST;
  if(tileMap->getTileType(worldPosition + Vector2i(1.0f, 0.0f)) == tileType)
    result |= ST_EAST;
  if(tileMap->getTileType(worldPosition + Vector2i(1.0f, 1.0f)) == tileType)
    result |= ST_SOUTH_EAST;
  if(tileMap->getTileType(worldPosition + Vector2i(0.0f, 1.0f)) == tileType)
    result |= ST_SOUTH;
  if(tileMap->getTileType(worldPosition + Vector2i(-1.0f, 1.0f)) == tileType)
    result |= ST_SOUTH_WEST;
  if(tileMap->getTileType(worldPosition + Vector2i(-1.0f, 0.0f)) == tileType)
    result |= ST_WEST;
  if(tileMap->getTileType(worldPosition + Vector2i(-1.0f, -1.0f)) == tileType)
    result |= ST_NORTH_WEST;
  
  return result;
}

TileList
Level::getAffectedTiles(const CollisionCheckData& collisionCheckData) const
{
  TileList affectedTiles;
  
  const Vector2i& tileChunkSize = tileMap->getTileChunkSize();
  
  EntityPosition cornerPosition = collisionCheckData.basePosition + collisionCheckData.collisionRect[0];
  EntityPosition tempDeltaPos = cornerPosition + collisionCheckData.deltaVector;

  // If i'm moving right then i'm adding width to "delta" position to include rect width
  if(collisionCheckData.deltaVector.x >= 0) tempDeltaPos += Vector2f(collisionCheckData.collisionRect.width, 0);
  else cornerPosition += Vector2f(collisionCheckData.collisionRect.width, 0);

  // If i'm moving down then i'm adding height to "delta" position to include rect height
  if(collisionCheckData.deltaVector.y >= 0) tempDeltaPos += Vector2f(0, collisionCheckData.collisionRect.height);
  else cornerPosition += Vector2f(0, collisionCheckData.collisionRect.height);
  
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

WorldCollisionResult
Level::checkCollisionsWithTiles(TileList& tiles,
				const CollisionCheckData& collisionCheckData) const 
{
  
  WorldCollisionResult collisionResult; 
  
  // Checking Each Tile 
  for(auto tileIt = tiles.begin(); tileIt != tiles.end(); tileIt++)
  {
    
    // Only Collide With Wall Tiles
    if(tileMap->getTileType(*tileIt) == TILE_TYPE_WALL)
    {
      
      // Distance Of The Tile From The Position 
      Vector2f localTilePosition = EntityPosition::calculateDistanceInTiles(collisionCheckData.basePosition,
									    EntityPosition(*tileIt),
									    tileMap->getTileChunkSize());
      
      // Preparing Objects for Minkowsy's Collision Checking 
      float halfWidth = collisionCheckData.collisionRect.width / 2.0f;
      float halfHeight = collisionCheckData.collisionRect.height / 2.0f;
	
      FloatRect localTileRect(localTilePosition.x - halfWidth, localTilePosition.y - halfHeight,
			      1.0f + collisionCheckData.collisionRect.width,
			      1.0f + collisionCheckData.collisionRect.height);
      
      // Positioning Player Point In The Center of Previous Collision Rect
      Vector2f playerPoint = collisionCheckData.collisionRect[0] + Vector2f(halfWidth, halfHeight);
      
      // Checking Each of 4 walls and Getting The One That Collides First
      for(int wallIndex=0; wallIndex < 4; wallIndex++)
      {
	float tempT = localTileRect.getMaxTime(playerPoint, collisionCheckData.deltaVector, wallIndex);
	
	if(tempT < collisionResult.maxAllowedT)
	{
	  collisionResult.maxAllowedT = tempT;
	  
	  if(wallIndex%2 == 0)
	    collisionResult.collisionPlane = COLLISION_PLANE_HORIZONTAL;
	  else
	    collisionResult.collisionPlane = COLLISION_PLANE_VERTICAL;
	  
	}
      }
    }
  }
  
  return collisionResult;
}
  
WorldCollisionResult
Level::checkWorldCollision(const CollisionCheckData& collisionCheckData) const
{
  TileList affectedTiles = getAffectedTiles(collisionCheckData);
  WorldCollisionResult collisionResult = checkCollisionsWithTiles(affectedTiles, collisionCheckData);
  return collisionResult;
}

EntityCollisionResult
Level::checkEntityCollision(const Entity* entity,
						  const CollisionCheckData& collisionCheckData) const
{
  
  float maxAllowedT = 1.0f;
  int wallIndex = -1;
  
  float halfWidth = collisionCheckData.collisionRect.width / 2.0f;
  float halfHeight = collisionCheckData.collisionRect.height / 2.0f;
  
  // Centering EntityPoint For Minkowsky
  Vector2f entityPosition = collisionCheckData.collisionRect[0] + Vector2f(halfWidth, halfHeight);
  
  EntityCollisionResult collisionResult;
  
  for(auto entity2 = entityList[0].begin(); entity2 != entityList[0].end(); entity2++)
  {
    // If The Entities are The Same we don't check Collisions(Comparing Pointers)
    if(entity == (*entity2).get()) continue;
    if(!(*entity2)->isAlive() || !(*entity2)->canCollideWithEntities()) continue;
    
    // Check Collisions
    // Convert To Local Space By Subtracting deltaVector
    
    EntityPtr entityPtr2 = *entity2;
    
    // Collision Rect For Second Object
    FloatRect collisionRect2 = entityPtr2->getCollisionRect();
    
    Vector2f localRectPosition = EntityPosition::calculateDistanceInTiles(collisionCheckData.basePosition,
									  entityPtr2->getPosition(),
									  tileMap->getTileChunkSize());
    
    // Minkowsky Addition
    FloatRect collidingRect(localRectPosition.x + collisionRect2.left - halfWidth,
			    localRectPosition.y + collisionRect2.top - halfHeight,
			    collisionRect2.width + collisionCheckData.collisionRect.width,
			    collisionRect2.height + collisionCheckData.collisionRect.height);
    
    // Checking Each of 4 walls and Getting The One That Collides First
    for(int wallIndex=0; wallIndex < 4; wallIndex++)
    {
      float tempT = collidingRect.getMaxTime(entityPosition, collisionCheckData.deltaVector, wallIndex);
      
      if(tempT < collisionResult.maxAllowedT && tempT >= 0.0f) 
      {
	collisionResult.maxAllowedT = tempT;
	collisionResult.collidedEntity = (*entity2).get();
	
	if(wallIndex%2 == 0)
	  collisionResult.collisionPlane = COLLISION_PLANE_HORIZONTAL;
	else
	  collisionResult.collisionPlane = COLLISION_PLANE_VERTICAL;
      }
    }
  }
  
  return collisionResult;
}

bool
Level::addEntity(EntityPtr& entityPtr)
{
  if(!isCollidingWithLevel(entityPtr.get()))
  {
    entityPtr->setLevel(this);
    pendingEntityList.push_back(entityPtr);
    return true;
  }
  else
  {
    return false;
  }
  
}

void
Level::addOverlayEntity(EntityPtr& entityPtr)
{
  entityPtr->setLevel(this);
  entityList[1].push_back(entityPtr);
}

EventNameList
Level::getEntityEvents()
{
  EventNameList eventNameList;
  eventNameList.push_back("SpawnEntity");
  return eventNameList;
}

void
Level::onEvent(const std::string& eventName, EventArgumentDataMap eventDataMap)
{
  //std::cout << "Here\n";
  
  /*  
      if(eventName == "SpawnEntity")
      {
      //std::cout << "Spawning Entity \n";
      
      }
  */
  
}

bool
Level::isCollidingWithLevel(Entity* entity) const
{
  const EntityPosition& entityPosition = entity->getPosition();
  const FloatRect& collisionRect = entity->getCollisionRect();
  
  // Checking Collisions with Tiles - Cause It's Faster
  
  CollisionCheckData collisionCheckData = { entityPosition, collisionRect,  Vector2f() };
  
  TileList affectedTiles = getAffectedTiles(collisionCheckData);
  for(auto tileIt = affectedTiles.begin(); tileIt != affectedTiles.end(); tileIt++)
  {
    TILE_TYPE tileType = tileMap->getTileType(*tileIt);
    if(tileType == TILE_TYPE_WALL || tileType == TILE_TYPE_VOID)
    {
      return true;
    }
  }

  if(entity->canCollideWithEntities())
  {
      
    for(auto entityIt = entityList[0].begin(); entityIt != entityList[0].end(); entityIt++)
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

void
Level::killCollidingEntities()
{
  for(auto entityPtr = entityList[0].begin(); entityPtr != entityList[0].end(); entityPtr++)
  {
    Entity* entity = (*entityPtr).get();
    if(isCollidingWithLevel(entity))
    {
      std::cout << "Entity died a horrible death ! \n";
      entity->die();
    }
  }
}
