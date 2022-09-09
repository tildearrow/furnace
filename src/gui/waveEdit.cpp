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
#include <math.h>
#include <imgui.h>

const char* waveGenBaseShapes[4]={
  "Sine",
  "Triangle",
  "Saw",
  "Pulse"
};

const float multFactors[16]={
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
};

void FurnaceGUI::doGenerateWave() {
  float finalResult[256];
  if (curWave<0 || curWave>=(int)e->song.wave.size()) return;

  DivWavetable* wave=e->song.wave[curWave];
  memset(finalResult,0,sizeof(float)*256);

  if (wave->len<2) return;

  if (waveGenFM) {
    for (int i=0; i<wave->len; i++) {
      float pos=(float)i/(float)wave->len;
      float s0=sin(pos*multFactors[waveGenMult[0]])*waveGenTL[0];
      float s1=sin((pos+(waveGenFMCon1[0]?s0:0.0f))*multFactors[waveGenMult[1]])*waveGenTL[1];
      float s2=sin((pos+(waveGenFMCon1[1]?s0:0.0f)+(waveGenFMCon2[0]?s1:0.0f))*multFactors[waveGenMult[2]])*waveGenTL[2];
      float s3=sin((pos+(waveGenFMCon1[2]?s0:0.0f)+(waveGenFMCon2[1]?s1:0.0f)+(waveGenFMCon3[0]?s2:0.0f))*multFactors[waveGenMult[3]])*waveGenTL[3];

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
  ImGui::SetNextWindowSizeConstraints(ImVec2(300.0f*dpiScale,300.0f*dpiScale),ImVec2(scrW*dpiScale,scrH*dpiScale));
  if (ImGui::Begin("Wavetable Editor",&waveEditOpen,globalWinFlags|(settings.allowEditDocking?0:ImGuiWindowFlags_NoDocking))) {
    if (curWave<0 || curWave>=(int)e->song.wave.size()) {
      ImGui::Text("no wavetable selected");
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
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_FLOPPY_O "##WESave")) {
          doAction(GUI_ACTION_WAVE_LIST_SAVE);
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
          ImGui::SetTooltip("use a width of:\n- any on Amiga/N163\n- 32 on Game Boy, PC Engine, SCC, Konami Bubble System, Namco WSG and WonderSwan\n- 64 on FDS\n- 128 on X1-010\nany other widths will be scaled during playback.");
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
          ImGui::SetTooltip("use a height of:\n- 15 for Game Boy, WonderSwan, Namco WSG, Konami Bubble System, X1-010 Envelope shape and N163\n- 31 for PC Engine\n- 63 for FDS\n- 255 for X1-010 and SCC\nany other heights will be scaled during playback.");
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
          PlotNoLerp("##Waveform",wavePreview,wave->len+1,0,NULL,0,wave->max,contentRegion);
        } else {
          PlotCustom("##Waveform",wavePreview,wave->len,0,NULL,0,wave->max,contentRegion,sizeof(float),ImVec4(1.0f,1.0f,1.0f,1.0f),0,NULL,true);
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
                  if (CWSliderInt("##WGMULT",&waveGenMult[i],0,15)) {
                    doGenerateWave();
                  }
                  ImGui::PopID();

                  ImGui::TableNextColumn();
                  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                  ImGui::PushID(i);
                  if (CWSliderFloat("##WGFB",&waveGenFB[i],0.0f,7.0f)) {
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
            if (ImGui::BeginTabItem("Mangle")) {
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
      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x); // wavetable text input size found here
      if (ImGui::InputText("##MMLWave",&mmlStringW)) {
        decodeMMLStrW(mmlStringW,wave->data,wave->len,wave->max,waveHex);
      }
      if (!ImGui::IsItemActive()) {
        encodeMMLStr(mmlStringW,wave->data,wave->len,-1,-1,waveHex);
      }
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_WAVE_EDIT;
  ImGui::End();
}
