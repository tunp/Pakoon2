//
// PAKOON Physics Engine Core
// 
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#include <ctime>
#include <algorithm>

using namespace std;

#include "PakoonPhysicsEngine.h"

// const double g_cdBrakesFriction = 0.5;
const double g_cdBrakesFriction = 0.75;
const double g_cdGravityZ = 0.0010; // was 0.0012
const double g_cdAccelerationFactor = 0.005;

//*************************************************************************************************
class BString {
  BBodyPoint *m_pBodyPoint;
public:
  int     m_nP1;
  int     m_nP2;
  double  m_dIdealLength;
  double  m_dRigidity;
  double  m_dFactor1;
  double  m_dFactor2;

  BString();

  inline double Length() {return (m_pBodyPoint[m_nP1].m_vLocation - m_pBodyPoint[m_nP2].m_vLocation).Length();};
  void          SetPoints(BBodyPoint *pPoints) {m_pBodyPoint = pPoints;};
};

//*************************************************************************************************
BString::BString() {
  m_nP1 = 0;
  m_nP2 = 0;
  m_dIdealLength = 1.0;
  m_dRigidity = 1.0;
}

//*************************************************************************************************
class PPEC_BBaseVehicle {
public:
  BString      *m_pString;  // Strings holding the body together
  int           m_nStrings; // Number of body strings

  void ApplyGroundEffectToAPoint(int i, 
                                 BVector& vNormal, 
                                 double depth, 
                                 double dGroundFriction, 
                                 double dBaseDepth,
                                 bool   bCarToCar);

  bool WheelTouchesGround(PPEC_Wheel *pWheel);
  // bool ApplyWheelForcesFromObjects(BWheel& rWheel);
  void ApplyWheelForces(PPEC_Wheel *pWheel);
  double BreakProfile(double dFrictionFactor);

  PPEC_Vehicle *m_pVehicle; // Pointer to the master (public) vehicle

  PPEC_BBaseVehicle();
  ~PPEC_BBaseVehicle();

  void Init();
	void ApplyGroundEffect();
  void ApplyAirDragAndGravity();
  void UpdateCarPointsUsingMomentumMap();
};


//*****************************************************************************
PPEC_BBaseVehicle::PPEC_BBaseVehicle() {
  m_pString = 0;
}

//*****************************************************************************
PPEC_BBaseVehicle::~PPEC_BBaseVehicle() {
  if(m_pString) {
    delete [] m_pString;
  }
}


//*****************************************************************************
void PPEC_BBaseVehicle::Init() {
  // Create strings
  int i, j;
  m_nStrings = 0;
  for(i = 0; i < m_pVehicle->m_nBodyPoints - 1; ++i) {
    for(j = i + 1;  j < m_pVehicle->m_nBodyPoints; ++j) {
      ++m_nStrings;
    }
  }
  int s = 0;
  if(m_pString) {
    delete m_pString;
    m_pString = 0;
  }
  m_pString = new BString[m_nStrings];
  for(i = 0; i < m_pVehicle->m_nBodyPoints - 1; ++i) {
    for(j = i + 1;  j < m_pVehicle->m_nBodyPoints; ++j) {
      m_pString[s].m_nP1 = i;
      m_pString[s].m_nP2 = j;
      m_pString[s].m_dRigidity = 0.7;
      m_pString[s].m_dIdealLength;
      m_pString[s].m_dIdealLength = (m_pVehicle->m_pBodyPoint[m_pString[s].m_nP1].m_vLocation - 
                                     m_pVehicle->m_pBodyPoint[m_pString[s].m_nP2].m_vLocation).Length();
      m_pString[s].m_dFactor1 = (m_pVehicle->m_pBodyPoint[m_pString[s].m_nP2].m_dMass / (m_pVehicle->m_pBodyPoint[m_pString[s].m_nP1].m_dMass + m_pVehicle->m_pBodyPoint[m_pString[i].m_nP2].m_dMass)) * m_pString[s].m_dRigidity;
      m_pString[s].m_dFactor2 = (m_pVehicle->m_pBodyPoint[m_pString[s].m_nP1].m_dMass / (m_pVehicle->m_pBodyPoint[m_pString[s].m_nP1].m_dMass + m_pVehicle->m_pBodyPoint[m_pString[i].m_nP2].m_dMass)) * m_pString[s].m_dRigidity;
      m_pString[s].SetPoints(m_pVehicle->m_pBodyPoint);
      ++s;
    }
  }
}


