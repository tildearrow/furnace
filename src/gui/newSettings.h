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

    bool drawSettings(ImGuiTextFilter* filter);
    bool categoryPassFilterRecursive(ImGuiTextFilter* filter);
    void drawSidebar(ImGuiTextFilter* filter, float* targetScrollPos);

    ~SettingsCategory();
};
