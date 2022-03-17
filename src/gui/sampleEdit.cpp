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
#include <math.h>
#include "../ta-log.h"
#include "IconsFontAwesome4.h"
#include "misc/cpp/imgui_stdlib.h"
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
        if (ImGui::BeginCombo("Type",sampleType.c_str())) {
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
        if (ImGui::InputInt("Rate (Hz)",&sample->rate,10,200)) {
          if (sample->rate<100) sample->rate=100;
          if (sample->rate>96000) sample->rate=96000;
        }

        ImGui::TableNextColumn();
        if (ImGui::InputInt("Pitch of C-4 (Hz)",&sample->centerRate,10,200)) {
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

      ImVec2 avail=ImGui::GetContentRegionAvail();
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

        ImGui::Image(sampleTex,avail);
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
