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
#include "imgui_internal.h"
#include "fonts.h"
#include "util.h"
#include "../utfutils.h"
#include "guiConst.h"
#include "intConst.h"
#include "IconsFontAwesome4.h"
#include "furIcons.h"
#include "misc/cpp/imgui_stdlib.h"
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

#define SAMPLE_RATE_SELECTABLE(x) \
  if (ImGui::Selectable(#x,settings.audioRate==x)) { \
    settings.audioRate=x; \
    SETTINGS_CHANGED; \
  }

#define BUFFER_SIZE_SELECTABLE(x) \
  if (ImGui::Selectable(#x,settings.audioBufSize==x)) { \
    settings.audioBufSize=x; \
    SETTINGS_CHANGED; \
  }

#define SETTING(n,f) Setting(n,[this]f)

#define SETTING_COND(n,f,c) Setting(n,[this]f,[this]{return c;})

#define SETTING_SEPARATOR Setting(NULL,[]{ImGui::Separator();})

#define SETTING_NEWLINE Setting(NULL,[]{ImGui::NewLine();})

#define SETTINGS_CHANGED settingsChanged=true

#define MIDI_SPECIFIC_CONTROL(i) \
          SETTING(specificControls[i],{ \
            ImGui::PushID(i); \
            if (ImGui::Combo(specificControls[i],&midiMap.valueInputSpecificStyle[i],LocalizedComboGetter,valueSInputStyles,4)) SETTINGS_CHANGED; \
            ImGui::PopID(); \
          }), \
          SETTING_COND(_("Control##valueCCS"),{ \
            ImGui::PushID(i); \
            ImGui::Indent(); \
            if (ImGui::InputInt(_("Control##valueCCS"),&midiMap.valueInputSpecificSingle[i],1,16)) { \
              if (midiMap.valueInputSpecificSingle[i]<0) midiMap.valueInputSpecificSingle[i]=0; \
              if (midiMap.valueInputSpecificSingle[i]>127) midiMap.valueInputSpecificSingle[i]=127; \
              SETTINGS_CHANGED; \
            } \
            ImGui::Unindent(); \
            ImGui::PopID(); \
          },midiMap.valueInputSpecificStyle[i]==3), \
          SETTING_COND(_("Control##valueCCS"),{ \
            ImGui::PushID(i); \
            ImGui::Indent(); \
            if (ImGui::InputInt(_("MSB CC##valueCC1"),&midiMap.valueInputSpecificMSB[i],1,16)) { \
              if (midiMap.valueInputSpecificMSB[i]<0) midiMap.valueInputSpecificMSB[i]=0; \
              if (midiMap.valueInputSpecificMSB[i]>127) midiMap.valueInputSpecificMSB[i]=127; \
              SETTINGS_CHANGED; \
            } \
            if (ImGui::InputInt(_("LSB CC##valueCC2"),&midiMap.valueInputSpecificLSB[i],1,16)) { \
              if (midiMap.valueInputSpecificLSB[i]<0) midiMap.valueInputSpecificLSB[i]=0; \
              if (midiMap.valueInputSpecificLSB[i]>127) midiMap.valueInputSpecificLSB[i]=127; \
              SETTINGS_CHANGED; \
            } \
            ImGui::Unindent(); \
            ImGui::PopID(); \
          },(midiMap.valueInputSpecificStyle[i]>0) && (midiMap.valueInputSpecificStyle[i]!=3)) \

#define EMU_CORES(sysL,setC,setCR,arr) \
        SETTING(sysL,{ \
          if (ImGui::BeginTable("##" sysL "Cores", 3)) { \
            ImGui::TableSetupColumn("##sys",ImGuiTableColumnFlags_WidthStretch,1.0f); \
            ImGui::TableSetupColumn("##core",ImGuiTableColumnFlags_WidthStretch,2.5f); \
            ImGui::TableSetupColumn("##coreRend",ImGuiTableColumnFlags_WidthStretch,2.5f); \
            ImGui::TableNextRow(); \
            ImGui::TableNextColumn(); \
            ImGui::AlignTextToFramePadding(); \
            ImGui::ScrollText(ImGui::GetID(sysL),sysL,ImGui::GetCursorScreenPos()); \
            ImGui::TableNextColumn(); \
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x); \
            if (ImGui::Combo("##" sysL "C",&setC,arr,sizeof(arr)/sizeof(const char*))) SETTINGS_CHANGED; \
            ImGui::TableNextColumn(); \
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x); \
            if (ImGui::Combo("##" sysL "CR",&setCR,arr,sizeof(arr)/sizeof(const char*))) SETTINGS_CHANGED; \
            ImGui::EndTable(); \
          } \
        })

