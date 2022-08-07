//
// BGame: Center location for controlling the game
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#include "BGame.h"
#include "FileIOHelpers.h"
#include "BTextures.h" 
#include "SoundModule.h"
#include "BMessages.h"
#include "HeightMap.h"
#include "Pakoon1View.h"

#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>

using namespace std;

BGame::TGameMode BGame::m_gameMode = BGame::SLALOM;

BSimulation   BGame::m_simulation;
BPlayer       BGame::m_player;
BCmdModule    BGame::m_cmdModule;
BNavSatWnd    BGame::m_navsatWnd;
BServiceWnd   BGame::m_serviceWnd;
BSceneEditor  BGame::m_sceneEditor;
CPakoon1View *BGame::m_pView = 0;

int     BGame::m_nDispWidth;
int     BGame::m_nDispHeight;
int     BGame::m_nDispBits;
int     BGame::m_nDispHz;
char    BGame::m_cOnScreenInfo;
int     BGame::m_nSkyDetail;
int     BGame::m_nDistantDetail;
#ifdef __EMSCRIPTEN__
int     BGame::m_nTerrainResolution = 5; // SLOW MACHINE
#else
int     BGame::m_nTerrainResolution = 2;
#endif
int     BGame::m_nDustAndClouds;
int     BGame::m_nCarDetails;
int     BGame::m_nWaterSurface = 0;
int     BGame::m_nMusicVolume = 50;
int     BGame::m_nVehicleVolume = 50;
int     BGame::m_nSoundscape;
int     BGame::m_nWaterDetail;
int     BGame::m_nColorMode;
int     BGame::m_nScreenFormat;
int     BGame::m_nTextureSmoothness; 
bool    BGame::m_bNavSat;
bool    BGame::m_bService;
bool    BGame::m_bSlowMotion;
bool    BGame::m_bFrozen;
int     BGame::m_nShowEffects;
bool    BGame::m_bShowQuickHelp;
bool    BGame::m_bDrawOnScreenTracking;
double  BGame::m_dNavSatHandleAngle;
double  BGame::m_dServiceHandleAngle;
unsigned BGame::m_clockFrozenStart = 0;
unsigned BGame::m_clockLastLift = 0;
int     BGame::m_nFreezeRefCount = 0;
int     BGame::m_nPhysicsSteps; 
bool    BGame::m_bShowHint;
unsigned BGame::m_clockHintStart;
int     BGame::m_nYesNoSelection = 1; // Yes by default
bool    BGame::m_bShowGameMenu = false;
bool    BGame::m_bShowCancelQuestion = false;
bool    BGame::m_bSceneEditorMode = true;
bool    BGame::m_bFadingIn = false;
unsigned BGame::m_clockFadeStart = 0;

string BGame::m_sScene = "";
string BGame::m_sVehicle = "";

bool    BGame::m_bMenuMode = true;
BMenu  *BGame::m_pMenuCurrent = 0;
BMenu  *BGame::m_pMenuPrevious = 0;
BMenu   BGame::m_menuMain;
BMenu   BGame::m_menuMultiplay;
BMenu   BGame::m_menuChooseGameMode;
BMenu   BGame::m_menuChooseScene;
BMenu   BGame::m_menuChooseVehicle;
BMenu   BGame::m_menuSettings;
BMenu   BGame::m_menuCredits;
BMenu   BGame::m_menuHiscores;
BMenu   BGame::m_menuPrecachingTerrain;
bool    BGame::m_bMenusCreated = false;
bool    BGame::m_bSettingsFromGame = false;

BMenu   BGame::m_menuGame;

bool    BGame::m_bGameLoading = false;
bool    BGame::m_bGameReadyToStart = false;
bool    BGame::m_bQuitPending = false;

SDL_mutex *BGame::m_csMutex;
double  BGame::m_dProgressMax = 1.0;
double  BGame::m_dProgressPos = 0.0;

bool   BGame::m_bJumpToHome = false;

bool   BGame::m_bBuyingVehicle = false;
bool   BGame::m_bCannotBuyVehicle = false;
double BGame::m_dPurchasePrice = 100.0;

bool    BGame::m_bMultiProcessor = false;

bool    BGame::m_bAnalyzerMode = false;
unsigned BGame::m_clockAnalyzerStarted = 0;
int     BGame::m_nVisualize = 255; // all on

bool        BGame::m_bRaceStarted;
bool        BGame::m_bRaceFinished;
double      BGame::m_dRaceTime;
double      BGame::m_dAirTime;

bool        BGame::m_bRecordSlalom;
bool        BGame::m_bPassFromRightSlalom;
string     BGame::m_sRacePosition;

bool        BGame::m_bSlalomPolesVisualOK;
bool        BGame::m_bForceBreak;
double      BGame::m_dLiftStarted;

#ifdef ENABLE_MULTIPLAY
int           BGame::m_nRemotePlayers;
BRemotePlayer BGame::m_remotePlayer[4];
#endif

double      BGame::m_dRefTime[7];
int         BGame::m_nRefK;
string     BGame::m_sRefTime;

#ifdef ENABLE_MULTIPLAY
bool        BGame::m_bMultiplayOn;
bool        BGame::m_bExitingMultiplay;
bool        BGame::m_bOKToProceedInMultiplayMenu;
bool        BGame::m_bMultiplayRaceStarter;
unsigned     BGame::m_clockMultiRaceStarter;
int       BGame::m_clockOffsetFromZeroTime;
int         BGame::m_nPlayersInGoal;
int       BGame::m_nMultiplayPort = 2345;

int         BGame::m_nMultiplayMessages;
string     BGame::m_sMultiplayMessages[5];
bool        BGame::m_bChatMessage[5];
unsigned     BGame::m_clockMultiplayMessages[5];
bool        BGame::m_bTABChatting;
string     BGame::m_sChatMsg;
#endif

bool        BGame::m_bNight;

BUISelectionList BGame::m_listYesNo;
BUISelectionList BGame::m_listOK;

BUISelectionList BGame::m_listHSSpeedrace;
BUISelectionList BGame::m_listHSSlalom;
BUISelectionList BGame::m_listHSAirtime;

// BUISelectionList BGame::m_sellistGameMenu;

int              BGame::m_nController;
BControllerState BGame::m_controllerstate; // Access to a controller, such as a joystick or a wheel

#ifdef ENABLE_MULTIPLAY
BMultiPlay       BGame::m_multiplay;
#endif
//GUID             BGame::m_guidServiceProviders[10];







//*************************************************************************************************
BGame::BGame() {

  time_t ltime;
  time(&ltime);

  FILE *fp = fopen("Pakoon1.log", "w");
  fprintf(fp, "Pakoon1 started (build 4) *** %s--------------------------------------------\n", ctime(&ltime));
  fclose(fp);

  m_cmdModule.SetSim(&m_simulation);
  m_nController = 0;          // 0 = keyboard, 1 = joystick
  m_cOnScreenInfo = FPS;  // Show fps, speed, altitude etc.
  m_nSkyDetail = 2;           // High
  m_nDistantDetail = 2;       // High
  m_nColorMode = 1;           // Color
  m_nWaterDetail = 1;         // Draw water surface
  m_nScreenFormat = 0;        // Full screen
  m_nTextureSmoothness = 2;   // Silky
  m_bNavSat = false;
  m_bService = false;
  m_bSlowMotion = false;
  m_bFrozen = false;
  m_dNavSatHandleAngle = 0;
  m_dServiceHandleAngle = -20;
  m_nShowEffects = 0;
  m_bShowQuickHelp = false;
  m_bDrawOnScreenTracking = true;
  m_nPhysicsSteps = 10;
  m_bShowHint = false;
  m_clockHintStart = SDL_GetTicks();
  m_nCarDetails = 1;

  static string sYesNo[2] = {"Yes", "No"};
  static string sOK[1] = {"OK"};
  m_listYesNo.SetItems(sYesNo, 2);
  m_listYesNo.SelectItem("Yes");
  m_listOK.SetItems(sOK, 1);
  m_listOK.SelectItem("OK");

  static string sHighscoresInit[7] = {"0:00.00", "0:00.00", "0:00.00", "0:00.00", "0:00.00", "0:00.00", "0:00.00"};
  m_listHSSpeedrace.SetItems(sHighscoresInit, 7);
  m_listHSSlalom.SetItems(sHighscoresInit, 7);
  m_listHSAirtime.SetItems(sHighscoresInit, 7);

  //static string sGameMenu[5] = {"Back to Game", "Restart Race", "Settings", "Help", "Quit to Main Menu"};
  //m_sellistGameMenu.SetItems(sGameMenu, 5);
  //m_sellistGameMenu.SelectItem("Back to Game");

  m_bRaceStarted = false;
  m_bRaceFinished = false;
  m_dRaceTime = 0;
  m_dAirTime  = 0;

  m_bRecordSlalom = false;
  m_bPassFromRightSlalom = true;

  m_bSlalomPolesVisualOK = false;
  m_bForceBreak = false;
  m_dLiftStarted = 0;

#ifdef ENABLE_MULTIPLAY
  m_nRemotePlayers = 0;
#endif

  // Setup common tracking targets
  m_simulation.AddTrackingTarget("FUEL", BVector(0, 0, 0), 1, 0, 0);         // Red Fuel

  m_csMutex = SDL_CreateMutex();

  m_pMenuCurrent = &m_menuMain;
  m_sRacePosition = "1/1";

  m_dRefTime[0] = -1.0;
  m_dRefTime[1] = -1.0;
  m_dRefTime[2] = -1.0;
  m_dRefTime[3] = -1.0;
  m_dRefTime[4] = -1.0;
  m_dRefTime[5] = -1.0;
  m_dRefTime[6] = -1.0;
  m_nRefK = 0;
  m_sRefTime = "";

#ifdef ENABLE_MULTIPLAY
  m_bMultiplayOn = false;
  m_bExitingMultiplay = false;
  m_bOKToProceedInMultiplayMenu = false;
  m_bMultiplayRaceStarter = false;
  m_clockMultiRaceStarter = 0;
  m_clockOffsetFromZeroTime = 0;
  m_nPlayersInGoal = 0;

  m_nMultiplayMessages = 0;
  m_bTABChatting = false;
  m_sChatMsg = "";
#endif

  m_bNight = false;
}

//*************************************************************************************************
BGame::~BGame() {
  SDL_DestroyMutex(m_csMutex);
}



#ifdef ENABLE_MULTIPLAY
//*************************************************************************************************
int BGame::AddRemotePlayer(int id, char *pPlayerName, BClientConnection *client_connection) {

  if(m_nRemotePlayers >= 4) {
    // fully booked
    return -1;
  }

  int nIndex = m_nRemotePlayers;
  m_remotePlayer[nIndex].m_id = nIndex; //FIXME laitettiin id ennen
  strcpy(m_remotePlayer[nIndex].m_sName, (char *) pPlayerName);
  m_remotePlayer[nIndex].m_sCurrentMenuSel = "";
  m_remotePlayer[nIndex].m_bSelectionMade = false;
  m_remotePlayer[nIndex].m_state = BRemotePlayer::WANTS_TO_SELECT_NEW_RACE;
  if(nIndex != GetMyPlace()) {
    m_remotePlayer[nIndex].m_bSelf = false;
  }
  
  m_remotePlayer[nIndex].client_connection = client_connection;

  stringstream sMsg;
  sMsg << "New player " << m_remotePlayer[nIndex].m_sName << " has joined";
  ShowMultiplayMessage(sMsg.str());
  SoundModule::PlayMultiplayerJoinSound();

  ++m_nRemotePlayers;

  return nIndex;
}



//*************************************************************************************************
void BGame::HandlePlayerExit(char *pPlayerInfo) {
  // Find player
  int nIndex = int(pPlayerInfo[0]) - 1;
  
  if(nIndex < m_nRemotePlayers) {

    stringstream sMsg;
    sMsg << m_remotePlayer[nIndex].m_sName << " has left";
    ShowMultiplayMessage(sMsg.str());
    SoundModule::PlayMultiplayerLeftSound();

    // Remove remote player by replacing it with the last one and decreasing player counter.
    // Remember also to update nMyPlace, if necessary.
    if(nIndex != (m_nRemotePlayers - 1)) {
      m_remotePlayer[nIndex] = m_remotePlayer[m_nRemotePlayers - 1];
      if(GetMyPlace() == (m_nRemotePlayers - 1)) {
        GetMultiplay()->GetParams()->m_nMyPlace = nIndex;
      }
    }
    --m_nRemotePlayers;
  }
}



//*************************************************************************************************
void BGame::HandlePlayerAbnormalExit(int id) {
  // Find player
  int i = 0;
  bool bFound = false;
  for(i = 0; i < m_nRemotePlayers; ++i) {
    if(!m_remotePlayer[i].m_bSelf &&  
       (m_remotePlayer[i].m_id == id)) {
      bFound = true;
      break;
    }
  }
  
  if(bFound) {

    stringstream sMsg;
    sMsg << m_remotePlayer[i].m_sName << " has left (abnormally)";
    ShowMultiplayMessage(sMsg.str());
    SoundModule::PlayMultiplayerLeftSound();

    // Remove remote player by replacing it with the last one and decreasing player counter.
    // Remember also to update nMyPlace, if necessary.
    if(i != (m_nRemotePlayers - 1)) {
      m_remotePlayer[i] = m_remotePlayer[m_nRemotePlayers - 1];
      if(GetMyPlace() == (m_nRemotePlayers - 1)) {
        GetMultiplay()->GetParams()->m_nMyPlace = i;
      }
    }
    --m_nRemotePlayers;
  }
}



