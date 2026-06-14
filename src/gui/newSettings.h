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

#include "../ta-utils.h"
#include "imgui.h"
#include <functional>
#include <initializer_list>

class FurnaceGUI;

// user-defined function that gets called when the setting value gets changed
typedef std::function<void(void)> entryCallback;
// if none of the below types suit the setting, the draw function may be user-defined.
// return true on setting change
typedef std::function<bool(void)> boolReturnFunction;

#define falseLambda []{return false;}
#define trueLambda []{return true;}

// keybinds vector
typedef std::vector<int> keybindList;

// common setting types.
// some use external data (passed via extData)
enum SettingType {
  SettingNone=0,
  SettingCheckbox,    // basic checkbox. no extData required
  SettingInvCheckbox, // save as above but inverted
  SettingRadio,       // radio buttons. extData - array of SettingEntryMultiChoiceExtData<int>
  SettingComboInt,    // combo, where the setting value is an integer. extData - array of SettingEntryMultiChoiceExtData<int>
  SettingComboStr,    // combo, where the setting value is a string. extData - array of SettingEntryMultiChoiceExtData<String>
  SettingSliderFloat, // float slider. extData - pointer to NumericInputExtData<float>
  SettingSliderInt,   // int slider. extData - pointer to NumericInputExtData<int>
  SettingInputInt,    // int input. extData - pointer to NumericInputExtData<int>
  SettingInputStr,    // string input with optional file dialog. extData - pointer to SettingEntryTextInputExtData
  SettingColor,       // color picker. no extData required
  SettingKeybind,     // keybind input. extData - pointer to the action number (int)

  SettingCustom       // user-defined type
};

// below are the external data structs

// multi-choice setting value definition
// used by SettingRadio, SettingComboInt, SettingComboStr
template <typename T>
struct SettingEntryMultiChoiceExtData {
  // the text of the choice. if localized, use _N()
  const char* choice;
  // the value of the choice
  T value;
  // (optional) tooltip for the choice
  const char* tooltip;
  SettingEntryMultiChoiceExtData():
    choice(NULL),
    value(T()),
    tooltip(NULL) {}
  SettingEntryMultiChoiceExtData(const char* c, T v, const char* t=NULL):
    choice(c),
    value(v),
    tooltip(t) {}
};

// numeric input definition
// used by SettingSliderFloat, SettingSliderInt, SettingInputInt
template <typename T>
struct SettingEntryNumericInputExtData {
  // minimum and maximum value of the input
  T minV, maxV;
  // (optional) input step sizes
  T step, stepFast;
  // (optional) display format
  const char* fmt;
  SettingEntryNumericInputExtData():
    minV(0), maxV(0),
    step(1), stepFast(16),
    fmt(NULL) {}
  SettingEntryNumericInputExtData(T _min, T _max):
    minV(_min), maxV(_max),
    step(1), stepFast(16),
    fmt(NULL) {}
  SettingEntryNumericInputExtData(T _min, T _max, const char* _fmt):
    minV(_min), maxV(_max),
    step(1), stepFast(16),
    fmt(_fmt) {}
  SettingEntryNumericInputExtData(T _min, T _max, T s, T sf, const char* _fmt=NULL):
    minV(_min), maxV(_max),
    step(s), stepFast(sf),
    fmt(_fmt) {}
};

// text input definition
// used by SettingInputStr
struct SettingEntryTextInputExtData {
  // hint text inside text input
  const char* hint;
  // which dialog to open. set to -1 if not a file dialog input
  int dialogNum;
  SettingEntryTextInputExtData():
    hint(NULL),
    dialogNum(-1) {}
  SettingEntryTextInputExtData(const char* h, int d=-1):
    hint(h),
    dialogNum(d) {}
  SettingEntryTextInputExtData(int d):
    hint(NULL),
    dialogNum(-1) {}
};

// SettingEntry class
// defines a setting
class SettingEntry {
  // setting type, see the SettingType enum for all the available types
  SettingType type;
  // the text that will be displayed and searched for
  // note: SettingCustom does not draw the label.
  const char* label;
  // the hover tooltip for the setting. set to NULL if unused
  const char* tooltip;
  // the setting name in the config file (currently unused)
  const char* confName;
  // pointer to the setting value (a member of FurnaceGUI::settings, FurnaceGUI::uiColors, FurnaceGUI::actionKeys)
  void* value;
  // external data for the setting. different types have different data.
  // these pointers are deleted by destroy()
  void* extData; int extDataCount;
  // external condition lambda for when to display the setting
  boolReturnFunction settingCondition;
  // callback function that runs when the setting value is changed
  entryCallback callback;
  // only used for SettingCustom, the draw function of the setting
  boolReturnFunction customDrawFunction;

  // helper functions for the setting value
  template<typename T> T getValue() {return *(T*)value;}
  template<typename T> void setValue(T v) {*(T*)value=v;}