//*****************************************************************************
void PPEC_BBaseVehicle::UpdateCarPointsUsingMomentumMap() {
  for(int i = 0; i < m_nStrings; ++i) {
    BString *pString = &(m_pString[i]);
    BBodyPoint *p1 = &(m_pVehicle->m_pBodyPoint[pString->m_nP1]);
    BBodyPoint *p2 = &(m_pVehicle->m_pBodyPoint[pString->m_nP2]);
    BVector vNew1 = p1->m_vLocation + p1->m_vector;
    BVector vNew2 = p2->m_vLocation + p2->m_vector;

    double dLenFix = (pString->m_dIdealLength - (vNew1 - vNew2).Length());
    double dLengthFix1 = dLenFix * pString->m_dFactor1;
    double dLengthFix2 = dLenFix * pString->m_dFactor2;
    BVector vFix1 = vNew1 - vNew2;
    vFix1.ToUnitLength();
    p1->m_vector += vFix1 * dLengthFix1;
    p2->m_vector += vFix1 * -dLengthFix2;
	}
}


//*****************************************************************************
void PPEC_BBaseVehicle::ApplyAirDragAndGravity() {
  // Add constant z-vector to all points to apply gravity
  int i;
  for(i = 0; i < m_pVehicle->m_nBodyPoints; ++i) {
    m_pVehicle->m_pBodyPoint[i].m_vector += m_pVehicle->m_vGravity;
  }

  if(m_pVehicle->m_bAerodynamics) {
    // Calculate point specific airdrags

    double dDragScaler = 0.02;
    if(m_pVehicle->m_bHasAirplaneControls) {
      dDragScaler = 0.06;
    }

    // Add a counterforce to apply airdrag
    for(int i = 0; i < m_pVehicle->m_nBodyPoints; ++i) {
      BVector vPointVector = m_pVehicle->m_pBodyPoint[i].m_vector;
      double dSpeed = m_pVehicle->m_pBodyPoint[i].m_vector.Length();
      double dDrag = dSpeed * dSpeed * dDragScaler;
      
      // See if drag is smaller due to orientation
      BVector vMaxDrag = m_pVehicle->m_orientation.m_vRight   * m_pVehicle->m_pAeroPoint[i].m_vMaxDragDir.m_dX +
                         m_pVehicle->m_orientation.m_vForward * m_pVehicle->m_pAeroPoint[i].m_vMaxDragDir.m_dY +
                         m_pVehicle->m_orientation.m_vUp      * -m_pVehicle->m_pAeroPoint[i].m_vMaxDragDir.m_dZ;
      BVector vTo = vPointVector;
      vTo.ToUnitLength();

      double dDragWithSign = vTo.ScalarProduct(vMaxDrag);
      double dDragFactor = fabs(dDragWithSign);
      dDragFactor = sqrt(dDragFactor);
      // double dDragFinal = ((m_pVehicle->m_pAeroPoint[i].m_dRelMinDrag + dDragFactor) / (m_pVehicle->m_pAeroPoint[i].m_dRelMinDrag + 1.0)) * dDrag;
      double dDragFinal = dDragFactor * dDrag * m_pVehicle->m_pAeroPoint[i].m_dMaxDrag;
      double dDragFinal2 = (1.0 - dDragFactor) * dDrag * m_pVehicle->m_pAeroPoint[i].m_dMaxDrag * m_pVehicle->m_pAeroPoint[i].m_dRelMinDrag;
      BVector vFaceDrag = vPointVector * -1.0;

      if(dDragWithSign >= 0.0) {
        m_pVehicle->m_pBodyPoint[i].m_vector += vMaxDrag * -dDragFinal; //  * (1.0 - m_pVehicle->m_pAeroPoint[i].m_dRelMinDrag);
      } else {
        m_pVehicle->m_pBodyPoint[i].m_vector += vMaxDrag * dDragFinal; //  * (1.0 - m_pVehicle->m_pAeroPoint[i].m_dRelMinDrag);
      }
      m_pVehicle->m_pBodyPoint[i].m_vector += vFaceDrag * dDragFinal2;

      // m_pVehicle->m_pBodyPoint[i].m_vector += vFaceDrag * dDragFinal2 * m_pVehicle->m_pAeroPoint[i].m_dRelMinDrag;
      m_pVehicle->m_pBodyPoint[i].m_dLatestDragFactor = vMaxDrag.Length() * dDragFinal;

      // Lift
      double dDrag3 = dSpeed * dSpeed * dDragScaler * 1.15;
      vMaxDrag = vMaxDrag * dDrag3;
      m_pVehicle->m_pBodyPoint[i].m_vDragLift = (vMaxDrag * (1.0 - dDragFactor) * m_pVehicle->m_pAeroPoint[i].m_dLift * 0.75);
      m_pVehicle->m_pBodyPoint[i].m_vector += m_pVehicle->m_pBodyPoint[i].m_vDragLift;
    }

    // Apply control surface effects
    if(m_pVehicle->m_bHasAirplaneControls) {
      // First elevator
      double dSpeed = m_pVehicle->m_pBodyPoint[m_pVehicle->m_airplane.m_nElevBodyPoint].m_vector.Length();
      double dEffectFactor = dSpeed * dSpeed * 0.03;
      BVector vMaxEffect = m_pVehicle->m_orientation.m_vRight   * m_pVehicle->m_airplane.m_vElevMaxEffect.m_dX +
                           m_pVehicle->m_orientation.m_vForward * m_pVehicle->m_airplane.m_vElevMaxEffect.m_dY +
                           m_pVehicle->m_orientation.m_vUp      * -m_pVehicle->m_airplane.m_vElevMaxEffect.m_dZ;
      BVector vEffect = m_pVehicle->m_orientation.m_vRight   * m_pVehicle->m_airplane.m_vElevEffect.m_dX +
                        m_pVehicle->m_orientation.m_vForward * m_pVehicle->m_airplane.m_vElevEffect.m_dY +
                        m_pVehicle->m_orientation.m_vUp      * -m_pVehicle->m_airplane.m_vElevEffect.m_dZ;
      BVector vTo = m_pVehicle->m_pBodyPoint[m_pVehicle->m_airplane.m_nElevBodyPoint].m_vector;
      vTo.ToUnitLength();
      double dEffect = fabs(vMaxEffect.ScalarProduct(vTo)) * 
                       m_pVehicle->m_airplane.m_dElevEffect *
                       dEffectFactor * 
                       (m_pVehicle->m_airplane.m_dElevator + m_pVehicle->m_airplane.m_dElevTrim);
      m_pVehicle->m_pBodyPoint[m_pVehicle->m_airplane.m_nElevBodyPoint].m_vector += vEffect * -dEffect * 0.92;

      // Then Rudder
      dSpeed = m_pVehicle->m_pBodyPoint[m_pVehicle->m_airplane.m_nRudderBodyPoint].m_vector.Length();
      dEffectFactor = dSpeed * dSpeed * dDragScaler;
      vMaxEffect = m_pVehicle->m_orientation.m_vRight   * m_pVehicle->m_airplane.m_vRudderMaxEffect.m_dX +
                   m_pVehicle->m_orientation.m_vForward * m_pVehicle->m_airplane.m_vRudderMaxEffect.m_dY +
                   m_pVehicle->m_orientation.m_vUp      * -m_pVehicle->m_airplane.m_vRudderMaxEffect.m_dZ;
      vEffect = m_pVehicle->m_orientation.m_vRight   * m_pVehicle->m_airplane.m_vRudderEffect.m_dX +
                m_pVehicle->m_orientation.m_vForward * m_pVehicle->m_airplane.m_vRudderEffect.m_dY +
                m_pVehicle->m_orientation.m_vUp      * -m_pVehicle->m_airplane.m_vRudderEffect.m_dZ;
      vTo = m_pVehicle->m_pBodyPoint[m_pVehicle->m_airplane.m_nRudderBodyPoint].m_vector;
      vTo.ToUnitLength();
      dEffect = fabs(vMaxEffect.ScalarProduct(vTo)) * 
                m_pVehicle->m_airplane.m_dRudderEffect *
                dEffectFactor * 
                m_pVehicle->m_airplane.m_dRudder;
      m_pVehicle->m_pBodyPoint[m_pVehicle->m_airplane.m_nRudderBodyPoint].m_vector += vEffect * -dEffect * 0.35;

      // Then Ailerons
      dSpeed = m_pVehicle->m_pBodyPoint[m_pVehicle->m_airplane.m_nAilrnsBodyPoint1].m_vector.Length();
      dEffectFactor = dSpeed * dSpeed * dDragScaler;
      vMaxEffect = m_pVehicle->m_orientation.m_vRight   * m_pVehicle->m_airplane.m_vAilrnsMaxEffect.m_dX +
                   m_pVehicle->m_orientation.m_vForward * m_pVehicle->m_airplane.m_vAilrnsMaxEffect.m_dY +
                   m_pVehicle->m_orientation.m_vUp      * -m_pVehicle->m_airplane.m_vAilrnsMaxEffect.m_dZ;
      vEffect = m_pVehicle->m_orientation.m_vRight   * m_pVehicle->m_airplane.m_vAilrnsEffect.m_dX +
                m_pVehicle->m_orientation.m_vForward * m_pVehicle->m_airplane.m_vAilrnsEffect.m_dY +
                m_pVehicle->m_orientation.m_vUp      * -m_pVehicle->m_airplane.m_vAilrnsEffect.m_dZ;
      vTo = m_pVehicle->m_pBodyPoint[m_pVehicle->m_airplane.m_nAilrnsBodyPoint1].m_vector;
      vTo.ToUnitLength();
      dEffect = fabs(vMaxEffect.ScalarProduct(vTo)) * 
                m_pVehicle->m_airplane.m_dAilrnsEffect *
                dEffectFactor * 
                m_pVehicle->m_airplane.m_dAilerons;
      m_pVehicle->m_pBodyPoint[m_pVehicle->m_airplane.m_nAilrnsBodyPoint1].m_vector += vEffect * -dEffect * 1.35;

      dSpeed = m_pVehicle->m_pBodyPoint[m_pVehicle->m_airplane.m_nAilrnsBodyPoint2].m_vector.Length();
      dEffectFactor = dSpeed * dSpeed * dDragScaler;
      vTo = m_pVehicle->m_pBodyPoint[m_pVehicle->m_airplane.m_nAilrnsBodyPoint2].m_vector;
      vTo.ToUnitLength();
      dEffect = fabs(vMaxEffect.ScalarProduct(vTo)) * 
                m_pVehicle->m_airplane.m_dAilrnsEffect *
                dEffectFactor * 
                m_pVehicle->m_airplane.m_dAilerons;
      m_pVehicle->m_pBodyPoint[m_pVehicle->m_airplane.m_nAilrnsBodyPoint2].m_vector += vEffect * dEffect * 1.35;
    }
  } else {

    // Use equal air drag for all points

    // Add a counterforce to apply airdrag
    // double dDrag = m_pVehicle->m_dSpeed * m_pVehicle->m_dSpeed * 0.03;
    double dDrag = m_pVehicle->m_dSpeed * m_pVehicle->m_dSpeed * 0.01;
    if(dDrag > 0.9) {
      dDrag = 0.9;
    }
    for(int i = 0; i < m_pVehicle->m_nBodyPoints; ++i) {
      m_pVehicle->m_pBodyPoint[i].m_vector += m_pVehicle->m_pBodyPoint[i].m_vector * -dDrag;
    }
  }
} 




