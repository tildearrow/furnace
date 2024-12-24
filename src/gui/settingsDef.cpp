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

// NEW NEW SETTINGS HERE
void FurnaceGUI::setupSettingsCategories() {
  settings.categories={
    SettingsCategory(_("General"),{
      SettingsCategory(_("Program"),{},{
#ifdef HAVE_LOCALE
        SETTING(_("Language"),{
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
                SETTINGS_CHANGED;
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
        }),
#endif
        Setting(_("Render backend"),[this]{
          String curRenderBackend=settings.renderBackend.empty()?GUI_BACKEND_DEFAULT_NAME:settings.renderBackend;
          if (ImGui::BeginCombo(_("Render backend"),curRenderBackend.c_str())) {
#ifdef HAVE_RENDER_SDL
            if (ImGui::Selectable("SDL Renderer",curRenderBackend=="SDL")) {
              settings.renderBackend="SDL";
              SETTINGS_CHANGED;
            }
#endif
#ifdef HAVE_RENDER_DX11
            if (ImGui::Selectable("DirectX 11",curRenderBackend=="DirectX 11")) {
              settings.renderBackend="DirectX 11";
              SETTINGS_CHANGED;
            }
#endif
#ifdef HAVE_RENDER_DX9
            if (ImGui::Selectable("DirectX 9",curRenderBackend=="DirectX 9")) {
              settings.renderBackend="DirectX 9";
              SETTINGS_CHANGED;
            }
#endif
#ifdef HAVE_RENDER_METAL
            if (ImGui::Selectable("Metal",curRenderBackend=="Metal")) {
              settings.renderBackend="Metal";
              SETTINGS_CHANGED;
            }
#endif
#ifdef HAVE_RENDER_GL
#ifdef USE_GLES
            if (ImGui::Selectable("OpenGL ES 2.0",curRenderBackend=="OpenGL ES 2.0")) {
              settings.renderBackend="OpenGL ES 2.0";
              SETTINGS_CHANGED;
            }
#else
            if (ImGui::Selectable("OpenGL 3.0",curRenderBackend=="OpenGL 3.0")) {
              settings.renderBackend="OpenGL 3.0";
              SETTINGS_CHANGED;
            }
            if (ImGui::Selectable("OpenGL 2.0",curRenderBackend=="OpenGL 2.0")) {
              settings.renderBackend="OpenGL 2.0";
              SETTINGS_CHANGED;
            }
#endif
#endif
#ifdef HAVE_RENDER_GL1
            if (ImGui::Selectable("OpenGL 1.1",curRenderBackend=="OpenGL 1.1")) {
              settings.renderBackend="OpenGL 1.1";
              SETTINGS_CHANGED;
            }
#endif
            if (ImGui::Selectable("Software",curRenderBackend=="Software")) {
              settings.renderBackend="Software";
              SETTINGS_CHANGED;
            }
            ImGui::EndCombo();
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("you may need to restart Furnace for this setting to take effect."));
          }
          ImGui::TextWrapped(_("current backend: %s\n%s\n%s\n%s"),rend->getBackendName(),rend->getVendorName(),rend->getDeviceName(),rend->getAPIVersion());
        }),
        SETTING(_("VSync"),{
          bool vsyncB=settings.vsync;
          if (ImGui::Checkbox(_("VSync"),&vsyncB)) {
            settings.vsync=vsyncB;
            SETTINGS_CHANGED;
            if (rend!=NULL) {
              rend->setSwapInterval(settings.vsync);
            }
          }
        }),
        SETTING(_("Frame rate limit"),{
          if (ImGui::SliderInt(_("Frame rate limit"),&settings.frameRateLimit,0,250,settings.frameRateLimit==0?_("Unlimited"):"%d")) {
            SETTINGS_CHANGED;
          }
          if (settings.frameRateLimit<0) settings.frameRateLimit=0;
          if (settings.frameRateLimit>1000) settings.frameRateLimit=1000;

          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("only applies when VSync is disabled."));
          }
        }),
        SETTING(_("Display render time"),{
          bool displayRenderTimeB=settings.displayRenderTime;
          if (ImGui::Checkbox(_("Display render time"),&displayRenderTimeB)) {
            settings.displayRenderTime=displayRenderTimeB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING_COND(_("Late render clear"),{
          bool renderClearPosB=settings.renderClearPos;
          if (ImGui::Checkbox(_("Late render clear"),&renderClearPosB)) {
            settings.renderClearPos=renderClearPosB;
            SETTINGS_CHANGED;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("calls rend->clear() after rend->present(). might reduce UI latency by one frame in some drivers."));
          }
        },settings.renderBackend!="Metal"),
        SETTING(_("Power-saving mode"),{
          bool powerSaveB=settings.powerSave;
          if (ImGui::Checkbox(_("Power-saving mode"),&powerSaveB)) {
            settings.powerSave=powerSaveB;
            SETTINGS_CHANGED;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("saves power by lowering the frame rate to 2fps when idle.\nmay cause issues under Mesa drivers!"));
          }
        }),
#ifndef IS_MOBILE
        SETTING(_("Disable threaded input (restart after changing!)"),{
          bool noThreadedInputB=settings.noThreadedInput;
          if (ImGui::Checkbox(_("Disable threaded input (restart after changing!)"),&noThreadedInputB)) {
            settings.noThreadedInput=noThreadedInputB;
            SETTINGS_CHANGED;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("threaded input processes key presses for note preview on a separate thread (on supported platforms), which reduces latency.\nhowever, crashes have been reported when threaded input is on. enable this option if that is the case."));
          }
        }),
#endif
        SETTING(_("Enable event delay"),{
          bool eventDelayB=settings.eventDelay;
          if (ImGui::Checkbox(_("Enable event delay"),&eventDelayB)) {
            settings.eventDelay=eventDelayB;
            SETTINGS_CHANGED;
            applyUISettings(false);
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("may cause issues with high-polling-rate mice when previewing notes."));
          }
        }),
        SETTING(_("Per-channel oscilloscope threads"),{
          pushWarningColor(settings.chanOscThreads>cpuCores,settings.chanOscThreads>(cpuCores*2));
          if (ImGui::InputInt(_("Per-channel oscilloscope threads"),&settings.chanOscThreads)) {
            if (settings.chanOscThreads<0) settings.chanOscThreads=0;
            if (settings.chanOscThreads>(cpuCores*3)) settings.chanOscThreads=cpuCores*3;
            if (settings.chanOscThreads>256) settings.chanOscThreads=256;
            SETTINGS_CHANGED;
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
        }),
        Setting(_("Oscilloscope rendering engine:"),[this]{
          ImGui::Text(_("Oscilloscope rendering engine:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("ImGui line plot"),settings.shaderOsc==0)) {
            settings.shaderOsc=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("render using Dear ImGui's built-in line drawing functions."));
          }
          if (ImGui::RadioButton(_("GLSL (if available)"),settings.shaderOsc==1)) {
            settings.shaderOsc=1;
            SETTINGS_CHANGED;
          }
          if (ImGui::IsItemHovered()) {
#ifdef USE_GLES
            ImGui::SetTooltip(_("render using shaders that run on the graphics card.\nonly available in OpenGL ES 2.0 render backend."));
#else
            ImGui::SetTooltip(_("render using shaders that run on the graphics card.\nonly available in OpenGL 3.0 render backend."));
#endif
          }
          ImGui::Unindent();
        }),
      }),
      SettingsCategory(_("File"),{},{
        SETTING(_("Use system file picker"),{
          bool sysFileDialogB=settings.sysFileDialog;
          if (ImGui::Checkbox(_("Use system file picker"),&sysFileDialogB)) {
            settings.sysFileDialog=sysFileDialogB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Number of recent files"),{
          if (ImGui::InputInt(_("Number of recent files"),&settings.maxRecentFile,1,5)) {
            if (settings.maxRecentFile<0) settings.maxRecentFile=0;
            if (settings.maxRecentFile>30) settings.maxRecentFile=30;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Compress when saving"),{
          bool compressB=settings.compress;
          if (ImGui::Checkbox(_("Compress when saving"),&compressB)) {
            settings.compress=compressB;
            SETTINGS_CHANGED;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("use zlib to compress saved songs."));
          }
        }),
        SETTING(_("Save unused patterns"),{
          bool saveUnusedPatternsB=settings.saveUnusedPatterns;
          if (ImGui::Checkbox(_("Save unused patterns"),&saveUnusedPatternsB)) {
            settings.saveUnusedPatterns=saveUnusedPatternsB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Use new pattern format when saving"),{
          bool newPatternFormatB=settings.newPatternFormat;
          if (ImGui::Checkbox(_("Use new pattern format when saving"),&newPatternFormatB)) {
            settings.newPatternFormat=newPatternFormatB;
            SETTINGS_CHANGED;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("use a packed format which saves space when saving songs.\ndisable if you need compatibility with older Furnace and/or tools\nwhich do not support this format."));
          }
        }),
        SETTING(_("Don't apply compatibility flags when loading .dmf"),{
          bool noDMFCompatB=settings.noDMFCompat;
          if (ImGui::Checkbox(_("Don't apply compatibility flags when loading .dmf"),&noDMFCompatB)) {
            settings.noDMFCompat=noDMFCompatB;
            SETTINGS_CHANGED;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("do not report any issues arising from the use of this option!"));
          }
        }),
        SETTING(_("Play after opening song:"),{
          ImGui::Text(_("Play after opening song:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("No##pol0"),settings.playOnLoad==0)) {
            settings.playOnLoad=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Only if already playing##pol1"),settings.playOnLoad==1)) {
            settings.playOnLoad=1;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Yes##pol0"),settings.playOnLoad==2)) {
            settings.playOnLoad=2;
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
        }),
        SETTING(_("Audio export loop/fade out time:"),{
          ImGui::Text(_("Audio export loop/fade out time:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("Set to these values on start-up:##fot0"),settings.persistFadeOut==0)) {
            settings.persistFadeOut=0;
            SETTINGS_CHANGED;
          }
          ImGui::BeginDisabled(settings.persistFadeOut);
          ImGui::Indent();
          if (ImGui::InputInt(_("Loops"),&settings.exportLoops,1,2)) {
            if (settings.exportLoops<0) settings.exportLoops=0;
            audioExportOptions.loops=settings.exportLoops;
            SETTINGS_CHANGED;
          }
          if (ImGui::InputDouble(_("Fade out (seconds)"),&settings.exportFadeOut,1.0,2.0,"%.1f")) {
            if (settings.exportFadeOut<0.0) settings.exportFadeOut=0.0;
            audioExportOptions.fadeOut=settings.exportFadeOut;
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
          ImGui::EndDisabled();
          if (ImGui::RadioButton(_("Remember last values##fot1"),settings.persistFadeOut==1)) {
            settings.persistFadeOut=1;
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
        }),
        SETTING(_("Store instrument name in .fui"),{
          bool writeInsNamesB=settings.writeInsNames;
          if (ImGui::Checkbox(_("Store instrument name in .fui"),&writeInsNamesB)) {
            settings.writeInsNames=writeInsNamesB;
            SETTINGS_CHANGED;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("when enabled, saving an instrument will store its name.\nthis may increase file size."));
          }
        }),
        SETTING(_("Load instrument name from .fui"),{
          bool readInsNamesB=settings.readInsNames;
          if (ImGui::Checkbox(_("Load instrument name from .fui"),&readInsNamesB)) {
            settings.readInsNames=readInsNamesB;
            SETTINGS_CHANGED;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("when enabled, loading an instrument will use the stored name (if present).\notherwise, it will use the file name."));
          }
        }),
        SETTING(_("Auto-fill file name when saving"),{
          bool autoFillSaveB=settings.autoFillSave;
          if (ImGui::Checkbox(_("Auto-fill file name when saving"),&autoFillSaveB)) {
            settings.autoFillSave=autoFillSaveB;
            SETTINGS_CHANGED;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("fill the file name field with an appropriate file name when saving or exporting."));
          }
        })
      }),
      SettingsCategory(_("New Song"),{},{
        SETTING(_("Initial system:"),{
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
            SETTINGS_CHANGED;
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
            SETTINGS_CHANGED;
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
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Name"),{
          ImGui::AlignTextToFramePadding();
          ImGui::Text(_("Name"));
          ImGui::SameLine();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::InputText("##InitSysName",&settings.initialSysName)) SETTINGS_CHANGED;
        }),
        SETTING(_("Initial system:"),{ // not the real setting name but gotta find it somehow
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

              if (sysID!=DIV_SYSTEM_NULL) {
                settings.initialSys.set(fmt::sprintf("id%d",i),(int)e->systemToFileFur(sysID));
                settings.initialSys.set(fmt::sprintf("flags%d",i),"");
                SETTINGS_CHANGED;
                ImGui::CloseCurrentPopup();
              }

              ImGui::EndCombo();
            }

            ImGui::SameLine();
            if (ImGui::Checkbox(_("Invert"),&doInvert)) {
              sysVol=-sysVol;
              settings.initialSys.set(fmt::sprintf("vol%d",i),sysVol);
              SETTINGS_CHANGED;
            }
            ImGui::SameLine();
            //ImGui::BeginDisabled(settings.initialSys.size()<=4);
            pushDestColor();
            if (ImGui::Button(ICON_FA_MINUS "##InitSysRemove")) {
              doRemove=i;
              SETTINGS_CHANGED;
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
              SETTINGS_CHANGED;
            } rightClickable
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x-ImGui::GetFrameHeightWithSpacing()*2.0-ImGui::GetStyle().ItemSpacing.x*2.0);
            if (CWSliderFloat(_("Panning"),&sysPan,-1.0f,1.0f)) {
              if (sysPan<-1.0f) sysPan=-1.0f;
              if (sysPan>1.0f) sysPan=1.0f;
              settings.initialSys.set(fmt::sprintf("pan%d",i),(float)sysPan);
              SETTINGS_CHANGED;
            } rightClickable
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x-ImGui::GetFrameHeightWithSpacing()*2.0-ImGui::GetStyle().ItemSpacing.x*2.0);
            if (CWSliderFloat(_("Front/Rear"),&sysPanFR,-1.0f,1.0f)) {
              if (sysPanFR<-1.0f) sysPanFR=-1.0f;
              if (sysPanFR>1.0f) sysPanFR=1.0f;
              settings.initialSys.set(fmt::sprintf("fr%d",i),(float)sysPanFR);
              SETTINGS_CHANGED;
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
              SETTINGS_CHANGED;
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
        }),
        SETTING(_("When creating new song:"),{
          ImGui::Text(_("When creating new song:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("Display system preset selector##NSB0"),settings.newSongBehavior==0)) {
            settings.newSongBehavior=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Start with initial system##NSB1"),settings.newSongBehavior==1)) {
            settings.newSongBehavior=1;
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
        }),
        SETTING(_("Default author name"),{
          ImGui::AlignTextToFramePadding();
          ImGui::Text(_("Default author name"));
          ImGui::SameLine();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::InputText("##DefAuthName", &settings.defaultAuthorName)) SETTINGS_CHANGED;
        })
      }),
      SettingsCategory(_("Start-up"),{},{
        SETTING(_("Play intro on start-up:"),{
          ImGui::Text(_("Play intro on start-up:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("No##pis0"),settings.alwaysPlayIntro==0)) {
            settings.alwaysPlayIntro=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Short##pis1"),settings.alwaysPlayIntro==1)) {
            settings.alwaysPlayIntro=1;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Full (short when loading song)##pis2"),settings.alwaysPlayIntro==2)) {
            settings.alwaysPlayIntro=2;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Full (always)##pis3"),settings.alwaysPlayIntro==3)) {
            settings.alwaysPlayIntro=3;
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
        }),
        SETTING(_("Disable fade-in during start-up"),{
          bool disableFadeInB=settings.disableFadeIn;
          if (ImGui::Checkbox(_("Disable fade-in during start-up"),&disableFadeInB)) {
            settings.disableFadeIn=disableFadeInB;
            SETTINGS_CHANGED;
          }
        })
      }),
      SettingsCategory(_("Behavior"),{},{
        SETTING(_("New instruments are blank"),{
          bool blankInsB=settings.blankIns;
          if (ImGui::Checkbox(_("New instruments are blank"),&blankInsB)) {
            settings.blankIns=blankInsB;
            SETTINGS_CHANGED;
          }
        })
      }),
      SettingsCategory(_("Configuration"),{},{
        SETTING(NULL,{
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
        })
      }),
      SettingsCategory(_("Import"),{},{
        SETTING(_("Use OPL3 instead of OPL2 for S3M import"),{
          bool s3mOPL3B=settings.s3mOPL3;
          if (ImGui::Checkbox(_("Use OPL3 instead of OPL2 for S3M import"),&s3mOPL3B)) {
            settings.s3mOPL3=s3mOPL3B;
            SETTINGS_CHANGED;
          }
        })
      }),
#ifdef IS_MOBILE
      SettingsCategory(_("Android"),{
        SettingsCategory(_("Vibrator"),{},{
          SETTING(_("Strength"),{
            if (ImGui::SliderFloat(_("Strength"),&settings.vibrationStrength,0.0f,1.0f)) {
              if (settings.vibrationStrength<0.0f) settings.vibrationStrength=0.0f;
              if (settings.vibrationStrength>1.0f) settings.vibrationStrength=1.0f;
              SETTINGS_CHANGED;
            }
          }),
          SETTING(_("Length"),{
            if (ImGui::SliderInt(_("Length"),&settings.vibrationLength,10,500,"%d ms")) {
              if (settings.vibrationLength<10) settings.vibrationLength=10;
              if (settings.vibrationLength>500) settings.vibrationLength=500;
              SETTINGS_CHANGED;
            }
          }),
        })
      },{
        SETTING(_("Enable background playback (restart!)"),{
          bool backgroundPlayB=settings.backgroundPlay;
          if (ImGui::Checkbox(_("Enable background playback (restart!)"),&backgroundPlayB)) {
            settings.backgroundPlay=backgroundPlayB;
            SETTINGS_CHANGED;
          }
        })
      }),
#endif
    },{}),
    SettingsCategory(_("Audio"),{
      SettingsCategory(_("Output"),{},{
#if defined(HAVE_JACK) || defined(HAVE_PA)
        Setting(_("Backend"),[this]{
          int prevAudioEngine=settings.audioEngine;
          if (ImGui::BeginCombo(_("Backend"),audioBackends[settings.audioEngine])) {
#ifdef HAVE_JACK
            if (ImGui::Selectable("JACK",settings.audioEngine==DIV_AUDIO_JACK)) {
              settings.audioEngine=DIV_AUDIO_JACK;
              SETTINGS_CHANGED;
            }
#endif
            if (ImGui::Selectable("SDL",settings.audioEngine==DIV_AUDIO_SDL)) {
              settings.audioEngine=DIV_AUDIO_SDL;
              SETTINGS_CHANGED;
            }
#ifdef HAVE_PA
            if (ImGui::Selectable("PortAudio",settings.audioEngine==DIV_AUDIO_PORTAUDIO)) {
              settings.audioEngine=DIV_AUDIO_PORTAUDIO;
              SETTINGS_CHANGED;
            }
#endif
            if (settings.audioEngine!=prevAudioEngine) {
              audioEngineChanged=true;
              settings.audioDevice="";
              settings.audioChans=2;
            }
            ImGui::EndCombo();
          }
        }),
#endif
        SETTING_COND(_("Driver"),{
          if (ImGui::BeginCombo(_("Driver"),settings.sdlAudioDriver.empty()?_("Automatic"):settings.sdlAudioDriver.c_str())) {
            if (ImGui::Selectable(_("Automatic"),settings.sdlAudioDriver.empty())) {
              settings.sdlAudioDriver="";
              SETTINGS_CHANGED;
            }
            for (String& i: availAudioDrivers) {
              if (ImGui::Selectable(i.c_str(),i==settings.sdlAudioDriver)) {
                settings.sdlAudioDriver=i;
                SETTINGS_CHANGED;
              }
            }
            ImGui::EndCombo();
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("you may need to restart Furnace for this setting to take effect."));
          }
        },settings.audioEngine==DIV_AUDIO_SDL),
        SETTING(_("Device"),{
          if (audioEngineChanged) {
            // ImGui::BeginDisabled();
            if (ImGui::BeginCombo(_("Device"),_("<click on OK or Apply first>"))) {
              ImGui::Text(_("ALERT - TRESPASSER DETECTED"));
              if (ImGui::IsItemHovered()) {
                showError(_("you have been arrested for trying to engage with a disabled combo box."));
                ImGui::CloseCurrentPopup();
              }
              ImGui::EndCombo();
            }
            // ImGui::EndDisabled();
          } else {
            String audioDevName=settings.audioDevice.empty()?_("<System default>"):settings.audioDevice;
            if (ImGui::BeginCombo(_("Device"),audioDevName.c_str())) {
              if (ImGui::Selectable(_("<System default>"),settings.audioDevice.empty())) {
                settings.audioDevice="";
                SETTINGS_CHANGED;
              }
              for (String& i: e->getAudioDevices()) {
                if (ImGui::Selectable(i.c_str(),i==settings.audioDevice)) {
                  settings.audioDevice=i;
                  SETTINGS_CHANGED;
                }
              }
              ImGui::EndCombo();
            }
          }
        }),
        SETTING(_("Sample rate"),{
          String sr=fmt::sprintf("%d",settings.audioRate);
          if (ImGui::BeginCombo(_("Sample rate"),sr.c_str())) {
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
        }),
        SETTING(_("Outputs"),{
          if (ImGui::InputInt(_("Outputs"),&settings.audioChans,1,2)) {
            if (settings.audioChans<1) settings.audioChans=1;
            if (settings.audioChans>16) settings.audioChans=16;
            SETTINGS_CHANGED;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("common values:\n- 1 for mono\n- 2 for stereo"));
          }
        }),
        SETTING(_("Buffer size"),{
          String bs=fmt::sprintf(_("%d (latency: ~%.1fms)"),settings.audioBufSize,2000.0*(double)settings.audioBufSize/(double)MAX(1,settings.audioRate));
          if (ImGui::BeginCombo(_("Buffer size"),bs.c_str())) {
            BUFFER_SIZE_SELECTABLE(64);
            BUFFER_SIZE_SELECTABLE(128);
            BUFFER_SIZE_SELECTABLE(256);
            BUFFER_SIZE_SELECTABLE(512);
            BUFFER_SIZE_SELECTABLE(1024);
            BUFFER_SIZE_SELECTABLE(2048);
            ImGui::EndCombo();
          }
        }),
        SETTING(_("Multi-threaded (EXPERIMENTAL)"),{
          bool renderPoolThreadsB=(settings.renderPoolThreads>0);
          if (ImGui::Checkbox(_("Multi-threaded (EXPERIMENTAL)"),&renderPoolThreadsB)) {
            if (renderPoolThreadsB) {
              settings.renderPoolThreads=2;
            } else {
              settings.renderPoolThreads=0;
            }
            SETTINGS_CHANGED;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("runs chip emulation on separate threads.\nmay increase performance when using heavy emulation cores.\n\nwarnings:\n- experimental!\n- only useful on multi-chip songs."));
          }
        }),
        SETTING_COND(_("Number of threads"),{
          pushWarningColor(settings.renderPoolThreads>cpuCores,settings.renderPoolThreads>cpuCores);
          if (ImGui::InputInt(_("Number of threads"),&settings.renderPoolThreads)) {
            if (settings.renderPoolThreads<2) settings.renderPoolThreads=2;
            if (settings.renderPoolThreads>32) settings.renderPoolThreads=32;
            SETTINGS_CHANGED;
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
        },settings.renderPoolThreads>0),
        SETTING(_("Low-latency mode"),{
          bool lowLatencyB=settings.lowLatency;
          if (ImGui::Checkbox(_("Low-latency mode"),&lowLatencyB)) {
            settings.lowLatency=lowLatencyB;
            SETTINGS_CHANGED;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("reduces latency by running the engine faster than the tick rate.\nuseful for live playback/jam mode.\n\nwarning: only enable if your buffer size is small (10ms or less)."));
          }
        }),
        SETTING(_("Force mono audio"),{
          bool forceMonoB=settings.forceMono;
          if (ImGui::Checkbox(_("Force mono audio"),&forceMonoB)) {
            settings.forceMono=forceMonoB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING_COND(_("Exclusive mode"),{
          bool wasapiExB=settings.wasapiEx;
          if (ImGui::Checkbox(_("Exclusive mode"),&wasapiExB)) {
            settings.wasapiEx=wasapiExB;
            SETTINGS_CHANGED;
          }
        },settings.audioEngine==DIV_AUDIO_PORTAUDIO && settings.audioDevice.find("[Windows WASAPI] ")==0),
        Setting(NULL,[this]{
          TAAudioDesc& audioWant=e->getAudioDescWant();
          TAAudioDesc& audioGot=e->getAudioDescGot();
#ifdef HAVE_LOCALE
          ImGui::Text(ngettext("want: %d samples @ %.0fHz (%d channel)","want: %d samples @ %.0fHz (%d channels)",audioWant.outChans),audioWant.bufsize,audioWant.rate,audioWant.outChans);
          ImGui::Text(ngettext("got: %d samples @ %.0fHz (%d channel)","got: %d samples @ %.0fHz (%d channels)",audioGot.outChans),audioGot.bufsize,audioGot.rate,audioGot.outChans);
#else
          ImGui::Text(_GN("want: %d samples @ %.0fHz (%d channel)","want: %d samples @ %.0fHz (%d channels)",audioWant.outChans),audioWant.bufsize,audioWant.rate,audioWant.outChans);
          ImGui::Text(_GN("got: %d samples @ %.0fHz (%d channel)","got: %d samples @ %.0fHz (%d channels)",audioGot.outChans),audioGot.bufsize,audioGot.rate,audioGot.outChans);
#endif
        }),
      }),
      SettingsCategory(_("Mixing"),{},{
        SETTING(_("Quality"),{
          if (ImGui::Combo(_("Quality"),&settings.audioQuality,LocalizedComboGetter,audioQualities,2)) SETTINGS_CHANGED;
        }),
        SETTING(_("Software clipping"),{
          bool clampSamplesB=settings.clampSamples;
          if (ImGui::Checkbox(_("Software clipping"),&clampSamplesB)) {
            settings.clampSamples=clampSamplesB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("DC offset correction"),{
          bool audioHiPassB=settings.audioHiPass;
          if (ImGui::Checkbox(_("DC offset correction"),&audioHiPassB)) {
            settings.audioHiPass=audioHiPassB;
            SETTINGS_CHANGED;
          }
        })
      }),
      SettingsCategory(_("Metronome"),{},{
        SETTING(_("Volume"),{
          if (ImGui::SliderInt(_("Volume##metr"),&settings.metroVol,0,200,"%d%%")) {
            if (settings.metroVol<0) settings.metroVol=0;
            if (settings.metroVol>200) settings.metroVol=200;
            e->setMetronomeVol(((float)settings.metroVol)/100.0f);
            SETTINGS_CHANGED;
          }
        })
      }),
      SettingsCategory(_("Sample preview"),{},{
        SETTING(_("Volume"),{
          if (ImGui::SliderInt(_("Volume##smpr"),&settings.sampleVol,0,100,"%d%%")) {
            if (settings.sampleVol<0) settings.sampleVol=0;
            if (settings.sampleVol>100) settings.sampleVol=100;
            e->setSamplePreviewVol(((float)settings.sampleVol)/100.0f);
            SETTINGS_CHANGED;
          }
        })
      })
    },{}),
    SettingsCategory(_("MIDI"),{
      SettingsCategory(_("MIDI input"),{
        SettingsCategory(_("Per-column control change"),{},{
          MIDI_SPECIFIC_CONTROL(0),
          MIDI_SPECIFIC_CONTROL(1),
          MIDI_SPECIFIC_CONTROL(2),
          MIDI_SPECIFIC_CONTROL(3),
          MIDI_SPECIFIC_CONTROL(4),
          MIDI_SPECIFIC_CONTROL(5),
          MIDI_SPECIFIC_CONTROL(6),
          MIDI_SPECIFIC_CONTROL(7),
          MIDI_SPECIFIC_CONTROL(8),
          MIDI_SPECIFIC_CONTROL(9),
          MIDI_SPECIFIC_CONTROL(10),
          MIDI_SPECIFIC_CONTROL(11),
          MIDI_SPECIFIC_CONTROL(12),
          MIDI_SPECIFIC_CONTROL(13),
          MIDI_SPECIFIC_CONTROL(14),
          MIDI_SPECIFIC_CONTROL(15),
          MIDI_SPECIFIC_CONTROL(16),
          MIDI_SPECIFIC_CONTROL(17),
        })
      },{
        SETTING(_("MIDI input"),{
          String midiInName=settings.midiInDevice.empty()?_("<disabled>"):settings.midiInDevice;
          bool hasToReloadMidi=false;
          if (ImGui::BeginCombo("##MidiInDevice",midiInName.c_str())) {
            if (ImGui::Selectable(_("<disabled>"),settings.midiInDevice.empty())) {
              settings.midiInDevice="";
              hasToReloadMidi=true;
              SETTINGS_CHANGED;
            }
            for (String& i: e->getMidiIns()) {
              if (ImGui::Selectable(i.c_str(),i==settings.midiInDevice)) {
                settings.midiInDevice=i;
                hasToReloadMidi=true;
                SETTINGS_CHANGED;
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
        }),
        SETTING(_("Note input"),{
          if (ImGui::Checkbox(_("Note input"),&midiMap.noteInput)) SETTINGS_CHANGED;
        }),
        SETTING(_("Velocity input"),{
          if (ImGui::Checkbox(_("Velocity input"),&midiMap.volInput)) SETTINGS_CHANGED;
        }),
        // TODO
        //ImGui::Checkbox(_("Use raw velocity value (don't map from linear to log)"),&midiMap.rawVolume);
        //ImGui::Checkbox(_("Polyphonic/chord input"),&midiMap.polyInput);
        // then convert to new new settings
        SETTING(_("Map MIDI channels to direct channels"),{
          if (ImGui::Checkbox(_("Map MIDI channels to direct channels"),&midiMap.directChannel)) {
            e->setMidiDirect(midiMap.directChannel);
            e->setMidiDirectProgram(midiMap.directChannel && midiMap.directProgram);
            SETTINGS_CHANGED;
          }
        }),
        SETTING_COND(_("Program change pass-through"),{
          if (ImGui::Checkbox(_("Program change pass-through"),&midiMap.directProgram)) {
            e->setMidiDirectProgram(midiMap.directChannel && midiMap.directProgram);
            SETTINGS_CHANGED;
          }
        },midiMap.directChannel),
        SETTING(_("Map Yamaha FM voice data to instruments"),{
          if (ImGui::Checkbox(_("Map Yamaha FM voice data to instruments"),&midiMap.yamahaFMResponse)) SETTINGS_CHANGED;
        }),
        SETTING_COND(_("Program change is instrument selection"),{
          if (ImGui::Checkbox(_("Program change is instrument selection"),&midiMap.programChange)) SETTINGS_CHANGED;
        },!(midiMap.directChannel && midiMap.directProgram)),
        //ImGui::Checkbox(_("Listen to MIDI clock"),&midiMap.midiClock);
        //ImGui::Checkbox(_("Listen to MIDI time code"),&midiMap.midiTimeCode);
        SETTING(_("Value input style"),{
          if (ImGui::Combo(_("Value input style"),&midiMap.valueInputStyle,LocalizedComboGetter,valueInputStyles,7)) SETTINGS_CHANGED;
        }),
        SETTING_COND(_("Control##valueCCS"),{
          if (ImGui::InputInt(_("Control##valueCCS"),&midiMap.valueInputControlSingle,1,16)) {
            if (midiMap.valueInputControlSingle<0) midiMap.valueInputControlSingle=0;
            if (midiMap.valueInputControlSingle>127) midiMap.valueInputControlSingle=127;
            SETTINGS_CHANGED;
          }
        },midiMap.valueInputStyle==6),
        SETTING_COND(_("CC of upper nibble##valueCC1"),{
          if (ImGui::InputInt(_("CC of upper nibble##valueCC1"),&midiMap.valueInputControlMSB,1,16)) {
            if (midiMap.valueInputControlMSB<0) midiMap.valueInputControlMSB=0;
            if (midiMap.valueInputControlMSB>127) midiMap.valueInputControlMSB=127;
            SETTINGS_CHANGED;
          }
        },midiMap.valueInputStyle==4),
        SETTING_COND(_("MSB CC##valueCC1"),{
          if (ImGui::InputInt(_("MSB CC##valueCC1"),&midiMap.valueInputControlMSB,1,16)) {
            if (midiMap.valueInputControlMSB<0) midiMap.valueInputControlMSB=0;
            if (midiMap.valueInputControlMSB>127) midiMap.valueInputControlMSB=127;
            SETTINGS_CHANGED;
          }
        },midiMap.valueInputStyle==5),
        SETTING_COND(_("CC of lower nibble##valueCC2"),{
          if (ImGui::InputInt(_("CC of lower nibble##valueCC2"),&midiMap.valueInputControlLSB,1,16)) {
            if (midiMap.valueInputControlLSB<0) midiMap.valueInputControlLSB=0;
            if (midiMap.valueInputControlLSB>127) midiMap.valueInputControlLSB=127;
            SETTINGS_CHANGED;
          }
        },midiMap.valueInputStyle==4),
        SETTING_COND(_("LSB CC##valueCC2"),{
          if (ImGui::InputInt(_("LSB CC##valueCC2"),&midiMap.valueInputControlLSB,1,16)) {
            if (midiMap.valueInputControlLSB<0) midiMap.valueInputControlLSB=0;
            if (midiMap.valueInputControlLSB>127) midiMap.valueInputControlLSB=127;
            SETTINGS_CHANGED;
          }
        },midiMap.valueInputStyle==5),
        SETTING_SEPARATOR,
        SETTING(_("Volume curve"),{
          if (ImGui::SliderFloat(_("Volume curve"),&midiMap.volExp,0.01,8.0,"%.2f")) {
            if (midiMap.volExp<0.01) midiMap.volExp=0.01;
            if (midiMap.volExp>8.0) midiMap.volExp=8.0;
            e->setMidiVolExp(midiMap.volExp);
            SETTINGS_CHANGED;
          } rightClickable
          float curve[128];
          for (int i=0; i<128; i++) {
            curve[i]=(int)(pow((double)i/127.0,midiMap.volExp)*127.0);
          }
          ImGui::PlotLines("##VolCurveDisplay",curve,128,0,_("Volume curve"),0.0,127.0,ImVec2(200.0f*dpiScale,200.0f*dpiScale));
        }),
        SETTING(_("Actions:"),{
          ImGui::AlignTextToFramePadding();
          ImGui::Text(_("Actions:"));
          ImGui::SameLine();
          if (ImGui::Button(ICON_FA_PLUS "##AddAction")) {
            midiMap.binds.push_back(MIDIBind());
            SETTINGS_CHANGED;
          }
          ImGui::SameLine();
          if (ImGui::Button(ICON_FA_EXTERNAL_LINK "##AddLearnAction")) {
            midiMap.binds.push_back(MIDIBind());
            learning=midiMap.binds.size()-1;
            SETTINGS_CHANGED;
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
                    SETTINGS_CHANGED;
                  }
                }
                ImGui::EndCombo();
              }

              ImGui::TableNextColumn();
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              if (ImGui::BeginCombo("##BChannel",messageChannels[bind.channel])) {
                if (ImGui::Selectable(messageChannels[16],bind.channel==16)) {
                  bind.channel=16;
                  SETTINGS_CHANGED;
                }
                for (int j=0; j<16; j++) {
                  if (ImGui::Selectable(messageChannels[j],bind.channel==j)) {
                    bind.channel=j;
                    SETTINGS_CHANGED;
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
                  SETTINGS_CHANGED;
                }
                for (int j=0; j<128; j++) {
                  const char* nName="???";
                  if ((j+60)>0 && (j+60)<180) {
                    nName=noteNames[j+60];
                  }
                  snprintf(bindID,1024,"%d (0x%.2X, %s)##BV1_%d",j,j,nName,j);
                  if (ImGui::Selectable(bindID,bind.data1==j)) {
                    bind.data1=j;
                    SETTINGS_CHANGED;
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
                  SETTINGS_CHANGED;
                }
                for (int j=0; j<128; j++) {
                  snprintf(bindID,1024,"%d (0x%.2X)##BV2_%d",j,j,j);
                  if (ImGui::Selectable(bindID,bind.data2==j)) {
                    bind.data2=j;
                    SETTINGS_CHANGED;
                  }
                }
                ImGui::EndCombo();
              }

              ImGui::TableNextColumn();
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              if (ImGui::BeginCombo("##BAction",(bind.action==0)?_("--none--"):guiActions[bind.action].friendlyName)) {
                if (ImGui::Selectable(_("--none--"),bind.action==0)) {
                  bind.action=0;
                  SETTINGS_CHANGED;
                }
                for (int j=0; j<GUI_ACTION_MAX; j++) {
                  if (strcmp(guiActions[j].friendlyName,"")==0) continue;
                  if (strstr(guiActions[j].friendlyName,"---")==guiActions[j].friendlyName) {
                    ImGui::TextUnformatted(guiActions[j].friendlyName);
                  } else {
                    snprintf(bindID,1024,"%s##BA_%d",_(guiActions[j].friendlyName),j);
                    if (ImGui::Selectable(bindID,bind.action==j)) {
                      bind.action=j;
                      SETTINGS_CHANGED;
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
                SETTINGS_CHANGED;
              }
              popToggleColors();

              ImGui::TableNextColumn();
              if (ImGui::Button(ICON_FA_TIMES "##BRemove")) {
                midiMap.binds.erase(midiMap.binds.begin()+i);
                if (learning==(int)i) learning=-1;
                i--;
                SETTINGS_CHANGED;
              }

              ImGui::PopID();
            }
            ImGui::EndTable();
          }
        })
      }),
      SettingsCategory(_("MIDI output"),{},{
        SETTING(_("MIDI output"),{
          String midiOutName=settings.midiOutDevice.empty()?_("<disabled>"):settings.midiOutDevice;
          if (ImGui::BeginCombo(_("MIDI output"),midiOutName.c_str())) {
            if (ImGui::Selectable(_("<disabled>"),settings.midiOutDevice.empty())) {
              settings.midiOutDevice="";
              SETTINGS_CHANGED;
            }
            for (String& i: e->getMidiIns()) {
              if (ImGui::Selectable(i.c_str(),i==settings.midiOutDevice)) {
                settings.midiOutDevice=i;
                SETTINGS_CHANGED;
              }
            }
            ImGui::EndCombo();
          }
        }),
        SETTING(_("Output mode:"),{
          ImGui::Text(_("Output mode:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("Off (use for TX81Z)"),settings.midiOutMode==0)) {
            settings.midiOutMode=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Melodic"),settings.midiOutMode==1)) {
            settings.midiOutMode=1;
            SETTINGS_CHANGED;
          }
          /*
          if (ImGui::RadioButton(_("Light Show (use for Launchpad)"),settings.midiOutMode==2)) {
            settings.midiOutMode=2;
            SETTINGS_CHANGED;
          }*/
          ImGui::Unindent();
        }),
        SETTING(_("Send Program Change"),{
          bool midiOutProgramChangeB=settings.midiOutProgramChange;
          if (ImGui::Checkbox(_("Send Program Change"),&midiOutProgramChangeB)) {
            settings.midiOutProgramChange=midiOutProgramChangeB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Send MIDI clock"),{
          bool midiOutClockB=settings.midiOutClock;
          if (ImGui::Checkbox(_("Send MIDI clock"),&midiOutClockB)) {
            settings.midiOutClock=midiOutClockB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Send MIDI timecode"),{
          bool midiOutTimeB=settings.midiOutTime;
          if (ImGui::Checkbox(_("Send MIDI timecode"),&midiOutTimeB)) {
            settings.midiOutTime=midiOutTimeB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING_COND(_("Timecode frame rate:"),{
          ImGui::Text(_("Timecode frame rate:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("Closest to Tick Rate"),settings.midiOutTimeRate==0)) {
            settings.midiOutTimeRate=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Film (24fps)"),settings.midiOutTimeRate==1)) {
            settings.midiOutTimeRate=1;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("PAL (25fps)"),settings.midiOutTimeRate==2)) {
            settings.midiOutTimeRate=2;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("NTSC drop (29.97fps)"),settings.midiOutTimeRate==3)) {
            settings.midiOutTimeRate=3;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("NTSC non-drop (30fps)"),settings.midiOutTimeRate==4)) {
            settings.midiOutTimeRate=4;
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
        },settings.midiOutTime)
      })
    },{}),
    SettingsCategory(_("Emulation"),{
      SettingsCategory(_("Cores"),{},{
        EMU_CORES("YM2151", settings.arcadeCore, settings.arcadeCoreRender, arcadeCores),
        EMU_CORES("YM2612", settings.ym2612Core, settings.ym2612CoreRender, ym2612Cores),
        EMU_CORES("SN76489", settings.snCore, settings.snCoreRender, snCores),
        EMU_CORES("NES", settings.nesCore, settings.nesCoreRender, nesCores),
        EMU_CORES("FDS", settings.fdsCore, settings.fdsCoreRender, nesCores),
        EMU_CORES("SID", settings.c64Core, settings.c64CoreRender, c64Cores),
        EMU_CORES("POKEY", settings.pokeyCore, settings.pokeyCoreRender, pokeyCores),
        EMU_CORES("OPN", settings.opn1Core, settings.opn1CoreRender, opnCores),
        EMU_CORES("OPNA", settings.opnaCore, settings.opnaCoreRender, opnCores),
        EMU_CORES("OPNB", settings.opnbCore, settings.opnbCoreRender, opnCores),
        EMU_CORES("OPL/OPL2/Y8950", settings.opl2Core, settings.opl2Core, opl2Cores),
        EMU_CORES("OPL3", settings.opl3Core, settings.opl3CoreRender, opl3Cores),
        EMU_CORES("OPL4", settings.opl4Core, settings.opl4CoreRender, opl4Cores),
        EMU_CORES("ESFM", settings.esfmCore, settings.esfmCoreRender, esfmCores),
        EMU_CORES("OPLL", settings.opllCore, settings.opllCoreRender, opllCores),
        EMU_CORES("AY-3-8910/SSG", settings.ayCore, settings.ayCoreRender, ayCores),
      }),
      SettingsCategory(_("Quality"),{},{
        CORE_QUALITY("Bubble System WSG",bubsysQuality,bubsysQualityRender),
        CORE_QUALITY("Game Boy",gbQuality,gbQualityRender),
        CORE_QUALITY("Nintendo DS",ndsQuality,ndsQualityRender),
        CORE_QUALITY("PC Engine",pceQuality,pceQualityRender),
        CORE_QUALITY("PowerNoise",pnQuality,pnQualityRender),
        CORE_QUALITY("SAA1099",saaQuality,saaQualityRender),
        CORE_QUALITY("SCC",sccQuality,sccQualityRender),
        CORE_QUALITY("SID (dSID)",dsidQuality,dsidQualityRender),
        CORE_QUALITY("SM8521",smQuality,smQualityRender),
        CORE_QUALITY("Virtual Boy",vbQuality,vbQualityRender),
        CORE_QUALITY("WonderSwan",swanQuality,swanQualityRender),
      }),
      SettingsCategory(_("Other"),{},{
        SETTING(_("PC Speaker strategy"),{
          if (ImGui::Combo(_("PC Speaker strategy"),&settings.pcSpeakerOutMethod,LocalizedComboGetter,pcspkrOutMethods,5)) SETTINGS_CHANGED;
        }),
        Setting(NULL,[]{ // subsection?
          ImGui::Separator();
          ImGui::Text(_("Sample ROMs:"));
        }),
        SETTING(_("OPL4 YRW801 path"),{
          ImGui::AlignTextToFramePadding();
          ImGui::Text(_("OPL4 YRW801 path"));
          ImGui::SameLine();
          ImGui::InputText("##YRW801Path",&settings.yrw801Path);
          ImGui::SameLine();
          if (ImGui::Button(ICON_FA_FOLDER "##YRW801Load")) {
            openFileDialog(GUI_FILE_YRW801_ROM_OPEN);
          }
        })
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
      })
    },{}),
    SettingsCategory(_("Keyboard"),{
      SettingsCategory(_("Global hotkeys"),{},{
        KEYBIND(GUI_ACTION_NEW),
        KEYBIND(GUI_ACTION_CLEAR),
        KEYBIND(GUI_ACTION_OPEN),
        KEYBIND(GUI_ACTION_OPEN_BACKUP),
        KEYBIND(GUI_ACTION_SAVE),
        KEYBIND(GUI_ACTION_SAVE_AS),
        KEYBIND(GUI_ACTION_EXPORT),
        KEYBIND(GUI_ACTION_UNDO),
        KEYBIND(GUI_ACTION_REDO),
        KEYBIND(GUI_ACTION_PLAY_TOGGLE),
        KEYBIND(GUI_ACTION_PLAY),
        KEYBIND(GUI_ACTION_STOP),
        KEYBIND(GUI_ACTION_PLAY_START),
        KEYBIND(GUI_ACTION_PLAY_REPEAT),
        KEYBIND(GUI_ACTION_PLAY_CURSOR),
        KEYBIND(GUI_ACTION_STEP_ONE),
        KEYBIND(GUI_ACTION_OCTAVE_UP),
        KEYBIND(GUI_ACTION_OCTAVE_DOWN),
        KEYBIND(GUI_ACTION_INS_UP),
        KEYBIND(GUI_ACTION_INS_DOWN),
        KEYBIND(GUI_ACTION_STEP_UP),
        KEYBIND(GUI_ACTION_STEP_DOWN),
        KEYBIND(GUI_ACTION_TOGGLE_EDIT),
        KEYBIND(GUI_ACTION_METRONOME),
        KEYBIND(GUI_ACTION_REPEAT_PATTERN),
        KEYBIND(GUI_ACTION_FOLLOW_ORDERS),
        KEYBIND(GUI_ACTION_FOLLOW_PATTERN),
        KEYBIND(GUI_ACTION_FULLSCREEN),
        KEYBIND(GUI_ACTION_TX81Z_REQUEST),
        KEYBIND(GUI_ACTION_PANIC),
      }),
      SettingsCategory(_("Window activation"),{},{
        KEYBIND(GUI_ACTION_WINDOW_FIND),
        KEYBIND(GUI_ACTION_WINDOW_SETTINGS),
        KEYBIND(GUI_ACTION_WINDOW_SONG_INFO),
        KEYBIND(GUI_ACTION_WINDOW_SUBSONGS),
        KEYBIND(GUI_ACTION_WINDOW_SPEED),
        KEYBIND(GUI_ACTION_WINDOW_INS_LIST),
        KEYBIND(GUI_ACTION_WINDOW_WAVE_LIST),
        KEYBIND(GUI_ACTION_WINDOW_SAMPLE_LIST),
        KEYBIND(GUI_ACTION_WINDOW_ORDERS),
        KEYBIND(GUI_ACTION_WINDOW_PATTERN),
        KEYBIND(GUI_ACTION_WINDOW_MIXER),
        KEYBIND(GUI_ACTION_WINDOW_GROOVES),
        KEYBIND(GUI_ACTION_WINDOW_CHANNELS),
        KEYBIND(GUI_ACTION_WINDOW_PAT_MANAGER),
        KEYBIND(GUI_ACTION_WINDOW_SYS_MANAGER),
        KEYBIND(GUI_ACTION_WINDOW_COMPAT_FLAGS),
        KEYBIND(GUI_ACTION_WINDOW_NOTES),
        KEYBIND(GUI_ACTION_WINDOW_INS_EDIT),
        KEYBIND(GUI_ACTION_WINDOW_WAVE_EDIT),
        KEYBIND(GUI_ACTION_WINDOW_SAMPLE_EDIT),
        KEYBIND(GUI_ACTION_WINDOW_EDIT_CONTROLS),
        KEYBIND(GUI_ACTION_WINDOW_PIANO),
        KEYBIND(GUI_ACTION_WINDOW_OSCILLOSCOPE),
        KEYBIND(GUI_ACTION_WINDOW_CHAN_OSC),
        KEYBIND(GUI_ACTION_WINDOW_XY_OSC),
        KEYBIND(GUI_ACTION_WINDOW_VOL_METER),
        KEYBIND(GUI_ACTION_WINDOW_CLOCK),
        KEYBIND(GUI_ACTION_WINDOW_REGISTER_VIEW),
        KEYBIND(GUI_ACTION_WINDOW_LOG),
        KEYBIND(GUI_ACTION_WINDOW_STATS),
        KEYBIND(GUI_ACTION_WINDOW_MEMORY),
        KEYBIND(GUI_ACTION_WINDOW_EFFECT_LIST),
        KEYBIND(GUI_ACTION_WINDOW_DEBUG),
        KEYBIND(GUI_ACTION_WINDOW_CS_PLAYER),
        KEYBIND(GUI_ACTION_WINDOW_ABOUT),
        KEYBIND(GUI_ACTION_COLLAPSE_WINDOW),
        KEYBIND(GUI_ACTION_CLOSE_WINDOW),

        KEYBIND(GUI_ACTION_COMMAND_PALETTE),
        KEYBIND(GUI_ACTION_CMDPAL_RECENT),
        KEYBIND(GUI_ACTION_CMDPAL_INSTRUMENTS),
        KEYBIND(GUI_ACTION_CMDPAL_SAMPLES),
      }),
      SettingsCategory(_("Note input"),{},{
        SETTING(_("Note input"),{
          std::vector<MappedInput> sorted; /* not a fan of the sorting. modifying the note value will make it jump */
          if (ImGui::BeginTable("keysNoteInput",4)) {
            for (std::map<int,int>::value_type& i: noteKeys) {
              std::vector<MappedInput>::iterator j;
              for (j=sorted.begin(); j!=sorted.end(); j++) {
                if (j->val>i.second) {
                  break;
                }
              }
              sorted.insert(j,MappedInput(i.first,i.second));
            }
  
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
  
            for (MappedInput& i: sorted) {
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::Text("%s",SDL_GetScancodeName((SDL_Scancode)i.scan));
              ImGui::TableNextColumn();
              if (i.val==102) {
                snprintf(id,4095,_("Macro release##SNType_%d"),i.scan);
                if (ImGui::Button(id)) {
                  noteKeys[i.scan]=0;
                }
              } else if (i.val==101) {
                snprintf(id,4095,_("Note release##SNType_%d"),i.scan);
                if (ImGui::Button(id)) {
                  noteKeys[i.scan]=102;
                }
              } else if (i.val==100) {
                snprintf(id,4095,_("Note off##SNType_%d"),i.scan);
                if (ImGui::Button(id)) {
                  noteKeys[i.scan]=101;
                }
              } else {
                snprintf(id,4095,_("Note##SNType_%d"),i.scan);
                if (ImGui::Button(id)) {
                  noteKeys[i.scan]=100;
                }
              }
              ImGui::TableNextColumn();
              if (i.val<100) {
                snprintf(id,4095,"##SNValue_%d",i.scan);
                if (ImGui::InputInt(id,&i.val,1,12)) {
                  if (i.val<0) i.val=0;
                  if (i.val>96) i.val=96;
                  noteKeys[i.scan]=i.val;
                  SETTINGS_CHANGED;
                }
              }
              ImGui::TableNextColumn();
              snprintf(id,4095,ICON_FA_TIMES "##SNRemove_%d",i.scan);
              if (ImGui::Button(id)) {
                noteKeys.erase(i.scan);
                SETTINGS_CHANGED;
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
                  noteKeys[(SDL_Scancode)i]=0;
                  SETTINGS_CHANGED;
                }
              }
              ImGui::EndCombo();
            }
          }
        })
      }),
      SettingsCategory(_("Pattern"),{},{
        KEYBIND(GUI_ACTION_PAT_NOTE_UP),
        KEYBIND(GUI_ACTION_PAT_NOTE_DOWN),
        KEYBIND(GUI_ACTION_PAT_OCTAVE_UP),
        KEYBIND(GUI_ACTION_PAT_OCTAVE_DOWN),
        KEYBIND(GUI_ACTION_PAT_VALUE_UP),
        KEYBIND(GUI_ACTION_PAT_VALUE_DOWN),
        KEYBIND(GUI_ACTION_PAT_VALUE_UP_COARSE),
        KEYBIND(GUI_ACTION_PAT_VALUE_DOWN_COARSE),
        KEYBIND(GUI_ACTION_PAT_SELECT_ALL),
        KEYBIND(GUI_ACTION_PAT_CUT),
        KEYBIND(GUI_ACTION_PAT_COPY),
        KEYBIND(GUI_ACTION_PAT_PASTE),
        KEYBIND(GUI_ACTION_PAT_PASTE_MIX),
        KEYBIND(GUI_ACTION_PAT_PASTE_MIX_BG),
        KEYBIND(GUI_ACTION_PAT_PASTE_FLOOD),
        KEYBIND(GUI_ACTION_PAT_PASTE_OVERFLOW),
        KEYBIND(GUI_ACTION_PAT_CURSOR_UP),
        KEYBIND(GUI_ACTION_PAT_CURSOR_DOWN),
        KEYBIND(GUI_ACTION_PAT_CURSOR_LEFT),
        KEYBIND(GUI_ACTION_PAT_CURSOR_RIGHT),
        KEYBIND(GUI_ACTION_PAT_CURSOR_UP_ONE),
        KEYBIND(GUI_ACTION_PAT_CURSOR_DOWN_ONE),
        KEYBIND(GUI_ACTION_PAT_CURSOR_LEFT_CHANNEL),
        KEYBIND(GUI_ACTION_PAT_CURSOR_RIGHT_CHANNEL),
        KEYBIND(GUI_ACTION_PAT_CURSOR_PREVIOUS_CHANNEL),
        KEYBIND(GUI_ACTION_PAT_CURSOR_NEXT_CHANNEL),
        KEYBIND(GUI_ACTION_PAT_CURSOR_BEGIN),
        KEYBIND(GUI_ACTION_PAT_CURSOR_END),
        KEYBIND(GUI_ACTION_PAT_CURSOR_UP_COARSE),
        KEYBIND(GUI_ACTION_PAT_CURSOR_DOWN_COARSE),
        KEYBIND(GUI_ACTION_PAT_SELECTION_UP),
        KEYBIND(GUI_ACTION_PAT_SELECTION_DOWN),
        KEYBIND(GUI_ACTION_PAT_SELECTION_LEFT),
        KEYBIND(GUI_ACTION_PAT_SELECTION_RIGHT),
        KEYBIND(GUI_ACTION_PAT_SELECTION_UP_ONE),
        KEYBIND(GUI_ACTION_PAT_SELECTION_DOWN_ONE),
        KEYBIND(GUI_ACTION_PAT_SELECTION_BEGIN),
        KEYBIND(GUI_ACTION_PAT_SELECTION_END),
        KEYBIND(GUI_ACTION_PAT_SELECTION_UP_COARSE),
        KEYBIND(GUI_ACTION_PAT_SELECTION_DOWN_COARSE),
        KEYBIND(GUI_ACTION_PAT_MOVE_UP),
        KEYBIND(GUI_ACTION_PAT_MOVE_DOWN),
        KEYBIND(GUI_ACTION_PAT_MOVE_LEFT_CHANNEL),
        KEYBIND(GUI_ACTION_PAT_MOVE_RIGHT_CHANNEL),
        KEYBIND(GUI_ACTION_PAT_DELETE),
        KEYBIND(GUI_ACTION_PAT_PULL_DELETE),
        KEYBIND(GUI_ACTION_PAT_INSERT),
        KEYBIND(GUI_ACTION_PAT_MUTE_CURSOR),
        KEYBIND(GUI_ACTION_PAT_SOLO_CURSOR),
        KEYBIND(GUI_ACTION_PAT_UNMUTE_ALL),
        KEYBIND(GUI_ACTION_PAT_NEXT_ORDER),
        KEYBIND(GUI_ACTION_PAT_PREV_ORDER),
        KEYBIND(GUI_ACTION_PAT_COLLAPSE),
        KEYBIND(GUI_ACTION_PAT_INCREASE_COLUMNS),
        KEYBIND(GUI_ACTION_PAT_DECREASE_COLUMNS),
        KEYBIND(GUI_ACTION_PAT_INTERPOLATE),
        KEYBIND(GUI_ACTION_PAT_FADE),
        KEYBIND(GUI_ACTION_PAT_INVERT_VALUES),
        KEYBIND(GUI_ACTION_PAT_FLIP_SELECTION),
        KEYBIND(GUI_ACTION_PAT_COLLAPSE_ROWS),
        KEYBIND(GUI_ACTION_PAT_EXPAND_ROWS),
        KEYBIND(GUI_ACTION_PAT_COLLAPSE_PAT),
        KEYBIND(GUI_ACTION_PAT_EXPAND_PAT),
        KEYBIND(GUI_ACTION_PAT_COLLAPSE_SONG),
        KEYBIND(GUI_ACTION_PAT_EXPAND_SONG),
        KEYBIND(GUI_ACTION_PAT_LATCH),
        KEYBIND(GUI_ACTION_PAT_CLEAR_LATCH),
        KEYBIND(GUI_ACTION_PAT_ABSORB_INSTRUMENT),
        KEYBIND(GUI_ACTION_PAT_CURSOR_UNDO),
        KEYBIND(GUI_ACTION_PAT_CURSOR_REDO),
      }),
      SettingsCategory(_("Instrument list"),{},{
        KEYBIND(GUI_ACTION_INS_LIST_ADD),
        KEYBIND(GUI_ACTION_INS_LIST_DUPLICATE),
        KEYBIND(GUI_ACTION_INS_LIST_OPEN),
        KEYBIND(GUI_ACTION_INS_LIST_OPEN_REPLACE),
        KEYBIND(GUI_ACTION_INS_LIST_SAVE),
        KEYBIND(GUI_ACTION_INS_LIST_SAVE_DMP),
        KEYBIND(GUI_ACTION_INS_LIST_MOVE_UP),
        KEYBIND(GUI_ACTION_INS_LIST_MOVE_DOWN),
        KEYBIND(GUI_ACTION_INS_LIST_DELETE),
        KEYBIND(GUI_ACTION_INS_LIST_EDIT),
        KEYBIND(GUI_ACTION_INS_LIST_UP),
        KEYBIND(GUI_ACTION_INS_LIST_DOWN),
        KEYBIND(GUI_ACTION_INS_LIST_DIR_VIEW),
      }),
      SettingsCategory(_("Wavetable list"),{},{
        KEYBIND(GUI_ACTION_WAVE_LIST_ADD),
        KEYBIND(GUI_ACTION_WAVE_LIST_DUPLICATE),
        KEYBIND(GUI_ACTION_WAVE_LIST_OPEN),
        KEYBIND(GUI_ACTION_WAVE_LIST_OPEN_REPLACE),
        KEYBIND(GUI_ACTION_WAVE_LIST_SAVE),
        KEYBIND(GUI_ACTION_WAVE_LIST_SAVE_DMW),
        KEYBIND(GUI_ACTION_WAVE_LIST_SAVE_RAW),
        KEYBIND(GUI_ACTION_WAVE_LIST_MOVE_UP),
        KEYBIND(GUI_ACTION_WAVE_LIST_MOVE_DOWN),
        KEYBIND(GUI_ACTION_WAVE_LIST_DELETE),
        KEYBIND(GUI_ACTION_WAVE_LIST_EDIT),
        KEYBIND(GUI_ACTION_WAVE_LIST_UP),
        KEYBIND(GUI_ACTION_WAVE_LIST_DOWN),
        KEYBIND(GUI_ACTION_WAVE_LIST_DIR_VIEW),
      }),
      SettingsCategory(_("Sample list"),{},{
        KEYBIND(GUI_ACTION_SAMPLE_LIST_ADD),
        KEYBIND(GUI_ACTION_SAMPLE_LIST_DUPLICATE),
        KEYBIND(GUI_ACTION_SAMPLE_CREATE_WAVE),
        KEYBIND(GUI_ACTION_SAMPLE_LIST_OPEN),
        KEYBIND(GUI_ACTION_SAMPLE_LIST_OPEN_REPLACE),
        KEYBIND(GUI_ACTION_SAMPLE_LIST_OPEN_RAW),
        KEYBIND(GUI_ACTION_SAMPLE_LIST_OPEN_REPLACE_RAW),
        KEYBIND(GUI_ACTION_SAMPLE_LIST_SAVE),
        KEYBIND(GUI_ACTION_SAMPLE_LIST_SAVE_RAW),
        KEYBIND(GUI_ACTION_SAMPLE_LIST_MOVE_UP),
        KEYBIND(GUI_ACTION_SAMPLE_LIST_MOVE_DOWN),
        KEYBIND(GUI_ACTION_SAMPLE_LIST_DELETE),
        KEYBIND(GUI_ACTION_SAMPLE_LIST_EDIT),
        KEYBIND(GUI_ACTION_SAMPLE_LIST_UP),
        KEYBIND(GUI_ACTION_SAMPLE_LIST_DOWN),
        KEYBIND(GUI_ACTION_SAMPLE_LIST_PREVIEW),
        KEYBIND(GUI_ACTION_SAMPLE_LIST_STOP_PREVIEW),
        KEYBIND(GUI_ACTION_SAMPLE_LIST_DIR_VIEW),
        KEYBIND(GUI_ACTION_SAMPLE_LIST_MAKE_MAP),
      }),
      SettingsCategory(_("Orders"),{},{
        KEYBIND(GUI_ACTION_ORDERS_UP),
        KEYBIND(GUI_ACTION_ORDERS_DOWN),
        KEYBIND(GUI_ACTION_ORDERS_LEFT),
        KEYBIND(GUI_ACTION_ORDERS_RIGHT),
        KEYBIND(GUI_ACTION_ORDERS_INCREASE),
        KEYBIND(GUI_ACTION_ORDERS_DECREASE),
        KEYBIND(GUI_ACTION_ORDERS_EDIT_MODE),
        KEYBIND(GUI_ACTION_ORDERS_LINK),
        KEYBIND(GUI_ACTION_ORDERS_ADD),
        KEYBIND(GUI_ACTION_ORDERS_DUPLICATE),
        KEYBIND(GUI_ACTION_ORDERS_DEEP_CLONE),
        KEYBIND(GUI_ACTION_ORDERS_DUPLICATE_END),
        KEYBIND(GUI_ACTION_ORDERS_DEEP_CLONE_END),
        KEYBIND(GUI_ACTION_ORDERS_REMOVE),
        KEYBIND(GUI_ACTION_ORDERS_MOVE_UP),
        KEYBIND(GUI_ACTION_ORDERS_MOVE_DOWN),
        KEYBIND(GUI_ACTION_ORDERS_REPLAY),
      }),
      SettingsCategory(_("Sample editor"),{},{
        KEYBIND(GUI_ACTION_SAMPLE_SELECT),
        KEYBIND(GUI_ACTION_SAMPLE_DRAW),
        KEYBIND(GUI_ACTION_SAMPLE_CUT),
        KEYBIND(GUI_ACTION_SAMPLE_COPY),
        KEYBIND(GUI_ACTION_SAMPLE_PASTE),
        KEYBIND(GUI_ACTION_SAMPLE_PASTE_REPLACE),
        KEYBIND(GUI_ACTION_SAMPLE_PASTE_MIX),
        KEYBIND(GUI_ACTION_SAMPLE_SELECT_ALL),
        KEYBIND(GUI_ACTION_SAMPLE_RESIZE),
        KEYBIND(GUI_ACTION_SAMPLE_RESAMPLE),
        KEYBIND(GUI_ACTION_SAMPLE_AMPLIFY),
        KEYBIND(GUI_ACTION_SAMPLE_NORMALIZE),
        KEYBIND(GUI_ACTION_SAMPLE_FADE_IN),
        KEYBIND(GUI_ACTION_SAMPLE_FADE_OUT),
        KEYBIND(GUI_ACTION_SAMPLE_INSERT),
        KEYBIND(GUI_ACTION_SAMPLE_SILENCE),
        KEYBIND(GUI_ACTION_SAMPLE_DELETE),
        KEYBIND(GUI_ACTION_SAMPLE_TRIM),
        KEYBIND(GUI_ACTION_SAMPLE_REVERSE),
        KEYBIND(GUI_ACTION_SAMPLE_INVERT),
        KEYBIND(GUI_ACTION_SAMPLE_SIGN),
        KEYBIND(GUI_ACTION_SAMPLE_FILTER),
        KEYBIND(GUI_ACTION_SAMPLE_PREVIEW),
        KEYBIND(GUI_ACTION_SAMPLE_STOP_PREVIEW),
        KEYBIND(GUI_ACTION_SAMPLE_ZOOM_IN),
        KEYBIND(GUI_ACTION_SAMPLE_ZOOM_OUT),
        KEYBIND(GUI_ACTION_SAMPLE_ZOOM_AUTO),
        KEYBIND(GUI_ACTION_SAMPLE_MAKE_INS),
        KEYBIND(GUI_ACTION_SAMPLE_SET_LOOP),
      })
    },{
      SETTING(_("Keyboard"),{
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
      })
    }),
    SettingsCategory(_("Interface"),{
      SettingsCategory(_("Layout"),{},{
        SETTING(_("Workspace layout:"),{
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
        }),
        SETTING(_("Allow docking editors"),{
          bool allowEditDockingB=settings.allowEditDocking;
          if (ImGui::Checkbox(_("Allow docking editors"),&allowEditDockingB)) {
            settings.allowEditDocking=allowEditDockingB;
            SETTINGS_CHANGED;
          }
        }),
#ifndef IS_MOBILE
        SETTING(_("Remember window position"),{
          bool saveWindowPosB=settings.saveWindowPos;
          if (ImGui::Checkbox(_("Remember window position"),&saveWindowPosB)) {
            settings.saveWindowPos=saveWindowPosB;
            SETTINGS_CHANGED;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("remembers the window's last position on start-up."));
          }
        }),
#endif
        SETTING(_("Only allow window movement when clicking on title bar"),{
          bool moveWindowTitleB=settings.moveWindowTitle;
          if (ImGui::Checkbox(_("Only allow window movement when clicking on title bar"),&moveWindowTitleB)) {
            settings.moveWindowTitle=moveWindowTitleB;
            applyUISettings(false);
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Center pop-up windows"),{
          bool centerPopupB=settings.centerPopup;
          if (ImGui::Checkbox(_("Center pop-up windows"),&centerPopupB)) {
            settings.centerPopup=centerPopupB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Play/edit controls layout:"),{
          ImGui::Text(_("Play/edit controls layout:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("Classic##ecl0"),settings.controlLayout==0)) {
            settings.controlLayout=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Compact##ecl1"),settings.controlLayout==1)) {
            settings.controlLayout=1;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Compact (vertical)##ecl2"),settings.controlLayout==2)) {
            settings.controlLayout=2;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Split##ecl3"),settings.controlLayout==3)) {
            settings.controlLayout=3;
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
        }),
        SETTING(_("Position of buttons in Orders:"),{
          ImGui::Text(_("Position of buttons in Orders:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("Top##obp0"),settings.orderButtonPos==0)) {
            settings.orderButtonPos=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Left##obp1"),settings.orderButtonPos==1)) {
            settings.orderButtonPos=1;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Right##obp2"),settings.orderButtonPos==2)) {
            settings.orderButtonPos=2;
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
        })
      }),
      SettingsCategory(_("Mouse"),{},{
        SETTING(_("Double-click time (seconds)"),{
          if (CWSliderFloat(_("Double-click time (seconds)"),&settings.doubleClickTime,0.02,1.0,"%.2f")) {
            if (settings.doubleClickTime<0.02) settings.doubleClickTime=0.02;
            if (settings.doubleClickTime>1.0) settings.doubleClickTime=1.0;

            applyUISettings(false);
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Don't raise pattern editor on click"),{
          bool avoidRaisingPatternB=settings.avoidRaisingPattern;
          if (ImGui::Checkbox(_("Don't raise pattern editor on click"),&avoidRaisingPatternB)) {
            settings.avoidRaisingPattern=avoidRaisingPatternB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Focus pattern editor when selecting instrument"),{
          bool insFocusesPatternB=settings.insFocusesPattern;
          if (ImGui::Checkbox(_("Focus pattern editor when selecting instrument"),&insFocusesPatternB)) {
            settings.insFocusesPattern=insFocusesPatternB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Note preview behavior:"),{
          ImGui::Text(_("Note preview behavior:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("Never##npb0"),settings.notePreviewBehavior==0)) {
            settings.notePreviewBehavior=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("When cursor is in Note column##npb1"),settings.notePreviewBehavior==1)) {
            settings.notePreviewBehavior=1;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("When cursor is in Note column or not in edit mode##npb2"),settings.notePreviewBehavior==2)) {
            settings.notePreviewBehavior=2;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Always##npb3"),settings.notePreviewBehavior==3)) {
            settings.notePreviewBehavior=3;
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
        }),
        SETTING(_("Allow dragging selection:"),{
          ImGui::Text(_("Allow dragging selection:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("No##dms0"),settings.dragMovesSelection==0)) {
            settings.dragMovesSelection=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Yes##dms1"),settings.dragMovesSelection==1)) {
            settings.dragMovesSelection=1;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Yes (while holding Ctrl only)##dms2"),settings.dragMovesSelection==2)) {
            settings.dragMovesSelection=2;
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
        }),
        SETTING(_("Toggle channel solo on:"),{
          ImGui::Text(_("Toggle channel solo on:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("Right-click or double-click##soloA"),settings.soloAction==0)) {
            settings.soloAction=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Right-click##soloR"),settings.soloAction==1)) {
            settings.soloAction=1;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Double-click##soloD"),settings.soloAction==2)) {
            settings.soloAction=2;
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
        }),
        SETTING(_("Modifier for alternate wheel-scrolling (vertical/zoom/slider-input):"),{
          ImGui::Text(_("Modifier for alternate wheel-scrolling (vertical/zoom/slider-input):"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("Ctrl or Meta/Cmd##cwm1"),settings.ctrlWheelModifier==0)) {
            settings.ctrlWheelModifier=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Ctrl##cwm2"),settings.ctrlWheelModifier==1)) {
            settings.ctrlWheelModifier=1;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Meta/Cmd##cwm3"),settings.ctrlWheelModifier==2)) {
            settings.ctrlWheelModifier=2;
            SETTINGS_CHANGED;
          }
          // technically this key is called Option on mac, but we call it Alt in getKeyName(s)
          if (ImGui::RadioButton(_("Alt##cwm4"),settings.ctrlWheelModifier==3)) {
            settings.ctrlWheelModifier=3;
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
        }),
        SETTING(_("Double click selects entire column"),{
          bool doubleClickColumnB=settings.doubleClickColumn;
          if (ImGui::Checkbox(_("Double click selects entire column"),&doubleClickColumnB)) {
            settings.doubleClickColumn=doubleClickColumnB;
            SETTINGS_CHANGED;
          }
        })
      }),
      SettingsCategory(_("Cursor behavior"),{},{
        SETTING(_("Insert pushes entire channel row"),{
          bool insertBehaviorB=settings.insertBehavior;
          if (ImGui::Checkbox(_("Insert pushes entire channel row"),&insertBehaviorB)) {
            settings.insertBehavior=insertBehaviorB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Pull delete affects entire channel row"),{
          bool pullDeleteRowB=settings.pullDeleteRow;
          if (ImGui::Checkbox(_("Pull delete affects entire channel row"),&pullDeleteRowB)) {
            settings.pullDeleteRow=pullDeleteRowB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Push value when overwriting instead of clearing it"),{
          bool pushNibbleB=settings.pushNibble;
          if (ImGui::Checkbox(_("Push value when overwriting instead of clearing it"),&pushNibbleB)) {
            settings.pushNibble=pushNibbleB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Keyboard note/value input repeat (hold key to input continuously)"),{
          bool inputRepeatB=settings.inputRepeat;
          if (ImGui::Checkbox(_("Keyboard note/value input repeat (hold key to input continuously)"),&inputRepeatB)) {
            settings.inputRepeat=inputRepeatB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Effect input behavior:"),{
          ImGui::Text(_("Effect input behavior:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("Move down##eicb0"),settings.effectCursorDir==0)) {
            settings.effectCursorDir=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Move to effect value (otherwise move down)##eicb1"),settings.effectCursorDir==1)) {
            settings.effectCursorDir=1;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Move to effect value/next effect and wrap around##eicb2"),settings.effectCursorDir==2)) {
            settings.effectCursorDir=2;
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
        }),
        SETTING(_("Delete effect value when deleting effect"),{
          bool effectDeletionAltersValueB=settings.effectDeletionAltersValue;
          if (ImGui::Checkbox(_("Delete effect value when deleting effect"),&effectDeletionAltersValueB)) {
            settings.effectDeletionAltersValue=effectDeletionAltersValueB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Change current instrument when changing instrument column (absorb)"),{
          bool absorbInsInputB=settings.absorbInsInput;
          if (ImGui::Checkbox(_("Change current instrument when changing instrument column (absorb)"),&absorbInsInputB)) {
            settings.absorbInsInput=absorbInsInputB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Remove instrument value when inserting note off/release"),{
          bool removeInsOffB=settings.removeInsOff;
          if (ImGui::Checkbox(_("Remove instrument value when inserting note off/release"),&removeInsOffB)) {
            settings.removeInsOff=removeInsOffB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Remove volume value when inserting note off/release"),{
          bool removeVolOffB=settings.removeVolOff;
          if (ImGui::Checkbox(_("Remove volume value when inserting note off/release"),&removeVolOffB)) {
            settings.removeVolOff=removeVolOffB;
            SETTINGS_CHANGED;
          }
        })
      }),
      SettingsCategory(_("Cursor movement"),{},{
        SETTING(_("Wrap horizontally:"),{
          ImGui::Text(_("Wrap horizontally:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("No##wrapH0"),settings.wrapHorizontal==0)) {
            settings.wrapHorizontal=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Yes##wrapH1"),settings.wrapHorizontal==1)) {
            settings.wrapHorizontal=1;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Yes, and move to next/prev row##wrapH2"),settings.wrapHorizontal==2)) {
            settings.wrapHorizontal=2;
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
        }),
        SETTING(_("Wrap vertically:"),{
          ImGui::Text(_("Wrap vertically:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("No##wrapV0"),settings.wrapVertical==0)) {
            settings.wrapVertical=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Yes##wrapV1"),settings.wrapVertical==1)) {
            settings.wrapVertical=1;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Yes, and move to next/prev pattern##wrapV2"),settings.wrapVertical==2)) {
            settings.wrapVertical=2;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Yes, and move to next/prev pattern (wrap around)##wrapV2"),settings.wrapVertical==3)) {
            settings.wrapVertical=3;
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
        }),
        SETTING(_("Cursor movement keys behavior:"),{
          ImGui::Text(_("Cursor movement keys behavior:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("Move by one##cmk0"),settings.scrollStep==0)) {
            settings.scrollStep=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Move by Edit Step##cmk1"),settings.scrollStep==1)) {
            settings.scrollStep=1;
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
        }),
        SETTING(_("Move cursor by edit step on delete"),{
          bool stepOnDeleteB=settings.stepOnDelete;
          if (ImGui::Checkbox(_("Move cursor by edit step on delete"),&stepOnDeleteB)) {
            settings.stepOnDelete=stepOnDeleteB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Move cursor by edit step on insert (push)"),{
          bool stepOnInsertB=settings.stepOnInsert;
          if (ImGui::Checkbox(_("Move cursor by edit step on insert (push)"),&stepOnInsertB)) {
            settings.stepOnInsert=stepOnInsertB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Move cursor up on backspace-delete"),{
          bool pullDeleteBehaviorB=settings.pullDeleteBehavior;
          if (ImGui::Checkbox(_("Move cursor up on backspace-delete"),&pullDeleteBehaviorB)) {
            settings.pullDeleteBehavior=pullDeleteBehaviorB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Move cursor to end of clipboard content when pasting"),{
          bool cursorPastePosB=settings.cursorPastePos;
          if (ImGui::Checkbox(_("Move cursor to end of clipboard content when pasting"),&cursorPastePosB)) {
            settings.cursorPastePos=cursorPastePosB;
            SETTINGS_CHANGED;
          }
        })
      }),
      SettingsCategory(_("Scrolling"),{},{
        SETTING(_("Change order when scrolling outside of pattern bounds:"),{
          ImGui::Text(_("Change order when scrolling outside of pattern bounds:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("No##pscroll0"),settings.scrollChangesOrder==0)) {
            settings.scrollChangesOrder=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Yes##pscroll1"),settings.scrollChangesOrder==1)) {
            settings.scrollChangesOrder=1;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Yes, and wrap around song##pscroll2"),settings.scrollChangesOrder==2)) {
            settings.scrollChangesOrder=2;
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
        }),
        SETTING(_("Cursor follows current order when moving it"),{
          bool cursorFollowsOrderB=settings.cursorFollowsOrder;
          if (ImGui::Checkbox(_("Cursor follows current order when moving it"),&cursorFollowsOrderB)) {
            settings.cursorFollowsOrder=cursorFollowsOrderB;
            SETTINGS_CHANGED;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("applies when playback is stopped."));
          }
        }),
        SETTING(_("Don't scroll when moving cursor"),{
          bool cursorMoveNoScrollB=settings.cursorMoveNoScroll;
          if (ImGui::Checkbox(_("Don't scroll when moving cursor"),&cursorMoveNoScrollB)) {
            settings.cursorMoveNoScroll=cursorMoveNoScrollB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Move cursor with scroll wheel:"),{
          ImGui::Text(_("Move cursor with scroll wheel:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("No##csw0"),settings.cursorFollowsWheel==0)) {
            settings.cursorFollowsWheel=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Yes##csw1"),settings.cursorFollowsWheel==1)) {
            settings.cursorFollowsWheel=1;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Inverted##csw2"),settings.cursorFollowsWheel==2)) {
            settings.cursorFollowsWheel=2;
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
        }),
        SETTING_COND(_("How many steps to move with each scroll wheel step?"),{
          ImGui::Text(_("How many steps to move with each scroll wheel step?"));
          if (ImGui::RadioButton(_("One##cws0"),settings.cursorWheelStep==0)) {
            settings.cursorWheelStep=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Edit Step##cws1"),settings.cursorWheelStep==1)) {
            settings.cursorWheelStep=1;
            SETTINGS_CHANGED;
          }
        },settings.cursorFollowsWheel)
      }),
      SettingsCategory(_("Assets"),{},{
        SETTING(_("Display instrument type menu when adding instrument"),{
          bool insTypeMenuB=settings.insTypeMenu;
          if (ImGui::Checkbox(_("Display instrument type menu when adding instrument"),&insTypeMenuB)) {
            settings.insTypeMenu=insTypeMenuB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Select asset after opening one"),{
          bool selectAssetOnLoadB=settings.selectAssetOnLoad;
          if (ImGui::Checkbox(_("Select asset after opening one"),&selectAssetOnLoadB)) {
            settings.selectAssetOnLoad=selectAssetOnLoadB;
            SETTINGS_CHANGED;
          }
        })
      })
    },{}),
    SettingsCategory(_("Appearance"),{
      SettingsCategory(_("Scaling"),{},{
        SETTING(_("Automatic UI scaling factor"),{
          bool dpiScaleAuto=(settings.dpiScale<0.5f);
          if (ImGui::Checkbox(_("Automatic UI scaling factor"),&dpiScaleAuto)) {
            if (dpiScaleAuto) {
              settings.dpiScale=0.0f;
            } else {
              settings.dpiScale=1.0f;
            }
            SETTINGS_CHANGED;
          }
        }),
        SETTING_COND(_("UI scaling factor"),{
          if (ImGui::SliderFloat(_("UI scaling factor"),&settings.dpiScale,1.0f,3.0f,"%.2fx")) {
            if (settings.dpiScale<0.5f) settings.dpiScale=0.5f;
            if (settings.dpiScale>3.0f) settings.dpiScale=3.0f;
            SETTINGS_CHANGED;
          } rightClickable
        },!(settings.dpiScale<0.5f)),
        SETTING(_("Icon size"),{
          if (ImGui::InputInt(_("Icon size"),&settings.iconSize,1,3)) {
            if (settings.iconSize<3) settings.iconSize=3;
            if (settings.iconSize>48) settings.iconSize=48;
            SETTINGS_CHANGED;
          }
        }),
      }),
      SettingsCategory(_("Text"),{},{
#ifdef HAVE_FREETYPE
        SETTING(_("Font renderer"),{
          if (ImGui::Combo(_("Font renderer"),&settings.fontBackend,fontBackends,2)) SETTINGS_CHANGED;
        }),
#else
        SETTING(NULL,{settings.fontBackend=0;}),
#endif
        SETTING_SEPARATOR,
        SETTING(_("Main font"),{
          if (ImGui::Combo(_("Main font"),&settings.mainFont,LocalizedComboGetter,mainFonts,7)) SETTINGS_CHANGED;
        }),
        SETTING_COND(_("Main font"),{
          ImGui::InputText("##MainFontPath",&settings.mainFontPath);
          ImGui::SameLine();
          if (ImGui::Button(ICON_FA_FOLDER "##MainFontLoad")) {
            openFileDialog(GUI_FILE_LOAD_MAIN_FONT);
            SETTINGS_CHANGED;
          }
        },settings.mainFont==6),
        SETTING(_("Main font size"),{
          if (ImGui::InputInt(_("Main font size"),&settings.mainFontSize,1,3)) {
            if (settings.mainFontSize<3) settings.mainFontSize=3;
            if (settings.mainFontSize>96) settings.mainFontSize=96;
            SETTINGS_CHANGED;
          }
        }),
        SETTING_NEWLINE,
        SETTING(_("Header font"),{
          if (ImGui::Combo(_("Header font"),&settings.headFont,LocalizedComboGetter,mainFonts,7)) SETTINGS_CHANGED;
        }),
        SETTING_COND(_("Header font"),{
          ImGui::InputText("##HeadFontPath",&settings.headFontPath);
          ImGui::SameLine();
          if (ImGui::Button(ICON_FA_FOLDER "##HeadFontLoad")) {
            openFileDialog(GUI_FILE_LOAD_HEAD_FONT);
            SETTINGS_CHANGED;
          }
        },settings.headFont==6),
        SETTING(_("Header font size"),{
          if (ImGui::InputInt(_("Header font size"),&settings.headFontSize,1,3)) {
            if (settings.headFontSize<3) settings.headFontSize=3;
            if (settings.headFontSize>96) settings.headFontSize=96;
            SETTINGS_CHANGED;
          }
        }),
        SETTING_NEWLINE,
        SETTING(_("Pattern font"),{
          if (ImGui::Combo(_("Pattern font"),&settings.patFont,LocalizedComboGetter,mainFonts,7)) SETTINGS_CHANGED;
        }),
        SETTING_COND(_("Pattern font"),{
          ImGui::InputText("##PatFontPath",&settings.patFontPath);
          ImGui::SameLine();
          if (ImGui::Button(ICON_FA_FOLDER "##PatFontLoad")) {
            openFileDialog(GUI_FILE_LOAD_PAT_FONT);
            SETTINGS_CHANGED;
          }
        },settings.patFont==6),
        SETTING(_("Pattern font size"),{
          if (ImGui::InputInt(_("Pattern font size"),&settings.patFontSize,1,3)) {
            if (settings.patFontSize<3) settings.patFontSize=3;
            if (settings.patFontSize>96) settings.patFontSize=96;
            SETTINGS_CHANGED;
          }
        }),
        SETTING_COND(_("Anti-aliased fonts"),{
          bool fontAntiAliasB=settings.fontAntiAlias;
          if (ImGui::Checkbox(_("Anti-aliased fonts"),&fontAntiAliasB)) {
            settings.fontAntiAlias=fontAntiAliasB;
            SETTINGS_CHANGED;
          }
        },settings.fontBackend==1),
        SETTING_COND(_("Support bitmap fonts"),{
          bool fontBitmapB=settings.fontBitmap;
          if (ImGui::Checkbox(_("Support bitmap fonts"),&fontBitmapB)) {
            settings.fontBitmap=fontBitmapB;
            SETTINGS_CHANGED;
          }
        },settings.fontBackend==1),
        SETTING_COND(_("Hinting:"),{
          ImGui::Text(_("Hinting:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("Off (soft)##fh0"),settings.fontHinting==0)) {
            settings.fontHinting=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Slight##fh1"),settings.fontHinting==1)) {
            settings.fontHinting=1;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Normal##fh2"),settings.fontHinting==2)) {
            settings.fontHinting=2;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Full (hard)##fh3"),settings.fontHinting==3)) {
            settings.fontHinting=3;
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
        },settings.fontBackend==1),
        SETTING_COND(_("Auto-hinter:"),{
          ImGui::Text(_("Auto-hinter:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("Disable##fah0"),settings.fontAutoHint==0)) {
            settings.fontAutoHint=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Enable##fah1"),settings.fontAutoHint==1)) {
            settings.fontAutoHint=1;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Force##fah2"),settings.fontAutoHint==2)) {
            settings.fontAutoHint=2;
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
        },settings.fontBackend==1),
        SETTING(_("Oversample"),{
          ImGui::Text(_("Oversample"));

          ImGui::SameLine();
          if (ImGui::RadioButton(_("1×##fos1"),settings.fontOversample==1)) {
            settings.fontOversample=1;
            SETTINGS_CHANGED;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("saves video memory. reduces font rendering quality.\nuse for pixel/bitmap fonts."));
          }
          ImGui::SameLine();
          if (ImGui::RadioButton(_("2×##fos2"),settings.fontOversample==2)) {
            settings.fontOversample=2;
            SETTINGS_CHANGED;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("default."));
          }
          ImGui::SameLine();
          if (ImGui::RadioButton(_("3×##fos3"),settings.fontOversample==3)) {
            settings.fontOversample=3;
            SETTINGS_CHANGED;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("slightly better font rendering quality.\nuses more video memory."));
          }
        }),
        SETTING(_("Load fallback font"),{
          bool loadFallbackB=settings.loadFallback;
          if (ImGui::Checkbox(_("Load fallback font"),&loadFallbackB)) {
            settings.loadFallback=loadFallbackB;
            SETTINGS_CHANGED;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("disable to save video memory."));
          }
        }),
        SETTING(_("Display Japanese characters"),{
          bool loadJapaneseB=settings.loadJapanese;
          if (ImGui::Checkbox(_("Display Japanese characters"),&loadJapaneseB)) {
            settings.loadJapanese=loadJapaneseB;
            SETTINGS_CHANGED;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_(
              "Only toggle this option if you have enough graphics memory.\n"
              "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
              "このオプションは、十分なグラフィックメモリがある場合にのみ切り替えてください。\n"
              "これは、Dear ImGuiにダイナミックフォントアトラスが実装されるまでの一時的な解決策です。"
            ));
          }
        }),
        SETTING(_("Display Chinese (Simplified) characters"),{
          bool loadChineseB=settings.loadChinese;
          if (ImGui::Checkbox(_("Display Chinese (Simplified) characters"),&loadChineseB)) {
            settings.loadChinese=loadChineseB;
            SETTINGS_CHANGED;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_(
              "Only toggle this option if you have enough graphics memory.\n"
              "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
              "请在确保你有足够的显存后再启动此设定\n"
              "这是一个在ImGui实现动态字体加载之前的临时解决方案"
            ));
          }
        }),
        SETTING(_("Display Chinese (Traditional) characters"),{
          bool loadChineseTraditionalB=settings.loadChineseTraditional;
          if (ImGui::Checkbox(_("Display Chinese (Traditional) characters"),&loadChineseTraditionalB)) {
            settings.loadChineseTraditional=loadChineseTraditionalB;
            SETTINGS_CHANGED;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_(
              "Only toggle this option if you have enough graphics memory.\n"
              "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
              "請在確保你有足夠的顯存后再啟動此設定\n"
              "這是一個在ImGui實現動態字體加載之前的臨時解決方案"
            ));
          }
        }),
        SETTING(_("Display Korean characters"),{
          bool loadKoreanB=settings.loadKorean;
          if (ImGui::Checkbox(_("Display Korean characters"),&loadKoreanB)) {
            settings.loadKorean=loadKoreanB;
            SETTINGS_CHANGED;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_(
              "Only toggle this option if you have enough graphics memory.\n"
              "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
              "그래픽 메모리가 충분한 경우에만 이 옵션을 선택하십시오.\n"
              "이 옵션은 Dear ImGui에 동적 글꼴 아틀라스가 구현될 때까지 임시 솔루션입니다."
            ));
          }
        })
      }),
      SettingsCategory(_("Program"),{},{
        SETTING(_("Title bar:"),{
          ImGui::Text(_("Title bar:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("Furnace##tbar0"),settings.titleBarInfo==0)) {
            settings.titleBarInfo=0;
            updateWindowTitle();
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Song Name - Furnace##tbar1"),settings.titleBarInfo==1)) {
            settings.titleBarInfo=1;
            updateWindowTitle();
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("file_name.fur - Furnace##tbar2"),settings.titleBarInfo==2)) {
            settings.titleBarInfo=2;
            updateWindowTitle();
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("/path/to/file.fur - Furnace##tbar3"),settings.titleBarInfo==3)) {
            settings.titleBarInfo=3;
            updateWindowTitle();
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
        }),
        SETTING(_("Display system name on title bar"),{
          bool titleBarSysB=settings.titleBarSys;
          if (ImGui::Checkbox(_("Display system name on title bar"),&titleBarSysB)) {
            settings.titleBarSys=titleBarSysB;
            updateWindowTitle();
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Display chip names instead of \"multi-system\" in title bar"),{
          bool noMultiSystemB=settings.noMultiSystem;
          if (ImGui::Checkbox(_("Display chip names instead of \"multi-system\" in title bar"),&noMultiSystemB)) {
            settings.noMultiSystem=noMultiSystemB;
            updateWindowTitle();
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Status bar:"),{
          ImGui::Text(_("Status bar:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("Cursor details##sbar0"),settings.statusDisplay==0)) {
            settings.statusDisplay=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("File path##sbar1"),settings.statusDisplay==1)) {
            settings.statusDisplay=1;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Cursor details or file path##sbar2"),settings.statusDisplay==2)) {
            settings.statusDisplay=2;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Nothing##sbar3"),settings.statusDisplay==3)) {
            settings.statusDisplay=3;
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
        }),
        SETTING(_("Display playback status when playing"),{
          bool playbackTimeB=settings.playbackTime;
          if (ImGui::Checkbox(_("Display playback status when playing"),&playbackTimeB)) {
            settings.playbackTime=playbackTimeB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Export options layout:"),{
          ImGui::Text(_("Export options layout:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("Sub-menus in File menu##eol0"),settings.exportOptionsLayout==0)) {
            settings.exportOptionsLayout=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Modal window with tabs##eol1"),settings.exportOptionsLayout==1)) {
            settings.exportOptionsLayout=1;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Modal windows with options in File menu##eol2"),settings.exportOptionsLayout==2)) {
            settings.exportOptionsLayout=2;
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
        }),
        SETTING(_("Capitalize menu bar"),{
          bool capitalMenuBarB=settings.capitalMenuBar;
          if (ImGui::Checkbox(_("Capitalize menu bar"),&capitalMenuBarB)) {
            settings.capitalMenuBar=capitalMenuBarB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Display add/configure/change/remove chip menus in File menu"),{
          bool classicChipOptionsB=settings.classicChipOptions;
          if (ImGui::Checkbox(_("Display add/configure/change/remove chip menus in File menu"),&classicChipOptionsB)) {
            settings.classicChipOptions=classicChipOptionsB;
            SETTINGS_CHANGED;
          }
        })
      }),
      SettingsCategory(_("Orders"),{},{
        // sorry. temporarily disabled until ImGui has a way to add separators in tables arbitrarily.
        /*bool sysSeparatorsB=settings.sysSeparators;
        if (ImGui::Checkbox(_("Add separators between systems in Orders"),&sysSeparatorsB)) {
          settings.sysSeparators=sysSeparatorsB;
        }*/
        SETTING(_("Highlight channel at cursor in Orders"),{
          bool ordersCursorB=settings.ordersCursor;
          if (ImGui::Checkbox(_("Highlight channel at cursor in Orders"),&ordersCursorB)) {
            settings.ordersCursor=ordersCursorB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Orders row number format:"),{
          ImGui::Text(_("Orders row number format:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("Decimal##orbD"),settings.orderRowsBase==0)) {
            settings.orderRowsBase=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Hexadecimal##orbH"),settings.orderRowsBase==1)) {
            settings.orderRowsBase=1;
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
        })
      }),
      SettingsCategory(_("Pattern"),{},{
        SETTING(_("Center pattern view"),{
          bool centerPatternB=settings.centerPattern;
          if (ImGui::Checkbox(_("Center pattern view"),&centerPatternB)) {
            settings.centerPattern=centerPatternB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Overflow pattern highlights"),{
          bool overflowHighlightB=settings.overflowHighlight;
          if (ImGui::Checkbox(_("Overflow pattern highlights"),&overflowHighlightB)) {
            settings.overflowHighlight=overflowHighlightB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Display previous/next pattern"),{
          bool viewPrevPatternB=settings.viewPrevPattern;
          if (ImGui::Checkbox(_("Display previous/next pattern"),&viewPrevPatternB)) {
            settings.viewPrevPattern=viewPrevPatternB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Pattern row number format:"),{
          ImGui::Text(_("Pattern row number format:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("Decimal##prbD"),settings.patRowsBase==0)) {
            settings.patRowsBase=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Hexadecimal##prbH"),settings.patRowsBase==1)) {
            settings.patRowsBase=1;
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
        }),
        Setting(NULL,[]{
          ImGui::Text(_("Pattern view labels:"));
        }),
        SETTING(_("Note off (3-char)"),{
          ImGui::PushFont(patFont);
          if (ImGui::InputTextWithHint("##PVLOff","OFF",&settings.noteOffLabel)) SETTINGS_CHANGED;
          ImGui::PopFont();
          ImGui::SameLine();
          ImGui::Text(_("Note off (3-char)"));
        }),
        SETTING(_("Note release (3-char)"),{
          ImGui::PushFont(patFont);
          if (ImGui::InputTextWithHint("##PVLRel","===",&settings.noteRelLabel)) SETTINGS_CHANGED;
          ImGui::PopFont();
          ImGui::SameLine();
          ImGui::Text(_("Note release (3-char)"));
        }),
        SETTING(_("Macro release (3-char)"),{
          ImGui::PushFont(patFont);
          if (ImGui::InputTextWithHint("##PVLMacroRel","REL",&settings.macroRelLabel)) SETTINGS_CHANGED;
          ImGui::PopFont();
          ImGui::SameLine();
          ImGui::Text(_("Macro release (3-char)"));
        }),
        SETTING(_("Empty field (3-char)"),{
          ImGui::PushFont(patFont);
          if (ImGui::InputTextWithHint("##PVLE3","...",&settings.emptyLabel)) SETTINGS_CHANGED;
          ImGui::PopFont();
          ImGui::SameLine();
          ImGui::Text(_("Empty field (3-char)"));
        }),
        SETTING(_("Empty field (2-char)"),{
          ImGui::PushFont(patFont);
          if (ImGui::InputTextWithHint("##PVLE2","..",&settings.emptyLabel2)) SETTINGS_CHANGED;
          ImGui::PopFont();
          ImGui::SameLine();
          ImGui::Text(_("Empty field (2-char)"));
        }),
        Setting(NULL,[]{
          ImGui::Text(_("Pattern view spacing after:"));
        }),
        SETTING(_("Note"),{
          if (CWSliderInt(_("Note"),&settings.noteCellSpacing,0,32)) {
            if (settings.noteCellSpacing<0) settings.noteCellSpacing=0;
            if (settings.noteCellSpacing>32) settings.noteCellSpacing=32;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Instrument"),{
          if (CWSliderInt(_("Instrument"),&settings.insCellSpacing,0,32)) {
            if (settings.insCellSpacing<0) settings.insCellSpacing=0;
            if (settings.insCellSpacing>32) settings.insCellSpacing=32;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Volume"),{
          if (CWSliderInt(_("Volume"),&settings.volCellSpacing,0,32)) {
            if (settings.volCellSpacing<0) settings.volCellSpacing=0;
            if (settings.volCellSpacing>32) settings.volCellSpacing=32;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Effect"),{
          if (CWSliderInt(_("Effect"),&settings.effectCellSpacing,0,32)) {
            if (settings.effectCellSpacing<0) settings.effectCellSpacing=0;
            if (settings.effectCellSpacing>32) settings.effectCellSpacing=32;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Effect value"),{
          if (CWSliderInt(_("Effect value"),&settings.effectValCellSpacing,0,32)) {
            if (settings.effectValCellSpacing<0) settings.effectValCellSpacing=0;
            if (settings.effectValCellSpacing>32) settings.effectValCellSpacing=32;
            SETTINGS_CHANGED;
          }
        }),
        SETTING_SEPARATOR,
        SETTING(_("Single-digit effects for 00-0F"),{
          bool oneDigitEffectsB=settings.oneDigitEffects;
          if (ImGui::Checkbox(_("Single-digit effects for 00-0F"),&oneDigitEffectsB)) {
            settings.oneDigitEffects=oneDigitEffectsB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Use flats instead of sharps"),{
          bool flatNotesB=settings.flatNotes;
          if (ImGui::Checkbox(_("Use flats instead of sharps"),&flatNotesB)) {
            settings.flatNotes=flatNotesB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Use German notation"),{
          bool germanNotationB=settings.germanNotation;
          if (ImGui::Checkbox(_("Use German notation"),&germanNotationB)) {
            settings.germanNotation=germanNotationB;
            SETTINGS_CHANGED;
          }
        })
      }),
      SettingsCategory(_("Channel"),{},{
        SETTING(_("Channel style:"),{
          ImGui::Text(_("Channel style:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("Classic##CHS0"),settings.channelStyle==0)) {
            settings.channelStyle=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Line##CHS1"),settings.channelStyle==1)) {
            settings.channelStyle=1;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Round##CHS2"),settings.channelStyle==2)) {
            settings.channelStyle=2;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Split button##CHS3"),settings.channelStyle==3)) {
            settings.channelStyle=3;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Square border##CH42"),settings.channelStyle==4)) {
            settings.channelStyle=4;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Round border##CHS5"),settings.channelStyle==5)) {
            settings.channelStyle=5;
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
        }),
        SETTING(_("Channel volume bar:"),{
          ImGui::Text(_("Channel volume bar:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("None##CHV0"),settings.channelVolStyle==0)) {
            settings.channelVolStyle=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Simple##CHV1"),settings.channelVolStyle==1)) {
            settings.channelVolStyle=1;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Stereo##CHV2"),settings.channelVolStyle==2)) {
            settings.channelVolStyle=2;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Real##CHV3"),settings.channelVolStyle==3)) {
            settings.channelVolStyle=3;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Real (stereo)##CHV4"),settings.channelVolStyle==4)) {
            settings.channelVolStyle=4;
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
        }),
        SETTING(_("Channel feedback style:"),{
          ImGui::Text(_("Channel feedback style:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("Off##CHF0"),settings.channelFeedbackStyle==0)) {
            settings.channelFeedbackStyle=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Note##CHF1"),settings.channelFeedbackStyle==1)) {
            settings.channelFeedbackStyle=1;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Volume##CHF2"),settings.channelFeedbackStyle==2)) {
            settings.channelFeedbackStyle=2;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Active##CHF3"),settings.channelFeedbackStyle==3)) {
            settings.channelFeedbackStyle=3;
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
        }),
        SETTING(_("Channel font:"),{
          ImGui::Text(_("Channel font:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("Regular##CHFont0"),settings.channelFont==0)) {
            settings.channelFont=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Monospace##CHFont1"),settings.channelFont==1)) {
            settings.channelFont=1;
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
        }),
        SETTING(_("Center channel name"),{
          bool channelTextCenterB=settings.channelTextCenter;
          if (ImGui::Checkbox(_("Center channel name"),&channelTextCenterB)) {
            settings.channelTextCenter=channelTextCenterB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Channel colors:"),{
          ImGui::Text(_("Channel colors:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("Single##CHC0"),settings.channelColors==0)) {
            settings.channelColors=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Channel type##CHC1"),settings.channelColors==1)) {
            settings.channelColors=1;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Instrument type##CHC2"),settings.channelColors==2)) {
            settings.channelColors=2;
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
        }),
        SETTING(_("Channel name colors:"),{
          ImGui::Text(_("Channel name colors:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("Single##CTC0"),settings.channelTextColors==0)) {
            settings.channelTextColors=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Channel type##CTC1"),settings.channelTextColors==1)) {
            settings.channelTextColors=1;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Instrument type##CTC2"),settings.channelTextColors==2)) {
            settings.channelTextColors=2;
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
        })
      }),
      SettingsCategory(_("Assets"),{},{
        SETTING(_("Unified instrument/wavetable/sample list"),{
          bool unifiedDataViewB=settings.unifiedDataView;
          if (ImGui::Checkbox(_("Unified instrument/wavetable/sample list"),&unifiedDataViewB)) {
            settings.unifiedDataView=unifiedDataViewB;
            SETTINGS_CHANGED;
          }
          if (settings.unifiedDataView) {
            settings.horizontalDataView=0;
          }
        }),
        SETTING(_("Horizontal instrument/wavetable list"),{
          ImGui::BeginDisabled(settings.unifiedDataView);
          bool horizontalDataViewB=settings.horizontalDataView;
          if (ImGui::Checkbox(_("Horizontal instrument/wavetable list"),&horizontalDataViewB)) {
            settings.horizontalDataView=horizontalDataViewB;
            SETTINGS_CHANGED;
          }
          ImGui::EndDisabled();
        }),
        SETTING(_("Instrument list icon style:"),{
          ImGui::Text(_("Instrument list icon style:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("None##iis0"),settings.insIconsStyle==0)) {
            settings.insIconsStyle=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Graphical icons##iis1"),settings.insIconsStyle==1)) {
            settings.insIconsStyle=1;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Letter icons##iis2"),settings.insIconsStyle==2)) {
            settings.insIconsStyle=2;
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
        }),
        SETTING(_("Colorize instrument editor using instrument type"),{
          bool insEditColorizeB=settings.insEditColorize;
          if (ImGui::Checkbox(_("Colorize instrument editor using instrument type"),&insEditColorizeB)) {
            settings.insEditColorize=insEditColorizeB;
            SETTINGS_CHANGED;
          }
        })
      }),
      SettingsCategory(_("Macro Editor"),{},{
        SETTING(_("Macro editor layout:"),{
          ImGui::Text(_("Macro editor layout:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("Unified##mel0"),settings.macroLayout==0)) {
            settings.macroLayout=0;
            SETTINGS_CHANGED;
          }
          /*
          if (ImGui::RadioButton(_("Tabs##mel1"),settings.macroLayout==1)) {
            settings.macroLayout=1;
            SETTINGS_CHANGED;
          }
          */
          if (ImGui::RadioButton(_("Grid##mel2"),settings.macroLayout==2)) {
            settings.macroLayout=2;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Single (with list)##mel3"),settings.macroLayout==3)) {
            settings.macroLayout=3;
            SETTINGS_CHANGED;
          }
          /*
          if (ImGui::RadioButton(_("Single (combo box)##mel4"),settings.macroLayout==4)) {
            settings.macroLayout=4;
            SETTINGS_CHANGED;
          }
          */
          ImGui::Unindent();
        }),
        SETTING(_("Use classic macro editor vertical slider"),{
          bool oldMacroVSliderB=settings.oldMacroVSlider;
          if (ImGui::Checkbox(_("Use classic macro editor vertical slider"),&oldMacroVSliderB)) {
            settings.oldMacroVSlider=oldMacroVSliderB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Macro step size/horizontal zoom:"),{
          ImGui::BeginDisabled(settings.macroLayout==2);
          ImGui::Text(_("Macro step size/horizontal zoom:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("Manual"),settings.autoMacroStepSize==0)) {
            settings.autoMacroStepSize=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Automatic per macro"),settings.autoMacroStepSize==1)) {
            settings.autoMacroStepSize=1;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Automatic (use longest macro)"),settings.autoMacroStepSize==2)) {
            settings.autoMacroStepSize=2;
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
          ImGui::EndDisabled();
        })
      }),
      SettingsCategory(_("Wave Editor"),{},{
        SETTING(_("Use compact wave editor"),{
          bool waveLayoutB=settings.waveLayout;
          if (ImGui::Checkbox(_("Use compact wave editor"),&waveLayoutB)) {
            settings.waveLayout=waveLayoutB;
            SETTINGS_CHANGED;
          }
        })
      }),
      SettingsCategory(_("FM Editor"),{},{
        SETTING(_("FM parameter names:"),{
          ImGui::Text(_("FM parameter names:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("Friendly##fmn0"),settings.fmNames==0)) {
            settings.fmNames=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Technical##fmn1"),settings.fmNames==1)) {
            settings.fmNames=1;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Technical (alternate)##fmn2"),settings.fmNames==2)) {
            settings.fmNames=2;
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
        }),
        SETTING(_("Use standard OPL waveform names"),{
          bool oplStandardWaveNamesB=settings.oplStandardWaveNames;
          if (ImGui::Checkbox(_("Use standard OPL waveform names"),&oplStandardWaveNamesB)) {
            settings.oplStandardWaveNames=oplStandardWaveNamesB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("FM parameter editor layout:"),{
          ImGui::Text(_("FM parameter editor layout:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("Modern##fml0"),settings.fmLayout==0)) {
            settings.fmLayout=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Modern with more labels##fml7"),settings.fmLayout==7)) {
            settings.fmLayout=7;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Compact (2x2, classic)##fml1"),settings.fmLayout==1)) {
            settings.fmLayout=1;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Compact (1x4)##fml2"),settings.fmLayout==2)) {
            settings.fmLayout=2;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Compact (4x1)##fml3"),settings.fmLayout==3)) {
            settings.fmLayout=3;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Alternate (2x2)##fml4"),settings.fmLayout==4)) {
            settings.fmLayout=4;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Alternate (1x4)##fml5"),settings.fmLayout==5)) {
            settings.fmLayout=5;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Alternate (4x1)##fml5"),settings.fmLayout==6)) {
            settings.fmLayout=6;
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
        }),
        SETTING(_("Position of Sustain in FM editor:"),{
          ImGui::Text(_("Position of Sustain in FM editor:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("Between Decay and Sustain Rate##susp0"),settings.susPosition==0)) {
            settings.susPosition=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("After Release Rate##susp1"),settings.susPosition==1)) {
            settings.susPosition=1;
            SETTINGS_CHANGED;
          }
          ImGui::BeginDisabled(settings.fmLayout!=0);
          if (ImGui::RadioButton(_("After Release Rate, after spacing##susp2"),settings.susPosition==2)) {
            settings.susPosition=2;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("After TL##susp3"),settings.susPosition==3)) {
            settings.susPosition=3;
            SETTINGS_CHANGED;
          }
          ImGui::EndDisabled();
          ImGui::Unindent();
        }),
        SETTING(_("Use separate colors for carriers/modulators in FM editor"),{
          bool separateFMColorsB=settings.separateFMColors;
          if (ImGui::Checkbox(_("Use separate colors for carriers/modulators in FM editor"),&separateFMColorsB)) {
            settings.separateFMColors=separateFMColorsB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Unsigned FM detune values"),{
          bool unsignedDetuneB=settings.unsignedDetune;
          if (ImGui::Checkbox(_("Unsigned FM detune values"),&unsignedDetuneB)) {
            settings.unsignedDetune=unsignedDetuneB;
            SETTINGS_CHANGED;
          }
        })
      }),
      SettingsCategory(_("Memory Composition"),{},{
        SETTING(_("Chip memory usage unit:"),{
          ImGui::Text(_("Chip memory usage unit:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("Bytes##MUU0"),settings.memUsageUnit==0)) {
            settings.memUsageUnit=0;
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Kilobytes##MUU1"),settings.memUsageUnit==1)) {
            settings.memUsageUnit=1;
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
        })
      }),
      SettingsCategory(_("Oscilloscope"),{},{
        SETTING(_("Rounded corners"),{
          bool oscRoundedCornersB=settings.oscRoundedCorners;
          if (ImGui::Checkbox(_("Rounded corners"),&oscRoundedCornersB)) {
            settings.oscRoundedCorners=oscRoundedCornersB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Border"),{
          bool oscBorderB=settings.oscBorder;
          if (ImGui::Checkbox(_("Border"),&oscBorderB)) {
            settings.oscBorder=oscBorderB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Mono"),{
          bool oscMonoB=settings.oscMono;
          if (ImGui::Checkbox(_("Mono"),&oscMonoB)) {
            settings.oscMono=oscMonoB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Anti-aliased"),{
          bool oscAntiAliasB=settings.oscAntiAlias;
          if (ImGui::Checkbox(_("Anti-aliased"),&oscAntiAliasB)) {
            settings.oscAntiAlias=oscAntiAliasB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Fill entire window"),{
          bool oscTakesEntireWindowB=settings.oscTakesEntireWindow;
          if (ImGui::Checkbox(_("Fill entire window"),&oscTakesEntireWindowB)) {
            settings.oscTakesEntireWindow=oscTakesEntireWindowB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Waveform goes out of bounds"),{
          bool oscEscapesBoundaryB=settings.oscEscapesBoundary;
          if (ImGui::Checkbox(_("Waveform goes out of bounds"),&oscEscapesBoundaryB)) {
            settings.oscEscapesBoundary=oscEscapesBoundaryB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Line size"),{
          if (ImGui::SliderFloat(_("Line size"),&settings.oscLineSize,0.25f,16.0f,"%.1f")) {
            if (settings.oscLineSize<0.25f) settings.oscLineSize=0.25f;
            if (settings.oscLineSize>16.0f) settings.oscLineSize=16.0f;
            SETTINGS_CHANGED;
          } rightClickable
        })
      }),
      SettingsCategory(_("Song Comments"),{},{
        SETTING(_("Wrap text"),{
          bool songNotesWrapB=settings.songNotesWrap;
          if (ImGui::Checkbox(_("Wrap text"), &songNotesWrapB)) {
            settings.songNotesWrap=songNotesWrapB;
            settingsChanged=true;
          }
        }),
      }),
      SettingsCategory(_("Windows"),{},{
        SETTING(_("Rounded window corners"),{
          bool roundedWindowsB=settings.roundedWindows;
          if (ImGui::Checkbox(_("Rounded window corners"),&roundedWindowsB)) {
            settings.roundedWindows=roundedWindowsB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Rounded buttons"),{
          bool roundedButtonsB=settings.roundedButtons;
          if (ImGui::Checkbox(_("Rounded buttons"),&roundedButtonsB)) {
            settings.roundedButtons=roundedButtonsB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Rounded menu corners"),{
          bool roundedMenusB=settings.roundedMenus;
          if (ImGui::Checkbox(_("Rounded menu corners"),&roundedMenusB)) {
            settings.roundedMenus=roundedMenusB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Rounded tabs"),{
          bool roundedTabsB=settings.roundedTabs;
          if (ImGui::Checkbox(_("Rounded tabs"),&roundedTabsB)) {
            settings.roundedTabs=roundedTabsB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Rounded scrollbars"),{
          bool roundedScrollbarsB=settings.roundedScrollbars;
          if (ImGui::Checkbox(_("Rounded scrollbars"),&roundedScrollbarsB)) {
            settings.roundedScrollbars=roundedScrollbarsB;
            SETTINGS_CHANGED;
          }
        }),
        SETTING(_("Borders around widgets"),{
          bool frameBordersB=settings.frameBorders;
          if (ImGui::Checkbox(_("Borders around widgets"),&frameBordersB)) {
            settings.frameBorders=frameBordersB;
            SETTINGS_CHANGED;
          }
        })
      }),
    },{}),
    SettingsCategory(_("Color"),{
      SettingsCategory(_("Interface"),{},{
        SETTING(_("Frame shading"),{
          if (ImGui::SliderInt(_("Frame shading"),&settings.guiColorsShading,0,100,"%d%%")) {
            if (settings.guiColorsShading<0) settings.guiColorsShading=0;
            if (settings.guiColorsShading>100) settings.guiColorsShading=100;
            applyUISettings(false);
            SETTINGS_CHANGED;
          }
        }),
        SETTING_COND(_("Color scheme type:"),{
          ImGui::Text(_("Color scheme type:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("Dark##gcb0"),settings.guiColorsBase==0)) {
            settings.guiColorsBase=0;
            applyUISettings(false);
            SETTINGS_CHANGED;
          }
          if (ImGui::RadioButton(_("Light##gcb1"),settings.guiColorsBase==1)) {
            settings.guiColorsBase=1;
            applyUISettings(false);
            SETTINGS_CHANGED;
          }
          ImGui::Unindent();
        },settings.basicColors),
        UI_COLOR_CONFIG_COND(GUI_COLOR_ACCENT_PRIMARY,_("Primary"),settings.basicColors),
        UI_COLOR_CONFIG_COND(GUI_COLOR_ACCENT_SECONDARY,_("Secondary"),settings.basicColors),
        UI_COLOR_CONFIG_COND(GUI_COLOR_BUTTON,_("Button"),!settings.basicColors),
        UI_COLOR_CONFIG_COND(GUI_COLOR_BUTTON_HOVER,_("Button (hovered)"),!settings.basicColors),
        UI_COLOR_CONFIG_COND(GUI_COLOR_BUTTON_ACTIVE,_("Button (active)"),!settings.basicColors),
        UI_COLOR_CONFIG_COND(GUI_COLOR_TAB,_("Tab"),!settings.basicColors),
        UI_COLOR_CONFIG_COND(GUI_COLOR_TAB_HOVER,_("Tab (hovered)"),!settings.basicColors),
        UI_COLOR_CONFIG_COND(GUI_COLOR_TAB_ACTIVE,_("Tab (active)"),!settings.basicColors),
        UI_COLOR_CONFIG_COND(GUI_COLOR_TAB_UNFOCUSED,_("Tab (unfocused)"),!settings.basicColors),
        UI_COLOR_CONFIG_COND(GUI_COLOR_TAB_UNFOCUSED_ACTIVE,_("Tab (unfocused and active)"),!settings.basicColors),
        UI_COLOR_CONFIG_COND(GUI_COLOR_IMGUI_HEADER,_("ImGui header"),!settings.basicColors),
        UI_COLOR_CONFIG_COND(GUI_COLOR_IMGUI_HEADER_HOVER,_("ImGui header (hovered)"),!settings.basicColors),
        UI_COLOR_CONFIG_COND(GUI_COLOR_IMGUI_HEADER_ACTIVE,_("ImGui header (active)"),!settings.basicColors),
        UI_COLOR_CONFIG_COND(GUI_COLOR_RESIZE_GRIP,_("Resize grip"),!settings.basicColors),
        UI_COLOR_CONFIG_COND(GUI_COLOR_RESIZE_GRIP_HOVER,_("Resize grip (hovered)"),!settings.basicColors),
        UI_COLOR_CONFIG_COND(GUI_COLOR_RESIZE_GRIP_ACTIVE,_("Resize grip (active)"),!settings.basicColors),
        UI_COLOR_CONFIG_COND(GUI_COLOR_WIDGET_BACKGROUND,_("Widget background"),!settings.basicColors),
        UI_COLOR_CONFIG_COND(GUI_COLOR_WIDGET_BACKGROUND_HOVER,_("Widget background (hovered)"),!settings.basicColors),
        UI_COLOR_CONFIG_COND(GUI_COLOR_WIDGET_BACKGROUND_ACTIVE,_("Widget background (active)"),!settings.basicColors),
        UI_COLOR_CONFIG_COND(GUI_COLOR_SLIDER_GRAB,_("Slider grab"),!settings.basicColors),
        UI_COLOR_CONFIG_COND(GUI_COLOR_SLIDER_GRAB_ACTIVE,_("Slider grab (active)"),!settings.basicColors),
        UI_COLOR_CONFIG_COND(GUI_COLOR_TITLE_BACKGROUND_ACTIVE,_("Title background (active)"),!settings.basicColors),
        UI_COLOR_CONFIG_COND(GUI_COLOR_CHECK_MARK,_("Checkbox/radio button mark"),!settings.basicColors),
        UI_COLOR_CONFIG_COND(GUI_COLOR_TEXT_SELECTION,_("Text selection"),!settings.basicColors),
        UI_COLOR_CONFIG_COND(GUI_COLOR_PLOT_LINES,_("Line plot"),!settings.basicColors),
        UI_COLOR_CONFIG_COND(GUI_COLOR_PLOT_LINES_HOVER,_("Line plot (hovered)"),!settings.basicColors),
        UI_COLOR_CONFIG_COND(GUI_COLOR_PLOT_HISTOGRAM,_("Histogram plot"),!settings.basicColors),
        UI_COLOR_CONFIG_COND(GUI_COLOR_PLOT_HISTOGRAM_HOVER,_("Histogram plot (hovered)"),!settings.basicColors),
        UI_COLOR_CONFIG_COND(GUI_COLOR_TABLE_ROW_EVEN,_("Table row (even)"),!settings.basicColors),
        UI_COLOR_CONFIG_COND(GUI_COLOR_TABLE_ROW_ODD,_("Table row (odd)"),!settings.basicColors),
      }),
      SettingsCategory(_("Interface (other)"),{},{
        UI_COLOR_CONFIG(GUI_COLOR_BACKGROUND,_("Background")),
        UI_COLOR_CONFIG(GUI_COLOR_FRAME_BACKGROUND,_("Window background")),
        UI_COLOR_CONFIG(GUI_COLOR_FRAME_BACKGROUND_CHILD,_("Sub-window background")),
        UI_COLOR_CONFIG(GUI_COLOR_FRAME_BACKGROUND_POPUP,_("Pop-up background")),
        UI_COLOR_CONFIG(GUI_COLOR_MODAL_BACKDROP,_("Modal backdrop")),
        UI_COLOR_CONFIG(GUI_COLOR_HEADER,_("Header")),
        UI_COLOR_CONFIG(GUI_COLOR_TEXT,_("Text")),
        UI_COLOR_CONFIG(GUI_COLOR_TEXT_DISABLED,_("Text (disabled)")),
        UI_COLOR_CONFIG(GUI_COLOR_TITLE_INACTIVE,_("Title bar (inactive)")),
        UI_COLOR_CONFIG(GUI_COLOR_TITLE_COLLAPSED,_("Title bar (collapsed)")),
        UI_COLOR_CONFIG(GUI_COLOR_MENU_BAR,_("Menu bar")),
        UI_COLOR_CONFIG(GUI_COLOR_BORDER,_("Border")),
        UI_COLOR_CONFIG(GUI_COLOR_BORDER_SHADOW,_("Border shadow")),
        UI_COLOR_CONFIG(GUI_COLOR_SCROLL,_("Scroll bar")),
        UI_COLOR_CONFIG(GUI_COLOR_SCROLL_HOVER,_("Scroll bar (hovered)")),
        UI_COLOR_CONFIG(GUI_COLOR_SCROLL_ACTIVE,_("Scroll bar (clicked)")),
        UI_COLOR_CONFIG(GUI_COLOR_SCROLL_BACKGROUND,_("Scroll bar background")),
        UI_COLOR_CONFIG(GUI_COLOR_SEPARATOR,_("Separator")),
        UI_COLOR_CONFIG(GUI_COLOR_SEPARATOR_HOVER,_("Separator (hover)")),
        UI_COLOR_CONFIG(GUI_COLOR_SEPARATOR_ACTIVE,_("Separator (active)")),
        UI_COLOR_CONFIG(GUI_COLOR_DOCKING_PREVIEW,_("Docking preview")),
        UI_COLOR_CONFIG(GUI_COLOR_DOCKING_EMPTY,_("Docking empty")),
        UI_COLOR_CONFIG(GUI_COLOR_TABLE_HEADER,_("Table header")),
        UI_COLOR_CONFIG(GUI_COLOR_TABLE_BORDER_HARD,_("Table border (hard)")),
        UI_COLOR_CONFIG(GUI_COLOR_TABLE_BORDER_SOFT,_("Table border (soft)")),
        UI_COLOR_CONFIG(GUI_COLOR_DRAG_DROP_TARGET,_("Drag and drop target")),
        UI_COLOR_CONFIG(GUI_COLOR_NAV_WIN_HIGHLIGHT,_("Window switcher (highlight)")),
        UI_COLOR_CONFIG(GUI_COLOR_NAV_WIN_BACKDROP,_("Window switcher backdrop")),
      }),
      SettingsCategory(_("Miscellaneous"),{},{
        UI_COLOR_CONFIG(GUI_COLOR_TOGGLE_ON,_("Toggle on")),
        UI_COLOR_CONFIG(GUI_COLOR_TOGGLE_OFF,_("Toggle off")),
        UI_COLOR_CONFIG(GUI_COLOR_PLAYBACK_STAT,_("Playback status")),
        UI_COLOR_CONFIG(GUI_COLOR_DESTRUCTIVE,_("Destructive hint")),
        UI_COLOR_CONFIG(GUI_COLOR_WARNING,_("Warning hint")),
        UI_COLOR_CONFIG(GUI_COLOR_ERROR,_("Error hint")),
      }),
      SettingsCategory(_("File Picker (built-in)"),{},{
        UI_COLOR_CONFIG(GUI_COLOR_FILE_DIR,_("Directory")),
        UI_COLOR_CONFIG(GUI_COLOR_FILE_SONG_NATIVE,_("Song (native)")),
        UI_COLOR_CONFIG(GUI_COLOR_FILE_SONG_IMPORT,_("Song (import)")),
        UI_COLOR_CONFIG(GUI_COLOR_FILE_INSTR,_("Instrument")),
        UI_COLOR_CONFIG(GUI_COLOR_FILE_AUDIO,_("Audio")),
        UI_COLOR_CONFIG(GUI_COLOR_FILE_WAVE,_("Wavetable")),
        UI_COLOR_CONFIG(GUI_COLOR_FILE_VGM,_("VGM")),
        UI_COLOR_CONFIG(GUI_COLOR_FILE_ZSM,_("ZSM")),
        UI_COLOR_CONFIG(GUI_COLOR_FILE_FONT,_("Font")),
        UI_COLOR_CONFIG(GUI_COLOR_FILE_OTHER,_("Other")),
      }),
      SettingsCategory(_("Oscilloscope"),{
        SettingsCategory(_("Wave (non-mono)"),{},{
          UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE_CH0,_("Waveform (1)")),
          UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE_CH1,_("Waveform (2)")),
          UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE_CH2,_("Waveform (3)")),
          UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE_CH3,_("Waveform (4)")),
          UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE_CH4,_("Waveform (5)")),
          UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE_CH5,_("Waveform (6)")),
          UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE_CH6,_("Waveform (7)")),
          UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE_CH7,_("Waveform (8)")),
          UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE_CH8,_("Waveform (9)")),
          UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE_CH9,_("Waveform (10)")),
          UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE_CH10,_("Waveform (11)")),
          UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE_CH11,_("Waveform (12)")),
          UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE_CH12,_("Waveform (13)")),
          UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE_CH13,_("Waveform (14)")),
          UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE_CH14,_("Waveform (15)")),
          UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE_CH15,_("Waveform (16)")),
        })
      },{
        UI_COLOR_CONFIG(GUI_COLOR_OSC_BORDER,_("Border")),
        UI_COLOR_CONFIG(GUI_COLOR_OSC_BG1,_("Background (top-left)")),
        UI_COLOR_CONFIG(GUI_COLOR_OSC_BG2,_("Background (top-right)")),
        UI_COLOR_CONFIG(GUI_COLOR_OSC_BG3,_("Background (bottom-left)")),
        UI_COLOR_CONFIG(GUI_COLOR_OSC_BG4,_("Background (bottom-right)")),
        UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE,_("Waveform")),
        UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE_PEAK,_("Waveform (clip)")),
        UI_COLOR_CONFIG(GUI_COLOR_OSC_REF,_("Reference")),
        UI_COLOR_CONFIG(GUI_COLOR_OSC_GUIDE,_("Guide")),
      }),
      SettingsCategory(_("Volume Meter"),{},{
        UI_COLOR_CONFIG(GUI_COLOR_VOLMETER_LOW,_("Low")),
        UI_COLOR_CONFIG(GUI_COLOR_VOLMETER_HIGH,_("High")),
        UI_COLOR_CONFIG(GUI_COLOR_VOLMETER_PEAK,_("Clip")),
      }),
      SettingsCategory(_("Orders"),{},{
        UI_COLOR_CONFIG(GUI_COLOR_ORDER_ROW_INDEX,_("Order number")),
        UI_COLOR_CONFIG(GUI_COLOR_ORDER_ACTIVE,_("Playing order background")),
        UI_COLOR_CONFIG(GUI_COLOR_SONG_LOOP,_("Song loop")),
        UI_COLOR_CONFIG(GUI_COLOR_ORDER_SELECTED,_("Selected order")),
        UI_COLOR_CONFIG(GUI_COLOR_ORDER_SIMILAR,_("Similar patterns")),
        UI_COLOR_CONFIG(GUI_COLOR_ORDER_INACTIVE,_("Inactive patterns")),
      }),
      SettingsCategory(_("Envelope View"),{},{
        UI_COLOR_CONFIG(GUI_COLOR_FM_ENVELOPE,_("Envelope")),
        UI_COLOR_CONFIG(GUI_COLOR_FM_ENVELOPE_SUS_GUIDE,_("Sustain guide")),
        UI_COLOR_CONFIG(GUI_COLOR_FM_ENVELOPE_RELEASE,_("Release")),
      }),
      SettingsCategory(_("FM Editor"),{},{
        UI_COLOR_CONFIG(GUI_COLOR_FM_ALG_BG,_("Algorithm background")),
        UI_COLOR_CONFIG(GUI_COLOR_FM_ALG_LINE,_("Algorithm lines")),
        UI_COLOR_CONFIG(GUI_COLOR_FM_MOD,_("Modulator")),
        UI_COLOR_CONFIG(GUI_COLOR_FM_CAR,_("Carrier")),

        UI_COLOR_CONFIG(GUI_COLOR_FM_SSG,_("SSG-EG")),
        UI_COLOR_CONFIG(GUI_COLOR_FM_WAVE,_("Waveform")),
        Setting(NULL,[]{ImGui::TextWrapped(_("(the following colors only apply when \"Use separate colors for carriers/modulators in FM editor\" is on!)"));}),
        UI_COLOR_CONFIG(GUI_COLOR_FM_PRIMARY_MOD,_("Mod. accent (primary)")),
        UI_COLOR_CONFIG(GUI_COLOR_FM_SECONDARY_MOD,_("Mod. accent (secondary)")),
        UI_COLOR_CONFIG(GUI_COLOR_FM_BORDER_MOD,_("Mod. border")),
        UI_COLOR_CONFIG(GUI_COLOR_FM_BORDER_SHADOW_MOD,_("Mod. border shadow")),

        UI_COLOR_CONFIG(GUI_COLOR_FM_PRIMARY_CAR,_("Car. accent (primary)")),
        UI_COLOR_CONFIG(GUI_COLOR_FM_SECONDARY_CAR,_("Car. accent (secondary)")),
        UI_COLOR_CONFIG(GUI_COLOR_FM_BORDER_CAR,_("Car. border")),
        UI_COLOR_CONFIG(GUI_COLOR_FM_BORDER_SHADOW_CAR,_("Car. border shadow")),
      }),
      SettingsCategory(_("Macro Editor"),{},{
        UI_COLOR_CONFIG(GUI_COLOR_MACRO_VOLUME,_("Volume")),
        UI_COLOR_CONFIG(GUI_COLOR_MACRO_PITCH,_("Pitch")),
        UI_COLOR_CONFIG(GUI_COLOR_MACRO_WAVE,_("Wave")),
        UI_COLOR_CONFIG(GUI_COLOR_MACRO_NOISE,_("Noise")),
        UI_COLOR_CONFIG(GUI_COLOR_MACRO_FILTER,_("Filter")),
        UI_COLOR_CONFIG(GUI_COLOR_MACRO_ENVELOPE,_("Envelope")),
        UI_COLOR_CONFIG(GUI_COLOR_MACRO_GLOBAL,_("Global Parameter")),
        UI_COLOR_CONFIG(GUI_COLOR_MACRO_OTHER,_("Other")),
        UI_COLOR_CONFIG(GUI_COLOR_MACRO_HIGHLIGHT,_("Step Highlight")),
      }),
      SettingsCategory(_("Instrument Types"),{},{
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_FM,_("FM (OPN)")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_STD,_("SN76489/Sega PSG")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_T6W28,_("T6W28")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_GB,_("Game Boy")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_C64,_("C64")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_AMIGA,_("Amiga/Generic Sample")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_PCE,_("PC Engine")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_AY,_("AY-3-8910/SSG")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_AY8930,_("AY8930")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_TIA,_("TIA")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_SAA1099,_("SAA1099")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_VIC,_("VIC")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_PET,_("PET")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_VRC6,_("VRC6")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_VRC6_SAW,_("VRC6 (saw)")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_OPLL,_("FM (OPLL)")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_OPL,_("FM (OPL)")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_FDS,_("FDS")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_VBOY,_("Virtual Boy")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_N163,_("Namco 163")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_SCC,_("Konami SCC")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_OPZ,_("FM (OPZ)")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_POKEY,_("POKEY")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_BEEPER,_("PC Beeper")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_SWAN,_("WonderSwan")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_MIKEY,_("Lynx")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_VERA,_("VERA")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_X1_010,_("X1-010")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_ES5506,_("ES5506")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_MULTIPCM,_("MultiPCM")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_SNES,_("SNES")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_SU,_("Sound Unit")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_NAMCO,_("Namco WSG")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_OPL_DRUMS,_("FM (OPL Drums)")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_OPM,_("FM (OPM)")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_NES,_("NES")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_MSM6258,_("MSM6258")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_MSM6295,_("MSM6295")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_ADPCMA,_("ADPCM-A")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_ADPCMB,_("ADPCM-B")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_SEGAPCM,_("Sega PCM")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_QSOUND,_("QSound")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_YMZ280B,_("YMZ280B")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_RF5C68,_("RF5C68")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_MSM5232,_("MSM5232")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_K007232,_("K007232")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_GA20,_("GA20")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_POKEMINI,_("Pokémon Mini")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_SM8521,_("SM8521")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_PV1000,_("PV-1000")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_K053260,_("K053260")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_TED,_("TED")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_C140,_("C140")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_C219,_("C219")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_ESFM,_("ESFM")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_POWERNOISE,_("PowerNoise (noise)")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_POWERNOISE_SLOPE,_("PowerNoise (slope)")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_DAVE,_("Dave")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_NDS,_("Nintendo DS")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_GBA_DMA,_("GBA DMA")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_GBA_MINMOD,_("GBA MinMod")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_BIFURCATOR,_("Bifurcator")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_SID2,_("SID2")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_SUPERVISION,_("Supervision")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_UPD1771C,_("μPD1771C")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_SID3,_("SID3")),
        UI_COLOR_CONFIG(GUI_COLOR_INSTR_UNKNOWN,_("Other/Unknown")),
      }),
      SettingsCategory(_("Channel"),{},{
        UI_COLOR_CONFIG(GUI_COLOR_CHANNEL_BG,_("Single color (background)")),
        UI_COLOR_CONFIG(GUI_COLOR_CHANNEL_FG,_("Single color (text)")),
        UI_COLOR_CONFIG(GUI_COLOR_CHANNEL_FM,_("FM")),
        UI_COLOR_CONFIG(GUI_COLOR_CHANNEL_PULSE,_("Pulse")),
        UI_COLOR_CONFIG(GUI_COLOR_CHANNEL_NOISE,_("Noise")),
        UI_COLOR_CONFIG(GUI_COLOR_CHANNEL_PCM,_("PCM")),
        UI_COLOR_CONFIG(GUI_COLOR_CHANNEL_WAVE,_("Wave")),
        UI_COLOR_CONFIG(GUI_COLOR_CHANNEL_OP,_("FM operator")),
        UI_COLOR_CONFIG(GUI_COLOR_CHANNEL_MUTED,_("Muted")),
      }),
      SettingsCategory(_("Pattern"),{},{
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_PLAY_HEAD,_("Playhead")),
        UI_COLOR_CONFIG(GUI_COLOR_EDITING,_("Editing")),
        UI_COLOR_CONFIG(GUI_COLOR_EDITING_CLONE,_("Editing (will clone)")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_CURSOR,_("Cursor")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_CURSOR_HOVER,_("Cursor (hovered)")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_CURSOR_ACTIVE,_("Cursor (clicked)")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_SELECTION,_("Selection")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_SELECTION_HOVER,_("Selection (hovered)")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_SELECTION_ACTIVE,_("Selection (clicked)")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_HI_1,_("Highlight 1")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_HI_2,_("Highlight 2")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_ROW_INDEX,_("Row number")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_ROW_INDEX_HI1,_("Row number (highlight 1)")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_ROW_INDEX_HI2,_("Row number (highlight 2)")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_ACTIVE,_("Note")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_ACTIVE_HI1,_("Note (highlight 1)")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_ACTIVE_HI2,_("Note (highlight 2)")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_INACTIVE,_("Blank")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_INACTIVE_HI1,_("Blank (highlight 1)")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_INACTIVE_HI2,_("Blank (highlight 2)")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_INS,_("Instrument")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_INS_WARN,_("Instrument (invalid type)")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_INS_ERROR,_("Instrument (out of range)")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_VOLUME_MIN,_("Volume (0%)")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_VOLUME_HALF,_("Volume (50%)")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_VOLUME_MAX,_("Volume (100%)")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_EFFECT_INVALID,_("Invalid effect")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_EFFECT_PITCH,_("Pitch effect")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_EFFECT_VOLUME,_("Volume effect")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_EFFECT_PANNING,_("Panning effect")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_EFFECT_SONG,_("Song effect")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_EFFECT_TIME,_("Time effect")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_EFFECT_SPEED,_("Speed effect")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_EFFECT_SYS_PRIMARY,_("Primary specific effect")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_EFFECT_SYS_SECONDARY,_("Secondary specific effect")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_EFFECT_MISC,_("Miscellaneous")),
        UI_COLOR_CONFIG(GUI_COLOR_EE_VALUE,_("External command output")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_OFF,_("Status: off/disabled")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_REL,_("Status: off + macro rel")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_REL_ON,_("Status: on + macro rel")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_ON,_("Status: on")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_VOLUME,_("Status: volume")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_PITCH,_("Status: pitch")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_PANNING,_("Status: panning")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_SYS1,_("Status: chip (primary)")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_SYS2,_("Status: chip (secondary)")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_MIXING,_("Status: mixing")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_DSP,_("Status: DSP effect")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_NOTE,_("Status: note altering")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_MISC1,_("Status: misc color 1")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_MISC2,_("Status: misc color 2")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_MISC3,_("Status: misc color 3")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_ATTACK,_("Status: attack")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_DECAY,_("Status: decay")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_SUSTAIN,_("Status: sustain")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_RELEASE,_("Status: release")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_DEC_LINEAR,_("Status: decrease linear")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_DEC_EXP,_("Status: decrease exp")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_INC,_("Status: increase")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_BENT,_("Status: bent")),
        UI_COLOR_CONFIG(GUI_COLOR_PATTERN_STATUS_DIRECT,_("Status: direct")),
      }),
      SettingsCategory(_("Sample Editor"),{},{
        UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_BG,_("Background")),
        UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_FG,_("Waveform")),
        UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_TIME_BG,_("Time background")),
        UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_TIME_FG,_("Time text")),
        UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_LOOP,_("Loop region")),
        UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_CENTER,_("Center guide")),
        UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_GRID,_("Grid")),
        UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_SEL,_("Selection")),
        UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_SEL_POINT,_("Selection points")),
        UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_NEEDLE,_("Preview needle")),
        UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_NEEDLE_PLAYING,_("Playing needles")),
        UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_LOOP_POINT,_("Loop markers")),
        UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_CHIP_DISABLED,_("Chip select: disabled")),
        UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_CHIP_ENABLED,_("Chip select: enabled")),
        UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_CHIP_WARNING,_("Chip select: enabled (failure)")),
      }),
      SettingsCategory(_("Pattern Manager"),{},{
        UI_COLOR_CONFIG(GUI_COLOR_PAT_MANAGER_NULL,_("Unallocated")),
        UI_COLOR_CONFIG(GUI_COLOR_PAT_MANAGER_UNUSED,_("Unused")),
        UI_COLOR_CONFIG(GUI_COLOR_PAT_MANAGER_USED,_("Used")),
        UI_COLOR_CONFIG(GUI_COLOR_PAT_MANAGER_OVERUSED,_("Overused")),
        UI_COLOR_CONFIG(GUI_COLOR_PAT_MANAGER_EXTREMELY_OVERUSED,_("Really overused")),
        UI_COLOR_CONFIG(GUI_COLOR_PAT_MANAGER_COMBO_BREAKER,_("Combo Breaker")),
      }),
      SettingsCategory(_("Piano"),{},{
        UI_COLOR_CONFIG(GUI_COLOR_PIANO_BACKGROUND,_("Background")),
        UI_COLOR_CONFIG(GUI_COLOR_PIANO_KEY_TOP,_("Upper key")),
        UI_COLOR_CONFIG(GUI_COLOR_PIANO_KEY_TOP_HIT,_("Upper key (feedback)")),
        UI_COLOR_CONFIG(GUI_COLOR_PIANO_KEY_TOP_ACTIVE,_("Upper key (pressed)")),
        UI_COLOR_CONFIG(GUI_COLOR_PIANO_KEY_BOTTOM,_("Lower key")),
        UI_COLOR_CONFIG(GUI_COLOR_PIANO_KEY_BOTTOM_HIT,_("Lower key (feedback)")),
        UI_COLOR_CONFIG(GUI_COLOR_PIANO_KEY_BOTTOM_ACTIVE,_("Lower key (pressed)")),
      }),
      SettingsCategory(_("Clock"),{},{
        UI_COLOR_CONFIG(GUI_COLOR_CLOCK_TEXT,_("Clock text")),
        UI_COLOR_CONFIG(GUI_COLOR_CLOCK_BEAT_LOW,_("Beat (off)")),
        UI_COLOR_CONFIG(GUI_COLOR_CLOCK_BEAT_HIGH,_("Beat (on)")),
      }),
      SettingsCategory(_("Patchbay"),{},{
        UI_COLOR_CONFIG(GUI_COLOR_PATCHBAY_PORTSET,_("PortSet")),
        UI_COLOR_CONFIG(GUI_COLOR_PATCHBAY_PORT,_("Port")),
        UI_COLOR_CONFIG(GUI_COLOR_PATCHBAY_PORT_HIDDEN,_("Port (hidden/unavailable)")),
        UI_COLOR_CONFIG(GUI_COLOR_PATCHBAY_CONNECTION,_("Connection (selected)")),
        UI_COLOR_CONFIG(GUI_COLOR_PATCHBAY_CONNECTION_BG,_("Connection (other)")),
      }),
      SettingsCategory(_("Memory Composition"),{},{
        UI_COLOR_CONFIG(GUI_COLOR_MEMORY_BG,_("Background")),
        UI_COLOR_CONFIG(GUI_COLOR_MEMORY_DATA,_("Waveform data")),
        UI_COLOR_CONFIG(GUI_COLOR_MEMORY_FREE,_("Unknown")),
        //UI_COLOR_CONFIG(GUI_COLOR_MEMORY_PADDING,_("")),
        UI_COLOR_CONFIG(GUI_COLOR_MEMORY_RESERVED,_("Reserved")),
        UI_COLOR_CONFIG(GUI_COLOR_MEMORY_SAMPLE,_("Sample")),
        UI_COLOR_CONFIG(GUI_COLOR_MEMORY_SAMPLE_ALT1,_("Sample (alternate 1)")),
        UI_COLOR_CONFIG(GUI_COLOR_MEMORY_SAMPLE_ALT2,_("Sample (alternate 2)")),
        UI_COLOR_CONFIG(GUI_COLOR_MEMORY_SAMPLE_ALT3,_("Sample (alternate 3)")),
        UI_COLOR_CONFIG(GUI_COLOR_MEMORY_WAVE_RAM,_("Wave RAM")),
        UI_COLOR_CONFIG(GUI_COLOR_MEMORY_WAVE_STATIC,_("Wavetable (static)")),
        UI_COLOR_CONFIG(GUI_COLOR_MEMORY_ECHO,_("Echo buffer")),
        UI_COLOR_CONFIG(GUI_COLOR_MEMORY_N163_LOAD,_("Namco 163 load pos")),
        UI_COLOR_CONFIG(GUI_COLOR_MEMORY_N163_PLAY,_("Namco 163 play pos")),
        UI_COLOR_CONFIG(GUI_COLOR_MEMORY_BANK0,_("Sample (bank 0)")),
        UI_COLOR_CONFIG(GUI_COLOR_MEMORY_BANK1,_("Sample (bank 1)")),
        UI_COLOR_CONFIG(GUI_COLOR_MEMORY_BANK2,_("Sample (bank 2)")),
        UI_COLOR_CONFIG(GUI_COLOR_MEMORY_BANK3,_("Sample (bank 3)")),
        UI_COLOR_CONFIG(GUI_COLOR_MEMORY_BANK4,_("Sample (bank 4)")),
        UI_COLOR_CONFIG(GUI_COLOR_MEMORY_BANK5,_("Sample (bank 5)")),
        UI_COLOR_CONFIG(GUI_COLOR_MEMORY_BANK6,_("Sample (bank 6)")),
        UI_COLOR_CONFIG(GUI_COLOR_MEMORY_BANK7,_("Sample (bank 7)")),
      }),
      SettingsCategory(_("Log Viewer"),{},{
        UI_COLOR_CONFIG(GUI_COLOR_LOGLEVEL_ERROR,_("Log level: Error")),
        UI_COLOR_CONFIG(GUI_COLOR_LOGLEVEL_WARNING,_("Log level: Warning")),
        UI_COLOR_CONFIG(GUI_COLOR_LOGLEVEL_INFO,_("Log level: Info")),
        UI_COLOR_CONFIG(GUI_COLOR_LOGLEVEL_DEBUG,_("Log level: Debug")),
        UI_COLOR_CONFIG(GUI_COLOR_LOGLEVEL_TRACE,_("Log level: Trace/Verbose")),
      })
    },{
      SETTING(_("Color scheme"),{
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
      }),
      SETTING(_("Guru mode"),{
        bool basicColorsB=!settings.basicColors;
        if (ImGui::Checkbox(_("Guru mode"),&basicColorsB)) {
          settings.basicColors=!basicColorsB;
          applyUISettings(false);
          SETTINGS_CHANGED;
        }
      })
    }),
    SettingsCategory(_("Backup"),{ // FIXME: backups table in search results is very very big
      SettingsCategory(_("Backup Management"),{},{
        Setting(_("Backup Management"),[this]{
          purgeDateChanged=false;

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
        })
      })
    },{
      SETTING(_("Enable backup system"),{
        bool backupEnableB=settings.backupEnable;
        if (ImGui::Checkbox(_("Enable backup system"),&backupEnableB)) {
          settings.backupEnable=backupEnableB;
          SETTINGS_CHANGED;
        }
      }),
      SETTING(_("Interval (in seconds)"),{
        if (ImGui::InputInt(_("Interval (in seconds)"),&settings.backupInterval)) {
          if (settings.backupInterval<10) settings.backupInterval=10;
          if (settings.backupInterval>86400) settings.backupInterval=86400;
        }
      }),
      SETTING(_("Backups per file"),{
        if (ImGui::InputInt(_("Backups per file"),&settings.backupMaxCopies)) {
          if (settings.backupMaxCopies<1) settings.backupMaxCopies=1;
          if (settings.backupMaxCopies>100) settings.backupMaxCopies=100;
        }
      })
    })
  };

  settings.activeCategory=settings.categories[0];
}
