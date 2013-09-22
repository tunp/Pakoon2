//
// Ground
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#include "stdafx.h"
#include "BGround.h"
#include "BTerrain.h"

#include "OpenGLHelpers.h"
#include "BGame.h"

const double dPI = 3.141592654;
extern bool g_cbBlackAndWhite;



BGround::BGround() {

  // Load ground height map
  LoadGroundFromFile(".\\World\\ground.raw");

  // Load stationary objects
  LoadObjectsFromFile(".\\World\\objects.dat");

  // Load ground textures
  // LoadTextures();

  m_dCoarseDistance = 16000.0;
}

BGround::~BGround() {
  if(m_pObjects) {
    delete [] m_pObjects;
  }
}



extern double Random(double dRange);


void BGround::LoadObjectsFromFile(CString sFilename) {
} // LoadObjectsFromFile


void BGround::RenderSky(int nDispList) {
  GLfloat fLight1DiffuseG[ 4];
  GLfloat fLight1SpecularG[ 4];
  fLight1DiffuseG[0] = 0.0f;
  fLight1DiffuseG[1] = 0.0f;
  fLight1DiffuseG[2] = 0.0f;
  fLight1DiffuseG[3] = 0.0f;
  fLight1SpecularG[0] = 0.0f;
  fLight1SpecularG[1] = 0.0f;
  fLight1SpecularG[2] = 0.0f;
  fLight1SpecularG[3] = 0.0f;
  glLightfv(GL_LIGHT0, GL_DIFFUSE,  fLight1DiffuseG);
  glLightfv(GL_LIGHT0, GL_SPECULAR,  fLight1SpecularG);
  glShadeModel(GL_SMOOTH);
  glEnable(GL_CULL_FACE);
  // sky
  OpenGLHelpers::SetColorFull(1, 1, 1, 1);
  for(int y = 0; y < 9; ++y) {
    glBegin(GL_TRIANGLE_STRIP);
    for(int x = 0; x < 21; ++x) {
      if(y == 0) {
        if(nDispList == m_nDLSkyCoarse) {
          glTexCoord2f(double(x) / 20.0, 1.0 / 8.0);
        } else if(nDispList == m_nDLSkyDetailed) {
          glTexCoord2f(double(x) / 20.0, 1.0 / 128.0);
        }
      } else {
        glTexCoord2f(double(x) / 20.0, double(y) / 10.0);
      }
      glVertex3f(cos(double(x) / 20.0 * 2.0 * dPI) * cos(double(y) / 10.0 * dPI / 2.0) * cdWorldHemisphereRadius, 
                 sin(double(x) / 20.0 * 2.0 * dPI) * cos(double(y) / 10.0 * dPI / 2.0) * cdWorldHemisphereRadius, 
                 sin(double(y) / 10.0 * dPI / 2.0) * -cdWorldHemisphereRadius);
      glTexCoord2f(double(x) / 20.0, double(y+1) / 10.0);
      glVertex3f(cos(double(x) / 20.0 * 2.0 * dPI) * cos(double(y+1) / 10.0 * dPI / 2.0) * cdWorldHemisphereRadius, 
                 sin(double(x) / 20.0 * 2.0 * dPI) * cos(double(y+1) / 10.0 * dPI / 2.0) * cdWorldHemisphereRadius, 
                 sin(double(y+1) / 10.0 * dPI / 2.0) * -cdWorldHemisphereRadius);
    }
    glEnd();
  }
  glBegin(GL_TRIANGLE_FAN);
  if(nDispList == m_nDLSkyCoarse) {
    glTexCoord2f(0.5, 7.0/8.0);
  } else if(nDispList == m_nDLSkyDetailed) {
    glTexCoord2f(0.5, 127.0/128.0);
  }
  glVertex3f(0.0, 0.0, -cdWorldHemisphereRadius);
  int x;
  for(x = 20; x >= 0; --x) {
        glTexCoord2f(float(x) / 20.0, 9.0 / 10.0);
        glVertex3f(cos(double(x) / 20.0 * 2.0 * dPI) * cos(9.0 / 10.0 * dPI / 2.0) * cdWorldHemisphereRadius, 
                   sin(double(x) / 20.0 * 2.0 * dPI) * cos(9.0 / 10.0 * dPI / 2.0) * cdWorldHemisphereRadius, 
                   sin(9.0 / 10.0 * dPI / 2.0) * -cdWorldHemisphereRadius);
  }
  glEnd();
  glDisable(GL_CULL_FACE);  
  glDisable(GL_TEXTURE_2D);
  fLight1DiffuseG[0] = 0.8f;
  fLight1DiffuseG[1] = 0.8f;
  fLight1DiffuseG[2] = 0.8f;
  fLight1DiffuseG[3] = 0.8f;
  fLight1SpecularG[0] = 0.5f;
  fLight1SpecularG[1] = 0.5f;
  fLight1SpecularG[2] = 0.5f;
  fLight1SpecularG[3] = 0.5f;
  glLightfv(GL_LIGHT0, GL_DIFFUSE,  fLight1DiffuseG);
  glLightfv(GL_LIGHT0, GL_SPECULAR,  fLight1SpecularG);
}


