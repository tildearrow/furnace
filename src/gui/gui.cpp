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

#define _USE_MATH_DEFINES
#include "gui.h"
#include "util.h"
#include "icon.h"
#include "../ta-log.h"
#include "../fileutils.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include "ImGuiFileDialog.h"
#include "IconsFontAwesome4.h"
#include "misc/cpp/imgui_stdlib.h"
#include "plot_nolerp.h"
#include "guiConst.h"
#include "intConst.h"
#include <stdint.h>
#include <zlib.h>
#include <fmt/printf.h>
#include <stdexcept>

#ifdef __APPLE__
extern "C" {
#include "macstuff.h"
}
#endif

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include "../utfutils.h"
#define LAYOUT_INI "\\layout.ini"
#define BACKUP_FUR "\\backup.fur"
#else
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#define LAYOUT_INI "/layout.ini"
#define BACKUP_FUR "/backup.fur"
#endif

#include "actionUtil.h"

bool Particle::update(float frameTime) {
  pos.x+=speed.x*frameTime;
  pos.y+=speed.y*frameTime;
  speed.x*=1.0-((1.0-friction)*frameTime);
  speed.y*=1.0-((1.0-friction)*frameTime);
  speed.y+=gravity*frameTime;
  life-=lifeSpeed*frameTime;
  return (life>0);
}

void FurnaceGUI::bindEngine(DivEngine* eng) {
  e=eng;
}

const char* FurnaceGUI::noteName(short note, short octave) {
  if (note==100) {
    return "OFF";
  } else if (note==101) { // note off and envelope release
    return "===";
  } else if (note==102) { // envelope release only
    return "REL";
  } else if (octave==0 && note==0) {
    return "...";
  } else if (note==0 && octave!=0) {
    return "BUG";
  }
  int seek=(note+(signed char)octave*12)+60;
  if (seek<0 || seek>=180) {
    return "???";
  }
  if (settings.germanNotation) return noteNamesG[seek];
  return noteNames[seek];
}

bool FurnaceGUI::decodeNote(const char* what, short& note, short& octave) {
  if (strlen(what)!=3) return false;
  if (strcmp(what,"...")==0) {
    note=0;
    octave=0;
    return true;
  }
  if (strcmp(what,"???")==0) {
    note=0;
    octave=0;
    return true;
  }
  if (strcmp(what,"OFF")==0) {
    note=100;
    octave=0;
    return true;
  }
  if (strcmp(what,"===")==0) {
    note=101;
    octave=0;
    return true;
  }
  if (strcmp(what,"REL")==0) {
    note=102;
    octave=0;
    return true;
  }
  for (int i=0; i<180; i++) {
    if (strcmp(what,noteNames[i])==0) {
      if ((i%12)==0) {
        note=12;
        octave=(unsigned char)((i/12)-6);
      } else {
        note=i%12;
        octave=(unsigned char)((i/12)-5);
      }
      return true;
    }
  }
  return false;
}

String FurnaceGUI::encodeKeyMap(std::map<int,int>& map) {
  String ret;
  for (std::map<int,int>::value_type& i: map) {
    ret+=fmt::sprintf("%d:%d;",i.first,i.second);
  }
  return ret;
}

void FurnaceGUI::decodeKeyMap(std::map<int,int>& map, String source) {
  map.clear();
  bool inValue=false;
  bool negateKey=false;
  bool negateValue=false;
  int key=0;
  int val=0;
  for (char& i: source) {
    switch (i) {
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        if (inValue) {
          val*=10;
          val+=i-'0';
        } else {
          key*=10;
          key+=i-'0';
        }
        break;
      case '-':
        if (inValue) {
          negateValue=true;
        } else {
          negateKey=true;
        }
        break;
      case ':':
        inValue=true;
        break;
      case ';':
        if (inValue) {
          map[negateKey?-key:key]=negateValue?-val:val;
        }
        key=0;
        val=0;
        inValue=false;
        negateKey=false;
        negateValue=false;
        break;
    }
  }
}

void FurnaceGUI::encodeMMLStr(String& target, unsigned char* macro, unsigned char macroLen, signed char macroLoop, signed char macroRel) {
  target="";
  char buf[32];
  for (int i=0; i<macroLen; i++) {
    if (i==macroLoop) target+="| ";
    if (i==macroRel) target+="/ ";
    if (i==macroLen-1) {
      snprintf(buf,31,"%d",macro[i]);
    } else {
      snprintf(buf,31,"%d ",macro[i]);
    }
    target+=buf;
  }
}

void FurnaceGUI::encodeMMLStr(String& target, int* macro, int macroLen, int macroLoop, int macroRel, bool hex) {
  target="";
  char buf[32];
  for (int i=0; i<macroLen; i++) {
    if (i==macroLoop) target+="| ";
    if (i==macroRel) target+="/ ";
    if (hex) {
      if (i==macroLen-1) {
        snprintf(buf,31,"%.2X",macro[i]);
      } else {
        snprintf(buf,31,"%.2X ",macro[i]);
      }
    } else {
      if (i==macroLen-1) {
        snprintf(buf,31,"%d",macro[i]);
      } else {
        snprintf(buf,31,"%d ",macro[i]);
      }
    }
    target+=buf;
  }
}

void FurnaceGUI::decodeMMLStrW(String& source, int* macro, int& macroLen, int macroMax, bool hex) {
  int buf=0;
  bool negaBuf=false;
  bool hasVal=false;
  macroLen=0;
  for (char& i: source) {
    switch (i) {
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        hasVal=true;
        buf*=hex?16:10;
        buf+=i-'0';
        break;
      case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
        if (hex) {
          hasVal=true;
          buf*=16;
          buf+=10+i-'A';
        }
        break;
      case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
        if (hex) {
          hasVal=true;
          buf*=16;
          buf+=10+i-'a';
        }
        break;
      case '-':
        if (!hasVal) {
          hasVal=true;
          negaBuf=true;
        }
        break;
      case ' ':
        if (hasVal) {
          hasVal=false;
          negaBuf=false;
          macro[macroLen]=negaBuf?-buf:buf;
          if (macro[macroLen]<0) macro[macroLen]=0;
          if (macro[macroLen]>macroMax) macro[macroLen]=macroMax;
          macroLen++;
          buf=0;
        }
        break;
    }
    if (macroLen>=256) break;
  }
  if (hasVal && macroLen<256) {
    hasVal=false;
    negaBuf=false;
    macro[macroLen]=negaBuf?-buf:buf;
    if (macro[macroLen]<0) macro[macroLen]=0;
    if (macro[macroLen]>macroMax) macro[macroLen]=macroMax;
    macroLen++;
    buf=0;
  }
}

void FurnaceGUI::decodeMMLStr(String& source, unsigned char* macro, unsigned char& macroLen, signed char& macroLoop, int macroMin, int macroMax, signed char& macroRel) {
  int buf=0;
  bool hasVal=false;
  macroLen=0;
  macroLoop=-1;
  macroRel=-1;
  for (char& i: source) {
    switch (i) {
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        hasVal=true;
        buf*=10;
        buf+=i-'0';
        break;
      case ' ':
        if (hasVal) {
          hasVal=false;
          macro[macroLen]=buf;
          if (macro[macroLen]<macroMin) macro[macroLen]=macroMin;
          if (macro[macroLen]>macroMax) macro[macroLen]=macroMax;
          macroLen++;
          buf=0;
        }
        break;
      case '|':
        if (macroLoop==-1) {
          macroLoop=macroLen;
        }
        break;
      case '/':
        if (macroRel==-1) {
          macroRel=macroLen;
        }
        break;
    }
    if (macroLen>=128) break;
  }
  if (hasVal && macroLen<128) {
    hasVal=false;
    macro[macroLen]=buf;
    if (macro[macroLen]<macroMin) macro[macroLen]=macroMin;
    if (macro[macroLen]>macroMax) macro[macroLen]=macroMax;
    macroLen++;
    buf=0;
  }
}

void FurnaceGUI::decodeMMLStr(String& source, int* macro, unsigned char& macroLen, signed char& macroLoop, int macroMin, int macroMax, signed char& macroRel) {
  int buf=0;
  bool negaBuf=false;
  bool hasVal=false;
  macroLen=0;
  macroLoop=-1;
  macroRel=-1;
  for (char& i: source) {
    switch (i) {
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        hasVal=true;
        buf*=10;
        buf+=i-'0';
        break;
      case '-':
        if (!hasVal) {
          hasVal=true;
          negaBuf=true;
        }
        break;
      case ' ':
        if (hasVal) {
          hasVal=false;
          macro[macroLen]=negaBuf?-buf:buf;
          negaBuf=false;
          if (macro[macroLen]<macroMin) macro[macroLen]=macroMin;
          if (macro[macroLen]>macroMax) macro[macroLen]=macroMax;
          macroLen++;
          buf=0;
        }
        break;
      case '|':
        if (macroLoop==-1) {
          macroLoop=macroLen;
        }
        break;
      case '/':
        if (macroRel==-1) {
          macroRel=macroLen;
        }
        break;
    }
    if (macroLen>=128) break;
  }
  if (hasVal && macroLen<128) {
    hasVal=false;
    macro[macroLen]=negaBuf?-buf:buf;
    negaBuf=false;
    if (macro[macroLen]<macroMin) macro[macroLen]=macroMin;
    if (macro[macroLen]>macroMax) macro[macroLen]=macroMax;
    macroLen++;
    buf=0;
  }
}

#define CW_ADDITION(T) \
  if (p_min!=NULL && p_max!=NULL) { \
    if (*((T*)p_min)>*((T*)p_max)) { \
      if (wheelY<0) { \
        if ((*((T*)p_data)-wheelY)>*((T*)p_min)) { \
          *((T*)p_data)=*((T*)p_min); \
        } else { \
          *((T*)p_data)-=wheelY; \
        } \
      } else { \
        if ((*((T*)p_data)-wheelY)<*((T*)p_max)) { \
          *((T*)p_data)=*((T*)p_max); \
        } else { \
          *((T*)p_data)-=wheelY; \
        } \
      } \
    } else { \
      if (wheelY>0) { \
        if ((*((T*)p_data)+wheelY)>*((T*)p_max)) { \
          *((T*)p_data)=*((T*)p_max); \
        } else { \
          *((T*)p_data)+=wheelY; \
        } \
      } else { \
        if ((*((T*)p_data)+wheelY)<*((T*)p_min)) { \
          *((T*)p_data)=*((T*)p_min); \
        } else { \
          *((T*)p_data)+=wheelY; \
        } \
      } \
    } \
  }

bool FurnaceGUI::CWSliderScalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags) {
  if (ImGui::SliderScalar(label,data_type,p_data,p_min,p_max,format,flags)) {
    return true;
  }
  if (ImGui::IsItemHovered() && ctrlWheeling) {
    switch (data_type) {
      case ImGuiDataType_U8:
        CW_ADDITION(unsigned char);
        break;
      case ImGuiDataType_S8:
        CW_ADDITION(signed char);
        break;
      case ImGuiDataType_U16:
        CW_ADDITION(unsigned short);
        break;
      case ImGuiDataType_S16:
        CW_ADDITION(short);
        break;
      case ImGuiDataType_U32:
        CW_ADDITION(unsigned int);
        break;
      case ImGuiDataType_S32:
        CW_ADDITION(int);
        break;
      case ImGuiDataType_Float:
        CW_ADDITION(float);
        break;
      case ImGuiDataType_Double:
        CW_ADDITION(double);
        break;
    }
    return true;
  }
  return false;
}

bool FurnaceGUI::CWVSliderScalar(const char* label, const ImVec2& size, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags) {
  if (ImGui::VSliderScalar(label,size,data_type,p_data,p_min,p_max,format,flags)) {
    return true;
  }
  if (ImGui::IsItemHovered() && ctrlWheeling) {
    switch (data_type) {
      case ImGuiDataType_U8:
        CW_ADDITION(unsigned char);
        break;
      case ImGuiDataType_S8:
        CW_ADDITION(signed char);
        break;
      case ImGuiDataType_U16:
        CW_ADDITION(unsigned short);
        break;
      case ImGuiDataType_S16:
        CW_ADDITION(short);
        break;
      case ImGuiDataType_U32:
        CW_ADDITION(unsigned int);
        break;
      case ImGuiDataType_S32:
        CW_ADDITION(int);
        break;
      case ImGuiDataType_Float:
        CW_ADDITION(float);
        break;
      case ImGuiDataType_Double:
        CW_ADDITION(double);
        break;
    }
    return true;
  }
  return false;
}

bool FurnaceGUI::CWSliderInt(const char* label, int* v, int v_min, int v_max, const char* format, ImGuiSliderFlags flags) {
  return CWSliderScalar(label,ImGuiDataType_S32,v,&v_min,&v_max,format,flags);
}

bool FurnaceGUI::CWSliderFloat(const char* label, float* v, float v_min, float v_max, const char* format, ImGuiSliderFlags flags) {
  return CWSliderScalar(label,ImGuiDataType_Float,v,&v_min,&v_max,format,flags);
}

bool FurnaceGUI::CWVSliderInt(const char* label, const ImVec2& size, int* v, int v_min, int v_max, const char* format, ImGuiSliderFlags flags) {
  return CWVSliderScalar(label,size,ImGuiDataType_S32,v,&v_min,&v_max,format,flags);
}

const char* FurnaceGUI::getSystemName(DivSystem which) {
  if (settings.chipNames) {
    return e->getSystemChips(which);
  }
  return e->getSystemName(which);
}

void FurnaceGUI::updateScroll(int amount) {
  float lineHeight=(patFont->FontSize+2*dpiScale);
  nextScroll=lineHeight*amount;
}

void FurnaceGUI::addScroll(int amount) {
  float lineHeight=(patFont->FontSize+2*dpiScale);
  nextAddScroll=lineHeight*amount;
}

void FurnaceGUI::setFileName(String name) {
#ifdef _WIN32
  wchar_t ret[4096];
  WString ws=utf8To16(name.c_str());
  int index=0;
  for (wchar_t& i: ws) {
    ret[index++]=i;
    if (index>=4095) break;
  }
  ret[index]=0;
  if (GetFullPathNameW(ws.c_str(),4095,ret,NULL)==0) {
    curFileName=name;
  } else {
    curFileName=utf16To8(ret);
  }
#else
  char ret[4096];
  if (realpath(name.c_str(),ret)==NULL) {
    curFileName=name;
  } else {
    curFileName=ret;
  }
#endif
}

void FurnaceGUI::updateWindowTitle() {
  if (e->song.name.empty()) {
    SDL_SetWindowTitle(sdlWin,fmt::sprintf("Furnace (%s)",e->getSongSystemName()).c_str());
  } else {
    SDL_SetWindowTitle(sdlWin,fmt::sprintf("%s - Furnace (%s)",e->song.name,e->getSongSystemName()).c_str());
  }
}

