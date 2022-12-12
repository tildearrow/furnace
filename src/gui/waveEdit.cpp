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

#define _USE_MATH_DEFINES
#include "gui.h"
#include "plot_nolerp.h"
#include "IconsFontAwesome4.h"
#include "misc/cpp/imgui_stdlib.h"
#include <fmt/printf.h>
#include <math.h>
#include <imgui.h>

const char* waveGenBaseShapes[4]={
  "Sine",
  "Triangle",
  "Saw",
  "Pulse"
};

const char* waveInterpolations[4]={
  "None",
  "Linear",
  "Cosine",
  "Cubic"
};

const float multFactors[17]={
  M_PI,
  2*M_PI,
  4*M_PI,
  6*M_PI,
  8*M_PI,
  10*M_PI,
  12*M_PI,
  14*M_PI,
  16*M_PI,
  18*M_PI,
  20*M_PI,
  22*M_PI,
  24*M_PI,
  26*M_PI,
  28*M_PI,
  30*M_PI,
  32*M_PI,
};

void FurnaceGUI::doGenerateWave() {
  float finalResult[256];
  if (curWave<0 || curWave>=(int)e->song.wave.size()) return;

  DivWavetable* wave=e->song.wave[curWave];
  memset(finalResult,0,sizeof(float)*256);

  if (wave->len<2) return;

  if (waveGenFM) {
    float s0fb0=0;
    float s0fb1=0;
    float s1fb0=0;
    float s1fb1=0;
    float s2fb0=0;
    float s2fb1=0;
    float s3fb0=0;
    float s3fb1=0;
    for (int i=0; i<wave->len; i++) {
      float pos=(float)i/(float)wave->len;
      float s0=sin((pos+(waveGenFB[0]?((s0fb0+s0fb1)*pow(2.0f,waveGenFB[0]-8)):0.0f))*multFactors[waveGenMult[0]])*waveGenTL[0];
      s0fb0=s0fb1;
      s0fb1=s0;
      
      float s1=sin((pos+(waveGenFB[1]?((s1fb0+s1fb1)*pow(2.0f,waveGenFB[1]-8)):0.0f)+(waveGenFMCon1[0]?s0:0.0f))*multFactors[waveGenMult[1]])*waveGenTL[1];
      s1fb0=s1fb1;
      s1fb1=s1;

      float s2=sin((pos+(waveGenFB[2]?((s2fb0+s2fb1)*pow(2.0f,waveGenFB[2]-8)):0.0f)+(waveGenFMCon1[1]?s0:0.0f)+(waveGenFMCon2[0]?s1:0.0f))*multFactors[waveGenMult[2]])*waveGenTL[2];
      s2fb0=s2fb1;
      s2fb1=s2;

      float s3=sin((pos+(waveGenFB[3]?((s3fb0+s3fb1)*pow(2.0f,waveGenFB[3]-8)):0.0f)+(waveGenFMCon1[2]?s0:0.0f)+(waveGenFMCon2[1]?s1:0.0f)+(waveGenFMCon3[0]?s2:0.0f))*multFactors[waveGenMult[3]])*waveGenTL[3];
      s3fb0=s3fb1;
      s3fb1=s3;

      if (waveGenFMCon1[3]) finalResult[i]+=s0;
      if (waveGenFMCon2[2]) finalResult[i]+=s1;
      if (waveGenFMCon3[1]) finalResult[i]+=s2;
      finalResult[i]+=s3;
    }
  } else {
    switch (waveGenBaseShape) {
      case 0: // sine
        for (int i=0; i<wave->len; i++) {
          for (int j=0; j<16; j++) {
            float pos=fmod((waveGenPhase[j]*wave->len)+(i*(j+1)),wave->len);
            float partial=sin((0.5+pos)*2.0*M_PI/(double)wave->len);
            partial=pow(partial,waveGenPower);
            partial*=waveGenAmp[j];
            finalResult[i]+=partial;
          }
        }
        break;
      case 1: // triangle
        for (int i=0; i<wave->len; i++) {
          for (int j=0; j<16; j++) {
            float pos=fmod((waveGenPhase[j]*wave->len)+(i*(j+1)),wave->len);
            float partial=4.0*(0.5-fabs(0.5-(pos/(double)(wave->len-1))))-1.0;
            partial=pow(partial,waveGenPower);
            partial*=waveGenAmp[j];
            finalResult[i]+=partial;
          }
        }
        break;
      case 2: // saw
        for (int i=0; i<wave->len; i++) {
          for (int j=0; j<16; j++) {
            float pos=fmod((waveGenPhase[j]*wave->len)+(i*(j+1)),wave->len);
            float partial=((2*pos)/(double)(wave->len-1))-1.0;
            partial=pow(partial,waveGenPower);
            partial*=waveGenAmp[j];
            finalResult[i]+=partial;
          }
        }
        break;
      case 3: // pulse
        for (int i=0; i<wave->len; i++) {
          for (int j=0; j<16; j++) {
            float pos=fmod((waveGenPhase[j]*wave->len)+(i*(j+1)),wave->len);
            float partial=(pos>=(waveGenDuty*wave->len))?1:-1;
            partial=pow(partial,waveGenPower);
            partial*=waveGenAmp[j];
            finalResult[i]+=partial;
          }
        }
        break;
    }
  }

  for (int i=waveGenInvertPoint*wave->len; i<wave->len; i++) {
    finalResult[i]=-finalResult[i];
  }

  for (int i=0; i<wave->len; i++) {
    finalResult[i]=(1.0+finalResult[i])*0.5;
    if (finalResult[i]<0.0f) finalResult[i]=0.0f;
    if (finalResult[i]>1.0f) finalResult[i]=1.0f;
    wave->data[i]=round(finalResult[i]*wave->max);
  }

  e->notifyWaveChange(curWave);
  MARK_MODIFIED;
}

