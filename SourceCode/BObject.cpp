//
// Object
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#include "stdafx.h"
#include "BObject.h"
#include "BTextures.h"
#include "OpenGLHelpers.h"
#include "FileIOHelpers.h"
#include "BGame.h"
#include "HeightMap.h"

//*****************************************************************************
void BFace::SetPoints(int nPoints, BVector *pvPoints) {
  ASSERT(nPoints >= 3 && nPoints <= 5);
  for(int i = 0; i < min(nPoints, 5); ++i) {
    m_vPoint[i] = pvPoints[i];
  }
  m_nPoints = min(nPoints, 5);
}

//*****************************************************************************
void BFace::CalculateNormal() {
  // Calculate normal for the face
  BVector v1, v2;
  v1 = m_vPoint[1] - m_vPoint[0];
  v2 = m_vPoint[2] - m_vPoint[1];
  m_vNormal = v1.CrossProduct(v2);
  m_vNormal.ToUnitLength();
}


//*****************************************************************************
BObjectPart::~BObjectPart() {
  if(m_pFace) {
    delete [] m_pFace;
  }  
}

//*****************************************************************************
BObject::BObject() {
  m_pOBJData = 0;
  m_nParts = 0;
  m_pPart = 0;
  m_nCollDetParts = 0;
  m_pCollDetPart = 0;
  m_sName = _T("Object");
  m_type = USER_DEF;
  m_collisionDetection = ACCURATE;
  m_vLocation.Set(0, 0, 0);
  m_vCenter.Set(0, 0, 0);
  m_dScale = 1.0;
  m_dScale2 = 1.0;
  m_dZRotation = 0.0;
  m_dRadius = 0;
  m_dRed = 1.0;
  m_dGreen = 1.0;
  m_dBlue = 1.0;
  m_dFriction = 0.5;
  m_nDL = -1;
  m_nDLShadow = -1;
  m_bHasShadow = true;
  m_bDLIsValid = false;
  m_bShadowPolygonOK = false;
  m_vCenterOnGround.Set(0, 0, 0);
  m_dActiveRadius = 1.0;
}

//*****************************************************************************
BObject::~BObject() {
  if(m_pPart) {
    delete [] m_pPart;
  }
}


double AngleBetweenVectorsAlongZ(BVector &v1, BVector &v2, double dSign) {
  BVector vZ1 = v1;
  vZ1.m_dZ = 0.0;
  vZ1.ToUnitLength();
  BVector vZ2 = v2;
  vZ2.m_dX = 0.0;
  vZ2.ToUnitLength();
  double dAngle = acos(vZ1.ScalarProduct(vZ2));
  if(dSign < 0.0) {
    dAngle = -dAngle;
  }
  return dAngle;
}


double LengthAlongZ(BVector &rv) {
  return sqrt(rv.m_dX * rv.m_dX + rv.m_dY * rv.m_dY);
}

void RotateAroundZ(BVector &rVector, double dAngle) {
  double dA1, dL1;
  dA1 = AngleBetweenVectorsAlongZ(rVector, BVector(0, 1, 0), rVector.m_dX);
  dL1 = LengthAlongZ(rVector);
  rVector.m_dX = sin(dA1 + dAngle) * dL1;
  rVector.m_dY = cos(dA1 + dAngle) * dL1;
}


