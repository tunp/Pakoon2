// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#include "BNavSatWnd.h"
#include "OpenGLHelpers.h"
#include "OpenGLExtFunctions.h"
#include "HeightMap.h"
#include "BTextures.h"
#include "BGame.h"



// Just use global variables for inter-thread communication. 
// C'mon man, this is a game, not a mission critical medical application.
SDL_mutex *g_csComm; // semafore to global data
BVector          g_vPos = BVector(0, 0, 0);
bool             g_bExit = false;
bool             g_bDoNewImage = true;
bool             g_bContinueImage = false;
bool             g_bFinal = false;  
bool             g_bNewImageAvailable = false;
GLubyte          g_ubMapImageTemp[256 * 256 * 3 + 1024];
GLubyte          g_ubMapImage[256 * 256 * 3 + 1024];
double           g_dHeightMap[256 * 256];
int              g_nImageSize = 1;

double           g_dXOffset = -99999.9;
double           g_dYOffset = -99999.9;
// double           g_dScale   = 1600.0 / 256.0;
double           g_dScale   = (2600.0 * 1.0) / 256.0;
int              g_nStartStepSize = 256;


// Thread prototype
int DrawLandscapeGIFStyle(void *data);



//*****************************************************************************
BNavSatWnd::BNavSatWnd() {
  m_bTracking = false;
  m_hThread = 0;
  g_csComm = SDL_CreateMutex();
}

//*****************************************************************************
BNavSatWnd::~BNavSatWnd() {
  SDL_DestroyMutex(g_csComm);
}

//*****************************************************************************
void BNavSatWnd::StartTracking(BVector vPos) {
  // First make sure the thread nows which area to calculate
  SDL_LockMutex(g_csComm);

  // Check if we need new image
  BVector vRelLoc(0, 0, 0);
  vRelLoc.m_dX = (vPos.m_dX - g_dXOffset) / g_dScale;
  vRelLoc.m_dY = (vPos.m_dY - g_dYOffset) / g_dScale;
  if(!(vRelLoc.m_dX > 0.0 && vRelLoc.m_dX < 255 && 
       vRelLoc.m_dY > 0.0 && vRelLoc.m_dY < 255)) {
    g_dXOffset = vPos.m_dX - g_dScale * 128.0;
    g_dYOffset = vPos.m_dY - g_dScale * 128.0;
    g_bNewImageAvailable = true;
    g_bDoNewImage = true;
    g_bContinueImage = false;
    g_nStartStepSize = 256;

    // Clear old images
    memset(g_ubMapImageTemp, 0, 256 * 256 * 3);
    memset(g_ubMapImage, 0, 256 * 256 * 3);
    memset(g_dHeightMap, 0, 256 * 256);

  } else {
    g_bContinueImage = true;
  }
  SDL_UnlockMutex(g_csComm);

  // Then start the worker thread
  if(!m_bTracking) {
    // Start a new thread to calculate the map texture
    g_bExit = false;    

	m_hThread = SDL_CreateThread(DrawLandscapeGIFStyle, "DrawLandscapeGIFStyle", (void *) this);
    if(m_hThread) {
      m_bTracking = true;
    }
  }
}


//*****************************************************************************
BVector BNavSatWnd::Track(BVector vPos) {
  BVector vRelLoc(0, 0, 0);
  SDL_LockMutex(g_csComm);
  g_vPos = vPos;
  vRelLoc.m_dX = (vPos.m_dX - g_dXOffset) / g_dScale;
  vRelLoc.m_dY = (vPos.m_dY - g_dYOffset) / g_dScale;

  if(!(vRelLoc.m_dX > 0.0 && vRelLoc.m_dX < 255 && 
       vRelLoc.m_dY > 0.0 && vRelLoc.m_dY < 255)) {
    // We need a new image
    g_dXOffset = vPos.m_dX - g_dScale * 128.0;
    g_dYOffset = vPos.m_dY - g_dScale * 128.0;
    g_bDoNewImage = true;
    g_bContinueImage = false;
    g_nStartStepSize = 256;

    // Clear old images
    memset(g_ubMapImageTemp, 0, 256 * 256 * 3);
    memset(g_ubMapImage, 0, 256 * 256 * 3);
    memset(g_dHeightMap, 0, 256 * 256);
  }

  SDL_UnlockMutex(g_csComm);
  return vRelLoc;
}

