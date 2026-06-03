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
#include <imgui.h>

static const char* valueInputStyles[]={
  _N("Disabled/custom"),
  _N("Two octaves (0 is C-4, F is D#5)"),
  _N("Raw (note number is value)"),
  _N("Two octaves alternate (lower keys are 0-9, upper keys are A-F)"),
  _N("Use dual control change (one for each nibble)"),
  _N("Use 14-bit control change"),
  _N("Use single control change (imprecise)")
};

static String stripName(String what) {
  String ret;
  for (char& i: what) {
    if ((i>='A' && i<='Z') || (i>='a' && i<='z') || (i>='0' && i<='9')) {
      ret+=i;
    } else {
      ret+='-';
    }
  }
  return ret;
}

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
  if (filter->PassFilter(_(label))) return true;
  switch (type) {
    case SettingRadio:
    case SettingComboInt: {
      assert(extData && "SettingComboInt requires extData!");
      SettingEntryMultiChoiceExtData<int>* choices=(SettingEntryMultiChoiceExtData<int>*)extData;
      for (int i=0; i<extDataCount; i++) {
        if (filter->PassFilter(_(choices[i].choice))) return true;
      }
      break;
    }
    case SettingComboStr: {
      assert(extData && "SettingComboInt requires extData!");
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

// commented out (deemed unnecessary and fails Android build)
/*SettingsCategory::SettingsCategory(const SettingsCategory& s) {
  name=s.name;
  settings=s.settings;
  children=s.children;
  scrollPos=s.scrollPos;
}*/

bool SettingsCategory::drawSettings(ImGuiTextFilter* filter, bool doFilter, FurnaceGUI* gui) {
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
    ImGui::Indent();
    for (SettingEntry& s:settings) {
      if (filter->IsActive() && doFilter) {
        if (!s.passesFilter(filter)) continue;
      }
      if (s.draw(gui)) ret=true;
    }
    ImGui::Unindent();
  }
  ImGui::Indent();
  for (SettingsCategory& c:children) {
    if (c.drawSettings(filter,doFilter,gui)) ret=true;
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
/*bool SettingsCategory::drawSidebar(ImGuiTextFilter* filter, float* targetScrollPos) {
  bool ret=false;
  ImGui::BeginDisabled(filter->IsActive() && !filter->PassFilter(_(name)));
  if (ImGui::Selectable(_(name))) {
    *targetScrollPos=scrollPos;
    ret=true;
  }
  ImGui::EndDisabled();
  ImGui::Indent();
  for (SettingsCategory& s:children) {
    ret|=s.drawSidebar(filter,targetScrollPos);
  }
  ImGui::Unindent();
  return ret;
}*/

void SettingsCategory::deleteRecursive() {
  for (size_t i=0; i<settings.size(); i++) {
    settings[i].destroy();
  }
  for (size_t i=0; i<children.size(); i++) {
    children[i].deleteRecursive();
  }
}

#define _S SettingEntry
#define CATEGORY_BEGIN(_name) allSettings.push_back(SettingsCategory(_name,
#define CATEGORY_END ));
#define SUBCATEGORY SettingsCategory

#define SETTING_CHECKBOX(_label,_value) SettingEntry::Checkbox(_label,#_value,&settings._value)
#define SETTING_INV_CHECKBOX(_label,_value) SettingEntry::Checkbox(_label,#_value,&settings._value,true)
#define SETTING_COLOR(_which) \
  SettingEntry::Color(guiColors[_which].friendlyName,NULL,&uiColors[_which])
#define SETTING_KEYBIND(_which) \
  SettingEntry::Keybind(guiActions[_which].friendlyName,NULL,&actionKeys[_which],_which)

void FurnaceGUI::initSettings() {
  CATEGORY_BEGIN(_N("General")) {},{
    SUBCATEGORY(_N("Program"),{
#ifdef HAVE_LOCALE
      SettingEntry::ComboString(
        _N("Language"),
        "locale",&settings.locale,{
          {"<System>",""},
          {"English", "en", "restart Furnace for this setting to take effect."},
          {"Bahasa Indonesia (50%?)", "id", "???"},
          //{"Deutsch (0%)", "de", "Starten Sie Furnace neu, damit diese Einstellung wirksam wird."},
          {"Español", "es", "reinicia Furnace para que esta opción tenga efecto."},
          //{"Suomi (0%)", "fi", "käynnistä Furnace uudelleen, jotta tämä asetus tulee voimaan."},
          {"Français (10%)", "fr", "redémarrer Furnace pour que ce réglage soit effectif."},
          //{"Հայերեն (1%)", "hy", "???"},
          //{"日本語 (0%)", "ja", "???"},
          {"한국어 (25%)", "ko", "이 설정을 적용하려면 Furnace를 다시 시작해야 합니다."},
          //{"Nederlands (4%)", "nl", "start Furnace opnieuw op om deze instelling effectief te maken."},
          {"Polski (95%)", "pl", "aby to ustawienie było skuteczne, należy ponownie uruchomić program."},
          {"Português (Brasil) (70%)", "pt_BR", "reinicie o Furnace para que essa configuração entre em vigor."},
          {"Русский", "ru", "перезапустите программу, чтобы эта настройка вступила в силу."},
          {"Slovenčina (15%)", "sk", "???"},
          {"Svenska", "sv", "starta om programmet för att denna inställning ska träda i kraft."},
          //{"ไทย (0%)", "th", "???"},
          //{"Türkçe (0%)", "tr", "bu ayarı etkin hale getirmek için programı yeniden başlatın."},
          //{"Українська (0%)", "uk", "перезапустіть програму, щоб це налаштування набуло чинності."},
          {"中文 (15%)", "zh", "???"},
        }
      ),
#endif
      SettingEntry::ComboString(
        _N("Render backend"),
        "renderBackend",&settings.renderBackend,{
#ifdef HAVE_RENDER_SDL
          {"SDL Renderer","SDL"},
#endif
#ifdef HAVE_RENDER_DX11
          {"DirectX 11","DirectX 11"},
#endif
#ifdef HAVE_RENDER_DX9
          {"DirectX 9","DirectX 9"},
#endif
#ifdef HAVE_RENDER_METAL
          {"Metal","Metal"},
#endif
#ifdef HAVE_RENDER_GL
#ifdef USE_GLES
          {"OpenGL ES 2.0","OpenGL ES 2.0"},
#else
          {"OpenGL 3.0","OpenGL 3.0"},
          {"OpenGL 2.0","OpenGL 2.0"},
#endif
#endif
#ifdef HAVE_RENDER_GL1
          {"OpenGL 1.1","OpenGL 1.1"},
#endif
          {"Software","Software"},
        }
      ).Tooltip(_("you may need to restart Furnace for this setting to take effect.")),
      SettingEntry(_N("Advanced render backend settings"),NULL,[this]{
        bool ret=false;
        if (ImGui::TreeNode(_("Advanced render backend settings"))) {
          String curRenderBackend=settings.renderBackend.empty()?GUI_BACKEND_DEFAULT_NAME:settings.renderBackend;
          if (curRenderBackend=="SDL") {
            if (ImGui::BeginCombo(_("Render driver"),settings.renderDriver.empty()?_("Automatic"):settings.renderDriver.c_str())) {
              if (ImGui::Selectable(_("Automatic"),settings.renderDriver.empty())) {
                settings.renderDriver="";
                ret=true;
              }
              for (String& i: availRenderDrivers) {
                if (ImGui::Selectable(i.c_str(),i==settings.renderDriver)) {
                  settings.renderDriver=i;
                  ret=true;
                }
              }
              ImGui::EndCombo();
            }
            if (ImGui::IsItemHovered()) {
              ImGui::SetTooltip(_("you may need to restart Furnace for this setting to take effect."));
            }
          } else if (curRenderBackend.find("OpenGL")==0) {
            ImGui::TextWrapped(_("beware: changing these settings may render Furnace unusable! do so at your own risk.\nstart Furnace with -safemode if you mess something up."));
            if (ImGui::InputInt(_("Red bits"),&settings.glRedSize)) {
              if (settings.glRedSize<0) settings.glRedSize=0;
              if (settings.glRedSize>32) settings.glRedSize=32;
              ret=true;
            }
            if (ImGui::InputInt(_("Green bits"),&settings.glGreenSize)) {
              if (settings.glGreenSize<0) settings.glGreenSize=0;
              if (settings.glGreenSize>32) settings.glGreenSize=32;
              ret=true;
            }
            if (ImGui::InputInt(_("Blue bits"),&settings.glBlueSize)) {
              if (settings.glBlueSize<0) settings.glBlueSize=0;
              if (settings.glBlueSize>32) settings.glBlueSize=32;
              ret=true;
            }
            if (ImGui::InputInt(_("Alpha bits"),&settings.glAlphaSize)) {
              if (settings.glAlphaSize<0) settings.glAlphaSize=0;
              if (settings.glAlphaSize>32) settings.glAlphaSize=32;
              ret=true;
            }
            if (ImGui::InputInt(_("Color depth"),&settings.glDepthSize)) {
              if (settings.glDepthSize<0) settings.glDepthSize=0;
              if (settings.glDepthSize>128) settings.glDepthSize=128;
              ret=true;
            }
            
            if (ImGui::Checkbox(_("Set stencil and buffer sizes"),&settings.glSetBS)) {
              ret=true;
            }

            if (settings.glSetBS) {
              if (ImGui::InputInt(_("Stencil buffer size"),&settings.glStencilSize)) {
                if (settings.glStencilSize<0) settings.glStencilSize=0;
                if (settings.glStencilSize>32) settings.glStencilSize=32;
                ret=true;
              }
              if (ImGui::InputInt(_("Buffer size"),&settings.glBufferSize)) {
                if (settings.glBufferSize<0) settings.glBufferSize=0;
                if (settings.glBufferSize>128) settings.glBufferSize=128;
                ret=true;
              }
            }

            if (ImGui::Checkbox(_("Double buffer"),&settings.glDoubleBuffer)) {
              ret=true;
            }

            ImGui::TextWrapped(_("the following values are common (in red, green, blue, alpha order):\n- 24 bits: 8, 8, 8, 0\n- 16 bits: 5, 6, 5, 0\n- 32 bits (with alpha): 8, 8, 8, 8\n- 30 bits (deep): 10, 10, 10, 0"));
          } else {
            ImGui::Text(_("nothing to configure"));
          }
          ImGui::TreePop();
        }
        return ret;
      }),
      SettingEntry(_N("Render backend information"),NULL,[this]{
        ImGui::TextWrapped(_("current backend: %s\n%s\n%s\n%s"),rend->getBackendName(),rend->getVendorName(),rend->getDeviceName(),rend->getAPIVersion());
        return false;
      }),
      SETTING_CHECKBOX(
        _N("VSync"),
        vsync
      ).Callback([this]{
        if (rend!=NULL) {
          rend->setSwapInterval(settings.vsync);
        }
      }),
      // TODO: dynamic text?
      SettingEntry::SliderInt(
        _N("Frame rate limit"),
        "frameRateLimit",&settings.frameRateLimit,
        {0,250}
      ).Tooltip(_("only applies when VSync is disabled.")),
      SETTING_CHECKBOX(
        _N("Display render time"),
        displayRenderTime
      ),
      SETTING_CHECKBOX(
        _N("Late render clear"),
        renderClearPos
      ).Tooltip(_("calls rend->clear() after rend->present(). might reduce UI latency by one frame in some drivers."))
#ifdef HAVE_RENDER_METAL
       .Condition([this]{return settings.renderBackend!="Metal";})
#endif
      ,
      SETTING_CHECKBOX(
        _N("Power-saving mode"),
        powerSave
      ).Tooltip(_("saves power by lowering the frame rate to 2fps when idle.\nmay cause issues under Mesa drivers!")),
#ifndef IS_MOBILE
      SETTING_CHECKBOX(
        _N("Disable threaded input (restart after changing!)"),
        noThreadedInput
      ).Tooltip(_("threaded input processes key presses for note preview on a separate thread (on supported platforms), which reduces latency.\nhowever, crashes have been reported when threaded input is on. enable this option if that is the case.")),
#endif
      SETTING_CHECKBOX(
        _N("Enable event delay"),
        eventDelay
      ).Tooltip(_("may cause issues with high-polling-rate mice when previewing notes.")).Callback([this]{
        applyUISettings(false);
      }),
    }),
#ifdef IS_MOBILE
    SUBCATEGORY(_N("Vibration"),{
      SettingEntry::SliderFloat(
        _N("Strength"),
        "vibrationStrength",&settings.vibrationStrength,
        {0.0f,1.0f}
      ),
      SettingEntry::SliderInt(
        _N("Length"),
        "vibrationLength",&settings.vibrationLength,
        {10,500,"%d ms"}
      )
    }),
#endif
    SUBCATEGORY(_N("File"),{
#ifndef FLATPAK_WORKAROUNDS
      SETTING_CHECKBOX(
        _N("Use system file picker"),
        sysFileDialog
      ),
#endif
      SettingEntry::InputInt(
        _N("Number of recent files"),
        "maxRecentFile",&settings.maxRecentFile,
        {0,30,1,5}
      ),
      SETTING_CHECKBOX(
        _N("Compress when saving"),
        compress
      ).Tooltip(_("use zlib to compress saved songs.")),
      SETTING_CHECKBOX(
        _N("Save unused patterns"),
        saveUnusedPatterns
      ),
      SETTING_CHECKBOX(
        _N("Don't apply compatibility flags when loading .dmf"),
        noDMFCompat
      ).Tooltip(_("do not report any issues arising from the use of this option!")),
      SettingEntry::Radio(
        _N("Play after opening song:"),"playOnLoad",&settings.playOnLoad,{
        {_N("No##pol0"),0},
        {_N("Only if already playing##pol1"),1},
        {_N("Yes##pol0"),2},
      }),
      // I hate this setting
      SettingEntry(_N("Audio export loop/fade out time:"),NULL,[this]{
        bool ret=false;
        ImGui::Text(_("Audio export loop/fade out time:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("Set to these values on start-up:##fot0"),settings.persistFadeOut==0)) {
          settings.persistFadeOut=0;
          ret=true;
        }
        ImGui::BeginDisabled(settings.persistFadeOut);
        ImGui::Indent();
        if (ImGui::InputInt(_("Loops"),&settings.exportLoops,1,2)) {
          if (settings.exportLoops<0) settings.exportLoops=0;
          audioExportOptions.loops=settings.exportLoops;
          ret=true;
        }
        if (ImGui::InputDouble(_("Fade out (seconds)"),&settings.exportFadeOut,1.0,2.0,"%.1f")) {
          if (settings.exportFadeOut<0.0) settings.exportFadeOut=0.0;
          audioExportOptions.fadeOut=settings.exportFadeOut;
          ret=true;
        }
        ImGui::Unindent();
        ImGui::EndDisabled();
        if (ImGui::RadioButton(_("Remember last values##fot1"),settings.persistFadeOut==1)) {
          settings.persistFadeOut=1;
          ret=true;
        }
        ImGui::Unindent();

        return ret;
      }),
      SETTING_CHECKBOX(
        _N("Store instrument name in .fui"),
        writeInsNames
      ).Tooltip(_("when enabled, saving an instrument will store its name.\nthis may increase file size.")),
      SETTING_CHECKBOX(
        _N("Load instrument name from .fui"),
        readInsNames
      ).Tooltip(_("when enabled, loading an instrument will use the stored name (if present).\notherwise, it will use the file name.")),
      SETTING_CHECKBOX(
        _N("Auto-fill file name when saving"),
        autoFillSave
      ).Tooltip(_("fill the file name field with an appropriate file name when saving or exporting.")),
    }),
    SUBCATEGORY(_N("New Song"),{
      SettingEntry(_N("Initial system:"),NULL,[this]{
        bool ret=false;
        ImGui::AlignTextToFramePadding();
        ImGui::Text(_("Initial system:"));
        ImGui::SameLine();
        if (ImGui::Button(_("Current system"))) {
          settings.initialSys.clear();
          for (int i=0; i<e->song.systemLen; i++) {
            settings.initialSys.set(fmt::sprintf("id%d",i),e->systemToFileFur(e->song.system[i]));
            settings.initialSys.set(fmt::sprintf("vol%d",i),(float)e->song.systemVol[i]);
            settings.initialSys.set(fmt::sprintf("pan%d",i),(float)e->song.systemPan[i]);
            settings.initialSys.set(fmt::sprintf("fr%d",i),(float)e->song.systemPanFR[i]);
            settings.initialSys.set(fmt::sprintf("flags%d",i),e->song.systemFlags[i].toBase64());
          }
          settings.initialSysName=e->song.systemName;
          ret=true;
        }
        ImGui::SameLine();
        if (ImGui::Button(_("Randomize"))) {
          settings.initialSys.clear();
          int howMany=1+rand()%3;
          int totalAvailSys=0;
          for (totalAvailSys=0; availableSystems[totalAvailSys]; totalAvailSys++);
          if (totalAvailSys>0) {
            for (int i=0; i<howMany; i++) {
              DivSystem theSystem=DIV_SYSTEM_DUMMY;
              do {
                theSystem=(DivSystem)availableSystems[rand()%totalAvailSys];
              } while (!settings.hiddenSystems && CHECK_HIDDEN_SYSTEM(theSystem));
              settings.initialSys.set(fmt::sprintf("id%d",i),e->systemToFileFur(theSystem));
              settings.initialSys.set(fmt::sprintf("vol%d",i),1.0f);
              settings.initialSys.set(fmt::sprintf("pan%d",i),0.0f);
              settings.initialSys.set(fmt::sprintf("fr%d",i),0.0f);
              settings.initialSys.set(fmt::sprintf("flags%d",i),"");
            }
          } else {
            settings.initialSys.set("id0",e->systemToFileFur(DIV_SYSTEM_DUMMY));
            settings.initialSys.set("vol0",1.0f);
            settings.initialSys.set("pan0",0.0f);
            settings.initialSys.set("fr0",0.0f);
            settings.initialSys.set("flags0","");
            howMany=1;
          }
          // randomize system name
          std::vector<String> wordPool[6];
          for (int i=0; i<howMany; i++) {
            int wpPos=0;
            DivSystem sysID=e->systemFromFileFur(settings.initialSys.getInt(fmt::sprintf("id%d",i),0));
            String sName=e->getSystemName(sysID);
            String nameWord;
            sName+=" ";
            for (char& i: sName) {
              if (i==' ') {
                if (nameWord!="") {
                  wordPool[wpPos++].push_back(nameWord);
                  if (wpPos>=6) break;
                  nameWord="";
                }
              } else {
                nameWord+=i;
              }
            }
          }
          settings.initialSysName="";
          for (int i=0; i<6; i++) {
            if (wordPool[i].empty()) continue;
            settings.initialSysName+=wordPool[i][rand()%wordPool[i].size()];
            settings.initialSysName+=" ";
          }
          ret=true;
        }
        ImGui::SameLine();
        if (ImGui::Button(_("Reset to defaults"))) {
          settings.initialSys.clear();
          settings.initialSys.set("id0",e->systemToFileFur(DIV_SYSTEM_YM2612));
          settings.initialSys.set("vol0",1.0f);
          settings.initialSys.set("pan0",0.0f);
          settings.initialSys.set("fr0",0.0f);
          settings.initialSys.set("flags0","");
          settings.initialSys.set("id1",e->systemToFileFur(DIV_SYSTEM_SMS));
          settings.initialSys.set("vol1",0.5f);
          settings.initialSys.set("pan1",0.0f);
          settings.initialSys.set("fr1",0.0f);
          settings.initialSys.set("flags1","");
          settings.initialSysName="Sega Genesis/Mega Drive";
          ret=true;
        }

        ImGui::AlignTextToFramePadding();
        ImGui::Text(_("Name"));
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputText("##InitSysName",&settings.initialSysName)) ret=true;

        int sysCount=0;
        int doRemove=-1;
        for (size_t i=0; settings.initialSys.getInt(fmt::sprintf("id%d",i),0); i++) {
          DivSystem sysID=e->systemFromFileFur(settings.initialSys.getInt(fmt::sprintf("id%d",i),0));
          float sysVol=settings.initialSys.getFloat(fmt::sprintf("vol%d",i),0);
          float sysPan=settings.initialSys.getFloat(fmt::sprintf("pan%d",i),0);
          float sysPanFR=settings.initialSys.getFloat(fmt::sprintf("fr%d",i),0);

          sysCount=i+1;

          //bool doRemove=false;
          bool doInvert=(sysVol<0);
          float vol=fabs(sysVol);
          ImGui::PushID(i);

          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x-ImGui::CalcTextSize(_("Invert")).x-ImGui::GetFrameHeightWithSpacing()*2.0-ImGui::GetStyle().ItemSpacing.x*2.0);
            if (ImGui::BeginCombo("##System",getSystemName(sysID),ImGuiComboFlags_HeightLargest)) {

            sysID=systemPicker(true);

            if (sysID!=DIV_SYSTEM_NULL)
            {
              settings.initialSys.set(fmt::sprintf("id%d",i),(int)e->systemToFileFur(sysID));
              settings.initialSys.set(fmt::sprintf("flags%d",i),"");
              ret=true;

              ImGui::CloseCurrentPopup();
            }

            ImGui::EndCombo();
          }

          ImGui::SameLine();
          if (ImGui::Checkbox(_("Invert"),&doInvert)) {
            sysVol=-sysVol;
            settings.initialSys.set(fmt::sprintf("vol%d",i),sysVol);
            ret=true;
          }
          ImGui::SameLine();
          //ImGui::BeginDisabled(settings.initialSys.size()<=4);
          pushDestColor();
          if (ImGui::Button(ICON_FA_MINUS "##InitSysRemove")) {
            doRemove=i;
            ret=true;
          }
          popDestColor();
          //ImGui::EndDisabled();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x-ImGui::GetFrameHeightWithSpacing()*2.0-ImGui::GetStyle().ItemSpacing.x*2.0);
          if (CWSliderFloat(_("Volume"),&vol,0.0f,3.0f)) {
            if (doInvert) {
              if (vol<0.0001) vol=0.0001;
            }
            if (vol<0) vol=0;
            if (vol>10) vol=10;
            sysVol=doInvert?-vol:vol;
            settings.initialSys.set(fmt::sprintf("vol%d",i),(float)sysVol);
            ret=true;
          } rightClickable
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x-ImGui::GetFrameHeightWithSpacing()*2.0-ImGui::GetStyle().ItemSpacing.x*2.0);
          if (CWSliderFloat(_("Panning"),&sysPan,-1.0f,1.0f)) {
            if (sysPan<-1.0f) sysPan=-1.0f;
            if (sysPan>1.0f) sysPan=1.0f;
            settings.initialSys.set(fmt::sprintf("pan%d",i),(float)sysPan);
            ret=true;
          } rightClickable
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x-ImGui::GetFrameHeightWithSpacing()*2.0-ImGui::GetStyle().ItemSpacing.x*2.0);
          if (CWSliderFloat(_("Front/Rear"),&sysPanFR,-1.0f,1.0f)) {
            if (sysPanFR<-1.0f) sysPanFR=-1.0f;
            if (sysPanFR>1.0f) sysPanFR=1.0f;
            settings.initialSys.set(fmt::sprintf("fr%d",i),(float)sysPanFR);
            ret=true;
          } rightClickable

          // oh please MSVC don't cry
          if (ImGui::TreeNode(_("Configure"))) {
            String sysFlagsS=settings.initialSys.getString(fmt::sprintf("flags%d",i),"");
            DivConfig sysFlags;
            sysFlags.loadFromBase64(sysFlagsS.c_str());
            if (drawSysConf(-1,i,sysID,sysFlags,false)) {
              settings.initialSys.set(fmt::sprintf("flags%d",i),sysFlags.toBase64());
            }
            ImGui::TreePop();
            ret=true;
          }

          ImGui::PopID();
        }

        if (doRemove>=0 && sysCount>1) {
          for (int i=doRemove; i<sysCount-1; i++) {
            int sysID=settings.initialSys.getInt(fmt::sprintf("id%d",i+1),0);
            float sysVol=settings.initialSys.getFloat(fmt::sprintf("vol%d",i+1),0);
            float sysPan=settings.initialSys.getFloat(fmt::sprintf("pan%d",i+1),0);
            float sysPanFR=settings.initialSys.getFloat(fmt::sprintf("fr%d",i+1),0);
            String sysFlags=settings.initialSys.getString(fmt::sprintf("flags%d",i+1),"");
            settings.initialSys.set(fmt::sprintf("id%d",i),sysID);
            settings.initialSys.set(fmt::sprintf("vol%d",i),sysVol);
            settings.initialSys.set(fmt::sprintf("pan%d",i),sysPan);
            settings.initialSys.set(fmt::sprintf("fr%d",i),sysPanFR);
            settings.initialSys.set(fmt::sprintf("flags%d",i),sysFlags);
          }

          settings.initialSys.remove(fmt::sprintf("id%d",sysCount-1));
          settings.initialSys.remove(fmt::sprintf("vol%d",sysCount-1));
          settings.initialSys.remove(fmt::sprintf("pan%d",sysCount-1));
          settings.initialSys.remove(fmt::sprintf("fr%d",sysCount-1));
          settings.initialSys.remove(fmt::sprintf("flags%d",sysCount-1));
        }

        if (sysCount<32) if (ImGui::Button(ICON_FA_PLUS "##InitSysAdd")) {
          settings.initialSys.set(fmt::sprintf("id%d",sysCount),(int)e->systemToFileFur(DIV_SYSTEM_YM2612));
          settings.initialSys.set(fmt::sprintf("vol%d",sysCount),1.0f);
          settings.initialSys.set(fmt::sprintf("pan%d",sysCount),0.0f);
          settings.initialSys.set(fmt::sprintf("fr%d",sysCount),0.0f);
          settings.initialSys.set(fmt::sprintf("flags%d",sysCount),"");
        }

        return ret;
      }),
      SettingEntry::Radio(
        _N("When creating new song:"),"newSongBehavior",&settings.newSongBehavior,{
        {_N("Display system preset selector##NSB0"),0},
        {_N("Start with initial system##NSB1"),1},
      }),
      SettingEntry::InputText(
        _N("Default author name"),
        "defaultAuthorName",&settings.defaultAuthorName
      ),
    }),
    SUBCATEGORY(_N("Start-up"),{
#ifndef NO_INTRO
      SettingEntry::Radio(
        _N("Play intro on start-up:"),"alwaysPlayIntro",&settings.alwaysPlayIntro,{
        {_N("No##pis0"),0},
        {_N("Short##pis1"),1},
        {_N("Full (short when loading song)##pis2"),2},
        {_N("Full (always)##pis3"),3},
      }),
#endif
      SETTING_CHECKBOX(
        _N("Disable fade-in during start-up"),
        disableFadeIn
      ),
      SETTING_CHECKBOX(
        _N("Do not maximize on start-up when the Furnace window is too big"),
        noMaximizeWorkaround
      ),
    }),
    SUBCATEGORY(_N("Behavior"),{
      SETTING_CHECKBOX(
        _N("New instruments are blank"),
        blankIns
      ),
      SETTING_CHECKBOX(
        _N("Allow note input with open warning"),
        warnNotePassthrough
      ).Tooltip(_("allows passthrough for notes while warnings are open; only ESC will be used for warnings")),
    }),
    // TODO: configuration import/export/reset options should be in a menu next to search.
    SUBCATEGORY(_N("Import"),{
      SETTING_CHECKBOX(
        _N("Use OPL3 instead of OPL2 for S3M import"),
        s3mOPL3
      ),
      SETTING_CHECKBOX(
        _N("Load sample fine tuning when importing a sample"),
        sampleImportInstDetune
      ).Tooltip(_("this may result in glitches with some samples.")),
    }),
#ifdef ANDROID
    SUBCATEGORY(_N("Android"),{
      SETTING_CHECKBOX(
        _N("Enable background playback (restart!)"),
        backgroundPlay
      )
    }),
#endif
  }
  CATEGORY_END
  CATEGORY_BEGIN(_N("Audio")) {},{
    SUBCATEGORY(_N("Output"),{
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
      }).Callback([this]{
        audioEngineChanged=true;
        settings.audioDevice="";
        settings.audioChans=2;
      }),
#endif
      SettingEntry(_N("Driver"),NULL,[this]{
        bool ret=false;
        if (ImGui::BeginCombo(_("Driver##SDLADriver"),settings.sdlAudioDriver.empty()?_("Automatic"):settings.sdlAudioDriver.c_str())) {
          if (ImGui::Selectable(_("Automatic"),settings.sdlAudioDriver.empty())) {
            settings.sdlAudioDriver="";
            ret=true;
          }
          for (String& i: availAudioDrivers) {
            if (ImGui::Selectable(i.c_str(),i==settings.sdlAudioDriver)) {
              settings.sdlAudioDriver=i;
              ret=true;
            }
          }
          ImGui::EndCombo();
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("you may need to restart Furnace for this setting to take effect."));
        }
        return ret;
      }).Condition([this]{
        return (settings.audioEngine==DIV_AUDIO_SDL);
      }),
      SettingEntry(_N("Device"),NULL,[this]{
        bool ret=false;
        if (audioEngineChanged) {
          ImGui::BeginDisabled();
          if (ImGui::BeginCombo(_("Device##AudioDevice"),_("<click on OK or Apply first>"))) {
            ImGui::Text(_("ALERT - TRESPASSER DETECTED"));
            if (ImGui::IsItemHovered()) {
              showError(_("you have been arrested for trying to engage with a disabled combo box."));
              ImGui::CloseCurrentPopup();
            }
            ImGui::EndCombo();
          }
          ImGui::EndDisabled();
        } else {
          String audioDevName=settings.audioDevice.empty()?_("<System default>"):settings.audioDevice;
          if (ImGui::BeginCombo(_("Device##AudioDevice"),audioDevName.c_str())) {
            if (ImGui::Selectable(_("<System default>"),settings.audioDevice.empty())) {
              settings.audioDevice="";
              ret=true;
            }
            for (String& i: e->getAudioDevices()) {
              if (ImGui::Selectable(i.c_str(),i==settings.audioDevice)) {
                settings.audioDevice=i;
                ret=true;
              }
            }
            ImGui::EndCombo();
          }
        }
        return ret;
      }),
#ifdef HAVE_ASIO
      SettingEntry(_N("ASIO control panel"),NULL,[this]{
        if (ImGui::Button(_("Control panel"))) {
          if (e->audioBackendCommand(TA_AUDIO_CMD_SETUP)!=1) {
            showError(_("this driver doesn't have a control panel."));
          }
        }
        return false;
      }).Condition([this]{return settings.audioEngine==DIV_AUDIO_ASIO;}),
#endif
      SettingEntry::ComboInt(
        _N("Sample rate"),
        "audioRate",&settings.audioRate,{
          {"8000",8000},
          {"16000",16000},
          {"22050",22050},
          {"32000",32000},
          {"44100",44100},
          {"48000",48000},
          {"88200",88200},
          {"96000",96000},
          {"192000",192000}
        }
      ),
      SettingEntry::InputInt(
        "Outputs",
        "audioChans",&settings.audioChans,
        {1,16,1,2}
      ).Tooltip(_("common values:\n- 1 for mono\n- 2 for stereo")),
      // TODO: dynamic label again. this time for latency
      // String bs=fmt::sprintf(_("%d (latency: ~%.1fms)"),settings.audioBufSize,2000.0*(double)settings.audioBufSize/(double)MAX(1,settings.audioRate));
      SettingEntry::ComboInt(
        _N("Buffer size"),
        "audioBufSize",&settings.audioBufSize,{
          {"64",64},
          {"128",128},
          {"256",256},
          {"512",512},
          {"1024",1024},
          {"2048",2048}
        }
      ),
      SettingEntry(_N("Multi-threaded"),NULL,[this]{
        bool ret=false;
        bool renderPoolThreadsB=(settings.renderPoolThreads>0);
        if (ImGui::Checkbox(_("Multi-threaded (EXPERIMENTAL)"),&renderPoolThreadsB)) {
          if (renderPoolThreadsB) {
            settings.renderPoolThreads=2;
          } else {
            settings.renderPoolThreads=0;
          }
          ret=true;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("runs chip emulation on separate threads.\nmay increase performance when using heavy emulation cores.\n\nwarnings:\n- experimental!\n- only useful on multi-chip songs."));
        }

        if (renderPoolThreadsB) {
          pushWarningColor(settings.renderPoolThreads>cpuCores,settings.renderPoolThreads>cpuCores);
          if (ImGui::InputInt(_("Number of threads"),&settings.renderPoolThreads)) {
            if (settings.renderPoolThreads<2) settings.renderPoolThreads=2;
            if (settings.renderPoolThreads>32) settings.renderPoolThreads=32;
            ret=true;
          }
          if (settings.renderPoolThreads>=DIV_MAX_CHIPS) {
            if (ImGui::IsItemHovered()) {
              ImGui::SetTooltip(_("that's the limit!"));
            }
          } else if (settings.renderPoolThreads>cpuCores) {
            if (ImGui::IsItemHovered()) {
              ImGui::SetTooltip(_("it is a VERY bad idea to set this number higher than your CPU core count (%d)!"),cpuCores);
            }
          }
          popWarningColor();
        }
        return ret;
      }),
      SETTING_CHECKBOX(
        _N("Low-latency mode"),
        lowLatency
      ).Tooltip(_("reduces latency by running the engine faster than the tick rate.\nuseful for live playback/jam mode.\n\nwarning: only enable if your buffer size is small (10ms or less).")),
      SETTING_CHECKBOX(
        _N("Force mono audio"),
        forceMono
      ),
      SETTING_CHECKBOX(
        _N("Exclusive mode"),
        wasapiEx
      ).Condition([this]{
        if (settings.audioEngine==DIV_AUDIO_PORTAUDIO) {
          if (settings.audioDevice.find("[Windows WASAPI] ")==0) {
            return true;
          }
        }
        return false;
      }),
      SettingEntry(_N("Audio information"),NULL,[this]{
        TAAudioDesc& audioWant=e->getAudioDescWant();
        TAAudioDesc& audioGot=e->getAudioDescGot();

#ifdef HAVE_LOCALE
        ImGui::Text(ngettext("want: %d samples @ %.0fHz (%d channel)","want: %d samples @ %.0fHz (%d channels)",audioWant.outChans),audioWant.bufsize,audioWant.rate,audioWant.outChans);
        ImGui::Text(ngettext("got: %d samples @ %.0fHz (%d channel)","got: %d samples @ %.0fHz (%d channels)",audioGot.outChans),audioGot.bufsize,audioGot.rate,audioGot.outChans);
#else
        ImGui::Text(_GN("want: %d samples @ %.0fHz (%d channel)","want: %d samples @ %.0fHz (%d channels)",audioWant.outChans),audioWant.bufsize,audioWant.rate,audioWant.outChans);
        ImGui::Text(_GN("got: %d samples @ %.0fHz (%d channel)","got: %d samples @ %.0fHz (%d channels)",audioGot.outChans),audioGot.bufsize,audioGot.rate,audioGot.outChans);
#endif
        return false;
      }),
    }),
    SUBCATEGORY(_N("Mixing"),{
      SettingEntry::ComboInt(
        _N("Quality"),
        "audioQuality",&settings.audioQuality,{
          {_N("High"),0},
          {_N("Low"),1},
        }
      ),
      SETTING_CHECKBOX(
        _N("Software clipping"),
        clampSamples
      ),
      SETTING_CHECKBOX(
        _N("DC offset correction"),
        audioHiPass
      ),
      SettingEntry::SliderInt(
        _N("Metronome volume"),
        "metroVol",&settings.metroVol,
        {0,200,"%d%%"}
      ).Callback([this]{e->setMetronomeVol(((float)settings.metroVol)/100.0f);}),
      SettingEntry::SliderInt(
        _N("Sample preview volume"),
        "sampleVol",&settings.sampleVol,
        {0,100,"%d%%"}
      ).Callback([this]{e->setSamplePreviewVol(((float)settings.sampleVol)/100.0f);})
    })
  }
  CATEGORY_END
  CATEGORY_BEGIN(_N("MIDI")) {},{
    SUBCATEGORY(_N("MIDI input"),{
      SettingEntry(_N("MIDI input device/re-scan"),NULL,[this]{
        bool ret=false;
        ImGui::AlignTextToFramePadding();
        ImGui::Text(_("MIDI input"));
        ImGui::SameLine();
        String midiInName=settings.midiInDevice.empty()?_("<disabled>"):settings.midiInDevice;
        bool hasToReloadMidi=false;
        if (ImGui::BeginCombo("##MidiInDevice",midiInName.c_str())) {
          if (ImGui::Selectable(_("<disabled>"),settings.midiInDevice.empty())) {
            settings.midiInDevice="";
            hasToReloadMidi=true;
            ret=true;
          }
          for (String& i: e->getMidiIns()) {
            if (ImGui::Selectable(i.c_str(),i==settings.midiInDevice)) {
              settings.midiInDevice=i;
              hasToReloadMidi=true;
              ret=true;
            }
          }
          ImGui::EndCombo();
        }

        ImGui::SameLine();
        if (ImGui::Button(_("Re-scan MIDI devices"))) {
          e->rescanMidiDevices();
          audioEngineChanged=true;
          ret=false;
        }

        if (hasToReloadMidi) {
          midiMap.read(e->getConfigPath()+DIR_SEPARATOR_STR+"midiIn_"+stripName(settings.midiInDevice)+".cfg");
          midiMap.compile();
        }

        if (ImGui::Checkbox(_("Note input"),&midiMap.noteInput)) ret=true;
        if (ImGui::Checkbox(_("Velocity input"),&midiMap.volInput)) ret=true;
        // TODO
        //ImGui::Checkbox(_("Use raw velocity value (don't map from linear to log)"),&midiMap.rawVolume);
        //ImGui::Checkbox(_("Polyphonic/chord input"),&midiMap.polyInput);
        if (ImGui::Checkbox(_("Map MIDI channels to direct channels"),&midiMap.directChannel)) {
          e->setMidiDirect(midiMap.directChannel);
          e->setMidiDirectProgram(midiMap.directChannel && midiMap.directProgram);
          ret=true;
        }
        if (midiMap.directChannel) {
          if (ImGui::Checkbox(_("Program change pass-through"),&midiMap.directProgram)) {
            e->setMidiDirectProgram(midiMap.directChannel && midiMap.directProgram);
            ret=true;
          }
        }
        if (ImGui::Checkbox(_("Map Yamaha FM voice data to instruments"),&midiMap.yamahaFMResponse)) ret=true;
        if (!(midiMap.directChannel && midiMap.directProgram)) {
          if (ImGui::Checkbox(_("Program change is instrument selection"),&midiMap.programChange)) ret=true;
        }
        //ImGui::Checkbox(_("Listen to MIDI clock"),&midiMap.midiClock);
        //ImGui::Checkbox(_("Listen to MIDI time code"),&midiMap.midiTimeCode);
        if (ImGui::Combo(_("Value input style"),&midiMap.valueInputStyle,LocalizedComboGetter,valueInputStyles,7)) ret=true;
        if (midiMap.valueInputStyle>3) {
          if (midiMap.valueInputStyle==6) {
            if (ImGui::InputInt(_("Control##valueCCS"),&midiMap.valueInputControlSingle,1,16)) {
              if (midiMap.valueInputControlSingle<0) midiMap.valueInputControlSingle=0;
              if (midiMap.valueInputControlSingle>127) midiMap.valueInputControlSingle=127;
              ret=true;
            }
          } else {
            if (ImGui::InputInt((midiMap.valueInputStyle==4)?_("CC of upper nibble##valueCC1"):_("MSB CC##valueCC1"),&midiMap.valueInputControlMSB,1,16)) {
              if (midiMap.valueInputControlMSB<0) midiMap.valueInputControlMSB=0;
              if (midiMap.valueInputControlMSB>127) midiMap.valueInputControlMSB=127;
              ret=true;
            }
            if (ImGui::InputInt((midiMap.valueInputStyle==4)?_("CC of lower nibble##valueCC2"):_("LSB CC##valueCC2"),&midiMap.valueInputControlLSB,1,16)) {
              if (midiMap.valueInputControlLSB<0) midiMap.valueInputControlLSB=0;
              if (midiMap.valueInputControlLSB>127) midiMap.valueInputControlLSB=127;
              ret=true;
            }
          }
        }

        return ret;
      }),
    }),
    SUBCATEGORY(_N("MIDI output"),{
    }),
  }
  CATEGORY_END
  CATEGORY_BEGIN(_N("Emulation")) {
    SettingEntry::ComboInt(
      _N("PC Speaker strategy"),
      "pcSpeakerOutMethod",&settings.pcSpeakerOutMethod,{
        {_N("evdev SND_TONE"),0},
        {_N("KIOCSOUND on /dev/tty1"),1},
        {_N("/dev/port"),2},
        {_N("KIOCSOUND on standard output"),3},
        {_N("outb()"),4},
      })
  },{
    SUBCATEGORY(_N("Sample ROMs"),{
      SettingEntry::Path(
        _N("OPL4 YRW801 path"),
        "yrw801Path",&settings.yrw801Path,
        GUI_FILE_YRW801_ROM_OPEN
      )
    })
  }
  CATEGORY_END
  CATEGORY_BEGIN(_N("Appearance")) {
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
      {0.0f,2.0f}
    ).Condition([this]{return settings.channelFeedbackStyle==4;})
  },{
    SUBCATEGORY(_N("Scaling"),{
      SettingEntry(_N("Automatic UI scaling factor"),NULL,[this]{
        bool dpiScaleAuto=(settings.dpiScale<0.5f), ret=false;
        if (ImGui::Checkbox(_("Automatic UI scaling factor"),&dpiScaleAuto)) {
          if (dpiScaleAuto) {
            settings.dpiScale=0.0f;
          } else {
            settings.dpiScale=1.0f;
          }
          ret=true;
        }
        return ret;
      }),
      SettingEntry::SliderFloat(
        _N("UI scaling factor"),"dpiScale",
        &settings.dpiScale,
        {1.0f,3.0f}
      ).Condition([this]{return settings.dpiScale>0.5f;}),
      SettingEntry::SliderInt(
        _N("Icon size"),"iconSize",
        &settings.iconSize,
        {3,48}
      ),
    }),
    SUBCATEGORY(_N("Text"),{
#ifdef HAVE_FREETYPE
      SettingEntry::ComboInt(
        _N("Font renderer"),"fontBackend",
        &settings.fontBackend,{
          {"stb_truetype",0},
          {"FreeType",1}
        }
      ),
#endif
      SettingEntry::ComboInt(
        _N("Main font"),"mainFont",
        &settings.mainFont,{
          {"IBM Plex Sans",0},
          {"Liberation Sans",1},
          {"Exo",2},
          {"Proggy Clean",3},
          {"GNU Unifont",4},
          {_N("<Use system font>"),5},
          {_N("<Custom...>"),6}
        }
      ),
      SettingEntry::Path(
        _N("Main font path"),"mainFontPath",
        &settings.mainFontPath,
        GUI_FILE_LOAD_MAIN_FONT
      ).Condition([this]{return settings.mainFont==6;}),
      SettingEntry::InputInt(
        _N("Main font size"),"mainFontSize",
        &settings.mainFontSize,
        {3,96,1,3}
      ),
      SettingEntry::ComboInt(
        _N("Header font"),"headFont",
        &settings.headFont,{
          {"IBM Plex Sans",0},
          {"Liberation Sans",1},
          {"Exo",2},
          {"Proggy Clean",3},
          {"GNU Unifont",4},
          {_N("<Use system font>"),5},
          {_N("<Custom...>"),6}
        }
      ),
      SettingEntry::Path(
        _N("Header font path"),"headFontPath",
        &settings.headFontPath,
        GUI_FILE_LOAD_HEAD_FONT
      ).Condition([this]{return settings.headFont==6;}),
      SettingEntry::InputInt(
        _N("Header font size"),"headFontSize",
        &settings.headFontSize,
        {3,96,1,3}
      ),
      SettingEntry::ComboInt(
        _N("Pattern font"),"patFont",
        &settings.patFont,{
          {"IBM Plex Sans",0},
          {"Liberation Sans",1},
          {"Mononoki",2},
          {"PT Mono",3},
          {"GNU Unifont",4},
          {_N("<Use system font>"),5},
          {_N("<Custom...>"),6}
        }
      ),
      SettingEntry::Path(
        _N("Pattern font path"),"patFontPath",
        &settings.patFontPath,
        GUI_FILE_LOAD_PAT_FONT
      ).Condition([this]{return settings.patFont==6;}),
      SettingEntry::InputInt(
        _N("Pattern font size"),"patFontSize",
        &settings.patFontSize,
        {3,96,1,3}
      ),
      SETTING_CHECKBOX(
        _N("Anti-aliased fonts"),fontAntiAlias
      ).Condition([this]{return settings.fontBackend==1;}),
      SETTING_CHECKBOX(
        _N("Support bitmap fonts"),fontBitmap
      ).Condition([this]{return settings.fontBackend==1;}),
      SettingEntry::Radio(
        _N("Hinting:"),"fontHinting",
        &settings.fontHinting,{
          {_N("Off (soft)##fh0"),0},
          {_N("Slight##fh1"),1},
          {_N("Normal##fh2"),2},
          {_N("Full (hard)##fh3"),3}
        }
      ).Condition([this]{return settings.fontBackend==1;}),
      SettingEntry::Radio(
        _N("Auto-hinter:"),"fontAutoHint",
        &settings.fontAutoHint,{
          {_N("Disable##fah0"),0},
          {_N("Enable##fah1"),1},
          {_N("Force##fah2"),2}
        }
      ).Condition([this]{return settings.fontBackend==1;}),
      SettingEntry::Radio(
        _N("Oversample:"),"fontOversample",
        &settings.fontOversample,{
          {_N("1×##fos1"),1,_N("saves video memory. reduces font rendering quality.\nuse for pixel/bitmap fonts.")},
          {_N("2×##fos2"),2,_N("default.")},
          {_N("3×##fos3"),3,_N("slightly better font rendering quality.\nuses more video memory.")}
        }
      ),
      SETTING_CHECKBOX(
        _N("Load fallback font"),loadFallback
      ).Tooltip(_N("disable to save video memory.")),
      SETTING_CHECKBOX(
        _N("Load fallback font (pattern)"),loadFallbackPat
      ).Tooltip(_N("disable to save video memory.")),
    }),
    SUBCATEGORY(_N("Program"),{
      SettingEntry::Radio(
        _N("Title bar:"),"titleBarInfo",
        &settings.titleBarInfo,{
          {_N("Furnace##tbar0"),0},
          {_N("Song Name - Furnace##tbar1"),1},
          {_N("file_name.fur - Furnace##tbar2"),2},
          {_N("/path/to/file.fur - Furnace##tbar3"),3}
        }
      ).Callback([this]{updateWindowTitle();}),
      SETTING_CHECKBOX(
        _N("Display system name on title bar"),titleBarSys
      ).Callback([this]{updateWindowTitle();}),
      SETTING_CHECKBOX(
        _N("Display chip names instead of \"multi-system\" in title bar"),noMultiSystem
      ).Callback([this]{updateWindowTitle();}),
      SettingEntry::Radio(
        _N("Status bar:"),"statusDisplay",
        &settings.statusDisplay,{
          {_N("Cursor details##sbar0"),0},
          {_N("File path##sbar1"),1},
          {_N("Cursor details or file path##sbar2"),2},
          {_N("Nothing##sbar3"),3}
        }
      ),
      SETTING_CHECKBOX(_("Display playback status when playing"),playbackTime),
      SettingEntry::Radio(
        _N("Export options layout:"),"exportOptionsLayout",
        &settings.exportOptionsLayout,{
          {_N("Sub-menus in File menu##eol0"),0},
          {_N("Modal window with tabs##eol1"),1},
          {_N("Modal windows with options in File menu##eol2"),2}
        }
      ),
      SETTING_CHECKBOX(_N("Capitalize menu bar"),capitalMenuBar),
      SETTING_CHECKBOX(_N("Display add/configure/change/remove chip menus in File menu"),classicChipOptions)
    }),
    SUBCATEGORY(_N("Orders"),{
      // sorry. temporarily disabled until ImGui has a way to add separators in tables arbitrarily.
      // SETTING_CHECKBOX(_N("Add separators between systems in Orders"),sysSeparators),
      SETTING_CHECKBOX(_N("Orders row number format:"),ordersCursor),
      SettingEntry::Radio(
        _N("Orders row number format:"),"orderRowsBase",
        &settings.orderRowsBase,{
          {_N("Decimal##orbD"),0},
          {_N("Hexadecimal##orbH"),1}
        }
      ),
    }),
    SUBCATEGORY(_N("Pattern"),{
      SETTING_CHECKBOX(_N("Center pattern view"),centerPattern),
      SETTING_CHECKBOX(_N("Overflow pattern highlights"),overflowHighlight),
      SETTING_CHECKBOX(_N("Display previous/next pattern"),viewPrevPattern),
      SettingEntry::Radio(
        _N("Pattern row number format:"),"patRowsBase",
        &settings.patRowsBase,{
          {_N("Decimal##prbD"),0},
          {_N("Hexadecimal##prbH"),1}
        }
      ),
      SETTING_CHECKBOX(_("Single-digit effects for 00-0F"),oneDigitEffects),
      SETTING_CHECKBOX(_("Use flats instead of sharps"),flatNotes),
      SETTING_CHECKBOX(_("Use German notation"),germanNotation),
    },{
      SUBCATEGORY(_N("Pattern view labels"),{
        SettingEntry::InputText(
          _N("Note off (3-char)"),
          "noteOffLabel",&settings.noteOffLabel,
          "OFF"
        ),
        SettingEntry::InputText(
          _N("Note release (3-char)"),
          "noteRelLabel",&settings.noteRelLabel,
          "==="
        ),
        SettingEntry::InputText(
          _N("Macro release (3-char)"),
          "macroRelLabel",&settings.macroRelLabel,
          "REL"
        ),
        SettingEntry::InputText(
          _N("Empty field (3-char)"),
          "emptyLabel",&settings.emptyLabel,
          "..."
        ),
        SettingEntry::InputText(
          _N("Empty field (2-char)"),
          "emptyLabel2",&settings.emptyLabel2,
          ".."
        ),
      }),
      SUBCATEGORY(_N("Pattern view spacing after:"),{
        SettingEntry::InputInt(
          _N("Note"),"noteCellSpacing",
          &settings.noteCellSpacing,
          {0,32}
        ),
        SettingEntry::InputInt(
          _N("Instrument"),"insCellSpacing",
          &settings.insCellSpacing,
          {0,32}
        ),
        SettingEntry::InputInt(
          _N("Volume"),"volCellSpacing",
          &settings.volCellSpacing,
          {0,32}
        ),
        SettingEntry::InputInt(
          _N("Effect"),"effectCellSpacing",
          &settings.effectCellSpacing,
          {0,32}
        ),
        SettingEntry::InputInt(
          _N("Effect value"),"effectValCellSpacing",
          &settings.effectValCellSpacing,
          {0,32}
        ),
      })
    })
  }
  CATEGORY_END
  CATEGORY_BEGIN(_N("Color")) {
    SETTING_INV_CHECKBOX(_N("Guru mode"),basicColors),
    SettingEntry::SliderInt(
      _N("Frame shading"),
      "guiColorsShading",&settings.guiColorsShading,
      {0,100,"%d%%"}
    ).Callback([this]{applyUISettings(false);}),
    SettingEntry::Radio(
      _N("Color scheme type:"),
      "guiColorsBase",&settings.guiColorsBase,
      {
        {_N("Dark##gcb0"),0},
        {_N("Light##gcb1"),1}
      }
    ).Callback([this]{applyUISettings(false);})
    .Condition([this]{return settings.basicColors;}),
  },{
    SUBCATEGORY(_N("Interface"),{
#define BASIC_MODE .Condition([this]{return settings.basicColors;})
      SETTING_COLOR(GUI_COLOR_ACCENT_PRIMARY) BASIC_MODE,
      SETTING_COLOR(GUI_COLOR_ACCENT_SECONDARY) BASIC_MODE,
#define GURU_MODE .Condition([this]{return !settings.basicColors;})
      SETTING_COLOR(GUI_COLOR_BUTTON) GURU_MODE,
      SETTING_COLOR(GUI_COLOR_BUTTON_HOVER) GURU_MODE,
      SETTING_COLOR(GUI_COLOR_BUTTON_ACTIVE) GURU_MODE,
      SETTING_COLOR(GUI_COLOR_TAB) GURU_MODE,
      SETTING_COLOR(GUI_COLOR_TAB_HOVER) GURU_MODE,
      SETTING_COLOR(GUI_COLOR_TAB_ACTIVE) GURU_MODE,
      SETTING_COLOR(GUI_COLOR_TAB_SELECTED_OVERLINE) GURU_MODE,
      SETTING_COLOR(GUI_COLOR_TAB_UNFOCUSED) GURU_MODE,
      SETTING_COLOR(GUI_COLOR_TAB_UNFOCUSED_ACTIVE) GURU_MODE,
      SETTING_COLOR(GUI_COLOR_TAB_DIMMED_SELECTED_OVERLINE) GURU_MODE,
      SETTING_COLOR(GUI_COLOR_IMGUI_HEADER) GURU_MODE,
      SETTING_COLOR(GUI_COLOR_IMGUI_HEADER_HOVER) GURU_MODE,
      SETTING_COLOR(GUI_COLOR_IMGUI_HEADER_ACTIVE) GURU_MODE,
      SETTING_COLOR(GUI_COLOR_RESIZE_GRIP) GURU_MODE,
      SETTING_COLOR(GUI_COLOR_RESIZE_GRIP_HOVER) GURU_MODE,
      SETTING_COLOR(GUI_COLOR_RESIZE_GRIP_ACTIVE) GURU_MODE,
      SETTING_COLOR(GUI_COLOR_WIDGET_BACKGROUND) GURU_MODE,
      SETTING_COLOR(GUI_COLOR_WIDGET_BACKGROUND_HOVER) GURU_MODE,
      SETTING_COLOR(GUI_COLOR_WIDGET_BACKGROUND_ACTIVE) GURU_MODE,
      SETTING_COLOR(GUI_COLOR_SLIDER_GRAB) GURU_MODE,
      SETTING_COLOR(GUI_COLOR_SLIDER_GRAB_ACTIVE) GURU_MODE,
      SETTING_COLOR(GUI_COLOR_TITLE_BACKGROUND_ACTIVE) GURU_MODE,
      SETTING_COLOR(GUI_COLOR_CHECK_MARK) GURU_MODE,
      SETTING_COLOR(GUI_COLOR_TEXT_LINK) GURU_MODE,
      SETTING_COLOR(GUI_COLOR_TEXT_SELECTION) GURU_MODE,
      SETTING_COLOR(GUI_COLOR_TREE_LINES) GURU_MODE,
      SETTING_COLOR(GUI_COLOR_PLOT_LINES) GURU_MODE,
      SETTING_COLOR(GUI_COLOR_PLOT_LINES_HOVER) GURU_MODE,
      SETTING_COLOR(GUI_COLOR_PLOT_HISTOGRAM) GURU_MODE,
      SETTING_COLOR(GUI_COLOR_PLOT_HISTOGRAM_HOVER) GURU_MODE,
      SETTING_COLOR(GUI_COLOR_TABLE_ROW_EVEN) GURU_MODE,
      SETTING_COLOR(GUI_COLOR_TABLE_ROW_ODD) GURU_MODE,
    }),
    SUBCATEGORY(_N("Interface (other)"),{
      SETTING_COLOR(GUI_COLOR_BACKGROUND),
      SETTING_COLOR(GUI_COLOR_FRAME_BACKGROUND),
      SETTING_COLOR(GUI_COLOR_FRAME_BACKGROUND_CHILD),
      SETTING_COLOR(GUI_COLOR_FRAME_BACKGROUND_POPUP),
      SETTING_COLOR(GUI_COLOR_MODAL_BACKDROP),
      SETTING_COLOR(GUI_COLOR_HEADER),
      SETTING_COLOR(GUI_COLOR_TEXT),
      SETTING_COLOR(GUI_COLOR_TEXT_DISABLED),
      SETTING_COLOR(GUI_COLOR_TITLE_INACTIVE),
      SETTING_COLOR(GUI_COLOR_TITLE_COLLAPSED),
      SETTING_COLOR(GUI_COLOR_MENU_BAR),
      SETTING_COLOR(GUI_COLOR_BORDER),
      SETTING_COLOR(GUI_COLOR_BORDER_SHADOW),
      SETTING_COLOR(GUI_COLOR_SCROLL),
      SETTING_COLOR(GUI_COLOR_SCROLL_HOVER),
      SETTING_COLOR(GUI_COLOR_SCROLL_ACTIVE),
      SETTING_COLOR(GUI_COLOR_SCROLL_BACKGROUND),
      SETTING_COLOR(GUI_COLOR_SEPARATOR),
      SETTING_COLOR(GUI_COLOR_SEPARATOR_HOVER),
      SETTING_COLOR(GUI_COLOR_SEPARATOR_ACTIVE),
      SETTING_COLOR(GUI_COLOR_DOCKING_PREVIEW),
      SETTING_COLOR(GUI_COLOR_DOCKING_EMPTY),
      SETTING_COLOR(GUI_COLOR_TABLE_HEADER),
      SETTING_COLOR(GUI_COLOR_TABLE_BORDER_HARD),
      SETTING_COLOR(GUI_COLOR_TABLE_BORDER_SOFT),
      SETTING_COLOR(GUI_COLOR_DRAG_DROP_TARGET),
      SETTING_COLOR(GUI_COLOR_NAV_WIN_HIGHLIGHT),
      SETTING_COLOR(GUI_COLOR_NAV_WIN_BACKDROP),
      SETTING_COLOR(GUI_COLOR_INPUT_TEXT_CURSOR),
    }),
    SUBCATEGORY(_N("Miscellaneous"), {
      SETTING_COLOR(GUI_COLOR_TOGGLE_ON),
      SETTING_COLOR(GUI_COLOR_TOGGLE_OFF),
      SETTING_COLOR(GUI_COLOR_PLAYBACK_STAT),
      SETTING_COLOR(GUI_COLOR_DESTRUCTIVE),
      SETTING_COLOR(GUI_COLOR_WARNING),
      SETTING_COLOR(GUI_COLOR_ERROR),
    }),
    SUBCATEGORY(_N("File Picker (built-in)"), {
      SETTING_COLOR(GUI_COLOR_FILE_DIR),
      SETTING_COLOR(GUI_COLOR_FILE_SONG_NATIVE),
      SETTING_COLOR(GUI_COLOR_FILE_SONG_IMPORT),
      SETTING_COLOR(GUI_COLOR_FILE_INSTR),
      SETTING_COLOR(GUI_COLOR_FILE_AUDIO),
      SETTING_COLOR(GUI_COLOR_FILE_AUDIO_COMPRESSED),
      SETTING_COLOR(GUI_COLOR_FILE_WAVE),
      SETTING_COLOR(GUI_COLOR_FILE_VGM),
      SETTING_COLOR(GUI_COLOR_FILE_ZSM),
      SETTING_COLOR(GUI_COLOR_FILE_FONT),
      SETTING_COLOR(GUI_COLOR_FILE_OTHER),
    }),
    SUBCATEGORY(_N("Oscilloscope"), {
      SETTING_COLOR(GUI_COLOR_OSC_BORDER),
      SETTING_COLOR(GUI_COLOR_OSC_BG1),
      SETTING_COLOR(GUI_COLOR_OSC_BG2),
      SETTING_COLOR(GUI_COLOR_OSC_BG3),
      SETTING_COLOR(GUI_COLOR_OSC_BG4),
      SETTING_COLOR(GUI_COLOR_OSC_WAVE),
      SETTING_COLOR(GUI_COLOR_OSC_WAVE_PEAK),
      SETTING_COLOR(GUI_COLOR_OSC_REF),
      SETTING_COLOR(GUI_COLOR_OSC_GUIDE),
    },{
      SUBCATEGORY(_N("Wave (non-mono)"), {
        SETTING_COLOR(GUI_COLOR_OSC_WAVE_CH0),
        SETTING_COLOR(GUI_COLOR_OSC_WAVE_CH1),
        SETTING_COLOR(GUI_COLOR_OSC_WAVE_CH2),
        SETTING_COLOR(GUI_COLOR_OSC_WAVE_CH3),
        SETTING_COLOR(GUI_COLOR_OSC_WAVE_CH4),
        SETTING_COLOR(GUI_COLOR_OSC_WAVE_CH5),
        SETTING_COLOR(GUI_COLOR_OSC_WAVE_CH6),
        SETTING_COLOR(GUI_COLOR_OSC_WAVE_CH7),
        SETTING_COLOR(GUI_COLOR_OSC_WAVE_CH8),
        SETTING_COLOR(GUI_COLOR_OSC_WAVE_CH9),
        SETTING_COLOR(GUI_COLOR_OSC_WAVE_CH10),
        SETTING_COLOR(GUI_COLOR_OSC_WAVE_CH11),
        SETTING_COLOR(GUI_COLOR_OSC_WAVE_CH12),
        SETTING_COLOR(GUI_COLOR_OSC_WAVE_CH13),
        SETTING_COLOR(GUI_COLOR_OSC_WAVE_CH14),
        SETTING_COLOR(GUI_COLOR_OSC_WAVE_CH15),
      }),
    }),
    SUBCATEGORY(_N("Volume Meter"), {
      SETTING_COLOR(GUI_COLOR_VOLMETER_LOW),
      SETTING_COLOR(GUI_COLOR_VOLMETER_HIGH),
      SETTING_COLOR(GUI_COLOR_VOLMETER_PEAK),
    }),
    SUBCATEGORY(_N("Orders"), {
      SETTING_COLOR(GUI_COLOR_ORDER_ROW_INDEX),
      SETTING_COLOR(GUI_COLOR_ORDER_ACTIVE),
      SETTING_COLOR(GUI_COLOR_SONG_LOOP),
      SETTING_COLOR(GUI_COLOR_ORDER_SELECTED),
      SETTING_COLOR(GUI_COLOR_ORDER_SIMILAR),
      SETTING_COLOR(GUI_COLOR_ORDER_INACTIVE),
    }),
    SUBCATEGORY(_N("Envelope View"), {
      SETTING_COLOR(GUI_COLOR_FM_ENVELOPE),
      SETTING_COLOR(GUI_COLOR_FM_ENVELOPE_SUS_GUIDE),
      SETTING_COLOR(GUI_COLOR_FM_ENVELOPE_RELEASE),}
    ),
    SUBCATEGORY(_N("FM Editor"), {
      SETTING_COLOR(GUI_COLOR_FM_ALG_BG),
      SETTING_COLOR(GUI_COLOR_FM_ALG_LINE),
      SETTING_COLOR(GUI_COLOR_FM_MOD),
      SETTING_COLOR(GUI_COLOR_FM_CAR),

      SETTING_COLOR(GUI_COLOR_FM_SSG),
      SETTING_COLOR(GUI_COLOR_FM_WAVE),

      SETTING_COLOR(GUI_COLOR_FM_PRIMARY_MOD),
      SETTING_COLOR(GUI_COLOR_FM_SECONDARY_MOD),
      SETTING_COLOR(GUI_COLOR_FM_BORDER_MOD),
      SETTING_COLOR(GUI_COLOR_FM_BORDER_SHADOW_MOD),

      SETTING_COLOR(GUI_COLOR_FM_PRIMARY_CAR),
      SETTING_COLOR(GUI_COLOR_FM_SECONDARY_CAR),
      SETTING_COLOR(GUI_COLOR_FM_BORDER_CAR),
      SETTING_COLOR(GUI_COLOR_FM_BORDER_SHADOW_CAR),}
    ),
    SUBCATEGORY(_N("Macro Editor"), {
      SETTING_COLOR(GUI_COLOR_MACRO_VOLUME),
      SETTING_COLOR(GUI_COLOR_MACRO_PITCH),
      SETTING_COLOR(GUI_COLOR_MACRO_WAVE),
      SETTING_COLOR(GUI_COLOR_MACRO_NOISE),
      SETTING_COLOR(GUI_COLOR_MACRO_FILTER),
      SETTING_COLOR(GUI_COLOR_MACRO_ENVELOPE),
      SETTING_COLOR(GUI_COLOR_MACRO_GLOBAL),
      SETTING_COLOR(GUI_COLOR_MACRO_OTHER),
      SETTING_COLOR(GUI_COLOR_MACRO_HIGHLIGHT),
    }),
    SUBCATEGORY(_N("Multi-instrument Play"), {
      SETTING_COLOR(GUI_COLOR_MULTI_INS_1),
      SETTING_COLOR(GUI_COLOR_MULTI_INS_2),
      SETTING_COLOR(GUI_COLOR_MULTI_INS_3),
      SETTING_COLOR(GUI_COLOR_MULTI_INS_4),
      SETTING_COLOR(GUI_COLOR_MULTI_INS_5),
      SETTING_COLOR(GUI_COLOR_MULTI_INS_6),
      SETTING_COLOR(GUI_COLOR_MULTI_INS_7),
    }),
    SUBCATEGORY(_N("Instrument Types"), {
      SETTING_COLOR(GUI_COLOR_INSTR_FM),
      SETTING_COLOR(GUI_COLOR_INSTR_STD),
      SETTING_COLOR(GUI_COLOR_INSTR_T6W28),
      SETTING_COLOR(GUI_COLOR_INSTR_GB),
      SETTING_COLOR(GUI_COLOR_INSTR_C64),
      SETTING_COLOR(GUI_COLOR_INSTR_AMIGA),
      SETTING_COLOR(GUI_COLOR_INSTR_PCE),
      SETTING_COLOR(GUI_COLOR_INSTR_AY),
      SETTING_COLOR(GUI_COLOR_INSTR_AY8930),
      SETTING_COLOR(GUI_COLOR_INSTR_TIA),
      SETTING_COLOR(GUI_COLOR_INSTR_SAA1099),
      SETTING_COLOR(GUI_COLOR_INSTR_VIC),
      SETTING_COLOR(GUI_COLOR_INSTR_PET),
      SETTING_COLOR(GUI_COLOR_INSTR_VRC6),
      SETTING_COLOR(GUI_COLOR_INSTR_VRC6_SAW),
      SETTING_COLOR(GUI_COLOR_INSTR_OPLL),
      SETTING_COLOR(GUI_COLOR_INSTR_OPL),
      SETTING_COLOR(GUI_COLOR_INSTR_FDS),
      SETTING_COLOR(GUI_COLOR_INSTR_VBOY),
      SETTING_COLOR(GUI_COLOR_INSTR_N163),
      SETTING_COLOR(GUI_COLOR_INSTR_SCC),
      SETTING_COLOR(GUI_COLOR_INSTR_OPZ),
      SETTING_COLOR(GUI_COLOR_INSTR_POKEY),
      SETTING_COLOR(GUI_COLOR_INSTR_BEEPER),
      SETTING_COLOR(GUI_COLOR_INSTR_SWAN),
      SETTING_COLOR(GUI_COLOR_INSTR_MIKEY),
      SETTING_COLOR(GUI_COLOR_INSTR_VERA),
      SETTING_COLOR(GUI_COLOR_INSTR_X1_010),
      SETTING_COLOR(GUI_COLOR_INSTR_ES5506),
      SETTING_COLOR(GUI_COLOR_INSTR_MULTIPCM),
      SETTING_COLOR(GUI_COLOR_INSTR_SNES),
      SETTING_COLOR(GUI_COLOR_INSTR_SU),
      SETTING_COLOR(GUI_COLOR_INSTR_NAMCO),
      SETTING_COLOR(GUI_COLOR_INSTR_OPL_DRUMS),
      SETTING_COLOR(GUI_COLOR_INSTR_OPM),
      SETTING_COLOR(GUI_COLOR_INSTR_NES),
      SETTING_COLOR(GUI_COLOR_INSTR_MSM6258),
      SETTING_COLOR(GUI_COLOR_INSTR_MSM6295),
      SETTING_COLOR(GUI_COLOR_INSTR_ADPCMA),
      SETTING_COLOR(GUI_COLOR_INSTR_ADPCMB),
      SETTING_COLOR(GUI_COLOR_INSTR_SEGAPCM),
      SETTING_COLOR(GUI_COLOR_INSTR_QSOUND),
      SETTING_COLOR(GUI_COLOR_INSTR_YMZ280B),
      SETTING_COLOR(GUI_COLOR_INSTR_RF5C68),
      SETTING_COLOR(GUI_COLOR_INSTR_MSM5232),
      SETTING_COLOR(GUI_COLOR_INSTR_K007232),
      SETTING_COLOR(GUI_COLOR_INSTR_GA20),
      SETTING_COLOR(GUI_COLOR_INSTR_POKEMINI),
      SETTING_COLOR(GUI_COLOR_INSTR_SM8521),
      SETTING_COLOR(GUI_COLOR_INSTR_PV1000),
      SETTING_COLOR(GUI_COLOR_INSTR_K053260),
      SETTING_COLOR(GUI_COLOR_INSTR_TED),
      SETTING_COLOR(GUI_COLOR_INSTR_C140),
      SETTING_COLOR(GUI_COLOR_INSTR_C219),
      SETTING_COLOR(GUI_COLOR_INSTR_ESFM),
      SETTING_COLOR(GUI_COLOR_INSTR_POWERNOISE),
      SETTING_COLOR(GUI_COLOR_INSTR_POWERNOISE_SLOPE),
      SETTING_COLOR(GUI_COLOR_INSTR_DAVE),
      SETTING_COLOR(GUI_COLOR_INSTR_NDS),
      SETTING_COLOR(GUI_COLOR_INSTR_GBA_DMA),
      SETTING_COLOR(GUI_COLOR_INSTR_GBA_MINMOD),
      SETTING_COLOR(GUI_COLOR_INSTR_BIFURCATOR),
      SETTING_COLOR(GUI_COLOR_INSTR_SID2),
      SETTING_COLOR(GUI_COLOR_INSTR_SUPERVISION),
      SETTING_COLOR(GUI_COLOR_INSTR_UPD1771C),
      SETTING_COLOR(GUI_COLOR_INSTR_SID3),
      SETTING_COLOR(GUI_COLOR_INSTR_UNKNOWN),
    }),
    SUBCATEGORY(_N("Channel"), {
      SETTING_COLOR(GUI_COLOR_CHANNEL_BG),
      SETTING_COLOR(GUI_COLOR_CHANNEL_FG),
      SETTING_COLOR(GUI_COLOR_CHANNEL_FM),
      SETTING_COLOR(GUI_COLOR_CHANNEL_PULSE),
      SETTING_COLOR(GUI_COLOR_CHANNEL_NOISE),
      SETTING_COLOR(GUI_COLOR_CHANNEL_PCM),
      SETTING_COLOR(GUI_COLOR_CHANNEL_WAVE),
      SETTING_COLOR(GUI_COLOR_CHANNEL_OP),
      SETTING_COLOR(GUI_COLOR_CHANNEL_MUTED),
    }),
    SUBCATEGORY(_N("Pattern"), {
      SETTING_COLOR(GUI_COLOR_PATTERN_BG),
      SETTING_COLOR(GUI_COLOR_PATTERN_PLAY_HEAD),
      SETTING_COLOR(GUI_COLOR_EDITING),
      SETTING_COLOR(GUI_COLOR_EDITING_CLONE),
      SETTING_COLOR(GUI_COLOR_PATTERN_CURSOR),
      SETTING_COLOR(GUI_COLOR_PATTERN_CURSOR_HOVER),
      SETTING_COLOR(GUI_COLOR_PATTERN_CURSOR_ACTIVE),
      SETTING_COLOR(GUI_COLOR_PATTERN_SELECTION),
      SETTING_COLOR(GUI_COLOR_PATTERN_SELECTION_HOVER),
      SETTING_COLOR(GUI_COLOR_PATTERN_SELECTION_ACTIVE),
      SETTING_COLOR(GUI_COLOR_PATTERN_HI_1),
      SETTING_COLOR(GUI_COLOR_PATTERN_HI_2),
      SETTING_COLOR(GUI_COLOR_PATTERN_ROW_INDEX),
      SETTING_COLOR(GUI_COLOR_PATTERN_ROW_INDEX_HI1),
      SETTING_COLOR(GUI_COLOR_PATTERN_ROW_INDEX_HI2),
      SETTING_COLOR(GUI_COLOR_PATTERN_ACTIVE),
      SETTING_COLOR(GUI_COLOR_PATTERN_ACTIVE_HI1),
      SETTING_COLOR(GUI_COLOR_PATTERN_ACTIVE_HI2),
      SETTING_COLOR(GUI_COLOR_PATTERN_INACTIVE),
      SETTING_COLOR(GUI_COLOR_PATTERN_INACTIVE_HI1),
      SETTING_COLOR(GUI_COLOR_PATTERN_INACTIVE_HI2),
      SETTING_COLOR(GUI_COLOR_PATTERN_INS),
      SETTING_COLOR(GUI_COLOR_PATTERN_INS_WARN),
      SETTING_COLOR(GUI_COLOR_PATTERN_INS_ERROR),
      SETTING_COLOR(GUI_COLOR_PATTERN_VOLUME_MIN),
      SETTING_COLOR(GUI_COLOR_PATTERN_VOLUME_HALF),
      SETTING_COLOR(GUI_COLOR_PATTERN_VOLUME_MAX),
      SETTING_COLOR(GUI_COLOR_PATTERN_EFFECT_INVALID),
      SETTING_COLOR(GUI_COLOR_PATTERN_EFFECT_PITCH),
      SETTING_COLOR(GUI_COLOR_PATTERN_EFFECT_VOLUME),
      SETTING_COLOR(GUI_COLOR_PATTERN_EFFECT_PANNING),
      SETTING_COLOR(GUI_COLOR_PATTERN_EFFECT_SONG),
      SETTING_COLOR(GUI_COLOR_PATTERN_EFFECT_TIME),
      SETTING_COLOR(GUI_COLOR_PATTERN_EFFECT_SPEED),
      SETTING_COLOR(GUI_COLOR_PATTERN_EFFECT_SYS_PRIMARY),
      SETTING_COLOR(GUI_COLOR_PATTERN_EFFECT_SYS_SECONDARY),
      SETTING_COLOR(GUI_COLOR_PATTERN_EFFECT_MISC),
      SETTING_COLOR(GUI_COLOR_EE_VALUE),
      SETTING_COLOR(GUI_COLOR_PATTERN_STATUS_OFF),
      SETTING_COLOR(GUI_COLOR_PATTERN_STATUS_REL),
      SETTING_COLOR(GUI_COLOR_PATTERN_STATUS_REL_ON),
      SETTING_COLOR(GUI_COLOR_PATTERN_STATUS_ON),
      SETTING_COLOR(GUI_COLOR_PATTERN_STATUS_VOLUME),
      SETTING_COLOR(GUI_COLOR_PATTERN_STATUS_PITCH),
      SETTING_COLOR(GUI_COLOR_PATTERN_STATUS_PANNING),
      SETTING_COLOR(GUI_COLOR_PATTERN_STATUS_SYS1),
      SETTING_COLOR(GUI_COLOR_PATTERN_STATUS_SYS2),
      SETTING_COLOR(GUI_COLOR_PATTERN_STATUS_MIXING),
      SETTING_COLOR(GUI_COLOR_PATTERN_STATUS_DSP),
      SETTING_COLOR(GUI_COLOR_PATTERN_STATUS_NOTE),
      SETTING_COLOR(GUI_COLOR_PATTERN_STATUS_MISC1),
      SETTING_COLOR(GUI_COLOR_PATTERN_STATUS_MISC2),
      SETTING_COLOR(GUI_COLOR_PATTERN_STATUS_MISC3),
      SETTING_COLOR(GUI_COLOR_PATTERN_STATUS_ATTACK),
      SETTING_COLOR(GUI_COLOR_PATTERN_STATUS_DECAY),
      SETTING_COLOR(GUI_COLOR_PATTERN_STATUS_SUSTAIN),
      SETTING_COLOR(GUI_COLOR_PATTERN_STATUS_RELEASE),
      SETTING_COLOR(GUI_COLOR_PATTERN_STATUS_DEC_LINEAR),
      SETTING_COLOR(GUI_COLOR_PATTERN_STATUS_DEC_EXP),
      SETTING_COLOR(GUI_COLOR_PATTERN_STATUS_INC),
      SETTING_COLOR(GUI_COLOR_PATTERN_STATUS_BENT),
      SETTING_COLOR(GUI_COLOR_PATTERN_STATUS_DIRECT),
      SETTING_COLOR(GUI_COLOR_PATTERN_STATUS_WARNING),
      SETTING_COLOR(GUI_COLOR_PATTERN_STATUS_ERROR),
    }),
    SUBCATEGORY(_N("Sample Editor"), {
      SETTING_COLOR(GUI_COLOR_SAMPLE_BG),
      SETTING_COLOR(GUI_COLOR_SAMPLE_FG),
      SETTING_COLOR(GUI_COLOR_SAMPLE_TIME_BG),
      SETTING_COLOR(GUI_COLOR_SAMPLE_TIME_FG),
      SETTING_COLOR(GUI_COLOR_SAMPLE_LOOP),
      SETTING_COLOR(GUI_COLOR_SAMPLE_CENTER),
      SETTING_COLOR(GUI_COLOR_SAMPLE_GRID),
      SETTING_COLOR(GUI_COLOR_SAMPLE_SEL),
      SETTING_COLOR(GUI_COLOR_SAMPLE_SEL_POINT),
      SETTING_COLOR(GUI_COLOR_SAMPLE_NEEDLE),
      SETTING_COLOR(GUI_COLOR_SAMPLE_NEEDLE_PLAYING),
      SETTING_COLOR(GUI_COLOR_SAMPLE_LOOP_POINT),
      SETTING_COLOR(GUI_COLOR_SAMPLE_LOOP_HINT),
      SETTING_COLOR(GUI_COLOR_SAMPLE_CHIP_DISABLED),
      SETTING_COLOR(GUI_COLOR_SAMPLE_CHIP_ENABLED),
      SETTING_COLOR(GUI_COLOR_SAMPLE_CHIP_WARNING),
    }),
    SUBCATEGORY(_N("Pattern Manager"), {
      SETTING_COLOR(GUI_COLOR_PAT_MANAGER_NULL),
      SETTING_COLOR(GUI_COLOR_PAT_MANAGER_UNUSED),
      SETTING_COLOR(GUI_COLOR_PAT_MANAGER_USED),
      SETTING_COLOR(GUI_COLOR_PAT_MANAGER_OVERUSED),
      SETTING_COLOR(GUI_COLOR_PAT_MANAGER_EXTREMELY_OVERUSED),
      SETTING_COLOR(GUI_COLOR_PAT_MANAGER_COMBO_BREAKER),
    }),
    SUBCATEGORY(_N("Piano"), {
      SETTING_COLOR(GUI_COLOR_PIANO_BACKGROUND),
      SETTING_COLOR(GUI_COLOR_PIANO_KEY_TOP),
      SETTING_COLOR(GUI_COLOR_PIANO_KEY_TOP_HIT),
      SETTING_COLOR(GUI_COLOR_PIANO_KEY_TOP_ACTIVE),
      SETTING_COLOR(GUI_COLOR_PIANO_KEY_BOTTOM),
      SETTING_COLOR(GUI_COLOR_PIANO_KEY_BOTTOM_HIT),
      SETTING_COLOR(GUI_COLOR_PIANO_KEY_BOTTOM_ACTIVE),
    }),
    SUBCATEGORY(_N("Clock"), {
      SETTING_COLOR(GUI_COLOR_CLOCK_TEXT),
      SETTING_COLOR(GUI_COLOR_CLOCK_BEAT_LOW),
      SETTING_COLOR(GUI_COLOR_CLOCK_BEAT_HIGH),
    }),
    SUBCATEGORY(_N("Patchbay"), {
      SETTING_COLOR(GUI_COLOR_PATCHBAY_PORTSET),
      SETTING_COLOR(GUI_COLOR_PATCHBAY_PORT),
      SETTING_COLOR(GUI_COLOR_PATCHBAY_PORT_HIDDEN),
      SETTING_COLOR(GUI_COLOR_PATCHBAY_CONNECTION),
      SETTING_COLOR(GUI_COLOR_PATCHBAY_CONNECTION_BG),
    }),
    SUBCATEGORY(_N("Memory Composition"), {
      SETTING_COLOR(GUI_COLOR_MEMORY_BG),
      SETTING_COLOR(GUI_COLOR_MEMORY_DATA),
      SETTING_COLOR(GUI_COLOR_MEMORY_FREE),
      // SETTING_COLOR(GUI_COLOR_MEMORY_PADDING),
      SETTING_COLOR(GUI_COLOR_MEMORY_RESERVED),
      SETTING_COLOR(GUI_COLOR_MEMORY_SAMPLE),
      SETTING_COLOR(GUI_COLOR_MEMORY_SAMPLE_ALT1),
      SETTING_COLOR(GUI_COLOR_MEMORY_SAMPLE_ALT2),
      SETTING_COLOR(GUI_COLOR_MEMORY_SAMPLE_ALT3),
      SETTING_COLOR(GUI_COLOR_MEMORY_WAVE_RAM),
      SETTING_COLOR(GUI_COLOR_MEMORY_WAVE_STATIC),
      SETTING_COLOR(GUI_COLOR_MEMORY_ECHO),
      SETTING_COLOR(GUI_COLOR_MEMORY_N163_LOAD),
      SETTING_COLOR(GUI_COLOR_MEMORY_N163_PLAY),
      SETTING_COLOR(GUI_COLOR_MEMORY_BANK0),
      SETTING_COLOR(GUI_COLOR_MEMORY_BANK1),
      SETTING_COLOR(GUI_COLOR_MEMORY_BANK2),
      SETTING_COLOR(GUI_COLOR_MEMORY_BANK3),
      SETTING_COLOR(GUI_COLOR_MEMORY_BANK4),
      SETTING_COLOR(GUI_COLOR_MEMORY_BANK5),
      SETTING_COLOR(GUI_COLOR_MEMORY_BANK6),
      SETTING_COLOR(GUI_COLOR_MEMORY_BANK7),
    }),
    SUBCATEGORY(_N("Tuner"), {
      SETTING_COLOR(GUI_COLOR_TUNER_NEEDLE),
      SETTING_COLOR(GUI_COLOR_TUNER_SCALE_LOW),
      SETTING_COLOR(GUI_COLOR_TUNER_SCALE_HIGH),
    }),
    SUBCATEGORY(_N("Log Viewer"), {
      SETTING_COLOR(GUI_COLOR_LOGLEVEL_ERROR),
      SETTING_COLOR(GUI_COLOR_LOGLEVEL_WARNING),
      SETTING_COLOR(GUI_COLOR_LOGLEVEL_INFO),
      SETTING_COLOR(GUI_COLOR_LOGLEVEL_DEBUG),
      SETTING_COLOR(GUI_COLOR_LOGLEVEL_TRACE),
    }),
  }
  CATEGORY_END
  CATEGORY_BEGIN(_N("Keyboard")) {},{
    SUBCATEGORY(_N("Global hotkeys"),{
      SETTING_KEYBIND(GUI_ACTION_NEW),
      SETTING_KEYBIND(GUI_ACTION_CLEAR),
      SETTING_KEYBIND(GUI_ACTION_OPEN),
      SETTING_KEYBIND(GUI_ACTION_OPEN_BACKUP),
      SETTING_KEYBIND(GUI_ACTION_SAVE),
      SETTING_KEYBIND(GUI_ACTION_SAVE_AS),
      SETTING_KEYBIND(GUI_ACTION_EXPORT),
      SETTING_KEYBIND(GUI_ACTION_UNDO),
      SETTING_KEYBIND(GUI_ACTION_REDO),
      SETTING_KEYBIND(GUI_ACTION_PLAY_TOGGLE),
      SETTING_KEYBIND(GUI_ACTION_PLAY),
      SETTING_KEYBIND(GUI_ACTION_STOP),
      SETTING_KEYBIND(GUI_ACTION_PLAY_START),
      SETTING_KEYBIND(GUI_ACTION_PLAY_REPEAT),
      SETTING_KEYBIND(GUI_ACTION_PLAY_CURSOR),
      SETTING_KEYBIND(GUI_ACTION_STEP_ONE),
      SETTING_KEYBIND(GUI_ACTION_OCTAVE_UP),
      SETTING_KEYBIND(GUI_ACTION_OCTAVE_DOWN),
      SETTING_KEYBIND(GUI_ACTION_INS_UP),
      SETTING_KEYBIND(GUI_ACTION_INS_DOWN),
      SETTING_KEYBIND(GUI_ACTION_STEP_UP),
      SETTING_KEYBIND(GUI_ACTION_STEP_DOWN),
      SETTING_KEYBIND(GUI_ACTION_TOGGLE_EDIT),
      SETTING_KEYBIND(GUI_ACTION_METRONOME),
      SETTING_KEYBIND(GUI_ACTION_ORDER_LOCK),
      SETTING_KEYBIND(GUI_ACTION_REPEAT_PATTERN),
      SETTING_KEYBIND(GUI_ACTION_FOLLOW_ORDERS),
      SETTING_KEYBIND(GUI_ACTION_FOLLOW_PATTERN),
      SETTING_KEYBIND(GUI_ACTION_FULLSCREEN),
      SETTING_KEYBIND(GUI_ACTION_TX81Z_REQUEST),
      SETTING_KEYBIND(GUI_ACTION_PANIC),
    }),
    SUBCATEGORY(_N("Note input"),{
      SettingEntry(_N("Note input"),NULL,[this]{
        bool ret=false;
        if (ImGui::BeginTable("keysNoteInput",4)) {
          ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed);
          ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthFixed);
          ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthStretch);
          ImGui::TableSetupColumn("c4",ImGuiTableColumnFlags_WidthFixed);
          static char id[4096];

          ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
          ImGui::TableNextColumn();
          ImGui::Text(_("Key"));
          ImGui::TableNextColumn();
          ImGui::Text(_("Type"));
          ImGui::TableNextColumn();
          ImGui::Text(_("Value"));
          ImGui::TableNextColumn();
          ImGui::Text(_("Remove"));

          for (size_t _i=0; _i<noteKeysRaw.size(); _i++) {
            ImGui::PushID(_i);
            MappedInput& i=noteKeysRaw[_i];
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%s",SDL_GetScancodeName((SDL_Scancode)i.scan));
            ImGui::TableNextColumn();
            if (i.val==102) {
              if (ImGui::Button(_("Macro release##SNType"))) {
                i.val=0;
                ret=true;
              }
            } else if (i.val==101) {
              if (ImGui::Button(_("Note release##SNType"))) {
                i.val=102;
                ret=true;
              }
            } else if (i.val==100) {
              if (ImGui::Button(_("Note off##SNType"))) {
                i.val=101;
                ret=true;
              }
            } else {
              if (ImGui::Button(_("Note##SNType"))) {
                i.val=100;
                ret=true;
              }
            }
            ImGui::TableNextColumn();
            if (i.val<100) {
              const char* note=noteName(i.val+60);
              snprintf(id,4095,_("%2d | %c%c (+%d oct.)"),
                i.val,note[0],note[1]=='-'?' ':note[1],i.val/12);
              ImGui::PushFont(patFont);
              if (ImGui::InputScalar("##SNValue",ImGuiDataType_S32,&i.val,&_ONE,&_TWELVE,id)) {
                if (i.val<0) i.val=0;
                if (i.val>96) i.val=96;
                ret=true;
              }
              ImGui::PopFont();
            }
            ImGui::TableNextColumn();
            if (ImGui::Button(ICON_FA_TIMES "##SNRemove")) {
              noteKeysRaw.erase(noteKeysRaw.begin()+_i);
              _i--;
              ret=true;
            }
            ImGui::PopID();
          }
          ImGui::EndTable();

          if (ImGui::BeginCombo("##SNAddNew",_("Add..."))) {
            for (int i=0; i<SDL_NUM_SCANCODES; i++) {
              const char* sName=SDL_GetScancodeName((SDL_Scancode)i);
              if (sName==NULL) continue;
              if (sName[0]==0) continue;
              snprintf(id,4095,"%s##SNNewKey_%d",sName,i);
              if (ImGui::Selectable(id)) {
                bool alreadyThere=false;
                for (MappedInput& j: noteKeysRaw) {
                  if (j.scan==i) {
                    alreadyThere=true;
                    break;
                  }
                }
                if (alreadyThere) {
                  showError(_("that key is bound already!"));
                } else {
                  noteKeysRaw.push_back(MappedInput(i,0));
                  ret=true;
                }
              }
            }
            ImGui::EndCombo();
          }
        }
        return ret;
      }),
    })
  }
  CATEGORY_END
}