//*****************************************************************************
void BObject::Setup() {
  // Calculate center and place points according to origin, scale and rotation
  int nPoints = 0;
  BVector vCenter(0, 0, 0);
  int nFace, nPart;
  int i;

  for(nPart = 0; nPart < m_nCollDetParts; ++nPart) {
    for(nFace = 0; nFace < m_pCollDetPart[nPart].m_nFaces; ++nFace) {
      for(i = 0; i < m_pCollDetPart[nPart].m_pFace[nFace].m_nPoints; ++i) {
        BVector v = m_pCollDetPart[nPart].m_pFace[nFace].m_vPoint[i];
        double dLen = v.Length();
        if((v.m_dX != 0.0) || (v.m_dY != 0.0)) {
          RotateAroundZ(v, m_dZRotation / 180.0 * 3.141592654);
        }
        m_pCollDetPart[nPart].m_pFace[nFace].m_vPoint[i] = v;
        ++nPoints;
        vCenter += m_pCollDetPart[nPart].m_pFace[nFace].m_vPoint[i];
      }
      m_pCollDetPart[nPart].m_pFace[nFace].CalculateNormal();
    }
  }
  m_vCenter = vCenter * (1.0 / double(nPoints));
  m_dRadius = 0.1;
  for(nPart = 0; nPart < m_nCollDetParts; ++nPart) {
    for(nFace = 0; nFace < m_pCollDetPart[nPart].m_nFaces; ++nFace) {
      for(i = 0; i < m_pCollDetPart[nPart].m_pFace[nFace].m_nPoints; ++i) {
        if((m_vCenter - m_pCollDetPart[nPart].m_pFace[nFace].m_vPoint[i]).Length() > m_dRadius) {
          m_dRadius = (m_vCenter - m_pCollDetPart[nPart].m_pFace[nFace].m_vPoint[i]).Length();
        }
      }
    }
  }
  m_vCenter += m_vLocation;

  if(m_collisionDetection == ACCURATE) {
    CreateXYBoundary();
  } else if(m_collisionDetection == BOUNDING_SPHERE) {
    // find out the radius from the visualization points.
    nPoints = 0;
    vCenter.Set(0, 0, 0);

    for(nPart = 0; nPart < m_nParts; ++nPart) {
      for(i = 0; i < m_pPart[nPart].m_nVertices; ++i) {
        ++nPoints;
        vCenter += m_pPart[nPart].m_pOBJData->m_pvOBJPoints[m_pPart[nPart].m_nVertexStart + i];
      }
    }

    m_vCenter = vCenter * (1.0 / double(nPoints));
    m_dRadius = 0.1;
    for(nPart = 0; nPart < m_nParts; ++nPart) {
      for(i = 0; i < m_pPart[nPart].m_nVertices; ++i) {
        if((m_vCenter - m_pPart[nPart].m_pOBJData->m_pvOBJPoints[m_pPart[nPart].m_nVertexStart + i]).Length() > m_dRadius) {
          m_dRadius = (m_vCenter - m_pPart[nPart].m_pOBJData->m_pvOBJPoints[m_pPart[nPart].m_nVertexStart + i]).Length();
        }
      }
    }
    m_vCenter += m_vLocation;
  }

  // Create shadow polygon
  if(!m_bShadowPolygonOK && m_bHasShadow) {
    CreateShadowPolygon();
  }

  // Set center on ground
  BVector vTmp = m_vCenter;
  vTmp.m_dZ = 0;
  double dTmp = 0;
  BVector vInternalOffset = BGame::GetSimulation()->GetTerrain()->m_vOffset;
  double dNewZ = -HeightMap::CalcHeightAt(vInternalOffset.m_dX + vTmp.m_dX, 
                                          vInternalOffset.m_dY + vTmp.m_dY, 
                                          dTmp, 
                                          BGame::GetSimulation()->GetScene()->m_terrainStyle,
                                          8,
                                          vInternalOffset.m_dY);

  m_vCenterOnGround.Set(m_vCenter.m_dX, m_vCenter.m_dY, dNewZ);
}

//*****************************************************************************
double BObject::PointIsInsideObject(BVector& rvPoint, 
                                    BVector& rvNormal, 
                                    double& rdGroundFriction, 
                                    double& rdBaseDepth) {

  // NOTE: BOUNDING BOX DETECTION IS NOT YET SUPPORTED!!!

  // First check whether point is within object radius.
  BVector vFromCenter = rvPoint - m_vCenter;
  if(vFromCenter.Length() > m_dRadius) {
    return -1.0;
  }
  if(m_collisionDetection == TCollDet::BOUNDING_SPHERE) {
    rvNormal = vFromCenter;
    rvNormal.ToUnitLength();
    rdBaseDepth = m_dRadius - vFromCenter.Length();
    return rdBaseDepth;
  }

  // Point is inside object if it's on the inside of each face of some part.
  // If inside, return the normal of the part & face whose's surface is closest to
  // the point

  int    nHitFace = 0, nHitPart = 0;
  double dMinDepth = 9999.0;

  bool bInsideObject = false;
  for(int nPart = 0; nPart < m_nCollDetParts; ++nPart) {
    bool bInsidePart = true;
    for(int nFace = 0; nFace < m_pCollDetPart[nPart].m_nFaces; ++nFace) {
      BVector vTmp;
      vTmp = rvPoint - (m_pCollDetPart[nPart].m_pFace[nFace].m_vPoint[0] + m_vLocation);
      double dDepth = -m_pCollDetPart[nPart].m_pFace[nFace].m_vNormal.ScalarProduct(vTmp);
      if(dDepth > 0.0) {
        if(dDepth < dMinDepth) {
          nHitFace = nFace;
          nHitPart = nPart;
          dMinDepth = dDepth;
        }
      } else {
        bInsidePart = false;
        break; // Not inside object
      }
    }
    if(bInsidePart) {
      bInsideObject = true;
      break;
    }
  }
  if(bInsideObject) {
    rvNormal = m_pCollDetPart[nHitPart].m_pFace[nHitFace].m_vNormal;
    rdGroundFriction = m_pCollDetPart[nHitPart].m_dFriction;
    rdBaseDepth = dMinDepth;
    return dMinDepth;
  } else {
    return -1.0;
  }
}


