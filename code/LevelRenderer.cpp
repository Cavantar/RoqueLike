#include "LevelRenderer.h"
#include <iostream>
#include "Profiler.h"
#include "Noise.h"

void EntityRenderThing::render(LevelRenderer* levelRenderer)
{
  levelRenderer->renderEntity(entityRenderData, entityPositionOnScreen);
}

void SpriteRenderThing::render(LevelRenderer* levelRenderer)
{
  levelRenderer->renderSprites(sprites);
}

bool compareEntityRenderThing(const RenderThingPtr& ent1, const RenderThingPtr& ent2)
{
  return ent1->bottomY < ent2->bottomY;
}

LevelRenderer::LevelRenderer() : window(NULL), tileSizeInPixels(0)
{
  bool loadedFont = font.loadFromFile("chiller.ttf");
  assert(loadedFont);
}

void
LevelRenderer::renderLevel(const LevelPtr& level, EntityPosition& cameraPosition)
{
  assert(window);
  
  this->level = level.get();
  
  const TileMapPtr& tileMap = level->getTileMap();

  EntityListForRendering entitiesForRendering = getEntityListForRendering(level->getEntityList(0), cameraPosition,
									  tileMap->getTileChunkSize());

  SfmlProfiler::get()->start("BasicTileRender");
  EntityListForRendering tilesForRendering = renderTileMap(tileMap, cameraPosition);
  SfmlProfiler::get()->end("BasicTileRender");
  
  // Combining both lists
  entitiesForRendering.splice(entitiesForRendering.end(), tilesForRendering);
  entitiesForRendering.sort(compareEntityRenderThing);
  
  SfmlProfiler::get()->start("SortRender");
  renderSortedEntities(entitiesForRendering);
  SfmlProfiler::get()->end("SortRender");

  entitiesForRendering.clear();
  
  // renderEntities(level->getEntityList(0), cameraPosition,
  // 		   tileMap->getTileChunkSize());
  
  renderEntities(level->getEntityList(1), cameraPosition,
		 tileMap->getTileChunkSize());
  
}

int LevelRenderer::getSpriteIndex(TILE_STATE tileState, int tileHash)
{
  int spriteIndex;
  
  switch(tileState)
  {
  case TS_CORNER_TOPLEFT:
    spriteIndex = 2;
    break;
  case TS_CORNER_TOPRIGHT:
    spriteIndex = 3;
    break;
  case TS_CORNER_BOTTOMLEFT:
    spriteIndex = 4;
    break;
  case TS_CORNER_BOTTOMRIGHT:
    spriteIndex = 5;
    break;
  case TS_WALL_LEFT:
    spriteIndex = 6 + (tileHash % 3);
    break;
  case TS_WALL_TOP:
    spriteIndex = 9 + (tileHash % 3);
    break;
  case TS_WALL_BOTTOM:
    spriteIndex = 12 + (tileHash % 3);
    break;
  case TS_WALL_RIGHT:
    spriteIndex = 15 + (tileHash % 3);
    break;
  case TS_ONE_WAY_LEFT:
    spriteIndex = 18;
    break;
  case TS_ONE_WAY_UP:
    spriteIndex = 19;
    break;
  case TS_ONE_WAY_RIGHT:
    spriteIndex = 20;
    break;
  case TS_ONE_WAY_DOWN:
    spriteIndex = 21;
    break;
  case TS_PATH_HORIZONTAL:
    spriteIndex = 22 + (tileHash % 2);
    break;
  case TS_PATH_VERTICAL:
    spriteIndex = 24 + (tileHash % 2);
    break;
  case TS_SURROUNDED_BY_OTHERS:
    spriteIndex = 26;
    break;
  case TS_CORNER_EDGE_TOPLEFT:
    spriteIndex = 27;
    break;
  case TS_CORNER_EDGE_TOPRIGHT:
    spriteIndex = 28;
    break;
  case TS_CORNER_EDGE_BOTTOMLEFT:
    spriteIndex = 29;
    break;
  case TS_CORNER_EDGE_BOTTOMRIGHT:
    spriteIndex = 30;
    break;
  case TS_CORNERP_EDGE_TOPLEFT:
    spriteIndex = 31;
    break;
  case TS_CORNERP_EDGE_TOPRIGHT:
    spriteIndex = 32;
    break;
  case TS_CORNERP_EDGE_BOTTOMLEFT:
    spriteIndex = 33;
    break;
  case TS_CORNERP_EDGE_BOTTOMRIGHT:
    spriteIndex = 34;
    break;
  case TS_CORNERT_DOWN:
    spriteIndex = 39;
    break;
  case TS_CORNERT_RIGHT:
    spriteIndex = 40;
    break;
  case TS_CORNERT_LEFT:
    spriteIndex = 41;
    break;
  case TS_CORNERT_UP:
    spriteIndex = 42;
    break;
  case TS_SURROUNDED_BY_SELF:
    {      
      assert(0);
    } break;
  default:

    // Path Variation 
    if(tileState & TS_PATH_HORIZONTAL && !(tileState & TS_PATH_VERTICAL) && (tileState & ST_CORNERS))
    {
      spriteIndex = 22 + (tileHash % 2);
    }
    else if(tileState & TS_PATH_VERTICAL && !(tileState & TS_PATH_HORIZONTAL) && (tileState & ST_CORNERS))
    {
      spriteIndex = 24 + (tileHash % 2);
    }
    else spriteIndex  = 1;//(rand() % 5) + 1;
  }
  return spriteIndex;
  
}

