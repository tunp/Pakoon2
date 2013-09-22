//
// BUI: Misc UI components
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#pragma once

#include "BaseClasses.h"
#include "BTextRenderer.h"

class CPakoon1View;
class BMenu;
class BMenuItem;


//*************************************************************************************************
class BUIEdit {
  int     m_nMaxLength;
  CString m_sPrompt;

public:

  CString m_sValue;
  enum TStatus {EDITING, READY, CANCELED};
  TStatus status;

public:
  BUIEdit();
  void    Setup(CString sPrompt, CString sValue, int nMaxLength);
  void    ProcessChar(unsigned char c);
  CString GetValue(TStatus &rStatus);
  void    DrawAt(double dX, 
                 double dY, 
                 bool bCursor,
                 BTextRenderer::TTextAlign textAlign = BTextRenderer::TTextAlign::ALIGN_CENTER,
                 BMenuItem *pMenuItem = 0);
};


//*************************************************************************************************
class BUISelectionList {
  CString  m_sPrompt;
public:
  int      m_nSelected;
  CString *m_psItems;
  int      m_nSavedSelection;
  int      m_nItems;
public:
  double   m_dOffsetToLeft;
  BUISelectionList();
  void SetItems(CString *psItems, int nItems, CString sPrompt = "");
  int  GetNofItems() {return m_nItems;}
  int  SelectItem(CString sItem);
  void AdvanceSelection(int nAmount);
  void SaveSelection();
  void Cancel();
  int  GetSelectedItem(CString &sItemText);
  void DrawAt(double dX, 
              double dY, 
              BTextRenderer::TTextAlign textAlign = BTextRenderer::TTextAlign::ALIGN_LEFT,
              double dRed = 1, 
              double dGreen = 1, 
              double dBlue = 1,
              bool bScrolling = true,
              bool bWithBackground = false,
              bool bFlashCursor = true,
              BMenu *pMenu = 0,
              bool   bWithTriangle = true);
};


//*************************************************************************************************
class BUI {
public:
  BUI();

  static BTextRenderer    *TextRenderer()  {return &m_textRenderer;}
  static BUISelectionList *GetActiveSelectionList() {return m_pSelList;}
  static void              StartUsingSelectionList(BUISelectionList *pList, 
                                                   void (CPakoon1View::*pPrevKeyDownFunction)(UINT, UINT, UINT));
  static void (CPakoon1View::*StopUsingSelectionList())(UINT, UINT, UINT) {m_pSelList = 0; return m_pPrevKeyDownFunction;}
  static void              StartUsingSlider(int *pnSliderValue, 
                                            void (CPakoon1View::*pPrevKeyDownFunction)(UINT, UINT, UINT));
  static void (CPakoon1View::*StopUsingSlider())(UINT, UINT, UINT) {m_pnSliderValue = 0; return m_pPrevKeyDownFunction;}

  static void              StartUsingEditbox(CString *psValue, 
                                             void (CPakoon1View::*pPrevKeyDownFunction)(UINT, UINT, UINT));
  static void (CPakoon1View::*StopUsingEditbox())(UINT, UINT, UINT) {return m_pPrevKeyDownFunction;}

  static CString              m_sPrevSValue;
  static CString             *m_psValue;
  static int                  m_nPrevSliderValue;
  static int                 *m_pnSliderValue;
  static BTextRenderer        m_textRenderer;
  static BUISelectionList    *m_pSelList;
  static void (CPakoon1View::*m_pPrevKeyDownFunction)(UINT, UINT, UINT);
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