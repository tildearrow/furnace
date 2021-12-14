#include "gui.h"
#include "SDL_events.h"
#include "SDL_render.h"
#include "SDL_video.h"
#include "fonts.h"
#include "../ta-log.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"
#include <cstdio>
#include <fmt/printf.h>
#include <stdexcept>

const int _ZERO=0;
const int _ONE=1;
const int _THREE=3;
const int _SEVEN=7;
const int _FIFTEEN=15;
const int _THIRTY_ONE=31;
const int _SIXTY_FOUR=64;
const int _ONE_HUNDRED=100;
const int _ONE_HUNDRED_TWENTY_SEVEN=127;

const FurnaceGUIColors fxColors[16]={
  GUI_COLOR_PATTERN_EFFECT_MISC, // 00
  GUI_COLOR_PATTERN_EFFECT_PITCH, // 01
  GUI_COLOR_PATTERN_EFFECT_PITCH, // 02
  GUI_COLOR_PATTERN_EFFECT_PITCH, // 03
  GUI_COLOR_PATTERN_EFFECT_PITCH, // 04
  GUI_COLOR_PATTERN_EFFECT_VOLUME, // 05
  GUI_COLOR_PATTERN_EFFECT_VOLUME, // 06
  GUI_COLOR_PATTERN_EFFECT_VOLUME, // 07
  GUI_COLOR_PATTERN_EFFECT_PANNING, // 08
  GUI_COLOR_PATTERN_EFFECT_SPEED, // 09
  GUI_COLOR_PATTERN_EFFECT_VOLUME, // 0A
  GUI_COLOR_PATTERN_EFFECT_SONG, // 0B
  GUI_COLOR_PATTERN_EFFECT_TIME, // 0C
  GUI_COLOR_PATTERN_EFFECT_SONG, // 0D
  GUI_COLOR_PATTERN_EFFECT_INVALID, // 0E
  GUI_COLOR_PATTERN_EFFECT_SPEED, // 0F
};

const FurnaceGUIColors extFxColors[16]={
  GUI_COLOR_PATTERN_EFFECT_MISC, // E0
  GUI_COLOR_PATTERN_EFFECT_PITCH, // E1
  GUI_COLOR_PATTERN_EFFECT_PITCH, // E2
  GUI_COLOR_PATTERN_EFFECT_MISC, // E3
  GUI_COLOR_PATTERN_EFFECT_MISC, // E4
  GUI_COLOR_PATTERN_EFFECT_PITCH, // E5
  GUI_COLOR_PATTERN_EFFECT_INVALID, // E6
  GUI_COLOR_PATTERN_EFFECT_INVALID, // E7
  GUI_COLOR_PATTERN_EFFECT_INVALID, // E8
  GUI_COLOR_PATTERN_EFFECT_INVALID, // E9
  GUI_COLOR_PATTERN_EFFECT_MISC, // EA
  GUI_COLOR_PATTERN_EFFECT_MISC, // EB
  GUI_COLOR_PATTERN_EFFECT_TIME, // EC
  GUI_COLOR_PATTERN_EFFECT_TIME, // ED
  GUI_COLOR_PATTERN_EFFECT_SONG, // EE
  GUI_COLOR_PATTERN_EFFECT_SONG, // EF
};

const int opOrder[4]={
  0, 2, 1, 3
};

const char* noteNames[120]={
  "C-0", "C#0", "D-0", "D#0", "E-0", "F-0", "F#0", "G-0", "G#0", "A-0", "A#0", "B-0",
  "C-1", "C#1", "D-1", "D#1", "E-1", "F-1", "F#1", "G-1", "G#1", "A-1", "A#1", "B-1",
  "C-2", "C#2", "D-2", "D#2", "E-2", "F-2", "F#2", "G-2", "G#2", "A-2", "A#2", "B-2",
  "C-3", "C#3", "D-3", "D#3", "E-3", "F-3", "F#3", "G-3", "G#3", "A-3", "A#3", "B-3",
  "C-4", "C#4", "D-4", "D#4", "E-4", "F-4", "F#4", "G-4", "G#4", "A-4", "A#4", "B-4",
  "C-5", "C#5", "D-5", "D#5", "E-5", "F-5", "F#5", "G-5", "G#5", "A-5", "A#5", "B-5",
  "C-6", "C#6", "D-6", "D#6", "E-6", "F-6", "F#6", "G-6", "G#6", "A-6", "A#6", "B-6",
  "C-7", "C#7", "D-7", "D#7", "E-7", "F-7", "F#7", "G-7", "G#7", "A-7", "A#7", "B-7",
  "C-8", "C#8", "D-8", "D#8", "E-8", "F-8", "F#8", "G-8", "G#8", "A-8", "A#8", "B-8",
  "C-9", "C#9", "D-9", "D#9", "E-9", "F-9", "F#9", "G-9", "G#9", "A-9", "A#9", "B-9"
};

