#ifndef DLG_ON_SCREEN_KBD_H
#define DLG_ON_SCREEN_KBD_H

#include "Dialog.h"
#include "KbdButton.h"

#include "../Pakoon1View.h"

class DlgOnScreenKbd : public Dialog {
private:
	CPakoon1View *pView;
  unsigned int esc_code;
public:
	DlgOnScreenKbd(CPakoon1View *pView, SDL_Rect m_rectWnd);
	~DlgOnScreenKbd();
	bool onMousePress(int x, int y);
	bool onMouseRelease(int x, int y);
	bool onMouseMove(int x, int y);
  bool onFingerDown(int x, int y, int finger_id);
	static void keyPressCallback(void *p, Button *b);
	static void keyReleaseCallback(void *p, Button *b);
	void keyPress(unsigned int *key_press);
	void keyRelease(unsigned int *key_press);
  bool needsRedraw();
};

#endif
