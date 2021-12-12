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

  ImVec2 macroDragStart;
  ImVec2 macroDragAreaSize;
  int* macroDragTarget;
  int macroDragLen;
  int macroDragMin, macroDragMax;
  bool macroDragActive;

  ImVec2 macroLoopDragStart;
  ImVec2 macroLoopDragAreaSize;
  signed char* macroLoopDragTarget;
  int macroLoopDragLen;
  bool macroLoopDragActive;

  public:
    void bindEngine(DivEngine* eng);
    bool loop();
    bool init();
    FurnaceGUI();
};
