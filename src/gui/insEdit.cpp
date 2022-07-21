/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2022 tildearrow and contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "gui.h"
#include "imgui_internal.h"
#include "IconsFontAwesome4.h"
#include "misc/cpp/imgui_stdlib.h"
#include "guiConst.h"
#include "intConst.h"
#include <fmt/printf.h>
#include <imgui.h>
#include "plot_nolerp.h"

const char* ssgEnvTypes[8]={
  "Down Down Down", "Down.", "Down Up Down Up", "Down UP", "Up Up Up", "Up.", "Up Down Up Down", "Up DOWN"
};

const char* fmParamNames[3][32]={
  {"Algorithm", "Feedback", "LFO > Freq", "LFO > Amp", "Attack", "Decay", "Decay 2", "Release", "Sustain", "Level", "EnvScale", "Multiplier", "Detune", "Detune 2", "SSG-EG", "AM", "AM Depth", "Vibrato Depth", "Sustained", "Sustained", "Level Scaling", "Sustain", "Vibrato", "Waveform", "Key Scale Rate", "OP2 Half Sine", "OP1 Half Sine", "EnvShift", "Reverb", "Fine", "LFO2 > Freq", "LFO2 > Amp"},
  {"ALG", "FB", "FMS/PMS", "AMS", "AR", "DR", "SR", "RR", "SL", "TL", "KS", "MULT", "DT", "DT2", "SSG-EG", "AM", "AMD", "FMD", "EGT", "EGT", "KSL", "SUS", "VIB", "WS", "KSR", "DC", "DM", "EGS", "REV", "Fine", "FMS/PMS2", "AMS2"},
  {"ALG", "FB", "FMS/PMS", "AMS", "AR", "DR", "D2R", "RR", "SL", "TL", "RS", "MULT", "DT", "DT2", "SSG-EG", "AM", "DAM", "DVB", "EGT", "EGS", "KSL", "SUS", "VIB", "WS", "KSR", "DC", "DM", "EGS", "REV", "Fine", "FMS/PMS2", "AMS2"}
};

const char* fmParamShortNames[3][32]={
  {"ALG", "FB", "FMS", "AMS", "A", "D", "D2", "R", "S", "TL", "RS", "ML", "DT", "DT2", "SSG", "AM", "DAM", "DVB", "SUS", "SUS", "KSL", "SUS", "VIB", "WS", "KSR", "DC", "DM", "EGS", "REV", "Fine", "FMS2", "AMS2"},
  {"ALG", "FB", "FMS", "AMS", "A", "D", "SR", "R", "S", "TL", "KS", "ML", "DT", "DT2", "SSG", "AM", "AMD", "FMD", "EGT", "EGT", "KSL", "SUS", "VIB", "WS", "KSR", "DC", "DM", "EGS", "REV", "Fine", "FMS2", "AMS2"},
  {"ALG", "FB", "FMS", "AMS", "A", "D", "D2", "R", "S", "TL", "RS", "ML", "DT", "DT2", "SSG", "AM", "DAM", "DVB", "EGT", "EGS", "KSL", "SUS", "VIB", "WS", "KSR", "DC", "DM", "EGS", "REV", "Fine", "FMS2", "AMS2"}
};

const char* opllInsNames[17]={
  "User",
  "Violin",
  "Guitar",
  "Piano",
  "Flute",
  "Clarinet",
  "Oboe",
  "Trumpet",
  "Organ",
  "Horn",
  "Synth",
  "Harpsichord",
  "Vibraphone",
  "Synth Bass",
  "Acoustic Bass",
  "Electric Guitar",
  "Drums"
};

const char* oplWaveforms[8]={
  "Sine", "Half Sine", "Absolute Sine", "Quarter Sine", "Squished Sine", "Squished AbsSine", "Square", "Derived Square"
};

const char* oplWaveformsStandard[8]={
  "Sine", "Half Sine", "Absolute Sine", "Pulse Sine", "Sine (Even Periods)", "AbsSine (Even Periods)", "Square", "Derived Square"
};

const char* opzWaveforms[8]={
  "Sine", "Triangle", "Cut Sine", "Cut Triangle", "Squished Sine", "Squished Triangle", "Squished AbsSine", "Squished AbsTriangle"
};

const char* oplDrumNames[4]={
  "Snare", "Tom", "Top", "HiHat"
};

const bool opIsOutput[8][4]={
  {false,false,false,true},
  {false,false,false,true},
  {false,false,false,true},
  {false,false,false,true},
  {false,true,false,true},
  {false,true,true,true},
  {false,true,true,true},
  {true,true,true,true}
};

const bool opIsOutputOPL[4][4]={
  {false,false,false,true},
  {true,false,false,true},
  {false,true,false,true},
  {true,false,true,true}
};

enum FMParams {
  FM_ALG=0,
  FM_FB=1,
  FM_FMS=2,
  FM_AMS=3,
  FM_AR=4,
  FM_DR=5,
  FM_D2R=6,
  FM_RR=7,
  FM_SL=8,
  FM_TL=9,
  FM_RS=10,
  FM_MULT=11,
  FM_DT=12,
  FM_DT2=13,
  FM_SSG=14,
  FM_AM=15,
  FM_DAM=16,
  FM_DVB=17,
  FM_EGT=18,
  FM_EGS=19,
  FM_KSL=20,
  FM_SUS=21,
  FM_VIB=22,
  FM_WS=23,
  FM_KSR=24,
  FM_DC=25,
  FM_DM=26,
  FM_EGSHIFT=27,
  FM_REV=28,
  FM_FINE=29,
  FM_FMS2=30,
  FM_AMS2=31
};

#define FM_NAME(x) fmParamNames[settings.fmNames][x]
#define FM_SHORT_NAME(x) fmParamShortNames[settings.fmNames][x]

const char* fmOperatorBits[5]={
  "op1", "op2", "op3", "op4", NULL
};

const char* c64ShapeBits[5]={
  "triangle", "saw", "pulse", "noise", NULL
};

const char* ayShapeBits[4]={
  "tone", "noise", "envelope", NULL
};

const char* ayEnvBits[4]={
  "hold", "alternate", "direction", "enable"
};

const char* ssgEnvBits[5]={
  "0", "1", "2", "enabled", NULL
};

const char* saaEnvBits[9]={
  "mirror", "loop", "cut", "direction", "resolution", "fixed", "N/A","enabled", NULL
};

const char* filtModeBits[5]={
  "low", "band", "high", "ch3off", NULL
};

const char* c64SpecialBits[3]={
  "sync", "ring", NULL
};

const char* mikeyFeedbackBits[11] = {
  "0", "1", "2", "3", "4", "5", "7", "10", "11", "int", NULL
};

const char* x1_010EnvBits[8]={
  "enable", "oneshot", "split L/R", "HinvR", "VinvR", "HinvL", "VinvL", NULL
};

const char* n163UpdateBits[8]={
  "now", "every waveform changed", NULL
};

const char* suControlBits[5]={
  "ring mod", "low pass", "high pass", "band pass", NULL
};

const char* panBits[3]={
  "right", "left", NULL
};

const char* oneBit[2]={
  "on", NULL
};

const int orderedOps[4]={
  0, 2, 1, 3
};

const char* singleWSEffects[7]={
  "None",
  "Invert",
  "Add",
  "Subtract",
  "Average",
  "Phase",
  "Chorus"
};

const char* dualWSEffects[9]={
  "None (dual)",
  "Wipe",
  "Fade",
  "Fade (ping-pong)",
  "Overlay",
  "Negative Overlay",
  "Slide",
  "Mix Chorus",
  "Phase Modulation"
};

const char* macroAbsoluteMode="Fixed";
const char* macroRelativeMode="Relative";
const char* macroQSoundMode="QSound";

const char* macroDummyMode="Bug";

String macroHoverNote(int id, float val) {
  if (val<-60 || val>=120) return "???";
  return fmt::sprintf("%d: %s",id,noteNames[(int)val+60]);
}

String macroHover(int id, float val) {
  return fmt::sprintf("%d: %d",id,val);
}

String macroHoverLoop(int id, float val) {
  if (val>1) return "Release";
  if (val>0) return "Loop";
  return "";
}

String macroLFOWaves(int id, float val) {
  switch (((int)val)&3) {
    case 0:
      return "Saw";
    case 1:
      return "Square";
    case 2:
      return "Sine";
    case 3:
      return "Random";
    default:
      return "???";
  }
  return "???";
}

void addAALine(ImDrawList* dl, const ImVec2& p1, const ImVec2& p2, const ImU32 color, float thickness=1.0f) {
  ImVec2 pt[2];
  pt[0]=p1;
  pt[1]=p2;
  dl->AddPolyline(pt,2,color,ImDrawFlags_None,thickness);
}

void FurnaceGUI::drawSSGEnv(unsigned char type, const ImVec2& size) {
  ImDrawList* dl=ImGui::GetWindowDrawList();
  ImGuiWindow* window=ImGui::GetCurrentWindow();

  ImVec2 minArea=window->DC.CursorPos;
  ImVec2 maxArea=ImVec2(
    minArea.x+size.x,
    minArea.y+size.y
  );
  ImRect rect=ImRect(minArea,maxArea);
  ImGuiStyle& style=ImGui::GetStyle();
  ImU32 color=ImGui::GetColorU32(uiColors[GUI_COLOR_FM_SSG]);
  ImGui::ItemSize(size,style.FramePadding.y);
  if (ImGui::ItemAdd(rect,ImGui::GetID("ssgEnvDisplay"))) {
    ImGui::RenderFrame(rect.Min,rect.Max,ImGui::GetColorU32(ImGuiCol_FrameBg),true,style.FrameRounding);
    switch (type) {
      case 0:
        for (int i=0; i<4; i++) {
          ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2((float)i/4.0f,0.2));
          ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2((float)(i+1)/4.0f,0.8));
          addAALine(dl,pos1,pos2,color);
          pos1.x=pos2.x;
          if (i<3) addAALine(dl,pos1,pos2,color);
        }
        break;
      case 1: {
        ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.0,0.2));
        ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(0.25,0.8));
        addAALine(dl,pos1,pos2,color);

        pos1=ImLerp(rect.Min,rect.Max,ImVec2(1.0,0.8));
        addAALine(dl,pos1,pos2,color);
        break;
      }
      case 2: {
        ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.0,0.2));
        ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(0.25,0.8));
        addAALine(dl,pos1,pos2,color);

        pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.2));
        addAALine(dl,pos1,pos2,color);

        pos2=ImLerp(rect.Min,rect.Max,ImVec2(0.75,0.8));
        addAALine(dl,pos1,pos2,color);

        pos1=ImLerp(rect.Min,rect.Max,ImVec2(1.0,0.2));
        addAALine(dl,pos1,pos2,color);
        break;
      }
      case 3: {
        ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.0,0.2));
        ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(0.25,0.8));
        addAALine(dl,pos1,pos2,color);

        pos1.x=pos2.x;
        addAALine(dl,pos1,pos2,color);

        pos2=ImLerp(rect.Min,rect.Max,ImVec2(1.0,0.2));
        addAALine(dl,pos1,pos2,color);
        break;
      }
      case 4:
        for (int i=0; i<4; i++) {
          ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2((float)i/4.0f,0.8));
          ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2((float)(i+1)/4.0f,0.2));
          addAALine(dl,pos1,pos2,color);
          pos1.x=pos2.x;
          if (i<3) addAALine(dl,pos1,pos2,color);
        }
        break;
      case 5: {
        ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.0,0.8));
        ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(0.25,0.2));
        addAALine(dl,pos1,pos2,color);

        pos1=ImLerp(rect.Min,rect.Max,ImVec2(1.0,0.2));
        addAALine(dl,pos1,pos2,color);
        break;
      }
      case 6: {
        ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.0,0.8));
        ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(0.25,0.2));
        addAALine(dl,pos1,pos2,color);

        pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.8));
        addAALine(dl,pos1,pos2,color);

        pos2=ImLerp(rect.Min,rect.Max,ImVec2(0.75,0.2));
        addAALine(dl,pos1,pos2,color);

        pos1=ImLerp(rect.Min,rect.Max,ImVec2(1.0,0.8));
        addAALine(dl,pos1,pos2,color);
        break;
      }
      case 7: {
        ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.0,0.8));
        ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(0.25,0.2));
        addAALine(dl,pos1,pos2,color);

        pos1.x=pos2.x;
        addAALine(dl,pos1,pos2,color);

        pos2=ImLerp(rect.Min,rect.Max,ImVec2(1.0,0.8));
        addAALine(dl,pos1,pos2,color);
        break;
      }
    }
  }
}

void FurnaceGUI::drawWaveform(unsigned char type, bool opz, const ImVec2& size) {
  ImDrawList* dl=ImGui::GetWindowDrawList();
  ImGuiWindow* window=ImGui::GetCurrentWindow();

  ImVec2 waveform[65];
  const size_t waveformLen=64;

  ImVec2 minArea=window->DC.CursorPos;
  ImVec2 maxArea=ImVec2(
    minArea.x+size.x,
    minArea.y+size.y
  );
  ImRect rect=ImRect(minArea,maxArea);
  ImGuiStyle& style=ImGui::GetStyle();
  ImU32 color=ImGui::GetColorU32(uiColors[GUI_COLOR_FM_WAVE]);
  ImGui::ItemSize(size,style.FramePadding.y);
  if (ImGui::ItemAdd(rect,ImGui::GetID("wsDisplay"))) {
    ImGui::RenderFrame(rect.Min,rect.Max,ImGui::GetColorU32(ImGuiCol_FrameBg),true,style.FrameRounding);
    if (opz) {
      switch (type) {
        case 0:
          for (size_t i=0; i<=waveformLen; i++) {
            float x=(float)i/(float)waveformLen;
            float y=sin(x*2.0*M_PI);
            waveform[i]=ImLerp(rect.Min,rect.Max,ImVec2(x,0.5-y*0.4));
          }
          break;
        case 1:
          for (size_t i=0; i<=waveformLen; i++) {
            float x=(float)i/(float)waveformLen;
            float y=pow(sin(x*2.0*M_PI),2.0);
            if (x>=0.5) y=-y;
            waveform[i]=ImLerp(rect.Min,rect.Max,ImVec2(x,0.5-y*0.4));
          }
          break;
        case 2:
          for (size_t i=0; i<=waveformLen; i++) {
            float x=(float)i/(float)waveformLen;
            float y=MAX(0.0,sin(x*2.0*M_PI));
            waveform[i]=ImLerp(rect.Min,rect.Max,ImVec2(x,0.5-y*0.4));
          }
          break;
        case 3:
          for (size_t i=0; i<=waveformLen; i++) {
            float x=(float)i/(float)waveformLen;
            float y=pow(MAX(0.0,sin(x*2.0*M_PI)),2.0);
            waveform[i]=ImLerp(rect.Min,rect.Max,ImVec2(x,0.5-y*0.4));
          }
          break;
        case 4:
          for (size_t i=0; i<=waveformLen; i++) {
            float x=(float)i/(float)waveformLen;
            float y=(x>=0.5)?0.0:sin(x*4.0*M_PI);
            waveform[i]=ImLerp(rect.Min,rect.Max,ImVec2(x,0.5-y*0.4));
          }
          break;
        case 5:
          for (size_t i=0; i<=waveformLen; i++) {
            float x=(float)i/(float)waveformLen;
            float y=(x>=0.5)?0.0:pow(sin(x*4.0*M_PI),2.0);
            if (x>=0.25) y=-y;
            waveform[i]=ImLerp(rect.Min,rect.Max,ImVec2(x,0.5-y*0.4));
          }
          break;
        case 6:
          for (size_t i=0; i<=waveformLen; i++) {
            float x=(float)i/(float)waveformLen;
            float y=(x>=0.5)?0.0:fabs(sin(x*4.0*M_PI));
            waveform[i]=ImLerp(rect.Min,rect.Max,ImVec2(x,0.5-y*0.4));
          }
          break;
        case 7:
          for (size_t i=0; i<=waveformLen; i++) {
            float x=(float)i/(float)waveformLen;
            float y=(x>=0.5)?0.0:pow(sin(x*4.0*M_PI),2.0);
            waveform[i]=ImLerp(rect.Min,rect.Max,ImVec2(x,0.5-y*0.4));
          }
          break;
      }
    } else {
      switch (type) {
        case 0:
          for (size_t i=0; i<=waveformLen; i++) {
            float x=(float)i/(float)waveformLen;
            float y=sin(x*2.0*M_PI);
            waveform[i]=ImLerp(rect.Min,rect.Max,ImVec2(x,0.5-y*0.4));
          }
          break;
        case 1:
          for (size_t i=0; i<=waveformLen; i++) {
            float x=(float)i/(float)waveformLen;
            float y=MAX(0.0,sin(x*2.0*M_PI));
            waveform[i]=ImLerp(rect.Min,rect.Max,ImVec2(x,0.5-y*0.4));
          }
          break;
        case 2:
          for (size_t i=0; i<=waveformLen; i++) {
            float x=(float)i/(float)waveformLen;
            float y=fabs(sin(x*2.0*M_PI));
            waveform[i]=ImLerp(rect.Min,rect.Max,ImVec2(x,0.5-y*0.4));
          }
          break;
        case 3:
          for (size_t i=0; i<=waveformLen; i++) {
            float x=(float)i/(float)waveformLen;
            float y=fabs((tan(x*2.0*M_PI)>=0.0)?sin(x*2.0*M_PI):0.0);
            waveform[i]=ImLerp(rect.Min,rect.Max,ImVec2(x,0.5-y*0.4));
          }
          break;
        case 4:
          for (size_t i=0; i<=waveformLen; i++) {
            float x=(float)i/(float)waveformLen;
            float y=(x>=0.5)?0.0:sin(x*4.0*M_PI);
            waveform[i]=ImLerp(rect.Min,rect.Max,ImVec2(x,0.5-y*0.4));
          }
          break;
        case 5:
          for (size_t i=0; i<=waveformLen; i++) {
            float x=(float)i/(float)waveformLen;
            float y=(x>=0.5)?0.0:fabs(sin(x*4.0*M_PI));
            waveform[i]=ImLerp(rect.Min,rect.Max,ImVec2(x,0.5-y*0.4));
          }
          break;
        case 6:
          for (size_t i=0; i<=waveformLen; i++) {
            float x=(float)i/(float)waveformLen;
            float y=(x>=0.5)?-1.0:1.0;
            waveform[i]=ImLerp(rect.Min,rect.Max,ImVec2(x,0.5-y*0.4));
          }
          break;
        case 7:
          for (size_t i=0; i<=waveformLen; i++) {
            float x=(float)i/(float)waveformLen;
            float y=pow(2.0*(x-0.5),3.0);
            waveform[i]=ImLerp(rect.Min,rect.Max,ImVec2(x,0.5-y*0.4));
          }
          break;
      }
    }
    dl->AddPolyline(waveform,waveformLen+1,color,ImDrawFlags_None,dpiScale);
  }
}