//*****************************************************************************
void PPEC_BBaseVehicle::ApplyGroundEffectToAPoint(int i, 
                                                  BVector& vNormal, 
                                                  double depth, 
                                                  double dGroundFriction, 
                                                  double dBaseDepth,
                                                  bool bCarToCar) {
  // apply friction correction
  BVector vDir = m_pVehicle->m_pBodyPoint[i].m_vector;
  vDir.ToUnitLength();
  double dLossFactor = fabs(vNormal.ScalarProduct(vDir));

  if(!bCarToCar) {

    // Apply thermodynamic losses
    double dThermoLoss = 0.99 - (m_pVehicle->ThermoDynamicLoss() * 
                                dLossFactor * 
                                m_pVehicle->m_pBodyPoint[i].m_dFriction *
                                dGroundFriction * 
                                4.5);
    m_pVehicle->m_pBodyPoint[i].m_vector = m_pVehicle->m_pBodyPoint[i].m_vector * dThermoLoss;

    // Apply ground oriented thermoloss (to stop the ridiculous bounching)
    double dOnNormal = m_pVehicle->m_pBodyPoint[i].m_vector.ScalarProduct(vNormal);
    // m_pVehicle->m_pBodyPoint[i].m_vector = m_pVehicle->m_pBodyPoint[i].m_vector + vNormal * -dOnNormal * 0.7; // was * 0.25 and 0.5 and 0.4
    m_pVehicle->m_pBodyPoint[i].m_vector = m_pVehicle->m_pBodyPoint[i].m_vector + vNormal * -dOnNormal * 0.4; // was * 1.0 and 0.25 and 0.5 and 0.4
  }

  if(vNormal.m_dZ < -0.95) {
    vNormal.Set(0, 0, -1);
  }

  // "cannot be under ground" correction
  if(dBaseDepth > 0.0) {
    double dCBUG = dBaseDepth * m_pVehicle->GroundHardnessAt(m_pVehicle->m_pBodyPoint[i].m_vLocation) * 0.5;
    BVector vAntigravity = vNormal * dCBUG;
    m_pVehicle->m_pBodyPoint[i].m_vector = m_pVehicle->m_pBodyPoint[i].m_vector + vAntigravity;
  }
}

