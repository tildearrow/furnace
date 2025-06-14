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

#include <stdio.h>
#include <stdint.h>
#include "pch.h"
#ifdef HAVE_SDL2
#include "SDL_events.h"
#endif
#include "ta-log.h"
#include "fileutils.h"
#include "engine/engine.h"

#ifdef _WIN32
#include <windows.h>
#include <combaseapi.h>
#include <shellapi.h>
#if defined(HAVE_SDL2) && !defined(SUPPORT_XP)
#include <versionhelpers.h>
#endif
#include "utfutils.h"
#include "gui/shellScalingStub.h"

typedef HRESULT (WINAPI *SPDA)(PROCESS_DPI_AWARENESS);
#else
#include <signal.h>
#include <unistd.h>

struct sigaction termsa;
#endif

#ifdef SUPPORT_XP
#define TUT_INTRO_PLAYED true
#else
#define TUT_INTRO_PLAYED false
#endif

#ifdef HAVE_LOCALE
#ifdef HAVE_MOMO
#define TA_BINDTEXTDOMAIN momo_bindtextdomain
#define TA_TEXTDOMAIN momo_textdomain
#else
#define TA_BINDTEXTDOMAIN bindtextdomain
#define TA_TEXTDOMAIN textdomain
#endif

#ifdef HAVE_SETLOCALE
#include <locale.h>
#endif

#ifndef LC_CTYPE
#define LC_CTYPE 0
#endif
#ifndef LC_MESSAGES
#define LC_MESSAGES 1
#endif
#endif

#include "cli/cli.h"

#ifdef HAVE_GUI
#include "gui/gui.h"
#endif

DivEngine e;

#ifdef HAVE_GUI
FurnaceGUI g;
#endif

FurnaceCLI cli;

String outName;
String vgmOutName;
String cmdOutName;
String romOutName;
String txtOutName;
int benchMode=0;
int subsong=-1;
DivCSOptions csExportOptions;
DivAudioExportOptions exportOptions;
DivConfig romExportConfig;

#ifdef HAVE_GUI
bool consoleMode=false;
#else
bool consoleMode=true;
#endif

bool consoleNoStatus=false;
bool consoleNoControls=false;

bool displayEngineFailError=false;
bool displayLocaleFailError=false;
bool vgmOutDirect=false;

bool safeMode=false;
bool safeModeWithAudio=false;

bool infoMode=false;

bool noReportError=false;

std::vector<TAParam> params;

#ifdef HAVE_LOCALE
char reqLocaleCopy[64];
char localeDir[4096];

const char* localeDirs[]={
#ifdef __APPLE__
  "../Resources/locale",
#endif
  "locale",
  ".." DIR_SEPARATOR_STR "share" DIR_SEPARATOR_STR "locale",
  ".." DIR_SEPARATOR_STR "po" DIR_SEPARATOR_STR "locale",
#ifdef LOCALE_DIR
  LOCALE_DIR,
#endif
  NULL
};
#endif

bool getExePath(char* argv0, char* exePath, size_t maxSize) {
  if (argv0==NULL) return false;
#ifdef _WIN32
  wchar_t exePathW[4096];
  WString argv0W=utf8To16(argv0);
  if (GetFullPathNameW(argv0W.c_str(),4095,exePathW,NULL)==0) return false;
  String exePathS=utf16To8(exePathW);
  strncpy(exePath,exePathS.c_str(),maxSize);
#else
  if (realpath(argv0,exePath)==NULL) return false;
#endif
  char* lastChar=strrchr(exePath,DIR_SEPARATOR);
  if (lastChar==NULL) return false;
  *lastChar=0;
  return true;
}

TAParamResult pHelp(String) {
  printf("usage: furnace [params] [filename]\n"
         "you may specify the following parameters:\n");
  for (auto& i: params) {
    if (i.value) {
      printf("  -%s %s: %s\n",i.name.c_str(),i.valName.c_str(),i.desc.c_str());
    } else {
      printf("  -%s: %s\n",i.name.c_str(),i.desc.c_str());
    }
  }
  return TA_PARAM_QUIT;
}

TAParamResult pAudio(String val) {
  if (outName!="") {
    logE("can't use -audio and -output at the same time.");
    return TA_PARAM_ERROR;
  }
  if (val=="jack") {
    e.setAudio(DIV_AUDIO_JACK);
  } else if (val=="sdl") {
    e.setAudio(DIV_AUDIO_SDL);
  } else if (val=="portaudio") {
    e.setAudio(DIV_AUDIO_PORTAUDIO);
  } else if (val=="pipe") {
    e.setAudio(DIV_AUDIO_PIPE);
    changeLogOutput(stderr);
  } else {
    logE("invalid value for audio engine! valid values are: jack, sdl, portaudio, pipe.");
    return TA_PARAM_ERROR;
  }
  return TA_PARAM_SUCCESS;
}

