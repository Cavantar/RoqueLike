#include "Game.h"

#ifdef UNITY_BUILD

#include "Game.cpp"
#include "EntityPosition.cpp"
#include "TileMap.cpp"
#include "Entity.cpp"
#include "PlayerHud.cpp"
#include "Input.cpp"
#include "Event.cpp"
#include "EventManager.cpp"
#include "Level.cpp"
#include "LevelRenderer.cpp"
#include "LevelGenerator.cpp"
#include "SpriteManager.cpp"

#endif

#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>

#include <jpb\misc.h>

int main()
{
  redirectIOToConsole();

  Game game;
  game.start();

  return 0;
}
