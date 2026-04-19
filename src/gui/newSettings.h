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

#include "../engine/config.h"
#include "imgui.h"
#include <functional>
#include <initializer_list>

// user-defined function that gets called when the setting value gets changed
typedef std::function<void(void)> entryCallback;
// if none of the below types suit the setting, the draw function may be user-defined.
// return true on setting change
typedef std::function<bool(void)> boolReturnFunction;

#define falseLambda []{return false;}
#define trueLambda []{return true;}

// common setting types.
// some use external data (extData)
enum SettingType {
  SettingNone=0,
  SettingCheckbox,    // basic checkbox. no extData required
  SettingRadio,       // radio buttons. extData - array of SettingEntryMultiChoiceExtData<int>
  SettingComboInt,    // combo, where the setting value is an integer. extData - array of SettingEntryMultiChoiceExtData<int>
  SettingComboStr,    // combo, where the setting value is a string. extData - array of SettingEntryMultiChoiceExtData<String>
  SettingSliderFloat, // float slider. extData - pointer to NumericInputExtData<float>
  SettingSliderInt,   // int slider. extData - pointer to NumericInputExtData<int>
  SettingInputInt,    // int input. extData - pointer to NumericInputExtData<int>
  SettingInputStr,    // string input with optional file dialog. extData - pointer to SettingEntryTextInputExtData
  SettingColor,       // color picker. no extData required
  SettingKeybind,     // keybind input. no extData required

  SettingCustom       // user-defined type
};

// multi-choice setting value definition
// used by SettingRadio, SettingComboInt, SettingComboStr
template <typename T>
struct SettingEntryMultiChoiceExtData {
  // the text of the choice. if localized, use _N()
  const char* choice;
  // the value of the choice
  T value;
  // tooltip for the choice. set to NULL if unused
  const char* tooltip;
};

// numeric input definition
// used by SettingSliderFloat, SettingSliderInt, SettingInputInt
template <typename T>
struct SettingEntryNumericInputExtData {
  // minimum value of the input
  T min;
  // maximum value of the input
  T max;
  // optional, display format.
  // set to NULL if not used
  const char* fmt;
};

// text input definition
// used by SettingInputStr
struct SettingEntryTextInputExtData {
  // hint text inside text input
  // set to NULL if unused
  const char* hint;
  // whether the input is for a path. displays a file dialog button if true
  bool isPathInput;
  // the callback function of the file dialog. has to de defined if above is true
  entryCallback dialogCallback;
};

// SettingEntry class
// defines a setting
class SettingEntry {
  // setting type, see the SettingType enum for all the available types
  SettingType type;
  // the text that will be displayed and searched for
  const char* label;
  // the hover tooltip for the setting. set to NULL if unused
  const char* tooltip;
  // the setting name in the config file
  const char* confName;
  // pointer to the setting value (a member of FurnaceGUI::settings or FurnaceGUI::uiColors)
  void* value;
  // external data for the setting. different type have different data.
  // these pointers are deleted by destroy()
  void* extData; int extDataCount;
  // external condition furnace for when to display the setting
  boolReturnFunction settingCondition;
  // callback function that runs when the setting is interacted with
  entryCallback callback;
  // only used for SettingCustom
  boolReturnFunction customDrawFunction;

  // helper functions for the setting value
  template<typename T> T getValue() {return *(T*)value;}
  template<typename T> void setValue(T v) {*(T*)value=v;}

  public:
    SettingEntry();
    SettingEntry(SettingType t, const char* l, const char* n, void* v, void* x=NULL, int xn=0, const char* d=NULL, boolReturnFunction b=trueLambda, entryCallback f=[]{});
    SettingEntry(const char* l, const char* n, boolReturnFunction f, boolReturnFunction b=trueLambda);

