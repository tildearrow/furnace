/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2022 tildearrow and contributors
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
#ifdef HAVE_GUI
#include "SDL_events.h"
#endif
#include "ta-log.h"
#include "fileutils.h"
#include "engine/engine.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#else
#include <unistd.h>
#endif

#ifdef HAVE_GUI
#include "gui/gui.h"
#endif

DivEngine e;

#ifdef HAVE_GUI
FurnaceGUI g;
#endif

String outName;
String vgmOutName;
int loops=1;
DivAudioExportModes outMode=DIV_EXPORT_MODE_ONE;

#ifdef HAVE_GUI
bool consoleMode=false;
#else
bool consoleMode=true;
#endif

bool displayEngineFailError=false;

std::vector<TAParam> params;

bool pHelp(String) {
  printf("usage: furnace [params] [filename]\n"
         "you may specify the following parameters:\n");
  for (auto& i: params) {
    if (i.value) {
      printf("  -%s %s: %s\n",i.name.c_str(),i.valName.c_str(),i.desc.c_str());
    } else {
      printf("  -%s: %s\n",i.name.c_str(),i.desc.c_str());
    }
  }
  return false;
}

bool pAudio(String val) {
  if (outName!="") {
    logE("can't use -audio and -output at the same time.\n");
    return false;
  }
  if (val=="jack") {
    e.setAudio(DIV_AUDIO_JACK);
  } else if (val=="sdl") {
    e.setAudio(DIV_AUDIO_SDL);
  } else {
    logE("invalid value for audio engine! valid values are: jack, sdl.\n");
    return false;
  }
  return true;
}

bool pView(String val) {
  if (val=="pattern") {
    e.setView(DIV_STATUS_PATTERN);
  } else if (val=="commands") {
    e.setView(DIV_STATUS_COMMANDS);
  } else if (val=="nothing") {
    e.setView(DIV_STATUS_NOTHING);
  } else {
    logE("invalid value for view type! valid values are: pattern, commands, nothing.\n");
    return false;
  }
  return true;
}

bool pConsole(String val) {
  consoleMode=true;
  return true;
}

bool pLogLevel(String val) {
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
    logE("invalid value for loglevel! valid values are: trace, debug, info, warning, error.\n");
    return false;
  }
  return true;
}

bool pVersion(String) {
  printf("Furnace version " DIV_VERSION ".\n\n");
  printf("copyright (C) 2021-2022 tildearrow and contributors.\n");
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
  printf("- Dear ImGui by Omar Cornut (MIT)\n");
  printf("- Nuked-OPM by Nuke.YKT (LGPLv2.1)\n");
  printf("- Nuked-OPN2 by Nuke.YKT (LGPLv2.1)\n");
  printf("- ymfm by Aaron Giles (BSD 3-clause)\n");
  printf("- MAME SN76496 emulation core by Nicola Salmoria (BSD 3-clause)\n");
  printf("- MAME AY-3-8910 emulation core by Couriersud (BSD 3-clause)\n");
  printf("- MAME SAA1099 emulation core by Juergen Buchmueller and Manuel Abadia (BSD 3-clause)\n");
  printf("- SAASound (BSD 3-clause)\n");
  printf("- SameBoy by Lior Halphon (MIT)\n");
  printf("- Mednafen PCE by Mednafen Team (GPLv2)\n");
  printf("- puNES by FHorse (GPLv2)\n");
  printf("- reSID by Dag Lem (GPLv2)\n");
  printf("- Stella by Stella Team (GPLv2)\n");
  return false;
}

bool pWarranty(String) {
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
  return false;
}

bool pLoops(String val) {
  try {
    int count=std::stoi(val);
    if (count<0) {
      loops=0;
    } else {
      loops=count+1;
    }
  } catch (std::exception& e) {
    logE("loop count shall be a number.\n");
    return false;
  }
  return true;
}

bool pOutMode(String val) {
  if (val=="one") {
    outMode=DIV_EXPORT_MODE_ONE;
  } else if (val=="persys") {
    outMode=DIV_EXPORT_MODE_MANY_SYS;
  } else if (val=="perchan") {
    outMode=DIV_EXPORT_MODE_MANY_CHAN;
  } else {
    logE("invalid value for outmode! valid values are: one, persys and perchan.\n");
    return false;
  }
  return true;
}

bool pOutput(String val) {
  outName=val;
  e.setAudio(DIV_AUDIO_DUMMY);
  return true;
}

