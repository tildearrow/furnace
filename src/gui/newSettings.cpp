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

#include "newSettings.h"
#include "misc/cpp/imgui_stdlib.h"
#include "IconsFontAwesome4.h"
#include "gui.h"

SettingEntry::SettingEntry():
  type(SettingNone),
  label(NULL),
  tooltip(NULL),
  confName(NULL),
  value(NULL),
  extData(NULL),
  extDataCount(0),
  settingCondition(trueLambda),
  callback([]{}),
  customDrawFunction(falseLambda) {}

SettingEntry::SettingEntry(SettingType t, const char* l, const char* n, void* v, void* x, int xn, const char* d, boolReturnFunction b, entryCallback f) {
  type=t,
  label=l;
  confName=n;
  value=v;
  extData=x;
  extDataCount=xn;
  tooltip=d;
  settingCondition=b;
  callback=f;
  customDrawFunction=falseLambda;
}

SettingEntry::SettingEntry(const char* l, const char* n, boolReturnFunction f, boolReturnFunction b) {
  type=SettingCustom;
  label=l;
  confName=n;
  value=NULL;
  extData=NULL;
  extDataCount=0;
  tooltip=NULL;
  settingCondition=b;
  callback=[]{};
  customDrawFunction=f;
}

bool SettingEntry::draw() {
  bool ret=false;
  if (!settingCondition()) return false;
  switch (type) {
    case SettingCheckbox: {
      bool valueB=getValue<int>();
      if (ImGui::Checkbox(_(label),&valueB)) {
        setValue<int>(valueB);
        callback();
        ret=true;
      }
      break;
    }
    case SettingRadio: {
      assert(extData && "SettingRadio requires extData!");
      SettingEntryMultiChoiceExtData<int>* choices=(SettingEntryMultiChoiceExtData<int>*)extData;
      ImGui::BeginGroup();
      ImGui::TextUnformatted(_(label));
      ImGui::Indent();
      for (int i=0; i<extDataCount; i++) {
        SettingEntryMultiChoiceExtData<int> ch=choices[i];
        if (ImGui::RadioButton(_(ch.choice),getValue<int>()==ch.value)) {
          setValue<int>(ch.value);
          callback();
          ret=true;
        }
      }
      ImGui::Unindent();
      ImGui::EndGroup();
      break;
    }
    case SettingComboInt: {
      assert(extData && "SettingComboInt requires extData!");
      SettingEntryMultiChoiceExtData<int>* choices=(SettingEntryMultiChoiceExtData<int>*)extData;
      const char* preview=choices[0].choice; // fallback?
      for (size_t i=0; choices[i].choice; i++) {
        if (choices[i].value==getValue<int>()) {
          preview=choices[i].choice;
          break;
        }
      }
      if (ImGui::BeginCombo(_(label),_(preview))) {
        for (int i=0; i<extDataCount; i++) {
            if (ImGui::Selectable(_(choices[i].choice),getValue<int>()==choices[i].value)) {
            setValue(choices[i].value);
            callback();
            ret=true;
          }
        }
        ImGui::EndCombo();
      }
      break;
    }
    case SettingComboStr: {
      assert(extData && "SettingComboInt requires extData!");
      SettingEntryMultiChoiceExtData<String>* choices=(SettingEntryMultiChoiceExtData<String>*)extData;
      const char* preview=choices[0].choice; // fallback?
      for (int i=0; extDataCount; i++) {
        if (choices[i].value==getValue<String>()) {
          preview=choices[i].choice;
          break;
        }
      }
      if (ImGui::BeginCombo(_(label),_(preview))) {
        for (size_t i=0; choices[i].choice; i++) {
          if (ImGui::Selectable(_(choices[i].choice),getValue<String>()==choices[i].value)) {
            setValue(choices[i].value);
            callback();
            ret=true;
          }
        }
        ImGui::EndCombo();
      }
      break;
    }
    case SettingSliderFloat: {
      assert(extData && "SettingSliderFloat requires extData!");
      SettingEntryNumericInputExtData<float>* data=(SettingEntryNumericInputExtData<float>*)extData;
      if (ImGui::SliderFloat(_(label),(float*)value,data->min,data->max,data->fmt)) {
        if (getValue<float>()<data->min) setValue(data->min);
        if (getValue<float>()>data->max) setValue(data->max);
        callback();
        ret=true;
      }
      break;
    }
    case SettingSliderInt: {
      assert(extData && "SettingSliderInt requires extData!");
      SettingEntryNumericInputExtData<int>* data=(SettingEntryNumericInputExtData<int>*)extData;
      if (ImGui::SliderInt(_(label),(int*)value,data->min,data->max,data->fmt)) {
        if (getValue<int>()<data->min) setValue(data->min);
        if (getValue<int>()>data->max) setValue(data->max);
        callback();
        ret=true;
      }
      break;
    }
    case SettingInputInt: {
      assert(extData && "SettingInputInt requires extData!");
      SettingEntryNumericInputExtData<int>* data=(SettingEntryNumericInputExtData<int>*)extData;
      if (ImGui::InputInt(_(label),(int*)value)) {
        if (getValue<int>()<data->min) setValue(data->min);
        if (getValue<int>()>data->max) setValue(data->max);
        callback();
        ret=true;
      }
      break;
    }
    case SettingInputStr: {
      SettingEntryTextInputExtData* data=(SettingEntryTextInputExtData*)extData;
      const char* hint=NULL;
      if (data) {
        if (data->isPathInput) {
          ImGui::PushID(label);
          if (ImGui::Button(ICON_FA_FOLDER "##SettingPathButton")) {
            data->dialogCallback();
            ret=true;
          }
          ImGui::SameLine();
        }
        hint=data->hint;
      }
      if (ImGui::InputTextWithHint(_(label),hint,(String*)value)) {
        callback();
        ret=true;
      }
      break;
    }
    case SettingCustom:
      return customDrawFunction();
    case SettingNone:
    default:
      break;
  }
  if (tooltip) {
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("%s",_(tooltip));
    }
  }
  return ret;
}

