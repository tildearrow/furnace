#include "gui.h"
#include "SDL_clipboard.h"
#include "SDL_events.h"
#include "SDL_keycode.h"
#include "SDL_render.h"
#include "SDL_video.h"
#include "fonts.h"
#include "icon.h"
#include "../ta-log.h"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include "imgui_internal.h"
#include "ImGuiFileDialog.h"
#include "IconsFontAwesome4.h"
#include "plot_nolerp.h"
#include "misc/cpp/imgui_stdlib.h"
#include <zlib.h>
#include <fmt/printf.h>
#include <stdexcept>

#ifdef _WIN32
#include <windows.h>
#define LAYOUT_INI "\\layout.ini"
#else
#include <unistd.h>
#include <pwd.h>
#define LAYOUT_INI "/layout.ini"
#endif

const int _ZERO=0;
const int _ONE=1;
const int _THREE=3;
const int _SEVEN=7;
const int _NINE=9;
const int _TEN=10;
const int _FIFTEEN=15;
const int _THIRTY_ONE=31;
const int _SIXTY_FOUR=64;
const int _ONE_HUNDRED=100;
const int _ONE_HUNDRED_TWENTY_SEVEN=127;
const int _TWO_THOUSAND_FORTY_SEVEN=2047;
const int _FOUR_THOUSAND_NINETY_FIVE=4095;
const int _MINUS_ONE_HUNDRED_TWENTY_SEVEN=-127;

const FurnaceGUIColors fxColors[16]={
  GUI_COLOR_PATTERN_EFFECT_MISC, // 00
  GUI_COLOR_PATTERN_EFFECT_PITCH, // 01
  GUI_COLOR_PATTERN_EFFECT_PITCH, // 02
  GUI_COLOR_PATTERN_EFFECT_PITCH, // 03
  GUI_COLOR_PATTERN_EFFECT_PITCH, // 04
  GUI_COLOR_PATTERN_EFFECT_VOLUME, // 05
  GUI_COLOR_PATTERN_EFFECT_VOLUME, // 06
  GUI_COLOR_PATTERN_EFFECT_VOLUME, // 07
  GUI_COLOR_PATTERN_EFFECT_PANNING, // 08
  GUI_COLOR_PATTERN_EFFECT_SPEED, // 09
  GUI_COLOR_PATTERN_EFFECT_VOLUME, // 0A
  GUI_COLOR_PATTERN_EFFECT_SONG, // 0B
  GUI_COLOR_PATTERN_EFFECT_TIME, // 0C
  GUI_COLOR_PATTERN_EFFECT_SONG, // 0D
  GUI_COLOR_PATTERN_EFFECT_INVALID, // 0E
  GUI_COLOR_PATTERN_EFFECT_SPEED, // 0F
};

const FurnaceGUIColors extFxColors[16]={
  GUI_COLOR_PATTERN_EFFECT_MISC, // E0
  GUI_COLOR_PATTERN_EFFECT_PITCH, // E1
  GUI_COLOR_PATTERN_EFFECT_PITCH, // E2
  GUI_COLOR_PATTERN_EFFECT_MISC, // E3
  GUI_COLOR_PATTERN_EFFECT_MISC, // E4
  GUI_COLOR_PATTERN_EFFECT_PITCH, // E5
  GUI_COLOR_PATTERN_EFFECT_INVALID, // E6
  GUI_COLOR_PATTERN_EFFECT_INVALID, // E7
  GUI_COLOR_PATTERN_EFFECT_INVALID, // E8
  GUI_COLOR_PATTERN_EFFECT_INVALID, // E9
  GUI_COLOR_PATTERN_EFFECT_MISC, // EA
  GUI_COLOR_PATTERN_EFFECT_MISC, // EB
  GUI_COLOR_PATTERN_EFFECT_TIME, // EC
  GUI_COLOR_PATTERN_EFFECT_TIME, // ED
  GUI_COLOR_PATTERN_EFFECT_SONG, // EE
  GUI_COLOR_PATTERN_EFFECT_SONG, // EF
};

const int opOrder[4]={
  0, 2, 1, 3
};

const char* noteNames[120]={
  "C-0", "C#0", "D-0", "D#0", "E-0", "F-0", "F#0", "G-0", "G#0", "A-0", "A#0", "B-0",
  "C-1", "C#1", "D-1", "D#1", "E-1", "F-1", "F#1", "G-1", "G#1", "A-1", "A#1", "B-1",
  "C-2", "C#2", "D-2", "D#2", "E-2", "F-2", "F#2", "G-2", "G#2", "A-2", "A#2", "B-2",
  "C-3", "C#3", "D-3", "D#3", "E-3", "F-3", "F#3", "G-3", "G#3", "A-3", "A#3", "B-3",
  "C-4", "C#4", "D-4", "D#4", "E-4", "F-4", "F#4", "G-4", "G#4", "A-4", "A#4", "B-4",
  "C-5", "C#5", "D-5", "D#5", "E-5", "F-5", "F#5", "G-5", "G#5", "A-5", "A#5", "B-5",
  "C-6", "C#6", "D-6", "D#6", "E-6", "F-6", "F#6", "G-6", "G#6", "A-6", "A#6", "B-6",
  "C-7", "C#7", "D-7", "D#7", "E-7", "F-7", "F#7", "G-7", "G#7", "A-7", "A#7", "B-7",
  "C-8", "C#8", "D-8", "D#8", "E-8", "F-8", "F#8", "G-8", "G#8", "A-8", "A#8", "B-8",
  "C-9", "C#9", "D-9", "D#9", "E-9", "F-9", "F#9", "G-9", "G#9", "A-9", "A#9", "B-9"
};

const char* pitchLabel[11]={
  "1/6", "1/5", "1/4", "1/3", "1/2", "1x", "2x", "3x", "4x", "5x", "6x"
};

void FurnaceGUI::bindEngine(DivEngine* eng) {
  e=eng;
}

const char* FurnaceGUI::noteName(short note, short octave) {
  if (note==100) {
    return "OFF";
  } else if (octave==0 && note==0) {
    return "...";
  }
  int seek=note+octave*12;
  if (seek>=120) return "???";
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
  for (int i=0; i<120; i++) {
    if (strcmp(what,noteNames[i])==0) {
      if ((i%12)==0) {
        note=12;
        octave=(i/12)-1;
      } else {
        note=i%12;
        octave=i/12;
      }
      return true;
    }
  }
  return false;
}

void FurnaceGUI::updateScroll(int amount) {
  float lineHeight=(ImGui::GetTextLineHeight()+2*dpiScale);
  nextScroll=lineHeight*amount;
}

void FurnaceGUI::addScroll(int amount) {
  float lineHeight=(ImGui::GetTextLineHeight()+2*dpiScale);
  nextAddScroll=lineHeight*amount;
}

void FurnaceGUI::updateWindowTitle() {
  String type=e->getSystemName(e->song.system[0]);
  if (e->song.systemLen>1) type="multi-system";
  if (e->song.name.empty()) {
    SDL_SetWindowTitle(sdlWin,fmt::sprintf("Furnace (%s)",type).c_str());
  } else {
    SDL_SetWindowTitle(sdlWin,fmt::sprintf("%s - Furnace (%s)",e->song.name,type).c_str());
  }
}

const char* defaultLayout="[Window][DockSpaceViewport_11111111]\n\
Pos=0,24\n\
Size=1280,800\n\
Collapsed=0\n\
\n\
[Window][Debug##Default]\n\
Pos=60,60\n\
Size=400,400\n\
Collapsed=0\n\
\n\
[Window][Play/Edit Controls]\n\
Pos=351,24\n\
Size=220,231\n\
Collapsed=0\n\
DockId=0x00000007,0\n\
\n\
[Window][Song Information]\n\
Pos=904,24\n\
Size=376,231\n\
Collapsed=0\n\
DockId=0x00000004,0\n\
\n\
[Window][Orders]\n\
Pos=0,24\n\
Size=349,231\n\
Collapsed=0\n\
DockId=0x00000005,0\n\
\n\
[Window][Instruments]\n\
Pos=573,24\n\
Size=329,231\n\
Collapsed=0\n\
DockId=0x00000008,1\n\
\n\
[Window][Wavetables]\n\
Pos=573,24\n\
Size=329,231\n\
Collapsed=0\n\
DockId=0x00000008,2\n\
\n\
[Window][Samples]\n\
Pos=573,24\n\
Size=329,231\n\
Collapsed=0\n\
DockId=0x00000008,0\n\
\n\
[Window][Pattern]\n\
Pos=0,257\n\
Size=1280,800\n\
Collapsed=0\n\
DockId=0x00000002,0\n\
\n\
[Docking][Data]\n\
DockSpace         ID=0x8B93E3BD Window=0xA787BDB4 Pos=0,24 Size=1280,800 Split=Y Selected=0x6C01C512\n\
  DockNode        ID=0x00000001 Parent=0x8B93E3BD SizeRef=1280,231 Split=X Selected=0xF3094A52\n\
    DockNode      ID=0x00000003 Parent=0x00000001 SizeRef=902,231 Split=X Selected=0x65CC51DC\n\
      DockNode    ID=0x00000005 Parent=0x00000003 SizeRef=349,231 Selected=0xE283F8D8\n\
      DockNode    ID=0x00000006 Parent=0x00000003 SizeRef=551,231 Split=X Selected=0x756E3877\n\
        DockNode  ID=0x00000007 Parent=0x00000006 SizeRef=220,231 Selected=0xD2BA8AA2\n\
        DockNode  ID=0x00000008 Parent=0x00000006 SizeRef=329,231 Selected=0x756E3877\n\
    DockNode      ID=0x00000004 Parent=0x00000001 SizeRef=376,231 Selected=0xF3094A52\n\
  DockNode        ID=0x00000002 Parent=0x8B93E3BD SizeRef=1280,498 CentralNode=1 Selected=0x6C01C512\n\
\n\
";

void FurnaceGUI::prepareLayout() {
  FILE* check;
  check=fopen(finalLayoutPath,"r");
  if (check!=NULL) {
    fclose(check);
    return;
  }

  // copy initial layout
  logI("loading default layout.\n");
  check=fopen(finalLayoutPath,"w");
  if (check==NULL) {
    logW("could not write default layout!\n");
    return;
  }

  fwrite(defaultLayout,1,strlen(defaultLayout),check);
  fclose(check);
}

void FurnaceGUI::drawEditControls() {
  if (!editControlsOpen) return;
  if (ImGui::Begin("Play/Edit Controls",&editControlsOpen)) {
    ImGui::Text("Octave");
    ImGui::SameLine();
    if (ImGui::InputInt("##Octave",&curOctave,1,1)) {
      if (curOctave>6) curOctave=6;
      if (curOctave<0) curOctave=0;
    }

    ImGui::Text("Edit Step");
    ImGui::SameLine();
    if (ImGui::InputInt("##EditStep",&editStep,1,1)) {
      if (editStep>=e->song.patLen) editStep=e->song.patLen-1;
      if (editStep<0) editStep=0;
    }

    if (ImGui::Button(ICON_FA_PLAY "##Play")) {
      e->play();
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_STOP "##Stop")) {
      e->stop();
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
  }
  if (ImGui::IsWindowFocused()) curWindow=GUI_WINDOW_EDIT_CONTROLS;
  ImGui::End();
}

void FurnaceGUI::drawSongInfo() {
  if (!songInfoOpen) return;
  if (ImGui::Begin("Song Information",&songInfoOpen)) {
    ImGui::Text("Name");
    ImGui::SameLine();
    if (ImGui::InputText("##Name",&e->song.name)) updateWindowTitle();
    ImGui::Text("Author");
    ImGui::SameLine();
    ImGui::InputText("##Author",&e->song.author);

    ImGui::Text("TimeBase");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120.0f*dpiScale);
    unsigned char realTB=e->song.timeBase+1;
    if (ImGui::InputScalar("##TimeBase",ImGuiDataType_U8,&realTB,&_ONE,&_THREE)) {
      if (realTB<1) realTB=1;
      if (realTB>16) realTB=16;
      e->song.timeBase=realTB-1;
    }

    ImGui::Text("Speed");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120.0f*dpiScale);
    ImGui::InputScalar("##Speed1",ImGuiDataType_U8,&e->song.speed1,&_ONE,&_THREE);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120.0f*dpiScale);
    ImGui::InputScalar("##Speed2",ImGuiDataType_U8,&e->song.speed2,&_ONE,&_THREE);

    ImGui::Text("Highlight");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120.0f*dpiScale);
    ImGui::InputScalar("##Highlight1",ImGuiDataType_U8,&e->song.hilightA,&_ONE,&_THREE);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120.0f*dpiScale);
    ImGui::InputScalar("##Highlight2",ImGuiDataType_U8,&e->song.hilightB,&_ONE,&_THREE);

    ImGui::Text("Pattern Length");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120.0f*dpiScale);
    int patLen=e->song.patLen;
    if (ImGui::InputInt("##PatLength",&patLen,1,3)) {
      if (patLen<1) patLen=1;
      if (patLen>256) patLen=256;
      e->song.patLen=patLen;
    }

    ImGui::Text("Song Length");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120.0f*dpiScale);
    int ordLen=e->song.ordersLen;
    if (ImGui::InputInt("##OrdLength",&ordLen,1,3)) {
      if (ordLen<1) ordLen=1;
      if (ordLen>127) ordLen=127;
      e->song.ordersLen=ordLen;
    }

    ImGui::Text("Rate");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120.0f*dpiScale);
    int setHz=e->song.hz;
    if (ImGui::InputInt("##Rate",&setHz)) {
      if (setHz<10) setHz=10;
      if (setHz>999) setHz=999;
      e->setSongRate(setHz,setHz<52);
    }
    if (e->song.hz==50) {
      ImGui::SameLine();
      ImGui::Text("PAL");
    }
    if (e->song.hz==60) {
      ImGui::SameLine();
      ImGui::Text("NTSC");
    }
  }
  if (ImGui::IsWindowFocused()) curWindow=GUI_WINDOW_SONG_INFO;
  ImGui::End();
}

