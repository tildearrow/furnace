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

#define VALUE_DIGIT(x,label) \
  if (ImGui::Button(label,buttonSize)) { \
    valueInput(x,false); \
  }

void FurnaceGUI::drawPiano() {
  if (nextWindow==GUI_WINDOW_PIANO) {
    pianoOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!pianoOpen) return;
  if (mobileUI) {
    ImGui::SetNextWindowPos(ImVec2(patWindowPos.x,patWindowPos.y+patWindowSize.y));
    ImGui::SetNextWindowSize(portrait?ImVec2(canvasW,0.4*canvasW):ImVec2(canvasW-(0.16*canvasH),0.3*canvasH));
  }
  if (ImGui::Begin("Piano",&pianoOpen,((pianoOptions)?0:ImGuiWindowFlags_NoTitleBar)|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse|globalWinFlags)) {
    bool oldPianoKeyPressed[180];
    memcpy(oldPianoKeyPressed,pianoKeyPressed,180*sizeof(bool));
    memset(pianoKeyPressed,0,180*sizeof(bool));
    if (ImGui::BeginTable("PianoLayout",((pianoOptions && (!mobileUI || !portrait))?2:1),ImGuiTableFlags_BordersInnerV)) {
      int& off=(e->isPlaying() || pianoSharePosition)?pianoOffset:pianoOffsetEdit;
      int& oct=(e->isPlaying() || pianoSharePosition)?pianoOctaves:pianoOctavesEdit;
      bool view=(pianoView==PIANO_LAYOUT_AUTOMATIC)?(!e->isPlaying()):pianoView;
      if (pianoOptions && (!mobileUI || !portrait)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
      }
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,1.0f);

      ImGui::TableNextRow();
      if (pianoOptions) {
        ImGui::TableNextColumn();
        float optionSizeY=ImGui::GetContentRegionAvail().y*((mobileUI && portrait)?0.3:0.5)-ImGui::GetStyle().ItemSpacing.y;
        ImVec2 optionSize=ImVec2((mobileUI && portrait)?((ImGui::GetContentRegionAvail().x-ImGui::GetStyle().ItemSpacing.x*5.0f)/6.0f):(1.2f*optionSizeY),optionSizeY);
        if (pianoOptionsSet) {
          if (ImGui::Button("OFF##PianoNOff",optionSize)) {
            if (edit) noteInput(0,GUI_NOTE_OFF);
          }
          ImGui::SameLine();
          if (ImGui::Button("===##PianoNRel",optionSize)) {
            if (edit) noteInput(0,GUI_NOTE_OFF_RELEASE);
          }
        } else {
          if (ImGui::Button(ICON_FA_ARROW_LEFT "##PianoLeft",optionSize)) {
            off--;
            if (off<0) off=0;
          }
          ImGui::SameLine();
          if (ImGui::Button(ICON_FA_ARROW_RIGHT "##PianoRight",optionSize)) {
            off++;
            if ((off+oct)>14) off=15-oct;
          }
        }
        ImGui::SameLine();
        ImGui::Button(ICON_FA_ELLIPSIS_V "##PianoOptions",optionSize);
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Options");
        }
        if (ImGui::BeginPopupContextItem("PianoOptions",ImGuiPopupFlags_MouseButtonLeft)) {
          ImGui::Text("Key layout:");
          if (ImGui::RadioButton("Automatic",pianoView==PIANO_LAYOUT_AUTOMATIC)) {
            pianoView=PIANO_LAYOUT_AUTOMATIC;
          }
          if (ImGui::RadioButton("Standard",pianoView==PIANO_LAYOUT_STANDARD)) {
            pianoView=PIANO_LAYOUT_STANDARD;
          }
          if (ImGui::RadioButton("Continuous",pianoView==PIANO_LAYOUT_CONTINUOUS)) {
            pianoView=PIANO_LAYOUT_CONTINUOUS;
          }
          ImGui::Text("Value input pad:");
          if (ImGui::RadioButton("Disabled",pianoInputPadMode==PIANO_INPUT_PAD_DISABLE)) {
            pianoInputPadMode=PIANO_INPUT_PAD_DISABLE;
          }
          if (ImGui::RadioButton("Replace piano",pianoInputPadMode==PIANO_INPUT_PAD_REPLACE)) {
            pianoInputPadMode=PIANO_INPUT_PAD_REPLACE;
          }
          if (ImGui::RadioButton("Split (automatic)",pianoInputPadMode==PIANO_INPUT_PAD_SPLIT_AUTO)) {
            pianoInputPadMode=PIANO_INPUT_PAD_SPLIT_AUTO;
          }
          if (ImGui::RadioButton("Split (always visible)",pianoInputPadMode==PIANO_INPUT_PAD_SPLIT_VISIBLE)) {
            pianoInputPadMode=PIANO_INPUT_PAD_SPLIT_VISIBLE;
          }
          ImGui::Checkbox("Share play/edit offset/range",&pianoSharePosition);
          ImGui::EndPopup();
        }

        if (mobileUI && portrait) {
          ImGui::SameLine();
        }

        if (pianoOptionsSet) {
          if (ImGui::Button("REL##PianoNMRel",optionSize)) {
            if (edit) noteInput(0,GUI_NOTE_RELEASE);
          }
          ImGui::SameLine();
          if (ImGui::Button(ICON_FA_TIMES "##PianoDelP",optionSize)) {
            doDelete();
          }
        } else {
          if (ImGui::Button(ICON_FA_MINUS "##PianoOctaveDown",optionSize)) {
            oct--;
            if (oct<1) oct=1;
          }
          ImGui::SameLine();
          if (ImGui::Button(ICON_FA_PLUS "##PianoOctaveUp",optionSize)) {
            oct++;
            if (oct>15) oct=15;
            if ((off+oct)>14) off=15-oct;
          }
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_ELLIPSIS_H "##PianoSel",optionSize)) {
          pianoOptionsSet=!pianoOptionsSet;
        }
      }

      if (mobileUI && portrait) {
        ImGui::TableNextRow();
      }

      ImGui::TableNextColumn();
      if (pianoInputPadMode==PIANO_INPUT_PAD_REPLACE && cursor.xFine>0 && curWindow==GUI_WINDOW_PATTERN) {
        ImVec2 buttonSize=ImGui::GetContentRegionAvail();
        if (ImGui::BeginTable("InputPadP",8,ImGuiTableFlags_SizingFixedSame)) {
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          buttonSize.x/=8.0f;
          buttonSize.x-=ImGui::GetStyle().CellPadding.x*2.0f;
          buttonSize.y/=2.0f;
          buttonSize.y-=ImGui::GetStyle().CellPadding.y*2.0f;

          VALUE_DIGIT(0,"0");
          ImGui::TableNextColumn();
          VALUE_DIGIT(1,"1");
          ImGui::TableNextColumn();
          VALUE_DIGIT(2,"2");
          ImGui::TableNextColumn();
          VALUE_DIGIT(3,"3");
          ImGui::TableNextColumn();
          VALUE_DIGIT(4,"4");
          ImGui::TableNextColumn();
          VALUE_DIGIT(5,"5");
          ImGui::TableNextColumn();
          VALUE_DIGIT(6,"6");
          ImGui::TableNextColumn();
          VALUE_DIGIT(7,"7");

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          VALUE_DIGIT(8,"8");
          ImGui::TableNextColumn();
          VALUE_DIGIT(9,"9");
          ImGui::TableNextColumn();
          VALUE_DIGIT(10,"A");
          ImGui::TableNextColumn();
          VALUE_DIGIT(11,"B");
          ImGui::TableNextColumn();
          VALUE_DIGIT(12,"C");
          ImGui::TableNextColumn();
          VALUE_DIGIT(13,"D");
          ImGui::TableNextColumn();
          VALUE_DIGIT(14,"E");
          ImGui::TableNextColumn();
          VALUE_DIGIT(15,"F");

          ImGui::EndTable();
        }
      } else {
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
          bool canInput=false;
          if (ImGui::ItemHoverable(rect,ImGui::GetID("pianoDisplay"))) {
            canInput=true;
            ImGui::InhibitInertialScroll();
          }
          if (view) {
            int notes=oct*12;
            // evaluate input
            if (canInput) for (TouchPoint& i: activePoints) {
              if (rect.Contains(ImVec2(i.x,i.y))) {
                int note=(((i.x-rect.Min.x)/(rect.Max.x-rect.Min.x))*notes)+12*off;
                if (note<0) continue;
                if (note>=180) continue;
                pianoKeyPressed[note]=true;
              }
            }

            for (int i=0; i<notes; i++) {
              int note=i+12*off;
              if (note<0) continue;
              if (note>=180) continue;
              float pkh=pianoKeyHit[note];
              ImVec4 color=isTopKey[i%12]?uiColors[GUI_COLOR_PIANO_KEY_TOP]:uiColors[GUI_COLOR_PIANO_KEY_BOTTOM];
              if (pianoKeyPressed[note]) {
                color=isTopKey[i%12]?uiColors[GUI_COLOR_PIANO_KEY_TOP_ACTIVE]:uiColors[GUI_COLOR_PIANO_KEY_BOTTOM_ACTIVE];
              } else {
                ImVec4 colorHit=isTopKey[i%12]?uiColors[GUI_COLOR_PIANO_KEY_TOP_HIT]:uiColors[GUI_COLOR_PIANO_KEY_BOTTOM_HIT];
                color.x+=(colorHit.x-color.x)*pkh;
                color.y+=(colorHit.y-color.y)*pkh;
                color.z+=(colorHit.z-color.z)*pkh;
                color.w+=(colorHit.w-color.w)*pkh;
              }
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
            // evaluate input
            if (canInput) for (TouchPoint& i: activePoints) {
              if (rect.Contains(ImVec2(i.x,i.y))) {
                // top
                int o=((i.x-rect.Min.x)/(rect.Max.x-rect.Min.x))*oct;
                ImVec2 op0=ImLerp(rect.Min,rect.Max,ImVec2((float)o/oct,0.0f));
                ImVec2 op1=ImLerp(rect.Min,rect.Max,ImVec2((float)(o+1)/oct,1.0f));
                bool foundTopKey=false;

                for (int j=0; j<5; j++) {
                  int note=topKeyNotes[j]+12*(o+off);
                  if (note<0) continue;
                  if (note>=180) continue;
                  ImRect keyRect=ImRect(
                    ImLerp(op0,op1,ImVec2(topKeyStarts[j]-0.05f,0.0f)),
                    ImLerp(op0,op1,ImVec2(topKeyStarts[j]+0.05f,0.64f))
                  );
                  if (keyRect.Contains(ImVec2(i.x,i.y))) {
                    pianoKeyPressed[note]=true;
                    foundTopKey=true;
                    break;
                  }
                }
                if (foundTopKey) continue;

                // bottom
                int n=((i.x-rect.Min.x)/(rect.Max.x-rect.Min.x))*bottomNotes;
                int note=bottomKeyNotes[n%7]+12*((n/7)+off);
                if (note<0) continue;
                if (note>=180) continue;
                pianoKeyPressed[note]=true;
              }
            }

            for (int i=0; i<bottomNotes; i++) {
              int note=bottomKeyNotes[i%7]+12*((i/7)+off);
              if (note<0) continue;
              if (note>=180) continue;

              float pkh=pianoKeyHit[note];
              ImVec4 color=uiColors[GUI_COLOR_PIANO_KEY_BOTTOM];
              if (pianoKeyPressed[note]) {
                color=uiColors[GUI_COLOR_PIANO_KEY_BOTTOM_ACTIVE];
              } else {
                ImVec4 colorHit=uiColors[GUI_COLOR_PIANO_KEY_BOTTOM_HIT];
                color.x+=(colorHit.x-color.x)*pkh;
                color.y+=(colorHit.y-color.y)*pkh;
                color.z+=(colorHit.z-color.z)*pkh;
                color.w+=(colorHit.w-color.w)*pkh;
              }

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
                float pkh=pianoKeyHit[note];
                ImVec4 color=uiColors[GUI_COLOR_PIANO_KEY_TOP];
                if (pianoKeyPressed[note]) {
                  color=uiColors[GUI_COLOR_PIANO_KEY_TOP_ACTIVE];
                } else {
                  ImVec4 colorHit=uiColors[GUI_COLOR_PIANO_KEY_TOP_HIT];
                  color.x+=(colorHit.x-color.x)*pkh;
                  color.y+=(colorHit.y-color.y)*pkh;
                  color.z+=(colorHit.z-color.z)*pkh;
                  color.w+=(colorHit.w-color.w)*pkh;
                }
                ImVec2 p0=ImLerp(op0,op1,ImVec2(topKeyStarts[j]-0.05f,0.0f));
                ImVec2 p1=ImLerp(op0,op1,ImVec2(topKeyStarts[j]+0.05f,0.64f));
                dl->AddRectFilled(p0,p1,ImGui::GetColorU32(uiColors[GUI_COLOR_PIANO_BACKGROUND]));
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

        // TODO: wave and sample preview
        // first check released keys
        for (int i=0; i<180; i++) {
          int note=i-60;
          if (!pianoKeyPressed[i]) {
            if (pianoKeyPressed[i]!=oldPianoKeyPressed[i]) {
              switch (curWindow) {
                case GUI_WINDOW_WAVE_LIST:
                case GUI_WINDOW_WAVE_EDIT:
                  e->stopWavePreview();
                  break;
                case GUI_WINDOW_SAMPLE_LIST:
                case GUI_WINDOW_SAMPLE_EDIT:
                  e->stopSamplePreview();
                  break;
                default:
                  e->synchronized([this,note]() {
                    e->autoNoteOff(-1,note);
                  });
                  break;
              }
            }
          }
        }
        // then pressed ones
        for (int i=0; i<180; i++) {
          int note=i-60;
          if (pianoKeyPressed[i]) {
            if (pianoKeyPressed[i]!=oldPianoKeyPressed[i]) {
              switch (curWindow) {
                case GUI_WINDOW_WAVE_LIST:
                case GUI_WINDOW_WAVE_EDIT:
                  e->previewWave(curWave,note);
                  break;
                case GUI_WINDOW_SAMPLE_LIST:
                case GUI_WINDOW_SAMPLE_EDIT:
                  e->previewSample(curSample,note);
                  break;
                default:
                  e->synchronized([this,note]() {
                    e->autoNoteOn(-1,curIns,note);
                  });
                  if (edit && curWindow!=GUI_WINDOW_INS_LIST && curWindow!=GUI_WINDOW_INS_EDIT) noteInput(note,0);
                  break;
              }
            }
          }
        }
      }

      ImGui::EndTable();
    }
  }
  // don't worry about it
  //if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_PIANO;
  ImGui::End();

  // draw input pad if necessary
  if (curWindow==GUI_WINDOW_PATTERN && ((pianoInputPadMode==PIANO_INPUT_PAD_SPLIT_AUTO && cursor.xFine>0) || pianoInputPadMode==PIANO_INPUT_PAD_SPLIT_VISIBLE)) {
    if (ImGui::Begin("Input Pad",NULL,ImGuiWindowFlags_NoTitleBar)) {
      ImGui::BeginDisabled(cursor.xFine==0);
      if (ImGui::BeginTable("InputPad",3,ImGuiTableFlags_Borders)) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImVec2 buttonSize=ImGui::GetContentRegionAvail();
        buttonSize.y/=6.0f;
        buttonSize.y-=ImGui::GetStyle().CellPadding.y*2.0f;

        VALUE_DIGIT(10,"A");
        ImGui::TableNextColumn();
        VALUE_DIGIT(11,"B");
        ImGui::TableNextColumn();
        VALUE_DIGIT(12,"C");
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        VALUE_DIGIT(13,"D");
        ImGui::TableNextColumn();
        VALUE_DIGIT(14,"E");
        ImGui::TableNextColumn();
        VALUE_DIGIT(15,"F");

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        VALUE_DIGIT(1,"1");
        ImGui::TableNextColumn();
        VALUE_DIGIT(2,"2");
        ImGui::TableNextColumn();
        VALUE_DIGIT(3,"3");
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        VALUE_DIGIT(4,"4");
        ImGui::TableNextColumn();
        VALUE_DIGIT(5,"5");
        ImGui::TableNextColumn();
        VALUE_DIGIT(6,"6");
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        VALUE_DIGIT(7,"7");
        ImGui::TableNextColumn();
        VALUE_DIGIT(8,"8");
        ImGui::TableNextColumn();
        VALUE_DIGIT(9,"9");
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Button(ICON_FA_TIMES "##PianoDel",buttonSize)) {
          doDelete();
        }
        ImGui::TableNextColumn();
        VALUE_DIGIT(0,"0");
        ImGui::TableNextColumn();

        ImGui::EndTable();
      }
      ImGui::EndDisabled();
    }
    // don't worry about it either
    //if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_PIANO;
    ImGui::End();
  }
}