//*************************************************************************************************
void BGame::UpdatePlayerInfo(char *pPlayerInfo) { //FIXME kato et playerinfossa on oikeesti intin kokoeinen
  // Update the player name at the given location
  int nIndex = int(pPlayerInfo[0]) - 1;
  memcpy(&(m_remotePlayer[nIndex].m_id), pPlayerInfo + 1, sizeof(int));

  if(nIndex != GetMyPlace()) {
    m_remotePlayer[nIndex].m_bSelf = false;
  }

  strcpy(m_remotePlayer[nIndex].m_sName, (char *) pPlayerInfo + 1 + sizeof(int));
  if((nIndex + 1) > m_nRemotePlayers) {
    m_nRemotePlayers = nIndex + 1;

    stringstream sMsg;
    sMsg << "player " << m_remotePlayer[nIndex].m_sName << " has joined";
    ShowMultiplayMessage(sMsg.str());
  }
}


//*************************************************************************************************
void BGame::GetMultiplayerColor(int nIndex, double &dR, double &dG, double &dB) {

  dR = 1.0;
  dG = 0.5;
  dB = 0;

  if(m_bMultiplayOn) {
    switch(nIndex) {
      case 0: dR = 1;    dG = 0.25; dB = 0.25; break;
      case 1: dR = 0.25; dG = 1;    dB = 0.25; break;
      case 2: dR = 0.3;  dG = 0.3;  dB = 1;    break;
      case 3: dR = 1;    dG = 0.25; dB = 1;    break;
    }
  }
}



//*************************************************************************************************
void BGame::CheckForGameStart() {
  bool bAllReady = true;
  for(int i = 0; i < m_nRemotePlayers; ++i) {
    if(!m_remotePlayer[i].m_bReadyToStart && 
       (m_remotePlayer[i].m_state != BRemotePlayer::WANTS_TO_SELECT_NEW_RACE)) {
      bAllReady = false;
      break;
    }
  }
  if(bAllReady) {    
    GetMultiplay()->SendBroadcastMsg(BMultiPlay::START_GAME, "");
    m_bMultiplayRaceStarter = true;
    m_clockMultiRaceStarter = SDL_GetTicks();
    //GetView()->Invalidate();
  }
}


//*************************************************************************************************
void BGame::BroadcastCarSize() {
  char bMsg[3 + 4 * sizeof(double)];
  bMsg[0] = '-';
  bMsg[1] = BMultiPlay::MY_CAR_SIZE_IS;
  bMsg[2] = (char) GetMyPlace() + 1;

  memcpy(bMsg + 3,                      &(GetSimulation()->GetVehicle()->m_dVisualLength),  sizeof(double));
  memcpy(bMsg + 3 + 1 * sizeof(double), &(GetSimulation()->GetVehicle()->m_dVisualWidth),   sizeof(double));
  memcpy(bMsg + 3 + 2 * sizeof(double), &(GetSimulation()->GetVehicle()->m_dVisualHeight),  sizeof(double));
  memcpy(bMsg + 3 + 3 * sizeof(double), &(GetSimulation()->GetVehicle()->m_dTotalMass),     sizeof(double));

  (void) GetMultiplay()->SendBinaryBroadcastMsg(bMsg, 3 + 4 * sizeof(double));
}


//*************************************************************************************************
void BGame::BroadcastStateChange() {
  if(m_bMultiplayOn) {
    char bMsg[4];
    bMsg[0] = '-';
    bMsg[1] = BMultiPlay::MY_STATE_IS_NOW;
    bMsg[2] = (char) GetMyPlace() + 1;
    bMsg[3] = (char) m_remotePlayer[GetMyPlace()].m_state + 1;

    (void) GetMultiplay()->SendBinaryBroadcastMsg(bMsg, 4);
  }
}


//*************************************************************************************************
void BGame::BroadcastFinalPosition(int nIndex) {
  if(m_bMultiplayOn) {

    ++BGame::m_nPlayersInGoal;

    char bMsg[4];
    bMsg[0] = '-';
    bMsg[1] = BMultiPlay::HIS_FINAL_POSITION_IS;
    bMsg[2] = (char) nIndex + 1;
    bMsg[3] = (char) m_nPlayersInGoal + 1;

    stringstream sMsg;
    sMsg << m_remotePlayer[nIndex].m_sName << "'s final position is " << m_nPlayersInGoal;
    ShowMultiplayMessage(sMsg.str());

    (void) GetMultiplay()->SendBinaryBroadcastMsg(bMsg, 4);
  }
}

//*************************************************************************************************
void BGame::BroadcastInGoal() {

  if(m_bMultiplayOn) {
    if(GetMultiplay()->GetParams()->m_bHost) {
      // Record our own position, if we are the host
      BroadcastFinalPosition(GetMyPlace());
      m_remotePlayer[GetMyPlace()].m_nRacePosition = m_nPlayersInGoal;
      m_remotePlayer[GetMyPlace()].m_state = BRemotePlayer::FINISHED;
    } else {
      char bMsg[3];
      bMsg[0] = '-';
      bMsg[1] = BMultiPlay::I_AM_IN_GOAL;
      bMsg[2] = (char) GetMyPlace() + 1;

      (void) GetMultiplay()->SendBinaryBroadcastMsg(bMsg, 3);
    }
  }
}


//*************************************************************************************************
void BGame::BroadcastCarPosition() {
  // Send local car location to other remote players
  static unsigned clockNow;
  clockNow = SDL_GetTicks(); 

  if((clockNow - GetMultiplay()->GetParams()->m_clockPosLastSent) > (1000 / 50)) {

    GetMultiplay()->GetParams()->m_clockPosLastSent = clockNow;

    static char bMsg[2 + 3 * 3 * sizeof(float) + 1 + sizeof(int) + sizeof(long)];
    bMsg[0] = BMultiPlay::MY_POSITION_IS_THIS;
    bMsg[1] = (char) GetMyPlace() + 1;

    BVehicle *pVehicle = GetSimulation()->GetVehicle();

    BVector      vPos   = pVehicle->m_vLocation;
    BOrientation orient = pVehicle->m_orientation;

    float fPosX = float(vPos.m_dX);
    float fPosY = float(vPos.m_dY);
    float fPosZ = float(vPos.m_dZ);

    float fForwardDirX = float(orient.m_vForward.m_dX);
    float fForwardDirY = float(orient.m_vForward.m_dY);
    float fForwardDirZ = float(orient.m_vForward.m_dZ);

    float fRightDirX = float(orient.m_vRight.m_dX);
    float fRightDirY = float(orient.m_vRight.m_dY);
    float fRightDirZ = float(orient.m_vRight.m_dZ);

    memcpy(bMsg + 2,                     (void *) &fPosX, sizeof(float));
    memcpy(bMsg + 2 + 1 * sizeof(float), (void *) &fPosY, sizeof(float));
    memcpy(bMsg + 2 + 2 * sizeof(float), (void *) &fPosZ, sizeof(float));

    memcpy(bMsg + 2 + 3 * sizeof(float), (void *) &fForwardDirX, sizeof(float));
    memcpy(bMsg + 2 + 4 * sizeof(float), (void *) &fForwardDirY, sizeof(float));
    memcpy(bMsg + 2 + 5 * sizeof(float), (void *) &fForwardDirZ, sizeof(float));

    memcpy(bMsg + 2 + 6 * sizeof(float), (void *) &fRightDirX, sizeof(float));
    memcpy(bMsg + 2 + 7 * sizeof(float), (void *) &fRightDirY, sizeof(float));
    memcpy(bMsg + 2 + 8 * sizeof(float), (void *) &fRightDirZ, sizeof(float));

    char bTurn = char(pVehicle->m_dTurn * 50.0 + 50.0);
    memcpy(bMsg + 2 + 9 * sizeof(float), (void *) &bTurn, 1);

    int clockUniversal = BGame::GetMultiplayClock();
    memcpy(bMsg + 2 + 9 * sizeof(float) + 1, (void *) &clockUniversal, sizeof(int));

    memcpy(bMsg + 2 + 9 * sizeof(float) + 1 + sizeof(int), (void *) &(BGame::GetSimulation()->m_nSimulationTimeStep), sizeof(long));

    (void) GetMultiplay()->SendBinaryBroadcastMsg(bMsg, 2 + 3 * 3 * sizeof(float) + 1 + sizeof(int) + sizeof(long));
  }
}


//*************************************************************************************************
void BGame::RemoveOldestMultiplayMessage(bool bForce) {
  bool bCheckNext = true;
  do {
    if(m_nMultiplayMessages) {
      if(bForce || (m_clockMultiplayMessages[0] < SDL_GetTicks())) {
        // Delete first message
        for(int i = 1; i < m_nMultiplayMessages; ++i) {
          m_sMultiplayMessages[i - 1] = m_sMultiplayMessages[i];
          m_bChatMessage[i - 1] = m_bChatMessage[i];
          m_clockMultiplayMessages[i - 1] = m_clockMultiplayMessages[i];
        }
        --m_nMultiplayMessages;        
      }
      if(bForce) {
        bCheckNext = false;
      } else {
        bCheckNext = m_nMultiplayMessages && (m_clockMultiplayMessages[0] < SDL_GetTicks());
      }
    } else {
      bCheckNext = false;
    }
  } while(bCheckNext);
}


//*************************************************************************************************
void BGame::ShowMultiplayMessage(string sMsg, bool bChat) {
  // Add message to multiplay notification messages to display it for 2 seconds
  while(m_nMultiplayMessages >= 5) {
    // Kill oldest messages
    RemoveOldestMultiplayMessage(true);
  }

  // Add new message to the queue
  m_sMultiplayMessages[m_nMultiplayMessages] = sMsg;
  m_bChatMessage[m_nMultiplayMessages] = bChat;
  m_clockMultiplayMessages[m_nMultiplayMessages] = SDL_GetTicks() + 8000;
  ++m_nMultiplayMessages;
}
#endif



//*************************************************************************************************
void BGame::SetProgressRange(double dMax) {
  SDL_LockMutex(BGame::m_csMutex);
  m_dProgressMax = dMax;
  SDL_UnlockMutex(BGame::m_csMutex);
}


//*************************************************************************************************
void BGame::SetProgressPos(double dPos) {
  SDL_LockMutex(BGame::m_csMutex);
  m_dProgressPos = dPos;
  SDL_UnlockMutex(BGame::m_csMutex);
  if(!m_bMultiProcessor) {
    SDL_Delay(1);
  }
}


//*************************************************************************************************
double BGame::GetRelativeProgress() {
  double dRet;
  SDL_LockMutex(BGame::m_csMutex);
  dRet = m_dProgressPos / m_dProgressMax;
  SDL_UnlockMutex(BGame::m_csMutex);
  return dRet;
}


#ifdef ENABLE_MULTIPLAY
//*************************************************************************************************
void BGame::SetupMultiplayMenu() {
  /*
  string sServiceProviders[10];
  int nSPs = m_multiplay.GetServiceProviders(sServiceProviders, m_guidServiceProviders, 10);

  m_menuMultiplay.m_items[3].m_nAssocListItems = nSPs;
  m_menuMultiplay.m_items[3].m_sAssocListItems = new string[m_menuMultiplay.m_items[3].m_nAssocListItems];
  for(int i = 0; i < nSPs; ++i) {
    m_menuMultiplay.m_items[3].m_sAssocListItems[i] = sServiceProviders[i];
  }
  */
}
#endif




