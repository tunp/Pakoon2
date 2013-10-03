//
// Simulation
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#include "BSimulation.h"
#include "BObject.h"
#include "OpenGLHelpers.h"
#include "OpenGLExtFunctions.h"
#include "BTextures.h"
#include "SoundModule.h"
#include "Settings.h"
#include "BGame.h"
#include "BMessages.h"
#include "HeightMap.h"

const double BSimulation::g_cdMaxSpeed = 400.0;
const double BSimulation::g_cdTurnFactor = 0.015;
const double BSimulation::g_cdAirTurnFactor = 0.05;
const double BSimulation::g_cdPropThrustFactor = 0.05;

bool g_cbBlackAndWhite = false;
bool g_cbMipMap        = true;
extern double g_dRate;
extern double g_dPhysicsStepsInSecond;

BVector g_vInternalOffset;

//*****************************************************************************
double Random(double dRange) {
  return double(rand()) / double(RAND_MAX) * dRange;
}


BVector RandomVector(double dLength) {
  static BVector vRet;
  vRet.m_dX = Random(dLength) - dLength / 2.0;
  vRet.m_dY = Random(dLength) - dLength / 2.0;
  vRet.m_dZ = Random(dLength) - dLength / 2.0;
  return vRet;
}

//*****************************************************************************
// BASE ALGORITHM AND BASE VISUALIZATION
//*****************************************************************************

BSimulation::BSimulation() {
  m_bPaused = false;
  m_bLiftingUp = false; 
  m_dLiftZ = 0.0;
  // m_dAccelerationFactor = g_cdAccelerationFactor;
  m_dTurnFactor = g_cdTurnFactor;
  m_dAirTurnFactor = g_cdAirTurnFactor;
  m_dPropThrustFactor = g_cdPropThrustFactor;
  m_bRecordTrail = false;
  m_bRaining = false;
  m_fp = 0;
  m_nPhysicsStepsBetweenRender = 10;
  m_dPhysicsFraction = 0.0;
  m_bSteeringAidOn = true;
  m_bCalibrateSimulationSpeed = true;

  m_nDustCloudsHead1 = -1;
  m_nDustClouds1 = 0;
  m_nDustCloudsHead2 = -1;
  m_nDustClouds2 = 0;
  m_nDustCloudsHead3 = -1;
  m_nDustClouds3 = 0;

  m_nClouds = 3;

  // setup game resolution to be initially same as current
  //already set on CPakoon1View
  /*DEVMODE devmode;
  EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);
  BGame::m_nDispWidth  = devmode.dmPelsWidth;
  BGame::m_nDispHeight = devmode.dmPelsHeight;
  BGame::m_nDispBits   = devmode.dmBitsPerPel;
  BGame::m_nDispHz     = devmode.dmDisplayFrequency;*/

  // Setup camera and car location
  BVector vLocation(0, 0, 0);
  m_camera.SetLocation(vLocation);
  vLocation.m_dZ = -33.0 - 3.0;
  m_vehicle.Move(vLocation);

  // Setup targets
  m_targets = 0;

  m_nSimulationTimeStep = 0;
}



//*****************************************************************************
BSimulation::~BSimulation() {
}







//*****************************************************************************
void BSimulation::PreProcessVisualization() {
}

//*****************************************************************************
void BSimulation::PrePaint() {  

  g_vInternalOffset = GetTerrain()->m_vOffset;

	//FIXME
  /*if(ControllerModule::m_bInitialized && 
     (BGame::m_nController == 1) && 
     (ControllerModule::m_nCurrent >= 0)) {
    DIJOYSTATE stateRaw;
    if(ControllerModule::GetControllerState(BGame::m_controllerstate, &stateRaw)) {
      m_vehicle.m_dTurn = BGame::m_controllerstate.m_dTurn;

      m_bLiftingUp = false;
      if(BGame::m_controllerstate.m_bLift) {
        if(!m_bLiftingUp) {
          m_bLiftingUp = true;
          m_dLiftZ     = m_vehicle.m_pBodyPoint[m_vehicle.m_nLiftPoint1].m_vLocation.m_dZ;
        }
      }

      m_vehicle.m_bAccelerating = false; 
      m_vehicle.m_bReversing = false;
      if(BGame::m_controllerstate.m_dAcceleration > 0) {
        m_vehicle.m_dAccelerationFactor = BGame::m_controllerstate.m_dAcceleration * m_vehicle.m_dFuelFactor;
        m_vehicle.m_bAccelerating = true; 
      }
      if(BGame::m_controllerstate.m_dReverse > 0) {
        m_vehicle.m_dReversingFactor = BGame::m_controllerstate.m_dReverse * m_vehicle.m_dFuelFactor;
        m_vehicle.m_bReversing = true;
      }
      m_vehicle.m_bBreaking = BGame::m_controllerstate.m_dBrake > 0.5;

      if(BGame::m_controllerstate.m_bCameraEvent) {
        SwitchCameraMode();
      }      
    }
  }*/

  m_nPhysicsSteps = m_nPhysicsStepsBetweenRender;

  m_vehicle.m_bHitDetected = false;
  m_vehicle.m_dHitSpeed = 0;

  UpdateCar();      

  double dFraction = m_dPhysicsFraction;
  double dSimTimeFraction = m_dPhysicsFraction;

  int nExtraStep = 1;
  if(BGame::m_bSlowMotion) {
    nExtraStep = 0;
    dSimTimeFraction = 0.0;
  }

  int nPhysicsSteps = 0;

  for(int i = 0; i < m_nPhysicsSteps + nExtraStep; ++i) {
    if(i < m_nPhysicsSteps) {
      m_dPhysicsFraction = 1;
      ++nPhysicsSteps;
    } else {
      m_dPhysicsFraction = dFraction;
    }
    BVector vTmp(0, 0, 0);
    //SoundModule::Update3DSounds(m_vehicle.m_vLocation, 
    //                            m_vehicle.m_pBodyPoint[0].m_vector * 300.0,
    //                            m_camera.m_vLocation, 
    //                            m_camera.m_orientation, 
    //                            vTmp); // Wrong. Use camera's velocity.

		ApplySteering();

    m_vehicle.SimulateTimeStep();
    ++m_nSimulationTimeStep;

    // Check to increase AirTime
    if(!m_vehicle.m_bTouchMatter && 
       (fabs(BGame::m_dLiftStarted - BGame::m_dRaceTime) > g_dPhysicsStepsInSecond)) {
      BGame::m_dAirTime += m_dPhysicsFraction;
    }

    MoveCarPoints();
    UpdateCar();   

    SetUpCamera(&m_rectWnd);

    // Check for goal in multiplay
    if(BGame::m_bMultiplayOn && (BGame::m_bRaceStarted && !BGame::m_bRaceFinished)) {
      if(m_vehicle.m_vLocation.m_dY > 6000.0) {
        if(fabs(m_vehicle.m_vLocation.m_dX - GetScene()->m_vGoal.m_dX) < 25.0) {
          BGame::BroadcastInGoal();
          BGame::m_remotePlayer[BGame::GetMyPlace()].m_state = BRemotePlayer::FINISHED;
        } else {
          BGame::m_remotePlayer[BGame::GetMyPlace()].m_state = BRemotePlayer::MISSED_GOAL;
        }
        BGame::BroadcastStateChange();

        // This is ugly but we need to exit to next paint so that the race end is handled
        break;
      }
    }
  }

  if(BGame::m_bRaceStarted && !BGame::m_bRaceFinished) {
    if(BGame::m_gameMode == BGame::AIRTIME) {
      BGame::m_dRaceTime -= (double(nPhysicsSteps) + dSimTimeFraction);
    } else {
      BGame::m_dRaceTime += (double(nPhysicsSteps) + dSimTimeFraction);
    }
  }

  UpdateTrails();
  UpdateDustClouds();

  // Update Max G-Force value
  m_dMaxGForce = m_vehicle.GetMaxGForce();
}



//*****************************************************************************
void BSimulation::UpdateGameMusic() {
  // Update car engine sound
  static int nEngineBaseVol = 55;
  double dIdealRPM1 = m_vehicle.m_dSpeed * 200000.0;
  double dIdealRPM2 = m_vehicle.m_dSpeed * m_vehicle.m_pWheel[0]->m_dTTT * 200000.0;
  double dIdealRPM = dIdealRPM1 * 0.3 + dIdealRPM2 * 0.7;
  if(m_vehicle.m_bWheelsTouchGround) {
    // Try to reach ideal
    m_vehicle.m_dRPM += (dIdealRPM - m_vehicle.m_dRPM) / 10.0;
  } else {
    if(((m_vehicle.m_bAccelerating && (m_vehicle.m_dAccelerationFactor > 0.1)) || 
        (m_vehicle.m_bReversing && (m_vehicle.m_dReversingFactor > 0.1))) && 
       (m_vehicle.m_dRPM < 60000.0)) {
      m_vehicle.m_dRPM += 200.0;
    } else {
      m_vehicle.m_dRPM *= 0.99;
    }
  }
  SoundModule::SetGameMusicRPM(int(m_vehicle.m_dRPM));
}


//*****************************************************************************
void BSimulation::UpdateAirplaneControls() {

  // Apply propeller thrust

  if((m_vehicle.m_bPropeller || m_vehicle.m_bPropReverse) && (m_vehicle.m_dKerosineFactor > 1e-6)) {
    BVector vProp = (m_vehicle.m_orientation.m_vRight * m_vehicle.m_airplane.m_vPropDir.m_dX +
                     m_vehicle.m_orientation.m_vForward * m_vehicle.m_airplane.m_vPropDir.m_dY +
                     m_vehicle.m_orientation.m_vUp * -m_vehicle.m_airplane.m_vPropDir.m_dZ);
    vProp.ToUnitLength();
    vProp = vProp * m_vehicle.m_dPropellerFactor * m_vehicle.m_airplane.m_dPropEffect * m_vehicle.m_dHorsePowers * 0.01;

    m_vehicle.m_pBodyPoint[m_vehicle.m_airplane.m_nPropBodyPoint].m_vector += vProp;
  }

  // Rotate Propeller
  m_vehicle.m_airplane.m_dPropAngle += 2.1 * fabs(m_vehicle.m_dPropellerFactor) * 20.0;

  // Control surface forces are applied in PPEC_Vehicle::SimulateTimeStep()
}




//*****************************************************************************
void BSimulation::SwitchCameraMode() {
  switch(GetCamera()->m_locMode) {
    case BCamera::FIXED:
      GetCamera()->m_locMode = BCamera::FOLLOW;
      GetCamera()->m_dAngleOfView = 75.0;
      BMessages::Remove("camera");
      BMessages::Show(60, "camera", "camera: chase", 1);
      break;
    case BCamera::FOLLOW:
      GetCamera()->m_locMode = BCamera::OVERVIEW;
      GetCamera()->m_bInitLoc = true;
      GetCamera()->m_dAngleOfView = 80.0;
      BMessages::Remove("camera");
      BMessages::Show(60, "camera", "camera: overview", 1);
      break;
    case BCamera::OVERVIEW:
      GetCamera()->m_locMode = BCamera::INCAR;
      GetCamera()->m_dAngleOfView = 75.0;
      BMessages::Remove("camera");
      BMessages::Show(60, "camera", "camera: 1st person", 1);
      break;
    case BCamera::INCAR:
      GetCamera()->m_locMode = BCamera::ONSIDE;
      GetCamera()->m_dAngleOfView = 75.0;
      BMessages::Remove("camera");
      BMessages::Show(60, "camera", "camera: front wheel", 1);
      break;
    case BCamera::ONSIDE:
      GetCamera()->m_locMode = BCamera::SIDEVIEW;
      GetCamera()->m_dAngleOfView = 75.0;
      BMessages::Remove("camera");
      BMessages::Show(60, "camera", "camera: side", 1);
      break;
    case BCamera::SIDEVIEW:
      GetCamera()->m_locMode = BCamera::FIXED;
      GetCamera()->m_vFixLocation = GetCamera()->m_vLocation;
      GetCamera()->m_dAngleOfView = 75.0;
      BMessages::Remove("camera");
      BMessages::Show(60, "camera", "camera: stationary", 1);
      break;
  }
}




const double g_cdPI = 3.141592654;

