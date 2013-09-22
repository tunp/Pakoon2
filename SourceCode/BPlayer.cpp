//
// BPlayer: Player
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#include "stdafx.h"
#include "BPlayer.h"
#include "BGame.h"
#include "FileIOHelpers.h"


//*************************************************************************************************
BPlayer::BPlayer() {
  m_dCash = 50.0;
  m_dFuel = 25.0;
  m_dKerosine = 25.0;
  m_sValidVehicles = ">Bogian<";
  m_sSceneInfo = "";
}


//*************************************************************************************************
void BPlayer::SaveStateFile() {
  // Write in INI file format
  CString s;
  s.Format("%.1lf", m_dCash);
  FileHelpers::WriteKeyStringToINIFile("State", "Cash", s, "./Player.state");
  s.Format("%.1lf", m_dFuel);
  FileHelpers::WriteKeyStringToINIFile("State", "Fuel", s, "./Player.state");
  s.Format("%.1lf", m_dKerosine);
  FileHelpers::WriteKeyStringToINIFile("State", "Kerosine", s, "./Player.state");
  FileHelpers::WriteKeyStringToINIFile("State", "Vehicles", m_sValidVehicles, "./Player.state");
  FileHelpers::WriteKeyStringToINIFile("State", "Checksum", BGame::GetScrambleChecksum(), "./Player.state");
}

//*************************************************************************************************
void BPlayer::LoadStateFile() {
  FileHelpers::GetKeyDoubleFromINIFile("State", "Cash", 0.0, m_dCash, "./Player.state");
  FileHelpers::GetKeyDoubleFromINIFile("State", "Fuel", 0.0, m_dFuel, "./Player.state");
  FileHelpers::GetKeyDoubleFromINIFile("State", "Kerosine", 0.0, m_dKerosine, "./Player.state");
  FileHelpers::GetKeyStringFromINIFile("State", "Vehicles", "", m_sValidVehicles, "./Player.state");
  CString sChecksum;
  FileHelpers::GetKeyStringFromINIFile("State", "Checksum", "", sChecksum, "./Player.state");

  if(sChecksum.Compare(BGame::GetScrambleChecksum()) != 0) {
    // File has been tampered with, ask for info
    BGame::MyAfxMessageBox("Player.state file checksum doesn't match!");
    if(AfxMessageBox("The Player.State file checksum doesn't match.\n" 
                     "Note that you are not allowed to edit the state file.\n" 
                     "DO YOU WANT TO EXIT THE GAME?\n" 
                     "(If you answer No, player state will be reset to default values.)" , MB_YESNO) == IDYES) {
      BGame::MyAfxMessageBox("Exiting game.");
      BGame::m_bQuitPending = true;
    } else {
      BGame::MyAfxMessageBox("Resetting the player state to default values.");
      m_dCash = 1000.0;
      // m_dCash = 50.0;
      m_dFuel = 25.0;
      m_dKerosine = 25.0;
      m_sValidVehicles = ">Bogian<";
      SaveStateFile();
    }
  }

  m_dFuel = 100.0;
  m_dKerosine = 100.0;

}


//*************************************************************************************************
void BPlayer::SaveCurrentSceneInfo() {
  BScene   *pScene   = BGame::GetSimulation()->GetScene();
  BVehicle *pVehicle = BGame::GetSimulation()->GetVehicle();
  CString sSceneInfo;
  sSceneInfo.Format("%.2lf %.2lf %.2lf",
                    pVehicle->m_vLocation.m_dX,
                    pVehicle->m_vLocation.m_dY,
                    pVehicle->m_vLocation.m_dZ); 
  FileHelpers::WriteKeyStringToINIFile("State", pScene->m_sName, sSceneInfo, "./Player.state");
}


//*************************************************************************************************
bool BPlayer::LoadCurrentSceneInfo(BVector &rvVehicleLoc) {
  bool bRet = false;
  BScene *pScene = BGame::GetSimulation()->GetScene();
  CString sSceneInfo;
  FileHelpers::GetKeyStringFromINIFile("State", pScene->m_sName, "default", sSceneInfo, "./Player.state");
  if(sSceneInfo.CompareNoCase("default") != 0) {
    // Fetch scene info from string. The string is in format
    // "x y z", where x y z is vehicle location
    if(sSceneInfo.GetLength() >= 12) {
      // Fetch vehicle location
      BVector vVehicleLoc(0, 0, 0);

      sscanf(LPCTSTR(sSceneInfo), 
             "%lf %lf %lf", 
             &(vVehicleLoc.m_dX), 
             &(vVehicleLoc.m_dY), 
             &(vVehicleLoc.m_dZ));
      if((vVehicleLoc - BVector(-9999.9, -9999.9, -9999.9)).Length() > 0.1) {
        bRet = true;
        rvVehicleLoc = vVehicleLoc;
      }
    }
  }
  return false; // bRet;
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