//*************************************************************************************************
void BGame::SetupMenus() {
  //******************************************
  // Choose Vehicle
  //******************************************

  m_menuChooseVehicle.m_sName = "Select Car";
  m_menuChooseVehicle.m_type = BMenu::CHOOSE_VEHICLE;
  m_menuChooseVehicle.m_nTitleWidth = 284;
  m_menuChooseVehicle.m_nTitleHeight = 60;
  m_menuChooseVehicle.m_dTitleX = 0;
  m_menuChooseVehicle.m_dTitleY = (512.0 - 177.0) / 512.0;

  // Fetch vehicles

  // count .vehicle files
  int nVehicles = 0;
  
	ifstream vehicles_file("vehicles");
	string vehicle;
	vector<string> vehicles;
	while (!vehicles_file.eof()) {
		getline(vehicles_file, vehicle);
		if (!vehicle.empty() && vehicle[0] != '\r' && vehicle[0] != '\n') {
			vehicles.push_back(vehicle);
			nVehicles++;
		}
	}
	vehicles_file.close();

  m_menuChooseVehicle.m_nItems = nVehicles;
  m_menuChooseVehicle.m_sItems = new string[nVehicles];
  m_menuChooseVehicle.m_items = new BMenuItem[nVehicles];

  // Load .vehicle files
  int i = 0;
  for (int x = 0; x < nVehicles; x++) {
    m_menuChooseVehicle.m_items[i].m_sAssocFile = vehicles[x];
    FileHelpers::GetKeyStringFromINIFile("Properties",
                                         "Name",
                                         "",
                                         m_menuChooseVehicle.m_items[i].m_sText,
                                         vehicles[x]);
    string sImageFile;
    FileHelpers::GetKeyStringFromINIFile("Properties",
                                         "Image",
                                         "",
                                         sImageFile,
                                         vehicles[x]);
    m_menuChooseVehicle.m_items[i].m_nAssocImage = BTextures::LoadTexture(sImageFile, false);
    ++i;
  }
  for(i = 0; i < m_menuChooseVehicle.m_nItems; ++i) {
    m_menuChooseVehicle.m_sItems[i] = m_menuChooseVehicle.m_items[i].m_sText;
  }

  m_menuChooseVehicle.m_listMenu.SetItems(m_menuChooseVehicle.m_sItems, m_menuChooseVehicle.m_nItems);
  m_menuChooseVehicle.m_listMenu.SelectItem(m_menuChooseVehicle.m_sItems[0]);
  m_menuChooseVehicle.m_align = BTextRenderer::ALIGN_RIGHT;

  //******************************************
  // Choose Scene
  //******************************************

  m_menuChooseScene.m_sName = "Select Location";
  m_menuChooseScene.m_type = BMenu::CHOOSE_SCENE;
  m_menuChooseScene.m_nTitleWidth = 284;
  m_menuChooseScene.m_nTitleHeight = 60;
  m_menuChooseScene.m_dTitleX = 0;
  m_menuChooseScene.m_dTitleY = (512.0 - 238.0) / 512.0;
  
  // count .scene files
  int nScenes = 0;
  
	ifstream scenes_file("scenes");
	string scene;
	vector<string> scenes;
	while (!scenes_file.eof()) {
		getline(scenes_file, scene);
		if (!scene.empty() && scene[0] != '\r' && scene[0] != '\n') {
			scenes.push_back(scene);
			nScenes++;
		}
	}
	scenes_file.close();

  m_menuChooseScene.m_nItems = nScenes;
  m_menuChooseScene.m_sItems = new string[nScenes];
  m_menuChooseScene.m_items = new BMenuItem[nScenes];
  m_menuChooseScene.m_listMenu.m_dOffsetToLeft = 0.05;

  // Load .Scene files
    i = 0;
  for (int x = 0; x < nScenes; x++) {
    m_menuChooseScene.m_items[i].m_sAssocFile = scenes[x];
    FileHelpers::GetKeyStringFromINIFile("Properties",
                                         "Name",
                                         "",
                                         m_menuChooseScene.m_items[i].m_sText,
                                         scenes[x]);
    string sImageFile;
    FileHelpers::GetKeyStringFromINIFile("Properties",
                                         "Image",
                                         "",
                                         sImageFile,
                                         scenes[x]);
    m_menuChooseScene.m_items[i].m_nAssocImage = BTextures::LoadTexture(sImageFile, false);
    BVector vTmp;
    FileHelpers::GetKeyVectorFromINIFile("Properties",
                                         "MapPosition",
                                         BVector(580, 85, 0),
                                         vTmp,
                                         scenes[x]);
    m_menuChooseScene.m_items[i].m_nValue = int(vTmp.m_dX);
    m_menuChooseScene.m_items[i].m_nValue2 = int(vTmp.m_dY);
    ++i;
  }
  for(i = 0; i < m_menuChooseScene.m_nItems; ++i) {
    m_menuChooseScene.m_sItems[i] = m_menuChooseScene.m_items[i].m_sText;
  }

  m_menuChooseScene.m_listMenu.SetItems(m_menuChooseScene.m_sItems, m_menuChooseScene.m_nItems);
  m_menuChooseScene.m_listMenu.SelectItem(m_menuChooseScene.m_sItems[0]);
  m_menuChooseScene.m_align = BTextRenderer::ALIGN_RIGHT;

  //******************************************
  // Hiscores
  //******************************************

  m_menuHiscores.m_sName = "Highscores";
  m_menuHiscores.m_type = BMenu::HISCORES;
  m_menuHiscores.m_nTitleWidth = 284;
  m_menuHiscores.m_nTitleHeight = 60;
  m_menuHiscores.m_dTitleX = 0;
  m_menuHiscores.m_dTitleY = (512.0 - 238.0) / 512.0;

  m_menuHiscores.m_nItems = nScenes;
  m_menuHiscores.m_sItems = new string[nScenes];
  m_menuHiscores.m_items = new BMenuItem[nScenes];
  m_menuHiscores.m_listMenu.m_dOffsetToLeft = 0.05;

  for(i = 0; i < m_menuHiscores.m_nItems; ++i) {
    m_menuHiscores.m_sItems[i] = m_menuChooseScene.m_items[i].m_sText;
  }

  m_menuHiscores.m_listMenu.SetItems(m_menuHiscores.m_sItems, m_menuHiscores.m_nItems);
  m_menuHiscores.m_align = BTextRenderer::ALIGN_RIGHT;

  UpdateHighScoreMenu();

  //******************************************
  // PrecachingTerrain
  //******************************************

  m_menuPrecachingTerrain.m_sName = "LOADING";
  m_menuPrecachingTerrain.m_type = BMenu::PRECACHING_TERRAIN;
  m_menuPrecachingTerrain.m_nTitleWidth = 284;
  m_menuPrecachingTerrain.m_nTitleHeight = 60;
  m_menuPrecachingTerrain.m_dTitleX = 0;
  m_menuPrecachingTerrain.m_dTitleY = (512.0 - 123.0) / 512.0;

  m_bMenusCreated = true;

  //******************************************
  // MAIN
  //******************************************

  m_menuMain.m_sName = "Main Menu";
  m_menuMain.m_type = BMenu::MAIN;
  m_menuMain.m_nTitleWidth = 284;
  m_menuMain.m_nTitleHeight = 80;
  m_menuMain.m_dTitleX = 0;
  m_menuMain.m_dTitleY = (512.0 - 314.0) / 512.0;

  m_menuMain.m_nItems = 6;
  m_menuMain.m_sItems = new string[m_menuMain.m_nItems];
  m_menuMain.m_items = new BMenuItem[m_menuMain.m_nItems];

  m_menuMain.m_items[0].m_sText = "SINGLEPLAYER";
  m_menuMain.m_items[1].m_sText = "MULTIPLAYER";
  m_menuMain.m_items[2].m_sText = "SETTINGS";
  m_menuMain.m_items[3].m_sText = "HISCORES";
  m_menuMain.m_items[4].m_sText = "CREDITS";
  m_menuMain.m_items[5].m_sText = "EXIT";

  for(i = 0; i < m_menuMain.m_nItems; ++i) {
    m_menuMain.m_sItems[i] = m_menuMain.m_items[i].m_sText;
  }

  m_menuMain.m_listMenu.SetItems(m_menuMain.m_sItems, m_menuMain.m_nItems);
  m_menuMain.m_listMenu.SelectItem(m_menuMain.m_sItems[0]);
  m_menuMain.m_align = BTextRenderer::ALIGN_CENTER;

  //******************************************
  // GAME MODE
  //******************************************

  m_menuChooseGameMode.m_sName = "SELECT RACE TYPE";
  m_menuChooseGameMode.m_type = BMenu::GAMEMODE;
  m_menuChooseGameMode.m_nTitleWidth = 284;
  m_menuChooseGameMode.m_nTitleHeight = 80;
  m_menuChooseGameMode.m_dTitleX = 0;
  m_menuChooseGameMode.m_dTitleY = (512.0 - 314.0) / 512.0;

  m_menuChooseGameMode.m_nItems = 3;
  m_menuChooseGameMode.m_sItems = new string[m_menuChooseGameMode.m_nItems];
  m_menuChooseGameMode.m_items = new BMenuItem[m_menuChooseGameMode.m_nItems];

  m_menuChooseGameMode.m_items[0].m_sText = "SPEEDRACE";
  m_menuChooseGameMode.m_items[1].m_sText = "SLALOM";
  m_menuChooseGameMode.m_items[2].m_sText = "AIRTIME";

  for(i = 0; i < m_menuChooseGameMode.m_nItems; ++i) {
    m_menuChooseGameMode.m_sItems[i] = m_menuChooseGameMode.m_items[i].m_sText;
  }

  m_menuChooseGameMode.m_listMenu.SetItems(m_menuChooseGameMode.m_sItems, m_menuChooseGameMode.m_nItems);
  m_menuChooseGameMode.m_listMenu.SelectItem(m_menuChooseGameMode.m_sItems[0]);
  m_menuChooseGameMode.m_align = BTextRenderer::ALIGN_CENTER;

#ifdef ENABLE_MULTIPLAY
  //******************************************
  // MultiPlay
  //******************************************

  m_menuMultiplay.m_sName = "Multiplayer settings";
  m_menuMultiplay.m_type = BMenu::MULTIPLAYER;
  m_menuMultiplay.m_nTitleWidth = 284;
  m_menuMultiplay.m_nTitleHeight = 60;
  m_menuMultiplay.m_dTitleX = 0;
  m_menuMultiplay.m_dTitleY = (512.0 - 60.0) / 512.0;

  m_menuMultiplay.m_nItems = 4;
  m_menuMultiplay.m_sItems = new string[m_menuMultiplay.m_nItems];
  m_menuMultiplay.m_items = new BMenuItem[m_menuMultiplay.m_nItems];

  m_menuMultiplay.m_items[0].m_sText = "Player name:";
  m_menuMultiplay.m_items[0].m_type = BMenuItem::EDITBOX;
  m_menuMultiplay.m_items[0].m_sValue = GetMultiplay()->m_params.m_sPlayerName;
  m_menuMultiplay.m_items[0].m_ebAssocEditBox.Setup("", GetMultiplay()->m_params.m_sPlayerName, 16);
  m_menuMultiplay.m_items[1].m_sText = "Role:";
  m_menuMultiplay.m_items[1].m_type = BMenuItem::STRING_FROM_LIST;
    m_menuMultiplay.m_items[1].m_nAssocListItems = 2;
    m_menuMultiplay.m_items[1].m_sAssocListItems = new string[m_menuMultiplay.m_items[1].m_nAssocListItems];
    m_menuMultiplay.m_items[1].m_sAssocListItems[0] = "SERVER";
    m_menuMultiplay.m_items[1].m_sAssocListItems[1] = "CLIENT";
  m_menuMultiplay.m_items[1].m_listMenu.SetItems(m_menuMultiplay.m_items[1].m_sAssocListItems, m_menuMultiplay.m_items[1].m_nAssocListItems);
  if(GetMultiplay()->m_params.m_bHost) {
    m_menuMultiplay.m_items[1].m_nValue = 0;
  } else {
    m_menuMultiplay.m_items[1].m_nValue = 1;
  }
  m_menuMultiplay.m_items[1].m_listMenu.SelectItem(m_menuMultiplay.m_items[1].m_sAssocListItems[m_menuMultiplay.m_items[1].m_nValue]);

  m_menuMultiplay.m_items[2].m_sText = "SERVER IP/NAME:";
  m_menuMultiplay.m_items[2].m_type = BMenuItem::EDITBOX;
  m_menuMultiplay.m_items[2].m_sValue = GetMultiplay()->m_params.m_sHostIPAddress;
  m_menuMultiplay.m_items[2].m_bDisabled = m_menuMultiplay.m_items[1].m_nValue == 0;
  m_menuMultiplay.m_items[2].m_ebAssocEditBox.Setup("", GetMultiplay()->m_params.m_sHostIPAddress, 64);

  //m_menuMultiplay.m_items[3].m_sText = "Service Provider:";
  //m_menuMultiplay.m_items[3].m_type = BMenuItem::TType::STRING_FROM_LIST;
  //  m_menuMultiplay.m_items[3].m_nAssocListItems = 0;
  //  m_menuMultiplay.m_items[3].m_sAssocListItems = 0;

  m_menuMultiplay.m_items[3].m_sText = "START MULTIPLAY";
  m_menuMultiplay.m_items[3].m_type = BMenuItem::BASIC;
  m_menuMultiplay.m_items[3].m_dRed = 1.0;
  m_menuMultiplay.m_items[3].m_dGreen = 0.75;
  m_menuMultiplay.m_items[3].m_dBlue = 0.25;

  for(i = 0; i < m_menuMultiplay.m_nItems; ++i) {
    m_menuMultiplay.m_sItems[i] = m_menuMultiplay.m_items[i].m_sText;
  }

  m_menuMultiplay.m_listMenu.SetItems(m_menuMultiplay.m_sItems, m_menuMultiplay.m_nItems);
  m_menuMultiplay.m_listMenu.SelectItem(m_menuMultiplay.m_sItems[0]);
  m_menuMultiplay.m_align = BTextRenderer::ALIGN_RIGHT;  

#endif

  //******************************************
  // Settings
  //******************************************

  m_menuSettings.m_sName = "Settings";
  m_menuSettings.m_type = BMenu::SETTINGS;
  m_menuSettings.m_nTitleWidth = 284;
  m_menuSettings.m_nTitleHeight = 60;
  m_menuSettings.m_dTitleX = 0;
  m_menuSettings.m_dTitleY = (512.0 - 60.0) / 512.0;

  m_menuSettings.m_nItems = 9;
  m_menuSettings.m_sItems = new string[m_menuSettings.m_nItems];
  m_menuSettings.m_items = new BMenuItem[m_menuSettings.m_nItems];

  m_menuSettings.m_items[0].m_sText = "Resolution:";
  m_menuSettings.m_items[0].m_type = BMenuItem::STRING_FROM_LIST;
  m_menuSettings.m_items[1].m_sText = "Colors:";
  m_menuSettings.m_items[1].m_type = BMenuItem::STRING_FROM_LIST;
  m_menuSettings.m_items[2].m_sText = "Refresh:";
  m_menuSettings.m_items[2].m_type = BMenuItem::STRING_FROM_LIST;

  EnumerateScreenResolutions();

  m_menuSettings.m_items[3].m_sText = "Terrain Details:";
  m_menuSettings.m_items[3].m_type = BMenuItem::STRING_FROM_LIST;
    m_menuSettings.m_items[3].m_nAssocListItems = 6;
    m_menuSettings.m_items[3].m_sAssocListItems = new string[m_menuSettings.m_items[3].m_nAssocListItems];
    m_menuSettings.m_items[3].m_sAssocListItems[0] = "Minimum";
    m_menuSettings.m_items[3].m_sAssocListItems[1] = "Low";
    m_menuSettings.m_items[3].m_sAssocListItems[2] = "Medium";
    m_menuSettings.m_items[3].m_sAssocListItems[3] = "High";
    m_menuSettings.m_items[3].m_sAssocListItems[4] = "Maximum";
    m_menuSettings.m_items[3].m_sAssocListItems[5] = "(slow machine)";
    m_menuSettings.m_items[3].m_nValue = m_nTerrainResolution;
  m_menuSettings.m_items[4].m_sText = "Car Details:";
  m_menuSettings.m_items[4].m_type = BMenuItem::STRING_FROM_LIST;
    m_menuSettings.m_items[4].m_nAssocListItems = 3;
    m_menuSettings.m_items[4].m_sAssocListItems = new string[m_menuSettings.m_items[4].m_nAssocListItems];
    m_menuSettings.m_items[4].m_sAssocListItems[0] = "Always High";
    m_menuSettings.m_items[4].m_sAssocListItems[1] = "High for Singleplayer";
    m_menuSettings.m_items[4].m_sAssocListItems[2] = "Always Low";
    // m_menuSettings.m_items[4].m_nValue = m_nDustAndClouds;
    m_menuSettings.m_items[4].m_nValue = m_nCarDetails;
  m_menuSettings.m_items[5].m_sText = "TIME OF DAY:";
  m_menuSettings.m_items[5].m_type = BMenuItem::STRING_FROM_LIST;
    m_menuSettings.m_items[5].m_nAssocListItems = 2;
    m_menuSettings.m_items[5].m_sAssocListItems = new string[m_menuSettings.m_items[5].m_nAssocListItems];
    m_menuSettings.m_items[5].m_sAssocListItems[0] = "NOON";
    m_menuSettings.m_items[5].m_sAssocListItems[1] = "MIDNIGHT";
    m_menuSettings.m_items[5].m_nValue = m_nWaterSurface;
  m_menuSettings.m_items[6].m_sText = "Music Volume:";
  m_menuSettings.m_items[6].m_type = BMenuItem::SLIDER;
    m_menuSettings.m_items[6].m_nValue = m_nMusicVolume;
  m_menuSettings.m_items[7].m_sText = "Sound Effects Volume:";
  m_menuSettings.m_items[7].m_type = BMenuItem::SLIDER;
    m_menuSettings.m_items[7].m_nValue = m_nVehicleVolume;
  m_menuSettings.m_items[8].m_sText = "Soundscape:";
  m_menuSettings.m_items[8].m_type = BMenuItem::STRING_FROM_LIST;
    m_menuSettings.m_items[8].m_nAssocListItems = 2;
    m_menuSettings.m_items[8].m_sAssocListItems = new string[m_menuSettings.m_items[8].m_nAssocListItems];
    m_menuSettings.m_items[8].m_sAssocListItems[0] = "3D";
    m_menuSettings.m_items[8].m_sAssocListItems[1] = "Lame";
    m_menuSettings.m_items[8].m_nValue = m_nSoundscape;

  for(i = 0; i < m_menuSettings.m_nItems; ++i) {
    m_menuSettings.m_sItems[i] = m_menuSettings.m_items[i].m_sText;
  }

  m_menuSettings.m_listMenu.SetItems(m_menuSettings.m_sItems, m_menuSettings.m_nItems);
  m_menuSettings.m_listMenu.SelectItem(m_menuSettings.m_sItems[0]);
  m_menuSettings.m_align = BTextRenderer::ALIGN_RIGHT;

  //******************************************
  // Credits
  //******************************************

  m_menuCredits.m_sName = "Credits";
  m_menuCredits.m_type = BMenu::CREDITS;
  m_menuCredits.m_nTitleWidth = 0;
  m_menuCredits.m_nTitleHeight = 0;
  m_menuCredits.m_dTitleX = 0;
  m_menuCredits.m_dTitleY = 0;
  m_menuCredits.m_bDrawLine = false;

  m_menuCredits.m_nItems = 7;
  m_menuCredits.m_sItems = new string[m_menuCredits.m_nItems];
  m_menuCredits.m_items = new BMenuItem[m_menuCredits.m_nItems];

  m_menuCredits.m_items[0].m_nValue = 0; // Second at which to start showing this item
  m_menuCredits.m_items[0].m_nValue2 = 4; // Seconds to show this item
  m_menuCredits.m_items[0].m_nAssocImage = -1; // Just a blank delay

  m_menuCredits.m_items[1].m_nValue = 4;
  m_menuCredits.m_items[1].m_nValue2 = 10; // Seconds to show this item
  m_menuCredits.m_items[1].m_nAssocImage = BTextures::LoadTexture("./Textures/CreditsProgramming.tga", false);

  m_menuCredits.m_items[2].m_nValue = 14;
  m_menuCredits.m_items[2].m_nValue2 = 10; // Seconds to show this item
  m_menuCredits.m_items[2].m_nAssocImage = BTextures::LoadTexture("./Textures/CreditsMusic.tga", false);

  m_menuCredits.m_items[3].m_nValue = 24;
  m_menuCredits.m_items[3].m_nValue2 = 10; // Seconds to show this item
  m_menuCredits.m_items[3].m_nAssocImage = BTextures::LoadTexture("./Textures/CreditsBetaTesting.tga", false);

  m_menuCredits.m_items[4].m_nValue = 34;
  m_menuCredits.m_items[4].m_nValue2 = 13; // Seconds to show this item
  m_menuCredits.m_items[4].m_nAssocImage = BTextures::LoadTexture("./Textures/CreditsSpecialThanks1.tga", false);

  m_menuCredits.m_items[5].m_nValue = 47;
  m_menuCredits.m_items[5].m_nValue2 = 7; // Seconds to show this item
  m_menuCredits.m_items[5].m_nAssocImage = BTextures::LoadTexture("./Textures/CreditsEnd.tga", false);

  m_menuCredits.m_items[6].m_nValue = 54;
  m_menuCredits.m_items[6].m_nValue2 = 1; // Seconds to show this item
  m_menuCredits.m_items[6].m_nAssocImage = -1;

  m_menuCredits.m_listMenu.SetItems(m_menuCredits.m_sItems, 0);
  m_menuCredits.m_align = BTextRenderer::ALIGN_RIGHT;

  //******************************************
  // GAME MENU
  //******************************************

  m_menuGame.m_sName = "Game Menu";
  m_menuGame.m_type = BMenu::GAME;
  m_menuGame.m_nTitleWidth = 1;
  m_menuGame.m_nTitleHeight = 1;
  m_menuGame.m_dTitleX = 0;
  m_menuGame.m_dTitleY = 0;

  m_menuGame.m_nItems = 5;
  m_menuGame.m_sItems = new string[m_menuGame.m_nItems];
  m_menuGame.m_items = new BMenuItem[m_menuGame.m_nItems];

  m_menuGame.m_items[0].m_sText = "BACK TO GAME";
  m_menuGame.m_items[1].m_sText = "RESTART RACE";
  m_menuGame.m_items[2].m_sText = "SETTINGS";
  m_menuGame.m_items[3].m_sText = "HELP";
  m_menuGame.m_items[4].m_sText = "QUIT TO MAIN MENU";

  for(i = 0; i < m_menuGame.m_nItems; ++i) {
    m_menuGame.m_sItems[i] = m_menuGame.m_items[i].m_sText;
  }

  m_menuGame.m_listMenu.SetItems(m_menuGame.m_sItems, m_menuGame.m_nItems);
  m_menuGame.m_listMenu.SelectItem(m_menuGame.m_sItems[0]);
  m_menuGame.m_align = BTextRenderer::ALIGN_CENTER;

  m_bMenusCreated = true;
}


