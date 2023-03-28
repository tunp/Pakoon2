#ifndef KBDBUTTON_H
#define KBDBUTTON_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "Button.h"

class KbdButton : public Button {
private:
  unsigned int *press_key;
public:
  KbdButton(string text, SDL_Color color, SDL_Rect pos, unsigned int *press_key);
  unsigned int *getPressKey();
};

#endif
