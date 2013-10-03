//
// OpenGL helpers
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#include "OpenGLHelpers.h"
#include "BGame.h"

extern bool g_cbBlackAndWhite;
extern bool g_cbMipMap;


bool OpenGLHelpers::m_bMultiTexturing = false;


// Multitexturing support functions
#ifdef _WIN32
PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB = NULL;
PFNGLACTIVETEXTUREARBPROC   glActiveTextureARB = NULL;
#endif

GLuint OpenGLHelpers::m_nDLTex = 0;

void OpenGLHelpers::Init() {
  // Load multitexturing support
  LoadExtensionFunctions();

  // Initialize texture switching display list
  m_nDLTex = glGenLists(1);
  glNewList(m_nDLTex, GL_COMPILE);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glEndList();
}


void OpenGLHelpers::SwitchToTexture(int nTexture, bool bDisable) {
  if(m_bMultiTexturing) {
    if(nTexture == 0) {
      glActiveTextureARB(GL_TEXTURE0_ARB);
    } else {
      glActiveTextureARB(GL_TEXTURE1_ARB);
    }
  }
  if(bDisable) {
    glDisable(GL_TEXTURE_2D);
  } else {
    glEnable(GL_TEXTURE_2D);
  }

  if(bDisable) {
    // This is a hack but I don't know how to get rid of the second texture.
    if(m_bMultiTexturing) {
      glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1.0f/512.0f, 1.0f/512.0f);
    }
  } else {
    glCallList(m_nDLTex);
  }
}



bool OpenGLHelpers::LoadExtensionFunctions() {
  // Check for multitexturing support
  string sExt = (char *) glGetString(GL_EXTENSIONS);
  if(sExt.find("GL_ARB_multitexture") != -1) {
    // Load extension functions
    #ifdef _WIN32
    glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC) wglGetProcAddress("glMultiTexCoord2fARB");
    glActiveTextureARB   = (PFNGLACTIVETEXTUREARBPROC)   wglGetProcAddress("glActiveTextureARB");
    #endif
    m_bMultiTexturing = true;
    // m_bMultiTexturing = false; // no need for multitexturing yet
    return true;
  } else {
    BGame::MyAfxMessageBox("Multitexturing is not supported.");
    m_bMultiTexturing = false;
    return false;
  }
}





void OpenGLHelpers::SetColorFullSafe(double dRed, double dGreen, double dBlue, double dAlpha) {

  if(dRed > 1.0) {
    dRed = 1.0;
  }
  if(dGreen > 1.0) {
    dGreen = 1.0;
  }
  if(dBlue > 1.0) {
    dBlue = 1.0;
  }
  if(dRed < 0.0) {
    dRed = 0.0;
  }
  if(dGreen < 0.0) {
    dGreen = 0.0;
  }
  if(dBlue < 0.0) {
    dBlue = 0.0;
  }

  if(g_cbBlackAndWhite) {
    double bw;
    bw = (dRed*0.37+dGreen*0.42+dBlue*0.2);
    dRed = dGreen = dBlue = bw;
  }

  static GLfloat fCurColor[4];
  fCurColor[0] = (GLfloat) dRed;
  fCurColor[1] = (GLfloat) dGreen;
  fCurColor[2] = (GLfloat) dBlue;
  fCurColor[3] = (GLfloat) dAlpha;
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, fCurColor);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, fCurColor);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, fCurColor);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 10.0f);

}

void OpenGLHelpers::SetColorFace(double dRed, double dGreen, double dBlue, double dAlpha, GLenum face) {

  static GLfloat fCurColor[4];
  fCurColor[0] = (GLfloat) dRed;
  fCurColor[1] = (GLfloat) dGreen;
  fCurColor[2] = (GLfloat) dBlue;
  fCurColor[3] = (GLfloat) dAlpha;
  glMaterialfv(face, GL_AMBIENT, fCurColor);
  glMaterialfv(face, GL_DIFFUSE, fCurColor);
  glMaterialfv(face, GL_SPECULAR, fCurColor);
  glMaterialf(face, GL_SHININESS, 10.0f);

}


