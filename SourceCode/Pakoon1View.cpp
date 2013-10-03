// Pakoon1View.cpp : implementation of the CPakoon1View class
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#include "Pakoon1View.h"

#include "OpenGLHelpers.h"
#include "OpenGLExtFunctions.h"
#include "BTextures.h"
#include <GL/gl.h>
#include <GL/glu.h>

#include "SoundModule.h"
#include "Settings.h"
#include "BNavSatWnd.h"
#include "BServiceWnd.h"
#include "BMessages.h"
#include "HeightMap.h"
#include "BTextRenderer.h"
#include "BUI.h"

#include <sstream>

using namespace std;

extern bool g_cbBlackAndWhite;
extern double Random(double dRange);

double g_dPhysicsStepsInSecond = 200.0; // Was 200.0
bool g_bControl = false;
bool g_bShift = false;

double g_dRate = 30.0;
double g_d10LastFPS[500];
double g_dAveRate = 30.0;
extern double g_dExtraAlpha;

BVector g_vText;
static double g_cdPI = 3.141592654;

/////////////////////////////////////////////////////////////////////////////
// CPakoon1View construction/destruction

//*************************************************************************************************
CPakoon1View::CPakoon1View() {
  m_nMenuTime = 0;
  m_pDrawFunction = &CPakoon1View::OnDrawIntro;
  m_pKeyDownFunction = &CPakoon1View::OnKeyDownIntro;
  m_bDrawOnlyMenu = false;
  m_bFullRedraw = true;
  //m_hCursor = 0; //FIXME
  SoundModule::Initialize();
  m_bInitClock = true;
  m_bCreateDLs = false;
  m_bWireframe = false;
  m_bNormals = false;
  m_bIgnoreNextChar = false;
  m_clockMenuScroll = 0;
  //m_pThreadLoading = 0;
  m_clockHighlightMenu = 0;
  
  exit = false;
}

CPakoon1View::~CPakoon1View() {
}


//*************************************************************************************************
int CPakoon1View::OnCreate() {

  // Save current screen resolution
  //FIXME
  /*EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &g_devmodeOrig);

  BOOL retval = CView::OnCreate(lpCreateStruct);

  HWND hWnd = GetSafeHwnd();
  HDC hDC = ::GetDC(hWnd);
  PIXELFORMATDESCRIPTOR pfd = {
  sizeof(PIXELFORMATDESCRIPTOR), // size of this pfd
  1, // version number
  PFD_DRAW_TO_WINDOW |    // support window
    PFD_SUPPORT_OPENGL |  // support OpenGL
    // PFD_SUPPORT_GDI |     // support GDI
    PFD_DOUBLEBUFFER |    // double buffered
    PFD_NEED_SYSTEM_PALETTE, 
  PFD_TYPE_RGBA, // RGBA type
  32, // 32-bit color depth
  0, 0, 0, 0, 0, 0, // color bits ignored
  0, // no alpha buffer
  0, // shift bit ignored
  0, // no accumulation buffer
  0, 0, 0, 0, // accum bits ignored
  32, // 32-bit z-buffer 
  8, // 8-bit stencil buffer for reflections
  0, // no auxiliary buffer
  PFD_MAIN_PLANE, // main layer
  0, // reserved
  0, 0, 0 // layer masks ignored
  };
  int pixelformat = ChoosePixelFormat(hDC, &pfd);
  TRACE("Pixelformat %d\n", pixelformat);
  if(SetPixelFormat(hDC, pixelformat, &pfd) == FALSE) {
    BGame::MyAfxMessageBox("SetPixelFormat failed");
  }  
  m_hGLRC = wglCreateContext(hDC);
  wglMakeCurrent(hDC, m_hGLRC);*/

  // Check Player.State file integrity
  BGame::GetPlayer()->LoadStateFile();
  if((BGame::GetPlayer()->m_dCash <= 0.01) && (BGame::GetPlayer()->m_dFuel <= 0.01)) {
    // Assist player
    BGame::MyAfxMessageBox("Assisting player with 50 units of cash, 1/4 tank of fuel.");
    BGame::GetPlayer()->m_dCash = 50.0;
    BGame::GetPlayer()->m_dFuel = 25.0;
    BGame::GetPlayer()->SaveStateFile();
  }
  if((BGame::GetPlayer()->m_dCash > 0.0) && (BGame::GetPlayer()->m_dFuel <= 0.01)) {
    // Assist player
    BGame::MyAfxMessageBox("Assisting player with 1/4 tank of fuel.");
    BGame::GetPlayer()->m_dCash -= 25.0;
    BGame::GetPlayer()->m_dFuel = 25.0;
    BGame::GetPlayer()->SaveStateFile();
  }

  // Set view for BGame
  BGame::SetView(this);

  // Read settings for graphics etc.
  Settings::ReadSettings(m_game.GetSimulation());
  switch(BGame::m_nTerrainResolution) {
    case 0: m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_MINIMUM); break;
    case 1: m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_LOW); break;
    case 2: m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_MEDIUM); break;
    case 3: m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_HIGH); break;
    case 4: m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_MAXIMUM); break;
    case 5: m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_SLOW_MACHINE); break;
  }
  BGame::m_nShowEffects = BGame::m_nDustAndClouds;
  BGame::m_bNight = (BGame::m_nWaterSurface == 1);

  // Initialize common OpenGL features
  InitializeOpenGL();

  // Initialize multitexturing support
  OpenGLHelpers::Init();
  BTextures::Init();
  BUI::TextRenderer()->Init();

  m_game.GetSimulation()->GetTerrain()->PreProcessVisualization();
  m_game.GetSimulation()->PreProcessVisualization();

  // Check for multiprosessor support
  //FIXME
  /*SYSTEM_INFO si;
  GetSystemInfo(&si);
  BGame::m_bMultiProcessor = si.dwNumberOfProcessors > 1;*/

  // Change cursor to Pakoon! cursor
  //FIXME
 /*m_hCursor = AfxGetApp()->LoadCursor(IDC_POINTER);
  if(m_hCursor) {
    ::SetClassLong(GetSafeHwnd(), GCL_HCURSOR, 0L);
    ::SetCursor(m_hCursor);
  }

  if(m_hCursor) {
    ::SetCursor(m_hCursor);
  }*/
  SDL_ShowCursor(0);

  BGame::MyAfxMessageBox("----------------------");
  BGame::MyAfxMessageBox("- OpenGL Info        -");
  BGame::MyAfxMessageBox("----------------------");
  string sInfo, sInfo2;
  sInfo2 = (char *) glGetString(GL_VENDOR);
  sInfo = "Vendor: " + sInfo2;
  BGame::MyAfxMessageBox(sInfo);
  sInfo2 = (char *) glGetString(GL_RENDERER);
  sInfo = "Renderer: " + sInfo2;
  BGame::MyAfxMessageBox(sInfo);
  sInfo2 = (char *) glGetString(GL_VERSION);
  sInfo = "Version: " + sInfo2;
  BGame::MyAfxMessageBox(sInfo);
  sInfo2 = (char *) glGetString(GL_EXTENSIONS);
  sInfo = "Extensions: " + sInfo2;
  BGame::MyAfxMessageBox(sInfo);
  BGame::MyAfxMessageBox("----------------------");

  // Preplay sound effects to wake them up.
  int n = SoundModule::GetVehicleSoundsVolume();
  SoundModule::SetVehicleSoundsVolume(0);
  SoundModule::PlayMenuBrowseSound();
  SoundModule::PlayMenuScrollSound();
  SoundModule::PlayMenuBackSound();
  SoundModule::PlaySlalomPortSound();
  SoundModule::PlayDisqualifiedSound();
  SoundModule::PlayCountdown123Sound();
  SoundModule::PlayCountdownGoSound();
  SoundModule::PlayMultiplayerJoinSound();
  SoundModule::PlayMultiplayerLeftSound();
  SoundModule::PlayGoalFanfarSound();
  SoundModule::SetVehicleSoundsVolume(n);
	
  //omat
  BGame::m_nDispWidth = m_rectWnd.w;
  BGame::m_nDispHeight = m_rectWnd.h;

  return 0;
}

//*************************************************************************************************
void CPakoon1View::OnDestroy() {
  // Return to original display settings

	//FIXME ja alapuolikin
  /*DEVMODE devmode;
  EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);
  if((devmode.dmPelsWidth  != g_devmodeOrig.dmPelsWidth) || 
     (devmode.dmPelsHeight != g_devmodeOrig.dmPelsHeight) || 
     (devmode.dmBitsPerPel != g_devmodeOrig.dmBitsPerPel) ||
     (devmode.dmDisplayFrequency != g_devmodeOrig.dmDisplayFrequency)) {
    devmode.dmPelsWidth = g_devmodeOrig.dmPelsWidth;
    devmode.dmPelsHeight = g_devmodeOrig.dmPelsHeight;
    devmode.dmBitsPerPel = g_devmodeOrig.dmBitsPerPel;
    devmode.dmDisplayFrequency = g_devmodeOrig.dmDisplayFrequency;
    ChangeDisplaySettings(&devmode, 0);
  }

  HWND hWnd = GetSafeHwnd();
  HDC hDCWnd = ::GetDC(hWnd);
  wglMakeCurrent(hDCWnd, m_hGLRC);*/
  BTextures::Exit();
  /*HDC   hDC = wglGetCurrentDC();
  wglMakeCurrent(NULL, NULL);
  if (m_hGLRC)
    wglDeleteContext(m_hGLRC);
  if (hDC)
    ::ReleaseDC(GetSafeHwnd(), hDC);
  CView::OnDestroy();*/
}


//*************************************************************************************************
void CPakoon1View::InitializeOpenGL() {
  glClearDepth(1);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  glShadeModel(GL_SMOOTH);
  glEnable(GL_LIGHTING);
  glEnable(GL_NORMALIZE);

  glEnable( GL_LIGHT0);

  GLfloat fLight1AmbientG[ 4];
  GLfloat fLight1DiffuseG[ 4];
  GLfloat fLight1SpecularG[ 4];

  fLight1AmbientG[0] = 0;
  fLight1AmbientG[1] = 0;
  fLight1AmbientG[2] = 0;
  fLight1AmbientG[3] = 1;

  fLight1DiffuseG[0] = 1.4;
  fLight1DiffuseG[1] = 1.4;
  fLight1DiffuseG[2] = 1;
  fLight1DiffuseG[3] = 1;

  fLight1SpecularG[0] = 1.4;
  fLight1SpecularG[1] = 1.4;
  fLight1SpecularG[2] = 1;
  fLight1SpecularG[3] = 1;

  glLightfv( GL_LIGHT1, GL_AMBIENT,  fLight1AmbientG);
  glLightfv( GL_LIGHT1, GL_DIFFUSE,  fLight1DiffuseG);
  glLightfv( GL_LIGHT1, GL_SPECULAR, fLight1SpecularG);
  glLighti( GL_LIGHT1, GL_SPOT_CUTOFF, 30);
  glLighti( GL_LIGHT1, GL_SPOT_EXPONENT, 40);
  // glLightf( GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.001);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, (GLfloat) 1.0);
  glDisable( GL_LIGHT1);

  OpenGLHelpers::SetDefaultLighting();

  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  switch(m_game.m_nTextureSmoothness) {
    case 0: // Boxy
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      break;
    case 1: // Gritty
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      break;
    case 2: // Silky
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      break;
  }

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glClearColor(0.85, 0.85, 1, 0); /* For RGB-mode */

  GLfloat vFogColor[4];
  vFogColor[0] = 0.85f;
  vFogColor[1] = 0.85f;
  vFogColor[2] = 1.0;
  vFogColor[3] = 0.0;
  //vFogColor[0] = 0;
  //vFogColor[1] = 0;
  //vFogColor[2] = 0;
  //vFogColor[3] = 0;
  glEnable( GL_FOG);
  glFogi( GL_FOG_MODE, GL_LINEAR);
  glFogf( GL_FOG_START, 500.0f / 1.2f);
  glFogf( GL_FOG_END, 1100.0f / 1.2f);
  glFogfv( GL_FOG_COLOR, vFogColor);

  // Prepare for environment mapping
  // glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	// glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);	
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);

  return;
}






static bool g_bResolutionChanged = false;



//*************************************************************************************************
void CPakoon1View::OnDrawIntro() {
	//FIXME
  /*if(m_hCursor) {
    ::SetCursor(m_hCursor);
  }*/
  SDL_ShowCursor(0);

  // Check if we need to change display settings
  //FIXME
  /*if(!g_bResolutionChanged) {
    g_bResolutionChanged = true;
    DEVMODE devmode;
    EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);
    if((devmode.dmPelsWidth  != (DWORD)BGame::m_nDispWidth) || 
       (devmode.dmPelsHeight != (DWORD)BGame::m_nDispHeight) || 
       (devmode.dmBitsPerPel != (DWORD)BGame::m_nDispBits) ||
       (devmode.dmDisplayFrequency != (DWORD)BGame::m_nDispHz)) {
      devmode.dmPelsWidth = (DWORD)BGame::m_nDispWidth;
      devmode.dmPelsHeight = (DWORD)BGame::m_nDispHeight;
      devmode.dmBitsPerPel = (DWORD)BGame::m_nDispBits;
      devmode.dmDisplayFrequency = (DWORD)BGame::m_nDispHz;
      ChangeDisplaySettings(&devmode, CDS_FULLSCREEN);
      AfxGetMainWnd()->SetWindowPos(NULL, -2, -2, BGame::m_nDispWidth + 4, BGame::m_nDispHeight + 4, 0);
    }
  }

  HDC hDC = pDC->GetSafeHdc();
  wglMakeCurrent(hDC, m_hGLRC); */

  glClearColor(0, 0, 0, 0);

  // Init OpenGL
  glDrawBuffer(GL_BACK);

  // Reset OpenGL
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glViewport(0, 0, (GLint) m_rectWnd.w, (GLint) m_rectWnd.h);
  gluLookAt(0, -5, 0, 0, 0, 0, 0, 0, -1);

  GLfloat fLight1PositionG[ 4];
  fLight1PositionG[0] = (GLfloat) 1;
  fLight1PositionG[1] = (GLfloat) 1;
  fLight1PositionG[2] = (GLfloat) -1; // -1;
  fLight1PositionG[3] = (GLfloat) 0; /* w=0 -> directional light (not positional) */
  glLightfv( GL_LIGHT0, GL_POSITION, fLight1PositionG);                     

  //*************************************************
  // Draw Intro
  //*************************************************

  // Setup 2D rendering
  Setup2DRendering();

  static bool bInitClock = true;
  static clock_t clockIntro;
  if(bInitClock) {
    SoundModule::PreCacheIntroSound();
    SoundModule::StartIntroSound();
    clockIntro = clock();
    bInitClock = false;
  }
  double dPhase = double(clock() - clockIntro) / double(CLOCKS_PER_SEC) - 0.5;

  double dMOSAlpha = (5.0 - fabs(dPhase - 5.0)) / 2.5;
  if((dMOSAlpha > 1.0) || (dPhase > 5.0)) {
    dMOSAlpha = 1.0;
  }

  double dScale = 0.25 + (dPhase / 8.0);
  dScale = sqrt(dScale);
  if(dScale > 1.0) {
    dScale = 1.0;
  }

  OpenGLHelpers::SwitchToTexture(0);
  BTextures::Use(BTextures::MOS_LOGO);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // not mipmapped

  double dWidth = m_rectWnd.w / 2.0 * dScale;
  double dHeight = m_rectWnd.w / 2.0 / 8.0 * dScale;

  OpenGLHelpers::SetColorFull(1, 1, 1, dMOSAlpha);
  glBegin(GL_QUADS);
  glNormal3f(0, 0, -1);
  OpenGLHelpers::SetTexCoord(0, 0);
  glVertex3f(m_rectWnd.w / 2.0 - dWidth, m_rectWnd.h / 2.0 - dHeight, 0);
  OpenGLHelpers::SetTexCoord(0, 1);
  glVertex3f(m_rectWnd.w / 2.0 - dWidth, m_rectWnd.h / 2.0 + dHeight, 0);
  OpenGLHelpers::SetTexCoord(1, 1);
  glVertex3f(m_rectWnd.w / 2.0 + dWidth, m_rectWnd.h / 2.0 + dHeight, 0);
  OpenGLHelpers::SetTexCoord(1, 0);
  glVertex3f(m_rectWnd.w / 2.0 + dWidth, m_rectWnd.h / 2.0 - dHeight, 0);
  glEnd();

  glBlendFunc(GL_SRC_ALPHA, GL_ONE);

  // Draw FEEL THE CODE

  bool bFTCVisible = false;
  if(dScale >= 1.0) {
    bFTCVisible = true;
  }

  dScale = 0.88;

  dWidth = m_rectWnd.w / 2.0 * dScale;
  dHeight = m_rectWnd.w / 2.0 / 8.0 * dScale;

  BTextures::Use(BTextures::FTC_LOGO);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // not mipmapped

  glPushMatrix();
  glTranslatef(-5.0 * (dWidth / 512.0 / 0.88), -64.0 * (dWidth / 512.0 / 0.88) , 0);

  OpenGLHelpers::SetColorFull(1, 1, 1, bFTCVisible ? 0.75 : 0.0);
  glBegin(GL_QUADS);
  glNormal3f(0, 0, -1);
  OpenGLHelpers::SetTexCoord(0, 0);
  glVertex3f(m_rectWnd.w / 2.0 - dWidth, m_rectWnd.h / 2.0 - dHeight, 0);
  OpenGLHelpers::SetTexCoord(0, 1);
  glVertex3f(m_rectWnd.w / 2.0 - dWidth, m_rectWnd.h / 2.0 + dHeight, 0);
  OpenGLHelpers::SetTexCoord(1, 1);
  glVertex3f(m_rectWnd.w / 2.0 + dWidth, m_rectWnd.h / 2.0 + dHeight, 0);
  OpenGLHelpers::SetTexCoord(1, 0);
  glVertex3f(m_rectWnd.w / 2.0 + dWidth, m_rectWnd.h / 2.0 - dHeight, 0);
  glEnd();

  // Draw light wave over FEEL THE CODE logo

  double dFTCAlpha = bFTCVisible ? pow(sin((dPhase - 6.0) / 10.0 * 3.141592654), 2.0) : 0.0;

  BTextures::Use(BTextures::FTC_LOGO2);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // not mipmapped

  OpenGLHelpers::SetColorFull(1, 1, 1, dFTCAlpha);

  double dMinX = m_rectWnd.w / 2.0 - dWidth;
  double dMaxX = m_rectWnd.w / 2.0 + dWidth;
  double dMinY = m_rectWnd.h / 2.0 - dHeight;
  double dMaxY = m_rectWnd.h / 2.0 + dHeight;
  double dStepX = (dMaxX - dMinX) / 20.0;
  double dStepY = (dMaxY - dMinY) / 4.0;
  double dLoopStepX = 1.0 / 20.0;
  double dLoopStepY = 1.0 / 4.0;

  glBegin(GL_QUADS);
  glNormal3f(0, 0, -1);

  for(double dLoopX = 0.0; dLoopX < 0.99; dLoopX += dLoopStepX) {
    for(double dLoopY = 0.0; dLoopY < 0.99; dLoopY += dLoopStepY) {

      double dX = dMinX + (dLoopX * (dMaxX - dMinX));
      double dY = dMinY + (dLoopY * (dMaxY - dMinY));

      // double dAX = dLoopX + dPhase / 12.0;
      double dAX = dLoopX + dPhase / 16.0;
      double dAY = dLoopY + (dPhase + 11.0) / 40.0;

      double dAlphaX = dAX * 100.0;
      double dAlphaY = dAY * 30.0;
      double dAlpha = ((sin(dAlphaX) + 1.0) / 2.0) * ((cos(dAlphaY) + 1.0) / 2.0);
      OpenGLHelpers::SetColorFull(1, 1, 1, dFTCAlpha * dAlpha);
      OpenGLHelpers::SetTexCoord(dLoopX, dLoopY);
      glVertex3f(dX, dY, 0);

      dAlphaX = dAX * 100.0;
      dAlphaY = (dAY + dLoopStepY) * 30.0;
      dAlpha = ((sin(dAlphaX) + 1.0) / 2.0) * ((cos(dAlphaY) + 1.0) / 2.0);
      OpenGLHelpers::SetColorFull(1, 1, 1, dFTCAlpha * dAlpha);
      OpenGLHelpers::SetTexCoord(dLoopX, dLoopY + dLoopStepY);
      glVertex3f(dX, dY + dStepY, 0);

      dAlphaX = (dAX + dLoopStepX) * 100.0;
      dAlphaY = (dAY + dLoopStepY) * 30.0;
      dAlpha = ((sin(dAlphaX) + 1.0) / 2.0) * ((cos(dAlphaY) + 1.0) / 2.0);
      OpenGLHelpers::SetColorFull(1, 1, 1, dFTCAlpha * dAlpha);
      OpenGLHelpers::SetTexCoord(dLoopX + dLoopStepX, dLoopY + dLoopStepY);
      glVertex3f(dX + dStepX, dY + dStepY, 0);

      dAlphaX = (dAX + dLoopStepX) * 100.0;
      dAlphaY = dAY * 30.0;
      dAlpha = ((sin(dAlphaX) + 1.0) / 2.0) * ((cos(dAlphaY) + 1.0) / 2.0);
      OpenGLHelpers::SetColorFull(1, 1, 1, dFTCAlpha * dAlpha);
      OpenGLHelpers::SetTexCoord(dLoopX + dLoopStepX, dLoopY);
      glVertex3f(dX + dStepX, dY, 0);
    }
  }

  glEnd();

  glPopMatrix();

  End2DRendering();

  //glFinish();

  SDL_GL_SwapWindow(window);

  if(dPhase > 16.0) {
    // SoundModule::StopIntroSound();
    m_pDrawFunction = &CPakoon1View::OnDrawCurrentMenu;
    m_pKeyDownFunction = &CPakoon1View::OnKeyDownCurrentMenu;
  }
}







//*************************************************************************************************
void CPakoon1View::PrepareReferenceTimes(BRaceRecord &raceRecord) {
  if(raceRecord.m_bValid) {
    int nRefK = 0;
    for(int i = 0; i < raceRecord.m_nNextSlot; ++i) {
      int nK = int(raceRecord.m_frames[i].m_vLocation.m_dY) / 1000;
      bool bLastSlot = (nK <= nRefK) && (i == (raceRecord.m_nNextSlot - 1));
      if((nK > nRefK) || bLastSlot) {
        if(bLastSlot) {
          ++nK;
        }
        nRefK = nK;
        double dFraction1 = (double(nRefK) * 1000.0) - raceRecord.m_frames[i - 1].m_vLocation.m_dY;
        double dFraction2 = raceRecord.m_frames[i].m_vLocation.m_dY - (double(nRefK) * 1000.0);
        double dSum = dFraction1 + dFraction2; 
        dFraction1 = dFraction1 / dSum;
        dFraction2 = dFraction2 / dSum;

        BGame::m_dRefTime[nK] = raceRecord.m_frames[i - 1].m_dTime * dFraction1 + 
                                raceRecord.m_frames[i].m_dTime * dFraction2;
        if(nRefK >= 6) {
          break;
        }
      }
    }
  }
}





//*************************************************************************************************
void CPakoon1View::OnDrawCurrentMenu() {

  if(BGame::m_bQuitPending) {
    //AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_FILE_CLOSE, 0);
    setExit();
  }

  if(!BGame::m_bMenusCreated) {
    BGame::SetupMenus();

    // Setup for first menu
    BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                 &CPakoon1View::OnKeyDownCurrentMenu);

    // Start menu music
    SoundModule::StartMenuMusic();
    SoundModule::SetMenuMusicVolume(int(double(BGame::m_nMusicVolume) / 100.0 * 255.0));
    SoundModule::SetVehicleSoundsVolume(int(double(BGame::m_nVehicleVolume) / 100.0 * 255.0));

    return;
  }

  // Check for highlight exit
  if(BGame::m_bMultiplayOn && 
     BGame::m_bOKToProceedInMultiplayMenu && 
     ((clock() - m_clockHighlightMenu) > CLOCKS_PER_SEC)) {
    m_clockHighlightMenu = 0;
    ReturnPressedOnCurrentMenu();
    return;
  }

  SDL_LockMutex(BGame::m_csMutex);

  // Check if we need to change display settings
  //FIXME
  /*if(!g_bResolutionChanged) {
    g_bResolutionChanged = true;
    DEVMODE devmode;
    EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);
    if((devmode.dmPelsWidth  != (DWORD)BGame::m_nDispWidth) || 
       (devmode.dmPelsHeight != (DWORD)BGame::m_nDispHeight) || 
       (devmode.dmBitsPerPel != (DWORD)BGame::m_nDispBits) ||
       (devmode.dmDisplayFrequency != (DWORD)BGame::m_nDispHz)) {
      devmode.dmPelsWidth = (DWORD)BGame::m_nDispWidth;
      devmode.dmPelsHeight = (DWORD)BGame::m_nDispHeight;
      devmode.dmBitsPerPel = (DWORD)BGame::m_nDispBits;
      devmode.dmDisplayFrequency = (DWORD)BGame::m_nDispHz;
      ChangeDisplaySettings(&devmode, CDS_FULLSCREEN);
      AfxGetMainWnd()->SetWindowPos(NULL, -2, -2, BGame::m_nDispWidth + 4, BGame::m_nDispHeight + 4, 0);
    }
  }*/

  CheckForGameStart();

  //*************************************************
  // Preparations stuff
  //*************************************************

  glClearColor(0.85, 0.85, 1, 0); /* For RGB-mode */

	//FIXME
  /*if(m_hCursor) {
    ::SetCursor(m_hCursor);
  }*/
  SDL_ShowCursor(0);

  // Init OpenGL
  glDrawBuffer(GL_BACK);

  // Reset OpenGL
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glViewport(0, 0, (GLint) m_rectWnd.w, (GLint) m_rectWnd.h);
  gluLookAt(0, -5, 0, 0, 0, 0, 0, 0, -1);

  GLfloat fLight1PositionG[ 4];
  fLight1PositionG[0] = (GLfloat) 1;
  fLight1PositionG[1] = (GLfloat) 1;
  fLight1PositionG[2] = (GLfloat) -1; // -1;
  fLight1PositionG[3] = (GLfloat) 0; /* w=0 -> directional light (not positional) */
  glLightfv( GL_LIGHT0, GL_POSITION, fLight1PositionG);                     

  bool bScrolling = false;
  double dScrollPhase = 0;
  clock_t clockNow = clock();
  if((clockNow - m_clockMenuScroll) < (CLOCKS_PER_SEC / 2)) {
    bScrolling = true;
    dScrollPhase = double(clockNow - m_clockMenuScroll) / double(CLOCKS_PER_SEC / 2);
    dScrollPhase = (1.0 + sin(((dScrollPhase * 2.0) - 1.0) * 3.1415926 * 0.5)) * 0.5;
  }

  //*************************************************
  // Draw menu
  //*************************************************

  // Setup 2D rendering
  Setup2DRendering();

  // Draw background
  OpenGLHelpers::SetColorFull(0, 0, 0.5, 1);
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glDisable(GL_TEXTURE_2D);

  double dPhase1 = sin(2.0 * 3.141592654 * double(SDL_GetTicks() % 11031) / 11030.0);
  double dPhase2 = sin(2.0 * 3.141592654 * double(SDL_GetTicks() % 17131) / 17130.0);
  double dPhase3 = sin(2.0 * 3.141592654 * double(SDL_GetTicks() % 15131) / 15130.0);
  double dPhase4 = sin(2.0 * 3.141592654 * double(SDL_GetTicks() % 12131) / 12130.0);

  glBegin(GL_TRIANGLE_STRIP);
  glColor4d(0.1, 0.15, 0.65, 1);
  glVertex3f(0, 0, 0);
  glColor4d(0, 0, 0, 1);
  glVertex3f(0, m_rectWnd.h, 0);
  glColor4d(0.1, 0.15, 0.65, 1);
  glVertex3f(m_rectWnd.w, 0, 0);
  glColor4d(0, 0, 0, 1);
  glVertex3f(m_rectWnd.w, m_rectWnd.h, 0);
  glEnd();

  glColor4d(1, 1, 1, 1);
  glDisable(GL_COLOR_MATERIAL);

  if(BGame::m_pMenuCurrent != &(BGame::m_menuCredits)) {
    // Draw copyright info
    BUI::TextRenderer()->StartRenderingText();
    string sText = "(C) Copyright 2003 Mikko Oksalahti. Visit www.pakoon.com for updates.";
    BUI::TextRenderer()->DrawSmallTextAt(m_rectWnd.w / 2, 20, sText, sText.length(), BTextRenderer::ALIGN_CENTER, 0.75, 0.75, 0.75, 1);
    BUI::TextRenderer()->StopRenderingText();
  }

  if(BGame::m_bMultiplayOn) {
    // Draw Multiplay indicator
    BUI::TextRenderer()->StartRenderingText();
    BUI::TextRenderer()->DrawSmallTextAt(10, m_rectWnd.h - 128 - 20, "multiplay mode", 14, BTextRenderer::ALIGN_LEFT, 1, 0.75, 0.25, 1);
    for(int i = 0; i < BGame::m_nRemotePlayers; ++i) {

      string sExtra = "";

      switch(BGame::m_remotePlayer[i].m_state) {
        case BRemotePlayer::WANTS_TO_SELECT_NEW_RACE:
        case BRemotePlayer::PREPARING_TO_RACE:
          sExtra = "";
          break;
        case BRemotePlayer::WAITING_FOR_RACE:
          sExtra = "(Waiting to start another race)";
          break;
        case BRemotePlayer::RACING:
          sExtra = "(Racing)";
          break;
        case BRemotePlayer::FINISHED:
          if(BGame::m_remotePlayer[i].m_nRacePosition >= 4) {
			  stringstream val;
			  val << "(Finished " << BGame::m_remotePlayer[i].m_nRacePosition << "th)";
			  sExtra = val.str();
          } else if(BGame::m_remotePlayer[i].m_nRacePosition == 3) {
            sExtra = "(Finished 3rd)";
          } else if(BGame::m_remotePlayer[i].m_nRacePosition == 2) {
            sExtra = "(Finished 2nd)";
          } else if(BGame::m_remotePlayer[i].m_nRacePosition == 1) {
            sExtra = "(Finished 1st!)";
          }
          break;
        case BRemotePlayer::MISSED_GOAL:
          sExtra = "(Missed goal)";
          break;
        case BRemotePlayer::MISSED_POLE:
          sExtra = "(Missed a pole)";
          break;
      }

      double dR, dG, dB;
      BGame::GetMultiplayerColor(i, dR, dG, dB);
      stringstream sPlayer;
      sPlayer << "Player " << i + 1 << ": " << BGame::m_remotePlayer[i].m_sName << " " << sExtra;

      BUI::TextRenderer()->DrawSmallTextAt(20, m_rectWnd.h - 128 - 40 - 20 * i, sPlayer.str(), sPlayer.str().length(), BTextRenderer::ALIGN_LEFT, dR, dG, dB, 1);
    }

    BUI::TextRenderer()->StopRenderingText();
  }

  if(bScrolling) {
    DrawMenuTitle(BGame::m_pMenuPrevious, 1.0 - dScrollPhase, true);
    DrawMenuTitle(BGame::m_pMenuCurrent, dScrollPhase, false);
  } else {
    DrawMenuTitle(BGame::m_pMenuCurrent, 1.0, true);
  }

  glPushMatrix();
  glTranslatef(0, -64, 0);
  if(bScrolling) {
    // Draw previous and new menu scrolling
    glPushMatrix();
    if(m_scrollDir == SCROLL_RIGHT) {
      glTranslated(-int(double(m_rectWnd.w * dScrollPhase)), 0, 0);
    } else if(m_scrollDir == SCROLL_LEFT) {
      glTranslated(+int(double(m_rectWnd.w * dScrollPhase)), 0, 0);
    } else if(m_scrollDir == SCROLL_UP) {
      glTranslated(0, -int(double(m_rectWnd.h * dScrollPhase)), 0);
    } else if(m_scrollDir == SCROLL_DOWN) {
      glTranslated(0, +int(double(m_rectWnd.h * dScrollPhase)), 0);
    }
    DrawMenu(BGame::m_pMenuPrevious);
    glPopMatrix();

    glPushMatrix();
    if(m_scrollDir == SCROLL_RIGHT) {
      glTranslated(int(double(m_rectWnd.w * (1.0 - dScrollPhase))), 0, 0);
    } else if(m_scrollDir == SCROLL_LEFT) {
      glTranslated(-int(double(m_rectWnd.w * (1.0 - dScrollPhase))), 0, 0);
    } else if(m_scrollDir == SCROLL_UP) {
      glTranslated(0, +int(double(m_rectWnd.h * (1.0 - dScrollPhase))), 0);
    } else if(m_scrollDir == SCROLL_DOWN) {
      glTranslated(0, -int(double(m_rectWnd.h * (1.0 - dScrollPhase))), 0);
    }
    DrawMenu(BGame::m_pMenuCurrent);
    glPopMatrix();
  } else {
    DrawMenu(BGame::m_pMenuCurrent);
  }
  glPopMatrix();

  if(BGame::m_bGameReadyToStart) {
    // Draw loading mark
    double dCharWidth = BUI::TextRenderer()->GetCharWidth();
    double dCharHeight = BUI::TextRenderer()->GetCharHeight();
    BGame::m_bGameLoading = true;
    glPushMatrix();
    glTranslatef(m_rectWnd.w / 2, m_rectWnd.h / 2, 0);
    OpenGLHelpers::SetColorFull(1, 1, 1, 1);
    // DrawPanel(dCharWidth * 10, dCharHeight * 2.5, 1.0 * 0.3, 0.5 * 0.3, 0.0 * 0.3, 0.95);
    DrawPanel(dCharWidth * 10, dCharHeight * 2.5, 0.15, 0.15, 0.15, 0.9);
    BUI::TextRenderer()->StartRenderingText();
    BUI::TextRenderer()->DrawTextAt(2,
                                    -2,
                                    "LOADING...",
                                    BTextRenderer::ALIGN_CENTER,
                                    0,
                                    0,
                                    0,
                                    0.7);
    BUI::TextRenderer()->DrawTextAt(0,
                                    0,
                                    "LOADING...",
                                    BTextRenderer::ALIGN_CENTER,
                                    1,
                                    0.5,
                                    0,
                                    1);
    BUI::TextRenderer()->StopRenderingText();
    glPopMatrix();
  }

  DrawMultiplayMessages();

  //*************************************************
  // Finish draw
  //*************************************************

  //glFinish();

  SDL_GL_SwapWindow(window);

  SDL_UnlockMutex(BGame::m_csMutex);

  // And again and again and...
  if((BGame::m_pMenuCurrent == &(BGame::m_menuPrecachingTerrain)) && !BGame::m_bMultiProcessor) {
    SDL_Delay(1);
  }
}



