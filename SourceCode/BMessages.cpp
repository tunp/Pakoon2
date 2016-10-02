//
// BMessages: Show on-screen messages
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#include "BMessages.h"
#include "OpenGLHelpers.h"
#include "StringTools.h"


BLetter   BMessages::m_letters[65];
BMessage *BMessages::m_pMessages = 0;

//*****************************************************************************
BMessages::BMessages() {
  // Load letters from the file
  FILE *fp;
  fp = fopen("./Textures/MessageLetters.txt", "r");
  if(fp) {
    for(char c = 32; c < 97; ++c) {
      int nPart = 0;
      int i = c - 32;
      char cLine[256];
      nPart = 0;
      for(int nLetterLine = 0; nLetterLine < 5; ++nLetterLine) {
        cLine[0] = ' ';
        cLine[1] = ' ';
        cLine[2] = ' ';
        fgets(cLine, 255, fp);
        m_letters[i].m_cParts[nPart+0] = cLine[0];
        m_letters[i].m_cParts[nPart+1] = cLine[1];
        m_letters[i].m_cParts[nPart+2] = cLine[2];
        nPart += 3;
      }
    }
    fclose(fp);
  } else {
    BGame::MyAfxMessageBox("Cannot open ./Textures/MessageLetters.txt!");
  }
}

//*****************************************************************************
BMessages::~BMessages() {
  RemoveAll();
}


//*****************************************************************************
void BMessages::Show(int nCharsPerLine, 
                     string sId, 
                     string sText, 
                     int nSeconds, 
                     bool blink, 
                     double dR, 
                     double dG, 
                     double dB,
                     bool bFaint,
                     bool bSecondary) {
  StringTools::makeUpper(sId);
  StringTools::makeUpper(sText);
  // Add message to message list

  if(!Find(sId)) {
    BMessage *pNew = new BMessage;
    pNew->m_bSecondary = bSecondary;
    if(!m_pMessages) {
      m_pMessages = pNew;
    } else {
      BMessage *p = m_pMessages;
      while(p->m_pNext) {
        p = p->m_pNext;
      }
      p->m_pNext = pNew;
    }
    time_t timeNow;
    pNew->m_pNext = 0;
    pNew->m_nCharsPerLine = nCharsPerLine;
    pNew->m_clockStarted = SDL_GetTicks();
    pNew->m_timeRemove = time(&timeNow) + nSeconds;
    pNew->m_sText = sText;
    pNew->m_sId = sId;
    pNew->m_blink = blink;
    pNew->m_dR = dR;
    pNew->m_dG = dG;
    pNew->m_dB = dB;
    pNew->m_bFaint = bFaint;

    pNew->m_nCharPartWidth = int(double(BGame::m_nDispWidth) / double(nCharsPerLine)) / 3;
    if(pNew->m_nCharPartWidth < 2) {
      pNew->m_nCharPartWidth = 2;
    }
    pNew->m_nCharPartGap = 1; // pNew->m_nCharPartWidth / 4;
    if(pNew->m_nCharPartGap < 1) {
      pNew->m_nCharPartGap = 1;
    }
    pNew->m_nCharWidth   = pNew->m_nCharPartWidth * 3 + pNew->m_nCharPartGap * 2;
    pNew->m_nCharHeight  = pNew->m_nCharPartWidth * 5 + pNew->m_nCharPartGap * 4;
    pNew->m_nCharGap     = pNew->m_nCharPartGap * 4;
    pNew->m_nTotalWidth  = sText.length() * pNew->m_nCharWidth + abs((int)sText.length() - 1) * pNew->m_nCharGap;
  }
}

//*****************************************************************************
bool BMessages::Find(string sId) {
  StringTools::makeUpper(sId);
  BMessage *p = m_pMessages;
  while(p) {
    if(p->m_sId == sId) {
      return true;
    }
    p = p->m_pNext;
  }
  return false;
}


//*****************************************************************************
void BMessages::Remove(string sId) {
  StringTools::makeUpper(sId);
  if(m_pMessages) {
    BMessage *p = m_pMessages;
    if(p->m_sId == sId) {
      m_pMessages = p->m_pNext;
      delete p;
    } else {
      while(p->m_pNext && 
            (p->m_pNext->m_sId != sId)) {
        p = p->m_pNext;
      }
      if(p->m_pNext && 
        (p->m_pNext->m_sId == sId)) {
        BMessage *p2 = p->m_pNext;
        p->m_pNext = p2->m_pNext;
        delete p2;
      }
    }
  }
}