//*****************************************************************************
void BObject::CreateXYBoundary() {
  m_boundary.Empty();
  BVector points[10000];
  int    nXMin, nXMax, nYMin, nYMax;
  double dXMin = 99999.9, dXMax = -99999.9, dYMin = 99999.9, dYMax = -99999.9;
  int nPoints = 0;
  // Find all object points
  int nFace;
  int i;
  for(int nPart = 0; nPart < m_nCollDetParts; ++nPart) {
    for(nFace = 0; nFace < m_pCollDetPart[nPart].m_nFaces; ++nFace) {
      for(i = 0; i < m_pCollDetPart[nPart].m_pFace[nFace].m_nPoints; ++i) {

        BVector v = m_pCollDetPart[nPart].m_pFace[nFace].m_vPoint[i] + m_vCenter;
        points[nPoints] = v;

        if(points[nPoints].m_dX < dXMin) {
          nXMin = nPoints;
          dXMin = points[nPoints].m_dX;
        }

        if(points[nPoints].m_dX > dXMax) {
          nXMax = nPoints;
          dXMax = points[nPoints].m_dX;
        }

        if(points[nPoints].m_dY < dYMin) {
          nYMin = nPoints;
          dYMin = points[nPoints].m_dY;
        }

        if(points[nPoints].m_dY > dYMax) {
          nYMax = nPoints;
          dYMax = points[nPoints].m_dY;
        }

        ++nPoints;
      }
    }
  }

  BBoundaryPoint *pos;
  pos = m_boundary.AddAfter(m_boundary.Head(), points[nXMin]);
  pos = m_boundary.AddAfter(pos, points[nYMax]);
  pos = m_boundary.AddAfter(pos, points[nXMax]);
  pos = m_boundary.AddAfter(pos, points[nYMin]);

  for(i = 0; i < nPoints; ++i) {
    if(pos = m_boundary.PointIsOutside(points[i])) { // pos = start point of line outside which p is

      BBoundaryPoint *pNew = m_boundary.AddAfter(pos, points[i]);

      // Delete all points that violate the "always turn right" rule
      BBoundaryPoint *pLoop = pNew;
      do {
        // Test if *(pLoop->m_pNext) violates
        if(!m_boundary.TurnsRight(pLoop)) {
          m_boundary.Delete(pLoop->m_pNext);
        } else {
          pLoop = pLoop->m_pNext;
        }
      } while(pLoop && (pLoop != pNew));

      //BBoundaryPoint *pNew = m_boundary.AddAfter(pos, points[i]);
      //BBoundaryPoint *pLoop = pNew->m_pNext;
      //while(pLoop && (pLoop != pNew)) {
      //  if(m_boundary.PointIsLeftOfLine(pos, pLoop->m_dX, pLoop->m_dY)) {
      //    pLoop = m_boundary.Delete(pLoop);
      //  } else {
      //    pLoop = pLoop->m_pNext;
      //  }
      //}
    }
  }
}