//*****************************************************************************
void BSimulation::PaintSky(float fBrightness, bool bFog) {

  static bool bDLCreated = false;
  static int  nDLSky = -1;
  static int  nDLNightSky = -1;

  if(!bDLCreated) {
    bDLCreated = true;
    if(nDLSky == -1) {
      nDLSky = glGenLists(1);
    }
    if(nDLNightSky == -1) {
      nDLNightSky = glGenLists(1);
    }
    glNewList(nDLSky, GL_COMPILE_AND_EXECUTE);
    glDisable(GL_FOG);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT);

    GLfloat fCurColor[4];
    fCurColor[0] = (GLfloat) 1;
    fCurColor[1] = (GLfloat) 1;
    fCurColor[2] = (GLfloat) 1;
    fCurColor[3] = (GLfloat) 1;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, fCurColor);
    fCurColor[0] = (GLfloat) 0;
    fCurColor[1] = (GLfloat) 0;
    fCurColor[2] = (GLfloat) 0;
    fCurColor[3] = (GLfloat) 1;
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, fCurColor);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, fCurColor);

    GLfloat fLight1AmbientG[ 4];
    fLight1AmbientG[0] = 1;
    fLight1AmbientG[1] = 1;
    fLight1AmbientG[2] = 1;
    fLight1AmbientG[3] = 1;
    glLightfv( GL_LIGHT0, GL_AMBIENT,  fLight1AmbientG);

    double dBlue, dPower = 0.6;
    double y;
    for(y = 0.0; y < 19; y += 1.0) {
      glBegin(GL_TRIANGLE_STRIP);
      for(double x = 0; x < 21; x += 1.0) {
        dBlue = pow(y / 20.0, dPower);
        glColor3f((1.0 - dBlue) * 0.8, (1.0 - dBlue) * 0.8, 1 - dBlue * 0.7);
        glVertex3f(cos(x / 20.0 * 2.0 * g_cdPI) * cos(y / 20.0 * g_cdPI / 2.0) * cdWorldHemisphereRadius, 
                   sin(x / 20.0 * 2.0 * g_cdPI) * cos(y / 20.0 * g_cdPI / 2.0) * cdWorldHemisphereRadius, 
                   sin(y / 20.0 * g_cdPI / 2.0) * -cdWorldHemisphereRadius);
        dBlue = pow((y + 1.0) / 20.0, dPower);
        glColor3f((1.0 - dBlue) * 0.8, (1.0 - dBlue) * 0.8, 1 - dBlue * 0.7);
        glVertex3f(cos(x / 20.0 * 2.0 * g_cdPI) * cos((y + 1.0) / 20.0 * g_cdPI / 2.0) * cdWorldHemisphereRadius, 
                   sin(x / 20.0 * 2.0 * g_cdPI) * cos((y + 1.0) / 20.0 * g_cdPI / 2.0) * cdWorldHemisphereRadius, 
                   sin((y + 1.0) / 20.0 * g_cdPI / 2.0) * -cdWorldHemisphereRadius);
      }
      glEnd();
    }
    glBegin(GL_TRIANGLE_FAN);
    dBlue = 1.0;
    glColor3f((1.0 - dBlue) * 0.8, (1.0 - dBlue) * 0.8, 1 - dBlue * 0.7);
    glVertex3f(0.0, 0.0, -cdWorldHemisphereRadius);
    dBlue = pow(19.0 / 20.0, dPower);
    glColor3f((1.0 - dBlue) * 0.8, (1.0 - dBlue) * 0.8, 1 - dBlue * 0.7);
    double x;
    for(x = 20; x >= 0; x -= 1.0) {
          glVertex3f(cos(x / 20.0 * 2.0 * g_cdPI) * cos(19.0 / 20.0 * g_cdPI / 2.0) * cdWorldHemisphereRadius, 
                     sin(x / 20.0 * 2.0 * g_cdPI) * cos(19.0 / 20.0 * g_cdPI / 2.0) * cdWorldHemisphereRadius, 
                     sin(19.0 / 20.0 * g_cdPI / 2.0) * -cdWorldHemisphereRadius);
    }
    glEnd();

    fLight1AmbientG[0] = 0.2f;
    fLight1AmbientG[1] = 0.2f;
    fLight1AmbientG[2] = 0.2f;
    fLight1AmbientG[3] = 0.2f;
    glLightfv( GL_LIGHT0, GL_AMBIENT,  fLight1AmbientG);

    glDisable(GL_COLOR_MATERIAL);
    glEnable(GL_FOG);
    glEndList();

    // NIGHT SKY

    glNewList(nDLNightSky, GL_COMPILE_AND_EXECUTE);
    glDisable(GL_FOG);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT);

    fCurColor[0] = (GLfloat) 1;
    fCurColor[1] = (GLfloat) 1;
    fCurColor[2] = (GLfloat) 1;
    fCurColor[3] = (GLfloat) 1;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, fCurColor);
    fCurColor[0] = (GLfloat) 0;
    fCurColor[1] = (GLfloat) 0;
    fCurColor[2] = (GLfloat) 0;
    fCurColor[3] = (GLfloat) 1;
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, fCurColor);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, fCurColor);

    fLight1AmbientG[0] = 1;
    fLight1AmbientG[1] = 1;
    fLight1AmbientG[2] = 1;
    fLight1AmbientG[3] = 1;
    glLightfv( GL_LIGHT0, GL_AMBIENT,  fLight1AmbientG);

    dPower = 0.4;
    for(y = 0.0; y < 19; y += 1.0) {
      glBegin(GL_TRIANGLE_STRIP);
      for(double x = 0; x < 21; x += 1.0) {
        dBlue = pow(y / 20.0, dPower);
        glColor3f(0, 0, (1.0 - dBlue) * 0.2);
        glVertex3f(cos(x / 20.0 * 2.0 * g_cdPI) * cos(y / 20.0 * g_cdPI / 2.0) * cdWorldHemisphereRadius, 
                   sin(x / 20.0 * 2.0 * g_cdPI) * cos(y / 20.0 * g_cdPI / 2.0) * cdWorldHemisphereRadius, 
                   sin(y / 20.0 * g_cdPI / 2.0) * -cdWorldHemisphereRadius);
        dBlue = pow((y + 1.0) / 20.0, dPower);
        glColor3f(0, 0, (1.0 - dBlue) * 0.2);
        glVertex3f(cos(x / 20.0 * 2.0 * g_cdPI) * cos((y + 1.0) / 20.0 * g_cdPI / 2.0) * cdWorldHemisphereRadius, 
                   sin(x / 20.0 * 2.0 * g_cdPI) * cos((y + 1.0) / 20.0 * g_cdPI / 2.0) * cdWorldHemisphereRadius, 
                   sin((y + 1.0) / 20.0 * g_cdPI / 2.0) * -cdWorldHemisphereRadius);
      }
      glEnd();
    }
    glBegin(GL_TRIANGLE_FAN);
    dBlue = 1.0;
    glColor3f(0, 0, (1.0 - dBlue) * 0.2);
    glVertex3f(0.0, 0.0, -cdWorldHemisphereRadius);
    dBlue = pow(19.0 / 20.0, dPower);
    glColor3f(0, 0, (1.0 - dBlue) * 0.2);
    for(x = 20; x >= 0; x -= 1.0) {
          glVertex3f(cos(x / 20.0 * 2.0 * g_cdPI) * cos(19.0 / 20.0 * g_cdPI / 2.0) * cdWorldHemisphereRadius, 
                     sin(x / 20.0 * 2.0 * g_cdPI) * cos(19.0 / 20.0 * g_cdPI / 2.0) * cdWorldHemisphereRadius, 
                     sin(19.0 / 20.0 * g_cdPI / 2.0) * -cdWorldHemisphereRadius);
    }
    glEnd();

    fLight1AmbientG[0] = 0.2f;
    fLight1AmbientG[1] = 0.2f;
    fLight1AmbientG[2] = 0.2f;
    fLight1AmbientG[3] = 0.2f;
    glLightfv( GL_LIGHT0, GL_AMBIENT,  fLight1AmbientG);

    glDisable(GL_COLOR_MATERIAL);
    glEnable(GL_FOG);
    glEndList();

  } else {
    glPushMatrix();
    glTranslated(m_camera.m_vLocation.m_dX, m_camera.m_vLocation.m_dY, m_camera.m_vLocation.m_dZ);
    if(BGame::m_bNight) {
      glCallList(nDLNightSky);
    } else {
      glCallList(nDLSky);
    }
    glPopMatrix();
  }
}








//*****************************************************************************
int BSimulation::Paint(bool bCreateDLs, bool bWireframe, bool bNormals, SDL_Rect &rectWnd) {

  // Render sky
  if(BGame::m_nVisualize & BGame::SKY) {
    PaintSky(1.0f, false);
  }

  // Update terrain database
  int nOffTime = 0;
  glPushMatrix();
  if(BGame::m_nVisualize & BGame::TERRAIN) {
    clock_t clockStart = clock();
    m_terrain.MakeTerrainValid(m_vehicle.m_vLocation, 
                               m_camera.m_vLocation, 
                               m_camera.m_orientation.m_vForward, 
                               bCreateDLs, 
                               bWireframe, 
                               bNormals);
    nOffTime = clock() - clockStart;
    
    // Render terrain
    if(BGame::m_bNight) {
      OpenGLHelpers::SetMoonLighting();
    } else {
      OpenGLHelpers::SetDefaultLighting();
    }
    // OpenGLHelpers::SetEveningLighting();
    // OpenGLHelpers::SetMoonLighting();

    glDisable(GL_BLEND);

    m_terrain.Render(BGame::m_nSkyDetail, 
                     m_camera.m_vLocation, 
                     m_camera.m_orientation.m_vForward, 
                     0); 

    if(BGame::m_nShowEffects > 0) {
      m_terrain.RenderAlphaCircle(m_vehicle.m_vLocation, 5.0, 100.0);
    }
  }

  // Render Objects
  if(BGame::m_nVisualize & BGame::OBJECTS) {
    PaintSceneObjects();
  }

  glShadeModel(GL_SMOOTH);
  OpenGLHelpers::SetColorFull(1, 1, 1, 1);

  // Render shadows and trail marks (they all use same texture)
  if(BGame::m_nVisualize & BGame::DUSTANDCLOUDS) {
    if(!BGame::m_bSceneEditorMode) {
      OpenGLHelpers::SwitchToTexture(0);
      BTextures::Use(BTextures::SHADOW);
      glEnable(GL_BLEND);
      glDisable(GL_DEPTH_TEST);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      DrawShadowAndTrails();
      glPopMatrix();

      glDisable(GL_BLEND);
      glEnable(GL_DEPTH_TEST);
    }
  }
  glPopMatrix();

  // Render vehicle
  if(BGame::m_nVisualize & BGame::VEHICLE) {
    if((GetCamera()->m_locMode != BCamera::INCAR) && (!BGame::m_bSceneEditorMode)) {
      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);
      m_vehicle.Paint(m_nPhysicsSteps);
      glDisable(GL_CULL_FACE);
    }
  }

  DrawDustClouds();

  // If hit detected, play crash sound
  if(m_vehicle.m_bHitDetected) {
    SoundModule::PlayCrashSound(m_vehicle.m_dHitSpeed * 6.0);
    BVector vTmp(0, 0, 0);
    //SoundModule::Update3DSounds(m_vehicle.m_vLocation, 
    //                            m_vehicle.m_pBodyPoint[0].m_vector * 300.0,
    //                            m_camera.m_vLocation, 
    //                            m_camera.m_orientation, 
    //                            vTmp); // Wrong. Use camera's velocity.
  }

  return nOffTime;
}



//*****************************************************************************
void BSimulation::PaintSceneObjects() {
  // Render all objects within world sphere
  BScene *pScene = GetScene();
  BCamera *pCamera = GetCamera();
  int nObject;

  // First shadows

  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CW);
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
  glCullFace(GL_BACK);

  glDisable(GL_TEXTURE_2D); 
  OpenGLHelpers::SetColorFull(0, 0, 0, 0.3);
  for(nObject = 0; nObject < pScene->m_nObjects; ++nObject) {
    pScene->m_pObjects[nObject].m_bVisible = true;
    BVector vCamToObj = pScene->m_pObjects[nObject].m_vCenter - pCamera->m_vLocation;
    double dProjOnCamForward = pCamera->m_orientation.m_vForward.ScalarProduct(vCamToObj);
    // see if object is visible
    double dDist = (pScene->m_pObjects[nObject].m_vLocation - pCamera->m_vLocation).Length();
    if((dProjOnCamForward > -pScene->m_pObjects[nObject].m_dRadius) && 
       (vCamToObj.Length() < 1100.0)) {
      if(pScene->m_pObjects[nObject].m_bHasShadow) {
        glPushMatrix();
        glTranslated(pScene->m_pObjects[nObject].m_vLocation.m_dX,
                     pScene->m_pObjects[nObject].m_vLocation.m_dY,
                     0.0);
        glRotated(pScene->m_pObjects[nObject].m_dZRotation, 0, 0, -1);
        glScaled(pScene->m_pObjects[nObject].m_dScale2, 
                 pScene->m_pObjects[nObject].m_dScale2,
                 1.0);
        pScene->m_pObjects[nObject].DrawObject(true);
        glPopMatrix();
      }
    } else {
      pScene->m_pObjects[nObject].m_bVisible = false;
    }
  }

  glDisable(GL_COLOR_MATERIAL);
  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);

  // Then objects

  for(nObject = 0; nObject < pScene->m_nObjects; ++nObject) {
    if(pScene->m_pObjects[nObject].m_bVisible) {
      glPushMatrix();
      glTranslated(pScene->m_pObjects[nObject].m_vLocation.m_dX,
                   pScene->m_pObjects[nObject].m_vLocation.m_dY,
                   pScene->m_pObjects[nObject].m_vLocation.m_dZ);
      glRotated(pScene->m_pObjects[nObject].m_dZRotation, 0, 0, -1);
      glScaled(pScene->m_pObjects[nObject].m_dScale2, 
               pScene->m_pObjects[nObject].m_dScale2,
               pScene->m_pObjects[nObject].m_dScale2);
      pScene->m_pObjects[nObject].DrawObject(false);
      glPopMatrix();
    }
  }
  glFrontFace(GL_CCW);
}





//*****************************************************************************
static const double dPI2 = 2.0 * g_cdPI;




//*****************************************************************************
// PHYSICS SIMULATION
//*****************************************************************************



//*****************************************************************************
void BSimulation::EnsureVehicleIsOverGround() {
  int i;

  double dMaxRad = 0.0;

  // First find biggest wheel
  for(i = 0; i < m_vehicle.m_nWheels; ++i) {
    if(m_vehicle.m_pWheel[i]->m_dRadius > dMaxRad) {
      dMaxRad = m_vehicle.m_pWheel[i]->m_dRadius;
    }
  }

  double dMaxLift = 0.0;
  for(i = 0; i < m_vehicle.m_nBodyPoints; ++i) {
    BVector vNormal;
    BVector v = m_vehicle.m_pBodyPoint[i].m_vLocation;
    v.m_dZ += dMaxRad;
    double depth = PointUnderGroundShadow(v, vNormal);
    if(depth > dMaxLift) {
      dMaxLift = depth;
    }
  }
  m_vehicle.Move(BVector(0, 0, -1.0 - dMaxLift));
  UpdateCar();
}




//*****************************************************************************
void BSimulation::UpdateCarLocation() {
  int i;
  m_vehicle.m_vLocation.Set(0, 0, 0);
  for(i = 0; i < m_vehicle.m_nBodyPoints; ++i) {
    m_vehicle.m_vLocation += m_vehicle.m_pBodyPoint[i].m_vLocation;
  }
  m_vehicle.m_vLocation = m_vehicle.m_vLocation * (1.0 / double(m_vehicle.m_nBodyPoints));
}





