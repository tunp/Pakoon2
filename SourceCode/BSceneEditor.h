//
// BSceneEditor: Simple scene editor
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#pragma once

#include "BaseClasses.h"
#include "BUI.h"

#include <SDL2/SDL.h>
#include <string>

using namespace std;

class BObject;

//*****************************************************************************
class BSceneEditor {

  bool m_bSceneModified;

  string m_sAllObjects[1000];
  int     m_nAllObjects;
  string m_sObjectTypes[1];
  string m_sObjectShadows[2];
  string m_sOverwrite[2];

  void RecurseObjectFiles(string sPath, string sSubDir);
  void FetchAllObjects();

public:

  string          m_sActiveObject;
  string          m_sObjectType;
  string          m_sObjectShadow;
  BUIEdit          m_edit;
  BUISelectionList m_sellistAllObjects;
  BUISelectionList m_sellistObjectType;
  BUISelectionList m_sellistObjectShadow;
  BUISelectionList m_sellistOverwrite;

  enum TPhase {BASIC, 
               ASKING_OBJECT_NAME, 
               SELECTING_OBJECT_TYPE,
               SELECTING_OBJECT_SHADOW,
               SELECTING_OBJECT_FILE, 
               MOVING_OBJECT,
               SELECTING_SCENE_OBJECT,
               SELECTING_SCENE_OBJECT_TO_DELETE,
               ASKING_SCENE_DISPLAY_NAME,
               ASKING_SCENE_FILENAME,
               OVERWRITE_SCENE_FILE_QUESTION};

  TPhase m_phase;

  BSceneEditor();
  void Activate();
  void Deactivate();
  bool IsActive();
  void Draw(SDL_Rect &rectWnd);
  void HighlightActiveObject();
  void AdvancePhase();
  void CancelPhase();
  BObject *GetActiveObject();
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