//*****************************************************************************
void BObject::CreateShadowPolygon() {

  if(!m_nParts) {
    m_bShadowPolygonOK = false;
    return;
  }

  m_shadow.Empty();
  BVector points[10000];
  int    nXMin, nXMax, nYMin, nYMax;
  double dXMin = 99999.9, dXMax = -99999.9, dYMin = 99999.9, dYMax = -99999.9;
  int nPoints = 0;
  // Find all object points
  int i;
  for(int nPart = 0; nPart < m_nParts; ++nPart) {
    for(i = 0; i < m_pPart[nPart].m_nVertices; ++i) {
      points[nPoints].m_dX = m_pPart[nPart].m_pOBJData->m_pvOBJPoints[m_pPart[nPart].m_nVertexStart + i].m_dX;
      points[nPoints].m_dY = m_pPart[nPart].m_pOBJData->m_pvOBJPoints[m_pPart[nPart].m_nVertexStart + i].m_dY;

      if(points[nPoints].m_dX < dXMin) {
        nXMin = nPoints;
        dXMin = points[nPoints].m_dX;
      }

      if(points[nPoints].m_dX > dXMax) {
        nXMax = nPoints;
        dXMax = points[nPoints].m_dX;
      }

      if(points[nPoints].m_dY < dYMin) {
        nYMin = nPoints;
        dYMin = points[nPoints].m_dY;
      }

      if(points[nPoints].m_dY > dYMax) {
        nYMax = nPoints;
        dYMax = points[nPoints].m_dY;
      }

      ++nPoints;
    }
  }

  BBoundaryPoint *pos;
  pos = m_shadow.AddAfter(m_shadow.Head(), points[nXMin]);
  pos = m_shadow.AddAfter(pos, points[nYMax]);
  pos = m_shadow.AddAfter(pos, points[nXMax]);
  pos = m_shadow.AddAfter(pos, points[nYMin]);

  for(i = 0; i < nPoints; ++i) {
    if(pos = m_shadow.PointIsOutside(points[i])) { // pos = start point of line outside which p is

      BBoundaryPoint *pNew = m_shadow.AddAfter(pos, points[i]);

      // Delete all points that violate the "always turn right" rule
      BBoundaryPoint *pLoop = pNew;
      do {
        // Test if *(pLoop->m_pNext) violates
        if(!m_shadow.TurnsRight(pLoop)) {
          m_shadow.Delete(pLoop->m_pNext);
        } else {
          pLoop = pLoop->m_pNext;
        }
      } while(pLoop && (pLoop != pNew));
    }
  }
  m_bShadowPolygonOK = true;
}





