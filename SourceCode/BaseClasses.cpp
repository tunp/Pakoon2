//
// Base classes for the Pakoon1
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#include "BaseClasses.h"
#include "FileIOHelpers.h"
#include "BTextures.h"
#include "OpenGLHelpers.h"
#include "BGame.h"

//**********************************************************************

#define VDot(a, b, c) {a=(b).dX*(c).dX+(b).dY*(c).dY+(b).dZ*(c).dZ;}

//*************************************************************************************************
BBoundaryPoint *BBoundary::AlreadyInserted(BVector& rP) {
  if(m_pHead) {
    BBoundaryPoint *pLoop = m_pHead;
    do {
      if((fabs(pLoop->m_dX - rP.m_dX) < 0.001) && 
         (fabs(pLoop->m_dY - rP.m_dY) < 0.001)) {
        return pLoop;
      }
      pLoop = pLoop->m_pNext;
    } while(pLoop != m_pHead);
  } 
  return 0;
}

//*************************************************************************************************
void BBoundary::Empty() {
  while(m_pHead) {
    BBoundaryPoint *p = m_pHead;
    if(m_pHead->m_pNext != m_pHead) {
      BBoundaryPoint *pPrev = p;
      while(pPrev->m_pNext != p) {
        pPrev = pPrev->m_pNext;
      }
      pPrev->m_pNext = p->m_pNext;
      m_pHead = m_pHead->m_pNext;
    } else {
      m_pHead = 0;
    }
    delete p;
  }
}



//*************************************************************************************************
bool BBoundary::TurnsRight(BBoundaryPoint *p) {
  static BVector vUp(0, 0, -1);
  BVector v1(p->m_pNext->m_dX - p->m_dX, 
             p->m_pNext->m_dY - p->m_dY, 
             0);
  BVector v2(p->m_pNext->m_pNext->m_dX - p->m_pNext->m_dX, 
             p->m_pNext->m_pNext->m_dY - p->m_pNext->m_dY, 
             0);
  v1.ToUnitLength();
  BVector vRef = vUp.CrossProduct(v1);
  return vRef.ScalarProduct(v2) > 0.0;
}


//*************************************************************************************************
bool BBoundary::PointIsLeftOfLine(BBoundaryPoint *pStart, double dX, double dY) {
  static BVector vUp(0, 0, -1);
  BVector vTest(dX - pStart->m_dX, dY - pStart->m_dY, 0);
  BVector vTmp(pStart->m_pNext->m_dX - pStart->m_dX, 
               pStart->m_pNext->m_dY - pStart->m_dY, 
               0);
  vTmp.ToUnitLength();
  BVector vRef = vUp.CrossProduct(vTmp);
  return vRef.ScalarProduct(vTest) < 0.0;
}

//*************************************************************************************************
BBoundaryPoint *BBoundary::AddAfter(BBoundaryPoint *pAfter, BVector& rNew2D) {
  BBoundaryPoint *pOld;
  if(!(pOld = AlreadyInserted(rNew2D))) {
    BBoundaryPoint *pNew = new BBoundaryPoint;
    pNew->m_dX = rNew2D.m_dX;
    pNew->m_dY = rNew2D.m_dY;
    if(pAfter) {
      BBoundaryPoint *pTmp = pAfter->m_pNext;
      pAfter->m_pNext = pNew;
      pNew->m_pNext = pTmp;
    } else {
      m_pHead = pNew;
      m_pHead->m_pNext = m_pHead;
    }
    return pNew;
  } else {
    return pOld;
  }
}

//*************************************************************************************************
BBoundaryPoint *BBoundary::PointIsOutside(BVector& rP) {
  return PointIsOutside(rP.m_dX, rP.m_dY);
}

//*************************************************************************************************
BBoundaryPoint *BBoundary::PointIsOutside(double dX, double dY) {
  if(m_pHead) {
    BBoundaryPoint *pLoop = m_pHead;
    do {
      if(PointIsLeftOfLine(pLoop, dX, dY)) {
        return pLoop;
      }
      pLoop = pLoop->m_pNext;
    } while(pLoop != m_pHead);
  }
  return 0;
}