//*****************************************************************************
void BSimulation::UpdateCar() {
  // Place car
  UpdateCarLocation();

  m_pCenterBlock = FindTerrainBlock(m_vehicle.m_vLocation);

  // Determine car orientation
  m_vehicle.m_orientation.m_vForward = (m_vehicle.m_pBodyPoint[m_vehicle.m_nForwardPoints[1]].m_vLocation - m_vehicle.m_pBodyPoint[m_vehicle.m_nForwardPoints[0]].m_vLocation) +
                                       (m_vehicle.m_pBodyPoint[m_vehicle.m_nForwardPoints[3]].m_vLocation - m_vehicle.m_pBodyPoint[m_vehicle.m_nForwardPoints[2]].m_vLocation);
  m_vehicle.m_orientation.m_vForward.ToUnitLength();
  m_vehicle.m_orientation.m_vRight = (m_vehicle.m_pBodyPoint[m_vehicle.m_nRightPoints[1]].m_vLocation - m_vehicle.m_pBodyPoint[m_vehicle.m_nRightPoints[0]].m_vLocation) +
                                     (m_vehicle.m_pBodyPoint[m_vehicle.m_nRightPoints[3]].m_vLocation - m_vehicle.m_pBodyPoint[m_vehicle.m_nRightPoints[2]].m_vLocation);
  m_vehicle.m_orientation.m_vRight.ToUnitLength();
  m_vehicle.m_orientation.m_vUp = m_vehicle.m_orientation.m_vRight.CrossProduct(m_vehicle.m_orientation.m_vForward);
  m_vehicle.m_orientation.m_vUp.ToUnitLength();
  m_vehicle.m_orientation.m_vUp = m_vehicle.m_orientation.m_vUp * -1.0;

  // Determine car vector and speed
  m_vehicle.m_vector = (m_vehicle.m_pBodyPoint[m_vehicle.m_nForwardPoints[0]].m_vector + 
                        m_vehicle.m_pBodyPoint[m_vehicle.m_nForwardPoints[1]].m_vector +
                        m_vehicle.m_pBodyPoint[m_vehicle.m_nForwardPoints[2]].m_vector +
                        m_vehicle.m_pBodyPoint[m_vehicle.m_nForwardPoints[3]].m_vector) * 0.25;
  m_vehicle.m_dSpeed = m_vehicle.m_vector.Length();

  // Place wheels
  if(m_bSteeringAidOn) {
    m_vehicle.m_dSteeringAid = 1.0 - (m_vehicle.m_dSpeed / 0.3);
    if(m_vehicle.m_dSteeringAid < 0.3) {
      m_vehicle.m_dSteeringAid = 0.3;
    }
  } else {
    m_vehicle.m_dSteeringAid = 1.0;
  }
  for(int w = 0; w < m_vehicle.m_nWheels; ++w) {
    BWheel *pWheel = dynamic_cast<BWheel *>(m_vehicle.m_pWheel[w]);
    BVector vLoc = pWheel->m_vSuspBasePoint +
                   pWheel->m_vSuspDir * 
                   pWheel->m_dSuspRelaxedDistance +
                   pWheel->m_vSuspDir * 
                  -pWheel->m_dSuspension;
    pWheel->m_vLocation = m_vehicle.ToWorldCoord(vLoc);
    pWheel->m_orientation = m_vehicle.m_orientation;
    if(pWheel->m_bTurns) {
      // Apply steering to turning wheels
      pWheel->m_orientation.m_vForward += pWheel->m_orientation.m_vRight * m_vehicle.m_dTurn * m_vehicle.m_dSteeringAid * (pWheel->m_dThrow / 40.0);
      pWheel->m_orientation.m_vForward.ToUnitLength();
      pWheel->m_orientation.m_vRight = pWheel->m_orientation.m_vUp.CrossProduct(pWheel->m_orientation.m_vForward);
      pWheel->m_orientation.m_vRight.ToUnitLength();
    }
  }
}






//*****************************************************************************
void BSimulation::ApplySteering() {

  // Check for joystick info
  if(true) { // Always support keyboard as well
    // Use keyboard turning
    // If turning, do so
    if(m_vehicle.m_bTurningLeft) {
      // Turn left
      if(m_vehicle.m_dTurn > -0.7)  {
        m_vehicle.m_dTurn -= m_dTurnFactor;
      }
    } else if(m_vehicle.m_bTurningRight) {
      // Turn right
      if(m_vehicle.m_dTurn < 0.7)  {
        m_vehicle.m_dTurn += m_dTurnFactor;
      }
    } else {
      // Center wheels
      if(m_vehicle.m_dTurn > m_dTurnFactor * 3.0) {
        m_vehicle.m_dTurn -= m_dTurnFactor * 3.0;
      } else if(m_vehicle.m_dTurn < -m_dTurnFactor) {
        m_vehicle.m_dTurn += m_dTurnFactor * 3.0;
      } else {
        m_vehicle.m_dTurn = 0.0;
      }
    }
  }
} 



//*****************************************************************************
void BSimulation::MoveCarPoints() {
  for(int i = 0; i < m_vehicle.m_nBodyPoints; ++i) {
    m_vehicle.m_pBodyPoint[i].m_vLocation += (m_vehicle.m_pBodyPoint[i].m_vector * m_dPhysicsFraction);
  }
} 



//*****************************************************************************
double BSimulation::PointInsideObject(BVector& rvPoint, BVector& rvNormal, double &rdFriction, double &rdBaseDepth) {
  double depth;
  for(int o = 0; o < m_pCenterBlock->m_nObjects; ++o) {
    if((depth = m_pCenterBlock->m_objectArray[o]->PointIsInsideObject(rvPoint, rvNormal, rdFriction, rdBaseDepth)) > 0.0) {
      return depth;
    }
  }
  return -1.0;
}


//*****************************************************************************
double BSimulation::PointInsideRemoteCar(BVector& rvPoint, BVector& rvNormal, double &rdFriction, double &rdBaseDepth) {
  double  dDepth;
  double  dMinDepth = 999.9;
  BVector dMinNormal;
  for(int nCar = 0; nCar < BGame::m_nRemotePlayers; ++nCar) {
    if(BGame::m_remotePlayer[nCar].m_bSelf) {
      continue;
    }
    // Check against remote car box
    double dTimePassed = 0;

    int dwNow = BGame::GetMultiplayClock();
    if(dwNow > BGame::m_remotePlayer[nCar].m_clockLocationSent) {
      dTimePassed = double(dwNow - BGame::m_remotePlayer[nCar].m_clockLocationSent);
    } else {
      dTimePassed = -double(BGame::m_remotePlayer[nCar].m_clockLocationSent - dwNow);
    }

    BVector vToP = rvPoint - 
                   (BGame::m_remotePlayer[nCar].m_vLocation + 
                    BGame::m_remotePlayer[nCar].m_vVelocity * dTimePassed +
                    BGame::m_remotePlayer[nCar].m_vVelo1stDeriv * dTimePassed// +
                    /*BGame::m_remotePlayer[nCar].m_vVelo2ndDeriv * dTimePassed*/);

    if(vToP.Length() < BGame::m_remotePlayer[nCar].m_dRadius) {

      // Check against box sides to see collision.
      // Point must be inside every side to make a collision.

      // RIGHT SIDE
      double dProjected = BGame::m_remotePlayer[nCar].m_orientation.m_vRight.ScalarProduct(vToP);
      if(dProjected < BGame::m_remotePlayer[nCar].m_dWidth) {
        dMinDepth = BGame::m_remotePlayer[nCar].m_dWidth - dProjected;
        dMinNormal = BGame::m_remotePlayer[nCar].m_orientation.m_vRight;

        // LEFT SIDE
        dProjected = (BGame::m_remotePlayer[nCar].m_orientation.m_vRight * -1.0).ScalarProduct(vToP);
        dDepth = BGame::m_remotePlayer[nCar].m_dWidth - dProjected;
        if(dDepth > 0) {          
          if(dDepth < dMinDepth) {
            dMinDepth = dDepth;
            dMinNormal = BGame::m_remotePlayer[nCar].m_orientation.m_vRight * -1.0;
          }

          // FRONT SIDE
          dProjected = BGame::m_remotePlayer[nCar].m_orientation.m_vForward.ScalarProduct(vToP);
          dDepth = BGame::m_remotePlayer[nCar].m_dLen - dProjected;
          if(dDepth > 0) {          
            if(dDepth < dMinDepth) {
              dMinDepth = dDepth;
              dMinNormal = BGame::m_remotePlayer[nCar].m_orientation.m_vForward;
            }

            // BACK SIDE
            dProjected = (BGame::m_remotePlayer[nCar].m_orientation.m_vForward * -1.0).ScalarProduct(vToP);
            dDepth = BGame::m_remotePlayer[nCar].m_dLen - dProjected;
            if(dDepth > 0) {          
              if(dDepth < dMinDepth) {
                dMinDepth = dDepth;
                dMinNormal = BGame::m_remotePlayer[nCar].m_orientation.m_vForward * -1.0;
              }

              // TOP SIDE
              dProjected = BGame::m_remotePlayer[nCar].m_orientation.m_vUp.ScalarProduct(vToP);
              dDepth = BGame::m_remotePlayer[nCar].m_dHeight - dProjected;
              if(dDepth > 0) {          
                if(dDepth < dMinDepth) {
                  dMinDepth = dDepth;
                  dMinNormal = BGame::m_remotePlayer[nCar].m_orientation.m_vUp;
                }

                // BOTTOM SIDE
                dProjected = (BGame::m_remotePlayer[nCar].m_orientation.m_vUp * -1.0).ScalarProduct(vToP);
                dDepth = BGame::m_remotePlayer[nCar].m_dHeight - dProjected;
                if(dDepth > 0) {          
                  if(dDepth < dMinDepth) {
                    dMinDepth = dDepth;
                    dMinNormal = BGame::m_remotePlayer[nCar].m_orientation.m_vUp * -1.0;
                  }
                  rdBaseDepth = 0.5 * dMinDepth * (BGame::m_remotePlayer[nCar].m_dTotalMass / (BGame::m_remotePlayer[nCar].m_dTotalMass + GetVehicle()->m_dTotalMass));
                  rvNormal = dMinNormal;
                  rdFriction = 0.2;
                  return rdBaseDepth;
                }
              }
            }
          }
        }
      }
    }
  }
  return -1.0;
}








//*****************************************************************************
BTerrainBlock *BSimulation::FindTerrainBlock(BVector &rvPoint) {
  // Find the block where the point lies.
  BTerrainBlock *pCenter = m_terrain.m_ringVisible.GetHead();
  if(pCenter->PointIsInsideBlock(rvPoint)) {
    return pCenter;
  } else {
    // loop through the list to find the correct block
    BTerrainBlock *pRet = pCenter->m_pNext;
    while(pRet != pCenter) {
      if(pRet->PointIsInsideBlock(rvPoint)) {
        return pRet;
      }
      pRet = pRet->m_pNext;
    }
  }
  return 0;
}




// *********************************
// *********************************
// *********************************
// *********************************
// OLD TRIANGLE BASED VERSION
// *********************************
// *********************************
// *********************************
// *********************************

//*****************************************************************************
double BSimulation::PointUnderGround(BVector vPoint, 
                                     BVector& rvNormal, 
                                     double &rdFriction, 
                                     double &rdBaseDepth, 
                                     double &rdThermoLoss) {

  //if(BGame::m_nTerrainResolution > 1) {
  //  return PointUnderGroundAccurate(vPoint, rvNormal, rdFriction, rdBaseDepth, rdThermoLoss);
  //}

  // Find corresponding triangle and check depth from there
  BVector vPoint2 = vPoint;

  BTerrainBlock *pBlock = FindTerrainBlock(vPoint);
  if(pBlock) {
    vPoint.m_dX -= pBlock->m_vCorner.m_dX;
    vPoint.m_dY -= pBlock->m_vCorner.m_dY;

    static double dX, dY, dXRes, dYRes;
    double dTileSize = pBlock->m_dTileSize;
    int    nStep     = pBlock->m_nStep;
    int    nTiles    = pBlock->m_nSize - 1;
    dX = vPoint.m_dX / dTileSize;
    dY = vPoint.m_dY / dTileSize;

    // Square has been found. Check which triangle to use
    int x = int(dX);
    int y = int(dY);
    dXRes = dX - double(x);
    dYRes = dY - double(y);
    if(dXRes > dYRes) {
      // Check against triangle 1
      rvNormal = pBlock->m_pTiles[y * (nTiles / nStep) + x].m_vNormal1;
      // calculate depth
      double d1 = -pBlock->HeightAt(x       * nStep, y       * nStep);
      double d2 = -pBlock->HeightAt((x + 1) * nStep, y       * nStep);
      double d4 = -pBlock->HeightAt((x + 1) * nStep, (y + 1) * nStep);
      double dRet = vPoint.m_dZ - (d1 + dXRes * (d2 - d1) + dYRes * (d4 - d2));
      rdFriction = pBlock->m_pTiles[y * (nTiles / nStep) + x].m_dFriction;
      rdBaseDepth = rvNormal.ScalarProduct(BVector(0, 0, -dRet));
      return rdBaseDepth;
    } else {
      // Check against triangle 2
      rvNormal = pBlock->m_pTiles[y * (nTiles / nStep) + x].m_vNormal2;
      // calculate depth
      double d1 = -pBlock->HeightAt(x       * nStep, y       * nStep);
      double d3 = -pBlock->HeightAt(x       * nStep, (y + 1) * nStep);
      double d4 = -pBlock->HeightAt((x + 1) * nStep, (y + 1) * nStep);
      double dRet = vPoint.m_dZ - (d1 + dYRes * (d3 - d1) + dXRes * (d4 - d3));
      rdFriction = pBlock->m_pTiles[y * (nTiles / nStep) + x].m_dFriction;
      rdBaseDepth = rvNormal.ScalarProduct(BVector(0, 0, -dRet));
      return rdBaseDepth;
    }
  } else {
    // Something's wrong. Correct block was not found.
    rdFriction = 0.5; 
    rdBaseDepth = vPoint.m_dZ;
    rvNormal.Set(0.0, 0.0, -1.0);
    return vPoint.m_dZ;
  }
}