//*****************************************************************************
void BMessages::RemoveAll() {
  BMessage *p = m_pMessages;
  while(p) {
    BMessage *p2 = p;
    p = p->m_pNext;
    delete p2;
  }
  m_pMessages = 0;
}

//*****************************************************************************
void BMessages::Render() {
  // Display all messages. Remove old ones
  glDisable(GL_TEXTURE_2D);
  BMessage *p = m_pMessages;
  while(p) {
    p = RenderMessage(p);
  }
}

//*****************************************************************************
BMessage *BMessages::RenderMessage(BMessage *pMsg) {
  BMessage *pRet = pMsg->m_pNext;
  time_t timeNow;
  time(&timeNow);
  if(timeNow > pMsg->m_timeRemove) {
    // Remove this message
    Remove(pMsg->m_sId);
  } else {
    // Draw message
    if(!pMsg->m_blink || 
       (pMsg->m_blink && 
        (((SDL_GetTicks() - pMsg->m_clockStarted) % 1000) < 750))) {
      OpenGLHelpers::SetColorFull(0, 0, 0, 1);
      glPushMatrix();
      glTranslated(-double(pMsg->m_nTotalWidth) / 2.0, double(pMsg->m_nCharHeight) / 2.0, 0);
      if(pMsg->m_bSecondary) {
        glTranslated(0, -double(pMsg->m_nCharHeight * 2.0), 0);
      }
      int i = 0;
      while(i < pMsg->m_sText.length()) {
        RenderLetter(pMsg, pMsg->m_sText.at(i), false);
        glTranslated(pMsg->m_nCharWidth + pMsg->m_nCharGap, 0, 0);
        ++i;
      }
      glPopMatrix();
      OpenGLHelpers::SetColorFull(pMsg->m_dR, pMsg->m_dG, pMsg->m_dB, 1);
      glPushMatrix();
      glTranslated(-double(pMsg->m_nTotalWidth) / 2.0, double(pMsg->m_nCharHeight) / 2.0, 0);
      if(pMsg->m_bSecondary) {
        glTranslated(0, -double(pMsg->m_nCharHeight * 2.0), 0);
      }
      i = 0;
      while(i < pMsg->m_sText.length()) {
        RenderLetter(pMsg, pMsg->m_sText.at(i), true);
        glTranslated(pMsg->m_nCharWidth + pMsg->m_nCharGap, 0, 0);
        ++i;
      }
      glPopMatrix();
    }
  }
  return pRet;
}

