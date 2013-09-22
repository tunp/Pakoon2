//
// All textures for Pakoon! v1.0
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#pragma once

#include "stdafx.h"
#include "BTextures.h"
#include "OpenGLHelpers.h"
#include "BTerrain.h"
#include "BGame.h"
#include "time.h"

int      BTextures::m_nTextures = 0;
BTexture BTextures::m_textures[100]; // We allow at most 100 textures


//*****************************************************************************
void BTextures::Init() {

  // Load all internal textures from targa files
  LoadAllInternalTextures();
}



//*****************************************************************************
void BTextures::Exit() {
  for(int i = 0; i < m_nTextures; ++i) {
    if(m_textures[i].m_bValid) {
      OpenGLHelpers::FreeTexName(i, &(m_textures[i].m_nGLTexName));
      if(m_textures[i].m_pPixels) {
        delete [] m_textures[i].m_pPixels;
        m_textures[i].m_pPixels = 0;
      }
      m_textures[i].m_bValid = false;
    }
  }
}



//*****************************************************************************
void BTextures::LoadAllInternalTextures() {

  m_nTextures = 0;
  LoadTextureFromTGA(".\\Textures\\MainGameMenu.tga", MAIN_GAME_MENU, false);
  LoadTextureFromTGA(".\\Textures\\Wheeldetailed.tga", WHEEL, true);
  LoadTextureFromTGA(".\\Textures\\NavSatWndBigLetters.tga", PANEL, false);
  LoadTextureFromTGA(".\\Textures\\ShadowETC.tga", SHADOW, true);
  LoadTextureFromTGA(".\\Textures\\DustCloud.tga", DUSTCLOUD, true);
  LoadTextureFromTGA(".\\Textures\\EnvMap.tga", ENVMAP, true);
  LoadTextureFromTGA(".\\Textures\\EnvMapShiny.tga", ENVMAP_SHINY, true);
  LoadTextureFromTGA(".\\Textures\\QuickHelp.tga", QUICK_HELP, false);
  CString sTitle;
  srand((unsigned)time(NULL));
  sTitle.Format(".\\Textures\\MenuTitle_%d.tga", rand() % 6);
  LoadTextureFromTGA(sTitle, MENU_TITLES, true);
  LoadTextureFromTGA(".\\Textures\\OnScreenGameTexts.tga", ONSCREEN_GAME_TEXTS, false);
  LoadTextureFromTGA(".\\Textures\\ExtraScreenMessages.tga", EXTRA_SCREEN_MESSAGES, false);
  LoadTextureFromTGA(".\\Textures\\Earth.tga", EARTH, true);
  LoadTextureFromTGA(".\\Textures\\EarthSpecularMap.tga", EARTH_SPECULAR_MAP, true);
  LoadTextureFromTGA(".\\Textures\\Letters.tga", LETTERS, true);
  LoadTextureFromTGA(".\\Textures\\Pakoon2Logo.tga", LOGO, false);
  LoadTextureFromTGA(".\\Textures\\OldTube.tga", OLDTUBE, false);
  LoadTextureFromTGA(".\\Textures\\MOSLogo.tga", MOS_LOGO, false);
  LoadTextureFromTGA(".\\Textures\\FTCLogo_Dim.tga", FTC_LOGO, false);
  LoadTextureFromTGA(".\\Textures\\FTCLogo_Light.tga", FTC_LOGO2, false);
  m_nTextures = USER_BASE;
}


//*****************************************************************************
int BTextures::LoadTexture(CString sTextureName, bool bMipmapped) {
  // Check if caller wants an internal texture.
  // If not, see if the requested texture is already loaded.
  // If not, load and add a new tecture.
  // In all cases, return an index to the texture. 
  // This index can be used in the Use() function to activate that texture.

  if(sTextureName.CompareNoCase("INT_TXTR_NONE") == 0) {
    return NONE;
  } else if(sTextureName.CompareNoCase("INT_TXTR_CHROME") == 0) {
    return ENVMAP;
  } else if(sTextureName.CompareNoCase("INT_TXTR_BRICKS") == 0) {
    return BRICKS;
  } else if(sTextureName.CompareNoCase("INT_TXTR_ROCKS") == 0) {
    return ROCKS;
  } else if(sTextureName.CompareNoCase("INT_TXTR_GROUND") == 0) {
    return GROUND_BASE;
  } else if(sTextureName.CompareNoCase("INT_TXTR_SKY") == 0) {
    return SKY;
  } else {
    // Try to see if texture is already loaded
    for(int i = 0; i < m_nTextures; ++i) {
      if(m_textures[i].m_bValid && (sTextureName.CompareNoCase(m_textures[i].m_sFilename) == 0)) {
        return i;
      }
    }
    // Load new texture
    LoadTextureFromTGA(sTextureName, m_nTextures, bMipmapped);
    if(m_textures[m_nTextures].m_bValid) {
      ++m_nTextures;
      return m_nTextures - 1;
    } else {
      return NONE;
    }
  }

  return 0;
}


