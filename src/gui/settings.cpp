/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2025 tildearrow and contributors
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
#include "ImGuiFileDialog.h"
#include "IconsFontAwesome4.h"
#include "furIcons.h"
#include "misc/cpp/imgui_stdlib.h"
#include "misc/freetype/imgui_freetype.h"
#include "scaling.h"
#include <fmt/printf.h>

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

#define DEFAULT_NOTE_KEYS "5:7;6:4;7:3;8:16;10:6;11:8;12:24;13:10;16:11;17:9;18:26;19:28;20:12;21:17;22:1;23:19;24:23;25:5;26:14;27:2;28:21;29:0;30:100;31:13;32:15;34:18;35:20;36:22;38:25;39:27;43:100;46:101;47:29;48:31;53:102;"

#if defined(_WIN32) || defined(__APPLE__) || defined(IS_MOBILE)
#define POWER_SAVE_DEFAULT 1
#else
// currently off on Linux/other due to Mesa catch-up behavior.
#define POWER_SAVE_DEFAULT 0
#endif

#ifdef HAVE_FREETYPE
#define FONT_BACKEND_DEFAULT 1
#else
#define FONT_BACKEND_DEFAULT 0
#endif

#if defined(__HAIKU__) || defined(IS_MOBILE) || (defined(_WIN32) && !defined(_WIN64))
// NFD doesn't support Haiku
// NFD doesn't support Windows XP either
#define SYS_FILE_DIALOG_DEFAULT 0
#else
#define SYS_FILE_DIALOG_DEFAULT 1
#endif

// name, locale, restart message
const char* locales[][3]={
  {"<System>", "", ""},
  {"English", "en_US", "restart Furnace for this setting to take effect."},
  {"Bahasa Indonesia (50%?)", "id_ID", "???"},
  //{"Deutsch (0%)", "de_DE", "Starten Sie Furnace neu, damit diese Einstellung wirksam wird."},
  {"Español", "es_ES", "reinicia Furnace para que esta opción tenga efecto."},
  //{"Suomi (0%)", "fi_FI", "käynnistä Furnace uudelleen, jotta tämä asetus tulee voimaan."},
  //{"Français (0%)", "fr_FR", "redémarrer Furnace pour que ce réglage soit effectif."},
  //{"Հայերեն (1%)", "hy_AM", "???"},
  //{"日本語 (0%)", "ja_JP", "???"},
  {"한국어 (25%)", "ko_KR", "이 설정을 적용하려면 Furnace를 다시 시작해야 합니다."},
  //{"Nederlands (4%)", "nl_NL", "start Furnace opnieuw op om deze instelling effectief te maken."},
  {"Polski (95%)", "pl_PL", "aby to ustawienie było skuteczne, należy ponownie uruchomić program."},
  {"Português (Brasil) (90%)", "pt_BR", "reinicie o Furnace para que essa configuração entre em vigor."},
  {"Русский", "ru_RU", "перезапустите программу, чтобы эта настройка вступила в силу."},
  {"Slovenčina (15%)", "sk_SK", "???"},
  {"Svenska", "sv_SE", "starta om programmet för att denna inställning ska träda i kraft."},
  //{"ไทย (0%)", "th_TH", "???"},
  //{"Türkçe (0%)", "tr_TR", "bu ayarı etkin hale getirmek için programı yeniden başlatın."},
  //{"Українська (0%)", "uk_UA", "перезапустіть програму, щоб це налаштування набуло чинності."},
  {"中文 (15%)", "zh_CN", "???"},
  {NULL, NULL, NULL}
};

const char* fontBackends[]={
  "stb_truetype",
  "FreeType"
};

const char* mainFonts[]={
  "IBM Plex Sans",
  "Liberation Sans",
  "Exo",
  "Proggy Clean",
  "GNU Unifont",
  _N("<Use system font>"),
  _N("<Custom...>")
};

const char* headFonts[]={
  "IBM Plex Sans",
  "Liberation Sans",
  "Exo",
  "Proggy Clean",
  "GNU Unifont",
  _N("<Use system font>"),
  _N("<Custom...>")
};

const char* patFonts[]={
  "IBM Plex Mono",
  "Mononoki",
  "PT Mono",
  "Proggy Clean",
  "GNU Unifont",
  _N("<Use system font>"),
  _N("<Custom...>")
};

const char* audioBackends[]={
  "JACK",
  "SDL",
  "PortAudio"
};

const char* audioQualities[]={
  _N("High"),
  _N("Low")
};

const char* arcadeCores[]={
  "ymfm",
  "Nuked-OPM"
};

const char* ym2612Cores[]={
  "Nuked-OPN2",
  "ymfm",
  "YMF276-LLE"
};

const char* snCores[]={
  "MAME",
  "Nuked-PSG Mod"
};

const char* nesCores[]={
  "puNES",
  "NSFplay"
};

const char* c64Cores[]={
  "reSID",
  "reSIDfp",
  "dSID"
};

const char* pokeyCores[]={
  "Atari800 (mzpokeysnd)",
  _N("ASAP (C++ port)")
};

const char* opnCores[]={
  "ymfm",
  "Nuked-OPN2 (FM) + ymfm (SSG/ADPCM)",
  "YM2608-LLE"
};

const char* opl2Cores[]={
  "Nuked-OPL3",
  "ymfm",
  "YM3812-LLE"
};

const char* opl3Cores[]={
  "Nuked-OPL3",
  "ymfm",
  "YMF262-LLE"
};

const char* opl4Cores[]={
  "Nuked-OPL3 (FM) + openMSX (PCM)",
  "ymfm"
};

const char* esfmCores[]={
  "ESFMu",
  _N("ESFMu (fast)")
};

const char* opllCores[]={
  "Nuked-OPLL",
  "emu2413"
};

const char* ayCores[]={
  "MAME",
  "AtomicSSG"
};

const char* swanCores[]={
  "asiekierka new core",
  "Mednafen"
};

const char* coreQualities[]={
  _N("Lower"),
  _N("Low"),
  _N("Medium"),
  _N("High"),
  _N("Ultra"),
  _N("Ultimate")
};

const char* pcspkrOutMethods[]={
  _N("evdev SND_TONE"),
  _N("KIOCSOUND on /dev/tty1"),
  _N("/dev/port"),
  _N("KIOCSOUND on standard output"),
  _N("outb()")
};

const char* valueInputStyles[]={
  _N("Disabled/custom"),
  _N("Two octaves (0 is C-4, F is D#5)"),
  _N("Raw (note number is value)"),
  _N("Two octaves alternate (lower keys are 0-9, upper keys are A-F)"),
  _N("Use dual control change (one for each nibble)"),
  _N("Use 14-bit control change"),
  _N("Use single control change (imprecise)")
};

const char* valueSInputStyles[]={
  _N("Disabled/custom"),
  _N("Use dual control change (one for each nibble)"),
  _N("Use 14-bit control change"),
  _N("Use single control change (imprecise)")
};

const char* messageTypes[]={
  _N("--select--"),
  _N("???"),
  _N("???"),
  _N("???"),
  _N("???"),
  _N("???"),
  _N("???"),
  _N("???"),
  _N("Note Off"),
  _N("Note On"),
  _N("Aftertouch"),
  _N("Control"),
  _N("Program"),
  _N("ChanPressure"),
  _N("Pitch Bend"),
  _N("SysEx")
};

const char* messageChannels[]={
  "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", _N("Any")
};

const char* specificControls[18]={
  _N("Instrument"),
  _N("Volume"),
  _N("Effect 1 type"),
  _N("Effect 1 value"),
  _N("Effect 2 type"),
  _N("Effect 2 value"),
  _N("Effect 3 type"),
  _N("Effect 3 value"),
  _N("Effect 4 type"),
  _N("Effect 4 value"),
  _N("Effect 5 type"),
  _N("Effect 5 value"),
  _N("Effect 6 type"),
  _N("Effect 6 value"),
  _N("Effect 7 type"),
  _N("Effect 7 value"),
  _N("Effect 8 type"),
  _N("Effect 8 value")
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

#define UI_COLOR_CONFIG(what,label) \
  ImGui::PushID(what); \
  if (ImGui::ColorEdit4(label,(float*)&uiColors[what])) { \
    applyUISettings(false); \
    settingsChanged=true; \
  } \
  ImGui::PopID();

#define KEYBIND_CONFIG_BEGIN(id) \
  if (ImGui::BeginTable(id,2,ImGuiTableFlags_SizingFixedFit|ImGuiTableFlags_NoHostExtendX|ImGuiTableFlags_NoClip)) {

#define KEYBIND_CONFIG_END \
    ImGui::EndTable(); \
  }

#define CONFIG_SUBSECTION(what) \
  if (_subInit) { \
    ImGui::Separator(); \
  } else { \
    _subInit=true; \
  } \
  ImGui::PushFont(headFont); \
  ImGui::TextUnformatted(what); \
  ImGui::PopFont();

#define CONFIG_SECTION(what) \
  if (ImGui::BeginTabItem(what)) { \
    bool _subInit=false; \
    ImVec2 settingsViewSize=ImGui::GetContentRegionAvail(); \
    settingsViewSize.y-=ImGui::GetFrameHeight()+ImGui::GetStyle().WindowPadding.y; \
    if (ImGui::BeginChild("SettingsView",settingsViewSize,false))

#define END_SECTION } \
  ImGui::EndChild(); \
  ImGui::EndTabItem();

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

String stripName(String what) {
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
    if (ImGui::BeginTabBar("settingsTab")) {
      // NEW SETTINGS HERE
      CONFIG_SECTION(_("General")) {
        // SUBSECTION PROGRAM
        CONFIG_SUBSECTION(_("Program"));

#ifdef HAVE_LOCALE
        String curLocale=settings.locale;
        const char* localeRestart=locales[0][2];
        if (curLocale=="") {
          curLocale="<System>";
        } else {
          for (int i=1; locales[i][0]; i++) {
            if (strcmp(curLocale.c_str(),locales[i][1])==0) {
              curLocale=locales[i][0];
              break;
            }
          }
        }
        if (ImGui::BeginCombo(_("Language"),curLocale.c_str())) {
          for (int i=0; locales[i][0]; i++) {
            if (ImGui::Selectable(locales[i][0],strcmp(settings.locale.c_str(),locales[i][1])==0)) {
              settings.locale=locales[i][1];
              settingsChanged=true;
            }
            if (ImGui::IsItemHovered()) {
              ImGui::SetTooltip("%s",locales[i][2]);
            }
          }
          ImGui::EndCombo();
        } else {
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s",localeRestart);
          }
        }
#endif

        String curRenderBackend=settings.renderBackend.empty()?GUI_BACKEND_DEFAULT_NAME:settings.renderBackend;
        if (ImGui::BeginCombo(_("Render backend"),curRenderBackend.c_str())) {
#ifdef HAVE_RENDER_SDL
          if (ImGui::Selectable("SDL Renderer",curRenderBackend=="SDL")) {
            settings.renderBackend="SDL";
            settingsChanged=true;
          }
#endif
#ifdef HAVE_RENDER_DX11
          if (ImGui::Selectable("DirectX 11",curRenderBackend=="DirectX 11")) {
            settings.renderBackend="DirectX 11";
            settingsChanged=true;
          }
#endif
#ifdef HAVE_RENDER_DX9
          if (ImGui::Selectable("DirectX 9",curRenderBackend=="DirectX 9")) {
            settings.renderBackend="DirectX 9";
            settingsChanged=true;
          }
#endif
#ifdef HAVE_RENDER_METAL
          if (ImGui::Selectable("Metal",curRenderBackend=="Metal")) {
            settings.renderBackend="Metal";
            settingsChanged=true;
          }
#endif
#ifdef HAVE_RENDER_GL
#ifdef USE_GLES
          if (ImGui::Selectable("OpenGL ES 2.0",curRenderBackend=="OpenGL ES 2.0")) {
            settings.renderBackend="OpenGL ES 2.0";
            settingsChanged=true;
          }
#else
          if (ImGui::Selectable("OpenGL 3.0",curRenderBackend=="OpenGL 3.0")) {
            settings.renderBackend="OpenGL 3.0";
            settingsChanged=true;
          }
          if (ImGui::Selectable("OpenGL 2.0",curRenderBackend=="OpenGL 2.0")) {
            settings.renderBackend="OpenGL 2.0";
            settingsChanged=true;
          }
#endif
#endif
#ifdef HAVE_RENDER_GL1
          if (ImGui::Selectable("OpenGL 1.1",curRenderBackend=="OpenGL 1.1")) {
            settings.renderBackend="OpenGL 1.1";
            settingsChanged=true;
          }
#endif
          if (ImGui::Selectable("Software",curRenderBackend=="Software")) {
            settings.renderBackend="Software";
            settingsChanged=true;
          }
          ImGui::EndCombo();
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("you may need to restart Furnace for this setting to take effect."));
        }

        if (ImGui::TreeNode(_("Advanced render backend settings"))) {
          if (curRenderBackend=="SDL") {
            if (ImGui::BeginCombo(_("Render driver"),settings.renderDriver.empty()?_("Automatic"):settings.renderDriver.c_str())) {
              if (ImGui::Selectable(_("Automatic"),settings.renderDriver.empty())) {
                settings.renderDriver="";
                settingsChanged=true;
              }
              for (String& i: availRenderDrivers) {
                if (ImGui::Selectable(i.c_str(),i==settings.renderDriver)) {
                  settings.renderDriver=i;
                  settingsChanged=true;
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
              settingsChanged=true;
            }
            if (ImGui::InputInt(_("Green bits"),&settings.glGreenSize)) {
              if (settings.glGreenSize<0) settings.glGreenSize=0;
              if (settings.glGreenSize>32) settings.glGreenSize=32;
              settingsChanged=true;
            }
            if (ImGui::InputInt(_("Blue bits"),&settings.glBlueSize)) {
              if (settings.glBlueSize<0) settings.glBlueSize=0;
              if (settings.glBlueSize>32) settings.glBlueSize=32;
              settingsChanged=true;
            }
            if (ImGui::InputInt(_("Alpha bits"),&settings.glAlphaSize)) {
              if (settings.glAlphaSize<0) settings.glAlphaSize=0;
              if (settings.glAlphaSize>32) settings.glAlphaSize=32;
              settingsChanged=true;
            }
            if (ImGui::InputInt(_("Color depth"),&settings.glDepthSize)) {
              if (settings.glDepthSize<0) settings.glDepthSize=0;
              if (settings.glDepthSize>128) settings.glDepthSize=128;
              settingsChanged=true;
            }
            
            bool glSetBSB=settings.glSetBS;
            if (ImGui::Checkbox(_("Set stencil and buffer sizes"),&glSetBSB)) {
              settings.glSetBS=glSetBSB;
              settingsChanged=true;
            }

            if (settings.glSetBS) {
              if (ImGui::InputInt(_("Stencil buffer size"),&settings.glStencilSize)) {
                if (settings.glStencilSize<0) settings.glStencilSize=0;
                if (settings.glStencilSize>32) settings.glStencilSize=32;
                settingsChanged=true;
              }
              if (ImGui::InputInt(_("Buffer size"),&settings.glBufferSize)) {
                if (settings.glBufferSize<0) settings.glBufferSize=0;
                if (settings.glBufferSize>128) settings.glBufferSize=128;
                settingsChanged=true;
              }
            }

            bool glDoubleBufferB=settings.glDoubleBuffer;
            if (ImGui::Checkbox(_("Double buffer"),&glDoubleBufferB)) {
              settings.glDoubleBuffer=glDoubleBufferB;
              settingsChanged=true;
            }

            ImGui::TextWrapped(_("the following values are common (in red, green, blue, alpha order):\n- 24 bits: 8, 8, 8, 0\n- 16 bits: 5, 6, 5, 0\n- 32 bits (with alpha): 8, 8, 8, 8\n- 30 bits (deep): 10, 10, 10, 0"));
          } else {
            ImGui::Text(_("nothing to configure"));
          }
          ImGui::TreePop();
        }

        ImGui::TextWrapped(_("current backend: %s\n%s\n%s\n%s"),rend->getBackendName(),rend->getVendorName(),rend->getDeviceName(),rend->getAPIVersion());

        bool vsyncB=settings.vsync;
        if (ImGui::Checkbox(_("VSync"),&vsyncB)) {
          settings.vsync=vsyncB;
          settingsChanged=true;
          if (rend!=NULL) {
            rend->setSwapInterval(settings.vsync);
          }
        }

        if (ImGui::SliderInt(_("Frame rate limit"),&settings.frameRateLimit,0,250,settings.frameRateLimit==0?_("Unlimited"):"%d")) {
          settingsChanged=true;
        }
        if (settings.frameRateLimit<0) settings.frameRateLimit=0;
        if (settings.frameRateLimit>1000) settings.frameRateLimit=1000;
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("only applies when VSync is disabled."));
        }

        bool displayRenderTimeB=settings.displayRenderTime;
        if (ImGui::Checkbox(_("Display render time"),&displayRenderTimeB)) {
          settings.displayRenderTime=displayRenderTimeB;
          settingsChanged=true;
        }

        if (settings.renderBackend!="Metal") {
          bool renderClearPosB=settings.renderClearPos;
          if (ImGui::Checkbox(_("Late render clear"),&renderClearPosB)) {
            settings.renderClearPos=renderClearPosB;
            settingsChanged=true;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("calls rend->clear() after rend->present(). might reduce UI latency by one frame in some drivers."));
          }
        }

        bool powerSaveB=settings.powerSave;
        if (ImGui::Checkbox(_("Power-saving mode"),&powerSaveB)) {
          settings.powerSave=powerSaveB;
          settingsChanged=true;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("saves power by lowering the frame rate to 2fps when idle.\nmay cause issues under Mesa drivers!"));
        }

#ifndef IS_MOBILE
        bool noThreadedInputB=settings.noThreadedInput;
        if (ImGui::Checkbox(_("Disable threaded input (restart after changing!)"),&noThreadedInputB)) {
          settings.noThreadedInput=noThreadedInputB;
          settingsChanged=true;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("threaded input processes key presses for note preview on a separate thread (on supported platforms), which reduces latency.\nhowever, crashes have been reported when threaded input is on. enable this option if that is the case."));
        }
#endif

        bool eventDelayB=settings.eventDelay;
        if (ImGui::Checkbox(_("Enable event delay"),&eventDelayB)) {
          settings.eventDelay=eventDelayB;
          settingsChanged=true;
          applyUISettings(false);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("may cause issues with high-polling-rate mice when previewing notes."));
        }

#ifdef IS_MOBILE
        // SUBSECTION VIBRATION
        CONFIG_SUBSECTION(_("Vibration"));

        if (ImGui::SliderFloat(_("Strength"),&settings.vibrationStrength,0.0f,1.0f)) {
          if (settings.vibrationStrength<0.0f) settings.vibrationStrength=0.0f;
          if (settings.vibrationStrength>1.0f) settings.vibrationStrength=1.0f;
          settingsChanged=true;
        }

        if (ImGui::SliderInt(_("Length"),&settings.vibrationLength,10,500)) {
          if (settings.vibrationLength<10) settings.vibrationLength=10;
          if (settings.vibrationLength>500) settings.vibrationLength=500;
          settingsChanged=true;
        }
#endif

        // SUBSECTION FILE
        CONFIG_SUBSECTION(_("File"));

#ifndef FLATPAK_WORKAROUNDS
        bool sysFileDialogB=settings.sysFileDialog;
        if (ImGui::Checkbox(_("Use system file picker"),&sysFileDialogB)) {
          settings.sysFileDialog=sysFileDialogB;
          settingsChanged=true;
        }
