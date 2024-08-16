/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2024 tildearrow and contributors
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
#include "../ta-log.h"
#include <imgui.h>

const char* logLevels[5]={
  _N("ERROR"),
  _N("warning"),
  _N("info"),
  _N("debug"),
  _N("trace")
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
  if (ImGui::Begin("Log Viewer",&logOpen,globalWinFlags,_("Log Viewer"))) {
    ImGui::Checkbox(_("Follow"),&followLog);
    ImGui::SameLine();
    ImGui::Text(_("Level"));
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    ImGui::Combo("##LogLevel",&logLevel,LocalizedComboGetter,logLevels,5);
    if (ImGui::BeginTable("LogView",3,ImGuiTableFlags_ScrollY|ImGuiTableFlags_BordersInnerV)) {
      ImGui::PushFont(patFont);

      float timeChars=ImGui::CalcTextSize("00:00:00").x;
      float levelChars=ImGui::CalcTextSize("warning").x;

      ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed,timeChars);
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed,levelChars);
      ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch);

      ImGui::TableSetupScrollFreeze(0,1);

      ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(_("time"));
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(_("level"));
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(_("message"));

      int pos=logPosition;
      for (int i=0; i<TA_LOG_SIZE; i++) {
        const LogEntry& logEntry=logEntries[(pos+i)&(TA_LOG_SIZE-1)];
        if (!logEntry.ready) continue;
        if (logLevel<logEntry.loglevel) continue;
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("%02d:%02d:%02d",logEntry.time.tm_hour,logEntry.time.tm_min,logEntry.time.tm_sec);
        ImGui::TableNextColumn();
        ImGui::TextColored(uiColors[logColors[logEntry.loglevel]],"%s",_(logLevels[logEntry.loglevel]));
        ImGui::TableNextColumn();
        ImGui::TextWrapped("%s",logEntry.text.c_str());
      }
      ImGui::PopFont();

      if (followLog) {
        ImGui::SetScrollY(ImGui::GetScrollMaxY());
      }
      ImGui::EndTable();
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_LOG;
  ImGui::End();
}
