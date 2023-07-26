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
    ImGui::Text("Sort Effects:");
    ImGui::Checkbox("All",&effectShowAll);
    if (effectShowAll) {
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
    ImGui::Checkbox("Pitch",&effectShowPitch);
    ImGui::SameLine();
    ImGui::Checkbox("Song",&effectShowSong);
    ImGui::SameLine();
    ImGui::Checkbox("Time",&effectShowTime);
    ImGui::SameLine();
    ImGui::Checkbox("Speed",&effectShowSpeed);
    ImGui::SameLine();
    ImGui::Checkbox("Panning",&effectShowPanning);
    // ImGui::SameLine();
    ImGui::Checkbox("Volume",&effectShowVolume);
    ImGui::SameLine();
    ImGui::Checkbox("System (primary)",&effectShowSysPrimary);
    ImGui::SameLine();
    ImGui::Checkbox("System (secondary)",&effectShowSysSecondary);
    ImGui::SameLine();
    ImGui::Checkbox("Miscellanious",&effectShowMisc);

    effectShowAll = effectShowMisc && effectShowPanning && effectShowPitch && effectShowVolume && effectShowSong && effectShowTime && effectShowSpeed && effectShowSysPrimary && effectShowSysSecondary;
    
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
        // effectShow = effectShow && effectShowAll;
        if (name==prevName) {
          continue;
        }
        prevName=name;
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
