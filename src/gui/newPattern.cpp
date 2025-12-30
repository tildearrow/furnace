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

// for suck's fake Clang extension!
#define _USE_MATH_DEFINES
#include "gui.h"
#include "../ta-log.h"
#include "imgui_internal.h"
#include "IconsFontAwesome4.h"
#include "furIcons.h"
#include "misc/cpp/imgui_stdlib.h"
#include "guiConst.h"
#include "../utfutils.h"
#include <fmt/printf.h>

#define SETUP_ORDER_ALPHA \
  if (ord==curOrder) { \
    ImGui::GetStyle().Alpha=origAlpha; \
  } else { \
    ImGui::GetStyle().Alpha=disabledAlpha; \
  } \
  activeColor=ImGui::GetColorU32(uiColors[GUI_COLOR_PATTERN_ACTIVE]); \
  inactiveColor=ImGui::GetColorU32(uiColors[GUI_COLOR_PATTERN_INACTIVE]); \
  rowIndexColor=ImGui::GetColorU32(uiColors[GUI_COLOR_PATTERN_ROW_INDEX]);

// this is ImGui's TABLE_BORDER_SIZE.
#define PAT_BORDER_SIZE 1.0f

void FurnaceGUI::drawPatternNew() {
  if (nextWindow==GUI_WINDOW_PATTERN) {
    patternOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!patternOpen) return;

  if (e->isPlaying() && followPattern) {
    if (oldRowChanged || !e->isStepping()) {
      if (e->isStepping()) pendingStepUpdate=1;
      cursor.y=oldRow;
      cursor.order=curOrder;
      if (selStart.xCoarse==selEnd.xCoarse && selStart.xFine==selEnd.xFine && selStart.y==selEnd.y && selStart.order==selEnd.order && !selecting) {
        selStart=cursor;
        selEnd=cursor;
      }
    }
  }
  sel1=selStart;
  sel2=selEnd;
  if (sel2.order<sel1.order) {
    sel2.order^=sel1.order;
    sel1.order^=sel2.order;
    sel2.order^=sel1.order;
    sel2.y^=sel1.y;
    sel1.y^=sel2.y;
    sel2.y^=sel1.y;
  } else if (sel2.order==sel1.order && sel2.y<sel1.y) {
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

  //ImVec2 origWinPadding=ImGui::GetStyle().WindowPadding;
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,ImVec2(0.0f,0.0f));
  if (mobileUI) {
    patWindowPos=(portrait?ImVec2(0.0f,(mobileMenuPos*-0.65*canvasH)+(0.12*canvasW)):ImVec2((0.16*canvasH)+0.5*canvasW*mobileMenuPos,0.0f));
    patWindowSize=(portrait?ImVec2(canvasW,canvasH-(0.16*canvasW)-(0.12*canvasW)-(pianoOpen?(0.4*canvasW):0.0f)):ImVec2(canvasW-(0.16*canvasH),canvasH-(pianoOpen?(0.3*canvasH):0.0f)));
    ImGui::SetNextWindowPos(patWindowPos);
    ImGui::SetNextWindowSize(patWindowSize);
  }
  if (ImGui::Begin("PatternNew",&patternOpen,globalWinFlags|ImGuiWindowFlags_HorizontalScrollbar|(settings.avoidRaisingPattern?ImGuiWindowFlags_NoBringToFrontOnFocus:0)|((settings.cursorFollowsWheel && !selecting)?ImGuiWindowFlags_NoScrollWithMouse:0),_("Pattern"))) {
    if (!mobileUI) {
      patWindowPos=ImGui::GetWindowPos();
      patWindowSize=ImGui::GetWindowSize();
    }

    ImDrawList* dl=ImGui::GetWindowDrawList();
    char id[64];
    int firstOrd=curOrder;
    int chans=e->getTotalChannelCount();
    /*
    int displayChans=0;
    for (int i=0; i<chans; i++) {
      if (e->curSubSong->chanShow[i]) displayChans++;
    }*/

    ImGui::PushFont(patFont);
    float lineHeight=(ImGui::GetTextLineHeight()+2*dpiScale);
    dummyRows=(ImGui::GetWindowSize().y/lineHeight)/2;
    int totalRows=e->curSubSong->patLen+dummyRows*2;
    int firstRow=-dummyRows;
    while (firstRow<0) {
      firstRow+=e->curSubSong->patLen;
      firstOrd--;
    }

    // this could be moved somewhere else for performance...
    float oneCharSize=ImGui::CalcTextSize("A").x;
    fourChars=ImVec2(oneCharSize*4.0f,lineHeight);
    threeChars=ImVec2(oneCharSize*3.0f,lineHeight);
    twoChars=ImVec2(oneCharSize*2.0f,lineHeight);
    oneChar=ImVec2(oneCharSize,lineHeight);

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

    ImVec2 top=ImGui::GetCursorScreenPos();
    ImVec2 pos=top;

    ImVec2 size=ImVec2(0.0f,lineHeight*totalRows);

    size.x+=threeChars.x+oneChar.x;
    size.x+=PAT_BORDER_SIZE;

    // TODO: simplify
    for (int i=0; i<chans; i++) {
      patChanX[i]=size.x;
      if (!e->curSubSong->chanShow[i]) {
        continue;
      }
      int chanVolMax=e->getMaxVolumeChan(i);
      if (chanVolMax<1) chanVolMax=1;

      float thisChannelSize=noteCellSize.x;
      if (e->curSubSong->chanCollapse[i]<3) thisChannelSize+=insCellSize.x;
      if (e->curSubSong->chanCollapse[i]<2) thisChannelSize+=volCellSize.x;
      if (e->curSubSong->chanCollapse[i]<1) thisChannelSize+=(effectCellSize.x+effectValCellSize.x)*e->curSubSong->pat[i].effectCols;

      size.x+=thisChannelSize;
      size.x+=PAT_BORDER_SIZE;
    }
    patChanX[chans]=size.x;

    size.x+=oneChar.x;

    ImVec2 minArea=top;
    ImVec2 maxArea=ImVec2(
      minArea.x+size.x,
      minArea.y+size.y
    );
    ImRect rect=ImRect(minArea,maxArea);

    // create the view
    ImGui::ItemSize(size,ImGui::GetStyle().FramePadding.y);
    if (ImGui::ItemAdd(rect,ImGui::GetID("PatternView"))) {
      ImU32 activeColor=ImGui::GetColorU32(uiColors[GUI_COLOR_PATTERN_ACTIVE]);
      ImU32 inactiveColor=ImGui::GetColorU32(uiColors[GUI_COLOR_PATTERN_INACTIVE]);
      ImU32 rowIndexColor=ImGui::GetColorU32(uiColors[GUI_COLOR_PATTERN_ROW_INDEX]);
      float origAlpha=ImGui::GetStyle().Alpha;
      float disabledAlpha=ImGui::GetStyle().Alpha*ImGui::GetStyle().DisabledAlpha;

      // row number and highlights
      {
        int ord=firstOrd;
        int row=firstRow;
        bool isPlaying=e->isPlaying();
        SETUP_ORDER_ALPHA;
        for (int j=0; j<totalRows; j++) {
          if (ord>=0 && ord<e->curSubSong->ordersLen) {
            snprintf(id,63,"%3d",row);
            dl->AddText(pos,rowIndexColor,id);

            ImU32 thisRowBg=0;
            if (edit && cursor.y==row && cursor.order==ord && curWindowLast==GUI_WINDOW_PATTERN) {
              if (editClone && !isPatUnique && secondTimer<0.5) {
                thisRowBg=ImGui::GetColorU32(uiColors[GUI_COLOR_EDITING_CLONE]);
              } else {
                thisRowBg=ImGui::GetColorU32(uiColors[GUI_COLOR_EDITING]);
              }
            } else if (isPlaying && oldRow==row && ord==playOrder) {
              thisRowBg=ImGui::GetColorU32(uiColors[GUI_COLOR_PATTERN_PLAY_HEAD]);
            } else if (e->curSubSong->hilightB>0 && !(row%e->curSubSong->hilightB)) {
              thisRowBg=ImGui::GetColorU32(uiColors[GUI_COLOR_PATTERN_HI_2]);
            } else if (e->curSubSong->hilightA>0 && !(row%e->curSubSong->hilightA)) {
              thisRowBg=ImGui::GetColorU32(uiColors[GUI_COLOR_PATTERN_HI_1]);
            }

            if (thisRowBg) {
              dl->AddRectFilled(
                ImVec2(top.x+patChanX[0],pos.y),
                ImVec2(top.x+patChanX[chans],pos.y+lineHeight),
                thisRowBg
              );
            }
          }
          if (++row>=e->curSubSong->patLen) {
            row=0;
            ord++;
            SETUP_ORDER_ALPHA;
          }
          pos.y+=lineHeight;
        }
      }

      // channels
      for (int i=0; i<chans; i++) {
        if (!e->curSubSong->chanShow[i]) continue;

        ImVec2 thisTop=ImVec2(top.x+patChanX[i],top.y);
        pos=thisTop;

        dl->AddLine(
          ImVec2(thisTop.x-PAT_BORDER_SIZE*0.5,thisTop.y),
          ImVec2(thisTop.x-PAT_BORDER_SIZE*0.5,maxArea.y),
          ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_TableBorderLight]),
          PAT_BORDER_SIZE
        );

        int ord=firstOrd;
        int row=firstRow;
        int chanVolMax=e->getMaxVolumeChan(i);
        if (chanVolMax<1) chanVolMax=1;

        const DivPattern* pat=NULL;
        if (ord>=0 && ord<e->curSubSong->ordersLen) {
          pat=e->curSubSong->pat[i].getPattern(e->curOrders->ord[i][ord],true);
        }

        SETUP_ORDER_ALPHA;

        // rows
        for (int j=0; j<totalRows; j++) {
          if (pat) {
            // note
            snprintf(id,63,"%.31s",noteName(pat->newData[row][DIV_PAT_NOTE]));
            if (pat->newData[row][DIV_PAT_NOTE]==-1) {
              dl->AddText(pos,inactiveColor,id);
            } else {
              dl->AddText(pos,activeColor,id);
            }

            // instrument
            if (e->curSubSong->chanCollapse[i]<3) {
              pos.x+=threeChars.x;
              if (pat->newData[row][DIV_PAT_INS]==-1) {
                dl->AddText(pos,inactiveColor,emptyLabel2);
              } else {
                snprintf(id,63,"%.2X",pat->newData[row][DIV_PAT_INS]);
                if (pat->newData[row][DIV_PAT_INS]<0 || pat->newData[row][DIV_PAT_INS]>=e->song.insLen) {
                  dl->AddText(pos,ImGui::GetColorU32(uiColors[GUI_COLOR_PATTERN_INS_ERROR]),id);
                } else {
                  DivInstrumentType t=e->song.ins[pat->newData[row][DIV_PAT_INS]]->type;
                  if (t!=DIV_INS_AMIGA && t!=e->getPreferInsType(i)) {
                    dl->AddText(pos,ImGui::GetColorU32(uiColors[GUI_COLOR_PATTERN_INS_WARN]),id);
                  } else {
                    dl->AddText(pos,ImGui::GetColorU32(uiColors[GUI_COLOR_PATTERN_INS]),id);
                  }
                }
              }
            }

            // volume
            if (e->curSubSong->chanCollapse[i]<2) {
              pos.x+=twoChars.x;
              if (pat->newData[row][DIV_PAT_VOL]==-1) {
                dl->AddText(pos,inactiveColor,emptyLabel2);
              } else {
                int volColor=(pat->newData[row][DIV_PAT_VOL]*127)/chanVolMax;
                if (volColor>127) volColor=127;
                if (volColor<0) volColor=0;
                snprintf(id,63,"%.2X",pat->newData[row][DIV_PAT_VOL]);
                dl->AddText(pos,ImGui::GetColorU32(volColors[volColor]),id);
              }
            }

            // effects
            if (e->curSubSong->chanCollapse[i]<1) {
              for (int k=0; k<e->curPat[i].effectCols; k++) {
                int index=DIV_PAT_FX(k);
                int indexVal=DIV_PAT_FXVAL(k);
                ImU32 effectColor=inactiveColor;

                // effect
                pos.x+=twoChars.x;
                if (pat->newData[row][index]==-1) {
                  dl->AddText(pos,inactiveColor,emptyLabel2);
                } else {
                  if (pat->newData[row][index]>0xff) {
                    snprintf(id,63,"??");
                    effectColor=ImGui::GetColorU32(uiColors[GUI_COLOR_PATTERN_EFFECT_INVALID]);
                  } else {
                    const unsigned char data=pat->newData[row][index];
                    effectColor=ImGui::GetColorU32(uiColors[fxColors[data]]);
                    if (pat->newData[row][index]>=0x10 || settings.oneDigitEffects==0) {
                      snprintf(id,63,"%.2X",data);
                    } else {
                      snprintf(id,63," %.1X",data);
                    }
                  }
                  dl->AddText(pos,effectColor,id);
                }

                // effect value
                pos.x+=twoChars.x;
                if (pat->newData[row][indexVal]==-1) {
                  dl->AddText(pos,effectColor,emptyLabel2);
                } else {
                  snprintf(id,63,"%.2X",pat->newData[row][indexVal]);
                  dl->AddText(pos,effectColor,id);
                }
              }
            }
          }

          // go to next row
          if (++row>=e->curSubSong->patLen) {
            row=0;
            ord++;
            if (ord>=0 && ord<e->curSubSong->ordersLen) {
              pat=e->curSubSong->pat[i].getPattern(e->curOrders->ord[i][ord],true);
            } else {
              pat=NULL;
            }
            SETUP_ORDER_ALPHA;
          }
          pos.x=thisTop.x;
          pos.y+=lineHeight;
        }
      }

      dl->AddLine(
        ImVec2(top.x+patChanX[chans]-PAT_BORDER_SIZE*0.5,top.y),
        ImVec2(top.x+patChanX[chans]-PAT_BORDER_SIZE*0.5,maxArea.y),
        ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_TableBorderLight]),
        PAT_BORDER_SIZE
      );

      ImGui::GetStyle().Alpha=origAlpha;
    }
    ImGui::PopFont();
  }
  ImGui::PopStyleVar();
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_PATTERN;
  ImGui::End();
}