const char* defaultLayout="[Window][DockSpaceViewport_11111111]\n\
Pos=0,24\n\
Size=1280,731\n\
Collapsed=0\n\
\n\
[Window][Debug##Default]\n\
Pos=54,0\n\
Size=400,400\n\
Collapsed=0\n\
\n\
[Window][Play/Edit Controls]\n\
Pos=181,208\n\
Size=45,409\n\
Collapsed=0\n\
\n\
[Window][Song Information]\n\
Pos=978,24\n\
Size=302,217\n\
Collapsed=0\n\
DockId=0x00000004,0\n\
\n\
[Window][Orders]\n\
Pos=0,24\n\
Size=345,217\n\
Collapsed=0\n\
DockId=0x00000007,0\n\
\n\
[Window][Instruments]\n\
Pos=653,24\n\
Size=323,217\n\
Collapsed=0\n\
DockId=0x00000006,2\n\
\n\
[Window][Wavetables]\n\
Pos=653,24\n\
Size=323,217\n\
Collapsed=0\n\
DockId=0x00000006,1\n\
\n\
[Window][Samples]\n\
Pos=653,24\n\
Size=323,217\n\
Collapsed=0\n\
DockId=0x00000006,0\n\
\n\
[Window][Pattern]\n\
Pos=0,243\n\
Size=1246,512\n\
Collapsed=0\n\
DockId=0x0000000B,0\n\
\n\
[Window][Instrument Editor]\n\
Pos=372,102\n\
Size=682,604\n\
Collapsed=0\n\
\n\
[Window][Warning]\n\
Pos=481,338\n\
Size=346,71\n\
Collapsed=0\n\
\n\
[Window][Sample Editor]\n\
Pos=531,176\n\
Size=613,416\n\
Collapsed=0\n\
\n\
[Window][About Furnace]\n\
Size=1280,755\n\
Collapsed=0\n\
\n\
[Window][Save File##FileDialog]\n\
Pos=340,177\n\
Size=600,400\n\
Collapsed=0\n\
\n\
[Window][Wavetable Editor]\n\
Pos=253,295\n\
Size=748,378\n\
Collapsed=0\n\
\n\
[Window][Settings]\n\
Pos=655,224\n\
Size=601,508\n\
Collapsed=0\n\
\n\
[Window][Error]\n\
Pos=491,342\n\
Size=514,71\n\
Collapsed=0\n\
\n\
[Window][Mixer]\n\
Pos=63,55\n\
Size=450,215\n\
Collapsed=0\n\
\n\
[Window][Oscilloscope]\n\
Pos=347,94\n\
Size=304,105\n\
Collapsed=0\n\
DockId=0x0000000E,0\n\
\n\
[Window][Volume Meter]\n\
Pos=1248,243\n\
Size=32,512\n\
Collapsed=0\n\
DockId=0x0000000C,0\n\
\n\
[Window][Debug]\n\
Pos=113,148\n\
Size=945,473\n\
Collapsed=0\n\
\n\
[Window][Load Sample##FileDialog]\n\
Pos=40,0\n\
Size=1200,755\n\
Collapsed=0\n\
\n\
[Window][Open File##FileDialog]\n\
Pos=250,143\n\
Size=779,469\n\
Collapsed=0\n\
\n\
[Window][Export Audio##FileDialog]\n\
Pos=339,177\n\
Size=601,400\n\
Collapsed=0\n\
\n\
[Window][Rendering...]\n\
Pos=585,342\n\
Size=114,71\n\
Collapsed=0\n\
\n\
[Window][Export VGM##FileDialog]\n\
Pos=340,177\n\
Size=600,400\n\
Collapsed=0\n\
\n\
[Window][Warning##Save FileFileDialogOverWriteDialog]\n\
Pos=390,351\n\
Size=500,71\n\
Collapsed=0\n\
\n\
[Window][Statistics]\n\
Pos=596,307\n\
Size=512,219\n\
Collapsed=0\n\
\n\
[Window][Warning##Export VGMFileDialogOverWriteDialog]\n\
Pos=390,351\n\
Size=500,71\n\
Collapsed=0\n\
\n\
[Window][Compatibility Flags]\n\
Pos=682,287\n\
Size=347,262\n\
Collapsed=0\n\
\n\
[Window][Song Comments]\n\
Pos=60,60\n\
Size=395,171\n\
Collapsed=0\n\
\n\
[Window][Warning##Export AudioFileDialogOverWriteDialog]\n\
Pos=381,351\n\
Size=500,71\n\
Collapsed=0\n\
\n\
[Window][Select Font##FileDialog]\n\
Pos=340,177\n\
Size=600,400\n\
Collapsed=0\n\
\n\
[Window][Channels]\n\
Pos=60,60\n\
Size=368,449\n\
Collapsed=0\n\
\n\
[Window][Register View]\n\
Pos=847,180\n\
Size=417,393\n\
Collapsed=0\n\
\n\
[Window][New Song]\n\
Pos=267,110\n\
Size=746,534\n\
Collapsed=0\n\
\n\
[Window][Edit Controls]\n\
Pos=347,24\n\
Size=304,68\n\
Collapsed=0\n\
DockId=0x0000000D,0\n\
\n\
[Window][Play Controls]\n\
Pos=347,201\n\
Size=304,40\n\
Collapsed=0\n\
DockId=0x0000000A,0\n\
\n\
[Docking][Data]\n\
DockSpace             ID=0x8B93E3BD Window=0xA787BDB4 Pos=0,24 Size=1280,731 Split=Y Selected=0x6C01C512\n\
  DockNode            ID=0x00000001 Parent=0x8B93E3BD SizeRef=1280,217 Split=X Selected=0xF3094A52\n\
    DockNode          ID=0x00000003 Parent=0x00000001 SizeRef=976,231 Split=X Selected=0x65CC51DC\n\
      DockNode        ID=0x00000007 Parent=0x00000003 SizeRef=345,231 HiddenTabBar=1 Selected=0x8F5BFC9A\n\
      DockNode        ID=0x00000008 Parent=0x00000003 SizeRef=629,231 Split=X Selected=0xD2AD486B\n\
        DockNode      ID=0x00000005 Parent=0x00000008 SizeRef=304,406 Split=Y Selected=0x6D682373\n\
          DockNode    ID=0x00000009 Parent=0x00000005 SizeRef=292,175 Split=Y Selected=0x6D682373\n\
            DockNode  ID=0x0000000D Parent=0x00000009 SizeRef=292,68 HiddenTabBar=1 Selected=0xE57B1A9D\n\
            DockNode  ID=0x0000000E Parent=0x00000009 SizeRef=292,105 HiddenTabBar=1 Selected=0x6D682373\n\
          DockNode    ID=0x0000000A Parent=0x00000005 SizeRef=292,40 HiddenTabBar=1 Selected=0x0DE44CFF\n\
        DockNode      ID=0x00000006 Parent=0x00000008 SizeRef=323,406 Selected=0xD2AD486B\n\
    DockNode          ID=0x00000004 Parent=0x00000001 SizeRef=302,231 Selected=0x60B9D088\n\
  DockNode            ID=0x00000002 Parent=0x8B93E3BD SizeRef=1280,512 Split=X Selected=0x6C01C512\n\
    DockNode          ID=0x0000000B Parent=0x00000002 SizeRef=1246,503 CentralNode=1 HiddenTabBar=1 Selected=0xB9ADD0D5\n\
    DockNode          ID=0x0000000C Parent=0x00000002 SizeRef=32,503 HiddenTabBar=1 Selected=0x644DA2C1\n\n";

void FurnaceGUI::prepareLayout() {
  FILE* check;
  check=ps_fopen(finalLayoutPath,"r");
  if (check!=NULL) {
    fclose(check);
    return;
  }

  // copy initial layout
  logI("loading default layout.\n");
  check=ps_fopen(finalLayoutPath,"w");
  if (check==NULL) {
    logW("could not write default layout!\n");
    return;
  }

  fwrite(defaultLayout,1,strlen(defaultLayout),check);
  fclose(check);
}

float FurnaceGUI::calcBPM(int s1, int s2, float hz) {
  float hl=e->song.hilightA;
  if (hl<=0.0f) hl=4.0f;
  float timeBase=e->song.timeBase+1;
  float speedSum=s1+s2;
  if (timeBase<1.0f) timeBase=1.0f;
  if (speedSum<1.0f) speedSum=1.0f;
  return 120.0f*hz/(timeBase*hl*speedSum);
}

void FurnaceGUI::play(int row) {
  e->walkSong(loopOrder,loopRow,loopEnd);
  memset(lastIns,-1,sizeof(int)*DIV_MAX_CHANS);
  if (row>0) {
    e->playToRow(row);
  } else {
    e->play();
  }
  curNibble=false;
  orderNibble=false;
  activeNotes.clear();
}

void FurnaceGUI::stop() {
  e->walkSong(loopOrder,loopRow,loopEnd);
  e->stop();
  curNibble=false;
  orderNibble=false;
  activeNotes.clear();
}

void FurnaceGUI::previewNote(int refChan, int note, bool autoNote) {
  if (autoNote) {
    e->setMidiBaseChan(refChan);
    e->synchronized([this,note]() {
      e->autoNoteOn(-1,curIns,note);
    });
    return;
  }

  bool chanBusy[DIV_MAX_CHANS];
  memset(chanBusy,0,DIV_MAX_CHANS*sizeof(bool));
  for (ActiveNote& i: activeNotes) {
    if (i.chan<0 || i.chan>=DIV_MAX_CHANS) continue;
    chanBusy[i.chan]=true;
  }
  int chanCount=e->getTotalChannelCount();
  int i=refChan;
  do {
    if (!chanBusy[i]) {
      e->noteOn(i,curIns,note);
      activeNotes.push_back(ActiveNote(i,note));
      //printf("PUSHING: %d NOTE %d\n",i,note);
      return;
    }
    i++;
    if (i>=chanCount) i=0;
  } while (i!=refChan);
  //printf("FAILED TO FIND CHANNEL!\n");
}

void FurnaceGUI::stopPreviewNote(SDL_Scancode scancode, bool autoNote) {
  if (activeNotes.empty() && !autoNote) return;
  try {
    int key=noteKeys.at(scancode);
    int num=12*curOctave+key;
    if (num<-60) num=-60; // C-(-5)
    if (num>119) num=119; // B-9

    if (key==100) return;
    if (key==101) return;
    if (key==102) return;

    if (autoNote) {
      e->synchronized([this,num]() {
        e->autoNoteOff(-1,num);
      });
      return;
    }

    for (size_t i=0; i<activeNotes.size(); i++) {
      if (activeNotes[i].note==num) {
        e->noteOff(activeNotes[i].chan);
        //printf("REMOVING %d\n",activeNotes[i].chan);
        activeNotes.erase(activeNotes.begin()+i);
        break;
      }
    }
  } catch (std::out_of_range& e) {
  }
}

void FurnaceGUI::noteInput(int num, int key, int vol) {
  DivPattern* pat=e->song.pat[cursor.xCoarse].getPattern(e->song.orders.ord[cursor.xCoarse][e->getOrder()],true);
  
  prepareUndo(GUI_UNDO_PATTERN_EDIT);

  if (key==100) { // note off
    pat->data[cursor.y][0]=100;
    pat->data[cursor.y][1]=0;
  } else if (key==101) { // note off + env release
    pat->data[cursor.y][0]=101;
    pat->data[cursor.y][1]=0;
  } else if (key==102) { // env release only
    pat->data[cursor.y][0]=102;
    pat->data[cursor.y][1]=0;
  } else {
    pat->data[cursor.y][0]=num%12;
    pat->data[cursor.y][1]=num/12;
    if (pat->data[cursor.y][0]==0) {
      pat->data[cursor.y][0]=12;
      pat->data[cursor.y][1]--;
    }
    pat->data[cursor.y][1]=(unsigned char)pat->data[cursor.y][1];
    if (latchIns==-2) {
      pat->data[cursor.y][2]=curIns;
    } else if (latchIns!=-1 && !e->song.ins.empty()) {
      pat->data[cursor.y][2]=MIN(((int)e->song.ins.size())-1,latchIns);
    }
    int maxVol=e->getMaxVolumeChan(cursor.xCoarse);
    if (latchVol!=-1) {
      pat->data[cursor.y][3]=MIN(maxVol,latchVol);
    } else if (vol!=-1) {
      pat->data[cursor.y][3]=(vol*maxVol)/127;
    }
    if (latchEffect!=-1) pat->data[cursor.y][4]=latchEffect;
    if (latchEffectVal!=-1) pat->data[cursor.y][5]=latchEffectVal;
  }
  makeUndo(GUI_UNDO_PATTERN_EDIT);
  editAdvance();
  curNibble=false;
}

void FurnaceGUI::valueInput(int num, bool direct, int target) {
  DivPattern* pat=e->song.pat[cursor.xCoarse].getPattern(e->song.orders.ord[cursor.xCoarse][e->getOrder()],true);
  prepareUndo(GUI_UNDO_PATTERN_EDIT);
  if (target==-1) target=cursor.xFine+1;
  if (direct) {
    pat->data[cursor.y][target]=num&0xff;
  } else {
    if (pat->data[cursor.y][target]==-1) pat->data[cursor.y][target]=0;
    pat->data[cursor.y][target]=((pat->data[cursor.y][target]<<4)|num)&0xff;
  }
  if (cursor.xFine==1) { // instrument
    if (pat->data[cursor.y][target]>=(int)e->song.ins.size()) {
      pat->data[cursor.y][target]&=0x0f;
      if (pat->data[cursor.y][target]>=(int)e->song.ins.size()) {
        pat->data[cursor.y][target]=(int)e->song.ins.size()-1;
      }
    }
    makeUndo(GUI_UNDO_PATTERN_EDIT);
    if (direct) {
      curNibble=false;
    } else {
      if (e->song.ins.size()<16) {
        curNibble=false;
        editAdvance();
      } else {
        curNibble=!curNibble;
        if (!curNibble) editAdvance();
      }
    }
  } else if (cursor.xFine==2) {
    if (curNibble) {
      if (pat->data[cursor.y][target]>e->getMaxVolumeChan(cursor.xCoarse)) pat->data[cursor.y][target]=e->getMaxVolumeChan(cursor.xCoarse);
    } else {
      pat->data[cursor.y][target]&=15;
    }
    makeUndo(GUI_UNDO_PATTERN_EDIT);
    if (direct) {
      curNibble=false;
    } else {
      if (e->getMaxVolumeChan(cursor.xCoarse)<16) {
        curNibble=false;
        editAdvance();
      } else {
        curNibble=!curNibble;
        if (!curNibble) editAdvance();
      }
    }
  } else {
    makeUndo(GUI_UNDO_PATTERN_EDIT);
    if (direct) {
      curNibble=false;
    } else {
      curNibble=!curNibble;
      if (!curNibble) editAdvance();
    }
  }
}

