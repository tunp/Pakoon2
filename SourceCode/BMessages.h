//
// BMessages: Show on-screen messages
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#pragma once

#include "BaseClasses.h"
#include "BGame.h"
#include "BSimulation.h"
#include "BCmdModule.h"

#include <string>

using namespace std;

//*****************************************************************************
class BLetter {
public:
  char m_cParts[15]; 
};

//*****************************************************************************
class BMessage {
public:
  int       m_nCharPartWidth;
  int       m_nCharPartGap;
  int       m_nCharWidth;
  int       m_nCharHeight;
  int       m_nCharGap;
  int       m_nTotalWidth;
  double    m_dR;
  double    m_dG;
  double    m_dB;
  bool      m_blink;
  int       m_nCharsPerLine;
  string   m_sId;
  string   m_sText;
  time_t    m_timeRemove;
  unsigned   m_clockStarted;
  bool      m_bFaint;
  bool      m_bSecondary;
  BMessage *m_pNext;
};

//*****************************************************************************
class BMessages {
  static BLetter   m_letters[65];
  static BMessage *m_pMessages;
public:
  BMessages();
  ~BMessages();

  static void Show(int nCharsPerLine, 
                   string sId, 
                   string sText, 
                   int nSeconds, 
                   bool blink = false, 
                   double dR = 1, 
                   double dG = 1, 
                   double dB = 1,
                   bool bFaint = false,
                   bool bSecondary = false);
  static void      Remove(string sId);
  static void      RemoveAll();
  static void      Render();
  static bool      Find(string sId);

  static BMessage *RenderMessage(BMessage *pMsg);
  static void      RenderLetter(BMessage *pMsg, char c, bool bInside);
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
