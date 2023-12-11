/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2023 tildearrow and contributors
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

#define _USE_MATH_DEFINES
#include "gui.h"
#include "../ta-log.h"
#include "imgui_internal.h"
#include "../engine/macroInt.h"
#include "IconsFontAwesome4.h"
#include "furIcons.h"
#include "misc/cpp/imgui_stdlib.h"
#include "guiConst.h"
#include "intConst.h"
#include <fmt/printf.h>
#include <imgui.h>
#include "plot_nolerp.h"

extern "C" {
#include "../../extern/Nuked-OPLL/opll.h"
}

const char* ssgEnvTypes[8]={
  "Down Down Down", "Down.", "Down Up Down Up", "Down UP", "Up Up Up", "Up.", "Up Down Up Down", "Up DOWN"
};

const char* fmParamNames[3][32]={
  {"Algorithm", "Feedback", "LFO > Freq", "LFO > Amp", "Attack", "Decay", "Decay 2", "Release", "Sustain", "Level", "EnvScale", "Multiplier", "Detune", "Detune 2", "SSG-EG", "AM", "AM Depth", "Vibrato Depth", "Sustained", "Sustained", "Level Scaling", "Sustain", "Vibrato", "Waveform", "Scale Rate", "OP2 Half Sine", "OP1 Half Sine", "EnvShift", "Reverb", "Fine", "LFO2 > Freq", "LFO2 > Amp"},
  {"ALG", "FB", "FMS/PMS", "AMS", "AR", "DR", "SR", "RR", "SL", "TL", "KS", "MULT", "DT", "DT2", "SSG-EG", "AM", "AMD", "FMD", "EGT", "EGT", "KSL", "SUS", "VIB", "WS", "KSR", "DC", "DM", "EGS", "REV", "Fine", "FMS/PMS2", "AMS2"},
  {"ALG", "FB", "FMS/PMS", "AMS", "AR", "DR", "D2R", "RR", "SL", "TL", "RS", "MULT", "DT", "DT2", "SSG-EG", "AM", "DAM", "DVB", "EGT", "EGS", "KSL", "SUS", "VIB", "WS", "KSR", "DC", "DM", "EGS", "REV", "Fine", "FMS/PMS2", "AMS2"}
};

const char* fmParamShortNames[3][32]={
  {"ALG", "FB", "FMS", "AMS", "A", "D", "D2", "R", "S", "TL", "RS", "ML", "DT", "DT2", "SSG", "AM", "DAM", "DVB", "SUS", "SUS", "KSL", "SUS", "VIB", "WS", "KSR", "DC", "DM", "EGS", "REV", "Fine", "FMS2", "AMS2"},
  {"ALG", "FB", "FMS", "AMS", "A", "D", "SR", "R", "S", "TL", "KS", "ML", "DT", "DT2", "SSG", "AM", "AMD", "FMD", "EGT", "EGT", "KSL", "SUS", "VIB", "WS", "KSR", "DC", "DM", "EGS", "REV", "Fine", "FMS2", "AMS2"},
  {"ALG", "FB", "FMS", "AMS", "A", "D", "D2", "R", "S", "TL", "RS", "ML", "DT", "DT2", "SSG", "AM", "DAM", "DVB", "EGT", "EGS", "KSL", "SUS", "VIB", "WS", "KSR", "DC", "DM", "EGS", "REV", "Fine", "FMS2", "AMS2"}
};

const char* opllVariants[4]={
  "OPLL",
  "YMF281",
  "YM2423",
  "VRC7"
};

const char* opllInsNames[4][17]={
  /* YM2413 */ {
    "User",
    "1. Violin",
    "2. Guitar",
    "3. Piano",
    "4. Flute",
    "5. Clarinet",
    "6. Oboe",
    "7. Trumpet",
    "8. Organ",
    "9. Horn",
    "10. Synth",
    "11. Harpsichord",
    "12. Vibraphone",
    "13. Synth Bass",
    "14. Acoustic Bass",
    "15. Electric Guitar",
    "Drums"
  },
  /* YMF281 */ {
    "User",
    "1. Electric String",
    "2. Bow wow",
    "3. Electric Guitar",
    "4. Organ",
    "5. Clarinet",
    "6. Saxophone",
    "7. Trumpet",
    "8. Street Organ",
    "9. Synth Brass",
    "10. Electric Piano",
    "11. Bass",
    "12. Vibraphone",
    "13. Chime",
    "14. Tom Tom II",
    "15. Noise",
    "Drums"
  },
  /* YM2423 */ {
    "User",
    "1. Strings",
    "2. Guitar",
    "3. Electric Guitar",
    "4. Electric Piano",
    "5. Flute",
    "6. Marimba",
    "7. Trumpet",
    "8. Harmonica",
    "9. Tuba",
    "10. Synth Brass",
    "11. Short Saw",
    "12. Vibraphone",
    "13. Electric Guitar 2",
    "14. Synth Bass",
    "15. Sitar",
    "Drums"
  },
  // stolen from FamiTracker
  /* VRC7 */ {
    "User",
    "1. Bell",
    "2. Guitar",
    "3. Piano",
    "4. Flute",
    "5. Clarinet",
    "6. Rattling Bell",
    "7. Trumpet",
    "8. Reed Organ",
    "9. Soft Bell",
    "10. Xylophone",
    "11. Vibraphone",
    "12. Brass",
    "13. Bass Guitar",
    "14. Synth",
    "15. Chorus",
    "Drums"
  }
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

const char* macroTypeLabels[4]={
  ICON_FA_BAR_CHART "##IMacroType",
  ICON_FUR_ADSR "##IMacroType",
  ICON_FUR_TRI "##IMacroType",
  ICON_FA_SIGN_OUT "##IMacroType"
};

const char* macroLFOShapes[4]={
  "Triangle", "Saw", "Square", "How did you even"
};

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

const char* snesModeBits[6]={
  "noise", "echo", "pitch mod", "invert right", "invert left", NULL
};

const char* filtModeBits[5]={
  "low", "band", "high", "ch3off", NULL
};

const char* c64TestGateBits[5]={
  "gate", "sync", "ring", "test", NULL
};

const char* pokeyCtlBits[9]={
  "15KHz", "filter 2+4", "filter 1+3", "16-bit 3+4", "16-bit 1+2", "high3", "high1", "poly9", NULL
};

const char* mikeyFeedbackBits[11] = {
  "0", "1", "2", "3", "4", "5", "7", "10", "11", "int", NULL
};

const char* msm5232ControlBits[7]={
  "16'", "8'", "4'", "2'", "sustain", NULL
};

const char* tedControlBits[3]={
  "square", "noise", NULL
};

const char* c219ControlBits[4]={
  "noise", "invert", "surround", NULL
};

const char* x1_010EnvBits[8]={
  "enable", "oneshot", "split L/R", "HinvR", "VinvR", "HinvL", "VinvL", NULL
};

/*const char* n163UpdateBits[8]={
  "now", "every waveform changed", NULL
};*/

const char* suControlBits[5]={
  "ring mod", "low pass", "high pass", "band pass", NULL
};

const char* es5506FilterModes[4]={
  "HP/K2, HP/K2", "HP/K2, LP/K1", "LP/K2, LP/K2", "LP/K2, LP/K1",
};

const char* panBits[5]={
  "right", "left", "rear right", "rear left", NULL
};

const char* oneBit[2]={
  "on", NULL
};

const char* es5506EnvelopeModes[3]={
  "k1 slowdown", "k2 slowdown", NULL
};

const char* es5506ControlModes[3]={
  "pause", "reverse", NULL
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

const char* gbHWSeqCmdTypes[6]={
  "Envelope",
  "Sweep",
  "Wait",
  "Wait for Release",
  "Loop",
  "Loop until Release"
};

const char* suHWSeqCmdTypes[7]={
  "Volume Sweep",
  "Frequency Sweep",
  "Cutoff Sweep",
  "Wait",
  "Wait for Release",
  "Loop",
  "Loop until Release"
};

const char* snesGainModes[5]={
  "Direct",
  "Decrease (linear)",
  "Decrease (logarithmic)",
  "Increase (linear)",
  "Increase (bent line)"
};

const int detuneMap[2][8]={
  {-3, -2, -1, 0, 1, 2, 3, 4},
  { 7,  6,  5, 0, 1, 2, 3, 4}
};

const int detuneUnmap[2][11]={
  {0, 1, 2, 3, 4, 5, 6, 7, 0, 0, 0},
  {0, 0, 0, 3, 4, 5, 6, 7, 2, 1, 0}
};

const int kslMap[4]={
  0, 2, 1, 3
};

// do not change these!
// anything other than a checkbox will look ugly!
//
// if you really need to, and have a good rationale (and by good I mean a VERY
// good one), please tell me and we'll sort it out.
const char* macroAbsoluteMode="Fixed";
const char* macroRelativeMode="Relative";
const char* macroQSoundMode="QSound";
const char* macroDummyMode="Bug";

String macroHoverNote(int id, float val, void* u) {
  int* macroVal=(int*)u;
  if ((macroVal[id]&0xc0000000)==0x40000000 || (macroVal[id]&0xc0000000)==0x80000000) {
    if (val<-60 || val>=120) return "???";
    return fmt::sprintf("%d: %s",id,noteNames[(int)val+60]);
  }
  return fmt::sprintf("%d: %d",id,(int)val);
}

String macroHover(int id, float val, void* u) {
  return fmt::sprintf("%d: %d",id,(int)val);
}

String macroHoverLoop(int id, float val, void* u) {
  if (val>1) return "Release";
  if (val>0) return "Loop";
  return "";
}

String macroHoverBit30(int id, float val, void* u) {
  if (val>0) return "Fixed";
  return "Relative";
}

String macroHoverGain(int id, float val, void* u) {
  if (val>=224.0f) {
    return fmt::sprintf("%d: +%d (exponential)",id,(int)(val-224));
  }
  if (val>=192.0f) {
    return fmt::sprintf("%d: +%d (linear)",id,(int)(val-192));
  }
  if (val>=160.0f) {
    return fmt::sprintf("%d: -%d (exponential)",id,(int)(val-160));
  }
  if (val>=128.0f) {
    return fmt::sprintf("%d: -%d (linear)",id,(int)(val-128));
  }
  return fmt::sprintf("%d: %d (direct)",id,(int)val);
}

String macroHoverES5506FilterMode(int id, float val, void* u) {
  String mode="???";
  switch (((int)val)&3) {
    case 0:
      mode="HP/K2, HP/K2";
      break;
    case 1:
      mode="HP/K2, LP/K1";
      break;
    case 2:
      mode="LP/K2, LP/K2";
      break;
    case 3:
      mode="LP/K2, LP/K1";
      break;
    default:
      break;
  }
  return fmt::sprintf("%d: %s",id,mode);
}

String macroLFOWaves(int id, float val, void* u) {
  switch (((int)val)&3) {
    case 0:
      return "Saw";
    case 1:
      return "Square";
    case 2:
      return "Triangle";
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

void FurnaceGUI::drawFMEnv(unsigned char tl, unsigned char ar, unsigned char dr, unsigned char d2r, unsigned char rr, unsigned char sl, unsigned char sus, unsigned char egt, unsigned char algOrGlobalSus, float maxTl, float maxArDr, float maxRr, const ImVec2& size, unsigned short instType) {
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
    float rrPos=(float(maxRr-rr)/float(maxRr)); //end of RR

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
    } else if (d2r==0.0 || ((instType==DIV_INS_OPL || instType==DIV_INS_SNES) && sus==1.0) || (instType==DIV_INS_OPLL && egt!=0.0)) { //envelope stays at the sustain level forever
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
  updateFMPreview=true; \
}

#define PARAMETER MARK_MODIFIED; e->notifyInsChange(curIns); updateFMPreview=true;

String genericGuide(float value) {
  return fmt::sprintf("%d",(int)value);
}

inline int deBit30(const int val) {
  if ((val&0xc0000000)==0x40000000 || (val&0xc0000000)==0x80000000) return val^0x40000000;
  return val;
}

inline bool enBit30(const int val) {
  if ((val&0xc0000000)==0x40000000 || (val&0xc0000000)==0x80000000) return true;
  return false;
}


void FurnaceGUI::kvsConfig(DivInstrument* ins, bool supportsKVS) {
  if (fmPreviewOn) {
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("left click to restart\nmiddle click to pause\nright click to see algorithm");
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
      updateFMPreview=true;
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) {
      fmPreviewPaused=!fmPreviewPaused;
    }
  } else if (supportsKVS) {
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("left click to configure TL scaling\nright click to see FM preview");
    }
  } else {
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("right click to see FM preview");
    }
  }
  if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
    fmPreviewOn=!fmPreviewOn;
  }
  if (ImGui::IsItemHovered() && CHECK_LONG_HOLD) {
    NOTIFY_LONG_HOLD;
    fmPreviewOn=!fmPreviewOn;
  }
  if (!fmPreviewOn && supportsKVS) {
    int opCount=4;
    if (ins->type==DIV_INS_OPLL) opCount=2;
    if (ins->type==DIV_INS_OPL) opCount=(ins->fm.ops==4)?4:2;
    if (ImGui::BeginPopupContextItem("IKVSOpt",ImGuiPopupFlags_MouseButtonLeft)) {
      ImGui::Text("operator level changes with volume?");
      if (ImGui::BeginTable("KVSTable",4,ImGuiTableFlags_BordersInner)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthStretch);
        for (int i=0; i<4; i++) {
          int o=(opCount==4)?orderedOps[i]:i;
          if (!(i&1)) ImGui::TableNextRow();
          const char* label="AUTO##OPKVS";
          if (ins->fm.op[o].kvs==0) {
            label="NO##OPKVS";
          } else if (ins->fm.op[o].kvs==1) {
            label="YES##OPKVS";
          }
          ImGui::TableNextColumn();
          ImGui::Text("%d",i+1);
          ImGui::TableNextColumn();
          ImGui::PushID(o);
          if (ImGui::Button(label,ImVec2(ImGui::GetContentRegionAvail().x,0.0f))) {
            if (++ins->fm.op[o].kvs>2) ins->fm.op[o].kvs=0;
            PARAMETER;
          }
          ImGui::PopID();
        }
        ImGui::EndTable();
      }
      ImGui::EndPopup();
    }
  }
}

void FurnaceGUI::drawFMPreview(const ImVec2& size) {
  float asFloat[FM_PREVIEW_SIZE];
  for (int i=0; i<FM_PREVIEW_SIZE; i++) {
    asFloat[i]=(float)fmPreview[i]/8192.0f;
  }
  ImGui::PlotLines("##DebugFMPreview",asFloat,FM_PREVIEW_SIZE,0,NULL,-1.0,1.0,size);
}

void FurnaceGUI::drawMacroEdit(FurnaceGUIMacroDesc& i, int totalFit, float availableWidth, int index) {
  static float asFloat[256];
  static int asInt[256];
  static float loopIndicator[256];
  static float bit30Indicator[256];
  static bool doHighlight[256];

  if ((i.macro->open&6)==0) {
    for (int j=0; j<256; j++) {
      bit30Indicator[j]=0;
      if (j+macroDragScroll>=i.macro->len) {
        asFloat[j]=0;
        asInt[j]=0;
      } else {
        asFloat[j]=deBit30(i.macro->val[j+macroDragScroll]);
        asInt[j]=deBit30(i.macro->val[j+macroDragScroll])+i.bitOffset;
        if (i.bit30) bit30Indicator[j]=enBit30(i.macro->val[j+macroDragScroll]);
      }
      if (j+macroDragScroll>=i.macro->len || (j+macroDragScroll>i.macro->rel && i.macro->loop<i.macro->rel)) {
        loopIndicator[j]=0;
      } else {
        loopIndicator[j]=((i.macro->loop!=255 && (j+macroDragScroll)>=i.macro->loop))|((i.macro->rel!=255 && (j+macroDragScroll)==i.macro->rel)<<1);
      }
    }
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(0.0f,0.0f));

    if (i.macro->vZoom<1) {
      if (i.macro->macroType==DIV_MACRO_ARP) {
        i.macro->vZoom=24;
        i.macro->vScroll=120-12;
      } else if (i.macro->macroType==DIV_MACRO_PITCH) {
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

    memset(doHighlight,0,256*sizeof(bool));
    if (e->isRunning()) for (int j=0; j<e->getTotalChannelCount(); j++) {
      DivChannelState* chanState=e->getChanState(j);
      if (chanState==NULL) continue;

      if (chanState->keyOff) continue;
      if (chanState->lastIns!=curIns) continue;

      DivMacroInt* macroInt=e->getMacroInt(j);
      if (macroInt==NULL) continue;

      DivMacroStruct* macroStruct=macroInt->structByType(i.macro->macroType);
      if (macroStruct==NULL) continue;

      if (macroStruct->lastPos>i.macro->len) continue;
      if (macroStruct->lastPos<macroDragScroll) continue;
      if (macroStruct->lastPos>255) continue;
      if (!macroStruct->actualHad) continue;

      doHighlight[macroStruct->lastPos-macroDragScroll]=true;
    }

    if (i.isBitfield) {
      PlotBitfield("##IMacro",asInt,totalFit,0,i.bitfieldBits,i.max,ImVec2(availableWidth,(i.macro->open&1)?(i.height*dpiScale):(32.0f*dpiScale)),sizeof(float),doHighlight);
    } else {
      PlotCustom("##IMacro",asFloat,totalFit,macroDragScroll,NULL,i.min+i.macro->vScroll,i.min+i.macro->vScroll+i.macro->vZoom,ImVec2(availableWidth,(i.macro->open&1)?(i.height*dpiScale):(32.0f*dpiScale)),sizeof(float),i.color,i.macro->len-macroDragScroll,i.hoverFunc,i.hoverFuncUser,i.blockMode,(i.macro->open&1)?genericGuide:NULL,doHighlight);
    }
    if ((i.macro->open&1) && (ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right))) {
      ImGui::InhibitInertialScroll();
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
      macroDragBit30=i.bit30;
      macroDragSettingBit30=false;
      macroDragTarget=i.macro->val;
      macroDragChar=false;
      macroDragLineMode=(i.isBitfield)?false:ImGui::IsItemClicked(ImGuiMouseButton_Right);
      macroDragLineInitial=ImVec2(0,0);
      lastMacroDesc=i;
      processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
    }
    if ((i.macro->open&1)) {
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
          if (ImGui::VSliderInt("##IMacroVScroll",ImVec2(20.0f*dpiScale,i.height*dpiScale),&i.macro->vScroll,0,(i.max-i.min)-i.macro->vZoom,"",ImGuiSliderFlags_NoInput)) {
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

          ImGuiID scrollbarID=ImGui::GetID("##IMacroVScroll");
          ImGui::KeepAliveID(scrollbarID);
          if (ImGui::ScrollbarEx(scrollbarPos,scrollbarID,ImGuiAxis_Y,&scrollV,availV,contentsV,0)) {
            i.macro->vScroll=(i.max-i.min-i.macro->vZoom)-scrollV;
          }
        }
      }

      // bit 30 area
      if (i.bit30) {
        PlotCustom("##IMacroBit30",bit30Indicator,totalFit,macroDragScroll,NULL,0,1,ImVec2(availableWidth,12.0f*dpiScale),sizeof(float),i.color,i.macro->len-macroDragScroll,&macroHoverBit30);
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
          ImGui::InhibitInertialScroll();
          macroDragStart=ImGui::GetItemRectMin();
          macroDragAreaSize=ImVec2(availableWidth,12.0f*dpiScale);
          macroDragInitialValueSet=false;
          macroDragInitialValue=false;
          macroDragLen=totalFit;
          macroDragActive=true;
          macroDragBit30=i.bit30;
          macroDragSettingBit30=true;
          macroDragTarget=i.macro->val;
          macroDragChar=false;
          macroDragLineMode=false;
          macroDragLineInitial=ImVec2(0,0);
          lastMacroDesc=i;
          processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
        }
      }

      // loop area
      PlotCustom("##IMacroLoop",loopIndicator,totalFit,macroDragScroll,NULL,0,2,ImVec2(availableWidth,12.0f*dpiScale),sizeof(float),i.color,i.macro->len-macroDragScroll,&macroHoverLoop);
      if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        ImGui::InhibitInertialScroll();
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
        ImGui::InhibitInertialScroll();
        if (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift)) {
          i.macro->rel=255;
        } else {
          i.macro->loop=255;
        }
      }
      ImGui::SetNextItemWidth(availableWidth);
      String& mmlStr=mmlString[index];
      if (ImGui::InputText("##IMacroMML",&mmlStr)) {
        decodeMMLStr(mmlStr,i.macro->val,i.macro->len,i.macro->loop,i.min,(i.isBitfield)?((1<<(i.isBitfield?i.max:0))-1):i.max,i.macro->rel,i.bit30);
      }
      if (!ImGui::IsItemActive()) {
        encodeMMLStr(mmlStr,i.macro->val,i.macro->len,i.macro->loop,i.macro->rel,false,i.bit30);
      }
    }
    ImGui::PopStyleVar();
  } else {
    if (i.macro->open&2) {
      if (ImGui::BeginTable("MacroADSR",4)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.3);
        ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthStretch,0.3);
        //ImGui::TableSetupColumn("c4",ImGuiTableColumnFlags_WidthStretch,0.4);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Bottom");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputInt("##MABottom",&i.macro->val[0],1,16)) { PARAMETER
          if (i.macro->val[0]<i.min) i.macro->val[0]=i.min;
          if (i.macro->val[0]>i.max) i.macro->val[0]=i.max;
        }

        ImGui::TableNextColumn();
        ImGui::Text("Top");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputInt("##MATop",&i.macro->val[1],1,16)) { PARAMETER
          if (i.macro->val[1]<i.min) i.macro->val[1]=i.min;
          if (i.macro->val[1]>i.max) i.macro->val[1]=i.max;
        }

        /*ImGui::TableNextColumn();
        ImGui::Text("the envelope goes here");*/

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Attack");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (CWSliderInt("##MAAR",&i.macro->val[2],0,255)) { PARAMETER
          if (i.macro->val[2]<0) i.macro->val[2]=0;
          if (i.macro->val[2]>255) i.macro->val[2]=255;
        } rightClickable

        ImGui::TableNextColumn();
        ImGui::Text("Sustain");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (CWSliderInt("##MASL",&i.macro->val[5],0,255)) { PARAMETER
          if (i.macro->val[5]<0) i.macro->val[5]=0;
          if (i.macro->val[5]>255) i.macro->val[5]=255;
        } rightClickable

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Hold");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (CWSliderInt("##MAHT",&i.macro->val[3],0,255)) { PARAMETER
          if (i.macro->val[3]<0) i.macro->val[3]=0;
          if (i.macro->val[3]>255) i.macro->val[3]=255;
        } rightClickable

        ImGui::TableNextColumn();
        ImGui::Text("SusTime");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (CWSliderInt("##MAST",&i.macro->val[6],0,255)) { PARAMETER
          if (i.macro->val[6]<0) i.macro->val[6]=0;
          if (i.macro->val[6]>255) i.macro->val[6]=255;
        } rightClickable

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Decay");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (CWSliderInt("##MADR",&i.macro->val[4],0,255)) { PARAMETER
          if (i.macro->val[4]<0) i.macro->val[4]=0;
          if (i.macro->val[4]>255) i.macro->val[4]=255;
        } rightClickable

        ImGui::TableNextColumn();
        ImGui::Text("SusDecay");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (CWSliderInt("##MASR",&i.macro->val[7],0,255)) { PARAMETER
          if (i.macro->val[7]<0) i.macro->val[7]=0;
          if (i.macro->val[7]>255) i.macro->val[7]=255;
        } rightClickable

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Release");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (CWSliderInt("##MARR",&i.macro->val[8],0,255)) { PARAMETER
          if (i.macro->val[8]<0) i.macro->val[8]=0;
          if (i.macro->val[8]>255) i.macro->val[8]=255;
        } rightClickable

        ImGui::EndTable();
      }
    }
    if (i.macro->open&4) {
      if (ImGui::BeginTable("MacroLFO",4)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.3);
        ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthStretch,0.3);
        //ImGui::TableSetupColumn("c4",ImGuiTableColumnFlags_WidthStretch,0.4);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Bottom");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputInt("##MABottom",&i.macro->val[0],1,16)) { PARAMETER
          if (i.macro->val[0]<i.min) i.macro->val[0]=i.min;
          if (i.macro->val[0]>i.max) i.macro->val[0]=i.max;
        }

        ImGui::TableNextColumn();
        ImGui::Text("Top");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputInt("##MATop",&i.macro->val[1],1,16)) { PARAMETER
          if (i.macro->val[1]<i.min) i.macro->val[1]=i.min;
          if (i.macro->val[1]>i.max) i.macro->val[1]=i.max;
        }

        /*ImGui::TableNextColumn();
        ImGui::Text("the envelope goes here");*/

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Speed");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (CWSliderInt("##MLSpeed",&i.macro->val[11],0,255)) { PARAMETER
          if (i.macro->val[11]<0) i.macro->val[11]=0;
          if (i.macro->val[11]>255) i.macro->val[11]=255;
        } rightClickable

        ImGui::TableNextColumn();
        ImGui::Text("Phase");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (CWSliderInt("##MLPhase",&i.macro->val[13],0,1023)) { PARAMETER
          if (i.macro->val[13]<0) i.macro->val[13]=0;
          if (i.macro->val[13]>1023) i.macro->val[13]=1023;
        } rightClickable

        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Shape");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (CWSliderInt("##MLShape",&i.macro->val[12],0,2,macroLFOShapes[i.macro->val[12]&3])) { PARAMETER
          if (i.macro->val[12]<0) i.macro->val[12]=0;
          if (i.macro->val[12]>2) i.macro->val[12]=2;
        } rightClickable

        ImGui::EndTable();
      }
    }
  }
}

