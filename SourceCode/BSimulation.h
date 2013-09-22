//
// Simulation
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#pragma once

#include "stdafx.h"
#include "BaseClasses.h"
#include "BVehicle.h"
#include "BCamera.h"
#include "BTerrain.h"
#include "BScene.h"
#include "ControllerModule.h"
#include "OpenGLHelpers.h"

#include "..\PakoonPhysicsEngine\PakoonPhysicsEngine.h"



class BTrackingTarget {
public:
  BTrackingTarget *m_pNext;
  CString          m_sId;
  BVector          m_vLoc;
  BVector          m_vScreenPos;
  double           m_dRed;
  double           m_dGreen;
  double           m_dBlue;
};



const int g_cnDustClouds1 = 100;
const int g_cnDustClouds2 = 100;
const int g_cnDustClouds3 = 50;


class BCloudPuff {
public:
  BVector m_vLocation;
  double  m_dRadius;
};

class BCloud {
public:
  BVector    m_vCenter;
  int        m_nPuffs;
  BCloudPuff m_puff[10];
};


class BSimulation {

  BVehicle         m_vehicle;
  BCamera          m_camera;
  BScene           m_scene;
  BTerrain         m_terrain;

  OpenGLHelpers    m_tmpHelpers;

  int              m_nDustCloudsHead1;
  int              m_nDustClouds1;
  BDustCloud       m_dustClouds1[g_cnDustClouds1];

  int              m_nDustCloudsHead2;
  int              m_nDustClouds2;
  BDustCloud       m_dustClouds2[g_cnDustClouds2];

  int              m_nDustCloudsHead3;
  int              m_nDustClouds3;
  BDustCloud       m_dustClouds3[g_cnDustClouds3];

  int              m_nClouds;
  BCloud           m_cloud[3];

public:
  BTrackingTarget *m_targets;
private:

  FILE   *m_fp;

  void ApplyGroundEffectToAPoint(int i, 
                                 BVector& vNormal, 
                                 double depth, 
                                 double dGroundFriction, 
                                 double dBaseDepth);

public:

  CRect  m_rectWnd;

  long   m_nSimulationTimeStep;

  double m_dMaxGForce;

  static const double g_cdBrakesFriction;
  static const double g_cdMaxSpeed;
  static const double g_cdTurnFactor;
  static const double g_cdAirTurnFactor;
  static const double g_cdPropThrustFactor;

  bool   m_bLiftingUp;   // Is the car being lifted up by a virtual crane
  double m_dLiftZ;       // Z coordinate of the car when the lifting was started
  bool   m_bRaining;     // Debug code to the ground depth
  bool   m_bPaused;
  bool   m_bSteeringAidOn;
  bool   m_bCalibrateSimulationSpeed;

  bool   m_bRecordTrail;

  // double m_dAccelerationFactor;
  double m_dTurnFactor;
  double m_dAirTurnFactor;
  double m_dPropThrustFactor;
  int    m_nPhysicsStepsBetweenRender;
  double m_dPhysicsFraction;
  int    m_nPhysicsSteps;

  BTerrainBlock *m_pCenterBlock;

  BSimulation();
  ~BSimulation();

  BVehicle *GetVehicle() {return &m_vehicle;}
  BCamera  *GetCamera()  {return &m_camera;}
  BTerrain *GetTerrain() {return &m_terrain;}
  BScene   *GetScene()   {return &m_scene;}

  void StartRecording();
  void StopRecording();

  void PrePaint(CDC *pDC);
  void PreProcessVisualization();
  void PaintSceneObjects();
  void PaintSky(float fBrightness, bool bFog);
  int  Paint(CDC *pDC, bool bCreateDLs, bool bWireframe, bool bNormals, CRect &rectWnd);
  void DrawShadowAndTrails();
  void DrawTrails();
  void DrawDustClouds();
  void DrawSmokeTrails();

  void SetUpCamera(CRect *pRect = 0);
  void UpdateCar();
  void UpdateCarLocation();
  void EnsureVehicleIsOverGround();
  void SwitchCameraMode();
  void UpdateAirplaneControls();
  void UpdateTrails();
  void UpdateDustClouds();
  void CreateDustCloudAt(BVector vHitPoint, int nSlot, double dInitialAlpha = -1.0, BVector vInitial = BVector(0, 0, 0), double dOffset = 0.0);

  void UpdateGameMusic();
	void MoveCarPoints();
	void ApplySteering();

  BTerrainBlock *FindTerrainBlock(BVector &rvPoint);
  double PointUnderGround(BVector vPoint, BVector& rvNormal, double &rdFriction, double &rdBaseDepth, double &rdThermoLoss);
  double PointUnderGroundAccurate(BVector vPoint, BVector& rvNormal, double &rdFriction, double &rdBaseDepth, double &rdThermoLoss);
  double PointInsideObject(BVector& rvPoint, BVector& rvNormal, double &rdFriction, double &rdBaseDepth);
  double PointInsideRemoteCar(BVector& rvPoint, BVector& rvNormal, double &rdFriction, double &rdBaseDepth);
  double PointUnderGroundShadow(BVector vPoint, BVector& rvNormal);
  double PointUnderObjectsShadow(double& rdCandidate, BVector& rvPoint, BVector& rvNormal, bool& rbChanged);
  double Friction(BBodyPoint& rPoint);
  double Friction(BVector& rPoint);
  double GroundHardnessAt(BVector& rvLocation);

  void   AddTrackingTarget(CString sId, BVector vLoc, double dRed, double dGreen, double dBlue);
  void   RemoveTrackingTarget(CString sId);
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
