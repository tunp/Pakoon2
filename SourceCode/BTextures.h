//
// All textures for Pakoon! v1.0
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#pragma once

#ifdef _WIN32
#include <windows.h>
#endif

#include "BaseClasses.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include "Settings.h"

#include <string>

using namespace std;

class BTexture {
public:
  bool     m_bValid;
  string  m_sFilename;
  GLuint   m_nGLTexName;
  int      m_nXSize;
  int      m_nYSize;
  int      m_nComponents;
  bool     m_bMipmapped;
  GLubyte *m_pPixels;

  BTexture() {m_bValid = false; m_pPixels = 0;}
};

class BTextures {

  static bool LoadTextureFromTGA(const string sFilename, int nTextureIndex, bool bMipmapped);
  static void LoadAllInternalTextures();

public:
  static int      m_nTextures;
  static BTexture m_textures[100]; // We allow at most 100 textures
  enum Texture {NONE = 0,
                MAIN_GAME_MENU,
                SKY, 
                GROUND_COLOR_MAP, 
                GROUND_BASE, 
                CAR, 
                DUSTCLOUD, 
                SHADOW, 
                WHEEL, 
                DASHBOARD, 
                INTRO43AA, 
                COMPASS, 
                LED_NUMBERS,
                PANEL,
                SATELLITEIMAGE,
                ENVMAP,
                ENVMAP_SHINY,
                FLOWERS,   
                BRICKS,
                ROCKS,
                QUICK_HELP,
                QUICK_HELP_SCENE_EDITOR,
                MENU_TITLES,
                ONSCREEN_GAME_TEXTS,
                EXTRA_SCREEN_MESSAGES,
                EARTH,
                EARTH_SPECULAR_MAP,
                LETTERS,
                LOGO,
                OLDTUBE,
                MOS_LOGO,
                FTC_LOGO,
                FTC_LOGO2,
                USER_BASE}; // Always make sure USER_BASE is the last texture!
  static void Init();
  static void Exit();
  static void Use(int nTextureIndex);
  static int  LoadTexture(string sTextureName, bool bMipmapped = true);
  static int  ReloadTexture(int nTextureIndex, string sTextureName);
  static void StopUsingTexture(int nTextureIndex);

  static void GetRGBFromTexture(int nTextureIndex, 
                                int nX,
                                int nY,
                                int &rnRed, 
                                int &rnGreen, 
                                int &rnBlue, 
                                int &rnAlpha);
};



// PAKOON! Game, Source Code and Developer Package Copyright
// =========================================================
// 
// Restrictions related to PAKOON! Game and it's use
// -------------------------------------------------
// 
// You may download and play the PAKOON! game for free. You may also copy it freely to your friends and relatives as long as you 
// provide the original setup package (downloaded from www.nic.fi/~moxide) and the copyright included in it is also given. You 
// may also use the PAKOON! game for educational purposes, as long as the origin of the PAKOON! game (i.e. www.nic.fi/~moxide) 
// is mentioned and this copyright is also provided and the creator of the game (i.e. Mikko Oksalahti, email: 
// mikko.oksalahti@nic.fi) is notified of the use in advance.
// You may not sell or otherwise accept any payment for giving or offering the game to someone else. You may not offer the 
// PAKOON! game for free on any webpage, CD, DVD or other media without a written permission from the creator of the PAKOON! 
// game (i.e. Mikko Oksalahti, email: mikko.oksalahti@nic.fi).
// You may freely include a link to PAKOON! homepage (i.e. www.nic.fi/~moxide) from your own site.
// 
// 
// Restrictions related to PAKOON! Game
// Source Code and Developer Package and their use
// -----------------------------------------------
// 
// You may download and use the PAKOON! game source code for personal use. You may not use any part of the source code or the 
// developer package on any commercial or free game or other computer program intended for public distribution without a written 
// permission from the creator of the PAKOON! game (i.e. Mikko Oksalahti, email: mikko.oksalahti@nic.fi). You may use the 
// PAKOON! game source code or developer package for educational purposes, as long as the origin of the PAKOON! game (i.e. 
// www.nic.fi/~moxide) is mentioned and this copyright is also provided and the creator of the game (i.e. Mikko Oksalahti, 
// email: mikko.oksalahti@nic.fi) is notified of the use in advance.
// You may not sell or otherwise accept any payment for giving or offering the PAKOON! game source code or developer package to 
// someone else. You may not offer the PAKOON! game source code or developer package for free on any webpage, CD, DVD or other 
// media without a written permission from the creator of the PAKOON! game (i.e. Mikko Oksalahti, email: 
// mikko.oksalahti@nic.fi).
// 
// 
// Mikko Oksalahti
// Helsinki, FINLAND
// 7.10.2002
