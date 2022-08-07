//
// Pakoon1View.h : interface of the CPakoon1View class
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#if !defined(AFX_Pakoon1VIEW_H__73E6FC07_BC82_11D4_B532_0060B0F1F5DD__INCLUDED_)
#define AFX_Pakoon1VIEW_H__73E6FC07_BC82_11D4_B532_0060B0F1F5DD__INCLUDED_

//#if _MSC_VER > 1000
#pragma once
//#endif // _MSC_VER > 1000

//#include "STRMIF.H"
//#include "CONTROL.H"
//#include "UUIDS.H"
#include "BaseClasses.h"
#include "BGame.h"
#include "BMessages.h"

#include <string>

using namespace std;

class BMenuItemOBSOLETE {
public:
  SDL_Rect   m_rect;
  bool    m_bActive;
  int     m_nLight;
  //BMenuItemOBSOLETE() {m_rect.SetRectEmpty(); m_bActive = false; m_nLight = 0;}
};


class BSelectionHelper {
  public:
    int     m_nRefCount;
    string m_sSelection;
};



class CPakoon1View {
  SDL_Rect     m_rectWnd;
  BGame     m_game;
  BMessages m_messages;

  bool    m_bInitClock;

  bool    m_bWireframe;
  bool    m_bNormals;
  bool    m_bCreateDLs;

  //HCURSOR m_hCursor;

  BMenuItemOBSOLETE m_miMenu[7];

  //HGLRC   m_hGLRC;


public:
  unsigned m_nMenuTime; // Time spent viewing menu
  unsigned m_clockTimerStart;
  unsigned m_clockHighlightMenu;
private:

  int     m_chaMenuMusic; // Channel for the menu background music

  bool    m_bIgnoreNextChar;

  //SDL_Thread *m_pThreadLoading;

  enum TMenuScroll {SCROLL_LEFT, SCROLL_RIGHT, SCROLL_UP, SCROLL_DOWN};

  TMenuScroll m_scrollDir;
  unsigned     m_clockMenuScroll;

  void ProcessMouseInput(unsigned nFlags, SDL_Point point);

  void StartMenuScroll(TMenuScroll scroll);
  void DrawTexturedSphere();
  void DrawEarth(BMenu *pMenu);
  void DrawHiscores(BMenu *pMenu);
  void DrawMenu(BMenu *pMenu);
  void DrawMenuItemTextAtRelPos(int nX, int nY, int nItems, int nIndex, BMenuItem *pMenuItem);
  void DrawMenuItemSliderAtRelPos(int nX, int nY, int nItems, int nIndex, BMenuItem *pMenuItem);
  void DrawMenuItemEditBoxAtRelPos(int nX, int nY, int nItems, int nIndex, BMenuItem *pMenuItem);
  //void DrawMenuItem(CFont *pFont, int m, int nY, string sText, COLORREF color, SDL_Rect rectWnd);
  void DrawMenuTitle(BMenu *pMenu, double dAlpha, bool bFirstTime);
  void PrepareReferenceTimes(BRaceRecord &raceRecord);
  void DrawOldTubeEffect();
  void FixCarToBasicOrientation(double dSpeedFactor);

#ifdef ENABLE_MULTIPLAY
  void DrawMultiplayMenuStuff(BMenu *pMenu, double dX, double dY);
  void DrawRemoteMenuTriangleAt(BMenu *pMenu, int i, int nRelativeTriPos, int nPlayer);
  void DrawMultiplayMessage(int i, string sMsg, double dAlpha, bool bNormal, bool bChatColor);
  void DrawMultiplayMessages();
  void DrawFinalPosition();
#endif

  void CheckForGameStart();

public:

  bool m_bDrawOnlyMenu;
  bool m_bFullRedraw;

  void OnDrawIntro();     // OBSOLETE
  void OnDrawCurrentMenu();
  void OnDrawGame();