void FurnaceGUI::drawOrders() {
  char selID[64];
  if (!ordersOpen) return;
  if (ImGui::Begin("Orders",&ordersOpen)) {
    float regionX=ImGui::GetContentRegionAvail().x;
    ImVec2 prevSpacing=ImGui::GetStyle().ItemSpacing;
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,ImVec2(1.0f*dpiScale,1.0f*dpiScale));
    ImGui::Columns(2,NULL,false);
    ImGui::SetColumnWidth(-1,regionX-24.0f*dpiScale);
    if (ImGui::BeginTable("OrdersTable",1+e->getTotalChannelCount(),ImGuiTableFlags_SizingStretchSame|ImGuiTableFlags_ScrollX|ImGuiTableFlags_ScrollY)) {
      ImGui::PushFont(patFont);
      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,prevSpacing);
      ImGui::TableSetupScrollFreeze(1,1);
      float lineHeight=(ImGui::GetTextLineHeight()+4*dpiScale);
      if (e->isPlaying()) {
        if (followOrders) {
          ImGui::SetScrollY((e->getOrder()+1)*lineHeight-(ImGui::GetContentRegionAvail().y/2));
        }
      }
      ImGui::TableNextRow(0,lineHeight);
      ImGui::TableNextColumn();
      ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_ROW_INDEX]);
      for (int i=0; i<e->getTotalChannelCount(); i++) {
        ImGui::TableNextColumn();
        ImGui::Text("%s",e->getChannelShortName(i));
      }
      ImGui::PopStyleColor();
      for (int i=0; i<e->song.ordersLen; i++) {
        ImGui::TableNextRow(0,lineHeight);
        if (oldOrder1==i) ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,0x40ffffff);
        ImGui::TableNextColumn();
        ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_ROW_INDEX]);
        snprintf(selID,64,"%.2x##O_S%.2x",i,i);
        if (ImGui::Selectable(selID)) {
          e->setOrder(i);
        }
        ImGui::PopStyleColor();
        for (int j=0; j<e->getTotalChannelCount(); j++) {
          ImGui::TableNextColumn();
          snprintf(selID,64,"%.2x##O_%.2x_%.2x",e->song.orders.ord[j][i],j,i);
          if (ImGui::Selectable(selID)) {
            if (e->getOrder()==i) {
              prepareUndo(GUI_ACTION_CHANGE_ORDER);
              if (changeAllOrders) {
                for (int k=0; k<e->getTotalChannelCount(); k++) {
                  if (e->song.orders.ord[k][i]<0x7f) e->song.orders.ord[k][i]++;
                }
              } else {
                if (e->song.orders.ord[j][i]<0x7f) e->song.orders.ord[j][i]++;
              }
              makeUndo(GUI_ACTION_CHANGE_ORDER);
            } else {
              e->setOrder(i);
            }
          }
          if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
            if (e->getOrder()==i) {
              prepareUndo(GUI_ACTION_CHANGE_ORDER);
              if (changeAllOrders) {
                for (int k=0; k<e->getTotalChannelCount(); k++) {
                  if (e->song.orders.ord[k][i]>0) e->song.orders.ord[k][i]--;
                }
              } else {
                if (e->song.orders.ord[j][i]>0) e->song.orders.ord[j][i]--;
              }
              makeUndo(GUI_ACTION_CHANGE_ORDER);
            } else {
              e->setOrder(i);
            }
          }
        }
      }
      ImGui::PopStyleVar();
      ImGui::PopFont();
      ImGui::EndTable();
    }
    ImGui::NextColumn();
    if (ImGui::Button(ICON_FA_PLUS)) {
      // add order row (new)
      prepareUndo(GUI_ACTION_CHANGE_ORDER);
      e->addOrder(false,false);
      makeUndo(GUI_ACTION_CHANGE_ORDER);
    }
    if (ImGui::Button(ICON_FA_MINUS)) {
      // remove this order row
      prepareUndo(GUI_ACTION_CHANGE_ORDER);
      e->deleteOrder();
      makeUndo(GUI_ACTION_CHANGE_ORDER);
    }
    if (ImGui::Button(ICON_FA_FILES_O)) {
      // duplicate order row
      prepareUndo(GUI_ACTION_CHANGE_ORDER);
      e->addOrder(true,false);
      makeUndo(GUI_ACTION_CHANGE_ORDER);
    }
    if (ImGui::Button(ICON_FA_ANGLE_UP)) {
      // move order row up
      prepareUndo(GUI_ACTION_CHANGE_ORDER);
      e->moveOrderUp();
      makeUndo(GUI_ACTION_CHANGE_ORDER);
    }
    if (ImGui::Button(ICON_FA_ANGLE_DOWN)) {
      // move order row down
      prepareUndo(GUI_ACTION_CHANGE_ORDER);
      e->moveOrderDown();
      makeUndo(GUI_ACTION_CHANGE_ORDER);
    }
    if (ImGui::Button(ICON_FA_ANGLE_DOUBLE_DOWN)) {
      // duplicate order row at end
      prepareUndo(GUI_ACTION_CHANGE_ORDER);
      e->addOrder(true,true);
      makeUndo(GUI_ACTION_CHANGE_ORDER);
    }
    if (ImGui::Button(changeAllOrders?ICON_FA_LINK"##ChangeAll":ICON_FA_CHAIN_BROKEN"##ChangeAll")) {
      // whether to change one or all orders in a row
      changeAllOrders=!changeAllOrders;
    }
    ImGui::PopStyleVar();
  }
  if (ImGui::IsWindowFocused()) curWindow=GUI_WINDOW_ORDERS;
  oldOrder1=e->getOrder();
  ImGui::End();
}

void FurnaceGUI::drawInsList() {
  if (!insListOpen) return;
  if (ImGui::Begin("Instruments",&insListOpen)) {
    if (ImGui::Button(ICON_FA_PLUS "##InsAdd")) {
      curIns=e->addInstrument(cursor.xCoarse);
      modified=true;
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("InsUp",ImGuiDir_Up)) {
      if (e->moveInsUp(curIns)) curIns--;
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("InsDown",ImGuiDir_Down)) {
      if (e->moveInsDown(curIns)) curIns++;
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_TIMES "##InsDelete")) {
      e->delInstrument(curIns);
      modified=true;
      if (curIns>=(int)e->song.ins.size()) {
        curIns--;
      }
    }
    ImGui::Separator();
    for (int i=0; i<(int)e->song.ins.size(); i++) {
      DivInstrument* ins=e->song.ins[i];
      String name;
      switch (ins->type) {
        case DIV_INS_FM:
          ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_FM]);
          name=fmt::sprintf(ICON_FA_AREA_CHART " %.2x: %s##_INS%d\n",i,ins->name,i);
          break;
        case DIV_INS_STD:
          ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_STD]);
          name=fmt::sprintf(ICON_FA_BAR_CHART " %.2x: %s##_INS%d\n",i,ins->name,i);
          break;
        case DIV_INS_GB:
          ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_GB]);
          name=fmt::sprintf(ICON_FA_GAMEPAD " %.2x: %s##_INS%d\n",i,ins->name,i);
          break;
        case DIV_INS_C64:
          ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_C64]);
          name=fmt::sprintf(ICON_FA_KEYBOARD_O " %.2x: %s##_INS%d\n",i,ins->name,i);
          break;
        case DIV_INS_AMIGA:
          ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_AMIGA]);
          name=fmt::sprintf(ICON_FA_FUTBOL_O " %.2x: %s##_INS%d\n",i,ins->name,i);
          break;
        case DIV_INS_PCE:
          ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_PCE]);
          name=fmt::sprintf(ICON_FA_ID_BADGE " %.2x: %s##_INS%d\n",i,ins->name,i);
          break;
        case DIV_INS_AY:
          ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_AY]);
          name=fmt::sprintf(ICON_FA_BAR_CHART " %.2x: %s##_INS%d\n",i,ins->name,i);
          break;
        case DIV_INS_AY8930:
          ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_AY8930]);
          name=fmt::sprintf(ICON_FA_BAR_CHART " %.2x: %s##_INS%d\n",i,ins->name,i);
          break;
        case DIV_INS_TIA:
          ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_TIA]);
          name=fmt::sprintf(ICON_FA_BAR_CHART " %.2x: %s##_INS%d\n",i,ins->name,i);
          break;
        case DIV_INS_SAA1099:
          ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_SAA1099]);
          name=fmt::sprintf(ICON_FA_BAR_CHART " %.2x: %s##_INS%d\n",i,ins->name,i);
          break;
        default:
          ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_UNKNOWN]);
          name=fmt::sprintf(ICON_FA_QUESTION " %.2x: %s##_INS%d\n",i,ins->name,i);
          break;
      }
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
  }
  if (ImGui::IsWindowFocused()) curWindow=GUI_WINDOW_INS_LIST;
  ImGui::End();
}

int detuneTable[8]={
  0, 1, 2, 3, 0, -3, -2, -1
};

const char* insTypes[10]={
  "Standard", "FM", "Game Boy", "C64", "Amiga", "PC Engine", "AY-3-8910/SSG", "AY8930", "TIA", "SAA1099"
};

const char* ssgEnvTypes[8]={
  "Down Down Down", "Down.", "Down Up Down Up", "Down UP", "Up Up Up", "Up.", "Up Down Up Down", "Up DOWN"
};