Vector2f
LevelRenderer::getEntityPositionOnScreen(const EntityPtr& entity, EntityPosition& cameraPosition,
					 const Vector2i& tileChunkSize) const
{
  const sf::Vector2u windowDimensions = window->getSize();
  
  float tileChunkWidthInPixels = (float)tileChunkSize.x * tileSizeInPixels;
  float tileChunkHeightInPixels = (float)tileChunkSize.y * tileSizeInPixels;
  
  // cameraPosition identifies center of the viewport so we have to translate it
  
  float tilesPerScreenWidth = (float)windowDimensions.x/tileSizeInPixels;
  float tilesPerScreenHeight = (float)windowDimensions.y/tileSizeInPixels;
  
  EntityPosition topLeftViewport = cameraPosition;
  topLeftViewport.tileOffset.x -= tilesPerScreenWidth / 2.0f;
  topLeftViewport.tileOffset.y -= tilesPerScreenHeight / 2.0f;
  
  topLeftViewport.recanonicalize(tileChunkSize);
  
  // CameraPosition in Pixels Inside The Chunk
  Vector2f cameraOffset((float) topLeftViewport.worldPosition.tilePosition.x * tileSizeInPixels,
			(float) topLeftViewport.worldPosition.tilePosition.y * tileSizeInPixels);
  
  const EntityPosition& position = entity->getPosition();
  Vector2f entityPositionOnScreen = EntityPosition::calculateDistanceInTiles(topLeftViewport,
									     position,
									     tileChunkSize);
  // Converting Position To Pixels
  entityPositionOnScreen *= tileSizeInPixels;

  return entityPositionOnScreen;
}

EntityListForRendering
LevelRenderer::getEntityListForRendering(const EntityList& entityList,
				       EntityPosition& cameraPosition,
				       const Vector2i& tileChunkSize)
{
  EntityListForRendering resultEntityList;
  
  for(auto entityIt = entityList.begin() ; entityIt != entityList.end() ; entityIt++)
  {
    const EntityRenderData* entityRenderData = (*entityIt)->getRenderData();
    Vector2f entityPositionOnScreen = getEntityPositionOnScreen(*entityIt, cameraPosition, tileChunkSize);
    Vector2f dimensions = (*entityIt)->getDimensions() * tileSizeInPixels;

    
    RenderThing* renderThing = new EntityRenderThing(entityPositionOnScreen.y + dimensions.y,
						     entityRenderData, entityPositionOnScreen,
						     dimensions);
    
    resultEntityList.push_back(RenderThingPtr(renderThing));
  }
  
  return resultEntityList;
}

void
LevelRenderer::renderSortedEntities(EntityListForRendering& entityListForRendering)
{
  
  for(auto it = entityListForRendering.begin(); it != entityListForRendering.end(); it++)
  {
    (*it)->render(this);
  }
  entityListForRendering.clear();
}

