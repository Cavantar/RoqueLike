#pragma once

#include <SFML/Graphics.hpp>
#include "Entity.h"

class PlayerHud {
public:
  void setWindow(sf::RenderWindow* window) { this->window = window; }
  void setFont(sf::Font* font) { this->font = font; }
  
  void render(const Player* player);
  Vector2f renderText(const std::string& text, const Vector2f& textPos, const sf::Color& color,
		      bool nextLine = true);
  
  Vector2f renderBar(float fillPercentage, const sf::Color& baseColor, const Vector2f& position,
		     const Vector2f& dimensions, const std::string& text); 
		     
private:
  sf::RenderWindow* window;
  sf::Font* font;
};