void FurnaceGUI::drawInsEdit() {
  if (!insEditOpen) return;
  if (ImGui::Begin("Instrument Editor",&insEditOpen,ImGuiWindowFlags_NoDocking)) {
    if (curIns<0 || curIns>=(int)e->song.ins.size()) {
      ImGui::Text("no instrument selected");
    } else {
      DivInstrument* ins=e->song.ins[curIns];
      ImGui::InputText("Name",&ins->name);
      if (ins->type<0 || ins->type>9) ins->type=DIV_INS_FM;
      if (ImGui::SliderScalar("Type",ImGuiDataType_U8,&ins->type,&_ZERO,&_NINE,insTypes[ins->type])) {
        ins->mode=(ins->type==DIV_INS_FM);
      }

      if (ImGui::BeginTabBar("insEditTab")) {
        if (ins->type==DIV_INS_FM) if (ImGui::BeginTabItem("FM")) {
          ImGui::Columns(3,NULL,false);
          ImGui::SliderScalar("Algorithm",ImGuiDataType_U8,&ins->fm.alg,&_ZERO,&_SEVEN);
          ImGui::NextColumn();
          ImGui::SliderScalar("Feedback",ImGuiDataType_U8,&ins->fm.fb,&_ZERO,&_SEVEN);
          ImGui::NextColumn();
          ImGui::Text("Algorithm here!");
          ImGui::NextColumn();
          ImGui::SliderScalar("LFO > Freq",ImGuiDataType_U8,&ins->fm.fms,&_ZERO,&_SEVEN);
          ImGui::NextColumn();
          ImGui::SliderScalar("LFO > Amp",ImGuiDataType_U8,&ins->fm.ams,&_ZERO,&_THREE);
          ImGui::Columns(1);
          if (ImGui::BeginTable("FMOperators",2)) {
            for (int i=0; i<4; i++) {
              DivInstrumentFM::Operator& op=ins->fm.op[opOrder[i]];
              if ((i+1)&1) ImGui::TableNextRow();
              ImGui::TableNextColumn();

              ImGui::PushID(fmt::sprintf("op%d",i).c_str());
              ImGui::Text("Operator %d",i+1);
              ImGui::SliderScalar("Level",ImGuiDataType_U8,&op.tl,&_ZERO,&_ONE_HUNDRED_TWENTY_SEVEN);
              ImGui::SliderScalar("Attack",ImGuiDataType_U8,&op.ar,&_ZERO,&_THIRTY_ONE);
              ImGui::SliderScalar("Decay",ImGuiDataType_U8,&op.dr,&_ZERO,&_THIRTY_ONE);
              ImGui::SliderScalar("Sustain",ImGuiDataType_U8,&op.sl,&_ZERO,&_FIFTEEN);
              ImGui::SliderScalar("Decay 2",ImGuiDataType_U8,&op.d2r,&_ZERO,&_THIRTY_ONE);
              ImGui::SliderScalar("Release",ImGuiDataType_U8,&op.rr,&_ZERO,&_FIFTEEN);

              ImGui::SliderScalar("Multiplier",ImGuiDataType_U8,&op.mult,&_ZERO,&_FIFTEEN);
              ImGui::SliderScalar("EnvScale",ImGuiDataType_U8,&op.rs,&_ZERO,&_THREE);
              int detune=detuneTable[op.dt&7];
              if (ImGui::SliderInt("Detune",&detune,-3,3)) {
                op.dt=detune&7;
              }
              ImGui::SliderScalar("Detune 2",ImGuiDataType_U8,&op.dt2,&_ZERO,&_THREE);
              if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Only for Arcade platform");
              }
              bool ssgOn=op.ssgEnv&8;
              unsigned char ssgEnv=op.ssgEnv&7;
              if (ImGui::SliderScalar("SSG-EG",ImGuiDataType_U8,&ssgEnv,&_ZERO,&_SEVEN,ssgEnvTypes[ssgEnv])) {
                op.ssgEnv=(op.ssgEnv&8)|(ssgEnv&7);
              }
              ImGui::SameLine();
              if (ImGui::Checkbox("##SSGOn",&ssgOn)) {
                op.ssgEnv=(op.ssgEnv&7)|(ssgOn<<3);
              }
              if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Only for Genesis and Neo Geo platforms");
              }

              bool amOn=op.am;
              if (ImGui::Checkbox("AM",&amOn)) op.am=amOn;
              ImGui::PopID();
            }
            ImGui::EndTable();
          }
          ImGui::EndTabItem();
        }
        if (ins->type==DIV_INS_GB) if (ImGui::BeginTabItem("Game Boy")) {
          ImGui::SliderScalar("Volume",ImGuiDataType_U8,&ins->gb.envVol,&_ZERO,&_FIFTEEN);
          ImGui::SliderScalar("Envelope Length",ImGuiDataType_U8,&ins->gb.envLen,&_ZERO,&_SEVEN);
          ImGui::SliderScalar("Sound Length",ImGuiDataType_U8,&ins->gb.soundLen,&_ZERO,&_SIXTY_FOUR,ins->gb.soundLen>63?"Infinity":"%d");
          bool goesUp=ins->gb.envDir;
          if (ImGui::Checkbox("Up",&goesUp)) {
            ins->gb.envDir=goesUp;
          }
          ImGui::EndTabItem();
        }
        if (ins->type==DIV_INS_C64) if (ImGui::BeginTabItem("C64")) {
          ImGui::Text("Waveform");
          ImGui::SameLine();
          ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(ins->c64.triOn)?0.6f:0.2f,0.2f,1.0f));
          if (ImGui::Button("tri")) {
            ins->c64.triOn=!ins->c64.triOn;
          }
          ImGui::PopStyleColor();
          ImGui::SameLine();
          ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(ins->c64.sawOn)?0.6f:0.2f,0.2f,1.0f));
          if (ImGui::Button("saw")) {
            ins->c64.sawOn=!ins->c64.sawOn;
          }
          ImGui::PopStyleColor();
          ImGui::SameLine();
          ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(ins->c64.pulseOn)?0.6f:0.2f,0.2f,1.0f));
          if (ImGui::Button("pulse")) {
            ins->c64.pulseOn=!ins->c64.pulseOn;
          }
          ImGui::PopStyleColor();
          ImGui::SameLine();
          ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(ins->c64.noiseOn)?0.6f:0.2f,0.2f,1.0f));
          if (ImGui::Button("noise")) {
            ins->c64.noiseOn=!ins->c64.noiseOn;
          }
          ImGui::PopStyleColor();

          ImGui::SliderScalar("Attack",ImGuiDataType_U8,&ins->c64.a,&_ZERO,&_FIFTEEN);
          ImGui::SliderScalar("Decay",ImGuiDataType_U8,&ins->c64.d,&_ZERO,&_FIFTEEN);
          ImGui::SliderScalar("Sustain",ImGuiDataType_U8,&ins->c64.s,&_ZERO,&_FIFTEEN);
          ImGui::SliderScalar("Release",ImGuiDataType_U8,&ins->c64.r,&_ZERO,&_FIFTEEN);
          ImGui::SliderScalar("Duty",ImGuiDataType_U16,&ins->c64.duty,&_ZERO,&_FOUR_THOUSAND_NINETY_FIVE);

          bool ringMod=ins->c64.ringMod;
          if (ImGui::Checkbox("Ring Modulation",&ringMod)) ins->c64.ringMod=ringMod;
          bool oscSync=ins->c64.oscSync;
          if (ImGui::Checkbox("Oscillator Sync",&oscSync)) ins->c64.oscSync=oscSync;

          ImGui::Checkbox("Enable filter",&ins->c64.toFilter);
          ImGui::Checkbox("Initialize filter",&ins->c64.initFilter);
          
          ImGui::SliderScalar("Cutoff",ImGuiDataType_U16,&ins->c64.cut,&_ZERO,&_TWO_THOUSAND_FORTY_SEVEN);
          ImGui::SliderScalar("Resonance",ImGuiDataType_U8,&ins->c64.res,&_ZERO,&_FIFTEEN);

          ImGui::Text("Filter Mode");
          ImGui::SameLine();
          ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(ins->c64.lp)?0.6f:0.2f,0.2f,1.0f));
          if (ImGui::Button("low")) {
            ins->c64.lp=!ins->c64.lp;
          }
          ImGui::PopStyleColor();
          ImGui::SameLine();
          ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(ins->c64.bp)?0.6f:0.2f,0.2f,1.0f));
          if (ImGui::Button("band")) {
            ins->c64.bp=!ins->c64.bp;
          }
          ImGui::PopStyleColor();
          ImGui::SameLine();
          ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(ins->c64.hp)?0.6f:0.2f,0.2f,1.0f));
          if (ImGui::Button("high")) {
            ins->c64.hp=!ins->c64.hp;
          }
          ImGui::PopStyleColor();
          ImGui::SameLine();
          ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(ins->c64.ch3off)?0.6f:0.2f,0.2f,1.0f));
          if (ImGui::Button("ch3off")) {
            ins->c64.ch3off=!ins->c64.ch3off;
          }
          ImGui::PopStyleColor();

          ImGui::Checkbox("Volume Macro is Cutoff Macro",&ins->c64.volIsCutoff);
          ImGui::Checkbox("Absolute Cutoff Macro",&ins->c64.filterIsAbs);
          ImGui::Checkbox("Absolute Duty Macro",&ins->c64.dutyIsAbs);
          ImGui::EndTabItem();
        }
        if (ins->type==DIV_INS_AMIGA) if (ImGui::BeginTabItem("Amiga")) {
          ImGui::EndTabItem();
        }
        if (ins->type!=DIV_INS_FM) if (ImGui::BeginTabItem("Macros")) {
          float asFloat[128];
          float loopIndicator[128];

          // volume macro
          ImGui::Separator();
          if (ins->type==DIV_INS_C64 && ins->c64.volIsCutoff) {
            if (ins->c64.filterIsAbs) {
              ImGui::Text("Absolute Cutoff Macro");
            } else {
              ImGui::Text("Relative Cutoff Macro");
            }
          } else {
            ImGui::Text("Volume Macro");
          }
          for (int i=0; i<ins->std.volMacroLen; i++) {
            if (ins->type==DIV_INS_C64 && ins->c64.volIsCutoff && !ins->c64.filterIsAbs) {
              asFloat[i]=ins->std.volMacro[i]-18;
            } else {
              asFloat[i]=ins->std.volMacro[i];
            }
            loopIndicator[i]=(ins->std.volMacroLoop!=-1 && i>=ins->std.volMacroLoop);
          }
          ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(0.0f,0.0f));
          int volMax=(ins->type==DIV_INS_PCE || ins->type==DIV_INS_AY8930)?31:15;
          int volMin=0;
          if (ins->type==DIV_INS_C64) {
            if (ins->c64.volIsCutoff) {
              if (ins->c64.filterIsAbs) {
                volMax=2047;
              } else {
                volMin=-18;
                volMax=18;
              }
            }
          }
          ImGui::PlotHistogram("##IVolMacro",asFloat,ins->std.volMacroLen,0,NULL,volMin,volMax,ImVec2(400.0f*dpiScale,200.0f*dpiScale));
          if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            macroDragStart=ImGui::GetItemRectMin();
            macroDragAreaSize=ImVec2(400.0f*dpiScale,200.0f*dpiScale);
            macroDragMin=volMin;
            macroDragMax=volMax;
            macroDragLen=ins->std.volMacroLen;
            macroDragActive=true;
            macroDragTarget=ins->std.volMacro;
            processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
          }
          ImGui::PlotHistogram("##IVolMacroLoop",loopIndicator,ins->std.volMacroLen,0,NULL,0,1,ImVec2(400.0f*dpiScale,16.0f*dpiScale));
          if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            macroLoopDragStart=ImGui::GetItemRectMin();
            macroLoopDragAreaSize=ImVec2(400.0f*dpiScale,16.0f*dpiScale);
            macroLoopDragLen=ins->std.volMacroLen;
            macroLoopDragTarget=&ins->std.volMacroLoop;
            macroLoopDragActive=true;
            processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
          }
          ImGui::PopStyleVar();
          if (ImGui::InputScalar("Length##IVolMacroL",ImGuiDataType_U8,&ins->std.volMacroLen,&_ONE,&_THREE)) {
            if (ins->std.volMacroLen>127) ins->std.volMacroLen=127;
          }

          // arp macro
          ImGui::Separator();
          ImGui::Text("Arpeggio Macro");
          bool arpMode=ins->std.arpMacroMode;
          for (int i=0; i<ins->std.arpMacroLen; i++) {
            asFloat[i]=arpMode?ins->std.arpMacro[i]:(ins->std.arpMacro[i]-12);
            loopIndicator[i]=(ins->std.arpMacroLoop!=-1 && i>=ins->std.arpMacroLoop);
          }
          ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(0.0f,0.0f));
          ImGui::PlotHistogram("##IArpMacro",asFloat,ins->std.arpMacroLen,0,NULL,arpMode?arpMacroScroll:(arpMacroScroll-12),arpMacroScroll+(arpMode?24:12),ImVec2(400.0f*dpiScale,200.0f*dpiScale));
          if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            macroDragStart=ImGui::GetItemRectMin();
            macroDragAreaSize=ImVec2(400.0f*dpiScale,200.0f*dpiScale);
            macroDragMin=arpMacroScroll;
            macroDragMax=arpMacroScroll+24;
            macroDragLen=ins->std.arpMacroLen;
            macroDragActive=true;
            macroDragTarget=ins->std.arpMacro;
            processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
          }
          ImGui::SameLine();
          ImGui::VSliderInt("##IArpMacroPos",ImVec2(20.0f*dpiScale,200.0f*dpiScale),&arpMacroScroll,arpMode?0:-80,70);
          ImGui::PlotHistogram("##IArpMacroLoop",loopIndicator,ins->std.arpMacroLen,0,NULL,0,1,ImVec2(400.0f*dpiScale,16.0f*dpiScale));
          if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            macroLoopDragStart=ImGui::GetItemRectMin();
            macroLoopDragAreaSize=ImVec2(400.0f*dpiScale,16.0f*dpiScale);
            macroLoopDragLen=ins->std.arpMacroLen;
            macroLoopDragTarget=&ins->std.arpMacroLoop;
            macroLoopDragActive=true;
            processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
          }
          ImGui::PopStyleVar();
          if (ImGui::InputScalar("Length##IArpMacroL",ImGuiDataType_U8,&ins->std.arpMacroLen,&_ONE,&_THREE)) {
            if (ins->std.arpMacroLen>127) ins->std.arpMacroLen=127;
          }
          if (ImGui::Checkbox("Absolute",&arpMode)) {
            ins->std.arpMacroMode=arpMode;
            if (arpMode) {
              if (arpMacroScroll<0) arpMacroScroll=0;
            }
          }

          // duty macro
          int dutyMax=(ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930)?31:3;
          if (ins->type==DIV_INS_C64) {
            if (ins->c64.dutyIsAbs) {
              dutyMax=4095;
            } else {
              dutyMax=24;
            }
          }
          if (ins->type==DIV_INS_AY8930) {
            dutyMax=255;
          }
          if (ins->type==DIV_INS_TIA) {
            dutyMax=0;
          }
          if (dutyMax>0) {
            ImGui::Separator();
            if (ins->type==DIV_INS_C64) {
              if (ins->c64.dutyIsAbs) {
                ImGui::Text("Absolute Duty Macro");
              } else {
                ImGui::Text("Relative Duty Macro");
              }
            } else {
              if (ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930 || ins->type==DIV_INS_SAA1099) {
                ImGui::Text("Noise Frequency Macro");
              } else {
                ImGui::Text("Duty/Noise Mode Macro");
              }
            }
            bool dutyIsRel=(ins->type==DIV_INS_C64 && !ins->c64.dutyIsAbs);
            for (int i=0; i<ins->std.dutyMacroLen; i++) {
              asFloat[i]=ins->std.dutyMacro[i]-(dutyIsRel?12:0);
              loopIndicator[i]=(ins->std.dutyMacroLoop!=-1 && i>=ins->std.dutyMacroLoop);
            }
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(0.0f,0.0f));
            
            ImGui::PlotHistogram("##IDutyMacro",asFloat,ins->std.dutyMacroLen,0,NULL,dutyIsRel?-12:0,dutyMax-(dutyIsRel?12:0),ImVec2(400.0f*dpiScale,200.0f*dpiScale));
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
              macroDragStart=ImGui::GetItemRectMin();
              macroDragAreaSize=ImVec2(400.0f*dpiScale,200.0f*dpiScale);
              macroDragMin=0;
              macroDragMax=dutyMax;
              macroDragLen=ins->std.dutyMacroLen;
              macroDragActive=true;
              macroDragTarget=ins->std.dutyMacro;
              processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
            }
            ImGui::PlotHistogram("##IDutyMacroLoop",loopIndicator,ins->std.dutyMacroLen,0,NULL,0,1,ImVec2(400.0f*dpiScale,16.0f*dpiScale));
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
              macroLoopDragStart=ImGui::GetItemRectMin();
              macroLoopDragAreaSize=ImVec2(400.0f*dpiScale,16.0f*dpiScale);
              macroLoopDragLen=ins->std.dutyMacroLen;
              macroLoopDragTarget=&ins->std.dutyMacroLoop;
              macroLoopDragActive=true;
              processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
            }
            ImGui::PopStyleVar();
            if (ImGui::InputScalar("Length##IDutyMacroL",ImGuiDataType_U8,&ins->std.dutyMacroLen,&_ONE,&_THREE)) {
              if (ins->std.dutyMacroLen>127) ins->std.dutyMacroLen=127;
            }
          }

          // wave macro
          int waveMax=(ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930)?7:63;
          if (ins->type==DIV_INS_TIA) waveMax=15;
          if (ins->type==DIV_INS_C64) waveMax=8;
          if (ins->type==DIV_INS_SAA1099) waveMax=3;
          if (waveMax>0) {
            ImGui::Separator();
            ImGui::Text("Waveform Macro");
            for (int i=0; i<ins->std.waveMacroLen; i++) {
              asFloat[i]=ins->std.waveMacro[i];
              loopIndicator[i]=(ins->std.waveMacroLoop!=-1 && i>=ins->std.waveMacroLoop);
            }
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(0.0f,0.0f));
            
            ImGui::PlotHistogram("##IWaveMacro",asFloat,ins->std.waveMacroLen,0,NULL,0,waveMax,ImVec2(400.0f*dpiScale,200.0f*dpiScale));
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
              macroDragStart=ImGui::GetItemRectMin();
              macroDragAreaSize=ImVec2(400.0f*dpiScale,200.0f*dpiScale);
              macroDragMin=0;
              macroDragMax=waveMax;
              macroDragLen=ins->std.waveMacroLen;
              macroDragActive=true;
              macroDragTarget=ins->std.waveMacro;
              processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
            }
            ImGui::PlotHistogram("##IWaveMacroLoop",loopIndicator,ins->std.waveMacroLen,0,NULL,0,1,ImVec2(400.0f*dpiScale,16.0f*dpiScale));
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
              macroLoopDragStart=ImGui::GetItemRectMin();
              macroLoopDragAreaSize=ImVec2(400.0f*dpiScale,16.0f*dpiScale);
              macroLoopDragLen=ins->std.waveMacroLen;
              macroLoopDragTarget=&ins->std.waveMacroLoop;
              macroLoopDragActive=true;
              processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
            }
            ImGui::PopStyleVar();
            if (ImGui::InputScalar("Length##IWaveMacroL",ImGuiDataType_U8,&ins->std.waveMacroLen,&_ONE,&_THREE)) {
              if (ins->std.waveMacroLen>127) ins->std.waveMacroLen=127;
            }
          }

          // extra 1 macro
          int ex1Max=(ins->type==DIV_INS_AY8930)?15:0;
          if (ex1Max>0) {
            ImGui::Separator();
            if (ins->type==DIV_INS_AY8930) {
              ImGui::Text("Duty Macro");
            } else {
              ImGui::Text("Extra 1 Macro");
            }
            for (int i=0; i<ins->std.ex1MacroLen; i++) {
              asFloat[i]=ins->std.ex1Macro[i];
              loopIndicator[i]=(ins->std.ex1MacroLoop!=-1 && i>=ins->std.ex1MacroLoop);
            }
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(0.0f,0.0f));
            
            ImGui::PlotHistogram("##IEx1Macro",asFloat,ins->std.ex1MacroLen,0,NULL,0,ex1Max,ImVec2(400.0f*dpiScale,200.0f*dpiScale));
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
              macroDragStart=ImGui::GetItemRectMin();
              macroDragAreaSize=ImVec2(400.0f*dpiScale,200.0f*dpiScale);
              macroDragMin=0;
              macroDragMax=ex1Max;
              macroDragLen=ins->std.ex1MacroLen;
              macroDragActive=true;
              macroDragTarget=ins->std.ex1Macro;
              processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
            }
            ImGui::PlotHistogram("##IEx1MacroLoop",loopIndicator,ins->std.ex1MacroLen,0,NULL,0,1,ImVec2(400.0f*dpiScale,16.0f*dpiScale));
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
              macroLoopDragStart=ImGui::GetItemRectMin();
              macroLoopDragAreaSize=ImVec2(400.0f*dpiScale,16.0f*dpiScale);
              macroLoopDragLen=ins->std.ex1MacroLen;
              macroLoopDragTarget=&ins->std.ex1MacroLoop;
              macroLoopDragActive=true;
              processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
            }
            ImGui::PopStyleVar();
            if (ImGui::InputScalar("Length##IEx1MacroL",ImGuiDataType_U8,&ins->std.ex1MacroLen,&_ONE,&_THREE)) {
              if (ins->std.ex1MacroLen>127) ins->std.ex1MacroLen=127;
            }
          }

          ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
      }
    }
  }
  if (ImGui::IsWindowFocused()) curWindow=GUI_WINDOW_INS_EDIT;
  ImGui::End();
}