//*************************************************************************************************
void CPakoon1View::CheckForGameStart() {
  // Cater for Precaching Terrain menu special needs
  if(BGame::m_bGameLoading) {
    // Initiate game load
    BGame::GetSimulation()->GetScene()->LoadSceneFromFile(BGame::m_sScene);
    BGame::SetupScene();
    BGame::GetSimulation()->GetTerrain()->StartUsingScene(BGame::GetSimulation()->GetScene()->m_sName, 
                                                          BGame::GetSimulation()->GetScene()->m_vOrigin, 
                                                          BGame::GetSimulation()->GetScene()->m_dGroundTextureScaler1, 
                                                          BGame::GetSimulation()->GetScene()->m_dGroundTextureScaler2);
    // Prepare to enter game

    if(BGame::m_bMultiplayOn) {
      for(int i = 0; i < BGame::m_nRemotePlayers; ++i) {
        BGame::m_remotePlayer[i].m_bReadyToStart = false;
      }
      BGame::m_bMultiplayRaceStarter = false;
      BGame::m_clockMultiRaceStarter = 0;
      BGame::m_remotePlayer[BGame::GetMyPlace()].m_state = BRemotePlayer::WAITING_FOR_RACE;
      BGame::BroadcastStateChange();
    } else {
      BGame::m_nRemotePlayers = 0;
    }

    BGame::m_bRaceStarted  = false;
    BGame::m_bRaceFinished = false;
    if(BGame::m_gameMode == BGame::AIRTIME) {
      BGame::m_dRaceTime = BGame::GetSimulation()->GetScene()->m_dAirTimeMaxSec;
    } else {
      BGame::m_dRaceTime = 0.0;
    }
    BGame::m_dAirTime  = 0;
    BGame::m_bSlalomPolesVisualOK = false;
    BGame::m_bForceBreak = false;
    BGame::m_dLiftStarted = 0;
    BGame::m_cOnScreenInfo &= BGame::FPS; // Preserve fps setting

    BGame::GetPlayer()->LoadStateFile();
    BGame::GetSimulation()->GetScene()->PlaceTerrainObjects();
    BGame::GetSimulation()->GetTerrain()->CreateTerrainDisplayLists();
    BGame::GetSimulation()->GetVehicle()->LoadVehicleFromFile(BGame::m_sVehicle, true);
    BGame::GetSimulation()->GetVehicle()->m_dFuel = 100; // BGame::GetPlayer()->m_dFuel;
    BGame::GetSimulation()->GetVehicle()->PreProcessVisualization();

    BGame::GetSimulation()->GetCamera()->m_dFollowHeight = -3.0;

    // initialize multiplay remote cars
    if(BGame::m_bMultiplayOn) {

      // First setup local vehicle

      {
        //CFile fileTmp(BGame::m_sVehicle, CFile::modeRead | CFile::shareDenyNone); //FIXME
        BGame::m_remotePlayer[BGame::GetMyPlace()].m_pVehicle = BGame::GetSimulation()->GetVehicle();
        //BGame::m_remotePlayer[BGame::GetMyPlace()].m_sVehicleFilename = fileTmp.GetFileName(); //FIXME
        BGame::m_remotePlayer[BGame::GetMyPlace()].m_bVehicleReused = true;
      }

      // Load / Reuse remote cars

      for(int nRemote = 0; nRemote < BGame::m_nRemotePlayers; ++nRemote) {
        if(BGame::m_remotePlayer[nRemote].m_bSelf) {
          continue;
        }

        // Check if this remote car has been loaded already
        bool bReused = false;

        for(int i = 0; i < BGame::m_nRemotePlayers; ++i) {
          if((i == nRemote) || (BGame::m_remotePlayer[nRemote].m_bSelf)) {
            continue;
          }
          if(BGame::m_remotePlayer[i].m_pVehicle && 
             ((BGame::m_remotePlayer[nRemote].m_sVehicleFilename.compare(BGame::m_remotePlayer[i].m_sVehicleFilename) == 0))) {
            bReused = true;
            BGame::m_remotePlayer[nRemote].m_pVehicle = BGame::m_remotePlayer[i].m_pVehicle;
            BGame::m_remotePlayer[nRemote].m_bVehicleReused = true;
            break;
          }
        }

        if(!bReused) {
          // Load the remote vehicle

          if(BGame::m_remotePlayer[nRemote].m_pVehicle && 
             !BGame::m_remotePlayer[nRemote].m_bVehicleReused) {
            delete BGame::m_remotePlayer[nRemote].m_pVehicle;
          }

          BGame::m_remotePlayer[nRemote].m_bVehicleReused = false;

          BGame::m_remotePlayer[nRemote].m_pVehicle = new BVehicle();

          string sTmp = BGame::m_remotePlayer[nRemote].m_sVehicleFilename;

          BGame::m_remotePlayer[nRemote].m_pVehicle->LoadVehicleFromFile(sTmp, true);
          BGame::m_remotePlayer[nRemote].m_pVehicle->m_dFuel = 100; // BGame::GetPlayer()->m_dFuel;
          BGame::m_remotePlayer[nRemote].m_pVehicle->PreProcessVisualization();
        }
      }
    }

    // Initialize reference times
    BGame::m_dRefTime[0] = -1.0;
    BGame::m_dRefTime[1] = -1.0;
    BGame::m_dRefTime[2] = -1.0;
    BGame::m_dRefTime[3] = -1.0;
    BGame::m_dRefTime[4] = -1.0;
    BGame::m_dRefTime[5] = -1.0;
    BGame::m_dRefTime[6] = -1.0;
    BGame::m_nRefK = 0;
    BScene *pScene = BGame::GetSimulation()->GetScene();
    if(BGame::m_gameMode == BGame::SLALOM) {
      PrepareReferenceTimes(pScene->m_raceRecordSlalomTime);
    } else if(BGame::m_gameMode == BGame::SPEEDRACE) {
      PrepareReferenceTimes(pScene->m_raceRecordBestTime);
    }

    // (Dunno why, but this needs to be done again here)
    for(int o = 0; o < BGame::GetSimulation()->GetScene()->m_nObjects; ++o) {
      BGame::GetSimulation()->GetScene()->m_pObjects[o].RecreateShadow();
    }

    BMessages::RemoveAll();

    // Move vehicle to start location
    BVector vStartLocation = BGame::GetSimulation()->GetScene()->m_vStartLocation;
    BVector vSceneLoc; 
    if(BGame::GetPlayer()->LoadCurrentSceneInfo(vSceneLoc)) {
      vStartLocation = vSceneLoc;
    }
    BGame::GetSimulation()->UpdateCarLocation();
    BVector vLoc = BGame::GetSimulation()->GetVehicle()->m_vLocation;
    BGame::GetSimulation()->GetVehicle()->Move(vStartLocation - vLoc);

    if(BGame::m_bMultiplayOn) {
      // Position the car according to its place in the multiplay list
      switch(BGame::GetMyPlace()) {
        case 1:
          BGame::GetSimulation()->GetVehicle()->Move(BVector(5, 0, 0));
          break;
        case 2:
          BGame::GetSimulation()->GetVehicle()->Move(BVector(-5, 0, 0));
          break;
        case 3:
          BGame::GetSimulation()->GetVehicle()->Move(BVector(10, 0, 0));
          break;
      }
    }

    BGame::GetSimulation()->UpdateCar();

    // Make terrain valid so that the first simulation will work
    BGame::GetSimulation()->GetTerrain()->MakeTerrainValid(BGame::GetSimulation()->GetVehicle()->m_vLocation,
                                                           BGame::GetSimulation()->GetCamera()->m_vLocation,
                                                           BGame::GetSimulation()->GetCamera()->m_orientation.m_vForward,
                                                           true, 
                                                           false, 
                                                           false);
    BVector vOnGround, vNormal;
    BGame::GetSimulation()->EnsureVehicleIsOverGround();
    BGame::GetSimulation()->GetCamera()->m_locMode = BCamera::FOLLOW;
    BGame::GetSimulation()->GetCamera()->m_vLocation = BGame::GetSimulation()->GetVehicle()->m_vLocation + BVector(-20, -10, 0);
    BGame::GetSimulation()->GetCamera()->m_orientation.m_vForward = BGame::GetSimulation()->GetVehicle()->m_vLocation - BGame::GetSimulation()->GetCamera()->m_vLocation;
    BGame::GetSimulation()->GetCamera()->m_orientation.m_vUp = BVector(0, 0, -1);
    BGame::GetSimulation()->GetCamera()->m_orientation.m_vRight = BGame::GetSimulation()->GetCamera()->m_orientation.m_vUp.CrossProduct(BGame::GetSimulation()->GetCamera()->m_orientation.m_vForward);
    // Make terrain valid so that the first simulation will work
    BGame::GetSimulation()->GetTerrain()->MakeTerrainValid(BGame::GetSimulation()->GetVehicle()->m_vLocation,
                                                           BGame::GetSimulation()->GetCamera()->m_vLocation,
                                                           BGame::GetSimulation()->GetCamera()->m_orientation.m_vForward,
                                                           true, 
                                                           false, 
                                                           false);

    if(BGame::m_bMultiplayOn) {
      // report car size to other remote players
      BGame::BroadcastCarSize();
    }

    // Switch to game mode

    BGame::m_bGameLoading = false;
    BGame::m_bMenuMode = false;
    m_pDrawFunction = &CPakoon1View::OnDrawGame;
    m_pKeyDownFunction = &CPakoon1View::OnKeyDownGame;
    m_game.m_bFadingIn = true;
    m_game.m_clockFadeStart = clock();
    SoundModule::StopMenuMusic();
    SoundModule::StartGameMusic();
    SoundModule::SetGameMusicVolume(int(double(BGame::m_nMusicVolume) / 100.0 * 255.0));
    SoundModule::SetVehicleSoundsVolume(int(double(BGame::m_nVehicleVolume) / 100.0 * 255.0));
    m_bInitClock = true;

    if(BGame::m_bMultiplayOn) {
      BGame::m_bForceBreak = true;
    }

    SDL_ShowCursor(0);
  }
}



//*************************************************************************************************
void CPakoon1View::DrawMenuTitle(BMenu *pMenu, double dAlpha, bool bFirstTime) {
  // Draw menu title
  if(pMenu->m_nTitleWidth > 0) {

    if(bFirstTime) {

      // Draw title background
     
      OpenGLHelpers::SwitchToTexture(0);
      BTextures::Use(BTextures::MENU_TITLES);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // not mipmapped

      double dScaler = double(m_rectWnd.h) / 512.0;

      glBegin(GL_QUADS);

      OpenGLHelpers::SetColorFull(1, 1, 1, 1);

      OpenGLHelpers::SetTexCoord(0, 0);
      glVertex3f(0, m_rectWnd.h - 128.0, 0);
      OpenGLHelpers::SetTexCoord(0, 1);
      glVertex3f(0, m_rectWnd.h, 0);

      OpenGLHelpers::SetColorFull(1, 1, 1, 0.75);

      OpenGLHelpers::SetTexCoord(1, 1);
      glVertex3f(m_rectWnd.w, m_rectWnd.h, 0);
      OpenGLHelpers::SetTexCoord(1, 0);
      glVertex3f(m_rectWnd.w, m_rectWnd.h - 128.0, 0);

      glEnd();

      glDisable(GL_TEXTURE_2D);
      OpenGLHelpers::SetColorFull(1, 1, 1, 0.5);
      glBegin(GL_LINES);
      glVertex3f(0, m_rectWnd.h - 128.0, 0);
      glVertex3f(m_rectWnd.w, m_rectWnd.h - 128.0, 0);
      glEnd();
    }

    if(pMenu != &(BGame::m_menuMain)) {
      glPushMatrix();
      BUI::TextRenderer()->StartRenderingText();
      BUI::TextRenderer()->DrawTextAt(m_rectWnd.w / 2 + 2,
                                      m_rectWnd.h - 2 - 64,
                                      pMenu->m_sName,
                                      BTextRenderer::ALIGN_CENTER,
                                      0,
                                      0,
                                      0,
                                      0.7 * dAlpha);
      BUI::TextRenderer()->DrawTextAt(m_rectWnd.w / 2,
                                      m_rectWnd.h - 64,
                                      pMenu->m_sName,
                                      BTextRenderer::ALIGN_CENTER,
                                      1,
                                      0.75,
                                      0.25,
                                      dAlpha);
      BUI::TextRenderer()->StopRenderingText();
      glPopMatrix();
    } else {
      // For main menu, display the Pakoon2 logo
      OpenGLHelpers::SwitchToTexture(0);
      BTextures::Use(BTextures::LOGO);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // not mipmapped
      OpenGLHelpers::SetColorFull(1, 1, 1, dAlpha);

      glBegin(GL_QUADS);

      OpenGLHelpers::SetTexCoord(0, 0);
      glVertex3f(m_rectWnd.w / 2 - 256, m_rectWnd.h - 128, 0);
      OpenGLHelpers::SetTexCoord(0, 1);
      glVertex3f(m_rectWnd.w / 2 - 256, m_rectWnd.h, 0);
      OpenGLHelpers::SetTexCoord(1, 1);
      glVertex3f(m_rectWnd.w / 2 + 256, m_rectWnd.h, 0);
      OpenGLHelpers::SetTexCoord(1, 0);
      glVertex3f(m_rectWnd.w / 2 + 256, m_rectWnd.h - 128, 0);

      glEnd();
    }
  }
}


//*************************************************************************************************
void CPakoon1View::DrawHiscores(BMenu *pMenu) {
  // Draw lists of best times for Speedrace, Slalom and Airtime
  // OpenGLHelpers::SetColorFull(1, 0.5, 0.5, 1);

  double dCharHeight = BUI::TextRenderer()->GetCharHeight();

  BUI::TextRenderer()->StartRenderingText();
  string sText = "SPEEDRACE";
  BUI::TextRenderer()->DrawSmallTextAt(m_rectWnd.w / 2, 
                                       m_rectWnd.h / 2 + dCharHeight * (1.0 + (double(BGame::m_listHSSpeedrace.GetNofItems()) / 2.0)), 
                                       sText, 
                                       sText.length(), 
                                       BTextRenderer::ALIGN_LEFT, 
                                       1, 0.25, 0.25, 
                                       1);
  sText = "SLALOM";
  BUI::TextRenderer()->DrawSmallTextAt(m_rectWnd.w / 2 + 120 + 13,
                                       m_rectWnd.h / 2 + dCharHeight * (1.0 + (double(BGame::m_listHSSpeedrace.GetNofItems()) / 2.0)), 
                                       sText, 
                                       sText.length(), 
                                       BTextRenderer::ALIGN_LEFT, 
                                       0.25, 1, 0.25, 
                                       1);
  sText = "AIRTIME";
  BUI::TextRenderer()->DrawSmallTextAt(m_rectWnd.w / 2 + 240 + 8,
                                       m_rectWnd.h / 2 + dCharHeight * (1.0 + (double(BGame::m_listHSSpeedrace.GetNofItems()) / 2.0)), 
                                       sText, 
                                       sText.length(), 
                                       BTextRenderer::ALIGN_LEFT, 
                                       0.25, 0.25, 1, 
                                       1);
  BUI::TextRenderer()->StopRenderingText();

  BGame::m_listHSSpeedrace.DrawAt(m_rectWnd.w / 2.0, 
                                  m_rectWnd.h / 2, 
                                  BTextRenderer::ALIGN_LEFT, 
                                  1, 
                                  0.5, 
                                  0.5, 
                                  false);
  BGame::m_listHSSlalom.DrawAt(m_rectWnd.w / 2.0 + 120, 
                               m_rectWnd.h / 2, 
                               BTextRenderer::ALIGN_LEFT, 
                               0.5, 
                               1, 
                               0.5, 
                               false);
  BGame::m_listHSAirtime.DrawAt(m_rectWnd.w / 2.0 + 240, 
                                m_rectWnd.h / 2, 
                                BTextRenderer::ALIGN_LEFT, 
                                0.5, 
                                0.5, 
                                1, 
                                false);
}


//*************************************************************************************************
void CPakoon1View::DrawEarth(BMenu *pMenu) {
  glPushMatrix();
  GLdouble mtxCurr[16];
  glGetDoublev(GL_MODELVIEW_MATRIX, mtxCurr);
  
  // Draw Earth on the Choose Scene menu

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glScaled(-1.0, 1.0, 1.0);

  double dWidth = m_rectWnd.w - m_rectWnd.w / 3;

  double dAspect = dWidth / (double) m_rectWnd.h;
  gluPerspective(70.0, dAspect, 1.0f, 2000.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  int nScrollXOffset = int(mtxCurr[12]);
  int nScrollYOffset = int(mtxCurr[13]);

  glViewport(nScrollXOffset + (GLint) m_rectWnd.w / 3, 
             nScrollYOffset, 
             (GLint) dWidth, 
             (GLint) m_rectWnd.h);

  static int nPrevSel = -1;
  static BVector vPrevCamera(0, -1000, 0);
  static BVector vNewCamera(0, -1000, 0);
  static BVector vCamera(0, -1000, 0);
  static clock_t clockPrevStart = 0;
  clock_t clockNow = clock();

  string sTmp;
  int nSelected = pMenu->m_listMenu.GetSelectedItem(sTmp);
  if(nSelected != nPrevSel) {
    nPrevSel = nSelected;

    // Setup new camera location
    double dX = double(pMenu->m_items[nSelected].m_nValue);
    double dY = double(pMenu->m_items[nSelected].m_nValue2);
    vNewCamera.m_dX = cos(dX / 1024.0 * 2.0 * g_cdPI) * cos((256.0 - dY) / 256.0 * g_cdPI / 2.0) * 1000.0;
    vNewCamera.m_dY = sin(dX / 1024.0 * 2.0 * g_cdPI) * cos((256.0 - dY) / 256.0 * g_cdPI / 2.0) * 1000.0;
    vNewCamera.m_dZ = sin((256.0 - dY) / 256.0 * g_cdPI / 2.0) * -1000.0;
    vPrevCamera = vCamera;
    clockPrevStart = clock();
  }

  vCamera = vNewCamera;
  if((clockNow - clockPrevStart) < (CLOCKS_PER_SEC / 2)) {
    // We are going towards the new location
    double dPhase = double(clockNow - clockPrevStart) / double(CLOCKS_PER_SEC / 2);
    if(dPhase > 1.0) {
      dPhase = 1.0;
    }
    dPhase = (1.0 + sin(((dPhase * 2.0) - 1.0) * 3.1415926 * 0.5)) * 0.5;
    vCamera = vPrevCamera * (1.0 - dPhase) + vNewCamera * dPhase;
    vCamera.ToUnitLength();
    vCamera = vCamera * 1000.0;
  } else {
    // We have reached the new location
    vPrevCamera = vNewCamera;
  }

  BVector vForward = vCamera;
  vForward.ToUnitLength();
  BVector vRight = vForward.CrossProduct(BVector(0, 0, -1));
  vRight.ToUnitLength();
  BVector vUp = vRight.CrossProduct(vForward);
  vUp.ToUnitLength();
  vRight = vForward.CrossProduct(vUp);
  vRight.ToUnitLength();

  gluLookAt(vCamera.m_dX, 
            vCamera.m_dY, 
            vCamera.m_dZ,
            0,
            0, 
            0, 
            0, 
            0, 
            -1);

  GLfloat fLight1AmbientG[ 4];
  GLfloat fLight1DiffuseG[ 4];
  GLfloat fLight1SpecularG[ 4];

  fLight1AmbientG[0] = 0.1f;
  fLight1AmbientG[1] = 0.1f;
  fLight1AmbientG[2] = 0.1f;
  fLight1AmbientG[3] = 1;

  fLight1DiffuseG[0] = 1;
  fLight1DiffuseG[1] = 1;
  fLight1DiffuseG[2] = 1;
  fLight1DiffuseG[3] = 1;

  fLight1SpecularG[0] = 1;
  fLight1SpecularG[1] = 1;
  fLight1SpecularG[2] = 1;
  fLight1SpecularG[3] = 1;

  glLightfv( GL_LIGHT0, GL_AMBIENT,  fLight1AmbientG);
  glLightfv( GL_LIGHT0, GL_DIFFUSE,  fLight1DiffuseG);
  glLightfv( GL_LIGHT0, GL_SPECULAR, fLight1SpecularG);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, (GLfloat) 1.0);

  // OpenGLHelpers::SetDefaultLighting();
  // OpenGLHelpers::SetStrongLighting();

  BVector vLight = vCamera + vRight * -1000.0 + vUp * 1000.0;

  GLfloat fLight1PositionG[ 4];
  fLight1PositionG[0] = (GLfloat) vLight.m_dX;
  fLight1PositionG[1] = (GLfloat) vLight.m_dY;
  fLight1PositionG[2] = (GLfloat) vLight.m_dZ;
  fLight1PositionG[3] = (GLfloat) 1; /* w=0 -> directional light (not positional) */
  glLightfv( GL_LIGHT0, GL_POSITION, fLight1PositionG);                     

  OpenGLHelpers::SetColorFull(1, 1, 1, 1);
  glDisable(GL_FOG);
  glEnable(GL_CULL_FACE);
  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  OpenGLHelpers::SwitchToTexture(0);
  BTextures::Use(BTextures::EARTH);

  glPushMatrix();

  DrawTexturedSphere();

  glEnable(GL_DEPTH_TEST);

  fLight1AmbientG[0] = 0;
  fLight1AmbientG[1] = 0;
  fLight1AmbientG[2] = 0;
  fLight1AmbientG[3] = 1;

  fLight1DiffuseG[0] = 0;
  fLight1DiffuseG[1] = 0;
  fLight1DiffuseG[2] = 0;
  fLight1DiffuseG[3] = 1;

  fLight1SpecularG[0] = 1;
  fLight1SpecularG[1] = 1;
  fLight1SpecularG[2] = 1;
  fLight1SpecularG[3] = 1;

  glLightfv( GL_LIGHT0, GL_AMBIENT,  fLight1AmbientG);
  glLightfv( GL_LIGHT0, GL_DIFFUSE,  fLight1DiffuseG);
  glLightfv( GL_LIGHT0, GL_SPECULAR, fLight1SpecularG);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, (GLfloat) 5.0);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  OpenGLHelpers::SetColorFull(0.7, 0.7, 1, 0.5);
  OpenGLHelpers::SwitchToTexture(0);
  BTextures::Use(BTextures::EARTH_SPECULAR_MAP);

  DrawTexturedSphere();

  // Draw location beacon
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  fLight1AmbientG[0] = 0.1f;
  fLight1AmbientG[1] = 0.1f;
  fLight1AmbientG[2] = 0.1f;
  fLight1AmbientG[3] = 1;

  fLight1DiffuseG[0] = 1;
  fLight1DiffuseG[1] = 1;
  fLight1DiffuseG[2] = 1;
  fLight1DiffuseG[3] = 1;

  fLight1SpecularG[0] = 1;
  fLight1SpecularG[1] = 1;
  fLight1SpecularG[2] = 1;
  fLight1SpecularG[3] = 1;

  glLightfv( GL_LIGHT0, GL_AMBIENT,  fLight1AmbientG);
  glLightfv( GL_LIGHT0, GL_DIFFUSE,  fLight1DiffuseG);
  glLightfv( GL_LIGHT0, GL_SPECULAR, fLight1SpecularG);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, (GLfloat) 1.0);

  BVector vToCamera = vNewCamera;
  vToCamera.ToUnitLength();
  vToCamera = vToCamera * 300.0;

  double dAlpha = BGame::GetSmoothAlpha(); // fabs(double(clock() % CLOCKS_PER_SEC) - (double(CLOCKS_PER_SEC) / 2.0)) / (double(CLOCKS_PER_SEC) / 2.0);

  dAlpha = 0.5 + dAlpha * 0.5;
  glDisable(GL_CULL_FACE);
  glDisable(GL_TEXTURE_2D);
  OpenGLHelpers::SetColorFull(1, 0, 0, dAlpha);
  GLUquadricObj* pQuad = gluNewQuadric();
  glTranslatef(vToCamera.m_dX, vToCamera.m_dY, vToCamera.m_dZ);
  gluSphere(pQuad, 10, 20, 20);
  gluDeleteQuadric(pQuad);
  
  glPopMatrix();

  Setup2DRendering();

  glPopMatrix();
}


//*************************************************************************************************
void CPakoon1View::DrawMenu(BMenu *pMenu) {
  if(pMenu) {

    if(pMenu->m_type == BMenu::CHOOSE_SCENE) {
      DrawEarth(pMenu);
    }
    if(pMenu->m_type == BMenu::HISCORES) {
      DrawHiscores(pMenu);
    }

    double dCharHeight = BUI::TextRenderer()->GetCharHeight();
    double dCharWidth  = BUI::TextRenderer()->GetCharWidth();

    // Draw menu contents
    if(pMenu->m_listMenu.GetNofItems()) {
      int i;

      if(pMenu->m_type == BMenu::CHOOSE_VEHICLE) {
        for(i = 0; i < pMenu->m_nItems; ++i) {
          // Check for owned vehicles
          string sVehicle = ">" + pMenu->m_items[i].m_sText + "<";
          if(BGame::GetPlayer()->m_sValidVehicles.find(sVehicle) == -1) {
            pMenu->m_items[i].m_bDisabled = false; // allow all vehicles
          } else {
            pMenu->m_items[i].m_bDisabled = false;
          }
        }
      }

      // Draw menu's main list
      pMenu->m_listMenu.DrawAt(m_rectWnd.w / 2 - int(double(m_rectWnd.w) * pMenu->m_listMenu.m_dOffsetToLeft), 
                               m_rectWnd.h / 2, 
                               pMenu->m_align, 
                               1, 
                               1, 
                               1, 
                               false,
                               false,
                               (pMenu->m_listMenu.m_nSelected != -1) ? 
                               !(pMenu->m_items[pMenu->m_listMenu.m_nSelected].m_bOpen) : true,
                               pMenu);

      // Draw remote multiplayer selections
      if(BGame::m_bMultiplayOn) {
        DrawMultiplayMenuStuff(pMenu, 
                               m_rectWnd.w / 2 - int(double(m_rectWnd.w) * pMenu->m_listMenu.m_dOffsetToLeft), 
                               m_rectWnd.h / 2);
      }

      // If there are menu items that have an associated component,
      // draw them also
      for(i = 0; i < pMenu->m_nItems; ++i) {
        if(pMenu->m_items[i].m_type == BMenuItem::STRING_FROM_LIST) {
          // Draw list string
          DrawMenuItemTextAtRelPos(m_rectWnd.w / 2, 
                                   m_rectWnd.h / 2, 
                                   pMenu->m_nItems,
                                   i, 
                                   &(pMenu->m_items[i]));
        } else if(pMenu->m_items[i].m_type == BMenuItem::SLIDER) {
          // Draw slider
          DrawMenuItemSliderAtRelPos(m_rectWnd.w / 2, 
                                     m_rectWnd.h / 2, 
                                     pMenu->m_nItems,
                                     i, 
                                     &(pMenu->m_items[i]));
        } else if(pMenu->m_items[i].m_type == BMenuItem::EDITBOX) {
          // Draw slider
          DrawMenuItemEditBoxAtRelPos(m_rectWnd.w / 2, 
                                      m_rectWnd.h / 2, 
                                      pMenu->m_nItems,
                                      i, 
                                      &(pMenu->m_items[i]));
        }
      }

      // Draw open subitem
      for(i = 0; i < pMenu->m_nItems; ++i) {
        if(pMenu->m_items[i].m_bOpen) {
          if(pMenu->m_items[i].m_type == BMenuItem::STRING_FROM_LIST) {
            // Draw submenu
            pMenu->m_items[i].m_listMenu.DrawAt(m_rectWnd.w / 2 + 35, 
                                                m_rectWnd.h / 2 + double(i) * -dCharHeight + (dCharHeight * double(pMenu->m_nItems)) / 2.0, 
                                                BTextRenderer::ALIGN_LEFT,
                                                1, 
                                                1, 
                                                1,
                                                true,
                                                true,
                                                true,
                                                0,
                                                false);
          }
        }
      }

      // Draw associated image, if one is available
      if(pMenu->m_type != BMenu::CHOOSE_SCENE) {
        string sTmp;
        int nSelected = pMenu->m_listMenu.GetSelectedItem(sTmp);
        if((nSelected != -1) && (pMenu->m_items[nSelected].m_nAssocImage != -1)) {

          glBlendFunc(GL_SRC_ALPHA, GL_ONE);
          glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

          OpenGLHelpers::SetColorFull(1, 1, 1, 1);
          OpenGLHelpers::SwitchToTexture(0);
          BTextures::Use(pMenu->m_items[nSelected].m_nAssocImage);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // not mipmapped
          glPushMatrix();
          glTranslated(m_rectWnd.w / 2 + m_rectWnd.w / 40,
                       m_rectWnd.h / 2, 
                       0);
          glBegin(GL_TRIANGLE_STRIP);
          OpenGLHelpers::SetTexCoord(1.0/256.0, 1.0/256.0);
          glVertex3f(0, -127, 0);
          OpenGLHelpers::SetTexCoord(1.0/256.0, 255.0/256.0);
          glVertex3f(0, 128, 0);
          OpenGLHelpers::SetTexCoord(255.0/256.0, 1.0/256.0);
          glVertex3f(255, -127, 0);
          OpenGLHelpers::SetTexCoord(255.0/256.0, 255.0/256.0);
          glVertex3f(255, 128, 0); 
          glEnd();

          // If disabled, draw veil over image
          if(pMenu->m_items[nSelected].m_bDisabled) {
            OpenGLHelpers::SetColorFull(0, 0, 0, 0.5);
            glDisable(GL_TEXTURE_2D);
            glBegin(GL_LINES);
            for(i = 1; i < 256; i += 2) {
              glVertex3f(0, -128 + i, 0);
              glVertex3f(256, -128 + i, 0);
            }
            OpenGLHelpers::SetColorFull(0, 0, 0, 0.25);
            for(i = 0; i < 256; i += 2) {
              glVertex3f(0, -128 + i, 0);
              glVertex3f(256, -128 + i, 0);
            }
            glEnd();
            OpenGLHelpers::SetColorFull(1, 1, 1, 1);
          }

          glPopMatrix();
        }
      }
    }

    // Draw credits images with fade, if showing credits
    if(pMenu->m_type == BMenu::CREDITS) {
      // Find the active item(s)
      int    i;
      int    nItems = 0;
      int    nItemsToDraw[2];
      double dItemsAlpha[2];
      nItemsToDraw[0] = -1;
      nItemsToDraw[1] = -1;
      clock_t clockNow = clock();
      double dCurSecond = double(clockNow - pMenu->m_clockStarted) / double(CLOCKS_PER_SEC);

      if((BGame::m_pMenuCurrent == &(BGame::m_menuCredits)) && 
         (dCurSecond > (1 + pMenu->m_items[pMenu->m_nItems - 1].m_nValue + pMenu->m_items[pMenu->m_nItems - 1].m_nValue2))) {
        // Return to main menu
        BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
        // BGame::m_pMenuPrevious = &(BGame::m_menuCredits);
        BGame::m_pMenuCurrent = &(BGame::m_menuMain);
        BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                     &CPakoon1View::OnKeyDownCurrentMenu);
        StartMenuScroll(SCROLL_UP);
        return;
      } else {
        for(i = 0; i < pMenu->m_nItems; ++i) {
          double dFrom = double(pMenu->m_items[i].m_nValue);
          double dTo = dFrom + double(pMenu->m_items[i].m_nValue2);
          if((dCurSecond >= dFrom) && 
             (dCurSecond <= (dTo - 2.0))) {
            nItemsToDraw[nItems] = i;
            dItemsAlpha[nItems] = 1.0;
            ++nItems;
          } else if((dCurSecond >= (dFrom - 2.0)) && 
                    (dCurSecond <= (dFrom))) {
            // Fading in
            nItemsToDraw[nItems] = i;
            dItemsAlpha[nItems] = 0.5 * (dCurSecond - (dFrom - 2.0));
            ++nItems;
          } else if((dCurSecond >= (dTo - 2.0)) && 
                    (dCurSecond <= (dTo))) {
            // Fading out
            nItemsToDraw[nItems] = i;
            dItemsAlpha[nItems] = 1.0 - (0.5 * (dCurSecond - (dTo - 2.0)));
            ++nItems;
          }
          if(nItems == 2) {
            break;
          }
        }

        // Draw the two menu items (their images)
        glPushMatrix();
        glTranslated(m_rectWnd.w / 2 - 256.0,
                     m_rectWnd.h / 2 + 64.0, 
                     0);
        for(i = 0; i < nItems; ++i) {
          if(pMenu->m_items[nItemsToDraw[i]].m_nAssocImage >= 0) {
            OpenGLHelpers::SetColorFull(1, 1, 1, dItemsAlpha[i]);
            OpenGLHelpers::SwitchToTexture(0);
            BTextures::Use(pMenu->m_items[nItemsToDraw[i]].m_nAssocImage);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // not mipmapped
            glBegin(GL_TRIANGLE_STRIP);
            OpenGLHelpers::SetTexCoord(0, 0);
            glVertex3f(0, -128, 0);
            OpenGLHelpers::SetTexCoord(0, 1);
            glVertex3f(0, 128, 0);
            OpenGLHelpers::SetTexCoord(1, 0);
            glVertex3f(512, -128, 0);
            OpenGLHelpers::SetTexCoord(1, 1);
            glVertex3f(512, 128, 0); 
            glEnd();
          }
        }

        // Draw frames
        float dAlpha = 0.5;
        if((dCurSecond < (pMenu->m_items[1].m_nValue - 2.0)) || 
           (dCurSecond > (pMenu->m_items[pMenu->m_nItems - 1].m_nValue))) {
          dAlpha = 0;
        }
        if((dCurSecond >= (pMenu->m_items[1].m_nValue - 2.0)) && 
           (dCurSecond <= (pMenu->m_items[1].m_nValue))) {
          dAlpha = 0.5 * 0.5 * (dCurSecond - (pMenu->m_items[1].m_nValue - 2.0));
        }
        if((dCurSecond >= (pMenu->m_items[pMenu->m_nItems - 2].m_nValue + pMenu->m_items[pMenu->m_nItems - 2].m_nValue2 - 2.0)) && 
           (dCurSecond <= (pMenu->m_items[pMenu->m_nItems - 2].m_nValue + pMenu->m_items[pMenu->m_nItems - 2].m_nValue2))) {
          dAlpha = 0.5 * (1.0 - 0.5 * (dCurSecond - (pMenu->m_items[pMenu->m_nItems - 2].m_nValue + pMenu->m_items[pMenu->m_nItems - 2].m_nValue2 - 2.0)));
        }

        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
        glDisable(GL_TEXTURE_2D);

        glBegin(GL_LINES);
        glColor4f(1, 1, 1, dAlpha);
        glVertex3f(0, -128, 0);
        glVertex3f(0,  127, 0);
        glVertex3f(511, -128, 0);
        glVertex3f(511,  127, 0);
        glVertex3f(0, -128, 0);
        glVertex3f(511, -128, 0);
        glVertex3f(0, 127, 0);
        glVertex3f(511, 127, 0);

        glColor4f(0, 0, 0, 0);
        glVertex3f(0, -128 - 40, 0);
        glColor4f(1, 1, 1, dAlpha);
        glVertex3f(0, -128, 0);

        glColor4f(0, 0, 0, 0);
        glVertex3f(511, -128 - 40, 0);
        glColor4f(1, 1, 1, dAlpha);
        glVertex3f(511, -128, 0);

        glColor4f(0, 0, 0, 0);
        glVertex3f(0, 127 + 40, 0);
        glColor4f(1, 1, 1, dAlpha);
        glVertex3f(0, 127, 0);

        glColor4f(0, 0, 0, 0);
        glVertex3f(511, 127 + 40, 0);
        glColor4f(1, 1, 1, dAlpha);
        glVertex3f(511, 126, 0);

        glColor4f(0, 0, 0, 0);
        glVertex3f(-40, -128, 0);
        glColor4f(1, 1, 1, dAlpha);
        glVertex3f(0, -128, 0);

        glColor4f(0, 0, 0, 0);
        glVertex3f(-40, 127, 0);
        glColor4f(1, 1, 1, dAlpha);
        glVertex3f(0, 127, 0);

        glColor4f(0, 0, 0, 0);
        glVertex3f(511 + 40, -128, 0);
        glColor4f(1, 1, 1, dAlpha);
        glVertex3f(511, -128, 0);

        glColor4f(0, 0, 0, 0);
        glVertex3f(511 + 40, 127, 0);
        glColor4f(1, 1, 1, dAlpha);
        glVertex3f(511, 127, 0);

        glEnd();

        glDisable(GL_COLOR_MATERIAL);

        glPopMatrix();
      }
    }

    // Finally, draw message box over menu, if needed
    if(pMenu->m_type == BMenu::GAMEMODE) {
      glPushMatrix();
      glTranslated(m_rectWnd.w / 2, m_rectWnd.h / 2, 0);
      if(BGame::m_bExitingMultiplay) {
        DrawPanel(dCharWidth * 26, dCharHeight * 6, 0.3, 0.05, 0);
        BUI::TextRenderer()->StartRenderingText();
        BUI::TextRenderer()->DrawTextAt(0, dCharHeight, "Do you want to exit multiplay?", BTextRenderer::ALIGN_CENTER);
        string sTmp;
        BUI::TextRenderer()->StopRenderingText();
        BGame::m_listYesNo.DrawAt(0, -dCharHeight * 1.5, BTextRenderer::ALIGN_CENTER, 1, 1, 1, false);
      }
      glPopMatrix();
    }
  }
}


