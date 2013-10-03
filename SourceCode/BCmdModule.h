//
// BCmdModule: Internal command interface to control gameplay
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#pragma once

#include "BaseClasses.h"
#include "BSimulation.h"

#include <string>

using namespace std;

class BCmdModule;

//*****************************************************************************
class BCmdHandler{
public:
  BCmdHandler *m_pNext;
  string      m_sCommand;
  void       (*m_pfnHandler)(BSimulation *pSim, string sParams, bool bHelp);
};

void ListCommands(BSimulation *pSim, string sParams);

//*****************************************************************************
class BCmdModule {
  friend void ListCommands(BSimulation *pSim, string sParams, bool bHelp);
  BSimulation *m_pSim;
  BCmdHandler *m_pHandlers;
  void         RegisterCmdFunction(string sCommand, int nParams, void (*pfnHandler)(BSimulation *pSim, string sParams, bool bHelp));
  bool         StringsExactlySame(string s1, string sGiven);
  bool         StringsPartiallySame(string s1, string sGiven, int &rnParamStart);
  BCmdHandler *FindCommand(string sCommand, int &rnParamStart);
  void         Help(BSimulation *pSim, string sParams);
public:
  BCmdModule();
  void SetSim(BSimulation *pSim) {m_pSim = pSim;};
  void Run(string sCommand);
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