//*************************************************************************************************
BBoundaryPoint *BBoundary::Delete(BBoundaryPoint *p) {
  if(m_pHead) {
    if(p->m_pNext == p) {
      // Last one
      delete p;
      m_pHead = 0;
      return 0;
    }
    if(p == m_pHead) {
      m_pHead = p->m_pNext;
    }
    BBoundaryPoint *pPrev = p;
    while(pPrev->m_pNext != p) {
      pPrev = pPrev->m_pNext;
    }
    pPrev->m_pNext = p->m_pNext;
    delete p;
    return pPrev->m_pNext;
  }
  return 0;
}




//*************************************************************************************************
BPart::BPart() {
  m_nVertices = 0;
  m_nVertexStart = 0;
  m_nFaces = 0;
  m_nFaceStart = 0;
  m_nTexVertices = 0;
  m_nTexVertexStart = 0;
  m_bTextured = false;
  m_dRed = 1.0;
  m_dGreen = 1.0;
  m_dBlue = 1.0;
  m_dAmbient = 0.3;
  m_dDiffuse = 0.5;
  m_dSpecular = 0.5;
  m_dBrilliance = 1.0;
  m_bShiny = false;
  m_nTexture = 0;
  m_tmTextureMapping = WRAPAROUND;
}


//**********************************************************************
void BPart::LoadPartFromFile(string sFilename, 
                             string sSection,
                             bool bOnlyShape,
                             bool bUseMassCenter) {
  m_nVertexStart = m_pOBJData->m_nOBJPoints;
  m_nFaceStart = m_pOBJData->m_nOBJFaces;

  // Texture
  string sTmp;
  FileHelpers::GetKeyStringFromINIFile(sSection, "Texture", "INT_PREV", sTmp, sFilename);
  m_nTexture = BTextures::LoadTexture(sTmp);
  m_bTextured = (m_nTexture != BTextures::NONE);
  FileHelpers::GetKeyStringFromINIFile(sSection, "TextureMapping", "WrapAround, 1", sTmp, sFilename);
  m_dWrapFactor = 1.0;
  if(sTmp.compare("FromFile") == 0) {
    m_tmTextureMapping = BPart::FROMFILE;
  } else if(sTmp.compare("EnvMap") == 0) {
    m_tmTextureMapping = BPart::ENVMAP;
  } else {
    m_tmTextureMapping = BPart::WRAPAROUND;
    sscanf(sTmp.substr(10).c_str(), ", %lf", &m_dWrapFactor);
  }

  // Shape
  string sShapeFilename;
  FileHelpers::GetKeyStringFromINIFile(sSection, "Shape", "", sShapeFilename, sFilename);
  AppendWaveFrontOBJShape(sShapeFilename, 
                          m_nVertices, 
                          m_nFaces, 
                          bUseMassCenter, 
                          m_tmTextureMapping == BPart::FROMFILE,
                          true);

  if(bOnlyShape) {
    return;
  }

  // Color and lighting
  FileHelpers::GetKeyStringFromINIFile(sSection, "Color", "1, 1, 1", sTmp, sFilename);
  sscanf(sTmp.c_str(), "%lf, %lf, %lf", &(m_dRed), &(m_dGreen), &(m_dBlue));
  FileHelpers::GetKeyDoubleFromINIFile(sSection, "Ambient", 1.0, m_dAmbient, sFilename);
  FileHelpers::GetKeyDoubleFromINIFile(sSection, "Diffuse", 1.0, m_dDiffuse, sFilename);
  FileHelpers::GetKeyDoubleFromINIFile(sSection, "Specular", 1.0, m_dSpecular, sFilename);
  FileHelpers::GetKeyDoubleFromINIFile(sSection, "Brilliance", 1.0, m_dBrilliance, sFilename);
  FileHelpers::GetKeyStringFromINIFile(sSection, "Shiny", "false", sTmp, sFilename);
  m_bShiny = (sTmp.compare("true") == 0);
  FileHelpers::GetKeyStringFromINIFile(sSection, "Shading", "Smooth", sTmp, sFilename);
  m_shading = (sTmp.compare("Smooth") == 0) ? SMOOTH : FLAT; 
}



