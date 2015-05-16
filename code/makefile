TARGET = "RoqueLike.exe"

SrcDir = ../code/
CC=cl

SOURCES = \
	main.cpp \
	Game.cpp \
	EntityPosition.cpp \
	Event.cpp \
	EventManager.cpp \
	PlayerHud.cpp \
	TileMap.cpp \
	LevelGenerator.cpp \
	Level.cpp \
	Input.cpp \
	Entity.cpp \
	LevelRenderer.cpp

OBJECTS=$(SOURCES:.cpp=.obj)
FULLPATHSOURCES = $(patsubst %, $(SrcDir)%, $(SOURCES))

LIBS = \
	sfml-graphics-s.lib \
	sfml-window-s.lib \
	sfml-system-s.lib \
	opengl32.lib \
	winmm.lib \
	gdi32.lib \
	freetype.lib \
	glew.lib \
	jpeg.lib \
	user32.lib \
	Advapi32.lib 

SfmlDirectory = D:/Libs/SFML-2.2-64
IncludeDirectory = $(SfmlDirectory)/include
LibraryDirectory = $(SfmlDirectory)/lib

Defines = -DSFML_STATIC

CompilerOptions=$(Defines) -FC -Zi -EHsc -MD -MP -wd4503 -Fe$(TARGET) -I$(IncludeDirectory) -c
LinkerOptions=-LIBPATH:$(LibraryDirectory)

all: $(TARGET)
$(TARGET): $(OBJECTS)
	@echo && pushd ../build && link $(LinkerOptions) $(LIBS) $(OBJECTS)

%.obj: %.cpp
	@echo && pushd ../build && $(CC) $(CompilerOptions) %.cpp

