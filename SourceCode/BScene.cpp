//
// Scene
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#include "stdafx.h"
#include "BScene.h"
#include "BTextures.h"
#include "BGame.h"
#include "FileIOHelpers.h"
#include "HeightMap.h"

//*************************************************************************************************
BScene::BScene() {
  m_dFriction = 0.5;
  m_vOrigin.Set(0, 0, 0);
  m_vStartLocation.Set(0, 0, 0);
  m_vGoal.Set(0, 6000, 0);
  m_bSceneInUse = false;
  m_dGroundTextureScaler1 = 0.1;
  m_dGroundTextureScaler2 = 0.01;
  m_sSkyTexture = _T("");
  m_sGround1Texture = _T("");
  m_sGround2Texture = _T("");
  m_sEnvMapTexture = _T("");
  m_nObjects = 0;
  m_OBJData.Init();
  m_sName = "";
  m_sFilename = "";
  m_dBestAirTime = 0.0;
  m_dAirTimeMaxSec = 120.0;
  m_bVerified = false;
}


//*************************************************************************************************
BScene::~BScene() {
  CleanUp();
}


//*************************************************************************************************
void BScene::CleanUp() {
  m_nObjects = 0;

  BGame::GetSimulation()->GetTerrain()->StopUsingScene();
  BGame::GetSimulation()->GetVehicle()->Move(BGame::GetSimulation()->GetVehicle()->m_vLocation * -1.0);
  BGame::GetSimulation()->UpdateCarLocation();
  BGame::GetSimulation()->GetCamera()->m_vLocation.Set(0, 0, 0);
  m_slalom.m_nSlalomPoles = 0;
}

double g_dSceneFriction = 0.5;

extern double Random(double);

extern double g_dPhysicsStepsInSecond;

