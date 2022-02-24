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
#include "debug.h"
#include "fonts.h"
#include "icon.h"
#include "../ta-log.h"
#include "../fileutils.h"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include "imgui_internal.h"
#include "ImGuiFileDialog.h"
#include "IconsFontAwesome4.h"
#include "misc/cpp/imgui_stdlib.h"
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
#else
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#define LAYOUT_INI "/layout.ini"
#endif

bool Particle::update() {
  pos.x+=speed.x;
  pos.y+=speed.y;
  speed.x*=friction;
  speed.y*=friction;
  speed.y+=gravity;
  life-=lifeSpeed;
  return (life>0);
}

void FurnaceGUI::bindEngine(DivEngine* eng) {
  e=eng;
}

const char* noteNameNormal(short note, short octave) {
  if (note==100) { // note cut
    return "OFF";
  } else if (note==101) { // note off and envelope release
    return "===";
  } else if (note==102) { // envelope release only
    return "REL";
  } else if (octave==0 && note==0) {
    return "...";
  }
  int seek=(note+(signed char)octave*12)+60;
  if (seek<0 || seek>=180) {
    return "???";
  }
  return noteNames[seek];
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

void FurnaceGUI::encodeMMLStr(String& target, int* macro, int macroLen, int macroLoop, int macroRel) {
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

void FurnaceGUI::decodeMMLStrW(String& source, int* macro, int& macroLen, int macroMax) {
  int buf=0;
  bool negaBuf=false;
  bool hasVal=false;
  macroLen=0;
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
Pos=60,60\n\
Size=400,400\n\
Collapsed=0\n\
\n\
[Window][Play/Edit Controls]\n\
Pos=390,24\n\
Size=243,177\n\
Collapsed=0\n\
DockId=0x00000009,0\n\
\n\
[Window][Song Information]\n\
Pos=951,24\n\
Size=329,240\n\
Collapsed=0\n\
DockId=0x00000004,0\n\
\n\
[Window][Orders]\n\
Pos=0,24\n\
Size=388,240\n\
Collapsed=0\n\
DockId=0x00000005,0\n\
\n\
[Window][Instruments]\n\
Pos=635,24\n\
Size=314,240\n\
Collapsed=0\n\
DockId=0x00000008,1\n\
\n\
[Window][Wavetables]\n\
Pos=635,24\n\
Size=314,240\n\
Collapsed=0\n\
DockId=0x00000008,2\n\
\n\
[Window][Samples]\n\
Pos=635,24\n\
Size=314,240\n\
Collapsed=0\n\
DockId=0x00000008,0\n\
\n\
[Window][Pattern]\n\
Pos=0,266\n\
Size=1246,489\n\
Collapsed=0\n\
DockId=0x0000000B,0\n\
\n\
[Window][Open File##FileDialog]\n\
Pos=213,99\n\
Size=853,557\n\
Collapsed=0\n\
\n\
[Window][Instrument Editor]\n\
Pos=324,130\n\
Size=951,623\n\
Collapsed=0\n\
\n\
[Window][Warning]\n\
Pos=516,339\n\
Size=346,71\n\
Collapsed=0\n\
\n\
[Window][Load Sample##FileDialog]\n\
Pos=340,177\n\
Size=600,400\n\
Collapsed=0\n\
\n\
[Window][Sample Editor]\n\
Pos=238,298\n\
Size=551,286\n\
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
[Window][Load Wavetable##FileDialog]\n\
Pos=340,177\n\
Size=600,400\n\
Collapsed=0\n\
\n\
[Window][Wavetable Editor]\n\
Pos=228,81\n\
Size=580,368\n\
Collapsed=0\n\
\n\
[Window][Save Wavetable##FileDialog]\n\
Pos=340,177\n\
Size=600,400\n\
Collapsed=0\n\
\n\
[Window][Settings]\n\
Pos=495,97\n\
Size=552,559\n\
Collapsed=0\n\
\n\
[Window][Error]\n\
Pos=488,342\n\
Size=304,71\n\
Collapsed=0\n\
\n\
[Window][Export VGM##FileDialog]\n\
Pos=340,177\n\
Size=600,400\n\
Collapsed=0\n\
\n\
[Window][Mixer]\n\
Pos=60,60\n\
Size=450,215\n\
Collapsed=0\n\
\n\
[Window][Oscilloscope]\n\
Pos=390,203\n\
Size=243,61\n\
Collapsed=0\n\
DockId=0x0000000A,0\n\
\n\
[Window][Volume Meter]\n\
Pos=1248,266\n\
Size=32,489\n\
Collapsed=0\n\
DockId=0x0000000C,0\n\
\n\
[Window][Debug]\n\
Pos=38,96\n\
Size=1243,574\n\
Collapsed=0\n\
\n\
[Docking][Data]\n\
DockSpace           ID=0x8B93E3BD Window=0xA787BDB4 Pos=0,24 Size=1280,731 Split=Y Selected=0x6C01C512\n\
  DockNode          ID=0x00000001 Parent=0x8B93E3BD SizeRef=1280,240 Split=X Selected=0xF3094A52\n\
    DockNode        ID=0x00000003 Parent=0x00000001 SizeRef=949,231 Split=X Selected=0x65CC51DC\n\
      DockNode      ID=0x00000005 Parent=0x00000003 SizeRef=388,231 Selected=0xE283F8D8\n\
      DockNode      ID=0x00000006 Parent=0x00000003 SizeRef=559,231 Split=X Selected=0x756E3877\n\
        DockNode    ID=0x00000007 Parent=0x00000006 SizeRef=243,231 Split=Y Selected=0xD2BA8AA2\n\
          DockNode  ID=0x00000009 Parent=0x00000007 SizeRef=220,177 Selected=0xD2BA8AA2\n\
          DockNode  ID=0x0000000A Parent=0x00000007 SizeRef=220,61 HiddenTabBar=1 Selected=0x608FDEB4\n\
        DockNode    ID=0x00000008 Parent=0x00000006 SizeRef=314,231 Selected=0xD62F6EEB\n\
    DockNode        ID=0x00000004 Parent=0x00000001 SizeRef=329,231 Selected=0xF3094A52\n\
  DockNode          ID=0x00000002 Parent=0x8B93E3BD SizeRef=1280,489 Split=X Selected=0x6C01C512\n\
    DockNode        ID=0x0000000B Parent=0x00000002 SizeRef=1246,503 CentralNode=1 Selected=0x6C01C512\n\
    DockNode        ID=0x0000000C Parent=0x00000002 SizeRef=32,503 HiddenTabBar=1 Selected=0xD67E3EB0\n\
";

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

void FurnaceGUI::drawEditControls() {
  if (nextWindow==GUI_WINDOW_EDIT_CONTROLS) {
    editControlsOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!editControlsOpen) return;
  switch (settings.controlLayout) {
    case 0: // classic
      if (ImGui::Begin("Play/Edit Controls",&editControlsOpen)) {
        ImGui::Text("Octave");
        ImGui::SameLine();
        if (ImGui::InputInt("##Octave",&curOctave,1,1)) {
          if (curOctave>7) curOctave=7;
          if (curOctave<-5) curOctave=-5;
          for (size_t i=0; i<activeNotes.size(); i++) {
            e->noteOff(activeNotes[i].chan);
          }
          activeNotes.clear();
        }

        ImGui::Text("Edit Step");
        ImGui::SameLine();
        if (ImGui::InputInt("##EditStep",&editStep,1,1)) {
          if (editStep>=e->song.patLen) editStep=e->song.patLen-1;
          if (editStep<0) editStep=0;
        }

        if (ImGui::Button(ICON_FA_PLAY "##Play")) {
          play();
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_STOP "##Stop")) {
          stop();
        }
        ImGui::SameLine();
        ImGui::Checkbox("Edit",&edit);
        ImGui::SameLine();
        bool metro=e->getMetronome();
        if (ImGui::Checkbox("Metronome",&metro)) {
          e->setMetronome(metro);
        }

        ImGui::Text("Follow");
        ImGui::SameLine();
        ImGui::Checkbox("Orders",&followOrders);
        ImGui::SameLine();
        ImGui::Checkbox("Pattern",&followPattern);

        bool repeatPattern=e->getRepeatPattern();
        if (ImGui::Checkbox("Repeat pattern",&repeatPattern)) {
          e->setRepeatPattern(repeatPattern);
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_ARROW_DOWN "##StepOne")) {
          e->stepOne(cursor.y);
        }
      }
      if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_EDIT_CONTROLS;
      ImGui::End();
      break;
    case 1: // compact
      if (ImGui::Begin("Play/Edit Controls",&editControlsOpen,ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse)) {
        if (ImGui::Button(ICON_FA_STOP "##Stop")) {
          stop();
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_PLAY "##Play")) {
          play();
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_ARROW_DOWN "##StepOne")) {
          e->stepOne(cursor.y);
        }

        ImGui::SameLine();
        bool repeatPattern=e->getRepeatPattern();
        ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(repeatPattern)?0.6f:0.2f,0.2f,1.0f));
        if (ImGui::Button(ICON_FA_REPEAT "##RepeatPattern")) {
          e->setRepeatPattern(!repeatPattern);
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(edit)?0.6f:0.2f,0.2f,1.0f));
        if (ImGui::Button(ICON_FA_CIRCLE "##Edit")) {
          edit=!edit;
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();
        bool metro=e->getMetronome();
        ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(metro)?0.6f:0.2f,0.2f,1.0f));
        if (ImGui::Button(ICON_FA_BELL_O "##Metronome")) {
          e->setMetronome(!metro);
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();
        ImGui::Text("Octave");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(96.0f*dpiScale);
        if (ImGui::InputInt("##Octave",&curOctave,1,1)) {
          if (curOctave>7) curOctave=7;
          if (curOctave<-5) curOctave=-5;
          for (size_t i=0; i<activeNotes.size(); i++) {
            e->noteOff(activeNotes[i].chan);
          }
          activeNotes.clear();
        }

        ImGui::SameLine();
        ImGui::Text("Edit Step");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(96.0f*dpiScale);
        if (ImGui::InputInt("##EditStep",&editStep,1,1)) {
          if (editStep>=e->song.patLen) editStep=e->song.patLen-1;
          if (editStep<0) editStep=0;
        }

        ImGui::SameLine();
        ImGui::Text("Follow");
        ImGui::SameLine();
        ImGui::Checkbox("Orders",&followOrders);
        ImGui::SameLine();
        ImGui::Checkbox("Pattern",&followPattern);
      }
      if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_EDIT_CONTROLS;
      ImGui::End();
      break;
    case 2: // compact vertical
      if (ImGui::Begin("Play/Edit Controls",&editControlsOpen,ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse)) {
        if (ImGui::Button(ICON_FA_PLAY "##Play")) {
          play();
        }
        if (ImGui::Button(ICON_FA_STOP "##Stop")) {
          stop();
        }
        if (ImGui::Button(ICON_FA_ARROW_DOWN "##StepOne")) {
          e->stepOne(cursor.y);
        }

        bool repeatPattern=e->getRepeatPattern();
        ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(repeatPattern)?0.6f:0.2f,0.2f,1.0f));
        if (ImGui::Button(ICON_FA_REPEAT "##RepeatPattern")) {
          e->setRepeatPattern(!repeatPattern);
        }
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(edit)?0.6f:0.2f,0.2f,1.0f));
        if (ImGui::Button(ICON_FA_CIRCLE "##Edit")) {
          edit=!edit;
        }
        ImGui::PopStyleColor();

        bool metro=e->getMetronome();
        ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(metro)?0.6f:0.2f,0.2f,1.0f));
        if (ImGui::Button(ICON_FA_BELL_O "##Metronome")) {
          e->setMetronome(!metro);
        }
        ImGui::PopStyleColor();

        ImGui::Text("Oct.");
        float avail=ImGui::GetContentRegionAvail().x;
        ImGui::SetNextItemWidth(avail);
        if (ImGui::InputInt("##Octave",&curOctave,0,0)) {
          if (curOctave>7) curOctave=7;
          if (curOctave<-5) curOctave=-5;
          for (size_t i=0; i<activeNotes.size(); i++) {
            e->noteOff(activeNotes[i].chan);
          }
          activeNotes.clear();
        }

        ImGui::Text("Step");
        ImGui::SetNextItemWidth(avail);
        if (ImGui::InputInt("##EditStep",&editStep,0,0)) {
          if (editStep>=e->song.patLen) editStep=e->song.patLen-1;
          if (editStep<0) editStep=0;
        }

        ImGui::Text("Foll.");
        ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(followOrders)?0.6f:0.2f,0.2f,1.0f));
        if (ImGui::SmallButton("Ord##FollowOrders")) {
          followOrders=!followOrders;
        }
        ImGui::PopStyleColor();
        ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(followPattern)?0.6f:0.2f,0.2f,1.0f));
        if (ImGui::SmallButton("Pat##FollowPattern")) {
          followPattern=!followPattern;
        }
        ImGui::PopStyleColor();
      }
      if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_EDIT_CONTROLS;
      ImGui::End();
      break;
    case 3: // split
      if (ImGui::Begin("Play Controls",&editControlsOpen,ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse)) {
        if (e->isPlaying()) {
          if (ImGui::Button(ICON_FA_STOP "##Stop")) {
            stop();
          }
        } else {
          if (ImGui::Button(ICON_FA_PLAY "##Play")) {
            play();
          }
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_PLAY_CIRCLE "##PlayAgain")) {
          play();
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_ARROW_DOWN "##StepOne")) {
          e->stepOne(cursor.y);
        }

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(edit)?0.6f:0.2f,0.2f,1.0f));
        if (ImGui::Button(ICON_FA_CIRCLE "##Edit")) {
          edit=!edit;
        }
        ImGui::PopStyleColor();

        bool metro=e->getMetronome();
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(metro)?0.6f:0.2f,0.2f,1.0f));
        if (ImGui::Button(ICON_FA_BELL_O "##Metronome")) {
          e->setMetronome(!metro);
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();
        bool repeatPattern=e->getRepeatPattern();
        ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(repeatPattern)?0.6f:0.2f,0.2f,1.0f));
        if (ImGui::Button(ICON_FA_REPEAT "##RepeatPattern")) {
          e->setRepeatPattern(!repeatPattern);
        }
        ImGui::PopStyleColor();
      }
      if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_EDIT_CONTROLS;
      ImGui::End();

      if (ImGui::Begin("Edit Controls",&editControlsOpen)) {
        ImGui::Columns(2);
        ImGui::Text("Octave");
        ImGui::SameLine();
        float cursor=ImGui::GetCursorPosX();
        float avail=ImGui::GetContentRegionAvail().x;
        ImGui::SetNextItemWidth(avail);
        if (ImGui::InputInt("##Octave",&curOctave,1,1)) {
          if (curOctave>7) curOctave=7;
          if (curOctave<-5) curOctave=-5;
          for (size_t i=0; i<activeNotes.size(); i++) {
            e->noteOff(activeNotes[i].chan);
          }
          activeNotes.clear();
        }

        ImGui::Text("Step");
        ImGui::SameLine();
        ImGui::SetCursorPosX(cursor);
        ImGui::SetNextItemWidth(avail);
        if (ImGui::InputInt("##EditStep",&editStep,1,1)) {
          if (editStep>=e->song.patLen) editStep=e->song.patLen-1;
          if (editStep<0) editStep=0;
        }
        ImGui::NextColumn();

        ImGui::Checkbox("Follow orders",&followOrders);
        ImGui::Checkbox("Follow pattern",&followPattern);
      }
      if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_EDIT_CONTROLS;
      ImGui::End();
      break;
  }
}

void FurnaceGUI::drawSongInfo() {
  if (nextWindow==GUI_WINDOW_SONG_INFO) {
    songInfoOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!songInfoOpen) return;
  if (ImGui::Begin("Song Information",&songInfoOpen)) {
    if (ImGui::BeginTable("NameAuthor",2,ImGuiTableFlags_SizingStretchProp)) {
      ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed,0.0);
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0);
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("Name");
      ImGui::TableNextColumn();
      float avail=ImGui::GetContentRegionAvail().x;
      ImGui::SetNextItemWidth(avail);
      if (ImGui::InputText("##Name",&e->song.name)) updateWindowTitle();
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("Author");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      ImGui::InputText("##Author",&e->song.author);
      ImGui::EndTable();
    }

    if (ImGui::BeginTable("OtherProps",3,ImGuiTableFlags_SizingStretchProp)) {
      ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed,0.0);
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0);
      ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.0);

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("TimeBase");
      ImGui::TableNextColumn();
      float avail=ImGui::GetContentRegionAvail().x;
      ImGui::SetNextItemWidth(avail);
      unsigned char realTB=e->song.timeBase+1;
      if (ImGui::InputScalar("##TimeBase",ImGuiDataType_U8,&realTB,&_ONE,&_THREE)) {
        if (realTB<1) realTB=1;
        if (realTB>16) realTB=16;
        e->song.timeBase=realTB-1;
      }
      ImGui::TableNextColumn();
      float hl=e->song.hilightA;
      if (hl<=0.0f) hl=4.0f;
      float timeBase=e->song.timeBase+1;
      float speedSum=e->song.speed1+e->song.speed2;
      if (timeBase<1.0f) timeBase=1.0f;
      if (speedSum<1.0f) speedSum=1.0f;
      ImGui::Text("%.2f BPM",120.0f*(float)e->song.hz/(timeBase*hl*speedSum));

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("Speed");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      if (ImGui::InputScalar("##Speed1",ImGuiDataType_U8,&e->song.speed1,&_ONE,&_THREE)) {
        if (e->song.speed1<1) e->song.speed1=1;
        if (e->isPlaying()) play();
      }
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      if (ImGui::InputScalar("##Speed2",ImGuiDataType_U8,&e->song.speed2,&_ONE,&_THREE)) {
        if (e->song.speed2<1) e->song.speed2=1;
        if (e->isPlaying()) play();
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("Highlight");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      ImGui::InputScalar("##Highlight1",ImGuiDataType_U8,&e->song.hilightA,&_ONE,&_THREE);
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      ImGui::InputScalar("##Highlight2",ImGuiDataType_U8,&e->song.hilightB,&_ONE,&_THREE);

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("Pattern Length");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      int patLen=e->song.patLen;
      if (ImGui::InputInt("##PatLength",&patLen,1,3)) {
        if (patLen<1) patLen=1;
        if (patLen>256) patLen=256;
        e->song.patLen=patLen;
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("Song Length");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      int ordLen=e->song.ordersLen;
      if (ImGui::InputInt("##OrdLength",&ordLen,1,3)) {
        if (ordLen<1) ordLen=1;
        if (ordLen>127) ordLen=127;
        e->song.ordersLen=ordLen;
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("Tick Rate");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      int setHz=e->song.hz;
      if (ImGui::InputInt("##Rate",&setHz)) {
        if (setHz<10) setHz=10;
        if (setHz>999) setHz=999;
        e->setSongRate(setHz,setHz<52);
      }
      if (e->song.hz==50) {
        ImGui::TableNextColumn();
        ImGui::Text("PAL");
      }
      if (e->song.hz==60) {
        ImGui::TableNextColumn();
        ImGui::Text("NTSC");
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("Tuning (A-4)");
      ImGui::TableNextColumn();
      float tune=e->song.tuning;
      ImGui::SetNextItemWidth(avail);
      if (ImGui::InputFloat("##Tuning",&tune,1.0f,3.0f,"%g")) {
        if (tune<220.0f) tune=220.0f;
        if (tune>880.0f) tune=880.0f;
        e->song.tuning=tune;
      }
      ImGui::EndTable();
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_SONG_INFO;
  ImGui::End();
}

void FurnaceGUI::drawInsList() {
  if (nextWindow==GUI_WINDOW_INS_LIST) {
    insListOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!insListOpen) return;
  if (ImGui::Begin("Instruments",&insListOpen)) {
    if (ImGui::Button(ICON_FA_PLUS "##InsAdd")) {
      doAction(GUI_ACTION_INS_LIST_ADD);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FILES_O "##InsClone")) {
      doAction(GUI_ACTION_INS_LIST_DUPLICATE);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FOLDER_OPEN "##InsLoad")) {
      doAction(GUI_ACTION_INS_LIST_OPEN);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FLOPPY_O "##InsSave")) {
      doAction(GUI_ACTION_INS_LIST_SAVE);
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("InsUp",ImGuiDir_Up)) {
      doAction(GUI_ACTION_INS_LIST_MOVE_UP);
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("InsDown",ImGuiDir_Down)) {
      doAction(GUI_ACTION_INS_LIST_MOVE_DOWN);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_TIMES "##InsDelete")) {
      doAction(GUI_ACTION_INS_LIST_DELETE);
    }
    ImGui::Separator();
    if (ImGui::BeginTable("InsListScroll",1,ImGuiTableFlags_ScrollY)) {
      for (int i=0; i<(int)e->song.ins.size(); i++) {
        DivInstrument* ins=e->song.ins[i];
        String name;
        switch (ins->type) {
          case DIV_INS_FM:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_FM]);
            name=fmt::sprintf(ICON_FA_AREA_CHART " %.2X: %s##_INS%d\n",i,ins->name,i);
            break;
          case DIV_INS_STD:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_STD]);
            name=fmt::sprintf(ICON_FA_BAR_CHART " %.2X: %s##_INS%d\n",i,ins->name,i);
            break;
          case DIV_INS_GB:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_GB]);
            name=fmt::sprintf(ICON_FA_GAMEPAD " %.2X: %s##_INS%d\n",i,ins->name,i);
            break;
          case DIV_INS_C64:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_C64]);
            name=fmt::sprintf(ICON_FA_KEYBOARD_O " %.2X: %s##_INS%d\n",i,ins->name,i);
            break;
          case DIV_INS_AMIGA:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_AMIGA]);
            name=fmt::sprintf(ICON_FA_VOLUME_UP " %.2X: %s##_INS%d\n",i,ins->name,i);
            break;
          case DIV_INS_PCE:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_PCE]);
            name=fmt::sprintf(ICON_FA_ID_BADGE " %.2X: %s##_INS%d\n",i,ins->name,i);
            break;
          case DIV_INS_AY:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_AY]);
            name=fmt::sprintf(ICON_FA_BAR_CHART " %.2X: %s##_INS%d\n",i,ins->name,i);
            break;
          case DIV_INS_AY8930:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_AY8930]);
            name=fmt::sprintf(ICON_FA_BAR_CHART " %.2X: %s##_INS%d\n",i,ins->name,i);
            break;
          case DIV_INS_TIA:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_TIA]);
            name=fmt::sprintf(ICON_FA_BAR_CHART " %.2X: %s##_INS%d\n",i,ins->name,i);
            break;
          case DIV_INS_SAA1099:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_SAA1099]);
            name=fmt::sprintf(ICON_FA_BAR_CHART " %.2X: %s##_INS%d\n",i,ins->name,i);
            break;
          case DIV_INS_VIC:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_VIC]);
            name=fmt::sprintf(ICON_FA_BAR_CHART " %.2X: %s##_INS%d\n",i,ins->name,i);
            break;
          case DIV_INS_PET:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_PET]);
            name=fmt::sprintf(ICON_FA_SQUARE " %.2X: %s##_INS%d\n",i,ins->name,i);
            break;
          case DIV_INS_VRC6:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_VRC6]);
            name=fmt::sprintf(ICON_FA_BAR_CHART " %.2X: %s##_INS%d\n",i,ins->name,i);
            break;
          case DIV_INS_OPLL:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_OPLL]);
            name=fmt::sprintf(ICON_FA_AREA_CHART " %.2X: %s##_INS%d\n",i,ins->name,i);
            break;
          case DIV_INS_OPL:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_OPL]);
            name=fmt::sprintf(ICON_FA_AREA_CHART " %.2X: %s##_INS%d\n",i,ins->name,i);
            break;
          case DIV_INS_FDS:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_FDS]);
            name=fmt::sprintf(ICON_FA_FLOPPY_O " %.2X: %s##_INS%d\n",i,ins->name,i);
            break;
          case DIV_INS_VBOY:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_VBOY]);
            name=fmt::sprintf(ICON_FA_BINOCULARS " %.2X: %s##_INS%d\n",i,ins->name,i);
            break;
          case DIV_INS_N163:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_N163]);
            name=fmt::sprintf(ICON_FA_CALCULATOR " %.2X: %s##_INS%d\n",i,ins->name,i);
            break;
          case DIV_INS_SCC:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_SCC]);
            name=fmt::sprintf(ICON_FA_CALCULATOR " %.2X: %s##_INS%d\n",i,ins->name,i);
            break;
          case DIV_INS_OPZ:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_OPZ]);
            name=fmt::sprintf(ICON_FA_AREA_CHART " %.2X: %s##_INS%d\n",i,ins->name,i);
            break;
          case DIV_INS_POKEY:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_POKEY]);
            name=fmt::sprintf(ICON_FA_BAR_CHART " %.2X: %s##_INS%d\n",i,ins->name,i);
            break;
          case DIV_INS_BEEPER:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_BEEPER]);
            name=fmt::sprintf(ICON_FA_SQUARE " %.2X: %s##_INS%d\n",i,ins->name,i);
            break;
          case DIV_INS_SWAN:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_SWAN]);
            name=fmt::sprintf(ICON_FA_GAMEPAD " %.2X: %s##_INS%d\n",i,ins->name,i);
            break;
          case DIV_INS_MIKEY:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_MIKEY]);
            name=fmt::sprintf(ICON_FA_BAR_CHART " %.2X: %s##_INS%d\n",i,ins->name,i);
            break;
          default:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_UNKNOWN]);
            name=fmt::sprintf(ICON_FA_QUESTION " %.2X: %s##_INS%d\n",i,ins->name,i);
            break;
        }
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Selectable(name.c_str(),curIns==i)) {
          curIns=i;
        }
        ImGui::PopStyleColor();
        if (ImGui::IsItemHovered()) {
          if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            insEditOpen=true;
          }
        }
      }
      ImGui::EndTable();
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_INS_LIST;
  ImGui::End();
}