void FurnaceGUI::keyDown(SDL_Event& ev) {
  if (ImGuiFileDialog::Instance()->IsOpened()) return;
  if (aboutOpen) return;

  int mapped=ev.key.keysym.sym;
  if (ev.key.keysym.mod&KMOD_CTRL) {
    mapped|=FURKMOD_CTRL;
  }
  if (ev.key.keysym.mod&KMOD_ALT) {
    mapped|=FURKMOD_ALT;
  }
  if (ev.key.keysym.mod&KMOD_GUI) {
    mapped|=FURKMOD_META;
  }
  if (ev.key.keysym.mod&KMOD_SHIFT) {
    mapped|=FURKMOD_SHIFT;
  }

  if (bindSetActive) {
    if (!ev.key.repeat) {
      switch (ev.key.keysym.sym) {
        case SDLK_LCTRL: case SDLK_RCTRL:
        case SDLK_LALT: case SDLK_RALT:
        case SDLK_LGUI: case SDLK_RGUI:
        case SDLK_LSHIFT: case SDLK_RSHIFT:
          bindSetPending=false;
          actionKeys[bindSetTarget]=(mapped&(~FURK_MASK))|0xffffff;
          break;
        default:
          actionKeys[bindSetTarget]=mapped;
          bindSetActive=false;
          bindSetPending=false;
          bindSetTarget=0;
          bindSetPrevValue=0;
          parseKeybinds();
          break;
      }
    }
    return;
  }

  // PER-WINDOW KEYS
  switch (curWindow) {
    case GUI_WINDOW_PATTERN:
      try {
        int action=actionMapPat.at(mapped);
        if (action>0) {
          doAction(action);
          return;
        }
      } catch (std::out_of_range& e) {
      }
      // pattern input otherwise
      if (mapped&(FURKMOD_ALT|FURKMOD_CTRL|FURKMOD_META|FURKMOD_SHIFT)) break;
      if (!ev.key.repeat) {
        if (cursor.xFine==0) { // note
          try {
            int key=noteKeys.at(ev.key.keysym.scancode);
            int num=12*curOctave+key;

            if (num<-60) num=-60; // C-(-5)
            if (num>119) num=119; // B-9

            if (edit) {
              noteInput(num,key);
            }
            if (key!=100 && key!=101 && key!=102) {
              previewNote(cursor.xCoarse,num);
            }
          } catch (std::out_of_range& e) {
          }
        } else if (edit) { // value
          try {
            int num=valueKeys.at(ev.key.keysym.sym);
            valueInput(num);
          } catch (std::out_of_range& e) {
          }
        }
      }
      break;
    case GUI_WINDOW_ORDERS:
      try {
        int action=actionMapOrders.at(mapped);
        if (action>0) {
          doAction(action);
          return;
        }
      } catch (std::out_of_range& e) {
      }
      // order input otherwise
      if (mapped&(FURKMOD_ALT|FURKMOD_CTRL|FURKMOD_META|FURKMOD_SHIFT)) break;
      if (orderEditMode!=0) {
        try {
          int num=valueKeys.at(ev.key.keysym.sym);
          if (orderCursor>=0 && orderCursor<e->getTotalChannelCount()) {
            int curOrder=e->getOrder();
            e->lockSave([this,curOrder,num]() {
              e->song.orders.ord[orderCursor][curOrder]=((e->song.orders.ord[orderCursor][curOrder]<<4)|num)&0x7f;
            });
            if (orderEditMode==2 || orderEditMode==3) {
              curNibble=!curNibble;
              if (!curNibble) {
                if (orderEditMode==2) {
                  orderCursor++;
                  if (orderCursor>=e->getTotalChannelCount()) orderCursor=0;
                } else if (orderEditMode==3) {
                  if (curOrder<e->song.ordersLen-1) {
                    e->setOrder(curOrder+1);
                  }
                }
              }
            }
            e->walkSong(loopOrder,loopRow,loopEnd);
          }
        } catch (std::out_of_range& e) {
        }
      }
      break;
    case GUI_WINDOW_SAMPLE_EDIT:
      try {
        int action=actionMapSample.at(mapped);
        if (action>0) {
          doAction(action);
          return;
        }
      } catch (std::out_of_range& e) {
      }
      break;
    case GUI_WINDOW_INS_LIST:
      try {
        int action=actionMapInsList.at(mapped);
        if (action>0) {
          doAction(action);
          return;
        }
      } catch (std::out_of_range& e) {
      }
      break;
    case GUI_WINDOW_WAVE_LIST:
      try {
        int action=actionMapWaveList.at(mapped);
        if (action>0) {
          doAction(action);
          return;
        }
      } catch (std::out_of_range& e) {
      }
      break;
    case GUI_WINDOW_SAMPLE_LIST:
      try {
        int action=actionMapSampleList.at(mapped);
        if (action>0) {
          doAction(action);
          return;
        }
      } catch (std::out_of_range& e) {
      }
      break;
    default:
      break;
  }

  // GLOBAL KEYS
  try {
    int action=actionMapGlobal.at(mapped);
    if (action>0) {
      doAction(action);
      return;
    }
  } catch (std::out_of_range& e) {
  }

  // PER-WINDOW PREVIEW KEYS
  switch (curWindow) {
    case GUI_WINDOW_INS_EDIT:
    case GUI_WINDOW_INS_LIST:
    case GUI_WINDOW_EDIT_CONTROLS:
    case GUI_WINDOW_SONG_INFO:
      if (!ev.key.repeat) {
        try {
          int key=noteKeys.at(ev.key.keysym.scancode);
          int num=12*curOctave+key;
          if (key!=100 && key!=101 && key!=102) {
            previewNote(cursor.xCoarse,num,true);
          }
        } catch (std::out_of_range& e) {
        }
      }
      break;
    case GUI_WINDOW_SAMPLE_EDIT:
    case GUI_WINDOW_SAMPLE_LIST:
      if (!ev.key.repeat) {
        try {
          int key=noteKeys.at(ev.key.keysym.scancode);
          int num=12*curOctave+key;
          if (key!=100 && key!=101 && key!=102) {
            e->previewSample(curSample,num);
            samplePreviewOn=true;
            samplePreviewKey=ev.key.keysym.scancode;
            samplePreviewNote=num;
          }
        } catch (std::out_of_range& e) {
        }
      }
      break;
    case GUI_WINDOW_WAVE_LIST:
    case GUI_WINDOW_WAVE_EDIT:
      if (!ev.key.repeat) {
        try {
          int key=noteKeys.at(ev.key.keysym.scancode);
          int num=12*curOctave+key;
          if (key!=100 && key!=101 && key!=102) {
            e->previewWave(curWave,num);
            wavePreviewOn=true;
            wavePreviewKey=ev.key.keysym.scancode;
            wavePreviewNote=num;
          }
        } catch (std::out_of_range& e) {
        }
      }
      break;
    default:
      break;
  }
}

void FurnaceGUI::keyUp(SDL_Event& ev) {
  stopPreviewNote(ev.key.keysym.scancode,curWindow!=GUI_WINDOW_PATTERN);
  if (wavePreviewOn) {
    if (ev.key.keysym.scancode==wavePreviewKey) {
      wavePreviewOn=false;
      e->stopWavePreview();
    }
  }
  if (samplePreviewOn) {
    if (ev.key.keysym.scancode==samplePreviewKey) {
      samplePreviewOn=false;
      e->stopSamplePreview();
    }
  }
}

bool dirExists(String what) {
#ifdef _WIN32
  WString ws=utf8To16(what.c_str());
  return (PathIsDirectoryW(ws.c_str())!=FALSE);
#else
  struct stat st;
  if (stat(what.c_str(),&st)<0) return false;
  return (st.st_mode&S_IFDIR);
#endif
}

void FurnaceGUI::openFileDialog(FurnaceGUIFileDialogs type) {
  bool hasOpened=false;
  switch (type) {
    case GUI_FILE_OPEN:
      if (!dirExists(workingDirSong)) workingDirSong=getHomeDir();
      hasOpened=fileDialog->openLoad(
        "Open File",
        {"compatible files", "*.fur *.dmf *.mod",
         "all files", ".*"},
        "compatible files{.fur,.dmf,.mod},.*",
        workingDirSong,
        dpiScale
      );
      break;
    case GUI_FILE_SAVE:
      if (!dirExists(workingDirSong)) workingDirSong=getHomeDir();
      hasOpened=fileDialog->openSave(
        "Save File",
        {"Furnace song", "*.fur",
         "DefleMask 1.1 module", "*.dmf"},
        "Furnace song{.fur},DefleMask 1.1 module{.dmf}",
        workingDirSong,
        dpiScale
      );
      break;
    case GUI_FILE_SAVE_DMF_LEGACY:
      if (!dirExists(workingDirSong)) workingDirSong=getHomeDir();
      hasOpened=fileDialog->openSave(
        "Save File",
        {"DefleMask 1.0/legacy module", "*.dmf"},
        "DefleMask 1.0/legacy module{.dmf}",
        workingDirSong,
        dpiScale
      );
      break;
    case GUI_FILE_INS_OPEN:
      if (!dirExists(workingDirIns)) workingDirIns=getHomeDir();
      hasOpened=fileDialog->openLoad(
        "Load Instrument",
        {"compatible files", "*.fui *.dmp *.tfi *.vgi *.s3i *.sbi",
         "all files", ".*"},
        "compatible files{.fui,.dmp,.tfi,.vgi,.s3i,.sbi},.*",
        workingDirIns,
        dpiScale
      );
      break;
    case GUI_FILE_INS_SAVE:
      if (!dirExists(workingDirIns)) workingDirIns=getHomeDir();
      hasOpened=fileDialog->openSave(
        "Save Instrument",
        {"Furnace instrument", "*.fui"},
        "Furnace instrument{.fui}",
        workingDirIns,
        dpiScale
      );
      break;
    case GUI_FILE_WAVE_OPEN:
      if (!dirExists(workingDirWave)) workingDirWave=getHomeDir();
      hasOpened=fileDialog->openLoad(
        "Load Wavetable",
        {"compatible files", "*.fuw *.dmw",
         "all files", ".*"},
        "compatible files{.fuw,.dmw},.*",
        workingDirWave,
        dpiScale
      );
      break;
    case GUI_FILE_WAVE_SAVE:
      if (!dirExists(workingDirWave)) workingDirWave=getHomeDir();
      hasOpened=fileDialog->openSave(
        "Save Wavetable",
        {"Furnace wavetable", ".fuw"},
        "Furnace wavetable{.fuw}",
        workingDirWave,
        dpiScale
      );
      break;
    case GUI_FILE_SAMPLE_OPEN:
      if (!dirExists(workingDirSample)) workingDirSample=getHomeDir();
      hasOpened=fileDialog->openLoad(
        "Load Sample",
        {"Wave file", "*.wav",
         "all files", ".*"},
        "Wave file{.wav},.*",
        workingDirSample,
        dpiScale
      );
      break;
    case GUI_FILE_SAMPLE_SAVE:
      if (!dirExists(workingDirSample)) workingDirSample=getHomeDir();
      hasOpened=fileDialog->openSave(
        "Save Sample",
        {"Wave file", "*.wav"},
        "Wave file{.wav}",
        workingDirSample,
        dpiScale
      );
      break;
    case GUI_FILE_EXPORT_AUDIO_ONE:
      if (!dirExists(workingDirAudioExport)) workingDirAudioExport=getHomeDir();
      hasOpened=fileDialog->openSave(
        "Export Audio",
        {"Wave file", "*.wav"},
        "Wave file{.wav}",
        workingDirAudioExport,
        dpiScale
      );
      break;
    case GUI_FILE_EXPORT_AUDIO_PER_SYS:
      if (!dirExists(workingDirAudioExport)) workingDirAudioExport=getHomeDir();
      hasOpened=fileDialog->openSave(
        "Export Audio",
        {"Wave file", "*.wav"},
        "Wave file{.wav}",
        workingDirAudioExport,
        dpiScale
      );
      break;
    case GUI_FILE_EXPORT_AUDIO_PER_CHANNEL:
      if (!dirExists(workingDirAudioExport)) workingDirAudioExport=getHomeDir();
      hasOpened=fileDialog->openSave(
        "Export Audio",
        {"Wave file", "*.wav"},
        "Wave file{.wav}",
        workingDirAudioExport,
        dpiScale
      );
      break;
    case GUI_FILE_EXPORT_VGM:
      if (!dirExists(workingDirVGMExport)) workingDirVGMExport=getHomeDir();
      hasOpened=fileDialog->openSave(
        "Export VGM",
        {"VGM file", "*.vgm"},
        "VGM file{.vgm}",
        workingDirVGMExport,
        dpiScale
      );
      break;
    case GUI_FILE_EXPORT_ROM:
      showError("Coming soon!");
      break;
    case GUI_FILE_LOAD_MAIN_FONT:
      if (!dirExists(workingDirFont)) workingDirFont=getHomeDir();
      hasOpened=fileDialog->openLoad(
        "Select Font",
        {"compatible files", "*.ttf *.otf *.ttc"},
        "compatible files{.ttf,.otf,.ttc}",
        workingDirFont,
        dpiScale
      );
      break;
    case GUI_FILE_LOAD_PAT_FONT:
      if (!dirExists(workingDirFont)) workingDirFont=getHomeDir();
      hasOpened=fileDialog->openLoad(
        "Select Font",
        {"compatible files", "*.ttf *.otf *.ttc"},
        "compatible files{.ttf,.otf,.ttc}",
        workingDirFont,
        dpiScale
      );
      break;
  }
  if (hasOpened) curFileDialog=type;
  //ImGui::GetIO().ConfigFlags|=ImGuiConfigFlags_NavEnableKeyboard;
}

#define FURNACE_ZLIB_COMPRESS