//*************************************************************************************************
void BScene::LoadSceneFromFile(CString sFilename) {

  m_sFilename = sFilename;

  if(m_bSceneInUse) {
    CleanUp();
  }
  // Load scene
  m_bSceneInUse = true;

  // General properties

  FileHelpers::GetKeyStringFromINIFile("Properties", "Name", "default", m_sName, sFilename);
  FileHelpers::GetKeyStringFromINIFile("Properties", "Image", "default", m_sImageFilename, sFilename);
  FileHelpers::GetKeyDoubleFromINIFile("Properties", "GroundFriction", 0.5, m_dFriction, sFilename);
  g_dSceneFriction = m_dFriction;
  FileHelpers::GetKeyDoubleFromINIFile("Properties", "Ground1Scaler", 0.1, m_dGroundTextureScaler1, sFilename);
  FileHelpers::GetKeyDoubleFromINIFile("Properties", "Ground2Scaler", 0.01, m_dGroundTextureScaler2, sFilename);
  FileHelpers::GetKeyStringFromINIFile("Properties", "SkyTexture", "", m_sSkyTexture, sFilename);
  FileHelpers::GetKeyStringFromINIFile("Properties", "Ground1Texture", "", m_sGround1Texture, sFilename);
  FileHelpers::GetKeyStringFromINIFile("Properties", "Ground2Texture", "", m_sGround2Texture, sFilename);
  FileHelpers::GetKeyStringFromINIFile("Properties", "EnvMapTexture", "", m_sEnvMapTexture, sFilename);
  FileHelpers::GetKeyDoubleFromINIFile("Properties", "BestAirTime", 0.0, m_dBestAirTime, sFilename);
  FileHelpers::GetKeyDoubleFromINIFile("Properties", "AirTimeMaxSec", 120.0, m_dAirTimeMaxSec, sFilename);
  m_dAirTimeMaxSec *= g_dPhysicsStepsInSecond;

  CString sTmp;

  FileHelpers::GetKeyStringFromINIFile("Properties", "TerrainStyle", "Basic", sTmp, sFilename);
  m_terrainStyle = HeightMap::BASIC;
  if(sTmp.CompareNoCase("Basic") == 0) {
    m_terrainStyle = HeightMap::BASIC;
  } else if(sTmp.CompareNoCase("GrandValley") == 0) {
    m_terrainStyle = HeightMap::GRAND_VALLEY;
  } else if(sTmp.CompareNoCase("JumpLand") == 0) {
    m_terrainStyle = HeightMap::JUMP_LAND;
  } else if(sTmp.CompareNoCase("ValleyAlley") == 0) {
    m_terrainStyle = HeightMap::VALLEY_ALLEY;
  } else if(sTmp.CompareNoCase("TheBigDrop") == 0) {
    m_terrainStyle = HeightMap::THE_BIG_DROP;
  } else if(sTmp.CompareNoCase("AlleyLand") == 0) {
    m_terrainStyle = HeightMap::ALLEY_LAND;
  } else if(sTmp.CompareNoCase("HolesInTheCrust") == 0) {
    m_terrainStyle = HeightMap::HOLES_IN_THE_CRUST;
  }
  
  FileHelpers::GetKeyVectorFromINIFile("Properties", "MapPosition", BVector(580, 85, 0), m_vMapPosition, sFilename);

  if(!m_sSkyTexture.IsEmpty()) {
    BTextures::ReloadTexture(BTextures::Texture::SKY, m_sSkyTexture);
  }
  if(!m_sGround1Texture.IsEmpty()) {
    BTextures::ReloadTexture(BTextures::Texture::GROUND_BASE, m_sGround1Texture);
  }
  if(!m_sGround2Texture.IsEmpty()) {
    BTextures::ReloadTexture(BTextures::Texture::GROUND_COLOR_MAP, m_sGround2Texture);
  }
  if(!m_sEnvMapTexture.IsEmpty()) {
    BTextures::ReloadTexture(BTextures::Texture::ENVMAP, m_sEnvMapTexture);
  }

  // Shape/Geometry related properties

  FileHelpers::GetKeyVectorFromINIFile("Geometry", "Origin", BVector(0, 0, 0), m_vOrigin, sFilename);
  FileHelpers::GetKeyVectorFromINIFile("Geometry", "StartLocation", BVector(0, 0, 0), m_vStartLocation, sFilename);
  FileHelpers::GetKeyVectorFromINIFile("Geometry", "Goal", BVector(0, 6000, 0), m_vGoal, sFilename);

  // Load objects
  // First count them

  m_nObjects = 0;
  do {
    CString sHasSection;
    CString sSection;
    sSection.Format("Object%d", m_nObjects + 1);
    FileHelpers::GetKeyStringFromINIFile(sSection, "", "default", sHasSection, sFilename);
    if(sHasSection.CompareNoCase("default") != 0) {
      ++m_nObjects;
    } else {
      break;
    }
  } while(m_nObjects < 100); // just a sanity check to break the loop eventually

  // Read objects
  int nObject = 0;
  for(nObject = 0; nObject < m_nObjects; ++nObject) {
    CString sSection;
    sSection.Format("Object%d", nObject + 1);
    m_pObjects[nObject].SetOBJData(&m_OBJData);
    m_pObjects[nObject].LoadObjectFromFile(sFilename, sSection);
  }

  // Prepare objects
  m_OBJData.PrepareWaveFrontModel();
  for(nObject = 0; nObject < m_nObjects; ++nObject) {
    m_pObjects[nObject].Setup();
    m_pObjects[nObject].PreProcessVisualization();
  }

  // Load best time records
  LoadBestTimeRecord();
  LoadSlalomTimeRecord();
  LoadSlalom();


  CString sChecksum;
  FileHelpers::GetKeyStringFromINIFile("Properties", "Checksum", "<no checksum>", sChecksum, sFilename);

  CString sVerifyData;
  sVerifyData.Format("%.5lf%d%.5lf%.5lf%.5lf%.5lf%.5lf%.5lf%.5lf%.5lf%.5lf%.5lf", 
                     m_dFriction, 
                     (int) m_terrainStyle,
                     m_vGoal.m_dX,
                     m_vGoal.m_dY,
                     m_vGoal.m_dZ,
                     m_vOrigin.m_dX,
                     m_vOrigin.m_dY,
                     m_vOrigin.m_dZ,
                     m_vStartLocation.m_dX,
                     m_vStartLocation.m_dY,
                     m_vStartLocation.m_dZ,
                     m_dBestAirTime);

  for(int i = 0; i < m_slalom.m_nSlalomPoles; ++i) {
    CString sTmp;
    sTmp.Format("%.5lf%.5lf%.5lf",
                m_slalom.m_slalomPole[i].m_vLocation.m_dX,
                m_slalom.m_slalomPole[i].m_vLocation.m_dY,
                m_slalom.m_slalomPole[i].m_vLocation.m_dZ);
    sVerifyData += sTmp;
  }

  m_bVerified = (BGame::GetVerifyChecksum(sVerifyData).Compare(sChecksum) == 0);
}

