/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2024 tildearrow and contributors
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
#include <fmt/printf.h>
#include "imgui.h"
#include "imgui_internal.h"

#define CENTER_TEXT(text) \
  ImGui::SetCursorPosX(ImGui::GetCursorPosX()+0.5*(ImGui::GetContentRegionAvail().x-ImGui::CalcTextSize(text).x));

void FurnaceGUI::drawMemory() {
  if (nextWindow==GUI_WINDOW_MEMORY) {
    memoryOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!memoryOpen) return;
  if (ImGui::Begin("Memory Composition",&memoryOpen,globalWinFlags,_("Memory Composition"))) {
    ImDrawList* dl=ImGui::GetWindowDrawList();
    ImGuiWindow* window=ImGui::GetCurrentWindow();
    char tempID[1024];
    bool have=false;

    for (int i=0; i<e->song.systemLen; i++) {
      DivDispatch* dispatch=e->getDispatch(i);
      for (int j=0; j<4; j++) {
        const DivMemoryComposition* mc=dispatch->getMemCompo(j);
        if (mc==NULL) break;
        have=true;

        ImGui::Text("%s: %s",e->getSystemName(e->song.system[i]),mc->name.c_str());
        ImGui::SameLine();
        if (mc->capacity>=1024 && settings.memUsageUnit==1) {
          ImGui::Text("%dK/%dK (%.1f%%)",(int)mc->used>>10,(int)mc->capacity>>10,100.0*(double)mc->used/(double)mc->capacity);
        } else {
          ImGui::Text("%d/%d (%.1f%%)",(int)mc->used,(int)mc->capacity,100.0*(double)mc->used/(double)mc->capacity);
        }

        ImVec2 size=ImVec2(ImGui::GetContentRegionAvail().x,36.0f*dpiScale);
        ImVec2 minArea=window->DC.CursorPos;
        ImVec2 maxArea=ImVec2(
          minArea.x+size.x,
          minArea.y+size.y
        );
        ImRect rect=ImRect(minArea,maxArea);
        ImRect dataRect=rect;
        ImRect entryRect=rect;
        ImGuiStyle& style=ImGui::GetStyle();
        ImGui::ItemSize(size,style.FramePadding.y);
        snprintf(tempID,1023,"MC%d_%d",i,j);
        if (ImGui::ItemAdd(rect,ImGui::GetID(tempID))) {
          dl->AddRectFilled(rect.Min,rect.Max,ImGui::GetColorU32(uiColors[GUI_COLOR_MEMORY_BG]));

          if (mc->memory!=NULL && mc->waveformView!=DIV_MEMORY_WAVE_NONE) {
            dataRect.Max.y-=8.0f*dpiScale;
            entryRect.Min.y=entryRect.Max.y-8.0f*dpiScale;
          }

          int curHover=-1;
          int kIndex=0;
          if (mc->capacity>0) for (const DivMemoryEntry& k: mc->entries) {
            if (k.begin==k.end) {
              kIndex++;
              continue;
            }
            ImVec2 pos1=ImLerp(entryRect.Min,entryRect.Max,ImVec2((double)k.begin/(double)mc->capacity,(k.type==DIV_MEMORY_N163_LOAD)?0.5f:0.0f));
            ImVec2 pos2=ImLerp(entryRect.Min,entryRect.Max,ImVec2((double)k.end/(double)mc->capacity,(k.type==DIV_MEMORY_N163_PLAY)?0.5f:1.0f));
            ImVec2 linePos=pos1;
            linePos.y=rect.Max.y;

            dl->AddRectFilled(pos1,pos2,ImGui::GetColorU32(uiColors[GUI_COLOR_MEMORY_FREE+(int)k.type]));
            dl->AddLine(pos1,linePos,ImGui::GetColorU32(ImGuiCol_Border),dpiScale);

            if (ImGui::GetMousePos().x>=pos1.x &&
                ImGui::GetMousePos().x<=pos2.x &&
                ImGui::GetMousePos().y>=pos1.y &&
                ImGui::GetMousePos().y<=pos2.y) {
              curHover=kIndex;
            }
            kIndex++;
          }

          if (mc->memory!=NULL) {
            switch (mc->waveformView) {
              case DIV_MEMORY_WAVE_4BIT:
                for (int k=0; k<(int)(mc->capacity<<1); k++) {
                  unsigned char nibble=mc->memory[k>>1];
                  if (k&1) {
                    nibble>>=4;
                  } else {
                    nibble&=15;
                  }

                  ImVec2 pos1=ImLerp(dataRect.Min,dataRect.Max,ImVec2((double)k/(double)(mc->capacity<<1),1.0f-((float)(nibble+1)/16.0f)));
                  ImVec2 pos2=ImLerp(dataRect.Min,dataRect.Max,ImVec2((double)(k+1)/(double)(mc->capacity<<1),1.0f));
                  dl->AddRectFilled(pos1,pos2,ImGui::GetColorU32(uiColors[GUI_COLOR_MEMORY_DATA]));
                }
                break;
              case DIV_MEMORY_WAVE_8BIT_SIGNED:
                for (int k=0; k<(int)mc->capacity; k++) {
                  signed char val=(signed char)mc->memory[k];
                  ImVec2 pos1=ImLerp(dataRect.Min,dataRect.Max,ImVec2((double)k/(double)(mc->capacity),1.0f-((float)(val+129)/256.0f)));
                  ImVec2 pos2=ImLerp(dataRect.Min,dataRect.Max,ImVec2((double)(k+1)/(double)(mc->capacity),1.0f));
                  dl->AddRectFilled(pos1,pos2,ImGui::GetColorU32(uiColors[GUI_COLOR_MEMORY_DATA]));
                }
                break;
              default:
                break;
            }
          }

          if (ImGui::ItemHoverable(rect,ImGui::GetID(tempID),0)) {
            if (curHover>=0 && curHover<(int)mc->entries.size()) {
              const DivMemoryEntry& entry=mc->entries[curHover];
              if (ImGui::BeginTooltip()) {
                switch (entry.type) {
                  case DIV_MEMORY_SAMPLE:
                  case DIV_MEMORY_BANK0:
                  case DIV_MEMORY_BANK1:
                  case DIV_MEMORY_BANK2:
                  case DIV_MEMORY_BANK3:
                  case DIV_MEMORY_BANK4:
                  case DIV_MEMORY_BANK5:
                  case DIV_MEMORY_BANK6:
                  case DIV_MEMORY_BANK7: {
                    DivSample* sample=e->getSample(entry.asset);
                    ImGui::Text("%d: %s",curHover,sample->name.c_str());
                    if ((int)entry.type>=(int)DIV_MEMORY_BANK0) {
                      ImGui::Text(_("bank %d"),(int)entry.type-(int)DIV_MEMORY_BANK0);
                    }
                    if ((entry.end-entry.begin)>=1024 && settings.memUsageUnit==1) {
                      ImGui::Text("%d-%d ($%x-$%x): %dK ($%x)",(int)entry.begin,(int)entry.end-1,(int)entry.begin,(int)entry.end-1,(int)(entry.end-entry.begin)>>10,(int)(entry.end-entry.begin));
                    } else {
                      ImGui::Text("%d-%d ($%x-$%x): %d bytes ($%x)",(int)entry.begin,(int)entry.end-1,(int)entry.begin,(int)entry.end-1,(int)(entry.end-entry.begin),(int)(entry.end-entry.begin));
                    }
                    break;
                  }
                  default:
                    ImGui::Text("%d: %s",curHover,entry.name.c_str());
                    if ((entry.end-entry.begin)>=1024 && settings.memUsageUnit==1) {
                      ImGui::Text("%d-%d ($%x-$%x): %dK ($%x)",(int)entry.begin,(int)entry.end-1,(int)entry.begin,(int)entry.end-1,(int)(entry.end-entry.begin)>>10,(int)(entry.end-entry.begin));
                    } else {
                      ImGui::Text("%d-%d ($%x-$%x): %d bytes ($%x)",(int)entry.begin,(int)entry.end-1,(int)entry.begin,(int)entry.end-1,(int)(entry.end-entry.begin),(int)(entry.end-entry.begin));
                    }
                    break;
                }

                ImGui::EndTooltip();
              }
            }
          }
        }
      }
    }

    if (!have) {
      ImGui::SetCursorPosY(ImGui::GetCursorPosY()+(ImGui::GetContentRegionAvail().y-ImGui::GetFrameHeight()+ImGui::GetStyle().ItemSpacing.y)*0.5f);
      CENTER_TEXT(_("no chips with memory"));
      ImGui::Text(_("no chips with memory"));
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_MEMORY;
  ImGui::End();
}
