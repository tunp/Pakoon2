#include "SDL2/SDL.h"
#include <SDL2/SDL_ttf.h>

#include "Pakoon1View.h"

#include <iostream>

#ifdef __EMSCRIPTEN__
#include <GL/gl.h>
#include <emscripten.h>
extern "C" void initialize_gl4es();
#endif

using namespace std;

void mainLoop(CPakoon1View *pakoon1) {
  pakoon1->OnDraw();

  SDL_Event event;
  while(SDL_PollEvent(&event))
  {
    SDL_Point point;
    if(event.type == SDL_KEYDOWN) {
      pakoon1->OnKeyDown(event.key.keysym.sym, 0, 0);
      if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_BACKSPACE) {
        pakoon1->OnChar(event.key.keysym.sym, 0, 0);
      }
    } else if (event.type == SDL_KEYUP) {
      pakoon1->OnKeyUp(event.key.keysym.sym, 0, 0);
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
      if (event.button.button == 1) {
        point.x = event.motion.x;
        point.y = event.motion.y;
        //pakoon1.OnLButtonDown(point);
      }
    } else if (event.type == SDL_MOUSEBUTTONUP) {
      if (event.button.button == 1) {
        point.x = event.motion.x;
        point.y = event.motion.y;
        //pakoon1.OnLButtonUp(point);
      }
    } else if (event.type == SDL_MOUSEMOTION) {
      point.x = event.motion.x;
      point.y = event.motion.y;
      //pakoon1.OnMouseMove(point);
    } else if (event.type == SDL_FINGERDOWN) {
      pakoon1->OnFingerDown(event.tfinger.x, event.tfinger.y, event.tfinger.fingerId);
    } else if (event.type == SDL_FINGERUP) {
      pakoon1->OnFingerUp(event.tfinger.x, event.tfinger.y, event.tfinger.fingerId);
    } else if (event.type == SDL_QUIT) {
      pakoon1->setExit();
    } else if (event.type == SDL_TEXTINPUT) {
      //FIXME handle full text
      pakoon1->OnChar((int) event.text.text[0], 0, 0);
    }
  }
}

void emscripten_loop(void *pakoon1) {
  mainLoop((CPakoon1View *)pakoon1);
}

int main(int argc, char **argv) {
#ifdef __EMSCRIPTEN__
  initialize_gl4es();
#endif
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		cout << "SDL init failed" << endl;
	}

  if(TTF_Init() == -1) {
    cout << "TTF_Init: " << TTF_GetError() << endl;
    return 1;
  }

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); 
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16); 
	
#ifdef __EMSCRIPTEN__
	SDL_Window *window = SDL_CreateWindow("Pakoon2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_OPENGL);
#else
	SDL_Window *window = SDL_CreateWindow("Pakoon2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 0, 0, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
#endif
	if (window == NULL) {
		cout << "error creating window" << endl;
	}
	SDL_GLContext context = SDL_GL_CreateContext(window);

	CPakoon1View pakoon1;
	pakoon1.setWindow(window);
	SDL_Rect rect;
	SDL_GetWindowSize(window, &rect.w, &rect.h);
	pakoon1.setRectWnd(rect);
	pakoon1.OnCreate();
	
#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop_arg((em_arg_callback_func)emscripten_loop, &pakoon1, 0, 1);
	SDL_GL_SetSwapInterval(0);
#else
	SDL_GL_SetSwapInterval(0);
  while (!pakoon1.isExit()) {
    mainLoop(&pakoon1);
  }
#endif
	
  TTF_Quit();
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
	
	return 0;
}