  public:
    SettingEntry();
    SettingEntry(SettingType t, const char* l, const char* n, void* v, void* x=NULL, int xn=0, const char* d=NULL, boolReturnFunction b=trueLambda, entryCallback f=[]{});
    /** this constructor is used for custom settings
     * @param l the label of the setting
     * @param n the config key of the setting
     * @param f the custom drawing function (std::function<bool(void)>). shall return true on setting change.
     * @param b the setting condition lambda (std::function<bool(void)>). the setting will be drawn if said lambda returns true.
     */
    SettingEntry(const char* l, const char* n, boolReturnFunction f, boolReturnFunction b=trueLambda);

    // setting definition functions

    /**
     * basic checkbox
     * @param label the label of the setting
     * @param confName the config key of the setting
     * @param value pointer to a boolean value
     * @param inverted whether the checkbox is inverted (true->off, false->on)
     * @return a SettingEntry of the checkbox
     */
    static SettingEntry Checkbox(const char* label, const char* confName, bool* value, bool inverted=false) {
      return SettingEntry(inverted?SettingInvCheckbox:SettingCheckbox,label,confName,value);
    }
    /**
     * radio buttons
     * @param label the label of the setting
     * @param confName the config key of the setting
     * @param value pointer to a int value
     * @param entries the radio button entries. see SettingEntryMultiChoiceExtData for details.
     * @return a SettingEntry of the radio
     */
    static SettingEntry Radio(const char* label, const char* confName, int* value, std::initializer_list<SettingEntryMultiChoiceExtData<int>> entries) {
      SettingEntryMultiChoiceExtData<int>* data=new SettingEntryMultiChoiceExtData<int>[entries.size()];
      SettingEntryMultiChoiceExtData<int>* dataIter=data;
      for (auto i:entries) {
        *dataIter++=i;
      }
      return SettingEntry(SettingRadio,label,confName,value,data, entries.size());
    }
    /**
     * combo (dropdown list) with an integer value
     * @param label the label of the setting
     * @param confName the config key of the setting
     * @param value pointer to a int value
     * @param entries the combo entries. see SettingEntryMultiChoiceExtData for details.
     * @return a SettingEntry of the combo
     */
    static SettingEntry ComboInt(const char* label, const char* confName, int* value, std::initializer_list<SettingEntryMultiChoiceExtData<int>> entries) {
      SettingEntryMultiChoiceExtData<int>* data=new SettingEntryMultiChoiceExtData<int>[entries.size()];
      SettingEntryMultiChoiceExtData<int>* dataIter=data;
      for (auto i:entries) {
        *dataIter++=i;
      }
      return SettingEntry(SettingComboInt,label,confName,value,data, entries.size());
    }
    /**
     * combo (dropdown list) with a string value
     * @param label the label of the setting
     * @param confName the config key of the setting
     * @param value pointer to a String value
     * @param entries the combo entries. see SettingEntryMultiChoiceExtData for details.
     * @return a SettingEntry of the combo
     */
    static SettingEntry ComboString(const char* label, const char* confName, String* value, std::initializer_list<SettingEntryMultiChoiceExtData<const char*>> entries) {
      SettingEntryMultiChoiceExtData<const char*>* data=new SettingEntryMultiChoiceExtData<const char*>[entries.size()];
      SettingEntryMultiChoiceExtData<const char*>* dataIter=data;
      for (auto i:entries) {
        *dataIter++=i;
      }
      return SettingEntry(SettingComboStr,label,confName,value,data, entries.size());
    }
    /**
     * float slider
     * @param label the label of the setting
     * @param confName the config key of the setting
     * @param value pointer to a float value
     * @param limits the slider parameters. see SettingEntryNumericInputExtData for details
     * @return a SettingEntry of the slider
     */
    static SettingEntry SliderFloat(const char* label, const char* confName, float* value, SettingEntryNumericInputExtData<float> limits) {
      SettingEntryNumericInputExtData<float>* data=new SettingEntryNumericInputExtData<float>;
      *data=limits;
      return SettingEntry(SettingSliderFloat,label,confName,value,data,1);
    }
    /**
     * integer slider
     * @param label the label of the setting
     * @param confName the config key of the setting
     * @param value pointer to a int value
     * @param limits the slider parameters. see SettingEntryNumericInputExtData for details
     * @return a SettingEntry of the slider
     */
    static SettingEntry SliderInt(const char* label, const char* confName, int* value, SettingEntryNumericInputExtData<int> limits) {
      SettingEntryNumericInputExtData<int>* data=new SettingEntryNumericInputExtData<int>;
      *data=limits;
      return SettingEntry(SettingSliderInt,label,confName,value,data,1);
    }
    /**
     * integer input
     * @param label the label of the setting
     * @param confName the config key of the setting
     * @param value pointer to a int value
     * @param limits the input parameters. see SettingEntryNumericInputExtData for details
     * @return a SettingEntry of the input
     */
    static SettingEntry InputInt(const char* label, const char* confName, int* value, SettingEntryNumericInputExtData<int> limits) {
      SettingEntryNumericInputExtData<int>* dataPtr=new SettingEntryNumericInputExtData<int>;
      *dataPtr=limits;
      return SettingEntry(SettingInputInt,label,confName,value,dataPtr,1);
    }
    /**
     * text input
     * @param label the label of the setting
     * @param confName the config key of the setting
     * @param value pointer to a String value
     * @param hint the hint text shown in the input field. NULL if unused
     * @return a SettingEntry of the input
     */
    static SettingEntry InputText(const char* label, const char* confName, String* value, const char* hint=NULL) {
      SettingEntryTextInputExtData* dataPtr=new SettingEntryTextInputExtData(hint);
      return SettingEntry(SettingInputStr,label,confName,value,dataPtr,1);
    }
    /**
     * file path input with picker
     * @param label the label of the setting
     * @param confName the config key of the setting
     * @param value pointer to a String value
     * @param dialog a FurnaceGUIFileDialog ID corresponding to the file dialog
     * @param hint the hint text shown in the input field. NULL if unused
     * @return a SettingEntry of the input
     */
    static SettingEntry Path(const char* label, const char* confName, String* value, int dialog, const char* hint=NULL) {
      SettingEntryTextInputExtData* dataPtr=new SettingEntryTextInputExtData(hint,dialog);
      return SettingEntry(SettingInputStr,label,confName,value,dataPtr,1);
    }
    /**
     * color picker
     * @param label the label of the setting
     * @param confName the config key of the setting
     * @param value pointer to a ImVec4 value
     * @return a SettingEntry of the picker
     */
    static SettingEntry Color(const char* label, const char* confName, ImVec4* value) {
      return SettingEntry(SettingColor,label,confName,value);
    }
    /**
     * keybind assignment
     * @param label the label of the setting
     * @param confName the config key of the setting
     * @param keybind pointer to a keybindList (std::vector<int>) which contains the keybinds
     * @param actionNum a FurnaceGUIActions ID corresponding to the action
     * @return a SettingEntry
     */
    static SettingEntry Keybind(const char* label, const char* confName, keybindList* keybind, int actionNum) {
      int* actionPtr=new int;
      *actionPtr=actionNum;
      return SettingEntry(SettingKeybind,label,confName,keybind,actionPtr,1);
    }

