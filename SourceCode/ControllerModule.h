//
// All access controller module for all users of the game
// 
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#pragma once

#include "BaseClasses.h"

#include <string>

using namespace std;

class BControllerState {
public:
  double m_dTurn;         // -1 to +1, -1 = full left, +1 = full right
  double m_dAcceleration; // 0 to +1, +1 = full acceleration
  double m_dReverse;      // 0 to +1, +1 = full reverse
  double m_dBrake;        // 0 to +1, +1 = full break
  bool   m_bJetEvent;     // Jet requested
  bool   m_bHeliEvent;    // Heli requested
  bool   m_bLift;         // Lift on
  bool   m_bCameraEvent;  // Camera requested
};

class BKeyMap {
public:
  unsigned m_unPropeller;
  unsigned m_unPropellerReverse;
  unsigned m_unLeft;
  unsigned m_unRight;
  unsigned m_unAccelerate;
  unsigned m_unReverse;
  unsigned m_unBreak;
  unsigned m_unJet;
  unsigned m_unHeli;
  unsigned m_unLift;
  unsigned m_unCamera;

  BKeyMap();
};

class BControllerMapItem {
public:
  enum TJoyItem{X = 0, Y, Z, XROT, YROT, ZROT, SLIDER1, SLIDER2};

  bool     m_bButton;
  int      m_nButton;
  bool     m_bWhole;
  bool     m_bPlusSide;
  bool     m_bFlipped;
  TJoyItem m_jiPart;

  BControllerMapItem();
  //string ToString(bool& rbFlipped);
  //bool    FromString(string sItem);
};

class BControllerMap {
public:
  bool               m_bButtonState[32];
  BControllerMapItem m_cmiLeft;
  BControllerMapItem m_cmiRight;
  BControllerMapItem m_cmiAccelerate;
  BControllerMapItem m_cmiReverse;
  BControllerMapItem m_cmiBreak;
  BControllerMapItem m_cmiJet;
  BControllerMapItem m_cmiHeli;
  BControllerMapItem m_cmiLift;
  BControllerMapItem m_cmiCamera;
  BControllerMap();
};

class ControllerModule {
public:
  static bool           m_bInitialized;
  //static LPDIRECTINPUT  m_lpDirectInput;
  static int            m_nControllers;
  static string        m_sControllers[20];
  //static GUID           m_guids[20];
  static int            m_nCurrent;
  static bool           m_bCurrentInitialized;
  //static HWND           m_hwnd;
  //static LPDIRECTINPUTDEVICE2 m_pDIJoystick;
  static BKeyMap        m_keymap;
  static BControllerMap m_controllermap;

       ControllerModule();
  //static void Initialize(HWND hwndFrameWnd);
  //static bool SwitchToController(int nIndex);
  //static bool GetControllerState(BControllerState& rcs, DIJOYSTATE *pRawState);

  //static double GetValueFromRaw(BControllerMapItem* pcmi, DIJOYSTATE *pRawState);
  //static bool   GetButtonEventFromRaw(BControllerMapItem* pcmi, DIJOYSTATE *pRawState);
  //static bool   GetButtonValueFromRaw(BControllerMapItem* pcmi, DIJOYSTATE *pRawState);
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
