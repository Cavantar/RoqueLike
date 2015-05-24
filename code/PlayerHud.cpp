#include "PlayerHud.h"
#include "Mobs.h"
#include <sstream>
#include <iomanip>

void PlayerHud::render(const Player* player)
{
  const sf::Vector2u windowDimensions = window->getSize();

  Vector2f panelDimensions(windowDimensions.x * 0.20f, windowDimensions.y * 0.5f);
  Vector2f panelPosition(windowDimensions.x - panelDimensions.x - 2.0f, 2.0f);
  
  sf::RectangleShape rectangleShape(sf::Vector2f(panelDimensions.x, panelDimensions.y));
  rectangleShape.setPosition(panelPosition.x, panelPosition.y);
  rectangleShape.setOutlineThickness(2.0f);
  rectangleShape.setFillColor(sf::Color(255, 255, 255, 128));
  rectangleShape.setOutlineColor(sf::Color(0,0,0, 128));
  
  window->draw(rectangleShape);
  
  Vector2f textPos = panelPosition + Vector2f(20.0f, 20.0f);
  int skillPoints = player->getSkillPoints();

  std::stringstream tempText;
  tempText << std::fixed << std::setw(11) << std::setprecision(2);
  
  tempText << "Level " << player->getMobLevel();
  textPos = renderText(tempText.str(), textPos, sf::Color::Black);
  
  tempText.str(""); 
  tempText << "Shield: " << player->getShieldValue();
  if(skillPoints) tempText << " (1 - ^)";
  textPos = renderText(tempText.str(), textPos, sf::Color::Black);
  
  tempText.str(""); 
  tempText << "DamageValue: " << player->getDamageValue();
  if(skillPoints) tempText << " (2 - ^)";
  textPos = renderText(tempText.str(), textPos,  sf::Color::Black);
  
  tempText.str(""); 
  tempText << "MoveSpeed: " << player->getMovementSpeed();
  if(skillPoints) tempText << " (3 - ^)";
  textPos = renderText(tempText.str(), textPos, sf::Color::Black);
    
  tempText.str(""); 
  tempText << "BulletSpeed: " << player->getBulletVelocity();
  if(skillPoints) tempText << " (4 - ^)";
  textPos = renderText(tempText.str(), textPos, sf::Color::Black);
      
  tempText.str(""); 
  tempText << "Health: " << player->getHealth();
  if(skillPoints) tempText << " (5 - ^)";
  textPos = renderText(tempText.str(), textPos, sf::Color::Black);
        
  tempText.str(""); 
  tempText << "Stamina: " << player->getStamina();
  if(skillPoints) tempText << " (6 - ^)";
  textPos = renderText(tempText.str(), textPos, sf::Color::Black);
  
  if(skillPoints != 0)
  {
    tempText.str(""); 
    tempText << "SkillPoints: " << skillPoints;
    textPos = renderText(tempText.str(), textPos, sf::Color::Black);
  }
  
  Vector2f renderBarDimensions(windowDimensions.x * 0.3f, 25.0f);
  Vector2f renderBarPosition(windowDimensions.x - renderBarDimensions.x - 2.0f,
			     windowDimensions.y - (renderBarDimensions.y * 5));
  
  float fillPercentage = player->getHealth()/player->getMaxHealth();
  tempText.str("");
  tempText << player->getHealth() << " / " << player->getMaxHealth();
  renderBarPosition = renderBar(fillPercentage, sf::Color::Red, renderBarPosition,
				renderBarDimensions, tempText.str());

  fillPercentage = player->getStamina()/player->getMaxStamina();
  tempText.str("");
  tempText << player->getStamina() << " / " << player->getMaxStamina();
  renderBarPosition = renderBar(fillPercentage, sf::Color::Yellow, renderBarPosition,
				renderBarDimensions, tempText.str());
  
  fillPercentage = (player->getXp() - player->getCurrentLevelXp());
  fillPercentage /= (player->getNextLevelXp() - player->getCurrentLevelXp());
  
  tempText.str("");
  tempText << player->getXp() << " / " << player->getNextLevelXp();
  renderBarPosition = renderBar(fillPercentage, sf::Color::Green, renderBarPosition,
				renderBarDimensions, tempText.str());
  
}

Vector2f PlayerHud::renderText(const std::string& text, const Vector2f& textPos, const sf::Color& color,
			       bool nextLine)
{
  
  sf::Text tempText;
  tempText.setFont(*font);
  tempText.setString(text);
  tempText.setCharacterSize(24.0f);
  tempText.setPosition(textPos.x, textPos.y);
  tempText.setColor(color);
  
  window->draw(tempText);

  sf::FloatRect localTextBounds = tempText.getLocalBounds();
  return textPos + Vector2f(0, localTextBounds.height * 2);
}

Vector2f PlayerHud::renderBar(float fillPercentage, const sf::Color& baseColor, const Vector2f& position,
			      const Vector2f& dimensions, const std::string& text)
{
    
  sf::RectangleShape rectangleShape(sf::Vector2f(dimensions.x, dimensions.y));
  rectangleShape.setPosition(position.x, position.y);
  rectangleShape.setOutlineThickness(2.0f);
  rectangleShape.setFillColor(sf::Color(255, 255, 255, 128));
  rectangleShape.setOutlineColor(sf::Color(0, 0, 0));
  
  window->draw(rectangleShape);

  if(fillPercentage > 1.0f) fillPercentage = 1.0f;
  
  sf::Vector2f lifeBarDimensions((dimensions.x - 2)  * fillPercentage, dimensions.y - 2);
  rectangleShape.setSize(lifeBarDimensions);
  rectangleShape.setPosition(position.x + 1, position.y + 1);
  rectangleShape.setOutlineThickness(0);
  rectangleShape.setFillColor(baseColor);
  
  window->draw(rectangleShape);
  
  sf::Text tempText;
  tempText.setFont(*font);
  tempText.setString(text);
  tempText.setCharacterSize(20.0f);
  
  sf::FloatRect localTextBounds = tempText.getLocalBounds();
  Vector2f textPos = position + Vector2f(1, 1);
  textPos.x += ((dimensions.x - 2) / 2.0f) - (localTextBounds.width / 2.0f) ;
  
  tempText.setPosition(textPos.x, textPos.y);
  tempText.setColor(sf::Color::Black);
  
  window->draw(tempText);
  
  Vector2f nextPosition = position;
  nextPosition.y += dimensions.y * 1.5f ;
  
  return nextPosition;
}

