#include "../engine/engine.h"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include <SDL.h>

enum FurnaceGUIColors {
  GUI_COLOR_BACKGROUND=0,
  GUI_COLOR_FRAME_BACKGROUND,
  GUI_COLOR_HEADER,
  GUI_COLOR_ITEM_BACKGROUND,
  GUI_COLOR_ITEM_FOREGROUND,
  GUI_COLOR_PATTERN_HI_1,
  GUI_COLOR_PATTERN_HI_2,
  GUI_COLOR_PATTERN_ROW_INDEX,
  GUI_COLOR_PATTERN_ACTIVE,
  GUI_COLOR_PATTERN_INACTIVE,
  GUI_COLOR_PATTERN_INS,
  GUI_COLOR_PATTERN_VOLUME_MAX,
  GUI_COLOR_PATTERN_VOLUME_HALF,
  GUI_COLOR_PATTERN_VOLUME_MIN,
  GUI_COLOR_PATTERN_EFFECT_INVALID,
  GUI_COLOR_PATTERN_EFFECT_PITCH,
  GUI_COLOR_PATTERN_EFFECT_VOLUME,
  GUI_COLOR_PATTERN_EFFECT_PANNING,
  GUI_COLOR_PATTERN_EFFECT_SONG,
  GUI_COLOR_PATTERN_EFFECT_TIME,
  GUI_COLOR_PATTERN_EFFECT_SPEED,
  GUI_COLOR_PATTERN_EFFECT_SYS_PRIMARY,
  GUI_COLOR_PATTERN_EFFECT_SYS_SECONDARY,
  GUI_COLOR_PATTERN_EFFECT_MISC,
  GUI_COLOR_MAX
};

class FurnaceGUI {
  DivEngine* e;

  SDL_Window* sdlWin;
  SDL_Renderer* sdlRend;

  bool quit;

  int scrW, scrH;

  double dpiScale;

  ImFont* mainFont;
  ImFont* patFont;
  ImVec4 uiColors[GUI_COLOR_MAX];
  ImVec4 volColors[128];

  int curIns, curOctave, oldRow;

  int arpMacroScroll;

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

  float nextScroll;

  public:
    const char* noteName(short note, short octave);
    void bindEngine(DivEngine* eng);
    void updateScroll(int amount);
    bool loop();
    bool init();
    FurnaceGUI();
};