// *********************************
// *********************************
// *********************************
// *********************************
// NEW HEIGHT ALGORITHM BASED VERSION
// *********************************
// *********************************
// *********************************
// *********************************

//*****************************************************************************
double BSimulation::PointUnderGroundAccurate(BVector vPoint, 
                                             BVector& rvNormal, 
                                             double &rdFriction, 
                                             double &rdBaseDepth, 
                                             double &rdThermoLoss) {
  // Find corresponding triangle and check depth from there
  double dTmp;
  BVector vPoint2 = vPoint;

  BTerrainBlock *pBlock = FindTerrainBlock(vPoint);
  if(pBlock) {
    vPoint.m_dX -= pBlock->m_vCorner.m_dX;
    vPoint.m_dY -= pBlock->m_vCorner.m_dY;

    static double dX, dY, dXRes, dYRes;
    double dTileSize = pBlock->m_dTileSize;
    int    nStep     = pBlock->m_nStep;
    int    nTiles    = pBlock->m_nSize - 1;
    dX = vPoint.m_dX / dTileSize;
    dY = vPoint.m_dY / dTileSize;

    // Square has been found. Check which triangle to use
    int x = int(dX);
    int y = int(dY);
    dXRes = dX - double(x);
    dYRes = dY - double(y);
    if(dXRes > dYRes) {
      // Check against triangle 1
      rvNormal = pBlock->m_pTiles[y * (nTiles / nStep) + x].m_vNormal1;      
    } else {
      // Check against triangle 2
      rvNormal = pBlock->m_pTiles[y * (nTiles / nStep) + x].m_vNormal2;
    }
    rdFriction = pBlock->m_pTiles[y * (nTiles / nStep) + x].m_dFriction;
    // calculate depth
    rdBaseDepth = vPoint2.m_dZ + HeightMap::CalcHeightAt(g_vInternalOffset.m_dX + vPoint2.m_dX, 
                                                         g_vInternalOffset.m_dY + vPoint2.m_dY, 
                                                         dTmp, 
                                                         m_scene.m_terrainStyle,
                                                         8,
                                                         g_vInternalOffset.m_dY);
    rdBaseDepth = rvNormal.ScalarProduct(BVector(0, 0, -rdBaseDepth));
    return rdBaseDepth;
  } else {
    // Something's wrong. Correct block was not found.
    rdFriction = 0.5; 
    rdBaseDepth = vPoint.m_dZ;
    rvNormal.Set(0.0, 0.0, -1.0);
    return vPoint.m_dZ;
  }
}

//*****************************************************************************
double BSimulation::PointUnderGroundShadow(BVector vPoint, BVector& rvNormal) {
  // Find corresponding triangle and check depth from there
  BVector vObjectTest = vPoint;
  double dRet = vPoint.m_dZ;
  rvNormal.Set(0, 0, -1);

  BTerrainBlock *pBlock = FindTerrainBlock(vPoint);
  if(pBlock) {
    vPoint.m_dX -= pBlock->m_vCorner.m_dX;
    vPoint.m_dY -= pBlock->m_vCorner.m_dY;

    static double dX, dY, dXRes, dYRes;
    double dTileSize = pBlock->m_dTileSize;
    int    nStep     = pBlock->m_nStep;
    int    nTiles    = pBlock->m_nSize - 1;
    dX = vPoint.m_dX / dTileSize;
    dY = vPoint.m_dY / dTileSize;

    // Square has been found. Check which triangle to use
    int x = int(dX);
    int y = int(dY);
    dXRes = dX - double(x);
    dYRes = dY - double(y);
    if(dXRes > dYRes) {
      // Check against triangle 1
      rvNormal = pBlock->m_pTiles[y * (nTiles / nStep) + x].m_vNormal1;
      // calculate depth
      double d1 = -pBlock->HeightAt(x       * nStep, y       * nStep);
      double d2 = -pBlock->HeightAt((x + 1) * nStep, y       * nStep);
      double d4 = -pBlock->HeightAt((x + 1) * nStep, (y + 1) * nStep);
      dRet = vPoint.m_dZ - (d1 + dXRes * (d2 - d1) + dYRes * (d4 - d2));
    } else {
      // Check against triangle 2
      rvNormal = pBlock->m_pTiles[y * (nTiles / nStep) + x].m_vNormal2;
      // calculate depth
      double d1 = -pBlock->HeightAt(x       * nStep, y       * nStep);
      double d3 = -pBlock->HeightAt(x       * nStep, (y + 1) * nStep);
      double d4 = -pBlock->HeightAt((x + 1) * nStep, (y + 1) * nStep);
      dRet = vPoint.m_dZ - (d1 + dYRes * (d3 - d1) + dXRes * (d4 - d3));
    }
  } else {
    // Something's wrong. Correct block was not found. 
    rvNormal.Set(0.0, 0.0, -1.0);
    return vPoint.m_dZ;
  }

  // check if there's an object that the shadow should be cast on
  bool bChanged = false;
  double depth = PointUnderObjectsShadow(dRet, vObjectTest, rvNormal, bChanged);

  return depth;
}

//*****************************************************************************
double BSimulation::PointUnderObjectsShadow(double& rdCandidate, BVector& rvPoint, BVector& rvNormal, bool& rbChanged) {
  BObject *pCastObj;
  rbChanged = false;
  static BVector vUp(0, 0, -1);
  double dSmallestDepth = 999999.9;
  int nSelFace = -1;
  for(int o = 0; o < m_pCenterBlock->m_nObjects; ++o) {
    BObject *pObj = m_pCenterBlock->m_objectArray[o];
    if((rvPoint - pObj->m_vCenter).Length() < pObj->m_dRadius) {
      for(int nPart = 0; nPart < pObj->m_nCollDetParts; ++nPart) {
        for(int nFace = 0; nFace < pObj->m_pCollDetPart[nPart].m_nFaces; ++nFace) {
          if(pObj->m_pCollDetPart[nPart].m_pFace[nFace].m_vNormal.m_dZ < -0.01) { // Up facing surface
            double depth = pObj->m_pCollDetPart[nPart].m_pFace[nFace].m_vNormal.ScalarProduct(rvPoint - (pObj->m_pCollDetPart[nPart].m_pFace[nFace].m_vPoint[0] + pObj->m_vLocation));
            // double dProjDepth = (pObj->m_pCollDetPart[nPart].m_pFace[nFace].m_vNormal * depth).ScalarProduct(vUp);
            double dProjDepth = depth;
            if((dProjDepth > -0.5) && (fabs(dProjDepth) < fabs(dSmallestDepth))) {
              nSelFace = nFace;
              dSmallestDepth = dProjDepth;
              pCastObj = pObj;
              rvNormal = pObj->m_pCollDetPart[nPart].m_pFace[nFace].m_vNormal;
            }
          }
        }
      }
    }
  }
  
  if((fabs(dSmallestDepth) < fabs(rdCandidate)) && 
     !pCastObj->m_boundary.PointIsOutside(rvPoint.m_dX, rvPoint.m_dY)) {
    rbChanged = true;
    return -dSmallestDepth;
  } else {
    return rdCandidate;
  }
}


//*****************************************************************************
double BSimulation::Friction(BVector& rPoint) {
  return 0.5; // NOT READY Use correct ground friction!
}

//*****************************************************************************
double BSimulation::Friction(BBodyPoint& rPoint) {
  return rPoint.m_dFriction * 0.5; // NOT READY Use correct ground friction!
}




//*****************************************************************************
// CAMERA SETUP
//*****************************************************************************


extern double g_dRate;


//*****************************************************************************
void BSimulation::SetUpCamera(SDL_Rect *pRect) {
  // Always point towards the camera, always try to stay behind car
  static int nMode = 0;
  const static double cdTransitionSpeedMax = 0.025;
  static double dTransitionSpeedMax = cdTransitionSpeedMax;
  static double dTransitionSpeed = 0.0;
  BVector vUp(0, 0, -1);
  BVector vForward;

  BVector location = m_camera.m_vLocation;
  if(m_camera.m_locMode == BCamera::FIXED) {
    m_camera.m_vLocation = m_camera.m_vFixLocation;
    vForward = (m_vehicle.m_vLocation - m_camera.m_vLocation);
    if(dTransitionSpeedMax != cdTransitionSpeedMax) {
      dTransitionSpeed = 0.0;
      dTransitionSpeedMax = cdTransitionSpeedMax;
    }
  } else {
    if(m_camera.m_locMode == BCamera::FOLLOW) {
      double dBack = -m_vehicle.m_dVisualLength * 0.75;
      if(m_vehicle.m_dSpeed < 0.06) {

        // If stopped, place camera behind car
        if(nMode != 1) {
          dTransitionSpeed = 0.0;
        }
        nMode = 1;

        BVector vGoal = GetScene()->m_vGoal;
        if(!BGame::m_bRaceStarted) {
          vGoal = m_vehicle.m_vLocation + m_vehicle.m_orientation.m_vForward;
        } else if(m_vehicle.m_vLocation.m_dY > 6000.0) {
          dBack = -m_vehicle.m_dVisualLength;
          m_camera.m_dFollowHeight = -1.0;
          vGoal = m_vehicle.m_vLocation + BVector(0, -4.0, 0);
        } else {
          vGoal = m_vehicle.m_vLocation + BVector(0, 4.0, 0);
        }

        BVector vFromGoal = vGoal - m_vehicle.m_vLocation;
        vFromGoal.m_dZ /= 2.0;
        BVector vSpeed = vFromGoal;
        vSpeed.ToUnitLength();
        m_camera.m_vLocation = m_vehicle.m_vLocation + vSpeed * dBack;
        m_camera.m_vLocation.m_dZ = m_camera.m_vLocation.m_dZ + m_camera.m_dFollowHeight;
      } else {
        if(nMode != 2) {
          dTransitionSpeed = 0.0;
        }
        nMode = 2;
        // If moving, place camera behind car, on car's trail
        BVector vSpeed = m_vehicle.m_vector;
        vSpeed.ToUnitLength();
        m_camera.m_vLocation = m_vehicle.m_vLocation + vSpeed * dBack;
        m_camera.m_vLocation.m_dZ = m_camera.m_vLocation.m_dZ + m_camera.m_dFollowHeight;
      }
      vForward = (m_vehicle.m_vLocation + BVector(0, 0, -1) + m_vehicle.m_vector * 10.0 - m_camera.m_vLocation);
      if((dTransitionSpeedMax != cdTransitionSpeedMax) && (dTransitionSpeedMax != (cdTransitionSpeedMax * 2.0))) {
        dTransitionSpeed = 0.0;
        dTransitionSpeedMax = cdTransitionSpeedMax;
      }

      // Ensure camera is not too close to vehicle
      BVector vFromCarToCamera = m_camera.m_vLocation - m_vehicle.m_vLocation;
      if(vFromCarToCamera.Length() < m_vehicle.m_dVisualLength) {
        vFromCarToCamera.ToUnitLength();
        vFromCarToCamera = vFromCarToCamera * m_vehicle.m_dVisualLength;
        m_camera.m_vLocation = m_vehicle.m_vLocation + vFromCarToCamera;
      }
    } else if(m_camera.m_locMode == BCamera::OVERVIEW) {
      // Place camera high above
      if(m_camera.m_bInitLoc) {
        m_camera.m_bInitLoc = false;
        BVector vTowardsBack = BVector(-0.6, -0.1, 0);
        vTowardsBack.m_dZ = 0.0;
        vTowardsBack.ToUnitLength();
        m_camera.m_vOverview = vTowardsBack * 45.0 + BVector(0, 0, -30.0);
      }
      m_camera.m_vLocation = m_vehicle.m_vLocation + m_camera.m_vOverview;
      vForward = (m_vehicle.m_vLocation - m_camera.m_vLocation);
      if(dTransitionSpeedMax != cdTransitionSpeedMax) {
        dTransitionSpeed = 0.0;
        dTransitionSpeedMax = cdTransitionSpeedMax;
      }
    } else if(m_camera.m_locMode == BCamera::INCAR) {
      // Place camera over hood
      BVector vLoc = m_vehicle.m_vLocation + m_vehicle.m_orientation.m_vUp * m_vehicle.m_dVisualHeight * 0.4;
      m_camera.m_vLocation = vLoc;
      vUp      = m_vehicle.m_orientation.m_vUp;
      vForward = m_vehicle.m_orientation.m_vForward;
      dTransitionSpeedMax = 1.0;
      dTransitionSpeed = 1.0;
    } else if(m_camera.m_locMode == BCamera::ONSIDE) {
      // Place camera closely on left side of car
      BVector vLoc = m_vehicle.m_vLocation + BVector(-m_vehicle.m_dVisualWidth * 0.75, 0, 0);
      m_camera.m_vLocation = vLoc;
      vUp = m_vehicle.m_orientation.m_vUp;
      vForward = m_vehicle.m_orientation.m_vForward + vUp * 0.2;
      vForward.ToUnitLength();
      dTransitionSpeedMax = 1.0;
      dTransitionSpeed = 1.0;
    } else if(m_camera.m_locMode == BCamera::SIDEVIEW) {
      // Place camera on left side of car
      BVector vLoc = m_vehicle.m_vLocation - m_vehicle.m_orientation.m_vRight * 6.0 + m_vehicle.m_orientation.m_vUp * 2.0;
      m_camera.m_vLocation = vLoc;
      vUp = BVector(0, 0, -1);
      vForward = m_vehicle.m_vLocation - vLoc;
      vForward.ToUnitLength();
      dTransitionSpeedMax = 1.0;
      dTransitionSpeed = 1.0;
    }    
  }

  double dCalibXSpeed = dTransitionSpeed; // pow(dTransitionSpeed, 1 / (g_dRate / 60.0));

  m_camera.m_vLocation = location * (1.0 - dCalibXSpeed) + 
                         m_camera.m_vLocation * dCalibXSpeed;

  if(dTransitionSpeed < dTransitionSpeedMax) {
    dTransitionSpeed += 0.01;
  }

  if((m_camera.m_locMode != BCamera::INCAR) && 
     (m_camera.m_locMode != BCamera::ONSIDE)) {
    // Ensure that camera does not point too much away from the vehicle
    BVector vToCar = m_vehicle.m_vLocation - m_camera.m_vLocation;
    double dLen = vToCar.Length();
    vToCar.ToUnitLength();
    BVector vDiff = vForward - vToCar;
    if(vDiff.Length() > 0.1) {
      vDiff.ToUnitLength();
      vDiff = vDiff * 0.1;
      vForward = vToCar + vDiff;
      vForward.ToUnitLength();
    }
  }

  BOrientation orientation = m_camera.m_orientation;
  m_camera.m_orientation.m_vForward = vForward;
  m_camera.m_orientation.m_vUp = vUp;
  m_camera.m_orientation.m_vRight = m_camera.m_orientation.m_vForward.CrossProduct(m_camera.m_orientation.m_vUp);
  m_camera.m_orientation.Normalize();

  if(m_camera.m_locMode == BCamera::INCAR) {
    m_camera.m_orientation.m_vForward.m_dX = m_camera.m_orientation.m_vForward.m_dX * 0.9 + orientation.m_vForward.m_dX * (1.0 - 0.9);
    m_camera.m_orientation.m_vUp.m_dX      = m_camera.m_orientation.m_vUp.m_dX      * 0.9 + orientation.m_vUp.m_dX      * (1.0 - 0.9);
    m_camera.m_orientation.m_vRight.m_dX   = m_camera.m_orientation.m_vRight.m_dX   * 0.9 + orientation.m_vRight.m_dX   * (1.0 - 0.9);

    m_camera.m_orientation.m_vForward.m_dY = m_camera.m_orientation.m_vForward.m_dY * 0.9 + orientation.m_vForward.m_dY * (1.0 - 0.9);
    m_camera.m_orientation.m_vUp.m_dY      = m_camera.m_orientation.m_vUp.m_dY      * 0.9 + orientation.m_vUp.m_dY      * (1.0 - 0.9);
    m_camera.m_orientation.m_vRight.m_dY   = m_camera.m_orientation.m_vRight.m_dY   * 0.9 + orientation.m_vRight.m_dY   * (1.0 - 0.9);

    m_camera.m_orientation.m_vForward.m_dZ = m_camera.m_orientation.m_vForward.m_dZ * 0.2 + orientation.m_vForward.m_dZ * (1.0 - 0.2);
    m_camera.m_orientation.m_vUp.m_dZ      = m_camera.m_orientation.m_vUp.m_dZ      * 0.2 + orientation.m_vUp.m_dZ      * (1.0 - 0.2);
    m_camera.m_orientation.m_vRight.m_dZ   = m_camera.m_orientation.m_vRight.m_dZ   * 0.2 + orientation.m_vRight.m_dZ   * (1.0 - 0.2);
  } else {
    m_camera.m_orientation.m_vForward.m_dX = m_camera.m_orientation.m_vForward.m_dX * dTransitionSpeed * 1.0 + orientation.m_vForward.m_dX * (1.0 - dTransitionSpeed * 1.0);
    m_camera.m_orientation.m_vUp.m_dX      = m_camera.m_orientation.m_vUp.m_dX      * dTransitionSpeed * 1.0 + orientation.m_vUp.m_dX      * (1.0 - dTransitionSpeed * 1.0);
    m_camera.m_orientation.m_vRight.m_dX   = m_camera.m_orientation.m_vRight.m_dX   * dTransitionSpeed * 1.0 + orientation.m_vRight.m_dX   * (1.0 - dTransitionSpeed * 1.0);

    m_camera.m_orientation.m_vForward.m_dY = m_camera.m_orientation.m_vForward.m_dY * dTransitionSpeed * 1.0 + orientation.m_vForward.m_dY * (1.0 - dTransitionSpeed * 1.0);
    m_camera.m_orientation.m_vUp.m_dY      = m_camera.m_orientation.m_vUp.m_dY      * dTransitionSpeed * 1.0 + orientation.m_vUp.m_dY      * (1.0 - dTransitionSpeed * 1.0);
    m_camera.m_orientation.m_vRight.m_dY   = m_camera.m_orientation.m_vRight.m_dY   * dTransitionSpeed * 1.0 + orientation.m_vRight.m_dY   * (1.0 - dTransitionSpeed * 1.0);

    m_camera.m_orientation.m_vForward.m_dZ = m_camera.m_orientation.m_vForward.m_dZ * dTransitionSpeed * 0.2 + orientation.m_vForward.m_dZ * (1.0 - dTransitionSpeed * 0.2);
    m_camera.m_orientation.m_vUp.m_dZ      = m_camera.m_orientation.m_vUp.m_dZ      * dTransitionSpeed * 0.2 + orientation.m_vUp.m_dZ      * (1.0 - dTransitionSpeed * 0.2);
    m_camera.m_orientation.m_vRight.m_dZ   = m_camera.m_orientation.m_vRight.m_dZ   * dTransitionSpeed * 0.2 + orientation.m_vRight.m_dZ   * (1.0 - dTransitionSpeed * 0.2);
  }
  m_camera.m_orientation.Normalize();
}