const char* sampleNote[12]={
  "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

void FurnaceGUI::drawSampleList() {
  if (nextWindow==GUI_WINDOW_SAMPLE_LIST) {
    sampleListOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!sampleListOpen) return;
  if (ImGui::Begin("Samples",&sampleListOpen)) {
    if (ImGui::Button(ICON_FA_PLUS "##SampleAdd")) {
      doAction(GUI_ACTION_SAMPLE_LIST_ADD);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FOLDER_OPEN "##SampleLoad")) {
      doAction(GUI_ACTION_SAMPLE_LIST_OPEN);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FLOPPY_O "##SampleSave")) {
      doAction(GUI_ACTION_SAMPLE_LIST_SAVE);
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("SampleUp",ImGuiDir_Up)) {
      doAction(GUI_ACTION_SAMPLE_LIST_MOVE_UP);
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("SampleDown",ImGuiDir_Down)) {
      doAction(GUI_ACTION_SAMPLE_LIST_MOVE_DOWN);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_TIMES "##SampleDelete")) {
      doAction(GUI_ACTION_SAMPLE_LIST_DELETE);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_VOLUME_UP "##PreviewSampleL")) {
      doAction(GUI_ACTION_SAMPLE_LIST_PREVIEW);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_VOLUME_OFF "##StopSampleL")) {
      doAction(GUI_ACTION_SAMPLE_LIST_STOP_PREVIEW);
    }
    ImGui::Separator();
    if (ImGui::BeginTable("SampleListScroll",1,ImGuiTableFlags_ScrollY)) {
      for (int i=0; i<(int)e->song.sample.size(); i++) {
        DivSample* sample=e->song.sample[i];
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if ((i%12)==0) {
          if (i>0) ImGui::Unindent();
          ImGui::Text("Bank %d",i/12);
          ImGui::Indent();
        }
        if (ImGui::Selectable(fmt::sprintf("%s: %s##_SAM%d",sampleNote[i%12],sample->name,i).c_str(),curSample==i)) {
          curSample=i;
        }
        if (ImGui::IsItemHovered()) {
          if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            sampleEditOpen=true;
          }
        }
      }
      ImGui::EndTable();
    }
    ImGui::Unindent();
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_SAMPLE_LIST;
  ImGui::End();
}

void FurnaceGUI::drawSampleEdit() {
  if (nextWindow==GUI_WINDOW_SAMPLE_EDIT) {
    sampleEditOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!sampleEditOpen) return;
  if (ImGui::Begin("Sample Editor",&sampleEditOpen,settings.allowEditDocking?0:ImGuiWindowFlags_NoDocking)) {
    if (curSample<0 || curSample>=(int)e->song.sample.size()) {
      ImGui::Text("no sample selected");
    } else {
      DivSample* sample=e->song.sample[curSample];
      ImGui::InputText("Name",&sample->name);
      ImGui::Text("Length: %d",sample->length);
      if (ImGui::InputInt("Rate (Hz)",&sample->rate,10,200)) {
        if (sample->rate<100) sample->rate=100;
        if (sample->rate>96000) sample->rate=96000;
      }
      if (ImGui::InputInt("Pitch of C-4 (Hz)",&sample->centerRate,10,200)) {
        if (sample->centerRate<100) sample->centerRate=100;
        if (sample->centerRate>65535) sample->centerRate=65535;
      }
      ImGui::Text("effective rate: %dHz",e->getEffectiveSampleRate(sample->rate));
      bool doLoop=(sample->loopStart>=0);
      if (ImGui::Checkbox("Loop",&doLoop)) {
        if (doLoop) {
          sample->loopStart=0;
        } else {
          sample->loopStart=-1;
        }
      }
      if (doLoop) {
        ImGui::SameLine();
        if (ImGui::InputInt("##LoopPosition",&sample->loopStart,1,10)) {
          if (sample->loopStart<0 || sample->loopStart>=sample->length) {
            sample->loopStart=0;
          }
        }
      }
      if (ImGui::SliderScalar("Volume",ImGuiDataType_S8,&sample->vol,&_ZERO,&_ONE_HUNDRED,fmt::sprintf("%d%%%%",sample->vol*2).c_str())) {
        if (sample->vol<0) sample->vol=0;
        if (sample->vol>100) sample->vol=100;
      }
      if (ImGui::SliderScalar("Pitch",ImGuiDataType_S8,&sample->pitch,&_ZERO,&_TEN,pitchLabel[sample->pitch])) {
        if (sample->pitch<0) sample->pitch=0;
        if (sample->pitch>10) sample->pitch=10;
      }
      if (ImGui::Button("Apply")) {
        e->renderSamplesP();
      }
      ImGui::SameLine();
      if (ImGui::Button(ICON_FA_VOLUME_UP "##PreviewSample")) {
        e->previewSample(curSample);
      }
      ImGui::SameLine();
      if (ImGui::Button(ICON_FA_VOLUME_OFF "##StopSample")) {
        e->stopSamplePreview();
      }
      ImGui::Separator();
      bool considerations=false;
      ImGui::Text("notes:");
      if (sample->loopStart>=0) {
        considerations=true;
        ImGui::Text("- sample won't loop on Neo Geo ADPCM");
        if (sample->loopStart&1) {
          ImGui::Text("- sample loop start will be aligned to the nearest even sample on Amiga");
        }
      }
      if (sample->length&1) {
        considerations=true;
        ImGui::Text("- sample length will be aligned to the nearest even sample on Amiga");
      }
      if (sample->length>65535) {
        considerations=true;
        ImGui::Text("- maximum sample length on Sega PCM is 65536 samples");
      }
      if (sample->length>2097151) {
        considerations=true;
        ImGui::Text("- maximum sample length on Neo Geo ADPCM is 2097152 samples");
      }
      if (!considerations) {
        ImGui::Text("- none");
      }
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_SAMPLE_EDIT;
  ImGui::End();
}

void FurnaceGUI::drawMixer() {
  if (nextWindow==GUI_WINDOW_MIXER) {
    mixerOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!mixerOpen) return;
  ImGui::SetNextWindowSizeConstraints(ImVec2(400.0f*dpiScale,200.0f*dpiScale),ImVec2(scrW*dpiScale,scrH*dpiScale));
  if (ImGui::Begin("Mixer",&mixerOpen,settings.allowEditDocking?0:ImGuiWindowFlags_NoDocking)) {
    char id[32];
    for (int i=0; i<e->song.systemLen; i++) {
      snprintf(id,31,"MixS%d",i);
      bool doInvert=e->song.systemVol[i]&128;
      signed char vol=e->song.systemVol[i]&127;
      ImGui::PushID(id);
      ImGui::Text("%d. %s",i+1,getSystemName(e->song.system[i]));
      if (ImGui::SliderScalar("Volume",ImGuiDataType_S8,&vol,&_ZERO,&_ONE_HUNDRED_TWENTY_SEVEN)) {
        e->song.systemVol[i]=(e->song.systemVol[i]&128)|vol;
      }
      ImGui::SliderScalar("Panning",ImGuiDataType_S8,&e->song.systemPan[i],&_MINUS_ONE_HUNDRED_TWENTY_SEVEN,&_ONE_HUNDRED_TWENTY_SEVEN);
      if (ImGui::Checkbox("Invert",&doInvert)) {
        e->song.systemVol[i]^=128;
      }
      ImGui::PopID();
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_MIXER;
  ImGui::End();
}

void FurnaceGUI::drawOsc() {
  if (nextWindow==GUI_WINDOW_OSCILLOSCOPE) {
    oscOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!oscOpen) return;
  ImGui::SetNextWindowSizeConstraints(ImVec2(64.0f*dpiScale,32.0f*dpiScale),ImVec2(scrW*dpiScale,scrH*dpiScale));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,ImVec2(0,0));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,ImVec2(0,0));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing,ImVec2(0,0));
  if (ImGui::Begin("Oscilloscope",&oscOpen)) {
    float values[512];
    for (int i=0; i<512; i++) {
      int pos=i*e->oscSize/512;
      values[i]=(e->oscBuf[0][pos]+e->oscBuf[1][pos])*0.5f;
    }
    //ImGui::SetCursorPos(ImVec2(0,0));
    ImGui::BeginDisabled();
    ImGui::PlotLines("##SingleOsc",values,512,0,NULL,-1.0f,1.0f,ImGui::GetContentRegionAvail());
    ImGui::EndDisabled();
  }
  ImGui::PopStyleVar(3);
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_OSCILLOSCOPE;
  ImGui::End();
}

void FurnaceGUI::drawVolMeter() {
  if (nextWindow==GUI_WINDOW_VOL_METER) {
    volMeterOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!volMeterOpen) return;
  if (--isClipping<0) isClipping=0;
  ImGui::SetNextWindowSizeConstraints(ImVec2(6.0f*dpiScale,6.0f*dpiScale),ImVec2(scrW*dpiScale,scrH*dpiScale));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,ImVec2(0,0));
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(0,0));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,ImVec2(0,0));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing,ImVec2(0,0));
  if (ImGui::Begin("Volume Meter",&volMeterOpen)) {
    ImDrawList* dl=ImGui::GetWindowDrawList();
    bool aspectRatio=(ImGui::GetWindowSize().x/ImGui::GetWindowSize().y)>1.0;

    ImVec2 minArea=ImVec2(
      ImGui::GetWindowPos().x+ImGui::GetCursorPos().x,
      ImGui::GetWindowPos().y+ImGui::GetCursorPos().y
    );
    ImVec2 maxArea=ImVec2(
      ImGui::GetWindowPos().x+ImGui::GetCursorPos().x+ImGui::GetContentRegionAvail().x,
      ImGui::GetWindowPos().y+ImGui::GetCursorPos().y+ImGui::GetContentRegionAvail().y
    );
    ImRect rect=ImRect(minArea,maxArea);
    ImGuiStyle& style=ImGui::GetStyle();
    ImGui::ItemSize(ImVec2(4.0f,4.0f),style.FramePadding.y);
    ImU32 lowColor=ImGui::GetColorU32(uiColors[GUI_COLOR_VOLMETER_LOW]);
    if (ImGui::ItemAdd(rect,ImGui::GetID("volMeter"))) {
      ImGui::RenderFrame(rect.Min,rect.Max,ImGui::GetColorU32(ImGuiCol_FrameBg),true,style.FrameRounding);
      for (int i=0; i<2; i++) {
        peak[i]*=0.95;
        if (peak[i]<0.0001) peak[i]=0.0;
        for (int j=0; j<e->oscSize; j++) {
          if (fabs(e->oscBuf[i][j])>peak[i]) {
            peak[i]=fabs(e->oscBuf[i][j]);
          }
        }
        float logPeak=(20*log10(peak[i])/36.0);
        if (logPeak==NAN) logPeak=0.0;
        if (logPeak<-1.0) logPeak=-1.0;
        if (logPeak>0.0) {
          isClipping=8;
          logPeak=0.0;
        }
        logPeak+=1.0;
        ImU32 highColor=ImGui::GetColorU32(
          ImLerp(uiColors[GUI_COLOR_VOLMETER_LOW],uiColors[GUI_COLOR_VOLMETER_HIGH],logPeak)
        );
        ImRect s;
        if (aspectRatio) {
          s=ImRect(
            ImLerp(rect.Min,rect.Max,ImVec2(0,float(i)*0.5)),
            ImLerp(rect.Min,rect.Max,ImVec2(logPeak,float(i+1)*0.5))
          );
          if (i==0) s.Max.y-=dpiScale;
          if (isClipping) {
            dl->AddRectFilled(s.Min,s.Max,ImGui::GetColorU32(uiColors[GUI_COLOR_VOLMETER_PEAK]));
          } else {
            dl->AddRectFilledMultiColor(s.Min,s.Max,lowColor,highColor,highColor,lowColor);
          }
        } else {
          s=ImRect(
            ImLerp(rect.Min,rect.Max,ImVec2(float(i)*0.5,1.0-logPeak)),
            ImLerp(rect.Min,rect.Max,ImVec2(float(i+1)*0.5,1.0))
          );
          if (i==0) s.Max.x-=dpiScale;
          if (isClipping) {
            dl->AddRectFilled(s.Min,s.Max,ImGui::GetColorU32(uiColors[GUI_COLOR_VOLMETER_PEAK]));
          } else {
            dl->AddRectFilledMultiColor(s.Min,s.Max,highColor,highColor,lowColor,lowColor);
          }
        }
      }
      if (ImGui::IsItemHovered()) {
        if (aspectRatio) {
          ImGui::SetTooltip("%.1fdB",36*((ImGui::GetMousePos().x-ImGui::GetItemRectMin().x)/(rect.Max.x-rect.Min.x)-1.0));
        } else {
          ImGui::SetTooltip("%.1fdB",-(36+36*((ImGui::GetMousePos().y-ImGui::GetItemRectMin().y)/(rect.Max.y-rect.Min.y)-1.0)));
        }
      }
    }
  }
  ImGui::PopStyleVar(4);
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_VOL_METER;
  ImGui::End();
}

const char* aboutLine[]={
  "tildearrow",
  "is proud to present",
  "",
  ("Furnace " DIV_VERSION),
  "",
  "the free software chiptune tracker,",
  "compatible with DefleMask modules.",
  "",
  "zero disassembly.",
  "just clean-room design,",
  "time and dedication.",
  "",
  "> CREDITS <",
  "",
  "-- program --",
  "tildearrow",
  "laoo",
  "superctr",
  "",
  "-- graphics/UI design --",
  "tildearrow",
  "BlastBrothers",
  "",
  "-- documentation --",
  "tildearrow",
  "freq-mod",
  "nicco1690",
  "DeMOSic",
  "cam900",
  "",
  "-- demo songs --",
  "0x5066",
  "breakthetargets",
  "kleeder",
  "nicco1690",
  "NikonTeen",
  "SuperJet Spade",
  "TheDuccinator",
  "tildearrow",
  "Ultraprogramer",
  "",
  "-- additional feedback/fixes --",
  "fd",
  "OPNA2608",
  "plane",
  "TheEssem",
  "",
  "powered by:",
  "Dear ImGui by Omar Cornut",
  "SDL2 by Sam Lantinga",
  "zlib by Jean-loup Gailly",
  "and Mark Adler",
  "libsndfile by Erik de Castro Lopo",
  "Nuked-OPM & Nuked-OPN2 by Nuke.YKT",
  "ymfm by Aaron Giles",
  "MAME SN76496 by Nicola Salmoria",
  "MAME AY-3-8910 by Couriersud",
  "with AY8930 fixes by Eulous",
  "MAME SAA1099 by Juergen Buchmueller and Manuel Abadia",
  "SAASound",
  "SameBoy by Lior Halphon",
  "Mednafen PCE",
  "puNES by FHorse",
  "reSID by Dag Lem",
  "Stella by Stella Team",
  "QSound emulator by Ian Karlsson and Valley Bell",
  "",
  "greetings to:",
  "Delek",
  "fd",
  "ILLUMIDARO",
  "all members of Deflers of Noice!",
  "",
  "copyright © 2021-2022 tildearrow",
  "(and contributors).",
  "licensed under GPLv2+! see",
  "LICENSE for more information.",
  "",
  "help Furnace grow:",
  "https://github.com/tildearrow/furnace",
  "",
  "contact tildearrow at:",
  "https://tildearrow.org/?p=contact",
  "",
  "disclaimer:",
  "despite the fact this program works",
  "with the .dmf file format, it is NOT",
  "affiliated with Delek or DefleMask in",
  "any way, nor it is a replacement for",
  "the original program.",
  "",
  "it also comes with ABSOLUTELY NO WARRANTY.",
  "",
  "look out for Furnace 0.6 coming somewhere",
  "before the equinox with more systems",
  "and plenty of other things...",
  "",
  "thanks to all contributors/bug reporters!"
};

const size_t aboutCount = sizeof(aboutLine)/sizeof(aboutLine[0]);

