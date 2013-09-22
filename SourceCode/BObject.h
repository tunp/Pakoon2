//
// Object
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#pragma once

#include "BaseClasses.h"
#include "gl\gl.h"
#include "gl\glu.h"

class BFace {
public:
  int     m_nPoints;
  BVector m_vPoint[5];
  BVector m_vNormal;

  BFace() {m_nPoints = 0;}
  void SetPoints(int nPoints, BVector *pvPoints);
  void CalculateNormal();
};


class BObjectPart : public BPart {
public:
  double m_dFriction;
  int    m_nFaces;
  BFace *m_pFace;

  BObjectPart() {m_nFaces = 0; m_pFace = 0; m_dFriction = 0.5;}
  ~BObjectPart();
};


class BObject {

  BOBJData *m_pOBJData;

public:

  enum TCollDet {ACCURATE, BOUNDING_SPHERE, BOUNDING_BOX};
  enum TType {USER_DEF};

  int  m_nDL;
  int  m_nDLShadow;
  bool m_bDLIsValid;

  int       m_nParts; // visualization parts
  BPart    *m_pPart;  // visualization parts

  int          m_nCollDetParts; // collision detection parts
  BObjectPart *m_pCollDetPart;  // collision detection parts

  CString   m_sObjectFilename;

  CString   m_sName;
  TType     m_type;
  TCollDet  m_collisionDetection;

  BVector   m_vLocation;
  BVector   m_vCenter;
  double    m_dScale;
  double    m_dScale2;
  double    m_dZRotation;
  double    m_dRadius;
  double    m_dRed;
  double    m_dGreen;
  double    m_dBlue;
  double    m_dFriction;
  BVector   m_vCenterOnGround; // Same as m_vCenter except that the Z-coordinate is brought to the ground level
  double    m_dActiveRadius;   // Max distance (from m_vCenterOnGround) at which pickup or delivery is successful

  bool      m_bVisible;
  bool      m_bHasShadow;
  BBoundary m_boundary;
  BBoundary m_shadow;
  bool      m_bShadowPolygonOK;

  BObject();
  ~BObject();

  void SetOBJData(BOBJData *pOBJData) {m_pOBJData = pOBJData;}

  void LoadObjectFromFile(CString sFilename, CString sSection, bool bOnlyShape = false);
  void LoadCollisionPartFromFile(CString sFilename, CString sSection, BPart &rVisPart, BObjectPart &rPart);

  void   Setup();
  double PointIsInsideObject(BVector& rvPoint, 
                             BVector& rvNormal, 
                             double& rdGroundFriction, 
                             double& rdBaseDepth);
  void   CreateXYBoundary();
  void   CreateShadowPolygon();
  void   RecreateShadow();

  void PreProcessVisualization(bool bOnlyShadow = false);
  void DrawObject(bool bShadow);
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
