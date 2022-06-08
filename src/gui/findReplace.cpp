#include "gui.h"
#include "imgui.h"
#include "IconsFontAwesome4.h"
#include "misc/cpp/imgui_stdlib.h"
#include "guiConst.h"

const char* queryModes[GUI_QUERY_MAX]={
  "ignore",
  "equals",
  "not equal",
  "between",
  "not between",
  "any",
  "none"
};

const char* queryReplaceModes[GUI_QUERY_REPLACE_MAX]={
  "set",
  "add",
  "clear"
};

#define FIRST_VISIBLE(x) (x==GUI_QUERY_MATCH || x==GUI_QUERY_MATCH_NOT || x==GUI_QUERY_RANGE || x==GUI_QUERY_RANGE_NOT)
#define SECOND_VISIBLE(x) (x==GUI_QUERY_RANGE || x==GUI_QUERY_RANGE_NOT)

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
    int index=0;
    int eraseIndex=-1;
    char tempID[1024];
    for (FurnaceGUIFindQuery& i: curQuery) {
      if (ImGui::BeginTable("FindRep",4,ImGuiTableFlags_BordersOuter)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.5);
        ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.25);
        ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthStretch,0.25);
        ImGui::PushID(index);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Note");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::Combo("##NCondition",&i.noteMode,queryModes,GUI_QUERY_MAX);
        ImGui::TableNextColumn();
        if (FIRST_VISIBLE(i.noteMode)) {
          if ((i.noteMode==GUI_QUERY_RANGE || i.noteMode==GUI_QUERY_RANGE_NOT) && i.note>=120) {
            i.note=0;
          }
          if (i.note==130) {
            snprintf(tempID,1024,"REL");
          } else if (i.note==129) {
            snprintf(tempID,1024,"===");
          } else if (i.note==128) {
            snprintf(tempID,1024,"OFF");
          } else if (i.note>=-60 && i.note<120) {
            snprintf(tempID,1024,"%s",noteNames[i.note+60]);
          } else {
            snprintf(tempID,1024,"???");
            i.note=0;
          }
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::BeginCombo("##NN1",tempID)) {
            for (int j=0; j<180; j++) {
              snprintf(tempID,1024,"%s",noteNames[j]);
              if (ImGui::Selectable(tempID,i.note==(j-60))) {
                i.note=j-60;
              }
            }
            if (i.noteMode!=GUI_QUERY_RANGE && i.noteMode!=GUI_QUERY_RANGE_NOT) {
              if (ImGui::Selectable("OFF",i.note==128)) {
                i.note=128;
              }
              if (ImGui::Selectable("===",i.note==129)) {
                i.note=129;
              }
              if (ImGui::Selectable("REL",i.note==130)) {
                i.note=130;
              }
            }
            ImGui::EndCombo();
          }
        }
        ImGui::TableNextColumn();
        if (SECOND_VISIBLE(i.noteMode)) {
          if (i.noteMax<-60 || i.noteMax>=120) {
            i.noteMax=0;
          }
          if (i.noteMax>=-60 && i.noteMax<120) {
            snprintf(tempID,1024,"%s",noteNames[i.noteMax+60]);
          } else {
            snprintf(tempID,1024,"???");
          }
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::BeginCombo("##NN2",tempID)) {
            for (int j=0; j<180; j++) {
              snprintf(tempID,1024,"%s",noteNames[j]);
              if (ImGui::Selectable(tempID,i.noteMax==(j-60))) {
                i.noteMax=j-60;
              }
            }
            ImGui::EndCombo();
          }
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Ins");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::Combo("##ICondition",&i.insMode,queryModes,GUI_QUERY_MAX);
        ImGui::TableNextColumn();
        if (FIRST_VISIBLE(i.insMode)) {
          snprintf(tempID,1024,"%.2X",i.ins);
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::BeginCombo("II1",tempID)) {
            for (int j=0; j<256; j++) {
              snprintf(tempID,1024,"%.2X",j);
              if (ImGui::Selectable(tempID,i.ins==j)) {
                i.ins=j;
              }
            }
            ImGui::EndCombo();
          }
        }
        ImGui::TableNextColumn();
        if (SECOND_VISIBLE(i.insMode)) {
          snprintf(tempID,1024,"%.2X",i.insMax);
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::BeginCombo("II2",tempID)) {
            for (int j=0; j<256; j++) {
              snprintf(tempID,1024,"%.2X",j);
              if (ImGui::Selectable(tempID,i.insMax==j)) {
                i.insMax=j;
              }
            }
            ImGui::EndCombo();
          }
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Volume");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::Combo("##VCondition",&i.volMode,queryModes,GUI_QUERY_MAX);
        ImGui::TableNextColumn();
        if (FIRST_VISIBLE(i.volMode)) {
          snprintf(tempID,1024,"%.2X",i.vol);
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::BeginCombo("VV1",tempID)) {
            for (int j=0; j<256; j++) {
              snprintf(tempID,1024,"%.2X",j);
              if (ImGui::Selectable(tempID,i.vol==j)) {
                i.vol=j;
              }
            }
            ImGui::EndCombo();
          }
        }
        ImGui::TableNextColumn();
        if (SECOND_VISIBLE(i.volMode)) {
          snprintf(tempID,1024,"%.2X",i.volMax);
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::BeginCombo("VV2",tempID)) {
            for (int j=0; j<256; j++) {
              snprintf(tempID,1024,"%.2X",j);
              if (ImGui::Selectable(tempID,i.volMax==j)) {
                i.volMax=j;
              }
            }
            ImGui::EndCombo();
          }
        }

        for (int j=0; j<i.effectCount; j++) {
          ImGui::PushID(0x1000+j);
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::Text("Effect");
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          ImGui::Combo("##ECondition",&i.effectMode[j],queryModes,GUI_QUERY_MAX);
          ImGui::TableNextColumn();
          if (FIRST_VISIBLE(i.effectMode[j])) {
            snprintf(tempID,1024,"%.2X",i.effect[j]);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::BeginCombo("EE1",tempID)) {
              for (int k=0; k<256; k++) {
                snprintf(tempID,1024,"%.2X",k);
                if (ImGui::Selectable(tempID,i.effect[j]==k)) {
                  i.effect[j]=k;
                }
              }
              ImGui::EndCombo();
            }
          }
          ImGui::TableNextColumn();
          if (SECOND_VISIBLE(i.effectMode[j])) {
            snprintf(tempID,1024,"%.2X",i.effectMax[j]);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::BeginCombo("EE2",tempID)) {
              for (int k=0; k<256; k++) {
                snprintf(tempID,1024,"%.2X",k);
                if (ImGui::Selectable(tempID,i.effectMax[j]==k)) {
                  i.effectMax[j]=k;
                }
              }
              ImGui::EndCombo();
            }
          }
          
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::Text("Value");
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          ImGui::Combo("##EVCondition",&i.effectValMode[j],queryModes,GUI_QUERY_MAX);
          ImGui::TableNextColumn();
          if (FIRST_VISIBLE(i.effectValMode[j])) {
            snprintf(tempID,1024,"%.2X",i.effectVal[j]);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::BeginCombo("EV1",tempID)) {
              for (int k=0; k<256; k++) {
                snprintf(tempID,1024,"%.2X",k);
                if (ImGui::Selectable(tempID,i.effectVal[j]==k)) {
                  i.effectVal[j]=k;
                }
              }
              ImGui::EndCombo();
            }
          }
          ImGui::TableNextColumn();
          if (SECOND_VISIBLE(i.effectValMode[j])) {
            snprintf(tempID,1024,"%.2X",i.effectValMax[j]);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::BeginCombo("EV2",tempID)) {
              for (int k=0; k<256; k++) {
                snprintf(tempID,1024,"%.2X",k);
                if (ImGui::Selectable(tempID,i.effectValMax[j]==k)) {
                  i.effectValMax[j]=k;
                }
              }
              ImGui::EndCombo();
            }
          }

          ImGui::PopID();
        }
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Button(ICON_FA_MINUS "##DelQuery")) {
          eraseIndex=index;
        }
        ImGui::TableNextColumn();
        if (i.effectCount<8) {
          if (ImGui::Button("Add effect")) {
            i.effectCount++;
          }
        }
        ImGui::TableNextColumn();
        if (i.effectCount>0) {
          if (ImGui::Button("Remove effect")) {
            i.effectCount--;
          }
        }
        ImGui::PopID();
        ImGui::EndTable();
      }
      index++;
    }
    if (ImGui::Button("Find")) {

    }
    ImGui::SameLine();
    if (eraseIndex>=0) {
      curQuery.erase(curQuery.begin()+eraseIndex);
    }
    if (ImGui::Button(ICON_FA_PLUS "##AddQuery")) {
      curQuery.push_back(FurnaceGUIFindQuery());
    }

    if (ImGui::BeginTable("QueryLimits",2)) {
      ImGui::TableNextRow();
      ImGui::TableNextColumn();

      ImGui::Text("Search range:");

      if (ImGui::RadioButton("Song",curQueryRangeY==0)) {
        curQueryRangeY=0;
      }
      if (ImGui::RadioButton("Selection",curQueryRangeY==1)) {
        curQueryRangeY=1;
      }
      if (ImGui::RadioButton("Pattern",curQueryRangeY==2)) {
        curQueryRangeY=2;
      }

      ImGui::TableNextColumn();
      ImGui::Checkbox("Confine to channels",&curQueryRangeX);

      ImGui::BeginDisabled(!curQueryRangeX);
      snprintf(tempID,1024,"%d: %s",curQueryRangeXMin+1,e->getChannelName(curQueryRangeXMin));
      if (ImGui::BeginCombo("From",tempID)) {
        for (int i=0; i<e->getTotalChannelCount(); i++) {
          snprintf(tempID,1024,"%d: %s",i+1,e->getChannelName(i));
          if (ImGui::Selectable(tempID,curQueryRangeXMin==i)) {
            curQueryRangeXMin=i;
          }
        }
        ImGui::EndCombo();
      }

      snprintf(tempID,1024,"%d: %s",curQueryRangeXMax+1,e->getChannelName(curQueryRangeXMax));
      if (ImGui::BeginCombo("To",tempID)) {
        for (int i=0; i<e->getTotalChannelCount(); i++) {
          snprintf(tempID,1024,"%d: %s",i+1,e->getChannelName(i));
          if (ImGui::Selectable(tempID,curQueryRangeXMax==i)) {
            curQueryRangeXMax=i;
          }
        }
        ImGui::EndCombo();
      }
      ImGui::EndDisabled();
      
      ImGui::EndTable();
    }

    if (ImGui::TreeNode("Replace")) {
      if (ImGui::BeginTable("QueryReplace",3)) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("...");
        ImGui::EndTable();
      }
      if (ImGui::Button("Replace##QueryReplace")) {
        // TODO
      }
      ImGui::TreePop();
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_FIND;
  ImGui::End();
}
