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
#include <fmt/printf.h>
#include "IconsFontAwesome4.h"

const float topKeyStarts[5]={
  0.9f/7.0f, 2.1f/7.0f, 3.9f/7.0f, 5.0f/7.0f, 6.1f/7.0f
};

const int topKeyNotes[5]={
  1, 3, 6, 8, 10
};

const int bottomKeyNotes[7]={
  0, 2, 4, 5, 7, 9, 11
};

const bool isTopKey[12]={
  false, true, false, true, false, false, true, false, true, false, true, false
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
      int& off=(e->isPlaying() || pianoSharePosition)?pianoOffset:pianoOffsetEdit;
      int& oct=(e->isPlaying() || pianoSharePosition)?pianoOctaves:pianoOctavesEdit;
      bool view=(pianoView==2)?(!e->isPlaying()):pianoView;
      if (pianoOptions) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
      }
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch);

      ImGui::TableNextRow();
      if (pianoOptions) {
        ImGui::TableNextColumn();
        if (ImGui::Button(ICON_FA_ARROW_LEFT "##PianoLeft")) {
          off--;
          if (off<0) off=0;
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_ARROW_RIGHT "##PianoRight")) {
          off++;
          if ((off+oct)>14) off=15-oct;
        }
        ImGui::SameLine();
        ImGui::Button(ICON_FA_ELLIPSIS_V "##PianoOptions");
        if (ImGui::BeginPopupContextItem("PianoOptions",ImGuiPopupFlags_MouseButtonLeft)) {
          ImGui::Text("Key layout:");
          if (ImGui::RadioButton("Automatic",pianoView==2)) {
            pianoView=2;
          }
          if (ImGui::RadioButton("Standard",pianoView==0)) {
            pianoView=0;
          }
          if (ImGui::RadioButton("Continuous",pianoView==1)) {
            pianoView=1;
          }
          ImGui::Checkbox("Share play/edit offset/range",&pianoSharePosition);
          ImGui::EndPopup();
        }

        if (ImGui::Button(ICON_FA_MINUS "##PianoOctaveDown")) {
          oct--;
          if (oct<1) oct=1;
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_PLUS "##PianoOctaveUp")) {
          oct++;
          if (oct>15) oct=15;
          if ((off+oct)>14) off=15-oct;
        }
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
        if (view) {
          int notes=oct*12;
          for (int i=0; i<notes; i++) {
            int note=i+12*off;
            if (note<0) continue;
            if (note>=180) continue;
            float pkh=pianoKeyHit[note]*0.5;
            ImVec4 color=isTopKey[i%12]?ImVec4(pkh,pkh,pkh,1.0f):ImVec4(1.0f-pkh,1.0f-pkh,1.0f-pkh,1.0f);
            ImVec2 p0=ImLerp(rect.Min,rect.Max,ImVec2((float)i/notes,0.0f));
            ImVec2 p1=ImLerp(rect.Min,rect.Max,ImVec2((float)(i+1)/notes,1.0f));
            p1.x-=dpiScale;
            dl->AddRectFilled(p0,p1,ImGui::ColorConvertFloat4ToU32(color));
            if ((i%12)==0) {
              String label=fmt::sprintf("%d",(note-60)/12);
              ImVec2 pText=ImLerp(p0,p1,ImVec2(0.5f,1.0f));
              ImVec2 labelSize=ImGui::CalcTextSize(label.c_str());
              pText.x-=labelSize.x*0.5f;
              pText.y-=labelSize.y+ImGui::GetStyle().ItemSpacing.y;
              dl->AddText(pText,0xff404040,label.c_str());
            }
          }
        } else {
          int bottomNotes=7*oct;
          for (int i=0; i<bottomNotes; i++) {
            int note=bottomKeyNotes[i%7]+12*((i/7)+off);
            if (note<0) continue;
            if (note>=180) continue;
            float pkh=pianoKeyHit[note]*0.5;
            ImVec4 color=ImVec4(1.0f-pkh,1.0f-pkh,1.0f-pkh,1.0f);
            ImVec2 p0=ImLerp(rect.Min,rect.Max,ImVec2((float)i/bottomNotes,0.0f));
            ImVec2 p1=ImLerp(rect.Min,rect.Max,ImVec2((float)(i+1)/bottomNotes,1.0f));
            p1.x-=dpiScale;
            dl->AddRectFilled(p0,p1,ImGui::ColorConvertFloat4ToU32(color));
            if ((i%7)==0) {
              String label=fmt::sprintf("%d",(note-60)/12);
              ImVec2 pText=ImLerp(p0,p1,ImVec2(0.5f,1.0f));
              ImVec2 labelSize=ImGui::CalcTextSize(label.c_str());
              pText.x-=labelSize.x*0.5f;
              pText.y-=labelSize.y+ImGui::GetStyle().ItemSpacing.y;
              dl->AddText(pText,0xff404040,label.c_str());
            }
          }

          for (int i=0; i<oct; i++) {
            ImVec2 op0=ImLerp(rect.Min,rect.Max,ImVec2((float)i/oct,0.0f));
            ImVec2 op1=ImLerp(rect.Min,rect.Max,ImVec2((float)(i+1)/oct,1.0f));

            for (int j=0; j<5; j++) {
              int note=topKeyNotes[j]+12*(i+off);
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