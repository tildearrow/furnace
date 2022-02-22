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
#include "plot_nolerp.h"

const char* insTypes[23]={
  "Standard",
  "FM (4-operator)",
  "Game Boy",
  "C64",
  "Amiga/Sample",
  "PC Engine",
  "AY-3-8910/SSG",
  "AY8930",
  "TIA",
  "SAA1099",
  "VIC",
  "PET",
  "VRC6",
  "FM (OPLL)",
  "FM (OPL)",
  "FDS",
  "Virtual Boy",
  "Namco 163",
  "Konami SCC",
  "FM (OPZ)",
  "POKEY",
  "PC Beeper",
  "WonderSwan"
};

const char* ssgEnvTypes[8]={
  "Down Down Down", "Down.", "Down Up Down Up", "Down UP", "Up Up Up", "Up.", "Up Down Up Down", "Up DOWN"
};

const char* fmParamNames[3][16]={
  {"Algorithm", "Feedback", "LFO > Freq", "LFO > Amp", "Attack", "Decay", "Decay 2", "Release", "Sustain", "Level", "EnvScale", "Multiplier", "Detune", "Detune 2", "SSG-EG", "AM"},
  {"ALG", "FB", "FMS/PMS", "AMS", "AR", "DR", "SR", "RR", "SL", "TL", "KS", "MULT", "DT", "DT2", "SSG-EG", "AM"},
  {"ALG", "FB", "FMS/PMS", "AMS", "AR", "DR", "D2R", "RR", "SL", "TL", "RS", "MULT", "DT", "DT2", "SSG-EG", "AM"}
};

