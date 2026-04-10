/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2026 tildearrow and contributors
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
#include <cstddef>
#include <imgui.h>
#include <imgui_internal.h>
// #include "newSettings.h"

SettingEntry::SettingEntry(const char* n, std::function<bool(void)> f) {
  name=n;
  drawFunction=f;
}

SettingEntry::SettingEntry(const SettingEntry& s) {
  name=s.name;
  drawFunction=s.drawFunction;
}

SettingsCategory::SettingsCategory():
  name(NULL),
  settings({}),
  children({}),
  scrollPos(0.0f) {}

SettingsCategory::SettingsCategory(const char* n, std::initializer_list<SettingEntry> s, std::initializer_list<SettingsCategory> c) {
  name=n;
  settings=s;
  children=c;
  scrollPos=0.0f;
}

SettingsCategory::SettingsCategory(const SettingsCategory& s) {
  name=s.name;
  settings=s.settings;
  children=s.children;
  scrollPos=s.scrollPos;
}

bool SettingsCategory::drawSettings(ImGuiTextFilter* filter) {
  if (!filter->IsActive())
    scrollPos=ImGui::GetCursorPosY();
  ImGui::SeparatorText(name);
  // ImGui::SameLine();
  // ImGui::TextColored(ImVec4(1.0f,.5f,.5f,1.f), "%f",scrollPos);
  bool ret=false;
  for (SettingEntry& s:settings) {
    if (filter->IsActive()) {
      if (!filter->PassFilter(s.name)) continue;
    }
    if (s.drawFunction()) ret=true;
  }
  ImGui::Indent();
    for (SettingsCategory& c:children) {
      if (c.drawSettings(filter)) ret=true;
    }
  ImGui::Unindent();
  return ret;
}

bool SettingsCategory::categoryPassFilterRecursive(ImGuiTextFilter* filter) {
  if (filter->PassFilter(name)) return true;
  for (SettingsCategory& c:children) {
    if (c.categoryPassFilterRecursive(filter)) return true;
  }
  return false;
}

void SettingsCategory::drawSidebar(ImGuiTextFilter* filter, float* targetScrollPos) {
  if (children.empty()) {
    if (filter->IsActive()) {
      if (!filter->PassFilter(name)) return;
    }
    if (ImGui::Selectable(name)) {
      *targetScrollPos=scrollPos;
      return;
    }
  } else {
    bool popDisabled=false;
    ImGuiTreeNodeFlags treeFlags=ImGuiTreeNodeFlags_None;
    if (filter->IsActive()) {
      if (!filter->PassFilter(name)) {
        if (!categoryPassFilterRecursive(filter)) return;
        else {
          ImGui::BeginDisabled();
          popDisabled=true;
        }
      }
      treeFlags|=ImGuiTreeNodeFlags_DefaultOpen;
    }
    if (ImGui::TreeNodeEx(name, treeFlags)) {
      popDisabled=false;
      ImGui::EndDisabled();
      for (SettingsCategory& c:children)
        c.drawSidebar(filter, targetScrollPos);
      ImGui::TreePop();
    }
    if (popDisabled) ImGui::EndDisabled();
    if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) { // hack because IsItemClicked returns true when anywhere inside the node and on the rising edge
      if (*targetScrollPos==-1.0f) *targetScrollPos=scrollPos; // TODO: only works when the treenode is closing
    }
  }
}

SettingsCategory::~SettingsCategory() {
  settings.clear();
  children.clear();
}

#define _S SettingEntry
#define _C(...) allSettings.push_back(SettingsCategory(__VA_ARGS__))
#define _CC SettingsCategory

void FurnaceGUI::initSettings() {
  _C("Audio", {
    _S("Low-latency mode", [this]{
      bool lowLatencyB=settings.lowLatency;
      if (ImGui::Checkbox(_("Low-latency mode"),&lowLatencyB)) {
        settings.lowLatency=lowLatencyB;
        return true;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(_("reduces latency by running the engine faster than the tick rate.\nuseful for live playback/jam mode.\n\nwarning: only enable if your buffer size is small (10ms or less)."));
      }
      return false;
    }),
    _S("Force mono audio", [this]{
      bool forceMonoB=settings.forceMono;
      if (ImGui::Checkbox(_("Force mono audio"),&forceMonoB)) {
        settings.forceMono=forceMonoB;
        return true;
      }
      return false;
    }),
  },{
    _CC("child", {
      _S("why",[]{return false;})
    },{})
  });
}
