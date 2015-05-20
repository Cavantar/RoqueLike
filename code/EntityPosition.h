#pragma once

#include "Vector.h"

class WorldPosition{
 public:
  
  // Position of a TileChunk
  Vector3i tileChunkPosition;

  // Position of a Tile in a TileChunk 
  Vector2i tilePosition;
  
 WorldPosition(const Vector3i& tileChunkPosition, const Vector2i& tilePosition)
   : tileChunkPosition(tileChunkPosition), tilePosition(tilePosition) {}
  
  WorldPosition() {}

  // Changes both tileChunkPosition as well as tilePosition when tilePosition bounds are left
  void recanonicalize(const Vector2i tileChunkSize);

  // Calculates tile distance including those at both ends
  static Vector2i calculateDistanceInTilesInclusive(const WorldPosition& srcPosition,
						    const WorldPosition& dstPosition,
						    const Vector2i& tileChunkDimensions);
  
  WorldPosition operator+(Vector2i vector) const;
  WorldPosition operator-(Vector2i vector) const;
  
  void operator+=(Vector2i vector);
  void operator-=(Vector2i vector);
  
  bool operator==(const WorldPosition& worldPosition) const;
};

class EntityPosition{
 public:
  // Determines The Tile In The World
  WorldPosition worldPosition;
  
  // Offset From A Current Tile
  Vector2f tileOffset;
  
 EntityPosition(WorldPosition& worldPosition = WorldPosition(),
		Vector2f& tileOffset = Vector2f()) : worldPosition(worldPosition), tileOffset(tileOffset) {}
  
  // Changes both tileChunkPosition as well as tilePosition when tilePosition bounds are left
  // Based on tileOffset
  void recanonicalize(const Vector2i tileChunkSize);
  static Vector2f calculateDistanceInTiles(const EntityPosition& srcPosition,
					   const EntityPosition& dstPosition,
					   const Vector2i& tileChunkDimensions);
  
  void operator+=(const Vector2f& vector);
  void operator-=(const Vector2f& vector);
  EntityPosition operator+(const Vector2f& vector) const ;
};
