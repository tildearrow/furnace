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

// for suck's fake Clang extension!
#define _USE_MATH_DEFINES
#include "gui.h"
#include "../ta-log.h"
#include "imgui_internal.h"
#include "IconsFontAwesome4.h"
#include "misc/cpp/imgui_stdlib.h"
#include "guiConst.h"
#include "../utfutils.h"
#include <fmt/printf.h>

inline float randRange(float min, float max) {
  return min+((float)rand()/(float)RAND_MAX)*(max-min);
}

void _pushPartBlend(const ImDrawList* drawList, const ImDrawCmd* cmd) {
  if (cmd!=NULL) {
    if (cmd->UserCallbackData!=NULL) {
      ((FurnaceGUI*)cmd->UserCallbackData)->pushPartBlend();
    }
  }
}

void _popPartBlend(const ImDrawList* drawList, const ImDrawCmd* cmd) {
  if (cmd!=NULL) {
    if (cmd->UserCallbackData!=NULL) {
      ((FurnaceGUI*)cmd->UserCallbackData)->popPartBlend();
    }
  }
}

void FurnaceGUI::pushPartBlend() {
  SDL_SetRenderDrawBlendMode(sdlRend,SDL_BLENDMODE_ADD);
}

void FurnaceGUI::popPartBlend() {
  SDL_SetRenderDrawBlendMode(sdlRend,SDL_BLENDMODE_BLEND);
}

