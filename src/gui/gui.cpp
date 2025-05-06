#define _USE_MATH_DEFINES
// OK, sorry for inserting the define here but I'm so tired of this extension
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

// I hate you clangd extension!
// how about you DON'T insert random headers before this freaking important
// define!!!!!!

#include "gui.h"
#include "util.h"
#include "../ta-log.h"
#include "../fileutils.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "ImGuiFileDialog.h"
#include "IconsFontAwesome4.h"
#include "misc/cpp/imgui_stdlib.h"
#include "plot_nolerp.h"
#include "guiConst.h"
#include "intConst.h"
#include "scaling.h"
#include "introTune.h"
#include <stdint.h>
#include <zlib.h>
#include <fmt/printf.h>
#include <stdexcept>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include "../utfutils.h"
#define LAYOUT_INI "\\layout.ini"
#define BACKUPS_DIR "\\backups"
#else
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#define LAYOUT_INI "/layout.ini"
#define BACKUPS_DIR "/backups"
#endif

#ifdef IS_MOBILE
#define MOBILE_UI_DEFAULT true
#else
#define MOBILE_UI_DEFAULT false
#endif

#include "actionUtil.h"

#ifdef HAVE_SNDFILE
#include <sndfile.h>
#endif

bool Particle::update(float frameTime) {
  pos.x+=speed.x*frameTime;
  pos.y+=speed.y*frameTime;
  speed.x*=1.0-((1.0-friction)*frameTime);
  speed.y*=1.0-((1.0-friction)*frameTime);
  speed.y+=gravity*frameTime;
  life-=lifeSpeed*frameTime;
  return (life>0);
}

void FurnaceGUI::centerNextWindow(const char* name, float w, float h) {
  if (ImGui::IsPopupOpen(name)) {
    if (settings.centerPopup) {
      ImGui::SetNextWindowPos(ImVec2(w*0.5,h*0.5),ImGuiCond_Always,ImVec2(0.5,0.5));
    }
  }
}

void FurnaceGUI::bindEngine(DivEngine* eng) {
  e=eng;
  wavePreview.setEngine(e);
}

void FurnaceGUI::enableSafeMode() {
  safeMode=true;
}

