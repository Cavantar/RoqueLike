#include "Input.h"

Input::Input()
{
  clearKeyStates();
  memset(keysDown, 0, sf::Keyboard::KeyCount);
}

void Input::handleKeyPress(sf::Keyboard::Key keyIdentifier)
{
  keysPressed[keyIdentifier] = true;
  keysDown[keyIdentifier] = true;
}

void Input::handleKeyRelease(sf::Keyboard::Key keyIdentifier)
{
  keysReleased[keyIdentifier] = true;
  keysDown[keyIdentifier] = false;
}

void Input::clearKeyStates()
{
  memset(keysPressed, 0, sf::Keyboard::KeyCount);
  memset(keysReleased, 0, sf::Keyboard::KeyCount);
}
