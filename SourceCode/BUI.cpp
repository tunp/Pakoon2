//
// BUI: Misc UI components
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#include "BUI.h"
#include "BSimulation.h"
#include "BTextRenderer.h"
#include "BGame.h"
#include "Pakoon1View.h"
#include "BMenu.h"

#include <SDL2/SDL.h>

BTextRenderer BUI::m_textRenderer;
BUISelectionList *BUI::m_pSelList = 0;
void (CPakoon1View::* (BUI::m_pPrevKeyDownFunction))(unsigned, unsigned, unsigned) = 0;
int BUI::m_nPrevSliderValue = 0;
int *BUI::m_pnSliderValue = 0;
string BUI::m_sPrevSValue = "";
string *BUI::m_psValue = 0;

//*************************************************************************************************
void BUI::StartUsingSelectionList(BUISelectionList *pList, void (CPakoon1View::*pPrevKeyDownFunction)(unsigned, unsigned, unsigned)) {
  m_pSelList = pList;
  pList->SaveSelection();
  m_pPrevKeyDownFunction = pPrevKeyDownFunction;
  CPakoon1View *pView = BGame::GetView();
  pView->m_pKeyDownFunction = &CPakoon1View::OnKeyDownSelectionList;

}

//*************************************************************************************************
void BUI::StartUsingSlider(int *pnSliderValue, void (CPakoon1View::*pPrevKeyDownFunction)(unsigned, unsigned, unsigned)) {
  m_pnSliderValue = pnSliderValue;
  m_nPrevSliderValue = *pnSliderValue;
  m_pPrevKeyDownFunction = pPrevKeyDownFunction;
  CPakoon1View *pView = BGame::GetView();
  pView->m_pKeyDownFunction = &CPakoon1View::OnKeyDownSlider;

}

//*************************************************************************************************
void BUI::StartUsingEditbox(string *psValue, void (CPakoon1View::*pPrevKeyDownFunction)(unsigned, unsigned, unsigned)) {
  m_psValue = psValue;
  m_sPrevSValue = *psValue;
  m_pPrevKeyDownFunction = pPrevKeyDownFunction;
  CPakoon1View *pView = BGame::GetView();
  pView->m_pKeyDownFunction = &CPakoon1View::OnKeyDownEditbox;
}


//*************************************************************************************************
BUISelectionList::BUISelectionList() {
  m_nSelected = -1;
  m_psItems = 0;
  m_nItems = 0;
  m_sPrompt = "";
  m_dOffsetToLeft = 0.0;
}


//*************************************************************************************************
void BUISelectionList::SaveSelection() {
  m_nSavedSelection = m_nSelected;
}

//*************************************************************************************************
void BUISelectionList::Cancel() {
  m_nSelected = m_nSavedSelection;
}


//*************************************************************************************************
void BUISelectionList::SetItems(string *psItems, int nItems, string sPrompt) {
  m_psItems = psItems;
  m_nItems = nItems;
  m_sPrompt = sPrompt;
}


//*************************************************************************************************
int BUISelectionList::SelectItem(string sItem) {
  // Search for the string and select that item.
  // Return selected item's index
  for(int i = 0; i < m_nItems; ++i) {
    if(sItem.compare(m_psItems[i]) == 0) {
      m_nSelected = i;
      return i;
    }
  }
  m_nSelected = -1;
  return -1;
}


//*************************************************************************************************
void BUISelectionList::AdvanceSelection(int nAmount) {
  m_nSelected += nAmount;
  if(m_nSelected < -100) {
    m_nSelected = 0;
  }
  if(m_nSelected < 0) {
    m_nSelected = (m_nItems - 1);
  }
  if(m_nSelected > (m_nItems + 100)) {
    m_nSelected = (m_nItems - 1);
  }
  if(m_nSelected > (m_nItems - 1)) {
    m_nSelected = 0;
  }
}