#define CORE_QUALITY(_name,_play,_render) \
        SETTING(_name,{ \
          if (ImGui::BeginTable("##" _name "Cores", 3)) { \
            ImGui::TableSetupColumn("##sys",ImGuiTableColumnFlags_WidthStretch,1.0f); \
            ImGui::TableSetupColumn("##core",ImGuiTableColumnFlags_WidthStretch,2.0f); \
            ImGui::TableSetupColumn("##coreRend",ImGuiTableColumnFlags_WidthStretch,2.0f); \
            ImGui::TableNextRow(); \
            ImGui::TableNextColumn(); \
            ImGui::AlignTextToFramePadding(); \
            ImGui::ScrollText(ImGui::GetID(_name),_name,ImGui::GetCursorScreenPos()); \
            ImGui::TableNextColumn(); \
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x); \
            if (ImGui::Combo("##" _name "Q",&settings._play,LocalizedComboGetter,coreQualities,6)) SETTINGS_CHANGED; \
            ImGui::TableNextColumn(); \
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x); \
            if (ImGui::Combo("##" _name "QR",&settings._render,LocalizedComboGetter,coreQualities,6)) SETTINGS_CHANGED; \
            ImGui::EndTable(); \
          } \
        })

#define KEYBIND(x) \
        SETTING((guiActions[x].friendlyName),{ \
          const char* friendlyName=guiActions[x].friendlyName; \
          char temp[2048]; \
          snprintf(temp, 2048, "##%sKey",friendlyName); \
          if (ImGui::BeginTable(temp, 2)) { \
            ImGui::TableSetupColumn("##label",ImGuiTableColumnFlags_WidthStretch,2.0f); \
            ImGui::TableSetupColumn("##keyInputs",ImGuiTableColumnFlags_WidthStretch,5.0f); \
            ImGui::TableNextRow(); \
            ImGui::TableNextColumn(); \
            ImGui::AlignTextToFramePadding(); \
            ImGui::ScrollText(x,friendlyName,ImGui::GetCursorScreenPos()); \
            ImGui::TableNextColumn(); \
            ImGui::PushID(x); \
            for (size_t i=0; i<actionKeys[x].size()+1; i++) { \
              ImGui::PushID(i); \
              if (i>0) ImGui::SameLine(); \
              bool isPending=bindSetPending && bindSetTarget==x && bindSetTargetIdx==(int)i; \
              if (i<actionKeys[x].size()) { \
                if (ImGui::Button(isPending?_N("Press key..."):getKeyName(actionKeys[x][i]).c_str())) { \
                  promptKey(x,i); \
                  SETTINGS_CHANGED; \
                } \
                bool rightClicked=ImGui::IsItemClicked(ImGuiMouseButton_Right); \
                if (!rightClicked) { \
                  ImGui::SameLine(0.0f, 1.0f); \
                } \
                if (rightClicked || ImGui::Button(ICON_FA_TIMES)) { \
                  actionKeys[x].erase(actionKeys[x].begin()+i); \
                  if (isPending) { \
                    bindSetActive=false; \
                    bindSetPending=false; \
                  } \
                  parseKeybinds(); \
                } \
              } else { \
                if (ImGui::Button(isPending?_N("Press key..."):ICON_FA_PLUS)) { \
                  promptKey(x,i); \
                  SETTINGS_CHANGED; \
                } \
              } \
              ImGui::PopID(); /*i*/ \
            } \
            ImGui::PopID(); /*action*/ \
            ImGui::EndTable(); \
          } \
        })

#define UI_COLOR_CONFIG(what,label) \
  SETTING(label,{ \
    ImGui::PushID(what); \
    if (ImGui::ColorEdit4(label,(float*)&uiColors[what])) { \
      applyUISettings(false); \
      SETTINGS_CHANGED; \
    } \
    ImGui::PopID(); \
  })

#define UI_COLOR_CONFIG_COND(what,label,cond) \
  SETTING_COND(label,{ \
    ImGui::PushID(what); \
    if (ImGui::ColorEdit4(label,(float*)&uiColors[what])) { \
      applyUISettings(false); \
      SETTINGS_CHANGED; \
    } \
    ImGui::PopID(); \
  },cond)

extern const char* locales[][3];
extern const char* fontBackends[];
extern const char* mainFonts[];
extern const char* headFonts[];
extern const char* patFonts[];
extern const char* audioBackends[];
extern const char* audioQualities[];
extern const char* arcadeCores[];
extern const char* ym2612Cores[];
extern const char* snCores[];
extern const char* nesCores[];
extern const char* c64Cores[];
extern const char* pokeyCores[];
extern const char* opnCores[];
extern const char* opl2Cores[];
extern const char* opl3Cores[];
extern const char* opl4Cores[];
extern const char* esfmCores[];
extern const char* opllCores[];
extern const char* ayCores[];
extern const char* coreQualities[];
extern const char* pcspkrOutMethods[];
extern const char* valueInputStyles[];
extern const char* valueSInputStyles[];
extern const char* messageTypes[];
extern const char* messageChannels[];
extern const char* specificControls[];