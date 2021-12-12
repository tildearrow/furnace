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
const int _FIFTEEN=15;
const int _THIRTY_ONE=31;
const int _SIXTY_FOUR=64;
const int _ONE_HUNDRED=100;
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
          if (macroLoopDragActive) {
            if (macroLoopDragLen>0) {
              int x=(ev.motion.x-macroLoopDragStart.x)*macroLoopDragLen/macroLoopDragAreaSize.x;
              if (x<0) x=0;
              if (x>=macroLoopDragLen) x=-1;
              *macroLoopDragTarget=x;
            }
          }
          break;
        case SDL_MOUSEBUTTONUP:
          macroDragActive=false;
          macroLoopDragActive=false;
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

          // GB specifics
          if (e->song.system==DIV_SYSTEM_GB) {
            ImGui::SliderScalar("Volume",ImGuiDataType_U8,&ins->gb.envVol,&_ZERO,&_FIFTEEN);
            ImGui::SliderScalar("Envelope Length",ImGuiDataType_U8,&ins->gb.envLen,&_ZERO,&_SEVEN);
            ImGui::SliderScalar("Sound Length",ImGuiDataType_U8,&ins->gb.soundLen,&_ZERO,&_SIXTY_FOUR,ins->gb.soundLen>63?"Infinity":"%d");
            bool goesUp=ins->gb.envDir;
            if (ImGui::Checkbox("Up",&goesUp)) {
              ins->gb.envDir=goesUp;
            }
          }

          // C64 specifics
          if (e->song.system==DIV_SYSTEM_C64_6581 || e->song.system==DIV_SYSTEM_C64_8580) {
            ImGui::Text("Waveform");
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(ins->c64.triOn)?0.6f:0.2f,0.2f,1.0f));
            if (ImGui::Button("tri")) {
              ins->c64.triOn=!ins->c64.triOn;
            }
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(ins->c64.sawOn)?0.6f:0.2f,0.2f,1.0f));
            if (ImGui::Button("saw")) {
              ins->c64.sawOn=!ins->c64.sawOn;
            }
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(ins->c64.pulseOn)?0.6f:0.2f,0.2f,1.0f));
            if (ImGui::Button("pulse")) {
              ins->c64.pulseOn=!ins->c64.pulseOn;
            }
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(ins->c64.noiseOn)?0.6f:0.2f,0.2f,1.0f));
            if (ImGui::Button("noise")) {
              ins->c64.noiseOn=!ins->c64.noiseOn;
            }
            ImGui::PopStyleColor();

            ImGui::SliderScalar("Attack",ImGuiDataType_U8,&ins->c64.a,&_ZERO,&_FIFTEEN);
            ImGui::SliderScalar("Decay",ImGuiDataType_U8,&ins->c64.d,&_ZERO,&_FIFTEEN);
            ImGui::SliderScalar("Sustain",ImGuiDataType_U8,&ins->c64.s,&_ZERO,&_FIFTEEN);
            ImGui::SliderScalar("Release",ImGuiDataType_U8,&ins->c64.r,&_ZERO,&_FIFTEEN);
            ImGui::SliderScalar("Duty",ImGuiDataType_U8,&ins->c64.duty,&_ZERO,&_ONE_HUNDRED);

            bool ringMod=ins->c64.ringMod;
            if (ImGui::Checkbox("Ring Modulation",&ringMod)) ins->c64.ringMod=ringMod;
            bool oscSync=ins->c64.oscSync;
            if (ImGui::Checkbox("Oscillator Sync",&oscSync)) ins->c64.oscSync=oscSync;

            ImGui::Checkbox("Enable filter",&ins->c64.toFilter);
            ImGui::Checkbox("Initialize filter",&ins->c64.initFilter);
            
            ImGui::SliderScalar("Cutoff",ImGuiDataType_U8,&ins->c64.cut,&_ZERO,&_ONE_HUNDRED);
            ImGui::SliderScalar("Resonance",ImGuiDataType_U8,&ins->c64.res,&_ZERO,&_FIFTEEN);

            ImGui::Text("Filter Mode");
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(ins->c64.lp)?0.6f:0.2f,0.2f,1.0f));
            if (ImGui::Button("low")) {
              ins->c64.lp=!ins->c64.lp;
            }
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(ins->c64.bp)?0.6f:0.2f,0.2f,1.0f));
            if (ImGui::Button("band")) {
              ins->c64.bp=!ins->c64.bp;
            }
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(ins->c64.hp)?0.6f:0.2f,0.2f,1.0f));
            if (ImGui::Button("high")) {
              ins->c64.hp=!ins->c64.hp;
            }
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(ins->c64.ch3off)?0.6f:0.2f,0.2f,1.0f));
            if (ImGui::Button("ch3off")) {
              ins->c64.ch3off=!ins->c64.ch3off;
            }
            ImGui::PopStyleColor();

            ImGui::Checkbox("Volume Macro is Cutoff Macro",&ins->c64.volIsCutoff);
          }

          // volume macro
          if (e->song.system!=DIV_SYSTEM_GB) {
            ImGui::Separator();
            if ((e->song.system==DIV_SYSTEM_C64_6581 || e->song.system==DIV_SYSTEM_C64_8580) && ins->c64.volIsCutoff) {
              ImGui::Text("Relative Cutoff Macro");
            } else {
              ImGui::Text("Volume Macro");
            }
            for (int i=0; i<ins->std.volMacroLen; i++) {
              if ((e->song.system==DIV_SYSTEM_C64_6581 || e->song.system==DIV_SYSTEM_C64_8580) && ins->c64.volIsCutoff) {
                asFloat[i]=ins->std.volMacro[i]-18;
              } else {
                asFloat[i]=ins->std.volMacro[i];
              }
              loopIndicator[i]=(ins->std.volMacroLoop!=-1 && i>=ins->std.volMacroLoop);
            }
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(0.0f,0.0f));
            int volMax=e->getMaxVolume();
            if (e->song.system==DIV_SYSTEM_C64_6581 || e->song.system==DIV_SYSTEM_C64_8580) {
              if (ins->c64.volIsCutoff) volMax=36;
            }
            ImGui::PlotHistogram("##IVolMacro",asFloat,ins->std.volMacroLen,0,NULL,0,volMax,ImVec2(400.0f*dpiScale,200.0f*dpiScale));
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
              macroDragStart=ImGui::GetItemRectMin();
              macroDragAreaSize=ImVec2(400.0f*dpiScale,200.0f*dpiScale);
              macroDragMin=0;
              macroDragMax=volMax;
              macroDragLen=ins->std.volMacroLen;
              macroDragActive=true;
              macroDragTarget=ins->std.volMacro;
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
          }

          // arp macro
          ImGui::Separator();
          ImGui::Text("Arpeggio Macro");
          bool arpMode=ins->std.arpMacroMode;
          for (int i=0; i<ins->std.arpMacroLen; i++) {
            asFloat[i]=arpMode?ins->std.arpMacro[i]:(ins->std.arpMacro[i]-12);
            loopIndicator[i]=(ins->std.arpMacroLoop!=-1 && i>=ins->std.arpMacroLoop);
          }
          ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(0.0f,0.0f));
          ImGui::PlotHistogram("##IArpMacro",asFloat,ins->std.arpMacroLen,0,NULL,arpMode?arpMacroScroll:(arpMacroScroll-12),arpMacroScroll+(arpMode?24:12),ImVec2(400.0f*dpiScale,200.0f*dpiScale));
          if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            macroDragStart=ImGui::GetItemRectMin();
            macroDragAreaSize=ImVec2(400.0f*dpiScale,200.0f*dpiScale);
            macroDragMin=arpMacroScroll;
            macroDragMax=arpMacroScroll+24;
            macroDragLen=ins->std.arpMacroLen;
            macroDragActive=true;
            macroDragTarget=ins->std.arpMacro;
          }
          ImGui::SameLine();
          ImGui::VSliderInt("##IArpMacroPos",ImVec2(20.0f*dpiScale,200.0f*dpiScale),&arpMacroScroll,arpMode?0:-80,70);
          ImGui::PlotHistogram("##IArpMacroLoop",loopIndicator,ins->std.arpMacroLen,0,NULL,0,1,ImVec2(400.0f*dpiScale,16.0f*dpiScale));
          if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            macroLoopDragStart=ImGui::GetItemRectMin();
            macroLoopDragAreaSize=ImVec2(400.0f*dpiScale,16.0f*dpiScale);
            macroLoopDragLen=ins->std.arpMacroLen;
            macroLoopDragTarget=&ins->std.arpMacroLoop;
            macroLoopDragActive=true;
          }
          ImGui::PopStyleVar();
          if (ImGui::InputScalar("Length##IArpMacroL",ImGuiDataType_U8,&ins->std.arpMacroLen,&_ONE,&_THREE)) {
            if (ins->std.arpMacroLen>127) ins->std.arpMacroLen=127;
          }
          if (ImGui::Checkbox("Absolute",&arpMode)) {
            ins->std.arpMacroMode=arpMode;
            if (arpMode) {
              if (arpMacroScroll<0) arpMacroScroll=0;
            }
          }

          // duty macro
          ImGui::Separator();
          if (e->song.system==DIV_SYSTEM_C64_6581 || e->song.system==DIV_SYSTEM_C64_8580) {
            ImGui::Text("Relative Duty Macro");
          } else if (e->song.system==DIV_SYSTEM_YM2610 || e->song.system==DIV_SYSTEM_YM2610_EXT) {
            ImGui::Text("Noise Frequency Macro");
          } else {
            ImGui::Text("Duty/Noise Mode Macro");
          }
          for (int i=0; i<ins->std.dutyMacroLen; i++) {
            asFloat[i]=ins->std.dutyMacro[i];
            loopIndicator[i]=(ins->std.dutyMacroLoop!=-1 && i>=ins->std.dutyMacroLoop);
          }
          ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(0.0f,0.0f));
          int dutyMax=e->getMaxDuty();;
          ImGui::PlotHistogram("##IDutyMacro",asFloat,ins->std.dutyMacroLen,0,NULL,0,dutyMax,ImVec2(400.0f*dpiScale,200.0f*dpiScale));
          if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            macroDragStart=ImGui::GetItemRectMin();
            macroDragAreaSize=ImVec2(400.0f*dpiScale,200.0f*dpiScale);
            macroDragMin=0;
            macroDragMax=dutyMax;
            macroDragLen=ins->std.dutyMacroLen;
            macroDragActive=true;
            macroDragTarget=ins->std.dutyMacro;
          }
          ImGui::PlotHistogram("##IDutyMacroLoop",loopIndicator,ins->std.dutyMacroLen,0,NULL,0,1,ImVec2(400.0f*dpiScale,16.0f*dpiScale));
          if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            macroLoopDragStart=ImGui::GetItemRectMin();
            macroLoopDragAreaSize=ImVec2(400.0f*dpiScale,16.0f*dpiScale);
            macroLoopDragLen=ins->std.dutyMacroLen;
            macroLoopDragTarget=&ins->std.dutyMacroLoop;
            macroLoopDragActive=true;
          }
          ImGui::PopStyleVar();
          if (ImGui::InputScalar("Length##IDutyMacroL",ImGuiDataType_U8,&ins->std.dutyMacroLen,&_ONE,&_THREE)) {
            if (ins->std.dutyMacroLen>127) ins->std.dutyMacroLen=127;
          }

          // wave macro
          ImGui::Separator();
          ImGui::Text("Waveform Macro");
          for (int i=0; i<ins->std.waveMacroLen; i++) {
            asFloat[i]=ins->std.waveMacro[i];
            loopIndicator[i]=(ins->std.waveMacroLoop!=-1 && i>=ins->std.waveMacroLoop);
          }
          ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(0.0f,0.0f));
          int waveMax=e->getMaxWave();
          ImGui::PlotHistogram("##IWaveMacro",asFloat,ins->std.waveMacroLen,0,NULL,0,waveMax,ImVec2(400.0f*dpiScale,200.0f*dpiScale));
          if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            macroDragStart=ImGui::GetItemRectMin();
            macroDragAreaSize=ImVec2(400.0f*dpiScale,200.0f*dpiScale);
            macroDragMin=0;
            macroDragMax=waveMax;
            macroDragLen=ins->std.waveMacroLen;
            macroDragActive=true;
            macroDragTarget=ins->std.waveMacro;
          }
          ImGui::PlotHistogram("##IWaveMacroLoop",loopIndicator,ins->std.waveMacroLen,0,NULL,0,1,ImVec2(400.0f*dpiScale,16.0f*dpiScale));
          if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            macroLoopDragStart=ImGui::GetItemRectMin();
            macroLoopDragAreaSize=ImVec2(400.0f*dpiScale,16.0f*dpiScale);
            macroLoopDragLen=ins->std.waveMacroLen;
            macroLoopDragTarget=&ins->std.waveMacroLoop;
            macroLoopDragActive=true;
          }
          ImGui::PopStyleVar();
          if (ImGui::InputScalar("Length##IWaveMacroL",ImGuiDataType_U8,&ins->std.waveMacroLen,&_ONE,&_THREE)) {
            if (ins->std.waveMacroLen>127) ins->std.waveMacroLen=127;
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
  if (dpiScale!=1) SDL_SetWindowSize(sdlWin,scrW*dpiScale,scrH*dpiScale);

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
  arpMacroScroll(0),
  macroDragStart(0,0),
  macroDragAreaSize(0,0),
  macroDragTarget(NULL),
  macroDragLen(0),
  macroDragMin(0),
  macroDragMax(0),
  macroDragActive(false) {
}
