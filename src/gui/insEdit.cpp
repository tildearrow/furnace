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

const char* fmParamNames[3][27]={
  {"Algorithm", "Feedback", "LFO > Freq", "LFO > Amp", "Attack", "Decay", "Decay 2", "Release", "Sustain", "Level", "EnvScale", "Multiplier", "Detune", "Detune 2", "SSG-EG", "AM", "AM Depth", "Vibrato Depth", "Sustained", "Sustained", "Level Scaling", "Sustain", "Vibrato", "Waveform", "Key Scale Rate", "OP2 Half Sine", "OP1 Half Sine"},
  {"ALG", "FB", "FMS/PMS", "AMS", "AR", "DR", "SR", "RR", "SL", "TL", "KS", "MULT", "DT", "DT2", "SSG-EG", "AM", "AMD", "FMD", "EGT", "EGT", "KSL", "SUS", "VIB", "WS", "KSR", "DC", "DM"},
  {"ALG", "FB", "FMS/PMS", "AMS", "AR", "DR", "D2R", "RR", "SL", "TL", "RS", "MULT", "DT", "DT2", "SSG-EG", "AM", "DAM", "DVB", "EGT", "EGS", "KSL", "SUS", "VIB", "WS", "KSR", "DC", "DM"}
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

const char* opzWaveforms[8]={
  "Sine", "Triangle", "Cut Sine", "Cut Triangle", "Squished Sine", "Squished Triangle", "Squished AbsSine", "Squished AbsTriangle"
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
  FM_DM=26
};

#define FM_NAME(x) fmParamNames[settings.fmNames][x]

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

const char* oneBit[2]={
  "on", NULL
};

const int orderedOps[4]={
  0, 2, 1, 3
};

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
  ImU32 color=ImGui::GetColorU32(uiColors[GUI_COLOR_TEXT]);
  ImU32 colorL=ImGui::GetColorU32(ImVec4(uiColors[GUI_COLOR_TEXT].x,uiColors[GUI_COLOR_TEXT].y,uiColors[GUI_COLOR_TEXT].z,uiColors[GUI_COLOR_TEXT].w*0.33));
  ImGui::ItemSize(size,style.FramePadding.y);
  if (ImGui::ItemAdd(rect,ImGui::GetID("alg"))) {
    ImGui::RenderFrame(rect.Min,rect.Max,ImGui::GetColorU32(ImGuiCol_FrameBg),true,style.FrameRounding);
    const float circleRadius=6.0f*dpiScale+1.0f;
    switch (algType) {
      case FM_ALGS_4OP:
        switch (alg) {
          case 0: { // 1 > 2 > 3 > 4
            ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.2,0.5));
            ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(0.4,0.5));
            ImVec2 pos3=ImLerp(rect.Min,rect.Max,ImVec2(0.6,0.5));
            ImVec2 pos4=ImLerp(rect.Min,rect.Max,ImVec2(0.8,0.5));
            dl->AddCircleFilled(pos1,4.0f*dpiScale+1.0f,color);
            dl->AddCircle(pos1,6.0f*dpiScale+1.0f,color);
            addAALine(dl,pos1,pos2,colorL);
            dl->AddCircleFilled(pos2,4.0f*dpiScale+1.0f,color);
            addAALine(dl,pos2,pos3,colorL);
            dl->AddCircleFilled(pos3,4.0f*dpiScale+1.0f,color);
            addAALine(dl,pos3,pos4,colorL);
            dl->AddCircleFilled(pos4,4.0f*dpiScale+1.0f,color);

            pos1.x-=ImGui::CalcTextSize("1").x*0.5;
            pos2.x-=ImGui::CalcTextSize("2").x*0.5;
            pos3.x-=ImGui::CalcTextSize("3").x*0.5;
            pos4.x-=ImGui::CalcTextSize("4").x*0.5;
            pos1.y-=ImGui::CalcTextSize("1").y+circleRadius;
            pos2.y-=ImGui::CalcTextSize("2").y+circleRadius;
            pos3.y-=ImGui::CalcTextSize("3").y+circleRadius;
            pos4.y-=ImGui::CalcTextSize("4").y+circleRadius;
            dl->AddText(pos1,color,"1");
            dl->AddText(pos2,color,"2");
            dl->AddText(pos3,color,"3");
            dl->AddText(pos4,color,"4");
            break;
          }
          case 1: { // (1+2) > 3 > 4
            ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.25,0.3));
            ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(0.25,0.7));
            ImVec2 pos3=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.5));
            ImVec2 pos4=ImLerp(rect.Min,rect.Max,ImVec2(0.75,0.5));
            dl->AddCircleFilled(pos1,4.0f*dpiScale+1.0f,color);
            dl->AddCircle(pos1,6.0f*dpiScale+1.0f,color);
            addAALine(dl,pos1,pos3,colorL);
            dl->AddCircleFilled(pos2,4.0f*dpiScale+1.0f,color);
            addAALine(dl,pos2,pos3,colorL);
            dl->AddCircleFilled(pos3,4.0f*dpiScale+1.0f,color);
            addAALine(dl,pos3,pos4,colorL);
            dl->AddCircleFilled(pos4,4.0f*dpiScale+1.0f,color);

            pos2.x-=ImGui::CalcTextSize("2").x+circleRadius+3.0*dpiScale;
            pos1.x=pos2.x;
            pos3.x-=ImGui::CalcTextSize("3").x*0.5;
            pos4.x-=ImGui::CalcTextSize("4").x*0.5;
            pos1.y-=ImGui::CalcTextSize("1").y*0.5;
            pos2.y-=ImGui::CalcTextSize("2").y*0.5;
            pos3.y-=ImGui::CalcTextSize("3").y+circleRadius;
            pos4.y-=ImGui::CalcTextSize("4").y+circleRadius;
            dl->AddText(pos1,color,"1");
            dl->AddText(pos2,color,"2");
            dl->AddText(pos3,color,"3");
            dl->AddText(pos4,color,"4");
            break;
          }
          case 2: { // 1+(2>3) > 4
            ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.3));
            ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(0.25,0.7));
            ImVec2 pos3=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.7));
            ImVec2 pos4=ImLerp(rect.Min,rect.Max,ImVec2(0.75,0.5));
            dl->AddCircleFilled(pos1,4.0f*dpiScale+1.0f,color);
            dl->AddCircle(pos1,6.0f*dpiScale+1.0f,color);
            addAALine(dl,pos1,pos4,colorL);
            dl->AddCircleFilled(pos2,4.0f*dpiScale+1.0f,color);
            addAALine(dl,pos2,pos3,colorL);
            dl->AddCircleFilled(pos3,4.0f*dpiScale+1.0f,color);
            addAALine(dl,pos3,pos4,colorL);
            dl->AddCircleFilled(pos4,4.0f*dpiScale+1.0f,color);

            pos1.x-=ImGui::CalcTextSize("2").x+circleRadius+3.0*dpiScale;
            pos2.x-=ImGui::CalcTextSize("2").x+circleRadius+3.0*dpiScale;
            pos3.x-=ImGui::CalcTextSize("3").x+circleRadius+3.0*dpiScale;
            pos4.x-=ImGui::CalcTextSize("4").x*0.5;
            pos1.y-=ImGui::CalcTextSize("1").y*0.5;
            pos2.y-=ImGui::CalcTextSize("2").y*0.5;
            pos3.y-=ImGui::CalcTextSize("3").y*0.5;
            pos4.y-=ImGui::CalcTextSize("4").y+circleRadius;
            dl->AddText(pos1,color,"1");
            dl->AddText(pos2,color,"2");
            dl->AddText(pos3,color,"3");
            dl->AddText(pos4,color,"4");
            break;
          }
          case 3: { // (1>2)+3 > 4
            ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.25,0.3));
            ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.3));
            ImVec2 pos3=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.7));
            ImVec2 pos4=ImLerp(rect.Min,rect.Max,ImVec2(0.75,0.5));
            dl->AddCircleFilled(pos1,4.0f*dpiScale+1.0f,color);
            dl->AddCircle(pos1,6.0f*dpiScale+1.0f,color);
            addAALine(dl,pos1,pos2,colorL);
            dl->AddCircleFilled(pos2,4.0f*dpiScale+1.0f,color);
            addAALine(dl,pos2,pos4,colorL);
            dl->AddCircleFilled(pos3,4.0f*dpiScale+1.0f,color);
            addAALine(dl,pos3,pos4,colorL);
            dl->AddCircleFilled(pos4,4.0f*dpiScale+1.0f,color);

            pos1.x-=ImGui::CalcTextSize("2").x+circleRadius+3.0*dpiScale;
            pos2.x-=ImGui::CalcTextSize("2").x+circleRadius+3.0*dpiScale;
            pos3.x-=ImGui::CalcTextSize("3").x+circleRadius+3.0*dpiScale;
            pos4.x-=ImGui::CalcTextSize("4").x*0.5;
            pos1.y-=ImGui::CalcTextSize("1").y*0.5;
            pos2.y-=ImGui::CalcTextSize("2").y*0.5;
            pos3.y-=ImGui::CalcTextSize("3").y*0.5;
            pos4.y-=ImGui::CalcTextSize("4").y+circleRadius;
            dl->AddText(pos1,color,"1");
            dl->AddText(pos2,color,"2");
            dl->AddText(pos3,color,"3");
            dl->AddText(pos4,color,"4");
            break;
          }
          case 4: { // (1>2) + (3>4)
            ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.25,0.3));
            ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.3));
            ImVec2 pos3=ImLerp(rect.Min,rect.Max,ImVec2(0.25,0.7));
            ImVec2 pos4=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.7));
            ImVec2 pos5=ImLerp(rect.Min,rect.Max,ImVec2(0.75,0.5));
            dl->AddCircleFilled(pos1,4.0f*dpiScale+1.0f,color);
            dl->AddCircle(pos1,6.0f*dpiScale+1.0f,color);
            addAALine(dl,pos1,pos2,colorL);
            dl->AddCircleFilled(pos2,4.0f*dpiScale+1.0f,color);
            dl->AddCircleFilled(pos3,4.0f*dpiScale+1.0f,color);
            addAALine(dl,pos3,pos4,colorL);
            dl->AddCircleFilled(pos4,4.0f*dpiScale+1.0f,color);
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
            dl->AddText(pos1,color,"1");
            dl->AddText(pos2,color,"2");
            dl->AddText(pos3,color,"3");
            dl->AddText(pos4,color,"4");
            break;
          }
          case 5: { // 1 > (2+3+4)
            ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.25,0.5));
            ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.25));
            ImVec2 pos3=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.5));
            ImVec2 pos4=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.75));
            ImVec2 pos5=ImLerp(rect.Min,rect.Max,ImVec2(0.75,0.5));
            dl->AddCircleFilled(pos1,4.0f*dpiScale+1.0f,color);
            dl->AddCircle(pos1,6.0f*dpiScale+1.0f,color);
            addAALine(dl,pos1,pos2,colorL);
            addAALine(dl,pos1,pos3,colorL);
            addAALine(dl,pos1,pos4,colorL);
            dl->AddCircleFilled(pos2,4.0f*dpiScale+1.0f,color);
            dl->AddCircleFilled(pos3,4.0f*dpiScale+1.0f,color);
            dl->AddCircleFilled(pos4,4.0f*dpiScale+1.0f,color);
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
            dl->AddText(pos1,color,"1");
            dl->AddText(pos2,color,"2");
            dl->AddText(pos3,color,"3");
            dl->AddText(pos4,color,"4");
            break;
          }
          case 6: { // (1>2) + 3 + 4
            ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.25,0.25));
            ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.25));
            ImVec2 pos3=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.5));
            ImVec2 pos4=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.75));
            ImVec2 pos5=ImLerp(rect.Min,rect.Max,ImVec2(0.75,0.5));
            dl->AddCircleFilled(pos1,4.0f*dpiScale+1.0f,color);
            dl->AddCircle(pos1,6.0f*dpiScale+1.0f,color);
            addAALine(dl,pos1,pos2,colorL);
            dl->AddCircleFilled(pos2,4.0f*dpiScale+1.0f,color);
            dl->AddCircleFilled(pos3,4.0f*dpiScale+1.0f,color);
            dl->AddCircleFilled(pos4,4.0f*dpiScale+1.0f,color);
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
            dl->AddText(pos1,color,"1");
            dl->AddText(pos2,color,"2");
            dl->AddText(pos3,color,"3");
            dl->AddText(pos4,color,"4");
            break;
          }
          case 7: { // 1 + 2 + 3 + 4
            ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.25,0.2));
            ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(0.35,0.4));
            ImVec2 pos3=ImLerp(rect.Min,rect.Max,ImVec2(0.45,0.6));
            ImVec2 pos4=ImLerp(rect.Min,rect.Max,ImVec2(0.55,0.8));
            ImVec2 pos5=ImLerp(rect.Min,rect.Max,ImVec2(0.75,0.5));
            dl->AddCircleFilled(pos1,4.0f*dpiScale+1.0f,color);
            dl->AddCircle(pos1,6.0f*dpiScale+1.0f,color);
            dl->AddCircleFilled(pos2,4.0f*dpiScale+1.0f,color);
            dl->AddCircleFilled(pos3,4.0f*dpiScale+1.0f,color);
            dl->AddCircleFilled(pos4,4.0f*dpiScale+1.0f,color);
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
            dl->AddText(pos1,color,"1");
            dl->AddText(pos2,color,"2");
            dl->AddText(pos3,color,"3");
            dl->AddText(pos4,color,"4");
            break;
          }
        }
        break;
      case FM_ALGS_2OP_OPL:
        switch (alg) {
          case 0: { // 1 > 2
            ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.33,0.5));
            ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(0.67,0.5));
            dl->AddCircleFilled(pos1,4.0f*dpiScale+1.0f,color);
            dl->AddCircle(pos1,6.0f*dpiScale+1.0f,color);
            addAALine(dl,pos1,pos2,colorL);
            dl->AddCircleFilled(pos2,4.0f*dpiScale+1.0f,color);

            pos1.x-=ImGui::CalcTextSize("2").x+circleRadius+3.0*dpiScale;
            pos2.x+=circleRadius+3.0*dpiScale;
            pos1.y-=ImGui::CalcTextSize("1").y*0.5;
            pos2.y-=ImGui::CalcTextSize("2").y*0.5;
            dl->AddText(pos1,color,"1");
            dl->AddText(pos2,color,"2");
            break;
          }
          case 1: { // 1 + 2
            ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.33,0.5));
            ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(0.67,0.5));
            dl->AddCircleFilled(pos1,4.0f*dpiScale+1.0f,color);
            dl->AddCircle(pos1,6.0f*dpiScale+1.0f,color);
            dl->AddCircleFilled(pos2,4.0f*dpiScale+1.0f,color);

            pos1.x-=ImGui::CalcTextSize("2").x+circleRadius+3.0*dpiScale;
            pos2.x+=circleRadius+3.0*dpiScale;
            pos1.y-=ImGui::CalcTextSize("1").y*0.5;
            pos2.y-=ImGui::CalcTextSize("2").y*0.5;
            dl->AddText(pos1,color,"1");
            dl->AddText(pos2,color,"2");
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
            dl->AddCircleFilled(pos1,4.0f*dpiScale+1.0f,color);
            dl->AddCircle(pos1,6.0f*dpiScale+1.0f,color);
            addAALine(dl,pos1,pos2,colorL);
            dl->AddCircleFilled(pos2,4.0f*dpiScale+1.0f,color);
            addAALine(dl,pos2,pos3,colorL);
            dl->AddCircleFilled(pos3,4.0f*dpiScale+1.0f,color);
            addAALine(dl,pos3,pos4,colorL);
            dl->AddCircleFilled(pos4,4.0f*dpiScale+1.0f,color);

            pos1.x-=ImGui::CalcTextSize("1").x*0.5;
            pos2.x-=ImGui::CalcTextSize("2").x*0.5;
            pos3.x-=ImGui::CalcTextSize("3").x*0.5;
            pos4.x-=ImGui::CalcTextSize("4").x*0.5;
            pos1.y-=ImGui::CalcTextSize("1").y+circleRadius;
            pos2.y-=ImGui::CalcTextSize("2").y+circleRadius;
            pos3.y-=ImGui::CalcTextSize("3").y+circleRadius;
            pos4.y-=ImGui::CalcTextSize("4").y+circleRadius;
            dl->AddText(pos1,color,"1");
            dl->AddText(pos2,color,"2");
            dl->AddText(pos3,color,"3");
            dl->AddText(pos4,color,"4");
            break;
          }
          case 1: { // 1 + (2 > 3 > 4)
            ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.4,0.3));
            ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(0.2,0.7));
            ImVec2 pos3=ImLerp(rect.Min,rect.Max,ImVec2(0.4,0.7));
            ImVec2 pos4=ImLerp(rect.Min,rect.Max,ImVec2(0.6,0.7));
            ImVec2 pos5=ImLerp(rect.Min,rect.Max,ImVec2(0.8,0.7));
            dl->AddCircleFilled(pos1,4.0f*dpiScale+1.0f,color);
            dl->AddCircle(pos1,6.0f*dpiScale+1.0f,color);
            addAALine(dl,pos1,pos5,colorL);
            dl->AddCircleFilled(pos2,4.0f*dpiScale+1.0f,color);
            addAALine(dl,pos2,pos3,colorL);
            dl->AddCircleFilled(pos3,4.0f*dpiScale+1.0f,color);
            addAALine(dl,pos3,pos4,colorL);
            dl->AddCircleFilled(pos4,4.0f*dpiScale+1.0f,color);

            addAALine(dl,pos4,pos5,colorL);

            pos1.x-=ImGui::CalcTextSize("2").x+circleRadius+3.0*dpiScale;
            pos2.x-=ImGui::CalcTextSize("2").x+circleRadius+3.0*dpiScale;
            pos3.x-=ImGui::CalcTextSize("3").x+circleRadius+3.0*dpiScale;
            pos4.x-=ImGui::CalcTextSize("4").x*0.5;
            pos1.y-=ImGui::CalcTextSize("1").y*0.5;
            pos2.y-=ImGui::CalcTextSize("2").y*0.5;
            pos3.y-=ImGui::CalcTextSize("3").y*0.5;
            pos4.y-=ImGui::CalcTextSize("4").y+circleRadius;
            dl->AddText(pos1,color,"1");
            dl->AddText(pos2,color,"2");
            dl->AddText(pos3,color,"3");
            dl->AddText(pos4,color,"4");
            break;
          }
          case 2: { // (1>2) + (3>4)
            ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.25,0.3));
            ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.3));
            ImVec2 pos3=ImLerp(rect.Min,rect.Max,ImVec2(0.25,0.7));
            ImVec2 pos4=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.7));
            ImVec2 pos5=ImLerp(rect.Min,rect.Max,ImVec2(0.75,0.5));
            dl->AddCircleFilled(pos1,4.0f*dpiScale+1.0f,color);
            dl->AddCircle(pos1,6.0f*dpiScale+1.0f,color);
            addAALine(dl,pos1,pos2,colorL);
            dl->AddCircleFilled(pos2,4.0f*dpiScale+1.0f,color);
            dl->AddCircleFilled(pos3,4.0f*dpiScale+1.0f,color);
            addAALine(dl,pos3,pos4,colorL);
            dl->AddCircleFilled(pos4,4.0f*dpiScale+1.0f,color);
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
            dl->AddText(pos1,color,"1");
            dl->AddText(pos2,color,"2");
            dl->AddText(pos3,color,"3");
            dl->AddText(pos4,color,"4");
            break;
          }
          case 3: { // 1 + (2 > 3) + 4
            ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.25));
            ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(0.25,0.5));
            ImVec2 pos3=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.5));
            ImVec2 pos4=ImLerp(rect.Min,rect.Max,ImVec2(0.5,0.75));
            ImVec2 pos5=ImLerp(rect.Min,rect.Max,ImVec2(0.75,0.5));
            dl->AddCircleFilled(pos1,4.0f*dpiScale+1.0f,color);
            dl->AddCircle(pos1,6.0f*dpiScale+1.0f,color);
            addAALine(dl,pos2,pos3,colorL);
            dl->AddCircleFilled(pos2,4.0f*dpiScale+1.0f,color);
            dl->AddCircleFilled(pos3,4.0f*dpiScale+1.0f,color);
            dl->AddCircleFilled(pos4,4.0f*dpiScale+1.0f,color);
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
            dl->AddText(pos1,color,"1");
            dl->AddText(pos2,color,"2");
            dl->AddText(pos3,color,"3");
            dl->AddText(pos4,color,"4");
            break;
          }
        }
        break;
      default:
        break;
    }
  }
}

