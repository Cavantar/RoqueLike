@echo off

IF "%1" == "ninja" GOTO NINJA

if not exist ..\build mkdir ..\build
pushd ..\build

set UnityBuild=False

set LibsDirectory=..\libs
set IncludeDirectory=%LibsDirectory%\SFML-2.4.1\include
set LibraryDirectory=%LibsDirectory%\SFML-2.4.1\lib

set Libraries= ^
    sfml-graphics-s.lib ^
    sfml-window-s.lib ^
    sfml-system-s.lib ^
    opengl32.lib ^
    winmm.lib ^
    gdi32.lib ^
    freetype.lib ^
    jpeg.lib ^
    user32.lib ^
    jpb.lib ^
    Advapi32.lib

set Defines=/DSFML_STATIC

if "%UnityBuild%" == "True" (
echo Unity Build

set FilesToCompile=..\src\main.cpp
set Defines=%Defines% /DUNITY_BUILD
)

if "%UnityBuild%" == "False" (
echo NonUnity Build
set FilesToCompile= ^
    ..\src\EntityPosition.cpp ^
    ..\src\Event.cpp ^
    ..\src\EventManager.cpp ^
    ..\src\PlayerHud.cpp ^
    ..\src\Game.cpp ^
    ..\src\TileMap.cpp ^
    ..\src\LevelGenerator.cpp ^
    ..\src\Level.cpp ^
    ..\src\Input.cpp ^
    ..\src\Entity.cpp ^
    ..\src\LevelRenderer.cpp ^
    ..\src\SpriteManager.cpp ^
    ..\src\MiscFunctions.cpp ^
    ..\src\Mobs.cpp ^
    ..\src\main.cpp
)

REM Zi(Generate Debug information), FC(Full Path To Source), O2(Fast Code)

set CompilerOptions=%Defines% /FC /Zi /EHsc /MD /MP /wd4503 /nologo /FeRoqueLike.exe /I%IncludeDirectory% /I..\libs\jpb
set LinkerOptions=/link /LIBPATH:%LibraryDirectory% /LIBPATH:..\libs\jpb\lib

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