void FurnaceGUI::drawAlgorithm(unsigned char alg, FurnaceGUIFMAlgs algType, const ImVec2& size) {
  ImDrawList* dl=ImGui::GetWindowDrawList();
  ImGuiWindow* window=ImGui::GetCurrentWindow();

  ImVec2 minArea=window->DC.CursorPos;
  ImVec2 maxArea=ImVec2(
    minArea.x+size.x,
    minArea.y+size.y
  );
  ImRect rect=ImRect(minArea,maxArea);
  ImGuiStyle& style=ImGui::GetStyle();
  ImU32 colorM=ImGui::GetColorU32(uiColors[GUI_COLOR_FM_MOD]);
  ImU32 colorC=ImGui::GetColorU32(uiColors[GUI_COLOR_FM_CAR]);
  ImU32 colorL=ImGui::GetColorU32(uiColors[GUI_COLOR_FM_ALG_LINE]);
  ImGui::ItemSize(size,style.FramePadding.y);
  if (ImGui::ItemAdd(rect,ImGui::GetID("alg"))) {
    ImGui::RenderFrame(rect.Min,rect.Max,ImGui::GetColorU32(uiColors[GUI_COLOR_FM_ALG_BG]),true,style.FrameRounding);
    const float circleRadius=6.0f*dpiScale+1.0f;
    switch (algType) {
      case FM_ALGS_4OP:
        switch (alg) {
          case 0: { // 1 > 2 > 3 > 4
            ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.2,0.5));
            ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(0.4,0.5));
            ImVec2 pos3=ImLerp(rect.Min,rect.Max,ImVec2(0.6,0.5));
            ImVec2 pos4=ImLerp(rect.Min,rect.Max,ImVec2(0.8,0.5));
            dl->AddCircleFilled(pos1,4.0f*dpiScale+1.0f,colorM);
            dl->AddCircle(pos1,6.0f*dpiScale+1.0f,colorM);
            addAALine(dl,pos1,pos2,colorL);
            dl->AddCircleFilled(pos2,4.0f*dpiScale+1.0f,colorM);
            addAALine(dl,pos2,pos3,colorL);
            dl->AddCircleFilled(pos3,4.0f*dpiScale+1.0f,colorM);
            addAALine(dl,pos3,pos4,colorL);
            dl->AddCircleFilled(pos4,4.0f*dpiScale+1.0f,colorC);

            pos1.x-=ImGui::CalcTextSize("1").x*0.5;
            pos2.x-=ImGui::CalcTextSize("2").x*0.5;
            pos3.x-=ImGui::CalcTextSize("3").x*0.5;
            pos4.x-=ImGui::CalcTextSize("4").x*0.5;
            pos1.y-=ImGui::CalcTextSize("1").y+circleRadius;
            pos2.y-=ImGui::CalcTextSize("2").y+circleRadius;
            pos3.y-=ImGui::CalcTextSize("3").y+circleRadius;
            pos4.y-=ImGui::CalcTextSize("4").y+circleRadius;
            dl->AddText(pos1,colorM,"1");
            dl->AddText(pos2,colorM,"2");
            dl->AddText(pos3,colorM,"3");
            dl->AddText(pos4,colorC,"4");
            break;
          }
          case 1: { // (1+2) > 3 > 4
            ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.25,0.3));
            ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(0.25,0.7));
            ImVec2 pos3=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.5));
            ImVec2 pos4=ImLerp(rect.Min,rect.Max,ImVec2(0.75,0.5));
            dl->AddCircleFilled(pos1,4.0f*dpiScale+1.0f,colorM);
            dl->AddCircle(pos1,6.0f*dpiScale+1.0f,colorM);
            addAALine(dl,pos1,pos3,colorL);
            dl->AddCircleFilled(pos2,4.0f*dpiScale+1.0f,colorM);
            addAALine(dl,pos2,pos3,colorL);
            dl->AddCircleFilled(pos3,4.0f*dpiScale+1.0f,colorM);
            addAALine(dl,pos3,pos4,colorL);
            dl->AddCircleFilled(pos4,4.0f*dpiScale+1.0f,colorC);

            pos2.x-=ImGui::CalcTextSize("2").x+circleRadius+3.0*dpiScale;
            pos1.x=pos2.x;
            pos3.x-=ImGui::CalcTextSize("3").x*0.5;
            pos4.x-=ImGui::CalcTextSize("4").x*0.5;
            pos1.y-=ImGui::CalcTextSize("1").y*0.5;
            pos2.y-=ImGui::CalcTextSize("2").y*0.5;
            pos3.y-=ImGui::CalcTextSize("3").y+circleRadius;
            pos4.y-=ImGui::CalcTextSize("4").y+circleRadius;
            dl->AddText(pos1,colorM,"1");
            dl->AddText(pos2,colorM,"2");
            dl->AddText(pos3,colorM,"3");
            dl->AddText(pos4,colorC,"4");
            break;
          }
          case 2: { // 1+(2>3) > 4
            ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.3));
            ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(0.25,0.7));
            ImVec2 pos3=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.7));
            ImVec2 pos4=ImLerp(rect.Min,rect.Max,ImVec2(0.75,0.5));
            dl->AddCircleFilled(pos1,4.0f*dpiScale+1.0f,colorM);
            dl->AddCircle(pos1,6.0f*dpiScale+1.0f,colorM);
            addAALine(dl,pos1,pos4,colorL);
            dl->AddCircleFilled(pos2,4.0f*dpiScale+1.0f,colorM);
            addAALine(dl,pos2,pos3,colorL);
            dl->AddCircleFilled(pos3,4.0f*dpiScale+1.0f,colorM);
            addAALine(dl,pos3,pos4,colorL);
            dl->AddCircleFilled(pos4,4.0f*dpiScale+1.0f,colorC);

            pos1.x-=ImGui::CalcTextSize("2").x+circleRadius+3.0*dpiScale;
            pos2.x-=ImGui::CalcTextSize("2").x+circleRadius+3.0*dpiScale;
            pos3.x-=ImGui::CalcTextSize("3").x+circleRadius+3.0*dpiScale;
            pos4.x-=ImGui::CalcTextSize("4").x*0.5;
            pos1.y-=ImGui::CalcTextSize("1").y*0.5;
            pos2.y-=ImGui::CalcTextSize("2").y*0.5;
            pos3.y-=ImGui::CalcTextSize("3").y*0.5;
            pos4.y-=ImGui::CalcTextSize("4").y+circleRadius;
            dl->AddText(pos1,colorM,"1");
            dl->AddText(pos2,colorM,"2");
            dl->AddText(pos3,colorM,"3");
            dl->AddText(pos4,colorC,"4");
            break;
          }
          case 3: { // (1>2)+3 > 4
            ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.25,0.3));
            ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.3));
            ImVec2 pos3=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.7));
            ImVec2 pos4=ImLerp(rect.Min,rect.Max,ImVec2(0.75,0.5));
            dl->AddCircleFilled(pos1,4.0f*dpiScale+1.0f,colorM);
            dl->AddCircle(pos1,6.0f*dpiScale+1.0f,colorM);
            addAALine(dl,pos1,pos2,colorL);
            dl->AddCircleFilled(pos2,4.0f*dpiScale+1.0f,colorM);
            addAALine(dl,pos2,pos4,colorL);
            dl->AddCircleFilled(pos3,4.0f*dpiScale+1.0f,colorM);
            addAALine(dl,pos3,pos4,colorL);
            dl->AddCircleFilled(pos4,4.0f*dpiScale+1.0f,colorC);

            pos1.x-=ImGui::CalcTextSize("2").x+circleRadius+3.0*dpiScale;
            pos2.x-=ImGui::CalcTextSize("2").x+circleRadius+3.0*dpiScale;
            pos3.x-=ImGui::CalcTextSize("3").x+circleRadius+3.0*dpiScale;
            pos4.x-=ImGui::CalcTextSize("4").x*0.5;
            pos1.y-=ImGui::CalcTextSize("1").y*0.5;
            pos2.y-=ImGui::CalcTextSize("2").y*0.5;
            pos3.y-=ImGui::CalcTextSize("3").y*0.5;
            pos4.y-=ImGui::CalcTextSize("4").y+circleRadius;
            dl->AddText(pos1,colorM,"1");
            dl->AddText(pos2,colorM,"2");
            dl->AddText(pos3,colorM,"3");
            dl->AddText(pos4,colorC,"4");
            break;
          }
          case 4: { // (1>2) + (3>4)
            ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.25,0.3));
            ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.3));
            ImVec2 pos3=ImLerp(rect.Min,rect.Max,ImVec2(0.25,0.7));
            ImVec2 pos4=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.7));
            ImVec2 pos5=ImLerp(rect.Min,rect.Max,ImVec2(0.75,0.5));
            dl->AddCircleFilled(pos1,4.0f*dpiScale+1.0f,colorM);
            dl->AddCircle(pos1,6.0f*dpiScale+1.0f,colorM);
            addAALine(dl,pos1,pos2,colorL);
            dl->AddCircleFilled(pos2,4.0f*dpiScale+1.0f,colorC);
            dl->AddCircleFilled(pos3,4.0f*dpiScale+1.0f,colorM);
            addAALine(dl,pos3,pos4,colorL);
            dl->AddCircleFilled(pos4,4.0f*dpiScale+1.0f,colorC);
            addAALine(dl,pos2,pos5,colorL);
            addAALine(dl,pos4,pos5,colorL);

            pos1.x-=ImGui::CalcTextSize("2").x+circleRadius+3.0*dpiScale;
            pos2.x-=ImGui::CalcTextSize("2").x+circleRadius+3.0*dpiScale;
            pos3.x-=ImGui::CalcTextSize("3").x+circleRadius+3.0*dpiScale;
            pos4.x-=ImGui::CalcTextSize("4").x+circleRadius+3.0*dpiScale;
            pos1.y-=ImGui::CalcTextSize("1").y*0.5;
            pos2.y-=ImGui::CalcTextSize("2").y*0.5;
            pos3.y-=ImGui::CalcTextSize("3").y*0.5;
            pos4.y-=ImGui::CalcTextSize("4").y*0.5;
            dl->AddText(pos1,colorM,"1");
            dl->AddText(pos2,colorC,"2");
            dl->AddText(pos3,colorM,"3");
            dl->AddText(pos4,colorC,"4");
            break;
          }
          case 5: { // 1 > (2+3+4)
            ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.25,0.5));
            ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.25));
            ImVec2 pos3=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.5));
            ImVec2 pos4=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.75));
            ImVec2 pos5=ImLerp(rect.Min,rect.Max,ImVec2(0.75,0.5));
            dl->AddCircleFilled(pos1,4.0f*dpiScale+1.0f,colorM);
            dl->AddCircle(pos1,6.0f*dpiScale+1.0f,colorM);
            addAALine(dl,pos1,pos2,colorL);
            addAALine(dl,pos1,pos3,colorL);
            addAALine(dl,pos1,pos4,colorL);
            dl->AddCircleFilled(pos2,4.0f*dpiScale+1.0f,colorC);
            dl->AddCircleFilled(pos3,4.0f*dpiScale+1.0f,colorC);
            dl->AddCircleFilled(pos4,4.0f*dpiScale+1.0f,colorC);
            addAALine(dl,pos2,pos5,colorL);
            addAALine(dl,pos3,pos5,colorL);
            addAALine(dl,pos4,pos5,colorL);

            pos1.x-=ImGui::CalcTextSize("2").x+circleRadius+3.0*dpiScale;
            pos2.x-=ImGui::CalcTextSize("2").x+circleRadius+3.0*dpiScale;
            pos3.x-=ImGui::CalcTextSize("3").x+circleRadius+3.0*dpiScale;
            pos4.x-=ImGui::CalcTextSize("4").x+circleRadius+3.0*dpiScale;
            pos1.y-=ImGui::CalcTextSize("1").y*0.5;
            pos2.y-=ImGui::CalcTextSize("2").y*0.5;
            pos3.y-=ImGui::CalcTextSize("3").y*0.5;
            pos4.y-=ImGui::CalcTextSize("4").y*0.5;
            dl->AddText(pos1,colorM,"1");
            dl->AddText(pos2,colorC,"2");
            dl->AddText(pos3,colorC,"3");
            dl->AddText(pos4,colorC,"4");
            break;
          }
          case 6: { // (1>2) + 3 + 4
            ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.25,0.25));
            ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.25));
            ImVec2 pos3=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.5));
            ImVec2 pos4=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.75));
            ImVec2 pos5=ImLerp(rect.Min,rect.Max,ImVec2(0.75,0.5));
            dl->AddCircleFilled(pos1,4.0f*dpiScale+1.0f,colorM);
            dl->AddCircle(pos1,6.0f*dpiScale+1.0f,colorM);
            addAALine(dl,pos1,pos2,colorL);
            dl->AddCircleFilled(pos2,4.0f*dpiScale+1.0f,colorC);
            dl->AddCircleFilled(pos3,4.0f*dpiScale+1.0f,colorC);
            dl->AddCircleFilled(pos4,4.0f*dpiScale+1.0f,colorC);
            addAALine(dl,pos2,pos5,colorL);
            addAALine(dl,pos3,pos5,colorL);
            addAALine(dl,pos4,pos5,colorL);

            pos1.x-=ImGui::CalcTextSize("2").x+circleRadius+3.0*dpiScale;
            pos2.x-=ImGui::CalcTextSize("2").x+circleRadius+3.0*dpiScale;
            pos3.x-=ImGui::CalcTextSize("3").x+circleRadius+3.0*dpiScale;
            pos4.x-=ImGui::CalcTextSize("4").x+circleRadius+3.0*dpiScale;
            pos1.y-=ImGui::CalcTextSize("1").y*0.5;
            pos2.y-=ImGui::CalcTextSize("2").y*0.5;
            pos3.y-=ImGui::CalcTextSize("3").y*0.5;
            pos4.y-=ImGui::CalcTextSize("4").y*0.5;
            dl->AddText(pos1,colorM,"1");
            dl->AddText(pos2,colorC,"2");
            dl->AddText(pos3,colorC,"3");
            dl->AddText(pos4,colorC,"4");
            break;
          }
          case 7: { // 1 + 2 + 3 + 4
            ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.25,0.2));
            ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(0.35,0.4));
            ImVec2 pos3=ImLerp(rect.Min,rect.Max,ImVec2(0.45,0.6));
            ImVec2 pos4=ImLerp(rect.Min,rect.Max,ImVec2(0.55,0.8));
            ImVec2 pos5=ImLerp(rect.Min,rect.Max,ImVec2(0.75,0.5));
            dl->AddCircleFilled(pos1,4.0f*dpiScale+1.0f,colorC);
            dl->AddCircle(pos1,6.0f*dpiScale+1.0f,colorC);
            dl->AddCircleFilled(pos2,4.0f*dpiScale+1.0f,colorC);
            dl->AddCircleFilled(pos3,4.0f*dpiScale+1.0f,colorC);
            dl->AddCircleFilled(pos4,4.0f*dpiScale+1.0f,colorC);
            addAALine(dl,pos1,pos5,colorL);
            addAALine(dl,pos2,pos5,colorL);
            addAALine(dl,pos3,pos5,colorL);
            addAALine(dl,pos4,pos5,colorL);

            pos1.x-=ImGui::CalcTextSize("2").x+circleRadius+3.0*dpiScale;
            pos2.x-=ImGui::CalcTextSize("2").x+circleRadius+3.0*dpiScale;
            pos3.x-=ImGui::CalcTextSize("3").x+circleRadius+3.0*dpiScale;
            pos4.x-=ImGui::CalcTextSize("4").x+circleRadius+3.0*dpiScale;
            pos1.y-=ImGui::CalcTextSize("1").y*0.5;
            pos2.y-=ImGui::CalcTextSize("2").y*0.5;
            pos3.y-=ImGui::CalcTextSize("3").y*0.5;
            pos4.y-=ImGui::CalcTextSize("4").y*0.5;
            dl->AddText(pos1,colorC,"1");
            dl->AddText(pos2,colorC,"2");
            dl->AddText(pos3,colorC,"3");
            dl->AddText(pos4,colorC,"4");
            break;
          }
        }
        break;
      case FM_ALGS_2OP_OPL:
        switch (alg) {
          case 0: { // 1 > 2
            ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.33,0.5));
            ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(0.67,0.5));
            dl->AddCircleFilled(pos1,4.0f*dpiScale+1.0f,colorM);
            dl->AddCircle(pos1,6.0f*dpiScale+1.0f,colorM);
            addAALine(dl,pos1,pos2,colorL);
            dl->AddCircleFilled(pos2,4.0f*dpiScale+1.0f,colorC);

            pos1.x-=ImGui::CalcTextSize("2").x+circleRadius+3.0*dpiScale;
            pos2.x+=circleRadius+3.0*dpiScale;
            pos1.y-=ImGui::CalcTextSize("1").y*0.5;
            pos2.y-=ImGui::CalcTextSize("2").y*0.5;
            dl->AddText(pos1,colorM,"1");
            dl->AddText(pos2,colorC,"2");
            break;
          }
          case 1: { // 1 + 2
            ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.33,0.5));
            ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(0.67,0.5));
            dl->AddCircleFilled(pos1,4.0f*dpiScale+1.0f,colorC);
            dl->AddCircle(pos1,6.0f*dpiScale+1.0f,colorC);
            dl->AddCircleFilled(pos2,4.0f*dpiScale+1.0f,colorC);

            pos1.x-=ImGui::CalcTextSize("2").x+circleRadius+3.0*dpiScale;
            pos2.x+=circleRadius+3.0*dpiScale;
            pos1.y-=ImGui::CalcTextSize("1").y*0.5;
            pos2.y-=ImGui::CalcTextSize("2").y*0.5;
            dl->AddText(pos1,colorC,"1");
            dl->AddText(pos2,colorC,"2");
            break;
          }
        }
        break;
      case FM_ALGS_4OP_OPL:
        switch (alg) {
          case 0: { // 1 > 2 > 3 > 4
            ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.2,0.5));
            ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(0.4,0.5));
            ImVec2 pos3=ImLerp(rect.Min,rect.Max,ImVec2(0.6,0.5));
            ImVec2 pos4=ImLerp(rect.Min,rect.Max,ImVec2(0.8,0.5));
            dl->AddCircleFilled(pos1,4.0f*dpiScale+1.0f,colorM);
            dl->AddCircle(pos1,6.0f*dpiScale+1.0f,colorM);
            addAALine(dl,pos1,pos2,colorL);
            dl->AddCircleFilled(pos2,4.0f*dpiScale+1.0f,colorM);
            addAALine(dl,pos2,pos3,colorL);
            dl->AddCircleFilled(pos3,4.0f*dpiScale+1.0f,colorM);
            addAALine(dl,pos3,pos4,colorL);
            dl->AddCircleFilled(pos4,4.0f*dpiScale+1.0f,colorC);

            pos1.x-=ImGui::CalcTextSize("1").x*0.5;
            pos2.x-=ImGui::CalcTextSize("2").x*0.5;
            pos3.x-=ImGui::CalcTextSize("3").x*0.5;
            pos4.x-=ImGui::CalcTextSize("4").x*0.5;
            pos1.y-=ImGui::CalcTextSize("1").y+circleRadius;
            pos2.y-=ImGui::CalcTextSize("2").y+circleRadius;
            pos3.y-=ImGui::CalcTextSize("3").y+circleRadius;
            pos4.y-=ImGui::CalcTextSize("4").y+circleRadius;
            dl->AddText(pos1,colorM,"1");
            dl->AddText(pos2,colorM,"2");
            dl->AddText(pos3,colorM,"3");
            dl->AddText(pos4,colorC,"4");
            break;
          }
          case 1: { // 1 + (2 > 3 > 4)
            ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.4,0.3));
            ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(0.2,0.7));
            ImVec2 pos3=ImLerp(rect.Min,rect.Max,ImVec2(0.4,0.7));
            ImVec2 pos4=ImLerp(rect.Min,rect.Max,ImVec2(0.6,0.7));
            ImVec2 pos5=ImLerp(rect.Min,rect.Max,ImVec2(0.8,0.7));
            dl->AddCircleFilled(pos1,4.0f*dpiScale+1.0f,colorC);
            dl->AddCircle(pos1,6.0f*dpiScale+1.0f,colorC);
            addAALine(dl,pos1,pos5,colorL);
            dl->AddCircleFilled(pos2,4.0f*dpiScale+1.0f,colorM);
            addAALine(dl,pos2,pos3,colorL);
            dl->AddCircleFilled(pos3,4.0f*dpiScale+1.0f,colorM);
            addAALine(dl,pos3,pos4,colorL);
            dl->AddCircleFilled(pos4,4.0f*dpiScale+1.0f,colorC);

            addAALine(dl,pos4,pos5,colorL);

            pos1.x-=ImGui::CalcTextSize("2").x+circleRadius+3.0*dpiScale;
            pos2.x-=ImGui::CalcTextSize("2").x+circleRadius+3.0*dpiScale;
            pos3.x-=ImGui::CalcTextSize("3").x+circleRadius+3.0*dpiScale;
            pos4.x-=ImGui::CalcTextSize("4").x*0.5;
            pos1.y-=ImGui::CalcTextSize("1").y*0.5;
            pos2.y-=ImGui::CalcTextSize("2").y*0.5;
            pos3.y-=ImGui::CalcTextSize("3").y*0.5;
            pos4.y-=ImGui::CalcTextSize("4").y+circleRadius;
            dl->AddText(pos1,colorC,"1");
            dl->AddText(pos2,colorM,"2");
            dl->AddText(pos3,colorM,"3");
            dl->AddText(pos4,colorC,"4");
            break;
          }
          case 2: { // (1>2) + (3>4)
            ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.25,0.3));
            ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.3));
            ImVec2 pos3=ImLerp(rect.Min,rect.Max,ImVec2(0.25,0.7));
            ImVec2 pos4=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.7));
            ImVec2 pos5=ImLerp(rect.Min,rect.Max,ImVec2(0.75,0.5));
            dl->AddCircleFilled(pos1,4.0f*dpiScale+1.0f,colorM);
            dl->AddCircle(pos1,6.0f*dpiScale+1.0f,colorM);
            addAALine(dl,pos1,pos2,colorL);
            dl->AddCircleFilled(pos2,4.0f*dpiScale+1.0f,colorC);
            dl->AddCircleFilled(pos3,4.0f*dpiScale+1.0f,colorM);
            addAALine(dl,pos3,pos4,colorL);
            dl->AddCircleFilled(pos4,4.0f*dpiScale+1.0f,colorC);
            addAALine(dl,pos2,pos5,colorL);
            addAALine(dl,pos4,pos5,colorL);

            pos1.x-=ImGui::CalcTextSize("2").x+circleRadius+3.0*dpiScale;
            pos2.x-=ImGui::CalcTextSize("2").x+circleRadius+3.0*dpiScale;
            pos3.x-=ImGui::CalcTextSize("3").x+circleRadius+3.0*dpiScale;
            pos4.x-=ImGui::CalcTextSize("4").x+circleRadius+3.0*dpiScale;
            pos1.y-=ImGui::CalcTextSize("1").y*0.5;
            pos2.y-=ImGui::CalcTextSize("2").y*0.5;
            pos3.y-=ImGui::CalcTextSize("3").y*0.5;
            pos4.y-=ImGui::CalcTextSize("4").y*0.5;
            dl->AddText(pos1,colorM,"1");
            dl->AddText(pos2,colorC,"2");
            dl->AddText(pos3,colorM,"3");
            dl->AddText(pos4,colorC,"4");
            break;
          }
          case 3: { // 1 + (2 > 3) + 4
            ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.25));
            ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(0.25,0.5));
            ImVec2 pos3=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.5));
            ImVec2 pos4=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.75));
            ImVec2 pos5=ImLerp(rect.Min,rect.Max,ImVec2(0.75,0.5));
            dl->AddCircleFilled(pos1,4.0f*dpiScale+1.0f,colorC);
            dl->AddCircle(pos1,6.0f*dpiScale+1.0f,colorC);
            addAALine(dl,pos2,pos3,colorL);
            dl->AddCircleFilled(pos2,4.0f*dpiScale+1.0f,colorM);
            dl->AddCircleFilled(pos3,4.0f*dpiScale+1.0f,colorC);
            dl->AddCircleFilled(pos4,4.0f*dpiScale+1.0f,colorC);
            addAALine(dl,pos1,pos5,colorL);
            addAALine(dl,pos3,pos5,colorL);
            addAALine(dl,pos4,pos5,colorL);

            pos1.x-=ImGui::CalcTextSize("2").x+circleRadius+3.0*dpiScale;
            pos2.x-=ImGui::CalcTextSize("2").x+circleRadius+3.0*dpiScale;
            pos3.x-=ImGui::CalcTextSize("3").x+circleRadius+3.0*dpiScale;
            pos4.x-=ImGui::CalcTextSize("4").x+circleRadius+3.0*dpiScale;
            pos1.y-=ImGui::CalcTextSize("1").y*0.5;
            pos2.y-=ImGui::CalcTextSize("2").y*0.5;
            pos3.y-=ImGui::CalcTextSize("3").y*0.5;
            pos4.y-=ImGui::CalcTextSize("4").y*0.5;
            dl->AddText(pos1,colorC,"1");
            dl->AddText(pos2,colorM,"2");
            dl->AddText(pos3,colorC,"3");
            dl->AddText(pos4,colorC,"4");
            break;
          }
        }
        break;
      default:
        break;
    }
  }
}