void FurnaceGUI::drawFMEnv(unsigned char tl, unsigned char ar, unsigned char dr, unsigned char d2r, unsigned char rr, unsigned char sl, float maxTl, float maxArDr, const ImVec2& size) {
  ImDrawList* dl=ImGui::GetWindowDrawList();
  ImGuiWindow* window=ImGui::GetCurrentWindow();

  ImVec2 minArea=window->DC.CursorPos;
  ImVec2 maxArea=ImVec2(
    minArea.x+size.x,
    minArea.y+size.y
  );
  ImRect rect=ImRect(minArea,maxArea);
  ImGuiStyle& style=ImGui::GetStyle();
  ImU32 color=ImGui::GetColorU32(uiColors[GUI_COLOR_TEXT]);
  ImU32 colorS=ImGui::GetColorU32(uiColors[GUI_COLOR_SONG_LOOP]); //Relsease triangle and sustain horiz/vert line color
  ImGui::ItemSize(size,style.FramePadding.y);
  if (ImGui::ItemAdd(rect,ImGui::GetID("alg"))) {
    ImGui::RenderFrame(rect.Min,rect.Max,ImGui::GetColorU32(ImGuiCol_FrameBg),true,style.FrameRounding);

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
    ImVec2 posDecayRate0Pt=ImLerp(rect.Min,rect.Max,ImVec2(1.0,(tl/maxTl))); //Heght of the peak of AR, forever
    ImVec2 posDecay2Rate0Pt=ImLerp(rect.Min,rect.Max,ImVec2(1.0,(float)((tl/maxTl)+(sl/15.0)-((tl/maxTl)*(sl/15.0))))); //Heght of the peak of SR, forever

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
    } else if (d2r==0.0) { //if D2R = 0, the envelope stays at the sustain level forever
      dl->AddTriangleFilled(posRStart,posREnd,pos1,colorS); //draw release as shaded triangle behind everything
      addAALine(dl,pos3,posSLineHEnd,colorS); //draw horiz line through sustain level
      addAALine(dl,pos3,posSLineVEnd,colorS); //draw vert. line through sustain level
      addAALine(dl,pos1,pos2,color); //A
      addAALine(dl,pos2,pos3,color); //D
      addAALine(dl,pos3,posDecay2Rate0Pt,color); //Line from D to end of graph
    } else { //draw graph normally
      dl->AddTriangleFilled(posRStart,posREnd,pos1,colorS); //draw release as shaded triangle behind everything
      addAALine(dl,pos3,posSLineHEnd,colorS); //draw horiz line through sustain level
      addAALine(dl,pos3,posSLineVEnd,colorS); //draw vert. line through sustain level
      addAALine(dl,pos1,pos2,color); //A
      addAALine(dl,pos2,pos3,color); //D
      addAALine(dl,pos3,pos4,color); //D2
    }
    //dl->Flags^=ImDrawListFlags_AntiAliasedLines|ImDrawListFlags_AntiAliasedLinesUseTex;
  }
}