bool SettingEntry::passesFilter(ImGuiTextFilter* filter) {
  return filter->PassFilter(_(label));
}

void SettingEntry::loadConf(DivConfig& conf) {
  
}

void SettingEntry::saveConf(DivConfig& conf) {
  
}

void SettingEntry::destroy() {
  if (extData==NULL) return;
  switch (type) {
    case SettingRadio:
      delete[] (SettingEntryMultiChoiceExtData<int>*)extData;
      extData=NULL;
      break;
    default: break;
  }
}

SettingsCategory::SettingsCategory():
  name(NULL),
  settings({}),
  children({}),
  scrollPos(0.0f) {}

SettingsCategory::SettingsCategory(const char* n, std::initializer_list<SettingEntry> s) {
  name=n;
  settings=s;
  children={};
  scrollPos=0.0f;
}

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

bool SettingsCategory::drawSettings(ImGuiTextFilter* filter, bool doFilter) {
  // get Y position for scroll
  if (!(filter->IsActive() && doFilter)) scrollPos=ImGui::GetCursorPosY();
  // check whether to draw the name
  // if no settings in the category match the filter, then no
  bool drawOwnSettings=true;
  if (filter->IsActive() && doFilter) {
    // if there are no settings in the category, then no
    if (settings.empty()) drawOwnSettings=false;
    // otherwise...
    bool isCategoryEmpty=true;
    for (SettingEntry& s:settings) {
      if (s.passesFilter(filter)) {
        isCategoryEmpty=false;
        break;
      }
    }
    if (isCategoryEmpty) drawOwnSettings=false;
  }
  bool ret=false;
  if (drawOwnSettings) {
    ImGui::SeparatorText(_(name));
    // ImGui::SameLine();
    // ImGui::TextColored(ImVec4(1.0f,.5f,.5f,1.f), "%f",scrollPos);
    for (SettingEntry& s:settings) {
      if (filter->IsActive() && doFilter) {
        if (!s.passesFilter(filter)) continue;
      }
      if (s.draw()) ret=true;
    }
  }
  ImGui::Indent();
    for (SettingsCategory& c:children) {
      if (c.drawSettings(filter,doFilter)) ret=true;
    }
  ImGui::Unindent();
  return ret;
}

bool SettingsCategory::categoryPassFilterRecursive(ImGuiTextFilter* filter) {
  if (filter->PassFilter(_(name))) return true;
  for (SettingsCategory& c:children) {
    if (c.categoryPassFilterRecursive(filter)) return true;
  }
  return false;
}

