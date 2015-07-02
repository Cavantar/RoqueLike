#pragma once

#include <jpb/Vector.h>

class WorldPosition{
 public:
  
  // Position of a TileChunk
  Vec3i tileChunkPosition;

  // Position of a Tile in a TileChunk 
  Vec2i tilePosition;
  
 WorldPosition(const Vec3i& tileChunkPosition, const Vec2i& tilePosition)
   : tileChunkPosition(tileChunkPosition), tilePosition(tilePosition) {}
  
  WorldPosition() {}

  // Changes both tileChunkPosition as well as tilePosition when tilePosition bounds are left
  void recanonicalize(const Vec2i tileChunkSize);

  // Calculates tile distance including those at both ends
  static Vec2i calculateDistanceInTilesInclusive(const WorldPosition& srcPosition,
						    const WorldPosition& dstPosition,
						    const Vec2i& tileChunkDimensions);
  
  WorldPosition operator+(Vec2i vector) const;
  WorldPosition operator-(Vec2i vector) const;
  
  void operator+=(Vec2i vector);
  void operator-=(Vec2i vector);
  
  bool operator==(const WorldPosition& worldPosition) const;
};

class EntityPosition{
 public:
  // Determines The Tile In The World
  WorldPosition worldPosition;
  
  // Offset From A Current Tile
  Vec2f tileOffset;
  
 EntityPosition(WorldPosition& worldPosition = WorldPosition(),
		Vec2f& tileOffset = Vec2f()) : worldPosition(worldPosition), tileOffset(tileOffset) {}
  
  // Changes both tileChunkPosition as well as tilePosition when tilePosition bounds are left
  // Based on tileOffset
  void recanonicalize(const Vec2i tileChunkSize);
  static Vec2f calculateDistanceInTiles(const EntityPosition& srcPosition,
					   const EntityPosition& dstPosition,
					   const Vec2i& tileChunkDimensions);
  
  void operator+=(const Vec2f& vector);
  void operator-=(const Vec2f& vector);
  EntityPosition operator+(const Vec2f& vector) const ;
};