void OpenGLHelpers::SetColorFull(double dRed, double dGreen, double dBlue, double dAlpha) {

  static GLfloat fCurColor[4];
  fCurColor[0] = (GLfloat) dRed;
  fCurColor[1] = (GLfloat) dGreen;
  fCurColor[2] = (GLfloat) dBlue;
  fCurColor[3] = (GLfloat) dAlpha;
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, fCurColor);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, fCurColor);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, fCurColor);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 10.0f);

}

void OpenGLHelpers::SetColor(double dRed, double dGreen, double dBlue, double dAlpha) {
  glColor3d((GLdouble) dRed, (GLdouble) dGreen, (GLdouble) dBlue);
}




void OpenGLHelpers::SetMoonLighting() {
  GLfloat fLight1AmbientG[ 4];
  GLfloat fLight1DiffuseG[ 4];
  GLfloat fLight1SpecularG[ 4];

  fLight1AmbientG[0] = -0.1f;
  fLight1AmbientG[1] = -0.1f;
  fLight1AmbientG[2] = -0.05f;
  fLight1AmbientG[3] = 1.0f;

  fLight1DiffuseG[0] = 0.0f;
  fLight1DiffuseG[1] = 0.0f;
  fLight1DiffuseG[2] = 0.0f;
  fLight1DiffuseG[3] = 1.0f;

  fLight1SpecularG[0] = 0.0f;
  fLight1SpecularG[1] = 0.0f;
  fLight1SpecularG[2] = 0.0f;
  fLight1SpecularG[3] = 1.0f;

  glLightfv( GL_LIGHT0, GL_AMBIENT,  fLight1AmbientG);
  glLightfv( GL_LIGHT0, GL_DIFFUSE,  fLight1DiffuseG);
  glLightfv( GL_LIGHT0, GL_SPECULAR, fLight1SpecularG);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, (GLfloat) 1.0);
}

void OpenGLHelpers::SetEveningLighting() {
  GLfloat fLight1AmbientG[ 4];
  GLfloat fLight1DiffuseG[ 4];
  GLfloat fLight1SpecularG[ 4];

  fLight1AmbientG[0] = 0.05f;
  fLight1AmbientG[1] = 0.05f;
  fLight1AmbientG[2] = 0.1f;
  fLight1AmbientG[3] = 1.0f;

  fLight1DiffuseG[0] = 0.7f;
  fLight1DiffuseG[1] = 0.7f;
  fLight1DiffuseG[2] = 0.7f;
  fLight1DiffuseG[3] = 1.0f;

  fLight1SpecularG[0] = 0;
  fLight1SpecularG[1] = 0;
  fLight1SpecularG[2] = 0;
  fLight1SpecularG[3] = 1;

  glLightfv( GL_LIGHT0, GL_AMBIENT,  fLight1AmbientG);
  glLightfv( GL_LIGHT0, GL_DIFFUSE,  fLight1DiffuseG);
  glLightfv( GL_LIGHT0, GL_SPECULAR, fLight1SpecularG);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, (GLfloat) 10.0);
}

void OpenGLHelpers::SetDefaultLighting() {
  GLfloat fLight1AmbientG[ 4];
  GLfloat fLight1DiffuseG[ 4];
  GLfloat fLight1SpecularG[ 4];

  fLight1AmbientG[0] = 0.2f;
  fLight1AmbientG[1] = 0.2f;
  fLight1AmbientG[2] = 0.2f;
  fLight1AmbientG[3] = 0.2f;

  fLight1DiffuseG[0] = 0.7f;
  fLight1DiffuseG[1] = 0.7f;
  fLight1DiffuseG[2] = 0.7f;
  fLight1DiffuseG[3] = 0.7f;

  fLight1SpecularG[0] = 0.25f;
  fLight1SpecularG[1] = 0.25f;
  fLight1SpecularG[2] = 0.25f;
  fLight1SpecularG[3] = 0.25f;

  glLightfv( GL_LIGHT0, GL_AMBIENT,  fLight1AmbientG);
  glLightfv( GL_LIGHT0, GL_DIFFUSE,  fLight1DiffuseG);
  glLightfv( GL_LIGHT0, GL_SPECULAR, fLight1SpecularG);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, (GLfloat) 1.0);
}


