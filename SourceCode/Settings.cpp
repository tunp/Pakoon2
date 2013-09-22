//
// All access settings module for all users of the game
// 
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#include "stdafx.h"
#include "Settings.h"
#include "SoundModule.h"
#include "BGame.h"

CString Settings::m_sFilename = _T(".\\settings.dat");
CString Settings::m_sBrowserPath = _T("c:\\program files\\internet explorer\\iexplore.exe");

extern bool g_cbBlackAndWhite;
extern bool g_cbMipMap;


void Settings::ReadSettings(BSimulation *pSimulation) {
  char  sComputerName[MAX_COMPUTERNAME_LENGTH + 1];
  DWORD dwLen = MAX_COMPUTERNAME_LENGTH + 1;
  if(GetComputerName(sComputerName, &dwLen)) {
    m_sFilename = ".\\";
    m_sFilename += sComputerName;
    m_sFilename += "settings.dat";
  }
  FILE *fp;
  fp = fopen(m_sFilename, "r");
  if(fp) {
    char sLine[1024];
    fgets(sLine, 1024, fp);
    sscanf(sLine, "ScreenRes = %d, %d, %d, %d", 
      &(BGame::m_nDispWidth),
      &(BGame::m_nDispHeight),
      &(BGame::m_nDispBits),
      &(BGame::m_nDispHz));
    fgets(sLine, 1024, fp);
    sscanf(sLine, "TerrainResolution = %d", &(BGame::m_nTerrainResolution));
    fgets(sLine, 1024, fp);
    sscanf(sLine, "DustAndClouds = %d", &(BGame::m_nDustAndClouds));
    fgets(sLine, 1024, fp);
    sscanf(sLine, "MusicVolume = %d", &(BGame::m_nMusicVolume));
    fgets(sLine, 1024, fp);
    sscanf(sLine, "VehicleVolume = %d", &(BGame::m_nVehicleVolume));
    fgets(sLine, 1024, fp);
    sscanf(sLine, "SoundSpace = %d", &(BGame::m_nSoundscape));
    SoundModule::SetSoundSpace(1 - BGame::m_nSoundscape);
    fgets(sLine, 1024, fp);
    sscanf(sLine, "SkyDetail = %d", &(BGame::m_nSkyDetail));
    fgets(sLine, 1024, fp);
    sscanf(sLine, "DistantDetail = %d", &(BGame::m_nDistantDetail));
    fgets(sLine, 1024, fp);
    sscanf(sLine, "WaterDetail = %d", &(BGame::m_nWaterDetail));
    fgets(sLine, 1024, fp);
    sscanf(sLine, "ColorMode = %d", &(BGame::m_nColorMode));
    fgets(sLine, 1024, fp);
    sscanf(sLine, "ScreenFormat = %d", &(BGame::m_nScreenFormat));
    fgets(sLine, 1024, fp);
    sscanf(sLine, "TextureSmoothness = %d", &(BGame::m_nTextureSmoothness));
    g_cbMipMap = (BGame::m_nTextureSmoothness == 2);

    int nTmp;
    fgets(sLine, 1024, fp);
    sscanf(sLine, "MenuMusicVolume = %d", &nTmp);
    SoundModule::SetMenuMusicVolume(nTmp);
    fgets(sLine, 1024, fp);
    sscanf(sLine, "VehicleSoundsVolume = %d", &nTmp);
    SoundModule::SetVehicleSoundsVolume(nTmp);
    fgets(sLine, 1024, fp);
    sscanf(sLine, "MessageSoundVolume = %d", &nTmp);
    SoundModule::SetMessageSoundVolume(nTmp);

    fgets(sLine, 1024, fp);
    sscanf(sLine, "ControllerID = %d", &BGame::m_nController);

    fgets(sLine, 1024, fp);
    sscanf(sLine, "KEYMAP-Left = %u", &ControllerModule::m_keymap.m_unLeft);
    fgets(sLine, 1024, fp);
    sscanf(sLine, "KEYMAP-Right = %u", &ControllerModule::m_keymap.m_unRight);
    fgets(sLine, 1024, fp);
    sscanf(sLine, "KEYMAP-Accelerate = %u", &ControllerModule::m_keymap.m_unAccelerate);
    fgets(sLine, 1024, fp);
    sscanf(sLine, "KEYMAP-Reverse = %u", &ControllerModule::m_keymap.m_unReverse);
    fgets(sLine, 1024, fp);
    sscanf(sLine, "KEYMAP-Break = %u", &ControllerModule::m_keymap.m_unBreak);
    fgets(sLine, 1024, fp);
    sscanf(sLine, "KEYMAP-Jet = %u", &ControllerModule::m_keymap.m_unJet);
    fgets(sLine, 1024, fp);
    sscanf(sLine, "KEYMAP-Heli = %u", &ControllerModule::m_keymap.m_unHeli);
    fgets(sLine, 1024, fp);
    sscanf(sLine, "KEYMAP-Lift = %u", &ControllerModule::m_keymap.m_unLift);
    fgets(sLine, 1024, fp);
    sscanf(sLine, "KEYMAP-Camera = %u", &ControllerModule::m_keymap.m_unCamera);

    fgets(sLine, 1024, fp);
    ControllerModule::m_controllermap.m_cmiLeft.FromString(sLine + 27);
    fgets(sLine, 1024, fp);
    ControllerModule::m_controllermap.m_cmiRight.FromString(sLine + 27);
    fgets(sLine, 1024, fp);
    ControllerModule::m_controllermap.m_cmiAccelerate.FromString(sLine + 27);
    fgets(sLine, 1024, fp);
    ControllerModule::m_controllermap.m_cmiReverse.FromString(sLine + 27);
    fgets(sLine, 1024, fp);
    ControllerModule::m_controllermap.m_cmiBreak.FromString(sLine + 27);
    fgets(sLine, 1024, fp);
    ControllerModule::m_controllermap.m_cmiJet.FromString(sLine + 27);
    fgets(sLine, 1024, fp);
    ControllerModule::m_controllermap.m_cmiHeli.FromString(sLine + 27);
    fgets(sLine, 1024, fp);
    ControllerModule::m_controllermap.m_cmiLift.FromString(sLine + 27);
    fgets(sLine, 1024, fp);
    ControllerModule::m_controllermap.m_cmiCamera.FromString(sLine + 27);

    g_cbBlackAndWhite = (BGame::m_nColorMode == 0);
    fgets(sLine, 1024, fp);
    sscanf(sLine, "WaterSurface = %d", &(BGame::m_nWaterSurface));

    fgets(sLine, 1024, fp);
    if(strlen(sLine) > strlen("PlayerName = ")) {
      sLine[strlen(sLine) - 1] = '\0'; // eat up the newline from end
    }
    CString sTmp = sLine + strlen("PlayerName = ");
    BGame::GetMultiplay()->m_params.m_sPlayerName = sTmp;
    fgets(sLine, 1024, fp);
    BGame::GetMultiplay()->m_params.m_bHost = (sLine[strlen("Role = ")] == 'S');
    fgets(sLine, 1024, fp);
    if(strlen(sLine) > strlen("HostIP = ")) {
      sLine[strlen(sLine) - 1] = '\0'; // eat up the newline from end
    }
    sTmp = sLine + strlen("HostIP = ");
    BGame::GetMultiplay()->m_params.m_sHostIPAddress = sTmp;

    fgets(sLine, 1024, fp);
    sscanf(sLine, "CarDetails = %d", &(BGame::m_nCarDetails));

    fgets(sLine, 1024, fp);
    sscanf(sLine, "MultiplayPortNumber = %d", &(BGame::m_nMultiplayPort));

    fclose(fp);
  } else {
    CString sTmp;
    sTmp.Format("Cannot open %s. Using default settings.", m_sFilename);
    BGame::MyAfxMessageBox(sTmp, MB_OK);
  }
}