//*************************************************************************************************
void BScene::ValidateChecksum() {
  if(m_bVerified) {
    CString sVerifyData;
    sVerifyData.Format("%.5lf%d%.5lf%.5lf%.5lf%.5lf%.5lf%.5lf%.5lf%.5lf%.5lf%.5lf", 
                       m_dFriction, 
                       (int) m_terrainStyle,
                       m_vGoal.m_dX,
                       m_vGoal.m_dY,
                       m_vGoal.m_dZ,
                       m_vOrigin.m_dX,
                       m_vOrigin.m_dY,
                       m_vOrigin.m_dZ,
                       m_vStartLocation.m_dX,
                       m_vStartLocation.m_dY,
                       m_vStartLocation.m_dZ,
                       m_dBestAirTime);

    for(int i = 0; i < m_slalom.m_nSlalomPoles; ++i) {
      CString sTmp;
      sTmp.Format("%.5lf%.5lf%.5lf",
                  m_slalom.m_slalomPole[i].m_vLocation.m_dX,
                  m_slalom.m_slalomPole[i].m_vLocation.m_dY,
                  m_slalom.m_slalomPole[i].m_vLocation.m_dZ);
      sVerifyData += sTmp;
    }

    FileHelpers::WriteKeyStringToINIFile("Properties", "Checksum", BGame::GetVerifyChecksum(sVerifyData), m_sFilename);
  }
}


//*************************************************************************************************
void BScene::PlaceTerrainObjects(BTerrainBlock *pBlockSingle) {
  // Place objects on the terrain blocks
  PlaceObjectsOnTerrain(pBlockSingle);

  // Update object list
  UpdateObjectList();
}


//*************************************************************************************************
void BScene::UpdateObjectList() {
  // Update the selection list for scene objects
  for(int nObject = 0; nObject < m_nObjects; ++nObject) {
    m_sSceneObjectNames[nObject] = m_pObjects[nObject].m_sName;
  }
  m_sellistSceneObjects.SetItems(m_sSceneObjectNames, m_nObjects);
}


//*************************************************************************************************
void BScene::PlaceObjectsOnTerrain(BTerrainBlock *pBlockSingle) {
  for(int nObject = 0; nObject < m_nObjects; ++nObject) {
    // if object's bounding sphere "touches" terrain block,
    // add the object's pointer to the block.

    if(pBlockSingle) {
      // Place objects on single block (if not there already)
      BVector vCenter = pBlockSingle->GetCenter();
      if(pBlockSingle->PointIsInsideBlock(m_pObjects[nObject].m_vCenter, m_pObjects[nObject].m_dRadius)) {
        // Check to see if object is there already
        for(int i = 0; i < pBlockSingle->m_nObjects; ++i) {
          if(pBlockSingle->m_objectArray[i] == &m_pObjects[nObject]) {
            // The object is already in the block's object list --> go to next object
            continue;
          }
        }
        pBlockSingle->m_objectArray[pBlockSingle->m_nObjects] = &m_pObjects[nObject];
        ++pBlockSingle->m_nObjects;
      }
    } else {
      // Place objects on every visible block
      BTerrainBlock *pBlock = BGame::GetSimulation()->GetTerrain()->m_ringVisible.GetHead();
      do {      
        BVector vCenter = pBlock->GetCenter();
        if(pBlock->PointIsInsideBlock(m_pObjects[nObject].m_vCenter, m_pObjects[nObject].m_dRadius)) {
          pBlock->m_objectArray[pBlock->m_nObjects] = &m_pObjects[nObject];
          ++pBlock->m_nObjects;
        }

        pBlock = pBlock->m_pNext;
      } while (pBlock != BGame::GetSimulation()->GetTerrain()->m_ringVisible.GetHead());
    }
  }
}