//*****************************************************************************
void BGround::PreProcessVisualization() {
  // Generate ground as a opengl display list to speed up the visualization
  CreateGroundAreas();

  // Generate skies and ocean as a opengl display list to speed up the visualization

  m_nDLSkyCoarse = glGenLists(1);
  m_nDLSkyDetailed = glGenLists(1);
  glNewList(m_nDLSkyDetailed, GL_COMPILE);
  RenderSky(m_nDLSkyDetailed);
  glEndList();
  glNewList(m_nDLSkyCoarse, GL_COMPILE);
  RenderSky(m_nDLSkyCoarse);
  glEndList();

  // Water surface
  m_nDLWaterSurface = glGenLists(1);
  glNewList(m_nDLWaterSurface, GL_COMPILE);
  glDisable(GL_CULL_FACE);
  glNormal3f(0, 0, -1.0);

  double x, y, dTexScaler = 0.01;
  for(y = -cdWorldHemisphereRadius - cdWorldHemisphereRadius / 5.0; y < cdWorldHemisphereRadius + cdWorldHemisphereRadius / 5.0; y += cdWorldHemisphereRadius / 5.0) {
    glBegin(GL_TRIANGLE_STRIP);
    for(x = -cdWorldHemisphereRadius - cdWorldHemisphereRadius / 5.0; x < cdWorldHemisphereRadius + cdWorldHemisphereRadius / 5.0; x += cdWorldHemisphereRadius / 5.0) {

      OpenGLHelpers::SetTexCoord(x * dTexScaler, y * dTexScaler);
      glVertex3f(x, y, 0.0);
      OpenGLHelpers::SetTexCoord(x * dTexScaler, (y + cdWorldHemisphereRadius / 5.0) * dTexScaler);
      glVertex3f(x, y + cdWorldHemisphereRadius / 5.0, 0.0);
      OpenGLHelpers::SetTexCoord((x + cdWorldHemisphereRadius / 5.0) * dTexScaler, y * dTexScaler);
      glVertex3f(x + cdWorldHemisphereRadius / 5.0, y, 0.0);
      OpenGLHelpers::SetTexCoord((x + cdWorldHemisphereRadius / 5.0) * dTexScaler, (y + cdWorldHemisphereRadius / 5.0) * dTexScaler);
      glVertex3f(x + cdWorldHemisphereRadius / 5.0, y + cdWorldHemisphereRadius / 5.0, 0.0);
    }
    glEnd();
  }
  glEndList();
}


//*****************************************************************************
void BGround::Paint(int nSkyDetail, int nWaterDetail, BCamera &rCamera, TGroundType gt) {
  if(gt == GT_GROUND) {
    //glDisable(GL_DEPTH_TEST);

    // Render sky
    if(nSkyDetail == 1) {
      glCallList(m_nDLSkyCoarse);
    } else if(nSkyDetail == 2) {
      glCallList(m_nDLSkyDetailed);
    }
    //glEnable(GL_DEPTH_TEST);
  }
    
  // Render Objects
  glCallList(m_nDLObjects);
}



//*****************************************************************************
void BGround::PaintSky(int nSkyDetail) {
  // glDisable(GL_DEPTH_TEST);

  // Render sky
  if(nSkyDetail == 1) {
    glCallList(m_nDLSkyCoarse);
  } else if(nSkyDetail == 2) {
    glCallList(m_nDLSkyDetailed);
  }

  // glEnable(GL_DEPTH_TEST);
}