//*****************************************************************************
void PPEC_BBaseVehicle::ApplyGroundEffect() {

  bool bCarToCar;

  m_pVehicle->m_bTouchMatter = false;

  int i;
  double dThermoLoss = m_pVehicle->ThermoDynamicLoss();
  for(i = 0; i < m_pVehicle->m_nBodyPoints; ++i) {
    double depth, dBaseDepth, dGroundFriction;
    BVector vNormal;
    // Apply base ground effect
    if((depth = m_pVehicle->PointInsideMatter(m_pVehicle->m_pBodyPoint[i].m_vLocation, 
                                              vNormal, 
                                              dGroundFriction, 
                                              dBaseDepth,
                                              dThermoLoss,
                                              bCarToCar)) > 0.0) {
      m_pVehicle->m_bTouchMatter = true;
      ApplyGroundEffectToAPoint(i, vNormal, depth, dGroundFriction, dBaseDepth, bCarToCar);
      m_pVehicle->m_bHitDetected = true;
      if(dBaseDepth * m_pVehicle->m_pBodyPoint[i].m_vector.Length() > m_pVehicle->m_dHitSpeed) {
        m_pVehicle->m_dHitSpeed = dBaseDepth * m_pVehicle->m_pBodyPoint[i].m_vector.Length();
      }
    }
    if((dBaseDepth > 0.0) && 
       (m_pVehicle->m_dSpeed > 0.02)) {
      m_pVehicle->BodyTouchesMatterAt(m_pVehicle->m_pBodyPoint[i].m_vLocation);      
    }
	}

  m_pVehicle->m_bWheelsTouchGround = false;
	for(i = 0; i < m_pVehicle->m_nWheels; ++i) {
    m_pVehicle->m_pWheel[i]->m_dSuspension *= 0.5;
    if(WheelTouchesGround(m_pVehicle->m_pWheel[i])) {
      ApplyWheelForces(m_pVehicle->m_pWheel[i]);
      m_pVehicle->m_bWheelsTouchGround = true;
      m_pVehicle->m_bTouchMatter = true;
    }

    if(m_pVehicle->m_pWheel[i]->m_dSuspension < m_pVehicle->m_pWheel[i]->m_dMinSusp) {
      m_pVehicle->m_pWheel[i]->m_dSuspension = m_pVehicle->m_pWheel[i]->m_dMinSusp;
    }
    if(m_pVehicle->m_pWheel[i]->m_dSuspension > m_pVehicle->m_pWheel[i]->m_dMaxSusp) {
      m_pVehicle->m_pWheel[i]->m_dSuspension = m_pVehicle->m_pWheel[i]->m_dMaxSusp;
    }
	}
} 

