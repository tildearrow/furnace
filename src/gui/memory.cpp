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
  if (ImGui::Begin("Memory Composition",&memoryOpen,globalWinFlags)) {
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
        ImGui::Text("%d/%d (%.1f%%)",(int)mc->used,(int)mc->capacity,100.0*(double)mc->used/(double)mc->capacity);

        ImVec2 size=ImVec2(ImGui::GetContentRegionAvail().x,36.0f*dpiScale);
        ImVec2 minArea=window->DC.CursorPos;
        ImVec2 maxArea=ImVec2(
          minArea.x+size.x,
          minArea.y+size.y
        );
        ImRect rect=ImRect(minArea,maxArea);
        ImGuiStyle& style=ImGui::GetStyle();
        ImGui::ItemSize(size,style.FramePadding.y);
        snprintf(tempID,1023,"MC%d_%d",i,j);
        if (ImGui::ItemAdd(rect,ImGui::GetID(tempID))) {
          dl->AddRectFilled(rect.Min,rect.Max,ImGui::GetColorU32(uiColors[GUI_COLOR_MEMORY_BG]));

          int curHover=-1;
          int kIndex=0;
          if (mc->capacity>0) for (const DivMemoryEntry& k: mc->entries) {
            ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2((double)k.begin/(double)mc->capacity,0.0f));
            ImVec2 pos2=ImLerp(rect.Min,rect.Max,ImVec2((double)k.end/(double)mc->capacity,1.0f));
            ImVec2 linePos=pos1;
            linePos.y=rect.Max.y;

            dl->AddRectFilled(pos1,pos2,ImGui::GetColorU32(uiColors[GUI_COLOR_MEMORY_FREE+(int)k.type]));
            dl->AddLine(pos1,linePos,ImGui::GetColorU32(ImGuiCol_Border),dpiScale);

            if (ImGui::GetMousePos().x>=pos1.x && ImGui::GetMousePos().x<=pos2.x) {
              curHover=kIndex;
            }
            kIndex++;
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
                      ImGui::Text("bank %d",(int)entry.type-(int)DIV_MEMORY_BANK0);
                    }
                    ImGui::Text("%d-%d ($%x-$%x): %d bytes ($%x)",(int)entry.begin,(int)entry.end-1,(int)entry.begin,(int)entry.end-1,(int)(entry.end-entry.begin),(int)(entry.end-entry.begin));
                    ImGui::Text("click to open sample editor");
                    break;
                  }
                  default:
                    ImGui::Text("%d: %s",curHover,entry.name.c_str());
                    ImGui::Text("%d-%d ($%x-$%x): %d bytes ($%x)",(int)entry.begin,(int)entry.end-1,(int)entry.begin,(int)entry.end-1,(int)(entry.end-entry.begin),(int)(entry.end-entry.begin));
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
      CENTER_TEXT("no chips with memory");
      ImGui::Text("no chips with memory");
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_MEMORY;
  ImGui::End();
}