#define P(x) if (x) { \
  MARK_MODIFIED; \
  e->notifyInsChange(curIns); \
}

#define PARAMETER MARK_MODIFIED; e->notifyInsChange(curIns);

#define NORMAL_MACRO(macro,macroLen,macroLoop,macroRel,macroMin,macroHeight,macroName,displayName,displayHeight,displayLoop,bitfield,bfVal,drawSlider,sliderVal,sliderLow,macroDispMin,bitOff,macroMode,macroColor,mmlStr,macroAMin,macroAMax,hoverFunc,blockMode) \
  ImGui::TableNextRow(); \
  ImGui::TableNextColumn(); \
  ImGui::Text("%s",displayName); \
  ImGui::SameLine(); \
  if (ImGui::SmallButton(displayLoop?(ICON_FA_CHEVRON_UP "##IMacroOpen_" macroName):(ICON_FA_CHEVRON_DOWN "##IMacroOpen_" macroName))) { \
    displayLoop=!displayLoop; \
  } \
  if (displayLoop) { \
    ImGui::SetNextItemWidth(lenAvail); \
    if (ImGui::InputScalar("##IMacroLen_" macroName,ImGuiDataType_U8,&macroLen,&_ONE,&_THREE)) { MARK_MODIFIED \
      if (macroLen>127) macroLen=127; \
    } \
    if (macroMode!=NULL) { \
      ImGui::Checkbox("Fixed##IMacroMode_" macroName,macroMode); \
    } \
  } \
  ImGui::TableNextColumn(); \
  for (int j=0; j<256; j++) { \
    if (j+macroDragScroll>=macroLen) { \
      asFloat[j]=0; \
      asInt[j]=0; \
    } else { \
      asFloat[j]=macro[j+macroDragScroll]+macroDispMin; \
      asInt[j]=macro[j+macroDragScroll]+macroDispMin+bitOff; \
    } \
    if (j+macroDragScroll>=macroLen || (j+macroDragScroll>macroRel && macroLoop<macroRel)) { \
      loopIndicator[j]=0; \
    } else { \
      loopIndicator[j]=((macroLoop!=-1 && (j+macroDragScroll)>=macroLoop))|((macroRel!=-1 && (j+macroDragScroll)==macroRel)<<1); \
    } \
  } \
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(0.0f,0.0f)); \
  \
  if (bitfield) { \
    PlotBitfield("##IMacro_" macroName,asInt,totalFit,0,bfVal,macroHeight,ImVec2(availableWidth,(displayLoop)?(displayHeight*dpiScale):(32.0f*dpiScale))); \
  } else { \
    PlotCustom("##IMacro_" macroName,asFloat,totalFit,macroDragScroll,NULL,macroDispMin+macroMin,macroHeight+macroDispMin,ImVec2(availableWidth,(displayLoop)?(displayHeight*dpiScale):(32.0f*dpiScale)),sizeof(float),macroColor,macroLen-macroDragScroll,hoverFunc,blockMode); \
  } \
  if (displayLoop && ImGui::IsItemClicked(ImGuiMouseButton_Left)) { \
    macroDragStart=ImGui::GetItemRectMin(); \
    macroDragAreaSize=ImVec2(availableWidth,displayHeight*dpiScale); \
    macroDragMin=macroMin; \
    macroDragMax=macroHeight; \
    macroDragBitOff=bitOff; \
    macroDragBitMode=bitfield; \
    macroDragInitialValueSet=false; \
    macroDragInitialValue=false; \
    macroDragLen=totalFit; \
    macroDragActive=true; \
    macroDragTarget=macro; \
    macroDragChar=false; \
    processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y); \
  } \
  if (displayLoop) { \
    if (drawSlider) { \
      ImGui::SameLine(); \
      ImGui::VSliderInt("##IArpMacroPos",ImVec2(20.0f*dpiScale,displayHeight*dpiScale),sliderVal,sliderLow,70); \
    } \
    PlotCustom("##IMacroLoop_" macroName,loopIndicator,totalFit,macroDragScroll,NULL,0,2,ImVec2(availableWidth,12.0f*dpiScale),sizeof(float),macroColor,macroLen-macroDragScroll,&macroHoverLoop); \
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) { \
      macroLoopDragStart=ImGui::GetItemRectMin(); \
      macroLoopDragAreaSize=ImVec2(availableWidth,12.0f*dpiScale); \
      macroLoopDragLen=totalFit; \
      if (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift)) { \
        macroLoopDragTarget=&macroRel; \
      } else { \
        macroLoopDragTarget=&macroLoop; \
      } \
      macroLoopDragActive=true; \
      processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y); \
    } \
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) { \
      if (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift)) { \
        macroRel=-1; \
      } else { \
        macroLoop=-1; \
      } \
    } \
    ImGui::SetNextItemWidth(availableWidth); \
    if (ImGui::InputText("##IMacroMML_" macroName,&mmlStr)) { \
      decodeMMLStr(mmlStr,macro,macroLen,macroLoop,macroAMin,(bitfield)?((1<<macroAMax)-1):macroAMax,macroRel); \
    } \
    if (!ImGui::IsItemActive()) { \
      encodeMMLStr(mmlStr,macro,macroLen,macroLoop,macroRel); \
    } \
  } \
  ImGui::PopStyleVar();

#define OP_MACRO(macro,macroLen,macroLoop,macroRel,macroHeight,op,macroName,displayName,displayHeight,displayLoop,bitfield,bfVal,mmlStr) \
  ImGui::TableNextRow(); \
  ImGui::TableNextColumn(); \
  ImGui::Text("%s",displayName); \
  ImGui::SameLine(); \
  if (ImGui::SmallButton(displayLoop?(ICON_FA_CHEVRON_UP "##IOPMacroOpen_" macroName):(ICON_FA_CHEVRON_DOWN "##IOPMacroOpen_" macroName))) { \
    displayLoop=!displayLoop; \
  } \
  if (displayLoop) { \
    ImGui::SetNextItemWidth(lenAvail); \
    if (ImGui::InputScalar("##IOPMacroLen_" #op macroName,ImGuiDataType_U8,&macroLen,&_ONE,&_THREE)) { MARK_MODIFIED \
      if (macroLen>127) macroLen=127; \
    } \
  } \
  ImGui::TableNextColumn(); \
  for (int j=0; j<256; j++) { \
    if (j+macroDragScroll>=macroLen) { \
      asFloat[j]=0; \
      asInt[j]=0; \
    } else { \
      asFloat[j]=macro[j+macroDragScroll]; \
      asInt[j]=macro[j+macroDragScroll]; \
    } \
    if (j+macroDragScroll>=macroLen || (j+macroDragScroll>macroRel && macroLoop<macroRel)) { \
      loopIndicator[j]=0; \
    } else { \
      loopIndicator[j]=((macroLoop!=-1 && (j+macroDragScroll)>=macroLoop))|((macroRel!=-1 && (j+macroDragScroll)==macroRel)<<1); \
    } \
  } \
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(0.0f,0.0f)); \
  \
  if (bitfield) { \
    PlotBitfield("##IOPMacro_" #op macroName,asInt,totalFit,0,bfVal,macroHeight,ImVec2(availableWidth,displayLoop?(displayHeight*dpiScale):(24*dpiScale))); \
  } else { \
    PlotCustom("##IOPMacro_" #op macroName,asFloat,totalFit,macroDragScroll,NULL,0,macroHeight,ImVec2(availableWidth,displayLoop?(displayHeight*dpiScale):(24*dpiScale)),sizeof(float),uiColors[GUI_COLOR_MACRO_OTHER],macroLen-macroDragScroll); \
  } \
  if (displayLoop && ImGui::IsItemClicked(ImGuiMouseButton_Left)) { \
    macroDragStart=ImGui::GetItemRectMin(); \
    macroDragAreaSize=ImVec2(availableWidth,displayHeight*dpiScale); \
    macroDragMin=0; \
    macroDragMax=macroHeight; \
    macroDragBitOff=0; \
    macroDragBitMode=bitfield; \
    macroDragInitialValueSet=false; \
    macroDragInitialValue=false; \
    macroDragLen=totalFit; \
    macroDragActive=true; \
    macroDragCTarget=macro; \
    macroDragChar=true; \
    processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y); \
  } \
  if (displayLoop) { \
    PlotCustom("##IOPMacroLoop_" #op macroName,loopIndicator,totalFit,macroDragScroll,NULL,0,2,ImVec2(availableWidth,12.0f*dpiScale),sizeof(float),uiColors[GUI_COLOR_MACRO_OTHER],macroLen-macroDragScroll,&macroHoverLoop); \
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) { \
      macroLoopDragStart=ImGui::GetItemRectMin(); \
      macroLoopDragAreaSize=ImVec2(availableWidth,8.0f*dpiScale); \
      macroLoopDragLen=totalFit; \
      if (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift)) { \
        macroLoopDragTarget=&macroRel; \
      } else { \
        macroLoopDragTarget=&macroLoop; \
      } \
      macroLoopDragActive=true; \
      processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y); \
    } \
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) { \
      if (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift)) { \
        macroRel=-1; \
      } else { \
        macroLoop=-1; \
      } \
    } \
    ImGui::SetNextItemWidth(availableWidth); \
    if (ImGui::InputText("##IOPMacroMML_" macroName,&mmlStr)) { \
      decodeMMLStr(mmlStr,macro,macroLen,macroLoop,0,bitfield?((1<<macroHeight)-1):(macroHeight),macroRel); \
    } \
    if (!ImGui::IsItemActive()) { \
      encodeMMLStr(mmlStr,macro,macroLen,macroLoop,macroRel); \
    } \
  } \
  ImGui::PopStyleVar();

#define MACRO_BEGIN(reservedSpace) \
if (ImGui::BeginTable("MacroSpace",2)) { \
  ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed,0.0); \
  ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0); \
  ImGui::TableNextRow(); \
  ImGui::TableNextColumn(); \
  float lenAvail=ImGui::GetContentRegionAvail().x; \
  ImGui::Dummy(ImVec2(120.0f*dpiScale,dpiScale)); \
  ImGui::TableNextColumn(); \
  float availableWidth=ImGui::GetContentRegionAvail().x-reservedSpace; \
  int totalFit=MIN(127,availableWidth/(16*dpiScale)); \
  if (macroDragScroll>127-totalFit) { \
    macroDragScroll=127-totalFit; \
  } \
  ImGui::SetNextItemWidth(availableWidth); \
  if (ImGui::SliderInt("##MacroScroll",&macroDragScroll,0,127-totalFit,"")) { \
    if (macroDragScroll<0) macroDragScroll=0; \
    if (macroDragScroll>127-totalFit) macroDragScroll=127-totalFit; \
  }

#define MACRO_END \
  ImGui::TableNextRow(); \
  ImGui::TableNextColumn(); \
  ImGui::TableNextColumn(); \
  ImGui::SetNextItemWidth(availableWidth); \
  if (ImGui::SliderInt("##MacroScroll",&macroDragScroll,0,127-totalFit,"")) { \
    if (macroDragScroll<0) macroDragScroll=0; \
    if (macroDragScroll>127-totalFit) macroDragScroll=127-totalFit; \
  } \
  ImGui::EndTable(); \
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