//*************************************************************************************************
void CPakoon1View::DrawFinalPosition() {
  if(!BGame::m_bShowGameMenu && 
     (BGame::m_remotePlayer[BGame::GetMyPlace()].m_state == BRemotePlayer::FINISHED)) {
    string sFinalPos;
    switch(BGame::m_remotePlayer[BGame::GetMyPlace()].m_nRacePosition) {
      case 1:
        sFinalPos = "YOU'RE THE WINNER!";
        break;
      case 2:
        sFinalPos = "YOU FINISHED 2ND";
        break;
      case 3:
        sFinalPos = "YOU FINISHED 3RD";
        break;
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
      case 10:
		stringstream val;
		val << "YOU FINISHED " << BGame::m_remotePlayer[BGame::GetMyPlace()].m_nRacePosition << "TH";
		sFinalPos = val.str();
        break;
    }

    // Draw background

    double dAlpha = BGame::GetSmoothAlpha(); 
    dAlpha = 0.2 + 0.5 * dAlpha;

    glDisable(GL_TEXTURE_2D);

    double dWidth = BUI::TextRenderer()->GetStringWidth(sFinalPos);

    glBegin(GL_QUADS);

    OpenGLHelpers::SetColorFull(0, 0, 1, dAlpha);

    glVertex3f(m_rectWnd.w / 2 - dWidth / 2,
               m_rectWnd.h / 2 - 35, 0);
    glVertex3f(m_rectWnd.w / 2 + dWidth / 2,
               m_rectWnd.h / 2 - 35, 0);
    glVertex3f(m_rectWnd.w / 2 + dWidth / 2,
               m_rectWnd.h / 2 + 35, 0);
    glVertex3f(m_rectWnd.w / 2 - dWidth / 2,
               m_rectWnd.h / 2 + 35, 0);

    OpenGLHelpers::SetColorFull(0, 0, 0, dAlpha);

    glVertex3f(m_rectWnd.w / 2 - dWidth / 2,
               m_rectWnd.h / 2 - 28, 0);
    glVertex3f(m_rectWnd.w / 2 + dWidth / 2,
               m_rectWnd.h / 2 - 28, 0);
    glVertex3f(m_rectWnd.w / 2 + dWidth / 2,
               m_rectWnd.h / 2 + 28, 0);
    glVertex3f(m_rectWnd.w / 2 - dWidth / 2,
               m_rectWnd.h / 2 + 28, 0);

    glEnd();

    double dAngle;

    glBegin(GL_TRIANGLE_FAN);

    OpenGLHelpers::SetColorFull(0, 0, 1, dAlpha);

    glVertex3f(m_rectWnd.w / 2 - dWidth / 2, m_rectWnd.h / 2, 0);

    for(dAngle = 0; dAngle <= 180.0; dAngle += 20) {
      glVertex3f(m_rectWnd.w / 2 - dWidth / 2 - 35.0 * sin(dAngle / 180.0 * g_cdPI), 
                 m_rectWnd.h / 2 + 35.0 * cos(dAngle / 180.0 * g_cdPI), 0);
    }

    glEnd();

    glBegin(GL_TRIANGLE_FAN);

    glVertex3f(m_rectWnd.w / 2 + dWidth / 2, m_rectWnd.h / 2, 0);

    for(dAngle = 0; dAngle <= 180.0; dAngle += 20) {
      glVertex3f(m_rectWnd.w / 2 + dWidth / 2 + 35.0 * sin(dAngle / 180.0 * g_cdPI), 
                 m_rectWnd.h / 2 + 35.0 * cos(dAngle / 180.0 * g_cdPI), 0);
    }

    glEnd();

    glBegin(GL_TRIANGLE_FAN);

    OpenGLHelpers::SetColorFull(0, 0, 0, dAlpha);

    glVertex3f(m_rectWnd.w / 2 - dWidth / 2, m_rectWnd.h / 2, 0);

    for(dAngle = 0; dAngle <= 180.0; dAngle += 20) {
      glVertex3f(m_rectWnd.w / 2 - dWidth / 2 - 28.0 * sin(dAngle / 180.0 * g_cdPI), 
                 m_rectWnd.h / 2 + 28.0 * cos(dAngle / 180.0 * g_cdPI), 0);
    }

    glEnd();

    glBegin(GL_TRIANGLE_FAN);

    glVertex3f(m_rectWnd.w / 2 + dWidth / 2, m_rectWnd.h / 2, 0);

    for(dAngle = 0; dAngle <= 180.0; dAngle += 20) {
      glVertex3f(m_rectWnd.w / 2 + dWidth / 2 + 28.0 * sin(dAngle / 180.0 * g_cdPI), 
                 m_rectWnd.h / 2 + 28.0 * cos(dAngle / 180.0 * g_cdPI), 0);
    }

    glEnd();

    // Draw final position text

    BUI::TextRenderer()->StartRenderingText();
    BUI::TextRenderer()->DrawTextAt(m_rectWnd.w / 2, 
                                    m_rectWnd.h / 2, 
                                    sFinalPos, 
                                    BTextRenderer::ALIGN_CENTER, 
                                    1, 
                                    1, 
                                    1, 
                                    1);
    BUI::TextRenderer()->StopRenderingText();
  }
}


//*************************************************************************************************
void CPakoon1View::DrawMultiplayMessage(int i, string sMsg, double dAlpha, bool bNormal, bool bChatColor) {
  // Draw background

  glDisable(GL_TEXTURE_2D);
  glPushMatrix();

  double dWidth = 0.75 * BUI::TextRenderer()->GetStringWidth(sMsg);
  if(!bNormal) {
    dWidth = 0.75 * BUI::TextRenderer()->GetStringWidth(sMsg + "  ");
  }

  if(bNormal) {
    glTranslatef(0, 60 + 40 * i, 0);
  } else {
    glTranslatef(-m_rectWnd.w / 2 + dWidth / 2 + 30, m_rectWnd.h / 2, 0);
  }

  if(bNormal && !bChatColor) {
    OpenGLHelpers::SetColorFull(1, 0.5, 0, dAlpha);
  } else {
    OpenGLHelpers::SetColorFull(0, 0, 1, dAlpha);
  }

  glBegin(GL_QUADS);

  glVertex3f(m_rectWnd.w / 2 - dWidth / 2, -22, 0);
  glVertex3f(m_rectWnd.w / 2 + dWidth / 2, -22, 0);
  glVertex3f(m_rectWnd.w / 2 + dWidth / 2, 22, 0);
  glVertex3f(m_rectWnd.w / 2 - dWidth / 2, 22, 0);

  OpenGLHelpers::SetColorFull(0, 0, 0, dAlpha);

  glVertex3f(m_rectWnd.w / 2 - dWidth / 2, -16, 0);
  glVertex3f(m_rectWnd.w / 2 + dWidth / 2, -16, 0);
  glVertex3f(m_rectWnd.w / 2 + dWidth / 2, 16, 0);
  glVertex3f(m_rectWnd.w / 2 - dWidth / 2, 16, 0);

  glEnd();

  double dAngle;

  glBegin(GL_TRIANGLE_FAN);

  if(bNormal && !bChatColor) {
    OpenGLHelpers::SetColorFull(1, 0.5, 0, dAlpha);
  } else {
    OpenGLHelpers::SetColorFull(0, 0, 1, dAlpha);
  }

  glVertex3f(m_rectWnd.w / 2 - dWidth / 2, 0, 0);

  for(dAngle = 0; dAngle <= 180.0; dAngle += 20) {
    glVertex3f(m_rectWnd.w / 2 - dWidth / 2 - 22.0 * sin(dAngle / 180.0 * g_cdPI), 
               22.0 * cos(dAngle / 180.0 * g_cdPI), 0);
  }

  glEnd();

  glBegin(GL_TRIANGLE_FAN);

  glVertex3f(m_rectWnd.w / 2 + dWidth / 2, 0, 0);

  for(dAngle = 0; dAngle <= 180.0; dAngle += 20) {
    glVertex3f(m_rectWnd.w / 2 + dWidth / 2 + 22.0 * sin(dAngle / 180.0 * g_cdPI), 
               22.0 * cos(dAngle / 180.0 * g_cdPI), 0);
  }

  glEnd();

  glBegin(GL_TRIANGLE_FAN);

  OpenGLHelpers::SetColorFull(0, 0, 0, dAlpha);

  glVertex3f(m_rectWnd.w / 2 - dWidth / 2, 0, 0);

  for(dAngle = 0; dAngle <= 180.0; dAngle += 20) {
    glVertex3f(m_rectWnd.w / 2 - dWidth / 2 - 16.0 * sin(dAngle / 180.0 * g_cdPI), 
               16.0 * cos(dAngle / 180.0 * g_cdPI), 0);
  }

  glEnd();

  glBegin(GL_TRIANGLE_FAN);

  glVertex3f(m_rectWnd.w / 2 + dWidth / 2, 0, 0);

  for(dAngle = 0; dAngle <= 180.0; dAngle += 20) {
    glVertex3f(m_rectWnd.w / 2 + dWidth / 2 + 16.0 * sin(dAngle / 180.0 * g_cdPI), 
               16.0 * cos(dAngle / 180.0 * g_cdPI), 0);
  }

  glEnd();

  // Draw message text

  BUI::TextRenderer()->StartRenderingText();
  BUI::TextRenderer()->DrawSmallTextAt(m_rectWnd.w / 2, 
                                       0, 
                                       sMsg, 
                                       sMsg.length(), 
                                       BTextRenderer::ALIGN_CENTER,
                                       1, 1, 1, dAlpha);
  BUI::TextRenderer()->StopRenderingText();

  if(!bNormal) {
    // Draw cursor
    double dAlpha = fabs(double(clock() % CLOCKS_PER_SEC) - (double(CLOCKS_PER_SEC) / 2.0)) / (double(CLOCKS_PER_SEC) / 2.0);
    OpenGLHelpers::SetColorFull(0.5, 0.5, 1, dAlpha);
    glBegin(GL_QUADS);
    double dLen = BUI::TextRenderer()->GetCharWidth() * 0.5;
    double dXOffset = m_rectWnd.w / 2 + dWidth / 2 + 3 - dLen;
    double dCharHeight = BUI::TextRenderer()->GetCharHeight() * 0.5;
    glVertex3f(dXOffset, -dCharHeight / 2.0, 0);
    glVertex3f(dXOffset, dCharHeight / 2.0, 0);
    glVertex3f(dXOffset + dLen, dCharHeight / 2.0, 0);
    glVertex3f(dXOffset + dLen, -dCharHeight / 2.0, 0);
    glEnd();
  }


  glPopMatrix();
}
 

//*************************************************************************************************
void CPakoon1View::DrawMultiplayMessages() {
  if(BGame::m_nMultiplayMessages) {

    BGame::RemoveOldestMultiplayMessage();

    glPushMatrix();
    double dAlpha = 1.0;
    clock_t clockNow = clock();
    if((BGame::m_nMultiplayMessages) && 
       ((BGame::m_clockMultiplayMessages[0] - clockNow) < (CLOCKS_PER_SEC / 2))) {
      dAlpha = double(BGame::m_clockMultiplayMessages[0] - clockNow) / double(CLOCKS_PER_SEC / 2);
      glTranslatef(0, -40 + dAlpha * 40.0, 0);
    }

    for(int i = 0; i < BGame::m_nMultiplayMessages; ++i) {

      if(i > 0) {
        dAlpha = 1.0;
      }

      DrawMultiplayMessage(i, BGame::m_sMultiplayMessages[i], dAlpha, true, BGame::m_bChatMessage[i]);
    }
    glPopMatrix();
  }

  if(BGame::m_bMultiplayOn) {
    BUI::TextRenderer()->StartRenderingText();
    if(!BGame::m_bTABChatting) {
      // BUI::TextRenderer()->DrawSmallTextAt(5, m_rectWnd.h / 2, "TAB:CHAT", 8, BTextRenderer::ALIGN_LEFT, 0.25, 0.25, 1, 1);
      BUI::TextRenderer()->DrawTextAt(5, m_rectWnd.h / 2, "`", BTextRenderer::ALIGN_LEFT, 1, 1, 1, 1);
      BUI::TextRenderer()->DrawSmallTextAt(30, m_rectWnd.h / 2, "(TAB)", 1, BTextRenderer::ALIGN_LEFT, 0.25, 0.25, 1, 1);
    } else {
      // Draw chat message and it's cursor
      string sChatMsg = "` " + BGame::m_sChatMsg;
      DrawMultiplayMessage(0, sChatMsg, 1, false, true);
    }
    BUI::TextRenderer()->StopRenderingText();
  }
}

//*************************************************************************************************
void CPakoon1View::DrawMultiplayMenuStuff(BMenu *pMenu, double dX, double dY) {
  double dCharHeight = BUI::TextRenderer()->GetCharHeight();

  // Draw remote selection triangles
  glDisable(GL_TEXTURE_2D);
  glPushMatrix();
  glTranslated(dX, dY, 0);
  glTranslated(0, (dCharHeight * double(pMenu->m_nItems)) / 2.0, 0);

  for(int i = 0; i < pMenu->m_listMenu.m_nItems; ++i) {

    int nRelativeTriPos = 0;

    // Check if local player has a selection here

    if(pMenu->m_listMenu.m_nSelected == i) {
      // leave room for the local triangle
      nRelativeTriPos = 1;
    }

    // Check which players have selected this item
    for(int p = 0; p < BGame::m_nRemotePlayers; ++p) {
      if(p != BGame::GetMultiplay()->m_params.m_nMyPlace) {
        if(BGame::m_remotePlayer[p].m_sCurrentMenuSel.compare(pMenu->m_listMenu.m_psItems[i]) == 0) {
          DrawRemoteMenuTriangleAt(pMenu, i, nRelativeTriPos, p);
          ++nRelativeTriPos;
        }
      }
    }
  }
  glPopMatrix();
}


//*************************************************************************************************
void CPakoon1View::DrawRemoteMenuTriangleAt(BMenu *pMenu, int i, int nRelativeTriPos, int nPlayer) {
  double dCharHeight = BUI::TextRenderer()->GetCharHeight();
  double dLen = BUI::TextRenderer()->GetStringWidth(pMenu->m_listMenu.m_psItems[i]);
  double dYBase = double(i) * -dCharHeight;

  double dXOffset = 0;
  if(pMenu->m_align == BTextRenderer::ALIGN_CENTER) {
    dXOffset = -dLen / 2.0;
  } else if(pMenu->m_align == BTextRenderer::ALIGN_RIGHT) {
    dXOffset = -dLen;
  }

  glPushMatrix();
  glTranslated(dXOffset - 1 - nRelativeTriPos * 20, dYBase - 0.5, 0);

  double dAlpha = BGame::GetSmoothAlpha(); // fabs(double(clock() % CLOCKS_PER_SEC) - (double(CLOCKS_PER_SEC) / 2.0)) / (double(CLOCKS_PER_SEC) / 2.0);
  dAlpha = 0.5 + 0.5 * dAlpha;

  if(BGame::m_remotePlayer[nPlayer].m_bSelectionMade) {
    dAlpha = 1.0;
  }

  double dR, dG, dB;
  BGame::GetMultiplayerColor(nPlayer, dR, dG, dB);
  OpenGLHelpers::SetColorFull(dR, dG, dB, dAlpha);

  glBegin(GL_TRIANGLES);
  glVertex3f(-10,  0, 0);
  glVertex3f(-30, 10, 0);
  glVertex3f(-30, -10, 0);
  glEnd();

  glPopMatrix();
}


//*************************************************************************************************
void CPakoon1View::DrawTexturedSphere() {

  static bool bDLCreated = false;
  static int  nDL = -1;

  if(!bDLCreated) {
    bDLCreated = true;
    if(nDL == -1) {
      nDL = glGenLists(1);
    }

    glNewList(nDL, GL_COMPILE_AND_EXECUTE);

    double dRadius = 300.0;
    double dStrips = 40.0;
    double dBelts = 20.0;

    // Northern hemisphere
  
    double y;
    for(y = 0.0; y < dBelts; y += 1.0) {
      glBegin(GL_TRIANGLE_STRIP);
      for(double x = 0; x < (dStrips + 1.0); x += 1.0) {
        OpenGLHelpers::SetTexCoord(x / dStrips, 0.5 + (y + 1.0) / dBelts / 2.0);
        glNormal3f(cos(x / dStrips * 2.0 * g_cdPI) * cos((y + 1.0) / dBelts * g_cdPI / 2.0),
                   sin(x / dStrips * 2.0 * g_cdPI) * cos((y + 1.0) / dBelts * g_cdPI / 2.0),
                   -sin((y + 1.0) / dBelts * g_cdPI / 2.0));
        glVertex3f(cos(x / dStrips * 2.0 * g_cdPI) * cos((y + 1.0) / dBelts * g_cdPI / 2.0) * dRadius, 
                   sin(x / dStrips * 2.0 * g_cdPI) * cos((y + 1.0) / dBelts * g_cdPI / 2.0) * dRadius, 
                   sin((y + 1.0) / dBelts * g_cdPI / 2.0) * -dRadius);
        OpenGLHelpers::SetTexCoord(x / dStrips, 0.5 + y / dBelts / 2.0);
        glNormal3f(cos(x / dStrips * 2.0 * g_cdPI) * cos(y / dBelts * g_cdPI / 2.0),
                   sin(x / dStrips * 2.0 * g_cdPI) * cos(y / dBelts * g_cdPI / 2.0),
                   -sin(y / dBelts * g_cdPI / 2.0));
        glVertex3f(cos(x / dStrips * 2.0 * g_cdPI) * cos(y / dBelts * g_cdPI / 2.0) * dRadius, 
                   sin(x / dStrips * 2.0 * g_cdPI) * cos(y / dBelts * g_cdPI / 2.0) * dRadius, 
                   sin(y / dBelts * g_cdPI / 2.0) * -dRadius);
      }
      glEnd();
    }

    // Southern hemisphere
    for(y = 0.0; y > -dBelts; y -= 1.0) {
      glBegin(GL_TRIANGLE_STRIP);
      for(double x = 0; x < (dStrips + 1.0); x += 1.0) {
        OpenGLHelpers::SetTexCoord(x / dStrips, 0.5 + y / dBelts / 2.0);
        glNormal3f(cos(x / dStrips * 2.0 * g_cdPI) * cos(y / dBelts * g_cdPI / 2.0),
                   sin(x / dStrips * 2.0 * g_cdPI) * cos(y / dBelts * g_cdPI / 2.0),
                   -sin(y / dBelts * g_cdPI / 2.0));
        glVertex3f(cos(x / dStrips * 2.0 * g_cdPI) * cos(y / dBelts * g_cdPI / 2.0) * dRadius, 
                   sin(x / dStrips * 2.0 * g_cdPI) * cos(y / dBelts * g_cdPI / 2.0) * dRadius, 
                   sin(y / dBelts * g_cdPI / 2.0) * -dRadius);
        OpenGLHelpers::SetTexCoord(x / dStrips, 0.5 + (y - 1.0) / dBelts / 2.0);
        glNormal3f(cos(x / dStrips * 2.0 * g_cdPI) * cos((y - 1.0) / dBelts * g_cdPI / 2.0),
                   sin(x / dStrips * 2.0 * g_cdPI) * cos((y - 1.0) / dBelts * g_cdPI / 2.0),
                   -sin((y - 1.0) / dBelts * g_cdPI / 2.0));
        glVertex3f(cos(x / dStrips * 2.0 * g_cdPI) * cos((y - 1.0) / dBelts * g_cdPI / 2.0) * dRadius, 
                   sin(x / dStrips * 2.0 * g_cdPI) * cos((y - 1.0) / dBelts * g_cdPI / 2.0) * dRadius, 
                   sin((y - 1.0) / dBelts * g_cdPI / 2.0) * -dRadius);
      }
      glEnd();
    }
    glEndList();
  } else {
    glCallList(nDL);
  }
}


//*************************************************************************************************
void CPakoon1View::DrawMenuItemTextAtRelPos(int nX, int nY, int nItems, int nIndex, BMenuItem *pMenuItem) {
  double dCharHeight = BUI::TextRenderer()->GetCharHeight();
  double dCharWidth  = BUI::TextRenderer()->GetCharWidth();

  if((pMenuItem->m_nValue >= 0) && (pMenuItem->m_nValue < pMenuItem->m_nAssocListItems)) {

    glPushMatrix();
    glTranslated(10, (dCharHeight * double(nItems)) / 2.0, 0);

    BUI::TextRenderer()->StartRenderingText();
    BUI::TextRenderer()->DrawTextAt(nX,
                                    nY + double(nIndex) * -dCharHeight,
                                    pMenuItem->m_sAssocListItems[pMenuItem->m_nValue],
                                    BTextRenderer::ALIGN_LEFT,
                                    0.7,
                                    0.7,
                                    0.7);
    BUI::TextRenderer()->StopRenderingText();
    glPopMatrix();
  }  
}



//*************************************************************************************************
void CPakoon1View::DrawMenuItemEditBoxAtRelPos(int nX, int nY, int nItems, int nIndex, BMenuItem *pMenuItem) {
  double dCharHeight = BUI::TextRenderer()->GetCharHeight();
  glPushMatrix();
  glTranslated(nX + 10, -1 + nY + double(nIndex) * -dCharHeight + (dCharHeight * double(nItems)) / 2.0, 0);
  pMenuItem->m_ebAssocEditBox.DrawAt(0, 0, pMenuItem->m_bOpen, BTextRenderer::ALIGN_LEFT, pMenuItem);
  glPopMatrix();
}


//*************************************************************************************************
void CPakoon1View::DrawMenuItemSliderAtRelPos(int nX, int nY, int nItems, int nIndex, BMenuItem *pMenuItem) {
  double dCharHeight = BUI::TextRenderer()->GetCharHeight();
  double dCharWidth  = BUI::TextRenderer()->GetCharWidth();

  OpenGLHelpers::SwitchToTexture(1, true);
  OpenGLHelpers::SwitchToTexture(0, true);
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glDisable(GL_TEXTURE_2D);

  if((pMenuItem->m_nValue >= 0) && (pMenuItem->m_nValue <= 100)) {

    glPushMatrix();
    glTranslated(nX + 10, -1 + nY + double(nIndex) * -dCharHeight + (dCharHeight * double(nItems)) / 2.0, 0);

    double dDimmer = 0.75;
    if(pMenuItem->m_bOpen) {
      double dAlpha = BGame::GetSmoothAlpha(); // fabs(double(clock() % CLOCKS_PER_SEC) - (double(CLOCKS_PER_SEC) / 2.0)) / (double(CLOCKS_PER_SEC) / 2.0);
      dDimmer = 0.6 + 0.4 * dAlpha;
    }

    glBegin(GL_TRIANGLE_STRIP);

    glColor4d(dDimmer * 1, dDimmer * 1, dDimmer * 1, 1);
    glVertex3f(0, -dCharHeight / 6, 0);
    glColor4d(0, 0, 0, 0);
    glVertex3f(0, -dCharHeight / 6 - dCharHeight / 3, 0);
    glColor4d(dDimmer * 1, dDimmer * 1, dDimmer * 1, 1);
    glVertex3f(100, -dCharHeight / 6, 0);
    glColor4d(0, 0, 0, 0);
    glVertex3f(100, -dCharHeight / 6 - dCharHeight / 3, 0);

    glColor4d(dDimmer * 1, dDimmer * 1, dDimmer * 1, 1);
    glVertex3f(100, -dCharHeight / 6, 0);
    glColor4d(0, 0, 0, 0);
    glVertex3f(100 + dCharHeight / 3, -dCharHeight / 6, 0);
    glColor4d(dDimmer * 1, dDimmer * 1, dDimmer * 1, 1);
    glVertex3f(100, -dCharHeight / 6 + dCharHeight / 3, 0);
    glColor4d(0, 0, 0, 0);
    glVertex3f(100 + dCharHeight / 3, -dCharHeight / 6 + dCharHeight / 3, 0);

    glColor4d(dDimmer * 1, dDimmer * 1, dDimmer * 1, 1);
    glVertex3f(100, dCharHeight / 6, 0);
    glColor4d(0, 0, 0, 0);
    glVertex3f(100, dCharHeight / 6 + dCharHeight / 3, 0);
    glColor4d(dDimmer * 1, dDimmer * 1, dDimmer * 1, 1);
    glVertex3f(0, dCharHeight / 6, 0);
    glColor4d(0, 0, 0, 0);
    glVertex3f(0, dCharHeight / 6 + dCharHeight / 3, 0);

    glColor4d(dDimmer * 1, dDimmer * 1, dDimmer * 1, 1);
    glVertex3f(0, -dCharHeight / 6 + dCharHeight / 3, 0);
    glColor4d(0, 0, 0, 0);
    glVertex3f(0 - dCharHeight / 3, -dCharHeight / 6 + dCharHeight / 3, 0);
    glColor4d(dDimmer * 1, dDimmer * 1, dDimmer * 1, 1);
    glVertex3f(0, -dCharHeight / 6, 0);
    glColor4d(0, 0, 0, 0);
    glVertex3f(0 - dCharHeight / 3, -dCharHeight / 6, 0);

    glColor4d(dDimmer * 1, dDimmer * 1, dDimmer * 1, 1);
    glVertex3f(0, -dCharHeight / 6, 0);
    glColor4d(0, 0, 0, 0);
    glVertex3f(0, -dCharHeight / 6 - dCharHeight / 3, 0);

    glEnd();

    glBegin(GL_QUADS);
    glColor4d(dDimmer * 0.5, dDimmer * 0.65, dDimmer * 0.9, 1);
    glVertex3f(0, -dCharHeight / 6, 0);
    glColor4d(dDimmer * 0.3, dDimmer * 0.45, dDimmer * 0.7, 1);
    glVertex3f(pMenuItem->m_nValue, -dCharHeight / 6, 0);
    glColor4d(dDimmer * 0.4, dDimmer * 0.5, dDimmer * 0.8, 1);
    glVertex3f(pMenuItem->m_nValue, dCharHeight / 6, 0);
    glColor4d(dDimmer * 0.8, dDimmer * 0.9, dDimmer * 1, 1);
    glVertex3f(0, dCharHeight / 6, 0);
    glEnd();

    /*
    glBegin(GL_QUADS);
    glColor4d(dDimmer * 0.8, dDimmer * 0.9, dDimmer * 1, 1);
    glVertex3f(0, -dCharHeight / 6, 0);
    glColor4d(dDimmer * 0.4, dDimmer * 0.5, dDimmer * 0.8, 1);
    glVertex3f(pMenuItem->m_nValue, -dCharHeight / 6, 0);
    glColor4d(dDimmer * 0.3, dDimmer * 0.45, dDimmer * 0.7, 1);
    glVertex3f(pMenuItem->m_nValue, dCharHeight / 6, 0);
    glColor4d(dDimmer * 0.5, dDimmer * 0.65, dDimmer * 0.9, 1);
    glVertex3f(0, dCharHeight / 6, 0);
    glEnd();
    */
    // glColor4d(dDimmer * 1, dDimmer * 1, dDimmer * 1, 1);
    glColor4d(1, 1, 1, 1);
    glBegin(GL_LINE_STRIP);
    glVertex3f(0, -dCharHeight / 6, 0);
    glVertex3f(100, -dCharHeight / 6, 0);
    glVertex3f(100, dCharHeight / 6, 0);
    glVertex3f(0, dCharHeight / 6, 0);
    glVertex3f(0, -dCharHeight / 6, 0);
    glEnd();

    glPopMatrix();
  }  

  glColor4d(1, 1, 1, 1);
  glDisable(GL_COLOR_MATERIAL);
}



//*************************************************************************************************
void CPakoon1View::ReturnPressedOnGameMenu() {

  string sTmp;
  int nSelected = BGame::m_menuGame.m_listMenu.GetSelectedItem(sTmp);
  BMenuItem *pMenuItem = 0;
  if(nSelected != -1) {
    pMenuItem = &(BGame::m_menuGame.m_items[nSelected]);
    if(pMenuItem->m_bDisabled) {
      BUI::StartUsingSelectionList(&(BGame::m_menuGame.m_listMenu), 
                                   &CPakoon1View::OnKeyDownGame);
      return;
    }
  }

  switch(BGame::m_menuGame.m_listMenu.m_nSelected) {
    case 4:
      // Quit (Return to main menu)

      BGame::m_remotePlayer[BGame::GetMyPlace()].m_state = BRemotePlayer::WANTS_TO_SELECT_NEW_RACE;
      BGame::BroadcastStateChange();

      BGame::m_bOKToProceedInMultiplayMenu = false;

      { // Choose new random menu background
        stringstream sTitle;
        srand((unsigned)time(NULL));
        sTitle << "Textures/MenuTitle_" << rand() % 6 << ".tga";
        BTextures::ReloadTexture(BTextures::MENU_TITLES, sTitle.str());
      }

      BGame::GetPlayer()->m_dFuel = BGame::GetSimulation()->GetVehicle()->m_dFuel;
      BGame::GetPlayer()->SaveStateFile();
      BGame::GetPlayer()->SaveCurrentSceneInfo();
      SDL_ShowCursor(0);
      m_nMenuTime += BGame::ContinueSimulation();
      m_game.m_bShowGameMenu = false;
      BGame::m_bMenuMode = true;
      if(BGame::m_bMultiplayOn) {
        BGame::m_pMenuCurrent = &(BGame::m_menuChooseGameMode);
      } else {
        BGame::m_pMenuCurrent = &(BGame::m_menuMain);
      }
      m_pKeyDownFunction = &CPakoon1View::OnKeyDownCurrentMenu;
      BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                   &CPakoon1View::OnKeyDownCurrentMenu);

      if(BGame::m_bMultiplayOn) {
        BroadcastMenuBrowse();
      }

      m_pDrawFunction = &CPakoon1View::OnDrawCurrentMenu;
      BGame::m_bGameLoading = false;
      BGame::m_bGameReadyToStart = false;
      m_bDrawOnlyMenu = false;
      m_bFullRedraw = true;

	  // Save log info
	  {
		stringstream outputStream;
		BGame::MyAfxMessageBox("------------------------------------");
		BGame::MyAfxMessageBox("EXITING SCENE");
		string sLogInfo;
		sLogInfo = "Vehicle: " + BGame::GetSimulation()->GetVehicle()->m_sName;
		sLogInfo = "Scene: " + BGame::GetSimulation()->GetScene()->m_sName;
		BGame::MyAfxMessageBox(sLogInfo);
		outputStream << "Screen: " << BGame::m_nDispWidth << "*" << BGame::m_nDispHeight << "*" << BGame::m_nDispBits << " @ " << BGame::m_nDispHz << "Hz";
		sLogInfo = outputStream.str();
		BGame::MyAfxMessageBox(sLogInfo);
		outputStream.str("");
		outputStream << "Terrain: " << BGame::m_nTerrainResolution;
		sLogInfo = outputStream.str();
		BGame::MyAfxMessageBox(sLogInfo);
		outputStream.str("");
		outputStream << "Effects: dust=" << BGame::m_nDustAndClouds << " water=" << BGame::m_nWaterSurface;
		sLogInfo = outputStream.str();
		BGame::MyAfxMessageBox(sLogInfo);
		outputStream.str("");
		outputStream << "FPS: AVE=" << g_dRate << ", Last10=" <<
						g_d10LastFPS[0] << " " <<
						g_d10LastFPS[1] << " " <<
						g_d10LastFPS[2] << " " <<
						g_d10LastFPS[3] << " " <<
						g_d10LastFPS[4] << " " <<
						g_d10LastFPS[5] << " " <<
						g_d10LastFPS[6] << " " <<
						g_d10LastFPS[7] << " " <<
						g_d10LastFPS[8] << " " <<
						g_d10LastFPS[9];
		sLogInfo = outputStream.str();
		BGame::MyAfxMessageBox(sLogInfo);
	  }

      // Start menu music
      SoundModule::StopGameMusic(true);
      SoundModule::StartMenuMusic();
      SoundModule::SetMenuMusicVolume(int(double(BGame::m_nMusicVolume) / 100.0 * 255.0));

      break;
    case 1:
      {
        // Restart race

        BGame::GetSimulation()->GetTerrain()->StopUsingScene();
        BGame::GetSimulation()->GetTerrain()->StartUsingScene(BGame::GetSimulation()->GetScene()->m_sName, 
                                                              BGame::GetSimulation()->GetScene()->m_vOrigin, 
                                                              BGame::GetSimulation()->GetScene()->m_dGroundTextureScaler1, 
                                                              BGame::GetSimulation()->GetScene()->m_dGroundTextureScaler2);
        // Prepare to enter game

        if(BGame::m_bMultiplayOn) {
          for(int i = 0; i < BGame::m_nRemotePlayers; ++i) {
            BGame::m_remotePlayer[i].m_bReadyToStart = false;
          }
          BGame::m_bMultiplayRaceStarter = false;
          BGame::m_clockMultiRaceStarter = 0;
          BGame::m_remotePlayer[BGame::GetMyPlace()].m_state = BRemotePlayer::WAITING_FOR_RACE;
          BGame::BroadcastStateChange();
        } else {
          BGame::m_nRemotePlayers = 0;
        }

        BGame::m_bRaceStarted  = false;
        BGame::m_bRaceFinished = false;
        if(BGame::m_gameMode == BGame::AIRTIME) {
          BGame::m_dRaceTime = BGame::GetSimulation()->GetScene()->m_dAirTimeMaxSec;
        } else {
          BGame::m_dRaceTime = 0.0;
        }
        BGame::m_dAirTime  = 0;
        BGame::m_bForceBreak = false;
        BGame::GetSimulation()->GetVehicle()->m_bBreaking = false;
        BGame::m_dLiftStarted = 0;
        BGame::GetSimulation()->GetCamera()->m_dFollowHeight = -3.0;
        BGame::m_cOnScreenInfo &= BGame::FPS; // Preserve fps setting
        BGame::GetSimulation()->GetTerrain()->CreateTerrainDisplayLists();

        // Initialize reference times
        BGame::m_dRefTime[0] = -1.0;
        BGame::m_dRefTime[1] = -1.0;
        BGame::m_dRefTime[2] = -1.0;
        BGame::m_dRefTime[3] = -1.0;
        BGame::m_dRefTime[4] = -1.0;
        BGame::m_dRefTime[5] = -1.0;
        BGame::m_dRefTime[6] = -1.0;
        BGame::m_nRefK = 0;
        BScene *pScene = BGame::GetSimulation()->GetScene();
        if(BGame::m_gameMode == BGame::SLALOM) {
          PrepareReferenceTimes(pScene->m_raceRecordSlalomTime);
        } else if(BGame::m_gameMode == BGame::SPEEDRACE) {
          PrepareReferenceTimes(pScene->m_raceRecordBestTime);
        }

        BMessages::RemoveAll();

        pScene->m_slalom.m_nCurrentPole = 0;

        // Move vehicle to start location
        BVector vStartLocation = BGame::GetSimulation()->GetScene()->m_vStartLocation;
        BVector vSceneLoc; 
        if(BGame::GetPlayer()->LoadCurrentSceneInfo(vSceneLoc)) {
          vStartLocation = vSceneLoc;
        }
        BGame::GetSimulation()->UpdateCarLocation();
        BVector vLoc = BGame::GetSimulation()->GetVehicle()->m_vLocation;
        BGame::GetSimulation()->GetVehicle()->Move(vStartLocation - vLoc);

        if(BGame::m_bMultiplayOn) {
          // Position the car according to its place in the multiplay list
          switch(BGame::GetMyPlace()) {
            case 1:
              BGame::GetSimulation()->GetVehicle()->Move(BVector(5, 0, 0));
              break;
            case 2:
              BGame::GetSimulation()->GetVehicle()->Move(BVector(-5, 0, 0));
              break;
            case 3:
              BGame::GetSimulation()->GetVehicle()->Move(BVector(10, 0, 0));
              break;
          }
        }

        BGame::GetSimulation()->UpdateCar();

        // Make terrain valid so that the first simulation will work
        BGame::GetSimulation()->GetTerrain()->MakeTerrainValid(BGame::GetSimulation()->GetVehicle()->m_vLocation,
                                                               BGame::GetSimulation()->GetCamera()->m_vLocation,
                                                               BGame::GetSimulation()->GetCamera()->m_orientation.m_vForward,
                                                               true, 
                                                               false, 
                                                               false);
        BVector vOnGround, vNormal;
        FixCarToBasicOrientation(0.0);
        BGame::GetSimulation()->GetCamera()->m_locMode = BCamera::FOLLOW;
        BGame::GetSimulation()->GetCamera()->m_vLocation = BGame::GetSimulation()->GetVehicle()->m_vLocation + BVector(-20, -10, 0);
        BGame::GetSimulation()->GetCamera()->m_orientation.m_vForward = BGame::GetSimulation()->GetVehicle()->m_vLocation - BGame::GetSimulation()->GetCamera()->m_vLocation;
        BGame::GetSimulation()->GetCamera()->m_orientation.m_vUp = BVector(0, 0, -1);
        BGame::GetSimulation()->GetCamera()->m_orientation.m_vRight = BGame::GetSimulation()->GetCamera()->m_orientation.m_vUp.CrossProduct(BGame::GetSimulation()->GetCamera()->m_orientation.m_vForward);
        // Make terrain valid so that the first simulation will work
        BGame::GetSimulation()->GetTerrain()->MakeTerrainValid(BGame::GetSimulation()->GetVehicle()->m_vLocation,
                                                               BGame::GetSimulation()->GetCamera()->m_vLocation,
                                                               BGame::GetSimulation()->GetCamera()->m_orientation.m_vForward,
                                                               true, 
                                                               false, 
                                                               false);

        // Switch to game mode

        BGame::m_bGameLoading = false;
        BGame::m_bMenuMode = false;
        m_pDrawFunction = &CPakoon1View::OnDrawGame;
        m_pKeyDownFunction = &CPakoon1View::OnKeyDownGame;
        m_game.m_bFadingIn = true;
        m_game.m_clockFadeStart = clock();
        m_bInitClock = true;

        if(BGame::m_bMultiplayOn) {
          BGame::m_bForceBreak = true;
        }

        SDL_ShowCursor(0);

        m_nMenuTime += BGame::ContinueSimulation();
        m_pKeyDownFunction = &CPakoon1View::OnKeyDownGame;
        m_game.m_bShowGameMenu = false;
      }
      break;
    case 2:
      // Settings
      BGame::m_pMenuPrevious = 0;
      BGame::m_bMenuMode = true;
      BGame::m_bSettingsFromGame = true;
      BGame::m_bGameReadyToStart = false;
      BGame::m_bGameLoading = false;
      BGame::m_pMenuCurrent = &(BGame::m_menuSettings);
      m_pDrawFunction = &CPakoon1View::OnDrawCurrentMenu;
      BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                   &CPakoon1View::OnKeyDownSelectionList);
      
      break;
    case 3:
      // Show Help
      m_nMenuTime += BGame::ContinueSimulation();
      m_pKeyDownFunction = &CPakoon1View::OnKeyDownGame;
      m_game.FreezeSimulation(false);
      // show quick help
      m_game.m_bShowQuickHelp = true;
      break;
    case 0:
      // Return to game          
      m_nMenuTime += BGame::ContinueSimulation();
      m_pKeyDownFunction = &CPakoon1View::OnKeyDownGame;
      m_game.m_bShowGameMenu = false;
      break;
  }
}