//*****************************************************************************
BVector BNavSatWnd::GetRelLoc(BVector vPos) {
  BVector vRelLoc(0, 0, 0);
  SDL_LockMutex(g_csComm);
  vRelLoc.m_dX = (vPos.m_dX - g_dXOffset) / g_dScale;
  vRelLoc.m_dY = (vPos.m_dY - g_dYOffset) / g_dScale;
  SDL_UnlockMutex(g_csComm);
  return vRelLoc;
}


//*****************************************************************************
void BNavSatWnd::EndTracking() {
  if(m_bTracking) {
    if(m_hThread) {
      SDL_LockMutex(g_csComm);
      g_bExit = true;
      SDL_UnlockMutex(g_csComm);
      // Give thread max 2 seconds to exit. If not, kill it
      //FIXME
      /*if(WaitForSingleObject(m_hThread, 2000) != WAIT_OBJECT_0) {
        // Thread did not respond, kill it (not recommended)
        TerminateThread(m_hThread, 0);
      }*/
    }
    m_bTracking = false;
  }
}


//*****************************************************************************
int BNavSatWnd::ActivateCurrentMapTexture() {
  static int nOldSize = -1;
  int nRet = 1;
  SDL_LockMutex(g_csComm);
  if(g_bNewImageAvailable) {
    // Create texture object
    OpenGLHelpers::BindTexture(g_nImageSize, 
                               g_nImageSize, 
                               3, 
                               GL_RGB, 
                               g_ubMapImage, 
                               BTextures::SATELLITEIMAGE, 
                               BTextures::m_textures[BTextures::SATELLITEIMAGE].m_nGLTexName,
                               true);
    nOldSize = g_nImageSize;
    g_bNewImageAvailable = false;
  } else {
    // just use texture
    OpenGLHelpers::BindTexture(g_nImageSize, 
                               g_nImageSize, 
                               3, 
                               GL_RGB, 
                               g_ubMapImage, 
                               BTextures::SATELLITEIMAGE, 
                               BTextures::m_textures[BTextures::SATELLITEIMAGE].m_nGLTexName,
                               false);
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  nRet = g_nImageSize;
  SDL_UnlockMutex(g_csComm);
  return nRet;
}




//*****************************************************************************
void BNavSatWnd::SetResolution(double dRes) {
  double dNewScale = dRes / 256.0;
  SDL_LockMutex(g_csComm);
  if(dNewScale != g_dScale) {
    // We need a new image
    g_dXOffset = (g_dXOffset + g_dScale * 128.0) - dNewScale * 128.0;
    g_dYOffset = (g_dYOffset + g_dScale * 128.0) - dNewScale * 128.0;

    g_bDoNewImage = true;
    g_bContinueImage = false;
    g_nStartStepSize = 256;

    g_dScale = dNewScale;

    // Clear old images
    memset(g_ubMapImageTemp, 0, 256 * 256 * 3);
    memset(g_ubMapImage, 0, 256 * 256 * 3);
    memset(g_dHeightMap, 0, 256 * 256);
  }
  SDL_UnlockMutex(g_csComm);
}








//*****************************************************************************
int DrawLandscapeGIFStyle(void *data) {
  BVector vInternalOffset = BGame::GetSimulation()->GetTerrain()->m_vOffset;

  int nSkipper = 0;
  double dArea;
  BVector vY(0, 1, 0);
  BVector vToLight(-1, 0, -1.0);
  vToLight.ToUnitLength();
  BVector vToSky(0, 0, -1.0);
  double dMaxHeight = 0.0, dR, dG, dB;
  int nCounter = 1;
  g_dHeightMap[0] = HeightMap::CalcHeightAt(vInternalOffset.m_dX + g_dXOffset, 
                                          vInternalOffset.m_dY + g_dYOffset, 
                                          dArea, 
                                          BGame::GetSimulation()->GetScene()->m_terrainStyle,
                                          8,
                                          vInternalOffset.m_dY);

  double dBase = 64.0;
  double dColor = 0.7 * (255.0 - 64.0);
  if(g_dHeightMap[0] > 0.0) {
    BGame::GetSimulation()->GetTerrain()->GetColorForHeight(g_dHeightMap[0], dR, dG, dB);
    dR = dBase + dR;
    dG = dBase + dG;
    dB = dBase + dB;
  } else {
    dColor /= 2.0;
    double dDimmer = (64.0 + g_dHeightMap[0]) / 32.0;
    dR = (dBase + dColor * 0.3) * 0.2 + 160.0 * 0.8;
    dG = (dBase / 2.0 + dDimmer * (dBase / 2.0) + dColor * 0.5) * 0.2 + 160.0 * 0.8;
    dB = (dDimmer * dBase + dColor * 0.6) * 0.2 + 255.0 * 0.8;
  }

  // Offer first image
  g_ubMapImageTemp[0] = GLubyte(dR);
  g_ubMapImageTemp[1] = GLubyte(dG);
  g_ubMapImageTemp[2] = GLubyte(dB);

  SDL_LockMutex(g_csComm);
  g_ubMapImage[0] = GLubyte(dR);
  g_ubMapImage[1] = GLubyte(dG);
  g_ubMapImage[2] = GLubyte(dB);
  SDL_UnlockMutex(g_csComm);

  while(true) {

    // Check if new image is needed
    while(true) {
      SDL_LockMutex(g_csComm);
      if(!g_bDoNewImage && !g_bContinueImage) {
        // Check if time to exit
        if(g_bExit) {
          SDL_UnlockMutex(g_csComm);
          break;
        }
        SDL_UnlockMutex(g_csComm);
        SDL_Delay(100);
      } else {
        SDL_UnlockMutex(g_csComm);
        break;
      }
    }

    // Update internal offset since we might have switched to a new area
    vInternalOffset = BGame::GetSimulation()->GetTerrain()->m_vOffset;

    SDL_LockMutex(g_csComm);
    g_bDoNewImage = false;
    g_bContinueImage = false;
    SDL_UnlockMutex(g_csComm);

    // Check if time to exit
    SDL_LockMutex(g_csComm);
    if(g_bExit) {
      SDL_UnlockMutex(g_csComm);
      break;
    }
    SDL_UnlockMutex(g_csComm);

    // Start computing images at increasing resolution
    int nStepSize = g_nStartStepSize;
    int nOffset = nStepSize / 2;
    int nFlipFlop = 1;
    while(nStepSize >= 2) {
      SDL_LockMutex(g_csComm);
      g_bFinal = false;
      SDL_UnlockMutex(g_csComm);

      int x, y;
      // Calculate level
      nOffset = nStepSize / 2;
      nFlipFlop = 1;
      for(y = 0; y < 256; y += nStepSize / 2) {

        // Check if time to exit
        SDL_LockMutex(g_csComm);
        if(g_bExit || g_bDoNewImage) {
          SDL_UnlockMutex(g_csComm);
          break;
        }
        SDL_UnlockMutex(g_csComm);

        int nXStepSize;
        if(nFlipFlop < 0) {
          nXStepSize = nStepSize / 2;
        } else {
          nXStepSize = nStepSize;
        }

        int nHelper;
        if(nOffset == 0) {
          g_dHeightMap[y * 256] = HeightMap::CalcHeightAt(vInternalOffset.m_dX + g_dXOffset, 
                                                        vInternalOffset.m_dY + g_dScale * double(y) + g_dYOffset, 
                                                        dArea, 
                                                        BGame::GetSimulation()->GetScene()->m_terrainStyle,
                                                        8,
                                                        vInternalOffset.m_dY);
          ++nCounter;
          nHelper = nStepSize / 2;
        } else {
          nHelper = 0;
        }

        double dHShadow = g_dHeightMap[y * 256] - g_dScale * (nOffset + nHelper);

        for(x = nOffset + nHelper; x < 256; x += nXStepSize) {

          if(!(++nSkipper % 20) && !BGame::m_bMultiProcessor) {
            SDL_Delay(1);
          }

          g_dHeightMap[y * 256 + x] = HeightMap::CalcHeightAt(vInternalOffset.m_dX + g_dScale * double(x) + g_dXOffset, 
                                                            vInternalOffset.m_dY + g_dScale * double(y) + g_dYOffset, 
                                                            dArea, 
                                                            BGame::GetSimulation()->GetScene()->m_terrainStyle,
                                                        8,
                                                        vInternalOffset.m_dY);
          ++nCounter;

          BVector v2(g_dScale * double(nXStepSize + nOffset), 0, g_dHeightMap[y * 256 + x - nXStepSize + nOffset] - g_dHeightMap[y * 256 + x]);
          v2.ToUnitLength();
          BVector vNormal = vY.CrossProduct(v2);
          vNormal.ToUnitLength();

          double dCos, dBase, dColor;
          // Check for shadowing
          if(g_dHeightMap[y * 256 + x] < dHShadow) {
            dCos = vToSky.ScalarProduct(vNormal);
            dBase = 32.0;
            dColor = dCos * (255.0 - 64.0) * 0.5;
          } else {

            dCos = vToLight.ScalarProduct(vNormal);
            dBase = 64.0;
            if(dCos > 0.0) {
              dColor = dCos * (255.0 - 64.0);
            } else {
              dCos = vToSky.ScalarProduct(vNormal);
              dBase = 32.0;
              dColor = dCos * (255.0 - 64.0) * 0.5;
            }      

            dHShadow = g_dHeightMap[y * 256 + x];
          }
          dHShadow -= g_dScale * (nXStepSize);

          if(g_dHeightMap[y * 256 + x] > 0.0) {
            BGame::GetSimulation()->GetTerrain()->GetColorForHeight(g_dHeightMap[y * 256 + x], dR, dG, dB);
            dR = dBase + dColor * dR;
            dG = dBase + dColor * dG;
            dB = dBase + dColor * dB;
          } else {
            dColor /= 2.0;
            double dDimmer = (64.0 + g_dHeightMap[y * 256 + x]) / 32.0;
            dR = (dBase + dColor * 0.3) * 0.2 + 160.0 * 0.8;
            dG = (dBase / 2.0 + dDimmer * (dBase / 2.0) + dColor * 0.5) * 0.2 + 160.0 * 0.8;
            dB = (dDimmer * dBase + dColor * 0.6) * 0.2 + 255.0 * 0.8;
          }
          g_ubMapImageTemp[y * 256 * 3 + x * 3 + 0] = GLubyte(dR);
          g_ubMapImageTemp[y * 256 * 3 + x * 3 + 1] = GLubyte(dG);
          g_ubMapImageTemp[y * 256 * 3 + x * 3 + 2] = GLubyte(dB);
        }
        nFlipFlop *= -1;
        nOffset += (nFlipFlop * nStepSize / 2);
      }

      // Check if time to exit
      SDL_LockMutex(g_csComm);
      if(g_bExit || g_bDoNewImage) {
        SDL_UnlockMutex(g_csComm);
        break;
      }
      SDL_UnlockMutex(g_csComm);

      // Copy this level image for nav sat window
      SDL_LockMutex(g_csComm);

      // Find out resolution
      int nRes = 256 / (nStepSize / 2);
      int nStep = (nStepSize / 2);

      // Copy image from bigger to smaller image
      int nTmpY, nTmpX;
      nTmpY = 0;
      for(y = 0; y < 256; y += nStep) {
        nTmpX = 0;
        bool bSomeFound = false;
        for(x = 0; x < 256; ++x) {
          if((g_ubMapImageTemp[y * 3 * 256 + x * 3 + 0] + 
              g_ubMapImageTemp[y * 3 * 256 + x * 3 + 1] + 
              g_ubMapImageTemp[y * 3 * 256 + x * 3 + 2]) > 0.0) {
            g_ubMapImage[nTmpY * 3 * nRes + nTmpX * 3 + 0] = g_ubMapImageTemp[y * 3 * 256 + x * 3 + 0];
            g_ubMapImage[nTmpY * 3 * nRes + nTmpX * 3 + 1] = g_ubMapImageTemp[y * 3 * 256 + x * 3 + 1];
            g_ubMapImage[nTmpY * 3 * nRes + nTmpX * 3 + 2] = g_ubMapImageTemp[y * 3 * 256 + x * 3 + 2];
            x += (nStep - 1);
            ++nTmpX;
            bSomeFound = true;
          }          
        }
        //if(bSomeFound) {
          ++nTmpY;
        //}
      }
      g_nImageSize = nRes;
      g_bNewImageAvailable = true;

      SDL_UnlockMutex(g_csComm);

      // Check if time to exit
      SDL_LockMutex(g_csComm);
      if(g_bExit || g_bDoNewImage) {
        SDL_UnlockMutex(g_csComm);
        break;
      }
      SDL_UnlockMutex(g_csComm);

      // Go to next detail level
      nStepSize /= 2;
      g_nStartStepSize = nStepSize;
    }

    // Check if time to exit
    SDL_LockMutex(g_csComm);
    if(!g_bExit && !g_bDoNewImage) {
      SDL_UnlockMutex(g_csComm);
      // Shadowing
      int x, y;
      for(y = 0; y < 256; ++y) {
        double dHShadow = g_dHeightMap[y * 256] - g_dScale * 1.0;
        for(x = 1; x < 256; ++x) {

          BVector v2(g_dScale, 0, g_dHeightMap[y * 256 + x - 1] - g_dHeightMap[y * 256 + x]);
          v2.ToUnitLength();
          BVector vNormal = vY.CrossProduct(v2);
          vNormal.ToUnitLength();

          double dCos, dBase, dColor;
          // Check for shadowing
          if(g_dHeightMap[y * 256 + x] < dHShadow) {
            dCos = vToSky.ScalarProduct(vNormal);
            dBase = 32.0;
            dColor = dCos * (255.0 - 64.0) * 0.5;
          } else {

            dCos = vToLight.ScalarProduct(vNormal);

            dBase = 64.0;
            if(dCos > 0.0) {
              dColor = dCos * (255.0 - 64.0);
            } else {
              dCos = vToSky.ScalarProduct(vNormal);
              dBase = 32.0;
              dColor = dCos * (255.0 - 64.0) * 0.5;
            }      

            dHShadow = g_dHeightMap[y * 256 + x];
          }
          dHShadow -= g_dScale * 1.0;

          if(g_dHeightMap[y * 256 + x] > 0.0) {
            BGame::GetSimulation()->GetTerrain()->GetColorForHeight(g_dHeightMap[y * 256 + x], dR, dG, dB);
            dR = dBase + dColor * dR;
            dG = dBase + dColor * dG;
            dB = dBase + dColor * dB;
          } else {
            dColor /= 2.0;
            double dDimmer = (64.0 + g_dHeightMap[y * 256 + x]) / 32.0;
            dR = (dBase + dColor * 0.3) * 0.2 + 160.0 * 0.8;
            dG = (dBase / 2.0 + dDimmer * (dBase / 2.0) + dColor * 0.5) * 0.2 + 160.0 * 0.8;
            dB = (dDimmer * dBase + dColor * 0.6) * 0.2 + 255.0 * 0.8;
          }
          g_ubMapImageTemp[y * 256 * 3 + x * 3 + 0] = GLubyte(dR);
          g_ubMapImageTemp[y * 256 * 3 + x * 3 + 1] = GLubyte(dG);
          g_ubMapImageTemp[y * 256 * 3 + x * 3 + 2] = GLubyte(dB);
        }
      }
      // Copy final image
      SDL_LockMutex(g_csComm);
      for(y = 0; y < 256; ++y) {
        for(x = 0; x < 256; ++x) {
          g_ubMapImage[y * 3 * 256 + x * 3 + 0] = g_ubMapImageTemp[y * 3 * 256 + x * 3 + 0];
          g_ubMapImage[y * 3 * 256 + x * 3 + 1] = g_ubMapImageTemp[y * 3 * 256 + x * 3 + 1];
          g_ubMapImage[y * 3 * 256 + x * 3 + 2] = g_ubMapImageTemp[y * 3 * 256 + x * 3 + 2];
        }
      }
      g_nImageSize = 256;
      g_bNewImageAvailable = true;
      g_bFinal = true;
      SDL_UnlockMutex(g_csComm);
    } else {
      SDL_UnlockMutex(g_csComm);
    }
  }

  return 0;
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
