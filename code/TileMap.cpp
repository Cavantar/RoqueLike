#include <iostream>

#include "TileMap.h"

TileChunk::TileChunk(const uint32 width, const uint32 height)
{
  tileChunkData.resize(height);
  for(int i=0; i<width; i++)
  {
    tileChunkData[i].resize(width);
  }
}

TILE_TYPE TileChunk::getTileType(const Vector2i& tilePosition) const
{
  return tileChunkData[tilePosition.y][tilePosition.x];
}

void TileChunk::setTileType(const Vector2i& tilePosition,
			    const TILE_TYPE tileValue)
{
  tileChunkData[tilePosition.y][tilePosition.x] = tileValue;
}

void TileMap::setTileType(WorldPosition& tileWorldPosition, const TILE_TYPE tileType)
{

  tileWorldPosition.recanonicalize(tileChunkSize);
  
  // If it chunk doesn't exist it has be created
  if(!tileChunkMap.count(tileWorldPosition.tileChunkPosition))
  {
    tileChunkMap[tileWorldPosition.tileChunkPosition] = TileChunkPtr(new TileChunk(tileChunkSize.x, tileChunkSize.y));
  }
  
  return tileChunkMap[tileWorldPosition.tileChunkPosition]->setTileType(tileWorldPosition.tilePosition, tileType);
}

TILE_TYPE TileMap::getTileType(WorldPosition& tileWorldPosition) 
{
  tileWorldPosition.recanonicalize(tileChunkSize);
  
  // If The Chunk Doesn't exist we return void tile type  
  if(!tileChunkMap.count(tileWorldPosition.tileChunkPosition))
  {
    return TILE_TYPE_VOID;
  }
  else
  {
    return tileChunkMap[tileWorldPosition.tileChunkPosition]->getTileType(tileWorldPosition.tilePosition);
  }
  
}

bool TileMap::isRectangleOfTileType(WorldPosition startPosition,
					   Vector2i dimensions, TILE_TYPE tileType)
{
  for(int y = 0; y < dimensions.y; y++)
  {
    for(int x = 0; x < dimensions.x; x++)
    {
      if(getTileType(startPosition + Vector2i(x, y)) != tileType )
      {
	return false;
      }
    }
  }
  return true;
}
