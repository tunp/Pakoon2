//
// Vehicle
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#pragma once

#include "BaseClasses.h"
#include "..\PakoonPhysicsEngine\PakoonPhysicsEngine.h"
#include "gl\gl.h"
#include "gl\glu.h"


//*************************************************************************************************
class BTrailPoint {
public:
  BVector m_vLocation[2];
  bool    m_bStart;
  bool    m_bEnd;
  double  m_dStrength;
  BTrailPoint() {m_bEnd = m_bStart = false; m_dStrength = 0.0;}
};

//*************************************************************************************************
class BWheel : public PPEC_Wheel {
public:

  enum TWheelStyle{CHROMEDOME, OFFROAD, CLASSICAIRPLANE, MOMSNEWWHEELS, RACINGFEVER, SEMISPOKE};

  int          m_nDLWheel;
  int          m_nDLWheelBlade;

  BVector      m_vLocationOrig; // In "car's coordinate system"
  BVector      m_vLocation;     // In worlds's coordinate system

  double       m_dProfileHeight;
  double       m_dWidth;
  TWheelStyle  m_style;

  bool         m_bTurns;
  double       m_dThrow;
  bool         m_bLeft;
  bool         m_bBroken;
  double       m_dAngle;
  double       m_dAngleStep;

  int          m_nTrailPoints;
  int          m_nTrailHead;
  BTrailPoint  m_pTrailPoint[g_cnMaxTrailPoints];

  BWheel() {m_dAngle = m_dAngleStep = 0.0; 
            m_bInGround = false; 
            m_nTrailPoints = 0; 
            m_nTrailHead = -1; 
            m_nGroundHits = 0; 
            m_pTrailPoint[0].m_bStart = true;
            m_bTouchesGround = false;}
  void SetPoints(BBodyPoint *pPoints) {m_pBodyPoint = pPoints;};
};


//*************************************************************************************************
class BRotor {
public:
  BVector       m_vExitPoint;
  BVector       m_vExitDir;
  double        m_dBladeLength;
  double        m_dBladeTipWidth;
  int           m_nBodyPoints[4];
  double        m_dBodyEffects[4];
  double        m_dEffect;

  double        m_dHeliMode;     // Heli mode phase
  int           m_nHeliMode;     // Helicopter mode phase
  bool          m_bHeliModeActivating; // Going to heli mode or coming out?
  bool          m_bHeliLifting;  
  bool          m_bHeliDescending;  
  bool          m_bHeliForwarding;
  bool          m_bHeliBacking;
  bool          m_bHeliRighting;  
  bool          m_bHeliLefting;
  double        m_dHeliLift;
  double        m_dHeliForward;
  double        m_dHeliRight;
  bool          m_bHeliOK;
  bool          m_bHeliBladeOK[3];
  bool          m_bHeliCoverOK;
  bool          m_bHeliHatchesOK;
  double        m_dHeliLeftHatchOffsetAngle;
  double        m_dHeliRightHatchOffsetAngle;
  double        m_dHeliBladePower;
};

//*************************************************************************************************
class BJet {
public:
  BVector       m_vExitPoint;
  BVector       m_vExitDir;
  double        m_dExitDiameter;
  int           m_nBodyPoints[4];
  double        m_dBodyEffects[4];
  double        m_dEffect;

  double        m_dJetMode;           // Jet mode phase
  int           m_nJetMode;           // Jet mode phase
  bool          m_bJetModeActivating; // Going to jet mode or coming out?
};


//*************************************************************************************************
class BSmokeTrail {
public:
  BVector m_vLocation;
  BVector m_vRight;
  BVector m_vUp;
  double  m_dAlpha;
};

//*************************************************************************************************
// The whole vehicle
class BVehicle : public PPEC_Vehicle {

public:

  BBodyPoint *m_pOrigBodyPoint;

  // General properties

  CString       m_sName;
  CString       m_sImageFilename;

  bool          m_bVerified;

  // Smoketrail related

  int         m_nTrailpoints;
  int         m_nTrailpoint[5];
  int         m_nTrailpointEntries;
  BSmokeTrail m_trailpointEntry[5][100];
  int         m_nTrailpointHead;

