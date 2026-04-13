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

#ifndef NEW_SETTINGS_H
#define NEW_SETTINGS_H

#include "imgui.h"
#include <functional>
#include <initializer_list>

struct SettingEntry {
  const char* name;
  std::function<bool(void)> drawFunction;
  SettingEntry(const char* n, std::function<bool(void)> f);
  SettingEntry(const SettingEntry& s);
};

class SettingsCategory {
  const char* name;
  std::vector<SettingEntry> settings;
  std::vector<SettingsCategory> children;
  float scrollPos;
  public:
    SettingsCategory();
    SettingsCategory(const char* n, std::initializer_list<SettingEntry> s, std::initializer_list<SettingsCategory> c);
    SettingsCategory(const SettingsCategory& s);

    bool drawSettings(ImGuiTextFilter* filter, bool doFilter);
    bool categoryPassFilterRecursive(ImGuiTextFilter* filter);
    bool drawSidebar(ImGuiTextFilter* filter, float* targetScrollPos);

    ~SettingsCategory();
};

extern const char* locales[][3];
extern const char* fontBackends[];
extern const char* mainFonts[];
extern const char* headFonts[];
extern const char* patFonts[];
extern const char* audioBackends[];
extern const char* audioQualities[];
extern const char* arcadeCores[];
extern const char* ym2612Cores[];
extern const char* snCores[];
extern const char* nesCores[];
extern const char* c64Cores[];
extern const char* pokeyCores[];
extern const char* opnCores[];
extern const char* opl2Cores[];
extern const char* opl3Cores[];
extern const char* opl4Cores[];
extern const char* esfmCores[];
extern const char* opllCores[];
extern const char* ayCores[];
extern const char* swanCores[];
extern const char* coreQualities[];
extern const char* pcSpeakerOutMethods[];
extern const char* valueInputStyles[];
extern const char* valueSInputStyles[];
extern const char* messageTypes[];
extern const char* specificControls[];

#define SETTING_CHECKBOX(name,setting) \
  SettingEntry(_N(name),[this] { \
    bool B##setting=settings.setting; \
    if (ImGui::Checkbox(_(name),&B##setting)) { \
      settings.setting=B##setting; \
      return true; \
    } \
    return false; \
  })

#define SETTING_RADIO(name,setting) \
  SettingEntry(_N(name),[this] { \
    ImGui::Text(_(name)); \
    ImGui::Indent(); \
    bool ret=false; \

#define SETTING_RADIO_BUTTON(name,setting,v) \
    if (ImGui::RadioButton(_(name),settings.setting==v)) { \
      settings.setting=v; \
      ret=true; \
    }

#define SETTING_RADIO_END \
    ImGui::Unindent(); \
    return ret; \
  })

#define SAMPLE_RATE_SELECTABLE(x) \
  if (ImGui::Selectable(#x,settings.audioRate==x)) { \
    settings.audioRate=x; \
    settingsChanged=true; \
  }

#define BUFFER_SIZE_SELECTABLE(x) \
  if (ImGui::Selectable(#x,settings.audioBufSize==x)) { \
    settings.audioBufSize=x; \
    settingsChanged=true; \
  }

#define UI_COLOR_CONFIG(what,label) \
  ImGui::PushID(what); \
  if (ImGui::ColorEdit4(label,(float*)&uiColors[what])) { \
    applyUISettings(false); \
    settingsChanged=true; \
  } \
  ImGui::PopID();

#define KEYBIND_CONFIG_BEGIN(id) \
  if (ImGui::BeginTable(id,2,ImGuiTableFlags_SizingFixedFit|ImGuiTableFlags_NoHostExtendX|ImGuiTableFlags_NoClip)) {

#define KEYBIND_CONFIG_END \
    ImGui::EndTable(); \
  }

#define CORE_QUALITY(_name,_play,_render) \
  ImGui::TableNextRow(); \
  ImGui::TableNextColumn(); \
  ImGui::AlignTextToFramePadding(); \
  ImGui::Text(_name); \
  ImGui::TableNextColumn(); \
  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x); \
  if (ImGui::Combo("##" _name "Q",&settings._play,LocalizedComboGetter,coreQualities,6)) settingsChanged=true; \
  ImGui::TableNextColumn(); \
  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x); \
  if (ImGui::Combo("##" _name "QR",&settings._render,LocalizedComboGetter,coreQualities,6)) settingsChanged=true;

#endif
