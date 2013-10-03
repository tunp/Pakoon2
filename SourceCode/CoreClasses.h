//
// Bare core classes for the Pakoon1
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#pragma once

#include <math.h>

class BIndexGuard {
  int m_nMax;
  int m_nMapTo;
public:
  BIndexGuard(int nMax, int nMapTo) {m_nMax = nMax; m_nMapTo = nMapTo;};
  inline int operator()(int nCandidate) {
    if((nCandidate < 0) || (nCandidate > m_nMax)) {
      return m_nMapTo;
    } else {
      return nCandidate;
    }
  };
};

class BVector {
public:
  double m_dX;
  double m_dY;
  double m_dZ;

  inline BVector() {m_dX = 0.0; m_dY = 0.0; m_dZ = 0.0;};
  inline BVector(double a, double b, double c) {m_dX = a; m_dY = b; m_dZ = c;};

  inline void    Set(double a, double b, double c) {m_dX = a; m_dY = b; m_dZ = c;};
  inline BVector operator+(BVector p) {
    BVector vRet;
    vRet.m_dX = m_dX + p.m_dX;
    vRet.m_dY = m_dY + p.m_dY;
    vRet.m_dZ = m_dZ + p.m_dZ;
    return vRet;
  };
  inline BVector& operator+=(BVector p) {
    m_dX += p.m_dX;
    m_dY += p.m_dY;
    m_dZ += p.m_dZ;
    return *this;
  };
  inline BVector operator-(BVector p) {
    BVector vRet;
    vRet.m_dX = m_dX - p.m_dX;
    vRet.m_dY = m_dY - p.m_dY;
    vRet.m_dZ = m_dZ - p.m_dZ;
    return vRet;
  };
  inline BVector& operator-=(BVector p) {
    m_dX -= p.m_dX;
    m_dY -= p.m_dY;
    m_dZ -= p.m_dZ;
    return *this;
  };
  inline BVector operator*(double d) {
    BVector vRet;
    vRet.m_dX = m_dX * d;
    vRet.m_dY = m_dY * d;
    vRet.m_dZ = m_dZ * d;
    return vRet;
  };
  inline double Length() {return sqrt((m_dX * m_dX) + (m_dY * m_dY) + (m_dZ * m_dZ));};
  inline BVector CrossProduct(BVector p) {
    BVector vRet;
    vRet.Set(m_dY * p.m_dZ - m_dZ * p.m_dY,
             m_dZ * p.m_dX - m_dX * p.m_dZ,
             m_dX * p.m_dY - m_dY * p.m_dX);
    return vRet;
  };
  inline double  ScalarProduct(BVector p) {
    return m_dX * p.m_dX + m_dY * p.m_dY + m_dZ * p.m_dZ;
  };
  inline void    ToUnitLength() {
    double dLen = Length();
    m_dX *= 1.0 / dLen;
    m_dY *= 1.0 / dLen;
    m_dZ *= 1.0 / dLen;
  };
};

class BOrientation {
public:

  BVector m_vForward;
  BVector m_vRight;
  BVector m_vUp;

  BOrientation() {m_vForward.Set(0, 1, 0);m_vRight.Set(1, 0, 0);m_vUp.Set(0, 0, -1);};

  void Normalize() {m_vForward.ToUnitLength(); m_vRight.ToUnitLength(); m_vUp.ToUnitLength();};
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
