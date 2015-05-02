#pragma once
#include <SFML/Graphics.hpp>

class Input{
 private:
  bool keysPressed[sf::Keyboard::KeyCount];
  bool keysReleased[sf::Keyboard::KeyCount];
  bool keysDown[sf::Keyboard::KeyCount];
  
 public:
  Input();
  
  void handleKeyPress(const sf::Keyboard::Key keyIdentifier);
  void handleKeyRelease(const sf::Keyboard::Key keyIdentifier);

  // TODO: Is This Safe ? 
  bool isKeyPressed(const sf::Keyboard::Key keyIdentifier) const {return keysPressed[keyIdentifier];}
  bool isKeyReleased(const sf::Keyboard::Key keyIdentifier) const {return keysReleased[keyIdentifier];}
  bool isKeyDown(const sf::Keyboard::Key keyIdentifier) const {return keysDown[keyIdentifier];}
  
  // Clears keysPressed and keysReleased Arrays so those events will be visible for 1 frame.
  // Should Be Called At The End Of The Frame
  
  void clearKeyStates();
};