const char* opllInsNames[18]={
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
  "Drums (compatibility only!)"
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
  FM_AM=15
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
    //ImReallyTiredOfThisGarbage();
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
            dl->AddLine(pos1,pos2,colorL);
            dl->AddCircleFilled(pos2,4.0f*dpiScale+1.0f,color);
            dl->AddLine(pos2,pos3,colorL);
            dl->AddCircleFilled(pos3,4.0f*dpiScale+1.0f,color);
            dl->AddLine(pos3,pos4,colorL);
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
            dl->AddLine(pos1,pos3,colorL);
            dl->AddCircleFilled(pos2,4.0f*dpiScale+1.0f,color);
            dl->AddLine(pos2,pos3,colorL);
            dl->AddCircleFilled(pos3,4.0f*dpiScale+1.0f,color);
            dl->AddLine(pos3,pos4,colorL);
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
            dl->AddLine(pos1,pos4,colorL);
            dl->AddCircleFilled(pos2,4.0f*dpiScale+1.0f,color);
            dl->AddLine(pos2,pos3,colorL);
            dl->AddCircleFilled(pos3,4.0f*dpiScale+1.0f,color);
            dl->AddLine(pos3,pos4,colorL);
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
            dl->AddLine(pos1,pos2,colorL);
            dl->AddCircleFilled(pos2,4.0f*dpiScale+1.0f,color);
            dl->AddLine(pos2,pos4,colorL);
            dl->AddCircleFilled(pos3,4.0f*dpiScale+1.0f,color);
            dl->AddLine(pos3,pos4,colorL);
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
            dl->AddLine(pos1,pos2,colorL);
            dl->AddCircleFilled(pos2,4.0f*dpiScale+1.0f,color);
            dl->AddCircleFilled(pos3,4.0f*dpiScale+1.0f,color);
            dl->AddLine(pos3,pos4,colorL);
            dl->AddCircleFilled(pos4,4.0f*dpiScale+1.0f,color);
            dl->AddLine(pos2,pos5,colorL);
            dl->AddLine(pos4,pos5,colorL);

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
            dl->AddLine(pos1,pos2,colorL);
            dl->AddLine(pos1,pos3,colorL);
            dl->AddLine(pos1,pos4,colorL);
            dl->AddCircleFilled(pos2,4.0f*dpiScale+1.0f,color);
            dl->AddCircleFilled(pos3,4.0f*dpiScale+1.0f,color);
            dl->AddCircleFilled(pos4,4.0f*dpiScale+1.0f,color);
            dl->AddLine(pos2,pos5,colorL);
            dl->AddLine(pos3,pos5,colorL);
            dl->AddLine(pos4,pos5,colorL);

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
            dl->AddLine(pos1,pos2,colorL);
            dl->AddCircleFilled(pos2,4.0f*dpiScale+1.0f,color);
            dl->AddCircleFilled(pos3,4.0f*dpiScale+1.0f,color);
            dl->AddCircleFilled(pos4,4.0f*dpiScale+1.0f,color);
            dl->AddLine(pos2,pos5,colorL);
            dl->AddLine(pos3,pos5,colorL);
            dl->AddLine(pos4,pos5,colorL);

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
            dl->AddLine(pos1,pos5,colorL);
            dl->AddLine(pos2,pos5,colorL);
            dl->AddLine(pos3,pos5,colorL);
            dl->AddLine(pos4,pos5,colorL);

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

void FurnaceGUI::drawFMEnv(unsigned char tl, unsigned char ar, unsigned char dr, unsigned char d2r, unsigned char rr, unsigned char sl, const ImVec2& size) {
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
    float arPos=float(31-ar)/31.0; //peak of AR, start of DR
    float drPos=arPos+((sl/15.0)*(float(31-dr)/31.0)); //end of DR, start of D2R
    float d2rPos=drPos+(((15.0-sl)/15.0)*(float(31.0-d2r)/31.0)); //End of D2R
    float rrPos=(float(15-rr)/15.0); //end of RR

    //shrink all the x positions horizontally
    arPos/=2.0;
    drPos/=2.0;
    d2rPos/=2.0;
    rrPos/=1.0;

    ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.0,1.0)); //the bottom corner
    ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2(arPos,(tl/127.0))); //peak of AR, start of DR
    ImVec2 pos3=ImLerp(rect.Min,rect.Max,ImVec2(drPos,(float)((tl/127.0)+(sl/15.0)-((tl/127.0)*(sl/15.0))))); //end of DR, start of D2R
    ImVec2 pos4=ImLerp(rect.Min,rect.Max,ImVec2(d2rPos,1.0)); //end of D2R
    ImVec2 posRStart=ImLerp(rect.Min,rect.Max,ImVec2(0.0,(tl/127.0))); //release start
    ImVec2 posREnd=ImLerp(rect.Min,rect.Max,ImVec2(rrPos,1.0));//release end
    ImVec2 posSLineHEnd=ImLerp(rect.Min,rect.Max,ImVec2(1.0,(float)((tl/127.0)+(sl/15.0)-((tl/127.0)*(sl/15.0))))); //sustain horizontal line end
    ImVec2 posSLineVEnd=ImLerp(rect.Min,rect.Max,ImVec2(drPos,1.0)); //sustain vertical line end
    ImVec2 posDecayRate0Pt=ImLerp(rect.Min,rect.Max,ImVec2(1.0,(tl/127.0))); //Heght of the peak of AR, forever
    ImVec2 posDecay2Rate0Pt=ImLerp(rect.Min,rect.Max,ImVec2(1.0,(float)((tl/127.0)+(sl/15.0)-((tl/127.0)*(sl/15.0))))); //Heght of the peak of SR, forever

    if (ar==0.0) { //if AR = 0, the envelope never starts
      dl->AddTriangleFilled(posRStart,posREnd,pos1,colorS); //draw release as shaded triangle behind everything
      dl->AddLine(pos1,pos4,color); //draw line on ground
    }
    else if (dr==0.0 && sl!=0.0) { //if DR = 0 and SL is not 0, then the envelope stays at max volume forever
      dl->AddTriangleFilled(posRStart,posREnd,pos1,colorS); //draw release as shaded triangle behind everything
      //dl->AddLine(pos3,posSLineHEnd,colorS); //draw horiz line through sustain level
      //dl->AddLine(pos3,posSLineVEnd,colorS); //draw vert. line through sustain level
      dl->AddLine(pos1,pos2,color); //A
      dl->AddLine(pos2,posDecayRate0Pt,color); //Line from A to end of graph
    }
    else if(d2r==0.0) { //if D2R = 0, the envelope stays at the sustain level forever
      dl->AddTriangleFilled(posRStart,posREnd,pos1,colorS); //draw release as shaded triangle behind everything
      dl->AddLine(pos3,posSLineHEnd,colorS); //draw horiz line through sustain level
      dl->AddLine(pos3,posSLineVEnd,colorS); //draw vert. line through sustain level
      dl->AddLine(pos1,pos2,color); //A
      dl->AddLine(pos2,pos3,color); //D
      dl->AddLine(pos3,posDecay2Rate0Pt,color); //Line from D to end of graph
    }
    else { //draw graph normally
      dl->AddTriangleFilled(posRStart,posREnd,pos1,colorS); //draw release as shaded triangle behind everything
      dl->AddLine(pos3,posSLineHEnd,colorS); //draw horiz line through sustain level
      dl->AddLine(pos3,posSLineVEnd,colorS); //draw vert. line through sustain level
      dl->AddLine(pos1,pos2,color); //A
      dl->AddLine(pos2,pos3,color); //D
      dl->AddLine(pos3,pos4,color); //D2
    }
  }
}

#define P(x) if (x) { \
  modified=true; \
  e->notifyInsChange(curIns); \
}

#define PARAMETER modified=true; e->notifyInsChange(curIns);

