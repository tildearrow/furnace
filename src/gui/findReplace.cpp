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

#include "gui.h"
#include "imgui.h"
#include "IconsFontAwesome4.h"
#include "misc/cpp/imgui_stdlib.h"
#include "guiConst.h"
#include "intConst.h"
#include "../ta-log.h"

const char* queryModes[GUI_QUERY_MAX]={
  "ignore",
  "equals",
  "not equal",
  "between",
  "not between",
  "any",
  "none"
};

const char* queryReplaceModes[GUI_QUERY_REPLACE_MAX]={
  "set",
  "add",
  "add (overflow)",
  "scale",
  "clear"
};

int queryNote(int note, int octave) {
  if (note==100) {
    return 128;
  } else if (note==101) { // note off and envelope release
    return 129;
  } else if (note==102) { // envelope release only
    return 130;
  } else if (octave==0 && note==0) {
    return -61;
  } else if (note==0 && octave!=0) {
    return -61; // bug note?
  }
  int seek=(note+(signed char)octave*12);
  if (seek<-60 || seek>=120) {
    return -61; // out of range note
  }
  return seek;
}

bool checkCondition(int mode, int arg, int argMax, int val, bool noteMode=false) {
  const int emptyVal=noteMode?-61:-1;
  switch (mode) {
    case GUI_QUERY_IGNORE:
      return true;
      break;
    case GUI_QUERY_MATCH:
      return (val==arg);
      break;
    case GUI_QUERY_MATCH_NOT:
      return (val!=emptyVal && val!=arg);
      break;
    case GUI_QUERY_RANGE:
      return (val>=arg && val<=argMax);
      break;
    case GUI_QUERY_RANGE_NOT:
      return (val!=emptyVal && (val<arg || val>argMax) && (!noteMode || val<120));
      break;
    case GUI_QUERY_ANY:
      return (val!=emptyVal);
      break;
    case GUI_QUERY_NONE:
      return (val==emptyVal);
      break;
  }
  return false;
}

void FurnaceGUI::doFind() {
  int firstOrder=0;
  int lastOrder=e->curSubSong->ordersLen-1;

  if (curQueryRangeY==1 || curQueryRangeY==2) {
    firstOrder=curOrder;
    lastOrder=curOrder;
  }

  int firstRow=0;
  int lastRow=e->curSubSong->patLen-1;

  if (curQueryRangeY==1) {
    firstRow=selStart.y;
    lastRow=selEnd.y;
  }

  int firstChan=0;
  int lastChan=e->getTotalChannelCount()-1;

  if (curQueryRangeX) {
    firstChan=curQueryRangeXMin;
    lastChan=curQueryRangeXMax;
  }

  curQueryResults.clear();

  for (int i=firstOrder; i<=lastOrder; i++) {
    for (int j=firstRow; j<=lastRow; j++) {
      for (int k=firstChan; k<=lastChan; k++) {
        DivPattern* p=e->curPat[k].getPattern(e->curOrders->ord[k][i],false);
        bool matched=false;
        for (FurnaceGUIFindQuery& l: curQuery) {
          if (matched) break;

          if (!checkCondition(l.noteMode,l.note,l.noteMax,queryNote(p->data[j][0],p->data[j][1]),true)) continue;
          if (!checkCondition(l.insMode,l.ins,l.insMax,p->data[j][2])) continue;
          if (!checkCondition(l.volMode,l.vol,l.volMax,p->data[j][3])) continue;

          if (l.effectCount>0) {
            bool notMatched=false;
            switch (curQueryEffectPos) {
              case 0: // no
                for (int m=0; m<l.effectCount; m++) {
                  bool allGood=false;
                  for (int n=0; n<e->curPat[k].effectCols; n++) {
                    if (!checkCondition(l.effectMode[m],l.effect[m],l.effectMax[m],p->data[j][4+n*2])) continue;
                    if (!checkCondition(l.effectValMode[m],l.effectVal[m],l.effectValMax[m],p->data[j][5+n*2])) continue;
                    allGood=true;
                    break;
                  }
                  if (!allGood) {
                    notMatched=true;
                    break;
                  }
                }
                break;
              case 1: { // lax
                // locate first effect
                int posOfFirst=-1;
                for (int m=0; m<e->curPat[k].effectCols; m++) {
                  if (!checkCondition(l.effectMode[0],l.effect[0],l.effectMax[0],p->data[j][4+m*2])) continue;
                  if (!checkCondition(l.effectValMode[0],l.effectVal[0],l.effectValMax[0],p->data[j][5+m*2])) continue;
                  posOfFirst=m;
                  break;
                }
                if (posOfFirst<0) {
                  notMatched=true;
                  break;
                }
                // make sure we aren't too far to the right
                if ((posOfFirst+l.effectCount)>e->curPat[k].effectCols) {
                  notMatched=true;
                  break;
                }
                // search from first effect location
                for (int m=0; m<l.effectCount; m++) {
                  if (!checkCondition(l.effectMode[m],l.effect[m],l.effectMax[m],p->data[j][4+(m+posOfFirst)*2])) {
                    notMatched=true;
                    break;
                  }
                  if (!checkCondition(l.effectValMode[m],l.effectVal[m],l.effectValMax[m],p->data[j][5+(m+posOfFirst)*2])) {
                    notMatched=true;
                    break;
                  }
                }
                break;
              }
              case 2: // strict
                int effectMax=l.effectCount;
                if (effectMax>e->curPat[k].effectCols) {
                  notMatched=true;
                } else {
                  for (int m=0; m<effectMax; m++) {
                    if (!checkCondition(l.effectMode[m],l.effect[m],l.effectMax[m],p->data[j][4+m*2])) {
                      notMatched=true;
                      break;
                    }
                    if (!checkCondition(l.effectValMode[m],l.effectVal[m],l.effectValMax[m],p->data[j][5+m*2])) {
                      notMatched=true;
                      break;
                    }
                  }
                }
                break;
            }
            if (notMatched) continue;
          }

          matched=true;
        }
        if (matched) {
          curQueryResults.push_back(FurnaceGUIQueryResult(e->getCurrentSubSong(),i,k,j));
        }
      }
    }
  }
  queryViewingResults=true;
}