#endif

        if (ImGui::InputInt(_("Number of recent files"),&settings.maxRecentFile,1,5)) {
          if (settings.maxRecentFile<0) settings.maxRecentFile=0;
          if (settings.maxRecentFile>30) settings.maxRecentFile=30;
          settingsChanged=true;
        }

        bool compressB=settings.compress;
        if (ImGui::Checkbox(_("Compress when saving"),&compressB)) {
          settings.compress=compressB;
          settingsChanged=true;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("use zlib to compress saved songs."));
        }

        bool saveUnusedPatternsB=settings.saveUnusedPatterns;
        if (ImGui::Checkbox(_("Save unused patterns"),&saveUnusedPatternsB)) {
          settings.saveUnusedPatterns=saveUnusedPatternsB;
          settingsChanged=true;
        }

        bool newPatternFormatB=settings.newPatternFormat;
        if (ImGui::Checkbox(_("Use new pattern format when saving"),&newPatternFormatB)) {
          settings.newPatternFormat=newPatternFormatB;
          settingsChanged=true;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("use a packed format which saves space when saving songs.\ndisable if you need compatibility with older Furnace and/or tools\nwhich do not support this format."));
        }

        bool noDMFCompatB=settings.noDMFCompat;
        if (ImGui::Checkbox(_("Don't apply compatibility flags when loading .dmf"),&noDMFCompatB)) {
          settings.noDMFCompat=noDMFCompatB;
          settingsChanged=true;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("do not report any issues arising from the use of this option!"));
        }

        ImGui::Text(_("Play after opening song:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("No##pol0"),settings.playOnLoad==0)) {
          settings.playOnLoad=0;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Only if already playing##pol1"),settings.playOnLoad==1)) {
          settings.playOnLoad=1;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Yes##pol0"),settings.playOnLoad==2)) {
          settings.playOnLoad=2;
          settingsChanged=true;
        }
        ImGui::Unindent();

        ImGui::Text(_("Audio export loop/fade out time:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("Set to these values on start-up:##fot0"),settings.persistFadeOut==0)) {
          settings.persistFadeOut=0;
          settingsChanged=true;
        }
        ImGui::BeginDisabled(settings.persistFadeOut);
        ImGui::Indent();
        if (ImGui::InputInt(_("Loops"),&settings.exportLoops,1,2)) {
          if (settings.exportLoops<0) settings.exportLoops=0;
          audioExportOptions.loops=settings.exportLoops;
          settingsChanged=true;
        }
        if (ImGui::InputDouble(_("Fade out (seconds)"),&settings.exportFadeOut,1.0,2.0,"%.1f")) {
          if (settings.exportFadeOut<0.0) settings.exportFadeOut=0.0;
          audioExportOptions.fadeOut=settings.exportFadeOut;
          settingsChanged=true;
        }
        ImGui::Unindent();
        ImGui::EndDisabled();
        if (ImGui::RadioButton(_("Remember last values##fot1"),settings.persistFadeOut==1)) {
          settings.persistFadeOut=1;
          settingsChanged=true;
        }
        ImGui::Unindent();

        bool writeInsNamesB=settings.writeInsNames;
        if (ImGui::Checkbox(_("Store instrument name in .fui"),&writeInsNamesB)) {
          settings.writeInsNames=writeInsNamesB;
          settingsChanged=true;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("when enabled, saving an instrument will store its name.\nthis may increase file size."));
        }

        bool readInsNamesB=settings.readInsNames;
        if (ImGui::Checkbox(_("Load instrument name from .fui"),&readInsNamesB)) {
          settings.readInsNames=readInsNamesB;
          settingsChanged=true;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("when enabled, loading an instrument will use the stored name (if present).\notherwise, it will use the file name."));
        }

        bool autoFillSaveB=settings.autoFillSave;
        if (ImGui::Checkbox(_("Auto-fill file name when saving"),&autoFillSaveB)) {
          settings.autoFillSave=autoFillSaveB;
          settingsChanged=true;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("fill the file name field with an appropriate file name when saving or exporting."));
        }

        // SUBSECTION NEW SONG
        CONFIG_SUBSECTION(_("New Song"));
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
          settingsChanged=true;
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
          settingsChanged=true;
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
          settingsChanged=true;
        }

        ImGui::AlignTextToFramePadding();
        ImGui::Text(_("Name"));
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputText("##InitSysName",&settings.initialSysName)) settingsChanged=true;

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
              settingsChanged=true;

              ImGui::CloseCurrentPopup();
            }

            ImGui::EndCombo();
          }

          ImGui::SameLine();
          if (ImGui::Checkbox(_("Invert"),&doInvert)) {
            sysVol=-sysVol;
            settings.initialSys.set(fmt::sprintf("vol%d",i),sysVol);
            settingsChanged=true;
          }
          ImGui::SameLine();
          //ImGui::BeginDisabled(settings.initialSys.size()<=4);
          pushDestColor();
          if (ImGui::Button(ICON_FA_MINUS "##InitSysRemove")) {
            doRemove=i;
            settingsChanged=true;
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
            settingsChanged=true;
          } rightClickable
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x-ImGui::GetFrameHeightWithSpacing()*2.0-ImGui::GetStyle().ItemSpacing.x*2.0);
          if (CWSliderFloat(_("Panning"),&sysPan,-1.0f,1.0f)) {
            if (sysPan<-1.0f) sysPan=-1.0f;
            if (sysPan>1.0f) sysPan=1.0f;
            settings.initialSys.set(fmt::sprintf("pan%d",i),(float)sysPan);
            settingsChanged=true;
          } rightClickable
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x-ImGui::GetFrameHeightWithSpacing()*2.0-ImGui::GetStyle().ItemSpacing.x*2.0);
          if (CWSliderFloat(_("Front/Rear"),&sysPanFR,-1.0f,1.0f)) {
            if (sysPanFR<-1.0f) sysPanFR=-1.0f;
            if (sysPanFR>1.0f) sysPanFR=1.0f;
            settings.initialSys.set(fmt::sprintf("fr%d",i),(float)sysPanFR);
            settingsChanged=true;
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
            settingsChanged=true;
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

        ImGui::Text(_("When creating new song:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("Display system preset selector##NSB0"),settings.newSongBehavior==0)) {
          settings.newSongBehavior=0;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Start with initial system##NSB1"),settings.newSongBehavior==1)) {
          settings.newSongBehavior=1;
          settingsChanged=true;
        }
        if (ImGui::InputText(_("Default author name"), &settings.defaultAuthorName)) settingsChanged=true;
        ImGui::Unindent();

        // SUBSECTION START-UP
        CONFIG_SUBSECTION(_("Start-up"));
        ImGui::Text(_("Play intro on start-up:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("No##pis0"),settings.alwaysPlayIntro==0)) {
          settings.alwaysPlayIntro=0;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Short##pis1"),settings.alwaysPlayIntro==1)) {
          settings.alwaysPlayIntro=1;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Full (short when loading song)##pis2"),settings.alwaysPlayIntro==2)) {
          settings.alwaysPlayIntro=2;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Full (always)##pis3"),settings.alwaysPlayIntro==3)) {
          settings.alwaysPlayIntro=3;
          settingsChanged=true;
        }
        ImGui::Unindent();

        bool disableFadeInB=settings.disableFadeIn;
        if (ImGui::Checkbox(_("Disable fade-in during start-up"),&disableFadeInB)) {
          settings.disableFadeIn=disableFadeInB;
          settingsChanged=true;
        }

        // SUBSECTION BEHAVIOR
        CONFIG_SUBSECTION(_("Behavior"));
        bool blankInsB=settings.blankIns;
        if (ImGui::Checkbox(_("New instruments are blank"),&blankInsB)) {
          settings.blankIns=blankInsB;
          settingsChanged=true;
        }

        // SUBSECTION CONFIGURATION
        CONFIG_SUBSECTION(_("Configuration"));
        if (ImGui::Button(_("Import"))) {
          openFileDialog(GUI_FILE_IMPORT_CONFIG);
        }
        ImGui::SameLine();
        if (ImGui::Button(_("Export"))) {
          openFileDialog(GUI_FILE_EXPORT_CONFIG);
        }
        pushDestColor();
        if (ImGui::Button(_("Factory Reset"))) {
          showWarning(_("Are you sure you want to reset all Furnace settings?\nYou must restart Furnace after doing so."),GUI_WARN_RESET_CONFIG);
        }
        popDestColor();

        // SUBSECTION IMPORT
        CONFIG_SUBSECTION(_("Import"));
        bool s3mOPL3B=settings.s3mOPL3;
        if (ImGui::Checkbox(_("Use OPL3 instead of OPL2 for S3M import"),&s3mOPL3B)) {
          settings.s3mOPL3=s3mOPL3B;
          settingsChanged=true;
        }

#ifdef ANDROID
        // SUBSECTION ANDROID
        CONFIG_SUBSECTION(_("Android"));
        bool backgroundPlayB=settings.backgroundPlay;
        if (ImGui::Checkbox(_("Enable background playback (restart!)"),&backgroundPlayB)) {
          settings.backgroundPlay=backgroundPlayB;
          settingsChanged=true;
        }
#endif

        END_SECTION;
      }
      CONFIG_SECTION(_("Audio")) {
        // SUBSECTION OUTPUT
        CONFIG_SUBSECTION(_("Output"));
        if (ImGui::BeginTable("##Output",2)) {
          ImGui::TableSetupColumn("##Label",ImGuiTableColumnFlags_WidthFixed);
          ImGui::TableSetupColumn("##Combo",ImGuiTableColumnFlags_WidthStretch);
#if defined(HAVE_JACK) || defined(HAVE_PA)
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text(_("Backend"));
          ImGui::TableNextColumn();
          int prevAudioEngine=settings.audioEngine;
          if (ImGui::BeginCombo("##Backend",audioBackends[settings.audioEngine])) {
#ifdef HAVE_JACK
            if (ImGui::Selectable("JACK",settings.audioEngine==DIV_AUDIO_JACK)) {
              settings.audioEngine=DIV_AUDIO_JACK;
              settingsChanged=true;
            }
#endif
            if (ImGui::Selectable("SDL",settings.audioEngine==DIV_AUDIO_SDL)) {
              settings.audioEngine=DIV_AUDIO_SDL;
              settingsChanged=true;
            }
#ifdef HAVE_PA
            if (ImGui::Selectable("PortAudio",settings.audioEngine==DIV_AUDIO_PORTAUDIO)) {
              settings.audioEngine=DIV_AUDIO_PORTAUDIO;
              settingsChanged=true;
            }
#endif
            if (settings.audioEngine!=prevAudioEngine) {
              audioEngineChanged=true;
              settings.audioDevice="";
              settings.audioChans=2;
            }
            ImGui::EndCombo();
          }
#endif

          if (settings.audioEngine==DIV_AUDIO_SDL) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text(_("Driver"));
            ImGui::TableNextColumn();
            if (ImGui::BeginCombo("##SDLADriver",settings.sdlAudioDriver.empty()?_("Automatic"):settings.sdlAudioDriver.c_str())) {
              if (ImGui::Selectable(_("Automatic"),settings.sdlAudioDriver.empty())) {
                settings.sdlAudioDriver="";
                settingsChanged=true;
              }
              for (String& i: availAudioDrivers) {
                if (ImGui::Selectable(i.c_str(),i==settings.sdlAudioDriver)) {
                  settings.sdlAudioDriver=i;
                  settingsChanged=true;
                }
              }
              ImGui::EndCombo();
            }
            if (ImGui::IsItemHovered()) {
              ImGui::SetTooltip(_("you may need to restart Furnace for this setting to take effect."));
            }
          }

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text(_("Device"));
          ImGui::TableNextColumn();
          if (audioEngineChanged) {
            ImGui::BeginDisabled();
            if (ImGui::BeginCombo("##AudioDevice",_("<click on OK or Apply first>"))) {
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
            if (ImGui::BeginCombo("##AudioDevice",audioDevName.c_str())) {
              if (ImGui::Selectable(_("<System default>"),settings.audioDevice.empty())) {
                settings.audioDevice="";
                settingsChanged=true;
              }
              for (String& i: e->getAudioDevices()) {
                if (ImGui::Selectable(i.c_str(),i==settings.audioDevice)) {
                  settings.audioDevice=i;
                  settingsChanged=true;
                }
              }
              ImGui::EndCombo();
            }
          }

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text(_("Sample rate"));
          ImGui::TableNextColumn();
          String sr=fmt::sprintf("%d",settings.audioRate);
          if (ImGui::BeginCombo("##SampleRate",sr.c_str())) {
            SAMPLE_RATE_SELECTABLE(8000);
            SAMPLE_RATE_SELECTABLE(16000);
            SAMPLE_RATE_SELECTABLE(22050);
            SAMPLE_RATE_SELECTABLE(32000);
            SAMPLE_RATE_SELECTABLE(44100);
            SAMPLE_RATE_SELECTABLE(48000);
            SAMPLE_RATE_SELECTABLE(88200);
            SAMPLE_RATE_SELECTABLE(96000);
            SAMPLE_RATE_SELECTABLE(192000);
            ImGui::EndCombo();
          }

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text(_("Outputs"));
          ImGui::TableNextColumn();
          if (ImGui::InputInt("##AudioChansI",&settings.audioChans,1,2)) {
            if (settings.audioChans<1) settings.audioChans=1;
            if (settings.audioChans>16) settings.audioChans=16;
            settingsChanged=true;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("common values:\n- 1 for mono\n- 2 for stereo"));
          }

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text(_("Buffer size"));
          ImGui::TableNextColumn();
          String bs=fmt::sprintf(_("%d (latency: ~%.1fms)"),settings.audioBufSize,2000.0*(double)settings.audioBufSize/(double)MAX(1,settings.audioRate));
          if (ImGui::BeginCombo("##BufferSize",bs.c_str())) {
            BUFFER_SIZE_SELECTABLE(64);
            BUFFER_SIZE_SELECTABLE(128);
            BUFFER_SIZE_SELECTABLE(256);
            BUFFER_SIZE_SELECTABLE(512);
            BUFFER_SIZE_SELECTABLE(1024);
            BUFFER_SIZE_SELECTABLE(2048);
            ImGui::EndCombo();
          }
          ImGui::EndTable();
        }

        bool renderPoolThreadsB=(settings.renderPoolThreads>0);
        if (ImGui::Checkbox(_("Multi-threaded (EXPERIMENTAL)"),&renderPoolThreadsB)) {
          if (renderPoolThreadsB) {
            settings.renderPoolThreads=2;
          } else {
            settings.renderPoolThreads=0;
          }
          settingsChanged=true;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("runs chip emulation on separate threads.\nmay increase performance when using heavy emulation cores.\n\nwarnings:\n- experimental!\n- only useful on multi-chip songs."));
        }

        if (renderPoolThreadsB) {
          pushWarningColor(settings.renderPoolThreads>cpuCores,settings.renderPoolThreads>cpuCores);
          if (ImGui::InputInt(_("Number of threads"),&settings.renderPoolThreads)) {
            if (settings.renderPoolThreads<2) settings.renderPoolThreads=2;
            if (settings.renderPoolThreads>32) settings.renderPoolThreads=32;
            settingsChanged=true;
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

        bool lowLatencyB=settings.lowLatency;
        if (ImGui::Checkbox(_("Low-latency mode"),&lowLatencyB)) {
          settings.lowLatency=lowLatencyB;
          settingsChanged=true;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("reduces latency by running the engine faster than the tick rate.\nuseful for live playback/jam mode.\n\nwarning: only enable if your buffer size is small (10ms or less)."));
        }

        bool forceMonoB=settings.forceMono;
        if (ImGui::Checkbox(_("Force mono audio"),&forceMonoB)) {
          settings.forceMono=forceMonoB;
          settingsChanged=true;
        }

        if (settings.audioEngine==DIV_AUDIO_PORTAUDIO) {
          if (settings.audioDevice.find("[Windows WASAPI] ")==0) {
            bool wasapiExB=settings.wasapiEx;
            if (ImGui::Checkbox(_("Exclusive mode"),&wasapiExB)) {
              settings.wasapiEx=wasapiExB;
              settingsChanged=true;
            }
          }
        }

        TAAudioDesc& audioWant=e->getAudioDescWant();
        TAAudioDesc& audioGot=e->getAudioDescGot();

#ifdef HAVE_LOCALE
        ImGui::Text(ngettext("want: %d samples @ %.0fHz (%d channel)","want: %d samples @ %.0fHz (%d channels)",audioWant.outChans),audioWant.bufsize,audioWant.rate,audioWant.outChans);
        ImGui::Text(ngettext("got: %d samples @ %.0fHz (%d channel)","got: %d samples @ %.0fHz (%d channels)",audioGot.outChans),audioGot.bufsize,audioGot.rate,audioGot.outChans);
#else
        ImGui::Text(_GN("want: %d samples @ %.0fHz (%d channel)","want: %d samples @ %.0fHz (%d channels)",audioWant.outChans),audioWant.bufsize,audioWant.rate,audioWant.outChans);
        ImGui::Text(_GN("got: %d samples @ %.0fHz (%d channel)","got: %d samples @ %.0fHz (%d channels)",audioGot.outChans),audioGot.bufsize,audioGot.rate,audioGot.outChans);
#endif

        // SUBSECTION MIXING
        CONFIG_SUBSECTION(_("Mixing"));
        ImGui::AlignTextToFramePadding();
        ImGui::Text(_("Quality"));
        ImGui::SameLine();
        if (ImGui::Combo("##Quality",&settings.audioQuality,LocalizedComboGetter,audioQualities,2)) settingsChanged=true;

        bool clampSamplesB=settings.clampSamples;
        if (ImGui::Checkbox(_("Software clipping"),&clampSamplesB)) {
          settings.clampSamples=clampSamplesB;
          settingsChanged=true;
        }

        bool audioHiPassB=settings.audioHiPass;
        if (ImGui::Checkbox(_("DC offset correction"),&audioHiPassB)) {
          settings.audioHiPass=audioHiPassB;
          settingsChanged=true;
        }

        // SUBSECTION METRONOME
        CONFIG_SUBSECTION(_("Metronome"));
        ImGui::AlignTextToFramePadding();
        ImGui::Text(_("Volume"));
        ImGui::SameLine();
        if (ImGui::SliderInt("##MetroVol",&settings.metroVol,0,200,"%d%%")) {
          if (settings.metroVol<0) settings.metroVol=0;
          if (settings.metroVol>200) settings.metroVol=200;
          e->setMetronomeVol(((float)settings.metroVol)/100.0f);
          settingsChanged=true;
        }

        // SUBSECTION SAMPLE PREVIEW
        CONFIG_SUBSECTION(_("Sample preview"));
        ImGui::AlignTextToFramePadding();
        ImGui::Text(_("Volume"));
        ImGui::SameLine();
        if (ImGui::SliderInt("##SampleVol",&settings.sampleVol,0,100,"%d%%")) {
          if (settings.sampleVol<0) settings.sampleVol=0;
          if (settings.sampleVol>100) settings.sampleVol=100;
          e->setSamplePreviewVol(((float)settings.sampleVol)/100.0f);
          settingsChanged=true;
        }

        END_SECTION;
      }
      CONFIG_SECTION(_("MIDI")) {
        // SUBSECTION MIDI INPUT
        CONFIG_SUBSECTION(_("MIDI input"));
        ImGui::AlignTextToFramePadding();
        ImGui::Text(_("MIDI input"));
        ImGui::SameLine();
        String midiInName=settings.midiInDevice.empty()?_("<disabled>"):settings.midiInDevice;
        bool hasToReloadMidi=false;
        if (ImGui::BeginCombo("##MidiInDevice",midiInName.c_str())) {
          if (ImGui::Selectable(_("<disabled>"),settings.midiInDevice.empty())) {
            settings.midiInDevice="";
            hasToReloadMidi=true;
            settingsChanged=true;
          }
          for (String& i: e->getMidiIns()) {
            if (ImGui::Selectable(i.c_str(),i==settings.midiInDevice)) {
              settings.midiInDevice=i;
              hasToReloadMidi=true;
              settingsChanged=true;
            }
          }
          ImGui::EndCombo();
        }

        ImGui::SameLine();
        if (ImGui::Button(_("Re-scan MIDI devices"))) {
          e->rescanMidiDevices();
          audioEngineChanged=true;
          settingsChanged=false;
        }

        if (hasToReloadMidi) {
          midiMap.read(e->getConfigPath()+DIR_SEPARATOR_STR+"midiIn_"+stripName(settings.midiInDevice)+".cfg");
          midiMap.compile();
        }

        if (ImGui::Checkbox(_("Note input"),&midiMap.noteInput)) settingsChanged=true;
        if (ImGui::Checkbox(_("Velocity input"),&midiMap.volInput)) settingsChanged=true;
        // TODO
        //ImGui::Checkbox(_("Use raw velocity value (don't map from linear to log)"),&midiMap.rawVolume);
        //ImGui::Checkbox(_("Polyphonic/chord input"),&midiMap.polyInput);
        if (ImGui::Checkbox(_("Map MIDI channels to direct channels"),&midiMap.directChannel)) {
          e->setMidiDirect(midiMap.directChannel);
          e->setMidiDirectProgram(midiMap.directChannel && midiMap.directProgram);
          settingsChanged=true;
        }
        if (midiMap.directChannel) {
          if (ImGui::Checkbox(_("Program change pass-through"),&midiMap.directProgram)) {
            e->setMidiDirectProgram(midiMap.directChannel && midiMap.directProgram);
            settingsChanged=true;
          }
        }
        if (ImGui::Checkbox(_("Map Yamaha FM voice data to instruments"),&midiMap.yamahaFMResponse)) settingsChanged=true;
        if (!(midiMap.directChannel && midiMap.directProgram)) {
          if (ImGui::Checkbox(_("Program change is instrument selection"),&midiMap.programChange)) settingsChanged=true;
        }
        //ImGui::Checkbox(_("Listen to MIDI clock"),&midiMap.midiClock);
        //ImGui::Checkbox(_("Listen to MIDI time code"),&midiMap.midiTimeCode);
        if (ImGui::Combo(_("Value input style"),&midiMap.valueInputStyle,LocalizedComboGetter,valueInputStyles,7)) settingsChanged=true;
        if (midiMap.valueInputStyle>3) {
          if (midiMap.valueInputStyle==6) {
            if (ImGui::InputInt(_("Control##valueCCS"),&midiMap.valueInputControlSingle,1,16)) {
              if (midiMap.valueInputControlSingle<0) midiMap.valueInputControlSingle=0;
              if (midiMap.valueInputControlSingle>127) midiMap.valueInputControlSingle=127;
              settingsChanged=true;
            }
          } else {
            if (ImGui::InputInt((midiMap.valueInputStyle==4)?_("CC of upper nibble##valueCC1"):_("MSB CC##valueCC1"),&midiMap.valueInputControlMSB,1,16)) {
              if (midiMap.valueInputControlMSB<0) midiMap.valueInputControlMSB=0;
              if (midiMap.valueInputControlMSB>127) midiMap.valueInputControlMSB=127;
              settingsChanged=true;
            }
            if (ImGui::InputInt((midiMap.valueInputStyle==4)?_("CC of lower nibble##valueCC2"):_("LSB CC##valueCC2"),&midiMap.valueInputControlLSB,1,16)) {
              if (midiMap.valueInputControlLSB<0) midiMap.valueInputControlLSB=0;
              if (midiMap.valueInputControlLSB>127) midiMap.valueInputControlLSB=127;
              settingsChanged=true;
            }
          }
        }
        if (ImGui::TreeNode(_("Per-column control change"))) {
          for (int i=0; i<18; i++) {
            ImGui::PushID(i);
            if (ImGui::Combo(specificControls[i],&midiMap.valueInputSpecificStyle[i],LocalizedComboGetter,valueSInputStyles,4)) settingsChanged=true;
            if (midiMap.valueInputSpecificStyle[i]>0) {
              ImGui::Indent();
              if (midiMap.valueInputSpecificStyle[i]==3) {
                if (ImGui::InputInt(_("Control##valueCCS"),&midiMap.valueInputSpecificSingle[i],1,16)) {
                  if (midiMap.valueInputSpecificSingle[i]<0) midiMap.valueInputSpecificSingle[i]=0;
                  if (midiMap.valueInputSpecificSingle[i]>127) midiMap.valueInputSpecificSingle[i]=127;
                  settingsChanged=true;
                }
              } else {
                if (ImGui::InputInt((midiMap.valueInputSpecificStyle[i]==4)?_("CC of upper nibble##valueCC1"):_("MSB CC##valueCC1"),&midiMap.valueInputSpecificMSB[i],1,16)) {
                  if (midiMap.valueInputSpecificMSB[i]<0) midiMap.valueInputSpecificMSB[i]=0;
                  if (midiMap.valueInputSpecificMSB[i]>127) midiMap.valueInputSpecificMSB[i]=127;
                  settingsChanged=true;
                }
                if (ImGui::InputInt((midiMap.valueInputSpecificStyle[i]==4)?_("CC of lower nibble##valueCC2"):_("LSB CC##valueCC2"),&midiMap.valueInputSpecificLSB[i],1,16)) {
                  if (midiMap.valueInputSpecificLSB[i]<0) midiMap.valueInputSpecificLSB[i]=0;
                  if (midiMap.valueInputSpecificLSB[i]>127) midiMap.valueInputSpecificLSB[i]=127;
                  settingsChanged=true;
                }
              }
              ImGui::Unindent();
            }
            ImGui::PopID();
          }
          ImGui::TreePop();
        }
        if (ImGui::SliderFloat(_("Volume curve"),&midiMap.volExp,0.01,8.0,"%.2f")) {
          if (midiMap.volExp<0.01) midiMap.volExp=0.01;
          if (midiMap.volExp>8.0) midiMap.volExp=8.0;
          e->setMidiVolExp(midiMap.volExp);
          settingsChanged=true;
        } rightClickable
        float curve[128];
        for (int i=0; i<128; i++) {
          curve[i]=(int)(pow((double)i/127.0,midiMap.volExp)*127.0);
        }
        ImGui::PlotLines("##VolCurveDisplay",curve,128,0,_("Volume curve"),0.0,127.0,ImVec2(200.0f*dpiScale,200.0f*dpiScale));

        ImGui::AlignTextToFramePadding();
        ImGui::Text(_("Actions:"));
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_PLUS "##AddAction")) {
          midiMap.binds.push_back(MIDIBind());
          settingsChanged=true;
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_EXTERNAL_LINK "##AddLearnAction")) {
          midiMap.binds.push_back(MIDIBind());
          learning=midiMap.binds.size()-1;
          settingsChanged=true;
        }
        if (learning!=-1) {
          ImGui::SameLine();
          ImGui::Text(_("(learning! press a button or move a slider/knob/something on your device.)"));
        }

        if (ImGui::BeginTable("MIDIActions",7)) {
          ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.2);
          ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.1);
          ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.3);
          ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthStretch,0.2);
          ImGui::TableSetupColumn("c4",ImGuiTableColumnFlags_WidthStretch,0.5);
          ImGui::TableSetupColumn("c5",ImGuiTableColumnFlags_WidthFixed);
          ImGui::TableSetupColumn("c6",ImGuiTableColumnFlags_WidthFixed);

          ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
          ImGui::TableNextColumn();
          ImGui::Text(_("Type"));
          ImGui::TableNextColumn();
          ImGui::Text(_("Channel"));
          ImGui::TableNextColumn();
          ImGui::Text(_("Note/Control"));
          ImGui::TableNextColumn();
          ImGui::Text(_("Velocity/Value"));
          ImGui::TableNextColumn();
          ImGui::Text(_("Action"));
          ImGui::TableNextColumn();
          ImGui::TableNextColumn();

          for (size_t i=0; i<midiMap.binds.size(); i++) {
            MIDIBind& bind=midiMap.binds[i];
            char bindID[1024];
            ImGui::PushID(i);
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::BeginCombo("##BType",messageTypes[bind.type])) {
              for (int j=8; j<15; j++) {
                if (ImGui::Selectable(messageTypes[j],bind.type==j)) {
                  bind.type=j;
                  settingsChanged=true;
                }
              }
              ImGui::EndCombo();
            }

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::BeginCombo("##BChannel",messageChannels[bind.channel])) {
              if (ImGui::Selectable(messageChannels[16],bind.channel==16)) {
                bind.channel=16;
                settingsChanged=true;
              }
              for (int j=0; j<16; j++) {
                if (ImGui::Selectable(messageChannels[j],bind.channel==j)) {
                  bind.channel=j;
                  settingsChanged=true;
                }
              }
              ImGui::EndCombo();
            }

            ImGui::TableNextColumn();
            if (bind.data1==128) {
              snprintf(bindID,1024,_("Any"));
            } else {
              const char* nName="???";
              if ((bind.data1+60)>0 && (bind.data1+60)<180) {
                nName=noteNames[bind.data1+60];
              }
              snprintf(bindID,1024,"%d (0x%.2X, %s)",bind.data1,bind.data1,nName);
            }
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::BeginCombo("##BValue1",bindID)) {
              if (ImGui::Selectable(_("Any"),bind.data1==128)) {
                bind.data1=128;
                settingsChanged=true;
              }
              for (int j=0; j<128; j++) {
                const char* nName="???";
                if ((j+60)>0 && (j+60)<180) {
                  nName=noteNames[j+60];
                }
                snprintf(bindID,1024,"%d (0x%.2X, %s)##BV1_%d",j,j,nName,j);
                if (ImGui::Selectable(bindID,bind.data1==j)) {
                  bind.data1=j;
                  settingsChanged=true;
                }
              }
              ImGui::EndCombo();
            }

            ImGui::TableNextColumn();
            if (bind.data2==128) {
              snprintf(bindID,1024,_("Any"));
            } else {
              snprintf(bindID,1024,"%d (0x%.2X)",bind.data2,bind.data2);
            }
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::BeginCombo("##BValue2",bindID)) {
              if (ImGui::Selectable(_("Any"),bind.data2==128)) {
                bind.data2=128;
                settingsChanged=true;
              }
              for (int j=0; j<128; j++) {
                snprintf(bindID,1024,"%d (0x%.2X)##BV2_%d",j,j,j);
                if (ImGui::Selectable(bindID,bind.data2==j)) {
                  bind.data2=j;
                  settingsChanged=true;
                }
              }
              ImGui::EndCombo();
            }

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::BeginCombo("##BAction",(bind.action==0)?_("--none--"):guiActions[bind.action].friendlyName)) {
              if (ImGui::Selectable(_("--none--"),bind.action==0)) {
                bind.action=0;
                settingsChanged=true;
              }
              for (int j=0; j<GUI_ACTION_MAX; j++) {
                if (strcmp(guiActions[j].friendlyName,"")==0) continue;
                if (strstr(guiActions[j].friendlyName,"---")==guiActions[j].friendlyName) {
                  ImGui::TextUnformatted(guiActions[j].friendlyName);
                } else {
                  snprintf(bindID,1024,"%s##BA_%d",_(guiActions[j].friendlyName),j);
                  if (ImGui::Selectable(bindID,bind.action==j)) {
                    bind.action=j;
                    settingsChanged=true;
                  }
                }
              }
              ImGui::EndCombo();
            }

            ImGui::TableNextColumn();
            pushToggleColors(learning==(int)i);
            if (ImGui::Button((learning==(int)i)?(_("waiting...##BLearn")):(_("Learn##BLearn")))) {
              if (learning==(int)i) {
                learning=-1;
              } else {
                learning=i;
              }
              settingsChanged=true;
            }
            popToggleColors();

            ImGui::TableNextColumn();
            if (ImGui::Button(ICON_FA_TIMES "##BRemove")) {
              midiMap.binds.erase(midiMap.binds.begin()+i);
              if (learning==(int)i) learning=-1;
              i--;
              settingsChanged=true;
            }

            ImGui::PopID();
          }
          ImGui::EndTable();
        }

        // SUBSECTION MIDI OUTPUT
        CONFIG_SUBSECTION(_("MIDI output"));
        ImGui::AlignTextToFramePadding();
        ImGui::Text(_("MIDI output"));
        ImGui::SameLine();
        String midiOutName=settings.midiOutDevice.empty()?_("<disabled>"):settings.midiOutDevice;
        if (ImGui::BeginCombo("##MidiOutDevice",midiOutName.c_str())) {
          if (ImGui::Selectable(_("<disabled>"),settings.midiOutDevice.empty())) {
            settings.midiOutDevice="";
            settingsChanged=true;
          }
          for (String& i: e->getMidiIns()) {
            if (ImGui::Selectable(i.c_str(),i==settings.midiOutDevice)) {
              settings.midiOutDevice=i;
              settingsChanged=true;
            }
          }
          ImGui::EndCombo();
        }

        ImGui::Text(_("Output mode:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("Off (use for TX81Z)"),settings.midiOutMode==0)) {
          settings.midiOutMode=0;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Melodic"),settings.midiOutMode==1)) {
          settings.midiOutMode=1;
          settingsChanged=true;
        }
        /*
        if (ImGui::RadioButton(_("Light Show (use for Launchpad)"),settings.midiOutMode==2)) {
          settings.midiOutMode=2;
        }*/
        ImGui::Unindent();

        bool midiOutProgramChangeB=settings.midiOutProgramChange;
        if (ImGui::Checkbox(_("Send Program Change"),&midiOutProgramChangeB)) {
          settings.midiOutProgramChange=midiOutProgramChangeB;
          settingsChanged=true;
        }

        bool midiOutClockB=settings.midiOutClock;
        if (ImGui::Checkbox(_("Send MIDI clock"),&midiOutClockB)) {
          settings.midiOutClock=midiOutClockB;
          settingsChanged=true;
        }

        bool midiOutTimeB=settings.midiOutTime;
        if (ImGui::Checkbox(_("Send MIDI timecode"),&midiOutTimeB)) {
          settings.midiOutTime=midiOutTimeB;
          settingsChanged=true;
        }

        if (settings.midiOutTime) {
          ImGui::Text(_("Timecode frame rate:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("Closest to Tick Rate"),settings.midiOutTimeRate==0)) {
            settings.midiOutTimeRate=0;
            settingsChanged=true;
          }
          if (ImGui::RadioButton(_("Film (24fps)"),settings.midiOutTimeRate==1)) {
            settings.midiOutTimeRate=1;
            settingsChanged=true;
          }
          if (ImGui::RadioButton(_("PAL (25fps)"),settings.midiOutTimeRate==2)) {
            settings.midiOutTimeRate=2;
            settingsChanged=true;
          }
          if (ImGui::RadioButton(_("NTSC drop (29.97fps)"),settings.midiOutTimeRate==3)) {
            settings.midiOutTimeRate=3;
            settingsChanged=true;
          }
          if (ImGui::RadioButton(_("NTSC non-drop (30fps)"),settings.midiOutTimeRate==4)) {
            settings.midiOutTimeRate=4;
            settingsChanged=true;
          }
          ImGui::Unindent();
        }

        END_SECTION;
      }
      CONFIG_SECTION(_("Emulation")) {
        // SUBSECTION CORES
        CONFIG_SUBSECTION(_("Cores"));
        if (ImGui::BeginTable("##Cores",3)) {
          ImGui::TableSetupColumn("##System",ImGuiTableColumnFlags_WidthFixed);
          ImGui::TableSetupColumn("##PlaybackCores",ImGuiTableColumnFlags_WidthStretch);
          ImGui::TableSetupColumn("##RenderCores",ImGuiTableColumnFlags_WidthStretch);
          ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
          ImGui::TableNextColumn();
          ImGui::Text(_("System"));
          ImGui::TableNextColumn();
          ImGui::Text(_("Playback Core(s)"));
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("used for playback"));
          }
          ImGui::TableNextColumn();
          ImGui::Text(_("Render Core(s)"));
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("used in audio export"));
          }

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text("YM2151");
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##ArcadeCore",&settings.arcadeCore,arcadeCores,2)) settingsChanged=true;
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##ArcadeCoreRender",&settings.arcadeCoreRender,arcadeCores,2)) settingsChanged=true;

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text("YM2612");
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##YM2612Core",&settings.ym2612Core,ym2612Cores,3)) settingsChanged=true;
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##YM2612CoreRender",&settings.ym2612CoreRender,ym2612Cores,3)) settingsChanged=true;

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text("SN76489");
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##SNCore",&settings.snCore,snCores,2)) settingsChanged=true;
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##SNCoreRender",&settings.snCoreRender,snCores,2)) settingsChanged=true;

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text("NES");
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##NESCore",&settings.nesCore,nesCores,2)) settingsChanged=true;
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##NESCoreRender",&settings.nesCoreRender,nesCores,2)) settingsChanged=true;

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text("FDS");
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##FDSCore",&settings.fdsCore,nesCores,2)) settingsChanged=true;
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##FDSCoreRender",&settings.fdsCoreRender,nesCores,2)) settingsChanged=true;

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text("SID");
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##C64Core",&settings.c64Core,c64Cores,3)) settingsChanged=true;
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##C64CoreRender",&settings.c64CoreRender,c64Cores,3)) settingsChanged=true;

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text("POKEY");
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##POKEYCore",&settings.pokeyCore,pokeyCores,2)) settingsChanged=true;
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##POKEYCoreRender",&settings.pokeyCoreRender,pokeyCores,2)) settingsChanged=true;

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text("OPN");
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##OPNCore",&settings.opn1Core,opnCores,3)) settingsChanged=true;
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##OPNCoreRender",&settings.opn1CoreRender,opnCores,3)) settingsChanged=true;

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text("OPNA");
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##OPNACore",&settings.opnaCore,opnCores,3)) settingsChanged=true;
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##OPNACoreRender",&settings.opnaCoreRender,opnCores,3)) settingsChanged=true;

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text("OPNB");
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##OPNBCore",&settings.opnbCore,opnCores,3)) settingsChanged=true;
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##OPNBCoreRender",&settings.opnbCoreRender,opnCores,3)) settingsChanged=true;

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text("OPL/OPL2/Y8950");
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##OPL2Core",&settings.opl2Core,opl2Cores,3)) settingsChanged=true;
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##OPL2CoreRender",&settings.opl2CoreRender,opl2Cores,3)) settingsChanged=true;

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text("OPL3");
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##OPL3Core",&settings.opl3Core,opl3Cores,3)) settingsChanged=true;
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##OPL3CoreRender",&settings.opl3CoreRender,opl3Cores,3)) settingsChanged=true;

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text("OPL4");
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##OPL4Core",&settings.opl4Core,opl4Cores,2)) settingsChanged=true;
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##OPL4CoreRender",&settings.opl4CoreRender,opl4Cores,2)) settingsChanged=true;

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text("ESFM");
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##ESFMCore",&settings.esfmCore,LocalizedComboGetter,esfmCores,2)) settingsChanged=true;
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##ESFMCoreRender",&settings.esfmCoreRender,LocalizedComboGetter,esfmCores,2)) settingsChanged=true;

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text("OPLL");
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##OPLLCore",&settings.opllCore,opllCores,2)) settingsChanged=true;
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##OPLLCoreRender",&settings.opllCoreRender,opllCores,2)) settingsChanged=true;

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text("AY-3-8910/SSG");
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##AYCore",&settings.ayCore,ayCores,2)) settingsChanged=true;
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##AYCoreRender",&settings.ayCoreRender,ayCores,2)) settingsChanged=true;

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text("WonderSwan");
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##SwanCore",&settings.swanCore,swanCores,2)) settingsChanged=true;
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##SwanCoreRender",&settings.swanCoreRender,swanCores,2)) settingsChanged=true;

          ImGui::EndTable();
        }

        // SUBSECTION OTHER
        CONFIG_SUBSECTION(_("Quality"));
        if (ImGui::BeginTable("##CoreQual",3)) {
          ImGui::TableSetupColumn("##System",ImGuiTableColumnFlags_WidthFixed);
          ImGui::TableSetupColumn("##PlaybackCores",ImGuiTableColumnFlags_WidthStretch);
          ImGui::TableSetupColumn("##RenderCores",ImGuiTableColumnFlags_WidthStretch);
          ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
          ImGui::TableNextColumn();
          ImGui::Text(_("System"));
          ImGui::TableNextColumn();
          ImGui::Text(_("Playback"));
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("used for playback"));
          }
          ImGui::TableNextColumn();
          ImGui::Text(_("Render"));
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("used in audio export"));
          }

          CORE_QUALITY("Game Boy",gbQuality,gbQualityRender);
          CORE_QUALITY("PowerNoise",pnQuality,pnQualityRender);
          CORE_QUALITY("SAA1099",saaQuality,saaQualityRender);
          CORE_QUALITY("SID (dSID)",dsidQuality,dsidQualityRender);

          ImGui::EndTable();
        }

        // SUBSECTION OTHER
        CONFIG_SUBSECTION(_("Other"));

        ImGui::AlignTextToFramePadding();
        ImGui::Text(_("PC Speaker strategy"));
        ImGui::SameLine();
        if (ImGui::Combo("##PCSOutMethod",&settings.pcSpeakerOutMethod,LocalizedComboGetter,pcspkrOutMethods,5)) settingsChanged=true;

        ImGui::Separator();
        ImGui::Text(_("Sample ROMs:"));

        ImGui::AlignTextToFramePadding();
        ImGui::Text(_("OPL4 YRW801 path"));
        ImGui::SameLine();
        ImGui::InputText("##YRW801Path",&settings.yrw801Path);
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_FOLDER "##YRW801Load")) {
          openFileDialog(GUI_FILE_YRW801_ROM_OPEN);
        }

        /*
        ImGui::AlignTextToFramePadding();
        ImGui::Text(_("MultiPCM TG100 path"));
        ImGui::SameLine();
        ImGui::InputText("##TG100Path",&settings.tg100Path);
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_FOLDER "##TG100Load")) {
          openFileDialog(GUI_FILE_TG100_ROM_OPEN);
        }

        ImGui::AlignTextToFramePadding();
        ImGui::Text(_("MultiPCM MU5 path"));
        ImGui::SameLine();
        ImGui::InputText("##MU5Path",&settings.mu5Path);
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_FOLDER "##MU5Load")) {
          openFileDialog(GUI_FILE_MU5_ROM_OPEN);
        }
        */

        END_SECTION;
      }
      CONFIG_SECTION(_("Keyboard")) {
        // SUBSECTION LAYOUT
        CONFIG_SUBSECTION(_("Keyboard"));
        if (ImGui::Button(_("Import"))) {
          openFileDialog(GUI_FILE_IMPORT_KEYBINDS);
        }
        ImGui::SameLine();
        if (ImGui::Button(_("Export"))) {
          openFileDialog(GUI_FILE_EXPORT_KEYBINDS);
        }
        ImGui::SameLine();
        if (ImGui::Button(_("Reset defaults"))) {
          showWarning(_("Are you sure you want to reset the keyboard settings?"),GUI_WARN_RESET_KEYBINDS);
        }
        if (ImGui::BeginChild("##HotkeysList",ImVec2(0,0),false,ImGuiWindowFlags_HorizontalScrollbar)) {
          if (ImGui::TreeNode(_("Global hotkeys"))) {
            KEYBIND_CONFIG_BEGIN("keysGlobal");

            drawKeybindSettingsTableRow(GUI_ACTION_NEW);
            drawKeybindSettingsTableRow(GUI_ACTION_CLEAR);
            drawKeybindSettingsTableRow(GUI_ACTION_OPEN);
            drawKeybindSettingsTableRow(GUI_ACTION_OPEN_BACKUP);
            drawKeybindSettingsTableRow(GUI_ACTION_SAVE);
            drawKeybindSettingsTableRow(GUI_ACTION_SAVE_AS);
            drawKeybindSettingsTableRow(GUI_ACTION_EXPORT);
            drawKeybindSettingsTableRow(GUI_ACTION_UNDO);
            drawKeybindSettingsTableRow(GUI_ACTION_REDO);
            drawKeybindSettingsTableRow(GUI_ACTION_PLAY_TOGGLE);
            drawKeybindSettingsTableRow(GUI_ACTION_PLAY);
            drawKeybindSettingsTableRow(GUI_ACTION_STOP);
            drawKeybindSettingsTableRow(GUI_ACTION_PLAY_START);
            drawKeybindSettingsTableRow(GUI_ACTION_PLAY_REPEAT);
            drawKeybindSettingsTableRow(GUI_ACTION_PLAY_CURSOR);
            drawKeybindSettingsTableRow(GUI_ACTION_STEP_ONE);
            drawKeybindSettingsTableRow(GUI_ACTION_OCTAVE_UP);
            drawKeybindSettingsTableRow(GUI_ACTION_OCTAVE_DOWN);
            drawKeybindSettingsTableRow(GUI_ACTION_INS_UP);
            drawKeybindSettingsTableRow(GUI_ACTION_INS_DOWN);
            drawKeybindSettingsTableRow(GUI_ACTION_STEP_UP);
            drawKeybindSettingsTableRow(GUI_ACTION_STEP_DOWN);
            drawKeybindSettingsTableRow(GUI_ACTION_TOGGLE_EDIT);
            drawKeybindSettingsTableRow(GUI_ACTION_METRONOME);
            drawKeybindSettingsTableRow(GUI_ACTION_REPEAT_PATTERN);
            drawKeybindSettingsTableRow(GUI_ACTION_FOLLOW_ORDERS);
            drawKeybindSettingsTableRow(GUI_ACTION_FOLLOW_PATTERN);
            drawKeybindSettingsTableRow(GUI_ACTION_FULLSCREEN);
            drawKeybindSettingsTableRow(GUI_ACTION_TX81Z_REQUEST);
            drawKeybindSettingsTableRow(GUI_ACTION_PANIC);

            KEYBIND_CONFIG_END;
            ImGui::TreePop();
          }
          if (ImGui::TreeNode(_("Window activation"))) {
            KEYBIND_CONFIG_BEGIN("keysWindow");

            drawKeybindSettingsTableRow(GUI_ACTION_WINDOW_FIND);
            drawKeybindSettingsTableRow(GUI_ACTION_WINDOW_SETTINGS);
            drawKeybindSettingsTableRow(GUI_ACTION_WINDOW_SONG_INFO);
            drawKeybindSettingsTableRow(GUI_ACTION_WINDOW_SUBSONGS);
            drawKeybindSettingsTableRow(GUI_ACTION_WINDOW_SPEED);
            drawKeybindSettingsTableRow(GUI_ACTION_WINDOW_INS_LIST);
            drawKeybindSettingsTableRow(GUI_ACTION_WINDOW_WAVE_LIST);
            drawKeybindSettingsTableRow(GUI_ACTION_WINDOW_SAMPLE_LIST);
            drawKeybindSettingsTableRow(GUI_ACTION_WINDOW_ORDERS);
            drawKeybindSettingsTableRow(GUI_ACTION_WINDOW_PATTERN);
            drawKeybindSettingsTableRow(GUI_ACTION_WINDOW_MIXER);
            drawKeybindSettingsTableRow(GUI_ACTION_WINDOW_GROOVES);
            drawKeybindSettingsTableRow(GUI_ACTION_WINDOW_CHANNELS);
            drawKeybindSettingsTableRow(GUI_ACTION_WINDOW_PAT_MANAGER);
            drawKeybindSettingsTableRow(GUI_ACTION_WINDOW_SYS_MANAGER);
            drawKeybindSettingsTableRow(GUI_ACTION_WINDOW_COMPAT_FLAGS);
            drawKeybindSettingsTableRow(GUI_ACTION_WINDOW_NOTES);
            drawKeybindSettingsTableRow(GUI_ACTION_WINDOW_INS_EDIT);
            drawKeybindSettingsTableRow(GUI_ACTION_WINDOW_WAVE_EDIT);
            drawKeybindSettingsTableRow(GUI_ACTION_WINDOW_SAMPLE_EDIT);
            drawKeybindSettingsTableRow(GUI_ACTION_WINDOW_EDIT_CONTROLS);
            drawKeybindSettingsTableRow(GUI_ACTION_WINDOW_PIANO);
            drawKeybindSettingsTableRow(GUI_ACTION_WINDOW_OSCILLOSCOPE);
            drawKeybindSettingsTableRow(GUI_ACTION_WINDOW_CHAN_OSC);
            drawKeybindSettingsTableRow(GUI_ACTION_WINDOW_XY_OSC);
            drawKeybindSettingsTableRow(GUI_ACTION_WINDOW_VOL_METER);
            drawKeybindSettingsTableRow(GUI_ACTION_WINDOW_CLOCK);
            drawKeybindSettingsTableRow(GUI_ACTION_WINDOW_REGISTER_VIEW);
            drawKeybindSettingsTableRow(GUI_ACTION_WINDOW_LOG);
            drawKeybindSettingsTableRow(GUI_ACTION_WINDOW_STATS);
            drawKeybindSettingsTableRow(GUI_ACTION_WINDOW_MEMORY);
            drawKeybindSettingsTableRow(GUI_ACTION_WINDOW_EFFECT_LIST);
            drawKeybindSettingsTableRow(GUI_ACTION_WINDOW_DEBUG);
            drawKeybindSettingsTableRow(GUI_ACTION_WINDOW_CS_PLAYER);
            drawKeybindSettingsTableRow(GUI_ACTION_WINDOW_ABOUT);
            drawKeybindSettingsTableRow(GUI_ACTION_COLLAPSE_WINDOW);
            drawKeybindSettingsTableRow(GUI_ACTION_CLOSE_WINDOW);

            drawKeybindSettingsTableRow(GUI_ACTION_COMMAND_PALETTE);
            drawKeybindSettingsTableRow(GUI_ACTION_CMDPAL_RECENT);
            drawKeybindSettingsTableRow(GUI_ACTION_CMDPAL_INSTRUMENTS);
            drawKeybindSettingsTableRow(GUI_ACTION_CMDPAL_SAMPLES);

            KEYBIND_CONFIG_END;
            ImGui::TreePop();
          }
          if (ImGui::TreeNode(_("Note input"))) {
            if (ImGui::BeginTable("keysNoteInput",4)) {
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
                MappedInput& i=noteKeysRaw[_i];
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s",SDL_GetScancodeName((SDL_Scancode)i.scan));
                ImGui::TableNextColumn();
                if (i.val==102) {
                  snprintf(id,4095,_("Macro release##SNType_%d"),i.scan);
                  if (ImGui::Button(id)) {
                    i.val=0;
                  }
                } else if (i.val==101) {
                  snprintf(id,4095,_("Note release##SNType_%d"),i.scan);
                  if (ImGui::Button(id)) {
                    i.val=102;
                  }
                } else if (i.val==100) {
                  snprintf(id,4095,_("Note off##SNType_%d"),i.scan);
                  if (ImGui::Button(id)) {
                    i.val=101;
                  }
                } else {
                  snprintf(id,4095,_("Note##SNType_%d"),i.scan);
                  if (ImGui::Button(id)) {
                    i.val=100;
                  }
                }
                ImGui::TableNextColumn();
                if (i.val<100) {
                  snprintf(id,4095,"##SNValue_%d",i.scan);
                  if (ImGui::InputInt(id,&i.val,1,12)) {
                    if (i.val<0) i.val=0;
                    if (i.val>96) i.val=96;
                    settingsChanged=true;
                  }
                }
                ImGui::TableNextColumn();
                snprintf(id,4095,ICON_FA_TIMES "##SNRemove_%d",i.scan);
                if (ImGui::Button(id)) {
                  noteKeysRaw.erase(noteKeysRaw.begin()+_i);
                  _i--;
                  settingsChanged=true;
                }
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
                      settingsChanged=true;
                    }
                  }
                }
                ImGui::EndCombo();
              }
            }
            ImGui::TreePop();
          }
          if (ImGui::TreeNode(_("Pattern"))) {
            KEYBIND_CONFIG_BEGIN("keysPattern");

            drawKeybindSettingsTableRow(GUI_ACTION_PAT_NOTE_UP);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_NOTE_DOWN);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_OCTAVE_UP);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_OCTAVE_DOWN);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_VALUE_UP);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_VALUE_DOWN);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_VALUE_UP_COARSE);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_VALUE_DOWN_COARSE);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_SELECT_ALL);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_CUT);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_COPY);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_PASTE);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_PASTE_MIX);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_PASTE_MIX_BG);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_PASTE_FLOOD);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_PASTE_OVERFLOW);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_CURSOR_UP);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_CURSOR_DOWN);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_CURSOR_LEFT);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_CURSOR_RIGHT);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_CURSOR_UP_ONE);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_CURSOR_DOWN_ONE);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_CURSOR_LEFT_CHANNEL);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_CURSOR_RIGHT_CHANNEL);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_CURSOR_PREVIOUS_CHANNEL);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_CURSOR_NEXT_CHANNEL);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_CURSOR_BEGIN);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_CURSOR_END);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_CURSOR_UP_COARSE);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_CURSOR_DOWN_COARSE);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_SELECTION_UP);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_SELECTION_DOWN);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_SELECTION_LEFT);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_SELECTION_RIGHT);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_SELECTION_UP_ONE);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_SELECTION_DOWN_ONE);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_SELECTION_BEGIN);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_SELECTION_END);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_SELECTION_UP_COARSE);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_SELECTION_DOWN_COARSE);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_MOVE_UP);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_MOVE_DOWN);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_MOVE_LEFT_CHANNEL);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_MOVE_RIGHT_CHANNEL);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_DELETE);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_PULL_DELETE);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_INSERT);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_MUTE_CURSOR);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_SOLO_CURSOR);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_UNMUTE_ALL);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_NEXT_ORDER);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_PREV_ORDER);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_COLLAPSE);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_INCREASE_COLUMNS);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_DECREASE_COLUMNS);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_INTERPOLATE);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_FADE);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_INVERT_VALUES);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_FLIP_SELECTION);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_COLLAPSE_ROWS);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_EXPAND_ROWS);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_COLLAPSE_PAT);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_EXPAND_PAT);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_COLLAPSE_SONG);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_EXPAND_SONG);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_LATCH);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_CLEAR_LATCH);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_ABSORB_INSTRUMENT);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_CURSOR_UNDO);
            drawKeybindSettingsTableRow(GUI_ACTION_PAT_CURSOR_REDO);

            KEYBIND_CONFIG_END;
            ImGui::TreePop();
          }
          if (ImGui::TreeNode(_("Instrument list"))) {
            KEYBIND_CONFIG_BEGIN("keysInsList");

            drawKeybindSettingsTableRow(GUI_ACTION_INS_LIST_ADD);
            drawKeybindSettingsTableRow(GUI_ACTION_INS_LIST_DUPLICATE);
            drawKeybindSettingsTableRow(GUI_ACTION_INS_LIST_OPEN);
            drawKeybindSettingsTableRow(GUI_ACTION_INS_LIST_OPEN_REPLACE);
            drawKeybindSettingsTableRow(GUI_ACTION_INS_LIST_SAVE);
            drawKeybindSettingsTableRow(GUI_ACTION_INS_LIST_SAVE_DMP);
            drawKeybindSettingsTableRow(GUI_ACTION_INS_LIST_MOVE_UP);
            drawKeybindSettingsTableRow(GUI_ACTION_INS_LIST_MOVE_DOWN);
            drawKeybindSettingsTableRow(GUI_ACTION_INS_LIST_DELETE);
            drawKeybindSettingsTableRow(GUI_ACTION_INS_LIST_EDIT);
            drawKeybindSettingsTableRow(GUI_ACTION_INS_LIST_UP);
            drawKeybindSettingsTableRow(GUI_ACTION_INS_LIST_DOWN);
            drawKeybindSettingsTableRow(GUI_ACTION_INS_LIST_DIR_VIEW);

            KEYBIND_CONFIG_END;
            ImGui::TreePop();
          }
          if (ImGui::TreeNode(_("Wavetable list"))) {
            KEYBIND_CONFIG_BEGIN("keysWaveList");

            drawKeybindSettingsTableRow(GUI_ACTION_WAVE_LIST_ADD);
            drawKeybindSettingsTableRow(GUI_ACTION_WAVE_LIST_DUPLICATE);
            drawKeybindSettingsTableRow(GUI_ACTION_WAVE_LIST_OPEN);
            drawKeybindSettingsTableRow(GUI_ACTION_WAVE_LIST_OPEN_REPLACE);
            drawKeybindSettingsTableRow(GUI_ACTION_WAVE_LIST_SAVE);
            drawKeybindSettingsTableRow(GUI_ACTION_WAVE_LIST_SAVE_DMW);
            drawKeybindSettingsTableRow(GUI_ACTION_WAVE_LIST_SAVE_RAW);
            drawKeybindSettingsTableRow(GUI_ACTION_WAVE_LIST_MOVE_UP);
            drawKeybindSettingsTableRow(GUI_ACTION_WAVE_LIST_MOVE_DOWN);
            drawKeybindSettingsTableRow(GUI_ACTION_WAVE_LIST_DELETE);
            drawKeybindSettingsTableRow(GUI_ACTION_WAVE_LIST_EDIT);
            drawKeybindSettingsTableRow(GUI_ACTION_WAVE_LIST_UP);
            drawKeybindSettingsTableRow(GUI_ACTION_WAVE_LIST_DOWN);
            drawKeybindSettingsTableRow(GUI_ACTION_WAVE_LIST_DIR_VIEW);

            KEYBIND_CONFIG_END;
            ImGui::TreePop();
          }
          if (ImGui::TreeNode(_("Sample list"))) {
            KEYBIND_CONFIG_BEGIN("keysSampleList");

            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_LIST_ADD);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_LIST_DUPLICATE);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_CREATE_WAVE);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_LIST_OPEN);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_LIST_OPEN_REPLACE);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_LIST_OPEN_RAW);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_LIST_OPEN_REPLACE_RAW);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_LIST_SAVE);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_LIST_SAVE_RAW);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_LIST_MOVE_UP);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_LIST_MOVE_DOWN);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_LIST_DELETE);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_LIST_EDIT);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_LIST_UP);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_LIST_DOWN);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_LIST_PREVIEW);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_LIST_STOP_PREVIEW);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_LIST_DIR_VIEW);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_LIST_MAKE_MAP);

            KEYBIND_CONFIG_END;
            ImGui::TreePop();
          }
          if (ImGui::TreeNode(_("Orders"))) {
            KEYBIND_CONFIG_BEGIN("keysOrders");

            drawKeybindSettingsTableRow(GUI_ACTION_ORDERS_UP);
            drawKeybindSettingsTableRow(GUI_ACTION_ORDERS_DOWN);
            drawKeybindSettingsTableRow(GUI_ACTION_ORDERS_LEFT);
            drawKeybindSettingsTableRow(GUI_ACTION_ORDERS_RIGHT);
            drawKeybindSettingsTableRow(GUI_ACTION_ORDERS_INCREASE);
            drawKeybindSettingsTableRow(GUI_ACTION_ORDERS_DECREASE);
            drawKeybindSettingsTableRow(GUI_ACTION_ORDERS_EDIT_MODE);
            drawKeybindSettingsTableRow(GUI_ACTION_ORDERS_LINK);
            drawKeybindSettingsTableRow(GUI_ACTION_ORDERS_ADD);
            drawKeybindSettingsTableRow(GUI_ACTION_ORDERS_DUPLICATE);
            drawKeybindSettingsTableRow(GUI_ACTION_ORDERS_DEEP_CLONE);
            drawKeybindSettingsTableRow(GUI_ACTION_ORDERS_DUPLICATE_END);
            drawKeybindSettingsTableRow(GUI_ACTION_ORDERS_DEEP_CLONE_END);
            drawKeybindSettingsTableRow(GUI_ACTION_ORDERS_REMOVE);
            drawKeybindSettingsTableRow(GUI_ACTION_ORDERS_MOVE_UP);
            drawKeybindSettingsTableRow(GUI_ACTION_ORDERS_MOVE_DOWN);
            drawKeybindSettingsTableRow(GUI_ACTION_ORDERS_REPLAY);

            KEYBIND_CONFIG_END;
            ImGui::TreePop();
          }
          if (ImGui::TreeNode(_("Sample editor"))) {
            KEYBIND_CONFIG_BEGIN("keysSampleEdit");

            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_SELECT);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_DRAW);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_CUT);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_COPY);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_PASTE);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_PASTE_REPLACE);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_PASTE_MIX);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_SELECT_ALL);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_RESIZE);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_RESAMPLE);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_AMPLIFY);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_NORMALIZE);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_FADE_IN);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_FADE_OUT);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_INSERT);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_SILENCE);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_DELETE);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_TRIM);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_REVERSE);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_INVERT);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_SIGN);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_FILTER);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_PREVIEW);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_STOP_PREVIEW);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_ZOOM_IN);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_ZOOM_OUT);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_ZOOM_AUTO);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_MAKE_INS);
            drawKeybindSettingsTableRow(GUI_ACTION_SAMPLE_SET_LOOP);

            KEYBIND_CONFIG_END;
            ImGui::TreePop();
          }
        }
        ImGui::EndChild();
        END_SECTION;
      }
      CONFIG_SECTION(_("Interface")) {
        // SUBSECTION LAYOUT
        CONFIG_SUBSECTION(_("Layout"));
        ImGui::AlignTextToFramePadding();
        ImGui::Text(_("Workspace layout:"));
        ImGui::SameLine();
        if (ImGui::Button(_("Import"))) {
          openFileDialog(GUI_FILE_IMPORT_LAYOUT);
        }
        ImGui::SameLine();
        if (ImGui::Button(_("Export"))) {
          openFileDialog(GUI_FILE_EXPORT_LAYOUT);
        }
        ImGui::SameLine();
        if (ImGui::Button(_("Reset"))) {
          showWarning(_("Are you sure you want to reset the workspace layout?"),GUI_WARN_RESET_LAYOUT);
        }

        bool allowEditDockingB=settings.allowEditDocking;
        if (ImGui::Checkbox(_("Allow docking editors"),&allowEditDockingB)) {
          settings.allowEditDocking=allowEditDockingB;
          settingsChanged=true;
        }

#ifndef IS_MOBILE
          bool saveWindowPosB=settings.saveWindowPos;
          if (ImGui::Checkbox(_("Remember window position"),&saveWindowPosB)) {
            settings.saveWindowPos=saveWindowPosB;
            settingsChanged=true;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("remembers the window's last position on start-up."));
          }