    // setting definition functions
    static SettingEntry Checkbox(const char* label, const char* confName, int* value) {
      return SettingEntry(SettingCheckbox,label,confName,value);
    }
    static SettingEntry Radio(const char* label, const char* confName, int* value, std::initializer_list<SettingEntryMultiChoiceExtData<int>> entries) {
      SettingEntryMultiChoiceExtData<int>* data=new SettingEntryMultiChoiceExtData<int>[entries.size()];
      SettingEntryMultiChoiceExtData<int>* dataIter=data;
      for (auto i:entries) {
        *dataIter++=i;
      }
      return SettingEntry(SettingRadio,label,confName,value,data, entries.size());
    }
    static SettingEntry ComboInt(const char* label, const char* confName, int* value, std::initializer_list<SettingEntryMultiChoiceExtData<int>> entries) {
      SettingEntryMultiChoiceExtData<int>* data=new SettingEntryMultiChoiceExtData<int>[entries.size()];
      SettingEntryMultiChoiceExtData<int>* dataIter=data;
      for (auto i:entries) {
        *dataIter++=i;
      }
      return SettingEntry(SettingComboInt,label,confName,value,data, entries.size());
    }
    static SettingEntry ComboString(const char* label, const char* confName, String* value, std::initializer_list<SettingEntryMultiChoiceExtData<const char*>> entries) {
      SettingEntryMultiChoiceExtData<const char*>* data=new SettingEntryMultiChoiceExtData<const char*>[entries.size()];
      SettingEntryMultiChoiceExtData<const char*>* dataIter=data;
      for (auto i:entries) {
        *dataIter++=i;
      }
      return SettingEntry(SettingComboStr,label,confName,value,data, entries.size());
    }
    static SettingEntry SliderFloat(const char* label, const char* confName, float* value, SettingEntryNumericInputExtData<float> limits) {
      SettingEntryNumericInputExtData<float>* data=new SettingEntryNumericInputExtData<float>;
      *data=limits;
      return SettingEntry(SettingSliderFloat,label,confName,value,data,1);
    }
    static SettingEntry SliderInt(const char* label, const char* confName, int* value, SettingEntryNumericInputExtData<int> limits) {
      SettingEntryNumericInputExtData<int>* data=new SettingEntryNumericInputExtData<int>;
      *data=limits;
      return SettingEntry(SettingSliderInt,label,confName,value,data,1);
    }
    static SettingEntry InputInt(const char* label, const char* confName, int* value, SettingEntryNumericInputExtData<int> limits) {
      SettingEntryNumericInputExtData<int>* dataPtr=new SettingEntryNumericInputExtData<int>;
      *dataPtr=limits;
      return SettingEntry(SettingInputInt,label,confName,value,dataPtr,1);
    }
    static SettingEntry InputText(const char* label, const char* confName, String* value, const char* hint=NULL) {
      SettingEntryTextInputExtData* dataPtr=new SettingEntryTextInputExtData;
      dataPtr->hint=hint;
      dataPtr->isPathInput=false;
      return SettingEntry(SettingInputStr,label,confName,value,dataPtr,1);
    }
    static SettingEntry Path(const char* label, const char* confName, String* value, entryCallback dialogCallback, const char* hint=NULL) {
      SettingEntryTextInputExtData* dataPtr=new SettingEntryTextInputExtData;
      dataPtr->hint=hint;
      dataPtr->isPathInput=true;
      dataPtr->dialogCallback=dialogCallback;
      return SettingEntry(SettingInputStr,label,confName,value,dataPtr,1);
    }
    static SettingEntry Color(const char* label, const char* confName, ImVec4* value) {
      return SettingEntry(SettingColor,label,confName,value);
    }

    // misc. data functions
    SettingEntry& Tooltip(const char* text) {
      tooltip=text;
      return *this;
    }
    SettingEntry& Condition(boolReturnFunction f) {
      settingCondition=f;
      return *this;
    }
    SettingEntry& Callback(entryCallback f) {
      callback=f;
      return *this;
    }

    bool draw();
    bool passesFilter(ImGuiTextFilter* filter);

    void loadConf(DivConfig& conf);
    void saveConf(DivConfig& conf);

    void destroy();
};

class SettingsCategory {
  const char* name;
  std::vector<SettingEntry> settings;
  std::vector<SettingsCategory> children;
  float scrollPos;
  public:
    SettingsCategory();
    SettingsCategory(const char* n, std::initializer_list<SettingEntry> s);
    SettingsCategory(const char* n, std::initializer_list<SettingEntry> s, std::initializer_list<SettingsCategory> c);
    SettingsCategory(const SettingsCategory& s);

    bool drawSettings(ImGuiTextFilter* filter, bool doFilter);
    bool categoryPassFilterRecursive(ImGuiTextFilter* filter);
    bool drawSidebar(ImGuiTextFilter* filter, float* targetScrollPos);

    void deleteRecursive();
};

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

#define UI_COLOR_CONFIG(what) \
  ImGui::PushID(what); \
  if (ImGui::ColorEdit4(_(guiColors[what].friendlyName),(float*)&uiColors[what])) { \
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