void FurnaceGUI::bindEngine(DivEngine* eng) {
  e=eng;
}

const char* FurnaceGUI::noteName(short note, short octave) {
  if (note==100) {
    return "OFF";
  } else if (octave==0 && note==0) {
    return "...";
  }
  int seek=note+octave*12;
  if (seek>=120) return "???";
  return noteNames[seek];
}

void FurnaceGUI::updateScroll(int amount) {
  float lineHeight=(ImGui::GetTextLineHeight()+2*dpiScale);
  nextScroll=lineHeight*amount;
}

void FurnaceGUI::drawSongInfo() {
  if (!songInfoOpen) return;
  if (ImGui::Begin("Song Information",&songInfoOpen)) {
    ImGui::InputText("Name",&e->song.name);
    ImGui::InputText("Author",&e->song.author);
    ImGui::InputScalar("Speed 1",ImGuiDataType_U8,&e->song.speed1,&_ONE,&_THREE);
    ImGui::InputScalar("Speed 2",ImGuiDataType_U8,&e->song.speed2,&_ONE,&_THREE);
    ImGui::InputScalar("Highlight 1",ImGuiDataType_U8,&e->song.hilightA,&_ONE,&_THREE);
    ImGui::InputScalar("Highlight 2",ImGuiDataType_U8,&e->song.hilightB,&_ONE,&_THREE);
    unsigned char ord=e->getOrder();
    if (ImGui::InputScalar("Order",ImGuiDataType_U8,&ord)) {
      e->setOrder(ord);
    }
    if (ImGui::Button("Play")) {
      e->play();
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop")) {
      e->stop();
    }
  }
  if (ImGui::IsWindowFocused()) curWindow=GUI_WINDOW_SONG_INFO;
  ImGui::End();
}