//*****************************************************************************
bool PPEC_BBaseVehicle::WheelTouchesGround(PPEC_Wheel *pWheel) {
  double dTmp;
  bool bCarToCar;

  BVector vLoc = pWheel->m_vSuspBasePoint + 
                 pWheel->m_vSuspDir * pWheel->m_dSuspRelaxedDistance + 
                 pWheel->m_vSuspDir * -pWheel->m_dSuspension;
  pWheel->m_vLocSample = m_pVehicle->ToWorldCoord(vLoc);

  pWheel->m_vLocSample = pWheel->m_vLocSample + (pWheel->m_orientation.m_vUp * -pWheel->m_dRadius);

  pWheel->m_dDepth = m_pVehicle->PointInsideMatter(pWheel->m_vLocSample, 
                                                   pWheel->m_vGroundNormal, 
                                                   pWheel->m_dGroundFriction, 
                                                   pWheel->m_dBaseDepth,
                                                   dTmp,
                                                   bCarToCar);
  pWheel->m_dDepth = pWheel->m_dBaseDepth;

  if(pWheel->m_dDepth > 0.0) {
    // Calculate TTT
    BVector vDir = pWheel->m_pBodyPoint[pWheel->m_nBodyPoint].m_vector;
    vDir.ToUnitLength();

    double dTTT1 = fabs(vDir.ScalarProduct(pWheel->m_orientation.m_vForward));
    pWheel->m_dTTT = dTTT1;
    if(pWheel->m_dTTT > 0.85) {
      pWheel->m_dTTT = 1.0;
    }
    pWheel->m_bTouchesGround = true;

    if(m_pVehicle->m_dSpeed > 0.1) {
      m_pVehicle->WheelTouchesMatterAt(pWheel->m_vLocSample, pWheel->m_dRadius);
    }
    return true;
  } else {
    return false;
  }
}