// draw a pattern row
inline void FurnaceGUI::patternRow(int i, bool isPlaying, float lineHeight, int chans, int ord, const DivPattern** patCache, bool inhibitSel) {
  static char id[64];
  bool selectedRow=(i>=sel1.y && i<=sel2.y && !inhibitSel);
  ImGui::TableNextRow(0,lineHeight);
  ImGui::TableNextColumn();
  float cursorPosY=ImGui::GetCursorPos().y-ImGui::GetScrollY();
  // check if the row is visible
  if (cursorPosY<-lineHeight || cursorPosY>ImGui::GetWindowSize().y) {
    return;
  }
  // check if we are in range
  if (ord<0 || ord>=e->curSubSong->ordersLen) {
    return;
  }
  if (i<0 || i>=e->curSubSong->patLen) {
    return;
  }
  bool isPushing=false;
  ImVec4 activeColor=uiColors[GUI_COLOR_PATTERN_ACTIVE];
  ImVec4 inactiveColor=uiColors[GUI_COLOR_PATTERN_INACTIVE];
  ImVec4 rowIndexColor=uiColors[GUI_COLOR_PATTERN_ROW_INDEX];
  if (e->curSubSong->hilightB>0 && !(i%e->curSubSong->hilightB)) {
    activeColor=uiColors[GUI_COLOR_PATTERN_ACTIVE_HI2];
    inactiveColor=uiColors[GUI_COLOR_PATTERN_INACTIVE_HI2];
    rowIndexColor=uiColors[GUI_COLOR_PATTERN_ROW_INDEX_HI2];
  } else if (e->curSubSong->hilightA>0 && !(i%e->curSubSong->hilightA)) {
    activeColor=uiColors[GUI_COLOR_PATTERN_ACTIVE_HI1];
    inactiveColor=uiColors[GUI_COLOR_PATTERN_INACTIVE_HI1];
    rowIndexColor=uiColors[GUI_COLOR_PATTERN_ROW_INDEX_HI1];
  }
  // check overflow highlight
  if (settings.overflowHighlight) {
    if (edit && cursor.y==i && curWindowLast==GUI_WINDOW_PATTERN) {
      ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,ImGui::GetColorU32(uiColors[GUI_COLOR_EDITING]));
    } else if (isPlaying && oldRow==i && ord==e->getOrder()) {
      ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,ImGui::GetColorU32(uiColors[GUI_COLOR_PATTERN_PLAY_HEAD]));
    } else if (e->curSubSong->hilightB>0 && !(i%e->curSubSong->hilightB)) {
      ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,ImGui::GetColorU32(uiColors[GUI_COLOR_PATTERN_HI_2]));
    } else if (e->curSubSong->hilightA>0 && !(i%e->curSubSong->hilightA)) {
      ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,ImGui::GetColorU32(uiColors[GUI_COLOR_PATTERN_HI_1]));
    }
  } else {
    isPushing=true;
    if (edit && cursor.y==i && curWindowLast==GUI_WINDOW_PATTERN) {
      ImGui::PushStyleColor(ImGuiCol_Header,ImGui::GetColorU32(uiColors[GUI_COLOR_EDITING]));
    } else if (isPlaying && oldRow==i && ord==e->getOrder()) {
      ImGui::PushStyleColor(ImGuiCol_Header,ImGui::GetColorU32(uiColors[GUI_COLOR_PATTERN_PLAY_HEAD]));
    } else if (e->curSubSong->hilightB>0 && !(i%e->curSubSong->hilightB)) {
      ImGui::PushStyleColor(ImGuiCol_Header,ImGui::GetColorU32(uiColors[GUI_COLOR_PATTERN_HI_2]));
    } else if (e->curSubSong->hilightA>0 && !(i%e->curSubSong->hilightA)) {
      ImGui::PushStyleColor(ImGuiCol_Header,ImGui::GetColorU32(uiColors[GUI_COLOR_PATTERN_HI_1]));
    } else {
      isPushing=false;
    }
  }
  // row number
  ImGui::PushStyleColor(ImGuiCol_Text,rowIndexColor);
  
  if (settings.patRowsBase==1) {
    snprintf(id,63," %.2X ##PR_%d",i,i);
  } else {
    snprintf(id,63,"%3d ##PR_%d",i,i);
  }
  ImGui::Selectable(id,false,ImGuiSelectableFlags_NoPadWithHalfSpacing,fourChars);
  if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
    updateSelection(0,0,i,true);
  }
  if (ImGui::IsItemClicked()) {
    startSelection(0,0,i,true);
  }
  if (ImGui::IsItemActive() && CHECK_LONG_HOLD) {
    ImGui::InhibitInertialScroll();
    NOTIFY_LONG_HOLD;
  }
  ImGui::PopStyleColor();
  // for each column
  for (int j=0; j<chans; j++) {
    // check if channel is not hidden
    if (!e->curSubSong->chanShow[j]) {
      patChanX[j]=ImGui::GetCursorScreenPos().x;
      continue;
    }
    int chanVolMax=e->getMaxVolumeChan(j);
    if (chanVolMax<1) chanVolMax=1;
    const DivPattern* pat=patCache[j];
    ImGui::TableNextColumn();
    patChanX[j]=ImGui::GetCursorScreenPos().x;

    // selection highlight flags
    int sel1XSum=sel1.xCoarse*32+sel1.xFine;
    int sel2XSum=sel2.xCoarse*32+sel2.xFine;
    int j32=j*32;
    bool selectedNote=selectedRow && (j32>=sel1XSum && j32<=sel2XSum);
    bool selectedIns=selectedRow && (j32+1>=sel1XSum && j32+1<=sel2XSum);
    bool selectedVol=selectedRow && (j32+2>=sel1XSum && j32+2<=sel2XSum);
    bool cursorNote=(cursor.y==i && cursor.xCoarse==j && cursor.xFine==0 && curWindowLast==GUI_WINDOW_PATTERN);
    bool cursorIns=(cursor.y==i && cursor.xCoarse==j && cursor.xFine==1 && curWindowLast==GUI_WINDOW_PATTERN);
    bool cursorVol=(cursor.y==i && cursor.xCoarse==j && cursor.xFine==2 && curWindowLast==GUI_WINDOW_PATTERN);

    // note
    snprintf(id,63,"%.31s##PN_%d_%d",noteName(pat->data[i][0],pat->data[i][1]),i,j);
    if (pat->data[i][0]==0 && pat->data[i][1]==0) {
      ImGui::PushStyleColor(ImGuiCol_Text,inactiveColor);
    } else {
      ImGui::PushStyleColor(ImGuiCol_Text,activeColor);
    }
    if (cursorNote) {
      ImGui::PushStyleColor(ImGuiCol_Header,uiColors[GUI_COLOR_PATTERN_CURSOR]);
      ImGui::PushStyleColor(ImGuiCol_HeaderActive,uiColors[GUI_COLOR_PATTERN_CURSOR_ACTIVE]);
      ImGui::PushStyleColor(ImGuiCol_HeaderHovered,uiColors[GUI_COLOR_PATTERN_CURSOR_HOVER]);
      ImGui::Selectable(id,true,ImGuiSelectableFlags_NoPadWithHalfSpacing,noteCellSize);
      demandX=ImGui::GetCursorPosX();
      ImGui::PopStyleColor(3);
    } else {
      if (selectedNote) ImGui::PushStyleColor(ImGuiCol_Header,uiColors[GUI_COLOR_PATTERN_SELECTION]);
      ImGui::Selectable(id,isPushing || selectedNote,ImGuiSelectableFlags_NoPadWithHalfSpacing,noteCellSize);
      if (selectedNote) ImGui::PopStyleColor();
    }
    if (ImGui::IsItemClicked()) {
      startSelection(j,0,i);
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
      updateSelection(j,0,i);
    }
    if (ImGui::IsItemActive() && CHECK_LONG_HOLD) {
      ImGui::InhibitInertialScroll();
      NOTIFY_LONG_HOLD;
    }
    ImGui::PopStyleColor();

    // the following is only visible when the channel is not collapsed
    if (e->curSubSong->chanCollapse[j]<3) {
      // instrument
      if (pat->data[i][2]==-1) {
        ImGui::PushStyleColor(ImGuiCol_Text,inactiveColor);
        snprintf(id,63,"%.31s##PI_%d_%d",emptyLabel2,i,j);
      } else {
        if (pat->data[i][2]<0 || pat->data[i][2]>=e->song.insLen) {
          ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_INS_ERROR]);
        } else {
          DivInstrumentType t=e->song.ins[pat->data[i][2]]->type;
          if (t!=DIV_INS_AMIGA && t!=e->getPreferInsType(j)) {
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_INS_WARN]);
          } else {
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_INS]);
          }
        }
        snprintf(id,63,"%.2X##PI_%d_%d",pat->data[i][2],i,j);
      }
      ImGui::SameLine(0.0f,0.0f);
      if (cursorIns) {
        ImGui::PushStyleColor(ImGuiCol_Header,uiColors[GUI_COLOR_PATTERN_CURSOR]);
        ImGui::PushStyleColor(ImGuiCol_HeaderActive,uiColors[GUI_COLOR_PATTERN_CURSOR_ACTIVE]);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered,uiColors[GUI_COLOR_PATTERN_CURSOR_HOVER]);
        ImGui::Selectable(id,true,ImGuiSelectableFlags_NoPadWithHalfSpacing,insCellSize);
        demandX=ImGui::GetCursorPosX();
        ImGui::PopStyleColor(3);
      } else {
        if (selectedIns) ImGui::PushStyleColor(ImGuiCol_Header,uiColors[GUI_COLOR_PATTERN_SELECTION]);
        ImGui::Selectable(id,isPushing || selectedIns,ImGuiSelectableFlags_NoPadWithHalfSpacing,insCellSize);
        if (selectedIns) ImGui::PopStyleColor();
      }
      if (ImGui::IsItemClicked()) {
        startSelection(j,1,i);
      }
      if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
        updateSelection(j,1,i);
      }
      if (ImGui::IsItemActive() && CHECK_LONG_HOLD) {
        ImGui::InhibitInertialScroll();
        NOTIFY_LONG_HOLD;
      }
      ImGui::PopStyleColor();
    }

    if (e->curSubSong->chanCollapse[j]<2) {
      // volume
      if (pat->data[i][3]==-1) {
        snprintf(id,63,"%.31s##PV_%d_%d",emptyLabel2,i,j);
        ImGui::PushStyleColor(ImGuiCol_Text,inactiveColor);
      } else {
        int volColor=(pat->data[i][3]*127)/chanVolMax;
        if (volColor>127) volColor=127;
        if (volColor<0) volColor=0;
        snprintf(id,63,"%.2X##PV_%d_%d",pat->data[i][3],i,j);
        ImGui::PushStyleColor(ImGuiCol_Text,volColors[volColor]);
      }
      ImGui::SameLine(0.0f,0.0f);
      if (cursorVol) {
        ImGui::PushStyleColor(ImGuiCol_Header,uiColors[GUI_COLOR_PATTERN_CURSOR]);
        ImGui::PushStyleColor(ImGuiCol_HeaderActive,uiColors[GUI_COLOR_PATTERN_CURSOR_ACTIVE]);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered,uiColors[GUI_COLOR_PATTERN_CURSOR_HOVER]);
        ImGui::Selectable(id,true,ImGuiSelectableFlags_NoPadWithHalfSpacing,volCellSize);
        demandX=ImGui::GetCursorPosX();
        ImGui::PopStyleColor(3);
      } else {
        if (selectedVol) ImGui::PushStyleColor(ImGuiCol_Header,uiColors[GUI_COLOR_PATTERN_SELECTION]);
        ImGui::Selectable(id,isPushing || selectedVol,ImGuiSelectableFlags_NoPadWithHalfSpacing,volCellSize);
        if (selectedVol) ImGui::PopStyleColor();
      }
      if (ImGui::IsItemClicked()) {
        startSelection(j,2,i);
      }
      if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
        updateSelection(j,2,i);
      }
      if (ImGui::IsItemActive() && CHECK_LONG_HOLD) {
        ImGui::InhibitInertialScroll();
        NOTIFY_LONG_HOLD;
      }
      ImGui::PopStyleColor();
    }

    if (e->curSubSong->chanCollapse[j]<1) {
      // effects
      for (int k=0; k<e->curPat[j].effectCols; k++) {
        int index=4+(k<<1);
        bool selectedEffect=selectedRow && (j32+index-1>=sel1XSum && j32+index-1<=sel2XSum);
        bool selectedEffectVal=selectedRow && (j32+index>=sel1XSum && j32+index<=sel2XSum);
        bool cursorEffect=(cursor.y==i && cursor.xCoarse==j && cursor.xFine==index-1 && curWindowLast==GUI_WINDOW_PATTERN);
        bool cursorEffectVal=(cursor.y==i && cursor.xCoarse==j && cursor.xFine==index && curWindowLast==GUI_WINDOW_PATTERN);
        
        // effect
        if (pat->data[i][index]==-1) {
          snprintf(id,63,"%.31s##PE%d_%d_%d",emptyLabel2,k,i,j);
          ImGui::PushStyleColor(ImGuiCol_Text,inactiveColor);
        } else {
          if (pat->data[i][index]>0xff) {
            snprintf(id,63,"??##PE%d_%d_%d",k,i,j);
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_EFFECT_INVALID]);
          } else {
            const unsigned char data=pat->data[i][index];
            snprintf(id,63,"%.2X##PE%d_%d_%d",data,k,i,j);
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[fxColors[data]]);
          }
        }
        ImGui::SameLine(0.0f,0.0f);
        if (cursorEffect) {
          ImGui::PushStyleColor(ImGuiCol_Header,uiColors[GUI_COLOR_PATTERN_CURSOR]);  
          ImGui::PushStyleColor(ImGuiCol_HeaderActive,uiColors[GUI_COLOR_PATTERN_CURSOR_ACTIVE]);
          ImGui::PushStyleColor(ImGuiCol_HeaderHovered,uiColors[GUI_COLOR_PATTERN_CURSOR_HOVER]);
          ImGui::Selectable(id,true,ImGuiSelectableFlags_NoPadWithHalfSpacing,effectCellSize);
          demandX=ImGui::GetCursorPosX();
          ImGui::PopStyleColor(3);
        } else {
          if (selectedEffect) ImGui::PushStyleColor(ImGuiCol_Header,uiColors[GUI_COLOR_PATTERN_SELECTION]);
          ImGui::Selectable(id,isPushing || selectedEffect,ImGuiSelectableFlags_NoPadWithHalfSpacing,effectCellSize);
          if (selectedEffect) ImGui::PopStyleColor();
        }
        if (ImGui::IsItemClicked()) {
          startSelection(j,index-1,i);
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
          updateSelection(j,index-1,i);
        }
        if (ImGui::IsItemActive() && CHECK_LONG_HOLD) {
          ImGui::InhibitInertialScroll();
          NOTIFY_LONG_HOLD;
        }

        // effect value
        if (pat->data[i][index+1]==-1) {
          snprintf(id,63,"%.31s##PF%d_%d_%d",emptyLabel2,k,i,j);
        } else {
          snprintf(id,63,"%.2X##PF%d_%d_%d",pat->data[i][index+1],k,i,j);
        }
        ImGui::SameLine(0.0f,0.0f);
        if (cursorEffectVal) {
          ImGui::PushStyleColor(ImGuiCol_Header,uiColors[GUI_COLOR_PATTERN_CURSOR]);  
          ImGui::PushStyleColor(ImGuiCol_HeaderActive,uiColors[GUI_COLOR_PATTERN_CURSOR_ACTIVE]);
          ImGui::PushStyleColor(ImGuiCol_HeaderHovered,uiColors[GUI_COLOR_PATTERN_CURSOR_HOVER]);
          ImGui::Selectable(id,true,ImGuiSelectableFlags_NoPadWithHalfSpacing,effectValCellSize);
          demandX=ImGui::GetCursorPosX();
          ImGui::PopStyleColor(3);
        } else {
          if (selectedEffectVal) ImGui::PushStyleColor(ImGuiCol_Header,uiColors[GUI_COLOR_PATTERN_SELECTION]);
          ImGui::Selectable(id,isPushing || selectedEffectVal,ImGuiSelectableFlags_NoPadWithHalfSpacing,effectValCellSize);
          if (selectedEffectVal) ImGui::PopStyleColor();
        }
        if (ImGui::IsItemClicked()) {
          startSelection(j,index,i);
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
          updateSelection(j,index,i);
        }
        if (ImGui::IsItemActive() && CHECK_LONG_HOLD) {
          ImGui::InhibitInertialScroll();
          NOTIFY_LONG_HOLD;
        }
        ImGui::PopStyleColor();
      }
    }
  }
  if (isPushing) {
    ImGui::PopStyleColor();
  }
  ImGui::TableNextColumn();
  patChanX[chans]=ImGui::GetCursorScreenPos().x;
}