//*****************************************************************************
void BGround::PaintObjects(BCamera &rCamera) {
  // Render Objects
  glCallList(m_nDLObjects);
}


//*****************************************************************************
void BGround::PaintWaterSurface(double dBrightness, double dAlpha) {
  OpenGLHelpers::SetColorFull(0.3 * dBrightness, 0.4 * dBrightness, 0.5 * dBrightness, dAlpha);
  glCallList(m_nDLWaterSurface);
}




//******************************************************************************
void BGround::CreateGroundAreas() {

  m_nDLGroundSand = glGenLists(1);
  glNewList(m_nDLGroundSand, GL_COMPILE);
  glEnable(GL_CULL_FACE);  
  glEnable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
  OpenGLHelpers::SetColorFull(0.78, 0.58, 0.38, 0);
  glShadeModel(GL_SMOOTH);
  GLfloat fLight[4];
  fLight[0] = 0.0f;
  fLight[1] = 0.0f;
  fLight[2] = 0.0f;
  fLight[3] = 0.0f;
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, fLight);
  glEndList();

  m_nDLGroundRock = glGenLists(1);
  glNewList(m_nDLGroundRock, GL_COMPILE);
  glEnable(GL_CULL_FACE);  
  glEnable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
  OpenGLHelpers::SetColorFull(1.0, 0.9, 0.7, 0);
  glShadeModel(GL_SMOOTH);
  glEndList();

  m_nDLGroundWater = glGenLists(1);
  glNewList(m_nDLGroundWater, GL_COMPILE);
  glDisable(GL_CULL_FACE);  
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_SRC_COLOR);
  OpenGLHelpers::SetColorFull(0.15, 0.25, 0.35, 0);
  glShadeModel(GL_SMOOTH);
  glEndList();
}


bool BGround::GroundTriangleIsOfType(int x, int y, int nSize, int nTriangle, TGroundType gt) {
  BIndexGuard safe(g_cnGroundXSize * g_cnGroundYSize - 1, 0);
  BVector vNormal;
  double d1, d2, d3;
  if(nTriangle == 1) {
    vNormal = m_sqrHeightMapNormals[safe(y * g_cnGroundXSize + x)].m_vNormal1;
    d1 = m_dHeightMap[safe(y * g_cnGroundXSize + x)];
    d2 = m_dHeightMap[safe(y * g_cnGroundXSize + x+nSize)];
    d3 = m_dHeightMap[safe((y+nSize) * g_cnGroundXSize + x+nSize)];
  } else {
    vNormal = m_sqrHeightMapNormals[safe(y * g_cnGroundXSize + x)].m_vNormal2;
    d1 = m_dHeightMap[safe(y * g_cnGroundXSize + x)];
    d2 = m_dHeightMap[safe((y+nSize) * g_cnGroundXSize + x)];
    d3 = m_dHeightMap[safe((y+nSize) * g_cnGroundXSize + x+nSize)];
  }

  switch(gt) {
    case GT_SAND:
      return (vNormal.m_dZ < -0.5) && !GroundTriangleIsOfType(x, y, nSize, nTriangle, GT_WATER);
      break;
    case GT_ROCK:
      return (vNormal.m_dZ >= -0.5) && !GroundTriangleIsOfType(x, y, nSize, nTriangle, GT_WATER);
      break;
    case GT_WATER:
      return (d1 > g_cdWaterLevel) || (d2 > g_cdWaterLevel) || (d3 > g_cdWaterLevel);
      break;
  }
  return false;
}



double BGround::SurfaceHeightAt(int x, int y, TGroundType gt) {
  BIndexGuard safe(g_cnGroundXSize * g_cnGroundYSize - 1, 0);
  if(gt == GT_WATER) {
    return g_cdWaterLevel;
  } else {
    return m_dHeightMap[safe(y * g_cnGroundXSize + x)];
  }
}


