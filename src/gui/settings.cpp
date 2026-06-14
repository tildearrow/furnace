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

#include "gui.h"
#include "imgui_internal.h"
#include "fonts.h"
#include "../ta-log.h"
#include "../fileutils.h"
#include "../utfutils.h"
#include "util.h"
#include "guiConst.h"
#include "intConst.h"
#include "IconsFontAwesome4.h"
#include "furIcons.h"
#include "misc/cpp/imgui_stdlib.h"
#include "misc/freetype/imgui_freetype.h"
#include "scaling.h"
#include <fmt/printf.h>

// some of these arent needed i think...
#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#endif

#include "newSettings.h"

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

void FurnaceGUI::promptKey(int which, int bindIdx) {
  bindSetTarget=which;
  bindSetTargetIdx=bindIdx;
  bindSetActive=true;
  bindSetPending=true;
  if (bindIdx>=(int)actionKeys[which].size()) {
    bindSetPrevValue=0;
    actionKeys[which].push_back(0);
  } else {
    bindSetPrevValue=actionKeys[which][bindIdx];
    actionKeys[which][bindIdx]=0;
  }
}

void FurnaceGUI::drawSettings() {
  bool settingsRequested=false;
  if (nextWindow==GUI_WINDOW_SETTINGS) {
    settingsOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
    settingsRequested=true;
  }
  if (!settingsOpen) return;
  if (mobileUI) {
    ImVec2 setWindowPos=ImVec2(0,0);
    ImVec2 setWindowSize=ImVec2(canvasW,canvasH);
    ImGui::SetNextWindowPos(setWindowPos);
    ImGui::SetNextWindowSize(setWindowSize);
  } else {
    ImGui::SetNextWindowSizeConstraints(ImVec2(200.0f*dpiScale,100.0f*dpiScale),ImVec2(canvasW,canvasH));
  }
  if (ImGui::Begin("Settings",&settingsOpen,ImGuiWindowFlags_NoDocking|ImGuiWindowFlags_NoScrollWithMouse|ImGuiWindowFlags_NoScrollbar|globalWinFlags,_("Settings"))) {
    if (!settingsOpen) {
      if (settingsChanged) {
        settingsOpen=true;
        showWarning(_("Do you want to save your settings?"),GUI_WARN_CLOSE_SETTINGS);
      } else {
        settingsOpen=false;
      }
    }
    const float buttonsHeight=ImGui::GetFontSize()+ImGui::GetStyle().FramePadding.y*4.0f;
    const bool vertical=ImGui::GetWindowHeight()>ImGui::GetWindowWidth();
    if (ImGui::BeginTable("nnsTable",vertical?1:2,ImGuiTableFlags_Resizable|(vertical?ImGuiTableFlags_BordersInnerH:0))) {
      if (vertical) {
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch);
      } else {
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.2f);
        ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.8f);
      }
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x-ImGui::CalcTextSize(ICON_FA_BARS).x-ImGui::GetStyle().FramePadding.x*4.0f);
      if (ImGui::InputTextWithHint("##nnsSearch",_("Search..."),settingsFilter.InputBuf,IM_ARRAYSIZE(settingsFilter.InputBuf))) {
        settingsFilter.Build();
        settingsShowItemResults=true;
      }
      if (settingsRequested) {
        settingsRequested=false;
        ImGui::SetKeyboardFocusHere(-1);
      }
      ImGui::SameLine();
      ImGui::Button(ICON_FA_BARS "##SettingsImportExportReset");
      if (ImGui::BeginPopupContextItem("##SettingsImportExportResetPopup",ImGuiPopupFlags_MouseButtonLeft)) {
        if (ImGui::MenuItem(_("import..."))) {
          openFileDialog(GUI_FILE_IMPORT_CONFIG);
          ImGui::CloseCurrentPopup();
        }
        if (ImGui::MenuItem(_("export..."))) {
          openFileDialog(GUI_FILE_EXPORT_CONFIG);
          ImGui::CloseCurrentPopup();
        }
        pushDestColor();
        if (ImGui::MenuItem(_("factory reset"))) {
          showWarning(_("Are you sure you want to reset all Furnace settings?\nYou must restart Furnace after doing so."),GUI_WARN_RESET_CONFIG);
          ImGui::CloseCurrentPopup();
        }
        popDestColor();
        ImGui::EndPopup();
      }
      SettingsCategory* resetScroll=NULL;
      ImVec2 childSize=ImGui::GetContentRegionAvail();
      if (vertical) {
        childSize.y/=3.0f;
      } else {
        childSize.y-=buttonsHeight;
      }
      if (ImGui::BeginChild("nnsSidebar",childSize)) {
        for (size_t i=0; i<allSettings.size(); i++) {
          SettingsCategory* clicked=allSettings[i].drawSidebar(&settingsFilter,this);
          if (clicked) {
            settingsShowItemResults=false;
            resetScroll=clicked;
          }
        }
      }
      ImGui::EndChild();
      if (vertical) ImGui::TableNextRow();
      ImGui::TableNextColumn();
      childSize=ImGui::GetContentRegionAvail();
      childSize.y-=buttonsHeight;
      if (ImGui::BeginChild("nnsEntries",childSize)) {
        if (settingsFilter.IsActive() && settingsShowItemResults) {
          for (SettingsCategory& c: allSettings) {
            if (c.drawSettings(&settingsFilter,settingsShowItemResults,this,0,resetScroll))
              settingsChanged=true;
          }
        } else if (curCategory!=NULL) {
          if (curCategory->drawSettings(&settingsFilter,settingsShowItemResults,this,0,resetScroll))
            settingsChanged=true;
        }
      }
      ImGui::EndChild();
      ImGui::EndTable();
    }
    ImGui::Separator();
    if (ImGui::Button(_("OK##SettingsOK"))) {
      settingsOpen=false;
      willCommit=true;
      settingsChanged=false;
    }
    ImGui::SameLine();
    if (ImGui::Button(_("Cancel##SettingsCancel"))) {
      settingsOpen=false;
      audioEngineChanged=false;
      syncSettings();
      settingsChanged=false;
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(!settingsChanged);
    if (ImGui::Button(_("Apply##SettingsApply"))) {
      willCommit=true;
      settingsChanged=false;
    }
    ImGui::EndDisabled();
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_SETTINGS;
  ImGui::End();
}

void FurnaceGUI::syncSettings() {
  readConfig(e->getConfObject());

  parseKeybinds();

  midiMap.read(e->getConfigPath()+DIR_SEPARATOR_STR+"midiIn_"+stripName(settings.midiInDevice)+".cfg");
  midiMap.compile();

  e->setMidiDirect(midiMap.directChannel);
  e->setMidiDirectProgram(midiMap.directChannel && midiMap.directProgram);
  e->setMidiVolExp(midiMap.volExp);
  e->setMetronomeVol(((float)settings.metroVol)/100.0f);
  e->setSamplePreviewVol(((float)settings.sampleVol)/100.0f);

  if (rend!=NULL) {
    rend->setSwapInterval(settings.vsync);
  }

  backupTimer=settings.backupInterval;
}

void FurnaceGUI::commitSettings() {
  bool sampleROMsChanged=settings.yrw801Path!=e->getConfString("yrw801Path","") ||
    settings.tg100Path!=e->getConfString("tg100Path","") ||
    settings.mu5Path!=e->getConfString("mu5Path","");

  bool coresChanged=(
    settings.arcadeCore!=e->getConfInt("arcadeCore",0) ||
    settings.ym2612Core!=e->getConfInt("ym2612Core",0) ||
    settings.snCore!=e->getConfInt("snCore",0) ||
    settings.nesCore!=e->getConfInt("nesCore",0) ||
    settings.fdsCore!=e->getConfInt("fdsCore",0) ||
    settings.c64Core!=e->getConfInt("c64Core",0) ||
    settings.pokeyCore!=e->getConfInt("pokeyCore",1) ||
    settings.opn1Core!=e->getConfInt("opn1Core",1) ||
    settings.opnaCore!=e->getConfInt("opnaCore",1) ||
    settings.opnbCore!=e->getConfInt("opnbCore",1) ||
    settings.opl2Core!=e->getConfInt("opl2Core",0) ||
    settings.opl3Core!=e->getConfInt("opl3Core",0) ||
    settings.opl4Core!=e->getConfInt("opl4Core",0) ||
    settings.esfmCore!=e->getConfInt("esfmCore",0) ||
    settings.opllCore!=e->getConfInt("opllCore",0) ||
    settings.ayCore!=e->getConfInt("ayCore",0) ||
    settings.swanCore!=e->getConfInt("swanCore",0) ||
    settings.dsidQuality!=e->getConfInt("dsidQuality",3) ||
    settings.gbQuality!=e->getConfInt("gbQuality",3) ||
    settings.pnQuality!=e->getConfInt("pnQuality",3) ||
    settings.saaQuality!=e->getConfInt("saaQuality",3) ||
    settings.audioQuality!=e->getConfInt("audioQuality",0) ||
    settings.audioHiPass!=e->getConfBool("audioHiPass",1)
  );

  writeConfig(e->getConfObject());

  parseKeybinds();

  midiMap.compile();
  midiMap.write(e->getConfigPath()+DIR_SEPARATOR_STR+"midiIn_"+stripName(settings.midiInDevice)+".cfg");

  e->saveConf();

  while (!recentFile.empty() && (int)recentFile.size()>settings.maxRecentFile) {
    recentFile.pop_back();
  }

  if (sampleROMsChanged) {
    if (e->loadSampleROMs()) {
      showError(e->getLastError());
    }
  }

  if (!e->switchMaster(coresChanged)) {
    showError(_("could not initialize audio!"));
  }

  ImGui::GetIO().Fonts->Clear();

  applyUISettings();

  audioEngineChanged=false;
}

