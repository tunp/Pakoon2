#ifndef ITEM_H
#define ITEM_H

class Item {
protected:
	SDL_Surface *surface;
  bool needs_redraw;
private:
	SDL_Rect pos;
public:
	Item(SDL_Rect pos);
	virtual ~Item();
	//virtual void init();
	//virtual void destroy();
	virtual SDL_Surface *getSurface();
	void setSurface(SDL_Surface *surface);
	void setPos(SDL_Rect pos);
	SDL_Rect *getPos();
	virtual bool onMousePress(int x, int y);
	virtual void onMouseRelease(int x, int y);
	virtual void onMouseMove(int x, int y);
	virtual bool onFingerDown(int x, int y, int finger_id);
	virtual bool onFingerUp(int x, int y, int finger_id);
  bool needsRedraw();
  void drawDone();
};

#endif