void BGround::RenderAreaFull(int nX, int nY, int nSize, TGroundType gt) {
  BIndexGuard safe(g_cnGroundXSize * g_cnGroundYSize - 1, 0);
  double dBase;
  double dScale;
  glBegin(GL_TRIANGLES);
  for(int x = nX; x < nX + nSize; ++x) {
    for(int y = nY; y < nY + nSize; ++y) {
      if(GroundTriangleIsOfType(x, y, 1, 1, gt)) {
        dBase  = Random(1.0);
        dScale = 0.5 + Random(1.0);
        if(gt == GT_ROCK) {
          glTexCoord2f(dBase + dScale * (x+y), dBase + fabs(m_dHeightMap[safe(y * g_cnGroundXSize + x)] / g_cdGroundScale * dScale));
        } else if(gt == GT_SAND) {
          glTexCoord2f(x * 0.3, y * 0.3);
        }
        glNormal3f(m_sqrHeightMapNormals[safe(y * g_cnGroundXSize + x)].m_vNormalVis.m_dX,
                   m_sqrHeightMapNormals[safe(y * g_cnGroundXSize + x)].m_vNormalVis.m_dY,
                   m_sqrHeightMapNormals[safe(y * g_cnGroundXSize + x)].m_vNormalVis.m_dZ);
        glVertex3f(x * g_cdGroundScale, 
                   y * g_cdGroundScale, 
                   SurfaceHeightAt(x, y, gt));

        if(gt == GT_ROCK) {
          glTexCoord2f(dBase + dScale * (x+y+1), dBase + fabs(m_dHeightMap[safe(y * g_cnGroundXSize + x+1)] / g_cdGroundScale * dScale));
        } else if(gt == GT_SAND) {
          glTexCoord2f((x+1) * 0.3, y * 0.3);
        }
        glNormal3f(m_sqrHeightMapNormals[safe(y * g_cnGroundXSize + x+1)].m_vNormalVis.m_dX,
                   m_sqrHeightMapNormals[safe(y * g_cnGroundXSize + x+1)].m_vNormalVis.m_dY,
                   m_sqrHeightMapNormals[safe(y * g_cnGroundXSize + x+1)].m_vNormalVis.m_dZ);
        glVertex3f((x+1) * g_cdGroundScale, 
                   y * g_cdGroundScale, 
                   SurfaceHeightAt(x+1, y, gt));

        if(gt == GT_ROCK) {
          glTexCoord2f(dBase + dScale * (x+y+2), dBase + fabs(m_dHeightMap[safe((y+1) * g_cnGroundXSize + x+1)] / g_cdGroundScale * dScale));
        } else if(gt == GT_SAND) {
          glTexCoord2f((x+1) * 0.3, (y+1) * 0.3);
        }
        glNormal3f(m_sqrHeightMapNormals[safe((y+1) * g_cnGroundXSize + x+1)].m_vNormalVis.m_dX,
                   m_sqrHeightMapNormals[safe((y+1) * g_cnGroundXSize + x+1)].m_vNormalVis.m_dY,
                   m_sqrHeightMapNormals[safe((y+1) * g_cnGroundXSize + x+1)].m_vNormalVis.m_dZ);
        glVertex3f((x+1) * g_cdGroundScale, 
                   (y+1) * g_cdGroundScale, 
                   SurfaceHeightAt(x+1, y+1, gt));

        switch(gt) {
          case GT_SAND:
            m_sqrHeightMapNormals[safe(y * g_cnGroundXSize + x)].m_dFriction1 = 0.5;
            m_sqrHeightMapNormals[safe(y * g_cnGroundXSize + x)].m_dSurfaceDepth1 = 0.03;
            break;
          case GT_ROCK:
            m_sqrHeightMapNormals[safe(y * g_cnGroundXSize + x)].m_dFriction1 = 0.9;
            m_sqrHeightMapNormals[safe(y * g_cnGroundXSize + x)].m_dSurfaceDepth1 = 0.0;
            break;
          case GT_WATER:
            m_sqrHeightMapNormals[safe(y * g_cnGroundXSize + x)].m_dFriction1 = 0.2;
            m_sqrHeightMapNormals[safe(y * g_cnGroundXSize + x)].m_dSurfaceDepth1 = 0.0;
            m_sqrHeightMapNormals[safe(y * g_cnGroundXSize + x)].m_bWater1 = true;
            break;
        }
      }
      if(GroundTriangleIsOfType(x, y, 1, 2, gt)) {
        dBase  = Random(1.0);
        dScale = 0.5 + Random(1.0);
        if(gt == GT_ROCK) {
          glTexCoord2f(dBase + dScale * (x+y), dBase + fabs(m_dHeightMap[safe(y * g_cnGroundXSize + x)] / g_cdGroundScale * dScale));
        } else if(gt == GT_SAND) {
          glTexCoord2f(x * 0.3, y * 0.3);
        }
        glNormal3f(m_sqrHeightMapNormals[safe(y * g_cnGroundXSize + x)].m_vNormalVis.m_dX,
                   m_sqrHeightMapNormals[safe(y * g_cnGroundXSize + x)].m_vNormalVis.m_dY,
                   m_sqrHeightMapNormals[safe(y * g_cnGroundXSize + x)].m_vNormalVis.m_dZ);
        glVertex3f(x * g_cdGroundScale, 
                   y * g_cdGroundScale,
                   SurfaceHeightAt(x, y, gt));

        if(gt == GT_ROCK) {
          glTexCoord2f(dBase + dScale * (x+y+2), dBase + fabs(m_dHeightMap[safe((y+1) * g_cnGroundXSize + x+1)] / g_cdGroundScale * dScale));
        } else if(gt == GT_SAND) {
          glTexCoord2f((x+1) * 0.3, (y+1) * 0.3);
        }
        glNormal3f(m_sqrHeightMapNormals[safe((y+1) * g_cnGroundXSize + x+1)].m_vNormalVis.m_dX,
                   m_sqrHeightMapNormals[safe((y+1) * g_cnGroundXSize + x+1)].m_vNormalVis.m_dY,
                   m_sqrHeightMapNormals[safe((y+1) * g_cnGroundXSize + x+1)].m_vNormalVis.m_dZ);
        glVertex3f((x+1) * g_cdGroundScale, 
                   (y+1) * g_cdGroundScale, 
                   SurfaceHeightAt(x+1, y+1, gt));

        if(gt == GT_ROCK) {
          glTexCoord2f(dBase + dScale * (x+y+1), dBase + fabs(m_dHeightMap[safe((y+1) * g_cnGroundXSize + x)] / g_cdGroundScale * dScale));
        } else if(gt == GT_SAND) {
          glTexCoord2f(x * 0.3, (y+1) * 0.3);
        }
        glNormal3f(m_sqrHeightMapNormals[safe((y+1) * g_cnGroundXSize + x)].m_vNormalVis.m_dX,
                   m_sqrHeightMapNormals[safe((y+1) * g_cnGroundXSize + x)].m_vNormalVis.m_dY,
                   m_sqrHeightMapNormals[safe((y+1) * g_cnGroundXSize + x)].m_vNormalVis.m_dZ);
        glVertex3f(x * g_cdGroundScale, 
                   (y+1) * g_cdGroundScale, 
                   SurfaceHeightAt(x, y+1, gt));

        switch(gt) {
          case GT_SAND:
            m_sqrHeightMapNormals[safe(y * g_cnGroundXSize + x)].m_dFriction2 = 0.5;
            m_sqrHeightMapNormals[safe(y * g_cnGroundXSize + x)].m_dSurfaceDepth2 = 0.03;
            break;
          case GT_ROCK:
            m_sqrHeightMapNormals[safe(y * g_cnGroundXSize + x)].m_dFriction2 = 0.9;
            m_sqrHeightMapNormals[safe(y * g_cnGroundXSize + x)].m_dSurfaceDepth2 = 0.0;
            break;
          case GT_WATER:
            m_sqrHeightMapNormals[safe(y * g_cnGroundXSize + x)].m_dFriction2 = 0.2;
            m_sqrHeightMapNormals[safe(y * g_cnGroundXSize + x)].m_dSurfaceDepth2 = 0.0;
            m_sqrHeightMapNormals[safe(y * g_cnGroundXSize + x)].m_bWater2 = true;
            break;
        }
      }
    }
  }
  glEnd();
}