//*****************************************************************************
int BTextures::ReloadTexture(int nTextureIndex, CString sTextureName) {
  if(m_textures[nTextureIndex].m_bValid) {
    // Delete existing texture
    OpenGLHelpers::FreeTexName(nTextureIndex, &(m_textures[nTextureIndex].m_nGLTexName));
    m_textures[nTextureIndex].m_bValid = false;
  }
  LoadTextureFromTGA(sTextureName, nTextureIndex, true);
  return nTextureIndex;
}



//*****************************************************************************
bool BTextures::LoadTextureFromTGA(const CString sFilename, int nTextureIndex, bool bMipmapped) {

  BTexture *pTexture = &(m_textures[nTextureIndex]);

  HANDLE  hFile;
  DWORD   pdwRead;

  hFile = CreateFile(LPCTSTR(sFilename),
                     GENERIC_READ,
                     FILE_SHARE_READ,
                     NULL,
                     OPEN_EXISTING,
                     FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                     NULL);
  if(hFile != INVALID_HANDLE_VALUE) {
    unsigned char ucHeader[18];
    unsigned char *pucLine = 0;
    int nXRes, nYRes, x, y, nBits, nComponents;
    // Just assume file is correct original Targa file, no checking is done.
    // Read in the resolution from the header
    ReadFile(hFile, ucHeader, 18, &pdwRead, NULL);

    nXRes = (int) ucHeader[12] + (int) ucHeader[13] * 256;
    nYRes = (int) ucHeader[14] + (int) ucHeader[15] * 256;
    nBits = (int) ucHeader[16];
    if(nBits == 24) {
      nComponents = 3;
    } else if(nBits == 32) {
      nComponents = 4;
    } else {
      // Assume 3 components
      nComponents = 1;
    }
    bool bFlipped = false;
    if(ucHeader[17] & 16) {
      bFlipped = true;
    }

    // Sanity checks
    if(nXRes < 1)
      nXRes = 1;
    if(nYRes < 1)
      nYRes = 1;
    if(nXRes > 2048)
      nXRes = 2048;
    if(nYRes > 2048)
      nYRes = 2048;

    // Setup texture
    pTexture->m_sFilename = sFilename;
    pTexture->m_bMipmapped = bMipmapped;
    pTexture->m_bValid = true;
    pTexture->m_nXSize = nXRes;
    pTexture->m_nYSize = nYRes;
    pTexture->m_nComponents = nComponents;
    OpenGLHelpers::CreateTexName(nTextureIndex, &(pTexture->m_nGLTexName));

    // Reserve space for the pixels
    if(pTexture->m_pPixels) {
      delete [] pTexture->m_pPixels;
    }
    pTexture->m_pPixels = new GLubyte[pTexture->m_nXSize * pTexture->m_nYSize * pTexture->m_nComponents];

    // Read in pixels
    pucLine = new unsigned char[nXRes * nComponents];
    int nYStart, nYEnd, nYStep;
    if(bFlipped) {
      nYStart = nYRes - 1;
      nYEnd = -1;
      nYStep = -1;
    } else {
      nYStart = 0;
      nYEnd = nYRes;
      nYStep = 1;
    }
    for(y = nYStart; y != nYEnd; y += nYStep) {
      if(!ReadFile(hFile, pucLine, nXRes * nComponents, &pdwRead, NULL)) {
        BGame::MyAfxMessageBox("Error encountered while reading file.");
        CloseHandle(hFile);
        delete pucLine;
        return false;
      }
      if(nComponents == 3) {
        for(x = 0; x < nXRes; ++x) {
          pTexture->m_pPixels[y * nXRes * 3 + x * 3 + 0] = (BYTE) pucLine[x * 3 + 2];
          pTexture->m_pPixels[y * nXRes * 3 + x * 3 + 1] = (BYTE) pucLine[x * 3 + 1];
          pTexture->m_pPixels[y * nXRes * 3 + x * 3 + 2] = (BYTE) pucLine[x * 3 + 0];
        }
      } else if(nComponents == 4) {
        for(x = 0; x < nXRes; ++x) {
          pTexture->m_pPixels[y * nXRes * 4 + x * 4 + 0] = (BYTE) pucLine[x * 4 + 2];
          pTexture->m_pPixels[y * nXRes * 4 + x * 4 + 1] = (BYTE) pucLine[x * 4 + 1];
          pTexture->m_pPixels[y * nXRes * 4 + x * 4 + 2] = (BYTE) pucLine[x * 4 + 0];
          pTexture->m_pPixels[y * nXRes * 4 + x * 4 + 3] = (BYTE) pucLine[x * 4 + 3];
        }
      } else {
        for(x = 0; x < nXRes; ++x) {
          pTexture->m_pPixels[y * nXRes + x] = (BYTE) pucLine[x];
        }
      }
    }
    delete pucLine;
    CloseHandle(hFile);

    return true;
  } else {
    CString sTmp;
    sTmp.Format("Cannot open %s!", LPCTSTR(sFilename));
    BGame::MyAfxMessageBox(sTmp);
    return false;
  }
} // LoadTextureFromTGA