//*****************************************************************************
// EFFECTS: TRAILS
//*****************************************************************************


//*****************************************************************************
void BSimulation::UpdateTrails() {
  bool bSkidding = false;
  for(int w = 0; w < m_vehicle.m_nWheels; ++w) {
    BWheel *pWheel = dynamic_cast<BWheel *>(m_vehicle.m_pWheel[w]);
    if(m_vehicle.m_dSpeed > 0.01) {
      if(pWheel->m_nGroundHits > (m_nPhysicsStepsBetweenRender / 2)) { // 5 = 15/3, 15 = number of updates between visualization
        bSkidding = true;
        int nPrev = pWheel->m_nTrailHead;

        // Insert new trail point
        if(pWheel->m_nTrailPoints < g_cnMaxTrailPoints) {
          ++pWheel->m_nTrailPoints;
        }
        pWheel->m_nTrailHead = (pWheel->m_nTrailHead + 1) % g_cnMaxTrailPoints;
        BVector vNormal, vNormalObj;
        double dTmp, dBaseDepth;
        double depth = PointUnderGround(pWheel->m_vLocSample, vNormal, dTmp, dBaseDepth, dTmp);
        bool bChanged = false;
        depth = PointUnderObjectsShadow(depth, pWheel->m_vLocSample, vNormalObj, bChanged);
        BVector vLoc;
        if(bChanged) {
          vLoc = pWheel->m_vLocSample + vNormalObj * depth;
        } else {
          vLoc = pWheel->m_vLocSample + vNormal * dBaseDepth;
        }
        BVector v1 = vLoc + pWheel->m_orientation.m_vRight * -(pWheel->m_dWidth / 2.0);
        BVector v2 = vLoc + pWheel->m_orientation.m_vRight * (pWheel->m_dWidth / 2.0);
        double d;
        d = PointUnderGround(v1, vNormal, dTmp, dBaseDepth, dTmp);
        d = PointUnderObjectsShadow(d, v1, vNormalObj, bChanged);
        if(bChanged) {
          v1 += vNormalObj * d;
        } else {
          v1 += vNormal * dBaseDepth;
        }
        d = PointUnderGround(v2, vNormal, dTmp, dBaseDepth, dTmp);
        d = PointUnderObjectsShadow(d, v2, vNormalObj, bChanged);
        if(bChanged) {
          v2 += vNormalObj * d;
        } else {
          v2 += vNormal * dBaseDepth;
        }
        pWheel->m_pTrailPoint[pWheel->m_nTrailHead].m_vLocation[0] = v1;
        pWheel->m_pTrailPoint[pWheel->m_nTrailHead].m_vLocation[1] = v2;
        pWheel->m_pTrailPoint[pWheel->m_nTrailHead].m_bStart = false;
        pWheel->m_pTrailPoint[pWheel->m_nTrailHead].m_bEnd   = false;
        pWheel->m_pTrailPoint[pWheel->m_nTrailHead].m_dStrength = double(pWheel->m_nGroundHits) / m_nPhysicsSteps;

        if(pWheel->m_nTrailPoints > 1) {
          if(pWheel->m_pTrailPoint[nPrev].m_bEnd) {
            pWheel->m_pTrailPoint[pWheel->m_nTrailHead].m_bStart = true;
          }
        }

      } else {
        if(pWheel->m_nTrailPoints) {
          // Mark end of this trail
          pWheel->m_pTrailPoint[pWheel->m_nTrailHead].m_bEnd = true;
        }
      }      
    } else {
      if(pWheel->m_nTrailPoints) {
        // Mark end of this trail
        pWheel->m_pTrailPoint[pWheel->m_nTrailHead].m_bEnd = true;
      }
    }
    pWheel->m_nGroundHits = 0;
  }

  // Update skid sound volume
  if(bSkidding) {
    if(m_vehicle.m_dSpeedKmh < 400.0) {
      SoundModule::SetSkidSoundVolume(int(m_vehicle.m_dSpeedKmh / 4.0));
    } else {
      SoundModule::SetSkidSoundVolume(100);
    }
  } else {
    SoundModule::SetSkidSoundVolume(0);
  }
}



//*****************************************************************************
void BSimulation::UpdateDustClouds() {

  double dTimeRatio = double(m_nPhysicsStepsBetweenRender) / 11.0;
  double dTimeRatio2 = 1.0;
  if(BGame::m_bSlowMotion) {
    dTimeRatio = 0.3;
    dTimeRatio2 = 0.15;
  }

  // Create new clouds
  static int nSkipper = 0;
  for(int w = 0; w < m_vehicle.m_nWheels; ++w) {
    BWheel *pWheel = dynamic_cast<BWheel *>(m_vehicle.m_pWheel[w]);
    if(m_vehicle.m_dSpeed > 0.1) {
      if(pWheel->m_bTouchesGround) {
        // Generate dust cloud at wheel position
        BVector vHitPoint = pWheel->m_vLocation + BVector(0, 0, pWheel->m_dDepth);
        CreateDustCloudAt(vHitPoint, 1, -1.0, BVector(0, 0, 0), pWheel->m_dRadius);
      }
    }
    // Prepare for next render frame
    pWheel->m_bTouchesGround = false;
  }

  // Update all clouds
  int i;
  for(i = 0; i < m_nDustClouds1; ++i) {
    m_dustClouds1[i].m_dAlpha -= 0.05 * dTimeRatio;
    if(m_dustClouds1[i].m_dAlpha < 0.0) {
      m_dustClouds1[i].m_dAlpha = 0.0;
    }
    m_dustClouds1[i].m_dSize += 0.05 * dTimeRatio;
    if(m_dustClouds1[i].m_dSize > 10.0) {
      m_dustClouds1[i].m_dSize = 10.0;
    }
    m_dustClouds1[i].m_vLocation += m_dustClouds1[i].m_vector * dTimeRatio2;
    double dTmp = m_dustClouds1[i].m_vector.m_dZ;
    m_dustClouds1[i].m_vector = m_dustClouds1[i].m_vector * 0.999 + RandomVector(0.03) * dTimeRatio2;
    m_dustClouds1[i].m_vector.m_dZ = dTmp;
  }
  for(i = 0; i < m_nDustClouds2; ++i) {
    m_dustClouds2[i].m_dAlpha *= 0.8;
    if(m_dustClouds2[i].m_dAlpha < 0.0) {
      m_dustClouds2[i].m_dAlpha = 0.0;
    }
    m_dustClouds2[i].m_dSize += 0.1 * dTimeRatio;
    if(m_dustClouds2[i].m_dSize > 10.0) {
      m_dustClouds2[i].m_dSize = 10.0;
    }
    m_dustClouds2[i].m_vLocation += m_dustClouds2[i].m_vector * dTimeRatio2;
    double dTmp = m_dustClouds2[i].m_vector.m_dZ;
    m_dustClouds2[i].m_vector = m_dustClouds2[i].m_vector * 0.999 + RandomVector(0.02) * dTimeRatio2;
    m_dustClouds2[i].m_vector.m_dZ = dTmp;
  }
  for(i = 0; i < m_nDustClouds3; ++i) {
    m_dustClouds3[i].m_dAlpha -= 0.3 * dTimeRatio;
    if(m_dustClouds3[i].m_dAlpha < 0.0) {
      m_dustClouds3[i].m_dAlpha = 0.0;
    }
    m_dustClouds3[i].m_dSize += 0.1 * dTimeRatio;
    if(m_dustClouds3[i].m_dSize > 10.0) {
      m_dustClouds3[i].m_dSize = 10.0;
    }
    m_dustClouds3[i].m_vLocation += m_dustClouds3[i].m_vector * dTimeRatio2;
    double dTmp = m_dustClouds3[i].m_vector.m_dZ;
    m_dustClouds3[i].m_vector = m_dustClouds3[i].m_vector * 0.999 + RandomVector(0.02) * dTimeRatio2;
    m_dustClouds3[i].m_vector.m_dZ = dTmp;
  }
}


