#ifndef BUTTON_H
#define BUTTON_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "Item.h"

class Button : public Item {
private:
	string text;
	SDL_Color color;
	bool pressed;
	bool selected;
  int finger_id;
	SDL_Surface *button_up;
	SDL_Surface *button_down;
	void (*button_func)(void *p, Button *b);
	void (*button_press_func)(void *p, Button *b);
	void (*button_release_func)(void *p, Button *b);
	void *p;
public:
	//void init();
	//void destroy();
	Button(string text, SDL_Color color, SDL_Rect pos);
	~Button();
	void drawButton();
	bool onMousePress(int x, int y);
	void onMouseRelease(int x, int y);
	bool onFingerDown(int x, int y, int finger_id);
	bool onFingerUp(int x, int y, int finger_id);
	void setButtonFunc(void (*button_func)(void *p, Button *b), void *p);
	void setButtonPressFunc(void (*button_press_func)(void *p, Button *b), void *p);
	void setButtonReleaseFunc(void (*button_release_func)(void *p, Button *b), void *p);
	void setText(string text);
	string getText();
	void setColor(SDL_Color color);
	void setSelected(bool selected);
	void selectSurface();
};

#endif