//*************************************************************************************************
void CPakoon1View::CancelPressedOnGameMenu() {
  if(m_game.m_bShowQuickHelp) {
    m_game.m_bShowQuickHelp = false;
    m_nMenuTime += BGame::ContinueSimulation();
  } else {        
    m_nMenuTime += BGame::ContinueSimulation();
    m_pKeyDownFunction = &CPakoon1View::OnKeyDownGame;
    m_game.m_bShowGameMenu = false;
  }
}



//*************************************************************************************************
void CPakoon1View::ReturnPressedOnCurrentMenu() {
  string sTmp;
  int nSelected = BGame::m_pMenuCurrent->m_listMenu.GetSelectedItem(sTmp);
  BMenuItem *pMenuItem = 0;
  if(nSelected != -1) {
    pMenuItem = &(BGame::m_pMenuCurrent->m_items[nSelected]);
    if(pMenuItem->m_bDisabled) {
      BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                   &CPakoon1View::OnKeyDownCurrentMenu);
      
      return;
    }
  }

  if(BGame::m_pMenuCurrent->m_type == BMenu::SETTINGS) {
    // See if we need to open/close sublist/slider
    if(nSelected != -1) {
      if(!pMenuItem->m_bOpen) {
        if(pMenuItem->m_type == BMenuItem::STRING_FROM_LIST) {
          // Open menu
          pMenuItem->m_bOpen = true;
          pMenuItem->m_listMenu.SetItems(pMenuItem->m_sAssocListItems, pMenuItem->m_nAssocListItems);
          pMenuItem->m_listMenu.SelectItem(pMenuItem->m_sAssocListItems[pMenuItem->m_nValue]);
          BUI::StartUsingSelectionList(&(pMenuItem->m_listMenu), 
                                       &CPakoon1View::OnKeyDownCurrentMenu);
        } else if(pMenuItem->m_type == BMenuItem::SLIDER) {
          // Open slider
          pMenuItem->m_bOpen = true;
          BUI::StartUsingSlider(&(pMenuItem->m_nValue), 
                                &CPakoon1View::OnKeyDownCurrentMenu);
        }
        
      } else {
        if(pMenuItem->m_type == BMenuItem::STRING_FROM_LIST) {
          // Update value
          pMenuItem->m_nValue = pMenuItem->m_listMenu.GetSelectedItem(sTmp);
        } else if(pMenuItem->m_type == BMenuItem::SLIDER) {
          // Set the selected volume
          if(pMenuItem->m_sText.compare("Music Volume:") == 0) {
            SoundModule::SetMenuMusicVolume(int(double(pMenuItem->m_nValue) / 100.0 * 255.0));
          } else if(pMenuItem->m_sText.compare("Sound Effects Volume:") == 0) {
            SoundModule::SetVehicleSoundsVolume(int(double(pMenuItem->m_nValue) / 100.0 * 255.0));
          }
        }
        // Close menu
        pMenuItem->m_bOpen = false;
        BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                     &CPakoon1View::OnKeyDownCurrentMenu);
        
      }
    }
  } else if(BGame::m_pMenuCurrent->m_type == BMenu::MAIN) {

    switch(nSelected) {
      case 0: // SINGLEPLAYER
        BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
        BGame::m_pMenuCurrent = &(BGame::m_menuChooseGameMode);
        BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                     &CPakoon1View::OnKeyDownCurrentMenu);
        StartMenuScroll(SCROLL_RIGHT);
        
        break;
      case 1: // MULTIPLAYER

        BGame::GetMultiplay()->InitMultiplaySession();
        BGame::SetupMultiplayMenu();

        BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
        BGame::m_pMenuCurrent = &(BGame::m_menuMultiplay);
        BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                     &CPakoon1View::OnKeyDownCurrentMenu);
        StartMenuScroll(SCROLL_RIGHT);
        
        break;
      case 2: // SETTINGS
        BGame::m_bSettingsFromGame = false;
        BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
        BGame::m_pMenuCurrent = &(BGame::m_menuSettings);
        BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                     &CPakoon1View::OnKeyDownCurrentMenu);
        StartMenuScroll(SCROLL_LEFT);
        
        break;
      case 3: // HISCORES
        BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
        BGame::m_pMenuCurrent = &(BGame::m_menuHiscores);
        BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                     &CPakoon1View::OnKeyDownCurrentMenu);
        StartMenuScroll(SCROLL_UP);
        
        break;
      case 4: // CREDITS
        BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
        BGame::m_pMenuCurrent = &(BGame::m_menuCredits);
        BGame::m_menuCredits.m_clockStarted = clock();
        BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                     &CPakoon1View::OnKeyDownCurrentMenu);
        StartMenuScroll(SCROLL_DOWN);
        
        break;
      case 5: // EXIT
        // Exit
        SoundModule::SetMenuMusicVolume(0);
        SoundModule::SetVehicleSoundsVolume(0);
        SoundModule::StopMenuMusic(true);

        // Write settings for graphics etc.
        BGame::GetPlayer()->SaveStateFile();
        Settings::WriteSettings(m_game.GetSimulation());

        setExit();
        break;
    }
  } else if(BGame::m_pMenuCurrent->m_type == BMenu::MULTIPLAYER) {

    // See if we need to open/close sublist/slider
    if(nSelected != -1) {
      if(!pMenuItem->m_bOpen) {
        if(pMenuItem->m_type == BMenuItem::STRING_FROM_LIST) {
          // Open menu
          pMenuItem->m_bOpen = true;
          pMenuItem->m_listMenu.SetItems(pMenuItem->m_sAssocListItems, pMenuItem->m_nAssocListItems);
          pMenuItem->m_listMenu.SelectItem(pMenuItem->m_sAssocListItems[pMenuItem->m_nValue]);
          BUI::StartUsingSelectionList(&(pMenuItem->m_listMenu), 
                                       &CPakoon1View::OnKeyDownCurrentMenu);
        } else if(pMenuItem->m_type == BMenuItem::EDITBOX) {
          // Open editbox
          pMenuItem->m_bOpen = true;
          BUI::StartUsingEditbox(&(pMenuItem->m_sValue), 
                                 &CPakoon1View::OnKeyDownCurrentMenu);
          pMenuItem->m_ebAssocEditBox.status = BUIEdit::EDITING;
        }
        
      } else {
        if(pMenuItem->m_type == BMenuItem::STRING_FROM_LIST) {
          // Update value
          pMenuItem->m_nValue = pMenuItem->m_listMenu.GetSelectedItem(sTmp);

          // Check if Server IP edit box status needs to be changed
          if(pMenuItem->m_sText.compare("Role:") == 0) {
            bool bDisable = (sTmp.compare("SERVER") == 0);
            BGame::m_pMenuCurrent->m_items[2].m_bDisabled = bDisable;
          }
        } else if(pMenuItem->m_type == BMenuItem::EDITBOX) {
          // Close editbox
          BUIEdit::TStatus status;
          pMenuItem->m_sValue = pMenuItem->m_ebAssocEditBox.GetValue(status);
        }
        // Close menu
        pMenuItem->m_bOpen = false;
        BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                     &CPakoon1View::OnKeyDownCurrentMenu);
        
      }
    }

    switch(nSelected) {
      case 3: // START MULTIPLAY
        {
          BMultiplayParams params;

          string sRole;
          BGame::m_menuMultiplay.m_items[1].m_listMenu.GetSelectedItem(sRole);

          params.m_bHost = (sRole.compare("SERVER") == 0);
          params.m_sHostIPAddress = BGame::m_menuMultiplay.m_items[2].m_sValue;
          params.m_sPlayerName = BGame::m_menuMultiplay.m_items[0].m_sValue;

          BGame::GetMultiplay()->StartMultiplaySession(&params);
          SoundModule::PlayMultiplayerJoinSound();

          BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
          BGame::m_pMenuCurrent = &(BGame::m_menuChooseGameMode);
          BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                       &CPakoon1View::OnKeyDownCurrentMenu);
          StartMenuScroll(SCROLL_RIGHT);
          BroadcastMenuBrowse();

          
          break;
        }
    }
  } else if(BGame::m_pMenuCurrent->m_type == BMenu::GAMEMODE) {

    // Get rid of intro sound, if it's still playing
    SoundModule::StopIntroSound(); 

    if(BGame::m_bExitingMultiplay) {
      // Check whether user wants to exit multiplay or not
      string sTmp;
      int nYesNo = BGame::m_listYesNo.GetSelectedItem(sTmp);
      if(nYesNo == 0) {
        // exit multiplay    
        BGame::GetMultiplay()->EndMultiplaySession();
        SoundModule::PlayMultiplayerLeftSound();

        // go back to main menu
        StartMenuScroll(SCROLL_LEFT);
        BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
        BGame::m_pMenuCurrent = &(BGame::m_menuMain);
      } else {
        // close dialog and remain in this menu
        BGame::m_bExitingMultiplay = false;
      }
      BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                   &CPakoon1View::OnKeyDownCurrentMenu);
    } else {

      if(BGame::m_bMultiplayOn && !BGame::m_bOKToProceedInMultiplayMenu) {
        BroadcastMenuSelection();
      } else {
        BGame::m_bOKToProceedInMultiplayMenu = false;
        string sTmp;
        int nSelected = BGame::m_pMenuCurrent->m_listMenu.GetSelectedItem(sTmp);

        switch(nSelected) {
          case 0: // SLALOM
          case 1: // SPEEDRACE
          case 2: // AIRTIME
            if(nSelected == 0) {
              BGame::m_gameMode = BGame::SPEEDRACE;
            } else if(nSelected == 1) {
              BGame::m_gameMode = BGame::SLALOM;
            } else if(nSelected == 2) {
              BGame::m_gameMode = BGame::AIRTIME;
            }
            BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
            BGame::m_pMenuCurrent = &(BGame::m_menuChooseScene);
            BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                         &CPakoon1View::OnKeyDownCurrentMenu);
            StartMenuScroll(SCROLL_RIGHT);
            BroadcastMenuBrowse();
                        
            break;
        }
      }
    }
  } else if(BGame::m_pMenuCurrent->m_type == BMenu::CHOOSE_SCENE) {

    if(BGame::m_bMultiplayOn) {
      BGame::m_remotePlayer[BGame::GetMyPlace()].m_state = BRemotePlayer::PREPARING_TO_RACE;
      BGame::BroadcastStateChange();
    }

    if(BGame::m_bMultiplayOn && !BGame::m_bOKToProceedInMultiplayMenu) {
      BroadcastMenuSelection();
    } else {
      BGame::m_bOKToProceedInMultiplayMenu = false;
      // Load Scene
      BGame::m_sScene = BGame::m_pMenuCurrent->m_items[nSelected].m_sAssocFile;

      BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
      BGame::m_pMenuCurrent = &(BGame::m_menuChooseVehicle);
      BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                   &CPakoon1View::OnKeyDownCurrentMenu);
      StartMenuScroll(SCROLL_RIGHT);
      BroadcastMenuBrowse();
      
    }
  } else if(BGame::m_pMenuCurrent->m_type == BMenu::CHOOSE_VEHICLE) {

    BGame::m_sVehicle = BGame::m_pMenuCurrent->m_items[nSelected].m_sAssocFile;

    if(BGame::m_bBuyingVehicle) {
      // Check whether user wants to buy the selected vehicle
      int nYesNo = BGame::m_listYesNo.GetSelectedItem(sTmp);
      if(nYesNo == 0) {
        // Buy        
        BGame::GetPlayer()->m_sValidVehicles += (">" + BGame::m_pMenuCurrent->m_items[nSelected].m_sText + "<");
        BGame::GetPlayer()->m_dCash -= BGame::m_dPurchasePrice;
        BGame::GetPlayer()->SaveStateFile();
      }
      BGame::m_bBuyingVehicle = false;
      BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                   &CPakoon1View::OnKeyDownCurrentMenu);
    } else if(BGame::m_bCannotBuyVehicle) {
      // exit cannot buy message
      BGame::m_bCannotBuyVehicle = false;
      BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                   &CPakoon1View::OnKeyDownCurrentMenu);
    } else if(BGame::m_pMenuCurrent->m_items[nSelected].m_bDisabled) {
      double dPrice = 100.0;
      if(BGame::m_pMenuCurrent->m_items[nSelected].m_sText.compare("Spirit") == 0) {
        dPrice = 500.0;
      } else if(BGame::m_pMenuCurrent->m_items[nSelected].m_sText.compare("Veyronette") == 0) {
        dPrice = 300.0;
      }
      BGame::m_dPurchasePrice = dPrice;
      if(BGame::GetPlayer()->m_dCash >= dPrice) {
        BUI::StartUsingSelectionList(&(BGame::m_listYesNo), &CPakoon1View::OnKeyDownCurrentMenu);
        BGame::m_bBuyingVehicle = true;
      } else {
        BUI::StartUsingSelectionList(&(BGame::m_listOK), &CPakoon1View::OnKeyDownCurrentMenu);
        BGame::m_bCannotBuyVehicle = true;
      }
    } else {

      if(BGame::m_bMultiplayOn && !BGame::m_bOKToProceedInMultiplayMenu) {

        // Broadcast the selected vehicle filename for all remote players

		//FIXME
        /*{
          CFile fileTmp(BGame::m_pMenuCurrent->m_items[nSelected].m_sAssocFile, CFile::modeRead | CFile::shareDenyNone);
          BroadcastSelectedVehicleFilename(fileTmp.GetFileName());
        }*/

        // Broadcast a vehicle has been selected

        BroadcastMenuSelection();
      } else {
        BGame::m_bOKToProceedInMultiplayMenu = false;
        // Initiate game load
        BGame::m_bGameLoading = false;
        BGame::m_bGameReadyToStart = true;
      }
    }
    
  } else if(BGame::m_pMenuCurrent->m_type == BMenu::CREDITS) {
    // "Do nothing"
    BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                 &CPakoon1View::OnKeyDownCurrentMenu);
    
  } else if(BGame::m_pMenuCurrent->m_type == BMenu::HISCORES) {
    // "Do nothing"
    BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                 &CPakoon1View::OnKeyDownCurrentMenu);
    
  }
}


//*************************************************************************************************
void CPakoon1View::CancelPressedOnCurrentMenu() {
  if(BGame::m_pMenuCurrent->m_type == BMenu::MAIN) {
    // NOT YET DONE, JUST RETURN TO MAIN MENU!!!
    // Setup for first menu
    BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                 &CPakoon1View::OnKeyDownCurrentMenu);
    // Exit
    //SoundModule::StopMenuMusic();

    // Write settings for graphics etc.
    //Settings::WriteSettings(m_game.GetSimulation());

    //AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_FILE_CLOSE, 0);
  } else if(BGame::m_pMenuCurrent->m_type == BMenu::SETTINGS) {
    // See if we need to open/close sublist/slider
    BMenu *pMenu = BGame::m_pMenuCurrent;
    string sTmp;
    int nSelected = pMenu->m_listMenu.GetSelectedItem(sTmp);
    if(nSelected != -1) {
      BMenuItem *pMenuItem = &(pMenu->m_items[nSelected]);
      if(pMenuItem->m_bOpen) {
        // Close menu
        pMenuItem->m_bOpen = false;
        BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                     &CPakoon1View::OnKeyDownCurrentMenu);
        
      } else {

        // Save settings
        string sTmp;
        //sscanf(pMenu->m_items[0].m_sAssocListItems[pMenu->m_items[0].m_nValue].c_str(), "%d*%d", &(BGame::m_nDispWidth), &(BGame::m_nDispHeight)); //we use desktop resolution only for now
        sscanf(pMenu->m_items[1].m_sAssocListItems[pMenu->m_items[1].m_nValue].c_str(), "%d", &(BGame::m_nDispBits));
        sscanf(pMenu->m_items[2].m_sAssocListItems[pMenu->m_items[2].m_nValue].c_str(), "%d", &(BGame::m_nDispHz));
        BGame::m_nTerrainResolution = pMenu->m_items[3].m_nValue;
        BGame::m_nCarDetails = pMenu->m_items[4].m_nValue;
        BGame::m_nWaterSurface = pMenu->m_items[5].m_nValue;
        BGame::m_nMusicVolume = pMenu->m_items[6].m_nValue;
        BGame::m_nVehicleVolume = pMenu->m_items[7].m_nValue;
        BGame::m_nSoundscape = pMenu->m_items[8].m_nValue;

        Settings::WriteSettings(BGame::GetSimulation());

        switch(BGame::m_nTerrainResolution) {
          case 0: m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_MINIMUM); break;
          case 1: m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_LOW); break;
          case 2: m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_MEDIUM); break;
          case 3: m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_HIGH); break;
          case 4: m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_MAXIMUM); break;
          case 5: m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_SLOW_MACHINE); break;
        }

        BGame::m_nShowEffects = BGame::m_nDustAndClouds;
        BGame::m_bNight = (BGame::m_nWaterSurface == 1);

        // See if resolution needs to be changed
        //FIXME
        /*DEVMODE devmode;
        EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);
        if((devmode.dmPelsWidth  != (DWORD)BGame::m_nDispWidth) || 
           (devmode.dmPelsHeight != (DWORD)BGame::m_nDispHeight) || 
           (devmode.dmBitsPerPel != (DWORD)BGame::m_nDispBits) ||
           (devmode.dmDisplayFrequency != (DWORD)BGame::m_nDispHz)) {
          devmode.dmPelsWidth = (DWORD)BGame::m_nDispWidth;
          devmode.dmPelsHeight = (DWORD)BGame::m_nDispHeight;
          devmode.dmBitsPerPel = (DWORD)BGame::m_nDispBits;
          devmode.dmDisplayFrequency = (DWORD)BGame::m_nDispHz;
          ChangeDisplaySettings(&devmode, CDS_FULLSCREEN);
          AfxGetMainWnd()->SetWindowPos(NULL, -2, -2, BGame::m_nDispWidth + 4, BGame::m_nDispHeight + 4, 0);
        }*/

        // Go back to main menu or game menu

        if(BGame::m_bSettingsFromGame) {
          BGame::m_bMenuMode = false;
          BUI::StartUsingSelectionList(&(BGame::m_menuGame.m_listMenu), &CPakoon1View::OnKeyDownGame);
          BGame::m_bShowGameMenu = true;
          m_pDrawFunction = &CPakoon1View::OnDrawGame;
          // m_game.m_bFadingIn = true;
          // m_game.m_clockFadeStart = clock();
          SDL_ShowCursor(0);
        } else {
          StartMenuScroll(SCROLL_RIGHT);
          BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
          BGame::m_pMenuCurrent = &(BGame::m_menuMain);
          BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                       &CPakoon1View::OnKeyDownCurrentMenu);
        }
        
      }
    }
  } else if(BGame::m_pMenuCurrent->m_type == BMenu::MULTIPLAYER) {
    BMenu *pMenu = BGame::m_pMenuCurrent;
    string sTmp;
    int nSelected = pMenu->m_listMenu.GetSelectedItem(sTmp);
    if(nSelected != -1) {
      BMenuItem *pMenuItem = &(pMenu->m_items[nSelected]);
      if(pMenuItem->m_bOpen) {
        // Close menu
        pMenuItem->m_bOpen = false;
        BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                     &CPakoon1View::OnKeyDownCurrentMenu);
        
      } else {
        StartMenuScroll(SCROLL_LEFT);
        BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
        BGame::m_pMenuCurrent = &(BGame::m_menuMain);
        BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                     &CPakoon1View::OnKeyDownCurrentMenu);
        
      }
    }
  } else if(BGame::m_pMenuCurrent->m_type == BMenu::GAMEMODE) {
    if(BGame::m_bMultiplayOn) {
      BGame::m_bExitingMultiplay = true;

      // Start using yesno key handler
      BGame::m_listYesNo.SelectItem("No");
      BUI::StartUsingSelectionList(&(BGame::m_listYesNo), &CPakoon1View::OnKeyDownCurrentMenu);
    } else {
      StartMenuScroll(SCROLL_LEFT);
      BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
      BGame::m_pMenuCurrent = &(BGame::m_menuMain);
      BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                   &CPakoon1View::OnKeyDownCurrentMenu);
    }
    
  } else if(BGame::m_pMenuCurrent->m_type == BMenu::CHOOSE_SCENE) {
    if(!BGame::m_bMultiplayOn) { // Don't allow back menu command on multiplay
      StartMenuScroll(SCROLL_LEFT);
      BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
      BGame::m_pMenuCurrent = &(BGame::m_menuChooseGameMode);
      BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                   &CPakoon1View::OnKeyDownCurrentMenu);
      
    } else {
      BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                   &CPakoon1View::OnKeyDownCurrentMenu);
    }
  } else if(BGame::m_pMenuCurrent->m_type == BMenu::CHOOSE_VEHICLE) {
    if(!BGame::m_bMultiplayOn) { // Don't allow back menu command on multiplay
      if(BGame::m_bBuyingVehicle || BGame::m_bCannotBuyVehicle) {
        BGame::m_bBuyingVehicle = false;
        BGame::m_bCannotBuyVehicle = false;
        BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                     &CPakoon1View::OnKeyDownCurrentMenu);
      } else {
        StartMenuScroll(SCROLL_LEFT);
        BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
        BGame::m_pMenuCurrent = &(BGame::m_menuChooseScene);
        BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                     &CPakoon1View::OnKeyDownCurrentMenu);
      }
      
    } else {
      BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                   &CPakoon1View::OnKeyDownCurrentMenu);
    }
  } else if(BGame::m_pMenuCurrent->m_type == BMenu::CREDITS) {
    StartMenuScroll(SCROLL_UP);
    BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
    BGame::m_pMenuCurrent = &(BGame::m_menuMain);
    BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                 &CPakoon1View::OnKeyDownCurrentMenu);
    
  } else if(BGame::m_pMenuCurrent->m_type == BMenu::HISCORES) {
    StartMenuScroll(SCROLL_DOWN);
    BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
    BGame::m_pMenuCurrent = &(BGame::m_menuMain);
    BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                 &CPakoon1View::OnKeyDownCurrentMenu);
    
  }
}


//*************************************************************************************************
void CPakoon1View::StartMenuScroll(TMenuScroll scroll) {
  m_clockMenuScroll = clock();
  m_scrollDir = scroll;
}



//*************************************************************************************************
void CPakoon1View::BroadcastMenuBrowse() {
  if(BGame::m_bMultiplayOn) {
    stringstream sMsg;
    string sSelection;
    (void) BGame::m_pMenuCurrent->m_listMenu.GetSelectedItem(sSelection);
    sMsg << char(BGame::GetMultiplay()->m_params.m_nMyPlace + 1) << sSelection;
    BGame::GetMultiplay()->SendBroadcastMsg(BMultiPlay::MENU_BROWSE, sMsg.str());
  }
}

//*************************************************************************************************
void CPakoon1View::BroadcastMenuSelection() {

  if(BGame::m_bMultiplayOn) {
    string sSelection;
    (void) BGame::m_pMenuCurrent->m_listMenu.GetSelectedItem(sSelection);

    if(BGame::GetMultiplay()->GetParams()->m_bHost) {
      BGame::m_remotePlayer[BGame::GetMyPlace()].m_sCurrentMenuSel = sSelection;
      BGame::m_remotePlayer[BGame::GetMyPlace()].m_bSelectionMade = true;

      // Check for menu proceed

      CheckForMultiplayMenuProceed();

    } else {
      stringstream sMsg;
      sMsg << char(BGame::GetMultiplay()->m_params.m_nMyPlace + 1) << sSelection;
      BGame::GetMultiplay()->SendBroadcastMsg(BMultiPlay::MENU_SELECTION, sMsg.str());
    }
  }
}


//*************************************************************************************************
void CPakoon1View::BroadcastSelectedVehicleFilename(string sFilename) {
  stringstream sMsg;
  sMsg << char(BGame::GetMultiplay()->m_params.m_nMyPlace + 1) << sFilename;
  BGame::GetMultiplay()->SendBroadcastMsg(BMultiPlay::I_CHOSE_VEHICLE_FILENAME, sMsg.str());
}


//*************************************************************************************************
void CPakoon1View::CheckForMultiplayMenuProceed() {
  // If all have made a selection and we are the host, 
  // tell players to highlight the selection and proceed to next menu

  if(BGame::GetMultiplay()->GetParams()->m_bHost) {
    bool bAllHaveSelected = true;
    for(int i = 0; i < BGame::m_nRemotePlayers; ++i) {
      if(!BGame::m_remotePlayer[i].m_bSelectionMade) {
        bAllHaveSelected = false;
        break;
      }
    }
    if(bAllHaveSelected) {

      // Check which was selected (use host-biased democracy)

      int i;
      int              nSelections = 0;
      BSelectionHelper selections[100];

      for(i = 0; i < 100; ++i) {
        selections[i].m_nRefCount = 0;
        selections[i].m_sSelection = "";
      }

      for(i = 0; i < BGame::m_nRemotePlayers; ++i) {
        bool bFound = false;
        for(int j = 0; j < nSelections; ++j) {
          if(selections[j].m_sSelection.compare(BGame::m_remotePlayer[i].m_sCurrentMenuSel) == 0) {
            ++(selections[j].m_nRefCount);
            bFound = true;
          }
        }
        if(!bFound) {
          selections[nSelections].m_sSelection = BGame::m_remotePlayer[i].m_sCurrentMenuSel;
          selections[nSelections].m_nRefCount = 1;
          ++nSelections;
        }
      }

      // Check if unique selection can be found
      int nMax = 1, nMaxIndex = -1;
      bool bUniqueMaxFound = false;
      for(i = 0; i < nSelections; ++i) {
        if(selections[i].m_nRefCount >= nMax) {
          if(selections[i].m_nRefCount > nMax) {
            nMax = selections[i].m_nRefCount;
            nMaxIndex = i;
            bUniqueMaxFound = true;
          } else {
            bUniqueMaxFound = false;
          }
        }
      }

      string sSelection;

      if(bUniqueMaxFound) {
        // majority rules
        sSelection = selections[nMaxIndex].m_sSelection;
      } else {
        // In case of confusion, host decides
        sSelection = BGame::m_remotePlayer[BGame::GetMyPlace()].m_sCurrentMenuSel;
      }

      string sMsg = sSelection;
      BGame::GetMultiplay()->SendBroadcastMsg(BMultiPlay::HIGHLIGHT_MENU_SELECTION, sMsg);

      // Syncronize watches
      BGame::GetMultiplay()->SendBroadcastMsg(BMultiPlay::CLOCK_IS_NOW_0, "");

      SDL_LockMutex(BGame::m_csMutex);
      BGame::m_clockOffsetFromZeroTime = SDL_GetTicks();
      SDL_UnlockMutex(BGame::m_csMutex);

      // Emulate self message
      HighlightMenuSelection(sSelection);
    }    
  }
}



//*************************************************************************************************
void CPakoon1View::HighlightMenuSelection(string sSelection) {
  // In multiplay mode, proceed as if return has been pressed
  int i;
  for(i = 0; i < BGame::m_nRemotePlayers; ++i) {
    BGame::m_remotePlayer[i].m_bSelectionMade = false;
  }

  // Give 1 second feedback to user about the selection!!!

  for(i = 0; i < BGame::m_pMenuCurrent->m_listMenu.m_nItems; ++i) {
    if(BGame::m_pMenuCurrent->m_listMenu.m_psItems[i].compare(sSelection) == 0) {

      if(BGame::m_pMenuCurrent->m_type != BMenu::CHOOSE_VEHICLE) { // Everyone gets to choose their own vehicle
        BGame::m_pMenuCurrent->m_listMenu.m_nSelected = i;
      }
      break;
    }
  }
  
  // Highlight commonly selected menu item
  m_clockHighlightMenu = clock();
  BGame::m_bOKToProceedInMultiplayMenu = true;
  
}