EntityListForRendering
LevelRenderer::renderTileChunk(const TileChunkPtr& tileChunk, const Vector2f& screenChunkPosition,
			       const Vector3i& tileChunkPosition)
{
  EntityListForRendering tilesForSortedInChunk;
  
  sf::RectangleShape rectangleShape = sf::RectangleShape(sf::Vector2f(tileSizeInPixels, tileSizeInPixels));
  
  const TileChunkData& tileChunkData = tileChunk->getTileChunkData();
  const sf::Vector2u windowDimensions = window->getSize();
  
  static const float wallHeight = 2.0f;
  
  int tileChunkHeight = tileChunkData.size();
  int tileChunkWidth = tileChunkData[0].size();

  int minX = 0;
  int maxX = tileChunkWidth;
  
  int minY = 0;
  int maxY = tileChunkHeight;

  if(screenChunkPosition.x < 0) minX = (-screenChunkPosition.x / tileSizeInPixels);
  if(screenChunkPosition.y < 0) minY = (-screenChunkPosition.y / tileSizeInPixels);
  
  if(screenChunkPosition.x > windowDimensions.x)
    maxX -= (screenChunkPosition.x - windowDimensions.x) / tileSizeInPixels;
  
  if(screenChunkPosition.y > windowDimensions.y)
    maxY -= (screenChunkPosition.y - windowDimensions.y) / tileSizeInPixels;
  
  sf::Vector2f screenTilePosition = sf::Vector2f(screenChunkPosition.x, screenChunkPosition.y);
  rectangleShape.setPosition(screenTilePosition);
    
  for(int y = minY; y < maxY; ++y)
  {
    
    for(int x = minX; x < maxX; ++x)
    {
      
      if(tileChunkData[y][x] == TILE_TYPE_VOID)
      {
	continue;
      }
      
      screenTilePosition = sf::Vector2f(screenChunkPosition.x + (x * tileSizeInPixels),
					screenChunkPosition.y + (y * tileSizeInPixels));
      
      rectangleShape.setPosition(screenTilePosition);

      	  
      NoiseParams noiseParams = {0.05, 3, 2.0f, 0.5f};
      Vector2f globalTilePosition(x + tileChunkPosition.x * tileChunkWidth,
				  y + tileChunkPosition.y * tileChunkHeight);
	  
      float floatHash = Noise::sumPerlin(globalTilePosition, noiseParams);
      floatHash = (floatHash + 1.0f) / 2.0f;
	  
      int tileHash = (int)(floatHash * 100.0f);
      tileHash = abs(tileHash);

      TILE_TYPE tileType = tileChunkData[y][x];
      
      switch(tileType)
      {
      case TILE_TYPE_WALL:
	{

	  screenTilePosition.y -= (wallHeight - 1.0f) * tileSizeInPixels;
	  
	  sf::Sprite tileSprite;
	  float finalScale = tileSizeInPixels / 16.0f;
	  
	  WorldPosition tempWorldPosition(tileChunkPosition, Vector2i(x, y));
	  std::string spriteName = "wallTop1_";
	  
	  TILE_STATE tileState = (TILE_STATE)level->getSurroundingTileData(tempWorldPosition, TILE_TYPE_WALL);
	  
	  int spriteIndex = -1;
	  if(tileState != TS_SURROUNDED_BY_SELF)
	  {
	    spriteIndex = getSpriteIndex(tileState, tileHash);
	  }
	  else spriteIndex = 1;
	  
	  spriteName += std::to_string(spriteIndex);
	  tileSprite = spriteManager->getSprite(spriteName);
	  
	  tileSprite.setScale(finalScale, finalScale);
	  tileSprite.setPosition(screenTilePosition - sf::Vector2f(0, tileSizeInPixels));

	  SpriteList spriteList;
	  spriteList.push_back(tileSprite);
	  
	  //window->draw(tileSprite);
	  
	  if(!(tileState & ST_SOUTH))
	  {
	    std::string spriteName = "wall1_";
	    
	    float ordinaryWallPercentage = 70.0f;
	    
	    int tileKind = ((x ^ y ^ (int)tileChunk.get()));
	    tileKind = abs(tileKind);
	    
	    if(tileHash < ordinaryWallPercentage)
	    {
	      // 2, 4, 5
	      tileKind = 2 + ((tileHash) % 3);
	      if(tileKind != 2) tileKind ++;
	    }
	    else 
	    {
	      tileKind = 3;
	      //if(tileKind == 2) tileKind = 6;
	      //if(tileKind == 3) tileKind = 7;
	    }
	    
	    spriteName += std::to_string(tileKind);
	    tileSprite = spriteManager->getSprite(spriteName);
	    
	    tileSprite.setScale(finalScale, finalScale);
	    tileSprite.setPosition(screenTilePosition);
	    
	    spriteList.push_back(tileSprite);
	    
	    //window->draw(tileSprite);
	  }
	  
	  RenderThing* renderThing = new SpriteRenderThing(spriteList,
							   screenTilePosition.y + tileSizeInPixels * 2);
	  
	  tilesForSortedInChunk.push_back(RenderThingPtr(renderThing));
	  
	} break;
      case TILE_TYPE_STONE_SPEED_GROUND:
      case TILE_TYPE_STONE_ICE_GROUND:
      case TILE_TYPE_STONE_GROUND:
	{
	  rectangleShape.setSize(sf::Vector2f(tileSizeInPixels, tileSizeInPixels));
	  rectangleShape.setFillColor(sf::Color(128, 128, 128));
	  
	  /*
	    
	    static const int p1 = 73856093;
	    static const int p2 = 19349663;
	    static const int p3 = 83492791;
	    
	    int tileHash = (x * p1) ^ (y * p2) ^ ((int)tileChunk.get() * p3);
	    
	  */	  
	  
	  std::string spriteName = "floor1_";
	  WorldPosition tempWorldPosition(tileChunkPosition, Vector2i(x, y));
	  
	  TILE_STATE tileState = (TILE_STATE)level->getSurroundingTileData(tempWorldPosition, tileType);
	  int spriteIndex = -1;
	  if(tileState != TS_SURROUNDED_BY_SELF)
	  {
	    spriteIndex = getSpriteIndex(tileState, tileHash);
	  }
	  else
	  {
	    // Common Tile Occurence Chance
	    static const float commonTileChance = 65.0f;
	    if(tileHash >  commonTileChance && ((x ^ y ^ (int)tileChunk.get()) % 3) == 0)
	    {
	      spriteIndex = 66 + (tileHash % 4);
	    }
	    else
	    {
	      spriteIndex = 62 + (tileHash % 4);
	    }
	  }
	  
	  spriteName += std::to_string(spriteIndex);
	  sf::Sprite tileSprite = spriteManager->getSprite(spriteName);

	  if(tileType == TILE_TYPE_STONE_ICE_GROUND) tileSprite.setColor(sf::Color(165, 242, 243));
	  if(tileType == TILE_TYPE_STONE_SPEED_GROUND) tileSprite.setColor(sf::Color(250,128,114));
	  
	  sf::Color maxColor = sf::Color::Red;
	  sf::Color minColor = sf::Color::Green;
	  
	  // if(tileHash > 65) tileSprite.setColor(maxColor);
	  // else tileSprite.setColor(minColor);
	  
	  float finalScale = tileSizeInPixels / 16.0f;
	  
	  tileSprite.setScale(finalScale, finalScale);
	  tileSprite.setPosition(screenTilePosition);
	  
	  window->draw(tileSprite);
	  
	} break;
	
      } // switch
    }
  }
  
  return tilesForSortedInChunk;
}