#define NORMAL_MACRO(macro,macroLen,macroLoop,macroRel,macroMin,macroHeight,macroName,displayName,displayHeight,displayLoop,bitfield,bfVal,drawSlider,sliderVal,sliderLow,macroDispMin,bitOff,macroMode,macroColor,mmlStr,macroAMin,macroAMax,hoverFunc) \
  ImGui::TableNextRow(); \
  ImGui::TableNextColumn(); \
  ImGui::Text("%s",displayName); \
  ImGui::SameLine(); \
  if (ImGui::SmallButton(displayLoop?(ICON_FA_CHEVRON_UP "##IMacroOpen_" macroName):(ICON_FA_CHEVRON_DOWN "##IMacroOpen_" macroName))) { \
    displayLoop=!displayLoop; \
  } \
  if (displayLoop) { \
    ImGui::SetNextItemWidth(lenAvail); \
    if (ImGui::InputScalar("##IMacroLen_" macroName,ImGuiDataType_U8,&macroLen,&_ONE,&_THREE)) { \
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
    PlotCustom("##IMacro_" macroName,asFloat,totalFit,macroDragScroll,NULL,macroDispMin+macroMin,macroHeight+macroDispMin,ImVec2(availableWidth,(displayLoop)?(displayHeight*dpiScale):(32.0f*dpiScale)),sizeof(float),macroColor,macroLen-macroDragScroll,hoverFunc); \
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
    if (ImGui::InputScalar("##IOPMacroLen_" #op macroName,ImGuiDataType_U8,&macroLen,&_ONE,&_THREE)) { \
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
      ImGui::InputText("Name",&ins->name);
      if (ins->type<0 || ins->type>22) ins->type=DIV_INS_FM;
      int insType=ins->type;
      if (ImGui::Combo("Type",&insType,insTypes,23)) {
        ins->type=(DivInstrumentType)insType;
      }

      if (ImGui::BeginTabBar("insEditTab")) {
        if (ins->type==DIV_INS_FM) {
          char label[32];
          float asFloat[256];
          int asInt[256];
          float loopIndicator[256];
          if (ImGui::BeginTabItem("FM")) {
            if (ImGui::BeginTable("fmDetails",3,ImGuiTableFlags_SizingStretchSame)) {
              ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.0);
              ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0);
              ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.0);
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              P(ImGui::SliderScalar(FM_NAME(FM_FB),ImGuiDataType_U8,&ins->fm.fb,&_ZERO,&_SEVEN));
              P(ImGui::SliderScalar(FM_NAME(FM_FMS),ImGuiDataType_U8,&ins->fm.fms,&_ZERO,&_SEVEN));
              ImGui::TableNextColumn();
              P(ImGui::SliderScalar(FM_NAME(FM_ALG),ImGuiDataType_U8,&ins->fm.alg,&_ZERO,&_SEVEN));
              P(ImGui::SliderScalar(FM_NAME(FM_AMS),ImGuiDataType_U8,&ins->fm.ams,&_ZERO,&_THREE));
              ImGui::TableNextColumn();
              drawAlgorithm(ins->fm.alg,FM_ALGS_4OP,ImVec2(ImGui::GetContentRegionAvail().x,48.0*dpiScale));
              ImGui::EndTable();
            }
            if (ImGui::BeginTable("FMOperators",2,ImGuiTableFlags_SizingStretchSame)) {
              for (int i=0; i<4; i++) {
                DivInstrumentFM::Operator& op=ins->fm.op[opOrder[i]];
                if ((i+1)&1) ImGui::TableNextRow();
                ImGui::TableNextColumn();

                ImGui::PushID(fmt::sprintf("op%d",i).c_str());
                ImGui::Dummy(ImVec2(dpiScale,dpiScale));
                ImGui::Text("Operator %d",i+1);
                //48.0 controls vert scaling; default 96
                drawFMEnv(op.tl,op.ar,op.dr,op.d2r,op.rr,op.sl,ImVec2(ImGui::GetContentRegionAvail().x,48.0*dpiScale));
                P(ImGui::SliderScalar(FM_NAME(FM_AR),ImGuiDataType_U8,&op.ar,&_ZERO,&_THIRTY_ONE));
                P(ImGui::SliderScalar(FM_NAME(FM_DR),ImGuiDataType_U8,&op.dr,&_ZERO,&_THIRTY_ONE));
                P(ImGui::SliderScalar(FM_NAME(FM_SL),ImGuiDataType_U8,&op.sl,&_ZERO,&_FIFTEEN));
                P(ImGui::SliderScalar(FM_NAME(FM_D2R),ImGuiDataType_U8,&op.d2r,&_ZERO,&_THIRTY_ONE));
                P(ImGui::SliderScalar(FM_NAME(FM_RR),ImGuiDataType_U8,&op.rr,&_ZERO,&_FIFTEEN));
                P(ImGui::SliderScalar(FM_NAME(FM_TL),ImGuiDataType_U8,&op.tl,&_ZERO,&_ONE_HUNDRED_TWENTY_SEVEN));

                ImGui::Separator();

                P(ImGui::SliderScalar(FM_NAME(FM_RS),ImGuiDataType_U8,&op.rs,&_ZERO,&_THREE));
                P(ImGui::SliderScalar(FM_NAME(FM_MULT),ImGuiDataType_U8,&op.mult,&_ZERO,&_FIFTEEN));
                
                int detune=(op.dt&7)-3;
                if (ImGui::SliderInt(FM_NAME(FM_DT),&detune,-3,3)) { PARAMETER
                  op.dt=detune+3;
                }
                P(ImGui::SliderScalar(FM_NAME(FM_DT2),ImGuiDataType_U8,&op.dt2,&_ZERO,&_THREE));
                if (ImGui::IsItemHovered()) {
                  ImGui::SetTooltip("Only for Arcade system");
                }
                bool ssgOn=op.ssgEnv&8;
                unsigned char ssgEnv=op.ssgEnv&7;
                if (ImGui::SliderScalar(FM_NAME(FM_SSG),ImGuiDataType_U8,&ssgEnv,&_ZERO,&_SEVEN,ssgEnvTypes[ssgEnv])) { PARAMETER
                  op.ssgEnv=(op.ssgEnv&8)|(ssgEnv&7);
                }
                if (ImGui::Checkbox("SSG-EG On",&ssgOn)) { PARAMETER
                  op.ssgEnv=(op.ssgEnv&7)|(ssgOn<<3);
                }
                if (ImGui::IsItemHovered()) {
                  ImGui::SetTooltip("Only for Genesis and Neo Geo systems");
                }
                ImGui::SameLine();
                bool amOn=op.am;
                if (ImGui::Checkbox(FM_NAME(FM_AM),&amOn)) { PARAMETER
                  op.am=amOn;
                }
                ImGui::PopID();
              }
              ImGui::EndTable();
            }
            ImGui::EndTabItem();
          }
          if (ImGui::BeginTabItem("FM Macros")) {
            MACRO_BEGIN(0);
            NORMAL_MACRO(ins->std.algMacro,ins->std.algMacroLen,ins->std.algMacroLoop,ins->std.algMacroRel,0,7,"alg",FM_NAME(FM_ALG),96,ins->std.algMacroOpen,false,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[0],0,7,NULL);
            NORMAL_MACRO(ins->std.fbMacro,ins->std.fbMacroLen,ins->std.fbMacroLoop,ins->std.fbMacroRel,0,7,"fb",FM_NAME(FM_FB),96,ins->std.fbMacroOpen,false,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[1],0,7,NULL);
            NORMAL_MACRO(ins->std.fmsMacro,ins->std.fmsMacroLen,ins->std.fmsMacroLoop,ins->std.fmsMacroRel,0,7,"fms",FM_NAME(FM_FMS),96,ins->std.fmsMacroOpen,false,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[2],0,7,NULL);
            NORMAL_MACRO(ins->std.amsMacro,ins->std.amsMacroLen,ins->std.amsMacroLoop,ins->std.amsMacroRel,0,3,"ams",FM_NAME(FM_AMS),48,ins->std.amsMacroOpen,false,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[3],0,3,NULL);
            
            NORMAL_MACRO(ins->std.ex1Macro,ins->std.ex1MacroLen,ins->std.ex1MacroLoop,ins->std.ex1MacroRel,0,127,"ex1","AM Depth",128,ins->std.ex1MacroOpen,false,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[4],0,127,NULL);
            NORMAL_MACRO(ins->std.ex2Macro,ins->std.ex2MacroLen,ins->std.ex2MacroLoop,ins->std.ex2MacroRel,0,127,"ex2","PM Depth",128,ins->std.ex2MacroOpen,false,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[5],0,127,NULL);
            NORMAL_MACRO(ins->std.ex3Macro,ins->std.ex3MacroLen,ins->std.ex3MacroLoop,ins->std.ex3MacroRel,0,255,"ex3","LFO Speed",128,ins->std.ex3MacroOpen,false,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[6],0,255,NULL);
            NORMAL_MACRO(ins->std.waveMacro,ins->std.waveMacroLen,ins->std.waveMacroLoop,ins->std.waveMacroRel,0,3,"wave","LFO Shape",48,ins->std.waveMacroOpen,false,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_WAVE],mmlString[7],0,3,&macroLFOWaves);
            MACRO_END;
            ImGui::EndTabItem();
          }
          for (int i=0; i<4; i++) {
            snprintf(label,31,"OP%d Macros",i+1);
            if (ImGui::BeginTabItem(label)) {
              ImGui::PushID(i);
              MACRO_BEGIN(0);
              int ordi=orderedOps[i];
              OP_MACRO(ins->std.opMacros[ordi].tlMacro,ins->std.opMacros[ordi].tlMacroLen,ins->std.opMacros[ordi].tlMacroLoop,ins->std.opMacros[ordi].tlMacroRel,127,ordi,"tl",FM_NAME(FM_TL),128,ins->std.opMacros[ordi].tlMacroOpen,false,NULL,mmlString[0]);
              OP_MACRO(ins->std.opMacros[ordi].arMacro,ins->std.opMacros[ordi].arMacroLen,ins->std.opMacros[ordi].arMacroLoop,ins->std.opMacros[ordi].arMacroRel,31,ordi,"ar",FM_NAME(FM_AR),64,ins->std.opMacros[ordi].arMacroOpen,false,NULL,mmlString[1]);
              OP_MACRO(ins->std.opMacros[ordi].drMacro,ins->std.opMacros[ordi].drMacroLen,ins->std.opMacros[ordi].drMacroLoop,ins->std.opMacros[ordi].drMacroRel,31,ordi,"dr",FM_NAME(FM_DR),64,ins->std.opMacros[ordi].drMacroOpen,false,NULL,mmlString[2]);
              OP_MACRO(ins->std.opMacros[ordi].d2rMacro,ins->std.opMacros[ordi].d2rMacroLen,ins->std.opMacros[ordi].d2rMacroLoop,ins->std.opMacros[ordi].d2rMacroRel,31,ordi,"d2r",FM_NAME(FM_D2R),64,ins->std.opMacros[ordi].d2rMacroOpen,false,NULL,mmlString[3]);
              OP_MACRO(ins->std.opMacros[ordi].rrMacro,ins->std.opMacros[ordi].rrMacroLen,ins->std.opMacros[ordi].rrMacroLoop,ins->std.opMacros[ordi].rrMacroRel,15,ordi,"rr",FM_NAME(FM_RR),64,ins->std.opMacros[ordi].rrMacroOpen,false,NULL,mmlString[4]);
              OP_MACRO(ins->std.opMacros[ordi].slMacro,ins->std.opMacros[ordi].slMacroLen,ins->std.opMacros[ordi].slMacroLoop,ins->std.opMacros[ordi].slMacroRel,15,ordi,"sl",FM_NAME(FM_SL),64,ins->std.opMacros[ordi].slMacroOpen,false,NULL,mmlString[5]);
              OP_MACRO(ins->std.opMacros[ordi].rsMacro,ins->std.opMacros[ordi].rsMacroLen,ins->std.opMacros[ordi].rsMacroLoop,ins->std.opMacros[ordi].rsMacroRel,3,ordi,"rs",FM_NAME(FM_RS),32,ins->std.opMacros[ordi].rsMacroOpen,false,NULL,mmlString[6]);
              OP_MACRO(ins->std.opMacros[ordi].multMacro,ins->std.opMacros[ordi].multMacroLen,ins->std.opMacros[ordi].multMacroLoop,ins->std.opMacros[ordi].multMacroRel,15,ordi,"mult",FM_NAME(FM_MULT),64,ins->std.opMacros[ordi].multMacroOpen,false,NULL,mmlString[7]);
              OP_MACRO(ins->std.opMacros[ordi].dtMacro,ins->std.opMacros[ordi].dtMacroLen,ins->std.opMacros[ordi].dtMacroLoop,ins->std.opMacros[ordi].dtMacroRel,7,ordi,"dt",FM_NAME(FM_DT),64,ins->std.opMacros[ordi].dtMacroOpen,false,NULL,mmlString[8]);
              OP_MACRO(ins->std.opMacros[ordi].dt2Macro,ins->std.opMacros[ordi].dt2MacroLen,ins->std.opMacros[ordi].dt2MacroLoop,ins->std.opMacros[ordi].dt2MacroRel,3,ordi,"dt2",FM_NAME(FM_DT2),32,ins->std.opMacros[ordi].dt2MacroOpen,false,NULL,mmlString[9]);
              OP_MACRO(ins->std.opMacros[ordi].amMacro,ins->std.opMacros[ordi].amMacroLen,ins->std.opMacros[ordi].amMacroLoop,ins->std.opMacros[ordi].amMacroRel,1,ordi,"am",FM_NAME(FM_AM),32,ins->std.opMacros[ordi].amMacroOpen,true,NULL,mmlString[10]);
              OP_MACRO(ins->std.opMacros[ordi].ssgMacro,ins->std.opMacros[ordi].ssgMacroLen,ins->std.opMacros[ordi].ssgMacroLoop,ins->std.opMacros[ordi].ssgMacroRel,4,ordi,"ssg",FM_NAME(FM_SSG),64,ins->std.opMacros[ordi].ssgMacroOpen,true,ssgEnvBits,mmlString[11]);
              MACRO_END;
              ImGui::PopID();
              ImGui::EndTabItem();
            }
          }
        }
        if (ins->type==DIV_INS_GB) if (ImGui::BeginTabItem("Game Boy")) {
          P(ImGui::SliderScalar("Volume",ImGuiDataType_U8,&ins->gb.envVol,&_ZERO,&_FIFTEEN));
          P(ImGui::SliderScalar("Envelope Length",ImGuiDataType_U8,&ins->gb.envLen,&_ZERO,&_SEVEN));
          P(ImGui::SliderScalar("Sound Length",ImGuiDataType_U8,&ins->gb.soundLen,&_ZERO,&_SIXTY_FOUR,ins->gb.soundLen>63?"Infinity":"%d"));
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
          ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(ins->c64.triOn)?0.6f:0.2f,0.2f,1.0f));
          if (ImGui::Button("tri")) { PARAMETER
            ins->c64.triOn=!ins->c64.triOn;
          }
          ImGui::PopStyleColor();
          ImGui::SameLine();
          ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(ins->c64.sawOn)?0.6f:0.2f,0.2f,1.0f));
          if (ImGui::Button("saw")) { PARAMETER
            ins->c64.sawOn=!ins->c64.sawOn;
          }
          ImGui::PopStyleColor();
          ImGui::SameLine();
          ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(ins->c64.pulseOn)?0.6f:0.2f,0.2f,1.0f));
          if (ImGui::Button("pulse")) { PARAMETER
            ins->c64.pulseOn=!ins->c64.pulseOn;
          }
          ImGui::PopStyleColor();
          ImGui::SameLine();
          ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(ins->c64.noiseOn)?0.6f:0.2f,0.2f,1.0f));
          if (ImGui::Button("noise")) { PARAMETER
            ins->c64.noiseOn=!ins->c64.noiseOn;
          }
          ImGui::PopStyleColor();

          P(ImGui::SliderScalar("Attack",ImGuiDataType_U8,&ins->c64.a,&_ZERO,&_FIFTEEN));
          P(ImGui::SliderScalar("Decay",ImGuiDataType_U8,&ins->c64.d,&_ZERO,&_FIFTEEN));
          P(ImGui::SliderScalar("Sustain",ImGuiDataType_U8,&ins->c64.s,&_ZERO,&_FIFTEEN));
          P(ImGui::SliderScalar("Release",ImGuiDataType_U8,&ins->c64.r,&_ZERO,&_FIFTEEN));
          P(ImGui::SliderScalar("Duty",ImGuiDataType_U16,&ins->c64.duty,&_ZERO,&_FOUR_THOUSAND_NINETY_FIVE));

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
          
          P(ImGui::SliderScalar("Cutoff",ImGuiDataType_U16,&ins->c64.cut,&_ZERO,&_TWO_THOUSAND_FORTY_SEVEN));
          P(ImGui::SliderScalar("Resonance",ImGuiDataType_U8,&ins->c64.res,&_ZERO,&_FIFTEEN));

          ImGui::Text("Filter Mode");
          ImGui::SameLine();
          ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(ins->c64.lp)?0.6f:0.2f,0.2f,1.0f));
          if (ImGui::Button("low")) { PARAMETER
            ins->c64.lp=!ins->c64.lp;
          }
          ImGui::PopStyleColor();
          ImGui::SameLine();
          ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(ins->c64.bp)?0.6f:0.2f,0.2f,1.0f));
          if (ImGui::Button("band")) { PARAMETER
            ins->c64.bp=!ins->c64.bp;
          }
          ImGui::PopStyleColor();
          ImGui::SameLine();
          ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(ins->c64.hp)?0.6f:0.2f,0.2f,1.0f));
          if (ImGui::Button("high")) { PARAMETER
            ins->c64.hp=!ins->c64.hp;
          }
          ImGui::PopStyleColor();
          ImGui::SameLine();
          ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(ins->c64.ch3off)?0.6f:0.2f,0.2f,1.0f));
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
          ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Macros")) {
          float asFloat[256];
          int asInt[256];
          float loopIndicator[256];
          const char* volumeLabel="Volume";

          int volMax=(ins->type==DIV_INS_PCE || ins->type==DIV_INS_AY8930)?31:15;
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
          if (ins->type==DIV_INS_AMIGA) {
            volMax=64;
          }
          if (ins->type==DIV_INS_FM) {
            volMax=127;
          }
          if (ins->type==DIV_INS_GB) {
            volMax=0;
          }

          bool arpMode=ins->std.arpMacroMode;

          const char* dutyLabel="Duty/Noise";
          int dutyMax=(ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930)?31:3;
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
          if (ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930 || ins->type==DIV_INS_FM) {
            dutyLabel="Noise Freq";
          }
          if (ins->type==DIV_INS_AY8930) {
            dutyMax=255;
          }
          if (ins->type==DIV_INS_TIA || ins->type==DIV_INS_PCE || ins->type==DIV_INS_AMIGA) {
            dutyMax=0;
          }
          bool dutyIsRel=(ins->type==DIV_INS_C64 && !ins->c64.dutyIsAbs);

          int waveMax=(ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930)?3:63;
          bool bitMode=false;
          if (ins->type==DIV_INS_C64 || ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930 || ins->type==DIV_INS_SAA1099) {
            bitMode=true;
          }
          if (ins->type==DIV_INS_STD) waveMax=0;
          if (ins->type==DIV_INS_TIA) waveMax=15;
          if (ins->type==DIV_INS_C64) waveMax=4;
          if (ins->type==DIV_INS_SAA1099) waveMax=2;
          if (ins->type==DIV_INS_FM) waveMax=0;

          const char** waveNames=ayShapeBits;
          if (ins->type==DIV_INS_C64) waveNames=c64ShapeBits;

          int ex1Max=(ins->type==DIV_INS_AY8930)?8:0;
          int ex2Max=(ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930)?4:0;

          if (ins->type==DIV_INS_C64) {
            ex1Max=4;
            ex2Max=15;
          }
          if (ins->type==DIV_INS_SAA1099) ex1Max=8;

          if (settings.macroView==0) { // modern view
            MACRO_BEGIN(28*dpiScale);
            if (volMax>0) {
              NORMAL_MACRO(ins->std.volMacro,ins->std.volMacroLen,ins->std.volMacroLoop,ins->std.volMacroRel,volMin,volMax,"vol",volumeLabel,160,ins->std.volMacroOpen,false,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_VOLUME],mmlString[0],volMin,volMax,NULL);
            }
            NORMAL_MACRO(ins->std.arpMacro,ins->std.arpMacroLen,ins->std.arpMacroLoop,ins->std.arpMacroRel,arpMacroScroll,arpMacroScroll+24,"arp","Arpeggio",160,ins->std.arpMacroOpen,false,NULL,true,&arpMacroScroll,(arpMode?0:-80),0,0,&ins->std.arpMacroMode,uiColors[GUI_COLOR_MACRO_PITCH],mmlString[1],-92,94,(ins->std.arpMacroMode?(&macroHoverNote):NULL));
            if (dutyMax>0) {
              NORMAL_MACRO(ins->std.dutyMacro,ins->std.dutyMacroLen,ins->std.dutyMacroLoop,ins->std.dutyMacroRel,0,dutyMax,"duty",dutyLabel,160,ins->std.dutyMacroOpen,false,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[2],0,dutyMax,NULL);
            }
            if (waveMax>0) {
              NORMAL_MACRO(ins->std.waveMacro,ins->std.waveMacroLen,ins->std.waveMacroLoop,ins->std.waveMacroRel,0,waveMax,"wave","Waveform",bitMode?64:160,ins->std.waveMacroOpen,bitMode,waveNames,false,NULL,0,0,((ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930)?1:0),NULL,uiColors[GUI_COLOR_MACRO_WAVE],mmlString[3],0,waveMax,NULL);
            }
            if (ex1Max>0) {
              if (ins->type==DIV_INS_C64) {
                NORMAL_MACRO(ins->std.ex1Macro,ins->std.ex1MacroLen,ins->std.ex1MacroLoop,ins->std.ex1MacroRel,0,ex1Max,"ex1","Filter Mode",64,ins->std.ex1MacroOpen,true,filtModeBits,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[4],0,ex1Max,NULL);
              } else if (ins->type==DIV_INS_SAA1099) {
                NORMAL_MACRO(ins->std.ex1Macro,ins->std.ex1MacroLen,ins->std.ex1MacroLoop,ins->std.ex1MacroRel,0,ex1Max,"ex1","Envelope",160,ins->std.ex1MacroOpen,true,saaEnvBits,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[4],0,ex1Max,NULL);
              } else {
                NORMAL_MACRO(ins->std.ex1Macro,ins->std.ex1MacroLen,ins->std.ex1MacroLoop,ins->std.ex1MacroRel,0,ex1Max,"ex1","Duty",160,ins->std.ex1MacroOpen,false,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[4],0,ex1Max,NULL);
              }
            }
            if (ex2Max>0) {
              if (ins->type==DIV_INS_C64) {
                NORMAL_MACRO(ins->std.ex2Macro,ins->std.ex2MacroLen,ins->std.ex2MacroLoop,ins->std.ex2MacroRel,0,ex2Max,"ex2","Resonance",64,ins->std.ex2MacroOpen,false,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[5],0,ex2Max,NULL);
              } else {
                NORMAL_MACRO(ins->std.ex2Macro,ins->std.ex2MacroLen,ins->std.ex2MacroLoop,ins->std.ex2MacroRel,0,ex2Max,"ex2","Envelope",64,ins->std.ex2MacroOpen,true,ayEnvBits,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[5],0,ex2Max,NULL);
              }
            }
            if (ins->type==DIV_INS_C64) {
              NORMAL_MACRO(ins->std.ex3Macro,ins->std.ex3MacroLen,ins->std.ex3MacroLoop,ins->std.ex3MacroRel,0,2,"ex3","Special",32,ins->std.ex3MacroOpen,true,c64SpecialBits,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[6],0,2,NULL);
            }
            if (ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930) {
              NORMAL_MACRO(ins->std.ex3Macro,ins->std.ex3MacroLen,ins->std.ex3MacroLoop,ins->std.ex3MacroRel,0,15,"ex3","AutoEnv Num",96,ins->std.ex3MacroOpen,false,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[6],0,15,NULL);
              NORMAL_MACRO(ins->std.algMacro,ins->std.algMacroLen,ins->std.algMacroLoop,ins->std.algMacroRel,0,15,"alg","AutoEnv Den",96,ins->std.algMacroOpen,false,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[7],0,15,NULL);
            }
            if (ins->type==DIV_INS_AY8930) {
              // oh my i am running out of macros
              NORMAL_MACRO(ins->std.fbMacro,ins->std.fbMacroLen,ins->std.fbMacroLoop,ins->std.fbMacroRel,0,8,"fb","Noise AND Mask",96,ins->std.fbMacroOpen,true,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[8],0,8,NULL);
              NORMAL_MACRO(ins->std.fmsMacro,ins->std.fmsMacroLen,ins->std.fmsMacroLoop,ins->std.fmsMacroRel,0,8,"fms","Noise OR Mask",96,ins->std.fmsMacroOpen,true,NULL,false,NULL,0,0,0,NULL,uiColors[GUI_COLOR_MACRO_OTHER],mmlString[9],0,8,NULL);
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

#undef P
#undef PARAMETER

void FurnaceGUI::drawWaveList() {
  if (nextWindow==GUI_WINDOW_WAVE_LIST) {
    waveListOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!waveListOpen) return;
  float wavePreview[256];
  if (ImGui::Begin("Wavetables",&waveListOpen)) {
    if (ImGui::Button(ICON_FA_PLUS "##WaveAdd")) {
      doAction(GUI_ACTION_WAVE_LIST_ADD);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FILES_O "##WaveClone")) {
      doAction(GUI_ACTION_WAVE_LIST_DUPLICATE);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FOLDER_OPEN "##WaveLoad")) {
      doAction(GUI_ACTION_WAVE_LIST_OPEN);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FLOPPY_O "##WaveSave")) {
      doAction(GUI_ACTION_WAVE_LIST_SAVE);
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("WaveUp",ImGuiDir_Up)) {
      doAction(GUI_ACTION_WAVE_LIST_UP);
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("WaveDown",ImGuiDir_Down)) {
      doAction(GUI_ACTION_WAVE_LIST_DOWN);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_TIMES "##WaveDelete")) {
      doAction(GUI_ACTION_WAVE_LIST_DELETE);
    }
    ImGui::Separator();
    if (ImGui::BeginTable("WaveListScroll",1,ImGuiTableFlags_ScrollY)) {
      for (int i=0; i<(int)e->song.wave.size(); i++) {
        DivWavetable* wave=e->song.wave[i];
        for (int i=0; i<wave->len; i++) {
          wavePreview[i]=wave->data[i];
        }
        if (wave->len>0) wavePreview[wave->len]=wave->data[wave->len-1];
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Selectable(fmt::sprintf("%d##_WAVE%d\n",i,i).c_str(),curWave==i)) {
          curWave=i;
        }
        if (ImGui::IsItemHovered()) {
          if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            waveEditOpen=true;
          }
        }
        ImGui::SameLine();
        PlotNoLerp(fmt::sprintf("##_WAVEP%d",i).c_str(),wavePreview,wave->len+1,0,NULL,0,wave->max);
      }
      ImGui::EndTable();
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_WAVE_LIST;
  ImGui::End();
}

