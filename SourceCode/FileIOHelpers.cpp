//
// File IO helpers
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#include "stdafx.h"
#include "FileIOHelpers.h"


int FileHelpers::WriteKeyStringToINIFile(CString  sSection,        // section name
                                         CString  sKeyName,        // key name
                                         CString  sValue,          // value to write
                                         CString  sFilename) {     // initialization file name

  return WritePrivateProfileString(LPCTSTR(sSection),  // section name
                                   LPCTSTR(sKeyName),  // key name
                                   LPCTSTR(sValue),    // string to add
                                   LPCTSTR(sFilename));// initialization file
}



int FileHelpers::GetKeyStringFromINIFile(CString  sSection,        // section name
                                         CString  sKeyName,        // key name
                                         CString  sDefault,        // default string
                                         CString &sReturnedString, // destination buffer
                                         CString  sFilename) {     // initialization file name
  // Wrapper for Windows INIT read function
  static char lpReturnedString[1024];
  DWORD nRet = 0;
  if(sKeyName.IsEmpty()) {
    DWORD nRet = GetPrivateProfileString(LPCTSTR(sSection),
                                         NULL,
                                         LPCTSTR(sDefault),
                                         lpReturnedString,
                                         1024,
                                         LPCTSTR(sFilename));
  } else {
    DWORD nRet = GetPrivateProfileString(LPCTSTR(sSection),
                                         LPCTSTR(sKeyName),
                                         LPCTSTR(sDefault),
                                         lpReturnedString,
                                         1024,
                                         LPCTSTR(sFilename));
  }
  sReturnedString = lpReturnedString;
  return int(nRet);
}


int FileHelpers::GetKeyDoubleFromINIFile(CString  sSection,        // section name
                                         CString  sKeyName,        // key name
                                         double   dDefault,        // default
                                         double  &dReturnedDouble, // destination buffer
                                         CString  sFilename) {     // initialization file name
  CString sValue, sDefault;
  sDefault = _T("default");
  int nRet = GetKeyStringFromINIFile(sSection, sKeyName, sDefault, sValue, sFilename);
  if(sValue.Compare(sDefault) == 0) {
    dReturnedDouble = dDefault;
  } else {
    sscanf(LPCTSTR(sValue), "%lf", &dReturnedDouble);
  }
  return nRet;
}

int FileHelpers::GetKeyIntFromINIFile(CString  sSection,        // section name
                                      CString  sKeyName,        // key name
                                      int      nDefault,        // default
                                      int     &nReturnedInt,    // destination buffer
                                      CString  sFilename) {     // initialization file name
  double dValue;
  int nRet = GetKeyDoubleFromINIFile(sSection, sKeyName, double(nDefault), dValue, sFilename);
  nReturnedInt = int(dValue);
  return nRet;
} 


int FileHelpers::GetKeyVectorFromINIFile(CString  sSection,        // section name
                                         CString  sKeyName,        // key name
                                         BVector  vDefault,        // default
                                         BVector &vReturnedVector, // destination buffer
                                         CString  sFilename) {     // initialization file name
  CString sValue, sDefault;
  sDefault = _T("default");
  int nRet = GetKeyStringFromINIFile(sSection, sKeyName, sDefault, sValue, sFilename);
  if(sValue.Compare(sDefault) == 0) {
    vReturnedVector = vDefault;
  } else {
    sscanf(LPCTSTR(sValue), 
           "%lf, %lf, %lf", 
           &(vReturnedVector.m_dX), 
           &(vReturnedVector.m_dY), 
           &(vReturnedVector.m_dZ));
  }
  return nRet;
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