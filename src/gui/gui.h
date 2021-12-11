#include "../engine/engine.h"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include <SDL.h>

class FurnaceGUI {
  DivEngine* e;

  SDL_Window* sdlWin;
  SDL_Renderer* sdlRend;

  bool quit;

  int scrW, scrH;

  double dpiScale;

  ImFont* mainFont;
  ImFont* patFont;

  int curIns, curOctave;

  public:
    void bindEngine(DivEngine* eng);
    bool loop();
    bool init();
    FurnaceGUI();
};