TAParamResult pView(String val) {
  if (val=="pattern") {
    e.setView(DIV_STATUS_PATTERN);
  } else if (val=="commands") {
    e.setView(DIV_STATUS_COMMANDS);
  } else if (val=="nothing") {
    e.setView(DIV_STATUS_NOTHING);
  } else {
    logE("invalid value for view type! valid values are: pattern, commands, nothing.");
    return TA_PARAM_ERROR;
  }
  return TA_PARAM_SUCCESS;
}

TAParamResult pConsole(String val) {
  consoleMode=true;
  return TA_PARAM_SUCCESS;
}

TAParamResult pQuiet(String val) {
  noReportError=true;
  return TA_PARAM_SUCCESS;
}

TAParamResult pNoStatus(String val) {
  consoleNoStatus=true;
  return TA_PARAM_SUCCESS;
}

TAParamResult pNoControls(String val) {
  consoleNoControls=true;
  return TA_PARAM_SUCCESS;
}

TAParamResult pSafeMode(String val) {
#ifdef HAVE_GUI
  safeMode=true;
  return TA_PARAM_SUCCESS;
#else
  logE("Furnace was compiled without the GUI. safe mode is pointless.");
  return TA_PARAM_ERROR;
#endif
}

TAParamResult pSafeModeAudio(String val) {
#ifdef HAVE_GUI
  safeMode=true;
  safeModeWithAudio=true;
  return TA_PARAM_SUCCESS;
#else
  logE("Furnace was compiled without the GUI. safe mode is pointless.");
  return TA_PARAM_ERROR;
#endif
}

TAParamResult pDirect(String val) {
  vgmOutDirect=true;
  return TA_PARAM_SUCCESS;
}

TAParamResult pInfo(String val) {
  infoMode=true;
  return TA_PARAM_SUCCESS;
}

TAParamResult pLogLevel(String val) {
  if (val=="trace") {
    logLevel=LOGLEVEL_TRACE;
  } else if (val=="debug") {
    logLevel=LOGLEVEL_DEBUG;
  } else if (val=="info") {
    logLevel=LOGLEVEL_INFO;
  } else if (val=="warning") {
    logLevel=LOGLEVEL_WARN;
  } else if (val=="error") {
    logLevel=LOGLEVEL_ERROR;
  } else {
    logE("invalid value for loglevel! valid values are: trace, debug, info, warning, error.");
    return TA_PARAM_ERROR;
  }
  return TA_PARAM_SUCCESS;
}