void FurnaceGUI::drawWaveList() {
  if (!waveListOpen) return;
  float wavePreview[256];
  if (ImGui::Begin("Wavetables",&waveListOpen)) {
    if (ImGui::Button(ICON_FA_PLUS "##WaveAdd")) {
      curWave=e->addWave();
      modified=true;
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("WaveUp",ImGuiDir_Up)) {
      if (e->moveWaveUp(curWave)) curWave--;
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("WaveDown",ImGuiDir_Down)) {
      if (e->moveWaveDown(curWave)) curWave++;
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_TIMES "##WaveDelete")) {
      e->delWave(curWave);
      modified=true;
      if (curWave>=(int)e->song.wave.size()) {
        curWave--;
      }
    }
    ImGui::Separator();
    for (int i=0; i<(int)e->song.wave.size(); i++) {
      DivWavetable* wave=e->song.wave[i];
      for (int i=0; i<wave->len; i++) {
        wavePreview[i]=wave->data[i];
      }
      if (wave->len>0) wavePreview[wave->len]=wave->data[wave->len-1];
      if (ImGui::Selectable(fmt::sprintf("%.2x##_WAVE%d\n",i,i).c_str(),curWave==i)) {
        curWave=i;
      }
      if (ImGui::IsItemHovered()) {
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
          waveEditOpen=true;
        }
      }
      ImGui::SameLine();
      PlotNoLerp(fmt::sprintf("##_WAVEP%d",i).c_str(),wavePreview,wave->len+1,0,NULL,0,wave->max);
    }
  }
  if (ImGui::IsWindowFocused()) curWindow=GUI_WINDOW_WAVE_LIST;
  ImGui::End();
}

void FurnaceGUI::drawWaveEdit() {
  if (!waveEditOpen) return;
  float wavePreview[256];
  if (ImGui::Begin("Wavetable Editor",&waveEditOpen,ImGuiWindowFlags_NoDocking)) {
    if (curWave<0 || curWave>=(int)e->song.wave.size()) {
      ImGui::Text("no wavetable selected");
    } else {
      DivWavetable* wave=e->song.wave[curWave];
      ImGui::Text("Width");
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("use a width of 32 on Game Boy and PC Engine.\nany other widths will be scaled during playback.");
      }
      ImGui::SameLine();
      ImGui::SetNextItemWidth(128.0f*dpiScale);
      if (ImGui::InputInt("##_WTW",&wave->len,1,2)) {
        if (wave->len>256) wave->len=256;
        if (wave->len<1) wave->len=1;
      }
      ImGui::SameLine();
      ImGui::Text("Height");
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("use a height of:\n- 15 for Game Boy\n- 31 for PC Engine\nany other heights will be scaled during playback.");
      }
      ImGui::SameLine();
      ImGui::SetNextItemWidth(128.0f*dpiScale);
      if (ImGui::InputInt("##_WTH",&wave->max,1,2)) {
        if (wave->max>255) wave->max=255;
        if (wave->max<1) wave->max=1;
      }
      for (int i=0; i<wave->len; i++) {
        wavePreview[i]=wave->data[i];
      }
      if (wave->len>0) wavePreview[wave->len]=wave->data[wave->len-1];
      ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(0.0f,0.0f));
      ImVec2 contentRegion=ImGui::GetContentRegionAvail();
      PlotNoLerp("##Waveform",wavePreview,wave->len+1,0,NULL,0,wave->max,contentRegion);
      if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        waveDragStart=ImGui::GetItemRectMin();
        waveDragAreaSize=contentRegion;
        waveDragMin=0;
        waveDragMax=wave->max;
        waveDragLen=wave->len;
        waveDragActive=true;
        waveDragTarget=wave->data;
        processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
      }
      ImGui::PopStyleVar();
    }
  }
  if (ImGui::IsWindowFocused()) curWindow=GUI_WINDOW_WAVE_EDIT;
  ImGui::End();
}

const char* sampleNote[12]={
  "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

void FurnaceGUI::drawSampleList() {
  if (!sampleListOpen) return;
  if (ImGui::Begin("Samples",&sampleListOpen)) {
    if (ImGui::Button(ICON_FA_PLUS "##SampleAdd")) {
      curSample=e->addSample();
      modified=true;
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FOLDER_OPEN "##SampleLoad")) {
      openFileDialog(GUI_FILE_SAMPLE_OPEN);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FLOPPY_O "##SampleSave")) {
      if (curSample>=0 && curSample<(int)e->song.sample.size()) openFileDialog(GUI_FILE_SAMPLE_SAVE);
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("SampleUp",ImGuiDir_Up)) {
      if (e->moveSampleUp(curSample)) curSample--;
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("SampleDown",ImGuiDir_Down)) {
      if (e->moveSampleDown(curSample)) curSample++;
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_TIMES "##SampleDelete")) {
      e->delSample(curSample);
      modified=true;
      if (curSample>=(int)e->song.sample.size()) {
        curSample--;
      }
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_VOLUME_UP "##PreviewSampleL")) {
      e->previewSample(curSample);
    }
    ImGui::Separator();
    for (int i=0; i<(int)e->song.sample.size(); i++) {
      DivSample* sample=e->song.sample[i];
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
    ImGui::Unindent();
  }
  if (ImGui::IsWindowFocused()) curWindow=GUI_WINDOW_SAMPLE_LIST;
  ImGui::End();
}

void FurnaceGUI::drawSampleEdit() {
  if (!sampleEditOpen) return;
  if (ImGui::Begin("Sample Editor",&sampleEditOpen,ImGuiWindowFlags_NoDocking)) {
    if (curSample<0 || curSample>=(int)e->song.sample.size()) {
      ImGui::Text("no sample selected");
    } else {
      DivSample* sample=e->song.sample[curSample];
      ImGui::InputText("Name",&sample->name);
      if (ImGui::SliderInt("Rate",&sample->rate,4000,32000,"%dHz")) {
        if (sample->rate<4000) sample->rate=4000;
        if (sample->rate>32000) sample->rate=32000;
      }
      ImGui::Text("effective rate: %dHz",e->getEffectiveSampleRate(sample->rate));
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
    }
  }
  if (ImGui::IsWindowFocused()) curWindow=GUI_WINDOW_SAMPLE_EDIT;
  ImGui::End();
}

void FurnaceGUI::drawMixer() {
  if (!mixerOpen) return;
  if (ImGui::Begin("Mixer",&mixerOpen,ImGuiWindowFlags_NoDocking)) {
    char id[32];
    ImGui::Columns(3);
    for (int i=0; i<e->song.systemLen; i++) {
      snprintf(id,31,"MixS%d",i);
      bool doInvert=e->song.systemVol[i]&128;
      signed char vol=e->song.systemVol[i]&127;
      ImGui::PushID(id);
      if (ImGui::SliderScalar("##Volume",ImGuiDataType_S8,&vol,&_ZERO,&_ONE_HUNDRED_TWENTY_SEVEN)) {
        e->song.systemVol[i]=(e->song.systemVol[i]&128)|vol;
      }
      ImGui::NextColumn();
      ImGui::SliderScalar("##Panning",ImGuiDataType_S8,&e->song.systemPan[i],&_MINUS_ONE_HUNDRED_TWENTY_SEVEN,&_ONE_HUNDRED_TWENTY_SEVEN);
      ImGui::NextColumn();
      if (ImGui::Checkbox("Invert",&doInvert)) {
        e->song.systemVol[i]^=128;
      }
      ImGui::NextColumn();
      ImGui::PopID();
    }
  }
  ImGui::End();
}