void FurnaceGUI::drawOrders() {
  char selID[16];
  if (!ordersOpen) return;
  if (ImGui::Begin("Orders",&ordersOpen)) {
    if (ImGui::BeginTable("OrdersTable",1+e->getChannelCount(e->song.system))) {
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
  }
  if (ImGui::IsWindowFocused()) curWindow=GUI_WINDOW_ORDERS;
  ImGui::End();
}

void FurnaceGUI::drawInsList() {
  if (!insListOpen) return;
  if (ImGui::Begin("Instruments",&insListOpen)) {
    for (int i=0; i<(int)e->song.ins.size(); i++) {
      DivInstrument* ins=e->song.ins[i];
      if (ImGui::Selectable(fmt::sprintf("%d: %s##_INS%d\n",i,ins->name,i).c_str(),curIns==i)) {
        curIns=i;
      }
    }
  }
  if (ImGui::IsWindowFocused()) curWindow=GUI_WINDOW_INS_LIST;
  ImGui::End();
}

void FurnaceGUI::drawInsEdit() {
  if (!insEditOpen) return;
  if (ImGui::Begin("Instrument Editor",&insEditOpen,ImGuiWindowFlags_NoDocking)) {
    if (curIns>=(int)e->song.ins.size()) {
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
        int dutyMax=e->getMaxDuty();
        if (dutyMax>0) {
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
        }

        // wave macro
        int waveMax=e->getMaxWave();
        if (waveMax>0) {
          ImGui::Separator();
          ImGui::Text("Waveform Macro");
          for (int i=0; i<ins->std.waveMacroLen; i++) {
            asFloat[i]=ins->std.waveMacro[i];
            loopIndicator[i]=(ins->std.waveMacroLoop!=-1 && i>=ins->std.waveMacroLoop);
          }
          ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(0.0f,0.0f));
          
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
  }
  if (ImGui::IsWindowFocused()) curWindow=GUI_WINDOW_INS_EDIT;
  ImGui::End();
}

void FurnaceGUI::drawPattern() {
  if (!patternOpen) return;
  SelectionPoint sel1=selStart;
  SelectionPoint sel2=selEnd;
  if (sel2.y<sel1.y) {
    sel2.y^=sel1.y;
    sel1.y^=sel2.y;
    sel2.y^=sel1.y;
  }
  if (sel2.xCoarse<sel1.xCoarse) {
    sel2.xCoarse^=sel1.xCoarse;
    sel1.xCoarse^=sel2.xCoarse;
    sel2.xCoarse^=sel1.xCoarse;

    sel2.xFine^=sel1.xFine;
    sel1.xFine^=sel2.xFine;
    sel2.xFine^=sel1.xFine;
  } else if (sel2.xCoarse==sel1.xCoarse && sel2.xFine<sel1.xFine) {
    sel2.xFine^=sel1.xFine;
    sel1.xFine^=sel2.xFine;
    sel2.xFine^=sel1.xFine;
  }
  if (ImGui::Begin("Pattern",&patternOpen)) {
    ImGui::SetWindowSize(ImVec2(scrW*dpiScale,scrH*dpiScale));
    char id[32];
    ImGui::PushFont(patFont);
    unsigned char ord=e->getOrder();
    int chans=e->getChannelCount(e->song.system);
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding,ImVec2(0.0f,0.0f));
    if (ImGui::BeginTable("PatternView",chans+1,ImGuiTableFlags_BordersInnerV|ImGuiTableFlags_ScrollX|ImGuiTableFlags_ScrollY|ImGuiTableFlags_NoPadInnerX)) {
      ImGui::TableSetupColumn("pos",ImGuiTableColumnFlags_WidthFixed);
      int curRow=e->getRow();
      if (e->isPlaying()) updateScroll(curRow);
      if (nextScroll>-0.5f) {
        ImGui::SetScrollY(nextScroll);
        nextScroll=-1.0f;
      }
      ImGui::TableSetupScrollFreeze(1,1);
      for (int i=0; i<chans; i++) {
        ImGui::TableSetupColumn(fmt::sprintf("c%d",i).c_str(),ImGuiTableColumnFlags_WidthFixed);
      }
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      for (int i=0; i<chans; i++) {
        ImGui::TableNextColumn();
        ImGui::Text("%s",e->getChannelName(i));
      }
      float oneCharSize=ImGui::CalcTextSize("A").x;
      float lineHeight=(ImGui::GetTextLineHeight()+2*dpiScale);
      ImVec2 threeChars=ImVec2(oneCharSize*3.0f,lineHeight);
      ImVec2 twoChars=ImVec2(oneCharSize*2.0f,lineHeight);
      //ImVec2 oneChar=ImVec2(oneCharSize,lineHeight);
      int dummyRows=(ImGui::GetWindowSize().y/lineHeight)/2;
      for (int i=0; i<dummyRows-1; i++) {
        ImGui::TableNextRow(0,lineHeight);
        ImGui::TableNextColumn();
      }
      for (int i=0; i<e->song.patLen; i++) {
        bool selectedRow=(i>=sel1.y && i<=sel2.y);
        ImGui::TableNextRow(0,lineHeight);
        if ((lineHeight*(i+dummyRows+1))-ImGui::GetScrollY()<0) {
          continue;
        }
        if ((lineHeight*(i+dummyRows))-ImGui::GetScrollY()>ImGui::GetWindowSize().y) {
          continue;          
        }
        ImGui::TableNextColumn();
        if (e->isPlaying() && oldRow==i) {
          ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,0x40ffffff);
        } else if (e->song.hilightB>0 && !(i%e->song.hilightB)) {
          ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,ImGui::GetColorU32(uiColors[GUI_COLOR_PATTERN_HI_2]));
        } else if (e->song.hilightA>0 && !(i%e->song.hilightA)) {
          ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,ImGui::GetColorU32(uiColors[GUI_COLOR_PATTERN_HI_1]));
        }
        ImGui::TextColored(uiColors[GUI_COLOR_PATTERN_ROW_INDEX],"%3d ",i);
        for (int j=0; j<chans; j++) {
          int chanVolMax=e->getMaxVolumeChan(j);
          DivPattern* pat=e->song.pat[j].getPattern(e->song.orders.ord[j][ord],true);
          ImGui::TableNextColumn();

          int sel1XSum=sel1.xCoarse*32+sel1.xFine;
          int sel2XSum=sel2.xCoarse*32+sel2.xFine;
          int j32=j*32;
          bool selectedNote=selectedRow && (j32>=sel1XSum && j32<=sel2XSum);
          bool selectedIns=selectedRow && (j32+1>=sel1XSum && j32+1<=sel2XSum);
          bool selectedVol=selectedRow && (j32+2>=sel1XSum && j32+2<=sel2XSum);


          sprintf(id,"%s##PN_%d_%d",noteName(pat->data[i][0],pat->data[i][1]),i,j);
          if (pat->data[i][0]==0 && pat->data[i][1]==0) {
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_INACTIVE]);
          } else {
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_ACTIVE]);
          }
          ImGui::Selectable(id,selectedNote,ImGuiSelectableFlags_NoPadWithHalfSpacing,threeChars);
          if (ImGui::IsItemClicked()) {
            startSelection(j,0,i);
          }
          if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
            updateSelection(j,0,i);
          }
          ImGui::PopStyleColor();

          if (pat->data[i][2]==-1) {
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_INACTIVE]);
            sprintf(id,"..##PI_%d_%d",i,j);
          } else {
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_INS]);
            sprintf(id,"%.2X##PI_%d_%d",pat->data[i][2],i,j);
          }
          ImGui::SameLine(0.0f,0.0f);
          ImGui::Selectable(id,selectedIns,ImGuiSelectableFlags_NoPadWithHalfSpacing,twoChars);
          if (ImGui::IsItemClicked()) {
            startSelection(j,1,i);
          }
          if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
            updateSelection(j,1,i);
          }
          ImGui::PopStyleColor();

          if (pat->data[i][3]==-1) {
            sprintf(id,"..##PV_%d_%d",i,j);
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_INACTIVE]);
          } else {
            int volColor=(pat->data[i][3]*127)/chanVolMax;
            if (volColor>127) volColor=127;
            sprintf(id,"%.2X##PV_%d_%d",pat->data[i][3],i,j);
            ImGui::PushStyleColor(ImGuiCol_Text,volColors[volColor]);
          }
          ImGui::SameLine(0.0f,0.0f);
          ImGui::Selectable(id,selectedVol,ImGuiSelectableFlags_NoPadWithHalfSpacing,twoChars);
          if (ImGui::IsItemClicked()) {
            startSelection(j,2,i);
          }
          if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
            updateSelection(j,2,i);
          }
          ImGui::PopStyleColor();

          for (int k=0; k<e->song.pat[j].effectRows; k++) {
            int index=4+(k<<1);
            bool selectedEffect=selectedRow && (j32+index-1>=sel1XSum && j32+index-1<=sel2XSum);
            bool selectedEffectVal=selectedRow && (j32+index>=sel1XSum && j32+index<=sel2XSum);
            if (pat->data[i][index]==-1) {
              sprintf(id,"..##PE%d_%d_%d",k,i,j);
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_INACTIVE]);
            } else {
              sprintf(id,"%.2X##PE%d_%d_%d",pat->data[i][index],k,i,j);
              if (pat->data[i][index]<0x10) {
                ImGui::PushStyleColor(ImGuiCol_Text,uiColors[fxColors[pat->data[i][index]]]);
              } else if (pat->data[i][index]<0x20) {
                ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_EFFECT_SYS_PRIMARY]);
              } else if (pat->data[i][index]<0x30) {
                ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_EFFECT_SYS_SECONDARY]);
              } else if (pat->data[i][index]<0xe0) {
                ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_EFFECT_INVALID]);
              } else if (pat->data[i][index]<0xf0) {
                ImGui::PushStyleColor(ImGuiCol_Text,uiColors[extFxColors[pat->data[i][index]-0xe0]]);
              } else {
                ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_EFFECT_INVALID]);
              }
            }
            ImGui::SameLine(0.0f,0.0f);
            ImGui::Selectable(id,selectedEffect,ImGuiSelectableFlags_NoPadWithHalfSpacing,twoChars);
            if (ImGui::IsItemClicked()) {
              startSelection(j,index-1,i);
            }
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
              updateSelection(j,index-1,i);
            }
            if (pat->data[i][index+1]==-1) {
              sprintf(id,"..##PF%d_%d_%d",k,i,j);
            } else {
              sprintf(id,"%.2X##PF%d_%d_%d",pat->data[i][index+1],k,i,j);
            }
            ImGui::SameLine(0.0f,0.0f);
            ImGui::Selectable(id,selectedEffectVal,ImGuiSelectableFlags_NoPadWithHalfSpacing,twoChars);
            if (ImGui::IsItemClicked()) {
              startSelection(j,index,i);
            }
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
              updateSelection(j,index,i);
            }
            ImGui::PopStyleColor();
          }
        }
      }
      for (int i=0; i<=dummyRows; i++) {
        ImGui::TableNextRow(0,lineHeight);
        ImGui::TableNextColumn();
      }
      oldRow=curRow;
      ImGui::EndTable();
    }
    ImGui::PopStyleVar();
    ImGui::PopFont();
  }
  if (ImGui::IsWindowFocused()) {
    curWindow=GUI_WINDOW_PATTERN;
  } else {
    // TODO: what?!
    curWindow=GUI_WINDOW_PATTERN;
  }
  ImGui::End();
}