void FurnaceGUI::drawFMEnv(unsigned char tl, unsigned char ar, unsigned char dr, unsigned char d2r, unsigned char rr, unsigned char sl, unsigned char sus, unsigned char egt, unsigned char algOrGlobalSus, float maxTl, float maxArDr, const ImVec2& size, unsigned short instType) {
  ImDrawList* dl=ImGui::GetWindowDrawList();
  ImGuiWindow* window=ImGui::GetCurrentWindow();

  ImVec2 minArea=window->DC.CursorPos;
  ImVec2 maxArea=ImVec2(
    minArea.x+size.x,
    minArea.y+size.y
  );
  ImRect rect=ImRect(minArea,maxArea);
  ImGuiStyle& style=ImGui::GetStyle();
  ImU32 color=ImGui::GetColorU32(uiColors[GUI_COLOR_FM_ENVELOPE]);
  ImU32 colorR=ImGui::GetColorU32(uiColors[GUI_COLOR_FM_ENVELOPE_RELEASE]); // Relsease triangle
  ImU32 colorS=ImGui::GetColorU32(uiColors[GUI_COLOR_FM_ENVELOPE_SUS_GUIDE]); // Sustain horiz/vert line color
  ImGui::ItemSize(size,style.FramePadding.y);
  if (ImGui::ItemAdd(rect,ImGui::GetID("fmEnv"))) {
    ImGui::RenderFrame(rect.Min,rect.Max,ImGui::GetColorU32(ImGuiCol_FrameBg),true,style.FrameRounding);

    //Adjust for OPLL global sustain setting
    if (instType==DIV_INS_OPLL && algOrGlobalSus==1.0){
      rr = 5.0;
    }
    //calculate x positions
    float arPos=float(maxArDr-ar)/maxArDr; //peak of AR, start of DR
    float drPos=arPos+((sl/15.0)*(float(maxArDr-dr)/maxArDr)); //end of DR, start of D2R
    float d2rPos=drPos+(((15.0-sl)/15.0)*(float(31.0-d2r)/31.0)); //End of D2R
    float rrPos=(float(15-rr)/15.0); //end of RR

    //shrink all the x positions horizontally
    arPos/=2.0;
    drPos/=2.0;
    d2rPos/=2.0;
    rrPos/=1.0;

    ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.0,1.0)); //the bottom corner
    ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(arPos,(tl/maxTl))); //peak of AR, start of DR
    ImVec2 pos3=ImLerp(rect.Min,rect.Max,ImVec2(drPos,(float)((tl/maxTl)+(sl/15.0)-((tl/maxTl)*(sl/15.0))))); //end of DR, start of D2R
    ImVec2 pos4=ImLerp(rect.Min,rect.Max,ImVec2(d2rPos,1.0)); //end of D2R
    ImVec2 posRStart=ImLerp(rect.Min,rect.Max,ImVec2(0.0,(tl/maxTl))); //release start
    ImVec2 posREnd=ImLerp(rect.Min,rect.Max,ImVec2(rrPos,1.0));//release end
    ImVec2 posSLineHEnd=ImLerp(rect.Min,rect.Max,ImVec2(1.0,(float)((tl/maxTl)+(sl/15.0)-((tl/maxTl)*(sl/15.0))))); //sustain horizontal line end
    ImVec2 posSLineVEnd=ImLerp(rect.Min,rect.Max,ImVec2(drPos,1.0)); //sustain vertical line end
    ImVec2 posDecayRate0Pt=ImLerp(rect.Min,rect.Max,ImVec2(1.0,(tl/maxTl))); //Height of the peak of AR, forever
    ImVec2 posDecay2Rate0Pt=ImLerp(rect.Min,rect.Max,ImVec2(1.0,(float)((tl/maxTl)+(sl/15.0)-((tl/maxTl)*(sl/15.0))))); //Height of the peak of SR, forever

    //dl->Flags=ImDrawListFlags_AntiAliasedLines|ImDrawListFlags_AntiAliasedLinesUseTex;
    if (ar==0.0) { //if AR = 0, the envelope never starts
      dl->AddTriangleFilled(posRStart,posREnd,pos1,colorS); //draw release as shaded triangle behind everything
      addAALine(dl,pos1,pos4,color); //draw line on ground
    } else if (dr==0.0 && sl!=0.0) { //if DR = 0 and SL is not 0, then the envelope stays at max volume forever
      dl->AddTriangleFilled(posRStart,posREnd,pos1,colorS); //draw release as shaded triangle behind everything
      //addAALine(dl,pos3,posSLineHEnd,colorS); //draw horiz line through sustain level
      //addAALine(dl,pos3,posSLineVEnd,colorS); //draw vert. line through sustain level
      addAALine(dl,pos1,pos2,color); //A
      addAALine(dl,pos2,posDecayRate0Pt,color); //Line from A to end of graph
    } else if (d2r==0.0 || (instType==DIV_INS_OPL && sus==1.0) || (instType==DIV_INS_OPLL && egt!=0.0)) { //envelope stays at the sustain level forever
      dl->AddTriangleFilled(posRStart,posREnd,pos1,colorS); //draw release as shaded triangle behind everything
      addAALine(dl,pos3,posSLineHEnd,colorR); //draw horiz line through sustain level
      addAALine(dl,pos3,posSLineVEnd,colorR); //draw vert. line through sustain level
      addAALine(dl,pos1,pos2,color); //A
      addAALine(dl,pos2,pos3,color); //D
      addAALine(dl,pos3,posDecay2Rate0Pt,color); //Line from D to end of graph
    } else { //draw graph normally
      dl->AddTriangleFilled(posRStart,posREnd,pos1,colorS); //draw release as shaded triangle behind everything
      addAALine(dl,pos3,posSLineHEnd,colorR); //draw horiz line through sustain level
      addAALine(dl,pos3,posSLineVEnd,colorR); //draw vert. line through sustain level
      addAALine(dl,pos1,pos2,color); //A
      addAALine(dl,pos2,pos3,color); //D
      addAALine(dl,pos3,pos4,color); //D2
    }
    //dl->Flags^=ImDrawListFlags_AntiAliasedLines|ImDrawListFlags_AntiAliasedLinesUseTex;
  }
}

void FurnaceGUI::drawGBEnv(unsigned char vol, unsigned char len, unsigned char sLen, bool dir, const ImVec2& size) {
  ImDrawList* dl=ImGui::GetWindowDrawList();
  ImGuiWindow* window=ImGui::GetCurrentWindow();

  ImVec2 minArea=window->DC.CursorPos;
  ImVec2 maxArea=ImVec2(
    minArea.x+size.x,
    minArea.y+size.y
  );
  ImRect rect=ImRect(minArea,maxArea);
  ImGuiStyle& style=ImGui::GetStyle();
  ImU32 color=ImGui::GetColorU32(uiColors[GUI_COLOR_FM_ENVELOPE]);
  //ImU32 colorS=ImGui::GetColorU32(uiColors[GUI_COLOR_FM_ENVELOPE_SUS_GUIDE]); // Sustain horiz/vert line color
  ImGui::ItemSize(size,style.FramePadding.y);
  if (ImGui::ItemAdd(rect,ImGui::GetID("gbEnv"))) {
    ImGui::RenderFrame(rect.Min,rect.Max,ImGui::GetColorU32(ImGuiCol_FrameBg),true,style.FrameRounding);
    
    float volY=1.0-((float)vol/15.0);
    float lenPos=(sLen>62)?1.0:((float)sLen/384.0);
    float envEndPoint=((float)len/7.0)*((float)(dir?(15-vol):vol)/15.0);

    ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.0,volY));
    ImVec2 pos2;
    if (dir) {
      if (len>0) {
        if (lenPos<envEndPoint) {
          pos2=ImLerp(rect.Min,rect.Max,ImVec2(lenPos,volY*(1.0-(lenPos/envEndPoint))));
        } else {
          pos2=ImLerp(rect.Min,rect.Max,ImVec2(envEndPoint,0.0));
        }
      } else {
        pos2=ImLerp(rect.Min,rect.Max,ImVec2(lenPos,volY));
      }
    } else {
      if (len>0) {
        if (lenPos<envEndPoint) {
          pos2=ImLerp(rect.Min,rect.Max,ImVec2(lenPos,volY+(1.0-volY)*(lenPos/envEndPoint)));
        } else {
          pos2=ImLerp(rect.Min,rect.Max,ImVec2(envEndPoint,1.0));
        }
      } else {
        pos2=ImLerp(rect.Min,rect.Max,ImVec2(lenPos,volY));
      }
    }
    ImVec2 pos3=ImLerp(rect.Min,rect.Max,ImVec2(lenPos,(len>0 || sLen<63)?((dir && sLen>62)?0.0:1.0):volY));

    addAALine(dl,pos1,pos2,color);
    if (lenPos>=envEndPoint && sLen<63 && dir) {
      pos3=ImLerp(rect.Min,rect.Max,ImVec2(lenPos,0.0));
      addAALine(dl,pos2,pos3,color);
      ImVec2 pos4=ImLerp(rect.Min,rect.Max,ImVec2(lenPos,1.0));
      addAALine(dl,pos3,pos4,color);
    } else {
      addAALine(dl,pos2,pos3,color);
    }
  }
}

#define P(x) if (x) { \
  MARK_MODIFIED; \
  e->notifyInsChange(curIns); \
}

#define PARAMETER MARK_MODIFIED; e->notifyInsChange(curIns);

String genericGuide(float value) {
  return fmt::sprintf("%d",(int)value);
}

void FurnaceGUI::drawMacros(std::vector<FurnaceGUIMacroDesc>& macros) {
  float asFloat[256];
  int asInt[256];
  float loopIndicator[256];
  int index=0;

  float reservedSpace=(settings.oldMacroVSlider)?(20.0f*dpiScale+ImGui::GetStyle().ItemSpacing.x):ImGui::GetStyle().ScrollbarSize;

  if (ImGui::BeginTable("MacroSpace",2)) {
    ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed,0.0);
    ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0);
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    float lenAvail=ImGui::GetContentRegionAvail().x;
    //ImGui::Dummy(ImVec2(120.0f*dpiScale,dpiScale));
    ImGui::SetNextItemWidth(120.0f*dpiScale);
    if (ImGui::InputInt("##MacroPointSize",&macroPointSize,1,16)) {
      if (macroPointSize<1) macroPointSize=1;
      if (macroPointSize>256) macroPointSize=256;
    }
    ImGui::TableNextColumn();
    float availableWidth=ImGui::GetContentRegionAvail().x-reservedSpace;
    int totalFit=MIN(128,availableWidth/MAX(1,macroPointSize*dpiScale));
    if (macroDragScroll>128-totalFit) {
      macroDragScroll=128-totalFit;
    }
    ImGui::SetNextItemWidth(availableWidth);
    if (CWSliderInt("##MacroScroll",&macroDragScroll,0,128-totalFit,"")) {
      if (macroDragScroll<0) macroDragScroll=0;
      if (macroDragScroll>128-totalFit) macroDragScroll=128-totalFit;
    }

    // draw macros
    for (FurnaceGUIMacroDesc& i: macros) {
      ImGui::PushID(index);
      ImGui::TableNextRow();

      // description
      ImGui::TableNextColumn();
      ImGui::Text("%s",i.displayName);
      ImGui::SameLine();
      if (ImGui::SmallButton(i.macro->open?(ICON_FA_CHEVRON_UP "##IMacroOpen"):(ICON_FA_CHEVRON_DOWN "##IMacroOpen"))) {
        i.macro->open=!i.macro->open;
      }
      if (i.macro->open) {
        ImGui::SetNextItemWidth(lenAvail);
        if (ImGui::InputScalar("##IMacroLen",ImGuiDataType_U8,&i.macro->len,&_ONE,&_THREE)) { MARK_MODIFIED
          if (i.macro->len>128) i.macro->len=128;
        }
        if (i.modeName!=NULL) {
          bool modeVal=i.macro->mode;
          String modeName=fmt::sprintf("%s##IMacroMode",i.modeName);
          if (ImGui::Checkbox(modeName.c_str(),&modeVal)) {
            i.macro->mode=modeVal;
          }
        }
      }

      // macro area
      ImGui::TableNextColumn();
      for (int j=0; j<256; j++) {
        if (j+macroDragScroll>=i.macro->len) {
          asFloat[j]=0;
          asInt[j]=0;
        } else {
          asFloat[j]=i.macro->val[j+macroDragScroll];
          asInt[j]=i.macro->val[j+macroDragScroll]+i.bitOffset;
        }
        if (j+macroDragScroll>=i.macro->len || (j+macroDragScroll>i.macro->rel && i.macro->loop<i.macro->rel)) {
          loopIndicator[j]=0;
        } else {
          loopIndicator[j]=((i.macro->loop!=-1 && (j+macroDragScroll)>=i.macro->loop))|((i.macro->rel!=-1 && (j+macroDragScroll)==i.macro->rel)<<1);
        }
      }
      ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(0.0f,0.0f));

      if (i.macro->vZoom<1) {
        if (i.macro->name=="arp") {
          i.macro->vZoom=24;
          i.macro->vScroll=120-12;
        } else if (i.macro->name=="pitch") {
          i.macro->vZoom=128;
          i.macro->vScroll=2048-64;
        } else {
          i.macro->vZoom=i.max-i.min;
          i.macro->vScroll=0;
        }
      }
      if (i.macro->vZoom>(i.max-i.min)) {
        i.macro->vZoom=i.max-i.min;
      }
           
      if (i.isBitfield) {
        PlotBitfield("##IMacro",asInt,totalFit,0,i.bitfieldBits,i.max,ImVec2(availableWidth,(i.macro->open)?(i.height*dpiScale):(32.0f*dpiScale)));
      } else {
        PlotCustom("##IMacro",asFloat,totalFit,macroDragScroll,NULL,i.min+i.macro->vScroll,i.min+i.macro->vScroll+i.macro->vZoom,ImVec2(availableWidth,(i.macro->open)?(i.height*dpiScale):(32.0f*dpiScale)),sizeof(float),i.color,i.macro->len-macroDragScroll,i.hoverFunc,i.blockMode,i.macro->open?genericGuide:NULL);
      }
      if (i.macro->open && (ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right))) {
        macroDragStart=ImGui::GetItemRectMin();
        macroDragAreaSize=ImVec2(availableWidth,i.height*dpiScale);
        if (i.isBitfield) {
          macroDragMin=i.min;
          macroDragMax=i.max;
        } else {
          macroDragMin=i.min+i.macro->vScroll;
          macroDragMax=i.min+i.macro->vScroll+i.macro->vZoom;
        }
        macroDragBitOff=i.bitOffset;
        macroDragBitMode=i.isBitfield;
        macroDragInitialValueSet=false;
        macroDragInitialValue=false;
        macroDragLen=totalFit;
        macroDragActive=true;
        macroDragTarget=i.macro->val;
        macroDragChar=false;
        macroDragLineMode=(i.isBitfield)?false:ImGui::IsItemClicked(ImGuiMouseButton_Right);
        macroDragLineInitial=ImVec2(0,0);
        lastMacroDesc=i;
        processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
      }
      if (i.macro->open) {
        if (ImGui::IsItemHovered()) {
          if (ctrlWheeling) {
            if (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift)) {
              i.macro->vZoom+=wheelY*(1+(i.macro->vZoom>>4));
              if (i.macro->vZoom<1) i.macro->vZoom=1;
              if (i.macro->vZoom>(i.max-i.min)) i.macro->vZoom=i.max-i.min;
              if ((i.macro->vScroll+i.macro->vZoom)>(i.max-i.min)) {
                i.macro->vScroll=(i.max-i.min)-i.macro->vZoom;
              }
            } else {
              macroPointSize+=wheelY;
              if (macroPointSize<1) macroPointSize=1;
              if (macroPointSize>256) macroPointSize=256;
            }
          } else if ((ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift)) && wheelY!=0) {
            i.macro->vScroll+=wheelY*(1+(i.macro->vZoom>>4));
            if (i.macro->vScroll<0) i.macro->vScroll=0;
            if (i.macro->vScroll>((i.max-i.min)-i.macro->vZoom)) i.macro->vScroll=(i.max-i.min)-i.macro->vZoom;
          }
        }

        // slider
        if (!i.isBitfield) {
          if (settings.oldMacroVSlider) {
            ImGui::SameLine(0.0f);
            if (ImGui::VSliderInt("IMacroVScroll",ImVec2(20.0f*dpiScale,i.height*dpiScale),&i.macro->vScroll,0,(i.max-i.min)-i.macro->vZoom,"")) {
              if (i.macro->vScroll<0) i.macro->vScroll=0;
              if (i.macro->vScroll>((i.max-i.min)-i.macro->vZoom)) i.macro->vScroll=(i.max-i.min)-i.macro->vZoom;
            }
            if (ImGui::IsItemHovered() && ctrlWheeling) {
              i.macro->vScroll+=wheelY*(1+(i.macro->vZoom>>4));
              if (i.macro->vScroll<0) i.macro->vScroll=0;
              if (i.macro->vScroll>((i.max-i.min)-i.macro->vZoom)) i.macro->vScroll=(i.max-i.min)-i.macro->vZoom;
            }
          } else {
            ImS64 scrollV=(i.max-i.min-i.macro->vZoom)-i.macro->vScroll;
            ImS64 availV=i.macro->vZoom;
            ImS64 contentsV=(i.max-i.min);

            ImGui::SameLine(0.0f);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX()-ImGui::GetStyle().ItemSpacing.x);
            ImRect scrollbarPos=ImRect(ImGui::GetCursorScreenPos(),ImGui::GetCursorScreenPos());
            scrollbarPos.Max.x+=ImGui::GetStyle().ScrollbarSize;
            scrollbarPos.Max.y+=i.height*dpiScale;
            ImGui::Dummy(ImVec2(ImGui::GetStyle().ScrollbarSize,i.height*dpiScale));
            if (ImGui::IsItemHovered() && ctrlWheeling) {
              i.macro->vScroll+=wheelY*(1+(i.macro->vZoom>>4));
              if (i.macro->vScroll<0) i.macro->vScroll=0;
              if (i.macro->vScroll>((i.max-i.min)-i.macro->vZoom)) i.macro->vScroll=(i.max-i.min)-i.macro->vZoom;
            }

            ImGuiID scrollbarID=ImGui::GetID("IMacroVScroll");
            ImGui::KeepAliveID(scrollbarID);
            if (ImGui::ScrollbarEx(scrollbarPos,scrollbarID,ImGuiAxis_Y,&scrollV,availV,contentsV,0)) {
              i.macro->vScroll=(i.max-i.min-i.macro->vZoom)-scrollV;
            }
          }
        }

        // loop area
        PlotCustom("##IMacroLoop",loopIndicator,totalFit,macroDragScroll,NULL,0,2,ImVec2(availableWidth,12.0f*dpiScale),sizeof(float),i.color,i.macro->len-macroDragScroll,&macroHoverLoop);
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
          macroLoopDragStart=ImGui::GetItemRectMin();
          macroLoopDragAreaSize=ImVec2(availableWidth,12.0f*dpiScale);
          macroLoopDragLen=totalFit;
          if (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift)) {
            macroLoopDragTarget=&i.macro->rel;
          } else {
            macroLoopDragTarget=&i.macro->loop;
          }
          macroLoopDragActive=true;
          processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
        }
        if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
          if (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift)) {
            i.macro->rel=-1;
          } else {
            i.macro->loop=-1;
          }
        }
        ImGui::SetNextItemWidth(availableWidth);
        String& mmlStr=mmlString[index];
        if (ImGui::InputText("##IMacroMML",&mmlStr)) {
          decodeMMLStr(mmlStr,i.macro->val,i.macro->len,i.macro->loop,i.min,(i.isBitfield)?((1<<(i.isBitfield?i.max:0))-1):i.max,i.macro->rel);
        }
        if (!ImGui::IsItemActive()) {
          encodeMMLStr(mmlStr,i.macro->val,i.macro->len,i.macro->loop,i.macro->rel);
        }
      }
      ImGui::PopStyleVar();
      ImGui::PopID();
      index++;
    }

    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(availableWidth);
    if (CWSliderInt("##MacroScroll",&macroDragScroll,0,128-totalFit,"")) {
      if (macroDragScroll<0) macroDragScroll=0;
      if (macroDragScroll>128-totalFit) macroDragScroll=128-totalFit;
    }
    ImGui::EndTable();
  }
}

#define DRUM_FREQ(name,db,df,prop) \
  ImGui::TableNextRow(); \
  ImGui::TableNextColumn(); \
  if (ins->type==DIV_INS_OPLL) { \
    block=(prop>>9)&7; \
    fNum=prop&511; \
  } else { \
    block=(prop>>10)&7; \
    fNum=prop&1023; \
  } \
  ImGui::Text(name); \
  ImGui::TableNextColumn(); \
  if (ImGui::InputInt(db,&block,1,1)) { \
    if (block<0) block=0; \
    if (block>7) block=7; \
    if (ins->type==DIV_INS_OPLL) { \
      prop=(block<<9)|fNum; \
    } else { \
      prop=(block<<10)|fNum; \
    } \
  } \
  ImGui::TableNextColumn(); \
  if (ImGui::InputInt(df,&fNum,1,1)) { \
    if (fNum<0) fNum=0; \
    if (ins->type==DIV_INS_OPLL) { \
      if (fNum>511) fNum=511; \
      prop=(block<<9)|fNum; \
    } else { \
      if (fNum>1023) fNum=1023; \
      prop=(block<<10)|fNum; \
    } \
  }


#define CENTER_TEXT(text) \
  ImGui::SetCursorPosX(ImGui::GetCursorPosX()+0.5*(ImGui::GetContentRegionAvail().x-ImGui::CalcTextSize(text).x));

#define CENTER_VSLIDER \
  ImGui::SetCursorPosX(ImGui::GetCursorPosX()+0.5f*ImGui::GetContentRegionAvail().x-10.0f*dpiScale);

#define CENTER_TEXT_20(text) \
  ImGui::SetCursorPosX(ImGui::GetCursorPosX()+0.5*(20.0f*dpiScale-ImGui::CalcTextSize(text).x));