static char *myfgets(char *string, int n, FILE *stream, bool bPakoon3D) {
  if(!bPakoon3D) {
    return fgets(string, n, stream);
  } else {
    return 0;
  }
}

 


//**********************************************************************
void BPart::AppendWaveFrontOBJShape(string sShapeFilename, 
                                    int &rnVertices,
                                    int &rnFaces,
                                    bool bUseMassCenter,
                                    bool bReadTexCoords,
                                    bool bSplitQuads) {

  // Load in the WaveFront/Pakoon3d model file
  //
  // Relevant tags in the beginning of the lines:
  // 
  //    v   = vertex
  //    vt  = texture coordinate
  //    f   = face

  BVector      *pOBJTexCoords = new BVector[100000];
  BOBJVertex   *pOBJPoints = new BOBJVertex[100000];
  BOBJFace     *pOBJFaces  = new BOBJFace[100000];
  int          *pnSplitFaces = new int[100000];

  bool bPakoon3D = false;

  int nOriginalFaces = 0, nSplitFaces = 0;
  int nOBJPoints = 0;
  int nOBJFaces = 0;
  int nOBJTexCoords = 0;
  int i;

  bool bVertexFound = false;
  bool bVerticesRead = false;

  BVector vMassCenter(0, 0, 0);

  if(bUseMassCenter) {
    vMassCenter = m_pOBJData->m_vMassCenter;
  }

  FILE *fp;
  fp = fopen(sShapeFilename.c_str(), "r");
  if(fp) {
    char sLine[1024];
    sLine[0] = sLine[1] = '-'; // just something that's not a tag
    char *p = myfgets(sLine, 1024, fp, bPakoon3D);
    while(p) {
      if((sLine[0] == 'v') && (sLine[1] == ' ')) {
        // Add a point
        bVertexFound = true;
        double dX, dY, dZ;
        sscanf(sLine + 2, "%lf %lf %lf", &dX, &dY, &dZ);

        // Respect the geometry transformations given in the vehicle file 
        // (and the new origin calculated from the body points)
        double dMirror = 1.0;
        if(m_pOBJData->m_sRightDir.find("-") != -1) {
          dMirror = -1.0;
        }
        if(m_pOBJData->m_sRightDir.find("X") != -1) {
          pOBJPoints[nOBJPoints].m_dX = -dX * m_pOBJData->m_dScale * dMirror - vMassCenter.m_dX;
        } else if(m_pOBJData->m_sRightDir.find("Y") != -1) {
          pOBJPoints[nOBJPoints].m_dX = -dY * m_pOBJData->m_dScale * dMirror - vMassCenter.m_dX;
        } else if(m_pOBJData->m_sRightDir.find("Z") != -1) {
          pOBJPoints[nOBJPoints].m_dX = -dZ * m_pOBJData->m_dScale * dMirror - vMassCenter.m_dX;
        }

        dMirror = 1.0;
        if(m_pOBJData->m_sForwardDir.find("-") != -1) {
          dMirror = -1.0;
        }
        if(m_pOBJData->m_sForwardDir.find("X") != -1) {
          pOBJPoints[nOBJPoints].m_dY = dX * m_pOBJData->m_dScale * dMirror - vMassCenter.m_dY;
        } else if(m_pOBJData->m_sForwardDir.find("Y") != -1) {
          pOBJPoints[nOBJPoints].m_dY = dY * m_pOBJData->m_dScale * dMirror - vMassCenter.m_dY;
        } else if(m_pOBJData->m_sForwardDir.find("Z") != -1) {
          pOBJPoints[nOBJPoints].m_dY = dZ * m_pOBJData->m_dScale * dMirror - vMassCenter.m_dY;
        }

        dMirror = 1.0;
        if(m_pOBJData->m_sDownDir.find("-") != -1) {
          dMirror = -1.0;
        }
        if(m_pOBJData->m_sDownDir.find("X") != -1) {
          pOBJPoints[nOBJPoints].m_dZ = dX * m_pOBJData->m_dScale * dMirror - vMassCenter.m_dZ;
        } else if(m_pOBJData->m_sDownDir.find("Y") != -1) {
          pOBJPoints[nOBJPoints].m_dZ = dY * m_pOBJData->m_dScale * dMirror - vMassCenter.m_dZ;
        } else if(m_pOBJData->m_sDownDir.find("Z") != -1) {
          pOBJPoints[nOBJPoints].m_dZ = dZ * m_pOBJData->m_dScale * dMirror - vMassCenter.m_dZ;
        }
        ++nOBJPoints;
      } else {
        if(bVertexFound) {
          bVerticesRead = true;
        }
      }

      if(bVerticesRead) {
        // Look for faces and texture coordinates also
        if((sLine[0] == 'f') && (sLine[1] == ' ')) {
          // Add a face
          int n1, n2, n3, n4;
          int nT1, nT2, nT3, nT4;

          // vertex indices are separated by a space. Read in max 4 indices 
          // (divide them into two triangles)

          char *p = sLine + 2;
          if(string(p).find("/") != -1) {
            sscanf(p, "%d/%d", &n1, &nT1);
          } else {
            sscanf(p, "%d", &n1);
          }
          while(*p && (*p != ' ')) {
            ++p;
          }
          if(*p) {
            ++p; // Skip space
            if(string(p).find("/") != -1) {
              sscanf(p, "%d/%d", &n2, &nT2);
            } else {
              sscanf(p, "%d", &n1);
            }
            while(*p && (*p != ' ')) {
              ++p;
            }
            if(*p) {
              ++p; // Skip space
              if(string(p).find("/") != -1) {
                sscanf(p, "%d/%d", &n3, &nT3);
              } else {
                sscanf(p, "%d", &n1);
              }
              while(*p && (*p != ' ')) {
                ++p;
              }
              if(*p) {
                ++p; // Skip space
                if(string(p).find("/") != -1) {
                  sscanf(p, "%d/%d", &n4, &nT4);
                } else {
                  sscanf(p, "%d", &n1);
                }
        
                if(bSplitQuads) {
                  pOBJFaces[nOBJFaces].m_nV1 = n1 - 1;
                  pOBJFaces[nOBJFaces].m_nV2 = n2 - 1;
                  pOBJFaces[nOBJFaces].m_nV3 = n4 - 1;
                  pOBJFaces[nOBJFaces].m_nVertices = 3;
                  pOBJFaces[nOBJFaces].m_nTexCoords[0] = nT4 - 1;
                  pOBJFaces[nOBJFaces].m_nTexCoords[1] = nT3 - 1;
                  pOBJFaces[nOBJFaces].m_nTexCoords[2] = nT1 - 1;
                  ++nOBJFaces;
                  pOBJFaces[nOBJFaces].m_nV1 = n3 - 1;
                  pOBJFaces[nOBJFaces].m_nV2 = n4 - 1;
                  pOBJFaces[nOBJFaces].m_nV3 = n2 - 1;
                  pOBJFaces[nOBJFaces].m_nVertices = 3;
                  pOBJFaces[nOBJFaces].m_nTexCoords[0] = nT2 - 1;
                  pOBJFaces[nOBJFaces].m_nTexCoords[1] = nT1 - 1;
                  pOBJFaces[nOBJFaces].m_nTexCoords[2] = nT3 - 1;
                  ++nOBJFaces;

                  // Save info about the split for FromFile texture mapping
                  pnSplitFaces[nSplitFaces] = nOriginalFaces;
                  ++nSplitFaces;

                } else {
                  pOBJFaces[nOBJFaces].m_nV1 = n1 - 1;
                  pOBJFaces[nOBJFaces].m_nV2 = n2 - 1;
                  pOBJFaces[nOBJFaces].m_nV3 = n3 - 1;
                  pOBJFaces[nOBJFaces].m_nV4 = n4 - 1;
                  pOBJFaces[nOBJFaces].m_nVertices = 4;
                  ++nOBJFaces;
                }
                ++nOriginalFaces;
              } else {
                pOBJFaces[nOBJFaces].m_nV1 = n1 - 1;
                pOBJFaces[nOBJFaces].m_nV2 = n2 - 1;
                pOBJFaces[nOBJFaces].m_nV3 = n3 - 1;
                pOBJFaces[nOBJFaces].m_nVertices = 3;
                pOBJFaces[nOBJFaces].m_nTexCoords[0] = nT3 - 1;
                pOBJFaces[nOBJFaces].m_nTexCoords[1] = nT2 - 1;
                pOBJFaces[nOBJFaces].m_nTexCoords[2] = nT1 - 1;
                ++nOBJFaces;
                ++nOriginalFaces;
              }
            }
          }
        } else if(bReadTexCoords && (sLine[0] == 'v') && (sLine[1] == 't')) {
          // Store texture coordinate
          char *p = sLine + 2;
          sscanf(p, "%lf %lf", &(pOBJTexCoords[nOBJTexCoords].m_dX), &(pOBJTexCoords[nOBJTexCoords].m_dY));
          ++nOBJTexCoords;
        }
      }

      sLine[0] = sLine[1] = '-'; // just something that's not a tag
      p = myfgets(sLine, 1024, fp, bPakoon3D);
    }
    fclose(fp);
  } else {
    string sErr = "Cannot open file " + sShapeFilename + " for reading";
    BGame::MyAfxMessageBox(sErr);
  }

  // Remap texture coordinates to face corners
  if(bReadTexCoords) {
    for(i = 0; i < nOBJFaces; ++i) {
      // Handle normal triangle
      pOBJFaces[i].m_vTexCoords[0] = pOBJTexCoords[pOBJFaces[i].m_nTexCoords[0]];
      pOBJFaces[i].m_vTexCoords[1] = pOBJTexCoords[pOBJFaces[i].m_nTexCoords[1]];
      pOBJFaces[i].m_vTexCoords[2] = pOBJTexCoords[pOBJFaces[i].m_nTexCoords[2]];
    }
  }

  // Transfer the read vertices and faces to the global vertex/face arrays.
  for(i = 0; i < nOBJPoints; ++i) {
    m_pOBJData->m_pvOBJPoints[m_pOBJData->m_nOBJPoints + i] = pOBJPoints[i];
  }
  for(i = 0; i < nOBJFaces; ++i) {
    m_pOBJData->m_pOBJFaces[m_pOBJData->m_nOBJFaces + i].m_nV1 = pOBJFaces[i].m_nV1 + m_pOBJData->m_nOBJPoints;
    m_pOBJData->m_pOBJFaces[m_pOBJData->m_nOBJFaces + i].m_nV2 = pOBJFaces[i].m_nV2 + m_pOBJData->m_nOBJPoints;
    m_pOBJData->m_pOBJFaces[m_pOBJData->m_nOBJFaces + i].m_nV3 = pOBJFaces[i].m_nV3 + m_pOBJData->m_nOBJPoints;
    m_pOBJData->m_pOBJFaces[m_pOBJData->m_nOBJFaces + i].m_nV4 = pOBJFaces[i].m_nV4 + m_pOBJData->m_nOBJPoints;
    m_pOBJData->m_pOBJFaces[m_pOBJData->m_nOBJFaces + i].m_nVertices = pOBJFaces[i].m_nVertices;
    m_pOBJData->m_pOBJFaces[m_pOBJData->m_nOBJFaces + i].m_vTexCoords[0] = pOBJFaces[i].m_vTexCoords[0];
    m_pOBJData->m_pOBJFaces[m_pOBJData->m_nOBJFaces + i].m_vTexCoords[1] = pOBJFaces[i].m_vTexCoords[1];
    m_pOBJData->m_pOBJFaces[m_pOBJData->m_nOBJFaces + i].m_vTexCoords[2] = pOBJFaces[i].m_vTexCoords[2];
  }
  m_pOBJData->m_nOBJPoints += nOBJPoints;
  m_pOBJData->m_nOBJFaces += nOBJFaces;

  rnVertices = nOBJPoints;
  rnFaces = nOBJFaces;

  delete [] pOBJPoints;
  delete [] pOBJFaces;
  delete [] pnSplitFaces;
  delete [] pOBJTexCoords;
}

