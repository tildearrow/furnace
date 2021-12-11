#include "gui.h"
#include "fonts.h"
#include "../ta-log.h"
#include "imgui.h"
#include <fmt/printf.h>

void FurnaceGUI::bindEngine(DivEngine* eng) {
  e=eng;
}

bool FurnaceGUI::loop() {
  while (!quit) {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
      ImGui_ImplSDL2_ProcessEvent(&ev);
      switch (ev.type) {
        case SDL_QUIT:
          quit=true;
          return true;
          break;
      }
    }
    
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame(sdlWin);
    ImGui::NewFrame();

    if (ImGui::Begin("Debug")) {
      ImGui::Text("Hello world!\n");
      ImGui::InputScalar("Speed 1",ImGuiDataType_U8,&e->song.speed1);
      ImGui::InputScalar("Speed 2",ImGuiDataType_U8,&e->song.speed2);
      unsigned char ord=e->getOrder();
      if (ImGui::InputScalar("Order",ImGuiDataType_U8,&ord)) {
        e->setOrder(ord);
      }
      if (ImGui::Button("Play")) {
        e->play();
      }
      if (ImGui::Button("Stop")) {
        e->stop();
      }
    }
    ImGui::End();

    if (ImGui::Begin("Instruments")) {
      for (int i=0; i<e->song.ins.size(); i++) {
        DivInstrument* ins=e->song.ins[i];
        if (ImGui::Selectable(fmt::sprintf("%d: %s##_INS%d\n",i,ins->name,i).c_str(),curIns==i)) {
          curIns=i;
        }
      }
    }
    ImGui::End();

    SDL_RenderClear(sdlRend);
    ImGui::Render();
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(sdlRend);
  }
  return false;
}

bool FurnaceGUI::init() {
  sdlWin=SDL_CreateWindow("Furnace",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,scrW*dpiScale,scrH*dpiScale,SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALLOW_HIGHDPI);
  if (sdlWin==NULL) {
    logE("could not open window!\n");
    return false;
  }

  sdlRend=SDL_CreateRenderer(sdlWin,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC|SDL_RENDERER_TARGETTEXTURE);

  if (sdlRend==NULL) {
    logE("could not init renderer! %s\n",SDL_GetError());
    return false;
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGui::StyleColorsDark();
  ImGui_ImplSDL2_InitForSDLRenderer(sdlWin);
  ImGui_ImplSDLRenderer_Init(sdlRend);

  ImGui::GetStyle().ScaleAllSizes(dpiScale);

  if ((mainFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(defFont_main_compressed_data,defFont_main_compressed_size,18*dpiScale))==NULL) {
    logE("could not load UI font!\n");
    return false;
  }
  if ((patFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(defFont_pat_compressed_data,defFont_pat_compressed_size,18*dpiScale))==NULL) {
    logE("could not load pattern font!\n");
    return false;
  }
  return true;
}

FurnaceGUI::FurnaceGUI():
  e(NULL),
  quit(false),
  scrW(1280),
  scrH(800),
  dpiScale(1),
  curIns(0),
  curOctave(3) {
}
