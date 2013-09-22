//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//
const int    cnMapSize = 128;
const double cdPercistence = 0.6;
const int    cnOctaves = 8;

#pragma once

#include <math.h>
#include "BaseClasses.h"

namespace HeightMap {
  enum    TTerrain {BASIC, 
                    JUMP_LAND, 
                    THE_BIG_DROP,
                    VALLEY_ALLEY,
                    ALLEY_LAND,
                    GRAND_VALLEY, 
                    LAKE_POWELL, 
                    PAINTED_DESERT, 
                    CONTINENTS, 
                    SCOTLAND, 
                    MIXER, 
                    HOLES_IN_THE_CRUST,
                    NOISE};
  void    GenerateMap(double dXOffset, double dYOffset, double dScale);
  double  HeightAt(double dX, double dY);
  double  CalcHeightAt(double dX, double dY, double &rdArea, TTerrain nTerrain = BASIC, int nOctaves = cnOctaves, double dYOffset = 0.0);
  BVector NormalAt(double dX, double dY);
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