//*************************************************************************************************
void BGame::UpdateSettings() {
  m_menuSettings.m_items[3].m_nValue = m_nTerrainResolution;
  m_menuSettings.m_items[4].m_nValue = m_nCarDetails;
}




//*************************************************************************************************
void BGame::EnumerateScreenResolutions() {

  // First enumerate all to see how many there are

  int i;
  int nModes = SDL_GetNumDisplayModes(0); //FIXME always 0 display
  //BOOL bRet;
  int bRet;
  //DEVMODE devmode;
  int nSetting = 0;
  /*do {
    bRet = EnumDisplaySettings(NULL, nSetting, &devmode);
    ++nModes;
    ++nSetting;
  } while(bRet);*/

  int      nTmp;
  int      nAllRes = 0;
  string *psAllRes = new string[nModes];
  int      nAllColors = 0;
  string *psAllColors = new string[nModes];
  int      nAllRefresh = 0;
  string *psAllRefresh = new string[nModes];

  string sTmp;
  nSetting = 0;
  //do {
  for (int x = 0; x < nModes; x++) {
    //bRet = EnumDisplaySettings(NULL, nSetting, &devmode);
    SDL_DisplayMode mode;
    if (SDL_GetDisplayMode(0, x, &mode) >= 0) {
		//sTmp.Format("%ld*%ld PIXELS", devmode.dmPelsWidth, devmode.dmPelsHeight);
		stringstream format;
		format << mode.w << "*" << mode.h << " PIXELS";
		sTmp = format.str();
		if(!FindStringFromArray(sTmp, psAllRes, nAllRes, nTmp)) {
		  psAllRes[nAllRes] = sTmp;
		  ++nAllRes;
		}
		//sTmp.Format("%ld BITS", devmode.dmBitsPerPel);
		format.str("");
		format << SDL_BITSPERPIXEL(mode.format) << " BITS";
		sTmp = format.str();
		if(!FindStringFromArray(sTmp, psAllColors, nAllColors, nTmp)) {
		  psAllColors[nAllColors] = sTmp;
		  ++nAllColors;
		}
		//sTmp.Format("%ld HERTZ", devmode.dmDisplayFrequency);
		format.str("");
		format << mode.refresh_rate << " HERTZ";
		sTmp = format.str();
		if(!FindStringFromArray(sTmp, psAllRefresh, nAllRefresh, nTmp)) {
		  psAllRefresh[nAllRefresh] = sTmp;
		  ++nAllRefresh;
		}
		++nSetting;
	}
  //} while(bRet);
	}

  m_menuSettings.m_items[0].m_nAssocListItems = nAllRes;
  m_menuSettings.m_items[0].m_sAssocListItems = new string[nAllRes];
  for(i = 0; i < nAllRes; ++i) {
    m_menuSettings.m_items[0].m_sAssocListItems[i] = psAllRes[i];
  }

  m_menuSettings.m_items[1].m_nAssocListItems = nAllColors;
  m_menuSettings.m_items[1].m_sAssocListItems = new string[nAllColors];
  for(i = 0; i < nAllColors; ++i) {
    m_menuSettings.m_items[1].m_sAssocListItems[i] = psAllColors[i];
  }

  m_menuSettings.m_items[2].m_nAssocListItems = nAllRefresh;
  m_menuSettings.m_items[2].m_sAssocListItems = new string[nAllRefresh];
  for(i = 0; i < nAllRefresh; ++i) {
    m_menuSettings.m_items[2].m_sAssocListItems[i] = psAllRefresh[i];
  }

  // Preselect current mode
  //sTmp.Format("%d*%d PIXELS", m_nDispWidth, m_nDispHeight);
	stringstream format;
	format << m_nDispWidth << "*" << m_nDispHeight << " PIXELS";
	sTmp = format.str();
  //(void) FindStringFromArray(sTmp, psAllRes, nAllRes, m_menuSettings.m_items[0].m_nValue)
  if (!FindStringFromArray(sTmp, psAllRes, nAllRes, m_menuSettings.m_items[0].m_nValue)) {
	  m_menuSettings.m_items[0].m_nValue = 0;
  }
  //sTmp.Format("%d BITS", m_nDispBits);
	format.str("");
	format << m_nDispBits << " BITS";
	sTmp = format.str();
  //(void) FindStringFromArray(sTmp, psAllColors, nAllColors, m_menuSettings.m_items[1].m_nValue);
  if (!FindStringFromArray(sTmp, psAllColors, nAllColors, m_menuSettings.m_items[1].m_nValue)) {
	  m_menuSettings.m_items[1].m_nValue = 0;
  }
  m_menuSettings.m_items[1].m_nValue = 0;
  //sTmp.Format("%d HERTZ", m_nDispHz);
	format.str("");
	format << m_nDispHz << " HERTZ";
	sTmp = format.str();
  //(void) FindStringFromArray(sTmp, psAllRefresh, nAllRefresh, m_menuSettings.m_items[2].m_nValue);
  if (!FindStringFromArray(sTmp, psAllRefresh, nAllRefresh, m_menuSettings.m_items[2].m_nValue)) {
	  m_menuSettings.m_items[2].m_nValue = 0;
  }

  delete [] psAllRes;
  delete [] psAllColors;
  delete [] psAllRefresh;
}


//*************************************************************************************************
bool BGame::FindStringFromArray(string s, string *psArray, int nItems, int &rnIndex) {
  rnIndex = -1;
  for(int i = 0; i < nItems; ++i) {
    if(s.compare(psArray[i]) == 0) {
      rnIndex = i;
      return true;
    }
  }
  return false;
}



//*************************************************************************************************
void BGame::FreezeSimulation(bool bPause) {
  if(m_nFreezeRefCount == 0) {
    SoundModule::SetGameMusicVolume(int(double(BGame::m_nMusicVolume) / 100.0 * 128.0));
    m_nPhysicsSteps = m_simulation.m_nPhysicsStepsBetweenRender;
    m_clockFrozenStart = SDL_GetTicks();
    m_bFrozen = true;
    if(bPause) {
      GetSimulation()->m_bPaused = true;
    }
  }
  ++m_nFreezeRefCount;
}