//*************************************************************************************************
void CPakoon1View::OnDrawGame() {
  static int nFrameNo = 0;
  static clock_t clockLastCheckPoint = clock();
  static clock_t clockLastCheckPoint2 = clock();
  static string sRate = "";

  BScene  *pScene  = BGame::GetSimulation()->GetScene();
  BCamera *pCamera = m_game.GetSimulation()->GetCamera();

  pCamera->m_vSpeed = pCamera->m_vLocation - pCamera->m_vPrevLocation;
  pCamera->m_vPrevLocation = pCamera->m_vLocation;

  BGame::UpdateAnalyzer();

  if(m_bInitClock) {
    clockLastCheckPoint = clock();
    m_bInitClock = false;
  }

  // glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_FOG_BIT | GL_DEPTH_BUFFER_BIT | GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT | GL_POLYGON_BIT | GL_STENCIL_BUFFER_BIT | GL_TEXTURE_BIT);

  glDrawBuffer(GL_BACK);

  // Reset OpenGL

  GLfloat vFogColor[4];
  vFogColor[0] = 0.85f;
  vFogColor[1] = 0.85f;
  vFogColor[2] = 1.0;
  vFogColor[3] = 0.0;
  if(BGame::m_bNight) {
    glClearColor(0, 0, 0.2, 0); /* For RGB-mode */
    vFogColor[0] = 0;
    vFogColor[1] = 0;
    vFogColor[2] = 0.2f;
    vFogColor[3] = 0;
  } else {
    glClearColor(0.85, 0.85, 1, 0); /* For RGB-mode */
  }
  glFogfv( GL_FOG_COLOR, vFogColor);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glScaled(-1.0, 1.0, 1.0);

  double dScreenFormat = 1.0;
  if(m_game.m_nScreenFormat == 1) {
    dScreenFormat = 2.0 / 3.0;
  }
  double aspect = 1.0;
  aspect = (double) m_rectWnd.w / (double) (m_rectWnd.h * dScreenFormat);

  {
    if(BGame::m_bRecordSlalom) {
      stringstream sSlalomRecord;
      if(BGame::m_bPassFromRightSlalom) {
        sSlalomRecord << "RIGHT (" << pScene->m_slalom.m_nCurrentPole << ")";
      } else {
        sSlalomRecord << "LEFT (" << pScene->m_slalom.m_nCurrentPole << ")";
      }
      BMessages::Remove("slalomrecord");
      BMessages::Show(40, "slalomrecord", sSlalomRecord.str(), 60, false, 0.5, 0.5, 1.0, false, true);
    }
  }

  // Record data for Replay/Best time
  static double timePrevRecord = -9999.9;
  if(BGame::m_bRaceStarted && 
     !BGame::m_bRaceFinished && 
     (((BGame::m_dRaceTime - timePrevRecord) / g_dPhysicsStepsInSecond) > 0.033)) {
    timePrevRecord = BGame::m_dRaceTime;
    if(pScene->m_raceRecord.m_nNextSlot < pScene->m_raceRecord.m_nMaxSlot) {
      pScene->m_raceRecord.m_bValid = true;
      BVehicle *pVehicle = BGame::GetSimulation()->GetVehicle();
      pScene->m_raceRecord.m_frames[pScene->m_raceRecord.m_nNextSlot].m_dTime = BGame::m_dRaceTime;
      pScene->m_raceRecord.m_frames[pScene->m_raceRecord.m_nNextSlot].m_vLocation = pVehicle->m_vLocation;
      pScene->m_raceRecord.m_frames[pScene->m_raceRecord.m_nNextSlot].m_vForward = pVehicle->m_orientation.m_vForward;
      pScene->m_raceRecord.m_frames[pScene->m_raceRecord.m_nNextSlot].m_vRight = pVehicle->m_orientation.m_vRight;
      ++(pScene->m_raceRecord.m_nNextSlot);
    }
  }

  static int nNowFrame = 0;
  if(!BGame::m_bRaceStarted) {
    BGame::m_dAirTime = 0.0;
    if(BGame::GetSimulation()->GetVehicle()->m_vLocation.m_dY > 100.0) {

      // Handle start of race

      nNowFrame = 0;
      timePrevRecord = -9999.9;
      pScene->m_raceRecord.m_nNextSlot = 0;
      pScene->m_raceRecord.m_dCarLength = BGame::GetSimulation()->GetVehicle()->m_dVisualLength;
      pScene->m_raceRecord.m_dCarWidth  = BGame::GetSimulation()->GetVehicle()->m_dVisualWidth;
      pScene->m_raceRecord.m_dCarHeight = BGame::GetSimulation()->GetVehicle()->m_dVisualHeight;
      BGame::m_bRaceStarted = true;
      if(BGame::m_gameMode == BGame::AIRTIME) {
        BGame::m_dRaceTime = pScene->m_dAirTimeMaxSec;
      } else {
        BGame::m_dRaceTime = 0.0;
      }
      m_clockTimerStart = clock();
      BGame::m_cOnScreenInfo |= BGame::TIMER_STARTED;

      if(!pScene->m_bVerified) {
        BGame::ShowMultiplayMessage("UNVERIFIED SCENE");
      }
      if(!BGame::GetSimulation()->GetVehicle()->m_bVerified) {
        BGame::ShowMultiplayMessage("UNVERIFIED CAR");
      }
    }
  }
  if(!BGame::m_bRaceFinished && (BGame::GetSimulation()->GetVehicle()->m_vLocation.m_dY > 6000.0)) {

    // Handle end of race

    BGame::m_bRaceFinished = true;
    BGame::m_bForceBreak = true;

    if(BGame::m_bRecordSlalom) {
      pScene->m_slalom.m_nSlalomPoles = pScene->m_slalom.m_nCurrentPole;
      pScene->SaveSlalom();
      BGame::m_bRecordSlalom = false;

      // Save also goal
      pScene->m_vGoal = BGame::GetSimulation()->GetVehicle()->m_vLocation;
      pScene->m_vGoal.m_dY = 6000.0;
      pScene->m_vGoal.m_dZ = 0.0;
      pScene->Save();
    } else {
      // Check that goal was succesfully hit
      BVector vVehicle = BGame::GetSimulation()->GetVehicle()->m_vLocation;
      if(fabs(vVehicle.m_dX - pScene->m_vGoal.m_dX) < 25.0) {

        SoundModule::PlayGoalFanfarSound();

        BGame::m_remotePlayer[BGame::GetMyPlace()].m_state = BRemotePlayer::FINISHED;
        BGame::BroadcastStateChange();

        pScene->m_raceRecord.m_dTotalTime = BGame::m_dRaceTime;
        int nMinutes = BGame::m_dRaceTime / g_dPhysicsStepsInSecond / 60;
        int nSeconds = (BGame::m_dRaceTime - (nMinutes * g_dPhysicsStepsInSecond * 60)) / g_dPhysicsStepsInSecond;
        int n100Seconds = (100 * (BGame::m_dRaceTime - (nMinutes * g_dPhysicsStepsInSecond * 60 + nSeconds * g_dPhysicsStepsInSecond))) / g_dPhysicsStepsInSecond;

        if(BGame::m_gameMode == BGame::SPEEDRACE) {
          if(pScene->m_bVerified && BGame::GetSimulation()->GetVehicle()->m_bVerified &&
             (!pScene->m_raceRecordBestTime.m_bValid || 
              (pScene->m_raceRecordBestTime.m_dTotalTime > pScene->m_raceRecord.m_dTotalTime))) {
            // Copy new best time record
            BGame::m_cOnScreenInfo |= BGame::NEW_RECORD;
            pScene->m_raceRecordBestTime.m_bValid = true;
            pScene->m_raceRecordBestTime = pScene->m_raceRecord;
            for(int i = 0; i < pScene->m_raceRecord.m_nNextSlot; ++i) {
              pScene->m_raceRecordBestTime.m_frames[i] = pScene->m_raceRecord.m_frames[i];
            }
            // Save the best time record fpr the scene
            pScene->SaveBestTimeRecord();
          }
        } else if(BGame::m_gameMode == BGame::SLALOM) {
          if(pScene->m_bVerified && BGame::GetSimulation()->GetVehicle()->m_bVerified &&
             (!pScene->m_raceRecordSlalomTime.m_bValid || 
              (pScene->m_raceRecordSlalomTime.m_dTotalTime > pScene->m_raceRecord.m_dTotalTime))) {
            // Copy new best time record
            BGame::m_cOnScreenInfo |= BGame::NEW_RECORD;
            pScene->m_raceRecordSlalomTime.m_bValid = true;
            pScene->m_raceRecordSlalomTime = pScene->m_raceRecord;
            for(int i = 0; i < pScene->m_raceRecord.m_nNextSlot; ++i) {
              pScene->m_raceRecordSlalomTime.m_frames[i] = pScene->m_raceRecord.m_frames[i];
            }
            // Save the best time record fpr the scene
            pScene->SaveSlalomTimeRecord();
          }
        } else if(BGame::m_gameMode == BGame::AIRTIME) {
          if(pScene->m_bVerified && BGame::GetSimulation()->GetVehicle()->m_bVerified &&
             (BGame::m_dAirTime > pScene->m_dBestAirTime)) {
            // Copy new best time record
            BGame::m_cOnScreenInfo |= BGame::NEW_RECORD;
            pScene->m_dBestAirTime = BGame::m_dAirTime;
            // Save the best time record fpr the scene
            pScene->Save();
            pScene->ValidateChecksum();
          }
        }

        // Update high scores

        if(pScene->m_bVerified && BGame::GetSimulation()->GetVehicle()->m_bVerified) {
          if(BGame::m_gameMode == BGame::AIRTIME) {
            BGame::UpdateHighScores(pScene->m_sName, BGame::m_gameMode, BGame::m_dAirTime);
          } else {
            BGame::UpdateHighScores(pScene->m_sName, BGame::m_gameMode, pScene->m_raceRecord.m_dTotalTime);
          }
        }
      } else {

        if(!(BGame::m_cOnScreenInfo & BGame::DISQUALIFIED_GOAL)) {
          BGame::m_remotePlayer[BGame::GetMyPlace()].m_state = BRemotePlayer::MISSED_GOAL;
          BGame::BroadcastStateChange();
          BGame::m_cOnScreenInfo |= BGame::DISQUALIFIED_GOAL;
          SoundModule::PlayDisqualifiedSound();
        }
      }
    }
  }

  // Give losers a booster
  double dAccFactor = BGame::GetSimulation()->GetVehicle()->m_dAccelerationFactor;

  if(BGame::m_bMultiplayOn) {
    double dMaxDist = -9000.0;

    for(int i = 0; i < BGame::m_nRemotePlayers; ++i) {
      if(BGame::m_remotePlayer[i].m_bSelf) {
        continue;
      }
      double dDist = BGame::m_remotePlayer[i].m_vLocation.m_dY - BGame::GetSimulation()->GetVehicle()->m_vLocation.m_dY;

      if(dDist > dMaxDist) {
        dMaxDist = dDist;
      }
    }

    if(dMaxDist > 0) {
      double dFactor = 1.0 + (dMaxDist / 350.0);
      if(dFactor > 4.0) {
        dFactor = 4.0;
      }
      BGame::GetSimulation()->GetVehicle()->m_dAccelerationFactor = BGame::GetSimulation()->GetVehicle()->m_dAccelerationFactor * dFactor;
    }
  }

  m_game.GetSimulation()->m_rectWnd = m_rectWnd;
  if(!m_game.m_bFrozen) {
    if(BGame::m_bForceBreak) {
      BGame::GetSimulation()->GetVehicle()->m_bBreaking = true;
      BGame::GetSimulation()->GetVehicle()->m_bAccelerating = false;
      BGame::GetSimulation()->GetVehicle()->m_bReversing = false;
    }
    m_game.GetSimulation()->PrePaint();
  }

  // Restore original acceleration factor
  if(BGame::m_bMultiplayOn) {
    BGame::GetSimulation()->GetVehicle()->m_dAccelerationFactor = dAccFactor;
  }

  // Send car location to other multiplay players
  if(BGame::m_bMultiplayOn) {
    BGame::BroadcastCarPosition();
  }

  if(pCamera->m_locMode == BCamera::INCAR) {
    gluPerspective(pCamera->m_dAngleOfView, aspect, 1.0f, float(cdWorldHemisphereRadius * 1.5));
  } else if(pCamera->m_locMode == BCamera::OVERVIEW) {
    gluPerspective(pCamera->m_dAngleOfView, aspect, 10.0f, float(cdWorldHemisphereRadius * 1.5));
  } else if(pCamera->m_locMode == BCamera::ONSIDE) {
    gluPerspective(pCamera->m_dAngleOfView, aspect, 0.2f, float(cdWorldHemisphereRadius * 1.5));
  } else {
    gluPerspective(pCamera->m_dAngleOfView, aspect, 1.0f, float(cdWorldHemisphereRadius * 1.5));
  }

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Set look at -point for camera 
  glViewport(0, 
             (GLint) ((m_rectWnd.h - m_rectWnd.h * dScreenFormat) / 2), 
             (GLint) m_rectWnd.w, 
             (GLint) (double(m_rectWnd.h) * dScreenFormat));

  static double dSortaClock = 0.0;
  static clock_t clockPrev = SDL_GetTicks();
  clock_t clockNow = SDL_GetTicks();
  dSortaClock += 0.001 * double(clockNow - clockPrev);
  clockPrev = clockNow;
  double dVert;
  double dHoriz;
  if(pCamera->m_locMode != BCamera::INCAR) {
    dVert = 0.125 * sin(dSortaClock) - 0.125 / 2.0;
    dHoriz = 0.125 * sin(dSortaClock / 3.0) - 0.125 / 2.0;
  } else {
    dVert = 0.0;
    dHoriz = 0.0;
  }

  if(!BGame::GetSceneEditor()->IsActive()) {
    // m_game.GetSimulation()->SetUpCamera(&rect);
  } else {
    BVector vVehicleTo = pCamera->m_vLocation + pCamera->m_orientation.m_vForward * 5.0;
    BVector vVehicleLoc = m_game.GetSimulation()->GetVehicle()->m_vLocation;
    m_game.GetSimulation()->GetVehicle()->Move(vVehicleTo - vVehicleLoc);
    m_game.GetSimulation()->UpdateCar();
    m_game.GetSimulation()->GetVehicle()->m_orientation = pCamera->m_orientation;

    // Ensure camera is above ground
    double dTmp;
    BVector vInternalOffset = BGame::GetSimulation()->GetTerrain()->m_vOffset;
    double dZ = -HeightMap::CalcHeightAt(vInternalOffset.m_dX + pCamera->m_vLocation.m_dX, 
                                         vInternalOffset.m_dY + pCamera->m_vLocation.m_dY, 
                                         dTmp, 
                                         pScene->m_terrainStyle,
                                         8,
                                         vInternalOffset.m_dY);
    if(pCamera->m_vLocation.m_dZ > (dZ - 1.0)) {
      pCamera->m_vLocation.m_dZ = (dZ - 1.0);
    }
  }

  gluLookAt(pCamera->m_vLocation.m_dX + dHoriz, 
            pCamera->m_vLocation.m_dY, 
            pCamera->m_vLocation.m_dZ + dVert,
            pCamera->m_vLocation.m_dX + pCamera->m_orientation.m_vForward.m_dX + dHoriz,
            pCamera->m_vLocation.m_dY + pCamera->m_orientation.m_vForward.m_dY, 
            pCamera->m_vLocation.m_dZ + pCamera->m_orientation.m_vForward.m_dZ + dVert, 
            pCamera->m_orientation.m_vUp.m_dX, 
            pCamera->m_orientation.m_vUp.m_dY, 
            pCamera->m_orientation.m_vUp.m_dZ);

  GLfloat fLight1PositionG[ 4];
  fLight1PositionG[0] = (GLfloat) 1;
  fLight1PositionG[1] = (GLfloat) 1;
  fLight1PositionG[2] = (GLfloat) -1;
  fLight1PositionG[3] = (GLfloat) 0; // w=0 -> directional light (not positional)
  glLightfv( GL_LIGHT0, GL_POSITION, fLight1PositionG);                     

  if(BGame::m_bNight) {
    BVehicle *pVehicle = BGame::GetSimulation()->GetVehicle();

    GLfloat fLight2PositionG[ 4];
    fLight2PositionG[0] = (GLfloat) pVehicle->m_vLocation.m_dX + pVehicle->m_orientation.m_vUp.m_dX * 10.0 + pVehicle->m_orientation.m_vForward.m_dX * -10.0;
    fLight2PositionG[1] = (GLfloat) pVehicle->m_vLocation.m_dY + pVehicle->m_orientation.m_vUp.m_dY * 10.0 + pVehicle->m_orientation.m_vForward.m_dY * -10.0;
    fLight2PositionG[2] = (GLfloat) pVehicle->m_vLocation.m_dZ + pVehicle->m_orientation.m_vUp.m_dZ * 10.0 + pVehicle->m_orientation.m_vForward.m_dZ * -10.0;
    fLight2PositionG[3] = (GLfloat) 1;
    glLightfv( GL_LIGHT1, GL_POSITION, fLight2PositionG);
    fLight2PositionG[0] = (GLfloat) pVehicle->m_orientation.m_vForward.m_dX * 100.0 + pVehicle->m_orientation.m_vUp.m_dX * -10.0;
    fLight2PositionG[1] = (GLfloat) pVehicle->m_orientation.m_vForward.m_dY * 100.0 + pVehicle->m_orientation.m_vUp.m_dY * -10.0;
    fLight2PositionG[2] = (GLfloat) pVehicle->m_orientation.m_vForward.m_dZ * 100.0 + pVehicle->m_orientation.m_vUp.m_dZ * -10.0;
    fLight2PositionG[3] = (GLfloat) 1;
    glLightfv( GL_LIGHT1, GL_SPOT_DIRECTION, fLight2PositionG);
    glEnable(GL_LIGHT1);
  } else {
    glDisable(GL_LIGHT1);
  }

  // Draw the world

  glEnable(GL_DEPTH_TEST);
  m_nMenuTime += m_game.GetSimulation()->Paint(m_bCreateDLs, m_bWireframe, m_bNormals, m_rectWnd);
  m_bCreateDLs = false;

  if(BGame::m_bMultiplayOn) {
    // Draw remote cars
    SDL_LockMutex(BGame::m_csMutex);
    DrawRemoteCars();
    SDL_UnlockMutex(BGame::m_csMutex);
  }


  // Draw ghost car, if there is one
  if(BGame::m_gameMode == BGame::SPEEDRACE) {
    if(pScene->m_raceRecordBestTime.m_bValid && 
       BGame::m_bRaceStarted && 
       !BGame::m_bRaceFinished) {
      DrawGhostCar(nNowFrame, &(pScene->m_raceRecordBestTime));
    }
  } else if(BGame::m_gameMode == BGame::SLALOM) {
    if(pScene->m_raceRecordSlalomTime.m_bValid && 
       BGame::m_bRaceStarted && 
       !BGame::m_bRaceFinished) {
      DrawGhostCar(nNowFrame, &(pScene->m_raceRecordSlalomTime));
    }
  } else if(BGame::m_gameMode == BGame::AIRTIME) {
    if(BGame::m_dAirTime >= pScene->m_dBestAirTime) {
      BGame::m_sRacePosition = "1/2";
    } else {
      BGame::m_sRacePosition = "2/2";
    }

    if(BGame::m_dRaceTime < 0) {

      if(!(BGame::m_cOnScreenInfo & BGame::DISQUALIFIED_OUT_OF_TIME)) {
        BGame::m_bForceBreak = true;
        BGame::m_cOnScreenInfo |= BGame::DISQUALIFIED_OUT_OF_TIME;
        BGame::m_bRaceFinished = true;
        SoundModule::PlayDisqualifiedSound();
      }
    }
  }

  // Check for reference time
  if(BGame::m_gameMode != BGame::AIRTIME) {
    int nRefK = BGame::GetSimulation()->GetVehicle()->m_vLocation.m_dY / 1000.0;
    if(nRefK > BGame::m_nRefK) {
      BGame::m_nRefK = nRefK;
      BGame::m_cOnScreenInfo |= BGame::REF_TIME;
      m_clockTimerStart = clock();
      double dRefTime = BGame::m_dRaceTime - BGame::m_dRefTime[nRefK];
      char cSign = '+';
      if(dRefTime < 0.0) {
        cSign = '-';
        dRefTime = fabs(dRefTime);
      }
      int nSeconds = dRefTime / g_dPhysicsStepsInSecond;
      int n100Seconds = (100 * (dRefTime - nSeconds * g_dPhysicsStepsInSecond)) / g_dPhysicsStepsInSecond;
      stringstream val;
      val << cSign << nSeconds << "." << n100Seconds;
      BGame::m_sRefTime = val.str();
    }
  }

  // Draw goal indicator
  // First determine goal
  BVector vGoal = pScene->m_vGoal;
  if(BGame::m_gameMode == BGame::SLALOM) {
    if(!BGame::m_bRecordSlalom && 
       (pScene->m_slalom.m_nCurrentPole < pScene->m_slalom.m_nSlalomPoles)) {
      vGoal = pScene->m_slalom.m_slalomPole[pScene->m_slalom.m_nCurrentPole].m_vLocation;
    }
  }

  DrawGoalArrow(vGoal);
  
  // Draw slalom poles, if game mode is slalom.
  // Also check for correct pole passes
  if(BGame::m_gameMode == BGame::SLALOM) {
    DrawActiveSlalomPoles();

    if(!BGame::m_bRecordSlalom) {
      BVector vVehicle = BGame::GetSimulation()->GetVehicle()->m_vLocation;

      while((pScene->m_slalom.m_nCurrentPole < pScene->m_slalom.m_nSlalomPoles) && 
            (vVehicle.m_dY > pScene->m_slalom.m_slalomPole[pScene->m_slalom.m_nCurrentPole].m_vLocation.m_dY)) {
        // Check that the pole has been passed from the correct side
        bool bPassed = true;
        if(pScene->m_slalom.m_slalomPole[pScene->m_slalom.m_nCurrentPole].m_bPassFromRight) {
          bPassed = vVehicle.m_dX >= pScene->m_slalom.m_slalomPole[pScene->m_slalom.m_nCurrentPole].m_vLocation.m_dX;
        } else {
          bPassed = vVehicle.m_dX <= pScene->m_slalom.m_slalomPole[pScene->m_slalom.m_nCurrentPole].m_vLocation.m_dX;
        }

        if(!bPassed) {

          if(!(BGame::m_cOnScreenInfo & BGame::DISQUALIFIED_WRONG_SIDE)) {
            BGame::m_cOnScreenInfo |= BGame::DISQUALIFIED_WRONG_SIDE;

            if(!BGame::m_bMultiplayOn) {
              BGame::m_bForceBreak = true;
            } else {
              BGame::m_remotePlayer[BGame::GetMyPlace()].m_state = BRemotePlayer::MISSED_POLE;
              BGame::BroadcastStateChange();
            }
            BGame::m_bRaceFinished = true;
            SoundModule::PlayDisqualifiedSound();
          }
          break;
        }

        SoundModule::PlaySlalomPortSound();

        ++(pScene->m_slalom.m_nCurrentPole);
      }
    }
  }

  // Scene editor stuff (active object highlight)
  if(m_game.GetSceneEditor()->IsActive()) {
    m_game.GetSceneEditor()->HighlightActiveObject();
  }

  // Draw 2D graphics
  Setup2DRendering();

  if(BGame::m_nShowEffects > 1) {
    DrawOldTubeEffect();
  }

  DrawOnScreenGameTexts(vGoal);
  DrawExtraScreenTexts();

  if(BGame::m_bNavSat) {
    DrawNavSat();
    SDL_Delay(1);
  }

  // Scene editor stuff
  if(m_game.GetSceneEditor()->IsActive()) {
    m_game.GetSceneEditor()->Draw(m_rectWnd);
  }

  // If something requires attention, draw a faint veil
  if(m_game.m_bShowGameMenu || 
     m_game.m_bShowQuickHelp) {
    OpenGLHelpers::DrawVeil(0, 0, 0.2, 0.5, m_rectWnd);
  }

  if(BGame::m_bShowQuickHelp) {
    DrawQuickHelp();
  } else if(m_game.m_bShowGameMenu) {
    DrawGameMenu();
  }

  // Draw hint text if it's on
  if(m_game.m_bShowHint) {
    DrawKeyboardHint();
  }

  if(!m_game.m_bShowGameMenu && !m_game.m_bShowQuickHelp && !m_game.m_bShowCancelQuestion) {
    glPushMatrix();
    glTranslated(m_rectWnd.w / 2.0, m_rectWnd.h * 0.35, 0);
    m_messages.Render();
    glPopMatrix();
  }

  // Draw mouse cursor, if needed
  if(((BGame::m_bService) || (BGame::m_bNavSat)) && 
     !(m_game.m_bShowGameMenu || m_game.m_bShowQuickHelp || m_game.m_bShowCancelQuestion)) { 
    DrawMouseCursor(m_rectWnd);
  }

  // On top of everything, draw fade in if we are in the first second
  if(BGame::m_bFadingIn) {
    if((clock() - BGame::m_clockFadeStart) > CLOCKS_PER_SEC) {
      m_game.m_bFadingIn = false;
      if(BGame::m_bMultiplayOn) {
        string sMsg = "0";
        sMsg[0] = char(BGame::GetMultiplay()->m_params.m_nMyPlace + 1);
        BGame::GetMultiplay()->SendBroadcastMsg(BMultiPlay::I_AM_READY_TO_START_GAME, sMsg);
        if(BGame::GetMultiplay()->m_params.m_bHost) {

          BGame::m_remotePlayer[BGame::GetMyPlace()].m_bReadyToStart = true;

          // Check if all are ready to start game

          BGame::CheckForGameStart();
        }
      }
    }
  }

  // Draw multiplay countdown, if needed
  if(BGame::m_bMultiplayOn) {
    if(BGame::m_bMultiplayRaceStarter) {

      static int nPrevCountdownID = -1; // just a helper to control the sound effects

      clock_t clockNow = clock();
      if((clockNow - BGame::m_clockMultiRaceStarter) < (CLOCKS_PER_SEC * 3)) {
        // Draw countdown numbers

        OpenGLHelpers::SwitchToTexture(0);
        BTextures::Use(BTextures::ONSCREEN_GAME_TEXTS);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // not mipmapped

        string sCountdown = "";
        if((clockNow - BGame::m_clockMultiRaceStarter) < (CLOCKS_PER_SEC * 1)) {
          sCountdown = "3";
          OpenGLHelpers::SetColorFull(1, 0.25, 0.25, 1);
          if(nPrevCountdownID != 3) {
            nPrevCountdownID = 3;
            SoundModule::PlayCountdown123Sound();
          }
        } else if((clockNow - BGame::m_clockMultiRaceStarter) < (CLOCKS_PER_SEC * 2)) {
          sCountdown = "2";
          OpenGLHelpers::SetColorFull(1, 0.75, 0.25, 1);
          if(nPrevCountdownID != 2) {
            nPrevCountdownID = 2;
            SoundModule::PlayCountdown123Sound();
          }
        } else {
          sCountdown = "1";
          OpenGLHelpers::SetColorFull(0.25, 1, 0.25, 1);
          if(nPrevCountdownID != 1) {
            nPrevCountdownID = 1;
            SoundModule::PlayCountdown123Sound();
          }
        }
        glPushMatrix();
        glScalef(2.0, 2.0, 2.0);
        DrawGameStringAt(sCountdown, m_rectWnd.w / 4 - 15, m_rectWnd.h / 2 - m_rectWnd.h / 8);
        glPopMatrix();
        glDisable(GL_TEXTURE_2D);
        
      } else {
        // Start actual game

        nPrevCountdownID = -1;
        SoundModule::PlayCountdownGoSound();

        for(int i = 0; i < BGame::m_nRemotePlayers; ++i) {
          BGame::m_remotePlayer[i].m_bReadyToStart = false;
        }

        BGame::m_nPlayersInGoal = 0;

        BGame::m_remotePlayer[BGame::GetMyPlace()].m_state = BRemotePlayer::RACING;
        BGame::BroadcastStateChange();

        BGame::m_bMultiplayRaceStarter = false;
        BGame::m_bForceBreak = false;
        BGame::GetSimulation()->GetVehicle()->m_bBreaking = false;
      }
    }
  }

  if(BGame::m_bMultiplayOn) {
    if(BGame::m_remotePlayer[BGame::GetMyPlace()].m_state == BRemotePlayer::FINISHED) {
      DrawFinalPosition();
    }
  }

  DrawMultiplayMessages();

  End2DRendering();

  // Calculate framerates
  clockNow = clock();
  g_d10LastFPS[nFrameNo] = double(clockNow - clockLastCheckPoint2) / double(CLOCKS_PER_SEC);
  if(g_d10LastFPS[nFrameNo] < 0.00001) {
    g_d10LastFPS[nFrameNo] = 999.9;
  } else {
    g_d10LastFPS[nFrameNo] = 1.0 / g_d10LastFPS[nFrameNo];
  }
  clockLastCheckPoint2 = clockNow;

  // if(++nFrameNo == 15) {
  if(++nFrameNo == 30) {
    g_dRate = 30.0 / (double(clockNow - (clockLastCheckPoint)) / double(CLOCKS_PER_SEC));
    g_dAveRate += g_dRate;
    if(m_nMenuTime) {
      m_nMenuTime = 0;
    }

    m_game.GetSimulation()->GetVehicle()->m_dSpeedKmh = m_game.GetSimulation()->GetVehicle()->m_dSpeed * 
                                                        g_dRate * 
                                                        m_game.GetSimulation()->m_nPhysicsStepsBetweenRender * 
                                                        3.6;

	stringstream format;
	format << "FPS:" << g_dRate <<
		", Speed:" << m_game.GetSimulation()->GetVehicle()->m_dSpeedKmh <<
		" km/h (" << m_game.GetSimulation()->GetVehicle()->m_dSpeed <<
		"), SimSteps:" << m_game.GetSimulation()->m_nPhysicsStepsBetweenRender <<
		", CamLoc:(" << m_game.GetSimulation()->GetCamera()->m_vLocation.m_dX <<
		", " << m_game.GetSimulation()->GetCamera()->m_vLocation.m_dY <<
		", " << m_game.GetSimulation()->GetCamera()->m_vLocation.m_dZ << ")";
	sRate = format.str();

    // Calculate the absolute time sync ratio (ATSR)
    m_game.GetSimulation()->m_dMaxGForce = 0.0;
    static int nSkipAmount = 1;
    static int nSkipper = 0;
    if(++nSkipper >= nSkipAmount) {
      if(nSkipAmount < 1) {
        ++nSkipAmount;
      }
      nSkipper = 0;
      g_dAveRate /= double(nSkipAmount);
      m_game.GetSimulation()->m_nPhysicsStepsBetweenRender = int(g_dPhysicsStepsInSecond / g_dAveRate);
      double dFraction = (g_dPhysicsStepsInSecond / g_dRate) - double(int(g_dPhysicsStepsInSecond / g_dRate));
      m_game.GetSimulation()->m_dPhysicsFraction = dFraction; 
      m_game.GetSimulation()->m_bCalibrateSimulationSpeed  = false;
      g_dAveRate = 0;
    }

    nFrameNo = 0;
    clockLastCheckPoint = clockNow;
  }

  // Override realistic physics time when in slow motion
  if(BGame::m_bSlowMotion) {
    m_game.GetSimulation()->m_nPhysicsStepsBetweenRender = 1;
  }

  SDL_GL_SwapWindow(window);

  // glPopAttrib();

  if(!m_game.GetSimulation()->m_bPaused) {
    
  }
}


//*************************************************************************************************
void CPakoon1View::DrawRemoteCars() {

  // Prepare to save screen location for player names

  GLdouble modelMatrix[16];
  GLdouble projMatrix[16];
  GLint    viewport[4];

  glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
  glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
  glGetIntegerv(GL_VIEWPORT, viewport);

  for(int i = 0; i < BGame::m_nRemotePlayers; ++i) {

    if(BGame::m_remotePlayer[i].m_bSelf) {
      continue;
    }

    BOrientation orient = BGame::m_remotePlayer[i].m_orientation;

    double dTimePassed = 0;

    int dwNow = BGame::GetMultiplayClock();
    if(dwNow > BGame::m_remotePlayer[i].m_clockLocationSent) {
      dTimePassed = double(dwNow - BGame::m_remotePlayer[i].m_clockLocationSent);
    } else {
      dTimePassed = -double(BGame::m_remotePlayer[i].m_clockLocationSent - dwNow);
    }

    BVector vLoc = BGame::m_remotePlayer[i].m_vLocation + 
                   BGame::m_remotePlayer[i].m_vVelocity * dTimePassed + 
                   BGame::m_remotePlayer[i].m_vVelo1stDeriv * dTimePassed; // +
                   //BGame::m_remotePlayer[i].m_vVelo2ndDeriv * dTimePassed;

    BVector vNameLocation = vLoc + 
                            orient.m_vForward * BGame::m_remotePlayer[i].m_dLen + 
                            BVector(0, 0, -BGame::m_remotePlayer[i].m_dHeight);

    if(gluProject(vNameLocation.m_dX, 
                  vNameLocation.m_dY, 
                  vNameLocation.m_dZ,
                  modelMatrix,
                  projMatrix,
                  viewport,
                  &(BGame::m_remotePlayer[i].m_vOnScreen.m_dX),
                  &(BGame::m_remotePlayer[i].m_vOnScreen.m_dY),
                  &(BGame::m_remotePlayer[i].m_vOnScreen.m_dZ))) {
    }

    glPushMatrix();

    GLdouble mtxVehicle[16];

    mtxVehicle[ 0] = orient.m_vRight.m_dX;
    mtxVehicle[ 1] = orient.m_vRight.m_dY;
    mtxVehicle[ 2] = orient.m_vRight.m_dZ;

    mtxVehicle[ 4] = orient.m_vForward.m_dX;
    mtxVehicle[ 5] = orient.m_vForward.m_dY;
    mtxVehicle[ 6] = orient.m_vForward.m_dZ;

    mtxVehicle[ 8] = -orient.m_vUp.m_dX;
    mtxVehicle[ 9] = -orient.m_vUp.m_dY;
    mtxVehicle[10] = -orient.m_vUp.m_dZ;

    mtxVehicle[12] = vLoc.m_dX;
    mtxVehicle[13] = vLoc.m_dY;
    mtxVehicle[14] = vLoc.m_dZ - 0.0;

    mtxVehicle[ 3] = 0.0;
    mtxVehicle[ 7] = 0.0;
    mtxVehicle[11] = 0.0;

    mtxVehicle[15] = 1.0;

    glMultMatrixd(mtxVehicle);

    double dLen, dHeight, dWidth;
    dLen    = BGame::m_remotePlayer[i].m_dLen;
    dHeight = BGame::m_remotePlayer[i].m_dHeight / 2.0;
    dWidth  = BGame::m_remotePlayer[i].m_dWidth;

    double dR, dG, dB;
    BGame::GetMultiplayerColor(i, dR, dG, dB);
    OpenGLHelpers::SetColorFull(dR, dG, dB, 1);

    BVehicle *pVehicle = BGame::m_remotePlayer[i].m_pVehicle; // BGame::GetSimulation()->GetVehicle();

    glCallList(pVehicle->m_nDLVehicleBody);

    // Draw Wheels
    glDisable(GL_CULL_FACE);
    for(int w = 0; w < pVehicle->m_nWheels; ++w) {
      glPushMatrix();
      BWheel *pWheel = dynamic_cast<BWheel *>(pVehicle->m_pWheel[w]);
      BVector vLoc = pWheel->m_vLocationOrig + pWheel->m_vSuspDir * -0.2;
      glTranslated(vLoc.m_dX, vLoc.m_dY, vLoc.m_dZ);

      if(pWheel->m_bTurns) {
        // Rotate according to turn
        glRotated(BGame::m_remotePlayer[i].m_dTurn * (pWheel->m_dThrow / 40.0) * 45.0, 0, 0, -1);
      }
      if(pWheel->m_bLeft) {
        // Rotate to make left wheel
        glRotated(180.0, 0, 0, -1);
        glRotated(pWheel->m_dAngle / g_cdPI * 180.0, -1, 0, 0);
      } else {
        glRotated(-pWheel->m_dAngle / g_cdPI * 180.0, -1, 0, 0);
      }
      glCallList(pWheel->m_nDLWheel);
      glPopMatrix();
    }

    glPopMatrix();

    // Draw velocity vectors
    /*
    BVector vLocVect = vLoc + BGame::m_remotePlayer[i].m_orientation.m_vUp * BGame::m_remotePlayer[i].m_dHeight;

    glBegin(GL_LINES);

    OpenGLHelpers::SetColorFull(1, 0, 0, 1);
    glVertex3f(vLocVect.m_dX, vLocVect.m_dY, vLocVect.m_dZ);
    vLocVect = vLocVect + BGame::m_remotePlayer[i].m_vVelocity * dTimePassed;
    glVertex3f(vLocVect.m_dX, vLocVect.m_dY, vLocVect.m_dZ);

    OpenGLHelpers::SetColorFull(0, 1, 0, 1);
    vLocVect = vLocVect + BGame::m_remotePlayer[i].m_vVelo1stDeriv * dTimePassed;
    glVertex3f(vLocVect.m_dX, vLocVect.m_dY, vLocVect.m_dZ);

    OpenGLHelpers::SetColorFull(0, 0, 1, 1);
    vLocVect = vLocVect + BGame::m_remotePlayer[i].m_vVelo2ndDeriv * dTimePassed;
    glVertex3f(vLocVect.m_dX, vLocVect.m_dY, vLocVect.m_dZ);

    glEnd();
    */


    /*
    glDisable(GL_CULL_FACE);

    glBegin(GL_QUADS);

    glNormal3f(-0.7, 0.7, 0);
    glVertex3f(-dWidth * 2.0, dLen,        dHeight);
    glVertex3f( 0.0,          dLen * 2.0,  dHeight);
    glVertex3f( 0.0,          dLen * 2.0, -dHeight);
    glVertex3f(-dWidth * 2.0, dLen,       -dHeight);

    glNormal3f( 0.7, 0.7, 0);
    glVertex3f( dWidth * 2.0, dLen,        dHeight);
    glVertex3f( 0.0,          dLen * 2.0,  dHeight);
    glVertex3f( 0.0,          dLen * 2.0, -dHeight);
    glVertex3f( dWidth * 2.0, dLen,       -dHeight);

    glEnd();

    glBegin(GL_TRIANGLES);

    glNormal3f(0, 0, -1);
    glVertex3f(-dWidth * 2.0, dLen,       -dHeight);
    glVertex3f( 0.0,          dLen * 2.0, -dHeight);
    glVertex3f( dWidth * 2.0, dLen,       -dHeight);

    glNormal3f(0, 0, 1);
    glVertex3f(-dWidth * 2.0, dLen,       dHeight);
    glVertex3f( 0.0,          dLen * 2.0, dHeight);
    glVertex3f( dWidth * 2.0, dLen,       dHeight);

    glEnd();

    glBegin(GL_QUADS);

    glNormal3f(0, 1, 0);
    glVertex3f(-dWidth,       dLen,  dHeight);
    glVertex3f(-dWidth * 2.0, dLen,  dHeight);
    glVertex3f(-dWidth * 2.0, dLen, -dHeight);
    glVertex3f(-dWidth,       dLen, -dHeight);

    glVertex3f(dWidth,       dLen,  dHeight);
    glVertex3f(dWidth * 2.0, dLen,  dHeight);
    glVertex3f(dWidth * 2.0, dLen, -dHeight);
    glVertex3f(dWidth,       dLen, -dHeight);

    glNormal3f(-1, 0, 0);
    glVertex3f(-dWidth,  dLen, -dHeight);
    glVertex3f(-dWidth,  dLen,  dHeight);
    glVertex3f(-dWidth, -dLen,  dHeight);
    glVertex3f(-dWidth, -dLen, -dHeight);

    glNormal3f(1, 0, 0);
    glVertex3f(dWidth,  dLen, -dHeight);
    glVertex3f(dWidth,  dLen,  dHeight);
    glVertex3f(dWidth, -dLen,  dHeight);
    glVertex3f(dWidth, -dLen, -dHeight);

    glNormal3f(0, 0, -1);
    glVertex3f(-dWidth,  dLen, -dHeight);
    glVertex3f( dWidth,  dLen, -dHeight);
    glVertex3f( dWidth, -dLen, -dHeight);
    glVertex3f(-dWidth, -dLen, -dHeight);

    glNormal3f(0, 0, 1);
    glVertex3f(-dWidth,  dLen, dHeight);
    glVertex3f( dWidth,  dLen, dHeight);
    glVertex3f( dWidth, -dLen, dHeight);
    glVertex3f(-dWidth, -dLen, dHeight);

    glNormal3f(0, -1, 0);
    glVertex3f(-dWidth, -dLen, -dHeight);
    glVertex3f(-dWidth, -dLen,  dHeight);
    glVertex3f( dWidth, -dLen,  dHeight);
    glVertex3f( dWidth, -dLen, -dHeight);

    glEnd();
    */

  }
}