bool pVGMOut(String val) {
  vgmOutName=val;
  e.setAudio(DIV_AUDIO_DUMMY);
  return true;
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
  params.push_back(TAParam("L","loglevel",true,pLogLevel,"debug|info|warning|error","set the log level (info by default)"));
  params.push_back(TAParam("v","view",true,pView,"pattern|commands|nothing","set visualization (pattern by default)"));
  params.push_back(TAParam("c","console",false,pConsole,"","enable console mode"));

  params.push_back(TAParam("l","loops",true,pLoops,"<count>","set number of loops (-1 means loop forever)"));
  params.push_back(TAParam("o","outmode",true,pOutMode,"one|persys|perchan","set file output mode"));

  params.push_back(TAParam("V","version",false,pVersion,"","view information about Furnace."));
  params.push_back(TAParam("W","warranty",false,pWarranty,"","view warranty disclaimer."));
}

int main(int argc, char** argv) {
#if !(defined(__APPLE__) || defined(_WIN32))
  // workaround for Wayland HiDPI issue
  if (getenv("SDL_VIDEODRIVER")==NULL) {
    setenv("SDL_VIDEODRIVER","x11",1);
  }
#endif
  outName="";
  vgmOutName="";

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
            logE("incomplete param %s.\n",arg.c_str());
            return 1;
          }
        }
      } else {
        val=arg.substr(eqSplit+1);
        arg=arg.substr(0,eqSplit);
      }
      for (size_t j=0; j<params.size(); j++) {
        if (params[j].name==arg || params[j].shortName==arg) {
          if (!params[j].func(val)) return 1;
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
    logI("usage: %s file\n",argv[0]);
    return 1;
  }
  logI("Furnace version " DIV_VERSION ".\n");
  if (!fileName.empty()) {
    logI("loading module...\n");
    FILE* f=ps_fopen(fileName.c_str(),"rb");
    if (f==NULL) {
      perror("error");
      return 1;
    }
    if (fseek(f,0,SEEK_END)<0) {
      perror("size error");
      fclose(f);
      return 1;
    }
    ssize_t len=ftell(f);
    if (len==(SIZE_MAX>>1)) {
      perror("could not get file length");
      fclose(f);
      return 1;
    }
    if (len<1) {
      if (len==0) {
        printf("that file is empty!\n");
      } else {
        perror("tell error");
      }
      fclose(f);
      return 1;
    }
    unsigned char* file=new unsigned char[len];
    if (fseek(f,0,SEEK_SET)<0) {
      perror("size error");
      fclose(f);
      delete[] file;
      return 1;
    }
    if (fread(file,1,(size_t)len,f)!=(size_t)len) {
      perror("read error");
      fclose(f);
      delete[] file;
      return 1;
    }
    fclose(f);
    if (!e.load(file,(size_t)len)) {
      logE("could not open file!\n");
      return 1;
    }
  }
  if (!e.init()) {
    logE("could not initialize engine!\n");
    if (consoleMode) {
      return 1;
    } else {
      displayEngineFailError=true;
    }
  }
  if (outName!="" || vgmOutName!="") {
    if (vgmOutName!="") {
      SafeWriter* w=e.saveVGM();
      if (w!=NULL) {
        FILE* f=fopen(vgmOutName.c_str(),"wb");
        if (f!=NULL) {
          fwrite(w->getFinalBuf(),1,w->size(),f);
          fclose(f);
        } else {
          logE("could not open file! %s\n",strerror(errno));
        }
        w->finish();
        delete w;
      } else {
        logE("could not write VGM!\n");
      }
    }
    if (outName!="") {
      e.setConsoleMode(true);
      e.saveAudio(outName.c_str(),loops,outMode);
      e.waitAudioFile();
    }
    return 0;
  }

  if (consoleMode) {
    logI("playing...\n");
    e.play();
#ifdef HAVE_GUI
    SDL_Event ev;
    while (true) {
      SDL_WaitEvent(&ev);
      if (ev.type==SDL_QUIT) break;
    }
    e.quit();
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

#ifdef HAVE_GUI
  g.bindEngine(&e);
  if (!g.init()) return 1;

  if (displayEngineFailError) {
    logE("displaying engine fail error.\n");
    g.showError("error while initializing audio!");
  }

  if (!fileName.empty()) {
    g.setFileName(fileName);
  }

  g.loop();
  logI("closing GUI.\n");
  g.finish();
#else
  logE("GUI requested but GUI not compiled!\n");
#endif

  logI("stopping engine.\n");
  e.quit();
  return 0;
}

#ifdef _WIN32
#include "winMain.cpp"
#endif