//*************************************************************************************************
void BScene::Save() {

  // Create scene subdirectory, if one doesn't exist already

  // Save Scene file
  FILE *fp;
  fp = fopen(m_sFilename, "w");
  if(fp) {
    fprintf(fp, "; Scene file saved by Pakoon Scene Editor v1.ONE\n\n");

    fprintf(fp, "[Properties]\n");
    fprintf(fp, "\n");
    fprintf(fp, " Name = %s\n", m_sName);
    fprintf(fp, " Image = %s\n", m_sImageFilename);
    fprintf(fp, " GroundFriction = %g\n", m_dFriction);
    fprintf(fp, " Ground1Scaler = %g\n", m_dGroundTextureScaler1);
    fprintf(fp, " Ground2Scaler = %g\n", m_dGroundTextureScaler2);
    fprintf(fp, " SkyTexture = %s\n", m_sSkyTexture);
    fprintf(fp, " Ground1Texture = %s\n", m_sGround1Texture);
    fprintf(fp, " Ground2Texture = %s\n", m_sGround2Texture);
    fprintf(fp, " EnvMapTexture = %s\n", m_sEnvMapTexture);
    fprintf(fp, " BestAirTime = %.5lf\n", m_dBestAirTime);
    CString sTmp;
    if(m_terrainStyle == HeightMap::BASIC) {
      sTmp = "Basic";
    } else if(m_terrainStyle == HeightMap::GRAND_VALLEY) {
      sTmp = "GrandValley";
    } else if(m_terrainStyle == HeightMap::JUMP_LAND) {
      sTmp = "JumpLand";
    } else if(m_terrainStyle == HeightMap::THE_BIG_DROP) {
      sTmp = "TheBigDrop";
    } else if(m_terrainStyle == HeightMap::VALLEY_ALLEY) {
      sTmp = "ValleyAlley";
    } else if(m_terrainStyle == HeightMap::THE_BIG_DROP) {
      sTmp = "TheBigDrop";
    } else if(m_terrainStyle == HeightMap::ALLEY_LAND) {
      sTmp = "AlleyLand";
    } else if(m_terrainStyle == HeightMap::HOLES_IN_THE_CRUST) {
      sTmp = "HolesInTheCrust";
    }
    fprintf(fp, " TerrainStyle = %s\n", sTmp);
    fprintf(fp, " MapPosition = %.0lf, %.0lf, %.0lf\n", m_vMapPosition.m_dX, m_vMapPosition.m_dY, m_vMapPosition.m_dZ);

    fprintf(fp, "\n");

    fprintf(fp, "[Geometry]\n");
    fprintf(fp, "\n");
    fprintf(fp, " Origin = %lf, %lf, %lf\n", m_vOrigin.m_dX, m_vOrigin.m_dY, m_vOrigin.m_dZ);
    fprintf(fp, " StartLocation = %lf, %lf, %lf\n", m_vStartLocation.m_dX, m_vStartLocation.m_dY, m_vStartLocation.m_dZ);
    fprintf(fp, " Goal = %lf, %lf, %lf\n", m_vGoal.m_dX, m_vGoal.m_dY, m_vGoal.m_dZ);
    fprintf(fp, "\n");

    for(int i = 0; i < m_nObjects; ++i) {
      sTmp.Format("[Object%d]\n", i + 1);
      fprintf(fp, sTmp);
      fprintf(fp, "\n");
      fprintf(fp, " Name = %s\n", m_pObjects[i].m_sName);
      fprintf(fp, " Type = Other\n");
      fprintf(fp, " ObjectFile = %s\n", m_pObjects[i].m_sObjectFilename);
      fprintf(fp, " Location = %g, %g, %g\n", m_pObjects[i].m_vLocation.m_dX, m_pObjects[i].m_vLocation.m_dY, m_pObjects[i].m_vLocation.m_dZ);
      fprintf(fp, " ZRotation = %g\n", m_pObjects[i].m_dZRotation);
      fprintf(fp, " Scale = %g\n", m_pObjects[i].m_dScale2);
      fprintf(fp, " Shadow = %s\n", (m_pObjects[i].m_bHasShadow ? "True" : "False"));
      fprintf(fp, " ActiveRadius = %g\n", m_pObjects[i].m_dActiveRadius);
      fprintf(fp, "\n");
    }
    fclose(fp);
  }
}

void BScene::LoadTimeRecord(CString sFileExt, BRaceRecord &raceRecord) {
  CString sFilename;
  sFilename.Format(".\\Player\\%s%s", m_sName, sFileExt);
  FILE *fp;
  fp = fopen(LPCTSTR(sFilename), "r");
  if(fp) {
    raceRecord.m_bValid = true;
    fscanf(fp, "%lf", &(raceRecord.m_dTotalTime));
    fscanf(fp, "%lf", &(raceRecord.m_dCarHeight));
    fscanf(fp, "%lf", &(raceRecord.m_dCarLength));
    fscanf(fp, "%lf", &(raceRecord.m_dCarWidth));
    fscanf(fp, "%d", &(raceRecord.m_nNextSlot));
    fscanf(fp, "%d", &(raceRecord.m_nMaxSlot));
    for(int i = 0; i < raceRecord.m_nNextSlot; ++i) {
      fscanf(fp, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", 
              &(raceRecord.m_frames[i].m_dTime),
              &(raceRecord.m_frames[i].m_vLocation.m_dX),
              &(raceRecord.m_frames[i].m_vLocation.m_dY),
              &(raceRecord.m_frames[i].m_vLocation.m_dZ),
              &(raceRecord.m_frames[i].m_vForward.m_dX),
              &(raceRecord.m_frames[i].m_vForward.m_dY),
              &(raceRecord.m_frames[i].m_vForward.m_dZ),
              &(raceRecord.m_frames[i].m_vRight.m_dX),
              &(raceRecord.m_frames[i].m_vRight.m_dY),
              &(raceRecord.m_frames[i].m_vRight.m_dZ));
    }
    fclose(fp);
  } else {
    raceRecord.m_bValid = false;
  }
}

