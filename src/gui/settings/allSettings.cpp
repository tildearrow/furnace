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

#include "../newSettings.h"
#include "misc/cpp/imgui_stdlib.h"
#include "IconsFontAwesome4.h"
#include "../gui.h"
#include "../guiConst.h"
#include "../util.h"
#include "../intConst.h"

static const char* valueInputStyles[]={
  _N("Disabled/custom"),
  _N("Two octaves (0 is C-4, F is D#5)"),
  _N("Raw (note number is value)"),
  _N("Two octaves alternate (lower keys are 0-9, upper keys are A-F)"),
  _N("Use dual control change (one for each nibble)"),
  _N("Use 14-bit control change"),
  _N("Use single control change (imprecise)")
};

static const char* valueSInputStyles[]={
  _N("Disabled/custom"),
  _N("Use dual control change (one for each nibble)"),
  _N("Use 14-bit control change"),
  _N("Use single control change (imprecise)")
};

static const char* messageTypes[]={
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

static const char* messageChannels[]={
  "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", _N("Any")
};

static const char* specificControls[18]={
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


static const char* arcadeCores[]={
  "ymfm",
  "Nuked-OPM"
};

static const char* ym2612Cores[]={
  "Nuked-OPN2",
  "ymfm",
  "YMF276-LLE"
};

static const char* snCores[]={
  "MAME",
  "Nuked-PSG Mod"
};

static const char* nesCores[]={
  "puNES",
  "NSFplay"
};

static const char* c64Cores[]={
  "reSID",
  "reSIDfp",
  "dSID"
};

static const char* pokeyCores[]={
  "Atari800 (mzpokeysnd)",
  _N("ASAP (C++ port)")
};

static const char* opnCores[]={
  "ymfm",
  "Nuked-OPN2 (FM) + ymfm (SSG/ADPCM)",
  "YM2608-LLE"
};

static const char* opl2Cores[]={
  "Nuked-OPL3",
  "ymfm",
  "YM3812-LLE"
};

static const char* opl3Cores[]={
  "Nuked-OPL3",
  "ymfm",
  "YMF262-LLE"
};

static const char* opl4Cores[]={
  "Nuked-OPL3 (FM) + openMSX (PCM)",
  "ymfm"
};

static const char* esfmCores[]={
  "ESFMu",
  _N("ESFMu (fast)")
};

static const char* opllCores[]={
  "Nuked-OPLL",
  "emu2413"
};

static const char* ayCores[]={
  "MAME",
  "AtomicSSG"
};

static const char* coreQualities[]={
  _N("Lower"),
  _N("Low"),
  _N("Medium"),
  _N("High"),
  _N("Ultra"),
  _N("Ultimate")
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

#define CATEGORY_BEGIN(_name) allSettings.push_back(SettingsCategory(_name,
#define CATEGORY_END ));
#define SUBCATEGORY SettingsCategory

#define SETTING_CHECKBOX(_label,_value) SettingEntry::Checkbox(_label,#_value,&settings._value)
#define SETTING_INV_CHECKBOX(_label,_value) SettingEntry::Checkbox(_label,#_value,&settings._value,true)
#define SETTING_COLOR(_which) \
  SettingEntry::Color(guiColors[_which].friendlyName,NULL,&uiColors[_which])
#define SETTING_KEYBIND(_which) \
  SettingEntry::Keybind(guiActions[_which].friendlyName,NULL,&actionKeys[_which],_which)

/*
  HOW TO ADD TO SETTINGS
  settings consist of SettingEntry-s inside SettingsCategory-s.
  - categories start with CATEGORY_BEGIN and end with CATEGORY_END
    - this is done to comply with MSVC (MSVC doesn't like preprocessor directives inside macro expansions)
  - subcategories use the SUBCATEGORY macro
    a typical category definition looks like
      CATEGORY_BEGIN(_N("Category")) {
        SettingEntry::...(...),
        SettingEntry::...(...),
        SettingEntry::...(...),
      },{
        SUBCATEGORY(_N("Subcategory"),{
          SettingEntry::...(...),
          SettingEntry::...(...),
          SettingEntry::...(...),
        }),
      }
      CATEGORY_END,
    - if localized, (sub)category labels should be inside the _N() macro (not _() !)
    - subcategories can contain their own subcategories
  - use the static definition functions for common types.
   a setting definition usually looks like
      SettingEntry::Setting(
        _N("Setting"),
        "setting", &settings.setting,
        ...
      )
    - if localized, labels should be in the _N() macro (not _() !)
      - using _() is okay inside custom draw functions
    - for checkboxes, you can use the SETTING_CHECKBOX or SETTING_INV_CHECKBOX macros
    - for colors, you can use the SETTING_COLOR macro
    - for keybinds, you can use the SETTING_KEYBIND macro
    - typical setting types look like:
      SETTING_CHECKBOX(
        _N("Checkbox"),
        checkbox // will expand to "checkbox",&settings.checkbox
      ),
      SettingEntry::Radio(
        _N("Radio"),
        "radio",&settings.radio,
        {
          {_N("Option 1"),0,_N("this option has a tooltip!")},
          {_N("Option 2"),1},
          {_N("Option 3"),2},
        }
      ),
      SettingEntry::ComboInt(
        _N("Intger Combo"),
        "comboInt",&settings.comboInt,
        {
          {_N("Option 1"),0,_N("this option has a tooltip!")},
          {_N("Option 2"),1},
          {_N("Option 3"),2},
        }
      ),
      SettingEntry::ComboString(
        _N("String Combo"),
        "comboStr",&settings.comboStr,
        {
          {_N("Option 1"),"Option 1",_N("this option has a tooltip!")},
          {_N("Option 2"),"Option 2"},
          {_N("Option 3"),"Option 3"},
        }
      ),
      SettingEntry::SliderFloat(
        _N("Float slider"),
        "sliderFloat",&settings.sliderFloat,
        {
          -1.0f,1.0f, // slider range
          "%g"        // optional format string
        }
      ),
      SettingEntry::SliderInt(
        _N("Integer slider"),
        "sliderInt",&settings.sliderInt,
        {
          0,100, // slider range
          "%d%%" // optional format string
        }
      ),
      SettingEntry::InputInt(
        _N("integer input"),
        "inputInt",&settings.inputInt,
        {
          0,256,  // input range
          1,5,    // optional input steps - slow (regular click) and fast (ctrl+click)
          "%d px" // optional format string
        }
      ),
      SettingEntry::InputText(
        _N("Text input"),
        "inputText",&settings.inputText,
        "text here..." // optional hint
      ),
      SettingEntry::Path(
        _N("File path"),
        "filePath",&settings.filePath,
        GUI_FILE_OPEN,        // file dialog ID
        "/home/user/song.fur" // optional hint
      ),
      SETTING_COLOR(GUI_COLOR_ACCENT_PRIMARY), // color ID
      SETTING_KEYBIND(GUI_ACTION_NEW) // action ID
    - custom settings look like
      SettingEntry(
        _N("Custom Setting"), // note that this label will only be used for search
        "customSetting",      // usually NULL
        [this] {
          bool ret=false;
          if (...) {
            ...
            ret=true;
          }
          return ret;
        }
      ),
  - use Tooltip() to add a tooltip to the setting (text that will be visible when hovering on the setting):
    SettingEntry::...(
      ...
    ).Tooltip(_N("this setting does ...")),
  - use Condition() to add a condition to the setting (when the setting will be drawn):
    SettingEntry::...(
      ...
    ).Condition([this]{return someNumber==5}), // setting will be drawn when someNumber is equal to 5
  - use Callback() to run code when the setting value changes:
    SettingEntry::...(
      ...
    ).Callback([this]{updateSomething();})
*/

void FurnaceGUI::initSettings() {
  CATEGORY_BEGIN(_N("General")) {},{
    SUBCATEGORY(_N("Program"),{
      // the codes are somewhere else...
      SettingEntry(_N("Cheat Codes"),NULL,[this]{
        ImGui::Text(_("Enter code:"));
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
          if (checker==0x78444162 && checker1==0x2306) {
            mmlString[30]=_("$2500 withdrawn successfully!");
            for (int i=0; i<e->getTotalChannelCount(); i++) {
              for (int j=0; j<DIV_MAX_PATTERNS; j++) {
                if (e->curSubSong->pat[i].data[j]!=NULL) {
                  DivPattern* p=e->curSubSong->pat[i].data[j];
                  for (int k=0; k<DIV_MAX_ROWS; k++) {
                    if (p->newData[k][DIV_PAT_NOTE]>=0 && p->newData[k][DIV_PAT_NOTE]<180) {
                      int newNote=((6+p->newData[k][DIV_PAT_NOTE])/12)*12;
                      p->newData[k][DIV_PAT_NOTE]=CLAMP(newNote,0,168);
                    }
                  }
                }
              }
            }
          }
          if (checker==0x94222d83 && checker1==0x6600) {
            mmlString[30]=_("enabled \"comfortable\" mode");
            ImGuiStyle& sty=ImGui::GetStyle();
            sty.FramePadding=ImVec2(20.0f*dpiScale,20.0f*dpiScale);
            sty.ItemSpacing=ImVec2(10.0f*dpiScale,10.0f*dpiScale);
            sty.ItemInnerSpacing=ImVec2(10.0f*dpiScale,10.0f*dpiScale);
            settingsOpen=false;
          }
          if ((checker==0x2222225c && checker1==0x2d2) ||
              (checker==0x4444447e && checker1==0x146)) {
            mmlString[30]=_("Oh my god... Kill me now so I don't have to go through that again!");
            for (int i=0; i<e->getTotalChannelCount(); i++) {
              for (int j=0; j<DIV_MAX_PATTERNS; j++) {
                if (e->curSubSong->pat[i].data[j]!=NULL) {
                  DivPattern* p=e->curSubSong->pat[i].data[j];
                  for (int k=0; k<DIV_MAX_ROWS; k++) {
                    if (p->newData[k][DIV_PAT_NOTE]>=0 && p->newData[k][DIV_PAT_NOTE]<180) {
                      int newNote=p->newData[k][DIV_PAT_NOTE]+(rand()%40)-18;
                      p->newData[k][DIV_PAT_NOTE]=CLAMP(newNote,60,179);
                    }
                  }
                }
              }
            }
          }

          mmlString[31]="";
        }
        ImGui::Text("%s",mmlString[30].c_str());
        return false;
      }).Condition([this] {
        return nonLatchNibble;
      }),
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
        if (ImGui::TreeNode(_("Per-column control change"))) {
          for (int i=0; i<18; i++) {
            ImGui::PushID(i);
            if (ImGui::Combo(specificControls[i],&midiMap.valueInputSpecificStyle[i],LocalizedComboGetter,valueSInputStyles,4)) ret=true;
            if (midiMap.valueInputSpecificStyle[i]>0) {
              ImGui::Indent();
              if (midiMap.valueInputSpecificStyle[i]==3) {
                if (ImGui::InputInt(_("Control##valueCCS"),&midiMap.valueInputSpecificSingle[i],1,16)) {
                  if (midiMap.valueInputSpecificSingle[i]<0) midiMap.valueInputSpecificSingle[i]=0;
                  if (midiMap.valueInputSpecificSingle[i]>127) midiMap.valueInputSpecificSingle[i]=127;
                  ret=true;
                }
              } else {
                if (ImGui::InputInt((midiMap.valueInputSpecificStyle[i]==4)?_("CC of upper nibble##valueCC1"):_("MSB CC##valueCC1"),&midiMap.valueInputSpecificMSB[i],1,16)) {
                  if (midiMap.valueInputSpecificMSB[i]<0) midiMap.valueInputSpecificMSB[i]=0;
                  if (midiMap.valueInputSpecificMSB[i]>127) midiMap.valueInputSpecificMSB[i]=127;
                  ret=true;
                }
                if (ImGui::InputInt((midiMap.valueInputSpecificStyle[i]==4)?_("CC of lower nibble##valueCC2"):_("LSB CC##valueCC2"),&midiMap.valueInputSpecificLSB[i],1,16)) {
                  if (midiMap.valueInputSpecificLSB[i]<0) midiMap.valueInputSpecificLSB[i]=0;
                  if (midiMap.valueInputSpecificLSB[i]>127) midiMap.valueInputSpecificLSB[i]=127;
                  ret=true;
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
          ret=true;
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
          ret=true;
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_EXTERNAL_LINK "##AddLearnAction")) {
          midiMap.binds.push_back(MIDIBind());
          learning=midiMap.binds.size()-1;
          ret=true;
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
                  ret=true;
                }
              }
              ImGui::EndCombo();
            }

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::BeginCombo("##BChannel",messageChannels[bind.channel])) {
              if (ImGui::Selectable(messageChannels[16],bind.channel==16)) {
                bind.channel=16;
                ret=true;
              }
              for (int j=0; j<16; j++) {
                if (ImGui::Selectable(messageChannels[j],bind.channel==j)) {
                  bind.channel=j;
                  ret=true;
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
                ret=true;
              }
              for (int j=0; j<128; j++) {
                const char* nName="???";
                if ((j+60)>0 && (j+60)<180) {
                  nName=noteNames[j+60];
                }
                snprintf(bindID,1024,"%d (0x%.2X, %s)##BV1_%d",j,j,nName,j);
                if (ImGui::Selectable(bindID,bind.data1==j)) {
                  bind.data1=j;
                  ret=true;
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
                ret=true;
              }
              for (int j=0; j<128; j++) {
                snprintf(bindID,1024,"%d (0x%.2X)##BV2_%d",j,j,j);
                if (ImGui::Selectable(bindID,bind.data2==j)) {
                  bind.data2=j;
                  ret=true;
                }
              }
              ImGui::EndCombo();
            }

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::BeginCombo("##BAction",(bind.action==0)?_("--none--"):guiActions[bind.action].friendlyName)) {
              if (ImGui::Selectable(_("--none--"),bind.action==0)) {
                bind.action=0;
                ret=true;
              }
              for (int j=0; j<GUI_ACTION_MAX; j++) {
                if (strcmp(guiActions[j].friendlyName,"")==0) continue;
                if (strstr(guiActions[j].friendlyName,"---")==guiActions[j].friendlyName) {
                  ImGui::TextUnformatted(guiActions[j].friendlyName);
                } else {
                  snprintf(bindID,1024,"%s##BA_%d",_(guiActions[j].friendlyName),j);
                  if (ImGui::Selectable(bindID,bind.action==j)) {
                    bind.action=j;
                    ret=true;
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
              ret=true;
            }
            popToggleColors();

            ImGui::TableNextColumn();
            if (ImGui::Button(ICON_FA_TIMES "##BRemove")) {
              midiMap.binds.erase(midiMap.binds.begin()+i);
              if (learning==(int)i) learning=-1;
              i--;
              ret=true;
            }

            ImGui::PopID();
          }
          ImGui::EndTable();
        }

        return ret;
      }),
    }),
    SUBCATEGORY(_N("MIDI output"),{
      SettingEntry(_N("MIDI output device"),NULL,[this]{
        bool ret=false;
        ImGui::AlignTextToFramePadding();
        ImGui::Text(_("MIDI output"));
        ImGui::SameLine();
        String midiOutName=settings.midiOutDevice.empty()?_("<disabled>"):settings.midiOutDevice;
        if (ImGui::BeginCombo("##MidiOutDevice",midiOutName.c_str())) {
          if (ImGui::Selectable(_("<disabled>"),settings.midiOutDevice.empty())) {
            settings.midiOutDevice="";
            ret=true;
          }
          for (String& i: e->getMidiIns()) {
            if (ImGui::Selectable(i.c_str(),i==settings.midiOutDevice)) {
              settings.midiOutDevice=i;
              ret=true;
            }
          }
          ImGui::EndCombo();
        }

        ImGui::Text(_("Output mode:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("Off (use for TX81Z)"),settings.midiOutMode==0)) {
          settings.midiOutMode=0;
          ret=true;
        }
        if (ImGui::RadioButton(_("Melodic"),settings.midiOutMode==1)) {
          settings.midiOutMode=1;
          ret=true;
        }
        /*
        if (ImGui::RadioButton(_("Light Show (use for Launchpad)"),settings.midiOutMode==2)) {
          settings.midiOutMode=2;
        }*/
        ImGui::Unindent();

        bool midiOutProgramChangeB=settings.midiOutProgramChange;
        if (ImGui::Checkbox(_("Send Program Change"),&midiOutProgramChangeB)) {
          settings.midiOutProgramChange=midiOutProgramChangeB;
          ret=true;
        }

        bool midiOutClockB=settings.midiOutClock;
        if (ImGui::Checkbox(_("Send MIDI clock"),&midiOutClockB)) {
          settings.midiOutClock=midiOutClockB;
          ret=true;
        }

        bool midiOutTimeB=settings.midiOutTime;
        if (ImGui::Checkbox(_("Send MIDI timecode"),&midiOutTimeB)) {
          settings.midiOutTime=midiOutTimeB;
          ret=true;
        }

        if (settings.midiOutTime) {
          ImGui::Text(_("Timecode frame rate:"));
          ImGui::Indent();
          if (ImGui::RadioButton(_("Closest to Tick Rate"),settings.midiOutTimeRate==0)) {
            settings.midiOutTimeRate=0;
            ret=true;
          }
          if (ImGui::RadioButton(_("Film (24fps)"),settings.midiOutTimeRate==1)) {
            settings.midiOutTimeRate=1;
            ret=true;
          }
          if (ImGui::RadioButton(_("PAL (25fps)"),settings.midiOutTimeRate==2)) {
            settings.midiOutTimeRate=2;
            ret=true;
          }
          if (ImGui::RadioButton(_("NTSC drop (29.97fps)"),settings.midiOutTimeRate==3)) {
            settings.midiOutTimeRate=3;
            ret=true;
          }
          if (ImGui::RadioButton(_("NTSC non-drop (30fps)"),settings.midiOutTimeRate==4)) {
            settings.midiOutTimeRate=4;
            ret=true;
          }
          ImGui::Unindent();
        }
        return ret;
      }),
    }),
  }
  CATEGORY_END
  CATEGORY_BEGIN(_N("Emulation")) {},{
    SUBCATEGORY(_N("Cores"),{
      // the table of doom...
      SettingEntry(_N("Emulation cores"),NULL,[this]{
        bool ret=false;
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

          CORE_SETTING("YM2151",arcadeCore,arcadeCores);
          CORE_SETTING("YM2612",ym2612Core,ym2612Cores);
          CORE_SETTING("SN76489",snCore,snCores);
          CORE_SETTING("NES",nesCore,nesCores);
          CORE_SETTING("FDS",fdsCore,nesCores);
          CORE_SETTING("SID",c64Core,c64Cores);
          CORE_SETTING("POKEY",pokeyCore,pokeyCores);
          CORE_SETTING("OPN",opn1Core,opnCores);
          CORE_SETTING("OPNA",opnaCore,opnCores);
          CORE_SETTING("OPNB",opnbCore,opnCores);
          CORE_SETTING("OPL/OPL2/Y8950",opl2Core,opl2Cores);
          CORE_SETTING("OPL3",opl3Core,opl3Cores);
          CORE_SETTING("OPL4",opl4Core,opl4Cores);
          CORE_SETTING("ESFM",esfmCore,esfmCores);
          CORE_SETTING("OPLL",opllCore,opllCores);
          CORE_SETTING("AY-3-8910/SSG",ayCore,ayCores);

          ImGui::EndTable();
        }
        return ret;
      }),
    }),
    SUBCATEGORY(_N("Quality"),{
      SettingEntry(_N("Core quality"),NULL,[this]{
        bool ret=false;
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
        return ret;
      }),
    }),
    SUBCATEGORY(_N("Other"),{
      SettingEntry::ComboInt(
        _N("PC Speaker strategy"),
        "pcSpeakerOutMethod",&settings.pcSpeakerOutMethod,{
          {_N("evdev SND_TONE"),0},
          {_N("KIOCSOUND on /dev/tty1"),1},
          {_N("/dev/port"),2},
          {_N("KIOCSOUND on standard output"),3},
          {_N("outb()"),4},
        }
      )
    }),
    SUBCATEGORY(_N("Sample ROMs"),{
      SettingEntry::Path(
        _N("OPL4 YRW801 path"),
        "yrw801Path",&settings.yrw801Path,
        GUI_FILE_YRW801_ROM_OPEN
      ),
      /*
      SettingEntry::Path(
        _N("MultiPCM TG100 path"),
        "tg100Path",&settings.tg100Path,
        GUI_FILE_TG100_ROM_OPEN
      ),
      SettingEntry::Path(
        _N("MultiPCM MU5 path"),
        "mu5Path",&settings.mu5Path,
        GUI_FILE_MU5_ROM_OPEN
      ),
      */
    })
  }
  CATEGORY_END
  CATEGORY_BEGIN(_N("Interface")) {
    SettingEntry(_N("Workspace layout import/export/reset"),NULL,[this]{
      ImGui::AlignTextToFramePadding();
      ImGui::Text(_("Workspace layout:"));
      ImGui::SameLine();
      if (ImGui::Button(_("Import##intf"))) {
        openFileDialog(GUI_FILE_IMPORT_LAYOUT);
      }
      ImGui::SameLine();
      if (ImGui::Button(_("Export##intf"))) {
        openFileDialog(GUI_FILE_EXPORT_LAYOUT);
      }
      ImGui::SameLine();
      if (ImGui::Button(_("Reset##intf"))) {
        showWarning(_("Are you sure you want to reset the workspace layout?"),GUI_WARN_RESET_LAYOUT);
      }
      return false;
    }),
  },{
    SUBCATEGORY(_N("Layout"),{
      SETTING_CHECKBOX(
        _N("Allow docking editors"),
        allowEditDocking
      ),
#ifndef IS_MOBILE
      SETTING_CHECKBOX(
        _N("Remember window position"),
        saveWindowPos
      ).Tooltip(_("remembers the window's last position on start-up.")),
#endif
      SETTING_CHECKBOX(
        _N("Only allow window movement when clicking on title bar"),
        moveWindowTitle
      ).Callback([this]{
        applyUISettings(false);
      }),
      SETTING_CHECKBOX(
        _N("Center pop-up windows"),
        centerPopup
      ),
      SettingEntry::Radio(
        _N("Play/edit controls layout:"),
        "controlLayout",&settings.controlLayout,{
          {_N("Classic##ecl0"),0},
          {_N("Compact##ecl1"),1},
          {_N("Compact (vertical)##ecl2"),2},
          {_N("Split##ecl3"),3},
        }
      ),
      SettingEntry::Radio(
        _N("Position of buttons in Orders:"),
        "orderButtonPos",&settings.orderButtonPos,{
          {_N("Top##obp0"),0},
          {_N("Left##obp1"),1},
          {_N("Right##obp2"),2},
        }
      ),
    }),
    SUBCATEGORY(_N("Mouse"),{
      SettingEntry::SliderFloat(
        _N("Double-click time (seconds)"),
        "doubleClickTime",&settings.doubleClickTime,
        {0.02f,1.0f}
      ),
      SETTING_CHECKBOX(
        _N("Don't raise pattern editor on click"),
        avoidRaisingPattern
      ),
      SETTING_CHECKBOX(
        _N("Focus pattern editor when selecting instrument"),
        insFocusesPattern
      ),
      SETTING_CHECKBOX(
        _N("Draggable instruments/samples/waves"),
        draggableDataView
      ),
      SettingEntry::Radio(
        _N("Note preview behavior:"),
        "notePreviewBehavior",&settings.notePreviewBehavior,{
          {_N("Never##npb0"),0},
          {_N("When cursor is in Note column##npb1"),1},
          {_N("When cursor is in Note column or not in edit mode##npb2"),2},
          {_N("Always##npb3"),3},
        }
      ),
      SettingEntry::Radio(
        _N("Allow dragging selection:"),
        "dragMovesSelection",&settings.dragMovesSelection,{
          {_N("No##dms0"),0},
          {_N("Yes##dms1"),1},
          {_N("Yes (while holding Ctrl only)##dms2"),2},
          {_N("Yes (copy)##dms3"),3},
          {_N("Yes (while holding Ctrl only and copy)##dms4"),4},
          {_N("Yes (holding Ctrl copies)##dms5"),5},
        }
      ),
      SettingEntry::Radio(
        _N("Toggle channel solo on:"),
        "soloAction",&settings.soloAction,{
          {_N("Right-click or double-click##soloA"),0},
          {_N("Right-click##soloR"),1},
          {_N("Double-click##soloD"),2},
        }
      ),
      SettingEntry::Radio(
        _N("Modifier for alternate wheel-scrolling (vertical/zoom/slider-input):"),
        "ctrlWheelModifier",&settings.ctrlWheelModifier,{
          {_N("Ctrl or Meta/Cmd##cwm1"),0},
          {_N("Ctrl##cwm2"),1},
          {_N("Meta/Cmd##cwm3"),2},
          // technically this key is called Option on mac, but we call it Alt in getKeyName(s)
          {_N("Alt##cwm4"),3},
        }
      ),
      SETTING_CHECKBOX(
        _N("Double click selects entire column"),
        doubleClickColumn
      ),
    }),
    SUBCATEGORY(_N("Cursor behavior"),{
      SETTING_CHECKBOX(
        _N("Insert pushes entire channel row"),
        insertBehavior
      ),
      SETTING_CHECKBOX(
        _N("Pull delete affects entire channel row"),
        pullDeleteRow
      ),
      SETTING_CHECKBOX(
        _N("Push value when overwriting instead of clearing it"),
        pushNibble
      ),
      SETTING_CHECKBOX(
        _N("Keyboard note/value input repeat (hold key to input continuously)"),
        inputRepeat
      ),
      SettingEntry::Radio(
        _N("Effect input behavior:"),
        "effectCursorDir",&settings.effectCursorDir,{
          {_N("Move down##eicb0"),0},
          {_N("Move to effect value (otherwise move down)##eicb1"),1},
          {_N("Move to effect value/next effect and wrap around##eicb2"),2},
        }
      ),
      SETTING_CHECKBOX(
        _N("Delete effect value when deleting effect"),
        effectDeletionAltersValue
      ),
      SETTING_CHECKBOX(
        _N("Change current instrument when changing instrument column (absorb)"),
        absorbInsInput
      ),
      SETTING_CHECKBOX(
        _N("Remove instrument value when inserting note off/release"),
        removeInsOff
      ),
      SETTING_CHECKBOX(
        _N("Remove volume value when inserting note off/release"),
        removeVolOff
      ),
    }),
    SUBCATEGORY(_N("Cursor movement"),{
      SettingEntry::Radio(
        _N("Wrap horizontally:"),
        "wrapHorizontal",&settings.wrapHorizontal,{
          {_N("No##wrapH0"),0},
          {_N("Yes##wrapH1"),1},
          {_N("Yes, and move to next/prev row##wrapH2"),2},
        }
      ),
      SettingEntry::Radio(
        _N("Wrap vertically:"),
        "wrapVertical",&settings.wrapVertical,{
          {_N("No##wrapV0"),0},
          {_N("Yes##wrapV1"),1},
          {_N("Yes, and move to next/prev pattern##wrapV2"),2},
          {_N("Yes, and move to next/prev pattern (wrap around)##wrapV3"),3},
        }
      ),
      SettingEntry::Radio(
        _N("Cursor movement keys behavior:"),
        "scrollStep",&settings.scrollStep,{
          {_N("Move by one##cmk0"),0},
          {_N("Move by Edit Step##cmk1"),1},
        }
      ),
      SETTING_CHECKBOX(
        _N("Move cursor by edit step on delete"),
        stepOnDelete
      ),
      SETTING_CHECKBOX(
        _N("Move cursor by edit step on insert (push)"),
        stepOnInsert
      ),
      SETTING_CHECKBOX(
        _N("Move cursor up on backspace-delete"),
        pullDeleteBehavior
      ),
      SETTING_CHECKBOX(
        _N("Move cursor to end of clipboard content when pasting"),
        cursorPastePos
      ),
    }),
    SUBCATEGORY(_N("Scrolling"),{
      SettingEntry::Radio(
        _N("Change order when scrolling outside of pattern bounds:"),
        "scrollChangesOrder",&settings.scrollChangesOrder,{
          {_N("No##pscroll0"),0},
          {_N("Yes##pscroll1"),1},
          {_N("Yes, and wrap around song##pscroll2"),2},
        }
      ),
      SETTING_CHECKBOX(
        _N("Cursor follows current order when moving it"),
        cursorFollowsOrder
      ).Tooltip(_("applies when playback is stopped.")),
      SETTING_CHECKBOX(
        _N("Don't scroll when moving cursor"),
        cursorMoveNoScroll
      ),
      SettingEntry::Radio(
        _N("Move cursor with scroll wheel:"),
        "cursorFollowsWheel",&settings.cursorFollowsWheel,{
          {_N("No##csw0"),0},
          {_N("Yes##csw1"),1},
          {_N("Inverted##csw2"),2},
        }
      ),
      SettingEntry::Radio(
        _N("How many steps to move with each scroll wheel step?"),
        "cursorWheelStep",&settings.cursorWheelStep,{
          {_N("One##cws0"),0},
          {_N("Edit Step##cws1"),1},
          {_N("Coarse Step##cws2"),2},
        }
      ).Condition([this]{
        return settings.cursorFollowsWheel;
      }),
    }),
    SUBCATEGORY(_N("Assets"),{
      SETTING_CHECKBOX(
        _N("Display instrument type menu when adding instrument"),
        insTypeMenu
      ),
      SETTING_CHECKBOX(
        _N("Select asset after opening one"),
        selectAssetOnLoad
      ),
    }),
  }
  CATEGORY_END
  CATEGORY_BEGIN(_N("Appearance")) {},{
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
      ).Condition([this]{
        return settings.dpiScale>0.5f;
      }),
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
      SettingEntry::InputInt(
        _N("Header font size (2nd level)"),"headFontSize2",
        &settings.headFontSize2,
        {3,96,1,3}
      ),
      SettingEntry::InputInt(
        _N("Header font size (3rd level)"),"headFontSize3",
        &settings.headFontSize3,
        {3,96,1,3}
      ),
      SettingEntry::InputInt(
        _N("Header font size (4th level)"),"headFontSize4",
        &settings.headFontSize4,
        {3,96,1,3}
      ),
      SettingEntry::ComboInt(
        _N("Pattern font"),"patFont",
        &settings.patFont,{
          {"IBM Plex Mono",0},
          {"Mononoki",1},
          {"PT Mono",2},
          {"Proggy Clean",3},
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
        _N("Anti-aliased fonts"),
        fontAntiAlias
      ).Condition([this]{
        return settings.fontBackend==1;
      }),
      SETTING_CHECKBOX(
        _N("Support bitmap fonts"),
        fontBitmap
      ).Condition([this]{
        return settings.fontBackend==1;
      }),
      SettingEntry::Radio(
        _N("Hinting:"),"fontHinting",
        &settings.fontHinting,{
          {_N("Off (soft)##fh0"),0},
          {_N("Slight##fh1"),1},
          {_N("Normal##fh2"),2},
          {_N("Full (hard)##fh3"),3}
        }
      ).Condition([this]{
        return settings.fontBackend==1;
      }),
      SettingEntry::Radio(
        _N("Auto-hinter:"),"fontAutoHint",
        &settings.fontAutoHint,{
          {_N("Disable##fah0"),0},
          {_N("Enable##fah1"),1},
          {_N("Force##fah2"),2}
        }
      ).Condition([this]{
        return settings.fontBackend==1;
      }),
      SettingEntry::Radio(
        _N("Oversample:"),"fontOversample",
        &settings.fontOversample,{
          {_N("1×##fos1"),1,_N("saves video memory. reduces font rendering quality.\nuse for pixel/bitmap fonts.")},
          {_N("2×##fos2"),2,_N("default.")},
          {_N("3×##fos3"),3,_N("slightly better font rendering quality.\nuses more video memory.")}
        }
      ),
      SETTING_CHECKBOX(
        _N("Load fallback font"),
        loadFallback
      ).Tooltip(_N("disable to save video memory.")),
      SETTING_CHECKBOX(
        _N("Load fallback font (pattern)"),
        loadFallbackPat
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
      ).Callback([this]{
        updateWindowTitle();
      }),
      SETTING_CHECKBOX(
        _N("Display system name on title bar"),
        titleBarSys
      ).Callback([this]{
        updateWindowTitle();
      }),
      SETTING_CHECKBOX(
        _N("Display chip names instead of \"multi-system\" in title bar"),
        noMultiSystem
      ).Callback([this]{
        updateWindowTitle();
      }),
      SettingEntry::Radio(
        _N("Status bar:"),"statusDisplay",
        &settings.statusDisplay,{
          {_N("Cursor details##sbar0"),0},
          {_N("File path##sbar1"),1},
          {_N("Cursor details or file path##sbar2"),2},
          {_N("Nothing##sbar3"),3}
        }
      ),
      SETTING_CHECKBOX(
        _("Display playback status when playing"),
        playbackTime
      ),
      SettingEntry::Radio(
        _N("Export options layout:"),"exportOptionsLayout",
        &settings.exportOptionsLayout,{
          {_N("Sub-menus in File menu##eol0"),0},
          {_N("Modal window with tabs##eol1"),1},
          {_N("Modal windows with options in File menu##eol2"),2}
        }
      ),
      SETTING_CHECKBOX(
        _N("Capitalize menu bar"),
        capitalMenuBar
      ),
      SETTING_CHECKBOX(
        _N("Display add/configure/change/remove chip menus in File menu"),
        classicChipOptions
      )
    }),
    SUBCATEGORY(_N("Orders"),{
      // sorry. temporarily disabled until ImGui has a way to add separators in tables arbitrarily.
      // SETTING_CHECKBOX(_N("Add separators between systems in Orders"),sysSeparators),
      SETTING_CHECKBOX(
        _N("Highlight channel at cursor in Orders"),
        ordersCursor
      ),
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
    }),
    SUBCATEGORY(_N("Channel"),{
      SettingEntry::Radio(
        _N("Channel style:"),
        "channelStyle",&settings.channelStyle,{
          {_N("Classic##CHS0"),0},
          {_N("Line##CHS1"),1},
          {_N("Round##CHS2"),2},
          {_N("Split button##CHS3"),3},
          {_N("Square border##CHS4"),4},
          {_N("Round border##CHS5"),5},
        }
      ),
      SettingEntry::Radio(
        _N("Channel volume bar:"),
        "channelVolStyle",&settings.channelVolStyle,{
          {_N("None##CHV0"),0},
          {_N("Simple##CHV1"),1},
          {_N("Stereo##CHV2"),2},
          {_N("Real##CHV3"),3},
          {_N("Real (stereo)##CHV4"),4},
        }
      ),
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
      ).Condition([this]{
        return settings.channelFeedbackStyle==4;
      }),
      SettingEntry::Radio(
        _N("Channel font:"),
        "channelFont",&settings.channelFont,{
          {_N("Regular##CHFont0"),0},
          {_N("Monospace##CHFont1"),1},
        }
      ),
      SETTING_CHECKBOX(_("Center channel name"),channelTextCenter),
      SettingEntry::Radio(
        _N("Channel colors:"),
        "channelColors",&settings.channelColors,{
          {_N("Single##CHC0"),0},
          {_N("Channel type##CHC1"),1},
          {_N("Instrument type##CHC2"),2},
        }
      ),
      SettingEntry::Radio(
        _N("Channel name colors:"),
        "channelTextColors",&settings.channelTextColors,{
          {_N("Single##CTC0"),0},
          {_N("Channel type##CTC1"),1},
          {_N("Instrument type##CTC2"),2},
        }
      ),
    }),
    SUBCATEGORY(_N("Assets"),{
      SETTING_CHECKBOX(_("Unified instrument/wavetable/sample list"),unifiedDataView).Callback([this] {
        if (settings.unifiedDataView) {
          settings.horizontalDataView=0;
        }
      }),
      SETTING_CHECKBOX(_("Horizontal instrument/wavetable list"),horizontalDataView).Condition([this] {
        return !settings.unifiedDataView;
      }),
      SettingEntry::Radio(
        _N("Instrument list icon style:"),
        "insIconsStyle",&settings.insIconsStyle,{
          {_N("None##iis0"),0},
          {_N("Graphical icons##iis1"),1},
          {_N("Letter icons##iis2"),2},
        }
      ),
      SETTING_CHECKBOX(_("Colorize instrument editor using instrument type"),insEditColorize),
    }),
    SUBCATEGORY(_N("Macro Editor"),{
      SettingEntry::Radio(
        _N("Macro editor layout:"),
        "macroLayout",&settings.macroLayout,{
          {_N("Unified##mel0"),0},
          {_N("Tabs##mel1"),1},
          {_N("Grid##mel2"),2},
          {_N("Single (with list)##mel3"),3},
          {_N("Single (combo box)##mel4"),4},
        }
      ),
      SETTING_CHECKBOX(_("Use classic macro editor vertical slider"),oldMacroVSlider),
      SettingEntry::Radio(
        _N("Macro step size/horizontal zoom::"),
        "autoMacroStepSize",&settings.autoMacroStepSize,{
          {_N("Manual"),0},
          {_N("Automatic per macro"),1},
          {_N("Automatic (use longest macro)"),2},
        }
      ).Condition([this] {
        return settings.macroLayout!=2;
      }),
    }),
    SUBCATEGORY(_N("FM Editor"),{
      SettingEntry::Radio(
        _N("FM parameter names:"),
        "fmNames",&settings.fmNames,{
          {_N("Friendly##fmn0"),0},
          {_N("Technical##fmn1"),1},
          {_N("Technical (alternate)##fmn2"),2},
        }
      ),
      SETTING_CHECKBOX(_("Use standard OPL waveform names"),oplStandardWaveNames),
      SettingEntry::Radio(
        _N("FM parameter editor layout:"),
        "fmLayout",&settings.fmLayout,{
          {_N("Modern##fml0"),0},
          {_N("Modern with more labels##fml7"),7},
          {_N("Compact (2x2, classic)##fml1"),1},
          {_N("Compact (1x4)##fml2"),2},
          {_N("Compact (4x1)##fml3"),3},
          {_N("Alternate (2x2)##fml4"),4},
          {_N("Alternate (1x4)##fml5"),5},
          {_N("Alternate (4x1)##fml5"),6},
        }
      ),
      // HACK: the original code disabled the last two options if settings.fmLayout!=0. we can't do that here...
      SettingEntry::Radio(
        _N("Position of Sustain in FM editor:"),
        "susPosition",&settings.susPosition,{
          {_("Between Decay and Sustain Rate##susp0"),0},
          {_("After Release Rate##susp1"),1},
          {_("After Release Rate, after spacing##susp2"),2},
          {_("After TL##susp3"),3},
        }
      ).Condition([this] {
        return settings.fmLayout==0;
      }),
      SettingEntry::Radio(
        _N("Position of Sustain in FM editor:"),
        "susPosition",&settings.susPosition,{
          {_("Between Decay and Sustain Rate##susp0"),0},
          {_("After Release Rate##susp1"),1},
        }
      ).Condition([this] {
        return settings.fmLayout!=0;
      }),
      SETTING_CHECKBOX(_("Use separate colors for carriers/modulators in FM editor"),separateFMColors),
      SETTING_CHECKBOX(_("Unsigned FM detune values"),unsignedDetune),
    }),
    SUBCATEGORY(_N("Memory Composition"),{
      SettingEntry::Radio(
        _N("Chip memory usage unit:"),
        "memUsageUnit",&settings.memUsageUnit,{
          {_N("Bytes##MUU0"),0},
          {_N("Kilobytes##MUU1"),1},
        }
      ),
    }),
    SUBCATEGORY(_N("Oscilloscope"),{
      SETTING_CHECKBOX(_("Rounded corners"),oscRoundedCorners),
      SETTING_CHECKBOX(_("Border"),oscBorder),
      SETTING_CHECKBOX(_("Mono"),oscMono),
      SETTING_CHECKBOX(_("Anti-aliased"),oscAntiAlias),
      SETTING_CHECKBOX(_("Fill entire window"),oscTakesEntireWindow),
      SETTING_CHECKBOX(_("Waveform goes out of bounds"),oscEscapesBoundary),
      SettingEntry::SliderFloat(
        _N("Line size"),
        "oscLineSize",&settings.oscLineSize,
        {0.25f,16.0f,"%.1f"}
      ),
      // ...
      SettingEntry(_N("Per-channel oscilloscope threads"),NULL,[this]{
        bool ret=false;
        pushWarningColor(settings.chanOscThreads>cpuCores,settings.chanOscThreads>(cpuCores*2));
        if (ImGui::InputInt(_("Per-channel oscilloscope threads"),&settings.chanOscThreads)) {
          if (settings.chanOscThreads<0) settings.chanOscThreads=0;
          if (settings.chanOscThreads>(cpuCores*3)) settings.chanOscThreads=cpuCores*3;
          if (settings.chanOscThreads>256) settings.chanOscThreads=256;
          ret=true;
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
        return ret;
      }),
      SettingEntry::Radio(
        _N("Oscilloscope rendering engine:"),
        "shaderOsc",&settings.shaderOsc,{
          {_N("ImGui line plot"),0,_N("render using Dear ImGui's built-in line drawing functions.")},
#ifdef USE_GLES
          {_N("GLSL (if available)"),1,_N("render using shaders that run on the graphics card.\nonly available in OpenGL ES 2.0 render backend.")},
#else
          {_N("GLSL (if available)"),1,_N("render using shaders that run on the graphics card.\nonly available in OpenGL 3.0 render backend.")},
#endif
        }
      ),
    }),
    SUBCATEGORY(_N("Song Comments"),{
      SETTING_CHECKBOX(_("Wrap text"),songNotesWrap),
    }),
    SUBCATEGORY(_N("Chip Manager"),{
      SETTING_CHECKBOX(_("Show channel indicators"),rackShowLEDs),
    }),
    SUBCATEGORY(_N("Mixer"),{
      SettingEntry::Radio(
        _N("Mixer layout:"),
        "mixerLayout",&settings.mixerLayout,{
          {_N("Horizontal##mixl0"),0},
          {_N("Vertical##mixl1"),1},
        }
      ),
      SettingEntry::Radio(
        _N("Mixer style:"),
        "mixerStyle",&settings.mixerStyle,{
          {_N("No volume meters"),0},
          {_N("Volume meters separate"),1},
          {_N("Volume meters in volume sliders"),2},
        }
      ),
    }),
    SUBCATEGORY(_N("Windows"),{
      SETTING_CHECKBOX(_("Rounded window corners"),roundedWindows),
      SETTING_CHECKBOX(_("Rounded buttons"),roundedButtons),
      SETTING_CHECKBOX(_("Rounded menu corners"),roundedMenus),
      SETTING_CHECKBOX(_("Rounded tabs"),roundedTabs),
      SETTING_CHECKBOX(_("Rounded scrollbars"),roundedScrollbars),
      SETTING_CHECKBOX(_("Borders around widgets"),frameBorders),
    }),
  }
  CATEGORY_END
  CATEGORY_BEGIN(_N("Color")) {
    SettingEntry(_N("Color scheme Import/Export/Reset defaults"),NULL,[this]{
      ImGui::AlignTextToFramePadding();
      ImGui::Text(_("Color scheme:"));
      ImGui::SameLine();
      if (ImGui::Button(_("Import##col"))) {
        openFileDialog(GUI_FILE_IMPORT_COLORS);
      }
      ImGui::SameLine();
      if (ImGui::Button(_("Export##col"))) {
        openFileDialog(GUI_FILE_EXPORT_COLORS);
      }
      ImGui::SameLine();
      if (ImGui::Button(_("Reset defaults##col"))) {
        showWarning(_("Are you sure you want to reset the color scheme?"),GUI_WARN_RESET_COLORS);
      }
      return false;
    }),
    SETTING_INV_CHECKBOX(_N("Guru mode"),basicColors),
  },{
    SUBCATEGORY(_N("Interface"),{
      SettingEntry::SliderInt(
        _N("Frame shading"),
        "guiColorsShading",&settings.guiColorsShading,
        {0,100,"%d%%"}
      ).Callback([this]{applyUISettings(false);}),
#define BASIC_MODE .Condition([this]{return settings.basicColors;})
      SettingEntry::Radio(
        _N("Color scheme type:"),
        "guiColorsBase",&settings.guiColorsBase,
        {
          {_N("Dark##gcb0"),0},
          {_N("Light##gcb1"),1}
        }
      ).Callback([this]{applyUISettings(false);}) BASIC_MODE,
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
  CATEGORY_BEGIN(_N("Keyboard")) {
    SettingEntry(_N("Keyboard Import/Export/Reset defaults"),NULL,[this]{
      if (ImGui::Button(_("Import##kbd"))) {
        openFileDialog(GUI_FILE_IMPORT_KEYBINDS);
      }
      ImGui::SameLine();
      if (ImGui::Button(_("Export##kbd"))) {
        openFileDialog(GUI_FILE_EXPORT_KEYBINDS);
      }
      ImGui::SameLine();
      if (ImGui::Button(_("Reset defaults##kbd"))) {
        showWarning(_("Are you sure you want to reset the keyboard settings?"),GUI_WARN_RESET_KEYBINDS);
      }
      return false;
    })
  },{
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
    SUBCATEGORY(_N("Window activation"),{
      SETTING_KEYBIND(GUI_ACTION_WINDOW_FIND),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_SETTINGS),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_SONG_INFO),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_SUBSONGS),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_SPEED),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_INS_LIST),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_WAVE_LIST),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_SAMPLE_LIST),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_ORDERS),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_PATTERN),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_MIXER),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_GROOVES),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_CHANNELS),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_PAT_MANAGER),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_SYS_MANAGER),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_COMPAT_FLAGS),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_NOTES),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_INS_EDIT),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_WAVE_EDIT),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_SAMPLE_EDIT),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_EDIT_CONTROLS),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_PIANO),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_OSCILLOSCOPE),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_CHAN_OSC),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_XY_OSC),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_VOL_METER),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_CLOCK),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_REGISTER_VIEW),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_LOG),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_STATS),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_MEMORY),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_EFFECT_LIST),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_DEBUG),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_CS_PLAYER),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_REF_PLAYER),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_TUNER),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_SPECTRUM),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_MULTI_INS_SETUP),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_BACKUPS_MANAGER),
      SETTING_KEYBIND(GUI_ACTION_WINDOW_ABOUT),
      SETTING_KEYBIND(GUI_ACTION_COLLAPSE_WINDOW),
      SETTING_KEYBIND(GUI_ACTION_CLOSE_WINDOW),

      SETTING_KEYBIND(GUI_ACTION_COMMAND_PALETTE),
      SETTING_KEYBIND(GUI_ACTION_CMDPAL_RECENT),
      SETTING_KEYBIND(GUI_ACTION_CMDPAL_INSTRUMENTS),
      SETTING_KEYBIND(GUI_ACTION_CMDPAL_SAMPLES),
      SETTING_KEYBIND(GUI_ACTION_CMDPAL_INSTRUMENT_CHANGE),
      SETTING_KEYBIND(GUI_ACTION_CMDPAL_ADD_CHIP),
    }),
    SUBCATEGORY(_N("Note input"),{
      SettingEntry(_N("Note input"),NULL,[this]{
        bool ret=false;
        if (ImGui::BeginTable("keysNoteInput",3)) {
          ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed);
          ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthFixed);
          ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthStretch);
          static char id[4096];

          ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
          ImGui::TableNextColumn();
          ImGui::Text(_("Key"));
          ImGui::TableNextColumn();
          ImGui::Text(_("Type"));
          ImGui::TableNextColumn();
          ImGui::Text(_("Value"));

          for (size_t _i=0; _i<noteKeysRaw.size(); _i++) {
            ImGui::PushID(_i);
            MappedInput& i=noteKeysRaw[_i];
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            if (ImGui::Button(ICON_FA_TIMES "##SNRemove")) {
              noteKeysRaw.erase(noteKeysRaw.begin()+_i);
              _i--;
              ret=true;
            }
            ImGui::SameLine();
            ImGui::Text("%s",SDL_GetScancodeName((SDL_Scancode)i.scan));
            ImGui::TableNextColumn();
            if (i.val==103) {
              if (ImGui::Button(_("Toggle raw note##SNType"))) {
                i.val=0;
                ret=true;
              }
            } else if (i.val==102) {
              if (ImGui::Button(_("Macro release##SNType"))) {
                i.val=103;
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
              snprintf(id,4095,_("%%2d (%c%c, +%d oct.)"),
                note[0],note[1]=='-'?' ':note[1],i.val/12);
              if (ImGui::InputScalar("##SNValue",ImGuiDataType_S32,&i.val,&_ONE,&_TWELVE,id)) {
                if (i.val<0) i.val=0;
                if (i.val>96) i.val=96;
                ret=true;
              }
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
    }),
    SUBCATEGORY(_N("Pattern"),{
      SETTING_KEYBIND(GUI_ACTION_PAT_NOTE_UP),
      SETTING_KEYBIND(GUI_ACTION_PAT_NOTE_DOWN),
      SETTING_KEYBIND(GUI_ACTION_PAT_OCTAVE_UP),
      SETTING_KEYBIND(GUI_ACTION_PAT_OCTAVE_DOWN),
      SETTING_KEYBIND(GUI_ACTION_PAT_VALUE_UP),
      SETTING_KEYBIND(GUI_ACTION_PAT_VALUE_DOWN),
      SETTING_KEYBIND(GUI_ACTION_PAT_VALUE_UP_COARSE),
      SETTING_KEYBIND(GUI_ACTION_PAT_VALUE_DOWN_COARSE),
      SETTING_KEYBIND(GUI_ACTION_PAT_SELECT_ALL),
      SETTING_KEYBIND(GUI_ACTION_PAT_CUT),
      SETTING_KEYBIND(GUI_ACTION_PAT_COPY),
      SETTING_KEYBIND(GUI_ACTION_PAT_PASTE),
      SETTING_KEYBIND(GUI_ACTION_PAT_PASTE_MIX),
      SETTING_KEYBIND(GUI_ACTION_PAT_PASTE_MIX_BG),
      SETTING_KEYBIND(GUI_ACTION_PAT_PASTE_FLOOD),
      SETTING_KEYBIND(GUI_ACTION_PAT_PASTE_OVERFLOW),
      SETTING_KEYBIND(GUI_ACTION_PAT_CURSOR_UP),
      SETTING_KEYBIND(GUI_ACTION_PAT_CURSOR_DOWN),
      SETTING_KEYBIND(GUI_ACTION_PAT_CURSOR_LEFT),
      SETTING_KEYBIND(GUI_ACTION_PAT_CURSOR_RIGHT),
      SETTING_KEYBIND(GUI_ACTION_PAT_CURSOR_UP_ONE),
      SETTING_KEYBIND(GUI_ACTION_PAT_CURSOR_DOWN_ONE),
      SETTING_KEYBIND(GUI_ACTION_PAT_CURSOR_LEFT_CHANNEL),
      SETTING_KEYBIND(GUI_ACTION_PAT_CURSOR_RIGHT_CHANNEL),
      SETTING_KEYBIND(GUI_ACTION_PAT_CURSOR_PREVIOUS_CHANNEL),
      SETTING_KEYBIND(GUI_ACTION_PAT_CURSOR_NEXT_CHANNEL),
      SETTING_KEYBIND(GUI_ACTION_PAT_CURSOR_BEGIN),
      SETTING_KEYBIND(GUI_ACTION_PAT_CURSOR_END),
      SETTING_KEYBIND(GUI_ACTION_PAT_CURSOR_UP_COARSE),
      SETTING_KEYBIND(GUI_ACTION_PAT_CURSOR_DOWN_COARSE),
      SETTING_KEYBIND(GUI_ACTION_PAT_SELECTION_UP),
      SETTING_KEYBIND(GUI_ACTION_PAT_SELECTION_DOWN),
      SETTING_KEYBIND(GUI_ACTION_PAT_SELECTION_LEFT),
      SETTING_KEYBIND(GUI_ACTION_PAT_SELECTION_RIGHT),
      SETTING_KEYBIND(GUI_ACTION_PAT_SELECTION_UP_ONE),
      SETTING_KEYBIND(GUI_ACTION_PAT_SELECTION_DOWN_ONE),
      SETTING_KEYBIND(GUI_ACTION_PAT_SELECTION_BEGIN),
      SETTING_KEYBIND(GUI_ACTION_PAT_SELECTION_END),
      SETTING_KEYBIND(GUI_ACTION_PAT_SELECTION_UP_COARSE),
      SETTING_KEYBIND(GUI_ACTION_PAT_SELECTION_DOWN_COARSE),
      SETTING_KEYBIND(GUI_ACTION_PAT_MOVE_UP),
      SETTING_KEYBIND(GUI_ACTION_PAT_MOVE_DOWN),
      SETTING_KEYBIND(GUI_ACTION_PAT_MOVE_LEFT_CHANNEL),
      SETTING_KEYBIND(GUI_ACTION_PAT_MOVE_RIGHT_CHANNEL),
      SETTING_KEYBIND(GUI_ACTION_PAT_DELETE),
      SETTING_KEYBIND(GUI_ACTION_PAT_PULL_DELETE),
      SETTING_KEYBIND(GUI_ACTION_PAT_INSERT),
      SETTING_KEYBIND(GUI_ACTION_PAT_MUTE_CURSOR),
      SETTING_KEYBIND(GUI_ACTION_PAT_SOLO_CURSOR),
      SETTING_KEYBIND(GUI_ACTION_PAT_UNMUTE_ALL),
      SETTING_KEYBIND(GUI_ACTION_PAT_NEXT_ORDER),
      SETTING_KEYBIND(GUI_ACTION_PAT_PREV_ORDER),
      SETTING_KEYBIND(GUI_ACTION_PAT_COLLAPSE),
      SETTING_KEYBIND(GUI_ACTION_PAT_COLLAPSE_SELECTED),
      SETTING_KEYBIND(GUI_ACTION_PAT_EXPAND_SELECTED),
      SETTING_KEYBIND(GUI_ACTION_PAT_INCREASE_COLUMNS),
      SETTING_KEYBIND(GUI_ACTION_PAT_DECREASE_COLUMNS),
      SETTING_KEYBIND(GUI_ACTION_PAT_INTERPOLATE),
      SETTING_KEYBIND(GUI_ACTION_PAT_FADE),
      SETTING_KEYBIND(GUI_ACTION_PAT_INVERT_VALUES),
      SETTING_KEYBIND(GUI_ACTION_PAT_FLIP_SELECTION),
      SETTING_KEYBIND(GUI_ACTION_PAT_COLLAPSE_ROWS),
      SETTING_KEYBIND(GUI_ACTION_PAT_EXPAND_ROWS),
      SETTING_KEYBIND(GUI_ACTION_PAT_COLLAPSE_PAT),
      SETTING_KEYBIND(GUI_ACTION_PAT_EXPAND_PAT),
      SETTING_KEYBIND(GUI_ACTION_PAT_COLLAPSE_SONG),
      SETTING_KEYBIND(GUI_ACTION_PAT_EXPAND_SONG),
      SETTING_KEYBIND(GUI_ACTION_PAT_LATCH),
      SETTING_KEYBIND(GUI_ACTION_PAT_CLEAR_LATCH),
      SETTING_KEYBIND(GUI_ACTION_PAT_ABSORB_INSTRUMENT),
      SETTING_KEYBIND(GUI_ACTION_PAT_CURSOR_UNDO),
      SETTING_KEYBIND(GUI_ACTION_PAT_CURSOR_REDO),
    }),
    SUBCATEGORY(_N("Instrument list"),{
      SETTING_KEYBIND(GUI_ACTION_INS_LIST_ADD),
      SETTING_KEYBIND(GUI_ACTION_INS_LIST_DUPLICATE),
      SETTING_KEYBIND(GUI_ACTION_INS_LIST_OPEN),
      SETTING_KEYBIND(GUI_ACTION_INS_LIST_OPEN_REPLACE),
      SETTING_KEYBIND(GUI_ACTION_INS_LIST_SAVE),
      SETTING_KEYBIND(GUI_ACTION_INS_LIST_SAVE_DMP),
      SETTING_KEYBIND(GUI_ACTION_INS_LIST_MOVE_UP),
      SETTING_KEYBIND(GUI_ACTION_INS_LIST_MOVE_DOWN),
      SETTING_KEYBIND(GUI_ACTION_INS_LIST_DELETE),
      SETTING_KEYBIND(GUI_ACTION_INS_LIST_EDIT),
      SETTING_KEYBIND(GUI_ACTION_INS_LIST_UP),
      SETTING_KEYBIND(GUI_ACTION_INS_LIST_DOWN),
      SETTING_KEYBIND(GUI_ACTION_INS_LIST_DIR_VIEW),
    }),
    SUBCATEGORY(_N("Wavetable list"),{
      SETTING_KEYBIND(GUI_ACTION_WAVE_LIST_ADD),
      SETTING_KEYBIND(GUI_ACTION_WAVE_LIST_DUPLICATE),
      SETTING_KEYBIND(GUI_ACTION_WAVE_LIST_OPEN),
      SETTING_KEYBIND(GUI_ACTION_WAVE_LIST_OPEN_REPLACE),
      SETTING_KEYBIND(GUI_ACTION_WAVE_LIST_SAVE),
      SETTING_KEYBIND(GUI_ACTION_WAVE_LIST_SAVE_DMW),
      SETTING_KEYBIND(GUI_ACTION_WAVE_LIST_SAVE_RAW),
      SETTING_KEYBIND(GUI_ACTION_WAVE_LIST_MOVE_UP),
      SETTING_KEYBIND(GUI_ACTION_WAVE_LIST_MOVE_DOWN),
      SETTING_KEYBIND(GUI_ACTION_WAVE_LIST_DELETE),
      SETTING_KEYBIND(GUI_ACTION_WAVE_LIST_EDIT),
      SETTING_KEYBIND(GUI_ACTION_WAVE_LIST_UP),
      SETTING_KEYBIND(GUI_ACTION_WAVE_LIST_DOWN),
      SETTING_KEYBIND(GUI_ACTION_WAVE_LIST_DIR_VIEW),
    }),
    SUBCATEGORY(_N("Sample list"),{
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_LIST_ADD),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_LIST_DUPLICATE),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_CREATE_WAVE),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_LIST_OPEN),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_LIST_OPEN_REPLACE),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_LIST_OPEN_RAW),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_LIST_OPEN_REPLACE_RAW),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_LIST_SAVE),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_LIST_SAVE_RAW),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_LIST_MOVE_UP),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_LIST_MOVE_DOWN),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_LIST_DELETE),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_LIST_EDIT),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_LIST_UP),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_LIST_DOWN),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_LIST_PREVIEW),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_LIST_STOP_PREVIEW),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_LIST_DIR_VIEW),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_LIST_MAKE_MAP),
    }),
    SUBCATEGORY(_N("Orders"),{
      SETTING_KEYBIND(GUI_ACTION_ORDERS_UP),
      SETTING_KEYBIND(GUI_ACTION_ORDERS_DOWN),
      SETTING_KEYBIND(GUI_ACTION_ORDERS_LEFT),
      SETTING_KEYBIND(GUI_ACTION_ORDERS_RIGHT),
      SETTING_KEYBIND(GUI_ACTION_ORDERS_INCREASE),
      SETTING_KEYBIND(GUI_ACTION_ORDERS_DECREASE),
      SETTING_KEYBIND(GUI_ACTION_ORDERS_EDIT_MODE),
      SETTING_KEYBIND(GUI_ACTION_ORDERS_LINK),
      SETTING_KEYBIND(GUI_ACTION_ORDERS_ADD),
      SETTING_KEYBIND(GUI_ACTION_ORDERS_DUPLICATE),
      SETTING_KEYBIND(GUI_ACTION_ORDERS_DEEP_CLONE),
      SETTING_KEYBIND(GUI_ACTION_ORDERS_DUPLICATE_END),
      SETTING_KEYBIND(GUI_ACTION_ORDERS_DEEP_CLONE_END),
      SETTING_KEYBIND(GUI_ACTION_ORDERS_REMOVE),
      SETTING_KEYBIND(GUI_ACTION_ORDERS_MOVE_UP),
      SETTING_KEYBIND(GUI_ACTION_ORDERS_MOVE_DOWN),
      SETTING_KEYBIND(GUI_ACTION_ORDERS_REPLAY),
    }),
    SUBCATEGORY(_N("Sample editor"),{
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_SELECT),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_DRAW),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_CUT),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_COPY),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_PASTE),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_PASTE_REPLACE),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_PASTE_MIX),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_SELECT_ALL),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_RESIZE),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_RESAMPLE),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_AMPLIFY),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_NORMALIZE),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_FADE_IN),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_FADE_OUT),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_INSERT),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_SILENCE),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_DELETE),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_TRIM),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_REVERSE),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_INVERT),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_SIGN),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_FILTER),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_PREVIEW),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_STOP_PREVIEW),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_ZOOM_IN),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_ZOOM_OUT),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_ZOOM_AUTO),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_MAKE_INS),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_SET_LOOP),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_TRIM_AFTER_LOOP),
      SETTING_KEYBIND(GUI_ACTION_SAMPLE_TRIM_TO_LOOP),
    })
  }
  CATEGORY_END
  CATEGORY_BEGIN(_N("Backup Configuration")) {
    SETTING_CHECKBOX(
      _N("Enable backup system"),
      backupEnable
    ),
    SettingEntry::InputInt(
      _N("Interval (in seconds)"),"backupInterval",
      &settings.backupInterval,
      {10,86400}
    ),
    SettingEntry::InputInt(
      _N("Backups per file"),"backupMaxCopies",
      &settings.backupMaxCopies,
      {1,100}
    ),
    SettingEntry(_N("Backups"),NULL,[this]{
      if (ImGui::Button(_("Open backup management..."))) {
        nextWindow=GUI_WINDOW_BACKUPS_MANAGER;
        backupsManagerOpen=true;
      }
      return false;
    })
  }
  CATEGORY_END
  curCategory=&allSettings[0];
}
