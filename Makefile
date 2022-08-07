CC = g++
CFLAGS = -c
LDFLAGS = -lSDL2 -lSDL2_net -lGL -lGLU -lvorbisfile

EMXX = em++
EMXXFLAGS = -Oz -c -s USE_SDL=2 -s USE_VORBIS=1 -I/home/jettis/git/gl4es/include
EMXXLINK = -s USE_SDL=2 -s USE_VORBIS=1 -s FULL_ES2=1 -L/home/jettis/git/gl4es/lib -lGL4es -L/home/jettis/git/GLU/.libs -lGLU4es -s ALLOW_MEMORY_GROWTH=1

OBJECTS_IN_SOURCE_FOLDER = main Pakoon1View BGame BMessages BPlayer SoundModule Settings BTerrain OpenGLHelpers BTextures BTextRenderer BSimulation BUI BScene BVehicle BObject BSceneEditor BNavSatWnd BServiceWnd BCmdModule ControllerModule FileIOHelpers BMenu StringTools BCamera BaseClasses HeightMap PerlinNoise
OBJECTS = $(addprefix SourceCode/,$(OBJECTS_IN_SOURCE_FOLDER)) PakoonPhysicsEngine/PakoonPhysicsEngine
ASSETS = vehicles scenes Textures/MessageLetters.txt Textures/MainGameMenu.tga Textures/wheeldetailed.tga Textures/NavSatWndBigLetters.tga Textures/shadowetc.tga Textures/DustCloud.tga Textures/EnvMap.tga Textures/EnvMapShiny.tga Textures/QuickHelp.tga Textures/MenuTitle_5.tga Textures/OnScreenGameTexts.tga Textures/ExtraScreenMessages.tga Textures/Earth.tga Textures/EarthSpecularMap.tga Textures/Letters.tga Textures/Pakoon2Logo.tga Textures/OldTube.tga Textures/MOSLogo.tga Textures/FTCLogo_Dim.tga Textures/FTCLogo_Light.tga Textures/MenuTitle_0.tga Sounds/AtlantisOwesMeMoney.ogg Checy57.vehicle a_Mutalahti.scene b_Oskaloosa.scene c_Baixo-Longa.scene d_SabrinaCoast.scene e_BadKissingen.scene f_Kikiakki.scene g_Japura.scene Textures/CreditsProgramming.tga Textures/CreditsMusic.tga Textures/CreditsBetaTesting.tga Textures/CreditsSpecialThanks1.tga Textures/CreditsEnd.tga Sounds/p!2_-_01_-_menumusic.ogg Sounds/menu-scroll.ogg Sounds/menu-change_setting.ogg Textures/MenuTitle_4.tga Textures/Snowy1.tga Textures/IceWorld_EnvMap.tga Textures/BasicShinyFinish.tga Models/Chevy57_RedPaint.obj Models/Chevy57_Chrome.obj Models/Chevy57_Window.obj Models/Chevy57_Black.obj Models/Chevy57_White.obj Sounds/p!2_-_02_-_ingame01.ogg Textures/MenuTitle_3.tga Sounds/menu-back.ogg Textures/MenuTitle_1.tga Textures/Chevy57.tga Textures/IceWorld.tga Textures/MenuTitle_2.tga Sounds/CRASH.ogg Sounds/GoalFanfar.ogg Textures/Grass256.tga Sounds/player-disqualify.ogg Sounds/player-slalom_portsuccess.ogg
EXECUTABLE = Pakoon

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS:%=%.o)
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) $< -o $@

Pakoon.html: $(OBJECTS:%=%.em.o) $(ASSETS)
	$(EMXX) $(EMXXLINK) $(filter %.o,$^) $(ASSETS:%=--preload-file %) \
--preload-file 'Player/Bad Kissingen, GermanySlalom.dat' \
--preload-file 'Player/Baixo-Longa, AngolaSlalom.dat' \
--preload-file 'Player/Japura, BrazilSlalom.dat' \
--preload-file 'Player/Kikiakki, RussiaSlalom.dat' \
--preload-file 'Player/Mutalahti, FinlandSlalom.dat' \
--preload-file 'Player/Oskaloosa, Iowa USASlalom.dat' \
--preload-file 'Player/Sabrina Coast, AntarctisSlalom.dat' \
-o $@

%.em.o: %.cpp
	$(EMXX) $(EMXXFLAGS) $< -o $@

.PHONY: clean
clean:
	rm $(EXECUTABLE) $(OBJECTS:%=%.o)