//*****************************************************************************
void PPEC_BBaseVehicle::ApplyWheelForces(PPEC_Wheel *pWheel) {
  pWheel->m_bInGround = true;

  BBodyPoint* pBodyPoint = &(pWheel->m_pBodyPoint[pWheel->m_nBodyPoint]);

  BVector vSpeed = pBodyPoint->m_vector;
  double dSpeed = vSpeed.Length();
  vSpeed.ToUnitLength();
  double dThermoLoss = fabs(vSpeed.ScalarProduct(pWheel->m_vGroundNormal));
  dThermoLoss *= (1.0 - fabs(vSpeed.ScalarProduct(pWheel->m_orientation.m_vForward)));

  if(pWheel->m_dDepth > 0.05) {
    pWheel->m_dDepth -= 0.05;

    double dPerpendicularity = fabs(pWheel->m_orientation.m_vUp.ScalarProduct(pWheel->m_vGroundNormal)); 
    double dPrevSuspension = pWheel->m_dSuspension;
    double dNewSuspension  = dPrevSuspension + dPerpendicularity * pWheel->m_dDepth;

    double dSuspGets = ((dPrevSuspension + dNewSuspension) * 0.5) / pWheel->m_dMaxSuspThrow;

    if(dSuspGets > 1.0) {
      dSuspGets = 1.0;
    }
    dSuspGets = 1.0 - pow(dSuspGets, 1.0 / pWheel->m_dSuspStiffness);

    // Viscosity damping
    double dViscDamp = pWheel->m_dDepth / pWheel->m_dMaxSuspThrow;
    if(dViscDamp > 1.0) {
      dViscDamp = 1.0;
    }

    dSuspGets *= (1.0 - dViscDamp);

    pWheel->m_dSuspension += dSuspGets * pWheel->m_dDepth;

	  double dBodyPointGets = 1.0 - dSuspGets;

    // Transfer the ground force to the body point
    double dSlide = fabs(pWheel->m_vGroundNormal.ScalarProduct(BVector(0, 0, -1)));
    // dSlide *= 1.3;
    dSlide *= 0.3;
    if(dSlide > 1.0) {
      dSlide = 1.0;
    }
    
    BVector vRightOnGround = pWheel->m_orientation.m_vForward.CrossProduct(BVector(0, 0, -1));
    vRightOnGround.ToUnitLength();
    double dCorr = pWheel->m_vGroundNormal.ScalarProduct(vRightOnGround);
    BVector vCorr = pWheel->m_vGroundNormal + vRightOnGround * -(dCorr * 0.85 * dSlide);

    double dGroundForceFactor = (dBodyPointGets * pWheel->m_dDepth * m_pVehicle->GroundHardnessAt(pWheel->m_vLocSample) * 0.5);
    pBodyPoint->m_vector += vCorr * dGroundForceFactor;

    // Apply thermodynamic loss
    pBodyPoint->m_vector = pBodyPoint->m_vector * (1.0 - 0.3 * dBodyPointGets * dThermoLoss);
    
    // Apply ground oriented thermoloss (to stop the ridiculous bounching)
    double dOnNormal = pBodyPoint->m_vector.ScalarProduct(pWheel->m_vGroundNormal);
    pBodyPoint->m_vector = pBodyPoint->m_vector + pWheel->m_vGroundNormal * -dOnNormal * 0.2; // was * 0.15

    pWheel->m_dDepth += 0.05;
  }

  // Then frictions (brake and wheel orientation based)
  double dFrictionFactor = 0.00099;
  if(m_pVehicle->m_bBreaking) {
    dFrictionFactor = g_cdBrakesFriction * pWheel->m_dBrakeFactor * pWheel->m_dFriction * pWheel->m_dGroundFriction;
    dFrictionFactor = BreakProfile(dFrictionFactor);
  } else {
    dFrictionFactor = max(dFrictionFactor, (1.0 - pWheel->m_dTTT) * pWheel->m_dFriction * pWheel->m_dGroundFriction);
  }

  // Apply ground oriented friction
  double dLossFactor = 1.0 - (dFrictionFactor * 0.1);
  pBodyPoint->m_vector = (pBodyPoint->m_vector) * dLossFactor;

  // Then acceleration
  if(m_pVehicle->m_bAccelerating || m_pVehicle->m_bReversing) { // 4WD
    double dFactorModified = (m_pVehicle->m_bAccelerating ? m_pVehicle->m_dAccelerationFactor : -m_pVehicle->m_dReversingFactor) * m_pVehicle->m_dHorsePowers;
    if(dSpeed < (fabs(dFactorModified) * 0.3)) {
      dFactorModified *= (0.4 + 0.6 * (dSpeed / (fabs(dFactorModified) * 0.3)));
    }
    double dAccFactor = fabs(pWheel->m_vGroundNormal.m_dZ) * /* * pWheel->m_vGroundNormal.m_dZ) * */
                         pWheel->m_dDriveFactor *
                         g_cdAccelerationFactor * 
                         dFactorModified;
    pBodyPoint->m_vector += pWheel->m_orientation.m_vForward * dAccFactor;
  }

  // Apply turning
  if(!m_pVehicle->m_bBreaking ) {
    BVector vWheelVector = pBodyPoint->m_vector;
    BVector vIdeal = pWheel->m_orientation.m_vForward;
    if((vIdeal.ScalarProduct(vWheelVector) < 0.0) && 
       !m_pVehicle->m_bAccelerating) {
      vIdeal = vIdeal * -1.0;
    }
    vIdeal.ToUnitLength();

    BVector vReality = vWheelVector;
    double  dRealLen = vReality.Length();
    vReality.ToUnitLength();

    double dEffect = pWheel->m_dTTT * pWheel->m_dFriction * pWheel->m_dGroundFriction * 0.2;

    pBodyPoint->m_vector = vIdeal * dRealLen * dEffect + pBodyPoint->m_vector * (1.0 - dEffect);
  }

  // Record hit point for trails
  if(dFrictionFactor > 0.001) {
    ++pWheel->m_nGroundHits;
  }
}

