#include "gui.h"
#include "imgui.h"
#include "IconsFontAwesome4.h"
#include "misc/cpp/imgui_stdlib.h"

const char* queryModes[GUI_QUERY_MAX]={
  "ignore",
  "equals",
  "not equal",
  "between",
  "not between",
  "any",
  "none"
};

void FurnaceGUI::drawFindReplace() {
  if (nextWindow==GUI_WINDOW_FIND) {
    findOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!findOpen) return;
  ImGui::SetNextWindowSizeConstraints(ImVec2(64.0f*dpiScale,32.0f*dpiScale),ImVec2(scrW*dpiScale,scrH*dpiScale));
  if (ImGui::Begin("Find/Replace",&findOpen,globalWinFlags)) {
    if (curQuery.empty()) {
      curQuery.push_back(FurnaceGUIFindQuery());
    }
    ImGui::Text("Find");
    for (FurnaceGUIFindQuery& i: curQuery) {
      if (ImGui::BeginTable("FindRep",4,ImGuiTableFlags_BordersOuter)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.5);
        ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.25);
        ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthStretch,0.25);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Note");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::Combo("##NCondition",&i.noteMode,queryModes,GUI_QUERY_MAX);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Ins");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::Combo("##ICondition",&i.noteMode,queryModes,GUI_QUERY_MAX);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Volume");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::Combo("##VCondition",&i.noteMode,queryModes,GUI_QUERY_MAX);

        for (int j=0; j<i.effectCount; j++) {
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::Text("Effect");
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          ImGui::Combo("##ECondition",&i.noteMode,queryModes,GUI_QUERY_MAX);
          
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::Text("Value");
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          ImGui::Combo("##EVCondition",&i.noteMode,queryModes,GUI_QUERY_MAX);
        }
        ImGui::EndTable();
      }
    }

    if (ImGui::TreeNode("Replace")) {
      ImGui::Text("Replace controls here.");
      ImGui::TreePop();
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_FIND;
  ImGui::End();
}
