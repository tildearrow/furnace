/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2026 tildearrow and contributors
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

#include "insEditCommon.h"
#include "../guiConst.h"
#include "../intConst.h"
#include "IconsFontAwesome4.h"
extern "C" {
#include "../../extern/Nuked-OPLL/opll.h"
}

// TODO: improve this code. it is horrible.

const char* ssgEnvTypes[8]={
  _N("Down Down Down"),
  _N("Down."),
  _N("Down Up Down Up"),
  _N("Down UP"),
  _N("Up Up Up"),
  _N("Up."),
  _N("Up Down Up Down"),
  _N("Up DOWN")
};

const char* opllVariants[4]={
  "OPLL",
  "YMF281",
  "YM2423",
  "VRC7"
};

const char* opllInsNames[4][17]={
  /* YM2413 */ {
    _N("User"),
    _N("1. Violin"),
    _N("2. Guitar"),
    _N("3. Piano"),
    _N("4. Flute"),
    _N("5. Clarinet"),
    _N("6. Oboe"),
    _N("7. Trumpet"),
    _N("8. Organ"),
    _N("9. Horn"),
    _N("10. Synth"),
    _N("11. Harpsichord"),
    _N("12. Vibraphone"),
    _N("13. Synth Bass"),
    _N("14. Acoustic Bass"),
    _N("15. Electric Guitar"),
    _N("Drums")
  },
  /* YMF281 */ {
    _N("User"),
    _N("1. Electric String"),
    _N("2. Bow wow"),
    _N("3. Electric Guitar"),
    _N("4. Organ"),
    _N("5. Clarinet"),
    _N("6. Saxophone"),
    _N("7. Trumpet"),
    _N("8. Street Organ"),
    _N("9. Synth Brass"),
    _N("10. Electric Piano"),
    _N("11. Bass"),
    _N("12. Vibraphone"),
    _N("13. Chime"),
    _N("14. Tom Tom II"),
    _N("15. Noise"),
    _N("Drums")
  },
  /* YM2423 */ {
    _N("User"),
    _N("1. Strings"),
    _N("2. Guitar"),
    _N("3. Electric Guitar"),
    _N("4. Electric Piano"),
    _N("5. Flute"),
    _N("6. Marimba"),
    _N("7. Trumpet"),
    _N("8. Harmonica"),
    _N("9. Tuba"),
    _N("10. Synth Brass"),
    _N("11. Short Saw"),
    _N("12. Vibraphone"),
    _N("13. Electric Guitar 2"),
    _N("14. Synth Bass"),
    _N("15. Sitar"),
    _N("Drums")
  },
  // stolen from FamiTracker
  /* VRC7 */ {
    _N("User"),
    _N("1. Bell"),
    _N("2. Guitar"),
    _N("3. Piano"),
    _N("4. Flute"),
    _N("5. Clarinet"),
    _N("6. Rattling Bell"),
    _N("7. Trumpet"),
    _N("8. Reed Organ"),
    _N("9. Soft Bell"),
    _N("10. Xylophone"),
    _N("11. Vibraphone"),
    _N("12. Brass"),
    _N("13. Bass Guitar"),
    _N("14. Synth"),
    _N("15. Chorus"),
    _N("Drums")
  }
};

const char* oplWaveforms[8]={
  _N("Sine"),
  _N("Half Sine"),
  _N("Absolute Sine"),
  _N("Quarter Sine"),
  _N("Squished Sine"),
  _N("Squished AbsSine"),
  _N("Square"),
  _N("Derived Square")
};

const char* oplWaveformsStandard[8]={
  _N("Sine"),
  _N("Half Sine"),
  _N("Absolute Sine"),
  _N("Pulse Sine"),
  _N("Sine (Even Periods)"),
  _N("AbsSine (Even Periods)"),
  _N("Square"),
  _N("Derived Square")
};

const char* opzWaveforms[8]={
  _N("Sine"),
  _N("Triangle"),
  _N("Cut Sine"),
  _N("Cut Triangle"),
  _N("Squished Sine"),
  _N("Squished Triangle"),
  _N("Squished AbsSine"),
  _N("Squished AbsTriangle")
};

const char* oplDrumNames[4]={
  _N("Snare"),
  _N("Tom"),
  _N("Top"),
  _N("HiHat")
};

const char* esfmNoiseModeNames[4]={
  _N("Normal"),
  _N("Snare"),
  _N("HiHat"),
  _N("Top")
};