void BGround::RenderAreaCoarse(int nX, int nY, int nSize, TGroundType gt){
  double dBase;
  double dScale;
  BIndexGuard safe(g_cnGroundXSize * g_cnGroundYSize - 1, 0);
  if(AreaFlatEnough(nX, nY, nSize)) {
    // Render are using corner points
    int x = nX;
    int y = nY;
    glBegin(GL_TRIANGLES);
    if(GroundTriangleIsOfType(x, y, nSize, 1, gt)) {
      dBase  = Random(1.0);
      dScale = 0.5 + Random(1.0) * double(nSize);
      if(gt == GT_ROCK) {
        glTexCoord2f(dBase + dScale * (x+y), dBase + fabs(m_dHeightMap[safe(y * g_cnGroundXSize + x)] / g_cdGroundScale * dScale));
      } else if(gt == GT_SAND) {
        glTexCoord2f(x * 0.3 * nSize, y * 0.3 * nSize);
      }
      glNormal3f(m_sqrHeightMapNormals[safe(y * g_cnGroundXSize + x)].m_vNormalVis.m_dX,
                 m_sqrHeightMapNormals[safe(y * g_cnGroundXSize + x)].m_vNormalVis.m_dY,
                 m_sqrHeightMapNormals[safe(y * g_cnGroundXSize + x)].m_vNormalVis.m_dZ);
      glVertex3f(x * g_cdGroundScale, y * g_cdGroundScale, m_dHeightMap[safe(y * g_cnGroundXSize + x)]);

      if(gt == GT_ROCK) {
        glTexCoord2f(dBase + dScale * (x+y+nSize), dBase + fabs(m_dHeightMap[safe(y * g_cnGroundXSize + (x+nSize))] / g_cdGroundScale * dScale));
      } else if(gt == GT_SAND) {
        glTexCoord2f((x+nSize) * 0.3 * nSize, y * 0.3 * nSize);
      }
      glNormal3f(m_sqrHeightMapNormals[safe(y * g_cnGroundXSize + x+nSize)].m_vNormalVis.m_dX,
                 m_sqrHeightMapNormals[safe(y * g_cnGroundXSize + x+nSize)].m_vNormalVis.m_dY,
                 m_sqrHeightMapNormals[safe(y * g_cnGroundXSize + x+nSize)].m_vNormalVis.m_dZ);
      glVertex3f((x+nSize) * g_cdGroundScale, y * g_cdGroundScale, m_dHeightMap[safe(y * g_cnGroundXSize + (x+nSize))]);

      if(gt == GT_ROCK) {
        glTexCoord2f(dBase + dScale * (x+y+2*nSize), dBase + fabs(m_dHeightMap[safe((y+nSize) * g_cnGroundXSize + (x+nSize))] / g_cdGroundScale * dScale));
      } else if(gt == GT_SAND) {
        glTexCoord2f((x+nSize) * 0.3 * nSize, (y+nSize) * 0.3 * nSize);
      }
      glNormal3f(m_sqrHeightMapNormals[safe((y+nSize) * g_cnGroundXSize + x+nSize)].m_vNormalVis.m_dX,
                 m_sqrHeightMapNormals[safe((y+nSize) * g_cnGroundXSize + x+nSize)].m_vNormalVis.m_dY,
                 m_sqrHeightMapNormals[safe((y+nSize) * g_cnGroundXSize + x+nSize)].m_vNormalVis.m_dZ);
      glVertex3f((x+nSize) * g_cdGroundScale, (y+nSize) * g_cdGroundScale, m_dHeightMap[safe((y+nSize) * g_cnGroundXSize + (x+nSize))]);
    }
    if(GroundTriangleIsOfType(x, y, nSize, 2, gt)) {
      dBase  = Random(1.0);
      dScale = 0.5 + Random(1.0) * double(nSize);
      if(gt == GT_ROCK) {
        glTexCoord2f(dBase + dScale * (x+y), dBase + fabs(m_dHeightMap[safe(y * g_cnGroundXSize + x)] / g_cdGroundScale * dScale));
      } else if(gt == GT_SAND) {
        glTexCoord2f(x * 0.3 * nSize, y * 0.3 * nSize);
      }
      glNormal3f(m_sqrHeightMapNormals[safe(y * g_cnGroundXSize + x)].m_vNormalVis.m_dX,
                 m_sqrHeightMapNormals[safe(y * g_cnGroundXSize + x)].m_vNormalVis.m_dY,
                 m_sqrHeightMapNormals[safe(y * g_cnGroundXSize + x)].m_vNormalVis.m_dZ);
      glVertex3f(x * g_cdGroundScale, y * g_cdGroundScale, m_dHeightMap[safe(y * g_cnGroundXSize + x)]);

      if(gt == GT_ROCK) {
        glTexCoord2f(dBase + dScale * (x+y+2*nSize), dBase + fabs(m_dHeightMap[safe((y+nSize) * g_cnGroundXSize + (x+nSize))] / g_cdGroundScale * dScale));
      } else if(gt == GT_SAND) {
        glTexCoord2f((x+nSize) * 0.3 * nSize, (y+nSize) * 0.3 * nSize);
      }
      glNormal3f(m_sqrHeightMapNormals[safe((y+nSize) * g_cnGroundXSize + x+nSize)].m_vNormalVis.m_dX,
                 m_sqrHeightMapNormals[safe((y+nSize) * g_cnGroundXSize + x+nSize)].m_vNormalVis.m_dY,
                 m_sqrHeightMapNormals[safe((y+nSize) * g_cnGroundXSize + x+nSize)].m_vNormalVis.m_dZ);
      glVertex3f((x+nSize) * g_cdGroundScale, (y+nSize) * g_cdGroundScale, m_dHeightMap[safe((y+nSize) * g_cnGroundXSize + (x+nSize))]);

      if(gt == GT_ROCK) {
        glTexCoord2f(dBase + dScale * (x+y+nSize), dBase + fabs(m_dHeightMap[safe((y+nSize) * g_cnGroundXSize + x)] / g_cdGroundScale * dScale));
      } else if(gt == GT_SAND) {
        glTexCoord2f(x * 0.3 * nSize, (y+nSize) * 0.3 * nSize);
      }
      glNormal3f(m_sqrHeightMapNormals[safe((y+nSize) * g_cnGroundXSize + x)].m_vNormalVis.m_dX,
                 m_sqrHeightMapNormals[safe((y+nSize) * g_cnGroundXSize + x)].m_vNormalVis.m_dY,
                 m_sqrHeightMapNormals[safe((y+nSize) * g_cnGroundXSize + x)].m_vNormalVis.m_dZ);
      glVertex3f(x * g_cdGroundScale, (y+nSize) * g_cdGroundScale, m_dHeightMap[safe((y+nSize) * g_cnGroundXSize + x)]);
    }
    glEnd();
  } else {
    nSize /= 2;
    RenderAreaCoarse(nX, nY, nSize, gt);
    RenderAreaCoarse(nX + nSize, nY, nSize, gt);
    RenderAreaCoarse(nX, nY + nSize, nSize, gt);
    RenderAreaCoarse(nX + nSize, nY + nSize, nSize, gt);
  }
}