//*****************************************************************************
void BObject::PreProcessVisualization(bool bOnlyShadow) {
  if(m_nDL == -1) { // create new list only if not already created. Otherwise use the existing list.
    m_nDL = glGenLists(1);
  }
  if(m_nDLShadow == -1) { // create new list only if not already created. Otherwise use the existing list.
    m_nDLShadow = glGenLists(1);
  }

  // Render the triangles in the block into the display list

  // (888 NOT READY, USE ACTUAL TEXTURE MAPPING!)

  // Render actual object
  if(!bOnlyShadow) {

    // First check if the same object already exists.
    // If so, reuse it's display list.
    bool bDLDone = false;
    BScene *pScene = BGame::GetSimulation()->GetScene();
    int i;
    for(i = 0; i < pScene->m_nObjects; ++i) {
      if((&(pScene->m_pObjects[i]) != this) && 
         (pScene->m_pObjects[i].m_sObjectFilename.CompareNoCase(m_sObjectFilename) == 0) &&
         pScene->m_pObjects[i].m_bDLIsValid) {
        m_nDL = pScene->m_pObjects[i].m_nDL;
        bDLDone = true;
        break;
      }
    }

    if(!bDLDone) {
      glNewList(m_nDL, GL_COMPILE);
      if(m_nParts) {
        int nPart;
        m_pPart[0].RenderPart(1);
        for(nPart = 0; nPart < m_nParts; ++nPart) {
          m_pPart[nPart].SetPartColor();
          m_pPart[nPart].RenderPart(0);
        }
        m_pPart[0].RenderPart(2);
      }

      glEndList();
      m_bDLIsValid = true;
    }
  }

  // Render shadow
  glNewList(m_nDLShadow, GL_COMPILE);

  if(m_bHasShadow) {

    // First setup heights
    BBoundaryPoint *p = m_shadow.Head();
    do {
      double dTmp = 0;
      BVector vInternalOffset = BGame::GetSimulation()->GetTerrain()->m_vOffset;
      BVector vRotated = *p;
      RotateAroundZ(vRotated, m_dZRotation);
      vRotated = vRotated * m_dScale2;
      double dHeight = HeightMap::CalcHeightAt(vInternalOffset.m_dX + vRotated.m_dX + m_vLocation.m_dX, 
                                               vInternalOffset.m_dY + vRotated.m_dY + m_vLocation.m_dY, 
                                               dTmp, 
                                               BGame::GetSimulation()->GetScene()->m_terrainStyle,
                                               8,
                                               vInternalOffset.m_dY);
      p->m_dZ = -dHeight;
      p = p->m_pNext;
    } while(p != m_shadow.Head());

    // Draw shadow (first base convex polygon)
    glColor4d(0, 0, 0, 0.3);
    glBegin(GL_POLYGON);
    glNormal3f(0, 0, -1.0);

    p = m_shadow.Head();
    do {
      glVertex3f(p->m_dX, p->m_dY, p->m_dZ);
      p = p->m_pNext;
    } while(p != m_shadow.Head());
    glEnd();

    // Then the penumbras
    glBegin(GL_TRIANGLES);
    glNormal3f(0, 0, -1.0);

    p = m_shadow.Head();
    BVector vUp(0, 0, -1);

    do {
      // One segment
      BVector vToNext = *(p->m_pNext) - *p;
      vToNext.ToUnitLength();
      BVector vLeft = vToNext.CrossProduct(vUp);
      vLeft.ToUnitLength();
      glColor4d(0, 0, 0, 0.3);
      glVertex3f(p->m_dX, p->m_dY, p->m_dZ);
      glColor4d(0, 0, 0, 0.0);
      glVertex3f(p->m_dX + vLeft.m_dX, p->m_dY + vLeft.m_dY, p->m_dZ);
      glVertex3f(p->m_pNext->m_dX + vLeft.m_dX, p->m_pNext->m_dY + vLeft.m_dY, p->m_pNext->m_dZ);
      glColor4d(0, 0, 0, 0.3);
      glVertex3f(p->m_dX, p->m_dY, p->m_dZ);
      glColor4d(0, 0, 0, 0.0);
      glVertex3f(p->m_pNext->m_dX + vLeft.m_dX, p->m_pNext->m_dY + vLeft.m_dY, p->m_pNext->m_dZ);
      glColor4d(0, 0, 0, 0.3);
      glVertex3f(p->m_pNext->m_dX, p->m_pNext->m_dY, p->m_pNext->m_dZ);

      // Connector to next segment
      glColor4d(0, 0, 0, 0.3);
      glVertex3f(p->m_pNext->m_dX, p->m_pNext->m_dY, p->m_pNext->m_dZ);
      glColor4d(0, 0, 0, 0.0);
      glVertex3f(p->m_pNext->m_dX + vLeft.m_dX, p->m_pNext->m_dY + vLeft.m_dY, p->m_pNext->m_dZ);

      vToNext = *(p->m_pNext->m_pNext) - *(p->m_pNext);
      vToNext.ToUnitLength();
      vLeft = vToNext.CrossProduct(vUp);
      vLeft.ToUnitLength();

      glVertex3f(p->m_pNext->m_dX + vLeft.m_dX, p->m_pNext->m_dY + vLeft.m_dY, p->m_pNext->m_dZ);

      p = p->m_pNext;
    } while(p != m_shadow.Head());

    glEnd();
  }

  glEndList();
}






//*****************************************************************************
void BObject::DrawObject(bool bShadow) {
  if(bShadow) {
    glCallList(m_nDLShadow); 
  } else {
    glCallList(m_nDL); 
  }
}