void Settings::WriteSettings(BSimulation *pSimulation) {
  char  sComputerName[MAX_COMPUTERNAME_LENGTH + 1];
  DWORD dwLen = MAX_COMPUTERNAME_LENGTH + 1;
  if(GetComputerName(sComputerName, &dwLen)) {
    m_sFilename = ".\\";
    m_sFilename += sComputerName;
    m_sFilename += "settings.dat";
  }
  FILE *fp;
  fp = fopen(m_sFilename, "w");
  if(fp) {
    fprintf(fp, "ScreenRes = %d, %d, %d, %d\n", 
            BGame::m_nDispWidth,
            BGame::m_nDispHeight,
            BGame::m_nDispBits,
            BGame::m_nDispHz);
    fprintf(fp, "TerrainResolution = %d\n", BGame::m_nTerrainResolution);
    fprintf(fp, "DustAndClouds = %d\n", BGame::m_nDustAndClouds);
    fprintf(fp, "MusicVolume = %d\n", BGame::m_nMusicVolume);
    fprintf(fp, "VehicleVolume = %d\n", BGame::m_nVehicleVolume);
    fprintf(fp, "SoundSpace = %d\n", BGame::m_nSoundscape);
    fprintf(fp, "SkyDetail = %d\n", BGame::m_nSkyDetail);
    fprintf(fp, "DistantDetail = %d\n", BGame::m_nDistantDetail);
    fprintf(fp, "WaterDetail = %d\n", BGame::m_nWaterDetail);
    fprintf(fp, "ColorMode = %d\n", BGame::m_nColorMode);
    fprintf(fp, "ScreenFormat = %d\n", BGame::m_nScreenFormat);
    fprintf(fp, "TextureSmoothness = %d\n", BGame::m_nTextureSmoothness);

    fprintf(fp, "MenuMusicVolume = %d\n", SoundModule::GetMenuMusicVolume());
    fprintf(fp, "VehicleSoundsVolume = %d\n", SoundModule::GetVehicleSoundsVolume());
    fprintf(fp, "MessageSoundVolume = %d\n", SoundModule::GetMessageSoundVolume());

    fprintf(fp, "ControllerID = %d\n", BGame::m_nController);

    fprintf(fp, "KEYMAP-Left = %u\n", ControllerModule::m_keymap.m_unLeft);
    fprintf(fp, "KEYMAP-Right = %u\n", ControllerModule::m_keymap.m_unRight);
    fprintf(fp, "KEYMAP-Accelerate = %u\n", ControllerModule::m_keymap.m_unAccelerate);
    fprintf(fp, "KEYMAP-Reverse = %u\n", ControllerModule::m_keymap.m_unReverse);
    fprintf(fp, "KEYMAP-Break = %u\n", ControllerModule::m_keymap.m_unBreak);
    fprintf(fp, "KEYMAP-Jet = %u\n", ControllerModule::m_keymap.m_unJet);
    fprintf(fp, "KEYMAP-Heli = %u\n", ControllerModule::m_keymap.m_unHeli);
    fprintf(fp, "KEYMAP-Lift = %u\n", ControllerModule::m_keymap.m_unLift);
    fprintf(fp, "KEYMAP-Camera = %u\n", ControllerModule::m_keymap.m_unCamera);

    bool bTmp;
    CString sTmp;
    sTmp = ControllerModule::m_controllermap.m_cmiLeft.ToString(bTmp);
    if(bTmp) { sTmp += " Flipped";}
    fprintf(fp, "CONTROLLERMAP-Left       = %s\n", sTmp);
    sTmp = ControllerModule::m_controllermap.m_cmiRight.ToString(bTmp);
    if(bTmp) { sTmp += " Flipped";}
    fprintf(fp, "CONTROLLERMAP-Right      = %s\n", sTmp);
    sTmp = ControllerModule::m_controllermap.m_cmiAccelerate.ToString(bTmp);
    if(bTmp) { sTmp += " Flipped";}
    fprintf(fp, "CONTROLLERMAP-Accelerate = %s\n", sTmp);
    sTmp = ControllerModule::m_controllermap.m_cmiReverse.ToString(bTmp);
    if(bTmp) { sTmp += " Flipped";}
    fprintf(fp, "CONTROLLERMAP-Reverse    = %s\n", sTmp);
    sTmp = ControllerModule::m_controllermap.m_cmiBreak.ToString(bTmp);
    if(bTmp) { sTmp += " Flipped";}
    fprintf(fp, "CONTROLLERMAP-Break      = %s\n", sTmp);
    sTmp = ControllerModule::m_controllermap.m_cmiJet.ToString(bTmp);
    if(bTmp) { sTmp += " Flipped";}
    fprintf(fp, "CONTROLLERMAP-Jet        = %s\n", sTmp);
    sTmp = ControllerModule::m_controllermap.m_cmiHeli.ToString(bTmp);
    if(bTmp) { sTmp += " Flipped";}
    fprintf(fp, "CONTROLLERMAP-Heli       = %s\n", sTmp);
    sTmp = ControllerModule::m_controllermap.m_cmiLift.ToString(bTmp);
    if(bTmp) { sTmp += " Flipped";}
    fprintf(fp, "CONTROLLERMAP-Lift       = %s\n", sTmp);
    sTmp = ControllerModule::m_controllermap.m_cmiCamera.ToString(bTmp);
    if(bTmp) { sTmp += " Flipped";}
    fprintf(fp, "CONTROLLERMAP-Camera     = %s\n", sTmp);
    fprintf(fp, "WaterSurface = %d\n", BGame::m_nWaterSurface);

    fprintf(fp, "PlayerName = %s\n", LPCTSTR(BGame::GetMultiplay()->m_params.m_sPlayerName));
    fprintf(fp, "Role = %s\n", BGame::GetMultiplay()->m_params.m_bHost ? "Server" : "Client");
    fprintf(fp, "HostIP = %s\n", LPCTSTR(BGame::GetMultiplay()->m_params.m_sHostIPAddress));

    fprintf(fp, "CarDetails = %d\n", BGame::m_nCarDetails);

    fprintf(fp, "MultiplayPortNumber = %d\n", BGame::m_nMultiplayPort);

    fclose(fp);
  } else {
    CString sTmp;
    sTmp.Format("Cannot open %s. Settings were not saved.", m_sFilename);
    BGame::MyAfxMessageBox(sTmp);
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