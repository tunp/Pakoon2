#include <string>
#include <iostream>

using namespace std;

#include "KbdButton.h"

KbdButton::KbdButton(string text, SDL_Color color, SDL_Rect pos, unsigned int *press_key) : Button(text, color, pos) {
  this->press_key = press_key;
}

unsigned int *KbdButton::getPressKey() {
  return this->press_key;
}