void FurnaceGUI::drawAbout() {
  // do stuff
  if (ImGui::Begin("About Furnace",NULL,ImGuiWindowFlags_Modal|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoDocking|ImGuiWindowFlags_NoTitleBar)) {
    ImGui::SetWindowPos(ImVec2(0,0));
    ImGui::SetWindowSize(ImVec2(scrW*dpiScale,scrH*dpiScale));
    ImGui::PushFont(bigFont);
    ImDrawList* dl=ImGui::GetWindowDrawList();
    float r=0;
    float g=0;
    float b=0;
    float peakMix=settings.partyTime?((peak[0]+peak[1])*0.5):0.3;
    ImGui::ColorConvertHSVtoRGB(aboutHue,1.0,0.25+MIN(0.75f,peakMix*0.75f),r,g,b);
    dl->AddRectFilled(ImVec2(0,0),ImVec2(scrW*dpiScale,scrH*dpiScale),0xff000000);
    bool skip=false;
    bool skip2=false;
    for (int i=(-80-sin(double(aboutSin)*2*M_PI/120.0)*80.0)*2; i<scrW; i+=160) {
      skip2=!skip2;
      skip=skip2;
      for (int j=(-80-cos(double(aboutSin)*2*M_PI/150.0)*80.0)*2; j<scrH; j+=160) {
        skip=!skip;
        if (skip) continue;
        dl->AddRectFilled(ImVec2(i*dpiScale,j*dpiScale),ImVec2((i+160)*dpiScale,(j+160)*dpiScale),ImGui::GetColorU32(ImVec4(r*0.25,g*0.25,b*0.25,1.0)));
      }
    }

    skip=false;
    skip2=false;
    for (int i=(-80-cos(double(aboutSin)*2*M_PI/120.0)*80.0)*2; i<scrW; i+=160) {
      skip2=!skip2;
      skip=skip2;
      for (int j=(-80-sin(double(aboutSin)*2*M_PI/150.0)*80.0)*2; j<scrH; j+=160) {
        skip=!skip;
        if (skip) continue;
        dl->AddRectFilled(ImVec2(i*dpiScale,j*dpiScale),ImVec2((i+160)*dpiScale,(j+160)*dpiScale),ImGui::GetColorU32(ImVec4(r*0.5,g*0.5,b*0.5,1.0)));
      }
    }

    skip=false;
    skip2=false;
    for (int i=(-160+fmod(aboutSin*2,160))*2; i<scrW; i+=160) {
      skip2=!skip2;
      skip=skip2;
      for (int j=(-240-cos(double(aboutSin*M_PI/300.0))*240.0)*2; j<scrH; j+=160) {
        skip=!skip;
        if (skip) continue;
        dl->AddRectFilled(ImVec2(i*dpiScale,j*dpiScale),ImVec2((i+160)*dpiScale,(j+160)*dpiScale),ImGui::GetColorU32(ImVec4(r*0.75,g*0.75,b*0.75,1.0)));
      }
    }

    for (size_t i=0; i<aboutCount; i++) {
      double posX=(scrW*dpiScale/2.0)+(sin(double(i)*0.5+double(aboutScroll)/90.0)*120*dpiScale)-(ImGui::CalcTextSize(aboutLine[i]).x*0.5);
      double posY=(scrH-aboutScroll+42*i)*dpiScale;
      if (posY<-80*dpiScale || posY>scrH*dpiScale) continue;
      dl->AddText(bigFont,bigFont->FontSize,
                  ImVec2(posX+dpiScale,posY+dpiScale),
                  0xff000000,aboutLine[i]);
      dl->AddText(bigFont,bigFont->FontSize,
                  ImVec2(posX+dpiScale,posY-dpiScale),
                  0xff000000,aboutLine[i]);
      dl->AddText(bigFont,bigFont->FontSize,
                  ImVec2(posX-dpiScale,posY+dpiScale),
                  0xff000000,aboutLine[i]);
      dl->AddText(bigFont,bigFont->FontSize,
                  ImVec2(posX-dpiScale,posY-dpiScale),
                  0xff000000,aboutLine[i]);
      dl->AddText(bigFont,bigFont->FontSize,
                  ImVec2(posX,posY),
                  0xffffffff,aboutLine[i]);
    }
    ImGui::PopFont();

    float timeScale=60.0f*ImGui::GetIO().DeltaTime;

    aboutHue+=(0.001+peakMix*0.004)*timeScale;
    aboutScroll+=(2+(peakMix>0.78)*3)*timeScale;
    aboutSin+=(1+(peakMix>0.75)*2)*timeScale;

    while (aboutHue>1) aboutHue--;
    while (aboutSin>=2400) aboutSin-=2400;
    if (aboutScroll>(42*aboutCount+scrH)) aboutScroll=-20;
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_ABOUT;
  ImGui::End();
}

void FurnaceGUI::drawDebug() {
  static int bpOrder;
  static int bpRow;
  static int bpTick;
  static bool bpOn;
  if (nextWindow==GUI_WINDOW_DEBUG) {
    debugOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!debugOpen) return;
  ImGui::SetNextWindowSizeConstraints(ImVec2(400.0f*dpiScale,200.0f*dpiScale),ImVec2(scrW*dpiScale,scrH*dpiScale));
  if (ImGui::Begin("Debug",&debugOpen,ImGuiWindowFlags_NoDocking)) {
    ImGui::Text("NOTE: use with caution.");
    if (ImGui::TreeNode("Debug Controls")) {
      if (e->isHalted()) {
        if (ImGui::Button("Resume")) e->resume();
      } else {
        if (ImGui::Button("Pause")) e->halt();
      }
      ImGui::SameLine();
      if (ImGui::Button("Frame Advance")) e->haltWhen(DIV_HALT_TICK);
      ImGui::SameLine();
      if (ImGui::Button("Row Advance")) e->haltWhen(DIV_HALT_ROW);
      ImGui::SameLine();
      if (ImGui::Button("Pattern Advance")) e->haltWhen(DIV_HALT_PATTERN);

      if (ImGui::Button("Panic")) e->syncReset();
      ImGui::SameLine();
      if (ImGui::Button("Abort")) {
        abort();
      }
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Breakpoint")) {
      ImGui::InputInt("Order",&bpOrder);
      ImGui::InputInt("Row",&bpRow);
      ImGui::InputInt("Tick",&bpTick);
      ImGui::Checkbox("Enable",&bpOn);
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Dispatch Status")) {
      ImGui::Text("for best results set latency to minimum or use the Frame Advance button.");
      ImGui::Columns(e->getTotalChannelCount());
      for (int i=0; i<e->getTotalChannelCount(); i++) {
        void* ch=e->getDispatchChanState(i);
        ImGui::TextColored(uiColors[GUI_COLOR_ACCENT_PRIMARY],"Ch. %d: %d, %d",i,e->dispatchOfChan[i],e->dispatchChanOfChan[i]);
        if (ch==NULL) {
          ImGui::Text("NULL");
        } else {
          putDispatchChan(ch,e->dispatchChanOfChan[i],e->sysOfChan[i]);
        }
        ImGui::NextColumn();
      }
      ImGui::Columns();
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Playback Status")) {
      ImGui::Text("for best results set latency to minimum or use the Frame Advance button.");
      ImGui::Columns(e->getTotalChannelCount());
      for (int i=0; i<e->getTotalChannelCount(); i++) {
        DivChannelState* ch=e->getChanState(i);
        ImGui::TextColored(uiColors[GUI_COLOR_ACCENT_PRIMARY],"Channel %d:",i);
        if (ch==NULL) {
          ImGui::Text("NULL");
        } else {
          ImGui::Text("* General:");
          ImGui::Text("- note = %d",ch->note);
          ImGui::Text("- oldNote = %d",ch->oldNote);
          ImGui::Text("- pitch = %d",ch->pitch);
          ImGui::Text("- portaSpeed = %d",ch->portaSpeed);
          ImGui::Text("- portaNote = %d",ch->portaNote);
          ImGui::Text("- volume = %.4x",ch->volume);
          ImGui::Text("- volSpeed = %d",ch->volSpeed);
          ImGui::Text("- cut = %d",ch->cut);
          ImGui::Text("- rowDelay = %d",ch->rowDelay);
          ImGui::Text("- volMax = %.4x",ch->volMax);
          ImGui::Text("- delayOrder = %d",ch->delayOrder);
          ImGui::Text("- delayRow = %d",ch->delayRow);
          ImGui::Text("- retrigSpeed = %d",ch->retrigSpeed);
          ImGui::Text("- retrigTick = %d",ch->retrigTick);
          ImGui::PushStyleColor(ImGuiCol_Text,(ch->vibratoDepth>0)?uiColors[GUI_COLOR_MACRO_VOLUME]:uiColors[GUI_COLOR_TEXT]);
          ImGui::Text("* Vibrato:");
          ImGui::Text("- depth = %d",ch->vibratoDepth);
          ImGui::Text("- rate = %d",ch->vibratoRate);
          ImGui::Text("- pos = %d",ch->vibratoPos);
          ImGui::Text("- dir = %d",ch->vibratoDir);
          ImGui::Text("- fine = %d",ch->vibratoFine);
          ImGui::PopStyleColor();
          ImGui::PushStyleColor(ImGuiCol_Text,(ch->tremoloDepth>0)?uiColors[GUI_COLOR_MACRO_VOLUME]:uiColors[GUI_COLOR_TEXT]);
          ImGui::Text("* Tremolo:");
          ImGui::Text("- depth = %d",ch->tremoloDepth);
          ImGui::Text("- rate = %d",ch->tremoloRate);
          ImGui::Text("- pos = %d",ch->tremoloPos);
          ImGui::PopStyleColor();
          ImGui::PushStyleColor(ImGuiCol_Text,(ch->arp>0)?uiColors[GUI_COLOR_MACRO_VOLUME]:uiColors[GUI_COLOR_TEXT]);
          ImGui::Text("* Arpeggio:");
          ImGui::Text("- arp = %.2X",ch->arp);
          ImGui::Text("- stage = %d",ch->arpStage);
          ImGui::Text("- ticks = %d",ch->arpTicks);
          ImGui::PopStyleColor();
          ImGui::Text("* Miscellaneous:");
          ImGui::TextColored(ch->doNote?uiColors[GUI_COLOR_MACRO_VOLUME]:uiColors[GUI_COLOR_HEADER],">> Do Note");
          ImGui::TextColored(ch->legato?uiColors[GUI_COLOR_MACRO_VOLUME]:uiColors[GUI_COLOR_HEADER],">> Legato");
          ImGui::TextColored(ch->portaStop?uiColors[GUI_COLOR_MACRO_VOLUME]:uiColors[GUI_COLOR_HEADER],">> PortaStop");
          ImGui::TextColored(ch->keyOn?uiColors[GUI_COLOR_MACRO_VOLUME]:uiColors[GUI_COLOR_HEADER],">> Key On");
          ImGui::TextColored(ch->keyOff?uiColors[GUI_COLOR_MACRO_VOLUME]:uiColors[GUI_COLOR_HEADER],">> Key Off");
          ImGui::TextColored(ch->nowYouCanStop?uiColors[GUI_COLOR_MACRO_VOLUME]:uiColors[GUI_COLOR_HEADER],">> NowYouCanStop");
          ImGui::TextColored(ch->stopOnOff?uiColors[GUI_COLOR_MACRO_VOLUME]:uiColors[GUI_COLOR_HEADER],">> Stop on Off");
          ImGui::TextColored(ch->arpYield?uiColors[GUI_COLOR_MACRO_VOLUME]:uiColors[GUI_COLOR_HEADER],">> Arp Yield");
          ImGui::TextColored(ch->delayLocked?uiColors[GUI_COLOR_MACRO_VOLUME]:uiColors[GUI_COLOR_HEADER],">> DelayLocked");
          ImGui::TextColored(ch->inPorta?uiColors[GUI_COLOR_MACRO_VOLUME]:uiColors[GUI_COLOR_HEADER],">> InPorta");
          ImGui::TextColored(ch->scheduledSlideReset?uiColors[GUI_COLOR_MACRO_VOLUME]:uiColors[GUI_COLOR_HEADER],">> SchedSlide");
        }
        ImGui::NextColumn();
      }
      ImGui::Columns();
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Playground")) {
      if (pgSys<0 || pgSys>=e->song.systemLen) pgSys=0;
      if (ImGui::BeginCombo("System",fmt::sprintf("%d. %s",pgSys+1,e->getSystemName(e->song.system[pgSys])).c_str())) {
        for (int i=0; i<e->song.systemLen; i++) {
          if (ImGui::Selectable(fmt::sprintf("%d. %s",i+1,e->getSystemName(e->song.system[i])).c_str())) {
            pgSys=i;
            break;
          }
        }
        ImGui::EndCombo();
      }
      ImGui::Text("Program");
      if (pgProgram.empty()) {
        ImGui::Text("-nothing here-");
      } else {
        char id[32];
        for (size_t index=0; index<pgProgram.size(); index++) {
          DivRegWrite& i=pgProgram[index];
          snprintf(id,31,"pgw%d",(int)index);
          ImGui::PushID(id);
          ImGui::SetNextItemWidth(100.0f*dpiScale);
          ImGui::InputScalar("##PAddress",ImGuiDataType_U32,&i.addr,NULL,NULL,"%.2X",ImGuiInputTextFlags_CharsHexadecimal);
          ImGui::SameLine();
          ImGui::Text("=");
          ImGui::SameLine();
          ImGui::SetNextItemWidth(100.0f*dpiScale);
          ImGui::InputScalar("##PValue",ImGuiDataType_U16,&i.val,NULL,NULL,"%.2X",ImGuiInputTextFlags_CharsHexadecimal);
          ImGui::SameLine();
          if (ImGui::Button(ICON_FA_TIMES "##PRemove")) {
            pgProgram.erase(pgProgram.begin()+index);
            index--;
          }
          ImGui::PopID();
        }
      }
      if (ImGui::Button("Execute")) {
        e->poke(pgSys,pgProgram);
      }
      ImGui::SameLine();
      if (ImGui::Button("Clear")) {
        pgProgram.clear();
      }
      
      ImGui::Text("Address");
      ImGui::SameLine();
      ImGui::SetNextItemWidth(100.0f*dpiScale);
      ImGui::InputInt("##PAddress",&pgAddr,0,0,ImGuiInputTextFlags_CharsHexadecimal);
      ImGui::SameLine();
      ImGui::Text("Value");
      ImGui::SameLine();
      ImGui::SetNextItemWidth(100.0f*dpiScale);
      ImGui::InputInt("##PValue",&pgVal,0,0,ImGuiInputTextFlags_CharsHexadecimal);
      ImGui::SameLine();
      if (ImGui::Button("Write")) {
        e->poke(pgSys,pgAddr,pgVal);
      }
      ImGui::SameLine();
      if (ImGui::Button("Add")) {
        pgProgram.push_back(DivRegWrite(pgAddr,pgVal));
      }
      if (ImGui::TreeNode("Register Cheatsheet")) {
        const char** sheet=e->getRegisterSheet(pgSys);
        if (sheet==NULL) {
          ImGui::Text("no cheatsheet available for this system.");
        } else {
          if (ImGui::BeginTable("RegisterSheet",2,ImGuiTableFlags_SizingFixedSame)) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Name");
            ImGui::TableNextColumn();
            ImGui::Text("Address");
            for (int i=0; sheet[i]!=NULL; i+=2) {
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::Text("%s",sheet[i]);
              ImGui::TableNextColumn();
              ImGui::Text("$%s",sheet[i+1]);
            }
            ImGui::EndTable();
          }
        }
        ImGui::TreePop();
      }
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Settings")) {
      if (ImGui::Button("Sync")) syncSettings();
      ImGui::SameLine();
      if (ImGui::Button("Commit")) commitSettings();
      ImGui::SameLine();
      if (ImGui::Button("Force Load")) e->loadConf();
      ImGui::SameLine();
      if (ImGui::Button("Force Save")) e->saveConf();
      ImGui::TreePop();
    }
    ImGui::Text("Song format version %d",e->song.version);
    ImGui::Text("Furnace version " DIV_VERSION " (%d)",DIV_ENGINE_VERSION);
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_DEBUG;
  ImGui::End();
}

void FurnaceGUI::drawStats() {
  if (nextWindow==GUI_WINDOW_STATS) {
    statsOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!statsOpen) return;
  if (ImGui::Begin("Statistics",&statsOpen)) {
    String adpcmUsage=fmt::sprintf("%d/16384KB",e->adpcmMemLen/1024);
    String adpcmBUsage=fmt::sprintf("%d/16384KB",e->adpcmBMemLen/1024);
    String qsoundUsage=fmt::sprintf("%d/16384KB",e->qsoundMemLen/1024);
    ImGui::Text("ADPCM-A");
    ImGui::SameLine();
    ImGui::ProgressBar(((float)e->adpcmMemLen)/16777216.0f,ImVec2(-FLT_MIN,0),adpcmUsage.c_str());
    ImGui::Text("ADPCM-B");
    ImGui::SameLine();
    ImGui::ProgressBar(((float)e->adpcmBMemLen)/16777216.0f,ImVec2(-FLT_MIN,0),adpcmBUsage.c_str());
    ImGui::Text("QSound");
    ImGui::SameLine();
    ImGui::ProgressBar(((float)e->qsoundMemLen)/16777216.0f,ImVec2(-FLT_MIN,0),qsoundUsage.c_str());
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_STATS;
  ImGui::End();
}

void FurnaceGUI::drawCompatFlags() {
  if (nextWindow==GUI_WINDOW_COMPAT_FLAGS) {
    compatFlagsOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!compatFlagsOpen) return;
  if (ImGui::Begin("Compatibility Flags",&compatFlagsOpen)) {
    ImGui::TextWrapped("these flags are stored in the song when saving in .fur format, and are automatically enabled when saving in .dmf format.");
    ImGui::Checkbox("Limit slide range",&e->song.limitSlides);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("when enabled, slides are limited to a compatible range.\nmay cause problems with slides in negative octaves.");
    }
    ImGui::Checkbox("Linear pitch control",&e->song.linearPitch);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("linear pitch:\n- slides work in frequency/period space\n- E5xx and 04xx effects work in tonality space\nnon-linear pitch:\n- slides work in frequency/period space\n- E5xx and 04xx effects work on frequency/period space");
    }
    ImGui::Checkbox("Proper noise layout on NES and PC Engine",&e->song.properNoiseLayout);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("use a proper noise channel note mapping (0-15) instead of a rather unusual compatible one.\nunlocks all noise frequencies on PC Engine.");
    }
    ImGui::Checkbox("Game Boy instrument duty is wave volume",&e->song.waveDutyIsVol);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("if enabled, an instrument with duty macro in the wave channel will be mapped to wavetable volume.");
    }

    ImGui::Checkbox("Restart macro on portamento",&e->song.resetMacroOnPorta);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("when enabled, a portamento effect will reset the channel's macro if used in combination with a note.");
    }
    ImGui::Checkbox("Legacy volume slides",&e->song.legacyVolumeSlides);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("simulate glitchy volume slide behavior by silently overflowing the volume when the slide goes below 0.");
    }
    ImGui::Checkbox("Compatible arpeggio",&e->song.compatibleArpeggio);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("delay arpeggio by one tick on every new note.");
    }
    ImGui::Checkbox("Reset slides after note off",&e->song.noteOffResetsSlides);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("when enabled, note off will reset the channel's slide effect.");
    }
    ImGui::Checkbox("Reset portamento after reaching target",&e->song.targetResetsSlides);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("when enabled, the slide effect is disabled after it reaches its target.");
    }
    ImGui::Checkbox("Ignore duplicate slide effects",&e->song.ignoreDuplicateSlides);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("if this is on, only the first slide of a row in a channel will be considered.");
    }

    ImGui::Text("Loop modality:");
    if (ImGui::RadioButton("Reset channels",e->song.loopModality==0)) {
      e->song.loopModality=0;
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("select to reset channels on loop. may trigger a voltage click on every loop!");
    }
    if (ImGui::RadioButton("Soft reset channels",e->song.loopModality==1)) {
      e->song.loopModality=1;
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("select to turn channels off on loop.");
    }
    if (ImGui::RadioButton("Do nothing",e->song.loopModality==2)) {
      e->song.loopModality=2;
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("select to not reset channels on loop.");
    }

    ImGui::Separator();

    ImGui::TextWrapped("the following flags are for compatibility with older Furnace versions.");

    ImGui::Checkbox("Arpeggio inhibits non-porta slides",&e->song.arpNonPorta);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("behavior changed in 0.5.5");
    }
    ImGui::Checkbox("Wack FM algorithm macro",&e->song.algMacroBehavior);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("behavior changed in 0.5.5");
    }
    ImGui::Checkbox("Broken shortcut slides (E1xy/E2xy)",&e->song.brokenShortcutSlides);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("behavior changed in 0.5.7");
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_COMPAT_FLAGS;
  ImGui::End();
}