//*****************************************************************************
void BObject::LoadObjectFromFile(CString sFilename, CString sSection, bool bOnlyShape) {

  // General
  CString sTmp;

  if(!bOnlyShape) {
    FileHelpers::GetKeyStringFromINIFile(sSection, "Name", sSection, m_sName, sFilename);
    FileHelpers::GetKeyStringFromINIFile(sSection, "Type", "USER_DEF", sTmp, sFilename);
    m_type = USER_DEF;

    FileHelpers::GetKeyStringFromINIFile(sSection, "ObjectFile", "default", m_sObjectFilename, sFilename);
    FileHelpers::GetKeyVectorFromINIFile(sSection, "Location", BVector(0, 0, 0), m_vLocation, sFilename);
    FileHelpers::GetKeyDoubleFromINIFile(sSection, "Scale", 1, m_dScale2, sFilename);
    FileHelpers::GetKeyDoubleFromINIFile(sSection, "ZRotation", 0, m_dZRotation, sFilename);
    FileHelpers::GetKeyStringFromINIFile(sSection, "Shadow", "True", sTmp, sFilename);
    m_bHasShadow = (sTmp.CompareNoCase("True") == 0);
    FileHelpers::GetKeyDoubleFromINIFile(sSection, "ActiveRadius", 1.0, m_dActiveRadius, sFilename);
  }

  FileHelpers::GetKeyStringFromINIFile("Properties", "CollisionDetection", "Accurate", sTmp, m_sObjectFilename);
  if(sTmp.CompareNoCase("Accurate") == 0) {
    m_collisionDetection = ACCURATE;
  } else if(sTmp.CompareNoCase("BoundingSphere") == 0) {
    m_collisionDetection = BOUNDING_SPHERE;
  } else {
    m_collisionDetection = BOUNDING_BOX;
  }

  // Geometry
  FileHelpers::GetKeyStringFromINIFile("Geometry", "RightDir", "+X", m_pOBJData->m_sRightDir, m_sObjectFilename);
  FileHelpers::GetKeyStringFromINIFile("Geometry", "ForwardDir", "+Y", m_pOBJData->m_sForwardDir, m_sObjectFilename);
  FileHelpers::GetKeyStringFromINIFile("Geometry", "DownDir", "+Z", m_pOBJData->m_sDownDir, m_sObjectFilename);
  FileHelpers::GetKeyDoubleFromINIFile("Geometry", "Scale", 1.0, m_pOBJData->m_dScale, m_sObjectFilename);

  // Load in the Parts
  // First count how many there are
  m_nParts = 0;
  do {
    CString sHasSection;
    CString sSection;
    sSection.Format("Part%d", m_nParts + 1);
    FileHelpers::GetKeyStringFromINIFile(sSection, "", "default", sHasSection, m_sObjectFilename);
    if(sHasSection.CompareNoCase("default") != 0) {
      ++m_nParts;
    } else {
      break;
    }
  } while(m_nParts < 10000); // just a sanity check to break the loop eventually

  if(m_pPart) {
    delete [] m_pPart;
  }
  m_pPart = new BPart[m_nParts];

  // Read parts
  for(int nPart = 0; nPart < m_nParts; ++nPart) {
    CString sSection;
    sSection.Format("Part%d", nPart + 1);
    m_pPart[nPart].SetOBJData(m_pOBJData);
    m_pPart[nPart].LoadPartFromFile(m_sObjectFilename, sSection, false, false);
  }

  if(m_collisionDetection == ACCURATE) {
    // Read/Setup collision detection parts

    if(m_pCollDetPart) {
      delete [] m_pCollDetPart;
    }

    m_nCollDetParts = m_nParts;
    m_pCollDetPart = new BObjectPart[m_nCollDetParts];

    for(int nPart = 0; nPart < m_nParts; ++nPart) {
      CString sSection;
      sSection.Format("Part%d", nPart + 1);
      LoadCollisionPartFromFile(m_sObjectFilename, sSection, m_pPart[nPart], m_pCollDetPart[nPart]);
    }
  }
}




