#include "LevelRenderer.h"
#include <iostream>

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
  
  EntityListForRendering tilesForRendering = renderTileMap(tileMap, cameraPosition);


  //std::cout << tilesForRendering.size() << std::endl;
  
  // Combining both lists
  entitiesForRendering.splice(entitiesForRendering.end(), tilesForRendering);
  
  entitiesForRendering.sort(compareEntityRenderThing);
  
  renderSortedEntities(entitiesForRendering);
  entitiesForRendering.clear();
  
  // renderEntities(level->getEntityList(0), cameraPosition,
  // 		   tileMap->getTileChunkSize());
  
  renderEntities(level->getEntityList(1), cameraPosition,
		 tileMap->getTileChunkSize());
  
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
    const EntityRenderData& entityRenderData = (*entityIt)->getRenderData();
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
      
      switch(tileChunkData[y][x])
      {
      case TILE_TYPE_WALL:
	{
	  	  
	  int tileKind = ((x ^ y ^ (int)tileChunk.get()) % 30) ;
	  tileKind = abs(tileKind);
	  tileKind++;
	  
	  screenTilePosition.y -= (wallHeight - 1.0f) * tileSizeInPixels;
	  
	  sf::Sprite tileSprite;
	  float finalScale = tileSizeInPixels / 16.0f;
	  
	  WorldPosition tempWorldPosition(tileChunkPosition, Vector2i(x, y));
	  char surroundingTiles = level->getSurroundingTileData(tempWorldPosition, TILE_TYPE_WALL);

	  std::string spriteName = "wallTop1_";
	  if(tileKind > 9) spriteName += char(tileKind/10 + '0');
	  spriteName += char(tileKind%10 + '0');
	  tileSprite = spriteManager->getSprite(spriteName);
	  
	  tileSprite.setScale(finalScale, finalScale);
	  tileSprite.setPosition(screenTilePosition - sf::Vector2f(0, tileSizeInPixels));

	  SpriteList spriteList;
	  spriteList.push_back(tileSprite);
	  
	  //window->draw(tileSprite);
	  
	  if(!(surroundingTiles & ST_SOUTH))
	  {
	    std::string spriteName = "wall1_";
	    int tileKind = ((x ^ y ^ (int)tileChunk.get()) % 7);
	    tileKind = abs(tileKind);
	    tileKind++;
	    
	    spriteName += char((tileKind%10) + '0');
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
      case TILE_TYPE_STONE_GROUND:
	{
	  rectangleShape.setSize(sf::Vector2f(tileSizeInPixels, tileSizeInPixels));
	  rectangleShape.setFillColor(sf::Color(128, 128, 128));
	  
	  int tileKind = ((x ^ y ^ (int)tileChunk.get()) % 60) ;
	  tileKind = abs(tileKind);
	  
	  std::string spriteName = "floor1_";
	  if((tileKind+1) > 9) spriteName += char(((tileKind+1)/10) + '0');
	  spriteName += char(((tileKind+1)%10) + '0');
	  
	  sf::Sprite tileSprite = spriteManager->getSprite(spriteName);

	  float finalScale = tileSizeInPixels / 16.0f;
	  
	  tileSprite.setScale(finalScale, finalScale);
	  tileSprite.setPosition(screenTilePosition);
	  
	  window->draw(tileSprite);
	  
	} break;
      case TILE_TYPE_ICE_GROUND:
	{
	  rectangleShape.setSize(sf::Vector2f(tileSizeInPixels, tileSizeInPixels));
	  rectangleShape.setFillColor(sf::Color(165,242, 243));
	  
	  window->draw(rectangleShape);

	} break;
      case TILE_TYPE_SPEED_GROUND:
	{
	  rectangleShape.setSize(sf::Vector2f(tileSizeInPixels, tileSizeInPixels));
	  rectangleShape.setFillColor(sf::Color(250,128,114));
	  
	  window->draw(rectangleShape);
	  
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
  
  //sf::RenderWindow& window = *this->window;
  const sf::Vector2u windowDimensions = window->getSize();
  const TileChunkMap& tileChunkMap = tileMap->getTileChunkMap();
  
  // To Determine How many Chunks I have to render, I have to know it's width in pixels
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
LevelRenderer::renderEntity(const EntityRenderData& entityRenderData, Vector2f entityPositionOnScreen)
{
  const sf::Vector2u windowDimensions = window->getSize();
  
  switch(entityRenderData.type) {
  case ER_PRIMITIVE:
    {
      const Vector2f& entityDimensions = entityRenderData.dimensionsInTiles;
      const sf::Color entityColor(entityRenderData.color.x, entityRenderData.color.y, entityRenderData.color.z);
      
      switch(entityRenderData.primitiveType) {
      case PT_RECTANGLE:
	{
	  sf::RectangleShape rectangleShape;
	  rectangleShape.setOutlineThickness(0);
	  rectangleShape.setSize(sf::Vector2f(entityDimensions.x * tileSizeInPixels,
					      entityDimensions.y * tileSizeInPixels));
	  rectangleShape.setFillColor(entityColor);
	  rectangleShape.setPosition(entityPositionOnScreen.x, entityPositionOnScreen.y);
	  
	  window->draw(rectangleShape);
	} break;
      case PT_CIRCLE:
	{
	  sf::CircleShape circleShape;
	  circleShape.setOutlineThickness(1.0f * (tileSizeInPixels / 40.0f));
	  
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
      sf::RectangleShape rectangleShape;
      rectangleShape.setOutlineThickness(0);
      
      // Drawing Main Sprite
      
      // this should be read from the sprite object
      sf::Vector2f entityDimensions;
      
      sf::Sprite mobSprite = spriteManager->getSprite(entityRenderData.spriteName);
      
      // CONSTANT ALERT !!!
      
      float finalScale = tileSizeInPixels / 16.0f;
      mobSprite.setScale(finalScale, finalScale);
      mobSprite.setPosition(entityPositionOnScreen.x, entityPositionOnScreen.y);
      
      window->draw(mobSprite);
      
      entityDimensions = sf::Vector2f(mobSprite.getTextureRect().width / 16.0f,
				      mobSprite.getTextureRect().height / 16.0f );
      
      // // Temporarily
      // if(entityRenderData.spriteName == "Player") entityDimensions = sf::Vector2f(0.8f, 2.0f);
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
      
      lifeBarSize.x *= entityRenderData.life;
      
      rectangleShape.setSize(lifeBarSize);
      rectangleShape.setFillColor(sf::Color((1.0f - entityRenderData.life) * 255, entityRenderData.life * 255, 0));
      
      window->draw(rectangleShape);
      
      // Drawing Entity Text

      float textHeightInTiles = 0.5f;
      float textDistanceFromLifeBar = 0.1f;
	
      sf::Text entityText;
      entityText.setFont(font);
      entityText.setString(entityRenderData.caption);
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
      entityText.setString(entityRenderData.text);
      float textHeightInTiles = entityRenderData.fontSize;
      entityText.setCharacterSize(textHeightInTiles * tileSizeInPixels);

      sf::Color textColor = sf::Color(entityRenderData.textColor.x,
				      entityRenderData.textColor.y,
				      entityRenderData.textColor.z,
				      255 - entityRenderData.textFadeValue);
      
      sf::FloatRect localTextBounds = entityText.getLocalBounds();
      entityText.setPosition(entityPositionOnScreen.x, entityPositionOnScreen.y);
      entityText.setColor(textColor);
      
      window->draw(entityText);

      
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
    const EntityRenderData& entityRenderData = (*entityIt)->getRenderData();
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

