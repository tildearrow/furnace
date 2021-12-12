#include "gui.h"
#include "SDL_events.h"
#include "SDL_video.h"
#include "fonts.h"
#include "../ta-log.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <cstdio>
#include <fmt/printf.h>

const int _ZERO=0;
const int _ONE=1;
const int _THREE=3;
const int _SEVEN=7;
const int _FIFTEEN=15;;
const int _THIRTY_ONE=31;
const int _ONE_HUNDRED_TWENTY_SEVEN=127;

const int opOrder[4]={
  0, 2, 1, 3
};

void FurnaceGUI::bindEngine(DivEngine* eng) {
  e=eng;
}

bool FurnaceGUI::loop() {
  while (!quit) {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
      ImGui_ImplSDL2_ProcessEvent(&ev);
      switch (ev.type) {
        case SDL_MOUSEMOTION:
          if (macroDragActive) {
            // do macro drag here!
            if (macroDragLen>0) {
              int x=(ev.motion.x-macroDragStart.x)*macroDragLen/macroDragAreaSize.x;
              if (x<0) x=0;
              if (x>=macroDragLen) x=macroDragLen-1;
              int y=round(macroDragMax-((ev.motion.y-macroDragStart.y)*(double(macroDragMax-macroDragMin)/(double)macroDragAreaSize.y)));
              if (y>macroDragMax) y=macroDragMax;
              if (y<macroDragMin) y=macroDragMin;
              macroDragTarget[x]=y;
            }
          }
          break;
        case SDL_MOUSEBUTTONUP:
          macroDragActive=false;
          break;
        case SDL_QUIT:
          quit=true;
          return true;
          break;
      }
    }
    
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame(sdlWin);
    ImGui::NewFrame();

    ImGui::BeginMainMenuBar();
    if (ImGui::BeginMenu("file")) {
      ImGui::MenuItem("new");
      ImGui::MenuItem("open...");
      ImGui::Separator();
      ImGui::MenuItem("save");
      ImGui::MenuItem("save as...");
      ImGui::Separator();
      if (ImGui::MenuItem("exit")) {
        quit=true;
      }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();

    if (ImGui::Begin("Playback")) {
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

    char selID[16];
    if (ImGui::Begin("Orders")) {
      ImGui::BeginTable("OrdersTable",1+e->getChannelCount(e->song.system));
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      for (int i=0; i<e->getChannelCount(e->song.system); i++) {
        ImGui::TableNextColumn();
        ImGui::Text("%s",e->getChannelShortName(i));
      }
      for (int i=0; i<e->song.ordersLen; i++) {
        ImGui::TableNextRow();
        if (e->getOrder()==i) ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,0x40ffffff);
        ImGui::TableNextColumn();
        snprintf(selID,16,"%.2x##O_S%.2x",i,i);
        if (ImGui::Selectable(selID)) {
          e->setOrder(i);
        }
        for (int j=0; j<e->getChannelCount(e->song.system); j++) {
          ImGui::TableNextColumn();
          snprintf(selID,16,"%.2x##O_%.2x_%.2x",e->song.orders.ord[j][i],j,i);
          if (ImGui::Selectable(selID)) {
            if (e->song.orders.ord[j][i]<0x7f) e->song.orders.ord[j][i]++;
          }
          if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
            if (e->song.orders.ord[j][i]>0) e->song.orders.ord[j][i]--;
          }
        }
      }

      ImGui::EndTable();
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

    if (ImGui::Begin("Instrument Editor")) {
      if (curIns>=e->song.ins.size()) {
        ImGui::Text("no instrument selected");
      } else {
        DivInstrument* ins=e->song.ins[curIns];
        ImGui::InputText("Name",&ins->name);
        if (e->isFMSystem(e->song.system)) ImGui::Checkbox("FM",&ins->mode);

        if (ins->mode) { // FM
          ImGui::SliderScalar("Algorithm",ImGuiDataType_U8,&ins->fm.alg,&_ZERO,&_SEVEN);
          ImGui::SliderScalar("Feedback",ImGuiDataType_U8,&ins->fm.fb,&_ZERO,&_SEVEN);
          ImGui::SliderScalar("LFO > Frequency",ImGuiDataType_U8,&ins->fm.fms,&_ZERO,&_SEVEN);
          ImGui::SliderScalar("LFO > Amplitude",ImGuiDataType_U8,&ins->fm.ams,&_ZERO,&_THREE);
          for (int i=0; i<4; i++) {
            DivInstrumentFM::Operator& op=ins->fm.op[opOrder[i]];
            ImGui::Separator();

            ImGui::PushID(fmt::sprintf("op%d",i).c_str());
            ImGui::Text("Operator %d",i+1);
            ImGui::SliderScalar("Level",ImGuiDataType_U8,&op.tl,&_ZERO,&_ONE_HUNDRED_TWENTY_SEVEN);
            ImGui::SliderScalar("Attack",ImGuiDataType_U8,&op.ar,&_ZERO,&_THIRTY_ONE);
            ImGui::SliderScalar("Decay",ImGuiDataType_U8,&op.dr,&_ZERO,&_THIRTY_ONE);
            ImGui::SliderScalar("Sustain",ImGuiDataType_U8,&op.sl,&_ZERO,&_FIFTEEN);
            ImGui::SliderScalar("Decay 2",ImGuiDataType_U8,&op.d2r,&_ZERO,&_THIRTY_ONE);
            ImGui::SliderScalar("Release",ImGuiDataType_U8,&op.rr,&_ZERO,&_FIFTEEN);
            ImGui::SliderScalar("Multiplier",ImGuiDataType_U8,&op.mult,&_ZERO,&_FIFTEEN);
            ImGui::SliderScalar("EnvScale",ImGuiDataType_U8,&op.rs,&_ZERO,&_THREE);
            ImGui::SliderScalar("Detune",ImGuiDataType_U8,&op.dt,&_ZERO,&_SEVEN);
            if (e->song.system==DIV_SYSTEM_ARCADE) {
              ImGui::SliderScalar("Detune 2",ImGuiDataType_U8,&op.dt2,&_ZERO,&_THREE);
            } else {
              bool ssgOn=op.ssgEnv&8;
              unsigned char ssgEnv=op.ssgEnv&7;
              ImGui::SliderScalar("SSG-EG",ImGuiDataType_U8,&ssgEnv,&_ZERO,&_SEVEN);
              ImGui::SameLine();
              if (ImGui::Checkbox("##SSGOn",&ssgOn)) {
                op.ssgEnv=(op.ssgEnv&7)|(ssgOn<<3);
              }
            }

            bool amOn=op.am;
            if (ImGui::Checkbox("AM",&amOn)) op.am=amOn;
            ImGui::PopID();
          }
        } else { // STD
          float asFloat[128];
          float loopIndicator[128];

          // volume macro
          ImGui::Separator();
          ImGui::Text("Volume Macro");
          for (int i=0; i<ins->std.volMacroLen; i++) {
            asFloat[i]=ins->std.volMacro[i];
            loopIndicator[i]=(ins->std.volMacroLoop!=-1 && i>=ins->std.volMacroLoop);
          }
          ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(0.0f,0.0f));
          int volMax=e->getMaxVolume();
          ImGui::PlotHistogram("##IVolMacro",asFloat,ins->std.volMacroLen,0,NULL,0,volMax,ImVec2(400.0f*dpiScale,200.0f*dpiScale));
          if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            macroDragStart=ImGui::GetItemRectMin();
            macroDragAreaSize=ImVec2(400.0f*dpiScale,200.0f*dpiScale);
            macroDragMin=0;
            macroDragMax=volMax;
            macroDragLen=ins->std.volMacroLen;
            macroDragActive=true;
            macroDragTarget=ins->std.volMacro;
            printf("Start.\n");
          }
          ImGui::PlotHistogram("##IVolMacroLoop",loopIndicator,ins->std.volMacroLen,0,NULL,0,1,ImVec2(400.0f*dpiScale,16.0f*dpiScale));
          if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            macroLoopDragStart=ImGui::GetItemRectMin();
            macroLoopDragAreaSize=ImVec2(400.0f*dpiScale,16.0f*dpiScale);
            macroLoopDragLen=ins->std.volMacroLen;
            macroLoopDragTarget=&ins->std.volMacroLoop;
            macroLoopDragActive=true;
          }
          ImGui::PopStyleVar();
          if (ImGui::InputScalar("Length##IVolMacroL",ImGuiDataType_U8,&ins->std.volMacroLen,&_ONE,&_THREE)) {
            if (ins->std.volMacroLen>127) ins->std.volMacroLen=127;
          }

          // arp macro
          ImGui::Separator();
          ImGui::Text("Arpeggio Macro");
          for (int i=0; i<ins->std.arpMacroLen; i++) {
            asFloat[i]=ins->std.arpMacro[i];
          }
          ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(0.0f,0.0f));
          ImGui::PlotHistogram("##IArpMacro",asFloat,ins->std.arpMacroLen,0,NULL,-92,82,ImVec2(400.0f*dpiScale,200.0f*dpiScale));
          if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            macroDragStart=ImGui::GetItemRectMin();
            macroDragAreaSize=ImVec2(400.0f*dpiScale,200.0f*dpiScale);
            macroDragMin=-92;
            macroDragMax=82;
            macroDragLen=ins->std.arpMacroLen;
            macroDragActive=true;
            macroDragTarget=ins->std.arpMacro;
            printf("Start.\n");
          }
          ImGui::PopStyleVar();
          if (ImGui::InputScalar("Length##IArpMacroL",ImGuiDataType_U8,&ins->std.arpMacroLen,&_ONE,&_THREE)) {
            if (ins->std.arpMacroLen>127) ins->std.arpMacroLen=127;
          }
        }
      }
    }
    ImGui::End();

    if (ImGui::Begin("Pattern")) {
      ImGui::Text("TODO");
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
  float dpiScaleF;

  sdlWin=SDL_CreateWindow("Furnace",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,scrW*dpiScale,scrH*dpiScale,SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALLOW_HIGHDPI);
  if (sdlWin==NULL) {
    logE("could not open window!\n");
    return false;
  }
  SDL_GetDisplayDPI(SDL_GetWindowDisplayIndex(sdlWin),&dpiScaleF,NULL,NULL);
  dpiScale=round(dpiScaleF/96.0f);
  if (dpiScale<1) dpiScale=1;
  SDL_SetWindowSize(sdlWin,scrW*dpiScale,scrH*dpiScale);

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
  curOctave(3),
  macroDragStart(0,0),
  macroDragAreaSize(0,0),
  macroDragTarget(NULL),
  macroDragLen(0),
  macroDragMin(0),
  macroDragMax(0),
  macroDragActive(false) {
}
