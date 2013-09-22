//
// BGame: Center location for controlling the gameplay
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#pragma once

#include "BaseClasses.h"
#include "BSimulation.h"
#include "BPlayer.h"
#include "BCmdModule.h"
#include "BNavSatWnd.h"
#include "BServiceWnd.h"
#include "BSceneEditor.h" 
#include "BMenu.h"
#include "BUI.h"

// DirectPlay stuff
#include <dplay8.h>

class CPakoon1View;



//*****************************************************************************
class BMultiplayParams {
public:
  bool    m_bHost;
  CString m_sHostIPAddress;
  CString m_sPlayerName;
  int     m_nMyPlace;
  clock_t m_clockPosLastSent;

  BMultiplayParams() {m_bHost = true; 
                      m_sHostIPAddress = m_sPlayerName = _T(""); 
                      m_nMyPlace = 0;
                      m_clockPosLastSent = 0;}
};

//*****************************************************************************
class BMultiPlay {

  IDirectPlay8Peer *m_pDP;
  bool              m_bDPInitialized;

public:

  BMultiplayParams  m_params;

  enum TTinyMessages{WHO_AM_I = 1,
                     YOU_ARE,
                     PLAYER_INFO,
                     MENU_BROWSE,
                     MENU_SELECTION,
                     HIGHLIGHT_MENU_SELECTION,
                     MY_CAR_SIZE_IS,
                     I_CHOSE_VEHICLE_FILENAME,
                     I_AM_READY_TO_START_GAME,
                     START_GAME,
                     MY_POSITION_IS_THIS,
                     CLOCK_IS_NOW_0,
                     MY_STATE_IS_NOW,
                     I_AM_IN_GOAL,
                     HIS_FINAL_POSITION_IS,
                     HOST_YOUR_ID_IS,
                     REPORT_YOUR_STATE,
                     CHAT_MESSAGE,
                     I_EXITED};

  BMultiPlay();
  ~BMultiPlay();

  BMultiplayParams *GetParams() {return &m_params;}

  bool InitMultiplaySession();
  bool StartMultiplaySession(BMultiplayParams *pParams);
  bool EndMultiplaySession();
  int  GetServiceProviders(CString *psServiceProviders, GUID *pGuids, int nMax);

  bool SendPeerMsg(BYTE bMsg, DPNID id, CString sMsgText);
  bool SendBroadcastMsg(BYTE bTinyMsg, CString sMsgText);
  bool SendBinaryBroadcastMsg(BYTE *pbMsg, int nSize);

  void ProcessMultiplayMessage(PDPNMSG_RECEIVE pReceiveMsg);
};



//*****************************************************************************
class BRemotePlayer {
public:

  enum TRemoteState {WANTS_TO_SELECT_NEW_RACE, 
                     PREPARING_TO_RACE, 
                     WAITING_FOR_RACE, 
                     RACING, 
                     FINISHED, 
                     MISSED_GOAL,
                     MISSED_POLE};

  BRemotePlayer() {m_bReadyToStart = false; 
                   m_bSelectionMade = false; 
                   m_bSelf = false;
                   m_vVelocity.Set(0, 0, 0);
                   m_vVelo1stDeriv.Set(0, 0, 0);
                   m_vVelo2ndDeriv.Set(0, 0, 0);
                   m_pVehicle = 0;
                   m_sVehicleFilename = _T("");
                   m_bVehicleReused = false;
                   m_state = WANTS_TO_SELECT_NEW_RACE;}

  ~BRemotePlayer();

  // ID stuff

  bool         m_bSelf;
  DPNID        m_id;
  char         m_sName[100];     // Remote Player name

  // Position stuff

  DWORD        m_clockLocationReceived;
  DWORD        m_clockLocationSent;
  long         m_nLocationSent;
  long         m_nPrevLocationSent;
  BVector      m_vLocation;
  BOrientation m_orientation;
  DWORD        m_clockPrevLocationSent;
  BVector      m_vLocationPrev;
  BOrientation m_orientationPrev;
  BVector      m_vVelocity; // A vector that tells where car travels in one clock tick
  BVector      m_vVelo1stDeriv;
  BVector      m_vVelo2ndDeriv;
  BVector      m_vOnScreen;

  // Vehicle stuff

  BVehicle    *m_pVehicle;  // pointer to a vehicle (for visualization purposes only)
  CString      m_sVehicleFilename; // Filename where the m_pVehicle was loaded from
  bool         m_bVehicleReused;