void FurnaceGUI::startSelection(int xCoarse, int xFine, int y) {
  selStart.xCoarse=xCoarse;
  selStart.xFine=xFine;
  selStart.y=y;
  selEnd.xCoarse=xCoarse;
  selEnd.xFine=xFine;
  selEnd.y=y;
  selecting=true;
}

void FurnaceGUI::updateSelection(int xCoarse, int xFine, int y) {
  if (!selecting) return;
  selEnd.xCoarse=xCoarse;
  selEnd.xFine=xFine;
  selEnd.y=y;
}

void FurnaceGUI::finishSelection() {
  // swap points if needed
  if (selEnd.y<selStart.y) {
    selEnd.y^=selStart.y;
    selStart.y^=selEnd.y;
    selEnd.y^=selStart.y;
  }
  if (selEnd.xCoarse<selStart.xCoarse) {
    selEnd.xCoarse^=selStart.xCoarse;
    selStart.xCoarse^=selEnd.xCoarse;
    selEnd.xCoarse^=selStart.xCoarse;

    selEnd.xFine^=selStart.xFine;
    selStart.xFine^=selEnd.xFine;
    selEnd.xFine^=selStart.xFine;
  } else if (selEnd.xCoarse==selStart.xCoarse && selEnd.xFine<selStart.xFine) {
    selEnd.xFine^=selStart.xFine;
    selStart.xFine^=selEnd.xFine;
    selEnd.xFine^=selStart.xFine;
  }
  selecting=false;
}

