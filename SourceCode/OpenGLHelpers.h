//
// OpenGL helpers
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#pragma once

#include "BaseClasses.h"
#include "gl\gl.h"
#include "gl\glu.h"
#include "gl\glext.h"


class OpenGLHelpers {

  static GLuint m_nDLTex;

public:

  static bool m_bMultiTexturing;

  static void Init();
  static bool LoadExtensionFunctions();
  static void SetColorFullSafe(double dRed, double dGreen, double dBlue, double dAlpha);
  static void SetColorFull(double dRed, double dGreen, double dBlue, double dAlpha);
  static void SetColorFace(double dRed, double dGreen, double dBlue, double dAlpha, GLenum face);
  static void SetColor(double dRed, double dGreen, double dBlue, double dAlpha);
  static void SwitchToTexture(int nTexture = 0, bool bDisable = false);
  static void SetMoonLighting();
  static void SetDefaultLighting();
  static void SetEveningLighting();
  static void SetStrongLighting();
  static void EndStrongLighting();
  static void Triangle(BVector& p1, BVector& p2, BVector& p3);
  static void TxtrTriangle(BVector& p1, double dTxtrX1, double dTxtrY1, 
                           BVector& p2, double dTxtrX2, double dTxtrY2, 
                           BVector& p3, double dTxtrX3, double dTxtrY3);
  static void TxtrTriangleBare(BVector& p1, double dTxtrX1, double dTxtrY1, 
                               BVector& p2, double dTxtrX2, double dTxtrY2, 
                               BVector& p3, double dTxtrX3, double dTxtrY3);
  static void TriangleStrip(BVector *pvPoints, int nPoints);
  static void TriangleStripWithNormals(BVector *pvPoints, BVector *pvNormals, int nPoints);
  static void TriangleStripTexturedX(BVector *pvPoints, 
                                     int      nPoints, 
                                     int      nOrig1, 
                                     int      nOrig2, 
                                     double   dTXTRMin, 
                                     double   dTXTRMax);
  static void TriangleFan(BVector *pvPoints, int nPoints);
  static void TriangleFanWithNormals(BVector *pvPoints, BVector *pvNormals, int nPoints);
  static void Line(BVector& p1, BVector& p2);
  static void Lines(BVector *pvPoints, int nPoints);

  static void CreateTexName(int nTexIndex, GLuint *pnTexName);
  static void FreeTexName(int nTexIndex, GLuint *pnTexName);

  static void CreateMipMaps(GLubyte *pStart, int nWidth, int nHeight, int nComponents);
  static void BindMipMapTexture(int nWidth, int nHeight, int nComponents, GLenum format, GLubyte *pStart, int nTexIndex, GLuint nTexName);
  static void BindTexture(int nWidth, int nHeight, int nComponents, GLenum format, GLubyte *pStart, int nTexIndex, GLuint nTexName, bool bForceCreate = false);
  static void SetTexCoord(double x, double y);
  static void DrawVeil(double dRed, double dGreen, double dBlue, double dAlpha, CRect &rectWnd);
  static void DrawTexturedRectangle(double dX, 
                                    double dY, 
                                    double dTexX, 
                                    double dTexY, 
                                    double dXSize, 
                                    double dYSize,
                                    double dTexXSize, 
                                    double dTexYSize);
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