const char* FurnaceGUI::noteName(short note, short octave) {
  if (note==100) {
    return noteOffLabel;
  } else if (note==101) { // note off and envelope release
    return noteRelLabel;
  } else if (note==102) { // envelope release only
    return macroRelLabel;
  } else if (octave==0 && note==0) {
    return emptyLabel;
  } else if (note==0 && octave!=0) {
    return "BUG";
  }
  int seek=(note+(signed char)octave*12)+60;
  if (seek<0 || seek>=180) {
    return "???";
  }
  if (settings.flatNotes) {
    if (settings.germanNotation) return noteNamesGF[seek];
    return noteNamesF[seek];
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

void FurnaceGUI::encodeMMLStr(String& target, int* macro, int macroLen, int macroLoop, int macroRel, bool hex, bool bit30) {
  target="";
  char buf[32];
  for (int i=0; i<macroLen; i++) {
    if (i==macroLoop) target+="| ";
    if (i==macroRel) target+="/ ";
    if (bit30 && ((macro[i]&0xc0000000)==0x40000000 || (macro[i]&0xc0000000)==0x80000000)) target+="@";
    int macroVal=macro[i];
    if (macro[i]<0) {
      if (!(macroVal&0x40000000)) macroVal|=0x40000000;
    } else {
      if (macroVal&0x40000000) macroVal&=~0x40000000;
    }
    if (hex) {
      if (i==macroLen-1) {
        snprintf(buf,31,"%.2X",macroVal);
      } else {
        snprintf(buf,31,"%.2X ",macroVal);
      }
    } else {
      if (i==macroLen-1) {
        snprintf(buf,31,"%d",macroVal);
      } else {
        snprintf(buf,31,"%d ",macroVal);
      }
    }
    target+=buf;
  }
}

void FurnaceGUI::decodeMMLStrW(String& source, int* macro, int& macroLen, int macroMin, int macroMax, bool hex) {
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
          macro[macroLen]=negaBuf?-buf:buf;
          negaBuf=false;
          if (macro[macroLen]<macroMin) macro[macroLen]=macroMin;
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
    macro[macroLen]=negaBuf?-buf:buf;
    negaBuf=false;
    if (macro[macroLen]<macroMin) macro[macroLen]=macroMin;
    if (macro[macroLen]>macroMax) macro[macroLen]=macroMax;
    macroLen++;
    buf=0;
  }
}

void FurnaceGUI::decodeMMLStr(String& source, int* macro, unsigned char& macroLen, unsigned char& macroLoop, int macroMin, int macroMax, unsigned char& macroRel, bool bit30) {
  int buf=0;
  bool negaBuf=false;
  bool setBit30=false;
  bool hasVal=false;
  macroLen=0;
  macroLoop=255;
  macroRel=255;
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
      case '@':
        if (bit30) {
          setBit30=true;
        }
        break;
      case ' ':
        if (hasVal) {
          hasVal=false;
          macro[macroLen]=negaBuf?-buf:buf;
          negaBuf=false;
          if (macro[macroLen]<macroMin) macro[macroLen]=macroMin;
          if (macro[macroLen]>macroMax) macro[macroLen]=macroMax;
          if (setBit30) macro[macroLen]^=0x40000000;
          setBit30=false;
          macroLen++;
          buf=0;
        }
        break;
      case '|':
        if (hasVal) {
          hasVal=false;
          macro[macroLen]=negaBuf?-buf:buf;
          negaBuf=false;
          if (macro[macroLen]<macroMin) macro[macroLen]=macroMin;
          if (macro[macroLen]>macroMax) macro[macroLen]=macroMax;
          if (setBit30) macro[macroLen]^=0x40000000;
          setBit30=false;
          macroLen++;
          buf=0;
        }
        if (macroLoop==255) {
          macroLoop=macroLen;
        }
        break;
      case '/':
        if (hasVal) {
          hasVal=false;
          macro[macroLen]=negaBuf?-buf:buf;
          negaBuf=false;
          if (macro[macroLen]<macroMin) macro[macroLen]=macroMin;
          if (macro[macroLen]>macroMax) macro[macroLen]=macroMax;
          if (setBit30) macro[macroLen]^=0x40000000;
          setBit30=false;
          macroLen++;
          buf=0;
        }
        if (macroRel==255) {
          macroRel=macroLen;
        }
        break;
    }
    if (macroLen>=255) break;
  }
  if (hasVal && macroLen<255) {
    hasVal=false;
    macro[macroLen]=negaBuf?-buf:buf;
    negaBuf=false;
    if (macro[macroLen]<macroMin) macro[macroLen]=macroMin;
    if (macro[macroLen]>macroMax) macro[macroLen]=macroMax;
    if (setBit30) macro[macroLen]^=0x40000000;
    setBit30=false;
    macroLen++;
    buf=0;
    MARK_MODIFIED;
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

bool FurnaceGUI::isCtrlWheelModifierHeld() const {
  switch (settings.ctrlWheelModifier) {
    case 0:
      return ImGui::IsKeyDown(ImGuiMod_Ctrl) || ImGui::IsKeyDown(ImGuiMod_Super);
    case 1:
      return ImGui::IsKeyDown(ImGuiMod_Ctrl);
    case 2:
      return ImGui::IsKeyDown(ImGuiMod_Super);
    case 3:
      return ImGui::IsKeyDown(ImGuiMod_Alt);
    default:
      return false;
  }
}

bool FurnaceGUI::CWSliderScalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags) {
  flags^=ImGuiSliderFlags_AlwaysClamp;
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
  flags^=ImGuiSliderFlags_AlwaysClamp;
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

bool FurnaceGUI::InvCheckbox(const char* label, bool* value) {
  bool t=!(*value);
  if (ImGui::Checkbox(label,&t)) {
    *value=!t;
    return true;
  }
  return false;
}

bool FurnaceGUI::LocalizedComboGetter(void* data, int idx, const char** out_text) {
  const char* const* items=(const char* const*)data;
  if (out_text) *out_text=_(items[idx]);
  return true;
}

void FurnaceGUI::sameLineMaybe(float width) {
  if (width<0.0f) width=ImGui::GetFrameHeight();

  ImGui::SameLine();
  if (ImGui::GetContentRegionAvail().x<width) ImGui::NewLine();
}

const char* FurnaceGUI::getSystemName(DivSystem which) {
  /*
  if (settings.chipNames) {
    return e->getSystemChips(which);
  }
  */
  return e->getSystemName(which);
}

void FurnaceGUI::updateScroll(int amount) {
  float lineHeight=(patFont->FontSize+2*dpiScale);
  nextScroll=lineHeight*amount;
  haveHitBounds=false;
}

void FurnaceGUI::addScroll(int amount) {
  float lineHeight=(patFont->FontSize+2*dpiScale);
  nextAddScroll=lineHeight*amount;
  haveHitBounds=false;
}

void FurnaceGUI::addScrollX(int amount) {
  float lineHeight=(patFont->FontSize+2*dpiScale);
  nextAddScrollX=lineHeight*amount;
  haveHitBounds=false;
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
  backupLock.lock();
  if (GetFullPathNameW(ws.c_str(),4095,ret,NULL)==0) {
    curFileName=name;
  } else {
    curFileName=utf16To8(ret);
  }
  backupLock.unlock();
#else
  char ret[4096];
  backupLock.lock();
  if (realpath(name.c_str(),ret)==NULL) {
    curFileName=name;
  } else {
    curFileName=ret;
  }
  backupLock.unlock();
#endif
  updateWindowTitle();
  pushRecentFile(curFileName);
  if (settings.alwaysPlayIntro==2) shortIntro=true;
}

void FurnaceGUI::updateWindowTitle() {
  String title;
  switch (settings.titleBarInfo) {
    case 0:
      title="Furnace";
      break;
    case 1:
      if (e->song.name.empty()) {
        title="Furnace";
      } else {
        title=fmt::sprintf("%s - Furnace",e->song.name);
      }
      break;
    case 2:
      if (curFileName.empty()) {
        title="Furnace";
      } else {
        String shortName;
        size_t pos=curFileName.rfind(DIR_SEPARATOR);
        if (pos==String::npos) {
          shortName=curFileName;
        } else {
          shortName=curFileName.substr(pos+1);
        }
        title=fmt::sprintf("%s - Furnace",shortName);
      }
      break;
    case 3:
      if (curFileName.empty()) {
        title="Furnace";
      } else {
        title=fmt::sprintf("%s - Furnace",curFileName);
      }
      break;
  }

  if (settings.titleBarSys) {
    if (e->song.systemName!="") {
      title+=fmt::sprintf(" (%s)",e->song.systemName);
    }
  }

  if (sdlWin!=NULL) SDL_SetWindowTitle(sdlWin,title.c_str());

  if (e->song.insLen==1) {
    unsigned int checker=0x11111111;
    unsigned int checker1=0;
    DivInstrument* ins=e->getIns(0);
    if (ins->name.size()==15 && e->curSubSong->ordersLen==8) {
      for (int i=0; i<15; i++) {
        checker^=ins->name[i]<<i;
        checker1+=ins->name[i];
        checker=(checker>>1|(((checker)^(checker>>2)^(checker>>3)^(checker>>5))&1)<<31);
        checker1<<=1;
      }
      if (checker==0x5ec4497d && checker1==0x6347ee) nonLatchNibble=true;
    }
  }
}

void FurnaceGUI::autoDetectSystemIter(std::vector<FurnaceGUISysDef>& category, bool& isMatch, std::map<DivSystem,int>& defCountMap, std::map<DivSystem,DivConfig>& defConfMap, std::map<DivSystem,int>& sysCountMap, std::map<DivSystem,DivConfig>& sysConfMap) {
  for (FurnaceGUISysDef& j: category) {
    if (!j.orig.empty()) {
      defCountMap.clear();
      defConfMap.clear();
      for (FurnaceGUISysDefChip& k: j.orig) {
        auto it=defCountMap.find(k.sys);
        if (it==defCountMap.cend()) {
          defCountMap[k.sys]=1;
        } else {
          it->second++;
        }
        DivConfig dc;
        dc.loadFromMemory(k.flags.c_str());
        defConfMap[k.sys]=dc;
      }
      if (defCountMap.size()==sysCountMap.size()) {
        isMatch=true;
        /*logV("trying on defCountMap: %s",j.name);
        for (std::pair<DivSystem,int> k: defCountMap) {
          logV("- %s: %d",e->getSystemName(k.first),k.second);
        }*/
        for (std::pair<DivSystem,int> k: defCountMap) {
          auto countI=sysCountMap.find(k.first);
          if (countI==sysCountMap.cend()) {
            isMatch=false;
            break;
          } else if (countI->second!=k.second) {
            isMatch=false;
            break;
          }

          auto confI=sysConfMap.find(k.first);
          if (confI==sysConfMap.cend()) {
            isMatch=false;
            break;
          }
          DivConfig& sysDC=confI->second;
          auto defConfI=defConfMap.find(k.first);
          if (defConfI==defConfMap.cend()) {
            isMatch=false;
            break;
          }
          for (std::pair<String,String> l: defConfI->second.configMap()) {
            if (!sysDC.has(l.first)) {
              isMatch=false;
              break;
            }
            if (sysDC.getString(l.first,"")!=l.second) {
              isMatch=false;
              break;
            }
          }
          if (!isMatch) break;
        }
        if (isMatch) {
          logV("match found!");
          e->song.systemName=j.name;
          break;
        }
      }
    }
    if (!j.subDefs.empty()) autoDetectSystemIter(j.subDefs,isMatch,defCountMap,defConfMap,sysCountMap,sysConfMap);
    if (isMatch) break;
  }
}

void FurnaceGUI::autoDetectSystem() {
  std::map<DivSystem,int> sysCountMap;
  std::map<DivSystem,DivConfig> sysConfMap;
  for (int i=0; i<e->song.systemLen; i++) {
    auto it=sysCountMap.find(e->song.system[i]);
    if (it==sysCountMap.cend()) {
      sysCountMap[e->song.system[i]]=1;
    } else {
      it->second++;
    }
    sysConfMap[e->song.system[i]]=e->song.systemFlags[i];
  }

  logV("sysCountMap:");
  for (std::pair<DivSystem,int> k: sysCountMap) {
    logV("%s: %d",e->getSystemName(k.first),k.second);
  }

  bool isMatch=false;
  std::map<DivSystem,int> defCountMap;
  std::map<DivSystem,DivConfig> defConfMap;
  for (FurnaceGUISysCategory& i: sysCategories) {
    autoDetectSystemIter(i.systems,isMatch,defCountMap,defConfMap,sysCountMap,sysConfMap);
    if (isMatch) break;
  }

  if (!isMatch) {
    bool isFirst=true;
    e->song.systemName="";
    for (std::pair<DivSystem,int> k: sysCountMap) {
      if (!isFirst) e->song.systemName+=" + ";
      if (k.second>1) {
        e->song.systemName+=fmt::sprintf("%d×",k.second);
      }
      e->song.systemName+=e->getSystemName(k.first);
      isFirst=false;
    }
  }
}

void FurnaceGUI::updateROMExportAvail() {
  memset(romExportAvail,0,sizeof(bool)*DIV_ROM_MAX);
  romExportExists=false;

  for (int i=0; i<DIV_ROM_MAX; i++) {
    if (e->isROMExportViable((DivROMExportOptions)i)) {
      romExportAvail[i]=true;
      romExportExists=true;
    }
  }

  if (!romExportAvail[romTarget]) {
    // find a new one
    romTarget=DIV_ROM_ABSTRACT;
    for (int i=0; i<DIV_ROM_MAX; i++) {
      const DivROMExportDef* newDef=e->getROMExportDef((DivROMExportOptions)i);
      if (newDef!=NULL) {
        if (romExportAvail[i]) {
          romTarget=(DivROMExportOptions)i;
          romMultiFile=newDef->multiOutput;
          romConfig=DivConfig();
          if (newDef->fileExt==NULL) {
            romFilterName="";
            romFilterExt="";
          } else {
            romFilterName=newDef->fileType;
            romFilterExt=newDef->fileExt;
          }
          break;
        }
      }
    }
  }
}

ImVec4 FurnaceGUI::channelColor(int ch) {
  switch (settings.channelColors) {
    case 0:
      return uiColors[GUI_COLOR_CHANNEL_BG];
      break;
    case 1:
      return uiColors[GUI_COLOR_CHANNEL_FM+e->getChannelType(ch)];
      break;
    case 2:
      return uiColors[GUI_COLOR_INSTR_STD+e->getPreferInsType(ch)];
      break;
  }
  // invalid
  return uiColors[GUI_COLOR_TEXT];
}

ImVec4 FurnaceGUI::channelTextColor(int ch) {
  switch (settings.channelTextColors) {
    case 0:
      return uiColors[GUI_COLOR_CHANNEL_FG];
      break;
    case 1:
      return uiColors[GUI_COLOR_CHANNEL_FM+e->getChannelType(ch)];
      break;
    case 2:
      return uiColors[GUI_COLOR_INSTR_STD+e->getPreferInsType(ch)];
      break;
  }
  // invalid
  return uiColors[GUI_COLOR_TEXT];
}

const char* defaultLayout="[Window][DockSpaceViewport_11111111]\n\
Pos=0,24\n\
Size=1280,776\n\
Collapsed=0\n\
\n\
[Window][Debug##Default]\n\
Pos=54,43\n\
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
Size=302,179\n\
Collapsed=0\n\
DockId=0x0000000F,0\n\
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
DockId=0x00000006,0\n\
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
DockId=0x00000006,2\n\
\n\
[Window][Pattern]\n\
Pos=0,243\n\
Size=939,557\n\
Collapsed=0\n\
DockId=0x00000017,0\n\
\n\
[Window][Instrument Editor]\n\
Pos=229,126\n\
Size=856,652\n\
Collapsed=0\n\
\n\
[Window][Warning]\n\
Pos=481,338\n\
Size=264,86\n\
Collapsed=0\n\
\n\
[Window][Sample Editor]\n\
Pos=47,216\n\
Size=1075,525\n\
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
Size=1280,941\n\
Collapsed=0\n\
\n\
[Window][Error]\n\
Pos=491,342\n\
Size=514,71\n\
Collapsed=0\n\
\n\
[Window][Mixer]\n\
Pos=429,198\n\
Size=453,355\n\
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
Size=32,557\n\
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
Pos=0,581\n\
Size=1246,219\n\
Collapsed=0\n\
DockId=0x00000016,0\n\
\n\
[Window][Warning##Export VGMFileDialogOverWriteDialog]\n\
Pos=390,351\n\
Size=500,71\n\
Collapsed=0\n\
\n\
[Window][Compatibility Flags]\n\
Pos=388,132\n\
Size=580,641\n\
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
Pos=829,243\n\
Size=417,557\n\
Collapsed=0\n\
DockId=0x00000014,0\n\
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
[Window][Subsongs]\n\
Pos=978,24\n\
Size=302,217\n\
Collapsed=0\n\
DockId=0x00000010,1\n\
\n\
[Window][Oscilloscope (per-channel)]\n\
Pos=1095,243\n\
Size=151,557\n\
Collapsed=0\n\
DockId=0x00000012,0\n\
\n\
[Window][Piano]\n\
Pos=177,669\n\
Size=922,118\n\
Collapsed=0\n\
\n\
[Window][Log Viewer]\n\
Pos=60,60\n\
Size=541,637\n\
Collapsed=0\n\
\n\
[Window][Pattern Manager]\n\
Pos=60,60\n\
Size=1099,366\n\
Collapsed=0\n\
\n\
[Window][Chip Manager]\n\
Pos=60,60\n\
Size=490,407\n\
Collapsed=0\n\
\n\
[Window][Speed]\n\
Pos=978,24\n\
Size=302,217\n\
Collapsed=0\n\
DockId=0x00000010,2\n\
\n\
[Window][Song Info##Song Information]\n\
Pos=978,24\n\
Size=302,217\n\
Collapsed=0\n\
DockId=0x00000010,0\n\
\n\
[Window][Effect List]\n\
Pos=941,243\n\
Size=305,557\n\
Collapsed=0\n\
DockId=0x00000018,0\n\
\n\
[Window][Intro]\n\
Pos=0,0\n\
Size=2560,1600\n\
Collapsed=0\n\
\n\
[Window][Welcome]\n\
Pos=944,666\n\
Size=672,268\n\
Collapsed=0\n\
\n\
[Window][Grooves]\n\
Pos=416,314\n\
Size=463,250\n\
Collapsed=0\n\
\n\
[Window][Clock]\n\
Pos=60,60\n\
Size=145,184\n\
Collapsed=0\n\
\n\
[Window][Oscilloscope (X-Y)]\n\
Pos=60,60\n\
Size=300,300\n\
Collapsed=0\n\
\n\
[Docking][Data]\n\
DockSpace             ID=0x8B93E3BD Window=0xA787BDB4 Pos=0,24 Size=1280,776 Split=Y Selected=0x6C01C512\n\
  DockNode            ID=0x00000001 Parent=0x8B93E3BD SizeRef=1280,217 Split=X Selected=0xF3094A52\n\
    DockNode          ID=0x00000003 Parent=0x00000001 SizeRef=976,231 Split=X Selected=0x65CC51DC\n\
      DockNode        ID=0x00000007 Parent=0x00000003 SizeRef=345,231 HiddenTabBar=1 Selected=0x8F5BFC9A\n\
      DockNode        ID=0x00000008 Parent=0x00000003 SizeRef=629,231 Split=X Selected=0xD2AD486B\n\
        DockNode      ID=0x00000005 Parent=0x00000008 SizeRef=304,406 Split=Y Selected=0x6D682373\n\
          DockNode    ID=0x00000009 Parent=0x00000005 SizeRef=292,175 Split=Y Selected=0x6D682373\n\
            DockNode  ID=0x0000000D Parent=0x00000009 SizeRef=292,68 HiddenTabBar=1 Selected=0xE57B1A9D\n\
            DockNode  ID=0x0000000E Parent=0x00000009 SizeRef=292,105 HiddenTabBar=1 Selected=0x6D682373\n\
          DockNode    ID=0x0000000A Parent=0x00000005 SizeRef=292,40 HiddenTabBar=1 Selected=0x0DE44CFF\n\
        DockNode      ID=0x00000006 Parent=0x00000008 SizeRef=323,406 Selected=0xB75D68C7\n\
    DockNode          ID=0x00000004 Parent=0x00000001 SizeRef=302,231 Split=Y Selected=0x60B9D088\n\
      DockNode        ID=0x0000000F Parent=0x00000004 SizeRef=302,179 Selected=0x60B9D088\n\
      DockNode        ID=0x00000010 Parent=0x00000004 SizeRef=302,36 Selected=0x82BEE2E5\n\
  DockNode            ID=0x00000002 Parent=0x8B93E3BD SizeRef=1280,512 Split=X Selected=0x6C01C512\n\
    DockNode          ID=0x0000000B Parent=0x00000002 SizeRef=1246,503 Split=X Selected=0xB9ADD0D5\n\
      DockNode        ID=0x00000011 Parent=0x0000000B SizeRef=1093,557 Split=X Selected=0xB9ADD0D5\n\
        DockNode      ID=0x00000013 Parent=0x00000011 SizeRef=827,557 Split=Y Selected=0xB9ADD0D5\n\
          DockNode    ID=0x00000015 Parent=0x00000013 SizeRef=1246,336 Split=X Selected=0xB9ADD0D5\n\
            DockNode  ID=0x00000017 Parent=0x00000015 SizeRef=939,557 CentralNode=1 HiddenTabBar=1 Selected=0xB9ADD0D5\n\
            DockNode  ID=0x00000018 Parent=0x00000015 SizeRef=305,557 Selected=0xB94874DD\n\
          DockNode    ID=0x00000016 Parent=0x00000013 SizeRef=1246,219 Selected=0xAD8E88F2\n\
        DockNode      ID=0x00000014 Parent=0x00000011 SizeRef=417,557 Selected=0x425428FB\n\
      DockNode        ID=0x00000012 Parent=0x0000000B SizeRef=151,557 HiddenTabBar=1 Selected=0x4C07BC58\n\
    DockNode          ID=0x0000000C Parent=0x00000002 SizeRef=32,503 HiddenTabBar=1 Selected=0x644DA2C1\n";


void FurnaceGUI::prepareLayout() {
  FILE* check;
  check=ps_fopen(finalLayoutPath,"r");
  if (check!=NULL) {
    fclose(check);
    return;
  }

  // copy initial layout
  logI("loading default layout.");
  check=ps_fopen(finalLayoutPath,"w");
  if (check==NULL) {
    logW("could not write default layout!");
    return;
  }

  fwrite(defaultLayout,1,strlen(defaultLayout),check);
  fclose(check);
}

float FurnaceGUI::calcBPM(const DivGroovePattern& speeds, float hz, int vN, int vD) {
  float hl=e->curSubSong->hilightA;
  if (hl<=0.0f) hl=4.0f;
  float timeBase=e->curSubSong->timeBase+1;
  float speedSum=0;
  for (int i=0; i<MIN(16,speeds.len); i++) {
    speedSum+=speeds.val[i];
  }
  speedSum/=MAX(1,speeds.len);
  if (timeBase<1.0f) timeBase=1.0f;
  if (speedSum<1.0f) speedSum=1.0f;
  if (vD<1) vD=1;
  return (60.0f*hz/(timeBase*hl*speedSum))*(float)vN/(float)vD;
}

void FurnaceGUI::play(int row) {
  if (e->getStreamPlayer()) {
    e->killStream();
  }
  memset(chanOscVol,0,DIV_MAX_CHANS*sizeof(float));
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    chanOscChan[i].pitch=0.0f;
  }
  memset(chanOscBright,0,DIV_MAX_CHANS*sizeof(float));
  e->walkSong(loopOrder,loopRow,loopEnd);
  memset(lastIns,-1,sizeof(int)*DIV_MAX_CHANS);
  if (followPattern) makeCursorUndo();
  if (!followPattern) e->setOrder(curOrder);
  if (row>=0) {
    if (!e->playToRow(row)) {
      showError(_("the song is over!"));
    }
  } else {
    if (!e->play()) {
      showError(_("the song is over!"));
    }
  }
  curNibble=false;
  orderNibble=false;
  activeNotes.clear();
}

void FurnaceGUI::setOrder(unsigned char order, bool forced) {
  curOrder=order;
  if (followPattern || forced) {
    e->setOrder(order);
  }
}

void FurnaceGUI::stop() {
  bool wasPlaying=e->isPlaying();
  e->walkSong(loopOrder,loopRow,loopEnd);
  e->stop();
  curNibble=false;
  orderNibble=false;
  if (followPattern && wasPlaying) {
    nextScroll=-1.0f;
    nextAddScroll=0.0f;
    e->getPlayPos(curOrder, cursor.y);
    if (selStart.xCoarse==selEnd.xCoarse && selStart.xFine==selEnd.xFine && selStart.y==selEnd.y && !selecting) {
      selStart=cursor;
      selEnd=cursor;
    }
    updateScroll(cursor.y);
  }
}

void FurnaceGUI::previewNote(int refChan, int note, bool autoNote) {
  e->setMidiBaseChan(refChan);
  e->synchronized([this,note]() {
    if (!e->autoNoteOn(-1,curIns,note)) failedNoteOn=true;
  });
}

void FurnaceGUI::stopPreviewNote(SDL_Scancode scancode, bool autoNote) {
  auto it=noteKeys.find(scancode);
  if (it!=noteKeys.cend()) {
    int key=it->second;
    int num=12*curOctave+key;
    if (num<-60) num=-60; // C-(-5)
    if (num>119) num=119; // B-9

    if (key==100) return;
    if (key==101) return;
    if (key==102) return;

    e->synchronized([this,num]() {
      e->autoNoteOff(-1,num);
      failedNoteOn=false;
    });
  }
}

void FurnaceGUI::noteInput(int num, int key, int vol) {
  int ch=cursor.xCoarse;
  int ord=curOrder;
  int y=cursor.y;
  int tick=0;
  int speed=0;

  if (e->isPlaying() && !e->isStepping() && followPattern) {
    e->getPlayPosTick(ord,y,tick,speed);
    if (tick<=(speed/2)) { // round
      // TODO: detect 0Dxx/0Bxx?
      if (++y>=e->curSubSong->patLen) {
        y=0;
        if (++ord>=e->curSubSong->ordersLen) {
          ord=0;
        }
      }
    }
  }

  logV("chan %d, %d:%d %d/%d",ch,ord,y,tick,speed);

  DivPattern* pat=e->curPat[ch].getPattern(e->curOrders->ord[ch][ord],true);
  bool removeIns=false;

  prepareUndo(GUI_UNDO_PATTERN_EDIT);

  if (key==GUI_NOTE_OFF) { // note off
    pat->data[y][0]=100;
    pat->data[y][1]=0;
    removeIns=true;
  } else if (key==GUI_NOTE_OFF_RELEASE) { // note off + env release
    pat->data[y][0]=101;
    pat->data[y][1]=0;
    removeIns=true;
  } else if (key==GUI_NOTE_RELEASE) { // env release only
    pat->data[y][0]=102;
    pat->data[y][1]=0;
    removeIns=true;
  } else {
    pat->data[y][0]=num%12;
    pat->data[y][1]=num/12;
    if (pat->data[y][0]==0) {
      pat->data[y][0]=12;
      pat->data[y][1]--;
    }
    pat->data[y][1]=(unsigned char)pat->data[y][1];
    if (latchIns==-2) {
      if (curIns>=(int)e->song.ins.size()) curIns=-1;
      if (curIns>=0) {
        pat->data[y][2]=curIns;
      }
    } else if (latchIns!=-1 && !e->song.ins.empty()) {
      pat->data[y][2]=MIN(((int)e->song.ins.size())-1,latchIns);
    }
    int maxVol=e->getMaxVolumeChan(ch);
    if (latchVol!=-1) {
      pat->data[y][3]=MIN(maxVol,latchVol);
    } else if (vol!=-1) {
      pat->data[y][3]=e->mapVelocity(ch,pow((float)vol/127.0f,midiMap.volExp));
    }
    if (latchEffect!=-1) pat->data[y][4]=latchEffect;
    if (latchEffectVal!=-1) pat->data[y][5]=latchEffectVal;
  }
  if (removeIns) {
    if (settings.removeInsOff) {
      pat->data[y][2]=-1;
    }
    if (settings.removeVolOff) {
      pat->data[y][3]=-1;
    }
  }
  makeUndo(GUI_UNDO_PATTERN_EDIT);
  editAdvance();
  curNibble=false;
}

void FurnaceGUI::valueInput(int num, bool direct, int target) {
  int ch=cursor.xCoarse;
  int ord=curOrder;
  int y=cursor.y;

  if (e->isPlaying() && !e->isStepping() && followPattern) {
    e->getPlayPos(ord,y);
  }

  DivPattern* pat=e->curPat[ch].getPattern(e->curOrders->ord[ch][ord],true);
  prepareUndo(GUI_UNDO_PATTERN_EDIT);
  if (target==-1) target=cursor.xFine+1;
  if (direct) {
    pat->data[y][target]=num&0xff;
  } else {
    if (pat->data[y][target]==-1) pat->data[y][target]=0;
    if (!settings.pushNibble && !curNibble) {
      pat->data[y][target]=num;
    } else {
      pat->data[y][target]=((pat->data[y][target]<<4)|num)&0xff;
    }
  }
  if (cursor.xFine==1) { // instrument
    if (pat->data[y][target]>=(int)e->song.ins.size()) {
      pat->data[y][target]&=0x0f;
      if (pat->data[y][target]>=(int)e->song.ins.size()) {
        pat->data[y][target]=(int)e->song.ins.size()-1;
      }
    }
    if (settings.absorbInsInput) {
      curIns=pat->data[y][target];
      wavePreviewInit=true;
      updateFMPreview=true;
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
      if (pat->data[y][target]>e->getMaxVolumeChan(ch)) pat->data[y][target]=e->getMaxVolumeChan(ch);
    } else {
      pat->data[y][target]&=15;
    }
    makeUndo(GUI_UNDO_PATTERN_EDIT);
    if (direct) {
      curNibble=false;
    } else {
      if (e->getMaxVolumeChan(ch)<16) {
        curNibble=false;
        if (pat->data[y][target]>e->getMaxVolumeChan(ch)) pat->data[y][target]=e->getMaxVolumeChan(ch);
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
      if (!curNibble) {
        if (!settings.effectCursorDir) {
          editAdvance();
        } else {
          if (settings.effectCursorDir==2) {
            if (++cursor.xFine>=(3+(e->curPat[ch].effectCols*2))) {
              cursor.xFine=3;
            }
          } else {
            if (cursor.xFine&1) {
              cursor.xFine++;
            } else {
              editAdvance();
              cursor.xFine--;
            }
          }
        }
      }
    }
  }
}

void FurnaceGUI::orderInput(int num) {
  if (orderCursor>=0 && orderCursor<e->getTotalChannelCount()) {
    prepareUndo(GUI_UNDO_CHANGE_ORDER);
    e->lockSave([this,num]() {
      if (!curNibble && !settings.pushNibble) e->curOrders->ord[orderCursor][curOrder]=0;
      e->curOrders->ord[orderCursor][curOrder]=((e->curOrders->ord[orderCursor][curOrder]<<4)|num);
    });
    MARK_MODIFIED;
    curNibble=!curNibble;
    if (orderEditMode==2 || orderEditMode==3) {
      if (!curNibble) {
        if (orderEditMode==2) {
          orderCursor++;
          if (orderCursor>=e->getTotalChannelCount()) orderCursor=0;
        } else if (orderEditMode==3) {
          if (curOrder<e->curSubSong->ordersLen-1) {
            setOrder(curOrder+1);
          }
        }
      }
    }
    e->walkSong(loopOrder,loopRow,loopEnd);
    makeUndo(GUI_UNDO_CHANGE_ORDER);
  }
}

#define changeLatch(x) \
  if (x<0) x=0; \
  if (!latchNibble && !settings.pushNibble) x=0; \
  x=(x<<4)|num; \
  latchNibble=!latchNibble; \
  if (!latchNibble) { \
    if (++latchTarget>4) latchTarget=0; \
  }

void FurnaceGUI::keyDown(SDL_Event& ev) {
  if (introPos<11.0 && !shortIntro) return;
  if (aboutOpen) return;
  if (cvOpen) return;

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

  if (!ImGuiFileDialog::Instance()->IsOpened()) {
    if (bindSetActive) {
      if (!ev.key.repeat) {
        switch (ev.key.keysym.sym) {
          case SDLK_LCTRL: case SDLK_RCTRL:
          case SDLK_LALT: case SDLK_RALT:
          case SDLK_LGUI: case SDLK_RGUI:
          case SDLK_LSHIFT: case SDLK_RSHIFT:
            bindSetPending=false;
            actionKeys[bindSetTarget][bindSetTargetIdx]=(mapped&(~FURK_MASK))|0xffffff;
            break;
          default:
            actionKeys[bindSetTarget][bindSetTargetIdx]=mapped;

            // de-dupe with an n^2 algorithm that will never ever be a problem (...but for real though)
            for (size_t i=0; i<actionKeys[bindSetTarget].size(); i++) {
              for (size_t j=i+1; j<actionKeys[bindSetTarget].size(); j++) {
                if (actionKeys[bindSetTarget][i]==actionKeys[bindSetTarget][j]) {
                  actionKeys[bindSetTarget].erase(actionKeys[bindSetTarget].begin()+j);
                }
              }
            }

            bindSetActive=false;
            bindSetPending=false;
            bindSetTarget=0;
            bindSetTargetIdx=0;
            bindSetPrevValue=0;
            parseKeybinds();
            break;
        }
      }
      return;
    }

    if (latchTarget) {
      if (mapped==SDLK_DELETE || mapped==SDLK_BACKSPACE) {
        switch (latchTarget) {
          case 1:
            latchIns=-1;
            break;
          case 2:
            latchVol=-1;
            break;
          case 3:
            latchEffect=-1;
            break;
          case 4:
            latchEffectVal=-1;
            break;
        }
      } else {
        auto it=valueKeys.find(ev.key.keysym.sym);
        if (it!=valueKeys.cend()) {
          int num=it->second;
          switch (latchTarget) {
            case 1: // instrument
              changeLatch(latchIns);
              break;
            case 2: // volume
              changeLatch(latchVol);
              break;
            case 3: // effect
              changeLatch(latchEffect);
              break;
            case 4: // effect value
              changeLatch(latchEffectVal);
              break;
          }
        }
      }
      return;
    }

    if (sampleMapWaitingInput) {
      switch (sampleMapColumn) {
        case 0: {
          if (ev.key.keysym.scancode==SDL_SCANCODE_DELETE) {
            alterSampleMap(0,-1);
            return;
          }
          auto it=valueKeys.find(ev.key.keysym.sym);
          if (it!=valueKeys.cend()) {
            int num=it->second;
            if (num<10) {
              alterSampleMap(0,num);
              return;
            }
          }
          break;
        }
        case 1: {
          if (ev.key.keysym.scancode==SDL_SCANCODE_DELETE) {
            alterSampleMap(1,-1);
            return;
          }
          auto it=noteKeys.find(ev.key.keysym.scancode);
          if (it!=noteKeys.cend()) {
            int key=it->second;
            int num=12*curOctave+key;

            if (num<-60) num=-60; // C-(-5)
            if (num>119) num=119; // B-9

            alterSampleMap(1,num);
            return;
          }
          break;
        }
        case 2: {
          if (ev.key.keysym.scancode==SDL_SCANCODE_DELETE) {
            alterSampleMap(2,-1);
            return;
          }
          auto it=valueKeys.find(ev.key.keysym.sym);
          if (it!=valueKeys.cend()) {
            int num=it->second;
            if (num<10) {
              alterSampleMap(2,num);
              return;
            }
          }
          break;
        }
        case 3: {
          if (ev.key.keysym.scancode==SDL_SCANCODE_DELETE) {
            alterSampleMap(3,-1);
            return;
          }
          auto it=valueKeys.find(ev.key.keysym.sym);
          if (it!=valueKeys.cend()) {
            int num=it->second;
            if (num<16) {
              alterSampleMap(3,num);
              return;
            }
          }
          break;
        }
      }
    }

    // PER-WINDOW KEYS
    switch (curWindow) {
      case GUI_WINDOW_PATTERN: {
        auto actionI=actionMapPat.find(mapped);
        if (actionI!=actionMapPat.cend()) {
          int action=actionI->second;
          if (action>0) {
            doAction(action);
            return;
          }
        }
        // pattern input otherwise
        if (mapped&(FURKMOD_ALT|FURKMOD_CTRL|FURKMOD_META|FURKMOD_SHIFT)) break;
        if (!ev.key.repeat || settings.inputRepeat) {
          if (cursor.xFine==0) { // note
            auto it=noteKeys.find(ev.key.keysym.scancode);
            if (it!=noteKeys.cend()) {
              int key=it->second;
              int num=12*curOctave+key;

              if (num<-60) num=-60; // C-(-5)
              if (num>119) num=119; // B-9

              if (edit) {
                noteInput(num,key);
              }
            }
          } else if (edit) { // value
            auto it=valueKeys.find(ev.key.keysym.sym);
            if (it!=valueKeys.cend()) {
              int num=it->second;
              valueInput(num);
            }
          }
        }
        break;
      }
      case GUI_WINDOW_ORDERS: {
        auto actionI=actionMapOrders.find(mapped);
        if (actionI!=actionMapOrders.cend()) {
          int action=actionI->second;
          if (action>0) {
            doAction(action);
            return;
          }
        }
        // order input otherwise
        if (mapped&(FURKMOD_ALT|FURKMOD_CTRL|FURKMOD_META|FURKMOD_SHIFT)) break;
        if (orderEditMode!=0) {
          auto it=valueKeys.find(ev.key.keysym.sym);
          if (it!=valueKeys.cend()) {
            int num=it->second;
            orderInput(num);
          }
        }
        break;
      }
      case GUI_WINDOW_SAMPLE_EDIT: {
        auto actionI=actionMapSample.find(mapped);
        if (actionI!=actionMapSample.cend()) {
          int action=actionI->second;
          if (action>0) {
            doAction(action);
            return;
          }
        }
        break;
      }
      case GUI_WINDOW_INS_LIST: {
        auto actionI=actionMapInsList.find(mapped);
        if (actionI!=actionMapInsList.cend()) {
          int action=actionI->second;
          if (action>0) {
            doAction(action);
            return;
          }
        }
        break;
      }
      case GUI_WINDOW_WAVE_LIST: {
        auto actionI=actionMapWaveList.find(mapped);
        if (actionI!=actionMapWaveList.cend()) {
          int action=actionI->second;
          if (action>0) {
            doAction(action);
            return;
          }
        }
        break;
      }
      case GUI_WINDOW_SAMPLE_LIST: {
        auto actionI=actionMapSampleList.find(mapped);
        if (actionI!=actionMapSampleList.cend()) {
          int action=actionI->second;
          if (action>0) {
            doAction(action);
            return;
          }
        }
        break;
      }
      default:
        break;
    }
  }

  // GLOBAL KEYS
  auto actionI=actionMapGlobal.find(mapped);
  if (actionI!=actionMapGlobal.cend()) {
    int action=actionI->second;
    if (action>0) {
      if (ImGuiFileDialog::Instance()->IsOpened()) {
        if (action!=GUI_ACTION_OCTAVE_UP && action!=GUI_ACTION_OCTAVE_DOWN) return;
      }
      doAction(action);
      return;
    }
  }
}

void FurnaceGUI::keyUp(SDL_Event& ev) {
  // nothing for now
}

bool dirExists(String s) {
  return dirExists(s.c_str());
}

void FurnaceGUI::openFileDialog(FurnaceGUIFileDialogs type) {
  bool hasOpened=false;

  String shortName;
  size_t shortNamePos=curFileName.rfind(DIR_SEPARATOR);
  if (shortNamePos!=String::npos && (shortNamePos+1)<curFileName.size()) {
    shortName=curFileName.substr(shortNamePos+1);
    // remove extension
    shortNamePos=shortName.rfind('.');
    if (shortNamePos!=String::npos) {
      shortName=shortName.substr(0,shortNamePos);
    }
  }

  switch (type) {
    case GUI_FILE_OPEN:
      if (!dirExists(workingDirSong)) workingDirSong=getHomeDir();
      hasOpened=fileDialog->openLoad(
        _("Open File"),
        {_("compatible files"), "*.fur *.dmf *.mod *.s3m *.xm *.it *.fc13 *.fc14 *.smod *.fc *.ftm *.0cc *.dnm *.eft *.fub *.tfe",
         _("all files"), "*"},
        workingDirSong,
        dpiScale
      );
      break;
    case GUI_FILE_OPEN_BACKUP:
      if (!dirExists(backupPath)) {
        showError(_("no backups made yet!"));
        break;
      }
      hasOpened=fileDialog->openLoad(
        _("Restore Backup"),
        {_("Furnace song"), "*.fur"},
        backupPath+String(DIR_SEPARATOR_STR),
        dpiScale
      );
      break;
    case GUI_FILE_SAVE:
      if (!dirExists(workingDirSong)) workingDirSong=getHomeDir();
      hasOpened=fileDialog->openSave(
        _("Save File"),
        {_("Furnace song"), "*.fur"},
        workingDirSong,
        dpiScale
      );
      break;
    case GUI_FILE_SAVE_DMF:
      if (!dirExists(workingDirSong)) workingDirSong=getHomeDir();
      hasOpened=fileDialog->openSave(
        _("Export DMF"),
        {_("DefleMask 1.1.3 module"), "*.dmf"},
        workingDirSong,
        dpiScale,
        (settings.autoFillSave)?shortName:""
      );
      break;
    case GUI_FILE_SAVE_DMF_LEGACY:
      if (!dirExists(workingDirSong)) workingDirSong=getHomeDir();
      hasOpened=fileDialog->openSave(
        _("Export DMF"),
        {_("DefleMask 1.0/legacy module"), "*.dmf"},
        workingDirSong,
        dpiScale,
        (settings.autoFillSave)?shortName:""
      );
      break;
    case GUI_FILE_INS_OPEN:
    case GUI_FILE_INS_OPEN_REPLACE:
      prevIns=-3;
      if (prevInsData!=NULL) {
        delete prevInsData;
        prevInsData=NULL;
      }
      prevInsData=new DivInstrument;
      *prevInsData=*e->getIns(curIns);
      if (!dirExists(workingDirIns)) workingDirIns=getHomeDir();
      hasOpened=fileDialog->openLoad(
        _("Load Instrument"),
        {_("all compatible files"), "*.fui *.dmp *.tfi *.vgi *.s3i *.sbi *.opli *.opni *.y12 *.bnk *.ff *.gyb *.opm *.wopl *.wopn",
         _("Furnace instrument"), "*.fui",
         _("DefleMask preset"), "*.dmp",
         _("TFM Music Maker instrument"), "*.tfi",
         _("VGM Music Maker instrument"), "*.vgi",
         _("Scream Tracker 3 instrument"), "*.s3i",
         _("SoundBlaster instrument"), "*.sbi",
         _("Wohlstand OPL instrument"), "*.opli",
         _("Wohlstand OPN instrument"), "*.opni",
         _("Gens KMod patch dump"), "*.y12",
         _("BNK file (AdLib)"), "*.bnk",
         _("FF preset bank"), "*.ff",
         _("2612edit GYB preset bank"), "*.gyb",
         _("VOPM preset bank"), "*.opm",
         _("Wohlstand WOPL bank"), "*.wopl",
         _("Wohlstand WOPN bank"), "*.wopn",
         _("all files"), "*"},
        workingDirIns,
        dpiScale,
        [this](const char* path) {
          int sampleCountBefore=e->song.sampleLen;
          std::vector<DivInstrument*> instruments=e->instrumentFromFile(path,false);
          if (!instruments.empty()) {
            if (e->song.sampleLen!=sampleCountBefore) {
              e->renderSamplesP();
            }
            if (curFileDialog==GUI_FILE_INS_OPEN_REPLACE) {
              if (prevIns==-3) {
                prevIns=curIns;
              }
              if (prevIns>=0 && prevIns<=(int)e->song.ins.size()) {
                *e->song.ins[prevIns]=*instruments[0];
              }
            } else {
              e->loadTempIns(instruments[0]);
              if (curIns!=-2) {
                prevIns=curIns;
              }
              curIns=-2;
            }
          }
          for (DivInstrument* i: instruments) delete i;
        },
        (type==GUI_FILE_INS_OPEN)
      );
      break;
    case GUI_FILE_INS_SAVE:
      if (!dirExists(workingDirIns)) workingDirIns=getHomeDir();
      hasOpened=fileDialog->openSave(
        _("Save Instrument"),
        {_("Furnace instrument"), "*.fui"},
        workingDirIns,
        dpiScale,
        (settings.autoFillSave)?e->getIns(curIns)->name:""
      );
      break;
    case GUI_FILE_INS_SAVE_DMP:
      if (!dirExists(workingDirIns)) workingDirIns=getHomeDir();
      hasOpened=fileDialog->openSave(
        _("Save Instrument"),
        {_("DefleMask preset"), "*.dmp"},
        workingDirIns,
        dpiScale,
        (settings.autoFillSave)?e->getIns(curIns)->name:""
      );
      break;
    case GUI_FILE_INS_SAVE_ALL:
      if (!dirExists(workingDirIns)) workingDirIns=getHomeDir();
      hasOpened=fileDialog->openSelectDir(
        _("Save All Instruments"),
        workingDirIns,
        dpiScale
      );
      break;
    case GUI_FILE_WAVE_OPEN:
    case GUI_FILE_WAVE_OPEN_REPLACE:
      if (!dirExists(workingDirWave)) workingDirWave=getHomeDir();
      hasOpened=fileDialog->openLoad(
        _("Load Wavetable"),
        {_("compatible files"), "*.fuw *.dmw",
         _("all files"), "*"},
        workingDirWave,
        dpiScale,
        NULL, // TODO
        (type==GUI_FILE_WAVE_OPEN)
      );
      break;
    case GUI_FILE_WAVE_SAVE:
      if (!dirExists(workingDirWave)) workingDirWave=getHomeDir();
      hasOpened=fileDialog->openSave(
        _("Save Wavetable"),
        {_("Furnace wavetable"), ".fuw"},
        workingDirWave,
        dpiScale
      );
      break;
    case GUI_FILE_WAVE_SAVE_DMW:
      if (!dirExists(workingDirWave)) workingDirWave=getHomeDir();
      hasOpened=fileDialog->openSave(
        _("Save Wavetable"),
        {_("DefleMask wavetable"), ".dmw"},
        workingDirWave,
        dpiScale
      );
      break;
    case GUI_FILE_WAVE_SAVE_RAW:
      if (!dirExists(workingDirWave)) workingDirWave=getHomeDir();
      hasOpened=fileDialog->openSave(
        _("Save Wavetable"),
        {_("raw data"), ".raw"},
        workingDirWave,
        dpiScale
      );
      break;
    case GUI_FILE_WAVE_SAVE_ALL:
      if (!dirExists(workingDirWave)) workingDirWave=getHomeDir();
      hasOpened=fileDialog->openSelectDir(
        _("Save All Wavetables"),
        workingDirWave,
        dpiScale
      );
      break;
    case GUI_FILE_SAMPLE_OPEN:
    case GUI_FILE_SAMPLE_OPEN_REPLACE:
      if (!dirExists(workingDirSample)) workingDirSample=getHomeDir();
      hasOpened=fileDialog->openLoad(
        _("Load Sample"),
        audioLoadFormats,
        workingDirSample,
        dpiScale,
        NULL, // TODO
        (type==GUI_FILE_SAMPLE_OPEN)
      );
      break;
    case GUI_FILE_SAMPLE_OPEN_RAW:
    case GUI_FILE_SAMPLE_OPEN_REPLACE_RAW:
      if (!dirExists(workingDirSample)) workingDirSample=getHomeDir();
      hasOpened=fileDialog->openLoad(
        _("Load Raw Sample"),
        {_("all files"), "*"},
        workingDirSample,
        dpiScale
      );
      break;
    case GUI_FILE_SAMPLE_SAVE:
      if (!dirExists(workingDirSample)) workingDirSample=getHomeDir();
      hasOpened=fileDialog->openSave(
        _("Save Sample"),
        {_("Wave file"), "*.wav"},
        workingDirSample,
        dpiScale,
        (settings.autoFillSave)?e->getSample(curSample)->name:""
      );
      break;
    case GUI_FILE_SAMPLE_SAVE_RAW:
      if (!dirExists(workingDirSample)) workingDirSample=getHomeDir();
      hasOpened=fileDialog->openSave(
        _("Save Raw Sample"),
        {_("all files"), "*"},
        workingDirSample,
        dpiScale,
        (settings.autoFillSave)?e->getSample(curSample)->name:""
      );
      break;
    case GUI_FILE_SAMPLE_SAVE_ALL:
      if (!dirExists(workingDirSample)) workingDirSample=getHomeDir();
      hasOpened=fileDialog->openSelectDir(
        _("Save All Samples"),
        workingDirSample,
        dpiScale
      );
      break;
    case GUI_FILE_EXPORT_AUDIO_ONE:
      if (!dirExists(workingDirAudioExport)) workingDirAudioExport=getHomeDir();
      hasOpened=fileDialog->openSave(
        _("Export Audio"),
        {_("Wave file"), "*.wav"},
        workingDirAudioExport,
        dpiScale,
        (settings.autoFillSave)?shortName:""
      );
      break;
    case GUI_FILE_EXPORT_AUDIO_PER_SYS:
      if (!dirExists(workingDirAudioExport)) workingDirAudioExport=getHomeDir();
      hasOpened=fileDialog->openSave(
        _("Export Audio"),
        {_("Wave file"), "*.wav"},
        workingDirAudioExport,
        dpiScale,
        (settings.autoFillSave)?shortName:""
      );
      break;
    case GUI_FILE_EXPORT_AUDIO_PER_CHANNEL:
      if (!dirExists(workingDirAudioExport)) workingDirAudioExport=getHomeDir();
      hasOpened=fileDialog->openSave(
        _("Export Audio"),
        {_("Wave file"), "*.wav"},
        workingDirAudioExport,
        dpiScale,
        (settings.autoFillSave)?shortName:""
      );
      break;
    case GUI_FILE_EXPORT_VGM:
      if (!dirExists(workingDirVGMExport)) workingDirVGMExport=getHomeDir();
      hasOpened=fileDialog->openSave(
        _("Export VGM"),
        {_("VGM file"), "*.vgm"},
        workingDirVGMExport,
        dpiScale,
        (settings.autoFillSave)?shortName:""
      );
      break;
    case GUI_FILE_EXPORT_TEXT:
      if (!dirExists(workingDirROMExport)) workingDirROMExport=getHomeDir();
      hasOpened=fileDialog->openSave(
        _("Export Command Stream"),
        {_("text file"), "*.txt"},
        workingDirROMExport,
        dpiScale,
        (settings.autoFillSave)?shortName:""
      );
      break;
    case GUI_FILE_EXPORT_CMDSTREAM:
      if (!dirExists(workingDirROMExport)) workingDirROMExport=getHomeDir();
      hasOpened=fileDialog->openSave(
        _("Export Command Stream"),
        {_("binary file"), "*.bin"},
        workingDirROMExport,
        dpiScale,
        (settings.autoFillSave)?shortName:""
      );
      break;
    case GUI_FILE_EXPORT_ROM:
      if (!dirExists(workingDirROMExport)) workingDirROMExport=getHomeDir();
      if (romMultiFile) {
        hasOpened=fileDialog->openSelectDir(
          _("Export ROM"),
          workingDirROMExport,
          dpiScale
        );
      } else {
        hasOpened=fileDialog->openSave(
          _("Export ROM"),
          {romFilterName, "*"+romFilterExt},
          workingDirROMExport,
          dpiScale,
          (settings.autoFillSave)?shortName:""
        );
      }
      break;
    case GUI_FILE_LOAD_MAIN_FONT:
      if (!dirExists(workingDirFont)) workingDirFont=getHomeDir();
      hasOpened=fileDialog->openLoad(
        _("Select Font"),
        {_("compatible files"), "*.ttf *.otf *.ttc *.dfont *.pcf *.psf"},
        workingDirFont,
        dpiScale
      );
      break;
    case GUI_FILE_LOAD_HEAD_FONT:
      if (!dirExists(workingDirFont)) workingDirFont=getHomeDir();
      hasOpened=fileDialog->openLoad(
        _("Select Font"),
        {_("compatible files"), "*.ttf *.otf *.ttc *.dfont *.pcf *.psf"},
        workingDirFont,
        dpiScale
      );
      break;
    case GUI_FILE_LOAD_PAT_FONT:
      if (!dirExists(workingDirFont)) workingDirFont=getHomeDir();
      hasOpened=fileDialog->openLoad(
        _("Select Font"),
        {_("compatible files"), "*.ttf *.otf *.ttc *.dfont *.pcf *.psf"},
        workingDirFont,
        dpiScale
      );
      break;
    case GUI_FILE_IMPORT_COLORS:
      if (!dirExists(workingDirColors)) workingDirColors=getHomeDir();
      hasOpened=fileDialog->openLoad(
        _("Select Color File"),
        {_("configuration files"), "*.cfgc"},
        workingDirColors,
        dpiScale
      );
      break;
    case GUI_FILE_IMPORT_KEYBINDS:
      if (!dirExists(workingDirKeybinds)) workingDirKeybinds=getHomeDir();
      hasOpened=fileDialog->openLoad(
        _("Select Keybind File"),
        {_("configuration files"), "*.cfgk"},
        workingDirKeybinds,
        dpiScale
      );
      break;
    case GUI_FILE_IMPORT_LAYOUT:
      if (!dirExists(workingDirLayout)) workingDirLayout=getHomeDir();
      hasOpened=fileDialog->openLoad(
        _("Select Layout File"),
        {_(".ini files"), "*.ini"},
        workingDirLayout,
        dpiScale
      );
      break;
    case GUI_FILE_IMPORT_USER_PRESETS:
    case GUI_FILE_IMPORT_USER_PRESETS_REPLACE:
      if (!dirExists(workingDirConfig)) workingDirConfig=getHomeDir();
      hasOpened=fileDialog->openLoad(
        _("Select User Presets File"),
        {_("configuration files"), "*.cfgu"},
        workingDirConfig,
        dpiScale
      );
      break;
    case GUI_FILE_IMPORT_CONFIG:
      if (!dirExists(workingDirConfig)) workingDirConfig=getHomeDir();
      hasOpened=fileDialog->openLoad(
        _("Select Settings File"),
        {_("configuration files"), "*.cfg"},
        workingDirConfig,
        dpiScale
      );
      break;
    case GUI_FILE_EXPORT_COLORS:
      if (!dirExists(workingDirColors)) workingDirColors=getHomeDir();
      hasOpened=fileDialog->openSave(
        _("Export Colors"),
        {_("configuration files"), "*.cfgc"},
        workingDirColors,
        dpiScale
      );
      break;
    case GUI_FILE_EXPORT_KEYBINDS:
      if (!dirExists(workingDirKeybinds)) workingDirKeybinds=getHomeDir();
      hasOpened=fileDialog->openSave(
        _("Export Keybinds"),
        {_("configuration files"), "*.cfgk"},
        workingDirKeybinds,
        dpiScale
      );
      break;
    case GUI_FILE_EXPORT_LAYOUT:
      if (!dirExists(workingDirLayout)) workingDirLayout=getHomeDir();
      hasOpened=fileDialog->openSave(
        _("Export Layout"),
        {_(".ini files"), "*.ini"},
        workingDirLayout,
        dpiScale
      );
      break;
    case GUI_FILE_EXPORT_USER_PRESETS:
      if (!dirExists(workingDirConfig)) workingDirConfig=getHomeDir();
      hasOpened=fileDialog->openSave(
        _("Export User Presets"),
        {_("configuration files"), "*.cfgu"},
        workingDirConfig,
        dpiScale
      );
      break;
    case GUI_FILE_EXPORT_CONFIG:
      if (!dirExists(workingDirConfig)) workingDirConfig=getHomeDir();
      hasOpened=fileDialog->openSave(
        _("Export Settings"),
        {_("configuration files"), "*.cfg"},
        workingDirConfig,
        dpiScale
      );
      break;
    case GUI_FILE_YRW801_ROM_OPEN:
    case GUI_FILE_TG100_ROM_OPEN:
    case GUI_FILE_MU5_ROM_OPEN:
      if (!dirExists(workingDirSample)) workingDirSample=getHomeDir();
      hasOpened=fileDialog->openLoad(
        _("Load ROM"),
        {_("compatible files"), "*.rom *.bin",
         _("all files"), "*"},
        workingDirROM,
        dpiScale
      );
      break;
    case GUI_FILE_CMDSTREAM_OPEN:
      if (!dirExists(workingDirROM)) workingDirROM=getHomeDir();
      hasOpened=fileDialog->openLoad(
        _("Play Command Stream"),
        {_("command stream"), "*.bin",
         _("all files"), "*"},
        workingDirROM,
        dpiScale
      );
      break;
    case GUI_FILE_TEST_OPEN:
      if (!dirExists(workingDirTest)) workingDirTest=getHomeDir();
      hasOpened=fileDialog->openLoad(
        _("Open Test"),
        {_("compatible files"), "*.fur *.dmf *.mod",
         _("another option"), "*.wav *.ttf",
         _("all files"), "*"},
        workingDirTest,
        dpiScale,
        [](const char* path) {
          if (path!=NULL) {
            logI("Callback Result: %s",path);
          } else {
            logI("Callback Result: NULL");
          }
        }
      );
      break;
    case GUI_FILE_TEST_OPEN_MULTI:
      if (!dirExists(workingDirTest)) workingDirTest=getHomeDir();
      hasOpened=fileDialog->openLoad(
        _("Open Test (Multi)"),
        {_("compatible files"), "*.fur *.dmf *.mod",
         _("another option"), "*.wav *.ttf",
         _("all files"), "*"},
        workingDirTest,
        dpiScale,
        [](const char* path) {
          if (path!=NULL) {
            logI("Callback Result: %s",path);
          } else {
            logI("Callback Result: NULL");
          }
        },
        true
      );
      break;
    case GUI_FILE_TEST_SAVE:
      if (!dirExists(workingDirTest)) workingDirTest=getHomeDir();
      hasOpened=fileDialog->openSave(
        _("Save Test"),
        {_("Furnace song"), "*.fur",
         _("DefleMask module"), "*.dmf"},
        workingDirTest,
        dpiScale
      );
      break;
  }
  if (hasOpened) curFileDialog=type;
  //ImGui::GetIO().ConfigFlags|=ImGuiConfigFlags_NavEnableKeyboard;
}

int FurnaceGUI::save(String path, int dmfVersion) {
  SafeWriter* w;
  logD("saving file...");
  if (dmfVersion) {
    if (dmfVersion<24) dmfVersion=24;
    w=e->saveDMF(dmfVersion);
  } else {
    w=e->saveFur(false,settings.newPatternFormat);
  }
  if (w==NULL) {
    lastError=e->getLastError();
    logE("couldn't save! %s",lastError);
    return 3;
  }
  logV("opening file for writing...");
  FILE* outFile=ps_fopen(path.c_str(),"wb");
  if (outFile==NULL) {
    lastError=strerror(errno);
    logE("couldn't save! %s",lastError);
    w->finish();
    return 1;
  }
  if (settings.compress) {
    unsigned char zbuf[131072];
    int ret;
    z_stream zl;
    memset(&zl,0,sizeof(z_stream));
    ret=deflateInit(&zl,Z_DEFAULT_COMPRESSION);
    if (ret!=Z_OK) {
      logE("zlib error!");
      lastError=_("compression error");
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
        logE("zlib stream error!");
        lastError=_("zlib stream error");
        deflateEnd(&zl);
        fclose(outFile);
        w->finish();
        return 2;
      }
      size_t amount=131072-zl.avail_out;
      if (amount>0) {
        if (fwrite(zbuf,1,amount,outFile)!=amount) {
          logE("did not write entirely: %s!",strerror(errno));
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
      logE("zlib finish stream error!");
      lastError=_("zlib finish stream error");
      deflateEnd(&zl);
      fclose(outFile);
      w->finish();
      return 2;
    }
    if (131072-zl.avail_out>0) {
      if (fwrite(zbuf,1,131072-zl.avail_out,outFile)!=(131072-zl.avail_out)) {
        logE("did not write entirely: %s!",strerror(errno));
        lastError=strerror(errno);
        deflateEnd(&zl);
        fclose(outFile);
        w->finish();
        return 1;
      }
    }
    deflateEnd(&zl);
  } else {
    if (fwrite(w->getFinalBuf(),1,w->size(),outFile)!=w->size()) {
      logE("did not write entirely: %s!",strerror(errno));
      lastError=strerror(errno);
      fclose(outFile);
      w->finish();
      return 1;
    }
  }
  fclose(outFile);
  w->finish();
  backupLock.lock();
  curFileName=path;
  backupLock.unlock();
  modified=false;
  updateWindowTitle();
  if (!e->getWarnings().empty()) {
    showWarning(e->getWarnings(),GUI_WARN_GENERIC);
  }
  pushRecentFile(path);
  pushRecentSys(path.c_str());
  logD("save complete.");
  return 0;
}

int FurnaceGUI::load(String path) {
  bool wasPlaying=e->isPlaying();
  if (!path.empty()) {
    logI("loading module...");
    FILE* f=ps_fopen(path.c_str(),"rb");
    if (f==NULL) {
      perror("error");
      lastError=strerror(errno);
      return 1;
    }
    if (fseek(f,0,SEEK_END)<0) {
      perror("size error");
      lastError=fmt::sprintf(_("on seek: %s"),strerror(errno));
      fclose(f);
      return 1;
    }
    ssize_t len=ftell(f);
    if (len==(SIZE_MAX>>1)) {
      perror("could not get file length");
      lastError=fmt::sprintf(_("on pre tell: %s"),strerror(errno));
      fclose(f);
      return 1;
    }
    if (len<1) {
      if (len==0) {
        logE("that file is empty!");
        lastError=_("file is empty");
      } else {
        perror("tell error");
        lastError=fmt::sprintf(_("on tell: %s"),strerror(errno));
      }
      fclose(f);
      return 1;
    }
    if (fseek(f,0,SEEK_SET)<0) {
      perror("size error");
      lastError=fmt::sprintf(_("on get size: %s"),strerror(errno));
      fclose(f);
      return 1;
    }
    unsigned char* file=new unsigned char[len];
    if (fread(file,1,(size_t)len,f)!=(size_t)len) {
      perror("read error");
      lastError=fmt::sprintf(_("on read: %s"),strerror(errno));
      fclose(f);
      delete[] file;
      return 1;
    }
    fclose(f);
    if (!e->load(file,(size_t)len,path.c_str())) {
      lastError=e->getLastError();
      logE("could not open file!");
      return 1;
    }
  }
  backupLock.lock();
  curFileName=path;
  backupLock.unlock();
  modified=false;
  curNibble=false;
  orderNibble=false;
  orderCursor=-1;
  curOrder=0;
  oldRow=0;
  samplePos=0;
  updateSampleTex=true;
  selStart=SelectionPoint();
  selEnd=SelectionPoint();
  cursor=SelectionPoint();
  lastError=_("everything OK");
  undoHist.clear();
  redoHist.clear();
  updateWindowTitle();
  updateROMExportAvail();
  updateScroll(0);
  if (!e->getWarnings().empty()) {
    showWarning(e->getWarnings(),GUI_WARN_GENERIC);
  }
  pushRecentFile(path);
  // walk song
  e->walkSong(loopOrder,loopRow,loopEnd);
  // do not auto-play a backup
  if (path.find(backupPath)!=0) {
    if (settings.playOnLoad==2 || (settings.playOnLoad==1 && wasPlaying)) {
      play();
    }
  } else {
    // warn the user
    showWarning(_("you have loaded a backup!\nif you need to, please save it somewhere.\n\nDO NOT RELY ON THE BACKUP SYSTEM FOR AUTO-SAVE!\nFurnace will not save backups of backups."),GUI_WARN_GENERIC);
  }

  // if this is a PC module import, warn the user on the first import.
  if (!tutorial.importedMOD && e->song.version==DIV_VERSION_MOD) {
    showWarning(_("you have imported a ProTracker/SoundTracker/PC module!\nkeep the following in mind:\n\n- Furnace is not a replacement for your MOD player\n- import is not perfect. your song may sound different:\n  - E6x pattern loop is not supported\n\nhave fun!"),GUI_WARN_IMPORT);
  }
  if (!tutorial.importedS3M && e->song.version==DIV_VERSION_S3M) {
    showWarning(_("you have imported a Scream Tracker 3 module!\nkeep the following in mind:\n\n- Furnace is not a replacement for your S3M player\n- import is not perfect. your song may sound different:\n  - OPL instruments may be detuned\n\nhave fun!"),GUI_WARN_IMPORT);
  }
  if (!tutorial.importedXM && e->song.version==DIV_VERSION_XM) {
    showWarning(_("you have imported a FastTracker II module!\nkeep the following in mind:\n\n- Furnace is not a replacement for your XM player\n- import is not perfect. your song may sound different:\n  - envelopes have been converted to macros\n  - global volume changes are not supported\n\nhave fun!"),GUI_WARN_IMPORT);
  }
  if (!tutorial.importedIT && e->song.version==DIV_VERSION_IT) {
    showWarning(_("you have imported an Impulse Tracker module!\nkeep the following in mind:\n\n- Furnace is not a replacement for your IT player\n- import is not perfect. your song may sound different:\n  - envelopes have been converted to macros\n  - global volume changes are not supported\n  - channel volume changes are not supported\n  - New Note Actions (NNA) are not supported\n\nhave fun!"),GUI_WARN_IMPORT);
  }
  return 0;
}

void FurnaceGUI::openRecentFile(String path) {
  if (modified) {
    nextFile=path;
    showWarning(_("Unsaved changes! Save changes before opening file?"),GUI_WARN_OPEN_DROP);
  } else {
    if (load(path)>0) {
      showError(fmt::sprintf(_("Error while loading file! (%s)"),lastError));
    }
  }
}

void FurnaceGUI::pushRecentFile(String path) {
  if (path.empty()) return;
  if (path.find(backupPath)==0) return;
  for (int i=0; i<(int)recentFile.size(); i++) {
    if (recentFile[i]==path) {
      recentFile.erase(i);
      i--;
    }
  }
  recentFile.push_front(path);

  while (!recentFile.empty() && (int)recentFile.size()>settings.maxRecentFile) {
    recentFile.pop_back();
  }
}

void FurnaceGUI::pushRecentSys(const char* path) {
#ifdef _WIN32
  WString widePath=utf8To16(path);
  SHAddToRecentDocs(SHARD_PATHW,widePath.c_str());
#endif
}

void FurnaceGUI::delFirstBackup(String name) {
  std::vector<String> listOfFiles;
#ifdef _WIN32
  String findPath=backupPath+String(DIR_SEPARATOR_STR)+name+String("*.fur");
  WIN32_FIND_DATAW next;
  HANDLE backDir=FindFirstFileW(utf8To16(findPath.c_str()).c_str(),&next);
  if (backDir!=INVALID_HANDLE_VALUE) {
    do {
      listOfFiles.push_back(utf16To8(next.cFileName));
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
    struct dirent* next=readdir(backDir);
    if (next==NULL) break;
    if (strstr(next->d_name,name.c_str())!=next->d_name) continue;
    listOfFiles.push_back(String(next->d_name));
  }
  closedir(backDir);
#endif

  std::sort(listOfFiles.begin(),listOfFiles.end(),[](const String& a, const String& b) -> bool {
    return strcmp(a.c_str(),b.c_str())<0;
  });

  int totalDelete=((int)listOfFiles.size())-settings.backupMaxCopies;
  for (int i=0; i<totalDelete; i++) {
    String toDelete=backupPath+String(DIR_SEPARATOR_STR)+listOfFiles[i];
    deleteFile(toDelete.c_str());
  }
}

int FurnaceGUI::loadStream(String path) {
  if (!path.empty()) {
    logI("loading stream...");
    FILE* f=ps_fopen(path.c_str(),"rb");
    if (f==NULL) {
      perror("error");
      lastError=strerror(errno);
      return 1;
    }
    if (fseek(f,0,SEEK_END)<0) {
      perror("size error");
      lastError=fmt::sprintf(_("on seek: %s"),strerror(errno));
      fclose(f);
      return 1;
    }
    ssize_t len=ftell(f);
    if (len==(SIZE_MAX>>1)) {
      perror("could not get file length");
      lastError=fmt::sprintf(_("on pre tell: %s"),strerror(errno));
      fclose(f);
      return 1;
    }
    if (len<1) {
      if (len==0) {
        logE("that file is empty!");
        lastError=_("file is empty");
      } else {
        perror("tell error");
        lastError=fmt::sprintf(_("on tell: %s"),strerror(errno));
      }
      fclose(f);
      return 1;
    }
    if (fseek(f,0,SEEK_SET)<0) {
      perror("size error");
      lastError=fmt::sprintf(_("on get size: %s"),strerror(errno));
      fclose(f);
      return 1;
    }
    unsigned char* file=new unsigned char[len];
    if (fread(file,1,(size_t)len,f)!=(size_t)len) {
      perror("read error");
      lastError=fmt::sprintf(_("on read: %s"),strerror(errno));
      fclose(f);
      delete[] file;
      return 1;
    }
    fclose(f);
    if (!e->playStream(file,(size_t)len)) {
      lastError=e->getLastError();
      logE("could not open file!");
      return 1;
    }
  }
  return 0;
}


void FurnaceGUI::exportAudio(String path, DivAudioExportModes mode) {
  songOrdersLengths.clear();

  int loopOrder=0;
  int loopRow=0;
  int loopEnd=0;
  e->walkSong(loopOrder,loopRow,loopEnd);

  e->findSongLength(loopOrder,loopRow,audioExportOptions.fadeOut,songFadeoutSectionLength,songHasSongEndCommand,songOrdersLengths,songLength); // for progress estimation

  songLoopedSectionLength=songLength;
  for (int i=0; i<loopOrder; i++) {
    songLoopedSectionLength-=songOrdersLengths[i];
  }
  songLoopedSectionLength-=loopRow;

  e->saveAudio(path.c_str(),audioExportOptions);

  totalFiles=0;
  e->getTotalAudioFiles(totalFiles);
  int totalLoops=0;

  lengthOfOneFile=songLength;

  if (!songHasSongEndCommand) {
    e->getTotalLoops(totalLoops);

    lengthOfOneFile+=songLoopedSectionLength*totalLoops;
    lengthOfOneFile+=songFadeoutSectionLength; // account for fadeout
  }

  totalLength=lengthOfOneFile*totalFiles;

  curProgress=0.0f;

  displayExporting=true;
}

void FurnaceGUI::exportCmdStream(bool target, String path) {
  csExportPath=path;
  csExportTarget=target;
  csExportDone=false;
  csExportThread=new std::thread([this]() {
    SafeWriter* w=e->saveCommand(&csProgress,csExportOptions);
    csExportResult=w;
    csExportDone=true;
  });
  displayExportingCS=true;
}

void FurnaceGUI::editStr(String* which) {
  editString=which;
  displayEditString=true;
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

String FurnaceGUI::getLastError() {
  return lastError;
}

// what monster did I just create here?
#define B30(tt) (macroDragBit30?((((tt)&0xc0000000)==0x40000000 || ((tt)&0xc0000000)==0x80000000)?0x40000000:0):0)

#define MACRO_DRAG(t) \
  if (macroDragSettingBit30) { \
    if (macroDragLastX!=x || macroDragLastY!=y) { \
      macroDragLastX=x; \
      macroDragLastY=y; \
      if (macroDragInitialValueSet) { \
        if (!macroDragInitialValue) { \
          if (t[x]&0x80000000) { \
            t[x]&=~0x40000000; \
          } else { \
            t[x]|=0x40000000; \
          } \
        } else { \
          if (t[x]&0x80000000) { \
            t[x]|=0x40000000; \
          } else { \
            t[x]&=~0x40000000; \
          } \
        } \
      } else { \
        macroDragInitialValue=(((t[x])&0xc0000000)==0x40000000 || ((t[x])&0xc0000000)==0x80000000); \
        macroDragInitialValueSet=true; \
        t[x]^=0x40000000; \
      } \
    } \
  } else if (macroDragBitMode) { \
    if (macroDragLastX!=x || macroDragLastY!=y) { \
      macroDragLastX=x; \
      macroDragLastY=y; \
      if (macroDragInitialValueSet) { \
        if (macroDragInitialValue) { \
          t[x]=(((t[x])&((1<<macroDragMax)-1))&(~(1<<y))); \
        } else { \
          t[x]=(((t[x])&((1<<macroDragMax)-1))|(1<<y)); \
        } \
      } else { \
        macroDragInitialValue=(((t[x])&((1<<macroDragMax)-1))&(1<<y)); \
        macroDragInitialValueSet=true; \
        t[x]=(((t[x])&((1<<macroDragMax)-1))^(1<<y)); \
      } \
      t[x]&=(1<<macroDragMax)-1; \
    } \
  } else { \
    if (macroDragLineMode) { \
      if (!macroDragInitialValueSet) { \
        macroDragLineInitial=ImVec2(x,y); \
        macroDragLineInitialV=ImVec2(dragX,dragY); \
        macroDragInitialValueSet=true; \
        macroDragMouseMoved=false; \
      } else if (!macroDragMouseMoved) { \
        if ((pow(dragX-macroDragLineInitialV.x,2.0)+pow(dragY-macroDragLineInitialV.y,2.0))>=16.0f) { \
          macroDragMouseMoved=true; \
        } \
      } \
      if (macroDragMouseMoved) { \
        if ((int)round(x-macroDragLineInitial.x)==0) { \
          t[x]=B30(t[x])^(int)(macroDragLineInitial.y); \
        } else { \
          if ((int)round(x-macroDragLineInitial.x)<0) { \
            for (int i=0; i<=(int)round(macroDragLineInitial.x-x); i++) { \
              int index=(int)round(x+i); \
              if (index<0) continue; \
              t[index]=B30(t[index])^(int)(y+(macroDragLineInitial.y-y)*((float)i/(float)(macroDragLineInitial.x-x))); \
            } \
          } else { \
            for (int i=0; i<=(int)round(x-macroDragLineInitial.x); i++) { \
              int index=(int)round(i+macroDragLineInitial.x); \
              if (index<0) continue; \
              t[index]=B30(t[index])^(int)(macroDragLineInitial.y+(y-macroDragLineInitial.y)*((float)i/(x-macroDragLineInitial.x))); \
            } \
          } \
        } \
      } \
    } else { \
      t[x]=B30(t[x])^(y); \
    } \
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
      if (x>=macroLoopDragLen) {
        x=-1;
      } else {
        x+=macroDragScroll;
      }
      *macroLoopDragTarget=x;
    }
  }
  if (waveDragActive) {
    if (waveDragLen>0) {
      int x=(dragX-waveDragStart.x)*waveDragLen/MAX(1,waveDragAreaSize.x);
      if (x<0) x=0;
      if (x>=waveDragLen) x=waveDragLen-1;
      int y=(waveDragMax+1)-((dragY-waveDragStart.y)*(double((waveDragMax+1)-waveDragMin)/(double)MAX(1,waveDragAreaSize.y)));
      if (y>waveDragMax) y=waveDragMax;
      if (y<waveDragMin) y=waveDragMin;
      waveDragTarget[x]=y;
      notifyWaveChange=true;
      MARK_MODIFIED;
    }
  }
  if (sampleDragActive) {
    int x=samplePos+floor(double(dragX-sampleDragStart.x)*sampleZoom);
    int x1=samplePos+floor(double(dragX-sampleDragStart.x+1)*sampleZoom);
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
      if (sampleDragTarget) {
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
      }
    } else { // select
      if (sampleSelStart<0) {
        sampleSelStart=x;
      }
      sampleSelEnd=x;
    }
  }
  if (orderScrollLocked) {
    if (fabs(orderScrollRealOrigin.x-dragX)>2.0f*dpiScale || fabs(orderScrollRealOrigin.y-dragY)>2.0f*dpiScale) orderScrollTolerance=false;
    orderScroll=(orderScrollSlideOrigin-dragX)/(40.0*dpiScale);
    if (orderScroll<0.0f) orderScroll=0.0f;
    if (orderScroll>(float)e->curSubSong->ordersLen-1) orderScroll=e->curSubSong->ordersLen-1;
  }
}

#define checkExtension(x) \
  String lowerCase=fileName; \
  for (char& i: lowerCase) { \
    if (i>='A' && i<='Z') i+='a'-'A'; \
  } \
  if (lowerCase.size()<strlen(x) || lowerCase.rfind(x)!=lowerCase.size()-strlen(x)) { \
    fileName+=x; \
  }

#define drawOpMask(m) \
  ImGui::PushFont(patFont); \
  ImGui::PushID("om_" #m); \
  if (ImGui::BeginTable("opMaskTable",5,ImGuiTableFlags_Borders|ImGuiTableFlags_SizingFixedFit|ImGuiTableFlags_NoHostExtendX)) { \
    ImGui::TableNextRow(); \
    ImGui::TableNextColumn(); \
    ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_ACTIVE]); \
    if (ImGui::Selectable(m.note?"C-4##opMaskNote":"---##opMaskNote",m.note,ImGuiSelectableFlags_DontClosePopups)) { \
      m.note=!m.note; \
    } \
    ImGui::PopStyleColor(); \
    ImGui::TableNextColumn(); \
    ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_INS]); \
    if (ImGui::Selectable(m.ins?"01##opMaskIns":"--##opMaskIns",m.ins,ImGuiSelectableFlags_DontClosePopups)) { \
      m.ins=!m.ins; \
    } \
    ImGui::PopStyleColor(); \
    ImGui::TableNextColumn(); \
    ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_VOLUME_MAX]); \
    if (ImGui::Selectable(m.vol?"7F##opMaskVol":"--##opMaskVol",m.vol,ImGuiSelectableFlags_DontClosePopups)) { \
      m.vol=!m.vol; \
    } \
    ImGui::PopStyleColor(); \
    ImGui::TableNextColumn(); \
    ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_EFFECT_PITCH]); \
    if (ImGui::Selectable(m.effect?"04##opMaskEffect":"--##opMaskEffect",m.effect,ImGuiSelectableFlags_DontClosePopups)) { \
      m.effect=!m.effect; \
    } \
    ImGui::TableNextColumn(); \
    if (ImGui::Selectable(m.effectVal?"72##opMaskEffectVal":"--##opMaskEffectVal",m.effectVal,ImGuiSelectableFlags_DontClosePopups)) { \
      m.effectVal=!m.effectVal; \
    } \
    ImGui::PopStyleColor(); \
    ImGui::EndTable(); \
  } \
  ImGui::PopID(); \
  ImGui::PopFont();