bool FurnaceGUI::importColors(String path) {
  DivConfig c;
  if (!c.loadFromFile(path.c_str(),false,false)) {
    logW("error while opening color file for import: %s",strerror(errno));
    return false;
  }

  readConfig(c,GUI_SETTINGS_COLOR);

  applyUISettings(false);
  return true;
}

bool FurnaceGUI::exportColors(String path) {
  DivConfig c;

  c.set("configVersion",DIV_ENGINE_VERSION);
  writeConfig(c,GUI_SETTINGS_COLOR);

  FILE* f=ps_fopen(path.c_str(),"wb");
  if (f==NULL) {
    logW("error while opening color file for export: %s",strerror(errno));
    return false;
  }

  String result=c.toString();

  if (fwrite(result.c_str(),1,result.size(),f)!=result.size()) {
    logW("couldn't write color file entirely.");
  }

  fclose(f);
  return true;
}

bool FurnaceGUI::importKeybinds(String path) {
  DivConfig c;
  if (!c.loadFromFile(path.c_str(),false,false)) {
    logW("error while opening keybind file for import: %s",strerror(errno));
    return false;
  }
  resetKeybinds();
  if (c.has("configVersion")) {
    // new
    readConfig(c,GUI_SETTINGS_KEYBOARD);
  } else {
    // unoptimal
    for (auto& key: c.configMap()) {
      for (int i=0; i<GUI_ACTION_MAX; i++) {
        try {
          if (key.first==guiActions[i].name) {
            // old versions didn't support multi-bind
            actionKeys[i].clear();
            actionKeys[i].push_back(std::stoi(key.second));
            break;
          }
        } catch (std::out_of_range& e) {
          break;
        } catch (std::invalid_argument& e) {
          break;
        }
      }
    }
  }
  return true;
}

bool FurnaceGUI::exportKeybinds(String path) {
  DivConfig c;

  c.set("configVersion",DIV_ENGINE_VERSION);
  writeConfig(c,GUI_SETTINGS_KEYBOARD);

  FILE* f=ps_fopen(path.c_str(),"wb");
  if (f==NULL) {
    logW("error while opening keybind file for export: %s",strerror(errno));
    return false;
  }

  String result=c.toString();

  if (fwrite(result.c_str(),1,result.size(),f)!=result.size()) {
    logW("couldn't write keybind file entirely.");
  }

  fclose(f);
  return true;
}

bool FurnaceGUI::importLayout(String path) {
  if (mobileUI) {
    logW("but you are on the mobile UI!");
    return false;
  }
  FILE* f=ps_fopen(path.c_str(),"rb");
  if (f==NULL) {
    logW("error while opening keybind file for import: %s",strerror(errno));
    return false;
  }
  if (fseek(f,0,SEEK_END)<0) {
    fclose(f);
    return false;
  }
  ssize_t len=ftell(f);
  if (len==(SIZE_MAX>>1)) {
    fclose(f);
    return false;
  }
  if (len<1) {
    if (len==0) {
      logE("that file is empty!");
      lastError=_("file is empty");
    } else {
      perror("tell error");
    }
    fclose(f);
    return false;
  }
  if (fseek(f,0,SEEK_SET)<0) {
    perror("size error");
    lastError=fmt::sprintf(_("on get size: %s"),strerror(errno));
    fclose(f);
    return false;
  }
  pendingLayoutImport=new unsigned char[len];
  if (fread(pendingLayoutImport,1,(size_t)len,f)!=(size_t)len) {
    perror("read error");
    lastError=fmt::sprintf(_("on read: %s"),strerror(errno));
    fclose(f);
    delete[] pendingLayoutImport;
    return false;
  }
  fclose(f);

  pendingLayoutImportLen=len;
  return true;
}

bool FurnaceGUI::exportLayout(String path) {
  if (mobileUI) {
    logW("but you are on the mobile UI!");
    return false;
  }
  FILE* f=ps_fopen(path.c_str(),"wb");
  if (f==NULL) {
    logW("error while opening layout file for export: %s",strerror(errno));
    return false;
  }
  size_t dataSize=0;
  const char* data=ImGui::SaveIniSettingsToMemory(&dataSize);
  if (fwrite(data,1,dataSize,f)!=dataSize) {
    logW("error while exporting layout: %s",strerror(errno));
  }
  fclose(f);
  return true;
}

bool FurnaceGUI::importConfig(String path) {
  DivConfig prevConf=e->getConfObject();
  DivConfig& conf=e->getConfObject();
  conf.clear();
  if (!conf.loadFromFile(path.c_str(),false,false)) {
    showError(fmt::sprintf(_("error while loading config! (%s)"),strerror(errno)));
    conf=prevConf;
    return false;
  }
  syncState();
  syncSettings();
  willCommit=true;

  recentFile.clear();
  for (int i=0; i<settings.maxRecentFile; i++) {
    String r=e->getConfString(fmt::sprintf("recentFile%d",i),"");
    if (!r.empty()) {
      recentFile.push_back(r);
    }
  }
  return true;
}

bool FurnaceGUI::exportConfig(String path) {
  DivConfig exConf=e->getConfObject();
  writeConfig(exConf,GUI_SETTINGS_ALL);
  commitState(exConf);

  FILE* f=ps_fopen(path.c_str(),"wb");
  if (f==NULL) {
    logW("error while exporting config: %s",strerror(errno));
    return false;
  }

  String result=exConf.toString();

  if (fwrite(result.c_str(),1,result.size(),f)!=result.size()) {
    logW("couldn't write config entirely.");
  }

  fclose(f);
  return true;
}

void FurnaceGUI::resetColors() {
  for (int i=0; i<GUI_COLOR_MAX; i++) {
    uiColors[i]=ImGui::ColorConvertU32ToFloat4(guiColors[i].defaultColor);
  }
}

void FurnaceGUI::resetKeybinds() {
  for (int i=0; i<GUI_ACTION_MAX; i++) {
    if (guiActions[i].defaultBind.empty()) continue;
    actionKeys[i]=guiActions[i].defaultBind;
  }
  decodeKeyMap(noteKeys,DEFAULT_NOTE_KEYS);
  decompileNoteKeys();
  parseKeybinds();
}

void FurnaceGUI::assignActionMap(std::map<int,int>& actionMap, int first, int last) {
  actionMap.clear();
  for (int i=first; i<=last; i++) {
    for (int key: actionKeys[i]) {
      if (key&FURK_MASK) {
        actionMap[key]=i;
      }
    }
  }
}


void FurnaceGUI::parseKeybinds() {
  assignActionMap(actionMapGlobal, GUI_ACTION_GLOBAL_MIN+1, GUI_ACTION_GLOBAL_MAX-1);
  assignActionMap(actionMapPat, GUI_ACTION_PAT_MIN+1, GUI_ACTION_PAT_MAX-1);
  assignActionMap(actionMapInsList, GUI_ACTION_INS_LIST_MIN+1, GUI_ACTION_INS_LIST_MAX-1);
  assignActionMap(actionMapWaveList, GUI_ACTION_WAVE_LIST_MIN+1, GUI_ACTION_WAVE_LIST_MAX-1);
  assignActionMap(actionMapSampleList, GUI_ACTION_SAMPLE_LIST_MIN+1, GUI_ACTION_SAMPLE_LIST_MAX-1);
  assignActionMap(actionMapSample, GUI_ACTION_SAMPLE_MIN+1, GUI_ACTION_SAMPLE_MAX-1);
  assignActionMap(actionMapOrders, GUI_ACTION_ORDERS_MIN+1, GUI_ACTION_ORDERS_MAX-1);
}