//*****************************************************************************
void BObject::LoadCollisionPartFromFile(CString sFilename, CString sSection, BPart &rVisPart, BObjectPart &rPart) {
  // Read the collision parts from the object file. If they are not specified, 
  // use the visualization parts as the collision detection parts.

  // Friction
  FileHelpers::GetKeyDoubleFromINIFile(sSection, "Friction", 0.5, rPart.m_dFriction, sFilename);

  // Shape
  CString sShapeFilename = _T("");
  FileHelpers::GetKeyStringFromINIFile(sSection, "CollisionShape", "", sShapeFilename, sFilename);
  if(sShapeFilename.IsEmpty()) {
    // Use visualization part for collision detection
    // Convert read data to collision part data
    rPart.m_pFace = new BFace[rVisPart.m_nFaces];
    rPart.m_nFaces = rVisPart.m_nFaces;
    for(int nFace = 0; nFace < rPart.m_nFaces; ++nFace) {
      BVector vPoint[4];
      vPoint[0] = m_pOBJData->m_pvOBJPoints[m_pOBJData->m_pOBJFaces[rVisPart.m_nFaceStart + nFace].m_nV1];
      vPoint[1] = m_pOBJData->m_pvOBJPoints[m_pOBJData->m_pOBJFaces[rVisPart.m_nFaceStart + nFace].m_nV2];
      vPoint[2] = m_pOBJData->m_pvOBJPoints[m_pOBJData->m_pOBJFaces[rVisPart.m_nFaceStart + nFace].m_nV3];
      if(m_pOBJData->m_pOBJFaces[rVisPart.m_nFaceStart + nFace].m_nVertices > 3) {
        vPoint[3] = m_pOBJData->m_pvOBJPoints[m_pOBJData->m_pOBJFaces[rVisPart.m_nFaceStart + nFace].m_nV4];
      }
      rPart.m_pFace[nFace].SetPoints(m_pOBJData->m_pOBJFaces[rVisPart.m_nFaceStart + nFace].m_nVertices, vPoint);
      rPart.m_pFace[nFace].CalculateNormal();
    }
  } else {
    // Load collision part
    int nVertices = 0;
    int nFaces = 0;

    BOBJData OBJTmp;
    OBJTmp.Init();
    OBJTmp.m_dScale = m_pOBJData->m_dScale;
    OBJTmp.m_sDownDir = m_pOBJData->m_sDownDir;
    OBJTmp.m_sForwardDir = m_pOBJData->m_sForwardDir;
    OBJTmp.m_sRightDir = m_pOBJData->m_sRightDir;
    rPart.SetOBJData(&OBJTmp);
    rPart.AppendWaveFrontOBJShape(sShapeFilename, nVertices, nFaces, false, false, false);
    OBJTmp.PrepareWaveFrontModel();

    // Convert read data to collision part data
    rPart.m_pFace = new BFace[nFaces];
    rPart.m_nFaces = nFaces;
    for(int nFace = 0; nFace < nFaces; ++nFace) {
      BVector vPoint[4];
      vPoint[0] = OBJTmp.m_pvOBJPoints[OBJTmp.m_pOBJFaces[nFace].m_nV1] * m_dScale2;
      vPoint[1] = OBJTmp.m_pvOBJPoints[OBJTmp.m_pOBJFaces[nFace].m_nV2] * m_dScale2;
      vPoint[2] = OBJTmp.m_pvOBJPoints[OBJTmp.m_pOBJFaces[nFace].m_nV3] * m_dScale2;
      if(OBJTmp.m_pOBJFaces[nFace].m_nVertices > 3) {
        vPoint[3] = OBJTmp.m_pvOBJPoints[OBJTmp.m_pOBJFaces[nFace].m_nV4] * m_dScale2;
      }
      rPart.m_pFace[nFace].SetPoints(OBJTmp.m_pOBJFaces[nFace].m_nVertices, vPoint);
      rPart.m_pFace[nFace].CalculateNormal();
    }
  }
}


//*****************************************************************************
void BObject::RecreateShadow() {
  if(m_bHasShadow) {
    PreProcessVisualization(true);
  }
  // m_vCenterOnGround needs to be updated as well.

  // Set center on ground
  BVector vTmp = m_vCenter;
  vTmp.m_dZ = 0;
  double dTmp = 0;
  BVector vInternalOffset = BGame::GetSimulation()->GetTerrain()->m_vOffset;
  double dNewZ = -HeightMap::CalcHeightAt(vInternalOffset.m_dX + vTmp.m_dX, 
                                          vInternalOffset.m_dY + vTmp.m_dY, 
                                          dTmp, 
                                          BGame::GetSimulation()->GetScene()->m_terrainStyle,
                                          8,
                                          vInternalOffset.m_dY);

  m_vCenterOnGround.Set(m_vCenter.m_dX, m_vCenter.m_dY, dNewZ);

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