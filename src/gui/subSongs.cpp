#include "gui.h"
#include "imgui.h"
#include "IconsFontAwesome4.h"
#include "misc/cpp/imgui_stdlib.h"
#include "intConst.h"

void FurnaceGUI::drawSubSongs() {
  if (nextWindow==GUI_WINDOW_SUBSONGS) {
    subSongsOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!subSongsOpen) return;
  ImGui::SetNextWindowSizeConstraints(ImVec2(64.0f*dpiScale,32.0f*dpiScale),ImVec2(scrW*dpiScale,scrH*dpiScale));
  if (ImGui::Begin("Subsongs",&subSongsOpen,globalWinFlags)) {
    char id[1024];
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x-ImGui::GetFrameHeightWithSpacing()*2.0f-ImGui::GetStyle().ItemSpacing.x);
    if (e->curSubSong->name.empty()) {
      snprintf(id,1023,"%d. <no name>",(int)e->getCurrentSubSong()+1);
    } else {
      snprintf(id,1023,"%d. %s",(int)e->getCurrentSubSong()+1,e->curSubSong->name.c_str());
    }
    if (ImGui::BeginCombo("##SubSong",id)) {
      if (ImGui::BeginTable("SubSongSelection",2)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed);
        for (size_t i=0; i<e->song.subsong.size(); i++) {
          if (e->song.subsong[i]->name.empty()) {
            snprintf(id,1023,"%d. <no name>",(int)i+1);
          } else {
            snprintf(id,1023,"%d. %s",(int)i+1,e->song.subsong[i]->name.c_str());
          }
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          if (ImGui::Selectable(id,i==e->getCurrentSubSong())) {
            e->changeSongP(i);
            updateScroll(0);
            oldOrder=0;
            oldOrder1=0;
            oldRow=0;
            cursor.xCoarse=0;
            cursor.xFine=0;
            cursor.y=0;
            selStart=cursor;
            selEnd=cursor;
            curOrder=0;
          }
          ImGui::TableNextColumn();
          ImGui::PushID(i);
          if (ImGui::SmallButton(ICON_FA_ARROW_UP "##SubUp")) {
            e->moveSubSongUp(i);
          }
          ImGui::SameLine();
          if (ImGui::SmallButton(ICON_FA_ARROW_DOWN "##SubDown")) {
            e->moveSubSongDown(i);
          }
          ImGui::PopID();
        }
        ImGui::EndTable();
      }
      ImGui::EndCombo();
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_PLUS "##SubSongAdd")) {
      if (!e->addSubSong()) {
        showError("too many subsongs!");
      } else {
        e->changeSongP(e->song.subsong.size()-1);
        updateScroll(0);
        oldOrder=0;
        oldOrder1=0;
        oldRow=0;
        cursor.xCoarse=0;
        cursor.xFine=0;
        cursor.y=0;
        selStart=cursor;
        selEnd=cursor;
        curOrder=0;
        MARK_MODIFIED;
      }
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_MINUS "##SubSongDel")) {
      if (e->song.subsong.size()<=1) {
        showError("this is the only subsong!");
      } else {
        showWarning("are you sure you want to remove this subsong?",GUI_WARN_SUBSONG_DEL);
      }
    }

    ImGui::Text("Name");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::InputText("##SubSongName",&e->curSubSong->name)) {
      MARK_MODIFIED;
    }

    if (ImGui::BeginTable("OtherSubProps",3,ImGuiTableFlags_SizingStretchProp)) {
      ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed,0.0);
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0);
      ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.0);

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("TimeBase");
      ImGui::TableNextColumn();
      float avail=ImGui::GetContentRegionAvail().x;
      ImGui::SetNextItemWidth(avail);
      unsigned char realTB=e->curSubSong->timeBase+1;
      if (ImGui::InputScalar("##TimeBase",ImGuiDataType_U8,&realTB,&_ONE,&_THREE)) { MARK_MODIFIED
        if (realTB<1) realTB=1;
        if (realTB>16) realTB=16;
        e->curSubSong->timeBase=realTB-1;
      }
      ImGui::TableNextColumn();
      ImGui::Text("%.2f BPM",calcBPM(e->curSubSong->speed1,e->curSubSong->speed2,e->curSubSong->hz,e->curSubSong->virtualTempoN,e->curSubSong->virtualTempoD));

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("Speed");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      if (ImGui::InputScalar("##Speed1",ImGuiDataType_U8,&e->curSubSong->speed1,&_ONE,&_THREE)) { MARK_MODIFIED
        if (e->curSubSong->speed1<1) e->curSubSong->speed1=1;
        if (e->isPlaying()) play();
      }
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      if (ImGui::InputScalar("##Speed2",ImGuiDataType_U8,&e->curSubSong->speed2,&_ONE,&_THREE)) { MARK_MODIFIED
        if (e->curSubSong->speed2<1) e->curSubSong->speed2=1;
        if (e->isPlaying()) play();
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("Virtual Tempo");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      if (ImGui::InputScalar("##VTempoN",ImGuiDataType_S16,&e->curSubSong->virtualTempoN,&_ONE,&_THREE)) { MARK_MODIFIED
        if (e->curSubSong->virtualTempoN<1) e->curSubSong->virtualTempoN=1;
        if (e->curSubSong->virtualTempoN>255) e->curSubSong->virtualTempoN=255;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Numerator");
      }
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      if (ImGui::InputScalar("##VTempoD",ImGuiDataType_S16,&e->curSubSong->virtualTempoD,&_ONE,&_THREE)) { MARK_MODIFIED
        if (e->curSubSong->virtualTempoD<1) e->curSubSong->virtualTempoD=1;
        if (e->curSubSong->virtualTempoD>255) e->curSubSong->virtualTempoD=255;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Denominator (set to base tempo)");
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("Highlight");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      if (ImGui::InputScalar("##Highlight1",ImGuiDataType_U8,&e->curSubSong->hilightA,&_ONE,&_THREE)) {
        MARK_MODIFIED;
      }
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      if (ImGui::InputScalar("##Highlight2",ImGuiDataType_U8,&e->curSubSong->hilightB,&_ONE,&_THREE)) {
        MARK_MODIFIED;
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("Pattern Length");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      int patLen=e->curSubSong->patLen;
      if (ImGui::InputInt("##PatLength",&patLen,1,3)) { MARK_MODIFIED
        if (patLen<1) patLen=1;
        if (patLen>256) patLen=256;
        e->curSubSong->patLen=patLen;
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("Song Length");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      int ordLen=e->curSubSong->ordersLen;
      if (ImGui::InputInt("##OrdLength",&ordLen,1,3)) { MARK_MODIFIED
        if (ordLen<1) ordLen=1;
        if (ordLen>256) ordLen=256;
        e->curSubSong->ordersLen=ordLen;
        if (curOrder>=ordLen) {
          setOrder(ordLen-1);
        }
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      if (ImGui::Selectable(tempoView?"Base Tempo##TempoOrHz":"Tick Rate##TempoOrHz")) {
        tempoView=!tempoView;
      }
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      float setHz=tempoView?e->curSubSong->hz*2.5:e->curSubSong->hz;
      if (ImGui::InputFloat("##Rate",&setHz,1.0f,1.0f,"%g")) { MARK_MODIFIED
        if (tempoView) setHz/=2.5;
        if (setHz<10) setHz=10;
        if (setHz>999) setHz=999;
        e->setSongRate(setHz,setHz<52);
      }
      if (tempoView) {
        ImGui::TableNextColumn();
        ImGui::Text("= %gHz",e->curSubSong->hz);
      } else {
        if (e->curSubSong->hz>=49.98 && e->curSubSong->hz<=50.02) {
          ImGui::TableNextColumn();
          ImGui::Text("PAL");
        }
        if (e->curSubSong->hz>=59.9 && e->curSubSong->hz<=60.11) {
          ImGui::TableNextColumn();
          ImGui::Text("NTSC");
        }
      }

      ImGui::EndTable();
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_SUBSONGS;
  ImGui::End();
}
