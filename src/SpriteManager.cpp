#include "SpriteManager.h"

#include <iostream>
#include <assert.h>

void
SpriteManager::loadTexture(const std::string& filename)
{
  sf::Texture texture; 
  bool result = texture.loadFromFile("../resources/gfx/" + filename);
  if(!result)
  {
    std::cout << "Couldn't locate file:" + filename + " \n\n";
    assert(0);
  }
  std::cout << "Loaded File: " << filename << std::endl;
  textureVector.push_back(texture);
}

void
SpriteManager::loadSprite(const std::string& spriteName,
			  const IntRect& spriteRect,
			  int textureIndex)
{
  assert(textureIndex == - 1 ||
	 textureIndex <= textureVector.size());
  
  sf::Sprite& sprite = spriteMap[spriteName];
  sprite.setTextureRect(sf::IntRect(spriteRect.left, spriteRect.top,
				    spriteRect.width, spriteRect.height));
  
  std::cout << "Loaded Sprite: " << spriteName << std::endl;
  
  if(textureIndex != -1)
  {
    sprite.setTexture(textureVector[textureIndex]);
  }
  else
  {
    sprite.setTexture(textureVector.back());
  }
}

void
SpriteManager::loadSpriteSet(const std::string& spriteName,
			     const IntRect& spriteRect,
			     int numbOfSprites,
			     int startValue,
			     int textureIndex)
{
  IntRect currentSpriteRect = spriteRect;
  std::string currentSpriteName;
  
  for(int i = 0; i < numbOfSprites; i++)
  {
    currentSpriteName = spriteName;
    int currentSpriteIndex = (i+1+startValue);
      
    if(currentSpriteIndex > 9) currentSpriteName += char((currentSpriteIndex/10) + '0');
    currentSpriteName += char(((currentSpriteIndex)%10) + '0');
    
    loadSprite(currentSpriteName, currentSpriteRect, textureIndex);
    currentSpriteRect.left += currentSpriteRect.width;
  }
}

const sf::Sprite&
SpriteManager::getSprite(std::string spriteName) const
{
  if(spriteMap.count(spriteName))
  {
    return spriteMap.at(spriteName);
  }

  std::cout << "Couldn't find sprite: " << spriteName << std::endl;
  assert(0);
  return spriteMap.begin()->second;
}