extern double Random(double);

//**********************************************************************

void BPart::RenderPart(int nReset) {
  static bool     bTextured = false;
  static int      nTexture = -1;
  static int      tmMapping = -1;
  static int      nTextureUnit = -1;
  static TShading shading = NONE;
  if(nReset == 1) {
    // Reset rendering
    bTextured = false;
    nTexture = -1;
    nTextureUnit = -1;
    tmMapping = -1;
    shading = SMOOTH;
    return;
  } else if(nReset == 2) {
    // End rendering
    OpenGLHelpers::SwitchToTexture(0);
    glDisable(GL_TEXTURE_GEN_S);
	  glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_2D);
    return;
  }

  // shiny
  if(m_bShiny) {
    nTextureUnit = 1;
    OpenGLHelpers::SwitchToTexture(1);
    BTextures::Use(BTextures::ENVMAP_SHINY);
    glEnable(GL_TEXTURE_GEN_S);
	  glEnable(GL_TEXTURE_GEN_T);
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
  }

  if(m_bTextured && ((m_bTextured != bTextured) || (nTextureUnit != 0))) {
    nTextureUnit = 0;
    OpenGLHelpers::SwitchToTexture(0);
  }
  if(!m_bTextured && (m_bTextured != bTextured)) {
    glDisable(GL_TEXTURE_2D);
  }
  bTextured = m_bTextured;
  if(m_bTextured && (m_nTexture != nTexture)) {
    BTextures::Use(m_nTexture);
    nTexture = m_nTexture;
  }
  if(m_bTextured && ((m_tmTextureMapping == ENVMAP) && (tmMapping != m_tmTextureMapping))) {
    glEnable(GL_TEXTURE_GEN_S);
	  glEnable(GL_TEXTURE_GEN_T);
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    tmMapping = m_tmTextureMapping;
  }
  bTextured = m_bTextured;

  if(shading != m_shading) {
    if(m_shading == SMOOTH) {
      glShadeModel(GL_SMOOTH);
    } else {
      glShadeModel(GL_FLAT);
    }
  }
  shading = m_shading;

  glBegin(GL_TRIANGLES);
  for(int nFace = 0; nFace < m_nFaces; ++nFace) {

    BOBJVertex v1, v2, v3;
    v1 = m_pOBJData->m_pvOBJPoints[m_pOBJData->m_pOBJFaces[m_nFaceStart + nFace].m_nV1];
    v2 = m_pOBJData->m_pvOBJPoints[m_pOBJData->m_pOBJFaces[m_nFaceStart + nFace].m_nV2];
    v3 = m_pOBJData->m_pvOBJPoints[m_pOBJData->m_pOBJFaces[m_nFaceStart + nFace].m_nV3];
    double dX = v1.m_dX;
    double dY = v1.m_dY;
    BVector v1to2 = v2 - v1;
    double d1to2Len = v1to2.Length();
    v1to2.ToUnitLength();
    BVector v1to3 = v3 - v1;
    BVector vRight = v1to2.CrossProduct(m_pOBJData->m_pOBJFaces[m_nFaceStart + nFace].m_vNormal);
    vRight.ToUnitLength();
    double d3X = v1to3.ScalarProduct(vRight);
    double d3Y = v1to3.ScalarProduct(v1to2);

    // **************** VERTEX # 1 **************** 
    if(m_tmTextureMapping == WRAPAROUND) {
      // Create WrapAround texture coordinates
      OpenGLHelpers::SetTexCoord(dX, dY);
    }
    if(m_tmTextureMapping == FROMFILE) {
      OpenGLHelpers::SetTexCoord(m_pOBJData->m_pOBJFaces[m_nFaceStart + nFace].m_vTexCoords[0].m_dX, 
                                 m_pOBJData->m_pOBJFaces[m_nFaceStart + nFace].m_vTexCoords[0].m_dY);
    }

    if(m_shading == SMOOTH) {
      glNormal3f(v1.m_vNormal.m_dX, v1.m_vNormal.m_dY, v1.m_vNormal.m_dZ);
    } else {
      BVector vNormal = m_pOBJData->m_pOBJFaces[m_nFaceStart + nFace].m_vNormal;
      glNormal3f(vNormal.m_dX, vNormal.m_dY, vNormal.m_dZ);
    }
    glVertex3f(v1.m_dX, v1.m_dY, v1.m_dZ);


    // **************** VERTEX # 2 **************** 
    if(m_tmTextureMapping == WRAPAROUND) {
      // Create WrapAround texture coordinates
      OpenGLHelpers::SetTexCoord(dX, dY + d1to2Len * m_dWrapFactor);
    }
    if(m_tmTextureMapping == FROMFILE) {
      OpenGLHelpers::SetTexCoord(m_pOBJData->m_pOBJFaces[m_nFaceStart + nFace].m_vTexCoords[1].m_dX, 
                                 m_pOBJData->m_pOBJFaces[m_nFaceStart + nFace].m_vTexCoords[1].m_dY);
    }
    if(m_shading == SMOOTH) {
      glNormal3f(v2.m_vNormal.m_dX, v2.m_vNormal.m_dY, v2.m_vNormal.m_dZ);
    }
    glVertex3f(v2.m_dX, v2.m_dY, v2.m_dZ);
  

    // **************** VERTEX # 3 **************** 
    if(m_tmTextureMapping == WRAPAROUND) {
      // Create WrapAround texture coordinates
      OpenGLHelpers::SetTexCoord(dX + d3X * m_dWrapFactor, dY + d3Y * m_dWrapFactor);
    }
    if(m_tmTextureMapping == FROMFILE) {
      OpenGLHelpers::SetTexCoord(m_pOBJData->m_pOBJFaces[m_nFaceStart + nFace].m_vTexCoords[2].m_dX, 
                                 m_pOBJData->m_pOBJFaces[m_nFaceStart + nFace].m_vTexCoords[2].m_dY);
    }
    if(m_shading == SMOOTH) {
      glNormal3f(v3.m_vNormal.m_dX, v3.m_vNormal.m_dY, v3.m_vNormal.m_dZ);
    }
    glVertex3f(v3.m_dX, v3.m_dY, v3.m_dZ);

  }
  glEnd();

  //glDrawElements(GL_TRIANGLES, 
  //               m_nFaces * 3, 
  //               GL_UNSIGNED_INT, 
  //               m_pOBJData->m_pnTriangleElemList + m_nFaceStart * 3);

  // shiny
  if(m_bShiny) {
    nTextureUnit = 1;
    OpenGLHelpers::SwitchToTexture(1);
    glDisable(GL_TEXTURE_GEN_S);
	  glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_2D);
  }
}


