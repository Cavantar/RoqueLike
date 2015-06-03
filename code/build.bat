@echo off

IF "%1" == "ninja" GOTO NINJA

if not exist ..\build mkdir ..\build
pushd ..\build

set UnityBuild=False

set LibsDirectory=E:\Libs
set IncludeDirectory=%LibsDirectory%\SFML-2.2-64\include
set LibraryDirectory=%LibsDirectory%\SFML-2.2-64\lib

set Libraries= ^
    sfml-graphics-s.lib ^
    sfml-window-s.lib ^
    sfml-system-s.lib ^
    opengl32.lib ^
    winmm.lib ^
    gdi32.lib ^
    freetype.lib ^
    glew.lib ^
    jpeg.lib ^
    user32.lib ^
    Advapi32.lib 

set Defines=/DSFML_STATIC

if "%UnityBuild%" == "True" (
echo Unity Build

set FilesToCompile=..\code\main.cpp
set Defines=%Defines% /DUNITY_BUILD
)

if "%UnityBuild%" == "False" (
echo NonUnity Build
set FilesToCompile= ^
    ..\code\EntityPosition.cpp ^
    ..\code\Event.cpp ^
    ..\code\EventManager.cpp ^
    ..\code\PlayerHud.cpp ^
    ..\code\Game.cpp ^
    ..\code\TileMap.cpp ^
    ..\code\LevelGenerator.cpp ^
    ..\code\Level.cpp ^
    ..\code\Input.cpp ^
    ..\code\Entity.cpp ^
    ..\code\LevelRenderer.cpp ^
    ..\code\SpriteManager.cpp ^
    ..\code\Profiler.cpp ^
    ..\code\Noise.cpp ^
    ..\code\MiscFunctions.cpp ^
    ..\code\Mobs.cpp ^
    ..\code\main.cpp
)

REM Zi(Generate Debug information), FC(Full Path To Source), O2(Fast Code)

set CompilerOptions=%Defines% /FC /Zi /EHsc /MD /MP /wd4503 /nologo /FeRoqueLike.exe /I%IncludeDirectory% 
set LinkerOptions=/link /LIBPATH:%LibraryDirectory%

REM /SUBSYSTEM:windows 
cl %CompilerOptions% %FilesToCompile% %Libraries% %LinkerOptions%

REM cd ../code
REM start "" nmake

popd

GOTO END 

:NINJA

echo NinjaBuild
ninja

:END

