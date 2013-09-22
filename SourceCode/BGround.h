//
// Ground
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#pragma once

#include "BaseClasses.h"
#include "BCamera.h"
#include "BObject.h"
#include "gl\gl.h"
#include "gl\glu.h"

const double g_cdGroundScale = 20.0;
const double g_cdGroundHeightScale = 0.5;
const int    g_cnGroundXSize = 96; // must be multiple of g_cnAreaSize
const int    g_cnGroundYSize = 48; // must be multiple of g_cnAreaSize
//const int    g_cnGroundXSize = 160; // must be multiple of g_cnAreaSize
//const int    g_cnGroundYSize = 112; // must be multiple of g_cnAreaSize
const int    g_cnAreaSize = 16;    // must be power of two
const int    g_cnAreas = (g_cnGroundXSize / g_cnAreaSize) * (g_cnGroundYSize / g_cnAreaSize);
const double g_cdWaterLevel = -2.499;

class BGroundArea {
public:
  int     m_nDLFullRes1;
  int     m_nDLFullRes2;
  int     m_nDLFullRes3;
  int     m_nDLCoarseRes1;
  int     m_nDLCoarseRes2;
  int     m_nDLCoarseRes3;
  BVector m_vCenter;
  BGroundArea() {m_nDLFullRes1 = 
                 m_nDLFullRes2 = 
                 m_nDLFullRes3 = 
                 m_nDLCoarseRes1 = 
                 m_nDLCoarseRes2 =
                 m_nDLCoarseRes3 = 0;}
};

class BGroundSquare {
public:
  double  m_dFriction1;
  double  m_dFriction2;
  double  m_dSurfaceDepth1;
  double  m_dSurfaceDepth2;
  bool    m_bWater1;
  bool    m_bWater2;
  BVector m_vNormal1;
  BVector m_vNormal2;
  BVector m_vNormalVis;
  BGroundSquare() {m_bWater1 = m_bWater2 = false;};
};

class BGround {

  int m_nDLSkyCoarse;
  int m_nDLSkyDetailed;
  int m_nDLSeaBed;
  int m_nDLWaterSurface;
  int m_nDLGroundWater;
  int m_nDLObjects;
  int m_nDLGroundSand;
  int m_nDLGroundRock;

public:

  enum TGroundType {GT_GROUND, GT_SAND, GT_ROCK, GT_WATER};

  BGroundArea   m_areas[g_cnAreas];
  // GLubyte       m_dMonVal[(128 * 128 * 3) * 2]; // reserve room also for mipmaps
  // GLubyte       m_dSkyMap[256 * 256 * 3];
  double        m_dHeightMap[g_cnGroundYSize * g_cnGroundXSize];
  BGroundSquare m_sqrHeightMapNormals[g_cnGroundYSize * g_cnGroundXSize];
  double        m_dCoarseDistance;

  int           m_nObjects;
  BObject      *m_pObjects;

  BGround();
  virtual ~BGround();

  void PreProcessVisualization();
  void CreateGroundAreas();
  bool GroundTriangleIsOfType(int x, int y, int nSize, int nTriangle, TGroundType gt);
  double SurfaceHeightAt(int x, int y, TGroundType gt);
  void RenderAreaFull(int nX, int nY, int nSize, TGroundType gt);
  void RenderAreaCoarse(int nX, int nY, int nSize, TGroundType gt);
  void RenderSky(int nDispList);
  bool AreaFlatEnough(int nX, int nY, int nSize);
  void PaintGroundAreas(BCamera &rCamera, TGroundType gt);
  void Paint(int nSkyDetail, int nWaterDetail, BCamera &rCamera, TGroundType gt);
  void PaintSky(int nSkyDetail);
  void PaintObjects(BCamera &rCamera);
  void PaintWaterSurface(double dBrightness, double dAlpha);
  void LoadGroundFromFile(CString sFilename);
  void LoadObjectsFromFile(CString sFilename);
  void LoadTextures();
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