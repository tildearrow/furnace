#include "gui.h"
#include "guiConst.h"
#include <imgui.h>

void FurnaceGUI::drawEffectList() {
  if (nextWindow==GUI_WINDOW_EFFECT_LIST) {
    effectListOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!effectListOpen) return;
  ImGui::SetNextWindowSizeConstraints(ImVec2(60.0f*dpiScale,20.0f*dpiScale),ImVec2(canvasW,canvasH));
  if (ImGui::Begin("Effect List",&effectListOpen,globalWinFlags)) {
    ImGui::Text("Chip at cursor: %s",e->getSystemName(e->sysOfChan[cursor.xCoarse]));
    if (ImGui::Button("Sort Effects")) ImGui::OpenPopup("effectSort");
    if (ImGui::BeginPopup("effectSort")) {
      ImGui::Checkbox("Pitch",&effectShowPitch);
      ImGui::Checkbox("Song",&effectShowSong);
      ImGui::Checkbox("Time",&effectShowTime);
      ImGui::Checkbox("Speed",&effectShowSpeed);
      ImGui::Checkbox("Panning",&effectShowPanning);
      ImGui::Checkbox("Volume",&effectShowVolume);
      ImGui::Checkbox("System (primary)",&effectShowSysPrimary);
      ImGui::Checkbox("System (secondary)",&effectShowSysSecondary);
      ImGui::Checkbox("Miscellaneous",&effectShowMisc);

      if (ImGui::Button("All")) {
        effectShowMisc = true;
        effectShowPanning = true;
        effectShowPitch = true;
        effectShowVolume = true;
        effectShowSong = true;
        effectShowTime = true;
        effectShowSpeed = true;
        effectShowSysPrimary = true;
        effectShowSysSecondary = true;
      }
      ImGui::SameLine();
      if (ImGui::Button("None")) {
        effectShowMisc = false;
        effectShowPanning = false;
        effectShowPitch = false;
        effectShowVolume = false;
        effectShowSong = false;
        effectShowTime = false;
        effectShowSpeed = false;
        effectShowSysPrimary = false;
        effectShowSysSecondary = false;
      }

      ImGui::EndPopup();
    }
    
    if (ImGui::BeginTable("effectList",2)) {
      ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch);

      ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
      ImGui::TableNextColumn();
      ImGui::Text("Name");
      ImGui::TableNextColumn();
      ImGui::Text("Description");

      const char* prevName=NULL;
      for (int i=0; i<256; i++) {
        const char* name=e->getEffectDesc(i,cursor.xCoarse);
        bool effectShow = true;
        if (name==prevName) {
          continue;
        }
        prevName=name;
        switch (fxColors[i]) {
          case GUI_COLOR_PATTERN_EFFECT_MISC: effectShow = effectShowMisc; break;
          case GUI_COLOR_PATTERN_EFFECT_SONG: effectShow = effectShowSong; break;
          case GUI_COLOR_PATTERN_EFFECT_SPEED: effectShow = effectShowSpeed; break;
          case GUI_COLOR_PATTERN_EFFECT_TIME: effectShow = effectShowTime; break;
          case GUI_COLOR_PATTERN_EFFECT_PITCH: effectShow = effectShowPitch; break;
          case GUI_COLOR_PATTERN_EFFECT_PANNING: effectShow = effectShowPanning; break;
          case GUI_COLOR_PATTERN_EFFECT_VOLUME: effectShow = effectShowVolume; break;
          case GUI_COLOR_PATTERN_EFFECT_SYS_PRIMARY: effectShow = effectShowSysPrimary; break;
          case GUI_COLOR_PATTERN_EFFECT_SYS_SECONDARY: effectShow = effectShowSysSecondary; break;
          default: effectShow = true; break;
        }
        if (name!=NULL && effectShow) {
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::PushFont(patFont);
          ImGui::PushStyleColor(ImGuiCol_Text,uiColors[fxColors[i]]);
          ImGui::Text("%c%c%c%c",name[0],name[1],name[2],name[3]);
          ImGui::PopStyleColor();
          ImGui::PopFont();

          ImGui::TableNextColumn();
          if (strlen(name)>6) {
            ImGui::TextWrapped("%s",&name[6]);
          } else {
            ImGui::Text("ERROR");
          }
        }
      }
      ImGui::EndTable();
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_EFFECT_LIST;
  ImGui::End();
}
