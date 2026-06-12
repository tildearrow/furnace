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
#include "guiConst.h"
#include "util.h"
#include "intConst.h"

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

bool SettingEntry::draw(FurnaceGUI* gui) {
  bool ret=false;
  if (!settingCondition()) return false;
  switch (type) {
    case SettingCheckbox: {
      if (ImGui::Checkbox(_(label),(bool*)value)) {
        callback();
        ret=true;
      }
      break;
    }
    case SettingInvCheckbox: {
      bool valueB=!getValue<bool>();
      if (ImGui::Checkbox(_(label),&valueB)) {
        setValue<bool>(!valueB);
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
        if (ImGui::RadioButton(_(choices[i].choice),getValue<int>()==choices[i].value)) {
          setValue<int>(choices[i].value);
          callback();
          ret=true;
        }
        if (choices[i].tooltip) if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("%s",_(choices[i].tooltip));
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
      for (int i=0; i<extDataCount; i++) {
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
          if (choices[i].tooltip) if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s",_(choices[i].tooltip));
          }
        }
        ImGui::EndCombo();
      }
      break;
    }
    case SettingComboStr: {
      assert(extData && "SettingComboStr requires extData!");
      SettingEntryMultiChoiceExtData<const char*>* choices=(SettingEntryMultiChoiceExtData<const char*>*)extData;
      const char* preview=choices[0].choice; // fallback?
      for (int i=0; i<extDataCount; i++) {
        if (choices[i].value==getValue<String>()) {
          preview=choices[i].choice;
          break;
        }
      }
      if (ImGui::BeginCombo(_(label),_(preview))) {
        for (int i=0; i<extDataCount; i++) {
          if (ImGui::Selectable(_(choices[i].choice),getValue<String>()==choices[i].value)) {
            setValue<String>(choices[i].value);
            callback();
            ret=true;
          }
          if (choices[i].tooltip) if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s",_(choices[i].tooltip));
          }
        }
        ImGui::EndCombo();
      }
      break;
    }
    case SettingSliderFloat: {
      assert(extData && "SettingSliderFloat requires extData!");
      SettingEntryNumericInputExtData<float>* data=(SettingEntryNumericInputExtData<float>*)extData;
      if (gui->CWSliderFloat(_(label),(float*)value,data->minV,data->maxV,data->fmt)) {
        if (getValue<float>()<data->minV) setValue(data->minV);
        if (getValue<float>()>data->maxV) setValue(data->maxV);
        callback();
        ret=true;
      } rightClickable
      break;
    }
    case SettingSliderInt: {
      assert(extData && "SettingSliderInt requires extData!");
      SettingEntryNumericInputExtData<int>* data=(SettingEntryNumericInputExtData<int>*)extData;
      if (gui->CWSliderInt(_(label),(int*)value,data->minV,data->maxV,data->fmt)) {
        if (getValue<int>()<data->minV) setValue(data->minV);
        if (getValue<int>()>data->maxV) setValue(data->maxV);
        callback();
        ret=true;
      } rightClickable
      break;
    }
    case SettingInputInt: {
      assert(extData && "SettingInputInt requires extData!");
      SettingEntryNumericInputExtData<int>* data=(SettingEntryNumericInputExtData<int>*)extData;
      if (ImGui::InputInt(_(label),(int*)value,data->step,data->stepFast)) {
        if (getValue<int>()<data->minV) setValue(data->minV);
        if (getValue<int>()>data->maxV) setValue(data->maxV);
        callback();
        ret=true;
      }
      break;
    }
    case SettingInputStr: {
      SettingEntryTextInputExtData* data=(SettingEntryTextInputExtData*)extData;
      const char* hint=NULL;
      if (data) {
        if (data->dialogNum!=-1) {
          ImGui::PushID(label);
          if (ImGui::Button(ICON_FA_FOLDER "##SettingPathButton")) {
            gui->openFileDialog((FurnaceGUIFileDialogs)data->dialogNum);
            ret=true;
          }
          ImGui::PopID();
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
    case SettingColor:
      ImGui::PushID(label);
      if (ImGui::ColorEdit4(_(label),(float*)value)) {
        gui->applyUISettings(false);
        callback();
        ret=true;
      }
      ImGui::PopID();
      break;
    case SettingKeybind: {
      keybindList* keybinds=(keybindList*)value;
      int actionIdx=*(int*)extData;
      ImGui::TextUnformatted(_(label));
      ImGui::SameLine();
      ImGui::PushID(label);
      for (size_t i=0; i<=keybinds->size(); i++) {
        ImGui::PushID(i);
        if (i>0) ImGui::SameLine();
        bool isPending=gui->bindSetPending && gui->bindSetTarget==actionIdx && gui->bindSetTargetIdx==(int)i;
        if (i<keybinds->size()) {
          if (ImGui::Button(isPending?_N("Press key..."):getKeyName(keybinds->at(i)).c_str())) {
            gui->promptKey(actionIdx,i);
            ret=true;
          }
          bool rightClicked=ImGui::IsItemClicked(ImGuiMouseButton_Right);
          if (!rightClicked) {
            ImGui::SameLine(0.0f, 1.0f);
          }
          if (rightClicked || ImGui::Button(ICON_FA_TIMES)) {
            keybinds->erase(keybinds->begin()+i);
            if (isPending) {
              gui->bindSetActive=false;
              gui->bindSetPending=false;
            }
            gui->parseKeybinds();
          }
        } else {
          if (ImGui::Button(isPending?_N("Press key..."):ICON_FA_PLUS)) {
            gui->promptKey(actionIdx,i);
            ret=true;
          }
        }
        ImGui::PopID(); // i
      }
      ImGui::PopID();
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
  if (!settingCondition()) return false;
  if (filter->PassFilter(_(label))) return true;
  switch (type) {
    case SettingRadio:
    case SettingComboInt: {
      SettingEntryMultiChoiceExtData<int>* choices=(SettingEntryMultiChoiceExtData<int>*)extData;
      for (int i=0; i<extDataCount; i++) {
        if (filter->PassFilter(_(choices[i].choice))) return true;
      }
      break;
    }
    case SettingComboStr: {
      SettingEntryMultiChoiceExtData<const char*>* choices=(SettingEntryMultiChoiceExtData<const char*>*)extData;
      for (int i=0; i<extDataCount; i++) {
        if (filter->PassFilter(_(choices[i].choice))) return true;
      }
      break;
    }
    default: break;
  }
  return false;
}

void SettingEntry::destroy() {
  if (extData==NULL) return;
  switch (type) {
    case SettingRadio:
    case SettingComboInt:
      delete[] (SettingEntryMultiChoiceExtData<int>*)extData;
      extData=NULL;
      break;
    case SettingComboStr:
      delete[] (SettingEntryMultiChoiceExtData<const char*>*)extData;
      extData=NULL;
      break;
    case SettingSliderFloat:
      delete (SettingEntryNumericInputExtData<float>*)extData;
      extData=NULL;
      break;
    case SettingSliderInt:
    case SettingInputInt:
      delete (SettingEntryNumericInputExtData<int>*)extData;
      extData=NULL;
      break;
    case SettingInputStr:
      delete (SettingEntryTextInputExtData*)extData;
      extData=NULL;
      break;
    case SettingKeybind:
      delete (int*)extData;
      extData=NULL;
      break;
    default: break;
  }
}

SettingsCategory::SettingsCategory():
  name(NULL),
  settings({}),
  children({}) {}

SettingsCategory::SettingsCategory(const char* n, std::initializer_list<SettingEntry> s) {
  name=n;
  settings=s;
  children={};
}

SettingsCategory::SettingsCategory(const char* n, std::initializer_list<SettingEntry> s, std::initializer_list<SettingsCategory> c) {
  name=n;
  settings=s;
  children=c;
}

// commented out (deemed unnecessary and fails Android build)
/*SettingsCategory::SettingsCategory(const SettingsCategory& s) {
  name=s.name;
  settings=s.settings;
  children=s.children;
  scrollPos=s.scrollPos;
}*/

bool SettingsCategory::drawSettings(ImGuiTextFilter* filter, bool doFilter, FurnaceGUI* gui, int depth, SettingsCategory* resetScroll) {
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
    switch (depth) {
      case 0:
        ImGui::PushFont(gui->headFont,gui->settings.headFontSize*gui->dpiScale);
        break;
      case 1:
        ImGui::PushFont(gui->headFont,gui->settings.headFontSize2*gui->dpiScale);
        break;
      case 2:
        ImGui::PushFont(gui->headFont,gui->settings.headFontSize3*gui->dpiScale);
        break;
      default:
        ImGui::PushFont(gui->headFont,gui->settings.headFontSize4*gui->dpiScale);
        break;
    }
    
    ImGui::SeparatorText(_(name));
    ImGui::PopFont();
    if (resetScroll==this) {
      resetScroll=NULL;
      ImGui::SetScrollHereY(0.0f);
    }
    //ImGui::Indent();
    for (SettingEntry& s:settings) {
      if (filter->IsActive() && doFilter) {
        if (!s.passesFilter(filter)) continue;
      }
      if (s.draw(gui)) ret=true;
    }
    //ImGui::Unindent();
  }
  //ImGui::Indent();
  for (SettingsCategory& c:children) {
    if (c.drawSettings(filter,doFilter,gui,depth+1,resetScroll)) ret=true;
  }
  //ImGui::Unindent();
  return ret;
}

int SettingsCategory::categoryPassFilterRecursive(ImGuiTextFilter* filter) {
  int ret=0;
  if (filter->PassFilter(_(name))) ret|=1;
  for (SettingsCategory& c:children) {
    if (c.categoryPassFilterRecursive(filter)) ret|=2;
  }
  return ret;
}
/*
bool SettingsCategory::drawSidebar(ImGuiTextFilter* filter, FurnaceGUI* gui) {
  // this code is a mess but it works, somehow
  // well, barely
  // the treenode clicking only works when its closed
  bool ret=false; // return if clicked
  bool popDisabled=false; // disable current node if it doesnt pass the filter but its children do
  bool drawChildren=true; // dont draw if doesnt have children or the children dont pass the filter
  bool forceOpen=false;
  ImGuiTreeNodeFlags treeFlags=ImGuiTreeNodeFlags_None;
  if (children.empty()) {
    drawChildren=false;
    if (filter->IsActive()) {
      if (!filter->PassFilter(_(name))) return false;
    }
  } else {
    if (filter->IsActive()) {
      switch (categoryPassFilterRecursive(filter)) {
        case 0: // none
          return false;
        case 1: // only self
          treeFlags|=ImGuiTreeNodeFlags_Leaf|ImGuiTreeNodeFlags_NoTreePushOnOpen;
          drawChildren=false;
          break;
        case 2: // only children
          popDisabled=true;
          forceOpen=true;
          break;
        case 3: // both
          forceOpen=true;
          break;
      }
    }
    if (gui->curCategory==this) {
      treeFlags|=ImGuiTreeNodeFlags_Selected;
    }
  }
  if (drawChildren) {
    ImGui::BeginDisabled(popDisabled);
    popDisabled=true;
    if (forceOpen) ImGui::SetNextItemOpen(true);
    if (ImGui::TreeNodeEx(_(name),treeFlags)) {
      ImGui::EndDisabled();
      popDisabled=false;
      for (size_t i=0; i<children.size(); i++)
        ret|=children[i].drawSidebar(filter,gui);
      ImGui::Dummy({0.f,0.f}); // I DONT KNOW WHY BUT WITHOUT THIS HOVERING ON THE LAST CHILD ALSO HOVERS ON THE TREENODE
      if (!(treeFlags&ImGuiTreeNodeFlags_NoTreePushOnOpen))
        ImGui::TreePop();
    }
    if (popDisabled) ImGui::EndDisabled();
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) { // hack because IsItemClicked returns true when anywhere inside the node and on the rising edge
      gui->curCategory=this;
      ret=true;
    }
  } else {
    ImGui::Selectable(_(name),gui->curCategory==this);
    if (ImGui::IsItemClicked()) {
      gui->curCategory=this;
      return true;
    }
  }
  return ret;
}*/
SettingsCategory* SettingsCategory::drawSidebar(ImGuiTextFilter* filter, FurnaceGUI* gui, SettingsCategory* parent) {
  SettingsCategory* ret=NULL;
  bool drawChildren=true;
  bool isDisabled=false;
  if (filter->IsActive()) {
    if (children.empty()) {
      drawChildren=false;
      if (!filter->PassFilter(_(name))) return NULL;
    } else {
      switch (categoryPassFilterRecursive(filter)) {
        case 0: // none
          return NULL;
        case 1: // only self
          drawChildren=false;
          break;
        case 2: // only children
          isDisabled=true;
          break;
        case 3: // both
          break;
      }
    }
  }
  ImGui::BeginDisabled(isDisabled);
  if (ImGui::Selectable(_(name),gui->curCategory==this)) {
    // show the top level category
    // I don't think it's a good idea to display a sub-category alone
    gui->curCategory=parent?parent:this;
    ret=this;
  }
  ImGui::EndDisabled();
  if (drawChildren) {
    ImGui::PushID(this);
    ImGui::Indent();
    for (size_t i=0; i<children.size(); i++) {
      SettingsCategory* newRet=children[i].drawSidebar(filter,gui,parent?parent:this);
      if (newRet) ret=newRet;
    }
    ImGui::Unindent();
    ImGui::PopID();
  }
  return ret;
}

void SettingsCategory::deleteRecursive() {
  for (size_t i=0; i<settings.size(); i++) {
    settings[i].destroy();
  }
  for (size_t i=0; i<children.size(); i++) {
    children[i].deleteRecursive();
  }
}