void FurnaceGUI::drawPattern() {
  if (!patternOpen) return;
  if (e->isPlaying() && followPattern) cursor.y=oldRow;
  SelectionPoint sel1=selStart;
  SelectionPoint sel2=selEnd;
  if (sel2.y<sel1.y) {
    sel2.y^=sel1.y;
    sel1.y^=sel2.y;
    sel2.y^=sel1.y;
  }
  if (sel2.xCoarse<sel1.xCoarse) {
    sel2.xCoarse^=sel1.xCoarse;
    sel1.xCoarse^=sel2.xCoarse;
    sel2.xCoarse^=sel1.xCoarse;

    sel2.xFine^=sel1.xFine;
    sel1.xFine^=sel2.xFine;
    sel2.xFine^=sel1.xFine;
  } else if (sel2.xCoarse==sel1.xCoarse && sel2.xFine<sel1.xFine) {
    sel2.xFine^=sel1.xFine;
    sel1.xFine^=sel2.xFine;
    sel2.xFine^=sel1.xFine;
  }
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,ImVec2(0.0f,0.0f));
  if (ImGui::Begin("Pattern",&patternOpen)) {
    //ImGui::SetWindowSize(ImVec2(scrW*dpiScale,scrH*dpiScale));
    patWindowPos=ImGui::GetWindowPos();
    patWindowSize=ImGui::GetWindowSize();
    char id[32];
    ImGui::PushFont(patFont);
    unsigned char ord=e->isPlaying()?oldOrder:e->getOrder();
    oldOrder=e->getOrder();
    int chans=e->getTotalChannelCount();
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding,ImVec2(0.0f,0.0f));
    ImGui::PushStyleColor(ImGuiCol_Header,uiColors[GUI_COLOR_PATTERN_SELECTION]);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered,uiColors[GUI_COLOR_PATTERN_SELECTION_HOVER]);
    ImGui::PushStyleColor(ImGuiCol_HeaderActive,uiColors[GUI_COLOR_PATTERN_SELECTION_ACTIVE]);
    if (ImGui::BeginTable("PatternView",chans+2,ImGuiTableFlags_BordersInnerV|ImGuiTableFlags_ScrollX|ImGuiTableFlags_ScrollY|ImGuiTableFlags_NoPadInnerX)) {
      ImGui::TableSetupColumn("pos",ImGuiTableColumnFlags_WidthFixed);
      char chanID[256];
      float lineHeight=(ImGui::GetTextLineHeight()+2*dpiScale);
      int curRow=e->getRow();
      if (e->isPlaying() && followPattern) updateScroll(curRow);
      if (nextScroll>-0.5f) {
        ImGui::SetScrollY(nextScroll);
        nextScroll=-1.0f;
        nextAddScroll=0.0f;
      }
      if (nextAddScroll!=0.0f) {
        ImGui::SetScrollY(ImGui::GetScrollY()+nextAddScroll);
        nextScroll=-1.0f;
        nextAddScroll=0.0f;
      }
      ImGui::TableSetupScrollFreeze(1,1);
      for (int i=0; i<chans; i++) {
        ImGui::TableSetupColumn(fmt::sprintf("c%d",i).c_str(),ImGuiTableColumnFlags_WidthFixed);
      }
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      if (ImGui::Selectable(extraChannelButtons?" --##ExtraChannelButtons":" ++##ExtraChannelButtons",false,ImGuiSelectableFlags_NoPadWithHalfSpacing,ImVec2(0.0f,lineHeight+1.0f*dpiScale))) {
        extraChannelButtons=!extraChannelButtons;
      }
      for (int i=0; i<chans; i++) {
        ImGui::TableNextColumn();
        snprintf(chanID,256," %s##_CH%d",e->getChannelName(i),i);
        bool muted=e->isChannelMuted(i);
        ImVec4 chanHead=muted?uiColors[GUI_COLOR_CHANNEL_MUTED]:uiColors[GUI_COLOR_CHANNEL_FM+e->getChannelType(i)];
        ImVec4 chanHeadActive=chanHead;
        ImVec4 chanHeadHover=chanHead;
        chanHead.x*=0.25; chanHead.y*=0.25; chanHead.z*=0.25;
        chanHeadActive.x*=0.8; chanHeadActive.y*=0.8; chanHeadActive.z*=0.8;
        chanHeadHover.x*=0.4; chanHeadHover.y*=0.4; chanHeadHover.z*=0.4;
        ImGui::PushStyleColor(ImGuiCol_Header,chanHead);
        ImGui::PushStyleColor(ImGuiCol_HeaderActive,chanHeadActive);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered,chanHeadHover);
        if (muted) ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_CHANNEL_MUTED]);
        if (ImGui::Selectable(chanID,!muted,ImGuiSelectableFlags_NoPadWithHalfSpacing,ImVec2(0.0f,lineHeight+1.0f*dpiScale))) {
          e->toggleMute(i);
        }
        if (muted) ImGui::PopStyleColor();
        ImGui::PopStyleColor(3);
        if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
          e->toggleSolo(i);
        }
        if (extraChannelButtons) {
          snprintf(chanID,256,"<##_LCH%d",i);
          ImGui::BeginDisabled(e->song.pat[i].effectRows<=1);
          ImGui::SetCursorPosX(ImGui::GetCursorPosX()+4.0f*dpiScale);
          if (ImGui::SmallButton(chanID)) {
            e->song.pat[i].effectRows--;
            if (e->song.pat[i].effectRows<1) e->song.pat[i].effectRows=1;
          }
          ImGui::EndDisabled();
          ImGui::SameLine();
          ImGui::BeginDisabled(e->song.pat[i].effectRows>=4);
          snprintf(chanID,256,">##_RCH%d",i);
          if (ImGui::SmallButton(chanID)) {
            e->song.pat[i].effectRows++;
            if (e->song.pat[i].effectRows>4) e->song.pat[i].effectRows=4;
          }
          ImGui::EndDisabled();
          ImGui::Spacing();
        }
      }
      ImGui::TableNextColumn();
      if (e->hasExtValue()) {
        ImGui::TextColored(uiColors[GUI_COLOR_EE_VALUE]," %.2X",e->getExtValue());
      }
      float oneCharSize=ImGui::CalcTextSize("A").x;
      ImVec2 threeChars=ImVec2(oneCharSize*3.0f,lineHeight);
      ImVec2 twoChars=ImVec2(oneCharSize*2.0f,lineHeight);
      //ImVec2 oneChar=ImVec2(oneCharSize,lineHeight);
      int dummyRows=(ImGui::GetWindowSize().y/lineHeight)/2;
      for (int i=0; i<dummyRows-1; i++) {
        ImGui::TableNextRow(0,lineHeight);
        ImGui::TableNextColumn();
      }
      for (int i=0; i<e->song.patLen; i++) {
        bool selectedRow=(i>=sel1.y && i<=sel2.y);
        ImGui::TableNextRow(0,lineHeight);
        if ((lineHeight*(i+dummyRows+1))-ImGui::GetScrollY()<0) {
          continue;
        }
        if ((lineHeight*(i+dummyRows))-ImGui::GetScrollY()>ImGui::GetWindowSize().y) {
          continue;          
        }
        ImGui::TableNextColumn();
        if (e->isPlaying() && oldRow==i) {
          ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,0x40ffffff);
        } else if (e->song.hilightB>0 && !(i%e->song.hilightB)) {
          ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,ImGui::GetColorU32(uiColors[GUI_COLOR_PATTERN_HI_2]));
        } else if (e->song.hilightA>0 && !(i%e->song.hilightA)) {
          ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,ImGui::GetColorU32(uiColors[GUI_COLOR_PATTERN_HI_1]));
        }
        ImGui::TextColored(uiColors[GUI_COLOR_PATTERN_ROW_INDEX],"%3d ",i);
        for (int j=0; j<chans; j++) {
          int chanVolMax=e->getMaxVolumeChan(j);
          DivPattern* pat=e->song.pat[j].getPattern(e->song.orders.ord[j][ord],true);
          ImGui::TableNextColumn();

          int sel1XSum=sel1.xCoarse*32+sel1.xFine;
          int sel2XSum=sel2.xCoarse*32+sel2.xFine;
          int j32=j*32;
          bool selectedNote=selectedRow && (j32>=sel1XSum && j32<=sel2XSum);
          bool selectedIns=selectedRow && (j32+1>=sel1XSum && j32+1<=sel2XSum);
          bool selectedVol=selectedRow && (j32+2>=sel1XSum && j32+2<=sel2XSum);
          bool cursorNote=(cursor.y==i && cursor.xCoarse==j && cursor.xFine==0);
          bool cursorIns=(cursor.y==i && cursor.xCoarse==j && cursor.xFine==1);
          bool cursorVol=(cursor.y==i && cursor.xCoarse==j && cursor.xFine==2);

          sprintf(id,"%s##PN_%d_%d",noteName(pat->data[i][0],pat->data[i][1]),i,j);
          if (pat->data[i][0]==0 && pat->data[i][1]==0) {
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_INACTIVE]);
          } else {
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_ACTIVE]);
          }
          if (cursorNote) {
            ImGui::PushStyleColor(ImGuiCol_Header,uiColors[GUI_COLOR_PATTERN_CURSOR]);
            ImGui::PushStyleColor(ImGuiCol_HeaderActive,uiColors[GUI_COLOR_PATTERN_CURSOR_ACTIVE]);
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered,uiColors[GUI_COLOR_PATTERN_CURSOR_HOVER]);
            ImGui::Selectable(id,true,ImGuiSelectableFlags_NoPadWithHalfSpacing,threeChars);
            ImGui::PopStyleColor(3);
          } else {
            ImGui::Selectable(id,selectedNote,ImGuiSelectableFlags_NoPadWithHalfSpacing,threeChars);
          }
          if (ImGui::IsItemClicked()) {
            startSelection(j,0,i);
          }
          if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
            //ImGui::SetTooltip("N: %d O: %d",pat->data[i][0],pat->data[i][1]);
            updateSelection(j,0,i);
          }
          ImGui::PopStyleColor();

          if (pat->data[i][2]==-1) {
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_INACTIVE]);
            sprintf(id,"..##PI_%d_%d",i,j);
          } else {
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_INS]);
            sprintf(id,"%.2X##PI_%d_%d",pat->data[i][2],i,j);
          }
          ImGui::SameLine(0.0f,0.0f);
          if (cursorIns) {
            ImGui::PushStyleColor(ImGuiCol_Header,uiColors[GUI_COLOR_PATTERN_CURSOR]);
            ImGui::PushStyleColor(ImGuiCol_HeaderActive,uiColors[GUI_COLOR_PATTERN_CURSOR_ACTIVE]);
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered,uiColors[GUI_COLOR_PATTERN_CURSOR_HOVER]);
            ImGui::Selectable(id,true,ImGuiSelectableFlags_NoPadWithHalfSpacing,twoChars);
            ImGui::PopStyleColor(3);
          } else {
            ImGui::Selectable(id,selectedIns,ImGuiSelectableFlags_NoPadWithHalfSpacing,twoChars);
          }
          if (ImGui::IsItemClicked()) {
            startSelection(j,1,i);
          }
          if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
            updateSelection(j,1,i);
          }
          ImGui::PopStyleColor();

          if (pat->data[i][3]==-1) {
            sprintf(id,"..##PV_%d_%d",i,j);
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_INACTIVE]);
          } else {
            int volColor=(pat->data[i][3]*127)/chanVolMax;
            if (volColor>127) volColor=127;
            if (volColor<0) volColor=0;
            sprintf(id,"%.2X##PV_%d_%d",pat->data[i][3],i,j);
            ImGui::PushStyleColor(ImGuiCol_Text,volColors[volColor]);
          }
          ImGui::SameLine(0.0f,0.0f);
          if (cursorVol) {
            ImGui::PushStyleColor(ImGuiCol_Header,uiColors[GUI_COLOR_PATTERN_CURSOR]);
            ImGui::PushStyleColor(ImGuiCol_HeaderActive,uiColors[GUI_COLOR_PATTERN_CURSOR_ACTIVE]);
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered,uiColors[GUI_COLOR_PATTERN_CURSOR_HOVER]);
            ImGui::Selectable(id,true,ImGuiSelectableFlags_NoPadWithHalfSpacing,twoChars);
            ImGui::PopStyleColor(3);
          } else {
            ImGui::Selectable(id,selectedVol,ImGuiSelectableFlags_NoPadWithHalfSpacing,twoChars);
          }
          if (ImGui::IsItemClicked()) {
            startSelection(j,2,i);
          }
          if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
            updateSelection(j,2,i);
          }
          ImGui::PopStyleColor();

          for (int k=0; k<e->song.pat[j].effectRows; k++) {
            int index=4+(k<<1);
            bool selectedEffect=selectedRow && (j32+index-1>=sel1XSum && j32+index-1<=sel2XSum);
            bool selectedEffectVal=selectedRow && (j32+index>=sel1XSum && j32+index<=sel2XSum);
            bool cursorEffect=(cursor.y==i && cursor.xCoarse==j && cursor.xFine==index-1);
            bool cursorEffectVal=(cursor.y==i && cursor.xCoarse==j && cursor.xFine==index);
            if (pat->data[i][index]==-1) {
              sprintf(id,"..##PE%d_%d_%d",k,i,j);
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_INACTIVE]);
            } else {
              sprintf(id,"%.2X##PE%d_%d_%d",pat->data[i][index],k,i,j);
              if (pat->data[i][index]<0x10) {
                ImGui::PushStyleColor(ImGuiCol_Text,uiColors[fxColors[pat->data[i][index]]]);
              } else if (pat->data[i][index]<0x20) {
                ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_EFFECT_SYS_PRIMARY]);
              } else if (pat->data[i][index]<0x30) {
                ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_EFFECT_SYS_SECONDARY]);
              } else if (pat->data[i][index]<0x48) {
                ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_EFFECT_SYS_PRIMARY]);
              } else if (pat->data[i][index]<0xc0) {
                ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_EFFECT_INVALID]);
              } else if (pat->data[i][index]<0xd0) {
                ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_EFFECT_SPEED]);
              } else if (pat->data[i][index]<0xe0) {
                ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_EFFECT_INVALID]);
              } else if (pat->data[i][index]<0xf0) {
                ImGui::PushStyleColor(ImGuiCol_Text,uiColors[extFxColors[pat->data[i][index]-0xe0]]);
              } else {
                ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_EFFECT_INVALID]);
              }
            }
            ImGui::SameLine(0.0f,0.0f);
            if (cursorEffect) {
              ImGui::PushStyleColor(ImGuiCol_Header,uiColors[GUI_COLOR_PATTERN_CURSOR]);  
              ImGui::PushStyleColor(ImGuiCol_HeaderActive,uiColors[GUI_COLOR_PATTERN_CURSOR_ACTIVE]);
              ImGui::PushStyleColor(ImGuiCol_HeaderHovered,uiColors[GUI_COLOR_PATTERN_CURSOR_HOVER]);
              ImGui::Selectable(id,true,ImGuiSelectableFlags_NoPadWithHalfSpacing,twoChars);
              ImGui::PopStyleColor(3);
            } else {
              ImGui::Selectable(id,selectedEffect,ImGuiSelectableFlags_NoPadWithHalfSpacing,twoChars);
            }
            if (ImGui::IsItemClicked()) {
              startSelection(j,index-1,i);
            }
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
              updateSelection(j,index-1,i);
            }
            if (pat->data[i][index+1]==-1) {
              sprintf(id,"..##PF%d_%d_%d",k,i,j);
            } else {
              sprintf(id,"%.2X##PF%d_%d_%d",pat->data[i][index+1],k,i,j);
            }
            ImGui::SameLine(0.0f,0.0f);
            if (cursorEffectVal) {
              ImGui::PushStyleColor(ImGuiCol_Header,uiColors[GUI_COLOR_PATTERN_CURSOR]);  
              ImGui::PushStyleColor(ImGuiCol_HeaderActive,uiColors[GUI_COLOR_PATTERN_CURSOR_ACTIVE]);
              ImGui::PushStyleColor(ImGuiCol_HeaderHovered,uiColors[GUI_COLOR_PATTERN_CURSOR_HOVER]);
              ImGui::Selectable(id,true,ImGuiSelectableFlags_NoPadWithHalfSpacing,twoChars);
              ImGui::PopStyleColor(3);
            } else {
              ImGui::Selectable(id,selectedEffectVal,ImGuiSelectableFlags_NoPadWithHalfSpacing,twoChars);
            }
            if (ImGui::IsItemClicked()) {
              startSelection(j,index,i);
            }
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
              updateSelection(j,index,i);
            }
            ImGui::PopStyleColor();
          }
        }
      }
      for (int i=0; i<=dummyRows; i++) {
        ImGui::TableNextRow(0,lineHeight);
        ImGui::TableNextColumn();
      }
      oldRow=curRow;
      ImGui::EndTable();
    }
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();
    ImGui::PopFont();
  }
  ImGui::PopStyleVar();
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_PATTERN;
  ImGui::End();
}

const char* aboutLine[55]={
  "tildearrow",
  "is proud to present",
  "",
  ("Furnace " DIV_VERSION),
  "",
  "the free software chiptune tracker,",
  "and proof of concept of what can be",
  "done in two months.",
  "",
  "zero disassembly.",
  "zero reverse-engineering.",
  "only time and dedication.",
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
  "SameBoy by Lior Halphon",
  "Mednafen PCE",
  "puNES by FHorse",
  "reSID by Dag Lem",
  "Stella by Stella Team",
  "",
  "greetings to:",
  "Delek",
  "fd",
  "ILLUMIDARO",
  "all members of Deflers of Noice!",
  "",
  "copyright © 2021 tildearrow.",
  "licensed under GPLv2! see",
  "LICENSE.txt for more information.",
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
  "thanks to all contributors!"
};

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
    ImGui::ColorConvertHSVtoRGB(aboutHue,1.0,0.5,r,g,b);
    aboutHue+=0.001;
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
    for (int i=(-(160-(aboutSin*2)%160))*2; i<scrW; i+=160) {
      skip2=!skip2;
      skip=skip2;
      for (int j=(-240-cos(double(aboutSin*M_PI/300.0))*240.0)*2; j<scrH; j+=160) {
        skip=!skip;
        if (skip) continue;
        dl->AddRectFilled(ImVec2(i*dpiScale,j*dpiScale),ImVec2((i+160)*dpiScale,(j+160)*dpiScale),ImGui::GetColorU32(ImVec4(r*0.75,g*0.75,b*0.75,1.0)));
      }
    }

    for (int i=0; i<55; i++) {
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
    aboutScroll+=2;
    if (++aboutSin>=2400) aboutSin=0;
    if (aboutScroll>(42*55+scrH)) aboutScroll=-20;
  }
  ImGui::End();
}

const char* mainFonts[]={
  "IBM Plex Sans",
  "Liberation Sans",
  "Roboto",
  "<Use system font>",
  "<Custom...>"
};

const char* patFonts[]={
  "IBM Plex Mono",
  "Mononoki",
  "PT Mono",
  "Roboto Mono",
  "<Use system font>",
  "<Custom...>"
};

const char* audioBackends[]={
  "SDL",
  "JACK"
};

const char* arcadeCores[]={
  "ymfm",
  "Nuked-OPM"
};

