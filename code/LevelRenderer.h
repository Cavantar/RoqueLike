#pragma once

#include <SFML/Graphics.hpp>
#include "Level.h"
#include "SpriteManager.h"


class EntityRenderThing{
public:
  union {
    struct{
      const EntityRenderData& entityRenderData;
      Vector2f entityPositionOnScreen;
      Vector2f dimensions;
    };
    struct {
      TILE_TYPE tileType;
      Vector2f screenTilePosition;
      void* helperPointer;
    };
  };  
  EntityRenderThing(const EntityRenderData& entityRenderData,
		    Vector2f entityPositionOnScreen,
		    const Vector2f& dimensions) :
    entityRenderData(entityRenderData),
    entityPositionOnScreen(entityPositionOnScreen),
    dimensions(dimensions) {}
};
typedef std::list<EntityRenderThing> EntityListForRendering;

// Comparison function for sorting
bool compareEntityRenderThing(const EntityRenderThing& ent1, const EntityRenderThing& ent2);

class LevelRenderer{
public:
  LevelRenderer();
  
  void setWindow(sf::RenderWindow* window) { this->window = window; }
  void setTileSize(const float tileSizeInPixels) { this->tileSizeInPixels = tileSizeInPixels; }
  void setSpriteManager(SpriteManager* spriteManager) { this->spriteManager = spriteManager; }
  
  void renderLevel(const LevelPtr& level, EntityPosition& cameraPosition);
  sf::Font* getFont() { return &font;}
  
private:
  sf::Font font;
  
  float tileSizeInPixels;
  sf::RenderWindow* window;
  SpriteManager* spriteManager;
  Level* level;
  
  EntityListForRendering entityListForRendering;
  
  Vector2f getEntityPositionOnScreen(const EntityPtr& entity, EntityPosition& cameraPosition,
				     const Vector2i& tileChunkSize) const;
  
  // Getting the list of entities containing all the information to render them as well
  // as sort by their bottomY value
  EntityListForRendering getEntityListForRendering(const EntityList& entityList,
						   EntityPosition& cameraPosition,
						   const Vector2i& tileChunkSize);
  
  // Renders Entities that are in bounds of a chunk that is rendered
  void renderEntitiesIfBelowBoundary(float boundaryY, FloatRect acceptedPositionBoundary);
  
  
  void renderTileChunk(const TileChunkPtr& tileChunk, const Vector2f& screenChunkPosition,
		       const Vector3i& tileChunkPosition);
  
  void renderTileMap(const TileMapPtr& tileMap, EntityPosition& cameraPosition);
  
  void renderEntity(const EntityRenderData& entityRenderData, Vector2f entityPositionOnScreen);
  
  void renderEntities(const EntityList& entityList, EntityPosition& cameraPosition,
		      const Vector2i& tileChunkSize);

  
};