  double       m_dWidth;    // Actually, half width
  double       m_dLen;      // Actually, half length
  double       m_dHeight;   // Actually, half height
  double       m_dRadius;   // Radius of the car box
  double       m_dTurn;
  double       m_dTotalMass;

  // Multiplay control stuff

  TRemoteState m_state;
  int          m_nRacePosition;

  CString      m_sCurrentMenuSel; 
  bool         m_bSelectionMade;
  bool         m_bReadyToStart;
};



//*****************************************************************************
class BGame {

  static BMultiPlay        m_multiplay;
  static BSimulation       m_simulation;
  static BPlayer           m_player;
  static BCmdModule        m_cmdModule;
  static BNavSatWnd        m_navsatWnd;
  static BServiceWnd       m_serviceWnd;
  static BSceneEditor      m_sceneEditor;

  static clock_t           m_clockFrozenStart;
  static int               m_nFreezeRefCount;

  static bool    CheckHighscoresValidity();
  static void    ValidateHighscores();
  static CString GetHighscoresChecksum();

public:

  static CPakoon1View     *m_pView;

  enum TGameMode{SLALOM, AIRTIME, SPEEDRACE};
  enum TOnScreenInfo{FPS = 1, 
                     DISQUALIFIED_WRONG_SIDE = 2, 
                     DISQUALIFIED_OUT_OF_TIME = 4, 
                     DISQUALIFIED_GOAL = 8, 
                     TIMER_STARTED = 16,
                     NEW_RECORD = 32,
                     REF_TIME = 64};

  static TGameMode m_gameMode;

  static int  m_nDispWidth;
  static int  m_nDispHeight;
  static int  m_nDispBits;
  static int  m_nDispHz;
  static int  m_nTerrainResolution;
  static int  m_nDustAndClouds;
  static int  m_nCarDetails;
  static int  m_nWaterSurface;
  static int  m_nMusicVolume;
  static int  m_nVehicleVolume;
  static int  m_nSoundscape;
  static int  m_nSkyDetail;
  static int  m_nDistantDetail;
  static int  m_nWaterDetail;
  static int  m_nColorMode;
  static int  m_nScreenFormat;
  static int  m_nTextureSmoothness; 
  static int  m_nPhysicsSteps; 
  static bool m_bSlowMotion;  // "Bullet-time"
  static char m_cOnScreenInfo; // Extra on screen messages
  static bool m_bNavSat;      // Show Nav-Sat map?
  static bool m_bService;     // Show service window?
  static int  m_nShowEffects;    // Show light etc. effects?
  static bool m_bShowQuickHelp;
  static bool m_bDrawOnScreenTracking;
  static bool m_bFrozen;
  static bool m_bShowHint;
  static clock_t m_clockHintStart;
  static clock_t m_clockLastLift;
  static double m_dNavSatHandleAngle;
  static double m_dServiceHandleAngle;
  static int    m_nGameMenuSelection;
  static int    m_nYesNoSelection;
  static bool   m_bShowGameMenu;
  static bool   m_bShowCancelQuestion;
  static bool   m_bSceneEditorMode;
  static bool   m_bFadingIn;
  static clock_t m_clockFadeStart;
  static BMenu *m_pMenuCurrent;
  static BMenu *m_pMenuPrevious;
  static bool   m_bMenusCreated;
  static bool   m_bMenuMode;

  static CString m_sScene;
  static CString m_sVehicle;

  static BMenu m_menuMain;
  static BMenu m_menuMultiplay;
  static BMenu m_menuChooseGameMode;
  static BMenu m_menuChooseScene;
  static BMenu m_menuChooseVehicle;
  static BMenu m_menuSettings;
  static BMenu m_menuCredits;
  static BMenu m_menuHiscores;
  static BMenu m_menuPrecachingTerrain;
  static bool  m_bSettingsFromGame;

  static BMenu m_menuGame;

  static bool m_bGameLoading;
  static bool m_bLoadGame;
  static bool m_bGameReadyToStart;
  static bool m_bQuitPending;

  static CRITICAL_SECTION m_csMutex;
  static double m_dProgressMax;
  static double m_dProgressPos;

  static bool   m_bJumpToHome;
  static bool   m_bBuyingVehicle;
  static bool   m_bCannotBuyVehicle;
  static double m_dPurchasePrice;

  static bool    m_bMultiProcessor;

  static bool    m_bAnalyzerMode;
  static clock_t m_clockAnalyzerStarted;
  static int     m_nVisualize;

  static BUISelectionList m_listYesNo;
  static BUISelectionList m_listOK;

  static BUISelectionList m_listHSSpeedrace;
  static BUISelectionList m_listHSSlalom;
  static BUISelectionList m_listHSAirtime;

