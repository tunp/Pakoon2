//
// Scene
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#pragma once

#ifdef _WIN32
#include <windows.h>
#endif

#include "BaseClasses.h"
#include "BObject.h"
#include "BTerrain.h" 
#include "HeightMap.h"
#include "BUI.h"
#include <GL/gl.h>
#include <GL/glu.h>

#include <string>

using namespace std;


class BRecordFrame {
public:
  double  m_dTime;
  BVector m_vLocation;
  BVector m_vForward;
  BVector m_vRight;
};

class BRaceRecord {
public:
  bool   m_bValid;
  int    m_nNextSlot;
  int    m_nMaxSlot;
  double m_dCarLength;
  double m_dCarWidth;
  double m_dCarHeight;
  double m_dTotalTime;
  BRecordFrame m_frames[30 * 200];

  BRaceRecord() {m_bValid = false; m_nNextSlot = 0; m_nMaxSlot = 30 * 200; m_dTotalTime = 0.0;}
};


class BSlalomPole {
public:
  BVector m_vLocation;
  bool    m_bPassFromRight;
  BSlalomPole() {m_bPassFromRight = true;}
};
  

class BSlalom {
public:
  bool        m_bValid;
  int         m_nSlalomPoles;
  int         m_nCurrentPole;
  BSlalomPole m_slalomPole[200];
  BSlalom() {m_nSlalomPoles = m_nCurrentPole = 0;}
};

class BScene {

public:

  // General properties

  string       m_sFilename;
  string       m_sName;
  string       m_sImageFilename;
  double        m_dFriction;
  double        m_dGroundTextureScaler1;
  double        m_dGroundTextureScaler2;
  string       m_sSkyTexture;
  string       m_sGround1Texture;
  string       m_sGround2Texture;
  string       m_sEnvMapTexture;
  bool          m_bSceneInUse;  
  BVector       m_vMapPosition;
  HeightMap::TTerrain m_terrainStyle;

  bool          m_bVerified;

  // Shape/Geometry related properties

  BVector  m_vOrigin;
  BVector  m_vStartLocation;
  BOBJData m_OBJData;
  BVector  m_vGoal;
  double   m_dAirTimeMaxSec;

  // Objects

  int     m_nObjects;
  BObject m_pObjects[200];

  // Gameplay

  BRaceRecord m_raceRecord;
  BRaceRecord m_raceRecordBestTime;
  BRaceRecord m_raceRecordSlalomTime;
  BSlalom     m_slalom;
  double      m_dBestAirTime;

  string          m_sSceneObjectNames[100];
  BUISelectionList m_sellistSceneObjects;

  BScene();
  ~BScene();
  void CleanUp();
  void Save();
  void LoadSceneFromFile(string sFilename);
  void PlaceTerrainObjects(BTerrainBlock *pBlockSingle = 0);
  void PlaceObjectsOnTerrain(BTerrainBlock *pBlockSingle = 0);
  void UpdateObjectList();
  void ValidateChecksum();

  void LoadTimeRecord(string sFileExt, BRaceRecord &raceRecord);
  void SaveTimeRecord(string sFileExt, BRaceRecord &raceRecord);

  void SaveBestTimeRecord();
  void SaveSlalomTimeRecord();
  void LoadBestTimeRecord();
  void LoadSlalomTimeRecord();

  void LoadSlalom();
  void SaveSlalom();
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
