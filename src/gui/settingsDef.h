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

#include <cstdarg>
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

bool SettingDef::passesFilter(ImGuiTextFilter* filter, unsigned char toWhat) {
  return (filter->PassFilter(friendlyName) && (toWhat&1)) ||
         (filter->PassFilter(tooltip) && (toWhat&2));
}

void SettingDef::drawSetting(bool& changed) {
  ImGui::Text("%s",friendlyName);
  if (tooltip && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) ImGui::SetTooltip("%s",tooltip);
}

void SettingDef::saveSetting(DivConfig* conf) {
  conf->set(name,*(int*)data);
}

void SettingDef::loadSetting(DivConfig* conf) {
  *(int*)data=conf->getInt(name, 0);
}

SettingDef::~SettingDef() {
}

// children

class SettingCheckbox : public SettingDef {
  void* data;
  String name;
  const char* friendlyName;
  const char* tooltip;

  bool fallback;
  public:
    bool passesFilter(ImGuiTextFilter* filter, unsigned char toWhat) {
      return (filter->PassFilter(friendlyName) && toWhat&1) ||
             (filter->PassFilter(tooltip) && toWhat&2);
    }
    void drawSetting(bool& changed) {
      if (ImGui::Checkbox(friendlyName, (bool*)data)) {
        changed=true;
      }
      if (tooltip) {
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
          ImGui::SetTooltip("%s",tooltip);
        }
      }
    }
    void saveSetting(DivConfig* conf) {
      conf->set(name,(*(bool*)data)?1:0);
    }
    void loadSetting(DivConfig* conf){
      *(int*)data=conf->getInt(name, fallback?1:0);
      clampSetting(*(int*)data,0,1);
    }
    SettingCheckbox():
      data(NULL),
      name(""),
      friendlyName(""),
      tooltip(""),
      fallback(0) {}
    SettingCheckbox(int* _data, String _name, const char* _friendlyName, const char* _tooltip, bool _fallback) {
      data=_data;
      name=_name;
      friendlyName=_friendlyName;
      tooltip=_tooltip;
      fallback=_fallback;
    }
    ~SettingCheckbox() {
    }
};

class SettingSliderInt : public SettingDef {
  void* data;
  String name;
  const char* friendlyName;
  const char* tooltip;

  int fallback;
  int minV, maxV;
  const char* sliderFmt;
  ImGuiSliderFlags f;
  public:
    bool passesFilter(ImGuiTextFilter* filter, unsigned char toWhat) {
      return (filter->PassFilter(friendlyName) && toWhat&1) ||
             (filter->PassFilter(tooltip) && toWhat&2);
    }
    void drawSetting(bool& changed) {
      if (ImGui::SliderInt(friendlyName, (int*)data, minV, maxV, sliderFmt, f)) {
        clampSetting(*(int*)data, minV, maxV);
        changed=true;
      }
      if (tooltip) {
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
          ImGui::SetTooltip("%s",tooltip);
        }
      }
    }
    void saveSetting(DivConfig* conf) {
      conf->set(name,*(int*)data);
    }
    void loadSetting(DivConfig* conf){
      *(int*)data=conf->getInt(name, fallback);
      clampSetting(*(int*)data,minV,maxV);
    }
    SettingSliderInt():
      data(NULL),
      name(""),
      friendlyName(""),
      tooltip(""),
      minV(0),
      maxV(0),
      sliderFmt(""),
      f(0) {}
    SettingSliderInt(int* _data, String _name, const char* _friendlyName, const char* _tooltip, bool _fallback, int min, int max):
      sliderFmt("%d"),
      f(0) {
      data=_data;
      name=_name;
      friendlyName=_friendlyName;
      tooltip=_tooltip;
      fallback=_fallback;
      minV=min;
      maxV=max;
    }
    SettingSliderInt(int* _data, String _name, const char* _friendlyName, const char* _tooltip, bool _fallback, int min, int max, const char* fmt):
      f(0) {
      data=_data;
      name=_name;
      friendlyName=_friendlyName;
      tooltip=_tooltip;
      fallback=_fallback;
      minV=min;
      maxV=max;
      sliderFmt=fmt;
    }
    SettingSliderInt(int* _data, String _name, const char* _friendlyName, const char* _tooltip, bool _fallback, int min, int max, const char* fmt, ImGuiSliderFlags flags) {
      data=_data;
      name=_name;
      friendlyName=_friendlyName;
      tooltip=_tooltip;
      fallback=_fallback;
      minV=min;
      maxV=max;
      sliderFmt=fmt;
      f=flags;
    }
    ~SettingSliderInt() {
    }
};