    // misc. data functions

    /**
     * add a tooltip to a setting
     * @param text the tooltip text
     * @return the modified SettingEntry with the tooltip
     */
    SettingEntry& Tooltip(const char* text) {
      tooltip=text;
      return *this;
    }
    /**
     * add a draw condition to a setting
     * @param f the condition lambda (std::function<bool(void)>). the setting is drawn when said lambda returns true
     * @return the modified SettingEntry with the condition
     */
    SettingEntry& Condition(boolReturnFunction f) {
      settingCondition=f;
      return *this;
    }
    /**
     * add a change callback to a setting
     * @param f the callback lambda (std::function<void(void)>)
     * @return the modified SettingEntry with the callback
     */
    SettingEntry& Callback(entryCallback f) {
      callback=f;
      return *this;
    }

    // the draw function
    bool draw(FurnaceGUI* gui);
    // whether the setting passes the search filter
    bool passesFilter(ImGuiTextFilter* filter);
    // destroys the setting. deletes external data pointers
    void destroy();
};

class SettingsCategory {
  const char* name;
  std::vector<SettingEntry> settings;
  std::vector<SettingsCategory> children;
  public:
    SettingsCategory();
    SettingsCategory(const char* n, std::initializer_list<SettingEntry> s);
    SettingsCategory(const char* n, std::initializer_list<SettingEntry> s, std::initializer_list<SettingsCategory> c);
    //SettingsCategory(const SettingsCategory& s);

    bool drawSettings(ImGuiTextFilter* filter, bool doFilter, FurnaceGUI* gui, int depth, SettingsCategory* resetScroll);
    int categoryPassFilterRecursive(ImGuiTextFilter* filter);
    SettingsCategory* drawSidebar(ImGuiTextFilter* filter, FurnaceGUI* gui, SettingsCategory* parent=NULL);

    void deleteRecursive();
};

// this is still used?
// ...
// then why not one for cores?
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

// here it is. i hope it's good enough......
#define CORE_SETTING(_name,_setting,_combo) \
  ImGui::TableNextRow(); \
  ImGui::TableNextColumn(); \
  ImGui::AlignTextToFramePadding(); \
  ImGui::Text(_name); \
  ImGui::TableNextColumn(); \
  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x); \
  if (ImGui::Combo("##" _name "Core",&settings._setting,_combo,sizeof(_combo)/sizeof(_combo[0]))) ret=true; \
  ImGui::TableNextColumn(); \
  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x); \
  if (ImGui::Combo("##" _name "CoreRender",&settings._setting ## Render,_combo,sizeof(_combo)/sizeof(_combo[0]))) ret=true;


#endif
