// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#include "HeightMap.h"
#include "PerlinNoise.h"




double g_dMap1[cnMapSize][cnMapSize];

void HeightMap::GenerateMap(double dXOffset, double dYOffset, double dWidth) {
  // Calculate the whole map
  double dTmp;
  int    x, y;
  double dRatio = double(cnMapSize) / dWidth;
  for(y = 0; y < cnMapSize; ++y) {
    for(x = 0; x < cnMapSize; ++x) {
      g_dMap1[x][y] = CalcHeightAt(double(x) / dRatio + dXOffset, 
                                   double(y) / dRatio + dYOffset, 
                                   dTmp,
                                   HeightMap::BASIC,
                                   8,
                                   dYOffset);
    }
  }
}


double HeightMap::HeightAt(double dX, double dY) {
  if(int(dX) < 0 || int(dX) >= cnMapSize || int(dY) < 0 || int(dY) >= cnMapSize) {
    return 0.0;
  } else {
    return g_dMap1[int(dX)][int(dY)];
  }
}

inline double HeightMap::CalcHeightAt(double dX, double dY, double &rdArea, TTerrain nTerrain, int nOctaves, double dYOffset) {
  double dTmp = 0.0, dTmp2 = 0.0;
  double dMapSize = 100.0;
  switch(nTerrain) {
    case BASIC:
      {
        double dSlope = 0.5;
        double dOriginY = dY - dYOffset;
        dX /= 50.0;
        dY /= (80.0 * 1.2);
        dTmp = 
          (PerlinNoise::PerlinNoise_2D((dX + 7.0) / (100.0 / 33.7), 
                                       dY         / (100.0 / 33.7), 
                                       3, // was 3 and 2
                                       cdPercistence)) * 1.8;
        if(dOriginY < 300.0) {
          if(dOriginY < 20.0) {
            return 3000.0;
          } else {
            double dFadeIn = (dOriginY - 20.0) / 280.0;
            dFadeIn = (1.0 + sin(((dFadeIn * 2.0) - 1.0) * 3.1415926 * 0.5)) * 0.5;
            return (3000.0 - dFadeIn * dOriginY * dSlope) + dFadeIn * (60.0 * pow(dTmp + 0.75, 2));
          }
        } else if(dOriginY > (3000 / dSlope - 500.0)) {
          if(dOriginY > (3000 / dSlope)) {
            return 0.0;
          } else {
            double dFadeOut = ((3000 / dSlope) - dOriginY) / 500.0;
            dFadeOut = (1.0 + sin(((dFadeOut * 2.0) - 1.0) * 3.1415926 * 0.5)) * 0.5;
            return dFadeOut * (3000.0 - dOriginY * dSlope) + dFadeOut * (60.0 * pow(dTmp + 0.75, 2));
          }
        } else {
          return (3000.0 - dOriginY * dSlope) + 60.0 * pow(dTmp + 0.75, 2);
        }
      }
    case GRAND_VALLEY:
      {
        double dSlope = 0.5;
        double dOriginY = dY - dYOffset;
        double dExtraSin = sin((dOriginY / 6000.0) * 9 * 3.141592654) * 80.0;
        dX /= 50.0;
        dY /= 65.0;
        dTmp = 
          (PerlinNoise::PerlinNoise_2D((dX + 7.0) / (100.0 / 33.7), 
                                       dY         / (100.0 / 33.7), 
                                       2, // was 3 and 2
                                       cdPercistence)) * 1.3;
        dTmp += 0.1;
        if(dTmp < 0.0) {
          dTmp /= 2.0;
        } else {
          dTmp = pow(dTmp, 0.5);
        }
        dTmp *= -1.0;

        dTmp = dExtraSin + 150.0 * dTmp;

        if(dOriginY < 300.0) {
          if(dOriginY < 20.0) {
            return 3000.0;
          } else {
            double dFadeIn = (dOriginY - 20.0) / 280.0;
            dFadeIn = (1.0 + sin(((dFadeIn * 2.0) - 1.0) * 3.1415926 * 0.5)) * 0.5;
            return (3000.0 - dFadeIn * (dOriginY * dSlope)) + dFadeIn * dTmp;
          }
        } else if(dOriginY > (3000 / dSlope - 500.0)) {
          if(dOriginY > (3000 / dSlope)) {
            return 0.0;
          } else {
            double dFadeOut = ((3000 / dSlope) - dOriginY) / 500.0;
            dFadeOut = (1.0 + sin(((dFadeOut * 2.0) - 1.0) * 3.1415926 * 0.5)) * 0.5;
            return dFadeOut * (3000.0 - (dOriginY * dSlope)) + dFadeOut * dTmp;
          }
        } else {
          return (3000.0 - (dOriginY * dSlope)) + dTmp;
        }
      }
    case JUMP_LAND:
      {
        double dSlope = 0.5;
        double dOriginY = dY - dYOffset;
        double dExtraSin = sin((dOriginY / 6000.0) * 48 * 3.141592654) * 20.0;
        dX /= 50.0;
        dY /= (80.0 * 1.2);
        dTmp = 
          (PerlinNoise::PerlinNoise_2D((dX + 7.0) / (100.0 / 33.7), 
                                       dY         / (100.0 / 33.7), 
                                       3, // was 3 and 2
                                       cdPercistence)) * 1.8;
        if(dOriginY < 300.0) {
          if(dOriginY < 20.0) {
            return 3000.0;
          } else {
            double dFadeIn = (dOriginY - 20.0) / 280.0;
            dFadeIn = (1.0 + sin(((dFadeIn * 2.0) - 1.0) * 3.1415926 * 0.5)) * 0.5;
            return (3000.0 - dFadeIn * (dExtraSin + dOriginY * dSlope)) + dFadeIn * (60.0 * pow(dTmp + 0.75, 2));
          }
        } else if(dOriginY > (3000 / dSlope - 500.0)) {
          if(dOriginY > (3000 / dSlope)) {
            return 0.0;
          } else {
            double dFadeOut = ((3000 / dSlope) - dOriginY) / 500.0;
            dFadeOut = (1.0 + sin(((dFadeOut * 2.0) - 1.0) * 3.1415926 * 0.5)) * 0.5;
            return dFadeOut * (3000.0 - (dExtraSin + dOriginY * dSlope)) + dFadeOut * (60.0 * pow(dTmp + 0.75, 2));
          }
        } else {
          return (3000.0 - (dExtraSin + dOriginY * dSlope)) + 60.0 * pow(dTmp + 0.75, 2);
        }
      }
    case THE_BIG_DROP:
      {
        double dSlope = 0.5;
        double dOriginY = dY - dYOffset;
        double dExtraSin = (pow(sin((dOriginY / 6000.0) * 8 * 3.141592654), 1.0) + 5.0) * 70.0;
        dX /= 50.0;
        dY /= (80.0 * 1.2);
        dTmp = 
          (PerlinNoise::PerlinNoise_2D((dX + 7.0) / (100.0 / 33.7), 
                                       dY         / (100.0 / 33.7), 
                                       3, // was 3 and 2
                                       cdPercistence)) * 1.8;
        if(dOriginY < 300.0) {
          if(dOriginY < 20.0) {
            return 3000.0;
          } else {
            double dFadeIn = (dOriginY - 20.0) / 280.0;
            dFadeIn = (1.0 + sin(((dFadeIn * 2.0) - 1.0) * 3.1415926 * 0.5)) * 0.5;
            return (3000.0 - dFadeIn * (dExtraSin + dOriginY * dSlope)) + dFadeIn * (60.0 * pow(dTmp + 0.75, 2));
          }
        } else if(dOriginY > (3000 / dSlope - 500.0)) {
          if(dOriginY > (3000 / dSlope)) {
            return 0.0;
          } else {
            double dFadeOut = ((3000 / dSlope) - dOriginY) / 500.0;
            dFadeOut = (1.0 + sin(((dFadeOut * 2.0) - 1.0) * 3.1415926 * 0.5)) * 0.5;
            return dFadeOut * (3000.0 - (dExtraSin + dOriginY * dSlope)) + dFadeOut * (60.0 * pow(dTmp + 0.75, 2));
          }
        } else {
          return (3000.0 - (dExtraSin + dOriginY * dSlope)) + 60.0 * pow(dTmp + 0.75, 2);
        }
      }
    case VALLEY_ALLEY:
      {
        double dSlope = 0.5;
        double dOriginY = dY - dYOffset;
        // double dExtraSin = sin((dOriginY / 6000.0) * 15 * 3.141592654) * cos((dX / 6000.0) * 20 * 3.141592654) * 30.0; // 20.0;
        double dExtraSin = (PerlinNoise::PerlinNoise_2D((dX / 6000.0) * 10 * 3.141592654, 
                                                        (dY / 6000.0) * 10 * 3.141592654, 
                                                        2, // was 3 and 2
                                                        cdPercistence)) * 100.0;
        dX /= 50.0;
        dY /= (80.0 * 1.2);
        dTmp = 
          (PerlinNoise::PerlinNoise_2D((dX + 7.0) / (100.0 / 46.7), 
                                       dY         / (100.0 / 30.7), 
                                       3, // was 3 and 2
                                       cdPercistence)) + 0.8;
        if(dOriginY < 300.0) {
          if(dOriginY < 20.0) {
            return 3000.0;
          } else {
            double dFadeIn = (dOriginY - 20.0) / 280.0;
            dFadeIn = (1.0 + sin(((dFadeIn * 2.0) - 1.0) * 3.1415926 * 0.5)) * 0.5;
            double d = 100.0 * pow(dTmp, 40.0);
            if(d > 100.0) {
              d = 100.0;
            }
            return (3000.0 - dFadeIn * (dExtraSin + dOriginY * dSlope)) + dFadeIn * d;
          }
        } else if(dOriginY > (3000 / dSlope - 500.0)) {
          if(dOriginY > (3000 / dSlope)) {
            return 0.0;
          } else {
            double dFadeOut = ((3000 / dSlope) - dOriginY) / 500.0;
            dFadeOut = (1.0 + sin(((dFadeOut * 2.0) - 1.0) * 3.1415926 * 0.5)) * 0.5;
            double d = 100.0 * pow(dTmp, 40.0);
            if(d > 100.0) {
              d = 100.0;
            }
            return dFadeOut * (3000.0 - (dExtraSin + dOriginY * dSlope)) + dFadeOut * d;
          }
        } else {
          double d = 100.0 * pow(dTmp, 40.0);
          if(d > 100.0) {
            d = 100.0;
          }
          return (3000.0 - (dExtraSin + dOriginY * dSlope)) + d;
        }
      }
    case ALLEY_LAND:
      {
        double dSlope = 0.5;
        double dOriginY = dY - dYOffset;
        double dExtraSin = sin((dOriginY / 6000.0) * 40 * 3.141592654) * cos((dX / 6000.0) * 40 * 3.141592654) * 10.0; // 20.0;
        dX /= 50.0;
        dY /= (80.0 * 1.2);
        dTmp = 
          (PerlinNoise::PerlinNoise_2D((dX + 7.0) / (100.0 / 23.7), 
                                       dY         / (100.0 / 23.7), 
                                       3, // was 3 and 2
                                       cdPercistence * 2.0)) * 1.5;
        if(dOriginY < 300.0) {
          if(dOriginY < 20.0) {
            return 3000.0;
          } else {
            double dFadeIn = (dOriginY - 20.0) / 280.0;
            dFadeIn = (1.0 + sin(((dFadeIn * 2.0) - 1.0) * 3.1415926 * 0.5)) * 0.5;
            return (3000.0 - dFadeIn * (dExtraSin + dOriginY * dSlope)) + dFadeIn * (30.0 * pow(dTmp + 0.75, 3.0));
          }
        } else if(dOriginY > (3000 / dSlope - 500.0)) {
          if(dOriginY > (3000 / dSlope)) {
            return 0.0;
          } else {
            double dFadeOut = ((3000 / dSlope) - dOriginY) / 500.0;
            dFadeOut = (1.0 + sin(((dFadeOut * 2.0) - 1.0) * 3.1415926 * 0.5)) * 0.5;
            return dFadeOut * (3000.0 - (dExtraSin + dOriginY * dSlope)) + dFadeOut * (30.0 * pow(dTmp + 0.75, 3.0));
          }
        } else {
          return (3000.0 - (dExtraSin + dOriginY * dSlope)) + 30.0 * pow(dTmp + 0.75, 3.0);
        }
      }
    case HOLES_IN_THE_CRUST:
      {
        double dSlope = 0.5;
        double dOriginY = dY - dYOffset;
        dX /= 50.0;
        dY /= (80.0 * 1.2);
        dTmp = 
          (PerlinNoise::PerlinNoise_2D(dX / (100.0 / 27.7), 
                                       dY / (100.0 / 27.7), 
                                       2, // was 3 and 2
                                       cdPercistence)) * 1.4;

        dTmp2 = 
          (PerlinNoise::PerlinNoise_2D(dX / (100.0 / 48.7), 
                                       dY / (100.0 / 77.7), 
                                       2, // was 3 and 2
                                       cdPercistence)) * 1.8;

        dTmp2 += 0.4;
        if(dTmp2 > 0.0) {
          dTmp2 = 0.0;
        }
        dTmp2 *= 15.0;

        if(dOriginY < 300.0) {
          if(dOriginY < 20.0) {
            return 3000.0;
          } else {
            double dFadeIn = (dOriginY - 20.0) / 280.0;
            dFadeIn = (1.0 + sin(((dFadeIn * 2.0) - 1.0) * 3.1415926 * 0.5)) * 0.5;
            return (3000.0 - dFadeIn * dOriginY * dSlope) + dFadeIn * (30.0 * (dTmp2 + pow(dTmp + 0.75, 2)));
          }
        } else if(dOriginY > (3000 / dSlope - 500.0)) {
          if(dOriginY > (3000 / dSlope)) {
            return 0.0;
          } else {
            double dFadeOut = ((3000 / dSlope) - dOriginY) / 500.0;
            dFadeOut = (1.0 + sin(((dFadeOut * 2.0) - 1.0) * 3.1415926 * 0.5)) * 0.5;
            return dFadeOut * (3000.0 - dOriginY * dSlope) + dFadeOut * (30.0 * (dTmp2 + pow(dTmp + 0.75, 2)));
          }
        } else {
          return (3000.0 - dOriginY * dSlope) + 30.0 * (dTmp2 + pow(dTmp + 0.75, 2));
        }
      }
  }
  return 0.0;
}


BVector HeightMap::NormalAt(double dX, double dY) {
  BVector vNormal(0, 0, 0);
  BVector vRef(0, 1, 0);
  BVector vToNext(1, 0, HeightMap::HeightAt(dX + 1, dY) - HeightMap::HeightAt(dX, dY));
  vToNext.ToUnitLength();
  vNormal += vRef.CrossProduct(vToNext);

  vRef.Set(0, -1, 0);
  vToNext.Set(-1, 0, HeightMap::HeightAt(dX - 1, dY) - HeightMap::HeightAt(dX, dY));
  vToNext.ToUnitLength();
  vNormal += vRef.CrossProduct(vToNext);

  vRef.Set(1, 0, 0);
  vToNext.Set(0, 1, HeightMap::HeightAt(dX, dY - 1) - HeightMap::HeightAt(dX, dY));
  vToNext.ToUnitLength();
  vNormal += vRef.CrossProduct(vToNext);

  vRef.Set(-1, 0, 0);
  vToNext.Set(0, -1, HeightMap::HeightAt(dX, dY + 1) - HeightMap::HeightAt(dX, dY));
  vToNext.ToUnitLength();
  vNormal += vRef.CrossProduct(vToNext);

  vNormal.ToUnitLength();

  return vNormal;
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
