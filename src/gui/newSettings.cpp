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
      if (ImGui::SliderFloat(_(label),(float*)value,data->min,data->max,data->fmt)) {
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
      if (ImGui::SliderInt(_(label),(int*)value,data->min,data->max,data->fmt)) {
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
        if (data->isPathInput) {
          ImGui::PushID(label);
          if (ImGui::Button(ICON_FA_FOLDER "##SettingPathButton")) {
            data->dialogCallback();
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
        callback();
        ret=true;
      }
      ImGui::PopID();
      break;
    // case SettingKeybind: {
    //   keybindList* keybinds=(keybindList*)value;
    //   ImGui::PushID(label);
    //   for (size_t i=0; i<keybinds->size()+1; i++) {
    //     ImGui::PushID(i);
    //     if (i>0) ImGui::SameLine();
    //     bool isPending=bindSetPending && bindSetTarget==actionIdx && bindSetTargetIdx==(int)i;
    //     if (i<keybinds->size()) {
    //       if (ImGui::Button(isPending?_N("Press key..."):getKeyName(keybinds->at(i)).c_str())) {
    //         promptKey(actionIdx,i);
    //         ret=true;
    //       }
    //       bool rightClicked=ImGui::IsItemClicked(ImGuiMouseButton_Right);
    //       if (!rightClicked) {
    //         ImGui::SameLine(0.0f, 1.0f);
    //       }
    //       if (rightClicked || ImGui::Button(ICON_FA_TIMES)) {
    //         keybinds->erase(keybinds->begin()+i);
    //         if (isPending) {
    //           bindSetActive=false;
    //           bindSetPending=false;
    //         }
    //         parseKeybinds();
    //       }
    //     } else {
    //       if (ImGui::Button(isPending?_N("Press key..."):"+")) {
    //         promptKey(actionIdx,i);
    //         ret=true;
    //       }
    //     }
    //     ImGui::PopID(); // i
    //   }
    //   ImGui::PopID();
    //   break;
    // }
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
  SettingEntry::Color(guiColors[_which].friendlyName,NULL,&uiColors[_which]).Callback([this]{applyUISettings(false);})

#define FILE_DIALOG(d) [this]{openFileDialog(d);}

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
        FILE_DIALOG(GUI_FILE_YRW801_ROM_OPEN)
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
    SETTING_COLOR(GUI_COLOR_BUTTON),
    SETTING_COLOR(GUI_COLOR_BUTTON_HOVER),
    SETTING_COLOR(GUI_COLOR_BUTTON_ACTIVE),
    SETTING_COLOR(GUI_COLOR_TAB),
    SETTING_COLOR(GUI_COLOR_TAB_HOVER),
    SETTING_COLOR(GUI_COLOR_TAB_ACTIVE),
  });
}
