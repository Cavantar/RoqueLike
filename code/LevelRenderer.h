#pragma once

#include <SFML/Graphics.hpp>
#include "Level.h"
#include "SpriteManager.h"

typedef std::list<sf::Sprite> SpriteList;

class LevelRenderer;

// Render Thing Stuff
// ----------------------

class RenderThing {
public:
  RenderThing(bool entityThing, float bottomY) : entityThing(entityThing), bottomY(bottomY) {};
  virtual ~RenderThing() {};
  virtual bool isEntity() { return entityThing; }
  
  virtual void render(LevelRenderer* levelRenderer) = 0;
  
  bool entityThing;
  float bottomY;
};

typedef std::shared_ptr<RenderThing> RenderThingPtr;

class EntityRenderThing : public RenderThing {
public:
  const EntityRenderData* entityRenderData;
  Vector2f entityPositionOnScreen;
  Vector2f dimensions;
  
  EntityRenderThing(float bottomY, const EntityRenderData* entityRenderData,
		    Vector2f entityPositionOnScreen, Vector2f dimensions) :
    RenderThing(true, bottomY), entityRenderData(entityRenderData),
    entityPositionOnScreen(entityPositionOnScreen), dimensions(dimensions) {}
  
  void render(LevelRenderer* levelRenderer);
};

class SpriteRenderThing : public RenderThing {
public:
  SpriteList sprites;
  SpriteRenderThing(const SpriteList& sprites, float bottomY) :
    RenderThing(false, bottomY), sprites(sprites) {}
  
  void render(LevelRenderer* levelRenderer);
};
typedef std::list<RenderThingPtr> EntityListForRendering;

// Comparison function for sorting
bool compareEntityRenderThing(const RenderThingPtr& ent1, const RenderThingPtr& ent2);

// ----------------------


class LevelRenderer{
public:
  friend class EntityRenderThing;
  friend class SpriteRenderThing;
  
  LevelRenderer();
  
  void setWindow(sf::RenderWindow* window) { this->window = window; }
  void setTileSize(const float tileSizeInPixels) { this->tileSizeInPixels = tileSizeInPixels; }
  void setSpriteManager(SpriteManager* spriteManager) { this->spriteManager = spriteManager; }
  
  void renderLevel(const LevelPtr& level, EntityPosition& cameraPosition);
  sf::Font* getFont() { return &font;}

  // returns index of sprite that should rendered for given tileState
  int getSpriteIndex(TILE_STATE tileState, int tileHash);
  
private:
  sf::Font font;
  
  float tileSizeInPixels;
  sf::RenderWindow* window;
  SpriteManager* spriteManager;
  Level* level;

  // Gets the position of an entity in the world 
  Vector2f getEntityPositionOnScreen(const EntityPtr& entity, EntityPosition& cameraPosition,
				     const Vector2i& tileChunkSize) const;
  
  // Getting the list of entities containing all the information to render them as well
  // as sort by their bottomY value
  EntityListForRendering getEntityListForRendering(const EntityList& entityList,
						   EntityPosition& cameraPosition,
						   const Vector2i& tileChunkSize);
  
  // Renders Entities that are in bounds of a chunk that is rendered
  void renderSortedEntities(EntityListForRendering& entityListForRendering);

  // Returns List Of RenderObjects For Tiles that have to be sorted for rendering and renders those who don't
  EntityListForRendering renderTileChunk(const TileChunkPtr& tileChunk, const Vector2f& screenChunkPosition,
					 const Vector3i& tileChunkPosition);
  
  EntityListForRendering renderTileMap(const TileMapPtr& tileMap, EntityPosition& cameraPosition);
  
  void renderEntity(const EntityRenderData* entityRenderData, Vector2f entityPositionOnScreen);
  void renderEntities(const EntityList& entityList, EntityPosition& cameraPosition,
		      const Vector2i& tileChunkSize);

  // Render list of previously filled spriteObjects
  void renderSprites(const SpriteList& spriteList);
};