TAParamResult pVersion(String) {
  printf("Furnace version " DIV_VERSION ".\n\n");
  printf("copyright (C) 2021-2025 tildearrow and contributors.\n");
  printf("licensed under the GNU General Public License version 2 or later\n");
  printf("<https://www.gnu.org/licenses/old-licenses/gpl-2.0.html>.\n\n");
  printf("this is free software with ABSOLUTELY NO WARRANTY.\n");
  printf("pass the -warranty parameter for more information.\n\n");
  printf("DISCLAIMER: this program is not affiliated with Delek in any form.\n");
  printf("\n");
  printf("furnace is powered by:\n");
  printf("- libsndfile by Erik de Castro Lopo and rest of libsndfile team (LGPLv2.1)\n");
  printf("- SDL2 by Sam Lantinga (zlib license)\n");
  printf("- zlib by Jean-loup Gailly and Mark Adler (zlib license)\n");
  printf("- PortAudio (PortAudio license)\n");
  printf("- Weak-JACK by x42 (GPLv2)\n");
  printf("- RtMidi by Gary P. Scavone (RtMidi license)\n");
  printf("- backward-cpp by Google (MIT)\n");
  printf("- Dear ImGui by Omar Cornut (MIT)\n");
#ifdef HAVE_FREETYPE
  printf("- FreeType (GPLv2)\n");
#endif
  printf("- Portable File Dialogs by Sam Hocevar (WTFPL)\n");
  printf("- Native File Dialog (modified version) by Frogtoss Games (zlib license)\n");
  printf("- FFTW by Matteo Frigo and Steven G. Johnson (GPLv2)\n");
  printf("- Nuked-OPM by nukeykt (LGPLv2.1)\n");
  printf("- Nuked-OPN2 by nukeykt (LGPLv2.1)\n");
  printf("- Nuked-OPL3 by nukeykt (LGPLv2.1)\n");
  printf("- Nuked-OPLL by nukeykt (GPLv2)\n");
  printf("- Nuked-PSG (modified version) by nukeykt (GPLv2)\n");
  printf("- YM3812-LLE by nukeykt (GPLv2)\n");
  printf("- YMF262-LLE by nukeykt (GPLv2)\n");
  printf("- YMF276-LLE by nukeykt (GPLv2)\n");
  printf("- YM2608-LLE by nukeykt (GPLv2)\n");
  printf("- ESFMu (modified version) by Kagamiin~ (LGPLv2.1)\n");
  printf("- ymfm by Aaron Giles (BSD 3-clause)\n");
  printf("- emu2413 by Digital Sound Antiques (MIT)\n");
  printf("- adpcm by superctr (public domain)\n");
  printf("- adpcm-xq by David Bryant (BSD 3-clause)\n");
  printf("- MAME SN76496 emulation core by Nicola Salmoria (BSD 3-clause)\n");
  printf("- MAME AY-3-8910 emulation core by Couriersud (BSD 3-clause)\n");
  printf("- MAME SAA1099 emulation core by Juergen Buchmueller and Manuel Abadia (BSD 3-clause)\n");
  printf("- MAME Namco WSG by Nicola Salmoria and Aaron Giles (BSD 3-clause)\n");
  printf("- MAME RF5C68 core by Olivier Galibert and Aaron Giles (BSD 3-clause)\n");
  printf("- MAME MSM5232 core by Jarek Burczynski and Hiromitsu Shioya (GPLv2)\n");
  printf("- MAME MSM6258 core by Barry Rodewald (BSD 3-clause)\n");
  printf("- MAME YMZ280B core by Aaron Giles (BSD 3-clause)\n");
  printf("- MAME GA20 core (modified version) by Acho A. Tang, R. Belmont and Valley Bell (BSD 3-clause)\n");
  printf("- MAME SegaPCM core by Hiromitsu Shioya and Olivier Galibert (BSD 3-clause)\n");
  printf("- MAME µPD1771C-017 HLE core by David Viens (BSD 3-clause)\n");
  printf("- QSound core by superctr (BSD 3-clause)\n");
  printf("- VICE VIC-20 by Rami Rasanen and viznut (GPLv2)\n");
  printf("- VICE TED by Andreas Boose, Tibor Biczo and Marco van den Heuvel (GPLv2)\n");
  printf("- VERA core by Frank van den Hoef (BSD 2-clause)\n");
  printf("- SAASound by Dave Hooper and Simon Owen (BSD 3-clause)\n");
  printf("- SameBoy by Lior Halphon (MIT)\n");
  printf("- Mednafen PCE, WonderSwan and Virtual Boy by Mednafen Team (GPLv2)\n");
  printf("- Mednafen T6W28 by Blargg (GPLv2)\n");
  printf("- WonderSwan new core by asiekierka (zlib license)\n");
  printf("- SNES DSP core by Blargg (LGPLv2.1)\n");
  printf("- puNES (modified version) by FHorse (GPLv2)\n");
  printf("- NSFPlay by Brad Smith and Brezza (unknown open-source license)\n");
  printf("- reSID by Dag Lem (GPLv2)\n");
  printf("- reSIDfp by Dag Lem, Antti Lankila and Leandro Nini (GPLv2)\n");
  printf("- dSID by DefleMask Team (based on jsSID by Hermit) (MIT)\n");
  printf("- Stella by Stella Team (GPLv2)\n");
  printf("- vgsound_emu (second version, modified version) by cam900 (zlib license)\n");
  printf("- Impulse Tracker GUS volume table by Jeffrey Lim (BSD 3-clause)\n");
  printf("- Schism Tracker IT sample decompression (GPLv2)\n");
  printf("- Atari800 mzpokeysnd POKEY emulator by Michael Borisov (GPLv2)\n");
  printf("- ASAP POKEY emulator by Piotr Fusik ported to C++ by laoo (GPLv2)\n");
  printf("- SM8521 emulator (modified version) by cam900 (zlib license)\n");
  printf("- D65010G031 emulator (modified version) by cam900 (zlib license)\n");
  printf("- C140/C219 emulator (modified version) by cam900 (zlib license)\n");
  printf("- PowerNoise emulator by scratchminer (MIT)\n");
  printf("- ep128emu by Istvan Varga (GPLv2)\n");
  printf("- NDS sound emulator by cam900 (zlib license)\n");
  printf("- SID2 emulator by LTVA (GPLv2, modification of reSID emulator)\n");
  printf("- SID3 emulator by LTVA (MIT)\n");
  printf("- openMSX YMF278 emulator (modified version) by the openMSX developers (GPLv2)\n");
  printf("- Adlib-related formats import routines adapted from adlib2vgm by SudoMaker (AGPL-3.0 license)\n");
  return TA_PARAM_QUIT;
}

TAParamResult pWarranty(String) {
  printf("This program is free software; you can redistribute it and/or\n"
         "modify it under the terms of the GNU General Public License\n"
         "as published by the Free Software Foundation; either version 2\n"
         "of the License, or (at your option) any later version.\n\n"

         "This program is distributed in the hope that it will be useful,\n"
         "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
         "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
         "GNU General Public License for more details.\n\n"

         "You should have received a copy of the GNU General Public License\n"
         "along with this program; if not, write to the Free Software\n"
         "Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.\n");
  return TA_PARAM_QUIT;
}

