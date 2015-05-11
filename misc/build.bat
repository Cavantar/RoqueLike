@echo off

mkdir ..\build
pushd ..\build

set UnityBuild=True

set IncludeDirectory=D:\Libs\SFML-2.2-64\include
set LibraryDirectory=D:\Libs\SFML-2.2-64\lib

set Libraries=sfml-graphics-s.lib sfml-window-s.lib sfml-system-s.lib opengl32.lib winmm.lib gdi32.lib freetype.lib glew.lib jpeg.lib user32.lib Advapi32.lib

set Defines=/DSFML_STATIC

if "%UnityBuild%" == "True" (
echo Unity Build

set FilesToCompile=..\code\main.cpp
set Defines=%Defines% /DUNITY_BUILD
)

if "%UnityBuild%" == "False" (
echo NonUnity Build
set FilesToCompile=..\code\EntityPosition.cpp ..\code\Event.cpp ..\code\EventManager.cpp ..\code\PlayerHud.cpp ..\code\Game.cpp ..\code\TileMap.cpp   ..\code\LevelGenerator.cpp ..\code\Level.cpp ..\code\Input.cpp ..\code\Entity.cpp ..\code\LevelRenderer.cpp ..\code\main.cpp 
)

REM Zi(Generate Debug information), FC(Full Path To Source), O2(Fast Code)

set CompilerOptions=%Defines% /FC /Zi /EHsc /MD /MP /wd4503 /FeRoqueLike.exe /I%IncludeDirectory% 
set LinkerOptions=/link /LIBPATH:%LibraryDirectory%

REM /SUBSYSTEM:windows 
cl %CompilerOptions% %FilesToCompile% %Libraries% %LinkerOptions%

popd
