/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2023 tildearrow and contributors
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
#include <string>
#ifdef HAVE_SDL2
#include "SDL_events.h"
#endif
#include "ta-log.h"
#include "fileutils.h"
#include "engine/engine.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <combaseapi.h>
#include <shellapi.h>

#include "gui/shellScalingStub.h"

typedef HRESULT (WINAPI *SPDA)(PROCESS_DPI_AWARENESS);
#else
#include <unistd.h>
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
String zsmOutName;
String cmdOutName;
int loops=1;
int benchMode=0;
DivAudioExportModes outMode=DIV_EXPORT_MODE_ONE;

#ifdef HAVE_GUI
bool consoleMode=false;
#else
bool consoleMode=true;
#endif

bool displayEngineFailError=false;
bool cmdOutBinary=false;
bool vgmOutDirect=false;

std::vector<TAParam> params;

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
  } else {
    logE("invalid value for audio engine! valid values are: jack, sdl.");
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

TAParamResult pBinary(String val) {
  cmdOutBinary=true;
  return TA_PARAM_SUCCESS;
}

TAParamResult pDirect(String val) {
  vgmOutDirect=true;
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
  printf("copyright (C) 2021-2023 tildearrow and contributors.\n");
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
  printf("- RtMidi by Gary P. Scavone (RtMidi license)\n");
  printf("- backward-cpp by Google (MIT)\n");
  printf("- Dear ImGui by Omar Cornut (MIT)\n");
  printf("- Portable File Dialogs by Sam Hocevar (WTFPL)\n");
  printf("- Native File Dialog (modified version) by Frogtoss Games (zlib license)\n");
  printf("- FFTW by Matteo Frigo and Steven G. Johnson (GPLv2)\n");
  printf("- Nuked-OPM by Nuke.YKT (LGPLv2.1)\n");
  printf("- Nuked-OPN2 by Nuke.YKT (LGPLv2.1)\n");
  printf("- Nuked-OPL3 by Nuke.YKT (LGPLv2.1)\n");
  printf("- Nuked-OPLL by Nuke.YKT (GPLv2)\n");
  printf("- Nuked-PSG (modified version) by Nuke.YKT (GPLv2)\n");
  printf("- ymfm by Aaron Giles (BSD 3-clause)\n");
  printf("- adpcm by superctr (public domain)\n");
  printf("- MAME SN76496 emulation core by Nicola Salmoria (BSD 3-clause)\n");
  printf("- MAME AY-3-8910 emulation core by Couriersud (BSD 3-clause)\n");
  printf("- MAME SAA1099 emulation core by Juergen Buchmueller and Manuel Abadia (BSD 3-clause)\n");
  printf("- MAME Namco WSG by Nicola Salmoria and Aaron Giles (BSD 3-clause)\n");
  printf("- MAME RF5C68 core by Olivier Galibert and Aaron Giles (BSD 3-clause)\n");
  printf("- MAME MSM5232 core by Jarek Burczynski and Hiromitsu Shioya (GPLv2)\n");
  printf("- MAME MSM6258 core by Barry Rodewald (BSD 3-clause)\n");
  printf("- MAME YMZ280B core by Aaron Giles (BSD 3-clause)\n");
  printf("- MAME GA20 core by Acho A. Tang and R. Belmont (BSD 3-clause)\n");
  printf("- MAME SegaPCM core by Hiromitsu Shioya and Olivier Galibert (BSD 3-clause)\n");
  printf("- QSound core by superctr (BSD 3-clause)\n");
  printf("- VICE VIC-20 by Rami Rasanen and viznut (GPLv2)\n");
  printf("- VERA core by Frank van den Hoef (BSD 2-clause)\n");
  printf("- SAASound by Dave Hooper and Simon Owen (BSD 3-clause)\n");
  printf("- SameBoy by Lior Halphon (MIT)\n");
  printf("- Mednafen PCE, WonderSwan and Virtual Boy by Mednafen Team (GPLv2)\n");
  printf("- Mednafen T6W28 by Blargg (GPLv2)\n");
  printf("- SNES DSP core by Blargg (LGPLv2.1)\n");
  printf("- puNES by FHorse (GPLv2)\n");
  printf("- NSFPlay by Brad Smith and Brezza (unknown open-source license)\n");
  printf("- reSID by Dag Lem (GPLv2)\n");
  printf("- reSIDfp by Dag Lem, Antti Lankila and Leandro Nini (GPLv2)\n");
  printf("- Stella by Stella Team (GPLv2)\n");
  printf("- vgsound_emu (second version, modified version) by cam900 (zlib license)\n");
  printf("- MAME GA20 core by Acho A. Tang, R. Belmont, Valley Bell (BSD 3-clause)\n");
  printf("- Atari800 mzpokeysnd POKEY emulator by Michael Borisov (GPLv2)\n");
  printf("- ASAP POKEY emulator by Piotr Fusik ported to C++ by laoo (GPLv2)\n");
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
      loops=0;
    } else {
      loops=count+1;
    }
  } catch (std::exception& e) {
    logE("loop count shall be a number.");
    return TA_PARAM_ERROR;
  }
  return TA_PARAM_SUCCESS;
}