class SettingSliderFloat : public SettingDef {
  void* data;
  String name;
  const char* friendlyName;
  const char* tooltip;

  float fallback;
  float minV, maxV;
  const char* sliderFmt;
  ImGuiSliderFlags f;
  public:
    bool passesFilter(ImGuiTextFilter* filter, unsigned char toWhat) {
      return (filter->PassFilter(friendlyName) && toWhat&1) ||
             (filter->PassFilter(tooltip) && toWhat&2);
    }
    void drawSetting(bool& changed) {
      if (ImGui::SliderFloat(friendlyName, (float*)data, minV, maxV, sliderFmt, f)) {
        clampSetting(*(float*)data, minV, maxV);
        changed=true;
      }
      if (tooltip) {
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
          ImGui::SetTooltip("%s",tooltip);
        }
      }
    }
    void saveSetting(DivConfig* conf) {
      conf->set(name,*(float*)data);
    }
    void loadSetting(DivConfig* conf){
      *(float*)data=conf->getFloat(name, fallback);
      clampSetting(*(float*)data,minV,maxV);
    }
    SettingSliderFloat():
      data(NULL),
      name(""),
      friendlyName(""),
      tooltip(""),
      minV(0),
      maxV(0),
      sliderFmt(""),
      f(0) {}
    SettingSliderFloat(float* _data, String _name, const char* _friendlyName, const char* _tooltip, bool _fallback, float min, float max):
      sliderFmt("%g"),
      f(0) {
      data=_data;
      name=_name;
      friendlyName=_friendlyName;
      tooltip=_tooltip;
      fallback=_fallback;
      minV=min;
      maxV=max;
    }
    SettingSliderFloat(float* _data, String _name, const char* _friendlyName, const char* _tooltip, bool _fallback, float min, float max, const char* fmt):
      f(0) {
      data=_data;
      name=_name;
      friendlyName=_friendlyName;
      tooltip=_tooltip;
      fallback=_fallback;
      minV=min;
      maxV=max;
      sliderFmt=fmt;
    }
    SettingSliderFloat(float* _data, String _name, const char* _friendlyName, const char* _tooltip, bool _fallback, float min, float max, const char* fmt, ImGuiSliderFlags flags) {
      data=_data;
      name=_name;
      friendlyName=_friendlyName;
      tooltip=_tooltip;
      fallback=_fallback;
      minV=min;
      maxV=max;
      sliderFmt=fmt;
      f=flags;
    }
    ~SettingSliderFloat() {
    }
};

class SettingInputInt : public SettingDef {
  void* data;
  String name;
  const char* friendlyName;
  const char* tooltip;

