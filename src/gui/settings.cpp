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

#include "gui.h"
#include "settings.h"
#include "ImGuiFileDialog.h"
#include "../ta-log.h"
#include "../fileutils.h"
#include "misc/freetype/imgui_freetype.h"

void FurnaceGUI::drawSettingsCategory(SettingsCategory* cat) {
  bool filterActive=settings.filter.IsActive();
  if (cat->children.size()>0) {
    ImGuiTreeNodeFlags f=ImGuiTreeNodeFlags_SpanFullWidth|ImGuiTreeNodeFlags_OpenOnArrow|ImGuiTreeNodeFlags_OpenOnDoubleClick;
    if (settings.activeCategory.name==cat->name) f|=ImGuiTreeNodeFlags_Selected;
    ImGui::BeginDisabled(filterActive && !settings.filter.PassFilter(cat->name));
    if (filterActive) f|=ImGuiTreeNodeFlags_DefaultOpen;
    cat->expandChild=ImGui::TreeNodeEx(cat->name,f);
    ImGui::EndDisabled();
    if (ImGui::IsItemClicked()) {
      settings.activeCategory=*cat;
    }
    float indentWidth=ImGui::GetStyle().IndentSpacing;
    if (!cat->expandChild) indentWidth*=2.0f;
    if (cat->expandChild || filterActive) {
      ImGui::Indent(indentWidth);
      for (SettingsCategory child:cat->children) drawSettingsCategory(&child);
      ImGui::Unindent(indentWidth);
      if (cat->expandChild) ImGui::TreePop();
    }
  } else { // a lonely child...
    ImGui::BeginDisabled(filterActive && !settings.filter.PassFilter(cat->name));
    if (ImGui::Selectable(cat->name,settings.activeCategory.id==cat->id)) {
      settings.activeCategory=*cat;
    }
    ImGui::EndDisabled();
  }
}

void FurnaceGUI::searchDrawSettingItems(SettingsCategory* cat) {
  if (cat->children.size()>0) {
    for (SettingsCategory child:cat->children) {
      searchDrawSettingItems(&child);
    }
  }
  bool anyFound=false;
  for (Setting s:cat->settings) {
    if (s.passesFilter(&settings.filter)) {
      anyFound=true;
      break;
    }
  }
  if (anyFound) {
    ImGui::BulletText("%s",cat->name);
    ImGui::Indent();
    for (Setting s:cat->settings) {
      if (s.passesFilter(&settings.filter)) s.drawSetting();
    }
    ImGui::Unindent();
    ImGui::Separator();
  }
}

void FurnaceGUI::drawSettingsItems() {
  if (settings.filter.IsActive() && settings.activeCategory.id==0) {
    for (SettingsCategory cat:settings.categories) searchDrawSettingItems(&cat);
  } else {
    if (settings.activeCategory.id==0) return;
    for (Setting s:settings.activeCategory.settings) s.drawSetting();
  }
}