void FurnaceGUI::drawWaveEdit() {
  if (nextWindow==GUI_WINDOW_WAVE_EDIT) {
    waveEditOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!waveEditOpen) return;
  float wavePreview[256];
  ImGui::SetNextWindowSizeConstraints(ImVec2(450.0f*dpiScale,300.0f*dpiScale),ImVec2(scrW*dpiScale,scrH*dpiScale));
  if (ImGui::Begin("Wavetable Editor",&waveEditOpen,settings.allowEditDocking?0:ImGuiWindowFlags_NoDocking)) {
    if (curWave<0 || curWave>=(int)e->song.wave.size()) {
      ImGui::Text("no wavetable selected");
    } else {
      DivWavetable* wave=e->song.wave[curWave];
      ImGui::Text("Width");
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("use a width of 32 on Game Boy and PC Engine.\nany other widths will be scaled during playback.");
      }
      ImGui::SameLine();
      ImGui::SetNextItemWidth(128.0f*dpiScale);
      if (ImGui::InputInt("##_WTW",&wave->len,1,2)) {
        if (wave->len>256) wave->len=256;
        if (wave->len<1) wave->len=1;
        e->notifyWaveChange(curWave);
        if (wavePreviewOn) e->previewWave(curWave,wavePreviewNote);
        modified=true;
      }
      ImGui::SameLine();
      ImGui::Text("Height");
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("use a height of:\n- 15 for Game Boy\n- 31 for PC Engine\nany other heights will be scaled during playback.");
      }
      ImGui::SameLine();
      ImGui::SetNextItemWidth(128.0f*dpiScale);
      if (ImGui::InputInt("##_WTH",&wave->max,1,2)) {
        if (wave->max>255) wave->max=255;
        if (wave->max<1) wave->max=1;
        e->notifyWaveChange(curWave);
        modified=true;
      }
      for (int i=0; i<wave->len; i++) {
        wavePreview[i]=wave->data[i];
      }
      if (wave->len>0) wavePreview[wave->len]=wave->data[wave->len-1];

      if (ImGui::InputText("##MMLWave",&mmlStringW)) {
        decodeMMLStrW(mmlStringW,wave->data,wave->len,wave->max);
      }
      if (!ImGui::IsItemActive()) {
        encodeMMLStr(mmlStringW,wave->data,wave->len,-1,-1);
      }

      ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(0.0f,0.0f));
      ImVec2 contentRegion=ImGui::GetContentRegionAvail();
      PlotNoLerp("##Waveform",wavePreview,wave->len+1,0,NULL,0,wave->max,contentRegion);
      if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        waveDragStart=ImGui::GetItemRectMin();
        waveDragAreaSize=contentRegion;
        waveDragMin=0;
        waveDragMax=wave->max;
        waveDragLen=wave->len;
        waveDragActive=true;
        waveDragTarget=wave->data;
        processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
        e->notifyWaveChange(curWave);
        modified=true;
      }
      ImGui::PopStyleVar();
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_WAVE_EDIT;
  ImGui::End();
}

