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
      if (gui->CWSliderFloat(_(label),(float*)value,data->min,data->max,data->fmt)) {
        if (getValue<float>()<data->min) setValue(data->min);
        if (getValue<float>()>data->max) setValue(data->max);
        callback();
        ret=true;
      } rightClickable
      break;
    }
    case SettingSliderInt: {
      assert(extData && "SettingSliderInt requires extData!");
      SettingEntryNumericInputExtData<int>* data=(SettingEntryNumericInputExtData<int>*)extData;
      if (gui->CWSliderInt(_(label),(int*)value,data->min,data->max,data->fmt)) {
        if (getValue<int>()<data->min) setValue(data->min);
        if (getValue<int>()>data->max) setValue(data->max);
        callback();
        ret=true;
      } rightClickable
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

void SettingEntry::loadConf(DivConfig& conf) {
  
}

void SettingEntry::saveConf(DivConfig& conf) {
  
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

SettingsCategory::SettingsCategory(const SettingsCategory& s) {
  name=s.name;
  settings=s.settings;
  children=s.children;
  scrollPos=s.scrollPos;
}

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
#define _C(...) allSettings.push_back(SettingsCategory(__VA_ARGS__))
#define _CC SettingsCategory

#define SETTING_CHECKBOX(_label,_value) \
  SettingEntry::Checkbox(_label,#_value,&settings._value)

#define SETTING_COLOR(_which) \
  SettingEntry::Color(guiColors[_which].friendlyName,NULL,&uiColors[_which])
#define SETTING_KEYBIND(_which) \
  SettingEntry::Keybind(guiActions[_which].friendlyName,NULL,&actionKeys[_which],_which)

void FurnaceGUI::initSettings() {
  _C(_N("General"),{},{
    _CC(_N("Program"),{
#ifdef HAVE_LOCALE
      SettingEntry::ComboString(
        _N("Language"),
        "locale",&settings.locale,{
          {"<System>","",NULL},
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
      SettingEntry::Radio(
        _N("Play after opening song:"),"playOnLoad",&settings.playOnLoad,{
        {_N("No##pol0"),0,NULL},
        {_N("Only if already playing##pol1"),1,NULL},
        {_N("Yes##pol0"),2,NULL},
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
        {"JACK",DIV_AUDIO_JACK,NULL},
#endif
        {"SDL",DIV_AUDIO_SDL,NULL},
#ifdef HAVE_PA
        {"PortAudio",DIV_AUDIO_PORTAUDIO,NULL},
#endif
#ifdef HAVE_ASIO
        {"ASIO",DIV_AUDIO_ASIO,NULL},
#endif
      }),
#endif
    }),
    _CC(_N("Volumes"),{
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
  });
  _C(_N("Emulation"),{
    SettingEntry::ComboInt(
      _N("PC Speacker Strategy"),
      "pcSpeakerOutMethod",&settings.pcSpeakerOutMethod,{
        {_N("evdev SND_TONE"),0,NULL},
        {_N("KIOCSOUND on /dev/tty1"),1,NULL},
        {_N("/dev/port"),2,NULL},
        {_N("KIOCSOUND on standard output"),3,NULL},
        {_N("outb()"),4,NULL},
      })
  },{
    _CC(_N("Sample ROMs"),{
      SettingEntry::Path(
        _N("OPL4 YRW801 path"),
        "yrw801Path",&settings.yrw801Path,
        GUI_FILE_YRW801_ROM_OPEN
      )
    })
  });
  _C(_N("Appearance"),{
    SettingEntry::Radio(
      _N("Channel feedback style:"),
      "channelFeedbackStyle",&settings.channelFeedbackStyle,{
        {_N("Off##CHF0"),0,NULL},
        {_N("Note##CHF1"),1,NULL},
        {_N("Volume##CHF2"),2,NULL},
        {_N("Active##CHF3"),3,NULL},
        {_N("Volume (Real)##CHF4"),4,NULL},
      }
    ),
    SettingEntry::SliderFloat(
      _N("Gamma##CHF"),
      "channelFeedbackGamma",&settings.channelFeedbackGamma,
      {0.0f,2.0f,NULL}
    ).Condition([this]{return settings.channelFeedbackStyle==4;})
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
  _C(_N("Color"),{
    SETTING_CHECKBOX(_N("Guru mode"),basicColors),
    SettingEntry::SliderInt(
      _N("Frame shading"),
      "guiColorsShading",&settings.guiColorsShading,
      {0,100,NULL}
    ).Callback([this]{applyUISettings(false);}),
    SettingEntry::Radio(
      _N("Color scheme type:"),
      "guiColorsBase",&settings.guiColorsBase,
      {
        {_N("Dark##gcb0"),0,NULL},
        {_N("Light##gcb1"),1,NULL}
      }
    ).Callback([this]{applyUISettings(false);})
    .Condition([this]{return settings.basicColors;}),
  },{
    _CC(_N("Interface"),{
      SETTING_COLOR(GUI_COLOR_ACCENT_PRIMARY).Condition([this]{return settings.basicColors;}),
      SETTING_COLOR(GUI_COLOR_ACCENT_SECONDARY).Condition([this]{return settings.basicColors;}),

      SETTING_COLOR(GUI_COLOR_BUTTON).Condition([this]{return !settings.basicColors;}),
      SETTING_COLOR(GUI_COLOR_BUTTON_HOVER).Condition([this]{return !settings.basicColors;}),
      SETTING_COLOR(GUI_COLOR_BUTTON_ACTIVE).Condition([this]{return !settings.basicColors;}),
      SETTING_COLOR(GUI_COLOR_TAB).Condition([this]{return !settings.basicColors;}),
      SETTING_COLOR(GUI_COLOR_TAB_HOVER).Condition([this]{return !settings.basicColors;}),
      SETTING_COLOR(GUI_COLOR_TAB_ACTIVE).Condition([this]{return !settings.basicColors;}),
      SETTING_COLOR(GUI_COLOR_TAB_SELECTED_OVERLINE).Condition([this]{return !settings.basicColors;}),
      SETTING_COLOR(GUI_COLOR_TAB_UNFOCUSED).Condition([this]{return !settings.basicColors;}),
      SETTING_COLOR(GUI_COLOR_TAB_UNFOCUSED_ACTIVE).Condition([this]{return !settings.basicColors;}),
      SETTING_COLOR(GUI_COLOR_TAB_DIMMED_SELECTED_OVERLINE).Condition([this]{return !settings.basicColors;}),
      SETTING_COLOR(GUI_COLOR_IMGUI_HEADER).Condition([this]{return !settings.basicColors;}),
      SETTING_COLOR(GUI_COLOR_IMGUI_HEADER_HOVER).Condition([this]{return !settings.basicColors;}),
      SETTING_COLOR(GUI_COLOR_IMGUI_HEADER_ACTIVE).Condition([this]{return !settings.basicColors;}),
      SETTING_COLOR(GUI_COLOR_RESIZE_GRIP).Condition([this]{return !settings.basicColors;}),
      SETTING_COLOR(GUI_COLOR_RESIZE_GRIP_HOVER).Condition([this]{return !settings.basicColors;}),
      SETTING_COLOR(GUI_COLOR_RESIZE_GRIP_ACTIVE).Condition([this]{return !settings.basicColors;}),
      SETTING_COLOR(GUI_COLOR_WIDGET_BACKGROUND).Condition([this]{return !settings.basicColors;}),
      SETTING_COLOR(GUI_COLOR_WIDGET_BACKGROUND_HOVER).Condition([this]{return !settings.basicColors;}),
      SETTING_COLOR(GUI_COLOR_WIDGET_BACKGROUND_ACTIVE).Condition([this]{return !settings.basicColors;}),
      SETTING_COLOR(GUI_COLOR_SLIDER_GRAB).Condition([this]{return !settings.basicColors;}),
      SETTING_COLOR(GUI_COLOR_SLIDER_GRAB_ACTIVE).Condition([this]{return !settings.basicColors;}),
      SETTING_COLOR(GUI_COLOR_TITLE_BACKGROUND_ACTIVE).Condition([this]{return !settings.basicColors;}),
      SETTING_COLOR(GUI_COLOR_CHECK_MARK).Condition([this]{return !settings.basicColors;}),
      SETTING_COLOR(GUI_COLOR_TEXT_LINK).Condition([this]{return !settings.basicColors;}),
      SETTING_COLOR(GUI_COLOR_TEXT_SELECTION).Condition([this]{return !settings.basicColors;}),
      SETTING_COLOR(GUI_COLOR_TREE_LINES).Condition([this]{return !settings.basicColors;}),
      SETTING_COLOR(GUI_COLOR_PLOT_LINES).Condition([this]{return !settings.basicColors;}),
      SETTING_COLOR(GUI_COLOR_PLOT_LINES_HOVER).Condition([this]{return !settings.basicColors;}),
      SETTING_COLOR(GUI_COLOR_PLOT_HISTOGRAM).Condition([this]{return !settings.basicColors;}),
      SETTING_COLOR(GUI_COLOR_PLOT_HISTOGRAM_HOVER).Condition([this]{return !settings.basicColors;}),
      SETTING_COLOR(GUI_COLOR_TABLE_ROW_EVEN).Condition([this]{return !settings.basicColors;}),
      SETTING_COLOR(GUI_COLOR_TABLE_ROW_ODD).Condition([this]{return !settings.basicColors;}),
    }),
    _CC(_N("Interface (other)"),{
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
    _CC(_N("Miscellaneous"), {
      SETTING_COLOR(GUI_COLOR_TOGGLE_ON),
      SETTING_COLOR(GUI_COLOR_TOGGLE_OFF),
      SETTING_COLOR(GUI_COLOR_PLAYBACK_STAT),
      SETTING_COLOR(GUI_COLOR_DESTRUCTIVE),
      SETTING_COLOR(GUI_COLOR_WARNING),
      SETTING_COLOR(GUI_COLOR_ERROR),
    }),
    _CC(_N("File Picker (built-in)"), {
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
    _CC(_N("Oscilloscope"), {
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
      _CC(_N("Wave (non-mono)"), {
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
    _CC(_N("Volume Meter"), {
      SETTING_COLOR(GUI_COLOR_VOLMETER_LOW),
      SETTING_COLOR(GUI_COLOR_VOLMETER_HIGH),
      SETTING_COLOR(GUI_COLOR_VOLMETER_PEAK),
    }),
    _CC(_N("Orders"), {
      SETTING_COLOR(GUI_COLOR_ORDER_ROW_INDEX),
      SETTING_COLOR(GUI_COLOR_ORDER_ACTIVE),
      SETTING_COLOR(GUI_COLOR_SONG_LOOP),
      SETTING_COLOR(GUI_COLOR_ORDER_SELECTED),
      SETTING_COLOR(GUI_COLOR_ORDER_SIMILAR),
      SETTING_COLOR(GUI_COLOR_ORDER_INACTIVE),
    }),
    _CC(_N("Envelope View"), {
      SETTING_COLOR(GUI_COLOR_FM_ENVELOPE),
      SETTING_COLOR(GUI_COLOR_FM_ENVELOPE_SUS_GUIDE),
      SETTING_COLOR(GUI_COLOR_FM_ENVELOPE_RELEASE),}
    ),
    _CC(_N("FM Editor"), {
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
    _CC(_N("Macro Editor"), {
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
    _CC(_N("Multi-instrument Play"), {
      SETTING_COLOR(GUI_COLOR_MULTI_INS_1),
      SETTING_COLOR(GUI_COLOR_MULTI_INS_2),
      SETTING_COLOR(GUI_COLOR_MULTI_INS_3),
      SETTING_COLOR(GUI_COLOR_MULTI_INS_4),
      SETTING_COLOR(GUI_COLOR_MULTI_INS_5),
      SETTING_COLOR(GUI_COLOR_MULTI_INS_6),
      SETTING_COLOR(GUI_COLOR_MULTI_INS_7),
    }),
    _CC(_N("Instrument Types"), {
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
    _CC(_N("Channel"), {
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
    _CC(_N("Pattern"), {
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
    _CC(_N("Sample Editor"), {
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
    _CC(_N("Pattern Manager"), {
      SETTING_COLOR(GUI_COLOR_PAT_MANAGER_NULL),
      SETTING_COLOR(GUI_COLOR_PAT_MANAGER_UNUSED),
      SETTING_COLOR(GUI_COLOR_PAT_MANAGER_USED),
      SETTING_COLOR(GUI_COLOR_PAT_MANAGER_OVERUSED),
      SETTING_COLOR(GUI_COLOR_PAT_MANAGER_EXTREMELY_OVERUSED),
      SETTING_COLOR(GUI_COLOR_PAT_MANAGER_COMBO_BREAKER),
    }),
    _CC(_N("Piano"), {
      SETTING_COLOR(GUI_COLOR_PIANO_BACKGROUND),
      SETTING_COLOR(GUI_COLOR_PIANO_KEY_TOP),
      SETTING_COLOR(GUI_COLOR_PIANO_KEY_TOP_HIT),
      SETTING_COLOR(GUI_COLOR_PIANO_KEY_TOP_ACTIVE),
      SETTING_COLOR(GUI_COLOR_PIANO_KEY_BOTTOM),
      SETTING_COLOR(GUI_COLOR_PIANO_KEY_BOTTOM_HIT),
      SETTING_COLOR(GUI_COLOR_PIANO_KEY_BOTTOM_ACTIVE),
    }),
    _CC(_N("Clock"), {
      SETTING_COLOR(GUI_COLOR_CLOCK_TEXT),
      SETTING_COLOR(GUI_COLOR_CLOCK_BEAT_LOW),
      SETTING_COLOR(GUI_COLOR_CLOCK_BEAT_HIGH),
    }),
    _CC(_N("Patchbay"), {
      SETTING_COLOR(GUI_COLOR_PATCHBAY_PORTSET),
      SETTING_COLOR(GUI_COLOR_PATCHBAY_PORT),
      SETTING_COLOR(GUI_COLOR_PATCHBAY_PORT_HIDDEN),
      SETTING_COLOR(GUI_COLOR_PATCHBAY_CONNECTION),
      SETTING_COLOR(GUI_COLOR_PATCHBAY_CONNECTION_BG),
    }),
    _CC(_N("Memory Composition"), {
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
    _CC(_N("Tuner"), {
      SETTING_COLOR(GUI_COLOR_TUNER_NEEDLE),
      SETTING_COLOR(GUI_COLOR_TUNER_SCALE_LOW),
      SETTING_COLOR(GUI_COLOR_TUNER_SCALE_HIGH),
    }),
    _CC(_N("Log Viewer"), {
      SETTING_COLOR(GUI_COLOR_LOGLEVEL_ERROR),
      SETTING_COLOR(GUI_COLOR_LOGLEVEL_WARNING),
      SETTING_COLOR(GUI_COLOR_LOGLEVEL_INFO),
      SETTING_COLOR(GUI_COLOR_LOGLEVEL_DEBUG),
      SETTING_COLOR(GUI_COLOR_LOGLEVEL_TRACE),
    }),
  });
  _C(_N("Keyboard"),{},{
    _CC(_N("Global hotkeys"),{
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
    _CC(_N("Note input"),{
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
  });
}