//*************************************************************************************************
int BUISelectionList::GetSelectedItem(string &sItemText) {
  if(m_nSelected != -1) {
    sItemText = m_psItems[m_nSelected];
  }
  return m_nSelected;
}

const double g_cdPI = 3.141592654;

//*************************************************************************************************
void BUISelectionList::DrawAt(double dX, 
                              double dY, 
                              BTextRenderer::TTextAlign textAlign,
                              double dRed, 
                              double dGreen, 
                              double dBlue,
                              bool   bScrolling,
                              bool   bWithBackground,
                              bool   bFlashCursor,
                              BMenu *pMenu,
                              bool   bWithTriangle) {
  double dCharHeight = BUI::TextRenderer()->GetCharHeight();
  double dCharWidth  = BUI::TextRenderer()->GetCharWidth();

  glPushMatrix();

  glTranslated(dX, dY, 0);
  if(bScrolling && (m_nSelected != -1)) {
    glTranslated(0, dCharHeight * double(m_nSelected), 0);
  } else {
    glTranslated(0, (dCharHeight * double(m_nItems)) / 2.0, 0);
  }

  // Draw background, if requested
  int i;
  if(bWithBackground) {
    double dMaxLen = 0.0;
    for(i = 0; i < m_nItems; ++i) {
      if(BUI::TextRenderer()->GetStringWidth(m_psItems[i]) > dMaxLen) {
        dMaxLen = BUI::TextRenderer()->GetStringWidth(m_psItems[i]);
      }
    }
    double dXOffset = 0;
    if(textAlign == BTextRenderer::ALIGN_CENTER) {
      dXOffset = -dMaxLen / 2.0;
    } else if(textAlign == BTextRenderer::ALIGN_RIGHT) {
      dXOffset = -dMaxLen;
    }

    double dYBase = double(m_nItems - 1) * -dCharHeight;

    // Shadow
    glTranslated(6, -6, 0);
    OpenGLHelpers::SetColorFull(0, 0, 0, 0.5);
    glBegin(GL_QUADS);
    glVertex3f(-5 + dXOffset, dYBase - dCharHeight / 2.0, 0);
    glVertex3f(-5 + dXOffset, dYBase + double(m_nItems - 1) * dCharHeight + dCharHeight / 2.0, 0);
    glVertex3f(dXOffset + dMaxLen + 6, dYBase  + double(m_nItems - 1) * dCharHeight + dCharHeight / 2.0, 0);
    glVertex3f(dXOffset + dMaxLen + 6, dYBase - dCharHeight / 2.0, 0);
    glEnd();

    // Actual background
    glTranslated(-6, 6, 0);

    OpenGLHelpers::SetColorFull(0.15, 0.15, 0.15, 1);
    glBegin(GL_QUADS);
    glVertex3f(-5 + dXOffset, dYBase - dCharHeight / 2.0, 0);
    glVertex3f(-5 + dXOffset, dYBase + double(m_nItems - 1) * dCharHeight + dCharHeight / 2.0, 0);
    glVertex3f(dXOffset + dMaxLen + 6, dYBase  + double(m_nItems - 1) * dCharHeight + dCharHeight / 2.0, 0);
    glVertex3f(dXOffset + dMaxLen + 6, dYBase - dCharHeight / 2.0, 0);
    glEnd();

    OpenGLHelpers::SetColorFull(1, 1, 1, 1);
    glBegin(GL_LINE_STRIP);
    glVertex3f(-5 + dXOffset, dYBase - dCharHeight / 2.0, 0);
    glVertex3f(-5 + dXOffset, dYBase + double(m_nItems - 1) * dCharHeight + dCharHeight / 2.0, 0);
    glVertex3f(dXOffset + dMaxLen + 6, dYBase  + double(m_nItems - 1) * dCharHeight + dCharHeight / 2.0, 0);
    glVertex3f(dXOffset + dMaxLen + 6, dYBase - dCharHeight / 2.0, 0);
    glVertex3f(-5 + dXOffset, dYBase - dCharHeight / 2.0, 0);
    glEnd();
  }

  // First draw selection rectangle behind the selected item
  if(m_nSelected != -1) {
    glDisable(GL_TEXTURE_2D);
    double dLen = BUI::TextRenderer()->GetStringWidth(m_psItems[m_nSelected]);
    double dYBase = double(m_nSelected) * -dCharHeight;

    double dXOffset = 0;
    if(textAlign == BTextRenderer::ALIGN_CENTER) {
      dXOffset = -dLen / 2.0;
    } else if(textAlign == BTextRenderer::ALIGN_RIGHT) {
      dXOffset = -dLen;
    }

    double dAlpha = fabs(double(SDL_GetTicks() % 1000) - 500.0) / 500.0;
    dAlpha = 0.5 + 0.5 * dAlpha;

    if(BGame::m_bMultiplayOn &&
       BGame::m_bOKToProceedInMultiplayMenu) {
      dAlpha = (SDL_GetTicks() % 1000) / (1000 / 8) % 2;
    }

    if(!bFlashCursor || BGame::m_remotePlayer[BGame::GetMultiplay()->m_params.m_nMyPlace].m_bSelectionMade) {
      dAlpha = 0.75;
    }

    glPushMatrix();
    glTranslated(dXOffset - 1, dYBase - 0.5, 0);
    OpenGLHelpers::SetColorFull(1, 0.5, 0, dAlpha);

    double dRoundRad = 5.0;
    double dHeight = dCharHeight / 3.0;

    glBegin(GL_QUADS);

    glVertex3f(-4,  -dHeight, 0);
    glVertex3f(-9, -dHeight, 0);
    glVertex3f(-9,  dHeight, 0);
    glVertex3f(-4,   dHeight, 0);

    glVertex3f(dLen + 5,  -dHeight, 0);
    glVertex3f(dLen + 10, -dHeight, 0);
    glVertex3f(dLen + 10,  dHeight, 0);
    glVertex3f(dLen + 5,   dHeight, 0);

    glVertex3f(-4 + dRoundRad, -dHeight - 5, 0);
    glVertex3f(-4 + dRoundRad, -dHeight - 10, 0);
    glVertex3f(dLen + 5 - dRoundRad, -dHeight - 10, 0);
    glVertex3f(dLen + 5 - dRoundRad, -dHeight - 5, 0);

    glVertex3f(-4 + dRoundRad, dHeight + 5, 0);
    glVertex3f(-4 + dRoundRad, dHeight + 10, 0);
    glVertex3f(dLen + 5 - dRoundRad, dHeight + 10, 0);
    glVertex3f(dLen + 5 - dRoundRad, dHeight + 5, 0);

    glEnd();

    // Corners

    double dXCenter = 1;
    double dYCenter = -dHeight;
    double dAngle;
    glBegin(GL_QUAD_STRIP);
    for(dAngle = 180.0; dAngle <= 270.0; dAngle += 15.0) {
      glVertex3f(dXCenter + sin(dAngle / 180.0 * g_cdPI) * dRoundRad,
                 dYCenter + cos(dAngle / 180.0 * g_cdPI) * dRoundRad,
                 0.0);
      glVertex3f(dXCenter + sin(dAngle / 180.0 * g_cdPI) * dRoundRad * 2.0,
                 dYCenter + cos(dAngle / 180.0 * g_cdPI) * dRoundRad * 2.0,
                 0.0);
    }
    glEnd();

    dXCenter = dLen;
    dYCenter = -dHeight;
    glBegin(GL_QUAD_STRIP);
    for(dAngle = 90.0; dAngle <= 180.0; dAngle += 15.0) {
      glVertex3f(dXCenter + sin(dAngle / 180.0 * g_cdPI) * dRoundRad,
                 dYCenter + cos(dAngle / 180.0 * g_cdPI) * dRoundRad,
                 0.0);
      glVertex3f(dXCenter + sin(dAngle / 180.0 * g_cdPI) * dRoundRad * 2.0,
                 dYCenter + cos(dAngle / 180.0 * g_cdPI) * dRoundRad * 2.0,
                 0.0);
    }
    glEnd();

    dXCenter = 1;
    dYCenter = dHeight;
    glBegin(GL_QUAD_STRIP);
    for(dAngle = 270.0; dAngle <= 360.0; dAngle += 15.0) {
      glVertex3f(dXCenter + sin(dAngle / 180.0 * g_cdPI) * dRoundRad,
                 dYCenter + cos(dAngle / 180.0 * g_cdPI) * dRoundRad,
                 0.0);
      glVertex3f(dXCenter + sin(dAngle / 180.0 * g_cdPI) * dRoundRad * 2.0,
                 dYCenter + cos(dAngle / 180.0 * g_cdPI) * dRoundRad * 2.0,
                 0.0);
    }
    glEnd();

    dXCenter = dLen;
    dYCenter = dHeight;
    glBegin(GL_QUAD_STRIP);
    for(dAngle = 0.0; dAngle <= 90.0; dAngle += 15.0) {
      glVertex3f(dXCenter + sin(dAngle / 180.0 * g_cdPI) * dRoundRad,
                 dYCenter + cos(dAngle / 180.0 * g_cdPI) * dRoundRad,
                 0.0);
      glVertex3f(dXCenter + sin(dAngle / 180.0 * g_cdPI) * dRoundRad * 2.0,
                 dYCenter + cos(dAngle / 180.0 * g_cdPI) * dRoundRad * 2.0,
                 0.0);
    }
    glEnd();

    // Triangle 
    if(bWithTriangle) {
      double dR, dG, dB;
      if(BGame::m_bMultiplayOn) {
        BGame::GetMultiplayerColor(BGame::GetMultiplay()->m_params.m_nMyPlace, dR, dG, dB);
      } else {
        BGame::GetMultiplayerColor(0, dR, dG, dB);
      }
      OpenGLHelpers::SetColorFull(dR, dG, dB, dAlpha);

      glBegin(GL_TRIANGLES);
      glVertex3f(-10,  0, 0);
      glVertex3f(-30, 10, 0);
      glVertex3f(-30, -10, 0);
      glEnd();

      /*
      OpenGLHelpers::SetColorFull(0, 0, 0, 1);

      glBegin(GL_TRIANGLES);
      glVertex3f(-17,  0, 0);
      glVertex3f(-27, 5, 0);
      glVertex3f(-27, -5, 0);
      glEnd();
      */
    }

    glPopMatrix();
  }

  // Then draw items
  BUI::TextRenderer()->StartRenderingText();
  for(i = 0; i < m_nItems; ++i) {

    double dLen = double(m_psItems[i].length()) * dCharWidth;
    double dXOffset = 0;
    if(textAlign == BTextRenderer::ALIGN_CENTER) {
      dXOffset = -dLen / 2.0;
    } else if(textAlign == BTextRenderer::ALIGN_RIGHT) {
      dXOffset = -dLen;
    }

    double dR = dRed, dG = dGreen, dB = dBlue;
    double dAlpha = 1.0;
    if(pMenu) {
      // Check for special needs
      dR = pMenu->m_items[i].m_dRed;
      dG = pMenu->m_items[i].m_dGreen;
      dB = pMenu->m_items[i].m_dBlue;
      if(pMenu->m_items[i].m_bDisabled) {
        dAlpha = 0.2;
      }
    }
    BUI::TextRenderer()->DrawTextAt(0, // dXOffset, 
                                    double(i) * -dCharHeight, 
                                    m_psItems[i],
                                    textAlign,
                                    dR,
                                    dG,
                                    dB,
                                    dAlpha);
  }
  BUI::TextRenderer()->StopRenderingText();
  glPopMatrix();
}




