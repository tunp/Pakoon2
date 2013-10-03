CC = g++
CFLAGS = -c
LDFLAGS = -lSDL2 -lSDL2_net -lGL -lGLU -lvorbisfile

OBJECTS_IN_SOURCE_FOLDER = main.o Pakoon1View.o BGame.o BMessages.o BPlayer.o SoundModule.o Settings.o BTerrain.o OpenGLHelpers.o BTextures.o BTextRenderer.o BSimulation.o BUI.o BScene.o BVehicle.o BObject.o BSceneEditor.o BNavSatWnd.o BServiceWnd.o BCmdModule.o ControllerModule.o FileIOHelpers.o BMenu.o StringTools.o BCamera.o BaseClasses.o HeightMap.o PerlinNoise.o
OBJECTS = $(addprefix SourceCode/,$(OBJECTS_IN_SOURCE_FOLDER)) PakoonPhysicsEngine/PakoonPhysicsEngine.o
EXECUTABLE = Pakoon

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

$(OBJECTS): %.o: %.cpp
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm $(EXECUTABLE) $(OBJECTS)