bool BGround::AreaFlatEnough(int nX, int nY, int nSize) {
  BIndexGuard safe(g_cnGroundXSize * g_cnGroundYSize - 1, 0);
  if(nSize <= 1) {
    return true;
  } else {
    double dH00 = m_dHeightMap[safe(nY * g_cnGroundXSize + nX)];
    double dH10 = m_dHeightMap[safe(nY * g_cnGroundXSize + nX + 1)];
    double dH01 = m_dHeightMap[safe((nY + 1) * g_cnGroundXSize + nX)];
    double dH11 = m_dHeightMap[safe((nY + 1) * g_cnGroundXSize + nX + 1)];
    for(int y = nY; y <= nY + nSize; ++y) {
      for(int x = nX; x <= nX + nSize; ++x) {
        if((x != nX) && (x != nX + nSize) && (y != nY) && (y != nY + nSize)) {
          double dFX = 1.0 - double(x - nX) / double(nSize);
          double dFY = 1.0 - double(y - nY) / double(nSize);
          double dIdealHeight = m_dHeightMap[safe(y * g_cnGroundXSize + x)];
          double dInterpolatedHeight = dFX         *  dFY        * dH00 + 
                                       (1.0 - dFX) * dFY         * dH10 + 
                                       dFX         * (1.0 - dFY) * dH01 + 
                                       (1.0 - dFX) * (1.0 - dFY) * dH11;
          if(fabs(dInterpolatedHeight - dIdealHeight) > double(nSize) * 3.0) {
            return false;
          }
        }
      }
    }
  }
  return true;
}