//*****************************************************************************
void BSimulation::CreateDustCloudAt(BVector vHitPoint, int nSlot, double dInitialAlpha, BVector vInitial, double dOffset) {

  vHitPoint.m_dZ -= dOffset;

  double dR, dG, dB;
  BTerrain::GetColorForHeight(-vHitPoint.m_dZ, dR, dG, dB);
  dR = dR + (1.0 - dR) / 2.0 - 0.3;
  dG = dG + (1.0 - dG) / 2.0 - 0.3;
  dB = dB + (1.0 - dB) / 2.0 - 0.3;
  if(nSlot == 1) {
    m_nDustCloudsHead1 = (m_nDustCloudsHead1 + 1) % g_cnDustClouds1;
    m_dustClouds1[m_nDustCloudsHead1].m_dAlpha = 0.1 + 0.4 * (m_vehicle.m_dSpeed - 0.1) * 5.0;
    m_dustClouds1[m_nDustCloudsHead1].m_dSize = 0.1;
    m_dustClouds1[m_nDustCloudsHead1].m_vLocation = vHitPoint;
    m_dustClouds1[m_nDustCloudsHead1].m_vector = m_vehicle.m_vector * 0.95 + BVector(0, 0, -0.01);
    m_dustClouds1[m_nDustCloudsHead1].m_color.m_dX = dR;
    m_dustClouds1[m_nDustCloudsHead1].m_color.m_dY = dG;
    m_dustClouds1[m_nDustCloudsHead1].m_color.m_dZ = dB;
    ++m_nDustClouds1;
    if(m_nDustClouds1 > g_cnDustClouds1) {
      m_nDustClouds1 = g_cnDustClouds1;
    }
  } else if(nSlot == 2) {
    static int nSkipper = 0;
    if(!(++nSkipper % 3)) {
      m_nDustCloudsHead2 = (m_nDustCloudsHead2 + 1) % g_cnDustClouds2;
      m_dustClouds2[m_nDustCloudsHead2].m_dAlpha = 0.1 + 0.4 * (m_vehicle.m_dSpeed - 0.1) * 5.0;
      m_dustClouds2[m_nDustCloudsHead2].m_dSize = 0.1;
      m_dustClouds2[m_nDustCloudsHead2].m_vLocation = vHitPoint;
      m_dustClouds2[m_nDustCloudsHead2].m_vector = m_vehicle.m_vector * 0.95 + BVector(0, 0, -0.01);
      m_dustClouds2[m_nDustCloudsHead2].m_color.m_dX = dR;
      m_dustClouds2[m_nDustCloudsHead2].m_color.m_dY = dG;
      m_dustClouds2[m_nDustCloudsHead2].m_color.m_dZ = dB;
      ++m_nDustClouds2;
      if(m_nDustClouds2 > g_cnDustClouds2) {
        m_nDustClouds2 = g_cnDustClouds2;
      }
    }
  } else {
    m_nDustCloudsHead3 = (m_nDustCloudsHead3 + 1) % g_cnDustClouds3;
    m_dustClouds3[m_nDustCloudsHead3].m_dAlpha = dInitialAlpha;
    m_dustClouds3[m_nDustCloudsHead3].m_dSize = 0.1;
    m_dustClouds3[m_nDustCloudsHead3].m_vLocation = vHitPoint;
    m_dustClouds3[m_nDustCloudsHead3].m_vector = vInitial;
    m_dustClouds3[m_nDustCloudsHead3].m_color.m_dX = dR;
    m_dustClouds3[m_nDustCloudsHead3].m_color.m_dY = dG;
    m_dustClouds3[m_nDustCloudsHead3].m_color.m_dZ = dB;
    ++m_nDustClouds3;
    if(m_nDustClouds3 > g_cnDustClouds3) {
      m_nDustClouds3 = g_cnDustClouds3;
    }
  }
} 


//*****************************************************************************
void BSimulation::DrawShadowAndTrails() {
  static BVector vShadow[5];
  static int i;

  glDisable(GL_CULL_FACE);
  // Shadow
  if(m_camera.m_locMode != BCamera::INCAR) {
    OpenGLHelpers::SetColorFull(1, 1, 1, 1);

    vShadow[0] = m_vehicle.m_vLocation + m_vehicle.m_orientation.m_vRight * -(m_vehicle.m_dVisualWidth / 2.0) + m_vehicle.m_orientation.m_vForward *  (m_vehicle.m_dVisualLength / 2.0);
    vShadow[1] = m_vehicle.m_vLocation + m_vehicle.m_orientation.m_vRight * -(m_vehicle.m_dVisualWidth / 2.0) + m_vehicle.m_orientation.m_vForward * -(m_vehicle.m_dVisualLength / 2.0);
    vShadow[2] = m_vehicle.m_vLocation + m_vehicle.m_orientation.m_vRight *  (m_vehicle.m_dVisualWidth / 2.0) + m_vehicle.m_orientation.m_vForward *  (m_vehicle.m_dVisualLength / 2.0);
    vShadow[3] = m_vehicle.m_vLocation + m_vehicle.m_orientation.m_vRight *  (m_vehicle.m_dVisualWidth / 2.0) + m_vehicle.m_orientation.m_vForward * -(m_vehicle.m_dVisualLength / 2.0);
    for(i = 0; i < 4; ++i) {
      vShadow[i].m_dZ = vShadow[i].m_dZ - PointUnderGroundShadow(vShadow[i], vShadow[4]);
    }
    BVector vCenter = (vShadow[0] + vShadow[1] + vShadow[2] + vShadow[3]) * 0.25;
    vShadow[0] = (vShadow[0] - vCenter) * 1.3 + vCenter;
    vShadow[1] = (vShadow[1] - vCenter) * 1.3 + vCenter;
    vShadow[2] = (vShadow[2] - vCenter) * 1.3 + vCenter;
    vShadow[3] = (vShadow[3] - vCenter) * 1.3 + vCenter;

    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(vShadow[0].m_dX, vShadow[0].m_dY, vShadow[0].m_dZ);
    glTexCoord2f(0.0, 0.5);
    glVertex3f(vShadow[1].m_dX, vShadow[1].m_dY, vShadow[1].m_dZ);
    glTexCoord2f(0.5, 0.0);
    glVertex3f(vShadow[2].m_dX, vShadow[2].m_dY, vShadow[2].m_dZ);
    glTexCoord2f(0.5, 0.5);
    glVertex3f(vShadow[3].m_dX, vShadow[3].m_dY, vShadow[3].m_dZ);
    glEnd();  
  }

  // Multiplayer shadows
  if(BGame::m_bMultiplayOn) {
    OpenGLHelpers::SetColorFull(1, 1, 1, 1);
    for(int i = 0; i < BGame::m_nRemotePlayers; ++i) {
      if(BGame::m_remotePlayer[i].m_bSelf) {
        continue;
      }
      BRemotePlayer *pRemCar = &(BGame::m_remotePlayer[i]);

      double dTimePassed = 0;

      int dwNow = BGame::GetMultiplayClock();
      if(dwNow > pRemCar->m_clockLocationSent) {
        dTimePassed = double(dwNow - pRemCar->m_clockLocationSent);
      } else {
        dTimePassed = -double(pRemCar->m_clockLocationSent - dwNow);
      }

      BVector vLoc = pRemCar->m_vLocation + 
                     pRemCar->m_vVelocity * dTimePassed +
                     pRemCar->m_vVelo1stDeriv * dTimePassed; // +
                     //pRemCar->m_vVelo2ndDeriv * dTimePassed;

      vShadow[0] = vLoc + pRemCar->m_orientation.m_vRight * -(pRemCar->m_dWidth) + pRemCar->m_orientation.m_vForward *  (pRemCar->m_dLen);
      vShadow[1] = vLoc + pRemCar->m_orientation.m_vRight * -(pRemCar->m_dWidth) + pRemCar->m_orientation.m_vForward * -(pRemCar->m_dLen);
      vShadow[2] = vLoc + pRemCar->m_orientation.m_vRight *  (pRemCar->m_dWidth) + pRemCar->m_orientation.m_vForward *  (pRemCar->m_dLen);
      vShadow[3] = vLoc + pRemCar->m_orientation.m_vRight *  (pRemCar->m_dWidth) + pRemCar->m_orientation.m_vForward * -(pRemCar->m_dLen);
      for(i = 0; i < 4; ++i) {
        vShadow[i].m_dZ = vShadow[i].m_dZ - PointUnderGroundShadow(vShadow[i], vShadow[4]);
      }
      BVector vCenter = (vShadow[0] + vShadow[1] + vShadow[2] + vShadow[3]) * 0.25;
      vShadow[0] = (vShadow[0] - vCenter) * 1.3 + vCenter;
      vShadow[1] = (vShadow[1] - vCenter) * 1.3 + vCenter;
      vShadow[2] = (vShadow[2] - vCenter) * 1.3 + vCenter;
      vShadow[3] = (vShadow[3] - vCenter) * 1.3 + vCenter;

      glBegin(GL_TRIANGLE_STRIP);
      glTexCoord2f(0.0, 0.0);
      glVertex3f(vShadow[0].m_dX, vShadow[0].m_dY, vShadow[0].m_dZ);
      glTexCoord2f(0.0, 0.5);
      glVertex3f(vShadow[1].m_dX, vShadow[1].m_dY, vShadow[1].m_dZ);
      glTexCoord2f(0.5, 0.0);
      glVertex3f(vShadow[2].m_dX, vShadow[2].m_dY, vShadow[2].m_dZ);
      glTexCoord2f(0.5, 0.5);
      glVertex3f(vShadow[3].m_dX, vShadow[3].m_dY, vShadow[3].m_dZ);
      glEnd();  
    }
  }

  // Trails
  DrawTrails();
  glEnable(GL_CULL_FACE);
}

//*****************************************************************************
void BSimulation::DrawTrails() {
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  static BVector vTrail[4];
  // Loop through all wheels and draw their trails
  bool bBeginDone = false;
  for(int w = 0; w < m_vehicle.m_nWheels; ++w) {
    BWheel *pWheel = dynamic_cast<BWheel *>(m_vehicle.m_pWheel[w]);
    for(int i = pWheel->m_nTrailPoints - 1; i >= 0; --i) {
      int nTrailPoint = ((pWheel->m_nTrailHead + g_cnMaxTrailPoints) - i) % g_cnMaxTrailPoints;

      if(pWheel->m_pTrailPoint[nTrailPoint].m_bStart) {
        OpenGLHelpers::SetColorFull(1, 1, 1, 0);
      } else if(pWheel->m_pTrailPoint[nTrailPoint].m_bEnd) {
        OpenGLHelpers::SetColorFull(1, 1, 1, 0);
      } else {
        double dColor = pWheel->m_pTrailPoint[nTrailPoint].m_dStrength * 0.25; // was 0.75
        if(dColor > 1.0) {
          dColor = 1.0;
        }
        OpenGLHelpers::SetColorFull(1, 1, 1, dColor);
      }

      if(!bBeginDone) {
        glBegin(GL_TRIANGLE_STRIP);
        bBeginDone = true;
      }

      // Draw rectangle from previous to current
      vTrail[0] = pWheel->m_pTrailPoint[nTrailPoint].m_vLocation[1];
      glTexCoord2f(6.0/16.0, 11.0/16.0);
      glVertex3f(vTrail[0].m_dX, vTrail[0].m_dY, vTrail[0].m_dZ);
      vTrail[1] = pWheel->m_pTrailPoint[nTrailPoint].m_vLocation[0];
      glTexCoord2f(14.0/16.0, 11.0/16.0);
      glVertex3f(vTrail[1].m_dX, vTrail[1].m_dY, vTrail[1].m_dZ);

      if(pWheel->m_pTrailPoint[nTrailPoint].m_bEnd || (i == 0)) {
        glEnd();
        bBeginDone = false;
      }

    }
  }
}




//*****************************************************************************
void BSimulation::DrawSmokeTrails() {
  // OpenGLHelpers::SwitchToTexture(0);
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glEnable(GL_COLOR_MATERIAL);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

  // Draw the smoke trails
  int i;

  for(i = 0; i < m_vehicle.m_nTrailpoints; ++i) {

    glBegin(GL_TRIANGLE_STRIP);
    int nLoop = 0;
    int nEntry = m_vehicle.m_nTrailpointHead;
    while(nLoop < m_vehicle.m_nTrailpointEntries) {
      double dAlphaFactor = double(nLoop) / 100.0;
      double dLen = sqrt(double(100 - nLoop) / 100.0);

      if(m_vehicle.m_trailpointEntry[i][nEntry].m_dAlpha < 0.0) {
        m_vehicle.m_trailpointEntry[i][nEntry].m_dAlpha = 0.0;
      }

      glColor4d(1, 1, 1, 0.0);
      BVector vLoc = m_vehicle.m_trailpointEntry[i][nEntry].m_vLocation - m_vehicle.m_trailpointEntry[i][nEntry].m_vRight * dLen;
      glVertex3f(vLoc.m_dX, vLoc.m_dY, vLoc.m_dZ);
      glColor4d(1, 1, 1, m_vehicle.m_trailpointEntry[i][nEntry].m_dAlpha * dAlphaFactor);      
      vLoc = m_vehicle.m_trailpointEntry[i][nEntry].m_vLocation;
      glVertex3f(vLoc.m_dX, vLoc.m_dY, vLoc.m_dZ);

      ++nLoop;
      nEntry = (nEntry + 1) % 100;
    }
    glEnd();

    glBegin(GL_TRIANGLE_STRIP);
    nLoop = 0;
    nEntry = m_vehicle.m_nTrailpointHead;
    while(nLoop < m_vehicle.m_nTrailpointEntries) {
      double dAlphaFactor = double(nLoop) / 100.0;
      double dLen = sqrt(double(100 - nLoop) / 100.0);
      
      glColor4d(1, 1, 1, m_vehicle.m_trailpointEntry[i][nEntry].m_dAlpha * dAlphaFactor);
      BVector vLoc = m_vehicle.m_trailpointEntry[i][nEntry].m_vLocation;
      glVertex3f(vLoc.m_dX, vLoc.m_dY, vLoc.m_dZ);
      glColor4d(1, 1, 1, 0.0);
      vLoc = m_vehicle.m_trailpointEntry[i][nEntry].m_vLocation + m_vehicle.m_trailpointEntry[i][nEntry].m_vRight * dLen;
      glVertex3f(vLoc.m_dX, vLoc.m_dY, vLoc.m_dZ);
      
      ++nLoop;
      nEntry = (nEntry + 1) % 100;
    }
    glEnd();

  }

  glDisable(GL_BLEND);
  glDisable(GL_COLOR_MATERIAL);

  for(i = 0; i < m_vehicle.m_nTrailpoints; ++i) {
    for(int nLoop = 0; nLoop < 100; ++nLoop) {
      m_vehicle.m_trailpointEntry[i][nLoop].m_dAlpha -= 0.01;
    }
  }
}