void FurnaceGUI::pushAccentColors(const ImVec4& one, const ImVec4& two, const ImVec4& border, const ImVec4& borderShadow) {
  float hue, sat, val;

  ImVec4 primaryActive=one;
  ImVec4 primaryHover, primary;
  primaryHover.w=primaryActive.w;
  primary.w=primaryActive.w;
  ImGui::ColorConvertRGBtoHSV(primaryActive.x,primaryActive.y,primaryActive.z,hue,sat,val);
  if (settings.guiColorsBase) {
    primary=primaryActive;
    ImGui::ColorConvertHSVtoRGB(hue,sat*0.9,val*0.9,primaryHover.x,primaryHover.y,primaryHover.z);
    ImGui::ColorConvertHSVtoRGB(hue,sat,val*0.5,primaryActive.x,primaryActive.y,primaryActive.z);
  } else {
    ImGui::ColorConvertHSVtoRGB(hue,sat*0.9,val*0.5,primaryHover.x,primaryHover.y,primaryHover.z);
    ImGui::ColorConvertHSVtoRGB(hue,sat*0.8,val*0.35,primary.x,primary.y,primary.z);
  }

  ImVec4 secondaryActive=two;
  ImVec4 secondaryHover, secondary, secondarySemiActive;
  secondarySemiActive.w=secondaryActive.w;
  secondaryHover.w=secondaryActive.w;
  secondary.w=secondaryActive.w;
  ImGui::ColorConvertRGBtoHSV(secondaryActive.x,secondaryActive.y,secondaryActive.z,hue,sat,val);
  if (settings.guiColorsBase) {
    secondary=secondaryActive;
    ImGui::ColorConvertHSVtoRGB(hue,sat*0.9,val*0.7,secondarySemiActive.x,secondarySemiActive.y,secondarySemiActive.z);
    ImGui::ColorConvertHSVtoRGB(hue,sat*0.9,val*0.9,secondaryHover.x,secondaryHover.y,secondaryHover.z);
    ImGui::ColorConvertHSVtoRGB(hue,sat,val*0.5,secondaryActive.x,secondaryActive.y,secondaryActive.z);
  } else {
    ImGui::ColorConvertHSVtoRGB(hue,sat*0.9,val*0.75,secondarySemiActive.x,secondarySemiActive.y,secondarySemiActive.z);
    ImGui::ColorConvertHSVtoRGB(hue,sat*0.9,val*0.5,secondaryHover.x,secondaryHover.y,secondaryHover.z);
    ImGui::ColorConvertHSVtoRGB(hue,sat*0.9,val*0.25,secondary.x,secondary.y,secondary.z);
  }

  ImGui::PushStyleColor(ImGuiCol_Button,primary);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered,primaryHover);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive,primaryActive);
  ImGui::PushStyleColor(ImGuiCol_Tab,primary);
  ImGui::PushStyleColor(ImGuiCol_TabHovered,secondaryHover);
  ImGui::PushStyleColor(ImGuiCol_TabActive,secondarySemiActive);
  ImGui::PushStyleColor(ImGuiCol_TabUnfocused,primary);
  ImGui::PushStyleColor(ImGuiCol_TabUnfocusedActive,primaryHover);
  ImGui::PushStyleColor(ImGuiCol_Header,secondary);
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered,secondaryHover);
  ImGui::PushStyleColor(ImGuiCol_HeaderActive,secondaryActive);
  ImGui::PushStyleColor(ImGuiCol_ResizeGrip,secondary);
  ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered,secondaryHover);
  ImGui::PushStyleColor(ImGuiCol_ResizeGripActive,secondaryActive);
  ImGui::PushStyleColor(ImGuiCol_FrameBg,secondary);
  ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,secondaryHover);
  ImGui::PushStyleColor(ImGuiCol_FrameBgActive,secondaryActive);
  ImGui::PushStyleColor(ImGuiCol_SliderGrab,primaryActive);
  ImGui::PushStyleColor(ImGuiCol_SliderGrabActive,primaryActive);
  ImGui::PushStyleColor(ImGuiCol_TitleBgActive,primary);
  ImGui::PushStyleColor(ImGuiCol_CheckMark,primaryActive);
  ImGui::PushStyleColor(ImGuiCol_TextSelectedBg,secondaryHover);
  ImGui::PushStyleColor(ImGuiCol_Border,border);
  ImGui::PushStyleColor(ImGuiCol_BorderShadow,borderShadow);
}

void FurnaceGUI::popAccentColors() {
  ImGui::PopStyleColor(24);
}

void FurnaceGUI::pushDestColor() {
  pushAccentColors(uiColors[GUI_COLOR_DESTRUCTIVE],uiColors[GUI_COLOR_DESTRUCTIVE],uiColors[GUI_COLOR_DESTRUCTIVE],ImVec4(0.0f,0.0f,0.0f,0.0f));
}

void FurnaceGUI::popDestColor() {
  popAccentColors();
}

void FurnaceGUI::pushWarningColor(bool warnCond, bool errorCond) {
  if (warnColorPushed) {
    logE("warnColorPushed");
    abort();
  }
  if (errorCond) {
    pushAccentColors(uiColors[GUI_COLOR_ERROR],uiColors[GUI_COLOR_ERROR],uiColors[GUI_COLOR_ERROR],ImVec4(0.0f,0.0f,0.0f,0.0f));
    warnColorPushed=true;
  } else if (warnCond) {
    pushAccentColors(uiColors[GUI_COLOR_WARNING],uiColors[GUI_COLOR_WARNING],uiColors[GUI_COLOR_WARNING],ImVec4(0.0f,0.0f,0.0f,0.0f));
    warnColorPushed=true;
  }
}

void FurnaceGUI::popWarningColor() {
  if (warnColorPushed) {
    popAccentColors();
    warnColorPushed=false;
  }
}

#ifdef _WIN32
#define SYSTEM_FONT_PATH_1 "C:\\Windows\\Fonts\\segoeui.ttf"
#define SYSTEM_FONT_PATH_2 "C:\\Windows\\Fonts\\tahoma.ttf"
#define SYSTEM_FONT_PATH_3 "C:\\Windows\\Fonts\\micross.ttf"
#define SYSTEM_HEAD_FONT_PATH_1 "C:\\Windows\\Fonts\\segoeui.ttf"
#define SYSTEM_HEAD_FONT_PATH_2 "C:\\Windows\\Fonts\\tahoma.ttf"
#define SYSTEM_HEAD_FONT_PATH_3 "C:\\Windows\\Fonts\\micross.ttf"
#define SYSTEM_PAT_FONT_PATH_1 "C:\\Windows\\Fonts\\consola.ttf"
#define SYSTEM_PAT_FONT_PATH_2 "C:\\Windows\\Fonts\\cour.ttf"
// GOOD LUCK WITH THIS ONE - UNTESTED
#define SYSTEM_PAT_FONT_PATH_3 "C:\\Windows\\Fonts\\vgasys.fon"
#elif defined(__APPLE__)
#define SYSTEM_FONT_PATH_1 "/System/Library/Fonts/SFAANS.ttf"
#define SYSTEM_FONT_PATH_2 "/System/Library/Fonts/Helvetica.ttc"
#define SYSTEM_FONT_PATH_3 "/System/Library/Fonts/Helvetica.dfont"
#define SYSTEM_HEAD_FONT_PATH_1 "/System/Library/Fonts/SFAANS.ttf"
#define SYSTEM_HEAD_FONT_PATH_2 "/System/Library/Fonts/Helvetica.ttc"
#define SYSTEM_HEAD_FONT_PATH_3 "/System/Library/Fonts/Helvetica.dfont"
#define SYSTEM_PAT_FONT_PATH_1 "/System/Library/Fonts/SFNSMono.ttf"
#define SYSTEM_PAT_FONT_PATH_2 "/System/Library/Fonts/Courier New.ttf"
#define SYSTEM_PAT_FONT_PATH_3 "/System/Library/Fonts/Courier New.ttf"
#elif defined(ANDROID)
#define SYSTEM_FONT_PATH_1 "/system/fonts/Roboto-Regular.ttf"
#define SYSTEM_FONT_PATH_2 "/system/fonts/DroidSans.ttf"
#define SYSTEM_FONT_PATH_3 "/system/fonts/DroidSans.ttf"
// ???
#define SYSTEM_HEAD_FONT_PATH_1 "/system/fonts/Roboto-Regular.ttf"
#define SYSTEM_HEAD_FONT_PATH_2 "/system/fonts/DroidSans.ttf"
#define SYSTEM_HEAD_FONT_PATH_3 "/system/fonts/DroidSans.ttf"
#define SYSTEM_PAT_FONT_PATH_1 "/system/fonts/RobotoMono-Regular.ttf"
#define SYSTEM_PAT_FONT_PATH_2 "/system/fonts/DroidSansMono.ttf"
#define SYSTEM_PAT_FONT_PATH_3 "/system/fonts/CutiveMono.ttf"
#else
#define SYSTEM_FONT_PATH_1 "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
#define SYSTEM_FONT_PATH_2 "/usr/share/fonts/TTF/DejaVuSans.ttf"
#define SYSTEM_FONT_PATH_3 "/usr/share/fonts/ubuntu/Ubuntu-R.ttf"
#define SYSTEM_HEAD_FONT_PATH_1 "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
#define SYSTEM_HEAD_FONT_PATH_2 "/usr/share/fonts/TTF/DejaVuSans.ttf"
#define SYSTEM_HEAD_FONT_PATH_3 "/usr/share/fonts/ubuntu/Ubuntu-R.ttf"
#define SYSTEM_PAT_FONT_PATH_1 "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"
#define SYSTEM_PAT_FONT_PATH_2 "/usr/share/fonts/TTF/DejaVuSansMono.ttf"
#define SYSTEM_PAT_FONT_PATH_3 "/usr/share/fonts/ubuntu/UbuntuMono-R.ttf"
#endif

void setupLabel(const char* lStr, char* label, int len) {
  memset(label,0,32);
  for (int i=0, p=0; i<len; i++) {
    signed char cl;
    if (lStr[p]==0) {
      strncat(label," ",32);
    } else {
      decodeUTF8((const unsigned char*)&lStr[p],cl);
      memcpy(label+p,lStr+p,cl);
      p+=cl;
    }
  }
}

