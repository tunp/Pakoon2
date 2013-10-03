//
// PAKOON Physics Engine Core
//
// NOTE: This is the developer version and works for approx. 5 minutes.
//       After that the vehicle simply stops.
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#pragma once

#include "../SourceCode/CoreClasses.h"

//*************************************************************************************************
// Base class for vehicle body point
class BBodyPoint {
public:
  BVector m_vLocation;  // In worlds's coordinate system
  BVector m_vector;
  double  m_dFriction;
  double  m_dMass;

  BVector m_vDragLift;
  double  m_dLatestDragFactor;

  BBodyPoint() {m_dFriction = 1.0; m_dMass = 1.0;}
};

//*************************************************************************************************
class BAeroPoint {
public:
  BVector m_vMaxDragDir;
  double  m_dMaxDrag;
  double  m_dRelMinDrag;
  double  m_dLift;
};

//*************************************************************************************************
class BAirplane {
public:

  int     m_nRudderBodyPoint;
  BVector m_vRudderMaxEffect;
  BVector m_vRudderEffect;
  BVector m_vRudderPivot;
  BVector m_vRudderRotAxis;
  double  m_dRudderThrow;
  double  m_dRudderEffect;

  double  m_dRudder;

  int     m_nElevBodyPoint;
  BVector m_vElevMaxEffect;
  BVector m_vElevEffect;
  BVector m_vElevPivot;
  BVector m_vElevRotAxis;
  double  m_dElevThrow;
  double  m_dElevEffect;
  double  m_dElevTrim;

  double  m_dElevator;

  int     m_nAilrnsBodyPoint1;
  int     m_nAilrnsBodyPoint2;
  BVector m_vAilrnsMaxEffect;
  BVector m_vAilrnsEffect;
  double  m_dAilrnsEffect;

  double  m_dAilerons;

  int     m_nPropBodyPoint;
  BVector m_vPropDir;
  BVector m_vPropPos;
  double  m_dPropEffect;
  double  m_dPropAngle;

  double  m_dUseRudderForTurn;
};


//*************************************************************************************************
// Base class for vehicle wheel
class PPEC_Wheel {
public:
  double m_dRadius;

  // Suspension properties 
  double       m_dSuspRelaxedDistance;
  double       m_dSuspension;
  double       m_dMaxSuspThrow;
  double       m_dSuspStiffness;
  double       m_dMaxSusp;
  double       m_dMinSusp;

  int          m_nBodyPoint;
  BVector      m_vSuspDir;
  BVector      m_vSuspBasePoint;

  BVector      m_vLocSample;    // In worlds's coordinate system
  BOrientation m_orientation;

  int          m_bTouchesGround;
  bool         m_bInGround;

  // Auxiliary members
  double       m_dDepth;
  BVector      m_vGroundNormal;
  double       m_dGroundFriction;
  double       m_dBaseDepth;

  double       m_dTTT;      // Tendency To Turn (factor based on wheel orientation)
  BBodyPoint  *m_pBodyPoint;

  // Simulation related properties 
  double       m_dDriveFactor;
  double       m_dBrakeFactor;

  double       m_dFriction;
  int          m_nGroundHits;

  virtual void Init() {}
};


//*************************************************************************************************
// Base class for vehicle
class PPEC_Vehicle {
  const int m_nID; 
  // (reserved for internal use)

public:

  // **********************************************************************************************
  // Public member variables
  // **********************************************************************************************

  BVector m_vGravity;
  // [IN] Direction and strength of the gravity

  int m_nBodyPoints;   
  // [IN] Number of body points

  BBodyPoint *m_pBodyPoint;    
  // [IN] Points making the Vehicle body

  bool m_bWheelsTouchGround; 
  // [OUT] Valid when SimulateTimeStep() has been called

  bool m_bTouchMatter;
  // [OUT] Any part(wheel or body) touches matter. Valid when SimulateTimeStep() has been called

  bool          m_bAerodynamics; 
  // [IN] Is aerodynamics calculations being used?

  BAeroPoint   *m_pAeroPoint;    
  // [IN] Aerodynamics points

