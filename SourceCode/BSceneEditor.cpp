//
// BSceneEditor: Simple scene editor
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#pragma once

#include "stdafx.h"
#include "BSceneEditor.h"
#include "BGame.h"
#include "BSimulation.h"
#include "BCamera.h"
#include "BTextures.h"
#include "BScene.h"
#include "BObject.h"
#include "Pakoon1View.h"

//*****************************************************************************
BSceneEditor::BSceneEditor() {
  m_bSceneModified = false;
  BGame::m_bSceneEditorMode = false;
  m_phase = BASIC;
  m_nAllObjects = 0;
  m_sActiveObject = "";

  m_sObjectTypes[0] = "Other";
  m_sObjectShadows[0] = "no shadow";
  m_sObjectShadows[1] = "convex shadow";
  m_sOverwrite[0] = "overwrite existing scene file";
  m_sOverwrite[1] = "cancel save";
  m_sellistObjectType.SetItems(m_sObjectTypes, 1);
  m_sellistObjectType.SelectItem("Other");
  m_sellistObjectShadow.SetItems(m_sObjectShadows, 2);
  m_sellistObjectShadow.SelectItem("convex shadow");
  m_sellistOverwrite.SetItems(m_sOverwrite, 2);
  m_sellistOverwrite.SelectItem("cancel save");
}

//*****************************************************************************
bool BSceneEditor::IsActive() {
  return BGame::m_bSceneEditorMode;
}

//*****************************************************************************
void BSceneEditor::Activate() {
  m_bSceneModified = false;
  BGame::m_bSceneEditorMode = true;

  // Setup camera for editor mode
  BCamera *pCamera = BGame::GetSimulation()->GetCamera();
  pCamera->m_orientation.m_vForward.m_dZ = 0.0;
  pCamera->m_orientation.m_vForward.ToUnitLength();
  pCamera->m_orientation.m_vUp = BVector(0, 0, -1);
  pCamera->m_orientation.m_vRight = pCamera->m_orientation.m_vUp.CrossProduct(pCamera->m_orientation.m_vForward);
}


//*****************************************************************************
void BSceneEditor::Deactivate() {
  // Check for save!
  BGame::m_bSceneEditorMode = false;
}


