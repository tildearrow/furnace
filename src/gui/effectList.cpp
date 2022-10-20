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
        if (name==prevName) {
          continue;
        }
        prevName=name;
        if (name!=NULL) {
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
