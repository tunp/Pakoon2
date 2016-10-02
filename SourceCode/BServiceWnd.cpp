// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#include "BServiceWnd.h"
#include "OpenGLHelpers.h"
#include "OpenGLExtFunctions.h"
#include "BTextures.h"
#include "BGame.h"

#include <SDL2/SDL.h>

// Just use global variables for inter-thread communication. 
//*****************************************************************************
BServiceWnd::BServiceWnd() {
  m_bPrompting = false;
  m_nCursor = 0;
  for(int i = 0; i < 41 * 17; ++i) {
    m_sText[i] = ' ';
    m_cColors[i * 3 + 0] = (unsigned char) 255;
    m_cColors[i * 3 + 1] = (unsigned char) 255;
    m_cColors[i * 3 + 2] = (unsigned char) 255;
    m_cBright[i] = 0;
  }
  Output("SERVICE COMMAND INTERPRETER V1.2");
  Prompt();
}

//*****************************************************************************
BServiceWnd::~BServiceWnd() {
}

//*****************************************************************************
void BServiceWnd::AddChar(char c, double dR, double dG, double dB) {

  if(c == SDLK_BACKSPACE) {
    // Eat one character
    if((m_nCursor > 0) && (m_nCursor > m_nCommandStart)) {
      m_nCursor -= 1;
      m_sText[m_nCursor] = ' ';
    }
    return;
  }

  if(c == SDLK_RETURN) {
    // Issue command
    string sCommand;
    if(m_nCommandStart < 0) {
      m_nCommandStart = 0;
    }
	string valText = m_sText;
    sCommand = valText.substr(m_nCommandStart, m_nCursor - m_nCommandStart);
    Newline();
    m_bPrompting = false;
    BGame::Command()->Run(sCommand);

    // prompt for next
    Prompt();
    return;
  }

	char str[] = {c, '\0'};
  AddString(str, dR, dG, dB);
}


//*****************************************************************************
void BServiceWnd::AddString(string sText, double dR, double dG, double dB) {
  // Make room for string
  while((m_nCursor + sText.length()) >= 41 * 17) {
    memmove(m_sText, m_sText + 41, 41 * 17 - 41);
    memset(m_sText + 41 * 17 - 41, ' ', 41);
    memmove(m_cColors, m_cColors + 41 * 3, (41 * 17 - 41) * 3);
    memmove(m_cBright, m_cBright + 41, 41 * 17 - 41);
    m_nCursor -= 41;
    m_nCommandStart -= 41;
  }
  // Insert string
  for(int i = 0; i < sText.length(); ++i) {
    m_sText[m_nCursor + i] = sText.at(i);
    m_cColors[(m_nCursor + i) * 3 + 0] = (unsigned char) dR * 255.0;
    m_cColors[(m_nCursor + i) * 3 + 1] = (unsigned char) dG * 255.0;
    m_cColors[(m_nCursor + i) * 3 + 2] = (unsigned char) dB * 255.0;
    m_cBright[m_nCursor + i] = 20;
  }
  m_nCursor += sText.length();
}

//*****************************************************************************
void BServiceWnd::Newline(bool bForce) {
  int nToGo = 41 - m_nCursor % 41;
  if((nToGo != 41) || bForce) {
    m_nCursor += nToGo;
    while(m_nCursor >= 41 * 17) {
      memmove(m_sText, m_sText + 41, 41 * 17 - 41);
      memset(m_sText + 41 * 17 - 41, ' ', 41);
      memmove(m_cColors, m_cColors + 41 * 3, (41 * 17 - 41) * 3);
      memmove(m_cBright, m_cBright + 41, 41 * 17 - 41);
      m_nCursor -= 41;
      m_nCommandStart -= 41;
    }
  }
}

//*****************************************************************************
void BServiceWnd::Prompt() {
  Newline();
  AddChar('>', 0.75, 0.75, 0.75);

  m_nCommandStart = m_nCursor;
  m_bPrompting = true;
}

//*****************************************************************************
void BServiceWnd::Output(string sText, double dR, double dG, double dB) {
  if(!m_bPrompting) {
    AddString(sText, dR, dG, dB);
  }
}



//*****************************************************************************
void BServiceWnd::DrawTexts() {
  double xSize = 8.25;
  double ySize = 13.9;
  glPushMatrix();
  for(int y = 0; y < 17; ++y) {
    glPushMatrix();

    for(int x = 0; x < 41; ++x) {
      char c = m_sText[y * 41 + x];
      c -= ' ';
      if(m_cBright[y * 41 + x] > 0) {
        OpenGLHelpers::SetColorFullSafe(double(m_cColors[y * 41 * 3 + x * 3 + 0]) / 255.0 + double(m_cBright[y * 41 + x]) / 40.0, 
                                        double(m_cColors[y * 41 * 3 + x * 3 + 1]) / 255.0 + double(m_cBright[y * 41 + x]) / 40.0,
                                        double(m_cColors[y * 41 * 3 + x * 3 + 2]) / 255.0 + double(m_cBright[y * 41 + x]) / 40.0,
                                        1);
        --m_cBright[y * 41 + x];
      } else {
        OpenGLHelpers::SetColorFull(double(m_cColors[y * 41 * 3 + x * 3 + 0]) / 255.0, 
                                    double(m_cColors[y * 41 * 3 + x * 3 + 1]) / 255.0,
                                    double(m_cColors[y * 41 * 3 + x * 3 + 2]) / 255.0,
                                    1);
      }

      glBegin(GL_TRIANGLE_STRIP);
      int xOff = c % 10;
      int yOff = c / 10;
      OpenGLHelpers::SetTexCoord((401.0 + double(xOff) * xSize)/512.0, (511.0 - (284.0 + double(yOff) * ySize)) / 512.0);
      glVertex3f(0, ySize, 0);
      OpenGLHelpers::SetTexCoord((401.0 + double(xOff) * xSize)/512.0, (511.0 - (284.0 + double(yOff) * ySize + ySize - 0.8)) / 512.0);
      glVertex3f(0, 0, 0);
      OpenGLHelpers::SetTexCoord((401.0 + double(xOff) * xSize + xSize)/512.0, (511.0 - (284.0 + double(yOff) * ySize)) / 512.0);
      glVertex3f(xSize, ySize, 0);
      OpenGLHelpers::SetTexCoord((401.0 + double(xOff) * xSize + xSize)/512.0, (511.0 - (284.0 + double(yOff) * ySize + ySize - 0.8)) / 512.0);
      glVertex3f(xSize, 0, 0);
      glEnd();
      glTranslated(xSize, 0, 0);
    }
    glPopMatrix();
    glTranslated(0, -ySize, 0);
  }
  glPopMatrix();

  // Draw blinking cursor at correct location
  glDisable(GL_TEXTURE_2D);
  glPushMatrix();
  int nCurX = m_nCursor % 41;
  int nCurY = m_nCursor / 41;
  glTranslated(xSize * nCurX, -ySize * nCurY, 0);
  unsigned clockNow = SDL_GetTicks();
  OpenGLHelpers::SetColorFull(0, 
                              1,
                              0,
                              fabs(double(clockNow % 1000) - 500.0) / 500.0);
  glBegin(GL_TRIANGLE_STRIP);
  glVertex3f(0, ySize, 0);
  glVertex3f(0, 0, 0);
  glVertex3f(xSize, ySize, 0);
  glVertex3f(xSize, 0, 0);
  glEnd();
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
