// TxtrTools.h: interface for the TxtrTools class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TXTRTOOLS_H__C2B422AB_1DDF_4423_A1F2_6A418FB56FB6__INCLUDED_)
#define AFX_TXTRTOOLS_H__C2B422AB_1DDF_4423_A1F2_6A418FB56FB6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "OpenGLHelpers.h"

namespace TxtrTools {
  bool LoadTextureFromRAW(CString &sFilename, int nXSize, int nYSize, GLubyte *pBuffer);
  GLubyte PixelAt(int nX, int nY, int nSize);
};

#endif // !defined(AFX_TXTRTOOLS_H__C2B422AB_1DDF_4423_A1F2_6A418FB56FB6__INCLUDED_)