TAParamResult pLoops(String val) {
  try {
    int count=std::stoi(val);
    if (count<0) {
      exportOptions.loops=0;
    } else {
      exportOptions.loops=count;
    }
  } catch (std::exception& e) {
    logE("loop count shall be a number.");
    return TA_PARAM_ERROR;
  }
  return TA_PARAM_SUCCESS;
}

TAParamResult pSubSong(String val) {
  try {
    int v=std::stoi(val);
    if (v<0) {
      logE("sub-song shall be 0 or higher.");
      return TA_PARAM_ERROR;
    }
    subsong=v;
  } catch (std::exception& e) {
    logE("sub-song shall be a number.");
    return TA_PARAM_ERROR;
  }
  return TA_PARAM_SUCCESS;
}

TAParamResult pOutMode(String val) {
  if (val=="one") {
    exportOptions.mode=DIV_EXPORT_MODE_ONE;
  } else if (val=="persys") {
    exportOptions.mode=DIV_EXPORT_MODE_MANY_SYS;
  } else if (val=="perchan") {
    exportOptions.mode=DIV_EXPORT_MODE_MANY_CHAN;
  } else {
    logE("invalid value for outmode! valid values are: one, persys and perchan.");
    return TA_PARAM_ERROR;
  }
  return TA_PARAM_SUCCESS;
}

TAParamResult pBenchmark(String val) {
  if (val=="render") {
    benchMode=1;
  } else if (val=="seek") {
    benchMode=2;
  } else {
    logE("invalid value for benchmark! valid values are: render and seek.");
    return TA_PARAM_ERROR;
  }
  e.setAudio(DIV_AUDIO_DUMMY);
  return TA_PARAM_SUCCESS;
}

TAParamResult pOutput(String val) {
  outName=val;
  e.setAudio(DIV_AUDIO_DUMMY);
  return TA_PARAM_SUCCESS;
}

TAParamResult pVGMOut(String val) {
  vgmOutName=val;
  e.setAudio(DIV_AUDIO_DUMMY);
  return TA_PARAM_SUCCESS;
}

TAParamResult pCmdOut(String val) {
  cmdOutName=val;
  e.setAudio(DIV_AUDIO_DUMMY);
  return TA_PARAM_SUCCESS;
}

TAParamResult pROMOut(String val) {
  romOutName=val;
  e.setAudio(DIV_AUDIO_DUMMY);
  return TA_PARAM_SUCCESS;
}

TAParamResult pROMConf(String val) {
  size_t eqSplit=val.find_first_of('=');
  if (eqSplit==String::npos) {
    logE("invalid romconf parameter, must contain '=' as in: <key>=<value>");
    return TA_PARAM_ERROR;
  }
  String key=val.substr(0,eqSplit);
  String param=val.substr(eqSplit+1);
  romExportConfig.set(key,param);
  return TA_PARAM_SUCCESS;
}

TAParamResult pTxtOut(String val) {
  txtOutName=val;
  e.setAudio(DIV_AUDIO_DUMMY);
  return TA_PARAM_SUCCESS;
}

bool needsValue(String param) {
  for (size_t i=0; i<params.size(); i++) {
    if (params[i].name==param) {
      return params[i].value;
    }
  }
  return false;
}

void initParams() {
  params.push_back(TAParam("h","help",false,pHelp,"","display this help"));

  params.push_back(TAParam("a","audio",true,pAudio,"jack|sdl|portaudio|pipe","set audio engine (SDL by default)"));
  params.push_back(TAParam("o","output",true,pOutput,"<filename>","output audio to file"));
  params.push_back(TAParam("O","vgmout",true,pVGMOut,"<filename>","output .vgm data"));
  params.push_back(TAParam("D","direct",false,pDirect,"","set VGM export direct stream mode"));
  params.push_back(TAParam("C","cmdout",true,pCmdOut,"<filename>","output command stream"));
  params.push_back(TAParam("r","romout",true,pROMOut,"<filename|path>","export ROM file, or path for multi-file export"));
  params.push_back(TAParam("R","romconf",true,pROMConf,"<key>=<value>","set configuration parameter for ROM export"));
  params.push_back(TAParam("t","txtout",true,pTxtOut,"<filename>","export as text file"));
  params.push_back(TAParam("L","loglevel",true,pLogLevel,"debug|info|warning|error","set the log level (info by default)"));
  params.push_back(TAParam("v","view",true,pView,"pattern|commands|nothing","set visualization (nothing by default)"));
  params.push_back(TAParam("i","info",false,pInfo,"","get info about a song"));
  params.push_back(TAParam("c","console",false,pConsole,"","enable console mode"));
  params.push_back(TAParam("q","noreport",false,pQuiet,"","do not display message box on error"));
  params.push_back(TAParam("n","nostatus",false,pNoStatus,"","disable playback status in console mode"));
  params.push_back(TAParam("N","nocontrols",false,pNoControls,"","disable standard input controls in console mode"));

  params.push_back(TAParam("l","loops",true,pLoops,"<count>","set number of loops"));
  params.push_back(TAParam("s","subsong",true,pSubSong,"<number>","set sub-song"));
  params.push_back(TAParam("o","outmode",true,pOutMode,"one|persys|perchan","set file output mode"));
  params.push_back(TAParam("S","safemode",false,pSafeMode,"","enable safe mode (software rendering and no audio)"));
  params.push_back(TAParam("A","safeaudio",false,pSafeModeAudio,"","enable safe mode (with audio"));

  params.push_back(TAParam("B","benchmark",true,pBenchmark,"render|seek","run performance test"));

  params.push_back(TAParam("V","version",false,pVersion,"","view information about Furnace."));
  params.push_back(TAParam("W","warranty",false,pWarranty,"","view warranty disclaimer."));
}