void FurnaceGUI::editAdvance() {
  selStart.y+=editStep;
  if (selStart.y>=e->song.patLen) selStart.y=e->song.patLen-1;
  selEnd=selStart;
}

void FurnaceGUI::keyDown(SDL_Event& ev) {
  printf("CUR WINDOW: %d\n",curWindow);
  switch (curWindow) {
    case GUI_WINDOW_PATTERN: {
      if (selStart.xFine==0) { // note
        try {
          int num=12*curOctave+noteKeys.at(ev.key.keysym.sym);
          DivPattern* pat=e->song.pat[selStart.xCoarse].getPattern(e->song.orders.ord[selStart.xCoarse][e->getOrder()],true);

          pat->data[selStart.y][0]=num%12;
          pat->data[selStart.y][1]=num/12;
          editAdvance();
        } catch (std::out_of_range& e) {
        }
      } else { // value
        try {
          int num=valueKeys.at(ev.key.keysym.sym);
          DivPattern* pat=e->song.pat[selStart.xCoarse].getPattern(e->song.orders.ord[selStart.xCoarse][e->getOrder()],true);
          if (pat->data[selStart.y][selStart.xFine+1]==-1) pat->data[selStart.y][selStart.xFine+1]=0;
          pat->data[selStart.y][selStart.xFine+1]=((pat->data[selStart.y][selStart.xFine+1]<<4)|num)&0xff;
          curNibble=!curNibble;
          if (!curNibble) editAdvance();
        } catch (std::out_of_range& e) {
        }
      }
      break;
    }
    default:
      break;
  }
}