//*****************************************************************************
void BSimulation::DrawDustClouds() {
  if((m_nDustClouds1 > 0) || 
     (m_nDustClouds2 > 0) || 
     (m_nDustClouds3 > 0)) {

    GLfloat fLight1AmbientG[ 4];
    if(BGame::m_bNight) {
      fLight1AmbientG[0] = 0.25f;
      fLight1AmbientG[1] = 0.25f;
      fLight1AmbientG[2] = 0.25f;
      fLight1AmbientG[3] = 0.75f;
    } else {
      fLight1AmbientG[0] = 0.75f;
      fLight1AmbientG[1] = 0.75f;
      fLight1AmbientG[2] = 0.75f;
      fLight1AmbientG[3] = 0.75f;
    }
    glLightfv( GL_LIGHT0, GL_AMBIENT,  fLight1AmbientG);

    glNormal3f(0, 0, -1);
    OpenGLHelpers::SetColorFull(1, 1, 1, 1);    
    OpenGLHelpers::SwitchToTexture(0);
    BTextures::Use(BTextures::DUSTCLOUD);
    //glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    BVector v1, v2, v3, v4;

    BVector vToUp;
    BVector vToCamera;
    BVector vToRight;

    // First Dust
    int i;
    for(i = 0; i < m_nDustClouds1; ++i) {
      if(i % 40) {
        // Determine orientation
        vToUp.Set(0, 0, -1);
        vToCamera = m_camera.m_vLocation - m_dustClouds1[i].m_vLocation;
        vToCamera.ToUnitLength();
        vToRight = vToCamera.CrossProduct(vToUp);
        vToRight.ToUnitLength();
        vToUp = vToRight.CrossProduct(vToCamera);
        vToUp.ToUnitLength();
      }

      // Draw rectangle to represent the dust cloud

      if(m_dustClouds1[i].m_dAlpha > 0.01) {
        OpenGLHelpers::SetColorFull(m_dustClouds1[i].m_color.m_dX, 
                                    m_dustClouds1[i].m_color.m_dY, 
                                    m_dustClouds1[i].m_color.m_dZ, 
                                    m_dustClouds1[i].m_color.m_dX * m_dustClouds1[i].m_dAlpha / 2.0);
        v1 = m_dustClouds1[i].m_vLocation - vToRight * m_dustClouds1[i].m_dSize - vToUp * m_dustClouds1[i].m_dSize;
        v2 = m_dustClouds1[i].m_vLocation - vToRight * m_dustClouds1[i].m_dSize + vToUp * m_dustClouds1[i].m_dSize;
        v3 = m_dustClouds1[i].m_vLocation + vToRight * m_dustClouds1[i].m_dSize + vToUp * m_dustClouds1[i].m_dSize;
        v4 = m_dustClouds1[i].m_vLocation + vToRight * m_dustClouds1[i].m_dSize - vToUp * m_dustClouds1[i].m_dSize;
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex3f(v1.m_dX, v1.m_dY, v1.m_dZ);
        glTexCoord2f(0, 1);
        glVertex3f(v2.m_dX, v2.m_dY, v2.m_dZ);
        glTexCoord2f(1, 1);
        glVertex3f(v3.m_dX, v3.m_dY, v3.m_dZ);
        glTexCoord2f(1, 0);
        glVertex3f(v4.m_dX, v4.m_dY, v4.m_dZ);
        glEnd();
      }
    }

    for(i = 0; i < m_nDustClouds2; ++i) {
      if(i % 40) {
        // Determine orientation
        vToUp.Set(0, 0, -1);
        vToCamera = m_camera.m_vLocation - m_dustClouds2[i].m_vLocation;
        vToCamera.ToUnitLength();
        vToRight = vToCamera.CrossProduct(vToUp);
        vToRight.ToUnitLength();
        vToUp = vToRight.CrossProduct(vToCamera);
        vToUp.ToUnitLength();
      }

      // Draw rectangle to represent the dust cloud

      if(m_dustClouds2[i].m_dAlpha > 0.01) {
        OpenGLHelpers::SetColorFull(m_dustClouds2[i].m_color.m_dX, 
                                    m_dustClouds2[i].m_color.m_dY, 
                                    m_dustClouds2[i].m_color.m_dZ, 
                                    m_dustClouds2[i].m_color.m_dX * m_dustClouds2[i].m_dAlpha / 2.0);
        v1 = m_dustClouds2[i].m_vLocation - vToRight * m_dustClouds2[i].m_dSize - vToUp * m_dustClouds2[i].m_dSize;
        v2 = m_dustClouds2[i].m_vLocation - vToRight * m_dustClouds2[i].m_dSize + vToUp * m_dustClouds2[i].m_dSize;
        v3 = m_dustClouds2[i].m_vLocation + vToRight * m_dustClouds2[i].m_dSize + vToUp * m_dustClouds2[i].m_dSize;
        v4 = m_dustClouds2[i].m_vLocation + vToRight * m_dustClouds2[i].m_dSize - vToUp * m_dustClouds2[i].m_dSize;
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex3f(v1.m_dX, v1.m_dY, v1.m_dZ);
        glTexCoord2f(0, 1);
        glVertex3f(v2.m_dX, v2.m_dY, v2.m_dZ);
        glTexCoord2f(1, 1);
        glVertex3f(v3.m_dX, v3.m_dY, v3.m_dZ);
        glTexCoord2f(1, 0);
        glVertex3f(v4.m_dX, v4.m_dY, v4.m_dZ);
        glEnd();
      }
    }

    for(i = 0; i < m_nDustClouds3; ++i) {
      if(i % 10) {
        // Determine orientation
        vToUp.Set(0, 0, -1);
        vToCamera = m_camera.m_vLocation - m_dustClouds3[i].m_vLocation;
        vToCamera.ToUnitLength();
        vToRight = vToCamera.CrossProduct(vToUp);
        vToRight.ToUnitLength();
        vToUp = vToRight.CrossProduct(vToCamera);
        vToUp.ToUnitLength();
      }

      // Draw rectangle to represent the dust cloud

      if(m_dustClouds3[i].m_dAlpha > 0.01) {
        OpenGLHelpers::SetColorFull(m_dustClouds3[i].m_color.m_dX, 
                                    m_dustClouds3[i].m_color.m_dY, 
                                    m_dustClouds3[i].m_color.m_dZ, 
                                    m_dustClouds3[i].m_color.m_dX * m_dustClouds3[i].m_dAlpha / 2.0);
        v1 = m_dustClouds3[i].m_vLocation - vToRight * m_dustClouds3[i].m_dSize - vToUp * m_dustClouds3[i].m_dSize;
        v2 = m_dustClouds3[i].m_vLocation - vToRight * m_dustClouds3[i].m_dSize + vToUp * m_dustClouds3[i].m_dSize;
        v3 = m_dustClouds3[i].m_vLocation + vToRight * m_dustClouds3[i].m_dSize + vToUp * m_dustClouds3[i].m_dSize;
        v4 = m_dustClouds3[i].m_vLocation + vToRight * m_dustClouds3[i].m_dSize - vToUp * m_dustClouds3[i].m_dSize;
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex3f(v1.m_dX, v1.m_dY, v1.m_dZ);
        glTexCoord2f(0, 1);
        glVertex3f(v2.m_dX, v2.m_dY, v2.m_dZ);
        glTexCoord2f(1, 1);
        glVertex3f(v3.m_dX, v3.m_dY, v3.m_dZ);
        glTexCoord2f(1, 0);
        glVertex3f(v4.m_dX, v4.m_dY, v4.m_dZ);
        glEnd();
      }
    }

    glDepthMask(GL_TRUE);
    //glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

    fLight1AmbientG[0] = 0.2f;
    fLight1AmbientG[1] = 0.2f;
    fLight1AmbientG[2] = 0.2f;
    fLight1AmbientG[3] = 0.2f;
    glLightfv( GL_LIGHT0, GL_AMBIENT,  fLight1AmbientG);
  }
}






//*****************************************************************************
void BSimulation::AddTrackingTarget(string sId, BVector vLoc, double dRed, double dGreen, double dBlue) {
  // Add a tracking target to the tracking list
  // targets are drawn in the multi-purpose compass panel
  BTrackingTarget *pNew = new BTrackingTarget;
  pNew->m_sId = sId;
  pNew->m_vLoc = vLoc;
  pNew->m_dRed = dRed;
  pNew->m_dGreen = dGreen;
  pNew->m_dBlue = dBlue;
  pNew->m_pNext = 0;

  if(!m_targets) {
    // add first
    m_targets = pNew;
  } else {
    BTrackingTarget *p = m_targets;
    while(p->m_pNext) {
      p = p->m_pNext;
    }
    p->m_pNext = pNew;
  }
}


//*****************************************************************************
void BSimulation::RemoveTrackingTarget(string sId) {
  BTrackingTarget *pPrev = m_targets;
  BTrackingTarget *p = m_targets;
  while(p) {
    if(p->m_sId.compare(sId) == 0) {
      if(p == m_targets) {
        // delete first
        m_targets = p->m_pNext;
      } else {
        // delete not first
        pPrev->m_pNext = p->m_pNext;
      }
      delete p;
      return;
    }

    // Proceed to next
    pPrev = p;
    p = p->m_pNext;
  }
}












//*****************************************************************************
// RECORDING
//*****************************************************************************

void BSimulation::StartRecording() {
  m_fp = fopen("sound.txt", "w");
}

void BSimulation::StopRecording() {
  fclose(m_fp);
  m_fp = 0;
}




















// Old wheel simulation with basepoints 1 and 2.

#if 0


//*****************************************************************************
static double ViscosityDamped(double dSusp, double dForce) {
  // If force is small, allow it all
  // If force is strong, damp it to absord the shock
  double dForceFactor;
  if(fabs(dSusp) > 0.3) {
    dForceFactor = 0.0;
  } else {
    // dForceFactor = fabs(dSusp) / 0.4;
    dForceFactor = fabs(dSusp) / 0.3;
    dForceFactor = 1.0 - dForceFactor;
  }
  
  return dSusp + (dForce * dForceFactor);
}


