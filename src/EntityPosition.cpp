#include "EntityPosition.h"

void
WorldPosition::recanonicalize(const Vec2i tileChunkSize )
{
  while(tilePosition.x >= tileChunkSize.x)
  {
    tilePosition.x -= (int32)tileChunkSize.x;
    ++tileChunkPosition.x;
  }
  
  while(tilePosition.y >= (int32)tileChunkSize.y)
  {
    tilePosition.y -= (int32)tileChunkSize.y;
    ++tileChunkPosition.y;
  } 
  
  while(tilePosition.x < 0)
  {
    tilePosition.x += (int32)tileChunkSize.x;
    --tileChunkPosition.x;
  } 

  while(tilePosition.y < 0)
  {
    tilePosition.y += (int32)tileChunkSize.y;
    --tileChunkPosition.y;
  }
}

Vec2i
WorldPosition::calculateDistanceInTilesInclusive(const WorldPosition& srcPosition,
							  const WorldPosition& dstPosition,
							  const Vec2i& tileChunkDimensions)
{
  Vec2i deltaVec;
  
  // Adding Chunk Tiles
  deltaVec.x = (dstPosition.tileChunkPosition.x -
		   srcPosition.tileChunkPosition.x) * tileChunkDimensions.x;
  
  deltaVec.y = (dstPosition.tileChunkPosition.y -
		   srcPosition.tileChunkPosition.y) * tileChunkDimensions.y;
  
  // Adding Tiles Inside Of The Chunk
  deltaVec.x += dstPosition.tilePosition.x - srcPosition.tilePosition.x;
  deltaVec.y += dstPosition.tilePosition.y - srcPosition.tilePosition.y;
  
  return deltaVec;
}

WorldPosition
WorldPosition::operator+(Vec2i vector) const
{
  WorldPosition result = *this;
  result.tilePosition += vector;
  
  return result;
}

WorldPosition
WorldPosition::operator-(Vec2i vector) const
{
  WorldPosition result = *this;
  result.tilePosition -= vector;
  
  return result;
}

void
WorldPosition::operator+=(Vec2i vector)
{
  tilePosition += vector;
}

void
WorldPosition::operator-=(Vec2i vector)
{
  tilePosition -= vector;
}

bool
WorldPosition::operator==(const WorldPosition& worldPosition) const
{
  if(tileChunkPosition == worldPosition.tileChunkPosition &&
     tilePosition == worldPosition.tilePosition) return true;
  return false;
}

void
EntityPosition::recanonicalize(const Vec2i tileChunkSize)
{
  while(tileOffset.x >= 1.0f)
  {
    tileOffset.x -= 1.0f;
    ++worldPosition.tilePosition.x;
  }
  
  while(tileOffset.y >= 1.0f)
  {
    tileOffset.y -= 1.0f;
    ++worldPosition.tilePosition.y;
  }
  
  while(tileOffset.x < 0)
  {
    tileOffset.x += 1.0f;
    --worldPosition.tilePosition.x;
  }
  
  while(tileOffset.y < 0)
  {
    tileOffset.y += 1.0f;
    --worldPosition.tilePosition.y;
  }

  worldPosition.recanonicalize(tileChunkSize);
}

Vec2f
EntityPosition::calculateDistanceInTiles(const EntityPosition& srcPosition,
						  const EntityPosition& dstPosition,
						  const Vec2i& tileChunkDimensions)
{
  Vec2f deltaVec;

  // Adding Chunk Tiles
  deltaVec.x = (dstPosition.worldPosition.tileChunkPosition.x -
		   srcPosition.worldPosition.tileChunkPosition.x) * tileChunkDimensions.x;
  
  deltaVec.y = (dstPosition.worldPosition.tileChunkPosition.y -
		   srcPosition.worldPosition.tileChunkPosition.y) * tileChunkDimensions.y;
  
  // Adding Tiles Inside Of The Chunk
  deltaVec.x += dstPosition.worldPosition.tilePosition.x - srcPosition.worldPosition.tilePosition.x;
  deltaVec.y += dstPosition.worldPosition.tilePosition.y - srcPosition.worldPosition.tilePosition.y;

  // Adding Tile Offsets
  deltaVec.x += dstPosition.tileOffset.x - srcPosition.tileOffset.x;
  deltaVec.y += dstPosition.tileOffset.y - srcPosition.tileOffset.y;
  
  return deltaVec;
}

void
EntityPosition::operator+=(const Vec2f& vector)
{
  tileOffset += vector;
}

void
EntityPosition::operator-=(const Vec2f& vector)
{
  tileOffset -= vector;
}

EntityPosition
EntityPosition::operator+(const Vec2f& vector) const 
{
  EntityPosition result = *this;
  result += vector;
  return result;
}