//*************************************************************************************************
void CPakoon1View::DrawOldTubeEffect() {
  //glEnable(GL_BLEND);
  // glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
  glBlendFunc(GL_ZERO, GL_SRC_COLOR);

  OpenGLHelpers::SetColorFull(1, 1, 1, 1);
  OpenGLHelpers::SwitchToTexture(0);
  BTextures::Use(BTextures::OLDTUBE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  
  glBegin(GL_QUADS);
  OpenGLHelpers::SetTexCoord(0, 0);
  glVertex3f(0, 0, 0);
  OpenGLHelpers::SetTexCoord(1, 0);
  glVertex3f(m_rectWnd.w, 0, 0);
  OpenGLHelpers::SetTexCoord(1, 1);
  glVertex3f(m_rectWnd.w, m_rectWnd.h, 0);
  OpenGLHelpers::SetTexCoord(0, 1);
  glVertex3f(0, m_rectWnd.h, 0);
  glEnd();

  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

  //glDisable(GL_BLEND);
}


//*************************************************************************************************
void CPakoon1View::DrawKeyboardHint() {
  clock_t clockNow = clock();
  if((clockNow - m_game.m_clockHintStart) > CLOCKS_PER_SEC * 2) {
    m_game.m_bShowHint = false;
  } else {
    double dAlpha = 1.0 - double(clockNow - m_game.m_clockHintStart) / double(CLOCKS_PER_SEC * 2);

    // OpenGLHelpers::SetColorFull(0.3, 0.6, 0.8, dAlpha);
    OpenGLHelpers::SetColorFull(0.9, 0.2, 0.1, dAlpha);
    OpenGLHelpers::SwitchToTexture(0);
    BTextures::Use(BTextures::PANEL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glPushMatrix();
    glTranslated(m_rectWnd.w / 2.0, m_rectWnd.h / 2.0, 0);

    glBegin(GL_TRIANGLE_STRIP);
    OpenGLHelpers::SetTexCoord(0, 0);
    glVertex3f(-321.0 / 2.0, -57.0 / 2.0, 0);
    OpenGLHelpers::SetTexCoord(0, (512.0 - 455.0) / 512.0);
    glVertex3f(-321.0 / 2.0, 57.0 / 2.0, 0);
    OpenGLHelpers::SetTexCoord(321.0 / 512.0, 0);
    glVertex3f(321.0 / 2.0, -57.0 / 2.0, 0);
    OpenGLHelpers::SetTexCoord(321.0 / 512.0, (512.0 - 455.0) / 512.0);
    glVertex3f(321.0 / 2.0, 57.0 / 2.0, 0);
    glEnd();

    glPopMatrix();
  }
}


//*************************************************************************************************
void CPakoon1View::DrawGameMenu() {

  // Check if restart should be disabled
  if(BGame::m_bMultiplayOn) {
    bool bDisableRestart = false;
    for(int i = 0; i < BGame::m_nRemotePlayers; ++i) {
      if(BGame::m_remotePlayer[i].m_state == BRemotePlayer::WANTS_TO_SELECT_NEW_RACE) {
        bDisableRestart = true;
        break;
      }
    }
    BGame::m_menuGame.m_items[1].m_bDisabled = bDisableRestart;
  }

  // Main menu
  BGame::m_menuGame.m_listMenu.DrawAt(m_rectWnd.w / 2, 
                                      m_rectWnd.h / 2, 
                                      BTextRenderer::ALIGN_CENTER, 
                                      1, 
                                      1, 
                                      1, 
                                      false,
                                      false,
                                      true,
                                      &(BGame::m_menuGame));
}



//*************************************************************************************************
void CPakoon1View::DrawGoalArrow(BVector vGoal) {

  BCamera *pCamera = BGame::GetSimulation()->GetCamera();
  BScene  *pScene  = BGame::GetSimulation()->GetScene();

  GLdouble modelMatrix[16];
  GLdouble projMatrix[16];
  GLint    viewport[4];
  BVector  vCarOnScreen;

  glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
  glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
  glGetIntegerv(GL_VIEWPORT, viewport);

  BVector vPlace = pCamera->m_vLocation + 
                   pCamera->m_orientation.m_vForward * 100.0 +
                   pCamera->m_orientation.m_vUp * 58.0;

  BOrientation orient;
  orient.m_vForward = vGoal - BGame::GetSimulation()->GetVehicle()->m_vLocation;
  orient.m_vForward.ToUnitLength();
  orient.m_vRight   = orient.m_vForward.CrossProduct(BVector(0, 0, -1));
  orient.m_vRight.ToUnitLength();
  orient.m_vUp      = orient.m_vRight.CrossProduct(orient.m_vForward);
  orient.m_vUp.ToUnitLength();

  glDisable(GL_DEPTH_TEST);  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  double dAlpha = 0.2;
  if(BGame::m_bNight) {
    dAlpha = 0.5;
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
  }

  glPushMatrix();

  GLdouble mtxArrow[16];

  mtxArrow[ 0] = orient.m_vRight.m_dX;
  mtxArrow[ 1] = orient.m_vRight.m_dY;
  mtxArrow[ 2] = orient.m_vRight.m_dZ;

  mtxArrow[ 4] = orient.m_vForward.m_dX;
  mtxArrow[ 5] = orient.m_vForward.m_dY;
  mtxArrow[ 6] = orient.m_vForward.m_dZ;

  mtxArrow[ 8] = -orient.m_vUp.m_dX;
  mtxArrow[ 9] = -orient.m_vUp.m_dY;
  mtxArrow[10] = -orient.m_vUp.m_dZ;

  mtxArrow[12] = vPlace.m_dX;
  mtxArrow[13] = vPlace.m_dY;
  mtxArrow[14] = vPlace.m_dZ;

  mtxArrow[ 3] = 0.0;
  mtxArrow[ 7] = 0.0;
  mtxArrow[11] = 0.0;

  mtxArrow[15] = 1.0;

  glMultMatrixd(mtxArrow);

  double dLen, dHeight, dWidth;
  dLen    = 4.0;
  dHeight = 1.0;
  dWidth  = 2.0;

  if(BGame::m_gameMode == BGame::SLALOM) {
    if(pScene->m_slalom.m_nCurrentPole >= pScene->m_slalom.m_nSlalomPoles) {
      OpenGLHelpers::SetColorFull(0, 0, 1, dAlpha);
    } else {
      if(pScene->m_slalom.m_slalomPole[pScene->m_slalom.m_nCurrentPole].m_bPassFromRight) {
        OpenGLHelpers::SetColorFull(0, 1, 0, dAlpha);
      } else {
        OpenGLHelpers::SetColorFull(1, 0, 0, dAlpha);
      }
    }
  } else {
    OpenGLHelpers::SetColorFull(0, 0, 1, dAlpha);
  }

  glDisable(GL_CULL_FACE);

  glBegin(GL_QUADS);

  glNormal3f(-0.7, 0.7, 0);
  glVertex3f(-dWidth * 2.0, dLen,        dHeight);
  glVertex3f( 0.0,          dLen * 2.0,  dHeight);
  glVertex3f( 0.0,          dLen * 2.0, -dHeight);
  glVertex3f(-dWidth * 2.0, dLen,       -dHeight);

  glNormal3f( 0.7, 0.7, 0);
  glVertex3f( dWidth * 2.0, dLen,        dHeight);
  glVertex3f( 0.0,          dLen * 2.0,  dHeight);
  glVertex3f( 0.0,          dLen * 2.0, -dHeight);
  glVertex3f( dWidth * 2.0, dLen,       -dHeight);

  glEnd();

  glBegin(GL_TRIANGLES);

  glNormal3f(0, 0, -1);
  glVertex3f(-dWidth * 2.0, dLen,       -dHeight);
  glVertex3f( 0.0,          dLen * 2.0, -dHeight);
  glVertex3f( dWidth * 2.0, dLen,       -dHeight);

  glNormal3f(0, 0, 1);
  glVertex3f(-dWidth * 2.0, dLen,       dHeight);
  glVertex3f( 0.0,          dLen * 2.0, dHeight);
  glVertex3f( dWidth * 2.0, dLen,       dHeight);

  glEnd();

  glBegin(GL_QUADS);

  glNormal3f(0, 1, 0);
  glVertex3f(-dWidth,       dLen,  dHeight);
  glVertex3f(-dWidth * 2.0, dLen,  dHeight);
  glVertex3f(-dWidth * 2.0, dLen, -dHeight);
  glVertex3f(-dWidth,       dLen, -dHeight);

  glVertex3f(dWidth,       dLen,  dHeight);
  glVertex3f(dWidth * 2.0, dLen,  dHeight);
  glVertex3f(dWidth * 2.0, dLen, -dHeight);
  glVertex3f(dWidth,       dLen, -dHeight);

  glNormal3f(-1, 0, 0);
  glVertex3f(-dWidth,  dLen, -dHeight);
  glVertex3f(-dWidth,  dLen,  dHeight);
  glVertex3f(-dWidth, -dLen,  dHeight);
  glVertex3f(-dWidth, -dLen, -dHeight);

  glNormal3f(1, 0, 0);
  glVertex3f(dWidth,  dLen, -dHeight);
  glVertex3f(dWidth,  dLen,  dHeight);
  glVertex3f(dWidth, -dLen,  dHeight);
  glVertex3f(dWidth, -dLen, -dHeight);

  glNormal3f(0, 0, -1);
  glVertex3f(-dWidth,  dLen, -dHeight);
  glVertex3f( dWidth,  dLen, -dHeight);
  glVertex3f( dWidth, -dLen, -dHeight);
  glVertex3f(-dWidth, -dLen, -dHeight);

  glNormal3f(0, 0, 1);
  glVertex3f(-dWidth,  dLen, dHeight);
  glVertex3f( dWidth,  dLen, dHeight);
  glVertex3f( dWidth, -dLen, dHeight);
  glVertex3f(-dWidth, -dLen, dHeight);

  glNormal3f(0, -1, 0);
  glVertex3f(-dWidth, -dLen, -dHeight);
  glVertex3f(-dWidth, -dLen,  dHeight);
  glVertex3f( dWidth, -dLen,  dHeight);
  glVertex3f( dWidth, -dLen, -dHeight);

  glEnd();

  glPopMatrix();

  BVector vVehicle = BGame::GetSimulation()->GetVehicle()->m_vLocation;
  if((vVehicle.m_dY > (pScene->m_vGoal.m_dY - 2000.0)) && 
     (vVehicle.m_dY < pScene->m_vGoal.m_dY)) {
    // Draw goal indicator also
    OpenGLHelpers::SetColorFull(0, 0, 1, dAlpha);
    glDisable(GL_FOG);
    glBegin(GL_QUAD_STRIP);
    glNormal3f(0, 0, -1);
    for(double dAngle = 0; dAngle < g_cdPI * 2.0; dAngle += g_cdPI * 2.0 / 20.0) {
      glVertex3f(pScene->m_vGoal.m_dX + cos(dAngle) * 40.0, 
                 pScene->m_vGoal.m_dY + sin(dAngle) * 40.0 + 40.0,
                 0.0);
      glVertex3f(pScene->m_vGoal.m_dX + cos(dAngle) * 50.0,
                 pScene->m_vGoal.m_dY + sin(dAngle) * 50.0 + 40.0,
                 0.0);
    }
    glVertex3f(pScene->m_vGoal.m_dX + cos(0) * 40.0, 
               pScene->m_vGoal.m_dY + sin(0) * 40.0 + 40.0,
               0.0);
    glVertex3f(pScene->m_vGoal.m_dX + cos(0) * 50.0,
               pScene->m_vGoal.m_dY + sin(0) * 50.0 + 40.0,
               0.0);
    glEnd();
    glEnable(GL_FOG);
  }

  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);  

  if(vVehicle.m_dY > (pScene->m_vGoal.m_dY - 1100.0)) {
    OpenGLHelpers::SetColorFull(0.5, 0.5, 1, 1);
    GLUquadricObj* pQuad = gluNewQuadric();
    glPushMatrix();
    glTranslatef(pScene->m_vGoal.m_dX - 25.0,
                 pScene->m_vGoal.m_dY,
                 -20.0);
    gluCylinder(pQuad, 0.8, 0.8, 20.0, 20, 1);
    glTranslatef(50.0, 0.0, 0.0);
    gluCylinder(pQuad, 0.8, 0.8, 20.0, 20, 1);
    glPopMatrix();
    gluDeleteQuadric(pQuad);
  }

}


//*************************************************************************************************
void CPakoon1View::DrawDisqualified() {
  OpenGLHelpers::SetColorFull(1, 0.2, 0.2, 1);
  OpenGLHelpers::DrawTexturedRectangle(m_rectWnd.w / 2.0 - 249 / 2.0, 
                                       m_rectWnd.h - m_rectWnd.h / 3.0, 
                                       0, 
                                       56, 
                                       249, 
                                       58, 
                                       512, 
                                       256);
}


//*************************************************************************************************
void CPakoon1View::DrawExtraScreenTexts() {
  if(BGame::m_cOnScreenInfo & (~BGame::FPS)) {

    OpenGLHelpers::SwitchToTexture(0);
    if((BGame::m_cOnScreenInfo & BGame::TIMER_STARTED) ||
       (BGame::m_cOnScreenInfo & BGame::REF_TIME)) {
      BTextures::Use(BTextures::ONSCREEN_GAME_TEXTS);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // not mipmapped

    if(BGame::m_cOnScreenInfo & BGame::TIMER_STARTED) {
      // TIMER STARTED
      double dAlpha = 1.0;
      clock_t clockNow = clock();
      if((clockNow - m_clockTimerStart) > (CLOCKS_PER_SEC * 2)) {
        BGame::m_cOnScreenInfo -= BGame::TIMER_STARTED;
        dAlpha = 0.0;
      } else {
        dAlpha = 1.0 - double(clockNow - m_clockTimerStart) / double(CLOCKS_PER_SEC * 2);
      }
      OpenGLHelpers::SetColorFull(0.5, 0.5, 1, dAlpha);
      OpenGLHelpers::DrawTexturedRectangle(m_rectWnd.w / 2.0 - 166 / 2.0, 
                                           m_rectWnd.h - m_rectWnd.h / 3.0 - 40, 
                                           346, 
                                           127, 
                                           166, 
                                           40, 
                                           512, 
                                           128);
    } 
    if(BGame::m_cOnScreenInfo & BGame::REF_TIME) {
      // REFERENCE TIME
      double dAlpha = 1.0;
      clock_t clockNow = clock();
      if((clockNow - m_clockTimerStart) > (CLOCKS_PER_SEC * 4)) {
        BGame::m_cOnScreenInfo -= BGame::REF_TIME;
        dAlpha = 0.0;
      } else {
        dAlpha = 1.0 - double(clockNow - m_clockTimerStart) / double(CLOCKS_PER_SEC * 4);
      }
      if((BGame::m_sRefTime.length() > 0) && (BGame::m_sRefTime.at(0) == '-')) {
        OpenGLHelpers::SetColorFull(0, 1, 0, dAlpha);
      } else {
        OpenGLHelpers::SetColorFull(1, 0, 0, dAlpha);
      }
      DrawGameStringAt(BGame::m_sRefTime, m_rectWnd.w / 2.0 - 70, 
                       m_rectWnd.h - m_rectWnd.h / 6.0 - 40);
    }
    
    if(BGame::m_cOnScreenInfo & ~(BGame::TIMER_STARTED | BGame::REF_TIME)) {
      BTextures::Use(BTextures::EXTRA_SCREEN_MESSAGES);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // not mipmapped
    }

    if(BGame::m_cOnScreenInfo & BGame::DISQUALIFIED_WRONG_SIDE) {
      // DISQUALIFIED!: (you missed a pole)
      DrawDisqualified();
      OpenGLHelpers::DrawTexturedRectangle(m_rectWnd.w / 2.0 - 196 / 2.0, 
                                           m_rectWnd.h - m_rectWnd.h / 3.0 - 40, 
                                           0, 
                                           168, 
                                           196, 
                                           46, 
                                           512, 
                                           256);
    } 
    if(BGame::m_cOnScreenInfo & BGame::DISQUALIFIED_OUT_OF_TIME) {
      // DISQUALIFIED!: (out of time)
      DrawDisqualified();
      OpenGLHelpers::DrawTexturedRectangle(m_rectWnd.w / 2.0 - 135 / 2.0, 
                                           m_rectWnd.h - m_rectWnd.h / 3.0 - 40, 
                                           0, 
                                           211, 
                                           135, 
                                           44, 
                                           512, 
                                           256);
    } 
    if(BGame::m_cOnScreenInfo & BGame::DISQUALIFIED_GOAL) {
      // DISQUALIFIED!: (you missed the goal)
      DrawDisqualified();
      OpenGLHelpers::DrawTexturedRectangle(m_rectWnd.w / 2.0 - 218 / 2.0, 
                                           m_rectWnd.h - m_rectWnd.h / 3.0 - 40, 
                                           0, 
                                           255, 
                                           218, 
                                           45, 
                                           512, 
                                           256);
    } 
    
    if(BGame::m_cOnScreenInfo & BGame::NEW_RECORD) {
      BTextures::Use(BTextures::EXTRA_SCREEN_MESSAGES);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // not mipmapped
      // NEW TRACK RECORD!
      double dAlpha = 1.0;
      if((clock() % (CLOCKS_PER_SEC / 2)) > (CLOCKS_PER_SEC / 4)) {
        dAlpha = 0.25;
      }
      OpenGLHelpers::SetColorFull(0.2, 1, 0.2, dAlpha);
      OpenGLHelpers::DrawTexturedRectangle(m_rectWnd.w / 2.0 - 359 / 2.0, 
                                           m_rectWnd.h - m_rectWnd.h / 3.0 + 20, 
                                           0, 
                                           123, 
                                           359, 
                                           60, 
                                           512, 
                                           256);
    }

    glDisable(GL_TEXTURE_2D);
  }  

  if(BGame::m_bMultiplayOn) {

    int i, j;

    // Find out race positions
    BGame::m_remotePlayer[BGame::GetMyPlace()].m_vLocation = BGame::GetSimulation()->GetVehicle()->m_vLocation;
    for(i = 0; i < BGame::m_nRemotePlayers; ++i) {
      if(BGame::m_remotePlayer[i].m_state == BRemotePlayer::RACING) {
        BGame::m_remotePlayer[i].m_nRacePosition = 0;
      }
    }

    for(i = 0; i < BGame::m_nRemotePlayers; ++i) {

      if(BGame::m_remotePlayer[i].m_state != BRemotePlayer::RACING) {
        continue;
      }

      int    nCandidate = 0;
      double dBestY = -900.0;
      for(j = 0; j < BGame::m_nRemotePlayers; ++j) {
        if((BGame::m_remotePlayer[j].m_nRacePosition == 0) && 
           (BGame::m_remotePlayer[j].m_vLocation.m_dY > dBestY) && 
           (BGame::m_remotePlayer[j].m_state == BRemotePlayer::RACING)) {
          nCandidate = j;
          dBestY = BGame::m_remotePlayer[j].m_vLocation.m_dY;
        }
      }
      BGame::m_remotePlayer[nCandidate].m_nRacePosition = i + 1;
    }

    // Draw multiplayers
    BUI::TextRenderer()->StartRenderingText();
    for(i = 0; i < BGame::m_nRemotePlayers; ++i) {

      string sExtra = "";

      switch(BGame::m_remotePlayer[i].m_state) {
        case BRemotePlayer::WANTS_TO_SELECT_NEW_RACE:
          sExtra = "Waiting in main menu";
          break;
        case BRemotePlayer::PREPARING_TO_RACE:
          sExtra = "Preparing to race";
          break;
        case BRemotePlayer::WAITING_FOR_RACE:
          sExtra = "Ready to start race";
          break;
        case BRemotePlayer::RACING:
          if(BGame::m_remotePlayer[i].m_nRacePosition >= 4) {
			  stringstream val;
			  val << BGame::m_remotePlayer[i].m_nRacePosition << "th";
			  sExtra = val.str();
          } else if(BGame::m_remotePlayer[i].m_nRacePosition == 3) {
            sExtra = "3rd";
          } else if(BGame::m_remotePlayer[i].m_nRacePosition == 2) {
            sExtra = "2nd";
          } else if(BGame::m_remotePlayer[i].m_nRacePosition == 1) {
            sExtra = "1st!";
          }
          break;
        case BRemotePlayer::FINISHED:
          if(BGame::m_remotePlayer[i].m_nRacePosition >= 4) {
			  stringstream val;
			  val << "Finished " << BGame::m_remotePlayer[i].m_nRacePosition << "th";
			  sExtra = val.str();
          } else if(BGame::m_remotePlayer[i].m_nRacePosition == 3) {
            sExtra = "Finished 3rd";
          } else if(BGame::m_remotePlayer[i].m_nRacePosition == 2) {
            sExtra = "Finished 2nd";
          } else if(BGame::m_remotePlayer[i].m_nRacePosition == 1) {
            sExtra = "Finished 1st!";
          }
          break;
        case BRemotePlayer::MISSED_GOAL:
          sExtra = "DISQUALIFIED! (Missed goal)";
          break;
        case BRemotePlayer::MISSED_POLE:
          sExtra = "DISQUALIFIED! (Missed a pole)";
          break;
      }

      double dR, dG, dB;
      BGame::GetMultiplayerColor(i, dR, dG, dB);
      string sPlayer;
		sPlayer.assign(BGame::m_remotePlayer[i].m_sName);
		sPlayer.append(": ");
		sPlayer.append(sExtra);

      BUI::TextRenderer()->DrawSmallTextAt(5, 10 + 20 * i, sPlayer, sPlayer.length(), BTextRenderer::ALIGN_LEFT, dR * 0.5, dG * 0.5, dB * 0.5, 1);
    }

    BUI::TextRenderer()->StopRenderingText();
  }

}


//*************************************************************************************************
void CPakoon1View::DrawOnScreenGameTexts(BVector vGoal) {
  // Draw the stats 'n' stuff
  OpenGLHelpers::SwitchToTexture(0);
  BTextures::Use(BTextures::ONSCREEN_GAME_TEXTS);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // not mipmapped

  glPushMatrix();
  glTranslatef(5, -5, 0); // use a little margin for visual pleasense

  OpenGLHelpers::SetColorFull(0.65, 1, 0.65, 1);

  // Race time:
  OpenGLHelpers::DrawTexturedRectangle(0, m_rectWnd.h - 36, 0, 92, 106, 36, 512, 128);

  // Air time:
  OpenGLHelpers::DrawTexturedRectangle(0, m_rectWnd.h - 36 - 74 - 20, 109, 92, 93, 36, 512, 128);

  // Speed:
  OpenGLHelpers::DrawTexturedRectangle(0, m_rectWnd.h - 36 - 74 - 20 - 74 - 20, 140, 0, 135, 36, 512, 128);

  glTranslatef(-10, 0, 0); // use a little margin for visual pleasense

  // Race position:
  OpenGLHelpers::DrawTexturedRectangle(m_rectWnd.w - 144, m_rectWnd.h - 36, 207, 92, 141, 36, 512, 128);

  // Poles passed:
  if(BGame::m_gameMode == BGame::SLALOM) {
    OpenGLHelpers::DrawTexturedRectangle(m_rectWnd.w - 135, m_rectWnd.h - 36 - 74 - 20, 0, 0, 135, 36, 512, 128);
  }

  glTranslatef(10, 0, 0); // use a little margin for visual pleasense

  // *************************************
  // Draw timers and other variable values
  // *************************************

  BScene  *pScene  = BGame::GetSimulation()->GetScene();
  stringstream sRaceTime;
  stringstream sAirTime, sAirTime2;
  stringstream sSpeed;
  int nSecondsAir = (BGame::m_dAirTime) / g_dPhysicsStepsInSecond;
  int n100SecondsAir = (100 * (BGame::m_dAirTime - nSecondsAir * g_dPhysicsStepsInSecond)) / g_dPhysicsStepsInSecond;

  int nSecAirBest = pScene->m_dBestAirTime / g_dPhysicsStepsInSecond;
  int n100SecAirBest = (100 * (pScene->m_dBestAirTime - nSecAirBest * g_dPhysicsStepsInSecond)) / g_dPhysicsStepsInSecond;

  int nMinutesTotal = BGame::m_dRaceTime / g_dPhysicsStepsInSecond / 60;
  int nSecondsTotal = (BGame::m_dRaceTime - (nMinutesTotal * g_dPhysicsStepsInSecond * 60)) / g_dPhysicsStepsInSecond;
  int n100SecondsTotal = (100 * (BGame::m_dRaceTime - (nMinutesTotal * g_dPhysicsStepsInSecond * 60 + nSecondsTotal * g_dPhysicsStepsInSecond))) / g_dPhysicsStepsInSecond;
  if(n100SecondsTotal < 0) {
    nMinutesTotal = -nMinutesTotal;
    n100SecondsTotal = -n100SecondsTotal;
  }

  sRaceTime << nMinutesTotal << ":" << nSecondsTotal << "." << n100SecondsTotal;
  sAirTime << nSecondsAir << "." << n100SecondsAir;
  sAirTime2 << nSecAirBest << "." << n100SecAirBest;

  sSpeed << BGame::GetSimulation()->GetVehicle()->m_dSpeed * 
                         g_dPhysicsStepsInSecond * 
                         3.6;


  // Race time, air time and speed
  double dAlpha = 1.0;
  bool bHurryUp = ((BGame::m_gameMode == BGame::AIRTIME) && (BGame::m_dRaceTime < 15 * g_dPhysicsStepsInSecond));
  if((BGame::m_cOnScreenInfo & BGame::NEW_RECORD) || bHurryUp) {
    // NEW TRACK RECORD!
    if((clock() % (CLOCKS_PER_SEC / 2)) > (CLOCKS_PER_SEC / 4)) {
      dAlpha = 0.25;
    }
    if((BGame::m_gameMode == BGame::SPEEDRACE) || 
       (BGame::m_gameMode == BGame::SLALOM) || 
       bHurryUp) {
      OpenGLHelpers::SetColorFull(0.25, 1, 0.25, dAlpha);
    } else {
      OpenGLHelpers::SetColorFull(0.25, 1, 0.25, 1);
    }
    DrawGameStringAt(sRaceTime.str(), 0, m_rectWnd.h - 36 - 50);
    if((BGame::m_gameMode == BGame::AIRTIME) && !bHurryUp) {
      OpenGLHelpers::SetColorFull(0.25, 1, 0.25, dAlpha);
    } else {
      OpenGLHelpers::SetColorFull(0.25, 1, 0.25, 1);
    }
    DrawGameStringAt(sAirTime.str(), 0, m_rectWnd.h - 36 - 74 - 20 - 50);
    if(BGame::m_gameMode == BGame::AIRTIME) {
      OpenGLHelpers::SetColorFull(0.25, 1, 0.25, 0.15);
      DrawGameStringAt(sAirTime2.str(), 140, m_rectWnd.h - 36 - 74 - 20 - 50);
    }
    OpenGLHelpers::SetColorFull(0.25, 1, 0.25, 1);
    DrawGameStringAt(sSpeed.str(), 0, m_rectWnd.h - 36 - 74 - 20 - 74 - 20 - 50);
  } else {
    OpenGLHelpers::SetColorFull(0.25, 1, 0.25, 1);
    DrawGameStringAt(sRaceTime.str(), 0, m_rectWnd.h - 36 - 50);
    DrawGameStringAt(sAirTime.str(), 0, m_rectWnd.h - 36 - 74 - 20 - 50);
    DrawGameStringAt(sSpeed.str(), 0, m_rectWnd.h - 36 - 74 - 20 - 74 - 20 - 50);
    if(BGame::m_gameMode == BGame::AIRTIME) {
      OpenGLHelpers::SetColorFull(0.25, 1, 0.25, 0.15);
      DrawGameStringAt(sAirTime2.str(), 140, m_rectWnd.h - 36 - 74 - 20 - 50);
    }
  }

  // Race position and poles passed
  glTranslatef(-10, 0, 0); // use a little margin for visual pleasense

  string sPolesPassed;
  string sRacePosition = BGame::m_sRacePosition;

  if(BGame::m_bMultiplayOn) {
    /*
    int nAhead = 0;
    for(int i = 0; i < BGame::m_nRemotePlayers; ++i) {
      if(BGame::m_remotePlayer[i].m_bSelf) {
        continue;
      }
      if(BGame::m_remotePlayer[i].m_vLocation.m_dY > BGame::GetSimulation()->GetVehicle()->m_vLocation.m_dY) {
        ++nAhead;
      }
    }
    */
    // sRacePosition.Format("%d/%d", nAhead + 1, BGame::m_nRemotePlayers);
    stringstream val;
    val << BGame::m_remotePlayer[BGame::GetMyPlace()].m_nRacePosition << "/" << BGame::m_nRemotePlayers;
    sRacePosition = val.str();
  }

  if(BGame::m_gameMode == BGame::SLALOM) {
	  stringstream val;
	  val << pScene->m_slalom.m_nCurrentPole << "/" << pScene->m_slalom.m_nSlalomPoles;
	  sPolesPassed = val.str();
    DrawGameStringAt(sPolesPassed, m_rectWnd.w - sPolesPassed.length() * 28, m_rectWnd.h - 36 - 74 - 20 - 50);
  }

  if(BGame::m_bMultiplayOn) {
    if(!sRacePosition.empty() && sRacePosition.at(0) == '1') {
      OpenGLHelpers::SetColorFull(0.25, 1, 0.25, 1);
    } else if(!sRacePosition.empty() && sRacePosition.at(0) == '2') {
      OpenGLHelpers::SetColorFull(1, 1, 0.25, 1);
    } else if(!sRacePosition.empty() && sRacePosition.at(0) == '3') {
      OpenGLHelpers::SetColorFull(1, 0.6, 0.25, 1);
    } else if(!sRacePosition.empty() && sRacePosition.at(0) == '4') {
      OpenGLHelpers::SetColorFull(1, 0.25, 0.25, 1);
    }
  } else {
    if(!sRacePosition.empty() && sRacePosition.at(0) == '1') {
      OpenGLHelpers::SetColorFull(0.25, 1, 0.25, 1);
    } else {
      OpenGLHelpers::SetColorFull(1, 0.25, 0.25, 1);
    }
  }
  DrawGameStringAt(sRacePosition, m_rectWnd.w - sRacePosition.length() * 28, m_rectWnd.h - 36 - 50);

  glTranslatef(5, 0, 0); // center

  // Draw offset from goal
  if(BGame::m_gameMode != BGame::SLALOM) {
    stringstream sGoal;
    sGoal << vGoal.m_dX - BGame::GetSimulation()->GetVehicle()->m_vLocation.m_dX << "m";
    if(sGoal.str().at(0) != '-') {
      sGoal.str("+");
      sGoal << sGoal;
      glTranslatef(100, 0, 0); // on right
    } else {
      glTranslatef(-100, 0, 0); // on left
    }

    OpenGLHelpers::SetColorFull(0.25, 0.25, 1, 0.5);
    DrawGameStringAt(sGoal.str(), m_rectWnd.w / 2 - sGoal.str().length() * 30 / 2.0, m_rectWnd.h - 55);

    if(sGoal.str().at(0) != '-') {
      glTranslatef(-100, 0, 0); // back to center
    } else {
      glTranslatef(100, 0, 0); // back to center
    }
  }

  // Draw faint fps
  if(BGame::m_cOnScreenInfo & BGame::FPS) {
    stringstream sFPS;
    sFPS << g_dRate;
    glScalef(0.5, 0.5, 0.5);
    glTranslatef(m_rectWnd.w * 2 - sFPS.str().length() * 25 - 5, 10, 0); // use a little margin for visual pleasense
    OpenGLHelpers::SetColorFull(0.25, 0.25, 1.0, 0.5);
    DrawGameStringAt(sFPS.str(), 0, 0);
  }
  
  glPopMatrix();

  glDisable(GL_TEXTURE_2D);

  // Draw multiplay names
  if(BGame::m_bMultiplayOn) {
    BUI::TextRenderer()->StartRenderingText();
    for(int i = 0; i < BGame::m_nRemotePlayers; ++i) {
      if(BGame::m_remotePlayer[i].m_bSelf) {
        continue;
      }
      double dR, dG, dB;
      BGame::GetMultiplayerColor(i, dR, dG, dB);

      double dDist = BGame::m_remotePlayer[i].m_vLocation.m_dY - BGame::GetSimulation()->GetVehicle()->m_vLocation.m_dY;
      stringstream sPos;
      sPos << "(" << ((dDist > 0) ? "ahead" : "behind") << " " << ((dDist > 0) ? dDist : -dDist) << ")";

      double dAlpha = 1.0 - (fabs(dDist) / 1000.0);
      if(dAlpha < 0.2) {
        dAlpha = 0.2;
      }

      double dX = BGame::m_remotePlayer[i].m_vOnScreen.m_dX;

      BUI::TextRenderer()->DrawSmallTextAt(dX,
                                           BGame::m_remotePlayer[i].m_vOnScreen.m_dY + 18,
                                           BGame::m_remotePlayer[i].m_sName,
                                           strlen(BGame::m_remotePlayer[i].m_sName),
                                           BTextRenderer::ALIGN_CENTER,
                                           dR, dG, dB, dAlpha);
      BUI::TextRenderer()->DrawSmallTextAt(dX,
                                           BGame::m_remotePlayer[i].m_vOnScreen.m_dY,
                                           sPos.str(),
                                           sPos.str().length(),
                                           BTextRenderer::ALIGN_CENTER,
                                           dR, dG, dB, 0.6 * dAlpha);
    }
    BUI::TextRenderer()->StopRenderingText();
  }
}


//*************************************************************************************************
void CPakoon1View::DrawGameStringAt(string sTxt, double dX, double dY) {
  static int nNumberStart[10] = {0, 34, 62, 95, 129, 163, 195, 227, 261, 295};
  static int nNumberWidth[10] = {34, 26, 32, 32, 33, 31, 31, 33, 33, 33};
  int n;
  double dXPos = dX;
  for(int i = 0; i < sTxt.length(); ++i) {
    // Render one character
    char c = sTxt.at(i);
    switch(c) {
      case ':':
        OpenGLHelpers::DrawTexturedRectangle(dXPos, dY, 351, 55, 21, 55, 512, 128);
        dXPos += 22;
        break;
      case '.':
        OpenGLHelpers::DrawTexturedRectangle(dXPos, dY, 329, 55, 21, 55, 512, 128);
        dXPos += 21;
        break;
      case '/':
        OpenGLHelpers::DrawTexturedRectangle(dXPos, dY - 5, 372, 60, 30, 60, 512, 128);
        dXPos += 30;
        break;
      case '-':
        OpenGLHelpers::DrawTexturedRectangle(dXPos, dY, 404, 55, 27, 55, 512, 128);
        dXPos += 27;
        break;
      case '+':
        OpenGLHelpers::DrawTexturedRectangle(dXPos, dY, 432, 55, 41, 55, 512, 128);
        dXPos += 41;
        break;
      case 'm':
      case 'M':
        OpenGLHelpers::DrawTexturedRectangle(dXPos, dY, 477, 55, 34, 55, 512, 128);
        dXPos += 34;
        break;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        // draw one number
        n = c - '0';
        OpenGLHelpers::DrawTexturedRectangle(dXPos, dY, nNumberStart[n], 55, nNumberWidth[n] + 1, 55, 512, 128);
        dXPos += nNumberWidth[n];
    }
    dXPos -= 5;
  }
}


//*************************************************************************************************
void CPakoon1View::DrawActiveSlalomPoles() {

  static int nDLPoles = -1;

  if(!BGame::m_bSlalomPolesVisualOK) {
    if(nDLPoles == -1) {
      nDLPoles = glGenLists(2);
    }

    // Draw one pole to display list
    GLUquadricObj* pQuad = gluNewQuadric();
    glNewList(nDLPoles + 1, GL_COMPILE_AND_EXECUTE);
    gluCylinder(pQuad, 0.4, 0.4, 8.5, 20, 1);
    glEndList();
    gluDeleteQuadric(pQuad);

    // Draw all poles to a display list

    BScene *pScene = BGame::GetSimulation()->GetScene();

    glNewList(nDLPoles, GL_COMPILE_AND_EXECUTE);

    OpenGLHelpers::SetColorFull(1, 1, 1, 1);
    
    if(pScene->m_slalom.m_bValid) {

      // Poles

      int i;
      for(i = 0; i < pScene->m_slalom.m_nSlalomPoles; ++i) {
        // Draw pole
        BVector vLoc = pScene->m_slalom.m_slalomPole[i].m_vLocation;
        glPushMatrix();
        glTranslatef(vLoc.m_dX, vLoc.m_dY, vLoc.m_dZ - 8.2);
        glCallList(nDLPoles + 1);
        glPopMatrix();
      }

      // Flags
      glNormal3f(0, 0, -1);
      OpenGLHelpers::SetColorFull(0, 0.5, 0, 1);

      for(i = 0; i < pScene->m_slalom.m_nSlalomPoles; ++i) {
        BVector vLoc = pScene->m_slalom.m_slalomPole[i].m_vLocation;

        if(pScene->m_slalom.m_slalomPole[i].m_bPassFromRight) {
          glBegin(GL_TRIANGLES);
          glVertex3f(vLoc.m_dX + 0.25, vLoc.m_dY, vLoc.m_dZ - 8.0);
          glVertex3f(vLoc.m_dX + 4.0,  vLoc.m_dY, vLoc.m_dZ - 7.0);
          glVertex3f(vLoc.m_dX + 0.25, vLoc.m_dY, vLoc.m_dZ - 6.0);
          glEnd();
        }
      }

      OpenGLHelpers::SetColorFull(0.5, 0, 0, 1);

      for(i = 0; i < pScene->m_slalom.m_nSlalomPoles; ++i) {
        BVector vLoc = pScene->m_slalom.m_slalomPole[i].m_vLocation;

        if(!pScene->m_slalom.m_slalomPole[i].m_bPassFromRight) {
          glBegin(GL_TRIANGLES);
          glVertex3f(vLoc.m_dX - 0.25, vLoc.m_dY, vLoc.m_dZ - 8.0);
          glVertex3f(vLoc.m_dX - 4.0,  vLoc.m_dY, vLoc.m_dZ - 7.0);
          glVertex3f(vLoc.m_dX - 0.25, vLoc.m_dY, vLoc.m_dZ - 6.0);
          glEnd();
        }
      }
    }

    glEndList();

  } else {
    // Just call the valid display list
    glCallList(nDLPoles);
  }
}



//*************************************************************************************************
void CPakoon1View::DrawGhostCar(int &nNowFrame, BRaceRecord *pRaceRecord) {

  // Find correct frame
  while(((nNowFrame + 1) < pRaceRecord->m_nNextSlot) && 
        (pRaceRecord->m_frames[nNowFrame + 1].m_dTime < BGame::m_dRaceTime)) {
    ++nNowFrame;
  }

  // Draw "ghost vehicle"

  BVector vLoc = pRaceRecord->m_frames[nNowFrame].m_vLocation;
  BVector vToNext = pRaceRecord->m_frames[nNowFrame + 1].m_vLocation - vLoc;
  double dFraction = (BGame::m_dRaceTime - pRaceRecord->m_frames[nNowFrame].m_dTime) /
                     (pRaceRecord->m_frames[nNowFrame + 1].m_dTime - pRaceRecord->m_frames[nNowFrame].m_dTime);
  vLoc = vLoc + vToNext * dFraction;

  if(vLoc.m_dY > BGame::GetSimulation()->GetVehicle()->m_vLocation.m_dY) {
    BGame::m_sRacePosition = "2/2";
  } else {
    BGame::m_sRacePosition = "1/2";
  }

  glDepthMask(GL_FALSE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

  double dAlpha = 0.1;

  if(BGame::m_bNight) {
    dAlpha = 0.5;
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
  }

  BOrientation orient;
  orient.m_vForward = pRaceRecord->m_frames[nNowFrame].m_vForward * (1.0 - dFraction) + pRaceRecord->m_frames[nNowFrame + 1].m_vForward * dFraction;
  orient.m_vRight   = pRaceRecord->m_frames[nNowFrame].m_vRight * (1.0 - dFraction) + pRaceRecord->m_frames[nNowFrame].m_vRight * dFraction;
  orient.m_vForward.ToUnitLength();
  orient.m_vRight.ToUnitLength();
  orient.m_vUp      = orient.m_vRight.CrossProduct(orient.m_vForward);
  orient.m_vUp.ToUnitLength();

  glPushMatrix();

  GLdouble mtxVehicle[16];

  mtxVehicle[ 0] = orient.m_vRight.m_dX;
  mtxVehicle[ 1] = orient.m_vRight.m_dY;
  mtxVehicle[ 2] = orient.m_vRight.m_dZ;

  mtxVehicle[ 4] = orient.m_vForward.m_dX;
  mtxVehicle[ 5] = orient.m_vForward.m_dY;
  mtxVehicle[ 6] = orient.m_vForward.m_dZ;

  mtxVehicle[ 8] = -orient.m_vUp.m_dX;
  mtxVehicle[ 9] = -orient.m_vUp.m_dY;
  mtxVehicle[10] = -orient.m_vUp.m_dZ;

  mtxVehicle[12] = vLoc.m_dX;
  mtxVehicle[13] = vLoc.m_dY;
  mtxVehicle[14] = vLoc.m_dZ - 0.4;

  mtxVehicle[ 3] = 0.0;
  mtxVehicle[ 7] = 0.0;
  mtxVehicle[11] = 0.0;

  mtxVehicle[15] = 1.0;

  glMultMatrixd(mtxVehicle);

  double dLen, dHeight, dWidth;
  dLen    = pRaceRecord->m_dCarLength / 2.0;
  dHeight = pRaceRecord->m_dCarHeight / 2.0;
  dWidth  = pRaceRecord->m_dCarWidth / 2.0;

  OpenGLHelpers::SetColorFull(1, 0, 0, dAlpha);

  glDisable(GL_CULL_FACE);

  glBegin(GL_QUADS);

  glNormal3f(-0.7, 0.7, 0);
  glVertex3f(-dWidth * 2.0, dLen,        dHeight);
  glVertex3f( 0.0,          dLen * 2.0,  dHeight);
  glVertex3f( 0.0,          dLen * 2.0, -dHeight);
  glVertex3f(-dWidth * 2.0, dLen,       -dHeight);

  glNormal3f( 0.7, 0.7, 0);
  glVertex3f( dWidth * 2.0, dLen,        dHeight);
  glVertex3f( 0.0,          dLen * 2.0,  dHeight);
  glVertex3f( 0.0,          dLen * 2.0, -dHeight);
  glVertex3f( dWidth * 2.0, dLen,       -dHeight);

  glEnd();

  glBegin(GL_TRIANGLES);

  glNormal3f(0, 0, -1);
  glVertex3f(-dWidth * 2.0, dLen,       -dHeight);
  glVertex3f( 0.0,          dLen * 2.0, -dHeight);
  glVertex3f( dWidth * 2.0, dLen,       -dHeight);

  glNormal3f(0, 0, 1);
  glVertex3f(-dWidth * 2.0, dLen,       dHeight);
  glVertex3f( 0.0,          dLen * 2.0, dHeight);
  glVertex3f( dWidth * 2.0, dLen,       dHeight);

  glEnd();

  glBegin(GL_QUADS);

  glNormal3f(0, 1, 0);
  glVertex3f(-dWidth,       dLen,  dHeight);
  glVertex3f(-dWidth * 2.0, dLen,  dHeight);
  glVertex3f(-dWidth * 2.0, dLen, -dHeight);
  glVertex3f(-dWidth,       dLen, -dHeight);

  glVertex3f(dWidth,       dLen,  dHeight);
  glVertex3f(dWidth * 2.0, dLen,  dHeight);
  glVertex3f(dWidth * 2.0, dLen, -dHeight);
  glVertex3f(dWidth,       dLen, -dHeight);

  glNormal3f(-1, 0, 0);
  glVertex3f(-dWidth,  dLen, -dHeight);
  glVertex3f(-dWidth,  dLen,  dHeight);
  glVertex3f(-dWidth, -dLen,  dHeight);
  glVertex3f(-dWidth, -dLen, -dHeight);

  glNormal3f(1, 0, 0);
  glVertex3f(dWidth,  dLen, -dHeight);
  glVertex3f(dWidth,  dLen,  dHeight);
  glVertex3f(dWidth, -dLen,  dHeight);
  glVertex3f(dWidth, -dLen, -dHeight);

  glNormal3f(0, 0, -1);
  glVertex3f(-dWidth,  dLen, -dHeight);
  glVertex3f( dWidth,  dLen, -dHeight);
  glVertex3f( dWidth, -dLen, -dHeight);
  glVertex3f(-dWidth, -dLen, -dHeight);

  glNormal3f(0, 0, 1);
  glVertex3f(-dWidth,  dLen, dHeight);
  glVertex3f( dWidth,  dLen, dHeight);
  glVertex3f( dWidth, -dLen, dHeight);
  glVertex3f(-dWidth, -dLen, dHeight);

  glNormal3f(0, -1, 0);
  glVertex3f(-dWidth, -dLen, -dHeight);
  glVertex3f(-dWidth, -dLen,  dHeight);
  glVertex3f( dWidth, -dLen,  dHeight);
  glVertex3f( dWidth, -dLen, -dHeight);

  glEnd();

  glPopMatrix();

  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE);
}








GLint g_nMatrixMode;

//*************************************************************************************************
void CPakoon1View::Setup2DRendering() {
  // Setup 2D projection

  glGetIntegerv(GL_MATRIX_MODE, &g_nMatrixMode);

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

  //GetClientRect(&m_rectWnd); //FIXME

  // Set up projection geometry so that we can use screen coordinates
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, (GLfloat) m_rectWnd.w, 0, (GLfloat) m_rectWnd.h);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glViewport(0, 0, (GLint) m_rectWnd.w, (GLint) m_rectWnd.h);

  OpenGLHelpers::SetColorFull(1, 1, 1, 1);
  
  GLfloat fLight1BlackG[ 4];
  GLfloat fLight1AmbientG[ 4];
  fLight1AmbientG[0] = 1;
  fLight1AmbientG[1] = 1;
  fLight1AmbientG[2] = 1;
  fLight1AmbientG[3] = 1;

  fLight1BlackG[0] = 0;
  fLight1BlackG[1] = 0;
  fLight1BlackG[2] = 0;
  fLight1BlackG[3] = 1;
  glLightfv( GL_LIGHT0, GL_AMBIENT,  fLight1AmbientG);
  glLightfv( GL_LIGHT0, GL_DIFFUSE,  fLight1BlackG);
  glLightfv( GL_LIGHT0, GL_SPECULAR, fLight1BlackG);
}


//*************************************************************************************************
void CPakoon1View::End2DRendering() {
  glEnable(GL_CULL_FACE);

  GLfloat fLight1AmbientG[ 4];
  fLight1AmbientG[0] = 0.4f;
  fLight1AmbientG[1] = 0.4f;
  fLight1AmbientG[2] = 0.4f;
  fLight1AmbientG[3] = 0.0f;
  glLightfv( GL_LIGHT0, GL_AMBIENT,  fLight1AmbientG);

  glMatrixMode(g_nMatrixMode);
}






















//*************************************************************************************************
void CPakoon1View::DrawQuickHelp() {
  // Draw quick help as a texture
  OpenGLHelpers::SetColorFull(1, 1, 1, 1);

  OpenGLHelpers::SwitchToTexture(0);
  if(BGame::GetSceneEditor()->IsActive()) {
    BTextures::Use(BTextures::QUICK_HELP_SCENE_EDITOR);
  } else {
    BTextures::Use(BTextures::QUICK_HELP);
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(0, 1);
  glVertex3f((m_rectWnd.w - 512) / 2, m_rectWnd.h - (m_rectWnd.h - 512) / 2, 0);
  OpenGLHelpers::SetTexCoord(0, 0);
  glVertex3f((m_rectWnd.w - 512) / 2, (m_rectWnd.h - 512) / 2, 0);
  OpenGLHelpers::SetTexCoord(1, 1);
  glVertex3f((m_rectWnd.w - 512) / 2 + 512, m_rectWnd.h - (m_rectWnd.h - 512) / 2, 0);
  OpenGLHelpers::SetTexCoord(1, 0);
  glVertex3f((m_rectWnd.w - 512) / 2 + 512, (m_rectWnd.h - 512) / 2, 0);
  glEnd();

}



//*************************************************************************************************
void CPakoon1View::DrawNavSat() {
  // Draw navsat as a texture
  OpenGLHelpers::SetColorFull(1, 1, 1, 1);
  
  // NavSat window
  glPushMatrix();
  glTranslated(7, 7, 0);
  OpenGLHelpers::SwitchToTexture(0);
  int nRes = BGame::GetNavSat()->ActivateCurrentMapTexture();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  double dMaxTex = (double(nRes) - 1.0) / double(nRes);
  if(nRes == 256) {
    dMaxTex = 1.0;
  }
  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(0, dMaxTex);
  glVertex3f(0, 256, 0);
  OpenGLHelpers::SetTexCoord(0, 0);
  glVertex3f(0, 0, 0);
  OpenGLHelpers::SetTexCoord(dMaxTex, dMaxTex);
  glVertex3f(256, 256, 0);
  OpenGLHelpers::SetTexCoord(dMaxTex, 0);
  glVertex3f(256, 0, 0);
  glEnd();
  glDisable(GL_TEXTURE_2D);

  // Draw crosses on "ground glass"
  OpenGLHelpers::SetColorFull(0, 0, 0, 0.3);
  for(double y = 0; y < 128.0; y += 53.0) {
    for(double x = 0; x < 128.0; x += 53.0) {
      glBegin(GL_LINES);
      glVertex3f(128 + x - 10.0, 128 + y, 0);
      glVertex3f(128 + x + 10.0, 128 + y, 0);
      glVertex3f(128 + x, 128 + y - 10.0, 0);
      glVertex3f(128 + x, 128 + y + 10.0, 0);

      if(x != 0) {
        glVertex3f(128 - x - 10.0, 128 + y, 0);
        glVertex3f(128 - x + 10.0, 128 + y, 0);
        glVertex3f(128 - x, 128 + y - 10.0, 0);
        glVertex3f(128 - x, 128 + y + 10.0, 0);
      }

      if(y != 0) {
        glVertex3f(128 + x - 10.0, 128 - y, 0);
        glVertex3f(128 + x + 10.0, 128 - y, 0);
        glVertex3f(128 + x, 128 - y - 10.0, 0);
        glVertex3f(128 + x, 128 - y + 10.0, 0);
      }

      if((x != 0) && (y != 0)) {
        glVertex3f(128 - x - 10.0, 128 - y, 0);
        glVertex3f(128 - x + 10.0, 128 - y, 0);
        glVertex3f(128 - x, 128 - y - 10.0, 0);
        glVertex3f(128 - x, 128 - y + 10.0, 0);
      }

      glEnd();
    }
  }

  // Draw panel over image and indicators
  OpenGLHelpers::SetColorFull(1, 1, 1, 1);
  glEnable(GL_TEXTURE_2D);
  OpenGLHelpers::SwitchToTexture(0);
  BTextures::Use(BTextures::PANEL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  // Draw car indicator
  double dAngle;
  BVector vNorth(0, 1, 0);
  BVector vEast(1, 0, 0);
  double dCos = vNorth.ScalarProduct(m_game.GetSimulation()->GetVehicle()->m_orientation.m_vForward);
  double dTmp = vEast.ScalarProduct(m_game.GetSimulation()->GetVehicle()->m_orientation.m_vForward);
  if(dTmp > 0.0) {
    dAngle = -acos(dCos);
  } else {
    dAngle = acos(dCos);
  }
  dAngle = dAngle / 3.141592654 * 180.0;
  glPopMatrix();

  // Panel (in 4 parts to exclude the center text)
  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(120.0/512.0, (511.0-94.0)/512.0);
  glVertex3f(0, 298, 0);
  OpenGLHelpers::SetTexCoord(120.0/512.0, (511.0-145.0)/512.0);
  glVertex3f(0, 298 - 51, 0);
  OpenGLHelpers::SetTexCoord(392.0/512.0, (511.0-94.0)/512.0);
  glVertex3f(272, 298, 0);
  OpenGLHelpers::SetTexCoord(392.0/512.0, (511.0-145.0)/512.0);
  glVertex3f(272, 298 - 51, 0);
  glEnd();

  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(120.0/512.0, (511.0-145.0)/512.0);
  glVertex3f(0, 298 - 51, 0);
  OpenGLHelpers::SetTexCoord(120.0/512.0, (511.0-352.0)/512.0);
  glVertex3f(0, 298 - 278, 0);
  OpenGLHelpers::SetTexCoord(138.0/512.0, (511.0-145.0)/512.0);
  glVertex3f(18, 298 - 51, 0);
  OpenGLHelpers::SetTexCoord(138.0/512.0, (511.0-352.0)/512.0);
  glVertex3f(18, 298 - 278, 0);
  glEnd();

  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(375.0/512.0, (511.0-145.0)/512.0);
  glVertex3f(255, 298 - 51, 0);
  OpenGLHelpers::SetTexCoord(375.0/512.0, (511.0-372.0)/512.0);
  glVertex3f(255, 298 - 278, 0);
  OpenGLHelpers::SetTexCoord(393.0/512.0, (511.0-145.0)/512.0);
  glVertex3f(255+18, 298 - 51, 0);
  OpenGLHelpers::SetTexCoord(393.0/512.0, (511.0-372.0)/512.0);
  glVertex3f(255+18, 298 - 278, 0);
  glEnd();

  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(120.0/512.0, (511.0-372.0)/512.0);
  glVertex3f(0, 20, 0);
  OpenGLHelpers::SetTexCoord(120.0/512.0, (511.0-392.0)/512.0);
  glVertex3f(0, 0, 0);
  OpenGLHelpers::SetTexCoord(392.0/512.0, (511.0-372.0)/512.0);
  glVertex3f(272, 20, 0);
  OpenGLHelpers::SetTexCoord(392.0/512.0, (511.0-392.0)/512.0);
  glVertex3f(272, 0, 0);
  glEnd();

  // Draw handle
  glPushMatrix();
  glTranslated(135, 187, 0);
  glRotated(BGame::m_dNavSatHandleAngle, 0, 0, 1);
  glTranslated(0, 90.5, 0);
  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(423.0/512.0, (511.0-176.0)/512.0);
  glVertex3f(-12, 64, 0);
  OpenGLHelpers::SetTexCoord(423.0/512.0, (511.0-240.0)/512.0);
  glVertex3f(-12, 0, 0);
  OpenGLHelpers::SetTexCoord(447.0/512.0, (511.0-176.0)/512.0);
  glVertex3f(12, 64, 0);
  OpenGLHelpers::SetTexCoord(447.0/512.0, (511.0-240.0)/512.0);
  glVertex3f(12, 0, 0);
  glEnd();
  glPopMatrix();

  glDisable(GL_TEXTURE_2D);
}






//*************************************************************************************************
void CPakoon1View::DrawServiceWnd() {
  // Draw service window
  // Draw background and texts
  glDisable(GL_TEXTURE_2D);
  double dAlpha = (-BGame::m_dServiceHandleAngle + 20.0) / 40.0;
  OpenGLHelpers::SetColorFull(0, 0, 0, dAlpha);
  glBegin(GL_TRIANGLE_STRIP);
  glVertex3f(m_rectWnd.w - 7 - 256 - 100, m_rectWnd.h - 256 - 7, 0);
  glVertex3f(m_rectWnd.w - 7, m_rectWnd.h - 256 - 7, 0);
  glVertex3f(m_rectWnd.w - 7 - 256 - 100, m_rectWnd.h - 7, 0);
  glVertex3f(m_rectWnd.w - 7, m_rectWnd.h - 7, 0);
  glEnd();

  OpenGLHelpers::SwitchToTexture(0);
  BTextures::Use(BTextures::PANEL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  DrawServiceWndTexts(m_rectWnd);

  // Panel (draw in four parts to make it wider and to exclude the "It'll cost ya..." text)
  OpenGLHelpers::SetColorFull(1, 1, 1, 1);
  OpenGLHelpers::SwitchToTexture(0);
  BTextures::Use(BTextures::PANEL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(392.0/512.0, (511.0-(391.0-27.0))/512.0);
  glVertex3f(m_rectWnd.w - 27, m_rectWnd.h - 272, 0);
  OpenGLHelpers::SetTexCoord(392.0/512.0, (511.0-391.0)/512.0);
  glVertex3f(m_rectWnd.w, m_rectWnd.h - 272, 0);
  OpenGLHelpers::SetTexCoord(120.0/512.0, (511.0-(391.0-27.0))/512.0);
  glVertex3f(m_rectWnd.w - 27, m_rectWnd.h, 0);
  OpenGLHelpers::SetTexCoord(120.0/512.0, (511.0-391.0)/512.0);
  glVertex3f(m_rectWnd.w, m_rectWnd.h, 0);
  glEnd();

  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(392.0/512.0, (511.0-144.0)/512.0);
  glVertex3f(m_rectWnd.w - 347, m_rectWnd.h - 272, 0);
  OpenGLHelpers::SetTexCoord(392.0/512.0, (511.0-(391.0-27.0))/512.0);
  glVertex3f(m_rectWnd.w - 27, m_rectWnd.h - 272, 0);
  OpenGLHelpers::SetTexCoord((392.0 - 20.0)/512.0, (511.0-144.0)/512.0);
  glVertex3f(m_rectWnd.w - 347, m_rectWnd.h - 272 + 20.0, 0);
  OpenGLHelpers::SetTexCoord((392.0 - 20.0)/512.0, (511.0-(391.0-27.0))/512.0);
  glVertex3f(m_rectWnd.w - 27, m_rectWnd.h - 272 + 20.0, 0);
  glEnd();

  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(120.0/512.0, (511.0-144.0)/512.0);
  glVertex3f(m_rectWnd.w - 347, m_rectWnd.h, 0);
  OpenGLHelpers::SetTexCoord(120.0/512.0, (511.0-(391.0-27.0))/512.0);
  glVertex3f(m_rectWnd.w - 27, m_rectWnd.h, 0);
  OpenGLHelpers::SetTexCoord((120.0 + 20.0)/512.0, (511.0-144.0)/512.0);
  glVertex3f(m_rectWnd.w - 347, m_rectWnd.h - 20.0, 0);
  OpenGLHelpers::SetTexCoord((120.0 + 20.0)/512.0, (511.0-(391.0-27.0))/512.0);
  glVertex3f(m_rectWnd.w - 27, m_rectWnd.h - 20.0, 0);
  glEnd();

  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(392.0/512.0, (511.0-94.0)/512.0);
  glVertex3f(m_rectWnd.w - 397, m_rectWnd.h - 272, 0);
  OpenGLHelpers::SetTexCoord(392.0/512.0, (511.0-144.0)/512.0);
  glVertex3f(m_rectWnd.w - 347, m_rectWnd.h - 272, 0);
  OpenGLHelpers::SetTexCoord(120.0/512.0, (511.0-94.0)/512.0);
  glVertex3f(m_rectWnd.w - 397, m_rectWnd.h, 0);
  OpenGLHelpers::SetTexCoord(120.0/512.0, (511.0-144.0)/512.0);
  glVertex3f(m_rectWnd.w - 347, m_rectWnd.h, 0);
  glEnd();

  // Draw overlay to overwrite Near/Far texts with Clear/Black
  glPushMatrix();
  glTranslated(m_rectWnd.w - 394, m_rectWnd.h - 246, 0);
  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(366.0/512.0, (511.0-404.0)/512.0);
  glVertex3f(0, 0, 0);
  OpenGLHelpers::SetTexCoord(366.0/512.0, (511.0-433.0)/512.0);
  glVertex3f(29, 0, 0);
  OpenGLHelpers::SetTexCoord(149.0/512.0, (511.0-404.0)/512.0);
  glVertex3f(0, 217, 0);
  OpenGLHelpers::SetTexCoord(149.0/512.0, (511.0-433.0)/512.0);
  glVertex3f(29, 217, 0);
  glEnd();
  glPopMatrix();

  // Draw handle
  glPushMatrix();
  glTranslated(m_rectWnd.w - 287, m_rectWnd.h - 137, 0);
  glRotated(90 + BGame::m_dServiceHandleAngle, 0, 0, 1);
  glTranslated(0, 90.5, 0);
  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(423.0/512.0, (511.0-176.0)/512.0);
  glVertex3f(-12, 64, 0);
  OpenGLHelpers::SetTexCoord(423.0/512.0, (511.0-240.0)/512.0);
  glVertex3f(-12, 0, 0);
  OpenGLHelpers::SetTexCoord(447.0/512.0, (511.0-176.0)/512.0);
  glVertex3f(12, 64, 0);
  OpenGLHelpers::SetTexCoord(447.0/512.0, (511.0-240.0)/512.0);
  glVertex3f(12, 0, 0);
  glEnd();
  glPopMatrix();

  glDisable(GL_TEXTURE_2D);
}




//*************************************************************************************************
void CPakoon1View::DrawServiceWndTexts(SDL_Rect &rectWnd) {
  glPushMatrix();
  glTranslated(m_rectWnd.w - 256 - 100, m_rectWnd.h - 14 - 18, 0);
  BGame::GetServiceWnd()->DrawTexts();
  glPopMatrix();
}



//*************************************************************************************************
void CPakoon1View::DrawPanel(double dWidth, 
                             double dHeight, 
                             double dRed, 
                             double dGreen, 
                             double dBlue, 
                             double dAlpha) {
  // Draw an empty panel centered about the current origin

  // Draw background(s)
  glDisable(GL_TEXTURE_2D);

  OpenGLHelpers::SetColorFull(dRed, dGreen, dBlue, dAlpha);
  glBegin(GL_TRIANGLE_STRIP);
  glVertex3f(-dWidth / 2, -dHeight / 2, 0);
  glVertex3f(-dWidth / 2, dHeight / 2, 0);
  glVertex3f(dWidth / 2, -dHeight / 2, 0);
  glVertex3f(dWidth / 2, dHeight / 2, 0);
  glEnd();


  // Draw borders
  // OpenGLHelpers::SetColorFull(1, 0.8, 0.6, 1);
  OpenGLHelpers::SetColorFull(1, 1, 1, 1);
  OpenGLHelpers::SwitchToTexture(0);
  BTextures::Use(BTextures::PANEL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // not mipmapped

  glBegin(GL_QUADS);

  // Corners
  OpenGLHelpers::SetTexCoord(395.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(-dWidth / 2 - 14, dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(-dWidth / 2 - 14, dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(-dWidth / 2 + 14, dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(395.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(-dWidth / 2 + 14, dHeight / 2 + 14, 0);

  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(-dWidth / 2 - 14, -dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(115.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(-dWidth / 2 - 14, -dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(115.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(-dWidth / 2 + 14, -dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(-dWidth / 2 + 14, -dHeight / 2 + 14, 0);

  OpenGLHelpers::SetTexCoord(395.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(dWidth / 2 - 14, dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(dWidth / 2 - 14, dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(dWidth / 2 + 14, dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(395.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(dWidth / 2 + 14, dHeight / 2 + 14, 0);

  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(dWidth / 2 - 14, -dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(115.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(dWidth / 2 - 14, -dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(115.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(dWidth / 2 + 14, -dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(dWidth / 2 + 14, -dHeight / 2 + 14, 0);

  // Left and right border

  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(-dWidth / 2 - 14, dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(-dWidth / 2 - 14, -dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(-dWidth / 2 + 14, -dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(-dWidth / 2 + 14, dHeight / 2 - 14, 0);

  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(dWidth / 2 - 14, dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(dWidth / 2 - 14, -dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(dWidth / 2 + 14, -dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(dWidth / 2 + 14, dHeight / 2 - 14, 0);

  // Top and bottom borders
  OpenGLHelpers::SetTexCoord(395.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(-dWidth / 2 + 14, dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(-dWidth / 2 + 14, dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 254.0) / 512.0);
  glVertex3f(0, dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(395.0 / 512.0, (512.0 - 254.0) / 512.0);
  glVertex3f(0, dHeight / 2 + 14, 0);

  OpenGLHelpers::SetTexCoord(395.0 / 512.0, (512.0 - 254.0) / 512.0);
  glVertex3f(0, dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 254.0) / 512.0);
  glVertex3f(0, dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(dWidth / 2 - 14, dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(395.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(dWidth / 2 - 14, dHeight / 2 + 14, 0);

  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(-dWidth / 2 + 14, -dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(115.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(-dWidth / 2 + 14, -dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(115.0 / 512.0, (512.0 - 254.0) / 512.0);
  glVertex3f(0, -dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 254.0) / 512.0);
  glVertex3f(0, -dHeight / 2 + 14, 0);

  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 254.0) / 512.0);
  glVertex3f(0, -dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(115.0 / 512.0, (512.0 - 254.0) / 512.0);
  glVertex3f(0, -dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(115.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(dWidth / 2 - 14, -dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(dWidth / 2 - 14, -dHeight / 2 + 14, 0);

  glEnd();
}





/////////////////////////////////////////////////////////////////////////////
// CPakoon1View drawing

//*************************************************************************************************
void CPakoon1View::OnDraw() {
  if(m_pDrawFunction) {
    // Call active draw function
    (this->*m_pDrawFunction)();
  }
}

/////////////////////////////////////////////////////////////////////////////
// CPakoon1View message handlers

//*************************************************************************************************
void CPakoon1View::OnChar(unsigned nChar, unsigned nRepCnt, unsigned nFlags) {

  if(BGame::m_bMultiplayOn && BGame::m_bTABChatting) {
    this->OnKeyDownTABChatting(nChar);
    
    return;
  }

  if(m_pDrawFunction != &CPakoon1View::OnDrawGame) {
    return;
  }

  if(m_bIgnoreNextChar) {
    m_bIgnoreNextChar = false;
    return;
  }

  // Check if Scene Editor processes key input
  switch(m_game.GetSceneEditor()->m_phase) {
    case BSceneEditor::ASKING_OBJECT_NAME:
    case BSceneEditor::ASKING_SCENE_DISPLAY_NAME:
    case BSceneEditor::ASKING_SCENE_FILENAME:
      {
        
        m_game.GetSceneEditor()->m_edit.ProcessChar(nChar);
        BUIEdit::TStatus statusEdit;
        string sObjectName = m_game.GetSceneEditor()->m_edit.GetValue(statusEdit);
        if(statusEdit == BUIEdit::READY) {
          m_game.GetSceneEditor()->AdvancePhase();
          if(m_game.GetSceneEditor()->m_phase == BSceneEditor::SELECTING_OBJECT_TYPE) {
            BUI::StartUsingSelectionList(&(m_game.GetSceneEditor()->m_sellistObjectType), &CPakoon1View::OnKeyDownSceneEditor);
          }
        } else if(statusEdit == BUIEdit::CANCELED) {
          m_game.GetSceneEditor()->CancelPhase();
        }
        return;
      }
      break;
  }

  // Check if service window processes key input
  if((BGame::m_bService) && 
     ((toupper(nChar) >= 32) && 
      (toupper(nChar) <= 96) || 
      nChar == SDLK_BACKSPACE ||
      nChar == SDLK_RETURN)) {
    BGame::GetServiceWnd()->AddChar(toupper(nChar));
    return;
  }
}



//*************************************************************************************************
void CPakoon1View::OnKeyDownIntro(unsigned nChar, unsigned nRepCnt, unsigned nFlags) {
  SoundModule::StopIntroSound();
  m_pDrawFunction = &CPakoon1View::OnDrawCurrentMenu;
  m_pKeyDownFunction = &CPakoon1View::OnKeyDownCurrentMenu;
  
}



//*************************************************************************************************
void CPakoon1View::OnKeyDownGame(unsigned nChar, unsigned nRepCnt, unsigned nFlags) {
  if(m_pDrawFunction != &CPakoon1View::OnDrawGame) {
    return;
  }

  if(BGame::m_bTABChatting) {

    return;
  }

  static bool bOpen = false;
  // Check for user input

  if(m_game.GetSimulation()->GetCamera()->m_bNeedsToBeInitialized) {
    m_game.GetSimulation()->GetCamera()->m_locMode = BCamera::FOLLOW;
    m_game.GetSimulation()->GetCamera()->m_dAngleOfView = 75.0;
    m_game.GetSimulation()->GetCamera()->m_bNeedsToBeInitialized = false;
  }

  bool bProcessed = false;
  BVector vToHome;

  // Process non-writing key commands always
  switch(nChar) {
    case SDLK_TAB: 
      if(BGame::m_bMultiplayOn) {
        BGame::m_bTABChatting = true;
      }
      bProcessed = true;
      break;
    case SDLK_LCTRL:
      g_bControl = true;
      break;
    case SDLK_LSHIFT:
      g_bShift = true;
      break;
    case SDLK_ESCAPE:
      bProcessed = true;

      if(m_game.m_bShowQuickHelp) {
        m_game.m_bShowQuickHelp = false;        
        if(BGame::m_bShowGameMenu) {
          BUI::StartUsingSelectionList(&(BGame::m_menuGame.m_listMenu), &CPakoon1View::OnKeyDownGame);
        } else {
          m_nMenuTime += BGame::ContinueSimulation();
        }
      } else {
        BGame::FreezeSimulation();

        BUI::StartUsingSelectionList(&(BGame::m_menuGame.m_listMenu), &CPakoon1View::OnKeyDownGame);
        m_game.m_bShowGameMenu = !m_game.m_bShowGameMenu;
      }
      break;
    case SDLK_F1:
      bProcessed = true;
      if(!m_game.m_bShowQuickHelp) {
        BGame::FreezeSimulation(false);
        // show quick help
        m_game.m_bShowQuickHelp = true;
      }
      break;
    //case SDLK_HOME:
    //  bProcessed = true;
    //  m_game.GetSimulation()->GetCamera()->m_dFollowHeight = -3.0;
    //  vToHome = m_game.GetSimulation()->GetVehicle()->GetHomeLocation() - m_game.GetSimulation()->GetVehicle()->m_vLocation + BVector(35, 0, -20);
    //  m_game.GetSimulation()->GetVehicle()->Move(vToHome);
    //  m_game.GetSimulation()->UpdateCar();
    //  break;
    //case SDLK_DELETE:
    //  bProcessed = true;
    //  m_game.GetSimulation()->GetVehicle()->Move(BVector(0, 0, -1000.0));
    //  m_game.GetSimulation()->UpdateCar();
    //  break;
    case SDLK_F2:
      bProcessed = true;
      m_game.GetSimulation()->GetCamera()->m_locMode = BCamera::FOLLOW;
      m_game.GetSimulation()->GetCamera()->m_dAngleOfView = 80.0;
      m_messages.Remove("camera");
      m_messages.Show(60, "camera", "camera: chase", 1);
      break;
    case SDLK_F3:
      bProcessed = true;
      m_game.GetSimulation()->GetCamera()->m_locMode = BCamera::OVERVIEW;
      m_game.GetSimulation()->GetCamera()->m_bInitLoc = true;
      m_game.GetSimulation()->GetCamera()->m_dAngleOfView = 80.0;
      m_messages.Remove("camera");
      m_messages.Show(60, "camera", "camera: overview", 1);
      break;
    case SDLK_F4:
      bProcessed = true;
      m_game.GetSimulation()->GetCamera()->m_locMode = BCamera::INCAR;
      m_game.GetSimulation()->GetCamera()->m_dAngleOfView = 75.0;
      m_messages.Remove("camera");
      m_messages.Show(60, "camera", "camera: 1st person", 1);
      break;
    case SDLK_F5:
      bProcessed = true;
      m_game.GetSimulation()->GetCamera()->m_locMode = BCamera::FIXED;
      m_game.GetSimulation()->GetCamera()->m_vFixLocation = m_game.GetSimulation()->GetCamera()->m_vLocation;
      m_game.GetSimulation()->GetCamera()->m_dAngleOfView = 70.0;
      m_messages.Remove("camera");
      m_messages.Show(60, "camera", "camera: stationary", 1);
      break;
    case SDLK_F8:
      m_game.Command()->Run("toggle navsat");
      break;
    //case SDLK_F9:
    //  m_game.Command()->Run("toggle service");
    //  break;
    case SDLK_F11:
      bProcessed = true;
      m_game.GetSimulation()->m_bSteeringAidOn = !m_game.GetSimulation()->m_bSteeringAidOn;
      if(m_game.GetSimulation()->m_bSteeringAidOn) {
        m_messages.Remove("steeringaid");
        m_messages.Show(60, "steeringaid", "Steering aid on", 1);
      } else {
        m_messages.Remove("steeringaid");
        m_messages.Show(60, "steeringaid", "Steering aid off", 1);
      }
      break;
    //case SDLK_ADD:
    //  bProcessed = true;
    //  m_game.GetSimulation()->m_dAccelerationFactor *= 2.0;
    //  break;
    //case SDLK_SUBTRACT:
    //  bProcessed = true;
    //  m_game.GetSimulation()->m_dAccelerationFactor /= 2.0;
    //  break;
    case SDLK_PAGEUP:
      bProcessed = true;
      m_game.GetSimulation()->GetCamera()->m_dFollowHeight -= 1.0;
      break;
    case SDLK_PAGEDOWN:
      bProcessed = true;
      m_game.GetSimulation()->GetCamera()->m_dFollowHeight += 1.0;
      if(m_game.GetSimulation()->GetCamera()->m_dFollowHeight > 0.0) {
        m_game.GetSimulation()->GetCamera()->m_dFollowHeight = 0.0;
      }
      break;
  }

  if(!bProcessed) {
    // Check if service window processes key input
    if(!BGame::m_bService) {
      if(nChar == ControllerModule::m_keymap.m_unAccelerate) {
        bProcessed = true;
        m_game.GetSimulation()->GetVehicle()->m_bAccelerating = true;
        m_game.GetSimulation()->GetVehicle()->m_dAccelerationFactor = 1.0 * m_game.GetSimulation()->GetVehicle()->m_dFuelFactor;
      } else if(nChar == ControllerModule::m_keymap.m_unReverse) {
        bProcessed = true;
        m_game.GetSimulation()->GetVehicle()->m_bReversing = true;
        m_game.GetSimulation()->GetVehicle()->m_dReversingFactor = 1.0 * m_game.GetSimulation()->GetVehicle()->m_dFuelFactor;
      } else if(nChar == ControllerModule::m_keymap.m_unPropeller) {
        bProcessed = true;
        m_game.GetSimulation()->GetVehicle()->m_bPropeller = true;
      } else if(nChar == ControllerModule::m_keymap.m_unPropellerReverse) {
        bProcessed = true;
        m_game.GetSimulation()->GetVehicle()->m_bPropReverse = true;
      } else if(nChar == ControllerModule::m_keymap.m_unBreak) {
        bProcessed = true;
        m_game.GetSimulation()->GetVehicle()->m_bBreaking = true;
      } else if(nChar == ControllerModule::m_keymap.m_unLeft) {
        bProcessed = true;
        m_game.GetSimulation()->GetVehicle()->m_bTurningLeft = true;
      } else if(nChar == ControllerModule::m_keymap.m_unRight) {
        bProcessed = true;
        m_game.GetSimulation()->GetVehicle()->m_bTurningRight = true;
      } else if(nChar == ControllerModule::m_keymap.m_unCamera) {
        bProcessed = true;
        m_game.GetSimulation()->SwitchCameraMode();
      } else if(nChar == ControllerModule::m_keymap.m_unLift) {
        bProcessed = true;

        if(fabs(BGame::m_dLiftStarted - BGame::m_dRaceTime) > g_dPhysicsStepsInSecond) {
          // Correct car orientation
          FixCarToBasicOrientation(0.5);
          BGame::m_dLiftStarted = BGame::m_dRaceTime;
        }
      }

      switch(nChar) {
        case '0':
          bProcessed = true;
          m_game.FreezeSimulation();
          m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_SLOW_MACHINE);
          BGame::UpdateSettings();
          BGame::GetSimulation()->GetTerrain()->MakeTerrainValid(BGame::GetSimulation()->GetVehicle()->m_vLocation,
                                                                 BGame::GetSimulation()->GetCamera()->m_vLocation,
                                                                 BGame::GetSimulation()->GetCamera()->m_orientation.m_vForward,
                                                                 true, 
                                                                 false, 
                                                                 false);
          BGame::GetSimulation()->EnsureVehicleIsOverGround();
          BGame::m_dLiftStarted = BGame::m_dRaceTime;
          m_nMenuTime += BGame::ContinueSimulation();
          break;
        case '1':
          bProcessed = true;
          if(BGame::m_bNavSat && g_bControl) {
            BGame::Command()->Run("set navsat resolution 10400");
            BGame::m_dNavSatHandleAngle = 20.0;
          } else {
            m_game.FreezeSimulation();
            m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_MINIMUM);
            BGame::UpdateSettings();
            BGame::GetSimulation()->GetTerrain()->MakeTerrainValid(BGame::GetSimulation()->GetVehicle()->m_vLocation,
                                                                   BGame::GetSimulation()->GetCamera()->m_vLocation,
                                                                   BGame::GetSimulation()->GetCamera()->m_orientation.m_vForward,
                                                                   true, 
                                                                   false, 
                                                                   false);
            BGame::GetSimulation()->EnsureVehicleIsOverGround();
            BGame::m_dLiftStarted = BGame::m_dRaceTime;
            m_nMenuTime += BGame::ContinueSimulation();
          }
          break;
        case '2':
          bProcessed = true;
          if(BGame::m_bNavSat && g_bControl) {
            BGame::Command()->Run("set navsat resolution 5200");
            BGame::m_dNavSatHandleAngle = 10.0;
          } else {
            m_game.FreezeSimulation();
            m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_LOW);
            BGame::UpdateSettings();
            BGame::GetSimulation()->GetTerrain()->MakeTerrainValid(BGame::GetSimulation()->GetVehicle()->m_vLocation,
                                                                   BGame::GetSimulation()->GetCamera()->m_vLocation,
                                                                   BGame::GetSimulation()->GetCamera()->m_orientation.m_vForward,
                                                                   true, 
                                                                   false, 
                                                                   false);
            BGame::GetSimulation()->EnsureVehicleIsOverGround();
            BGame::m_dLiftStarted = BGame::m_dRaceTime;
            m_nMenuTime += BGame::ContinueSimulation();
          }
          break;
        case '3':
          bProcessed = true;
          if(BGame::m_bNavSat && g_bControl) {
            BGame::Command()->Run("set navsat resolution 2600");
            BGame::m_dNavSatHandleAngle = 0.0;
          } else {
            m_game.FreezeSimulation();
            m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_MEDIUM);
            BGame::UpdateSettings();
            BGame::GetSimulation()->GetTerrain()->MakeTerrainValid(BGame::GetSimulation()->GetVehicle()->m_vLocation,
                                                                   BGame::GetSimulation()->GetCamera()->m_vLocation,
                                                                   BGame::GetSimulation()->GetCamera()->m_orientation.m_vForward,
                                                                   true, 
                                                                   false, 
                                                                   false);
            BGame::GetSimulation()->EnsureVehicleIsOverGround();
            BGame::m_dLiftStarted = BGame::m_dRaceTime;
            m_nMenuTime += BGame::ContinueSimulation();
          }
          break;
        case '4':
          bProcessed = true;
          if(BGame::m_bNavSat && g_bControl) {
            BGame::Command()->Run("set navsat resolution 1300");
            BGame::m_dNavSatHandleAngle = -10.0;
          } else {
            m_game.FreezeSimulation();
            m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_HIGH);
            BGame::UpdateSettings();
            BGame::GetSimulation()->GetTerrain()->MakeTerrainValid(BGame::GetSimulation()->GetVehicle()->m_vLocation,
                                                                   BGame::GetSimulation()->GetCamera()->m_vLocation,
                                                                   BGame::GetSimulation()->GetCamera()->m_orientation.m_vForward,
                                                                   true, 
                                                                   false, 
                                                                   false);
            BGame::GetSimulation()->EnsureVehicleIsOverGround();
            BGame::m_dLiftStarted = BGame::m_dRaceTime;
            m_nMenuTime += BGame::ContinueSimulation();
          }
          break;
        case '5':
          bProcessed = true;
          if(BGame::m_bNavSat && g_bControl) {
            BGame::Command()->Run("set navsat resolution 650");
            BGame::m_dNavSatHandleAngle = -20.0;
          } else {
            m_game.FreezeSimulation();
            m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_MAXIMUM);
            BGame::UpdateSettings();
            BGame::GetSimulation()->GetTerrain()->MakeTerrainValid(BGame::GetSimulation()->GetVehicle()->m_vLocation,
                                                                   BGame::GetSimulation()->GetCamera()->m_vLocation,
                                                                   BGame::GetSimulation()->GetCamera()->m_orientation.m_vForward,
                                                                   true, 
                                                                   false, 
                                                                   false);
            BGame::GetSimulation()->EnsureVehicleIsOverGround();
            BGame::m_dLiftStarted = BGame::m_dRaceTime;
            m_nMenuTime += BGame::ContinueSimulation();
          }
          break;
        /* CUT OUT FROM THE PUBLIC RELEASE
        case 'w':
        case 'W':
          {
            bProcessed = true;
            // Check whether this is the second click
            static clock_t clockPrev = 0;
            clock_t clockNow = clock();
            if((clockNow - clockPrev) < (CLOCKS_PER_SEC / 3)) {
              m_game.GetSimulation()->GetVehicle()->m_bWireframe = !m_game.GetSimulation()->GetVehicle()->m_bWireframe;
              // m_bWireframe = !m_bWireframe;
              // m_bCreateDLs = true;
            }
            clockPrev = clockNow;
          }
          break;
        case 'r':
        case 'R':
          bProcessed = true;
          m_game.m_bRecordSlalom = !m_game.m_bRecordSlalom;
          BMessages::Remove("airtime");
          BMessages::Remove("slalomrecord");
          break;
        case 'p':
        case 'P':
          bProcessed = true;
          // Add new slalom pole for the current scene
          if(BGame::m_bRecordSlalom) {
            BScene   *pScene   = BGame::GetSimulation()->GetScene();
            BVehicle *pVehicle = BGame::GetSimulation()->GetVehicle();
            if(pScene->m_slalom.m_nCurrentPole < 199) {
              double dOffset = pVehicle->m_dVisualWidth * 2.0;
              if(BGame::m_bPassFromRightSlalom) {
                dOffset *= -1.0;              
              }

              double dTmp;
              BVector vOffset = BGame::GetSimulation()->GetTerrain()->m_vOffset;
              BVector vLoc = pVehicle->m_vLocation + BVector(dOffset, 0.0, 0.0);
              vLoc.m_dZ = -HeightMap::CalcHeightAt(vOffset.m_dX + vLoc.m_dX, 
                                                   vOffset.m_dY + vLoc.m_dY, 
                                                   dTmp, 
                                                   pScene->m_terrainStyle,
                                                   8,
                                                   vOffset.m_dY);
              pScene->m_slalom.m_slalomPole[pScene->m_slalom.m_nCurrentPole].m_vLocation = vLoc;
              pScene->m_slalom.m_slalomPole[pScene->m_slalom.m_nCurrentPole].m_bPassFromRight = BGame::m_bPassFromRightSlalom;
              ++(pScene->m_slalom.m_nCurrentPole);
              BGame::m_bPassFromRightSlalom = !BGame::m_bPassFromRightSlalom;
              BMessages::Show(40, "poleadded", "POLE ADDED", 1, false, 1, 0.5, 0.5);
            }
          }
          break;
        */
        case 'e':
        case 'E':
          bProcessed = true;
          ++m_game.m_nShowEffects;
          if(m_game.m_nShowEffects > 2) {
            m_game.m_nShowEffects = 0;
          }
          m_game.m_nDustAndClouds = m_game.m_nShowEffects;
          if(m_game.m_nShowEffects == 0) {
            m_messages.Remove("effects");
            m_messages.Show(60, "effects", "effects off", 1);
          } else if(m_game.m_nShowEffects == 1) {
            m_messages.Remove("effects");
            m_messages.Show(60, "effects", "effects: iceflare", 1);
          } else if(m_game.m_nShowEffects == 2) {
            m_messages.Remove("effects");
            m_messages.Show(60, "effects", "effects: iceflare & tube", 1);
          }
          BGame::UpdateSettings();
          break;
        /*
        case 'a':
        case 'A':
          {
            bProcessed = true;

            // Check whether this is the second click
            static clock_t clockPrev = 0;
            clock_t clockNow = clock();
            if((clockNow - clockPrev) < (CLOCKS_PER_SEC / 3)) {
              BGame::FreezeSimulation(false);
              // Start Analyzer
              BGame::m_bAnalyzerMode = true;
              BGame::m_clockAnalyzerStarted = clockNow;

              {
                BGame::MyAfxMessageBox("--------------------------");
                BGame::MyAfxMessageBox("ANALYZER STARTED!");
                BGame::MyAfxMessageBox("--------------------------");
                string sLogInfo;
                sLogInfo.Format("Vehicle: %s", BGame::GetSimulation()->GetVehicle()->m_sName);
                BGame::MyAfxMessageBox(sLogInfo);
                sLogInfo.Format("Scene: %s", BGame::GetSimulation()->GetScene()->m_sName);
                BGame::MyAfxMessageBox(sLogInfo);
                sLogInfo.Format("Screen: %d*%d*%d @ %dHz", BGame::m_nDispWidth, BGame::m_nDispHeight, BGame::m_nDispBits, BGame::m_nDispHz);
                BGame::MyAfxMessageBox(sLogInfo);
                sLogInfo.Format("Terrain: %d", BGame::m_nTerrainResolution);
                BGame::MyAfxMessageBox(sLogInfo);
                sLogInfo.Format("Effects: dust=%d water=%d", BGame::m_nDustAndClouds, BGame::m_nWaterSurface);
                BGame::MyAfxMessageBox(sLogInfo);
                sLogInfo.Format("FPS: AVE=%.2lf, Last10=%.1lf %.1lf %.1lf %.1lf %.1lf %.1lf %.1lf %.1lf %.1lf %.1lf ", 
                                g_dRate,
                                g_d10LastFPS[0],
                                g_d10LastFPS[1],
                                g_d10LastFPS[2],
                                g_d10LastFPS[3],
                                g_d10LastFPS[4],
                                g_d10LastFPS[5],
                                g_d10LastFPS[6],
                                g_d10LastFPS[7],
                                g_d10LastFPS[8],
                                g_d10LastFPS[9]);
                BGame::MyAfxMessageBox(sLogInfo);
              }
            }
            clockPrev = clockNow;
          }
          break;
        */
        case 'd':
        case 'D':
          {
            bProcessed = true;

            // Check whether this is the second click
            static clock_t clockPrev = 0;
            clock_t clockNow = clock();
            if((clockNow - clockPrev) < (CLOCKS_PER_SEC / 3)) {
            BGame::FreezeSimulation(false);
              m_game.GetSceneEditor()->Activate();
              m_messages.Remove("sceneeditor");
              m_messages.Show(50, "sceneeditor", "scene editor mode", 1);
              m_pKeyDownFunction = &CPakoon1View::OnKeyDownSceneEditor;
            }
            clockPrev = clockNow;
          }
          break;
        case 'i':
        case 'I':
          bProcessed = true;
          if(BGame::m_cOnScreenInfo & BGame::FPS) {
            BGame::m_cOnScreenInfo -= BGame::FPS;
          } else {
            BGame::m_cOnScreenInfo |= BGame::FPS;
          }
          break;
        case 'b':
        case 'B':
          bProcessed = true;
          m_game.m_bSlowMotion = !m_game.m_bSlowMotion;
          if(m_game.m_bSlowMotion) {
            m_messages.Remove("slow motion");
            m_messages.Show(40, "slow motion", "slow motion", 1);
          } else {
            m_messages.Remove("slow motion");
            m_messages.Show(40, "slow motion", "slow motion off", 1);
          }
          break;
        case 'f':
        case 'F':
          bProcessed = true;
          {
            if(!m_game.m_bFrozen) {
              m_messages.Show(40, "frozen", "Frozen (f)", 2);
              BGame::FreezeSimulation();
            } else {
              m_messages.Remove("frozen");
              m_nMenuTime += BGame::ContinueSimulation();
            }
          }
          break;
      }

      switch(nChar) {
        case SDLK_SPACE:
          bProcessed = true;
          m_game.GetSimulation()->GetVehicle()->m_bHandBreaking = true;
          break;
      }
    }
  }

  if(!bProcessed) {
    m_game.m_bShowHint = true;
    m_game.m_clockHintStart = clock();
  }
}


//*************************************************************************************************
void CPakoon1View::FixCarToBasicOrientation(double dSpeedFactor) {
  BVehicle *pVehicle = BGame::GetSimulation()->GetVehicle();
  BVector vDir = pVehicle->m_pBodyPoint[0].m_vector * dSpeedFactor;
  BVector vLoc = pVehicle->m_vLocation;
  for(int i = 0; i < pVehicle->m_nBodyPoints; ++i) {
    pVehicle->m_pBodyPoint[i].m_vLocation = vLoc + pVehicle->m_pOrigBodyPoint[i].m_vLocation;
    pVehicle->m_pBodyPoint[i].m_vector = vDir;
  }
  BGame::GetSimulation()->EnsureVehicleIsOverGround();
}




//*************************************************************************************************
void CPakoon1View::OnKeyDownCurrentMenu(unsigned nChar, unsigned nRepCnt, unsigned nFlags) {
  switch(nChar) {
    case SDLK_TAB: 
      if(BGame::m_bMultiplayOn) {
        BGame::m_bTABChatting = true;
      }
  }
}








//*************************************************************************************************
void CPakoon1View::OnKeyDownSceneEditor(unsigned nChar, unsigned nRepCnt, unsigned nFlags) {
  BCamera *pCamera = m_game.GetSimulation()->GetCamera();
  BScene  *pScene  = m_game.GetSimulation()->GetScene();

  switch(m_game.GetSceneEditor()->m_phase) {
    case BSceneEditor::ASKING_OBJECT_NAME:
    case BSceneEditor::ASKING_SCENE_DISPLAY_NAME:
    case BSceneEditor::ASKING_SCENE_FILENAME:
      return;
      break;
  }

  double dModeScaler = 20.0;
  if(g_bShift) {
    dModeScaler = 1.0;
  }
  if((m_game.GetSceneEditor()->m_phase == BSceneEditor::BASIC) &&
      g_bControl) {
    dModeScaler = 150.0;
  }

  switch(nChar) {
    case SDLK_LCTRL:
      g_bControl = true;
      break;
    case SDLK_LSHIFT:
      g_bShift = true;
      break;
    case SDLK_RETURN:
      // If moving object, return to camera mode
      if(BGame::GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) {
        BGame::GetSceneEditor()->AdvancePhase();
      }
      break;
    case 'e':
    case 'E':
      // Return to game mode (NOTE: SHOULD WE WARN ABOUT AN UNSAVED SCENE?)      
      m_game.GetSceneEditor()->Deactivate();
      m_pKeyDownFunction = &CPakoon1View::OnKeyDownGame;
      m_messages.Remove("sceneeditor");
      m_messages.Show(50, "sceneeditor", "game mode", 1);
      m_nMenuTime += BGame::ContinueSimulation();
      break;
    case SDLK_RIGHT: 
      if((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
         (!g_bControl)) {
        // Move object to right
        BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
        if(pObject) {
          pObject->m_vLocation += pCamera->m_orientation.m_vRight * 0.1 * dModeScaler;
          pObject->m_vCenter += pCamera->m_orientation.m_vRight * 0.1 * dModeScaler;
          pObject->RecreateShadow();
        }
      } else {
        if(!g_bShift) {
          pCamera->m_orientation.m_vForward = pCamera->m_orientation.m_vForward + pCamera->m_orientation.m_vRight * 0.02 * dModeScaler;
        } else {
          pCamera->m_orientation.m_vForward = pCamera->m_orientation.m_vForward + pCamera->m_orientation.m_vRight * 0.05 * dModeScaler;
        }
        pCamera->m_orientation.m_vForward.ToUnitLength();
        pCamera->m_orientation.m_vRight = pCamera->m_orientation.m_vUp.CrossProduct(pCamera->m_orientation.m_vForward);
      }
      break;
    case SDLK_LEFT:
      if((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
         (!g_bControl)) {
        // Move object to west
        BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
        if(pObject) {
          pObject->m_vLocation -= pCamera->m_orientation.m_vRight * 0.1 * dModeScaler;
          pObject->m_vCenter -= pCamera->m_orientation.m_vRight * 0.1 * dModeScaler;
          pObject->RecreateShadow();
        }
      } else {
        if(!g_bShift) {
          pCamera->m_orientation.m_vForward = pCamera->m_orientation.m_vForward + pCamera->m_orientation.m_vRight * -0.02 * dModeScaler;
        } else {
          pCamera->m_orientation.m_vForward = pCamera->m_orientation.m_vForward + pCamera->m_orientation.m_vRight * -0.05 * dModeScaler;
        }
        pCamera->m_orientation.m_vForward.ToUnitLength();
        pCamera->m_orientation.m_vRight = pCamera->m_orientation.m_vUp.CrossProduct(pCamera->m_orientation.m_vForward);
      }
      break;
    case SDLK_DOWN: 
      if((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
         (!g_bControl)) {
        // Move object to south
        BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
        if(pObject) {
          pObject->m_vLocation -= pCamera->m_orientation.m_vForward * 0.1 * dModeScaler;
          pObject->m_vCenter -= pCamera->m_orientation.m_vForward * 0.1 * dModeScaler;
          pObject->RecreateShadow();
        }
      } else {
        pCamera->m_vLocation = pCamera->m_vLocation + pCamera->m_orientation.m_vForward * -dModeScaler;
      }
      break;
    case SDLK_UP: 
      if((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
         (!g_bControl)) {
        // Move object to north
        BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
        if(pObject) {
          pObject->m_vLocation += pCamera->m_orientation.m_vForward * 0.1 * dModeScaler;
          pObject->m_vCenter += pCamera->m_orientation.m_vForward * 0.1 * dModeScaler;
          pObject->RecreateShadow();
        }
      } else {
        pCamera->m_vLocation = pCamera->m_vLocation + pCamera->m_orientation.m_vForward * dModeScaler;
      }
      break;
    case SDLK_PAGEDOWN:
      if((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
         (!g_bControl)) {
        // Move object down
        BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
        if(pObject) {
          pObject->m_vLocation.m_dZ += 0.1 * dModeScaler;
          pObject->m_vCenter.m_dZ += 0.1 * dModeScaler;
          pObject->RecreateShadow();
        }
      } else {
        pCamera->m_vLocation = pCamera->m_vLocation + pCamera->m_orientation.m_vUp * -dModeScaler;
      }
      break;
    case SDLK_PAGEUP:
      if((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
         (!g_bControl)) {
        // Move object up
        BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
        if(pObject) {
          pObject->m_vLocation.m_dZ -= 0.1 * dModeScaler;
          pObject->m_vCenter.m_dZ -= 0.1 * dModeScaler;
          pObject->RecreateShadow();
        }
      } else {
        pCamera->m_vLocation = pCamera->m_vLocation + pCamera->m_orientation.m_vUp * dModeScaler;
      }
      break;
    case SDLK_HOME:
      if((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
         (!g_bControl)) {
        // Rotate clockwise
        BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
        if(pObject) {
          pObject->m_dZRotation += 1 * dModeScaler;
          pObject->RecreateShadow();
        }
      }
      break;
    case SDLK_END:
      if((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
         (!g_bControl)) {
        // Rotate counter clockwise
        BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
        if(pObject) {
          pObject->m_dZRotation -= 1 * dModeScaler;
          pObject->RecreateShadow();
        }
      } 
      break;
    case 'o':
    case 'O':
      if((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
         (!g_bControl)) {
        // Scale object's active radius
        BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
        if(pObject) {
          double dScaler = 1.01;
          if(!g_bShift) {
            dScaler = 2.0;
          }
          pObject->m_dActiveRadius *= dScaler;
        }
      } 
      break;
    case 'i':
    case 'I':
      if((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
         (!g_bControl)) {
        // Shrink object's active radius
        BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
        if(pObject) {
          double dScaler = (1.0 / 1.01);
          if(!g_bShift) {
            dScaler = 0.5;
          }
          pObject->m_dActiveRadius *= dScaler;
        }
      } 
      break;
    case 'l':
    case 'L':
      if((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
         (!g_bControl)) {
        // Scale object bigger
        BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
        if(pObject) {
          double dScaler = 1.01;
          if(!g_bShift) {
            dScaler = 2.0;
          }
          pObject->m_dScale2 *= dScaler;
          pObject->m_dRadius *= dScaler;
          pObject->RecreateShadow();
        }
      } 
      break;
    case 'k':
    case 'K':
      if((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
         (!g_bControl)) {
        // Scale object bigger
        BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
        if(pObject) {
          double dScaler = (1.0 / 1.01);
          if(!g_bShift) {
            dScaler = 0.5;
          }
          pObject->m_dScale2 *= dScaler;
          pObject->m_dRadius *= dScaler;
          pObject->RecreateShadow();
        }
      } 
      break;
    case 'm':
    case 'M':
      BUI::StartUsingSelectionList(&(m_game.GetSimulation()->GetScene()->m_sellistSceneObjects), 
                                   &CPakoon1View::OnKeyDownSceneEditor);
      m_game.GetSimulation()->GetScene()->m_sellistSceneObjects.SelectItem(BGame::GetSceneEditor()->m_sActiveObject);
      m_game.GetSceneEditor()->m_phase = BSceneEditor::SELECTING_SCENE_OBJECT;
      break;
    case 'a':
    case 'A':
      m_game.GetSceneEditor()->m_edit.Setup("Object name:", "", 32);
      m_game.GetSceneEditor()->m_phase = BSceneEditor::ASKING_OBJECT_NAME;
      m_bIgnoreNextChar = true;
      break;
    case 'd':
    case 'D':
      BUI::StartUsingSelectionList(&(m_game.GetSimulation()->GetScene()->m_sellistSceneObjects), 
                                   &CPakoon1View::OnKeyDownSceneEditor);
      m_game.GetSimulation()->GetScene()->m_sellistSceneObjects.SelectItem(BGame::GetSceneEditor()->m_sActiveObject);
      m_game.GetSceneEditor()->m_phase = BSceneEditor::SELECTING_SCENE_OBJECT_TO_DELETE;
      break;
    case 's':
    case 'S':
      m_game.GetSceneEditor()->m_edit.Setup("Scene display name:", pScene->m_sName, 32);
      m_game.GetSceneEditor()->m_phase = BSceneEditor::ASKING_SCENE_DISPLAY_NAME;
      m_bIgnoreNextChar = true;
      break;
    case SDLK_F1:
      // show quick help
      m_game.m_bShowQuickHelp = true;
      break;
    case SDLK_ESCAPE:
      if(m_game.m_bShowQuickHelp) {
        m_game.m_bShowQuickHelp = false;        
      }
      break;
    case '0':
      m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_SLOW_MACHINE);
      BGame::UpdateSettings();
      break;
    case '1':
      if(BGame::m_bNavSat && g_bControl) {
        BGame::Command()->Run("set navsat resolution 10400");
        BGame::m_dNavSatHandleAngle = 20.0;
      } else {
        m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_MINIMUM);
        BGame::UpdateSettings();
      }
      break;
    case '2':
      if(BGame::m_bNavSat && g_bControl) {
        BGame::Command()->Run("set navsat resolution 5200");
        BGame::m_dNavSatHandleAngle = 10.0;
      } else {
        m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_LOW);
        BGame::UpdateSettings();
      }
      break;
    case '3':
      if(BGame::m_bNavSat && g_bControl) {
        BGame::Command()->Run("set navsat resolution 2600");
        BGame::m_dNavSatHandleAngle = 0.0;
      } else {
        m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_MEDIUM);
        BGame::UpdateSettings();
      }
      break;
    case '4':
      if(BGame::m_bNavSat && g_bControl) {
        BGame::Command()->Run("set navsat resolution 1300");
        BGame::m_dNavSatHandleAngle = -10.0;
      } else {
        m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_HIGH);
        BGame::UpdateSettings();
      }
      break;
    case '5':
      if(BGame::m_bNavSat && g_bControl) {
        BGame::Command()->Run("set navsat resolution 650");
        BGame::m_dNavSatHandleAngle = -20.0;
      } else {
        m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_MAXIMUM);
        BGame::UpdateSettings();
      }
      break;
    case SDLK_F8:
      m_game.Command()->Run("toggle navsat");
      break;
    //case SDLK_F9:
    //  m_game.Command()->Run("toggle service");
    //  break;
  }
}




//*************************************************************************************************
void CPakoon1View::OnKeyDownTABChatting(unsigned nChar) {
  unsigned char c = nChar;
  if(!((toupper(c) >= 32) && 
       (toupper(c) <= 96) || 
       (c == '.') ||
       (c == '/') ||
       (c == SDLK_BACKSPACE) ||
       (c == SDLK_RETURN) || 
       (c == SDLK_ESCAPE))) {
    // Bad character, no go
    return;
  }

  if(c == SDLK_BACKSPACE) {
    // Eat one character
    if(BGame::m_sChatMsg.length() > 0) {
      BGame::m_sChatMsg.erase(BGame::m_sChatMsg.length() - 1, 1);
    }
    return;
  }

  if(c == SDLK_RETURN) {
    // Send Chat message
    string sChat;
    sChat.assign(BGame::m_remotePlayer[BGame::GetMyPlace()].m_sName);
    sChat.append("` ");
    sChat.append(BGame::m_sChatMsg);
    BGame::GetMultiplay()->SendBroadcastMsg(BMultiPlay::CHAT_MESSAGE, sChat);
    BGame::ShowMultiplayMessage(sChat, true);
    BGame::m_bTABChatting = false;
    BGame::m_sChatMsg.clear();
    return;
  }

  if(c == SDLK_ESCAPE) {
    // Just end chat mode
    BGame::m_bTABChatting = false;
    return;
  }

  // Add writable character
  if(BGame::m_sChatMsg.length() < 128) {
    BGame::m_sChatMsg += c;
  }
}





//*************************************************************************************************
// This is a general purpose keyboard handler for active selection list
void CPakoon1View::OnKeyDownSelectionList(unsigned nChar, unsigned nRepCnt, unsigned nFlags) {
  BUISelectionList *pList = BUI::GetActiveSelectionList();
  if(!pList) {
    m_pKeyDownFunction = BUI::StopUsingSelectionList();
  }

  if(BGame::m_bTABChatting) {
    return;
  }

  bool bAdvancePhase = false;

  switch(nChar) {
    case SDLK_TAB: 
      if(BGame::m_bMultiplayOn) {
        BGame::m_bTABChatting = true;
      }
      break;
    case SDLK_DOWN: 
      pList->AdvanceSelection(1);
      SoundModule::PlayMenuBrowseSound();
      break;
    case SDLK_UP: 
      pList->AdvanceSelection(-1);
      SoundModule::PlayMenuBrowseSound();
      break;
    case SDLK_HOME:
      pList->AdvanceSelection(-1000); // to get to the beginning
      SoundModule::PlayMenuBrowseSound();
      break;
    case SDLK_END:
      pList->AdvanceSelection(1000); // to get to the end
      SoundModule::PlayMenuBrowseSound();
      break;
    case SDLK_RETURN:
      m_pKeyDownFunction = BUI::StopUsingSelectionList();
      bAdvancePhase = true;

      if(BGame::m_bMenuMode) {
        ReturnPressedOnCurrentMenu();
      } else if(BGame::m_bShowGameMenu) {
        ReturnPressedOnGameMenu();
      }
      SoundModule::PlayMenuScrollSound();

      break;
    case SDLK_ESCAPE:
      pList->Cancel();
      m_pKeyDownFunction = BUI::StopUsingSelectionList();      
      if(BGame::m_bSceneEditorMode) {
        m_game.GetSceneEditor()->CancelPhase();
      }
      bAdvancePhase = false;

      if(BGame::m_bMenuMode) {
        CancelPressedOnCurrentMenu();
      } else if(BGame::m_bShowGameMenu) {
        CancelPressedOnGameMenu();
      }
      SoundModule::PlayMenuBackSound();

      break;
  }

  // If multiplaying, report menu selection to other players
  if(BGame::m_bMultiplayOn && pList->m_nSelected >= 0) {
    BroadcastMenuBrowse();
  }

  if(bAdvancePhase && BGame::m_bSceneEditorMode) {
    BGame::GetSceneEditor()->AdvancePhase();
  }
}


//*************************************************************************************************
// This is a general purpose keyboard handler for active slider
void CPakoon1View::OnKeyDownSlider(unsigned nChar, unsigned nRepCnt, unsigned nFlags) {
  int *pnSliderValue = BUI::m_pnSliderValue;
  if(!pnSliderValue) {
    m_pKeyDownFunction = BUI::StopUsingSlider();
  }

  switch(nChar) {
    case SDLK_DOWN: 
      *pnSliderValue -= 10;
      break;
    case SDLK_UP: 
      *pnSliderValue += 10;
      break;
    case SDLK_LEFT: 
      *pnSliderValue -= 1;
      break;
    case SDLK_RIGHT: 
      *pnSliderValue += 1;
      break;
    case SDLK_HOME:
      *pnSliderValue = 0;
      break;
    case SDLK_END:
      *pnSliderValue = 100;
      break;
    case SDLK_RETURN:
      m_pKeyDownFunction = BUI::StopUsingSlider();

      if(BGame::m_bMenuMode) {
        ReturnPressedOnCurrentMenu();
      }

      break;
    case SDLK_ESCAPE:
      *pnSliderValue = BUI::m_nPrevSliderValue;
      m_pKeyDownFunction = BUI::StopUsingSlider();      

      if(BGame::m_bMenuMode) {
        CancelPressedOnCurrentMenu();
      }

      break;
  }

  if(*pnSliderValue < 0) {
    *pnSliderValue = 0;
  }
  if(*pnSliderValue > 100) {
    *pnSliderValue = 100;
  }
}


//*************************************************************************************************
// This is a general purpose keyboard handler for active Editbox
void CPakoon1View::OnKeyDownEditbox(unsigned nChar, unsigned nRepCnt, unsigned nFlags) {

  // Find active edit box

  if(BGame::m_bMenuMode) {
    if(BGame::m_pMenuCurrent) {
      int nSel = BGame::m_pMenuCurrent->m_listMenu.m_nSelected;
      BGame::m_pMenuCurrent->m_items[nSel].m_ebAssocEditBox.ProcessChar(nChar);
      if((BGame::m_pMenuCurrent->m_items[nSel].m_ebAssocEditBox.status == BUIEdit::READY) || 
         (BGame::m_pMenuCurrent->m_items[nSel].m_ebAssocEditBox.status == BUIEdit::CANCELED)) {
        if(BGame::m_pMenuCurrent->m_items[nSel].m_ebAssocEditBox.status == BUIEdit::CANCELED) {
          // Restore original value
          BGame::m_pMenuCurrent->m_items[nSel].m_ebAssocEditBox.m_sValue = BUI::m_sPrevSValue;
        }        

        // Close menu item
        m_pKeyDownFunction = BUI::StopUsingEditbox();
        ReturnPressedOnCurrentMenu();
      }
    }
  }
}




//*************************************************************************************************
void CPakoon1View::OnKeyDown(unsigned nChar, unsigned nRepCnt, unsigned nFlags) {
  if(m_pKeyDownFunction) {
    // Call active draw function
    (this->*m_pKeyDownFunction)(nChar, nRepCnt, nFlags);
  }

}

//*************************************************************************************************
void CPakoon1View::OnKeyUp(unsigned nChar, unsigned nRepCnt, unsigned nFlags) {

  // don't even ask about these...
  if(nChar == SDLK_LCTRL) {
    g_bControl = false;
  }
  if(nChar == SDLK_LSHIFT) {
    g_bShift = false;
  }

  if(nChar == ControllerModule::m_keymap.m_unAccelerate) {
    m_game.GetSimulation()->GetVehicle()->m_bAccelerating = false;
    m_game.GetSimulation()->GetVehicle()->m_dAccelerationFactor = 0.0;
    if(m_game.GetSimulation()->GetVehicle()->m_rotor.m_nHeliMode > 399) {
      m_game.GetSimulation()->GetVehicle()->m_rotor.m_bHeliForwarding = false;
    }
  } else if(nChar == ControllerModule::m_keymap.m_unReverse) {
    m_game.GetSimulation()->GetVehicle()->m_bReversing = false;
    if(m_game.GetSimulation()->GetVehicle()->m_rotor.m_nHeliMode > 399) {
      m_game.GetSimulation()->GetVehicle()->m_rotor.m_bHeliBacking = false;
    }
  } else if(nChar == ControllerModule::m_keymap.m_unPropeller) {
    m_game.GetSimulation()->GetVehicle()->m_bPropeller = false;
    m_game.GetSimulation()->GetVehicle()->m_rotor.m_bHeliLifting = false;
  } else if(nChar == ControllerModule::m_keymap.m_unPropellerReverse) {
    m_game.GetSimulation()->GetVehicle()->m_bPropReverse = false;
    m_game.GetSimulation()->GetVehicle()->m_rotor.m_bHeliDescending = false;
  } else if(nChar == ControllerModule::m_keymap.m_unBreak) {
    m_game.GetSimulation()->GetVehicle()->m_bBreaking = false;
  } else if(nChar == ControllerModule::m_keymap.m_unLeft) {
    m_game.GetSimulation()->GetVehicle()->m_bTurningLeft = false;
    if(m_game.GetSimulation()->GetVehicle()->m_rotor.m_nHeliMode > 399) {
      m_game.GetSimulation()->GetVehicle()->m_rotor.m_bHeliLefting = false;
    }
  } else if(nChar == ControllerModule::m_keymap.m_unRight) {
    m_game.GetSimulation()->GetVehicle()->m_bTurningRight = false;
    if(m_game.GetSimulation()->GetVehicle()->m_rotor.m_nHeliMode > 399) {
      m_game.GetSimulation()->GetVehicle()->m_rotor.m_bHeliRighting = false;
    }
  } else if(nChar == ControllerModule::m_keymap.m_unLift) {
    m_game.GetSimulation()->m_bLiftingUp = false;
  } else {
    switch(nChar) {
      case SDLK_SPACE:
        m_game.GetSimulation()->GetVehicle()->m_bHandBreaking = false;
        break;
    }
  }
}





//*************************************************************************************************
void CPakoon1View::ProcessMouseInput(unsigned nFlags, SDL_Point point) {
	//FIXME
  /*if(nFlags & MK_LBUTTON) {
    point.y = m_rectWnd.h - point.y;
    if(BGame::m_bNavSat) {
      // See if navsat resolution is to be changed
      if((point.x > (135 - 60)) && 
         (point.x < (135 + 60)) && 
         (point.y > 260) && 
         (point.y < (260 + 100))) {
        double dCurAngle = BGame::m_dNavSatHandleAngle;
        double dNewAngle;
        // possibly change resolution
        BVector vUp(0, 1, 0);
        BVector vRight(1, 0, 0);
        BVector vToMouse(point.x - 135.0, point.y - 187, 0);
        vToMouse.ToUnitLength();
        double dCos = vUp.ScalarProduct(vToMouse);
        double dTmp = vRight.ScalarProduct(vToMouse);
        if(dTmp < 0.0) {
          dNewAngle = acos(dCos);
        } else {
          dNewAngle = -acos(dCos);
        }
        dNewAngle = dNewAngle / 3.141592654 * 180.0;
        if(dNewAngle > 0.0) {
          dNewAngle = double(int((dNewAngle + 5.0) / 10.0)) * 10.0;
        } else {
          dNewAngle = double(int((dNewAngle - 5.0) / 10.0)) * 10.0;
        }
        if(fabs(dNewAngle) > 20.0) {
          dNewAngle = fabs(dNewAngle) / dNewAngle * 20.0;
        }
        if(dNewAngle != dCurAngle) {
          // Change to new resolution
          BGame::m_dNavSatHandleAngle = dNewAngle;
          if(dNewAngle == -20.0) {
            BGame::Command()->Run("set navsat resolution 650");    
          } else if(dNewAngle == -10.0) {
            BGame::Command()->Run("set navsat resolution 1300");    
          } else if(dNewAngle == 0.0) {
            BGame::Command()->Run("set navsat resolution 2600");    
          } else if(dNewAngle == 10.0) {
            BGame::Command()->Run("set navsat resolution 5200");    
          } else if(dNewAngle == 20.0) {
            BGame::Command()->Run("set navsat resolution 10400");
          }
        }
      }
    }
    if(BGame::m_bService) {
      // See if navsat resolution is to be changed
      if((point.y > m_rectWnd.h - (135 + 60)) && 
         (point.y < m_rectWnd.h - (135 - 60)) && 
         (point.x > m_rectWnd.w - (360 + 100)) && 
         (point.x < m_rectWnd.w - (360))) {
        double dCurAngle = BGame::m_dServiceHandleAngle;
        double dNewAngle;
        // possibly change resolution
        BVector vUp(-1, 0, 0);
        BVector vRight(0, 1, 0);
        BVector vToMouse(point.x - (m_rectWnd.w - 287), point.y - (m_rectWnd.h - 135.0), 0);
        vToMouse.ToUnitLength();
        double dCos = vUp.ScalarProduct(vToMouse);
        double dTmp = vRight.ScalarProduct(vToMouse);
        if(dTmp < 0.0) {
          dNewAngle = acos(dCos);
        } else {
          dNewAngle = -acos(dCos);
        }
        dNewAngle = dNewAngle / 3.141592654 * 180.0;
        if(fabs(dNewAngle) > 20.0) {
          dNewAngle = fabs(dNewAngle) / dNewAngle * 20.0;
        }
        BGame::m_dServiceHandleAngle = dNewAngle;
      }
    }
  }*/
}



//*************************************************************************************************
void CPakoon1View::OnMouseMove(unsigned nFlags, SDL_Point point) {
  if((m_pDrawFunction == &CPakoon1View::OnDrawGame) && 
     (BGame::m_bNavSat || BGame::m_bService)) {
    ProcessMouseInput(nFlags, point);
    return;
  }
}


bool g_bMouseButtonDown = false;


//*************************************************************************************************
void CPakoon1View::OnLButtonDown(unsigned nFlags, SDL_Point point) {
  g_bMouseButtonDown = true;
  if((m_pDrawFunction == &CPakoon1View::OnDrawGame) && 
     (BGame::m_bNavSat || BGame::m_bService)) {
    ProcessMouseInput(nFlags, point);
    return;
  }
}

//*************************************************************************************************
void CPakoon1View::OnLButtonUp(unsigned nFlags, SDL_Point point) {
  g_bMouseButtonDown = false;
}



//*************************************************************************************************
void CPakoon1View::DrawMouseCursor(SDL_Rect &rectWnd) {
	//FIXME
  /*SDL_Point pntMouse;
  GetCursorPos(&pntMouse);
  pntMouse.y = rectWnd.h - pntMouse.y;

  double dX = 458.0 / 512.0;
  if(g_bMouseButtonDown) {
    dX = 407.0 / 512.0;
  }

  glPushMatrix();
  glTranslatef(pntMouse.x -10, pntMouse.y + 10, 0);
  OpenGLHelpers::SetColor(1, 1, 1, 1);
  OpenGLHelpers::SwitchToTexture(0);
  BTextures::Use(BTextures::PANEL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // not mipmapped
  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(dX, 54.0 / 512);
  glVertex3f(0, 0, 0);
  OpenGLHelpers::SetTexCoord(dX, 0.0 / 512);
  glVertex3f(0, -54, 0);
  OpenGLHelpers::SetTexCoord(dX + (54.0 / 512.0), 54.0 / 512);
  glVertex3f(54, 0, 0);
  OpenGLHelpers::SetTexCoord(dX + (54.0 / 512.0), 0.0 / 512);
  glVertex3f(54, -54, 0);
  glEnd();
  glPopMatrix();*/
}




/*

  
          // Take controller into use, if not done already
          if(!ControllerModule::m_bCurrentInitialized) {
            ControllerModule::SwitchToController(ControllerModule::m_nCurrent);
          }

          // Start game
          SoundModule::StartSkidSound();
          SoundModule::StartGameMusic();
          m_game.GetSimulation()->GetCamera()->m_bNeedsToBeInitialized = true;

          // Help (invocation technique courtesy of Mr. Jacob Cody)
          {
            char sCurDir[1024];
            GetCurrentDirectory(1024, sCurDir);
            string sHelpPath = sCurDir;
            sHelpPath += "\\Help\\help.html";
            HINSTANCE hi;
            hi = ShellExecute(::GetDesktopWindow(), 
                              NULL, 
                              sHelpPath, 
                              NULL, 
                              NULL, 
                              SW_MAXIMIZE);
          }
*/


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