#ifdef _WIN32
void reportError(String what) {
  logE("%s",what);
  if (!noReportError) {
    MessageBox(NULL,what.c_str(),"Furnace",MB_OK|MB_ICONERROR);
  }
}
#elif defined(ANDROID) || defined(__APPLE__)
void reportError(String what) {
  logE("%s",what);
#ifdef HAVE_SDL2
  if (!noReportError) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error",what.c_str(),NULL);
  }
#endif
}
#else
void reportError(String what) {
  logE("%s",what);
  if (!noReportError) {
    // dummy
  }
}
#endif

#ifndef _WIN32
#ifdef HAVE_GUI
static void handleTermGUI(int) {
  g.requestQuit();
}
#endif
#endif

// TODO: CoInitializeEx on Windows?
// TODO: add crash log
int main(int argc, char** argv) {
  // Windows console thing - thanks dj.tuBIG/MaliceX
#ifdef _WIN32
#ifndef TA_SUBSYSTEM_CONSOLE
#ifndef SUPPORT_XP
  if (AttachConsole(ATTACH_PARENT_PROCESS)) {
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
    freopen("CONIN$", "r", stdin);
  }
#endif
#endif
#endif

  srand(time(NULL));

  initLog(stdout);
#ifdef _WIN32
  // set DPI awareness
  HMODULE shcore=LoadLibraryW(L"shcore.dll");
  if (shcore!=NULL) {
    SPDA ta_SetProcessDpiAwareness=(SPDA)GetProcAddress(shcore,"SetProcessDpiAwareness");
    if (ta_SetProcessDpiAwareness!=NULL) {
      HRESULT result=ta_SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
      if (result!=S_OK) {
        // ???
      }
    }
    if (!FreeLibrary(shcore)) {
      // ???
    }
  }

  // co initialize ex
  HRESULT coResult=CoInitializeEx(NULL,COINIT_MULTITHREADED);
  if (coResult!=S_OK) {
    logE("CoInitializeEx failed!");
  }
#endif
  outName="";
  vgmOutName="";
  cmdOutName="";
  romOutName="";
  txtOutName="";

  // load config for locale
  e.prePreInit();

#ifdef HAVE_LOCALE
  String reqLocale=e.getConfString("locale","");
  if (!reqLocale.empty()) {
    if (reqLocale.find(".")==String::npos) {
      reqLocale+=".UTF-8";
    }
  }
  strncpy(reqLocaleCopy,reqLocale.c_str(),63);
  if (reqLocale!="en_US.UTF-8") {
    const char* localeRet=NULL;
#ifdef HAVE_SETLOCALE
    if ((localeRet=setlocale(LC_CTYPE,reqLocaleCopy))==NULL) {
      logE("could not set locale (CTYPE)!");
      displayLocaleFailError=true;
    } else {
      logV("locale: %s",localeRet);
    }
    if ((localeRet=setlocale(LC_MESSAGES,reqLocaleCopy))==NULL) {
      logE("could not set locale (MESSAGES)!");
      displayLocaleFailError=true;
#ifdef HAVE_MOMO
      if (momo_setlocale(LC_MESSAGES,reqLocaleCopy)==NULL) {
        logV("Momo: could not set locale!");
      }
#endif
    } else {
      logV("locale: %s",localeRet);
#ifdef HAVE_MOMO
      if (momo_setlocale(LC_MESSAGES,localeRet)==NULL) {
        logV("Momo: could not set locale!");
      }
#endif
    }
#else
    if ((localeRet=momo_setlocale(LC_MESSAGES,reqLocaleCopy))==NULL) {
      logV("Momo: could not set locale!");
    } else {
      logV("locale: %s",localeRet);
    }
#endif

    char exePath[4096];
    memset(exePath,0,4096);
#ifndef ANDROID
    if (!getExePath(argv[0],exePath,4095)) memset(exePath,0,4096);
#endif

    memset(localeDir,0,4096);

    bool textDomainBound=false;
    for (int i=0; localeDirs[i]; i++) {
#ifdef ANDROID
      strncpy(localeDir,localeDirs[i],4095);
#else
      if (exePath[0]!=0 && localeDirs[i][0]!=DIR_SEPARATOR) {
        // do you NOT understand what memset IS
        char* i_s=exePath;
        for (int i_i=0; i_i<4095; i_i++) {
          localeDir[i_i]=*i_s;
          if ((*i_s)==0) break;
          i_s++;
        }
        strncat(localeDir,DIR_SEPARATOR_STR,4095);
        strncat(localeDir,localeDirs[i],4095);
      } else {
        strncpy(localeDir,localeDirs[i],4095);
      }
#endif
      logV("bind text domain: %s",localeDir);
#ifndef ANDROID
      if (!dirExists(localeDir)) continue;
#endif
      if ((localeRet=TA_BINDTEXTDOMAIN("furnace",localeDir))==NULL) {
        continue;
      } else {
        textDomainBound=true;
        logV("text domain 1: %s",localeRet);
        break;
      }
    }
    if (!textDomainBound) {
      logE("could not bind text domain!");
    } else {
      if ((localeRet=TA_TEXTDOMAIN("furnace"))==NULL) {
        logE("could not text domain!");
      } else {
        logV("text domain 2: %s",localeRet);
      }
    }
  }
#endif

  initParams();

  // parse arguments
  String arg, val, fileName;
  size_t eqSplit, argStart;
  for (int i=1; i<argc; i++) {
    arg=""; val="";
    if (argv[i][0]=='-') {
      if (argv[i][1]=='-') {
        argStart=2;
      } else {
        argStart=1;
      }
      arg=&argv[i][argStart];
      eqSplit=arg.find_first_of('=');
      if (eqSplit==String::npos) {
        if (needsValue(arg)) {
          if ((i+1)<argc) {
            val=argv[i+1];
            i++;
          } else {
            reportError(fmt::sprintf(_("incomplete param %s."),arg.c_str()));
            return 1;
          }
        }
      } else {
        val=arg.substr(eqSplit+1);
        arg=arg.substr(0,eqSplit);
      }
      for (size_t j=0; j<params.size(); j++) {
        if (params[j].name==arg || (params[j].shortName!="" && params[j].shortName==arg)) {
          switch (params[j].func(val)) {
            case TA_PARAM_ERROR:
              return 1;
              break;
            case TA_PARAM_SUCCESS:
              break;
            case TA_PARAM_QUIT:
              return 0;
              break;
          }
          break;
        }
      }
    } else {
      fileName=argv[i];
    }
  }

  e.setConsoleMode(consoleMode,!consoleNoStatus);

#ifdef _WIN32
  if (consoleMode) {
    HANDLE winin=GetStdHandle(STD_INPUT_HANDLE);
    HANDLE winout=GetStdHandle(STD_OUTPUT_HANDLE);
    int termprop=0;
    int termpropi=0;
    GetConsoleMode(winout,(LPDWORD)&termprop);
    GetConsoleMode(winin,(LPDWORD)&termpropi);
    termprop|=ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    termpropi&=~ENABLE_LINE_INPUT;
    SetConsoleMode(winout,termprop);
    SetConsoleMode(winin,termpropi);
  }
#endif

  if (fileName.empty() && consoleMode) {
    logI("usage: %s file",argv[0]);
    return 1;
  }

  const bool outputMode = outName!="" || vgmOutName!="" || cmdOutName!="" || romOutName!="" || txtOutName!="";

  if (fileName.empty() && (benchMode || infoMode || outputMode)) {
    logE("provide a file!");
    return 1;
  }

#if defined(HAVE_SDL2) && defined(_WIN32) && !defined(SUPPORT_XP)
  if (!IsWindows7OrGreater()) {
    SDL_SetHint("SDL_HINT_AUDIODRIVER","winmm");
  }
#endif

#ifdef HAVE_GUI
  if (e.preInit(consoleMode || benchMode || infoMode || outputMode)) {
    if (consoleMode || benchMode || infoMode || outputMode) {
      logW("engine wants safe mode, but Furnace GUI is not going to start.");
    } else {
      safeMode=true;
    }
  }
#else
  if (e.preInit(true)) {
    logW("engine wants safe mode, but Furnace GUI is not available.");
  }
#endif

  if (safeMode && (consoleMode || benchMode || infoMode || outputMode)) {
    logE("you can't use safe mode and console/export mode together.");
    return 1;
  }

  if (safeMode && !safeModeWithAudio) {
    e.setAudio(DIV_AUDIO_DUMMY);
  }

#if defined(HAVE_SDL2) && defined(ANDROID)
  if (e.getConfInt("backgroundPlay",0)!=0) {
    SDL_SetHint(SDL_HINT_ANDROID_BLOCK_ON_PAUSE,"0");
    SDL_SetHint(SDL_HINT_ANDROID_BLOCK_ON_PAUSE_PAUSEAUDIO,"0");
  }
#endif

  if (!fileName.empty() && ((!e.getConfBool("tutIntroPlayed",TUT_INTRO_PLAYED)) || e.getConfInt("alwaysPlayIntro",0)!=3 || consoleMode || benchMode || infoMode || outputMode)) {
    logI("loading module...");
    FILE* f=ps_fopen(fileName.c_str(),"rb");
    if (f==NULL) {
      reportError(fmt::sprintf(_("couldn't open file! (%s)"),strerror(errno)));
      e.everythingOK();
      finishLogFile();
      return 1;
    }
    if (fseek(f,0,SEEK_END)<0) {
      reportError(fmt::sprintf(_("couldn't open file! (couldn't get file size: %s)"),strerror(errno)));
      e.everythingOK();
      fclose(f);
      finishLogFile();
      return 1;
    }
    ssize_t len=ftell(f);
    if (len==(SIZE_MAX>>1)) {
      reportError(fmt::sprintf(_("couldn't open file! (couldn't get file length: %s)"),strerror(errno)));
      e.everythingOK();
      fclose(f);
      finishLogFile();
      return 1;
    }
    if (len<1) {
      if (len==0) {
        reportError(_("that file is empty!"));
      } else {
        reportError(fmt::sprintf(_("couldn't open file! (tell error: %s)"),strerror(errno)));
      }
      e.everythingOK();
      fclose(f);
      finishLogFile();
      return 1;
    }
    unsigned char* file=new unsigned char[len];
    if (fseek(f,0,SEEK_SET)<0) {
      reportError(fmt::sprintf(_("couldn't open file! (size error: %s)"),strerror(errno)));
      e.everythingOK();
      fclose(f);
      delete[] file;
      finishLogFile();
      return 1;
    }
    if (fread(file,1,(size_t)len,f)!=(size_t)len) {
      reportError(fmt::sprintf(_("couldn't open file! (read error: %s)"),strerror(errno)));
      e.everythingOK();
      fclose(f);
      delete[] file;
      finishLogFile();
      return 1;
    }
    fclose(f);
    if (!e.load(file,(size_t)len,fileName.c_str())) {
      reportError(fmt::sprintf(_("could not open file! (%s)"),e.getLastError()));
      e.everythingOK();
      finishLogFile();
      return 1;
    }
  }
  if (infoMode) {
    e.dumpSongInfo();
    finishLogFile();
    return 0;
  }

  if (!e.init()) {
    if (consoleMode) {
      reportError(_("could not initialize engine!"));
      finishLogFile();
      return 1;
    } else {
      logE("could not initialize engine!");
      displayEngineFailError=true;
    }
  }

  if (subsong!=-1) {
    e.changeSongP(subsong);
  }

  if (benchMode) {
    logI("starting benchmark!");
    if (benchMode==2) {
      e.benchmarkSeek();
    } else {
      e.benchmarkPlayback();
    }
    finishLogFile();
    return 0;
  }

  if (outputMode) {
    if (cmdOutName!="") {
      SafeWriter* w=e.saveCommand(NULL);
      if (w!=NULL) {
        FILE* f=ps_fopen(cmdOutName.c_str(),"wb");
        if (f!=NULL) {
          fwrite(w->getFinalBuf(),1,w->size(),f);
          fclose(f);
        } else {
          reportError(fmt::sprintf(_("could not open file! (%s)"),strerror(errno)));
        }
        w->finish();
        delete w;
      } else {
        reportError(_("could not write command stream!"));
      }
    }
    if (vgmOutName!="") {
      SafeWriter* w=e.saveVGM(NULL,true,0x171,false,vgmOutDirect);
      if (w!=NULL) {
        FILE* f=ps_fopen(vgmOutName.c_str(),"wb");
        if (f!=NULL) {
          fwrite(w->getFinalBuf(),1,w->size(),f);
          fclose(f);
        } else {
          reportError(fmt::sprintf(_("could not open file! (%s)"),strerror(errno)));
        }
        w->finish();
        delete w;
      } else {
        reportError(_("could not write VGM!"));
      }
    }
    if (outName!="") {
      e.setConsoleMode(true);
      e.saveAudio(outName.c_str(),exportOptions);
      e.waitAudioFile();
    }
    if (romOutName!="") {
      e.setConsoleMode(true);
      // select ROM target type
      DivROMExportOptions romTarget = DIV_ROM_ABSTRACT;
      String lowerCase=romOutName;
      for (char& i: lowerCase) {
        if (i>='A' && i<='Z') i+='a'-'A';
      }
      for (int i=0; i<DIV_ROM_MAX; i++) {
        DivROMExportOptions opt = (DivROMExportOptions)i;
        if (e.isROMExportViable(opt)) {
          const DivROMExportDef* newDef=e.getROMExportDef((DivROMExportOptions)i);
          if (newDef->fileExt &&
              lowerCase.length()>=strlen(newDef->fileExt) &&
              lowerCase.substr(lowerCase.length()-strlen(newDef->fileExt))==newDef->fileExt) {
            romTarget = opt;
            break; // extension matched, stop searching
          }
          if (romTarget == DIV_ROM_ABSTRACT) {
            romTarget = opt; // use first viable, but keep searching for extension match
          }
        }
      }
      if (romTarget > DIV_ROM_ABSTRACT && romTarget < DIV_ROM_MAX) {
        DivROMExport* pendingExport = e.buildROM(romTarget);
        if (pendingExport==NULL) {
          reportError(_("could not create exporter! you may want to report this issue..."));
        } else {
          pendingExport->setConf(romExportConfig);
          if (pendingExport->go(&e)) {
            pendingExport->wait();
            if (!pendingExport->hasFailed()) {
              for (DivROMExportOutput& i: pendingExport->getResult()) {
                String path=romOutName;
                if (e.getROMExportDef(romTarget)->multiOutput) {
                  path+=DIR_SEPARATOR_STR;
                  path+=i.name;
                }
                FILE* f=ps_fopen(path.c_str(),"wb");
                if (f!=NULL) {
                  fwrite(i.data->getFinalBuf(),1,i.data->size(),f);
                  fclose(f);
                } else {
                  reportError(fmt::sprintf(_("could not open file! (%s)"),strerror(errno)));
                }
              }
            } else {
              reportError(fmt::sprintf(_("ROM export failed! (%s)"),e.getLastError()));
            }
          } else {
            reportError(_("could not begin exporting process! TODO: elaborate"));
          }
        }
      } else {
        reportError(_("no matching ROM export target is available."));
      }
    }
    if (txtOutName!="") {
      e.setConsoleMode(true);
      SafeWriter* w=e.saveText(false);
      if (w!=NULL) {
        FILE* f=ps_fopen(txtOutName.c_str(),"wb");
        if (f!=NULL) {
          fwrite(w->getFinalBuf(),1,w->size(),f);
          fclose(f);
        } else {
          reportError(fmt::sprintf(_("could not open file! (%s)"),strerror(errno)));
        }
        w->finish();
        delete w;
      } else {
        reportError(_("could not write text!"));
      }
    }
    finishLogFile();
    return 0;
  }

  if (consoleMode) {
    bool cliSuccess=false;
    if (consoleNoStatus) {
      cli.noStatus();
    }
    if (consoleNoControls) {
      cli.noControls();
    }
    cli.bindEngine(&e);
    if (!cli.init()) {
      reportError(_("error while starting CLI!"));
    } else {
      cliSuccess=true;
    }
    logI(_("playing..."));
    e.play();
    if (cliSuccess) {
      cli.loop();
      cli.finish();
      e.quit();
      finishLogFile();
      return 0;
    } else {
#ifdef HAVE_SDL2
      SDL_Event ev;
      while (true) {
        SDL_WaitEvent(&ev);
        if (ev.type==SDL_QUIT) break;
      }
      e.quit();
      finishLogFile();
      return 0;
#else
      while (true) {
#ifdef _WIN32
        Sleep(500);
#else
        usleep(500000);
#endif
      }
#endif
    }
  }

#ifdef HAVE_GUI
  if (safeMode) g.enableSafeMode();
  g.bindEngine(&e);
  if (!g.init()) {
    reportError(g.getLastError());
    finishLogFile();
    e.everythingOK();
    return 1;
  }

  if (displayEngineFailError) {
    logE(_("displaying engine fail error."));
    g.showError(_("error while initializing audio!"));
  }

  if (displayLocaleFailError) {
#ifndef HAVE_MOMO
#ifdef __unix__
    g.showError("could not load language!\napparently your system does not support this language correctly.\nmake sure you've generated language data by editing /etc/locale.gen\nand then running locale-gen as root.");
#else
    g.showError("could not load language!\nthis is a bug!");
#endif
#endif
  }

  if (!fileName.empty()) {
    g.setFileName(fileName);
  }

#ifndef _WIN32
  sigemptyset(&termsa.sa_mask);
  termsa.sa_flags=0;
  termsa.sa_handler=handleTermGUI;
  sigaction(SIGTERM,&termsa,NULL);
#endif

  g.loop();
  logI("closing GUI.");
  g.finish(true);
#else
  logE("GUI requested but GUI not compiled!");
#endif

  logI("stopping engine.");
  e.quit(false);

  finishLogFile();

#ifdef _WIN32
  if (coResult==S_OK || coResult==S_FALSE) {
    CoUninitialize();
  }
#endif
  e.everythingOK();
  return 0;
}