int FurnaceGUI::save(String path, int dmfVersion) {
  SafeWriter* w;
  if (dmfVersion) {
    w=e->saveDMF(dmfVersion);
  } else {
    w=e->saveFur();
  }
  if (w==NULL) {
    lastError=e->getLastError();
    return 3;
  }
  FILE* outFile=ps_fopen(path.c_str(),"wb");
  if (outFile==NULL) {
    lastError=strerror(errno);
    w->finish();
    return 1;
  }
#ifdef FURNACE_ZLIB_COMPRESS
  unsigned char zbuf[131072];
  int ret;
  z_stream zl;
  memset(&zl,0,sizeof(z_stream));
  ret=deflateInit(&zl,Z_DEFAULT_COMPRESSION);
  if (ret!=Z_OK) {
    logE("zlib error!\n");
    lastError="compression error";
    fclose(outFile);
    w->finish();
    return 2;
  }
  zl.avail_in=w->size();
  zl.next_in=w->getFinalBuf();
  while (zl.avail_in>0) {
    zl.avail_out=131072;
    zl.next_out=zbuf;
    if ((ret=deflate(&zl,Z_NO_FLUSH))==Z_STREAM_ERROR) {
      logE("zlib stream error!\n");
      lastError="zlib stream error";
      deflateEnd(&zl);
      fclose(outFile);
      w->finish();
      return 2;
    }
    size_t amount=131072-zl.avail_out;
    if (amount>0) {
      if (fwrite(zbuf,1,amount,outFile)!=amount) {
        logE("did not write entirely: %s!\n",strerror(errno));
        lastError=strerror(errno);
        deflateEnd(&zl);
        fclose(outFile);
        w->finish();
        return 1;
      }
    }
  }
  zl.avail_out=131072;
  zl.next_out=zbuf;
  if ((ret=deflate(&zl,Z_FINISH))==Z_STREAM_ERROR) {
    logE("zlib finish stream error!\n");
    lastError="zlib finish stream error";
    deflateEnd(&zl);
    fclose(outFile);
    w->finish();
    return 2;
  }
  if (131072-zl.avail_out>0) {
    if (fwrite(zbuf,1,131072-zl.avail_out,outFile)!=(131072-zl.avail_out)) {
      logE("did not write entirely: %s!\n",strerror(errno));
      lastError=strerror(errno);
      deflateEnd(&zl);
      fclose(outFile);
      w->finish();
      return 1;
    }
  }
  deflateEnd(&zl);
#else
  if (fwrite(w->getFinalBuf(),1,w->size(),outFile)!=w->size()) {
    logE("did not write entirely: %s!\n",strerror(errno));
    lastError=strerror(errno);
    fclose(outFile);
    w->finish();
    return 1;
  }
#endif
  fclose(outFile);
  w->finish();
  curFileName=path;
  modified=false;
  if (!e->getWarnings().empty()) {
    showWarning(e->getWarnings(),GUI_WARN_GENERIC);
  }
  return 0;
}

int FurnaceGUI::load(String path) {
  if (!path.empty()) {
    logI("loading module...\n");
    FILE* f=ps_fopen(path.c_str(),"rb");
    if (f==NULL) {
      perror("error");
      lastError=strerror(errno);
      return 1;
    }
    if (fseek(f,0,SEEK_END)<0) {
      perror("size error");
      lastError=fmt::sprintf("on seek: %s",strerror(errno));
      fclose(f);
      return 1;
    }
    ssize_t len=ftell(f);
    if (len==(SIZE_MAX>>1)) {
      perror("could not get file length");
      lastError=fmt::sprintf("on pre tell: %s",strerror(errno));
      fclose(f);
      return 1;
    }
    if (len<1) {
      if (len==0) {
        logE("that file is empty!\n");
        lastError="file is empty";
      } else {
        perror("tell error");
        lastError=fmt::sprintf("on tell: %s",strerror(errno));
      }
      fclose(f);
      return 1;
    }
    unsigned char* file=new unsigned char[len];
    if (fseek(f,0,SEEK_SET)<0) {
      perror("size error");
      lastError=fmt::sprintf("on get size: %s",strerror(errno));
      fclose(f);
      delete[] file;
      return 1;
    }
    if (fread(file,1,(size_t)len,f)!=(size_t)len) {
      perror("read error");
      lastError=fmt::sprintf("on read: %s",strerror(errno));
      fclose(f);
      delete[] file;
      return 1;
    }
    fclose(f);
    if (!e->load(file,(size_t)len)) {
      lastError=e->getLastError();
      logE("could not open file!\n");
      return 1;
    }
  }
  curFileName=path;
  modified=false;
  curNibble=false;
  orderNibble=false;
  orderCursor=-1;
  samplePos=0;
  updateSampleTex=true;
  selStart=SelectionPoint();
  selEnd=SelectionPoint();
  cursor=SelectionPoint();
  lastError="everything OK";
  undoHist.clear();
  redoHist.clear();
  updateWindowTitle();
  if (!e->getWarnings().empty()) {
    showWarning(e->getWarnings(),GUI_WARN_GENERIC);
  }
  return 0;
}

void FurnaceGUI::exportAudio(String path, DivAudioExportModes mode) {
  e->saveAudio(path.c_str(),exportLoops+1,mode);
  displayExporting=true;
}

void FurnaceGUI::showWarning(String what, FurnaceGUIWarnings type) {
  warnString=what;
  warnAction=type;
  warnQuit=true;
}

void FurnaceGUI::showError(String what) {
  errorString=what;
  displayError=true;
}

#define MACRO_DRAG(t) \
  if (macroDragBitMode) { \
    if (macroDragLastX!=x || macroDragLastY!=y) { \
      macroDragLastX=x; \
      macroDragLastY=y; \
      if (macroDragInitialValueSet) { \
        if (macroDragInitialValue) { \
          t[x]=(((t[x]+macroDragBitOff)&((1<<macroDragMax)-1))&(~(1<<y)))-macroDragBitOff; \
        } else { \
          t[x]=(((t[x]+macroDragBitOff)&((1<<macroDragMax)-1))|(1<<y))-macroDragBitOff; \
        } \
      } else { \
        macroDragInitialValue=(((t[x]+macroDragBitOff)&((1<<macroDragMax)-1))&(1<<y)); \
        macroDragInitialValueSet=true; \
        t[x]=(((t[x]+macroDragBitOff)&((1<<macroDragMax)-1))^(1<<y))-macroDragBitOff; \
      } \
      t[x]&=(1<<macroDragMax)-1; \
    } \
  } else { \
    t[x]=y; \
  }

void FurnaceGUI::processDrags(int dragX, int dragY) {
  if (macroDragActive) {
    if (macroDragLen>0) {
      int x=((dragX-macroDragStart.x)*macroDragLen/MAX(1,macroDragAreaSize.x));
      if (x<0) x=0;
      if (x>=macroDragLen) x=macroDragLen-1;
      x+=macroDragScroll;
      int y;
      if (macroDragBitMode) {
        y=(int)(macroDragMax-((dragY-macroDragStart.y)*(double(macroDragMax-macroDragMin)/(double)MAX(1,macroDragAreaSize.y))));
      } else {
        y=round(macroDragMax-((dragY-macroDragStart.y)*(double(macroDragMax-macroDragMin)/(double)MAX(1,macroDragAreaSize.y))));
      }
      if (y>macroDragMax) y=macroDragMax;
      if (y<macroDragMin) y=macroDragMin;
      if (macroDragChar) {
        MACRO_DRAG(macroDragCTarget);
      } else {
        MACRO_DRAG(macroDragTarget);
      }
    }
  }
  if (macroLoopDragActive) {
    if (macroLoopDragLen>0) {
      int x=(dragX-macroLoopDragStart.x)*macroLoopDragLen/MAX(1,macroLoopDragAreaSize.x);
      if (x<0) x=0;
      if (x>=macroLoopDragLen) x=-1;
      x+=macroDragScroll;
      *macroLoopDragTarget=x;
    }
  }
  if (waveDragActive) {
    if (waveDragLen>0) {
      int x=(dragX-waveDragStart.x)*waveDragLen/MAX(1,waveDragAreaSize.x);
      if (x<0) x=0;
      if (x>=waveDragLen) x=waveDragLen-1;
      int y=round(waveDragMax-((dragY-waveDragStart.y)*(double(waveDragMax-waveDragMin)/(double)MAX(1,waveDragAreaSize.y))));
      if (y>waveDragMax) y=waveDragMax;
      if (y<waveDragMin) y=waveDragMin;
      waveDragTarget[x]=y;
      e->notifyWaveChange(curWave);
      MARK_MODIFIED;
    }
  }
  if (sampleDragActive) {
    int x=samplePos+round(double(dragX-sampleDragStart.x)*sampleZoom);
    int x1=samplePos+round(double(dragX-sampleDragStart.x+1)*sampleZoom);
    if (x<0) x=0;
    if (sampleDragMode) {
      if (x>=(int)sampleDragLen) x=sampleDragLen-1;
    } else {
      if (x>(int)sampleDragLen) x=sampleDragLen;
    }
    if (x1<0) x1=0;
    if (x1>=(int)sampleDragLen) x1=sampleDragLen-1;
    double y=0.5-double(dragY-sampleDragStart.y)/sampleDragAreaSize.y;
    if (sampleDragMode) { // draw
      if (sampleDrag16) {
        int val=y*65536;
        if (val<-32768) val=-32768;
        if (val>32767) val=32767;
        for (int i=x; i<=x1; i++) ((short*)sampleDragTarget)[i]=val;
      } else {
        int val=y*256;
        if (val<-128) val=-128;
        if (val>127) val=127;
        for (int i=x; i<=x1; i++) ((signed char*)sampleDragTarget)[i]=val;
      }
      updateSampleTex=true;
    } else { // select
      if (sampleSelStart<0) {
        sampleSelStart=x;
      }
      sampleSelEnd=x;
    }
  }
}

#define sysAddOption(x) \
  if (ImGui::MenuItem(getSystemName(x))) { \
    if (!e->addSystem(x)) { \
      showError("cannot add system! ("+e->getLastError()+")"); \
    } \
    updateWindowTitle(); \
  }

#define sysChangeOption(x,y) \
  if (ImGui::MenuItem(getSystemName(y),NULL,e->song.system[x]==y)) { \
    e->changeSystem(x,y); \
    updateWindowTitle(); \
  }

#define checkExtension(x) \
  String lowerCase=fileName; \
  for (char& i: lowerCase) { \
    if (i>='A' && i<='Z') i+='a'-'A'; \
  } \
  if (lowerCase.size()<4 || lowerCase.rfind(x)!=lowerCase.size()-4) { \
    fileName+=x; \
  }

#define checkExtensionDual(x,y,fallback) \
  String lowerCase=fileName; \
  for (char& i: lowerCase) { \
    if (i>='A' && i<='Z') i+='a'-'A'; \
  } \
  if (lowerCase.size()<4 || (lowerCase.rfind(x)!=lowerCase.size()-4 && lowerCase.rfind(y)!=lowerCase.size()-4)) { \
    fileName+=fallback; \
  }

