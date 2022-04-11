#include "gui.h"
#include "../ta-log.h"
#include <chrono>
#include <imgui.h>

const char* logLevels[5]={
  "ERROR",
  "warning",
  "info",
  "debug",
  "trace"
};

FurnaceGUIColors logColors[5]={
  GUI_COLOR_LOGLEVEL_ERROR,
  GUI_COLOR_LOGLEVEL_WARNING,
  GUI_COLOR_LOGLEVEL_INFO,
  GUI_COLOR_LOGLEVEL_DEBUG,
  GUI_COLOR_LOGLEVEL_TRACE
};

void FurnaceGUI::drawLog() {
  if (nextWindow==GUI_WINDOW_LOG) {
    logOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!logOpen) return;
  if (ImGui::Begin("Log Viewer",&logOpen)) {
    ImGui::Text("Level");
    ImGui::SameLine();
    ImGui::Combo("##LogLevel",&logLevel,logLevels,5);
    if (ImGui::BeginTable("LogView",3,ImGuiTableFlags_ScrollY|ImGuiTableFlags_BordersInnerV)) {
      ImGui::PushFont(patFont);

      float timeChars=ImGui::CalcTextSize("00:00:00").x;
      float levelChars=ImGui::CalcTextSize("warning").x;

      ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed,timeChars);
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed,levelChars);
      ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch);

      ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
      ImGui::TableNextColumn();
      ImGui::TextUnformatted("time");
      ImGui::TableNextColumn();
      ImGui::TextUnformatted("level");
      ImGui::TableNextColumn();
      ImGui::TextUnformatted("message");

      int pos=logPosition;
      for (int i=0; i<TA_LOG_SIZE; i++) {
        const LogEntry& logEntry=logEntries[(pos+i)&(TA_LOG_SIZE-1)];
        if (!logEntry.ready) continue;
        if (logLevel<logEntry.loglevel) continue;
        ssize_t t=std::chrono::time_point_cast<std::chrono::seconds>(logEntry.time).time_since_epoch().count();
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        // this will fail on 32-bit :<
        ImGui::Text("%02ld:%02ld:%02ld",(t/3600)%24,(t/60)%60,t%60);
        ImGui::TableNextColumn();
        ImGui::TextColored(uiColors[logColors[logEntry.loglevel]],"%s",logLevels[logEntry.loglevel]);
        ImGui::TableNextColumn();
        ImGui::TextWrapped("%s",logEntry.text.c_str());
      }
      ImGui::PopFont();
      ImGui::EndTable();
    }
  }
  ImGui::End();
}