//**********************************************************************

void BPart::SetPartColor(GLfloat fAlpha) {
  GLfloat fLight[4];

  fLight[0] = GLfloat(m_dRed   * m_dAmbient);
  fLight[1] = GLfloat(m_dGreen * m_dAmbient);
  fLight[2] = GLfloat(m_dBlue  * m_dAmbient);
  fLight[3] = fAlpha;
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, fLight);

  fLight[0] = GLfloat(m_dRed   * m_dDiffuse);
  fLight[1] = GLfloat(m_dGreen * m_dDiffuse);
  fLight[2] = GLfloat(m_dBlue  * m_dDiffuse);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, fLight);

  fLight[0] = GLfloat(m_dRed   * m_dSpecular);
  fLight[1] = GLfloat(m_dGreen * m_dSpecular);
  fLight[2] = GLfloat(m_dBlue  * m_dSpecular);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, fLight);

  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, GLfloat(m_dBrilliance));
}


//**********************************************************************
BOBJData::BOBJData() {
  m_nOBJPoints = 0;
  m_nOBJFaces = 0;
  m_pvOBJPoints = 0;
  m_pOBJFaces = 0;
  m_sRightDir = "+X";
  m_sForwardDir = "+Y";
  m_sDownDir = "+Z";
  m_dScale = 1.0;
  m_vMassCenter.Set(0, 0, 0);
}