String FurnaceGUI::stripName(String what) {
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

bool FurnaceGUI::splitBackupName(const char* input, String& backupName, struct tm& backupTime) {
  size_t len=strlen(input);
  if (len<4) return false;

  const char* firstHyphen=NULL;
  const char* secondHyphen=NULL;
  bool whichHyphen=false;
  bool isDateValid=true;
  // -YYYYMMDD-hhmmss.fur
  if (strcmp(&input[len-4],".fur")!=0) return false;
  // find two hyphens
  for (const char* i=input+len; i!=input; i--) {
    if ((*i)=='-') {
      if (whichHyphen) {
        firstHyphen=i;
        break;
      } else {
        secondHyphen=i;
        whichHyphen=true;
      }
    }
  }
  if (firstHyphen==NULL) return false;
  if (secondHyphen==NULL) return false;

  // get the time
  int whichChar=0;
  for (const char* i=secondHyphen+1; *i; i++) {
    if ((*i)<'0' || (*i)>'9') {
      isDateValid=false;
      break;
    }
    switch (whichChar++) {
      case 0:
        backupTime.tm_hour=((*i)-'0')*10;
        break;
      case 1:
        backupTime.tm_hour+=(*i)-'0';
        break;
      case 2:
        backupTime.tm_min=((*i)-'0')*10;
        break;
      case 3:
        backupTime.tm_min+=(*i)-'0';
        break;
      case 4:
        backupTime.tm_sec=((*i)-'0')*10;
        break;
      case 5:
        backupTime.tm_sec+=(*i)-'0';
        break;
    }
    if (whichChar>=6) break;
  }
  if (whichChar!=6) return false;
  if (!isDateValid) return false;
  if (backupTime.tm_hour>23) return false;
  if (backupTime.tm_min>59) return false;
  // intentional
  if (backupTime.tm_sec>60) return false;

  // get the date
  String theDate="";
  for (const char* i=firstHyphen+1; *i; i++) {
    if ((*i)=='-') break;
    if ((*i)<'0' || (*i)>'9') {
      isDateValid=false;
      break;
    }
    theDate+=*i;
  }
  if (!isDateValid) return false;
  if (theDate.size()<5) return false;
  if (theDate.size()>14) return false;
  String mmdd=theDate.substr(theDate.size()-4);
  if (mmdd.size()!=4) return false;
  backupTime.tm_mon=(mmdd[0]-'0')*10+(mmdd[1]-'0')-1;
  backupTime.tm_mday=(mmdd[2]-'0')*10+(mmdd[3]-'0');
  if (backupTime.tm_mon>12) return false;
  if (backupTime.tm_mday>31) return false;
  String yyyy=theDate.substr(0,theDate.size()-4);
  try {
    backupTime.tm_year=std::stoi(yyyy)-1900;
  } catch (std::exception& e) {
    return false;
  }

  backupName="";
  for (const char* i=input; i!=firstHyphen && (*i); i++) {
    backupName+=*i;
  }

  return true;
}

void FurnaceGUI::purgeBackups(int year, int month, int day) {
#ifdef _WIN32
  String findPath=backupPath+String(DIR_SEPARATOR_STR)+String("*.fur");
  WString findPathW=utf8To16(findPath.c_str());
  WIN32_FIND_DATAW next;
  HANDLE backDir=FindFirstFileW(findPathW.c_str(),&next);
  if (backDir!=INVALID_HANDLE_VALUE) {
    do {
      String backupName;
      struct tm backupTime;
      String cFileNameU=utf16To8(next.cFileName);
      bool deleteBackup=false;
      if (!splitBackupName(cFileNameU.c_str(),backupName,backupTime)) continue;

      if (year==0) {
        deleteBackup=true;
      } else if (backupTime.tm_year<(year-1900)) {
        deleteBackup=true;
      } else if (backupTime.tm_year==(year-1900) && backupTime.tm_mon<(month-1)) {
        deleteBackup=true;
      } else if (backupTime.tm_year==(year-1900) && backupTime.tm_mon==(month-1) && backupTime.tm_mday<day) {
        deleteBackup=true;
      }

      if (deleteBackup) {
        String nextPath=backupPath+DIR_SEPARATOR_STR+cFileNameU;
        deleteFile(nextPath.c_str());
      }
    } while (FindNextFileW(backDir,&next)!=0);
    FindClose(backDir);
  }
#else
  DIR* backDir=opendir(backupPath.c_str());
  if (backDir==NULL) {
    logW("could not open backups dir!");
    return;
  }
  while (true) {
    String backupName;
    struct tm backupTime;
    struct dirent* next=readdir(backDir);
    bool deleteBackup=false;
    if (next==NULL) break;
    if (strcmp(next->d_name,".")==0) continue;
    if (strcmp(next->d_name,"..")==0) continue;
    if (!splitBackupName(next->d_name,backupName,backupTime)) continue;

    if (year==0) {
      deleteBackup=true;
    } else if (backupTime.tm_year<(year-1900)) {
      deleteBackup=true;
    } else if (backupTime.tm_year==(year-1900) && backupTime.tm_mon<(month-1)) {
      deleteBackup=true;
    } else if (backupTime.tm_year==(year-1900) && backupTime.tm_mon==(month-1) && backupTime.tm_mday<day) {
      deleteBackup=true;
    }

    if (deleteBackup) {
      String nextPath=backupPath+DIR_SEPARATOR_STR+next->d_name;
      deleteFile(nextPath.c_str());
    }
  }
  closedir(backDir);
#endif
  refreshBackups=true;
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
  if (nextWindow==GUI_WINDOW_SETTINGS) {
    settingsOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
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
  if (ImGui::Begin("Settings",&settingsOpen,ImGuiWindowFlags_NoDocking|globalWinFlags,_("Settings"))) {
    if (!settingsOpen) {
      if (settingsChanged) {
        settingsOpen=true;
        showWarning(_("Do you want to save your settings?"),GUI_WARN_CLOSE_SETTINGS);
      } else {
        settingsOpen=false;
      }
    }

    bool vertical=ImGui::GetWindowSize().y>ImGui::GetWindowSize().x;
    ImVec2 settingsViewSize=ImGui::GetContentRegionAvail();
    settingsViewSize.y-=ImGui::GetFrameHeight()+ImGui::GetStyle().WindowPadding.y;
    if (ImGui::BeginTable("set3", vertical?1:2,ImGuiTableFlags_Resizable|ImGuiTableFlags_BordersOuterH,settingsViewSize)) {
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      if (settings.filter.Draw(_("Search"))) settings.activeCategory=SettingsCategory();
      ImVec2 settingsCatViewSize=ImGui::GetContentRegionAvail();
      settingsCatViewSize.y-=ImGui::GetFrameHeight()+ImGui::GetStyle().WindowPadding.y;
      if (ImGui::BeginChild("SettingCategories",vertical?settingsCatViewSize/ImVec2(1.0f,3.0f):settingsCatViewSize,false)) {
        for (SettingsCategory cat:settings.categories) drawSettingsCategory(&cat);
      }
      ImGui::EndChild();
      if (vertical) ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImVec2 settingsItemsViewSize=ImGui::GetContentRegionAvail();
      settingsItemsViewSize.y-=ImGui::GetFrameHeight()+ImGui::GetStyle().WindowPadding.y;
      if (vertical) ImGui::Separator();
      if (ImGui::BeginChild("SettingsItems",settingsItemsViewSize)) {
        drawSettingsItems();
        if ((strncmp(settings.filter.InputBuf,"cheat",6)==0) && !nonLatchNibble) {
          ImGui::Text("gotta unlock them first!");
        }
      }
      ImGui::EndChild();
      ImGui::EndTable();
    }

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
      settingsOpen=true;
      willCommit=true;
      settingsChanged=false;
    }
    ImGui::EndDisabled();
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_SETTINGS;
  ImGui::End();
}

#define clampSetting(x,minV,maxV) \
  if (x<minV) { \
    x=minV; \
  } \
  if (x>maxV) { \
    x=maxV; \
  }

void FurnaceGUI::readConfig(DivConfig& conf, FurnaceGUISettingGroups groups) {
  if (groups&GUI_SETTINGS_GENERAL) {
    settings.renderDriver=conf.getString("renderDriver","");
    settings.noDMFCompat=conf.getInt("noDMFCompat",0);

    settings.dpiScale=conf.getFloat("dpiScale",0.0f);

    settings.initialSysName=conf.getString("initialSysName","");

    // initial system
    String initialSys2=conf.getString("initialSys2","");
    bool oldVol=conf.getInt("configVersion",DIV_ENGINE_VERSION)<135;
    if (initialSys2.empty()) {
      initialSys2=e->decodeSysDesc(conf.getString("initialSys",""));
      oldVol=false;
    }
    settings.initialSys.clear();
    settings.initialSys.loadFromBase64(initialSys2.c_str());
    if (settings.initialSys.getInt("id0",0)==0) {
      settings.initialSys.clear();
      settings.initialSys.set("id0",e->systemToFileFur(DIV_SYSTEM_YM2612));
      settings.initialSys.set("vol0",1.0f);
      settings.initialSys.set("pan0",0.0f);
      settings.initialSys.set("fr0",0.0f);
      settings.initialSys.set("flags0","");
      settings.initialSys.set("id1",e->systemToFileFur(DIV_SYSTEM_SMS));
      settings.initialSys.set("vol1",0.5f);
      settings.initialSys.set("pan1",0);
      settings.initialSys.set("fr1",0);
      settings.initialSys.set("flags1","");
    } else {
      if (oldVol) {
        for (int i=0; settings.initialSys.getInt(fmt::sprintf("id%d",i),0); i++) {
          float newVol=settings.initialSys.getInt(fmt::sprintf("vol%d",i),64);
          float newPan=settings.initialSys.getInt(fmt::sprintf("pan%d",i),0);
          newVol/=64.0f;
          newPan/=127.0f;
          settings.initialSys.set(fmt::sprintf("vol%d",i),newVol);
          settings.initialSys.set(fmt::sprintf("pan%d",i),newPan);
        }
        conf.set("initialSys2",settings.initialSys.toBase64());
        conf.set("configVersion",DIV_ENGINE_VERSION);
      }
    }

    settings.noThreadedInput=conf.getInt("noThreadedInput",0);
    settings.powerSave=conf.getInt("powerSave",POWER_SAVE_DEFAULT);
    settings.eventDelay=conf.getInt("eventDelay",0);

    settings.renderBackend=conf.getString("renderBackend",GUI_BACKEND_DEFAULT_NAME);
    settings.renderClearPos=conf.getInt("renderClearPos",0);

    settings.glRedSize=conf.getInt("glRedSize",8);
    settings.glGreenSize=conf.getInt("glGreenSize",8);
    settings.glBlueSize=conf.getInt("glBlueSize",8);
    settings.glAlphaSize=conf.getInt("glAlphaSize",0);
    settings.glDepthSize=conf.getInt("glDepthSize",24);
    settings.glStencilSize=conf.getInt("glStencilSize",0);
    settings.glBufferSize=conf.getInt("glBufferSize",32);
    settings.glDoubleBuffer=conf.getInt("glDoubleBuffer",1);

    settings.vsync=conf.getInt("vsync",1);
    settings.frameRateLimit=conf.getInt("frameRateLimit",100);
    settings.displayRenderTime=conf.getInt("displayRenderTime",0);

    settings.chanOscThreads=conf.getInt("chanOscThreads",0);
    settings.renderPoolThreads=conf.getInt("renderPoolThreads",0);
    settings.shaderOsc=conf.getInt("shaderOsc",0);
    settings.writeInsNames=conf.getInt("writeInsNames",0);
    settings.readInsNames=conf.getInt("readInsNames",1);
    settings.defaultAuthorName=conf.getString("defaultAuthorName","");

    settings.hiddenSystems=conf.getInt("hiddenSystems",0);
    settings.allowEditDocking=conf.getInt("allowEditDocking",1);
    settings.sysFileDialog=conf.getInt("sysFileDialog",SYS_FILE_DIALOG_DEFAULT);
    settings.displayAllInsTypes=conf.getInt("displayAllInsTypes",0);
    settings.displayPartial=conf.getInt("displayPartial",0);

    settings.blankIns=conf.getInt("blankIns",0);

    settings.saveWindowPos=conf.getInt("saveWindowPos",1);

    settings.saveUnusedPatterns=conf.getInt("saveUnusedPatterns",0);
    settings.maxRecentFile=conf.getInt("maxRecentFile",10);

    settings.persistFadeOut=conf.getInt("persistFadeOut",1);
    settings.exportLoops=conf.getInt("exportLoops",0);
    settings.exportFadeOut=conf.getDouble("exportFadeOut",0.0);

    settings.doubleClickTime=conf.getFloat("doubleClickTime",0.3f);
    settings.disableFadeIn=conf.getInt("disableFadeIn",0);
    settings.alwaysPlayIntro=conf.getInt("alwaysPlayIntro",0);
    settings.iCannotWait=conf.getInt("iCannotWait",0);

    settings.compress=conf.getInt("compress",1);
    settings.newPatternFormat=conf.getInt("newPatternFormat",1);
    settings.newSongBehavior=conf.getInt("newSongBehavior",0);
    settings.playOnLoad=conf.getInt("playOnLoad",0);
    settings.centerPopup=conf.getInt("centerPopup",1);

    settings.vibrationStrength=conf.getFloat("vibrationStrength",0.5f);
    settings.vibrationLength=conf.getInt("vibrationLength",20);

    settings.s3mOPL3=conf.getInt("s3mOPL3",1);

    settings.backupEnable=conf.getInt("backupEnable",1);
    settings.backupInterval=conf.getInt("backupInterval",30);
    settings.backupMaxCopies=conf.getInt("backupMaxCopies",5);

    settings.autoFillSave=conf.getInt("autoFillSave",0);

    settings.locale=conf.getString("locale","");

    settings.backgroundPlay=conf.getInt("backgroundPlay",0);
  }

  if (groups&GUI_SETTINGS_AUDIO) {
    settings.audioEngine=(conf.getString("audioEngine","SDL")=="SDL")?1:0;
    if (conf.getString("audioEngine","SDL")=="JACK") {
      settings.audioEngine=DIV_AUDIO_JACK;
    } else if (conf.getString("audioEngine","SDL")=="PortAudio") {
      settings.audioEngine=DIV_AUDIO_PORTAUDIO;
    } else {
      settings.audioEngine=DIV_AUDIO_SDL;
    }
    settings.audioDevice=conf.getString("audioDevice","");
    settings.sdlAudioDriver=conf.getString("sdlAudioDriver","");
    settings.audioQuality=conf.getInt("audioQuality",0);
    settings.audioHiPass=conf.getInt("audioHiPass",1);
    settings.audioBufSize=conf.getInt("audioBufSize",1024);
    settings.audioRate=conf.getInt("audioRate",44100);
    settings.audioChans=conf.getInt("audioChans",2);

    settings.lowLatency=conf.getInt("lowLatency",0);

    settings.metroVol=conf.getInt("metroVol",100);
    settings.sampleVol=conf.getInt("sampleVol",50);

    settings.wasapiEx=conf.getInt("wasapiEx",0);

    settings.clampSamples=conf.getInt("clampSamples",0);
    settings.forceMono=conf.getInt("forceMono",0);
  }

  if (groups&GUI_SETTINGS_MIDI) {
    settings.midiInDevice=conf.getString("midiInDevice","");
    settings.midiOutDevice=conf.getString("midiOutDevice","");
    settings.midiOutClock=conf.getInt("midiOutClock",0);
    settings.midiOutTime=conf.getInt("midiOutTime",0);
    settings.midiOutProgramChange=conf.getInt("midiOutProgramChange",0);
    settings.midiOutMode=conf.getInt("midiOutMode",1);
    settings.midiOutTimeRate=conf.getInt("midiOutTimeRate",0);
  }

  if (groups&GUI_SETTINGS_KEYBOARD) {
    // keybinds
    for (int i=0; i<GUI_ACTION_MAX; i++) {
      if (guiActions[i].isNotABind()) continue;

      // use { -1 } as a fallback to let us know there was an issue
      actionKeys[i]=conf.getIntList(String("keybind_GUI_ACTION_")+String(guiActions[i].name),{-1});
      if (actionKeys[i].size()==1 && actionKeys[i][0]==-1) {
        actionKeys[i]=guiActions[i].defaultBind;
      } else {
        for (size_t j=0; j<actionKeys[i].size(); j++) {
          if (actionKeys[i][j]==-1) {
            actionKeys[i].erase(actionKeys[i].begin()+j);
          }
        }
      }
    }

    decodeKeyMap(noteKeys,conf.getString("noteKeys",DEFAULT_NOTE_KEYS));
  }

  if (groups&GUI_SETTINGS_BEHAVIOR) {
    settings.soloAction=conf.getInt("soloAction",0);
    settings.ctrlWheelModifier=conf.getInt("ctrlWheelModifier",0);
    settings.pullDeleteBehavior=conf.getInt("pullDeleteBehavior",1);
    settings.wrapHorizontal=conf.getInt("wrapHorizontal",0);
    settings.wrapVertical=conf.getInt("wrapVertical",0);

    settings.stepOnDelete=conf.getInt("stepOnDelete",0);
    settings.scrollStep=conf.getInt("scrollStep",0);
    settings.avoidRaisingPattern=conf.getInt("avoidRaisingPattern",0);
    settings.insFocusesPattern=conf.getInt("insFocusesPattern",1);
    settings.stepOnInsert=conf.getInt("stepOnInsert",0);
    settings.effectCursorDir=conf.getInt("effectCursorDir",1);
    settings.cursorPastePos=conf.getInt("cursorPastePos",1);

    settings.effectDeletionAltersValue=conf.getInt("effectDeletionAltersValue",1);

    settings.pushNibble=conf.getInt("pushNibble",0);
    settings.scrollChangesOrder=conf.getInt("scrollChangesOrder",0);
    settings.cursorMoveNoScroll=conf.getInt("cursorMoveNoScroll",0);

    settings.notePreviewBehavior=conf.getInt("notePreviewBehavior",1);
    
    settings.absorbInsInput=conf.getInt("absorbInsInput",0);
    
    settings.moveWindowTitle=conf.getInt("moveWindowTitle",1);

    settings.doubleClickColumn=conf.getInt("doubleClickColumn",1);
    settings.dragMovesSelection=conf.getInt("dragMovesSelection",2);

    settings.cursorFollowsOrder=conf.getInt("cursorFollowsOrder",1);

    settings.insertBehavior=conf.getInt("insertBehavior",1);
    settings.pullDeleteRow=conf.getInt("pullDeleteRow",1);
    settings.cursorFollowsWheel=conf.getInt("cursorFollowsWheel",0);
    settings.cursorWheelStep=conf.getInt("cursorWheelStep",0);
    settings.removeInsOff=conf.getInt("removeInsOff",0);
    settings.removeVolOff=conf.getInt("removeVolOff",0);
    settings.insTypeMenu=conf.getInt("insTypeMenu",1);

    settings.selectAssetOnLoad=conf.getInt("selectAssetOnLoad",1);

    settings.inputRepeat=conf.getInt("inputRepeat",1);
  }

  if (groups&GUI_SETTINGS_FONT) {
    settings.mainFontSize=conf.getInt("mainFontSize",GUI_FONT_SIZE_DEFAULT);
    settings.headFontSize=conf.getInt("headFontSize",27);
    settings.patFontSize=conf.getInt("patFontSize",GUI_FONT_SIZE_DEFAULT);
    settings.iconSize=conf.getInt("iconSize",GUI_ICON_SIZE_DEFAULT);

    settings.mainFont=conf.getInt("mainFont",GUI_MAIN_FONT_DEFAULT);
    settings.headFont=conf.getInt("headFont",0);
    settings.patFont=conf.getInt("patFont",GUI_PAT_FONT_DEFAULT);
    settings.mainFontPath=conf.getString("mainFontPath","");
    settings.headFontPath=conf.getString("headFontPath","");
    settings.patFontPath=conf.getString("patFontPath","");

    settings.loadJapanese=conf.getInt("loadJapanese",0);
    settings.loadChinese=conf.getInt("loadChinese",0);
    settings.loadChineseTraditional=conf.getInt("loadChineseTraditional",0);
    settings.loadKorean=conf.getInt("loadKorean",0);
    settings.loadFallback=conf.getInt("loadFallback",1);

    settings.fontBackend=conf.getInt("fontBackend",FONT_BACKEND_DEFAULT);
    settings.fontHinting=conf.getInt("fontHinting",GUI_FONT_HINTING_DEFAULT);
    settings.fontBitmap=conf.getInt("fontBitmap",0);
    settings.fontAutoHint=conf.getInt("fontAutoHint",1);
    settings.fontAntiAlias=conf.getInt("fontAntiAlias",GUI_FONT_ANTIALIAS_DEFAULT);
    settings.fontOversample=conf.getInt("fontOversample",GUI_OVERSAMPLE_DEFAULT);
  }

  if (groups&GUI_SETTINGS_APPEARANCE) {
    settings.oscRoundedCorners=conf.getInt("oscRoundedCorners",GUI_DECORATIONS_DEFAULT);
    settings.oscTakesEntireWindow=conf.getInt("oscTakesEntireWindow",0);
    settings.oscBorder=conf.getInt("oscBorder",1);
    settings.oscEscapesBoundary=conf.getInt("oscEscapesBoundary",0);
    settings.oscMono=conf.getInt("oscMono",1);
    settings.oscAntiAlias=conf.getInt("oscAntiAlias",1);
    settings.oscLineSize=conf.getFloat("oscLineSize",1.0f);

    settings.channelColors=conf.getInt("channelColors",1);
    settings.channelTextColors=conf.getInt("channelTextColors",0);
    settings.channelStyle=conf.getInt("channelStyle",1);
    settings.channelVolStyle=conf.getInt("channelVolStyle",0);
    settings.channelFeedbackStyle=conf.getInt("channelFeedbackStyle",1);
    settings.channelFont=conf.getInt("channelFont",1);
    settings.channelTextCenter=conf.getInt("channelTextCenter",1);

    settings.roundedWindows=conf.getInt("roundedWindows",GUI_DECORATIONS_DEFAULT);
    settings.roundedButtons=conf.getInt("roundedButtons",GUI_DECORATIONS_DEFAULT);
    settings.roundedMenus=conf.getInt("roundedMenus",0);
    settings.roundedTabs=conf.getInt("roundedTabs",GUI_DECORATIONS_DEFAULT);
    settings.roundedScrollbars=conf.getInt("roundedScrollbars",GUI_DECORATIONS_DEFAULT);

    settings.separateFMColors=conf.getInt("separateFMColors",0);
    settings.insEditColorize=conf.getInt("insEditColorize",0);

    settings.chipNames=conf.getInt("chipNames",0);
    settings.overflowHighlight=conf.getInt("overflowHighlight",0);
    settings.flatNotes=conf.getInt("flatNotes",0);
    settings.germanNotation=conf.getInt("germanNotation",0);

    settings.frameBorders=conf.getInt("frameBorders",0);

    settings.noteOffLabel=conf.getString("noteOffLabel","OFF");
    settings.noteRelLabel=conf.getString("noteRelLabel","===");
    settings.macroRelLabel=conf.getString("macroRelLabel","REL");
    settings.emptyLabel=conf.getString("emptyLabel","...");
    settings.emptyLabel2=conf.getString("emptyLabel2","..");

    settings.noteCellSpacing=conf.getInt("noteCellSpacing",0);
    settings.insCellSpacing=conf.getInt("insCellSpacing",0);
    settings.volCellSpacing=conf.getInt("volCellSpacing",0);
    settings.effectCellSpacing=conf.getInt("effectCellSpacing",0);
    settings.effectValCellSpacing=conf.getInt("effectValCellSpacing",0);

    settings.patRowsBase=conf.getInt("patRowsBase",0);
    settings.orderRowsBase=conf.getInt("orderRowsBase",1);
    settings.fmNames=conf.getInt("fmNames",0);
    settings.statusDisplay=conf.getInt("statusDisplay",0);
    settings.playbackTime=conf.getInt("playbackTime",1);
    settings.viewPrevPattern=conf.getInt("viewPrevPattern",1);
    settings.susPosition=conf.getInt("susPosition",0);

    settings.titleBarInfo=conf.getInt("titleBarInfo",1);
    settings.titleBarSys=conf.getInt("titleBarSys",1);

    settings.oplStandardWaveNames=conf.getInt("oplStandardWaveNames",0);

    settings.horizontalDataView=conf.getInt("horizontalDataView",0);
    settings.noMultiSystem=conf.getInt("noMultiSystem",0);
    settings.oldMacroVSlider=conf.getInt("oldMacroVSlider",0);
    settings.unsignedDetune=conf.getInt("unsignedDetune",0);
    settings.centerPattern=conf.getInt("centerPattern",0);
    settings.ordersCursor=conf.getInt("ordersCursor",1);
    settings.oneDigitEffects=conf.getInt("oneDigitEffects",0);
    settings.orderButtonPos=conf.getInt("orderButtonPos",2);
    settings.memUsageUnit=conf.getInt("memUsageUnit",1);
    settings.capitalMenuBar=conf.getInt("capitalMenuBar",0);
    settings.insIconsStyle=conf.getInt("insIconsStyle",1);
    settings.sysSeparators=conf.getInt("sysSeparators",1);

    settings.autoMacroStepSize=conf.getInt("autoMacroStepSize",0);
  }

  if (groups&GUI_SETTINGS_LAYOUTS) {
    settings.fmLayout=conf.getInt("fmLayout",4);
    settings.sampleLayout=conf.getInt("sampleLayout",0);
    settings.waveLayout=conf.getInt("waveLayout",0);
    settings.exportOptionsLayout=conf.getInt("exportOptionsLayout",1);
    settings.unifiedDataView=conf.getInt("unifiedDataView",0);
    settings.macroLayout=conf.getInt("macroLayout",0);
    settings.controlLayout=conf.getInt("controlLayout",3);
    settings.classicChipOptions=conf.getInt("classicChipOptions",0);
  }

  if (groups&GUI_SETTINGS_COLOR) {
    settings.guiColorsBase=conf.getInt("guiColorsBase",0);
    settings.guiColorsShading=conf.getInt("guiColorsShading",0);
    settings.basicColors=conf.getInt("basicColors",1);

    // colors
    for (int i=0; i<GUI_COLOR_MAX; i++) {
      uiColors[i]=ImGui::ColorConvertU32ToFloat4(conf.getInt(guiColors[i].name,guiColors[i].defaultColor));
    }
  }

  if (groups&GUI_SETTINGS_EMULATION) {
    settings.arcadeCore=conf.getInt("arcadeCore",0);
    settings.ym2612Core=conf.getInt("ym2612Core",0);
    settings.snCore=conf.getInt("snCore",0);
    settings.nesCore=conf.getInt("nesCore",0);
    settings.fdsCore=conf.getInt("fdsCore",0);
    settings.c64Core=conf.getInt("c64Core",0);
    settings.pokeyCore=conf.getInt("pokeyCore",1);
    settings.opn1Core=conf.getInt("opn1Core",1);
    settings.opnaCore=conf.getInt("opnaCore",1);
    settings.opnbCore=conf.getInt("opnbCore",1);
    settings.opl2Core=conf.getInt("opl2Core",0);
    settings.opl3Core=conf.getInt("opl3Core",0);
    settings.opl4Core=conf.getInt("opl4Core",0);
    settings.esfmCore=conf.getInt("esfmCore",0);
    settings.opllCore=conf.getInt("opllCore",0);
    settings.ayCore=conf.getInt("ayCore",0);

    settings.bubsysQuality=conf.getInt("bubsysQuality",3);
    settings.dsidQuality=conf.getInt("dsidQuality",3);
    settings.gbQuality=conf.getInt("gbQuality",3);
    settings.ndsQuality=conf.getInt("ndsQuality",3);
    settings.pceQuality=conf.getInt("pceQuality",3);
    settings.pnQuality=conf.getInt("pnQuality",3);
    settings.saaQuality=conf.getInt("saaQuality",3);
    settings.sccQuality=conf.getInt("sccQuality",3);
    settings.smQuality=conf.getInt("smQuality",3);
    settings.swanQuality=conf.getInt("swanQuality",3);
    settings.vbQuality=conf.getInt("vbQuality",3);

    settings.arcadeCoreRender=conf.getInt("arcadeCoreRender",1);
    settings.ym2612CoreRender=conf.getInt("ym2612CoreRender",0);
    settings.snCoreRender=conf.getInt("snCoreRender",0);
    settings.nesCoreRender=conf.getInt("nesCoreRender",0);
    settings.fdsCoreRender=conf.getInt("fdsCoreRender",1);
    settings.c64CoreRender=conf.getInt("c64CoreRender",1);
    settings.pokeyCoreRender=conf.getInt("pokeyCoreRender",1);
    settings.opn1CoreRender=conf.getInt("opn1CoreRender",1);
    settings.opnaCoreRender=conf.getInt("opnaCoreRender",1);
    settings.opnbCoreRender=conf.getInt("opnbCoreRender",1);
    settings.opl2CoreRender=conf.getInt("opl2CoreRender",0);
    settings.opl3CoreRender=conf.getInt("opl3CoreRender",0);
    settings.opl4CoreRender=conf.getInt("opl4CoreRender",0);
    settings.esfmCoreRender=conf.getInt("esfmCoreRender",0);
    settings.opllCoreRender=conf.getInt("opllCoreRender",0);
    settings.ayCoreRender=conf.getInt("ayCoreRender",0);

    settings.bubsysQualityRender=conf.getInt("bubsysQualityRender",3);
    settings.dsidQualityRender=conf.getInt("dsidQualityRender",3);
    settings.gbQualityRender=conf.getInt("gbQualityRender",3);
    settings.ndsQualityRender=conf.getInt("ndsQualityRender",3);
    settings.pceQualityRender=conf.getInt("pceQualityRender",3);
    settings.pnQualityRender=conf.getInt("pnQualityRender",3);
    settings.saaQualityRender=conf.getInt("saaQualityRender",3);
    settings.sccQualityRender=conf.getInt("sccQualityRender",3);
    settings.smQualityRender=conf.getInt("smQualityRender",3);
    settings.swanQualityRender=conf.getInt("swanQualityRender",3);
    settings.vbQualityRender=conf.getInt("vbQualityRender",3);

    settings.pcSpeakerOutMethod=conf.getInt("pcSpeakerOutMethod",0);

    settings.yrw801Path=conf.getString("yrw801Path","");
    settings.tg100Path=conf.getString("tg100Path","");
    settings.mu5Path=conf.getString("mu5Path","");
  }

  clampSetting(settings.mainFontSize,2,96);
  clampSetting(settings.headFontSize,2,96);
  clampSetting(settings.patFontSize,2,96);
  clampSetting(settings.iconSize,2,48);
  clampSetting(settings.audioEngine,0,2);
  clampSetting(settings.audioQuality,0,1);
  clampSetting(settings.audioHiPass,0,1);
  clampSetting(settings.audioBufSize,32,4096);
  clampSetting(settings.audioRate,8000,384000);
  clampSetting(settings.audioChans,1,16);
  clampSetting(settings.arcadeCore,0,1);
  clampSetting(settings.ym2612Core,0,2);
  clampSetting(settings.snCore,0,1);
  clampSetting(settings.nesCore,0,1);
  clampSetting(settings.fdsCore,0,1);
  clampSetting(settings.c64Core,0,2);
  clampSetting(settings.pokeyCore,0,1);
  clampSetting(settings.opn1Core,0,2);
  clampSetting(settings.opnaCore,0,2);
  clampSetting(settings.opnbCore,0,2);
  clampSetting(settings.opl2Core,0,2);
  clampSetting(settings.opl3Core,0,2);
  clampSetting(settings.opl4Core,0,1);
  clampSetting(settings.esfmCore,0,1);
  clampSetting(settings.opllCore,0,1);
  clampSetting(settings.ayCore,0,1);
  clampSetting(settings.bubsysQuality,0,5);
  clampSetting(settings.dsidQuality,0,5);
  clampSetting(settings.gbQuality,0,5);
  clampSetting(settings.ndsQuality,0,5);
  clampSetting(settings.pceQuality,0,5);
  clampSetting(settings.pnQuality,0,5);
  clampSetting(settings.saaQuality,0,5);
  clampSetting(settings.sccQuality,0,5);
  clampSetting(settings.smQuality,0,5);
  clampSetting(settings.swanQuality,0,5);
  clampSetting(settings.vbQuality,0,5);
  clampSetting(settings.arcadeCoreRender,0,1);
  clampSetting(settings.ym2612CoreRender,0,2);
  clampSetting(settings.snCoreRender,0,1);
  clampSetting(settings.nesCoreRender,0,1);
  clampSetting(settings.fdsCoreRender,0,1);
  clampSetting(settings.c64CoreRender,0,2);
  clampSetting(settings.pokeyCoreRender,0,1);
  clampSetting(settings.opn1CoreRender,0,2);
  clampSetting(settings.opnaCoreRender,0,2);
  clampSetting(settings.opnbCoreRender,0,2);
  clampSetting(settings.opl2CoreRender,0,2);
  clampSetting(settings.opl3CoreRender,0,2);
  clampSetting(settings.opl4CoreRender,0,1);
  clampSetting(settings.esfmCoreRender,0,1);
  clampSetting(settings.opllCoreRender,0,1);
  clampSetting(settings.ayCoreRender,0,1);
  clampSetting(settings.bubsysQualityRender,0,5);
  clampSetting(settings.dsidQualityRender,0,5);
  clampSetting(settings.gbQualityRender,0,5);
  clampSetting(settings.ndsQualityRender,0,5);
  clampSetting(settings.pceQualityRender,0,5);
  clampSetting(settings.pnQualityRender,0,5);
  clampSetting(settings.saaQualityRender,0,5);
  clampSetting(settings.sccQualityRender,0,5);
  clampSetting(settings.smQualityRender,0,5);
  clampSetting(settings.swanQualityRender,0,5);
  clampSetting(settings.vbQualityRender,0,5);
  clampSetting(settings.pcSpeakerOutMethod,0,4);
  clampSetting(settings.mainFont,0,6);
  clampSetting(settings.patFont,0,6);
  clampSetting(settings.patRowsBase,0,1);
  clampSetting(settings.orderRowsBase,0,1);
  clampSetting(settings.soloAction,0,2);
  clampSetting(settings.ctrlWheelModifier,0,3);
  clampSetting(settings.pullDeleteBehavior,0,1);
  clampSetting(settings.wrapHorizontal,0,2);
  clampSetting(settings.wrapVertical,0,3);
  clampSetting(settings.fmNames,0,2);
  clampSetting(settings.allowEditDocking,0,1);
  clampSetting(settings.chipNames,0,1);
  clampSetting(settings.overflowHighlight,0,1);
  clampSetting(settings.flatNotes,0,1);
  clampSetting(settings.germanNotation,0,1);
  clampSetting(settings.stepOnDelete,0,1);
  clampSetting(settings.scrollStep,0,1);
  clampSetting(settings.sysSeparators,0,1);
  clampSetting(settings.forceMono,0,1);
  clampSetting(settings.controlLayout,0,3);
  clampSetting(settings.statusDisplay,0,3);
  clampSetting(settings.dpiScale,0.0f,4.0f);
  clampSetting(settings.viewPrevPattern,0,1);
  clampSetting(settings.guiColorsBase,0,1);
  clampSetting(settings.guiColorsShading,0,100);
  clampSetting(settings.avoidRaisingPattern,0,1);
  clampSetting(settings.insFocusesPattern,0,1);
  clampSetting(settings.stepOnInsert,0,1);
  clampSetting(settings.unifiedDataView,0,1);
  clampSetting(settings.sysFileDialog,0,1);
  clampSetting(settings.roundedWindows,0,1);
  clampSetting(settings.roundedButtons,0,1);
  clampSetting(settings.roundedMenus,0,1);
  clampSetting(settings.roundedTabs,0,1);
  clampSetting(settings.roundedScrollbars,0,1);
  clampSetting(settings.loadJapanese,0,1);
  clampSetting(settings.loadChinese,0,1);
  clampSetting(settings.loadChineseTraditional,0,1);
  clampSetting(settings.loadKorean,0,1);
  clampSetting(settings.loadFallback,0,1);
  clampSetting(settings.fmLayout,0,6);
  clampSetting(settings.susPosition,0,3);
  clampSetting(settings.effectCursorDir,0,2);
  clampSetting(settings.cursorPastePos,0,1);
  clampSetting(settings.titleBarInfo,0,3);
  clampSetting(settings.titleBarSys,0,1);
  clampSetting(settings.frameBorders,0,1);
  clampSetting(settings.effectDeletionAltersValue,0,1);
  clampSetting(settings.sampleLayout,0,1);
  clampSetting(settings.waveLayout,0,1);
  clampSetting(settings.separateFMColors,0,1);
  clampSetting(settings.insEditColorize,0,1);
  clampSetting(settings.metroVol,0,200);
  clampSetting(settings.sampleVol,0,100);
  clampSetting(settings.pushNibble,0,1);
  clampSetting(settings.scrollChangesOrder,0,2);
  clampSetting(settings.oplStandardWaveNames,0,1);
  clampSetting(settings.cursorMoveNoScroll,0,1);
  clampSetting(settings.lowLatency,0,1);
  clampSetting(settings.notePreviewBehavior,0,3);
  clampSetting(settings.powerSave,0,1);
  clampSetting(settings.absorbInsInput,0,1);
  clampSetting(settings.eventDelay,0,1);
  clampSetting(settings.moveWindowTitle,0,1);
  clampSetting(settings.hiddenSystems,0,1);
  clampSetting(settings.horizontalDataView,0,1);
  clampSetting(settings.noMultiSystem,0,1);
  clampSetting(settings.oldMacroVSlider,0,1);
  clampSetting(settings.displayAllInsTypes,0,1);
  clampSetting(settings.displayPartial,0,1);
  clampSetting(settings.noteCellSpacing,0,32);
  clampSetting(settings.insCellSpacing,0,32);
  clampSetting(settings.volCellSpacing,0,32);
  clampSetting(settings.effectCellSpacing,0,32);
  clampSetting(settings.effectValCellSpacing,0,32);
  clampSetting(settings.doubleClickColumn,0,1);
  clampSetting(settings.blankIns,0,1);
  clampSetting(settings.dragMovesSelection,0,2);
  clampSetting(settings.unsignedDetune,0,1);
  clampSetting(settings.noThreadedInput,0,1);
  clampSetting(settings.saveWindowPos,0,1);
  clampSetting(settings.clampSamples,0,1);
  clampSetting(settings.saveUnusedPatterns,0,1);
  clampSetting(settings.channelColors,0,2);
  clampSetting(settings.channelTextColors,0,2);
  clampSetting(settings.channelStyle,0,5);
  clampSetting(settings.channelVolStyle,0,4);
  clampSetting(settings.channelFeedbackStyle,0,3);
  clampSetting(settings.channelFont,0,1);
  clampSetting(settings.channelTextCenter,0,1);
  clampSetting(settings.maxRecentFile,0,30);
  clampSetting(settings.midiOutClock,0,1);
  clampSetting(settings.midiOutTime,0,1);
  clampSetting(settings.midiOutProgramChange,0,1);
  clampSetting(settings.midiOutMode,0,2);
  clampSetting(settings.midiOutTimeRate,0,4);
  clampSetting(settings.centerPattern,0,1);
  clampSetting(settings.ordersCursor,0,1);
  clampSetting(settings.persistFadeOut,0,1);
  clampSetting(settings.macroLayout,0,4);
  clampSetting(settings.doubleClickTime,0.02,1.0);
  clampSetting(settings.oneDigitEffects,0,1);
  clampSetting(settings.disableFadeIn,0,1);
  clampSetting(settings.alwaysPlayIntro,0,3);
  clampSetting(settings.cursorFollowsOrder,0,1);
  clampSetting(settings.iCannotWait,0,1);
  clampSetting(settings.orderButtonPos,0,2);
  clampSetting(settings.compress,0,1);
  clampSetting(settings.newPatternFormat,0,1);
  clampSetting(settings.renderClearPos,0,1);
  clampSetting(settings.insertBehavior,0,1);
  clampSetting(settings.pullDeleteRow,0,1);
  clampSetting(settings.newSongBehavior,0,1);
  clampSetting(settings.memUsageUnit,0,1);
  clampSetting(settings.cursorFollowsWheel,0,2);
  clampSetting(settings.noDMFCompat,0,1);
  clampSetting(settings.removeInsOff,0,1);
  clampSetting(settings.removeVolOff,0,1);
  clampSetting(settings.playOnLoad,0,2);
  clampSetting(settings.insTypeMenu,0,1);
  clampSetting(settings.capitalMenuBar,0,1);
  clampSetting(settings.centerPopup,0,1);
  clampSetting(settings.insIconsStyle,0,2);
  clampSetting(settings.classicChipOptions,0,1);
  clampSetting(settings.exportOptionsLayout,0,2);
  clampSetting(settings.wasapiEx,0,1);
  clampSetting(settings.chanOscThreads,0,256);
  clampSetting(settings.renderPoolThreads,0,DIV_MAX_CHIPS);
  clampSetting(settings.writeInsNames,0,1);
  clampSetting(settings.readInsNames,0,1);
  clampSetting(settings.fontBackend,0,1);
  clampSetting(settings.fontHinting,0,3);
  clampSetting(settings.fontBitmap,0,1);
  clampSetting(settings.fontAutoHint,0,2);
  clampSetting(settings.fontAntiAlias,0,1);
  clampSetting(settings.fontOversample,1,3);
  clampSetting(settings.selectAssetOnLoad,0,1);
  clampSetting(settings.basicColors,0,1);
  clampSetting(settings.playbackTime,0,1);
  clampSetting(settings.shaderOsc,0,1);
  clampSetting(settings.oscLineSize,0.25f,16.0f);
  clampSetting(settings.cursorWheelStep,0,1);
  clampSetting(settings.vsync,0,4);
  clampSetting(settings.frameRateLimit,0,1000);
  clampSetting(settings.displayRenderTime,0,1);
  clampSetting(settings.vibrationStrength,0.0f,1.0f);
  clampSetting(settings.vibrationLength,10,500);
  clampSetting(settings.inputRepeat,0,1);
  clampSetting(settings.glRedSize,0,32);
  clampSetting(settings.glGreenSize,0,32);
  clampSetting(settings.glBlueSize,0,32);
  clampSetting(settings.glAlphaSize,0,32);
  clampSetting(settings.glDepthSize,0,128);
  clampSetting(settings.glStencilSize,0,32);
  clampSetting(settings.glDoubleBuffer,0,1);
  clampSetting(settings.backupEnable,0,1);
  clampSetting(settings.backupInterval,10,86400);
  clampSetting(settings.backupMaxCopies,1,100);
  clampSetting(settings.autoFillSave,0,1);
  clampSetting(settings.autoMacroStepSize,0,2);
  clampSetting(settings.s3mOPL3,0,1);
  clampSetting(settings.backgroundPlay,0,1);

  if (settings.exportLoops<0.0) settings.exportLoops=0.0;
  if (settings.exportFadeOut<0.0) settings.exportFadeOut=0.0;  
}

void FurnaceGUI::writeConfig(DivConfig& conf, FurnaceGUISettingGroups groups) {
  // general
  if (groups&GUI_SETTINGS_GENERAL) {
    conf.set("renderDriver",settings.renderDriver);
    conf.set("noDMFCompat",settings.noDMFCompat);

    conf.set("dpiScale",settings.dpiScale);

    conf.set("initialSys2",settings.initialSys.toBase64());
    conf.set("initialSysName",settings.initialSysName);

    conf.set("noThreadedInput",settings.noThreadedInput);
    conf.set("powerSave",settings.powerSave);
    conf.set("eventDelay",settings.eventDelay);

    conf.set("renderBackend",settings.renderBackend);
    conf.set("renderClearPos",settings.renderClearPos);

    conf.set("glRedSize",settings.glRedSize);
    conf.set("glGreenSize",settings.glGreenSize);
    conf.set("glBlueSize",settings.glBlueSize);
    conf.set("glAlphaSize",settings.glAlphaSize);
    conf.set("glDepthSize",settings.glDepthSize);
    conf.set("glStencilSize",settings.glStencilSize);
    conf.set("glDoubleBuffer",settings.glDoubleBuffer);

    conf.set("vsync",settings.vsync);
    conf.set("frameRateLimit",settings.frameRateLimit);
    conf.set("displayRenderTime",settings.displayRenderTime);

    conf.set("chanOscThreads",settings.chanOscThreads);
    conf.set("renderPoolThreads",settings.renderPoolThreads);
    conf.set("shaderOsc",settings.shaderOsc);
    conf.set("writeInsNames",settings.writeInsNames);
    conf.set("readInsNames",settings.readInsNames);
    conf.set("defaultAuthorName",settings.defaultAuthorName);

    conf.set("hiddenSystems",settings.hiddenSystems);
    conf.set("allowEditDocking",settings.allowEditDocking);
    conf.set("sysFileDialog",settings.sysFileDialog);
    conf.set("displayAllInsTypes",settings.displayAllInsTypes);
    conf.set("displayPartial",settings.displayPartial);

    conf.set("blankIns",settings.blankIns);

    conf.set("saveWindowPos",settings.saveWindowPos);
    
    conf.set("saveUnusedPatterns",settings.saveUnusedPatterns);
    conf.set("maxRecentFile",settings.maxRecentFile);
    
    conf.set("persistFadeOut",settings.persistFadeOut);
    conf.set("exportLoops",settings.exportLoops);
    conf.set("exportFadeOut",settings.exportFadeOut);

    conf.set("doubleClickTime",settings.doubleClickTime);
    conf.set("disableFadeIn",settings.disableFadeIn);
    conf.set("alwaysPlayIntro",settings.alwaysPlayIntro);
    conf.set("iCannotWait",settings.iCannotWait);

    conf.set("compress",settings.compress);
    conf.set("newPatternFormat",settings.newPatternFormat);
    conf.set("newSongBehavior",settings.newSongBehavior);
    conf.set("playOnLoad",settings.playOnLoad);
    conf.set("centerPopup",settings.centerPopup);

    conf.set("vibrationStrength",settings.vibrationStrength);
    conf.set("vibrationLength",settings.vibrationLength);

    conf.set("s3mOPL3",settings.s3mOPL3);

    conf.set("backupEnable",settings.backupEnable);
    conf.set("backupInterval",settings.backupInterval);
    conf.set("backupMaxCopies",settings.backupMaxCopies);

    conf.set("autoFillSave",settings.autoFillSave);

    conf.set("locale",settings.locale);

    conf.set("backgroundPlay",settings.backgroundPlay);
  }

  // audio
  if (groups&GUI_SETTINGS_AUDIO) {
    conf.set("audioEngine",String(audioBackends[settings.audioEngine]));
    conf.set("audioDevice",settings.audioDevice);
    conf.set("sdlAudioDriver",settings.sdlAudioDriver);
    conf.set("audioQuality",settings.audioQuality);
    conf.set("audioHiPass",settings.audioHiPass);
    conf.set("audioBufSize",settings.audioBufSize);
    conf.set("audioRate",settings.audioRate);
    conf.set("audioChans",settings.audioChans);

    conf.set("lowLatency",settings.lowLatency);

    conf.set("metroVol",settings.metroVol);
    conf.set("sampleVol",settings.sampleVol);

    conf.set("wasapiEx",settings.wasapiEx);

    conf.set("clampSamples",settings.clampSamples);
    conf.set("forceMono",settings.forceMono);
  }

  // MIDI
  if (groups&GUI_SETTINGS_MIDI) {
    conf.set("midiInDevice",settings.midiInDevice);
    conf.set("midiOutDevice",settings.midiOutDevice);
    conf.set("midiOutClock",settings.midiOutClock);
    conf.set("midiOutTime",settings.midiOutTime);
    conf.set("midiOutProgramChange",settings.midiOutProgramChange);
    conf.set("midiOutMode",settings.midiOutMode);
    conf.set("midiOutTimeRate",settings.midiOutTimeRate);
  }

  // keyboard
  if (groups&GUI_SETTINGS_KEYBOARD) {
    // keybinds
    for (int i=0; i<GUI_ACTION_MAX; i++) {
      if (guiActions[i].isNotABind()) continue;
      conf.set(String("keybind_GUI_ACTION_")+String(guiActions[i].name),actionKeys[i]);
    }

    conf.set("noteKeys",encodeKeyMap(noteKeys));
  }

  // behavior
  if (groups&GUI_SETTINGS_BEHAVIOR) {
    conf.set("soloAction",settings.soloAction);
    conf.set("ctrlWheelModifier",settings.ctrlWheelModifier);
    conf.set("pullDeleteBehavior",settings.pullDeleteBehavior);
    conf.set("wrapHorizontal",settings.wrapHorizontal);
    conf.set("wrapVertical",settings.wrapVertical);
    
    conf.set("stepOnDelete",settings.stepOnDelete);
    conf.set("scrollStep",settings.scrollStep);
    conf.set("avoidRaisingPattern",settings.avoidRaisingPattern);
    conf.set("insFocusesPattern",settings.insFocusesPattern);
    conf.set("stepOnInsert",settings.stepOnInsert);
    conf.set("effectCursorDir",settings.effectCursorDir);
    conf.set("cursorPastePos",settings.cursorPastePos);
    
    conf.set("effectDeletionAltersValue",settings.effectDeletionAltersValue);

    conf.set("pushNibble",settings.pushNibble);
    conf.set("scrollChangesOrder",settings.scrollChangesOrder);
    conf.set("cursorMoveNoScroll",settings.cursorMoveNoScroll);

    conf.set("notePreviewBehavior",settings.notePreviewBehavior);
    
    conf.set("absorbInsInput",settings.absorbInsInput);
    
    conf.set("moveWindowTitle",settings.moveWindowTitle);
    
    conf.set("doubleClickColumn",settings.doubleClickColumn);
    conf.set("dragMovesSelection",settings.dragMovesSelection);
    
    conf.set("cursorFollowsOrder",settings.cursorFollowsOrder);
    
    conf.set("insertBehavior",settings.insertBehavior);
    conf.set("pullDeleteRow",settings.pullDeleteRow);
    conf.set("cursorFollowsWheel",settings.cursorFollowsWheel);
    conf.set("cursorWheelStep",settings.cursorWheelStep);
    conf.set("removeInsOff",settings.removeInsOff);
    conf.set("removeVolOff",settings.removeVolOff);
    conf.set("insTypeMenu",settings.insTypeMenu);
    
    conf.set("selectAssetOnLoad",settings.selectAssetOnLoad);

    conf.set("inputRepeat",settings.inputRepeat);
  }

  // font
  if (groups&GUI_SETTINGS_FONT) {
    conf.set("mainFontSize",settings.mainFontSize);
    conf.set("headFontSize",settings.headFontSize);
    conf.set("patFontSize",settings.patFontSize);
    conf.set("iconSize",settings.iconSize);

    conf.set("mainFont",settings.mainFont);
    conf.set("headFont",settings.headFont);
    conf.set("patFont",settings.patFont);
    conf.set("mainFontPath",settings.mainFontPath);
    conf.set("headFontPath",settings.headFontPath);
    conf.set("patFontPath",settings.patFontPath);

    conf.set("loadJapanese",settings.loadJapanese);
    conf.set("loadChinese",settings.loadChinese);
    conf.set("loadChineseTraditional",settings.loadChineseTraditional);
    conf.set("loadKorean",settings.loadKorean);
    conf.set("loadFallback",settings.loadFallback);

    conf.set("fontBackend",settings.fontBackend);
    conf.set("fontHinting",settings.fontHinting);
    conf.set("fontBitmap",settings.fontBitmap);
    conf.set("fontAutoHint",settings.fontAutoHint);
    conf.set("fontAntiAlias",settings.fontAntiAlias);
    conf.set("fontOversample",settings.fontOversample);
  }

  // appearance
  if (groups&GUI_SETTINGS_APPEARANCE) {
    conf.set("oscRoundedCorners",settings.oscRoundedCorners);
    conf.set("oscTakesEntireWindow",settings.oscTakesEntireWindow);
    conf.set("oscBorder",settings.oscBorder);
    conf.set("oscEscapesBoundary",settings.oscEscapesBoundary);
    conf.set("oscMono",settings.oscMono);
    conf.set("oscAntiAlias",settings.oscAntiAlias);
    conf.set("oscLineSize",settings.oscLineSize);

    conf.set("channelColors",settings.channelColors);
    conf.set("channelTextColors",settings.channelTextColors);
    conf.set("channelStyle",settings.channelStyle);
    conf.set("channelVolStyle",settings.channelVolStyle);
    conf.set("channelFeedbackStyle",settings.channelFeedbackStyle);
    conf.set("channelFont",settings.channelFont);
    conf.set("channelTextCenter",settings.channelTextCenter);

    conf.set("roundedWindows",settings.roundedWindows);
    conf.set("roundedButtons",settings.roundedButtons);
    conf.set("roundedMenus",settings.roundedMenus);
    conf.set("roundedTabs",settings.roundedTabs);
    conf.set("roundedScrollbars",settings.roundedScrollbars);

    conf.set("separateFMColors",settings.separateFMColors);
    conf.set("insEditColorize",settings.insEditColorize);

    conf.set("chipNames",settings.chipNames);
    conf.set("overflowHighlight",settings.overflowHighlight);
    conf.set("flatNotes",settings.flatNotes);
    conf.set("germanNotation",settings.germanNotation);

    conf.set("frameBorders",settings.frameBorders);

    conf.set("noteOffLabel",settings.noteOffLabel);
    conf.set("noteRelLabel",settings.noteRelLabel);
    conf.set("macroRelLabel",settings.macroRelLabel);
    conf.set("emptyLabel",settings.emptyLabel);
    conf.set("emptyLabel2",settings.emptyLabel2);

    conf.set("noteCellSpacing",settings.noteCellSpacing);
    conf.set("insCellSpacing",settings.insCellSpacing);
    conf.set("volCellSpacing",settings.volCellSpacing);
    conf.set("effectCellSpacing",settings.effectCellSpacing);
    conf.set("effectValCellSpacing",settings.effectValCellSpacing);

    conf.set("patRowsBase",settings.patRowsBase);
    conf.set("orderRowsBase",settings.orderRowsBase);
    conf.set("fmNames",settings.fmNames);
    conf.set("statusDisplay",settings.statusDisplay);
    conf.set("playbackTime",settings.playbackTime);
    conf.set("viewPrevPattern",settings.viewPrevPattern);
    conf.set("susPosition",settings.susPosition);

    conf.set("titleBarInfo",settings.titleBarInfo);
    conf.set("titleBarSys",settings.titleBarSys);

    conf.set("oplStandardWaveNames",settings.oplStandardWaveNames);

    conf.set("horizontalDataView",settings.horizontalDataView);
    conf.set("noMultiSystem",settings.noMultiSystem);
    conf.set("oldMacroVSlider",settings.oldMacroVSlider);
    conf.set("unsignedDetune",settings.unsignedDetune);
    conf.set("centerPattern",settings.centerPattern);
    conf.set("ordersCursor",settings.ordersCursor);
    conf.set("oneDigitEffects",settings.oneDigitEffects);
    conf.set("orderButtonPos",settings.orderButtonPos);
    conf.set("memUsageUnit",settings.memUsageUnit);
    conf.set("capitalMenuBar",settings.capitalMenuBar);
    conf.set("insIconsStyle",settings.insIconsStyle);
    conf.set("sysSeparators",settings.sysSeparators);
    conf.set("autoMacroStepSize",settings.autoMacroStepSize);
  }

  // layout
  if (groups&GUI_SETTINGS_LAYOUTS) {
    conf.set("fmLayout",settings.fmLayout);
    conf.set("sampleLayout",settings.sampleLayout);
    conf.set("waveLayout",settings.waveLayout);
    conf.set("exportOptionsLayout",settings.exportOptionsLayout);
    conf.set("unifiedDataView",settings.unifiedDataView);
    conf.set("macroLayout",settings.macroLayout);
    conf.set("controlLayout",settings.controlLayout);
    conf.set("classicChipOptions",settings.classicChipOptions);
  }

  // color
  if (groups&GUI_SETTINGS_COLOR) {
    conf.set("guiColorsBase",settings.guiColorsBase);
    conf.set("guiColorsShading",settings.guiColorsShading);
    conf.set("basicColors",settings.basicColors);

    // colors
    for (int i=0; i<GUI_COLOR_MAX; i++) {
      conf.set(guiColors[i].name,(int)ImGui::ColorConvertFloat4ToU32(uiColors[i]));
    }
  }

  // emulation
  if (groups&GUI_SETTINGS_EMULATION) {
    conf.set("arcadeCore",settings.arcadeCore);
    conf.set("ym2612Core",settings.ym2612Core);
    conf.set("snCore",settings.snCore);
    conf.set("nesCore",settings.nesCore);
    conf.set("fdsCore",settings.fdsCore);
    conf.set("c64Core",settings.c64Core);
    conf.set("pokeyCore",settings.pokeyCore);
    conf.set("opn1Core",settings.opn1Core);
    conf.set("opnaCore",settings.opnaCore);
    conf.set("opnbCore",settings.opnbCore);
    conf.set("opl2Core",settings.opl2Core);
    conf.set("opl3Core",settings.opl3Core);
    conf.set("opl4Core",settings.opl4Core);
    conf.set("esfmCore",settings.esfmCore);
    conf.set("opllCore",settings.opllCore);
    conf.set("ayCore",settings.ayCore);

    conf.set("bubsysQuality",settings.bubsysQuality);
    conf.set("dsidQuality",settings.dsidQuality);
    conf.set("gbQuality",settings.gbQuality);
    conf.set("ndsQuality",settings.ndsQuality);
    conf.set("pceQuality",settings.pceQuality);
    conf.set("pnQuality",settings.pnQuality);
    conf.set("saaQuality",settings.saaQuality);
    conf.set("sccQuality",settings.sccQuality);
    conf.set("smQuality",settings.smQuality);
    conf.set("swanQuality",settings.swanQuality);
    conf.set("vbQuality",settings.vbQuality);

    conf.set("arcadeCoreRender",settings.arcadeCoreRender);
    conf.set("ym2612CoreRender",settings.ym2612CoreRender);
    conf.set("snCoreRender",settings.snCoreRender);
    conf.set("nesCoreRender",settings.nesCoreRender);
    conf.set("fdsCoreRender",settings.fdsCoreRender);
    conf.set("c64CoreRender",settings.c64CoreRender);
    conf.set("pokeyCoreRender",settings.pokeyCoreRender);
    conf.set("opn1CoreRender",settings.opn1CoreRender);
    conf.set("opnaCoreRender",settings.opnaCoreRender);
    conf.set("opnbCoreRender",settings.opnbCoreRender);
    conf.set("opl2CoreRender",settings.opl2CoreRender);
    conf.set("opl3CoreRender",settings.opl3CoreRender);
    conf.set("opl4CoreRender",settings.opl4CoreRender);
    conf.set("esfmCoreRender",settings.esfmCoreRender);
    conf.set("opllCoreRender",settings.opllCoreRender);
    conf.set("ayCoreRender",settings.ayCoreRender);

    conf.set("bubsysQualityRender",settings.bubsysQualityRender);
    conf.set("dsidQualityRender",settings.dsidQualityRender);
    conf.set("gbQualityRender",settings.gbQualityRender);
    conf.set("ndsQualityRender",settings.ndsQualityRender);
    conf.set("pceQualityRender",settings.pceQualityRender);
    conf.set("pnQualityRender",settings.pnQualityRender);
    conf.set("saaQualityRender",settings.saaQualityRender);
    conf.set("sccQualityRender",settings.sccQualityRender);
    conf.set("smQualityRender",settings.smQualityRender);
    conf.set("swanQualityRender",settings.swanQualityRender);
    conf.set("vbQualityRender",settings.vbQualityRender);

    conf.set("pcSpeakerOutMethod",settings.pcSpeakerOutMethod);

    conf.set("yrw801Path",settings.yrw801Path);
    conf.set("tg100Path",settings.tg100Path);
    conf.set("mu5Path",settings.mu5Path);
  }
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
    settings.bubsysQuality!=e->getConfInt("bubsysQuality",3) ||
    settings.dsidQuality!=e->getConfInt("dsidQuality",3) ||
    settings.gbQuality!=e->getConfInt("gbQuality",3) ||
    settings.ndsQuality!=e->getConfInt("ndsQuality",3) ||
    settings.pceQuality!=e->getConfInt("pceQuality",3) ||
    settings.pnQuality!=e->getConfInt("pnQuality",3) ||
    settings.saaQuality!=e->getConfInt("saaQuality",3) ||
    settings.sccQuality!=e->getConfInt("sccQuality",3) ||
    settings.smQuality!=e->getConfInt("smQuality",3) ||
    settings.swanQuality!=e->getConfInt("swanQuality",3) ||
    settings.vbQuality!=e->getConfInt("vbQuality",3) ||
    settings.audioQuality!=e->getConfInt("audioQuality",0) ||
    settings.audioHiPass!=e->getConfInt("audioHiPass",1)
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

  if (rend) {
    rend->destroyFontsTexture();
    if (rend->areTexturesSquare()) {
      ImGui::GetIO().Fonts->Flags|=ImFontAtlasFlags_Square;
    }
  }
  if (!ImGui::GetIO().Fonts->Build()) {
    logE("error while building font atlas!");
    showError(_("error while loading fonts! please check your settings."));
    ImGui::GetIO().Fonts->Clear();
    mainFont=ImGui::GetIO().Fonts->AddFontDefault();
    patFont=mainFont;
    bigFont=mainFont;
    headFont=mainFont;
    if (rend) {
      rend->destroyFontsTexture();
      if (rend->areTexturesSquare()) {
        ImGui::GetIO().Fonts->Flags|=ImFontAtlasFlags_Square;
      }
    }
    if (!ImGui::GetIO().Fonts->Build()) {
      logE("error again while building font atlas!");
    } else {
      rend->createFontsTexture();
    }
  } else {
    rend->createFontsTexture();
  }

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
  commitSettings();

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

#define IGFD_FileStyleByExtension IGFD_FileStyleByExtention

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
  sty.Colors[ImGuiCol_NavHighlight]=uiColors[GUI_COLOR_NAV_HIGHLIGHT];
  sty.Colors[ImGuiCol_NavWindowingHighlight]=uiColors[GUI_COLOR_NAV_WIN_HIGHLIGHT];
  sty.Colors[ImGuiCol_NavWindowingDimBg]=uiColors[GUI_COLOR_NAV_WIN_BACKDROP];
  sty.Colors[ImGuiCol_Text]=uiColors[GUI_COLOR_TEXT];
  sty.Colors[ImGuiCol_TextDisabled]=uiColors[GUI_COLOR_TEXT_DISABLED];

  if (settings.basicColors) {
    sty.Colors[ImGuiCol_Button]=primary;
    sty.Colors[ImGuiCol_ButtonHovered]=primaryHover;
    sty.Colors[ImGuiCol_ButtonActive]=primaryActive;
    sty.Colors[ImGuiCol_Tab]=primary;
    sty.Colors[ImGuiCol_TabHovered]=secondaryHover;
    sty.Colors[ImGuiCol_TabActive]=secondarySemiActive;
    sty.Colors[ImGuiCol_TabUnfocused]=primary;
    sty.Colors[ImGuiCol_TabUnfocusedActive]=primaryHover;
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
    sty.Colors[ImGuiCol_TextSelectedBg]=secondaryHoverActual;
    sty.Colors[ImGuiCol_PlotHistogram]=uiColors[GUI_COLOR_MACRO_OTHER];
    sty.Colors[ImGuiCol_PlotHistogramHovered]=uiColors[GUI_COLOR_MACRO_OTHER];
  } else {
    sty.Colors[ImGuiCol_Button]=uiColors[GUI_COLOR_BUTTON];
    sty.Colors[ImGuiCol_ButtonHovered]=uiColors[GUI_COLOR_BUTTON_HOVER];
    sty.Colors[ImGuiCol_ButtonActive]=uiColors[GUI_COLOR_BUTTON_ACTIVE];
    sty.Colors[ImGuiCol_Tab]=uiColors[GUI_COLOR_TAB];
    sty.Colors[ImGuiCol_TabHovered]=uiColors[GUI_COLOR_TAB_HOVER];
    sty.Colors[ImGuiCol_TabActive]=uiColors[GUI_COLOR_TAB_ACTIVE];
    sty.Colors[ImGuiCol_TabUnfocused]=uiColors[GUI_COLOR_TAB_UNFOCUSED];
    sty.Colors[ImGuiCol_TabUnfocusedActive]=uiColors[GUI_COLOR_TAB_UNFOCUSED_ACTIVE];
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
    sty.Colors[ImGuiCol_TextSelectedBg]=uiColors[GUI_COLOR_TEXT_SELECTION];
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
      ImGui::GetIO().Fonts->FontBuilderIO=ImGuiFreeType::GetBuilderForFreeType();
      ImGui::GetIO().Fonts->FontBuilderFlags&=~(
        ImGuiFreeTypeBuilderFlags_NoHinting|
        ImGuiFreeTypeBuilderFlags_NoAutoHint|
        ImGuiFreeTypeBuilderFlags_ForceAutoHint|
        ImGuiFreeTypeBuilderFlags_LightHinting|
        ImGuiFreeTypeBuilderFlags_MonoHinting|
        ImGuiFreeTypeBuilderFlags_Bold|
        ImGuiFreeTypeBuilderFlags_Oblique|
        ImGuiFreeTypeBuilderFlags_Monochrome|
        ImGuiFreeTypeBuilderFlags_LoadColor|
        ImGuiFreeTypeBuilderFlags_Bitmap
      );

      if (!settings.fontAntiAlias) ImGui::GetIO().Fonts->FontBuilderFlags|=ImGuiFreeTypeBuilderFlags_Monochrome;
      if (settings.fontBitmap) ImGui::GetIO().Fonts->FontBuilderFlags|=ImGuiFreeTypeBuilderFlags_Bitmap;

      switch (settings.fontHinting) {
        case 0: // off
          ImGui::GetIO().Fonts->FontBuilderFlags|=ImGuiFreeTypeBuilderFlags_NoHinting;
          break;
        case 1: // slight
          ImGui::GetIO().Fonts->FontBuilderFlags|=ImGuiFreeTypeBuilderFlags_LightHinting;
          break;
        case 2: // normal
          break;
        case 3: // full
          ImGui::GetIO().Fonts->FontBuilderFlags|=ImGuiFreeTypeBuilderFlags_MonoHinting;
          break;
      }

      switch (settings.fontAutoHint) {
        case 0: // off
          ImGui::GetIO().Fonts->FontBuilderFlags|=ImGuiFreeTypeBuilderFlags_NoAutoHint;
          break;
        case 1: // on
          break;
        case 2: // force
          ImGui::GetIO().Fonts->FontBuilderFlags|=ImGuiFreeTypeBuilderFlags_ForceAutoHint;
          break;
      }
    } else {
      ImGui::GetIO().Fonts->FontBuilderIO=ImFontAtlasGetBuilderForStbTruetype();
    }
#endif


    // set to 800 for now due to problems with unifont
    static const ImWchar upTo800[]={
      // base
      0x20,0x7e,0xa0,0x800,
      // for "Display characters" and language choices
      0x107, 0x107,
      0x10d, 0x10d,
      0x131, 0x131,
      0x142, 0x142,
      0x15f, 0x15f,
      0x17c, 0x17c,
      0x420, 0x420,
      0x423, 0x423,
      0x430, 0x430,
      0x431, 0x431,
      0x432, 0x432,
      0x433, 0x433,
      0x435, 0x435,
      0x437, 0x437,
      0x438, 0x438,
      0x439, 0x439,
      0x43a, 0x43a,
      0x43b, 0x43b,
      0x43c, 0x43c,
      0x43d, 0x43d,
      0x43e, 0x43e,
      0x43f, 0x43f,
      0x440, 0x440,
      0x441, 0x441,
      0x442, 0x442,
      0x443, 0x443,
      0x446, 0x446,
      0x447, 0x447,
      0x448, 0x448,
      0x449, 0x449,
      0x44b, 0x44b,
      0x44c, 0x44c,
      0x44d, 0x44d,
      0x44f, 0x44f,
      0x456, 0x456,
      0x457, 0x457,
      0x540, 0x540,
      0x561, 0x561,
      0x565, 0x565,
      0x575, 0x575,
      0x576, 0x576,
      0x580, 0x580,
      0xe17, 0xe17,
      0xe22, 0xe22,
      0xe44, 0xe44,
      0x3001, 0x3001,
      0x3002, 0x3002,
      0x3042, 0x3042,
      0x3044, 0x3044,
      0x3048, 0x3048,
      0x304c, 0x304c,
      0x304f, 0x304f,
      0x3053, 0x3053,
      0x3055, 0x3055,
      0x3059, 0x3059,
      0x3060, 0x3060,
      0x3066, 0x3066,
      0x3067, 0x3067,
      0x306a, 0x306a,
      0x306b, 0x306b,
      0x306e, 0x306e,
      0x306f, 0x306f,
      0x307e, 0x307e,
      0x307f, 0x307f,
      0x308a, 0x308a,
      0x308b, 0x308b,
      0x308c, 0x308c,
      0x30a2, 0x30a2,
      0x30a3, 0x30a3,
      0x30a4, 0x30a4,
      0x30a9, 0x30a9,
      0x30aa, 0x30aa,
      0x30af, 0x30af,
      0x30b0, 0x30b0,
      0x30b7, 0x30b7,
      0x30b9, 0x30b9,
      0x30c0, 0x30c0,
      0x30c3, 0x30c3,
      0x30c8, 0x30c8,
      0x30ca, 0x30ca,
      0x30d5, 0x30d5,
      0x30d7, 0x30d7,
      0x30df, 0x30df,
      0x30e1, 0x30e1,
      0x30e2, 0x30e2,
      0x30e7, 0x30e7,
      0x30e9, 0x30e9,
      0x30ea, 0x30ea,
      0x30f3, 0x30f3,
      0x4e00, 0x4e00,
      0x4e2a, 0x4e2a,
      0x4e34, 0x4e34,
      0x4e4b, 0x4e4b,
      0x4f53, 0x4f53,
      0x4f60, 0x4f60,
      0x4fdd, 0x4fdd,
      0x500b, 0x500b,
      0x518d, 0x518d,
      0x51b3, 0x51b3,
      0x5206, 0x5206,
      0x5207, 0x5207,
      0x524d, 0x524d,
      0x52a0, 0x52a0,
      0x52a8, 0x52a8,
      0x52d5, 0x52d5,
      0x5341, 0x5341,
      0x5408, 0x5408,
      0x540e, 0x540e,
      0x542f, 0x542f,
      0x555f, 0x555f,
      0x5728, 0x5728,
      0x5834, 0x5834,
      0x591f, 0x591f,
      0x5920, 0x5920,
      0x5b57, 0x5b57,
      0x5b58, 0x5b58,
      0x5b9a, 0x5b9a,
      0x5b9e, 0x5b9e,
      0x5b9f, 0x5b9f,
      0x5be6, 0x5be6,
      0x6001, 0x6001,
      0x614b, 0x614b,
      0x65b9, 0x65b9,
      0x65e5, 0x65e5,
      0x65f6, 0x65f6,
      0x662f, 0x662f,
      0x663e, 0x663e,
      0x6642, 0x6642,
      0x66ff, 0x66ff,
      0x6709, 0x6709,
      0x672c, 0x672c,
      0x6848, 0x6848,
      0x6b64, 0x6b64,
      0x6c7a, 0x6c7a,
      0x73b0, 0x73b0,
      0x73fe, 0x73fe,
      0x7684, 0x7684,
      0x786e, 0x786e,
      0x78ba, 0x78ba,
      0x7b56, 0x7b56,
      0x81e8, 0x81e8,
      0x88c5, 0x88c5,
      0x89e3, 0x89e3,
      0x8a2d, 0x8a2d,
      0x8a9e, 0x8a9e,
      0x8acb, 0x8acb,
      0x8bbe, 0x8bbe,
      0x8bf7, 0x8bf7,
      0x8db3, 0x8db3,
      0x8f09, 0x8f09,
      0x8f7d, 0x8f7d,
      0x8fd9, 0x8fd9,
      0x9019, 0x9019,
      0x986f, 0x986f,
      0x9ad4, 0x9ad4,
      0xac00, 0xac00,
      0xacbd, 0xacbd,
      0xad6c, 0xad6c,
      0xad6d, 0xad6d,
      0xadf8, 0xadf8,
      0xae00, 0xae00,
      0xae4c, 0xae4c,
      0xaf34, 0xaf34,
      0xb2c8, 0xb2c8,
      0xb2e4, 0xb2e4,
      0xb3d9, 0xb3d9,
      0xb420, 0xb420,
      0xb54c, 0xb54c,
      0xb77c, 0xb77c,
      0xb798, 0xb798,
      0xb824, 0xb824,
      0xb8e8, 0xb8e8,
      0xb97c, 0xb97c,
      0xb9ac, 0xb9ac,
      0xb9cc, 0xb9cc,
      0xba54, 0xba54,
      0xba74, 0xba74,
      0xbaa8, 0xbaa8,
      0xbd84, 0xbd84,
      0xc120, 0xc120,
      0xc124, 0xc124,
      0xc158, 0xc158,
      0xc194, 0xc194,
      0xc2a4, 0xc2a4,
      0xc2dc, 0xc2dc,
      0xc2ed, 0xc2ed,
      0xc544, 0xc544,
      0xc57c, 0xc57c,
      0xc5b4, 0xc5b4,
      0xc5d0, 0xc5d0,
      0xc624, 0xc624,
      0xc635, 0xc635,
      0xc6a9, 0xc6a9,
      0xc6b0, 0xc6b0,
      0xc740, 0xc740,
      0xc744, 0xc744,
      0xc774, 0xc774,
      0xc784, 0xc784,
      0xc785, 0xc785,
      0xc791, 0xc791,
      0xc801, 0xc801,
      0xc815, 0xc815,
      0xc9c0, 0xc9c0,
      0xcda9, 0xcda9,
      0xd0dd, 0xd0dd,
      0xd2c0, 0xd2c0,
      0xd53d, 0xd53d,
      0xd558, 0xd558,
      0xd55c, 0xd55c,
      0xd569, 0xd569,
      0xd574, 0xd574,
      0xd604, 0xd604,
      0
    };
    ImFontGlyphRangesBuilder range;
    ImVector<ImWchar> outRange;

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

    range.AddRanges(upTo800);
    if (settings.loadJapanese || localeRequiresJapanese) {
      range.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesJapanese());
    }
    if (settings.loadChinese || localeRequiresChinese) {
      range.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesChineseSimplifiedCommon());
    }
    if (settings.loadChineseTraditional || localeRequiresChineseTrad) {
      range.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesChineseFull());
    }
    if (settings.loadKorean || localeRequiresKorean) {
      range.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesKorean());
    }
    if (!localeExtraRanges.empty()) {
      range.AddRanges(localeExtraRanges.data());
    }
    // I'm terribly sorry
    range.UsedChars[0x80>>5]=0;

    range.BuildRanges(&outRange);
    if (fontRange!=NULL) delete[] fontRange;
    fontRange=new ImWchar[outRange.size()];
    int index=0;
    for (ImWchar& i: outRange) {
      fontRange[index++]=i;
    }

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
      if ((mainFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(settings.mainFontPath.c_str(),MAX(1,e->getConfInt("mainFontSize",18)*dpiScale),&fontConf,fontRange))==NULL) {
        logW("could not load UI font! reverting to default font");
        settings.mainFont=GUI_MAIN_FONT_DEFAULT;
        if ((mainFont=addFontZlib(builtinFont[settings.mainFont],builtinFontLen[settings.mainFont],MAX(1,e->getConfInt("mainFontSize",18)*dpiScale),&fontConf,fontRange))==NULL) {
          logE("could not load UI font! falling back to Proggy Clean.");
          mainFont=ImGui::GetIO().Fonts->AddFontDefault();
        }
      }
    } else if (settings.mainFont==5) { // system font
      if ((mainFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_FONT_PATH_1,MAX(1,e->getConfInt("mainFontSize",18)*dpiScale),&fontConf,fontRange))==NULL) {
        if ((mainFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_FONT_PATH_2,MAX(1,e->getConfInt("mainFontSize",18)*dpiScale),&fontConf,fontRange))==NULL) {
          if ((mainFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_FONT_PATH_3,MAX(1,e->getConfInt("mainFontSize",18)*dpiScale),&fontConf,fontRange))==NULL) {
            logW("could not load UI font! reverting to default font");
            settings.mainFont=GUI_MAIN_FONT_DEFAULT;
            if ((mainFont=addFontZlib(builtinFont[settings.mainFont],builtinFontLen[settings.mainFont],MAX(1,e->getConfInt("mainFontSize",18)*dpiScale),&fontConf,fontRange))==NULL) {
              logE("could not load UI font! falling back to Proggy Clean.");
              mainFont=ImGui::GetIO().Fonts->AddFontDefault();
            }
          }
        }
      }
    } else {
      if ((mainFont=addFontZlib(builtinFont[settings.mainFont],builtinFontLen[settings.mainFont],MAX(1,e->getConfInt("mainFontSize",18)*dpiScale),&fontConf,fontRange))==NULL) {
        logE("could not load UI font! falling back to Proggy Clean.");
        mainFont=ImGui::GetIO().Fonts->AddFontDefault();
      }
    }

    // four fallback fonts
    if (settings.loadJapanese ||
        settings.loadChinese ||
        settings.loadChineseTraditional ||
        settings.loadKorean ||
        localeRequiresJapanese ||
        localeRequiresChinese ||
        localeRequiresChineseTrad ||
        localeRequiresKorean ||
        settings.loadFallback) {
      mainFont=addFontZlib(font_plexSans_compressed_data,font_plexSans_compressed_size,MAX(1,e->getConfInt("mainFontSize",18)*dpiScale),&fc1,fontRange);
      mainFont=addFontZlib(font_plexSansJP_compressed_data,font_plexSansJP_compressed_size,MAX(1,e->getConfInt("mainFontSize",18)*dpiScale),&fc1,fontRange);
      mainFont=addFontZlib(font_plexSansKR_compressed_data,font_plexSansKR_compressed_size,MAX(1,e->getConfInt("mainFontSize",18)*dpiScale),&fc1,fontRange);
      mainFont=addFontZlib(font_unifont_compressed_data,font_unifont_compressed_size,MAX(1,e->getConfInt("mainFontSize",18)*dpiScale),&fc1,fontRange);
    }

    ImFontConfig fc;
    fc.MergeMode=true;
    fc.OversampleH=1;
    fc.OversampleV=1;
    fc.PixelSnapH=true;
    fc.GlyphMinAdvanceX=e->getConfInt("iconSize",16)*dpiScale;
    static const ImWchar fontRangeIcon[]={ICON_MIN_FA,ICON_MAX_FA,0};
    if ((iconFont=addFontZlib(iconFont_compressed_data,iconFont_compressed_size,MAX(1,e->getConfInt("iconSize",16)*dpiScale),&fc,fontRangeIcon))==NULL) {
      logE("could not load icon font!");
    }

    static const ImWchar fontRangeFurIcon[]={ICON_MIN_FUR,ICON_MAX_FUR,0};
    if ((furIconFont=addFontZlib(furIcons_compressed_data,furIcons_compressed_size,MAX(1,e->getConfInt("iconSize",16)*dpiScale),&fc,fontRangeFurIcon))==NULL) {
      logE("could not load Furnace icons font!");
    }

    if (settings.mainFontSize==settings.patFontSize && settings.patFont<5 && builtinFontM[settings.patFont]==builtinFont[settings.mainFont]) {
      logD("using main font for pat font.");
      patFont=mainFont;
    } else {
      if (settings.patFont==6) { // custom font
        if ((patFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(settings.patFontPath.c_str(),MAX(1,e->getConfInt("patFontSize",18)*dpiScale),&fontConfP,upTo800))==NULL) {
          logW("could not load pattern font! reverting to default font");
          settings.patFont=GUI_PAT_FONT_DEFAULT;
          if ((patFont=addFontZlib(builtinFontM[settings.patFont],builtinFontMLen[settings.patFont],MAX(1,e->getConfInt("patFontSize",18)*dpiScale),&fontConfP,upTo800))==NULL) {
            logE("could not load pattern font! falling back to Proggy Clean.");
            patFont=ImGui::GetIO().Fonts->AddFontDefault();
          }
        }
      } else if (settings.patFont==5) { // system font
        if ((patFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_PAT_FONT_PATH_1,MAX(1,e->getConfInt("patFontSize",18)*dpiScale),&fontConfP,upTo800))==NULL) {
          if ((patFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_PAT_FONT_PATH_2,MAX(1,e->getConfInt("patFontSize",18)*dpiScale),&fontConfP,upTo800))==NULL) {
            if ((patFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_PAT_FONT_PATH_3,MAX(1,e->getConfInt("patFontSize",18)*dpiScale),&fontConfP,upTo800))==NULL) {
              logW("could not load pattern font! reverting to default font");
              settings.patFont=GUI_PAT_FONT_DEFAULT;
              if ((patFont=addFontZlib(builtinFontM[settings.patFont],builtinFontMLen[settings.patFont],MAX(1,e->getConfInt("patFontSize",18)*dpiScale),&fontConfP,upTo800))==NULL) {
                logE("could not load pattern font! falling back to Proggy Clean.");
                patFont=ImGui::GetIO().Fonts->AddFontDefault();
              }
            }
          }
        }
      } else {
        if ((patFont=addFontZlib(builtinFontM[settings.patFont],builtinFontMLen[settings.patFont],MAX(1,e->getConfInt("patFontSize",18)*dpiScale),&fontConfP,upTo800))==NULL) {
          logE("could not load pattern font!");
          patFont=ImGui::GetIO().Fonts->AddFontDefault();
        }
      }
    }

    // 0x39B = Λ
    // Հայերեն
    // 한국어
    // Русский
    // č
    // ไทย
    static const ImWchar bigFontRange[]={0x20,0xFF,0x39b,0x39b,0x10d,0x10d,0x420,0x420,0x423,0x423,0x430,0x430,0x438,0x438,0x439,0x439,0x43a,0x43a,0x43d,0x43d,0x440,0x440,0x441,0x441,0x443,0x443,0x44c,0x44c,0x457,0x457,0x540,0x540,0x561,0x561,0x565,0x565,0x575,0x575,0x576,0x576,0x580,0x580,0xe17,0xe17,0xe22,0xe22,0xe44,0xe44,0x65e5,0x65e5,0x672c,0x672c,0x8a9e,0x8a9e,0xad6d,0xad6d,0xc5b4,0xc5b4,0xd55c,0xd55c,0};

    ImFontGlyphRangesBuilder bigFontRangeB;
    ImVector<ImWchar> outRangeB;

    bigFontRangeB.AddRanges(bigFontRange);
    if (!localeExtraRanges.empty()) {
      bigFontRangeB.AddRanges(localeExtraRanges.data());
    }
    // I'm terribly sorry
    bigFontRangeB.UsedChars[0x80>>5]=0;

    bigFontRangeB.BuildRanges(&outRangeB);
    if (fontRangeB!=NULL) delete[] fontRangeB;
    fontRangeB=new ImWchar[outRangeB.size()];
    index=0;
    for (ImWchar& i: outRangeB) {
      fontRangeB[index++]=i;
    }

    if ((bigFont=addFontZlib(font_plexSans_compressed_data,font_plexSans_compressed_size,MAX(1,40*dpiScale),&fontConfB,fontRangeB))==NULL) {
      logE("could not load big UI font!");
    }
    fontConfB.MergeMode=true;
    if ((bigFont=addFontZlib(font_plexSansJP_compressed_data,font_plexSansJP_compressed_size,MAX(1,40*dpiScale),&fontConfB,fontRangeB))==NULL) {
      logE("could not load big UI font (japanese)!");
    }
    if ((bigFont=addFontZlib(font_plexSansKR_compressed_data,font_plexSansKR_compressed_size,MAX(1,40*dpiScale),&fontConfB,fontRangeB))==NULL) {
      logE("could not load big UI font (korean)!");
    }
    if ((bigFont=addFontZlib(font_unifont_compressed_data,font_unifont_compressed_size,MAX(1,40*dpiScale),&fontConfB,fontRangeB))==NULL) {
      logE("could not load big UI font (fallback)!");
    }

    if (settings.mainFontSize==settings.headFontSize && settings.headFont<5 && builtinFont[settings.headFont]==builtinFont[settings.mainFont]) {
      logD("using main font for header font.");
      headFont=mainFont;
    } else {
      if (settings.headFont==6) { // custom font
        if ((headFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(settings.headFontPath.c_str(),MAX(1,e->getConfInt("headFontSize",27)*dpiScale),&fontConfH,upTo800))==NULL) {
          logW("could not load header font! reverting to default font");
          settings.headFont=0;
          if ((headFont=addFontZlib(builtinFont[settings.headFont],builtinFontLen[settings.headFont],MAX(1,e->getConfInt("headFontSize",27)*dpiScale),&fontConfH,upTo800))==NULL) {
            logE("could not load header font! falling back to IBM Plex Sans.");
            headFont=ImGui::GetIO().Fonts->AddFontDefault();
          }
        }
      } else if (settings.headFont==5) { // system font
        if ((headFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_HEAD_FONT_PATH_1,MAX(1,e->getConfInt("headFontSize",27)*dpiScale),&fontConfH,upTo800))==NULL) {
          if ((headFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_HEAD_FONT_PATH_2,MAX(1,e->getConfInt("headFontSize",27)*dpiScale),&fontConfH,upTo800))==NULL) {
            if ((headFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_HEAD_FONT_PATH_3,MAX(1,e->getConfInt("headFontSize",27)*dpiScale),&fontConfH,upTo800))==NULL) {
              logW("could not load header font! reverting to default font");
              settings.headFont=0;
              if ((headFont=addFontZlib(builtinFont[settings.headFont],builtinFontLen[settings.headFont],MAX(1,e->getConfInt("headFontSize",27)*dpiScale),&fontConfH,upTo800))==NULL) {
                logE("could not load header font! falling back to IBM Plex Sans.");
                headFont=ImGui::GetIO().Fonts->AddFontDefault();
              }
            }
          }
        }
      } else {
        if ((headFont=addFontZlib(builtinFont[settings.headFont],builtinFontLen[settings.headFont],MAX(1,e->getConfInt("headFontSize",27)*dpiScale),&fontConfH,upTo800))==NULL) {
          logE("could not load header font!");
          headFont=ImGui::GetIO().Fonts->AddFontDefault();
        }
      }
    }

    mainFont->FallbackChar='?';
    mainFont->EllipsisChar='.';
    mainFont->EllipsisCharCount=3;
  } else if (updateFonts) {
    // safe mode
    mainFont=ImGui::GetIO().Fonts->AddFontDefault();
    patFont=mainFont;
    bigFont=mainFont;
    headFont=mainFont;

    mainFont->FallbackChar='?';
    mainFont->EllipsisChar='.';
    mainFont->EllipsisCharCount=3;
  }


    ImGuiFileDialog::Instance()->okButtonString=_("OK");
    ImGuiFileDialog::Instance()->cancelButtonString=_("Cancel");
    ImGuiFileDialog::Instance()->searchString=_("Search");
    ImGuiFileDialog::Instance()->dirEntryString=_("[Dir]");
    ImGuiFileDialog::Instance()->linkEntryString=_("[Link]");
    ImGuiFileDialog::Instance()->fileEntryString=_("[File]");
    ImGuiFileDialog::Instance()->fileNameString=_("Name:");
    ImGuiFileDialog::Instance()->dirNameString=_("Path:");
    ImGuiFileDialog::Instance()->buttonResetSearchString=_("Reset search");
    ImGuiFileDialog::Instance()->buttonDriveString=_("Drives");
    ImGuiFileDialog::Instance()->buttonEditPathString=_("Edit path\nYou can also right click on path buttons");
    ImGuiFileDialog::Instance()->buttonResetPathString=_("Go to home directory");
    ImGuiFileDialog::Instance()->buttonParentDirString=_("Go to parent directory");
    ImGuiFileDialog::Instance()->buttonCreateDirString=_("Create Directory");
    ImGuiFileDialog::Instance()->tableHeaderFileNameString=_("File name");
    ImGuiFileDialog::Instance()->tableHeaderFileTypeString=_("Type");
    ImGuiFileDialog::Instance()->tableHeaderFileSizeString=_("Size");
    ImGuiFileDialog::Instance()->tableHeaderFileDateString=_("Date");
    ImGuiFileDialog::Instance()->OverWriteDialogTitleString=_("Warning");
    ImGuiFileDialog::Instance()->OverWriteDialogMessageString=_("The file you selected already exists! Would you like to overwrite it?");
    ImGuiFileDialog::Instance()->OverWriteDialogConfirmButtonString=_("Yes");
    ImGuiFileDialog::Instance()->OverWriteDialogCancelButtonString=_("No");
    ImGuiFileDialog::Instance()->DateTimeFormat=_("%Y/%m/%d %H:%M");

  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir,"",uiColors[GUI_COLOR_FILE_DIR],ICON_FA_FOLDER_O);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile,"",uiColors[GUI_COLOR_FILE_OTHER],ICON_FA_FILE_O);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".fur",uiColors[GUI_COLOR_FILE_SONG_NATIVE],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".fui",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".fuw",uiColors[GUI_COLOR_FILE_WAVE],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".dmp",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".dmw",uiColors[GUI_COLOR_FILE_WAVE],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".wav",uiColors[GUI_COLOR_FILE_AUDIO],ICON_FA_FILE_AUDIO_O);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".dmc",uiColors[GUI_COLOR_FILE_AUDIO],ICON_FA_FILE_AUDIO_O);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".brr",uiColors[GUI_COLOR_FILE_AUDIO],ICON_FA_FILE_AUDIO_O);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".vgm",uiColors[GUI_COLOR_FILE_VGM],ICON_FA_FILE_AUDIO_O);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".zsm",uiColors[GUI_COLOR_FILE_ZSM],ICON_FA_FILE_AUDIO_O);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".ttf",uiColors[GUI_COLOR_FILE_FONT],ICON_FA_FONT);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".otf",uiColors[GUI_COLOR_FILE_FONT],ICON_FA_FONT);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".ttc",uiColors[GUI_COLOR_FILE_FONT],ICON_FA_FONT);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".dfont",uiColors[GUI_COLOR_FILE_FONT],ICON_FA_FONT);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".fon",uiColors[GUI_COLOR_FILE_FONT],ICON_FA_FONT);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".pcf",uiColors[GUI_COLOR_FILE_FONT],ICON_FA_FONT);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".psf",uiColors[GUI_COLOR_FILE_FONT],ICON_FA_FONT);

  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".dmf",uiColors[GUI_COLOR_FILE_SONG_IMPORT],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".mod",uiColors[GUI_COLOR_FILE_SONG_IMPORT],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".s3m",uiColors[GUI_COLOR_FILE_SONG_IMPORT],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".xm",uiColors[GUI_COLOR_FILE_SONG_IMPORT],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".it",uiColors[GUI_COLOR_FILE_SONG_IMPORT],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".fc13",uiColors[GUI_COLOR_FILE_SONG_IMPORT],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".fc14",uiColors[GUI_COLOR_FILE_SONG_IMPORT],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".fc",uiColors[GUI_COLOR_FILE_SONG_IMPORT],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".smod",uiColors[GUI_COLOR_FILE_SONG_IMPORT],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".ftm",uiColors[GUI_COLOR_FILE_SONG_IMPORT],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".0cc",uiColors[GUI_COLOR_FILE_SONG_IMPORT],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".dnm",uiColors[GUI_COLOR_FILE_SONG_IMPORT],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".eft",uiColors[GUI_COLOR_FILE_SONG_IMPORT],ICON_FA_FILE);

  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".fub",uiColors[GUI_COLOR_FILE_SONG_IMPORT],ICON_FA_FILE);

  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".tfi",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".vgi",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".s3i",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".sbi",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".opli",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".opni",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".y12",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".bnk",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".fti",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".bti",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".ff",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".opm",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);

  if (updateFonts) {
    if (fileDialog!=NULL) delete fileDialog;
    fileDialog=new FurnaceGUIFileDialog(settings.sysFileDialog);

    fileDialog->mobileUI=mobileUI;
  }
}