void FurnaceGUI::drawInsEdit() {
  if (nextWindow==GUI_WINDOW_INS_EDIT) {
    insEditOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!insEditOpen) return;
  ImGui::SetNextWindowSizeConstraints(ImVec2(440.0f*dpiScale,400.0f*dpiScale),ImVec2(scrW*dpiScale,scrH*dpiScale));
  if (ImGui::Begin("Instrument Editor",&insEditOpen,settings.allowEditDocking?0:ImGuiWindowFlags_NoDocking)) {
    if (curIns<0 || curIns>=(int)e->song.ins.size()) {
      ImGui::Text("no instrument selected");
    } else {
      DivInstrument* ins=e->song.ins[curIns];
      if (ImGui::InputText("Name",&ins->name)) {
        MARK_MODIFIED;
      }
      if (ins->type<0 || ins->type>=DIV_INS_MAX) ins->type=DIV_INS_FM;
      int insType=ins->type;
      if (ImGui::Combo("Type",&insType,insTypes,DIV_INS_MAX,DIV_INS_MAX)) {
        ins->type=(DivInstrumentType)insType;
      }

      if (ImGui::BeginTabBar("insEditTab")) {
        if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPLL || ins->type==DIV_INS_OPZ) {
          char label[32];
          float asFloat[256];
          int asInt[256];
          float loopIndicator[256];
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
                case DIV_INS_OPZ:
                  ImGui::TableNextColumn();
                  P(ImGui::SliderScalar(FM_NAME(FM_FB),ImGuiDataType_U8,&ins->fm.fb,&_ZERO,&_SEVEN)); rightClickable
                  P(ImGui::SliderScalar(FM_NAME(FM_FMS),ImGuiDataType_U8,&ins->fm.fms,&_ZERO,&_SEVEN)); rightClickable
                  ImGui::TableNextColumn();
                  P(ImGui::SliderScalar(FM_NAME(FM_ALG),ImGuiDataType_U8,&ins->fm.alg,&_ZERO,&_SEVEN)); rightClickable
                  P(ImGui::SliderScalar(FM_NAME(FM_AMS),ImGuiDataType_U8,&ins->fm.ams,&_ZERO,&_THREE)); rightClickable
                  ImGui::TableNextColumn();
                  drawAlgorithm(ins->fm.alg,FM_ALGS_4OP,ImVec2(ImGui::GetContentRegionAvail().x,48.0*dpiScale));
                  break;
                case DIV_INS_OPL: {
                  bool fourOp=(ins->fm.ops==4);
                  bool drums=ins->fm.opllPreset==16;
                  int algMax=fourOp?3:1;
                  ImGui::TableNextColumn();
                  ins->fm.alg&=algMax;
                  P(ImGui::SliderScalar(FM_NAME(FM_FB),ImGuiDataType_U8,&ins->fm.fb,&_ZERO,&_SEVEN)); rightClickable
                  ImGui::BeginDisabled(ins->fm.opllPreset==16);
                  if (ImGui::Checkbox("4-op",&fourOp)) { PARAMETER
                    ins->fm.ops=fourOp?4:2;
                  }
                  ImGui::EndDisabled();
                  ImGui::TableNextColumn();
                  P(ImGui::SliderScalar(FM_NAME(FM_ALG),ImGuiDataType_U8,&ins->fm.alg,&_ZERO,&algMax)); rightClickable
                  if (ImGui::Checkbox("Drums",&drums)) { PARAMETER
                    ins->fm.opllPreset=drums?16:0;
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
                  P(ImGui::SliderScalar(FM_NAME(FM_FB),ImGuiDataType_U8,&ins->fm.fb,&_ZERO,&_SEVEN)); rightClickable
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

            if ((ins->type==DIV_INS_OPLL || ins->type==DIV_INS_OPL) && ins->fm.opllPreset==16) {
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
              P(ImGui::SliderScalar("Volume##TL",ImGuiDataType_U8,&ins->fm.op[1].tl,&_FIFTEEN,&_ZERO)); rightClickable
            }
            if (willDisplayOps) if (ImGui::BeginTable("FMOperators",2,ImGuiTableFlags_SizingStretchSame)) {
              for (int i=0; i<opCount; i++) {
                DivInstrumentFM::Operator& op=ins->fm.op[(opCount==4)?opOrder[i]:i];
                if ((i+1)&1) ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Separator();
                ImGui::PushID(fmt::sprintf("op%d",i).c_str());
                ImGui::Dummy(ImVec2(dpiScale,dpiScale));
                if (ins->type==DIV_INS_OPL && ins->fm.opllPreset==16) {
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

                ImGui::SameLine();

                int maxTl=127;
                if (ins->type==DIV_INS_OPLL) {
                  if (i==1) {
                    maxTl=15;
                  } else {
                    maxTl=63;
                  }
                }
                if (ins->type==DIV_INS_OPL) {
                  maxTl=63;
                }
                int maxArDr=(ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ)?31:15;

                bool ssgOn=op.ssgEnv&8;
                bool ksrOn=op.ksr;
                bool vibOn=op.vib;
                bool susOn=op.sus; // don't you make fun of this one
                unsigned char ssgEnv=op.ssgEnv&7;
                if (ins->type!=DIV_INS_OPL && ins->type!=DIV_INS_OPZ) {
                  if (ImGui::Checkbox((ins->type==DIV_INS_OPLL)?FM_NAME(FM_EGS):"SSG On",&ssgOn)) { PARAMETER
                    op.ssgEnv=(op.ssgEnv&7)|(ssgOn<<3);
                  }
                  if (ins->type==DIV_INS_FM) {
                    if (ImGui::IsItemHovered()) {
                      ImGui::SetTooltip("Only for Genesis and Neo Geo systems");
                    }
                  }
                }

                if (ins->type==DIV_INS_OPL) {
                  if (ImGui::Checkbox(FM_NAME(FM_SUS),&susOn)) { PARAMETER
                    op.sus=susOn;
                  }
                }

                //52.0 controls vert scaling; default 96
                drawFMEnv(op.tl&maxTl,op.ar&maxArDr,op.dr&maxArDr,(ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPLL)?((op.rr&15)*2):op.d2r&31,op.rr&15,op.sl&15,maxTl,maxArDr,ImVec2(ImGui::GetContentRegionAvail().x,52.0*dpiScale));
                //P(ImGui::SliderScalar(FM_NAME(FM_AR),ImGuiDataType_U8,&op.ar,&_ZERO,&_THIRTY_ONE)); rightClickable
                if (ImGui::BeginTable("opParams",2,ImGuiTableFlags_SizingStretchProp)) {
                  ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.0); \
                  ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed,0.0); \

                  ImGui::TableNextRow();
                  ImGui::TableNextColumn();
                  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                  op.ar&=maxArDr;
                  P(ImGui::SliderScalar("##AR",ImGuiDataType_U8,&op.ar,&maxArDr,&_ZERO)); rightClickable
                  ImGui::TableNextColumn();
                  ImGui::Text("%s",FM_NAME(FM_AR));

                  ImGui::TableNextRow();
                  ImGui::TableNextColumn();
                  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                  op.dr&=maxArDr;
                  P(ImGui::SliderScalar("##DR",ImGuiDataType_U8,&op.dr,&maxArDr,&_ZERO)); rightClickable
                  ImGui::TableNextColumn();
                  ImGui::Text("%s",FM_NAME(FM_DR));

                  ImGui::TableNextRow();
                  ImGui::TableNextColumn();
                  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                  P(ImGui::SliderScalar("##SL",ImGuiDataType_U8,&op.sl,&_FIFTEEN,&_ZERO)); rightClickable
                  ImGui::TableNextColumn();
                  ImGui::Text("%s",FM_NAME(FM_SL));

                  if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    P(ImGui::SliderScalar("##D2R",ImGuiDataType_U8,&op.d2r,&_THIRTY_ONE,&_ZERO)); rightClickable
                    ImGui::TableNextColumn();
                    ImGui::Text("%s",FM_NAME(FM_D2R));
                  }

                  ImGui::TableNextRow();
                  ImGui::TableNextColumn();
                  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                  P(ImGui::SliderScalar("##RR",ImGuiDataType_U8,&op.rr,&_FIFTEEN,&_ZERO)); rightClickable
                  ImGui::TableNextColumn();
                  ImGui::Text("%s",FM_NAME(FM_RR));

                  ImGui::TableNextRow();
                  ImGui::TableNextColumn();
                  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                  op.tl&=maxTl;
                  P(ImGui::SliderScalar("##TL",ImGuiDataType_U8,&op.tl,&maxTl,&_ZERO)); rightClickable
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
                    P(ImGui::SliderScalar("##RS",ImGuiDataType_U8,&op.rs,&_ZERO,&_THREE)); rightClickable
                    ImGui::TableNextColumn();
                    ImGui::Text("%s",FM_NAME(FM_RS));
                  } else {
                    P(ImGui::SliderScalar("##KSL",ImGuiDataType_U8,&op.ksl,&_ZERO,&_THREE)); rightClickable
                    ImGui::TableNextColumn();
                    ImGui::Text("%s",FM_NAME(FM_KSL));
                  }

                  ImGui::TableNextRow();
                  ImGui::TableNextColumn();
                  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                  P(ImGui::SliderScalar(FM_NAME(FM_MULT),ImGuiDataType_U8,&op.mult,&_ZERO,&_FIFTEEN)); rightClickable
                  ImGui::TableNextColumn();
                  ImGui::Text("%s",FM_NAME(FM_MULT));
                  
                  if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ) {
                    int detune=(op.dt&7)-3;
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    if (ImGui::SliderInt("##DT",&detune,-3,4)) { PARAMETER
                      op.dt=detune+3;
                    } rightClickable
                    ImGui::TableNextColumn();
                    ImGui::Text("%s",FM_NAME(FM_DT));

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    P(ImGui::SliderScalar("##DT2",ImGuiDataType_U8,&op.dt2,&_ZERO,&_THREE)); rightClickable
                    if (ImGui::IsItemHovered()) {
                      ImGui::SetTooltip("Only for Arcade system");
                    }
                    ImGui::TableNextColumn();
                    ImGui::Text("%s",FM_NAME(FM_DT2));

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    if (ImGui::SliderScalar("##SSG",ImGuiDataType_U8,&ssgEnv,&_ZERO,&_SEVEN,ssgEnvTypes[ssgEnv])) { PARAMETER
                      op.ssgEnv=(op.ssgEnv&8)|(ssgEnv&7);
                    } rightClickable
                    ImGui::TableNextColumn();
                    ImGui::Text("%s",FM_NAME(FM_SSG));
                  }

                  if (ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPZ) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    P(ImGui::SliderScalar("##WS",ImGuiDataType_U8,&op.ws,&_ZERO,&_SEVEN,(ins->type==DIV_INS_OPZ)?opzWaveforms[op.ws&7]:oplWaveforms[op.ws&7])); rightClickable
                    if (ins->type==DIV_INS_OPL && ImGui::IsItemHovered()) {
                      ImGui::SetTooltip("OPL2/3 only (last 4 waveforms are OPL3 only)");
                    }
                    ImGui::TableNextColumn();
                    ImGui::Text("%s",FM_NAME(FM_WS));
                  }
                  
                  ImGui::EndTable();
                }

                if (ins->type==DIV_INS_OPLL || ins->type==DIV_INS_OPL) {
                  if (ImGui::Checkbox(FM_NAME(FM_VIB),&vibOn)) { PARAMETER
                    op.vib=vibOn;
                  }
                  ImGui::SameLine();
                  if (ImGui::Checkbox(FM_NAME(FM_KSR),&ksrOn)) { PARAMETER
                    op.ksr=ksrOn;
                  }
                }

                ImGui::PopID();
              }
              ImGui::EndTable();
            }
            ImGui::EndTabItem();
          }
          if (ImGui::BeginTabItem("FM Macros")) {
            MACRO_BEGIN(0);
            if (ins->type==DIV_INS_OPLL) {
              NORMAL_MACRO(ins->std.algMacro,ins->std.algMacroLen,ins->std.algMacroLoop,ins->std.algMacroRel,0,1,"alg",FM_NAME(FM_SUS),32,ins->std.algMacroOpen,true,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[0],0,1,NULL,false);
              NORMAL_MACRO(ins->std.fbMacro,ins->std.fbMacroLen,ins->std.fbMacroLoop,ins->std.fbMacroRel,0,7,"fb",FM_NAME(FM_FB),96,ins->std.fbMacroOpen,false,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[1],0,7,NULL,false);
              NORMAL_MACRO(ins->std.fmsMacro,ins->std.fmsMacroLen,ins->std.fmsMacroLoop,ins->std.fmsMacroRel,0,1,"fms",FM_NAME(FM_DC),32,ins->std.fmsMacroOpen,true,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[2],0,1,NULL,false);
              NORMAL_MACRO(ins->std.amsMacro,ins->std.amsMacroLen,ins->std.amsMacroLoop,ins->std.amsMacroRel,0,1,"ams",FM_NAME(FM_DM),32,ins->std.amsMacroOpen,true,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[3],0,1,NULL,false);
            } else {
              NORMAL_MACRO(ins->std.algMacro,ins->std.algMacroLen,ins->std.algMacroLoop,ins->std.algMacroRel,0,7,"alg",FM_NAME(FM_ALG),96,ins->std.algMacroOpen,false,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[0],0,7,NULL,false);
              NORMAL_MACRO(ins->std.fbMacro,ins->std.fbMacroLen,ins->std.fbMacroLoop,ins->std.fbMacroRel,0,7,"fb",FM_NAME(FM_FB),96,ins->std.fbMacroOpen,false,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[1],0,7,NULL,false);
              if (ins->type!=DIV_INS_OPL) {
                NORMAL_MACRO(ins->std.fmsMacro,ins->std.fmsMacroLen,ins->std.fmsMacroLoop,ins->std.fmsMacroRel,0,7,"fms",FM_NAME(FM_FMS),96,ins->std.fmsMacroOpen,false,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[2],0,7,NULL,false);
                NORMAL_MACRO(ins->std.amsMacro,ins->std.amsMacroLen,ins->std.amsMacroLoop,ins->std.amsMacroRel,0,3,"ams",FM_NAME(FM_AMS),48,ins->std.amsMacroOpen,false,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[3],0,3,NULL,false);
              }
            }
            
            if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ) {
              NORMAL_MACRO(ins->std.ex1Macro,ins->std.ex1MacroLen,ins->std.ex1MacroLoop,ins->std.ex1MacroRel,0,127,"ex1","AM Depth",128,ins->std.ex1MacroOpen,false,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[4],0,127,NULL,false);
              NORMAL_MACRO(ins->std.ex2Macro,ins->std.ex2MacroLen,ins->std.ex2MacroLoop,ins->std.ex2MacroRel,0,127,"ex2","PM Depth",128,ins->std.ex2MacroOpen,false,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[5],0,127,NULL,false);
              NORMAL_MACRO(ins->std.ex3Macro,ins->std.ex3MacroLen,ins->std.ex3MacroLoop,ins->std.ex3MacroRel,0,255,"ex3","LFO Speed",128,ins->std.ex3MacroOpen,false,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[6],0,255,NULL,false);
              NORMAL_MACRO(ins->std.waveMacro,ins->std.waveMacroLen,ins->std.waveMacroLoop,ins->std.waveMacroRel,0,3,"wave","LFO Shape",48,ins->std.waveMacroOpen,false,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_WAVE],mmlString[7],0,3,&macroLFOWaves,false);
            }
            MACRO_END;
            ImGui::EndTabItem();
          }
          for (int i=0; i<opCount; i++) {
            snprintf(label,31,"OP%d Macros",i+1);
            if (ImGui::BeginTabItem(label)) {
              ImGui::PushID(i);
              MACRO_BEGIN(0);
              int ordi=(opCount==4)?orderedOps[i]:i;
              int maxTl=127;
              if (ins->type==DIV_INS_OPLL) {
                if (i==1) {
                  maxTl=15;
                } else {
                  maxTl=63;
                }
              }
              if (ins->type==DIV_INS_OPL) {
                maxTl=63;
              }
              int maxArDr=(ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ)?31:15;

              if (ins->type==DIV_INS_OPL) {
                OP_MACRO(ins->std.opMacros[ordi].tlMacro,ins->std.opMacros[ordi].tlMacroLen,ins->std.opMacros[ordi].tlMacroLoop,ins->std.opMacros[ordi].tlMacroRel,maxTl,ordi,"tl",FM_NAME(FM_TL),128,ins->std.opMacros[ordi].tlMacroOpen,false,NULL,mmlString[0]);
                OP_MACRO(ins->std.opMacros[ordi].arMacro,ins->std.opMacros[ordi].arMacroLen,ins->std.opMacros[ordi].arMacroLoop,ins->std.opMacros[ordi].arMacroRel,maxArDr,ordi,"ar",FM_NAME(FM_AR),64,ins->std.opMacros[ordi].arMacroOpen,false,NULL,mmlString[1]);
                OP_MACRO(ins->std.opMacros[ordi].drMacro,ins->std.opMacros[ordi].drMacroLen,ins->std.opMacros[ordi].drMacroLoop,ins->std.opMacros[ordi].drMacroRel,maxArDr,ordi,"dr",FM_NAME(FM_DR),64,ins->std.opMacros[ordi].drMacroOpen,false,NULL,mmlString[2]);
                OP_MACRO(ins->std.opMacros[ordi].slMacro,ins->std.opMacros[ordi].slMacroLen,ins->std.opMacros[ordi].slMacroLoop,ins->std.opMacros[ordi].slMacroRel,15,ordi,"sl",FM_NAME(FM_SL),64,ins->std.opMacros[ordi].slMacroOpen,false,NULL,mmlString[5]);
                OP_MACRO(ins->std.opMacros[ordi].rrMacro,ins->std.opMacros[ordi].rrMacroLen,ins->std.opMacros[ordi].rrMacroLoop,ins->std.opMacros[ordi].rrMacroRel,15,ordi,"rr",FM_NAME(FM_RR),64,ins->std.opMacros[ordi].rrMacroOpen,false,NULL,mmlString[4]);
                OP_MACRO(ins->std.opMacros[ordi].kslMacro,ins->std.opMacros[ordi].kslMacroLen,ins->std.opMacros[ordi].kslMacroLoop,ins->std.opMacros[ordi].kslMacroRel,3,ordi,"ksl",FM_NAME(FM_KSL),32,ins->std.opMacros[ordi].kslMacroOpen,false,NULL,mmlString[6]);
                OP_MACRO(ins->std.opMacros[ordi].multMacro,ins->std.opMacros[ordi].multMacroLen,ins->std.opMacros[ordi].multMacroLoop,ins->std.opMacros[ordi].multMacroRel,15,ordi,"mult",FM_NAME(FM_MULT),64,ins->std.opMacros[ordi].multMacroOpen,false,NULL,mmlString[7]);
                OP_MACRO(ins->std.opMacros[ordi].wsMacro,ins->std.opMacros[ordi].wsMacroLen,ins->std.opMacros[ordi].wsMacroLoop,ins->std.opMacros[ordi].wsMacroRel,7,ordi,"ws",FM_NAME(FM_WS),64,ins->std.opMacros[ordi].wsMacroOpen,false,NULL,mmlString[8]);

                OP_MACRO(ins->std.opMacros[ordi].amMacro,ins->std.opMacros[ordi].amMacroLen,ins->std.opMacros[ordi].amMacroLoop,ins->std.opMacros[ordi].amMacroRel,1,ordi,"am",FM_NAME(FM_AM),32,ins->std.opMacros[ordi].amMacroOpen,true,NULL,mmlString[9]);
                OP_MACRO(ins->std.opMacros[ordi].vibMacro,ins->std.opMacros[ordi].vibMacroLen,ins->std.opMacros[ordi].vibMacroLoop,ins->std.opMacros[ordi].vibMacroRel,1,ordi,"vib",FM_NAME(FM_VIB),32,ins->std.opMacros[ordi].vibMacroOpen,true,NULL,mmlString[10]);
                OP_MACRO(ins->std.opMacros[ordi].ksrMacro,ins->std.opMacros[ordi].ksrMacroLen,ins->std.opMacros[ordi].ksrMacroLoop,ins->std.opMacros[ordi].ksrMacroRel,1,ordi,"ksr",FM_NAME(FM_KSR),32,ins->std.opMacros[ordi].ksrMacroOpen,true,NULL,mmlString[11]);
                OP_MACRO(ins->std.opMacros[ordi].susMacro,ins->std.opMacros[ordi].susMacroLen,ins->std.opMacros[ordi].susMacroLoop,ins->std.opMacros[ordi].susMacroRel,1,ordi,"sus",FM_NAME(FM_SUS),32,ins->std.opMacros[ordi].susMacroOpen,true,NULL,mmlString[12]);
              } else if (ins->type==DIV_INS_OPLL) {
                OP_MACRO(ins->std.opMacros[ordi].tlMacro,ins->std.opMacros[ordi].tlMacroLen,ins->std.opMacros[ordi].tlMacroLoop,ins->std.opMacros[ordi].tlMacroRel,maxTl,ordi,"tl",FM_NAME(FM_TL),128,ins->std.opMacros[ordi].tlMacroOpen,false,NULL,mmlString[0]);
                OP_MACRO(ins->std.opMacros[ordi].arMacro,ins->std.opMacros[ordi].arMacroLen,ins->std.opMacros[ordi].arMacroLoop,ins->std.opMacros[ordi].arMacroRel,maxArDr,ordi,"ar",FM_NAME(FM_AR),64,ins->std.opMacros[ordi].arMacroOpen,false,NULL,mmlString[1]);
                OP_MACRO(ins->std.opMacros[ordi].drMacro,ins->std.opMacros[ordi].drMacroLen,ins->std.opMacros[ordi].drMacroLoop,ins->std.opMacros[ordi].drMacroRel,maxArDr,ordi,"dr",FM_NAME(FM_DR),64,ins->std.opMacros[ordi].drMacroOpen,false,NULL,mmlString[2]);
                OP_MACRO(ins->std.opMacros[ordi].slMacro,ins->std.opMacros[ordi].slMacroLen,ins->std.opMacros[ordi].slMacroLoop,ins->std.opMacros[ordi].slMacroRel,15,ordi,"sl",FM_NAME(FM_SL),64,ins->std.opMacros[ordi].slMacroOpen,false,NULL,mmlString[5]);
                OP_MACRO(ins->std.opMacros[ordi].rrMacro,ins->std.opMacros[ordi].rrMacroLen,ins->std.opMacros[ordi].rrMacroLoop,ins->std.opMacros[ordi].rrMacroRel,15,ordi,"rr",FM_NAME(FM_RR),64,ins->std.opMacros[ordi].rrMacroOpen,false,NULL,mmlString[4]);
                OP_MACRO(ins->std.opMacros[ordi].kslMacro,ins->std.opMacros[ordi].kslMacroLen,ins->std.opMacros[ordi].kslMacroLoop,ins->std.opMacros[ordi].kslMacroRel,3,ordi,"ksl",FM_NAME(FM_KSL),32,ins->std.opMacros[ordi].kslMacroOpen,false,NULL,mmlString[6]);
                OP_MACRO(ins->std.opMacros[ordi].multMacro,ins->std.opMacros[ordi].multMacroLen,ins->std.opMacros[ordi].multMacroLoop,ins->std.opMacros[ordi].multMacroRel,15,ordi,"mult",FM_NAME(FM_MULT),64,ins->std.opMacros[ordi].multMacroOpen,false,NULL,mmlString[7]);
                
                OP_MACRO(ins->std.opMacros[ordi].amMacro,ins->std.opMacros[ordi].amMacroLen,ins->std.opMacros[ordi].amMacroLoop,ins->std.opMacros[ordi].amMacroRel,1,ordi,"am",FM_NAME(FM_AM),32,ins->std.opMacros[ordi].amMacroOpen,true,NULL,mmlString[8]);
                OP_MACRO(ins->std.opMacros[ordi].vibMacro,ins->std.opMacros[ordi].vibMacroLen,ins->std.opMacros[ordi].vibMacroLoop,ins->std.opMacros[ordi].vibMacroRel,1,ordi,"vib",FM_NAME(FM_VIB),32,ins->std.opMacros[ordi].vibMacroOpen,true,NULL,mmlString[9]);
                OP_MACRO(ins->std.opMacros[ordi].ksrMacro,ins->std.opMacros[ordi].ksrMacroLen,ins->std.opMacros[ordi].ksrMacroLoop,ins->std.opMacros[ordi].ksrMacroRel,1,ordi,"ksr",FM_NAME(FM_KSR),32,ins->std.opMacros[ordi].ksrMacroOpen,true,NULL,mmlString[10]);
                OP_MACRO(ins->std.opMacros[ordi].egtMacro,ins->std.opMacros[ordi].egtMacroLen,ins->std.opMacros[ordi].egtMacroLoop,ins->std.opMacros[ordi].egtMacroRel,1,ordi,"egt",FM_NAME(FM_EGS),32,ins->std.opMacros[ordi].egtMacroOpen,true,NULL,mmlString[11]);
              } else {
                OP_MACRO(ins->std.opMacros[ordi].tlMacro,ins->std.opMacros[ordi].tlMacroLen,ins->std.opMacros[ordi].tlMacroLoop,ins->std.opMacros[ordi].tlMacroRel,maxTl,ordi,"tl",FM_NAME(FM_TL),128,ins->std.opMacros[ordi].tlMacroOpen,false,NULL,mmlString[0]);
                OP_MACRO(ins->std.opMacros[ordi].arMacro,ins->std.opMacros[ordi].arMacroLen,ins->std.opMacros[ordi].arMacroLoop,ins->std.opMacros[ordi].arMacroRel,maxArDr,ordi,"ar",FM_NAME(FM_AR),64,ins->std.opMacros[ordi].arMacroOpen,false,NULL,mmlString[1]);
                OP_MACRO(ins->std.opMacros[ordi].drMacro,ins->std.opMacros[ordi].drMacroLen,ins->std.opMacros[ordi].drMacroLoop,ins->std.opMacros[ordi].drMacroRel,maxArDr,ordi,"dr",FM_NAME(FM_DR),64,ins->std.opMacros[ordi].drMacroOpen,false,NULL,mmlString[2]);
                OP_MACRO(ins->std.opMacros[ordi].d2rMacro,ins->std.opMacros[ordi].d2rMacroLen,ins->std.opMacros[ordi].d2rMacroLoop,ins->std.opMacros[ordi].d2rMacroRel,31,ordi,"d2r",FM_NAME(FM_D2R),64,ins->std.opMacros[ordi].d2rMacroOpen,false,NULL,mmlString[3]);
                OP_MACRO(ins->std.opMacros[ordi].rrMacro,ins->std.opMacros[ordi].rrMacroLen,ins->std.opMacros[ordi].rrMacroLoop,ins->std.opMacros[ordi].rrMacroRel,15,ordi,"rr",FM_NAME(FM_RR),64,ins->std.opMacros[ordi].rrMacroOpen,false,NULL,mmlString[4]);
                OP_MACRO(ins->std.opMacros[ordi].slMacro,ins->std.opMacros[ordi].slMacroLen,ins->std.opMacros[ordi].slMacroLoop,ins->std.opMacros[ordi].slMacroRel,15,ordi,"sl",FM_NAME(FM_SL),64,ins->std.opMacros[ordi].slMacroOpen,false,NULL,mmlString[5]);
                OP_MACRO(ins->std.opMacros[ordi].rsMacro,ins->std.opMacros[ordi].rsMacroLen,ins->std.opMacros[ordi].rsMacroLoop,ins->std.opMacros[ordi].rsMacroRel,3,ordi,"rs",FM_NAME(FM_RS),32,ins->std.opMacros[ordi].rsMacroOpen,false,NULL,mmlString[6]);
                OP_MACRO(ins->std.opMacros[ordi].multMacro,ins->std.opMacros[ordi].multMacroLen,ins->std.opMacros[ordi].multMacroLoop,ins->std.opMacros[ordi].multMacroRel,15,ordi,"mult",FM_NAME(FM_MULT),64,ins->std.opMacros[ordi].multMacroOpen,false,NULL,mmlString[7]);
                OP_MACRO(ins->std.opMacros[ordi].dtMacro,ins->std.opMacros[ordi].dtMacroLen,ins->std.opMacros[ordi].dtMacroLoop,ins->std.opMacros[ordi].dtMacroRel,7,ordi,"dt",FM_NAME(FM_DT),64,ins->std.opMacros[ordi].dtMacroOpen,false,NULL,mmlString[8]);
                OP_MACRO(ins->std.opMacros[ordi].dt2Macro,ins->std.opMacros[ordi].dt2MacroLen,ins->std.opMacros[ordi].dt2MacroLoop,ins->std.opMacros[ordi].dt2MacroRel,3,ordi,"dt2",FM_NAME(FM_DT2),32,ins->std.opMacros[ordi].dt2MacroOpen,false,NULL,mmlString[9]);
                OP_MACRO(ins->std.opMacros[ordi].amMacro,ins->std.opMacros[ordi].amMacroLen,ins->std.opMacros[ordi].amMacroLoop,ins->std.opMacros[ordi].amMacroRel,1,ordi,"am",FM_NAME(FM_AM),32,ins->std.opMacros[ordi].amMacroOpen,true,NULL,mmlString[10]);
                OP_MACRO(ins->std.opMacros[ordi].ssgMacro,ins->std.opMacros[ordi].ssgMacroLen,ins->std.opMacros[ordi].ssgMacroLoop,ins->std.opMacros[ordi].ssgMacroRel,4,ordi,"ssg",FM_NAME(FM_SSG),64,ins->std.opMacros[ordi].ssgMacroOpen,true,ssgEnvBits,mmlString[11]);
              }
              MACRO_END;
              ImGui::PopID();
              ImGui::EndTabItem();
            }
          }
        }
        if (ins->type==DIV_INS_GB) if (ImGui::BeginTabItem("Game Boy")) {
          P(ImGui::SliderScalar("Volume",ImGuiDataType_U8,&ins->gb.envVol,&_ZERO,&_FIFTEEN)); rightClickable
          P(ImGui::SliderScalar("Envelope Length",ImGuiDataType_U8,&ins->gb.envLen,&_ZERO,&_SEVEN)); rightClickable
          P(ImGui::SliderScalar("Sound Length",ImGuiDataType_U8,&ins->gb.soundLen,&_ZERO,&_SIXTY_FOUR,ins->gb.soundLen>63?"Infinity":"%d")); rightClickable
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

          P(ImGui::SliderScalar("Attack",ImGuiDataType_U8,&ins->c64.a,&_ZERO,&_FIFTEEN)); rightClickable
          P(ImGui::SliderScalar("Decay",ImGuiDataType_U8,&ins->c64.d,&_ZERO,&_FIFTEEN)); rightClickable
          P(ImGui::SliderScalar("Sustain",ImGuiDataType_U8,&ins->c64.s,&_ZERO,&_FIFTEEN)); rightClickable
          P(ImGui::SliderScalar("Release",ImGuiDataType_U8,&ins->c64.r,&_ZERO,&_FIFTEEN)); rightClickable
          P(ImGui::SliderScalar("Duty",ImGuiDataType_U16,&ins->c64.duty,&_ZERO,&_FOUR_THOUSAND_NINETY_FIVE)); rightClickable

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
          
          P(ImGui::SliderScalar("Cutoff",ImGuiDataType_U16,&ins->c64.cut,&_ZERO,&_TWO_THOUSAND_FORTY_SEVEN)); rightClickable
          P(ImGui::SliderScalar("Resonance",ImGuiDataType_U8,&ins->c64.res,&_ZERO,&_FIFTEEN)); rightClickable

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
          ImGui::EndTabItem();
        }
        if (ins->type==DIV_INS_AMIGA) if (ImGui::BeginTabItem("Amiga/Sample")) {
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
          P(ImGui::Checkbox("Use sample map (does not work yet!)",&ins->amiga.useNoteMap));
          if (ins->amiga.useNoteMap) {
            if (ImGui::BeginTable("NoteMap",3,ImGuiTableFlags_ScrollY|ImGuiTableFlags_Borders|ImGuiTableFlags_SizingStretchSame)) {
              ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
              ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch);
              ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch);

              ImGui::TableSetupScrollFreeze(0,1);

              ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
              ImGui::TableNextColumn();
              ImGui::TableNextColumn();
              ImGui::Text("Sample");
              ImGui::TableNextColumn();
              ImGui::Text("Frequency");
              for (int i=0; i<120; i++) {
                ImGui::TableNextRow();
                ImGui::PushID(fmt::sprintf("NM_%d",i).c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%s",noteNames[60+i]);
                ImGui::TableNextColumn();
                if (ins->amiga.noteMap[i]<0 || ins->amiga.noteMap[i]>=e->song.sampleLen) {
                  sName="-- empty --";
                  ins->amiga.noteMap[i]=-1;
                } else {
                  sName=e->song.sample[ins->amiga.noteMap[i]]->name;
                }
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::BeginCombo("##SM",sName.c_str())) {
                  String id;
                  if (ImGui::Selectable("-- empty --",ins->amiga.noteMap[i]==-1)) { PARAMETER
                    ins->amiga.noteMap[i]=-1;
                  }
                  for (int j=0; j<e->song.sampleLen; j++) {
                    id=fmt::sprintf("%d: %s",j,e->song.sample[j]->name);
                    if (ImGui::Selectable(id.c_str(),ins->amiga.noteMap[i]==j)) { PARAMETER
                      ins->amiga.noteMap[i]=j;
                      if (ins->amiga.noteFreq[i]<=0) ins->amiga.noteFreq[i]=(int)((double)e->song.sample[j]->centerRate*pow(2.0,((double)i-48.0)/12.0));
                    }
                  }
                  ImGui::EndCombo();
                }
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::InputInt("##SF",&ins->amiga.noteFreq[i],50,500)) { PARAMETER
                  if (ins->amiga.noteFreq[i]<0) ins->amiga.noteFreq[i]=0;
                  if (ins->amiga.noteFreq[i]>262144) ins->amiga.noteFreq[i]=262144;
                }
                ImGui::PopID();
              }
              ImGui::EndTable();
            }
          }
          ImGui::EndTabItem();
        }
        if (ins->type==DIV_INS_N163) if (ImGui::BeginTabItem("N163")) {
          ImGui::Text("Initial waveform");
          if (ImGui::InputInt("##WAVE",&ins->n163.wave,1,10)) { PARAMETER
            if (ins->n163.wave<0) ins->n163.wave=0;
            if (ins->n163.wave>=e->song.waveLen) ins->n163.wave=e->song.waveLen-1;
          }
          ImGui::Text("Initial waveform position in RAM");
          if (ImGui::InputInt("##WAVEPOS",&ins->n163.wavePos,1,16)) { PARAMETER
            if (ins->n163.wavePos<0) ins->n163.wavePos=0;
            if (ins->n163.wavePos>255) ins->n163.wavePos=255;
          }
          ImGui::Text("Initial waveform length in RAM");
          if (ImGui::InputInt("##WAVELEN",&ins->n163.waveLen,4,16)) { PARAMETER
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
        if (ImGui::BeginTabItem("Macros")) {
          float asFloat[256];
          int asInt[256];
          float loopIndicator[256];
          const char* volumeLabel="Volume";

          int volMax=15;
          int volMin=0;
          if (ins->type==DIV_INS_C64) {
            if (ins->c64.volIsCutoff) {
              volumeLabel="Cutoff";
              if (ins->c64.filterIsAbs) {
                volMax=2047;
              } else {
                volMin=-18;
                volMax=18;
              }
            }
          }
          if ((ins->type==DIV_INS_PCE || ins->type==DIV_INS_AY8930)) {
            volMax=31;
          }
          if (ins->type==DIV_INS_OPL || ins->type==DIV_INS_VERA) {
            volMax=63;
          }
          if (ins->type==DIV_INS_AMIGA) {
            volMax=64;
          }
          if (ins->type==DIV_INS_FM || ins->type == DIV_INS_MIKEY) {
            volMax=127;
          }
          if (ins->type==DIV_INS_GB) {
            volMax=0;
          }
          if (ins->type==DIV_INS_PET) {
            volMax=1;
          }

          bool arpMode=ins->std.arpMacroMode;

          const char* dutyLabel="Duty/Noise";
          int dutyMax=3;
          if (ins->type==DIV_INS_C64) {
            dutyLabel="Duty";
            if (ins->c64.dutyIsAbs) {
              dutyMax=4095;
            } else {
              dutyMax=24;
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
          if (ins->type == DIV_INS_MIKEY) {
            dutyLabel = "Duty/Int";
            dutyMax = 10;
          }
          if (ins->type==DIV_INS_AY8930) {
            dutyMax=255;
          }
          if (ins->type==DIV_INS_TIA || ins->type==DIV_INS_AMIGA || ins->type==DIV_INS_SCC || ins->type==DIV_INS_PET || ins->type==DIV_INS_VIC) {
            dutyMax=0;
          }
          if (ins->type==DIV_INS_PCE) {
            dutyMax=1;
          }
          if (ins->type==DIV_INS_SWAN) {
            dutyLabel="Noise";
            dutyMax=8;
          }
          if (ins->type==DIV_INS_OPLL || ins->type==DIV_INS_OPL) {
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
          bool dutyIsRel=(ins->type==DIV_INS_C64 && !ins->c64.dutyIsAbs);

          int waveMax=(ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930 || ins->type==DIV_INS_VERA)?3:63;
          bool bitMode=false;
          if (ins->type==DIV_INS_C64 || ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930 || ins->type==DIV_INS_SAA1099) {
            bitMode=true;
          }
          if (ins->type==DIV_INS_STD) waveMax=0;
          if (ins->type==DIV_INS_TIA || ins->type==DIV_INS_VIC) waveMax=15;
          if (ins->type==DIV_INS_C64) waveMax=4;
          if (ins->type==DIV_INS_SAA1099) waveMax=2;
          if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPLL || ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPZ) waveMax=0;
          if (ins->type==DIV_INS_MIKEY) waveMax=0;
          if (ins->type==DIV_INS_PET) {
            waveMax=8;
            bitMode=true;
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
            ex2Max=63;
            ex2Bit=false;
          }
          if (ins->type==DIV_INS_N163) {
            ex1Max=252;
            ex2Max=2;
          }
          if (ins->type==DIV_INS_SAA1099) ex1Max=8;

          if (settings.macroView==0) { // modern view
            MACRO_BEGIN(28*dpiScale);
            if (volMax>0) {
              NORMAL_MACRO(ins->std.volMacro,ins->std.volMacroLen,ins->std.volMacroLoop,ins->std.volMacroRel,volMin,volMax,"vol",volumeLabel,160,ins->std.volMacroOpen,false,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_VOLUME],mmlString[0],volMin,volMax,NULL,false);
            }
            NORMAL_MACRO(ins->std.arpMacro,ins->std.arpMacroLen,ins->std.arpMacroLoop,ins->std.arpMacroRel,arpMacroScroll,arpMacroScroll+24,"arp","Arpeggio",160,ins->std.arpMacroOpen,false,NULL,true,&arpMacroScroll,(arpMode?-60:-80),0,0,&ins->std.arpMacroMode,uiColors[GUI_COLOR_MACRO_PITCH],mmlString[1],-92,94,(ins->std.arpMacroMode?(&macroHoverNote):NULL),true);
            if (dutyMax>0) {
              if (ins->type==DIV_INS_MIKEY) {
                NORMAL_MACRO(ins->std.dutyMacro,ins->std.dutyMacroLen,ins->std.dutyMacroLoop,ins->std.dutyMacroRel,0,dutyMax,"duty",dutyLabel,160,ins->std.dutyMacroOpen,true,mikeyFeedbackBits,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[2],0,dutyMax,NULL,false);
              }
              else {
                NORMAL_MACRO(ins->std.dutyMacro,ins->std.dutyMacroLen,ins->std.dutyMacroLoop,ins->std.dutyMacroRel,0,dutyMax,"duty",dutyLabel,160,ins->std.dutyMacroOpen,false,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[2],0,dutyMax,NULL,false);
              }            
            }
            if (waveMax>0) {
              NORMAL_MACRO(ins->std.waveMacro,ins->std.waveMacroLen,ins->std.waveMacroLoop,ins->std.waveMacroRel,0,waveMax,"wave","Waveform",bitMode?64:160,ins->std.waveMacroOpen,bitMode,waveNames,false,NULL,0,0,((ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930)?1:0),NULL,uiColors[GUI_COLOR_MACRO_WAVE],mmlString[3],0,waveMax,NULL,false);
            }
            if (ex1Max>0) {
              if (ins->type==DIV_INS_C64) {
                NORMAL_MACRO(ins->std.ex1Macro,ins->std.ex1MacroLen,ins->std.ex1MacroLoop,ins->std.ex1MacroRel,0,ex1Max,"ex1","Filter Mode",64,ins->std.ex1MacroOpen,true,filtModeBits,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[4],0,ex1Max,NULL,false);
              } else if (ins->type==DIV_INS_SAA1099) {
                NORMAL_MACRO(ins->std.ex1Macro,ins->std.ex1MacroLen,ins->std.ex1MacroLoop,ins->std.ex1MacroRel,0,ex1Max,"ex1","Envelope",160,ins->std.ex1MacroOpen,true,saaEnvBits,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[4],0,ex1Max,NULL,false);
              } else if (ins->type==DIV_INS_X1_010) {
                NORMAL_MACRO(ins->std.ex1Macro,ins->std.ex1MacroLen,ins->std.ex1MacroLoop,ins->std.ex1MacroRel,0,ex1Max,"ex1","Envelope Mode",160,ins->std.ex1MacroOpen,true,x1_010EnvBits,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[4],0,ex1Max,NULL,false);
              } else if (ins->type==DIV_INS_N163) {
                NORMAL_MACRO(ins->std.ex1Macro,ins->std.ex1MacroLen,ins->std.ex1MacroLoop,ins->std.ex1MacroRel,0,ex1Max,"ex1","Waveform len.",160,ins->std.ex1MacroOpen,false,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[4],0,ex1Max,NULL,false);
              } else {
                NORMAL_MACRO(ins->std.ex1Macro,ins->std.ex1MacroLen,ins->std.ex1MacroLoop,ins->std.ex1MacroRel,0,ex1Max,"ex1","Duty",160,ins->std.ex1MacroOpen,false,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[4],0,ex1Max,NULL,false);
              }
            }
            if (ex2Max>0) {
              if (ins->type==DIV_INS_C64) {
                NORMAL_MACRO(ins->std.ex2Macro,ins->std.ex2MacroLen,ins->std.ex2MacroLoop,ins->std.ex2MacroRel,0,ex2Max,"ex2","Resonance",64,ins->std.ex2MacroOpen,false,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[5],0,ex2Max,NULL,false);
              } else if (ins->type==DIV_INS_N163) {
                NORMAL_MACRO(ins->std.ex2Macro,ins->std.ex2MacroLen,ins->std.ex2MacroLoop,ins->std.ex2MacroRel,0,ex2Max,"ex2","Waveform update",64,ins->std.ex2MacroOpen,true,n163UpdateBits,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[5],0,ex2Max,NULL,false);
              } else {
                NORMAL_MACRO(ins->std.ex2Macro,ins->std.ex2MacroLen,ins->std.ex2MacroLoop,ins->std.ex2MacroRel,0,ex2Max,"ex2","Envelope",ex2Bit?64:160,ins->std.ex2MacroOpen,ex2Bit,ayEnvBits,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[5],0,ex2Max,NULL,false);
              }
            }
            if (ins->type==DIV_INS_C64) {
              NORMAL_MACRO(ins->std.ex3Macro,ins->std.ex3MacroLen,ins->std.ex3MacroLoop,ins->std.ex3MacroRel,0,2,"ex3","Special",32,ins->std.ex3MacroOpen,true,c64SpecialBits,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[6],0,2,NULL,false);
            }
            if (ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930 || ins->type==DIV_INS_X1_010) {
              NORMAL_MACRO(ins->std.ex3Macro,ins->std.ex3MacroLen,ins->std.ex3MacroLoop,ins->std.ex3MacroRel,0,15,"ex3","AutoEnv Num",96,ins->std.ex3MacroOpen,false,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[6],0,15,NULL,false);
              NORMAL_MACRO(ins->std.algMacro,ins->std.algMacroLen,ins->std.algMacroLoop,ins->std.algMacroRel,0,15,"alg","AutoEnv Den",96,ins->std.algMacroOpen,false,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[7],0,15,NULL,false);
            }
            if (ins->type==DIV_INS_AY8930) {
              // oh my i am running out of macros
              NORMAL_MACRO(ins->std.fbMacro,ins->std.fbMacroLen,ins->std.fbMacroLoop,ins->std.fbMacroRel,0,8,"fb","Noise AND Mask",96,ins->std.fbMacroOpen,true,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[8],0,8,NULL,false);
              NORMAL_MACRO(ins->std.fmsMacro,ins->std.fmsMacroLen,ins->std.fmsMacroLoop,ins->std.fmsMacroRel,0,8,"fms","Noise OR Mask",96,ins->std.fmsMacroOpen,true,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[9],0,8,NULL,false);
            }
            if (ins->type==DIV_INS_N163) {
              NORMAL_MACRO(ins->std.ex3Macro,ins->std.ex3MacroLen,ins->std.ex3MacroLoop,ins->std.ex3MacroRel,0,255,"ex3","Waveform to Load",160,ins->std.ex3MacroOpen,false,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[6],0,255,NULL,false);
              NORMAL_MACRO(ins->std.algMacro,ins->std.algMacroLen,ins->std.algMacroLoop,ins->std.algMacroRel,0,255,"alg","Wave pos. to Load",160,ins->std.algMacroOpen,false,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[7],0,255,NULL,false);
              NORMAL_MACRO(ins->std.fbMacro,ins->std.fbMacroLen,ins->std.fbMacroLoop,ins->std.fbMacroRel,0,252,"fb","Wave len. to Load",160,ins->std.fbMacroOpen,false,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[8],0,252,NULL,false);
              NORMAL_MACRO(ins->std.fmsMacro,ins->std.fmsMacroLen,ins->std.fmsMacroLoop,ins->std.fmsMacroRel,0,2,"fms","Waveform load",64,ins->std.fmsMacroOpen,true,n163UpdateBits,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[9],0,2,NULL,false);
            }

            MACRO_END;
          } else { // classic view
            // volume macro
            ImGui::Separator();
            if (ins->type==DIV_INS_C64 && ins->c64.volIsCutoff) {
              if (ins->c64.filterIsAbs) {
                ImGui::Text("Cutoff Macro");
              } else {
                ImGui::Text("Relative Cutoff Macro");
              }
            } else {
              ImGui::Text("Volume Macro");
            }
            for (int i=0; i<ins->std.volMacroLen; i++) {
              if (ins->type==DIV_INS_C64 && ins->c64.volIsCutoff && !ins->c64.filterIsAbs) {
                asFloat[i]=ins->std.volMacro[i]-18;
              } else {
                asFloat[i]=ins->std.volMacro[i];
              }
              loopIndicator[i]=(ins->std.volMacroLoop!=-1 && i>=ins->std.volMacroLoop);
            }
            macroDragScroll=0;
            if (volMax>0) {
              ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(0.0f,0.0f));
              ImGui::PlotHistogram("##IVolMacro",asFloat,ins->std.volMacroLen,0,NULL,volMin,volMax,ImVec2(400.0f*dpiScale,200.0f*dpiScale));
              if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                macroDragStart=ImGui::GetItemRectMin();
                macroDragAreaSize=ImVec2(400.0f*dpiScale,200.0f*dpiScale);
                macroDragMin=volMin;
                macroDragMax=volMax;
                macroDragLen=ins->std.volMacroLen;
                macroDragActive=true;
                macroDragTarget=ins->std.volMacro;
                macroDragChar=false;
                processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
              }
              ImGui::PlotHistogram("##IVolMacroLoop",loopIndicator,ins->std.volMacroLen,0,NULL,0,1,ImVec2(400.0f*dpiScale,16.0f*dpiScale));
              if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                macroLoopDragStart=ImGui::GetItemRectMin();
                macroLoopDragAreaSize=ImVec2(400.0f*dpiScale,16.0f*dpiScale);
                macroLoopDragLen=ins->std.volMacroLen;
                macroLoopDragTarget=&ins->std.volMacroLoop;
                macroLoopDragActive=true;
                processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
              }
              if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                ins->std.volMacroLoop=-1;
              }
              ImGui::PopStyleVar();
              if (ImGui::InputScalar("Length##IVolMacroL",ImGuiDataType_U8,&ins->std.volMacroLen,&_ONE,&_THREE)) {
                if (ins->std.volMacroLen>127) ins->std.volMacroLen=127;
              }
            }

            // arp macro
            ImGui::Separator();
            ImGui::Text("Arpeggio Macro");
            for (int i=0; i<ins->std.arpMacroLen; i++) {
              asFloat[i]=ins->std.arpMacro[i];
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
              macroDragChar=false;
              processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
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
              processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
            }
            if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
              ins->std.arpMacroLoop=-1;
            }
            ImGui::PopStyleVar();
            if (ImGui::InputScalar("Length##IArpMacroL",ImGuiDataType_U8,&ins->std.arpMacroLen,&_ONE,&_THREE)) {
              if (ins->std.arpMacroLen>127) ins->std.arpMacroLen=127;
            }
            if (ImGui::Checkbox("Fixed",&arpMode)) {
              ins->std.arpMacroMode=arpMode;
              if (arpMode) {
                if (arpMacroScroll<0) arpMacroScroll=0;
              }
            }

            // duty macro
            if (dutyMax>0) {
              ImGui::Separator();
              if (ins->type==DIV_INS_C64) {
                if (ins->c64.dutyIsAbs) {
                  ImGui::Text("Duty Macro");
                } else {
                  ImGui::Text("Relative Duty Macro");
                }
              } else {
                if (ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930 || ins->type==DIV_INS_SAA1099) {
                  ImGui::Text("Noise Frequency Macro");
                } else {
                  ImGui::Text("Duty/Noise Mode Macro");
                }
              }
              for (int i=0; i<ins->std.dutyMacroLen; i++) {
                asFloat[i]=ins->std.dutyMacro[i]-(dutyIsRel?12:0);
                loopIndicator[i]=(ins->std.dutyMacroLoop!=-1 && i>=ins->std.dutyMacroLoop);
              }
              ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(0.0f,0.0f));
              
              ImGui::PlotHistogram("##IDutyMacro",asFloat,ins->std.dutyMacroLen,0,NULL,dutyIsRel?-12:0,dutyMax-(dutyIsRel?12:0),ImVec2(400.0f*dpiScale,200.0f*dpiScale));
              if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                macroDragStart=ImGui::GetItemRectMin();
                macroDragAreaSize=ImVec2(400.0f*dpiScale,200.0f*dpiScale);
                macroDragMin=0;
                macroDragMax=dutyMax;
                macroDragLen=ins->std.dutyMacroLen;
                macroDragActive=true;
                macroDragTarget=ins->std.dutyMacro;
                macroDragChar=false;
                processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
              }
              ImGui::PlotHistogram("##IDutyMacroLoop",loopIndicator,ins->std.dutyMacroLen,0,NULL,0,1,ImVec2(400.0f*dpiScale,16.0f*dpiScale));
              if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                macroLoopDragStart=ImGui::GetItemRectMin();
                macroLoopDragAreaSize=ImVec2(400.0f*dpiScale,16.0f*dpiScale);
                macroLoopDragLen=ins->std.dutyMacroLen;
                macroLoopDragTarget=&ins->std.dutyMacroLoop;
                macroLoopDragActive=true;
                processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
              }
              if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                ins->std.dutyMacroLoop=-1;
              }
              ImGui::PopStyleVar();
              if (ImGui::InputScalar("Length##IDutyMacroL",ImGuiDataType_U8,&ins->std.dutyMacroLen,&_ONE,&_THREE)) {
                if (ins->std.dutyMacroLen>127) ins->std.dutyMacroLen=127;
              }
            }

            // wave macro
            if (waveMax>0) {
              ImGui::Separator();
              ImGui::Text("Waveform Macro");
              for (int i=0; i<ins->std.waveMacroLen; i++) {
                asFloat[i]=ins->std.waveMacro[i];
                if (ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930) {
                  asInt[i]=ins->std.waveMacro[i]+1;
                } else {
                  asInt[i]=ins->std.waveMacro[i];
                }
                loopIndicator[i]=(ins->std.waveMacroLoop!=-1 && i>=ins->std.waveMacroLoop);
              }
              ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(0.0f,0.0f));
              
              ImVec2 areaSize=ImVec2(400.0f*dpiScale,200.0f*dpiScale);
              if (ins->type==DIV_INS_C64 || ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930 || ins->type==DIV_INS_SAA1099) {
                areaSize=ImVec2(400.0f*dpiScale,waveMax*32.0f*dpiScale);
                PlotBitfield("##IWaveMacro",asInt,ins->std.waveMacroLen,0,(ins->type==DIV_INS_C64)?c64ShapeBits:ayShapeBits,waveMax,areaSize);
                bitMode=true;
              } else {
                ImGui::PlotHistogram("##IWaveMacro",asFloat,ins->std.waveMacroLen,0,NULL,0,waveMax,areaSize);
              }
              if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                macroDragStart=ImGui::GetItemRectMin();
                macroDragAreaSize=areaSize;
                macroDragMin=0;
                macroDragMax=waveMax;
                macroDragBitOff=(ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930)?1:0;
                macroDragBitMode=bitMode;
                macroDragInitialValueSet=false;
                macroDragInitialValue=false;
                macroDragLen=ins->std.waveMacroLen;
                macroDragActive=true;
                macroDragTarget=ins->std.waveMacro;
                macroDragChar=false;
                processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
              }
              ImGui::PlotHistogram("##IWaveMacroLoop",loopIndicator,ins->std.waveMacroLen,0,NULL,0,1,ImVec2(400.0f*dpiScale,16.0f*dpiScale));
              if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                macroLoopDragStart=ImGui::GetItemRectMin();
                macroLoopDragAreaSize=ImVec2(400.0f*dpiScale,16.0f*dpiScale);
                macroLoopDragLen=ins->std.waveMacroLen;
                macroLoopDragTarget=&ins->std.waveMacroLoop;
                macroLoopDragActive=true;
                processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
              }
              if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                ins->std.waveMacroLoop=-1;
              }
              ImGui::PopStyleVar();
              if (ImGui::InputScalar("Length##IWaveMacroL",ImGuiDataType_U8,&ins->std.waveMacroLen,&_ONE,&_THREE)) {
                if (ins->std.waveMacroLen>127) ins->std.waveMacroLen=127;
              }
            }

            // extra 1 macro
            if (ex1Max>0) {
              ImGui::Separator();
              if (ins->type==DIV_INS_AY8930) {
                ImGui::Text("Duty Macro");
              } else {
                ImGui::Text("Extra 1 Macro");
              }
              for (int i=0; i<ins->std.ex1MacroLen; i++) {
                asFloat[i]=ins->std.ex1Macro[i];
                loopIndicator[i]=(ins->std.ex1MacroLoop!=-1 && i>=ins->std.ex1MacroLoop);
              }
              ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(0.0f,0.0f));
              
              ImGui::PlotHistogram("##IEx1Macro",asFloat,ins->std.ex1MacroLen,0,NULL,0,ex1Max,ImVec2(400.0f*dpiScale,200.0f*dpiScale));
              if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                macroDragStart=ImGui::GetItemRectMin();
                macroDragAreaSize=ImVec2(400.0f*dpiScale,200.0f*dpiScale);
                macroDragMin=0;
                macroDragMax=ex1Max;
                macroDragLen=ins->std.ex1MacroLen;
                macroDragActive=true;
                macroDragTarget=ins->std.ex1Macro;
                macroDragChar=false;
                processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
              }
              ImGui::PlotHistogram("##IEx1MacroLoop",loopIndicator,ins->std.ex1MacroLen,0,NULL,0,1,ImVec2(400.0f*dpiScale,16.0f*dpiScale));
              if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                macroLoopDragStart=ImGui::GetItemRectMin();
                macroLoopDragAreaSize=ImVec2(400.0f*dpiScale,16.0f*dpiScale);
                macroLoopDragLen=ins->std.ex1MacroLen;
                macroLoopDragTarget=&ins->std.ex1MacroLoop;
                macroLoopDragActive=true;
                processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
              }
              if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                ins->std.ex1MacroLoop=-1;
              }
              ImGui::PopStyleVar();
              if (ImGui::InputScalar("Length##IEx1MacroL",ImGuiDataType_U8,&ins->std.ex1MacroLen,&_ONE,&_THREE)) {
                if (ins->std.ex1MacroLen>127) ins->std.ex1MacroLen=127;
              }
            }
          }
          ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
      }
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_INS_EDIT;
  ImGui::End();
}
