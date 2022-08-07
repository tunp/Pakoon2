//
// Base classes for the Pakoon1
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#pragma once

#ifdef _WIN32
#include <windows.h>
#endif

#include "CoreClasses.h"
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <string>

#ifndef __EMSCRIPTEN__
#define ENABLE_MULTIPLAY
#endif


using namespace std;

const int g_cnMaxTrailPoints = 100;

class BDustCloud {
public:
  BVector m_vLocation; // Where is it
  BVector m_vector;    // Where it's going
  bool    m_bWater;    // Water or dust?
  double  m_dAlpha;    // How dense it is
  double  m_dSize;     // How big it is
  BVector m_color;
  BDustCloud() {}
};


/*
class B2DPoint {
public:
  double m_dX;
  double m_dY;
};
*/

class BBoundaryPoint : public BVector {
public:
  BBoundaryPoint *m_pNext;
  BBoundaryPoint() {m_pNext = 0;}
};

class BBoundary {
  BBoundaryPoint *AlreadyInserted(BVector& rP);
public:
  BBoundaryPoint *m_pHead;
  BBoundary() {m_pHead = 0;}
  void            Empty();
  bool            PointIsLeftOfLine(BBoundaryPoint *pStart, double dX, double dY);
  bool            TurnsRight(BBoundaryPoint *p);
  BBoundaryPoint *Head() {return m_pHead;}
  BBoundaryPoint *AddAfter(BBoundaryPoint *pAfter, BVector& rNew2D);
  BBoundaryPoint *PointIsOutside(BVector& rP);
  BBoundaryPoint *PointIsOutside(double dX, double dY);
  BBoundaryPoint *Delete(BBoundaryPoint *p);
};


class BOBJVertex : public BVector {
public:
  BVector m_vNormal;
};


class BOBJFace {
public:
  int     m_nV1;
  int     m_nV2;
  int     m_nV3;
  int     m_nV4; // used only by the collision detection load
  int     m_nVertices;
  BVector m_vNormal;
  int     m_nTexCoords[3]; // used only for the FromFile texture mapping
  BVector m_vTexCoords[3]; // used only for the FromFile texture mapping
};


class BOBJData {
public:

  void Init();
  void Free();
  BOBJData();
  ~BOBJData();

  int           m_nOBJPoints;
  int           m_nOBJFaces;
  BOBJVertex   *m_pvOBJPoints;
  BOBJFace     *m_pOBJFaces;

  string       m_sRightDir;
  string       m_sForwardDir;
  string       m_sDownDir;
  double        m_dScale;

  BVector       m_vMassCenter;

  void PrepareWaveFrontModel();
};



// One (visualization) part of the vehicle/object
class BPart {

public:

  BOBJData *m_pOBJData;

  BPart();

  void AppendWaveFrontOBJShape(string sShapeFilename, 
                               int &rnVertices,
                               int &rnFaces,
                               bool bUseMassCenter,
                               bool bReadTexCoords,
                               bool bSplitQuads);

  void LoadPartFromFile(string sFilename, 
                        string sSection,
                        bool bOnlyShape,
                        bool bUseMassCenter);

  void SetPartColor(GLfloat fAlpha = 1.0);
  void RenderPart(int nReset);

  void SetOBJData(BOBJData *pObjData) {m_pOBJData = pObjData;}

  enum TTexMapping {FROMFILE, ENVMAP, WRAPAROUND};
  enum TShading {SMOOTH, FLAT, NONE};

  string     m_sShapeFilename;
  int         m_nVertices;
  int         m_nVertexStart;
  int         m_nFaces;
  int         m_nFaceStart;
  int         m_nTexVertices;
  int         m_nTexVertexStart;
  bool        m_bTextured;
  double      m_dRed;
  double      m_dGreen;
  double      m_dBlue;
  double      m_dAmbient;
  double      m_dDiffuse;
  double      m_dSpecular;
  double      m_dBrilliance;
  bool        m_bShiny;
  TShading    m_shading;
  int         m_nTexture;
  TTexMapping m_tmTextureMapping;
  double      m_dWrapFactor;
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