void FurnaceGUI::editOptions(bool topMenu) {
  char id[4096];
  editOptsVisible=true;

  if (ImGui::MenuItem(_("cut"),BIND_FOR(GUI_ACTION_PAT_CUT))) doCopy(true,true,selStart,selEnd);
  if (ImGui::MenuItem(_("copy"),BIND_FOR(GUI_ACTION_PAT_COPY))) doCopy(false,true,selStart,selEnd);
  if (ImGui::MenuItem(_("paste"),BIND_FOR(GUI_ACTION_PAT_PASTE))) doPaste();
  if (ImGui::BeginMenu(_("paste special..."))) {
    if (ImGui::MenuItem(_("paste mix"),BIND_FOR(GUI_ACTION_PAT_PASTE_MIX))) doPaste(GUI_PASTE_MODE_MIX_FG);
    if (ImGui::MenuItem(_("paste mix (background)"),BIND_FOR(GUI_ACTION_PAT_PASTE_MIX_BG))) doPaste(GUI_PASTE_MODE_MIX_BG);
    if (ImGui::BeginMenu(_("paste with ins (foreground)"))) {
      if (e->song.ins.empty()) {
        ImGui::Text(_("no instruments available"));
      }
      for (size_t i=0; i<e->song.ins.size(); i++) {
        snprintf(id,4095,"%.2X: %s",(int)i,e->song.ins[i]->name.c_str());
        if (ImGui::MenuItem(id)) {
          doPaste(GUI_PASTE_MODE_INS_FG,i);
        }
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu(_("paste with ins (background)"))) {
      if (e->song.ins.empty()) {
        ImGui::Text(_("no instruments available"));
      }
      for (size_t i=0; i<e->song.ins.size(); i++) {
        snprintf(id,4095,"%.2X: %s",(int)i,e->song.ins[i]->name.c_str());
        if (ImGui::MenuItem(id)) {
          doPaste(GUI_PASTE_MODE_INS_BG,i);
        }
      }
      ImGui::EndMenu();
    }
    if (ImGui::MenuItem(_("paste flood"),BIND_FOR(GUI_ACTION_PAT_PASTE_FLOOD))) doPaste(GUI_PASTE_MODE_FLOOD);
    if (ImGui::MenuItem(_("paste overflow"),BIND_FOR(GUI_ACTION_PAT_PASTE_OVERFLOW))) doPaste(GUI_PASTE_MODE_OVERFLOW);
    ImGui::EndMenu();
  }
  if (ImGui::MenuItem(_("delete"),BIND_FOR(GUI_ACTION_PAT_DELETE))) doDelete();
  if (topMenu) {
    if (ImGui::MenuItem(_("select all"),BIND_FOR(GUI_ACTION_PAT_SELECT_ALL))) doSelectAll();
  }
  ImGui::Separator();

  if (ImGui::BeginMenu(_("operation mask..."))) {
    drawOpMask(opMaskDelete);
    ImGui::SameLine();
    ImGui::Text(_("delete"));

    drawOpMask(opMaskPullDelete);
    ImGui::SameLine();
    ImGui::Text(_("pull delete"));

    drawOpMask(opMaskInsert);
    ImGui::SameLine();
    ImGui::Text(_("insert"));

    drawOpMask(opMaskPaste);
    ImGui::SameLine();
    ImGui::Text(_("paste"));

    drawOpMask(opMaskTransposeNote);
    ImGui::SameLine();
    ImGui::Text(_("transpose (note)"));

    drawOpMask(opMaskTransposeValue);
    ImGui::SameLine();
    ImGui::Text(_("transpose (value)"));

    drawOpMask(opMaskInterpolate);
    ImGui::SameLine();
    ImGui::Text(_("interpolate"));

    drawOpMask(opMaskFade);
    ImGui::SameLine();
    ImGui::Text(_("fade"));

    drawOpMask(opMaskInvertVal);
    ImGui::SameLine();
    ImGui::Text(_("invert values"));

    drawOpMask(opMaskScale);
    ImGui::SameLine();
    ImGui::Text(_("scale"));

    drawOpMask(opMaskRandomize);
    ImGui::SameLine();
    ImGui::Text(_("randomize"));

    drawOpMask(opMaskFlip);
    ImGui::SameLine();
    ImGui::Text(_("flip"));

    drawOpMask(opMaskCollapseExpand);
    ImGui::SameLine();
    ImGui::Text(_("collapse/expand"));

    ImGui::EndMenu();
  }

  ImGui::Text(_("input latch"));
  ImGui::PushFont(patFont);
  if (ImGui::BeginTable("inputLatchTable",5,ImGuiTableFlags_Borders|ImGuiTableFlags_SizingFixedFit|ImGuiTableFlags_NoHostExtendX)) {
    static char id[64];
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_ACTIVE]);
    ImGui::Text("C-4");
    ImGui::PopStyleColor();
    ImGui::TableNextColumn();
    ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_INS]);
    if (latchIns==-2) {
      strcpy(id,"&&##LatchIns");
    } else if (latchIns==-1) {
      strcpy(id,"..##LatchIns");
    } else {
      snprintf(id,63,"%.2X##LatchIns",latchIns&0xff);
    }
    if (ImGui::Selectable(id,latchTarget==1,ImGuiSelectableFlags_DontClosePopups)) {
      latchTarget=1;
      latchNibble=false;
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
      latchIns=-2;
    }
    if (ImGui::IsItemHovered()) {
      ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_TEXT]);
      ImGui::SetTooltip(_("&&: selected instrument\n..: no instrument"));
      ImGui::PopStyleColor();
    }
    ImGui::PopStyleColor();
    ImGui::TableNextColumn();
    ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_VOLUME_MAX]);
    if (latchVol==-1) {
      strcpy(id,"..##LatchVol");
    } else {
      snprintf(id,63,"%.2X##LatchVol",latchVol&0xff);
    }
    if (ImGui::Selectable(id,latchTarget==2,ImGuiSelectableFlags_DontClosePopups)) {
      latchTarget=2;
      latchNibble=false;
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
      latchVol=-1;
    }
    ImGui::PopStyleColor();
    ImGui::TableNextColumn();
    if (latchEffect==-1) {
      strcpy(id,"..##LatchFX");
      ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_INACTIVE]);
    } else {
      const unsigned char data=latchEffect;
      snprintf(id,63,"%.2X##LatchFX",data);
      ImGui::PushStyleColor(ImGuiCol_Text,uiColors[fxColors[data]]);
    }

    if (ImGui::Selectable(id,latchTarget==3,ImGuiSelectableFlags_DontClosePopups)) {
      latchTarget=3;
      latchNibble=false;
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
      latchEffect=-1;
    }
    ImGui::TableNextColumn();
    if (latchEffectVal==-1) {
      strcpy(id,"..##LatchFXV");
    } else {
      snprintf(id,63,"%.2X##LatchFXV",latchEffectVal&0xff);
    }
    if (ImGui::Selectable(id,latchTarget==4,ImGuiSelectableFlags_DontClosePopups)) {
      latchTarget=4;
      latchNibble=false;
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
      latchEffectVal=-1;
    }
    ImGui::PopStyleColor();
    ImGui::EndTable();
  }
  ImGui::PopFont();
  ImGui::SameLine();
  if (ImGui::Button(_("Set"))) {
    DivPattern* pat=e->curPat[cursor.xCoarse].getPattern(e->curOrders->ord[cursor.xCoarse][curOrder],true);
    latchIns=pat->data[cursor.y][2];
    latchVol=pat->data[cursor.y][3];
    latchEffect=pat->data[cursor.y][4];
    latchEffectVal=pat->data[cursor.y][5];
    latchTarget=0;
    latchNibble=false;
  }
  ImGui::SameLine();
  if (ImGui::Button(_("Reset"))) {
    latchIns=-2;
    latchVol=-1;
    latchEffect=-1;
    latchEffectVal=-1;
    latchTarget=0;
    latchNibble=false;
  }
  ImGui::Separator();

  if (ImGui::MenuItem(_("note up"),BIND_FOR(GUI_ACTION_PAT_NOTE_UP))) doTranspose(1,opMaskTransposeNote);
  if (ImGui::MenuItem(_("note down"),BIND_FOR(GUI_ACTION_PAT_NOTE_DOWN))) doTranspose(-1,opMaskTransposeNote);
  if (ImGui::MenuItem(_("octave up"),BIND_FOR(GUI_ACTION_PAT_OCTAVE_UP))) doTranspose(12,opMaskTransposeNote);
  if (ImGui::MenuItem(_("octave down"),BIND_FOR(GUI_ACTION_PAT_OCTAVE_DOWN)))  doTranspose(-12,opMaskTransposeNote);
  ImGui::Separator();
  if (ImGui::MenuItem(_("values up"),BIND_FOR(GUI_ACTION_PAT_VALUE_UP))) doTranspose(1,opMaskTransposeValue);
  if (ImGui::MenuItem(_("values down"),BIND_FOR(GUI_ACTION_PAT_VALUE_DOWN))) doTranspose(-1,opMaskTransposeValue);
  if (ImGui::MenuItem(_("values up (+16)"),BIND_FOR(GUI_ACTION_PAT_VALUE_UP_COARSE))) doTranspose(16,opMaskTransposeValue);
  if (ImGui::MenuItem(_("values down (-16)"),BIND_FOR(GUI_ACTION_PAT_VALUE_DOWN_COARSE)))  doTranspose(-16,opMaskTransposeValue);
  ImGui::Separator();
  ImGui::AlignTextToFramePadding();
  ImGui::Text(_("transpose"));
  ImGui::SameLine();
  ImGui::SetNextItemWidth(120.0f*dpiScale);
  if (ImGui::InputInt("##TransposeAmount",&transposeAmount,1,12)) {
    if (transposeAmount<-96) transposeAmount=-96;
    if (transposeAmount>96) transposeAmount=96;
  }
  ImGui::SameLine();
  if (ImGui::Button(_("Notes"))) {
    doTranspose(transposeAmount,opMaskTransposeNote);
    ImGui::CloseCurrentPopup();
  }
  ImGui::SameLine();
  if (ImGui::Button(_("Values"))) {
    doTranspose(transposeAmount,opMaskTransposeValue);
    ImGui::CloseCurrentPopup();
  }

  ImGui::Separator();
  if (ImGui::MenuItem(_("interpolate"),BIND_FOR(GUI_ACTION_PAT_INTERPOLATE))) doInterpolate();
  if (ImGui::BeginMenu(_("change instrument..."))) {
    if (e->song.ins.empty()) {
      ImGui::Text(_("no instruments available"));
    }
    for (size_t i=0; i<e->song.ins.size(); i++) {
      snprintf(id,4095,"%.2X: %s",(int)i,e->song.ins[i]->name.c_str());
      if (ImGui::MenuItem(id)) {
        doChangeIns(i);
      }
    }
    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu(_("gradient/fade..."))) {
    if (ImGui::InputInt(_("Start"),&fadeMin,1,16)) {
      if (fadeMin<0) fadeMin=0;
      if (fadeMode) {
        if (fadeMin>15) fadeMin=15;
      } else {
        if (fadeMin>255) fadeMin=255;
      }
    }
    if (ImGui::InputInt(_("End"),&fadeMax,1,16)) {
      if (fadeMax<0) fadeMax=0;
      if (fadeMode) {
        if (fadeMax>15) fadeMax=15;
      } else {
        if (fadeMax>255) fadeMax=255;
      }
    }
    if (ImGui::Checkbox(_("Nibble mode"),&fadeMode)) {
      if (fadeMode) {
        if (fadeMin>15) fadeMin=15;
        if (fadeMax>15) fadeMax=15;
      } else {
        if (fadeMin>255) fadeMin=255;
        if (fadeMax>255) fadeMax=255;
      }
    }
    if (ImGui::Button(_("Go ahead"))) {
      doFade(fadeMin,fadeMax,fadeMode);
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndMenu();
  }
  if (ImGui::BeginMenu(_("scale..."))) {
    if (ImGui::InputFloat("##ScaleMax",&scaleMax,1,10,"%.1f%%")) {
      if (scaleMax<0.0f) scaleMax=0.0f;
      if (scaleMax>25600.0f) scaleMax=25600.0f;
    }
    if (ImGui::Button(_("Scale"))) {
      doScale(scaleMax);
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndMenu();
  }
  if (ImGui::BeginMenu(_("randomize..."))) {
    if (ImGui::InputInt(_("Minimum"),&randomizeMin,1,16)) {
      if (randomizeMin<0) randomizeMin=0;
      if (randomMode) {
        if (randomizeMin>15) randomizeMin=15;
      } else {
        if (randomizeMin>255) randomizeMin=255;
      }
      if (randomizeMin>randomizeMax) randomizeMin=randomizeMax;
    }
    if (ImGui::InputInt(_("Maximum"),&randomizeMax,1,16)) {
      if (randomizeMax<0) randomizeMax=0;
      if (randomizeMax<randomizeMin) randomizeMax=randomizeMin;
      if (randomMode) {
        if (randomizeMax>15) randomizeMax=15;
      } else {
        if (randomizeMax>255) randomizeMax=255;
      }
    }
    if (ImGui::Checkbox(_("Nibble mode"),&randomMode)) {
      if (randomMode) {
        if (randomizeMin>15) randomizeMin=15;
        if (randomizeMax>15) randomizeMax=15;
      } else {
        if (randomizeMin>255) randomizeMin=255;
        if (randomizeMax>255) randomizeMax=255;
      }
    }
    if (selStart.xFine>2 || selEnd.xFine>2 || selStart.xCoarse!=selEnd.xCoarse) {
      ImGui::Checkbox(_("Set effect"),&randomizeEffect);
      if (randomizeEffect) {
        if (ImGui::InputScalar(_("Effect"),ImGuiDataType_S32,&randomizeEffectVal,&_ONE,&_SIXTEEN,"%.2X",ImGuiInputTextFlags_CharsHexadecimal)) {
          if (randomizeEffectVal<0) randomizeEffectVal=0;
          if (randomizeEffectVal>255) randomizeEffectVal=255;
        }
      }
    }
    if (ImGui::Button(_("Randomize"))) {
      doRandomize(randomizeMin,randomizeMax,randomMode,randomizeEffect,randomizeEffectVal);
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndMenu();
  }
  if (ImGui::MenuItem(_("invert values"),BIND_FOR(GUI_ACTION_PAT_INVERT_VALUES))) doInvertValues();

  ImGui::Separator();

  if (ImGui::MenuItem(_("flip selection"),BIND_FOR(GUI_ACTION_PAT_FLIP_SELECTION))) doFlip();

  ImGui::SetNextItemWidth(120.0f*dpiScale);
  if (ImGui::InputInt(_("collapse/expand amount##CollapseAmount"),&collapseAmount,1,4)) {
    if (collapseAmount<2) collapseAmount=2;
    if (collapseAmount>256) collapseAmount=256;
  }
  if (ImGui::MenuItem(_("collapse"),BIND_FOR(GUI_ACTION_PAT_COLLAPSE_ROWS))) doCollapse(collapseAmount,selStart,selEnd);
  if (ImGui::MenuItem(_("expand"),BIND_FOR(GUI_ACTION_PAT_EXPAND_ROWS))) doExpand(collapseAmount,selStart,selEnd);

  if (topMenu) {
    ImGui::Separator();
    if (ImGui::MenuItem(_("collapse pattern"),BIND_FOR(GUI_ACTION_PAT_COLLAPSE_PAT))) doAction(GUI_ACTION_PAT_COLLAPSE_PAT);
    if (ImGui::MenuItem(_("expand pattern"),BIND_FOR(GUI_ACTION_PAT_EXPAND_PAT))) doAction(GUI_ACTION_PAT_EXPAND_PAT);
  }

  if (topMenu) {
    ImGui::Separator();
    if (ImGui::MenuItem(_("collapse song"),BIND_FOR(GUI_ACTION_PAT_COLLAPSE_SONG))) doAction(GUI_ACTION_PAT_COLLAPSE_SONG);
    if (ImGui::MenuItem(_("expand song"),BIND_FOR(GUI_ACTION_PAT_EXPAND_SONG))) doAction(GUI_ACTION_PAT_EXPAND_SONG);
  }

  if (topMenu) {
    ImGui::Separator();
    if (ImGui::MenuItem(_("find/replace"),BIND_FOR(GUI_ACTION_WINDOW_FIND),findOpen)) {
      if (findOpen) {
        findOpen=false;
      } else {
        nextWindow=GUI_WINDOW_FIND;
      }
    }
  }
}

void FurnaceGUI::toggleMobileUI(bool enable, bool force) {
  if (mobileUI!=enable || force) {
    if (!mobileUI && enable) {
      if (!ImGui::SaveIniSettingsToDisk(finalLayoutPath,true)) {
        reportError(fmt::sprintf(_("could NOT save layout! %s"),strerror(errno)));
      }
    }
    mobileUI=enable;
    if (mobileUI) {
      ImGui::GetIO().IniFilename=NULL;
      ImGui::GetIO().ConfigFlags|=ImGuiConfigFlags_InertialScrollEnable;
      ImGui::GetIO().ConfigFlags|=ImGuiConfigFlags_NoHoverColors;
      ImGui::GetIO().AlwaysScrollText=true;
      fileDialog->mobileUI=true;
    } else {
      ImGui::GetIO().IniFilename=NULL;
      if (!ImGui::LoadIniSettingsFromDisk(finalLayoutPath,true)) {
        reportError(fmt::sprintf(_("could NOT load layout! %s"),strerror(errno)));
        ImGui::LoadIniSettingsFromMemory(defaultLayout);
      }
      ImGui::GetIO().ConfigFlags&=~ImGuiConfigFlags_InertialScrollEnable;
      ImGui::GetIO().ConfigFlags&=~ImGuiConfigFlags_NoHoverColors;
      ImGui::GetIO().AlwaysScrollText=false;
      fileDialog->mobileUI=false;
    }
  }
}

void FurnaceGUI::pushToggleColors(bool status) {
  ImVec4 toggleColor=status?uiColors[GUI_COLOR_TOGGLE_ON]:uiColors[GUI_COLOR_TOGGLE_OFF];
  ImGui::PushStyleColor(ImGuiCol_Button,toggleColor);
  if (!mobileUI) {
    if (settings.guiColorsBase) {
      toggleColor.x*=0.8f;
      toggleColor.y*=0.8f;
      toggleColor.z*=0.8f;
    } else {
      toggleColor.x=CLAMP(toggleColor.x*1.3f,0.0f,1.0f);
      toggleColor.y=CLAMP(toggleColor.y*1.3f,0.0f,1.0f);
      toggleColor.z=CLAMP(toggleColor.z*1.3f,0.0f,1.0f);
    }
  }
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered,toggleColor);
  if (settings.guiColorsBase) {
    toggleColor.x*=0.8f;
    toggleColor.y*=0.8f;
    toggleColor.z*=0.8f;
  } else {
    toggleColor.x=CLAMP(toggleColor.x*1.5f,0.0f,1.0f);
    toggleColor.y=CLAMP(toggleColor.y*1.5f,0.0f,1.0f);
    toggleColor.z=CLAMP(toggleColor.z*1.5f,0.0f,1.0f);
  }
  ImGui::PushStyleColor(ImGuiCol_ButtonActive,toggleColor);
}

void FurnaceGUI::popToggleColors() {
  ImGui::PopStyleColor(3);
}

int _processEvent(void* instance, SDL_Event* event) {
  return ((FurnaceGUI*)instance)->processEvent(event);
}

#if SDL_VERSION_ATLEAST(2,0,17)
#define VALID_MODS KMOD_NUM|KMOD_CAPS|KMOD_SCROLL
#else
#define VALID_MODS KMOD_NUM|KMOD_CAPS
#endif

int FurnaceGUI::processEvent(SDL_Event* ev) {
  if (introPos<11.0 && !shortIntro) return 1;
#ifdef IS_MOBILE
  if (ev->type==SDL_APP_TERMINATING) {
    // TODO: save last song state here
  } else if (ev->type==SDL_APP_WILLENTERBACKGROUND) {
    commitState(e->getConfObject());
    e->saveConf();
  }
#endif
  if (cvOpen) return 1;
  if (ev->type==SDL_KEYDOWN) {
    if (!ev->key.repeat && latchTarget==0 && !wantCaptureKeyboard && !sampleMapWaitingInput && (ev->key.keysym.mod&(~(VALID_MODS)))==0) {
      if (settings.notePreviewBehavior==0) return 1;
      switch (curWindow) {
        case GUI_WINDOW_SAMPLE_EDIT:
        case GUI_WINDOW_SAMPLE_LIST: {
          auto it=noteKeys.find(ev->key.keysym.scancode);
          if (it!=noteKeys.cend()) {
            int key=it->second;
            int num=12*curOctave+key;
            if (key!=100 && key!=101 && key!=102) {
              int pStart=-1;
              int pEnd=-1;
              if (curWindow==GUI_WINDOW_SAMPLE_EDIT) {
                if (sampleSelStart!=sampleSelEnd) {
                  pStart=sampleSelStart;
                  pEnd=sampleSelEnd;
                  if (pStart>pEnd) {
                    pStart^=pEnd;
                    pEnd^=pStart;
                    pStart^=pEnd;
                  }
                }
              }
              e->previewSample(curSample,num,pStart,pEnd);
              samplePreviewOn=true;
              samplePreviewKey=ev->key.keysym.scancode;
              samplePreviewNote=num;
            }
          }
          break;
        }
        case GUI_WINDOW_WAVE_LIST:
        case GUI_WINDOW_WAVE_EDIT: {
          auto it=noteKeys.find(ev->key.keysym.scancode);
          if (it!=noteKeys.cend()) {
            int key=it->second;
            int num=12*curOctave+key;
            if (key!=100 && key!=101 && key!=102) {
              e->previewWave(curWave,num);
              wavePreviewOn=true;
              wavePreviewKey=ev->key.keysym.scancode;
              wavePreviewNote=num;
            }
          }
          break;
        }
        case GUI_WINDOW_ORDERS: // ignore here
          break;
        case GUI_WINDOW_PATTERN:
          if (settings.notePreviewBehavior==1) {
            if (cursor.xFine!=0) break;
          } else if (settings.notePreviewBehavior==2) {
            if (edit && cursor.xFine!=0) break;
          }
          // fall-through
        default: {
          auto it=noteKeys.find(ev->key.keysym.scancode);
          if (it!=noteKeys.cend()) {
            int key=it->second;
            int num=12*curOctave+key;

            if (num<-60) num=-60; // C-(-5)
            if (num>119) num=119; // B-9

            if (key!=100 && key!=101 && key!=102) {
              previewNote(cursor.xCoarse,num);
            }
          }
          break;
        }
      }
    }
  } else if (ev->type==SDL_KEYUP) {
    stopPreviewNote(ev->key.keysym.scancode,true);
    if (wavePreviewOn) {
      if (ev->key.keysym.scancode==wavePreviewKey) {
        wavePreviewOn=false;
        e->stopWavePreview();
      }
    }
    if (samplePreviewOn) {
      if (ev->key.keysym.scancode==samplePreviewKey) {
        samplePreviewOn=false;
        e->stopSamplePreview();
      }
    }
  }
  return 1;
}

#define FIND_POINT(p,pid) \
  for (TouchPoint& i: activePoints) { \
    if (i.id==pid) { \
      p=&i; \
    } \
  }

void FurnaceGUI::processPoint(SDL_Event& ev) {
  switch (ev.type) {
    case SDL_MOUSEMOTION: {
      TouchPoint* point=NULL;
      FIND_POINT(point,-1);
      if (point!=NULL) {
        point->x=(double)ev.motion.x*((double)canvasW/(double)scrW);
        point->y=(double)ev.motion.y*((double)canvasH/(double)scrH);
      }
      break;
    }
    case SDL_MOUSEBUTTONDOWN: {
      if (ev.button.button!=SDL_BUTTON_LEFT) break;
      for (size_t i=0; i<activePoints.size(); i++) {
        TouchPoint& point=activePoints[i];
        if (point.id==-1) {
          releasedPoints.push_back(point);
          activePoints.erase(activePoints.begin()+i);
          break;
        }
      }
      TouchPoint newPoint(ev.button.x,ev.button.y);
#ifdef __APPLE__
      newPoint.x*=dpiScale;
      newPoint.y*=dpiScale;
#endif
      activePoints.push_back(newPoint);
      pressedPoints.push_back(newPoint);
      break;
    }
    case SDL_MOUSEBUTTONUP: {
      if (ev.button.button!=SDL_BUTTON_LEFT) break;
      for (size_t i=0; i<activePoints.size(); i++) {
        TouchPoint& point=activePoints[i];
        if (point.id==-1) {
          releasedPoints.push_back(point);
          activePoints.erase(activePoints.begin()+i);
          break;
        }
      }
      break;
    }
    case SDL_FINGERMOTION: {
      TouchPoint* point=NULL;
      FIND_POINT(point,ev.tfinger.fingerId);
      if (point!=NULL) {
        float prevX=point->x;
        float prevY=point->y;
        point->x=ev.tfinger.x*canvasW;
        point->y=ev.tfinger.y*canvasH;
        point->z=ev.tfinger.pressure;

        if (point->id==0) {
          ImGui::GetIO().AddMousePosEvent(point->x,point->y);
          pointMotion(point->x,point->y,point->x-prevX,point->y-prevY);
        }
      }
      break;
    }
    case SDL_FINGERDOWN: {
      for (size_t i=0; i<activePoints.size(); i++) {
        TouchPoint& point=activePoints[i];
        if (point.id==ev.tfinger.fingerId) {
          releasedPoints.push_back(point);
          activePoints.erase(activePoints.begin()+i);
          break;
        }
      }
      TouchPoint newPoint(ev.tfinger.fingerId,ev.tfinger.x*canvasW,ev.tfinger.y*canvasH,ev.tfinger.pressure);
      activePoints.push_back(newPoint);
      pressedPoints.push_back(newPoint);

      if (newPoint.id==0) {
        ImGui::GetIO().AddMousePosEvent(newPoint.x,newPoint.y);
        ImGui::GetIO().AddMouseButtonEvent(ImGuiMouseButton_Left,true);
        pointDown(newPoint.x,newPoint.y,0);
      }
      break;
    }
    case SDL_FINGERUP: {
      for (size_t i=0; i<activePoints.size(); i++) {
        TouchPoint& point=activePoints[i];
        if (point.id==ev.tfinger.fingerId) {
          if (point.id==0) {
            ImGui::GetIO().AddMouseButtonEvent(ImGuiMouseButton_Left,false);
            //ImGui::GetIO().AddMousePosEvent(-FLT_MAX,-FLT_MAX);
            pointUp(point.x,point.y,0);
          }

          releasedPoints.push_back(point);
          activePoints.erase(activePoints.begin()+i);

          break;
        }
      }
      break;
    }
  }
}

void FurnaceGUI::pointDown(int x, int y, int button) {
  aboutOpen=false;
  if (bindSetActive) {
    bindSetActive=false;
    bindSetPending=false;
    actionKeys[bindSetTarget][bindSetTargetIdx]=bindSetPrevValue;
    if (bindSetTargetIdx==(int)actionKeys[bindSetTarget].size()-1 && bindSetPrevValue<=0) {
      actionKeys[bindSetTarget].pop_back();
    }
    bindSetTarget=0;
    bindSetTargetIdx=0;
    bindSetPrevValue=0;
  }
  if (introPos<11.0 && !shortIntro) {
    introSkipDo=true;
  }
}

void FurnaceGUI::pointUp(int x, int y, int button) {
  if (macroDragActive || macroLoopDragActive || waveDragActive || (sampleDragActive && sampleDragMode && sampleDragTarget)) {
    MARK_MODIFIED;
  }
  if (macroDragActive && macroDragLineMode && !macroDragMouseMoved) {
    displayMacroMenu=true;
  }
  macroDragActive=false;
  macroDragBitMode=false;
  macroDragInitialValue=false;
  macroDragInitialValueSet=false;
  macroDragLastX=-1;
  macroDragLastY=-1;
  macroLoopDragActive=false;
  waveDragActive=false;
  if (introPos<11.0 && introSkip<0.5 && !shortIntro) {
    introSkipDo=false;
  }
  if (sampleDragActive) {
    logD("stopping sample drag");
    if (sampleDragMode) {
      e->renderSamplesP(curSample);
    } else {
      if (sampleSelStart>sampleSelEnd) {
        sampleSelStart^=sampleSelEnd;
        sampleSelEnd^=sampleSelStart;
        sampleSelStart^=sampleSelEnd;
      }
    }
  }
  sampleDragActive=false;
  if (orderScrollLocked) {
    int targetOrder=round(orderScroll);
    if (orderScrollTolerance) {
      targetOrder=round(orderScroll+(orderScrollRealOrigin.x-((float)canvasW/2.0f))/(40.0f*dpiScale));
    }
    if (targetOrder<0) targetOrder=0;
    if (targetOrder>e->curSubSong->ordersLen-1) targetOrder=e->curSubSong->ordersLen-1;
    if (curOrder!=targetOrder) setOrder(targetOrder);
  }
  orderScrollLocked=false;
  orderScrollTolerance=false;
  if (dragMobileMenu) {
    dragMobileMenu=false;
    if (mobileMenuOpen) {
      mobileMenuOpen=(mobileMenuPos>=0.85f);
    } else {
      mobileMenuOpen=(mobileMenuPos>=0.15f);
    }
  }
  if (dragMobileEditButton) {
    dragMobileEditButton=false;
  }
}

void FurnaceGUI::pointMotion(int x, int y, int xrel, int yrel) {
  if (selecting && (!mobileUI || mobilePatSel)) {
    // detect whether we have to scroll
    if (y<patWindowPos.y+2.0f*dpiScale) {
      addScroll(-1);
    }
    if (y>patWindowPos.y+patWindowSize.y-2.0f*dpiScale) {
      addScroll(1);
    }
    if (x<patWindowPos.x+(mobileUI?40.0f:4.0f)*dpiScale) {
      addScrollX(-1);
    }
    if (x>patWindowPos.x+patWindowSize.x-(mobileUI?40.0f:4.0f)*dpiScale) {
      addScrollX(1);
    }
  }
  if (macroDragActive || macroLoopDragActive || waveDragActive || sampleDragActive || orderScrollLocked) {
    int distance=fabs((double)xrel);
    if (distance<1) distance=1;
    float start=x-xrel;
    float end=x;
    float startY=y-yrel;
    float endY=y;
    for (int i=0; i<=distance; i++) {
      float fraction=(float)i/(float)distance;
      float x=start+(end-start)*fraction;
      float y=startY+(endY-startY)*fraction;
      processDrags(x,y);
    }
  }
}

// how many pixels should be visible at least at x/y dir
#define OOB_PIXELS_SAFETY 25

bool FurnaceGUI::detectOutOfBoundsWindow(SDL_Rect& failing) {
  int count=SDL_GetNumVideoDisplays();
  if (count<1) {
    logW("bounds check: error: %s",SDL_GetError());
    return false;
  }

  SDL_Rect rect;
  for (int i=0; i<count; i++) {
    if (SDL_GetDisplayUsableBounds(i,&rect)!=0) {
      logW("bounds check: error in display %d: %s",i,SDL_GetError());
      continue;
    }

    bool xbound=((rect.x+OOB_PIXELS_SAFETY)<=(scrX+scrW)) && ((rect.x+rect.w-OOB_PIXELS_SAFETY)>=scrX);
    bool ybound=((rect.y+OOB_PIXELS_SAFETY)<=(scrY+scrH)) && ((rect.y+rect.h-OOB_PIXELS_SAFETY)>=scrY);
    logD("bounds check: display %d is at %dx%dx%dx%d: %s%s",i,rect.x+OOB_PIXELS_SAFETY,rect.y+OOB_PIXELS_SAFETY,rect.x+rect.w-OOB_PIXELS_SAFETY,rect.y+rect.h-OOB_PIXELS_SAFETY,xbound?"x":"",ybound?"y":"");

    if (xbound && ybound) {
      return true;
    }
  }

  failing=rect;
  return false;
}

#define DECLARE_METRIC(_n) \
  uint64_t __perfM##_n;

#define MEASURE_BEGIN(_n) \
  __perfM##_n=SDL_GetPerformanceCounter();

#define MEASURE_END(_n) \
  if (perfMetricsLen<64) { \
    perfMetrics[perfMetricsLen++]=FurnaceGUIPerfMetric(#_n,SDL_GetPerformanceCounter()-__perfM##_n); \
  }

#define MEASURE(_n,_x) \
  MEASURE_BEGIN(_n) \
  _x; \
  MEASURE_END(_n)

#define IMPORT_CLOSE(x) \
  if (x) pendingLayoutImportReopen.push(&x); \
  x=false;

bool FurnaceGUI::loop() {
  DECLARE_METRIC(calcChanOsc)
  DECLARE_METRIC(mobileControls)
  DECLARE_METRIC(mobileOrderSel)
  DECLARE_METRIC(subSongs)
  DECLARE_METRIC(findReplace)
  DECLARE_METRIC(spoiler)
  DECLARE_METRIC(pattern)
  DECLARE_METRIC(editControls)
  DECLARE_METRIC(speed)
  DECLARE_METRIC(grooves)
  DECLARE_METRIC(songInfo)
  DECLARE_METRIC(orders)
  DECLARE_METRIC(intro)
  DECLARE_METRIC(sampleList)
  DECLARE_METRIC(sampleEdit)
  DECLARE_METRIC(waveList)
  DECLARE_METRIC(waveEdit)
  DECLARE_METRIC(insList)
  DECLARE_METRIC(insEdit)
  DECLARE_METRIC(mixer)
  DECLARE_METRIC(readOsc)
  DECLARE_METRIC(osc)
  DECLARE_METRIC(chanOsc)
  DECLARE_METRIC(xyOsc)
  DECLARE_METRIC(volMeter)
  DECLARE_METRIC(settings)
  DECLARE_METRIC(debug)
  DECLARE_METRIC(csPlayer)
  DECLARE_METRIC(stats)
  DECLARE_METRIC(memory)
  DECLARE_METRIC(compatFlags)
  DECLARE_METRIC(piano)
  DECLARE_METRIC(notes)
  DECLARE_METRIC(channels)
  DECLARE_METRIC(patManager)
  DECLARE_METRIC(sysManager)
  DECLARE_METRIC(clock)
  DECLARE_METRIC(regView)
  DECLARE_METRIC(log)
  DECLARE_METRIC(effectList)
  DECLARE_METRIC(userPresets)
  DECLARE_METRIC(popup)

#ifdef IS_MOBILE
  bool doThreadedInput=true;
#else
  bool doThreadedInput=!settings.noThreadedInput;
#endif
  if (doThreadedInput) {
    logD("key input: event filter");
    SDL_SetEventFilter(_processEvent,this);
  } else {
    logD("key input: main thread");
  }

  if (safeMode) {
    showError(_("Furnace has been started in Safe Mode.\nthis means that:\n\n- software rendering is being used\n- audio output may not work\n- font loading is disabled\n\ncheck any settings which may have made Furnace start up in this mode.\nfont loading is one of these."));
    settingsOpen=true;
  }

  while (!quit) {
    SDL_Event ev;
    if (e->isPlaying()) {
      WAKE_UP;
    }
    if (--drawHalt<=0) {
      drawHalt=0;
      if (settings.powerSave) SDL_WaitEventTimeout(NULL,500);
    }

    memcpy(perfMetricsLast,perfMetrics,64*sizeof(FurnaceGUIPerfMetric));
    perfMetricsLastLen=perfMetricsLen;
    perfMetricsLen=0;

    eventTimeBegin=SDL_GetPerformanceCounter();
    bool updateWindow=false;
    if (injectBackUp) {
      ImGui::GetIO().AddKeyEvent(ImGuiKey_Backspace,false);
      injectBackUp=false;
    }

    while (SDL_PollEvent(&ev)) {
      WAKE_UP;
      ImGui_ImplSDL2_ProcessEvent(&ev);
      processPoint(ev);
      if (!doThreadedInput) processEvent(&ev);
      switch (ev.type) {
        case SDL_MOUSEMOTION: {
          int motionX=(double)ev.motion.x*((double)canvasW/(double)scrW);
          int motionY=(double)ev.motion.y*((double)canvasH/(double)scrH);
          int motionXrel=(double)ev.motion.xrel*((double)canvasW/(double)scrW);
          int motionYrel=(double)ev.motion.yrel*((double)canvasH/(double)scrH);
          pointMotion(motionX,motionY,motionXrel,motionYrel);
          break;
        }
        case SDL_MOUSEBUTTONUP:
          pointUp(ev.button.x,ev.button.y,ev.button.button);
          insEditMayBeDirty=true;
          break;
        case SDL_MOUSEBUTTONDOWN:
          pointDown(ev.button.x,ev.button.y,ev.button.button);
          insEditMayBeDirty=true;
          break;
        case SDL_MOUSEWHEEL:
          wheelX+=ev.wheel.x;
          wheelY+=ev.wheel.y;
          insEditMayBeDirty=true;
          break;
        case SDL_WINDOWEVENT:
          switch (ev.window.event) {
            case SDL_WINDOWEVENT_RESIZED:
              scrW=ev.window.data1;
              scrH=ev.window.data2;
              portrait=(scrW<scrH);
              logV("portrait: %d (%dx%d)",portrait,scrW,scrH);
              logD("window resized to %dx%d",scrW,scrH);
              updateWindow=true;
              rend->resized(ev);
              break;
            case SDL_WINDOWEVENT_MOVED:
              scrX=ev.window.data1;
              scrY=ev.window.data2;
              updateWindow=true;
              shallDetectScale=2;
              logV("window moved to %dx%d",scrX,scrY);
              break;
            case SDL_WINDOWEVENT_SIZE_CHANGED:
              logV("window size changed to %dx%d",ev.window.data1,ev.window.data2);
              break;
            case SDL_WINDOWEVENT_MINIMIZED:
              logV("window minimized");
              break;
            case SDL_WINDOWEVENT_MAXIMIZED:
              scrMax=true;
              updateWindow=true;
              logV("window maximized");
              break;
            case SDL_WINDOWEVENT_RESTORED:
              scrMax=false;
              updateWindow=true;
              logV("window restored");
              break;
            case SDL_WINDOWEVENT_SHOWN:
              logV("window shown");
              break;
            case SDL_WINDOWEVENT_HIDDEN:
              logV("window hidden");
              break;
            case SDL_WINDOWEVENT_EXPOSED:
              logV("window exposed");
              break;
          }
          break;
#if SDL_VERSION_ATLEAST(2,0,4)
        case SDL_RENDER_DEVICE_RESET:
          killGraphics=true;
          break;
#endif
#if SDL_VERSION_ATLEAST(2,0,17)
        case SDL_DISPLAYEVENT: {
          switch (ev.display.event) {
            case SDL_DISPLAYEVENT_CONNECTED:
              logD("display %d connected!",ev.display.display);
              updateWindow=true;
              shallDetectScale=16;
              break;
            case SDL_DISPLAYEVENT_DISCONNECTED:
              logD("display %d disconnected!",ev.display.display);
              updateWindow=true;
              shallDetectScale=16;
              break;
            case SDL_DISPLAYEVENT_ORIENTATION:
              logD("display oriented to %d",ev.display.data1);
              updateWindow=true;
              break;
          }
          break;
        }
#endif
        case SDL_KEYDOWN:
          if (!ImGui::GetIO().WantCaptureKeyboard || (ImGuiFileDialog::Instance()->IsOpened() && !ImGui::GetIO().WantTextInput)) {
            keyDown(ev);
          }
          if (introPos<11.0 && !shortIntro) {
            if (ev.key.keysym.scancode==SDL_SCANCODE_SPACE || ev.key.keysym.scancode==SDL_SCANCODE_ESCAPE || ev.key.keysym.scancode==SDL_SCANCODE_RETURN) {
              introSkip=0.5;
            }
            introSkipDo=true;
          }
          insEditMayBeDirty=true;
#ifdef IS_MOBILE
          injectBackUp=true;
#endif
          break;
        case SDL_KEYUP:
          // for now
          insEditMayBeDirty=true;
          if (introPos<11.0 && introSkip<0.5 && !shortIntro) {
            introSkipDo=false;
          }
          break;
        case SDL_DROPFILE:
          if (ev.drop.file!=NULL) {
            if (introPos<11.0) {
              SDL_free(ev.drop.file);
              break;
            }
            int sampleCountBefore=e->song.sampleLen;
            std::vector<DivInstrument*> instruments=e->instrumentFromFile(ev.drop.file,true,settings.readInsNames);
            std::vector<DivSample*> samples = e->sampleFromFile(ev.drop.file);
            DivWavetable* droppedWave=NULL;
            //DivSample* droppedSample=NULL;
            if (!instruments.empty()) {
              if (e->song.sampleLen!=sampleCountBefore) {
                e->renderSamplesP();
              }
              if (!e->getWarnings().empty()) {
                showWarning(e->getWarnings(),GUI_WARN_GENERIC);
              }
              int instrumentCount=-1;
              for (DivInstrument* i: instruments) {
                instrumentCount=e->addInstrumentPtr(i);
              }
              if (instrumentCount>=0 && settings.selectAssetOnLoad) {
                curIns=instrumentCount-1;
              }
              nextWindow=GUI_WINDOW_INS_LIST;
              MARK_MODIFIED;
            } else if ((droppedWave=e->waveFromFile(ev.drop.file,false))!=NULL) {
              int waveCount=-1;
              waveCount=e->addWavePtr(droppedWave);
              if (waveCount>=0 && settings.selectAssetOnLoad) {
                curWave=waveCount-1;
              }
              nextWindow=GUI_WINDOW_WAVE_LIST;
              MARK_MODIFIED;
            } 
            else if (!samples.empty()) 
            {
              if (e->song.sampleLen!=sampleCountBefore) {
                //e->renderSamplesP();
              }
              if (!e->getWarnings().empty())
              {
                showWarning(e->getWarnings(),GUI_WARN_GENERIC);
              }
              int sampleCount=-1;
              for (DivSample* s: samples)
              {
                sampleCount=e->addSamplePtr(s);
              }
              //sampleCount=e->addSamplePtr(droppedSample);
              if (sampleCount>=0 && settings.selectAssetOnLoad) 
              {
                curSample=sampleCount;
                updateSampleTex=true;
              }
              nextWindow=GUI_WINDOW_SAMPLE_LIST;
              MARK_MODIFIED;
            } else if (modified) {
              nextFile=ev.drop.file;
              showWarning(_("Unsaved changes! Save changes before opening file?"),GUI_WARN_OPEN_DROP);
            } else {
              if (load(ev.drop.file)>0) {
                showError(fmt::sprintf(_("Error while loading file! (%s)"),lastError));
              }
            }
            SDL_free(ev.drop.file);
          }
          break;
        case SDL_USEREVENT:
          // used for MIDI wake up
          break;
        case SDL_QUIT:
          if (requestQuit()) {
            return true;
          }
          break;
      }
    }

    // update config x/y/w/h values based on scrMax state
    if (updateWindow) {
      logV("updateWindow is true");
      if (!scrMax && !fullScreen) {
        logV("updating scrConf");
        scrConfX=scrX;
        scrConfY=scrY;
        scrConfW=scrW;
        scrConfH=scrH;
      }
      if (rend!=NULL) {
        logV("restoring swap interval...");
        rend->setSwapInterval(settings.vsync);
      }
    }
    // update canvas size as well
    if (!rend->getOutputSize(canvasW,canvasH)) {
      logW("loop: error while getting output size!");
    } else {
      //logV("updateWindow: canvas size %dx%d",canvasW,canvasH);
      // and therefore window size
      int prevScrW=scrW;
      int prevScrH=scrH;
      SDL_GetWindowSize(sdlWin,&scrW,&scrH);
      if (prevScrW!=scrW || prevScrH!=scrH) {
        logV("size change 2: %dx%d (from %dx%d)",scrW,scrH,prevScrW,prevScrH);
      }

      ImGui::GetIO().InputScale=(float)canvasW/(float)scrW;
    }

    wantCaptureKeyboard=ImGui::GetIO().WantTextInput;

    if (wantCaptureKeyboard!=oldWantCaptureKeyboard) {
      oldWantCaptureKeyboard=wantCaptureKeyboard;
      if (wantCaptureKeyboard) {
        SDL_StartTextInput();
      } else {
        SDL_StopTextInput();
      }
    }

    if (wantCaptureKeyboard) {
      WAKE_UP;
    }

    if (ImGui::GetIO().IsSomethingHappening) {
      WAKE_UP;
    }

    if (ImGui::GetIO().MouseDown[0] || ImGui::GetIO().MouseDown[1] || ImGui::GetIO().MouseDown[2] || ImGui::GetIO().MouseDown[3] || ImGui::GetIO().MouseDown[4]) {
      WAKE_UP;
    }

    while (true) {
      midiLock.lock();
      midiWakeUp=true;
      if (midiQueue.empty()) {
        midiLock.unlock();
        break;
      }
      TAMidiMessage msg=midiQueue.front();
      midiLock.unlock();

      if (msg.type==TA_MIDI_SYSEX) {
        unsigned char* data=msg.sysExData.get();
        for (size_t i=0; i<msg.sysExLen; i++) {
          if ((i&15)==0) printf("\n");
          printf("%.2x ",data[i]);
        }
        printf("\n");

        if (!parseSysEx(data,msg.sysExLen)) {
          logW("error while parsing SysEx data!");
        }
      }

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
                  midiMap.volInput?msg.data[1]:-1
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
            if (midiMap.programChange && !(midiMap.directChannel && midiMap.directProgram)) {
              curIns=msg.data[0];
              if (curIns>=(int)e->song.ins.size()) curIns=e->song.ins.size()-1;
              wavePreviewInit=true;
              updateFMPreview=true;
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

    if (notifyWaveChange) {
      notifyWaveChange=false;
      e->notifyWaveChange(curWave);
    }

    eventTimeEnd=SDL_GetPerformanceCounter();

    if (SDL_GetWindowFlags(sdlWin)&SDL_WINDOW_MINIMIZED) {
      SDL_Delay(30);
      drawHalt=0;
      continue;
    }

    if (firstFrame && !safeMode && renderBackend!=GUI_BACKEND_SOFTWARE) {
      if (!tutorial.introPlayed || settings.alwaysPlayIntro==3 || (settings.alwaysPlayIntro==2 && curFileName.empty())) {
        unsigned char* introTemp=new unsigned char[intro_fur_len];
        memcpy(introTemp,intro_fur,intro_fur_len);
        e->load(introTemp,intro_fur_len);
      }
    }

    if (!e->isRunning()) {
      activeNotes.clear();
      memset(chanOscVol,0,DIV_MAX_CHANS*sizeof(float));
      for (int i=0; i<DIV_MAX_CHANS; i++) {
        chanOscChan[i].pitch=0.0f;
      }
      memset(chanOscBright,0,DIV_MAX_CHANS*sizeof(float));

      e->synchronized([this]() {
        for (int i=0; i<e->getTotalChannelCount(); i++) {
          DivDispatchOscBuffer* buf=e->getOscBuffer(i);
          if (buf!=NULL) {
            //buf->needle=0;
            //buf->readNeedle=0;
            // TODO: should we reset here?
          }
        }
      });
    }

    // recover from dead graphics
    if (rend->isDead() || killGraphics) {
      killGraphics=false;

      logW("graphics are dead! restarting...");
      
      if (sampleTex!=NULL) {
        rend->destroyTexture(sampleTex);
        sampleTex=NULL;
      }

      if (chanOscGradTex!=NULL) {
        rend->destroyTexture(chanOscGradTex);
        chanOscGradTex=NULL;
      }

      for (auto& i: images) {
        if (i.second->tex!=NULL) {
          rend->destroyTexture(i.second->tex);
          i.second->tex=NULL;
        }
      }

      commitState(e->getConfObject());
      rend->quitGUI();
      rend->quit();
      ImGui_ImplSDL2_Shutdown();

      int initAttempts=0;

      SDL_Delay(500);

      logD("starting render backend...");
      while (++initAttempts<=5) {
        if (rend->init(sdlWin,settings.vsync)) {
          break;
        }
        SDL_Delay(1000);
        logV("trying again...");
      }

      if (initAttempts>5) {
        reportError(_("can't keep going without graphics! Furnace will quit now."));
        quit=true;
        break;
      }

      rend->clear(ImVec4(0.0,0.0,0.0,1.0));
      rend->present();

      logD("preparing user interface...");
      rend->initGUI(sdlWin);

      logD("building font...");
      if (rend->areTexturesSquare()) {
        ImGui::GetIO().Fonts->Flags|=ImFontAtlasFlags_Square;
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
        }
      }

      firstFrame=true;
      mustClear=2;
      initialScreenWipe=1.0f;

      continue;
    }

    bool fontsFailed=false;

    layoutTimeBegin=SDL_GetPerformanceCounter();

    if (pendingLayoutImport!=NULL) {
      if (pendingLayoutImportStep==0) {
        IMPORT_CLOSE(editControlsOpen);
        IMPORT_CLOSE(ordersOpen);
        IMPORT_CLOSE(insListOpen);
        IMPORT_CLOSE(songInfoOpen);
        IMPORT_CLOSE(patternOpen);
        IMPORT_CLOSE(insEditOpen);
        IMPORT_CLOSE(waveListOpen);
        IMPORT_CLOSE(waveEditOpen);
        IMPORT_CLOSE(sampleListOpen);
        IMPORT_CLOSE(sampleEditOpen);
        IMPORT_CLOSE(aboutOpen);
        IMPORT_CLOSE(settingsOpen);
        IMPORT_CLOSE(mixerOpen);
        IMPORT_CLOSE(debugOpen);
        IMPORT_CLOSE(inspectorOpen);
        IMPORT_CLOSE(oscOpen);
        IMPORT_CLOSE(volMeterOpen);
        IMPORT_CLOSE(statsOpen);
        IMPORT_CLOSE(compatFlagsOpen);
        IMPORT_CLOSE(pianoOpen);
        IMPORT_CLOSE(notesOpen);
        IMPORT_CLOSE(channelsOpen);
        IMPORT_CLOSE(regViewOpen);
        IMPORT_CLOSE(logOpen);
        IMPORT_CLOSE(effectListOpen);
        IMPORT_CLOSE(chanOscOpen);
        IMPORT_CLOSE(subSongsOpen);
        IMPORT_CLOSE(findOpen);
        IMPORT_CLOSE(spoilerOpen);
        IMPORT_CLOSE(patManagerOpen);
        IMPORT_CLOSE(sysManagerOpen);
        IMPORT_CLOSE(clockOpen);
        IMPORT_CLOSE(speedOpen);
        IMPORT_CLOSE(groovesOpen);
        IMPORT_CLOSE(xyOscOpen);
        IMPORT_CLOSE(memoryOpen);
        IMPORT_CLOSE(csPlayerOpen);
        IMPORT_CLOSE(userPresetsOpen);
      } else if (pendingLayoutImportStep==1) {
        // let the UI settle
      } else if (pendingLayoutImportStep==2) {
        ImGui::LoadIniSettingsFromMemory((const char*)pendingLayoutImport,pendingLayoutImportLen);
      } else if (pendingLayoutImportStep==3) {
        // restore open windows
        while (!pendingLayoutImportReopen.empty()) {
          bool* next=pendingLayoutImportReopen.front();
          *next=true;
          pendingLayoutImportReopen.pop();
        }
      } else if (pendingLayoutImportStep==4) {
        delete[] pendingLayoutImport;
        pendingLayoutImport=NULL;
      }
      pendingLayoutImportStep++;
      if (pendingLayoutImport==NULL) pendingLayoutImportStep=0;
    }

    if (!rend->newFrame()) {
      fontsFailed=true;
    }
    ImGui_ImplSDL2_NewFrame(sdlWin);
    ImGui::NewFrame();

    // one second counter
    secondTimer+=ImGui::GetIO().DeltaTime;
    if (secondTimer>=1.0f) secondTimer=fmod(secondTimer,1.0f);

    curWindowLast=curWindow;
    curWindow=GUI_WINDOW_NOTHING;
    editOptsVisible=false;

    int nextPlayOrder=0;
    int nextOldRow=0;
    e->getPlayPos(nextPlayOrder,nextOldRow);
    oldRowChanged=false;
    playOrder=nextPlayOrder;
    if (followPattern) {
      curOrder=playOrder;
    }
    if (e->isPlaying()) {
      if (oldRow!=nextOldRow) oldRowChanged=true;
      oldRow=nextOldRow;
    }

    // check whether pattern of channel(s) at cursor/selection is/are unique
    isPatUnique=true;
    if (curOrder>=0 && curOrder<e->curSubSong->ordersLen && selStart.xCoarse>=0 && selStart.xCoarse<e->getTotalChannelCount() && selEnd.xCoarse>=0 && selEnd.xCoarse<e->getTotalChannelCount()) {
      for (int i=0; i<e->curSubSong->ordersLen; i++) {
        if (i==curOrder) continue;
        for (int j=selStart.xCoarse; j<=selEnd.xCoarse; j++) {
          if (e->curSubSong->orders.ord[j][i]==e->curSubSong->orders.ord[j][curOrder]) isPatUnique=false;
          break;
        }
        if (!isPatUnique) break;
      }
    }

    if (!mobileUI) {
      ImGui::BeginMainMenuBar();
      if (ImGui::BeginMenu(settings.capitalMenuBar?_("File"):_("file"))) {
        if (ImGui::MenuItem(_("new..."),BIND_FOR(GUI_ACTION_NEW))) {
          if (modified) {
            showWarning(_("Unsaved changes! Save changes before creating a new song?"),GUI_WARN_NEW);
          } else {
            displayNew=true;
          }
        }
        if (ImGui::MenuItem(_("open..."),BIND_FOR(GUI_ACTION_OPEN))) {
          if (modified) {
            showWarning(_("Unsaved changes! Save changes before opening another file?"),GUI_WARN_OPEN);
          } else {
            openFileDialog(GUI_FILE_OPEN);
          }
        }
        if (ImGui::BeginMenu(_("open recent"))) {
          exitDisabledTimer=1;
          for (int i=0; i<(int)recentFile.size(); i++) {
            String item=recentFile[i];
            if (ImGui::MenuItem(item.c_str())) {
              if (modified) {
                nextFile=item;
                showWarning(_("Unsaved changes! Save changes before opening file?"),GUI_WARN_OPEN_DROP);
              } else {
                recentFile.erase(i);
                i--;
              }
              openRecentFile(item);
            }
          }
          if (recentFile.empty()) {
            ImGui::Text(_("nothing here yet"));
          } else {
            ImGui::Separator();
            if (ImGui::MenuItem(_("clear history"))) {
              showWarning(_("Are you sure you want to clear the recent file list?"),GUI_WARN_CLEAR_HISTORY);
            }
          }
          ImGui::EndMenu();
        }
        ImGui::Separator();
        if (ImGui::MenuItem(_("save"),BIND_FOR(GUI_ACTION_SAVE))) {
          if (curFileName=="" || (curFileName.find(backupPath)==0) || e->song.version>=0xff00) {
            openFileDialog(GUI_FILE_SAVE);
          } else {
            if (save(curFileName,e->song.isDMF?e->song.version:0)>0) {
              showError(fmt::sprintf(_("Error while saving file! (%s)"),lastError));
            }
          }
        }
        if (ImGui::MenuItem(_("save as..."),BIND_FOR(GUI_ACTION_SAVE_AS))) {
          openFileDialog(GUI_FILE_SAVE);
        }
        ImGui::Separator();
        if (settings.exportOptionsLayout==0) {
          if (ImGui::BeginMenu(_("export audio..."))) {
            drawExportAudio();
            ImGui::EndMenu();
          }
          if (ImGui::BeginMenu(_("export VGM..."))) {
            drawExportVGM();
            ImGui::EndMenu();
          }
          if (romExportExists) {
            if (ImGui::BeginMenu(_("export ROM..."))) {
              drawExportROM();
              ImGui::EndMenu();
            }
          }
          if (ImGui::BeginMenu(_("export text..."))) {
            drawExportText();
            ImGui::EndMenu();
          }
          if (ImGui::BeginMenu(_("export command stream..."))) {
            drawExportCommand();
            ImGui::EndMenu();
          }
          if (ImGui::BeginMenu(_("export .dmf..."))) {
            drawExportDMF();
            ImGui::EndMenu();
          }
        } else if (settings.exportOptionsLayout==2) {
          if (ImGui::MenuItem(_("export audio..."))) {
            curExportType=GUI_EXPORT_AUDIO;
            displayExport=true;
          }
          if (ImGui::MenuItem(_("export VGM..."))) {
            curExportType=GUI_EXPORT_VGM;
            displayExport=true;
          }
          if (romExportExists) {
            if (ImGui::MenuItem(_("export ROM..."))) {
              curExportType=GUI_EXPORT_ROM;
              displayExport=true;
            }
          }
          if (ImGui::MenuItem(_("export text..."))) {
            curExportType=GUI_EXPORT_TEXT;
            displayExport=true;
          }
          if (ImGui::MenuItem(_("export command stream..."))) {
            curExportType=GUI_EXPORT_CMD_STREAM;
            displayExport=true;
          }
          if (ImGui::MenuItem(_("export .dmf..."))) {
            curExportType=GUI_EXPORT_DMF;
            displayExport=true;
          }
        } else {
          if (ImGui::MenuItem(_("export..."),BIND_FOR(GUI_ACTION_EXPORT))) {
            displayExport=true;
          }
        }
        ImGui::Separator();
        if (!settings.classicChipOptions) {
          if (ImGui::MenuItem(_("manage chips"))) {
            nextWindow=GUI_WINDOW_SYS_MANAGER;
          }
        } else {
          if (ImGui::BeginMenu(_("add chip..."))) {
            exitDisabledTimer=1;
            DivSystem picked=systemPicker(false);
            if (picked!=DIV_SYSTEM_NULL) {
              if (!e->addSystem(picked)) {
                showError(fmt::sprintf(_("cannot add chip! (%s)"),e->getLastError()));
              } else {
                MARK_MODIFIED;
              }
              ImGui::CloseCurrentPopup();
              if (e->song.autoSystem) {
                autoDetectSystem();
              }
              updateWindowTitle();
              updateROMExportAvail();
            }
            ImGui::EndMenu();
          }
          if (ImGui::BeginMenu(_("configure chip..."))) {
            exitDisabledTimer=1;
            for (int i=0; i<e->song.systemLen; i++) {
              if (ImGui::TreeNode(fmt::sprintf("%d. %s##_SYSP%d",i+1,getSystemName(e->song.system[i]),i).c_str())) {
                drawSysConf(i,i,e->song.system[i],e->song.systemFlags[i],true,true);
                ImGui::TreePop();
              }
            }
            ImGui::EndMenu();
          }
          if (ImGui::BeginMenu(_("change chip..."))) {
            exitDisabledTimer=1;
            ImGui::Checkbox(_("Preserve channel positions"),&preserveChanPos);
            for (int i=0; i<e->song.systemLen; i++) {
              if (ImGui::BeginMenu(fmt::sprintf("%d. %s##_SYSC%d",i+1,getSystemName(e->song.system[i]),i).c_str())) {
                DivSystem picked=systemPicker(false);
                if (picked!=DIV_SYSTEM_NULL) {
                  if (e->changeSystem(i,picked,preserveChanPos)) {
                    MARK_MODIFIED;
                    if (e->song.autoSystem) {
                      autoDetectSystem();
                    }
                    updateWindowTitle();
                    updateROMExportAvail();
                  } else {
                    showError(fmt::sprintf(_("cannot change chip! (%s)"),e->getLastError()));
                  }
                  ImGui::CloseCurrentPopup();
                }
                ImGui::EndMenu();
              }
            }
            ImGui::EndMenu();
          }
          if (ImGui::BeginMenu(_("remove chip..."))) {
            exitDisabledTimer=1;
            ImGui::Checkbox(_("Preserve channel positions"),&preserveChanPos);
            for (int i=0; i<e->song.systemLen; i++) {
              if (ImGui::MenuItem(fmt::sprintf("%d. %s##_SYSR%d",i+1,getSystemName(e->song.system[i]),i).c_str())) {
                if (!e->removeSystem(i,preserveChanPos)) {
                  showError(fmt::sprintf(_("cannot remove chip! (%s)"),e->getLastError()));
                } else {
                  MARK_MODIFIED;
                }
                if (e->song.autoSystem) {
                  autoDetectSystem();
                  updateWindowTitle();
                }
                updateROMExportAvail();
              }
            }
            ImGui::EndMenu();
          }
        }
#if defined(FURNACE_DATADIR) && defined(SHOW_OPEN_ASSETS_MENU_ENTRY)
        if (ImGui::MenuItem(_("open built-in assets directory"))) {
          SDL_OpenURL("file://" FURNACE_DATADIR);
        }
#endif
        ImGui::BeginDisabled(exitDisabledTimer);
        ImGui::Separator();
        if (ImGui::MenuItem(_("restore backup"),BIND_FOR(GUI_ACTION_OPEN_BACKUP))) {
          doAction(GUI_ACTION_OPEN_BACKUP);
        }
        ImGui::Separator();
        if (ImGui::MenuItem(_("exit..."),BIND_FOR(GUI_ACTION_QUIT))) {
          requestQuit();
        }
        ImGui::EndDisabled();
        ImGui::EndMenu();
      } else {
        exitDisabledTimer=0;
      }
      if (ImGui::BeginMenu(settings.capitalMenuBar?_("Edit"):_("edit"))) {
        ImGui::Text("...");
        ImGui::Separator();
        if (ImGui::MenuItem(_("undo"),BIND_FOR(GUI_ACTION_UNDO))) doUndo();
        if (ImGui::MenuItem(_("redo"),BIND_FOR(GUI_ACTION_REDO))) doRedo();
        ImGui::Separator();
        editOptions(true);
        ImGui::Separator();
        if (ImGui::MenuItem(_("clear..."))) {
          doAction(GUI_ACTION_CLEAR);
        }
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu(settings.capitalMenuBar?_("Settings"):_("settings"))) {
#ifndef IS_MOBILE
        if (ImGui::MenuItem(_("full screen"),BIND_FOR(GUI_ACTION_FULLSCREEN),fullScreen)) {
          doAction(GUI_ACTION_FULLSCREEN);
        }
#endif
        if (ImGui::MenuItem(_("lock layout"),NULL,lockLayout)) {
          lockLayout=!lockLayout;
        }
        if (ImGui::MenuItem(_("pattern visualizer"),NULL,fancyPattern)) {
          fancyPattern=!fancyPattern;
          e->enableCommandStream(fancyPattern);
          e->getCommandStream(cmdStream);
          cmdStream.clear();
        }
        if (ImGui::MenuItem(_("reset layout"))) {
          showWarning(_("Are you sure you want to reset the workspace layout?"),GUI_WARN_RESET_LAYOUT);
        }
#ifdef IS_MOBILE
        if (ImGui::MenuItem(_("switch to mobile view"))) {
          toggleMobileUI(!mobileUI);
        }
#endif
        if (ImGui::MenuItem(_("user systems..."),BIND_FOR(GUI_ACTION_WINDOW_USER_PRESETS))) {
          userPresetsOpen=true;
        }
        if (ImGui::MenuItem(_("settings..."),BIND_FOR(GUI_ACTION_WINDOW_SETTINGS))) {
          syncSettings();
          settingsOpen=true;
        }
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu(settings.capitalMenuBar?_("Window"):_("window"))) {
        if (ImGui::BeginMenu(_("song"))) {
          if (ImGui::MenuItem(_("song comments"), BIND_FOR(GUI_ACTION_WINDOW_NOTES), notesOpen)) notesOpen = !notesOpen;
          if (ImGui::MenuItem(_("song information"), BIND_FOR(GUI_ACTION_WINDOW_SONG_INFO), songInfoOpen)) songInfoOpen = !songInfoOpen;
          if (ImGui::MenuItem(_("subsongs"), BIND_FOR(GUI_ACTION_WINDOW_SUBSONGS), subSongsOpen)) subSongsOpen = !subSongsOpen;
          ImGui::Separator();
          if (ImGui::MenuItem(_("channels"),BIND_FOR(GUI_ACTION_WINDOW_CHANNELS),channelsOpen)) channelsOpen=!channelsOpen;
          if (ImGui::MenuItem(_("chip manager"),BIND_FOR(GUI_ACTION_WINDOW_SYS_MANAGER),sysManagerOpen)) sysManagerOpen=!sysManagerOpen;
          if (ImGui::MenuItem(_("orders"),BIND_FOR(GUI_ACTION_WINDOW_ORDERS),ordersOpen)) ordersOpen=!ordersOpen;
          if (ImGui::MenuItem(_("pattern"),BIND_FOR(GUI_ACTION_WINDOW_PATTERN),patternOpen)) patternOpen=!patternOpen;
          if (ImGui::MenuItem(_("pattern manager"),BIND_FOR(GUI_ACTION_WINDOW_PAT_MANAGER),patManagerOpen)) patManagerOpen=!patManagerOpen;
          if (ImGui::MenuItem(_("mixer"),BIND_FOR(GUI_ACTION_WINDOW_MIXER),mixerOpen)) mixerOpen=!mixerOpen;
          if (ImGui::MenuItem(_("compatibility flags"),BIND_FOR(GUI_ACTION_WINDOW_COMPAT_FLAGS),compatFlagsOpen)) compatFlagsOpen=!compatFlagsOpen;
          ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(_("assets"))) {
          if (settings.unifiedDataView) {
            if (ImGui::MenuItem(_("assets"), BIND_FOR(GUI_ACTION_WINDOW_INS_LIST), insListOpen)) insListOpen = !insListOpen;
          } else {
            if (ImGui::MenuItem(_("instruments"), BIND_FOR(GUI_ACTION_WINDOW_INS_LIST), insListOpen)) insListOpen = !insListOpen;
            if (ImGui::MenuItem(_("samples"), BIND_FOR(GUI_ACTION_WINDOW_SAMPLE_LIST), sampleListOpen)) sampleListOpen = !sampleListOpen;
            if (ImGui::MenuItem(_("wavetables"), BIND_FOR(GUI_ACTION_WINDOW_WAVE_LIST), waveListOpen)) waveListOpen = !waveListOpen;
          }
          ImGui::Separator();
          if (ImGui::MenuItem(_("instrument editor"), BIND_FOR(GUI_ACTION_WINDOW_INS_EDIT), insEditOpen)) insEditOpen = !insEditOpen;
          if (ImGui::MenuItem(_("sample editor"), BIND_FOR(GUI_ACTION_WINDOW_SAMPLE_EDIT), sampleEditOpen)) sampleEditOpen = !sampleEditOpen;
          if (ImGui::MenuItem(_("wavetable editor"), BIND_FOR(GUI_ACTION_WINDOW_WAVE_EDIT), waveEditOpen)) waveEditOpen = !waveEditOpen;
          ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(_("visualizers"))) {
          if (ImGui::MenuItem(_("oscilloscope (master)"),BIND_FOR(GUI_ACTION_WINDOW_OSCILLOSCOPE),oscOpen)) oscOpen=!oscOpen;
          if (ImGui::MenuItem(_("oscilloscope (per-channel)"),BIND_FOR(GUI_ACTION_WINDOW_CHAN_OSC),chanOscOpen)) chanOscOpen=!chanOscOpen;
          if (ImGui::MenuItem(_("oscilloscope (X-Y)"),BIND_FOR(GUI_ACTION_WINDOW_XY_OSC),xyOscOpen)) xyOscOpen=!xyOscOpen;
          if (ImGui::MenuItem(_("volume meter"),BIND_FOR(GUI_ACTION_WINDOW_VOL_METER),volMeterOpen)) volMeterOpen=!volMeterOpen;
          ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(_("tempo"))) {
          if (ImGui::MenuItem(_("clock"),BIND_FOR(GUI_ACTION_WINDOW_CLOCK),clockOpen)) clockOpen=!clockOpen;
          if (ImGui::MenuItem(_("grooves"),BIND_FOR(GUI_ACTION_WINDOW_GROOVES),groovesOpen)) groovesOpen=!groovesOpen;
          if (ImGui::MenuItem(_("speed"),BIND_FOR(GUI_ACTION_WINDOW_SPEED),speedOpen)) speedOpen=!speedOpen;
          ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(_("debug"))) {
          if (ImGui::MenuItem(_("log viewer"),BIND_FOR(GUI_ACTION_WINDOW_LOG),logOpen)) logOpen=!logOpen;
          if (ImGui::MenuItem(_("register view"),BIND_FOR(GUI_ACTION_WINDOW_REGISTER_VIEW),regViewOpen)) regViewOpen=!regViewOpen;
          if (ImGui::MenuItem(_("statistics"),BIND_FOR(GUI_ACTION_WINDOW_STATS),statsOpen)) statsOpen=!statsOpen;
          if (ImGui::MenuItem(_("memory composition"),BIND_FOR(GUI_ACTION_WINDOW_MEMORY),memoryOpen)) memoryOpen=!memoryOpen;
          if (ImGui::MenuItem(_("command stream player"),BIND_FOR(GUI_ACTION_WINDOW_CS_PLAYER),csPlayerOpen)) csPlayerOpen=!csPlayerOpen;
          ImGui::EndMenu();
        }
        ImGui::Separator();
        if (ImGui::MenuItem(_("effect list"),BIND_FOR(GUI_ACTION_WINDOW_EFFECT_LIST),effectListOpen)) effectListOpen=!effectListOpen;
        if (ImGui::MenuItem(_("play/edit controls"),BIND_FOR(GUI_ACTION_WINDOW_EDIT_CONTROLS),editControlsOpen)) editControlsOpen=!editControlsOpen;
        if (ImGui::MenuItem(_("piano/input pad"),BIND_FOR(GUI_ACTION_WINDOW_PIANO),pianoOpen)) pianoOpen=!pianoOpen;
        if (spoilerOpen) if (ImGui::MenuItem(_("spoiler"),NULL,spoilerOpen)) spoilerOpen=!spoilerOpen;

        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu(settings.capitalMenuBar?_("Help"):_("help"))) {
        if (ImGui::MenuItem(_("effect list"),BIND_FOR(GUI_ACTION_WINDOW_EFFECT_LIST),effectListOpen)) effectListOpen=!effectListOpen;
        if (ImGui::MenuItem(_("debug menu"),BIND_FOR(GUI_ACTION_WINDOW_DEBUG))) debugOpen=!debugOpen;
        if (ImGui::MenuItem(_("inspector"))) inspectorOpen=!inspectorOpen;
        if (ImGui::MenuItem(_("panic"),BIND_FOR(GUI_ACTION_PANIC))) e->syncReset();
        if (ImGui::MenuItem(_("welcome screen"))) tutorial.protoWelcome=false;
        if (ImGui::MenuItem(_("about..."),BIND_FOR(GUI_ACTION_WINDOW_ABOUT))) {
          aboutOpen=true;
          aboutScroll=0;
        }
        ImGui::EndMenu();
      }
      ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PLAYBACK_STAT]);
      if (e->isPlaying() && settings.playbackTime) {
        int totalTicks=e->getTotalTicks();
        int totalSeconds=e->getTotalSeconds();

        String info;

        DivGroovePattern gp=e->getSpeeds();
        if (gp.len==2) {
          info=fmt::sprintf(_("| Speed %d:%d"),gp.val[0],gp.val[1]);
        } else if (gp.len==1) {
          info=fmt::sprintf(_("| Speed %d"),gp.val[0]);
        } else {
          info=_("| Groove");
        }

        info+=fmt::sprintf(_(" @ %gHz (%g BPM) "),e->getCurHz(),calcBPM(e->getSpeeds(),e->getCurHz(),e->getVirtualTempoN(),e->getVirtualTempoD()));

        if (settings.orderRowsBase) {
          info+=fmt::sprintf(_("| Order %.2X/%.2X "),playOrder,e->curSubSong->ordersLen-1);
        } else {
          info+=fmt::sprintf(_("| Order %d/%d "),playOrder,e->curSubSong->ordersLen-1);
        }

        if (settings.patRowsBase) {
          info+=fmt::sprintf(_("| Row %.2X/%.2X "),oldRow,e->curSubSong->patLen);
        } else {
          info+=fmt::sprintf(_("| Row %d/%d "),oldRow,e->curSubSong->patLen);
        }

        info+=_("| ");

        if (totalSeconds==0x7fffffff) {
          info+=_("Don't you have anything better to do?");
        } else {
          if (totalSeconds>=86400) {
            int totalDays=totalSeconds/86400;
            int totalYears=totalDays/365;
            totalDays%=365;
            int totalMonths=totalDays/30;
            totalDays%=30;

#ifdef HAVE_LOCALE
            info+=fmt::sprintf(ngettext("%d year ","%d years ",totalYears),totalYears);
            info+=fmt::sprintf(ngettext("%d month ","%d months ",totalMonths),totalMonths);
            info+=fmt::sprintf(ngettext("%d day ","%d days ",totalDays),totalDays);
#else
            info+=fmt::sprintf(_GN("%d year ","%d years ",totalYears),totalYears);
            info+=fmt::sprintf(_GN("%d month ","%d months ",totalMonths),totalMonths);
            info+=fmt::sprintf(_GN("%d day ","%d days ",totalDays),totalDays);
#endif
          }

          if (totalSeconds>=3600) {
            info+=fmt::sprintf("%.2d:",(totalSeconds/3600)%24);
          }

          info+=fmt::sprintf("%.2d:%.2d.%.2d",(totalSeconds/60)%60,totalSeconds%60,totalTicks/10000);
        }

        ImGui::TextUnformatted(info.c_str());
      } else {
        bool hasInfo=false;
        String info;
        if (cursor.xCoarse>=0 && cursor.xCoarse<e->getTotalChannelCount()) {
          DivPattern* p=e->curPat[cursor.xCoarse].getPattern(e->curOrders->ord[cursor.xCoarse][curOrder],false);
          if (cursor.xFine>=0) switch (cursor.xFine) {
            case 0: // note
              if (p->data[cursor.y][0]>0) {
                if (p->data[cursor.y][0]==100) {
                  info=fmt::sprintf(_("Note off (cut)"));
                } else if (p->data[cursor.y][0]==101) {
                  info=fmt::sprintf(_("Note off (release)"));
                } else if (p->data[cursor.y][0]==102) {
                  info=fmt::sprintf(_("Macro release only"));
                } else {
                  info=fmt::sprintf(_("Note on: %s"),noteName(p->data[cursor.y][0],p->data[cursor.y][1]));
                }
                hasInfo=true;
              }
              break;
            case 1: // instrument
              if (p->data[cursor.y][2]>-1) {
                if (p->data[cursor.y][2]>=(int)e->song.ins.size()) {
                  info=fmt::sprintf(_("Ins %d: <invalid>"),p->data[cursor.y][2]);
                } else {
                  DivInstrument* ins=e->getIns(p->data[cursor.y][2]);
                  info=fmt::sprintf(_("Ins %d: %s"),p->data[cursor.y][2],ins->name);
                }
                hasInfo=true;
              }
              break;
            case 2: // volume
              if (p->data[cursor.y][3]>-1) {
                int maxVol=e->getMaxVolumeChan(cursor.xCoarse);
                if (maxVol<1 || p->data[cursor.y][3]>maxVol) {
                  info=fmt::sprintf(_("Set volume: %d (%.2X, INVALID!)"),p->data[cursor.y][3],p->data[cursor.y][3]);
                } else {
                  float realVol=e->getGain(cursor.xCoarse,p->data[cursor.y][3]);
                  info=fmt::sprintf(_("Set volume: %d (%.2X, %d%%)"),p->data[cursor.y][3],p->data[cursor.y][3],(int)(realVol*100.0f));
                }
                hasInfo=true;
              }
              break;
            default: // effect
              int actualCursor=((cursor.xFine+1)&(~1));
              if (p->data[cursor.y][actualCursor]>-1) {
                info=e->getEffectDesc(p->data[cursor.y][actualCursor],cursor.xCoarse,true);
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
        ImGui::Text(_("| modified"));
      }
      ImGui::EndMainMenuBar();
    }

    MEASURE(calcChanOsc,calcChanOsc());

    if (mobileUI) {
      globalWinFlags=ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoBringToFrontOnFocus;
      //globalWinFlags=ImGuiWindowFlags_NoTitleBar;
      // scene handling goes here!
      MEASURE(mobileControls,drawMobileControls());
      switch (mobScene) {
        case GUI_SCENE_ORDERS:
          ordersOpen=true;
          curWindow=GUI_WINDOW_ORDERS;
          MEASURE(orders,drawOrders());
          MEASURE(piano,drawPiano());
          break;
        case GUI_SCENE_INSTRUMENT:
          insEditOpen=true;
          curWindow=GUI_WINDOW_INS_EDIT;
          MEASURE(insEdit,drawInsEdit());
          MEASURE(piano,drawPiano());
          break;
        case GUI_SCENE_WAVETABLE:
          waveEditOpen=true;
          curWindow=GUI_WINDOW_WAVE_EDIT;
          MEASURE(waveEdit,drawWaveEdit());
          MEASURE(piano,drawPiano());
          break;
        case GUI_SCENE_SAMPLE:
          sampleEditOpen=true;
          curWindow=GUI_WINDOW_SAMPLE_EDIT;
          MEASURE(sampleEdit,drawSampleEdit());
          MEASURE(piano,drawPiano());
          break;
        case GUI_SCENE_CHANNELS:
          channelsOpen=true;
          curWindow=GUI_WINDOW_CHANNELS;
          MEASURE(channels,drawChannels());
          break;
        case GUI_SCENE_CHIPS:
          sysManagerOpen=true;
          curWindow=GUI_WINDOW_SYS_MANAGER;
          MEASURE(sysManager,drawSysManager());
          break;
        case GUI_SCENE_MIXER:
          mixerOpen=true;
          curWindow=GUI_WINDOW_MIXER;
          MEASURE(mixer,drawMixer());
          break;
        default:
          patternOpen=true;
          curWindow=GUI_WINDOW_PATTERN;
          MEASURE(pattern,drawPattern());
          MEASURE(piano,drawPiano());
          MEASURE(mobileOrderSel,drawMobileOrderSel());

          globalWinFlags=0;
          MEASURE(findReplace,drawFindReplace());
          break;
      }

      globalWinFlags=0;
      MEASURE(settings,drawSettings());
      MEASURE(debug,drawDebug());
      MEASURE(csPlayer,drawCSPlayer());
      MEASURE(log,drawLog());
      MEASURE(compatFlags,drawCompatFlags());
      MEASURE(stats,drawStats());
      MEASURE(readOsc,readOsc());
      MEASURE(osc,drawOsc());
      MEASURE(chanOsc,drawChanOsc());
      MEASURE(xyOsc,drawXYOsc());
      MEASURE(volMeter,drawVolMeter());
      MEASURE(grooves,drawGrooves());
      MEASURE(regView,drawRegView());
      MEASURE(memory,drawMemory());
      MEASURE(userPresets,drawUserPresets());
      MEASURE(patManager,drawPatManager());
    } else {
      globalWinFlags=0;
      ImGui::DockSpaceOverViewport(NULL,lockLayout?(ImGuiDockNodeFlags_NoWindowMenuButton|ImGuiDockNodeFlags_NoMove|ImGuiDockNodeFlags_NoResize|ImGuiDockNodeFlags_NoCloseButton|ImGuiDockNodeFlags_NoDocking|ImGuiDockNodeFlags_NoDockingSplitMe|ImGuiDockNodeFlags_NoDockingSplitOther):0);

      MEASURE(subSongs,drawSubSongs());
      MEASURE(findReplace,drawFindReplace());
      MEASURE(spoiler,drawSpoiler());
      MEASURE(pattern,drawPattern());
      MEASURE(editControls,drawEditControls());
      MEASURE(speed,drawSpeed());
      MEASURE(grooves,drawGrooves());
      MEASURE(songInfo,drawSongInfo());
      MEASURE(orders,drawOrders());
      MEASURE(sampleList,drawSampleList());
      MEASURE(sampleEdit,drawSampleEdit());
      MEASURE(waveList,drawWaveList());
      MEASURE(waveEdit,drawWaveEdit());
      MEASURE(insList,drawInsList());
      MEASURE(insEdit,drawInsEdit());
      MEASURE(mixer,drawMixer());

      MEASURE(readOsc,readOsc());

      MEASURE(osc,drawOsc());
      MEASURE(chanOsc,drawChanOsc());
      MEASURE(xyOsc,drawXYOsc());
      MEASURE(volMeter,drawVolMeter());
      MEASURE(settings,drawSettings());
      MEASURE(debug,drawDebug());
      MEASURE(csPlayer,drawCSPlayer());
      MEASURE(stats,drawStats());
      MEASURE(memory,drawMemory());
      MEASURE(compatFlags,drawCompatFlags());
      MEASURE(piano,drawPiano());
      MEASURE(notes,drawNotes());
      MEASURE(channels,drawChannels());
      MEASURE(patManager,drawPatManager());
      MEASURE(sysManager,drawSysManager());
      MEASURE(clock,drawClock());
      MEASURE(regView,drawRegView());
      MEASURE(log,drawLog());
      MEASURE(effectList,drawEffectList());
      MEASURE(userPresets,drawUserPresets());
    }

    // release selection if mouse released
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && selecting) {
      if (!selectingFull) cursor=selEnd;
      finishSelection();
      if (!mobileUI) {
        demandScrollX=true;
        if (cursor.xCoarse==selStart.xCoarse && cursor.xFine==selStart.xFine && cursor.y==selStart.y &&
            cursor.xCoarse==selEnd.xCoarse && cursor.xFine==selEnd.xFine && cursor.y==selEnd.y) {
          if (!settings.cursorMoveNoScroll) {
            updateScroll(cursor.y);
          }
        }
      }
    }

    for (int i=0; i<e->getTotalChannelCount(); i++) {
      keyHit1[i]-=0.2f;
      if (keyHit1[i]<0.0f) keyHit1[i]=0.0f;
    }

    if (inspectorOpen) ImGui::ShowMetricsWindow(&inspectorOpen);

    if (firstFrame) {
      firstFrame=false;
#ifdef IS_MOBILE
      SDL_GetWindowSize(sdlWin,&scrW,&scrH);
      portrait=(scrW<scrH);
      logV("portrait: %d (%dx%d)",portrait,scrW,scrH);

      rend->getOutputSize(canvasW,canvasH);
#endif
      if (patternOpen) nextWindow=GUI_WINDOW_PATTERN;
#ifdef __APPLE__
      SDL_RaiseWindow(sdlWin);
#endif
    }

#ifndef NFD_NON_THREADED
#ifndef FLATPAK_WORKAROUNDS
    if (fileDialog->isOpen() && settings.sysFileDialog) {
      ImGui::OpenPopup(_("System File Dialog Pending"));
    }

    if (ImGui::BeginPopupModal(_("System File Dialog Pending"),NULL,ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoBackground|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove)) {
      if (!fileDialog->isOpen()) {
        ImGui::CloseCurrentPopup();
      }
      ImDrawList* dl=ImGui::GetForegroundDrawList();
      dl->AddRectFilled(ImVec2(0.0f,0.0f),ImVec2(canvasW,canvasH),ImGui::ColorConvertFloat4ToU32(uiColors[GUI_COLOR_MODAL_BACKDROP]));
      ImGui::EndPopup();
    }
#endif
#endif

    if (fileDialog->render(mobileUI?ImVec2(canvasW-(portrait?0:(60.0*dpiScale)),canvasH-60.0*dpiScale):ImVec2(600.0f*dpiScale,400.0f*dpiScale),ImVec2(canvasW-((mobileUI && !portrait)?(60.0*dpiScale):0),canvasH-(mobileUI?(60.0*dpiScale):0)))) {
      bool openOpen=false;
      //ImGui::GetIO().ConfigFlags&=~ImGuiConfigFlags_NavEnableKeyboard;
      if ((curFileDialog==GUI_FILE_INS_OPEN || curFileDialog==GUI_FILE_INS_OPEN_REPLACE) && prevIns!=-3) {
        if (curFileDialog==GUI_FILE_INS_OPEN_REPLACE) {
          if (prevInsData!=NULL) {
            if (prevIns>=0 && prevIns<(int)e->song.ins.size()) {
              *e->song.ins[prevIns]=*prevInsData;
            }
          }
        } else {
          curIns=prevIns;
          wavePreviewInit=true;
          updateFMPreview=true;
        }
        prevIns=-3;
      }
      switch (curFileDialog) {
        case GUI_FILE_OPEN:
        case GUI_FILE_SAVE:
        case GUI_FILE_SAVE_DMF:
        case GUI_FILE_SAVE_DMF_LEGACY:
          workingDirSong=fileDialog->getPath()+DIR_SEPARATOR_STR;
          break;
        case GUI_FILE_INS_OPEN:
        case GUI_FILE_INS_OPEN_REPLACE:
        case GUI_FILE_INS_SAVE:
        case GUI_FILE_INS_SAVE_DMP:
        case GUI_FILE_INS_SAVE_ALL:
          workingDirIns=fileDialog->getPath()+DIR_SEPARATOR_STR;
          break;
        case GUI_FILE_WAVE_OPEN:
        case GUI_FILE_WAVE_OPEN_REPLACE:
        case GUI_FILE_WAVE_SAVE:
        case GUI_FILE_WAVE_SAVE_DMW:
        case GUI_FILE_WAVE_SAVE_RAW:
        case GUI_FILE_WAVE_SAVE_ALL:
          workingDirWave=fileDialog->getPath()+DIR_SEPARATOR_STR;
          break;
        case GUI_FILE_SAMPLE_OPEN:
        case GUI_FILE_SAMPLE_OPEN_RAW:
        case GUI_FILE_SAMPLE_OPEN_REPLACE:
        case GUI_FILE_SAMPLE_OPEN_REPLACE_RAW:
        case GUI_FILE_SAMPLE_SAVE:
        case GUI_FILE_SAMPLE_SAVE_RAW:
        case GUI_FILE_SAMPLE_SAVE_ALL:
          workingDirSample=fileDialog->getPath()+DIR_SEPARATOR_STR;
          break;
        case GUI_FILE_EXPORT_AUDIO_ONE:
        case GUI_FILE_EXPORT_AUDIO_PER_SYS:
        case GUI_FILE_EXPORT_AUDIO_PER_CHANNEL:
          workingDirAudioExport=fileDialog->getPath()+DIR_SEPARATOR_STR;
          break;
        case GUI_FILE_EXPORT_VGM:
          workingDirVGMExport=fileDialog->getPath()+DIR_SEPARATOR_STR;
          break;
        case GUI_FILE_EXPORT_ROM:
        case GUI_FILE_EXPORT_TEXT:
        case GUI_FILE_EXPORT_CMDSTREAM:
          workingDirROMExport=fileDialog->getPath()+DIR_SEPARATOR_STR;
          break;
        case GUI_FILE_LOAD_MAIN_FONT:
        case GUI_FILE_LOAD_HEAD_FONT:
        case GUI_FILE_LOAD_PAT_FONT:
          workingDirFont=fileDialog->getPath()+DIR_SEPARATOR_STR;
          break;
        case GUI_FILE_IMPORT_COLORS:
        case GUI_FILE_EXPORT_COLORS:
          workingDirColors=fileDialog->getPath()+DIR_SEPARATOR_STR;
          break;
        case GUI_FILE_IMPORT_KEYBINDS:
        case GUI_FILE_EXPORT_KEYBINDS:
          workingDirKeybinds=fileDialog->getPath()+DIR_SEPARATOR_STR;
          break;
        case GUI_FILE_IMPORT_LAYOUT:
        case GUI_FILE_EXPORT_LAYOUT:
          workingDirLayout=fileDialog->getPath()+DIR_SEPARATOR_STR;
          break;
        case GUI_FILE_IMPORT_USER_PRESETS:
        case GUI_FILE_IMPORT_USER_PRESETS_REPLACE:
        case GUI_FILE_EXPORT_USER_PRESETS:
        case GUI_FILE_IMPORT_CONFIG:
        case GUI_FILE_EXPORT_CONFIG:
          workingDirConfig=fileDialog->getPath()+DIR_SEPARATOR_STR;
          break;
        case GUI_FILE_YRW801_ROM_OPEN:
        case GUI_FILE_TG100_ROM_OPEN:
        case GUI_FILE_MU5_ROM_OPEN:
          workingDirROM=fileDialog->getPath()+DIR_SEPARATOR_STR;
          break;
        case GUI_FILE_CMDSTREAM_OPEN:
          workingDirROM=fileDialog->getPath()+DIR_SEPARATOR_STR;
          break;
        case GUI_FILE_TEST_OPEN:
        case GUI_FILE_TEST_OPEN_MULTI:
        case GUI_FILE_TEST_SAVE:
          workingDirTest=fileDialog->getPath()+DIR_SEPARATOR_STR;
          break;
        case GUI_FILE_OPEN_BACKUP:
          break;
      }
      if (fileDialog->isError()) {
#if defined(_WIN32) || defined(__APPLE__)
        showError(_("there was an error in the file dialog! you may want to report this issue to:\nhttps://github.com/tildearrow/furnace/issues\ncheck the Log Viewer (window > log viewer) for more information.\n\nfor now please disable the system file picker in Settings > General."));
#else
#ifdef ANDROID
        showError(_("can't do anything without Storage permissions!"));
#else
        showError(_("Zenity/KDialog not available!\nplease install one of these, or disable the system file picker in Settings > General."));
#endif
#endif
      }
      if (fileDialog->accepted()) {
        if (fileDialog->getFileName().empty()) {
          fileName="";
        } else {
          fileName=fileDialog->getFileName()[0];
        }
        if (fileName!="") {
          if (curFileDialog==GUI_FILE_SAVE) {
            checkExtension(".fur");
          }
          if (curFileDialog==GUI_FILE_SAVE_DMF) {
            checkExtension(".dmf");
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
          if (curFileDialog==GUI_FILE_INS_SAVE_DMP) {
            checkExtension(".dmp");
          }
          if (curFileDialog==GUI_FILE_WAVE_SAVE) {
            checkExtension(".fuw");
          }
          if (curFileDialog==GUI_FILE_WAVE_SAVE_DMW) {
            checkExtension(".dmw");
          }
          if (curFileDialog==GUI_FILE_WAVE_SAVE_RAW) {
            checkExtension(".raw");
          }
          if (curFileDialog==GUI_FILE_EXPORT_VGM) {
            checkExtension(".vgm");
          }
          if (curFileDialog==GUI_FILE_EXPORT_ROM) {
            checkExtension(romFilterExt.c_str());
          }
          if (curFileDialog==GUI_FILE_EXPORT_TEXT) {
            checkExtension(".txt");
          }
          if (curFileDialog==GUI_FILE_EXPORT_CMDSTREAM) {
            checkExtension(".bin");
          }
          if (curFileDialog==GUI_FILE_EXPORT_COLORS) {
            checkExtension(".cfgc");
          }
          if (curFileDialog==GUI_FILE_EXPORT_KEYBINDS) {
            checkExtension(".cfgk");
          }
          if (curFileDialog==GUI_FILE_EXPORT_LAYOUT) {
            checkExtension(".ini");
          }
          if (curFileDialog==GUI_FILE_EXPORT_USER_PRESETS) {
            checkExtension(".cfgu");
          }
          if (curFileDialog==GUI_FILE_EXPORT_CONFIG) {
            checkExtension(".cfg");
          }
          String copyOfName=fileName;
          switch (curFileDialog) {
            case GUI_FILE_OPEN:
            case GUI_FILE_OPEN_BACKUP:
              if (load(copyOfName)>0) {
                showError(fmt::sprintf(_("Error while loading file! (%s)"),lastError));
              }
              break;
            case GUI_FILE_SAVE: {
              bool saveWasSuccessful=true;
              if (save(copyOfName,0)>0) {
                showError(fmt::sprintf(_("Error while saving file! (%s)"),lastError));
                saveWasSuccessful=false;
              }
              if (saveWasSuccessful && postWarnAction!=GUI_WARN_GENERIC) {
                switch (postWarnAction) {
                  case GUI_WARN_QUIT:
                    quit=true;
                    break;
                  case GUI_WARN_NEW:
                    displayNew=true;
                    break;
                  case GUI_WARN_OPEN:
                    openOpen=true;
                    break;
                  case GUI_WARN_OPEN_DROP:
                    if (load(nextFile)>0) {
                      showError(fmt::sprintf(_("Error while loading file! (%s)"),lastError));
                    }
                    nextFile="";
                    break;
                  case GUI_WARN_OPEN_BACKUP:
                    openFileDialog(GUI_FILE_OPEN_BACKUP);
                    break;
                  case GUI_WARN_CV:
                    cvOpen=true;
                    break;
                  default:
                    break;
                }
                postWarnAction=GUI_WARN_GENERIC;
              } else if (postWarnAction==GUI_WARN_OPEN_DROP) {
                nextFile="";
              }
              break;
            }
            case GUI_FILE_SAVE_DMF:
              logD("saving: %s",copyOfName.c_str());
              if (save(copyOfName,26)>0) {
                showError(fmt::sprintf(_("Error while saving file! (%s)"),lastError));
              }
              break;
            case GUI_FILE_SAVE_DMF_LEGACY:
              logD("saving: %s",copyOfName.c_str());
              if (save(copyOfName,24)>0) {
                showError(fmt::sprintf(_("Error while saving file! (%s)"),lastError));
              }
              break;
            case GUI_FILE_INS_SAVE:
              if (curIns>=0 && curIns<(int)e->song.ins.size()) {
                if (e->song.ins[curIns]->save(copyOfName.c_str(),&e->song,settings.writeInsNames)) {
                  pushRecentSys(copyOfName.c_str());
                }
              }
              break;
            case GUI_FILE_INS_SAVE_DMP:
              if (curIns>=0 && curIns<(int)e->song.ins.size()) {
                if (!e->song.ins[curIns]->saveDMP(copyOfName.c_str())) {
                  showError(_("error while saving instrument! only the following instrument types are supported:\n- FM (OPN)\n- SN76489/Sega PSG\n- Game Boy\n- PC Engine\n- NES\n- C64\n- FM (OPLL)\n- FDS"));
                } else {
                  pushRecentSys(copyOfName.c_str());
                }
              }
              break;
            case GUI_FILE_INS_SAVE_ALL: {
              String errors;
              for (int i=0; i<e->song.insLen; i++) {
                String nextPath=copyOfName;
                nextPath+=DIR_SEPARATOR_STR;
                nextPath+=fmt::sprintf("%.2X_",i);
                for (char j: e->song.ins[i]->name) {
                  switch (j) {
                    // these chars are reserved
                    case '/': case '<': case '>': case ':': case '"': case '\\': case '|': case '?': case '*':
                      nextPath+='_';
                      break;
                    default:
                      nextPath+=j;
                      break;
                  }
                }
                nextPath+=".fui";
                logV("%s",nextPath);
                if (!e->song.ins[i]->save(nextPath.c_str(),&e->song,settings.writeInsNames)) {
                  errors+=fmt::sprintf("%s: could not save!\n",e->song.ins[i]->name);
                }
              }

              if (!errors.empty()) {
                showError(errors);
              }
              break;
            }
            case GUI_FILE_WAVE_SAVE_ALL: {
              String errors;
              for (int i=0; i<e->song.waveLen; i++) {
                String nextPath=copyOfName;
                nextPath+=DIR_SEPARATOR_STR;
                nextPath+=fmt::sprintf("%.2X.fuw",i);
                logV("%s",nextPath);
                if (!e->song.wave[i]->save(nextPath.c_str())) {
                  errors+=fmt::sprintf("%d: could not save!\n",i);
                }
              }

              if (!errors.empty()) {
                showError(errors);
              }
              break;
            }
            case GUI_FILE_SAMPLE_SAVE_ALL: {
              String errors;
              for (int i=0; i<e->song.sampleLen; i++) {
                String nextPath=copyOfName;
                nextPath+=DIR_SEPARATOR_STR;
                nextPath+=fmt::sprintf("%.2X_",i);
                for (char j: e->song.sample[i]->name) {
                  switch (j) {
                    // these chars are reserved
                    case '/': case '<': case '>': case ':': case '"': case '\\': case '|': case '?': case '*':
                      nextPath+='_';
                      break;
                    default:
                      nextPath+=j;
                      break;
                  }
                }
                nextPath+=".wav";
                logV("%s",nextPath);
                if (!e->song.sample[i]->save(nextPath.c_str())) {
                  errors+=fmt::sprintf("%s: could not save!\n",e->song.sample[i]->name);
                }
              }

              if (!errors.empty()) {
                showError(errors);
              }
              break;
            }
            case GUI_FILE_WAVE_SAVE:
              if (curWave>=0 && curWave<(int)e->song.wave.size()) {
                if (e->song.wave[curWave]->save(copyOfName.c_str())) {
                  pushRecentSys(copyOfName.c_str());
                }
              }
              break;
            case GUI_FILE_WAVE_SAVE_DMW:
              if (curWave>=0 && curWave<(int)e->song.wave.size()) {
                if (e->song.wave[curWave]->saveDMW(copyOfName.c_str())) {
                  pushRecentSys(copyOfName.c_str());
                }
              }
              break;
            case GUI_FILE_WAVE_SAVE_RAW:
              if (curWave>=0 && curWave<(int)e->song.wave.size()) {
                if (e->song.wave[curWave]->saveRaw(copyOfName.c_str())) {
                  pushRecentSys(copyOfName.c_str());
                }
              }
              break;
            case GUI_FILE_SAMPLE_OPEN: {
              String errs=_("there were some errors while loading samples:\n");
              bool warn=false;
              for (String i: fileDialog->getFileName()) {
                std::vector<DivSample*> samples=e->sampleFromFile(i.c_str());
                if (samples.empty()) {
                  if (fileDialog->getFileName().size()>1) {
                    warn=true;
                    errs+=fmt::sprintf("- %s: %s\n",i,e->getLastError());
                  } else {;
                    showError(e->getLastError());
                  }
                } 
                else 
                {
                  if((int)samples.size() == 1)
                  {
                    if (e->addSamplePtr(samples[0]) == -1)
                    {
                      if (fileDialog->getFileName().size()>1)
                      {
                        warn=true;
                        errs+=fmt::sprintf("- %s: %s\n",i,e->getLastError());
                      } 
                      else 
                      {
                        showError(e->getLastError());
                      }
                    } 
                    else 
                    {
                      MARK_MODIFIED;
                    }
                  }
                  else
                  {
                    for (DivSample* s: samples) { //ask which samples to load!
                      pendingSamples.push_back(std::make_pair(s,false));
                    }
                    displayPendingSamples=true;
                    replacePendingSample = false;
                  }
                }
              }
              if (warn) {
                showWarning(errs,GUI_WARN_GENERIC);
              }
              break;
            }
            case GUI_FILE_SAMPLE_OPEN_REPLACE: {
              std::vector<DivSample*> samples=e->sampleFromFile(copyOfName.c_str());
              if (samples.empty()) {
                showError(e->getLastError());
              } else {
                if ((int)samples.size()==1) {
                  if (curSample>=0 && curSample<(int)e->song.sample.size()) {
                    DivSample* s=samples[0];
                    e->lockEngine([this,s]() {
                      // if it crashes here please tell me...
                      DivSample* oldSample=e->song.sample[curSample];
                      e->song.sample[curSample]=s;
                      delete oldSample;
                      e->renderSamples();
                      MARK_MODIFIED;
                    });
                    updateSampleTex=true;
                  } else {
                    showError(_("...but you haven't selected a sample!"));
                    delete samples[0];
                  }
                } else {
                  for (DivSample* s: samples) { // ask which samples to load!
                    pendingSamples.push_back(std::make_pair(s,false));
                  }
                  displayPendingSamples=true;
                  replacePendingSample=true;
                }
              }
              break;
            }
            case GUI_FILE_SAMPLE_OPEN_RAW:
            case GUI_FILE_SAMPLE_OPEN_REPLACE_RAW:
              pendingRawSample=copyOfName;
              pendingRawSampleReplace=(curFileDialog==GUI_FILE_SAMPLE_OPEN_REPLACE_RAW);
              displayPendingRawSample=true;
              break;
            case GUI_FILE_SAMPLE_SAVE:
              if (curSample>=0 && curSample<(int)e->song.sample.size()) {
                if (!e->song.sample[curSample]->save(copyOfName.c_str())) {
                  showError(_("could not save sample! open Log Viewer for more information."));
                } else {
                  pushRecentSys(copyOfName.c_str());
                }
              }
              break;
            case GUI_FILE_SAMPLE_SAVE_RAW:
              if (curSample>=0 && curSample<(int)e->song.sample.size()) {
                if (!e->song.sample[curSample]->saveRaw(copyOfName.c_str())) {
                  showError(_("could not save sample! open Log Viewer for more information."));
                } else {
                  pushRecentSys(copyOfName.c_str());
                }
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
              std::vector<DivInstrument*> instruments;
              bool ask=false;
              bool warn=false;
              String warns=_("there were some warnings/errors while loading instruments:\n");
              int sampleCountBefore=e->song.sampleLen;
              for (String i: fileDialog->getFileName()) {
                std::vector<DivInstrument*> insTemp=e->instrumentFromFile(i.c_str(),true,settings.readInsNames);
                if (insTemp.empty()) {
                  warn=true;
                  warns+=fmt::sprintf(_("> %s: cannot load instrument! (%s)\n"),i,e->getLastError());
                } else if (!e->getWarnings().empty()) {
                  warn=true;
                  warns+=fmt::sprintf("> %s:\n%s\n",i,e->getWarnings());
                }
                if (insTemp.size()>1) ask=true;
                for (DivInstrument* j: insTemp) {
                  instruments.push_back(j);
                }
              }
              if (e->song.sampleLen!=sampleCountBefore) {
                e->renderSamplesP();
              }
              if (warn) {
                if (instruments.empty()) {
                  if (fileDialog->getFileName().size()>1) {
                    showError(warns);
                  } else {
                    showError(fmt::sprintf(_("cannot load instrument! (%s)"),e->getLastError()));
                  }
                } else {
                  showWarning(warns,GUI_WARN_GENERIC);
                }
              } else if (instruments.empty()) {
                showError(_("congratulations! you managed to load nothing.\nyou are entitled to a bug report."));
              }
              if (!instruments.empty()) {
                if (ask) { // ask which instruments to load
                  for (DivInstrument* i: instruments) {
                    pendingIns.push_back(std::make_pair(i,false));
                  }
                  displayPendingIns=true;
                  pendingInsSingle=false;
                } else { // load the only instrument
                  int instrumentCount=-1;
                  for (DivInstrument* i: instruments) {
                    instrumentCount=e->addInstrumentPtr(i);
                    MARK_MODIFIED;
                  }
                  if (instrumentCount>=0 && settings.selectAssetOnLoad) {
                    curIns=instrumentCount-1;
                  }
                }
              }
              break;
            }
            case GUI_FILE_INS_OPEN_REPLACE: {
              int sampleCountBefore=e->song.sampleLen;
              std::vector<DivInstrument*> instruments=e->instrumentFromFile(copyOfName.c_str(),true,settings.readInsNames);
              if (!instruments.empty()) {
                if (e->song.sampleLen!=sampleCountBefore) {
                  e->renderSamplesP();
                }
                if (!e->getWarnings().empty()) {
                  showWarning(e->getWarnings(),GUI_WARN_GENERIC);
                }
                if (instruments.size()>1) { // ask which instrument
                  for (DivInstrument* i: instruments) {
                    pendingIns.push_back(std::make_pair(i,false));
                  }
                  displayPendingIns=true;
                  pendingInsSingle=true;
                } else { // replace with the only instrument
                  if (curIns>=0 && curIns<(int)e->song.ins.size()) {
                    *e->song.ins[curIns]=*instruments[0];
                    MARK_MODIFIED;
                  } else {
                    showError(_("...but you haven't selected an instrument!"));
                  }
                  for (DivInstrument* i: instruments) {
                    delete i;
                  }
                }
              } else {
                showError(fmt::sprintf(_("cannot load instrument! (%s)"),e->getLastError()));
              }
              break;
            }
            case GUI_FILE_WAVE_OPEN: {
              String errs=_("there were some errors while loading wavetables:\n");
              bool warn=false;
              for (String i: fileDialog->getFileName()) {
                DivWavetable* wave=e->waveFromFile(i.c_str());
                if (wave==NULL) {
                  if (fileDialog->getFileName().size()>1) {
                    warn=true;
                    errs+=fmt::sprintf("- %s: %s\n",i,e->getLastError());
                  } else {
                    showError(fmt::sprintf(_("cannot load wavetable! (%s)"),e->getLastError()));
                  }
                } else {
                  int waveCount=-1;
                  waveCount=e->addWavePtr(wave);
                  if (waveCount==-1) {
                    if (fileDialog->getFileName().size()>1) {
                      warn=true;
                      errs+=fmt::sprintf("- %s: %s\n",i,e->getLastError());
                    } else {
                      showError(fmt::sprintf(_("cannot load wavetable! (%s)"),e->getLastError()));
                    }
                  } else {
                    if (settings.selectAssetOnLoad) {
                      curWave=waveCount-1;
                    }
                    MARK_MODIFIED;
                    RESET_WAVE_MACRO_ZOOM;
                  }
                }
              }
              if (warn) {
                showWarning(errs,GUI_WARN_GENERIC);
              }
              break;
            }
            case GUI_FILE_WAVE_OPEN_REPLACE: {
              DivWavetable* wave=e->waveFromFile(copyOfName.c_str());
              if (wave==NULL) {
                showError(fmt::sprintf(_("cannot load wavetable! (%s)"),e->getLastError()));
              } else {
                if (curWave>=0 && curWave<(int)e->song.wave.size()) {
                  e->lockEngine([this,wave]() {
                    *e->song.wave[curWave]=*wave;
                    MARK_MODIFIED;
                  });
                } else {
                  showError(_("...but you haven't selected a wavetable!"));
                }
                delete wave;
              }
              break;
            }
            case GUI_FILE_EXPORT_VGM: {
              SafeWriter* w=e->saveVGM(willExport,vgmExportLoop,vgmExportVersion,vgmExportPatternHints,vgmExportDirectStream,vgmExportTrailingTicks,vgmExportDPCM07);
              if (w!=NULL) {
                FILE* f=ps_fopen(copyOfName.c_str(),"wb");
                if (f!=NULL) {
                  fwrite(w->getFinalBuf(),1,w->size(),f);
                  fclose(f);
                  pushRecentSys(copyOfName.c_str());
                } else {
                  showError(_("could not open file!"));
                }
                w->finish();
                delete w;
                if (!e->getWarnings().empty()) {
                  showWarning(e->getWarnings(),GUI_WARN_GENERIC);
                }
              } else {
                showError(fmt::sprintf(_("could not write VGM! (%s)"),e->getLastError()));
              }
              break;
            }
            case GUI_FILE_EXPORT_ROM:
              romExportPath=copyOfName;
              pendingExport=e->buildROM(romTarget);
              if (pendingExport==NULL) {
                showError("could not create exporter! you may want to report this issue...");
              } else {
                pendingExport->setConf(romConfig);
                if (pendingExport->go(e)) {
                  displayExportingROM=true;
                  romExportSave=true;
                } else {
                  showError("could not begin exporting process! TODO: elaborate");
                }
              }
              break;
            case GUI_FILE_EXPORT_TEXT: {
              SafeWriter* w=e->saveText(false);
              if (w!=NULL) {
                FILE* f=ps_fopen(copyOfName.c_str(),"wb");
                if (f!=NULL) {
                  fwrite(w->getFinalBuf(),1,w->size(),f);
                  fclose(f);
                  pushRecentSys(copyOfName.c_str());
                } else {
                  showError(_("could not open file!"));
                }
                w->finish();
                delete w;
                if (!e->getWarnings().empty()) {
                  showWarning(e->getWarnings(),GUI_WARN_GENERIC);
                }
              } else {
                showError(fmt::sprintf(_("could not write text! (%s)"),e->getLastError()));
              }
              break;
            }
            case GUI_FILE_EXPORT_CMDSTREAM: {
              exportCmdStream(false,copyOfName);
              break;
            }
            case GUI_FILE_LOAD_MAIN_FONT:
              settings.mainFontPath=copyOfName;
              break;
            case GUI_FILE_LOAD_HEAD_FONT:
              settings.headFontPath=copyOfName;
              break;
            case GUI_FILE_LOAD_PAT_FONT:
              settings.patFontPath=copyOfName;
              break;
            case GUI_FILE_IMPORT_COLORS:
              importColors(copyOfName);
              break;
            case GUI_FILE_IMPORT_KEYBINDS:
              importKeybinds(copyOfName);
              break;
            case GUI_FILE_IMPORT_LAYOUT:
              importLayout(copyOfName);
              break;
            case GUI_FILE_IMPORT_USER_PRESETS:
              if (!loadUserPresets(false,copyOfName,true)) {
                showError(_("could not import user presets!"));
              }
              break;
            case GUI_FILE_IMPORT_USER_PRESETS_REPLACE:
              if (!loadUserPresets(false,copyOfName,false)) {
                showError(fmt::sprintf(_("could not import user presets! (%s)"),strerror(errno)));
              }
              break;
            case GUI_FILE_IMPORT_CONFIG:
              importConfig(copyOfName);
              break;
            case GUI_FILE_EXPORT_COLORS:
              exportColors(copyOfName);
              break;
            case GUI_FILE_EXPORT_KEYBINDS:
              exportKeybinds(copyOfName);
              break;
            case GUI_FILE_EXPORT_LAYOUT:
              exportLayout(copyOfName);
              break;
            case GUI_FILE_EXPORT_USER_PRESETS:
              if (!saveUserPresets(false,copyOfName)) {
                showError(fmt::sprintf(_("could not import user presets! (%s)"),strerror(errno)));
              }
              break;
            case GUI_FILE_EXPORT_CONFIG:
              exportConfig(copyOfName);
              break;
            case GUI_FILE_YRW801_ROM_OPEN:
              settings.yrw801Path=copyOfName;
              break;
            case GUI_FILE_TG100_ROM_OPEN:
              settings.tg100Path=copyOfName;
              break;
            case GUI_FILE_MU5_ROM_OPEN:
              settings.mu5Path=copyOfName;
              break;
            case GUI_FILE_CMDSTREAM_OPEN:
              if (loadStream(copyOfName)>0) {
                showError(fmt::sprintf(_("Error while loading file! (%s)"),lastError));
              }
              break;
            case GUI_FILE_TEST_OPEN:
              showWarning(fmt::sprintf(_("You opened: %s"),copyOfName),GUI_WARN_GENERIC);
              break;
            case GUI_FILE_TEST_OPEN_MULTI: {
              String msg=_("You opened:");
              for (String i: fileDialog->getFileName()) {
                msg+=fmt::sprintf("\n- %s",i);
              }
              showWarning(msg,GUI_WARN_GENERIC);
              break;
            }
            case GUI_FILE_TEST_SAVE:
              showWarning(fmt::sprintf(_("You saved: %s"),copyOfName),GUI_WARN_GENERIC);
              break;
          }
          curFileDialog=GUI_FILE_OPEN;
        }
      }
      fileDialog->close();
      postWarnAction=GUI_WARN_GENERIC;

      if (openOpen) {
        openFileDialog(GUI_FILE_OPEN);
      }
    }

    if (warnQuit && introPos>=11.0) {
      warnQuit=false;
      ImGui::OpenPopup(_("Warning"));
    }

    if (displayError && introPos>=11.0) {
      displayError=false;
      ImGui::OpenPopup(_("Error"));
    }

    if (displayPendingIns) {
      displayPendingIns=false;
      ImGui::OpenPopup(_("Select Instrument"));
    }

    if (displayPendingSamples) {
      displayPendingSamples=false;
      ImGui::OpenPopup(_("Select Sample"));
    }

    if (displayPendingRawSample) {
      displayPendingRawSample=false;
      ImGui::OpenPopup(_("Import Raw Sample"));
    }

    if (displayInsTypeList) {
      displayInsTypeList=false;
      ImGui::OpenPopup("InsTypeList");
    }

    if (displayWaveSizeList) {
      displayWaveSizeList=false;
      ImGui::OpenPopup("WaveSizeList");
    }

    if (displayExporting) {
      displayExporting=false;
      ImGui::OpenPopup(_("Rendering..."));
    }

    if (displayExportingROM) {
      displayExportingROM=false;
      ImGui::OpenPopup(_("ROM Export Progress"));
    }

    if (displayExportingCS) {
      displayExportingCS=false;
      ImGui::OpenPopup(_("CmdStream Export Progress"));
    }

    if (displayNew) {
      newSongQuery="";
      newSongFirstFrame=true;
      displayNew=false;
      if (settings.newSongBehavior==1) {
        e->createNewFromDefaults();
        undoHist.clear();
        redoHist.clear();
        curFileName="";
        modified=false;
        curNibble=false;
        orderNibble=false;
        orderCursor=-1;
        samplePos=0;
        updateSampleTex=true;
        selStart=SelectionPoint();
        selEnd=SelectionPoint();
        cursor=SelectionPoint();
        updateWindowTitle();
        updateROMExportAvail();
      } else {
        ImGui::OpenPopup(_("New Song"));
      }
    }

    if (displayPalette) {
      paletteSearchResults.clear();
      paletteQuery="";
      paletteFirstFrame=true;
      curPaletteChoice=0;
      displayPalette=false;
      ImGui::OpenPopup(_("Command Palette"));
    }

    if (displayExport) {
      displayExport=false;
      ImGui::OpenPopup(_("Export"));
    }

    if (displayEditString) {
      ImGui::OpenPopup("EditString");
    }

    if (nextWindow==GUI_WINDOW_ABOUT) {
      aboutOpen=true;
      nextWindow=GUI_WINDOW_NOTHING;
    }
    if (aboutOpen) drawAbout();

    MEASURE_BEGIN(popup);

    centerNextWindow(_("Rendering..."),canvasW,canvasH);
    // ImGui::SetNextWindowSize(ImVec2(0.0f,0.0f));
    if (ImGui::BeginPopupModal(_("Rendering..."),NULL,ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings)) {
      // WHAT the HELL?!
      WAKE_UP;
      if (audioExportOptions.mode!=DIV_EXPORT_MODE_MANY_CHAN) {
        ImGui::Text(_("Please wait..."));
      }
      float* progressLambda=&curProgress;
      int curPosInRows=0;
      int* curPosInRowsLambda=&curPosInRows;
      int loopsLeft=0;
      int* loopsLeftLambda=&loopsLeft;
      int totalLoops=0;
      int* totalLoopsLambda=&totalLoops;
      int curFile=0;
      int* curFileLambda=&curFile;
      if (e->isExporting()) {
        e->lockEngine(
          [this, progressLambda, curPosInRowsLambda, curFileLambda, loopsLeftLambda, totalLoopsLambda] () {
            int curRow=0; int curOrder=0;
            e->getCurSongPos(curRow, curOrder);
            *curFileLambda=0;
            e->getCurFileIndex(*curFileLambda);
            *curPosInRowsLambda=curRow;
            for (int i=0; i<curOrder; i++) *curPosInRowsLambda+=songOrdersLengths[i];
            if (!songHasSongEndCommand) {
              e->getLoopsLeft(*loopsLeftLambda);
              e->getTotalLoops(*totalLoopsLambda);
              if ((*totalLoopsLambda)!=(*loopsLeftLambda)) { // we are going 2nd, 3rd, etc. time through the song
                *curPosInRowsLambda-=(songLength-songLoopedSectionLength); // a hack so progress bar does not jump?
              }
              if (e->getIsFadingOut()) { // we are in fadeout??? why it works like that bruh
                // LIVE WITH IT damn it
                *curPosInRowsLambda-=(songLength-songLoopedSectionLength); // a hack so progress bar does not jump?
              }
            }
            *progressLambda=(float)((*curPosInRowsLambda)+((*totalLoopsLambda)-(*loopsLeftLambda))*songLength+lengthOfOneFile*(*curFileLambda))/(float)totalLength;
          }
        );
      }

      ImGui::Text(_("Row %d of %d"),curPosInRows+((totalLoops)-(loopsLeft))*songLength,lengthOfOneFile);
      if (audioExportOptions.mode==DIV_EXPORT_MODE_MANY_CHAN) ImGui::Text(_("Channel %d of %d"),curFile+1,totalFiles);

      ImGui::ProgressBar(curProgress,ImVec2(320.0f*dpiScale,0),fmt::sprintf("%.2f%%",curProgress*100.0f).c_str());

      if (ImGui::Button(_("Abort"))) {
        if (e->haltAudioFile()) {
          ImGui::CloseCurrentPopup();
        }
      }
      if (!e->isExporting()) {
        e->finishAudioFile();
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }

    ImVec2 romExportMinSize=mobileUI?ImVec2(canvasW-(portrait?0:(60.0*dpiScale)),canvasH-60.0*dpiScale):ImVec2(400.0f*dpiScale,200.0f*dpiScale);
    ImVec2 romExportMaxSize=ImVec2(canvasW-((mobileUI && !portrait)?(60.0*dpiScale):0),canvasH-(mobileUI?(60.0*dpiScale):0));

    centerNextWindow(_("ROM Export Progress"),canvasW,canvasH);
    ImGui::SetNextWindowSizeConstraints(romExportMinSize,romExportMaxSize);
    if (ImGui::BeginPopupModal(_("ROM Export Progress"),NULL)) {
      if (pendingExport==NULL) {
        ImGui::TextWrapped("%s",_("...ooooor you could try asking me a new ROM export?"));
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::Button(_("Erm what the sigma???"),ImVec2(ImGui::GetContentRegionAvail().x,0.0f))) {
          ImGui::CloseCurrentPopup();
        }
      } else {
        int progIndex=0;
        while (true) {
          DivROMExportProgress p=pendingExport->getProgress(progIndex);
          if (p.name.empty()) break;
          ImGui::Text("%s: %d%%",p.name.c_str(),(int)round(p.amount*100.0f));
          ImGui::ProgressBar(p.amount,ImVec2(-FLT_MIN,0));
          progIndex++;
        }
        ImVec2 romLogSize=ImGui::GetContentRegionAvail();
        romLogSize.y-=ImGui::GetFrameHeightWithSpacing();
        if (romLogSize.y<60.0f*dpiScale) romLogSize.y=60.0f*dpiScale;
        if (ImGui::BeginChild("Export Log",romLogSize,true)) {
          pendingExport->logLock.lock();
          ImGui::PushFont(patFont);
          for (String& i: pendingExport->exportLog) {
            ImGui::TextWrapped("%s",i.c_str());
          }
          if (romExportSave) {
            ImGui::SetScrollY(ImGui::GetScrollMaxY());
          }
          ImGui::PopFont();
          pendingExport->logLock.unlock();
        }
        ImGui::EndChild();
        if (pendingExport->isRunning()) {
          WAKE_UP;
          if (ImGui::Button(_("Abort"),ImVec2(ImGui::GetContentRegionAvail().x,0.0f))) {
            pendingExport->abort();
            delete pendingExport;
            pendingExport=NULL;
            romExportSave=false;
            ImGui::CloseCurrentPopup();
          }
        } else {
          if (romExportSave) {
            pendingExport->wait();
            if (!pendingExport->hasFailed()) {
              // save files here (romExportPath)
              for (DivROMExportOutput& i: pendingExport->getResult()) {
                String path=romExportPath;
                if (romMultiFile) {
                  path+=DIR_SEPARATOR_STR;
                  path+=i.name;
                }
                FILE* outFile=ps_fopen(path.c_str(),"wb");
                if (outFile!=NULL) {
                  fwrite(i.data->getFinalBuf(),1,i.data->size(),outFile);
                  fclose(outFile);
                } else {
                  // TODO: handle failure here
                }
                i.data->finish();
                delete i.data;
              }
            }
            romExportSave=false;
          }
          if (pendingExport!=NULL) {
            if (pendingExport->hasFailed()) {
              ImGui::AlignTextToFramePadding();
              ImGui::TextUnformatted(_("Error!"));
              ImGui::SameLine();
            }
          }
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Button(_("OK"),ImVec2(ImGui::GetContentRegionAvail().x,0.0f))) {
            delete pendingExport;
            pendingExport=NULL;
            ImGui::CloseCurrentPopup();
          }
        }
      }
      ImGui::EndPopup();
    }

    centerNextWindow(_("CmdStream Export Progress"),canvasW,canvasH);
    ImGui::SetNextWindowSizeConstraints(romExportMinSize,romExportMaxSize);
    if (ImGui::BeginPopupModal(_("CmdStream Export Progress"),NULL)) {
      if (csExportThread==NULL) {
        ImGui::TextWrapped("%s",_("it appears your Furnace has too many bugs in it. any song you can export?"));
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::Button(_("Talk With Devs"),ImVec2(ImGui::GetContentRegionAvail().x/3.0f,0.0f))) {
          ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button(_("Ask on Bug Report"),ImVec2(ImGui::GetContentRegionAvail().x/3.0f,0.0f))) {
          ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button(_("View Issues"),ImVec2(ImGui::GetContentRegionAvail().x/3.0f,0.0f))) {
          ImGui::CloseCurrentPopup();
        }
      } else {
        WAKE_UP;
        ImGui::Text("Exporting...");
        ImGui::Text("opt stage: %d",csProgress.optStage);
        ImGui::Text("pass: %d/%d",csProgress.optCurrent,csProgress.optTotal);
        ImGui::Text("find: %d/%d",csProgress.findCurrent,csProgress.findTotal);
        ImGui::Text("expand: %d/%d",csProgress.expandCurrent,csProgress.optCurrent);
        ImGui::Text("benefit: %d/%d",csProgress.origCurrent,csProgress.origCount);

        // check whether we're done
        if (csExportDone) {
          csExportThread->join();
          delete csExportThread;
          csExportThread=NULL;

          if (csExportTarget) { // command stream player
            if (csExportResult!=NULL) {
              if (!e->playStream(csExportResult->getFinalBuf(),csExportResult->size())) {
                showError(e->getLastError());
                csExportResult->finish();
                delete csExportResult;
              } else {
                csExportResult->disown();
                delete csExportResult;
              }
            } else {
              showError(_("oh no! it broke!"));
            }
            csExportResult=NULL;
          } else { // command stream export
            if (csExportResult!=NULL) {
              FILE* f=ps_fopen(csExportPath.c_str(),"wb");
              if (f!=NULL) {
                fwrite(csExportResult->getFinalBuf(),1,csExportResult->size(),f);
                fclose(f);
                pushRecentSys(csExportPath.c_str());
              } else {
                showError(_("could not open file!"));
              }
              csExportResult->finish();
              delete csExportResult;
              if (!e->getWarnings().empty()) {
                showWarning(e->getWarnings(),GUI_WARN_GENERIC);
              }
            } else {
              showError(fmt::sprintf(_("could not write command stream! (%s)"),e->getLastError()));
            }
            csExportResult=NULL;
          }

          ImGui::CloseCurrentPopup();
        }
      }
      ImGui::EndPopup();
    }

    drawTutorial();

    ImVec2 newSongMinSize=mobileUI?ImVec2(canvasW-(portrait?0:(60.0*dpiScale)),canvasH-60.0*dpiScale):ImVec2(400.0f*dpiScale,200.0f*dpiScale);
    ImVec2 newSongMaxSize=ImVec2(canvasW-((mobileUI && !portrait)?(60.0*dpiScale):0),canvasH-(mobileUI?(60.0*dpiScale):0));
    ImGui::SetNextWindowSizeConstraints(newSongMinSize,newSongMaxSize);
    if (ImGui::BeginPopupModal(_("New Song"),NULL,ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoScrollWithMouse|ImGuiWindowFlags_NoScrollbar)) {
      ImGui::SetWindowPos(ImVec2(((canvasW)-ImGui::GetWindowSize().x)*0.5,((canvasH)-ImGui::GetWindowSize().y)*0.5));
      if (ImGui::GetWindowSize().x<newSongMinSize.x || ImGui::GetWindowSize().y<newSongMinSize.y) {
        ImGui::SetWindowSize(newSongMinSize,ImGuiCond_Always);
      }
      drawNewSong();
      ImGui::EndPopup();
    }

    ImVec2 wsize=ImVec2(canvasW*0.9,canvasH*0.4);
    ImGui::SetNextWindowPos(ImVec2((canvasW-wsize.x)*0.5,50*dpiScale));
    ImGui::SetNextWindowSize(wsize,ImGuiCond_Always);
    if (ImGui::BeginPopup(_("Command Palette"),ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoSavedSettings)) {
      drawPalette();
      ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal(_("Export"),NULL,ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoScrollWithMouse|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::SetWindowPos(ImVec2(((canvasW)-ImGui::GetWindowSize().x)*0.5,((canvasH)-ImGui::GetWindowSize().y)*0.5));
      drawExport();
      ImGui::EndPopup();
    }

    centerNextWindow(_("Error"),canvasW,canvasH);
    if (ImGui::BeginPopupModal(_("Error"),NULL,ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::Text(_("%s"),errorString.c_str());
      if (ImGui::Button(_("OK"))) {
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }

    centerNextWindow(_("Warning"),canvasW,canvasH);
    if (ImGui::BeginPopupModal(_("Warning"),NULL,ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::Text("%s",warnString.c_str());
      switch (warnAction) {
        case GUI_WARN_QUIT:
          if (ImGui::Button(_("Yes"))) {
            ImGui::CloseCurrentPopup();
            if (curFileName=="" || curFileName.find(backupPath)==0 || e->song.version>=0xff00) {
              openFileDialog(GUI_FILE_SAVE);
              postWarnAction=GUI_WARN_QUIT;
            } else {
              if (save(curFileName,e->song.isDMF?e->song.version:0)>0) {
                showError(fmt::sprintf(_("Error while saving file! (%s)"),lastError));
              } else {
                quit=true;
              }
            }
          }
          ImGui::SameLine();
          if (ImGui::Button(_("No"))) {
            ImGui::CloseCurrentPopup();
            quit=true;
          }
          ImGui::SameLine();
          if (ImGui::Button(_("Cancel")) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            ImGui::CloseCurrentPopup();
          }
          break;
        case GUI_WARN_NEW:
          if (ImGui::Button(_("Yes"))) {
            ImGui::CloseCurrentPopup();
            if (curFileName=="" || curFileName.find(backupPath)==0 || e->song.version>=0xff00) {
              openFileDialog(GUI_FILE_SAVE);
              postWarnAction=GUI_WARN_NEW;
            } else {
              if (save(curFileName,e->song.isDMF?e->song.version:0)>0) {
                showError(fmt::sprintf(_("Error while saving file! (%s)"),lastError));
              } else {
                displayNew=true;
              }
            }
          }
          ImGui::SameLine();
          if (ImGui::Button(_("No"))) {
            ImGui::CloseCurrentPopup();
            displayNew=true;
          }
          ImGui::SameLine();
          if (ImGui::Button(_("Cancel")) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            ImGui::CloseCurrentPopup();
          }
          break;
        case GUI_WARN_OPEN:
          if (ImGui::Button(_("Yes"))) {
            ImGui::CloseCurrentPopup();
            if (curFileName=="" || curFileName.find(backupPath)==0 || e->song.version>=0xff00) {
              openFileDialog(GUI_FILE_SAVE);
              postWarnAction=GUI_WARN_OPEN;
            } else {
              if (save(curFileName,e->song.isDMF?e->song.version:0)>0) {
                showError(fmt::sprintf(_("Error while saving file! (%s)"),lastError));
              } else {
                openFileDialog(GUI_FILE_OPEN);
              }
            }
          }
          ImGui::SameLine();
          if (ImGui::Button(_("No"))) {
            ImGui::CloseCurrentPopup();
            openFileDialog(GUI_FILE_OPEN);
          }
          ImGui::SameLine();
          if (ImGui::Button(_("Cancel")) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            ImGui::CloseCurrentPopup();
          }
          break;
        case GUI_WARN_CV:
          if (ImGui::Button(_("Yes"))) {
            ImGui::CloseCurrentPopup();
            if (curFileName=="" || curFileName.find(backupPath)==0 || e->song.version>=0xff00) {
              openFileDialog(GUI_FILE_SAVE);
              postWarnAction=GUI_WARN_CV;
            } else {
              if (save(curFileName,e->song.isDMF?e->song.version:0)>0) {
                showError(fmt::sprintf(_("Error while saving file! (%s)"),lastError));
              } else {
                cvOpen=true;
              }
            }
          }
          ImGui::SameLine();
          if (ImGui::Button(_("No"))) {
            ImGui::CloseCurrentPopup();
            cvOpen=true;
          }
          ImGui::SameLine();
          if (ImGui::Button(_("Cancel")) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            ImGui::CloseCurrentPopup();
          }
          break;
        case GUI_WARN_OPEN_BACKUP:
          if (ImGui::Button(_("Yes"))) {
            ImGui::CloseCurrentPopup();
            if (curFileName=="" || curFileName.find(backupPath)==0 || e->song.version>=0xff00) {
              openFileDialog(GUI_FILE_SAVE);
              postWarnAction=GUI_WARN_OPEN_BACKUP;
            } else {
              if (save(curFileName,e->song.isDMF?e->song.version:0)>0) {
                showError(fmt::sprintf(_("Error while saving file! (%s)"),lastError));
              } else {
                openFileDialog(GUI_FILE_OPEN_BACKUP);
              }
            }
          }
          ImGui::SameLine();
          if (ImGui::Button(_("No"))) {
            ImGui::CloseCurrentPopup();
            openFileDialog(GUI_FILE_OPEN_BACKUP);
          }
          ImGui::SameLine();
          if (ImGui::Button(_("Cancel")) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            ImGui::CloseCurrentPopup();
          }
          break;
        case GUI_WARN_OPEN_DROP:
          if (ImGui::Button(_("Yes"))) {
            ImGui::CloseCurrentPopup();
            if (curFileName=="" || curFileName.find(backupPath)==0 || e->song.version>=0xff00) {
              openFileDialog(GUI_FILE_SAVE);
              postWarnAction=GUI_WARN_OPEN_DROP;
            } else {
              if (save(curFileName,e->song.isDMF?e->song.version:0)>0) {
                showError(fmt::sprintf(_("Error while saving file! (%s)"),lastError));
                nextFile="";
              } else {
                if (load(nextFile)>0) {
                  showError(fmt::sprintf(_("Error while loading file! (%s)"),lastError));
                }
                nextFile="";
              }
            }
          }
          ImGui::SameLine();
          if (ImGui::Button(_("No"))) {
            ImGui::CloseCurrentPopup();
            if (load(nextFile)>0) {
              showError(fmt::sprintf(_("Error while loading file! (%s)"),lastError));
            }
            nextFile="";
          }
          ImGui::SameLine();
          if (ImGui::Button(_("Cancel")) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            ImGui::CloseCurrentPopup();
            nextFile="";
          }
          break;
        case GUI_WARN_RESET_LAYOUT:
          if (ImGui::Button(_("Yes"))) {
            ImGui::CloseCurrentPopup();
            if (!mobileUI) {
              ImGui::LoadIniSettingsFromMemory(defaultLayout);
              if (!ImGui::SaveIniSettingsToDisk(finalLayoutPath,true)) {
                reportError(fmt::sprintf(_("could NOT save layout! %s"),strerror(errno)));
              }
            }
            settingsChanged=true;
          }
          ImGui::SameLine();
          if (ImGui::Button(_("No"))) {
            ImGui::CloseCurrentPopup();
          }
          break;
        case GUI_WARN_RESET_KEYBINDS:
          if (ImGui::Button(_("Yes"))) {
            ImGui::CloseCurrentPopup();
            resetKeybinds();
            settingsChanged=true;
          }
          ImGui::SameLine();
          if (ImGui::Button(_("No"))) {
            ImGui::CloseCurrentPopup();
          }
          break;
        case GUI_WARN_RESET_COLORS:
          if (ImGui::Button(_("Yes"))) {
            ImGui::CloseCurrentPopup();
            resetColors();
            applyUISettings(false);
            settingsChanged=true;
          }
          ImGui::SameLine();
          if (ImGui::Button(_("No"))) {
            ImGui::CloseCurrentPopup();
          }
          break;
        case GUI_WARN_CLOSE_SETTINGS:
          if (ImGui::Button(_("Yes"))) {
            ImGui::CloseCurrentPopup();
            settingsOpen=false;
            willCommit=true;
            settingsChanged=false;
          }
          ImGui::SameLine();
          if (ImGui::Button(_("No"))) {
            ImGui::CloseCurrentPopup();
            settingsOpen=false;
            syncSettings();
            settingsChanged=false;
          }
          ImGui::SameLine();
          if (ImGui::Button(_("Cancel")) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            ImGui::CloseCurrentPopup();
          }
          break;
        case GUI_WARN_CLEAR:
          if (ImGui::BeginTable("EraseOpt",2,ImGuiTableFlags_BordersInnerV)) {
            ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.5f);
            ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.5f);
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::PushFont(headFont);
            ImGui::AlignTextToFramePadding();
            ImGui::Text(_("Erasing"));
            ImGui::PopFont();

            if (ImGui::Button(_("All subsongs"))) {
              stop();
              e->clearSubSongs();
              curOrder=0;
              MARK_MODIFIED;
              ImGui::CloseCurrentPopup();
            }
            if (ImGui::Button(_("Current subsong"))) {
              stop();
              e->lockEngine([this]() {
                e->curSubSong->clearData();
              });
              e->setOrder(0);
              curOrder=0;
              MARK_MODIFIED;
              ImGui::CloseCurrentPopup();
            }
            if (ImGui::Button(_("Orders"))) {
              stop();
              e->lockEngine([this]() {
                memset(e->curOrders->ord,0,DIV_MAX_CHANS*DIV_MAX_PATTERNS);
                e->curSubSong->ordersLen=1;
              });
              e->setOrder(0);
              curOrder=0;
              MARK_MODIFIED;
              ImGui::CloseCurrentPopup();
            }
            if (ImGui::Button(_("Pattern"))) {
              stop();
              e->lockEngine([this]() {
                for (int i=0; i<e->getTotalChannelCount(); i++) {
                  DivPattern* pat=e->curPat[i].getPattern(e->curOrders->ord[i][curOrder],true);
                  memset(pat->data,-1,DIV_MAX_ROWS*DIV_MAX_COLS*sizeof(short));
                  for (int j=0; j<DIV_MAX_ROWS; j++) {
                    pat->data[j][0]=0;
                    pat->data[j][1]=0;
                  }
                }
              });
              MARK_MODIFIED;
              ImGui::CloseCurrentPopup();
            }
            if (ImGui::Button(_("Instruments"))) {
              stop();
              e->lockEngine([this]() {
                e->song.clearInstruments();
              });
              curIns=-1;
              MARK_MODIFIED;
              ImGui::CloseCurrentPopup();
            }
            if (ImGui::Button(_("Wavetables"))) {
              stop();
              e->lockEngine([this]() {
                e->song.clearWavetables();
              });
              curWave=0;
              MARK_MODIFIED;
              ImGui::CloseCurrentPopup();
            }
            if (ImGui::Button(_("Samples"))) {
              stop();
              e->lockEngine([this]() {
                e->song.clearSamples();
              });
              curSample=0;
              MARK_MODIFIED;
              ImGui::CloseCurrentPopup();
            }

            ImGui::TableNextColumn();
            ImGui::PushFont(headFont);
            ImGui::AlignTextToFramePadding();
            ImGui::Text(_("Optimization"));
            ImGui::PopFont();

            if (ImGui::Button(_("De-duplicate patterns"))) {
              stop();
              e->lockEngine([this]() {
                e->curSubSong->optimizePatterns();
                e->curSubSong->rearrangePatterns();
              });
              MARK_MODIFIED;
              ImGui::CloseCurrentPopup();
            }
            if (ImGui::Button(_("Remove unused instruments"))) {
              stop();
              e->delUnusedIns();
              MARK_MODIFIED;
              ImGui::CloseCurrentPopup();
            }
            /*
            if (ImGui::Button(_("Remove unused wavetables"))) {
              stop();
              e->delUnusedWaves();
              MARK_MODIFIED;
              ImGui::CloseCurrentPopup();
            }*/
            if (ImGui::Button(_("Remove unused samples"))) {
              stop();
              e->delUnusedSamples();
              MARK_MODIFIED;
              ImGui::CloseCurrentPopup();
            }

            ImGui::EndTable();
          }

          if (ImGui::BeginTable("EraseOptFooter",3)) {
            ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.5f);
            ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.5f);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TableNextColumn();
            if (ImGui::Button(_("Never mind! Cancel")) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
              ImGui::CloseCurrentPopup();
            }
            ImGui::TableNextColumn();
            ImGui::EndTable();
          }
          break;
        case GUI_WARN_SUBSONG_DEL:
          if (ImGui::Button(_("Yes"))) {
            if (e->removeSubSong(e->getCurrentSubSong())) {
              undoHist.clear();
              redoHist.clear();
              updateScroll(0);
              oldRow=0;
              cursor.xCoarse=0;
              cursor.xFine=0;
              cursor.y=0;
              selStart=cursor;
              selEnd=cursor;
              curOrder=0;
              MARK_MODIFIED;
            }
            ImGui::CloseCurrentPopup();
          }
          ImGui::SameLine();
          if (ImGui::Button(_("No"))) {
            ImGui::CloseCurrentPopup();
          }
          break;
        case GUI_WARN_SYSTEM_DEL:
          if (ImGui::Button(_("Yes"))) {
            e->removeSystem(sysToDelete,preserveChanPos);
            if (e->song.autoSystem) {
              autoDetectSystem();
              updateWindowTitle();
              MARK_MODIFIED;
            }
            updateROMExportAvail();
            ImGui::CloseCurrentPopup();
          }
          ImGui::SameLine();
          if (ImGui::Button(_("No"))) {
            ImGui::CloseCurrentPopup();
          }
          break;
        case GUI_WARN_CLEAR_HISTORY:
          if (ImGui::Button(_("Yes"))) {
            recentFile.clear();
            ImGui::CloseCurrentPopup();
          }
          ImGui::SameLine();
          if (ImGui::Button(_("No"))) {
            ImGui::CloseCurrentPopup();
          }
          break;
        case GUI_WARN_RESET_CONFIG:
          pushDestColor();
          if (ImGui::Button(_("Yes"))) {
            e->factoryReset();
            quit=true;
            ImGui::CloseCurrentPopup();
          }
          popDestColor();
          ImGui::SameLine();
          if (ImGui::Button(_("No"))) {
            ImGui::CloseCurrentPopup();
          }
          break;
        case GUI_WARN_IMPORT:
          if (ImGui::Button(_("Got it"))) {
            switch (e->song.version) {
              case DIV_VERSION_MOD:
                tutorial.importedMOD=true;
                break;
              case DIV_VERSION_S3M:
                tutorial.importedS3M=true;
                break;
              case DIV_VERSION_XM:
                tutorial.importedXM=true;
                break;
              case DIV_VERSION_IT:
                tutorial.importedIT=true;
                break;
            }
            commitTutorial();
            ImGui::CloseCurrentPopup();
          }
          break;
        case GUI_WARN_GENERIC:
          if (ImGui::Button(_("OK"))) {
            ImGui::CloseCurrentPopup();
          }
          break;
      }
      ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("InsTypeList",ImGuiWindowFlags_NoMove|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoSavedSettings)) {
      char temp[1024];
      if (displayInsTypeListMakeInsSample==-2) {
        ImGui::Text(_("Drum kit mode:"));
        if (ImGui::RadioButton(_("Normal"),!makeDrumkitMode)) {
          makeDrumkitMode=false;
        }
        if (ImGui::RadioButton(_("12 samples per octave"),makeDrumkitMode)) {
          makeDrumkitMode=true;
        }

        if (!makeDrumkitMode) {
          ImGui::Text(_("Starting octave"));
          ImGui::SameLine();
          if (ImGui::InputInt("##DKOctave",&makeDrumkitOctave,1,3)) {
            if (makeDrumkitOctave<0) makeDrumkitOctave=0;
            if (makeDrumkitOctave>9) makeDrumkitOctave=9;
          }
        }

        ImGui::Separator();
      }
      for (DivInstrumentType& i: makeInsTypeList) {
        strncpy(temp,insTypes[i][0],1023);
        if (ImGui::MenuItem(temp)) {
          // create ins
          curIns=e->addInstrument(-1,i);
          if (curIns==-1) {
            showError(_("too many instruments!"));
          } else {
            if (displayInsTypeListMakeInsSample==-2) {
              e->song.ins[curIns]->type=i;
              e->song.ins[curIns]->name=_("Drum Kit");
              e->song.ins[curIns]->amiga.useNoteMap=true;
              if (i!=DIV_INS_AMIGA) e->song.ins[curIns]->amiga.useSample=true;

              if (makeDrumkitMode) {
                for (int j=0; j<120; j++) {
                  e->song.ins[curIns]->amiga.noteMap[j].freq=48;
                  e->song.ins[curIns]->amiga.noteMap[j].dpcmFreq=15;
                  e->song.ins[curIns]->amiga.noteMap[j].map=j%12;
                  if ((j%12)>=e->song.sampleLen) continue;
                }
              } else {
                int index=-makeDrumkitOctave*12;
                for (int j=0; j<120; j++) {
                  e->song.ins[curIns]->amiga.noteMap[j].freq=48;
                  e->song.ins[curIns]->amiga.noteMap[j].dpcmFreq=15;
                  if (index<0 || index>=e->song.sampleLen) {
                    index++;
                    continue;
                  }
                  e->song.ins[curIns]->amiga.noteMap[j].map=index++;
                }
              }

              nextWindow=GUI_WINDOW_INS_EDIT;
              wavePreviewInit=true;
              updateFMPreview=true;
            } else if (displayInsTypeListMakeInsSample>=0 && displayInsTypeListMakeInsSample<(int)e->song.sample.size()) {
              e->song.ins[curIns]->type=i;
              e->song.ins[curIns]->name=e->song.sample[displayInsTypeListMakeInsSample]->name;
              e->song.ins[curIns]->amiga.initSample=displayInsTypeListMakeInsSample;
              if (i!=DIV_INS_AMIGA) e->song.ins[curIns]->amiga.useSample=true;
              nextWindow=GUI_WINDOW_INS_EDIT;
              wavePreviewInit=true;
              updateFMPreview=true;
            }

            if (settings.blankIns) {
              e->song.ins[curIns]->fm.fb=0;
              for (int i=0; i<4; i++) {
                e->song.ins[curIns]->fm.op[i]=DivInstrumentFM::Operator();
                e->song.ins[curIns]->fm.op[i].ar=31;
                e->song.ins[curIns]->fm.op[i].dr=31;
                e->song.ins[curIns]->fm.op[i].rr=15;
                e->song.ins[curIns]->fm.op[i].tl=127;
                e->song.ins[curIns]->fm.op[i].dt=3;

            e->song.ins[curIns]->esfm.op[i].ct=0;
            e->song.ins[curIns]->esfm.op[i].dt=0;
            e->song.ins[curIns]->esfm.op[i].modIn=0;
            e->song.ins[curIns]->esfm.op[i].outLvl=0;
              }
            }

            MARK_MODIFIED;
          }
        }
      }
      ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("WaveSizeList",ImGuiWindowFlags_NoMove|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoSavedSettings)) {
      char temp[1024];
      for (FurnaceGUIWaveSizeEntry i: waveSizeList) {
        snprintf(temp,1023,"%d×%d (%s)",i.width,i.height,i.sys);
        if (ImGui::MenuItem(temp)) {
          // create wave
          curWave=e->addWave();
          if (curWave==-1) {
            showError(_("too many wavetables!"));
          } else {
            e->song.wave[curWave]->len=i.width;
            e->song.wave[curWave]->max=i.height-1;
            for (int j=0; j<i.width; j++) {
              e->song.wave[curWave]->data[j]=(j*i.height)/i.width;
            }
            MARK_MODIFIED;
            RESET_WAVE_MACRO_ZOOM;
          }
        }
      }
      ImGui::EndPopup();
    }

    // TODO:
    // - multiple selection
    // - replace instrument
    centerNextWindow(_("Select Instrument"),canvasW,canvasH);
    if (ImGui::BeginPopupModal(_("Select Instrument"),NULL,ImGuiWindowFlags_AlwaysAutoResize)) {
      bool quitPlease=false;
      if (pendingInsSingle) {
        ImGui::Text(_("this is an instrument bank! select which one to use:"));
      } else {
        ImGui::AlignTextToFramePadding();
        ImGui::Text(_("this is an instrument bank! select which ones to load:"));
        ImGui::SameLine();
        if (ImGui::Button(_("All"))) {
          for (std::pair<DivInstrument*,bool>& i: pendingIns) {
            i.second=true;
          }
        }
        ImGui::SameLine();
        if (ImGui::Button(_("None"))) {
          for (std::pair<DivInstrument*,bool>& i: pendingIns) {
            i.second=false;
          }
        }
      }
      bool anySelected=false;
      float sizeY=ImGui::GetFrameHeightWithSpacing()*pendingIns.size();
      if (sizeY>(canvasH-180.0*dpiScale)) {
        sizeY=canvasH-180.0*dpiScale;
        if (sizeY<60.0*dpiScale) sizeY=60.0*dpiScale;
      }
      if (ImGui::BeginTable("PendingInsList",1,ImGuiTableFlags_ScrollY,ImVec2(0.0f,sizeY))) {
        for (size_t i=0; i<pendingIns.size(); i++) {
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          String id=fmt::sprintf("%d: %s",(int)i,pendingIns[i].first->name);
          if (pendingInsSingle) {
            if (ImGui::Selectable(id.c_str())) {
              pendingIns[i].second=true;
              quitPlease=true;
            }
          } else {
            ImGui::Checkbox(id.c_str(),&pendingIns[i].second);
          }
          if (pendingIns[i].second) anySelected=true;
        }
        ImGui::EndTable();
      }
      if (!pendingInsSingle) {
        ImGui::BeginDisabled(!anySelected);
        if (ImGui::Button(_("OK"))) {
          quitPlease=true;
        }
        ImGui::EndDisabled();
        ImGui::SameLine();
      }
      if (ImGui::Button(_("Cancel")) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        for (std::pair<DivInstrument*,bool>& i: pendingIns) {
          i.second=false;
        }
        quitPlease=true;
      }
      if (quitPlease) {
        ImGui::CloseCurrentPopup();
        for (std::pair<DivInstrument*,bool>& i: pendingIns) {
          if (!i.second || pendingInsSingle) {
            if (i.second) {
              if (curIns>=0 && curIns<(int)e->song.ins.size()) {
                *e->song.ins[curIns]=*i.first;
              } else {
                showError(_("...but you haven't selected an instrument!"));
              }
            }
            delete i.first;
          } else {
            e->addInstrumentPtr(i.first);
          }
        }
        pendingIns.clear();
      }
      ImGui::EndPopup();
    }

    // TODO: fix style
    centerNextWindow(_("Select Sample"),canvasW,canvasH);
    if (ImGui::BeginPopupModal(_("Select Sample"),NULL,ImGuiWindowFlags_AlwaysAutoResize)) {
      bool quitPlease=false;

      ImGui::AlignTextToFramePadding();
      ImGui::Text(_("this is a sample bank! select which ones to load:"));
      ImGui::SameLine();
      if (ImGui::Button(_("All"))) {
        for (std::pair<DivSample*,bool>& i: pendingSamples) {
          i.second=true;
        }
      }
      ImGui::SameLine();
      if (ImGui::Button(_("None"))) {
        for (std::pair<DivSample*,bool>& i: pendingSamples) {
          i.second=false;
        }
      }
      bool reissueSearch=false;

      bool anySelected=false;
      float sizeY=ImGui::GetFrameHeightWithSpacing()*pendingSamples.size();
      if (sizeY>(canvasH-180.0*dpiScale)) 
      {
        sizeY=canvasH-180.0*dpiScale;
        if (sizeY<60.0*dpiScale) sizeY=60.0*dpiScale;
      }
      if (ImGui::BeginTable("PendingSamplesList",1,ImGuiTableFlags_ScrollY,ImVec2(0.0f,sizeY))) 
      {
        if (sampleBankSearchQuery.empty())
        {
          for (size_t i=0; i<pendingSamples.size(); i++) 
          {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            String id=fmt::sprintf("%d: %s",(int)i,pendingSamples[i].first->name);
            if (pendingInsSingle) 
            {
              if (ImGui::Selectable(id.c_str())) 
              {
                pendingSamples[i].second=true;
                quitPlease=true;
              }
            } 
            else 
            {
              // TODO:fixstyle from hereonwards
              ImGuiIO& io = ImGui::GetIO();
              if(ImGui::Checkbox(id.c_str(),&pendingSamples[i].second) && io.KeyShift)
              {
                for(int jj = (int)i - 1; jj >= 0; jj--)
                {
                  if(pendingSamples[jj].second) //pressed shift and there's selected item above
                  {
                    for(int k = jj; k < (int)i; k++)
                    {
                      pendingSamples[k].second = true;
                    }

                    break;
                  }
                }
              }
            }
            if (pendingSamples[i].second) anySelected=true;
          }
        }
        else //display search results
        {
          if(reissueSearch)
          {
            String lowerCase=sampleBankSearchQuery;

            for (char& ii: lowerCase) 
            {
              if (ii>='A' && ii<='Z') ii+='a'-'A';
            }

            sampleBankSearchResults.clear();
            for (int j=0; j < (int)pendingSamples.size(); j++) 
            {
              String lowerCase1 = pendingSamples[j].first->name;

              for (char& ii: lowerCase1) 
              {
                if (ii>='A' && ii<='Z') ii+='a'-'A';
              }

              if (lowerCase1.find(lowerCase)!=String::npos) 
              {
                sampleBankSearchResults.push_back(std::make_pair(pendingSamples[j].first, pendingSamples[j].second));
              }
            }
          }

          for (size_t i=0; i<sampleBankSearchResults.size(); i++)
          {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            String id=fmt::sprintf("%d: %s",(int)i,sampleBankSearchResults[i].first->name);

            ImGuiIO& io = ImGui::GetIO();
            if(ImGui::Checkbox(id.c_str(),&sampleBankSearchResults[i].second) && io.KeyShift)
            {
              for(int jj = (int)i - 1; jj >= 0; jj--)
              {
                if(sampleBankSearchResults[jj].second) //pressed shift and there's selected item above
                {
                  for(int k = jj; k < (int)i; k++)
                  {
                    sampleBankSearchResults[k].second = true;
                  }

                  break;
                }
              }
            }
            if (sampleBankSearchResults[i].second) anySelected=true;
          }

          for (size_t i=0; i<pendingSamples.size(); i++)
          {
            if(sampleBankSearchResults.size() > 0)
            {
              for (size_t j=0; j<sampleBankSearchResults.size(); j++)
              {
                if(sampleBankSearchResults[j].first == pendingSamples[i].first && sampleBankSearchResults[j].second && pendingSamples[i].first != NULL)
                {
                  pendingSamples[i].second = true;
                  if (pendingSamples[i].second) anySelected=true;
                  break;
                }
              }
            }
          }
        }
        ImGui::EndTable();
      }

      ImGui::BeginDisabled(!anySelected);
      if (ImGui::Button(_("OK"))) {
        quitPlease=true;
      }
      ImGui::EndDisabled();
      ImGui::SameLine();

      if (ImGui::Button(_("Cancel")) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        for (std::pair<DivSample*,bool>& i: pendingSamples) {
          i.second=false;
        }
        quitPlease=true;
      }
      if (quitPlease) 
      {
        ImGui::CloseCurrentPopup();
        int counter = 0;
        for (std::pair<DivSample*,bool>& i: pendingSamples) 
        {
          if (!i.second)
          {
            delete i.first;
          }
          else
          {
            if(counter == 0 && replacePendingSample)
            {
              *e->song.sample[curSample]=*i.first;
              replacePendingSample = false;
            }
            else
            {
              e->addSamplePtr(i.first);
            }
          }
          counter++;
        }

        curSample = (int)e->song.sample.size() - 1;
        pendingSamples.clear();
      }

      ImGui::EndPopup();
    }

    centerNextWindow(_("Import Raw Sample"),canvasW,canvasH);
    if (ImGui::BeginPopupModal(_("Import Raw Sample"),NULL,ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::Text(_("Data type:"));
      for (int i=0; i<DIV_SAMPLE_DEPTH_MAX; i++) {
        if (sampleDepths[i]==NULL) continue;
       if (ImGui::RadioButton(sampleDepths[i],pendingRawSampleDepth==i)) pendingRawSampleDepth=i;
      }

      if (pendingRawSampleDepth!=DIV_SAMPLE_DEPTH_8BIT && pendingRawSampleDepth!=DIV_SAMPLE_DEPTH_16BIT) {
        pendingRawSampleChannels=1;
        pendingRawSampleBigEndian=false;
      }

      ImGui::AlignTextToFramePadding();
      ImGui::Text(_("Sample rate"));
      ImGui::SameLine();
      ImGui::SetNextItemWidth(120.0f*dpiScale);
      if (ImGui::InputInt("##RSRate",&pendingRawSampleRate,100,1000)) {
        if (pendingRawSampleRate<100) pendingRawSampleRate=100;
        if (pendingRawSampleRate>384000) pendingRawSampleRate=384000;
      }

      if (pendingRawSampleDepth==DIV_SAMPLE_DEPTH_8BIT || pendingRawSampleDepth==DIV_SAMPLE_DEPTH_16BIT) {
        ImGui::AlignTextToFramePadding();
        ImGui::Text(_("Channels"));
        ImGui::SameLine();
        ImGui::SetNextItemWidth(120.0f*dpiScale);
        if (ImGui::InputInt("##RSChans",&pendingRawSampleChannels,1,2)) {
        }
        ImGui::Text(_("(will be mixed down to mono)"));
        ImGui::Checkbox(_("Unsigned"),&pendingRawSampleUnsigned);
      }

      if (pendingRawSampleDepth==DIV_SAMPLE_DEPTH_16BIT) {
        ImGui::Checkbox(_("Big endian"),&pendingRawSampleBigEndian);
      }

      if (pendingRawSampleDepth==DIV_SAMPLE_DEPTH_YMZ_ADPCM ||
          pendingRawSampleDepth==DIV_SAMPLE_DEPTH_QSOUND_ADPCM ||
          pendingRawSampleDepth==DIV_SAMPLE_DEPTH_ADPCM_A ||
          pendingRawSampleDepth==DIV_SAMPLE_DEPTH_ADPCM_B ||
          pendingRawSampleDepth==DIV_SAMPLE_DEPTH_VOX) {
        ImGui::Checkbox(_("Swap nibbles"),&pendingRawSampleSwapNibbles);
      }

      if (pendingRawSampleDepth==DIV_SAMPLE_DEPTH_8BIT) {
        ImGui::Checkbox(_("Swap words"),&pendingRawSampleBigEndian);
      }

      if (pendingRawSampleDepth==DIV_SAMPLE_DEPTH_MULAW) {
        ImGui::Text(_("Encoding:"));
        ImGui::Indent();
        if (ImGui::RadioButton("G.711",pendingRawSampleSwapNibbles==0)) {
          pendingRawSampleSwapNibbles=0;
        }
        if (ImGui::RadioButton("Namco",pendingRawSampleSwapNibbles==1)) {
          pendingRawSampleSwapNibbles=1;
        }
        ImGui::Unindent();
      }

      if (pendingRawSampleDepth==DIV_SAMPLE_DEPTH_1BIT ||
          pendingRawSampleDepth==DIV_SAMPLE_DEPTH_1BIT_DPCM) {
        ImGui::Checkbox(_("Reverse bit order"),&pendingRawSampleSwapNibbles);
      }

      if (ImGui::Button(_("OK"))) {
        DivSample* s=e->sampleFromFileRaw(pendingRawSample.c_str(),(DivSampleDepth)pendingRawSampleDepth,pendingRawSampleChannels,pendingRawSampleBigEndian,pendingRawSampleUnsigned,pendingRawSampleSwapNibbles,pendingRawSampleRate);
        if (s==NULL) {
          showError(e->getLastError());
        } else {
          if (pendingRawSampleReplace) {
            if (curSample>=0 && curSample<(int)e->song.sample.size()) {
              e->lockEngine([this,s]() {
                // if it crashes here please tell me...
                DivSample* oldSample=e->song.sample[curSample];
                e->song.sample[curSample]=s;
                delete oldSample;
                e->renderSamples();
                MARK_MODIFIED;
              });
              updateSampleTex=true;
            } else {
              showError(_("...but you haven't selected a sample!"));
              delete s;
            }
          } else {
            if (e->addSamplePtr(s)==-1) {
              showError(e->getLastError());
            } else {
              MARK_MODIFIED;
            }
          }
        }
        ImGui::CloseCurrentPopup();
      }
      ImGui::SameLine();
      if (ImGui::Button(_("Cancel")) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("EditString",ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoSavedSettings)) {
      if (editString==NULL) {
        ImGui::Text(_("Error! No string provided!"));
      } else {
        if (displayEditString) {
          ImGui::SetItemDefaultFocus();
          ImGui::SetKeyboardFocusHere();
        }
        ImGui::InputText("##StringVal",editString);
      }
      displayEditString=false;
      ImGui::SameLine();
      if (ImGui::Button(_("OK")) || ImGui::IsKeyPressed(ImGuiKey_Enter,false)) {
        editString=NULL;
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    } else {
      editString=NULL;
    }

    MEASURE_END(popup);

    if ((!tutorial.introPlayed || settings.alwaysPlayIntro!=0) && renderBackend!=GUI_BACKEND_SOFTWARE) {
      MEASURE_BEGIN(intro);
      initialScreenWipe=0;
      if (settings.alwaysPlayIntro==1) {
        shortIntro=true;
      }
      drawIntro(introPos);
      MEASURE_END(intro);
    } else {
      introPos=12.0;
    }

#ifdef DIV_UNSTABLE
    {
      ImDrawList* dl=ImGui::GetForegroundDrawList();
      ImVec2 markPos=ImVec2(canvasW-ImGui::CalcTextSize(DIV_VERSION).x-6.0*dpiScale,4.0*dpiScale);
      ImVec4 markColor=uiColors[GUI_COLOR_TEXT];
      markColor.w=0.67f;

      dl->AddText(markPos,ImGui::ColorConvertFloat4ToU32(markColor),DIV_VERSION);
    }
#endif

    if (settings.displayRenderTime) {
      String renderTime=fmt::sprintf("%.0fµs",ImGui::GetIO().DeltaTime*1000000.0);
      String renderTime2=fmt::sprintf("%.1f FPS",1.0/ImGui::GetIO().DeltaTime);
      ImDrawList* dl=ImGui::GetForegroundDrawList();
      ImVec2 markPos=ImVec2(canvasW-ImGui::CalcTextSize(renderTime.c_str()).x-60.0*dpiScale,4.0*dpiScale);
      ImVec2 markPos2=ImVec2(canvasW-ImGui::CalcTextSize(renderTime2.c_str()).x-160.0*dpiScale,4.0*dpiScale);

      dl->AddText(markPos,0xffffffff,renderTime.c_str());
      dl->AddText(markPos2,0xffffffff,renderTime2.c_str());

      //logV("%s (%s)",renderTime,renderTime2);
    }

    layoutTimeEnd=SDL_GetPerformanceCounter();

    // backup trigger
    if (modified && settings.backupEnable) {
      if (backupTimer>0) {
        backupTimer=(backupTimer-ImGui::GetIO().DeltaTime);
        if (backupTimer<=0) {
          backupTask=std::async(std::launch::async,[this]() -> bool {
            backupLock.lock();
            logV("backupPath: %s",backupPath);
            logV("curFileName: %s",curFileName);
            if (curFileName.find(backupPath)==0) {
              logD("backup file open. not saving backup.");
              backupTimer=settings.backupInterval;
              backupLock.unlock();
              return true;
            }
            if (!dirExists(backupPath.c_str())) {
              if (!makeDir(backupPath.c_str())) {
                logW("could not create backup directory!");
                backupTimer=settings.backupInterval;
                backupLock.unlock();
                return false;
              }
            }
            logD("saving backup...");
            SafeWriter* w=e->saveFur(true,true);
            logV("writing file...");

            if (w!=NULL) {
              size_t sepPos=curFileName.rfind(DIR_SEPARATOR);
              String backupPreBaseName;
              String backupBaseName;
              String backupFileName;
              if (sepPos==String::npos) {
                backupPreBaseName=curFileName;
              } else {
                backupPreBaseName=curFileName.substr(sepPos+1);
              }

              size_t dotPos=backupPreBaseName.rfind('.');
              if (dotPos!=String::npos) {
                backupPreBaseName=backupPreBaseName.substr(0,dotPos);
              }

              for (char i: backupPreBaseName) {
                if (backupBaseName.size()>=48) break;
                if ((i>='0' && i<='9') || (i>='A' && i<='Z') || (i>='a' && i<='z') || i=='_' || i=='-' || i==' ') backupBaseName+=i;
              }

              if (backupBaseName.empty()) backupBaseName="untitled";

              backupFileName=backupBaseName;

              time_t curTime=time(NULL);
              struct tm curTM;
#ifdef _WIN32
              struct tm* tempTM=localtime(&curTime);
              if (tempTM==NULL) {
                backupFileName+="-unknownTime.fur";
              } else {
                curTM=*tempTM;
                backupFileName+=fmt::sprintf("-%d%.2d%.2d-%.2d%.2d%.2d.fur",curTM.tm_year+1900,curTM.tm_mon+1,curTM.tm_mday,curTM.tm_hour,curTM.tm_min,curTM.tm_sec);
              }
#else
              if (localtime_r(&curTime,&curTM)==NULL) {
                backupFileName+="-unknownTime.fur";
              } else {
                backupFileName+=fmt::sprintf("-%d%.2d%.2d-%.2d%.2d%.2d.fur",curTM.tm_year+1900,curTM.tm_mon+1,curTM.tm_mday,curTM.tm_hour,curTM.tm_min,curTM.tm_sec);
              }
#endif

              String finalPath=backupPath+String(DIR_SEPARATOR_STR)+backupFileName;
              
              FILE* outFile=ps_fopen(finalPath.c_str(),"wb");
              if (outFile!=NULL) {
                if (fwrite(w->getFinalBuf(),1,w->size(),outFile)!=w->size()) {
                  logW("did not write backup entirely: %s!",strerror(errno));
                }
                fclose(outFile);
              } else {
                logW("could not save backup: %s!",strerror(errno));
              }
              w->finish();

              // delete previous backup if there are too many
              delFirstBackup(backupBaseName);
            }
            logD("backup saved.");
            backupTimer=settings.backupInterval;
            backupLock.unlock();
            return true;
          });
        }
      }
    }

    sampleMapWaitingInput=(curWindow==GUI_WINDOW_INS_EDIT && sampleMapFocused);
    
    curWindowThreadSafe=curWindow;

    if (curWindow!=curWindowLast) {
      int curWindowCat=0;
      int lastWindowCat=0;

      switch (curWindow) {
        case GUI_WINDOW_WAVE_LIST:
        case GUI_WINDOW_WAVE_EDIT:
          curWindowCat=1;
          break;
        case GUI_WINDOW_SAMPLE_LIST:
        case GUI_WINDOW_SAMPLE_EDIT:
          curWindowCat=2;
          break;
        default:
          curWindowCat=0;
          break;
      }
      switch (curWindowLast) {
        case GUI_WINDOW_WAVE_LIST:
        case GUI_WINDOW_WAVE_EDIT:
          lastWindowCat=1;
          break;
        case GUI_WINDOW_SAMPLE_LIST:
        case GUI_WINDOW_SAMPLE_EDIT:
          lastWindowCat=2;
          break;
        default:
          lastWindowCat=0;
          break;
      }

      if (curWindowCat!=lastWindowCat) {
        switch (lastWindowCat) {
          case 0:
            e->autoNoteOffAll();
            failedNoteOn=false;
            break;
          case 1:
            e->stopWavePreview();
            break;
          case 2:
            e->stopSamplePreview();
            break;
        }
      }
    }

    if (!settings.renderClearPos || renderBackend==GUI_BACKEND_METAL) {
      rend->clear(uiColors[GUI_COLOR_BACKGROUND]);
    }
    renderTimeBegin=SDL_GetPerformanceCounter();
    ImGui::Render();
    renderTimeEnd=SDL_GetPerformanceCounter();
    drawTimeBegin=SDL_GetPerformanceCounter();
    rend->renderGUI();
    if (mustClear) {
      rend->clear(ImVec4(0,0,0,0));
      mustClear--;
      if (mustClear==0) e->everythingOK();
    } else {
      if (initialScreenWipe>0.0f && !settings.disableFadeIn) {
        WAKE_UP;
        initialScreenWipe-=ImGui::GetIO().DeltaTime*5.0f;
        if (initialScreenWipe>0.0f) {
          rend->wipe(pow(initialScreenWipe,2.0f));
        }
      } else if (settings.disableFadeIn) {
        initialScreenWipe=0.0f;
      }
    }
    drawTimeEnd=SDL_GetPerformanceCounter();
    swapTimeBegin=SDL_GetPerformanceCounter();
    if (!settings.vsync || !rend->canVSync()) {
      if (settings.frameRateLimit>0) {
        unsigned int presentDelay=SDL_GetPerformanceFrequency()/settings.frameRateLimit;
        if ((nextPresentTime-swapTimeBegin)<presentDelay) {
#ifdef _WIN32
          unsigned int mDivider=SDL_GetPerformanceFrequency()/1000;
          Sleep((unsigned int)(nextPresentTime-swapTimeBegin)/mDivider);
#else
          unsigned int mDivider=SDL_GetPerformanceFrequency()/1000000;
          usleep((unsigned int)(nextPresentTime-swapTimeBegin)/mDivider);
#endif
          nextPresentTime+=presentDelay;
        } else {
          nextPresentTime=swapTimeBegin+presentDelay;
        }
      }
    }
    rend->present();
    if (settings.renderClearPos && renderBackend!=GUI_BACKEND_METAL) {
      rend->clear(uiColors[GUI_COLOR_BACKGROUND]);
    }
    swapTimeEnd=SDL_GetPerformanceCounter();

    layoutTimeDelta=layoutTimeEnd-layoutTimeBegin;
    renderTimeDelta=renderTimeEnd-renderTimeBegin;
    drawTimeDelta=drawTimeEnd-drawTimeBegin;
    swapTimeDelta=swapTimeEnd-swapTimeBegin;
    eventTimeDelta=eventTimeEnd-eventTimeBegin;

    soloTimeout-=ImGui::GetIO().DeltaTime;
    if (soloTimeout<0) {
      soloTimeout=0;
    } else {
      WAKE_UP;
    }

    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
      exitDisabledTimer=0;
    }

    wheelX=0;
    wheelY=0;
    wantScrollListIns=false;
    wantScrollListWave=false;
    wantScrollListSample=false;

    pressedPoints.clear();
    releasedPoints.clear();

    if (willCommit) {
      commitSettings();
      willCommit=false;
    }

    // To check for instrument editor modification, we need an up-to-date `insEditMayBeDirty`
    // (based on incoming user input events), and we want any possible instrument modifications
    // to already have been made.
    checkRecordInstrumentUndoStep();

    if (shallDetectScale) {
      if (--shallDetectScale<1) {
        if (settings.dpiScale<0.5f) {
          const char* videoBackend=SDL_GetCurrentVideoDriver();      
          double newScale=getScaleFactor(videoBackend,sdlWin);
          if (newScale<0.1f) {
            logW("scale what?");
            newScale=1.0f;
          }

          if (newScale!=dpiScale) {
            logD("auto UI scale changed (%f != %f) - applying settings...",newScale,dpiScale);
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
          }
        }
      }
    }

    if (fontsFailed) {
      showError(_("it appears I couldn't load these fonts. any setting you can check?"));
      logE("couldn't load fonts");
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
    }

    if (!editOptsVisible) {
      latchTarget=0;
      latchNibble=false;
    }

    if (SDL_GetWindowFlags(sdlWin)&SDL_WINDOW_MINIMIZED) {
      SDL_Delay(100);
    }
  }
  return false;
}

bool FurnaceGUI::init() {
  logI("initializing GUI.");

  syncState();
  syncSettings();
  syncTutorial();

  recentFile.clear();
  for (int i=0; i<settings.maxRecentFile; i++) {
    String r=e->getConfString(fmt::sprintf("recentFile%d",i),"");
    if (!r.empty()) {
      recentFile.push_back(r);
    }
  }

  if (!settings.persistFadeOut) {
    audioExportOptions.loops=settings.exportLoops;
    audioExportOptions.fadeOut=settings.exportFadeOut;
  }

  initSystemPresets();

  e->setAutoNotePoly(noteInputPoly);

  SDL_SetHint(SDL_HINT_VIDEO_ALLOW_SCREENSAVER,"1");
#if SDL_VERSION_ATLEAST(2,0,17)
  SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS,"0");
#endif
  SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS,"0");
  // don't disable compositing on KWin
#if SDL_VERSION_ATLEAST(2,0,22)
  logV("setting window type to NORMAL.");
  SDL_SetHint(SDL_HINT_X11_WINDOW_TYPE,"_NET_WM_WINDOW_TYPE_NORMAL");
#endif

  // This sets the icon in wayland
  SDL_setenv("SDL_VIDEO_WAYLAND_WMCLASS", FURNACE_APP_ID, 0);

  // initialize SDL
  logD("initializing video...");
  if (SDL_Init(SDL_INIT_VIDEO)!=0) {
    logE("could not initialize video! %s",SDL_GetError());
    return false;
  }

#ifdef IS_MOBILE
  logD("initializing haptic...");
  if (SDL_Init(SDL_INIT_HAPTIC)!=0) {
    logW("could not initialize haptic! %s",SDL_GetError());
  }
#endif

  const char* videoBackend=SDL_GetCurrentVideoDriver();
  if (videoBackend!=NULL) {
    logV("video backend: %s",videoBackend);
    if (strcmp(videoBackend,"wayland")==0 ||
        strcmp(videoBackend,"cocoa")==0 ||
        strcmp(videoBackend,"uikit")==0) {
      sysManagedScale=true;
      logV("scaling managed by system.");
    } else {
      logV("scaling managed by application.");
    }
  } else {
    logV("could not get video backend name!");
  }

  // get scale factor
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

#if !(defined(__APPLE__) || defined(_WIN32))
  // get the icon (on macOS and Windows the icon is bundled with the app)
  const FurnaceGUIImage* furIcon=getImage(GUI_IMAGE_ICON);
  SDL_Surface* icon=NULL;
  if (furIcon!=NULL) {
    icon=SDL_CreateRGBSurfaceFrom(furIcon->data,furIcon->width,furIcon->height,32,256*4,0xff,0xff00,0xff0000,0xff000000);
  } else {
    logE("furIcon is NULL!");
  }
#endif

#ifdef IS_MOBILE
  scrW=960;
  scrH=540;
  scrX=0;
  scrY=0;
#else
  scrW=scrConfW=e->getConfInt("lastWindowWidth",GUI_WIDTH_DEFAULT);
  scrH=scrConfH=e->getConfInt("lastWindowHeight",GUI_HEIGHT_DEFAULT);
  scrX=scrConfX=e->getConfInt("lastWindowX",SDL_WINDOWPOS_CENTERED);
  scrY=scrConfY=e->getConfInt("lastWindowY",SDL_WINDOWPOS_CENTERED);
  scrMax=e->getConfBool("lastWindowMax",false);
#endif
  portrait=(scrW<scrH);
  logV("portrait: %d (%dx%d)",portrait,scrW,scrH);

  // if old config, scale size as it was stored unscaled before
  if (e->getConfInt("configVersion",0)<122 && !sysManagedScale) {
    logD("scaling window size to scale factor because configVersion is not present.");
    scrW*=dpiScale;
    scrH*=dpiScale;
  }

  // predict the canvas size
  if (sysManagedScale) {
    canvasW=scrW*dpiScale;
    canvasH=scrH*dpiScale;
  } else {
    canvasW=scrW;
    canvasH=scrH;
  }

#ifndef IS_MOBILE
  SDL_Rect displaySize;
#endif

#ifndef IS_MOBILE
  // if window would spawn out of bounds, force it to be get default position
  SDL_Rect bounds;
  if (!detectOutOfBoundsWindow(bounds)) {
    scrMax=false;
    scrX=scrConfX=SDL_WINDOWPOS_CENTERED;
    scrY=scrConfY=SDL_WINDOWPOS_CENTERED;

    // make sure our window isn't big
    /*if (bounds.w<scrW) {
      logD("resizing width because it does not fit");
      scrW=bounds.w-OOB_PIXELS_SAFETY*2;
      if (scrW<200) scrW=200;
    }
    if (bounds.h<scrH) {
      logD("resizing height because it does not fit");
      scrH=bounds.h-OOB_PIXELS_SAFETY*2;
      if (scrH<100) scrH=100;
    }*/
  }
#endif

  logV("window size: %dx%d",scrW,scrH);

  if (!initRender()) {
    if (settings.renderBackend!="Software") {
      settings.renderBackend="Software";
      e->setConf("renderBackend","Software");
      e->saveConf();
      lastError=fmt::sprintf(_("could not init renderer!\nfalling back to software renderer. please restart Furnace."));
    } else if (settings.renderBackend=="SDL") {
      lastError=fmt::sprintf(_("could not init renderer! %s\nfalling back to software renderer. please restart Furnace."),SDL_GetError());
      settings.renderBackend="Software";
      e->setConf("renderBackend","Software");
      e->saveConf();
    } else {
      lastError=fmt::sprintf(_("could not init renderer!"));
    }
    return false;
  }

  rend->preInit(e->getConfObject());

  logD("creating window...");
  sdlWin=SDL_CreateWindow("Furnace",scrX,scrY,scrW,scrH,SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALLOW_HIGHDPI|(scrMax?SDL_WINDOW_MAXIMIZED:0)|(fullScreen?SDL_WINDOW_FULLSCREEN_DESKTOP:0)|rend->getWindowFlags());
  if (sdlWin==NULL) {
    const char* sdlErr=SDL_GetError();
    lastError=fmt::sprintf(_("could not open window! %s"),sdlErr);
    if (settings.renderBackend!="Software" && strstr(sdlErr,"matching")!=NULL) {
      settings.renderBackend="Software";
      e->setConf("renderBackend","Software");
      e->saveConf();
      lastError+=_("\nfalling back to software renderer. please restart Furnace.");
    }
    return false;
  }

#ifndef IS_MOBILE
  if (SDL_GetDisplayUsableBounds(SDL_GetWindowDisplayIndex(sdlWin),&displaySize)==0) {
    bool mustChange=false;
    if (scrW>((displaySize.w)-48) && scrH>((displaySize.h)-64)) {
      // maximize
      SDL_MaximizeWindow(sdlWin);
      logD("maximizing as it doesn't fit (%dx%d+%d+%d).",displaySize.w,displaySize.h,displaySize.x,displaySize.y);
    }
    if (scrW>displaySize.w) {
      scrW=(displaySize.w)-32;
      mustChange=true;
    }
    if (scrH>displaySize.h) {
      scrH=(displaySize.h)-32;
      mustChange=true;
    }
    if (mustChange) {
      portrait=(scrW<scrH);
      logV("portrait: %d (%dx%d)",portrait,scrW,scrH);
      if (!fullScreen) {
        logD("setting window size to %dx%d as it goes off bounds (%dx%d+%d+%d).",scrW,scrH,displaySize.w,displaySize.h,displaySize.x,displaySize.y);
        SDL_SetWindowSize(sdlWin,scrW,scrH);
      }
    }
  }
#endif

#ifdef IS_MOBILE
  SDL_GetWindowSize(sdlWin,&scrW,&scrH);
  portrait=(scrW<scrH);
  logV("portrait: %d (%dx%d)",portrait,scrW,scrH);
#endif

#if !(defined(__APPLE__) || defined(_WIN32))
  if (icon!=NULL) {
    SDL_SetWindowIcon(sdlWin,icon);
    SDL_FreeSurface(icon);
  } else {
    logW("could not create icon!");
  }
#endif

  int numDriversA=SDL_GetNumAudioDrivers();
  if (numDriversA<0) {
    logW("could not list audio drivers! %s",SDL_GetError());
  } else {
    for (int i=0; i<numDriversA; i++) {
      const char* r=SDL_GetAudioDriver(i);
      if (r==NULL) continue;
      if (strcmp(r,"disk")==0) continue;
      if (strcmp(r,"dummy")==0) continue;
      availAudioDrivers.push_back(String(r));
    }
  }

  int numDrivers=SDL_GetNumRenderDrivers();
  if (numDrivers<0) {
    logW("could not list render drivers! %s",SDL_GetError());
  } else {
    SDL_RendererInfo ri;
    logV("available render drivers:");
    for (int i=0; i<numDrivers; i++) {
      int r=SDL_GetRenderDriverInfo(i,&ri);
      if (r!=0) continue;
      availRenderDrivers.push_back(String(ri.name));
      logV("- %s",ri.name);
    }
  }

  if (!settings.renderDriver.empty()) {
    SDL_SetHint(SDL_HINT_RENDER_DRIVER,settings.renderDriver.c_str());
  }

  logD("starting render backend...");
  if (!rend->init(sdlWin,settings.vsync)) {
    logE("it failed...");
    if (settings.renderBackend!="Software") {
      settings.renderBackend="Software";
      e->setConf("renderBackend","Software");
      e->saveConf();
      lastError=fmt::sprintf(_("could not init renderer!\nfalling back to software renderer. please restart Furnace."));
    } else if (settings.renderBackend=="SDL") {
      lastError=fmt::sprintf(_("could not init renderer! %s\nfalling back to software renderer. please restart Furnace."),SDL_GetError());
      settings.renderBackend="Software";
      e->setConf("renderBackend","Software");
      e->saveConf();
    } else {
      lastError=fmt::sprintf(_("could not init renderer!"));
    }
    return false;
  }
  logV("render backend started");

  // set best texture format
  unsigned int availTexFormats=rend->getTextureFormats();
  if (availTexFormats&GUI_TEXFORMAT_ABGR32) {
    bestTexFormat=GUI_TEXFORMAT_ABGR32;
  } else if (availTexFormats&GUI_TEXFORMAT_ARGB32) {
    bestTexFormat=GUI_TEXFORMAT_ARGB32;
  } else if (availTexFormats&GUI_TEXFORMAT_RGBA32) {
    bestTexFormat=GUI_TEXFORMAT_RGBA32;
  } else if (availTexFormats&GUI_TEXFORMAT_BGRA32) {
    bestTexFormat=GUI_TEXFORMAT_BGRA32;
  }

  // try acquiring the canvas size
  if (!rend->getOutputSize(canvasW,canvasH)) {
    logW("could not get renderer output size!");
  } else {
    logV("canvas size: %dx%d",canvasW,canvasH);
  }

  // special consideration for Wayland
  if (settings.dpiScale<0.5f) {
    if (strcmp(videoBackend,"wayland")==0) {
      int realW=scrW;
      int realH=scrH;

      SDL_GetWindowSize(sdlWin,&realW,&realH);

      if (realW<1) {
        logW("screen width is zero!\n");
        dpiScale=1.0;
      } else {
        dpiScale=(double)canvasW/(double)realW;
        logV("we're on Wayland... scaling factor: %f",dpiScale);
      }
    }
  }

  updateWindowTitle();
  updateROMExportAvail();

  logV("max texture size: %dx%d",rend->getMaxTextureWidth(),rend->getMaxTextureHeight());

  rend->clear(ImVec4(0.0,0.0,0.0,1.0));
  rend->present();

  logD("preparing user interface...");
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  rend->initGUI(sdlWin);

  ImGuiLocEntry guiLocalization[8];

  guiLocalization[0].Key=ImGuiLocKey_TableSizeOne;
  guiLocalization[0].Text=_("Size column to fit###SizeOne");
  guiLocalization[1].Key=ImGuiLocKey_TableSizeAllFit;
  guiLocalization[1].Text=_("Size all columns to fit###SizeAll");
  guiLocalization[2].Key=ImGuiLocKey_TableSizeAllDefault;
  guiLocalization[2].Text=_("Size all columns to default###SizeAll");
  guiLocalization[3].Key=ImGuiLocKey_TableResetOrder;
  guiLocalization[3].Text=_("Reset order###ResetOrder");
  guiLocalization[4].Key=ImGuiLocKey_WindowingMainMenuBar;
  guiLocalization[4].Text=_("(Main menu bar)");
  guiLocalization[5].Key=ImGuiLocKey_WindowingPopup;
  guiLocalization[5].Text=_("(Popup)");
  guiLocalization[6].Key=ImGuiLocKey_WindowingUntitled;
  guiLocalization[6].Text=_("(Untitled)");
  guiLocalization[7].Key=ImGuiLocKey_DockingHideTabBar;
  guiLocalization[7].Text=_("Hide tab bar###HideTabBar");

  ImGui::LocalizeRegisterEntries(guiLocalization,8);

  const char* localeSettings=_("LocaleSettings: ccjk");
  if (strlen(localeSettings)<20) {
    logE("the LocaleSettings string is incomplete!");
  } else {
    localeRequiresChinese=(localeSettings[16]=='C');
    localeRequiresChineseTrad=(localeSettings[17]=='C');
    localeRequiresJapanese=(localeSettings[18]=='J');
    localeRequiresKorean=(localeSettings[19]=='K');
    if (strlen(localeSettings)>21) {
      if (localeSettings[20]==' ') {
        ImWchar next=0;
        for (const char* i=&localeSettings[21]; *i; i++) {
          if (((*i)>='0' && (*i)<='9') || ((*i)>='A' && (*i)<='F')) {
            next<<=4;
            if ((*i)>='0' && (*i)<='9') {
              next|=(*i)-'0';
            } else {
              next|=(*i)-'A'+10;
            }
          } else {
            localeExtraRanges.push_back(next);
            next=0;
          }
        }
        if (next!=0) {
          localeExtraRanges.push_back(next);
        }
        localeExtraRanges.push_back(0);
      }
    }
  }
  if (!localeExtraRanges.empty()) {
    logV("locale extra ranges:");
    for (ImWchar i: localeExtraRanges) {
      logV("%x",i);
    }
  }

  loadUserPresets(true);

  applyUISettings();

  logD("building font...");
  if (rend->areTexturesSquare()) {
    ImGui::GetIO().Fonts->Flags|=ImFontAtlasFlags_Square;
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
    }
    if (!ImGui::GetIO().Fonts->Build()) {
      logE("error again while building font atlas!");
    }
  }

  logD("preparing layout...");
  strncpy(finalLayoutPath,(e->getConfigPath()+String(LAYOUT_INI)).c_str(),4095);
  backupPath=e->getConfigPath();
  if (backupPath.size()>0) {
    if (backupPath[backupPath.size()-1]==DIR_SEPARATOR) backupPath.resize(backupPath.size()-1);
  }
  backupPath+=String(BACKUPS_DIR);
  prepareLayout();

  ImGui::GetIO().ConfigFlags|=ImGuiConfigFlags_DockingEnable;
  toggleMobileUI(mobileUI,true);

  firstFrame=true;

  userEvents=SDL_RegisterEvents(1);

  e->setMidiCallback([this](const TAMidiMessage& msg) -> int {
    if (introPos<11.0) return -2;
    midiLock.lock();
    midiQueue.push(msg);
    if (userEvents!=0xffffffff && midiWakeUp) {
      midiWakeUp=false;
      userEvent.user.type=userEvents;
      userEvent.user.code=0;
      userEvent.user.data1=NULL;
      userEvent.user.data2=NULL;
      SDL_PushEvent(&userEvent);
    }
    midiLock.unlock();
    e->setMidiBaseChan(cursor.xCoarse);
    if (msg.type==TA_MIDI_SYSEX) return -2;
    if (midiMap.valueInputStyle!=0 && cursor.xFine!=0 && edit) return -2;
    if (!midiMap.noteInput) return -2;
    if (learning!=-1) return -2;
    if (midiMap.at(msg)) return -2;

    if (curWindowThreadSafe==GUI_WINDOW_WAVE_EDIT || curWindowThreadSafe==GUI_WINDOW_WAVE_LIST) {
      if ((msg.type&0xf0)==TA_MIDI_NOTE_ON) {
        e->previewWaveNoLock(curWave,msg.data[0]-12);
        wavePreviewNote=msg.data[0]-12;
      } else if ((msg.type&0xf0)==TA_MIDI_NOTE_OFF) {
        if (wavePreviewNote==msg.data[0]-12) {
          e->stopWavePreviewNoLock();
        }
      }
      return -2;
    }

    if (curWindowThreadSafe==GUI_WINDOW_SAMPLE_EDIT || curWindowThreadSafe==GUI_WINDOW_SAMPLE_LIST) {
      if ((msg.type&0xf0)==TA_MIDI_NOTE_ON) {
        e->previewSampleNoLock(curSample,msg.data[0]-12);
        samplePreviewNote=msg.data[0]-12;
      } else if ((msg.type&0xf0)==TA_MIDI_NOTE_OFF) {
        if (samplePreviewNote==msg.data[0]-12) {
          e->stopSamplePreviewNoLock();
        }
      }
      return -2;
    }

    if (midiMap.directChannel && midiMap.directProgram) return -1;
    return curIns;
  });

#ifdef IS_MOBILE
  vibrator=SDL_HapticOpen(0);
  if (vibrator==NULL) {
    logD("could not open vibration device: %s",SDL_GetError());
  } else {
    if (SDL_HapticRumbleInit(vibrator)==0) {
      vibratorAvailable=true;
    } else {
      logD("vibration not available: %s",SDL_GetError());
    }
  }
#endif

  cpuCores=SDL_GetCPUCount();
  if (cpuCores<1) cpuCores=1;

  time_t thisMakesNoSense=time(NULL);
  struct tm curTime;
#ifdef _WIN32
  struct tm* tempTM=localtime(&thisMakesNoSense);
  if (tempTM==NULL) {
    memset(&curTime,0,sizeof(struct tm));
  } else {
    memcpy(&curTime,tempTM,sizeof(struct tm));
    purgeYear=1900+curTime.tm_year-1;
    purgeMonth=curTime.tm_mon+1;
    purgeDay=curTime.tm_mday;
  }
#else
  if (localtime_r(&thisMakesNoSense,&curTime)==NULL) {
    memset(&curTime,0,sizeof(struct tm));
  } else {
    purgeYear=1900+curTime.tm_year-1;
    purgeMonth=curTime.tm_mon+1;
    purgeDay=curTime.tm_mday;
  }
#endif

  // initialize audio formats
  String compatFormats;

  audioLoadFormats.push_back(_("compatible files"));
  audioLoadFormats.push_back("");

#ifdef HAVE_SNDFILE
  int value=0;
  sf_command(NULL,SFC_GET_FORMAT_MAJOR_COUNT,&value,sizeof(int));
  logV("simple formats: %d",value);

  for (int i=0; i<value; i++) {
    SF_FORMAT_INFO f;
    f.format=i;
    if (sf_command(NULL,SFC_GET_FORMAT_MAJOR,&f,sizeof(SF_FORMAT_INFO))!=0) continue;
    logV("format %d: %s (%s)\n",i,f.name,f.extension);
    // these two are/will be handled somewhere else
    if (strcmp(f.extension,"raw")==0) continue;
    if (strcmp(f.extension,"xi")==0) continue;
    // just in case
    if (strcmp(f.extension,"dmc")==0) continue;
    if (strcmp(f.extension,"brr")==0) continue;
    audioLoadFormats.push_back(f.name);
    audioLoadFormats.push_back(fmt::sprintf("*.%s",f.extension));
    compatFormats+=fmt::sprintf("*.%s ",f.extension);
  }
#endif

  compatFormats+="*.dmc ";
  compatFormats+="*.brr ";

  compatFormats+="*.ppc ";
  compatFormats+="*.pps ";
  compatFormats+="*.pvi ";
  compatFormats+="*.pdx ";
  compatFormats+="*.pzi ";
  compatFormats+="*.p86 ";
  compatFormats+="*.p";
  audioLoadFormats[1]=compatFormats;

  audioLoadFormats.push_back(_("NES DPCM data"));
  audioLoadFormats.push_back("*.dmc");

  audioLoadFormats.push_back(_("SNES Bit Rate Reduction"));
  audioLoadFormats.push_back("*.brr");

  audioLoadFormats.push_back(_("PMD YM2608 ADPCM-B sample bank"));
  audioLoadFormats.push_back("*.ppc");

  audioLoadFormats.push_back(_("PDR 4-bit AY-3-8910 sample bank"));
  audioLoadFormats.push_back("*.pps");

  audioLoadFormats.push_back(_("FMP YM2608 ADPCM-B sample bank"));
  audioLoadFormats.push_back("*.pvi");

  audioLoadFormats.push_back(_("MDX OKI ADPCM sample bank"));
  audioLoadFormats.push_back("*.pdx");

  audioLoadFormats.push_back(_("FMP 8-bit PCM sample bank"));
  audioLoadFormats.push_back("*.pzi");

  audioLoadFormats.push_back(_("PMD 8-bit PCM sample bank"));
  audioLoadFormats.push_back("*.p86");

  audioLoadFormats.push_back(_("PMD OKI ADPCM sample bank"));
  audioLoadFormats.push_back("*.p");

  audioLoadFormats.push_back(_("all files"));
  audioLoadFormats.push_back("*");

  logI("done!");
  return true;
}

void FurnaceGUI::syncState() {
  String homeDir=getHomeDir();
  workingDir=e->getConfString("lastDir",homeDir);
  workingDirSong=e->getConfString("lastDirSong",workingDir);
  workingDirIns=e->getConfString("lastDirIns",workingDir);
  workingDirWave=e->getConfString("lastDirWave",workingDir);
  workingDirSample=e->getConfString("lastDirSample",workingDir);
  workingDirAudioExport=e->getConfString("lastDirAudioExport",workingDir);
  workingDirVGMExport=e->getConfString("lastDirVGMExport",workingDir);
  workingDirROMExport=e->getConfString("lastDirROMExport",workingDir);
  workingDirFont=e->getConfString("lastDirFont",workingDir);
  workingDirColors=e->getConfString("lastDirColors",workingDir);
  workingDirKeybinds=e->getConfString("lastDirKeybinds",workingDir);
  workingDirLayout=e->getConfString("lastDirLayout",workingDir);
  workingDirConfig=e->getConfString("lastDirConfig",workingDir);
  workingDirTest=e->getConfString("lastDirTest",workingDir);

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
  chanOscOpen=e->getConfBool("chanOscOpen",false);
  xyOscOpen=e->getConfBool("xyOscOpen",false);
  memoryOpen=e->getConfBool("memoryOpen",false);
  csPlayerOpen=e->getConfBool("csPlayerOpen",false);
  volMeterOpen=e->getConfBool("volMeterOpen",true);
  statsOpen=e->getConfBool("statsOpen",false);
  compatFlagsOpen=e->getConfBool("compatFlagsOpen",false);
#ifdef IS_MOBILE
  pianoOpen=e->getConfBool("pianoOpen",true);
#else
  pianoOpen=e->getConfBool("pianoOpen",false);
#endif
  notesOpen=e->getConfBool("notesOpen",false);
  channelsOpen=e->getConfBool("channelsOpen",false);
  patManagerOpen=e->getConfBool("patManagerOpen",false);
  sysManagerOpen=e->getConfBool("sysManagerOpen",false);
  clockOpen=e->getConfBool("clockOpen",false);
  speedOpen=e->getConfBool("speedOpen",true);
  groovesOpen=e->getConfBool("groovesOpen",false);
  regViewOpen=e->getConfBool("regViewOpen",false);
  logOpen=e->getConfBool("logOpen",false);
  effectListOpen=e->getConfBool("effectListOpen",true);
  subSongsOpen=e->getConfBool("subSongsOpen",true);
  findOpen=e->getConfBool("findOpen",false);
  spoilerOpen=e->getConfBool("spoilerOpen",false);
  userPresetsOpen=e->getConfBool("userPresetsOpen",false);

  insListDir=e->getConfBool("insListDir",false);
  waveListDir=e->getConfBool("waveListDir",false);
  sampleListDir=e->getConfBool("sampleListDir",false);

  tempoView=e->getConfBool("tempoView",true);
  waveHex=e->getConfBool("waveHex",false);
  waveSigned=e->getConfBool("waveSigned",false);
  waveGenVisible=e->getConfBool("waveGenVisible",false);
  waveEditStyle=e->getConfInt("waveEditStyle",0);
  int extraChannelButtons=e->getConfInt("extraChannelButtons",0);
  if (!e->hasConf("patExtraButtons")) {
    patExtraButtons=(extraChannelButtons==1);
  } else {
    patExtraButtons=e->getConfBool("patExtraButtons",false);
  }
  if (!e->hasConf("patChannelNames")) {
    patChannelNames=(extraChannelButtons==2);
  } else {
    patChannelNames=e->getConfBool("patChannelNames",false);
  }
  patChannelPairs=e->getConfBool("patChannelPairs",true);
  patChannelHints=e->getConfInt("patChannelHints",0);
  lockLayout=e->getConfBool("lockLayout",false);
#ifdef IS_MOBILE
  fullScreen=true;
#else
  fullScreen=e->getConfBool("fullScreen",false);
#endif
  mobileUI=e->getConfBool("mobileUI",MOBILE_UI_DEFAULT);
  edit=e->getConfBool("edit",false);
  followOrders=e->getConfBool("followOrders",true);
  followPattern=e->getConfBool("followPattern",true);
  noteInputPoly=e->getConfBool("noteInputPoly",true);
  audioExportOptions.loops=e->getConfInt("exportLoops",0);
  if (audioExportOptions.loops<0) audioExportOptions.loops=0;
  audioExportOptions.fadeOut=e->getConfDouble("exportFadeOut",0.0);
  if (audioExportOptions.fadeOut<0.0) audioExportOptions.fadeOut=0.0;
  orderEditMode=e->getConfInt("orderEditMode",0);
  if (orderEditMode<0) orderEditMode=0;
  if (orderEditMode>3) orderEditMode=3;

  oscZoom=e->getConfFloat("oscZoom",0.5f);
  oscZoomSlider=e->getConfBool("oscZoomSlider",false);
  oscWindowSize=e->getConfFloat("oscWindowSize",20.0f);

  pianoOctaves=e->getConfInt("pianoOctaves",pianoOctaves);
  pianoOctavesEdit=e->getConfInt("pianoOctavesEdit",pianoOctavesEdit);
  pianoOptions=e->getConfBool("pianoOptions",pianoOptions);
  pianoSharePosition=e->getConfBool("pianoSharePosition",pianoSharePosition);
  pianoOptionsSet=e->getConfBool("pianoOptionsSet",pianoOptionsSet);
  pianoReadonly=e->getConfBool("pianoReadonly",false);
  pianoOffset=e->getConfInt("pianoOffset",pianoOffset);
  pianoOffsetEdit=e->getConfInt("pianoOffsetEdit",pianoOffsetEdit);
  pianoView=e->getConfInt("pianoView",pianoView);
  pianoInputPadMode=e->getConfInt("pianoInputPadMode",pianoInputPadMode);

  chanOscCols=e->getConfInt("chanOscCols",3);
  chanOscAutoColsType=e->getConfInt("chanOscAutoColsType",0);
  chanOscColorX=e->getConfInt("chanOscColorX",GUI_OSCREF_CENTER);
  chanOscColorY=e->getConfInt("chanOscColorY",GUI_OSCREF_CENTER);
  chanOscCenterStrat=e->getConfInt("chanOscCenterStrat",1);
  chanOscTextX=e->getConfFloat("chanOscTextX",0.0f);
  chanOscTextY=e->getConfFloat("chanOscTextY",0.0f);
  chanOscAmplify=e->getConfFloat("chanOscAmplify",0.95f);
  chanOscLineSize=e->getConfFloat("chanOscLineSize",1.0f);
  chanOscWindowSize=e->getConfFloat("chanOscWindowSize",20.0f);
  chanOscWaveCorr=e->getConfBool("chanOscWaveCorr",true);
  chanOscOptions=e->getConfBool("chanOscOptions",false);
  chanOscNormalize=e->getConfBool("chanOscNormalize",false);
  chanOscRandomPhase=e->getConfBool("chanOscRandomPhase",false);
  chanOscTextFormat=e->getConfString("chanOscTextFormat","%c");
  chanOscColor.x=e->getConfFloat("chanOscColorR",1.0f);
  chanOscColor.y=e->getConfFloat("chanOscColorG",1.0f);
  chanOscColor.z=e->getConfFloat("chanOscColorB",1.0f);
  chanOscColor.w=e->getConfFloat("chanOscColorA",1.0f);
  chanOscTextColor.x=e->getConfFloat("chanOscTextColorR",1.0f);
  chanOscTextColor.y=e->getConfFloat("chanOscTextColorG",1.0f);
  chanOscTextColor.z=e->getConfFloat("chanOscTextColorB",1.0f);
  chanOscTextColor.w=e->getConfFloat("chanOscTextColorA",0.75f);
  chanOscUseGrad=e->getConfBool("chanOscUseGrad",false);
  chanOscGrad.fromString(e->getConfString("chanOscGrad",""));
  chanOscGrad.render();

  xyOscXChannel=e->getConfInt("xyOscXChannel",0);
  xyOscXInvert=e->getConfBool("xyOscXInvert",false);
  xyOscYChannel=e->getConfInt("xyOscYChannel",1);
  xyOscYInvert=e->getConfBool("xyOscYInvert",false);
  xyOscZoom=e->getConfFloat("xyOscZoom",1.0f);
  xyOscSamples=e->getConfInt("xyOscSamples",32768);
  xyOscDecayTime=e->getConfFloat("xyOscDecayTime",10.0f);
  xyOscIntensity=e->getConfFloat("xyOscIntensity",2.0f);
  xyOscThickness=e->getConfFloat("xyOscThickness",2.0f);

  cvHiScore=e->getConfInt("cvHiScore",25000);
}

void FurnaceGUI::commitState(DivConfig& conf) {
  if (!mobileUI) {
    if (!ImGui::SaveIniSettingsToDisk(finalLayoutPath,true)) {
      reportError(fmt::sprintf(_("could NOT save layout! %s"),strerror(errno)));
    }
  }

  conf.set("configVersion",(int)DIV_ENGINE_VERSION);

  conf.set("lastDir",workingDir);
  conf.set("lastDirSong",workingDirSong);
  conf.set("lastDirIns",workingDirIns);
  conf.set("lastDirWave",workingDirWave);
  conf.set("lastDirSample",workingDirSample);
  conf.set("lastDirAudioExport",workingDirAudioExport);
  conf.set("lastDirVGMExport",workingDirVGMExport);
  conf.set("lastDirROMExport",workingDirROMExport);
  conf.set("lastDirFont",workingDirFont);
  conf.set("lastDirColors",workingDirColors);
  conf.set("lastDirKeybinds",workingDirKeybinds);
  conf.set("lastDirLayout",workingDirLayout);
  conf.set("lastDirConfig",workingDirConfig);
  conf.set("lastDirTest",workingDirTest);

  // commit last open windows
  conf.set("editControlsOpen",editControlsOpen);
  conf.set("ordersOpen",ordersOpen);
  conf.set("insListOpen",insListOpen);
  conf.set("songInfoOpen",songInfoOpen);
  conf.set("patternOpen",patternOpen);
  conf.set("insEditOpen",insEditOpen);
  conf.set("waveListOpen",waveListOpen);
  conf.set("waveEditOpen",waveEditOpen);
  conf.set("sampleListOpen",sampleListOpen);
  conf.set("sampleEditOpen",sampleEditOpen);
  conf.set("settingsOpen",settingsOpen);
  conf.set("mixerOpen",mixerOpen);
  conf.set("oscOpen",oscOpen);
  conf.set("chanOscOpen",chanOscOpen);
  conf.set("xyOscOpen",xyOscOpen);
  conf.set("memoryOpen",memoryOpen);
  conf.set("csPlayerOpen",csPlayerOpen);
  conf.set("volMeterOpen",volMeterOpen);
  conf.set("statsOpen",statsOpen);
  conf.set("compatFlagsOpen",compatFlagsOpen);
  conf.set("pianoOpen",pianoOpen);
  conf.set("notesOpen",notesOpen);
  conf.set("channelsOpen",channelsOpen);
  conf.set("patManagerOpen",patManagerOpen);
  conf.set("sysManagerOpen",sysManagerOpen);
  conf.set("clockOpen",clockOpen);
  conf.set("speedOpen",speedOpen);
  conf.set("groovesOpen",groovesOpen);
  conf.set("regViewOpen",regViewOpen);
  conf.set("logOpen",logOpen);
  conf.set("effectListOpen",effectListOpen);
  conf.set("subSongsOpen",subSongsOpen);
  conf.set("findOpen",findOpen);
  conf.set("spoilerOpen",spoilerOpen);
  conf.set("userPresetsOpen",userPresetsOpen);

  // commit dir state
  conf.set("insListDir",insListDir);
  conf.set("waveListDir",waveListDir);
  conf.set("sampleListDir",sampleListDir);

  // commit last window size
  conf.set("lastWindowWidth",scrConfW);
  conf.set("lastWindowHeight",scrConfH);
  conf.set("lastWindowX",settings.saveWindowPos?scrConfX:(int)SDL_WINDOWPOS_CENTERED);
  conf.set("lastWindowY",settings.saveWindowPos?scrConfY:(int)SDL_WINDOWPOS_CENTERED);
  conf.set("lastWindowMax",scrMax);

  conf.set("tempoView",tempoView);
  conf.set("waveHex",waveHex);
  conf.set("waveSigned",waveSigned);
  conf.set("waveGenVisible",waveGenVisible);
  conf.set("waveEditStyle",waveEditStyle);
  conf.set("patExtraButtons",patExtraButtons);
  conf.set("patChannelNames",patChannelNames);
  conf.set("patChannelPairs",patChannelPairs);
  conf.set("patChannelHints",(int)patChannelHints);
  conf.set("lockLayout",lockLayout);
  conf.set("fullScreen",fullScreen);
  conf.set("mobileUI",mobileUI);
  conf.set("edit",edit);
  conf.set("followOrders",followOrders);
  conf.set("followPattern",followPattern);
  conf.set("orderEditMode",orderEditMode);
  conf.set("noteInputPoly",noteInputPoly);
  if (settings.persistFadeOut) {
    conf.set("exportLoops",audioExportOptions.loops);
    conf.set("exportFadeOut",audioExportOptions.fadeOut);
  }

  // commit oscilloscope state
  conf.set("oscZoom",oscZoom);
  conf.set("oscZoomSlider",oscZoomSlider);
  conf.set("oscWindowSize",oscWindowSize);

  // commit piano state
  conf.set("pianoOctaves",pianoOctaves);
  conf.set("pianoOctavesEdit",pianoOctavesEdit);
  conf.set("pianoOptions",pianoOptions);
  conf.set("pianoSharePosition",pianoSharePosition);
  conf.set("pianoOptionsSet",pianoOptionsSet);
  conf.set("pianoReadonly",pianoReadonly);
  conf.set("pianoOffset",pianoOffset);
  conf.set("pianoOffsetEdit",pianoOffsetEdit);
  conf.set("pianoView",pianoView);
  conf.set("pianoInputPadMode",pianoInputPadMode);

  // commit per-chan osc state
  conf.set("chanOscCols",chanOscCols);
  conf.set("chanOscAutoColsType",chanOscAutoColsType);
  conf.set("chanOscColorX",chanOscColorX);
  conf.set("chanOscColorY",chanOscColorY);
  conf.set("chanOscCenterStrat",chanOscCenterStrat);
  conf.set("chanOscTextX",chanOscTextX);
  conf.set("chanOscTextY",chanOscTextY);
  conf.set("chanOscAmplify",chanOscAmplify);
  conf.set("chanOscLineSize",chanOscLineSize);
  conf.set("chanOscWindowSize",chanOscWindowSize);
  conf.set("chanOscWaveCorr",chanOscWaveCorr);
  conf.set("chanOscOptions",chanOscOptions);
  conf.set("chanOscNormalize",chanOscNormalize);
  conf.set("chanOscRandomPhase",chanOscRandomPhase);
  conf.set("chanOscTextFormat",chanOscTextFormat);
  conf.set("chanOscColorR",chanOscColor.x);
  conf.set("chanOscColorG",chanOscColor.y);
  conf.set("chanOscColorB",chanOscColor.z);
  conf.set("chanOscColorA",chanOscColor.w);
  conf.set("chanOscTextColorR",chanOscTextColor.x);
  conf.set("chanOscTextColorG",chanOscTextColor.y);
  conf.set("chanOscTextColorB",chanOscTextColor.z);
  conf.set("chanOscTextColorA",chanOscTextColor.w);
  conf.set("chanOscUseGrad",chanOscUseGrad);
  conf.set("chanOscGrad",chanOscGrad.toString());

  // commit x-y osc state
  conf.set("xyOscXChannel",xyOscXChannel);
  conf.set("xyOscXInvert",xyOscXInvert);
  conf.set("xyOscYChannel",xyOscYChannel);
  conf.set("xyOscYInvert",xyOscYInvert);
  conf.set("xyOscZoom",xyOscZoom);
  conf.set("xyOscSamples",xyOscSamples);
  conf.set("xyOscDecayTime",xyOscDecayTime);
  conf.set("xyOscIntensity",xyOscIntensity);
  conf.set("xyOscThickness",xyOscThickness);

  // commit recent files
  for (int i=0; i<30; i++) {
    String key=fmt::sprintf("recentFile%d",i);
    if (i>=settings.maxRecentFile || i>=(int)recentFile.size()) {
      conf.set(key,"");
    } else {
      conf.set(key,recentFile[i]);
    }
  }

  conf.set("cvHiScore",cvHiScore);
}

bool FurnaceGUI::finish(bool saveConfig) {
  commitState(e->getConfObject());
  if (userPresetsOpen) {
    saveUserPresets(true);
  }
  if (saveConfig) {
    logI("saving config.");
    e->saveConf();
  }
  rend->quitGUI();
  ImGui_ImplSDL2_Shutdown();
  quitRender();
  ImGui::DestroyContext();
  SDL_DestroyWindow(sdlWin);

  if (vibrator) {
    SDL_HapticClose(vibrator);
  }

  for (int i=0; i<DIV_MAX_OUTPUTS; i++) {
    if (oscValues[i]) {
      delete[] oscValues[i];
      oscValues[i]=NULL;
    }
  }
  if (oscValuesAverage) {
    delete[] oscValuesAverage;
    oscValuesAverage=NULL;
  }

  if (backupTask.valid()) {
    backupTask.get();
  }

  if (chanOscWorkPool!=NULL) {
    delete chanOscWorkPool;
  }

  return true;
}

bool FurnaceGUI::requestQuit() {
  if (modified && !cvOpen) {
    showWarning(_("Unsaved changes! Save changes before quitting?"),GUI_WARN_QUIT);
  } else {
    quit=true;
  }
  return quit;
}

FurnaceGUI::FurnaceGUI():
  e(NULL),
  renderBackend(GUI_BACKEND_SDL),
  rend(NULL),
  sdlWin(NULL),
  vibrator(NULL),
  vibratorAvailable(false),
  cv(NULL),
  lastCVFrame(0),
  cvFrameTime(100000),
  cvFrameHold(0),
  sampleTex(NULL),
  sampleTexW(0),
  sampleTexH(0),
  updateSampleTex(true),
  quit(false),
  warnQuit(false),
  willCommit(false),
  edit(false),
  editClone(false),
  isPatUnique(false),
  modified(false),
  displayError(false),
  displayExporting(false),
  vgmExportLoop(true),
  vgmExportPatternHints(false),
  vgmExportDPCM07(false),
  vgmExportDirectStream(false),
  displayInsTypeList(false),
  portrait(false),
  injectBackUp(false),
  mobileMenuOpen(false),
  warnColorPushed(false),
  wantCaptureKeyboard(false),
  oldWantCaptureKeyboard(false),
  displayMacroMenu(false),
  displayNew(false),
  displayPalette(false),
  fullScreen(false),
  preserveChanPos(false),
  sysDupCloneChannels(true),
  sysDupEnd(false),
  noteInputPoly(true),
  notifyWaveChange(false),
  wantScrollListIns(false),
  wantScrollListWave(false),
  wantScrollListSample(false),
  displayPendingIns(false),
  pendingInsSingle(false),
  displayPendingRawSample(false),
  snesFilterHex(false),
  modTableHex(false),
  displayEditString(false),
  displayPendingSamples(false),
  replacePendingSample(false),
  displayExportingROM(false),
  displayExportingCS(false),
  changeCoarse(false),
  mobileEdit(false),
  killGraphics(false),
  safeMode(false),
  midiWakeUp(true),
  makeDrumkitMode(false),
  audioEngineChanged(false),
  settingsChanged(false),
  debugFFT(false),
  vgmExportVersion(0x171),
  vgmExportTrailingTicks(-1),
  drawHalt(10),
  macroPointSize(16),
  waveEditStyle(0),
  displayInsTypeListMakeInsSample(-1),
  makeDrumkitOctave(3),
  mobileEditPage(0),
  wheelCalmDown(0),
  shallDetectScale(0),
  cpuCores(0),
  secondTimer(0.0f),
  userEvents(0xffffffff),
  mobileMenuPos(0.0f),
  autoButtonSize(0.0f),
  mobileEditAnim(0.0f),
  mobileEditButtonPos(0.7f,0.7f),
  mobileEditButtonSize(60.0f,60.0f),
  curSysSection(NULL),
  updateFMPreview(true),
  fmPreviewOn(false),
  fmPreviewPaused(false),
  fmPreviewOPN(NULL),
  fmPreviewOPM(NULL),
  fmPreviewOPL(NULL),
  fmPreviewOPLL(NULL),
  fmPreviewOPZ(NULL),
  fmPreviewOPZInterface(NULL),
  editString(NULL),
  pendingRawSampleDepth(8),
  pendingRawSampleChannels(1),
  pendingRawSampleRate(32000),
  pendingRawSampleUnsigned(false),
  pendingRawSampleBigEndian(false),
  pendingRawSampleSwapNibbles(false),
  pendingRawSampleReplace(false),
  globalWinFlags(0),
  curFileDialog(GUI_FILE_OPEN),
  warnAction(GUI_WARN_OPEN),
  postWarnAction(GUI_WARN_GENERIC),
  mobScene(GUI_SCENE_PATTERN),
  fileDialog(NULL),
  scrW(GUI_WIDTH_DEFAULT),
  scrH(GUI_HEIGHT_DEFAULT),
  scrConfW(GUI_WIDTH_DEFAULT),
  scrConfH(GUI_HEIGHT_DEFAULT),
  canvasW(GUI_WIDTH_DEFAULT),
  canvasH(GUI_HEIGHT_DEFAULT),
  scrX(SDL_WINDOWPOS_CENTERED),
  scrY(SDL_WINDOWPOS_CENTERED),
  scrConfX(SDL_WINDOWPOS_CENTERED),
  scrConfY(SDL_WINDOWPOS_CENTERED),
  scrMax(false),
  sysManagedScale(false),
  dpiScale(1),
  aboutScroll(0),
  aboutSin(0),
  aboutHue(0.0f),
  backupTimer(0.0),
  totalBackupSize(0),
  refreshBackups(true),
  learning(-1),
  mainFont(NULL),
  iconFont(NULL),
  furIconFont(NULL),
  patFont(NULL),
  bigFont(NULL),
  headFont(NULL),
  fontRange(NULL),
  songLength(0),
  songLoopedSectionLength(0),
  songFadeoutSectionLength(0),
  songHasSongEndCommand(false),
  lengthOfOneFile(0),
  totalLength(0),
  curProgress(0.0f),
  totalFiles(0),
  localeRequiresJapanese(false),
  localeRequiresChinese(false),
  localeRequiresChineseTrad(false),
  localeRequiresKorean(false),
  prevInsData(NULL),
  cachedCurInsPtr(NULL),
  insEditMayBeDirty(false),
  pendingLayoutImport(NULL),
  pendingLayoutImportLen(0),
  pendingLayoutImportStep(0),
  curIns(0),
  curWave(0),
  curSample(0),
  curOctave(3),
  curOrder(0),
  playOrder(0),
  prevIns(0),
  oldRow(0),
  editStep(1),
  editStepCoarse(16),
  soloChan(-1),
  orderEditMode(0),
  orderCursor(-1),
  loopOrder(-1),
  loopRow(-1),
  loopEnd(-1),
  isClipping(0),
  newSongCategory(0),
  latchTarget(0),
  wheelX(0),
  wheelY(0),
  dragSourceX(0),
  dragSourceY(0),
  dragDestinationX(0),
  dragDestinationY(0),
  oldBeat(-1),
  oldBar(-1),
  curGroove(-1),
  exitDisabledTimer(0),
  curPaletteChoice(0),
  curPaletteType(0),
  soloTimeout(0.0f),
  purgeYear(2021),
  purgeMonth(4),
  purgeDay(4),
  patExtraButtons(false),
  patChannelNames(false),
  patChannelPairs(true),
  patChannelHints(0),
  newSongFirstFrame(false),
  oldRowChanged(false),
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
  logOpen(false),
  effectListOpen(false),
  chanOscOpen(false),
  subSongsOpen(true),
  findOpen(false),
  spoilerOpen(false),
  patManagerOpen(false),
  sysManagerOpen(false),
  clockOpen(false),
  speedOpen(true),
  groovesOpen(false),
  xyOscOpen(false),
  memoryOpen(false),
  csPlayerOpen(false),
  cvOpen(false),
  userPresetsOpen(false),
  cvNotSerious(false),
  shortIntro(false),
  insListDir(false),
  waveListDir(false),
  sampleListDir(false),
  clockShowReal(true),
  clockShowRow(true),
  clockShowBeat(true),
  clockShowMetro(true),
  clockShowTime(true),
  selecting(false),
  selectingFull(false),
  dragging(false),
  curNibble(false),
  orderNibble(false),
  followOrders(true),
  followPattern(true),
  changeAllOrders(false),
  mobileUI(MOBILE_UI_DEFAULT),
  collapseWindow(false),
  demandScrollX(false),
  fancyPattern(false),
  firstFrame(true),
  tempoView(true),
  waveHex(false),
  waveSigned(false),
  waveGenVisible(false),
  lockLayout(false),
  editOptsVisible(false),
  latchNibble(false),
  nonLatchNibble(false),
  keepLoopAlive(false),
  keepGrooveAlive(false),
  orderScrollLocked(false),
  orderScrollTolerance(false),
  dragMobileMenu(false),
  dragMobileEditButton(false),
  wantGrooveListFocus(false),
  mobilePatSel(false),
  lastAssetType(0),
  curWindow(GUI_WINDOW_NOTHING),
  nextWindow(GUI_WINDOW_NOTHING),
  curWindowLast(GUI_WINDOW_NOTHING),
  curWindowThreadSafe(GUI_WINDOW_NOTHING),
  failedNoteOn(false),
  lastPatternWidth(0.0f),
  longThreshold(0.48f),
  buttonLongThreshold(0.20f),
  lastAudioLoadsPos(0),
  latchNote(-1),
  latchIns(-2),
  latchVol(-1),
  latchEffect(-1),
  latchEffectVal(-1),
  wavePreviewLen(32),
  wavePreviewHeight(255),
  wavePreviewInit(true),
  wavePreviewPaused(false),
  pgSys(0),
  pgAddr(0),
  pgVal(0),
  curQueryRangeX(false),
  curQueryBackwards(false),
  curQueryRangeXMin(0), curQueryRangeXMax(0),
  curQueryRangeY(0),
  curQueryEffectPos(0),
  queryReplaceEffectCount(0),
  queryReplaceEffectPos(1),
  queryReplaceNoteMode(0),
  queryReplaceInsMode(0),
  queryReplaceVolMode(0),
  queryReplaceNote(0),
  queryReplaceIns(0),
  queryReplaceVol(0),
  queryReplaceNoteDo(false),
  queryReplaceInsDo(false),
  queryReplaceVolDo(false),
  queryViewingResults(false),
  wavePreviewOn(false),
  wavePreviewKey((SDL_Scancode)0),
  wavePreviewNote(0),
  samplePreviewOn(false),
  samplePreviewKey((SDL_Scancode)0),
  samplePreviewNote(0),
  sampleMapSelStart(-1),
  sampleMapSelEnd(-1),
  sampleMapDigit(0),
  sampleMapColumn(0),
  sampleMapFocused(false),
  sampleMapWaitingInput(false),
  macroDragStart(0,0),
  macroDragAreaSize(0,0),
  macroDragCTarget(NULL),
  macroDragTarget(NULL),
  macroDragLen(0),
  macroDragMin(0),
  macroDragMax(0),
  macroDragLastX(-1),
  macroDragLastY(-1),
  macroDragScroll(0),
  macroDragBitMode(false),
  macroDragInitialValueSet(false),
  macroDragInitialValue(false),
  macroDragChar(false),
  macroDragBit30(false),
  macroDragSettingBit30(false),
  macroDragLineMode(false),
  macroDragMouseMoved(false),
  macroDragLineInitial(0,0),
  macroDragLineInitialV(0,0),
  macroDragActive(false),
  lastMacroDesc(NULL,NULL,0,0,0.0f),
  macroOffX(0),
  macroOffY(0),
  macroScaleX(100.0f),
  macroScaleY(100.0f),
  macroRandMin(0),
  macroRandMax(0),
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
  bindSetTargetIdx(0),
  bindSetPrevValue(0),
  bindSetActive(false),
  bindSetPending(false),
  nextScroll(-1.0f),
  nextAddScroll(0.0f),
  orderScroll(0.0f),
  orderScrollSlideOrigin(0.0f),
  orderScrollRealOrigin(0.0f,0.0f),
  dragMobileMenuOrigin(0.0f,0.0f),
  layoutTimeBegin(0),
  layoutTimeEnd(0),
  layoutTimeDelta(0),
  renderTimeBegin(0),
  renderTimeEnd(0),
  renderTimeDelta(0),
  drawTimeBegin(0),
  drawTimeEnd(0),
  drawTimeDelta(0),
  swapTimeBegin(0),
  swapTimeEnd(0),
  swapTimeDelta(0),
  eventTimeBegin(0),
  eventTimeEnd(0),
  eventTimeDelta(0),
  nextPresentTime(0),
  perfMetricsLen(0),
  chanToMove(-1),
  sysToMove(-1),
  sysToDelete(-1),
  opToMove(-1),
  assetToMove(-1),
  dirToMove(-1),
  insToMove(-1),
  waveToMove(-1),
  sampleToMove(-1),
  transposeAmount(0),
  randomizeMin(0),
  randomizeMax(255),
  fadeMin(0),
  fadeMax(255),
  collapseAmount(2),
  randomizeEffectVal(0),
  playheadY(0.0f),
  scaleMax(100.0f),
  fadeMode(false),
  randomMode(false),
  haveHitBounds(false),
  randomizeEffect(false),
  pendingStepUpdate(0),
  oldOrdersLen(0),
  sampleZoom(1.0),
  prevSampleZoom(1.0),
  minSampleZoom(1.0),
  samplePos(0),
  resizeSize(1024),
  silenceSize(1024),
  resampleTarget(32000),
  resampleStrat(5),
  amplifyVol(100.0),
  sampleSelStart(-1),
  sampleSelEnd(-1),
  sampleInfo(true),
  sampleCompatRate(false),
  sampleDragActive(false),
  sampleDragMode(false),
  sampleDrag16(false),
  sampleZoomAuto(true),
  sampleSelTarget(0),
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
  sampleFilterSweep(false),
  sampleFilterFirstFrame(true),
  sampleCrossFadeLoopLength(0),
  sampleCrossFadeLoopLaw(50),
  sampleFilterPower(1),
  sampleClipboard(NULL),
  sampleClipboardLen(0),
  openSampleResizeOpt(false),
  openSampleResampleOpt(false),
  openSampleAmplifyOpt(false),
  openSampleSilenceOpt(false),
  openSampleFilterOpt(false),
  openSampleCrossFadeOpt(false),
  selectedPortSet(0x1fff),
  selectedSubPort(-1),
  hoveredPortSet(0x1fff),
  hoveredSubPort(-1),
  portDragActive(false),
  displayHiddenPorts(false),
  displayInternalPorts(false),
  subPortPos(0.0f,0.0f),
  oscTotal(0),
  oscWidth(512),
  oscValuesAverage(NULL),
  oscZoom(0.5f),
  oscWindowSize(20.0f),
  oscInput(0.0f),
  oscInput1(0.0f),
  oscZoomSlider(false),
  chanOscCols(3),
  chanOscAutoColsType(0),
  chanOscColorX(GUI_OSCREF_CENTER),
  chanOscColorY(GUI_OSCREF_CENTER),
  chanOscCenterStrat(1),
  chanOscWindowSize(20.0f),
  chanOscTextX(0.0f),
  chanOscTextY(0.0f),
  chanOscAmplify(0.95f),
  chanOscLineSize(1.0f),
  chanOscWaveCorr(true),
  chanOscOptions(false),
  updateChanOscGradTex(true),
  chanOscUseGrad(false),
  chanOscNormalize(false),
  chanOscRandomPhase(false),
  chanOscTextFormat("%c"),
  chanOscColor(1.0f,1.0f,1.0f,1.0f),
  chanOscTextColor(1.0f,1.0f,1.0f,0.75f),
  chanOscGrad(64,64),
  chanOscGradTex(NULL),
  chanOscWorkPool(NULL),
  xyOscPointTex(NULL),
  xyOscOptions(false),
  xyOscXChannel(0),
  xyOscXInvert(false),
  xyOscYChannel(1),
  xyOscYInvert(false),
  xyOscZoom(1.0f),
  xyOscSamples(32768),
  xyOscDecayTime(10.0f),
  xyOscIntensity(2.0f),
  xyOscThickness(2.0f),
  followLog(true),
#ifdef IS_MOBILE
  pianoOctaves(7),
  pianoOctavesEdit(2),
  pianoOptions(true),
  pianoSharePosition(false),
  pianoOptionsSet(false),
  pianoReadonly(false),
  pianoOffset(6),
  pianoOffsetEdit(9),
  pianoView(PIANO_LAYOUT_AUTOMATIC),
  pianoInputPadMode(PIANO_INPUT_PAD_SPLIT_AUTO),
#else
  pianoOctaves(7),
  pianoOctavesEdit(4),
  pianoOptions(false),
  pianoSharePosition(true),
  pianoReadonly(false),
  pianoOffset(6),
  pianoOffsetEdit(6),
  pianoView(PIANO_LAYOUT_STANDARD),
  pianoInputPadMode(PIANO_INPUT_PAD_DISABLE),
#endif
  hasACED(false),
  waveGenBaseShape(0),
  waveInterpolation(0),
  waveGenDuty(0.5f),
  waveGenPower(1),
  waveGenInvertPoint(1.0f),
  waveGenScaleX(32),
  waveGenScaleY(32),
  waveGenOffsetX(0),
  waveGenOffsetY(0),
  waveGenSmooth(1),
  waveGenAmplify(1.0f),
  waveGenFM(false),
  introPos(0.0),
  introSkip(0.0),
  monitorPos(0.0),
  mustClear(2),
  initialScreenWipe(1.0f),
  introSkipDo(false),
  introStopped(false),
  curTutorial(-1),
  curTutorialStep(0),
  csDisAsmAddr(0),
  csExportThread(NULL),
  csExportResult(NULL),
  csExportTarget(false),
  csExportDone(false),
  dmfExportVersion(0),
  curExportType(GUI_EXPORT_NONE),
  romTarget(DIV_ROM_ABSTRACT),
  romMultiFile(false),
  romExportSave(false),
  pendingExport(NULL),
  romExportExists(false) {
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

  memset(willExport,1,DIV_MAX_CHIPS*sizeof(bool));

  memset(peak,0,DIV_MAX_OUTPUTS*sizeof(float));

  opMaskTransposeNote.note=true;
  opMaskTransposeNote.ins=false;
  opMaskTransposeNote.vol=false;
  opMaskTransposeNote.effect=false;
  opMaskTransposeNote.effectVal=false;

  opMaskTransposeValue.note=false;
  opMaskTransposeValue.ins=true;
  opMaskTransposeValue.vol=true;
  opMaskTransposeValue.effect=false;
  opMaskTransposeValue.effectVal=true;

  memset(patChanX,0,sizeof(float)*(DIV_MAX_CHANS+1));
  memset(patChanSlideY,0,sizeof(float)*(DIV_MAX_CHANS+1));
  memset(lastIns,-1,sizeof(int)*DIV_MAX_CHANS);
  memset(oscValues,0,sizeof(void*)*DIV_MAX_OUTPUTS);

  memset(chanOscLP0,0,sizeof(float)*DIV_MAX_CHANS);
  memset(chanOscLP1,0,sizeof(float)*DIV_MAX_CHANS);
  memset(chanOscVol,0,sizeof(float)*DIV_MAX_CHANS);
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    chanOscChan[i].pitch=0.0f;
  }
  memset(chanOscBright,0,sizeof(float)*DIV_MAX_CHANS);
  memset(lastCorrPos,0,sizeof(short)*DIV_MAX_CHANS);

  memset(acedData,0,23);

  memset(waveGenAmp,0,sizeof(float)*16);
  memset(waveGenPhase,0,sizeof(float)*16);
  waveGenTL[0]=0.0f;
  waveGenTL[1]=0.0f;
  waveGenTL[2]=0.0f;
  waveGenTL[3]=1.0f;
  fmWaveform[0]=0;
  fmWaveform[1]=0;
  fmWaveform[2]=0;
  fmWaveform[3]=0;
  waveGenMult[0]=1;
  waveGenMult[1]=1;
  waveGenMult[2]=1;
  waveGenMult[3]=1;
  memset(waveGenFB,0,sizeof(int)*4);
  memset(waveGenFMCon0,0,sizeof(bool)*5);
  memset(waveGenFMCon1,0,sizeof(bool)*5);
  memset(waveGenFMCon2,0,sizeof(bool)*5);
  memset(waveGenFMCon3, 0, sizeof(bool) * 5);
  memset(waveGenFMCon4,0,sizeof(bool)*5);

  waveGenAmp[0]=1.0f;
  waveGenFMCon0[0]=false;
  waveGenFMCon1[0]=true;
  waveGenFMCon2[1]=true;
  waveGenFMCon3[2]=true;
  waveGenFMCon4[0]=false;

  waveGenFMCon0[4]=false;
  waveGenFMCon1[4]=false;
  waveGenFMCon2[4]=false;
  waveGenFMCon3[4]=true;

  memset(keyHit,0,sizeof(float)*DIV_MAX_CHANS);
  memset(keyHit1,0,sizeof(float)*DIV_MAX_CHANS);

  memset(lastAudioLoads,0,sizeof(float)*120);

  memset(pianoKeyHit,0,sizeof(float)*180);
  memset(pianoKeyPressed,0,sizeof(bool)*180);

  memset(queryReplaceEffectMode,0,sizeof(int)*8);
  memset(queryReplaceEffectValMode,0,sizeof(int)*8);
  memset(queryReplaceEffect,0,sizeof(int)*8);
  memset(queryReplaceEffectVal,0,sizeof(int)*8);
  memset(queryReplaceEffectDo,0,sizeof(bool)*8);
  memset(queryReplaceEffectValDo,0,sizeof(bool)*8);

  chanOscGrad.bgColor=ImVec4(0.0f,0.0f,0.0f,1.0f);

  memset(noteOffLabel,0,32);
  memset(noteRelLabel,0,32);
  memset(macroRelLabel,0,32);
  memset(emptyLabel,0,32);
  memset(emptyLabel2,0,32);
  // effect sorting
  memset(effectsShow,1,sizeof(bool)*10);

  memset(romExportAvail,0,sizeof(bool)*DIV_ROM_MAX);

  songOrdersLengths.clear();

  strncpy(noteOffLabel,"OFF",32);
  strncpy(noteRelLabel,"===",32);
  strncpy(macroRelLabel,"REL",32);
  strncpy(emptyLabel,"...",32);
  strncpy(emptyLabel2,"..",32);
}
