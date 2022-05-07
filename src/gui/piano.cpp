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
#include "guiConst.h"
#include "imgui.h"
#include "imgui_internal.h"

const float topKeyStarts[5]={
  0.9f/7.0f, 2.1f/7.0f, 3.9f/7.0f, 5.0f/7.0f, 6.1f/7.0f
};

const int topKeyNotes[5]={
  1, 3, 6, 8, 10
};

const int bottomKeyNotes[7]={
  0, 2, 4, 5, 7, 9, 11
};

// TODO: actually implement a piano!
void FurnaceGUI::drawPiano() {
  if (nextWindow==GUI_WINDOW_PIANO) {
    pianoOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!pianoOpen) return;
  if (ImGui::Begin("Piano",&pianoOpen,(pianoOptions)?0:ImGuiWindowFlags_NoTitleBar)) {
    if (ImGui::BeginTable("PianoLayout",pianoOptions?2:1,ImGuiTableFlags_BordersInnerV)) {
      if (pianoOptions) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
      }
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch);

      ImGui::TableNextRow();
      if (pianoOptions) {
        ImGui::TableNextColumn();
        ImGui::Button("Options");
      }

      ImGui::TableNextColumn();
      ImGuiWindow* window=ImGui::GetCurrentWindow();
      ImVec2 size=ImGui::GetContentRegionAvail();
      ImDrawList* dl=ImGui::GetWindowDrawList();

      ImVec2 minArea=window->DC.CursorPos;
      ImVec2 maxArea=ImVec2(
        minArea.x+size.x,
        minArea.y+size.y
      );
      ImRect rect=ImRect(minArea,maxArea);

      // render piano
      //ImGui::ItemSize(size,ImGui::GetStyle().FramePadding.y);
      if (ImGui::ItemAdd(rect,ImGui::GetID("pianoDisplay"))) {
        int bottomNotes=7*pianoOctaves;
        for (int i=0; i<bottomNotes; i++) {
          int note=bottomKeyNotes[i%7]+12*((i/7)+pianoOffset);
          if (note<0) continue;
          if (note>=180) continue;
          float pkh=pianoKeyHit[note]*0.5;
          ImVec4 color=ImVec4(1.0f-pkh,1.0f-pkh,1.0f-pkh,1.0f);
          ImVec2 p0=ImLerp(rect.Min,rect.Max,ImVec2((float)i/bottomNotes,0.0f));
          ImVec2 p1=ImLerp(rect.Min,rect.Max,ImVec2((float)(i+1)/bottomNotes,1.0f));
          p1.x-=dpiScale;
          dl->AddRectFilled(p0,p1,ImGui::ColorConvertFloat4ToU32(color));
        }

        for (int i=0; i<pianoOctaves; i++) {
          ImVec2 op0=ImLerp(rect.Min,rect.Max,ImVec2((float)i/pianoOctaves,0.0f));
          ImVec2 op1=ImLerp(rect.Min,rect.Max,ImVec2((float)(i+1)/pianoOctaves,1.0f));

          for (int j=0; j<5; j++) {
            int note=topKeyNotes[j]+12*(i+pianoOffset);
            if (note<0) continue;
            if (note>=180) continue;
            float pkh=pianoKeyHit[note]*0.5;
            ImVec4 color=ImVec4(pkh,pkh,pkh,1.0f);
            ImVec2 p0=ImLerp(op0,op1,ImVec2(topKeyStarts[j]-0.05f,0.0f));
            ImVec2 p1=ImLerp(op0,op1,ImVec2(topKeyStarts[j]+0.05f,0.64f));
            dl->AddRectFilled(p0,p1,0xff000000);
            p0.x+=dpiScale;
            p1.x-=dpiScale;
            p1.y-=dpiScale;
            dl->AddRectFilled(p0,p1,ImGui::ColorConvertFloat4ToU32(color));
          }
        }

        const float reduction=ImGui::GetIO().DeltaTime*60.0f*0.12;
        for (int i=0; i<180; i++) {
          pianoKeyHit[i]-=reduction;
          if (pianoKeyHit[i]<0) pianoKeyHit[i]=0;
        }
      }

      if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        pianoOptions=!pianoOptions;
      }

      ImGui::EndTable();
    }
    /*
    for (int i=0; i<e->getTotalChannelCount(); i++) {
      DivChannelState* cs=e->getChanState(i);
      if (cs->keyOn) {
        const char* noteName=NULL;
        if (cs->note<-60 || cs->note>120) {
          noteName="???";
        } else {
          noteName=noteNames[cs->note+60];
        }
        ImGui::Text("%d: %s",i,noteName);
      }
    }*/
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_PIANO;
  ImGui::End();
}