const char* esfmNoiseModeDescriptions[4]={
  _N("Noise disabled"),
  _N("Square + noise"),
  _N("Ringmod from OP3 + noise"),
  _N("Ringmod from OP3 + double pitch ModInput\nWARNING - has emulation issues; subject to change")
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
    dl->AddPolyline(waveform,waveformLen+1,color,dpiScale,ImDrawFlags_None);
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

void FurnaceGUI::drawESFMAlgorithm(DivInstrumentESFM& esfm, const ImVec2& size) {
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
  ImU32 colorsL[8];
  for (int i=0; i<8; i++){
    float alpha=(float)i/7.0f;
    ImVec4 color=uiColors[GUI_COLOR_FM_ALG_LINE];
    color.w *= alpha;
    colorsL[i]=ImGui::GetColorU32(color);
  }
  ImGui::ItemSize(size,style.FramePadding.y);
  if (ImGui::ItemAdd(rect,ImGui::GetID("alg"))) {
    ImGui::RenderFrame(rect.Min,rect.Max,ImGui::GetColorU32(uiColors[GUI_COLOR_FM_ALG_BG]),true,style.FrameRounding);
    const float circleRadius=6.0f*dpiScale+1.0f;
    //int modFb = esfm.op[0].modIn&7;
    int mod12 = esfm.op[1].modIn&7;
    int mod23 = esfm.op[2].modIn&7;
    int mod34 = esfm.op[3].modIn&7;
    int out1 = esfm.op[0].outLvl&7;
    int out2 = esfm.op[1].outLvl&7;
    int out3 = esfm.op[2].outLvl&7;
    int out4 = esfm.op[3].outLvl&7;
    bool isMod[4];
    for (int i=0; i<4; i++) {
      DivInstrumentESFM::Operator& opE=esfm.op[i];
      isMod[i]=true;
      if (opE.outLvl==7) isMod[i]=false;
      else if (opE.outLvl>0) {
        if (i==3) isMod[i]=false;
        else {
          DivInstrumentESFM::Operator& opENext=esfm.op[i+1];
          if (opENext.modIn==0) isMod[i]=false;
          else if ((opE.outLvl-opENext.modIn)>=2) isMod[i]=false;
        }
      }
    }

    ImVec2 posOp1=ImLerp(rect.Min,rect.Max,ImVec2(0.2f,0.25f));
    ImVec2 posOp2=ImLerp(rect.Min,rect.Max,ImVec2(0.4f,0.25f));
    ImVec2 posOp3=ImLerp(rect.Min,rect.Max,ImVec2(0.6f,0.25f));
    ImVec2 posOp4=ImLerp(rect.Min,rect.Max,ImVec2(0.8f,0.25f));
    ImVec2 posBusbarOp1=ImLerp(rect.Min,rect.Max,ImVec2(0.2f,0.75f));
    ImVec2 posBusbarOp2=ImLerp(rect.Min,rect.Max,ImVec2(0.4f,0.75f));
    ImVec2 posBusbarOp3=ImLerp(rect.Min,rect.Max,ImVec2(0.6f,0.75f));
    ImVec2 posBusbarOp4=ImLerp(rect.Min,rect.Max,ImVec2(0.8f,0.75f));

    ImVec2 posBusbarStart=ImLerp(rect.Min,rect.Max,ImVec2(0.15f,0.75f));
    ImVec2 posBusbarEnd=ImLerp(rect.Min,rect.Max,ImVec2(0.85f,0.75f));
    bool busbarStartSeen=false;
    for (int i=0; i<4; i++) {
      DivInstrumentESFM::Operator& opE=esfm.op[i];
      if (opE.outLvl>0) {
        if (!busbarStartSeen) {
          busbarStartSeen=true;
          posBusbarStart=ImLerp(rect.Min,rect.Max,ImVec2(0.15f+0.2f*i,0.75f));
        }
        posBusbarEnd=ImLerp(rect.Min,rect.Max,ImVec2(0.25f+0.2f*i,0.75f));
      }
    }

    if (mod12) addAALine(dl,posOp1,posOp2,colorsL[mod12]);
    if (mod23) addAALine(dl,posOp2,posOp3,colorsL[mod23]);
    if (mod34) addAALine(dl,posOp3,posOp4,colorsL[mod34]);
    if (out1) addAALine(dl,posOp1,posBusbarOp1,colorsL[out1]);
    if (out2) addAALine(dl,posOp2,posBusbarOp2,colorsL[out2]);
    if (out3) addAALine(dl,posOp3,posBusbarOp3,colorsL[out3]);
    if (out4) addAALine(dl,posOp4,posBusbarOp4,colorsL[out4]);

    addAALine(dl,posBusbarStart,posBusbarEnd,colorsL[2]);
    // addAALine(dl,posBusbarEnd,posBusbarEnd+ImVec2(-8.0f*dpiScale,-4.0f*dpiScale),colorsL[2]);
    // addAALine(dl,posBusbarEnd,posBusbarEnd+ImVec2(-8.0f*dpiScale,4.0f*dpiScale),colorsL[2]);
    // addAALine(dl,posBusbarStart+ImVec2(4.0f*dpiScale,-4.0f*dpiScale),posBusbarStart+ImVec2(4.0f*dpiScale,4.0f*dpiScale),colorsL[2]);

    dl->AddCircle(posOp1,6.0f*dpiScale+1.0f,isMod[0]?colorM:colorC);
    dl->AddCircleFilled(posOp1,4.0f*dpiScale+1.0f,isMod[0]?colorM:colorC);
    dl->AddCircleFilled(posOp2,4.0f*dpiScale+1.0f,isMod[1]?colorM:colorC);
    dl->AddCircleFilled(posOp3,4.0f*dpiScale+1.0f,isMod[2]?colorM:colorC);
    dl->AddCircleFilled(posOp4,4.0f*dpiScale+1.0f,isMod[3]?colorM:colorC);

    posOp1.x-=ImGui::CalcTextSize("1").x+circleRadius+3.0f*dpiScale;
    posOp2.x-=ImGui::CalcTextSize("2").x+circleRadius+3.0f*dpiScale;
    posOp3.x-=ImGui::CalcTextSize("3").x+circleRadius+3.0f*dpiScale;
    posOp4.x-=ImGui::CalcTextSize("4").x+circleRadius+3.0f*dpiScale;
    posOp1.y-=ImGui::CalcTextSize("1").y*0.5f;
    posOp2.y-=ImGui::CalcTextSize("2").y*0.5f;
    posOp3.y-=ImGui::CalcTextSize("3").y*0.5f;
    posOp4.y-=ImGui::CalcTextSize("4").y*0.5f;
    dl->AddText(posOp1,isMod[0]?colorM:colorC,"1");
    dl->AddText(posOp2,isMod[1]?colorM:colorC,"2");
    dl->AddText(posOp3,isMod[2]?colorM:colorC,"3");
    dl->AddText(posOp4,isMod[3]?colorM:colorC,"4");
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
    } else if (d2r==0.0 || ((instType==DIV_INS_OPL || instType==DIV_INS_SNES || instType==DIV_INS_ESFM) && sus==1.0) || (instType==DIV_INS_OPLL && egt!=0.0)) { //envelope stays at the sustain level forever
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

void FurnaceGUI::kvsConfig(DivInstrument* ins, bool supportsKVS) {
  if (fmPreviewOn) {
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip(_("left click to restart\nmiddle click to pause\nright click to see algorithm"));
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
      updateFMPreview=true;
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) {
      fmPreviewPaused=!fmPreviewPaused;
    }
  } else if (supportsKVS) {
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip(_("left click to configure TL scaling\nright click to see FM preview"));
    }
  } else {
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip(_("right click to see FM preview"));
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
      ImGui::Text(_("operator level changes with volume?"));
      if (ImGui::BeginTable("KVSTable",4,ImGuiTableFlags_BordersInner)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthStretch);
        for (int i=0; i<4; i++) {
          int o=(opCount==4 && ins->type!=DIV_INS_ESFM)?orderedOps[i]:i;
          if (!(i&1)) ImGui::TableNextRow();
          const char* label=_("AUTO##OPKVS");
          if (ins->fm.op[o].kvs==0) {
            label=_("NO##OPKVS");
          } else if (ins->fm.op[o].kvs==1) {
            label=_("YES##OPKVS");
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
      ImGui::Text(_("(copying)")); \
    } else { \
      ImGui::Text(_("(swapping)")); \
    } \
    ImGui::EndDragDropSource(); \
  } else if (ImGui::IsItemHovered()) { \
    ImGui::SetTooltip(_("- drag to swap operator\n- shift-drag to copy operator")); \
  } \
  if (ImGui::BeginDragDropTarget()) { \
    const ImGuiPayload* dragItem=ImGui::AcceptDragDropPayload("FUR_OP"); \
    if (dragItem!=NULL) { \
      if (dragItem->IsDataType("FUR_OP")) { \
        if (opToMove!=i && opToMove>=0) { \
          int destOp=(opCount==4 && ins->type!=DIV_INS_OPL_DRUMS && ins->type!=DIV_INS_ESFM)?opOrder[i]:i; \
          int sourceOp=(opCount==4 && ins->type!=DIV_INS_OPL_DRUMS && ins->type!=DIV_INS_ESFM)?opOrder[opToMove]:opToMove; \
          if (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift)) { \
            e->lockEngine([ins,destOp,sourceOp]() { \
              ins->fm.op[destOp]=ins->fm.op[sourceOp]; \
              ins->esfm.op[destOp]=ins->esfm.op[sourceOp]; \
            }); \
          } else { \
            e->lockEngine([ins,destOp,sourceOp]() { \
              DivInstrumentFM::Operator origOp=ins->fm.op[sourceOp]; \
              DivInstrumentESFM::Operator origOpE=ins->esfm.op[sourceOp]; \
              ins->fm.op[sourceOp]=ins->fm.op[destOp]; \
              ins->esfm.op[sourceOp]=ins->esfm.op[destOp]; \
              ins->fm.op[destOp]=origOp; \
              ins->esfm.op[destOp]=origOpE; \
            }); \
          } \
          PARAMETER; \
        } \
        opToMove=-1; \
      } \
    } \
    ImGui::EndDragDropTarget(); \
  }

void FurnaceGUI::insTabFMModernHeader(DivInstrument* ins) {
  ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
  ImGui::TableNextColumn();
  if (ins->type==DIV_INS_ESFM) {
    ImGui::TableNextColumn();
    CENTER_TEXT(ESFM_SHORT_NAME(ESFM_MODIN));
    ImGui::TextUnformatted(ESFM_SHORT_NAME(ESFM_MODIN));
    TOOLTIP_TEXT(ESFM_LONG_NAME(ESFM_MODIN));
    ImGui::TableNextColumn();
    ImGui::TableNextColumn();
    CENTER_TEXT(ESFM_SHORT_NAME(ESFM_DELAY));
    ImGui::TextUnformatted(ESFM_SHORT_NAME(ESFM_DELAY));
    TOOLTIP_TEXT(ESFM_LONG_NAME(ESFM_DELAY));
  }
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
  if (settings.susPosition==2) {
    ImGui::TableNextColumn();
    CENTER_TEXT(FM_SHORT_NAME(FM_SL));
    ImGui::TextUnformatted(FM_SHORT_NAME(FM_SL));
    TOOLTIP_TEXT(FM_NAME(FM_SL));
  }
  ImGui::TableNextColumn();
  CENTER_TEXT(FM_SHORT_NAME(FM_TL));
  ImGui::TextUnformatted(FM_SHORT_NAME(FM_TL));
  TOOLTIP_TEXT(FM_NAME(FM_TL));
  if (settings.susPosition==3) {
    ImGui::TableNextColumn();
    CENTER_TEXT(FM_SHORT_NAME(FM_SL));
    ImGui::TextUnformatted(FM_SHORT_NAME(FM_SL));
    TOOLTIP_TEXT(FM_NAME(FM_SL));
  }
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
    ImGui::TableNextColumn();
    CENTER_TEXT(FM_SHORT_NAME(FM_TS));
    ImGui::TextUnformatted(FM_SHORT_NAME(FM_TS));
    TOOLTIP_TEXT(FM_NAME(FM_TS));
  }
  if (ins->type==DIV_INS_ESFM) {
    ImGui::TableNextColumn();
    CENTER_TEXT(ESFM_SHORT_NAME(ESFM_OUTLVL));
    ImGui::TextUnformatted(ESFM_SHORT_NAME(ESFM_OUTLVL));
    TOOLTIP_TEXT(ESFM_LONG_NAME(ESFM_OUTLVL));
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
  if (ins->type==DIV_INS_ESFM) {
    ImGui::TableNextColumn();
    CENTER_TEXT(ESFM_SHORT_NAME(ESFM_CT));
    ImGui::TextUnformatted(ESFM_SHORT_NAME(ESFM_CT));
    TOOLTIP_TEXT(ESFM_LONG_NAME(ESFM_CT));
  }
  ImGui::TableNextColumn();
  if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM) {
    CENTER_TEXT(FM_SHORT_NAME(FM_DT));
    ImGui::TextUnformatted(FM_SHORT_NAME(FM_DT));
    TOOLTIP_TEXT(FM_NAME(FM_DT));
    ImGui::TableNextColumn();
  }
  if (ins->type==DIV_INS_ESFM) {
    CENTER_TEXT(ESFM_SHORT_NAME(ESFM_DT));
    ImGui::TextUnformatted(ESFM_SHORT_NAME(ESFM_DT));
    TOOLTIP_TEXT(ESFM_LONG_NAME(ESFM_DT));
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
  if (ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_ESFM) {
    ImGui::TableNextColumn();
    CENTER_TEXT(FM_NAME(FM_WS));
    ImGui::TextUnformatted(FM_NAME(FM_WS));
  } else if (ins->type!=DIV_INS_OPLL && ins->type!=DIV_INS_OPM) {
    ImGui::TableNextColumn();
    CENTER_TEXT(FM_NAME(FM_SSG));
    ImGui::TextUnformatted(FM_NAME(FM_SSG));
  }
  ImGui::TableNextColumn();
  CENTER_TEXT(_("Envelope"));
  ImGui::TextUnformatted(_("Envelope"));
}

void FurnaceGUI::insTabFM(DivInstrument* ins) {
  int opCount=4;
  if (ins->type==DIV_INS_OPLL) opCount=2;
  if (ins->type==DIV_INS_OPL) opCount=(ins->fm.ops==4)?4:2;
  bool opsAreMutable=(ins->type==DIV_INS_FM || ins->type==DIV_INS_OPM || ins->type==DIV_INS_OPZ);

  // this determines which features are hidden from the OPL editor.
  int oplType=0;
  for (int i=0; i<e->song.systemLen; i++) {
    switch (e->song.system[i]) {
      case DIV_SYSTEM_OPL:
      case DIV_SYSTEM_OPL_DRUMS:
      case DIV_SYSTEM_Y8950:
      case DIV_SYSTEM_Y8950_DRUMS:
        if (oplType<1) oplType=1;
        break;
      case DIV_SYSTEM_OPL2:
      case DIV_SYSTEM_OPL2_DRUMS:
        if (oplType<2) oplType=2;
        break;
      case DIV_SYSTEM_OPL3:
      case DIV_SYSTEM_OPL3_DRUMS:
      case DIV_SYSTEM_OPL4:
      case DIV_SYSTEM_OPL4_DRUMS:
      case DIV_SYSTEM_YMU759:
      case DIV_SYSTEM_ESFM: // to assist in porting
        if (oplType<3) oplType=3;
        break;
      default:
        break;
    }
  }
  // expose all features if no OPL chips are present
  if (oplType==0) oplType=3;

  int wsMax=7;
  if (oplType==2) wsMax=3;
  if (oplType==1) wsMax=0;

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

    if (ImGui::BeginTable("fmDetails",3,(ins->type==DIV_INS_ESFM)?ImGuiTableFlags_SizingStretchProp:ImGuiTableFlags_SizingStretchSame)) {
      String blockTxt=_("Automatic");
      if (ins->fm.block>=1) {
        blockTxt=fmt::sprintf("%d",ins->fm.block-1);
      }

      ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,((ins->type==DIV_INS_ESFM)?0.40f:0.0f));
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,((ins->type==DIV_INS_ESFM)?0.25f:0.0f));
      ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,((ins->type==DIV_INS_ESFM)?0.35f:0.0f));

      ImGui::TableNextRow();
      switch (ins->type) {
        case DIV_INS_FM:
        case DIV_INS_OPM:
          ImGui::TableNextColumn();
          P(CWSliderScalar(FM_NAME(FM_FB),ImGuiDataType_U8,&ins->fm.fb,&_ZERO,&_SEVEN)); rightClickable
          P(CWSliderScalar(FM_NAME(FM_FMS),ImGuiDataType_U8,&ins->fm.fms,&_ZERO,&_SEVEN)); rightClickable
          if (ins->type==DIV_INS_FM) {
            P(CWSliderScalar(FM_NAME(FM_BLOCK),ImGuiDataType_U8,&ins->fm.block,&_ZERO,&_EIGHT,blockTxt.c_str())); rightClickable
          }
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
          P(ImGui::Checkbox("FMS LFO2##FMS",&ins->fm.fmsLFO));
          ImGui::SameLine();
          P(ImGui::Checkbox("AMS LFO2##AMS",&ins->fm.amsLFO));
          ImGui::TableNextColumn();
          P(CWSliderScalar(FM_NAME(FM_ALG),ImGuiDataType_U8,&ins->fm.alg,&_ZERO,&_SEVEN)); rightClickable
          P(CWSliderScalar(FM_NAME(FM_AMS),ImGuiDataType_U8,&ins->fm.ams,&_ZERO,&_THREE)); rightClickable
          ImGui::AlignTextToFramePadding();
          ImGui::TextUnformatted(_("Tremolo"));
          ImGui::SameLine();
          if (ImGui::Button(ins->fm.tremLFO?"LFO4###TLFO":"LFO3###TLFO")) {
            ins->fm.tremLFO=!ins->fm.tremLFO;
            PARAMETER;
          }
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

          if (ImGui::Button(_("Request from TX81Z"))) {
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
          P(CWSliderScalar(FM_NAME(FM_BLOCK),ImGuiDataType_U8,&ins->fm.block,&_ZERO,&_EIGHT,blockTxt.c_str())); rightClickable
          ImGui::TableNextColumn();
          P(CWSliderScalar(FM_NAME(FM_ALG),ImGuiDataType_U8,&ins->fm.alg,&_ZERO,&algMax)); rightClickable
          if (ins->type==DIV_INS_OPL) {
            ImGui::BeginDisabled(ins->fm.opllPreset==16);
            if (fourOp || oplType>=3) {
              pushWarningColor(oplType<3);
              if (ImGui::Checkbox("4-op",&fourOp)) { PARAMETER
                ins->fm.ops=fourOp?4:2;
              }
              if (oplType<3 && ImGui::IsItemHovered()) {
                ImGui::SetTooltip(_("4-op mode is not available in OPL1/2!"));
              }
              popWarningColor();
            }
            ImGui::EndDisabled();
            ImGui::SameLine();
            if (ImGui::Checkbox(_("Drums"),&drums)) { PARAMETER
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
          ImGui::EndDisabled();
          P(CWSliderScalar(FM_NAME(FM_BLOCK),ImGuiDataType_U8,&ins->fm.block,&_ZERO,&_EIGHT,blockTxt.c_str())); rightClickable
          if (ins->fm.opllPreset!=0) {
            ins->fm.op[1].tl&=15;
            P(CWSliderScalar(_("Volume##TL"),ImGuiDataType_U8,&ins->fm.op[1].tl,&_FIFTEEN,&_ZERO)); rightClickable
          }
          ImGui::TableNextColumn();
          if (ImGui::Checkbox(FM_NAME(FM_SUS),&sus)) { PARAMETER
            ins->fm.alg=sus;
          }
          ImGui::BeginDisabled(ins->fm.opllPreset!=0);
          if (ImGui::Checkbox(FM_NAME(FM_DC),&dc)) { PARAMETER
            fmOrigin.fms=dc;
          }
          ImGui::SameLine();
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

          if (ImGui::BeginCombo("##LLPreset",_(opllInsNames[presentWhich][ins->fm.opllPreset]))) {
            if (isPresentCount>1) {
              if (ImGui::BeginTable("LLPresetList",isPresentCount)) {
                ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
                for (int i=0; i<4; i++) {
                  if (!isPresent[i]) continue;
                  ImGui::TableNextColumn();
                  ImGui::Text(_("%s name"),opllVariants[i]);
                }
                for (int i=0; i<17; i++) {
                  ImGui::TableNextRow();
                  for (int j=0; j<4; j++) {
                    if (!isPresent[j]) continue;
                    ImGui::TableNextColumn();
                    ImGui::PushID(j*17+i);
                    if (ImGui::Selectable(_(opllInsNames[j][i]),ins->fm.opllPreset==i)) { PARAMETER
                      ins->fm.opllPreset=i;
                    }
                    ImGui::PopID();
                    if (ins->fm.opllPreset==i) ImGui::SetItemDefaultFocus();
                  }
                }
                ImGui::EndTable();
              }
            } else {
              for (int i=0; i<17; i++) {
                if (ImGui::Selectable(_(opllInsNames[presentWhich][i]),ins->fm.opllPreset==i)) { PARAMETER
                  ins->fm.opllPreset=i;
                }
                if (ins->fm.opllPreset==i) ImGui::SetItemDefaultFocus();
              }
            }
            ImGui::EndCombo();
          }
          break;
        }
        case DIV_INS_ESFM: {
          ImGui::TableNextColumn();
          P(CWSliderScalar(ESFM_LONG_NAME(ESFM_NOISE),ImGuiDataType_U8,&ins->esfm.noise,&_ZERO,&_THREE,_(esfmNoiseModeNames[ins->esfm.noise&3]))); rightClickable
          ImGui::TextUnformatted(_(esfmNoiseModeDescriptions[ins->esfm.noise&3]));
          ImGui::TableNextColumn();
          P(CWSliderScalar(FM_NAME(FM_BLOCK),ImGuiDataType_U8,&ins->fm.block,&_ZERO,&_EIGHT,blockTxt.c_str())); rightClickable
          ImGui::TableNextColumn();
          if (fmPreviewOn) {
            drawFMPreview(ImVec2(ImGui::GetContentRegionAvail().x,48.0*dpiScale));
            if (!fmPreviewPaused) {
              renderFMPreview(ins,1);
              WAKE_UP;
            }
          } else {
            drawESFMAlgorithm(ins->esfm, ImVec2(ImGui::GetContentRegionAvail().x,48.0*dpiScale));
          }
          kvsConfig(ins);
        }
        default:
          break;
      }
      ImGui::EndTable();
    }
    if (ins->type==DIV_INS_OPLL && ins->fm.opllPreset==16) {
      ImGui::Text(_("this volume slider only works in compatibility (non-drums) system."));
    }

    if (((ins->type==DIV_INS_OPLL || ins->type==DIV_INS_OPL) && ins->fm.opllPreset==16) || ins->type==DIV_INS_OPL_DRUMS) {
      ins->fm.ops=2;
      P(ImGui::Checkbox(_("Fixed frequency mode"),&ins->fm.fixedDrums));
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(_("when enabled, drums will be set to the specified frequencies, ignoring the note."));
      }
      if (ins->fm.fixedDrums) {
        int block=0;
        int fNum=0;
        if (ImGui::BeginTable("fixedDrumSettings",3)) {
          ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
          ImGui::TableNextColumn();
          ImGui::Text(_("Drum"));
          ImGui::TableNextColumn();
          ImGui::Text(_("Block"));
          ImGui::TableNextColumn();
          ImGui::Text(_("FreqNum"));

          DRUM_FREQ(_("Kick"),"##DBlock0","##DFreq0",ins->fm.kickFreq);
          DRUM_FREQ(_("Snare/Hi-hat"),"##DBlock1","##DFreq1",ins->fm.snareHatFreq);
          DRUM_FREQ(_("Tom/Top"),"##DBlock2","##DFreq2",ins->fm.tomTopFreq);
          ImGui::EndTable();
        }
      }
    }

    bool willDisplayOps=true;
    if (ins->type==DIV_INS_OPLL && ins->fm.opllPreset!=0) willDisplayOps=false;
    if (!willDisplayOps && ins->type==DIV_INS_OPLL) {
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
        opllPreview.fms=patch->dc;
        opllPreview.ams=patch->dm;

        opllPreview.op[0].tl=patch->tl;
        opllPreview.op[1].tl=ins->fm.op[1].tl;

        for (int i=0; i<2; i++) {
          opllPreview.op[i].am=patch->am[i];
          opllPreview.op[i].vib=patch->vib[i];
          opllPreview.op[i].ssgEnv=patch->et[i]?8:0;
          opllPreview.op[i].ksr=patch->ksr[i];
          opllPreview.op[i].ksl=patch->ksl[i];
          opllPreview.op[i].mult=patch->multi[i];
          opllPreview.op[i].ar=patch->ar[i];
          opllPreview.op[i].dr=patch->dr[i];
          opllPreview.op[i].sl=patch->sl[i];
          opllPreview.op[i].rr=patch->rr[i];
        }
      }
    }

    ImGui::BeginDisabled(!willDisplayOps);
    if (settings.fmLayout==0 || settings.fmLayout==7) { // modern (why didn't I comment this?!)
      int numCols=15;
      if (ins->type==DIV_INS_OPL ||ins->type==DIV_INS_OPL_DRUMS) numCols=13;
      if (ins->type==DIV_INS_OPLL) numCols=12;
      if (ins->type==DIV_INS_OPZ) numCols=20;
      if (ins->type==DIV_INS_ESFM) numCols=19;
      if (ImGui::BeginTable("FMOperators",numCols,ImGuiTableFlags_SizingStretchProp|ImGuiTableFlags_BordersH|ImGuiTableFlags_BordersOuterV)) {
        // configure columns
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed); // op name
        if (ins->type==DIV_INS_ESFM) {
          ImGui::TableSetupColumn("c0e0",ImGuiTableColumnFlags_WidthStretch,0.05f); // outLvl
          ImGui::TableSetupColumn("c0e1",ImGuiTableColumnFlags_WidthFixed); // -separator-
          ImGui::TableSetupColumn("c0e2",ImGuiTableColumnFlags_WidthStretch,0.05f); // delay
        }
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.05f); // ar
        ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.05f); // dr
        if (settings.susPosition==0) {
          ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthStretch,0.05f); // sl
        }
        if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM) {
          ImGui::TableSetupColumn("c4",ImGuiTableColumnFlags_WidthStretch,0.05f); // d2r
        }
        ImGui::TableSetupColumn("c5",ImGuiTableColumnFlags_WidthStretch,0.05f); // rr
        if (settings.susPosition==1) {
          ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthStretch,0.05f); // sl
        }
        ImGui::TableSetupColumn("c6",ImGuiTableColumnFlags_WidthFixed); // -separator-
        if (settings.susPosition==2) {
          ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthStretch,0.05f); // sl
        }
        ImGui::TableSetupColumn("c7",ImGuiTableColumnFlags_WidthStretch,0.05f); // tl
        if (settings.susPosition==3) {
          ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthStretch,0.05f); // sl
        }
        ImGui::TableSetupColumn("c8",ImGuiTableColumnFlags_WidthStretch,0.05f); // rs/ksl
        if (ins->type==DIV_INS_OPZ) {
          ImGui::TableSetupColumn("c8z0",ImGuiTableColumnFlags_WidthStretch,0.05f); // egs
          ImGui::TableSetupColumn("c8z1",ImGuiTableColumnFlags_WidthStretch,0.05f); // rev
          ImGui::TableSetupColumn("c8z2",ImGuiTableColumnFlags_WidthStretch,0.05f); // ts
        }
        if (ins->type==DIV_INS_ESFM) {
          ImGui::TableSetupColumn("c8e0",ImGuiTableColumnFlags_WidthStretch,0.05f); // outLvl
        }
        ImGui::TableSetupColumn("c9",ImGuiTableColumnFlags_WidthStretch,0.05f); // mult

        if (ins->type==DIV_INS_OPZ) {
          ImGui::TableSetupColumn("c9z",ImGuiTableColumnFlags_WidthStretch,0.05f); // fine
        }

        if (ins->type==DIV_INS_ESFM) {
          ImGui::TableSetupColumn("c9e",ImGuiTableColumnFlags_WidthStretch,0.05f); // ct
        }

        if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM || ins->type==DIV_INS_ESFM) {
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

        float sliderHeight=((ImGui::GetContentRegionAvail().y-ImGui::GetFrameHeightWithSpacing()*(settings.fmLayout==7?4.0f:1.0f))/opCount)-ImGui::GetStyle().ItemSpacing.y;

        // header
        if (settings.fmLayout==0) {
          insTabFMModernHeader(ins);
        }

        // main view
        for (int i=0; i<opCount; i++) {
          DivInstrumentFM::Operator& op=fmOrigin.op[(opCount==4 && ins->type!=DIV_INS_OPL_DRUMS && ins->type!=DIV_INS_ESFM)?opOrder[i]:i];
          DivInstrumentESFM::Operator& opE=ins->esfm.op[i];

          // modern with more labels
          if (settings.fmLayout==7) {
            insTabFMModernHeader(ins);
          }

          ImGui::TableNextRow();
          ImGui::TableNextColumn();

          // push colors
          if (settings.separateFMColors) {
            bool mod=true;
            if (ins->type==DIV_INS_OPL_DRUMS) {
              mod=false;
            } else if (ins->type==DIV_INS_ESFM) {
              // this is the same as the KVS heuristic in platform/esfm.h
              if (opE.outLvl==7) {
                mod=false;
              } else if (opE.outLvl>0) {
                if (i==3) {
                  mod=false;
                } else {
                  DivInstrumentESFM::Operator& opENext=ins->esfm.op[i+1];
                  if (opENext.modIn==0) {
                    mod=false;
                  } else if ((opE.outLvl-opENext.modIn)>=2) {
                    mod=false;
                  }
                }
              }
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

          if (i==0) {
            float sliderMinHeightOPL=ImGui::GetFrameHeight()*4.0+ImGui::GetStyle().ItemSpacing.y*3.0;
            float sliderMinHeightESFM=ImGui::GetFrameHeight()*5.0+ImGui::GetStyle().ItemSpacing.y*4.0;
            if ((ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_OPLL) && sliderHeight<sliderMinHeightOPL) {
              sliderHeight=sliderMinHeightOPL;
            }
            if (ins->type==DIV_INS_ESFM && sliderHeight<sliderMinHeightESFM) {
              sliderHeight=sliderMinHeightESFM;
            }
          }

          ImGui::PushID(fmt::sprintf("op%d",i).c_str());
          String opNameLabel;
          if (ins->type==DIV_INS_OPL_DRUMS) {
            opNameLabel=fmt::sprintf("%s",oplDrumNames[i]);
          } else if (ins->type==DIV_INS_OPL && fmOrigin.opllPreset==16) {
            if (i==1) {
              opNameLabel=_("Kick");
            } else {
              opNameLabel=_("Env");
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
          if (ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_ESFM) {
            maxTl=63;
          }
          int maxArDr=(ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM)?31:15;
          bool ssgOn=op.ssgEnv&8;
          bool ksrOn=op.ksr;
          bool vibOn=op.vib;
          bool susOn=op.sus;
          bool fixedOn=opE.fixed;
          unsigned char ssgEnv=op.ssgEnv&7;

          if (ins->type==DIV_INS_ESFM) {
            ImGui::TableNextColumn();
            CENTER_VSLIDER;
            P(CWVSliderScalar("##MODIN",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&opE.modIn,&_ZERO,&_SEVEN)); rightClickable
            ImGui::TableNextColumn();
            ImGui::Dummy(ImVec2(4.0f*dpiScale,2.0f*dpiScale));
            ImGui::TableNextColumn();
            opE.delay&=7;
            CENTER_VSLIDER;
            P(CWVSliderScalar("##DELAY",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&opE.delay,&_ZERO,&_SEVEN)); rightClickable
          }

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

          if (settings.susPosition==2) {
            ImGui::TableNextColumn();
            op.sl&=15;
            CENTER_VSLIDER;
            P(CWVSliderScalar("##SL",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.sl,&_FIFTEEN,&_ZERO)); rightClickable
          }

          ImGui::TableNextColumn();
          op.tl&=maxTl;
          CENTER_VSLIDER;
          P(CWVSliderScalar("##TL",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.tl,&maxTl,&_ZERO)); rightClickable

          if (settings.susPosition==3) {
            ImGui::TableNextColumn();
            op.sl&=15;
            CENTER_VSLIDER;
            P(CWVSliderScalar("##SL",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.sl,&_FIFTEEN,&_ZERO)); rightClickable
          }

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

            ImGui::TableNextColumn();
            CENTER_VSLIDER;
            P(CWVSliderScalar("##TS",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.ssgEnv,&_ZERO,&_THREE)); rightClickable
          }

          if (ins->type==DIV_INS_ESFM) {
            ImGui::TableNextColumn();
            CENTER_VSLIDER;
            P(CWVSliderScalar("##OUTLVL",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&opE.outLvl,&_ZERO,&_SEVEN)); rightClickable
          }

          ImGui::TableNextColumn();
          pushWarningColor(ins->type==DIV_INS_OPL_DRUMS && i==0);
          CENTER_VSLIDER;
          P(CWVSliderScalar("##MULT",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.mult,&_ZERO,&_FIFTEEN)); rightClickable
          if (ins->type==DIV_INS_OPL_DRUMS && i==0) {
            if (ImGui::IsItemHovered()) {
              ImGui::SetTooltip("%s",_("Snare's multiplier is determined by HiHat's."));
            }
          }
          popWarningColor();

          if (ins->type==DIV_INS_OPZ) {
            ImGui::TableNextColumn();
            CENTER_VSLIDER;
            bool egtOn=op.egt;
            if (!egtOn) {
              P(CWVSliderScalar("##FINE",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.dvb,&_ZERO,&_FIFTEEN)); rightClickable
            }
          }

          if (ins->type==DIV_INS_ESFM) {
            ImGui::TableNextColumn();
            CENTER_VSLIDER;
            P(CWVSliderScalar("##CT",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_S8,&opE.ct,&_MINUS_TWENTY_FOUR,&_TWENTY_FOUR)); rightClickable
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
              bool susOn=op.sus;
              if (egtOn) {
                ImGui::SetCursorPosY(ImGui::GetCursorPosY()+0.5*(sliderHeight-ImGui::GetFrameHeight()*6.0-ImGui::GetStyle().ItemSpacing.y*3.5));
              } else {
                ImGui::SetCursorPosY(ImGui::GetCursorPosY()+0.5*(sliderHeight-ImGui::GetFrameHeight()*4.0-ImGui::GetStyle().ItemSpacing.y*1.0));
              }
              if (ImGui::Checkbox("AM",&amOn)) { PARAMETER
                op.am=amOn;
              }
              if (ImGui::Checkbox(_("TL Ramp##TLRamp"),&ksrOn)) { PARAMETER
                op.ksr=ksrOn;
              }
              if (ImGui::Checkbox(_("Fixed"),&egtOn)) { PARAMETER
                op.egt=egtOn;
              }
              if (egtOn) {
                pushWarningColor(susOn && !e->song.compatFlags.linearPitch);
                if (ImGui::Checkbox(_("Pitch control"),&susOn)) { PARAMETER
                  op.sus=susOn;
                  // HACK: reset zoom and scroll in fixed pitch macros so that they draw correctly
                  ins->temp.vZoom[DIV_MACRO_OP_SSG+(i<<5)]=-1;
                  ins->temp.vZoom[DIV_MACRO_OP_SUS+(i<<5)]=-1;
                }
                popWarningColor();
                if (ImGui::IsItemHovered()) {
                  if (susOn && !e->song.compatFlags.linearPitch) {
                    ImGui::SetTooltip(_("only works on linear pitch! go to Compatibility Flags > Pitch/Playback and set Pitch linearity to Full."));
                  } else {
                    ImGui::SetTooltip(_("use op's arpeggio and pitch macros control instead of block/f-num macros"));
                  }
                }
              }
              if (egtOn && !susOn) {
                int block=op.dt;
                int freqNum=(op.mult<<4)|(op.dvb&15);
                if (ImGui::InputInt(_("Block"),&block,1,1)) {
                  if (block<0) block=0;
                  if (block>7) block=7;
                  op.dt=block;
                }
                if (ImGui::InputInt(_("FreqNum"),&freqNum,1,16)) {
                  if (freqNum<0) freqNum=0;
                  if (freqNum>255) freqNum=255;
                  op.mult=freqNum>>4;
                  op.dvb=freqNum&15;
                }
              }
            } else {
              // disappoint me!!!
              bool displayTLRamp=false;
              if (ins->type==DIV_INS_OPM) {
                displayTLRamp=false;
                for (int i=0; i<e->song.systemLen; i++) {
                  if (e->song.system[i]==DIV_SYSTEM_YM2151) {
                    // test for YM2164 (OPP)
                    if (e->song.systemFlags[i].getInt("chipType",0)==1) {
                      displayTLRamp=true;
                      break;
                    }
                  }
                }
              }
              if (displayTLRamp) {
                ImGui::SetCursorPosY(ImGui::GetCursorPosY()+0.5*(sliderHeight-ImGui::GetFrameHeight()*2.0-ImGui::GetStyle().ItemSpacing.y*1.0));
                if (ImGui::Checkbox("AM##AM",&amOn)) { PARAMETER
                  op.am=amOn;
                }
                // why does the text run away?!
                if (ImGui::Checkbox(_("TL Ramp##TLRamp"),&ksrOn)) { PARAMETER
                  op.ksr=ksrOn;
                }
              } else {
                ImGui::SetCursorPosY(ImGui::GetCursorPosY()+0.5*(sliderHeight-ImGui::GetFrameHeight()));
                if (ImGui::Checkbox("##AM",&amOn)) { PARAMETER
                  op.am=amOn;
                }
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
              if (CWSliderScalar("##SSG",ImGuiDataType_U8,&ssgEnv,&_ZERO,&_SEVEN,_(ssgEnvTypes[ssgEnv]))) { PARAMETER
                op.ssgEnv=(op.ssgEnv&8)|(ssgEnv&7);
              }
            }
          } else if (ins->type==DIV_INS_ESFM) {
            ImGui::TableNextColumn();
            CENTER_VSLIDER;
            P(CWVSliderScalar("##DT",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_S8,&opE.dt,&_MINUS_ONE_HUNDRED_TWENTY_EIGHT,&_ONE_HUNDRED_TWENTY_SEVEN)); rightClickable

            ImGui::TableNextColumn();
            bool amOn=op.am;
            bool leftOn=opE.left;
            bool rightOn=opE.right;

            ImGui::SetCursorPosY(ImGui::GetCursorPosY()+0.5*(sliderHeight-ImGui::GetFrameHeight()*5.0-ImGui::GetStyle().ItemSpacing.y*4.0));
            ImVec2 curPosBeforeDummy = ImGui::GetCursorPos();
            ImGui::Dummy(ImVec2(ImGui::GetFrameHeightWithSpacing()*2.0f+ImGui::CalcTextSize(FM_SHORT_NAME(FM_DAM)).x*2.0f,1.0f));
            ImGui::SetCursorPos(curPosBeforeDummy);

            if (ImGui::BeginTable("panCheckboxes",(fixedOn)?3:2,ImGuiTableFlags_SizingStretchProp)) {
              ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,1.0);
              ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,1.0);
              if (fixedOn) {
                ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,1.2);
              }

              float yCoordBeforeTablePadding=ImGui::GetCursorPosY();
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::SetCursorPosY(yCoordBeforeTablePadding);
              if (ImGui::Checkbox(ESFM_SHORT_NAME(ESFM_LEFT),&leftOn)) { PARAMETER
                opE.left=leftOn;
              }
              ImGui::TableNextColumn();
              ImGui::SetCursorPosY(yCoordBeforeTablePadding);
              if (ImGui::Checkbox(ESFM_SHORT_NAME(ESFM_RIGHT),&rightOn)) { PARAMETER
                opE.right=rightOn;
              }
              if (fixedOn) {
                ImGui::TableNextColumn();
                ImGui::SetCursorPosY(yCoordBeforeTablePadding);
                if (ImGui::Checkbox(FM_SHORT_NAME(FM_AM),&amOn)) { PARAMETER
                  op.am=amOn;
                }
              }
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              if (ImGui::Checkbox(FM_SHORT_NAME(FM_KSR),&ksrOn)) { PARAMETER
                op.ksr=ksrOn;
              }
              ImGui::TableNextColumn();
              if (ImGui::Checkbox(FM_SHORT_NAME(FM_SUS),&susOn)) { PARAMETER
                op.sus=susOn;
              }
              if (fixedOn) {
                bool damOn=op.dam;
                ImGui::TableNextColumn();
                if (ImGui::Checkbox(FM_SHORT_NAME(FM_DAM),&damOn)) { PARAMETER
                  op.dam=damOn;
                }
              }
              ImGui::EndTable();
            }
            ImGui::SetCursorPosY(ImGui::GetCursorPosY()-0.5*ImGui::GetStyle().ItemSpacing.y);
            if (ImGui::Checkbox(ESFM_NAME(ESFM_FIXED),&fixedOn)) { PARAMETER
              opE.fixed=fixedOn;
              // HACK: reset zoom and scroll in fixed pitch macros so that they draw correctly
              ins->temp.vZoom[DIV_MACRO_OP_SSG+(i<<5)]=-1;
              ins->temp.vZoom[DIV_MACRO_OP_DT+(i<<5)]=-1;
            }
            if (ins->type==DIV_INS_ESFM) {
              if (fixedOn) {
                int block=(opE.ct>>2)&7;
                int freqNum=((opE.ct&3)<<8)|((unsigned char)opE.dt);
                if (ImGui::InputInt(_("Block"),&block,1,1)) {
                  if (block<0) block=0;
                  if (block>7) block=7;
                  opE.ct=(opE.ct&(~(7<<2)))|(block<<2);
                }
                if (ImGui::InputInt(_("FreqNum"),&freqNum,1,16)) {
                  if (freqNum<0) freqNum=0;
                  if (freqNum>1023) freqNum=1023;
                  opE.dt=freqNum&0xff;
                  opE.ct=(opE.ct&(~3))|(freqNum>>8);
                }
              } else {
                if (ImGui::BeginTable("amVibCheckboxes",2,ImGuiTableFlags_SizingStretchSame)) {
                  ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.0);
                  ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0);

                  float yCoordBeforeTablePadding=ImGui::GetCursorPosY();
                  ImGui::TableNextRow();
                  ImGui::TableNextColumn();
                  ImGui::SetCursorPosY(yCoordBeforeTablePadding);
                  if (ImGui::Checkbox(FM_SHORT_NAME(FM_AM),&amOn)) { PARAMETER
                    op.am=amOn;
                  }
                  ImGui::TableNextColumn();
                  ImGui::SetCursorPosY(yCoordBeforeTablePadding);
                  if (ImGui::Checkbox(FM_SHORT_NAME(FM_VIB),&vibOn)) { PARAMETER
                    op.vib=vibOn;
                  }
                  bool damOn=op.dam;
                  bool dvbOn=op.dvb;
                  ImGui::TableNextRow();
                  ImGui::TableNextColumn();
                  if (ImGui::Checkbox(FM_SHORT_NAME(FM_DAM),&damOn)) { PARAMETER
                    op.dam=damOn;
                  }
                  ImGui::TableNextColumn();
                  if (ImGui::Checkbox(FM_SHORT_NAME(FM_DVB),&dvbOn)) { PARAMETER
                    op.dvb=dvbOn;
                  }
                  ImGui::EndTable();
                }
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

          if (ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_ESFM) {
            ImGui::TableNextColumn();
            ImGui::Dummy(ImVec2(4.0f*dpiScale,2.0f*dpiScale));
            ImGui::TableNextColumn();

            drawWaveform(op.ws&wsMax,ins->type==DIV_INS_OPZ,ImVec2(ImGui::GetContentRegionAvail().x,sliderHeight-ImGui::GetFrameHeightWithSpacing()*((ins->type==DIV_INS_ESFM && fixedOn)?3.0f:1.0f)));
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            P(CWSliderScalar("##WS",ImGuiDataType_U8,&op.ws,&_ZERO,&wsMax,(ins->type==DIV_INS_OPZ)?opzWaveforms[op.ws&wsMax]:(settings.oplStandardWaveNames?oplWaveformsStandard[op.ws&wsMax]:oplWaveforms[op.ws&wsMax]))); rightClickable
            if (ins->type==DIV_INS_ESFM && fixedOn) {
              if (ImGui::Checkbox(FM_SHORT_NAME(FM_VIB),&vibOn)) { PARAMETER
                op.vib=vibOn;
              }
              bool dvbOn=op.dvb;
              if (ImGui::Checkbox(FM_SHORT_NAME(FM_DVB),&dvbOn)) { PARAMETER
                op.dvb=dvbOn;
              }
            }
          } else if (ins->type==DIV_INS_OPLL || ins->type==DIV_INS_OPM) {
            ImGui::TableNextColumn();
            ImGui::Dummy(ImVec2(4.0f*dpiScale,2.0f*dpiScale));
          }

          ImGui::TableNextColumn();
          drawFMEnv(op.tl&maxTl,op.ar&maxArDr,op.dr&maxArDr,(ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_OPLL || ins->type==DIV_INS_ESFM)?((op.rr&15)*2):op.d2r&31,op.rr&15,op.sl&15,op.sus,op.ssgEnv&8,fmOrigin.alg,maxTl,maxArDr,15,ImVec2(ImGui::GetContentRegionAvail().x,sliderHeight),ins->type);

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
          DivInstrumentFM::Operator& op=fmOrigin.op[(opCount==4 && ins->type!=DIV_INS_OPL_DRUMS && ins->type!=DIV_INS_ESFM)?opOrder[i]:i];
          DivInstrumentESFM::Operator& opE=ins->esfm.op[i];
          if ((settings.fmLayout!=6 && ((i+1)&1)) || i==0 || settings.fmLayout==5) ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::PushID(fmt::sprintf("op%d",i).c_str());

          // push colors
          if (settings.separateFMColors) {
            bool mod=true;
            if (ins->type==DIV_INS_OPL_DRUMS) {
              mod=false;
            } else if (ins->type==DIV_INS_ESFM) {
              // this is the same as the KVS heuristic in platform/esfm.h
              if (opE.outLvl==7) mod=false;
              else if (opE.outLvl>0) {
                if (i==3) mod=false;
                else {
                  DivInstrumentESFM::Operator& opENext=ins->esfm.op[i+1];
                  if (opENext.modIn==0) mod=false;
                  else if ((opE.outLvl-opENext.modIn)>=2) mod=false;
                }
              }
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
              snprintf(tempID,1024,_("Envelope 2 (kick only)"));
            } else {
              snprintf(tempID,1024,_("Envelope"));
            }
          } else {
            snprintf(tempID,1024,_("Operator %d"),i+1);
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
          float waveWidth=140.0*dpiScale*((ins->type==DIV_INS_ESFM)?0.85f:1.0f);
          float waveHeight=sliderHeight-ImGui::GetFrameHeightWithSpacing()*((ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPL || ins->type==DIV_INS_ESFM)?5.0f:4.5f);

          int maxTl=127;
          if (ins->type==DIV_INS_OPLL) {
            if (i==1) {
              maxTl=15;
            } else {
              maxTl=63;
            }
          }
          if (ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_ESFM) {
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
            if (ins->type==DIV_INS_ESFM) {
              CENTER_TEXT_20(ESFM_SHORT_NAME(ESFM_DELAY));
              ImGui::TextUnformatted(ESFM_SHORT_NAME(ESFM_DELAY));
              TOOLTIP_TEXT(ESFM_LONG_NAME(ESFM_DELAY));
            } else {
              CENTER_TEXT_20(FM_SHORT_NAME(FM_AR));
              ImGui::TextUnformatted(FM_SHORT_NAME(FM_AR));
              TOOLTIP_TEXT(FM_NAME(FM_AR));
            }
            ImGui::TableNextColumn();
            if (ins->type==DIV_INS_FM) {
              ImGui::Text(_("SSG-EG"));
            } else if (ins->type!=DIV_INS_OPM) {
              ImGui::Text(_("Waveform"));
            }
            ImGui::TableNextColumn();
            ImGui::Text(_("Envelope"));
            ImGui::TableNextColumn();

            // A/D/S/R
            ImGui::TableNextColumn();

            if (ins->type==DIV_INS_ESFM) {
              opE.delay&=7;
              P(CWVSliderScalar("##DELAY",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&opE.delay,&_ZERO,&_SEVEN)); rightClickable
              ImGui::SameLine();
            }

            op.ar&=maxArDr;
            float textX_AR=ImGui::GetCursorPosX();
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

            if (settings.susPosition>0) {
              ImGui::SameLine();
              op.sl&=15;
              textX_SL=ImGui::GetCursorPosX();
              P(CWVSliderScalar("##SL",ImVec2(20.0f*dpiScale,sliderHeight),ImGuiDataType_U8,&op.sl,&_FIFTEEN,&_ZERO)); rightClickable
            }

            ImVec2 prevCurPos=ImGui::GetCursorPos();

            // labels
            if (ins->type==DIV_INS_ESFM) {
              ImGui::SetCursorPos(ImVec2(textX_AR,textY));
              CENTER_TEXT_20(FM_SHORT_NAME(FM_AR));
              ImGui::TextUnformatted(FM_SHORT_NAME(FM_AR));
              TOOLTIP_TEXT(FM_NAME(FM_AR));
            }

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
            ImGui::Dummy(ImVec2(0,0));
            
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
                if (CWSliderScalar("##SSG",ImGuiDataType_U8,&ssgEnv,&_ZERO,&_SEVEN,_(ssgEnvTypes[ssgEnv]))) { PARAMETER
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

                // OPP TL ramp
                bool displayTLRamp=false;
                if (ins->type==DIV_INS_OPM) {
                  displayTLRamp=false;
                  for (int i=0; i<e->song.systemLen; i++) {
                    if (e->song.system[i]==DIV_SYSTEM_YM2151) {
                      // test for YM2164 (OPP)
                      if (e->song.systemFlags[i].getInt("chipType",0)==1) {
                        displayTLRamp=true;
                        break;
                      }
                    }
                  }
                }
                if (displayTLRamp) {
                  if (ImGui::Checkbox(_("TL Ramp##TLRamp"),&ksrOn)) { PARAMETER
                    op.ksr=ksrOn;
                  }
                }
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
                drawWaveform(op.ws&wsMax,ins->type==DIV_INS_OPZ,ImVec2(waveWidth,waveHeight));
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                P(CWSliderScalar("##WS",ImGuiDataType_U8,&op.ws,&_ZERO,&wsMax,(ins->type==DIV_INS_OPZ)?opzWaveforms[op.ws&wsMax]:(settings.oplStandardWaveNames?oplWaveformsStandard[op.ws&wsMax]:oplWaveforms[op.ws&wsMax]))); rightClickable

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

                pushWarningColor(ins->type==DIV_INS_OPL_DRUMS && i==0);          
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_MULT));
                P(CWSliderScalar("##MULT",ImGuiDataType_U8,&op.mult,&_ZERO,&_FIFTEEN,tempID)); rightClickable
                if (ins->type==DIV_INS_OPL_DRUMS && i==0) {
                  if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("%s",_("Snare's multiplier is determined by HiHat's."));
                  }
                }
                popWarningColor();

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
                drawWaveform(op.ws&wsMax,ins->type==DIV_INS_OPZ,ImVec2(waveWidth,waveHeight));
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                P(CWSliderScalar("##WS",ImGuiDataType_U8,&op.ws,&_ZERO,&wsMax,(ins->type==DIV_INS_OPZ)?opzWaveforms[op.ws&wsMax]:(settings.oplStandardWaveNames?oplWaveformsStandard[op.ws&wsMax]:oplWaveforms[op.ws&wsMax]))); rightClickable

                // params
                ImGui::Separator();
                if (egtOn) {
                  if (!op.sus) {
                    int block=op.dt;
                    int freqNum=(op.mult<<4)|(op.dvb&15);
                    ImGui::Text(_("Block"));
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    ImVec2 cursorAlign=ImGui::GetCursorPos();
                    if (ImGui::InputInt("##Block",&block,1,1)) {
                      if (block<0) block=0;
                      if (block>7) block=7;
                      op.dt=block;
                    }
                    
                    ImGui::Text(_("Freq"));
                    ImGui::SameLine();
                    ImGui::SetCursorPos(ImVec2(cursorAlign.x,ImGui::GetCursorPosY()));
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    if (ImGui::InputInt("##FreqNum",&freqNum,1,16)) {
                      if (freqNum<0) freqNum=0;
                      if (freqNum>255) freqNum=255;
                      op.mult=freqNum>>4;
                      op.dvb=freqNum&15;
                    }
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
                  ImGui::SetTooltip(_("Only on YM2151 and YM2414 (OPM and OPZ)"));
                }

                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_RS));
                P(CWSliderScalar("##RS",ImGuiDataType_U8,&op.rs,&_ZERO,&_THREE,tempID)); rightClickable
                break;
              }
              case DIV_INS_ESFM:
                // waveform
                drawWaveform(op.ws&wsMax,ins->type==DIV_INS_OPZ,ImVec2(waveWidth,waveHeight));
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                P(CWSliderScalar("##WS",ImGuiDataType_U8,&op.ws,&_ZERO,&wsMax,(ins->type==DIV_INS_OPZ)?opzWaveforms[op.ws&wsMax]:(settings.oplStandardWaveNames?oplWaveformsStandard[op.ws&wsMax]:oplWaveforms[op.ws&wsMax]))); rightClickable

                // params
                ImGui::Separator();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_MULT));
                P(CWSliderScalar("##MULT",ImGuiDataType_U8,&op.mult,&_ZERO,&_FIFTEEN,tempID)); rightClickable

                if (opE.fixed) {
                  int block=(opE.ct>>2)&7;
                  int freqNum=((opE.ct&3)<<8)|((unsigned char)opE.dt);
                  ImGui::Text(_("Blk"));
                  if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip(_("Block"));
                  }
                  ImGui::SameLine();
                  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                  //ImVec2 cursorAlign=ImGui::GetCursorPos();
                  if (ImGui::InputInt("##Block",&block,1,1)) {
                    if (block<0) block=0;
                    if (block>7) block=7;
                    opE.ct=(opE.ct&(~(7<<2)))|(block<<2);
                  }

                  ImGui::Text(_("F"));
                  if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip(_("Frequency (F-Num)"));
                  }
                  ImGui::SameLine();
                  //ImGui::SetCursorPos(ImVec2(cursorAlign.x,ImGui::GetCursorPosY()));
                  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                  if (ImGui::InputInt("##FreqNum",&freqNum,1,16)) {
                    if (freqNum<0) freqNum=0;
                    if (freqNum>1023) freqNum=1023;
                    opE.dt=freqNum&0xff;
                    opE.ct=(opE.ct&(~3))|(freqNum>>8);
                  }
                } else {
                  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                  snprintf(tempID,1024,"%s: %%d",ESFM_NAME(ESFM_CT));
                  P(CWSliderScalar("##CT",ImGuiDataType_S8,&opE.ct,&_MINUS_TWENTY_FOUR,&_TWENTY_FOUR,tempID)); rightClickable

                  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                  snprintf(tempID,1024,"%s: %%d",ESFM_NAME(ESFM_DT));
                  P(CWSliderScalar("##DT",ImGuiDataType_S8,&opE.dt,&_MINUS_ONE_HUNDRED_TWENTY_EIGHT,&_ONE_HUNDRED_TWENTY_SEVEN,tempID)); rightClickable
                }

                if (ImGui::BeginTable("panCheckboxes",2,ImGuiTableFlags_SizingStretchSame)) {
                  ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.0f);
                  ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0f);

                  float yPosOutsideTablePadding=ImGui::GetCursorPosY();
                  bool leftOn=opE.left;
                  bool rightOn=opE.right;
                  ImGui::TableNextRow();
                  ImGui::TableNextColumn();
                  ImGui::SetCursorPosY(yPosOutsideTablePadding);
                  if (ImGui::Checkbox(ESFM_SHORT_NAME(ESFM_LEFT),&leftOn)) { PARAMETER
                    opE.left=leftOn;
                  }
                  ImGui::TableNextColumn();
                  ImGui::SetCursorPosY(yPosOutsideTablePadding);
                  if (ImGui::Checkbox(ESFM_SHORT_NAME(ESFM_RIGHT),&rightOn)) { PARAMETER
                    opE.right=rightOn;
                  }
                  ImGui::EndTable();
                }
                break;
              default:
                break;
            }

            ImGui::TableNextColumn();
            float envHeight=sliderHeight;//-ImGui::GetStyle().ItemSpacing.y*2.0f;
            if (ins->type==DIV_INS_OPZ) {
              envHeight-=ImGui::GetFrameHeightWithSpacing()*2.0f;
            }
            if (ins->type==DIV_INS_ESFM) {
              envHeight-=ImGui::GetFrameHeightWithSpacing()*3.0f;
            }
            drawFMEnv(op.tl&maxTl,op.ar&maxArDr,op.dr&maxArDr,(ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_OPLL || ins->type==DIV_INS_ESFM)?((op.rr&15)*2):op.d2r&31,op.rr&15,op.sl&15,op.sus,op.ssgEnv&8,fmOrigin.alg,maxTl,maxArDr,15,ImVec2(ImGui::GetContentRegionAvail().x,envHeight),ins->type);

            if (ins->type==DIV_INS_OPZ) {
              ImGui::Separator();
              if (ImGui::BeginTable("FMParamsInnerOPZ",2)) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                if (!egtOn) {
                  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                  snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_FINE));
                  P(CWSliderScalar("##FINE",ImGuiDataType_U8,&op.dvb,&_ZERO,&_FIFTEEN,tempID)); rightClickable
                } else {
                  bool susOn=op.sus;
                  pushWarningColor(susOn && !e->song.compatFlags.linearPitch);
                  if (ImGui::Checkbox(_("Pitch control"),&susOn)) { PARAMETER
                    op.sus=susOn;
                    // HACK: reset zoom and scroll in fixed pitch macros so that they draw correctly
                    ins->temp.vZoom[DIV_MACRO_OP_SSG+(i<<5)]=-1;
                    ins->temp.vZoom[DIV_MACRO_OP_SUS+(i<<5)]=-1;
                  }
                  popWarningColor();
                  if (ImGui::IsItemHovered()) {
                    if (susOn && !e->song.compatFlags.linearPitch) {
                      ImGui::SetTooltip(_("only works on linear pitch! go to Compatibility Flags > Pitch/Playback and set Pitch linearity to Full."));
                    } else {
                      ImGui::SetTooltip(_("use op's arpeggio and pitch macros control instead of block/f-num macros"));
                    }
                  }
                }

                ImGui::TableNextColumn();
                bool amOn=op.am;
                if (ImGui::Checkbox(FM_NAME(FM_AM),&amOn)) { PARAMETER
                  op.am=amOn;
                }
                ImGui::SameLine();
                if (ImGui::Checkbox(_("Fixed"),&egtOn)) { PARAMETER
                  op.egt=egtOn;
                }

                // this chip has too many parameters. I can't think of a better placement for the tremolo setting.
                float oneTinySlider=(ImGui::GetContentRegionAvail().x-ImGui::GetStyle().ItemSpacing.x)*0.5f;
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(oneTinySlider);
                snprintf(tempID,1024,"%s: %%d",FM_SHORT_NAME(FM_EGSHIFT));
                P(CWSliderScalar("##EGShift",ImGuiDataType_U8,&op.ksl,&_ZERO,&_THREE,tempID)); rightClickable
                if (ImGui::IsItemHovered()) {
                  ImGui::SetTooltip("%s",FM_NAME(FM_EGSHIFT));
                }

                ImGui::SameLine();
                ImGui::SetNextItemWidth(oneTinySlider);
                snprintf(tempID,1024,"%s: %%d",FM_SHORT_NAME(FM_TS));
                P(CWSliderScalar("##TS",ImGuiDataType_U8,&op.ssgEnv,&_ZERO,&_THREE,tempID)); rightClickable
                if (ImGui::IsItemHovered()) {
                  ImGui::SetTooltip("%s",FM_NAME(FM_TS));
                }

                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_REV));
                P(CWSliderScalar("##REV",ImGuiDataType_U8,&op.dam,&_ZERO,&_SEVEN,tempID)); rightClickable

                ImGui::TableNextColumn();


                ImGui::EndTable();
              }
            }

            if (ins->type==DIV_INS_ESFM) {
              ImGui::Separator();
              if (ImGui::BeginTable("FMParamsInnerESFM",2)) {
                ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.64f);
                ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.36f);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                snprintf(tempID,1024,"%s: %%d",FM_NAME(FM_KSL));
                int ksl=kslMap[op.ksl&3];
                if (CWSliderInt("##KSL",&ksl,0,3,tempID)) {
                  op.ksl=kslMap[ksl&3];
                  PARAMETER;
                } rightClickable

                bool amOn=op.am;
                bool fixedOn=opE.fixed;
                ImGui::TableNextColumn();
                if (ImGui::Checkbox(FM_SHORT_NAME(FM_KSR),&ksrOn)) { PARAMETER
                  op.ksr=ksrOn;
                }
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                if (ImGui::BeginTable("vibAmCheckboxes",2)) {
                  ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.0f);
                  ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0f);

                  float yPosOutsideTablePadding=ImGui::GetCursorPosY();
                  ImGui::TableNextRow();
                  ImGui::TableNextColumn();
                  ImGui::SetCursorPosY(yPosOutsideTablePadding);
                  if (ImGui::Checkbox(FM_SHORT_NAME(FM_VIB),&vibOn)) { PARAMETER
                    op.vib=vibOn;
                  }
                  ImGui::TableNextColumn();
                  ImGui::SetCursorPosY(yPosOutsideTablePadding);
                  if (ImGui::Checkbox(FM_SHORT_NAME(FM_AM),&amOn)) { PARAMETER
                    op.am=amOn;
                  }

                  bool damOn=op.dam;
                  bool dvbOn=op.dvb;
                  ImGui::TableNextRow();
                  ImGui::TableNextColumn();
                  if (ImGui::Checkbox(FM_SHORT_NAME(FM_DVB),&dvbOn)) { PARAMETER
                    op.dvb=dvbOn;
                  }
                  ImGui::TableNextColumn();
                  if (ImGui::Checkbox(FM_SHORT_NAME(FM_DAM),&damOn)) { PARAMETER
                    op.dam=damOn;
                  }
                  ImGui::EndTable();
                }
                ImGui::TableNextColumn();
                if (ImGui::Checkbox(FM_SHORT_NAME(FM_SUS),&susOn)) { PARAMETER
                  op.sus=susOn;
                }
                if (ImGui::Checkbox(ESFM_NAME(ESFM_FIXED),&fixedOn)) { PARAMETER
                  opE.fixed=fixedOn;
                  // HACK: reset zoom and scroll in fixed pitch macros so that they draw correctly
                  ins->temp.vZoom[DIV_MACRO_OP_SSG+(i<<5)]=-1;
                  ins->temp.vZoom[DIV_MACRO_OP_DT+(i<<5)]=-1;
                }

                ImGui::EndTable();
              }
            }

            ImGui::TableNextColumn();
            op.tl&=maxTl;
            float tlSliderWidth=(ins->type==DIV_INS_ESFM)?20.0f*dpiScale:ImGui::GetFrameHeight();
            float tlSliderHeight=sliderHeight-((ins->type==DIV_INS_FM || ins->type==DIV_INS_OPM || ins->type==DIV_INS_OPZ)?(ImGui::GetFrameHeightWithSpacing()+ImGui::CalcTextSize(FM_SHORT_NAME(FM_AM)).y+ImGui::GetStyle().ItemSpacing.y):0.0f);
            float textX_tl=ImGui::GetCursorPosX();
            P(CWVSliderScalar("##TL",ImVec2(tlSliderWidth,tlSliderHeight),ImGuiDataType_U8,&op.tl,&maxTl,&_ZERO)); rightClickable

            if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPM) {
              CENTER_TEXT(FM_SHORT_NAME(FM_AM));
              ImGui::TextUnformatted(FM_SHORT_NAME(FM_AM));
              TOOLTIP_TEXT(FM_NAME(FM_AM));
              bool amOn=op.am;
              if (ImGui::Checkbox("##AM",&amOn)) { PARAMETER
                op.am=amOn;
              }
            } else if (ins->type==DIV_INS_OPZ) {
              CENTER_TEXT(FM_SHORT_NAME(FM_TLRAMP));
              ImGui::TextUnformatted(FM_SHORT_NAME(FM_TLRAMP));
              TOOLTIP_TEXT(FM_NAME(FM_TLRAMP));
              bool ksrOn=op.ksr;
              if (ImGui::Checkbox("##TLRamp",&ksrOn)) { PARAMETER
                op.ksr=ksrOn;
              }
            }

            if (ins->type==DIV_INS_ESFM) {
              ImGui::SameLine();
              float textX_outLvl=ImGui::GetCursorPosX();
              P(CWVSliderScalar("##OUTLVL",ImVec2(tlSliderWidth,tlSliderHeight),ImGuiDataType_U8,&opE.outLvl,&_ZERO,&_SEVEN)); rightClickable

              ImGui::SameLine();
              float textX_modIn=ImGui::GetCursorPosX();
              P(CWVSliderScalar("##MODIN",ImVec2(tlSliderWidth,tlSliderHeight),ImGuiDataType_U8,&opE.modIn,&_ZERO,&_SEVEN)); rightClickable

              prevCurPos=ImGui::GetCursorPos();
              ImGui::SetCursorPos(ImVec2(textX_tl,textY));
              CENTER_TEXT_20(FM_SHORT_NAME(FM_TL));
              ImGui::TextUnformatted(FM_SHORT_NAME(FM_TL));
              TOOLTIP_TEXT(FM_NAME(FM_TL));

              ImGui::SetCursorPos(ImVec2(textX_outLvl,textY));
              CENTER_TEXT_20(ESFM_SHORT_NAME(ESFM_OUTLVL));
              ImGui::TextUnformatted(ESFM_SHORT_NAME(ESFM_OUTLVL));
              TOOLTIP_TEXT(ESFM_LONG_NAME(ESFM_OUTLVL));

              ImGui::SetCursorPos(ImVec2(textX_modIn,textY));
              CENTER_TEXT_20(ESFM_SHORT_NAME(ESFM_MODIN));
              ImGui::TextUnformatted(ESFM_SHORT_NAME(ESFM_MODIN));
              TOOLTIP_TEXT(ESFM_LONG_NAME(ESFM_MODIN));

              ImGui::SetCursorPos(prevCurPos);
            } else {
              prevCurPos=ImGui::GetCursorPos();
              ImGui::SetCursorPos(ImVec2(textX_tl,textY));
              CENTER_TEXT(FM_SHORT_NAME(FM_TL));
              ImGui::TextUnformatted(FM_SHORT_NAME(FM_TL));
              TOOLTIP_TEXT(FM_NAME(FM_TL));

              ImGui::SetCursorPos(prevCurPos);
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
    } else { // classic...... erm COMPACT
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
          DivInstrumentFM::Operator& op=fmOrigin.op[(opCount==4 && ins->type!=DIV_INS_OPL_DRUMS && ins->type!=DIV_INS_ESFM)?opOrder[i]:i];
          DivInstrumentESFM::Operator& opE=ins->esfm.op[i];
          if ((settings.fmLayout!=3 && ((i+1)&1)) || i==0 || settings.fmLayout==2) ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::Separator();
          ImGui::PushID(fmt::sprintf("op%d",i).c_str());

          // push colors
          if (settings.separateFMColors) {
            bool mod=true;
            if (ins->type==DIV_INS_OPL_DRUMS) {
              mod=false;
            } else if (ins->type==DIV_INS_ESFM) {
              // this is the same as the KVS heuristic in platform/esfm.h
              if (opE.outLvl==7) {
                mod=false;
              } else if (opE.outLvl>0) {
                if (i==3) {
                  mod=false;
                } else {
                  DivInstrumentESFM::Operator& opENext=ins->esfm.op[i+1];
                  if (opENext.modIn==0) {
                    mod=false;
                  } else if ((opE.outLvl-opENext.modIn)>=2) {
                    mod=false;
                  }
                }
              }
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
              opNameLabel=_("Envelope 2 (kick only)");
            } else {
              opNameLabel=_("Envelope");
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

          // OPP TL ramp
          bool displayTLRamp=false;
          if (ins->type==DIV_INS_OPZ) {
            displayTLRamp=true;
          } else if (ins->type==DIV_INS_OPM) {
            displayTLRamp=false;
            for (int i=0; i<e->song.systemLen; i++) {
              if (e->song.system[i]==DIV_SYSTEM_YM2151) {
                // test for YM2164 (OPP)
                if (e->song.systemFlags[i].getInt("chipType",0)==1) {
                  displayTLRamp=true;
                  break;
                }
              }
            }
          }
          if (displayTLRamp) {
            ImGui::SameLine();
            bool ksrOn=op.ksr; // why is this not defined?!?!?!?
            if (ImGui::Checkbox(_("TL Ramp##TLRamp"),&ksrOn)) { PARAMETER
              op.ksr=ksrOn;
            }
          }

          int maxTl=127;
          if (ins->type==DIV_INS_OPLL) {
            if (i==1) {
              maxTl=15;
            } else {
              maxTl=63;
            }
          }
          if (ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_ESFM) {
            maxTl=63;
          }
          int maxArDr=(ins->type==DIV_INS_FM || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM)?31:15;

          bool ssgOn=op.ssgEnv&8;
          bool ksrOn=op.ksr;
          bool vibOn=op.vib;
          bool susOn=op.sus; // don't you make fun of this one
          unsigned char ssgEnv=op.ssgEnv&7;
          if (ins->type!=DIV_INS_OPL && ins->type!=DIV_INS_OPL_DRUMS && ins->type!=DIV_INS_OPZ && ins->type!=DIV_INS_OPM && ins->type!=DIV_INS_ESFM) {
            ImGui::SameLine();
            if (ImGui::Checkbox((ins->type==DIV_INS_OPLL)?FM_NAME(FM_EGS):_("SSG On"),&ssgOn)) { PARAMETER
              op.ssgEnv=(op.ssgEnv&7)|(ssgOn<<3);
            }
          }

          if (ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_ESFM) {
            ImGui::SameLine();
            if (ImGui::Checkbox(FM_NAME(FM_SUS),&susOn)) { PARAMETER
              op.sus=susOn;
            }
          }

          if (ins->type==DIV_INS_OPZ) {
            ImGui::SameLine();
            bool fixedOn=op.egt;
            if (ImGui::Checkbox(_("Fixed"),&fixedOn)) { PARAMETER
              op.egt=fixedOn;
            }
            bool susOn=op.sus;
            if (fixedOn) {
              ImGui::SameLine();
              pushWarningColor(susOn && !e->song.compatFlags.linearPitch);
              if (ImGui::Checkbox(_("Pitch control"),&susOn)) { PARAMETER
                op.sus=susOn;
                // HACK: reset zoom and scroll in fixed pitch macros so that they draw correctly
                ins->temp.vZoom[DIV_MACRO_OP_SSG+(i<<5)]=-1;
                ins->temp.vZoom[DIV_MACRO_OP_SUS+(i<<5)]=-1;
              }
              popWarningColor();
              if (ImGui::IsItemHovered()) {
                if (susOn && !e->song.compatFlags.linearPitch) {
                  ImGui::SetTooltip(_("only works on linear pitch! go to Compatibility Flags > Pitch/Playback and set Pitch linearity to Full."));
                } else {
                  ImGui::SetTooltip(_("use op's arpeggio and pitch macros control instead of block/f-num macros"));
                }
              }
            }
          }

          //52.0 controls vert scaling; default 96
          drawFMEnv(op.tl&maxTl,op.ar&maxArDr,op.dr&maxArDr,(ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_OPLL || ins->type==DIV_INS_ESFM)?((op.rr&15)*2):op.d2r&31,op.rr&15,op.sl&15,op.sus,op.ssgEnv&8,fmOrigin.alg,maxTl,maxArDr,15,ImVec2(ImGui::GetContentRegionAvail().x,52.0*dpiScale),ins->type);
          //P(CWSliderScalar(FM_NAME(FM_AR),ImGuiDataType_U8,&op.ar,&_ZERO,&_THIRTY_ONE)); rightClickable
          if (ImGui::BeginTable("opParams",2,ImGuiTableFlags_SizingStretchProp)) {
            ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.0); \
            ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed,0.0); \

            if (ins->type==DIV_INS_ESFM) {
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              opE.delay&=7;
              P(CWSliderScalar("##DELAY",ImGuiDataType_U8,&opE.delay,&_ZERO,&_SEVEN)); rightClickable
              ImGui::TableNextColumn();
              ImGui::Text("%s",ESFM_NAME(ESFM_DELAY));
            }

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

            if (settings.susPosition>0) {
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

              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              P(CWSliderScalar(FM_NAME(FM_TS),ImGuiDataType_U8,&op.ssgEnv,&_ZERO,&_THREE)); rightClickable
              ImGui::TableNextColumn();
              ImGui::Text("%s",FM_SHORT_NAME(FM_TS));
              if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s",FM_NAME(FM_TS));
              }
            }

            if (ins->type==DIV_INS_OPZ) {
              if (op.egt) {
                bool susOn=op.sus;
                if (!susOn) {
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
                  ImGui::Text(_("FreqNum"));
                }
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
              pushWarningColor(ins->type==DIV_INS_OPL_DRUMS && i==0);          
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              P(CWSliderScalar(FM_NAME(FM_MULT),ImGuiDataType_U8,&op.mult,&_ZERO,&_FIFTEEN)); rightClickable
              if (ins->type==DIV_INS_OPL_DRUMS && i==0) {
                if (ImGui::IsItemHovered()) {
                  ImGui::SetTooltip("%s",_("Snare's multiplier is determined by HiHat's."));
                }
              }
              popWarningColor();
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
                if (CWSliderScalar("##SSG",ImGuiDataType_U8,&ssgEnv,&_ZERO,&_SEVEN,_(ssgEnvTypes[ssgEnv]))) { PARAMETER
                  op.ssgEnv=(op.ssgEnv&8)|(ssgEnv&7);
                } rightClickable
                ImGui::TableNextColumn();
                ImGui::Text("%s",FM_NAME(FM_SSG));
              }
            }

            if (ins->type==DIV_INS_ESFM) {
              bool fixedOn=opE.fixed;
              if (fixedOn) {
                int block=(opE.ct>>2)&7;
                int freqNum=((opE.ct&3)<<8)|((unsigned char)opE.dt);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::InputInt("##Block",&block,1,1)) {
                  if (block<0) block=0;
                  if (block>7) block=7;
                  opE.ct=(opE.ct&(~(7<<2)))|(block<<2);
                }
                ImGui::TableNextColumn();
                ImGui::Text(_("Block"));
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::InputInt("##FreqNum",&freqNum,1,16)) {
                  if (freqNum<0) freqNum=0;
                  if (freqNum>1023) freqNum=1023;
                  opE.dt=freqNum&0xff;
                  opE.ct=(opE.ct&(~3))|(freqNum>>8);
                }
                ImGui::TableNextColumn();
                ImGui::Text(_("FreqNum"));
              } else {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                P(CWSliderScalar("##CT",ImGuiDataType_S8,&opE.ct,&_MINUS_TWENTY_FOUR,&_TWENTY_FOUR)); rightClickable
                ImGui::TableNextColumn();
                ImGui::Text("%s",ESFM_NAME(ESFM_CT));

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                P(CWSliderScalar("##DT",ImGuiDataType_S8,&opE.dt,&_MINUS_ONE_HUNDRED_TWENTY_EIGHT,&_ONE_HUNDRED_TWENTY_SEVEN)); rightClickable
                ImGui::TableNextColumn();
                ImGui::Text("%s",ESFM_NAME(ESFM_DT));
              }

            }

            if (ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_ESFM) {
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              P(CWSliderScalar("##WS",ImGuiDataType_U8,&op.ws,&_ZERO,&wsMax,(ins->type==DIV_INS_OPZ)?opzWaveforms[op.ws&wsMax]:(settings.oplStandardWaveNames?oplWaveformsStandard[op.ws&wsMax]:oplWaveforms[op.ws&wsMax]))); rightClickable
              ImGui::TableNextColumn();
              ImGui::Text("%s",FM_NAME(FM_WS));
            }

            if (ins->type==DIV_INS_ESFM) {
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::Separator();
              ImGui::TableNextColumn();
              ImGui::Separator();

              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              P(CWSliderScalar("##OUTLVL",ImGuiDataType_U8,&opE.outLvl,&_ZERO,&_SEVEN)); rightClickable
              ImGui::TableNextColumn();
              ImGui::Text("%s",ESFM_NAME(ESFM_OUTLVL));

              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              P(CWSliderScalar("##MODIN",ImGuiDataType_U8,&opE.modIn,&_ZERO,&_SEVEN)); rightClickable
              ImGui::TableNextColumn();
              ImGui::Text("%s",ESFM_NAME(ESFM_MODIN));
            }

            ImGui::EndTable();
          }

          if (ins->type==DIV_INS_OPLL || ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_ESFM) {
            if (ImGui::Checkbox(FM_NAME(FM_VIB),&vibOn)) { PARAMETER
              op.vib=vibOn;
            }
            ImGui::SameLine();
            if (ImGui::Checkbox(FM_NAME(FM_KSR),&ksrOn)) { PARAMETER
              op.ksr=ksrOn;
            }
          }

          if (ins->type==DIV_INS_ESFM) {
            bool dvbOn=op.dvb;
            bool damOn=op.dam;
            bool leftOn=opE.left;
            bool rightOn=opE.right;
            bool fixedOn=opE.fixed;
            if (ImGui::Checkbox(FM_NAME(FM_DVB),&dvbOn)) { PARAMETER
              op.dvb=dvbOn;
            }
            ImGui::SameLine();
            if (ImGui::Checkbox(FM_NAME(FM_DAM),&damOn)) { PARAMETER
              op.dam=damOn;
            }
            if (ImGui::Checkbox(ESFM_NAME(ESFM_LEFT),&leftOn)) { PARAMETER
              opE.left=leftOn;
            }
            ImGui::SameLine();
            if (ImGui::Checkbox(ESFM_NAME(ESFM_RIGHT),&rightOn)) { PARAMETER
              opE.right=rightOn;
            }
            ImGui::SameLine();
            if (ImGui::Checkbox(ESFM_NAME(ESFM_FIXED),&fixedOn)) { PARAMETER
              opE.fixed=fixedOn;
              // HACK: reset zoom and scroll in fixed pitch macros so that they draw correctly
              ins->temp.vZoom[DIV_MACRO_OP_SSG+(i<<5)]=-1;
              ins->temp.vZoom[DIV_MACRO_OP_DT+(i<<5)]=-1;
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
}