  int fallback;
  int minV, maxV;
  const char* sliderFmt;
  ImGuiInputTextFlags f;
  public:
    bool passesFilter(ImGuiTextFilter* filter, unsigned char toWhat) {
      return (filter->PassFilter(friendlyName) && toWhat&1) ||
             (filter->PassFilter(tooltip) && toWhat&2);
    }
    void drawSetting(bool& changed) {
      if (ImGui::InputScalar(friendlyName, ImGuiDataType_S32, (int*)data, NULL, NULL, sliderFmt, f)) {
        clampSetting(*(int*)data, minV, maxV);
        changed=true;
      }
      if (tooltip) {
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
          ImGui::SetTooltip("%s",tooltip);
        }
      }
    }
    void saveSetting(DivConfig* conf) {
      conf->set(name,*(int*)data);
    }
    void loadSetting(DivConfig* conf){
      *(int*)data=conf->getInt(name, fallback);
      clampSetting(*(int*)data,minV,maxV);
    }
    SettingInputInt():
      data(NULL),
      name(""),
      friendlyName(""),
      tooltip(""),
      minV(0),
      maxV(0),
      sliderFmt(""),
      f(0) {}
    SettingInputInt(int* _data, String _name, const char* _friendlyName, const char* _tooltip, bool _fallback, int min, int max):
      sliderFmt("%d"),
      f(0) {
      data=_data;
      name=_name;
      friendlyName=_friendlyName;
      tooltip=_tooltip;
      fallback=_fallback;
      minV=min;
      maxV=max;
    }
    SettingInputInt(int* _data, String _name, const char* _friendlyName, const char* _tooltip, bool _fallback, int min, int max, const char* fmt):
      f(0) {
      data=_data;
      name=_name;
      friendlyName=_friendlyName;
      tooltip=_tooltip;
      fallback=_fallback;
      minV=min;
      maxV=max;
      sliderFmt=fmt;
    }
    SettingInputInt(int* _data, String _name, const char* _friendlyName, const char* _tooltip, bool _fallback, int min, int max, const char* fmt, ImGuiInputTextFlags flags) {
      data=_data;
      name=_name;
      friendlyName=_friendlyName;
      tooltip=_tooltip;
      fallback=_fallback;
      minV=min;
      maxV=max;
      sliderFmt=fmt;
      f=flags;
    }
    ~SettingInputInt() {
    }
};

class SettingDropdown : public SettingDef {
  void* data;
  String name;
  const char* friendlyName;
  const char* tooltip;

  int fallback;
  const char** options;
  int optionsCount;
  ImGuiComboFlags f;
  std::function<void()> interactFunc;
  std::function<void()> setupFunc;
  public:
    bool passesFilter(ImGuiTextFilter* filter, unsigned char toWhat) {
      return (filter->PassFilter(friendlyName) && toWhat&1) ||
             (filter->PassFilter(tooltip) && toWhat&2);
    }
    void drawSetting(bool& changed) {
      setupFunc();
      if (ImGui::BeginCombo(friendlyName,options[*(int*)data],f)) {
        for (unsigned short i=0; i<optionsCount; i++) {
          if (ImGui::Selectable(options[i],i==*(int*)data)) {
            *(int*)data=i;
            changed=true;
            interactFunc();
          }
        }
        ImGui::EndCombo();
      }
      if (tooltip) {
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
          ImGui::SetTooltip("%s",tooltip);
        }
      }
    }
    void saveSetting(DivConfig* conf) {
      conf->set(name,(*(bool*)data)?1:0);
    }
    void loadSetting(DivConfig* conf){
      *(int*)data=conf->getInt(name, fallback?1:0);
      clampSetting(*(int*)data,0,1);
    }
    SettingDropdown():
      data(NULL),
      name(""),
      friendlyName(""),
      tooltip(""),
      fallback(0),
      options(NULL),
      optionsCount(0),
      f(0),
      interactFunc([]{}),
      setupFunc([]{}) {}
    SettingDropdown(int* _data, String _name, const char* _friendlyName, const char* _tooltip, int _fallback, const char** _options, int _optionsCount):
      f(0),
      interactFunc([]{}),
      setupFunc([]{}) {
      data=_data;
      name=_name;
      friendlyName=_friendlyName;
      tooltip=_tooltip;
      fallback=_fallback;
      options=_options;
      optionsCount=_optionsCount;
    }
    SettingDropdown(int* _data, String _name, const char* _friendlyName, const char* _tooltip, int _fallback, const char** _options, int _optionsCount, ImGuiComboFlags flags):
      interactFunc([]{}),
      setupFunc([]{}) {
      data=_data;
      name=_name;
      friendlyName=_friendlyName;
      tooltip=_tooltip;
      fallback=_fallback;
      options=_options;
      optionsCount=_optionsCount;
      f=flags;
    }
    SettingDropdown(int* _data, String _name, const char* _friendlyName, const char* _tooltip, int _fallback, const char** _options, int _optionsCount, ImGuiComboFlags flags, std::function<void()> _interactFunc):
      setupFunc([]{}) {
      data=_data;
      name=_name;
      friendlyName=_friendlyName;
      tooltip=_tooltip;
      fallback=_fallback;
      options=_options;
      optionsCount=_optionsCount;
      f=flags;
      interactFunc=_interactFunc;
    }
    SettingDropdown(int* _data, String _name, const char* _friendlyName, const char* _tooltip, int _fallback, const char** _options, int _optionsCount, ImGuiComboFlags flags, std::function<void()> _interactFunc, std::function<void()> _setupFunc) {
      data=_data;
      name=_name;
      friendlyName=_friendlyName;
      tooltip=_tooltip;
      fallback=_fallback;
      options=_options;
      optionsCount=_optionsCount;
      f=flags;
      interactFunc=_interactFunc;
      setupFunc=_setupFunc;
    }
    ~SettingDropdown() {
    }
};