#define CENTER_TEXT(text) \
  ImGui::SetCursorPosX(ImGui::GetCursorPosX()+0.5*(ImGui::GetContentRegionAvail().x-ImGui::CalcTextSize(text).x));

void FurnaceGUI::drawWaveEdit() {
  if (nextWindow==GUI_WINDOW_WAVE_EDIT) {
    waveEditOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!waveEditOpen) return;
  float wavePreview[257];
  if (mobileUI) {
    patWindowPos=(portrait?ImVec2(0.0f,(mobileMenuPos*-0.65*canvasH)):ImVec2((0.16*canvasH)+0.5*canvasW*mobileMenuPos,0.0f));
    patWindowSize=(portrait?ImVec2(canvasW,canvasH-(0.16*canvasW)-(pianoOpen?(0.4*canvasW):0.0f)):ImVec2(canvasW-(0.16*canvasH),canvasH-(pianoOpen?(0.3*canvasH):0.0f)));
    ImGui::SetNextWindowPos(patWindowPos);
    ImGui::SetNextWindowSize(patWindowSize);
  } else {
    ImGui::SetNextWindowSizeConstraints(ImVec2(300.0f*dpiScale,300.0f*dpiScale),ImVec2(canvasW,canvasH));
  }
  if (ImGui::Begin("Wavetable Editor",&waveEditOpen,globalWinFlags|(settings.allowEditDocking?0:ImGuiWindowFlags_NoDocking))) {
    if (curWave<0 || curWave>=(int)e->song.wave.size()) {
      ImGui::SetCursorPosY(ImGui::GetCursorPosY()+(ImGui::GetContentRegionAvail().y-ImGui::GetFrameHeightWithSpacing()*2.0f)*0.5f);
      CENTER_TEXT("no wavetable selected");
      ImGui::Text("no wavetable selected");
      if (ImGui::BeginTable("noAssetCenter",3)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.5f);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.5f);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();

        if (e->song.wave.size()>0) {
          if (ImGui::BeginCombo("##WaveSelect","select one...")) {
            if (ImGui::BeginTable("WaveSelCombo",1,ImGuiTableFlags_ScrollY)) {
              actualWaveList();
              ImGui::EndTable();
            }
            ImGui::EndCombo();
          }
          ImGui::SameLine();
          ImGui::TextUnformatted("or");
          ImGui::SameLine();
        }
        if (ImGui::Button("Open")) {
          doAction(GUI_ACTION_WAVE_LIST_OPEN);
        }
        ImGui::SameLine();
        ImGui::TextUnformatted("or");
        ImGui::SameLine();
        if (ImGui::Button("Create New")) {
          doAction(GUI_ACTION_WAVE_LIST_ADD);
        }

        ImGui::TableNextColumn();
        ImGui::EndTable();
      }
    } else {
      DivWavetable* wave=e->song.wave[curWave];

      if (ImGui::BeginTable("WEProps",2)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(80.0f*dpiScale);
        if (ImGui::InputInt("##CurWave",&curWave,1,1)) {
          if (curWave<0) curWave=0;
          if (curWave>=(int)e->song.wave.size()) curWave=e->song.wave.size()-1;
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_FOLDER_OPEN "##WELoad")) {
          doAction(GUI_ACTION_WAVE_LIST_OPEN_REPLACE);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Open");
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_FLOPPY_O "##WESave")) {
          doAction(GUI_ACTION_WAVE_LIST_SAVE);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Save");
        }
        if (ImGui::BeginPopupContextItem("WaveSaveFormats",ImGuiMouseButton_Right)) {
          if (ImGui::MenuItem("save as .dmw...")) {
            doAction(GUI_ACTION_WAVE_LIST_SAVE_DMW);
          }
          if (ImGui::MenuItem("save raw...")) {
            doAction(GUI_ACTION_WAVE_LIST_SAVE_RAW);
          }
          ImGui::EndPopup();
        }
        ImGui::SameLine();

        if (ImGui::RadioButton("Steps",waveEditStyle==0)) {
          waveEditStyle=0;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Lines",waveEditStyle==1)) {
          waveEditStyle=1;
        }

        ImGui::TableNextColumn();
        ImGui::Text("Width");
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("use a width of:\n- any on Amiga/N163\n- 32 on Game Boy, PC Engine, SCC, Konami Bubble System, Namco WSG, Virtual Boy and WonderSwan\n- 64 on FDS\n- 128 on X1-010\nany other widths will be scaled during playback.");
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(96.0f*dpiScale);
        if (ImGui::InputInt("##_WTW",&wave->len,1,2)) {
          if (wave->len>256) wave->len=256;
          if (wave->len<1) wave->len=1;
          e->notifyWaveChange(curWave);
          if (wavePreviewOn) e->previewWave(curWave,wavePreviewNote);
          MARK_MODIFIED;
        }
        ImGui::SameLine();
        ImGui::Text("Height");
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("use a height of:\n- 15 for Game Boy, WonderSwan, Namco WSG, Konami Bubble System, X1-010 Envelope shape and N163\n- 31 for PC Engine\n- 63 for FDS and Virtual Boy\n- 255 for X1-010 and SCC\nany other heights will be scaled during playback.");
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(96.0f*dpiScale);
        if (ImGui::InputInt("##_WTH",&wave->max,1,2)) {
          if (wave->max>255) wave->max=255;
          if (wave->max<1) wave->max=1;
          e->notifyWaveChange(curWave);
          MARK_MODIFIED;
        }

        ImGui::SameLine();
        if (ImGui::Button(waveGenVisible?(ICON_FA_CHEVRON_RIGHT "##WEWaveGen"):(ICON_FA_CHEVRON_LEFT "##WEWaveGen"))) {
          waveGenVisible=!waveGenVisible;
        }

        ImGui::EndTable();
      }

      for (int i=0; i<wave->len; i++) {
        if (wave->data[i]>wave->max) wave->data[i]=wave->max;
        wavePreview[i]=wave->data[i];
        if (waveSigned && !waveHex) {
          wavePreview[i]-=(int)((wave->max+1)/2);
        }
      }
      if (wave->len>0) wavePreview[wave->len]=wave->data[wave->len-1];

      if (ImGui::BeginTable("WEWaveSection",waveGenVisible?2:1)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch);
        if (waveGenVisible) ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed,250.0f*dpiScale);
        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(0.0f,0.0f));

        ImVec2 contentRegion=ImGui::GetContentRegionAvail(); // wavetable graph size determined here
        contentRegion.y-=ImGui::GetFrameHeightWithSpacing()+ImGui::GetStyle().WindowPadding.y;
        if (waveEditStyle) {
          PlotNoLerp("##Waveform",wavePreview,wave->len+1,0,NULL,(waveSigned && !waveHex)?(-(int)((wave->max+1)/2)):0,(waveSigned && !waveHex)?((int)(wave->max/2)):wave->max,contentRegion);
        } else {
          PlotCustom("##Waveform",wavePreview,wave->len,0,NULL,(waveSigned && !waveHex)?(-(int)((wave->max+1)/2)):0,(waveSigned && !waveHex)?((int)(wave->max/2)):wave->max,contentRegion,sizeof(float),ImVec4(1.0f,1.0f,1.0f,1.0f),0,NULL,NULL,true);
        }
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

        if (waveGenVisible) {
          ImGui::TableNextColumn();

          if (ImGui::BeginTabBar("WaveGenOpt")) {
            if (ImGui::BeginTabItem("Shapes")) {
              waveGenFM=false;

              if (waveGenBaseShape<0) waveGenBaseShape=0;
              if (waveGenBaseShape>3) waveGenBaseShape=3;
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              if (CWSliderInt("##WGShape",&waveGenBaseShape,0,3,waveGenBaseShapes[waveGenBaseShape])) {
                if (waveGenBaseShape<0) waveGenBaseShape=0;
                if (waveGenBaseShape>3) waveGenBaseShape=3;
                doGenerateWave();
              }

              if (ImGui::BeginTable("WGShapeProps",2)) {
                ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Duty");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (CWSliderFloat("##WGDuty",&waveGenDuty,0.0f,1.0f)) {
                  doGenerateWave();
                }

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Exponent");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (CWSliderInt("##WGExp",&waveGenPower,1,8)) {
                  doGenerateWave();
                }

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("XOR Point");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (CWSliderFloat("##WGXOR",&waveGenInvertPoint,0.0f,1.0f)) {
                  doGenerateWave();
                }

                ImGui::EndTable();
              }

              if (ImGui::TreeNode("Amplitude/Phase")) {
                if (ImGui::BeginTable("WGShapeProps",3)) {
                  ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
                  ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.6f);
                  ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.4f);

                  for (int i=0; i<16; i++) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%d",i+1);
                    ImGui::TableNextColumn();
                    ImGui::PushID(140+i);
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    if (CWSliderFloat("##WGAmp",&waveGenAmp[i],-1.0f,1.0f)) {
                      doGenerateWave();
                    }
                    if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) {
                      waveGenAmp[i]=0.0f;
                      doGenerateWave();
                    }
                    ImGui::PopID();
                    ImGui::TableNextColumn();
                    ImGui::PushID(140+i);
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    if (CWSliderFloat("##WGPhase",&waveGenPhase[i],0.0f,1.0f)) {
                      doGenerateWave();
                    }
                    if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) {
                      waveGenPhase[i]=0.0f;
                      doGenerateWave();
                    }
                    ImGui::PopID();
                  }

                  ImGui::EndTable();
                }
                ImGui::TreePop();
              }
              ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("FM")) {
              waveGenFM=true;

              if (ImGui::BeginTable("WGFMProps",4)) {
                ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed,ImGui::CalcTextSize("Op").x);
                ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.5);
                ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.25);
                ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthStretch,0.25);

                ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
                ImGui::TableNextColumn();
                ImGui::Text("Op");
                ImGui::TableNextColumn();
                ImGui::Text("Level");
                ImGui::TableNextColumn();
                ImGui::Text("Mult");
                ImGui::TableNextColumn();
                ImGui::Text("FB");

                for (int i=0; i<4; i++) {
                  ImGui::TableNextRow();
                  ImGui::TableNextColumn();
                  ImGui::Text("%d",i+1);

                  ImGui::TableNextColumn();
                  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                  ImGui::PushID(i);
                  if (CWSliderFloat("##WGTL",&waveGenTL[i],0.0f,1.0f)) {
                    doGenerateWave();
                  }
                  ImGui::PopID();

                  ImGui::TableNextColumn();
                  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                  ImGui::PushID(i);
                  if (CWSliderInt("##WGMULT",&waveGenMult[i],1,16)) {
                    doGenerateWave();
                  }
                  ImGui::PopID();

                  ImGui::TableNextColumn();
                  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                  ImGui::PushID(i);
                  if (CWSliderInt("##WGFB",&waveGenFB[i],0,7)) {
                    doGenerateWave();
                  }
                  ImGui::PopID();
                }

                ImGui::EndTable();
              }

              CENTER_TEXT("Connection Diagram");
              ImGui::Text("Connection Diagram");

              if (ImGui::BeginTable("WGFMCon",5)) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text(">>");
                ImGui::TableNextColumn();
                ImGui::Text("2");
                ImGui::TableNextColumn();
                ImGui::Text("3");
                ImGui::TableNextColumn();
                ImGui::Text("4");
                ImGui::TableNextColumn();
                ImGui::Text("Out");

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("1");
                ImGui::TableNextColumn();
                if (ImGui::Checkbox("##Con12",&waveGenFMCon1[0])) {
                  doGenerateWave();
                }
                ImGui::TableNextColumn();
                if (ImGui::Checkbox("##Con13",&waveGenFMCon1[1])) {
                  doGenerateWave();
                }
                ImGui::TableNextColumn();
                if (ImGui::Checkbox("##Con14",&waveGenFMCon1[2])) {
                  doGenerateWave();
                }
                ImGui::TableNextColumn();
                if (ImGui::Checkbox("##Con1O",&waveGenFMCon1[3])) {
                  doGenerateWave();
                }

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("2");
                ImGui::TableNextColumn();
                // blank
                ImGui::TableNextColumn();
                if (ImGui::Checkbox("##Con23",&waveGenFMCon2[0])) {
                  doGenerateWave();
                }
                ImGui::TableNextColumn();
                if (ImGui::Checkbox("##Con24",&waveGenFMCon2[1])) {
                  doGenerateWave();
                }
                ImGui::TableNextColumn();
                if (ImGui::Checkbox("##Con2O",&waveGenFMCon2[2])) {
                  doGenerateWave();
                }

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("3");
                ImGui::TableNextColumn();
                // blank
                ImGui::TableNextColumn();
                // blank
                ImGui::TableNextColumn();
                if (ImGui::Checkbox("##Con34",&waveGenFMCon3[0])) {
                  doGenerateWave();
                }
                ImGui::TableNextColumn();
                if (ImGui::Checkbox("##Con3O",&waveGenFMCon3[1])) {
                  doGenerateWave();
                }

                ImGui::EndTable();
              }
              ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("WaveTools")) {
              if (ImGui::BeginTable("WGParamItems",2)) {
                ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::InputInt("##WGScaleX",&waveGenScaleX,1,16)) {
                  if (waveGenScaleX<2) waveGenScaleX=2;
                  if (waveGenScaleX>256) waveGenScaleX=256;
                }
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (CWSliderInt("##WGInterpolation",&waveInterpolation,0,3,waveInterpolations[waveInterpolation])) {
                  if (waveInterpolation<0) waveInterpolation=0;
                  if (waveInterpolation>3) waveInterpolation=3;
                }
                ImGui::TableNextColumn();
                if (ImGui::Button("Scale X")) {
                  if (waveGenScaleX>0 && wave->len!=waveGenScaleX) e->lockEngine([this,wave]() {
                    int origData[256];
                    // Copy original wave to temp buffer
                    // If longer than 256 samples, return
                    if (wave->len>256) {
                      showError("wavetable longer than 256 samples!");
                      return;
                    }
                    memcpy(origData,wave->data,wave->len*sizeof(int));

                    float t=0; // Index used into `origData`

                    for (int i=0; i<waveGenScaleX; i++, t+=(float)wave->len/waveGenScaleX) {
                      switch (waveInterpolation) {
                        case 0: {
                          wave->data[i]=origData[i*wave->len/waveGenScaleX];
                          break;
                        }
                        case 1: { // Linear
                          int idx=t; // Implicitly floors `t`
                          int s0=origData[(idx)%wave->len];
                          int s1=origData[(idx+1)%wave->len];
                          double mu=(t-idx);
                          wave->data[i]=s0+mu*s1-(mu*s0);
                          break;
                        }
                        case 2: { // Cosine
                          int idx=t; // Implicitly floors `t`
                          int s0=origData[(idx)%wave->len];
                          int s1=origData[(idx+1)%wave->len];
                          double mu=(t-idx);
                          double muCos=(1-cos(mu*M_PI))/2;
                          wave->data[i]=s0+muCos*s1-(muCos*s0);
                          break;
                        }
                        case 3: { // Cubic Spline
                          int idx=t; // Implicitly floors `t`
                          int s0=origData[((idx-1%wave->len+wave->len)%wave->len)];
                          int s1=origData[(idx)%wave->len];
                          int s2=origData[(idx+1)%wave->len];
                          int s3=origData[(idx+2)%wave->len];
                          double mu=(t-idx);
                          double mu2=mu*mu;
                          double a0=-0.5*s0+1.5*s1-1.5*s2+0.5*s3;
                          double a1=s0-2.5*s1+2*s2-0.5*s3;
                          double a2=-0.5*s0+0.5*s2;
                          double a3=s1;
                          wave->data[i]=(a0*mu*mu2+a1*mu2+a2*mu+a3);
                          break;
                        }
                        default: { // No interpolation
                          wave->data[i]=origData[i*wave->len/waveGenScaleX];
                          break;
                        }
                      }
                    }
                    wave->len=waveGenScaleX;
                    MARK_MODIFIED;
                  });
                }

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::InputInt("##WGScaleY",&waveGenScaleY,1,16)) {
                  if (waveGenScaleY<2) waveGenScaleY=2;
                  if (waveGenScaleY>256) waveGenScaleY=256;
                }
                ImGui::TableNextColumn();
                if (ImGui::Button("Scale Y")) {
                  if (waveGenScaleY>0 && wave->max!=(waveGenScaleY-1)) e->lockEngine([this,wave]() {
                    for (int i=0; i<wave->len; i++) {
                      wave->data[i]=(wave->data[i]*(waveGenScaleY+1))/(wave->max+1);
                    }
                    wave->max=waveGenScaleY-1;
                    MARK_MODIFIED;
                  });
                }

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::InputInt("##WGOffsetX",&waveGenOffsetX,1,16)) {
                  if (waveGenOffsetX<-wave->len+1) waveGenOffsetX=-wave->len+1;
                  if (waveGenOffsetX>wave->len-1) waveGenOffsetX=wave->len-1;
                }
                ImGui::TableNextColumn();
                if (ImGui::Button("Offset X")) {
                  if (waveGenOffsetX!=0 && wave->len>0) e->lockEngine([this,wave]() {
                    int origData[256];
                    memcpy(origData,wave->data,wave->len*sizeof(int));
                    int realOff=-waveGenOffsetX;
                    while (realOff<0) realOff+=wave->len;

                    for (int i=0; i<wave->len; i++) {
                      wave->data[i]=origData[(i+realOff)%wave->len];
                    }
                    MARK_MODIFIED;
                  });
                }

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::InputInt("##WGOffsetY",&waveGenOffsetY,1,16)) {
                  if (waveGenOffsetY<-wave->max) waveGenOffsetY=-wave->max;
                  if (waveGenOffsetY>wave->max) waveGenOffsetY=wave->max;
                }
                ImGui::TableNextColumn();
                if (ImGui::Button("Offset Y")) {
                  if (waveGenOffsetY!=0) e->lockEngine([this,wave]() {
                    for (int i=0; i<wave->len; i++) {
                      wave->data[i]=CLAMP(wave->data[i]+waveGenOffsetY,0,wave->max);
                    }
                    MARK_MODIFIED;
                  });
                }

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::InputInt("##WGSmooth",&waveGenSmooth,1,4)) {
                  if (waveGenSmooth>wave->len) waveGenSmooth=wave->len;
                  if (waveGenSmooth<1) waveGenSmooth=1;
                }
                ImGui::TableNextColumn();
                if (ImGui::Button("Smooth")) {
                  if (waveGenSmooth>0) e->lockEngine([this,wave]() {
                    int origData[256];
                    memcpy(origData,wave->data,wave->len*sizeof(int));
                    for (int i=0; i<wave->len; i++) { 
                      int dataSum=0;
                      for (int j=i; j<i+waveGenSmooth+1; j++) {
                        int pos=(j-((waveGenSmooth+1)/2));
                        while (pos<0) pos+=wave->len;
                        dataSum+=origData[pos%wave->len];
                      }
                      dataSum/=waveGenSmooth+1;
                      wave->data[i]=dataSum;
                    }
                    MARK_MODIFIED;
                  });
                }

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                float amp=waveGenAmplify*100.0f;
                if (ImGui::InputFloat("##WGAmplify",&amp,1.0f,10.0f)) {
                  waveGenAmplify=amp/100.0f;
                  if (waveGenAmplify<0.0f) waveGenAmplify=0.0f;
                  if (waveGenAmplify>100.0f) waveGenAmplify=100.0f;
                }
                ImGui::TableNextColumn();
                if (ImGui::Button("Amplify")) {
                  if (waveGenAmplify!=1.0f) e->lockEngine([this,wave]() {
                    for (int i=0; i<wave->len; i++) {
                      wave->data[i]=CLAMP(round((float)(wave->data[i]-(int)( /* Clang can you stop complaining */ (int)(wave->max+1)/(int)2))*waveGenAmplify),(int)(-((wave->max+1)/2)),(int)(wave->max/2))+(int)((wave->max+1)/2);
                    }
                    MARK_MODIFIED;
                  });
                }

                ImGui::EndTable();
              }

              ImVec2 buttonSize=ImGui::GetContentRegionAvail();
              buttonSize.y=0.0f;
              ImVec2 buttonSizeHalf=buttonSize;
              buttonSizeHalf.x-=ImGui::GetStyle().ItemSpacing.x;
              buttonSizeHalf.x*=0.5;

              if (ImGui::Button("Normalize",buttonSize)) {
                e->lockEngine([this,wave]() {
                  // find lowest point
                  int lowest=wave->max;
                  for (int i=0; i<wave->len; i++) {
                    if (wave->data[i]<lowest) lowest=wave->data[i];
                  }

                  // find highest point
                  int highest=0;
                  for (int i=0; i<wave->len; i++) {
                    if (wave->data[i]>highest) highest=wave->data[i];
                  }

                  // abort if lowest and highest points are equal
                  if (lowest==highest) return;

                  // abort if lowest and highest points already span the entire height
                  if (lowest==wave->max && highest==0) return;

                  // apply offset
                  for (int i=0; i<wave->len; i++) {
                    wave->data[i]-=lowest;
                  }
                  highest-=lowest;

                  // scale
                  for (int i=0; i<wave->len; i++) {
                    wave->data[i]=(wave->data[i]*wave->max)/highest;
                  }
                  MARK_MODIFIED;
                });
              }
              if (ImGui::Button("Invert",buttonSize)) {
                e->lockEngine([this,wave]() {
                  for (int i=0; i<wave->len; i++) {
                    wave->data[i]=wave->max-wave->data[i];
                  }
                  MARK_MODIFIED;
                });
              }

              if (ImGui::Button("Half",buttonSizeHalf)) {
                int origData[256];
                memcpy(origData,wave->data,wave->len*sizeof(int));

                for (int i=0; i<wave->len; i++) {
                  wave->data[i]=origData[i>>1];
                }
                MARK_MODIFIED;
              }
              ImGui::SameLine();
              if (ImGui::Button("Double",buttonSizeHalf)) {
                int origData[256];
                memcpy(origData,wave->data,wave->len*sizeof(int));

                for (int i=0; i<wave->len; i++) {
                  wave->data[i]=origData[(i*2)%wave->len];
                }
                MARK_MODIFIED;
              }

              if (ImGui::Button("Convert Signed/Unsigned",buttonSize)) {
                if (wave->max>0) e->lockEngine([this,wave]() {
                  for (int i=0; i<wave->len; i++) {
                    if (wave->data[i]>(wave->max/2)) {
                      wave->data[i]-=(wave->max+1)/2;
                    } else {
                      wave->data[i]+=(wave->max+1)/2;
                    }
                  }
                  MARK_MODIFIED;
                });
              }
              if (ImGui::Button("Randomize",buttonSize)) {
                if (wave->max>0) e->lockEngine([this,wave]() {
                  for (int i=0; i<wave->len; i++) {
                    wave->data[i]=rand()%(wave->max+1);
                  }
                  MARK_MODIFIED;
                });
              }
              ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
          }
        }
        ImGui::EndTable();
      }

      if (ImGui::RadioButton("Dec",!waveHex)) {
        waveHex=false;
      }
      ImGui::SameLine();
      if (ImGui::RadioButton("Hex",waveHex)) {
        waveHex=true;
      }
      ImGui::SameLine();
      if (!waveHex) if (ImGui::Button(waveSigned?"±##WaveSign":"+##WaveSign",ImVec2(ImGui::GetFrameHeight(),ImGui::GetFrameHeight()))) {
        waveSigned=!waveSigned;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Signed/Unsigned");
      }
      ImGui::SameLine();
      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x); // wavetable text input size found here
      if (ImGui::InputText("##MMLWave",&mmlStringW)) {
        int actualData[256];
        decodeMMLStrW(mmlStringW,actualData,wave->len,(waveSigned && !waveHex)?(-((wave->max+1)/2)):0,(waveSigned && !waveHex)?(wave->max/2):wave->max,waveHex);
        MARK_MODIFIED;
        if (waveSigned && !waveHex) {
          for (int i=0; i<wave->len; i++) {
            actualData[i]+=(wave->max+1)/2;
          }
        }
        memcpy(wave->data,actualData,wave->len*sizeof(int));
      }
      if (!ImGui::IsItemActive()) {
        int actualData[256];
        memcpy(actualData,wave->data,256*sizeof(int));
        if (waveSigned && !waveHex) {
          for (int i=0; i<wave->len; i++) {
            actualData[i]-=(wave->max+1)/2;
          }
        }
        encodeMMLStr(mmlStringW,actualData,wave->len,-1,-1,waveHex);
      }
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_WAVE_EDIT;
  ImGui::End();
}