void FurnaceGUI::editOptions(bool topMenu) {
  char id[4096];
  if (ImGui::MenuItem("cut",BIND_FOR(GUI_ACTION_PAT_CUT))) doCopy(true);
  if (ImGui::MenuItem("copy",BIND_FOR(GUI_ACTION_PAT_COPY))) doCopy(false);
  if (ImGui::MenuItem("paste",BIND_FOR(GUI_ACTION_PAT_PASTE))) doPaste();
  if (ImGui::BeginMenu("paste special...")) {
    if (ImGui::MenuItem("paste mix",BIND_FOR(GUI_ACTION_PAT_PASTE_MIX))) doPaste(GUI_PASTE_MODE_MIX_FG);
    if (ImGui::MenuItem("paste mix (background)",BIND_FOR(GUI_ACTION_PAT_PASTE_MIX_BG))) doPaste(GUI_PASTE_MODE_MIX_BG);
    if (ImGui::MenuItem("paste flood",BIND_FOR(GUI_ACTION_PAT_PASTE_FLOOD))) doPaste(GUI_PASTE_MODE_FLOOD);
    if (ImGui::MenuItem("paste overflow",BIND_FOR(GUI_ACTION_PAT_PASTE_OVERFLOW))) doPaste(GUI_PASTE_MODE_OVERFLOW);
    ImGui::EndMenu();
  }
  if (ImGui::MenuItem("delete",BIND_FOR(GUI_ACTION_PAT_DELETE))) doDelete();
  if (topMenu) {
    if (ImGui::MenuItem("select all",BIND_FOR(GUI_ACTION_PAT_SELECT_ALL))) doSelectAll();
  }
  ImGui::Separator();

  ImGui::Text("operation mask");
  ImGui::SameLine();

  ImGui::PushFont(patFont);
  if (ImGui::BeginTable("opMaskTable",5,ImGuiTableFlags_Borders|ImGuiTableFlags_SizingFixedFit|ImGuiTableFlags_NoHostExtendX)) {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_ACTIVE]);
    if (ImGui::Selectable(opMaskNote?"C-4##opMaskNote":"---##opMaskNote",opMaskNote,ImGuiSelectableFlags_DontClosePopups)) {
      opMaskNote=!opMaskNote;
    }
    ImGui::PopStyleColor();
    ImGui::TableNextColumn();
    ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_INS]);
    if (ImGui::Selectable(opMaskIns?"01##opMaskIns":"--##opMaskIns",opMaskIns,ImGuiSelectableFlags_DontClosePopups)) {
      opMaskIns=!opMaskIns;
    }
    ImGui::PopStyleColor();
    ImGui::TableNextColumn();
    ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_VOLUME_MAX]);
    if (ImGui::Selectable(opMaskVol?"7F##opMaskVol":"--##opMaskVol",opMaskVol,ImGuiSelectableFlags_DontClosePopups)) {
      opMaskVol=!opMaskVol;
    }
    ImGui::PopStyleColor();
    ImGui::TableNextColumn();
    ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_EFFECT_PITCH]);
    if (ImGui::Selectable(opMaskEffect?"04##opMaskEffect":"--##opMaskEffect",opMaskEffect,ImGuiSelectableFlags_DontClosePopups)) {
      opMaskEffect=!opMaskEffect;
    }
    ImGui::TableNextColumn();
    if (ImGui::Selectable(opMaskEffectVal?"72##opMaskEffectVal":"--##opMaskEffectVal",opMaskEffectVal,ImGuiSelectableFlags_DontClosePopups)) {
      opMaskEffectVal=!opMaskEffectVal;
    }
    ImGui::PopStyleColor();
    ImGui::EndTable();
  }
  ImGui::PopFont();

  ImGui::Text("input latch");
  if (ImGui::MenuItem("set latch",BIND_FOR(GUI_ACTION_PAT_LATCH))) {
    // TODO
  }
  ImGui::Separator();

  if (ImGui::MenuItem("note up",BIND_FOR(GUI_ACTION_PAT_NOTE_UP))) doTranspose(1);
  if (ImGui::MenuItem("note down",BIND_FOR(GUI_ACTION_PAT_NOTE_DOWN))) doTranspose(-1);
  if (ImGui::MenuItem("octave up",BIND_FOR(GUI_ACTION_PAT_OCTAVE_UP))) doTranspose(12);
  if (ImGui::MenuItem("octave down",BIND_FOR(GUI_ACTION_PAT_OCTAVE_DOWN)))  doTranspose(-12);
  if (ImGui::InputInt("##TransposeAmount",&transposeAmount,1,1)) {
    if (transposeAmount<-96) transposeAmount=-96;
    if (transposeAmount>96) transposeAmount=96;
  }
  ImGui::SameLine();
  if (ImGui::Button("Transpose")) {
    doTranspose(transposeAmount);
    ImGui::CloseCurrentPopup();
  }
  
  ImGui::Separator();
  if (ImGui::MenuItem("interpolate",BIND_FOR(GUI_ACTION_PAT_INTERPOLATE))) doInterpolate();
  if (ImGui::BeginMenu("change instrument...")) {
    if (e->song.ins.empty()) {
      ImGui::Text("no instruments available");
    }
    for (size_t i=0; i<e->song.ins.size(); i++) {
      snprintf(id,4095,"%.2X: %s",(int)i,e->song.ins[i]->name.c_str());
      if (ImGui::MenuItem(id)) {
        doChangeIns(i);
      }
    }
    ImGui::EndMenu();
  }
  if (ImGui::BeginMenu("gradient/fade...")) {
    if (ImGui::InputInt("Start",&fadeMin,1,1)) {
      if (fadeMin<0) fadeMin=0;
      if (fadeMode) {
        if (fadeMin>15) fadeMin=15;
      } else {
        if (fadeMin>255) fadeMin=255;
      }
    }
    if (ImGui::InputInt("End",&fadeMax,1,1)) {
      if (fadeMax<0) fadeMax=0;
      if (fadeMode) {
        if (fadeMax>15) fadeMax=15;
      } else {
        if (fadeMax>255) fadeMax=255;
      }
    }
    if (ImGui::Checkbox("Nibble mode",&fadeMode)) {
      if (fadeMode) {
        if (fadeMin>15) fadeMin=15;
        if (fadeMax>15) fadeMax=15;
      } else {
        if (fadeMin>255) fadeMin=255;
        if (fadeMax>255) fadeMax=255;
      }
    }
    if (ImGui::Button("Go ahead")) {
      doFade(fadeMin,fadeMax,fadeMode);
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndMenu();
  }
  if (ImGui::BeginMenu("scale...")) {
    if (ImGui::InputFloat("##ScaleMax",&scaleMax,1,1,"%.1f%%")) {
      if (scaleMax<0.0f) scaleMax=0.0f;
      if (scaleMax>25600.0f) scaleMax=25600.0f;
    }
    if (ImGui::Button("Scale")) {
      doScale(scaleMax);
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndMenu();
  }
  if (ImGui::BeginMenu("randomize...")) {
    if (ImGui::InputInt("Minimum",&randomizeMin,1,1)) {
      if (randomizeMin<0) randomizeMin=0;
      if (randomMode) {
        if (randomizeMin>15) randomizeMin=15;
      } else {
        if (randomizeMin>255) randomizeMin=255;
      }
      if (randomizeMin>randomizeMax) randomizeMin=randomizeMax;
    }
    if (ImGui::InputInt("Maximum",&randomizeMax,1,1)) {
      if (randomizeMax<0) randomizeMax=0;
      if (randomizeMax<randomizeMin) randomizeMax=randomizeMin;
      if (randomMode) {
        if (randomizeMax>15) randomizeMax=15;
      } else {
        if (randomizeMax>255) randomizeMax=255;
      }
    }
    if (ImGui::Checkbox("Nibble mode",&randomMode)) {
      if (randomMode) {
        if (randomizeMin>15) randomizeMin=15;
        if (randomizeMax>15) randomizeMax=15;
      } else {
        if (randomizeMin>255) randomizeMin=255;
        if (randomizeMax>255) randomizeMax=255;
      }
    }
    // TODO: add an option to set effect to specific value?
    if (ImGui::Button("Randomize")) {
      doRandomize(randomizeMin,randomizeMax,randomMode);
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndMenu();
  }
  if (ImGui::MenuItem("invert values",BIND_FOR(GUI_ACTION_PAT_INVERT_VALUES))) doInvertValues();

  ImGui::Separator();

  if (ImGui::MenuItem("flip selection",BIND_FOR(GUI_ACTION_PAT_FLIP_SELECTION))) doFlip();
  if (ImGui::MenuItem("collapse",BIND_FOR(GUI_ACTION_PAT_COLLAPSE_ROWS))) doCollapse(2);
  if (ImGui::MenuItem("expand",BIND_FOR(GUI_ACTION_PAT_EXPAND_ROWS))) doExpand(2);

  if (topMenu) {
    ImGui::Separator();
    ImGui::MenuItem("collapse pattern",BIND_FOR(GUI_ACTION_PAT_COLLAPSE_PAT));
    ImGui::MenuItem("expand pattern",BIND_FOR(GUI_ACTION_PAT_EXPAND_PAT));

    ImGui::Separator();
    ImGui::MenuItem("collapse song",BIND_FOR(GUI_ACTION_PAT_COLLAPSE_SONG));
    ImGui::MenuItem("expand song",BIND_FOR(GUI_ACTION_PAT_EXPAND_SONG));
  }
}

bool FurnaceGUI::loop() {
  while (!quit) {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
      ImGui_ImplSDL2_ProcessEvent(&ev);
      switch (ev.type) {
        case SDL_MOUSEMOTION: {
          int motionX=ev.motion.x;
          int motionY=ev.motion.y;
          int motionXrel=ev.motion.xrel;
          int motionYrel=ev.motion.yrel;
#ifdef __APPLE__
          motionX*=dpiScale;
          motionY*=dpiScale;
          motionXrel*=dpiScale;
          motionYrel*=dpiScale;
#endif
          if (selecting) {
            // detect whether we have to scroll
            if (motionY<patWindowPos.y+2.0f*dpiScale) {
              addScroll(-1);
            }
            if (motionY>patWindowPos.y+patWindowSize.y-2.0f*dpiScale) {
              addScroll(1);
            }
          }
          if (macroDragActive || macroLoopDragActive || waveDragActive || sampleDragActive) {
            int distance=fabs((double)motionXrel);
            if (distance<1) distance=1;
            float start=motionX-motionXrel;
            float end=motionX;
            float startY=motionY-motionYrel;
            float endY=motionY;
            for (int i=0; i<=distance; i++) {
              float fraction=(float)i/(float)distance;
              float x=start+(end-start)*fraction;
              float y=startY+(endY-startY)*fraction;
              processDrags(x,y);
            }
          }
          break;
        }
        case SDL_MOUSEBUTTONUP:
          if (macroDragActive || macroLoopDragActive || waveDragActive || (sampleDragActive && sampleDragMode)) {
            MARK_MODIFIED;
          }
          macroDragActive=false;
          macroDragBitMode=false;
          macroDragInitialValue=false;
          macroDragInitialValueSet=false;
          macroDragLastX=-1;
          macroDragLastY=-1;
          macroLoopDragActive=false;
          waveDragActive=false;
          if (sampleDragActive) {
            logD("stopping sample drag\n");
            if (sampleDragMode) {
              e->renderSamplesP();
            } else {
              if (sampleSelStart>sampleSelEnd) {
                sampleSelStart^=sampleSelEnd;
                sampleSelEnd^=sampleSelStart;
                sampleSelStart^=sampleSelEnd;
              }
            }
          }
          sampleDragActive=false;
          if (selecting) {
            cursor=selEnd;
            finishSelection();
            demandScrollX=true;
            if (cursor.xCoarse==selStart.xCoarse && cursor.xFine==selStart.xFine && cursor.y==selStart.y &&
                cursor.xCoarse==selEnd.xCoarse && cursor.xFine==selEnd.xFine && cursor.y==selEnd.y) {
              updateScroll(cursor.y);
            }
          }
          break;
        case SDL_MOUSEBUTTONDOWN:
          aboutOpen=false;
          if (bindSetActive) {
            bindSetActive=false;
            bindSetPending=false;
            actionKeys[bindSetTarget]=bindSetPrevValue;
            bindSetTarget=0;
            bindSetPrevValue=0;
          }
          break;
        case SDL_MOUSEWHEEL:
          wheelX+=ev.wheel.x;
          wheelY+=ev.wheel.y;
          break;
        case SDL_WINDOWEVENT:
          switch (ev.window.event) {
            case SDL_WINDOWEVENT_RESIZED:
#ifdef __APPLE__
              scrW=ev.window.data1;
              scrH=ev.window.data2;
#else
              scrW=ev.window.data1/dpiScale;
              scrH=ev.window.data2/dpiScale;
#endif
              break;
          }
          break;
        case SDL_KEYDOWN:
          if (!ImGui::GetIO().WantCaptureKeyboard) {
            keyDown(ev);
          }
          break;
        case SDL_KEYUP:
          if (!ImGui::GetIO().WantCaptureKeyboard) {
            keyUp(ev);
          } else {
            stopPreviewNote(ev.key.keysym.scancode);
            if (wavePreviewOn) {
              if (ev.key.keysym.scancode==wavePreviewKey) {
                wavePreviewOn=false;
                e->stopWavePreview();
              }
            }
            if (samplePreviewOn) {
              if (ev.key.keysym.scancode==samplePreviewKey) {
                samplePreviewOn=false;
                e->stopSamplePreview();
              }
            }
          }
          break;
        case SDL_DROPFILE:
          if (ev.drop.file!=NULL) {
            if (modified) {
              nextFile=ev.drop.file;
              showWarning("Unsaved changes! Are you sure?",GUI_WARN_OPEN_DROP);
            } else {
              if (load(ev.drop.file)>0) {
                showError(fmt::sprintf("Error while loading file! (%s)",lastError));
              }
            }
            SDL_free(ev.drop.file);
          }
          break;
        case SDL_QUIT:
          if (modified) {
            showWarning("Unsaved changes! Are you sure you want to quit?",GUI_WARN_QUIT);
          } else {
            quit=true;
            return true;
          }
          break;
      }
    }
    
    while (true) {
      midiLock.lock();
      if (midiQueue.empty()) {
        midiLock.unlock();
        break;
      }
      TAMidiMessage msg=midiQueue.front();
      midiLock.unlock();

      // parse message here
      if (learning!=-1) {
        if (learning>=0 && learning<(int)midiMap.binds.size()) {
          midiMap.binds[learning].type=msg.type>>4;
          midiMap.binds[learning].channel=msg.type&15;
          midiMap.binds[learning].data1=msg.data[0];
          switch (msg.type&0xf0) {
            case TA_MIDI_NOTE_OFF:
            case TA_MIDI_NOTE_ON:
            case TA_MIDI_AFTERTOUCH:
            case TA_MIDI_PITCH_BEND:
            case TA_MIDI_CONTROL:
              midiMap.binds[learning].data2=msg.data[1];
              break;
            default:
              midiMap.binds[learning].data2=128;
              break;
          }
        }
        learning=-1;
      } else {
        int action=midiMap.at(msg);
        if (action!=0) {
          doAction(action);
        } else switch (msg.type&0xf0) {
          case TA_MIDI_NOTE_ON:
            if (midiMap.valueInputStyle==0 || midiMap.valueInputStyle>3 || cursor.xFine==0) {
              if (midiMap.noteInput && edit && msg.data[1]!=0) {
                noteInput(
                  msg.data[0]-12,
                  0,
                  midiMap.volInput?((int)(pow((double)msg.data[1]/127.0,midiMap.volExp)*127.0)):-1
                );
              }
            } else {
              if (edit && msg.data[1]!=0) {
                switch (midiMap.valueInputStyle) {
                  case 1: {
                    int val=msg.data[0]%24;
                    if (val<16) {
                      valueInput(val);
                    }
                    break;
                  }
                  case 2:
                    valueInput(msg.data[0]&15);
                    break;
                  case 3:
                    int val=altValues[msg.data[0]%24];
                    if (val>=0) {
                      valueInput(val);
                    }
                    break;
                }
              }
            }
            break;
          case TA_MIDI_PROGRAM:
            if (midiMap.programChange) {
              curIns=msg.data[0];
              if (curIns>=(int)e->song.ins.size()) curIns=e->song.ins.size()-1;
            }
            break;
          case TA_MIDI_CONTROL:
            bool gchanged=false;
            if (msg.data[0]==midiMap.valueInputControlMSB) {
              midiMap.valueInputCurMSB=msg.data[1];
              gchanged=true;
            }
            if (msg.data[0]==midiMap.valueInputControlLSB) {
              midiMap.valueInputCurLSB=msg.data[1];
              gchanged=true;
            }
            if (msg.data[0]==midiMap.valueInputControlSingle) {
              midiMap.valueInputCurSingle=msg.data[1];
              gchanged=true;
            }
            if (gchanged && cursor.xFine>0) {
              switch (midiMap.valueInputStyle) {
                case 4: // dual CC
                  valueInput(((midiMap.valueInputCurMSB>>3)<<4)|(midiMap.valueInputCurLSB>>3),true);
                  break;
                case 5: // 14-bit
                  valueInput((midiMap.valueInputCurMSB<<1)|(midiMap.valueInputCurLSB>>6),true);
                  break;
                case 6: // single CC
                  valueInput((midiMap.valueInputCurSingle*255)/127,true);
                  break;
              }
            }

            for (int i=0; i<18; i++) {
              bool changed=false;
              if (midiMap.valueInputSpecificStyle[i]!=0) {
                if (msg.data[0]==midiMap.valueInputSpecificMSB[i]) {
                  changed=true;
                  midiMap.valueInputCurMSBS[i]=msg.data[1];
                }
                if (msg.data[0]==midiMap.valueInputSpecificLSB[i]) {
                  changed=true;
                  midiMap.valueInputCurLSBS[i]=msg.data[1];
                }
                if (msg.data[0]==midiMap.valueInputSpecificSingle[i]) {
                  changed=true;
                  midiMap.valueInputCurSingleS[i]=msg.data[1];
                }

                if (changed) switch (midiMap.valueInputStyle) {
                  case 1: // dual CC
                    valueInput(((midiMap.valueInputCurMSBS[i]>>3)<<4)|(midiMap.valueInputCurLSBS[i]>>3),true,i+2);
                    break;
                  case 2: // 14-bit
                    valueInput((midiMap.valueInputCurMSBS[i]<<1)|(midiMap.valueInputCurLSBS[i]>>6),true,i+2);
                    break;
                  case 3: // single CC
                    valueInput((midiMap.valueInputCurSingleS[i]*255)/127,true,i+2);
                    break;
                }
              }
            }
            break;
        }
      }

      midiLock.lock();
      midiQueue.pop();
      midiLock.unlock();
    }
    
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame(sdlWin);
    ImGui::NewFrame();

    curWindow=GUI_WINDOW_NOTHING;

    ImGui::BeginMainMenuBar();
    if (ImGui::BeginMenu("file")) {
      if (ImGui::MenuItem("new...")) {
        if (modified) {
          showWarning("Unsaved changes! Are you sure?",GUI_WARN_NEW);
        } else {
          displayNew=true;
        }
      }
      if (ImGui::MenuItem("open...",BIND_FOR(GUI_ACTION_OPEN))) {
        if (modified) {
          showWarning("Unsaved changes! Are you sure?",GUI_WARN_OPEN);
        } else {
          openFileDialog(GUI_FILE_OPEN);
        }
      }
      ImGui::Separator();
      if (ImGui::MenuItem("save",BIND_FOR(GUI_ACTION_SAVE))) {
        if (curFileName=="" || curFileName==backupPath || e->song.version>=0xff00) {
          openFileDialog(GUI_FILE_SAVE);
        } else {
          if (save(curFileName,e->song.isDMF?e->song.version:0)>0) {
            showError(fmt::sprintf("Error while saving file! (%s)",lastError));
          }
        }
      }
      if (ImGui::MenuItem("save as...",BIND_FOR(GUI_ACTION_SAVE_AS))) {
        openFileDialog(GUI_FILE_SAVE);
      }
      if (ImGui::MenuItem("save as .dmf (1.0/legacy)...",BIND_FOR(GUI_ACTION_SAVE_AS))) {
        openFileDialog(GUI_FILE_SAVE_DMF_LEGACY);
      }
      ImGui::Separator();
      if (ImGui::BeginMenu("export audio...")) {
        if (ImGui::MenuItem("one file")) {
          openFileDialog(GUI_FILE_EXPORT_AUDIO_ONE);
        }
        if (ImGui::MenuItem("multiple files (one per system)")) {
          openFileDialog(GUI_FILE_EXPORT_AUDIO_PER_SYS);
        }
        if (ImGui::MenuItem("multiple files (one per channel)")) {
          openFileDialog(GUI_FILE_EXPORT_AUDIO_PER_CHANNEL);
        }
        if (ImGui::InputInt("Loops",&exportLoops,1,2)) {
          if (exportLoops<0) exportLoops=0;
        }
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("export VGM...")) {
        ImGui::Text("settings:");
        if (ImGui::BeginCombo("format version",fmt::sprintf("%d.%.2x",vgmExportVersion>>8,vgmExportVersion&0xff).c_str())) {
          for (int i=0; i<6; i++) {
            if (ImGui::Selectable(fmt::sprintf("%d.%.2x",vgmVersions[i]>>8,vgmVersions[i]&0xff).c_str(),vgmExportVersion==vgmVersions[i])) {
              vgmExportVersion=vgmVersions[i];
            }
          }
          ImGui::EndCombo();
        }
        ImGui::Checkbox("loop",&vgmExportLoop);
        ImGui::Text("systems to export:");
        bool hasOneAtLeast=false;
        for (int i=0; i<e->song.systemLen; i++) {
          int minVersion=e->minVGMVersion(e->song.system[i]);
          ImGui::BeginDisabled(minVersion>vgmExportVersion || minVersion==0);
          ImGui::Checkbox(fmt::sprintf("%d. %s##_SYSV%d",i+1,getSystemName(e->song.system[i]),i).c_str(),&willExport[i]);
          ImGui::EndDisabled();
          if (minVersion>vgmExportVersion) {
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
              ImGui::SetTooltip("this system is only available in VGM %d.%.2x and higher!",minVersion>>8,minVersion&0xff);
            }
          } else if (minVersion==0) {
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
              ImGui::SetTooltip("this system is not supported by the VGM format!");
            }
          } else {
            if (willExport[i]) hasOneAtLeast=true;
          }
        }
        ImGui::Text("select the systems you wish to export,");
        ImGui::Text("but only up to %d of each type.",(vgmExportVersion>=0x151)?2:1);
        if (hasOneAtLeast) {
          if (ImGui::MenuItem("click to export")) {
            openFileDialog(GUI_FILE_EXPORT_VGM);
          }
        } else {
          ImGui::Text("nothing to export");
        }
        ImGui::EndMenu();
      }
      ImGui::Separator();
      if (ImGui::BeginMenu("add system...")) {
        for (int j=0; availableSystems[j]; j++) {
          sysAddOption((DivSystem)availableSystems[j]);
        }
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("configure system...")) {
        for (int i=0; i<e->song.systemLen; i++) {
          if (ImGui::TreeNode(fmt::sprintf("%d. %s##_SYSP%d",i+1,getSystemName(e->song.system[i]),i).c_str())) {
            drawSysConf(i);
            ImGui::TreePop();
          }
        }
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("change system...")) {
        for (int i=0; i<e->song.systemLen; i++) {
          if (ImGui::BeginMenu(fmt::sprintf("%d. %s##_SYSC%d",i+1,getSystemName(e->song.system[i]),i).c_str())) {
            for (int j=0; availableSystems[j]; j++) {
              sysChangeOption(i,(DivSystem)availableSystems[j]);
            }
            ImGui::EndMenu();
          }
        }
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("remove system...")) {
        for (int i=0; i<e->song.systemLen; i++) {
          if (ImGui::MenuItem(fmt::sprintf("%d. %s##_SYSR%d",i+1,getSystemName(e->song.system[i]),i).c_str())) {
            if (!e->removeSystem(i)) {
              showError("cannot remove system! ("+e->getLastError()+")");
            }
          }
        }
        ImGui::EndMenu();
      }
      ImGui::Separator();
      if (ImGui::MenuItem("restore backup",BIND_FOR(GUI_ACTION_OPEN_BACKUP))) {
        doAction(GUI_ACTION_OPEN_BACKUP);
      }
      ImGui::Separator();
      if (ImGui::MenuItem("exit")) {
        if (modified) {
          showWarning("Unsaved changes! Are you sure you want to quit?",GUI_WARN_QUIT);
        } else {
          quit=true;
        }
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("edit")) {
      if (ImGui::MenuItem("undo",BIND_FOR(GUI_ACTION_UNDO))) doUndo();
      if (ImGui::MenuItem("redo",BIND_FOR(GUI_ACTION_REDO))) doRedo();
      ImGui::Separator();
      editOptions(true);
      /*ImGui::Separator();
      ImGui::MenuItem("clear...");*/
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("settings")) {
      if (ImGui::MenuItem("lock layout",NULL,lockLayout)) {
        lockLayout=!lockLayout;
      }
      if (ImGui::MenuItem("visualizer",NULL,fancyPattern)) {
        fancyPattern=!fancyPattern;
        e->enableCommandStream(fancyPattern);
        e->getCommandStream(cmdStream);
        cmdStream.clear();
      }
      if (ImGui::MenuItem("reset layout")) {
        showWarning("Are you sure you want to reset the workspace layout?",GUI_WARN_RESET_LAYOUT);
      }
      if (ImGui::MenuItem("settings...",BIND_FOR(GUI_ACTION_WINDOW_SETTINGS))) {
        syncSettings();
        settingsOpen=true;
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("window")) {
      if (ImGui::MenuItem("song information",BIND_FOR(GUI_ACTION_WINDOW_SONG_INFO),songInfoOpen)) songInfoOpen=!songInfoOpen;
      if (ImGui::MenuItem("instruments",BIND_FOR(GUI_ACTION_WINDOW_INS_LIST),insListOpen)) insListOpen=!insListOpen;
      if (ImGui::MenuItem("wavetables",BIND_FOR(GUI_ACTION_WINDOW_WAVE_LIST),waveListOpen)) waveListOpen=!waveListOpen;
      if (ImGui::MenuItem("samples",BIND_FOR(GUI_ACTION_WINDOW_SAMPLE_LIST),sampleListOpen)) sampleListOpen=!sampleListOpen;
      if (ImGui::MenuItem("orders",BIND_FOR(GUI_ACTION_WINDOW_ORDERS),ordersOpen)) ordersOpen=!ordersOpen;
      if (ImGui::MenuItem("pattern",BIND_FOR(GUI_ACTION_WINDOW_PATTERN),patternOpen)) patternOpen=!patternOpen;
      if (ImGui::MenuItem("mixer",BIND_FOR(GUI_ACTION_WINDOW_MIXER),mixerOpen)) mixerOpen=!mixerOpen;
      if (ImGui::MenuItem("channels",BIND_FOR(GUI_ACTION_WINDOW_CHANNELS),channelsOpen)) channelsOpen=!channelsOpen;
      if (ImGui::MenuItem("compatibility flags",BIND_FOR(GUI_ACTION_WINDOW_COMPAT_FLAGS),compatFlagsOpen)) compatFlagsOpen=!compatFlagsOpen;
      if (ImGui::MenuItem("song comments",BIND_FOR(GUI_ACTION_WINDOW_NOTES),notesOpen)) notesOpen=!notesOpen;
      ImGui::Separator();
      if (ImGui::MenuItem("instrument editor",BIND_FOR(GUI_ACTION_WINDOW_INS_EDIT),insEditOpen)) insEditOpen=!insEditOpen;
      if (ImGui::MenuItem("wavetable editor",BIND_FOR(GUI_ACTION_WINDOW_WAVE_EDIT),waveEditOpen)) waveEditOpen=!waveEditOpen;
      if (ImGui::MenuItem("sample editor",BIND_FOR(GUI_ACTION_WINDOW_SAMPLE_EDIT),sampleEditOpen)) sampleEditOpen=!sampleEditOpen;
      ImGui::Separator();
      if (ImGui::MenuItem("play/edit controls",BIND_FOR(GUI_ACTION_WINDOW_EDIT_CONTROLS),editControlsOpen)) editControlsOpen=!editControlsOpen;
      if (ImGui::MenuItem("piano/input pad",BIND_FOR(GUI_ACTION_WINDOW_PIANO),pianoOpen)) pianoOpen=!pianoOpen;
      if (ImGui::MenuItem("oscilloscope",BIND_FOR(GUI_ACTION_WINDOW_OSCILLOSCOPE),oscOpen)) oscOpen=!oscOpen;
      if (ImGui::MenuItem("volume meter",BIND_FOR(GUI_ACTION_WINDOW_VOL_METER),volMeterOpen)) volMeterOpen=!volMeterOpen;
      if (ImGui::MenuItem("register view",BIND_FOR(GUI_ACTION_WINDOW_REGISTER_VIEW),regViewOpen)) regViewOpen=!regViewOpen;
      if (ImGui::MenuItem("statistics",BIND_FOR(GUI_ACTION_WINDOW_STATS),statsOpen)) statsOpen=!statsOpen;
     
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("help")) {
      if (ImGui::MenuItem("debug menu",BIND_FOR(GUI_ACTION_WINDOW_DEBUG))) debugOpen=!debugOpen;
      if (ImGui::MenuItem("panic",BIND_FOR(GUI_ACTION_PANIC))) e->syncReset();
      if (ImGui::MenuItem("about...",BIND_FOR(GUI_ACTION_WINDOW_ABOUT))) {
        aboutOpen=true;
        aboutScroll=0;
      }
      ImGui::EndMenu();
    }
    ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PLAYBACK_STAT]);
    if (e->isPlaying()) {
      int totalTicks=e->getTotalTicks();
      int totalSeconds=e->getTotalSeconds();
      ImGui::Text("| Speed %d:%d @ %gHz (%g BPM) | Order %d/%d | Row %d/%d | %d:%.2d:%.2d.%.2d",e->getSpeed1(),e->getSpeed2(),e->getCurHz(),calcBPM(e->getSpeed1(),e->getSpeed2(),e->getCurHz()),e->getOrder(),e->song.ordersLen,e->getRow(),e->song.patLen,totalSeconds/3600,(totalSeconds/60)%60,totalSeconds%60,totalTicks/10000);
    } else {
      bool hasInfo=false;
      String info;
      if (cursor.xCoarse>=0 && cursor.xCoarse<e->getTotalChannelCount()) {
        DivPattern* p=e->song.pat[cursor.xCoarse].getPattern(e->song.orders.ord[cursor.xCoarse][e->getOrder()],false);
        if (cursor.xFine>=0) switch (cursor.xFine) {
          case 0: // note
            if (p->data[cursor.y][0]>0) {
              if (p->data[cursor.y][0]==100) {
                info=fmt::sprintf("Note off (cut)");
              } else if (p->data[cursor.y][0]==101) {
                info=fmt::sprintf("Note off (release)");
              } else if (p->data[cursor.y][0]==102) {
                info=fmt::sprintf("Macro release only");
              } else {
                info=fmt::sprintf("Note on: %s",noteName(p->data[cursor.y][0],p->data[cursor.y][1]));
              }
              hasInfo=true;
            }
            break;
          case 1: // instrument
            if (p->data[cursor.y][2]>-1) {
              if (p->data[cursor.y][2]>=(int)e->song.ins.size()) {
                info=fmt::sprintf("Ins %d: <invalid>",p->data[cursor.y][2]);
              } else {
                DivInstrument* ins=e->getIns(p->data[cursor.y][2]);
                info=fmt::sprintf("Ins %d: %s",p->data[cursor.y][2],ins->name);
              }
              hasInfo=true;
            }
            break;
          case 2: // volume
            if (p->data[cursor.y][3]>-1) {
              int maxVol=e->getMaxVolumeChan(cursor.xCoarse);
              if (maxVol<1 || p->data[cursor.y][3]>maxVol) {
                info=fmt::sprintf("Set volume: %d (%.2X, INVALID!)",p->data[cursor.y][3],p->data[cursor.y][3]);
              } else {
                info=fmt::sprintf("Set volume: %d (%.2X, %d%%)",p->data[cursor.y][3],p->data[cursor.y][3],(p->data[cursor.y][3]*100)/maxVol);
              }
              hasInfo=true;
            }
            break;
          default: // effect
            int actualCursor=((cursor.xFine+1)&(~1));
            if (p->data[cursor.y][actualCursor]>-1) {
              info=e->getEffectDesc(p->data[cursor.y][actualCursor],cursor.xCoarse);
              hasInfo=true;
            }
            break;
        }
      }
      if (hasInfo && (settings.statusDisplay==0 || settings.statusDisplay==2)) {
        ImGui::Text("| %s",info.c_str());
      } else if (settings.statusDisplay==1 || settings.statusDisplay==2) {
        if (curFileName!="") ImGui::Text("| %s",curFileName.c_str());
      }
    }
    ImGui::PopStyleColor();
    if (modified) {
      ImGui::Text("| modified");
    }
    ImGui::EndMainMenuBar();

    ImGui::DockSpaceOverViewport(NULL,lockLayout?(ImGuiDockNodeFlags_NoResize|ImGuiDockNodeFlags_NoCloseButton|ImGuiDockNodeFlags_NoDocking|ImGuiDockNodeFlags_NoDockingSplitMe|ImGuiDockNodeFlags_NoDockingSplitOther):0);

    drawPattern();
    drawEditControls();
    drawSongInfo();
    drawOrders();
    drawSampleList();
    drawSampleEdit();
    drawWaveList();
    drawWaveEdit();
    drawInsList();
    drawInsEdit();
    drawMixer();
    drawOsc();
    drawVolMeter();
    drawSettings();
    drawDebug();
    drawStats();
    drawCompatFlags();
    drawPiano();
    drawNotes();
    drawChannels();
    drawRegView();

    if (inspectorOpen) ImGui::ShowMetricsWindow(&inspectorOpen);

    if (firstFrame) {
      firstFrame=false;
      if (patternOpen) nextWindow=GUI_WINDOW_PATTERN;
#ifdef __APPLE__
      SDL_RaiseWindow(sdlWin);
#endif
    }

    if (fileDialog->render(ImVec2(600.0f*dpiScale,400.0f*dpiScale),ImVec2(scrW*dpiScale,scrH*dpiScale))) {
      //ImGui::GetIO().ConfigFlags&=~ImGuiConfigFlags_NavEnableKeyboard;
      switch (curFileDialog) {
        case GUI_FILE_OPEN:
        case GUI_FILE_SAVE:
        case GUI_FILE_SAVE_DMF_LEGACY:
          workingDirSong=fileDialog->getPath()+DIR_SEPARATOR_STR;
          break;
        case GUI_FILE_INS_OPEN:
        case GUI_FILE_INS_SAVE:
          workingDirIns=fileDialog->getPath()+DIR_SEPARATOR_STR;
          break;
        case GUI_FILE_WAVE_OPEN:
        case GUI_FILE_WAVE_SAVE:
          workingDirWave=fileDialog->getPath()+DIR_SEPARATOR_STR;
          break;
        case GUI_FILE_SAMPLE_OPEN:
        case GUI_FILE_SAMPLE_SAVE:
          workingDirSample=fileDialog->getPath()+DIR_SEPARATOR_STR;
          break;
        case GUI_FILE_EXPORT_AUDIO_ONE:
        case GUI_FILE_EXPORT_AUDIO_PER_SYS:
        case GUI_FILE_EXPORT_AUDIO_PER_CHANNEL:
          workingDirAudioExport=fileDialog->getPath()+DIR_SEPARATOR_STR;
          break;
        case GUI_FILE_EXPORT_VGM:
        case GUI_FILE_EXPORT_ROM:
          workingDirVGMExport=fileDialog->getPath()+DIR_SEPARATOR_STR;
          break;
        case GUI_FILE_LOAD_MAIN_FONT:
        case GUI_FILE_LOAD_PAT_FONT:
          workingDirFont=fileDialog->getPath()+DIR_SEPARATOR_STR;
          break;
      }
      if (fileDialog->accepted()) {
        fileName=fileDialog->getFileName();
        if (fileName!="") {
          if (curFileDialog==GUI_FILE_SAVE) {
            // we can't tell whether the user chose .dmf or .fur in the system file picker
            const char* fallbackExt=(settings.sysFileDialog || ImGuiFileDialog::Instance()->GetCurrentFilter()=="Furnace song")?".fur":".dmf";
            checkExtensionDual(".fur",".dmf",fallbackExt);
          }
          if (curFileDialog==GUI_FILE_SAVE_DMF_LEGACY) {
            checkExtension(".dmf");
          }
          if (curFileDialog==GUI_FILE_SAMPLE_SAVE ||
              curFileDialog==GUI_FILE_EXPORT_AUDIO_ONE ||
              curFileDialog==GUI_FILE_EXPORT_AUDIO_PER_SYS ||
              curFileDialog==GUI_FILE_EXPORT_AUDIO_PER_CHANNEL) {
            checkExtension(".wav");
          }
          if (curFileDialog==GUI_FILE_INS_SAVE) {
            checkExtension(".fui");
          }
          if (curFileDialog==GUI_FILE_WAVE_SAVE) {
            checkExtension(".fuw");
          }
          if (curFileDialog==GUI_FILE_EXPORT_VGM) {
            checkExtension(".vgm");
          }
          String copyOfName=fileName;
          switch (curFileDialog) {
            case GUI_FILE_OPEN:
              if (load(copyOfName)>0) {
                showError(fmt::sprintf("Error while loading file! (%s)",lastError));
              }
              break;
            case GUI_FILE_SAVE: {
              logD("saving: %s\n",copyOfName.c_str());
              String lowerCase=fileName;
              for (char& i: lowerCase) {
                if (i>='A' && i<='Z') i+='a'-'A';
              }
              if ((lowerCase.size()<4 || lowerCase.rfind(".dmf")!=lowerCase.size()-4)) {
                if (save(copyOfName,0)>0) {
                  showError(fmt::sprintf("Error while saving file! (%s)",lastError));
                }
              } else {
                if (save(copyOfName,25)>0) {
                  showError(fmt::sprintf("Error while saving file! (%s)",lastError));
                }
              }
              break;
            }
            case GUI_FILE_SAVE_DMF_LEGACY:
              logD("saving: %s\n",copyOfName.c_str());
              if (save(copyOfName,24)>0) {
                showError(fmt::sprintf("Error while saving file! (%s)",lastError));
              }
              break;
            case GUI_FILE_INS_SAVE:
              if (curIns>=0 && curIns<(int)e->song.ins.size()) {
                e->song.ins[curIns]->save(copyOfName.c_str());
              }
              break;
            case GUI_FILE_WAVE_SAVE:
              if (curWave>=0 && curWave<(int)e->song.wave.size()) {
                e->song.wave[curWave]->save(copyOfName.c_str());
              }
              break;
            case GUI_FILE_SAMPLE_OPEN:
              if (e->addSampleFromFile(copyOfName.c_str())==-1) {
                showError(e->getLastError());
              } else {
                MARK_MODIFIED;
              }
              break;
            case GUI_FILE_SAMPLE_SAVE:
              if (curSample>=0 && curSample<(int)e->song.sample.size()) {
                e->song.sample[curSample]->save(copyOfName.c_str());
              }
              break;
            case GUI_FILE_EXPORT_AUDIO_ONE:
              exportAudio(copyOfName,DIV_EXPORT_MODE_ONE);
              break;
            case GUI_FILE_EXPORT_AUDIO_PER_SYS:
              exportAudio(copyOfName,DIV_EXPORT_MODE_MANY_SYS);
              break;
            case GUI_FILE_EXPORT_AUDIO_PER_CHANNEL:
              exportAudio(copyOfName,DIV_EXPORT_MODE_MANY_CHAN);
              break;
            case GUI_FILE_INS_OPEN: {
              std::vector<DivInstrument*> instruments=e->instrumentFromFile(copyOfName.c_str());
              if (!instruments.empty()) {
                if (!e->getWarnings().empty()) {
                  showWarning(e->getWarnings(),GUI_WARN_GENERIC);
                }
                for (DivInstrument* i: instruments) {
                  e->addInstrumentPtr(i);
                }
              } else {
                showError("cannot load instrument! ("+e->getLastError()+")");
              }
              break;
            }
            case GUI_FILE_WAVE_OPEN:
              e->addWaveFromFile(copyOfName.c_str());
              MARK_MODIFIED;
              break;
            case GUI_FILE_EXPORT_VGM: {
              SafeWriter* w=e->saveVGM(willExport,vgmExportLoop,vgmExportVersion);
              if (w!=NULL) {
                FILE* f=fopen(copyOfName.c_str(),"wb");
                if (f!=NULL) {
                  fwrite(w->getFinalBuf(),1,w->size(),f);
                  fclose(f);
                } else {
                  showError("could not open file!");
                }
                w->finish();
                delete w;
                if (!e->getWarnings().empty()) {
                  showWarning(e->getWarnings(),GUI_WARN_GENERIC);
                }
              } else {
                showError(fmt::sprintf("could not write VGM! (%s)",e->getLastError()));
              }
              break;
            }
            case GUI_FILE_EXPORT_ROM:
              showError("Coming soon!");
              break;
            case GUI_FILE_LOAD_MAIN_FONT:
              settings.mainFontPath=copyOfName;
              break;
            case GUI_FILE_LOAD_PAT_FONT:
              settings.patFontPath=copyOfName;
              break;
          }
          curFileDialog=GUI_FILE_OPEN;
        }
      }
      fileDialog->close();
    }

    if (warnQuit) {
      warnQuit=false;
      ImGui::OpenPopup("Warning");
    }

    if (displayError) {
      displayError=false;
      ImGui::OpenPopup("Error");
    }

    if (displayExporting) {
      displayExporting=false;
      ImGui::OpenPopup("Rendering...");
    }

    if (displayNew) {
      displayNew=false;
      ImGui::OpenPopup("New Song");
    }

    if (nextWindow==GUI_WINDOW_ABOUT) {
      aboutOpen=true;
      nextWindow=GUI_WINDOW_NOTHING;
    }
    if (aboutOpen) drawAbout();

    if (ImGui::BeginPopupModal("Rendering...",NULL,ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::Text("Please wait...\n");
      if (ImGui::Button("Abort")) {
        if (e->haltAudioFile()) {
          ImGui::CloseCurrentPopup();
        }
      }
      if (!e->isExporting()) {
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }

    ImGui::SetNextWindowSizeConstraints(ImVec2(400.0f*dpiScale,200.0f*dpiScale),ImVec2(scrW*dpiScale,scrH*dpiScale));
    if (ImGui::BeginPopupModal("New Song",NULL,ImGuiWindowFlags_NoMove)) {
      ImGui::SetWindowPos(ImVec2(((scrW*dpiScale)-ImGui::GetWindowSize().x)*0.5,((scrH*dpiScale)-ImGui::GetWindowSize().y)*0.5));
      drawNewSong();
      ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Error",NULL,ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::Text("%s",errorString.c_str());
      if (ImGui::Button("OK")) {
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Warning",NULL,ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::Text("%s",warnString.c_str());
      if (ImGui::Button(warnAction==GUI_WARN_GENERIC?"OK":"Yes")) {
        ImGui::CloseCurrentPopup();
        switch (warnAction) {
          case GUI_WARN_QUIT:
            quit=true;
            break;
          case GUI_WARN_NEW:
            displayNew=true;
            break;
          case GUI_WARN_OPEN:
            openFileDialog(GUI_FILE_OPEN);
            break;
          case GUI_WARN_OPEN_BACKUP:
            if (load(backupPath)>0) {
              showError("No backup available! (or unable to open it)");
            }
            break;
          case GUI_WARN_OPEN_DROP:
            if (load(nextFile)>0) {
              showError(fmt::sprintf("Error while loading file! (%s)",lastError));
            }
            nextFile="";
            break;
          case GUI_WARN_RESET_LAYOUT:
            ImGui::LoadIniSettingsFromMemory(defaultLayout);
            ImGui::SaveIniSettingsToDisk(finalLayoutPath);
            break;
          case GUI_WARN_GENERIC:
            break;
        }
      }
      if (warnAction!=GUI_WARN_GENERIC) {
        ImGui::SameLine();
        if (ImGui::Button("No")) {
          ImGui::CloseCurrentPopup();
        }
      }
      ImGui::EndPopup();
    }

    // backup trigger
    if (modified) {
      if (backupTimer>0) {
        backupTimer-=ImGui::GetIO().DeltaTime;
        if (backupTimer<=0) {
          backupTask=std::async(std::launch::async,[this]() -> bool {
            if (backupPath==curFileName) {
              logD("backup file open. not saving backup.\n");
              return true;
            }
            logD("saving backup...\n");
            SafeWriter* w=e->saveFur(true);
          
            if (w!=NULL) {
              FILE* outFile=ps_fopen(backupPath.c_str(),"wb");
              if (outFile!=NULL) {
                if (fwrite(w->getFinalBuf(),1,w->size(),outFile)!=w->size()) {
                  logW("did not write backup entirely: %s!\n",strerror(errno));
                  w->finish();
                }
                fclose(outFile);
              } else {
                logW("could not save backup: %s!\n",strerror(errno));
                w->finish();
              }
            }
            backupTimer=30.0;
            return true;
          });
        }
      }
    }

    SDL_SetRenderDrawColor(sdlRend,uiColors[GUI_COLOR_BACKGROUND].x*255,
                                   uiColors[GUI_COLOR_BACKGROUND].y*255,
                                   uiColors[GUI_COLOR_BACKGROUND].z*255,
                                   uiColors[GUI_COLOR_BACKGROUND].w*255);
    SDL_RenderClear(sdlRend);
    ImGui::Render();
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(sdlRend);

    if (--soloTimeout<0) soloTimeout=0;

    wheelX=0;
    wheelY=0;

    if (willCommit) {
      commitSettings();
      willCommit=false;
    }

    if (SDL_GetWindowFlags(sdlWin)&SDL_WINDOW_MINIMIZED) {
      SDL_Delay(100);
    }
  }
  return false;
}

bool FurnaceGUI::init() {
#ifndef __APPLE__
  float dpiScaleF;
#endif

  String homeDir=getHomeDir();
  workingDir=e->getConfString("lastDir",homeDir);
  workingDirSong=e->getConfString("lastDirSong",workingDir);
  workingDirIns=e->getConfString("lastDirIns",workingDir);
  workingDirWave=e->getConfString("lastDirWave",workingDir);
  workingDirSample=e->getConfString("lastDirSample",workingDir);
  workingDirAudioExport=e->getConfString("lastDirAudioExport",workingDir);
  workingDirVGMExport=e->getConfString("lastDirVGMExport",workingDir);
  workingDirFont=e->getConfString("lastDirFont",workingDir);

  editControlsOpen=e->getConfBool("editControlsOpen",true);
  ordersOpen=e->getConfBool("ordersOpen",true);
  insListOpen=e->getConfBool("insListOpen",true);
  songInfoOpen=e->getConfBool("songInfoOpen",true);
  patternOpen=e->getConfBool("patternOpen",true);
  insEditOpen=e->getConfBool("insEditOpen",false);
  waveListOpen=e->getConfBool("waveListOpen",true);
  waveEditOpen=e->getConfBool("waveEditOpen",false);
  sampleListOpen=e->getConfBool("sampleListOpen",true);
  sampleEditOpen=e->getConfBool("sampleEditOpen",false);
  settingsOpen=e->getConfBool("settingsOpen",false);
  mixerOpen=e->getConfBool("mixerOpen",false);
  oscOpen=e->getConfBool("oscOpen",true);
  volMeterOpen=e->getConfBool("volMeterOpen",true);
  statsOpen=e->getConfBool("statsOpen",false);
  compatFlagsOpen=e->getConfBool("compatFlagsOpen",false);
  pianoOpen=e->getConfBool("pianoOpen",false);
  notesOpen=e->getConfBool("notesOpen",false);
  channelsOpen=e->getConfBool("channelsOpen",false);
  regViewOpen=e->getConfBool("regViewOpen",false);

  tempoView=e->getConfBool("tempoView",true);
  waveHex=e->getConfBool("waveHex",false);
  lockLayout=e->getConfBool("lockLayout",false);

  syncSettings();

  if (settings.dpiScale>=0.5f) {
    dpiScale=settings.dpiScale;
  }

#if !(defined(__APPLE__) || defined(_WIN32))
  unsigned char* furIcon=getFurnaceIcon();
  SDL_Surface* icon=SDL_CreateRGBSurfaceFrom(furIcon,256,256,32,256*4,0xff,0xff00,0xff0000,0xff000000);
#endif

  scrW=e->getConfInt("lastWindowWidth",1280);
  scrH=e->getConfInt("lastWindowHeight",800);

#ifndef __APPLE__
  SDL_Rect displaySize;
#endif

  SDL_SetHint("SDL_HINT_VIDEO_ALLOW_SCREENSAVER","1");

  SDL_Init(SDL_INIT_VIDEO);

  sdlWin=SDL_CreateWindow("Furnace",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,scrW*dpiScale,scrH*dpiScale,SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALLOW_HIGHDPI);
  if (sdlWin==NULL) {
    logE("could not open window! %s\n",SDL_GetError());
    return false;
  }

#ifndef __APPLE__
  if (settings.dpiScale<0.5f) {
    SDL_GetDisplayDPI(SDL_GetWindowDisplayIndex(sdlWin),&dpiScaleF,NULL,NULL);
    dpiScale=round(dpiScaleF/96.0f);
    if (dpiScale<1) dpiScale=1;
    if (dpiScale!=1) SDL_SetWindowSize(sdlWin,scrW*dpiScale,scrH*dpiScale);

    if (SDL_GetDisplayUsableBounds(SDL_GetWindowDisplayIndex(sdlWin),&displaySize)==0) {
      if (scrW>displaySize.w/dpiScale) scrW=(displaySize.w/dpiScale)-32;
      if (scrH>displaySize.h/dpiScale) scrH=(displaySize.h/dpiScale)-32;
      SDL_SetWindowSize(sdlWin,scrW*dpiScale,scrH*dpiScale);
    }
  }
#endif

#if !(defined(__APPLE__) || defined(_WIN32))
  if (icon!=NULL) {
    SDL_SetWindowIcon(sdlWin,icon);
    SDL_FreeSurface(icon);
    free(furIcon);
  } else {
    logW("could not create icon!\n");
  }
#endif

  sdlRend=SDL_CreateRenderer(sdlWin,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC|SDL_RENDERER_TARGETTEXTURE);

  if (sdlRend==NULL) {
    logE("could not init renderer! %s\n",SDL_GetError());
    return false;
  }

#ifdef __APPLE__
  dpiScale=getMacDPIScale();
#endif

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGui_ImplSDL2_InitForSDLRenderer(sdlWin,sdlRend);
  ImGui_ImplSDLRenderer_Init(sdlRend);

  applyUISettings();

  if (!ImGui::GetIO().Fonts->Build()) {
    logE("error while building font atlas!\n");
    showError("error while loading fonts! please check your settings.");
    ImGui::GetIO().Fonts->Clear();
    mainFont=ImGui::GetIO().Fonts->AddFontDefault();
    patFont=mainFont;
    ImGui_ImplSDLRenderer_DestroyFontsTexture();
    if (!ImGui::GetIO().Fonts->Build()) {
      logE("error again while building font atlas!\n");
    }
  }

  strncpy(finalLayoutPath,(e->getConfigPath()+String(LAYOUT_INI)).c_str(),4095);
  backupPath=e->getConfigPath()+String(BACKUP_FUR);
  prepareLayout();

  ImGui::GetIO().ConfigFlags|=ImGuiConfigFlags_DockingEnable;
  ImGui::GetIO().IniFilename=finalLayoutPath;
  ImGui::LoadIniSettingsFromDisk(finalLayoutPath);

  updateWindowTitle();

  for (int i=0; i<DIV_MAX_CHANS; i++) {
    oldPat[i]=new DivPattern;
  }

  firstFrame=true;

  // TODO: MIDI mapping time!
  e->setMidiCallback([this](const TAMidiMessage& msg) -> int {
    midiLock.lock();
    midiQueue.push(msg);
    midiLock.unlock();
    e->setMidiBaseChan(cursor.xCoarse);
    if (midiMap.valueInputStyle!=0 && cursor.xFine!=0 && edit) return -2;
    if (!midiMap.noteInput) return -2;
    if (learning!=-1) return -2;
    if (midiMap.at(msg)) return -2;
    return curIns;
  });

  return true;
}

bool FurnaceGUI::finish() {
  ImGui::SaveIniSettingsToDisk(finalLayoutPath);
  ImGui_ImplSDLRenderer_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
  SDL_DestroyRenderer(sdlRend);
  SDL_DestroyWindow(sdlWin);

  e->setConf("lastDir",workingDir);
  e->setConf("lastDirSong",workingDirSong);
  e->setConf("lastDirIns",workingDirIns);
  e->setConf("lastDirWave",workingDirWave);
  e->setConf("lastDirSample",workingDirSample);
  e->setConf("lastDirAudioExport",workingDirAudioExport);
  e->setConf("lastDirVGMExport",workingDirVGMExport);
  e->setConf("lastDirFont",workingDirFont);

  // commit last open windows
  e->setConf("editControlsOpen",editControlsOpen);
  e->setConf("ordersOpen",ordersOpen);
  e->setConf("insListOpen",insListOpen);
  e->setConf("songInfoOpen",songInfoOpen);
  e->setConf("patternOpen",patternOpen);
  e->setConf("insEditOpen",insEditOpen);
  e->setConf("waveListOpen",waveListOpen);
  e->setConf("waveEditOpen",waveEditOpen);
  e->setConf("sampleListOpen",sampleListOpen);
  e->setConf("sampleEditOpen",sampleEditOpen);
  e->setConf("settingsOpen",settingsOpen);
  e->setConf("mixerOpen",mixerOpen);
  e->setConf("oscOpen",oscOpen);
  e->setConf("volMeterOpen",volMeterOpen);
  e->setConf("statsOpen",statsOpen);
  e->setConf("compatFlagsOpen",compatFlagsOpen);
  e->setConf("pianoOpen",pianoOpen);
  e->setConf("notesOpen",notesOpen);
  e->setConf("channelsOpen",channelsOpen);
  e->setConf("regViewOpen",regViewOpen);

  // commit last window size
  e->setConf("lastWindowWidth",scrW);
  e->setConf("lastWindowHeight",scrH);

  e->setConf("tempoView",tempoView);
  e->setConf("waveHex",waveHex);
  e->setConf("lockLayout",lockLayout);

  for (int i=0; i<DIV_MAX_CHANS; i++) {
    delete oldPat[i];
  }

  if (backupTask.valid()) {
    backupTask.get();
  }

  return true;
}

FurnaceGUI::FurnaceGUI():
  e(NULL),
  sampleTex(NULL),
  sampleTexW(0),
  sampleTexH(0),
  updateSampleTex(true),
  quit(false),
  warnQuit(false),
  willCommit(false),
  edit(false),
  modified(false),
  displayError(false),
  displayExporting(false),
  vgmExportLoop(true),
  displayNew(false),
  vgmExportVersion(0x171),
  curFileDialog(GUI_FILE_OPEN),
  warnAction(GUI_WARN_OPEN),
  fileDialog(NULL),
  scrW(1280),
  scrH(800),
  dpiScale(1),
  aboutScroll(0),
  aboutSin(0),
  aboutHue(0.0f),
  backupTimer(15.0),
  learning(-1),
  mainFont(NULL),
  iconFont(NULL),
  patFont(NULL),
  bigFont(NULL),
  fontRange(NULL),
  curIns(0),
  curWave(0),
  curSample(0),
  curOctave(3),
  oldRow(0),
  oldOrder(0),
  oldOrder1(0),
  editStep(1),
  exportLoops(0),
  soloChan(-1),
  soloTimeout(0),
  orderEditMode(0),
  orderCursor(-1),
  loopOrder(-1),
  loopRow(-1),
  loopEnd(-1),
  isClipping(0),
  extraChannelButtons(0),
  patNameTarget(-1),
  newSongCategory(0),
  wheelX(0),
  wheelY(0),
  editControlsOpen(true),
  ordersOpen(true),
  insListOpen(true),
  songInfoOpen(true),
  patternOpen(true),
  insEditOpen(false),
  waveListOpen(true),
  waveEditOpen(false),
  sampleListOpen(true),
  sampleEditOpen(false),
  aboutOpen(false),
  settingsOpen(false),
  mixerOpen(false),
  debugOpen(false),
  inspectorOpen(false),
  oscOpen(true),
  volMeterOpen(true),
  statsOpen(false),
  compatFlagsOpen(false),
  pianoOpen(false),
  notesOpen(false),
  channelsOpen(false),
  regViewOpen(false),
  /*
  editControlsDocked(false),
  ordersDocked(false),
  insListDocked(false),
  songInfoDocked(false),
  patternDocked(false),
  insEditDocked(false),
  waveListDocked(false),
  waveEditDocked(false),
  sampleListDocked(false),
  sampleEditDocked(false),
  aboutDocked(false),
  settingsDocked(false),
  mixerDocked(false),
  debugDocked(false),
  inspectorDocked(false),
  oscDocked(false),
  volMeterDocked(false),
  statsDocked(false),
  compatFlagsDocked(false),
  pianoDocked(false),
  notesDocked(false),
  channelsDocked(false),
  regViewDocked(false),
  */
  selecting(false),
  curNibble(false),
  orderNibble(false),
  followOrders(true),
  followPattern(true),
  changeAllOrders(false),
  collapseWindow(false),
  demandScrollX(false),
  fancyPattern(false),
  wantPatName(false),
  firstFrame(true),
  tempoView(true),
  waveHex(false),
  lockLayout(false),
  curWindow(GUI_WINDOW_NOTHING),
  nextWindow(GUI_WINDOW_NOTHING),
  nextDesc(NULL),
  opMaskNote(true),
  opMaskIns(true),
  opMaskVol(true),
  opMaskEffect(true),
  opMaskEffectVal(true),
  latchNote(-1),
  latchIns(-2),
  latchVol(-1),
  latchEffect(-1),
  latchEffectVal(-1),
  wavePreviewOn(false),
  wavePreviewKey((SDL_Scancode)0),
  wavePreviewNote(0),
  samplePreviewOn(false),
  samplePreviewKey((SDL_Scancode)0),
  samplePreviewNote(0),
  arpMacroScroll(0),
  macroDragStart(0,0),
  macroDragAreaSize(0,0),
  macroDragCTarget(NULL),
  macroDragTarget(NULL),
  macroDragLen(0),
  macroDragMin(0),
  macroDragMax(0),
  macroDragLastX(-1),
  macroDragLastY(-1),
  macroDragBitOff(0),
  macroDragScroll(0),
  macroDragBitMode(false),
  macroDragInitialValueSet(false),
  macroDragInitialValue(false),
  macroDragChar(false),
  macroDragActive(false),
  macroLoopDragStart(0,0),
  macroLoopDragAreaSize(0,0),
  macroLoopDragTarget(NULL),
  macroLoopDragLen(0),
  macroLoopDragActive(false),
  waveDragStart(0,0),
  waveDragAreaSize(0,0),
  waveDragTarget(NULL),
  waveDragLen(0),
  waveDragMin(0),
  waveDragMax(0),
  waveDragActive(false),
  bindSetTarget(0),
  bindSetPrevValue(0),
  bindSetActive(false),
  bindSetPending(false),
  nextScroll(-1.0f),
  nextAddScroll(0.0f),
  transposeAmount(0),
  randomizeMin(0),
  randomizeMax(255),
  fadeMin(0),
  fadeMax(255),
  scaleMax(100.0f),
  fadeMode(false),
  randomMode(false),
  oldOrdersLen(0),
  sampleZoom(1.0),
  prevSampleZoom(1.0),
  samplePos(0),
  resizeSize(1024),
  silenceSize(1024),
  resampleTarget(32000),
  resampleStrat(5),
  amplifyVol(100.0),
  sampleSelStart(-1),
  sampleSelEnd(-1),
  sampleDragActive(false),
  sampleDragMode(false),
  sampleDrag16(false),
  sampleZoomAuto(true),
  sampleDragTarget(NULL),
  sampleDragStart(0,0),
  sampleDragAreaSize(0,0),
  sampleDragLen(0),
  sampleFilterL(1.0f),
  sampleFilterB(0.0f),
  sampleFilterH(0.0f),
  sampleFilterRes(0.25f),
  sampleFilterCutStart(16000.0f),
  sampleFilterCutEnd(100.0f),
  sampleFilterPower(1),
  sampleClipboard(NULL),
  sampleClipboardLen(0),
  openSampleResizeOpt(false),
  openSampleResampleOpt(false),
  openSampleAmplifyOpt(false),
  openSampleSilenceOpt(false),
  openSampleFilterOpt(false) {
  // value keys
  valueKeys[SDLK_0]=0;
  valueKeys[SDLK_1]=1;
  valueKeys[SDLK_2]=2;
  valueKeys[SDLK_3]=3;
  valueKeys[SDLK_4]=4;
  valueKeys[SDLK_5]=5;
  valueKeys[SDLK_6]=6;
  valueKeys[SDLK_7]=7;
  valueKeys[SDLK_8]=8;
  valueKeys[SDLK_9]=9;
  valueKeys[SDLK_a]=10;
  valueKeys[SDLK_b]=11;
  valueKeys[SDLK_c]=12;
  valueKeys[SDLK_d]=13;
  valueKeys[SDLK_e]=14;
  valueKeys[SDLK_f]=15;
  valueKeys[SDLK_KP_0]=0;
  valueKeys[SDLK_KP_1]=1;
  valueKeys[SDLK_KP_2]=2;
  valueKeys[SDLK_KP_3]=3;
  valueKeys[SDLK_KP_4]=4;
  valueKeys[SDLK_KP_5]=5;
  valueKeys[SDLK_KP_6]=6;
  valueKeys[SDLK_KP_7]=7;
  valueKeys[SDLK_KP_8]=8;
  valueKeys[SDLK_KP_9]=9;

  initSystemPresets();

  memset(willExport,1,32*sizeof(bool));

  peak[0]=0;
  peak[1]=0;

  memset(actionKeys,0,GUI_ACTION_MAX*sizeof(int));

  memset(patChanX,0,sizeof(float)*(DIV_MAX_CHANS+1));
  memset(patChanSlideY,0,sizeof(float)*(DIV_MAX_CHANS+1));
  memset(lastIns,-1,sizeof(int)*DIV_MAX_CHANS);
}
