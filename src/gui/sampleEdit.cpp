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
#include <imgui.h>
#include <math.h>
#include "../ta-log.h"
#include "IconsFontAwesome4.h"
#include "misc/cpp/imgui_stdlib.h"
#include <fmt/printf.h>
#include "guiConst.h"

void FurnaceGUI::drawSampleEdit() {
  if (nextWindow==GUI_WINDOW_SAMPLE_EDIT) {
    sampleEditOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!sampleEditOpen) return;
  if (ImGui::Begin("Sample Editor",&sampleEditOpen,settings.allowEditDocking?0:ImGuiWindowFlags_NoDocking)) {
    if (curSample<0 || curSample>=(int)e->song.sample.size()) {
      ImGui::Text("no sample selected");
    } else {
      DivSample* sample=e->song.sample[curSample];
      String sampleType="Invalid";
      if (sample->depth<17) {
        if (sampleDepths[sample->depth]!=NULL) {
          sampleType=sampleDepths[sample->depth];
        }
      }
      ImGui::Text("Name");
      ImGui::SameLine();
      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
      ImGui::InputText("##SampleName",&sample->name);

      if (ImGui::BeginTable("SampleProps",4,ImGuiTableFlags_SizingStretchSame)) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Type");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::BeginCombo("##SampleType",sampleType.c_str())) {
          for (int i=0; i<17; i++) {
            if (sampleDepths[i]==NULL) continue;
            if (ImGui::Selectable(sampleDepths[i])) {
              sample->depth=i;
              e->renderSamplesP();
              updateSampleTex=true;
            }
            if (ImGui::IsItemHovered()) {
              ImGui::SetTooltip("no undo for sample type change operations!");
            }
          }
          ImGui::EndCombo();
        }

        ImGui::TableNextColumn();
        ImGui::Text("Rate (Hz)");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputInt("##SampleRate",&sample->rate,10,200)) {
          if (sample->rate<100) sample->rate=100;
          if (sample->rate>96000) sample->rate=96000;
        }

        ImGui::TableNextColumn();
        ImGui::Text("C-4 (Hz)");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputInt("##SampleCenter",&sample->centerRate,10,200)) {
          if (sample->centerRate<100) sample->centerRate=100;
          if (sample->centerRate>65535) sample->centerRate=65535;
        }

        ImGui::TableNextColumn();
        bool doLoop=(sample->loopStart>=0);
        if (ImGui::Checkbox("Loop",&doLoop)) {
          if (doLoop) {
            sample->loopStart=0;
          } else {
            sample->loopStart=-1;
          }
          updateSampleTex=true;
        }
        if (doLoop) {
          ImGui::SameLine();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::InputInt("##LoopPosition",&sample->loopStart,1,10)) {
            if (sample->loopStart<0 || sample->loopStart>=(int)sample->samples) {
              sample->loopStart=0;
            }
            updateSampleTex=true;
          }
        }
        ImGui::EndTable();
      }

      if (ImGui::InputDouble("Zoom",&sampleZoom,0.1,2.0)) {
        if (sampleZoom<0.01) sampleZoom=0.01;
        updateSampleTex=true;
      }
      if (ImGui::InputInt("Pos",&samplePos,1,10)) {
        if (samplePos>=(int)sample->samples) samplePos=sample->samples-1;
        if (samplePos<0) samplePos=0;
        updateSampleTex=true;
      }

      /*
      if (ImGui::Button("Apply")) {
        e->renderSamplesP();
      }
      ImGui::SameLine();
      if (ImGui::Button(ICON_FA_VOLUME_UP "##PreviewSample")) {
        e->previewSample(curSample);
      }
      ImGui::SameLine();
      if (ImGui::Button(ICON_FA_VOLUME_OFF "##StopSample")) {
        e->stopSamplePreview();
      }*/
      ImGui::Separator();

      if (ImGui::Button(ICON_FA_I_CURSOR "##SSelect")) {
        sampleDragMode=false;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Edit mode: Select");
      }
      ImGui::SameLine();
      if (ImGui::Button(ICON_FA_PENCIL "##SDraw")) {
        sampleDragMode=true;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Edit mode: Draw");
      }
      ImGui::SameLine();
      ImGui::Dummy(ImVec2(4.0*dpiScale,dpiScale));
      ImGui::SameLine();
      ImGui::Button(ICON_FA_ARROWS_H "##SResize");
      if (ImGui::IsItemClicked()) {
        resizeSize=sample->samples;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Resize");
      }
      if (ImGui::BeginPopupContextItem("SResizeOpt",ImGuiPopupFlags_MouseButtonLeft)) {
        if (ImGui::InputInt("Samples",&resizeSize,1,64)) {
          if (resizeSize<0) resizeSize=0;
          if (resizeSize>16777215) resizeSize=16777215;
        }
        if (ImGui::Button("Resize")) {
          e->synchronized([this,sample]() {
            if (!sample->resize(resizeSize)) {
              showError("couldn't resize! make sure your sample is 8 or 16-bit.");
            }
            e->renderSamples();
          });
          updateSampleTex=true;
          ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
      } else {
        resizeSize=sample->samples;
      }
      ImGui::SameLine();
      ImGui::Button(ICON_FA_EXPAND "##SResample");
      if (ImGui::IsItemClicked()) {
        resampleTarget=sample->rate;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Resample");
      }
      if (ImGui::BeginPopupContextItem("SResampleOpt",ImGuiPopupFlags_MouseButtonLeft)) {
        ImGui::Text("Rate");
        if (ImGui::InputDouble("##SRRate",&resampleTarget,1.0,50.0,"%g")) {
          if (resampleTarget<0) resampleTarget=0;
          if (resampleTarget>96000) resampleTarget=96000;
        }
        ImGui::SameLine();
        if (ImGui::Button("0.5x")) {
          resampleTarget*=0.5;
        }
        ImGui::SameLine();
        if (ImGui::Button("==")) {
          resampleTarget=sample->rate;
        }
        ImGui::SameLine();
        if (ImGui::Button("2.0x")) {
          resampleTarget*=2.0;
        }
        double factor=resampleTarget/(double)sample->rate;
        if (ImGui::InputDouble("Factor",&factor,0.125,0.5,"%g")) {
          resampleTarget=(double)sample->rate*factor;
          if (resampleTarget<0) resampleTarget=0;
          if (resampleTarget>96000) resampleTarget=96000;
        }
        ImGui::Combo("Filter",&resampleStrat,resampleStrats,6);
        if (ImGui::Button("Resample")) {
          e->synchronized([this,sample]() {
            if (!sample->resample(resampleTarget,resampleStrat)) {
              showError("couldn't resample! make sure your sample is 8 or 16-bit.");
            }
            e->renderSamples();
          });
          updateSampleTex=true;
          ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
      } else {
        resampleTarget=sample->rate;
      }
      ImGui::SameLine();
      ImGui::Dummy(ImVec2(4.0*dpiScale,dpiScale));
      ImGui::SameLine();
      ImGui::Button(ICON_FA_VOLUME_UP "##SAmplify");
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Amplify");
      }
      if (ImGui::BeginPopupContextItem("SAmplifyOpt",ImGuiPopupFlags_MouseButtonLeft)) {
        ImGui::Text("Volume");
        if (ImGui::InputFloat("##SRVolume",&amplifyVol,10.0,50.0,"%g%%")) {
          if (amplifyVol<0) amplifyVol=0;
          if (amplifyVol>10000) amplifyVol=10000;
        }
        ImGui::SameLine();
        ImGui::Text("(%.1fdB)",20.0*log10(amplifyVol/100.0f));
        ImGui::Button("Apply");
        ImGui::EndPopup();
      }
      ImGui::SameLine();
      ImGui::Button(ICON_FA_ARROWS_V "##SNormalize");
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Normalize");
      }
      ImGui::SameLine();
      ImGui::Button(ICON_FA_ERASER "##SSilence");
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Apply silence");
      }
      ImGui::SameLine();
      ImGui::Dummy(ImVec2(4.0*dpiScale,dpiScale));
      ImGui::SameLine();
      ImGui::Button(ICON_FA_BACKWARD "##SReverse");
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Reverse");
      }
      ImGui::SameLine();
      ImGui::Button(ICON_FA_SORT_AMOUNT_ASC "##SInvert");
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Invert");
      }
      ImGui::SameLine();
      ImGui::Button(ICON_FA_LEVEL_DOWN "##SSign");
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Signed/unsigned exchange");
      }
      ImGui::SameLine();
      ImGui::Button(ICON_FA_INDUSTRY "##SFilter");
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Apply filter");
      }

      ImGui::Separator();

      ImVec2 avail=ImGui::GetContentRegionAvail();
      avail.y-=ImGui::GetFontSize()+ImGui::GetStyle().ItemSpacing.y;
      int availX=avail.x;
      int availY=avail.y;

      if (sampleTex==NULL || sampleTexW!=avail.x || sampleTexH!=avail.y) {
        if (sampleTex!=NULL) {
          SDL_DestroyTexture(sampleTex);
          sampleTex=NULL;
        }
        if (avail.x>=1 && avail.y>=1) {
          logD("recreating sample texture.\n");
          sampleTex=SDL_CreateTexture(sdlRend,SDL_PIXELFORMAT_ABGR8888,SDL_TEXTUREACCESS_STREAMING,avail.x,avail.y);
          sampleTexW=avail.x;
          sampleTexH=avail.y;
          if (sampleTex==NULL) {
            logE("error while creating sample texture! %s\n",SDL_GetError());
          } else {
            updateSampleTex=true;
          }
        }
      }

      if (sampleTex!=NULL) {
        if (updateSampleTex) {
          unsigned int* data=NULL;
          int pitch=0;
          logD("updating sample texture.\n");
          if (SDL_LockTexture(sampleTex,NULL,(void**)&data,&pitch)!=0) {
            logE("error while locking sample texture! %s\n",SDL_GetError());
          } else {
            ImU32 bgColor=ImGui::GetColorU32(ImGuiCol_FrameBg);
            ImU32 lineColor=ImGui::GetColorU32(ImGuiCol_PlotLines);
            for (int i=0; i<availX*availY; i++) {
              data[i]=bgColor;
            }
            unsigned int xCoarse=samplePos;
            unsigned int xFine=0;
            unsigned int xAdvanceCoarse=sampleZoom;
            unsigned int xAdvanceFine=fmod(sampleZoom,1.0)*16777216;
            for (unsigned int i=0; i<(unsigned int)availX; i++) {
              if (xCoarse>=sample->samples) break;
              int y1, y2;
              int totalAdvance=0;
              y1=((unsigned short)sample->data16[xCoarse]^0x8000)*availY/65536;
              xFine+=xAdvanceFine;
              if (xFine>=16777216) {
                xFine-=16777216;
                totalAdvance++;
              }
              totalAdvance+=xAdvanceCoarse;
              if (xCoarse>=sample->samples) break;
              do {
                y2=((unsigned short)sample->data16[xCoarse]^0x8000)*availY/65536;
                if (y1>y2) {
                  y2^=y1;
                  y1^=y2;
                  y2^=y1;
                }
                for (int j=y1; j<=y2; j++) {
                  data[i+availX*j]=lineColor;
                }
                if (totalAdvance>0) xCoarse++;
              } while ((totalAdvance--)>0);
            }
            SDL_UnlockTexture(sampleTex);
          }
          updateSampleTex=false;
        }

        ImGui::ImageButton(sampleTex,avail,ImVec2(0,0),ImVec2(1,1),0);
        if (ImGui::IsItemClicked()) {
          logD("drawing\n");
        }
        String statusBar=sampleDragMode?"Draw":"Select";

        if (!sampleDragMode) {
          if (sampleSelStart>=0 && sampleSelEnd>=0) {
            int start=sampleSelStart;
            int end=sampleSelEnd;
            if (start>end) {
              start^=end;
              end^=start;
              start^=end;
            }
            statusBar+=fmt::sprintf(" (%d-%d)",start,end);
          }
        }

        if (ImGui::IsItemHovered()) {
          int posX=-1;
          int posY=0;
          ImVec2 pos=ImGui::GetMousePos();
          pos.x-=ImGui::GetItemRectMin().x;
          pos.y-=ImGui::GetItemRectMin().y;

          if (sampleZoom>0) {
            posX=samplePos+pos.x*sampleZoom;
            if (posX>(int)sample->samples) posX=-1;
          }
          posY=(0.5-pos.y/ImGui::GetItemRectSize().y)*((sample->depth==8)?255:32767);
          if (posX>=0) {
            statusBar+=fmt::sprintf(" | (%d, %d)",posX,posY);
          }
        }
        
        ImGui::Text("%s",statusBar.c_str());
      }

      /*
      bool considerations=false;
      ImGui::Text("notes:");
      if (sample->loopStart>=0) {
        considerations=true;
        ImGui::Text("- sample won't loop on Neo Geo ADPCM-A and X1-010");
        if (sample->loopStart&1) {
          ImGui::Text("- sample loop start will be aligned to the nearest even sample on Amiga");
        }
        if (sample->loopStart>0) {
          ImGui::Text("- sample loop start will be ignored on Neo Geo ADPCM-B");
        }
      }
      if (sample->samples&1) {
        considerations=true;
        ImGui::Text("- sample length will be aligned to the nearest even sample on Amiga");
      }
      if (sample->samples&511) {
        considerations=true;
        ImGui::Text("- sample length will be aligned and padded to 512 sample units on Neo Geo ADPCM.");
      }
      if (sample->samples&4095) {
        considerations=true;
        ImGui::Text("- sample length will be aligned and padded to 4096 sample units on X1-010.");
      }
      if (sample->samples>65535) {
        considerations=true;
        ImGui::Text("- maximum sample length on Sega PCM and QSound is 65536 samples");
      }
      if (sample->samples>131071) {
        considerations=true;
        ImGui::Text("- maximum sample length on X1-010 is 131072 samples");
      }
      if (sample->samples>2097151) {
        considerations=true;
        ImGui::Text("- maximum sample length on Neo Geo ADPCM is 2097152 samples");
      }
      if (!considerations) {
        ImGui::Text("- none");
      }*/
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_SAMPLE_EDIT;
  ImGui::End();
}