EntityListForRendering
LevelRenderer::renderTileMap(const TileMapPtr& tileMap, EntityPosition& cameraPosition)
{
  EntityListForRendering tilesForSortedRendering;
  
  const sf::Vector2u windowDimensions = window->getSize();
  const TileChunkMap& tileChunkMap = tileMap->getTileChunkMap();
  
  // To Determine How many Chunks I have to render, I have to know their width in pixels
  Vector2i tileChunkSize = tileMap->getTileChunkSize();
  Vector2f tileChunkSizeInPixels(tileChunkSize.x, tileChunkSize.y);
  tileChunkSizeInPixels *= tileSizeInPixels;
  
  // How many Chunks I have to render
  float chunksPerScreenWidth = (float)windowDimensions.x/tileChunkSizeInPixels.x;
  float chunksPerScreenHeight = (float)windowDimensions.y/tileChunkSizeInPixels.y;
  
  // cameraPosition identifies center of the viewport so we have to translate it
  cameraPosition.recanonicalize(tileChunkSize);
  
  float tilesPerScreenWidth = (float)windowDimensions.x/tileSizeInPixels;
  float tilesPerScreenHeight = (float)windowDimensions.y/tileSizeInPixels;
  
  EntityPosition topLeftViewport = cameraPosition;
  topLeftViewport.tileOffset.x -= tilesPerScreenWidth / 2.0f;
  topLeftViewport.tileOffset.y -= tilesPerScreenHeight / 2.0f;
  
  topLeftViewport.recanonicalize(tileChunkSize);
  
  // CameraPosition in Pixels Inside The Chunk - determines how much i have to translate chunks 
  Vector2f cameraOffset((float) topLeftViewport.worldPosition.tilePosition.x * tileSizeInPixels,
			(float) topLeftViewport.worldPosition.tilePosition.y * tileSizeInPixels);
  
  // Determining UpperLeftCorner and LowerRightCorner Chunks
  Vector3i topLeftChunkPosition = topLeftViewport.worldPosition.tileChunkPosition;
  Vector3i bottomRightChunkPosition = topLeftChunkPosition +
    Vector3i(ceil(chunksPerScreenWidth) + 1, ceil(chunksPerScreenHeight) + 1, 0);

  //Vector2f cameraPositionInPixels = 
  
  for(int y = topLeftChunkPosition.y; y < bottomRightChunkPosition.y; y++)
  {
    
    for(int x = topLeftChunkPosition.x; x < bottomRightChunkPosition.x; x++)
    {
      
      sf::CircleShape shape = sf::CircleShape(tileChunkSizeInPixels.x / 2.0f);
      shape.setScale(1.0f, (float)tileChunkSize.y / (float)tileChunkSize.x);
      shape.setFillColor(sf::Color(128 - ((abs(y)%8) * 16), 0 , 128 - ((abs(x)%8) * 16)));
      
      Vector2f screenChunkPosition;
      
      // screenChunkPosition is composed of numbOfChunk that should be rendered multiplied by pixel width
      // subtracted by how much chunk is out of screen(offset) in Pixels
      // subtracted by subTile Offset in Pixels

      screenChunkPosition.x = (x - topLeftChunkPosition.x) * tileChunkSizeInPixels.x -
	cameraOffset.x - topLeftViewport.tileOffset.x * tileSizeInPixels;
      
      screenChunkPosition.y = (y - topLeftChunkPosition.y) * tileChunkSizeInPixels.y -
	cameraOffset.y - topLeftViewport.tileOffset.y * tileSizeInPixels;
      
      shape.setPosition(screenChunkPosition.x, screenChunkPosition.y);
      window->draw(shape);

      Vector3i tileChunkPosition(x, y, cameraPosition.worldPosition.tileChunkPosition.z);
      
      // If The Chunk Doesn't Exist We don't render anything
      if(tileChunkMap.count(tileChunkPosition))
      {
	EntityListForRendering tilesForSortedInChunk = renderTileChunk(tileChunkMap.at(tileChunkPosition),
								       screenChunkPosition, tileChunkPosition);

	// It Moves Data 
	tilesForSortedRendering.splice(tilesForSortedRendering.end(), tilesForSortedInChunk);
      }
    }
  }
  
  return tilesForSortedRendering;
}