//*****************************************************************************
void BTextures::GetRGBFromTexture(int nTextureIndex, 
                                  int nX,
                                  int nY,
                                  int &rnRed, 
                                  int &rnGreen, 
                                  int &rnBlue, 
                                  int &rnAlpha) {
  if((nX < 0) || 
     (nX > m_textures[nTextureIndex].m_nXSize - 1) ||
     (nY < 0) || 
     (nY > m_textures[nTextureIndex].m_nYSize - 1)) {
    rnRed = rnGreen = rnBlue = rnAlpha = 0;
    return;
  }
  if(m_textures[nTextureIndex].m_nComponents == 3) {
    rnRed   = m_textures[nTextureIndex].m_pPixels[(nY * m_textures[nTextureIndex].m_nXSize + nX) * 3];
    rnGreen = m_textures[nTextureIndex].m_pPixels[(nY * m_textures[nTextureIndex].m_nXSize + nX) * 3 + 1];
    rnBlue  = m_textures[nTextureIndex].m_pPixels[(nY * m_textures[nTextureIndex].m_nXSize + nX) * 3 + 2];
  } else if(m_textures[nTextureIndex].m_nComponents != 1) {
    rnRed   = m_textures[nTextureIndex].m_pPixels[(nY * m_textures[nTextureIndex].m_nXSize + nX) * 4];
    rnGreen = m_textures[nTextureIndex].m_pPixels[(nY * m_textures[nTextureIndex].m_nXSize + nX) * 4 + 1];
    rnBlue  = m_textures[nTextureIndex].m_pPixels[(nY * m_textures[nTextureIndex].m_nXSize + nX) * 4 + 2];
    rnAlpha = m_textures[nTextureIndex].m_pPixels[(nY * m_textures[nTextureIndex].m_nXSize + nX) * 4 + 3];
  }
}




//*****************************************************************************
void BTextures::Use(int nTextureIndex) {

  GLenum format = GL_RGB;
  if(m_textures[nTextureIndex].m_nComponents == 1) {
    format = GL_LUMINANCE;
  } else if(m_textures[nTextureIndex].m_nComponents == 3) {
    format = GL_RGB;
  } else {
    format = GL_RGBA;
  }

  if(m_textures[nTextureIndex].m_bMipmapped) {
    OpenGLHelpers::BindMipMapTexture(m_textures[nTextureIndex].m_nXSize, 
                                     m_textures[nTextureIndex].m_nYSize, 
                                     m_textures[nTextureIndex].m_nComponents, 
                                     format, 
                                     m_textures[nTextureIndex].m_pPixels, 
                                     nTextureIndex, 
                                     m_textures[nTextureIndex].m_nGLTexName);
  } else {
    OpenGLHelpers::BindTexture(m_textures[nTextureIndex].m_nXSize, 
                               m_textures[nTextureIndex].m_nYSize, 
                               m_textures[nTextureIndex].m_nComponents, 
                               format, 
                               m_textures[nTextureIndex].m_pPixels, 
                               nTextureIndex, 
                               m_textures[nTextureIndex].m_nGLTexName);
  }
}


//*****************************************************************************
void BTextures::StopUsingTexture(int nTextureIndex) {
  if(m_textures[nTextureIndex].m_bValid) {
  }
}



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