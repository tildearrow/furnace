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
      *(int*)data=conf->getInt(name, fallback?1:0);
      clampSetting(*(int*)data,0,1);
    }
    SettingDefCheckbox():
      data(NULL),
      name(""),
      friendlyName(""),
      tooltip(""),
      fallback(0) {}
    SettingDefCheckbox(int* _data, String _name, const char* _friendlyName, const char* _tooltip, bool _fallback) {
      data=_data;
      name=_name;
      friendlyName=_friendlyName;
      tooltip=_tooltip;
      fallback=_fallback;
    }
};

class SettingDefSliderInt : public SettingDef {
  void* data;
  String name;
  const char* friendlyName;
  const char* tooltip;

  int fallback;
  int minV, maxV;
  const char* sliderFmt;
  ImGuiSliderFlags f;
  public:
    void drawSetting(bool& changed) {
      if (ImGui::SliderInt(friendlyName, (int*)data, minV, maxV, sliderFmt, f)) {
        clampSetting(*(int*)data, minV, maxV);
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
      conf->set(name,*(int*)data);
    }
    void loadSetting(DivConfig* conf){
      *(int*)data=conf->getInt(name, fallback);
      clampSetting(*(int*)data,minV,maxV);
    }
    SettingDefSliderInt():
      data(NULL),
      name(""),
      friendlyName(""),
      tooltip(""),
      minV(0),
      maxV(0),
      sliderFmt(""),
      f(0) {}
    SettingDefSliderInt(int* _data, String _name, const char* _friendlyName, const char* _tooltip, bool _fallback, int min, int max):
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
    SettingDefSliderInt(int* _data, String _name, const char* _friendlyName, const char* _tooltip, bool _fallback, int min, int max, const char* fmt):
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
    SettingDefSliderInt(int* _data, String _name, const char* _friendlyName, const char* _tooltip, bool _fallback, int min, int max, const char* fmt, ImGuiSliderFlags flags) {
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
};

class SettingDefSliderFloat : public SettingDef {
  void* data;
  String name;
  const char* friendlyName;
  const char* tooltip;

  float fallback;
  float minV, maxV;
  const char* sliderFmt;
  ImGuiSliderFlags f;
  public:
    void drawSetting(bool& changed) {
      if (ImGui::SliderFloat(friendlyName, (float*)data, minV, maxV, sliderFmt, f)) {
        clampSetting(*(float*)data, minV, maxV);
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
      conf->set(name,*(float*)data);
    }
    void loadSetting(DivConfig* conf){
      *(float*)data=conf->getFloat(name, fallback);
      clampSetting(*(float*)data,minV,maxV);
    }
    SettingDefSliderFloat():
      data(NULL),
      name(""),
      friendlyName(""),
      tooltip(""),
      minV(0),
      maxV(0),
      sliderFmt(""),
      f(0) {}
    SettingDefSliderFloat(float* _data, String _name, const char* _friendlyName, const char* _tooltip, bool _fallback, float min, float max):
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
    SettingDefSliderFloat(float* _data, String _name, const char* _friendlyName, const char* _tooltip, bool _fallback, float min, float max, const char* fmt):
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
    SettingDefSliderFloat(float* _data, String _name, const char* _friendlyName, const char* _tooltip, bool _fallback, float min, float max, const char* fmt, ImGuiSliderFlags flags) {
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
};

class SettingDefInputInt : public SettingDef {
  void* data;
  String name;
  const char* friendlyName;
  const char* tooltip;

  int fallback;
  int minV, maxV;
  const char* sliderFmt;
  ImGuiInputTextFlags f;
  public:
    void drawSetting(bool& changed) {
      if (ImGui::InputScalar(friendlyName, ImGuiDataType_S32, (int*)data, NULL, NULL, sliderFmt, f)) {
        clampSetting(*(int*)data, minV, maxV);
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
      conf->set(name,*(int*)data);
    }
    void loadSetting(DivConfig* conf){
      *(int*)data=conf->getInt(name, fallback);
      clampSetting(*(int*)data,minV,maxV);
    }
    SettingDefInputInt():
      data(NULL),
      name(""),
      friendlyName(""),
      tooltip(""),
      minV(0),
      maxV(0),
      sliderFmt(""),
      f(0) {}
    SettingDefInputInt(int* _data, String _name, const char* _friendlyName, const char* _tooltip, bool _fallback, int min, int max):
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
    SettingDefInputInt(int* _data, String _name, const char* _friendlyName, const char* _tooltip, bool _fallback, int min, int max, const char* fmt):
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
    SettingDefInputInt(int* _data, String _name, const char* _friendlyName, const char* _tooltip, bool _fallback, int min, int max, const char* fmt, ImGuiInputTextFlags flags) {
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
};

class SettingDefDropdown : public SettingDef {
  void* data;
  String name;
  const char* friendlyName;
  const char* tooltip;

  int fallback;
  const char** options;
  int optionsCount;
  ImGuiComboFlags f;
  public:
    void drawSetting(bool& changed) {
      if (ImGui::BeginCombo(friendlyName,options[*(int*)data],f)) {
        for (unsigned short i=0; i<optionsCount; i++) {
          if (ImGui::Selectable(options[i],i==*(int*)data)) {
            *(int*)data=i;
            changed=true;
          }
        }
        ImGui::EndCombo();
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
      *(int*)data=conf->getInt(name, fallback?1:0);
      clampSetting(*(int*)data,0,1);
    }
    SettingDefDropdown():
      data(NULL),
      name(""),
      friendlyName(""),
      tooltip(""),
      fallback(0),
      options(NULL),
      optionsCount(0),
      f(0) {}
    SettingDefDropdown(int* _data, String _name, const char* _friendlyName, const char* _tooltip, int _fallback, const char** _options, int _optionsCount):
      f(0) {
      data=_data;
      name=_name;
      friendlyName=_friendlyName;
      tooltip=_tooltip;
      fallback=_fallback;
      options=_options;
      optionsCount=_optionsCount;
    }
    SettingDefDropdown(int* _data, String _name, const char* _friendlyName, const char* _tooltip, int _fallback, const char** _options, int _optionsCount, ImGuiComboFlags flags) {
      data=_data;
      name=_name;
      friendlyName=_friendlyName;
      tooltip=_tooltip;
      fallback=_fallback;
      options=_options;
      optionsCount=_optionsCount;
      f=flags;
    }
};

class SettingDefRadio : public SettingDef {
  void* data;
  String name;
  const char* friendlyName;
  const char* tooltip;

  int fallback;
  const char** options;
  int optionsCount;
  public:
    void drawSetting(bool& changed) {
      ImGui::Text("%s",friendlyName);
      ImGui::Indent();
      for (unsigned short i=0; i<optionsCount; i++) {
        if (ImGui::RadioButton(options[i],i==*(int*)data)) {
          *(int*)data=i;
          changed=true;
        }
      }
      ImGui::Unindent();
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
      *(int*)data=conf->getInt(name, fallback?1:0);
      clampSetting(*(int*)data,0,1);
    }
    SettingDefRadio():
      data(NULL),
      name(""),
      friendlyName(""),
      tooltip(""),
      fallback(0),
      options(NULL),
      optionsCount(0) {}
    SettingDefRadio(int* _data, String _name, const char* _friendlyName, const char* _tooltip, int _fallback, const char** _options, int _optionsCount) {
      data=_data;
      name=_name;
      friendlyName=_friendlyName;
      tooltip=_tooltip;
      fallback=_fallback;
      options=_options;
      optionsCount=_optionsCount;
    }
};

#endif
