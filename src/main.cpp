#include <exception>
#include <stdio.h>
#include <string>
#include "ta-log.h"
#include "engine/engine.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

#define DIV_VERSION "dev8"

DivEngine e;

String outName;

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

bool pLogLevel(String val) {
  if (val=="debug") {
    logLevel=LOGLEVEL_DEBUG;
  } else if (val=="info") {
    logLevel=LOGLEVEL_INFO;
  } else if (val=="warning") {
    logLevel=LOGLEVEL_WARN;
  } else if (val=="error") {
    logLevel=LOGLEVEL_ERROR;
  } else {
    logE("invalid value for loglevel! valid values are: debug, info, warning, error.\n");
    return false;
  }
  return true;
}

bool pVersion(String) {
  printf("Furnace version " DIV_VERSION ".\n\n");
  printf("developed by tildearrow. copyright (C) 2021.\n");
  printf("licensed under the GNU General Public License version 2\n");
  printf("<https://www.gnu.org/licenses/old-licenses/gpl-2.0.html>.\n\n");
  printf("this is free software with ABSOLUTELY NO WARRANTY.\n");
  printf("pass the -warranty parameter for more information.\n\n");
  printf("DISCLAIMER: this program is not affiliated with Delek in any form.\n");
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
      e.setLoops(-1);
    } else {
      e.setLoops(count+1);
    }
  } catch (std::exception& e) {
    logE("loop count shall be a number.\n");
    return false;
  }
  return true;
}

bool pOutput(String val) {
  outName=val;
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
  params.push_back(TAParam("L","loglevel",true,pLogLevel,"debug|info|warning|error","set the log level (info by default)"));
  params.push_back(TAParam("v","view",true,pView,"pattern|commands|nothing","set visualization (pattern by default)"));

  params.push_back(TAParam("l","loops",true,pLoops,"<count>","set number of loops (-1 means loop forever)"));

  params.push_back(TAParam("V","version",false,pVersion,"","view information about Furnace."));
  params.push_back(TAParam("W","warranty",false,pWarranty,"","view warranty disclaimer."));
}

int main(int argc, char** argv) {
  outName="";
#ifdef _WIN32
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
      printf("fn: %s\n",fileName.c_str());
    }
  }

  if (fileName.empty()) {
    logI("usage: %s file\n",argv[0]);
    return 1;
  }
  logI("Furnace version " DIV_VERSION ".\n");
  logI("loading module...\n");
  FILE* f=fopen(fileName.c_str(),"rb");
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
  if (len==0x7fffffffffffffff) {
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
    return 1;
  }
  if (fread(file,1,(size_t)len,f)!=(size_t)len) {
    perror("read error");
    fclose(f);
    return 1;
  }
  fclose(f);
  if (!e.load((void*)file,(size_t)len)) {
    logE("could not open file!\n");
    return 1;
  }
  if (!e.init(outName)) {
    logE("could not initialize engine!\n");
    return 1;
  }
  if (outName!="") return 0;
  logI("playing...\n");
  e.play();
  while (true) {
#ifdef _WIN32
    Sleep(500);
#else
    usleep(500000);
#endif
  }
  return 0;
}