//*************************************************************************************************
BUIEdit::BUIEdit() {
  m_nMaxLength = 0;
  m_sValue = "";
  m_sPrompt = "";
}


//*************************************************************************************************
void BUIEdit::Setup(string sPrompt, string sValue, int nMaxLength) {
  m_sPrompt = sPrompt;
  m_sValue = sValue;
  m_nMaxLength = nMaxLength;
  status = EDITING;
}


//*************************************************************************************************
void BUIEdit::ProcessChar(unsigned char c) {

  if(c == 190) {
    c = '.';
  }

  if(!((toupper(c) >= 32) && 
       (toupper(c) <= 96) || 
       (c == '.') ||
       (c == '/') ||
       (c == SDLK_BACKSPACE) ||
       (c == SDLK_RETURN) || 
       (c == SDLK_ESCAPE))) {
    // Bad character, no go
    return;
  }

  if(c == SDLK_BACKSPACE) {
    // Eat one character
    if(m_sValue.length() > 0) {
      m_sValue.erase(m_sValue.length() - 1, 1);
    }
    return;
  }

  if(c == SDLK_RETURN) {
    status = READY;
    return;
  }

  if(c == SDLK_ESCAPE) {
    status = CANCELED;
    return;
  }

  // Add writable character
  if(m_sValue.length() < m_nMaxLength) {
    m_sValue += c;
  }
  return;
}


