#pragma once

#include <vector>
#include <memory>
#include <unordered_map>

#include "EntityPosition.h"

enum TILE_TYPE{
  TILE_TYPE_VOID,
  TILE_TYPE_WALL,
  TILE_TYPE_STONE_GROUND,
  TILE_TYPE_STONE_ICE_GROUND,
  TILE_TYPE_STONE_SPEED_GROUND
};

// TileChunkData[y][x] accessor order
typedef std::vector<std::vector<TILE_TYPE>> TileChunkData;

class TileChunk{
 private:
  TileChunkData tileChunkData;
  
public:
  TileChunk(const uint32 width, const uint32 height);
  
  TILE_TYPE getTileType(const Vec2i& tilePosition) const;
  void setTileType(const Vec2i& tilePosition, const TILE_TYPE tileType);
  
  const TileChunkData& getTileChunkData() const { return tileChunkData; } 
};

typedef std::shared_ptr<TileChunk> TileChunkPtr;
typedef std::unordered_map<Vec3i, TileChunkPtr> TileChunkMap;

class TileMap{
  friend class LevelGenerator;
  friend class SimpleLevelGenerator;
  
public:
  TileMap(const Vec2i tileChunkSize) :
    tileChunkSize(tileChunkSize) {}
  
  void setTileType(WorldPosition& tileWorldPosition, const TILE_TYPE tileType);
  bool isRectangleOfTileType(WorldPosition startPosition, Vec2i dimensions, TILE_TYPE tileType); 
  
  // Can't Be const because of hashmap,
  TILE_TYPE getTileType(WorldPosition& tileWorldPosition);
  const TileChunkMap& getTileChunkMap() const { return tileChunkMap; }
  
  void recanonicalize(EntityPosition& entityPosition) const;
  
  const Vec2i& getTileChunkSize() const { return tileChunkSize; }
  
private:
  Vec2i tileChunkSize;
  TileChunkMap tileChunkMap;
};

typedef std::shared_ptr<TileMap> TileMapPtr;