void FurnaceGUI::drawSettings() {
  if (!settingsOpen) return;
  int curAudioBackend=0;
  int curArcadeCore=0;
  int curMainFont=0;
  int curPatFont=0;
  if (ImGui::Begin("Settings",NULL,ImGuiWindowFlags_NoDocking)) {
    if (ImGui::BeginTabBar("settingsTab")) {
      if (ImGui::BeginTabItem("General")) {
        ImGui::Text("Hello world!");
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Audio")) {
        ImGui::Text("Backend");
        ImGui::SameLine();
        ImGui::Combo("##Backend",&curAudioBackend,audioBackends,2);

        ImGui::Text("Sample rate");

        ImGui::Text("Buffer size");

        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Emulation")) {
        ImGui::Text("Arcade core");
        ImGui::SameLine();
        ImGui::Combo("##ArcadeCore",&curArcadeCore,arcadeCores,2);

        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Appearance")) {
        ImGui::Text("Main font");
        ImGui::SameLine();
        ImGui::Combo("##MainFont",&curMainFont,mainFonts,5);
        if (ImGui::InputInt("Size##MainFontSize",&mainFontSize)) {
          if (mainFontSize<3) mainFontSize=3;
          if (mainFontSize>96) mainFontSize=96;
        }
        ImGui::Text("Pattern font");
        ImGui::SameLine();
        ImGui::Combo("##PatFont",&curPatFont,patFonts,6);
        if (ImGui::InputInt("Size##PatFontSize",&patFontSize)) {
          if (patFontSize<3) patFontSize=3;
          if (patFontSize>96) patFontSize=96;
        }
        ImGui::EndTabItem();
      }
      ImGui::EndTabBar();
    }
    ImGui::Separator();
    if (ImGui::Button("OK##SettingsOK")) {
      settingsOpen=false;
      willCommit=true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel##SettingsCancel")) {
      settingsOpen=false;
    }
  }
  ImGui::End();
}

void FurnaceGUI::commitSettings() {
  e->setConf("mainFontSize",mainFontSize);
  e->setConf("patFontSize",patFontSize);

  e->saveConf();

  ImGui::GetIO().Fonts->Clear();
  if ((mainFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(defFont_main_compressed_data,defFont_main_compressed_size,e->getConfInt("mainFontSize",18)*dpiScale))==NULL) {
    logE("could not load UI font!\n");
  }

  ImFontConfig fc;
  fc.MergeMode=true;
  fc.GlyphMinAdvanceX=e->getConfInt("iconSize",16)*dpiScale;
  static const ImWchar fontRange[]={ICON_MIN_FA,ICON_MAX_FA,0};
  if ((iconFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(iconFont_compressed_data,iconFont_compressed_size,e->getConfInt("iconSize",16)*dpiScale,&fc,fontRange))==NULL) {
    logE("could not load icon font!\n");
  }
  if ((patFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(defFont_pat_compressed_data,defFont_pat_compressed_size,e->getConfInt("patFontSize",18)*dpiScale))==NULL) {
    logE("could not load pattern font!\n");
  }
  if ((bigFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(defFont_main_compressed_data,defFont_main_compressed_size,40*dpiScale))==NULL) {
    logE("could not load big UI font!\n");
  }

  ImGui_ImplSDLRenderer_DestroyFontsTexture();
  ImGui::GetIO().Fonts->Build();
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
}

void FurnaceGUI::moveCursor(int x, int y) {
  finishSelection();
  curNibble=false;
  if (x!=0) {
    if (x>0) {
      for (int i=0; i<x; i++) {
        if (++cursor.xFine>=3+e->song.pat[cursor.xCoarse].effectRows*2) {
          cursor.xFine=0;
          if (++cursor.xCoarse>=e->getTotalChannelCount()) {
            cursor.xCoarse=e->getTotalChannelCount()-1;
            cursor.xFine=2+e->song.pat[cursor.xCoarse].effectRows*2;
          }
        }
      }
    } else {
      for (int i=0; i<-x; i++) {
        if (--cursor.xFine<0) {
          if (--cursor.xCoarse<0) {
            cursor.xCoarse=0;
            cursor.xFine=0;
          } else {
            cursor.xFine=2+e->song.pat[cursor.xCoarse].effectRows*2;
          }
        }
      }
    }
  }
  if (y!=0) {
    cursor.y+=y;
    if (cursor.y<0) cursor.y=0;
    if (cursor.y>=e->song.patLen) cursor.y=e->song.patLen-1;
  }
  selStart=cursor;
  selEnd=cursor;
  updateScroll(cursor.y);
}

void FurnaceGUI::moveCursorTop() {
  finishSelection();
  curNibble=false;
  if (cursor.y==0) {
    cursor.xCoarse=0;
    cursor.xFine=0;
  } else {
    cursor.y=0;
  }
  selStart=cursor;
  selEnd=cursor;
  updateScroll(cursor.y);
}

