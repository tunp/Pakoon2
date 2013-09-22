//
// BSceneModule
//

#pragma once

#include "BaseClasses.h"
#include "BObject.h"
#include "gl\gl.h"
#include "gl\glu.h"

class BSceneModule {
  void DestroyCurrentScene();
  bool LoadCommonSceneParams(CString sSceneName);
  bool LoadObjects(CString sSceneName);
  char *ReadVectors(FILE *fp, BVector *pVectors, int nMaxToRead);
public:
  CString  m_sSceneName;
  int      m_nObjects;
  BObject *m_pObject;
  BVector  m_vOrigin;

  BSceneModule() {m_sSceneName = ""; m_nObjects = 0; m_pObject = 0;}
  void LoadScene(CString sSceneName = "Startup");
};