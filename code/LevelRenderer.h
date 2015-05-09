#pragma once

#include <SFML/Graphics.hpp>
#include "Level.h"

class LevelRenderer{
 public:
  LevelRenderer();
  
  void setWindow(sf::RenderWindow* window) { this->window = window; }
void setTileSize(const float tileSizeInPixels) { this->tileSizeInPixels = tileSizeInPixels; }

void renderLevel(const LevelPtr& level, EntityPosition& cameraPosition);

 private:
  sf::Font font;
  
  float tileSizeInPixels;
  sf::RenderWindow* window;
  
  void renderTileChunk(const TileChunkPtr& tileChunk, const Vector2f& screenChunkPosition,
		       const float tileSize);
  void renderTileMap(const TileMapPtr& tileMap, EntityPosition& cameraPosition);
    
  void render(const EntityRenderData& entityRenderData, Vector2f entityPositionOnScreen);
  
  void renderEntity(const EntityPtr& entity, EntityPosition& cameraPosition,
		    const Vector2i& tileChunkSize);
  
  void renderEntities(const EntityList& entityList, EntityPosition& cameraPosition,
		      const Vector2i& tileChunkSize);
  
};