//*************************************************************************************************
string BUIEdit::GetValue(TStatus &rStatus) {
  rStatus = status;
  return m_sValue;
}


//*************************************************************************************************
void BUIEdit::DrawAt(double dX, double dY, bool bCursor, BTextRenderer::TTextAlign textAlign, BMenuItem *pMenuItem) {
  double dCharHeight = BUI::TextRenderer()->GetCharHeight();
  double dCharWidth  = BUI::TextRenderer()->GetCharWidth();

  glPushMatrix();

  glTranslated(dX, dY, 0);

  if(textAlign == BTextRenderer::ALIGN_LEFT) {
    glTranslated(BUI::TextRenderer()->GetStringWidth(m_sPrompt), 0, 0);
  }

  BUI::TextRenderer()->StartRenderingText();

  double dAlpha = 1.0;
  if(pMenuItem && pMenuItem->m_bDisabled) {
    dAlpha = 0.2;
  }

  // Prompt
  BUI::TextRenderer()->DrawTextAt(0, 
                                  0, 
                                  m_sPrompt,
                                  BTextRenderer::ALIGN_RIGHT,
                                  1,
                                  0.75,
                                  0.5,
                                  dAlpha);


  // Value
  BUI::TextRenderer()->DrawTextAt(0, 
                                  0, 
                                  m_sValue,
                                  BTextRenderer::ALIGN_LEFT,
                                  0.75,
                                  0.75,
                                  0.75,
                                  dAlpha);

  BUI::TextRenderer()->StopRenderingText();

  // Cursor

  if(bCursor) {

    glDisable(GL_TEXTURE_2D);
    double dXOffset = BUI::TextRenderer()->GetStringWidth(m_sValue);
    double dLen = dCharWidth * 0.7;

    double dAlpha = fabs(double(SDL_GetTicks() % 1000) - 500.0) / 500.0;

    OpenGLHelpers::SetColorFull(1, 0.5, 0, 0.5 + 0.5 * dAlpha);
    glTranslated(-1, -1, 0);
    glBegin(GL_QUADS);
    glVertex3f(dXOffset, -dCharHeight / 2.0, 0);
    glVertex3f(dXOffset, dCharHeight / 2.0, 0);
    glVertex3f(dXOffset + dLen, dCharHeight / 2.0, 0);
    glVertex3f(dXOffset + dLen, -dCharHeight / 2.0, 0);
    glEnd();
  }

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