void BGround::PaintGroundAreas(BCamera &rCamera, TGroundType gt) {
}


//*****************************************************************************
void BGround::LoadGroundFromFile(CString sFilename) {
  int x, y;
  FILE *fp = fopen(sFilename, "r");
  if(fp) {    
    for(y = 0; y < g_cnGroundYSize; ++y) {
      for(x = 0; x < g_cnGroundXSize; ++x) {
        unsigned char cTmp;
        fscanf(fp, "%c", &cTmp);
        m_dHeightMap[y * g_cnGroundXSize + x] = -double(255 - cTmp) * g_cdGroundHeightScale;
      }
    }
    fclose(fp);

    // Precalculate ground normals
    // Initialize with level ground
    for(y = 0; y < g_cnGroundYSize; ++y) {
      for(x = 0; x < g_cnGroundXSize; ++x) {
        m_sqrHeightMapNormals[y * g_cnGroundXSize + x].m_vNormal1 = BVector(0, 0, -1);
        m_sqrHeightMapNormals[y * g_cnGroundXSize + x].m_vNormal2 = BVector(0, 0, -1);
      }
    }
    // Calculate triangle normals for the simulation
    for(y = 1; y < (g_cnGroundYSize - 1); ++y) {
      for(x = 1; x < (g_cnGroundXSize - 1); ++x) {
        BVector v1, v2, v3, v4;
        v1.Set(x * g_cdGroundScale, y * g_cdGroundScale, m_dHeightMap[y * g_cnGroundXSize + x]);
        v2.Set((x+1) * g_cdGroundScale, y * g_cdGroundScale, m_dHeightMap[y * g_cnGroundXSize + (x+1)]);
        v3.Set(x * g_cdGroundScale, (y+1) * g_cdGroundScale, m_dHeightMap[(y+1) * g_cnGroundXSize + x]);
        v4.Set((x+1) * g_cdGroundScale, (y+1) * g_cdGroundScale, m_dHeightMap[(y+1) * g_cnGroundXSize + (x+1)]);
        m_sqrHeightMapNormals[y * g_cnGroundXSize + x].m_vNormal1 = (v1 - v2).CrossProduct(v4 - v2);
        m_sqrHeightMapNormals[y * g_cnGroundXSize + x].m_vNormal1.ToUnitLength();
        m_sqrHeightMapNormals[y * g_cnGroundXSize + x].m_vNormal2 = (v4 - v3).CrossProduct(v1 - v3);
        m_sqrHeightMapNormals[y * g_cnGroundXSize + x].m_vNormal2.ToUnitLength();
      }
    }
    // calculate visualization normals
    for(y = 0; y < g_cnGroundYSize; ++y) {
      m_sqrHeightMapNormals[y * g_cnGroundXSize].m_vNormalVis = BVector(0, 0, -1);
    }
    for(x = 0; x < g_cnGroundXSize; ++x) {
      m_sqrHeightMapNormals[x].m_vNormalVis = BVector(0, 0, -1);
    }

    for(y = 1; y < g_cnGroundYSize; ++y) {
      for(x = 1; x < g_cnGroundXSize; ++x) {
        m_sqrHeightMapNormals[y * g_cnGroundXSize + x].m_vNormalVis = 
          m_sqrHeightMapNormals[(y - 1) * g_cnGroundXSize + x - 1].m_vNormal1 +
          m_sqrHeightMapNormals[(y - 1) * g_cnGroundXSize + x - 1].m_vNormal2 +
          m_sqrHeightMapNormals[(y - 1) * g_cnGroundXSize + x].m_vNormal1 +
          m_sqrHeightMapNormals[(y - 1) * g_cnGroundXSize + x].m_vNormal2 +
          m_sqrHeightMapNormals[y * g_cnGroundXSize + x - 1].m_vNormal1 +
          m_sqrHeightMapNormals[y * g_cnGroundXSize + x - 1].m_vNormal2 +
          m_sqrHeightMapNormals[y * g_cnGroundXSize + x].m_vNormal1 +
          m_sqrHeightMapNormals[y * g_cnGroundXSize + x].m_vNormal2;
        m_sqrHeightMapNormals[y * g_cnGroundXSize + x].m_vNormalVis.ToUnitLength();
      }
    }
  } else {
    BGame::MyAfxMessageBox("Cannot open ground file!");
  }
}

void BGround::LoadTextures() {
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