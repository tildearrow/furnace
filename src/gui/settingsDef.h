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

#ifndef SETTINGSDEF_H
#define SETTINGSDEF_H

#define SETTINGS_CHANGED settingsChanged=true;

#define clampSetting(x,minV,maxV) \
  if (x<minV) { \
    x=minV; \
  } \
  if (x>maxV) { \
    x=maxV; \
  }

#include "gui.h"
#include <imgui.h>

// abstract functions

void SettingDef::drawSetting(bool& changed) {
  ImGui::Text("%s",friendlyName);
  if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) ImGui::SetTooltip("%s",tooltip);
}

void SettingDef::saveSetting(DivConfig* conf) {
  conf->set(name,*(int*)data);
}

void SettingDef::loadSetting(DivConfig* conf) {
  *(int*)data=conf->getInt(name, 0);
}

// children

class SettingDefCheckbox : public SettingDef {
  void* data;
  String name;
  const char* friendlyName;
  const char* tooltip;

  bool fallback;
  public:
    void drawSetting(bool& changed) {
      if (ImGui::Checkbox(friendlyName, (bool*)data)) {
        changed=true;
      }
      if (tooltip) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.5f,0.5f,0.5f,0.9f),"(?)");
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
          ImGui::SetTooltip("%s",tooltip);
        }
      }
    }
    void saveSetting(DivConfig* conf) {
      conf->set(name,(*(bool*)data)?1:0);
    }
    void loadSetting(DivConfig* conf){
      *(bool*)data=conf->getInt(name, fallback?1:0);
      clampSetting(*(bool*)data,0,1);
    }
    SettingDefCheckbox():
      data(NULL),
      name(""),
      friendlyName(""),
      tooltip("") {}
    SettingDefCheckbox(int* _data, String _name, const char* _friendlyName, const char* _tooltip, bool _fallback) {
      data=_data;
      name=_name;
      friendlyName=_friendlyName;
      tooltip=_tooltip;
      fallback=_fallback;
    }
};

#endif