void FurnaceGUI::moveCursorBottom() {
  finishSelection();
  curNibble=false;
  if (cursor.y==e->song.patLen-1) {
    cursor.xCoarse=e->getTotalChannelCount()-1;
    cursor.xFine=2+e->song.pat[cursor.xCoarse].effectRows*2;
  } else {
    cursor.y=e->song.patLen-1;
  }
  selStart=cursor;
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
    case GUI_ACTION_CHANGE_ORDER:
      oldOrders=e->song.orders;
      oldOrdersLen=e->song.ordersLen;
      break;
    case GUI_ACTION_PATTERN_EDIT:
    case GUI_ACTION_PATTERN_DELETE:
    case GUI_ACTION_PATTERN_PULL:
    case GUI_ACTION_PATTERN_PUSH:
    case GUI_ACTION_PATTERN_CUT:
    case GUI_ACTION_PATTERN_PASTE:
      for (int i=0; i<e->getTotalChannelCount(); i++) {
        memcpy(oldPat[i],e->song.pat[i].getPattern(e->song.orders.ord[i][order],false),sizeof(DivPattern));
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
    case GUI_ACTION_CHANGE_ORDER:
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
    case GUI_ACTION_PATTERN_EDIT:
    case GUI_ACTION_PATTERN_DELETE:
    case GUI_ACTION_PATTERN_PULL:
    case GUI_ACTION_PATTERN_PUSH:
    case GUI_ACTION_PATTERN_CUT:
    case GUI_ACTION_PATTERN_PASTE:
      for (int i=0; i<e->getTotalChannelCount(); i++) {
        DivPattern* p=e->song.pat[i].getPattern(e->song.orders.ord[i][order],false);
        for (int j=0; j<e->song.patLen; j++) {
          for (int k=0; k<16; k++) {
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
    if (undoHist.size()>maxUndoSteps) undoHist.pop_front();
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
  prepareUndo(GUI_ACTION_PATTERN_DELETE);
  curNibble=false;

  int iCoarse=selStart.xCoarse;
  int iFine=selStart.xFine;
  int ord=e->getOrder();
  for (; iCoarse<=selEnd.xCoarse; iCoarse++) {
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

  makeUndo(GUI_ACTION_PATTERN_DELETE);
}

void FurnaceGUI::doPullDelete() {
  finishSelection();
  prepareUndo(GUI_ACTION_PATTERN_PULL);
  curNibble=false;

  int iCoarse=selStart.xCoarse;
  int iFine=selStart.xFine;
  int ord=e->getOrder();
  for (; iCoarse<=selEnd.xCoarse; iCoarse++) {
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

  makeUndo(GUI_ACTION_PATTERN_PULL);
}

void FurnaceGUI::doInsert() {
  finishSelection();
  prepareUndo(GUI_ACTION_PATTERN_PUSH);
  curNibble=false;

  int iCoarse=selStart.xCoarse;
  int iFine=selStart.xFine;
  int ord=e->getOrder();
  for (; iCoarse<=selEnd.xCoarse; iCoarse++) {
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

  makeUndo(GUI_ACTION_PATTERN_PUSH);
}

void FurnaceGUI::doCopy(bool cut) {
  finishSelection();
  if (cut) {
    curNibble=false;
    prepareUndo(GUI_ACTION_PATTERN_CUT);
  }
  clipboard=fmt::sprintf("org.tildearrow.furnace - Pattern Data (%d)\n%d",DIV_ENGINE_VERSION,selStart.xFine);

  for (int j=selStart.y; j<=selEnd.y; j++) {
    int iCoarse=selStart.xCoarse;
    int iFine=selStart.xFine;
    int ord=e->getOrder();
    clipboard+='\n';
    for (; iCoarse<=selEnd.xCoarse; iCoarse++) {
      DivPattern* pat=e->song.pat[iCoarse].getPattern(e->song.orders.ord[iCoarse][ord],true);
      for (; iFine<3+e->song.pat[iCoarse].effectRows*2 && (iCoarse<selEnd.xCoarse || iFine<=selEnd.xFine); iFine++) {
        if (iFine==0) {
          clipboard+=noteName(pat->data[j][0],pat->data[j][1]);
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
    makeUndo(GUI_ACTION_PATTERN_CUT);
  }
}

void FurnaceGUI::doPaste() {
  finishSelection();
  prepareUndo(GUI_ACTION_PATTERN_PASTE);
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

  int j=cursor.y;
  char note[4];
  int ord=e->getOrder();
  for (size_t i=2; i<data.size() && j<e->song.patLen; i++) {
    size_t charPos=0;
    int iCoarse=cursor.xCoarse;
    int iFine=(startOff>2 && cursor.xFine>2)?(((cursor.xFine-1)&(~1))|1):startOff;

    String& line=data[i];

    while (charPos<line.size() && iCoarse<e->getTotalChannelCount()) {
      DivPattern* pat=e->song.pat[iCoarse].getPattern(e->song.orders.ord[iCoarse][ord],true);
      if (line[charPos]=='|') {
        iCoarse++;
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
          if (iFine<(3+e->song.pat[cursor.xCoarse].effectRows*2)) pat->data[j][iFine+1]=val;
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

  makeUndo(GUI_ACTION_PATTERN_PASTE);
}

void FurnaceGUI::doUndo() {
  if (undoHist.empty()) return;
  UndoStep& us=undoHist.back();
  redoHist.push_back(us);
  modified=true;

  switch (us.type) {
    case GUI_ACTION_CHANGE_ORDER:
      e->song.ordersLen=us.oldOrdersLen;
      for (UndoOrderData& i: us.ord) {
        e->song.orders.ord[i.chan][i.ord]=i.oldVal;
      }
      break;
    case GUI_ACTION_PATTERN_EDIT:
    case GUI_ACTION_PATTERN_DELETE:
    case GUI_ACTION_PATTERN_PULL:
    case GUI_ACTION_PATTERN_PUSH:
    case GUI_ACTION_PATTERN_CUT:
    case GUI_ACTION_PATTERN_PASTE:
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
    case GUI_ACTION_CHANGE_ORDER:
      e->song.ordersLen=us.newOrdersLen;
      for (UndoOrderData& i: us.ord) {
        e->song.orders.ord[i.chan][i.ord]=i.newVal;
      }
      break;
    case GUI_ACTION_PATTERN_EDIT:
    case GUI_ACTION_PATTERN_DELETE:
    case GUI_ACTION_PATTERN_PULL:
    case GUI_ACTION_PATTERN_PUSH:
    case GUI_ACTION_PATTERN_CUT:
    case GUI_ACTION_PATTERN_PASTE:
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

void FurnaceGUI::keyDown(SDL_Event& ev) {
  // GLOBAL KEYS
  if (ev.key.keysym.mod&KMOD_CTRL) {
    switch (ev.key.keysym.sym) {
      case SDLK_z:
        if (ev.key.keysym.mod&KMOD_SHIFT) {
          doRedo();
        } else {
          doUndo();
        }
        break;
    }
  } else switch (ev.key.keysym.sym) {
    case SDLK_F5:
      if (!e->isPlaying()) e->play();
      break;
    case SDLK_F6:
      e->play();
      break;
    case SDLK_F7:
      e->play();
      break;
    case SDLK_F8:
      e->stop();
      break;
    case SDLK_RETURN:
      if (e->isPlaying()) {
        e->stop();
      } else {
        e->play();
      }
      break;
  }
  // PER-WINDOW KEYS
  switch (curWindow) {
    case GUI_WINDOW_PATTERN: {
      if (ev.key.keysym.mod&KMOD_CTRL) {
        switch (ev.key.keysym.sym) {
          case SDLK_a:
            doSelectAll();
            break;
          case SDLK_x:
            doCopy(true);
            break;
          case SDLK_c:
            doCopy(false);
            break;
          case SDLK_v:
            doPaste();
            break;
        }
      } else if (ev.key.keysym.mod&KMOD_ALT) {
        // nothing. prevents accidental OFF note.
      } else switch (ev.key.keysym.sym) {
        case SDLK_SPACE:
          edit=!edit;
          break;
        case SDLK_UP:
          moveCursor(0,-1);
          break;
        case SDLK_DOWN:
          moveCursor(0,1);
          break;
        case SDLK_LEFT:
          moveCursor(-1,0);
          break;
        case SDLK_RIGHT:
          moveCursor(1,0);
          break;
        case SDLK_PAGEUP:
          moveCursor(0,-16);
          break;
        case SDLK_PAGEDOWN:
          moveCursor(0,16);
          break;
        case SDLK_HOME:
          moveCursorTop();
          break;
        case SDLK_END:
          moveCursorBottom();
          break;
        case SDLK_DELETE:
          doDelete();
          break;
        case SDLK_BACKSPACE:
          doPullDelete();
          break;
        case SDLK_INSERT:
          doInsert();
          break;
        default:
          if (!ev.key.repeat) {
            if (cursor.xFine==0) { // note
              try {
                int key=noteKeys.at(ev.key.keysym.sym);
                int num=12*curOctave+key;

                if (edit) {
                  DivPattern* pat=e->song.pat[cursor.xCoarse].getPattern(e->song.orders.ord[cursor.xCoarse][e->getOrder()],true);
                  
                  prepareUndo(GUI_ACTION_PATTERN_EDIT);

                  if (key==100) { // note off
                    pat->data[cursor.y][0]=100;
                    pat->data[cursor.y][1]=0;
                  } else {
                    pat->data[cursor.y][0]=num%12;
                    pat->data[cursor.y][1]=num/12;
                    if (pat->data[cursor.y][0]==0) {
                      pat->data[cursor.y][0]=12;
                      pat->data[cursor.y][1]--;
                    }
                    pat->data[cursor.y][2]=curIns;
                    e->noteOn(cursor.xCoarse,curIns,num);
                    noteOffOnRelease=true;
                    noteOffOnReleaseKey=ev.key.keysym.sym;
                    noteOffOnReleaseChan=cursor.xCoarse;
                  }
                  makeUndo(GUI_ACTION_PATTERN_EDIT);
                  editAdvance();
                  curNibble=false;
                } else {
                  if (key!=100) {
                    e->noteOn(cursor.xCoarse,curIns,num);
                    noteOffOnRelease=true;
                    noteOffOnReleaseKey=ev.key.keysym.sym;
                    noteOffOnReleaseChan=cursor.xCoarse;
                  }
                }
              } catch (std::out_of_range& e) {
              }
            } else if (edit) { // value
              try {
                int num=valueKeys.at(ev.key.keysym.sym);
                DivPattern* pat=e->song.pat[cursor.xCoarse].getPattern(e->song.orders.ord[cursor.xCoarse][e->getOrder()],true);
                prepareUndo(GUI_ACTION_PATTERN_EDIT);
                if (pat->data[cursor.y][cursor.xFine+1]==-1) pat->data[cursor.y][cursor.xFine+1]=0;
                pat->data[cursor.y][cursor.xFine+1]=((pat->data[cursor.y][cursor.xFine+1]<<4)|num)&0xff;
                if (cursor.xFine==1) { // instrument
                  if (pat->data[cursor.y][cursor.xFine+1]>=(int)e->song.ins.size()) {
                    pat->data[cursor.y][cursor.xFine+1]&=0x0f;
                    if (pat->data[cursor.y][cursor.xFine+1]>=(int)e->song.ins.size()) {
                      pat->data[cursor.y][cursor.xFine+1]=(int)e->song.ins.size()-1;
                    }
                  }
                  makeUndo(GUI_ACTION_PATTERN_EDIT);
                  if (e->song.ins.size()<16) {
                    curNibble=false;
                    editAdvance();
                  } else {
                    curNibble=!curNibble;
                    if (!curNibble) editAdvance();
                  }
                } else if (cursor.xFine==2) { // volume
                  pat->data[cursor.y][cursor.xFine+1]&=e->getMaxVolumeChan(cursor.xCoarse);
                  makeUndo(GUI_ACTION_PATTERN_EDIT);
                  if (e->getMaxVolumeChan(cursor.xCoarse)<16) {
                    curNibble=false;
                    editAdvance();
                  } else {
                    curNibble=!curNibble;
                    if (!curNibble) editAdvance();
                  }
                } else {
                  makeUndo(GUI_ACTION_PATTERN_EDIT);
                  curNibble=!curNibble;
                  if (!curNibble) editAdvance();
                }
              } catch (std::out_of_range& e) {
              }
            }
          }
          break;
      }
      break;
    }
    case GUI_WINDOW_INS_EDIT:
    case GUI_WINDOW_INS_LIST:
      if (!ev.key.repeat) {
        try {
          int key=noteKeys.at(ev.key.keysym.sym);
          int num=12*curOctave+key;
          if (key!=100) {
            e->noteOn(cursor.xCoarse,curIns,num);
            noteOffOnRelease=true;
            noteOffOnReleaseKey=ev.key.keysym.sym;
            noteOffOnReleaseChan=cursor.xCoarse;
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
  if (noteOffOnRelease) {
    if (ev.key.keysym.sym==noteOffOnReleaseKey) {
      noteOffOnRelease=false;
      e->noteOff(noteOffOnReleaseChan);
    }
  }
}

void FurnaceGUI::openFileDialog(FurnaceGUIFileDialogs type) {
  switch (type) {
    case GUI_FILE_OPEN:
      ImGuiFileDialog::Instance()->OpenModal("FileDialog","Open File","compatible files{.fur,.dmf},.*",workingDir);
      break;
    case GUI_FILE_SAVE:
      ImGuiFileDialog::Instance()->OpenModal("FileDialog","Save File","Furnace song{.fur},DefleMask module{.dmf}",workingDir);
      break;
    case GUI_FILE_SAMPLE_OPEN:
      ImGuiFileDialog::Instance()->OpenModal("FileDialog","Load Sample","Wave file{.wav},.*",workingDir);
      break;
    case GUI_FILE_SAMPLE_SAVE:
      ImGuiFileDialog::Instance()->OpenModal("FileDialog","Save Sample","Wave file{.wav}",workingDir);
      break;
  }
  curFileDialog=type;
}

#define FURNACE_ZLIB_COMPRESS

int FurnaceGUI::save(String path) {
  SafeWriter* w;
  if (path.rfind(".dmf")==path.size()-4) {
    w=e->saveDMF();
  } else {
    w=e->saveFur();
  }
  if (w==NULL) {
    lastError=e->getLastError();
    return 3;
  }
  FILE* outFile=fopen(path.c_str(),"wb");
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
  return 0;
}

int FurnaceGUI::load(String path) {
  if (!path.empty()) {
    logI("loading module...\n");
    FILE* f=fopen(path.c_str(),"rb");
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
    if (len==0x7fffffffffffffff) {
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
  lastError="everything OK";
  undoHist.clear();
  redoHist.clear();
  updateWindowTitle();
  return 0;
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

void FurnaceGUI::processDrags(int dragX, int dragY) {
  if (macroDragActive) {
    if (macroDragLen>0) {
      int x=(dragX-macroDragStart.x)*macroDragLen/macroDragAreaSize.x;
      if (x<0) x=0;
      if (x>=macroDragLen) x=macroDragLen-1;
      int y=round(macroDragMax-((dragY-macroDragStart.y)*(double(macroDragMax-macroDragMin)/(double)macroDragAreaSize.y)));
      if (y>macroDragMax) y=macroDragMax;
      if (y<macroDragMin) y=macroDragMin;
      macroDragTarget[x]=y;
    }
  }
  if (macroLoopDragActive) {
    if (macroLoopDragLen>0) {
      int x=(dragX-macroLoopDragStart.x)*macroLoopDragLen/macroLoopDragAreaSize.x;
      if (x<0) x=0;
      if (x>=macroLoopDragLen) x=-1;
      *macroLoopDragTarget=x;
    }
  }
  if (waveDragActive) {
    if (waveDragLen>0) {
      int x=(dragX-waveDragStart.x)*waveDragLen/waveDragAreaSize.x;
      if (x<0) x=0;
      if (x>=waveDragLen) x=waveDragLen-1;
      int y=round(waveDragMax-((dragY-waveDragStart.y)*(double(waveDragMax-waveDragMin)/(double)waveDragAreaSize.y)));
      if (y>waveDragMax) y=waveDragMax;
      if (y<waveDragMin) y=waveDragMin;
      waveDragTarget[x]=y;
    }
  }
}

#define sysAddOption(x) \
  if (ImGui::MenuItem(e->getSystemName(x))) { \
    if (!e->addSystem(x)) { \
      showError("cannot add platform! ("+e->getLastError()+")"); \
    } \
    updateWindowTitle(); \
  }

#define sysChangeOption(x,y) \
  if (ImGui::MenuItem(e->getSystemName(y),NULL,e->song.system[x]==y)) { \
    e->changeSystem(x,y); \
    updateWindowTitle(); \
  }

bool FurnaceGUI::loop() {
  while (!quit) {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
      ImGui_ImplSDL2_ProcessEvent(&ev);
      switch (ev.type) {
        case SDL_MOUSEMOTION:
          if (selecting) {
            // detect whether we have to scroll
            if (ev.motion.y<patWindowPos.y) {
              addScroll(-1);
            }
            if (ev.motion.y>patWindowPos.y+patWindowSize.y) {
              addScroll(1);
            }
          }
          processDrags(ev.motion.x,ev.motion.y);
          break;
        case SDL_MOUSEBUTTONUP:
          if (macroDragActive || macroLoopDragActive || waveDragActive) modified=true;
          macroDragActive=false;
          macroLoopDragActive=false;
          waveDragActive=false;
          if (selecting) {
            finishSelection();
            if (cursor.xCoarse==selStart.xCoarse && cursor.xFine==selStart.xFine && cursor.y==selStart.y &&
                cursor.xCoarse==selEnd.xCoarse && cursor.xFine==selEnd.xFine && cursor.y==selEnd.y) {
              updateScroll(cursor.y);
            }
          }
          break;
        case SDL_MOUSEBUTTONDOWN:
          aboutOpen=false;
          break;
        case SDL_WINDOWEVENT:
          switch (ev.window.event) {
            case SDL_WINDOWEVENT_RESIZED:
              scrW=ev.window.data1/dpiScale;
              scrH=ev.window.data2/dpiScale;
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
          updateWindowTitle();
        }
      }
      if (ImGui::MenuItem("open...")) {
        if (modified) {
          showWarning("Unsaved changes! Are you sure?",GUI_WARN_OPEN);
        } else {
          openFileDialog(GUI_FILE_OPEN);
        }
      }
      ImGui::Separator();
      if (ImGui::MenuItem("save")) {
        if (curFileName=="") {
          openFileDialog(GUI_FILE_SAVE);
        } else {
          if (save(curFileName)>0) {
            showError(fmt::sprintf("Error while saving file! (%s)",lastError));
          }
        }
      }
      if (ImGui::MenuItem("save as...")) {
        openFileDialog(GUI_FILE_SAVE);
      }
      ImGui::Separator();
      if (ImGui::BeginMenu("add platform...")) {
        sysAddOption(DIV_SYSTEM_GENESIS);
        sysAddOption(DIV_SYSTEM_GENESIS_EXT);
        sysAddOption(DIV_SYSTEM_SMS);
        sysAddOption(DIV_SYSTEM_GB);
        sysAddOption(DIV_SYSTEM_PCE);
        sysAddOption(DIV_SYSTEM_NES);
        sysAddOption(DIV_SYSTEM_C64_8580);
        sysAddOption(DIV_SYSTEM_C64_6581);
        sysAddOption(DIV_SYSTEM_ARCADE);
        sysAddOption(DIV_SYSTEM_YM2610);
        sysAddOption(DIV_SYSTEM_YM2610_EXT);
        sysAddOption(DIV_SYSTEM_AY8910);
        sysAddOption(DIV_SYSTEM_AMIGA);
        sysAddOption(DIV_SYSTEM_YM2151);
        sysAddOption(DIV_SYSTEM_YM2612);
        sysAddOption(DIV_SYSTEM_TIA);
        sysAddOption(DIV_SYSTEM_SAA1099);
        sysAddOption(DIV_SYSTEM_AY8930);
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("change platform...")) {
        for (int i=0; i<e->song.systemLen; i++) {
          if (ImGui::BeginMenu(fmt::sprintf("%d. %s##_SYSC%d",i+1,e->getSystemName(e->song.system[i]),i).c_str())) {
            sysChangeOption(i,DIV_SYSTEM_GENESIS);
            sysChangeOption(i,DIV_SYSTEM_GENESIS_EXT);
            sysChangeOption(i,DIV_SYSTEM_SMS);
            sysChangeOption(i,DIV_SYSTEM_GB);
            sysChangeOption(i,DIV_SYSTEM_PCE);
            sysChangeOption(i,DIV_SYSTEM_NES);
            sysChangeOption(i,DIV_SYSTEM_C64_8580);
            sysChangeOption(i,DIV_SYSTEM_C64_6581);
            sysChangeOption(i,DIV_SYSTEM_ARCADE);
            sysChangeOption(i,DIV_SYSTEM_YM2610);
            sysChangeOption(i,DIV_SYSTEM_YM2610_EXT);
            sysChangeOption(i,DIV_SYSTEM_AY8910);
            sysChangeOption(i,DIV_SYSTEM_AMIGA);
            sysChangeOption(i,DIV_SYSTEM_YM2151);
            sysChangeOption(i,DIV_SYSTEM_YM2612);
            sysChangeOption(i,DIV_SYSTEM_TIA);
            sysChangeOption(i,DIV_SYSTEM_SAA1099);
            sysChangeOption(i,DIV_SYSTEM_AY8930);
            ImGui::EndMenu();
          }
        }
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("remove platform...")) {
        for (int i=0; i<e->song.systemLen; i++) {
          if (ImGui::MenuItem(fmt::sprintf("%d. %s##_SYSR%d",i+1,e->getSystemName(e->song.system[i]),i).c_str())) {
            if (!e->removeSystem(i)) {
              showError("cannot remove platform! ("+e->getLastError()+")");
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
      if (ImGui::MenuItem("undo","Ctrl-Z")) doUndo();
      if (ImGui::MenuItem("redo","Shift-Ctrl-Z")) doRedo();
      ImGui::Separator();
      if (ImGui::MenuItem("cut","Ctrl-X")) doCopy(true);
      if (ImGui::MenuItem("copy","Ctrl-C")) doCopy(false);
      if (ImGui::MenuItem("paste","Ctrl-V")) doPaste();
      if (ImGui::MenuItem("delete","Delete")) doDelete();
      if (ImGui::MenuItem("select all","Ctrl-A")) doSelectAll();
      ImGui::Separator();
      ImGui::MenuItem("note up","Alt-Q");
      ImGui::MenuItem("note down","Alt-A");
      ImGui::MenuItem("octave up","Alt-Shift-Q");
      ImGui::MenuItem("octave down","Alt-Shift-A");
      ImGui::Separator();
      ImGui::MenuItem("clear...");
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("settings")) {
      if (ImGui::MenuItem("settings...")) settingsOpen=true;
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("window")) {
      if (ImGui::MenuItem("play/edit controls")) editControlsOpen=!editControlsOpen;
      if (ImGui::MenuItem("song information")) songInfoOpen=!songInfoOpen;
      if (ImGui::MenuItem("instruments")) insListOpen=!insListOpen;
      if (ImGui::MenuItem("instrument editor")) insEditOpen=!insEditOpen;
      if (ImGui::MenuItem("wavetables")) waveListOpen=!waveListOpen;
      if (ImGui::MenuItem("wavetable editor")) waveEditOpen=!waveEditOpen;
      if (ImGui::MenuItem("samples")) sampleListOpen=!sampleListOpen;
      if (ImGui::MenuItem("sample editor")) sampleEditOpen=!sampleEditOpen;
      if (ImGui::MenuItem("orders")) ordersOpen=!ordersOpen;
      if (ImGui::MenuItem("pattern")) patternOpen=!patternOpen;
      if (ImGui::MenuItem("mixer")) mixerOpen=!mixerOpen;
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("help")) {
      if (ImGui::MenuItem("about...")) {
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
      if (curFileName!="") ImGui::Text("| %s",curFileName.c_str());
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
    drawPattern();
    drawSettings();

    if (ImGuiFileDialog::Instance()->Display("FileDialog")) {
      if (ImGuiFileDialog::Instance()->IsOk()) {
        fileName=ImGuiFileDialog::Instance()->GetFilePathName();
        if (fileName!="") {
          if (curFileDialog==GUI_FILE_SAVE) {
            if (ImGuiFileDialog::Instance()->GetCurrentFilter()=="Furnace song") {
              if (fileName.size()<4 || fileName.rfind(".fur")!=fileName.size()-4) {
                fileName+=".fur";
              }
            } else {
              if (fileName.size()<4 || fileName.rfind(".dmf")!=fileName.size()-4) {
                fileName+=".dmf";
              }
            }
          }
          if (curFileDialog==GUI_FILE_SAMPLE_SAVE) {
            if (fileName.size()<4 || fileName.rfind(".wav")!=fileName.size()-4) {
              fileName+=".wav";
            }
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
              if (save(copyOfName)>0) {
                showError(fmt::sprintf("Error while saving file! (%s)",lastError));
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

    if (aboutOpen) drawAbout();

    if (ImGui::BeginPopupModal("Error",NULL,ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::Text("%s",errorString.c_str());
      if (ImGui::Button("OK")) {
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Warning",NULL,ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::Text("%s",warnString.c_str());
      if (ImGui::Button("Yes")) {
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
            updateWindowTitle();
            break;
          case GUI_WARN_OPEN:
            openFileDialog(GUI_FILE_OPEN);
            break;
        }
      }
      ImGui::SameLine();
      if (ImGui::Button("No")) {
        ImGui::CloseCurrentPopup();
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

    if (willCommit) {
      commitSettings();
      willCommit=false;
    }
  }
  return false;
}

#define IGFD_FileStyleByExtension IGFD_FileStyleByExtention

String getHomeDir() {
  String ret;
  char tempDir[4096];

#ifdef _WIN32
  char* up=getenv("USERPROFILE");
  if (up!=NULL) {
    ret=up;
    ret+='\\';
  }
#else
  char* home=getenv("HOME");
  if (home!=NULL) {
    ret=home;
    ret+='/';
  } else {
    int uid=getuid();
    struct passwd* entry=getpwuid(uid);
    if (entry!=NULL) {
      if (entry->pw_dir!=NULL) {
        ret=entry->pw_dir;
        ret+='/';
      }
    }
  }
#endif

  if (ret=="") { // fallback
#ifdef _WIN32
    GetCurrentDirectory(4095,tempDir);
    ret=tempDir;
    ret+='\\';
#else
    getcwd(tempDir,4095);
    ret=tempDir;
    ret+='/';
#endif
  }

  return ret;
}

bool FurnaceGUI::init() {
  float dpiScaleF;

  workingDir=e->getConfString("lastDir",getHomeDir());

#ifndef __APPLE__
  unsigned char* furIcon=getFurnaceIcon();
  SDL_Surface* icon=SDL_CreateRGBSurfaceFrom(furIcon,256,256,32,256*4,0xff,0xff00,0xff0000,0xff000000);
#endif

  sdlWin=SDL_CreateWindow("Furnace",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,scrW*dpiScale,scrH*dpiScale,SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALLOW_HIGHDPI);
  if (sdlWin==NULL) {
    logE("could not open window!\n");
    return false;
  }

  SDL_GetDisplayDPI(SDL_GetWindowDisplayIndex(sdlWin),&dpiScaleF,NULL,NULL);
  dpiScale=round(dpiScaleF/96.0f);
  if (dpiScale<1) dpiScale=1;
  if (dpiScale!=1) SDL_SetWindowSize(sdlWin,scrW*dpiScale,scrH*dpiScale);

#ifndef __APPLE__
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

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGui::StyleColorsDark();
  ImGuiStyle& sty=ImGui::GetStyle();

  sty.Colors[ImGuiCol_WindowBg]=uiColors[GUI_COLOR_FRAME_BACKGROUND];

  ImGui_ImplSDL2_InitForSDLRenderer(sdlWin);
  ImGui_ImplSDLRenderer_Init(sdlRend);

  sty.ScaleAllSizes(dpiScale);

  strncpy(finalLayoutPath,(e->getConfigPath()+String(LAYOUT_INI)).c_str(),4095);
  prepareLayout();

  ImGui::GetIO().ConfigFlags|=ImGuiConfigFlags_DockingEnable;
  ImGui::GetIO().IniFilename=finalLayoutPath;

  if ((mainFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(defFont_main_compressed_data,defFont_main_compressed_size,e->getConfInt("mainFontSize",18)*dpiScale))==NULL) {
    logE("could not load UI font!\n");
    return false;
  }

  ImFontConfig fc;
  fc.MergeMode=true;
  fc.GlyphMinAdvanceX=e->getConfInt("iconSize",16)*dpiScale;
  static const ImWchar fontRange[]={ICON_MIN_FA,ICON_MAX_FA,0};
  if ((iconFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(iconFont_compressed_data,iconFont_compressed_size,e->getConfInt("iconSize",16)*dpiScale,&fc,fontRange))==NULL) {
    logE("could not load icon font!\n");
    return false;
  }
  if ((patFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(defFont_pat_compressed_data,defFont_pat_compressed_size,e->getConfInt("patFontSize",18)*dpiScale))==NULL) {
    logE("could not load pattern font!\n");
    return false;
  }
  if ((bigFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(defFont_main_compressed_data,defFont_main_compressed_size,40*dpiScale))==NULL) {
    logE("could not load big UI font!\n");
    return false;
  }

  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir,"",ImVec4(0.0f,1.0f,1.0f,1.0f),ICON_FA_FOLDER_O);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile,"",ImVec4(0.7f,0.7f,0.7f,1.0f),ICON_FA_FILE_O);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".fur",ImVec4(0.5f,1.0f,0.5f,1.0f),ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".dmf",ImVec4(0.5f,1.0f,0.5f,1.0f),ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".wav",ImVec4(1.0f,1.0f,0.5f,1.0f),ICON_FA_FILE_AUDIO_O);

  updateWindowTitle();

  for (int i=0; i<DIV_MAX_CHANS; i++) {
    oldPat[i]=new DivPattern;
  }
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
  curFileDialog(GUI_FILE_OPEN),
  warnAction(GUI_WARN_OPEN),
  scrW(1280),
  scrH(800),
  dpiScale(1),
  aboutScroll(0),
  aboutSin(0),
  aboutHue(0.0f),
  mainFontSize(18),
  patFontSize(18),
  maxUndoSteps(100),
  curIns(0),
  curWave(0),
  curSample(0),
  curOctave(3),
  oldRow(0),
  oldOrder(0),
  oldOrder1(0),
  editStep(1),
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
  selecting(false),
  curNibble(false),
  extraChannelButtons(false),
  followOrders(true),
  followPattern(true),
  changeAllOrders(false),
  curWindow(GUI_WINDOW_NOTHING),
  noteOffOnRelease(false),
  noteOffOnReleaseKey(0),
  noteOffOnReleaseChan(0),
  arpMacroScroll(0),
  macroDragStart(0,0),
  macroDragAreaSize(0,0),
  macroDragTarget(NULL),
  macroDragLen(0),
  macroDragMin(0),
  macroDragMax(0),
  macroDragActive(false),
  nextScroll(-1.0f),
  nextAddScroll(0.0f),
  oldOrdersLen(0) {
  uiColors[GUI_COLOR_BACKGROUND]=ImVec4(0.1f,0.1f,0.1f,1.0f);
  uiColors[GUI_COLOR_FRAME_BACKGROUND]=ImVec4(0.0f,0.0f,0.0f,0.85f);
  uiColors[GUI_COLOR_INSTR_FM]=ImVec4(0.6f,0.9f,1.0f,1.0f);
  uiColors[GUI_COLOR_INSTR_STD]=ImVec4(0.6f,1.0f,0.5f,1.0f);
  uiColors[GUI_COLOR_INSTR_GB]=ImVec4(1.0f,1.0f,0.5f,1.0f);
  uiColors[GUI_COLOR_INSTR_C64]=ImVec4(0.85f,0.8f,1.0f,1.0f);
  uiColors[GUI_COLOR_INSTR_AMIGA]=ImVec4(1.0f,0.5f,0.5f,1.0f);
  uiColors[GUI_COLOR_INSTR_PCE]=ImVec4(1.0f,0.8f,0.5f,1.0f);
  uiColors[GUI_COLOR_INSTR_AY]=ImVec4(1.0f,0.5f,1.0f,1.0f);
  uiColors[GUI_COLOR_INSTR_AY8930]=ImVec4(0.7f,0.5f,1.0f,1.0f);
  uiColors[GUI_COLOR_INSTR_TIA]=ImVec4(1.0f,0.6f,0.4f,1.0f);
  uiColors[GUI_COLOR_INSTR_SAA1099]=ImVec4(0.3f,0.3f,1.0f,1.0f);
  uiColors[GUI_COLOR_INSTR_UNKNOWN]=ImVec4(0.3f,0.3f,0.3f,1.0f);
  uiColors[GUI_COLOR_CHANNEL_FM]=ImVec4(0.2f,0.8f,1.0f,1.0f);
  uiColors[GUI_COLOR_CHANNEL_PULSE]=ImVec4(0.4f,1.0f,0.2f,1.0f);
  uiColors[GUI_COLOR_CHANNEL_NOISE]=ImVec4(0.8f,0.8f,0.8f,1.0f);
  uiColors[GUI_COLOR_CHANNEL_PCM]=ImVec4(1.0f,0.9f,0.2f,1.0f);
  uiColors[GUI_COLOR_CHANNEL_WAVE]=ImVec4(1.0f,0.5f,0.2f,1.0f);
  uiColors[GUI_COLOR_CHANNEL_OP]=ImVec4(0.2f,0.4f,1.0f,1.0f);
  uiColors[GUI_COLOR_CHANNEL_MUTED]=ImVec4(0.5f,0.5f,0.5f,1.0f);
  uiColors[GUI_COLOR_PATTERN_CURSOR]=ImVec4(0.1f,0.3f,0.5f,1.0f);
  uiColors[GUI_COLOR_PATTERN_CURSOR_HOVER]=ImVec4(0.2f,0.4f,0.6f,1.0f);
  uiColors[GUI_COLOR_PATTERN_CURSOR_ACTIVE]=ImVec4(0.2f,0.5f,0.7f,1.0f);
  uiColors[GUI_COLOR_PATTERN_SELECTION]=ImVec4(0.15f,0.15f,0.2f,1.0f);
  uiColors[GUI_COLOR_PATTERN_SELECTION_HOVER]=ImVec4(0.2f,0.2f,0.3f,1.0f);
  uiColors[GUI_COLOR_PATTERN_SELECTION_ACTIVE]=ImVec4(0.4f,0.4f,0.5f,1.0f);
  uiColors[GUI_COLOR_PATTERN_HI_1]=ImVec4(0.6f,0.6f,0.6f,0.2f);
  uiColors[GUI_COLOR_PATTERN_HI_2]=ImVec4(0.5f,0.8f,1.0f,0.2f);
  uiColors[GUI_COLOR_PATTERN_ROW_INDEX]=ImVec4(0.5f,0.8f,1.0f,1.0f);
  uiColors[GUI_COLOR_PATTERN_ACTIVE]=ImVec4(1.0f,1.0f,1.0f,1.0f);
  uiColors[GUI_COLOR_PATTERN_INACTIVE]=ImVec4(0.5f,0.5f,0.5f,1.0f);
  uiColors[GUI_COLOR_PATTERN_INS]=ImVec4(0.4f,0.7f,1.0f,1.0f);
  uiColors[GUI_COLOR_PATTERN_VOLUME_MIN]=ImVec4(0.0f,0.5f,0.0f,1.0f);
  uiColors[GUI_COLOR_PATTERN_VOLUME_HALF]=ImVec4(0.0f,0.75f,0.0f,1.0f);
  uiColors[GUI_COLOR_PATTERN_VOLUME_MAX]=ImVec4(0.0f,1.0f,0.0f,1.0f);
  uiColors[GUI_COLOR_PATTERN_EFFECT_INVALID]=ImVec4(1.0f,0.0f,0.0f,1.0f);
  uiColors[GUI_COLOR_PATTERN_EFFECT_PITCH]=ImVec4(1.0f,1.0f,0.0f,1.0f);
  uiColors[GUI_COLOR_PATTERN_EFFECT_VOLUME]=ImVec4(0.0f,1.0f,0.0f,1.0f);
  uiColors[GUI_COLOR_PATTERN_EFFECT_PANNING]=ImVec4(0.0f,1.0f,1.0f,1.0f);
  uiColors[GUI_COLOR_PATTERN_EFFECT_SONG]=ImVec4(1.0f,0.0f,0.0f,1.0f);
  uiColors[GUI_COLOR_PATTERN_EFFECT_TIME]=ImVec4(0.5f,0.0f,1.0f,1.0f);
  uiColors[GUI_COLOR_PATTERN_EFFECT_SPEED]=ImVec4(1.0f,0.0f,1.0f,1.0f);
  uiColors[GUI_COLOR_PATTERN_EFFECT_SYS_PRIMARY]=ImVec4(0.5f,1.0f,0.0f,1.0f);
  uiColors[GUI_COLOR_PATTERN_EFFECT_SYS_SECONDARY]=ImVec4(0.0f,1.0f,0.5f,1.0f);
  uiColors[GUI_COLOR_PATTERN_EFFECT_MISC]=ImVec4(0.3f,0.3f,1.0f,1.0f);
  uiColors[GUI_COLOR_EE_VALUE]=ImVec4(0.0f,1.0f,1.0f,1.0f);
  uiColors[GUI_COLOR_PLAYBACK_STAT]=ImVec4(0.6f,0.6f,0.6f,1.0f);

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

  // octave 1
  noteKeys[SDLK_z]=0;
  noteKeys[SDLK_s]=1;
  noteKeys[SDLK_x]=2;
  noteKeys[SDLK_d]=3;
  noteKeys[SDLK_c]=4;
  noteKeys[SDLK_v]=5;
  noteKeys[SDLK_g]=6;
  noteKeys[SDLK_b]=7;
  noteKeys[SDLK_h]=8;
  noteKeys[SDLK_n]=9;
  noteKeys[SDLK_j]=10;
  noteKeys[SDLK_m]=11;

  // octave 2
  noteKeys[SDLK_q]=12;
  noteKeys[SDLK_2]=13;
  noteKeys[SDLK_w]=14;
  noteKeys[SDLK_3]=15;
  noteKeys[SDLK_e]=16;
  noteKeys[SDLK_r]=17;
  noteKeys[SDLK_5]=18;
  noteKeys[SDLK_t]=19;
  noteKeys[SDLK_6]=20;
  noteKeys[SDLK_y]=21;
  noteKeys[SDLK_7]=22;
  noteKeys[SDLK_u]=23;

  // octave 3
  noteKeys[SDLK_i]=24;
  noteKeys[SDLK_9]=25;
  noteKeys[SDLK_o]=26;
  noteKeys[SDLK_0]=27;
  noteKeys[SDLK_p]=28;
  noteKeys[SDLK_LEFTBRACKET]=29;
  noteKeys[SDLK_RIGHTBRACKET]=31;

  // note off
  noteKeys[SDLK_EQUALS]=100;
  noteKeys[SDLK_TAB]=100;

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
}