#define BUTTON_TO_SET_MODE(buttonType) \
  if (buttonType(macroTypeLabels[(i.macro->open>>1)&3])) { \
    unsigned char prevOpen=i.macro->open; \
    if (i.macro->open>=4) { \
      i.macro->open&=(~6); \
    } else { \
      i.macro->open+=2; \
    } \
\
    /* check whether macro type is now ADSR/LFO or sequence */ \
    if (((prevOpen&6)?1:0)!=((i.macro->open&6)?1:0)) { \
      /* swap memory */ \
      /* this way the macro isn't corrupted if the user decides to go */ \
      /* back to sequence mode */ \
      i.macro->len^=i.macro->lenMemory; \
      i.macro->lenMemory^=i.macro->len; \
      i.macro->len^=i.macro->lenMemory; \
\
      for (int j=0; j<16; j++) { \
        i.macro->val[j]^=i.macro->typeMemory[j]; \
        i.macro->typeMemory[j]^=i.macro->val[j]; \
        i.macro->val[j]^=i.macro->typeMemory[j]; \
      } \
\
      /* if ADSR/LFO, populate min/max */ \
      if (i.macro->open&6) { \
        if (i.macro->val[0]==0 && i.macro->val[1]==0) { \
          i.macro->val[0]=i.min; \
          i.macro->val[1]=i.max; \
        } \
        i.macro->val[0]=CLAMP(i.macro->val[0],i.min,i.max); \
        i.macro->val[1]=CLAMP(i.macro->val[1],i.min,i.max); \
      } \
    } \
    PARAMETER; \
  } \
  if (ImGui::IsItemHovered()) { \
    switch (i.macro->open&6) { \
      case 0: \
        ImGui::SetTooltip("Macro type: Sequence"); \
        break; \
      case 2: \
        ImGui::SetTooltip("Macro type: ADSR"); \
        break; \
      case 4: \
        ImGui::SetTooltip("Macro type: LFO"); \
        break; \
      default: \
        ImGui::SetTooltip("Macro type: What's going on here?"); \
        break; \
    } \
  } \
  if (i.macro->open&6) { \
    i.macro->len=16; \
  }

#define BUTTON_TO_SET_PROPS(_x) \
  pushToggleColors(_x.macro->speed!=1 || _x.macro->delay); \
  ImGui::Button(ICON_FA_ELLIPSIS_H "##IMacroSet"); \
  popToggleColors(); \
  if (ImGui::IsItemHovered()) { \
    ImGui::SetTooltip("Delay/Step Length"); \
  } \
  if (ImGui::BeginPopupContextItem("IMacroSetP",ImGuiPopupFlags_MouseButtonLeft)) { \
    if (ImGui::InputScalar("Step Length (ticks)##IMacroSpeed",ImGuiDataType_U8,&_x.macro->speed,&_ONE,&_THREE)) { \
      if (_x.macro->speed<1) _x.macro->speed=1; \
      MARK_MODIFIED; \
    } \
    if (ImGui::InputScalar("Delay##IMacroDelay",ImGuiDataType_U8,&_x.macro->delay,&_ONE,&_THREE)) { \
      MARK_MODIFIED; \
    } \
    ImGui::EndPopup(); \
  }

#define BUTTON_TO_SET_RELEASE(buttonType) \
  pushToggleColors(i.macro->open&8); \
  if (buttonType(ICON_FA_BOLT "##IMacroRelMode")) { \
    i.macro->open^=8; \
  } \
  if (ImGui::IsItemHovered()) { \
    if (i.macro->open&8) { \
      ImGui::SetTooltip("Release mode: Active (jump to release pos)"); \
    } else { \
      ImGui::SetTooltip("Release mode: Passive (delayed release)"); \
    } \
  } \
  popToggleColors(); \

void FurnaceGUI::drawMacros(std::vector<FurnaceGUIMacroDesc>& macros, FurnaceGUIMacroEditState& state) {
  int index=0;
  float reservedSpace=(settings.oldMacroVSlider)?(20.0f*dpiScale+ImGui::GetStyle().ItemSpacing.x):ImGui::GetStyle().ScrollbarSize;
  switch (settings.macroLayout) {
    case 0: {
      if (ImGui::BeginTable("MacroSpace",2)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed,0.0);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        float lenAvail=ImGui::GetContentRegionAvail().x;
        //ImGui::Dummy(ImVec2(120.0f*dpiScale,dpiScale));
        ImGui::SetNextItemWidth(120.0f*dpiScale);
        if (ImGui::InputInt("##MacroPointSize",&macroPointSize,1,4)) {
          if (macroPointSize<1) macroPointSize=1;
          if (macroPointSize>256) macroPointSize=256;
        }
        ImGui::TableNextColumn();
        float availableWidth=ImGui::GetContentRegionAvail().x-reservedSpace;
        int totalFit=MIN(255,availableWidth/MAX(1,macroPointSize*dpiScale));
        int scrollMax=0;
        for (FurnaceGUIMacroDesc& i: macros) {
          if (i.macro->len>scrollMax) scrollMax=i.macro->len;
        }
        scrollMax-=totalFit;
        if (scrollMax<0) scrollMax=0;
        if (macroDragScroll>scrollMax) {
          macroDragScroll=scrollMax;
        }
        ImGui::BeginDisabled(scrollMax<1);
        ImGui::SetNextItemWidth(availableWidth);
        if (CWSliderInt("##MacroScroll",&macroDragScroll,0,scrollMax,"")) {
          if (macroDragScroll<0) macroDragScroll=0;
          if (macroDragScroll>scrollMax) macroDragScroll=scrollMax;
        }
        ImGui::EndDisabled();

        // draw macros
        for (FurnaceGUIMacroDesc& i: macros) {
          ImGui::PushID(index);
          ImGui::TableNextRow();

          // description
          ImGui::TableNextColumn();
          ImGui::Text("%s",i.displayName);
          ImGui::SameLine();
          if (ImGui::SmallButton((i.macro->open&1)?(ICON_FA_CHEVRON_UP "##IMacroOpen"):(ICON_FA_CHEVRON_DOWN "##IMacroOpen"))) {
            i.macro->open^=1;
          }
          if (i.macro->open&1) {
            if ((i.macro->open&6)==0) {
              ImGui::SetNextItemWidth(lenAvail);
              int macroLen=i.macro->len;
              if (ImGui::InputScalar("##IMacroLen",ImGuiDataType_U8,&macroLen,&_ONE,&_THREE)) { MARK_MODIFIED
                if (macroLen<0) macroLen=0;
                if (macroLen>255) macroLen=255;
                i.macro->len=macroLen;
              }
            }
            BUTTON_TO_SET_MODE(ImGui::Button);
            ImGui::SameLine();
            BUTTON_TO_SET_PROPS(i);
            if ((i.macro->open&6)==0) {
              ImGui::SameLine();
              BUTTON_TO_SET_RELEASE(ImGui::Button);
            }
            // do not change this!
            // anything other than a checkbox will look ugly!
            // if you really need more than two macro modes please tell me.
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
          drawMacroEdit(i,totalFit,availableWidth,index);
          ImGui::PopID();
          index++;
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        ImGui::BeginDisabled(scrollMax<1);
        ImGui::SetNextItemWidth(availableWidth);
        if (CWSliderInt("##MacroScroll",&macroDragScroll,0,scrollMax,"")) {
          if (macroDragScroll<0) macroDragScroll=0;
          if (macroDragScroll>scrollMax) macroDragScroll=scrollMax;
        }
        ImGui::EndDisabled();
        ImGui::EndTable();
      }
      break;
    }
    case 1: {
      ImGui::Text("Tabs");
      break;
    }
    case 2: {
      int columns=round(ImGui::GetContentRegionAvail().x/(400.0*dpiScale));
      int curColumn=0;
      if (columns<1) columns=1;
      if (ImGui::BeginTable("MacroGrid",columns,ImGuiTableFlags_BordersInner)) {
        for (FurnaceGUIMacroDesc& i: macros) {
          if (curColumn==0) ImGui::TableNextRow();
          ImGui::TableNextColumn();

          if (++curColumn>=columns) curColumn=0;
          
          float availableWidth=ImGui::GetContentRegionAvail().x-reservedSpace;
          int totalFit=i.macro->len;
          if (totalFit<1) totalFit=1;

          ImGui::PushID(index);

          ImGui::TextUnformatted(i.displayName);
          ImGui::SameLine();
          if (ImGui::SmallButton((i.macro->open&1)?(ICON_FA_CHEVRON_UP "##IMacroOpen"):(ICON_FA_CHEVRON_DOWN "##IMacroOpen"))) {
            i.macro->open^=1;
          }

          if (i.macro->open&1) {
            ImGui::SameLine();
            BUTTON_TO_SET_MODE(ImGui::SmallButton);
          }

          drawMacroEdit(i,totalFit,availableWidth,index);

          if (i.macro->open&1) {
            if ((i.macro->open&6)==0) {
              ImGui::Text("Length");
              ImGui::SameLine();
              ImGui::SetNextItemWidth(120.0f*dpiScale);
              int macroLen=i.macro->len;
              if (ImGui::InputScalar("##IMacroLen",ImGuiDataType_U8,&macroLen,&_ONE,&_THREE)) { MARK_MODIFIED
                if (macroLen<0) macroLen=0;
                if (macroLen>255) macroLen=255;
                i.macro->len=macroLen;
              }
              ImGui::SameLine();
            }
            BUTTON_TO_SET_PROPS(i);
            if ((i.macro->open&6)==0) {
              ImGui::SameLine();
              BUTTON_TO_SET_RELEASE(ImGui::Button);
            }
            if (i.modeName!=NULL) {
              bool modeVal=i.macro->mode;
              String modeName=fmt::sprintf("%s##IMacroMode",i.modeName);
              ImGui::SameLine();
              if (ImGui::Checkbox(modeName.c_str(),&modeVal)) {
                i.macro->mode=modeVal;
              }
            }
          }

          ImGui::PopID();
          index++;
        }
        ImGui::EndTable();
      }
      break;
    }
    case 3: {
      if (ImGui::BeginTable("MacroList",2,ImGuiTableFlags_Borders)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        for (size_t i=0; i<macros.size(); i++) {
          if (ImGui::Selectable(macros[i].displayName,state.selectedMacro==(int)i)) {
            state.selectedMacro=i;
          }
        }

        ImGui::TableNextColumn();
        float availableWidth=ImGui::GetContentRegionAvail().x-reservedSpace;
        int totalFit=MIN(255,availableWidth/MAX(1,macroPointSize*dpiScale));
        if (macroDragScroll>255-totalFit) {
          macroDragScroll=255-totalFit;
        }

        if (state.selectedMacro<0 || state.selectedMacro>=(int)macros.size()) {
          state.selectedMacro=0;
        }

        if (state.selectedMacro>=0 && state.selectedMacro<(int)macros.size()) {
          FurnaceGUIMacroDesc& m=macros[state.selectedMacro];
          m.macro->open|=1;

          float availableWidth=ImGui::GetContentRegionAvail().x-reservedSpace;
          int totalFit=MIN(255,availableWidth/MAX(1,macroPointSize*dpiScale));
          int scrollMax=0;
          for (FurnaceGUIMacroDesc& i: macros) {
            if (i.macro->len>scrollMax) scrollMax=i.macro->len;
          }
          scrollMax-=totalFit;
          if (scrollMax<0) scrollMax=0;
          if (macroDragScroll>scrollMax) {
            macroDragScroll=scrollMax;
          }
          ImGui::BeginDisabled(scrollMax<1);
          ImGui::SetNextItemWidth(availableWidth);
          if (CWSliderInt("##MacroScroll",&macroDragScroll,0,scrollMax,"")) {
            if (macroDragScroll<0) macroDragScroll=0;
            if (macroDragScroll>scrollMax) macroDragScroll=scrollMax;
          }
          ImGui::EndDisabled();

          ImGui::SameLine();
          ImGui::Button(ICON_FA_SEARCH_PLUS "##MacroZoomB");
          if (ImGui::BeginPopupContextItem("MacroZoomP",ImGuiPopupFlags_MouseButtonLeft)) {
            ImGui::SetNextItemWidth(120.0f*dpiScale);
            if (ImGui::InputInt("##MacroPointSize",&macroPointSize,1,4)) {
              if (macroPointSize<1) macroPointSize=1;
              if (macroPointSize>256) macroPointSize=256;
            }
            ImGui::EndPopup();
          }

          m.height=ImGui::GetContentRegionAvail().y-ImGui::GetFontSize()-ImGui::GetFrameHeightWithSpacing()-(m.bit30?28.0f:12.0f)*dpiScale-ImGui::GetStyle().ItemSpacing.y*3.0f;
          if (m.height<10.0f*dpiScale) m.height=10.0f*dpiScale;
          m.height/=dpiScale;
          drawMacroEdit(m,totalFit,availableWidth,index);

          if (m.macro->open&1) {
            if ((m.macro->open&6)==0) {
              ImGui::Text("Length");
              ImGui::SameLine();
              ImGui::SetNextItemWidth(120.0f*dpiScale);
              int macroLen=m.macro->len;
              if (ImGui::InputScalar("##IMacroLen",ImGuiDataType_U8,&macroLen,&_ONE,&_THREE)) { MARK_MODIFIED
                if (macroLen<0) macroLen=0;
                if (macroLen>255) macroLen=255;
                m.macro->len=macroLen;
              }
              ImGui::SameLine();
            }
            ImGui::Text("StepLen");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(120.0f*dpiScale);
            if (ImGui::InputScalar("##IMacroSpeed",ImGuiDataType_U8,&m.macro->speed,&_ONE,&_THREE)) {
              if (m.macro->speed<1) m.macro->speed=1;
              MARK_MODIFIED;
            }
            ImGui::SameLine();
            ImGui::Text("Delay");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(120.0f*dpiScale);
            if (ImGui::InputScalar("##IMacroDelay",ImGuiDataType_U8,&m.macro->delay,&_ONE,&_THREE)) {
              MARK_MODIFIED;
            }
            ImGui::SameLine();
            {
              FurnaceGUIMacroDesc& i=m;
              BUTTON_TO_SET_MODE(ImGui::Button);
              if ((i.macro->open&6)==0) {
                ImGui::SameLine();
                BUTTON_TO_SET_RELEASE(ImGui::Button);
              }
            }
            if (m.modeName!=NULL) {
              bool modeVal=m.macro->mode;
              String modeName=fmt::sprintf("%s##IMacroMode",m.modeName);
              ImGui::SameLine();
              if (ImGui::Checkbox(modeName.c_str(),&modeVal)) {
                m.macro->mode=modeVal;
              }
            }
          } else {
            ImGui::Text("The heck? No, this isn't even working correctly...");
          }
        } else {
          ImGui::Text("The only problem with that selectedMacro is that it's a bug...");
        }

        // goes here
        ImGui::EndTable();
      }
      break;
    }
    case 4: {
      ImGui::Text("Single (combo box)");
      break;
    }
  }
}

void FurnaceGUI::alterSampleMap(int column, int val) {
  if (curIns<0 || curIns>=(int)e->song.ins.size()) return;
  DivInstrument* ins=e->song.ins[curIns];
  int sampleMapMin=sampleMapSelStart;
  int sampleMapMax=sampleMapSelEnd;
  if (sampleMapMin>sampleMapMax) {
    sampleMapMin^=sampleMapMax;
    sampleMapMax^=sampleMapMin;
    sampleMapMin^=sampleMapMax;
  }

  for (int i=sampleMapMin; i<=sampleMapMax; i++) {
    if (i<0 || i>=120) continue;

    if (sampleMapColumn==1 && column==1) {
      ins->amiga.noteMap[i].freq=val;
    } else if (sampleMapColumn==0 && column==0) {
      if (val<0) {
        ins->amiga.noteMap[i].map=-1;
      } else if (sampleMapDigit>0) {
        ins->amiga.noteMap[i].map*=10;
        ins->amiga.noteMap[i].map+=val;
      } else {
        ins->amiga.noteMap[i].map=val;
      }
      if (ins->amiga.noteMap[i].map>=(int)e->song.sample.size()) {
        ins->amiga.noteMap[i].map=((int)e->song.sample.size())-1;
      }
    } else if (sampleMapColumn==2 && column==2) {
      if (val<0) {
        ins->amiga.noteMap[i].dpcmFreq=-1;
      } else if (sampleMapDigit>0) {
        ins->amiga.noteMap[i].dpcmFreq*=10;
        ins->amiga.noteMap[i].dpcmFreq+=val;
      } else {
        ins->amiga.noteMap[i].dpcmFreq=val;
      }
      if (ins->amiga.noteMap[i].dpcmFreq>15) {
        ins->amiga.noteMap[i].dpcmFreq%=10;
      }
    } else if (sampleMapColumn==3 && column==3) {
      if (val<0) {
        ins->amiga.noteMap[i].dpcmDelta=-1;
      } else if (sampleMapDigit>0) {
        if (ins->amiga.noteMap[i].dpcmDelta>7) {

          ins->amiga.noteMap[i].dpcmDelta=val;
        } else {
          ins->amiga.noteMap[i].dpcmDelta<<=4;
          ins->amiga.noteMap[i].dpcmDelta+=val;
        }
      } else {
        ins->amiga.noteMap[i].dpcmDelta=val;
      }
    }
  }

  bool advance=false;
  if (sampleMapColumn==1 && column==1) {
    advance=true;
  } else if (sampleMapColumn==0 && column==0) {
    int digits=1;
    if (e->song.sample.size()>=10) digits=2;
    if (e->song.sample.size()>=100) digits=3;
    if (++sampleMapDigit>=digits) {
      sampleMapDigit=0;
      advance=true;
    }
  } else if (sampleMapColumn==2 && column==2) {
    if (++sampleMapDigit>=2) {
      sampleMapDigit=0;
      advance=true;
    }
  } else if (sampleMapColumn==3 && column==3) {
    if (++sampleMapDigit>=2) {
      sampleMapDigit=0;
      advance=true;
    }
  }

  if (advance && sampleMapMin==sampleMapMax) {
    sampleMapSelStart++;
    if (sampleMapSelStart>119) sampleMapSelStart=119;
    sampleMapSelEnd=sampleMapSelStart;
  }

  MARK_MODIFIED;
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
  if (ImGui::InputInt(df,&fNum,1,16)) { \
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

#define TOOLTIP_TEXT(text) \
  if (ImGui::IsItemHovered()) { \
    ImGui::SetTooltip("%s", text); \
  }

#define OP_DRAG_POINT \
  if (ImGui::Button(ICON_FA_ARROWS)) { \
  } \
  if (ImGui::BeginDragDropSource()) { \
    opToMove=i; \
    ImGui::SetDragDropPayload("FUR_OP",NULL,0,ImGuiCond_Once); \
    ImGui::Button(ICON_FA_ARROWS "##SysDrag"); \
    ImGui::SameLine(); \
    if (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift)) { \
      ImGui::Text("(copying)"); \
    } else { \
      ImGui::Text("(swapping)"); \
    } \
    ImGui::EndDragDropSource(); \
  } else if (ImGui::IsItemHovered()) { \
    ImGui::SetTooltip("- drag to swap operator\n- shift-drag to copy operator"); \
  } \
  if (ImGui::BeginDragDropTarget()) { \
    const ImGuiPayload* dragItem=ImGui::AcceptDragDropPayload("FUR_OP"); \
    if (dragItem!=NULL) { \
      if (dragItem->IsDataType("FUR_OP")) { \
        if (opToMove!=i && opToMove>=0) { \
          int destOp=(opCount==4 && ins->type!=DIV_INS_OPL_DRUMS)?opOrder[i]:i; \
          int sourceOp=(opCount==4 && ins->type!=DIV_INS_OPL_DRUMS)?opOrder[opToMove]:opToMove; \
          if (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift)) { \
            e->lockEngine([ins,destOp,sourceOp]() { \
              ins->fm.op[destOp]=ins->fm.op[sourceOp]; \
            }); \
          } else { \
            e->lockEngine([ins,destOp,sourceOp]() { \
              DivInstrumentFM::Operator origOp=ins->fm.op[sourceOp]; \
              ins->fm.op[sourceOp]=ins->fm.op[destOp]; \
              ins->fm.op[destOp]=origOp; \
            }); \
          } \
          PARAMETER; \
        } \
        opToMove=-1; \
      } \
    } \
    ImGui::EndDragDropTarget(); \
  }

void FurnaceGUI::insTabSample(DivInstrument* ins) {
  const char* sampleTabName="Sample";
  if (ins->type==DIV_INS_NES) sampleTabName="DPCM";
  if (ImGui::BeginTabItem(sampleTabName)) {
    if (ins->type==DIV_INS_NES && e->song.oldDPCM) {
      ImGui::Text("new DPCM features disabled (compatibility)!");
      if (ImGui::Button("click here to enable them.")) {
        e->song.oldDPCM=false;
        MARK_MODIFIED;
      }
      ImGui::EndTabItem();
      return;
    }

    String sName;
    bool wannaOpenSMPopup=false;
    if (ins->amiga.initSample<0 || ins->amiga.initSample>=e->song.sampleLen) {
      sName="none selected";
    } else {
      sName=e->song.sample[ins->amiga.initSample]->name;
    }
    if (ins->type==DIV_INS_PCE ||
        ins->type==DIV_INS_MIKEY ||
        ins->type==DIV_INS_X1_010 ||
        ins->type==DIV_INS_SWAN ||
        ins->type==DIV_INS_AY ||
        ins->type==DIV_INS_AY8930 ||
        ins->type==DIV_INS_VRC6 ||
        ins->type==DIV_INS_SU) {
      P(ImGui::Checkbox("Use sample",&ins->amiga.useSample));
      if (ins->type==DIV_INS_X1_010) {
        if (ImGui::InputInt("Sample bank slot##BANKSLOT",&ins->x1_010.bankSlot,1,4)) { PARAMETER
          if (ins->x1_010.bankSlot<0) ins->x1_010.bankSlot=0;
          if (ins->x1_010.bankSlot>=7) ins->x1_010.bankSlot=7;
        }
      }
    }
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Sample");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::BeginCombo("##ISample",sName.c_str())) {
      String id;
      for (int i=0; i<e->song.sampleLen; i++) {
        id=fmt::sprintf("%d: %s",i,e->song.sample[i]->name);
        if (ImGui::Selectable(id.c_str(),ins->amiga.initSample==i)) { PARAMETER
          ins->amiga.initSample=i;
        }
      }
      ImGui::EndCombo();
    }
    // Wavetable
    if (ins->type==DIV_INS_AMIGA || ins->type==DIV_INS_SNES) {
      ImGui::BeginDisabled(ins->amiga.useNoteMap);
      P(ImGui::Checkbox("Use wavetable (Amiga/SNES/Generic DAC only)",&ins->amiga.useWave));
      if (ins->amiga.useWave) {
        int len=ins->amiga.waveLen+1;
        int origLen=len;
        if (ImGui::InputInt("Width",&len,2,16)) {
          if (ins->type==DIV_INS_SNES) {
            if (len<16) len=16;
            if (len>256) len=256;
            if (len>origLen) {
              ins->amiga.waveLen=((len+15)&(~15))-1;
            } else {
              ins->amiga.waveLen=(len&(~15))-1;
            }
          } else {
            if (len<2) len=2;
            if (len>256) len=256;
            ins->amiga.waveLen=(len&(~1))-1;
          }
          PARAMETER
        }
      }
      ImGui::EndDisabled();
    }
    // Note map
    ImGui::BeginDisabled(ins->amiga.useWave);
    P(ImGui::Checkbox("Use sample map",&ins->amiga.useNoteMap));
    if (ins->amiga.useNoteMap) {
      if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows)) sampleMapFocused=false;
      if (curWindowLast!=GUI_WINDOW_INS_EDIT) sampleMapFocused=false;
      if (!sampleMapFocused) sampleMapDigit=0;
      if (ImGui::BeginTable("NoteMap",(ins->type==DIV_INS_NES)?5:4,ImGuiTableFlags_ScrollY|ImGuiTableFlags_Borders|ImGuiTableFlags_SizingStretchSame)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthFixed);
        if (ins->type==DIV_INS_NES) ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c4",ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableSetupScrollFreeze(0,1);

        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        ImGui::Text("#");
        if (ins->type==DIV_INS_NES) {
          ImGui::TableNextColumn();
          ImGui::Text("pitch");
          ImGui::TableNextColumn();
          ImGui::Text("delta");
        } else {
          ImGui::TableNextColumn();
          ImGui::Text("note");
        }
        ImGui::TableNextColumn();
        ImGui::Text("sample name");
        int sampleMapMin=sampleMapSelStart;
        int sampleMapMax=sampleMapSelEnd;
        if (sampleMapMin>sampleMapMax) {
          sampleMapMin^=sampleMapMax;
          sampleMapMax^=sampleMapMin;
          sampleMapMin^=sampleMapMax;
        }

        ImGui::PushStyleColor(ImGuiCol_Header,ImGui::GetColorU32(ImGuiCol_HeaderHovered));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive,ImGui::GetColorU32(ImGuiCol_HeaderHovered));
        for (int i=0; i<120; i++) {
          DivInstrumentAmiga::SampleMap& sampleMap=ins->amiga.noteMap[i];
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,ImGui::GetColorU32(ImGuiCol_TableHeaderBg));
          ImGui::AlignTextToFramePadding();
          ImGui::Text("%s",noteNames[60+i]);
          ImGui::TableNextColumn();
          if (sampleMap.map<0 || sampleMap.map>=e->song.sampleLen) {
            sName=fmt::sprintf("---##SM%d",i);
            sampleMap.map=-1;
          } else {
            sName=fmt::sprintf("%3d##SM%d",sampleMap.map,i);
          }
          ImGui::PushFont(patFont);
          ImGui::AlignTextToFramePadding();
          ImGui::SetNextItemWidth(ImGui::CalcTextSize("00000").x);
          ImGui::Selectable(sName.c_str(),(sampleMapWaitingInput && sampleMapColumn==0 && i>=sampleMapMin && i<=sampleMapMax));
          if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            sampleMapFocused=true;
            sampleMapColumn=0;
            sampleMapDigit=0;
            sampleMapSelStart=i;
            sampleMapSelEnd=i;

            sampleMapMin=sampleMapSelStart;
            sampleMapMax=sampleMapSelEnd;
            if (sampleMapMin>sampleMapMax) {
              sampleMapMin^=sampleMapMax;
              sampleMapMax^=sampleMapMin;
              sampleMapMin^=sampleMapMax;
            }
            ImGui::InhibitInertialScroll();
          }
          if (sampleMapFocused && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            sampleMapSelEnd=i;
          }
          if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
            if (sampleMapSelStart==sampleMapSelEnd) {
              sampleMapFocused=true;
              sampleMapColumn=0;
              sampleMapDigit=0;
              sampleMapSelStart=i;
              sampleMapSelEnd=i;

              sampleMapMin=sampleMapSelStart;
              sampleMapMax=sampleMapSelEnd;
              if (sampleMapMin>sampleMapMax) {
                sampleMapMin^=sampleMapMax;
                sampleMapMax^=sampleMapMin;
                sampleMapMin^=sampleMapMax;
              }
            }
            if (sampleMapFocused) {
              wannaOpenSMPopup=true;
            }
          }
          ImGui::PopFont();

          if (ins->type==DIV_INS_NES) {
            // pitch
            ImGui::TableNextColumn();
            ImGui::PushFont(patFont);
            ImGui::AlignTextToFramePadding();
            ImGui::SetNextItemWidth(ImGui::CalcTextSize("0000").x);
            if (sampleMap.dpcmFreq<0) {
              sName=fmt::sprintf(" -- ##SD1%d",i);
            } else {
              sName=fmt::sprintf(" %2d ##SD1%d",sampleMap.dpcmFreq,i);
            }
            ImGui::Selectable(sName.c_str(),(sampleMapWaitingInput && sampleMapColumn==2 && i>=sampleMapMin && i<=sampleMapMax));

            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
              sampleMapFocused=true;
              sampleMapColumn=2;
              sampleMapDigit=0;
              sampleMapSelStart=i;
              sampleMapSelEnd=i;

              sampleMapMin=sampleMapSelStart;
              sampleMapMax=sampleMapSelEnd;
              if (sampleMapMin>sampleMapMax) {
                sampleMapMin^=sampleMapMax;
                sampleMapMax^=sampleMapMin;
                sampleMapMin^=sampleMapMax;
              }
              ImGui::InhibitInertialScroll();
            }
            if (sampleMapFocused && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
              sampleMapSelEnd=i;
            }
            if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
              if (sampleMapSelStart==sampleMapSelEnd) {
                sampleMapFocused=true;
                sampleMapColumn=2;
                sampleMapDigit=0;
                sampleMapSelStart=i;
                sampleMapSelEnd=i;

                sampleMapMin=sampleMapSelStart;
                sampleMapMax=sampleMapSelEnd;
                if (sampleMapMin>sampleMapMax) {
                  sampleMapMin^=sampleMapMax;
                  sampleMapMax^=sampleMapMin;
                  sampleMapMin^=sampleMapMax;
                }
              }
              if (sampleMapFocused) {
                wannaOpenSMPopup=true;
              }
            }

            ImGui::PopFont();

            // delta
            ImGui::TableNextColumn();
            ImGui::PushFont(patFont);
            ImGui::AlignTextToFramePadding();
            ImGui::SetNextItemWidth(ImGui::CalcTextSize("0000").x);
            if (sampleMap.dpcmDelta<0) {
              sName=fmt::sprintf(" -- ##SD2%d",i);
            } else {
              sName=fmt::sprintf(" %2X ##SD2%d",sampleMap.dpcmDelta,i);
            }
            ImGui::Selectable(sName.c_str(),(sampleMapWaitingInput && sampleMapColumn==3 && i>=sampleMapMin && i<=sampleMapMax));

            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
              sampleMapFocused=true;
              sampleMapColumn=3;
              sampleMapDigit=0;
              sampleMapSelStart=i;
              sampleMapSelEnd=i;

              sampleMapMin=sampleMapSelStart;
              sampleMapMax=sampleMapSelEnd;
              if (sampleMapMin>sampleMapMax) {
                sampleMapMin^=sampleMapMax;
                sampleMapMax^=sampleMapMin;
                sampleMapMin^=sampleMapMax;
              }
              ImGui::InhibitInertialScroll();
            }
            if (sampleMapFocused && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
              sampleMapSelEnd=i;
            }
            if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
              if (sampleMapSelStart==sampleMapSelEnd) {
                sampleMapFocused=true;
                sampleMapColumn=3;
                sampleMapDigit=0;
                sampleMapSelStart=i;
                sampleMapSelEnd=i;

                sampleMapMin=sampleMapSelStart;
                sampleMapMax=sampleMapSelEnd;
                if (sampleMapMin>sampleMapMax) {
                  sampleMapMin^=sampleMapMax;
                  sampleMapMax^=sampleMapMin;
                  sampleMapMin^=sampleMapMax;
                }
              }
              if (sampleMapFocused) {
                wannaOpenSMPopup=true;
              }
            }

            ImGui::PopFont();
          } else {
            ImGui::TableNextColumn();
            sName="???";
            if ((sampleMap.freq+60)>0 && (sampleMap.freq+60)<180) {
              sName=noteNames[sampleMap.freq+60];
            }
            sName+=fmt::sprintf("##SN%d",i);
            ImGui::PushFont(patFont);
            ImGui::AlignTextToFramePadding();
            ImGui::SetNextItemWidth(ImGui::CalcTextSize("00000").x);
            ImGui::Selectable(sName.c_str(),(sampleMapWaitingInput && sampleMapColumn==1 && i>=sampleMapMin && i<=sampleMapMax));
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
              sampleMapFocused=true;
              sampleMapColumn=1;
              sampleMapDigit=0;
              sampleMapSelStart=i;
              sampleMapSelEnd=i;

              sampleMapMin=sampleMapSelStart;
              sampleMapMax=sampleMapSelEnd;
              if (sampleMapMin>sampleMapMax) {
                sampleMapMin^=sampleMapMax;
                sampleMapMax^=sampleMapMin;
                sampleMapMin^=sampleMapMax;
              }
              ImGui::InhibitInertialScroll();
            }
            if (sampleMapFocused && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
              sampleMapSelEnd=i;
            }
            if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
              if (sampleMapSelStart==sampleMapSelEnd) {
                sampleMapFocused=true;
                sampleMapColumn=1;
                sampleMapDigit=0;
                sampleMapSelStart=i;
                sampleMapSelEnd=i;

                sampleMapMin=sampleMapSelStart;
                sampleMapMax=sampleMapSelEnd;
                if (sampleMapMin>sampleMapMax) {
                  sampleMapMin^=sampleMapMax;
                  sampleMapMax^=sampleMapMin;
                  sampleMapMin^=sampleMapMax;
                }
              }
              if (sampleMapFocused) {
                wannaOpenSMPopup=true;
              }
            }
            ImGui::PopFont();
          }

          ImGui::TableNextColumn();
          String prevName="---";
          if (sampleMap.map>=0 && sampleMap.map<e->song.sampleLen) {
            prevName=e->song.sample[sampleMap.map]->name;
          }
          ImGui::PushID(i+2);
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::BeginCombo("##SMSample",prevName.c_str())) {
            if (ImGui::Selectable("---")) {
              sampleMap.map=-1;
            }
            for (int k=0; k<e->song.sampleLen; k++) {
              String itemName=fmt::sprintf("%d: %s",k,e->song.sample[k]->name);
              if (ImGui::Selectable(itemName.c_str())) {
                sampleMap.map=k;
              }
            }
            ImGui::EndCombo();
          }
          ImGui::PopID();
        }
        ImGui::PopStyleColor(2);
        ImGui::EndTable();
      }
    } else {
      sampleMapFocused=false;
    }
    ImGui::EndDisabled();
    if (wannaOpenSMPopup) {
      ImGui::OpenPopup("SampleMapUtils");
    }
    if (ImGui::BeginPopup("SampleMapUtils",ImGuiWindowFlags_NoMove|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoSavedSettings)) {
      if (sampleMapSelStart==sampleMapSelEnd && sampleMapSelStart>=0 && sampleMapSelStart<120) {
        if (ins->type==DIV_INS_NES) {
          if (ImGui::MenuItem("set entire map to this pitch")) {
            if (sampleMapSelStart>=0 && sampleMapSelStart<120) {
              for (int i=0; i<120; i++) {
                if (i==sampleMapSelStart) continue;
                ins->amiga.noteMap[i].dpcmFreq=ins->amiga.noteMap[sampleMapSelStart].dpcmFreq;
              }
            }
          }
          if (ImGui::MenuItem("set entire map to this delta counter value")) {
            if (sampleMapSelStart>=0 && sampleMapSelStart<120) {
              for (int i=0; i<120; i++) {
                if (i==sampleMapSelStart) continue;
                ins->amiga.noteMap[i].dpcmDelta=ins->amiga.noteMap[sampleMapSelStart].dpcmDelta;
              }
            }
          }
        } else {
          if (ImGui::MenuItem("set entire map to this note")) {
            if (sampleMapSelStart>=0 && sampleMapSelStart<120) {
              for (int i=0; i<120; i++) {
                if (i==sampleMapSelStart) continue;
                ins->amiga.noteMap[i].freq=ins->amiga.noteMap[sampleMapSelStart].freq;
              }
            }
          }
        }
        if (ImGui::MenuItem("set entire map to this sample")) {
          if (sampleMapSelStart>=0 && sampleMapSelStart<120) {
            for (int i=0; i<120; i++) {
              if (i==sampleMapSelStart) continue;
              ins->amiga.noteMap[i].map=ins->amiga.noteMap[sampleMapSelStart].map;
            }
          }
        }
      }
      if (ins->type==DIV_INS_NES) {
        if (ImGui::MenuItem("reset pitches")) {
          for (int i=0; i<120; i++) {
            ins->amiga.noteMap[i].dpcmFreq=15;
          }
        }
        if (ImGui::MenuItem("clear delta counter values")) {
          for (int i=0; i<120; i++) {
            ins->amiga.noteMap[i].dpcmDelta=-1;
          }
        }
      } else {
        if (ImGui::MenuItem("reset notes")) {
          for (int i=0; i<120; i++) {
            ins->amiga.noteMap[i].freq=i;
          }
        }
      }
      if (ImGui::MenuItem("clear map samples")) {
        for (int i=0; i<120; i++) {
          ins->amiga.noteMap[i].map=-1;
        }
      }
      ImGui::EndPopup();
    }
    ImGui::EndTabItem();
  } else {
    sampleMapFocused=false;
  }
}