void FurnaceGUI::drawPiano() {
  if (nextWindow==GUI_WINDOW_PIANO) {
    pianoOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!pianoOpen) return;
  if (ImGui::Begin("Piano",&pianoOpen)) {
    for (int i=0; i<e->getTotalChannelCount(); i++) {
      DivChannelState* cs=e->getChanState(i);
      if (cs->keyOn) {
        const char* noteName=NULL;
        if (cs->note<-60 || cs->note>120) {
          noteName="???";
        } else {
          noteName=noteNames[cs->note+60];
        }
        ImGui::Text("%d: %s",i,noteName);
      }
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_PIANO;
  ImGui::End();
}

// NOTE: please don't ask me to enable text wrap.
//       Dear ImGui doesn't have that feature. D:
void FurnaceGUI::drawNotes() {
  if (nextWindow==GUI_WINDOW_NOTES) {
    notesOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!notesOpen) return;
  if (ImGui::Begin("Song Comments",&notesOpen)) {
    ImGui::InputTextMultiline("##SongNotes",&e->song.notes,ImGui::GetContentRegionAvail());
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_NOTES;
  ImGui::End();
}

void FurnaceGUI::drawChannels() {
  if (nextWindow==GUI_WINDOW_CHANNELS) {
    channelsOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!channelsOpen) return;
  if (ImGui::Begin("Channels",&channelsOpen)) {
    if (ImGui::BeginTable("ChannelList",3)) {
      ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed,0.0);
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0);
      ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthFixed,48.0f*dpiScale);
      for (int i=0; i<e->getTotalChannelCount(); i++) {
        ImGui::PushID(i);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Checkbox("##Visible",&e->song.chanShow[i]);
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputTextWithHint("##ChanName",e->getChannelName(i),&e->song.chanName[i]);
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputTextWithHint("##ChanShortName",e->getChannelShortName(i),&e->song.chanShortName[i]);
        ImGui::PopID();
      }
      ImGui::EndTable();
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_CHANNELS;
  ImGui::End();
}

void FurnaceGUI::drawRegView() {
  if (nextWindow==GUI_WINDOW_REGISTER_VIEW) {
    channelsOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!regViewOpen) return;
  if (ImGui::Begin("Register View",&regViewOpen)) {
    for (int i=0; i<e->song.systemLen; i++) {
      ImGui::Text("%d. %s",i+1,getSystemName(e->song.system[i]));
      int size=0;
      int depth=8;
      unsigned char* regPool=e->getRegisterPool(i,size,depth);
      unsigned short* regPoolW=(unsigned short*)regPool;
      if (regPool==NULL) {
        ImGui::Text("- no register pool available");
      } else {
        ImGui::PushFont(patFont);
        if (ImGui::BeginTable("Memory",17)) {
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          for (int i=0; i<16; i++) {
            ImGui::TableNextColumn();
            ImGui::TextColored(uiColors[GUI_COLOR_PATTERN_ROW_INDEX]," %X",i);
          }
          for (int i=0; i<=((size-1)>>4); i++) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextColored(uiColors[GUI_COLOR_PATTERN_ROW_INDEX],"%.2X",i*16);
            for (int j=0; j<16; j++) {
              ImGui::TableNextColumn();
              if (i*16+j>=size) continue;
              if (depth == 8) {
                ImGui::Text("%.2x",regPool[i*16+j]);
              } else if (depth == 16) {
                ImGui::Text("%.4x",regPoolW[i*16+j]);
              } else {
                ImGui::Text("??");
              }
            }
          }
          ImGui::EndTable();
        }
        ImGui::PopFont();
      }
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_REGISTER_VIEW;
  ImGui::End();
}

void FurnaceGUI::startSelection(int xCoarse, int xFine, int y) {
  if (xCoarse!=selStart.xCoarse || xFine!=selStart.xFine || y!=selStart.y) {
    curNibble=false;
  }
  cursor.xCoarse=xCoarse;
  cursor.xFine=xFine;
  cursor.y=y;
  selStart.xCoarse=xCoarse;
  selStart.xFine=xFine;
  selStart.y=y;
  selEnd.xCoarse=xCoarse;
  selEnd.xFine=xFine;
  selEnd.y=y;
  selecting=true;
}

void FurnaceGUI::updateSelection(int xCoarse, int xFine, int y) {
  if (!selecting) return;
  selEnd.xCoarse=xCoarse;
  selEnd.xFine=xFine;
  selEnd.y=y;
}

void FurnaceGUI::finishSelection() {
  // swap points if needed
  if (selEnd.y<selStart.y) {
    selEnd.y^=selStart.y;
    selStart.y^=selEnd.y;
    selEnd.y^=selStart.y;
  }
  if (selEnd.xCoarse<selStart.xCoarse) {
    selEnd.xCoarse^=selStart.xCoarse;
    selStart.xCoarse^=selEnd.xCoarse;
    selEnd.xCoarse^=selStart.xCoarse;

    selEnd.xFine^=selStart.xFine;
    selStart.xFine^=selEnd.xFine;
    selEnd.xFine^=selStart.xFine;
  } else if (selEnd.xCoarse==selStart.xCoarse && selEnd.xFine<selStart.xFine) {
    selEnd.xFine^=selStart.xFine;
    selStart.xFine^=selEnd.xFine;
    selEnd.xFine^=selStart.xFine;
  }
  selecting=false;

  // boundary check
  int chanCount=e->getTotalChannelCount();

  if (selStart.xCoarse<0) selStart.xCoarse=0;
  if (selStart.xCoarse>=chanCount) selStart.xCoarse=chanCount-1;
  if (selStart.y<0) selStart.y=0;
  if (selStart.y>=e->song.patLen) selStart.y=e->song.patLen-1;
  if (selEnd.xCoarse<0) selEnd.xCoarse=0;
  if (selEnd.xCoarse>=chanCount) selEnd.xCoarse=chanCount-1;
  if (selEnd.y<0) selEnd.y=0;
  if (selEnd.y>=e->song.patLen) selEnd.y=e->song.patLen-1;
  if (cursor.xCoarse<0) cursor.xCoarse=0;
  if (cursor.xCoarse>=chanCount) cursor.xCoarse=chanCount-1;
  if (cursor.y<0) cursor.y=0;
  if (cursor.y>=e->song.patLen) cursor.y=e->song.patLen-1;

  if (e->song.chanCollapse[selEnd.xCoarse]) {
    selStart.xFine=0;
  }
  if (e->song.chanCollapse[selEnd.xCoarse]) {
    selEnd.xFine=2+e->song.pat[cursor.xCoarse].effectRows*2;
  }
}

#define DETERMINE_FIRST \
  int firstChannel=0; \
  for (int i=0; i<e->getTotalChannelCount(); i++) { \
    if (e->song.chanShow[i]) { \
      firstChannel=i; \
      break; \
    } \
  } \

#define DETERMINE_LAST \
  int lastChannel=0; \
  for (int i=e->getTotalChannelCount()-1; i>=0; i--) { \
    if (e->song.chanShow[i]) { \
      lastChannel=i+1; \
      break; \
    } \
  }

#define DETERMINE_FIRST_LAST \
  DETERMINE_FIRST \
  DETERMINE_LAST

void FurnaceGUI::moveCursor(int x, int y, bool select) {
  if (!select) {
    finishSelection();
  }

  DETERMINE_FIRST_LAST;
  
  curNibble=false;
  if (x!=0) {
    demandScrollX=true;
    if (x>0) {
      for (int i=0; i<x; i++) {
        if (++cursor.xFine>=(e->song.chanCollapse[cursor.xCoarse]?1:(3+e->song.pat[cursor.xCoarse].effectRows*2))) {
          cursor.xFine=0;
          if (++cursor.xCoarse>=lastChannel) {
            if (settings.wrapHorizontal!=0 && !select) {
              cursor.xCoarse=firstChannel;
              if (settings.wrapHorizontal==2) y++;
            } else {
              cursor.xCoarse=lastChannel-1;
              cursor.xFine=e->song.chanCollapse[cursor.xCoarse]?0:(2+e->song.pat[cursor.xCoarse].effectRows*2);
            }
          } else {
            while (!e->song.chanShow[cursor.xCoarse]) {
              cursor.xCoarse++;
              if (cursor.xCoarse>=e->getTotalChannelCount()) break;
            }
          }
        }
      }
    } else {
      for (int i=0; i<-x; i++) {
        if (--cursor.xFine<0) {
          if (--cursor.xCoarse<firstChannel) {
            if (settings.wrapHorizontal!=0 && !select) {
              cursor.xCoarse=lastChannel-1;
              cursor.xFine=2+e->song.pat[cursor.xCoarse].effectRows*2;
              if (settings.wrapHorizontal==2) y--;
            } else {
              cursor.xCoarse=firstChannel;
              cursor.xFine=0;
            }
          } else {
            while (!e->song.chanShow[cursor.xCoarse]) {
              cursor.xCoarse--;
              if (cursor.xCoarse<0) break;
            }
            if (e->song.chanCollapse[cursor.xCoarse]) {
              cursor.xFine=0;
            } else {
              cursor.xFine=2+e->song.pat[cursor.xCoarse].effectRows*2;
            }
          }
        }
      }
    }
  }
  if (y!=0) {
    if (y>0) {
      for (int i=0; i<y; i++) {
        cursor.y++;
        if (cursor.y>=e->song.patLen) {
          if (settings.wrapVertical!=0 && !select) {
            cursor.y=0;
            if (settings.wrapVertical==2) {
              if (!e->isPlaying() && e->getOrder()<(e->song.ordersLen-1)) {
                e->setOrder(e->getOrder()+1);
              } else {
                cursor.y=e->song.patLen-1;
              }
            }
          } else {
            cursor.y=e->song.patLen-1;
          }
        }
      }
    } else {
      for (int i=0; i<-y; i++) {
        cursor.y--;
        if (cursor.y<0) {
          if (settings.wrapVertical!=0 && !select) {
            cursor.y=e->song.patLen-1;
            if (settings.wrapVertical==2) {
              if (!e->isPlaying() && e->getOrder()>0) {
                e->setOrder(e->getOrder()-1);
              } else {
                cursor.y=0;
              }
            }
          } else {
            cursor.y=0;
          }
        }
      }
    }
  }
  if (!select) {
    selStart=cursor;
  }
  selEnd=cursor;
  updateScroll(cursor.y);
}

void FurnaceGUI::moveCursorPrevChannel(bool overflow) {
  finishSelection();
  curNibble=false;

  DETERMINE_FIRST_LAST;

  do {
    cursor.xCoarse--;
    if (cursor.xCoarse<0) break;
  } while (!e->song.chanShow[cursor.xCoarse]);
  if (cursor.xCoarse<firstChannel) {
    if (overflow) {
      cursor.xCoarse=lastChannel-1;
    } else {
      cursor.xCoarse=firstChannel;
    }
  }

  selStart=cursor;
  selEnd=cursor;
}

void FurnaceGUI::moveCursorNextChannel(bool overflow) {
  finishSelection();
  curNibble=false;

  DETERMINE_FIRST_LAST;

  do {
    cursor.xCoarse++;
    if (cursor.xCoarse>=e->getTotalChannelCount()) break;
  } while (!e->song.chanShow[cursor.xCoarse]);
  if (cursor.xCoarse>=lastChannel) {
    if (overflow) {
      cursor.xCoarse=firstChannel;
    } else {
      cursor.xCoarse=lastChannel-1;
    }
  }

  selStart=cursor;
  selEnd=cursor;
}

void FurnaceGUI::moveCursorTop(bool select) {
  finishSelection();
  curNibble=false;
  if (cursor.y==0) {
    DETERMINE_FIRST;
    cursor.xCoarse=firstChannel;
    cursor.xFine=0;
  } else {
    cursor.y=0;
  }
  selStart=cursor;
  if (!select) {
    selEnd=cursor;
  }
  updateScroll(cursor.y);
}

void FurnaceGUI::moveCursorBottom(bool select) {
  finishSelection();
  curNibble=false;
  if (cursor.y==e->song.patLen-1) {
    DETERMINE_LAST;
    cursor.xCoarse=lastChannel-1;
    cursor.xFine=2+e->song.pat[cursor.xCoarse].effectRows*2;
  } else {
    cursor.y=e->song.patLen-1;
  }
  if (!select) {
    selStart=cursor;
  }
  selEnd=cursor;
  updateScroll(cursor.y);
}

void FurnaceGUI::editAdvance() {
  finishSelection();
  cursor.y+=editStep;
  if (cursor.y>=e->song.patLen) cursor.y=e->song.patLen-1;
  selStart=cursor;
  selEnd=cursor;
  updateScroll(cursor.y);
}

void FurnaceGUI::prepareUndo(ActionType action) {
  int order=e->getOrder();
  switch (action) {
    case GUI_UNDO_CHANGE_ORDER:
      oldOrders=e->song.orders;
      oldOrdersLen=e->song.ordersLen;
      break;
    case GUI_UNDO_PATTERN_EDIT:
    case GUI_UNDO_PATTERN_DELETE:
    case GUI_UNDO_PATTERN_PULL:
    case GUI_UNDO_PATTERN_PUSH:
    case GUI_UNDO_PATTERN_CUT:
    case GUI_UNDO_PATTERN_PASTE:
      for (int i=0; i<e->getTotalChannelCount(); i++) {
        e->song.pat[i].getPattern(e->song.orders.ord[i][order],false)->copyOn(oldPat[i]);
      }
      break;
  }
}

void FurnaceGUI::makeUndo(ActionType action) {
  bool doPush=false;
  UndoStep s;
  s.type=action;
  s.cursor=cursor;
  s.selStart=selStart;
  s.selEnd=selEnd;
  int order=e->getOrder();
  s.order=order;
  s.nibble=curNibble;
  switch (action) {
    case GUI_UNDO_CHANGE_ORDER:
      for (int i=0; i<DIV_MAX_CHANS; i++) {
        for (int j=0; j<128; j++) {
          if (oldOrders.ord[i][j]!=e->song.orders.ord[i][j]) {
            s.ord.push_back(UndoOrderData(i,j,oldOrders.ord[i][j],e->song.orders.ord[i][j]));
          }
        }
      }
      s.oldOrdersLen=oldOrdersLen;
      s.newOrdersLen=e->song.ordersLen;
      if (oldOrdersLen!=e->song.ordersLen) {
        doPush=true;
      }
      if (!s.ord.empty()) {
        doPush=true;
      }
      break;
    case GUI_UNDO_PATTERN_EDIT:
    case GUI_UNDO_PATTERN_DELETE:
    case GUI_UNDO_PATTERN_PULL:
    case GUI_UNDO_PATTERN_PUSH:
    case GUI_UNDO_PATTERN_CUT:
    case GUI_UNDO_PATTERN_PASTE:
      for (int i=0; i<e->getTotalChannelCount(); i++) {
        DivPattern* p=e->song.pat[i].getPattern(e->song.orders.ord[i][order],false);
        for (int j=0; j<e->song.patLen; j++) {
          for (int k=0; k<32; k++) {
            if (p->data[j][k]!=oldPat[i]->data[j][k]) {
              s.pat.push_back(UndoPatternData(i,e->song.orders.ord[i][order],j,k,oldPat[i]->data[j][k],p->data[j][k]));
            }
          }
        }
      }
      if (!s.pat.empty()) {
        doPush=true;
      }
      break;
  }
  if (doPush) {
    modified=true;
    undoHist.push_back(s);
    redoHist.clear();
    if (undoHist.size()>settings.maxUndoSteps) undoHist.pop_front();
  }
}

void FurnaceGUI::doSelectAll() {
  finishSelection();
  curNibble=false;
  if (selStart.xFine==0 && selEnd.xFine==2+e->song.pat[selEnd.xCoarse].effectRows*2) {
    if (selStart.y==0 && selEnd.y==e->song.patLen-1) { // select entire pattern
      selStart.xCoarse=0;
      selStart.xFine=0;
      selEnd.xCoarse=e->getTotalChannelCount()-1;
      selEnd.xFine=2+e->song.pat[selEnd.xCoarse].effectRows*2;
    } else { // select entire column
      selStart.y=0;
      selEnd.y=e->song.patLen-1;
    }
  } else {
    int selStartX=0;
    int selEndX=0;
    // find row position
    for (SelectionPoint i; i.xCoarse!=selStart.xCoarse || i.xFine!=selStart.xFine; selStartX++) {
      i.xFine++;
      if (i.xFine>=3+e->song.pat[i.xCoarse].effectRows*2) {
        i.xFine=0;
        i.xCoarse++;
      }
    }
    for (SelectionPoint i; i.xCoarse!=selEnd.xCoarse || i.xFine!=selEnd.xFine; selEndX++) {
      i.xFine++;
      if (i.xFine>=3+e->song.pat[i.xCoarse].effectRows*2) {
        i.xFine=0;
        i.xCoarse++;
      }
    }

    float aspect=float(selEndX-selStartX+1)/float(selEnd.y-selStart.y+1);
    if (aspect<1.0f && !(selStart.y==0 && selEnd.y==e->song.patLen-1)) { // up-down
      selStart.y=0;
      selEnd.y=e->song.patLen-1;
    } else { // left-right
      selStart.xFine=0;
      selEnd.xFine=2+e->song.pat[selEnd.xCoarse].effectRows*2;
    }
  }
}

void FurnaceGUI::doDelete() {
  finishSelection();
  prepareUndo(GUI_UNDO_PATTERN_DELETE);
  curNibble=false;

  int iCoarse=selStart.xCoarse;
  int iFine=selStart.xFine;
  int ord=e->getOrder();
  for (; iCoarse<=selEnd.xCoarse; iCoarse++) {
    if (!e->song.chanShow[iCoarse]) continue;
    DivPattern* pat=e->song.pat[iCoarse].getPattern(e->song.orders.ord[iCoarse][ord],true);
    for (; iFine<3+e->song.pat[iCoarse].effectRows*2 && (iCoarse<selEnd.xCoarse || iFine<=selEnd.xFine); iFine++) {
      for (int j=selStart.y; j<=selEnd.y; j++) {
        if (iFine==0) {
          pat->data[j][iFine]=0;
          if (selStart.y==selEnd.y) pat->data[j][2]=-1;
        }
        pat->data[j][iFine+1]=(iFine<1)?0:-1;
      }
    }
    iFine=0;
  }

  makeUndo(GUI_UNDO_PATTERN_DELETE);
}

void FurnaceGUI::doPullDelete() {
  finishSelection();
  prepareUndo(GUI_UNDO_PATTERN_PULL);
  curNibble=false;

  if (settings.pullDeleteBehavior) {
    if (--selStart.y<0) selStart.y=0;
    if (--selEnd.y<0) selEnd.y=0;
    if (--cursor.y<0) cursor.y=0;
    updateScroll(cursor.y);
  }

  int iCoarse=selStart.xCoarse;
  int iFine=selStart.xFine;
  int ord=e->getOrder();
  for (; iCoarse<=selEnd.xCoarse; iCoarse++) {
    if (!e->song.chanShow[iCoarse]) continue;
    DivPattern* pat=e->song.pat[iCoarse].getPattern(e->song.orders.ord[iCoarse][ord],true);
    for (; iFine<3+e->song.pat[iCoarse].effectRows*2 && (iCoarse<selEnd.xCoarse || iFine<=selEnd.xFine); iFine++) {
      for (int j=selStart.y; j<e->song.patLen; j++) {
        if (j<e->song.patLen-1) {
          if (iFine==0) {
            pat->data[j][iFine]=pat->data[j+1][iFine];
          }
          pat->data[j][iFine+1]=pat->data[j+1][iFine+1];
        } else {
          if (iFine==0) {
            pat->data[j][iFine]=0;
          }
          pat->data[j][iFine+1]=(iFine<1)?0:-1;
        }
      }
    }
    iFine=0;
  }

  makeUndo(GUI_UNDO_PATTERN_PULL);
}

void FurnaceGUI::doInsert() {
  finishSelection();
  prepareUndo(GUI_UNDO_PATTERN_PUSH);
  curNibble=false;

  int iCoarse=selStart.xCoarse;
  int iFine=selStart.xFine;
  int ord=e->getOrder();
  for (; iCoarse<=selEnd.xCoarse; iCoarse++) {
    if (!e->song.chanShow[iCoarse]) continue;
    DivPattern* pat=e->song.pat[iCoarse].getPattern(e->song.orders.ord[iCoarse][ord],true);
    for (; iFine<3+e->song.pat[iCoarse].effectRows*2 && (iCoarse<selEnd.xCoarse || iFine<=selEnd.xFine); iFine++) {
      for (int j=e->song.patLen-1; j>=selStart.y; j--) {
        if (j==selStart.y) {
          if (iFine==0) {
            pat->data[j][iFine]=0;
          }
          pat->data[j][iFine+1]=(iFine<1)?0:-1;
        } else {
          if (iFine==0) {
            pat->data[j][iFine]=pat->data[j-1][iFine];
          }
          pat->data[j][iFine+1]=pat->data[j-1][iFine+1];
        }
      }
    }
    iFine=0;
  }

  makeUndo(GUI_UNDO_PATTERN_PUSH);
}

void FurnaceGUI::doTranspose(int amount) {
  finishSelection();
  prepareUndo(GUI_UNDO_PATTERN_DELETE);
  curNibble=false;

  int iCoarse=selStart.xCoarse;
  int iFine=selStart.xFine;
  int ord=e->getOrder();
  for (; iCoarse<=selEnd.xCoarse; iCoarse++) {
    if (!e->song.chanShow[iCoarse]) continue;
    DivPattern* pat=e->song.pat[iCoarse].getPattern(e->song.orders.ord[iCoarse][ord],true);
    for (; iFine<3+e->song.pat[iCoarse].effectRows*2 && (iCoarse<selEnd.xCoarse || iFine<=selEnd.xFine); iFine++) {
      for (int j=selStart.y; j<=selEnd.y; j++) {
        if (iFine==0) {
          int origNote=pat->data[j][0];
          int origOctave=(signed char)pat->data[j][1];
          if (origNote!=0 && origNote!=100 && origNote!=101 && origNote!=102) {
            origNote+=amount;
            while (origNote>12) {
              origNote-=12;
              origOctave++;
            }
            while (origNote<1) {
              origNote+=12;
              origOctave--;
            }
            if (origOctave>7) {
              origNote=12;
              origOctave=7;
            }
            if (origOctave<-5) {
              origNote=1;
              origOctave=-5;
            }
            pat->data[j][0]=origNote;
            pat->data[j][1]=(unsigned char)origOctave;
          }
        }
      }
    }
    iFine=0;
  }

  makeUndo(GUI_UNDO_PATTERN_DELETE);
}

void FurnaceGUI::doCopy(bool cut) {
  finishSelection();
  if (cut) {
    curNibble=false;
    prepareUndo(GUI_UNDO_PATTERN_CUT);
  }
  clipboard=fmt::sprintf("org.tildearrow.furnace - Pattern Data (%d)\n%d",DIV_ENGINE_VERSION,selStart.xFine);

  for (int j=selStart.y; j<=selEnd.y; j++) {
    int iCoarse=selStart.xCoarse;
    int iFine=selStart.xFine;
    if (iFine>3 && !(iFine&1)) {
      iFine--;
    }
    int ord=e->getOrder();
    clipboard+='\n';
    for (; iCoarse<=selEnd.xCoarse; iCoarse++) {
      if (!e->song.chanShow[iCoarse]) continue;
      DivPattern* pat=e->song.pat[iCoarse].getPattern(e->song.orders.ord[iCoarse][ord],true);
      for (; iFine<3+e->song.pat[iCoarse].effectRows*2 && (iCoarse<selEnd.xCoarse || iFine<=selEnd.xFine); iFine++) {
        if (iFine==0) {
          clipboard+=noteNameNormal(pat->data[j][0],pat->data[j][1]);
          if (cut) {
            pat->data[j][0]=0;
            pat->data[j][1]=0;
          }
        } else {
          if (pat->data[j][iFine+1]==-1) {
            clipboard+="..";
          } else {
            clipboard+=fmt::sprintf("%.2X",pat->data[j][iFine+1]);
          }
          if (cut) {
            pat->data[j][iFine+1]=-1;
          }
        }
      }
      clipboard+='|';
      iFine=0;
    }
  }
  SDL_SetClipboardText(clipboard.c_str());

  if (cut) {
    makeUndo(GUI_UNDO_PATTERN_CUT);
  }
}

void FurnaceGUI::doPaste() {
  finishSelection();
  prepareUndo(GUI_UNDO_PATTERN_PASTE);
  char* clipText=SDL_GetClipboardText();
  if (clipText!=NULL) {
    if (clipText[0]) {
      clipboard=clipText;
    }
    SDL_free(clipText);
  }
  std::vector<String> data;
  String tempS;
  for (char i: clipboard) {
    if (i=='\r') continue;
    if (i=='\n') {
      data.push_back(tempS);
      tempS="";
      continue;
    }
    tempS+=i;
  }
  data.push_back(tempS);

  int startOff=-1;
  bool invalidData=false;
  if (data.size()<2) return;
  if (data[0]!=fmt::sprintf("org.tildearrow.furnace - Pattern Data (%d)",DIV_ENGINE_VERSION)) return;
  if (sscanf(data[1].c_str(),"%d",&startOff)!=1) return;
  if (startOff<0) return;

  DETERMINE_LAST;

  int j=cursor.y;
  char note[4];
  int ord=e->getOrder();
  for (size_t i=2; i<data.size() && j<e->song.patLen; i++) {
    size_t charPos=0;
    int iCoarse=cursor.xCoarse;
    int iFine=(startOff>2 && cursor.xFine>2)?(((cursor.xFine-1)&(~1))|1):startOff;

    String& line=data[i];

    while (charPos<line.size() && iCoarse<lastChannel) {
      DivPattern* pat=e->song.pat[iCoarse].getPattern(e->song.orders.ord[iCoarse][ord],true);
      if (line[charPos]=='|') {
        iCoarse++;
        if (iCoarse<lastChannel) while (!e->song.chanShow[iCoarse]) {
          iCoarse++;
          if (iCoarse>=lastChannel) break;
        }
        iFine=0;
        charPos++;
        continue;
      }
      if (iFine==0) {
        if (charPos>=line.size()) {
          invalidData=true;
          break;
        }
        note[0]=line[charPos++];
        if (charPos>=line.size()) {
          invalidData=true;
          break;
        }
        note[1]=line[charPos++];
        if (charPos>=line.size()) {
          invalidData=true;
          break;
        }
        note[2]=line[charPos++];
        note[3]=0;

        if (!decodeNote(note,pat->data[j][0],pat->data[j][1])) {
          invalidData=true;
          break;
        }
      } else {
        if (charPos>=line.size()) {
          invalidData=true;
          break;
        }
        note[0]=line[charPos++];
        if (charPos>=line.size()) {
          invalidData=true;
          break;
        }
        note[1]=line[charPos++];
        note[2]=0;

        if (strcmp(note,"..")==0) {
          pat->data[j][iFine+1]=-1;
        } else {
          unsigned int val=0;
          if (sscanf(note,"%2X",&val)!=1) {
            invalidData=true;
            break;
          }
          if (iFine<(3+e->song.pat[iCoarse].effectRows*2)) pat->data[j][iFine+1]=val;
        }
      }
      iFine++;
    }
    
    if (invalidData) {
      logW("invalid clipboard data! failed at line %d char %d\n",i,charPos);
      logW("%s\n",line.c_str());
      break;
    }
    j++;
  }

  makeUndo(GUI_UNDO_PATTERN_PASTE);
}

void FurnaceGUI::doUndo() {
  if (undoHist.empty()) return;
  UndoStep& us=undoHist.back();
  redoHist.push_back(us);
  modified=true;

  switch (us.type) {
    case GUI_UNDO_CHANGE_ORDER:
      e->song.ordersLen=us.oldOrdersLen;
      for (UndoOrderData& i: us.ord) {
        e->song.orders.ord[i.chan][i.ord]=i.oldVal;
      }
      break;
    case GUI_UNDO_PATTERN_EDIT:
    case GUI_UNDO_PATTERN_DELETE:
    case GUI_UNDO_PATTERN_PULL:
    case GUI_UNDO_PATTERN_PUSH:
    case GUI_UNDO_PATTERN_CUT:
    case GUI_UNDO_PATTERN_PASTE:
      for (UndoPatternData& i: us.pat) {
        DivPattern* p=e->song.pat[i.chan].getPattern(i.pat,true);
        p->data[i.row][i.col]=i.oldVal;
      }
      if (!e->isPlaying()) {
        cursor=us.cursor;
        selStart=us.selStart;
        selEnd=us.selEnd;
        curNibble=us.nibble;
        updateScroll(cursor.y);
        e->setOrder(us.order);
      }
      break;
  }

  undoHist.pop_back();
}

void FurnaceGUI::doRedo() {
  if (redoHist.empty()) return;
  UndoStep& us=redoHist.back();
  undoHist.push_back(us);
  modified=true;

  switch (us.type) {
    case GUI_UNDO_CHANGE_ORDER:
      e->song.ordersLen=us.newOrdersLen;
      for (UndoOrderData& i: us.ord) {
        e->song.orders.ord[i.chan][i.ord]=i.newVal;
      }
      break;
    case GUI_UNDO_PATTERN_EDIT:
    case GUI_UNDO_PATTERN_DELETE:
    case GUI_UNDO_PATTERN_PULL:
    case GUI_UNDO_PATTERN_PUSH:
    case GUI_UNDO_PATTERN_CUT:
    case GUI_UNDO_PATTERN_PASTE:
      for (UndoPatternData& i: us.pat) {
        DivPattern* p=e->song.pat[i.chan].getPattern(i.pat,true);
        p->data[i.row][i.col]=i.newVal;
      }
      if (!e->isPlaying()) {
        cursor=us.cursor;
        selStart=us.selStart;
        selEnd=us.selEnd;
        curNibble=us.nibble;
        updateScroll(cursor.y);
        e->setOrder(us.order);
      }

      break;
  }

  redoHist.pop_back();
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

void FurnaceGUI::previewNote(int refChan, int note) {
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

void FurnaceGUI::stopPreviewNote(SDL_Scancode scancode) {
  if (activeNotes.empty()) return;
  try {
    int key=noteKeys.at(scancode);
    int num=12*curOctave+key;

    if (key==100) return;
    if (key==101) return;
    if (key==102) return;

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

void FurnaceGUI::doAction(int what) {
  switch (what) {
    case GUI_ACTION_OPEN:
      if (modified) {
        showWarning("Unsaved changes! Are you sure?",GUI_WARN_OPEN);
      } else {
        openFileDialog(GUI_FILE_OPEN);
      }
      break;
    case GUI_ACTION_SAVE:
      if (curFileName=="") {
        openFileDialog(GUI_FILE_SAVE);
      } else {
        if (save(curFileName,e->song.isDMF?e->song.version:0)>0) {
          showError(fmt::sprintf("Error while saving file! (%s)",lastError));
        }
      }
      break;
    case GUI_ACTION_SAVE_AS:
      openFileDialog(GUI_FILE_SAVE);
      break;
    case GUI_ACTION_UNDO:
      doUndo();
      break;
    case GUI_ACTION_REDO:
      doRedo();
      break;
    case GUI_ACTION_PLAY_TOGGLE:
      if (e->isPlaying() && !e->isStepping()) {
        stop();
      } else {
        play();
      }
      break;
    case GUI_ACTION_PLAY:
      play();
      break;
    case GUI_ACTION_STOP:
      stop();
      break;
    case GUI_ACTION_PLAY_REPEAT:
      play();
      e->setRepeatPattern(true);
      break;
    case GUI_ACTION_PLAY_CURSOR:
      if (e->isPlaying() && !e->isStepping()) {
        stop();
      } else {
        play(cursor.y);
      }
      break;
    case GUI_ACTION_STEP_ONE:
      e->stepOne(cursor.y);
      break;
    case GUI_ACTION_OCTAVE_UP:
      if (++curOctave>7) {
        curOctave=7;
      } else {
        for (size_t i=0; i<activeNotes.size(); i++) {
          e->noteOff(activeNotes[i].chan);
        }
        activeNotes.clear();
      }
      break;
    case GUI_ACTION_OCTAVE_DOWN:
      if (--curOctave<-5) {
        curOctave=-5;
      } else {
        for (size_t i=0; i<activeNotes.size(); i++) {
          e->noteOff(activeNotes[i].chan);
        }
        activeNotes.clear();
      }
      break;
    case GUI_ACTION_INS_UP:
      if (--curIns<-1) curIns=-1;
      break;
    case GUI_ACTION_INS_DOWN:
      if (++curIns>=(int)e->song.ins.size()) curIns=((int)e->song.ins.size())-1;
      break;
    case GUI_ACTION_STEP_UP:
      if (++editStep>64) editStep=64;
      break;
    case GUI_ACTION_STEP_DOWN:
      if (--editStep<0) editStep=0;
      break;
    case GUI_ACTION_TOGGLE_EDIT:
      edit=!edit;
      break;
    case GUI_ACTION_METRONOME:
      e->setMetronome(!e->getMetronome());
      break;
    case GUI_ACTION_REPEAT_PATTERN:
      e->setRepeatPattern(!e->getRepeatPattern());
      break;
    case GUI_ACTION_FOLLOW_ORDERS:
      followOrders=!followOrders;
      break;
    case GUI_ACTION_FOLLOW_PATTERN:
      followPattern=!followPattern;
      break;
    case GUI_ACTION_PANIC:
      e->syncReset();
      break;

    case GUI_ACTION_WINDOW_EDIT_CONTROLS:
      nextWindow=GUI_WINDOW_EDIT_CONTROLS;
      break;
    case GUI_ACTION_WINDOW_ORDERS:
      nextWindow=GUI_WINDOW_ORDERS;
      break;
    case GUI_ACTION_WINDOW_INS_LIST:
      nextWindow=GUI_WINDOW_INS_LIST;
      break;
    case GUI_ACTION_WINDOW_INS_EDIT:
      nextWindow=GUI_WINDOW_INS_EDIT;
      break;
    case GUI_ACTION_WINDOW_SONG_INFO:
      nextWindow=GUI_WINDOW_SONG_INFO;
      break;
    case GUI_ACTION_WINDOW_PATTERN:
      nextWindow=GUI_WINDOW_PATTERN;
      break;
    case GUI_ACTION_WINDOW_WAVE_LIST:
      nextWindow=GUI_WINDOW_WAVE_LIST;
      break;
    case GUI_ACTION_WINDOW_WAVE_EDIT:
      nextWindow=GUI_WINDOW_WAVE_EDIT;
      break;
    case GUI_ACTION_WINDOW_SAMPLE_LIST:
      nextWindow=GUI_WINDOW_SAMPLE_LIST;
      break;
    case GUI_ACTION_WINDOW_SAMPLE_EDIT:
      nextWindow=GUI_WINDOW_SAMPLE_EDIT;
      break;
    case GUI_ACTION_WINDOW_ABOUT:
      nextWindow=GUI_WINDOW_ABOUT;
      break;
    case GUI_ACTION_WINDOW_SETTINGS:
      nextWindow=GUI_WINDOW_SETTINGS;
      break;
    case GUI_ACTION_WINDOW_MIXER:
      nextWindow=GUI_WINDOW_MIXER;
      break;
    case GUI_ACTION_WINDOW_DEBUG:
      nextWindow=GUI_WINDOW_DEBUG;
      break;
    case GUI_ACTION_WINDOW_OSCILLOSCOPE:
      nextWindow=GUI_WINDOW_OSCILLOSCOPE;
      break;
    case GUI_ACTION_WINDOW_VOL_METER:
      nextWindow=GUI_WINDOW_VOL_METER;
      break;
    case GUI_ACTION_WINDOW_STATS:
      nextWindow=GUI_WINDOW_STATS;
      break;
    case GUI_ACTION_WINDOW_COMPAT_FLAGS:
      nextWindow=GUI_WINDOW_COMPAT_FLAGS;
      break;
    case GUI_ACTION_WINDOW_PIANO:
      nextWindow=GUI_WINDOW_PIANO;
      break;
    case GUI_ACTION_WINDOW_NOTES:
      nextWindow=GUI_WINDOW_NOTES;
      break;
    case GUI_ACTION_WINDOW_CHANNELS:
      nextWindow=GUI_WINDOW_CHANNELS;
      break;
    case GUI_ACTION_WINDOW_REGISTER_VIEW:
      nextWindow=GUI_WINDOW_REGISTER_VIEW;
      break;
    
    case GUI_ACTION_COLLAPSE_WINDOW:
      collapseWindow=true;
      break;
    case GUI_ACTION_CLOSE_WINDOW:
      switch (curWindow) {
        case GUI_WINDOW_EDIT_CONTROLS:
          editControlsOpen=false;
          break;
        case GUI_WINDOW_SONG_INFO:
          songInfoOpen=false;
          break;
        case GUI_WINDOW_ORDERS:
          ordersOpen=false;
          break;
        case GUI_WINDOW_INS_LIST:
          insListOpen=false;
          break;
        case GUI_WINDOW_PATTERN:
          patternOpen=false;
          break;
        case GUI_WINDOW_INS_EDIT:
          insEditOpen=false;
          break;
        case GUI_WINDOW_WAVE_LIST:
          waveListOpen=false;
          break;
        case GUI_WINDOW_WAVE_EDIT:
          waveEditOpen=false;
          break;
        case GUI_WINDOW_SAMPLE_LIST:
          sampleListOpen=false;
          break;
        case GUI_WINDOW_SAMPLE_EDIT:
          sampleEditOpen=false;
          break;
        case GUI_WINDOW_MIXER:
          mixerOpen=false;
          break;
        case GUI_WINDOW_ABOUT:
          aboutOpen=false;
          break;
        case GUI_WINDOW_SETTINGS:
          settingsOpen=false;
          break;
        case GUI_WINDOW_DEBUG:
          debugOpen=false;
          break;
        case GUI_WINDOW_OSCILLOSCOPE:
          oscOpen=false;
          break;
        case GUI_WINDOW_VOL_METER:
          volMeterOpen=false;
          break;
        case GUI_WINDOW_STATS:
          statsOpen=false;
          break;
        case GUI_WINDOW_COMPAT_FLAGS:
          compatFlagsOpen=false;
          break;
        case GUI_WINDOW_PIANO:
          pianoOpen=false;
          break;
        case GUI_WINDOW_NOTES:
          notesOpen=false;
          break;
        case GUI_WINDOW_CHANNELS:
          channelsOpen=false;
          break;
        case GUI_WINDOW_REGISTER_VIEW:
          regViewOpen=false;
          break;
        default:
          break;
      }
      curWindow=GUI_WINDOW_NOTHING;
      break;

    case GUI_ACTION_PAT_NOTE_UP:
      doTranspose(1);
      break;
    case GUI_ACTION_PAT_NOTE_DOWN:
      doTranspose(-1);
      break;
    case GUI_ACTION_PAT_OCTAVE_UP:
      doTranspose(12);
      break;
    case GUI_ACTION_PAT_OCTAVE_DOWN:
      doTranspose(-12);
      break;
    case GUI_ACTION_PAT_SELECT_ALL:
      doSelectAll();
      break;
    case GUI_ACTION_PAT_CUT:
      doCopy(true);
      break;
    case GUI_ACTION_PAT_COPY:
      doCopy(false);
      break;
    case GUI_ACTION_PAT_PASTE:
      doPaste();
      break;
    case GUI_ACTION_PAT_CURSOR_UP:
      moveCursor(0,-MAX(1,settings.scrollStep?editStep:1),false);
      break;
    case GUI_ACTION_PAT_CURSOR_DOWN:
      moveCursor(0,MAX(1,settings.scrollStep?editStep:1),false);
      break;
    case GUI_ACTION_PAT_CURSOR_LEFT:
      moveCursor(-1,0,false);
      break;
    case GUI_ACTION_PAT_CURSOR_RIGHT:
      moveCursor(1,0,false);
      break;
    case GUI_ACTION_PAT_CURSOR_UP_ONE:
      moveCursor(0,-1,false);
      break;
    case GUI_ACTION_PAT_CURSOR_DOWN_ONE:
      moveCursor(0,1,false);
      break;
    case GUI_ACTION_PAT_CURSOR_LEFT_CHANNEL:
      moveCursorPrevChannel(false);
      break;
    case GUI_ACTION_PAT_CURSOR_RIGHT_CHANNEL:
      moveCursorNextChannel(false);
      break;
    case GUI_ACTION_PAT_CURSOR_NEXT_CHANNEL:
      moveCursorNextChannel(true);
      break;
    case GUI_ACTION_PAT_CURSOR_PREVIOUS_CHANNEL:
      moveCursorPrevChannel(true);
      break;
    case GUI_ACTION_PAT_CURSOR_BEGIN:
      moveCursorTop(false);
      break;
    case GUI_ACTION_PAT_CURSOR_END:
      moveCursorBottom(false);
      break;
    case GUI_ACTION_PAT_CURSOR_UP_COARSE:
      moveCursor(0,-16,false);
      break;
    case GUI_ACTION_PAT_CURSOR_DOWN_COARSE:
      moveCursor(0,16,false);
      break;
    case GUI_ACTION_PAT_SELECTION_UP:
      moveCursor(0,-MAX(1,settings.scrollStep?editStep:1),true);
      break;
    case GUI_ACTION_PAT_SELECTION_DOWN:
      moveCursor(0,MAX(1,settings.scrollStep?editStep:1),true);
      break;
    case GUI_ACTION_PAT_SELECTION_LEFT:
      moveCursor(-1,0,true);
      break;
    case GUI_ACTION_PAT_SELECTION_RIGHT:
      moveCursor(1,0,true);
      break;
    case GUI_ACTION_PAT_SELECTION_UP_ONE:
      moveCursor(0,-1,true);
      break;
    case GUI_ACTION_PAT_SELECTION_DOWN_ONE:
      moveCursor(0,1,true);
      break;
    case GUI_ACTION_PAT_SELECTION_BEGIN:
      moveCursorTop(true);
      break;
    case GUI_ACTION_PAT_SELECTION_END:
      moveCursorBottom(true);
      break;
    case GUI_ACTION_PAT_SELECTION_UP_COARSE:
      moveCursor(0,-16,true);
      break;
    case GUI_ACTION_PAT_SELECTION_DOWN_COARSE:
      moveCursor(0,16,true);
      break;
    case GUI_ACTION_PAT_DELETE:
      doDelete();
      if (settings.stepOnDelete) {
        moveCursor(0,editStep,false);
      }
      break;
    case GUI_ACTION_PAT_PULL_DELETE:
      doPullDelete();
      break;
    case GUI_ACTION_PAT_INSERT:
      doInsert();
      break;
    case GUI_ACTION_PAT_MUTE_CURSOR:
      if (cursor.xCoarse<0 || cursor.xCoarse>=e->getTotalChannelCount()) break;
      e->toggleMute(cursor.xCoarse);
      break;
    case GUI_ACTION_PAT_SOLO_CURSOR:
      if (cursor.xCoarse<0 || cursor.xCoarse>=e->getTotalChannelCount()) break;
      e->toggleSolo(cursor.xCoarse);
      break;
    case GUI_ACTION_PAT_UNMUTE_ALL:
      e->unmuteAll();
      break;
    case GUI_ACTION_PAT_NEXT_ORDER:
      if (e->getOrder()<e->song.ordersLen-1) {
        e->setOrder(e->getOrder()+1);
      }
      break;
    case GUI_ACTION_PAT_PREV_ORDER:
      if (e->getOrder()>0) {
        e->setOrder(e->getOrder()-1);
      }
      break;
    case GUI_ACTION_PAT_COLLAPSE:
      if (cursor.xCoarse<0 || cursor.xCoarse>=e->getTotalChannelCount()) break;
      e->song.chanCollapse[cursor.xCoarse]=!e->song.chanCollapse[cursor.xCoarse];
      break;
    case GUI_ACTION_PAT_INCREASE_COLUMNS:
      if (cursor.xCoarse<0 || cursor.xCoarse>=e->getTotalChannelCount()) break;
      e->song.pat[cursor.xCoarse].effectRows++;
              if (e->song.pat[cursor.xCoarse].effectRows>8) e->song.pat[cursor.xCoarse].effectRows=8;
      break;
    case GUI_ACTION_PAT_DECREASE_COLUMNS:
      if (cursor.xCoarse<0 || cursor.xCoarse>=e->getTotalChannelCount()) break;
      e->song.pat[cursor.xCoarse].effectRows--;
      if (e->song.pat[cursor.xCoarse].effectRows<1) e->song.pat[cursor.xCoarse].effectRows=1;
      break;

    case GUI_ACTION_INS_LIST_ADD:
      curIns=e->addInstrument(cursor.xCoarse);
      modified=true;
      break;
    case GUI_ACTION_INS_LIST_DUPLICATE:
      if (curIns>=0 && curIns<(int)e->song.ins.size()) {
        int prevIns=curIns;
        curIns=e->addInstrument(cursor.xCoarse);
        (*e->song.ins[curIns])=(*e->song.ins[prevIns]);
        modified=true;
      }
      break;
    case GUI_ACTION_INS_LIST_OPEN:
      openFileDialog(GUI_FILE_INS_OPEN);
      break;
    case GUI_ACTION_INS_LIST_SAVE:
      if (curIns>=0 && curIns<(int)e->song.ins.size()) openFileDialog(GUI_FILE_INS_SAVE);
      break;
    case GUI_ACTION_INS_LIST_MOVE_UP:
      if (e->moveInsUp(curIns)) curIns--;
      break;
    case GUI_ACTION_INS_LIST_MOVE_DOWN:
      if (e->moveInsDown(curIns)) curIns++;
      break;
    case GUI_ACTION_INS_LIST_DELETE:
      if (curIns>=0 && curIns<(int)e->song.ins.size()) {
        e->delInstrument(curIns);
        modified=true;
        if (curIns>=(int)e->song.ins.size()) {
          curIns--;
        }
      }
      break;
    case GUI_ACTION_INS_LIST_EDIT:
      insEditOpen=true;
      break;
    case GUI_ACTION_INS_LIST_UP:
      if (--curIns<0) curIns=0;
      break;
    case GUI_ACTION_INS_LIST_DOWN:
      if (++curIns>=(int)e->song.ins.size()) curIns=((int)e->song.ins.size())-1;
      break;
    
    case GUI_ACTION_WAVE_LIST_ADD:
      curWave=e->addWave();
      modified=true;
      break;
    case GUI_ACTION_WAVE_LIST_DUPLICATE:
      if (curWave>=0 && curWave<(int)e->song.wave.size()) {
        int prevWave=curWave;
        curWave=e->addWave();
        (*e->song.wave[curWave])=(*e->song.wave[prevWave]);
        modified=true;
      }
      break;
    case GUI_ACTION_WAVE_LIST_OPEN:
      openFileDialog(GUI_FILE_WAVE_OPEN);
      break;
    case GUI_ACTION_WAVE_LIST_SAVE:
      if (curWave>=0 && curWave<(int)e->song.wave.size()) openFileDialog(GUI_FILE_WAVE_SAVE);
      break;
    case GUI_ACTION_WAVE_LIST_MOVE_UP:
      if (e->moveWaveUp(curWave)) curWave--;
      break;
    case GUI_ACTION_WAVE_LIST_MOVE_DOWN:
      if (e->moveWaveDown(curWave)) curWave++;
      break;
    case GUI_ACTION_WAVE_LIST_DELETE:
      if (curWave>=0 && curWave<(int)e->song.wave.size()) {
        e->delWave(curWave);
        modified=true;
        if (curWave>=(int)e->song.wave.size()) {
          curWave--;
        }
      }
      break;
    case GUI_ACTION_WAVE_LIST_EDIT:
      waveEditOpen=true;
      break;
    case GUI_ACTION_WAVE_LIST_UP:
      if (--curWave<0) curWave=0;
      break;
    case GUI_ACTION_WAVE_LIST_DOWN:
      if (++curWave>=(int)e->song.wave.size()) curWave=((int)e->song.wave.size())-1;
      break;

    case GUI_ACTION_SAMPLE_LIST_ADD:
      curSample=e->addSample();
      modified=true;
      break;
    case GUI_ACTION_SAMPLE_LIST_OPEN:
      openFileDialog(GUI_FILE_SAMPLE_OPEN);
      break;
    case GUI_ACTION_SAMPLE_LIST_SAVE:
      if (curSample>=0 && curSample<(int)e->song.sample.size()) openFileDialog(GUI_FILE_SAMPLE_SAVE);
      break;
    case GUI_ACTION_SAMPLE_LIST_MOVE_UP:
      if (e->moveSampleUp(curSample)) curSample--;
      break;
    case GUI_ACTION_SAMPLE_LIST_MOVE_DOWN:
      if (e->moveSampleDown(curSample)) curSample++;
      break;
    case GUI_ACTION_SAMPLE_LIST_DELETE:
      e->delSample(curSample);
      modified=true;
      if (curSample>=(int)e->song.sample.size()) {
        curSample--;
      }
      break;
    case GUI_ACTION_SAMPLE_LIST_EDIT:
      sampleEditOpen=true;
      break;
    case GUI_ACTION_SAMPLE_LIST_UP:
      if (--curSample<0) curSample=0;
      break;
    case GUI_ACTION_SAMPLE_LIST_DOWN:
      if (++curSample>=(int)e->song.sample.size()) curSample=((int)e->song.sample.size())-1;
      break;
    case GUI_ACTION_SAMPLE_LIST_PREVIEW:
      e->previewSample(curSample);
      break;
    case GUI_ACTION_SAMPLE_LIST_STOP_PREVIEW:
      e->stopSamplePreview();
      break;

    case GUI_ACTION_ORDERS_UP:
      if (e->getOrder()>0) {
        e->setOrder(e->getOrder()-1);
      }
      break;
    case GUI_ACTION_ORDERS_DOWN:
      if (e->getOrder()<e->song.ordersLen-1) {
        e->setOrder(e->getOrder()+1);
      }
      break;
    case GUI_ACTION_ORDERS_LEFT: {
      DETERMINE_FIRST;

      do {
        orderCursor--;
        if (orderCursor<firstChannel) {
          orderCursor=firstChannel;
          break;
        }
      } while (!e->song.chanShow[orderCursor]);
      break;
    }
    case GUI_ACTION_ORDERS_RIGHT: {
      DETERMINE_LAST;

      do {
        orderCursor++;
        if (orderCursor>=lastChannel) {
          orderCursor=lastChannel-1;
          break;
        }
      } while (!e->song.chanShow[orderCursor]);
      break;
    }
    case GUI_ACTION_ORDERS_INCREASE: {
      if (orderCursor<0 || orderCursor>=e->getTotalChannelCount()) break;
      int curOrder=e->getOrder();
      if (e->song.orders.ord[orderCursor][curOrder]<0x7f) {
        e->song.orders.ord[orderCursor][curOrder]++;
      }
      break;
    }
    case GUI_ACTION_ORDERS_DECREASE: {
      if (orderCursor<0 || orderCursor>=e->getTotalChannelCount()) break;
      int curOrder=e->getOrder();
      if (e->song.orders.ord[orderCursor][curOrder]>0) {
        e->song.orders.ord[orderCursor][curOrder]--;
      }
      break;
    }
    case GUI_ACTION_ORDERS_EDIT_MODE:
      orderEditMode++;
      if (orderEditMode>3) orderEditMode=0;
      break;
    case GUI_ACTION_ORDERS_LINK:
      changeAllOrders=!changeAllOrders;
      break;
    case GUI_ACTION_ORDERS_ADD:
      prepareUndo(GUI_UNDO_CHANGE_ORDER);
      e->addOrder(false,false);
      makeUndo(GUI_UNDO_CHANGE_ORDER);
      break;
    case GUI_ACTION_ORDERS_DUPLICATE:
      prepareUndo(GUI_UNDO_CHANGE_ORDER);
      e->addOrder(true,false);
      makeUndo(GUI_UNDO_CHANGE_ORDER);
      break;
    case GUI_ACTION_ORDERS_DEEP_CLONE:
      prepareUndo(GUI_UNDO_CHANGE_ORDER);
      e->deepCloneOrder(false);
      makeUndo(GUI_UNDO_CHANGE_ORDER);
      if (!e->getWarnings().empty()) {
        showWarning(e->getWarnings(),GUI_WARN_GENERIC);
      }
      break;
    case GUI_ACTION_ORDERS_DUPLICATE_END:
      prepareUndo(GUI_UNDO_CHANGE_ORDER);
      e->addOrder(true,true);
      makeUndo(GUI_UNDO_CHANGE_ORDER);
      break;
    case GUI_ACTION_ORDERS_DEEP_CLONE_END:
      prepareUndo(GUI_UNDO_CHANGE_ORDER);
      e->deepCloneOrder(true);
      makeUndo(GUI_UNDO_CHANGE_ORDER);
      if (!e->getWarnings().empty()) {
        showWarning(e->getWarnings(),GUI_WARN_GENERIC);
      }
      break;
    case GUI_ACTION_ORDERS_REMOVE:
      prepareUndo(GUI_UNDO_CHANGE_ORDER);
      e->deleteOrder();
      makeUndo(GUI_UNDO_CHANGE_ORDER);
      break;
    case GUI_ACTION_ORDERS_MOVE_UP:
      prepareUndo(GUI_UNDO_CHANGE_ORDER);
      e->moveOrderUp();
      makeUndo(GUI_UNDO_CHANGE_ORDER);
      break;
    case GUI_ACTION_ORDERS_MOVE_DOWN:
      prepareUndo(GUI_UNDO_CHANGE_ORDER);
      e->moveOrderDown();
      makeUndo(GUI_UNDO_CHANGE_ORDER);
      break;
    case GUI_ACTION_ORDERS_REPLAY:
      e->setOrder(e->getOrder());
      break;
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

            if (edit) {
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
                pat->data[cursor.y][2]=curIns;
                previewNote(cursor.xCoarse,num);
              }
              makeUndo(GUI_UNDO_PATTERN_EDIT);
              editAdvance();
              curNibble=false;
            } else {
              if (key!=100 && key!=101 && key!=102) {
                previewNote(cursor.xCoarse,num);
              }
            }
          } catch (std::out_of_range& e) {
          }
        } else if (edit) { // value
          try {
            int num=valueKeys.at(ev.key.keysym.sym);
            DivPattern* pat=e->song.pat[cursor.xCoarse].getPattern(e->song.orders.ord[cursor.xCoarse][e->getOrder()],true);
            prepareUndo(GUI_UNDO_PATTERN_EDIT);
            if (pat->data[cursor.y][cursor.xFine+1]==-1) pat->data[cursor.y][cursor.xFine+1]=0;
            pat->data[cursor.y][cursor.xFine+1]=((pat->data[cursor.y][cursor.xFine+1]<<4)|num)&0xff;
            if (cursor.xFine==1) { // instrument
              if (pat->data[cursor.y][cursor.xFine+1]>=(int)e->song.ins.size()) {
                pat->data[cursor.y][cursor.xFine+1]&=0x0f;
                if (pat->data[cursor.y][cursor.xFine+1]>=(int)e->song.ins.size()) {
                  pat->data[cursor.y][cursor.xFine+1]=(int)e->song.ins.size()-1;
                }
              }
              makeUndo(GUI_UNDO_PATTERN_EDIT);
              if (e->song.ins.size()<16) {
                curNibble=false;
                editAdvance();
              } else {
                curNibble=!curNibble;
                if (!curNibble) editAdvance();
              }
            } else if (cursor.xFine==2) {
              if (curNibble) {
                if (pat->data[cursor.y][cursor.xFine+1]>e->getMaxVolumeChan(cursor.xCoarse)) pat->data[cursor.y][cursor.xFine+1]=e->getMaxVolumeChan(cursor.xCoarse);
              } else {
                pat->data[cursor.y][cursor.xFine+1]&=15;
              }
              makeUndo(GUI_UNDO_PATTERN_EDIT);
              if (e->getMaxVolumeChan(cursor.xCoarse)<16) {
                curNibble=false;
                editAdvance();
              } else {
                curNibble=!curNibble;
                if (!curNibble) editAdvance();
              }
            } else {
              makeUndo(GUI_UNDO_PATTERN_EDIT);
              curNibble=!curNibble;
              if (!curNibble) editAdvance();
            }
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
            e->song.orders.ord[orderCursor][curOrder]=((e->song.orders.ord[orderCursor][curOrder]<<4)|num)&0x7f;
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
            previewNote(cursor.xCoarse,num);
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
  if (!dirExists(workingDir)) workingDir=getHomeDir();
  ImGuiFileDialog::Instance()->DpiScale=dpiScale;
  switch (type) {
    case GUI_FILE_OPEN:
      ImGuiFileDialog::Instance()->OpenModal("FileDialog","Open File","compatible files{.fur,.dmf},.*",workingDir);
      break;
    case GUI_FILE_SAVE:
      ImGuiFileDialog::Instance()->OpenModal("FileDialog","Save File","Furnace song{.fur},DefleMask 1.1 module{.dmf}",workingDir,1,nullptr,ImGuiFileDialogFlags_ConfirmOverwrite);
      break;
    case GUI_FILE_SAVE_DMF_LEGACY:
      ImGuiFileDialog::Instance()->OpenModal("FileDialog","Save File","DefleMask 1.0/legacy module{.dmf}",workingDir,1,nullptr,ImGuiFileDialogFlags_ConfirmOverwrite);
      break;
    case GUI_FILE_INS_OPEN:
      ImGuiFileDialog::Instance()->OpenModal("FileDialog","Load Instrument","compatible files{.fui,.dmp,.tfi,.vgi},.*",workingDir);
      break;
    case GUI_FILE_INS_SAVE:
      ImGuiFileDialog::Instance()->OpenModal("FileDialog","Save Instrument","Furnace instrument{.fui}",workingDir,1,nullptr,ImGuiFileDialogFlags_ConfirmOverwrite);
      break;
    case GUI_FILE_WAVE_OPEN:
      ImGuiFileDialog::Instance()->OpenModal("FileDialog","Load Wavetable","compatible files{.fuw,.dmw},.*",workingDir);
      break;
    case GUI_FILE_WAVE_SAVE:
      ImGuiFileDialog::Instance()->OpenModal("FileDialog","Save Wavetable","Furnace wavetable{.fuw}",workingDir,1,nullptr,ImGuiFileDialogFlags_ConfirmOverwrite);
      break;
    case GUI_FILE_SAMPLE_OPEN:
      ImGuiFileDialog::Instance()->OpenModal("FileDialog","Load Sample","Wave file{.wav},.*",workingDir);
      break;
    case GUI_FILE_SAMPLE_SAVE:
      ImGuiFileDialog::Instance()->OpenModal("FileDialog","Save Sample","Wave file{.wav}",workingDir,1,nullptr,ImGuiFileDialogFlags_ConfirmOverwrite);
      break;
    case GUI_FILE_EXPORT_AUDIO_ONE:
      ImGuiFileDialog::Instance()->OpenModal("FileDialog","Export Audio","Wave file{.wav}",workingDir,1,nullptr,ImGuiFileDialogFlags_ConfirmOverwrite);
      break;
    case GUI_FILE_EXPORT_AUDIO_PER_SYS:
      ImGuiFileDialog::Instance()->OpenModal("FileDialog","Export Audio","Wave file{.wav}",workingDir,1,nullptr,ImGuiFileDialogFlags_ConfirmOverwrite);
      break;
    case GUI_FILE_EXPORT_AUDIO_PER_CHANNEL:
      ImGuiFileDialog::Instance()->OpenModal("FileDialog","Export Audio","Wave file{.wav}",workingDir,1,nullptr,ImGuiFileDialogFlags_ConfirmOverwrite);
      break;
    case GUI_FILE_EXPORT_VGM:
      ImGuiFileDialog::Instance()->OpenModal("FileDialog","Export VGM",".vgm",workingDir,1,nullptr,ImGuiFileDialogFlags_ConfirmOverwrite);
      break;
    case GUI_FILE_EXPORT_ROM:
      showError("Coming soon!");
      break;
    case GUI_FILE_LOAD_MAIN_FONT:
      ImGuiFileDialog::Instance()->OpenModal("FileDialog","Select Font","compatible files{.ttf,.otf,.ttc}",workingDir);
      break;
    case GUI_FILE_LOAD_PAT_FONT:
      ImGuiFileDialog::Instance()->OpenModal("FileDialog","Select Font","compatible files{.ttf,.otf,.ttc}",workingDir);
      break;
  }
  curFileDialog=type;
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
        printf("that file is empty!\n");
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
      modified=true;
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

#define BIND_FOR(x) getKeyName(actionKeys[x],true).c_str()

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
            if (motionY<patWindowPos.y) {
              addScroll(-1);
            }
            if (motionY>patWindowPos.y+patWindowSize.y) {
              addScroll(1);
            }
          }
          if (macroDragActive || macroLoopDragActive || waveDragActive) {
            int distance=fabs(motionXrel);
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
          if (macroDragActive || macroLoopDragActive || waveDragActive) modified=true;
          macroDragActive=false;
          macroDragBitMode=false;
          macroDragInitialValue=false;
          macroDragInitialValueSet=false;
          macroDragLastX=-1;
          macroDragLastY=-1;
          macroLoopDragActive=false;
          waveDragActive=false;
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
    
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame(sdlWin);
    ImGui::NewFrame();

    curWindow=GUI_WINDOW_NOTHING;

    ImGui::BeginMainMenuBar();
    if (ImGui::BeginMenu("file")) {
      if (ImGui::MenuItem("new")) {
        if (modified) {
          showWarning("Unsaved changes! Are you sure?",GUI_WARN_NEW);
        } else {
          e->createNew();
          undoHist.clear();
          redoHist.clear();
          curFileName="";
          modified=false;
          curNibble=false;
          orderNibble=false;
          orderCursor=-1;
          selStart=SelectionPoint();
          selEnd=SelectionPoint();
          cursor=SelectionPoint();
          updateWindowTitle();
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
        if (curFileName=="") {
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
        ImGui::Checkbox("loop",&vgmExportLoop);
        ImGui::Text("systems to export:");;
        bool hasOneAtLeast=false;
        for (int i=0; i<e->song.systemLen; i++) {
          ImGui::BeginDisabled(!e->isVGMExportable(e->song.system[i]));
          ImGui::Checkbox(fmt::sprintf("%d. %s##_SYSV%d",i+1,getSystemName(e->song.system[i]),i).c_str(),&willExport[i]);
          ImGui::EndDisabled();
          if (!e->isVGMExportable(e->song.system[i])) {
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
              ImGui::SetTooltip("this system is not supported by the VGM format!");
            }
          } else {
            if (willExport[i]) hasOneAtLeast=true;
          }
        }
        ImGui::Text("select the systems you wish to export,");
        ImGui::Text("but only up to 2 of each type.");
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
        sysAddOption(DIV_SYSTEM_YM2612);
        sysAddOption(DIV_SYSTEM_YM2612_EXT);
        sysAddOption(DIV_SYSTEM_SMS);
        sysAddOption(DIV_SYSTEM_GB);
        sysAddOption(DIV_SYSTEM_PCE);
        sysAddOption(DIV_SYSTEM_NES);
        sysAddOption(DIV_SYSTEM_C64_8580);
        sysAddOption(DIV_SYSTEM_C64_6581);
        sysAddOption(DIV_SYSTEM_YM2151);
        sysAddOption(DIV_SYSTEM_SEGAPCM);
        sysAddOption(DIV_SYSTEM_SEGAPCM_COMPAT);
        sysAddOption(DIV_SYSTEM_YM2610);
        sysAddOption(DIV_SYSTEM_YM2610_EXT);
        sysAddOption(DIV_SYSTEM_YM2610_FULL);
        sysAddOption(DIV_SYSTEM_YM2610_FULL_EXT);
        sysAddOption(DIV_SYSTEM_AY8910);
        sysAddOption(DIV_SYSTEM_AMIGA);
        sysAddOption(DIV_SYSTEM_TIA);
        sysAddOption(DIV_SYSTEM_SAA1099);
        sysAddOption(DIV_SYSTEM_AY8930);
        sysAddOption(DIV_SYSTEM_LYNX);
        sysAddOption(DIV_SYSTEM_QSOUND);
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("configure system...")) {
        for (int i=0; i<e->song.systemLen; i++) {
          if (ImGui::TreeNode(fmt::sprintf("%d. %s##_SYSP%d",i+1,getSystemName(e->song.system[i]),i).c_str())) {
            unsigned int flags=e->song.systemFlags[i];
            bool restart=settings.restartOnFlagChange;
            bool sysPal=flags&1;
            switch (e->song.system[i]) {
              case DIV_SYSTEM_YM2612:
              case DIV_SYSTEM_YM2612_EXT: {
                if (ImGui::RadioButton("NTSC (7.67MHz)",(flags&3)==0)) {
                  e->setSysFlags(i,(flags&0x80000000)|0,restart);
                  updateWindowTitle();
                }
                if (ImGui::RadioButton("PAL (7.61MHz)",(flags&3)==1)) {
                  e->setSysFlags(i,(flags&0x80000000)|1,restart);
                  updateWindowTitle();
                }
                if (ImGui::RadioButton("FM Towns (8MHz)",(flags&3)==2)) {
                  e->setSysFlags(i,(flags&0x80000000)|2,restart);
                  updateWindowTitle();
                }
                if (ImGui::RadioButton("AtGames Genesis (6.13MHz)",(flags&3)==3)) {
                  e->setSysFlags(i,(flags&0x80000000)|3,restart);
                  updateWindowTitle();
                }
                bool ladder=flags&0x80000000;
                if (ImGui::Checkbox("Enable DAC distortion",&ladder)) {
                  e->setSysFlags(i,(flags&(~0x80000000))|(ladder?0x80000000:0),restart);
                  updateWindowTitle();
                }
                break;
              }
              case DIV_SYSTEM_SMS: {
                ImGui::Text("Clock rate:");
                if (ImGui::RadioButton("NTSC (3.58MHz)",(flags&3)==0)) {
                  e->setSysFlags(i,(flags&(~3))|0,restart);
                  updateWindowTitle();
                }
                if (ImGui::RadioButton("PAL (3.55MHz)",(flags&3)==1)) {
                  e->setSysFlags(i,(flags&(~3))|1,restart);
                  updateWindowTitle();
                }
                if (ImGui::RadioButton("BBC Micro (4MHz)",(flags&3)==2)) {
                  e->setSysFlags(i,(flags&(~3))|2,restart);
                  updateWindowTitle();
                }
                ImGui::Text("Chip type:");
                if (ImGui::RadioButton("Sega VDP/Master System",((flags>>2)&3)==0)) {
                  e->setSysFlags(i,(flags&(~12))|0,restart);
                  updateWindowTitle();
                }
                if (ImGui::RadioButton("TI SN76489",((flags>>2)&3)==1)) {
                  e->setSysFlags(i,(flags&(~12))|4,restart);
                  updateWindowTitle();
                }
                if (ImGui::RadioButton("TI SN76489 with Atari-like short noise",((flags>>2)&3)==2)) {
                  e->setSysFlags(i,(flags&(~12))|8,restart);
                  updateWindowTitle();
                }
                /*if (ImGui::RadioButton("Game Gear",(flags>>2)==3)) {
                  e->setSysFlags(i,(flags&3)|12);
                }*/

                bool noPhaseReset=flags&16;
                if (ImGui::Checkbox("Disable noise period change phase reset",&noPhaseReset)) {
                  e->setSysFlags(i,(flags&(~16))|(noPhaseReset<<4),restart);
                  updateWindowTitle();
                }
                break;
              }
              case DIV_SYSTEM_YM2151:
                if (ImGui::RadioButton("NTSC (3.58MHz)",flags==0)) {
                  e->setSysFlags(i,0,restart);
                  updateWindowTitle();
                }
                if (ImGui::RadioButton("PAL (3.55MHz)",flags==1)) {
                  e->setSysFlags(i,1,restart);
                  updateWindowTitle();
                }
                if (ImGui::RadioButton("X68000 (4MHz)",flags==2)) {
                  e->setSysFlags(i,2,restart);
                  updateWindowTitle();
                }
                break;
              case DIV_SYSTEM_NES:
                if (ImGui::RadioButton("NTSC (1.79MHz)",flags==0)) {
                  e->setSysFlags(i,0,restart);
                  updateWindowTitle();
                }
                if (ImGui::RadioButton("PAL (1.67MHz)",flags==1)) {
                  e->setSysFlags(i,1,restart);
                  updateWindowTitle();
                }
                if (ImGui::RadioButton("Dendy (1.77MHz)",flags==2)) {
                  e->setSysFlags(i,2,restart);
                  updateWindowTitle();
                }
                break;
              case DIV_SYSTEM_AY8910:
              case DIV_SYSTEM_AY8930: {
                ImGui::Text("Clock rate:");
                if (ImGui::RadioButton("1.79MHz (ZX Spectrum/MSX NTSC)",(flags&15)==0)) {
                  e->setSysFlags(i,(flags&(~15))|0,restart);
                  updateWindowTitle();
                }
                if (ImGui::RadioButton("1.77MHz (ZX Spectrum/MSX PAL)",(flags&15)==1)) {
                  e->setSysFlags(i,(flags&(~15))|1,restart);
                  updateWindowTitle();
                }
                if (ImGui::RadioButton("1.75MHz (ZX Spectrum)",(flags&15)==2)) {
                  e->setSysFlags(i,(flags&(~15))|2,restart);
                  updateWindowTitle();
                }
                if (ImGui::RadioButton("2MHz (Atari ST)",(flags&15)==3)) {
                  e->setSysFlags(i,(flags&(~15))|3,restart);
                  updateWindowTitle();
                }
                if (ImGui::RadioButton("1.5MHz (Vectrex)",(flags&15)==4)) {
                  e->setSysFlags(i,(flags&(~15))|4,restart);
                  updateWindowTitle();
                }
                if (ImGui::RadioButton("1MHz (Amstrad CPC)",(flags&15)==5)) {
                  e->setSysFlags(i,(flags&(~15))|5,restart);
                  updateWindowTitle();
                }
                if (ImGui::RadioButton("0.89MHz (Sunsoft 5B)",(flags&15)==6)) {
                  e->setSysFlags(i,(flags&(~15))|6,restart);
                  updateWindowTitle();
                }
                if (ImGui::RadioButton("1.67MHz (?)",(flags&15)==7)) {
                  e->setSysFlags(i,(flags&(~15))|7,restart);
                  updateWindowTitle();
                }
                if (ImGui::RadioButton("0.83MHz (Sunsoft 5B on PAL)",(flags&15)==8)) {
                  e->setSysFlags(i,(flags&(~15))|8,restart);
                  updateWindowTitle();
                }
                if (e->song.system[i]==DIV_SYSTEM_AY8910) {
                  ImGui::Text("Chip type:");
                  if (ImGui::RadioButton("AY-3-8910",(flags&0x30)==0)) {
                    e->setSysFlags(i,(flags&(~0x30))|0,restart);
                    updateWindowTitle();
                  }
                  if (ImGui::RadioButton("YM2149(F)",(flags&0x30)==16)) {
                    e->setSysFlags(i,(flags&(~0x30))|16,restart);
                    updateWindowTitle();
                  }
                  if (ImGui::RadioButton("Sunsoft 5B",(flags&0x30)==32)) {
                    e->setSysFlags(i,(flags&(~0x30))|32,restart);
                    updateWindowTitle();
                  }
                }
                bool stereo=flags&0x40;
                ImGui::BeginDisabled((flags&0x30)==32);
                if (ImGui::Checkbox("Stereo##_AY_STEREO",&stereo)) {
                  e->setSysFlags(i,(flags&(~0x40))|(stereo?0x40:0),restart);
                  updateWindowTitle();
                }
                ImGui::EndDisabled();
                break;
              }
              case DIV_SYSTEM_SAA1099:
                if (ImGui::RadioButton("SAM Coupé (8MHz)",flags==0)) {
                  e->setSysFlags(i,0,restart);
                  updateWindowTitle();
                }
                if (ImGui::RadioButton("NTSC (7.15MHz)",flags==1)) {
                  e->setSysFlags(i,1,restart);
                  updateWindowTitle();
                }
                if (ImGui::RadioButton("PAL (7.09MHz)",flags==2)) {
                  e->setSysFlags(i,2,restart);
                  updateWindowTitle();
                }
                break;
              case DIV_SYSTEM_AMIGA: {
                ImGui::Text("Stereo separation:");
                int stereoSep=(flags>>8)&127;
                if (ImGui::SliderInt("##StereoSep",&stereoSep,0,127)) {
                  if (stereoSep<0) stereoSep=0;
                  if (stereoSep>127) stereoSep=127;
                  e->setSysFlags(i,(flags&1)|((stereoSep&127)<<8),restart);
                  updateWindowTitle();
                }
                /* TODO LATER: I want 0.5 out already
                if (ImGui::RadioButton("Amiga 500 (OCS)",(flags&2)==0)) {
                  e->setSysFlags(i,flags&1);
                }
                if (ImGui::RadioButton("Amiga 1200 (AGA)",(flags&2)==2)) {
                  e->setSysFlags(i,(flags&1)|2);
                }*/
                sysPal=flags&1;
                if (ImGui::Checkbox("PAL",&sysPal)) {
                  e->setSysFlags(i,(flags&2)|sysPal,restart);
                  updateWindowTitle();
                }
                break;
              }
              case DIV_SYSTEM_QSOUND: {
                ImGui::Text("Echo delay:");
                int echoBufSize=2725 - (flags & 4095);
                if (ImGui::SliderInt("##EchoBufSize",&echoBufSize,0,2725)) {
                  if (echoBufSize<0) echoBufSize=0;
                  if (echoBufSize>2725) echoBufSize=2725;
                  e->setSysFlags(i,(flags & ~4095) | ((2725 - echoBufSize) & 4095),restart);
                  updateWindowTitle();
                }
                ImGui::Text("Echo feedback:");
                int echoFeedback=(flags>>12)&255;
                if (ImGui::SliderInt("##EchoFeedback",&echoFeedback,0,255)) {
                  if (echoFeedback<0) echoFeedback=0;
                  if (echoFeedback>255) echoFeedback=255;
                  e->setSysFlags(i,(flags & ~(255 << 12)) | ((echoFeedback & 255) << 12),restart);
                  updateWindowTitle();
                }
                break;
              }
              case DIV_SYSTEM_GB:
              case DIV_SYSTEM_YM2610:
              case DIV_SYSTEM_YM2610_EXT:
              case DIV_SYSTEM_YM2610_FULL:
              case DIV_SYSTEM_YM2610_FULL_EXT:
              case DIV_SYSTEM_YMU759:
                ImGui::Text("nothing to configure");
                break;
              default:
                if (ImGui::Checkbox("PAL",&sysPal)) {
                  e->setSysFlags(i,sysPal,restart);
                  updateWindowTitle();
                }
                break;
            }
            ImGui::TreePop();
          }
        }
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("change system...")) {
        for (int i=0; i<e->song.systemLen; i++) {
          if (ImGui::BeginMenu(fmt::sprintf("%d. %s##_SYSC%d",i+1,getSystemName(e->song.system[i]),i).c_str())) {
            sysChangeOption(i,DIV_SYSTEM_YM2612);
            sysChangeOption(i,DIV_SYSTEM_YM2612_EXT);
            sysChangeOption(i,DIV_SYSTEM_SMS);
            sysChangeOption(i,DIV_SYSTEM_GB);
            sysChangeOption(i,DIV_SYSTEM_PCE);
            sysChangeOption(i,DIV_SYSTEM_NES);
            sysChangeOption(i,DIV_SYSTEM_C64_8580);
            sysChangeOption(i,DIV_SYSTEM_C64_6581);
            sysChangeOption(i,DIV_SYSTEM_YM2151);
            sysChangeOption(i,DIV_SYSTEM_SEGAPCM);
            sysChangeOption(i,DIV_SYSTEM_SEGAPCM_COMPAT);
            sysChangeOption(i,DIV_SYSTEM_YM2610);
            sysChangeOption(i,DIV_SYSTEM_YM2610_EXT);
            sysChangeOption(i,DIV_SYSTEM_YM2610_FULL);
            sysChangeOption(i,DIV_SYSTEM_YM2610_FULL_EXT);
            sysChangeOption(i,DIV_SYSTEM_AY8910);
            sysChangeOption(i,DIV_SYSTEM_AMIGA);
            sysChangeOption(i,DIV_SYSTEM_TIA);
            sysChangeOption(i,DIV_SYSTEM_SAA1099);
            sysChangeOption(i,DIV_SYSTEM_AY8930);
            sysChangeOption(i,DIV_SYSTEM_LYNX);
            sysChangeOption(i,DIV_SYSTEM_QSOUND);
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
      if (ImGui::MenuItem("cut",BIND_FOR(GUI_ACTION_PAT_CUT))) doCopy(true);
      if (ImGui::MenuItem("copy",BIND_FOR(GUI_ACTION_PAT_COPY))) doCopy(false);
      if (ImGui::MenuItem("paste",BIND_FOR(GUI_ACTION_PAT_PASTE))) doPaste();
      if (ImGui::MenuItem("delete",BIND_FOR(GUI_ACTION_PAT_DELETE))) doDelete();
      if (ImGui::MenuItem("select all",BIND_FOR(GUI_ACTION_PAT_SELECT_ALL))) doSelectAll();
      ImGui::Separator();
      if (ImGui::MenuItem("note up",BIND_FOR(GUI_ACTION_PAT_NOTE_UP))) doTranspose(1);
      if (ImGui::MenuItem("note down",BIND_FOR(GUI_ACTION_PAT_NOTE_DOWN))) doTranspose(-1);
      if (ImGui::MenuItem("octave up",BIND_FOR(GUI_ACTION_PAT_OCTAVE_UP))) doTranspose(12);
      if (ImGui::MenuItem("octave down",BIND_FOR(GUI_ACTION_PAT_OCTAVE_DOWN)))  doTranspose(-12);
      /*ImGui::Separator();
      ImGui::MenuItem("clear...");*/
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("settings")) {
      if (ImGui::MenuItem("reset layout")) {
        ImGui::LoadIniSettingsFromMemory(defaultLayout);
        ImGui::SaveIniSettingsToDisk(finalLayoutPath);
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
      ImGui::Text("| Speed %d:%d @ %dHz | Order %d/%d | Row %d/%d | %d:%.2d:%.2d.%.2d",e->getSpeed1(),e->getSpeed2(),e->getCurHz(),e->getOrder(),e->song.ordersLen,e->getRow(),e->song.patLen,totalSeconds/3600,(totalSeconds/60)%60,totalSeconds%60,totalTicks/10000);
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

    ImGui::DockSpaceOverViewport();

    drawEditControls();
    drawSongInfo();
    drawOrders();
    drawInsList();
    drawInsEdit();
    drawWaveList();
    drawWaveEdit();
    drawSampleList();
    drawSampleEdit();
    drawMixer();
    drawOsc();
    drawVolMeter();
    drawPattern();
    drawSettings();
    drawDebug();
    drawStats();
    drawCompatFlags();
    drawPiano();
    drawNotes();
    drawChannels();
    drawRegView();

    if (ImGuiFileDialog::Instance()->Display("FileDialog",ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoMove,ImVec2(600.0f*dpiScale,400.0f*dpiScale),ImVec2(scrW*dpiScale,scrH*dpiScale))) {
      //ImGui::GetIO().ConfigFlags&=~ImGuiConfigFlags_NavEnableKeyboard;
      if (ImGuiFileDialog::Instance()->IsOk()) {
        fileName=ImGuiFileDialog::Instance()->GetFilePathName();
        if (fileName!="") {
          if (curFileDialog==GUI_FILE_SAVE) {
            if (ImGuiFileDialog::Instance()->GetCurrentFilter()=="Furnace song") {
              checkExtension(".fur");
            } else {
              checkExtension(".dmf");
            }
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
            case GUI_FILE_SAVE:
              printf("saving: %s\n",copyOfName.c_str());
              if (ImGuiFileDialog::Instance()->GetCurrentFilter()=="Furnace song") {
                if (save(copyOfName,0)>0) {
                  showError(fmt::sprintf("Error while saving file! (%s)",lastError));
                }
              } else {
                if (save(copyOfName,25)>0) {
                  showError(fmt::sprintf("Error while saving file! (%s)",lastError));
                }
              }
              break;
            case GUI_FILE_SAVE_DMF_LEGACY:
              printf("saving: %s\n",copyOfName.c_str());
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
              e->addSampleFromFile(copyOfName.c_str());
              modified=true;
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
            case GUI_FILE_INS_OPEN:
              if (e->addInstrumentFromFile(copyOfName.c_str())) {
                if (!e->getWarnings().empty()) {
                  showWarning(e->getWarnings(),GUI_WARN_GENERIC);
                }
              } else {
                showError("cannot load instrument! ("+e->getLastError()+")");
              }
              break;
            case GUI_FILE_WAVE_OPEN:
              e->addWaveFromFile(copyOfName.c_str());
              modified=true;
              break;
            case GUI_FILE_EXPORT_VGM: {
              SafeWriter* w=e->saveVGM(willExport,vgmExportLoop);
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
                showError("could not write VGM. dang it.");
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
      workingDir=ImGuiFileDialog::Instance()->GetCurrentPath();
#ifdef _WIN32
      workingDir+='\\';
#else
      workingDir+='/';
#endif
      ImGuiFileDialog::Instance()->Close();
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
            e->createNew();
            undoHist.clear();
            redoHist.clear();
            curFileName="";
            modified=false;
            curNibble=false;
            orderNibble=false;
            orderCursor=-1;
            selStart=SelectionPoint();
            selEnd=SelectionPoint();
            cursor=SelectionPoint();
            updateWindowTitle();
            break;
          case GUI_WARN_OPEN:
            openFileDialog(GUI_FILE_OPEN);
            break;
          case GUI_WARN_OPEN_DROP:
            if (load(nextFile)>0) {
              showError(fmt::sprintf("Error while loading file! (%s)",lastError));
            }
            nextFile="";
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

    SDL_SetRenderDrawColor(sdlRend,uiColors[GUI_COLOR_BACKGROUND].x*255,
                                   uiColors[GUI_COLOR_BACKGROUND].y*255,
                                   uiColors[GUI_COLOR_BACKGROUND].z*255,
                                   uiColors[GUI_COLOR_BACKGROUND].w*255);
    SDL_RenderClear(sdlRend);
    ImGui::Render();
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(sdlRend);

    if (--soloTimeout<0) soloTimeout=0;

    if (willCommit) {
      commitSettings();
      willCommit=false;
    }
  }
  return false;
}

void FurnaceGUI::parseKeybinds() {
  actionMapGlobal.clear();
  actionMapPat.clear();
  actionMapInsList.clear();
  actionMapWaveList.clear();
  actionMapSampleList.clear();
  actionMapOrders.clear();

  for (int i=GUI_ACTION_GLOBAL_MIN+1; i<GUI_ACTION_GLOBAL_MAX; i++) {
    if (actionKeys[i]&FURK_MASK) {
      actionMapGlobal[actionKeys[i]]=i;
    }
  }

  for (int i=GUI_ACTION_PAT_MIN+1; i<GUI_ACTION_PAT_MAX; i++) {
    if (actionKeys[i]&FURK_MASK) {
      actionMapPat[actionKeys[i]]=i;
    }
  }

  for (int i=GUI_ACTION_INS_LIST_MIN+1; i<GUI_ACTION_INS_LIST_MAX; i++) {
    if (actionKeys[i]&FURK_MASK) {
      actionMapInsList[actionKeys[i]]=i;
    }
  }

  for (int i=GUI_ACTION_WAVE_LIST_MIN+1; i<GUI_ACTION_WAVE_LIST_MAX; i++) {
    if (actionKeys[i]&FURK_MASK) {
      actionMapWaveList[actionKeys[i]]=i;
    }
  }

  for (int i=GUI_ACTION_SAMPLE_LIST_MIN+1; i<GUI_ACTION_SAMPLE_LIST_MAX; i++) {
    if (actionKeys[i]&FURK_MASK) {
      actionMapSampleList[actionKeys[i]]=i;
    }
  }

  for (int i=GUI_ACTION_ORDERS_MIN+1; i<GUI_ACTION_ORDERS_MAX; i++) {
    if (actionKeys[i]&FURK_MASK) {
      actionMapOrders[actionKeys[i]]=i;
    }
  }
}

#define IGFD_FileStyleByExtension IGFD_FileStyleByExtention

#define GET_UI_COLOR(target,def) \
  uiColors[target]=ImGui::ColorConvertU32ToFloat4(e->getConfInt(#target,ImGui::GetColorU32(def)));

#ifdef _WIN32
#define SYSTEM_FONT_PATH_1 "C:\\Windows\\Fonts\\segoeui.ttf"
#define SYSTEM_FONT_PATH_2 "C:\\Windows\\Fonts\\tahoma.ttf"
// TODO!
#define SYSTEM_FONT_PATH_3 "C:\\Windows\\Fonts\\tahoma.ttf"
// TODO!
#define SYSTEM_PAT_FONT_PATH_1 "C:\\Windows\\Fonts\\consola.ttf"
#define SYSTEM_PAT_FONT_PATH_2 "C:\\Windows\\Fonts\\cour.ttf"
// GOOD LUCK WITH THIS ONE - UNTESTED
#define SYSTEM_PAT_FONT_PATH_3 "C:\\Windows\\Fonts\\vgasys.fon"
#elif defined(__APPLE__)
#define SYSTEM_FONT_PATH_1 "/System/Library/Fonts/SFAANS.ttf"
#define SYSTEM_FONT_PATH_2 "/System/Library/Fonts/Helvetica.ttc"
#define SYSTEM_FONT_PATH_3 "/System/Library/Fonts/Helvetica.dfont"
#define SYSTEM_PAT_FONT_PATH_1 "/System/Library/Fonts/SFNSMono.ttf"
#define SYSTEM_PAT_FONT_PATH_2 "/System/Library/Fonts/Courier New.ttf"
#define SYSTEM_PAT_FONT_PATH_3 "/System/Library/Fonts/Courier New.ttf"
#else
#define SYSTEM_FONT_PATH_1 "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
#define SYSTEM_FONT_PATH_2 "/usr/share/fonts/TTF/DejaVuSans.ttf"
#define SYSTEM_FONT_PATH_3 "/usr/share/fonts/ubuntu/Ubuntu-R.ttf"
#define SYSTEM_PAT_FONT_PATH_1 "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"
#define SYSTEM_PAT_FONT_PATH_2 "/usr/share/fonts/TTF/DejaVuSansMono.ttf"
#define SYSTEM_PAT_FONT_PATH_3 "/usr/share/fonts/ubuntu/UbuntuMono-R.ttf"
#endif

void FurnaceGUI::applyUISettings() {
  ImGuiStyle sty;
  ImGui::StyleColorsDark(&sty);

  if (settings.dpiScale>=0.5f) dpiScale=settings.dpiScale;

  GET_UI_COLOR(GUI_COLOR_BACKGROUND,ImVec4(0.1f,0.1f,0.1f,1.0f));
  GET_UI_COLOR(GUI_COLOR_FRAME_BACKGROUND,ImVec4(0.0f,0.0f,0.0f,0.85f));
  GET_UI_COLOR(GUI_COLOR_MODAL_BACKDROP,ImVec4(0.0f,0.0f,0.0f,0.55f));
  GET_UI_COLOR(GUI_COLOR_HEADER,ImVec4(0.2f,0.2f,0.2f,1.0f));
  GET_UI_COLOR(GUI_COLOR_TEXT,ImVec4(1.0f,1.0f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_ACCENT_PRIMARY,ImVec4(0.06f,0.53f,0.98f,1.0f));
  GET_UI_COLOR(GUI_COLOR_ACCENT_SECONDARY,ImVec4(0.26f,0.59f,0.98f,1.0f));
  GET_UI_COLOR(GUI_COLOR_EDITING,ImVec4(0.2f,0.1f,0.1f,1.0f));
  GET_UI_COLOR(GUI_COLOR_SONG_LOOP,ImVec4(0.3f,0.5f,0.8f,0.4f));
  GET_UI_COLOR(GUI_COLOR_VOLMETER_LOW,ImVec4(0.2f,0.6f,0.2f,1.0f));
  GET_UI_COLOR(GUI_COLOR_VOLMETER_HIGH,ImVec4(1.0f,0.9f,0.2f,1.0f));
  GET_UI_COLOR(GUI_COLOR_VOLMETER_PEAK,ImVec4(1.0f,0.1f,0.1f,1.0f));
  GET_UI_COLOR(GUI_COLOR_MACRO_VOLUME,ImVec4(0.2f,1.0f,0.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_MACRO_PITCH,ImVec4(1.0f,0.8f,0.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_MACRO_OTHER,ImVec4(0.0f,0.9f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_MACRO_WAVE,ImVec4(1.0f,0.4f,0.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_FM,ImVec4(0.6f,0.9f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_STD,ImVec4(0.6f,1.0f,0.5f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_GB,ImVec4(1.0f,1.0f,0.5f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_C64,ImVec4(0.85f,0.8f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_AMIGA,ImVec4(1.0f,0.5f,0.5f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_PCE,ImVec4(1.0f,0.8f,0.5f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_AY,ImVec4(1.0f,0.5f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_AY8930,ImVec4(0.7f,0.5f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_TIA,ImVec4(1.0f,0.6f,0.4f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_SAA1099,ImVec4(0.3f,0.3f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_VIC,ImVec4(0.2f,1.0f,0.6f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_PET,ImVec4(1.0f,1.0f,0.8f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_VRC6,ImVec4(1.0f,0.9f,0.5f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_OPLL,ImVec4(0.6f,0.7f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_OPL,ImVec4(0.3f,1.0f,0.9f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_FDS,ImVec4(0.8f,0.5f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_VBOY,ImVec4(1.0f,0.1f,0.1f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_N163,ImVec4(1.0f,0.4f,0.1f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_SCC,ImVec4(0.7f,1.0f,0.3f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_OPZ,ImVec4(0.2f,0.8f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_POKEY,ImVec4(0.5f,1.0f,0.3f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_BEEPER,ImVec4(0.0f,1.0f,0.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_SWAN,ImVec4(0.3f,0.5f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_MIKEY,ImVec4(0.5f,1.0f,0.3f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_UNKNOWN,ImVec4(0.3f,0.3f,0.3f,1.0f));
  GET_UI_COLOR(GUI_COLOR_CHANNEL_FM,ImVec4(0.2f,0.8f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_CHANNEL_PULSE,ImVec4(0.4f,1.0f,0.2f,1.0f));
  GET_UI_COLOR(GUI_COLOR_CHANNEL_NOISE,ImVec4(0.8f,0.8f,0.8f,1.0f));
  GET_UI_COLOR(GUI_COLOR_CHANNEL_PCM,ImVec4(1.0f,0.9f,0.2f,1.0f));
  GET_UI_COLOR(GUI_COLOR_CHANNEL_WAVE,ImVec4(1.0f,0.5f,0.2f,1.0f));
  GET_UI_COLOR(GUI_COLOR_CHANNEL_OP,ImVec4(0.2f,0.4f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_CHANNEL_MUTED,ImVec4(0.5f,0.5f,0.5f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_CURSOR,ImVec4(0.1f,0.3f,0.5f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_CURSOR_HOVER,ImVec4(0.2f,0.4f,0.6f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_CURSOR_ACTIVE,ImVec4(0.2f,0.5f,0.7f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_SELECTION,ImVec4(0.15f,0.15f,0.2f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_SELECTION_HOVER,ImVec4(0.2f,0.2f,0.3f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_SELECTION_ACTIVE,ImVec4(0.4f,0.4f,0.5f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_HI_1,ImVec4(0.6f,0.6f,0.6f,0.2f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_HI_2,ImVec4(0.5f,0.8f,1.0f,0.2f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_ROW_INDEX,ImVec4(0.5f,0.8f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_ACTIVE,ImVec4(1.0f,1.0f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_INACTIVE,ImVec4(0.5f,0.5f,0.5f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_INS,ImVec4(0.4f,0.7f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_VOLUME_MIN,ImVec4(0.0f,0.5f,0.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_VOLUME_HALF,ImVec4(0.0f,0.75f,0.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_VOLUME_MAX,ImVec4(0.0f,1.0f,0.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_EFFECT_INVALID,ImVec4(1.0f,0.0f,0.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_EFFECT_PITCH,ImVec4(1.0f,1.0f,0.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_EFFECT_VOLUME,ImVec4(0.0f,1.0f,0.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_EFFECT_PANNING,ImVec4(0.0f,1.0f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_EFFECT_SONG,ImVec4(1.0f,0.0f,0.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_EFFECT_TIME,ImVec4(0.5f,0.0f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_EFFECT_SPEED,ImVec4(1.0f,0.0f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_EFFECT_SYS_PRIMARY,ImVec4(0.5f,1.0f,0.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_EFFECT_SYS_SECONDARY,ImVec4(0.0f,1.0f,0.5f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_EFFECT_MISC,ImVec4(0.3f,0.3f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_EE_VALUE,ImVec4(0.0f,1.0f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PLAYBACK_STAT,ImVec4(0.6f,0.6f,0.6f,1.0f));

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
  ImGui::ColorConvertHSVtoRGB(hue,sat*0.9,val*0.5,primaryHover.x,primaryHover.y,primaryHover.z);
  ImGui::ColorConvertHSVtoRGB(hue,sat*0.8,val*0.35,primary.x,primary.y,primary.z);

  ImVec4 secondaryActive=uiColors[GUI_COLOR_ACCENT_SECONDARY];
  ImVec4 secondaryHover, secondary, secondarySemiActive;
  secondarySemiActive.w=secondaryActive.w;
  secondaryHover.w=secondaryActive.w;
  secondary.w=secondaryActive.w;
  ImGui::ColorConvertRGBtoHSV(secondaryActive.x,secondaryActive.y,secondaryActive.z,hue,sat,val);
  ImGui::ColorConvertHSVtoRGB(hue,sat*0.9,val*0.75,secondarySemiActive.x,secondarySemiActive.y,secondarySemiActive.z);
  ImGui::ColorConvertHSVtoRGB(hue,sat*0.9,val*0.5,secondaryHover.x,secondaryHover.y,secondaryHover.z);
  ImGui::ColorConvertHSVtoRGB(hue,sat*0.9,val*0.25,secondary.x,secondary.y,secondary.z);


  sty.Colors[ImGuiCol_WindowBg]=uiColors[GUI_COLOR_FRAME_BACKGROUND];
  sty.Colors[ImGuiCol_ModalWindowDimBg]=uiColors[GUI_COLOR_MODAL_BACKDROP];
  sty.Colors[ImGuiCol_Text]=uiColors[GUI_COLOR_TEXT];

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
  sty.Colors[ImGuiCol_TextSelectedBg]=secondaryHover;
  sty.Colors[ImGuiCol_PlotHistogram]=uiColors[GUI_COLOR_MACRO_OTHER];
  sty.Colors[ImGuiCol_PlotHistogramHovered]=uiColors[GUI_COLOR_MACRO_OTHER];

  sty.ScaleAllSizes(dpiScale);

  ImGui::GetStyle()=sty;

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

  // set to 800 for now due to problems with unifont
  static const ImWchar loadEverything[]={0x20,0x800,0};

  if (settings.mainFont<0 || settings.mainFont>6) settings.mainFont=0;
  if (settings.patFont<0 || settings.patFont>6) settings.patFont=0;

  if (settings.mainFont==6 && settings.mainFontPath.empty()) {
    logW("UI font path is empty! reverting to default font\n");
    settings.mainFont=0;
  }
  if (settings.patFont==6 && settings.patFontPath.empty()) {
    logW("pattern font path is empty! reverting to default font\n");
    settings.patFont=0;
  }

  if (settings.mainFont==6) { // custom font
    if ((mainFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(settings.mainFontPath.c_str(),e->getConfInt("mainFontSize",18)*dpiScale,NULL,loadEverything))==NULL) {
      logW("could not load UI font! reverting to default font\n");
      settings.mainFont=0;
      if ((mainFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(builtinFont[settings.mainFont],builtinFontLen[settings.mainFont],e->getConfInt("mainFontSize",18)*dpiScale,NULL,loadEverything))==NULL) {
        logE("could not load UI font! falling back to Proggy Clean.\n");
        mainFont=ImGui::GetIO().Fonts->AddFontDefault();
      }
    }
  } else if (settings.mainFont==5) { // system font
    if ((mainFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_FONT_PATH_1,e->getConfInt("mainFontSize",18)*dpiScale,NULL,loadEverything))==NULL) {
      if ((mainFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_FONT_PATH_2,e->getConfInt("mainFontSize",18)*dpiScale,NULL,loadEverything))==NULL) {
        if ((mainFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_FONT_PATH_3,e->getConfInt("mainFontSize",18)*dpiScale,NULL,loadEverything))==NULL) {
          logW("could not load UI font! reverting to default font\n");
          settings.mainFont=0;
          if ((mainFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(builtinFont[settings.mainFont],builtinFontLen[settings.mainFont],e->getConfInt("mainFontSize",18)*dpiScale,NULL,loadEverything))==NULL) {
            logE("could not load UI font! falling back to Proggy Clean.\n");
            mainFont=ImGui::GetIO().Fonts->AddFontDefault();
          }
        }
      }
    }
  } else {
    if ((mainFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(builtinFont[settings.mainFont],builtinFontLen[settings.mainFont],e->getConfInt("mainFontSize",18)*dpiScale,NULL,loadEverything))==NULL) {
      logE("could not load UI font! falling back to Proggy Clean.\n");
      mainFont=ImGui::GetIO().Fonts->AddFontDefault();
    }
  }

  ImFontConfig fc;
  fc.MergeMode=true;
  fc.GlyphMinAdvanceX=e->getConfInt("iconSize",16)*dpiScale;
  static const ImWchar fontRange[]={ICON_MIN_FA,ICON_MAX_FA,0};
  if ((iconFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(iconFont_compressed_data,iconFont_compressed_size,e->getConfInt("iconSize",16)*dpiScale,&fc,fontRange))==NULL) {
    logE("could not load icon font!\n");
  }
  if (settings.mainFontSize==settings.patFontSize && settings.patFont<5 && builtinFontM[settings.patFont]==builtinFont[settings.mainFont]) {
    logD("using main font for pat font.\n");
    patFont=mainFont;
  } else {
    if (settings.patFont==6) { // custom font
      if ((patFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(settings.patFontPath.c_str(),e->getConfInt("patFontSize",18)*dpiScale,NULL,loadEverything))==NULL) {
        logW("could not load pattern font! reverting to default font\n");
        settings.patFont=0;
        if ((patFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(builtinFontM[settings.patFont],builtinFontMLen[settings.patFont],e->getConfInt("patFontSize",18)*dpiScale,NULL,loadEverything))==NULL) {
          logE("could not load pattern font! falling back to Proggy Clean.\n");
          patFont=ImGui::GetIO().Fonts->AddFontDefault();
        }
      }
    } else if (settings.patFont==5) { // system font
      if ((patFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_PAT_FONT_PATH_1,e->getConfInt("patFontSize",18)*dpiScale,NULL,loadEverything))==NULL) {
        if ((patFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_PAT_FONT_PATH_2,e->getConfInt("patFontSize",18)*dpiScale,NULL,loadEverything))==NULL) {
          if ((patFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_PAT_FONT_PATH_3,e->getConfInt("patFontSize",18)*dpiScale,NULL,loadEverything))==NULL) {
            logW("could not load pattern font! reverting to default font\n");
            settings.patFont=0;
            if ((patFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(builtinFontM[settings.patFont],builtinFontMLen[settings.patFont],e->getConfInt("patFontSize",18)*dpiScale,NULL,loadEverything))==NULL) {
              logE("could not load pattern font! falling back to Proggy Clean.\n");
              patFont=ImGui::GetIO().Fonts->AddFontDefault();
            }
          }
        }
      }
    } else {
      if ((patFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(builtinFontM[settings.patFont],builtinFontMLen[settings.patFont],e->getConfInt("patFontSize",18)*dpiScale,NULL,loadEverything))==NULL) {
        logE("could not load pattern font!\n");
        patFont=ImGui::GetIO().Fonts->AddFontDefault();
      }
   }
  }
  if ((bigFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(font_plexSans_compressed_data,font_plexSans_compressed_size,40*dpiScale))==NULL) {
    logE("could not load big UI font!\n");
  }
}

bool FurnaceGUI::init() {
#ifndef __APPLE__
  float dpiScaleF;
#endif

  workingDir=e->getConfString("lastDir",getHomeDir());

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
    logE("could not open window!\n");
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
  prepareLayout();

  ImGui::GetIO().ConfigFlags|=ImGuiConfigFlags_DockingEnable;
  ImGui::GetIO().IniFilename=finalLayoutPath;
  ImGui::LoadIniSettingsFromDisk(finalLayoutPath);

  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir,"",ImVec4(0.0f,1.0f,1.0f,1.0f),ICON_FA_FOLDER_O);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile,"",ImVec4(0.7f,0.7f,0.7f,1.0f),ICON_FA_FILE_O);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".fur",ImVec4(0.5f,1.0f,0.5f,1.0f),ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".fui",ImVec4(1.0f,0.5f,0.5f,1.0f),ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".fuw",ImVec4(1.0f,0.75f,0.5f,1.0f),ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".dmf",ImVec4(0.5f,1.0f,0.5f,1.0f),ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".dmp",ImVec4(1.0f,0.5f,0.5f,1.0f),ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".dmw",ImVec4(1.0f,0.75f,0.5f,1.0f),ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".wav",ImVec4(1.0f,1.0f,0.5f,1.0f),ICON_FA_FILE_AUDIO_O);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".vgm",ImVec4(1.0f,1.0f,0.5f,1.0f),ICON_FA_FILE_AUDIO_O);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".ttf",ImVec4(0.3f,1.0f,0.6f,1.0f),ICON_FA_FONT);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".otf",ImVec4(0.3f,1.0f,0.6f,1.0f),ICON_FA_FONT);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".ttc",ImVec4(0.3f,1.0f,0.6f,1.0f),ICON_FA_FONT);

  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".tfi",ImVec4(1.0f,0.5f,0.5f,1.0f),ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".vgi",ImVec4(1.0f,0.5f,0.5f,1.0f),ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".fti",ImVec4(1.0f,0.5f,0.5f,1.0f),ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".bti",ImVec4(1.0f,0.5f,0.5f,1.0f),ICON_FA_FILE);

  updateWindowTitle();

  for (int i=0; i<DIV_MAX_CHANS; i++) {
    oldPat[i]=new DivPattern;
  }

#ifdef __APPLE__
  SDL_RaiseWindow(sdlWin);
#endif
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

  for (int i=0; i<DIV_MAX_CHANS; i++) {
    delete oldPat[i];
  }
  return true;
}

FurnaceGUI::FurnaceGUI():
  e(NULL),
  quit(false),
  warnQuit(false),
  willCommit(false),
  edit(false),
  modified(false),
  displayError(false),
  displayExporting(false),
  vgmExportLoop(true),
  curFileDialog(GUI_FILE_OPEN),
  warnAction(GUI_WARN_OPEN),
  scrW(1280),
  scrH(800),
  dpiScale(1),
  aboutScroll(0),
  aboutSin(0),
  aboutHue(0.0f),
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
  oscOpen(true),
  volMeterOpen(true),
  statsOpen(false),
  compatFlagsOpen(false),
  pianoOpen(false),
  notesOpen(false),
  channelsOpen(false),
  regViewOpen(false),
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
  curWindow(GUI_WINDOW_NOTHING),
  nextWindow(GUI_WINDOW_NOTHING),
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
  oldOrdersLen(0) {

  // octave 1
  /*
  noteKeys[SDL_SCANCODE_Z]=0;
  noteKeys[SDL_SCANCODE_S]=1;
  noteKeys[SDL_SCANCODE_X]=2;
  noteKeys[SDL_SCANCODE_D]=3;
  noteKeys[SDL_SCANCODE_C]=4;
  noteKeys[SDL_SCANCODE_V]=5;
  noteKeys[SDL_SCANCODE_G]=6;
  noteKeys[SDL_SCANCODE_B]=7;
  noteKeys[SDL_SCANCODE_H]=8;
  noteKeys[SDL_SCANCODE_N]=9;
  noteKeys[SDL_SCANCODE_J]=10;
  noteKeys[SDL_SCANCODE_M]=11;

  // octave 2
  noteKeys[SDL_SCANCODE_Q]=12;
  noteKeys[SDL_SCANCODE_2]=13;
  noteKeys[SDL_SCANCODE_W]=14;
  noteKeys[SDL_SCANCODE_3]=15;
  noteKeys[SDL_SCANCODE_E]=16;
  noteKeys[SDL_SCANCODE_R]=17;
  noteKeys[SDL_SCANCODE_5]=18;
  noteKeys[SDL_SCANCODE_T]=19;
  noteKeys[SDL_SCANCODE_6]=20;
  noteKeys[SDL_SCANCODE_Y]=21;
  noteKeys[SDL_SCANCODE_7]=22;
  noteKeys[SDL_SCANCODE_U]=23;

  // octave 3
  noteKeys[SDL_SCANCODE_I]=24;
  noteKeys[SDL_SCANCODE_9]=25;
  noteKeys[SDL_SCANCODE_O]=26;
  noteKeys[SDL_SCANCODE_0]=27;
  noteKeys[SDL_SCANCODE_P]=28;
  noteKeys[SDL_SCANCODE_LEFTBRACKET]=29;
  noteKeys[SDL_SCANCODE_RIGHTBRACKET]=31;

  // note off
  noteKeys[SDL_SCANCODE_TAB]=100;
  noteKeys[SDL_SCANCODE_1]=100;

  // note off + env release
  noteKeys[SDL_SCANCODE_EQUALS]=101;

  // env release
  noteKeys[SDL_SCANCODE_GRAVE]=102;
  */

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

  memset(willExport,1,32*sizeof(bool));

  peak[0]=0;
  peak[1]=0;

  memset(actionKeys,0,GUI_ACTION_MAX*sizeof(int));

  memset(patChanX,0,sizeof(float)*(DIV_MAX_CHANS+1));
  memset(patChanSlideY,0,sizeof(float)*(DIV_MAX_CHANS+1));
  memset(lastIns,-1,sizeof(int)*DIV_MAX_CHANS);
}