void FurnaceGUI::doReplace() {
  doFind();
  queryViewingResults=false;

  bool* touched[DIV_MAX_CHANS];
  memset(touched,0,DIV_MAX_CHANS*sizeof(bool*));

  UndoStep us;
  us.type=GUI_UNDO_REPLACE;

  short prevVal[DIV_MAX_COLS];
  memset(prevVal,0,DIV_MAX_COLS*sizeof(short));

  for (FurnaceGUIQueryResult& i: curQueryResults) {
    int patIndex=e->song.subsong[i.subsong]->orders.ord[i.x][i.order];
    DivPattern* p=e->song.subsong[i.subsong]->pat[i.x].getPattern(patIndex,true);
    if (touched[i.x]==NULL) {
      touched[i.x]=new bool[DIV_MAX_PATTERNS*DIV_MAX_ROWS];
      memset(touched[i.x],0,DIV_MAX_PATTERNS*DIV_MAX_ROWS*sizeof(bool));
    }
    if (touched[i.x][(patIndex<<8)|i.y]) continue;
    touched[i.x][(patIndex<<8)|i.y]=true;

    memcpy(prevVal,p->data[i.y],DIV_MAX_COLS*sizeof(short));

    if (queryReplaceNoteDo) {
      switch (queryReplaceNoteMode) {
        case GUI_QUERY_REPLACE_SET:
          if (queryReplaceNote==130) { // macro release
            p->data[i.y][0]=102;
            p->data[i.y][1]=0;
          } else if (queryReplaceNote==129) { // note release
            p->data[i.y][0]=101;
            p->data[i.y][1]=0;
          } else if (queryReplaceNote==128) { // note off
            p->data[i.y][0]=100;
            p->data[i.y][1]=0;
          } else if (queryReplaceNote>=-60 && queryReplaceNote<120) { // note
            p->data[i.y][0]=(queryReplaceNote+60)%12;
            if (p->data[i.y][0]==0) p->data[i.y][0]=12;
            p->data[i.y][1]=(unsigned char)((queryReplaceNote-1)/12);
          } else { // invalid
            p->data[i.y][0]=0;
            p->data[i.y][1]=0;
          }
          break;
        case GUI_QUERY_REPLACE_ADD:
          if (p->data[i.y][0]<100) {
            int note=queryNote(p->data[i.y][0],p->data[i.y][1]);
            if (note>=-60 && note<120) {
              note+=queryReplaceNote;
              if (note<-60) note=-60;
              if (note>119) note=119;

              p->data[i.y][0]=(note+60)%12;
              p->data[i.y][1]=(unsigned char)(((note+60)/12)-5);
              if (p->data[i.y][0]==0) {
                p->data[i.y][0]=12;
                p->data[i.y][1]=(unsigned char)(p->data[i.y][1]-1);
              }
            }
          }
          break;
        case GUI_QUERY_REPLACE_ADD_OVERFLOW:
          if (p->data[i.y][0]<100) {
            int note=queryNote(p->data[i.y][0],p->data[i.y][1]);
            if (note>=-60 && note<120) {
              note+=queryReplaceNote;
              if (note<-60) {
                while (note<-60) note+=180;
              } else if (note>119) {
                while (note>119) note-=180;
              }

              p->data[i.y][0]=(note+60)%12;
              p->data[i.y][1]=(unsigned char)(((note+60)/12)-5);
              if (p->data[i.y][0]==0) {
                p->data[i.y][0]=12;
                p->data[i.y][1]=(unsigned char)(p->data[i.y][1]-1);
              }
            }
          }
          break;
        case GUI_QUERY_REPLACE_SCALE:
          break;
        case GUI_QUERY_REPLACE_CLEAR:
          p->data[i.y][0]=0;
          p->data[i.y][1]=0;
          break;
      }
    }

    if (queryReplaceInsDo) {
      switch (queryReplaceInsMode) {
        case GUI_QUERY_REPLACE_SET:
          p->data[i.y][2]=queryReplaceIns;
          break;
        case GUI_QUERY_REPLACE_ADD:
          if (p->data[i.y][2]>=0) {
            p->data[i.y][2]+=queryReplaceIns;
            if (p->data[i.y][2]<0) p->data[i.y][2]=0;
            if (p->data[i.y][2]>255) p->data[i.y][2]=255;
          }
          break;
        case GUI_QUERY_REPLACE_ADD_OVERFLOW:
          if (p->data[i.y][2]>=0) p->data[i.y][2]=(p->data[i.y][2]+queryReplaceIns)&0xff;
          break;
        case GUI_QUERY_REPLACE_SCALE:
          if (p->data[i.y][2]>=0) {
            p->data[i.y][2]=(p->data[i.y][2]*queryReplaceIns)/100;
            if (p->data[i.y][2]<0) p->data[i.y][2]=0;
            if (p->data[i.y][2]>255) p->data[i.y][2]=255;
          }
          break;
        case GUI_QUERY_REPLACE_CLEAR:
          p->data[i.y][2]=-1;
          break;
      }
    }

    if (queryReplaceVolDo) {
      switch (queryReplaceVolMode) {
        case GUI_QUERY_REPLACE_SET:
          p->data[i.y][3]=queryReplaceVol;
          break;
        case GUI_QUERY_REPLACE_ADD:
          if (p->data[i.y][3]>=0) {
            p->data[i.y][3]+=queryReplaceVol;
            if (p->data[i.y][3]<0) p->data[i.y][3]=0;
            if (p->data[i.y][3]>255) p->data[i.y][3]=255;
          }
          break;
        case GUI_QUERY_REPLACE_ADD_OVERFLOW:
          if (p->data[i.y][3]>=0) p->data[i.y][3]=(p->data[i.y][3]+queryReplaceVol)&0xff;
          break;
        case GUI_QUERY_REPLACE_SCALE:
          if (p->data[i.y][3]>=0) {
            p->data[i.y][3]=(p->data[i.y][3]*queryReplaceVol)/100;
            if (p->data[i.y][3]<0) p->data[i.y][3]=0;
            if (p->data[i.y][3]>255) p->data[i.y][3]=255;
          }
          break;
        case GUI_QUERY_REPLACE_CLEAR:
          p->data[i.y][3]=-1;
          break;
      }
    }

    signed char effectOrder[8];
    memset(effectOrder,-1,8);

    switch (queryReplaceEffectPos) {
      case 0: // clear
        for (int j=0; j<e->song.subsong[i.subsong]->pat[i.x].effectCols; j++) {
          effectOrder[j]=j;
        }
        break;
      case 1: { // replace matches
        int placementIndex=0;
        for (int j=0; j<e->song.subsong[i.subsong]->pat[i.x].effectCols; j++) {
          if (p->data[i.y][4+j*2]!=-1 || p->data[i.y][5+j*2]!=-1) {
            effectOrder[placementIndex++]=j;
          }
        }
        break;
      }
      case 2: { // replace matches then free spaces
        int placementIndex=0;
        for (int j=0; j<e->song.subsong[i.subsong]->pat[i.x].effectCols; j++) {
          if (p->data[i.y][4+j*2]!=-1 || p->data[i.y][5+j*2]!=-1) {
            effectOrder[placementIndex++]=j;
          }
        }
        for (int j=0; j<e->song.subsong[i.subsong]->pat[i.x].effectCols; j++) {
          if (p->data[i.y][4+j*2]==-1 && p->data[i.y][5+j*2]==-1) {
            effectOrder[placementIndex++]=j;
          }
        }
        break;
      }
      case 3: { // insert in free spaces
        int placementIndex=0;
        for (int j=0; j<e->song.subsong[i.subsong]->pat[i.x].effectCols; j++) {
          if (p->data[i.y][4+j*2]==-1 && p->data[i.y][5+j*2]==-1) {
            effectOrder[placementIndex++]=j;
          }
        }
        break;
      }
    }

    for (int j=0; j<queryReplaceEffectCount; j++) {
      signed char pos=effectOrder[j];
      if (pos==-1) continue;
      if (queryReplaceEffectDo[j]) {
        switch (queryReplaceEffectMode[j]) {
          case GUI_QUERY_REPLACE_SET:
            p->data[i.y][4+pos*2]=queryReplaceEffect[j];
            break;
          case GUI_QUERY_REPLACE_ADD:
            if (p->data[i.y][4+pos*2]>=0) {
              p->data[i.y][4+pos*2]+=queryReplaceEffect[j];
              if (p->data[i.y][4+pos*2]<0) p->data[i.y][4+pos*2]=0;
              if (p->data[i.y][4+pos*2]>255) p->data[i.y][4+pos*2]=255;
            }
            break;
          case GUI_QUERY_REPLACE_ADD_OVERFLOW:
            if (p->data[i.y][4+pos*2]>=0) p->data[i.y][4+pos*2]=(p->data[i.y][4+pos*2]+queryReplaceEffect[j])&0xff;
            break;
          case GUI_QUERY_REPLACE_SCALE:
            if (p->data[i.y][4+pos*2]>=0) {
              p->data[i.y][4+pos*2]=(p->data[i.y][4+pos*2]*queryReplaceEffect[j])/100;
              if (p->data[i.y][4+pos*2]<0) p->data[i.y][4+pos*2]=0;
              if (p->data[i.y][4+pos*2]>255) p->data[i.y][4+pos*2]=255;
            }
            break;
          case GUI_QUERY_REPLACE_CLEAR:
            p->data[i.y][4+pos*2]=-1;
            break;
        }
      }

      if (queryReplaceEffectValDo[j]) {
        switch (queryReplaceEffectValMode[j]) {
          case GUI_QUERY_REPLACE_SET:
            p->data[i.y][5+pos*2]=queryReplaceEffectVal[j];
            break;
          case GUI_QUERY_REPLACE_ADD:
            if (p->data[i.y][5+pos*2]>=0) {
              p->data[i.y][5+pos*2]+=queryReplaceEffectVal[j];
              if (p->data[i.y][5+pos*2]<0) p->data[i.y][5+pos*2]=0;
              if (p->data[i.y][5+pos*2]>255) p->data[i.y][5+pos*2]=255;
            }
            break;
          case GUI_QUERY_REPLACE_ADD_OVERFLOW:
            if (p->data[i.y][5+pos*2]>=0) p->data[i.y][5+pos*2]=(p->data[i.y][5+pos*2]+queryReplaceEffectVal[j])&0xff;
            break;
          case GUI_QUERY_REPLACE_SCALE:
            if (p->data[i.y][5+pos*2]>=0) {
              p->data[i.y][5+pos*2]=(p->data[i.y][5+pos*2]*queryReplaceEffectVal[j])/100;
              if (p->data[i.y][5+pos*2]<0) p->data[i.y][5+pos*2]=0;
              if (p->data[i.y][5+pos*2]>255) p->data[i.y][5+pos*2]=255;
            }
            break;
          case GUI_QUERY_REPLACE_CLEAR:
            p->data[i.y][5+pos*2]=-1;
            break;
        }
      }
    }

    // issue undo step
    for (int j=0; j<DIV_MAX_COLS; j++) {
      if (p->data[i.y][j]!=prevVal[j]) {
        us.pat.push_back(UndoPatternData(i.subsong,i.x,patIndex,i.y,j,prevVal[j],p->data[i.y][j]));
      }
    }
  }

  for (int i=0; i<DIV_MAX_CHANS; i++) {
    if (touched[i]!=NULL) delete[] touched[i];
  }

  if (!curQueryResults.empty()) {
    MARK_MODIFIED;
  }

  if (!us.pat.empty()) {
    undoHist.push_back(us);
    redoHist.clear();
    if (undoHist.size()>settings.maxUndoSteps) undoHist.pop_front();
  }
}