void
LevelRenderer::renderEntity(const EntityRenderData* entityRenderData, Vector2f entityPositionOnScreen)
{
  const sf::Vector2u windowDimensions = window->getSize();
  
  switch(entityRenderData->type) {
  case ER_PRIMITIVE:
    {
      const PrimitiveRenderData& primitiveRenderData = *((PrimitiveRenderData*)entityRenderData);
      
      const Vector2f& entityDimensions = primitiveRenderData.dimensionsInTiles;
      const sf::Color entityColor(primitiveRenderData.color.x, primitiveRenderData.color.y, primitiveRenderData.color.z,
				  primitiveRenderData.colorAlpha * 255.0f);

      float outlineThickness = primitiveRenderData.outlineThickness;
      
      switch(primitiveRenderData.primitiveType) {
      case PT_RECTANGLE:
	{
	  sf::RectangleShape rectangleShape;
	  
	  rectangleShape.setOutlineThickness(outlineThickness * (tileSizeInPixels / 40.0f));
	  rectangleShape.setSize(sf::Vector2f(entityDimensions.x * tileSizeInPixels,
					      entityDimensions.y * tileSizeInPixels));
	  
	  rectangleShape.setFillColor(entityColor);
	  rectangleShape.setPosition(entityPositionOnScreen.x, entityPositionOnScreen.y);
	  
	  window->draw(rectangleShape);
	} break;
      case PT_CIRCLE:
	{
	  sf::CircleShape circleShape;
	  
	  circleShape.setOutlineThickness(outlineThickness * (tileSizeInPixels / 40.0f));
	  
	  circleShape.setRadius(entityDimensions.x * tileSizeInPixels * 0.5f);
	  circleShape.setScale(1.0f,
			       (entityDimensions.y * tileSizeInPixels) / (entityDimensions.x * tileSizeInPixels));
	  circleShape.setFillColor(entityColor);
	  circleShape.setOutlineColor(sf::Color::Black);
	  circleShape.setPosition(entityPositionOnScreen.x, entityPositionOnScreen.y);

	  window->draw(circleShape);
	} break;
      }
    } break;
  case ER_MOB:
    {
      const MobRenderData& mobRenderData = *((MobRenderData*)entityRenderData);
      
      sf::RectangleShape rectangleShape;
      rectangleShape.setOutlineThickness(0);
      
      // Drawing Main Sprite
      
      // this should be read from the sprite object
      sf::Vector2f entityDimensions;
      
      sf::Sprite mobSprite = spriteManager->getSprite(mobRenderData.spriteName);
      
      // CONSTANT ALERT !!!
      
      float finalScale = tileSizeInPixels / 16.0f;
      mobSprite.setScale(finalScale, finalScale);
      mobSprite.setPosition(entityPositionOnScreen.x, entityPositionOnScreen.y);
      
      window->draw(mobSprite);
      
      entityDimensions = sf::Vector2f(mobSprite.getTextureRect().width / 16.0f,
				      mobSprite.getTextureRect().height / 16.0f );
      
      // // Temporarily
      // if(mobRenderData.spriteName == "Player") entityDimensions = sf::Vector2f(0.8f, 2.0f);
      // else entityDimensions = sf::Vector2f(1.0f, 1.0f);
	
      sf::Vector2f entityDimensionsInPixels = entityDimensions * tileSizeInPixels;
      
      if(entityPositionOnScreen.x + entityDimensionsInPixels.x < 0 ||
	 entityPositionOnScreen.y + entityDimensionsInPixels.y < 0 ||
	 entityPositionOnScreen.x >= windowDimensions.x ||
	 entityPositionOnScreen.y >= windowDimensions.y) break;
      
      rectangleShape.setSize(entityDimensionsInPixels);
      
      rectangleShape.setFillColor(sf::Color::Blue);
      rectangleShape.setPosition(entityPositionOnScreen.x, entityPositionOnScreen.y);
      
      //window->draw(rectangleShape);
      
      // Drawing Life Bar
      
      float lifeBarHeight = 0.2f;
      float distanceFromLifeBar = 0.1f;
      
      sf::Vector2f lifeBarSize(entityDimensions.x * tileSizeInPixels,
			       lifeBarHeight * tileSizeInPixels);
      
      sf::Vector2f lifeBarPosition(entityPositionOnScreen.x,
				   entityPositionOnScreen.y - lifeBarSize.y -
				   (distanceFromLifeBar * tileSizeInPixels));
      // Drawing Black Background First
      
      rectangleShape.setSize(lifeBarSize);
      rectangleShape.setOutlineThickness(1.0f * (tileSizeInPixels/40.0f));
      rectangleShape.setOutlineColor(sf::Color::Black);
      rectangleShape.setPosition(lifeBarPosition);
      
      rectangleShape.setFillColor(sf::Color::Black);
      
      window->draw(rectangleShape);
      
      // Drawing Actual Life - Colored
      
      lifeBarSize.x *= mobRenderData.life;
      
      rectangleShape.setSize(lifeBarSize);
      rectangleShape.setFillColor(sf::Color((1.0f - mobRenderData.life) * 255, mobRenderData.life * 255, 0));
      
      window->draw(rectangleShape);
      
      // Drawing Entity Text

      float textHeightInTiles = 0.5f;
      float textDistanceFromLifeBar = 0.1f;
	
      sf::Text entityText;
      entityText.setFont(font);
      entityText.setString(mobRenderData.caption);
      entityText.setCharacterSize(textHeightInTiles * tileSizeInPixels);

      sf::FloatRect localTextBounds = entityText.getLocalBounds();
      float widthLeft = entityDimensionsInPixels.x - localTextBounds.width;
      
      entityText.setPosition(lifeBarPosition + sf::Vector2f(widthLeft/ 2.0f,
							    -(textDistanceFromLifeBar + textHeightInTiles) *
							    tileSizeInPixels));
      entityText.setColor(sf::Color::Black);
      
      window->draw(entityText);
      
    } break;
  case ER_OVERLAYTEXT:
    {
      const OverlayTextRenderData& overlayTextRenderData = *((OverlayTextRenderData*)entityRenderData);

      sf::RectangleShape rectangleShape;
      rectangleShape.setOutlineThickness(0);
      
      sf::Vector2f entityDimensions;
      
      entityDimensions = sf::Vector2f(2.0f, 1.0f);
      
      sf::Vector2f entityDimensionsInPixels = entityDimensions * tileSizeInPixels;
      rectangleShape.setSize(entityDimensionsInPixels);
      
      rectangleShape.setFillColor(sf::Color::Blue);
      rectangleShape.setPosition(entityPositionOnScreen.x, entityPositionOnScreen.y);
      
      //window->draw(rectangleShape);
      
      sf::Text entityText;
      entityText.setFont(font);
      entityText.setString(overlayTextRenderData.text);
      float textHeightInTiles = overlayTextRenderData.fontSize;
      entityText.setCharacterSize(textHeightInTiles * tileSizeInPixels);

      sf::Color textColor = sf::Color(overlayTextRenderData.textColor.x,
				      overlayTextRenderData.textColor.y,
				      overlayTextRenderData.textColor.z,
				      255 - overlayTextRenderData.textFadeValue);
      
      sf::FloatRect localTextBounds = entityText.getLocalBounds();
      entityText.setPosition(entityPositionOnScreen.x, entityPositionOnScreen.y);
      entityText.setColor(textColor);
      
      window->draw(entityText);

      
    } break;
  case ER_BASICSPRITE:
    {
      const BasicSpriteRenderData& basicSpriteRenderData = *((BasicSpriteRenderData*)entityRenderData);
      
      sf::Sprite mobSprite = spriteManager->getSprite(basicSpriteRenderData.basicSpriteName);
      
      // CONSTANT ALERT !!!
      float finalScale = tileSizeInPixels / 16.0f;
      mobSprite.setScale(finalScale, finalScale);
      mobSprite.setPosition(entityPositionOnScreen.x, entityPositionOnScreen.y);
      
      window->draw(mobSprite);
      
    } break;
  }
  
}

void
LevelRenderer::renderEntities(const EntityList& entityList, EntityPosition& cameraPosition,
			      const Vector2i& tileChunkSize)
{
  for(auto entityIt = entityList.begin(); entityIt != entityList.end(); entityIt++)
  {
    Vector2f entityPositionOnScreen = getEntityPositionOnScreen(*entityIt, cameraPosition, tileChunkSize);
    const EntityRenderData* entityRenderData = (*entityIt)->getRenderData();
    renderEntity(entityRenderData, entityPositionOnScreen);
  }
}

void
LevelRenderer::renderSprites(const SpriteList& spriteList)
{
  
  for(auto i = spriteList.begin(); i != spriteList.end(); i++)
  {
    window->draw(*i);
  }
  
}