  void OnKeyDownGame          (unsigned nChar, unsigned nRepCnt, unsigned nFlags);
  void OnKeyDownSceneEditor   (unsigned nChar, unsigned nRepCnt, unsigned nFlags);
  void OnKeyDownSelectionList (unsigned nChar, unsigned nRepCnt, unsigned nFlags);
  void OnKeyDownSlider        (unsigned nChar, unsigned nRepCnt, unsigned nFlags);
  void OnKeyDownEditbox       (unsigned nChar, unsigned nRepCnt, unsigned nFlags);
  void OnKeyDownCurrentMenu   (unsigned nChar, unsigned nRepCnt, unsigned nFlags);
  void OnKeyDownIntro         (unsigned nChar, unsigned nRepCnt, unsigned nFlags);

  void OnKeyDownTABChatting(unsigned nChar);

  void ReturnPressedOnCurrentMenu();
  void CancelPressedOnCurrentMenu();
  void ReturnPressedOnGameMenu();
  void CancelPressedOnGameMenu();

  void Setup2DRendering();
  void End2DRendering();
  void DrawNavSat();
  void DrawQuickHelp();
  void DrawServiceWnd();
  void DrawServiceWndTexts(SDL_Rect &rectWnd);
  void DrawSrvText(string sText);
  void DrawPanel(double dWidth, 
                 double dHeight, 
                 double dRed = 0, 
                 double dGreen = 0, 
                 double dBlue = 0, 
                 double dAlpha = 1);


  void DrawKeyboardHint();
  void DrawGameMenu();

  void DrawDisqualified();
  void DrawGoalArrow(BVector vGoal);
  void DrawOnScreenGameTexts(BVector vGoal);
  void DrawExtraScreenTexts();
  void DrawGameStringAt(string sTxt, double dX, double dY);

  void DrawRemoteCars();
  void DrawGhostCar(int &nNowFrame, BRaceRecord *pRaceRecord);
  void DrawActiveSlalomPoles();

  void DrawMouseCursor(SDL_Rect &rectWnd);
  BVector ColorForGForce(double dGForce);

#ifdef ENABLE_MULTIPLAY
  void BroadcastMenuBrowse();
  void BroadcastMenuSelection();
  void BroadcastSelectedVehicleFilename(string sFilename);

  void CheckForMultiplayMenuProceed();
#endif
  void HighlightMenuSelection(string sSelection);

private:

  //IMediaControl *m_pMediaControl;

public: // create from serialization only
  CPakoon1View();
  //DECLARE_DYNCREATE(CPakoon1View)

// Attributes
public:
  void (CPakoon1View::*m_pDrawFunction)();
  void (CPakoon1View::*m_pKeyDownFunction)(unsigned, unsigned, unsigned);

  //CPakoon1Doc* GetDocument();

// Operations
public:

// Overrides
// ClassWizard generated virtual function overrides
//{{AFX_VIRTUAL(CPakoon1View)
public:
virtual void OnDraw();  // overridden to draw this view
//virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
//}}AFX_VIRTUAL

void InitializeOpenGL();

// Implementation
public:
  virtual ~CPakoon1View();
  #ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext& dc) const;
  #endif

protected:

// Generated message map functions
public:
  //{{AFX_MSG(CPakoon1View)
  int  OnCreate();
  bool OnEraseBkgnd();
  void OnDestroy();
  void OnChar(unsigned nChar, unsigned nRepCnt, unsigned nFlags);
  void OnKeyDown(unsigned nChar, unsigned nRepCnt, unsigned nFlags);
  void OnKeyUp(unsigned nChar, unsigned nRepCnt, unsigned nFlags);
  void OnMouseMove(unsigned nFlags, SDL_Point point);
  void OnLButtonDown(unsigned nFlags, SDL_Point point);
  void OnLButtonUp(unsigned nFlags, SDL_Point point);
  //}}AFX_MSG
  //DECLARE_MESSAGE_MAP()
  
  bool exit;
  SDL_Window *window;
  
  bool isExit() {
	  return exit;
  }
  
  void setExit() {
	  exit = true;
  }
  
  void setWindow(SDL_Window *window) {
	  this->window = window;
  }
  
  void setRectWnd(SDL_Rect rect) {
	  m_rectWnd = rect;
  }
};

//#ifndef _DEBUG  // debug version in Pakoon1View.cpp
//inline CPakoon1Doc* CPakoon1View::GetDocument()
//   { return (CPakoon1Doc*)m_pDocument; }
//#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_Pakoon1VIEW_H__73E6FC07_BC82_11D4_B532_0060B0F1F5DD__INCLUDED_)



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