void OpenGLHelpers::SetStrongLighting() {
  GLfloat fLight1AmbientG[ 4];
  GLfloat fLight1DiffuseG[ 4];
  GLfloat fLight1SpecularG[ 4];

  fLight1AmbientG[0] = 0;
  fLight1AmbientG[1] = 0;
  fLight1AmbientG[2] = 0;
  fLight1AmbientG[3] = 0;

  fLight1DiffuseG[0] = 0;
  fLight1DiffuseG[1] = 0;
  fLight1DiffuseG[2] = 0;
  fLight1DiffuseG[3] = 0;

  fLight1SpecularG[0] = 1;
  fLight1SpecularG[1] = 1;
  fLight1SpecularG[2] = 1;
  fLight1SpecularG[3] = 1;

  glLightfv( GL_LIGHT0, GL_AMBIENT,  fLight1AmbientG);
  glLightfv( GL_LIGHT0, GL_DIFFUSE,  fLight1DiffuseG);
  glLightfv( GL_LIGHT0, GL_SPECULAR, fLight1SpecularG);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, (GLfloat) 20.0);

  GLfloat fLight1PositionG[ 4];
  fLight1PositionG[0] = (GLfloat) 0;
  fLight1PositionG[1] = (GLfloat) 10000.0f;
  fLight1PositionG[2] = (GLfloat) -2000.0f; // -1;
  fLight1PositionG[3] = (GLfloat) 1; /* w=0 -> directional light (not positional) */
  glLightfv( GL_LIGHT0, GL_POSITION, fLight1PositionG);                     
}

void OpenGLHelpers::EndStrongLighting() {
  if(BGame::m_bNight) {
    SetMoonLighting();
  } else {
    SetDefaultLighting();
  }

  GLfloat fLight1PositionG[ 4];
  fLight1PositionG[0] = (GLfloat) 1;
  fLight1PositionG[1] = (GLfloat) 1;
  fLight1PositionG[2] = (GLfloat) -1;
  fLight1PositionG[3] = (GLfloat) 0; // w=0 -> directional light (not positional)
  glLightfv( GL_LIGHT0, GL_POSITION, fLight1PositionG);                     
}



void OpenGLHelpers::Triangle(BVector& p1, BVector& p2, BVector& p3) {
  glBegin(GL_TRIANGLES);
  glVertex3f(p1.m_dX, p1.m_dY, p1.m_dZ);
  glVertex3f(p2.m_dX, p2.m_dY, p2.m_dZ);
  glVertex3f(p3.m_dX, p3.m_dY, p3.m_dZ);
  glEnd();
}

void OpenGLHelpers::TxtrTriangleBare(BVector& p1, double dTxtrX1, double dTxtrY1, 
                                     BVector& p2, double dTxtrX2, double dTxtrY2, 
                                     BVector& p3, double dTxtrX3, double dTxtrY3) {
  glTexCoord2f(dTxtrX1, dTxtrY1);
  glVertex3f(p1.m_dX, p1.m_dY, p1.m_dZ);
  glTexCoord2f(dTxtrX2, dTxtrY2);
  glVertex3f(p2.m_dX, p2.m_dY, p2.m_dZ);
  glTexCoord2f(dTxtrX3, dTxtrY3);
  glVertex3f(p3.m_dX, p3.m_dY, p3.m_dZ);
}