void FurnaceGUI::drawPattern() {
  //int delta0=SDL_GetPerformanceCounter();
  if (nextWindow==GUI_WINDOW_PATTERN) {
    patternOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!patternOpen) return;

  bool inhibitMenu=false;
  float scrollX=0;

  if (e->isPlaying() && followPattern && (!e->isStepping() || pendingStepUpdate)) {
    cursor.y=oldRow+((pendingStepUpdate)?1:0);
    if (selStart.xCoarse==selEnd.xCoarse && selStart.xFine==selEnd.xFine && selStart.y==selEnd.y && !selecting) {
      selStart=cursor;
      selEnd=cursor;
    }
  }
  demandX=0;
  sel1=selStart;
  sel2=selEnd;
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
  if (mobileUI) {
    patWindowPos=(portrait?ImVec2(0.0f,(mobileMenuPos*-0.65*canvasH)+(0.12*canvasW)):ImVec2((0.16*canvasH)+0.5*canvasW*mobileMenuPos,0.0f));
    patWindowSize=(portrait?ImVec2(canvasW,canvasH-(0.16*canvasW)-(0.12*canvasW)-(pianoOpen?(0.4*canvasW):0.0f)):ImVec2(canvasW-(0.16*canvasH),canvasH-(pianoOpen?(0.3*canvasH):0.0f)));
    ImGui::SetNextWindowPos(patWindowPos);
    ImGui::SetNextWindowSize(patWindowSize);
  }
  if (ImGui::Begin("Pattern",&patternOpen,globalWinFlags|(settings.avoidRaisingPattern?ImGuiWindowFlags_NoBringToFrontOnFocus:0))) {
    if (!mobileUI) {
      patWindowPos=ImGui::GetWindowPos();
      patWindowSize=ImGui::GetWindowSize();
    }
    //char id[32];
    ImGui::PushFont(patFont);
    int ord=oldOrder;
    if (followPattern) {
      curOrder=e->getOrder();
    }
    oldOrder=curOrder;
    int chans=e->getTotalChannelCount();
    int displayChans=0;
    const DivPattern* patCache[DIV_MAX_CHANS];
    for (int i=0; i<chans; i++) {
      if (e->curSubSong->chanShow[i]) displayChans++;
    }
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding,ImVec2(0.0f,0.0f));
    ImGui::PushStyleColor(ImGuiCol_Header,uiColors[GUI_COLOR_PATTERN_SELECTION]);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered,uiColors[GUI_COLOR_PATTERN_SELECTION_HOVER]);
    ImGui::PushStyleColor(ImGuiCol_HeaderActive,uiColors[GUI_COLOR_PATTERN_SELECTION_ACTIVE]);
    if (settings.centerPattern) {
      float centerOff=(ImGui::GetContentRegionAvail().x-lastPatternWidth)*0.5;
      if (centerOff>0.0f) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX()+centerOff);
      }
    }
    if (ImGui::BeginTable("PatternView",displayChans+2,ImGuiTableFlags_BordersInnerV|ImGuiTableFlags_ScrollX|ImGuiTableFlags_ScrollY|ImGuiTableFlags_NoPadInnerX|ImGuiTableFlags_NoBordersInFrozenArea)) {
      ImGui::TableSetupColumn("pos",ImGuiTableColumnFlags_WidthFixed);
      char chanID[2048];
      float lineHeight=(ImGui::GetTextLineHeight()+2*dpiScale);
      int curRow=e->getRow();
      if (e->isPlaying() && followPattern && (!e->isStepping() || pendingStepUpdate)) updateScroll(curRow);
      pendingStepUpdate=false;
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
        if (!e->curSubSong->chanShow[i]) continue;
        ImGui::TableSetupColumn(fmt::sprintf("c%d",i).c_str(),ImGuiTableColumnFlags_WidthFixed);
      }
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      float lpwStart=ImGui::GetCursorPosX();
      if (ImGui::Selectable((extraChannelButtons==2)?" --##ExtraChannelButtons":" ++##ExtraChannelButtons",false,ImGuiSelectableFlags_NoPadWithHalfSpacing,ImVec2(0.0f,lineHeight+1.0f*dpiScale))) {
        if (++extraChannelButtons>2) extraChannelButtons=0;
      }
      if (ImGui::IsItemHovered() && !mobileUI) {
        if (extraChannelButtons==2) {
          ImGui::SetTooltip("Pattern names (click to collapse)\nRight-click for visualizer");
        } else if (extraChannelButtons==1) {
          ImGui::SetTooltip("Expanded (click for pattern names)\nRight-click for visualizer");
        } else {
          ImGui::SetTooltip("Compact (click to expand)\nRight-click for visualizer");
        }
      }
      if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        fancyPattern=!fancyPattern;
        inhibitMenu=true;
        e->enableCommandStream(fancyPattern);
        e->getCommandStream(cmdStream);
        cmdStream.clear();
      }
      for (int i=0; i<chans; i++) {
        if (!e->curSubSong->chanShow[i]) continue;
        ImGui::TableNextColumn();
        bool displayTooltip=false;

        bool muted=e->isChannelMuted(i);
        ImVec4 chanHead=muted?uiColors[GUI_COLOR_CHANNEL_MUTED]:channelColor(i);
        ImVec4 chanHeadActive=chanHead;
        ImVec4 chanHeadHover=chanHead;
        ImVec4 chanHeadBase=chanHead;

        if (e->keyHit[i]) {
          keyHit1[i]=1.0f;
          if (settings.channelFeedbackStyle==1) {
            keyHit[i]=0.2;
            if (!muted) {
              int note=e->getChanState(i)->note+60;
              if (note>=0 && note<180) {
                pianoKeyHit[note]=1.0;
              }
            }
          }
          e->keyHit[i]=false;
        }
        if (settings.channelFeedbackStyle==2 && e->isRunning()) {
          float amount=((float)(e->getChanState(i)->volume>>8)/(float)e->getMaxVolumeChan(i));
          if (!e->getChanState(i)->keyOn) amount=0.0f;
          keyHit[i]=amount*0.2f;
          if (!muted) {
            int note=e->getChanState(i)->note+60;
            if (note>=0 && note<180) {
              pianoKeyHit[note]=amount;
            }
          }
        } else if (settings.channelFeedbackStyle==3 && e->isRunning()) {
          bool active=e->getChanState(i)->keyOn;
          keyHit[i]=active?0.2f:0.0f;
          if (!muted) {
            int note=e->getChanState(i)->note+60;
            if (note>=0 && note<180) {
              pianoKeyHit[note]=active?1.0f:0.0f;
            }
          }
        }
        if (settings.guiColorsBase) {
          chanHead.x*=1.0-keyHit[i]; chanHead.y*=1.0-keyHit[i]; chanHead.z*=1.0-keyHit[i];
          chanHeadActive.x*=0.5; chanHeadActive.y*=0.5; chanHeadActive.z*=0.5;
          chanHeadHover.x*=0.9-keyHit[i]; chanHeadHover.y*=0.9-keyHit[i]; chanHeadHover.z*=0.9-keyHit[i];
        } else {
          chanHead.x*=0.25+keyHit[i]; chanHead.y*=0.25+keyHit[i]; chanHead.z*=0.25+keyHit[i];
          chanHeadActive.x*=0.8; chanHeadActive.y*=0.8; chanHeadActive.z*=0.8;
          chanHeadHover.x*=0.4+keyHit[i]; chanHeadHover.y*=0.4+keyHit[i]; chanHeadHover.z*=0.4+keyHit[i];
        }
        keyHit[i]-=((settings.channelStyle==0)?0.02:0.01)*60.0*ImGui::GetIO().DeltaTime;
        if (keyHit[i]<0) keyHit[i]=0;
        ImGui::PushStyleColor(ImGuiCol_Header,chanHead);
        ImGui::PushStyleColor(ImGuiCol_HeaderActive,chanHeadActive);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered,chanHeadHover);
        ImGui::PushStyleColor(ImGuiCol_Text,ImGui::GetColorU32(channelTextColor(i)));
        if (settings.channelStyle==0) ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,ImGui::GetColorU32(chanHead));
        if (muted) ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_CHANNEL_MUTED]);
        if (settings.channelFont==0) ImGui::PushFont(mainFont);

        // TODO: appearance
        ImGuiWindow* window=ImGui::GetCurrentWindow();
        ImVec2 size=ImVec2(
          1.0f,
          lineHeight+1.0f*dpiScale
        );
        ImDrawList* dl=ImGui::GetWindowDrawList();

        if (settings.channelStyle!=0) {
          size.y+=6.0f*dpiScale;
        }

        if (settings.channelStyle==2) {
          size.y+=6.0f*dpiScale;
        }
        
        ImVec2 minArea=window->DC.CursorPos;
        ImVec2 maxArea=ImVec2(
          minArea.x+window->WorkRect.Max.x-window->WorkRect.Min.x,
          minArea.y+size.y
        );
        ImRect rect=ImRect(minArea,maxArea);
        float padding=ImGui::CalcTextSize("A").x;

        ImVec2 minLabelArea=minArea;
        ImVec2 maxLabelArea=maxArea;

        if (e->curSubSong->chanCollapse[i]) {
          const char* chName=e->getChannelShortName(i);
          if (strlen(chName)>3) {
            snprintf(chanID,2048,"...");
          } else {
            snprintf(chanID,2048,"%s",chName);
          }
          displayTooltip=true;
        } else {
          minLabelArea.x+=padding;
          maxLabelArea.x-=padding;
          if (settings.channelStyle==3) { // split button
            maxLabelArea.x-=ImGui::GetFrameHeightWithSpacing();
          }
          const char* chName=e->getChannelName(i);
          float chNameLimit=maxLabelArea.x-minLabelArea.x;
          if (ImGui::CalcTextSize(chName).x>chNameLimit) {
            String shortChName;
            float totalAdvanced=0.0f;
            float ellipsisSize=ImGui::CalcTextSize("...").x;
            for (const char* j=chName; *j;) {
              signed char l;
              int ch=decodeUTF8((const unsigned char*)j,l);

              totalAdvanced+=ImGui::GetFont()->GetCharAdvance(ch);
              if (totalAdvanced>(chNameLimit-ellipsisSize)) break;

              for (int k=0; k<l; k++) {
                shortChName+=j[k];
              }

              j+=l;
            }
            shortChName+="...";
            snprintf(chanID,2048,"%s",shortChName.c_str());
            displayTooltip=true;
          } else {
            snprintf(chanID,2048,"%s",chName);
          }
        }

        if (settings.channelTextCenter) {
          minLabelArea.x+=0.5f*(maxLabelArea.x-minLabelArea.x-ImGui::CalcTextSize(chanID).x);
        }

        if (extraChannelButtons==0 || settings.channelVolStyle!=0) ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,ImVec2(0.0f,0.0f));

        ImGui::PushID(2048+i);
        switch (settings.channelStyle) {
          case 0: // classic
            ImGui::ItemSize(size,ImGui::GetStyle().FramePadding.y);
            if (ImGui::ItemAdd(rect,ImGui::GetID(chanID))) {
              bool hovered=ImGui::ItemHoverable(rect,ImGui::GetID(chanID));
              ImU32 col=(hovered || (mobileUI && ImGui::IsMouseDown(ImGuiMouseButton_Left)))?ImGui::GetColorU32(ImGuiCol_HeaderHovered):ImGui::GetColorU32(ImGuiCol_Header);
              dl->AddRectFilled(rect.Min,rect.Max,col);
              dl->AddText(ImVec2(minLabelArea.x,rect.Min.y),ImGui::GetColorU32(channelTextColor(i)),chanID);
            }
            break;
          case 1: { // line
            ImGui::ItemSize(size,ImGui::GetStyle().FramePadding.y);
            if (ImGui::ItemAdd(rect,ImGui::GetID(chanID))) {
              bool hovered=ImGui::ItemHoverable(rect,ImGui::GetID(chanID));
              ImU32 fadeCol0=ImGui::GetColorU32(ImVec4(
                chanHeadBase.x,
                chanHeadBase.y,
                chanHeadBase.z,
                (hovered && (!mobileUI || ImGui::IsMouseDown(ImGuiMouseButton_Left)))?0.25f:0.0f
              ));
              ImU32 fadeCol=ImGui::GetColorU32(ImVec4(
                chanHeadBase.x,
                chanHeadBase.y,
                chanHeadBase.z,
                (hovered && (!mobileUI || ImGui::IsMouseDown(ImGuiMouseButton_Left)))?0.5f:MIN(1.0f,chanHeadBase.w*keyHit[i]*4.0f)
              ));
              dl->AddRectFilledMultiColor(rect.Min,rect.Max,fadeCol0,fadeCol0,fadeCol,fadeCol);
              dl->AddLine(ImVec2(rect.Min.x,rect.Max.y),ImVec2(rect.Max.x,rect.Max.y),ImGui::GetColorU32(chanHeadBase),2.0f*dpiScale);
              dl->AddText(ImVec2(minLabelArea.x,rect.Min.y+3.0*dpiScale),ImGui::GetColorU32(channelTextColor(i)),chanID);
            }
            break;
          }
          case 2: { // round
            ImGui::ItemSize(size,ImGui::GetStyle().FramePadding.y);
            if (ImGui::ItemAdd(rect,ImGui::GetID(chanID))) {
              bool hovered=ImGui::ItemHoverable(rect,ImGui::GetID(chanID));
              ImU32 fadeCol0=ImGui::GetColorU32(ImVec4(
                chanHeadBase.x,
                chanHeadBase.y,
                chanHeadBase.z,
                (hovered && (!mobileUI || ImGui::IsMouseDown(ImGuiMouseButton_Left)))?0.5f:MIN(1.0f,0.3f+chanHeadBase.w*keyHit[i]*1.5f)
              ));
              ImU32 fadeCol=ImGui::GetColorU32(ImVec4(
                chanHeadBase.x,
                chanHeadBase.y,
                chanHeadBase.z,
                (hovered && (!mobileUI || ImGui::IsMouseDown(ImGuiMouseButton_Left)))?0.3f:MIN(1.0f,0.2f+chanHeadBase.w*keyHit[i]*1.2f)
              ));
              ImVec2 rMin=rect.Min;
              ImVec2 rMax=rect.Max;
              rMin.x+=3.0f*dpiScale;
              rMin.y+=6.0f*dpiScale;
              rMax.x-=3.0f*dpiScale;
              rMax.y-=6.0f*dpiScale;
              dl->AddRectFilledMultiColor(rMin,rMax,fadeCol0,fadeCol0,fadeCol,fadeCol,4.0f*dpiScale);
              dl->AddText(ImVec2(minLabelArea.x,rect.Min.y+6.0*dpiScale),ImGui::GetColorU32(channelTextColor(i)),chanID);
            }
            break;
          }
          case 3: // split button
            ImGui::Dummy(ImVec2(1.0f,2.0f*dpiScale));
            //ImGui::SetCursorPosX(minLabelArea.x);
            ImGui::TextUnformatted(chanID);
            ImGui::SameLine();
            ImGui::PushFont(mainFont);
            ImGui::SmallButton(muted?ICON_FA_VOLUME_OFF:ICON_FA_VOLUME_UP);
            ImGui::PopFont();
            break;
          case 4: { // square border
            ImGui::ItemSize(size,ImGui::GetStyle().FramePadding.y);
            if (ImGui::ItemAdd(rect,ImGui::GetID(chanID))) {
              bool hovered=ImGui::ItemHoverable(rect,ImGui::GetID(chanID));
              ImU32 fadeCol=ImGui::GetColorU32(ImVec4(
                chanHeadBase.x,
                chanHeadBase.y,
                chanHeadBase.z,
                (hovered && (!mobileUI || ImGui::IsMouseDown(ImGuiMouseButton_Left)))?1.0f:MIN(1.0f,0.2f+chanHeadBase.w*keyHit[i]*4.0f)
              ));
              ImVec2 rMin=rect.Min;
              ImVec2 rMax=rect.Max;
              rMin.x+=2.0f*dpiScale;
              rMin.y+=3.0f*dpiScale;
              rMax.x-=3.0f*dpiScale;
              rMax.y-=3.0f*dpiScale;
              dl->AddRect(rMin,rMax,fadeCol,0.0f,2.0*dpiScale);
              dl->AddText(ImVec2(minLabelArea.x,rect.Min.y+3.0*dpiScale),ImGui::GetColorU32(channelTextColor(i)),chanID);
            }
            break;
          }
          case 5: { // round border
            ImGui::ItemSize(size,ImGui::GetStyle().FramePadding.y);
            if (ImGui::ItemAdd(rect,ImGui::GetID(chanID))) {
              bool hovered=ImGui::ItemHoverable(rect,ImGui::GetID(chanID));
              ImU32 fadeCol=ImGui::GetColorU32(ImVec4(
                chanHeadBase.x,
                chanHeadBase.y,
                chanHeadBase.z,
                (hovered && (!mobileUI || ImGui::IsMouseDown(ImGuiMouseButton_Left)))?1.0f:MIN(1.0f,0.2f+chanHeadBase.w*keyHit[i]*4.0f)
              ));
              ImVec2 rMin=rect.Min;
              ImVec2 rMax=rect.Max;
              rMin.x+=2.0f*dpiScale;
              rMin.y+=3.0f*dpiScale;
              rMax.x-=3.0f*dpiScale;
              rMax.y-=3.0f*dpiScale;
              dl->AddRect(rMin,rMax,fadeCol,4.0f*dpiScale,ImDrawFlags_RoundCornersAll,2.0*dpiScale);
              dl->AddText(ImVec2(minLabelArea.x,rect.Min.y+3.0*dpiScale),ImGui::GetColorU32(channelTextColor(i)),chanID);
            }
            break;
          }
        }
        ImGui::PopID();

        if (extraChannelButtons==0 || settings.channelVolStyle!=0) ImGui::PopStyleVar();

        if (displayTooltip && ImGui::IsItemHovered() && !mobileUI) {
          ImGui::SetTooltip("%s",e->getChannelName(i));
        }
        if (settings.channelFont==0) ImGui::PopFont();
        if (mobileUI) {
          if (ImGui::IsItemHovered()) {
            if (CHECK_LONG_HOLD) {
              NOTIFY_LONG_HOLD;
              e->toggleSolo(i);
              soloChan=i;
            }
            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !ImGui::WasInertialScroll()) {
              if (soloChan!=i) {
                e->toggleMute(i);
              } else {
                soloChan=-1;
              }
            }
          } 
        } else {
          if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            if (settings.soloAction!=1 && soloTimeout>0 && soloChan==i) {
              e->toggleSolo(i);
              soloTimeout=0;
            } else {
              e->toggleMute(i);
              soloTimeout=20;
              soloChan=i;
            }
          }
        }
        if (muted) ImGui::PopStyleColor();
        ImGui::PopStyleColor(4);
        if (settings.soloAction!=2 && !mobileUI) if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
          inhibitMenu=true;
          e->toggleSolo(i);
        }

        if (settings.channelStyle==3) {
          ImGui::Dummy(ImVec2(1.0f,2.0f*dpiScale));
        }

        // volume bar
        if (settings.channelVolStyle!=0) {
          ImVec2 sizeV=ImVec2(
            1.0f,
            6.0*dpiScale
          );
          ImVec2 minAreaV=window->DC.CursorPos;
          ImVec2 maxAreaV=ImVec2(
            minAreaV.x+window->WorkRect.Max.x-window->WorkRect.Min.x,
            minAreaV.y+sizeV.y
          );
          ImRect rectV=ImRect(minAreaV,maxAreaV);
          ImGui::ItemSize(sizeV,ImGui::GetStyle().FramePadding.y);
          if (ImGui::ItemAdd(rectV,ImGui::GetID(chanID))) {
            float xLeft=0.0f;
            float xRight=1.0f;

            if (e->keyHit[i]) {
              keyHit1[i]=1.0f;
              e->keyHit[i]=false;
            }

            if (e->isRunning()) {
              DivChannelState* cs=e->getChanState(i);
              float stereoPan=(float)(e->convertPanSplitToLinearLR(cs->panL,cs->panR,256)-128)/128.0;
              switch (settings.channelVolStyle) {
                case 1: // simple
                  xRight=((float)(e->getChanState(i)->volume>>8)/(float)e->getMaxVolumeChan(i))*0.9+(keyHit1[i]*0.1f);
                  break;
                case 2: { // stereo
                  float amount=((float)(e->getChanState(i)->volume>>8)/(float)e->getMaxVolumeChan(i))*0.4+(keyHit1[i]*0.1f);
                  xRight=0.5+amount*(1.0+MIN(0.0,stereoPan));
                  xLeft=0.5-amount*(1.0-MAX(0.0,stereoPan));
                  break;
                }
                case 3: // real
                  xRight=chanOscVol[i];
                  break;
                case 4: // real (stereo)
                  xRight=0.5+chanOscVol[i]*0.5*(1.0+MIN(0.0,stereoPan));
                  xLeft=0.5-chanOscVol[i]*0.5*(1.0-MAX(0.0,stereoPan));
                  break;
              }

              dl->AddRectFilled(
                ImLerp(rectV.Min,rectV.Max,ImVec2(xLeft,0.0f)),
                ImLerp(rectV.Min,rectV.Max,ImVec2(xRight,1.0f)),
                ImGui::GetColorU32(chanHeadBase)
              );
            }
            keyHit1[i]-=0.2f;
            if (keyHit1[i]<0.0f) keyHit1[i]=0.0f;
          }
        }
        
        // extra buttons
        if (extraChannelButtons==2) {
          DivPattern* pat=e->curPat[i].getPattern(e->curOrders->ord[i][ord],true);
          ImGui::PushFont(mainFont);
          if (patNameTarget==i) {
            snprintf(chanID,2048,"##PatNameI%d_%d",i,ord);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x-(8.0f*dpiScale));
            ImGui::SetCursorPosX(ImGui::GetCursorPosX()+4.0f*dpiScale);
            ImGui::InputText(chanID,&pat->name);
            if (wantPatName) {
              wantPatName=false;
              ImGui::SetItemDefaultFocus();
              ImGui::SetKeyboardFocusHere(-1);
            } else {
              if (!ImGui::IsItemActive()) {
                patNameTarget=-1;
              }
            }
          } else {
            snprintf(chanID,2048," %s##PatName%d",pat->name.c_str(),i);
            if (ImGui::Selectable(chanID,true,ImGuiSelectableFlags_NoPadWithHalfSpacing,ImVec2(0.0f,lineHeight+1.0f*dpiScale))) {
              patNameTarget=i;
              wantPatName=true;
              snprintf(chanID,2048,"##PatNameI%d_%d",i,ord);
              ImGui::SetActiveID(ImGui::GetID(chanID),ImGui::GetCurrentWindow());
            }
          }
          ImGui::PopFont();
        } else if (extraChannelButtons==1) {
          snprintf(chanID,2048,"%c##_HCH%d",e->curSubSong->chanCollapse[i]?'+':'-',i);
          ImGui::SetCursorPosX(ImGui::GetCursorPosX()+4.0f*dpiScale);
          if (ImGui::SmallButton(chanID)) {
            if (e->curSubSong->chanCollapse[i]==0) {
              e->curSubSong->chanCollapse[i]=3;
            } else if (e->curSubSong->chanCollapse[i]>0) {
              e->curSubSong->chanCollapse[i]--;
            }
          }
          if (!e->curSubSong->chanCollapse[i]) {
            ImGui::SameLine();
            snprintf(chanID,2048,"<##_LCH%d",i);
            ImGui::BeginDisabled(e->curPat[i].effectCols<=1);
            if (ImGui::SmallButton(chanID)) {
              e->curPat[i].effectCols--;
              if (e->curPat[i].effectCols<1) e->curPat[i].effectCols=1;
            }
            ImGui::EndDisabled();
            ImGui::SameLine();
            ImGui::BeginDisabled(e->curPat[i].effectCols>=DIV_MAX_EFFECTS);
            snprintf(chanID,2048,">##_RCH%d",i);
            if (ImGui::SmallButton(chanID)) {
              e->curPat[i].effectCols++;
              if (e->curPat[i].effectCols>DIV_MAX_EFFECTS) e->curPat[i].effectCols=DIV_MAX_EFFECTS;
            }
            ImGui::EndDisabled();
          }
          ImGui::Spacing();
        }
      }
      ImGui::TableNextColumn();
      lastPatternWidth=ImGui::GetCursorPosX()-lpwStart+ImGui::GetStyle().ScrollbarSize;
      if (e->hasExtValue()) {
        ImGui::TextColored(uiColors[GUI_COLOR_EE_VALUE]," %.2X",e->getExtValue());
      }
      float oneCharSize=ImGui::CalcTextSize("A").x;
      fourChars=ImVec2(oneCharSize*4.0f,lineHeight);
      threeChars=ImVec2(oneCharSize*3.0f,lineHeight);
      twoChars=ImVec2(oneCharSize*2.0f,lineHeight);
      //ImVec2 oneChar=ImVec2(oneCharSize,lineHeight);

      noteCellSize=threeChars;
      noteCellSize.x+=(float)settings.noteCellSpacing*dpiScale;
      insCellSize=twoChars;
      insCellSize.x+=(float)settings.insCellSpacing*dpiScale;
      volCellSize=twoChars;
      volCellSize.x+=(float)settings.volCellSpacing*dpiScale;
      effectCellSize=twoChars;
      effectCellSize.x+=(float)settings.effectCellSpacing*dpiScale;
      effectValCellSize=twoChars;
      effectValCellSize.x+=(float)settings.effectValCellSpacing*dpiScale;

      dummyRows=(ImGui::GetWindowSize().y/lineHeight)/2;

      // オップナー2608 i owe you one more for this horrible code
      // previous pattern
      ImGui::BeginDisabled();
      ImGui::PushStyleVar(ImGuiStyleVar_FrameShading,0.0f);
      if (settings.viewPrevPattern) {
        if ((ord-1)>=0) for (int i=0; i<chans; i++) {
          patCache[i]=e->curPat[i].getPattern(e->curOrders->ord[i][ord-1],true);
        }
        for (int i=0; i<dummyRows-1; i++) {
          patternRow(e->curSubSong->patLen+i-dummyRows+1,e->isPlaying(),lineHeight,chans,ord-1,patCache,true);
        }
      } else {
        for (int i=0; i<dummyRows-1; i++) {
          ImGui::TableNextRow(0,lineHeight);
          ImGui::TableNextColumn();
        }
      }
      ImGui::EndDisabled();
      // active area
      for (int i=0; i<chans; i++) {
        patCache[i]=e->curPat[i].getPattern(e->curOrders->ord[i][ord],true);
      }
      for (int i=0; i<e->curSubSong->patLen; i++) {
        patternRow(i,e->isPlaying(),lineHeight,chans,ord,patCache,false);
      }
      // next pattern
      ImGui::BeginDisabled();
      if (settings.viewPrevPattern) {
        if ((ord+1)<e->curSubSong->ordersLen) for (int i=0; i<chans; i++) {
          patCache[i]=e->curPat[i].getPattern(e->curOrders->ord[i][ord+1],true);
        }
        for (int i=0; i<=dummyRows; i++) {
          patternRow(i,e->isPlaying(),lineHeight,chans,ord+1,patCache,true);
        }
      } else {
        for (int i=0; i<=dummyRows; i++) {
          ImGui::TableNextRow(0,lineHeight);
          ImGui::TableNextColumn();
        }
      }
      ImGui::EndDisabled();
      ImGui::PopStyleVar();
      oldRow=curRow;
      if (demandScrollX) {
        int totalDemand=demandX-ImGui::GetScrollX();
        if (totalDemand<80) {
          ImGui::SetScrollX(demandX-200*dpiScale);
        } else if (totalDemand>(ImGui::GetWindowWidth()-200*dpiScale)) {
          ImGui::SetScrollX(demandX+200*dpiScale);
        }
        demandScrollX=false;
      }
      scrollX=ImGui::GetScrollX();

      // overflow changes order
      // TODO: this is very unreliable and sometimes it can warp you out of the song
      if (settings.scrollChangesOrder && !e->isPlaying()) {
        if (wheelY!=0) {
          if (wheelY>0) {
            if (ImGui::GetScrollY()<=0) {
              if (haveHitBounds) {
                if (curOrder>0) {
                  setOrder(curOrder-1);
                  ImGui::SetScrollY(ImGui::GetScrollMaxY());
                  updateScroll(e->curSubSong->patLen);
                }
                haveHitBounds=false;
              } else {
                haveHitBounds=true;
              }
            } else {
              haveHitBounds=false;
            }
          } else {
            if (ImGui::GetScrollY()>=ImGui::GetScrollMaxY()) {
              if (haveHitBounds) {
                if (curOrder<(e->curSubSong->ordersLen-1)) {
                  setOrder(curOrder+1);
                  ImGui::SetScrollY(0);
                  updateScroll(0);
                }
                haveHitBounds=false;
              } else {
                haveHitBounds=true;
              }
            } else {
              haveHitBounds=false;
            }
          }
        }
      }
      ImGui::EndTable();
    }

    if (fancyPattern) { // visualizer
      e->getCommandStream(cmdStream);
      ImDrawList* dl=ImGui::GetWindowDrawList();
      ImVec2 off=ImVec2(0.0f,ImGui::GetWindowPos().y);
      
      // commands
      for (DivCommand& i: cmdStream) {
        if (i.cmd==DIV_CMD_PITCH) continue;
        if (i.cmd==DIV_CMD_NOTE_PORTA) continue;
        //if (i.cmd==DIV_CMD_NOTE_ON) continue;
        if (i.cmd==DIV_CMD_PRE_PORTA) continue;
        if (i.cmd==DIV_CMD_PRE_NOTE) continue;
        if (i.cmd==DIV_CMD_SAMPLE_BANK) continue;
        if (i.cmd==DIV_CMD_GET_VOLUME) continue;
        if (i.cmd==DIV_ALWAYS_SET_VOLUME) continue;
        if (i.cmd==DIV_CMD_HINT_VOLUME ||
            i.cmd==DIV_CMD_HINT_PORTA ||
            i.cmd==DIV_CMD_HINT_LEGATO ||
            i.cmd==DIV_CMD_HINT_VOL_SLIDE ||
            i.cmd==DIV_CMD_HINT_ARPEGGIO ||
            i.cmd==DIV_CMD_HINT_PITCH ||
            i.cmd==DIV_CMD_HINT_VIBRATO ||
            i.cmd==DIV_CMD_HINT_VIBRATO_RANGE ||
            i.cmd==DIV_CMD_HINT_VIBRATO_SHAPE) continue;

        float width=patChanX[i.chan+1]-patChanX[i.chan];
        float speedX=0.0f;
        float speedY=-18.0f;
        float grav=0.6f;
        float frict=1.0f;
        float life=255.0f;
        float lifeSpeed=8.0f;
        float spread=5.0f;
        int num=3;
        const char* partIcon=ICON_FA_MICROCHIP;
        ImU32* color=noteGrad;

        switch (i.cmd) {
          case DIV_CMD_NOTE_ON: {
            float strength=CLAMP(i.value,0,119);
            partIcon=ICON_FA_ASTERISK;
            life=80.0f+((i.value==DIV_NOTE_NULL)?0.0f:(strength*0.3f));
            lifeSpeed=3.0f;
            num=6+(strength/16);
            break;
          }
          case DIV_CMD_LEGATO:
            partIcon=ICON_FA_COG;
            color=insGrad;
            life=64.0f;
            lifeSpeed=2.0f;
            break;
          case DIV_CMD_NOTE_OFF:
          case DIV_CMD_NOTE_OFF_ENV:
          case DIV_CMD_ENV_RELEASE:
            partIcon=ICON_FA_ASTERISK;
            speedX=0.0f;
            speedY=0.0f;
            grav=0.0f;
            life=24.0f;
            lifeSpeed=4.0f;
            break;
          case DIV_CMD_VOLUME: {
            float scaledVol=(float)i.value/(float)e->getMaxVolumeChan(i.chan);
            if (scaledVol>1.0f) scaledVol=1.0f;
            speedY=-18.0f-(10.0f*scaledVol);
            life=128+scaledVol*127;
            partIcon=ICON_FA_VOLUME_UP;
            num=12.0f*pow(scaledVol,2.0);
            color=volGrad;
            break;
          }
          case DIV_CMD_INSTRUMENT: {
            if (lastIns[i.chan]==i.value) {
              num=0;
              break;
            }
            lastIns[i.chan]=i.value;
            speedX=0.0f;
            speedY=0.0f;
            grav=0.0f;
            frict=0.98;
            spread=30.0f;
            life=128.0f;
            lifeSpeed=6.0f;
            color=insGrad;
            num=7+pow(i.value,0.6);
            break;
          }
          case DIV_CMD_PANNING: {
            float ratio=(float)(128-e->convertPanSplitToLinearLR(i.value,i.value2,256))/128.0f;
            logV("ratio %f",ratio);
            speedX=-22.0f*sin(ratio*M_PI*0.5);
            speedY=-22.0f*cos(ratio*M_PI*0.5);
            spread=5.0f+fabs(sin(ratio*M_PI*0.5))*7.0f;
            grav=0.0f;
            frict=0.96f;
            if (i.value==i.value2) {
              partIcon=ICON_FA_ARROWS_H;
            } else if (ratio>0) {
              partIcon=ICON_FA_ARROW_LEFT;
            } else {
              partIcon=ICON_FA_ARROW_RIGHT;
            }
            num=9;
            color=panGrad;
            break;
          }
          case DIV_CMD_SAMPLE_FREQ:
            speedX=0.0f;
            speedY=0.0f;
            grav=0.0f;
            frict=0.98;
            spread=19.0f;
            life=128.0f;
            lifeSpeed=3.0f;
            color=sysCmd2Grad;
            num=10+pow(i.value,0.6);
            break;
          default:
            //printf("unhandled %d\n",i.cmd);
            color=sysCmd1Grad;
            break;
        }

        for (int j=0; j<num; j++) {
          particles.push_back(Particle(
            color,
            partIcon,
            off.x+patChanX[i.chan]+fmod(rand(),width)-scrollX,
            off.y+(ImGui::GetWindowHeight()*0.5f)+randRange(0,patFont->FontSize),
            (speedX+randRange(-spread,spread))*0.5*dpiScale,
            (speedY+randRange(-spread,spread))*0.5*dpiScale,
            grav,
            frict,
            life-randRange(0,8),
            lifeSpeed
          ));
        }
      }

      float frameTime=ImGui::GetIO().DeltaTime*60.0f;

      // note slides and vibrato
      ImVec2 arrowPoints[7];
      if (e->isPlaying()) for (int i=0; i<chans; i++) {
        if (!e->curSubSong->chanShow[i]) continue;
        DivChannelState* ch=e->getChanState(i);
        if (ch->portaSpeed>0) {
          ImVec4 col=uiColors[GUI_COLOR_PATTERN_EFFECT_PITCH];
          col.w*=0.2;
          float width=patChanX[i+1]-patChanX[i];

          particles.push_back(Particle(
            pitchGrad,
            (ch->portaNote<=ch->note)?ICON_FA_CHEVRON_DOWN:ICON_FA_CHEVRON_UP,
            off.x+patChanX[i]+fmod(rand(),width)-scrollX,
            off.y+fmod(rand(),MAX(1,ImGui::GetWindowHeight())),
            0.0f,
            (7.0f+(rand()%5)+pow(ch->portaSpeed,0.7f))*((ch->portaNote<=ch->note)?1:-1),
            0.0f,
            1.0f,
            255.0f,
            15.0f
          ));

          if (width>0.1) for (float j=-patChanSlideY[i]; j<ImGui::GetWindowHeight(); j+=width*0.7) {
            ImVec2 tMin=ImVec2(off.x+patChanX[i]-scrollX,off.y+j);
            ImVec2 tMax=ImVec2(off.x+patChanX[i+1]-scrollX,off.y+j+width*0.6);
            if (ch->portaNote<=ch->note) {
              arrowPoints[0]=ImLerp(tMin,tMax,ImVec2(0.1,1.0-0.8));
              arrowPoints[1]=ImLerp(tMin,tMax,ImVec2(0.5,1.0-0.0));
              arrowPoints[2]=ImLerp(tMin,tMax,ImVec2(0.9,1.0-0.8));
              arrowPoints[3]=ImLerp(tMin,tMax,ImVec2(0.8,1.0-1.0));
              arrowPoints[4]=ImLerp(tMin,tMax,ImVec2(0.5,1.0-0.37));
              arrowPoints[5]=ImLerp(tMin,tMax,ImVec2(0.2,1.0-1.0));
              arrowPoints[6]=arrowPoints[0];
              dl->AddPolyline(arrowPoints,7,ImGui::GetColorU32(col),ImDrawFlags_None,5.0f*dpiScale);
            } else {
              arrowPoints[0]=ImLerp(tMin,tMax,ImVec2(0.1,0.8));
              arrowPoints[1]=ImLerp(tMin,tMax,ImVec2(0.5,0.0));
              arrowPoints[2]=ImLerp(tMin,tMax,ImVec2(0.9,0.8));
              arrowPoints[3]=ImLerp(tMin,tMax,ImVec2(0.8,1.0));
              arrowPoints[4]=ImLerp(tMin,tMax,ImVec2(0.5,0.37));
              arrowPoints[5]=ImLerp(tMin,tMax,ImVec2(0.2,1.0));
              arrowPoints[6]=arrowPoints[0];
              dl->AddPolyline(arrowPoints,7,ImGui::GetColorU32(col),ImDrawFlags_None,5.0f*dpiScale);
            }
          }
          patChanSlideY[i]+=((ch->portaNote<=ch->note)?-8:8)*dpiScale*frameTime;
          if (width>0) {
            if (patChanSlideY[i]<0) {
              patChanSlideY[i]=-fmod(-patChanSlideY[i],width*0.7);
            } else {
              patChanSlideY[i]=fmod(patChanSlideY[i],width*0.7);
            }
          }
        }
        if (ch->vibratoDepth>0) {
          ImVec4 col=uiColors[GUI_COLOR_PATTERN_EFFECT_PITCH];
          col.w*=0.2;
          float width=patChanX[i+1]-patChanX[i];

          particles.push_back(Particle(
            pitchGrad,
            ICON_FA_GLASS,
            off.x+patChanX[i]+(width*0.5+0.5*sin(M_PI*(float)ch->vibratoPosGiant/64.0f)*width)-scrollX,
            off.y+(ImGui::GetWindowHeight()*0.5f)+randRange(0,patFont->FontSize),
            randRange(-4.0f,4.0f),
            2.0f*(3.0f+(rand()%5)+ch->vibratoRate),
            0.4f,
            1.0f,
            128.0f,
            4.0f
          ));
        }
      }

      // particle simulation
      ImDrawList* fdl=ImGui::GetForegroundDrawList();
      if (!particles.empty()) WAKE_UP;
      fdl->AddCallback(_pushPartBlend,this);
      for (size_t i=0; i<particles.size(); i++) {
        Particle& part=particles[i];
        if (part.update(frameTime)) {
          if (part.life>255) part.life=255;
          fdl->AddText(
            iconFont,
            iconFont->FontSize,
            ImVec2(part.pos.x-iconFont->FontSize*0.5,part.pos.y-iconFont->FontSize*0.5),
            part.colors[(int)part.life],
            part.type
          );
        } else {
          particles.erase(particles.begin()+i);
          i--;
        }
      }
      fdl->AddCallback(_popPartBlend,this);
    }

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();
    ImGui::PopFont();
  }
  ImGui::PopStyleVar();
  if (patternOpen) {
    if (!inhibitMenu && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) ImGui::OpenPopup("patternActionMenu");
    if (ImGui::BeginPopup("patternActionMenu",ImGuiWindowFlags_NoMove|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoSavedSettings)) {
      editOptions(false);
      ImGui::EndPopup();
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_PATTERN;
  ImGui::End();
  //int delta1=SDL_GetPerformanceCounter();
  //logV("render time: %dµs",(delta1-delta0)/(SDL_GetPerformanceFrequency()/1000000));
}