TAParamResult pOutMode(String val) {
  if (val=="one") {
    outMode=DIV_EXPORT_MODE_ONE;
  } else if (val=="persys") {
    outMode=DIV_EXPORT_MODE_MANY_SYS;
  } else if (val=="perchan") {
    outMode=DIV_EXPORT_MODE_MANY_CHAN;
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

TAParamResult pZSMOut(String val) {
  zsmOutName=val;
  e.setAudio(DIV_AUDIO_DUMMY);
  return TA_PARAM_SUCCESS;
}

TAParamResult pCmdOut(String val) {
  cmdOutName=val;
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

  params.push_back(TAParam("a","audio",true,pAudio,"jack|sdl","set audio engine (SDL by default)"));
  params.push_back(TAParam("o","output",true,pOutput,"<filename>","output audio to file"));
  params.push_back(TAParam("O","vgmout",true,pVGMOut,"<filename>","output .vgm data"));
  params.push_back(TAParam("D","direct",false,pDirect,"","set VGM export direct stream mode"));
  params.push_back(TAParam("Z","zsmout",true,pZSMOut,"<filename>","output .zsm data for Commander X16 Zsound"));
  params.push_back(TAParam("C","cmdout",true,pCmdOut,"<filename>","output command stream"));
  params.push_back(TAParam("b","binary",false,pBinary,"","set command stream output format to binary"));
  params.push_back(TAParam("L","loglevel",true,pLogLevel,"debug|info|warning|error","set the log level (info by default)"));
  params.push_back(TAParam("v","view",true,pView,"pattern|commands|nothing","set visualization (pattern by default)"));
  params.push_back(TAParam("c","console",false,pConsole,"","enable console mode"));

  params.push_back(TAParam("l","loops",true,pLoops,"<count>","set number of loops (-1 means loop forever)"));
  params.push_back(TAParam("o","outmode",true,pOutMode,"one|persys|perchan","set file output mode"));

  params.push_back(TAParam("B","benchmark",true,pBenchmark,"render|seek","run performance test"));

  params.push_back(TAParam("V","version",false,pVersion,"","view information about Furnace."));
  params.push_back(TAParam("W","warranty",false,pWarranty,"","view warranty disclaimer."));
}

#ifdef _WIN32
void reportError(String what) {
  logE("%s",what);
  MessageBox(NULL,what.c_str(),"Furnace",MB_OK|MB_ICONERROR);
}
#elif defined(ANDROID)
void reportError(String what) {
  logE("%s",what);
#ifdef HAVE_SDL2
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error",what.c_str(),NULL);
#endif
}
#else
void reportError(String what) {
  logE("%s",what);
}
#endif

// TODO: CoInitializeEx on Windows?
// TODO: add crash log
int main(int argc, char** argv) {
  // Windows console thing - thanks dj.tuBIG/MaliceX
#ifdef _WIN32

  if (AttachConsole(ATTACH_PARENT_PROCESS)) {
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
    freopen("CONIN$", "r", stdin);
  }
#endif

  srand(time(NULL));

  initLog();
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
  zsmOutName="";
  cmdOutName="";

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
            reportError(fmt::sprintf("incomplete param %s.",arg.c_str()));
            return 1;
          }
        }
      } else {
        val=arg.substr(eqSplit+1);
        arg=arg.substr(0,eqSplit);
      }
      for (size_t j=0; j<params.size(); j++) {
        if (params[j].name==arg || params[j].shortName==arg) {
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

  e.setConsoleMode(consoleMode);

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

  e.preInit();

  if (!fileName.empty() && ((!e.getConfBool("tutIntroPlayed",false)) || e.getConfInt("alwaysPlayIntro",0)!=3 || consoleMode || benchMode || outName!="" || vgmOutName!="" || cmdOutName!="")) {
    logI("loading module...");
    FILE* f=ps_fopen(fileName.c_str(),"rb");
    if (f==NULL) {
      reportError(fmt::sprintf("couldn't open file! (%s)",strerror(errno)));
      finishLogFile();
      return 1;
    }
    if (fseek(f,0,SEEK_END)<0) {
      reportError(fmt::sprintf("couldn't open file! (couldn't get file size: %s)",strerror(errno)));
      fclose(f);
      finishLogFile();
      return 1;
    }
    ssize_t len=ftell(f);
    if (len==(SIZE_MAX>>1)) {
      reportError(fmt::sprintf("couldn't open file! (couldn't get file length: %s)",strerror(errno)));
      fclose(f);
      finishLogFile();
      return 1;
    }
    if (len<1) {
      if (len==0) {
        reportError("that file is empty!");
      } else {
        reportError(fmt::sprintf("couldn't open file! (tell error: %s)",strerror(errno)));
      }
      fclose(f);
      finishLogFile();
      return 1;
    }
    unsigned char* file=new unsigned char[len];
    if (fseek(f,0,SEEK_SET)<0) {
      reportError(fmt::sprintf("couldn't open file! (size error: %s)",strerror(errno)));
      fclose(f);
      delete[] file;
      finishLogFile();
      return 1;
    }
    if (fread(file,1,(size_t)len,f)!=(size_t)len) {
      reportError(fmt::sprintf("couldn't open file! (read error: %s)",strerror(errno)));
      fclose(f);
      delete[] file;
      finishLogFile();
      return 1;
    }
    fclose(f);
    if (!e.load(file,(size_t)len)) {
      reportError(fmt::sprintf("could not open file! (%s)",e.getLastError()));
      finishLogFile();
      return 1;
    }
  }
  if (!e.init()) {
    if (consoleMode) {
      reportError("could not initialize engine!");
      finishLogFile();
      return 1;
    } else {
      logE("could not initialize engine!");
      displayEngineFailError=true;
    }
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
  if (outName!="" || vgmOutName!="" || cmdOutName!="") {
    if (cmdOutName!="") {
      SafeWriter* w=e.saveCommand(cmdOutBinary);
      if (w!=NULL) {
        FILE* f=fopen(cmdOutName.c_str(),"wb");
        if (f!=NULL) {
          fwrite(w->getFinalBuf(),1,w->size(),f);
          fclose(f);
        } else {
          reportError(fmt::sprintf("could not open file! (%s)",e.getLastError()));
        }
        w->finish();
        delete w;
      } else {
        reportError("could not write command stream!");
      }
    }
    if (vgmOutName!="") {
      SafeWriter* w=e.saveVGM(NULL,true,0x171,false,vgmOutDirect);
      if (w!=NULL) {
        FILE* f=fopen(vgmOutName.c_str(),"wb");
        if (f!=NULL) {
          fwrite(w->getFinalBuf(),1,w->size(),f);
          fclose(f);
        } else {
          reportError(fmt::sprintf("could not open file! (%s)",e.getLastError()));
        }
        w->finish();
        delete w;
      } else {
        reportError("could not write VGM!");
      }
    }
    if (outName!="") {
      e.setConsoleMode(true);
      e.saveAudio(outName.c_str(),loops,outMode);
      e.waitAudioFile();
    }
    finishLogFile();
    return 0;
  }

  if (consoleMode) {
    bool cliSuccess=false;
    cli.bindEngine(&e);
    if (!cli.init()) {
      reportError("error while starting CLI!");
    } else {
      cliSuccess=true;
    }
    logI("playing...");
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
  g.bindEngine(&e);
  if (!g.init()) {
    reportError(g.getLastError());
    finishLogFile();
    return 1;
  }

  if (displayEngineFailError) {
    logE("displaying engine fail error.");
    g.showError("error while initializing audio!");
  }

  if (!fileName.empty()) {
    g.setFileName(fileName);
  }

  g.loop();
  logI("closing GUI.");
  g.finish();
#else
  logE("GUI requested but GUI not compiled!");
#endif

  logI("stopping engine.");
  e.quit();

  finishLogFile();

#ifdef _WIN32
  if (coResult==S_OK || coResult==S_FALSE) {
    CoUninitialize();
  }
#endif
  return 0;
}