//*************************************************************************************************
unsigned BGame::ContinueSimulation() {
  if(m_nFreezeRefCount > 0) {
    --m_nFreezeRefCount;
    if((m_nFreezeRefCount == 0) && GetSimulation()->m_bPaused) {
      SoundModule::SetGameMusicVolume(int(double(BGame::m_nMusicVolume) / 100.0 * 255.0));
      GetSimulation()->m_bPaused = false;
      m_simulation.m_nPhysicsStepsBetweenRender = m_nPhysicsSteps;
      m_bFrozen = false;
      return SDL_GetTicks() - m_clockFrozenStart;
    }
    if(m_nFreezeRefCount == 0) {
      m_simulation.m_nPhysicsStepsBetweenRender = m_nPhysicsSteps;
      SoundModule::SetGameMusicVolume(int(double(BGame::m_nMusicVolume) / 100.0 * 255.0));
      m_bFrozen = false;
      return SDL_GetTicks() - m_clockFrozenStart;
    }
  }
  return 0;
}


//*************************************************************************************************
void BGame::SetupScene() {
}





extern double Random(double dRange);







extern double g_dRate;
extern double g_d10LastFPS[];

//*************************************************************************************************
void BGame::UpdateAnalyzer() {
  if(!m_bAnalyzerMode) {
    return;
  }

  // See in which phase we are in
  static int nPhase = -1;
  unsigned clockNow = SDL_GetTicks();

  int nNewPhase = (clockNow - m_clockAnalyzerStarted) / 1000 / 3;
  if(nPhase != nNewPhase) {
    // Exit old phase
    if(nPhase != -1) {
      string sInfo;
      MyAfxMessageBox("---------------------------------------------------------");
      string sVis;
      switch(nPhase) {
        case 0: sVis = "SKY"; break;
        case 1: sVis = "WATER"; break;
        case 2: sVis = "TERRAIN"; break;
        case 3: sVis = "VEHICLE"; break;
        case 4: sVis = "GASSTATIONS"; break;
        case 5: sVis = "OBJECTS"; break;
        case 6: sVis = "DUSTANDCLOUDS"; break;
        case 7: sVis = "GRAPHICS2D"; break;
        case 8: sVis = "ALL & ~SKY"; break;
        case 9: sVis = "ALL & ~WATER"; break;
        case 10: sVis = "ALL & ~TERRAIN"; break;
        case 11: sVis = "ALL & ~VEHICLE"; break;
        case 12: sVis = "ALL & ~GASSTATIONS"; break;
        case 13: sVis = "ALL & ~OBJECTS"; break;
        case 14: sVis = "ALL & ~DUSTANDCLOUDS"; break;
        case 15: sVis = "ALL & ~GRAPHICS2D"; break;
        case 16: sVis = "WATER (2)"; break;
        case 17: sVis = "WATER (1)"; break;
        case 18: sVis = "WATER (0)"; break;
        case 19: sVis = "ALL (water 0)"; break;
        case 20: sVis = "ALL (water 2)"; break;
      }
      if(nPhase == 7) {
        sVis = "GRAPHICS2D";
      }
      //FIXME
      /*sInfo.Format("ANALYZER: Phase %d Info (Visualize = %s)", nPhase, sVis);
      MyAfxMessageBox(sInfo);
      sInfo.Format("FPS: AVE=%.2lf, Last10=%.1lf %.1lf %.1lf %.1lf %.1lf %.1lf %.1lf %.1lf %.1lf %.1lf ", 
                    g_dRate,
                    g_d10LastFPS[0],
                    g_d10LastFPS[1],
                    g_d10LastFPS[2],
                    g_d10LastFPS[3],
                    g_d10LastFPS[4],
                    g_d10LastFPS[5],
                    g_d10LastFPS[6],
                    g_d10LastFPS[7],
                    g_d10LastFPS[8],
                    g_d10LastFPS[9]);
      BGame::MyAfxMessageBox(sInfo);*/
      MyAfxMessageBox("---------------------------------------------------------");
    }
    // Enter new phase
    nPhase = nNewPhase;
    switch(nPhase) {
      case 0:
        m_nVisualize = SKY;
        m_nWaterSurface = 0;
        break;
      case 1:
        m_nVisualize = WATER;
        break;
      case 2:
        m_nVisualize = TERRAIN;
        break;
      case 3:
        m_nVisualize = VEHICLE;
        break;
      case 4:
        m_nVisualize = GASSTATIONS;
        break;
      case 5:
        m_nVisualize = OBJECTS;
        break;
      case 6:
        m_nVisualize = DUSTANDCLOUDS;
        break;
      case 7:
        m_nVisualize = GRAPHICS2D;
        break;
      case 8:
        m_nVisualize = ALL & ~SKY;
        break;
      case 9:
        m_nVisualize = ALL & ~WATER;
        break;
      case 10:
        m_nVisualize = ALL & ~TERRAIN;
        break;
      case 11:
        m_nVisualize = ALL & ~VEHICLE;
        break;
      case 12:
        m_nVisualize = ALL & ~GASSTATIONS;
        break;
      case 13:
        m_nVisualize = ALL & ~OBJECTS;
        break;
      case 14:
        m_nVisualize = ALL & ~DUSTANDCLOUDS;
        break;
      case 15:
        m_nVisualize = ALL & ~GRAPHICS2D;
        break;
      case 16:
        m_nVisualize = WATER;
        m_nWaterSurface = 2;
        break;
      case 17:
        m_nVisualize = WATER;
        m_nWaterSurface = 1;
        break;
      case 18:
        m_nVisualize = WATER;
        m_nWaterSurface = 0;
        break;
      case 19:
        m_nVisualize = ALL;
        break;
      case 20:
        m_nVisualize = ALL;
        m_nWaterSurface = 2;
        break;
      case 21:
        // End analyzer
        nPhase = -1;
        m_bAnalyzerMode = false;
        BGame::MyAfxMessageBox("--------------------------");
        BGame::MyAfxMessageBox("ANALYZER ENDED!");
        BGame::MyAfxMessageBox("--------------------------");
        GetView()->m_nMenuTime += ContinueSimulation();
        BMessages::Show(40, "analyzer", "ANALYZING COMPLETE!", 2);
        break;
    }
  }
}





//*************************************************************************************************
string BGame::GetScrambleChecksum() {
  static string sSeed = "Settings, Main, Medium, TerrainResolution";
  string sRet = sSeed;
  stringstream sState;
  sState << m_player.m_dCash << ":" << m_player.m_dFuel << ":" << m_player.m_dKerosine << ":" << m_player.m_sValidVehicles << ":" << m_player.m_sSceneInfo;

  // calculate checksum string
  int i;

  int nStart = 0;
  int nEnd = sRet.length() - 1;
  int nStep = 1;
  int nCounter = 0;

  for(i = 0; i < sState.str().length(); ++i) {
    nCounter = 0;

    if(nStep > 0) {
      nStart = 0;
      nEnd = sRet.length() - 1;
    } else {
      nStart = sRet.length() - 1;
      nEnd = 0;
    }

    unsigned char c = (unsigned char) sState.str()[i];
    for(int j = nStart; j != nEnd; j += nStep) {
      unsigned char cOld = (unsigned char) sRet[j];
      unsigned char cNew = ((cOld ^ ~c) + c * nCounter) % 256;
      sRet[j] = cNew;
      ++nCounter;
    }
    nStep *= -1;
  }

  // map to readable characters
  for(i = 0; i < sRet.length(); ++i) {
    unsigned char c = (unsigned char) sRet[i];
    if(c < 10) {
      sRet[i] = ((unsigned char) (sRet[i]) % ('9' - '0')) + '0';
    } else if(c < 128) {
      sRet[i] = ((unsigned char) (sRet[i]) % ('z' - 'a')) + 'a';
    } else {
      sRet[i] = ((unsigned char) (sRet[i]) % ('Z' - 'A')) + 'A';
    }
  }

  return sRet;
}


//*************************************************************************************************
string BGame::GetVerifyChecksum(string sSource) {
  static string sSeed = "PAKOON2.Many! Copyright 2003 Mikko Oksalahti";
  string sRet = sSeed;

  // calculate checksum string
  int i;

  int nStart = 0;
  int nEnd = sRet.length() - 1;
  int nStep = 1;
  int nCounter = 0;

  for(i = 0; i < sSource.length(); ++i) {
    nCounter = 0;

    if(nStep > 0) {
      nStart = 0;
      nEnd = sRet.length() - 1;
    } else {
      nStart = sRet.length() - 1;
      nEnd = 0;
    }

    unsigned char c = (unsigned char) sSource[i];
    for(int j = nStart; j != nEnd; j += nStep) {
      unsigned char cOld = (unsigned char) sRet[j];
      unsigned char cNew = ((cOld ^ ~c) + c * nCounter) % 251;
      sRet[j] = cNew;
      ++nCounter;
    }
    nStep *= -1;
  }

  // map to readable characters
  for(i = 0; i < sRet.length(); ++i) {
    unsigned char c = (unsigned char) sRet[i];
    if(c < 13) {
      sRet[i] = ((unsigned char) (sRet[i]) % ('9' - '0')) + '0';
    } else if(c < 133) {
      sRet[i] =  ((unsigned char) (sRet[i]) % ('z' - 'a')) + 'a';
    } else {
      sRet[i] = ((unsigned char) (sRet[i]) % ('Z' - 'A')) + 'A';
    }
  }

  return sRet;
}





//*************************************************************************************************
void BGame::MyAfxMessageBox(string sText, int nTmp) {

  time_t ltime;
  time(&ltime);
  struct tm *newtime;
  newtime = localtime(&ltime);

  FILE *fp = fopen("Pakoon1.log", "a");
  fprintf(fp, "%.24s: %s", asctime(newtime), sText.c_str());
  fprintf(fp, "\n");
  fclose(fp);
  
  cout << sText << endl;
}

static double g_cdPI = 3.141592654;

//*************************************************************************************************
double BGame::GetSmoothAlpha() {
  double dAlpha = fabs(double(SDL_GetTicks() % 1000) - 500.0) / 500.0;
  // return sin(dAlpha * g_cdPI / 2.0);
  return dAlpha;
}

//*************************************************************************************************
void BGame::UpdateHighScores(string sSceneName, TGameMode gameMode, double dTime) {
  // See if new high score was made
  string sTmp;
  double dOldTime = dTime;
  FileHelpers::GetKeyStringFromINIFile("AllTimeHigh",
                                       sSceneName,
                                       "0 0 0",
                                       sTmp,
                                       "./Player/Highscores.dat");

  bool bUpdate = false;
  double dRace = 0, dSlalom = 0, dAir = 0;

  sscanf(sTmp.c_str(), "%lf %lf %lf", &dRace, &dSlalom, &dAir);

  switch(gameMode) {
    case SPEEDRACE:
      if((dRace == 0) || (dTime < dRace)) {
        dRace = dTime;
        bUpdate = true;
      }
      break;
    case SLALOM:
      if((dSlalom == 0) || (dTime < dSlalom)) {
        dSlalom = dTime;
        bUpdate = true;
      }
      break;
    case AIRTIME:
      if(dTime > dAir) {
        dAir = dTime;
        bUpdate = true;
      }
      break;
  }

  if(bUpdate) {

    // Check whether the previous highscores.dat was verified. If it was, only then update
    if(CheckHighscoresValidity()) {
		stringstream val;
      val << dRace << " " << dSlalom << " " << dAir;
      sTmp = val.str();
      FileHelpers::WriteKeyStringToINIFile("AllTimeHigh", sSceneName, sTmp, "./Player/Highscores.dat");
      ValidateHighscores();

      UpdateHighScoreMenu();
    }
  }
}


extern double g_dPhysicsStepsInSecond;


//*************************************************************************************************
void BGame::UpdateHighScoreMenu() {

  static string sHighscoresSpeed[100];
  static string sHighscoresSlalom[100];
  static string sHighscoresAir[100];

  for(int i = 0; i < m_menuChooseScene.m_nItems; ++i) {
    string sScene;
    sScene = m_menuChooseScene.m_items[i].m_sText;
    string sItem;
    FileHelpers::GetKeyStringFromINIFile("AllTimeHigh",
                                         sScene,
                                         "0 0 0",
                                         sItem,
                                         "./Player/Highscores.dat");

    // Update Highscore menu's selection lists

    stringstream sTmp;
    sTmp.fill('0');
    int nMinutesTotal = 0;
    int nSecondsTotal = 0;
    int n100SecondsTotal = 0;

    double dRace = 0.0, dSlalom = 0.0, dAir = 0.0;
    sscanf(sItem.c_str(), "%lf %lf %lf", &dRace, &dSlalom, &dAir);

    if(dRace != 0.0) {
      nMinutesTotal = dRace / g_dPhysicsStepsInSecond / 60;
      nSecondsTotal = (dRace - (nMinutesTotal * g_dPhysicsStepsInSecond * 60)) / g_dPhysicsStepsInSecond;
      n100SecondsTotal = (100 * (dRace - (nMinutesTotal * g_dPhysicsStepsInSecond * 60 + nSecondsTotal * g_dPhysicsStepsInSecond))) / g_dPhysicsStepsInSecond;
      sTmp << setw(2) << nMinutesTotal << ":" << setw(2) << nSecondsTotal << "." << setw(2) << n100SecondsTotal;
      sHighscoresSpeed[i] = sTmp.str();
    } else {
      sHighscoresSpeed[i] = " ";
    }

    if(dSlalom != 0.0) {
      nMinutesTotal = dSlalom / g_dPhysicsStepsInSecond / 60;
      nSecondsTotal = (dSlalom - (nMinutesTotal * g_dPhysicsStepsInSecond * 60)) / g_dPhysicsStepsInSecond;
      n100SecondsTotal = (100 * (dSlalom - (nMinutesTotal * g_dPhysicsStepsInSecond * 60 + nSecondsTotal * g_dPhysicsStepsInSecond))) / g_dPhysicsStepsInSecond;
      sTmp.str("");
      sTmp << setw(2) << nMinutesTotal << ":" << setw(2) << nSecondsTotal << "." << setw(2) << n100SecondsTotal;
      sHighscoresSlalom[i] = sTmp.str();
    } else {
      sHighscoresSlalom[i] = " ";
    }

    if(dAir != 0.0) {
      nMinutesTotal = dAir / g_dPhysicsStepsInSecond / 60;
      nSecondsTotal = (dAir - (nMinutesTotal * g_dPhysicsStepsInSecond * 60)) / g_dPhysicsStepsInSecond;
      n100SecondsTotal = (100 * (dAir - (nMinutesTotal * g_dPhysicsStepsInSecond * 60 + nSecondsTotal * g_dPhysicsStepsInSecond))) / g_dPhysicsStepsInSecond;
      sTmp.str("");
      sTmp << setw(2) << nMinutesTotal << ":" << setw(2) << nSecondsTotal << "." << setw(2) << n100SecondsTotal;
      sHighscoresAir[i] = sTmp.str();
    } else {
      sHighscoresAir[i] = " ";
    }
  }
  m_listHSSpeedrace.SetItems(sHighscoresSpeed, m_menuChooseScene.m_nItems);
  m_listHSSlalom.SetItems(sHighscoresSlalom, m_menuChooseScene.m_nItems);
  m_listHSAirtime.SetItems(sHighscoresAir, m_menuChooseScene.m_nItems);
}