void FurnaceGUI::decompileNoteKeys() {
  noteKeysRaw.clear();
  for (std::map<int,int>::value_type& i: noteKeys) {
    std::vector<MappedInput>::iterator j;
    for (j=noteKeysRaw.begin(); j!=noteKeysRaw.end(); j++) {
      if (j->val>i.second) {
        break;
      }
    }
    noteKeysRaw.insert(j,MappedInput(i.first,i.second));
  }
}

void FurnaceGUI::compileNoteKeys() {
  noteKeys.clear();
  for (MappedInput& i: noteKeysRaw) {
    noteKeys[i.scan]=i.val;
  }
  decompileNoteKeys();
}

void FurnaceGUI::applyUISettings(bool updateFonts) {
  ImGuiStyle sty;
  if (settings.guiColorsBase) {
    ImGui::StyleColorsLight(&sty);
  } else {
    ImGui::StyleColorsDark(&sty);
  }

  if (dpiScale<0.1) dpiScale=0.1;

  setupLabel(settings.noteOffLabel.c_str(),noteOffLabel,3);
  setupLabel(settings.noteRelLabel.c_str(),noteRelLabel,3);
  setupLabel(settings.macroRelLabel.c_str(),macroRelLabel,3);
  setupLabel(settings.emptyLabel.c_str(),emptyLabel,3);
  setupLabel(settings.emptyLabel2.c_str(),emptyLabel2,2);

  if (updateFonts) {
    // get scale factor
    const char* videoBackend=SDL_GetCurrentVideoDriver();
    if (settings.dpiScale>=0.5f) {
      logD("setting UI scale factor from config (%f).",settings.dpiScale);
      dpiScale=settings.dpiScale;
    } else {
      logD("auto-detecting UI scale factor.");
      dpiScale=getScaleFactor(videoBackend,sdlWin);
      logD("scale factor: %f",dpiScale);
      if (dpiScale<0.1f) {
        logW("scale what?");
        dpiScale=1.0f;
      }
    }
  }

  if (updateFonts) {
    // chan osc work pool
    if (chanOscWorkPool!=NULL) {
      delete chanOscWorkPool;
      chanOscWorkPool=NULL;
    }
  }

  for (int i=0; i<64; i++) {
    ImVec4 col1=uiColors[GUI_COLOR_PATTERN_VOLUME_MIN];
    ImVec4 col2=uiColors[GUI_COLOR_PATTERN_VOLUME_HALF];
    ImVec4 col3=uiColors[GUI_COLOR_PATTERN_VOLUME_MAX];
    volColors[i]=ImVec4(col1.x+((col2.x-col1.x)*float(i)/64.0f),
                        col1.y+((col2.y-col1.y)*float(i)/64.0f),
                        col1.z+((col2.z-col1.z)*float(i)/64.0f),
                        1.0f);
    volColors[i+64]=ImVec4(col2.x+((col3.x-col2.x)*float(i)/64.0f),
                           col2.y+((col3.y-col2.y)*float(i)/64.0f),
                           col2.z+((col3.z-col2.z)*float(i)/64.0f),
                           1.0f);
  }

  float hue, sat, val;

  ImVec4 primaryActive=uiColors[GUI_COLOR_ACCENT_PRIMARY];
  ImVec4 primaryHover, primary;
  primaryHover.w=primaryActive.w;
  primary.w=primaryActive.w;
  ImGui::ColorConvertRGBtoHSV(primaryActive.x,primaryActive.y,primaryActive.z,hue,sat,val);
  if (settings.guiColorsBase) {
    primary=primaryActive;
    ImGui::ColorConvertHSVtoRGB(hue,sat*0.9,val*0.9,primaryHover.x,primaryHover.y,primaryHover.z);
    ImGui::ColorConvertHSVtoRGB(hue,sat,val*0.5,primaryActive.x,primaryActive.y,primaryActive.z);
  } else {
    ImGui::ColorConvertHSVtoRGB(hue,sat*0.9,val*0.5,primaryHover.x,primaryHover.y,primaryHover.z);
    ImGui::ColorConvertHSVtoRGB(hue,sat*0.8,val*0.35,primary.x,primary.y,primary.z);
  }

  ImVec4 secondaryActive=uiColors[GUI_COLOR_ACCENT_SECONDARY];
  ImVec4 secondaryHoverActual, secondaryHover, secondary, secondarySemiActive;
  secondarySemiActive.w=secondaryActive.w;
  secondaryHover.w=secondaryActive.w;
  secondary.w=secondaryActive.w;
  ImGui::ColorConvertRGBtoHSV(secondaryActive.x,secondaryActive.y,secondaryActive.z,hue,sat,val);
  if (settings.guiColorsBase) {
    secondary=secondaryActive;
    ImGui::ColorConvertHSVtoRGB(hue,sat*0.9,val*0.7,secondarySemiActive.x,secondarySemiActive.y,secondarySemiActive.z);
    ImGui::ColorConvertHSVtoRGB(hue,sat*0.9,val*0.9,secondaryHover.x,secondaryHover.y,secondaryHover.z);
    ImGui::ColorConvertHSVtoRGB(hue,sat,val*0.5,secondaryActive.x,secondaryActive.y,secondaryActive.z);
  } else {
    ImGui::ColorConvertHSVtoRGB(hue,sat*0.9,val*0.75,secondarySemiActive.x,secondarySemiActive.y,secondarySemiActive.z);
    ImGui::ColorConvertHSVtoRGB(hue,sat*0.9,val*0.5,secondaryHover.x,secondaryHover.y,secondaryHover.z);
    ImGui::ColorConvertHSVtoRGB(hue,sat*0.9,val*0.25,secondary.x,secondary.y,secondary.z);
  }

  secondaryHoverActual=secondaryHover;

  // TODO: improve
  if (mobileUI) { // disable all hovered colors
    primaryHover=primary;
    secondaryHover=secondary;
  }

  sty.Colors[ImGuiCol_WindowBg]=uiColors[GUI_COLOR_FRAME_BACKGROUND];
  sty.Colors[ImGuiCol_ChildBg]=uiColors[GUI_COLOR_FRAME_BACKGROUND_CHILD];
  sty.Colors[ImGuiCol_PopupBg]=uiColors[GUI_COLOR_FRAME_BACKGROUND_POPUP];
  sty.Colors[ImGuiCol_TitleBg]=uiColors[GUI_COLOR_TITLE_INACTIVE];
  sty.Colors[ImGuiCol_TitleBgCollapsed]=uiColors[GUI_COLOR_TITLE_COLLAPSED];
  sty.Colors[ImGuiCol_MenuBarBg]=uiColors[GUI_COLOR_MENU_BAR];
  sty.Colors[ImGuiCol_ModalWindowDimBg]=uiColors[GUI_COLOR_MODAL_BACKDROP];
  sty.Colors[ImGuiCol_ScrollbarBg]=uiColors[GUI_COLOR_SCROLL_BACKGROUND];
  sty.Colors[ImGuiCol_ScrollbarGrab]=uiColors[GUI_COLOR_SCROLL];
  sty.Colors[ImGuiCol_ScrollbarGrabHovered]=uiColors[GUI_COLOR_SCROLL_HOVER];
  sty.Colors[ImGuiCol_ScrollbarGrabActive]=uiColors[GUI_COLOR_SCROLL_ACTIVE];
  sty.Colors[ImGuiCol_Separator]=uiColors[GUI_COLOR_SEPARATOR];
  sty.Colors[ImGuiCol_SeparatorHovered]=uiColors[GUI_COLOR_SEPARATOR_HOVER];
  sty.Colors[ImGuiCol_SeparatorActive]=uiColors[GUI_COLOR_SEPARATOR_ACTIVE];
  sty.Colors[ImGuiCol_DockingPreview]=uiColors[GUI_COLOR_DOCKING_PREVIEW];
  sty.Colors[ImGuiCol_DockingEmptyBg]=uiColors[GUI_COLOR_DOCKING_EMPTY];
  sty.Colors[ImGuiCol_TableHeaderBg]=uiColors[GUI_COLOR_TABLE_HEADER];
  sty.Colors[ImGuiCol_TableBorderStrong]=uiColors[GUI_COLOR_TABLE_BORDER_HARD];
  sty.Colors[ImGuiCol_TableBorderLight]=uiColors[GUI_COLOR_TABLE_BORDER_SOFT];
  sty.Colors[ImGuiCol_DragDropTarget]=uiColors[GUI_COLOR_DRAG_DROP_TARGET];
  sty.Colors[ImGuiCol_NavCursor]=uiColors[GUI_COLOR_NAV_HIGHLIGHT];
  sty.Colors[ImGuiCol_NavWindowingHighlight]=uiColors[GUI_COLOR_NAV_WIN_HIGHLIGHT];
  sty.Colors[ImGuiCol_NavWindowingDimBg]=uiColors[GUI_COLOR_NAV_WIN_BACKDROP];
  sty.Colors[ImGuiCol_Text]=uiColors[GUI_COLOR_TEXT];
  sty.Colors[ImGuiCol_TextDisabled]=uiColors[GUI_COLOR_TEXT_DISABLED];

  // new stuff
  sty.Colors[ImGuiCol_InputTextCursor]=uiColors[GUI_COLOR_INPUT_TEXT_CURSOR];

  if (settings.basicColors) {
    sty.Colors[ImGuiCol_Button]=primary;
    sty.Colors[ImGuiCol_ButtonHovered]=primaryHover;
    sty.Colors[ImGuiCol_ButtonActive]=primaryActive;
    sty.Colors[ImGuiCol_Tab]=primary;
    sty.Colors[ImGuiCol_TabHovered]=secondaryHover;
    sty.Colors[ImGuiCol_TabSelected]=secondarySemiActive;
    sty.Colors[ImGuiCol_TabSelectedOverline]=secondaryActive;
    sty.Colors[ImGuiCol_TabDimmed]=primary;
    sty.Colors[ImGuiCol_TabDimmedSelected]=primaryHover;
    sty.Colors[ImGuiCol_TabDimmedSelectedOverline]=primaryActive;
    sty.Colors[ImGuiCol_Header]=secondary;
    sty.Colors[ImGuiCol_HeaderHovered]=secondaryHover;
    sty.Colors[ImGuiCol_HeaderActive]=secondaryActive;
    sty.Colors[ImGuiCol_ResizeGrip]=secondary;
    sty.Colors[ImGuiCol_ResizeGripHovered]=secondaryHover;
    sty.Colors[ImGuiCol_ResizeGripActive]=secondaryActive;
    sty.Colors[ImGuiCol_FrameBg]=secondary;
    sty.Colors[ImGuiCol_FrameBgHovered]=secondaryHover;
    sty.Colors[ImGuiCol_FrameBgActive]=secondaryActive;
    sty.Colors[ImGuiCol_SliderGrab]=primaryActive;
    sty.Colors[ImGuiCol_SliderGrabActive]=primaryActive;
    sty.Colors[ImGuiCol_TitleBgActive]=primary;
    sty.Colors[ImGuiCol_CheckMark]=primaryActive;
    sty.Colors[ImGuiCol_TextLink]=secondaryActive;
    sty.Colors[ImGuiCol_TextSelectedBg]=secondaryHoverActual;
    sty.Colors[ImGuiCol_TreeLines]=uiColors[GUI_COLOR_BORDER];
    sty.Colors[ImGuiCol_PlotHistogram]=uiColors[GUI_COLOR_MACRO_OTHER];
    sty.Colors[ImGuiCol_PlotHistogramHovered]=uiColors[GUI_COLOR_MACRO_OTHER];
  } else {
    sty.Colors[ImGuiCol_Button]=uiColors[GUI_COLOR_BUTTON];
    sty.Colors[ImGuiCol_ButtonHovered]=uiColors[GUI_COLOR_BUTTON_HOVER];
    sty.Colors[ImGuiCol_ButtonActive]=uiColors[GUI_COLOR_BUTTON_ACTIVE];
    sty.Colors[ImGuiCol_Tab]=uiColors[GUI_COLOR_TAB];
    sty.Colors[ImGuiCol_TabHovered]=uiColors[GUI_COLOR_TAB_HOVER];
    sty.Colors[ImGuiCol_TabSelected]=uiColors[GUI_COLOR_TAB_ACTIVE];
    sty.Colors[ImGuiCol_TabSelectedOverline]=uiColors[GUI_COLOR_TAB_SELECTED_OVERLINE];
    sty.Colors[ImGuiCol_TabDimmed]=uiColors[GUI_COLOR_TAB_UNFOCUSED];
    sty.Colors[ImGuiCol_TabDimmedSelected]=uiColors[GUI_COLOR_TAB_UNFOCUSED_ACTIVE];
    sty.Colors[ImGuiCol_TabDimmedSelectedOverline]=uiColors[GUI_COLOR_TAB_DIMMED_SELECTED_OVERLINE];
    sty.Colors[ImGuiCol_Header]=uiColors[GUI_COLOR_IMGUI_HEADER];
    sty.Colors[ImGuiCol_HeaderHovered]=uiColors[GUI_COLOR_IMGUI_HEADER_HOVER];
    sty.Colors[ImGuiCol_HeaderActive]=uiColors[GUI_COLOR_IMGUI_HEADER_ACTIVE];
    sty.Colors[ImGuiCol_ResizeGrip]=uiColors[GUI_COLOR_RESIZE_GRIP];
    sty.Colors[ImGuiCol_ResizeGripHovered]=uiColors[GUI_COLOR_RESIZE_GRIP_HOVER];
    sty.Colors[ImGuiCol_ResizeGripActive]=uiColors[GUI_COLOR_RESIZE_GRIP_ACTIVE];
    sty.Colors[ImGuiCol_FrameBg]=uiColors[GUI_COLOR_WIDGET_BACKGROUND];
    sty.Colors[ImGuiCol_FrameBgHovered]=uiColors[GUI_COLOR_WIDGET_BACKGROUND_HOVER];
    sty.Colors[ImGuiCol_FrameBgActive]=uiColors[GUI_COLOR_WIDGET_BACKGROUND_ACTIVE];
    sty.Colors[ImGuiCol_SliderGrab]=uiColors[GUI_COLOR_SLIDER_GRAB];
    sty.Colors[ImGuiCol_SliderGrabActive]=uiColors[GUI_COLOR_SLIDER_GRAB_ACTIVE];
    sty.Colors[ImGuiCol_TitleBgActive]=uiColors[GUI_COLOR_TITLE_BACKGROUND_ACTIVE];
    sty.Colors[ImGuiCol_CheckMark]=uiColors[GUI_COLOR_CHECK_MARK];
    sty.Colors[ImGuiCol_TextLink]=uiColors[GUI_COLOR_TEXT_LINK];
    sty.Colors[ImGuiCol_TextSelectedBg]=uiColors[GUI_COLOR_TEXT_SELECTION];
    sty.Colors[ImGuiCol_TreeLines]=uiColors[GUI_COLOR_TREE_LINES];
    sty.Colors[ImGuiCol_PlotLines]=uiColors[GUI_COLOR_PLOT_LINES];
    sty.Colors[ImGuiCol_PlotLinesHovered]=uiColors[GUI_COLOR_PLOT_LINES_HOVER];
    sty.Colors[ImGuiCol_PlotHistogram]=uiColors[GUI_COLOR_PLOT_HISTOGRAM];
    sty.Colors[ImGuiCol_PlotHistogramHovered]=uiColors[GUI_COLOR_PLOT_HISTOGRAM_HOVER];
    sty.Colors[ImGuiCol_TableRowBg]=uiColors[GUI_COLOR_TABLE_ROW_EVEN];
    sty.Colors[ImGuiCol_TableRowBgAlt]=uiColors[GUI_COLOR_TABLE_ROW_ODD];
  }
  sty.Colors[ImGuiCol_Border]=uiColors[GUI_COLOR_BORDER];
  sty.Colors[ImGuiCol_BorderShadow]=uiColors[GUI_COLOR_BORDER_SHADOW];

  if (settings.roundedWindows) sty.WindowRounding=8.0f;
  if (settings.roundedButtons) {
    sty.FrameRounding=6.0f;
    sty.GrabRounding=6.0f;
  }
  if (settings.roundedMenus) sty.PopupRounding=8.0f;
  if (settings.roundedTabs) {
    sty.TabRounding=4.0f;
  } else {
    sty.TabRounding=0.0f;
  }
  if (settings.roundedScrollbars) {
    sty.ScrollbarRounding=9.0f;
  } else {
    sty.ScrollbarRounding=0.0f;
  }

  if (settings.frameBorders) {
    sty.FrameBorderSize=1.0f;
  } else {
    sty.FrameBorderSize=0.0f;
  }

  if (settings.guiColorsShading>0) {
    sty.FrameShading=(float)settings.guiColorsShading/100.0f;
  }

  if (safeMode || renderBackend==GUI_BACKEND_SOFTWARE) {
    sty.WindowRounding=0.0f;
    sty.FrameRounding=0.0f;
    sty.GrabRounding=0.0f;
    sty.FrameShading=0.0f;
    sty.TabRounding=0.0f;
    sty.ScrollbarRounding=0.0f;
    sty.ChildRounding=0.0f;
    sty.PopupRounding=0.0f;
    sty.AntiAliasedLines=false;
    sty.AntiAliasedLinesUseTex=false;
    sty.AntiAliasedFill=false;
  }

  if (mobileUI) {
    sty.FramePadding=ImVec2(8.0f,6.0f);
  }

  sty.ScaleAllSizes(dpiScale);

  ImGui::GetStyle()=sty;

  updateSampleTex=true;

  ImGui::GetIO().ConfigInputTrickleEventQueue=settings.eventDelay;
  ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly=settings.moveWindowTitle;
  ImGui::GetIO().ConfigInertialScrollToleranceSqr=pow(dpiScale*4.0f,2.0f);
  ImGui::GetIO().MouseDoubleClickTime=settings.doubleClickTime;
  ImGui::GetIO().ScrollTextSpacing=8.0*dpiScale;
  ImGui::GetIO().ScrollTextSpeed=60.0*dpiScale;

  for (int i=0; i<256; i++) {
    ImVec4& base=uiColors[GUI_COLOR_PATTERN_EFFECT_PITCH];
    pitchGrad[i]=ImGui::GetColorU32(ImVec4(base.x,base.y,base.z,((float)i/255.0f)*base.w));
  }
  for (int i=0; i<256; i++) {
    ImVec4& base=uiColors[GUI_COLOR_PATTERN_ACTIVE];
    noteGrad[i]=ImGui::GetColorU32(ImVec4(base.x,base.y,base.z,((float)i/255.0f)*base.w));
  }
  for (int i=0; i<256; i++) {
    ImVec4& base=uiColors[GUI_COLOR_PATTERN_EFFECT_PANNING];
    panGrad[i]=ImGui::GetColorU32(ImVec4(base.x,base.y,base.z,((float)i/255.0f)*base.w));
  }
  for (int i=0; i<256; i++) {
    ImVec4& base=uiColors[GUI_COLOR_PATTERN_INS];
    insGrad[i]=ImGui::GetColorU32(ImVec4(base.x,base.y,base.z,((float)i/255.0f)*base.w));
  }
  for (int i=0; i<256; i++) {
    ImVec4& base=volColors[i/2];
    volGrad[i]=ImGui::GetColorU32(ImVec4(base.x,base.y,base.z,((float)i/255.0f)*base.w));
  }
  for (int i=0; i<256; i++) {
    ImVec4& base=uiColors[GUI_COLOR_PATTERN_EFFECT_SYS_PRIMARY];
    sysCmd1Grad[i]=ImGui::GetColorU32(ImVec4(base.x,base.y,base.z,((float)i/255.0f)*base.w));
  }
  for (int i=0; i<256; i++) {
    ImVec4& base=uiColors[GUI_COLOR_PATTERN_EFFECT_SYS_SECONDARY];
    sysCmd2Grad[i]=ImGui::GetColorU32(ImVec4(base.x,base.y,base.z,((float)i/255.0f)*base.w));
  }

  if (updateFonts && !safeMode) {
    // prepare
#ifdef HAVE_FREETYPE
    if (settings.fontBackend==1) {
      ImGui::GetIO().Fonts->SetFontLoader(ImGuiFreeType::GetFontLoader());
      ImGui::GetIO().Fonts->FontLoaderFlags&=~(
        ImGuiFreeTypeLoaderFlags_NoHinting|
        ImGuiFreeTypeLoaderFlags_NoAutoHint|
        ImGuiFreeTypeLoaderFlags_ForceAutoHint|
        ImGuiFreeTypeLoaderFlags_LightHinting|
        ImGuiFreeTypeLoaderFlags_MonoHinting|
        ImGuiFreeTypeLoaderFlags_Bold|
        ImGuiFreeTypeLoaderFlags_Oblique|
        ImGuiFreeTypeLoaderFlags_Monochrome|
        ImGuiFreeTypeLoaderFlags_LoadColor|
        ImGuiFreeTypeLoaderFlags_Bitmap
      );

      if (!settings.fontAntiAlias) ImGui::GetIO().Fonts->FontLoaderFlags|=ImGuiFreeTypeLoaderFlags_Monochrome;
      if (settings.fontBitmap) ImGui::GetIO().Fonts->FontLoaderFlags|=ImGuiFreeTypeLoaderFlags_Bitmap;

      switch (settings.fontHinting) {
        case 0: // off
          ImGui::GetIO().Fonts->FontLoaderFlags|=ImGuiFreeTypeLoaderFlags_NoHinting;
          break;
        case 1: // slight
          ImGui::GetIO().Fonts->FontLoaderFlags|=ImGuiFreeTypeLoaderFlags_LightHinting;
          break;
        case 2: // normal
          break;
        case 3: // full
          ImGui::GetIO().Fonts->FontLoaderFlags|=ImGuiFreeTypeLoaderFlags_MonoHinting;
          break;
      }

      switch (settings.fontAutoHint) {
        case 0: // off
          ImGui::GetIO().Fonts->FontLoaderFlags|=ImGuiFreeTypeLoaderFlags_NoAutoHint;
          break;
        case 1: // on
          break;
        case 2: // force
          ImGui::GetIO().Fonts->FontLoaderFlags|=ImGuiFreeTypeLoaderFlags_ForceAutoHint;
          break;
      }
    } else {
      ImGui::GetIO().Fonts->SetFontLoader(ImFontAtlasGetFontLoaderForStbTruetype());
    }
#endif

    ImFontConfig fontConf;
    ImFontConfig fontConfP;
    ImFontConfig fontConfB;
    ImFontConfig fontConfH;

    fontConf.OversampleV=1;
    fontConf.OversampleH=settings.fontOversample;
    fontConfP.OversampleV=1;
    fontConfP.OversampleH=settings.fontOversample;
    fontConfB.OversampleV=1;
    fontConfB.OversampleH=1;
    fontConfH.OversampleV=1;
    fontConfH.OversampleH=1;

    if (safeMode || renderBackend==GUI_BACKEND_SOFTWARE) {
      fontConf.OversampleV=1;
      fontConf.OversampleH=1;
      fontConfP.OversampleV=1;
      fontConfP.OversampleH=1;
      fontConfB.OversampleV=1;
      fontConfB.OversampleH=1;
      fontConfH.OversampleV=1;
      fontConfH.OversampleH=1;
    }

    //fontConf.RasterizerMultiply=1.5;
    //fontConfP.RasterizerMultiply=1.5;

    fontConf.PixelSnapH=0;
    fontConfP.PixelSnapH=0;
    fontConfB.PixelSnapH=0;
    fontConfH.PixelSnapH=0;

    if (settings.mainFont<0 || settings.mainFont>6) settings.mainFont=0;
    if (settings.headFont<0 || settings.headFont>6) settings.headFont=0;
    if (settings.patFont<0 || settings.patFont>6) settings.patFont=0;

    if (settings.mainFont==6 && settings.mainFontPath.empty()) {
      logW("UI font path is empty! reverting to default font");
      settings.mainFont=GUI_MAIN_FONT_DEFAULT;
    }
    if (settings.headFont==6 && settings.headFontPath.empty()) {
      logW("header font path is empty! reverting to default font");
      settings.headFont=0;
    }
    if (settings.patFont==6 && settings.patFontPath.empty()) {
      logW("pattern font path is empty! reverting to default font");
      settings.patFont=GUI_PAT_FONT_DEFAULT;
    }

    ImFontConfig fc1;
    fc1.MergeMode=true;
    // save memory
    fc1.OversampleH=1;
    fc1.OversampleV=1;

    if (settings.mainFont==6) { // custom font
      if ((mainFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(settings.mainFontPath.c_str(),MAX(1,e->getConfInt("mainFontSize",18)*dpiScale),&fontConf))==NULL) {
        logW("could not load UI font! reverting to default font");
        settings.mainFont=GUI_MAIN_FONT_DEFAULT;
        if ((mainFont=addFontZlib(builtinFont[settings.mainFont],builtinFontLen[settings.mainFont],MAX(1,e->getConfInt("mainFontSize",18)*dpiScale),&fontConf))==NULL) {
          logE("could not load UI font! falling back to Proggy Clean.");
          mainFont=ImGui::GetIO().Fonts->AddFontDefault();
        }
      }
    } else if (settings.mainFont==5) { // system font
      if ((mainFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_FONT_PATH_1,MAX(1,e->getConfInt("mainFontSize",18)*dpiScale),&fontConf))==NULL) {
        if ((mainFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_FONT_PATH_2,MAX(1,e->getConfInt("mainFontSize",18)*dpiScale),&fontConf))==NULL) {
          if ((mainFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_FONT_PATH_3,MAX(1,e->getConfInt("mainFontSize",18)*dpiScale),&fontConf))==NULL) {
            logW("could not load UI font! reverting to default font");
            settings.mainFont=GUI_MAIN_FONT_DEFAULT;
            if ((mainFont=addFontZlib(builtinFont[settings.mainFont],builtinFontLen[settings.mainFont],MAX(1,e->getConfInt("mainFontSize",18)*dpiScale),&fontConf))==NULL) {
              logE("could not load UI font! falling back to Proggy Clean.");
              mainFont=ImGui::GetIO().Fonts->AddFontDefault();
            }
          }
        }
      }
    } else {
      if ((mainFont=addFontZlib(builtinFont[settings.mainFont],builtinFontLen[settings.mainFont],MAX(1,e->getConfInt("mainFontSize",18)*dpiScale),&fontConf))==NULL) {
        logE("could not load UI font! falling back to Proggy Clean.");
        mainFont=ImGui::GetIO().Fonts->AddFontDefault();
      }
    }

    // four fallback fonts
    if (settings.loadFallback) {
      mainFont=addFontZlib(font_plexSans_compressed_data,font_plexSans_compressed_size,MAX(1,e->getConfInt("mainFontSize",18)*dpiScale),&fc1);
      mainFont=addFontZlib(font_plexSansJP_compressed_data,font_plexSansJP_compressed_size,MAX(1,e->getConfInt("mainFontSize",18)*dpiScale),&fc1);
      mainFont=addFontZlib(font_plexSansKR_compressed_data,font_plexSansKR_compressed_size,MAX(1,e->getConfInt("mainFontSize",18)*dpiScale),&fc1);
      mainFont=addFontZlib(font_unifont_compressed_data,font_unifont_compressed_size,MAX(1,e->getConfInt("mainFontSize",18)*dpiScale),&fc1);
    }

    ImFontConfig fc;
    fc.MergeMode=true;
    fc.OversampleH=1;
    fc.OversampleV=1;
    fc.PixelSnapH=true;
    fc.GlyphMinAdvanceX=e->getConfInt("iconSize",16)*dpiScale;
    if ((iconFont=addFontZlib(iconFont_compressed_data,iconFont_compressed_size,MAX(1,e->getConfInt("iconSize",16)*dpiScale),&fc))==NULL) {
      logE("could not load icon font!");
    }

    if ((furIconFont=addFontZlib(furIcons_compressed_data,furIcons_compressed_size,MAX(1,e->getConfInt("iconSize",16)*dpiScale),&fc))==NULL) {
      logE("could not load Furnace icons font!");
    }

    if (settings.mainFontSize==settings.patFontSize && settings.patFont<5 && builtinFontM[settings.patFont]==builtinFont[settings.mainFont]) {
      logD("using main font for pat font.");
      patFont=mainFont;
    } else {
      if (settings.patFont==6) { // custom font
        if ((patFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(settings.patFontPath.c_str(),MAX(1,e->getConfInt("patFontSize",18)*dpiScale),&fontConfP))==NULL) {
          logW("could not load pattern font! reverting to default font");
          settings.patFont=GUI_PAT_FONT_DEFAULT;
          if ((patFont=addFontZlib(builtinFontM[settings.patFont],builtinFontMLen[settings.patFont],MAX(1,e->getConfInt("patFontSize",18)*dpiScale),&fontConfP))==NULL) {
            logE("could not load pattern font! falling back to Proggy Clean.");
            patFont=ImGui::GetIO().Fonts->AddFontDefault();
          }
        }
      } else if (settings.patFont==5) { // system font
        if ((patFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_PAT_FONT_PATH_1,MAX(1,e->getConfInt("patFontSize",18)*dpiScale),&fontConfP))==NULL) {
          if ((patFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_PAT_FONT_PATH_2,MAX(1,e->getConfInt("patFontSize",18)*dpiScale),&fontConfP))==NULL) {
            if ((patFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_PAT_FONT_PATH_3,MAX(1,e->getConfInt("patFontSize",18)*dpiScale),&fontConfP))==NULL) {
              logW("could not load pattern font! reverting to default font");
              settings.patFont=GUI_PAT_FONT_DEFAULT;
              if ((patFont=addFontZlib(builtinFontM[settings.patFont],builtinFontMLen[settings.patFont],MAX(1,e->getConfInt("patFontSize",18)*dpiScale),&fontConfP))==NULL) {
                logE("could not load pattern font! falling back to Proggy Clean.");
                patFont=ImGui::GetIO().Fonts->AddFontDefault();
              }
            }
          }
        }
      } else {
        if ((patFont=addFontZlib(builtinFontM[settings.patFont],builtinFontMLen[settings.patFont],MAX(1,e->getConfInt("patFontSize",18)*dpiScale),&fontConfP))==NULL) {
          logE("could not load pattern font!");
          patFont=ImGui::GetIO().Fonts->AddFontDefault();
        }
      }
    }

    // four fallback fonts
    if (settings.loadFallbackPat && (
        localeRequiresJapanese ||
        localeRequiresChinese ||
        localeRequiresChineseTrad ||
        localeRequiresKorean)) {
      patFont=addFontZlib(font_plexMono_compressed_data,font_plexMono_compressed_size,MAX(1,e->getConfInt("patFontSize",18)*dpiScale),&fc1);
      patFont=addFontZlib(font_plexSansJP_compressed_data,font_plexSansJP_compressed_size,MAX(1,e->getConfInt("patFontSize",18)*dpiScale),&fc1);
      patFont=addFontZlib(font_plexSansKR_compressed_data,font_plexSansKR_compressed_size,MAX(1,e->getConfInt("patFontSize",18)*dpiScale),&fc1);
      patFont=addFontZlib(font_unifont_compressed_data,font_unifont_compressed_size,MAX(1,e->getConfInt("patFontSize",18)*dpiScale),&fc1);
    }

    if ((bigFont=addFontZlib(font_plexSans_compressed_data,font_plexSans_compressed_size,MAX(1,40*dpiScale),&fontConfB))==NULL) {
      logE("could not load big UI font!");
    }
    fontConfB.MergeMode=true;
    if ((bigFont=addFontZlib(font_plexSansJP_compressed_data,font_plexSansJP_compressed_size,MAX(1,40*dpiScale),&fontConfB))==NULL) {
      logE("could not load big UI font (japanese)!");
    }
    if ((bigFont=addFontZlib(font_plexSansKR_compressed_data,font_plexSansKR_compressed_size,MAX(1,40*dpiScale),&fontConfB))==NULL) {
      logE("could not load big UI font (korean)!");
    }
    if ((bigFont=addFontZlib(font_unifont_compressed_data,font_unifont_compressed_size,MAX(1,40*dpiScale),&fontConfB))==NULL) {
      logE("could not load big UI font (fallback)!");
    }

    if (settings.mainFontSize==settings.headFontSize && settings.headFont<5 && builtinFont[settings.headFont]==builtinFont[settings.mainFont]) {
      logD("using main font for header font.");
      headFont=mainFont;
    } else {
      if (settings.headFont==6) { // custom font
        if ((headFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(settings.headFontPath.c_str(),MAX(1,e->getConfInt("headFontSize",27)*dpiScale),&fontConfH))==NULL) {
          logW("could not load header font! reverting to default font");
          settings.headFont=0;
          if ((headFont=addFontZlib(builtinFont[settings.headFont],builtinFontLen[settings.headFont],MAX(1,e->getConfInt("headFontSize",27)*dpiScale),&fontConfH))==NULL) {
            logE("could not load header font! falling back to IBM Plex Sans.");
            headFont=ImGui::GetIO().Fonts->AddFontDefault();
          }
        }
      } else if (settings.headFont==5) { // system font
        if ((headFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_HEAD_FONT_PATH_1,MAX(1,e->getConfInt("headFontSize",27)*dpiScale),&fontConfH))==NULL) {
          if ((headFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_HEAD_FONT_PATH_2,MAX(1,e->getConfInt("headFontSize",27)*dpiScale),&fontConfH))==NULL) {
            if ((headFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_HEAD_FONT_PATH_3,MAX(1,e->getConfInt("headFontSize",27)*dpiScale),&fontConfH))==NULL) {
              logW("could not load header font! reverting to default font");
              settings.headFont=0;
              if ((headFont=addFontZlib(builtinFont[settings.headFont],builtinFontLen[settings.headFont],MAX(1,e->getConfInt("headFontSize",27)*dpiScale),&fontConfH))==NULL) {
                logE("could not load header font! falling back to fallback. wahahaha, get it? fallback.");
                headFont=ImGui::GetIO().Fonts->AddFontDefault();
              }
            }
          }
        }
      } else {
        if ((headFont=addFontZlib(builtinFont[settings.headFont],builtinFontLen[settings.headFont],MAX(1,e->getConfInt("headFontSize",27)*dpiScale),&fontConfH))==NULL) {
          logE("could not load header font!");
          headFont=ImGui::GetIO().Fonts->AddFontDefault();
        }
      }
    }

    // four fallback fonts
    if (settings.loadFallback) {
      headFont=addFontZlib(font_plexSans_compressed_data,font_plexSans_compressed_size,MAX(1,e->getConfInt("headFontSize",27)*dpiScale),&fc1);
      headFont=addFontZlib(font_plexSansJP_compressed_data,font_plexSansJP_compressed_size,MAX(1,e->getConfInt("headFontSize",27)*dpiScale),&fc1);
      headFont=addFontZlib(font_plexSansKR_compressed_data,font_plexSansKR_compressed_size,MAX(1,e->getConfInt("headFontSize",27)*dpiScale),&fc1);
      headFont=addFontZlib(font_unifont_compressed_data,font_unifont_compressed_size,MAX(1,e->getConfInt("headFontSize",27)*dpiScale),&fc1);
    }

    //mainFont->FallbackChar='?';
    //mainFont->EllipsisChar='.';
    //mainFont->EllipsisCharCount=3;
  } else if (updateFonts) {
    // safe mode
    mainFont=ImGui::GetIO().Fonts->AddFontDefault();
    patFont=mainFont;
    bigFont=mainFont;
    headFont=mainFont;
  }

  // set built-in file picker up (NEW)
  newFilePicker->clearTypes();

  newFilePicker->setTypeStyle(FP_TYPE_UNKNOWN,uiColors[GUI_COLOR_FILE_OTHER],ICON_FA_QUESTION);
  newFilePicker->setTypeStyle(FP_TYPE_NORMAL,uiColors[GUI_COLOR_FILE_OTHER],ICON_FA_FILE_O);
  newFilePicker->setTypeStyle(FP_TYPE_DIR,uiColors[GUI_COLOR_FILE_DIR],ICON_FA_FOLDER_O);
  newFilePicker->setTypeStyle(FP_TYPE_LINK,uiColors[GUI_COLOR_FILE_OTHER],ICON_FA_EXTERNAL_LINK);
  newFilePicker->setTypeStyle(FP_TYPE_PIPE,uiColors[GUI_COLOR_FILE_OTHER],ICON_FA_EXCHANGE);
  newFilePicker->setTypeStyle(FP_TYPE_SOCKET,uiColors[GUI_COLOR_FILE_OTHER],ICON_FA_PLUG);
  newFilePicker->setTypeStyle(FP_TYPE_CHARDEV,uiColors[GUI_COLOR_FILE_OTHER],ICON_FA_MICROCHIP);
  newFilePicker->setTypeStyle(FP_TYPE_BLOCKDEV,uiColors[GUI_COLOR_FILE_OTHER],ICON_FA_HDD_O);

  newFilePicker->registerType(".fur",uiColors[GUI_COLOR_FILE_SONG_NATIVE],ICON_FA_FILE);
  newFilePicker->registerType(".fui",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  newFilePicker->registerType(".dmp",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  newFilePicker->registerType(".fuw",uiColors[GUI_COLOR_FILE_WAVE],ICON_FA_FILE);
  newFilePicker->registerType(".dmw",uiColors[GUI_COLOR_FILE_WAVE],ICON_FA_FILE);

  newFilePicker->registerType(".wav",uiColors[GUI_COLOR_FILE_AUDIO],ICON_FA_FILE_AUDIO_O);
  newFilePicker->registerType(".dmc",uiColors[GUI_COLOR_FILE_AUDIO],ICON_FA_FILE_AUDIO_O);
  newFilePicker->registerType(".aiff",uiColors[GUI_COLOR_FILE_AUDIO],ICON_FA_FILE_AUDIO_O);
  newFilePicker->registerType(".aifc",uiColors[GUI_COLOR_FILE_AUDIO],ICON_FA_FILE_AUDIO_O);
  newFilePicker->registerType(".au",uiColors[GUI_COLOR_FILE_AUDIO],ICON_FA_FILE_AUDIO_O);
  newFilePicker->registerType(".caf",uiColors[GUI_COLOR_FILE_AUDIO],ICON_FA_FILE_AUDIO_O);
  newFilePicker->registerType(".iff",uiColors[GUI_COLOR_FILE_AUDIO],ICON_FA_FILE_AUDIO_O);
  newFilePicker->registerType(".mat",uiColors[GUI_COLOR_FILE_AUDIO],ICON_FA_FILE_AUDIO_O);
  newFilePicker->registerType(".w64",uiColors[GUI_COLOR_FILE_AUDIO],ICON_FA_FILE_AUDIO_O);
  newFilePicker->registerType(".wve",uiColors[GUI_COLOR_FILE_AUDIO],ICON_FA_FILE_AUDIO_O);
  newFilePicker->registerType(".rf64",uiColors[GUI_COLOR_FILE_AUDIO],ICON_FA_FILE_AUDIO_O);

  newFilePicker->registerType(".brr",uiColors[GUI_COLOR_FILE_AUDIO_COMPRESSED],ICON_FA_FILE_AUDIO_O);
  newFilePicker->registerType(".flac",uiColors[GUI_COLOR_FILE_AUDIO_COMPRESSED],ICON_FA_FILE_AUDIO_O);
  newFilePicker->registerType(".m1a",uiColors[GUI_COLOR_FILE_AUDIO_COMPRESSED],ICON_FA_FILE_AUDIO_O);
  newFilePicker->registerType(".mp1",uiColors[GUI_COLOR_FILE_AUDIO_COMPRESSED],ICON_FA_FILE_AUDIO_O);
  newFilePicker->registerType(".mp2",uiColors[GUI_COLOR_FILE_AUDIO_COMPRESSED],ICON_FA_FILE_AUDIO_O);
  newFilePicker->registerType(".mp3",uiColors[GUI_COLOR_FILE_AUDIO_COMPRESSED],ICON_FA_FILE_AUDIO_O);
  newFilePicker->registerType(".oga",uiColors[GUI_COLOR_FILE_AUDIO_COMPRESSED],ICON_FA_FILE_AUDIO_O);
  newFilePicker->registerType(".ogg",uiColors[GUI_COLOR_FILE_AUDIO_COMPRESSED],ICON_FA_FILE_AUDIO_O);
  newFilePicker->registerType(".opus",uiColors[GUI_COLOR_FILE_AUDIO_COMPRESSED],ICON_FA_FILE_AUDIO_O);
  newFilePicker->registerType(".pvf",uiColors[GUI_COLOR_FILE_AUDIO_COMPRESSED],ICON_FA_FILE_AUDIO_O);
  newFilePicker->registerType(".voc",uiColors[GUI_COLOR_FILE_AUDIO_COMPRESSED],ICON_FA_FILE_AUDIO_O);
  newFilePicker->registerType(".vox",uiColors[GUI_COLOR_FILE_AUDIO_COMPRESSED],ICON_FA_FILE_AUDIO_O);

  newFilePicker->registerType(".vgm",uiColors[GUI_COLOR_FILE_VGM],ICON_FA_FILE_AUDIO_O);
  newFilePicker->registerType(".zsm",uiColors[GUI_COLOR_FILE_ZSM],ICON_FA_FILE_AUDIO_O);

  newFilePicker->registerType(".ttf",uiColors[GUI_COLOR_FILE_FONT],ICON_FA_FONT);
  newFilePicker->registerType(".otf",uiColors[GUI_COLOR_FILE_FONT],ICON_FA_FONT);
  newFilePicker->registerType(".ttc",uiColors[GUI_COLOR_FILE_FONT],ICON_FA_FONT);
  newFilePicker->registerType(".dfont",uiColors[GUI_COLOR_FILE_FONT],ICON_FA_FONT);
  newFilePicker->registerType(".fon",uiColors[GUI_COLOR_FILE_FONT],ICON_FA_FONT);
  newFilePicker->registerType(".pcf",uiColors[GUI_COLOR_FILE_FONT],ICON_FA_FONT);
  newFilePicker->registerType(".psf",uiColors[GUI_COLOR_FILE_FONT],ICON_FA_FONT);

  newFilePicker->registerType(".dmf",uiColors[GUI_COLOR_FILE_SONG_IMPORT],ICON_FA_FILE);
  newFilePicker->registerType(".mod",uiColors[GUI_COLOR_FILE_SONG_IMPORT],ICON_FA_FILE);
  newFilePicker->registerType(".s3m",uiColors[GUI_COLOR_FILE_SONG_IMPORT],ICON_FA_FILE);
  newFilePicker->registerType(".xm",uiColors[GUI_COLOR_FILE_SONG_IMPORT],ICON_FA_FILE);
  newFilePicker->registerType(".it",uiColors[GUI_COLOR_FILE_SONG_IMPORT],ICON_FA_FILE);
  newFilePicker->registerType(".fc13",uiColors[GUI_COLOR_FILE_SONG_IMPORT],ICON_FA_FILE);
  newFilePicker->registerType(".fc14",uiColors[GUI_COLOR_FILE_SONG_IMPORT],ICON_FA_FILE);
  newFilePicker->registerType(".fc",uiColors[GUI_COLOR_FILE_SONG_IMPORT],ICON_FA_FILE);
  newFilePicker->registerType(".smod",uiColors[GUI_COLOR_FILE_SONG_IMPORT],ICON_FA_FILE);
  newFilePicker->registerType(".ftm",uiColors[GUI_COLOR_FILE_SONG_IMPORT],ICON_FA_FILE);
  newFilePicker->registerType(".0cc",uiColors[GUI_COLOR_FILE_SONG_IMPORT],ICON_FA_FILE);
  newFilePicker->registerType(".dnm",uiColors[GUI_COLOR_FILE_SONG_IMPORT],ICON_FA_FILE);
  newFilePicker->registerType(".eft",uiColors[GUI_COLOR_FILE_SONG_IMPORT],ICON_FA_FILE);

  newFilePicker->registerType(".fub",uiColors[GUI_COLOR_FILE_SONG_IMPORT],ICON_FA_FILE);

  newFilePicker->registerType(".tfi",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  newFilePicker->registerType(".vgi",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  newFilePicker->registerType(".s3i",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  newFilePicker->registerType(".sbi",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  newFilePicker->registerType(".opli",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  newFilePicker->registerType(".opni",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  newFilePicker->registerType(".y12",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  newFilePicker->registerType(".bnk",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  newFilePicker->registerType(".fti",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  newFilePicker->registerType(".bti",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  newFilePicker->registerType(".ff",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  newFilePicker->registerType(".opm",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);

  if (updateFonts) {
    if (fileDialog!=NULL) delete fileDialog;
#ifdef FLATPAK_WORKAROUNDS
    fileDialog=new FurnaceGUIFileDialog(false,newFilePicker);
#else
    fileDialog=new FurnaceGUIFileDialog(settings.sysFileDialog,newFilePicker);
#endif

    fileDialog->mobileUI=mobileUI;
  }
}