  static bool    m_bRaceStarted;
  static bool    m_bRaceFinished;
  static double  m_dRaceTime;
  static double  m_dAirTime;

  static bool    m_bRecordSlalom;
  static bool    m_bPassFromRightSlalom;
  static CString m_sRacePosition;

  static bool    m_bSlalomPolesVisualOK;

  static bool    m_bForceBreak;
  static double  m_dLiftStarted;

  static bool    m_bNight;

  static int           m_nRemotePlayers;
  static BRemotePlayer m_remotePlayer[4];

  static double     m_dRefTime[7];
  static int        m_nRefK;
  static CString    m_sRefTime;

  static bool       m_bMultiplayOn;
  static bool       m_bExitingMultiplay;
  static bool       m_bOKToProceedInMultiplayMenu;
  static bool       m_bMultiplayRaceStarter;
  static clock_t    m_clockMultiRaceStarter;
  static DWORD      m_clockOffsetFromZeroTime;
  static int        m_nPlayersInGoal;
  static DWORD      m_nMultiplayPort;

  static int        m_nMultiplayMessages;
  static CString    m_sMultiplayMessages[5];
  static bool       m_bChatMessage[5];
  static clock_t    m_clockMultiplayMessages[5];
  static bool       m_bTABChatting;
  static CString    m_sChatMsg;

  static GUID       m_guidServiceProviders[10];

  // static BUISelectionList m_sellistGameMenu;

  static int              m_nController;
  static BControllerState m_controllerstate; // Access to a controller, such as a joystick or a wheel

  BGame();
  ~BGame();

  // Multiplay stuff
  static int            AddRemotePlayer(DPNID id, BYTE *pPlayerName);
  static void           HandlePlayerExit(BYTE *pPlayerInfo);
  static void           HandlePlayerAbnormalExit(DPNID id);
  static void           UpdatePlayerInfo(BYTE *pPlayerInfo);
  static void           CheckForGameStart();
  static void           GetMultiplayerColor(int nIndex, double &dR, double &dG, double &dB);
  static void           BroadcastCarPosition();
  static void           BroadcastCarSize();
  static void           BroadcastStateChange();
  static void           BroadcastInGoal();
  static void           BroadcastFinalPosition(int nIndex);
  static void           ShowMultiplayMessage(CString sMsg, bool bChat = false);
  static void           RemoveOldestMultiplayMessage(bool bForce = false);
  static int            GetMyPlace() {return GetMultiplay()->GetParams()->m_nMyPlace;}

  // General stuff
  static BMultiPlay    *GetMultiplay()   {return &m_multiplay;}
  static BSimulation   *GetSimulation()  {return &m_simulation;}
  static BPlayer       *GetPlayer()      {return &m_player;}
  static BCmdModule    *Command()        {return &m_cmdModule;}
  static BNavSatWnd    *GetNavSat()      {return &m_navsatWnd;}
  static BServiceWnd   *GetServiceWnd()  {return &m_serviceWnd;}
  static BSceneEditor  *GetSceneEditor() {return &m_sceneEditor;}
  static CPakoon1View  *GetView()        {return m_pView;}
  static void           SetView(CPakoon1View *pView) {m_pView = pView;}

  static void           SetupScene();

  static void           SetProgressRange(double dMax);
  static void           SetProgressPos(double dPos);
  static double         GetRelativeProgress();

  static CString        GetScrambleChecksum();
  static CString        GetVerifyChecksum(CString sSource);

  static void           UpdateHighScores(CString sSceneName, TGameMode gameMode, double dTime);
  static void           UpdateHighScoreMenu();

  static void           FreezeSimulation(bool bPause = false);
  static clock_t        ContinueSimulation();
  static void           SetupMenus();
  static void           SetupMultiplayMenu();
  static void           UpdateSettings();
  static void           EnumerateScreenResolutions();
  static bool           FindStringFromArray(CString s, CString *psArray, int nItems, int &rnIndex);

  static DWORD          GetMultiplayClock() {return ::GetTickCount() - m_clockOffsetFromZeroTime;}

  static double         GetSmoothAlpha();


  enum TAnalyzerVis {SKY = 1,
                     WATER = 2,
                     TERRAIN = 4,
                     VEHICLE = 8,
                     GASSTATIONS = 16,
                     OBJECTS = 32,
                     DUSTANDCLOUDS = 64,
                     GRAPHICS2D = 128,
                     ALL = 255};

  static void           UpdateAnalyzer();

  static void           MyAfxMessageBox(CString sText, int nTmp = 0);
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