#endif

        bool moveWindowTitleB=settings.moveWindowTitle;
        if (ImGui::Checkbox(_("Only allow window movement when clicking on title bar"),&moveWindowTitleB)) {
          settings.moveWindowTitle=moveWindowTitleB;
          applyUISettings(false);
          settingsChanged=true;
        }

        bool centerPopupB=settings.centerPopup;
        if (ImGui::Checkbox(_("Center pop-up windows"),&centerPopupB)) {
          settings.centerPopup=centerPopupB;
          settingsChanged=true;
        }

        ImGui::Text(_("Play/edit controls layout:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("Classic##ecl0"),settings.controlLayout==0)) {
          settings.controlLayout=0;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Compact##ecl1"),settings.controlLayout==1)) {
          settings.controlLayout=1;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Compact (vertical)##ecl2"),settings.controlLayout==2)) {
          settings.controlLayout=2;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Split##ecl3"),settings.controlLayout==3)) {
          settings.controlLayout=3;
          settingsChanged=true;
        }
        ImGui::Unindent();

        ImGui::Text(_("Position of buttons in Orders:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("Top##obp0"),settings.orderButtonPos==0)) {
          settings.orderButtonPos=0;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Left##obp1"),settings.orderButtonPos==1)) {
          settings.orderButtonPos=1;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Right##obp2"),settings.orderButtonPos==2)) {
          settings.orderButtonPos=2;
          settingsChanged=true;
        }
        ImGui::Unindent();

        // SUBSECTION MOUSE
        CONFIG_SUBSECTION(_("Mouse"));

        if (CWSliderFloat(_("Double-click time (seconds)"),&settings.doubleClickTime,0.02,1.0,"%.2f")) {
          if (settings.doubleClickTime<0.02) settings.doubleClickTime=0.02;
          if (settings.doubleClickTime>1.0) settings.doubleClickTime=1.0;

          applyUISettings(false);
          settingsChanged=true;
        }

        bool avoidRaisingPatternB=settings.avoidRaisingPattern;
        if (ImGui::Checkbox(_("Don't raise pattern editor on click"),&avoidRaisingPatternB)) {
          settings.avoidRaisingPattern=avoidRaisingPatternB;
          settingsChanged=true;
        }

        bool insFocusesPatternB=settings.insFocusesPattern;
        if (ImGui::Checkbox(_("Focus pattern editor when selecting instrument"),&insFocusesPatternB)) {
          settings.insFocusesPattern=insFocusesPatternB;
          settingsChanged=true;
        }

        bool draggableDataViewB=settings.draggableDataView;
        if (ImGui::Checkbox(_("Draggable instruments/samples/waves"),&draggableDataViewB)) {
          settings.draggableDataView=draggableDataViewB;
          settingsChanged=true;
        }

        ImGui::Text(_("Note preview behavior:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("Never##npb0"),settings.notePreviewBehavior==0)) {
          settings.notePreviewBehavior=0;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("When cursor is in Note column##npb1"),settings.notePreviewBehavior==1)) {
          settings.notePreviewBehavior=1;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("When cursor is in Note column or not in edit mode##npb2"),settings.notePreviewBehavior==2)) {
          settings.notePreviewBehavior=2;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Always##npb3"),settings.notePreviewBehavior==3)) {
          settings.notePreviewBehavior=3;
          settingsChanged=true;
        }
        ImGui::Unindent();

        ImGui::Text(_("Allow dragging selection:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("No##dms0"),settings.dragMovesSelection==0)) {
          settings.dragMovesSelection=0;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Yes##dms1"),settings.dragMovesSelection==1)) {
          settings.dragMovesSelection=1;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Yes (while holding Ctrl only)##dms2"),settings.dragMovesSelection==2)) {
          settings.dragMovesSelection=2;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Yes (copy)##dms3"),settings.dragMovesSelection==3)) {
          settings.dragMovesSelection=3;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Yes (while holding Ctrl only and copy)##dms4"),settings.dragMovesSelection==4)) {
          settings.dragMovesSelection=4;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Yes (holding Ctrl copies)##dms5"),settings.dragMovesSelection==5)) {
          settings.dragMovesSelection=5;
          settingsChanged=true;
        }
        ImGui::Unindent();

        ImGui::Text(_("Toggle channel solo on:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("Right-click or double-click##soloA"),settings.soloAction==0)) {
          settings.soloAction=0;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Right-click##soloR"),settings.soloAction==1)) {
          settings.soloAction=1;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Double-click##soloD"),settings.soloAction==2)) {
          settings.soloAction=2;
          settingsChanged=true;
        }
        ImGui::Unindent();

        ImGui::Text(_("Modifier for alternate wheel-scrolling (vertical/zoom/slider-input):"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("Ctrl or Meta/Cmd##cwm1"),settings.ctrlWheelModifier==0)) {
          settings.ctrlWheelModifier=0;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Ctrl##cwm2"),settings.ctrlWheelModifier==1)) {
          settings.ctrlWheelModifier=1;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Meta/Cmd##cwm3"),settings.ctrlWheelModifier==2)) {
          settings.ctrlWheelModifier=2;
          settingsChanged=true;
        }
        // technically this key is called Option on mac, but we call it Alt in getKeyName(s)
        if (ImGui::RadioButton(_("Alt##cwm4"),settings.ctrlWheelModifier==3)) {
          settings.ctrlWheelModifier=3;
          settingsChanged=true;
        }
        ImGui::Unindent();

        bool doubleClickColumnB=settings.doubleClickColumn;
        if (ImGui::Checkbox(_("Double click selects entire column"),&doubleClickColumnB)) {
          settings.doubleClickColumn=doubleClickColumnB;
          settingsChanged=true;
        }

        // SUBSECTION CURSOR BEHAVIOR
        CONFIG_SUBSECTION(_("Cursor behavior"));
        bool insertBehaviorB=settings.insertBehavior;
        if (ImGui::Checkbox(_("Insert pushes entire channel row"),&insertBehaviorB)) {
          settings.insertBehavior=insertBehaviorB;
          settingsChanged=true;
        }

        bool pullDeleteRowB=settings.pullDeleteRow;
        if (ImGui::Checkbox(_("Pull delete affects entire channel row"),&pullDeleteRowB)) {
          settings.pullDeleteRow=pullDeleteRowB;
          settingsChanged=true;
        }

        bool pushNibbleB=settings.pushNibble;
        if (ImGui::Checkbox(_("Push value when overwriting instead of clearing it"),&pushNibbleB)) {
          settings.pushNibble=pushNibbleB;
          settingsChanged=true;
        }

        bool inputRepeatB=settings.inputRepeat;
        if (ImGui::Checkbox(_("Keyboard note/value input repeat (hold key to input continuously)"),&inputRepeatB)) {
          settings.inputRepeat=inputRepeatB;
          settingsChanged=true;
        }

        ImGui::Text(_("Effect input behavior:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("Move down##eicb0"),settings.effectCursorDir==0)) {
          settings.effectCursorDir=0;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Move to effect value (otherwise move down)##eicb1"),settings.effectCursorDir==1)) {
          settings.effectCursorDir=1;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Move to effect value/next effect and wrap around##eicb2"),settings.effectCursorDir==2)) {
          settings.effectCursorDir=2;
          settingsChanged=true;
        }
        ImGui::Unindent();

        bool effectDeletionAltersValueB=settings.effectDeletionAltersValue;
        if (ImGui::Checkbox(_("Delete effect value when deleting effect"),&effectDeletionAltersValueB)) {
          settings.effectDeletionAltersValue=effectDeletionAltersValueB;
          settingsChanged=true;
        }

        bool absorbInsInputB=settings.absorbInsInput;
        if (ImGui::Checkbox(_("Change current instrument when changing instrument column (absorb)"),&absorbInsInputB)) {
          settings.absorbInsInput=absorbInsInputB;
          settingsChanged=true;
        }

        bool removeInsOffB=settings.removeInsOff;
        if (ImGui::Checkbox(_("Remove instrument value when inserting note off/release"),&removeInsOffB)) {
          settings.removeInsOff=removeInsOffB;
          settingsChanged=true;
        }

        bool removeVolOffB=settings.removeVolOff;
        if (ImGui::Checkbox(_("Remove volume value when inserting note off/release"),&removeVolOffB)) {
          settings.removeVolOff=removeVolOffB;
          settingsChanged=true;
        }

        // SUBSECTION CURSOR MOVEMENT
        CONFIG_SUBSECTION(_("Cursor movement"));

        ImGui::Text(_("Wrap horizontally:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("No##wrapH0"),settings.wrapHorizontal==0)) {
          settings.wrapHorizontal=0;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Yes##wrapH1"),settings.wrapHorizontal==1)) {
          settings.wrapHorizontal=1;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Yes, and move to next/prev row##wrapH2"),settings.wrapHorizontal==2)) {
          settings.wrapHorizontal=2;
          settingsChanged=true;
        }
        ImGui::Unindent();

        ImGui::Text(_("Wrap vertically:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("No##wrapV0"),settings.wrapVertical==0)) {
          settings.wrapVertical=0;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Yes##wrapV1"),settings.wrapVertical==1)) {
          settings.wrapVertical=1;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Yes, and move to next/prev pattern##wrapV2"),settings.wrapVertical==2)) {
          settings.wrapVertical=2;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Yes, and move to next/prev pattern (wrap around)##wrapV2"),settings.wrapVertical==3)) {
          settings.wrapVertical=3;
          settingsChanged=true;
        }
        ImGui::Unindent();

        ImGui::Text(_("Cursor movement keys behavior:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("Move by one##cmk0"),settings.scrollStep==0)) {
          settings.scrollStep=0;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Move by Edit Step##cmk1"),settings.scrollStep==1)) {
          settings.scrollStep=1;
          settingsChanged=true;
        }
        ImGui::Unindent();

        bool stepOnDeleteB=settings.stepOnDelete;
        if (ImGui::Checkbox(_("Move cursor by edit step on delete"),&stepOnDeleteB)) {
          settings.stepOnDelete=stepOnDeleteB;
          settingsChanged=true;
        }

        bool stepOnInsertB=settings.stepOnInsert;
        if (ImGui::Checkbox(_("Move cursor by edit step on insert (push)"),&stepOnInsertB)) {
          settings.stepOnInsert=stepOnInsertB;
          settingsChanged=true;
        }

        bool pullDeleteBehaviorB=settings.pullDeleteBehavior;
        if (ImGui::Checkbox(_("Move cursor up on backspace-delete"),&pullDeleteBehaviorB)) {
          settings.pullDeleteBehavior=pullDeleteBehaviorB;
          settingsChanged=true;
        }

        bool cursorPastePosB=settings.cursorPastePos;
        if (ImGui::Checkbox(_("Move cursor to end of clipboard content when pasting"),&cursorPastePosB)) {
          settings.cursorPastePos=cursorPastePosB;
          settingsChanged=true;
        }

        // SUBSECTION SCROLLING
        CONFIG_SUBSECTION(_("Scrolling"));

        ImGui::Text(_("Change order when scrolling outside of pattern bounds:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("No##pscroll0"),settings.scrollChangesOrder==0)) {
          settings.scrollChangesOrder=0;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Yes##pscroll1"),settings.scrollChangesOrder==1)) {
          settings.scrollChangesOrder=1;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Yes, and wrap around song##pscroll2"),settings.scrollChangesOrder==2)) {
          settings.scrollChangesOrder=2;
          settingsChanged=true;
        }
        ImGui::Unindent();

        bool cursorFollowsOrderB=settings.cursorFollowsOrder;
        if (ImGui::Checkbox(_("Cursor follows current order when moving it"),&cursorFollowsOrderB)) {
          settings.cursorFollowsOrder=cursorFollowsOrderB;
          settingsChanged=true;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("applies when playback is stopped."));
        }

        bool cursorMoveNoScrollB=settings.cursorMoveNoScroll;
        if (ImGui::Checkbox(_("Don't scroll when moving cursor"),&cursorMoveNoScrollB)) {
          settings.cursorMoveNoScroll=cursorMoveNoScrollB;
          settingsChanged=true;
        }

        ImGui::Text(_("Move cursor with scroll wheel:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("No##csw0"),settings.cursorFollowsWheel==0)) {
          settings.cursorFollowsWheel=0;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Yes##csw1"),settings.cursorFollowsWheel==1)) {
          settings.cursorFollowsWheel=1;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Inverted##csw2"),settings.cursorFollowsWheel==2)) {
          settings.cursorFollowsWheel=2;
          settingsChanged=true;
        }
        ImGui::Unindent();

        if (settings.cursorFollowsWheel) {
          ImGui::Text(_("How many steps to move with each scroll wheel step?"));
          if (ImGui::RadioButton(_("One##cws0"),settings.cursorWheelStep==0)) {
            settings.cursorWheelStep=0;
            settingsChanged=true;
          }
          if (ImGui::RadioButton(_("Edit Step##cws1"),settings.cursorWheelStep==1)) {
            settings.cursorWheelStep=1;
            settingsChanged=true;
          }
        }

        // SUBSECTION ASSETS
        CONFIG_SUBSECTION(_("Assets"));

        bool insTypeMenuB=settings.insTypeMenu;
        if (ImGui::Checkbox(_("Display instrument type menu when adding instrument"),&insTypeMenuB)) {
          settings.insTypeMenu=insTypeMenuB;
          settingsChanged=true;
        }

        bool selectAssetOnLoadB=settings.selectAssetOnLoad;
        if (ImGui::Checkbox(_("Select asset after opening one"),&selectAssetOnLoadB)) {
          settings.selectAssetOnLoad=selectAssetOnLoadB;
          settingsChanged=true;
        }

        END_SECTION;
      }
      CONFIG_SECTION(_("Appearance")) {
        // SUBSECTION INTERFACE
        CONFIG_SUBSECTION(_("Scaling"));
        bool dpiScaleAuto=(settings.dpiScale<0.5f);
        if (ImGui::Checkbox(_("Automatic UI scaling factor"),&dpiScaleAuto)) {
          if (dpiScaleAuto) {
            settings.dpiScale=0.0f;
          } else {
            settings.dpiScale=1.0f;
          }
          settingsChanged=true;
        }
        if (!dpiScaleAuto) {
          if (ImGui::SliderFloat(_("UI scaling factor"),&settings.dpiScale,1.0f,3.0f,"%.2fx")) {
            if (settings.dpiScale<0.5f) settings.dpiScale=0.5f;
            if (settings.dpiScale>3.0f) settings.dpiScale=3.0f;
            settingsChanged=true;
          } rightClickable
        }

        if (ImGui::InputInt(_("Icon size"),&settings.iconSize,1,3)) {
          if (settings.iconSize<3) settings.iconSize=3;
          if (settings.iconSize>48) settings.iconSize=48;
          settingsChanged=true;
        }

        // SUBSECTION TEXT
        CONFIG_SUBSECTION(_("Text"));
        if (ImGui::BeginTable("##Text",2)) {
          ImGui::TableSetupColumn("##Label",ImGuiTableColumnFlags_WidthFixed);
          ImGui::TableSetupColumn("##Combos",ImGuiTableColumnFlags_WidthStretch);
#ifdef HAVE_FREETYPE
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text(_("Font renderer"));
          ImGui::TableNextColumn();
          if (ImGui::Combo("##FontBack",&settings.fontBackend,fontBackends,2)) settingsChanged=true;
#else
          settings.fontBackend=0;
#endif

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text(_("Main font"));
          ImGui::TableNextColumn();
          if (ImGui::Combo("##MainFont",&settings.mainFont,LocalizedComboGetter,mainFonts,7)) settingsChanged=true;
          if (settings.mainFont==6) {
            ImGui::InputText("##MainFontPath",&settings.mainFontPath);
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_FOLDER "##MainFontLoad")) {
              openFileDialog(GUI_FILE_LOAD_MAIN_FONT);
              settingsChanged=true;
            }
          }
          if (ImGui::InputInt(_("Size##MainFontSize"),&settings.mainFontSize,1,3)) {
            if (settings.mainFontSize<3) settings.mainFontSize=3;
            if (settings.mainFontSize>96) settings.mainFontSize=96;
            settingsChanged=true;
          }
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text(_("Header font"));
          ImGui::TableNextColumn();
          if (ImGui::Combo("##HeadFont",&settings.headFont,LocalizedComboGetter,headFonts,7)) settingsChanged=true;
          if (settings.headFont==6) {
            ImGui::InputText("##HeadFontPath",&settings.headFontPath);
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_FOLDER "##HeadFontLoad")) {
              openFileDialog(GUI_FILE_LOAD_HEAD_FONT);
              settingsChanged=true;
            }
          }
          if (ImGui::InputInt(_("Size##HeadFontSize"),&settings.headFontSize,1,3)) {
            if (settings.headFontSize<3) settings.headFontSize=3;
            if (settings.headFontSize>96) settings.headFontSize=96;
            settingsChanged=true;
          }
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text(_("Pattern font"));
          ImGui::TableNextColumn();
          if (ImGui::Combo("##PatFont",&settings.patFont,LocalizedComboGetter,patFonts,7)) settingsChanged=true;
          if (settings.patFont==6) {
            ImGui::InputText("##PatFontPath",&settings.patFontPath);
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_FOLDER "##PatFontLoad")) {
              openFileDialog(GUI_FILE_LOAD_PAT_FONT);
              settingsChanged=true;
            }
          }
          if (ImGui::InputInt(_("Size##PatFontSize"),&settings.patFontSize,1,3)) {
            if (settings.patFontSize<3) settings.patFontSize=3;
            if (settings.patFontSize>96) settings.patFontSize=96;
            settingsChanged=true;
          }
          ImGui::EndTable();
        }

        if (settings.fontBackend==1) {
          bool fontAntiAliasB=settings.fontAntiAlias;
          if (ImGui::Checkbox(_("Anti-aliased fonts"),&fontAntiAliasB)) {
            settings.fontAntiAlias=fontAntiAliasB;
            settingsChanged=true;
          }

          bool fontBitmapB=settings.fontBitmap;
          if (ImGui::Checkbox(_("Support bitmap fonts"),&fontBitmapB)) {
            settings.fontBitmap=fontBitmapB;
            settingsChanged=true;
          }

          ImGui::Text(_("Hinting:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("Off (soft)##fh0"),settings.fontHinting==0)) {
            settings.fontHinting=0;
            settingsChanged=true;
          }
          if (ImGui::RadioButton(_("Slight##fh1"),settings.fontHinting==1)) {
            settings.fontHinting=1;
            settingsChanged=true;
          }
          if (ImGui::RadioButton(_("Normal##fh2"),settings.fontHinting==2)) {
            settings.fontHinting=2;
            settingsChanged=true;
          }
          if (ImGui::RadioButton(_("Full (hard)##fh3"),settings.fontHinting==3)) {
            settings.fontHinting=3;
            settingsChanged=true;
          }
          ImGui::Unindent();

          ImGui::Text(_("Auto-hinter:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("Disable##fah0"),settings.fontAutoHint==0)) {
            settings.fontAutoHint=0;
            settingsChanged=true;
          }
          if (ImGui::RadioButton(_("Enable##fah1"),settings.fontAutoHint==1)) {
            settings.fontAutoHint=1;
            settingsChanged=true;
          }
          if (ImGui::RadioButton(_("Force##fah2"),settings.fontAutoHint==2)) {
            settings.fontAutoHint=2;
            settingsChanged=true;
          }
          ImGui::Unindent();
        }

        ImGui::Text(_("Oversample"));

        ImGui::SameLine();
        if (ImGui::RadioButton(_("1×##fos1"),settings.fontOversample==1)) {
          settings.fontOversample=1;
          settingsChanged=true;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("saves video memory. reduces font rendering quality.\nuse for pixel/bitmap fonts."));
        }
        ImGui::SameLine();
        if (ImGui::RadioButton(_("2×##fos2"),settings.fontOversample==2)) {
          settings.fontOversample=2;
          settingsChanged=true;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("default."));
        }
        ImGui::SameLine();
        if (ImGui::RadioButton(_("3×##fos3"),settings.fontOversample==3)) {
          settings.fontOversample=3;
          settingsChanged=true;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("slightly better font rendering quality.\nuses more video memory."));
        }

        bool loadFallbackB=settings.loadFallback;
        if (ImGui::Checkbox(_("Load fallback font"),&loadFallbackB)) {
          settings.loadFallback=loadFallbackB;
          settingsChanged=true;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("disable to save video memory."));
        }

        bool loadFallbackPatB=settings.loadFallbackPat;
        if (ImGui::Checkbox(_("Load fallback font (pattern)"),&loadFallbackPatB)) {
          settings.loadFallbackPat=loadFallbackPatB;
          settingsChanged=true;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("disable to save video memory."));
        }

        bool loadJapaneseB=settings.loadJapanese;
        if (ImGui::Checkbox(_("Display Japanese characters"),&loadJapaneseB)) {
          settings.loadJapanese=loadJapaneseB;
          settingsChanged=true;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_(
            "Only toggle this option if you have enough graphics memory.\n"
            "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
            "このオプションは、十分なグラフィックメモリがある場合にのみ切り替えてください。\n"
            "これは、Dear ImGuiにダイナミックフォントアトラスが実装されるまでの一時的な解決策です。"
          ));
        }

        bool loadChineseB=settings.loadChinese;
        if (ImGui::Checkbox(_("Display Chinese (Simplified) characters"),&loadChineseB)) {
          settings.loadChinese=loadChineseB;
          settingsChanged=true;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_(
            "Only toggle this option if you have enough graphics memory.\n"
            "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
            "请在确保你有足够的显存后再启动此设定\n"
            "这是一个在ImGui实现动态字体加载之前的临时解决方案"
          ));
        }

        bool loadChineseTraditionalB=settings.loadChineseTraditional;
        if (ImGui::Checkbox(_("Display Chinese (Traditional) characters"),&loadChineseTraditionalB)) {
          settings.loadChineseTraditional=loadChineseTraditionalB;
          settingsChanged=true;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_(
            "Only toggle this option if you have enough graphics memory.\n"
            "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
            "請在確保你有足夠的顯存后再啟動此設定\n"
            "這是一個在ImGui實現動態字體加載之前的臨時解決方案"
          ));
        }

        bool loadKoreanB=settings.loadKorean;
        if (ImGui::Checkbox(_("Display Korean characters"),&loadKoreanB)) {
          settings.loadKorean=loadKoreanB;
          settingsChanged=true;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_(
            "Only toggle this option if you have enough graphics memory.\n"
            "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
            "그래픽 메모리가 충분한 경우에만 이 옵션을 선택하십시오.\n"
            "이 옵션은 Dear ImGui에 동적 글꼴 아틀라스가 구현될 때까지 임시 솔루션입니다."
          ));
        }

        // SUBSECTION PROGRAM
        CONFIG_SUBSECTION(_("Program"));
        ImGui::Text(_("Title bar:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("Furnace##tbar0"),settings.titleBarInfo==0)) {
          settings.titleBarInfo=0;
          updateWindowTitle();
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Song Name - Furnace##tbar1"),settings.titleBarInfo==1)) {
          settings.titleBarInfo=1;
          updateWindowTitle();
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("file_name.fur - Furnace##tbar2"),settings.titleBarInfo==2)) {
          settings.titleBarInfo=2;
          updateWindowTitle();
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("/path/to/file.fur - Furnace##tbar3"),settings.titleBarInfo==3)) {
          settings.titleBarInfo=3;
          updateWindowTitle();
          settingsChanged=true;
        }
        ImGui::Unindent();

        bool titleBarSysB=settings.titleBarSys;
        if (ImGui::Checkbox(_("Display system name on title bar"),&titleBarSysB)) {
          settings.titleBarSys=titleBarSysB;
          updateWindowTitle();
          settingsChanged=true;
        }

        bool noMultiSystemB=settings.noMultiSystem;
        if (ImGui::Checkbox(_("Display chip names instead of \"multi-system\" in title bar"),&noMultiSystemB)) {
          settings.noMultiSystem=noMultiSystemB;
          updateWindowTitle();
          settingsChanged=true;
        }

        ImGui::Text(_("Status bar:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("Cursor details##sbar0"),settings.statusDisplay==0)) {
          settings.statusDisplay=0;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("File path##sbar1"),settings.statusDisplay==1)) {
          settings.statusDisplay=1;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Cursor details or file path##sbar2"),settings.statusDisplay==2)) {
          settings.statusDisplay=2;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Nothing##sbar3"),settings.statusDisplay==3)) {
          settings.statusDisplay=3;
          settingsChanged=true;
        }
        ImGui::Unindent();

        bool playbackTimeB=settings.playbackTime;
        if (ImGui::Checkbox(_("Display playback status when playing"),&playbackTimeB)) {
          settings.playbackTime=playbackTimeB;
          settingsChanged=true;
        }

        ImGui::Text(_("Export options layout:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("Sub-menus in File menu##eol0"),settings.exportOptionsLayout==0)) {
          settings.exportOptionsLayout=0;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Modal window with tabs##eol1"),settings.exportOptionsLayout==1)) {
          settings.exportOptionsLayout=1;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Modal windows with options in File menu##eol2"),settings.exportOptionsLayout==2)) {
          settings.exportOptionsLayout=2;
          settingsChanged=true;
        }
        ImGui::Unindent();

        bool capitalMenuBarB=settings.capitalMenuBar;
        if (ImGui::Checkbox(_("Capitalize menu bar"),&capitalMenuBarB)) {
          settings.capitalMenuBar=capitalMenuBarB;
          settingsChanged=true;
        }

        bool classicChipOptionsB=settings.classicChipOptions;
        if (ImGui::Checkbox(_("Display add/configure/change/remove chip menus in File menu"),&classicChipOptionsB)) {
          settings.classicChipOptions=classicChipOptionsB;
          settingsChanged=true;
        }

        // SUBSECTION ORDERS
        CONFIG_SUBSECTION(_("Orders"));
        // sorry. temporarily disabled until ImGui has a way to add separators in tables arbitrarily.
        /*bool sysSeparatorsB=settings.sysSeparators;
        if (ImGui::Checkbox(_("Add separators between systems in Orders"),&sysSeparatorsB)) {
          settings.sysSeparators=sysSeparatorsB;
        }*/

        bool ordersCursorB=settings.ordersCursor;
        if (ImGui::Checkbox(_("Highlight channel at cursor in Orders"),&ordersCursorB)) {
          settings.ordersCursor=ordersCursorB;
          settingsChanged=true;
        }

        ImGui::Text(_("Orders row number format:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("Decimal##orbD"),settings.orderRowsBase==0)) {
          settings.orderRowsBase=0;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Hexadecimal##orbH"),settings.orderRowsBase==1)) {
          settings.orderRowsBase=1;
          settingsChanged=true;
        }
        ImGui::Unindent();

        // SUBSECTION PATTERN
        CONFIG_SUBSECTION(_("Pattern"));
        bool centerPatternB=settings.centerPattern;
        if (ImGui::Checkbox(_("Center pattern view"),&centerPatternB)) {
          settings.centerPattern=centerPatternB;
          settingsChanged=true;
        }

        bool overflowHighlightB=settings.overflowHighlight;
        if (ImGui::Checkbox(_("Overflow pattern highlights"),&overflowHighlightB)) {
          settings.overflowHighlight=overflowHighlightB;
          settingsChanged=true;
        }

        bool viewPrevPatternB=settings.viewPrevPattern;
        if (ImGui::Checkbox(_("Display previous/next pattern"),&viewPrevPatternB)) {
          settings.viewPrevPattern=viewPrevPatternB;
          settingsChanged=true;
        }

        ImGui::Text(_("Pattern row number format:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("Decimal##prbD"),settings.patRowsBase==0)) {
          settings.patRowsBase=0;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Hexadecimal##prbH"),settings.patRowsBase==1)) {
          settings.patRowsBase=1;
          settingsChanged=true;
        }
        ImGui::Unindent();

        ImGui::Text(_("Pattern view labels:"));
        ImGui::PushFont(patFont);
        if (ImGui::InputTextWithHint("##PVLOff","OFF",&settings.noteOffLabel)) settingsChanged=true;
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::Text(_("Note off (3-char)"));
        ImGui::PushFont(patFont);
        if (ImGui::InputTextWithHint("##PVLRel","===",&settings.noteRelLabel)) settingsChanged=true;
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::Text(_("Note release (3-char)"));
        ImGui::PushFont(patFont);
        if (ImGui::InputTextWithHint("##PVLMacroRel","REL",&settings.macroRelLabel)) settingsChanged=true;
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::Text(_("Macro release (3-char)"));
        ImGui::PushFont(patFont);
        if (ImGui::InputTextWithHint("##PVLE3","...",&settings.emptyLabel)) settingsChanged=true;
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::Text(_("Empty field (3-char)"));
        ImGui::PushFont(patFont);
        if (ImGui::InputTextWithHint("##PVLE2","..",&settings.emptyLabel2)) settingsChanged=true;
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::Text(_("Empty field (2-char)"));

        ImGui::Text(_("Pattern view spacing after:"));

        if (CWSliderInt(_("Note"),&settings.noteCellSpacing,0,32)) {
          if (settings.noteCellSpacing<0) settings.noteCellSpacing=0;
          if (settings.noteCellSpacing>32) settings.noteCellSpacing=32;
          settingsChanged=true;
        }

        if (CWSliderInt(_("Instrument"),&settings.insCellSpacing,0,32)) {
          if (settings.insCellSpacing<0) settings.insCellSpacing=0;
          if (settings.insCellSpacing>32) settings.insCellSpacing=32;
          settingsChanged=true;
        }

        if (CWSliderInt(_("Volume"),&settings.volCellSpacing,0,32)) {
          if (settings.volCellSpacing<0) settings.volCellSpacing=0;
          if (settings.volCellSpacing>32) settings.volCellSpacing=32;
          settingsChanged=true;
        }

        if (CWSliderInt(_("Effect"),&settings.effectCellSpacing,0,32)) {
          if (settings.effectCellSpacing<0) settings.effectCellSpacing=0;
          if (settings.effectCellSpacing>32) settings.effectCellSpacing=32;
          settingsChanged=true;
        }

        if (CWSliderInt(_("Effect value"),&settings.effectValCellSpacing,0,32)) {
          if (settings.effectValCellSpacing<0) settings.effectValCellSpacing=0;
          if (settings.effectValCellSpacing>32) settings.effectValCellSpacing=32;
          settingsChanged=true;
        }

        bool oneDigitEffectsB=settings.oneDigitEffects;
        if (ImGui::Checkbox(_("Single-digit effects for 00-0F"),&oneDigitEffectsB)) {
          settings.oneDigitEffects=oneDigitEffectsB;
          settingsChanged=true;
        }

        bool flatNotesB=settings.flatNotes;
        if (ImGui::Checkbox(_("Use flats instead of sharps"),&flatNotesB)) {
          settings.flatNotes=flatNotesB;
          settingsChanged=true;
        }

        bool germanNotationB=settings.germanNotation;
        if (ImGui::Checkbox(_("Use German notation"),&germanNotationB)) {
          settings.germanNotation=germanNotationB;
          settingsChanged=true;
        }

        // SUBSECTION CHANNEL
        CONFIG_SUBSECTION(_("Channel"));

        ImGui::Text(_("Channel style:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("Classic##CHS0"),settings.channelStyle==0)) {
          settings.channelStyle=0;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Line##CHS1"),settings.channelStyle==1)) {
          settings.channelStyle=1;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Round##CHS2"),settings.channelStyle==2)) {
          settings.channelStyle=2;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Split button##CHS3"),settings.channelStyle==3)) {
          settings.channelStyle=3;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Square border##CH42"),settings.channelStyle==4)) {
          settings.channelStyle=4;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Round border##CHS5"),settings.channelStyle==5)) {
          settings.channelStyle=5;
          settingsChanged=true;
        }
        ImGui::Unindent();

        ImGui::Text(_("Channel volume bar:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("None##CHV0"),settings.channelVolStyle==0)) {
          settings.channelVolStyle=0;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Simple##CHV1"),settings.channelVolStyle==1)) {
          settings.channelVolStyle=1;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Stereo##CHV2"),settings.channelVolStyle==2)) {
          settings.channelVolStyle=2;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Real##CHV3"),settings.channelVolStyle==3)) {
          settings.channelVolStyle=3;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Real (stereo)##CHV4"),settings.channelVolStyle==4)) {
          settings.channelVolStyle=4;
          settingsChanged=true;
        }
        ImGui::Unindent();

        ImGui::Text(_("Channel feedback style:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("Off##CHF0"),settings.channelFeedbackStyle==0)) {
          settings.channelFeedbackStyle=0;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Note##CHF1"),settings.channelFeedbackStyle==1)) {
          settings.channelFeedbackStyle=1;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Volume##CHF2"),settings.channelFeedbackStyle==2)) {
          settings.channelFeedbackStyle=2;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Active##CHF3"),settings.channelFeedbackStyle==3)) {
          settings.channelFeedbackStyle=3;
          settingsChanged=true;
        }
        ImGui::Unindent();

        ImGui::Text(_("Channel font:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("Regular##CHFont0"),settings.channelFont==0)) {
          settings.channelFont=0;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Monospace##CHFont1"),settings.channelFont==1)) {
          settings.channelFont=1;
          settingsChanged=true;
        }
        ImGui::Unindent();

        bool channelTextCenterB=settings.channelTextCenter;
        if (ImGui::Checkbox(_("Center channel name"),&channelTextCenterB)) {
          settings.channelTextCenter=channelTextCenterB;
          settingsChanged=true;
        }

        ImGui::Text(_("Channel colors:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("Single##CHC0"),settings.channelColors==0)) {
          settings.channelColors=0;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Channel type##CHC1"),settings.channelColors==1)) {
          settings.channelColors=1;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Instrument type##CHC2"),settings.channelColors==2)) {
          settings.channelColors=2;
          settingsChanged=true;
        }
        ImGui::Unindent();

        ImGui::Text(_("Channel name colors:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("Single##CTC0"),settings.channelTextColors==0)) {
          settings.channelTextColors=0;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Channel type##CTC1"),settings.channelTextColors==1)) {
          settings.channelTextColors=1;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Instrument type##CTC2"),settings.channelTextColors==2)) {
          settings.channelTextColors=2;
          settingsChanged=true;
        }
        ImGui::Unindent();

        // SUBSECTION ASSETS
        CONFIG_SUBSECTION(_("Assets"));
        bool unifiedDataViewB=settings.unifiedDataView;
        if (ImGui::Checkbox(_("Unified instrument/wavetable/sample list"),&unifiedDataViewB)) {
          settings.unifiedDataView=unifiedDataViewB;
          settingsChanged=true;
        }
        if (settings.unifiedDataView) {
          settings.horizontalDataView=0;
        }

        ImGui::BeginDisabled(settings.unifiedDataView);
        bool horizontalDataViewB=settings.horizontalDataView;
        if (ImGui::Checkbox(_("Horizontal instrument/wavetable list"),&horizontalDataViewB)) {
          settings.horizontalDataView=horizontalDataViewB;
          settingsChanged=true;
        }
        ImGui::EndDisabled();

        ImGui::Text(_("Instrument list icon style:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("None##iis0"),settings.insIconsStyle==0)) {
          settings.insIconsStyle=0;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Graphical icons##iis1"),settings.insIconsStyle==1)) {
          settings.insIconsStyle=1;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Letter icons##iis2"),settings.insIconsStyle==2)) {
          settings.insIconsStyle=2;
          settingsChanged=true;
        }
        ImGui::Unindent();

        bool insEditColorizeB=settings.insEditColorize;
        if (ImGui::Checkbox(_("Colorize instrument editor using instrument type"),&insEditColorizeB)) {
          settings.insEditColorize=insEditColorizeB;
          settingsChanged=true;
        }

        // SUBSECTION MACRO EDITOR
        CONFIG_SUBSECTION(_("Macro Editor"));
        ImGui::Text(_("Macro editor layout:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("Unified##mel0"),settings.macroLayout==0)) {
          settings.macroLayout=0;
          settingsChanged=true;
        }
        /*
        if (ImGui::RadioButton(_("Tabs##mel1"),settings.macroLayout==1)) {
          settings.macroLayout=1;
          settingsChanged=true;
        }
        */
        if (ImGui::RadioButton(_("Grid##mel2"),settings.macroLayout==2)) {
          settings.macroLayout=2;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Single (with list)##mel3"),settings.macroLayout==3)) {
          settings.macroLayout=3;
          settingsChanged=true;
        }
        /*
        if (ImGui::RadioButton(_("Single (combo box)##mel4"),settings.macroLayout==4)) {
          settings.macroLayout=4;
          settingsChanged=true;
        }
        */
        ImGui::Unindent();

        bool oldMacroVSliderB=settings.oldMacroVSlider;
        if (ImGui::Checkbox(_("Use classic macro editor vertical slider"),&oldMacroVSliderB)) {
          settings.oldMacroVSlider=oldMacroVSliderB;
          settingsChanged=true;
        }

        ImGui::BeginDisabled(settings.macroLayout==2);
        ImGui::Text(_("Macro step size/horizontal zoom:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("Manual"),settings.autoMacroStepSize==0)) {
          settings.autoMacroStepSize=0;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Automatic per macro"),settings.autoMacroStepSize==1)) {
          settings.autoMacroStepSize=1;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Automatic (use longest macro)"),settings.autoMacroStepSize==2)) {
          settings.autoMacroStepSize=2;
          settingsChanged=true;
        }
        ImGui::Unindent();
        ImGui::EndDisabled();

        // SUBSECTION WAVE EDITOR
        CONFIG_SUBSECTION(_("Wave Editor"));
        bool waveLayoutB=settings.waveLayout;
        if (ImGui::Checkbox(_("Use compact wave editor"),&waveLayoutB)) {
          settings.waveLayout=waveLayoutB;
          settingsChanged=true;
        }

        // SUBSECTION FM EDITOR
        CONFIG_SUBSECTION(_("FM Editor"));
        ImGui::Text(_("FM parameter names:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("Friendly##fmn0"),settings.fmNames==0)) {
          settings.fmNames=0;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Technical##fmn1"),settings.fmNames==1)) {
          settings.fmNames=1;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Technical (alternate)##fmn2"),settings.fmNames==2)) {
          settings.fmNames=2;
          settingsChanged=true;
        }
        ImGui::Unindent();

        bool oplStandardWaveNamesB=settings.oplStandardWaveNames;
        if (ImGui::Checkbox(_("Use standard OPL waveform names"),&oplStandardWaveNamesB)) {
          settings.oplStandardWaveNames=oplStandardWaveNamesB;
          settingsChanged=true;
        }

        ImGui::Text(_("FM parameter editor layout:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("Modern##fml0"),settings.fmLayout==0)) {
          settings.fmLayout=0;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Modern with more labels##fml7"),settings.fmLayout==7)) {
          settings.fmLayout=7;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Compact (2x2, classic)##fml1"),settings.fmLayout==1)) {
          settings.fmLayout=1;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Compact (1x4)##fml2"),settings.fmLayout==2)) {
          settings.fmLayout=2;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Compact (4x1)##fml3"),settings.fmLayout==3)) {
          settings.fmLayout=3;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Alternate (2x2)##fml4"),settings.fmLayout==4)) {
          settings.fmLayout=4;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Alternate (1x4)##fml5"),settings.fmLayout==5)) {
          settings.fmLayout=5;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Alternate (4x1)##fml5"),settings.fmLayout==6)) {
          settings.fmLayout=6;
          settingsChanged=true;
        }
        ImGui::Unindent();

        ImGui::Text(_("Position of Sustain in FM editor:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("Between Decay and Sustain Rate##susp0"),settings.susPosition==0)) {
          settings.susPosition=0;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("After Release Rate##susp1"),settings.susPosition==1)) {
          settings.susPosition=1;
          settingsChanged=true;
        }
        ImGui::BeginDisabled(settings.fmLayout!=0);
        if (ImGui::RadioButton(_("After Release Rate, after spacing##susp2"),settings.susPosition==2)) {
          settings.susPosition=2;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("After TL##susp3"),settings.susPosition==3)) {
          settings.susPosition=3;
          settingsChanged=true;
        }
        ImGui::EndDisabled();
        ImGui::Unindent();

        bool separateFMColorsB=settings.separateFMColors;
        if (ImGui::Checkbox(_("Use separate colors for carriers/modulators in FM editor"),&separateFMColorsB)) {
          settings.separateFMColors=separateFMColorsB;
          settingsChanged=true;
        }

        bool unsignedDetuneB=settings.unsignedDetune;
        if (ImGui::Checkbox(_("Unsigned FM detune values"),&unsignedDetuneB)) {
          settings.unsignedDetune=unsignedDetuneB;
          settingsChanged=true;
        }

        // SUBSECTION MEMORY COMPOSITION
        CONFIG_SUBSECTION(_("Memory Composition"));
        ImGui::Text(_("Chip memory usage unit:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("Bytes##MUU0"),settings.memUsageUnit==0)) {
          settings.memUsageUnit=0;
          settingsChanged=true;
        }
        if (ImGui::RadioButton(_("Kilobytes##MUU1"),settings.memUsageUnit==1)) {
          settings.memUsageUnit=1;
          settingsChanged=true;
        }
        ImGui::Unindent();

        // SUBSECTION OSCILLOSCOPE
        CONFIG_SUBSECTION(_("Oscilloscope"));
        bool oscRoundedCornersB=settings.oscRoundedCorners;
        if (ImGui::Checkbox(_("Rounded corners"),&oscRoundedCornersB)) {
          settings.oscRoundedCorners=oscRoundedCornersB;
          settingsChanged=true;
        }

        bool oscBorderB=settings.oscBorder;
        if (ImGui::Checkbox(_("Border"),&oscBorderB)) {
          settings.oscBorder=oscBorderB;
          settingsChanged=true;
        }

        bool oscMonoB=settings.oscMono;
        if (ImGui::Checkbox(_("Mono"),&oscMonoB)) {
          settings.oscMono=oscMonoB;
          settingsChanged=true;
        }

        bool oscAntiAliasB=settings.oscAntiAlias;
        if (ImGui::Checkbox(_("Anti-aliased"),&oscAntiAliasB)) {
          settings.oscAntiAlias=oscAntiAliasB;
          settingsChanged=true;
        }

        bool oscTakesEntireWindowB=settings.oscTakesEntireWindow;
        if (ImGui::Checkbox(_("Fill entire window"),&oscTakesEntireWindowB)) {
          settings.oscTakesEntireWindow=oscTakesEntireWindowB;
          settingsChanged=true;
        }

        bool oscEscapesBoundaryB=settings.oscEscapesBoundary;
        if (ImGui::Checkbox(_("Waveform goes out of bounds"),&oscEscapesBoundaryB)) {
          settings.oscEscapesBoundary=oscEscapesBoundaryB;
          settingsChanged=true;
        }

        if (ImGui::SliderFloat(_("Line size"),&settings.oscLineSize,0.25f,16.0f,"%.1f")) {
          if (settings.oscLineSize<0.25f) settings.oscLineSize=0.25f;
          if (settings.oscLineSize>16.0f) settings.oscLineSize=16.0f;
          settingsChanged=true;
        } rightClickable

        pushWarningColor(settings.chanOscThreads>cpuCores,settings.chanOscThreads>(cpuCores*2));
        if (ImGui::InputInt(_("Per-channel oscilloscope threads"),&settings.chanOscThreads)) {
          if (settings.chanOscThreads<0) settings.chanOscThreads=0;
          if (settings.chanOscThreads>(cpuCores*3)) settings.chanOscThreads=cpuCores*3;
          if (settings.chanOscThreads>256) settings.chanOscThreads=256;
          settingsChanged=true;
        }
        if (settings.chanOscThreads>=(cpuCores*3)) {
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("you're being silly, aren't you? that's enough."));
          }
        } else if (settings.chanOscThreads>(cpuCores*2)) {
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("what are you doing? stop!"));
          }
        } else if (settings.chanOscThreads>cpuCores) {
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("it is a bad idea to set this number higher than your CPU core count (%d)!"),cpuCores);
          }
        }
        popWarningColor();

        ImGui::Text(_("Oscilloscope rendering engine:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("ImGui line plot"),settings.shaderOsc==0)) {
          settings.shaderOsc=0;
          settingsChanged=true;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("render using Dear ImGui's built-in line drawing functions."));
        }
        if (ImGui::RadioButton(_("GLSL (if available)"),settings.shaderOsc==1)) {
          settings.shaderOsc=1;
          settingsChanged=true;
        }
        if (ImGui::IsItemHovered()) {
#ifdef USE_GLES
          ImGui::SetTooltip(_("render using shaders that run on the graphics card.\nonly available in OpenGL ES 2.0 render backend."));
#else
          ImGui::SetTooltip(_("render using shaders that run on the graphics card.\nonly available in OpenGL 3.0 render backend."));
#endif
        }
        ImGui::Unindent();

        // SUBSECTION SONG COMMENTS
        CONFIG_SUBSECTION(_("Song Comments"));
        bool songNotesWrapB=settings.songNotesWrap;
        if (ImGui::Checkbox(_("Wrap text"), &songNotesWrapB)) {
          settings.songNotesWrap=songNotesWrapB;
          settingsChanged=true;
        }

        // SUBSECTION WINDOWS
        CONFIG_SUBSECTION(_("Windows"));
        bool roundedWindowsB=settings.roundedWindows;
        if (ImGui::Checkbox(_("Rounded window corners"),&roundedWindowsB)) {
          settings.roundedWindows=roundedWindowsB;
          settingsChanged=true;
        }

        bool roundedButtonsB=settings.roundedButtons;
        if (ImGui::Checkbox(_("Rounded buttons"),&roundedButtonsB)) {
          settings.roundedButtons=roundedButtonsB;
          settingsChanged=true;
        }

        bool roundedMenusB=settings.roundedMenus;
        if (ImGui::Checkbox(_("Rounded menu corners"),&roundedMenusB)) {
          settings.roundedMenus=roundedMenusB;
          settingsChanged=true;
        }

        bool roundedTabsB=settings.roundedTabs;
        if (ImGui::Checkbox(_("Rounded tabs"),&roundedTabsB)) {
          settings.roundedTabs=roundedTabsB;
          settingsChanged=true;
        }

        bool roundedScrollbarsB=settings.roundedScrollbars;
        if (ImGui::Checkbox(_("Rounded scrollbars"),&roundedScrollbarsB)) {
          settings.roundedScrollbars=roundedScrollbarsB;
          settingsChanged=true;
        }

        bool frameBordersB=settings.frameBorders;
        if (ImGui::Checkbox(_("Borders around widgets"),&frameBordersB)) {
          settings.frameBorders=frameBordersB;
          settingsChanged=true;
        }

        END_SECTION;
      }
      CONFIG_SECTION(_("Color")) {
        // SUBSECTION COLOR SCHEME
        CONFIG_SUBSECTION(_("Color scheme"));
        if (ImGui::Button(_("Import"))) {
          openFileDialog(GUI_FILE_IMPORT_COLORS);
        }
        ImGui::SameLine();
        if (ImGui::Button(_("Export"))) {
          openFileDialog(GUI_FILE_EXPORT_COLORS);
        }
        ImGui::SameLine();
        if (ImGui::Button(_("Reset defaults"))) {
          showWarning(_("Are you sure you want to reset the color scheme?"),GUI_WARN_RESET_COLORS);
        }
        bool basicColorsB=!settings.basicColors;
        if (ImGui::Checkbox(_("Guru mode"),&basicColorsB)) {
          settings.basicColors=!basicColorsB;
          applyUISettings(false);
          settingsChanged=true;
        }
        if (settings.basicColors) {
          if (ImGui::TreeNode(_("Interface"))) {
            if (ImGui::SliderInt(_("Frame shading"),&settings.guiColorsShading,0,100,"%d%%")) {
              if (settings.guiColorsShading<0) settings.guiColorsShading=0;
              if (settings.guiColorsShading>100) settings.guiColorsShading=100;
              applyUISettings(false);
              settingsChanged=true;
            }
            ImGui::Text(_("Color scheme type:"));
            ImGui::Indent();
            if (ImGui::RadioButton(_("Dark##gcb0"),settings.guiColorsBase==0)) {
              settings.guiColorsBase=0;
              applyUISettings(false);
              settingsChanged=true;
            }
            if (ImGui::RadioButton(_("Light##gcb1"),settings.guiColorsBase==1)) {
              settings.guiColorsBase=1;
              applyUISettings(false);
              settingsChanged=true;
            }
            ImGui::Unindent();

            ImGui::Text(_("Accent colors:"));
            ImGui::Indent();
            UI_COLOR_CONFIG(GUI_COLOR_ACCENT_PRIMARY,_("Primary"));
            UI_COLOR_CONFIG(GUI_COLOR_ACCENT_SECONDARY,_("Secondary"));
            ImGui::Unindent();

            ImGui::TreePop();
          }
        } else {
          if (ImGui::TreeNode(_("Interface"))) {
            if (ImGui::SliderInt(_("Frame shading"),&settings.guiColorsShading,0,100,"%d%%")) {
              if (settings.guiColorsShading<0) settings.guiColorsShading=0;
              if (settings.guiColorsShading>100) settings.guiColorsShading=100;
              applyUISettings(false);
              settingsChanged=true;
            }

            UI_COLOR_CONFIG(GUI_COLOR_BUTTON,_("Button"));
            UI_COLOR_CONFIG(GUI_COLOR_BUTTON_HOVER,_("Button (hovered)"));
            UI_COLOR_CONFIG(GUI_COLOR_BUTTON_ACTIVE,_("Button (active)"));
            UI_COLOR_CONFIG(GUI_COLOR_TAB,_("Tab"));
            UI_COLOR_CONFIG(GUI_COLOR_TAB_HOVER,_("Tab (hovered)"));
            UI_COLOR_CONFIG(GUI_COLOR_TAB_ACTIVE,_("Tab (active)"));
            UI_COLOR_CONFIG(GUI_COLOR_TAB_UNFOCUSED,_("Tab (unfocused)"));
            UI_COLOR_CONFIG(GUI_COLOR_TAB_UNFOCUSED_ACTIVE,_("Tab (unfocused and active)"));
            UI_COLOR_CONFIG(GUI_COLOR_IMGUI_HEADER,_("ImGui header"));
            UI_COLOR_CONFIG(GUI_COLOR_IMGUI_HEADER_HOVER,_("ImGui header (hovered)"));
            UI_COLOR_CONFIG(GUI_COLOR_IMGUI_HEADER_ACTIVE,_("ImGui header (active)"));
            UI_COLOR_CONFIG(GUI_COLOR_RESIZE_GRIP,_("Resize grip"));
            UI_COLOR_CONFIG(GUI_COLOR_RESIZE_GRIP_HOVER,_("Resize grip (hovered)"));
            UI_COLOR_CONFIG(GUI_COLOR_RESIZE_GRIP_ACTIVE,_("Resize grip (active)"));
            UI_COLOR_CONFIG(GUI_COLOR_WIDGET_BACKGROUND,_("Widget background"));
            UI_COLOR_CONFIG(GUI_COLOR_WIDGET_BACKGROUND_HOVER,_("Widget background (hovered)"));
            UI_COLOR_CONFIG(GUI_COLOR_WIDGET_BACKGROUND_ACTIVE,_("Widget background (active)"));
            UI_COLOR_CONFIG(GUI_COLOR_SLIDER_GRAB,_("Slider grab"));
            UI_COLOR_CONFIG(GUI_COLOR_SLIDER_GRAB_ACTIVE,_("Slider grab (active)"));
            UI_COLOR_CONFIG(GUI_COLOR_TITLE_BACKGROUND_ACTIVE,_("Title background (active)"));
            UI_COLOR_CONFIG(GUI_COLOR_CHECK_MARK,_("Checkbox/radio button mark"));
            UI_COLOR_CONFIG(GUI_COLOR_TEXT_SELECTION,_("Text selection"));
            UI_COLOR_CONFIG(GUI_COLOR_PLOT_LINES,_("Line plot"));
            UI_COLOR_CONFIG(GUI_COLOR_PLOT_LINES_HOVER,_("Line plot (hovered)"));
            UI_COLOR_CONFIG(GUI_COLOR_PLOT_HISTOGRAM,_("Histogram plot"));
            UI_COLOR_CONFIG(GUI_COLOR_PLOT_HISTOGRAM_HOVER,_("Histogram plot (hovered)"));
            UI_COLOR_CONFIG(GUI_COLOR_TABLE_ROW_EVEN,_("Table row (even)"));
            UI_COLOR_CONFIG(GUI_COLOR_TABLE_ROW_ODD,_("Table row (odd)"));

            ImGui::TreePop();
          }
        }
        if (ImGui::TreeNode(_("Interface (other)"))) {
          UI_COLOR_CONFIG(GUI_COLOR_BACKGROUND,_("Background"));
          UI_COLOR_CONFIG(GUI_COLOR_FRAME_BACKGROUND,_("Window background"));
          UI_COLOR_CONFIG(GUI_COLOR_FRAME_BACKGROUND_CHILD,_("Sub-window background"));
          UI_COLOR_CONFIG(GUI_COLOR_FRAME_BACKGROUND_POPUP,_("Pop-up background"));
          UI_COLOR_CONFIG(GUI_COLOR_MODAL_BACKDROP,_("Modal backdrop"));
          UI_COLOR_CONFIG(GUI_COLOR_HEADER,_("Header"));
          UI_COLOR_CONFIG(GUI_COLOR_TEXT,_("Text"));
          UI_COLOR_CONFIG(GUI_COLOR_TEXT_DISABLED,_("Text (disabled)"));
          UI_COLOR_CONFIG(GUI_COLOR_TITLE_INACTIVE,_("Title bar (inactive)"));
          UI_COLOR_CONFIG(GUI_COLOR_TITLE_COLLAPSED,_("Title bar (collapsed)"));
          UI_COLOR_CONFIG(GUI_COLOR_MENU_BAR,_("Menu bar"));
          UI_COLOR_CONFIG(GUI_COLOR_BORDER,_("Border"));
          UI_COLOR_CONFIG(GUI_COLOR_BORDER_SHADOW,_("Border shadow"));
          UI_COLOR_CONFIG(GUI_COLOR_SCROLL,_("Scroll bar"));
          UI_COLOR_CONFIG(GUI_COLOR_SCROLL_HOVER,_("Scroll bar (hovered)"));
          UI_COLOR_CONFIG(GUI_COLOR_SCROLL_ACTIVE,_("Scroll bar (clicked)"));
          UI_COLOR_CONFIG(GUI_COLOR_SCROLL_BACKGROUND,_("Scroll bar background"));
          UI_COLOR_CONFIG(GUI_COLOR_SEPARATOR,_("Separator"));
          UI_COLOR_CONFIG(GUI_COLOR_SEPARATOR_HOVER,_("Separator (hover)"));
          UI_COLOR_CONFIG(GUI_COLOR_SEPARATOR_ACTIVE,_("Separator (active)"));
          UI_COLOR_CONFIG(GUI_COLOR_DOCKING_PREVIEW,_("Docking preview"));
          UI_COLOR_CONFIG(GUI_COLOR_DOCKING_EMPTY,_("Docking empty"));
          UI_COLOR_CONFIG(GUI_COLOR_TABLE_HEADER,_("Table header"));
          UI_COLOR_CONFIG(GUI_COLOR_TABLE_BORDER_HARD,_("Table border (hard)"));
          UI_COLOR_CONFIG(GUI_COLOR_TABLE_BORDER_SOFT,_("Table border (soft)"));
          UI_COLOR_CONFIG(GUI_COLOR_DRAG_DROP_TARGET,_("Drag and drop target"));
          UI_COLOR_CONFIG(GUI_COLOR_NAV_WIN_HIGHLIGHT,_("Window switcher (highlight)"));
          UI_COLOR_CONFIG(GUI_COLOR_NAV_WIN_BACKDROP,_("Window switcher backdrop"));
          ImGui::TreePop();
        }
        if (ImGui::TreeNode(_("Miscellaneous"))) {
          UI_COLOR_CONFIG(GUI_COLOR_TOGGLE_ON,_("Toggle on"));
          UI_COLOR_CONFIG(GUI_COLOR_TOGGLE_OFF,_("Toggle off"));
          UI_COLOR_CONFIG(GUI_COLOR_PLAYBACK_STAT,_("Playback status"));
          UI_COLOR_CONFIG(GUI_COLOR_DESTRUCTIVE,_("Destructive hint"));
          UI_COLOR_CONFIG(GUI_COLOR_WARNING,_("Warning hint"));
          UI_COLOR_CONFIG(GUI_COLOR_ERROR,_("Error hint"));
          ImGui::TreePop();
        }
        if (ImGui::TreeNode(_("File Picker (built-in)"))) {
          UI_COLOR_CONFIG(GUI_COLOR_FILE_DIR,_("Directory"));
          UI_COLOR_CONFIG(GUI_COLOR_FILE_SONG_NATIVE,_("Song (native)"));
          UI_COLOR_CONFIG(GUI_COLOR_FILE_SONG_IMPORT,_("Song (import)"));
          UI_COLOR_CONFIG(GUI_COLOR_FILE_INSTR,_("Instrument"));
          UI_COLOR_CONFIG(GUI_COLOR_FILE_AUDIO,_("Audio"));
          UI_COLOR_CONFIG(GUI_COLOR_FILE_WAVE,_("Wavetable"));
          UI_COLOR_CONFIG(GUI_COLOR_FILE_VGM,_("VGM"));
          UI_COLOR_CONFIG(GUI_COLOR_FILE_ZSM,_("ZSM"));
          UI_COLOR_CONFIG(GUI_COLOR_FILE_FONT,_("Font"));
          UI_COLOR_CONFIG(GUI_COLOR_FILE_OTHER,_("Other"));
          ImGui::TreePop();
        }
        if (ImGui::TreeNode(_("Oscilloscope"))) {
          UI_COLOR_CONFIG(GUI_COLOR_OSC_BORDER,_("Border"));
          UI_COLOR_CONFIG(GUI_COLOR_OSC_BG1,_("Background (top-left)"));
          UI_COLOR_CONFIG(GUI_COLOR_OSC_BG2,_("Background (top-right)"));
          UI_COLOR_CONFIG(GUI_COLOR_OSC_BG3,_("Background (bottom-left)"));
          UI_COLOR_CONFIG(GUI_COLOR_OSC_BG4,_("Background (bottom-right)"));
          UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE,_("Waveform"));
          UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE_PEAK,_("Waveform (clip)"));
          UI_COLOR_CONFIG(GUI_COLOR_OSC_REF,_("Reference"));
          UI_COLOR_CONFIG(GUI_COLOR_OSC_GUIDE,_("Guide"));

          if (ImGui::TreeNode(_("Wave (non-mono)"))) {
            UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE_CH0,_("Waveform (1)"));
            UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE_CH1,_("Waveform (2)"));
            UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE_CH2,_("Waveform (3)"));
            UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE_CH3,_("Waveform (4)"));
            UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE_CH4,_("Waveform (5)"));
            UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE_CH5,_("Waveform (6)"));
            UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE_CH6,_("Waveform (7)"));
            UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE_CH7,_("Waveform (8)"));
            UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE_CH8,_("Waveform (9)"));
            UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE_CH9,_("Waveform (10)"));
            UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE_CH10,_("Waveform (11)"));
            UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE_CH11,_("Waveform (12)"));
            UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE_CH12,_("Waveform (13)"));
            UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE_CH13,_("Waveform (14)"));
            UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE_CH14,_("Waveform (15)"));
            UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE_CH15,_("Waveform (16)"));
            ImGui::TreePop();
          }
          ImGui::TreePop();
        }
        if (ImGui::TreeNode(_("Volume Meter"))) {
          UI_COLOR_CONFIG(GUI_COLOR_VOLMETER_LOW,_("Low"));
          UI_COLOR_CONFIG(GUI_COLOR_VOLMETER_HIGH,_("High"));
          UI_COLOR_CONFIG(GUI_COLOR_VOLMETER_PEAK,_("Clip"));
          ImGui::TreePop();
        }
        if (ImGui::TreeNode(_("Orders"))) {
          UI_COLOR_CONFIG(GUI_COLOR_ORDER_ROW_INDEX,_("Order number"));
          UI_COLOR_CONFIG(GUI_COLOR_ORDER_ACTIVE,_("Playing order background"));
          UI_COLOR_CONFIG(GUI_COLOR_SONG_LOOP,_("Song loop"));
          UI_COLOR_CONFIG(GUI_COLOR_ORDER_SELECTED,_("Selected order"));
          UI_COLOR_CONFIG(GUI_COLOR_ORDER_SIMILAR,_("Similar patterns"));
          UI_COLOR_CONFIG(GUI_COLOR_ORDER_INACTIVE,_("Inactive patterns"));
          ImGui::TreePop();
        }
        if (ImGui::TreeNode(_("Envelope View"))) {
          UI_COLOR_CONFIG(GUI_COLOR_FM_ENVELOPE,_("Envelope"));
          UI_COLOR_CONFIG(GUI_COLOR_FM_ENVELOPE_SUS_GUIDE,_("Sustain guide"));
          UI_COLOR_CONFIG(GUI_COLOR_FM_ENVELOPE_RELEASE,_("Release"));

          ImGui::TreePop();
        }
        if (ImGui::TreeNode(_("FM Editor"))) {
          UI_COLOR_CONFIG(GUI_COLOR_FM_ALG_BG,_("Algorithm background"));
          UI_COLOR_CONFIG(GUI_COLOR_FM_ALG_LINE,_("Algorithm lines"));
          UI_COLOR_CONFIG(GUI_COLOR_FM_MOD,_("Modulator"));
          UI_COLOR_CONFIG(GUI_COLOR_FM_CAR,_("Carrier"));

          UI_COLOR_CONFIG(GUI_COLOR_FM_SSG,_("SSG-EG"));
          UI_COLOR_CONFIG(GUI_COLOR_FM_WAVE,_("Waveform"));

          ImGui::TextWrapped(_("(the following colors only apply when \"Use separate colors for carriers/modulators in FM editor\" is on!)"));

          UI_COLOR_CONFIG(GUI_COLOR_FM_PRIMARY_MOD,_("Mod. accent (primary)"));
          UI_COLOR_CONFIG(GUI_COLOR_FM_SECONDARY_MOD,_("Mod. accent (secondary)"));
          UI_COLOR_CONFIG(GUI_COLOR_FM_BORDER_MOD,_("Mod. border"));
          UI_COLOR_CONFIG(GUI_COLOR_FM_BORDER_SHADOW_MOD,_("Mod. border shadow"));

          UI_COLOR_CONFIG(GUI_COLOR_FM_PRIMARY_CAR,_("Car. accent (primary)"));
          UI_COLOR_CONFIG(GUI_COLOR_FM_SECONDARY_CAR,_("Car. accent (secondary)"));
          UI_COLOR_CONFIG(GUI_COLOR_FM_BORDER_CAR,_("Car. border"));
          UI_COLOR_CONFIG(GUI_COLOR_FM_BORDER_SHADOW_CAR,_("Car. border shadow"));

          ImGui::TreePop();
        }
        if (ImGui::TreeNode(_("Macro Editor"))) {
          UI_COLOR_CONFIG(GUI_COLOR_MACRO_VOLUME,_("Volume"));
          UI_COLOR_CONFIG(GUI_COLOR_MACRO_PITCH,_("Pitch"));
          UI_COLOR_CONFIG(GUI_COLOR_MACRO_WAVE,_("Wave"));
          UI_COLOR_CONFIG(GUI_COLOR_MACRO_NOISE,_("Noise"));
          UI_COLOR_CONFIG(GUI_COLOR_MACRO_FILTER,_("Filter"));
          UI_COLOR_CONFIG(GUI_COLOR_MACRO_ENVELOPE,_("Envelope"));
          UI_COLOR_CONFIG(GUI_COLOR_MACRO_GLOBAL,_("Global Parameter"));
          UI_COLOR_CONFIG(GUI_COLOR_MACRO_OTHER,_("Other"));
          UI_COLOR_CONFIG(GUI_COLOR_MACRO_HIGHLIGHT,_("Step Highlight"));
          ImGui::TreePop();
        }
        if (ImGui::TreeNode(_("Instrument Types"))) {
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_FM,_("FM (OPN)"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_STD,_("SN76489/Sega PSG"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_T6W28,_("T6W28"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_GB,_("Game Boy"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_C64,_("C64"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_AMIGA,_("Amiga/Generic Sample"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_PCE,_("PC Engine"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_AY,_("AY-3-8910/SSG"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_AY8930,_("AY8930"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_TIA,_("TIA"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_SAA1099,_("SAA1099"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_VIC,_("VIC"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_PET,_("PET"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_VRC6,_("VRC6"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_VRC6_SAW,_("VRC6 (saw)"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_OPLL,_("FM (OPLL)"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_OPL,_("FM (OPL)"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_FDS,_("FDS"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_VBOY,_("Virtual Boy"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_N163,_("Namco 163"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_SCC,_("Konami SCC"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_OPZ,_("FM (OPZ)"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_POKEY,_("POKEY"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_BEEPER,_("PC Beeper"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_SWAN,_("WonderSwan"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_MIKEY,_("Lynx"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_VERA,_("VERA"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_X1_010,_("X1-010"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_ES5506,_("ES5506"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_MULTIPCM,_("MultiPCM"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_SNES,_("SNES"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_SU,_("Sound Unit"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_NAMCO,_("Namco WSG"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_OPL_DRUMS,_("FM (OPL Drums)"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_OPM,_("FM (OPM)"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_NES,_("NES"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_MSM6258,_("MSM6258"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_MSM6295,_("MSM6295"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_ADPCMA,_("ADPCM-A"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_ADPCMB,_("ADPCM-B"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_SEGAPCM,_("Sega PCM"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_QSOUND,_("QSound"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_YMZ280B,_("YMZ280B"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_RF5C68,_("RF5C68"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_MSM5232,_("MSM5232"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_K007232,_("K007232"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_GA20,_("GA20"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_POKEMINI,_("Pokémon Mini"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_SM8521,_("SM8521"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_PV1000,_("PV-1000"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_K053260,_("K053260"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_TED,_("TED"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_C140,_("C140"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_C219,_("C219"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_ESFM,_("ESFM"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_POWERNOISE,_("PowerNoise (noise)"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_POWERNOISE_SLOPE,_("PowerNoise (slope)"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_DAVE,_("Dave"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_NDS,_("Nintendo DS"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_GBA_DMA,_("GBA DMA"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_GBA_MINMOD,_("GBA MinMod"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_BIFURCATOR,_("Bifurcator"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_SID2,_("SID2"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_SUPERVISION,_("Supervision"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_UPD1771C,_("μPD1771C"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_SID3,_("SID3"));
          UI_COLOR_CONFIG(GUI_COLOR_INSTR_UNKNOWN,_("Other/Unknown"));
          ImGui::TreePop();
        }
        if (ImGui::TreeNode(_("Channel"))) {
          UI_COLOR_CONFIG(GUI_COLOR_CHANNEL_BG,_("Single color (background)"));
          UI_COLOR_CONFIG(GUI_COLOR_CHANNEL_FG,_("Single color (text)"));
          UI_COLOR_CONFIG(GUI_COLOR_CHANNEL_FM,_("FM"));
          UI_COLOR_CONFIG(GUI_COLOR_CHANNEL_PULSE,_("Pulse"));
          UI_COLOR_CONFIG(GUI_COLOR_CHANNEL_NOISE,_("Noise"));
          UI_COLOR_CONFIG(GUI_COLOR_CHANNEL_PCM,_("PCM"));
          UI_COLOR_CONFIG(GUI_COLOR_CHANNEL_WAVE,_("Wave"));
          UI_COLOR_CONFIG(GUI_COLOR_CHANNEL_OP,_("FM operator"));
          UI_COLOR_CONFIG(GUI_COLOR_CHANNEL_MUTED,_("Muted"));
          ImGui::TreePop();
        }
        if (ImGui::TreeNode(_("Pattern"))) {
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_PLAY_HEAD,_("Playhead"));
          UI_COLOR_CONFIG(GUI_COLOR_EDITING,_("Editing"));
          UI_COLOR_CONFIG(GUI_COLOR_EDITING_CLONE,_("Editing (will clone)"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_CURSOR,_("Cursor"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_CURSOR_HOVER,_("Cursor (hovered)"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_CURSOR_ACTIVE,_("Cursor (clicked)"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_SELECTION,_("Selection"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_SELECTION_HOVER,_("Selection (hovered)"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_SELECTION_ACTIVE,_("Selection (clicked)"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_HI_1,_("Highlight 1"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_HI_2,_("Highlight 2"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_ROW_INDEX,_("Row number"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_ROW_INDEX_HI1,_("Row number (highlight 1)"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_ROW_INDEX_HI2,_("Row number (highlight 2)"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_ACTIVE,_("Note"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_ACTIVE_HI1,_("Note (highlight 1)"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_ACTIVE_HI2,_("Note (highlight 2)"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_INACTIVE,_("Blank"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_INACTIVE_HI1,_("Blank (highlight 1)"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_INACTIVE_HI2,_("Blank (highlight 2)"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_INS,_("Instrument"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_INS_WARN,_("Instrument (invalid type)"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_INS_ERROR,_("Instrument (out of range)"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_VOLUME_MIN,_("Volume (0%)"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_VOLUME_HALF,_("Volume (50%)"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_VOLUME_MAX,_("Volume (100%)"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_EFFECT_INVALID,_("Invalid effect"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_EFFECT_PITCH,_("Pitch effect"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_EFFECT_VOLUME,_("Volume effect"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_EFFECT_PANNING,_("Panning effect"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_EFFECT_SONG,_("Song effect"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_EFFECT_TIME,_("Time effect"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_EFFECT_SPEED,_("Speed effect"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_EFFECT_SYS_PRIMARY,_("Primary specific effect"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_EFFECT_SYS_SECONDARY,_("Secondary specific effect"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_EFFECT_MISC,_("Miscellaneous"));
          UI_COLOR_CONFIG(GUI_COLOR_EE_VALUE,_("External command output"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_OFF,_("Status: off/disabled"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_REL,_("Status: off + macro rel"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_REL_ON,_("Status: on + macro rel"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_ON,_("Status: on"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_VOLUME,_("Status: volume"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_PITCH,_("Status: pitch"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_PANNING,_("Status: panning"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_SYS1,_("Status: chip (primary)"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_SYS2,_("Status: chip (secondary)"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_MIXING,_("Status: mixing"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_DSP,_("Status: DSP effect"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_NOTE,_("Status: note altering"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_MISC1,_("Status: misc color 1"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_MISC2,_("Status: misc color 2"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_MISC3,_("Status: misc color 3"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_ATTACK,_("Status: attack"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_DECAY,_("Status: decay"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_SUSTAIN,_("Status: sustain"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_RELEASE,_("Status: release"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_DEC_LINEAR,_("Status: decrease linear"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_DEC_EXP,_("Status: decrease exp"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_INC,_("Status: increase"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_BENT,_("Status: bent"));
          UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_DIRECT,_("Status: direct"));
          ImGui::TreePop();
        }
        if (ImGui::TreeNode(_("Sample Editor"))) {
          UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_BG,_("Background"));
          UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_FG,_("Waveform"));
          UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_TIME_BG,_("Time background"));
          UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_TIME_FG,_("Time text"));
          UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_LOOP,_("Loop region"));
          UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_CENTER,_("Center guide"));
          UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_GRID,_("Grid"));
          UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_SEL,_("Selection"));
          UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_SEL_POINT,_("Selection points"));
          UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_NEEDLE,_("Preview needle"));
          UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_NEEDLE_PLAYING,_("Playing needles"));
          UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_LOOP_POINT,_("Loop markers"));
          UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_CHIP_DISABLED,_("Chip select: disabled"));
          UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_CHIP_ENABLED,_("Chip select: enabled"));
          UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_CHIP_WARNING,_("Chip select: enabled (failure)"));
          ImGui::TreePop();
        }
        if (ImGui::TreeNode(_("Pattern Manager"))) {
          UI_COLOR_CONFIG(GUI_COLOR_PAT_MANAGER_NULL,_("Unallocated"));
          UI_COLOR_CONFIG(GUI_COLOR_PAT_MANAGER_UNUSED,_("Unused"));
          UI_COLOR_CONFIG(GUI_COLOR_PAT_MANAGER_USED,_("Used"));
          UI_COLOR_CONFIG(GUI_COLOR_PAT_MANAGER_OVERUSED,_("Overused"));
          UI_COLOR_CONFIG(GUI_COLOR_PAT_MANAGER_EXTREMELY_OVERUSED,_("Really overused"));
          UI_COLOR_CONFIG(GUI_COLOR_PAT_MANAGER_COMBO_BREAKER,_("Combo Breaker"));
          ImGui::TreePop();
        }
        if (ImGui::TreeNode(_("Piano"))) {
          UI_COLOR_CONFIG(GUI_COLOR_PIANO_BACKGROUND,_("Background"));
          UI_COLOR_CONFIG(GUI_COLOR_PIANO_KEY_TOP,_("Upper key"));
          UI_COLOR_CONFIG(GUI_COLOR_PIANO_KEY_TOP_HIT,_("Upper key (feedback)"));
          UI_COLOR_CONFIG(GUI_COLOR_PIANO_KEY_TOP_ACTIVE,_("Upper key (pressed)"));
          UI_COLOR_CONFIG(GUI_COLOR_PIANO_KEY_BOTTOM,_("Lower key"));
          UI_COLOR_CONFIG(GUI_COLOR_PIANO_KEY_BOTTOM_HIT,_("Lower key (feedback)"));
          UI_COLOR_CONFIG(GUI_COLOR_PIANO_KEY_BOTTOM_ACTIVE,_("Lower key (pressed)"));
          ImGui::TreePop();
        }
        if (ImGui::TreeNode(_("Clock"))) {
          UI_COLOR_CONFIG(GUI_COLOR_CLOCK_TEXT,_("Clock text"));
          UI_COLOR_CONFIG(GUI_COLOR_CLOCK_BEAT_LOW,_("Beat (off)"));
          UI_COLOR_CONFIG(GUI_COLOR_CLOCK_BEAT_HIGH,_("Beat (on)"));

          ImGui::TreePop();
        }
        if (ImGui::TreeNode(_("Patchbay"))) {
          UI_COLOR_CONFIG(GUI_COLOR_PATCHBAY_PORTSET,_("PortSet"));
          UI_COLOR_CONFIG(GUI_COLOR_PATCHBAY_PORT,_("Port"));
          UI_COLOR_CONFIG(GUI_COLOR_PATCHBAY_PORT_HIDDEN,_("Port (hidden/unavailable)"));
          UI_COLOR_CONFIG(GUI_COLOR_PATCHBAY_CONNECTION,_("Connection (selected)"));
          UI_COLOR_CONFIG(GUI_COLOR_PATCHBAY_CONNECTION_BG,_("Connection (other)"));

          ImGui::TreePop();
        }
        if (ImGui::TreeNode(_("Memory Composition"))) {
          UI_COLOR_CONFIG(GUI_COLOR_MEMORY_BG,_("Background"));
          UI_COLOR_CONFIG(GUI_COLOR_MEMORY_DATA,_("Waveform data"));
          UI_COLOR_CONFIG(GUI_COLOR_MEMORY_FREE,_("Unknown"));
          //UI_COLOR_CONFIG(GUI_COLOR_MEMORY_PADDING,_(""));
          UI_COLOR_CONFIG(GUI_COLOR_MEMORY_RESERVED,_("Reserved"));
          UI_COLOR_CONFIG(GUI_COLOR_MEMORY_SAMPLE,_("Sample"));
          UI_COLOR_CONFIG(GUI_COLOR_MEMORY_SAMPLE_ALT1,_("Sample (alternate 1)"));
          UI_COLOR_CONFIG(GUI_COLOR_MEMORY_SAMPLE_ALT2,_("Sample (alternate 2)"));
          UI_COLOR_CONFIG(GUI_COLOR_MEMORY_SAMPLE_ALT3,_("Sample (alternate 3)"));
          UI_COLOR_CONFIG(GUI_COLOR_MEMORY_WAVE_RAM,_("Wave RAM"));
          UI_COLOR_CONFIG(GUI_COLOR_MEMORY_WAVE_STATIC,_("Wavetable (static)"));
          UI_COLOR_CONFIG(GUI_COLOR_MEMORY_ECHO,_("Echo buffer"));
          UI_COLOR_CONFIG(GUI_COLOR_MEMORY_N163_LOAD,_("Namco 163 load pos"));
          UI_COLOR_CONFIG(GUI_COLOR_MEMORY_N163_PLAY,_("Namco 163 play pos"));
          UI_COLOR_CONFIG(GUI_COLOR_MEMORY_BANK0,_("Sample (bank 0)"));
          UI_COLOR_CONFIG(GUI_COLOR_MEMORY_BANK1,_("Sample (bank 1)"));
          UI_COLOR_CONFIG(GUI_COLOR_MEMORY_BANK2,_("Sample (bank 2)"));
          UI_COLOR_CONFIG(GUI_COLOR_MEMORY_BANK3,_("Sample (bank 3)"));
          UI_COLOR_CONFIG(GUI_COLOR_MEMORY_BANK4,_("Sample (bank 4)"));
          UI_COLOR_CONFIG(GUI_COLOR_MEMORY_BANK5,_("Sample (bank 5)"));
          UI_COLOR_CONFIG(GUI_COLOR_MEMORY_BANK6,_("Sample (bank 6)"));
          UI_COLOR_CONFIG(GUI_COLOR_MEMORY_BANK7,_("Sample (bank 7)"));

          ImGui::TreePop();
        }
        if (ImGui::TreeNode(_("Log Viewer"))) {
          UI_COLOR_CONFIG(GUI_COLOR_LOGLEVEL_ERROR,_("Log level: Error"));
          UI_COLOR_CONFIG(GUI_COLOR_LOGLEVEL_WARNING,_("Log level: Warning"));
          UI_COLOR_CONFIG(GUI_COLOR_LOGLEVEL_INFO,_("Log level: Info"));
          UI_COLOR_CONFIG(GUI_COLOR_LOGLEVEL_DEBUG,_("Log level: Debug"));
          UI_COLOR_CONFIG(GUI_COLOR_LOGLEVEL_TRACE,_("Log level: Trace/Verbose"));
          ImGui::TreePop();
        }
        END_SECTION;
      }
      CONFIG_SECTION(_("Backup")) {
        // SUBSECTION SETTINGS
        CONFIG_SUBSECTION(_("Configuration"));

        bool backupEnableB=settings.backupEnable;
        if (ImGui::Checkbox(_("Enable backup system"),&backupEnableB)) {
          settings.backupEnable=backupEnableB;
          settingsChanged=true;
        }

        if (ImGui::InputInt(_("Interval (in seconds)"),&settings.backupInterval)) {
          if (settings.backupInterval<10) settings.backupInterval=10;
          if (settings.backupInterval>86400) settings.backupInterval=86400;
        }

        if (ImGui::InputInt(_("Backups per file"),&settings.backupMaxCopies)) {
          if (settings.backupMaxCopies<1) settings.backupMaxCopies=1;
          if (settings.backupMaxCopies>100) settings.backupMaxCopies=100;
        }

        // SUBSECTION SETTINGS
        CONFIG_SUBSECTION(_("Backup Management"));
        bool purgeDateChanged=false;

        ImGui::AlignTextToFramePadding();
        ImGui::Text(_("Purge before:"));
        ImGui::SameLine();
        ImGui::SetNextItemWidth(60.0f*dpiScale);
        if (ImGui::InputInt("##PYear",&purgeYear,0,0)) purgeDateChanged=true;
        ImGui::SameLine();
        ImGui::SetNextItemWidth(40.0f*dpiScale);
        if (ImGui::InputInt("##PMonth",&purgeMonth,0,0)) purgeDateChanged=true;
        ImGui::SameLine();
        ImGui::SetNextItemWidth(40.0f*dpiScale);
        if (ImGui::InputInt("##PDay",&purgeDay,0,0)) purgeDateChanged=true;

        if (purgeDateChanged) {
          // check month/day validity
          time_t thisMakesNoSense=time(NULL);
          bool tmFailed=false;
          struct tm curTime;
#ifdef _WIN32
          struct tm* tempTM=localtime(&thisMakesNoSense);
          if (tempTM==NULL) {
            memset(&curTime,0,sizeof(struct tm));
            tmFailed=true;
          } else {
            memcpy(&curTime,tempTM,sizeof(struct tm));
          }
#else
          if (localtime_r(&thisMakesNoSense,&curTime)==NULL) {
            memset(&curTime,0,sizeof(struct tm));
            tmFailed=true;
          }
#endif

          // don't allow dates in the future
          if (!tmFailed) {
            int curYear=curTime.tm_year+1900;
            int curMonth=curTime.tm_mon+1;
            int curDay=curTime.tm_mday;

            if (purgeYear<1) purgeYear=1;
            if (purgeYear>curYear) purgeYear=curYear;

            if (purgeYear==curYear) {
              if (purgeMonth>curMonth) purgeMonth=curMonth;

              if (purgeMonth==curMonth) {
                if (purgeDay>curDay) purgeDay=curDay;
              }
            }
          }

          // general checks
          if (purgeYear<1) purgeYear=1;
          if (purgeMonth<1) purgeMonth=1;
          if (purgeMonth>12) purgeMonth=12;
          if (purgeDay<1) purgeDay=1;

          // 1752 calendar alignment
          if (purgeYear==1752 && purgeMonth==9) {
            if (purgeDay>2 && purgeDay<14) purgeDay=2;
          }
          if (purgeMonth==2) {
            // leap year
            if ((purgeYear&3)==0 && ((purgeYear%100)!=0 || (purgeYear%400)==0)) {
              if (purgeDay>29) purgeDay=29;
            } else {
              if (purgeDay>28) purgeDay=28;
            }
          } else if (purgeMonth==1 || purgeMonth==3 || purgeMonth==5 || purgeMonth==7 || purgeMonth==8 || purgeMonth==10 || purgeMonth==12) {
            if (purgeDay>31) purgeDay=31;
          } else {
            if (purgeDay>30) purgeDay=30;
          }
        }

        ImGui::SameLine();
        if (ImGui::Button(_("Go##PDate"))) {
          purgeBackups(purgeYear,purgeMonth,purgeDay);
        }

        backupEntryLock.lock();
        ImGui::AlignTextToFramePadding();
        if (totalBackupSize>=(1ULL<<50ULL)) {
          ImGui::Text(_("%" PRIu64 "PB used"),totalBackupSize>>50);
        } else if (totalBackupSize>=(1ULL<<40ULL)) {
          ImGui::Text(_("%" PRIu64 "TB used"),totalBackupSize>>40);
        } else if (totalBackupSize>=(1ULL<<30ULL)) {
          ImGui::Text(_("%" PRIu64 "GB used"),totalBackupSize>>30);
        } else if (totalBackupSize>=(1ULL<<20ULL)) {
          ImGui::Text(_("%" PRIu64 "MB used"),totalBackupSize>>20);
        } else if (totalBackupSize>=(1ULL<<10ULL)) {
          ImGui::Text(_("%" PRIu64 "KB used"),totalBackupSize>>10);
        } else {
          ImGui::Text(_("%" PRIu64 " bytes used"),totalBackupSize);
        }

        ImGui::SameLine();

        if (ImGui::Button(_("Refresh"))) {
          refreshBackups=true;
        }
        ImGui::SameLine();
        if (ImGui::Button(_("Delete all"))) {
          purgeBackups(0,0,0);
        }

        if (ImGui::BeginTable("BackupList",3,ImGuiTableFlags_ScrollY|ImGuiTableFlags_Borders)) {
          ImGui::TableSetupColumn(_("Name"),ImGuiTableColumnFlags_WidthStretch,0.6f);
          ImGui::TableSetupColumn(_("Size"),ImGuiTableColumnFlags_WidthStretch,0.15f);
          ImGui::TableSetupColumn(_("Latest"),ImGuiTableColumnFlags_WidthStretch,0.25f);

          ImGui::TableHeadersRow();

          for (FurnaceGUIBackupEntry& i: backupEntries) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(i.name.c_str());
            ImGui::TableNextColumn();
            if (i.size>=(1ULL<<50ULL)) {
              ImGui::Text(_("%" PRIu64 "P"),i.size>>50);
            } else if (i.size>=(1ULL<<40ULL)) {
              ImGui::Text(_("%" PRIu64 "T"),i.size>>40);
            } else if (i.size>=(1ULL<<30ULL)) {
              ImGui::Text(_("%" PRIu64 "G"),i.size>>30);
            } else if (i.size>=(1ULL<<20ULL)) {
              ImGui::Text(_("%" PRIu64 "M"),i.size>>20);
            } else if (i.size>=(1ULL<<10ULL)) {
              ImGui::Text(_("%" PRIu64 "K"),i.size>>10);
            } else {
              ImGui::Text(_("%" PRIu64 ""),i.size);
            }
            ImGui::TableNextColumn();
            ImGui::Text("%d-%02d-%02d",i.lastEntryTime.tm_year+1900,i.lastEntryTime.tm_mon+1,i.lastEntryTime.tm_mday);
          }

          ImGui::EndTable();
        }
        backupEntryLock.unlock();
        if (refreshBackups) {
          refreshBackups=false;
          if (backupEntryTask.valid()) backupEntryTask.get();
          backupEntryTask=std::async(std::launch::async,[this]() -> bool {
            backupEntryLock.lock();
            backupEntries.clear();
            totalBackupSize=0;
            backupEntryLock.unlock();

#ifdef _WIN32
            String findPath=backupPath+String(DIR_SEPARATOR_STR)+String("*.fur");
            WString findPathW=utf8To16(findPath.c_str());
            WIN32_FIND_DATAW next;
            HANDLE backDir=FindFirstFileW(findPathW.c_str(),&next);
            if (backDir!=INVALID_HANDLE_VALUE) {
              do {
                FurnaceGUIBackupEntry nextEntry;
                String cFileNameU=utf16To8(next.cFileName);
                if (!splitBackupName(cFileNameU.c_str(),nextEntry.name,nextEntry.lastEntryTime)) continue;

                nextEntry.size=(((uint64_t)next.nFileSizeHigh)<<32)|next.nFileSizeLow;

                backupEntryLock.lock();
                backupEntries.push_back(nextEntry);
                totalBackupSize+=nextEntry.size;
                backupEntryLock.unlock();
              } while (FindNextFileW(backDir,&next)!=0);
              FindClose(backDir);
            }
#else
            DIR* backDir=opendir(backupPath.c_str());
            if (backDir==NULL) {
              logW("could not open backups dir!");
              return false;
            }
            while (true) {
              FurnaceGUIBackupEntry nextEntry;
              struct stat nextStat;
              struct dirent* next=readdir(backDir);
              if (next==NULL) break;
              if (strcmp(next->d_name,".")==0) continue;
              if (strcmp(next->d_name,"..")==0) continue;
              if (!splitBackupName(next->d_name,nextEntry.name,nextEntry.lastEntryTime)) continue;

              String nextPath=backupPath+DIR_SEPARATOR_STR+next->d_name;

              if (stat(nextPath.c_str(),&nextStat)>=0) {
                nextEntry.size=nextStat.st_size;
              }

              backupEntryLock.lock();
              backupEntries.push_back(nextEntry);
              totalBackupSize+=nextEntry.size;
              backupEntryLock.unlock();
            }
            closedir(backDir);
#endif

            // sort and merge
            backupEntryLock.lock();
            std::sort(backupEntries.begin(),backupEntries.end(),[](const FurnaceGUIBackupEntry& a, const FurnaceGUIBackupEntry& b) -> bool {
              int sc=strcmp(a.name.c_str(),b.name.c_str());
              if (sc==0) {
                if (a.lastEntryTime.tm_year==b.lastEntryTime.tm_year) {
                  if (a.lastEntryTime.tm_mon==b.lastEntryTime.tm_mon) {
                    if (a.lastEntryTime.tm_mday==b.lastEntryTime.tm_mday) {
                      if (a.lastEntryTime.tm_hour==b.lastEntryTime.tm_hour) {
                        if (a.lastEntryTime.tm_min==b.lastEntryTime.tm_min) {
                          return (a.lastEntryTime.tm_sec<b.lastEntryTime.tm_sec);
                        } else {
                          return (a.lastEntryTime.tm_min<b.lastEntryTime.tm_min);
                        }
                      } else {
                        return (a.lastEntryTime.tm_hour<b.lastEntryTime.tm_hour);
                      }
                    } else {
                      return (a.lastEntryTime.tm_mday<b.lastEntryTime.tm_mday);
                    }
                  } else {
                    return (a.lastEntryTime.tm_mon<b.lastEntryTime.tm_mon);
                  }
                } else {
                  return (a.lastEntryTime.tm_year<b.lastEntryTime.tm_year);
                }
              }

              return sc<0;
            });
            for (size_t i=1; i<backupEntries.size(); i++) {
              FurnaceGUIBackupEntry& prevEntry=backupEntries[i-1];
              FurnaceGUIBackupEntry& thisEntry=backupEntries[i];

              if (thisEntry.name==prevEntry.name) {
                prevEntry.size+=thisEntry.size;
                backupEntries.erase(backupEntries.begin()+i);
                i--;
              }
            }
            backupEntryLock.unlock();
            return true;
          });
        }

        END_SECTION;
      }
      if (nonLatchNibble) {
        // ok, so you decided to read the code.
        // these are the cheat codes:
        // "Debug" - toggles mobile UI
        // "Nice Amiga cover of the song!" - enables hidden systems (YMU759/Dummy)
        // "42 63" - enables all instrument types
        // "4-bit FDS" - enables partial pitch linearity option
        // "Power of the Chip" - enables options for multi-threaded audio
        // "btcdbcb" - use modern UI padding
        // "????" - enables stuff
        CONFIG_SECTION(_("Cheat Codes")) {
          // SUBSECTION ENTER CODE:
          CONFIG_SUBSECTION(_("Enter code:"));
          ImGui::InputText("##CheatCode",&mmlString[31]);
          if (ImGui::Button(_("Submit"))) {
            unsigned int checker=0x11111111;
            unsigned int checker1=0;
            int index=0;
            mmlString[30]=_("invalid code");

            for (char& i: mmlString[31]) {
              checker^=((unsigned int)i)<<index;
              checker1+=i;
              checker=(checker>>1|(((checker)^(checker>>2)^(checker>>3)^(checker>>5))&1)<<31);
              checker1<<=1;
              index=(index+1)&31;
            }
            if (checker==0x90888b65 && checker1==0x1482) {
              mmlString[30]=_("toggled alternate UI");
              toggleMobileUI(!mobileUI);
            }
            if (checker==0x5a42a113 && checker1==0xe4ef451e) {
              mmlString[30]=_(":smile: :star_struck: :sunglasses: :ok_hand:");
              settings.hiddenSystems=!settings.hiddenSystems;
            }
            if (checker==0x3affa803 && checker1==0x37db2520) {
              mmlString[30]=_("now cutting FM chip costs");
              settings.mswEnabled=!settings.mswEnabled;
            }
            if (checker==0xe888896b && checker1==0xbde) {
              mmlString[30]=_("enabled all instrument types");
              settings.displayAllInsTypes=!settings.displayAllInsTypes;
            }
            if (checker==0x3f88abcc && checker1==0xf4a6) {
              mmlString[30]=_("OK, if I bring your Partial pitch linearity will you stop bothering me?");
              settings.displayPartial=1;
            }
            if (checker==0x94222d83 && checker1==0x6600) {
              mmlString[30]=_("enabled \"comfortable\" mode");
              ImGuiStyle& sty=ImGui::GetStyle();
              sty.FramePadding=ImVec2(20.0f*dpiScale,20.0f*dpiScale);
              sty.ItemSpacing=ImVec2(10.0f*dpiScale,10.0f*dpiScale);
              sty.ItemInnerSpacing=ImVec2(10.0f*dpiScale,10.0f*dpiScale);
              settingsOpen=false;
            }

            mmlString[31]="";
          }
          ImGui::Text("%s",mmlString[30].c_str());

          END_SECTION;
        }
      }
      ImGui::EndTabBar();
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
      settingsOpen=true;
      willCommit=true;
      settingsChanged=false;
    }
    ImGui::EndDisabled();
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_SETTINGS;
  ImGui::End();
}

void FurnaceGUI::drawKeybindSettingsTableRow(FurnaceGUIActions actionIdx) {
  ImGui::TableNextRow();
  ImGui::TableNextColumn();
  ImGui::AlignTextToFramePadding();
  ImGui::TextUnformatted(guiActions[actionIdx].friendlyName);
  ImGui::TableNextColumn();
  ImGui::PushID(actionIdx);
  for (size_t i=0; i<actionKeys[actionIdx].size()+1; i++) {
    ImGui::PushID(i);
    if (i>0) ImGui::SameLine();
    bool isPending=bindSetPending && bindSetTarget==actionIdx && bindSetTargetIdx==(int)i;
    if (i<actionKeys[actionIdx].size()) {
      if (ImGui::Button(isPending?_N("Press key..."):getKeyName(actionKeys[actionIdx][i]).c_str())) {
        promptKey(actionIdx,i);
        settingsChanged=true;
      }
      bool rightClicked=ImGui::IsItemClicked(ImGuiMouseButton_Right);
      if (!rightClicked) {
        ImGui::SameLine(0.0f, 1.0f);
      }
      if (rightClicked || ImGui::Button(ICON_FA_TIMES)) {
        actionKeys[actionIdx].erase(actionKeys[actionIdx].begin()+i);
        if (isPending) {
          bindSetActive=false;
          bindSetPending=false;
        }
        parseKeybinds();
      }
    } else {
      if (ImGui::Button(isPending?_N("Press key..."):"+")) {
        promptKey(actionIdx,i);
        settingsChanged=true;
      }
    }
    ImGui::PopID(); // i
  }
  ImGui::PopID(); // action
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
    settings.glSetBS=conf.getInt("glSetBS",0);
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
    settings.mswEnabled=conf.getInt("mswEnabled",0);
    settings.allowEditDocking=conf.getInt("allowEditDocking",1);
#ifndef FLATPAK_WORKAROUNDS
    settings.sysFileDialog=conf.getInt("sysFileDialog",SYS_FILE_DIALOG_DEFAULT);
#endif
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
    decompileNoteKeys();
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
    settings.draggableDataView=conf.getInt("draggableDataView",1);

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
    settings.loadFallbackPat=conf.getInt("loadFallbackPat",1);

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

    settings.songNotesWrap=conf.getInt("songNotesWrap", 0);

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
    settings.swanCore=conf.getInt("swanCore",0);

    settings.dsidQuality=conf.getInt("dsidQuality",3);
    settings.gbQuality=conf.getInt("gbQuality",3);
    settings.pnQuality=conf.getInt("pnQuality",3);
    settings.saaQuality=conf.getInt("saaQuality",3);

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
    settings.swanCoreRender=conf.getInt("swanCoreRender",0);

    settings.dsidQualityRender=conf.getInt("dsidQualityRender",3);
    settings.gbQualityRender=conf.getInt("gbQualityRender",3);
    settings.pnQualityRender=conf.getInt("pnQualityRender",3);
    settings.saaQualityRender=conf.getInt("saaQualityRender",3);

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
  clampSetting(settings.swanCore,0,1);
  clampSetting(settings.dsidQuality,0,5);
  clampSetting(settings.gbQuality,0,5);
  clampSetting(settings.pnQuality,0,5);
  clampSetting(settings.saaQuality,0,5);
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
  clampSetting(settings.swanCoreRender,0,1);
  clampSetting(settings.dsidQualityRender,0,5);
  clampSetting(settings.gbQualityRender,0,5);
  clampSetting(settings.pnQualityRender,0,5);
  clampSetting(settings.saaQualityRender,0,5);
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
#ifndef FLATPAK_WORKAROUNDS
  clampSetting(settings.sysFileDialog,0,1);
#endif
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
  clampSetting(settings.loadFallbackPat,0,1);
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
  clampSetting(settings.mswEnabled,0,1);
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
  clampSetting(settings.dragMovesSelection,0,5);
  clampSetting(settings.draggableDataView,0,1);
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
  clampSetting(settings.songNotesWrap, 0, 1);
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
  clampSetting(settings.glSetBS,0,1);
  clampSetting(settings.glStencilSize,0,32);
  clampSetting(settings.glBufferSize,0,128);
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
    conf.set("glSetBS",settings.glSetBS);
    conf.set("glStencilSize",settings.glStencilSize);
    conf.set("glBufferSize",settings.glBufferSize);
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
    conf.set("mswEnabled",settings.mswEnabled);
    conf.set("allowEditDocking",settings.allowEditDocking);
#ifndef FLATPAK_WORKAROUNDS
    conf.set("sysFileDialog",settings.sysFileDialog);
#endif
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

    compileNoteKeys();
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
    conf.set("draggableDataView",settings.draggableDataView);

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
    conf.set("loadFallbackPat",settings.loadFallbackPat);

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

    conf.set("songNotesWrap",settings.songNotesWrap);

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
    conf.set("swanCore",settings.swanCore);

    conf.set("dsidQuality",settings.dsidQuality);
    conf.set("gbQuality",settings.gbQuality);
    conf.set("pnQuality",settings.pnQuality);
    conf.set("saaQuality",settings.saaQuality);

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
    conf.set("swanCoreRender",settings.swanCoreRender);

    conf.set("dsidQualityRender",settings.dsidQualityRender);
    conf.set("gbQualityRender",settings.gbQualityRender);
    conf.set("pnQualityRender",settings.pnQualityRender);
    conf.set("saaQualityRender",settings.saaQualityRender);

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
    settings.swanCore!=e->getConfInt("swanCore",0) ||
    settings.dsidQuality!=e->getConfInt("dsidQuality",3) ||
    settings.gbQuality!=e->getConfInt("gbQuality",3) ||
    settings.pnQuality!=e->getConfInt("pnQuality",3) ||
    settings.saaQuality!=e->getConfInt("saaQuality",3) ||
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

    // four fallback fonts
    if (settings.loadFallbackPat && (settings.loadJapanese ||
        settings.loadChinese ||
        settings.loadChineseTraditional ||
        settings.loadKorean ||
        localeRequiresJapanese ||
        localeRequiresChinese ||
        localeRequiresChineseTrad ||
        localeRequiresKorean)) {
      patFont=addFontZlib(font_plexMono_compressed_data,font_plexMono_compressed_size,MAX(1,e->getConfInt("patFontSize",18)*dpiScale),&fc1,fontRange);
      patFont=addFontZlib(font_plexSansJP_compressed_data,font_plexSansJP_compressed_size,MAX(1,e->getConfInt("patFontSize",18)*dpiScale),&fc1,fontRange);
      patFont=addFontZlib(font_plexSansKR_compressed_data,font_plexSansKR_compressed_size,MAX(1,e->getConfInt("patFontSize",18)*dpiScale),&fc1,fontRange);
      patFont=addFontZlib(font_unifont_compressed_data,font_unifont_compressed_size,MAX(1,e->getConfInt("patFontSize",18)*dpiScale),&fc1,fontRange);
    }

    // 0x39B = Λ
    // Հայերեն
    // 한국어
    // Русский
    // č
    // ń
    // ไทย
    static const ImWchar bigFontRange[]={0x20,0xFF,0x39b,0x39b,0x10d,0x10d,0x144,0x144,0x420,0x420,0x423,0x423,0x430,0x430,0x438,0x438,0x439,0x439,0x43a,0x43a,0x43d,0x43d,0x440,0x440,0x441,0x441,0x443,0x443,0x44c,0x44c,0x457,0x457,0x540,0x540,0x561,0x561,0x565,0x565,0x575,0x575,0x576,0x576,0x580,0x580,0xe17,0xe17,0xe22,0xe22,0xe44,0xe44,0x65e5,0x65e5,0x672c,0x672c,0x8a9e,0x8a9e,0xad6d,0xad6d,0xc5b4,0xc5b4,0xd55c,0xd55c,0};

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
#ifdef FLATPAK_WORKAROUNDS
    fileDialog=new FurnaceGUIFileDialog(false);
#else
    fileDialog=new FurnaceGUIFileDialog(settings.sysFileDialog);
#endif

    fileDialog->mobileUI=mobileUI;
  }
}
