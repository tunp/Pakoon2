//
// BTextRenderer: Misc text rendering routines
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#include "stdafx.h"
#include "BTextRenderer.h"
#include "BTextures.h"
#include "BSimulation.h"



//*************************************************************************************************
BTextRenderer::BTextRenderer() {
  m_dCharWidth = 20.0;
  m_dCharHeight = 40.0;
}


//*************************************************************************************************
double BTextRenderer::GetStringWidth(CString sTxt) {
  double dWidth = 0;
  for(int i = 0; i < sTxt.GetLength(); ++i) {
    char c = sTxt[i];
    c = toupper(c);
    c -= ' ';
    dWidth += m_letters[c].m_dXSize * 256.0;
  }
  return dWidth;
}


//*************************************************************************************************
void BTextRenderer::Init() {
  // Find out letter coordinates
  m_nBaseline[0] = 36;
  m_nBaseline[1] = 76;
  m_nBaseline[2] = 116;
  m_nBaseline[3] = 156;
  m_nBaseline[4] = 196;
  m_nBaseline[5] = 236;

  // search for characters
  int nLineHeight = 40;
  int nOffsetFromBaseline = 5;
  int nLine = 0;
  int nXPos = 0;

  // handle SPACE separately
  m_letters[0].m_dX = 235.0 / 256.0;
  m_letters[0].m_dY = (255.0 - 40.0) / 256.0;
  m_letters[0].m_dXSize = 10.0 / 256.0;
  m_letters[0].m_dYSize = 40.0 / 256.0;

  // handle all other printable characters
  int nRed, nGreen, nBlue, nAlpha;
  for(char c = '!'; c <= '`'; ++c) {
    char cUpper = toupper(c);
    if(cUpper == ':') {
      cUpper = ':';
    }
    int nCharIndex = cUpper - ' ';

    // Advance nXPos until we find a non-clear alpha pixel
    bool bAlphaFound = false;
    int nYStart = 256 - m_nBaseline[nLine] - nOffsetFromBaseline;
    int nYEnd   = 256 - m_nBaseline[nLine] - nOffsetFromBaseline + nLineHeight;
    while(!bAlphaFound) {
      for(int nYPos = nYStart; nYPos < nYEnd; ++nYPos) {
        BTextures::GetRGBFromTexture(BTextures::Texture::LETTERS,
                                     nXPos,
                                     nYPos,
                                     nRed, nGreen, nBlue, nAlpha);
        if(nAlpha > 0) {
          bAlphaFound = true;
          break;
        }
      }
      if(!bAlphaFound) {
        ++nXPos;
        if(nXPos > 255) {
          ++nLine;
          nYStart = 256 - m_nBaseline[nLine] - nOffsetFromBaseline;
          nYEnd   = 256 - m_nBaseline[nLine] - nOffsetFromBaseline + nLineHeight;
          nXPos = 0;
        }
      }
    }

    // we have now found the character start position. Search for the char (x) size
    m_letters[nCharIndex].m_dX = double(nXPos) / 256.0;
    m_letters[nCharIndex].m_dY = double(256 - m_nBaseline[nLine] - nOffsetFromBaseline) / 256.0;
    m_letters[nCharIndex].m_dYSize = double(nLineHeight) / 256.0;
  
    bAlphaFound = true;
    while(bAlphaFound) {
      bAlphaFound = false;
      for(int nYPos = nYStart; nYPos < nYEnd; ++nYPos) {
        BTextures::GetRGBFromTexture(BTextures::Texture::LETTERS,
                                     nXPos,
                                     nYPos,
                                     nRed, nGreen, nBlue, nAlpha);
        if(nAlpha > 0) {
          bAlphaFound = true;
          break;
        }
      }
      ++nXPos;
      if(nXPos > 255) {
        bAlphaFound = false;
        nXPos = 255;
      }
    }
    m_letters[nCharIndex].m_dXSize = double(nXPos) / 256.0 - m_letters[nCharIndex].m_dX;
  }

  // Manually adjust 'I's kerning
  m_letters['I' - ' '].m_dXSize += 2.0 / 256.0;
}


//*************************************************************************************************
void BTextRenderer::StartRenderingText() {
  OpenGLHelpers::SwitchToTexture(0);
  BTextures::Use(BTextures::Texture::LETTERS);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

//*************************************************************************************************
void BTextRenderer::StopRenderingText() {
  glDisable(GL_TEXTURE_2D);
}


//*************************************************************************************************
void BTextRenderer::DrawTextAt(double dX, 
                               double dY, 
                               CString sText, 
                               TTextAlign textalign,
                               double dRed, 
                               double dGreen, 
                               double dBlue, 
                               double dAlpha) {

  OpenGLHelpers::SetColorFull(dRed, dGreen, dBlue, dAlpha);

  glPushMatrix();
  glTranslated(dX, dY - m_dCharHeight / 2.0, 0);

  if(textalign == ALIGN_RIGHT) {
    glTranslated(-GetStringWidth(sText), 0, 0);
  } else if(textalign == ALIGN_CENTER) {
    glTranslated(-GetStringWidth(sText) / 2.0, 0, 0);
  }

  // Draw text char by char
  for(int i = 0; i < sText.GetLength(); ++i) {
    char c = sText[i];
    c = toupper(c);
    c -= ' ';

    glBegin(GL_QUADS);

    OpenGLHelpers::SetTexCoord(m_letters[c].m_dX, m_letters[c].m_dY);
    glVertex3f(0, 0, 0);
    OpenGLHelpers::SetTexCoord(m_letters[c].m_dX, m_letters[c].m_dY + m_letters[c].m_dYSize);
    glVertex3f(0, m_letters[c].m_dYSize * 256.0, 0);
    OpenGLHelpers::SetTexCoord(m_letters[c].m_dX + m_letters[c].m_dXSize, m_letters[c].m_dY + m_letters[c].m_dYSize);
    glVertex3f(m_letters[c].m_dXSize * 256.0, m_letters[c].m_dYSize * 256.0, 0);
    OpenGLHelpers::SetTexCoord(m_letters[c].m_dX + m_letters[c].m_dXSize, m_letters[c].m_dY);
    glVertex3f(m_letters[c].m_dXSize * 256.0, 0, 0);

    glEnd();

    glTranslated(m_letters[c].m_dXSize * 256.0, 0, 0);
  }

  glPopMatrix();
}


//*************************************************************************************************
void BTextRenderer::DrawSmallTextAt(double dX, 
                                    double dY, 
                                    CString sText, 
                                    int     nChars,
                                    TTextAlign textalign,
                                    double dRed, 
                                    double dGreen, 
                                    double dBlue, 
                                    double dAlpha) {

  double xSize = 8.25;
  double ySize = 13.9;

  OpenGLHelpers::SetColorFull(dRed, dGreen, dBlue, dAlpha);

  glPushMatrix();
  glTranslated(dX, dY, 0);
  glScalef(0.75, 0.5, 0.5);
  DrawTextAt(0, 0, sText, textalign, dRed, dGreen, dBlue, dAlpha);
  glPopMatrix();
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