//**********************************************************************
BOBJData::~BOBJData() {
  Free();
}

//**********************************************************************
void BOBJData::Init() {
  Free();
  m_nOBJPoints = 0;
  m_nOBJFaces = 0;
  m_pvOBJPoints = new BOBJVertex[100000];
  m_pOBJFaces = new BOBJFace[100000];
}

//**********************************************************************
void BOBJData::Free() {
  if(m_pvOBJPoints) {
    delete [] m_pvOBJPoints;
    m_pvOBJPoints = 0;
  }
  if(m_pOBJFaces) {
    delete [] m_pOBJFaces;
    m_pOBJFaces = 0;
  }
  m_nOBJPoints = 0;
  m_nOBJFaces = 0;
}


extern double Random(double);

//**********************************************************************

void BOBJData::PrepareWaveFrontModel() {
  // Calculate normals for vertexes
  // Create vertex etc. arrays for drawing the model

  int i;

  for(i = 0; i < m_nOBJFaces; ++i) {

    double d1to2Len = (m_pvOBJPoints[m_pOBJFaces[i].m_nV2] - m_pvOBJPoints[m_pOBJFaces[i].m_nV1]).Length();
    double d2to3Len = (m_pvOBJPoints[m_pOBJFaces[i].m_nV2] - m_pvOBJPoints[m_pOBJFaces[i].m_nV3]).Length();
    double d1to3Len = (m_pvOBJPoints[m_pOBJFaces[i].m_nV3] - m_pvOBJPoints[m_pOBJFaces[i].m_nV1]).Length();

    // Normal
    BVector v1, v2;
    v1 = m_pvOBJPoints[m_pOBJFaces[i].m_nV2] - m_pvOBJPoints[m_pOBJFaces[i].m_nV1];
    v1.ToUnitLength();
    v2 = m_pvOBJPoints[m_pOBJFaces[i].m_nV3] - m_pvOBJPoints[m_pOBJFaces[i].m_nV1];
    v2.ToUnitLength();
    m_pOBJFaces[i].m_vNormal = v2.CrossProduct(v1);
    m_pOBJFaces[i].m_vNormal.ToUnitLength();
  }

  // Calculate vertex normals
  for(i = 0; i < m_nOBJPoints; ++i) {
    m_pvOBJPoints[i].m_vNormal.Set(0, 0, 0);
  }

  for(i = 0; i < m_nOBJFaces; ++i) {
    m_pvOBJPoints[m_pOBJFaces[i].m_nV1].m_vNormal += m_pOBJFaces[i].m_vNormal;
    m_pvOBJPoints[m_pOBJFaces[i].m_nV2].m_vNormal += m_pOBJFaces[i].m_vNormal;
    m_pvOBJPoints[m_pOBJFaces[i].m_nV3].m_vNormal += m_pOBJFaces[i].m_vNormal;
  }

  for(i = 0; i < m_nOBJPoints; ++i) {
    m_pvOBJPoints[i].m_vNormal.ToUnitLength();
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