  // Shape/Geometry related properties

  int           m_nParts;
  BPart        *m_pPart;
  BPart         m_partShadow;
  BPart         m_partRudder;
  BPart         m_partElevator;
  BPart         m_partPropeller;

  BOBJData      m_OBJData;

  // General simulation related properties

  double        m_dTotalMass;
  double        m_dSpeedKmh;     // current speed (in km/h)
  BVector       m_vHomeLocation; // Initial location
  BVector       m_vector;        // Vector describing delta to next position
  double        m_dVisualWidth;  // Width,
  double        m_dVisualLength; // Length and
  double        m_dVisualHeight; // Height are used for shadowing if shadow object is not given

  int           m_nForwardPoints[4]; // Indices of body points defining forward direction
  int           m_nRightPoints[4];   // Indices of body points defining right direction

  int           m_nLiftPoint1;   // Body point, which is being lift when user presses 'L'
  int           m_nLiftPoint2;   // Body point, which is being lift when user presses 'L'

  bool          m_bPropeller;    // Prop thrust is on
  bool          m_bPropReverse;  // Prop reverse thrust is on
  double        m_dPropellerFactor; // Prop thrust factor

  bool          m_bWireframe;    // Draw as wireframe
  double        m_dSteeringAid;

  // Engine related properties

  BVector       m_vFuelLocation; // Location of the fueling plug
  double        m_dFuelDistance; // Maximum distance from fuel plug to nozzle for succesful fueling
  double        m_dRPM; // Engine rounds per minute or something like that

  // Heli/Rotor related properties

  bool          m_bHasRotor;
  BRotor        m_rotor;

  // Jet related properties

  bool          m_bHasJet;
  BJet          m_jet;

  // Fuel related properties

  double        m_dFuel;     // Amount of fuel for Vehicle mode
  double        m_dKerosine; // Amount of fuel for jet and copter mode
  double        m_dFuelFactor;     // 1 or 0 depending whether there's fuel or not
  double        m_dKerosineFactor; // (same for kerosine)
  
  // Visualization related properties

  int           m_nDLVehicleBody;
  int           m_nDLElevator;
  int           m_nDLRudder;
  int           m_nDLPropeller;

  // Methods

  BVehicle();
  virtual ~BVehicle();

  bool   LoadWheelFromFile(CString sFilename, 
                           int nWheel, 
                           BWheel *pWheel);

  void LoadVehicleFromFile(CString sFilename, bool bLocalCar = true);
  void PrepareVehicle();
  void LoadTextures();
  void InitAll();
  void DeleteAll();

  virtual void BodyTouchesMatterAt(BVector vCollLoc);
  virtual void WheelTouchesMatterAt(BVector vLoc, double dRadius);
  virtual double GroundHardnessAt(BVector vLoc);
  virtual double PointInsideMatter(BVector vPoint,          // [IN]  Point which is checked for matter collision
                                   BVector& rvNormal,       // [OUT] Normal of the (closest) surface point
                                   double &rdFriction,      // [OUT] Friction of the (closest) surface point
                                   double &rdBaseDepth,     // [OUT] How deep inside the matter the vPoint is
                                   double &rdThermoLoss,    // [OUT] Thermodynamic loss at the collision point  
                                   bool   &rbCarCollision); // [OUT] Was this a car-to-car collision?

  void    Move(BVector& vRelMove);
  BVector GetHomeLocation() {return m_vHomeLocation;}
  void    Paint(int m_nPhysicsSteps);
  void    DrawPropeller();

  void    RotateVehicleToRightOrientation();
  void    PreProcessVisualization();
  void    PreProcessWheels();
  void    RenderWheelBlade(double dLen, 
                           double dWide, 
                           double dNarrow, 
                           double dWideRound,
                           double dNarrowRound,
                           double dDepthOut,
                           double dDepthIn,
                           int nWheel);
  void    ConsumeFuel(double dAmount);
  void    ConsumeKerosine(double dAmount);
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