#define FIRST_VISIBLE(x) (x==GUI_QUERY_MATCH || x==GUI_QUERY_MATCH_NOT || x==GUI_QUERY_RANGE || x==GUI_QUERY_RANGE_NOT)
#define SECOND_VISIBLE(x) (x==GUI_QUERY_RANGE || x==GUI_QUERY_RANGE_NOT)

void FurnaceGUI::drawFindReplace() {
  if (nextWindow==GUI_WINDOW_FIND) {
    findOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!findOpen) return;
  ImGui::SetNextWindowSizeConstraints(ImVec2(64.0f*dpiScale,32.0f*dpiScale),ImVec2(canvasW,canvasH));
  if (ImGui::Begin("Find/Replace",&findOpen,globalWinFlags)) {
    if (curQuery.empty()) {
      curQuery.push_back(FurnaceGUIFindQuery());
    }
    int index=0;
    int eraseIndex=-1;
    char tempID[1024];
    if (ImGui::BeginTabBar("FindOrReplace")) {
      if (ImGui::BeginTabItem("Find")) {
        if (queryViewingResults) {
          if (!curQueryResults.empty()) {
            ImVec2 avail=ImGui::GetContentRegionAvail();
            avail.y-=ImGui::GetFrameHeightWithSpacing();
            if (ImGui::BeginTable("FindResults",4,ImGuiTableFlags_Borders|ImGuiTableFlags_ScrollY,avail)) {
              ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed,ImGui::CalcTextSize("order").x);
              ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed,ImGui::CalcTextSize("row").x);
              ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch);
              ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthFixed);

              ImGui::TableSetupScrollFreeze(0,1);

              ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
              ImGui::TableNextColumn();
              ImGui::Text("order");
              ImGui::TableNextColumn();
              ImGui::Text("row");
              ImGui::TableNextColumn();
              ImGui::Text("channel");
              ImGui::TableNextColumn();
              ImGui::Text("go");

              int index=0;
              for (FurnaceGUIQueryResult& i: curQueryResults) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                if (settings.orderRowsBase==1) {
                  ImGui::Text("%.2X",i.order);
                } else {
                  ImGui::Text("%d",i.order);
                }
                ImGui::TableNextColumn();
                if (settings.patRowsBase==1) {
                  ImGui::Text("%.2X",i.y);
                } else {
                  ImGui::Text("%d",i.y);
                }
                ImGui::TableNextColumn();
                ImGui::Text("%d (%s)",i.x+1,e->getChannelName(i.x));
                if (ImGui::TableNextColumn()) {
                  snprintf(tempID,1024,ICON_FA_CHEVRON_RIGHT "##_FR%d",index);
                  if (ImGui::Selectable(tempID)) {
                    e->changeSongP(i.subsong);
                    if (e->isPlaying()) {
                      followPattern=false;
                    } else {
                      e->setOrder(i.order);
                    }
                    curOrder=i.order;
                    cursor.xCoarse=i.x;
                    cursor.xFine=0;
                    cursor.y=i.y;
                    selStart=cursor;
                    selEnd=cursor;
                    demandScrollX=true;
                    updateScroll(cursor.y);
                    nextWindow=GUI_WINDOW_PATTERN;
                  }
                }
                index++;
              }
              ImGui::EndTable();
            }
          } else {
            ImGui::Text("no matches found!");
          }
          if (ImGui::Button("Back")) {
            queryViewingResults=false;
          }
        } else {
          for (FurnaceGUIFindQuery& i: curQuery) {
            ImGui::PushID(index+0x100);
            if (ImGui::BeginTable("FindRep",4,ImGuiTableFlags_BordersOuter)) {
              ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
              ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.5);
              ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.25);
              ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthStretch,0.25);
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::Text("Note");
              ImGui::TableNextColumn();
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              ImGui::Combo("##NCondition",&i.noteMode,queryModes,GUI_QUERY_MAX);
              ImGui::TableNextColumn();
              if (FIRST_VISIBLE(i.noteMode)) {
                if ((i.noteMode==GUI_QUERY_RANGE || i.noteMode==GUI_QUERY_RANGE_NOT) && i.note>=120) {
                  i.note=0;
                }
                if (i.note==130) {
                  snprintf(tempID,1024,"%s##MREL",macroRelLabel);
                } else if (i.note==129) {
                  snprintf(tempID,1024,"%s##NREL",noteRelLabel);
                } else if (i.note==128) {
                  snprintf(tempID,1024,"%s##NOFF",noteOffLabel);
                } else if (i.note>=-60 && i.note<120) {
                  snprintf(tempID,1024,"%s",noteNames[i.note+60]);
                } else {
                  snprintf(tempID,1024,"???");
                  i.note=0;
                }
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::BeginCombo("##NN1",tempID)) {
                  for (int j=0; j<180; j++) {
                    snprintf(tempID,1024,"%s",noteNames[j]);
                    if (ImGui::Selectable(tempID,i.note==(j-60))) {
                      i.note=j-60;
                    }
                  }
                  if (i.noteMode!=GUI_QUERY_RANGE && i.noteMode!=GUI_QUERY_RANGE_NOT) {
                    if (ImGui::Selectable(noteOffLabel,i.note==128)) {
                      i.note=128;
                    }
                    if (ImGui::Selectable(noteRelLabel,i.note==129)) {
                      i.note=129;
                    }
                    if (ImGui::Selectable(macroRelLabel,i.note==130)) {
                      i.note=130;
                    }
                  }
                  ImGui::EndCombo();
                }
              }
              ImGui::TableNextColumn();
              if (SECOND_VISIBLE(i.noteMode)) {
                if (i.noteMax<-60 || i.noteMax>=120) {
                  i.noteMax=0;
                }
                if (i.noteMax>=-60 && i.noteMax<120) {
                  snprintf(tempID,1024,"%s",noteNames[i.noteMax+60]);
                } else {
                  snprintf(tempID,1024,"???");
                }
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::BeginCombo("##NN2",tempID)) {
                  for (int j=0; j<180; j++) {
                    snprintf(tempID,1024,"%s",noteNames[j]);
                    if (ImGui::Selectable(tempID,i.noteMax==(j-60))) {
                      i.noteMax=j-60;
                    }
                  }
                  ImGui::EndCombo();
                }
              }

              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::Text("Ins");
              ImGui::TableNextColumn();
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              ImGui::Combo("##ICondition",&i.insMode,queryModes,GUI_QUERY_MAX);
              ImGui::TableNextColumn();
              if (FIRST_VISIBLE(i.insMode)) {
                snprintf(tempID,1024,"%.2X",i.ins);
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::BeginCombo("II1",tempID)) {
                  for (int j=0; j<256; j++) {
                    snprintf(tempID,1024,"%.2X",j);
                    if (ImGui::Selectable(tempID,i.ins==j)) {
                      i.ins=j;
                    }
                  }
                  ImGui::EndCombo();
                }
              }
              ImGui::TableNextColumn();
              if (SECOND_VISIBLE(i.insMode)) {
                snprintf(tempID,1024,"%.2X",i.insMax);
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::BeginCombo("II2",tempID)) {
                  for (int j=0; j<256; j++) {
                    snprintf(tempID,1024,"%.2X",j);
                    if (ImGui::Selectable(tempID,i.insMax==j)) {
                      i.insMax=j;
                    }
                  }
                  ImGui::EndCombo();
                }
              }

              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::Text("Volume");
              ImGui::TableNextColumn();
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              ImGui::Combo("##VCondition",&i.volMode,queryModes,GUI_QUERY_MAX);
              ImGui::TableNextColumn();
              if (FIRST_VISIBLE(i.volMode)) {
                snprintf(tempID,1024,"%.2X",i.vol);
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::BeginCombo("VV1",tempID)) {
                  for (int j=0; j<256; j++) {
                    snprintf(tempID,1024,"%.2X",j);
                    if (ImGui::Selectable(tempID,i.vol==j)) {
                      i.vol=j;
                    }
                  }
                  ImGui::EndCombo();
                }
              }
              ImGui::TableNextColumn();
              if (SECOND_VISIBLE(i.volMode)) {
                snprintf(tempID,1024,"%.2X",i.volMax);
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::BeginCombo("VV2",tempID)) {
                  for (int j=0; j<256; j++) {
                    snprintf(tempID,1024,"%.2X",j);
                    if (ImGui::Selectable(tempID,i.volMax==j)) {
                      i.volMax=j;
                    }
                  }
                  ImGui::EndCombo();
                }
              }

              for (int j=0; j<i.effectCount; j++) {
                ImGui::PushID(0x1000+j);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Effect");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::Combo("##ECondition",&i.effectMode[j],queryModes,GUI_QUERY_MAX);
                ImGui::TableNextColumn();
                if (FIRST_VISIBLE(i.effectMode[j])) {
                  snprintf(tempID,1024,"%.2X",i.effect[j]);
                  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                  if (ImGui::BeginCombo("EE1",tempID)) {
                    for (int k=0; k<256; k++) {
                      snprintf(tempID,1024,"%.2X",k);
                      if (ImGui::Selectable(tempID,i.effect[j]==k)) {
                        i.effect[j]=k;
                      }
                    }
                    ImGui::EndCombo();
                  }
                }
                ImGui::TableNextColumn();
                if (SECOND_VISIBLE(i.effectMode[j])) {
                  snprintf(tempID,1024,"%.2X",i.effectMax[j]);
                  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                  if (ImGui::BeginCombo("EE2",tempID)) {
                    for (int k=0; k<256; k++) {
                      snprintf(tempID,1024,"%.2X",k);
                      if (ImGui::Selectable(tempID,i.effectMax[j]==k)) {
                        i.effectMax[j]=k;
                      }
                    }
                    ImGui::EndCombo();
                  }
                }
                
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Value");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::Combo("##EVCondition",&i.effectValMode[j],queryModes,GUI_QUERY_MAX);
                ImGui::TableNextColumn();
                if (FIRST_VISIBLE(i.effectValMode[j])) {
                  snprintf(tempID,1024,"%.2X",i.effectVal[j]);
                  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                  if (ImGui::BeginCombo("EV1",tempID)) {
                    for (int k=0; k<256; k++) {
                      snprintf(tempID,1024,"%.2X",k);
                      if (ImGui::Selectable(tempID,i.effectVal[j]==k)) {
                        i.effectVal[j]=k;
                      }
                    }
                    ImGui::EndCombo();
                  }
                }
                ImGui::TableNextColumn();
                if (SECOND_VISIBLE(i.effectValMode[j])) {
                  snprintf(tempID,1024,"%.2X",i.effectValMax[j]);
                  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                  if (ImGui::BeginCombo("EV2",tempID)) {
                    for (int k=0; k<256; k++) {
                      snprintf(tempID,1024,"%.2X",k);
                      if (ImGui::Selectable(tempID,i.effectValMax[j]==k)) {
                        i.effectValMax[j]=k;
                      }
                    }
                    ImGui::EndCombo();
                  }
                }

                ImGui::PopID();
              }
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              if (ImGui::Button(ICON_FA_MINUS "##DelQuery")) {
                eraseIndex=index;
              }
              ImGui::TableNextColumn();
              if (i.effectCount<8) {
                if (ImGui::Button("Add effect")) {
                  i.effectCount++;
                }
              }
              ImGui::TableNextColumn();
              if (i.effectCount>0) {
                if (ImGui::Button("Remove effect")) {
                  i.effectCount--;
                }
              }
              ImGui::EndTable();
            }
            ImGui::PopID();
            index++;
          }
          if (eraseIndex>=0) {
            curQuery.erase(curQuery.begin()+eraseIndex);
          }
          if (ImGui::Button(ICON_FA_PLUS "##AddQuery")) {
            curQuery.push_back(FurnaceGUIFindQuery());
          }

          if (ImGui::BeginTable("QueryLimits",3)) {
            ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.5f);
            ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.5f);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            ImGui::Text("Search range:");

            if (ImGui::RadioButton("Song",curQueryRangeY==0)) {
              curQueryRangeY=0;
            }
            if (ImGui::RadioButton("Selection",curQueryRangeY==1)) {
              curQueryRangeY=1;
            }
            if (ImGui::RadioButton("Pattern",curQueryRangeY==2)) {
              curQueryRangeY=2;
            }

            ImGui::TableNextColumn();
            ImGui::Checkbox("Confine to channels",&curQueryRangeX);

            ImGui::BeginDisabled(!curQueryRangeX);
            snprintf(tempID,1024,"%d: %s",curQueryRangeXMin+1,e->getChannelName(curQueryRangeXMin));
            if (ImGui::BeginCombo("From",tempID)) {
              for (int i=0; i<e->getTotalChannelCount(); i++) {
                snprintf(tempID,1024,"%d: %s",i+1,e->getChannelName(i));
                if (ImGui::Selectable(tempID,curQueryRangeXMin==i)) {
                  curQueryRangeXMin=i;
                }
              }
              ImGui::EndCombo();
            }

            snprintf(tempID,1024,"%d: %s",curQueryRangeXMax+1,e->getChannelName(curQueryRangeXMax));
            if (ImGui::BeginCombo("To",tempID)) {
              for (int i=0; i<e->getTotalChannelCount(); i++) {
                snprintf(tempID,1024,"%d: %s",i+1,e->getChannelName(i));
                if (ImGui::Selectable(tempID,curQueryRangeXMax==i)) {
                  curQueryRangeXMax=i;
                }
              }
              ImGui::EndCombo();
            }
            ImGui::EndDisabled();

            ImGui::TableNextColumn();
            ImGui::Text("Match effect position:");

            if (ImGui::RadioButton("No",curQueryEffectPos==0)) {
              curQueryEffectPos=0;
            }
            if (ImGui::IsItemHovered()) {
              ImGui::SetTooltip("match effects regardless of position.");
            }
            if (ImGui::RadioButton("Lax",curQueryEffectPos==1)) {
              curQueryEffectPos=1;
            }
            if (ImGui::IsItemHovered()) {
              ImGui::SetTooltip("match effects only if they appear in-order.");
            }
            if (ImGui::RadioButton("Strict",curQueryEffectPos==2)) {
              curQueryEffectPos=2;
            }
            if (ImGui::IsItemHovered()) {
              ImGui::SetTooltip("match effects only if they appear exactly as specified.");
            }

            ImGui::EndTable();
          }

          if (ImGui::Button("Find")) {
            doFind();
          }
        }
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Replace")) {
        if (ImGui::BeginTable("QueryReplace",3,ImGuiTableFlags_BordersOuter)) {
          ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
          ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.5);
          ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.5);

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::Checkbox("Note",&queryReplaceNoteDo);
          ImGui::TableNextColumn();
          ImGui::BeginDisabled(!queryReplaceNoteDo);
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          ImGui::Combo("##NRMode",&queryReplaceNoteMode,queryReplaceModes,GUI_QUERY_REPLACE_MAX);
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (queryReplaceNoteMode==GUI_QUERY_REPLACE_SET) {
            if (queryReplaceNote==130) {
              snprintf(tempID,1024,"%s##MREL",macroRelLabel);
            } else if (queryReplaceNote==129) {
              snprintf(tempID,1024,"%s##NREL",noteRelLabel);
            } else if (queryReplaceNote==128) {
              snprintf(tempID,1024,"%s##NOFF",noteOffLabel);
            } else if (queryReplaceNote>=-60 && queryReplaceNote<120) {
              snprintf(tempID,1024,"%s",noteNames[queryReplaceNote+60]);
            } else {
              snprintf(tempID,1024,"???");
              queryReplaceNote=0;
            }
            if (ImGui::BeginCombo("##NRValueC",tempID)) {
              for (int j=0; j<180; j++) {
                snprintf(tempID,1024,"%s",noteNames[j]);
                if (ImGui::Selectable(tempID,queryReplaceNote==(j-60))) {
                  queryReplaceNote=j-60;
                }
              }
              if (ImGui::Selectable(noteOffLabel,queryReplaceNote==128)) {
                queryReplaceNote=128;
              }
              if (ImGui::Selectable(noteRelLabel,queryReplaceNote==129)) {
                queryReplaceNote=129;
              }
              if (ImGui::Selectable(macroRelLabel,queryReplaceNote==130)) {
                queryReplaceNote=130;
              }
              ImGui::EndCombo();
            }
          } else if (queryReplaceNoteMode==GUI_QUERY_REPLACE_ADD || queryReplaceNoteMode==GUI_QUERY_REPLACE_ADD_OVERFLOW) {
            if (ImGui::InputInt("##NRValue",&queryReplaceNote,1,12)) {
              if (queryReplaceNote<-180) queryReplaceNote=-180;
              if (queryReplaceNote>180) queryReplaceNote=180;
            }
          } else if (queryReplaceNoteMode==GUI_QUERY_REPLACE_SCALE) {
            ImGui::Text("INVALID");
          }
          ImGui::EndDisabled();

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::Checkbox("Ins",&queryReplaceInsDo);
          ImGui::TableNextColumn();
          ImGui::BeginDisabled(!queryReplaceInsDo);
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          ImGui::Combo("##IRMode",&queryReplaceInsMode,queryReplaceModes,GUI_QUERY_REPLACE_MAX);
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (queryReplaceInsMode==GUI_QUERY_REPLACE_SET) {
            if (ImGui::InputScalar("##IRValueH",ImGuiDataType_S32,&queryReplaceIns,&_ONE,&_SIXTEEN,"%.2X",ImGuiInputTextFlags_CharsHexadecimal)) {
              if (queryReplaceIns<0) queryReplaceIns=0;
              if (queryReplaceIns>255) queryReplaceIns=255;
            }
          } else if (queryReplaceInsMode==GUI_QUERY_REPLACE_ADD || queryReplaceInsMode==GUI_QUERY_REPLACE_ADD_OVERFLOW) {
            if (ImGui::InputInt("##IRValue",&queryReplaceIns,1,12)) {
              if (queryReplaceIns<-255) queryReplaceIns=-255;
              if (queryReplaceIns>255) queryReplaceIns=255;
            }
          } else if (queryReplaceInsMode==GUI_QUERY_REPLACE_SCALE) {
            if (queryReplaceIns<0) queryReplaceIns=0;
            if (queryReplaceIns>400) queryReplaceIns=400;
            if (ImGui::InputInt("##IRValue",&queryReplaceIns,1,12)) {
              if (queryReplaceIns<0) queryReplaceIns=0;
              if (queryReplaceIns>400) queryReplaceIns=400;
            }
          }
          ImGui::EndDisabled();

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::Checkbox("Volume",&queryReplaceVolDo);
          ImGui::TableNextColumn();
          ImGui::BeginDisabled(!queryReplaceVolDo);
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          ImGui::Combo("##VRMode",&queryReplaceVolMode,queryReplaceModes,GUI_QUERY_REPLACE_MAX);
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (queryReplaceVolMode==GUI_QUERY_REPLACE_SET) {
            if (ImGui::InputScalar("##VRValueH",ImGuiDataType_S32,&queryReplaceVol,&_ONE,&_SIXTEEN,"%.2X",ImGuiInputTextFlags_CharsHexadecimal)) {
              if (queryReplaceVol<0) queryReplaceVol=0;
              if (queryReplaceVol>255) queryReplaceVol=255;
            }
          } else if (queryReplaceVolMode==GUI_QUERY_REPLACE_ADD || queryReplaceVolMode==GUI_QUERY_REPLACE_ADD_OVERFLOW) {
            if (ImGui::InputInt("##VRValue",&queryReplaceVol,1,12)) {
              if (queryReplaceVol<-255) queryReplaceVol=-255;
              if (queryReplaceVol>255) queryReplaceVol=255;
            }
          } else if (queryReplaceVolMode==GUI_QUERY_REPLACE_SCALE) {
            if (queryReplaceVol<0) queryReplaceVol=0;
            if (queryReplaceVol>400) queryReplaceVol=400;
            if (ImGui::InputInt("##VRValue",&queryReplaceVol,1,12)) {
              if (queryReplaceVol<0) queryReplaceVol=0;
              if (queryReplaceVol>400) queryReplaceVol=400;
            }
          }
          ImGui::EndDisabled();

          for (int i=0; i<queryReplaceEffectCount; i++) {
            ImGui::PushID(0x100+i);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Checkbox("Effect",&queryReplaceEffectDo[i]);
            ImGui::TableNextColumn();
            ImGui::BeginDisabled(!queryReplaceEffectDo[i]);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::Combo("##ERMode",&queryReplaceEffectMode[i],queryReplaceModes,GUI_QUERY_REPLACE_MAX);
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (queryReplaceEffectMode[i]==GUI_QUERY_REPLACE_SET) {
              if (ImGui::InputScalar("##ERValueH",ImGuiDataType_S32,&queryReplaceEffect[i],&_ONE,&_SIXTEEN,"%.2X",ImGuiInputTextFlags_CharsHexadecimal)) {
                if (queryReplaceEffect[i]<0) queryReplaceEffect[i]=0;
                if (queryReplaceEffect[i]>255) queryReplaceEffect[i]=255;
              }
            } else if (queryReplaceEffectMode[i]==GUI_QUERY_REPLACE_ADD || queryReplaceEffectMode[i]==GUI_QUERY_REPLACE_ADD_OVERFLOW) {
              if (ImGui::InputInt("##ERValue",&queryReplaceEffect[i],1,12)) {
                if (queryReplaceEffect[i]<-255) queryReplaceEffect[i]=-255;
                if (queryReplaceEffect[i]>255) queryReplaceEffect[i]=255;
              }
            } else if (queryReplaceEffectMode[i]==GUI_QUERY_REPLACE_SCALE) {
              if (queryReplaceEffect[i]<0) queryReplaceEffect[i]=0;
              if (queryReplaceEffect[i]>400) queryReplaceEffect[i]=400;
              if (ImGui::InputInt("##ERValue",&queryReplaceEffect[i],1,12)) {
                if (queryReplaceEffect[i]<0) queryReplaceEffect[i]=0;
                if (queryReplaceEffect[i]>400) queryReplaceEffect[i]=400;
              }
            }
            ImGui::EndDisabled();

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Checkbox("Value",&queryReplaceEffectValDo[i]);
            ImGui::TableNextColumn();
            ImGui::BeginDisabled(!queryReplaceEffectValDo[i]);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::Combo("##ERModeV",&queryReplaceEffectValMode[i],queryReplaceModes,GUI_QUERY_REPLACE_MAX);
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (queryReplaceEffectValMode[i]==GUI_QUERY_REPLACE_SET) {
              if (ImGui::InputScalar("##ERValueVH",ImGuiDataType_S32,&queryReplaceEffectVal[i],&_ONE,&_SIXTEEN,"%.2X",ImGuiInputTextFlags_CharsHexadecimal)) {
                if (queryReplaceEffectVal[i]<0) queryReplaceEffectVal[i]=0;
                if (queryReplaceEffectVal[i]>255) queryReplaceEffectVal[i]=255;
              }
            } else if (queryReplaceEffectValMode[i]==GUI_QUERY_REPLACE_ADD || queryReplaceEffectValMode[i]==GUI_QUERY_REPLACE_ADD_OVERFLOW) {
              if (ImGui::InputInt("##ERValueV",&queryReplaceEffectVal[i],1,12)) {
                if (queryReplaceEffectVal[i]<-255) queryReplaceEffectVal[i]=-255;
                if (queryReplaceEffectVal[i]>255) queryReplaceEffectVal[i]=255;
              }
            } else if (queryReplaceEffectValMode[i]==GUI_QUERY_REPLACE_SCALE) {
              if (queryReplaceEffectVal[i]<0) queryReplaceEffectVal[i]=0;
              if (queryReplaceEffectVal[i]>400) queryReplaceEffectVal[i]=400;
              if (ImGui::InputInt("##ERValueV",&queryReplaceEffectVal[i],1,12)) {
                if (queryReplaceEffectVal[i]<0) queryReplaceEffectVal[i]=0;
                if (queryReplaceEffectVal[i]>400) queryReplaceEffectVal[i]=400;
              }
            }
            ImGui::EndDisabled();

            ImGui::PopID();
          }

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::TableNextColumn();
          if (queryReplaceEffectCount<8) {
            if (ImGui::Button("Add effect")) {
              queryReplaceEffectCount++;
            }
          }
          ImGui::TableNextColumn();
          if (queryReplaceEffectCount>0) {
            if (ImGui::Button("Remove effect")) {
              queryReplaceEffectCount--;
            }
          }

          ImGui::EndTable();
        }
        ImGui::Text("Effect replace mode:");
        if (ImGui::RadioButton("Clear effects",queryReplaceEffectPos==0)) {
          queryReplaceEffectPos=0;
        }
        if (ImGui::RadioButton("Replace matches only",queryReplaceEffectPos==1)) {
          queryReplaceEffectPos=1;
        }
        if (ImGui::RadioButton("Replace matches, then free spaces",queryReplaceEffectPos==2)) {
          queryReplaceEffectPos=2;
        }
        if (ImGui::RadioButton("Insert in free spaces",queryReplaceEffectPos==3)) {
          queryReplaceEffectPos=3;
        }
        if (ImGui::Button("Replace##QueryReplace")) {
          doReplace();
        }
        ImGui::EndTabItem();
      }
      ImGui::EndTabBar();
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_FIND;
  ImGui::End();
}
