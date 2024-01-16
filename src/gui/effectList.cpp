#include "gui.h"
#include "guiConst.h"
#include <imgui.h>
#include "IconsFontAwesome4.h"

void FurnaceGUI::drawEffectList() {
  if (nextWindow==GUI_WINDOW_EFFECT_LIST) {
    effectListOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!effectListOpen) return;
  ImGui::SetNextWindowSizeConstraints(ImVec2(60.0f*dpiScale,20.0f*dpiScale),ImVec2(canvasW,canvasH));
  if (ImGui::Begin("Effect List",&effectListOpen,globalWinFlags)) {
    float availB=ImGui::GetContentRegionAvail().x-ImGui::GetFrameHeightWithSpacing();
    if (availB>0) {
      ImGui::PushTextWrapPos(availB);
      ImGui::TextWrapped("Chip at cursor: %s",e->getSystemName(e->sysOfChan[cursor.xCoarse]));
      ImGui::PopTextWrapPos();
      ImGui::SameLine();
    }
    ImGui::Button(ICON_FA_BARS "##SortEffects");
    if (ImGui::BeginPopupContextItem("effectSort",ImGuiPopupFlags_MouseButtonLeft)) {
      for (int i=0; i<9; i++) {
        ImGui::PushStyleColor(ImGuiCol_Text,uiColors[fxColorsSort[i]]);
        ImGui::Checkbox(fxColorsNames[i],&effectsShow[i]);
        ImGui::PopStyleColor();
      }

      if (ImGui::Button("All")) memset(effectsShow,1,sizeof(bool)*10);
      ImGui::SameLine();
      if (ImGui::Button("None")) memset(effectsShow,0,sizeof(bool)*10);

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
        bool effectShow=true;
        if (name==prevName) {
          continue;
        }
        prevName=name;
        switch (fxColors[i]) {
          case GUI_COLOR_PATTERN_EFFECT_MISC:
            effectShow=effectsShow[8];
            break;
          case GUI_COLOR_PATTERN_EFFECT_SONG:
            effectShow=effectsShow[1];
            break;
          case GUI_COLOR_PATTERN_EFFECT_SPEED:
            effectShow=effectsShow[3];
            break;
          case GUI_COLOR_PATTERN_EFFECT_TIME:
            effectShow=effectsShow[2];
            break;
          case GUI_COLOR_PATTERN_EFFECT_PITCH:
            effectShow=effectsShow[0];
            break;
          case GUI_COLOR_PATTERN_EFFECT_PANNING:
            effectShow=effectsShow[4];
            break;
          case GUI_COLOR_PATTERN_EFFECT_VOLUME:
            effectShow=effectsShow[5];
            break;
          case GUI_COLOR_PATTERN_EFFECT_SYS_PRIMARY:
            effectShow=effectsShow[6];
            break;
          case GUI_COLOR_PATTERN_EFFECT_SYS_SECONDARY:
            effectShow=effectsShow[7];
            break;
          default:
            effectShow=true;
            break;
        }
        if (fxColors[i]==GUI_COLOR_PATTERN_EFFECT_PANNING) {
          DivDispatch* dispatch=e->getDispatch(e->dispatchOfChan[cursor.xCoarse]);
          if (dispatch!=NULL) {
            int outputs=dispatch->getOutputCount();
            if (outputs<2) {
              effectShow=false;
            }
            if (outputs<3) {
              if (i>=0x88 && i<=0x8f) {
                effectShow=false;
              }
            }
          }
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