void BScene::SaveTimeRecord(CString sFileExt, BRaceRecord &raceRecord) {
  CString sFilename;
  sFilename.Format(".\\Player\\%s%s", m_sName, sFileExt);
  FILE *fp;
  fp = fopen(LPCTSTR(sFilename), "w");
  if(fp) {
    fprintf(fp, "%g\n", raceRecord.m_dTotalTime);
    fprintf(fp, "%g\n", raceRecord.m_dCarHeight);
    fprintf(fp, "%g\n", raceRecord.m_dCarLength);
    fprintf(fp, "%g\n", raceRecord.m_dCarWidth);
    fprintf(fp, "%d\n", raceRecord.m_nNextSlot);
    fprintf(fp, "%d\n", raceRecord.m_nMaxSlot);
    for(int i = 0; i < raceRecord.m_nNextSlot; ++i) {
      fprintf(fp, "%g %g %g %g %g %g %g %g %g %g\n", 
              raceRecord.m_frames[i].m_dTime,
              raceRecord.m_frames[i].m_vLocation.m_dX,
              raceRecord.m_frames[i].m_vLocation.m_dY,
              raceRecord.m_frames[i].m_vLocation.m_dZ,
              raceRecord.m_frames[i].m_vForward.m_dX,
              raceRecord.m_frames[i].m_vForward.m_dY,
              raceRecord.m_frames[i].m_vForward.m_dZ,
              raceRecord.m_frames[i].m_vRight.m_dX,
              raceRecord.m_frames[i].m_vRight.m_dY,
              raceRecord.m_frames[i].m_vRight.m_dZ);
    }

    fclose(fp);
  }
}

void BScene::LoadBestTimeRecord() {
  LoadTimeRecord(_T("BestTime.dat"), m_raceRecordBestTime);
}

void BScene::LoadSlalomTimeRecord() {
  LoadTimeRecord(_T("SlalomTime.dat"), m_raceRecordSlalomTime);
}

void BScene::SaveBestTimeRecord() {
  SaveTimeRecord(_T("BestTime.dat"), m_raceRecordBestTime);
}

void BScene::SaveSlalomTimeRecord() {
  SaveTimeRecord(_T("SlalomTime.dat"), m_raceRecordSlalomTime);
}


void BScene::LoadSlalom() {
  CString sFilename;
  sFilename.Format(".\\Player\\%sSlalom.dat", m_sName);
  FILE *fp;
  fp = fopen(LPCTSTR(sFilename), "r");
  m_slalom.m_bValid = false;
  m_slalom.m_nSlalomPoles = 0;
  m_slalom.m_nCurrentPole = 0;

  if(fp) {
    int nTmp;
    m_slalom.m_bValid = true;
    fscanf(fp, "%d\n", &(m_slalom.m_nSlalomPoles));
    for(int i = 0; i < m_slalom.m_nSlalomPoles; ++i) {
      fscanf(fp, "%lf %lf %lf %d\n", 
              &(m_slalom.m_slalomPole[i].m_vLocation.m_dX),
              &(m_slalom.m_slalomPole[i].m_vLocation.m_dY),
              &(m_slalom.m_slalomPole[i].m_vLocation.m_dZ),
              &nTmp);
      m_slalom.m_slalomPole[i].m_bPassFromRight = (nTmp == 1);
    }

    fclose(fp);
  }
}


void BScene::SaveSlalom() {
  CString sFilename;
  sFilename.Format(".\\Player\\%sSlalom.dat", m_sName);
  FILE *fp;
  fp = fopen(LPCTSTR(sFilename), "w");
  if(fp) {
    fprintf(fp, "%d\n", m_slalom.m_nSlalomPoles);
    for(int i = 0; i < m_slalom.m_nSlalomPoles; ++i) {
      fprintf(fp, "%g %g %g %d\n", 
              m_slalom.m_slalomPole[i].m_vLocation.m_dX,
              m_slalom.m_slalomPole[i].m_vLocation.m_dY,
              m_slalom.m_slalomPole[i].m_vLocation.m_dZ,
              m_slalom.m_slalomPole[i].m_bPassFromRight ? 1 : 0);
    }

    fclose(fp);
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