#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <vector>

#include <jpb\Vector.h>
#include <jpb\Rect.h>

typedef std::unordered_map<std::string, sf::Sprite> SpriteMap;

class SpriteManager {
public:
  void loadTexture(const std::string& filename);
  
  // Texture has to be loaded before loading sprite
  void loadSprite(const std::string& spriteName,
		  const IntRect& spriteRect,
		  int textureIndex = -1);
  
  void loadSpriteSet(const std::string& spriteName,
		     const IntRect& spriteRect,
		     int numbOfSprites,
		     int startValue = 0,
		     int textureIndex = -1);
  
  const sf::Sprite& getSprite(std::string spriteName) const;
private:
  std::vector<sf::Texture> textureVector;
  SpriteMap spriteMap;
};