bool SettingsCategory::drawSidebar(ImGuiTextFilter* filter, float* targetScrollPos) {
  bool ret=false;
  if (children.empty()) {
    if (filter->IsActive()) {
      if (!filter->PassFilter(_(name))) return false;
    }
    ImGui::Selectable(_(name)); // should i use TreeNode with ImGuiTreeNodeFlags_Leaf?
    if (ImGui::IsItemClicked()) {
      *targetScrollPos=scrollPos;
      return true;
    }
  } else {
    bool popDisabled=false;
    ImGuiTreeNodeFlags treeFlags=ImGuiTreeNodeFlags_None;
    if (filter->IsActive()) {
      if (!filter->PassFilter(_(name))) {
        if (!categoryPassFilterRecursive(filter)) return false;
        else popDisabled=true;
      }
      treeFlags|=ImGuiTreeNodeFlags_DefaultOpen;
    }
    ImGui::BeginDisabled(popDisabled);
    popDisabled=true;
    if (ImGui::TreeNodeEx(_(name), treeFlags)) {
      ImGui::EndDisabled();
      popDisabled=false;
      for (SettingsCategory& c:children)
        ret=c.drawSidebar(filter, targetScrollPos);
      ImGui::TreePop();
    }
    if (popDisabled) ImGui::EndDisabled();
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) { // hack because IsItemClicked returns true when anywhere inside the node and on the rising edge
      if (*targetScrollPos==-1.0f) *targetScrollPos=scrollPos;
      ret=true;
    }
  }
  return ret;
}

SettingsCategory::~SettingsCategory() {
  // settings.clear();
  // children.clear();
}

#define _S SettingEntry
#define _C(...) allSettings.push_back(SettingsCategory(__VA_ARGS__))
#define _CC SettingsCategory

#define SETTING_CHECKBOX(_label,_value) \
  SettingEntry::Checkbox(_label,#_value,&settings._value)

void FurnaceGUI::initSettings() {
  _C(_N("General"),{},{
    _CC(_N("Program"),{
      SettingEntry::Radio(
        _N("Play after opening song:"),"playOnLoad",&settings.playOnLoad,{
        {_N("No##pol0"),0},
        {_N("Only if already playing##pol1"),1},
        {_N("Yes##pol0"),2},
      }),
      SETTING_CHECKBOX(
        _N("Store instrument name in .fui"),
        writeInsNames
      ),
    })
  });
  _C(_N("Audio"),{},{
    _CC(_N("Output"),{
#if defined(HAVE_JACK) || defined(HAVE_PA) || defined(HAVE_ASIO)
      SettingEntry::ComboInt(
        _N("Backend"),
        "audioEngine",&settings.audioEngine,{
#ifdef HAVE_JACK
        {"JACK",DIV_AUDIO_JACK},
#endif
        {"SDL",DIV_AUDIO_SDL},
#ifdef HAVE_PA
        {"PortAudio",DIV_AUDIO_PORTAUDIO},
#endif
#ifdef HAVE_ASIO
        {"ASIO",DIV_AUDIO_ASIO},
#endif
      }),
#endif
    })
  });
  _C(_N("Emulation"),{
    SettingEntry::ComboInt(
      _N("PC Speacker Strategy"),
      "pcSpeakerOutMethod",&settings.pcSpeakerOutMethod,{
        {_N("evdev SND_TONE"),0},
        {_N("KIOCSOUND on /dev/tty1"),1},
        {_N("/dev/port"),2},
        {_N("KIOCSOUND on standard output"),3},
        {_N("outb()"),4}
      })
  },{
    _CC(_N("Sample ROMs"),{
      SettingEntry::Path(
        _N("OPL4 YRW801 path"),
        "yrw801Path",&settings.yrw801Path,
        [this]{openFileDialog(GUI_FILE_YRW801_ROM_OPEN);}
      )
    })
  });
  _C(_N("Appearance"),{
    SettingEntry::Radio(
      _N("Channel feedback style:"),
      "channelFeedbackStyle",&settings.channelFeedbackStyle,{
        {_N("Off##CHF0"),0},
        {_N("Note##CHF1"),1},
        {_N("Volume##CHF2"),2},
        {_N("Active##CHF3"),3},
        {_N("Volume (Real)##CHF4"),4},
      }
    ),
    SettingEntry::SliderFloat(
      _N("Gamma##CHF"),
      "channelFeedbackGamma",&settings.channelFeedbackGamma,
      {0.0f,2.0f,NULL}
    ).condition([this]{return settings.channelFeedbackStyle==4;})
  },{
    _CC(_N("Pattern view labels"),{
      SettingEntry::InputText(
        _N("Note off (3-char)"),
        "noteOffLabel",&settings.noteOffLabel,
        "OFF"
      ),
      SettingEntry::InputText(
        _N("Note release (3-char)"),
        "macroRelLabel",&settings.macroRelLabel,
        "==="
      ),
    })
  });
}