void FurnaceGUI::drawInsEdit() {
  if (nextWindow==GUI_WINDOW_INS_EDIT) {
    insEditOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!insEditOpen) return;
  if (mobileUI) {
    patWindowPos=(portrait?ImVec2(0.0f,(mobileMenuPos*-0.65*canvasH)):ImVec2((0.16*canvasH)+0.5*canvasW*mobileMenuPos,0.0f));
    patWindowSize=(portrait?ImVec2(canvasW,canvasH-(0.16*canvasW)-(pianoOpen?(0.4*canvasW):0.0f)):ImVec2(canvasW-(0.16*canvasH),canvasH-(pianoOpen?(0.3*canvasH):0.0f)));
    ImGui::SetNextWindowPos(patWindowPos);
    ImGui::SetNextWindowSize(patWindowSize);
  } else {
    ImGui::SetNextWindowSizeConstraints(ImVec2(440.0f*dpiScale,400.0f*dpiScale),ImVec2(canvasW,canvasH));
  }
  if (ImGui::Begin("Instrument Editor",&insEditOpen,globalWinFlags|(settings.allowEditDocking?0:ImGuiWindowFlags_NoDocking))) {
    if (curIns<0 || curIns>=(int)e->song.ins.size()) {
      ImGui::SetCursorPosY(ImGui::GetCursorPosY()+(ImGui::GetContentRegionAvail().y-ImGui::GetFrameHeightWithSpacing()*(e->song.ins.empty()?2.0f:3.0f)+ImGui::GetStyle().ItemSpacing.y)*0.5f);
      CENTER_TEXT("no instrument selected");
      ImGui::Text("no instrument selected");
      if (ImGui::BeginTable("noAssetCenter",3)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.5f);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.5f);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();

        if (e->song.ins.size()>0) {
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::BeginCombo("##InsSelect","select one...")) {
            String name;
            for (size_t i=0; i<e->song.ins.size(); i++) {
              name=fmt::sprintf("%.2X: %s##_INSS%d",i,e->song.ins[i]->name,i);
              if (ImGui::Selectable(name.c_str(),curIns==(int)i)) {
                curIns=i;
                wavePreviewInit=true;
                updateFMPreview=true;
              }
            }
            ImGui::EndCombo();
          }
          ImGui::AlignTextToFramePadding();
          ImGui::TextUnformatted("or");
          ImGui::SameLine();
        }
        if (ImGui::Button("Open")) {
          doAction(GUI_ACTION_INS_LIST_OPEN);
        }
        ImGui::SameLine();
        ImGui::TextUnformatted("or");
        ImGui::SameLine();
        if (ImGui::Button("Create New")) {
          doAction(GUI_ACTION_INS_LIST_ADD);
        }

        ImGui::TableNextColumn();
        ImGui::EndTable();
      }
    } else {
      DivInstrument* ins=e->song.ins[curIns];
      if (updateFMPreview) {
        renderFMPreview(ins);
        updateFMPreview=false;
      }
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
              updateFMPreview=true;
            }
          }
          ImGui::EndCombo();
        }

        ImGui::TableNextColumn();
        ImGui::Text("Name");

        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::PushID(2+curIns);
        if (ImGui::InputText("##Name",&ins->name)) {
          MARK_MODIFIED;
        }
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Button(ICON_FA_FOLDER_OPEN "##IELoad")) {
          doAction(GUI_ACTION_INS_LIST_OPEN_REPLACE);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Open");
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_FLOPPY_O "##IESave")) {
          doAction(GUI_ACTION_INS_LIST_SAVE);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Save");
        }
        if (ImGui::BeginPopupContextItem("InsSaveFormats",ImGuiMouseButton_Right)) {
          if (ImGui::MenuItem("save as .dmp...")) {
            doAction(GUI_ACTION_INS_LIST_SAVE_DMP);
          }
          ImGui::EndPopup();
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
        bool warnType=true;
        for (DivInstrumentType i: e->getPossibleInsTypes()) {
          if (i==insType) {
            warnType=false;
          }
        }

        pushWarningColor(warnType,warnType && failedNoteOn);
        if (ImGui::BeginCombo("##Type",insTypes[insType][0])) {
          std::vector<DivInstrumentType> insTypeList;
          if (settings.displayAllInsTypes) {
            for (int i=0; insTypes[i][0]; i++) {
              insTypeList.push_back((DivInstrumentType)i);
            }
          } else {
            insTypeList=e->getPossibleInsTypes();
          }
          for (DivInstrumentType i: insTypeList) {
            if (ImGui::Selectable(insTypes[i][0],insType==i)) {
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
        } else if (warnType) {
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("none of the currently present chips are able to play this instrument type!");
          }
        }
        popWarningColor();

        ImGui::EndTable();
      }
      

      if (ImGui::BeginTabBar("insEditTab")) {
        std::vector<FurnaceGUIMacroDesc> macroList;
        if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPLL || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_OPM) {
          char label[32];
          int opCount=4;
          if (ins->type==DIV_INS_OPLL) opCount=2;
          if (ins->type==DIV_INS_OPL) opCount=(ins->fm.ops==4)?4:2;
          bool opsAreMutable=(ins->type==DIV_INS_FM || ins->type==DIV_INS_OPM);

          if (ImGui::BeginTabItem("FM")) {
            DivInstrumentFM& fmOrigin=(ins->type==DIV_INS_OPLL && ins->fm.opllPreset>0 && ins->fm.opllPreset<16)?opllPreview:ins->fm;

            bool isPresent[4];
            int isPresentCount=0;
            memset(isPresent,0,4*sizeof(bool));
            for (int i=0; i<e->song.systemLen; i++) {
              if (e->song.system[i]==DIV_SYSTEM_VRC7) {
                isPresent[3]=true;
              } else if (e->song.system[i]==DIV_SYSTEM_OPLL || e->song.system[i]==DIV_SYSTEM_OPLL_DRUMS) {
                isPresent[(e->song.systemFlags[i].getInt("patchSet",0))&3]=true;
              }
            }
            if (!isPresent[0] && !isPresent[1] && !isPresent[2] && !isPresent[3]) {
              isPresent[0]=true;
            }
            for (int i=0; i<4; i++) {
              if (isPresent[i]) isPresentCount++;
            }
            int presentWhich=0;
            for (int i=0; i<4; i++) {
              if (isPresent[i]) {
                presentWhich=i;
                break;
              }
            }

            if (ImGui::BeginTable("fmDetails",3,ImGuiTableFlags_SizingStretchSame)) {
              ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.0);
              ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0);
              ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.0);

              ImGui::TableNextRow();
              switch (ins->type) {
                case DIV_INS_FM:
                case DIV_INS_OPM:
                  ImGui::TableNextColumn();
                  P(CWSliderScalar(FM_NAME(FM_FB),ImGuiDataType_U8,&ins->fm.fb,&_ZERO,&_SEVEN)); rightClickable
                  P(CWSliderScalar(FM_NAME(FM_FMS),ImGuiDataType_U8,&ins->fm.fms,&_ZERO,&_SEVEN)); rightClickable
                  ImGui::TableNextColumn();
                  P(CWSliderScalar(FM_NAME(FM_ALG),ImGuiDataType_U8,&ins->fm.alg,&_ZERO,&_SEVEN)); rightClickable
                  P(CWSliderScalar(FM_NAME(FM_AMS),ImGuiDataType_U8,&ins->fm.ams,&_ZERO,&_THREE)); rightClickable
                  ImGui::TableNextColumn();
                  if (fmPreviewOn) {
                    drawFMPreview(ImVec2(ImGui::GetContentRegionAvail().x,48.0*dpiScale));
                    if (!fmPreviewPaused) {
                      renderFMPreview(ins,1);
                      WAKE_UP;
                    }
                  } else {
                    drawAlgorithm(ins->fm.alg,FM_ALGS_4OP,ImVec2(ImGui::GetContentRegionAvail().x,48.0*dpiScale));
                  }
                  kvsConfig(ins);
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
                  if (fmPreviewOn) {
                    drawFMPreview(ImVec2(ImGui::GetContentRegionAvail().x,48.0*dpiScale));
                    if (!fmPreviewPaused) {
                      renderFMPreview(ins,1);
                      WAKE_UP;
                    }
                  } else {
                    drawAlgorithm(ins->fm.alg,FM_ALGS_4OP,ImVec2(ImGui::GetContentRegionAvail().x,48.0*dpiScale));
                  }
                  kvsConfig(ins);

                  if (ImGui::Button("Request from TX81Z")) {
                    doAction(GUI_ACTION_TX81Z_REQUEST);
                  }
                  /* 
                  ImGui::SameLine();
                  if (ImGui::Button("Send to TX81Z")) {
                    showError("Coming soon!");
                  }
                  */
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
                  if (fmPreviewOn) {
                    drawFMPreview(ImVec2(ImGui::GetContentRegionAvail().x,48.0*dpiScale));
                    if (!fmPreviewPaused) {
                      renderFMPreview(ins,1);
                      WAKE_UP;
                    }
                  } else {
                    drawAlgorithm(ins->fm.alg&algMax,fourOp?FM_ALGS_4OP_OPL:FM_ALGS_2OP_OPL,ImVec2(ImGui::GetContentRegionAvail().x,48.0*dpiScale));
                  }
                  kvsConfig(ins);
                  break;
                }
                case DIV_INS_OPLL: {
                  bool dc=fmOrigin.fms;
                  bool dm=fmOrigin.ams;
                  bool sus=ins->fm.alg;
                  ImGui::TableNextColumn();
                  ImGui::BeginDisabled(ins->fm.opllPreset!=0);
                  P(CWSliderScalar(FM_NAME(FM_FB),ImGuiDataType_U8,&fmOrigin.fb,&_ZERO,&_SEVEN)); rightClickable
                  if (ImGui::Checkbox(FM_NAME(FM_DC),&dc)) { PARAMETER
                    fmOrigin.fms=dc;
                  }
                  ImGui::EndDisabled();
                  ImGui::TableNextColumn();
                  if (ImGui::Checkbox(FM_NAME(FM_SUS),&sus)) { PARAMETER
                    ins->fm.alg=sus;
                  }
                  ImGui::BeginDisabled(ins->fm.opllPreset!=0);
                  if (ImGui::Checkbox(FM_NAME(FM_DM),&dm)) { PARAMETER
                    fmOrigin.ams=dm;
                  }
                  ImGui::EndDisabled();
                  ImGui::TableNextColumn();
                  if (fmPreviewOn) {
                    drawFMPreview(ImVec2(ImGui::GetContentRegionAvail().x,24.0*dpiScale));
                    if (!fmPreviewPaused) {
                      renderFMPreview(ins,1);
                      WAKE_UP;
                    }
                  } else {
                    drawAlgorithm(0,FM_ALGS_2OP_OPL,ImVec2(ImGui::GetContentRegionAvail().x,24.0*dpiScale));
                  }
                  kvsConfig(ins,false);

                  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

                  if (ImGui::BeginCombo("##LLPreset",opllInsNames[presentWhich][ins->fm.opllPreset])) {
                    if (isPresentCount>1) {
                      if (ImGui::BeginTable("LLPresetList",isPresentCount)) {
                        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
                        for (int i=0; i<4; i++) {
                          if (!isPresent[i]) continue;
                          ImGui::TableNextColumn();
                          ImGui::Text("%s name",opllVariants[i]);
                        }
                        for (int i=0; i<17; i++) {
                          ImGui::TableNextRow();
                          for (int j=0; j<4; j++) {
                            if (!isPresent[j]) continue;
                            ImGui::TableNextColumn();
                            ImGui::PushID(j*17+i);
                            if (ImGui::Selectable(opllInsNames[j][i])) {
                              ins->fm.opllPreset=i;
                            }
                            ImGui::PopID();
                          }
                        }
                        ImGui::EndTable();
                      }
                    } else {
                      for (int i=0; i<17; i++) {
                        if (ImGui::Selectable(opllInsNames[presentWhich][i])) {
                          ins->fm.opllPreset=i;
                        }
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

              // update OPLL preset preview
              if (ins->fm.opllPreset>0 && ins->fm.opllPreset<16) {
                const opll_patch_t* patchROM=NULL;

                switch (presentWhich) {
                  case 1:
                    patchROM=OPLL_GetPatchROM(opll_type_ymf281);
                    break;
                  case 2:
                    patchROM=OPLL_GetPatchROM(opll_type_ym2423);
                    break;
                  case 3:
                    patchROM=OPLL_GetPatchROM(opll_type_ds1001);
                    break;
                  default:
                    patchROM=OPLL_GetPatchROM(opll_type_ym2413);
                    break;
                }

                const opll_patch_t* patch=&patchROM[ins->fm.opllPreset-1];

                opllPreview.alg=ins->fm.alg;
                opllPreview.fb=patch->fb;
                opllPreview.fms=patch->dm;
                opllPreview.ams=patch->dc;

                opllPreview.op[0].tl=patch->tl;
                opllPreview.op[1].tl=ins->fm.op[1].tl;

                for (int i=0; i<2; i++) {
                  opllPreview.op[i].am=patch->am[i];
                  opllPreview.op[i].vib=patch->vib[i];
                  opllPreview.op[i].ssgEnv=patch->et[i]?8:0;
                  opllPreview.op[i].ksr=patch->ksr[i];
                  opllPreview.op[i].mult=patch->multi[i];
                  opllPreview.op[i].ar=patch->ar[i];
                  opllPreview.op[i].dr=patch->dr[i];
                  opllPreview.op[i].sl=patch->sl[i];
                  opllPreview.op[i].rr=patch->rr[i];
                }
              }
            }

            ImGui::BeginDisabled(!willDisplayOps);
            if (settings.fmLayout==0) {
              int numCols=15;
              if (ins->type==DIV_INS_OPL ||ins->type==DIV_INS_OPL_DRUMS) numCols=13;
              if (ins->type==DIV_INS_OPLL) numCols=12;
              if (ins->type==DIV_INS_OPZ) numCols=19;
              if (ImGui::BeginTable("FMOperators",numCols,ImGuiTableFlags_SizingStretchProp|ImGuiTableFlags_BordersH|ImGuiTableFlags_BordersOuterV)) {
                // configure columns
                ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed); // op name
                ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.05f); // ar
                ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.05f); // dr
                ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthStretch,0.05f); // sl
                if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM) {
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

                if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM) {
                  ImGui::TableSetupColumn("c10",ImGuiTableColumnFlags_WidthStretch,0.05f); // dt
                }
                if (ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM) {
                  ImGui::TableSetupColumn("c11",ImGuiTableColumnFlags_WidthStretch,0.05f); // dt2
                }
                ImGui::TableSetupColumn("c15",ImGuiTableColumnFlags_WidthFixed); // am

                ImGui::TableSetupColumn("c12",ImGuiTableColumnFlags_WidthFixed); // -separator-
                if (ins->type!=DIV_INS_OPLL && ins->type!=DIV_INS_OPM) {
                  ImGui::TableSetupColumn("c13",ImGuiTableColumnFlags_WidthStretch,0.2f); // ssg/waveform
                }
                ImGui::TableSetupColumn("c14",ImGuiTableColumnFlags_WidthStretch,0.3f); // env

                // header
                ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
                ImGui::TableNextColumn();

                ImGui::TableNextColumn();
                CENTER_TEXT(FM_SHORT_NAME(FM_AR));
                ImGui::TextUnformatted(FM_SHORT_NAME(FM_AR));
                TOOLTIP_TEXT(FM_NAME(FM_AR));
                ImGui::TableNextColumn();
                CENTER_TEXT(FM_SHORT_NAME(FM_DR));
                ImGui::TextUnformatted(FM_SHORT_NAME(FM_DR));
                TOOLTIP_TEXT(FM_NAME(FM_DR));
                if (settings.susPosition==0) {
                  ImGui::TableNextColumn();
                  CENTER_TEXT(FM_SHORT_NAME(FM_SL));
                  ImGui::TextUnformatted(FM_SHORT_NAME(FM_SL));
                  TOOLTIP_TEXT(FM_NAME(FM_SL));
                }
                if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM) {
                  ImGui::TableNextColumn();
                  CENTER_TEXT(FM_SHORT_NAME(FM_D2R));
                  ImGui::TextUnformatted(FM_SHORT_NAME(FM_D2R));
                  TOOLTIP_TEXT(FM_NAME(FM_D2R));
                }
                ImGui::TableNextColumn();
                CENTER_TEXT(FM_SHORT_NAME(FM_RR));
                ImGui::TextUnformatted(FM_SHORT_NAME(FM_RR));
                TOOLTIP_TEXT(FM_NAME(FM_RR));
                if (settings.susPosition==1) {
                  ImGui::TableNextColumn();
                  CENTER_TEXT(FM_SHORT_NAME(FM_SL));
                  ImGui::TextUnformatted(FM_SHORT_NAME(FM_SL));
                  TOOLTIP_TEXT(FM_NAME(FM_SL));
                }
                ImGui::TableNextColumn();
                ImGui::TableNextColumn();
                CENTER_TEXT(FM_SHORT_NAME(FM_TL));
                ImGui::TextUnformatted(FM_SHORT_NAME(FM_TL));
                TOOLTIP_TEXT(FM_NAME(FM_TL));
                ImGui::TableNextColumn();
                if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM) {
                  CENTER_TEXT(FM_SHORT_NAME(FM_RS));
                  ImGui::TextUnformatted(FM_SHORT_NAME(FM_RS));
                  TOOLTIP_TEXT(FM_NAME(FM_RS));
                } else {
                  CENTER_TEXT(FM_SHORT_NAME(FM_KSL));
                  ImGui::TextUnformatted(FM_SHORT_NAME(FM_KSL));
                  TOOLTIP_TEXT(FM_NAME(FM_KSL));
                }
                if (ins->type==DIV_INS_OPZ) {
                  ImGui::TableNextColumn();
                  CENTER_TEXT(FM_SHORT_NAME(FM_EGSHIFT));
                  ImGui::TextUnformatted(FM_SHORT_NAME(FM_EGSHIFT));
                  TOOLTIP_TEXT(FM_NAME(FM_EGSHIFT));
                  ImGui::TableNextColumn();
                  CENTER_TEXT(FM_SHORT_NAME(FM_REV));
                  ImGui::TextUnformatted(FM_SHORT_NAME(FM_REV));
                  TOOLTIP_TEXT(FM_NAME(FM_REV));
                }
                ImGui::TableNextColumn();
                CENTER_TEXT(FM_SHORT_NAME(FM_MULT));
                ImGui::TextUnformatted(FM_SHORT_NAME(FM_MULT));
                TOOLTIP_TEXT(FM_NAME(FM_MULT));
                if (ins->type==DIV_INS_OPZ) {
                  ImGui::TableNextColumn();
                  CENTER_TEXT(FM_SHORT_NAME(FM_FINE));
                  ImGui::TextUnformatted(FM_SHORT_NAME(FM_FINE));
                  TOOLTIP_TEXT(FM_NAME(FM_FINE));
                }
                ImGui::TableNextColumn();
                if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM) {
                  CENTER_TEXT(FM_SHORT_NAME(FM_DT));
                  ImGui::TextUnformatted(FM_SHORT_NAME(FM_DT));
                  TOOLTIP_TEXT(FM_NAME(FM_DT));
                  ImGui::TableNextColumn();
                }
                if (ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM) {
                  CENTER_TEXT(FM_SHORT_NAME(FM_DT2));
                  ImGui::TextUnformatted(FM_SHORT_NAME(FM_DT2));
                  TOOLTIP_TEXT(FM_NAME(FM_DT2));
                  ImGui::TableNextColumn();
                }
                if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPM) {
                  CENTER_TEXT(FM_SHORT_NAME(FM_AM));
                  ImGui::TextUnformatted(FM_SHORT_NAME(FM_AM));
                  TOOLTIP_TEXT(FM_NAME(FM_AM));
                } else {
                  CENTER_TEXT("Other");
                  ImGui::TextUnformatted("Other");
                }
                ImGui::TableNextColumn();
                if (ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_OPZ) {
                  ImGui::TableNextColumn();
                  CENTER_TEXT(FM_NAME(FM_WS));
                  ImGui::TextUnformatted(FM_NAME(FM_WS));
                } else if (ins->type!=DIV_INS_OPLL && ins->type!=DIV_INS_OPM) {
                  ImGui::TableNextColumn();
                  CENTER_TEXT(FM_NAME(FM_SSG));
                  ImGui::TextUnformatted(FM_NAME(FM_SSG));
                }
                ImGui::TableNextColumn();
                CENTER_TEXT("Envelope");
                ImGui::TextUnformatted("Envelope");

                float sliderHeight=32.0f*dpiScale;

                for (int i=0; i<opCount; i++) {
                  DivInstrumentFM::Operator& op=fmOrigin.op[(opCount==4 && ins->type!=DIV_INS_OPL_DRUMS)?opOrder[i]:i];
                  ImGui::TableNextRow();
                  ImGui::TableNextColumn();

                  // push colors
                  if (settings.separateFMColors) {
                    bool mod=true;
                    if (ins->type==DIV_INS_OPL_DRUMS) {
                      mod=false;
                    } else if (opCount==4) {
                      if (ins->type==DIV_INS_OPL) {
                        if (opIsOutputOPL[fmOrigin.alg&3][i]) mod=false;
                      } else {
                        if (opIsOutput[fmOrigin.alg&7][i]) mod=false;
                      }
                    } else {
                      if (i==1 || (ins->type==DIV_INS_OPL && (fmOrigin.alg&1))) mod=false;
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
                  String opNameLabel;
                  if (ins->type==DIV_INS_OPL_DRUMS) {
                    opNameLabel=fmt::sprintf("%s",oplDrumNames[i]);
                  } else if (ins->type==DIV_INS_OPL && fmOrigin.opllPreset==16) {
                    if (i==1) {
                      opNameLabel="Kick";
                    } else {
                      opNameLabel="Env";
                    }
                  } else {
                    opNameLabel=fmt::sprintf("OP%d",i+1);
                  }
                  if (opsAreMutable) {
                    pushToggleColors(op.enable);
                    if (ImGui::Button(opNameLabel.c_str())) {
                      op.enable=!op.enable;
                      PARAMETER;
                    }
                    popToggleColors();
                  } else {
                    ImGui::TextUnformatted(opNameLabel.c_str());
                  }
                  
                  // drag point
                  OP_DRAG_POINT;

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
                  int maxArDr=(ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM)?31:15;
                  bool ssgOn=op.ssgEnv&8;
                  bool ksrOn=op.ksr;
                  bool vibOn=op.vib;
                  bool susOn=op.sus;
                  unsigned char ssgEnv=op.ssgEnv&7;

                  ImGui::TableNextColumn();
                  op.ar&=maxArDr;
                  CENTER_VSLIDER;
                  P(CWVSliderScalar("##AR",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.ar,&maxArDr,&_ZERO)); rightClickable

                  ImGui::TableNextColumn();
                  op.dr&=maxArDr;
                  CENTER_VSLIDER;
                  P(CWVSliderScalar("##DR",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.dr,&maxArDr,&_ZERO)); rightClickable

                  if (settings.susPosition==0) {
                    ImGui::TableNextColumn();
                    op.sl&=15;
                    CENTER_VSLIDER;
                    P(CWVSliderScalar("##SL",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.sl,&_FIFTEEN,&_ZERO)); rightClickable
                  }

                  if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM) {
                    ImGui::TableNextColumn();
                    op.d2r&=31;
                    CENTER_VSLIDER;
                    P(CWVSliderScalar("##D2R",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.d2r,&_THIRTY_ONE,&_ZERO)); rightClickable
                  }

                  ImGui::TableNextColumn();
                  op.rr&=15;
                  CENTER_VSLIDER;
                  P(CWVSliderScalar("##RR",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.rr,&_FIFTEEN,&_ZERO)); rightClickable

                  if (settings.susPosition==1) {
                    ImGui::TableNextColumn();
                    op.sl&=15;
                    CENTER_VSLIDER;
                    P(CWVSliderScalar("##SL",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.sl,&_FIFTEEN,&_ZERO)); rightClickable
                  }

                  ImGui::TableNextColumn();
                  ImGui::Dummy(ImVec2(4.0f*dpiScale,2.0f*dpiScale));

                  ImGui::TableNextColumn();
                  op.tl&=maxTl;
                  CENTER_VSLIDER;
                  P(CWVSliderScalar("##TL",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.tl,&maxTl,&_ZERO)); rightClickable

                  ImGui::TableNextColumn();
                  CENTER_VSLIDER;
                  if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM) {
                    P(CWVSliderScalar("##RS",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.rs,&_ZERO,&_THREE)); rightClickable
                  } else {
                    int ksl=ins->type==DIV_INS_OPLL?op.ksl:kslMap[op.ksl&3];
                    if (CWVSliderInt("##KSL",ImVec2(20.0f*dpiScale,sliderHeight),&ksl,0,3)) {
                      op.ksl=(ins->type==DIV_INS_OPLL?ksl:kslMap[ksl&3]);
                      PARAMETER;
                    } rightClickable
                  }

                  if (ins->type==DIV_INS_OPZ) {
                    ImGui::TableNextColumn();
                    CENTER_VSLIDER;
                    P(CWVSliderScalar("##EGS",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.ksl,&_ZERO,&_THREE)); rightClickable

                    ImGui::TableNextColumn();
                    CENTER_VSLIDER;
                    P(CWVSliderScalar("##REV",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.dam,&_ZERO,&_SEVEN)); rightClickable
                  }

                  ImGui::TableNextColumn();
                  CENTER_VSLIDER;
                  P(CWVSliderScalar("##MULT",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.mult,&_ZERO,&_FIFTEEN)); rightClickable

                  if (ins->type==DIV_INS_OPZ) {
                    ImGui::TableNextColumn();
                    CENTER_VSLIDER;
                    P(CWVSliderScalar("##FINE",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.dvb,&_ZERO,&_FIFTEEN)); rightClickable
                  }

                  if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM) {
                    int detune=detuneMap[settings.unsignedDetune?1:0][op.dt&7];
                    ImGui::TableNextColumn();
                    CENTER_VSLIDER;
                    if (CWVSliderInt("##DT",ImVec2(20.0f*dpiScale,sliderHeight),&detune,settings.unsignedDetune?0:-3,settings.unsignedDetune?7:4)) { PARAMETER
                      if (detune<-3) detune=-3;
                      if (detune>7) detune=7;
                      op.dt=detuneUnmap[settings.unsignedDetune?1:0][detune+3];
                    } rightClickable

                    if (ins->type!=DIV_INS_FM) {
                      ImGui::TableNextColumn();
                      CENTER_VSLIDER;
                      P(CWVSliderScalar("##DT2",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.dt2,&_ZERO,&_THREE)); rightClickable
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

                    if (ins->type!=DIV_INS_OPL && ins->type!=DIV_INS_OPL_DRUMS && ins->type!=DIV_INS_OPZ && ins->type!=DIV_INS_OPM) {
                      ImGui::TableNextColumn();
                      ImGui::Dummy(ImVec2(4.0f*dpiScale,2.0f*dpiScale));
                      ImGui::TableNextColumn();
                      ImGui::BeginDisabled(!ssgOn);
                      drawSSGEnv(op.ssgEnv&7,ImVec2(ImGui::GetContentRegionAvail().x,sliderHeight-ImGui::GetFrameHeightWithSpacing()));
                      ImGui::EndDisabled();
                      if (ImGui::Checkbox("##SSGOn",&ssgOn)) { PARAMETER
                        op.ssgEnv=(op.ssgEnv&7)|(ssgOn<<3);
                      }

                      ImGui::SameLine();
                      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                      if (CWSliderScalar("##SSG",ImGuiDataType_U8,&ssgEnv,&_ZERO,&_SEVEN,ssgEnvTypes[ssgEnv])) { PARAMETER
                        op.ssgEnv=(op.ssgEnv&8)|(ssgEnv&7);
                      }
                    }
                  } else if (ins->type!=DIV_INS_OPM) {
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
                  } else if (ins->type==DIV_INS_OPLL || ins->type==DIV_INS_OPM) {
                    ImGui::TableNextColumn();
                    ImGui::Dummy(ImVec2(4.0f*dpiScale,2.0f*dpiScale));
                  }

                  ImGui::TableNextColumn();
                  drawFMEnv(op.tl&maxTl,op.ar&maxArDr,op.dr&maxArDr,(ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_OPLL)?((op.rr&15)*2):op.d2r&31,op.rr&15,op.sl&15,op.sus,op.ssgEnv&8,fmOrigin.alg,maxTl,maxArDr,15,ImVec2(ImGui::GetContentRegionAvail().x,sliderHeight),ins->type);

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
              if (ImGui::BeginTable("AltFMOperators",columns,ImGuiTableFlags_SizingStretchSame|ImGuiTableFlags_BordersInner)) {
                for (int i=0; i<opCount; i++) {
                  DivInstrumentFM::Operator& op=fmOrigin.op[(opCount==4 && ins->type!=DIV_INS_OPL_DRUMS)?opOrder[i]:i];
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
                        if (opIsOutputOPL[fmOrigin.alg&3][i]) mod=false;
                      } else {
                        if (opIsOutput[fmOrigin.alg&7][i]) mod=false;
                      }
                    } else {
                      if (i==1 || (ins->type==DIV_INS_OPL && (fmOrigin.alg&1))) mod=false;
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
                  } else if (ins->type==DIV_INS_OPL && fmOrigin.opllPreset==16) {
                    if (i==1) {
                      snprintf(tempID,1024,"Envelope 2 (kick only)");
                    } else {
                      snprintf(tempID,1024,"Envelope");
                    }
                  } else {
                    snprintf(tempID,1024,"Operator %d",i+1);
                  }
                  float nextCursorPosX=ImGui::GetCursorPosX()+0.5*(ImGui::GetContentRegionAvail().x-ImGui::CalcTextSize(tempID).x-(opsAreMutable?(ImGui::GetStyle().FramePadding.x*2.0f):0.0f));
                  OP_DRAG_POINT;
                  ImGui::SameLine();
                  ImGui::SetCursorPosX(nextCursorPosX);
                  if (opsAreMutable) {
                    pushToggleColors(op.enable);
                    if (ImGui::Button(tempID)) {
                      op.enable=!op.enable;
                      PARAMETER;
                    }
                    popToggleColors();
                  } else {
                    ImGui::TextUnformatted(tempID);
                  }

                  float sliderHeight=200.0f*dpiScale;
                  float waveWidth=140.0*dpiScale;
                  float waveHeight=sliderHeight-ImGui::GetFrameHeightWithSpacing()*((ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPL)?5.0f:4.5f);

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
                  int maxArDr=(ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM)?31:15;

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
                    TOOLTIP_TEXT(FM_NAME(FM_AR));
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
                    ImGui::TextUnformatted(FM_SHORT_NAME(FM_TL));
                    TOOLTIP_TEXT(FM_NAME(FM_TL));

                    // A/D/S/R
                    ImGui::TableNextColumn();

                    op.ar&=maxArDr;
                    P(CWVSliderScalar("##AR",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.ar,&maxArDr,&_ZERO)); rightClickable

                    ImGui::SameLine();
                    op.dr&=maxArDr;
                    float textX_DR=ImGui::GetCursorPosX();
                    P(CWVSliderScalar("##DR",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.dr,&maxArDr,&_ZERO)); rightClickable

                    float textX_SL=0.0f;
                    if (settings.susPosition==0) {
                      ImGui::SameLine();
                      op.sl&=15;
                      textX_SL=ImGui::GetCursorPosX();
                      P(CWVSliderScalar("##SL",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.sl,&_FIFTEEN,&_ZERO)); rightClickable
                    }

                    float textX_D2R=0.0f;
                    if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM) {
                      ImGui::SameLine();
                      op.d2r&=31;
                      textX_D2R=ImGui::GetCursorPosX();
                      P(CWVSliderScalar("##D2R",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.d2r,&_THIRTY_ONE,&_ZERO)); rightClickable
                    }

                    ImGui::SameLine();
                    op.rr&=15;
                    float textX_RR=ImGui::GetCursorPosX();
                    P(CWVSliderScalar("##RR",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.rr,&_FIFTEEN,&_ZERO)); rightClickable

                    if (settings.susPosition==1) {
                      ImGui::SameLine();
                      op.sl&=15;
                      textX_SL=ImGui::GetCursorPosX();
                      P(CWVSliderScalar("##SL",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.sl,&_FIFTEEN,&_ZERO)); rightClickable
                    }

                    ImVec2 prevCurPos=ImGui::GetCursorPos();
                    
                    // labels
                    ImGui::SetCursorPos(ImVec2(textX_DR,textY));
                    CENTER_TEXT_20(FM_SHORT_NAME(FM_DR));
                    ImGui::TextUnformatted(FM_SHORT_NAME(FM_DR));
                    TOOLTIP_TEXT(FM_NAME(FM_DR));

                    ImGui::SetCursorPos(ImVec2(textX_SL,textY));
                    CENTER_TEXT_20(FM_SHORT_NAME(FM_SL));
                    ImGui::TextUnformatted(FM_SHORT_NAME(FM_SL));
                    TOOLTIP_TEXT(FM_NAME(FM_SL));

                    ImGui::SetCursorPos(ImVec2(textX_RR,textY));
                    CENTER_TEXT_20(FM_SHORT_NAME(FM_RR));
                    ImGui::TextUnformatted(FM_SHORT_NAME(FM_RR));
                    TOOLTIP_TEXT(FM_NAME(FM_RR));

                    if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM) {
                      ImGui::SetCursorPos(ImVec2(textX_D2R,textY));
                      CENTER_TEXT_20(FM_SHORT_NAME(FM_D2R));
                      ImGui::TextUnformatted(FM_SHORT_NAME(FM_D2R));
                      TOOLTIP_TEXT(FM_NAME(FM_D2R));
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

                        int detune=detuneMap[settings.unsignedDetune?1:0][op.dt&7];
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_DT));
                        if (CWSliderInt("##DT",&detune,settings.unsignedDetune?0:-3,settings.unsignedDetune?7:4,tempID)) { PARAMETER
                          if (detune<-3) detune=-3;
                          if (detune>7) detune=7;
                          op.dt=detuneUnmap[settings.unsignedDetune?1:0][detune+3];
                        } rightClickable

                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_RS));
                        P(CWSliderScalar("##RS",ImGuiDataType_U8,&op.rs,&_ZERO,&_THREE,tempID)); rightClickable

                        break;
                      }
                      case DIV_INS_OPM: {
                        drawWaveform(0,true,ImVec2(waveWidth,waveHeight));
                        
                        // params
                        ImGui::Separator();
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_MULT));
                        P(CWSliderScalar("##MULT",ImGuiDataType_U8,&op.mult,&_ZERO,&_FIFTEEN,tempID)); rightClickable

                        int detune=detuneMap[settings.unsignedDetune?1:0][op.dt&7];
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_DT));
                        if (CWSliderInt("##DT",&detune,settings.unsignedDetune?0:-3,settings.unsignedDetune?7:4,tempID)) { PARAMETER
                          if (detune<-3) detune=-3;
                          if (detune>7) detune=7;
                          op.dt=detuneUnmap[settings.unsignedDetune?1:0][detune+3];
                        } rightClickable

                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_DT2));
                        P(CWSliderScalar("##DT2",ImGuiDataType_U8,&op.dt2,&_ZERO,&_THREE,tempID)); rightClickable

                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_RS));
                        P(CWSliderScalar("##RS",ImGuiDataType_U8,&op.rs,&_ZERO,&_THREE,tempID)); rightClickable
                        break;
                      }
                      case DIV_INS_OPLL:
                        // waveform
                        drawWaveform(i==0?(fmOrigin.ams&1):(fmOrigin.fms&1),ins->type==DIV_INS_OPZ,ImVec2(waveWidth,waveHeight));

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
                      case DIV_INS_OPL_DRUMS: {
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
                        int ksl=kslMap[op.ksl&3];
                        if (CWSliderInt("##KSL",&ksl,0,3,tempID)) {
                          op.ksl=kslMap[ksl&3];
                          PARAMETER;
                        } rightClickable

                        break;
                      }
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

                          int detune=detuneMap[settings.unsignedDetune?1:0][op.dt&7];
                          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                          snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_DT));
                          if (CWSliderInt("##DT",&detune,settings.unsignedDetune?0:-3,settings.unsignedDetune?7:4,tempID)) { PARAMETER
                            if (detune<-3) detune=-3;
                            if (detune>7) detune=7;
                            op.dt=detuneUnmap[settings.unsignedDetune?1:0][detune+3];
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
                    drawFMEnv(op.tl&maxTl,op.ar&maxArDr,op.dr&maxArDr,(ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_OPLL)?((op.rr&15)*2):op.d2r&31,op.rr&15,op.sl&15,op.sus,op.ssgEnv&8,fmOrigin.alg,maxTl,maxArDr,15,ImVec2(ImGui::GetContentRegionAvail().x,envHeight),ins->type);

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
                    P(CWVSliderScalar("##TL",ImVec2(ImGui::GetFrameHeight(),sliderHeight-((ins->type==DIV_INS_FM || ins->type==DIV_INS_OPM)?(ImGui::GetFrameHeightWithSpacing()+ImGui::CalcTextSize(FM_SHORT_NAME(FM_AM)).y+ImGui::GetStyle().ItemSpacing.y):0.0f)),ImGuiDataType_U8,&op.tl,&maxTl,&_ZERO)); rightClickable

                    if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPM) {
                      CENTER_TEXT(FM_SHORT_NAME(FM_AM));
                      ImGui::TextUnformatted(FM_SHORT_NAME(FM_AM));
                      TOOLTIP_TEXT(FM_NAME(FM_AM));
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
                  DivInstrumentFM::Operator& op=fmOrigin.op[(opCount==4 && ins->type!=DIV_INS_OPL_DRUMS)?opOrder[i]:i];
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
                        if (opIsOutputOPL[fmOrigin.alg&3][i]) mod=false;
                      } else {
                        if (opIsOutput[fmOrigin.alg&7][i]) mod=false;
                      }
                    } else {
                      if (i==1 || (ins->type==DIV_INS_OPL && (fmOrigin.alg&1))) mod=false;
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
                  String opNameLabel;
                  OP_DRAG_POINT;
                  ImGui::SameLine();
                  if (ins->type==DIV_INS_OPL_DRUMS) {
                    opNameLabel=fmt::sprintf("%s",oplDrumNames[i]);
                  } else if (ins->type==DIV_INS_OPL && fmOrigin.opllPreset==16) {
                    if (i==1) {
                      opNameLabel="Envelope 2 (kick only)";
                    } else {
                      opNameLabel="Envelope";
                    }
                  } else {
                    opNameLabel=fmt::sprintf("OP%d",i+1);
                  }
                  if (opsAreMutable) {
                    pushToggleColors(op.enable);
                    if (ImGui::Button(opNameLabel.c_str())) {
                      op.enable=!op.enable;
                      PARAMETER;
                    }
                    popToggleColors();
                  } else {
                    ImGui::TextUnformatted(opNameLabel.c_str());
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
                  int maxArDr=(ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM)?31:15;

                  bool ssgOn=op.ssgEnv&8;
                  bool ksrOn=op.ksr;
                  bool vibOn=op.vib;
                  bool susOn=op.sus; // don't you make fun of this one
                  unsigned char ssgEnv=op.ssgEnv&7;
                  if (ins->type!=DIV_INS_OPL && ins->type!=DIV_INS_OPL_DRUMS && ins->type!=DIV_INS_OPZ && ins->type!=DIV_INS_OPM) {
                    ImGui::SameLine();
                    if (ImGui::Checkbox((ins->type==DIV_INS_OPLL)?FM_NAME(FM_EGS):"SSG On",&ssgOn)) { PARAMETER
                      op.ssgEnv=(op.ssgEnv&7)|(ssgOn<<3);
                    }
                  }

                  if (ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS) {
                    ImGui::SameLine();
                    if (ImGui::Checkbox(FM_NAME(FM_SUS),&susOn)) { PARAMETER
                      op.sus=susOn;
                    }
                  }

                  if (ins->type==DIV_INS_OPZ) {
                    ImGui::SameLine();
                    bool fixedOn=op.egt;
                    if (ImGui::Checkbox("Fixed",&fixedOn)) { PARAMETER
                      op.egt=fixedOn;
                    }
                  }

                  //52.0 controls vert scaling; default 96
                  drawFMEnv(op.tl&maxTl,op.ar&maxArDr,op.dr&maxArDr,(ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_OPLL)?((op.rr&15)*2):op.d2r&31,op.rr&15,op.sl&15,op.sus,op.ssgEnv&8,fmOrigin.alg,maxTl,maxArDr,15,ImVec2(ImGui::GetContentRegionAvail().x,52.0*dpiScale),ins->type);
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

                    if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM) {
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
                    if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM) {
                      P(CWSliderScalar("##RS",ImGuiDataType_U8,&op.rs,&_ZERO,&_THREE)); rightClickable
                      ImGui::TableNextColumn();
                      ImGui::Text("%s",FM_NAME(FM_RS));
                    } else {
                      int ksl=ins->type==DIV_INS_OPLL?op.ksl:kslMap[op.ksl&3];
                      if (CWSliderInt("##KSL",&ksl,0,3)) {
                        op.ksl=(ins->type==DIV_INS_OPLL?ksl:kslMap[ksl&3]);
                        PARAMETER;
                      } rightClickable
                      ImGui::TableNextColumn();
                      ImGui::Text("%s",FM_NAME(FM_KSL));
                    }

                    if (ins->type==DIV_INS_OPZ) {
                      ImGui::TableNextRow();
                      ImGui::TableNextColumn();
                      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                      P(CWSliderScalar(FM_NAME(FM_EGSHIFT),ImGuiDataType_U8,&op.ksl,&_ZERO,&_THREE)); rightClickable
                      ImGui::TableNextColumn();
                      ImGui::Text("%s",FM_NAME(FM_EGSHIFT));

                      ImGui::TableNextRow();
                      ImGui::TableNextColumn();
                      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                      P(CWSliderScalar(FM_NAME(FM_REV),ImGuiDataType_U8,&op.dam,&_ZERO,&_SEVEN)); rightClickable
                      ImGui::TableNextColumn();
                      ImGui::Text("%s",FM_NAME(FM_REV));
                    }

                    if (ins->type==DIV_INS_OPZ) {
                      if (op.egt) {
                        int block=op.dt;
                        int freqNum=(op.mult<<4)|(op.dvb&15);

                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        if (CWSliderInt(FM_NAME(FM_MULT),&block,0,7)) { PARAMETER
                          if (block<0) block=0;
                          if (block>7) block=7;
                          op.dt=block;
                        } rightClickable
                        ImGui::TableNextColumn();
                        ImGui::Text("Block");

                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        if (CWSliderInt(FM_NAME(FM_FINE),&freqNum,0,255)) { PARAMETER
                          if (freqNum<0) freqNum=0;
                          if (freqNum>255) freqNum=255;
                          op.mult=freqNum>>4;
                          op.dvb=freqNum&15;
                        } rightClickable
                        ImGui::TableNextColumn();
                        ImGui::Text("FreqNum");
                      } else {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        P(CWSliderScalar(FM_NAME(FM_MULT),ImGuiDataType_U8,&op.mult,&_ZERO,&_FIFTEEN)); rightClickable
                        ImGui::TableNextColumn();
                        ImGui::Text("%s",FM_NAME(FM_MULT));

                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        P(CWSliderScalar(FM_NAME(FM_FINE),ImGuiDataType_U8,&op.dvb,&_ZERO,&_FIFTEEN)); rightClickable
                        ImGui::TableNextColumn();
                        ImGui::Text("%s",FM_NAME(FM_FINE));
                      }
                    } else {
                      ImGui::TableNextRow();
                      ImGui::TableNextColumn();
                      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                      P(CWSliderScalar(FM_NAME(FM_MULT),ImGuiDataType_U8,&op.mult,&_ZERO,&_FIFTEEN)); rightClickable
                      ImGui::TableNextColumn();
                      ImGui::Text("%s",FM_NAME(FM_MULT));
                    }
                    
                    if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM) {
                      if (!(ins->type==DIV_INS_OPZ && op.egt)) {
                        int detune=detuneMap[settings.unsignedDetune?1:0][op.dt&7];
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        if (CWSliderInt("##DT",&detune,settings.unsignedDetune?0:-3,settings.unsignedDetune?7:4)) { PARAMETER
                          if (detune<-3) detune=-3;
                          if (detune>7) detune=7;
                          op.dt=detuneUnmap[settings.unsignedDetune?1:0][detune+3];
                        } rightClickable
                        ImGui::TableNextColumn();
                        ImGui::Text("%s",FM_NAME(FM_DT));
                      }

                      if (ins->type!=DIV_INS_FM) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        P(CWSliderScalar("##DT2",ImGuiDataType_U8,&op.dt2,&_ZERO,&_THREE)); rightClickable
                        ImGui::TableNextColumn();
                        ImGui::Text("%s",FM_NAME(FM_DT2));
                      }

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
            ImGui::EndDisabled();
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
            
            if (ins->type==DIV_INS_FM) {
              macroList.push_back(FurnaceGUIMacroDesc("LFO Speed",&ins->std.ex3Macro,0,8,96,uiColors[GUI_COLOR_MACRO_OTHER]));
            }
            if (ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM) {
              macroList.push_back(FurnaceGUIMacroDesc("AM Depth",&ins->std.ex1Macro,0,127,128,uiColors[GUI_COLOR_MACRO_OTHER]));
              macroList.push_back(FurnaceGUIMacroDesc("PM Depth",&ins->std.ex2Macro,0,127,128,uiColors[GUI_COLOR_MACRO_OTHER]));
              macroList.push_back(FurnaceGUIMacroDesc("LFO Speed",&ins->std.ex3Macro,0,255,128,uiColors[GUI_COLOR_MACRO_OTHER]));
              macroList.push_back(FurnaceGUIMacroDesc("LFO Shape",&ins->std.waveMacro,0,3,48,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,macroLFOWaves));
            }
            if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPM) {
              macroList.push_back(FurnaceGUIMacroDesc("OpMask",&ins->std.ex4Macro,0,4,128,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,fmOperatorBits));
            } else if (ins->type==DIV_INS_OPZ) {
              macroList.push_back(FurnaceGUIMacroDesc("AM Depth 2",&ins->std.ex5Macro,0,127,128,uiColors[GUI_COLOR_MACRO_OTHER]));
              macroList.push_back(FurnaceGUIMacroDesc("PM Depth 2",&ins->std.ex6Macro,0,127,128,uiColors[GUI_COLOR_MACRO_OTHER]));
              macroList.push_back(FurnaceGUIMacroDesc("LFO2 Speed",&ins->std.ex7Macro,0,255,128,uiColors[GUI_COLOR_MACRO_OTHER]));
              macroList.push_back(FurnaceGUIMacroDesc("LFO2 Shape",&ins->std.ex8Macro,0,3,48,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,macroLFOWaves));
            }
            drawMacros(macroList,macroEditStateFM);
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
              int maxArDr=(ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM)?31:15;

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
                if (ins->type==DIV_INS_OPM || ins->type==DIV_INS_OPZ) {
                  macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_DT2),&ins->std.opMacros[ordi].dt2Macro,0,3,32,uiColors[GUI_COLOR_MACRO_OTHER]));
                }
                macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_AM),&ins->std.opMacros[ordi].amMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));

                if (ins->type==DIV_INS_FM) {
                  macroList.push_back(FurnaceGUIMacroDesc(FM_NAME(FM_SSG),&ins->std.opMacros[ordi].ssgMacro,0,4,64,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,ssgEnvBits));
                }
              }
              drawMacros(macroList,macroEditStateOP[ordi]);
              ImGui::PopID();
              ImGui::EndTabItem();
            }
          }
        }
        if (ins->type==DIV_INS_GB) if (ImGui::BeginTabItem("Game Boy")) {
          P(ImGui::Checkbox("Use software envelope",&ins->gb.softEnv));
          P(ImGui::Checkbox("Initialize envelope on every note",&ins->gb.alwaysInit));

          ImGui::BeginDisabled(ins->gb.softEnv);
          if (ImGui::BeginTable("GBParams",2)) {
            ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.6f);
            ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.4f);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (ImGui::BeginTable("GBParamsI",2)) {
              ImGui::TableSetupColumn("ci0",ImGuiTableColumnFlags_WidthFixed);
              ImGui::TableSetupColumn("ci1",ImGuiTableColumnFlags_WidthStretch);

              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::Text("Volume");
              ImGui::TableNextColumn();
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              P(CWSliderScalar("##GBVolume",ImGuiDataType_U8,&ins->gb.envVol,&_ZERO,&_FIFTEEN)); rightClickable

              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::Text("Length");
              ImGui::TableNextColumn();
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              P(CWSliderScalar("##GBEnvLen",ImGuiDataType_U8,&ins->gb.envLen,&_ZERO,&_SEVEN)); rightClickable

              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::Text("Sound Length");
              ImGui::TableNextColumn();
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              P(CWSliderScalar("##GBSoundLen",ImGuiDataType_U8,&ins->gb.soundLen,&_ZERO,&_SIXTY_FOUR,ins->gb.soundLen>63?"Infinity":"%d")); rightClickable

              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::Text("Direction");
              ImGui::TableNextColumn();
              bool goesUp=ins->gb.envDir;
              if (ImGui::RadioButton("Up",goesUp)) { PARAMETER
                goesUp=true;
                ins->gb.envDir=goesUp;
              }
              ImGui::SameLine();
              if (ImGui::RadioButton("Down",!goesUp)) { PARAMETER
                goesUp=false;
                ins->gb.envDir=goesUp;
              }

              ImGui::EndTable();
            }

            ImGui::TableNextColumn();
            drawGBEnv(ins->gb.envVol,ins->gb.envLen,ins->gb.soundLen,ins->gb.envDir,ImVec2(ImGui::GetContentRegionAvail().x,100.0f*dpiScale));

            ImGui::EndTable();
          }

          if (ImGui::BeginChild("HWSeq",ImGui::GetContentRegionAvail(),true,ImGuiWindowFlags_MenuBar)) {
            ImGui::BeginMenuBar();
            ImGui::Text("Hardware Sequence");
            ImGui::EndMenuBar();

            if (ins->gb.hwSeqLen>0) if (ImGui::BeginTable("HWSeqList",3)) {
              ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
              ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch);
              ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthFixed);
              int curFrame=0;
              ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
              ImGui::TableNextColumn();
              ImGui::Text("Tick");
              ImGui::TableNextColumn();
              ImGui::Text("Command");
              ImGui::TableNextColumn();
              ImGui::Text("Move/Remove");
              for (int i=0; i<ins->gb.hwSeqLen; i++) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%d (#%d)",curFrame,i);
                ImGui::TableNextColumn();
                ImGui::PushID(i);
                if (ins->gb.hwSeq[i].cmd>=DivInstrumentGB::DIV_GB_HWCMD_MAX) {
                  ins->gb.hwSeq[i].cmd=0;
                }
                int cmd=ins->gb.hwSeq[i].cmd;
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::Combo("##HWSeqCmd",&cmd,gbHWSeqCmdTypes,DivInstrumentGB::DIV_GB_HWCMD_MAX)) {
                  if (ins->gb.hwSeq[i].cmd!=cmd) {
                    ins->gb.hwSeq[i].cmd=cmd;
                    ins->gb.hwSeq[i].data=0;
                  }
                }
                bool somethingChanged=false;
                switch (ins->gb.hwSeq[i].cmd) {
                  case DivInstrumentGB::DIV_GB_HWCMD_ENVELOPE: {
                    int hwsVol=(ins->gb.hwSeq[i].data&0xf0)>>4;
                    bool hwsDir=ins->gb.hwSeq[i].data&8;
                    int hwsLen=ins->gb.hwSeq[i].data&7;
                    int hwsSoundLen=ins->gb.hwSeq[i].data>>8;

                    if (CWSliderInt("Volume",&hwsVol,0,15)) {
                      somethingChanged=true;
                    }
                    if (CWSliderInt("Env Length",&hwsLen,0,7)) {
                      somethingChanged=true;
                    }
                    if (CWSliderInt("Sound Length",&hwsSoundLen,0,64,hwsSoundLen>63?"Infinity":"%d")) {
                      somethingChanged=true;
                    }
                    if (ImGui::RadioButton("Up",hwsDir)) { PARAMETER
                      hwsDir=true;
                      somethingChanged=true;
                    }
                    ImGui::SameLine();
                    if (ImGui::RadioButton("Down",!hwsDir)) { PARAMETER
                      hwsDir=false;
                      somethingChanged=true;
                    }

                    if (somethingChanged) {
                      ins->gb.hwSeq[i].data=(hwsLen&7)|(hwsDir?8:0)|(hwsVol<<4)|(hwsSoundLen<<8);
                      PARAMETER;
                    }
                    break;
                  }
                  case DivInstrumentGB::DIV_GB_HWCMD_SWEEP: {
                    int hwsShift=ins->gb.hwSeq[i].data&7;
                    int hwsSpeed=(ins->gb.hwSeq[i].data&0x70)>>4;
                    bool hwsDir=ins->gb.hwSeq[i].data&8;

                    if (CWSliderInt("Shift",&hwsShift,0,7)) {
                      somethingChanged=true;
                    }
                    if (CWSliderInt("Speed",&hwsSpeed,0,7)) {
                      somethingChanged=true;
                    }

                    if (ImGui::RadioButton("Up",!hwsDir)) { PARAMETER
                      hwsDir=false;
                      somethingChanged=true;
                    }
                    ImGui::SameLine();
                    if (ImGui::RadioButton("Down",hwsDir)) { PARAMETER
                      hwsDir=true;
                      somethingChanged=true;
                    }

                    if (somethingChanged) {
                      ins->gb.hwSeq[i].data=(hwsShift&7)|(hwsDir?8:0)|(hwsSpeed<<4);
                      PARAMETER;
                    }
                    break;
                  }
                  case DivInstrumentGB::DIV_GB_HWCMD_WAIT: {
                    int len=ins->gb.hwSeq[i].data+1;
                    curFrame+=ins->gb.hwSeq[i].data+1;

                    if (ImGui::InputInt("Ticks",&len,1,4)) {
                      if (len<1) len=1;
                      if (len>255) len=256;
                      somethingChanged=true;
                    }

                    if (somethingChanged) {
                      ins->gb.hwSeq[i].data=len-1;
                      PARAMETER;
                    }
                    break;
                  }
                  case DivInstrumentGB::DIV_GB_HWCMD_WAIT_REL:
                    curFrame++;
                    break;
                  case DivInstrumentGB::DIV_GB_HWCMD_LOOP:
                  case DivInstrumentGB::DIV_GB_HWCMD_LOOP_REL: {
                    int pos=ins->gb.hwSeq[i].data;

                    if (ImGui::InputInt("Position",&pos,1,1)) {
                      if (pos<0) pos=0;
                      if (pos>(ins->gb.hwSeqLen-1)) pos=(ins->gb.hwSeqLen-1);
                      somethingChanged=true;
                    }

                    if (somethingChanged) {
                      ins->gb.hwSeq[i].data=pos;
                      PARAMETER;
                    }
                    break;
                  }
                  default:
                    break;
                }
                ImGui::PopID();
                ImGui::TableNextColumn();
                ImGui::PushID(i+512);
                if (ImGui::Button(ICON_FA_CHEVRON_UP "##HWCmdUp")) {
                  if (i>0) {
                    e->lockEngine([ins,i]() {
                      ins->gb.hwSeq[i-1].cmd^=ins->gb.hwSeq[i].cmd;
                      ins->gb.hwSeq[i].cmd^=ins->gb.hwSeq[i-1].cmd;
                      ins->gb.hwSeq[i-1].cmd^=ins->gb.hwSeq[i].cmd;

                      ins->gb.hwSeq[i-1].data^=ins->gb.hwSeq[i].data;
                      ins->gb.hwSeq[i].data^=ins->gb.hwSeq[i-1].data;
                      ins->gb.hwSeq[i-1].data^=ins->gb.hwSeq[i].data;
                    });
                  }
                  MARK_MODIFIED;
                }
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_CHEVRON_DOWN "##HWCmdDown")) {
                  if (i<ins->gb.hwSeqLen-1) {
                    e->lockEngine([ins,i]() {
                      ins->gb.hwSeq[i+1].cmd^=ins->gb.hwSeq[i].cmd;
                      ins->gb.hwSeq[i].cmd^=ins->gb.hwSeq[i+1].cmd;
                      ins->gb.hwSeq[i+1].cmd^=ins->gb.hwSeq[i].cmd;

                      ins->gb.hwSeq[i+1].data^=ins->gb.hwSeq[i].data;
                      ins->gb.hwSeq[i].data^=ins->gb.hwSeq[i+1].data;
                      ins->gb.hwSeq[i+1].data^=ins->gb.hwSeq[i].data;
                    });
                  }
                  MARK_MODIFIED;
                }
                ImGui::SameLine();
                pushDestColor();
                if (ImGui::Button(ICON_FA_TIMES "##HWCmdDel")) {
                  for (int j=i; j<ins->gb.hwSeqLen-1; j++) {
                    ins->gb.hwSeq[j].cmd=ins->gb.hwSeq[j+1].cmd;
                    ins->gb.hwSeq[j].data=ins->gb.hwSeq[j+1].data;
                  }
                  ins->gb.hwSeqLen--;
                }
                popDestColor();
                ImGui::PopID();
              }
              ImGui::EndTable();
            }

            if (ImGui::Button(ICON_FA_PLUS "##HWCmdAdd")) {
              if (ins->gb.hwSeqLen<255) {
                ins->gb.hwSeq[ins->gb.hwSeqLen].cmd=0;
                ins->gb.hwSeq[ins->gb.hwSeqLen].data=0;
                ins->gb.hwSeqLen++;
              }
            }
          }
          ImGui::EndChild();
          ImGui::EndDisabled();
          ImGui::EndTabItem();
        }
        if (ins->type==DIV_INS_C64) if (ImGui::BeginTabItem("C64")) {
          ImGui::AlignTextToFramePadding();
          ImGui::Text("Waveform");
          ImGui::SameLine();
          pushToggleColors(ins->c64.triOn);
          if (ImGui::Button("tri")) { PARAMETER
            ins->c64.triOn=!ins->c64.triOn;
          }
          popToggleColors();
          ImGui::SameLine();
          pushToggleColors(ins->c64.sawOn);
          if (ImGui::Button("saw")) { PARAMETER
            ins->c64.sawOn=!ins->c64.sawOn;
          }
          popToggleColors();
          ImGui::SameLine();
          pushToggleColors(ins->c64.pulseOn);
          if (ImGui::Button("pulse")) { PARAMETER
            ins->c64.pulseOn=!ins->c64.pulseOn;
          }
          popToggleColors();
          ImGui::SameLine();
          pushToggleColors(ins->c64.noiseOn);
          if (ImGui::Button("noise")) { PARAMETER
            ins->c64.noiseOn=!ins->c64.noiseOn;
          }
          popToggleColors();

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
            P(CWVSliderScalar("##Attack",sliderSize,ImGuiDataType_U8,&ins->c64.a,&_ZERO,&_FIFTEEN)); rightClickable
            ImGui::TableNextColumn();
            P(CWVSliderScalar("##Decay",sliderSize,ImGuiDataType_U8,&ins->c64.d,&_ZERO,&_FIFTEEN)); rightClickable
            ImGui::TableNextColumn();
            P(CWVSliderScalar("##Sustain",sliderSize,ImGuiDataType_U8,&ins->c64.s,&_ZERO,&_FIFTEEN)); rightClickable
            ImGui::TableNextColumn();
            P(CWVSliderScalar("##Release",sliderSize,ImGuiDataType_U8,&ins->c64.r,&_ZERO,&_FIFTEEN)); rightClickable
            ImGui::TableNextColumn();
            drawFMEnv(0,16-ins->c64.a,16-ins->c64.d,15-ins->c64.r,15-ins->c64.r,15-ins->c64.s,0,0,0,15,16,15,ImVec2(ImGui::GetContentRegionAvail().x,sliderSize.y),ins->type);

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

          ImGui::AlignTextToFramePadding();
          ImGui::Text("Filter Mode");
          ImGui::SameLine();
          pushToggleColors(ins->c64.lp);
          if (ImGui::Button("low")) { PARAMETER
            ins->c64.lp=!ins->c64.lp;
          }
          popToggleColors();
          ImGui::SameLine();
          pushToggleColors(ins->c64.bp);
          if (ImGui::Button("band")) { PARAMETER
            ins->c64.bp=!ins->c64.bp;
          }
          popToggleColors();
          ImGui::SameLine();
          pushToggleColors(ins->c64.hp);
          if (ImGui::Button("high")) { PARAMETER
            ins->c64.hp=!ins->c64.hp;
          }
          popToggleColors();
          ImGui::SameLine();
          pushToggleColors(ins->c64.ch3off);
          if (ImGui::Button("ch3off")) { PARAMETER
            ins->c64.ch3off=!ins->c64.ch3off;
          }
          popToggleColors();

          if (ImGui::Checkbox("Absolute Cutoff Macro",&ins->c64.filterIsAbs)) {
            ins->std.algMacro.vZoom=-1;
            PARAMETER;
          }
          if (ImGui::Checkbox("Absolute Duty Macro",&ins->c64.dutyIsAbs)) {
            ins->std.dutyMacro.vZoom=-1;
            PARAMETER;
          }
          P(ImGui::Checkbox("Don't test before new note",&ins->c64.noTest));
          ImGui::EndTabItem();
        }
        if (ins->type==DIV_INS_SU) if (ImGui::BeginTabItem("Sound Unit")) {
          P(ImGui::Checkbox("Switch roles of frequency and phase reset timer",&ins->su.switchRoles));
          if (ImGui::BeginChild("HWSeqSU",ImGui::GetContentRegionAvail(),true,ImGuiWindowFlags_MenuBar)) {
            ImGui::BeginMenuBar();
            ImGui::Text("Hardware Sequence");
            ImGui::EndMenuBar();

            if (ins->su.hwSeqLen>0) if (ImGui::BeginTable("HWSeqListSU",3)) {
              ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
              ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch);
              ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthFixed);
              int curFrame=0;
              ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
              ImGui::TableNextColumn();
              ImGui::Text("Tick");
              ImGui::TableNextColumn();
              ImGui::Text("Command");
              ImGui::TableNextColumn();
              ImGui::Text("Move/Remove");
              for (int i=0; i<ins->su.hwSeqLen; i++) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%d (#%d)",curFrame,i);
                ImGui::TableNextColumn();
                ImGui::PushID(i);
                if (ins->su.hwSeq[i].cmd>=DivInstrumentSoundUnit::DIV_SU_HWCMD_MAX) {
                  ins->su.hwSeq[i].cmd=0;
                }
                int cmd=ins->su.hwSeq[i].cmd;
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::Combo("##HWSeqCmd",&cmd,suHWSeqCmdTypes,DivInstrumentSoundUnit::DIV_SU_HWCMD_MAX)) {
                  if (ins->su.hwSeq[i].cmd!=cmd) {
                    ins->su.hwSeq[i].cmd=cmd;
                    ins->su.hwSeq[i].val=0;
                    ins->su.hwSeq[i].bound=0;
                    ins->su.hwSeq[i].speed=0;
                  }
                }
                bool somethingChanged=false;
                switch (ins->su.hwSeq[i].cmd) {
                  case DivInstrumentSoundUnit::DIV_SU_HWCMD_VOL: {
                    int swPeriod=ins->su.hwSeq[i].speed;
                    int swBound=ins->su.hwSeq[i].bound;
                    int swVal=ins->su.hwSeq[i].val&31;
                    bool swDir=ins->su.hwSeq[i].val&32;
                    bool swLoop=ins->su.hwSeq[i].val&64;
                    bool swInvert=ins->su.hwSeq[i].val&128;

                    if (ImGui::InputInt("Period",&swPeriod,1,16)) {
                      if (swPeriod<0) swPeriod=0;
                      if (swPeriod>65535) swPeriod=65535;
                      somethingChanged=true;
                    }
                    if (CWSliderInt("Amount",&swVal,0,31)) {
                      somethingChanged=true;
                    }
                    if (CWSliderInt("Bound",&swBound,0,255)) {
                      somethingChanged=true;
                    }
                    if (ImGui::RadioButton("Up",swDir)) { PARAMETER
                      swDir=true;
                      somethingChanged=true;
                    }
                    ImGui::SameLine();
                    if (ImGui::RadioButton("Down",!swDir)) { PARAMETER
                      swDir=false;
                      somethingChanged=true;
                    }
                    if (ImGui::Checkbox("Loop",&swLoop)) { PARAMETER
                      somethingChanged=true;
                    }
                    ImGui::SameLine();
                    if (ImGui::Checkbox("Flip",&swInvert)) { PARAMETER
                      somethingChanged=true;
                    }

                    if (somethingChanged) {
                      ins->su.hwSeq[i].speed=swPeriod;
                      ins->su.hwSeq[i].bound=swBound;
                      ins->su.hwSeq[i].val=(swVal&31)|(swDir?32:0)|(swLoop?64:0)|(swInvert?128:0);
                      PARAMETER;
                    }
                    break;
                  }
                  case DivInstrumentSoundUnit::DIV_SU_HWCMD_PITCH:
                  case DivInstrumentSoundUnit::DIV_SU_HWCMD_CUT: {
                    int swPeriod=ins->su.hwSeq[i].speed;
                    int swBound=ins->su.hwSeq[i].bound;
                    int swVal=ins->su.hwSeq[i].val&127;
                    bool swDir=ins->su.hwSeq[i].val&128;

                    if (ImGui::InputInt("Period",&swPeriod,1,16)) {
                      if (swPeriod<0) swPeriod=0;
                      if (swPeriod>65535) swPeriod=65535;
                      somethingChanged=true;
                    }
                    if (CWSliderInt("Amount",&swVal,0,31)) {
                      somethingChanged=true;
                    }
                    if (CWSliderInt("Bound",&swBound,0,255)) {
                      somethingChanged=true;
                    }
                    if (ImGui::RadioButton("Up",swDir)) { PARAMETER
                      swDir=true;
                      somethingChanged=true;
                    }
                    ImGui::SameLine();
                    if (ImGui::RadioButton("Down",!swDir)) { PARAMETER
                      swDir=false;
                      somethingChanged=true;
                    }

                    if (somethingChanged) {
                      ins->su.hwSeq[i].speed=swPeriod;
                      ins->su.hwSeq[i].bound=swBound;
                      ins->su.hwSeq[i].val=(swVal&127)|(swDir?128:0);
                      PARAMETER;
                    }
                    break;
                  }
                  case DivInstrumentSoundUnit::DIV_SU_HWCMD_WAIT: {
                    int len=ins->su.hwSeq[i].val+1;
                    curFrame+=ins->su.hwSeq[i].val+1;

                    if (ImGui::InputInt("Ticks",&len)) {
                      if (len<1) len=1;
                      if (len>255) len=256;
                      somethingChanged=true;
                    }

                    if (somethingChanged) {
                      ins->su.hwSeq[i].val=len-1;
                      PARAMETER;
                    }
                    break;
                  }
                  case DivInstrumentSoundUnit::DIV_SU_HWCMD_WAIT_REL:
                    curFrame++;
                    break;
                  case DivInstrumentSoundUnit::DIV_SU_HWCMD_LOOP:
                  case DivInstrumentSoundUnit::DIV_SU_HWCMD_LOOP_REL: {
                    int pos=ins->su.hwSeq[i].val;

                    if (ImGui::InputInt("Position",&pos,1,4)) {
                      if (pos<0) pos=0;
                      if (pos>(ins->su.hwSeqLen-1)) pos=(ins->su.hwSeqLen-1);
                      somethingChanged=true;
                    }

                    if (somethingChanged) {
                      ins->su.hwSeq[i].val=pos;
                      PARAMETER;
                    }
                    break;
                  }
                  default:
                    break;
                }
                ImGui::PopID();
                ImGui::TableNextColumn();
                ImGui::PushID(i+512);
                if (ImGui::Button(ICON_FA_CHEVRON_UP "##HWCmdUp")) {
                  if (i>0) {
                    e->lockEngine([ins,i]() {
                      ins->su.hwSeq[i-1].cmd^=ins->su.hwSeq[i].cmd;
                      ins->su.hwSeq[i].cmd^=ins->su.hwSeq[i-1].cmd;
                      ins->su.hwSeq[i-1].cmd^=ins->su.hwSeq[i].cmd;

                      ins->su.hwSeq[i-1].speed^=ins->su.hwSeq[i].speed;
                      ins->su.hwSeq[i].speed^=ins->su.hwSeq[i-1].speed;
                      ins->su.hwSeq[i-1].speed^=ins->su.hwSeq[i].speed;

                      ins->su.hwSeq[i-1].val^=ins->su.hwSeq[i].val;
                      ins->su.hwSeq[i].val^=ins->su.hwSeq[i-1].val;
                      ins->su.hwSeq[i-1].val^=ins->su.hwSeq[i].val;

                      ins->su.hwSeq[i-1].bound^=ins->su.hwSeq[i].bound;
                      ins->su.hwSeq[i].bound^=ins->su.hwSeq[i-1].bound;
                      ins->su.hwSeq[i-1].bound^=ins->su.hwSeq[i].bound;
                    });
                  }
                  MARK_MODIFIED;
                }
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_CHEVRON_DOWN "##HWCmdDown")) {
                  if (i<ins->su.hwSeqLen-1) {
                    e->lockEngine([ins,i]() {
                      ins->su.hwSeq[i+1].cmd^=ins->su.hwSeq[i].cmd;
                      ins->su.hwSeq[i].cmd^=ins->su.hwSeq[i+1].cmd;
                      ins->su.hwSeq[i+1].cmd^=ins->su.hwSeq[i].cmd;

                      ins->su.hwSeq[i+1].speed^=ins->su.hwSeq[i].speed;
                      ins->su.hwSeq[i].speed^=ins->su.hwSeq[i+1].speed;
                      ins->su.hwSeq[i+1].speed^=ins->su.hwSeq[i].speed;

                      ins->su.hwSeq[i+1].val^=ins->su.hwSeq[i].val;
                      ins->su.hwSeq[i].val^=ins->su.hwSeq[i+1].val;
                      ins->su.hwSeq[i+1].val^=ins->su.hwSeq[i].val;

                      ins->su.hwSeq[i+1].bound^=ins->su.hwSeq[i].bound;
                      ins->su.hwSeq[i].bound^=ins->su.hwSeq[i+1].bound;
                      ins->su.hwSeq[i+1].bound^=ins->su.hwSeq[i].bound;
                    });
                  }
                  MARK_MODIFIED;
                }
                ImGui::SameLine();
                pushDestColor();
                if (ImGui::Button(ICON_FA_TIMES "##HWCmdDel")) {
                  for (int j=i; j<ins->su.hwSeqLen-1; j++) {
                    ins->su.hwSeq[j].cmd=ins->su.hwSeq[j+1].cmd;
                    ins->su.hwSeq[j].speed=ins->su.hwSeq[j+1].speed;
                    ins->su.hwSeq[j].val=ins->su.hwSeq[j+1].val;
                    ins->su.hwSeq[j].bound=ins->su.hwSeq[j+1].bound;
                  }
                  ins->su.hwSeqLen--;
                }
                popDestColor();
                ImGui::PopID();
              }
              ImGui::EndTable();
            }

            if (ImGui::Button(ICON_FA_PLUS "##HWCmdAdd")) {
              if (ins->su.hwSeqLen<255) {
                ins->su.hwSeq[ins->su.hwSeqLen].cmd=0;
                ins->su.hwSeq[ins->su.hwSeqLen].speed=0;
                ins->su.hwSeq[ins->su.hwSeqLen].val=0;
                ins->su.hwSeq[ins->su.hwSeqLen].bound=0;
                ins->su.hwSeqLen++;
              }
            }
          }
          ImGui::EndChild();
          ImGui::EndTabItem();
        }
        if (ins->type==DIV_INS_MSM6258 ||
            ins->type==DIV_INS_MSM6295 ||
            ins->type==DIV_INS_ADPCMA ||
            ins->type==DIV_INS_ADPCMB ||
            ins->type==DIV_INS_SEGAPCM ||
            ins->type==DIV_INS_QSOUND ||
            ins->type==DIV_INS_YMZ280B ||
            ins->type==DIV_INS_RF5C68 ||
            ins->type==DIV_INS_AMIGA ||
            ins->type==DIV_INS_MULTIPCM ||
            ins->type==DIV_INS_SU ||
            ins->type==DIV_INS_SNES ||
            ins->type==DIV_INS_ES5506 ||
            ins->type==DIV_INS_K007232 ||
            ins->type==DIV_INS_GA20 ||
            ins->type==DIV_INS_K053260 ||
            ins->type==DIV_INS_C140 ||
            ins->type==DIV_INS_C219) {
          insTabSample(ins);
        }
        if (ins->type==DIV_INS_N163) if (ImGui::BeginTabItem("Namco 163")) {
          bool preLoad=ins->n163.waveMode&0x1;
          if (ImGui::Checkbox("Load waveform",&preLoad)) { PARAMETER
            ins->n163.waveMode=(ins->n163.waveMode&~0x1)|(preLoad?0x1:0);
          }

          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("when enabled, a waveform will be loaded into RAM.\nwhen disabled, only the offset and length change.");
          }

          if (preLoad) {
            if (ImGui::InputInt("Waveform##WAVE",&ins->n163.wave,1,10)) { PARAMETER
              if (ins->n163.wave<0) ins->n163.wave=0;
              if (ins->n163.wave>=e->song.waveLen) ins->n163.wave=e->song.waveLen-1;
            }
          }

          ImGui::Separator();

          P(ImGui::Checkbox("Per-channel wave position/length",&ins->n163.perChanPos));

          if (ins->n163.perChanPos) {
            if (ImGui::BeginTable("N1PerChPos",3)) {
              ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
              ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.5f);
              ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.5f);

              ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
              ImGui::TableNextColumn();
              ImGui::Text("Ch");
              ImGui::TableNextColumn();
              ImGui::Text("Position");
              ImGui::TableNextColumn();
              ImGui::Text("Length");

              for (int i=0; i<8; i++) {
                ImGui::PushID(64+i);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Dummy(ImVec2(dpiScale,ImGui::GetFrameHeightWithSpacing()));
                ImGui::SameLine();
                ImGui::Text("%d",i+1);

                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::InputInt("##pcOff",&ins->n163.wavePosCh[i],1,16)) { PARAMETER
                  if (ins->n163.wavePosCh[i]<0) ins->n163.wavePosCh[i]=0;
                  if (ins->n163.wavePosCh[i]>255) ins->n163.wavePosCh[i]=255;
                }

                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::InputInt("##pcLen",&ins->n163.waveLenCh[i],4,16)) { PARAMETER
                  if (ins->n163.waveLenCh[i]<0) ins->n163.waveLenCh[i]=0;
                  if (ins->n163.waveLenCh[i]>252) ins->n163.waveLenCh[i]=252;
                  ins->n163.waveLenCh[i]&=0xfc;
                }
                ImGui::PopID();
              }

              ImGui::EndTable();
            }
          } else {
            if (ImGui::InputInt("Position##WAVEPOS",&ins->n163.wavePos,1,16)) { PARAMETER
              if (ins->n163.wavePos<0) ins->n163.wavePos=0;
              if (ins->n163.wavePos>255) ins->n163.wavePos=255;
            }
            if (ImGui::InputInt("Length##WAVELEN",&ins->n163.waveLen,4,16)) { PARAMETER
              if (ins->n163.waveLen<0) ins->n163.waveLen=0;
              if (ins->n163.waveLen>252) ins->n163.waveLen=252;
              ins->n163.waveLen&=0xfc;
            }
          }
          ImGui::EndTabItem();
        }
        if (ins->type==DIV_INS_FDS) if (ImGui::BeginTabItem("FDS")) {
          float modTable[32];
          int modTableInt[256];
          ImGui::Checkbox("Compatibility mode",&ins->fds.initModTableWithFirstWave);
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("only use for compatibility with .dmf modules!\n- initializes modulation table with first wavetable\n- does not alter modulation parameters on instrument change");
          }
          if (ImGui::InputInt("Modulation depth",&ins->fds.modDepth,1,4)) {
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
            modTableInt[i]=ins->fds.modTable[i];
          }
          ImVec2 modTableSize=ImVec2(ImGui::GetContentRegionAvail().x,96.0f*dpiScale);
          PlotCustom("ModTable",modTable,32,0,NULL,-4,3,modTableSize,sizeof(float),ImVec4(1.0f,1.0f,1.0f,1.0f),0,NULL,NULL,true);
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
            ImGui::InhibitInertialScroll();
          }

          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x); // wavetable text input size found here
          if (ImGui::InputText("##MMLModTable",&mmlStringModTable)) {
            int discardIt=0;
            memset(modTableInt,0,256*sizeof(int));
            decodeMMLStrW(mmlStringModTable,modTableInt,discardIt,-4,3,false);
            for (int i=0; i<32; i++) {
              if (i>=discardIt) {
                modTableInt[i]=0;
              } else {
                if (modTableInt[i]<-4) modTableInt[i]=-4;
                if (modTableInt[i]>3) modTableInt[i]=3;
              }
              ins->fds.modTable[i]=modTableInt[i];
            }
            MARK_MODIFIED;
          }
          if (!ImGui::IsItemActive()) {
            encodeMMLStr(mmlStringModTable,modTableInt,32,-1,-1,false);
          }
          ImGui::EndTabItem();
        }
        if (ins->type==DIV_INS_VBOY) if (ImGui::BeginTabItem("Virtual Boy")) {
          float modTable[32];
          int modTableInt[256];
          P(ImGui::Checkbox("Set modulation table (channel 5 only)",&ins->fds.initModTableWithFirstWave));

          ImGui::BeginDisabled(!ins->fds.initModTableWithFirstWave);
          for (int i=0; i<32; i++) {
            modTable[i]=ins->fds.modTable[i];
            modTableInt[i]=modTableHex?((unsigned char)ins->fds.modTable[i]):ins->fds.modTable[i];
          }
          ImVec2 modTableSize=ImVec2(ImGui::GetContentRegionAvail().x,256.0f*dpiScale);
          PlotCustom("ModTable",modTable,32,0,NULL,-128,127,modTableSize,sizeof(float),ImVec4(1.0f,1.0f,1.0f,1.0f),0,NULL,NULL,true);
          if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            macroDragStart=ImGui::GetItemRectMin();
            macroDragAreaSize=modTableSize;
            macroDragMin=-128;
            macroDragMax=127;
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
            ImGui::InhibitInertialScroll();
          }
          
          if (ImGui::Button(modTableHex?"Hex##MTHex":"Dec##MTHex")) {
            modTableHex=!modTableHex;
          }
          ImGui::SameLine();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x); // wavetable text input size found here
          if (ImGui::InputText("##MMLModTable",&mmlStringModTable)) {
            int discardIt=0;
            memset(modTableInt,0,256*sizeof(int));
            decodeMMLStrW(mmlStringModTable,modTableInt,discardIt,modTableHex?0:-128,modTableHex?255:127,modTableHex);
            for (int i=0; i<32; i++) {
              if (i>=discardIt) {
                modTableInt[i]=0;
              } else {
                if (modTableInt[i]>=128) modTableInt[i]-=256;
              }
              ins->fds.modTable[i]=modTableInt[i];
            }
            MARK_MODIFIED;
          }
          if (!ImGui::IsItemActive()) {
            encodeMMLStr(mmlStringModTable,modTableInt,32,-1,-1,modTableHex);
          }
          ImGui::SameLine();

          ImGui::EndDisabled();
          ImGui::EndTabItem();
        }
        if (ins->type==DIV_INS_ES5506) if (ImGui::BeginTabItem("ES5506")) {
          if (ImGui::BeginTable("ESParams",2,ImGuiTableFlags_SizingStretchSame)) {
            ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.0);
            ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0);
            // filter
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            P(CWSliderScalar("Filter Mode",ImGuiDataType_U8,&ins->es5506.filter.mode,&_ZERO,&_THREE,es5506FilterModes[ins->es5506.filter.mode&3]));
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            P(CWSliderScalar("Filter K1",ImGuiDataType_U16,&ins->es5506.filter.k1,&_ZERO,&_SIXTY_FIVE_THOUSAND_FIVE_HUNDRED_THIRTY_FIVE)); rightClickable
            ImGui::TableNextColumn();
            P(CWSliderScalar("Filter K2",ImGuiDataType_U16,&ins->es5506.filter.k2,&_ZERO,&_SIXTY_FIVE_THOUSAND_FIVE_HUNDRED_THIRTY_FIVE)); rightClickable
            // envelope
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            P(CWSliderScalar("Envelope count",ImGuiDataType_U16,&ins->es5506.envelope.ecount,&_ZERO,&_FIVE_HUNDRED_ELEVEN)); rightClickable
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            P(CWSliderScalar("Left Volume Ramp",ImGuiDataType_S8,&ins->es5506.envelope.lVRamp,&_MINUS_ONE_HUNDRED_TWENTY_EIGHT,&_ONE_HUNDRED_TWENTY_SEVEN)); rightClickable
            ImGui::TableNextColumn();
            P(CWSliderScalar("Right Volume Ramp",ImGuiDataType_S8,&ins->es5506.envelope.rVRamp,&_MINUS_ONE_HUNDRED_TWENTY_EIGHT,&_ONE_HUNDRED_TWENTY_SEVEN)); rightClickable
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            P(CWSliderScalar("Filter K1 Ramp",ImGuiDataType_S8,&ins->es5506.envelope.k1Ramp,&_MINUS_ONE_HUNDRED_TWENTY_EIGHT,&_ONE_HUNDRED_TWENTY_SEVEN)); rightClickable
            ImGui::TableNextColumn();
            P(CWSliderScalar("Filter K2 Ramp",ImGuiDataType_S8,&ins->es5506.envelope.k2Ramp,&_MINUS_ONE_HUNDRED_TWENTY_EIGHT,&_ONE_HUNDRED_TWENTY_SEVEN)); rightClickable
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Checkbox("K1 Ramp Slowdown",&ins->es5506.envelope.k1Slow);
            ImGui::TableNextColumn();
            ImGui::Checkbox("K2 Ramp Slowdown",&ins->es5506.envelope.k2Slow);
            ImGui::EndTable();
          }
          ImGui::EndTabItem();
        }
        if (ins->type==DIV_INS_MULTIPCM) {
          if (ImGui::BeginTabItem("MultiPCM")) {
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
              P(CWVSliderScalar("##Attack Rate",sliderSize,ImGuiDataType_U8,&ins->multipcm.ar,&_ZERO,&_FIFTEEN)); rightClickable
              ImGui::TableNextColumn();
              P(CWVSliderScalar("##Decay 1 Rate",sliderSize,ImGuiDataType_U8,&ins->multipcm.d1r,&_ZERO,&_FIFTEEN)); rightClickable
              ImGui::TableNextColumn();
              P(CWVSliderScalar("##Decay Level",sliderSize,ImGuiDataType_U8,&ins->multipcm.dl,&_ZERO,&_FIFTEEN)); rightClickable
              ImGui::TableNextColumn();
              P(CWVSliderScalar("##Decay 2 Rate",sliderSize,ImGuiDataType_U8,&ins->multipcm.d2r,&_ZERO,&_FIFTEEN)); rightClickable
              ImGui::TableNextColumn();
              P(CWVSliderScalar("##Release Rate",sliderSize,ImGuiDataType_U8,&ins->multipcm.rr,&_ZERO,&_FIFTEEN)); rightClickable
              ImGui::TableNextColumn();
              P(CWVSliderScalar("##Rate Correction",sliderSize,ImGuiDataType_U8,&ins->multipcm.rc,&_ZERO,&_FIFTEEN)); rightClickable
              ImGui::TableNextColumn();
              drawFMEnv(0,ins->multipcm.ar,ins->multipcm.d1r,ins->multipcm.d2r,ins->multipcm.rr,ins->multipcm.dl,0,0,0,127,15,15,ImVec2(ImGui::GetContentRegionAvail().x,sliderSize.y),ins->type);
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
        if (ins->type==DIV_INS_SNES) if (ImGui::BeginTabItem("SNES")) {
          P(ImGui::Checkbox("Use envelope",&ins->snes.useEnv));
          ImVec2 sliderSize=ImVec2(20.0f*dpiScale,128.0*dpiScale);
          if (ins->snes.useEnv) {
            if (ImGui::BeginTable("SNESEnvParams",ins->snes.sus?6:5,ImGuiTableFlags_NoHostExtendX)) {
              ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
              ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
              ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
              if (ins->snes.sus) {
                ImGui::TableSetupColumn("c2x",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
              }
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
              if (ins->snes.sus) {
                ImGui::TableNextColumn();
                CENTER_TEXT("D2");
                ImGui::TextUnformatted("D2");
              }
              ImGui::TableNextColumn();
              CENTER_TEXT("R");
              ImGui::TextUnformatted("R");
              ImGui::TableNextColumn();
              CENTER_TEXT("Envelope");
              ImGui::TextUnformatted("Envelope");

              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              P(CWVSliderScalar("##Attack",sliderSize,ImGuiDataType_U8,&ins->snes.a,&_ZERO,&_FIFTEEN)); rightClickable
              ImGui::TableNextColumn();
              P(CWVSliderScalar("##Decay",sliderSize,ImGuiDataType_U8,&ins->snes.d,&_ZERO,&_SEVEN)); rightClickable
              ImGui::TableNextColumn();
              P(CWVSliderScalar("##Sustain",sliderSize,ImGuiDataType_U8,&ins->snes.s,&_ZERO,&_SEVEN)); rightClickable
              if (ins->snes.sus) {
                ImGui::TableNextColumn();
                P(CWVSliderScalar("##Decay2",sliderSize,ImGuiDataType_U8,&ins->snes.d2,&_ZERO,&_THIRTY_ONE)); rightClickable
              }
              ImGui::TableNextColumn();
              P(CWVSliderScalar("##Release",sliderSize,ImGuiDataType_U8,&ins->snes.r,&_ZERO,&_THIRTY_ONE)); rightClickable
              ImGui::TableNextColumn();
              drawFMEnv(0,ins->snes.a+1,1+ins->snes.d*2,ins->snes.sus?ins->snes.d2:ins->snes.r,ins->snes.sus?ins->snes.r:31,(14-ins->snes.s*2),(ins->snes.r==0 || (ins->snes.sus && ins->snes.d2==0)),0,0,7,16,31,ImVec2(ImGui::GetContentRegionAvail().x,sliderSize.y),ins->type);

              ImGui::EndTable();
            }
            ImGui::Text("Sustain/release mode:");
            if (ImGui::RadioButton("Direct (cut on release)",ins->snes.sus==0)) {
              ins->snes.sus=0;
            }
            if (ImGui::RadioButton("Effective (linear decrease)",ins->snes.sus==1)) {
              ins->snes.sus=1;
            }
            if (ImGui::RadioButton("Effective (exponential decrease)",ins->snes.sus==2)) {
              ins->snes.sus=2;
            }
            if (ImGui::RadioButton("Delayed (write R on release)",ins->snes.sus==3)) {
              ins->snes.sus=3;
            }
          } else {
            if (ImGui::BeginTable("SNESGainParams",2,ImGuiTableFlags_NoHostExtendX)) {
              ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch);
              ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);

              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              CENTER_TEXT("Gain Mode");
              ImGui::TextUnformatted("Gain Mode");
              ImGui::TableNextColumn();
              CENTER_TEXT("Gain");
              ImGui::TextUnformatted("Gain");

              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              if (ImGui::RadioButton("Direct",ins->snes.gainMode==DivInstrumentSNES::GAIN_MODE_DIRECT)) {
                ins->snes.gainMode=DivInstrumentSNES::GAIN_MODE_DIRECT;
                PARAMETER;
              }
              if (ImGui::RadioButton("Decrease (linear)",ins->snes.gainMode==DivInstrumentSNES::GAIN_MODE_DEC_LINEAR)) {
                ins->snes.gainMode=DivInstrumentSNES::GAIN_MODE_DEC_LINEAR;
                PARAMETER;
              }
              if (ImGui::RadioButton("Decrease (logarithmic)",ins->snes.gainMode==DivInstrumentSNES::GAIN_MODE_DEC_LOG)) {
                ins->snes.gainMode=DivInstrumentSNES::GAIN_MODE_DEC_LOG;
                PARAMETER;
              }
              if (ImGui::RadioButton("Increase (linear)",ins->snes.gainMode==DivInstrumentSNES::GAIN_MODE_INC_LINEAR)) {
                ins->snes.gainMode=DivInstrumentSNES::GAIN_MODE_INC_LINEAR;
                PARAMETER;
              }
              if (ImGui::RadioButton("Increase (bent line)",ins->snes.gainMode==DivInstrumentSNES::GAIN_MODE_INC_INVLOG)) {
                ins->snes.gainMode=DivInstrumentSNES::GAIN_MODE_INC_INVLOG;
                PARAMETER;
              }

              ImGui::TableNextColumn();
              unsigned char gainMax=(ins->snes.gainMode==DivInstrumentSNES::GAIN_MODE_DIRECT)?127:31;
              if (ins->snes.gain>gainMax) ins->snes.gain=gainMax;
              P(CWVSliderScalar("##Gain",sliderSize,ImGuiDataType_U8,&ins->snes.gain,&_ZERO,&gainMax)); rightClickable

              ImGui::EndTable();
            }
            if (ins->snes.gainMode==DivInstrumentSNES::GAIN_MODE_DEC_LINEAR || ins->snes.gainMode==DivInstrumentSNES::GAIN_MODE_DEC_LOG) {
              ImGui::TextWrapped("using decrease modes will not produce any sound at all, unless you know what you are doing.\nit is recommended to use the Gain macro for decrease instead.");
            }
          }
          ImGui::EndTabItem();
        }
        if (ins->type==DIV_INS_GB ||
            (ins->type==DIV_INS_AMIGA && ins->amiga.useWave) ||
            (ins->type==DIV_INS_X1_010 && !ins->amiga.useSample) ||
            ins->type==DIV_INS_N163 ||
            ins->type==DIV_INS_FDS ||
            (ins->type==DIV_INS_SWAN && !ins->amiga.useSample) ||
            (ins->type==DIV_INS_PCE && !ins->amiga.useSample) ||
            (ins->type==DIV_INS_VBOY) ||
            ins->type==DIV_INS_SCC ||
            ins->type==DIV_INS_SNES ||
            ins->type==DIV_INS_NAMCO ||
            ins->type==DIV_INS_SM8521) {
          if (ImGui::BeginTabItem("Wavetable")) {
            switch (ins->type) {
              case DIV_INS_GB:
              case DIV_INS_NAMCO:
              case DIV_INS_SM8521:
              case DIV_INS_SWAN:
                wavePreviewLen=32;
                wavePreviewHeight=15;
                break;
              case DIV_INS_PCE:
                wavePreviewLen=32;
                wavePreviewHeight=31;
                break;
              case DIV_INS_VBOY:
                wavePreviewLen=32;
                wavePreviewHeight=63;
                break;
              case DIV_INS_SCC:
                wavePreviewLen=32;
                wavePreviewHeight=255;
                break;
              case DIV_INS_FDS:
                wavePreviewLen=64;
                wavePreviewHeight=63;
                break;
              case DIV_INS_N163:
                wavePreviewLen=ins->n163.waveLen;
                wavePreviewHeight=15;
                break;
              case DIV_INS_X1_010:
                wavePreviewLen=128;
                wavePreviewHeight=255;
                break;
              case DIV_INS_AMIGA:
                wavePreviewLen=ins->amiga.waveLen+1;
                wavePreviewHeight=255;
                break;
              case DIV_INS_SNES:
                wavePreviewLen=ins->amiga.waveLen+1;
                wavePreviewHeight=15;
                break;
              default:
                wavePreviewLen=32;
                wavePreviewHeight=31;
                break;
            }
            if (ImGui::Checkbox("Enable synthesizer",&ins->ws.enabled)) {
              wavePreviewInit=true;
            }
            if (ins->ws.enabled) {
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
              const bool isSingleWaveFX=(ins->ws.effect>=128);
              if (ImGui::BeginTable("WSPreview",isSingleWaveFX?3:2)) {
                DivWavetable* wave1=e->getWave(ins->ws.wave1);
                DivWavetable* wave2=e->getWave(ins->ws.wave2);
                if (wavePreviewInit) {
                  wavePreview.init(ins,wavePreviewLen,wavePreviewHeight,true);
                  wavePreviewInit=false;
                }
                float wavePreview1[257];
                float wavePreview2[257];
                float wavePreview3[257];
                for (int i=0; i<wave1->len; i++) {
                  if (wave1->data[i]>wave1->max) {
                    wavePreview1[i]=wave1->max;
                  } else {
                    wavePreview1[i]=wave1->data[i];
                  }
                }
                if (wave1->len>0) {
                  wavePreview1[wave1->len]=wave1->data[wave1->len-1];
                }
                for (int i=0; i<wave2->len; i++) {
                  if (wave2->data[i]>wave2->max) {
                    wavePreview2[i]=wave2->max;
                  } else {
                    wavePreview2[i]=wave2->data[i];
                  }
                }
                if (wave2->len>0) {
                  wavePreview2[wave2->len]=wave2->data[wave2->len-1];
                }
                if (ins->ws.enabled && (!wavePreviewPaused || wavePreviewInit)) {
                  wavePreview.tick(true);
                  WAKE_UP;
                }
                for (int i=0; i<wavePreviewLen; i++) {
                  wavePreview3[i]=wavePreview.output[i];
                }
                if (wavePreviewLen>0) {
                  wavePreview3[wavePreviewLen]=wavePreview3[wavePreviewLen-1];
                }

                float ySize=(isSingleWaveFX?96.0f:128.0f)*dpiScale;

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImVec2 size1=ImVec2(ImGui::GetContentRegionAvail().x,ySize);
                PlotNoLerp("##WaveformP1",wavePreview1,wave1->len+1,0,"Wave 1",0,wave1->max,size1);
                if (isSingleWaveFX) {
                  ImGui::TableNextColumn();
                  ImVec2 size2=ImVec2(ImGui::GetContentRegionAvail().x,ySize);
                  PlotNoLerp("##WaveformP2",wavePreview2,wave2->len+1,0,"Wave 2",0,wave2->max,size2);
                }
                ImGui::TableNextColumn();
                ImVec2 size3=ImVec2(ImGui::GetContentRegionAvail().x,ySize);
                PlotNoLerp("##WaveformP3",wavePreview3,wavePreviewLen+1,0,"Result",0,wavePreviewHeight,size3);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                if (ins->std.waveMacro.len>0) {
                  ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_WARNING]);
                  ImGui::AlignTextToFramePadding();
                  ImGui::Text("Wave 1 " ICON_FA_EXCLAMATION_TRIANGLE);
                  ImGui::PopStyleColor();
                  if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("waveform macro is controlling wave 1!\nthis value will be ineffective.");
                  }
                } else {
                  ImGui::AlignTextToFramePadding();
                  ImGui::Text("Wave 1");
                }
                ImGui::SameLine();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::InputInt("##SelWave1",&ins->ws.wave1,1,4)) {
                  if (ins->ws.wave1<0) ins->ws.wave1=0;
                  if (ins->ws.wave1>=(int)e->song.wave.size()) ins->ws.wave1=e->song.wave.size()-1;
                  wavePreviewInit=true;
                }
                if (ins->std.waveMacro.len>0) {
                  if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("waveform macro is controlling wave 1!\nthis value will be ineffective.");
                  }
                }
                if (isSingleWaveFX) {
                  ImGui::TableNextColumn();
                  ImGui::AlignTextToFramePadding();
                  ImGui::Text("Wave 2");
                  ImGui::SameLine();
                  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                  if (ImGui::InputInt("##SelWave2",&ins->ws.wave2,1,4)) {
                    if (ins->ws.wave2<0) ins->ws.wave2=0;
                    if (ins->ws.wave2>=(int)e->song.wave.size()) ins->ws.wave2=e->song.wave.size()-1;
                    wavePreviewInit=true;
                  }
                }
                ImGui::TableNextColumn();
                if (ImGui::Button(wavePreviewPaused?(ICON_FA_PLAY "##WSPause"):(ICON_FA_PAUSE "##WSPause"))) {
                  wavePreviewPaused=!wavePreviewPaused;
                }
                if (ImGui::IsItemHovered()) {
                  if (wavePreviewPaused) {
                    ImGui::SetTooltip("Resume preview");
                  } else {
                    ImGui::SetTooltip("Pause preview");
                  }
                }
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_REPEAT "##WSRestart")) {
                  wavePreviewInit=true;
                }
                if (ImGui::IsItemHovered()) {
                  ImGui::SetTooltip("Restart preview");
                }
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_UPLOAD "##WSCopy")) {
                  curWave=e->addWave();
                  if (curWave==-1) {
                    showError("too many wavetables!");
                  } else {
                    wantScrollList=true;
                    MARK_MODIFIED;
                    RESET_WAVE_MACRO_ZOOM;
                    nextWindow=GUI_WINDOW_WAVE_EDIT;

                    DivWavetable* copyWave=e->song.wave[curWave];
                    copyWave->len=wavePreviewLen;
                    copyWave->max=wavePreviewHeight;
                    memcpy(copyWave->data,wavePreview.output,256*sizeof(int));
                  }
                }
                if (ImGui::IsItemHovered()) {
                  ImGui::SetTooltip("Copy to new wavetable");
                }
                ImGui::SameLine();
                ImGui::Text("(%d×%d)",wavePreviewLen,wavePreviewHeight+1);
                ImGui::EndTable();
              }

              if (ImGui::InputScalar("Update Rate",ImGuiDataType_U8,&ins->ws.rateDivider,&_ONE,&_EIGHT)) {
                wavePreviewInit=true;
              }
              int speed=ins->ws.speed+1;
              if (ImGui::InputInt("Speed",&speed,1,8)) {
                if (speed<1) speed=1;
                if (speed>256) speed=256;
                ins->ws.speed=speed-1;
                wavePreviewInit=true;
              }

              if (ImGui::InputScalar("Amount",ImGuiDataType_U8,&ins->ws.param1,&_ONE,&_EIGHT)) {
                wavePreviewInit=true;
              }

              if (ins->ws.effect==DIV_WS_PHASE_MOD) {
                if (ImGui::InputScalar("Power",ImGuiDataType_U8,&ins->ws.param2,&_ONE,&_EIGHT)) {
                  wavePreviewInit=true;
                }
              }

              if (ImGui::Checkbox("Global",&ins->ws.global)) {
                wavePreviewInit=true;
              }
            } else {
              ImGui::TextWrapped("wavetable synthesizer disabled.\nuse the Waveform macro to set the wave for this instrument.");
            }

            ImGui::EndTabItem();
          }
        }
        if (ImGui::BeginTabItem("Macros")) {
          const char* volumeLabel="Volume";

          int volMax=15;
          int volMin=0;
          if (ins->type==DIV_INS_PCE || ins->type==DIV_INS_AY8930 || ins->type==DIV_INS_SM8521) {
            volMax=31;
          }
          if (ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_VERA || ins->type==DIV_INS_VRC6_SAW) {
            volMax=63;
          }
          if (ins->type==DIV_INS_AMIGA) {
            volMax=64;
          }
          if (ins->type==DIV_INS_FM || ins->type==DIV_INS_SEGAPCM || ins->type==DIV_INS_MIKEY ||
              ins->type==DIV_INS_MULTIPCM || ins->type==DIV_INS_SU || ins->type==DIV_INS_OPZ ||
              ins->type==DIV_INS_OPM || ins->type==DIV_INS_SNES || ins->type==DIV_INS_MSM5232 ||
              ins->type==DIV_INS_K053260) {
            volMax=127;
          }
          if (ins->type==DIV_INS_GB) {
            if (ins->gb.softEnv) {
              volMax=15;
            } else {
              volMax=0;
            }
          }
          if (ins->type==DIV_INS_PET || ins->type==DIV_INS_BEEPER || ins->type==DIV_INS_PV1000) {
            volMax=1;
          }
          if (ins->type==DIV_INS_FDS) {
            volMax=32;
          }
          if (ins->type==DIV_INS_ES5506) {
            volMax=4095;
          }
          if (ins->type==DIV_INS_MSM6258) {
            volMax=0;
          }
          if (ins->type==DIV_INS_MSM6295 || ins->type==DIV_INS_TED) {
            volMax=8;
          }
          if (ins->type==DIV_INS_ADPCMA) {
            volMax=31;
          }
          if (ins->type==DIV_INS_ADPCMB || ins->type==DIV_INS_YMZ280B || ins->type==DIV_INS_RF5C68 ||
              ins->type==DIV_INS_GA20 || ins->type==DIV_INS_C140 || ins->type==DIV_INS_C219) {
            volMax=255;
          }
          if (ins->type==DIV_INS_QSOUND) {
            volMax=16383;
          }
          if (ins->type==DIV_INS_POKEMINI) {
            volMax=2;
          }

          const char* dutyLabel="Duty/Noise";
          int dutyMin=0;
          int dutyMax=3;
          if (ins->type==DIV_INS_C64) {
            dutyLabel="Duty";
            if (ins->c64.dutyIsAbs) {
              dutyMax=4095;
            } else {
              dutyMin=-4095;
              dutyMax=4095;
            }
          }
          if (ins->type==DIV_INS_STD) {
            dutyLabel="Duty";
          }
          if (ins->type==DIV_INS_OPM || ins->type==DIV_INS_OPZ) {
            dutyMax=32;
          }
          if (ins->type==DIV_INS_AY) {
            dutyMax=ins->amiga.useSample?0:31;
          }
          if (ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930 || ins->type==DIV_INS_FM || ins->type==DIV_INS_OPM) {
            dutyLabel="Noise Freq";
          }
          if (ins->type==DIV_INS_POKEY) {
            dutyLabel="AUDCTL";
            dutyMax=8;
          }
          if (ins->type==DIV_INS_MIKEY) {
            dutyLabel="Duty/Int";
            dutyMax=ins->amiga.useSample?0:10;
          }
          if (ins->type==DIV_INS_MSM5232) {
            dutyLabel="Group Ctrl";
            dutyMax=5;
          }
          if (ins->type==DIV_INS_C219) {
            dutyLabel="Control";
            dutyMax=3;
          }
          if (ins->type==DIV_INS_BEEPER || ins->type==DIV_INS_POKEMINI) {
            dutyLabel="Pulse Width";
            dutyMax=255;
          }
          if (ins->type==DIV_INS_T6W28) {
            dutyLabel="Noise Type";
            dutyMax=1;
          }
          if (ins->type==DIV_INS_AY8930) {
            dutyMax=ins->amiga.useSample?0:255;
          }
          if (ins->type==DIV_INS_TIA || ins->type==DIV_INS_AMIGA || ins->type==DIV_INS_SCC ||
              ins->type==DIV_INS_PET || ins->type==DIV_INS_SEGAPCM ||
              ins->type==DIV_INS_FM || ins->type==DIV_INS_K007232 || ins->type==DIV_INS_GA20 ||
              ins->type==DIV_INS_SM8521 || ins->type==DIV_INS_PV1000 || ins->type==DIV_INS_K053260 ||
              ins->type==DIV_INS_C140) {
            dutyMax=0;
          }
          if (ins->type==DIV_INS_VBOY) {
            dutyLabel="Noise Length";
            dutyMax=7;
          }
          if (ins->type==DIV_INS_PCE) {
            dutyLabel="Noise";
            dutyMax=(!ins->amiga.useSample)?1:0;
          }
          if (ins->type==DIV_INS_NAMCO) {
            dutyLabel="Noise";
            dutyMax=1;
          }
          if (ins->type==DIV_INS_VIC) {
            dutyLabel="On/Off";
            dutyMax=1;
          }
          if (ins->type==DIV_INS_TED) {
            dutyLabel="Square/Noise";
            dutyMax=2;
          }
          if (ins->type==DIV_INS_SWAN) {
            dutyLabel="Noise";
            dutyMax=ins->amiga.useSample?0:8;
          }
          if (ins->type==DIV_INS_SNES) {
            dutyLabel="Noise Freq";
            dutyMax=31;
          }
          if (ins->type==DIV_INS_OPLL || ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS ||
              ins->type==DIV_INS_VRC6_SAW || ins->type==DIV_INS_FDS || ins->type==DIV_INS_MULTIPCM) {
            dutyMax=0;
          }
          if (ins->type==DIV_INS_VERA) {
            dutyLabel="Duty";
            dutyMax=63;
          }
          if (ins->type==DIV_INS_N163) {
            dutyLabel="Wave Pos";
            dutyMax=255;
          }
          if (ins->type==DIV_INS_VRC6) {
            dutyLabel="Duty";
            dutyMax=ins->amiga.useSample?0:7;
          }
          if (ins->type==DIV_INS_ES5506) {
            dutyLabel="Filter Mode";
            dutyMax=3;
          }
          if (ins->type==DIV_INS_SU) {
            dutyMax=127;
          }
          if (ins->type==DIV_INS_ES5506) {
            dutyLabel="Filter Mode";
            dutyMax=3;
          }
          if (ins->type==DIV_INS_MSM6258) {
            dutyLabel="Frequency Divider";
            dutyMax=2;
          }
          if (ins->type==DIV_INS_MSM6295) {
            dutyLabel="Frequency Divider";
            dutyMax=1;
          }
          if (ins->type==DIV_INS_ADPCMA) {
            dutyLabel="Global Volume";
            dutyMax=63;
          }
          if (ins->type==DIV_INS_ADPCMB || ins->type==DIV_INS_YMZ280B || ins->type==DIV_INS_RF5C68) {
            dutyMax=0;
          }
          if (ins->type==DIV_INS_QSOUND) {
            dutyLabel="Echo Level";
            dutyMax=32767;
          }

          const char* waveLabel="Waveform";
          int waveMax=(ins->type==DIV_INS_VERA)?3:(MAX(1,e->song.waveLen-1));
          bool waveBitMode=false;
          if (ins->type==DIV_INS_C64 || ins->type==DIV_INS_SAA1099) {
            waveBitMode=true;
          }
          if (ins->type==DIV_INS_STD || ins->type==DIV_INS_VRC6_SAW || ins->type==DIV_INS_NES ||
              ins->type==DIV_INS_T6W28 || ins->type==DIV_INS_PV1000)
              waveMax=0;
          if (ins->type==DIV_INS_TIA || ins->type==DIV_INS_VIC || ins->type==DIV_INS_OPLL) waveMax=15;
          if (ins->type==DIV_INS_C64) waveMax=4;
          if (ins->type==DIV_INS_SAA1099) waveMax=2;
          if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM) waveMax=0;
          if (ins->type==DIV_INS_MIKEY) waveMax=0;
          if (ins->type==DIV_INS_MULTIPCM) waveMax=0;
          if (ins->type==DIV_INS_ADPCMA) waveMax=0;
          if (ins->type==DIV_INS_ADPCMB) waveMax=0;
          if (ins->type==DIV_INS_QSOUND) waveMax=0;
          if (ins->type==DIV_INS_YMZ280B) waveMax=0;
          if (ins->type==DIV_INS_RF5C68) waveMax=0;
          if (ins->type==DIV_INS_MSM5232) waveMax=0;
          if (ins->type==DIV_INS_MSM6258) waveMax=0;
          if (ins->type==DIV_INS_MSM6295) waveMax=0;
          if (ins->type==DIV_INS_SEGAPCM) waveMax=0;
          if (ins->type==DIV_INS_K007232) waveMax=0;
          if (ins->type==DIV_INS_ES5506) waveMax=0;
          if (ins->type==DIV_INS_GA20) waveMax=0;
          if (ins->type==DIV_INS_K053260) waveMax=0;
          if (ins->type==DIV_INS_BEEPER) waveMax=0;
          if (ins->type==DIV_INS_POKEMINI) waveMax=0;
          if (ins->type==DIV_INS_TED) waveMax=0;
          if (ins->type==DIV_INS_C140) waveMax=0;
          if (ins->type==DIV_INS_C219) waveMax=0;
          if (ins->type==DIV_INS_SU || ins->type==DIV_INS_POKEY) waveMax=7;
          if (ins->type==DIV_INS_PET) {
            waveMax=8;
            waveBitMode=true;
          }
          if (ins->type==DIV_INS_VRC6) {
            waveMax=ins->amiga.useSample?(MAX(1,e->song.waveLen-1)):0;
          }
          
          if (ins->type==DIV_INS_OPLL) {
            waveLabel="Patch";
          }

          if (ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930) {
            waveMax=ins->amiga.useSample?0:3;
            waveBitMode=ins->amiga.useSample?false:true;
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
            ex1Max=ins->amiga.useSample?0:7;
            ex2Max=ins->amiga.useSample?0:255;
            ex2Bit=false;
          }
          if (ins->type==DIV_INS_N163) {
            ex1Max=252;
          }
          if (ins->type==DIV_INS_FDS) {
            ex1Max=63;
            ex2Max=4095;
          }
          if (ins->type==DIV_INS_SU) {
            ex1Max=16383;
            ex2Max=255;
          }
          if (ins->type==DIV_INS_MSM6258) {
            ex1Max=1;
          }
          if (ins->type==DIV_INS_QSOUND) {
            ex1Max=16383;
            ex2Max=2725;
          }
          if (ins->type==DIV_INS_SAA1099) ex1Max=8;
          if (ins->type==DIV_INS_ES5506) {
            ex1Max=65535;
            ex2Max=65535;
          }
          if (ins->type==DIV_INS_SNES) {
            ex1Max=5;
            ex2Max=255;
          }
          if (ins->type==DIV_INS_MSM5232) {
            ex1Max=5;
            ex2Max=11;
          }

          int panMin=0;
          int panMax=0;
          bool panSingle=false;
          bool panSingleNoBit=false;
          if (ins->type==DIV_INS_STD ||//Game Gear
              ins->type==DIV_INS_FM ||
              ins->type==DIV_INS_OPM ||
              ins->type==DIV_INS_GB ||
              ins->type==DIV_INS_OPZ ||
              ins->type==DIV_INS_MSM6258 ||
              ins->type==DIV_INS_VERA ||
              ins->type==DIV_INS_ADPCMA ||
              ins->type==DIV_INS_ADPCMB) {
            panMax=2;
            panSingle=true;
          }
          if (ins->type==DIV_INS_OPL ||
              ins->type==DIV_INS_OPL_DRUMS) {
            panMax=4;
            panSingle=true;
          }
          if (ins->type==DIV_INS_X1_010 || ins->type==DIV_INS_PCE || ins->type==DIV_INS_MIKEY ||
              ins->type==DIV_INS_SAA1099 || ins->type==DIV_INS_NAMCO || ins->type==DIV_INS_RF5C68 ||
              ins->type==DIV_INS_VBOY || ins->type==DIV_INS_T6W28 || ins->type==DIV_INS_K007232) {
            panMax=15;
          }
          if (ins->type==DIV_INS_SEGAPCM) {
            panMax=127;
          }
          if (ins->type==DIV_INS_AMIGA) {
            if (ins->std.panLMacro.mode) {
              panMin=-16;
              panMax=16;
            } else {
              panMin=0;
              panMax=127;
            }
          }
          if (ins->type==DIV_INS_QSOUND) {
            panMin=-16;
            panMax=16;
            panSingleNoBit=true;
          }
          if (ins->type==DIV_INS_MULTIPCM || ins->type==DIV_INS_YMZ280B) {
            panMin=-7;
            panMax=7;
            panSingleNoBit=true;
          }
          if (ins->type==DIV_INS_K053260) {
            panMin=-3;
            panMax=3;
            panSingleNoBit=true;
          }
          if (ins->type==DIV_INS_SU) {
            panMin=-127;
            panMax=127;
            panSingleNoBit=true;
          }
          if (ins->type==DIV_INS_SNES) {
            panMin=0;
            panMax=127;
          }
          if (ins->type==DIV_INS_C140 || ins->type==DIV_INS_C219) {
            panMin=0;
            panMax=255;
          }
          if (ins->type==DIV_INS_ES5506) {
            panMax=4095;
          }

          if (volMax>0) {
            macroList.push_back(FurnaceGUIMacroDesc(volumeLabel,&ins->std.volMacro,volMin,volMax,160,uiColors[GUI_COLOR_MACRO_VOLUME]));
          }
          if (ins->type!=DIV_INS_MSM6258 && ins->type!=DIV_INS_MSM6295 && ins->type!=DIV_INS_ADPCMA) {
            macroList.push_back(FurnaceGUIMacroDesc("Arpeggio",&ins->std.arpMacro,-120,120,160,uiColors[GUI_COLOR_MACRO_PITCH],true,NULL,macroHoverNote,false,NULL,0,true,ins->std.arpMacro.val));
          }
          if (dutyMax>0) {
            if (ins->type==DIV_INS_MIKEY) {
              macroList.push_back(FurnaceGUIMacroDesc(dutyLabel,&ins->std.dutyMacro,0,dutyMax,160,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,mikeyFeedbackBits));
            } else if (ins->type==DIV_INS_POKEY) {
              macroList.push_back(FurnaceGUIMacroDesc(dutyLabel,&ins->std.dutyMacro,0,dutyMax,160,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,pokeyCtlBits));
            } else if (ins->type==DIV_INS_TED) {
              macroList.push_back(FurnaceGUIMacroDesc(dutyLabel,&ins->std.dutyMacro,0,dutyMax,80,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,tedControlBits));
            } else if (ins->type==DIV_INS_MSM5232) {
              macroList.push_back(FurnaceGUIMacroDesc(dutyLabel,&ins->std.dutyMacro,0,dutyMax,160,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,msm5232ControlBits));
            } else if (ins->type==DIV_INS_ES5506) {
              macroList.push_back(FurnaceGUIMacroDesc(dutyLabel,&ins->std.dutyMacro,dutyMin,dutyMax,160,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,&macroHoverES5506FilterMode));
            } else if (ins->type==DIV_INS_C219) {
              macroList.push_back(FurnaceGUIMacroDesc(dutyLabel,&ins->std.dutyMacro,0,dutyMax,120,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,c219ControlBits));
            } else {
              macroList.push_back(FurnaceGUIMacroDesc(dutyLabel,&ins->std.dutyMacro,dutyMin,dutyMax,160,uiColors[GUI_COLOR_MACRO_OTHER]));
            }
          }
          if (waveMax>0) {
            macroList.push_back(FurnaceGUIMacroDesc(waveLabel,&ins->std.waveMacro,0,waveMax,(waveBitMode && ins->type!=DIV_INS_PET)?64:160,uiColors[GUI_COLOR_MACRO_WAVE],false,NULL,NULL,waveBitMode,waveNames,((ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930)?1:0)));
          }
          if (panMax>0) {
            if (panSingle) {
              macroList.push_back(FurnaceGUIMacroDesc("Panning",&ins->std.panLMacro,0,panMax,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,panBits));
            } else if (ins->type==DIV_INS_QSOUND) {
              macroList.push_back(FurnaceGUIMacroDesc("Panning",&ins->std.panLMacro,panMin,panMax,CLAMP(31+panMax-panMin,32,160),uiColors[GUI_COLOR_MACRO_OTHER]));
              macroList.push_back(FurnaceGUIMacroDesc("Surround",&ins->std.panRMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
            } else {
              if (panSingleNoBit || (ins->type==DIV_INS_AMIGA && ins->std.panLMacro.mode)) {
                macroList.push_back(FurnaceGUIMacroDesc("Panning",&ins->std.panLMacro,panMin,panMax,CLAMP(31+panMax-panMin,32,160),uiColors[GUI_COLOR_MACRO_OTHER],false,(ins->type==DIV_INS_AMIGA)?macroQSoundMode:NULL));
              } else {
                macroList.push_back(FurnaceGUIMacroDesc("Panning (left)",&ins->std.panLMacro,panMin,panMax,CLAMP(31+panMax-panMin,32,160),uiColors[GUI_COLOR_MACRO_OTHER],false,(ins->type==DIV_INS_AMIGA)?macroQSoundMode:NULL));
              }
              if (!panSingleNoBit) {
                if (ins->type==DIV_INS_AMIGA && ins->std.panLMacro.mode) {
                  macroList.push_back(FurnaceGUIMacroDesc("Surround",&ins->std.panRMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
                } else {
                  macroList.push_back(FurnaceGUIMacroDesc("Panning (right)",&ins->std.panRMacro,panMin,panMax,CLAMP(31+panMax-panMin,32,160),uiColors[GUI_COLOR_MACRO_OTHER]));
                }
              }
            }
          }
          if (ins->type!=DIV_INS_MSM5232 && ins->type!=DIV_INS_MSM6258 && ins->type!=DIV_INS_MSM6295 && ins->type!=DIV_INS_ADPCMA) {
            macroList.push_back(FurnaceGUIMacroDesc("Pitch",&ins->std.pitchMacro,-2048,2047,160,uiColors[GUI_COLOR_MACRO_PITCH],true,macroRelativeMode));
          }
          if (ins->type==DIV_INS_FM ||
              ins->type==DIV_INS_OPM ||
              ins->type==DIV_INS_STD ||
              ins->type==DIV_INS_NES ||
              ins->type==DIV_INS_OPL ||
              ins->type==DIV_INS_OPL_DRUMS ||
              ins->type==DIV_INS_OPZ ||
              ins->type==DIV_INS_PCE ||
              ins->type==DIV_INS_GB ||
              ins->type==DIV_INS_MSM6258 ||
              ins->type==DIV_INS_MSM6295 ||
              ins->type==DIV_INS_ADPCMA ||
              ins->type==DIV_INS_ADPCMB ||
              ins->type==DIV_INS_SEGAPCM ||
              ins->type==DIV_INS_QSOUND ||
              ins->type==DIV_INS_YMZ280B ||
              ins->type==DIV_INS_RF5C68 ||
              ins->type==DIV_INS_AMIGA ||
              ins->type==DIV_INS_OPLL ||
              ins->type==DIV_INS_AY ||
              ins->type==DIV_INS_AY8930 ||
              ins->type==DIV_INS_SWAN ||
              ins->type==DIV_INS_MULTIPCM ||
              (ins->type==DIV_INS_VRC6 && ins->amiga.useSample) ||
              ins->type==DIV_INS_SU ||
              ins->type==DIV_INS_MIKEY ||
              ins->type==DIV_INS_ES5506 ||
              ins->type==DIV_INS_T6W28 ||
              ins->type==DIV_INS_VBOY ||
              (ins->type==DIV_INS_X1_010 && ins->amiga.useSample) ||
              ins->type==DIV_INS_K007232 ||
              ins->type==DIV_INS_GA20 ||
              ins->type==DIV_INS_K053260 ||
              ins->type==DIV_INS_C140 ||
              ins->type==DIV_INS_C219 ||
              ins->type==DIV_INS_TED) {
            macroList.push_back(FurnaceGUIMacroDesc("Phase Reset",&ins->std.phaseResetMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
          }
          if (ex1Max>0) {
            if (ins->type==DIV_INS_C64) {
              int cutoffMin=-2047;
              int cutoffMax=2047;

              if (ins->c64.filterIsAbs) {
                cutoffMin=0;
                cutoffMax=2047;
              }
              macroList.push_back(FurnaceGUIMacroDesc("Cutoff",&ins->std.algMacro,cutoffMin,cutoffMax,160,uiColors[GUI_COLOR_MACRO_OTHER]));
              macroList.push_back(FurnaceGUIMacroDesc("Filter Mode",&ins->std.ex1Macro,0,ex1Max,64,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,filtModeBits));
            } else if (ins->type==DIV_INS_SAA1099) {
              macroList.push_back(FurnaceGUIMacroDesc("Envelope",&ins->std.ex1Macro,0,ex1Max,160,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,saaEnvBits));
            } else if (ins->type==DIV_INS_X1_010 && !ins->amiga.useSample) {
              macroList.push_back(FurnaceGUIMacroDesc("Envelope Mode",&ins->std.ex1Macro,0,ex1Max,160,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,x1_010EnvBits));
            } else if (ins->type==DIV_INS_N163) {
              macroList.push_back(FurnaceGUIMacroDesc("Wave Length",&ins->std.ex1Macro,0,ex1Max,160,uiColors[GUI_COLOR_MACRO_OTHER]));
            } else if (ins->type==DIV_INS_FDS) {
              macroList.push_back(FurnaceGUIMacroDesc("Mod Depth",&ins->std.ex1Macro,0,ex1Max,160,uiColors[GUI_COLOR_MACRO_OTHER]));
            } else if (ins->type==DIV_INS_SU) {
              macroList.push_back(FurnaceGUIMacroDesc("Cutoff",&ins->std.ex1Macro,0,ex1Max,160,uiColors[GUI_COLOR_MACRO_OTHER]));
            } else if (ins->type==DIV_INS_ES5506) {
              macroList.push_back(FurnaceGUIMacroDesc("Filter K1",&ins->std.ex1Macro,((ins->std.ex1Macro.mode==1)?(-ex1Max):0),ex1Max,160,uiColors[GUI_COLOR_MACRO_OTHER],false,macroRelativeMode));
            } else if (ins->type==DIV_INS_MSM6258) {
              macroList.push_back(FurnaceGUIMacroDesc("Clock Divider",&ins->std.ex1Macro,0,ex1Max,160,uiColors[GUI_COLOR_MACRO_OTHER]));
            } else if (ins->type==DIV_INS_QSOUND) {
              macroList.push_back(FurnaceGUIMacroDesc("Echo Feedback",&ins->std.ex1Macro,0,ex1Max,160,uiColors[GUI_COLOR_MACRO_OTHER]));
            } else if (ins->type==DIV_INS_SNES) {
              macroList.push_back(FurnaceGUIMacroDesc("Special",&ins->std.ex1Macro,0,ex1Max,96,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,snesModeBits));
            } else if (ins->type==DIV_INS_MSM5232) {
              macroList.push_back(FurnaceGUIMacroDesc("Group Attack",&ins->std.ex1Macro,0,ex1Max,96,uiColors[GUI_COLOR_MACRO_OTHER]));
            } else {
              macroList.push_back(FurnaceGUIMacroDesc("Duty",&ins->std.ex1Macro,0,ex1Max,160,uiColors[GUI_COLOR_MACRO_OTHER]));
            }
          }
          if (ex2Max>0) {
            if (ins->type==DIV_INS_C64) {
              macroList.push_back(FurnaceGUIMacroDesc("Resonance",&ins->std.ex2Macro,0,ex2Max,64,uiColors[GUI_COLOR_MACRO_OTHER]));
            } else if (ins->type==DIV_INS_FDS) {
              macroList.push_back(FurnaceGUIMacroDesc("Mod Speed",&ins->std.ex2Macro,0,ex2Max,160,uiColors[GUI_COLOR_MACRO_OTHER]));
            } else if (ins->type==DIV_INS_SU) {
              macroList.push_back(FurnaceGUIMacroDesc("Resonance",&ins->std.ex2Macro,0,ex2Max,160,uiColors[GUI_COLOR_MACRO_OTHER]));
            } else if (ins->type==DIV_INS_ES5506) {
              macroList.push_back(FurnaceGUIMacroDesc("Filter K2",&ins->std.ex2Macro,((ins->std.ex2Macro.mode==1)?(-ex2Max):0),ex2Max,160,uiColors[GUI_COLOR_MACRO_OTHER],false,macroRelativeMode));
            } else if (ins->type==DIV_INS_QSOUND) {
              macroList.push_back(FurnaceGUIMacroDesc("Echo Length",&ins->std.ex2Macro,0,ex2Max,160,uiColors[GUI_COLOR_MACRO_OTHER]));
            } else if (ins->type==DIV_INS_SNES) {
              macroList.push_back(FurnaceGUIMacroDesc("Gain",&ins->std.ex2Macro,0,ex2Max,256,uiColors[GUI_COLOR_MACRO_VOLUME],false,NULL,macroHoverGain,false));
            } else if (ins->type==DIV_INS_MSM5232) {
              macroList.push_back(FurnaceGUIMacroDesc("Group Decay",&ins->std.ex2Macro,0,ex2Max,160,uiColors[GUI_COLOR_MACRO_OTHER]));
            } else {
              macroList.push_back(FurnaceGUIMacroDesc("Envelope",&ins->std.ex2Macro,0,ex2Max,ex2Bit?64:160,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,ex2Bit,ayEnvBits));
            }
          }
          if (ins->type==DIV_INS_C64) {
            macroList.push_back(FurnaceGUIMacroDesc("Special",&ins->std.ex4Macro,0,4,64,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,c64TestGateBits));
            macroList.push_back(FurnaceGUIMacroDesc("Attack",&ins->std.ex5Macro,0,15,128,uiColors[GUI_COLOR_MACRO_OTHER]));
            macroList.push_back(FurnaceGUIMacroDesc("Decay",&ins->std.ex6Macro,0,15,128,uiColors[GUI_COLOR_MACRO_OTHER]));
            macroList.push_back(FurnaceGUIMacroDesc("Sustain",&ins->std.ex7Macro,0,15,128,uiColors[GUI_COLOR_MACRO_OTHER]));
            macroList.push_back(FurnaceGUIMacroDesc("Release",&ins->std.ex8Macro,0,15,128,uiColors[GUI_COLOR_MACRO_OTHER]));
          }
          if (ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930 || (ins->type==DIV_INS_X1_010 && !ins->amiga.useSample)) {
            macroList.push_back(FurnaceGUIMacroDesc("AutoEnv Num",&ins->std.ex3Macro,0,15,160,uiColors[GUI_COLOR_MACRO_OTHER]));
            macroList.push_back(FurnaceGUIMacroDesc("AutoEnv Den",&ins->std.algMacro,0,15,160,uiColors[GUI_COLOR_MACRO_OTHER]));
          }
          if (ins->type==DIV_INS_AY8930) {
            // oh my i am running out of macros
            macroList.push_back(FurnaceGUIMacroDesc("Noise AND Mask",&ins->std.fbMacro,0,8,96,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
            macroList.push_back(FurnaceGUIMacroDesc("Noise OR Mask",&ins->std.fmsMacro,0,8,96,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
          }
          if (ins->type==DIV_INS_FDS) {
            macroList.push_back(FurnaceGUIMacroDesc("Mod Position",&ins->std.ex3Macro,0,127,160,uiColors[GUI_COLOR_MACRO_OTHER]));
          }
          if (ins->type==DIV_INS_SU) {
            macroList.push_back(FurnaceGUIMacroDesc("Control",&ins->std.ex3Macro,0,4,64,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,suControlBits));
            macroList.push_back(FurnaceGUIMacroDesc("Phase Reset Timer",&ins->std.ex4Macro,0,65535,160,uiColors[GUI_COLOR_MACRO_OTHER])); // again reuse code from resonance macro but use ex4 instead
          }
          if (ins->type==DIV_INS_ES5506) {
            /*macroList.push_back(FurnaceGUIMacroDesc("Envelope counter",&ins->std.ex3Macro,0,511,160,uiColors[GUI_COLOR_MACRO_OTHER]));
            macroList.push_back(FurnaceGUIMacroDesc("Envelope left volume ramp",&ins->std.ex4Macro,-128,127,160,uiColors[GUI_COLOR_MACRO_OTHER]));
            macroList.push_back(FurnaceGUIMacroDesc("Envelope right volume ramp",&ins->std.ex5Macro,-128,127,160,uiColors[GUI_COLOR_MACRO_OTHER]));
            macroList.push_back(FurnaceGUIMacroDesc("Envelope K1 ramp",&ins->std.ex6Macro,-128,127,160,uiColors[GUI_COLOR_MACRO_OTHER]));
            macroList.push_back(FurnaceGUIMacroDesc("Envelope K2 ramp",&ins->std.ex7Macro,-128,127,160,uiColors[GUI_COLOR_MACRO_OTHER]));
            macroList.push_back(FurnaceGUIMacroDesc("Envelope mode",&ins->std.ex8Macro,0,2,64,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,es5506EnvelopeModes));*/
            macroList.push_back(FurnaceGUIMacroDesc("Outputs",&ins->std.fbMacro,0,5,64,uiColors[GUI_COLOR_MACRO_OTHER]));
            macroList.push_back(FurnaceGUIMacroDesc("Control",&ins->std.algMacro,0,2,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,es5506ControlModes));
          }
          if (ins->type==DIV_INS_MSM5232) {
            macroList.push_back(FurnaceGUIMacroDesc("Noise",&ins->std.ex3Macro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
          }

          drawMacros(macroList,macroEditStateMacros);
          ImGui::EndTabItem();
        }
        if (ins->type==DIV_INS_NES ||
            ins->type==DIV_INS_AY ||
            ins->type==DIV_INS_AY8930 ||
            ins->type==DIV_INS_MIKEY ||
            ins->type==DIV_INS_PCE ||
            ins->type==DIV_INS_X1_010 ||
            ins->type==DIV_INS_SWAN ||
            ins->type==DIV_INS_VRC6) {
          insTabSample(ins);
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
        lastMacroDesc.macro->loop=255;
        lastMacroDesc.macro->rel=255;
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

          if (lastMacroDesc.macro->loop<lastMacroDesc.macro->len) {
            lastMacroDesc.macro->loop+=macroOffX;
          } else {
            lastMacroDesc.macro->loop=255;
          }
          if ((lastMacroDesc.macro->rel+macroOffX)>=0 && (lastMacroDesc.macro->rel+macroOffX)<lastMacroDesc.macro->len) {
            lastMacroDesc.macro->rel+=macroOffX;
          } else {
            lastMacroDesc.macro->rel=255;
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
      if (ImGui::BeginMenu("randomize...")) {
        if (macroRandMin<lastMacroDesc.min) macroRandMin=lastMacroDesc.min;
        if (macroRandMin>lastMacroDesc.max) macroRandMin=lastMacroDesc.max;
        if (macroRandMax<lastMacroDesc.min) macroRandMax=lastMacroDesc.min;
        if (macroRandMax>lastMacroDesc.max) macroRandMax=lastMacroDesc.max;
        ImGui::InputInt("Min",&macroRandMin,1,10);
        ImGui::InputInt("Max",&macroRandMax,1,10);
        if (ImGui::Button("randomize")) {
          for (int i=0; i<lastMacroDesc.macro->len; i++) {
            int val=0;
            if (macroRandMax<=macroRandMin) {
              val=macroRandMin;
            } else {
              val=macroRandMin+(rand()%(macroRandMax-macroRandMin+1));
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