class SettingRadio : public SettingDef {
  void* data;
  String name;
  const char* friendlyName;
  const char* tooltip;

  int fallback;
  const char** options;
  int optionsCount;
  public:
    bool passesFilter(ImGuiTextFilter* filter, unsigned char toWhat) {
      return (filter->PassFilter(friendlyName) && toWhat&1) ||
             (filter->PassFilter(tooltip) && toWhat&2);
    }
    void drawSetting(bool& changed) {
      ImGui::Text("%s",friendlyName);
      if (tooltip) {
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
          ImGui::SetTooltip("%s",tooltip);
        }
      }
      ImGui::Indent();
      for (unsigned short i=0; i<optionsCount; i++) {
        if (ImGui::RadioButton(options[i],i==*(int*)data)) {
          *(int*)data=i;
          changed=true;
        }
      }
      ImGui::Unindent();
    }
    void saveSetting(DivConfig* conf) {
      conf->set(name,(*(bool*)data)?1:0);
    }
    void loadSetting(DivConfig* conf){
      *(int*)data=conf->getInt(name, fallback?1:0);
      clampSetting(*(int*)data,0,1);
    }
    SettingRadio():
      data(NULL),
      name(""),
      friendlyName(""),
      tooltip(""),
      fallback(0),
      options(NULL),
      optionsCount(0) {}
    SettingRadio(int* _data, String _name, const char* _friendlyName, const char* _tooltip, int _fallback, const char** _options, int _optionsCount) {
      data=_data;
      name=_name;
      friendlyName=_friendlyName;
      tooltip=_tooltip;
      fallback=_fallback;
      options=_options;
      optionsCount=_optionsCount;
    }
    ~SettingRadio() {
    }
};

class SettingDummyText : public SettingDef {
  const char* fmt;
  va_list args;
  public:
    bool passesFilter(ImGuiTextFilter* filter, unsigned char toWhat) {
      (void)filter;
      (void)toWhat;
      return false;
    }
    void drawSetting(bool& changed) {
      (void)changed;
      ImGui::Text(fmt,args);
    }
    void saveSetting(DivConfig* conf) {
      (void)conf;
    }
    void loadSetting(DivConfig* conf) {
      (void)conf;
    }
    SettingDummyText():
      fmt(NULL) {}
    SettingDummyText(const char* _fmt, ...) {
      fmt=_fmt;
      va_start(args,_fmt);
      va_end(args);
    }
    ~SettingDummyText() {
    }
};

#endif