void FurnaceGUI::keyUp(SDL_Event& ev) {
  
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
          if (selecting) finishSelection();
          break;
        case SDL_WINDOWEVENT:
          switch (ev.window.event) {
            case SDL_WINDOWEVENT_RESIZED:
              scrW=ev.window.data1/dpiScale;
              scrH=ev.window.data2/dpiScale;
              break;
          }
          break;
        case SDL_KEYDOWN:
          if (!ImGui::GetIO().WantCaptureKeyboard) {
            keyDown(ev);
          }
          break;
        case SDL_KEYUP:
          if (!ImGui::GetIO().WantCaptureKeyboard) {
            keyUp(ev);
          }
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

    curWindow=GUI_WINDOW_NOTHING;

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
    if (ImGui::BeginMenu("window")) {
      if (ImGui::MenuItem("song information")) songInfoOpen=!songInfoOpen;
      if (ImGui::MenuItem("instruments")) insListOpen=!insListOpen;
      if (ImGui::MenuItem("instrument editor")) insEditOpen=!insEditOpen;
      if (ImGui::MenuItem("orders")) ordersOpen=!ordersOpen;
      if (ImGui::MenuItem("pattern")) patternOpen=!patternOpen;
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();

    ImGui::DockSpaceOverViewport();

    drawSongInfo();
    drawOrders();
    drawInsList();
    drawInsEdit();
    drawPattern();

    SDL_SetRenderDrawColor(sdlRend,uiColors[GUI_COLOR_BACKGROUND].x*255,
                                   uiColors[GUI_COLOR_BACKGROUND].y*255,
                                   uiColors[GUI_COLOR_BACKGROUND].z*255,
                                   uiColors[GUI_COLOR_BACKGROUND].w*255);
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
  ImGuiStyle& sty=ImGui::GetStyle();

  sty.Colors[ImGuiCol_WindowBg]=uiColors[GUI_COLOR_FRAME_BACKGROUND];

  ImGui_ImplSDL2_InitForSDLRenderer(sdlWin);
  ImGui_ImplSDLRenderer_Init(sdlRend);

  sty.ScaleAllSizes(dpiScale);

  ImGui::GetIO().ConfigFlags|=ImGuiConfigFlags_DockingEnable;

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
  oldRow(0),
  editStep(1),
  ordersOpen(true),
  insListOpen(true),
  songInfoOpen(true),
  patternOpen(true),
  insEditOpen(false),
  selecting(false),
  curNibble(false),
  curWindow(GUI_WINDOW_NOTHING),
  arpMacroScroll(0),
  macroDragStart(0,0),
  macroDragAreaSize(0,0),
  macroDragTarget(NULL),
  macroDragLen(0),
  macroDragMin(0),
  macroDragMax(0),
  macroDragActive(false),
  nextScroll(-1.0f) {
  uiColors[GUI_COLOR_BACKGROUND]=ImVec4(0.1f,0.1f,0.1f,1.0f);
  uiColors[GUI_COLOR_FRAME_BACKGROUND]=ImVec4(0.0f,0.0f,0.0f,0.85f);
  uiColors[GUI_COLOR_PATTERN_HI_1]=ImVec4(0.6f,0.6f,0.6f,0.2f);
  uiColors[GUI_COLOR_PATTERN_HI_2]=ImVec4(0.5f,0.8f,1.0f,0.2f);
  uiColors[GUI_COLOR_PATTERN_ROW_INDEX]=ImVec4(0.5f,0.8f,1.0f,1.0f);
  uiColors[GUI_COLOR_PATTERN_ACTIVE]=ImVec4(1.0f,1.0f,1.0f,1.0f);
  uiColors[GUI_COLOR_PATTERN_INACTIVE]=ImVec4(0.5f,0.5f,0.5f,1.0f);
  uiColors[GUI_COLOR_PATTERN_INS]=ImVec4(0.4f,0.7f,1.0f,1.0f);
  uiColors[GUI_COLOR_PATTERN_VOLUME_MIN]=ImVec4(0.0f,0.5f,0.0f,1.0f);
  uiColors[GUI_COLOR_PATTERN_VOLUME_HALF]=ImVec4(0.0f,0.75f,0.0f,1.0f);
  uiColors[GUI_COLOR_PATTERN_VOLUME_MAX]=ImVec4(0.0f,1.0f,0.0f,1.0f);
  uiColors[GUI_COLOR_PATTERN_EFFECT_INVALID]=ImVec4(1.0f,0.0f,0.0f,1.0f);
  uiColors[GUI_COLOR_PATTERN_EFFECT_PITCH]=ImVec4(1.0f,1.0f,0.0f,1.0f);
  uiColors[GUI_COLOR_PATTERN_EFFECT_VOLUME]=ImVec4(0.0f,1.0f,0.0f,1.0f);
  uiColors[GUI_COLOR_PATTERN_EFFECT_PANNING]=ImVec4(0.0f,1.0f,1.0f,1.0f);
  uiColors[GUI_COLOR_PATTERN_EFFECT_SONG]=ImVec4(1.0f,0.0f,0.0f,1.0f);
  uiColors[GUI_COLOR_PATTERN_EFFECT_TIME]=ImVec4(0.5f,0.0f,1.0f,1.0f);
  uiColors[GUI_COLOR_PATTERN_EFFECT_SPEED]=ImVec4(1.0f,0.0f,1.0f,1.0f);
  uiColors[GUI_COLOR_PATTERN_EFFECT_SYS_PRIMARY]=ImVec4(0.5f,1.0f,0.0f,1.0f);
  uiColors[GUI_COLOR_PATTERN_EFFECT_SYS_SECONDARY]=ImVec4(0.0f,1.0f,0.5f,1.0f);
  uiColors[GUI_COLOR_PATTERN_EFFECT_MISC]=ImVec4(0.3f,0.3f,1.0f,1.0f);

  for (int i=0; i<63; i++) {
    ImVec4 col1=uiColors[GUI_COLOR_PATTERN_VOLUME_MIN];
    ImVec4 col2=uiColors[GUI_COLOR_PATTERN_VOLUME_HALF];
    ImVec4 col3=uiColors[GUI_COLOR_PATTERN_VOLUME_MAX];
    volColors[i]=ImVec4(col1.x+((col2.x-col1.x)*float(i)/64.0f),
                        col1.y+((col2.y-col1.y)*float(i)/64.0f),
                        col1.z+((col2.z-col1.z)*float(i)/64.0f),
                        1.0f);
    volColors[i+64]=ImVec4(col2.x+((col3.x-col2.x)*float(i)/64.0f),
                           col2.y+((col3.y-col2.y)*float(i)/64.0f),
                           col2.z+((col3.z-col2.z)*float(i)/64.0f),
                           1.0f);
  }

  // octave 1
  noteKeys[SDLK_z]=0;
  noteKeys[SDLK_s]=1;
  noteKeys[SDLK_x]=2;
  noteKeys[SDLK_d]=3;
  noteKeys[SDLK_c]=4;
  noteKeys[SDLK_v]=5;
  noteKeys[SDLK_g]=6;
  noteKeys[SDLK_b]=7;
  noteKeys[SDLK_h]=8;
  noteKeys[SDLK_n]=9;
  noteKeys[SDLK_j]=10;
  noteKeys[SDLK_m]=11;

  // octave 2
  noteKeys[SDLK_q]=12;
  noteKeys[SDLK_2]=13;
  noteKeys[SDLK_w]=14;
  noteKeys[SDLK_3]=15;
  noteKeys[SDLK_e]=16;
  noteKeys[SDLK_r]=17;
  noteKeys[SDLK_5]=18;
  noteKeys[SDLK_t]=19;
  noteKeys[SDLK_6]=20;
  noteKeys[SDLK_y]=21;
  noteKeys[SDLK_7]=22;
  noteKeys[SDLK_u]=23;

  // octave 3
  noteKeys[SDLK_i]=24;
  noteKeys[SDLK_9]=25;
  noteKeys[SDLK_o]=26;
  noteKeys[SDLK_0]=27;
  noteKeys[SDLK_p]=28;
  noteKeys[SDLK_LEFTBRACKET]=29;
  noteKeys[SDLK_RIGHTBRACKET]=31;

  // value keys
  valueKeys[SDLK_0]=0;
  valueKeys[SDLK_1]=1;
  valueKeys[SDLK_2]=2;
  valueKeys[SDLK_3]=3;
  valueKeys[SDLK_4]=4;
  valueKeys[SDLK_5]=5;
  valueKeys[SDLK_6]=6;
  valueKeys[SDLK_7]=7;
  valueKeys[SDLK_8]=8;
  valueKeys[SDLK_9]=9;
  valueKeys[SDLK_a]=10;
  valueKeys[SDLK_b]=11;
  valueKeys[SDLK_c]=12;
  valueKeys[SDLK_d]=13;
  valueKeys[SDLK_e]=14;
  valueKeys[SDLK_f]=15;
}