//*************************************************************************************************
bool BGame::CheckHighscoresValidity() {
  string sCorrect;
  string sCurrent;
  sCorrect = GetHighscoresChecksum();
  FileHelpers::GetKeyStringFromINIFile("AllTimeHigh",
                                       "Checksum",
                                       "0",
                                       sCurrent,
                                       "./Player/Highscores.dat");

  return sCorrect.compare(sCurrent) == 0;
}

//*************************************************************************************************
void BGame::ValidateHighscores() {
  string sCorrect = GetHighscoresChecksum();
  FileHelpers::WriteKeyStringToINIFile("AllTimeHigh", "Checksum", sCorrect, "./Player/Highscores.dat");
}

//*************************************************************************************************
string BGame::GetHighscoresChecksum() {
  // Loop through scenes to find correct checksum
  string sData;
  sData = "";
  for(int i = 0; i < m_menuChooseScene.m_nItems; ++i) {
    string sScene;
    sScene = m_menuChooseScene.m_items[i].m_sText;
    string sItem;
    FileHelpers::GetKeyStringFromINIFile("AllTimeHigh",
                                         sScene,
                                         "0 0 0",
                                         sItem,
                                         "./Player/Highscores.dat");
    sData = sData + sItem;
  }
  return GetVerifyChecksum(sData);
}




//*************************************************************************************************
//FIXME
/*HRESULT WINAPI DirectPlayMessageHandler(PVOID pvUserContext, 
                                        int dwMessageId, 
                                        PVOID pMsgBuffer) {
  // Try not to stay in this message handler for too long, otherwise
  // there will be a backlog of data.  
  // This function is called by the DirectPlay message handler pool of 
  // threads, so be careful of thread synchronization problems with shared memory

  switch(dwMessageId) {
    case DPN_MSGID_ADD_PLAYER_TO_GROUP:
    case DPN_MSGID_APPLICATION_DESC:
    case DPN_MSGID_ASYNC_OP_COMPLETE:
    case DPN_MSGID_CLIENT_INFO:
    case DPN_MSGID_CONNECT_COMPLETE:
    case DPN_MSGID_CREATE_GROUP:
    case DPN_MSGID_CREATE_PLAYER:
    case DPN_MSGID_DESTROY_GROUP:
    case DPN_MSGID_ENUM_HOSTS_QUERY:
    case DPN_MSGID_ENUM_HOSTS_RESPONSE:
    case DPN_MSGID_GROUP_INFO:
    case DPN_MSGID_INDICATE_CONNECT:
    case DPN_MSGID_INDICATED_CONNECT_ABORTED:
    case DPN_MSGID_PEER_INFO:
    case DPN_MSGID_REMOVE_PLAYER_FROM_GROUP:
    case DPN_MSGID_RETURN_BUFFER:
    case DPN_MSGID_SEND_COMPLETE:
    case DPN_MSGID_SERVER_INFO:
      break;
    case DPN_MSGID_HOST_MIGRATE:
      {
        PDPNMSG_HOST_MIGRATE pHostMigrateMsg;
        pHostMigrateMsg = (PDPNMSG_HOST_MIGRATE)pMsgBuffer;

        // Check to see if we are the new host
        if(pHostMigrateMsg->dpnidNewHost == BGame::m_remotePlayer[BGame::GetMyPlace()].m_id) {
          BGame::GetMultiplay()->GetParams()->m_bHost = true;
          BGame::ShowMultiplayMessage("YOU ARE THE NEW HOST");
          // Update state info
          BGame::GetMultiplay()->SendBroadcastMsg(BMultiPlay::TTinyMessages::REPORT_YOUR_STATE, "");
        }

        break;
      }
    case DPN_MSGID_TERMINATE_SESSION:
      {
        //PDPNMSG_TERMINATE_SESSION pTerminateSessionMsg;
        //pTerminateSessionMsg = (PDPNMSG_TERMINATE_SESSION)pMsgBuffer;

        //g_hrDialog = DPNERR_CONNECTIONLOST;
        //EndDialog( g_hDlg, 0 );
        break;
      }

    case DPN_MSGID_DESTROY_PLAYER:
      {
        // Handle player's (abnormal) exit

        PDPNMSG_DESTROY_PLAYER pPlayerExitMsg;
        pPlayerExitMsg = (PDPNMSG_DESTROY_PLAYER)pMsgBuffer;

        BGame::HandlePlayerAbnormalExit(pPlayerExitMsg->dpnidPlayer);
      }
      break;

    case DPN_MSGID_RECEIVE:
      {

        // Process Multiplay message

        PDPNMSG_RECEIVE pReceiveMsg;
        pReceiveMsg = (PDPNMSG_RECEIVE)pMsgBuffer;

        BGame::GetMultiplay()->ProcessMultiplayMessage(pReceiveMsg);

        break;
      }
  }

  return S_OK;
}*/


#ifdef ENABLE_MULTIPLAY
//*************************************************************************************************
BRemotePlayer::~BRemotePlayer() {
  if(m_pVehicle && !m_bVehicleReused) {
    delete m_pVehicle;
    m_pVehicle = 0;
  }
}



//*************************************************************************************************
BMultiPlay::BMultiPlay() {
  //m_pDP = 0; //FIXME
  m_bDPInitialized = false;
  // Initialize COM
  //HRESULT hr = CoInitialize(0); //FIXME
}

//*************************************************************************************************
BMultiPlay::~BMultiPlay() {
	//FIXME
  /*if(m_pDP && m_bDPInitialized) {
    m_pDP->Close(0);
    m_bDPInitialized = false;
  }*/

  // Release COM
  //CoUninitialize(); //FIXME
}

//*************************************************************************************************
bool BMultiPlay::InitMultiplaySession() {
	//FIXME
  /*HRESULT hr;

  // First create needed DirectPlay objects

  static bool bDirectPlayObjectsCreated = false;

  if(!bDirectPlayObjectsCreated) {

    // Create IDirectPlay8Peer

    if((hr = CoCreateInstance(CLSID_DirectPlay8Peer, NULL, 
                              CLSCTX_INPROC_SERVER,
                              IID_IDirectPlay8Peer, 
                              (LPVOID*) &m_pDP)) != 0) {
      // Report fail to start multiplay
      return false;
    }

    bDirectPlayObjectsCreated = true;

  }

  if(!m_bDPInitialized) {

    // Initialize DP

    if((hr = m_pDP->Initialize(NULL, DirectPlayMessageHandler, DPNINITIALIZE_DISABLEPARAMVAL)) != 0) {
      return false;
    }
    m_bDPInitialized = true;
  }*/
  
	if(SDLNet_Init() == -1) {
		cout << "SDLNet_Init: " << SDLNet_GetError() << endl;
		return false;
	}
  
  return true;
}

//GUID g_guidApp = { 0x2ae835d, 0x9179, 0x485f, { 0x83, 0x43, 0x90, 0x1d, 0x32, 0x7c, 0xe7, 0x94 } }; //FIXME

//*************************************************************************************************
bool BMultiPlay::StartMultiplaySession(BMultiplayParams *pParams) {
  Settings::WriteSettings(BGame::GetSimulation());

  //HRESULT hr;

  m_params = *pParams;

  // Host or connect to a game

  // First set peer info

  /*WCHAR wszPeerName[255];
  MulticharToWideChar(CP_ACP, 0, LPCTSTR(pParams->m_sPlayerName), -1, wszPeerName, 255);

  DPN_PLAYER_INFO dpnPlayerInfo;
  dpnPlayerInfo.dwSize = sizeof(DPN_PLAYER_INFO);
  dpnPlayerInfo.dwInfoFlags = DPNINFO_NAME;
  dpnPlayerInfo.pwszName = wszPeerName;
  dpnPlayerInfo.pvData = 0;
  dpnPlayerInfo.dwDataSize = 0;
  dpnPlayerInfo.dwPlayerFlags = 0;

  DPNHANDLE dpnHandle;

  if((hr = m_pDP->SetPeerInfo(&dpnPlayerInfo, 0, &dpnHandle, DPNSETPEERINFO_SYNC)) != 0) {
    return false;
  }
  
  DPN_APPLICATION_DESC dpnAppDesc;
  dpnAppDesc.dwSize = sizeof(DPN_APPLICATION_DESC);
  dpnAppDesc.dwFlags = DPNSESSION_MIGRATE_HOST | DPNSESSION_NODPNSVR; // since I provide port number
  dpnAppDesc.guidInstance = GUID_NULL;
  dpnAppDesc.guidApplication = g_guidApp;
  dpnAppDesc.dwMaxPlayers = 4;
  dpnAppDesc.dwCurrentPlayers = 0;
  dpnAppDesc.pwszSessionName = 0;
  dpnAppDesc.pwszPassword = 0;
  dpnAppDesc.pvReservedData = 0;
  dpnAppDesc.dwReservedDataSize = 0;
  dpnAppDesc.pvApplicationReservedData = 0;
  dpnAppDesc.dwApplicationReservedDataSize = 0;

  // Provide device address
  IDirectPlay8Address *pDeviceAddress;
  if((hr = CoCreateInstance(CLSID_DirectPlay8Address, 
                            NULL,	
                            CLSCTX_INPROC_SERVER,
                            IID_IDirectPlay8Address,
                            (LPVOID*) &pDeviceAddress)) != 0) {
    return false;
  }
  pDeviceAddress->SetSP(&CLSID_DP8SP_TCPIP);
  // int dwPort = 2502;
  int dwPort = BGame::m_nMultiplayPort;
  if((hr = pDeviceAddress->AddComponent(DPNA_KEY_PORT,
                                        &dwPort, 
                                        sizeof(dwPort),
                                        DPNA_DATATYPE_int)) != 0) {
    return false;
  }

  if(pParams->m_bHost) {

    // Host a game

    if((hr = m_pDP->Host(&dpnAppDesc,
                         &pDeviceAddress, // address device info array
                         1, // number of address device infos
                         0, // security attributes
                         0, // security credentials
                         0, // player context
                         0)) != 0) {
      return false;
    }

    // Store self as first remote player

    BGame::m_nRemotePlayers = 1;
    strcpy(BGame::m_remotePlayer[0].m_sName, LPCTSTR(pParams->m_sPlayerName));
    BGame::m_remotePlayer[0].m_sCurrentMenuSel = "";
    BGame::m_remotePlayer[0].m_bSelectionMade = false;
    BGame::m_remotePlayer[0].m_id = 0;
    BGame::m_remotePlayer[0].m_bSelf = true;
    BGame::m_remotePlayer[1].m_bSelf = false;
    BGame::m_remotePlayer[2].m_bSelf = false;
    BGame::m_remotePlayer[3].m_bSelf = false;
    BGame::GetMultiplay()->GetParams()->m_nMyPlace = 0;
  } else {

    // Connect to a game

    WCHAR wszHostAddress[255];
    MulticharToWideChar(CP_ACP, 0, LPCTSTR(pParams->m_sHostIPAddress), -1, wszHostAddress, 255);

    if((hr = pDeviceAddress->AddComponent(DPNA_KEY_HOSTNAME,
                                          wszHostAddress, 
                                          (int) (wcslen(wszHostAddress) + 1) * sizeof(WCHAR),
                                          DPNA_DATATYPE_STRING)) != 0) {
      return false;
    }

    if((hr = m_pDP->Connect(&dpnAppDesc,
                            pDeviceAddress, 
                            pDeviceAddress, 
                            0, // security attributes
                            0, // security credentials
                            0, // connect data
                            0, // size of connect data
                            0, // player context
                            0, // async context
                            0, // [out] async handle
                            DPNCONNECT_SYNC)) != 0) {
      return false;
    }

    // Clear list of remote players. 
    // Host will inform us about all remote players with the PLAYER_INFO message.
    BGame::m_nRemotePlayers = 0; 

    // Identify yourself in the game
    SendBroadcastMsg(WHO_AM_I, pParams->m_sPlayerName);
  }

  BGame::m_bMultiplayOn = true;
  BGame::m_bExitingMultiplay = false;*/
  
  int dwPort = BGame::m_nMultiplayPort;
  
  	IPaddress ip;
  
  if(pParams->m_bHost) {
	if(SDLNet_ResolveHost(&ip, NULL, dwPort)==-1) {
		cout << "SDLNet_ResolveHost: " << SDLNet_GetError() << endl;
		return false;
	}
	
    // Store self as first remote player

    BGame::m_nRemotePlayers = 1;
    strcpy(BGame::m_remotePlayer[0].m_sName, pParams->m_sPlayerName.c_str());
    BGame::m_remotePlayer[0].m_sCurrentMenuSel = "";
    BGame::m_remotePlayer[0].m_bSelectionMade = false;
    BGame::m_remotePlayer[0].m_id = 0;
    BGame::m_remotePlayer[0].m_bSelf = true;
    BGame::m_remotePlayer[1].m_bSelf = false;
    BGame::m_remotePlayer[2].m_bSelf = false;
    BGame::m_remotePlayer[3].m_bSelf = false;
    BGame::GetMultiplay()->GetParams()->m_nMyPlace = 0;
  } else {
	if(SDLNet_ResolveHost(&ip, pParams->m_sHostIPAddress.c_str(), dwPort)==-1) {
		cout << "SDLNet_ResolveHost: " << SDLNet_GetError() << " address was: " << pParams->m_sHostIPAddress.c_str() << endl;
		return false;
	}
	
    // Clear list of remote players. 
    // Host will inform us about all remote players with the PLAYER_INFO message.
    BGame::m_nRemotePlayers = 0; 
  }
  
	own_tcpsock = SDLNet_TCP_Open(&ip);
	if(!own_tcpsock) {
		cout << "SDLNet_TCP_Open: " << SDLNet_GetError() << endl;
		return false;
	}

  BGame::m_bMultiplayOn = true;
  BGame::m_bExitingMultiplay = false;
  
	if (pParams->m_bHost) {
		// create incoming connection listening thread
		incoming_connection_thread = SDL_CreateThread(listenIncomingConnections, "incoming_connection", (void *)this);
	} else {
		// Identify yourself in the game
		SendBroadcastMsg(WHO_AM_I, pParams->m_sPlayerName);
		
		listen_thread = SDL_CreateThread(receiveThread, "listen", (void *)this);
	}
  
  return true;
}