//*****************************************************************************
void BSimulation::ApplyWheelForces(BWheel& rWheel) {
  rWheel.m_bInGround = true;
  // BVector vGroundNormal;
  // double dGroundFriction, dBaseDepth;
  // (void) PointUnderGround(rWheel.m_vLocSample, vGroundNormal, dGroundFriction, dBaseDepth);
  BVector vSpeed = rWheel.m_pBodyPoint[rWheel.m_nBodyPoint].m_vector;
  vSpeed.ToUnitLength();
  double dThermoLoss = fabs(vSpeed.ScalarProduct(rWheel.m_vGroundNormal));
  dThermoLoss *= dThermoLoss;
  if(rWheel.m_dDepth > 0.015) {
    rWheel.m_dDepth -= 0.015;
    // First apply ground force as with any point touching the ground
    // 1) calculate f = Scalar(wheel.up, ground.normal)
    double dSuspFactor = fabs(rWheel.m_orientation.m_vUp.ScalarProduct(rWheel.m_vGroundNormal)); 
    // dSuspFactor = dSuspFactor * dSuspFactor * dSuspFactor;

    // 2) Modify wheel.susp with f * wheel.depth
    if(rWheel.m_dBaseDepth > 0.0) {
      rWheel.m_dSuspension = ViscosityDamped(rWheel.m_dSuspension, dSuspFactor * rWheel.m_dBaseDepth);
    }

    // Apply thermodynamic loss

    /*
    m_vehicle.m_pBodyPoint[rWheel.m_nBasePoint1].m_vector = m_vehicle.m_pBodyPoint[rWheel.m_nBasePoint1].m_vector *
                                                        (1.0 - 0.1 * dThermoLoss);
    m_vehicle.m_pBodyPoint[rWheel.m_nBasePoint2].m_vector = m_vehicle.m_pBodyPoint[rWheel.m_nBasePoint2].m_vector *
                                                        (1.0 - 0.1 * dThermoLoss);
    */

    m_vehicle.m_pBodyPoint[rWheel.m_nBodyPoint].m_vector = m_vehicle.m_pBodyPoint[rWheel.m_nBodyPoint].m_vector *
                                                       (1.0 - 0.2 * dThermoLoss);

    // 3) add ((1.0 - f) * wheel.depth) * ground.normal to base points 1 and 2
    if(rWheel.m_vGroundNormal.m_dZ < -0.85) {
      rWheel.m_vGroundNormal.Set(0, 0, -1);
    }

    m_vehicle.m_pBodyPoint[rWheel.m_nBodyPoint].m_vector += rWheel.m_vGroundNormal *
                                                     ((1.0 - dSuspFactor) * rWheel.m_dDepth * 0.05);

    // 4) Modify suspPoint's vector with wheel.up * 
    //    (wheel.suspension_wants_to_be_at_rest)
    //    (maybe all 3 points, suspPoint, basepoint1 and basepoint2)
    BVector vToRight;
    vToRight = BVector(0, 0, -1).CrossProduct(rWheel.m_orientation.m_vForward);
    vToRight.ToUnitLength();
    double dRightFactor = rWheel.m_orientation.m_vUp.ScalarProduct(vToRight);
    vToRight = vToRight * dRightFactor;
    BVector vWheelSortaUp = rWheel.m_orientation.m_vUp - vToRight;
    vWheelSortaUp.ToUnitLength();
    //BVector vSuspForce = rWheel.m_orientation.m_vUp * 
    //                    (rWheel.m_dSuspension * dSuspFactor * 0.02);
    BVector vSuspForce = vWheelSortaUp * 
                        (rWheel.m_dSuspension * dSuspFactor * 0.02);
    m_vehicle.m_pBodyPoint[rWheel.m_nBodyPoint].m_vector  += vSuspForce * 2.0;
    //m_vehicle.m_pBodyPoint[rWheel.m_nBasePoint1].m_vector += vSuspForce * 0.25;
    //m_vehicle.m_pBodyPoint[rWheel.m_nBasePoint2].m_vector += vSuspForce * 0.25;
    rWheel.m_dDepth += 0.015;
  }

  // Then frictions (brake and wheel orientation based)
  double dFrictionFactor = 0.001;
  if(m_vehicle.m_bBreaking) {
    dFrictionFactor = g_cdBrakesFriction * rWheel.m_dFriction * rWheel.m_dGroundFriction;
    dFrictionFactor = BreakProfile(dFrictionFactor);
  }

  // Break wheel if too strong force
  /*
  if(((1.0 - rWheel.m_dTTT) * rWheel.m_dFriction * dGroundFriction > 0.035) && 
    (rWheel.m_pBodyPoint[rWheel.m_nBodyPoint].m_vector.Length() > 0.2)) {
    rWheel.m_bBroken = true;
    rWheel.m_dRadius *= 0.85;
    rWheel.m_dFriction -= 0.1;
  }
  */

  dFrictionFactor = max(dFrictionFactor, (1.0 - rWheel.m_dTTT) * rWheel.m_dFriction * rWheel.m_dGroundFriction);

  // Apply ground oriented friction
  double dLossFactor = 1.0 - (dFrictionFactor * 0.2);

  //rWheel.m_pBodyPoint[rWheel.m_nBasePoint1].m_vector = (rWheel.m_pBodyPoint[rWheel.m_nBasePoint1].m_vector) * 
  //                                                   dLossFactor;
  //rWheel.m_pBodyPoint[rWheel.m_nBasePoint2].m_vector = (rWheel.m_pBodyPoint[rWheel.m_nBasePoint2].m_vector) * 
  //                                                   dLossFactor;
  rWheel.m_pBodyPoint[rWheel.m_nBodyPoint].m_vector = (rWheel.m_pBodyPoint[rWheel.m_nBodyPoint].m_vector) * 
                                                    dLossFactor;

  // Then acceleration
  if(m_vehicle.m_bAccelerating || m_vehicle.m_bReversing) { // 4WD

    //rWheel.m_pBodyPoint[rWheel.m_nBasePoint1].m_vector += rWheel.m_orientation.m_vForward * 
    //                                                   rWheel.m_dGroundFriction *
    //                                                   rWheel.m_dFriction *
    //                                                   m_dAccelerationFactor *
    //                                                   (m_vehicle.m_bAccelerating ? m_vehicle.m_dAccelerationFactor : -m_vehicle.m_dReversingFactor);
    //rWheel.m_pBodyPoint[rWheel.m_nBasePoint2].m_vector += rWheel.m_orientation.m_vForward * 
    //                                                   rWheel.m_dGroundFriction *
    //                                                   rWheel.m_dFriction *
    //                                                   m_dAccelerationFactor *
    //                                                   (m_vehicle.m_bAccelerating ? m_vehicle.m_dAccelerationFactor : -m_vehicle.m_dReversingFactor);

    rWheel.m_pBodyPoint[rWheel.m_nBodyPoint].m_vector += rWheel.m_orientation.m_vForward * 
                                                      rWheel.m_dGroundFriction *
                                                      rWheel.m_dFriction *
                                                      m_dAccelerationFactor * 4.0 *
                                                      (m_vehicle.m_bAccelerating ? m_vehicle.m_dAccelerationFactor : -m_vehicle.m_dReversingFactor);
  }

  // Apply turning
  if(!m_vehicle.m_bBreaking ) { // && !rWheel.m_bRear
    BVector vWheelVector = rWheel.m_pBodyPoint[rWheel.m_nBodyPoint].m_vector;
    BVector vIdeal = rWheel.m_orientation.m_vForward;
    if((vIdeal.ScalarProduct(vWheelVector) < 0.0) && 
       !m_vehicle.m_bAccelerating) {
      vIdeal = vIdeal * -1.0;
    }
    vIdeal.ToUnitLength();

    BVector vReality = vWheelVector;
    double  dRealLen = vReality.Length();
    vReality.ToUnitLength();

    double dEffect = rWheel.m_dTTT * rWheel.m_dFriction * rWheel.m_dGroundFriction * 0.05;

    rWheel.m_pBodyPoint[rWheel.m_nBodyPoint].m_vector = vIdeal * dRealLen * dEffect +
                                                     rWheel.m_pBodyPoint[rWheel.m_nBodyPoint].m_vector *
                                                     (1.0 - dEffect);  
  }

  // Record hit point for trails
  if(dFrictionFactor > 0.001) {
    ++rWheel.m_nGroundHits;
  }
}

  if(rWheel.m_dDepth > 0.015) {
    rWheel.m_dDepth -= 0.015;
    // First apply ground force as with any point touching the ground
    // 1) calculate f = Scalar(wheel.up, ground.normal)
    double dSuspFactor = fabs(rWheel.m_orientation.m_vUp.ScalarProduct(rWheel.m_vGroundNormal)); 
    // dSuspFactor = dSuspFactor * dSuspFactor * dSuspFactor;

    // 2) Modify wheel.susp with f * wheel.depth
    if(rWheel.m_dBaseDepth > 0.0) {
      rWheel.m_dSuspension = ViscosityDamped(rWheel.m_dSuspension, dSuspFactor * rWheel.m_dBaseDepth);
    }

    // Apply thermodynamic loss

    pBodyPoint->m_vector = pBodyPoint->m_vector * (1.0 - 0.2 * dThermoLoss);

    // 3) add ((1.0 - f) * wheel.depth) * ground.normal to base points 1 and 2
    if(rWheel.m_vGroundNormal.m_dZ < -0.85) {
      rWheel.m_vGroundNormal.Set(0, 0, -1);
    }

    pBodyPoint->m_vector += rWheel.m_vGroundNormal * ((1.0 - dSuspFactor) * rWheel.m_dDepth * 0.05);

    // 4) Modify suspPoint's vector with wheel.up * 
    //    (wheel.suspension_wants_to_be_at_rest)
    //    (maybe all 3 points, suspPoint, basepoint1 and basepoint2)
    BVector vToRight;
    vToRight = BVector(0, 0, -1).CrossProduct(rWheel.m_orientation.m_vForward);
    vToRight.ToUnitLength();
    double dRightFactor = rWheel.m_orientation.m_vUp.ScalarProduct(vToRight);
    vToRight = vToRight * dRightFactor;
    BVector vWheelSortaUp = rWheel.m_orientation.m_vUp - vToRight;
    vWheelSortaUp.ToUnitLength();
    BVector vSuspForce = vWheelSortaUp * (rWheel.m_dSuspension * dSuspFactor * 0.02);
    pBodyPoint->m_vector += vSuspForce * 2.0;
    rWheel.m_dDepth += 0.015;
  }


// Old wheel simulation without slide control (in comments).

//*****************************************************************************
void BSimulation::ApplyWheelForces(BWheel& rWheel) {
  rWheel.m_bInGround = true;

  BBodyPoint* pBodyPoint = &(rWheel.m_pBodyPoint[rWheel.m_nBodyPoint]);

  BVector vSpeed = pBodyPoint->m_vector;
  vSpeed.ToUnitLength();
  double dThermoLoss = fabs(vSpeed.ScalarProduct(rWheel.m_vGroundNormal));
  dThermoLoss *= (1.0 - fabs(vSpeed.ScalarProduct(rWheel.m_orientation.m_vForward)));

  if(rWheel.m_dDepth > 0.05) {
    rWheel.m_dDepth -= 0.05;

    // Smooth the depth function so that we don't jump so much over the surface
    // double dSmoothDepth = rWheel.m_dDepth * 10.0;
    // dSmoothDepth = dSmoothDepth - (( 2 * dSmoothDepth) / (pow(2, pow(dSmoothDepth + 1, 2))));
    // dSmoothDepth /= 10.0;

    double dPerpendicularity = fabs(rWheel.m_orientation.m_vUp.ScalarProduct(rWheel.m_vGroundNormal)); 
    double dPrevSuspension = rWheel.m_dSuspension;
    double dNewSuspension  = dPrevSuspension + dPerpendicularity * rWheel.m_dDepth;

    double dSuspGets = ((dPrevSuspension + dNewSuspension) * 0.5) / rWheel.m_dMaxSuspThrow;

    if(dSuspGets > 1.0) {
      dSuspGets = 1.0;
    }
    dSuspGets = 1.0 - pow(dSuspGets, 1.0 / rWheel.m_dSuspStiffness);

    // Viscosity damping
    double dViscDamp = rWheel.m_dDepth / rWheel.m_dMaxSuspThrow;
    if(dViscDamp > 1.0) {
      dViscDamp = 1.0;
    }

    dSuspGets *= (1.0 - dViscDamp);

    rWheel.m_dSuspension += dSuspGets * rWheel.m_dDepth;

	  double dBodyPointGets = 1.0 - dSuspGets;


    // 4) Modify suspPoint's vector with wheel.up * 
    //    (wheel.suspension_wants_to_be_at_rest)
    //    (maybe all 3 points, suspPoint, basepoint1 and basepoint2)
    //BVector vToRight;
    //vToRight = BVector(0, 0, -1).CrossProduct(rWheel.m_orientation.m_vForward);
    //vToRight.ToUnitLength();
    //double dRightFactor = rWheel.m_orientation.m_vUp.ScalarProduct(vToRight);
    //vToRight = vToRight * dRightFactor;
    //BVector vWheelSortaUp = rWheel.m_orientation.m_vUp - vToRight;
    //vWheelSortaUp.ToUnitLength();

    double dSlide = fabs(rWheel.m_vGroundNormal.ScalarProduct(BVector(0, 0, -1)));
    dSlide *= 1.3;
    if(dSlide > 1.0) {
      dSlide = 1.0;
    }
    BVector vRightOnGround = rWheel.m_orientation.m_vForward.CrossProduct(BVector(0, 0, -1));
    vRightOnGround.ToUnitLength();
    double dCorr = rWheel.m_vGroundNormal.ScalarProduct(vRightOnGround);
    BVector vCorr = rWheel.m_vGroundNormal + vRightOnGround * -(dCorr * 0.85 * dSlide);

    //pBodyPoint->m_vector += vWheelSortaUp * (dBodyPointGets * rWheel.m_dDepth * GroundHardnessAt(rWheel.m_vLocSample) * 0.5);
    //          pBodyPoint->m_vector += rWheel.m_vGroundNormal * 
    pBodyPoint->m_vector += vCorr *
                            (dBodyPointGets * rWheel.m_dDepth * GroundHardnessAt(rWheel.m_vLocSample) * 0.5);
    //pBodyPoint->m_vector += (vRight * dSideways +
    //                         rWheel.m_vGroundNormal) * 
    //                         (dBodyPointGets * dSmoothDepth * GroundHardnessAt(rWheel.m_vLocSample) * 0.5);

    // Apply thermodynamic loss
    pBodyPoint->m_vector = pBodyPoint->m_vector * (1.0 - 0.3 * dBodyPointGets * dThermoLoss);
    
    // Apply ground oriented thermoloss (to stop the ridiculous bounching)
    double dOnNormal = pBodyPoint->m_vector.ScalarProduct(rWheel.m_vGroundNormal);
    // pBodyPoint->m_vector = pBodyPoint->m_vector + rWheel.m_vGroundNormal * -dOnNormal * 0.25; // was * 0.15
    pBodyPoint->m_vector = pBodyPoint->m_vector + rWheel.m_vGroundNormal * -dOnNormal * 0.2; // was * 0.15

    rWheel.m_dDepth += 0.05;
  }

  // Then frictions (brake and wheel orientation based)
  double dFrictionFactor = 0.00099;
  if(m_vehicle.m_bBreaking) {
    dFrictionFactor = g_cdBrakesFriction * rWheel.m_dBrakeFactor * rWheel.m_dFriction * rWheel.m_dGroundFriction;
    dFrictionFactor = BreakProfile(dFrictionFactor);
  } else {
    dFrictionFactor = max(dFrictionFactor, (1.0 - rWheel.m_dTTT) * rWheel.m_dFriction * rWheel.m_dGroundFriction);
  }

  // Apply ground oriented friction
  double dLossFactor = 1.0 - (dFrictionFactor * 0.1);
  pBodyPoint->m_vector = (pBodyPoint->m_vector) * dLossFactor;

  // Then acceleration
  if(m_vehicle.m_bAccelerating || m_vehicle.m_bReversing) { // 4WD
    pBodyPoint->m_vector += rWheel.m_orientation.m_vForward * 
                            // rWheel.m_dGroundFriction *
                            // rWheel.m_dFriction *
                            (rWheel.m_vGroundNormal.m_dZ * rWheel.m_vGroundNormal.m_dZ) * 
                            rWheel.m_dDriveFactor *
                            m_dAccelerationFactor * 
                            m_vehicle.m_dHorsePowers * 
                            (m_vehicle.m_bAccelerating ? m_vehicle.m_dAccelerationFactor : -m_vehicle.m_dReversingFactor);
  }

  // Apply turning
  if(!m_vehicle.m_bBreaking ) {
    BVector vWheelVector = pBodyPoint->m_vector;
    BVector vIdeal = rWheel.m_orientation.m_vForward;
    if((vIdeal.ScalarProduct(vWheelVector) < 0.0) && 
       !m_vehicle.m_bAccelerating) {
      vIdeal = vIdeal * -1.0;
    }
    vIdeal.ToUnitLength();

    BVector vReality = vWheelVector;
    double  dRealLen = vReality.Length();
    vReality.ToUnitLength();

    // double dEffect = rWheel.m_dTTT * rWheel.m_dFriction * rWheel.m_dGroundFriction * 0.05;
    double dEffect = rWheel.m_dTTT * rWheel.m_dFriction * rWheel.m_dGroundFriction * 0.2;

    pBodyPoint->m_vector = vIdeal * dRealLen * dEffect + pBodyPoint->m_vector * (1.0 - dEffect);
  }

  // Record hit point for trails
  if(dFrictionFactor > 0.001) {
    ++rWheel.m_nGroundHits;
  }
}


#endif






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