void FurnaceGUI::drawInsEdit() {
  if (nextWindow==GUI_WINDOW_INS_EDIT) {
    insEditOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!insEditOpen) return;
  ImGui::SetNextWindowSizeConstraints(ImVec2(440.0f*dpiScale,400.0f*dpiScale),ImVec2(scrW*dpiScale,scrH*dpiScale));
  if (ImGui::Begin("Instrument Editor",&insEditOpen,globalWinFlags|(settings.allowEditDocking?0:ImGuiWindowFlags_NoDocking))) {
    if (curIns<0 || curIns>=(int)e->song.ins.size()) {
      ImGui::Text("no instrument selected");
    } else {
      DivInstrument* ins=e->song.ins[curIns];
      if (settings.insEditColorize) {
        pushAccentColors(uiColors[GUI_COLOR_INSTR_STD+ins->type],uiColors[GUI_COLOR_INSTR_STD+ins->type],uiColors[GUI_COLOR_INSTR_STD+ins->type],ImVec4(0.0f,0.0f,0.0f,0.0f));
      }
      if (ImGui::BeginTable("InsProp",3)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        String insIndex=fmt::sprintf("%.2X",curIns);
        ImGui::SetNextItemWidth(72.0f*dpiScale);
        if (ImGui::BeginCombo("##InsSelect",insIndex.c_str())) {
          String name;
          for (size_t i=0; i<e->song.ins.size(); i++) {
            name=fmt::sprintf("%.2X: %s##_INSS%d",i,e->song.ins[i]->name,i);
            if (ImGui::Selectable(name.c_str(),curIns==(int)i)) {
              curIns=i;
              ins=e->song.ins[curIns];
              wavePreviewInit=true;
            }
          }
          ImGui::EndCombo();
        }

        ImGui::TableNextColumn();
        ImGui::Text("Name");

        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputText("##Name",&ins->name)) {
          MARK_MODIFIED;
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Button(ICON_FA_FOLDER_OPEN "##IELoad")) {
          doAction(GUI_ACTION_INS_LIST_OPEN_REPLACE);
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_FLOPPY_O "##IESave")) {
          doAction(GUI_ACTION_INS_LIST_SAVE);
        }

        ImGui::TableNextColumn();
        ImGui::Text("Type");

        ImGui::TableNextColumn();
        if (ins->type>=DIV_INS_MAX) ins->type=DIV_INS_FM;
        int insType=ins->type;
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        /*
        if (ImGui::Combo("##Type",&insType,insTypes,DIV_INS_MAX,DIV_INS_MAX)) {
          ins->type=(DivInstrumentType)insType;
        }
        */
        if (ImGui::BeginCombo("##Type",insTypes[insType])) {
          std::vector<DivInstrumentType> insTypeList;
          if (settings.displayAllInsTypes) {
            for (int i=0; insTypes[i]; i++) {
              insTypeList.push_back((DivInstrumentType)i);
            }
          } else {
            insTypeList=e->getPossibleInsTypes();
          }
          for (DivInstrumentType i: insTypeList) {
            if (ImGui::Selectable(insTypes[i],insType==i)) {
              ins->type=i;

              // reset macro zoom
              ins->std.volMacro.vZoom=-1;
              ins->std.dutyMacro.vZoom=-1;
              ins->std.waveMacro.vZoom=-1;
              ins->std.ex1Macro.vZoom=-1;
              ins->std.ex2Macro.vZoom=-1;
              ins->std.ex3Macro.vZoom=-1;
              ins->std.ex4Macro.vZoom=-1;
              ins->std.ex5Macro.vZoom=-1;
              ins->std.ex6Macro.vZoom=-1;
              ins->std.ex7Macro.vZoom=-1;
              ins->std.ex8Macro.vZoom=-1;
              ins->std.panLMacro.vZoom=-1;
              ins->std.panRMacro.vZoom=-1;
              ins->std.phaseResetMacro.vZoom=-1;
              ins->std.algMacro.vZoom=-1;
              ins->std.fbMacro.vZoom=-1;
              ins->std.fmsMacro.vZoom=-1;
              ins->std.amsMacro.vZoom=-1;
              for (int j=0; j<4; j++) {
                ins->std.opMacros[j].amMacro.vZoom=-1;
                ins->std.opMacros[j].arMacro.vZoom=-1;
                ins->std.opMacros[j].drMacro.vZoom=-1;
                ins->std.opMacros[j].multMacro.vZoom=-1;
                ins->std.opMacros[j].rrMacro.vZoom=-1;
                ins->std.opMacros[j].slMacro.vZoom=-1;
                ins->std.opMacros[j].tlMacro.vZoom=-1;
                ins->std.opMacros[j].dt2Macro.vZoom=-1;
                ins->std.opMacros[j].rsMacro.vZoom=-1;
                ins->std.opMacros[j].dtMacro.vZoom=-1;
                ins->std.opMacros[j].d2rMacro.vZoom=-1;
                ins->std.opMacros[j].ssgMacro.vZoom=-1;
                ins->std.opMacros[j].damMacro.vZoom=-1;
                ins->std.opMacros[j].dvbMacro.vZoom=-1;
                ins->std.opMacros[j].egtMacro.vZoom=-1;
                ins->std.opMacros[j].kslMacro.vZoom=-1;
                ins->std.opMacros[j].susMacro.vZoom=-1;
                ins->std.opMacros[j].vibMacro.vZoom=-1;
                ins->std.opMacros[j].wsMacro.vZoom=-1;
                ins->std.opMacros[j].ksrMacro.vZoom=-1;
              }
            }
          }
          ImGui::EndCombo();
        }

        ImGui::EndTable();
      }
      

      if (ImGui::BeginTabBar("insEditTab")) {
        std::vector<FurnaceGUIMacroDesc> macroList;
        if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPLL || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPL_DRUMS) {
          char label[32];
          int opCount=4;
          if (ins->type==DIV_INS_OPLL) opCount=2;
          if (ins->type==DIV_INS_OPL) opCount=(ins->fm.ops==4)?4:2;

          if (ImGui::BeginTabItem("FM")) {
            if (ImGui::BeginTable("fmDetails",3,ImGuiTableFlags_SizingStretchSame)) {
              ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.0);
              ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0);
              ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.0);
              ImGui::TableNextRow();
              switch (ins->type) {
                case DIV_INS_FM:
                  ImGui::TableNextColumn();
                  P(CWSliderScalar(FM_NAME(FM_FB),ImGuiDataType_U8,&ins->fm.fb,&_ZERO,&_SEVEN)); rightClickable
                  P(CWSliderScalar(FM_NAME(FM_FMS),ImGuiDataType_U8,&ins->fm.fms,&_ZERO,&_SEVEN)); rightClickable
                  ImGui::TableNextColumn();
                  P(CWSliderScalar(FM_NAME(FM_ALG),ImGuiDataType_U8,&ins->fm.alg,&_ZERO,&_SEVEN)); rightClickable
                  P(CWSliderScalar(FM_NAME(FM_AMS),ImGuiDataType_U8,&ins->fm.ams,&_ZERO,&_THREE)); rightClickable
                  ImGui::TableNextColumn();
                  drawAlgorithm(ins->fm.alg,FM_ALGS_4OP,ImVec2(ImGui::GetContentRegionAvail().x,48.0*dpiScale));
                  break;
                case DIV_INS_OPZ:
                  ImGui::TableNextColumn();
                  P(CWSliderScalar(FM_NAME(FM_FB),ImGuiDataType_U8,&ins->fm.fb,&_ZERO,&_SEVEN)); rightClickable
                  P(CWSliderScalar(FM_NAME(FM_FMS),ImGuiDataType_U8,&ins->fm.fms,&_ZERO,&_SEVEN)); rightClickable
                  P(CWSliderScalar(FM_NAME(FM_FMS2),ImGuiDataType_U8,&ins->fm.fms2,&_ZERO,&_SEVEN)); rightClickable
                  ImGui::TableNextColumn();
                  P(CWSliderScalar(FM_NAME(FM_ALG),ImGuiDataType_U8,&ins->fm.alg,&_ZERO,&_SEVEN)); rightClickable
                  P(CWSliderScalar(FM_NAME(FM_AMS),ImGuiDataType_U8,&ins->fm.ams,&_ZERO,&_THREE)); rightClickable
                  P(CWSliderScalar(FM_NAME(FM_AMS2),ImGuiDataType_U8,&ins->fm.ams2,&_ZERO,&_THREE)); rightClickable
                  ImGui::TableNextColumn();
                  drawAlgorithm(ins->fm.alg,FM_ALGS_4OP,ImVec2(ImGui::GetContentRegionAvail().x,48.0*dpiScale));
                  if (ImGui::Button("Request from TX81Z")) {
                    doAction(GUI_ACTION_TX81Z_REQUEST);
                  }
                  ImGui::SameLine();
                  if (ImGui::Button("Send to TX81Z")) {
                    showError("Coming soon!");
                  }
                  break;
                case DIV_INS_OPL:
                case DIV_INS_OPL_DRUMS: {
                  bool fourOp=(ins->fm.ops==4 || ins->type==DIV_INS_OPL_DRUMS);
                  bool drums=ins->fm.opllPreset==16;
                  int algMax=fourOp?3:1;
                  ImGui::TableNextColumn();
                  ins->fm.alg&=algMax;
                  P(CWSliderScalar(FM_NAME(FM_FB),ImGuiDataType_U8,&ins->fm.fb,&_ZERO,&_SEVEN)); rightClickable
                  if (ins->type==DIV_INS_OPL) {
                    ImGui::BeginDisabled(ins->fm.opllPreset==16);
                    if (ImGui::Checkbox("4-op",&fourOp)) { PARAMETER
                      ins->fm.ops=fourOp?4:2;
                    }
                    ImGui::EndDisabled();
                  }
                  ImGui::TableNextColumn();
                  P(CWSliderScalar(FM_NAME(FM_ALG),ImGuiDataType_U8,&ins->fm.alg,&_ZERO,&algMax)); rightClickable
                  if (ins->type==DIV_INS_OPL) {
                    if (ImGui::Checkbox("Drums",&drums)) { PARAMETER
                      ins->fm.opllPreset=drums?16:0;
                    }
                  }
                  ImGui::TableNextColumn();
                  drawAlgorithm(ins->fm.alg&algMax,fourOp?FM_ALGS_4OP_OPL:FM_ALGS_2OP_OPL,ImVec2(ImGui::GetContentRegionAvail().x,48.0*dpiScale));
                  break;
                }
                case DIV_INS_OPLL: {
                  bool dc=ins->fm.fms;
                  bool dm=ins->fm.ams;
                  bool sus=ins->fm.alg;
                  ImGui::TableNextColumn();
                  ImGui::BeginDisabled(ins->fm.opllPreset!=0);
                  P(CWSliderScalar(FM_NAME(FM_FB),ImGuiDataType_U8,&ins->fm.fb,&_ZERO,&_SEVEN)); rightClickable
                  if (ImGui::Checkbox(FM_NAME(FM_DC),&dc)) { PARAMETER
                    ins->fm.fms=dc;
                  }
                  ImGui::EndDisabled();
                  ImGui::TableNextColumn();
                  if (ImGui::Checkbox(FM_NAME(FM_SUS),&sus)) { PARAMETER
                    ins->fm.alg=sus;
                  }
                  ImGui::BeginDisabled(ins->fm.opllPreset!=0);
                  if (ImGui::Checkbox(FM_NAME(FM_DM),&dm)) { PARAMETER
                    ins->fm.ams=dm;
                  }
                  ImGui::EndDisabled();
                  ImGui::TableNextColumn();
                  drawAlgorithm(0,FM_ALGS_2OP_OPL,ImVec2(ImGui::GetContentRegionAvail().x,24.0*dpiScale));
                  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                  if (ImGui::BeginCombo("##LLPreset",opllInsNames[ins->fm.opllPreset])) {
                    for (int i=0; i<17; i++) {
                      if (ImGui::Selectable(opllInsNames[i])) {
                        ins->fm.opllPreset=i;
                      }
                    }
                    ImGui::EndCombo();
                  }
                  break;
                }
                default:
                  break;
              }
              ImGui::EndTable();
            }

            if (((ins->type==DIV_INS_OPLL || ins->type==DIV_INS_OPL) && ins->fm.opllPreset==16) || ins->type==DIV_INS_OPL_DRUMS) {
              ins->fm.ops=2;
              P(ImGui::Checkbox("Fixed frequency mode",&ins->fm.fixedDrums));
              if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("when enabled, drums will be set to the specified frequencies, ignoring the note.");
              }
              if (ins->fm.fixedDrums) {
                int block=0;
                int fNum=0;
                if (ImGui::BeginTable("fixedDrumSettings",3)) {
                  ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
                  ImGui::TableNextColumn();
                  ImGui::Text("Drum");
                  ImGui::TableNextColumn();
                  ImGui::Text("Block");
                  ImGui::TableNextColumn();
                  ImGui::Text("FreqNum");

                  DRUM_FREQ("Kick","##DBlock0","##DFreq0",ins->fm.kickFreq);
                  DRUM_FREQ("Snare/Hi-hat","##DBlock1","##DFreq1",ins->fm.snareHatFreq);
                  DRUM_FREQ("Tom/Top","##DBlock2","##DFreq2",ins->fm.tomTopFreq);
                  ImGui::EndTable();
                }
              }
            }

            bool willDisplayOps=true;
            if (ins->type==DIV_INS_OPLL && ins->fm.opllPreset!=0) willDisplayOps=false;
            if (!willDisplayOps && ins->type==DIV_INS_OPLL) {
              ins->fm.op[1].tl&=15;
              P(CWSliderScalar("Volume##TL",ImGuiDataType_U8,&ins->fm.op[1].tl,&_FIFTEEN,&_ZERO)); rightClickable
              if (ins->fm.opllPreset==16) {
                ImGui::Text("this volume slider only works in compatibility (non-drums) system.");
              }
            }
            if (willDisplayOps) {
              if (settings.fmLayout==0) {
                int numCols=16;
                if (ins->type==DIV_INS_OPL ||ins->type==DIV_INS_OPL_DRUMS) numCols=13;
                if (ins->type==DIV_INS_OPLL) numCols=12;
                if (ins->type==DIV_INS_OPZ) numCols=19;
                if (ImGui::BeginTable("FMOperators",numCols,ImGuiTableFlags_SizingStretchProp|ImGuiTableFlags_BordersH|ImGuiTableFlags_BordersOuterV)) {
                  // configure columns
                  ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed); // op name
                  ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.05f); // ar
                  ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.05f); // dr
                  ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthStretch,0.05f); // sl
                  if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ) {
                    ImGui::TableSetupColumn("c4",ImGuiTableColumnFlags_WidthStretch,0.05f); // d2r
                  }
                  ImGui::TableSetupColumn("c5",ImGuiTableColumnFlags_WidthStretch,0.05f); // rr
                  ImGui::TableSetupColumn("c6",ImGuiTableColumnFlags_WidthFixed); // -separator-
                  ImGui::TableSetupColumn("c7",ImGuiTableColumnFlags_WidthStretch,0.05f); // tl
                  ImGui::TableSetupColumn("c8",ImGuiTableColumnFlags_WidthStretch,0.05f); // rs/ksl
                  if (ins->type==DIV_INS_OPZ) {
                    ImGui::TableSetupColumn("c8z0",ImGuiTableColumnFlags_WidthStretch,0.05f); // egs
                    ImGui::TableSetupColumn("c8z1",ImGuiTableColumnFlags_WidthStretch,0.05f); // rev
                  }
                  ImGui::TableSetupColumn("c9",ImGuiTableColumnFlags_WidthStretch,0.05f); // mult

                  if (ins->type==DIV_INS_OPZ) {
                    ImGui::TableSetupColumn("c9z",ImGuiTableColumnFlags_WidthStretch,0.05f); // fine
                  }

                  if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ) {
                    ImGui::TableSetupColumn("c10",ImGuiTableColumnFlags_WidthStretch,0.05f); // dt
                    ImGui::TableSetupColumn("c11",ImGuiTableColumnFlags_WidthStretch,0.05f); // dt2
                  }
                  ImGui::TableSetupColumn("c15",ImGuiTableColumnFlags_WidthFixed); // am

                  ImGui::TableSetupColumn("c12",ImGuiTableColumnFlags_WidthFixed); // -separator-
                  if (ins->type!=DIV_INS_OPLL) {
                    ImGui::TableSetupColumn("c13",ImGuiTableColumnFlags_WidthStretch,0.2f); // ssg/waveform
                  }
                  ImGui::TableSetupColumn("c14",ImGuiTableColumnFlags_WidthStretch,0.3f); // env

                  // header
                  ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
                  ImGui::TableNextColumn();

                  ImGui::TableNextColumn();
                  CENTER_TEXT(FM_SHORT_NAME(FM_AR));
                  ImGui::TextUnformatted(FM_SHORT_NAME(FM_AR));
                  ImGui::TableNextColumn();
                  CENTER_TEXT(FM_SHORT_NAME(FM_DR));
                  ImGui::TextUnformatted(FM_SHORT_NAME(FM_DR));
                  if (settings.susPosition==0) {
                    ImGui::TableNextColumn();
                    CENTER_TEXT(FM_SHORT_NAME(FM_SL));
                    ImGui::TextUnformatted(FM_SHORT_NAME(FM_SL));
                  }
                  if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ) {
                    ImGui::TableNextColumn();
                    CENTER_TEXT(FM_SHORT_NAME(FM_D2R));
                    ImGui::TextUnformatted(FM_SHORT_NAME(FM_D2R));
                  }
                  ImGui::TableNextColumn();
                  CENTER_TEXT(FM_SHORT_NAME(FM_RR));
                  ImGui::TextUnformatted(FM_SHORT_NAME(FM_RR));
                  if (settings.susPosition==1) {
                    ImGui::TableNextColumn();
                    CENTER_TEXT(FM_SHORT_NAME(FM_SL));
                    ImGui::TextUnformatted(FM_SHORT_NAME(FM_SL));
                  }
                  ImGui::TableNextColumn();
                  ImGui::TableNextColumn();
                  CENTER_TEXT(FM_SHORT_NAME(FM_TL));
                  ImGui::TextUnformatted(FM_SHORT_NAME(FM_TL));
                  ImGui::TableNextColumn();
                  if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ) {
                    CENTER_TEXT(FM_SHORT_NAME(FM_RS));
                    ImGui::TextUnformatted(FM_SHORT_NAME(FM_RS));
                  } else {
                    CENTER_TEXT(FM_SHORT_NAME(FM_KSL));
                    ImGui::TextUnformatted(FM_SHORT_NAME(FM_KSL));
                  }
                  if (ins->type==DIV_INS_OPZ) {
                    ImGui::TableNextColumn();
                    CENTER_TEXT(FM_SHORT_NAME(FM_EGSHIFT));
                    ImGui::TextUnformatted(FM_SHORT_NAME(FM_EGSHIFT));
                    ImGui::TableNextColumn();
                    CENTER_TEXT(FM_SHORT_NAME(FM_REV));
                    ImGui::TextUnformatted(FM_SHORT_NAME(FM_REV));
                  }
                  ImGui::TableNextColumn();
                  CENTER_TEXT(FM_SHORT_NAME(FM_MULT));
                  ImGui::TextUnformatted(FM_SHORT_NAME(FM_MULT));
                  if (ins->type==DIV_INS_OPZ) {
                    ImGui::TableNextColumn();
                    CENTER_TEXT(FM_SHORT_NAME(FM_FINE));
                    ImGui::TextUnformatted(FM_SHORT_NAME(FM_FINE));
                  }
                  ImGui::TableNextColumn();
                  if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ) {
                    CENTER_TEXT(FM_SHORT_NAME(FM_DT));
                    ImGui::TextUnformatted(FM_SHORT_NAME(FM_DT));
                    ImGui::TableNextColumn();
                    CENTER_TEXT(FM_SHORT_NAME(FM_DT2));
                    ImGui::TextUnformatted(FM_SHORT_NAME(FM_DT2));
                    ImGui::TableNextColumn();
                  }
                  if (ins->type==DIV_INS_FM) {
                    CENTER_TEXT(FM_SHORT_NAME(FM_AM));
                    ImGui::TextUnformatted(FM_SHORT_NAME(FM_AM));
                  } else {
                    CENTER_TEXT("Other");
                    ImGui::TextUnformatted("Other");
                  }
                  ImGui::TableNextColumn();
                  if (ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_OPZ) {
                    ImGui::TableNextColumn();
                    CENTER_TEXT(FM_NAME(FM_WS));
                    ImGui::TextUnformatted(FM_NAME(FM_WS));
                  } else if (ins->type!=DIV_INS_OPLL) {
                    ImGui::TableNextColumn();
                    CENTER_TEXT(FM_NAME(FM_SSG));
                    ImGui::TextUnformatted(FM_NAME(FM_SSG));
                  }
                  ImGui::TableNextColumn();
                  CENTER_TEXT("Envelope");
                  ImGui::TextUnformatted("Envelope");

                  float sliderHeight=32.0f*dpiScale;

                  for (int i=0; i<opCount; i++) {
                    DivInstrumentFM::Operator& op=ins->fm.op[(opCount==4 && ins->type!=DIV_INS_OPL_DRUMS)?opOrder[i]:i];
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();

                    // push colors
                    if (settings.separateFMColors) {
                      bool mod=true;
                      if (ins->type==DIV_INS_OPL_DRUMS) {
                        mod=false;
                      } else if (opCount==4) {
                        if (ins->type==DIV_INS_OPL) {
                          if (opIsOutputOPL[ins->fm.alg&3][i]) mod=false;
                        } else {
                          if (opIsOutput[ins->fm.alg&7][i]) mod=false;
                        }
                      } else {
                        if (i==1 || (ins->type==DIV_INS_OPL && (ins->fm.alg&1))) mod=false;
                      }
                      if (mod) {
                        pushAccentColors(
                          uiColors[GUI_COLOR_FM_PRIMARY_MOD],
                          uiColors[GUI_COLOR_FM_SECONDARY_MOD],
                          uiColors[GUI_COLOR_FM_BORDER_MOD],
                          uiColors[GUI_COLOR_FM_BORDER_SHADOW_MOD]
                        );
                      } else {
                        pushAccentColors(
                          uiColors[GUI_COLOR_FM_PRIMARY_CAR],
                          uiColors[GUI_COLOR_FM_SECONDARY_CAR],
                          uiColors[GUI_COLOR_FM_BORDER_CAR],
                          uiColors[GUI_COLOR_FM_BORDER_SHADOW_CAR]
                        );
                      }
                    }

                    if (i==0) sliderHeight=(ImGui::GetContentRegionAvail().y/opCount)-ImGui::GetStyle().ItemSpacing.y;

                    ImGui::PushID(fmt::sprintf("op%d",i).c_str());
                    if (ins->type==DIV_INS_OPL_DRUMS) {
                      ImGui::Text("%s",oplDrumNames[i]);
                    } else if (ins->type==DIV_INS_OPL && ins->fm.opllPreset==16) {
                      if (i==1) {
                        ImGui::Text("Kick");
                      } else {
                        ImGui::Text("Env");
                      }
                    } else {
                      ImGui::Text("OP%d",i+1);
                    }

                    int maxTl=127;
                    if (ins->type==DIV_INS_OPLL) {
                      if (i==1) {
                        maxTl=15;
                      } else {
                        maxTl=63;
                      }
                    }
                    if (ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS) {
                      maxTl=63;
                    }
                    int maxArDr=(ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ)?31:15;
                    bool ssgOn=op.ssgEnv&8;
                    bool ksrOn=op.ksr;
                    bool vibOn=op.vib;
                    bool susOn=op.sus;
                    unsigned char ssgEnv=op.ssgEnv&7;

                    ImGui::TableNextColumn();
                    op.ar&=maxArDr;
                    CENTER_VSLIDER;
                    P(CWVSliderScalar("##AR",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.ar,&maxArDr,&_ZERO));

                    ImGui::TableNextColumn();
                    op.dr&=maxArDr;
                    CENTER_VSLIDER;
                    P(CWVSliderScalar("##DR",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.dr,&maxArDr,&_ZERO));

                    if (settings.susPosition==0) {
                      ImGui::TableNextColumn();
                      op.sl&=15;
                      CENTER_VSLIDER;
                      P(CWVSliderScalar("##SL",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.sl,&_FIFTEEN,&_ZERO));
                    }

                    if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ) {
                      ImGui::TableNextColumn();
                      op.d2r&=31;
                      CENTER_VSLIDER;
                      P(CWVSliderScalar("##D2R",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.d2r,&_THIRTY_ONE,&_ZERO));
                    }

                    ImGui::TableNextColumn();
                    op.rr&=15;
                    CENTER_VSLIDER;
                    P(CWVSliderScalar("##RR",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.rr,&_FIFTEEN,&_ZERO));

                    if (settings.susPosition==1) {
                      ImGui::TableNextColumn();
                      op.sl&=15;
                      CENTER_VSLIDER;
                      P(CWVSliderScalar("##SL",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.sl,&_FIFTEEN,&_ZERO));
                    }

                    ImGui::TableNextColumn();
                    ImGui::Dummy(ImVec2(4.0f*dpiScale,2.0f*dpiScale));

                    ImGui::TableNextColumn();
                    op.tl&=maxTl;
                    CENTER_VSLIDER;
                    P(CWVSliderScalar("##TL",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.tl,&maxTl,&_ZERO));

                    ImGui::TableNextColumn();
                    CENTER_VSLIDER;
                    if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ) {
                      P(CWVSliderScalar("##RS",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.rs,&_ZERO,&_THREE));
                    } else {
                      P(CWVSliderScalar("##KSL",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.ksl,&_ZERO,&_THREE));
                    }

                    if (ins->type==DIV_INS_OPZ) {
                      ImGui::TableNextColumn();
                      CENTER_VSLIDER;
                      P(CWVSliderScalar("##EGS",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.ksl,&_ZERO,&_THREE));

                      ImGui::TableNextColumn();
                      CENTER_VSLIDER;
                      P(CWVSliderScalar("##REV",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.dam,&_ZERO,&_SEVEN));
                    }

                    ImGui::TableNextColumn();
                    CENTER_VSLIDER;
                    P(CWVSliderScalar("##MULT",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.mult,&_ZERO,&_FIFTEEN));

                    if (ins->type==DIV_INS_OPZ) {
                      ImGui::TableNextColumn();
                      CENTER_VSLIDER;
                      P(CWVSliderScalar("##FINE",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.dvb,&_ZERO,&_FIFTEEN));
                    }

                    if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ) {
                      int detune=(op.dt&7)-(settings.unsignedDetune?0:3);
                      ImGui::TableNextColumn();
                      CENTER_VSLIDER;
                      if (CWVSliderInt("##DT",ImVec2(20.0f*dpiScale,sliderHeight),&detune,-3,4)) { PARAMETER
                        op.dt=detune+(settings.unsignedDetune?0:3);
                      }

                      ImGui::TableNextColumn();
                      CENTER_VSLIDER;
                      P(CWVSliderScalar("##DT2",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.dt2,&_ZERO,&_THREE)); rightClickable
                      if (ImGui::IsItemHovered() && ins->type==DIV_INS_FM) {
                        ImGui::SetTooltip("Only on YM2151 (OPM)");
                      }

                      ImGui::TableNextColumn();
                      bool amOn=op.am;
                      if (ins->type==DIV_INS_OPZ) {
                        bool egtOn=op.egt;
                        if (egtOn) {
                          ImGui::SetCursorPosY(ImGui::GetCursorPosY()+0.5*(sliderHeight-ImGui::GetFrameHeight()*4.0-ImGui::GetStyle().ItemSpacing.y*3.0));
                        } else {
                          ImGui::SetCursorPosY(ImGui::GetCursorPosY()+0.5*(sliderHeight-ImGui::GetFrameHeight()*2.0-ImGui::GetStyle().ItemSpacing.y*1.0));
                        }
                        if (ImGui::Checkbox("AM",&amOn)) { PARAMETER
                          op.am=amOn;
                        }
                        if (ImGui::Checkbox("Fixed",&egtOn)) { PARAMETER
                          op.egt=egtOn;
                        }
                        if (egtOn) {
                          int block=op.dt;
                          int freqNum=(op.mult<<4)|(op.dvb&15);
                          if (ImGui::InputInt("Block",&block,1,1)) {
                            if (block<0) block=0;
                            if (block>7) block=7;
                            op.dt=block;
                          }
                          if (ImGui::InputInt("FreqNum",&freqNum,1,16)) {
                            if (freqNum<0) freqNum=0;
                            if (freqNum>255) freqNum=255;
                            op.mult=freqNum>>4;
                            op.dvb=freqNum&15;
                          }
                        }
                      } else {
                        ImGui::SetCursorPosY(ImGui::GetCursorPosY()+0.5*(sliderHeight-ImGui::GetFrameHeight()));
                        if (ImGui::Checkbox("##AM",&amOn)) { PARAMETER
                          op.am=amOn;
                        }
                      }

                      if (ins->type!=DIV_INS_OPL && ins->type!=DIV_INS_OPL_DRUMS && ins->type!=DIV_INS_OPZ) {
                        ImGui::TableNextColumn();
                        ImGui::Dummy(ImVec2(4.0f*dpiScale,2.0f*dpiScale));
                        ImGui::TableNextColumn();
                        ImGui::BeginDisabled(!ssgOn);
                        drawSSGEnv(op.ssgEnv&7,ImVec2(ImGui::GetContentRegionAvail().x,sliderHeight-ImGui::GetFrameHeightWithSpacing()));
                        ImGui::EndDisabled();
                        if (ImGui::Checkbox("##SSGOn",&ssgOn)) { PARAMETER
                          op.ssgEnv=(op.ssgEnv&7)|(ssgOn<<3);
                        }
                        if (ins->type==DIV_INS_FM) {
                          if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("Only for OPN family chips");
                          }
                        }

                        ImGui::SameLine();
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        if (CWSliderScalar("##SSG",ImGuiDataType_U8,&ssgEnv,&_ZERO,&_SEVEN,ssgEnvTypes[ssgEnv])) { PARAMETER
                          op.ssgEnv=(op.ssgEnv&8)|(ssgEnv&7);
                        }
                      }
                    } else {
                      ImGui::TableNextColumn();
                      bool amOn=op.am;
                      ImGui::SetCursorPosY(ImGui::GetCursorPosY()+0.5*(sliderHeight-ImGui::GetFrameHeight()*4.0-ImGui::GetStyle().ItemSpacing.y*3.0));
                      if (ImGui::Checkbox(FM_NAME(FM_AM),&amOn)) { PARAMETER
                        op.am=amOn;
                      }
                      if (ImGui::Checkbox(FM_NAME(FM_VIB),&vibOn)) { PARAMETER
                        op.vib=vibOn;
                      }
                      if (ImGui::Checkbox(FM_NAME(FM_KSR),&ksrOn)) { PARAMETER
                        op.ksr=ksrOn;
                      }
                      if (ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS) {
                        if (ImGui::Checkbox(FM_NAME(FM_SUS),&susOn)) { PARAMETER
                          op.sus=susOn;
                        }
                      } else if (ins->type==DIV_INS_OPLL) {
                        if (ImGui::Checkbox(FM_NAME(FM_EGS),&ssgOn)) { PARAMETER
                          op.ssgEnv=(op.ssgEnv&7)|(ssgOn<<3);
                        }
                      }
                    }

                    if (ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_OPZ) {
                      ImGui::TableNextColumn();
                      ImGui::Dummy(ImVec2(4.0f*dpiScale,2.0f*dpiScale));
                      ImGui::TableNextColumn();

                      drawWaveform(op.ws&7,ins->type==DIV_INS_OPZ,ImVec2(ImGui::GetContentRegionAvail().x,sliderHeight-ImGui::GetFrameHeightWithSpacing()));
                      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                      P(CWSliderScalar("##WS",ImGuiDataType_U8,&op.ws,&_ZERO,&_SEVEN,(ins->type==DIV_INS_OPZ)?opzWaveforms[op.ws&7]:(settings.oplStandardWaveNames?oplWaveformsStandard[op.ws&7]:oplWaveforms[op.ws&7]))); rightClickable
                      if ((ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS) && ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("OPL2/3 only (last 4 waveforms are OPL3 only)");
                      }
                    } else if (ins->type==DIV_INS_OPLL) {
                      ImGui::TableNextColumn();
                      ImGui::Dummy(ImVec2(4.0f*dpiScale,2.0f*dpiScale));
                    }

                    ImGui::TableNextColumn();
                    drawFMEnv(op.tl&maxTl,op.ar&maxArDr,op.dr&maxArDr,(ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_OPLL)?((op.rr&15)*2):op.d2r&31,op.rr&15,op.sl&15,op.sus,op.ssgEnv&8,ins->fm.alg,maxTl,maxArDr,ImVec2(ImGui::GetContentRegionAvail().x,sliderHeight),ins->type);

                    if (settings.separateFMColors) {
                      popAccentColors();
                    }

                    ImGui::PopID();
                  }

                  ImGui::EndTable();
                }
              } else if (settings.fmLayout>=4 && settings.fmLayout<=6) { // alternate
                int columns=2;
                switch (settings.fmLayout) {
                  case 4: // 2x2
                    columns=2;
                    break;
                  case 5: // 1x4
                    columns=1;
                    break;
                  case 6: // 4x1
                    columns=opCount;
                    break;
                }
                char tempID[1024];
                ImVec2 oldPadding=ImGui::GetStyle().CellPadding;
                ImGui::PushStyleVar(ImGuiStyleVar_CellPadding,ImVec2(8.0f*dpiScale,4.0f*dpiScale));
                if (ImGui::BeginTable("KGE93BSIEO3NOWBDJZBA",columns,ImGuiTableFlags_SizingStretchSame|ImGuiTableFlags_BordersInner)) {
                  for (int i=0; i<opCount; i++) {
                    DivInstrumentFM::Operator& op=ins->fm.op[(opCount==4 && ins->type!=DIV_INS_OPL_DRUMS)?opOrder[i]:i];
                    if ((settings.fmLayout!=6 && ((i+1)&1)) || i==0 || settings.fmLayout==5) ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::PushID(fmt::sprintf("op%d",i).c_str());

                    // push colors
                    if (settings.separateFMColors) {
                      bool mod=true;
                      if (ins->type==DIV_INS_OPL_DRUMS) {
                        mod=false;
                      } else if (opCount==4) {
                        if (ins->type==DIV_INS_OPL) {
                          if (opIsOutputOPL[ins->fm.alg&3][i]) mod=false;
                        } else {
                          if (opIsOutput[ins->fm.alg&7][i]) mod=false;
                        }
                      } else {
                        if (i==1 || (ins->type==DIV_INS_OPL && (ins->fm.alg&1))) mod=false;
                      }
                      if (mod) {
                        pushAccentColors(
                          uiColors[GUI_COLOR_FM_PRIMARY_MOD],
                          uiColors[GUI_COLOR_FM_SECONDARY_MOD],
                          uiColors[GUI_COLOR_FM_BORDER_MOD],
                          uiColors[GUI_COLOR_FM_BORDER_SHADOW_MOD]
                        );
                      } else {
                        pushAccentColors(
                          uiColors[GUI_COLOR_FM_PRIMARY_CAR],
                          uiColors[GUI_COLOR_FM_SECONDARY_CAR],
                          uiColors[GUI_COLOR_FM_BORDER_CAR],
                          uiColors[GUI_COLOR_FM_BORDER_SHADOW_CAR]
                        );
                      }
                    }

                    ImGui::Dummy(ImVec2(dpiScale,dpiScale));
                    if (ins->type==DIV_INS_OPL_DRUMS) {
                      snprintf(tempID,1024,"%s",oplDrumNames[i]);
                    } else if (ins->type==DIV_INS_OPL && ins->fm.opllPreset==16) {
                      if (i==1) {
                        snprintf(tempID,1024,"Envelope 2 (kick only)");
                      } else {
                        snprintf(tempID,1024,"Envelope");
                      }
                    } else {
                      snprintf(tempID,1024,"Operator %d",i+1);
                    }
                    CENTER_TEXT(tempID);
                    ImGui::TextUnformatted(tempID);

                    float sliderHeight=200.0f*dpiScale;
                    float waveWidth=140.0*dpiScale;
                    float waveHeight=sliderHeight-ImGui::GetFrameHeightWithSpacing()*(ins->type==DIV_INS_OPLL?4.5f:5.5f);

                    int maxTl=127;
                    if (ins->type==DIV_INS_OPLL) {
                      if (i==1) {
                        maxTl=15;
                      } else {
                        maxTl=63;
                      }
                    }
                    if (ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS) {
                      maxTl=63;
                    }
                    int maxArDr=(ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ)?31:15;

                    bool ssgOn=op.ssgEnv&8;
                    bool ksrOn=op.ksr;
                    bool vibOn=op.vib;
                    bool egtOn=op.egt;
                    bool susOn=op.sus; // yawn
                    unsigned char ssgEnv=op.ssgEnv&7;

                    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding,oldPadding);
                    if (ImGui::BeginTable("opParams",4,ImGuiTableFlags_BordersInnerV)) {
                      ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
                      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed,waveWidth);
                      ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch);
                      ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthFixed);

                      ImGui::TableNextRow();
                      ImGui::TableNextColumn();
                      float textY=ImGui::GetCursorPosY();
                      CENTER_TEXT_20(FM_SHORT_NAME(FM_AR));
                      ImGui::TextUnformatted(FM_SHORT_NAME(FM_AR));
                      ImGui::TableNextColumn();
                      if (ins->type==DIV_INS_FM) {
                        ImGui::Text("SSG-EG");
                      } else {
                        ImGui::Text("Waveform");
                      }
                      ImGui::TableNextColumn();
                      ImGui::Text("Envelope");
                      ImGui::TableNextColumn();
                      CENTER_TEXT(FM_SHORT_NAME(FM_TL));
                      ImGui::Text("TL");

                      // A/D/S/R
                      ImGui::TableNextColumn();

                      op.ar&=maxArDr;
                      P(CWVSliderScalar("##AR",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.ar,&maxArDr,&_ZERO));

                      ImGui::SameLine();
                      op.dr&=maxArDr;
                      float textX_DR=ImGui::GetCursorPosX();
                      P(CWVSliderScalar("##DR",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.dr,&maxArDr,&_ZERO));

                      float textX_SL=0.0f;
                      if (settings.susPosition==0) {
                        ImGui::SameLine();
                        op.sl&=15;
                        textX_SL=ImGui::GetCursorPosX();
                        P(CWVSliderScalar("##SL",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.sl,&_FIFTEEN,&_ZERO));
                      }

                      float textX_D2R=0.0f;
                      if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ) {
                        ImGui::SameLine();
                        op.d2r&=31;
                        textX_D2R=ImGui::GetCursorPosX();
                        P(CWVSliderScalar("##D2R",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.d2r,&_THIRTY_ONE,&_ZERO));
                      }

                      ImGui::SameLine();
                      op.rr&=15;
                      float textX_RR=ImGui::GetCursorPosX();
                      P(CWVSliderScalar("##RR",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.rr,&_FIFTEEN,&_ZERO));

                      if (settings.susPosition==1) {
                        ImGui::SameLine();
                        op.sl&=15;
                        textX_SL=ImGui::GetCursorPosX();
                        P(CWVSliderScalar("##SL",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.sl,&_FIFTEEN,&_ZERO));
                      }

                      ImVec2 prevCurPos=ImGui::GetCursorPos();
                      
                      // labels
                      ImGui::SetCursorPos(ImVec2(textX_DR,textY));
                      CENTER_TEXT_20(FM_SHORT_NAME(FM_DR));
                      ImGui::TextUnformatted(FM_SHORT_NAME(FM_DR));

                      ImGui::SetCursorPos(ImVec2(textX_SL,textY));
                      CENTER_TEXT_20(FM_SHORT_NAME(FM_SL));
                      ImGui::TextUnformatted(FM_SHORT_NAME(FM_SL));

                      ImGui::SetCursorPos(ImVec2(textX_RR,textY));
                      CENTER_TEXT_20(FM_SHORT_NAME(FM_RR));
                      ImGui::TextUnformatted(FM_SHORT_NAME(FM_RR));

                      if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ) {
                        ImGui::SetCursorPos(ImVec2(textX_D2R,textY));
                        CENTER_TEXT_20(FM_SHORT_NAME(FM_D2R));
                        ImGui::TextUnformatted(FM_SHORT_NAME(FM_D2R));
                      }

                      ImGui::SetCursorPos(prevCurPos);
                      
                      ImGui::TableNextColumn();
                      switch (ins->type) {
                        case DIV_INS_FM: {
                          // SSG
                          ImGui::BeginDisabled(!ssgOn);
                          drawSSGEnv(op.ssgEnv&7,ImVec2(waveWidth,waveHeight));
                          ImGui::EndDisabled();
                          if (ImGui::Checkbox("##SSGOn",&ssgOn)) { PARAMETER
                            op.ssgEnv=(op.ssgEnv&7)|(ssgOn<<3);
                          }
                          if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("Only for OPN family chips");
                          }

                          ImGui::SameLine();
                          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                          if (CWSliderScalar("##SSG",ImGuiDataType_U8,&ssgEnv,&_ZERO,&_SEVEN,ssgEnvTypes[ssgEnv])) { PARAMETER
                            op.ssgEnv=(op.ssgEnv&8)|(ssgEnv&7);
                          }
                          
                          // params
                          ImGui::Separator();
                          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                          snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_MULT));
                          P(CWSliderScalar("##MULT",ImGuiDataType_U8,&op.mult,&_ZERO,&_FIFTEEN,tempID)); rightClickable

                          int detune=(op.dt&7)-(settings.unsignedDetune?0:3);
                          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                          snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_DT));
                          if (CWSliderInt("##DT",&detune,-3,4,tempID)) { PARAMETER
                            op.dt=detune+(settings.unsignedDetune?0:3);
                          } rightClickable

                          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                          snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_DT2));
                          P(CWSliderScalar("##DT2",ImGuiDataType_U8,&op.dt2,&_ZERO,&_THREE,tempID)); rightClickable
                          if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("Only on YM2151 (OPM)");
                          }

                          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                          snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_RS));
                          P(CWSliderScalar("##RS",ImGuiDataType_U8,&op.rs,&_ZERO,&_THREE,tempID)); rightClickable

                          break;
                        }
                        case DIV_INS_OPLL:
                          // waveform
                          drawWaveform(i==0?(ins->fm.ams&1):(ins->fm.fms&1),ins->type==DIV_INS_OPZ,ImVec2(waveWidth,waveHeight));

                          // params
                          ImGui::Separator();
                          if (ImGui::BeginTable("FMParamsInner",2)) {
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            bool amOn=op.am;
                            if (ImGui::Checkbox(FM_NAME(FM_AM),&amOn)) { PARAMETER
                              op.am=amOn;
                            }
                            ImGui::TableNextColumn();
                            if (ImGui::Checkbox(FM_NAME(FM_KSR),&ksrOn)) { PARAMETER
                              op.ksr=ksrOn;
                            }

                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            if (ImGui::Checkbox(FM_NAME(FM_VIB),&vibOn)) { PARAMETER
                              op.vib=vibOn;
                            }
                            ImGui::TableNextColumn();
                            if (ImGui::Checkbox(FM_NAME(FM_EGS),&ssgOn)) { PARAMETER
                              op.ssgEnv=(op.ssgEnv&7)|(ssgOn<<3);
                            }
                            
                            ImGui::EndTable();
                          }

                          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                          snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_MULT));
                          P(CWSliderScalar("##MULT",ImGuiDataType_U8,&op.mult,&_ZERO,&_FIFTEEN,tempID)); rightClickable

                          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                          snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_KSL));
                          P(CWSliderScalar("##KSL",ImGuiDataType_U8,&op.ksl,&_ZERO,&_THREE,tempID)); rightClickable

                          break;
                        case DIV_INS_OPL:
                        case DIV_INS_OPL_DRUMS:
                          // waveform
                          drawWaveform(op.ws&7,ins->type==DIV_INS_OPZ,ImVec2(waveWidth,waveHeight));
                          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                          P(CWSliderScalar("##WS",ImGuiDataType_U8,&op.ws,&_ZERO,&_SEVEN,(ins->type==DIV_INS_OPZ)?opzWaveforms[op.ws&7]:(settings.oplStandardWaveNames?oplWaveformsStandard[op.ws&7]:oplWaveforms[op.ws&7]))); rightClickable
                          if ((ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS) && ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("OPL2/3 only (last 4 waveforms are OPL3 only)");
                          }

                          // params
                          ImGui::Separator();
                          if (ImGui::BeginTable("FMParamsInner",2)) {
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            bool amOn=op.am;
                            if (ImGui::Checkbox(FM_NAME(FM_AM),&amOn)) { PARAMETER
                              op.am=amOn;
                            }
                            ImGui::TableNextColumn();
                            if (ImGui::Checkbox(FM_NAME(FM_KSR),&ksrOn)) { PARAMETER
                              op.ksr=ksrOn;
                            }

                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            if (ImGui::Checkbox(FM_NAME(FM_VIB),&vibOn)) { PARAMETER
                              op.vib=vibOn;
                            }
                            ImGui::TableNextColumn();
                            if (ImGui::Checkbox(FM_NAME(FM_SUS),&susOn)) { PARAMETER
                              op.sus=susOn;
                            }
                            
                            ImGui::EndTable();
                          }

                          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                          snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_MULT));
                          P(CWSliderScalar("##MULT",ImGuiDataType_U8,&op.mult,&_ZERO,&_FIFTEEN,tempID)); rightClickable

                          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                          snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_KSL));
                          P(CWSliderScalar("##KSL",ImGuiDataType_U8,&op.ksl,&_ZERO,&_THREE,tempID)); rightClickable

                          break;
                        case DIV_INS_OPZ: {
                          // waveform
                          drawWaveform(op.ws&7,ins->type==DIV_INS_OPZ,ImVec2(waveWidth,waveHeight));
                          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                          P(CWSliderScalar("##WS",ImGuiDataType_U8,&op.ws,&_ZERO,&_SEVEN,(ins->type==DIV_INS_OPZ)?opzWaveforms[op.ws&7]:(settings.oplStandardWaveNames?oplWaveformsStandard[op.ws&7]:oplWaveforms[op.ws&7]))); rightClickable
                          if ((ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS) && ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("OPL2/3 only (last 4 waveforms are OPL3 only)");
                          }

                          // params
                          ImGui::Separator();
                          if (egtOn) {
                            int block=op.dt;
                            int freqNum=(op.mult<<4)|(op.dvb&15);
                            ImGui::Text("Block");
                            ImGui::SameLine();
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                            ImVec2 cursorAlign=ImGui::GetCursorPos();
                            if (ImGui::InputInt("##Block",&block,1,1)) {
                              if (block<0) block=0;
                              if (block>7) block=7;
                              op.dt=block;
                            }
                            
                            ImGui::Text("Freq");
                            ImGui::SameLine();
                            ImGui::SetCursorPos(ImVec2(cursorAlign.x,ImGui::GetCursorPosY()));
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                            if (ImGui::InputInt("##FreqNum",&freqNum,1,16)) {
                              if (freqNum<0) freqNum=0;
                              if (freqNum>255) freqNum=255;
                              op.mult=freqNum>>4;
                              op.dvb=freqNum&15;
                            }
                          } else {
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                            snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_MULT));
                            P(CWSliderScalar("##MULT",ImGuiDataType_U8,&op.mult,&_ZERO,&_FIFTEEN,tempID)); rightClickable

                            int detune=(op.dt&7)-(settings.unsignedDetune?0:3);
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                            snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_DT));
                            if (CWSliderInt("##DT",&detune,-3,4,tempID)) { PARAMETER
                              op.dt=detune+(settings.unsignedDetune?0:3);
                            } rightClickable
                          }

                          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                          snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_DT2));
                          P(CWSliderScalar("##DT2",ImGuiDataType_U8,&op.dt2,&_ZERO,&_THREE,tempID)); rightClickable
                          if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("Only on YM2151 (OPM)");
                          }

                          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                          snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_RS));
                          P(CWSliderScalar("##RS",ImGuiDataType_U8,&op.rs,&_ZERO,&_THREE,tempID)); rightClickable
                          break;
                        }
                        default:
                          break;
                      }

                      ImGui::TableNextColumn();
                      float envHeight=sliderHeight;//-ImGui::GetStyle().ItemSpacing.y*2.0f;
                      if (ins->type==DIV_INS_OPZ) {
                        envHeight-=ImGui::GetFrameHeightWithSpacing()*2.0f;
                      }
                      drawFMEnv(op.tl&maxTl,op.ar&maxArDr,op.dr&maxArDr,(ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_OPLL)?((op.rr&15)*2):op.d2r&31,op.rr&15,op.sl&15,op.sus,op.ssgEnv&8,ins->fm.alg,maxTl,maxArDr,ImVec2(ImGui::GetContentRegionAvail().x,envHeight),ins->type);

                      if (ins->type==DIV_INS_OPZ) {
                        ImGui::Separator();
                        if (ImGui::BeginTable("FMParamsInnerOPZ",2)) {
                          ImGui::TableNextRow();
                          ImGui::TableNextColumn();
                          if (!egtOn) {
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                            snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_FINE));
                            P(CWSliderScalar("##FINE",ImGuiDataType_U8,&op.dvb,&_ZERO,&_FIFTEEN,tempID)); rightClickable
                          }

                          ImGui::TableNextColumn();
                          bool amOn=op.am;
                          if (ImGui::Checkbox(FM_NAME(FM_AM),&amOn)) { PARAMETER
                            op.am=amOn;
                          }
                          ImGui::SameLine();
                          if (ImGui::Checkbox("Fixed",&egtOn)) { PARAMETER
                            op.egt=egtOn;
                          }

                          ImGui::TableNextRow();
                          ImGui::TableNextColumn();
                          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                          snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_EGSHIFT));
                          P(CWSliderScalar("##EGShift",ImGuiDataType_U8,&op.ksl,&_ZERO,&_THREE,tempID)); rightClickable

                          ImGui::TableNextColumn();
                          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                          snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_REV));
                          P(CWSliderScalar("##REV",ImGuiDataType_U8,&op.dam,&_ZERO,&_SEVEN,tempID)); rightClickable

                          ImGui::TableNextColumn();


                          ImGui::EndTable();
                        }
                      }

                      ImGui::TableNextColumn();
                      op.tl&=maxTl;
                      P(CWVSliderScalar("##TL",ImVec2(ImGui::GetFrameHeight(),sliderHeight-(ins->type==DIV_INS_FM?(ImGui::GetFrameHeightWithSpacing()+ImGui::CalcTextSize(FM_SHORT_NAME(FM_AM)).y+ImGui::GetStyle().ItemSpacing.y):0.0f)),ImGuiDataType_U8,&op.tl,&maxTl,&_ZERO));

                      if (ins->type==DIV_INS_FM) {
                        CENTER_TEXT(FM_SHORT_NAME(FM_AM));
                        ImGui::TextUnformatted(FM_SHORT_NAME(FM_AM));
                        bool amOn=op.am;
                        if (ImGui::Checkbox("##AM",&amOn)) { PARAMETER
                          op.am=amOn;
                        }
                      }

                      ImGui::EndTable();
                    }
                    ImGui::PopStyleVar();

                    if (settings.separateFMColors) {
                      popAccentColors();
                    }

                    ImGui::PopID();
                  }
                  ImGui::EndTable();
                }
                ImGui::PopStyleVar();
              } else { // classic
                int columns=2;
                switch (settings.fmLayout) {
                  case 1: // 2x2
                    columns=2;
                    break;
                  case 2: // 1x4
                    columns=1;
                    break;
                  case 3: // 4x1
                    columns=opCount;
                    break;
                }
                if (ImGui::BeginTable("FMOperators",columns,ImGuiTableFlags_SizingStretchSame)) {
                  for (int i=0; i<opCount; i++) {
                    DivInstrumentFM::Operator& op=ins->fm.op[(opCount==4 && ins->type!=DIV_INS_OPL_DRUMS)?opOrder[i]:i];
                    if ((settings.fmLayout!=3 && ((i+1)&1)) || i==0 || settings.fmLayout==2) ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Separator();
                    ImGui::PushID(fmt::sprintf("op%d",i).c_str());

                    // push colors
                    if (settings.separateFMColors) {
                      bool mod=true;
                      if (ins->type==DIV_INS_OPL_DRUMS) {
                        mod=false;
                      } else if (opCount==4) {
                        if (ins->type==DIV_INS_OPL) {
                          if (opIsOutputOPL[ins->fm.alg&3][i]) mod=false;
                        } else {
                          if (opIsOutput[ins->fm.alg&7][i]) mod=false;
                        }
                      } else {
                        if (i==1 || (ins->type==DIV_INS_OPL && (ins->fm.alg&1))) mod=false;
                      }
                      if (mod) {
                        pushAccentColors(
                          uiColors[GUI_COLOR_FM_PRIMARY_MOD],
                          uiColors[GUI_COLOR_FM_SECONDARY_MOD],
                          uiColors[GUI_COLOR_FM_BORDER_MOD],
                          uiColors[GUI_COLOR_FM_BORDER_SHADOW_MOD]
                        );
                      } else {
                        pushAccentColors(
                          uiColors[GUI_COLOR_FM_PRIMARY_CAR],
                          uiColors[GUI_COLOR_FM_SECONDARY_CAR],
                          uiColors[GUI_COLOR_FM_BORDER_CAR],
                          uiColors[GUI_COLOR_FM_BORDER_SHADOW_CAR]
                        );
                      }
                    }

                    ImGui::Dummy(ImVec2(dpiScale,dpiScale));
                    if (ins->type==DIV_INS_OPL_DRUMS) {
                      ImGui::Text("%s",oplDrumNames[i]);
                    } else if (ins->type==DIV_INS_OPL && ins->fm.opllPreset==16) {
                      if (i==1) {
                        ImGui::Text("Envelope 2 (kick only)");
                      } else {
                        ImGui::Text("Envelope");
                      }
                    } else {
                      ImGui::Text("OP%d",i+1);
                    }

                    ImGui::SameLine();

                    bool amOn=op.am;
                    if (ImGui::Checkbox(FM_NAME(FM_AM),&amOn)) { PARAMETER
                      op.am=amOn;
                    }

                    int maxTl=127;
                    if (ins->type==DIV_INS_OPLL) {
                      if (i==1) {
                        maxTl=15;
                      } else {
                        maxTl=63;
                      }
                    }
                    if (ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS) {
                      maxTl=63;
                    }
                    int maxArDr=(ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ)?31:15;

                    bool ssgOn=op.ssgEnv&8;
                    bool ksrOn=op.ksr;
                    bool vibOn=op.vib;
                    bool susOn=op.sus; // don't you make fun of this one
                    unsigned char ssgEnv=op.ssgEnv&7;
                    if (ins->type!=DIV_INS_OPL && ins->type!=DIV_INS_OPL_DRUMS && ins->type!=DIV_INS_OPZ) {
                      ImGui::SameLine();
                      if (ImGui::Checkbox((ins->type==DIV_INS_OPLL)?FM_NAME(FM_EGS):"SSG On",&ssgOn)) { PARAMETER
                        op.ssgEnv=(op.ssgEnv&7)|(ssgOn<<3);
                      }
                      if (ins->type==DIV_INS_FM) {
                        if (ImGui::IsItemHovered()) {
                          ImGui::SetTooltip("Only for OPN family chips");
                        }
                      }
                    }

                    if (ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS) {
                      ImGui::SameLine();
                      if (ImGui::Checkbox(FM_NAME(FM_SUS),&susOn)) { PARAMETER
                        op.sus=susOn;
                      }
                    }

                    //52.0 controls vert scaling; default 96
                    drawFMEnv(op.tl&maxTl,op.ar&maxArDr,op.dr&maxArDr,(ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_OPLL)?((op.rr&15)*2):op.d2r&31,op.rr&15,op.sl&15,op.sus,op.ssgEnv&8,ins->fm.alg,maxTl,maxArDr,ImVec2(ImGui::GetContentRegionAvail().x,52.0*dpiScale),ins->type);
                    //P(CWSliderScalar(FM_NAME(FM_AR),ImGuiDataType_U8,&op.ar,&_ZERO,&_THIRTY_ONE)); rightClickable
                    if (ImGui::BeginTable("opParams",2,ImGuiTableFlags_SizingStretchProp)) {
                      ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.0); \
                      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed,0.0); \

                      ImGui::TableNextRow();
                      ImGui::TableNextColumn();
                      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                      op.ar&=maxArDr;
                      P(CWSliderScalar("##AR",ImGuiDataType_U8,&op.ar,&maxArDr,&_ZERO)); rightClickable
                      ImGui::TableNextColumn();
                      ImGui::Text("%s",FM_NAME(FM_AR));

                      ImGui::TableNextRow();
                      ImGui::TableNextColumn();
                      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                      op.dr&=maxArDr;
                      P(CWSliderScalar("##DR",ImGuiDataType_U8,&op.dr,&maxArDr,&_ZERO)); rightClickable
                      ImGui::TableNextColumn();
                      ImGui::Text("%s",FM_NAME(FM_DR));

                      if (settings.susPosition==0) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        P(CWSliderScalar("##SL",ImGuiDataType_U8,&op.sl,&_FIFTEEN,&_ZERO)); rightClickable
                        ImGui::TableNextColumn();
                        ImGui::Text("%s",FM_NAME(FM_SL));
                      }

                      if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        P(CWSliderScalar("##D2R",ImGuiDataType_U8,&op.d2r,&_THIRTY_ONE,&_ZERO)); rightClickable
                        ImGui::TableNextColumn();
                        ImGui::Text("%s",FM_NAME(FM_D2R));
                      }

                      ImGui::TableNextRow();
                      ImGui::TableNextColumn();
                      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                      P(CWSliderScalar("##RR",ImGuiDataType_U8,&op.rr,&_FIFTEEN,&_ZERO)); rightClickable
                      ImGui::TableNextColumn();
                      ImGui::Text("%s",FM_NAME(FM_RR));

                      if (settings.susPosition==1) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        P(CWSliderScalar("##SL",ImGuiDataType_U8,&op.sl,&_FIFTEEN,&_ZERO)); rightClickable
                        ImGui::TableNextColumn();
                        ImGui::Text("%s",FM_NAME(FM_SL));
                      }

                      ImGui::TableNextRow();
                      ImGui::TableNextColumn();
                      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                      op.tl&=maxTl;
                      P(CWSliderScalar("##TL",ImGuiDataType_U8,&op.tl,&maxTl,&_ZERO)); rightClickable
                      ImGui::TableNextColumn();
                      ImGui::Text("%s",FM_NAME(FM_TL));

                      ImGui::TableNextRow();
                      ImGui::TableNextColumn();
                      ImGui::Separator();
                      ImGui::TableNextColumn();
                      ImGui::Separator();
                      
                      ImGui::TableNextRow();
                      ImGui::TableNextColumn();
                      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                      if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ) {
                        P(CWSliderScalar("##RS",ImGuiDataType_U8,&op.rs,&_ZERO,&_THREE)); rightClickable
                        ImGui::TableNextColumn();
                        ImGui::Text("%s",FM_NAME(FM_RS));
                      } else {
                        P(CWSliderScalar("##KSL",ImGuiDataType_U8,&op.ksl,&_ZERO,&_THREE)); rightClickable
                        ImGui::TableNextColumn();
                        ImGui::Text("%s",FM_NAME(FM_KSL));
                      }

                      ImGui::TableNextRow();
                      ImGui::TableNextColumn();
                      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                      P(CWSliderScalar(FM_NAME(FM_MULT),ImGuiDataType_U8,&op.mult,&_ZERO,&_FIFTEEN)); rightClickable
                      ImGui::TableNextColumn();
                      ImGui::Text("%s",FM_NAME(FM_MULT));
                      
                      if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ) {
                        int detune=(op.dt&7)-(settings.unsignedDetune?0:3);
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        if (CWSliderInt("##DT",&detune,-3,4)) { PARAMETER
                          op.dt=detune+(settings.unsignedDetune?0:3);
                        } rightClickable
                        ImGui::TableNextColumn();
                        ImGui::Text("%s",FM_NAME(FM_DT));

                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        P(CWSliderScalar("##DT2",ImGuiDataType_U8,&op.dt2,&_ZERO,&_THREE)); rightClickable
                        if (ImGui::IsItemHovered() && ins->type==DIV_INS_FM) {
                          ImGui::SetTooltip("Only on YM2151 (OPM)");
                        }
                        ImGui::TableNextColumn();
                        ImGui::Text("%s",FM_NAME(FM_DT2));

                        if (ins->type==DIV_INS_FM) { // OPN only
                          ImGui::TableNextRow();
                          ImGui::TableNextColumn();
                          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                          if (CWSliderScalar("##SSG",ImGuiDataType_U8,&ssgEnv,&_ZERO,&_SEVEN,ssgEnvTypes[ssgEnv])) { PARAMETER
                            op.ssgEnv=(op.ssgEnv&8)|(ssgEnv&7);
                          } rightClickable
                          ImGui::TableNextColumn();
                          ImGui::Text("%s",FM_NAME(FM_SSG));
                        }
                      }

                      if (ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_OPZ) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        P(CWSliderScalar("##WS",ImGuiDataType_U8,&op.ws,&_ZERO,&_SEVEN,(ins->type==DIV_INS_OPZ)?opzWaveforms[op.ws&7]:(settings.oplStandardWaveNames?oplWaveformsStandard[op.ws&7]:oplWaveforms[op.ws&7]))); rightClickable
                        if ((ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS) && ImGui::IsItemHovered()) {
                          ImGui::SetTooltip("OPL2/3 only (last 4 waveforms are OPL3 only)");
                        }
                        ImGui::TableNextColumn();
                        ImGui::Text("%s",FM_NAME(FM_WS));
                      }
                      
                      ImGui::EndTable();
                    }

                    if (ins->type==DIV_INS_OPLL || ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS) {
                      if (ImGui::Checkbox(FM_NAME(FM_VIB),&vibOn)) { PARAMETER
                        op.vib=vibOn;
                      }
                      ImGui::SameLine();
                      if (ImGui::Checkbox(FM_NAME(FM_KSR),&ksrOn)) { PARAMETER
                        op.ksr=ksrOn;
                      }
                    }

                    if (settings.separateFMColors) {
                      popAccentColors();
                    }

                    ImGui::PopID();
                  }
                  ImGui::EndTable();
                }
              }
            }
            ImGui::EndTabItem();
          }
          if (ImGui::BeginTabItem("FM Macros")) {
            if (ins->type==DIV_INS_OPLL) {
              macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_SUS),&ins->std.algMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
              macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_FB),&ins->std.fbMacro,0,7,96,uiColors[GUI_COLOR_MACRO_OTHER]));
              macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_DC),&ins->std.fmsMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
              macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_DM),&ins->std.amsMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
            } else {
              macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_ALG),&ins->std.algMacro,0,7,96,uiColors[GUI_COLOR_MACRO_OTHER]));
              macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_FB),&ins->std.fbMacro,0,7,96,uiColors[GUI_COLOR_MACRO_OTHER]));
              if (ins->type!=DIV_INS_OPL && ins->type!=DIV_INS_OPL_DRUMS) {
                if (ins->type==DIV_INS_OPZ) {
                  // TODO: FMS2/AMS2 macros
                  macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_FMS),&ins->std.fmsMacro,0,7,96,uiColors[GUI_COLOR_MACRO_OTHER]));
                  macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_AMS),&ins->std.amsMacro,0,3,48,uiColors[GUI_COLOR_MACRO_OTHER]));
                } else {
                  macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_FMS),&ins->std.fmsMacro,0,7,96,uiColors[GUI_COLOR_MACRO_OTHER]));
                  macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_AMS),&ins->std.amsMacro,0,3,48,uiColors[GUI_COLOR_MACRO_OTHER]));
                }
              }
            }
            
            if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ) {
              macroList.push_back(FurnaceGUIMacroDesc("AM Depth",&ins->std.ex1Macro,0,127,128,uiColors[GUI_COLOR_MACRO_OTHER]));
              macroList.push_back(FurnaceGUIMacroDesc("PM Depth",&ins->std.ex2Macro,0,127,128,uiColors[GUI_COLOR_MACRO_OTHER]));
              macroList.push_back(FurnaceGUIMacroDesc("LFO Speed",&ins->std.ex3Macro,0,255,128,uiColors[GUI_COLOR_MACRO_OTHER]));
              macroList.push_back(FurnaceGUIMacroDesc("LFO Shape",&ins->std.waveMacro,0,3,48,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,macroLFOWaves));
              macroList.push_back(FurnaceGUIMacroDesc("OpMask",&ins->std.ex4Macro,0,4,128,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,fmOperatorBits));
            }
            drawMacros(macroList);
            ImGui::EndTabItem();
          }
          for (int i=0; i<opCount; i++) {
            if (ins->type==DIV_INS_OPL_DRUMS) {
              if (i>0) break;
              snprintf(label,31,"Operator Macros");
            } else {
              snprintf(label,31,"OP%d Macros",i+1);
            }
            if (ImGui::BeginTabItem(label)) {
              ImGui::PushID(i);
              int ordi=(opCount==4)?orderedOps[i]:i;
              int maxTl=127;
              if (ins->type==DIV_INS_OPLL) {
                if (i==1) {
                  maxTl=15;
                } else {
                  maxTl=63;
                }
              }
              if (ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS) {
                maxTl=63;
              }
              int maxArDr=(ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ)?31:15;

              if (ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS) {
                macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_TL),&ins->std.opMacros[ordi].tlMacro,0,maxTl,128,uiColors[GUI_COLOR_MACRO_OTHER]));
                macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_AR),&ins->std.opMacros[ordi].arMacro,0,maxArDr,64,uiColors[GUI_COLOR_MACRO_OTHER]));
                macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_DR),&ins->std.opMacros[ordi].drMacro,0,maxArDr,64,uiColors[GUI_COLOR_MACRO_OTHER]));
                macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_SL),&ins->std.opMacros[ordi].slMacro,0,15,64,uiColors[GUI_COLOR_MACRO_OTHER]));
                macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_RR),&ins->std.opMacros[ordi].rrMacro,0,15,64,uiColors[GUI_COLOR_MACRO_OTHER]));
                macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_KSL),&ins->std.opMacros[ordi].kslMacro,0,3,32,uiColors[GUI_COLOR_MACRO_OTHER]));
                macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_MULT),&ins->std.opMacros[ordi].multMacro,0,15,64,uiColors[GUI_COLOR_MACRO_OTHER]));
                macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_WS),&ins->std.opMacros[ordi].wsMacro,0,7,64,uiColors[GUI_COLOR_MACRO_OTHER]));

                macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_AM),&ins->std.opMacros[ordi].amMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
                macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_VIB),&ins->std.opMacros[ordi].vibMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
                macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_KSR),&ins->std.opMacros[ordi].ksrMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
                macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_SUS),&ins->std.opMacros[ordi].susMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
              } else if (ins->type==DIV_INS_OPLL) {
                macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_TL),&ins->std.opMacros[ordi].tlMacro,0,maxTl,128,uiColors[GUI_COLOR_MACRO_OTHER]));
                macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_AR),&ins->std.opMacros[ordi].arMacro,0,maxArDr,64,uiColors[GUI_COLOR_MACRO_OTHER]));
                macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_DR),&ins->std.opMacros[ordi].drMacro,0,maxArDr,64,uiColors[GUI_COLOR_MACRO_OTHER]));
                macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_SL),&ins->std.opMacros[ordi].slMacro,0,15,64,uiColors[GUI_COLOR_MACRO_OTHER]));
                macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_RR),&ins->std.opMacros[ordi].rrMacro,0,15,64,uiColors[GUI_COLOR_MACRO_OTHER]));
                macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_KSL),&ins->std.opMacros[ordi].kslMacro,0,3,32,uiColors[GUI_COLOR_MACRO_OTHER]));
                macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_MULT),&ins->std.opMacros[ordi].multMacro,0,15,64,uiColors[GUI_COLOR_MACRO_OTHER]));

                macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_AM),&ins->std.opMacros[ordi].amMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
                macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_VIB),&ins->std.opMacros[ordi].vibMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
                macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_KSR),&ins->std.opMacros[ordi].ksrMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
                macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_EGS),&ins->std.opMacros[ordi].egtMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
              } else {
                macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_TL),&ins->std.opMacros[ordi].tlMacro,0,maxTl,128,uiColors[GUI_COLOR_MACRO_OTHER]));
                macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_AR),&ins->std.opMacros[ordi].arMacro,0,maxArDr,64,uiColors[GUI_COLOR_MACRO_OTHER]));
                macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_DR),&ins->std.opMacros[ordi].drMacro,0,maxArDr,64,uiColors[GUI_COLOR_MACRO_OTHER]));
                macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_D2R),&ins->std.opMacros[ordi].d2rMacro,0,31,64,uiColors[GUI_COLOR_MACRO_OTHER]));
                macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_RR),&ins->std.opMacros[ordi].rrMacro,0,15,64,uiColors[GUI_COLOR_MACRO_OTHER]));
                macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_SL),&ins->std.opMacros[ordi].slMacro,0,15,64,uiColors[GUI_COLOR_MACRO_OTHER]));
                macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_RS),&ins->std.opMacros[ordi].rsMacro,0,3,32,uiColors[GUI_COLOR_MACRO_OTHER]));
                macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_MULT),&ins->std.opMacros[ordi].multMacro,0,15,64,uiColors[GUI_COLOR_MACRO_OTHER]));
                macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_DT),&ins->std.opMacros[ordi].dtMacro,0,7,64,uiColors[GUI_COLOR_MACRO_OTHER]));
                macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_DT2),&ins->std.opMacros[ordi].dt2Macro,0,3,32,uiColors[GUI_COLOR_MACRO_OTHER]));
                macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_AM),&ins->std.opMacros[ordi].amMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));

                if (ins->type==DIV_INS_FM) {
                  macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_SSG),&ins->std.opMacros[ordi].ssgMacro,0,4,64,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,ssgEnvBits));
                }
              }
              drawMacros(macroList);
              ImGui::PopID();
              ImGui::EndTabItem();
            }
          }
        }
        if (ins->type==DIV_INS_GB) if (ImGui::BeginTabItem("Game Boy")) {
          P(CWSliderScalar("Volume",ImGuiDataType_U8,&ins->gb.envVol,&_ZERO,&_FIFTEEN)); rightClickable
          P(CWSliderScalar("Envelope Length",ImGuiDataType_U8,&ins->gb.envLen,&_ZERO,&_SEVEN)); rightClickable
          P(CWSliderScalar("Sound Length",ImGuiDataType_U8,&ins->gb.soundLen,&_ZERO,&_SIXTY_FOUR,ins->gb.soundLen>63?"Infinity":"%d")); rightClickable
          ImGui::Text("Envelope Direction:");

          bool goesUp=ins->gb.envDir;
          ImGui::SameLine();
          if (ImGui::RadioButton("Up",goesUp)) { PARAMETER
            goesUp=true;
            ins->gb.envDir=goesUp;
          }
          ImGui::SameLine();
          if (ImGui::RadioButton("Down",!goesUp)) { PARAMETER
            goesUp=false;
            ins->gb.envDir=goesUp;
          }

          drawGBEnv(ins->gb.envVol,ins->gb.envLen,ins->gb.soundLen,ins->gb.envDir,ImVec2(ImGui::GetContentRegionAvail().x,100.0f*dpiScale));
          ImGui::EndTabItem();
        }
        if (ins->type==DIV_INS_C64) if (ImGui::BeginTabItem("C64")) {
          ImGui::Text("Waveform");
          ImGui::SameLine();
          ImGui::PushStyleColor(ImGuiCol_Button,TOGGLE_COLOR(ins->c64.triOn));
          if (ImGui::Button("tri")) { PARAMETER
            ins->c64.triOn=!ins->c64.triOn;
          }
          ImGui::PopStyleColor();
          ImGui::SameLine();
          ImGui::PushStyleColor(ImGuiCol_Button,TOGGLE_COLOR(ins->c64.sawOn));
          if (ImGui::Button("saw")) { PARAMETER
            ins->c64.sawOn=!ins->c64.sawOn;
          }
          ImGui::PopStyleColor();
          ImGui::SameLine();
          ImGui::PushStyleColor(ImGuiCol_Button,TOGGLE_COLOR(ins->c64.pulseOn));
          if (ImGui::Button("pulse")) { PARAMETER
            ins->c64.pulseOn=!ins->c64.pulseOn;
          }
          ImGui::PopStyleColor();
          ImGui::SameLine();
          ImGui::PushStyleColor(ImGuiCol_Button,TOGGLE_COLOR(ins->c64.noiseOn));
          if (ImGui::Button("noise")) { PARAMETER
            ins->c64.noiseOn=!ins->c64.noiseOn;
          }
          ImGui::PopStyleColor();

          ImVec2 sliderSize=ImVec2(20.0f*dpiScale,128.0*dpiScale);

          if (ImGui::BeginTable("C64EnvParams",5,ImGuiTableFlags_NoHostExtendX)) {
            ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
            ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
            ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
            ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
            ImGui::TableSetupColumn("c4",ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            CENTER_TEXT("A");
            ImGui::TextUnformatted("A");
            ImGui::TableNextColumn();
            CENTER_TEXT("D");
            ImGui::TextUnformatted("D");
            ImGui::TableNextColumn();
            CENTER_TEXT("S");
            ImGui::TextUnformatted("S");
            ImGui::TableNextColumn();
            CENTER_TEXT("R");
            ImGui::TextUnformatted("R");
            ImGui::TableNextColumn();
            CENTER_TEXT("Envelope");
            ImGui::TextUnformatted("Envelope");

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            P(CWVSliderScalar("##Attack",sliderSize,ImGuiDataType_U8,&ins->c64.a,&_ZERO,&_FIFTEEN));
            ImGui::TableNextColumn();
            P(CWVSliderScalar("##Decay",sliderSize,ImGuiDataType_U8,&ins->c64.d,&_ZERO,&_FIFTEEN));
            ImGui::TableNextColumn();
            P(CWVSliderScalar("##Sustain",sliderSize,ImGuiDataType_U8,&ins->c64.s,&_ZERO,&_FIFTEEN));
            ImGui::TableNextColumn();
            P(CWVSliderScalar("##Release",sliderSize,ImGuiDataType_U8,&ins->c64.r,&_ZERO,&_FIFTEEN));
            ImGui::TableNextColumn();
            drawFMEnv(0,16-ins->c64.a,16-ins->c64.d,15-ins->c64.r,15-ins->c64.r,15-ins->c64.s,0,0,0,15,16,ImVec2(ImGui::GetContentRegionAvail().x,sliderSize.y),ins->type);

            ImGui::EndTable();
          }

          P(CWSliderScalar("Duty",ImGuiDataType_U16,&ins->c64.duty,&_ZERO,&_FOUR_THOUSAND_NINETY_FIVE)); rightClickable

          bool ringMod=ins->c64.ringMod;
          if (ImGui::Checkbox("Ring Modulation",&ringMod)) { PARAMETER
            ins->c64.ringMod=ringMod;
          }
          bool oscSync=ins->c64.oscSync;
          if (ImGui::Checkbox("Oscillator Sync",&oscSync)) { PARAMETER
            ins->c64.oscSync=oscSync;
          }

          P(ImGui::Checkbox("Enable filter",&ins->c64.toFilter));
          P(ImGui::Checkbox("Initialize filter",&ins->c64.initFilter));
          
          P(CWSliderScalar("Cutoff",ImGuiDataType_U16,&ins->c64.cut,&_ZERO,&_TWO_THOUSAND_FORTY_SEVEN)); rightClickable
          P(CWSliderScalar("Resonance",ImGuiDataType_U8,&ins->c64.res,&_ZERO,&_FIFTEEN)); rightClickable

          ImGui::Text("Filter Mode");
          ImGui::SameLine();
          ImGui::PushStyleColor(ImGuiCol_Button,TOGGLE_COLOR(ins->c64.lp));
          if (ImGui::Button("low")) { PARAMETER
            ins->c64.lp=!ins->c64.lp;
          }
          ImGui::PopStyleColor();
          ImGui::SameLine();
          ImGui::PushStyleColor(ImGuiCol_Button,TOGGLE_COLOR(ins->c64.bp));
          if (ImGui::Button("band")) { PARAMETER
            ins->c64.bp=!ins->c64.bp;
          }
          ImGui::PopStyleColor();
          ImGui::SameLine();
          ImGui::PushStyleColor(ImGuiCol_Button,TOGGLE_COLOR(ins->c64.hp));
          if (ImGui::Button("high")) { PARAMETER
            ins->c64.hp=!ins->c64.hp;
          }
          ImGui::PopStyleColor();
          ImGui::SameLine();
          ImGui::PushStyleColor(ImGuiCol_Button,TOGGLE_COLOR(ins->c64.ch3off));
          if (ImGui::Button("ch3off")) { PARAMETER
            ins->c64.ch3off=!ins->c64.ch3off;
          }
          ImGui::PopStyleColor();

          P(ImGui::Checkbox("Volume Macro is Cutoff Macro",&ins->c64.volIsCutoff));
          P(ImGui::Checkbox("Absolute Cutoff Macro",&ins->c64.filterIsAbs));
          P(ImGui::Checkbox("Absolute Duty Macro",&ins->c64.dutyIsAbs));
          P(ImGui::Checkbox("Don't test/gate before new note",&ins->c64.noTest));
          ImGui::EndTabItem();
        }
        if (ins->type==DIV_INS_AMIGA) if (ImGui::BeginTabItem("Sample")) {
          String sName;
          if (ins->amiga.initSample<0 || ins->amiga.initSample>=e->song.sampleLen) {
            sName="none selected";
          } else {
            sName=e->song.sample[ins->amiga.initSample]->name;
          }
          if (ImGui::BeginCombo("Initial Sample",sName.c_str())) {
            String id;
            for (int i=0; i<e->song.sampleLen; i++) {
              id=fmt::sprintf("%d: %s",i,e->song.sample[i]->name);
              if (ImGui::Selectable(id.c_str(),ins->amiga.initSample==i)) { PARAMETER
                ins->amiga.initSample=i;
              }
            }
            ImGui::EndCombo();
          }
          P(ImGui::Checkbox("Use wavetable (Amiga only)",&ins->amiga.useWave));
          if (ins->amiga.useWave) {
            int len=ins->amiga.waveLen+1;
            if (ImGui::InputInt("Width",&len,2,16)) {
              if (len<2) len=2;
              if (len>256) len=256;
              ins->amiga.waveLen=(len&(~1))-1;
              PARAMETER
            }
          }
          ImGui::BeginDisabled(ins->amiga.useWave);
          P(ImGui::Checkbox("Use sample map (does not work yet!)",&ins->amiga.useNoteMap));
          if (ins->amiga.useNoteMap) {
            // TODO: frequency map?
            if (ImGui::BeginTable("NoteMap",2,ImGuiTableFlags_ScrollY|ImGuiTableFlags_Borders|ImGuiTableFlags_SizingStretchSame)) {
              ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
              ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch);
              //ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch);

              ImGui::TableSetupScrollFreeze(0,1);

              ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
              ImGui::TableNextColumn();
              ImGui::TableNextColumn();
              ImGui::Text("Sample");
              /*ImGui::TableNextColumn();
              ImGui::Text("Frequency");*/
              for (int i=0; i<120; i++) {
                DivInstrumentAmiga::SampleMap& sampleMap=ins->amiga.noteMap[i];
                ImGui::TableNextRow();
                ImGui::PushID(fmt::sprintf("NM_%d",i).c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%s",noteNames[60+i]);
                ImGui::TableNextColumn();
                if (sampleMap.map<0 || sampleMap.map>=e->song.sampleLen) {
                  sName="-- empty --";
                  sampleMap.map=-1;
                } else {
                  sName=e->song.sample[sampleMap.map]->name;
                }
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::BeginCombo("##SM",sName.c_str())) {
                  String id;
                  if (ImGui::Selectable("-- empty --",sampleMap.map==-1)) { PARAMETER
                    sampleMap.map=-1;
                  }
                  for (int j=0; j<e->song.sampleLen; j++) {
                    id=fmt::sprintf("%d: %s",j,e->song.sample[j]->name);
                    if (ImGui::Selectable(id.c_str(),sampleMap.map==j)) { PARAMETER
                      sampleMap.map=j;
                      if (sampleMap.freq<=0) sampleMap.freq=(int)((double)e->song.sample[j]->centerRate*pow(2.0,((double)i-48.0)/12.0));
                    }
                  }
                  ImGui::EndCombo();
                }
                /*ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::InputInt("##SF",&sampleMap.freq,50,500)) { PARAMETER
                  if (sampleMap.freq<0) sampleMap.freq=0;
                  if (sampleMap.freq>262144) sampleMap.freq=262144;
                }*/
                ImGui::PopID();
              }
              ImGui::EndTable();
            }
          }
          ImGui::EndDisabled();
          ImGui::EndTabItem();
        }
        if (ins->type==DIV_INS_N163) if (ImGui::BeginTabItem("Namco 163")) {
          if (ImGui::InputInt("Waveform##WAVE",&ins->n163.wave,1,10)) { PARAMETER
            if (ins->n163.wave<0) ins->n163.wave=0;
            if (ins->n163.wave>=e->song.waveLen) ins->n163.wave=e->song.waveLen-1;
          }
          if (ImGui::InputInt("Offset##WAVEPOS",&ins->n163.wavePos,1,16)) { PARAMETER
            if (ins->n163.wavePos<0) ins->n163.wavePos=0;
            if (ins->n163.wavePos>255) ins->n163.wavePos=255;
          }
          if (ImGui::InputInt("Length##WAVELEN",&ins->n163.waveLen,4,16)) { PARAMETER
            if (ins->n163.waveLen<0) ins->n163.waveLen=0;
            if (ins->n163.waveLen>252) ins->n163.waveLen=252;
            ins->n163.waveLen&=0xfc;
          }

          bool preLoad=ins->n163.waveMode&0x1;
          if (ImGui::Checkbox("Load waveform before playback",&preLoad)) { PARAMETER
            ins->n163.waveMode=(ins->n163.waveMode&~0x1)|(preLoad?0x1:0);
          }
          bool waveMode=ins->n163.waveMode&0x2;
          if (ImGui::Checkbox("Update waveforms into RAM when every waveform changes",&waveMode)) { PARAMETER
            ins->n163.waveMode=(ins->n163.waveMode&~0x2)|(waveMode?0x2:0);
          }

          ImGui::EndTabItem();
        }
        if (ins->type==DIV_INS_FDS) if (ImGui::BeginTabItem("FDS")) {
          float modTable[32];
          ImGui::Checkbox("Compatibility mode",&ins->fds.initModTableWithFirstWave);
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("only use for compatibility with .dmf modules!\n- initializes modulation table with first wavetable\n- does not alter modulation parameters on instrument change");
          }
          if (ImGui::InputInt("Modulation depth",&ins->fds.modDepth,1,32)) {
            if (ins->fds.modDepth<0) ins->fds.modDepth=0;
            if (ins->fds.modDepth>63) ins->fds.modDepth=63;
          }
          if (ImGui::InputInt("Modulation speed",&ins->fds.modSpeed,1,4)) {
            if (ins->fds.modSpeed<0) ins->fds.modSpeed=0;
            if (ins->fds.modSpeed>4095) ins->fds.modSpeed=4095;
          }
          ImGui::Text("Modulation table");
          for (int i=0; i<32; i++) {
            modTable[i]=ins->fds.modTable[i];
          }
          ImVec2 modTableSize=ImVec2(ImGui::GetContentRegionAvail().x,96.0f*dpiScale);
          PlotCustom("ModTable",modTable,32,0,NULL,-4,3,modTableSize,sizeof(float),ImVec4(1.0f,1.0f,1.0f,1.0f),0,NULL,true);
          if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            macroDragStart=ImGui::GetItemRectMin();
            macroDragAreaSize=modTableSize;
            macroDragMin=-4;
            macroDragMax=3;
            macroDragBitOff=0;
            macroDragBitMode=false;
            macroDragInitialValueSet=false;
            macroDragInitialValue=false;
            macroDragLen=32;
            macroDragActive=true;
            macroDragCTarget=(unsigned char*)ins->fds.modTable;
            macroDragChar=true;
            macroDragLineMode=false;
            macroDragLineInitial=ImVec2(0,0);
            processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
          }
          ImGui::EndTabItem();
        }
        if (ins->type==DIV_INS_MULTIPCM) {
          if (ImGui::BeginTabItem("MultiPCM")) {
            String sName;
            if (ins->amiga.initSample<0 || ins->amiga.initSample>=e->song.sampleLen) {
              sName="none selected";
            } else {
              sName=e->song.sample[ins->amiga.initSample]->name;
            }
            if (ImGui::BeginCombo("Initial Sample",sName.c_str())) {
              String id;
              for (int i=0; i<e->song.sampleLen; i++) {
                id=fmt::sprintf("%d: %s",i,e->song.sample[i]->name);
                if (ImGui::Selectable(id.c_str(),ins->amiga.initSample==i)) {
                  ins->amiga.initSample=i;
                  PARAMETER
                }
              }
              ImGui::EndCombo();
            }
            ImVec2 sliderSize=ImVec2(20.0f*dpiScale,128.0*dpiScale);
            if (ImGui::BeginTable("MultiPCMADSRParams",7,ImGuiTableFlags_NoHostExtendX)) {
              ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
              ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
              ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
              ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
              ImGui::TableSetupColumn("c4",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
              ImGui::TableSetupColumn("c5",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
              ImGui::TableSetupColumn("c6",ImGuiTableColumnFlags_WidthStretch);

              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              CENTER_TEXT("AR");
              ImGui::TextUnformatted("AR");
              if (ImGui::IsItemHovered()) {
                  ImGui::SetTooltip("Attack Rate");
              }
              ImGui::TableNextColumn();
              CENTER_TEXT("D1R");
              ImGui::TextUnformatted("D1R");
              if (ImGui::IsItemHovered()) {
                  ImGui::SetTooltip("Decay 1 Rate");
              }
              ImGui::TableNextColumn();
              CENTER_TEXT("DL");
              ImGui::TextUnformatted("DL");
              if (ImGui::IsItemHovered()) {
                  ImGui::SetTooltip("Decay Level");
              }
              ImGui::TableNextColumn();
              CENTER_TEXT("D2R");
              ImGui::TextUnformatted("D2R");
              if (ImGui::IsItemHovered()) {
                  ImGui::SetTooltip("Decay 2 Rate");
              }
              ImGui::TableNextColumn();
              CENTER_TEXT("RR");
              ImGui::TextUnformatted("RR");
              if (ImGui::IsItemHovered()) {
                  ImGui::SetTooltip("Release Rate");
              }
              ImGui::TableNextColumn();
              CENTER_TEXT("RC");
              ImGui::TextUnformatted("RC");
              if (ImGui::IsItemHovered()) {
                  ImGui::SetTooltip("Rate Correction");
              }
              ImGui::TableNextColumn();
              CENTER_TEXT("Envelope");
              ImGui::TextUnformatted("Envelope");

              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              P(CWVSliderScalar("##Attack Rate",sliderSize,ImGuiDataType_U8,&ins->multipcm.ar,&_ZERO,&_FIFTEEN));
              ImGui::TableNextColumn();
              P(CWVSliderScalar("##Decay 1 Rate",sliderSize,ImGuiDataType_U8,&ins->multipcm.d1r,&_ZERO,&_FIFTEEN));
              ImGui::TableNextColumn();
              P(CWVSliderScalar("##Decay Level",sliderSize,ImGuiDataType_U8,&ins->multipcm.dl,&_ZERO,&_FIFTEEN));
              ImGui::TableNextColumn();
              P(CWVSliderScalar("##Decay 2 Rate",sliderSize,ImGuiDataType_U8,&ins->multipcm.d2r,&_ZERO,&_FIFTEEN));
              ImGui::TableNextColumn();
              P(CWVSliderScalar("##Release Rate",sliderSize,ImGuiDataType_U8,&ins->multipcm.rr,&_ZERO,&_FIFTEEN));
              ImGui::TableNextColumn();
              P(CWVSliderScalar("##Rate Correction",sliderSize,ImGuiDataType_U8,&ins->multipcm.rc,&_ZERO,&_FIFTEEN));
              ImGui::TableNextColumn();
              drawFMEnv(0,ins->multipcm.ar,ins->multipcm.d1r,ins->multipcm.d2r,ins->multipcm.rr,ins->multipcm.dl,0,0,0,127,15,ImVec2(ImGui::GetContentRegionAvail().x,sliderSize.y),ins->type);
              ImGui::EndTable();
            }
            if (ImGui::BeginTable("MultiPCMLFOParams",3,ImGuiTableFlags_SizingStretchSame)) {
              ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.0);
              ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0);
              ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.0);
              ImGui::TableNextColumn();
              P(CWSliderScalar("LFO Rate",ImGuiDataType_U8,&ins->multipcm.lfo,&_ZERO,&_SEVEN)); rightClickable
              ImGui::TableNextColumn();
              P(CWSliderScalar("PM Depth",ImGuiDataType_U8,&ins->multipcm.vib,&_ZERO,&_SEVEN)); rightClickable
              ImGui::TableNextColumn();
              P(CWSliderScalar("AM Depth",ImGuiDataType_U8,&ins->multipcm.am,&_ZERO,&_SEVEN)); rightClickable
              ImGui::EndTable();
            }
            ImGui::EndTabItem();
          }
        }
        if (ins->type==DIV_INS_GB ||
            (ins->type==DIV_INS_AMIGA && ins->amiga.useWave) ||
            ins->type==DIV_INS_X1_010 ||
            ins->type==DIV_INS_N163 ||
            ins->type==DIV_INS_FDS ||
            ins->type==DIV_INS_SWAN ||
            ins->type==DIV_INS_PCE ||
            ins->type==DIV_INS_SCC ||
            ins->type==DIV_INS_NAMCO) {
          if (ImGui::BeginTabItem("Wavetable")) {
            if (ImGui::Checkbox("Enable synthesizer",&ins->ws.enabled)) {
              wavePreviewInit=true;
            }
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ins->ws.effect&0x80) {
              if ((ins->ws.effect&0x7f)>=DIV_WS_DUAL_MAX) {
                ins->ws.effect=0;
                wavePreviewInit=true;
              }
            } else {
              if ((ins->ws.effect&0x7f)>=DIV_WS_SINGLE_MAX) {
                ins->ws.effect=0;
                wavePreviewInit=true;
              }
            }
            if (ImGui::BeginCombo("##WSEffect",(ins->ws.effect&0x80)?dualWSEffects[ins->ws.effect&0x7f]:singleWSEffects[ins->ws.effect&0x7f])) {
              ImGui::Text("Single-waveform");
              ImGui::Indent();
              for (int i=0; i<DIV_WS_SINGLE_MAX; i++) {
                if (ImGui::Selectable(singleWSEffects[i])) {
                  ins->ws.effect=i;
                  wavePreviewInit=true;
                }
              }
              ImGui::Unindent();
              ImGui::Text("Dual-waveform");
              ImGui::Indent();
              for (int i=129; i<DIV_WS_DUAL_MAX; i++) {
                if (ImGui::Selectable(dualWSEffects[i-128])) {
                  ins->ws.effect=i;
                  wavePreviewInit=true;
                }
              }
              ImGui::Unindent();
              ImGui::EndCombo();
            }
            if (ImGui::BeginTable("WSPreview",3)) {
              DivWavetable* wave1=e->getWave(ins->ws.wave1);
              DivWavetable* wave2=e->getWave(ins->ws.wave2);
              if (wavePreviewInit) {
                wavePreview.init(ins,wavePreviewLen,wavePreviewHeight,true);
                wavePreviewInit=false;
              }
              float wavePreview1[256];
              float wavePreview2[256];
              float wavePreview3[256];
              for (int i=0; i<wave1->len; i++) {
                if (wave1->data[i]>wave1->max) {
                  wavePreview1[i]=wave1->max;
                } else {
                  wavePreview1[i]=wave1->data[i];
                }
              }
              for (int i=0; i<wave2->len; i++) {
                if (wave2->data[i]>wave2->max) {
                  wavePreview2[i]=wave2->max;
                } else {
                  wavePreview2[i]=wave2->data[i];
                }
              }
              if (ins->ws.enabled) wavePreview.tick(true);
              for (int i=0; i<wavePreviewLen; i++) {
                if (wave2->data[i]>wavePreviewHeight) {
                  wavePreview3[i]=wavePreviewHeight;
                } else {
                  wavePreview3[i]=wavePreview.output[i];
                }
              }

              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImVec2 size1=ImVec2(ImGui::GetContentRegionAvail().x,64.0f*dpiScale);
              PlotNoLerp("##WaveformP1",wavePreview1,wave1->len+1,0,"Wave 1",0,wave1->max,size1);
              ImGui::TableNextColumn();
              ImVec2 size2=ImVec2(ImGui::GetContentRegionAvail().x,64.0f*dpiScale);
              PlotNoLerp("##WaveformP2",wavePreview2,wave2->len+1,0,"Wave 2",0,wave2->max,size2);
              ImGui::TableNextColumn();
              ImVec2 size3=ImVec2(ImGui::GetContentRegionAvail().x,64.0f*dpiScale);
              PlotNoLerp("##WaveformP3",wavePreview3,wavePreviewLen,0,"Result",0,wavePreviewHeight,size3);

              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::Text("Wave 1");
              ImGui::SameLine();
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              if (ImGui::InputInt("##SelWave1",&ins->ws.wave1,1,4)) {
                if (ins->ws.wave1<0) ins->ws.wave1=0;
                if (ins->ws.wave1>=(int)e->song.wave.size()) ins->ws.wave1=e->song.wave.size()-1;
                wavePreviewInit=true;
              }
              ImGui::TableNextColumn();
              ImGui::Text("Wave 2");
              ImGui::SameLine();
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              if (ImGui::InputInt("##SelWave2",&ins->ws.wave2,1,4)) {
                if (ins->ws.wave2<0) ins->ws.wave2=0;
                if (ins->ws.wave2>=(int)e->song.wave.size()) ins->ws.wave2=e->song.wave.size()-1;
                wavePreviewInit=true;
              }
              ImGui::TableNextColumn();
              if (ImGui::Button(ICON_FA_REPEAT "##WSRestart")) {
                wavePreviewInit=true;
              }
              ImGui::SameLine();
              ImGui::Text("Preview Width");
              ImGui::SameLine();
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              if (ImGui::InputInt("##SelWave3",&wavePreviewLen,1,4)) {
                if (wavePreviewLen<1) wavePreviewLen=1;
                if (wavePreviewLen>256) wavePreviewLen=256;
                wavePreviewInit=true;
              }
              ImGui::EndTable();
            }

            if (ImGui::InputScalar("Update Rate",ImGuiDataType_U8,&ins->ws.rateDivider,&_ONE,&_SEVEN)) {
              wavePreviewInit=true;
            }
            int speed=ins->ws.speed+1;
            if (ImGui::InputInt("Speed",&speed,1,16)) {
              if (speed<1) speed=1;
              if (speed>256) speed=256;
              ins->ws.speed=speed-1;
              wavePreviewInit=true;
            }

            if (ImGui::InputScalar("Amount",ImGuiDataType_U8,&ins->ws.param1,&_ONE,&_SEVEN)) {
              wavePreviewInit=true;
            }

            if (ins->ws.effect==DIV_WS_PHASE_MOD) {
              if (ImGui::InputScalar("Power",ImGuiDataType_U8,&ins->ws.param2,&_ONE,&_SEVEN)) {
                wavePreviewInit=true;
              }
            }

            if (ImGui::Checkbox("Global",&ins->ws.global)) {
              wavePreviewInit=true;
            }

            ImGui::EndTabItem();
          }
        }
        if (ImGui::BeginTabItem("Macros")) {
          const char* volumeLabel="Volume";

          int volMax=15;
          int volMin=0;
          if (ins->type==DIV_INS_C64) {
            if (ins->c64.volIsCutoff) {
              volumeLabel="Cutoff";
              if (ins->c64.filterIsAbs) {
                volMax=2047;
              } else {
                volMin=-64;
                volMax=64;
              }
            }
          }
          if ((ins->type==DIV_INS_PCE || ins->type==DIV_INS_AY8930)) {
            volMax=31;
          }
          if (ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_VERA || ins->type==DIV_INS_VRC6_SAW) {
            volMax=63;
          }
          if (ins->type==DIV_INS_AMIGA) {
            volMax=64;
          }
          if (ins->type==DIV_INS_FM || ins->type==DIV_INS_MIKEY || ins->type==DIV_INS_MULTIPCM || ins->type==DIV_INS_SU) {
            volMax=127;
          }
          if (ins->type==DIV_INS_GB) {
            volMax=0;
          }
          if (ins->type==DIV_INS_PET || ins->type==DIV_INS_BEEPER) {
            volMax=1;
          }
          if (ins->type==DIV_INS_FDS) {
            volMax=32;
          }

          const char* dutyLabel="Duty/Noise";
          int dutyMin=0;
          int dutyMax=3;
          if (ins->type==DIV_INS_C64) {
            dutyLabel="Duty";
            if (ins->c64.dutyIsAbs) {
              dutyMax=4095;
            } else {
              dutyMin=-96;
              dutyMax=96;
            }
          }
          if (ins->type==DIV_INS_FM) {
            dutyMax=32;
          }
          if ((ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930)) {
            dutyMax=31;
          }
          if (ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930 || ins->type==DIV_INS_FM) {
            dutyLabel="Noise Freq";
          }
          if (ins->type==DIV_INS_MIKEY) {
            dutyLabel="Duty/Int";
            dutyMax=10;
          }
          if (ins->type==DIV_INS_BEEPER) {
            dutyLabel="Pulse Width";
            dutyMax=255;
          }
          if (ins->type==DIV_INS_AY8930) {
            dutyMax=255;
          }
          if (ins->type==DIV_INS_TIA || ins->type==DIV_INS_AMIGA || ins->type==DIV_INS_SCC || ins->type==DIV_INS_PET || ins->type==DIV_INS_VIC) {
            dutyMax=0;
          }
          if (ins->type==DIV_INS_PCE || ins->type==DIV_INS_NAMCO) {
            dutyLabel="Noise";
            dutyMax=1;
          }
          if (ins->type==DIV_INS_SWAN) {
            dutyLabel="Noise";
            dutyMax=8;
          }
          if (ins->type==DIV_INS_OPLL || ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_VRC6_SAW || ins->type==DIV_INS_FDS || ins->type==DIV_INS_MULTIPCM) {
            dutyMax=0;
          }
          if (ins->type==DIV_INS_VERA) {
            dutyLabel="Duty";
            dutyMax=63;
          }
          if (ins->type==DIV_INS_N163) {
            dutyLabel="Waveform pos.";
            dutyMax=255;
          }
          if (ins->type==DIV_INS_VRC6) {
            dutyLabel="Duty";
            dutyMax=7;
          }
          if (ins->type==DIV_INS_SU) {
            dutyMax=127;
          }

          const char* waveLabel="Waveform";
          int waveMax=(ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930 || ins->type==DIV_INS_VERA)?3:255;
          bool bitMode=false;
          if (ins->type==DIV_INS_C64 || ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930 || ins->type==DIV_INS_SAA1099) {
            bitMode=true;
          }
          if (ins->type==DIV_INS_STD || ins->type==DIV_INS_VRC6 || ins->type==DIV_INS_VRC6_SAW) waveMax=0;
          if (ins->type==DIV_INS_TIA || ins->type==DIV_INS_VIC || ins->type==DIV_INS_OPLL) waveMax=15;
          if (ins->type==DIV_INS_C64) waveMax=4;
          if (ins->type==DIV_INS_SAA1099) waveMax=2;
          if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_OPZ) waveMax=0;
          if (ins->type==DIV_INS_MIKEY) waveMax=0;
          if (ins->type==DIV_INS_MULTIPCM) waveMax=0;
          if (ins->type==DIV_INS_SU) waveMax=7;
          if (ins->type==DIV_INS_PET) {
            waveMax=8;
            bitMode=true;
          }

          if (ins->type==DIV_INS_OPLL) {
            waveLabel="Patch";
          }

          const char** waveNames=NULL;
          if (ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930 || ins->type==DIV_INS_SAA1099) waveNames=ayShapeBits;
          if (ins->type==DIV_INS_C64) waveNames=c64ShapeBits;

          int ex1Max=(ins->type==DIV_INS_AY8930)?8:0;
          int ex2Max=(ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930)?4:0;
          bool ex2Bit=true;

          if (ins->type==DIV_INS_C64) {
            ex1Max=4;
            ex2Max=15;
          }
          if (ins->type==DIV_INS_X1_010) {
            dutyMax=0;
            ex1Max=7;
            ex2Max=255;
            ex2Bit=false;
          }
          if (ins->type==DIV_INS_N163) {
            ex1Max=252;
            ex2Max=2;
          }
          if (ins->type==DIV_INS_FDS) {
            ex1Max=63;
            ex2Max=4095;
          }
          if (ins->type==DIV_INS_SU) {
            ex1Max=16383;
            ex2Max=255;
          }
          if (ins->type==DIV_INS_SAA1099) ex1Max=8;

          int panMin=0;
          int panMax=0;
          bool panSingle=false;
          bool panSingleNoBit=false;
          if (ins->type==DIV_INS_STD ||//Game Gear
              ins->type==DIV_INS_FM ||
              ins->type==DIV_INS_OPL ||
              ins->type==DIV_INS_OPL_DRUMS ||
              ins->type==DIV_INS_GB ||
              ins->type==DIV_INS_OPZ ||
              ins->type==DIV_INS_VERA) {
            panMax=1;
            panSingle=true;
          }
          if (ins->type==DIV_INS_AMIGA) {
            panMax=127;
          }
          if (ins->type==DIV_INS_X1_010 || ins->type==DIV_INS_PCE || ins->type==DIV_INS_MIKEY || ins->type==DIV_INS_SAA1099 || ins->type==DIV_INS_NAMCO) {
            panMax=15;
          }
          if (ins->type==DIV_INS_AMIGA && ins->std.panLMacro.mode) {
            panMin=-16;
            panMax=16;
          }
          if (ins->type==DIV_INS_MULTIPCM) {
            panMin=-7;
            panMax=7;
            panSingleNoBit=true;
          }
          if (ins->type==DIV_INS_SU) {
            panMin=-127;
            panMax=127;
            panSingleNoBit=true;
          }

          if (volMax>0) {
            macroList.push_back(FurnaceGUIMacroDesc(volumeLabel,&ins->std.volMacro,volMin,volMax,160,uiColors[GUI_COLOR_MACRO_VOLUME]));
          }
          macroList.push_back(FurnaceGUIMacroDesc("Arpeggio",&ins->std.arpMacro,-120,120,160,uiColors[GUI_COLOR_MACRO_PITCH],true,macroAbsoluteMode,ins->std.arpMacro.mode?(&macroHoverNote):NULL));
          if (dutyMax>0) {
            if (ins->type==DIV_INS_MIKEY) {
              macroList.push_back(FurnaceGUIMacroDesc(dutyLabel,&ins->std.dutyMacro,0,dutyMax,160,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,mikeyFeedbackBits));
            } else {
              macroList.push_back(FurnaceGUIMacroDesc(dutyLabel,&ins->std.dutyMacro,dutyMin,dutyMax,160,uiColors[GUI_COLOR_MACRO_OTHER]));
            }
          }
          if (waveMax>0) {
            macroList.push_back(FurnaceGUIMacroDesc(waveLabel,&ins->std.waveMacro,0,waveMax,(bitMode && ins->type!=DIV_INS_PET)?64:160,uiColors[GUI_COLOR_MACRO_WAVE],false,NULL,NULL,bitMode,waveNames,((ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930)?1:0)));
          }
          if (panMax>0) {
            if (panSingle) {
              macroList.push_back(FurnaceGUIMacroDesc("Panning",&ins->std.panLMacro,0,2,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,panBits));
            } else {
              if (panSingleNoBit || (ins->type==DIV_INS_AMIGA && ins->std.panLMacro.mode)) {
                macroList.push_back(FurnaceGUIMacroDesc("Panning",&ins->std.panLMacro,panMin,panMax,(31+panMax-panMin),uiColors[GUI_COLOR_MACRO_OTHER],false,(ins->type==DIV_INS_AMIGA)?macroQSoundMode:NULL));
              } else {
                macroList.push_back(FurnaceGUIMacroDesc("Panning (left)",&ins->std.panLMacro,panMin,panMax,(31+panMax-panMin),uiColors[GUI_COLOR_MACRO_OTHER],false,(ins->type==DIV_INS_AMIGA)?macroQSoundMode:NULL));
              }
              if (!panSingleNoBit) {
                if (ins->type==DIV_INS_AMIGA && ins->std.panLMacro.mode) {
                  macroList.push_back(FurnaceGUIMacroDesc("Surround",&ins->std.panRMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
                } else {
                  macroList.push_back(FurnaceGUIMacroDesc("Panning (right)",&ins->std.panRMacro,panMin,panMax,(31+panMax-panMin),uiColors[GUI_COLOR_MACRO_OTHER]));
                }
              }
            }
          }
          macroList.push_back(FurnaceGUIMacroDesc("Pitch",&ins->std.pitchMacro,-2048,2047,160,uiColors[GUI_COLOR_MACRO_PITCH],true,macroRelativeMode));
          if (ins->type==DIV_INS_FM ||
              ins->type==DIV_INS_STD ||
              ins->type==DIV_INS_OPL ||
              ins->type==DIV_INS_OPL_DRUMS ||
              ins->type==DIV_INS_OPZ ||
              ins->type==DIV_INS_PCE ||
              ins->type==DIV_INS_GB ||
              ins->type==DIV_INS_AMIGA ||
              ins->type==DIV_INS_OPLL ||
              ins->type==DIV_INS_AY ||
              ins->type==DIV_INS_AY8930 ||
              ins->type==DIV_INS_SWAN ||
              ins->type==DIV_INS_MULTIPCM ||
              ins->type==DIV_INS_SU ||
              ins->type==DIV_INS_MIKEY) {
            macroList.push_back(FurnaceGUIMacroDesc("Phase Reset",&ins->std.phaseResetMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
          }
          if (ex1Max>0) {
            if (ins->type==DIV_INS_C64) {
              macroList.push_back(FurnaceGUIMacroDesc("Filter Mode",&ins->std.ex1Macro,0,ex1Max,64,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,filtModeBits));
            } else if (ins->type==DIV_INS_SAA1099) {
              macroList.push_back(FurnaceGUIMacroDesc("Envelope",&ins->std.ex1Macro,0,ex1Max,160,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,saaEnvBits));
            } else if (ins->type==DIV_INS_X1_010) {
              macroList.push_back(FurnaceGUIMacroDesc("Envelope Mode",&ins->std.ex1Macro,0,ex1Max,160,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,x1_010EnvBits));
            } else if (ins->type==DIV_INS_N163) {
              macroList.push_back(FurnaceGUIMacroDesc("Wave Length",&ins->std.ex1Macro,0,ex1Max,160,uiColors[GUI_COLOR_MACRO_OTHER]));
            } else if (ins->type==DIV_INS_FDS) {
              macroList.push_back(FurnaceGUIMacroDesc("Mod Depth",&ins->std.ex1Macro,0,ex1Max,160,uiColors[GUI_COLOR_MACRO_OTHER]));
            } else if (ins->type==DIV_INS_SU) {
              macroList.push_back(FurnaceGUIMacroDesc("Cutoff",&ins->std.ex1Macro,0,ex1Max,160,uiColors[GUI_COLOR_MACRO_OTHER]));
            } else {
              macroList.push_back(FurnaceGUIMacroDesc("Duty",&ins->std.ex1Macro,0,ex1Max,160,uiColors[GUI_COLOR_MACRO_OTHER]));
            }
          }
          if (ex2Max>0) {
            if (ins->type==DIV_INS_C64) {
              macroList.push_back(FurnaceGUIMacroDesc("Resonance",&ins->std.ex2Macro,0,ex2Max,64,uiColors[GUI_COLOR_MACRO_OTHER]));
            } else if (ins->type==DIV_INS_N163) {
              macroList.push_back(FurnaceGUIMacroDesc("Wave Update",&ins->std.ex2Macro,0,ex2Max,64,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,n163UpdateBits));
            } else if (ins->type==DIV_INS_FDS) {
              macroList.push_back(FurnaceGUIMacroDesc("Mod Speed",&ins->std.ex2Macro,0,ex2Max,160,uiColors[GUI_COLOR_MACRO_OTHER]));
            } else if (ins->type==DIV_INS_SU) {
              macroList.push_back(FurnaceGUIMacroDesc("Resonance",&ins->std.ex2Macro,0,ex2Max,160,uiColors[GUI_COLOR_MACRO_OTHER]));
            } else {
              macroList.push_back(FurnaceGUIMacroDesc("Envelope",&ins->std.ex2Macro,0,ex2Max,ex2Bit?64:160,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,ex2Bit,ayEnvBits));
            }
          }
          if (ins->type==DIV_INS_C64) {
            macroList.push_back(FurnaceGUIMacroDesc("Special",&ins->std.ex3Macro,0,2,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,c64SpecialBits));
            macroList.push_back(FurnaceGUIMacroDesc("Test/Gate",&ins->std.ex4Macro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
          }
          if (ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930 || ins->type==DIV_INS_X1_010) {
            macroList.push_back(FurnaceGUIMacroDesc("AutoEnv Num",&ins->std.ex3Macro,0,15,160,uiColors[GUI_COLOR_MACRO_OTHER]));
            macroList.push_back(FurnaceGUIMacroDesc("AutoEnv Den",&ins->std.algMacro,0,15,160,uiColors[GUI_COLOR_MACRO_OTHER]));
          }
          if (ins->type==DIV_INS_AY8930) {
            // oh my i am running out of macros
            macroList.push_back(FurnaceGUIMacroDesc("Noise AND Mask",&ins->std.fbMacro,0,8,96,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
            macroList.push_back(FurnaceGUIMacroDesc("Noise OR Mask",&ins->std.fmsMacro,0,8,96,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
          }
          if (ins->type==DIV_INS_N163) {
            macroList.push_back(FurnaceGUIMacroDesc("WaveLoad Wave",&ins->std.ex3Macro,0,255,160,uiColors[GUI_COLOR_MACRO_OTHER]));
            macroList.push_back(FurnaceGUIMacroDesc("WaveLoad Pos",&ins->std.algMacro,0,255,160,uiColors[GUI_COLOR_MACRO_OTHER]));
            macroList.push_back(FurnaceGUIMacroDesc("WaveLoad Len",&ins->std.fbMacro,0,252,160,uiColors[GUI_COLOR_MACRO_OTHER]));
            macroList.push_back(FurnaceGUIMacroDesc("WaveLoad Trigger",&ins->std.fmsMacro,0,2,160,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,n163UpdateBits));
          }
          if (ins->type==DIV_INS_FDS) {
            macroList.push_back(FurnaceGUIMacroDesc("Mod Position",&ins->std.ex3Macro,0,127,160,uiColors[GUI_COLOR_MACRO_OTHER]));
          }
          if (ins->type==DIV_INS_SU) {
            macroList.push_back(FurnaceGUIMacroDesc("Control",&ins->std.ex3Macro,0,4,64,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,suControlBits));
          }

          drawMacros(macroList);
          ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
      }
      if (settings.insEditColorize) {
        popAccentColors();
      }
    }
    if (displayMacroMenu) {
      displayMacroMenu=false;
      if (lastMacroDesc.macro!=NULL) {
        ImGui::OpenPopup("macroMenu");
      }
    }
    if (ImGui::BeginPopup("macroMenu",ImGuiWindowFlags_NoMove|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoSavedSettings)) {
      if (ImGui::MenuItem("copy")) {
        String mmlStr;
        encodeMMLStr(mmlStr,lastMacroDesc.macro->val,lastMacroDesc.macro->len,lastMacroDesc.macro->loop,lastMacroDesc.macro->rel);
        SDL_SetClipboardText(mmlStr.c_str());
      }
      if (ImGui::MenuItem("paste")) {
        String mmlStr;
        char* clipText=SDL_GetClipboardText();
        if (clipText!=NULL) {
          if (clipText[0]) {
            mmlStr=clipText;
          }
          SDL_free(clipText);
        }
        if (!mmlStr.empty()) {
          decodeMMLStr(mmlStr,lastMacroDesc.macro->val,lastMacroDesc.macro->len,lastMacroDesc.macro->loop,lastMacroDesc.min,(lastMacroDesc.isBitfield)?((1<<(lastMacroDesc.isBitfield?lastMacroDesc.max:0))-1):lastMacroDesc.max,lastMacroDesc.macro->rel);
        }
      }
      ImGui::Separator();
      if (ImGui::MenuItem("clear")) {
        lastMacroDesc.macro->len=0;
        lastMacroDesc.macro->loop=-1;
        lastMacroDesc.macro->rel=-1;
        for (int i=0; i<256; i++) {
          lastMacroDesc.macro->val[i]=0;
        }
      }
      if (ImGui::MenuItem("clear contents")) {
        for (int i=0; i<256; i++) {
          lastMacroDesc.macro->val[i]=0;
        }
      }
      ImGui::Separator();
      if (ImGui::BeginMenu("offset...")) {
        ImGui::InputInt("X",&macroOffX,1,10);
        ImGui::InputInt("Y",&macroOffY,1,10);
        if (ImGui::Button("offset")) {
          int oldData[256];
          memset(oldData,0,256*sizeof(int));
          memcpy(oldData,lastMacroDesc.macro->val,lastMacroDesc.macro->len*sizeof(int));

          for (int i=0; i<lastMacroDesc.macro->len; i++) {
            int val=0;
            if ((i-macroOffX)>=0 && (i-macroOffX)<lastMacroDesc.macro->len) {
              val=oldData[i-macroOffX]+macroOffY;
              if (val<lastMacroDesc.min) val=lastMacroDesc.min;
              if (val>lastMacroDesc.max) val=lastMacroDesc.max;
            }
            lastMacroDesc.macro->val[i]=val;
          }

          if (lastMacroDesc.macro->loop>=0 && lastMacroDesc.macro->loop<lastMacroDesc.macro->len) {
            lastMacroDesc.macro->loop+=macroOffX;
          } else {
            lastMacroDesc.macro->loop=-1;
          }
          if ((lastMacroDesc.macro->rel+macroOffX)>=0 && (lastMacroDesc.macro->rel+macroOffX)<lastMacroDesc.macro->len) {
            lastMacroDesc.macro->rel+=macroOffX;
          } else {
            lastMacroDesc.macro->rel=-1;
          }

          ImGui::CloseCurrentPopup();
        }
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("scale...")) {
        if (ImGui::InputFloat("X",&macroScaleX,1.0f,10.0f,"%.2f%%")) {
          if (macroScaleX<0.1) macroScaleX=0.1;
          if (macroScaleX>12800.0) macroScaleX=12800.0;
        }
        ImGui::InputFloat("Y",&macroScaleY,1.0f,10.0f,"%.2f%%");
        if (ImGui::Button("scale")) {
          int oldData[256];
          memset(oldData,0,256*sizeof(int));
          memcpy(oldData,lastMacroDesc.macro->val,lastMacroDesc.macro->len*sizeof(int));

          lastMacroDesc.macro->len=MIN(128,((double)lastMacroDesc.macro->len*(macroScaleX/100.0)));

          for (int i=0; i<lastMacroDesc.macro->len; i++) {
            int val=0;
            double posX=round((double)i*(100.0/macroScaleX)-0.01);
            if (posX>=0 && posX<lastMacroDesc.macro->len) {
              val=round((double)oldData[(int)posX]*(macroScaleY/100.0));
              if (val<lastMacroDesc.min) val=lastMacroDesc.min;
              if (val>lastMacroDesc.max) val=lastMacroDesc.max;
            }
            lastMacroDesc.macro->val[i]=val;
          }

          ImGui::CloseCurrentPopup();
        }
        ImGui::EndMenu();
      }
      
      ImGui::EndPopup();
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_INS_EDIT;
  ImGui::End();
}