void OpenGLHelpers::TxtrTriangle(BVector& p1, double dTxtrX1, double dTxtrY1, 
                                 BVector& p2, double dTxtrX2, double dTxtrY2, 
                                 BVector& p3, double dTxtrX3, double dTxtrY3) {
  glBegin(GL_TRIANGLES);
  glTexCoord2f(dTxtrX1, dTxtrY1);
  glVertex3f(p1.m_dX, p1.m_dY, p1.m_dZ);
  glTexCoord2f(dTxtrX2, dTxtrY2);
  glVertex3f(p2.m_dX, p2.m_dY, p2.m_dZ);
  glTexCoord2f(dTxtrX3, dTxtrY3);
  glVertex3f(p3.m_dX, p3.m_dY, p3.m_dZ);
  glEnd();
}

void OpenGLHelpers::TriangleStrip(BVector *pvPoints, int nPoints) {
  glBegin(GL_TRIANGLE_STRIP);
  for(int i = 0; i < nPoints; ++i) {
    glVertex3f(pvPoints[i].m_dX, pvPoints[i].m_dY, pvPoints[i].m_dZ);
  }
  glEnd();
}

void OpenGLHelpers::TriangleStripWithNormals(BVector *pvPoints, BVector *pvNormals, int nPoints) {
  glBegin(GL_TRIANGLE_STRIP);
  for(int i = 0; i < nPoints; ++i) {
    if(!(i % 2)) {
      glNormal3f(pvNormals[i/2].m_dX, pvNormals[i/2].m_dY, pvNormals[i/2].m_dZ);
    }
    glVertex3f(pvPoints[i].m_dX, pvPoints[i].m_dY, pvPoints[i].m_dZ);
  }
  glEnd();
}

void OpenGLHelpers::TriangleStripTexturedX(BVector *pvPoints, 
                                           int      nPoints, 
                                           int      nOrig1, 
                                           int      nOrig2, 
                                           double   dTXTRMin, 
                                           double   dTXTRMax) {
  double dOffset1, dFactor, dOffset2;
  dOffset1 = pvPoints[nOrig1].m_dY;
  dOffset2 = pvPoints[nOrig1].m_dZ;
  dFactor  = fabs(pvPoints[nOrig1].m_dY - pvPoints[nOrig2].m_dY);
  glBegin(GL_TRIANGLE_STRIP);
  for(int i = 0; i < nPoints; ++i) {
    glTexCoord2f((pvPoints[i].m_dY - dOffset1) * dFactor, 
                 (pvPoints[i].m_dZ - dOffset2) * dFactor);
    glVertex3f(pvPoints[i].m_dX, pvPoints[i].m_dY, pvPoints[i].m_dZ);
  }
  glEnd();
}


void OpenGLHelpers::TriangleFan(BVector *pvPoints, int nPoints) {
  glBegin(GL_TRIANGLE_FAN);
  for(int i = 0; i < nPoints; ++i) {
    glVertex3f(pvPoints[i].m_dX, pvPoints[i].m_dY, pvPoints[i].m_dZ);
  }
  glEnd();
}

void OpenGLHelpers::TriangleFanWithNormals(BVector *pvPoints, BVector *pvNormals, int nPoints) {
  glBegin(GL_TRIANGLE_FAN);
  for(int i = 0; i < nPoints; ++i) {
    glNormal3f(pvNormals[i].m_dX, pvNormals[i].m_dY, pvNormals[i].m_dZ);
    glVertex3f(pvPoints[i].m_dX, pvPoints[i].m_dY, pvPoints[i].m_dZ);
  }
  glEnd();
}

void OpenGLHelpers::Line(BVector p1, BVector p2) {
  glBegin(GL_LINES);
  glVertex3f(p1.m_dX, p1.m_dY, p1.m_dZ);
  glVertex3f(p2.m_dX, p2.m_dY, p2.m_dZ);
  glEnd();
}

void OpenGLHelpers::Lines(BVector *pvPoints, int nPoints) {
  glBegin(GL_LINES);
  for(int i = 0; i < nPoints; ++i) {
    glVertex3f(pvPoints[i].m_dX, pvPoints[i].m_dY, pvPoints[i].m_dZ);
  }
  glEnd();
}

