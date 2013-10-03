//
// BMenu: start menu components
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#pragma once

#include "BaseClasses.h"
#include "BTextRenderer.h"
#include "BUI.h"

#include <string>
#include <ctime>

using namespace std;

//*************************************************************************************************
class BMenuItem {
public:

  enum TType {BASIC, STRING_FROM_LIST, SLIDER, EDITBOX};

  TType    m_type;
  string  m_sText;
  string  m_sValue;
  int      m_nValue;
  int      m_nValue2;
  string  m_sAssocFile;
  int      m_nAssocImage;
  BUIEdit  m_ebAssocEditBox;
  bool     m_bDisabled;
  int      m_nAssocListItems;
  string *m_sAssocListItems;
  BUISelectionList m_listMenu;
  bool     m_bOpen;
  double   m_dRed;
  double   m_dGreen;
  double   m_dBlue;

  BMenuItem() {m_dRed = m_dGreen = m_dBlue = 1.0;
               m_type = BASIC; 
               m_nValue = 0; 
               m_sText = ""; 
               m_sAssocFile = ""; 
               m_nAssocImage = -1; 
               m_bOpen = false; 
               m_bDisabled = false;}
};


//*************************************************************************************************
class BMenu {
public:

  enum TType {MAIN, 
              GAMEMODE, 
              MULTIPLAYER, 
              CHOOSE_SCENE, 
              CHOOSE_VEHICLE, 
              PRECACHING_TERRAIN, 
              SETTINGS, 
              CREDITS, 
              HISCORES,
              GAME};

  string    m_sName;
  TType      m_type;
  int        m_nItems;
  BMenuItem *m_items;
  string   *m_sItems;

  int        m_nTitleWidth;
  int        m_nTitleHeight;
  double     m_dTitleX;
  double     m_dTitleY;

  bool       m_bDrawLine;
  clock_t    m_clockStarted;

  BUISelectionList m_listMenu;
  BTextRenderer::TTextAlign m_align;

  BMenu();
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