//*****************************************************************************
double PPEC_BBaseVehicle::BreakProfile(double dFrictionFactor) {
  // Break friction shall be speed dependent
  double dScale = 1.0 - dFrictionFactor;
  double dFactor = m_pVehicle->m_dSpeed / 0.28;
  if(dFactor > 1.0) {
    dFactor = 1.0;
  }
  // dFactor = pow(dFactor, 10.0);
  if(fabs(m_pVehicle->m_dSpeed) < 0.07) {
    return 1.0;
  } else {
    return dFrictionFactor + dScale * pow(1.0 - dFactor, 2.0);
  }
}






//*************************************************************************************************
//*************************************************************************************************
//*************************************************************************************************
//*************************************************************************************************
// 
// START OF PUBLIC INTERFACE
// 
//*************************************************************************************************
//*************************************************************************************************
//*************************************************************************************************
//*************************************************************************************************




//*************************************************************************************************
// GLOBAL STUFF 
const int          g_cnMaxVehicles = 100;
PPEC_BBaseVehicle *g_baseVehicles[g_cnMaxVehicles];

int GetFreeIndex() {
  static bool bInit = true;
  int i;
  if(bInit) {
    bInit = false;
    for(i = 0; i < g_cnMaxVehicles; ++i) {
      g_baseVehicles[i] = 0;
    }
  }

  for(i = 0; i < g_cnMaxVehicles; ++i) {
    if(!g_baseVehicles[i]) {
      return i;
      break;
    }
  }
  return 0;
}


