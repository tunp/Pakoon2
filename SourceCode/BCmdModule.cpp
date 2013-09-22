//
// BCmdModule: Internal command interface to control gameplay
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#include "stdafx.h"
#include "BCmdModule.h"
#include "BVehicle.h"
#include "BGame.h"
#include "BNavSatWnd.h"
#include "BServiceWnd.h"


//*****************************************************************************
//** COMMAND HANDLERS                                                        **
//*****************************************************************************

//*****************************************************************************
void PureHelp(BSimulation *pSim, CString sParams, bool bHelp) {
  if(bHelp) {
    BGame::GetServiceWnd()->Output("YOU DON'T NEED HELP FOR HELP.            ");
    BGame::GetServiceWnd()->Output("JUST TYPE HELP.                          ");
    return;
  }
}

//*****************************************************************************
void ListCommands(BSimulation *pSim, CString sParams, bool bHelp) {
  if(bHelp) {
    BGame::GetServiceWnd()->Output("LISTS ALL AVAILABLE COMMANDS.            ");
    return;
  }
  BCmdHandler *p = BGame::Command()->m_pHandlers;
  while(p) {
    BGame::GetServiceWnd()->Output(p->m_sCommand);
    if(p->m_pNext) {
      BGame::GetServiceWnd()->Output(", ");
    }
    p = p->m_pNext;
  }
}


//*****************************************************************************
void ToggleJet(BSimulation *pSim, CString sParams, bool bHelp) {
  if(bHelp) {
    BGame::GetServiceWnd()->Output("ACTIVATES/DEACTIVATES THE AUXILIARY      ");
    BGame::GetServiceWnd()->Output("JET BOOST. USE CAUTION WITH THE POWERFUL ");
    BGame::GetServiceWnd()->Output("JET. JET CONSUMES A LOT OF KEROSINE.");
    return;
  }
  // Turn jet mode on/off
  if(pSim->GetVehicle()->m_jet.m_nJetMode) {
    pSim->GetVehicle()->m_jet.m_bJetModeActivating = !pSim->GetVehicle()->m_jet.m_bJetModeActivating;
  } else {
    pSim->GetVehicle()->m_jet.m_nJetMode = 1;
    pSim->GetVehicle()->m_jet.m_bJetModeActivating = true;
  }
  if(pSim->GetVehicle()->m_jet.m_bJetModeActivating) {
    BGame::GetServiceWnd()->Output("JET MODE ACTIVATED");
  } else {
    BGame::GetServiceWnd()->Output("JET MODE DEACTIVATED");
  }
}


//*****************************************************************************
void ToggleNavSat(BSimulation *pSim, CString sParams, bool bHelp) {
  if(bHelp) {
    BGame::GetServiceWnd()->Output("DISPLAYES/HIDES THE NAVIGATION SATELLITE ");
    BGame::GetServiceWnd()->Output("(NAVSAT) PANEL. USE THE HANDLE TO SET    ");
    BGame::GetServiceWnd()->Output("THE SIZE OF THE IMAGE.");
    return;
  }
  BGame::m_bNavSat = !BGame::m_bNavSat;
  if(BGame::m_bNavSat) {
    // Start navsat
    BGame::GetNavSat()->StartTracking(BGame::GetSimulation()->GetVehicle()->m_vLocation);
    BGame::GetServiceWnd()->Output("NAVSAT TRACKING ACTIVATED");
  } else {
    // End navsat
    BGame::GetNavSat()->EndTracking();
  }
}

//*****************************************************************************
void ToggleService(BSimulation *pSim, CString sParams, bool bHelp) {
  if(bHelp) {
    BGame::GetServiceWnd()->Output("DISPLAYES/HIDES THIS PANEL.              ");
    return;
  }
  BGame::m_bService = !BGame::m_bService;
}


//*****************************************************************************
void Exit(BSimulation *pSim, CString sParams, bool bHelp) {
  if(bHelp) {
    BGame::GetServiceWnd()->Output("HIDES THIS PANEL.");
    return;
  }
  ToggleService(pSim, sParams, bHelp);
}


//*****************************************************************************
void SetNavSatResolution(BSimulation *pSim, CString sParams, bool bHelp) {
  if(bHelp) {
    BGame::GetServiceWnd()->Output("\"SET NAVSAT RESOLUTION <RES>\"            ", 0, 1, 0);
    BGame::GetServiceWnd()->Output("SETS THE SIZE OF THE AREA SHOWN IN THE   ");
    BGame::GetServiceWnd()->Output("NAVSAT PANEL. PARAMETER <RES> IS GIVEN IN");
    BGame::GetServiceWnd()->Output("METERS.");
    return;
  }
  double dRes;
  if(sscanf(LPCTSTR(sParams), "%lf", &dRes) == 1) {
    BGame::GetNavSat()->SetResolution(dRes);
    BGame::GetServiceWnd()->Output("NAVSAT TRACKING RESOLUTION CHANGED");
  } else {
    BGame::GetServiceWnd()->Output("SYNTAX ERROR: ILLEGAL RESOLUTION", 0.75, 0, 0);
  }
}








//*****************************************************************************
//** FRAMEWORK                                                               **
//*****************************************************************************

//*****************************************************************************
BCmdModule::BCmdModule() {
  m_pSim = 0;
  m_pHandlers = 0;

  // Register command handlers
  RegisterCmdFunction("TOGGLE JET", 0, ToggleJet);
  RegisterCmdFunction("TOGGLE NAVSAT", 0, ToggleNavSat);
  RegisterCmdFunction("TOGGLE SERVICE", 0, ToggleService);
  RegisterCmdFunction("HELP", 0, PureHelp);
  RegisterCmdFunction("LIST COMMANDS", 0, ListCommands);
  RegisterCmdFunction("SET NAVSAT RESOLUTION", 0, SetNavSatResolution);
  RegisterCmdFunction("EXIT", 0, Exit);
}