  bool m_bHasAirplaneControls;
  // [IN] Has airplane controls (i.e. rudder, ailerons and elevator

  BAirplane m_airplane;
  // [IN] Airplane stuff

  BOrientation  m_orientation;   
  // [IN] orientation of the Vehicle

  double m_dSpeed;        
  // [IN] current speed (meters/one update step)

  int m_nWheels;       
  // [IN] Number of wheels

  PPEC_Wheel *m_pWheel[10];
  // [IN] Wheels of the Vehicle (pointers to them)

  BVector       m_vLocation;     // vector describing center of mass location
  bool          m_bBreaking;     // Breaks are on?
  bool          m_bHandBreaking; // Handbreak is on?
  bool          m_bAccelerating; // Acceleration pedal is down?
  double        m_dAccelerationFactor;
  bool          m_bReversing;    // Acceleration pedal is down and gear is on R?
  double        m_dReversingFactor;
  bool          m_bTurningLeft;  // guess
  bool          m_bTurningRight; // guess
  double        m_dTurn;         // Amount of front wheel turn at the moment (1.0 = full right, -1.0 = full left, 0.0 = centered)
  double        m_dHorsePowers;

  bool   m_bHitDetected;
  double m_dHitSpeed;

  // **********************************************************************************************
  // Public member functions (not intended to be overridden in the derived class)
  // **********************************************************************************************

  PPEC_Vehicle();
  // Constructor

  virtual ~PPEC_Vehicle();
  // Destructor

  void Init();

  void SetGravity(BVector &rGravity) {m_vGravity = rGravity;}
  // Set the direction and strength of gravity

  BVector GetGravity() {return m_vGravity;}
  // Get the direction and strength of gravity

  double GetMaxGForce();
  // returns the G-force currently affecting the vehicle center

  void SimulateTimeStep();
  // Simulates one time step

  BVector ToWorldCoord(BVector vLoc);
  // Returns a vehicle local point in world coordinates

  // **********************************************************************************************
  // OVERRIDEABLES (intended to be overridden in the derived class)
  // **********************************************************************************************

  virtual void BodyTouchesMatterAt(BVector vCollLoc) {}
  // Called whenever a collision happens and there's enough speed. 
  // Override to add dust particles, scrathes on ground or whatever.

  virtual void WheelTouchesMatterAt(BVector vLoc, double dRadius) {}
  // Called whenever a wheel touches matter and there's enough speed. 
  // Override to add dust particles, skid marks on ground or whatever.

  virtual double GroundHardnessAt(BVector vLoc) {return 1.0;}
  // Override to return ground hardness at the given location (0.0 - 1.0).

  virtual double ThermoDynamicLoss() {return 0.4;} // 0 = lo loss, 1 = total loss
  // Returns the global thermodynamic loss for collisions

  virtual double PointInsideMatter(BVector vPoint,        // [IN]  Point which is checked for matter collision
                                   BVector& rvNormal,     // [OUT] Normal of the (closest) surface point
                                   double &rdFriction,    // [OUT] Friction of the (closest) surface point
                                   double &rdBaseDepth,   // [OUT] How deep inside the matter the vPoint is
                                   double &rdThermoLoss,  // [OUT] Thermodynamic loss at the collision point
                                   bool &rbCarCollision)  // [OUT] Was this a car-to-car collision?
                                   {return -1.0;}
  // DEFINE THIS FUNCTION YOURSELF IN THE DERIVED CLASS!
  // Function should return a distance, which indicates how much the 
  // point (vPoint) is inside the matter. Note that positive return value indicates 
  // point is inside matter, negative return value indicates point is outside matter.
  // In normal cases, the rdBaseDepth parameter should return same value as the whole function.
  // If you want to simulate some liquid or sparse matter (such as water or snow), you can use
  // rdBaseDepth to return how much point is inside a hard base matter and function return value
  // to indicate how much point is inside the sparse, soft matter. The physics engine allows point
  // to go inside the soft matter (function return value) but doesn't allow point to go inside
  // the hard matter (rdBaseDepth parameter).
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