//*************************************************************************************************
bool BMultiPlay::SendPeerMsg(char bMsg, int id, string sMsgText) {
  //DPN_BUFFER_DESC bufferDesc;
  //FIXME ID pitää myös laittaa mukaan, jos tarvitaan jossain
  char buffer[255];
  buffer[0] = '-'; // Indicates a non-during-game message
  buffer[1] = bMsg;
  sprintf((char*) buffer + 2, "%s", sMsgText.c_str());
  buffer[sMsgText.length() + 2] = 0;
  if (buffer[1] == 9) {
	  cout << "len: " << sMsgText.length() << endl;
	  cout << "teksti: " << (int)buffer[2] << endl;
  }

  /*bufferDesc.dwBufferSize = sMsgText.GetLength() + 3;
  bufferDesc.pBufferData = buffer;

  HRESULT hr;

  hr = m_pDP->SendTo(id,
                     &bufferDesc,
                     1,
                     0, // timeout
                     0, // async context
                     0, // phAsyncHandle,
                     DPNSEND_SYNC | DPNSEND_NOCOMPLETE | DPNSEND_NOLOOPBACK);

  return (hr == S_OK);*/
  
  int len = sMsgText.length() + 3;
  
  return send(id, buffer, len);
}

//*************************************************************************************************
bool BMultiPlay::SendBroadcastMsg(char bMsg, string sMsgText) {
  return SendPeerMsg(bMsg, -1, sMsgText); //-1 oli DPNID_ALL_PLAYERS_GROUP
}

//*************************************************************************************************
bool BMultiPlay::SendBinaryBroadcastMsg(char *pbMsg, int nSize) {
  /*DPN_BUFFER_DESC bufferDesc;
  bufferDesc.dwBufferSize = nSize;
  bufferDesc.pBufferData = pbMsg;

  HRESULT hr;

  hr = m_pDP->SendTo(DPNID_ALL_PLAYERS_GROUP,
                     &bufferDesc,
                     1,
                     0, // timeout
                     0, // async context
                     0, // phAsyncHandle,
                     DPNSEND_PRIORITY_HIGH | DPNSEND_SYNC | DPNSEND_NOCOMPLETE | DPNSEND_NOLOOPBACK);

  return (hr == S_OK);*/
  
  return send(-1, pbMsg, nSize);
}


extern double g_dPhysicsStepsInSecond;


//*************************************************************************************************
void BMultiPlay::ProcessMultiplayMessage(char *data, int data_size, BClientConnection *client_connection) {
  // Check whether this is a during-game message or a general message

  if(data_size < 1) {
    // empty message
    return;
  }

  if(data[0] == MY_POSITION_IS_THIS) {

    // Process on-game position message

    SDL_LockMutex(BGame::m_csMutex);
    {
      int clockNow = BGame::GetMultiplayClock();
      int nIndex = int(data[1]) - 1;

      // Save old location info
      BGame::m_remotePlayer[nIndex].m_clockPrevLocationSent = BGame::m_remotePlayer[nIndex].m_clockLocationSent;
      BGame::m_remotePlayer[nIndex].m_nPrevLocationSent = BGame::m_remotePlayer[nIndex].m_nLocationSent;
      BGame::m_remotePlayer[nIndex].m_vLocationPrev = BGame::m_remotePlayer[nIndex].m_vLocation;

      // Get new location info
      float fPosX, fPosY, fPosZ;
      float fForwardDirX, fForwardDirY, fForwardDirZ;
      float fRightDirX, fRightDirY, fRightDirZ;

      memcpy(&fPosX,        data + 2,                     sizeof(float));
      memcpy(&fPosY,        data + 2 + 1 * sizeof(float), sizeof(float));
      memcpy(&fPosZ,        data + 2 + 2 * sizeof(float), sizeof(float));

      memcpy(&fForwardDirX, data + 2 + 3 * sizeof(float), sizeof(float));
      memcpy(&fForwardDirY, data + 2 + 4 * sizeof(float), sizeof(float));
      memcpy(&fForwardDirZ, data + 2 + 5 * sizeof(float), sizeof(float));

      memcpy(&fRightDirX,   data + 2 + 6 * sizeof(float), sizeof(float));
      memcpy(&fRightDirY,   data + 2 + 7 * sizeof(float), sizeof(float));
      memcpy(&fRightDirZ,   data + 2 + 8 * sizeof(float), sizeof(float));

      char bTurn;
      memcpy(&bTurn, data + 2 + 9 * sizeof(float), 1);
      BGame::m_remotePlayer[nIndex].m_dTurn = double(bTurn - 50) / 50.0;

      memcpy(&(BGame::m_remotePlayer[nIndex].m_clockLocationSent), data + 2 + 9 * sizeof(float) + 1, sizeof(int));
      memcpy(&(BGame::m_remotePlayer[nIndex].m_nLocationSent), data + 2 + 9 * sizeof(float) + 1 + sizeof(int), sizeof(long));
      
      BGame::m_remotePlayer[nIndex].m_clockLocationReceived = clockNow;

      BGame::m_remotePlayer[nIndex].m_vLocation.Set(fPosX, fPosY, fPosZ);
      BGame::m_remotePlayer[nIndex].m_orientation.m_vForward.Set(fForwardDirX, fForwardDirY, fForwardDirZ);
      BGame::m_remotePlayer[nIndex].m_orientation.m_vRight.Set(fRightDirX, fRightDirY, fRightDirZ);
      BGame::m_remotePlayer[nIndex].m_orientation.m_vForward.ToUnitLength();
      BGame::m_remotePlayer[nIndex].m_orientation.m_vRight.ToUnitLength();
      BGame::m_remotePlayer[nIndex].m_orientation.m_vUp = BGame::m_remotePlayer[nIndex].m_orientation.m_vRight.CrossProduct(BGame::m_remotePlayer[nIndex].m_orientation.m_vForward);
      BGame::m_remotePlayer[nIndex].m_orientation.m_vUp.ToUnitLength();
      BGame::m_remotePlayer[nIndex].m_orientation.m_vUp = BGame::m_remotePlayer[nIndex].m_orientation.m_vUp * -1.0;

      BVector vDir = BGame::m_remotePlayer[nIndex].m_vLocation - BGame::m_remotePlayer[nIndex].m_vLocationPrev;

      // vDir = vDir * (1.0 / double(BGame::m_remotePlayer[nIndex].m_clockLocationSent - BGame::m_remotePlayer[nIndex].m_clockPrevLocationSent));

      if((BGame::m_remotePlayer[nIndex].m_nLocationSent - BGame::m_remotePlayer[nIndex].m_nPrevLocationSent) == 0) {
        vDir.Set(0, 0, 0);
      } else {
        vDir = vDir * (1.0 / double(BGame::m_remotePlayer[nIndex].m_nLocationSent - BGame::m_remotePlayer[nIndex].m_nPrevLocationSent));
      }
      vDir = vDir * (g_dPhysicsStepsInSecond / 1000.0);

      BVector vNewDeriv1 = vDir - BGame::m_remotePlayer[nIndex].m_vVelocity;
      BGame::m_remotePlayer[nIndex].m_vVelo2ndDeriv = vNewDeriv1 - BGame::m_remotePlayer[nIndex].m_vVelo1stDeriv;
      BGame::m_remotePlayer[nIndex].m_vVelo1stDeriv = vNewDeriv1;
      BGame::m_remotePlayer[nIndex].m_vVelocity = vDir;

      /*
      if(BGame::GetMultiplay()->GetParams()->m_bHost) {
        string sMsg;
        sMsg.Format("Receive-Loc: MyTime = %7lu oldSent = %7lu newSent = %7lu, OldLocY:%10.3lf NewLocY:%10.3lf vVelocityY: %10.7lf", 
                    (unsigned long) clockNow,
                    (unsigned long) BGame::m_remotePlayer[nIndex].m_clockPrevLocationSent, 
                    (unsigned long) BGame::m_remotePlayer[nIndex].m_clockLocationSent,
                    BGame::m_remotePlayer[nIndex].m_vLocationPrev.m_dY,
                    BGame::m_remotePlayer[nIndex].m_vLocation.m_dY,
                    BGame::m_remotePlayer[nIndex].m_vVelocity.m_dY);
        BGame::MyAfxMessageBox(sMsg);
      }
      */
    } 
    SDL_UnlockMutex(BGame::m_csMutex);

  } else if((data[0] == '-') && (data_size >= 2)) {

    // Process general multiplay message

    switch(data[1]) {

      case WHO_AM_I:

        // A client wants to know his place in the game

        if(m_params.m_bHost) {
          // Add a new client and inform him about his place
          int nIndex = BGame::AddRemotePlayer(0, data + 2, client_connection); //FIXME id oli pReceiveMsg->dpnidSender

          if(nIndex != -1) {
            // Send YOU_ARE message
            char sData[2];
            sData[0] = (char) nIndex;
            sData[1] = 0;
            SendPeerMsg(YOU_ARE, BGame::m_remotePlayer[nIndex].m_id, sData);
            BGame::m_pView->BroadcastMenuBrowse();
          }

          // Send PLAYER_INFO messages to all players to update their lists
          for(int i = 0; i < BGame::m_nRemotePlayers; ++i) {

            //string sMsg;
            //sMsg.Format("%c%s", char(i + 1), BGame::m_remotePlayer[i].m_sName);
            //SendBroadcastMsg(PLAYER_INFO, sMsg);

            char *bMsg = new char[3 + sizeof(int) + strlen(BGame::m_remotePlayer[i].m_sName) + 1];
            bMsg[0] = '-';
            bMsg[1] = BMultiPlay::PLAYER_INFO;
            bMsg[2] = char(i + 1);
            memcpy(bMsg + 3, &(BGame::m_remotePlayer[i].m_id), sizeof(int));
            strcpy((char *) bMsg + 3 + sizeof(int), BGame::m_remotePlayer[i].m_sName);

            (void) SendBinaryBroadcastMsg(bMsg, 3 + sizeof(int) + strlen(BGame::m_remotePlayer[i].m_sName) + 1);

            delete [] bMsg;
          }

          // Ask everyone to tell their state so the new guy knows the game
          SendBroadcastMsg(BMultiPlay::REPORT_YOUR_STATE, "");
        }

        break;

      case YOU_ARE:

        // Host tells me where I sit

        if(!m_params.m_bHost) {

          // Add your own name to the remote player list (right position)
    
          m_params.m_nMyPlace = int(data[2]);

          BGame::m_remotePlayer[0].m_bSelf = false;
          BGame::m_remotePlayer[1].m_bSelf = false;
          BGame::m_remotePlayer[2].m_bSelf = false;
          BGame::m_remotePlayer[3].m_bSelf = false;
          BGame::m_remotePlayer[m_params.m_nMyPlace].m_bSelf = true;
          BGame::m_remotePlayer[m_params.m_nMyPlace].m_state = BRemotePlayer::WANTS_TO_SELECT_NEW_RACE;
          BGame::m_remotePlayer[m_params.m_nMyPlace].m_id = 0;
          BGame::BroadcastStateChange();

          // Do the host a favor and tell him his id

			//FIXME no need?
          /*char *bMsg = new char[2 + sizeof(int)];
          bMsg[0] = '-';
          bMsg[1] = BMultiPlay::HOST_YOUR_ID_IS;
          memcpy(bMsg + 2, &(pReceiveMsg->dpnidSender), sizeof(int));
          (void) SendBinaryBroadcastMsg(bMsg, 2 + sizeof(int));*/
        }

        break;

      case HOST_YOUR_ID_IS:

        if(m_params.m_bHost) {

          // Store my id

          memcpy(&(BGame::m_remotePlayer[GetParams()->m_nMyPlace].m_id), data + 2, sizeof(int));

          // Broadcast updated player info (i.e. m_id) for other

          char *bMsg = new char[3 + sizeof(int) + strlen(BGame::m_remotePlayer[GetParams()->m_nMyPlace].m_sName)];
          bMsg[0] = '-';
          bMsg[1] = BMultiPlay::PLAYER_INFO;
          bMsg[2] = char(GetParams()->m_nMyPlace + 1);
          memcpy(bMsg + 3, &(BGame::m_remotePlayer[GetParams()->m_nMyPlace].m_id), sizeof(int));
          strcpy((char *) bMsg + 3 + sizeof(int), BGame::m_remotePlayer[GetParams()->m_nMyPlace].m_sName);

          (void) SendBinaryBroadcastMsg(bMsg, 3 + sizeof(int) + strlen(BGame::m_remotePlayer[GetParams()->m_nMyPlace].m_sName) + 1);
        }
        break;

      case REPORT_YOUR_STATE:

        {
          // Send your present state to other players

          BGame::BroadcastStateChange();
        }
        
      case PLAYER_INFO:

        // Save Player Info (this also adds new remote players to the list)

        if(!m_params.m_bHost) {
          BGame::UpdatePlayerInfo(data + 2);
        }

        break;

      case I_EXITED:

        // Handle player exit

        BGame::HandlePlayerExit(data + 2);

        break;

      case MENU_BROWSE:

        // Save menu browse info

        {
          int nIndex = int(data[2]) - 1;
          char sSelection[255];
          strcpy(sSelection, (char *) data + 3);
          if(!BGame::m_remotePlayer[nIndex].m_bSelectionMade) {
            BGame::m_remotePlayer[nIndex].m_sCurrentMenuSel = sSelection;
          }
        }
        break;

      case MENU_SELECTION:

        // Save menu selection

        {
          int nIndex = int(data[2]) - 1;
          char sSelection[255];
          strcpy(sSelection, (char *) data + 3);
          BGame::m_remotePlayer[nIndex].m_sCurrentMenuSel = sSelection;
          BGame::m_remotePlayer[nIndex].m_bSelectionMade = true;

          // Check if all have made selection. If so, highlight common selection

          BGame::GetView()->CheckForMultiplayMenuProceed();
        }
        break;

      case HIGHLIGHT_MENU_SELECTION:

        // Highlight commonly selection menu item

        {
          char sSelection[255];
          strcpy(sSelection, (char *) data + 2);
          BGame::GetView()->HighlightMenuSelection(sSelection);
        }
        break;

      case I_CHOSE_VEHICLE_FILENAME:

        // Save remote vehicle filename

        {
          int nIndex = int(data[2]) - 1;
          char sFilename[255];
          strcpy(sFilename, (char *) data + 3);

          if(BGame::m_remotePlayer[nIndex].m_pVehicle && 
             !BGame::m_remotePlayer[nIndex].m_bVehicleReused) {
            delete (BGame::m_remotePlayer[nIndex].m_pVehicle);
            BGame::m_remotePlayer[nIndex].m_pVehicle = 0;
            BGame::m_remotePlayer[nIndex].m_bVehicleReused = false;
          }

          BGame::m_remotePlayer[nIndex].m_sVehicleFilename = sFilename;
        }
        break;

      case MY_CAR_SIZE_IS:

        // Record remote vehicle size

        {
          int nIndex = int(data[2]) - 1;
          double dLen;
          double dWidth;
          double dHeight;
          double dMass;
          memcpy(&dLen,    data + 3,                      sizeof(double));
          memcpy(&dWidth,  data + 3 + 1 * sizeof(double), sizeof(double));
          memcpy(&dHeight, data + 3 + 2 * sizeof(double), sizeof(double));
          memcpy(&dMass,   data + 3 + 3 * sizeof(double), sizeof(double));
          BGame::m_remotePlayer[nIndex].m_dTotalMass = dMass;
          BGame::m_remotePlayer[nIndex].m_dLen    = dLen / 2.0;
          BGame::m_remotePlayer[nIndex].m_dHeight = dHeight; // To make collisions better
          BGame::m_remotePlayer[nIndex].m_dWidth  = dWidth / 2.0;
          BGame::m_remotePlayer[nIndex].m_dRadius = sqrt(BGame::m_remotePlayer[nIndex].m_dLen    * BGame::m_remotePlayer[nIndex].m_dLen +
                                                         BGame::m_remotePlayer[nIndex].m_dHeight * BGame::m_remotePlayer[nIndex].m_dHeight +
                                                         BGame::m_remotePlayer[nIndex].m_dWidth  * BGame::m_remotePlayer[nIndex].m_dWidth);
        }

        break;

      case MY_STATE_IS_NOW:

        // Record remote state

        {
          int nIndex = int(data[2]) - 1;
          BGame::m_remotePlayer[nIndex].m_state = static_cast<BRemotePlayer::TRemoteState>((*(data + 3)) - 1);
          if(BGame::m_remotePlayer[nIndex].m_state == BRemotePlayer::MISSED_POLE) {
            string sMsg;
            sMsg.assign(BGame::m_remotePlayer[nIndex].m_sName);
            sMsg.append("was disqualified");
            BGame::ShowMultiplayMessage(sMsg);
          }
        }

        break;

      case I_AM_IN_GOAL:

        {
          if(m_params.m_bHost) {

            // Report player final position

            int nIndex = int(data[2]) - 1;
            BGame::BroadcastFinalPosition(nIndex);
          }
        }
        break;

      case HIS_FINAL_POSITION_IS:

        // Record remote final position

        {
          int nIndex = int(data[2]) - 1;
          BGame::m_remotePlayer[nIndex].m_nRacePosition = static_cast<BRemotePlayer::TRemoteState>((*(data + 3)) - 1);
          BGame::m_remotePlayer[nIndex].m_state = BRemotePlayer::FINISHED;

          stringstream sMsg;
          sMsg << BGame::m_remotePlayer[nIndex].m_sName << "'s final position is " << BGame::m_remotePlayer[nIndex].m_nRacePosition;
          BGame::ShowMultiplayMessage(sMsg.str());
        }

        break;

      case I_AM_READY_TO_START_GAME:

        // Check if all are ready to start

        if(m_params.m_bHost) {
			cout << data[0] << " " << int(data[1]) << " " << int(data[2]) << " " << data_size << endl;
          int nIndex = int(data[2]) - 1;
          BGame::m_remotePlayer[nIndex].m_bReadyToStart = true;
		cout << nIndex << " is ready." << endl;
          // Check whether all are ready

          BGame::CheckForGameStart();
        }
        break;

      case START_GAME:

        // Start the race!

        BGame::m_bMultiplayRaceStarter = true;
        BGame::m_clockMultiRaceStarter = SDL_GetTicks();
        break;

      case CLOCK_IS_NOW_0:

        // Syncronize your watches!

        SDL_LockMutex(BGame::m_csMutex);
        BGame::m_clockOffsetFromZeroTime = SDL_GetTicks();
        SDL_UnlockMutex(BGame::m_csMutex);

        break;

      case CHAT_MESSAGE:

        // Show multiplayer chat message
        {
          string sMsg;
          sMsg = data + 2;
          BGame::ShowMultiplayMessage(sMsg, true);
        }

        break;
    }

    return;
  }

  // Process during-game messages

}