//*****************************************************************************
void BMessages::RenderLetter(BMessage *pMsg, char c, bool bInside) {
  // Draw letter parts

  if((c >= 32) && (c <= 96)) {
    c -= 32;
    for(int y = 0; y < 5; ++y) {
      for(int x = 0; x < 3; ++x) {
        switch(m_letters[c].m_cParts[y * 3 + x]) {
          case 'O':
            glBegin(GL_POLYGON);
            if(bInside) {
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap,
                         0);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap + pMsg->m_nCharPartWidth,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap,
                         0);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap + pMsg->m_nCharPartWidth,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap - pMsg->m_nCharPartWidth,
                         0);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap - pMsg->m_nCharPartWidth,
                         0);
            } else {
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap - pMsg->m_nCharPartGap * 4,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap + pMsg->m_nCharPartGap * 4,
                         0);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap + pMsg->m_nCharPartWidth + pMsg->m_nCharPartGap * 4,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap + pMsg->m_nCharPartGap * 4,
                         0);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap + pMsg->m_nCharPartWidth + pMsg->m_nCharPartGap * 4,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap - pMsg->m_nCharPartWidth - pMsg->m_nCharPartGap * 4,
                         0);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap - pMsg->m_nCharPartGap * 4,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap - pMsg->m_nCharPartWidth - pMsg->m_nCharPartGap * 4,
                         0);
            }
            glEnd();
            break;
          case 'J':
            if(bInside) {
              glBegin(GL_TRIANGLES);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap + pMsg->m_nCharPartWidth,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap,
                         0);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap + pMsg->m_nCharPartWidth,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap - pMsg->m_nCharPartWidth,
                         0);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap - pMsg->m_nCharPartWidth,
                         0);
              glEnd();
            } else {
              glBegin(GL_POLYGON);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap + pMsg->m_nCharPartWidth + pMsg->m_nCharPartGap * 4,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap + pMsg->m_nCharPartGap * 4,
                         0);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap + pMsg->m_nCharPartWidth + pMsg->m_nCharPartGap * 4,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap - pMsg->m_nCharPartWidth - pMsg->m_nCharPartGap * 4,
                         0);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap - pMsg->m_nCharPartGap * 4,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap - pMsg->m_nCharPartWidth - pMsg->m_nCharPartGap * 4,
                         0);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap - pMsg->m_nCharPartGap * 4,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap - pMsg->m_nCharPartWidth + pMsg->m_nCharPartGap * 4,
                         0);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap + pMsg->m_nCharPartWidth - pMsg->m_nCharPartGap * 4,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap + pMsg->m_nCharPartGap * 4,
                         0);
              glEnd();
            }
            break;
          case 'L':
            if(bInside) {
              glBegin(GL_TRIANGLES);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap,
                         0);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap + pMsg->m_nCharPartWidth,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap - pMsg->m_nCharPartWidth,
                         0);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap - pMsg->m_nCharPartWidth,
                         0);
              glEnd();
            } else {
              glBegin(GL_POLYGON);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap - pMsg->m_nCharPartGap * 4,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap + pMsg->m_nCharPartGap * 4,
                         0);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap + pMsg->m_nCharPartGap * 4,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap + pMsg->m_nCharPartGap * 4,
                         0);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap + pMsg->m_nCharPartWidth + pMsg->m_nCharPartGap * 4,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap - pMsg->m_nCharPartWidth + pMsg->m_nCharPartGap * 4,
                         0);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap + pMsg->m_nCharPartWidth + pMsg->m_nCharPartGap * 4,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap - pMsg->m_nCharPartWidth - pMsg->m_nCharPartGap * 4,
                         0);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap - pMsg->m_nCharPartGap * 4,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap - pMsg->m_nCharPartWidth - pMsg->m_nCharPartGap * 4,
                         0);
              glEnd();
            }
            break;
          case '7':
            if(bInside) {
              glBegin(GL_TRIANGLES);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap,
                         0);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap + pMsg->m_nCharPartWidth,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap,
                         0);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap + pMsg->m_nCharPartWidth,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap - pMsg->m_nCharPartWidth,
                         0);
              glEnd();
            } else {
              glBegin(GL_POLYGON);

              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap - pMsg->m_nCharPartGap * 4,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap - pMsg->m_nCharPartGap * 4,
                         0);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap - pMsg->m_nCharPartGap * 4,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap + pMsg->m_nCharPartGap * 4,
                         0);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap + pMsg->m_nCharPartWidth + pMsg->m_nCharPartGap * 4,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap + pMsg->m_nCharPartGap * 4,
                         0);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap + pMsg->m_nCharPartWidth + pMsg->m_nCharPartGap * 4,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap - pMsg->m_nCharPartWidth - pMsg->m_nCharPartGap * 4,
                         0);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap + pMsg->m_nCharPartWidth - pMsg->m_nCharPartGap * 4,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap - pMsg->m_nCharPartWidth - pMsg->m_nCharPartGap * 4,
                         0);
              glEnd();
            }
            break;
          case 'P':
            if(bInside) {
              glBegin(GL_TRIANGLES);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap,
                         0);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap + pMsg->m_nCharPartWidth,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap,
                         0);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap - pMsg->m_nCharPartWidth,
                         0);
              glEnd();
            } else {
              glBegin(GL_POLYGON);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap - pMsg->m_nCharPartGap * 4,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap + pMsg->m_nCharPartGap * 4,
                         0);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap + pMsg->m_nCharPartWidth + pMsg->m_nCharPartGap * 4,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap + pMsg->m_nCharPartGap * 4,
                         0);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap + pMsg->m_nCharPartWidth + pMsg->m_nCharPartGap * 4,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap - pMsg->m_nCharPartGap * 4,
                         0);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap + pMsg->m_nCharPartGap * 4,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap - pMsg->m_nCharPartWidth - pMsg->m_nCharPartGap * 4,
                         0);
              glVertex3f( x * pMsg->m_nCharPartWidth + x * pMsg->m_nCharPartGap - pMsg->m_nCharPartGap * 4,
                         -y * pMsg->m_nCharPartWidth - y * pMsg->m_nCharPartGap - pMsg->m_nCharPartWidth - pMsg->m_nCharPartGap * 4,
                         0);
              glEnd();
            }
            break;
        }
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
