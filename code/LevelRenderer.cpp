#include "LevelRenderer.h"
#include <iostream>

LevelRenderer::LevelRenderer() : window(NULL), tileSizeInPixels(0)
{
  bool loadedFont = font.loadFromFile("chiller.ttf");
  assert(loadedFont);
}

void
LevelRenderer::renderLevel(const LevelPtr& level, EntityPosition& cameraPosition)
{
  assert(window);

  const TileMapPtr& tileMap = level->getTileMap();
  
  renderTileMap(tileMap, cameraPosition);
  renderEntities(level->getEntityList(0), cameraPosition,
		 tileMap->getTileChunkSize());
  
  renderEntities(level->getEntityList(1), cameraPosition,
		 tileMap->getTileChunkSize());

}

void
LevelRenderer::renderTileChunk(const TileChunkPtr& tileChunk, const Vector2f& screenChunkPosition,
				    const float tileSize)
{
  sf::RectangleShape rectangleShape = sf::RectangleShape(sf::Vector2f(tileSize, tileSize));
  //rectangleShape.setOutlineThickness(1.0f * (tileSize / 20.0f));

  const TileChunkData& tileChunkData = tileChunk->getTileChunkData();
  const sf::Vector2u windowDimensions = window->getSize();
  
  static const float wallHeight = 1.0f;
  
  int tileChunkHeight = tileChunkData.size();
  int tileChunkWidth = tileChunkData[0].size();

  int minX = 0;
  int maxX = tileChunkWidth;
  
  int minY = 0;
  int maxY = tileChunkHeight;

  if(screenChunkPosition.x < 0) minX = (-screenChunkPosition.x / tileSize);
  if(screenChunkPosition.y < 0) minY = (-screenChunkPosition.y / tileSize);
  
  if(screenChunkPosition.x > windowDimensions.x) maxX -= (screenChunkPosition.x - windowDimensions.x) / tileSize;
  if(screenChunkPosition.y > windowDimensions.y) maxY -= (screenChunkPosition.y - windowDimensions.y) / tileSize;
  
  
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

      screenTilePosition = sf::Vector2f(screenChunkPosition.x + (x * tileSize),
					screenChunkPosition.y + (y * tileSize));
      
      rectangleShape.setPosition(screenTilePosition);
      
      switch(tileChunkData[y][x])
      {
      case TILE_TYPE_WALL:
	{
	  screenTilePosition.y -= (wallHeight - 1.0f) * tileSize;
	  rectangleShape.setPosition(screenTilePosition);
	  
	  rectangleShape.setSize(sf::Vector2f(tileSize, tileSize * wallHeight));
	  rectangleShape.setFillColor(sf::Color(150,150,150));
	} break;
      case TILE_TYPE_STONE_GROUND:
	{
	  rectangleShape.setSize(sf::Vector2f(tileSize, tileSize));
	  rectangleShape.setFillColor(sf::Color(128, 128, 128));
	} break;
      case TILE_TYPE_ICE_GROUND:
	{
	  rectangleShape.setSize(sf::Vector2f(tileSize, tileSize));
	  rectangleShape.setFillColor(sf::Color(165,242, 243));
	} break;
      case TILE_TYPE_SPEED_GROUND:
	{
	  rectangleShape.setSize(sf::Vector2f(tileSize, tileSize));
	  rectangleShape.setFillColor(sf::Color(250,128,114));
	} break;
      }
      
      window->draw(rectangleShape);
    }
  }
}

void
LevelRenderer::renderTileMap(const TileMapPtr& tileMap, EntityPosition& cameraPosition)
{
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
	renderTileChunk(tileChunkMap.at(tileChunkPosition), screenChunkPosition, tileSizeInPixels);
      }
    }
  }
  
}

void
LevelRenderer::render(const EntityRenderData& entityRenderData, Vector2f entityPositionOnScreen)
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
	  circleShape.setOutlineThickness(0);
	  
	  circleShape.setRadius(entityDimensions.x * tileSizeInPixels * 0.5f);
	  circleShape.setScale(1.0f,
			       (entityDimensions.y * tileSizeInPixels) / (entityDimensions.x * tileSizeInPixels));
	  circleShape.setFillColor(entityColor);
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

      // Temporarily
      if(entityRenderData.spriteName == "Player") entityDimensions = sf::Vector2f(0.8f, 2.0f);
      else entityDimensions = sf::Vector2f(1.0f, 1.0f);
      
      sf::Vector2f entityDimensionsInPixels = entityDimensions * tileSizeInPixels;
      
      if(entityPositionOnScreen.x + entityDimensionsInPixels.x < 0 ||
	 entityPositionOnScreen.y + entityDimensionsInPixels.y < 0 ||
	 entityPositionOnScreen.x >= windowDimensions.x ||
	 entityPositionOnScreen.y >= windowDimensions.y) break;
      
      rectangleShape.setSize(entityDimensionsInPixels);
      
      rectangleShape.setFillColor(sf::Color::Blue);
      rectangleShape.setPosition(entityPositionOnScreen.x, entityPositionOnScreen.y);
      
      window->draw(rectangleShape);
      
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
LevelRenderer::renderEntity(const EntityPtr& entity, EntityPosition& cameraPosition,
				 const Vector2i& tileChunkSize)
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
  
  const EntityRenderData& entityRenderData = entity->getRenderData();
  
  render(entityRenderData, entityPositionOnScreen);
}

void
LevelRenderer::renderEntities(const EntityList& entityList, EntityPosition& cameraPosition,
				   const Vector2i& tileChunkSize)
{
  for(auto entityIt = entityList.begin(); entityIt != entityList.end(); entityIt++)
  {
    renderEntity(*entityIt, cameraPosition, tileChunkSize);
  }
}