//*****************************************************************************
void BSceneEditor::Draw(CRect &rectWnd) {

  // Always draw the "Scene Editor Mode" indicator
  OpenGLHelpers::SwitchToTexture(0);
  BTextures::Use(BTextures::Texture::PANEL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  double dAlpha = fabs(double(clock() % CLOCKS_PER_SEC) - (double(CLOCKS_PER_SEC) / 2.0)) / (double(CLOCKS_PER_SEC) / 2.0);
  OpenGLHelpers::SetColorFull(dAlpha / 2, dAlpha / 2, 1, 1);
  glPushMatrix();
  glTranslated(30, rectWnd.Height() - 70.0, 0);

  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(255.0 / 512.0, (512.0 - 32.0) / 512.0);   glVertex3f(0, 0, 0);
  OpenGLHelpers::SetTexCoord(255.0 / 512.0, (512.0 - 0.0) / 512.0);    glVertex3f(0, 33.0, 0);
  OpenGLHelpers::SetTexCoord(485.0 / 512.0, (512.0 - 32.0) / 512.0);   glVertex3f(231.0, 0, 0);
  OpenGLHelpers::SetTexCoord(485.0 / 512.0, (512.0 - 0.0) / 512.0);    glVertex3f(231.0, 33.0, 0);
  glEnd();

  glPopMatrix();

  // Phase-specific stuff
  switch(m_phase) {
    case SELECTING_SCENE_OBJECT:
      OpenGLHelpers::DrawVeil(0, 0, 0.25, 0.5, rectWnd);
      BGame::GetSimulation()->GetScene()->m_sellistSceneObjects.DrawAt(rectWnd.Width() / 2.0,
                                                                       rectWnd.Height() / 2.0,
                                                                       BTextRenderer::TTextAlign::ALIGN_CENTER);
      break;
    case SELECTING_SCENE_OBJECT_TO_DELETE:
      OpenGLHelpers::DrawVeil(0.25, 0, 0, 0.5, rectWnd);
      BGame::GetSimulation()->GetScene()->m_sellistSceneObjects.DrawAt(rectWnd.Width() / 2.0,
                                                                       rectWnd.Height() / 2.0,
                                                                       BTextRenderer::TTextAlign::ALIGN_CENTER);
      break;
    case ASKING_OBJECT_NAME:
    case ASKING_SCENE_DISPLAY_NAME:
    case ASKING_SCENE_FILENAME:
      OpenGLHelpers::DrawVeil(0, 0, 0.25, 0.5, rectWnd);
      m_edit.DrawAt(rectWnd.Width() / 2.0,
                    rectWnd.Height() / 2.0,
                    true);
      break;
    case SELECTING_OBJECT_TYPE:
      OpenGLHelpers::DrawVeil(0, 0, 0.25, 0.5, rectWnd);
      m_sellistObjectType.DrawAt(rectWnd.Width() / 2.0,
                                 rectWnd.Height() / 2.0,
                                 BTextRenderer::TTextAlign::ALIGN_CENTER);
      break;
    case SELECTING_OBJECT_SHADOW:
      OpenGLHelpers::DrawVeil(0, 0, 0.25, 0.5, rectWnd);
      m_sellistObjectShadow.DrawAt(rectWnd.Width() / 2.0,
                                   rectWnd.Height() / 2.0,
                                   BTextRenderer::TTextAlign::ALIGN_CENTER);
      break;
    case SELECTING_OBJECT_FILE:
      OpenGLHelpers::DrawVeil(0, 0, 0.25, 0.5, rectWnd);
      m_sellistAllObjects.DrawAt(rectWnd.Width() / 2.0,
                                 rectWnd.Height() / 2.0,
                                 BTextRenderer::TTextAlign::ALIGN_CENTER);
      break;
    case OVERWRITE_SCENE_FILE_QUESTION:
      OpenGLHelpers::DrawVeil(0, 0, 0.25, 0.5, rectWnd);
      m_sellistOverwrite.DrawAt(rectWnd.Width() / 2.0,
                                rectWnd.Height() / 2.0,
                                BTextRenderer::TTextAlign::ALIGN_CENTER);
      break;
    case BASIC:
      break;
    case MOVING_OBJECT:
      break;
  }

  // Draw active object name

  glPushMatrix();
  glTranslated(30, rectWnd.Height() - 70.0, 0);

  double dColor = 1.0;
  if((m_phase == BASIC) || (m_phase == MOVING_OBJECT)) {
    dColor = 0.0;
  }

  BUI::TextRenderer()->StartRenderingText();

  if(!m_sActiveObject.IsEmpty()) {
    CString sText;
    sText = "ACTIVE OBJECT:" + m_sActiveObject;
    BUI::TextRenderer()->DrawTextAt(0, -40, sText, BTextRenderer::TTextAlign::ALIGN_LEFT, dColor, dColor, dColor);
  }

  switch(m_phase) {
    case SELECTING_SCENE_OBJECT:
      BUI::TextRenderer()->DrawTextAt(0, -15, "Select scene object", BTextRenderer::TTextAlign::ALIGN_LEFT, dColor, dColor, dColor);
      break;
    case SELECTING_SCENE_OBJECT_TO_DELETE:
      BUI::TextRenderer()->DrawTextAt(0, -15, "Select object to be deleted", BTextRenderer::TTextAlign::ALIGN_LEFT, 1, 0, 0);
      break;
    case ASKING_OBJECT_NAME:
      BUI::TextRenderer()->DrawTextAt(0, -15, "Enter object name", BTextRenderer::TTextAlign::ALIGN_LEFT, dColor, dColor, dColor);
      break;
    case ASKING_SCENE_DISPLAY_NAME:
      BUI::TextRenderer()->DrawTextAt(0, -15, "Enter scene display name", BTextRenderer::TTextAlign::ALIGN_LEFT, dColor, dColor, dColor);
      break;
    case ASKING_SCENE_FILENAME:
      BUI::TextRenderer()->DrawTextAt(0, -15, "Enter scene filename", BTextRenderer::TTextAlign::ALIGN_LEFT, dColor, dColor, dColor);
      break;
    case SELECTING_OBJECT_TYPE:
      BUI::TextRenderer()->DrawTextAt(0, -15, "Select object type", BTextRenderer::TTextAlign::ALIGN_LEFT, dColor, dColor, dColor);
      break;
    case SELECTING_OBJECT_SHADOW:
      BUI::TextRenderer()->DrawTextAt(0, -15, "Select object shadow", BTextRenderer::TTextAlign::ALIGN_LEFT, dColor, dColor, dColor);
      break;
    case SELECTING_OBJECT_FILE:
      BUI::TextRenderer()->DrawTextAt(0, -15, "Select object file", BTextRenderer::TTextAlign::ALIGN_LEFT, dColor, dColor, dColor);
      break;
    case BASIC:
      BUI::TextRenderer()->DrawTextAt(0, -15, "Camera mode", BTextRenderer::TTextAlign::ALIGN_LEFT, dColor, dColor, dColor);
      break;
    case MOVING_OBJECT:
      BUI::TextRenderer()->DrawTextAt(0, -15, "Place object", BTextRenderer::TTextAlign::ALIGN_LEFT, dColor, dColor, dColor);
      break;
    case OVERWRITE_SCENE_FILE_QUESTION:
      BUI::TextRenderer()->DrawTextAt(0, -15, "Confirm file overwrite", BTextRenderer::TTextAlign::ALIGN_LEFT, dColor, dColor, dColor);
      break;
  }

  BUI::TextRenderer()->StopRenderingText();

  glPopMatrix();

}



//*****************************************************************************
void BSceneEditor::CancelPhase() {
  m_phase = BASIC;
}



//*****************************************************************************
void BSceneEditor::AdvancePhase() {
  BScene *pScene = BGame::GetSimulation()->GetScene();
  switch(BGame::GetSceneEditor()->m_phase) {
    case ASKING_SCENE_DISPLAY_NAME:
      m_phase = ASKING_SCENE_FILENAME;
      // Ask for scene file name
      {
        BUIEdit::TStatus statusEdit;
        pScene->m_sName = m_edit.GetValue(statusEdit);
        m_edit.Setup("Scene filename:", pScene->m_sFilename, 32);
      }
      break;
    case ASKING_SCENE_FILENAME:
      m_phase = BASIC;
      {
        BUIEdit::TStatus statusEdit;
        pScene->m_sFilename = m_edit.GetValue(statusEdit);

        if((pScene->m_sFilename.GetLength() < 1) || 
          (pScene->m_sFilename[0] != '.')) {
          pScene->m_sFilename = "./" + pScene->m_sFilename;
        }

        // Check for file overwrite
        FILE *fp;
        fp = fopen(pScene->m_sFilename, "r");
        if(fp) {
          m_phase = OVERWRITE_SCENE_FILE_QUESTION;
          BUI::StartUsingSelectionList(&(BGame::GetSceneEditor()->m_sellistOverwrite), &CPakoon1View::OnKeyDownSceneEditor);
        } else {
          // Save scene
          pScene->Save();
        }
      }
      break;
    case OVERWRITE_SCENE_FILE_QUESTION:
      m_phase = BASIC;
      {
        CString sTmp = "";
        m_sellistOverwrite.GetSelectedItem(sTmp);
        if(sTmp.CompareNoCase("overwrite existing scene file") == 0) {
          // Save scene
          pScene->Save();
        }
      }
      break;
    case ASKING_OBJECT_NAME:
      m_phase = SELECTING_OBJECT_TYPE;
      BUIEdit::TStatus statusEdit;
      m_sActiveObject = m_edit.GetValue(statusEdit);
      // Load all object files into the object file list
      FetchAllObjects();
      break;
    case SELECTING_OBJECT_TYPE:
      m_sellistObjectType.GetSelectedItem(m_sObjectType);
      m_phase = SELECTING_OBJECT_SHADOW;
      BUI::StartUsingSelectionList(&(BGame::GetSceneEditor()->m_sellistObjectShadow), &CPakoon1View::OnKeyDownSceneEditor);
      break;
    case SELECTING_OBJECT_SHADOW:
      m_sellistObjectShadow.GetSelectedItem(m_sObjectShadow);
      m_phase = SELECTING_OBJECT_FILE;
      BUI::StartUsingSelectionList(&(BGame::GetSceneEditor()->m_sellistAllObjects), &CPakoon1View::OnKeyDownSceneEditor);
      break;
    case SELECTING_OBJECT_FILE:
      // Create a new object and insert that to the scene

      {
        // ObjectData
        pScene->m_pObjects[pScene->m_nObjects].SetOBJData(&(pScene->m_OBJData));

        // Name
        pScene->m_pObjects[pScene->m_nObjects].m_sName = m_sActiveObject;

        // Type
        pScene->m_pObjects[pScene->m_nObjects].m_type = BObject::TType::USER_DEF;

        // Object file
        m_sellistAllObjects.GetSelectedItem(pScene->m_pObjects[pScene->m_nObjects].m_sObjectFilename);

        // Location
        BCamera *pCamera = BGame::GetSimulation()->GetCamera();
        pScene->m_pObjects[pScene->m_nObjects].m_vLocation = pCamera->m_vLocation + pCamera->m_orientation.m_vForward * 20.0;

        // Scale
        pScene->m_pObjects[pScene->m_nObjects].m_dScale = 1.0;

        // ZRotation
        pScene->m_pObjects[pScene->m_nObjects].m_dZRotation = 0.0;

        // Shadow
        pScene->m_pObjects[pScene->m_nObjects].m_bHasShadow = (m_sObjectShadow.CompareNoCase("no shadow") != 0);

        // Load shape from file
        pScene->m_pObjects[pScene->m_nObjects].LoadObjectFromFile("", "", true);

        // Prepare OBJ data again
        pScene->m_OBJData.PrepareWaveFrontModel();

        // Prepare object
        pScene->m_pObjects[pScene->m_nObjects].Setup();
        pScene->m_pObjects[pScene->m_nObjects].PreProcessVisualization();

        // Reposition the object so that we don't hit it's bounding sphere
        pScene->m_pObjects[pScene->m_nObjects].m_vLocation = pCamera->m_vLocation + 
                                                             pCamera->m_orientation.m_vForward * (5.0 + pScene->m_pObjects[pScene->m_nObjects].m_dRadius);

        // Recreate the shadow
        pScene->m_pObjects[pScene->m_nObjects].RecreateShadow();

        // Add object
        ++pScene->m_nObjects;

        // Update the selection list for scene objects
        pScene->UpdateObjectList();
      }

      m_phase = MOVING_OBJECT;
      break;
    case SELECTING_SCENE_OBJECT:
      // Set active object
      pScene->m_sellistSceneObjects.GetSelectedItem(m_sActiveObject);
      m_phase = MOVING_OBJECT;
      break;
    case SELECTING_SCENE_OBJECT_TO_DELETE:
      // Delete active object
      {
        CString sTmp = "";
        pScene->m_sellistSceneObjects.GetSelectedItem(sTmp);

        // Delete object
        for(int i = 0; i < pScene->m_nObjects; ++i) {
          if(pScene->m_pObjects[i].m_sName.CompareNoCase(sTmp) == 0) {
            pScene->m_pObjects[i] = pScene->m_pObjects[pScene->m_nObjects - 1];
            pScene->m_pObjects[pScene->m_nObjects - 1].m_pCollDetPart = 0;
            pScene->m_pObjects[pScene->m_nObjects - 1].m_pPart = 0;
            pScene->m_pObjects[pScene->m_nObjects - 1].m_boundary.m_pHead = 0; // This leaks memory but that's life.
            --(pScene->m_nObjects);
            pScene->UpdateObjectList();
            break;
          }
        }
      }
      m_phase = BASIC;
      break;
    case MOVING_OBJECT:      
      m_phase = BASIC;
      break;
  }      
}



//*****************************************************************************
void BSceneEditor::RecurseObjectFiles(CString sPath, CString sSubDir) {
  // Iterate all directories and search for *.object files
  CFileFind finder;

  // start working for files
  // build a string with wildcards
  CString strWildcard(sPath);
  strWildcard += _T("\\*.object");

  BOOL bWorking = finder.FindFile(strWildcard);

  while(bWorking) {
    bWorking = finder.FindNextFile();

    // skip . and .. files; otherwise, we'd
    // recur infinitely!
    if(finder.IsDots())
       continue;

    // Add filename to object list
    m_sAllObjects[m_nAllObjects] = sSubDir + finder.GetFileName();
    ++m_nAllObjects;
  }

  finder.Close();

  // Recurse subdirectories

  CString strWildcard2(sPath);
  strWildcard2 += _T("\\*.*");

  bWorking = finder.FindFile(strWildcard2);

  while(bWorking) {
    bWorking = finder.FindNextFile();

    // skip . and .. files; otherwise, we'd
    // recur infinitely!
    if(finder.IsDots())
       continue;

    // if it's a directory, recursively search it
    if(finder.IsDirectory()) {
       CString sDirPath = finder.GetFilePath();
       RecurseObjectFiles(sDirPath, sSubDir + finder.GetFileTitle() + "/");
    }
  }

  finder.Close();
}



//*****************************************************************************
void BSceneEditor::FetchAllObjects() {

  m_nAllObjects = 0;
  RecurseObjectFiles(".", "./");

  m_sellistAllObjects.SetItems(m_sAllObjects, m_nAllObjects);
  if(m_nAllObjects) {
    m_sellistAllObjects.SelectItem(m_sAllObjects[0]);
  }
}


//*****************************************************************************
BObject *BSceneEditor::GetActiveObject() {
  // Look for active object from scene
  BScene *pScene = BGame::GetSimulation()->GetScene();
  for(int i = 0; i < pScene->m_nObjects; ++i) {
    if(pScene->m_pObjects[i].m_sName.CompareNoCase(m_sActiveObject) == 0) {
      return &(pScene->m_pObjects[i]);
    }
  }
  return 0;
}



//*****************************************************************************
void BSceneEditor::HighlightActiveObject() {
  // Draw a glowing sphere around the active object.
  // Also, if the object is BASE or CLIENT, draw it's active radius.
  double dAlpha = fabs(double(clock() % CLOCKS_PER_SEC) - (double(CLOCKS_PER_SEC) / 2.0)) / (double(CLOCKS_PER_SEC) / 2.0);

  if(!m_sActiveObject.IsEmpty()) {
    // Find the active object from scene
    BScene *pScene = BGame::GetSimulation()->GetScene();
    for(int i = 0; i < pScene->m_nObjects; ++i) {
      if(pScene->m_pObjects[i].m_sName.CompareNoCase(m_sActiveObject) == 0) {
        // Object found, draw sphere
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        // Draw highlight sphere

        OpenGLHelpers::SetColorFull(0.1 * dAlpha, 0.1 * dAlpha, 0.1 * dAlpha, dAlpha);

        glPushMatrix();

        GLUquadricObj* pQuad = gluNewQuadric();
        glTranslated(pScene->m_pObjects[i].m_vCenter.m_dX,
                     pScene->m_pObjects[i].m_vCenter.m_dY,
                     pScene->m_pObjects[i].m_vCenter.m_dZ);
        gluSphere(pQuad, pScene->m_pObjects[i].m_dRadius, 50, 50);

        glPopMatrix();
        gluDeleteQuadric(pQuad);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_BLEND);
        
        break;
      }
    }
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