//*************************************************************************************************
bool BMultiPlay::EndMultiplaySession() {

  // Inform others that I have exited

  char bMsg[3];
  bMsg[0] = '-';
  bMsg[1] = BMultiPlay::I_EXITED;
  bMsg[2] = char(BGame::GetMyPlace() + 1);

  (void) SendBinaryBroadcastMsg(bMsg, 3);

  SDL_Delay(500); 

	//FIXME
  /*if(m_pDP) {
    m_pDP->Close(0);
    m_bDPInitialized = false;
  }*/

  BGame::m_bMultiplayOn = false;
  BGame::m_bExitingMultiplay = false;
  return true;
}

//*************************************************************************************************
//FIXME
/*int BMultiPlay::GetServiceProviders(string *psServiceProviders, GUID *pGuids, int nMax) {

  int nRet = 0;

  if(m_pDP) {
    HRESULT hr;
    DPN_SERVICE_PROVIDER_INFO *spInfoArray = 0;
    int dwEnumData = 0;
    int dwServiceProviders = 0;

    // First get the buffer size
    hr = m_pDP->EnumServiceProviders(NULL, // enumerate all service providers
                                     NULL, // enumerate all service providers
                                     spInfoArray,
                                     &dwEnumData,
                                     &dwServiceProviders,
                                     0); // DPNENUMSERVICEPROVIDERS_ALL

    spInfoArray = reinterpret_cast<DPN_SERVICE_PROVIDER_INFO *>(new char[dwEnumData]);

    // get service providers
    hr = m_pDP->EnumServiceProviders(NULL, // enumerate all service providers
                                     NULL, // enumerate all service providers
                                     spInfoArray,
                                     &dwEnumData,
                                     &dwServiceProviders,
                                     0); // DPNENUMSERVICEPROVIDERS_ALL

    // Transfer service providers to the string list
    for(int i = 0; (i < dwServiceProviders) && (i < nMax); ++i) {
      psServiceProviders[i] = spInfoArray[i].pwszName;
      pGuids[i] = spInfoArray[i].guid;

      // Remove "DirectPlay8 " from start
      if(psServiceProviders[i].Left(12).CompareNoCase("DirectPlay8 ") == 0) {
        psServiceProviders[i] = psServiceProviders[i].Right(psServiceProviders[i].GetLength() - 12);
      }

      // Remove "Service Provider" from middle
      int nOccurrence;
      if((nOccurrence = psServiceProviders[i].Find(" Service Provider", 0)) != -1) {
        // Make up the string from the two parts, removing "service provider" text
        string sTmp;
        sTmp.Format("%.*s%s", 
                    nOccurrence,
                    LPCTSTR(psServiceProviders[i]), 
                    LPCTSTR(psServiceProviders[i]) + nOccurrence + 17);
        psServiceProviders[i] = sTmp;
      }
    }
    nRet = i;

    delete [] spInfoArray;
  }
  return nRet;
}*/

bool BMultiPlay::send(char to_id, char *buffer, char len, int ignore_place) {
	//to_id -1 = send everyone
	char *new_buffer = new char[len + 2];
	new_buffer[0] = len;
	new_buffer[1] = to_id;
	memcpy(new_buffer + 2, buffer, len);
	
	len += 2;
	
	if(GetParams()->m_bHost) {
		for (int x = 0; x < BGame::m_nRemotePlayers; x++) {
			if (x != BGame::GetMyPlace() && x != ignore_place && (to_id == -1 || to_id == x)) {
				int result = SDLNet_TCP_Send(*BGame::m_remotePlayer[x].client_connection->getSocket(), new_buffer, len);

				if (result < len) {
					cout << "SDLNet_TCP_Send: " << SDLNet_GetError() << endl;
					delete[] new_buffer;
					return false;
				}
			}
		}
	} else {
		int result = SDLNet_TCP_Send(own_tcpsock, new_buffer, len);
		if (result < len) {
			cout << "SDLNet_TCP_Send: " << SDLNet_GetError() << endl;
			delete[] new_buffer;
			return false;
		}
	}
	delete[] new_buffer;
	return true;
}

bool BMultiPlay::receive(TCPsocket socket, char *to_id, char **buffer, char *len) {
	int result = SDLNet_TCP_Recv(socket, len, 1);
	if (result <= 0) {
		return false;
	}
	result = SDLNet_TCP_Recv(socket, to_id, 1);
	if (result <= 0) {
		return false;
	}
	*buffer = new char[*len];
	result = SDLNet_TCP_Recv(socket, *buffer, *len);
	if (result <= 0) {
		delete[] buffer;
		return false;
	}
	return true;
}

int BMultiPlay::receiveThread(void *ptr) {
	BMultiPlay *bmp = (BMultiPlay *)ptr;
	while (BGame::m_bMultiplayOn) {
		char to_id;
		char *buffer;
		char len;
		if (bmp->receive(bmp->own_tcpsock, &to_id, &buffer, &len)) {
			bmp->ProcessMultiplayMessage(buffer, len);
			delete[] buffer;
		}
	}
	
	return 0;
}

int BMultiPlay::listenIncomingConnections(void *ptr) {
	BMultiPlay *bmp = (BMultiPlay *)ptr;

	while (BGame::m_bMultiplayOn) {
		TCPsocket socket = SDLNet_TCP_Accept(bmp->own_tcpsock);
		
		if (socket) {
			cout << "We have a new connection!" << endl;
			bmp->client_connections.push_back(new BClientConnection(socket, bmp));
		}
		
		SDL_Delay(200);
	}

	return 0;
}

BClientConnection::BClientConnection(TCPsocket socket, BMultiPlay *bmp) {
	thread_exit = false;
	this->socket = socket;
	this->bmp = bmp;
	listen_thread = SDL_CreateThread(receiveThread, "listen", (void *)this);
}

BClientConnection::~BClientConnection() {
	thread_exit = true;
	SDL_WaitThread(listen_thread, NULL);
}

int BClientConnection::receiveThread(void *ptr) {
	BClientConnection *bcc = (BClientConnection *)ptr;

	while (!bcc->thread_exit) {
		char to_id;
		char *buffer;
		char len;
		if (bcc->bmp->receive(bcc->socket, &to_id, &buffer, &len)) {
			//find sender place if there is one
			int sender_place = 0;
			while (BGame::m_remotePlayer[sender_place].client_connection != bcc && sender_place != BGame::m_nRemotePlayers) {
				sender_place++;
			}
			//if (to_id == -1) {
				//resend to all
				bcc->bmp->send(to_id, buffer, len, sender_place);
			//} else if (to_id != BGame::GetMyPlace()) {
				//resend to to_id
				//*BGame::m_remotePlayer[x].socket sendaa tonne
			//}
			
			if (to_id == BGame::GetMyPlace() || to_id == -1) {
				//we need to know about the content
				bcc->bmp->ProcessMultiplayMessage(buffer, len, bcc);
			}
		
			delete[] buffer;
		}
	}
	
	return 0;
}

#endif

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