void OpenGLHelpers::CreateMipMaps(GLubyte *pStart, int nWidth, int nHeight, int nComponents) {
  return; // This is now obsolete
}



bool   g_nBoundTextures[100] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void OpenGLHelpers::CreateTexName(int nTexIndex, GLuint *pnTexName) {
  glGenTextures(1, pnTexName);
  GLenum error = glGetError();
  g_nBoundTextures[nTexIndex] = false;
}

void OpenGLHelpers::FreeTexName(int nTexIndex, GLuint *pnTexName) {
  // Free the textures and mark them as not initialized
  glDeleteTextures(1, pnTexName);
  g_nBoundTextures[nTexIndex] = false;
}

void OpenGLHelpers::BindMipMapTexture(int nWidth, int nHeight, int nComponents, GLenum format, GLubyte *pStart, int nTexIndex, GLuint nTexName) {

  if(g_nBoundTextures[nTexIndex]) {
    glBindTexture(GL_TEXTURE_2D, nTexName);
    return;
  }

  g_nBoundTextures[nTexIndex] = true;
  glBindTexture(GL_TEXTURE_2D, nTexName);

  gluBuild2DMipmaps(GL_TEXTURE_2D,
                    nComponents,
                    nWidth,
                    nHeight,
                    format,
                    GL_UNSIGNED_BYTE,
                    pStart);
  return;
}

void OpenGLHelpers::BindTexture(int nWidth, int nHeight, int nComponents, GLenum format, GLubyte *pStart, int nTexIndex, GLuint nTexName, bool bForceCreate) {

  if(!bForceCreate) {
    if(g_nBoundTextures[nTexIndex]) {
      glBindTexture(GL_TEXTURE_2D, nTexName);
      return;
    }
  }

  g_nBoundTextures[nTexIndex] = true;
  glBindTexture(GL_TEXTURE_2D, nTexName);

  glTexImage2D(GL_TEXTURE_2D,
               0,
               nComponents,
               nWidth,
               nHeight,
               0,
               format,
               GL_UNSIGNED_BYTE,
               pStart);

  return;
}




void OpenGLHelpers::SetTexCoord(double x, double y) {
  if(m_bMultiTexturing) {
    glMultiTexCoord2fARB(GL_TEXTURE0_ARB, float(x), float(y));
  } else {
    glTexCoord2f(x, y);
  }
}


void OpenGLHelpers::DrawVeil(double dRed, double dGreen, double dBlue, double dAlpha, SDL_Rect &rectWnd) {
  OpenGLHelpers::SetColorFull(dRed, dGreen, dBlue, dAlpha);
  glDisable(GL_TEXTURE_2D);

  glBegin(GL_TRIANGLE_STRIP);
  glVertex3f(0, 0, 0);
  glVertex3f(0, rectWnd.h, 0);
  glVertex3f(rectWnd.w, 0, 0);
  glVertex3f(rectWnd.w, rectWnd.h, 0);
  glEnd();
  glEnable(GL_TEXTURE_2D);
}


void OpenGLHelpers::DrawTexturedRectangle(double dX, 
                                          double dY, 
                                          double dTexX, 
                                          double dTexY, 
                                          double dXSize, 
                                          double dYSize,
                                          double dTexXSize, 
                                          double dTexYSize) {
  glBegin(GL_QUADS);
  SetTexCoord(dTexX / dTexXSize, (dTexYSize - dTexY) / dTexYSize);
  glVertex2f(dX, dY);
  SetTexCoord((dTexX + dXSize) / dTexXSize, (dTexYSize - dTexY) / dTexYSize);
  glVertex2f(dX + dXSize, dY);
  SetTexCoord((dTexX + dXSize) / dTexXSize, (dTexYSize - dTexY + dYSize) / dTexYSize);
  glVertex2f(dX + dXSize, dY + dYSize);
  SetTexCoord(dTexX / dTexXSize, (dTexYSize - dTexY + dYSize) / dTexYSize);
  glVertex2f(dX, dY + dYSize);
  glEnd();
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