//*****************************************************************************
void BCmdModule::RegisterCmdFunction(CString sCommand, 
                                     int nParams, 
                                     void (*pfnHandler)(BSimulation *pSim, CString sParams, bool bHelp)) {
  // Add a new command handler
  BCmdHandler *p = m_pHandlers;
  if(!p) {
    // Add first
    p = new BCmdHandler;
    m_pHandlers = p;
  } else {
    // Add to the tail of the list
    while(p->m_pNext) {
      p = p->m_pNext;
    }
    BCmdHandler *pNew = new BCmdHandler;
    p->m_pNext = pNew;
    p = p->m_pNext;
  }
  p->m_pNext = 0;
  p->m_pfnHandler = pfnHandler;
  p->m_sCommand = sCommand;
}

//*****************************************************************************
bool BCmdModule::StringsExactlySame(CString s1, CString sGiven) {
  return(s1.CompareNoCase(sGiven) == 0);
}

//*****************************************************************************
bool BCmdModule::StringsPartiallySame(CString s1, CString sGiven, int &rnParamStart) {
  // Check if identical
  if(StringsExactlySame(s1, sGiven)) {
    return true;
  }
  // Check if correctly abbreviated
  int i1 = 0, i2 = 0;

  // Eat away leading spaces
  while((i2 < sGiven.GetLength()) && 
        (sGiven.GetAt(i2) == ' ')) {
    ++i2;
  }
  if(i2 >= sGiven.GetLength()) {
    return false;
  }
  while((i1 < s1.GetLength()) && 
    (i2 < sGiven.GetLength())) {
    if(sGiven.GetAt(i2) != s1.GetAt(i1)) {
      if(sGiven.GetAt(i2) == ' ') {
        // Skip to next words
        while((i2 < sGiven.GetLength()) && 
              (sGiven.GetAt(i2) == ' ')) {
          ++i2;
        }
        while((i1 < s1.GetLength()) && 
          (s1.GetAt(i1) != ' ')) {
          ++i1;
        }
        while((i1 < s1.GetLength()) && 
          (s1.GetAt(i1) == ' ')) {
          ++i1;
        }
      } else {
        return false;
      }
    } else {
      ++i2;
      ++i1;
    }
  }
  while((i2 < sGiven.GetLength()) && 
        (sGiven.GetAt(i2) == ' ')) {
    ++i2;
  }
  rnParamStart = i2;
  return true;
}


//*****************************************************************************
BCmdHandler *BCmdModule::FindCommand(CString sCommand, int &rnParamStart) {
  BCmdHandler *p = m_pHandlers;
  // First look for an exact match
  while(p && !StringsExactlySame(p->m_sCommand, sCommand)) {
    p = p->m_pNext;
  }
  if(!p) {
    // Then look for an abbreviated match
    p = m_pHandlers;
    while(p && !StringsPartiallySame(p->m_sCommand, sCommand, rnParamStart)) {
      p = p->m_pNext;
    }    
  } else {
    rnParamStart = p->m_sCommand.GetLength() + 1;
  }
  return p;
}



//*****************************************************************************
void BCmdModule::Help(BSimulation *pSim, CString sParams) {
  // BGame::GetServiceWnd()->Output("12345678901234567890123456789012345678901");
  int nTmp;
  if(!sParams.IsEmpty()) {
    BCmdHandler *pCommand = FindCommand(sParams, nTmp);
    if(pCommand) {
      (*(pCommand->m_pfnHandler))(m_pSim, sParams, true);
      return;
    } else {
      CString sError;
      sError.Format("THERE'S NO COMMAND \"%s\". TO SEE ALL COMMANDS, TYPE \"LIST COMMANDS\"", sParams);
      BGame::GetServiceWnd()->Output(sError, 0.75, 0, 0);
      return;
    }
  }
  BGame::GetServiceWnd()->Output("TO SEE ALL COMMANDS, TYPE \"LIST COMMANDS\"");
  BGame::GetServiceWnd()->Output("TO GET HELP ON INDIVIDUAL COMMAND, TYPE  ");
  BGame::GetServiceWnd()->Output("\"HELP XYZ\" (WHERE XYZ IS THE COMMAND)    ");
  BGame::GetServiceWnd()->Output("ALL COMMANDS CAN BE ABBREVIATED AS LONG  ");
  BGame::GetServiceWnd()->Output("AS THEY ARE UNAMBIGUOUS                  ");
  BGame::GetServiceWnd()->Output("(E.G. \"LI CO\" =  \"LIST COMMANDS\")        ");
}


//*****************************************************************************
void BCmdModule::Run(CString sCommand) {
  sCommand.MakeUpper();
  int nParamStart = 0;
  BCmdHandler *pCommand = FindCommand(sCommand, nParamStart);
  if(pCommand) {
    // Run command
    CString sParams = "";
    if(nParamStart < sCommand.GetLength()) {
      sParams = LPCTSTR(sCommand) + nParamStart;
    }
    if(pCommand->m_sCommand.Compare("HELP") == 0) {
      // Help is special command that needs access to other commands.
      // That's why it's handled within the class
      Help(m_pSim, sParams);
    } else {
      (*(pCommand->m_pfnHandler))(m_pSim, sParams);
    }
  } else {
    // Report command not found
    if(!sCommand.IsEmpty()) {
      CString sError;
      sError.Format("UNKNOWN COMMAND: <%s>", sCommand);
      BGame::GetServiceWnd()->Output(sError, 0.75, 0, 0);
    }
  }
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