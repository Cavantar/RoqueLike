#pragma once

#include <SFML/Graphics.hpp>
#include "Entity.h"

class PlayerHud {
public:
  void setWindow(sf::RenderWindow* window) { this->window = window; }
  void setFont(sf::Font* font) { this->font = font; }
  
  void render(const Player* player);
  Vec2f renderText(const std::string& text, const Vec2f& textPos, const sf::Color& color,
		      bool nextLine = true);
  
  Vec2f renderBar(float fillPercentage, const sf::Color& baseColor, const Vec2f& position,
		     const Vec2f& dimensions, const std::string& text); 
		     
private:
  sf::RenderWindow* window;
  sf::Font* font;
};