//*************************************************************************************************
PPEC_Vehicle::PPEC_Vehicle(): m_nID(GetFreeIndex()) {
  g_baseVehicles[m_nID] = new PPEC_BBaseVehicle;
  g_baseVehicles[m_nID]->m_pVehicle = this;
  m_vGravity.Set(0, 0, g_cdGravityZ);
}

//*************************************************************************************************
PPEC_Vehicle::~PPEC_Vehicle() {
  delete g_baseVehicles[m_nID];
  g_baseVehicles[m_nID] = 0;
}


//*****************************************************************************
void PPEC_Vehicle::Init() {
  g_baseVehicles[m_nID]->Init();
}


//*****************************************************************************
double PPEC_Vehicle::GetMaxGForce() {
  // Track changes in body point[0], which is the center of gravity point
  double dForce = 0.0;
  for(int i = 0; i < g_baseVehicles[m_nID]->m_nStrings; ++i) {
    BVector v1 = m_pBodyPoint[g_baseVehicles[m_nID]->m_pString[i].m_nP1].m_vLocation;
    BVector v2 = m_pBodyPoint[g_baseVehicles[m_nID]->m_pString[i].m_nP2].m_vLocation;
    dForce += fabs((v1 - v2).Length() - g_baseVehicles[m_nID]->m_pString[i].m_dIdealLength);
  }
  dForce *= 0.5;
  return dForce;
}


//**********************************************************************
BVector PPEC_Vehicle::ToWorldCoord(BVector vLoc) {
  return m_vLocation + 
         m_orientation.m_vRight   *  vLoc.m_dX + 
         m_orientation.m_vForward *  vLoc.m_dY + 
         m_orientation.m_vUp      * -vLoc.m_dZ;
}




//*************************************************************************************************
void PPEC_Vehicle::SimulateTimeStep() {
  static bool bInit = true;
  static clock_t clockFirstUse = 0;
  if(bInit) {
    bInit = false;
    clockFirstUse = clock();
  }

  /*
  if(clock() > (clockFirstUse + CLOCKS_PER_SEC * 39 * 7)) {
    for(int i = 0; i < m_nBodyPoints; ++i) {
      m_pBodyPoint[i].m_vector.Set(0, 0, 0);
    }
    return;
  }
  */

  PPEC_BBaseVehicle *p = g_baseVehicles[m_nID];
  p->ApplyGroundEffect();
  p->ApplyAirDragAndGravity();
  p->UpdateCarPointsUsingMomentumMap();
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
